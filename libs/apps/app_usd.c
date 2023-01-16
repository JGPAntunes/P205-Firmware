/*
* @file		app_usd.c
* @date		September 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the app_usd aplication.
*               Based on the Nordic SDK and FatFs - Generic FAT Filesystem Module 
*               http://elm-chan.org/fsw/ff/00index_e.html
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/
/* Interface */
#include "app_usd.h"

/* Utilities */
#include "sense_library/periph/rtc.h"

/* Debug - libs */
#include "sense_library/utils/debug.h"

/* FATFS */
#include "ff.h"
#include "diskio_blkdev.h"

/* SDK */
#include "nrf_gpio.h"
#include "nrf_block_dev_sdc.h"
#include "nrf_delay.h"

/* Debug - libs */
#include "sense_library/utils/debug.h"

/* Utils */
#include "sense_library/utils/utils.h"
/********************************** Private ************************************/
/* SDC block device definition */
NRF_BLOCK_DEV_SDC_DEFINE(
        m_block_dev_sdc,
        NRF_BLOCK_DEV_SDC_CONFIG(
                SDC_SECTOR_SIZE,
                APP_SDCARD_CONFIG(APP_USD_MOSI_PIN, APP_USD_MISO_PIN, APP_USD_SCK_PIN, APP_USD_CS_PIN)
         ),
         NFR_BLOCK_DEV_INFO_CONFIG("Nordic", "SDC", "1.00")
);

/* Filesystem object */
static FATFS fs;

/* Pointer to the directory object structure */
static DIR dir;

/* File information structure */
static FILINFO fno;

/* Pointer to the file object structure */
static FIL file;

/*
* Variável estado do loop de sampling
*/
static app_usd_sampling_states _current_state;

/*
* Variável de tentativas de inicialização da driver através da app
*/
static uint8_t _retries;

/*
* Ponteiro para a variável com a informação do paciente a recolher data
*/
static patient_info _patient_info;

/* 
* Variável que armazena internamente as configurações do equipamento 
*/
static device_config _device_config;

/* 
* Variável do estado do ficheiro APP_SDC_CONFIG_FILE do uSD
*/
static bool _valid_cfg_file;

/* 
* Variável do estado do ficheiro APP_SDC_CSV_FILE do uSD
*/
static bool _valid_csv_file;

/* 
* Variável do estado do paciente no ficheiro APP_SDC_CSV_FILE do uSD
*/
static bool _patient_checked;

/* 
* Variável do nome paciente no ficheiro APP_SDC_CSV_FILE do uSD
*/
static uint8_t _patient_name[APP_USD_DATAF_NAME_STR_SIZE];

/* 
* Variável do genero paciente no ficheiro APP_SDC_CSV_FILE do uSD
*/
static uint8_t _patient_gender[APP_USD_DATAF_NAME_STR_SIZE];

/*
* Buffer que armazena as medidas a serem escritas para o uSD.
*/
static uint8_t _meas_buffer[APP_USD_MEAS_BATCH_NUMBER][APP_USD_MEAS_BATCH_SIZE];

/*
* Variável que armazena o número de medidas armazenadas.
*/
static uint8_t _meas_buffer_index;

/*
* Variável que armazena o threshold para o número de uploads para o uSD.
*/
static uint8_t _upload_meas_th;

/*
* Variável que armazena o número de uploads para o uSD.
*/
static uint8_t _uploud_count;

/*
* Variável que armazena o número de meas definidas no config.h para o uSD.
*/
static uint8_t _meas_number;

/*
* Variável que armazena o número total em bytes das measurements
*/
static uint16_t _meas_size;

/*
* Variável que armazena o nome atual do APP_SDC_CSV_FILE a atualizar.
*/
char _actual_csv_file[APP_USD_DATAF_NAME_SIZE] = "\0";

/*
* Variável que armazena o número de ficheiros APP_SDC_CSV_FILE existentes no uSD.
*/
static uint8_t _data_file_count;

/*
* Variável que armazena o estado dos ficheiros APP_SDC_CSV_FILE e APP_SDC_CFG_FILE.
*/
static uint8_t _file_status;

/*
* Variável que armazena o estado da execução de save mode.
*/
static bool _save_mode_exec;

/* Private functions list */
void _app_usd_power_on(void);
void _app_usd_power_off(void);
uint8_t _app_usd_read_pacient_info(void);
void _app_usd_upload_meas(void);
bool _app_usd_mount(void);
bool _app_usd_unmount(void);
bool _app_usd_unmount_and_mount(void);
void _app_usd_init_global_variables(void);


/********************************** Public ************************************/
/*
 * @brief Function for initializing the uSD app. 
 *
 * @param[in] upload_meas_th  Value of measures threshold to upload to uSD                                 
 */
void app_usd_init(uint8_t upload_meas_th) {
   
  /* Inicialização de variáveis globais */  
  _app_usd_init_global_variables();

  /* Configuração do PIN de Enable Power */
  nrf_gpio_cfg_output(APP_USD_PWR_EN);
  _app_usd_power_off();  
  
  /* Precaution to discharge capacitors */
  nrf_delay_ms(1000);
  
  _save_mode_exec = false;
  /* Atualização interna das measures atuais */  
  uint8_t meas[] = MEASURES_CONTENT_SIZE;
  _meas_number = ARRAY_SIZE(meas);

  for(int i = 0 ; i < _meas_number ; i++) {
    _meas_size += meas[i];
  }
  
  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_1, "[app_usd_init] Init started \n");  

}


/*
 * @brief uSD application loop. 
 *
 * @param[in] save_mode  Value true to get in save_mode, false to exit     
 */
void app_usd_loop(bool save_mode) {

  uint32_t bytes_written = 0;
  uint32_t bytes_readed = 0;
  FRESULT ff_result;
  
  /* Colocar em save mode quando requerido e measurements todas enviadas */
  if(save_mode && _meas_buffer_index == 0) {
    _current_state = APP_USD_SAVE_MODE;
  }

  /* Aplication state machine */
  switch(_current_state) {
    case APP_USD_INIT:      
      
      if(_retries == 0) {
        return;
      }
            
      _app_usd_power_on();                                /* Power ON uSD */
      
      uint32_t bytes_written;
      FRESULT ff_result;
      DSTATUS disk_state = STA_NOINIT;
      
      /* Initialize FATFS disk I/O interface by providing the block device. */
      static diskio_blkdev_t drives[] = {
        DISKIO_BLOCKDEV_CONFIG(NRF_BLOCKDEV_BASE_ADDR(m_block_dev_sdc, block_dev), NULL)
      };

      diskio_blockdev_register(drives, ARRAY_SIZE(drives));
      
      debug_print_time(DEBUG_LEVEL_3, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_3, (uint8_t*) "[app_usd_loop] Initializing disk 0 (SDC)...\r\n");
      
      for(_retries; _retries && disk_state; --_retries) {
        disk_state = disk_initialize(0);
      }
      if(disk_state) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_loop] Disk initialization failed. Try re-insert uSD\r\n");            
        break;
      }

      uint32_t blocks_per_mb = (1024uL * 1024uL) / m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_size;
      uint32_t capacity = m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_count / blocks_per_mb;
      debug_print_time(DEBUG_LEVEL_3, rtc_get_milliseconds());
      debug_printf_string(DEBUG_LEVEL_3, (uint8_t*) "[app_usd_loop] Capacity: %d MB \r\n", capacity);
      
      /* Mount usd Card */
      if(!_app_usd_mount()) {
        break;
      }            
      _current_state = APP_USD_CONFIG;

      break;          
    case APP_USD_CONFIG:      
            
      /* Caso exista o ficheiro APP_USD_README_FILE */
      if(f_stat(APP_USD_README_FILE, NULL) == FR_OK) {

        /* Abre o ficheiro APP_USD_README_ID_SIZE ou cria caso não exista */
        ff_result = f_open(&file, APP_USD_README_FILE, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
        if (ff_result != FR_OK) {
          debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
          debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_loop] Unable to open or create file: %s\r\n", APP_USD_README_FILE);
          break;
        }

        uint8_t buffer[APP_USD_README_ID_SIZE];
        
        f_gets(buffer, APP_USD_README_ID_SIZE, &file);
        if (buffer == NULL) {
          debug_print_time(DEBUG_LEVEL_3, rtc_get_milliseconds());
          debug_printf_string(DEBUG_LEVEL_3, (uint8_t*) "[app_usd_loop] Unable to read %s\r\n", APP_USD_README_FILE);
          break;
        }
        
        /* Verificar o ID do APP_USD_README_FILE */
        if(!strcmp(buffer, (uint8_t*)APP_USD_README_ID)) {
          _current_state = APP_USD_GET_CFG_FILE;
          (void)f_close(&file);

          debug_print_time(DEBUG_LEVEL_3, rtc_get_milliseconds());
          debug_print_string(DEBUG_LEVEL_3, (uint8_t*) "[app_usd_loop] README write/creation was sucessfull\r\n");
          break;
        }
        
        /* ID do APP_USD_README_FILE não é válido, é necessário substituir */
        (void)f_close(&file);
        
      }      
      
      /* O ficheiro APP_USD_README_FILE não é válido ou não existe */
      ff_result = f_open(&file, APP_USD_README_FILE, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
      if (ff_result != FR_OK) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_loop] Unable to open or create file: %s\r\n", APP_USD_README_FILE);
        break;
      }
      
      uint8_t *data_tosend[] = {APP_USD_README_ID, APP_USD_README_STRING1, APP_USD_README_STRING2};
      uint8_t data_size_tosend[] = {APP_USD_README_ID_SIZE, APP_USD_README_STRING1_SIZE, APP_USD_README_STRING2_SIZE};
     
      for(int i = 0 ; i < ARRAY_SIZE(data_tosend) ; i++) {
                
        ff_result = f_write(&file, data_tosend[i], data_size_tosend[i] - 1, (UINT *) &bytes_written);
        if (ff_result != FR_OK) {
          debug_print_time(DEBUG_LEVEL_3, rtc_get_milliseconds());
          debug_print_string(DEBUG_LEVEL_3, (uint8_t*) "[app_usd_loop] Write failed.\r\n");
          break;
        } else {
          debug_print_time(DEBUG_LEVEL_3, rtc_get_milliseconds());
          debug_printf_string(DEBUG_LEVEL_3, (uint8_t*) "[app_usd_loop] %d bytes written.\r\n", bytes_written);
        }

      }

      (void)f_close(&file);            
      _current_state = APP_USD_GET_CFG_FILE;

      break;        
    case APP_USD_GET_CFG_FILE:
       
      if(f_stat(APP_USD_CONFIG_FILE, NULL) != FR_OK) {          /* Ficheiro APP_USD_CONFIG_FILE não existe */
        _valid_cfg_file = false;
        _current_state = APP_USD_GET_CSV_FILE;
        break;
      }                                                
              
      /* Abre o ficheiro APP_USD_CONFIG_FILE ou cria caso não exista */
      ff_result = f_open(&file, APP_USD_CONFIG_FILE, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
      if (ff_result != FR_OK) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_loop] Unable to open or create file: %s\r\n", APP_USD_CONFIG_FILE);
        break;
      } 

      uint16_t file_size = f_size(&file);
      char configs_buffer[APP_USD_CFG_SIZE] = "\0";
      uint8_t configs_found = 0;
      char config_strings[DEVICE_CONFIGS_NUMBER][30] = {"\0"};      
      
      ff_result = f_read(&file, configs_buffer, file_size, &bytes_readed);     
      utils_strtok_n_elems(&configs_buffer[APP_USD_CFG_TITLE_SIZE - 1], APP_USD_CFG_DELIMITER2, (int *)&configs_found, config_strings);
      
      /* Verificar número de configs */
      if((configs_found - 1) != DEVICE_CONFIGS_NUMBER){
        _valid_cfg_file = false;
        _current_state = APP_USD_GET_CSV_FILE;
        break;
      }     
      
      /* Percorrer as configs existentes */
      for(int i = 0 ; i < DEVICE_CONFIGS_NUMBER ; i++) {

        char config_string[APP_USD_CFG_ELEMENTS][30]= {"\0"};  
        utils_strtok_n_elems(config_strings[i], APP_USD_CFG_DELIMITER1, (int *)&configs_found, config_string);        

        if((i == 0) && (!strcmp(config_string[0], (uint8_t*)APP_USD_ID1))) {          
          _device_config.mode = atoi(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID2)) {
          _device_config.ecg = (bool)atoi(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID3)) {
          _device_config.ecg_lead_off = (bool)atoi(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID4)) {
          _device_config.ecg_gain = atoi(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID5)) {
          _device_config.ecg_accuracy = (bool)atoi(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID6)) {
          _device_config.pace = (bool)atoi(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID7)) {
          _device_config.resp_gain = atoi(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID8)) {
          _device_config.temp = (bool)atoi(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID9)) {
          _device_config.temp_low = utils_get_uint16_from_string(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID10)) {
          _device_config.temp_high = utils_get_uint16_from_string(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID11)) {
          _device_config.heart_rate_low = utils_get_uint16_from_string(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID12)) {
          _device_config.heart_rate_high = utils_get_uint16_from_string(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID13)) {
          _device_config.resp_rate_low = utils_get_uint16_from_string(config_string[1]);
        } else if(!strcmp(&config_string[0][1], (uint8_t*)APP_USD_ID14)) {
          _device_config.resp_rate_high = utils_get_uint16_from_string(config_string[1]);
        } else {
          _valid_cfg_file = false;
          _current_state = APP_USD_GET_CSV_FILE;
          break;
        }          
      }               
      
      (void)f_close(&file);      
      _valid_cfg_file = true;
      _current_state = APP_USD_GET_CSV_FILE;

      break;
    case APP_USD_GET_CSV_FILE:     
             
      /* Mount usd Card */
      if(!_app_usd_mount()) {
        break;
      }  
      
      /* Listar a diretoria */
      ff_result = f_opendir(&dir, "/");
      if (ff_result) { /* Listar diretoria deu erro */
        _valid_csv_file = false;
        _current_state = APP_USD_CONFIG_IDDLE;     
        break;
      }
      
      uint8_t file_name[APP_USD_DATAF_NAME_SIZE] = "\0";
      do {
        ff_result = f_readdir(&dir, &fno);
        if (ff_result != FR_OK) {   /* Leitura deu erro */
          _valid_csv_file = false;
          _current_state = APP_USD_CONFIG_IDDLE;
          break;
        }

        if (fno.fname[0]) {
          if (fno.fattrib & AM_DIR) {   
            debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
            debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_loop]    <DIR>   %s\r\n",(uint32_t)fno.fname);   
          } else {
            debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
            debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_loop] %9lu  %s\r\n", fno.fsize, (uint32_t)fno.fname);  
            
            memcpy(file_name, fno.fname, APP_USD_DATAF_NAME_DATA_SIZE - 1);
            if(!strcmp(file_name, APP_USD_DATAF_NAME)) {
              _data_file_count++;
            }
          }
        }
      }
      while (fno.fname[0]);      
      
      /* Não existe ficheiros */
      if(_data_file_count == 0) {
        _valid_csv_file = false;
        _current_state = APP_USD_CONFIG_IDDLE;
        break;
      }

      /* Existe ficheiros para validar */
      sprintf(file_name, APP_USD_CSV_FILE, _data_file_count - 1);

      /* O ficheiro APP_USD_CONFIG_FILE não é válido ou não existe */
      ff_result = f_open(&file, file_name, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
      if (ff_result != FR_OK) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_loop] Unable to open or create file: %s\r\n", file_name);
        break;
      }
      
      char patient_buffer[APP_USD_DATAF_PATIENT_INFO_SIZE] = "\0";
      int patient_fields_found = 0;
      char patient_string[APP_USD_PATIENT_ELEMENTS*2][30] = {"\0"};  
      uint8_t addr = 0;

      /* Offset no ponteiro do ficheiro para o ID do paciente */
      f_lseek(&file, APP_USD_DATAF_ID_OFFSET); 
      
      ff_result = f_read(&file, patient_buffer, APP_USD_DATAF_PATIENT_INFO_SIZE, &bytes_readed); 
      utils_strtok_n_elems(patient_buffer, APP_USD_PATIENT_DELIMITER1, &patient_fields_found, patient_string);
      
      /* Verificar número de fields do paciente */
      if((patient_fields_found - 1) != (APP_USD_PATIENT_ELEMENTS*2)){
        _valid_csv_file = false;
        _current_state = APP_USD_CONFIG_IDDLE;
        break;
      }

      /* Percorrer os campos do paciente existentes */
      for(int i = 0 ; i < APP_USD_PATIENT_ELEMENTS ; i++) {
        
        if(!strcmp(patient_string[addr], (uint8_t*)APP_USD_DATAF_ID_STR)) {          
          _patient_info.id = utils_get_uint32_from_string(patient_string[addr + 1]); /* It is uint32_t but the function is equivalent */
        } else if(!strcmp(&patient_string[addr][1], (uint8_t*)APP_USD_DATAF_NAME_STR)) {           
          memcpy(_patient_name, patient_string[addr + 1], APP_USD_DATAF_NAME_STR_SIZE); 
          _patient_info.name = _patient_name;
        } else if(!strcmp(&patient_string[addr][1], (uint8_t*)APP_USD_DATAF_AGE_STR)) {
          _patient_info.age = atoi(patient_string[addr + 1]);
        } else if(!strcmp(patient_string[addr], (uint8_t*)APP_USD_DATAF_GENDER_STR)) {
          memcpy(_patient_gender, patient_string[addr + 1], APP_USD_DATAF_NAME_STR_SIZE); 
          _patient_info.gender = _patient_gender;
        } else {
          _valid_cfg_file = false;
          _current_state = APP_USD_GET_CSV_FILE;
          break;
        }
        addr += 2;
      }
      
      (void)f_close(&file); 
      _valid_csv_file = true;
      _current_state = APP_USD_CONFIG_IDDLE;  

      break;    
    case APP_USD_CONFIG_IDDLE:

      if(_patient_checked && _valid_cfg_file) {                               /* Paciente verificado e CFG validado */
        /* Prevenção */
        if(!_app_usd_unmount_and_mount()) {
          break;
        }        
        _current_state = APP_USD_IDLE;
      }
              
      break;    
    case APP_USD_IDLE:
       
      /* Mounting precaution */
      if((_uploud_count == APP_USD_MEAS_UPLOAD_MOUNT_TH)) {        

        /* Prevenção */                                                
        (void)f_close(&file);  
        if(!_app_usd_unmount_and_mount()) {
          break;
        }      
        
        ff_result = f_open(&file, _actual_csv_file, FA_READ | FA_WRITE | FA_OPEN_APPEND);
        if (ff_result != FR_OK) {
          debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
          debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_loop] Unable to open or create file: %s\r\n", _actual_csv_file);
          break;
        }
        _uploud_count = 0;       
      }

      /* Verificar se é altura de enviar um pacote de measurements */
      if((_meas_buffer_index >= 1) && (_meas_buffer_index < (APP_USD_MEAS_BATCH_NUMBER + 1))) {        
        _app_usd_upload_meas();        
        _meas_buffer_index = 0;
        _uploud_count++;
        f_sync(&file);
      }     

      break; 
    case APP_USD_SAVE_MODE:
      /* In case of no operation or power save mode */
      if(!_save_mode_exec) {
        (void)f_close(&file);
        _app_usd_unmount();
        _app_usd_init_global_variables();
        _app_usd_power_off();
        _save_mode_exec = true;
      }

      if(!save_mode) {
        _save_mode_exec = false;
        _current_state = APP_USD_INIT;
      } 
    default: 
      break;
  }
}


/*
 * @brief Function to add device configuration to uSD APP_SDC_CONFIG_FILE
 *
 * @param[in] config          Struct with device configurations
 * @info                      Calling this function, will reset any device configuration
 *                            present in APP_USD_CONFIG_FILE 
 */
void app_usd_add_device_config(device_config *config) {

  uint32_t bytes_written = 0;
  FRESULT ff_result;
  uint16_t offset = 0;      
  uint8_t cfg_str[APP_USD_CFG_SIZE] = "\0";

  _device_config.mode = config->mode;
  _device_config.ecg = config->ecg;
  _device_config.ecg_lead_off = config->ecg_lead_off;
  _device_config.ecg_gain = config->ecg_gain;
  _device_config.ecg_accuracy = config->ecg_accuracy;
  _device_config.pace = config->pace;
  _device_config.resp_gain = config->resp_gain;
  _device_config.temp = config->temp;
  _device_config.temp_low = config->temp_low;
  _device_config.temp_high = config->temp_high;
  _device_config.heart_rate_low = config->heart_rate_low;
  _device_config.heart_rate_high = config->heart_rate_high;
  _device_config.resp_rate_low = config->resp_rate_low;
  _device_config.resp_rate_high = config->resp_rate_high;
  _device_config.mode = config->mode;

  /* Precaution */
  if(_file_status == APP_USD_CSV_OPEN || _file_status == APP_USD_CFG_OPEN) {
    (void)f_close(&file);
  }   

  /* Abrir ou criar ficheiro, caso exista será sobreposto */
  ff_result = f_open(&file, APP_USD_CONFIG_FILE, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
  if (ff_result != FR_OK) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_add_device_config] Unable to open or create file: %s\r\n", APP_USD_CONFIG_FILE);
    return;
  } 
  _file_status = APP_USD_CFG_OPEN;

  offset += sprintf(cfg_str, APP_USD_CFG_STR, APP_USD_ID1, _device_config.mode);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID2, (uint8_t)_device_config.ecg);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID3, (uint8_t)_device_config.ecg_lead_off);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID4, _device_config.ecg_gain);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID5, (uint8_t)_device_config.ecg_accuracy);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID6, (uint8_t)_device_config.pace);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID7, _device_config.resp_gain);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID8, (uint8_t)_device_config.temp);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID9, _device_config.temp_low);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID10, _device_config.temp_high);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID11, _device_config.heart_rate_low);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID12, _device_config.heart_rate_high);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID13, _device_config.resp_rate_low);
  offset += sprintf(&cfg_str[offset], APP_USD_CFG_STR, APP_USD_ID14, _device_config.resp_rate_high);

  uint8_t *device_cfg_send[] = {APP_USD_CFG_TITLE, cfg_str};
  uint8_t device_cfg_send_size[] = {APP_USD_CFG_TITLE_SIZE, strlen(cfg_str) + 1};

  for(int i = 0 ; i < ARRAY_SIZE(device_cfg_send) ; i++) {  
            
    ff_result = f_write(&file, device_cfg_send[i], device_cfg_send_size[i] - 1, (UINT *) &bytes_written);
    if (ff_result != FR_OK) {
      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_add_device_config] Write failed.\r\n");
      break;
    } 

  }
  (void)f_close(&file); 
  _file_status = APP_USD_CFG_CLOSED;

}


/*
 * @brief Function to get device configuration present in uSD APP_SDC_CONFIG_FILE
 *
 * @return    If device configuration valid return pointer to device_config, otherwise NULL
 */
device_config *app_usd_get_device_config(void) {

  if(_valid_cfg_file) {
    return &_device_config;
  }

  return NULL;
}


/*
 * @brief Function to add patient info to uSD APP_USD_CSV_FILE
 *
 * @param[in] patient         Struct with patient info
 * @param[in] overwrite       True if is to overwrite data measures in case of
 *                            the patient being the same
 * @info                      Calling this function, will reset any device configuration
 *                            present in APP_USD_CONFIG_FILE                 
 */
void app_usd_add_patient_info(patient_info *patient, bool overwrite) {
  
  uint8_t _patient_status = APP_USD_PATIENT_ADD;
  uint32_t bytes_written = 0;
  FRESULT ff_result;
  char file_name[APP_USD_DATAF_NAME_SIZE] = "\0";
  
  if((patient->id == _patient_info.id) && (!overwrite)) {           /* Paciente igual e manter dados das measures */
    _patient_status = APP_USD_PATIENT_NO_OVERWRITE;
  } else if((patient->id == _patient_info.id) && (overwrite)) {    /* Paciente igual e não manter dados das measures */
    _patient_status = APP_USD_PATIENT_OVERWRITE;      
  } else {                                                          /* Paciente não é igual e não manter dados das measures */
    _patient_status = APP_USD_PATIENT_ADD;
  
    _patient_info.id = patient->id;
    _patient_info.name = patient->name;
    _patient_info.gender = patient->gender;
    _patient_info.age = patient->age;
  }
  
  if((_patient_status == APP_USD_PATIENT_OVERWRITE) || (_patient_status == APP_USD_PATIENT_ADD)) {  

    if(_valid_csv_file) {                             /* Caso exista ficheiros para apagar */
      
      /* Apagar APP_USD_CSV_FILE existentes */
      for(int i = _data_file_count ; i >= 0 ; i--) {
        sprintf(file_name, APP_USD_CSV_FILE, i);
        f_unlink(file_name);
        _data_file_count = 0;
      }
    }
    
    /* Init file */
    sprintf(file_name, APP_USD_CSV_FILE, APP_USD_DATAF_NAME_INIT_NUMBER);
    _data_file_count++;
    
    /* Abrir ou criar ficheiro, caso exista será sobreposto */
    ff_result = f_open(&file, file_name, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
    if (ff_result != FR_OK) {
      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_add_patient_info] Unable to open or create file: %s\r\n", file_name);
      return;
    }
    _file_status = APP_USD_CSV_OPEN;


  } else {
    
    /* Precaution */
    if(_file_status == APP_USD_CSV_OPEN || _file_status == APP_USD_CFG_OPEN) {
      (void)f_close(&file);
    }   

    /* Add file */
    sprintf(file_name, APP_USD_CSV_FILE, _data_file_count);
    _data_file_count++;

    /* Abrir ou criar ficheiro, caso exista será sobreposto */
    ff_result = f_open(&file, file_name, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
    if (ff_result != FR_OK) {
      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_add_patient_info] Unable to open or create file: %s\r\n", file_name);
      return;
    }
    _file_status = APP_USD_CSV_OPEN;
  }  
   
  /* Formatar cabeçalho do ficheiro excel */
  uint8_t patient_info_str[APP_USD_DATAF_PATIENT_INFO_SIZE] = "\0";
  sprintf(patient_info_str, APP_USD_DATAF_PATIENT2, _patient_info.id, _patient_info.name, _patient_info.age, _patient_info.gender);
 
  static uint8_t *meas_title[] = MEASURES_CONTENT;
  uint8_t meas_title_send[APP_USD_MEAS_TITLE_SIZE] = "\0";
  uint8_t i_send = 0;
  uint8_t size = 0;
  
  /* Formatar cabeçalho de measurements existentes a enviar */
  for(int i = 0 ; i < DEVICE_CONFIGS_NUMBER; i++) {
    
    size = strlen(meas_title[i]);
    memcpy(&meas_title_send[i_send], meas_title[i], size);
    i_send += size;

    if(!strcmp(meas_title[i + 1], MEAS_TIMESTAMP)) {
      memcpy(&meas_title_send[i_send], ";;", 2*APP_USD_COL1_SIZE);     
      i_send += 2*APP_USD_COL1_SIZE;
    } else {
      memcpy(&meas_title_send[i_send], ";", 1*APP_USD_COL1_SIZE);
      i_send += APP_USD_COL1_SIZE;
    }

  }
  memcpy(&meas_title_send[i_send], "\n", 1*APP_USD_COL1_SIZE);  

  uint8_t *data_csv_send[] = {APP_USD_DATAF_TITLE, APP_USD_DATAF_DESCRIPTION1, APP_USD_DATAF_PATIENT1, patient_info_str, meas_title_send};      
  uint8_t data_csv_send_size[] = {APP_USD_DATAF_TITLE_SIZE, APP_USD_DATAF_DESCRIPTION1_SIZE, APP_USD_DATAF_PATIENT1_SIZE, APP_USD_DATAF_PATIENT_INFO_SIZE, ARRAY_SIZE(meas_title_send)};
                            
  for(int i = 0 ; i < ARRAY_SIZE(data_csv_send) ; i++) {  
            
    ff_result = f_write(&file, data_csv_send[i], data_csv_send_size[i] - 1, (UINT *) &bytes_written);
    if (ff_result != FR_OK) {
      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_add_patient_info] Write failed.\r\n");
      break;
    } 

  }
  memcpy(_actual_csv_file, file_name, APP_USD_DATAF_NAME_SIZE);
  (void)f_close(&file);
  _file_status = APP_USD_CSV_CLOSED;
  _patient_checked = true; 

}
  

/*
 * @brief Function to get patient info present in uSD APP_SDC_CSV_FILE
 *
 * @return    If patient info is valid return pointer to patient_info, otherwise NULL
 */
patient_info *app_usd_get_patient_info(void) {

  if(_valid_csv_file) {
    return &_patient_info;
  }

  return NULL;
}


/*
 * @brief Function to add measurement to internal buffer.
 *
 * @param[in] meas_buffer     Buffer with samples from RESP, ECG and Temperature.
 * @param[in] size            Buffer size in bytes.
 * @return    True if it was successful, false otherwise.
 */
bool app_usd_add_measurement(uint8_t *meas_buffer, uint8_t size) {

  if(_meas_buffer_index == APP_USD_MEAS_BATCH_NUMBER) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_add_measurement] Failed - internal buffer is full\r\n");

    return false;
  }

  if(size != _meas_size) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[app_usd_add_measurement] Failed - Samples size is different\r\n");

    return false;
  }

  /* Add measurment to the buffer */
  memcpy(_meas_buffer[_meas_buffer_index], meas_buffer, _meas_size); 

  _meas_buffer_index++;
}


/*
 * @brief Asks if the application is running.
 * @return    True if it is busy right now, false otherwise.
 */
bool app_usd_is_busy(void) {   
  if(_current_state == APP_USD_CONFIG_IDDLE || _current_state == APP_USD_IDLE || _current_state == APP_USD_NOP) {
    return false;
  } else {
    return true;
  }
}


/********************************** Private ************************************/
/* 
 * @brief Enables the power to the uSD circuit
 */
void _app_usd_power_on(void) {       
  nrf_gpio_pin_clear(APP_USD_PWR_EN);  
}


/* 
 * @brief Disables the power to the uSD circuit
 */
void _app_usd_power_off(void) {  
  /* Garantir que não existe operações nos pinos para não danificar o uSD */
  if(!app_sdc_busy_check()) {
    nrf_gpio_pin_set(APP_USD_PWR_EN);
  }
}


/* 
 * @brief Function to mount uSD Card.
 */
bool _app_usd_mount(void) {

  FRESULT ff_result = f_mount(&fs, "", 1);
  if (ff_result) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[_app_usd_mount] Mount failed. Try re-insert uSD\r\n");
    return false;
  }  
  return true;
}


/* 
 * @brief Function to unmount uSD Card.
 */
bool _app_usd_unmount(void) {

  FRESULT ff_result = f_mount(0, "", 0);
  if (ff_result) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[_app_usd_unmount] Unmount failed\r\n");
    return false;
  }  
  return true;
}

/* 
 * @brief Function to unmount and mount uSD Card.
 */
bool _app_usd_unmount_and_mount(void) {
  
  bool ret = false;
  
  ret = _app_usd_unmount();
  ret = _app_usd_mount();
  return ret;
}


/* 
 * @brief Function to initialize global variables.
 */
void _app_usd_init_global_variables() {
 
  //_upload_meas_th = upload_meas_th;
  _retries = 3;
  _meas_buffer_index = 0;
  _valid_cfg_file = false;
  _valid_csv_file = false;
  _patient_checked = false;
  _uploud_count = 0;
  _data_file_count = 0;
}

/* 
 * @brief Function to upload internal buffer measurements.
 */
void _app_usd_upload_meas(void) {

  uint32_t bytes_written = 0;
  int ff_result;
  
  static uint8_t *meas_title[] = MEASURES_CONTENT;
  static uint8_t meas[] = MEASURES_CONTENT_SIZE;  
  uint8_t var_8;
  uint16_t var_16;
  uint32_t var_32;
  uint64_t var_64;  
  
  /* Precaution */
  if(_file_status == APP_USD_CFG_OPEN) {
    (void)f_close(&file);
  } else if(_file_status == APP_USD_CSV_CLOSED) {
    ff_result = f_open(&file, _actual_csv_file, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
    if (ff_result != FR_OK) {
      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[_app_usd_upload_meas] Unable to open or create file: %s\r\n", _actual_csv_file);
      return;
    }
    f_lseek(&file, f_size(&file));
  }  
  
  /* Formatar cabeçalho de measurements existentes a enviar */
  for(int idx_meas = 0 ; idx_meas < APP_USD_MEAS_BATCH_NUMBER; idx_meas++) {
    
    uint8_t measures_send[APP_USD_MEAS_BATCH_SIZE] = "\0";
    uint8_t idx_send = 0;
    uint8_t idx_read = 0;

    for(int i = 0 ; i < DEVICE_MEASURES_NUMBER; i++) {
    
      switch(meas[i]) {

        case MEAS_1BYTE:
          sprintf(&measures_send[idx_send], "%d", &_meas_buffer[idx_meas][idx_read]);
          idx_send += MEAS_1BYTE;
          idx_read += MEAS_1BYTE;

          break;
        case MEAS_2BYTE:
          var_16 = utils_get_uint16_from_array(&_meas_buffer[idx_meas][idx_read]);
          sprintf(&measures_send[idx_send], "%d", var_16);
          idx_send += MEAS_2BYTE;
          idx_read += MEAS_2BYTE;

          break;
        case MEAS_4BYTE:
          var_32 = utils_get_uint32_from_array(&_meas_buffer[idx_meas][idx_read]);
          sprintf(&measures_send[idx_send], "%d", var_32);
          idx_send += MEAS_4BYTE;
          idx_read += MEAS_4BYTE;

          break;
        case MEAS_8BYTE:
          var_64 = utils_get_serial_from_array(&_meas_buffer[idx_meas][idx_read]);
          sprintf(&measures_send[idx_send], "%d", var_64);
          idx_send += MEAS_8BYTE;
          idx_read += MEAS_8BYTE;

          break;
        default:
          break;
      }

      if(!strcmp(meas_title[i + 1], MEAS_TIMESTAMP)) {
        memcpy(&measures_send[idx_send], ";;", 2*APP_USD_COL1_SIZE);     
        idx_send += 2*APP_USD_COL1_SIZE;
      } else {
        memcpy(&measures_send[idx_send], ";", APP_USD_COL1_SIZE);
        idx_send += APP_USD_COL1_SIZE;
      }

    }
    memcpy(&measures_send[idx_send], "\n", APP_USD_COL1_SIZE);

    ff_result = f_write(&file, measures_send, ARRAY_SIZE(measures_send) - 1, (UINT *) &bytes_written);
    if (ff_result != FR_OK) {
      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[_app_usd_upload_meas] Write failed.\r\n");    
    } 
  }  
}
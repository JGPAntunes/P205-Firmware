/*
* @file		ads129x.c
* @date		June 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the ads129x utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/

/* Interface */
#include "ads129x.h"

/* Utilities */
#include "spi_mngr.h"

/* Pheriperals */
#include "sense_library/periph/rtc.h"

/* Utils */
#include "sense_library/utils/debug.h"
#include "sense_library/utils/utils.h"

/* C standard library */
#include <math.h>

/* SDK */
#include "nrf_spi_mngr.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"

/********************************** Private ************************************/
/*
* Ponteiro para instância do SPI transaction manager
*/
static const nrf_spi_mngr_t *_p_nrf_spi_mngr;

/*
* Estrutura para config da instância SPI para os 4M de configuração
*/
static const nrf_drv_spi_config_t _spi_config_4M_t = SPI_MNGR_CONFIG2_T;

/*
* Variável de estado de inicialização da driver
*/
static bool _ads129x_state_init;

/*
* Variavél que armazena o número de tentativas da inicialização da driver
*/
static uint8_t _retries;

/*
* Variável estado do loop de inicialização
*/
static ads129x_init_states _current_state = ADS129X_INIT_CONFIG_ST; 

/*
* Variável do timestamp
*/
static uint64_t _current_timestamp;

/*
* Variável que regista o device a realizar inicialização
*/
static uint8_t _current_device;

/*
* Variável que regista se o power up já foi iniciado
*/
static bool _power_init;

/*
 * Buffer para leitura de dados de registos do ADS129x
 */
static uint8_t _id_read; 

/*
* Variável que define o device dos callbacks de configs
*/
static uint8_t _ads_device;

/*
* Variável do estado de standby do ADS1298 e ADS1296R
*/
static bool _ads_state_standby;

/*
* Variável do estado de start sample do ADS1298 e ADS1296R
*/
static bool _ads_state_start_sample;

/*
 * Data structure to save raw data from ADS129x
 */
static ads129x_data _ads129x_data;

/*
 * Data structure to return through the function ads129x_get_data()
 */
static uint8_t _ads129x_data_upload[ADS129X_DATA_SIZE];

/*
* Variável do estado de lead_off 
*/
static bool _lead_off;

/*
* Variável do ganho das PGAs do ECG
*/
static uint8_t _ecg_gain;

/*
* Variável do ganho da PGA do RESP
*/
static uint8_t _resp_gain;

/*
* Variável de estado do sinal de teste
*/
static bool _test_mode;

/* SPI callback functions list */
void ads129x_data_ready_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

/* Private functions list */
static bool _ads129x_cmd_getstatus(uint8_t next_status, uint8_t actual_status, uint8_t failed_status, uint64_t timeout);
bool _ads129x_configs(bool accuracy, bool lead_off, uint8_t pga_gain, uint8_t resp_gain, uint8_t ads129x, uint8_t test_mode);
bool _ads129x_read(void);
void _ads129x_get_voltage(void);
uint32_t _ads129x_utils_get_uint32_from_array_of_16bit(uint8_t *array);
bool _ads129x_standby(void);

/********************************** Public ************************************/
/*
 * @brief Function for initializing ADS1296R and ADS1298
 *
 * @retval                  It returns true when the inicialization reaches the 
 *                          end successfully or false while inicialization is still going.
 */
bool ads129x_init(void) {

  if(_retries == ADS129X_INIT_RETIRES) {
    return false;
  }
  
  _p_nrf_spi_mngr = spi_mngr_get_instance(SPI_MNGR_CONFIG1_INSTANCE);                 /* Obter ponteiro da manager SPI */
  
  /* Máquina de estados da inicialização do ADS1298 e ADS1296R */
  switch (_current_state) {
    case ADS129X_INIT_CONFIG_ST:
      /* Configuração dos pinos de ambos ADS129x */
      nrf_gpio_cfg(ADS129X_PACE_RST_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
      nrf_gpio_cfg(ADS129X_PWDN_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);
      nrf_gpio_cfg(ADS129X_8_CS_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);
      nrf_gpio_cfg(ADS129X_6R_CS_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);
    
      /* Configurar GPIO DRDY */
      nrf_drv_gpiote_in_config_t in_config = 
      {
        .sense = NRF_GPIOTE_POLARITY_HITOLO,                
        .pull = NRF_GPIO_PIN_PULLUP,                        
        .is_watcher = false,                                
        .hi_accuracy = false,                             
        .skip_gpio_setup = false 
      };

      /* Inicializar GPIO DRDY */
      uint32_t err_code = nrf_drv_gpiote_in_init(ADS129X_DRDY_PIN, &in_config, ads129x_data_ready_cb);

      /* Inicialização dos pinos de ambos ADS129x */
      ads129x_pace_rst(true);                                                       /* Resetar Latch do Pace Detection */
      ads129x_pace_rst(false);
      nrf_gpio_pin_clear(ADS129X_8_CS_PIN);                                         /* Manter os CS PINS a low no "Power-Up */
      nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);

      if(err_code != NRF_SUCCESS) {        
        _retries++;
      }      
       
      /* Init Global Variables */
      _ads129x_state_init = false;
      _current_device = ADS129X_8;
      _power_init = false;
      _ads_device = ADS129X_8;
      _ads_state_standby = false;
      _ads_state_start_sample = false;
      _lead_off = false;
      _ecg_gain = ADS129X_CHNSET_GAIN1;
      _resp_gain = ADS129X_CHNSET_GAIN1;
      _test_mode = false;
            
      debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds()); 
      debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_init] ADS129X_INIT_CONFIG_ST done \n");
      _current_state = ADS129X_POWER_INIT_ST;

      break;
    case ADS129X_POWER_INIT_ST: 
      
      /* Power é realizado em simultaneo para os 2 ADS129x */
      if(_current_device == ADS129X_8) {    
        
        if(!_power_init) {                                                          /* Caso já tenha sido inicializado */  
          nrf_gpio_pin_clear(ADS129X_PWDN_PIN);                                     /* Garantir que os devices estão desligados */
          nrf_delay_us(10);                                                         
          nrf_gpio_pin_set(ADS129X_PWDN_PIN);                                       /* Ativar os devices */
          _power_init = true;
          _current_timestamp = rtc_get_milliseconds();
        }         
        
        if((rtc_get_milliseconds() - _current_timestamp) > ADS129X_TPOR) {          /* Caso o device já tenha realizado power-up */
          nrf_gpio_pin_set(ADS129X_8_CS_PIN);                                           /* Manter o CS desligado (high) */
          nrf_gpio_pin_set(ADS129X_6R_CS_PIN);
          _power_init = false;
          debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds()); 
          debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_init] ADS129X_POWER_INIT_ST for ADS1298 done \n");
          _current_state = ADS129X_RESET_ST;   
        }         
      } else {      
        debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds()); 
        debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_init] ADS129X_POWER_INIT_ST for ADS1296R done \n");
        _current_state = ADS129X_RESET_ST; 
      }
                          
      break;
    case ADS129X_RESET_ST:   
      
      _current_state = ADS129X_RESET_ST; /* Dummy */
      /* Data a enviar para o ADS129x para reset do device */
      static uint8_t cmd_reset[] = {ADS129X_RESET_CMD};     

      /* Formalizar transferência para SPI manager */
      static nrf_spi_mngr_transfer_t const transfers = ADS129X_TRANSFER(cmd_reset, ARRAY_SIZE(cmd_reset), NULL, 0); 
      
      if(_ads_device == ADS129X_8) {
        nrf_gpio_pin_clear(ADS129X_8_CS_PIN);
      } else {
        nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);
      }        
      if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, &_spi_config_4M_t, &transfers, 1, NULL) != NRF_SUCCESS) {       
        debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
        debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_init] Reset try for ADS129%d error\n", _ads_device);
        _retries++; 
        _current_state = ADS129X_POWER_INIT_ST;
        return false;      
      }
      if(_ads_device == ADS129X_8) {
        nrf_gpio_pin_set(ADS129X_8_CS_PIN);
      } else {
        nrf_gpio_pin_set(ADS129X_6R_CS_PIN);
      }
      
      _current_timestamp = rtc_get_milliseconds();        
      _current_state = ADS129X_SDATAC_ST;
            
      break;
    case ADS129X_SDATAC_ST:
      
      /* Delay time for previous command */
      if((rtc_get_milliseconds() - _current_timestamp) > ADS129X_TRST) {

        /* Data a enviar para o ADS129x para Stop DATA Continous */ 
        static uint8_t cmd_stopdc[] = {ADS129X_SDATAC_CMD};                                   

        /* Formalizar transferência para SPI manager */
        static nrf_spi_mngr_transfer_t const transfers = ADS129X_TRANSFER(cmd_stopdc, ARRAY_SIZE(cmd_stopdc), NULL, 0); 
      
        if(_ads_device == ADS129X_8) {
          nrf_gpio_pin_clear(ADS129X_8_CS_PIN);
        } else {
          nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);
        }        
        if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, &_spi_config_4M_t, &transfers, 1, NULL) != NRF_SUCCESS) {       
          debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
          debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_init] Stop data continuous try for ADS129%d error\n", _ads_device);
          _retries++; 
          _current_state = ADS129X_POWER_INIT_ST;
          return false;      
        }
        if(_ads_device == ADS129X_8) {
          nrf_gpio_pin_set(ADS129X_8_CS_PIN);
        } else {
          nrf_gpio_pin_set(ADS129X_6R_CS_PIN);
        }
        
        _current_timestamp = rtc_get_milliseconds();
        _current_state = ADS129X_VOLTAGE_REF_ST;
      }      

      break;
    case ADS129X_VOLTAGE_REF_ST:
      
      /* Delay time for previous command */
      if((rtc_get_milliseconds() - _current_timestamp) > ADS129X_TSTOP) {

        /* Data a enviar para o ADS129x para ativar a tensão de referência interna */
        static uint8_t cmd_vref_on[] = {ADS129X_WREG_CMD_1BYTE_CONCAT(ADS129X_REG_CONFIG3), 
                                        ADS129X_WREG_CMD_2BYTE_CONCAT(0),
                                        ADS129X_CONFIG3_INIT};                                   

        /* Formalizar transferência para SPI manager */
        static nrf_spi_mngr_transfer_t const transfers = ADS129X_TRANSFER(cmd_vref_on, ARRAY_SIZE(cmd_vref_on), NULL, 0); 
      
        if(_ads_device == ADS129X_8) {
          nrf_gpio_pin_clear(ADS129X_8_CS_PIN);
        } else {
          nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);
        }        
        if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, &_spi_config_4M_t, &transfers, 1, NULL) != NRF_SUCCESS) {       
          debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
          debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_init] Vref activate try for ADS129%d error\n", _ads_device);
          _retries++; 
          _current_state = ADS129X_POWER_INIT_ST;
          return false;      
        }
        if(_ads_device == ADS129X_8) {
          nrf_gpio_pin_set(ADS129X_8_CS_PIN);
        } else {
          nrf_gpio_pin_set(ADS129X_6R_CS_PIN);
        }
        
        _current_state = ADS129X_GET_ID_ST;
      }

      break;
    case ADS129X_GET_ID_ST:

      /* Delay time for previous command */
      if((rtc_get_milliseconds() - _current_timestamp) > ADS129X_TVREF) {

        /* Data a enviar para o ADS129x para obter o ID do respetivo ADS129x */
        static uint8_t cmd_get_id[] = {ADS129X_RREG_CMD_1BYTE_CONCAT(ADS129X_REG_ID),       
                                       ADS129X_RREG_CMD_2BYTE_CONCAT(0)};                                   

        /* Formalizar transferência para SPI manager */
        static nrf_spi_mngr_transfer_t const transfers[] = {
          ADS129X_TRANSFER(cmd_get_id, ARRAY_SIZE(cmd_get_id), &_id_read, ADS129X_REG_BUFFER_SIZE),
          ADS129X_TRANSFER(NULL, 0, &_id_read, ADS129X_REG_BUFFER_SIZE),
        };
         
        if(_ads_device == ADS129X_8) {
          nrf_gpio_pin_clear(ADS129X_8_CS_PIN);
        } else {
          nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);
        }        
        if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, &_spi_config_4M_t, transfers, 2, NULL) != NRF_SUCCESS) {       
          debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
          debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_init] ID read try for ADS129%d error\n", _ads_device);
          _retries++; 
          _current_state = ADS129X_POWER_INIT_ST;
          return false;      
        }
        if(_ads_device == ADS129X_8) {
          nrf_gpio_pin_set(ADS129X_8_CS_PIN);
        } else {
          nrf_gpio_pin_set(ADS129X_6R_CS_PIN);
        }
        
        /* Verificar ID */
        if(_ads_device == ADS129X_8 && _id_read == ADS129X_8_ID_VALUE) {
          debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds()); 
          debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_init] ID of ADS1298 verified \n");
          _ads_device = ADS129X_6R;
          _current_state = ADS129X_POWER_INIT_ST; 
        } else if(_ads_device == ADS129X_6R && _id_read == ADS129X_6R_ID_VALUE) {
          debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds()); 
          debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_init] ID of ADS1296R verified \n");
          _ads129x_state_init = true;
          return true;
        } else {
          _retries++;
          _current_state = ADS129X_POWER_INIT_ST;
        }
      }
        
      break;
    default:
      break;

  }    
  return false;
             
}


/*
 * @brief Function to set ADS1296R and ADS1298 registers values for specific config
 *
 * @param[in] accuracy      True if high resolution, otherwise low power resolution
 * @param[in] lead_off      True if lead-off on, otherwise off 
 * @param[in] ecg_gain      ECG gain for all the input analog channels, and must be 1,2,3,4,6,8 or 12
 * @param[in] resp_gain     RESP gain analog input, and must be 1,2,3,4,6,8 or 12
 * @param[in] test_mode     It has the following possibilities: - In the last 3 modes only accuracy and ads129x is used
                              ADS129X_NORMAL - ECG and RESP are normaly configured
                              ADS129X_SUPPLY - measure internal supply 
                              ADS129X_TEMP   - measure internal temperature
                              ADS129X_SIGNAL - measure internal test signal                              
 * @retval                  Returns true if the comand is schedule successfull and valid gains, or false otherwise
 */
bool ads129x_configs(bool accuracy, bool lead_off, uint8_t ecg_gain, uint8_t resp_gain, uint8_t test_mode) {
  
  /* Update internal variables */
  if(!test_mode) {
    _lead_off = lead_off;
    _ecg_gain = ecg_gain;
    _resp_gain = resp_gain;
  }
  _test_mode = test_mode;
  
  if(_ads129x_state_init) {                                                         /* Prevenção de execução caso a ads129x_init_loop não esteja finalizada */
    
    /* Configuração do ADS1298 com base nos parametros de entrada */
    if(!_ads129x_configs(accuracy, lead_off, ecg_gain, resp_gain, ADS129X_8, test_mode)) {
      return false;
    }

    /* Configuração do ADS1296R com base nos parametros de entrada */
    if(!_ads129x_configs(accuracy, lead_off, ecg_gain, resp_gain, ADS129X_6R, test_mode)) {
      return false;
    }   
  } else {
    return false;
  }
  
  /* Colocar ADS129x em standby */
  if(!_ads129x_standby()) {
    return false;
  }
  
  _ads_state_standby = true;
  return true;

}


/*
 * @brief Function for ADS1296R and ADS1298 start sampling through the analog channels, has to be called in LOOP
 *
 * @retval                  Returns true if the START comand is schedule successfull, or false otherwise
 */
uint8_t ads129x_start_sample(void) {
  
  uint64_t timestamp_wakeup;

  /* Prevenção de execução caso a ads129x_configs não esteja finalizada */
  if(_ads_state_standby) {  
      
    /* Data a enviar para o ADS129x para WAKEPUP */
    static uint16_t cmd_wakeup[] = {ADS129X_WAKEUP_CMD};

    /* Formalizar transferência para SPI manager */
    static nrf_spi_mngr_transfer_t const transfers = ADS129X_TRANSFER(cmd_wakeup, ARRAY_SIZE(cmd_wakeup), NULL, 0);

    /* WAKEPUP ADS1298 */
    nrf_gpio_pin_clear(ADS129X_8_CS_PIN);
    if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers, 1, NULL) != NRF_SUCCESS) {       
      debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_start_sample] Wakeup try for ADS1298 error\n");
      return ADS129X_FAULT;      
    }
    nrf_gpio_pin_set(ADS129X_8_CS_PIN);

    /* WAKEPUP ADS1296R */
    nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);
    if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers, 1, NULL) != NRF_SUCCESS) {     
      debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_start_sample] Wakeup try for ADS1296R error\n");
      return ADS129X_FAULT;      
    }
    nrf_gpio_pin_set(ADS129X_6R_CS_PIN);   
    timestamp_wakeup = rtc_get_milliseconds();
  }

  /* Proceder para o start sampling após o wakeup dos ADS129x */
  if(((rtc_get_milliseconds() - timestamp_wakeup) > ADS129X_TWAKEUP)) {         /* Prevenção de execução caso WAKEUP não esteja finalizada */
    return ADS129X_IDDLE;
  }
  
  _ads_state_standby = false;                                                     /* Os ADS129x já estão em modo normal */

  /* Data a enviar para o ADS129x para start sampling dos ADS129x */
  static uint16_t cmd_start_sampling[] = {ADS129X_START_CMD};

  /* Formalizar transferência para SPI manager */
  static nrf_spi_mngr_transfer_t const transfers = ADS129X_TRANSFER(cmd_start_sampling, ARRAY_SIZE(cmd_start_sampling), NULL, 0);

  /* Enable sampling in ADS1298 */
  nrf_gpio_pin_clear(ADS129X_8_CS_PIN);
  if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers, 1, NULL) != NRF_SUCCESS) {       
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_start_sample] Start sample try for ADS1298 error\n");
    return ADS129X_FAULT;      
  }
  nrf_gpio_pin_set(ADS129X_8_CS_PIN);

  /* Enable sampling in ADS1296R */
  nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);
  if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers, 1, NULL) != NRF_SUCCESS) {     
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_start_sample] Start sample try for ADS1296R error\n");
    return ADS129X_FAULT;      
  }
  nrf_gpio_pin_set(ADS129X_6R_CS_PIN);
      
  _ads_state_start_sample = true;
  return ADS129X_TRUE;
}


/*
 * @brief Function to set ADS1296R and ADS1298 in data continous mode, allowing the sample to be read
 *
 * @retval                  Returns true if the RDATAC comand is schedule successfull, or false otherwise
 */
bool ads129x_start_datac(void) {

  if(!_ads_state_start_sample) {                          /* Prevenção de execução caso a ads129x_start_sample_loop ou a ads129x_user_configs não estejam finalizadas */
    return false;
  }
    
  /* Data a enviar para o ADS129x para start sampling dos ADS129x */
  static uint16_t cmd_rdatac[] = {ADS129X_RDATAC_CMD};

  /* Formalizar transferência para SPI manager */
  static nrf_spi_mngr_transfer_t const transfers = ADS129X_TRANSFER(cmd_rdatac, ARRAY_SIZE(cmd_rdatac), NULL, 0);

  /* Start data continuous in ADS1298 */
  nrf_gpio_pin_clear(ADS129X_8_CS_PIN);
  if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers, 1, NULL) != NRF_SUCCESS) {       
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_start_datac] Start data continuous try for ADS1298 error\n");
    return false;      
  }
  nrf_gpio_pin_set(ADS129X_8_CS_PIN);

  /* Start data continuous in ADS1296R */
  nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);
  if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers, 1, NULL) != NRF_SUCCESS) {     
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_start_datac] Start data continuous try for ADS1296R error\n");
    return false;      
  }
  nrf_gpio_pin_set(ADS129X_6R_CS_PIN);
    
  nrf_drv_gpiote_in_event_enable(ADS129X_DRDY_PIN, true);                         /* Permitir interrupções para Data Ready pin */  
  return true;   

}


/* 
 * @brief Function that returns the data structure 
 *
 * @return                Returns the data structure pointer
 */
uint8_t *ads129x_get_data(void) {
  return _ads129x_data_upload;
}


/*
 * @brief Function to stop data continous mode of ADS1296R and ADS1298 in data continous mode        
 *
 * @retval                  Returns true if the SDATAC comand is schedule successfull, or false otherwise
 * @note                    1º The ADS129x will continue sampling but won't put the data in the output register
 *                          2º Must be called prior to ads129x_user_configs if RDATAC enabled
 *                          3º Might be useful to call time to time prior to ads129x_start_sample() to ensure 
 *                          synchronization (re-start)
 */
bool ads129x_stop_datac(void) {

  /* Data a enviar para o ADS129x para start sampling dos ADS129x */
  static uint16_t cmd_sdatac[] = {ADS129X_SDATAC_CMD};

  /* Formalizar transferência para SPI manager */
  static nrf_spi_mngr_transfer_t const transfers = ADS129X_TRANSFER(cmd_sdatac, ARRAY_SIZE(cmd_sdatac), NULL, 0);

  /* Stop data continuous in ADS1298 */
  nrf_gpio_pin_clear(ADS129X_8_CS_PIN);
  if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers, 1, NULL) != NRF_SUCCESS) {       
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_stop_datac] Stop data continuous try for ADS1298 error\n");
    return false;      
  }
  nrf_gpio_pin_set(ADS129X_8_CS_PIN);

  /* Stop data continuous in ADS1296R */
  nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);
  if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers, 1, NULL) != NRF_SUCCESS) {     
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_stop_datac] Stop data continuous try for ADS1296R error\n");
    return false;      
  }
  nrf_gpio_pin_set(ADS129X_6R_CS_PIN);
  
  nrf_drv_gpiote_in_event_disable(ADS129X_DRDY_PIN);                              /* Desativar interrupções para Data Ready pin */  
  
  
  /* Colocar ADS129x em standby */
  if(!_ads129x_standby()) {
    return false;
  }
  
  _ads_state_standby = true;
  return true; 

}


/*
 * @brief Function to set ADS1296R and ADS1298 registers values for specific config given
 *        by the user
 *
 * @param[in] accuracy      True if high resolution, otherwise low power resolution
 * @param[in] lead_off      True if lead-off on, otherwise off 
 * @param[in] pga_gain      ECG gain for all the input analog channels, and must be 1,2,3,4,6,8 or 12
 * @param[in] resp_gain     RESP gain analog input, and must be 1,2,3,4,6,8 or 12
 * @param[in] test_mode     It has the following possibilities: - In the last 3 modes only accuracy and ads129x is used
                              ADS129X_NORMAL - ECG and RESP are normaly configured
                              ADS129X_SUPPLY - measure internal supply 
                              ADS129X_TEMP   - measure internal temperature
                              ADS129X_SIGNAL - measure internal test signal                              
 * @retval                  Returns true if the comand is schedule successfull and valid gains, or false otherwise
 */
bool ads129x_user_configs(bool accuracy, bool lead_off, uint8_t ecg_gain, uint8_t resp_gain, uint8_t test_mode) {

  /* Update internal variables */
  if(!test_mode) {
    _lead_off = lead_off;
    _ecg_gain = ecg_gain;
    _resp_gain = resp_gain;
  }
  _test_mode = test_mode;

  /* Prevenção de execução caso a ads129x_stop_datac não esteja finalizada */
  if(_ads_state_standby) { 
    
    /* Configuração do ADS1298 com base nos parametros de entrada */
    if(!_ads129x_configs(accuracy, lead_off, ecg_gain, resp_gain, ADS129X_8, test_mode)) {
      return false;
    }

    /* Configuração do ADS1296R com base nos parametros de entrada */
    if(!_ads129x_configs(accuracy, lead_off, ecg_gain, resp_gain, ADS129X_6R, test_mode)) {
      return false;
    } 
    return true;
  }
  return false;

}


/*
 * @brief Function to set or clear reset pin of pace latch
 *
 * @param[in] state         Define output pin state
 *                          state = true -> output = 1
 *                          state = false -> output = 0
 */
void ads129x_pace_rst(bool state) {

  if (state) {
    nrf_gpio_pin_set(ADS129X_PACE_RST_PIN);
  } else {
    nrf_gpio_pin_clear(ADS129X_PACE_RST_PIN);
  }

}


/*
 * @brief Function to set or clear reset pin of pace latch
 *
 * @retval                  Returns true if successful
 */
void ads129x_uninit(void) {
  
  nrf_drv_gpiote_in_event_disable(ADS129X_DRDY_PIN);                              /* Desativar interrupções para Data Ready pin */
  nrf_gpio_pin_clear(ADS129X_PWDN_PIN);
  _current_state = ADS129X_INIT_CONFIG_ST;
  
}

/********************************** Calbacks ************************************/
/*
 * @brief Function to handle pin DRDY change from low to high callback for data ready detection
 *
 * @param[in] pin           Pin that triggered this event.
 * @param[in] action        Action that led to triggering this event.
 */
void ads129x_data_ready_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

  if(pin != ADS129X_DRDY_PIN || action != NRF_GPIOTE_POLARITY_HITOLO) {
    return;
  }

  uint8_t read_reg_status = _ads129x_read();                                         /* Read Output register from both ADS129x */
  if(!read_reg_status) {
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_data_ready_cb] Erro no acesso de leitura do ADS129x \r\n");     
  }

}


/********************************** Private ************************************/
/*
 * @brief Function to set ADS129x registers values for specific config
 *
 * @param[in] accuracy      True if high resolution, otherwise low power resolution
 * @param[in] lead_off      True if lead-off on, otherwise off 
 * @param[in] pga_gain      PGA gain for all the input analog channels, and must be 1,2,3,4,6,8 or 12
 * @param[in] resp_gain     RESP gain analog input, and must be 1,2,3,4,6,8 or 12
 * @param[in] ads129x       Device ADS129X_6R or ADS129X_8 to configure
 * @param[in] test_mode     It has the following possibilities: - In the last 3 modes only accuracy, resp_gain and ads129x is used
                              ADS129X_NORMAL - ECG and RESP are normaly configured
                              ADS129X_SUPPLY - measure internal supply and RESP
                              ADS129X_TEMP   - measure internal temperature and RESP
                              ADS129X_SIGNAL - measure internal test signal and RESP                              
 * @retval                  Returns true if the comand is schedule successfull and valid gains, or false otherwise
 */
bool _ads129x_configs(bool accuracy, bool lead_off, uint8_t pga_gain, uint8_t resp_gain, uint8_t ads129x, uint8_t test_mode) {

  uint8_t pga_gain_valid = 0;
  uint8_t resp_gain_valid = 0;
  bool lead_off_valid = false;
  static uint8_t config[ADS129X_CONFIGS_SIZE];
  
  if(test_mode == ADS129X_NORMAL) {
    /* Escolher o ganho da PGA do ECG */
    switch(pga_gain){
      case ADS129X_GAIN1:
        pga_gain_valid = ADS129X_CHNSET_GAIN1;       
        break;
      case ADS129X_GAIN2:
        pga_gain_valid = ADS129X_CHNSET_GAIN2;       
        break;
      case ADS129X_GAIN3:
        pga_gain_valid = ADS129X_CHNSET_GAIN3;       
        break;
      case ADS129X_GAIN4:
        pga_gain_valid = ADS129X_CHNSET_GAIN4;       
        break;
      case ADS129X_GAIN6:
        pga_gain_valid = ADS129X_CHNSET_GAIN6;       
        break;
      case ADS129X_GAIN8:
        pga_gain_valid = ADS129X_CHNSET_GAIN8;       
        break;
      case ADS129X_GAIN12:
        pga_gain_valid = ADS129X_CHNSET_GAIN12;       
        break;
      default:  /* Invalid gain value */
        return false;
    }    
    lead_off_valid = lead_off;

  } else if(test_mode == ADS129X_SUPPLY) {
    pga_gain_valid = ADS129X_CHNSET_SUPPLY_MEAS;
  } else if(test_mode == ADS129X_TEMP) {
    pga_gain_valid = ADS129X_CHNSET_TEMP_MEAS;
  } else {
    pga_gain_valid = ADS129X_CHNSET_TEST_SIG_MEAS;
  }

  /* Escolher o ganho da PGA do RESP no caso do ADS1296R */
  if(ads129x == ADS129X_6R) {
    switch(resp_gain){
      case ADS129X_GAIN1:
        resp_gain_valid = ADS129X_CHNSET_GAIN1;       
        break;
      case ADS129X_GAIN2:
        resp_gain_valid = ADS129X_CHNSET_GAIN2;       
        break;
      case ADS129X_GAIN3:
        resp_gain_valid = ADS129X_CHNSET_GAIN3;       
        break;
      case ADS129X_GAIN4:
        resp_gain_valid = ADS129X_CHNSET_GAIN4;       
        break;
      case ADS129X_GAIN6:
        resp_gain_valid = ADS129X_CHNSET_GAIN6;       
        break;
      case ADS129X_GAIN8:
        resp_gain_valid = ADS129X_CHNSET_GAIN8;       
        break;
      case ADS129X_GAIN12:
        resp_gain_valid = ADS129X_CHNSET_GAIN12;       
        break;
      default:  /* Invalid gain value */
        return false;
    }
  } 
    
  if(ads129x == ADS129X_8) {                                                                                      /* Send configs for ADS1298 */

    /* Configs a enviar */    
    uint8_t config_tmp[ADS129X_CONFIGS_SIZE] = ADS129X_8_CONFIG_CMD(accuracy, lead_off_valid, pga_gain_valid);
    memcpy(config, config_tmp, ADS129X_CONFIGS_SIZE);

    /* Formalizar transferência para SPI manager */
    static nrf_spi_mngr_transfer_t const transfers = ADS129X_TRANSFER(config, ARRAY_SIZE(config), NULL, 0);

    nrf_gpio_pin_clear(ADS129X_8_CS_PIN);                                                                         /* Assert Chip Select */
    if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, &_spi_config_4M_t, &transfers, 1, NULL) != NRF_SUCCESS) {
      debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[_ads129x_configs] Configuration try for ADS1298 error \n");
      return false;
    }
    nrf_gpio_pin_set(ADS129X_8_CS_PIN);                                                                           /* Assert Chip Select */
  
  } else {                                                                                                        /* Send configs for ADS1296R */

    /* Configs a enviar */    
    uint8_t config_tmp[ADS129X_CONFIGS_SIZE] = ADS129X_6R_CONFIG_CMD(accuracy, lead_off_valid, pga_gain_valid, resp_gain_valid);
    memcpy(config, config_tmp, ADS129X_CONFIGS_SIZE);

    /* Formalizar transferência para SPI manager */
    static nrf_spi_mngr_transfer_t const transfers = ADS129X_TRANSFER(config, ARRAY_SIZE(config), NULL, 0);

    nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);                                                                         /* Assert Chip Select */
    if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, &_spi_config_4M_t, &transfers, 1, NULL) != NRF_SUCCESS) {
      debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[_ads129x_configs] Configuration try for ADS1296R error \n");
      return false;
    }
    nrf_gpio_pin_set(ADS129X_6R_CS_PIN);                                                                           /* Assert Chip Select */

  }

  return true; 

}


/*
 * @brief Function to schedule Status Word read data for ADS1298 and ADS1296R
 *
 * @retval                  Returns true if the read schedule is successfull, or false otherwise
 */
bool _ads129x_read(void) {

  static uint8_t buffer[ADS129X_SW_BUFFER_SIZE] = "\0";
 
  /* Formalizar transferências para SPI manager */
  static nrf_spi_mngr_transfer_t const transfers[] = 
  {
    ADS129X_TRANSFER(NULL, 0, _ads129x_data.ads1298_sw_buffer, ADS129X_SW_BUFFER_SIZE),   /* ADS1298 */
    ADS129X_TRANSFER(NULL, 0, _ads129x_data.ads1296r_sw_buffer, ADS129X_SW_BUFFER_SIZE),  /* ADS1296R */
  };  
  
  /* Get data from ADS1298 */
  nrf_gpio_pin_clear(ADS129X_8_CS_PIN);
  if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers[0], 1, NULL) != NRF_SUCCESS) {       
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[_ads129x_read] Get data from ADS1298 error\n");
    return false;      
  }
  nrf_gpio_pin_set(ADS129X_8_CS_PIN);

  /* Get data from ADS1296R */
  nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);
  if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers[1], 1, NULL) != NRF_SUCCESS) {     
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[_ads129x_read] Get data from ADS1296R error\n");
    return false;      
  }
  nrf_gpio_pin_set(ADS129X_6R_CS_PIN);
  
  _ads129x_data.timestamp = rtc_get_milliseconds();

  _ads129x_get_voltage();
  return true;

}

/*
 * @brief Function to convert data from ADS1298 and ADS1296R to voltage
 *
 */
void _ads129x_get_voltage(void) {
  
  uint32_t value_raw;
  float lsb;
  float voltage;
  uint16_t addr = ADS129X_DATA_IDX;      
  uint8_t actual_gain;
  uint32_t uvoltage;

  /* Get Updated Gain */
  if(_test_mode) {
    actual_gain = ADS129X_CHNSET_TEST_SIG_GAIN;
  } else {
    actual_gain = _ecg_gain;
  }
    
  /* Update Timestamp */
  utils_save_uint64_t_to_array_keep_endianness(&_ads129x_data_upload[0], _ads129x_data.timestamp);

  /* Verify Identifier */
  if(!ADS129X_VERIFY_DATA_ID(_ads129x_data.ads1298_sw_buffer[0]) || !ADS129X_VERIFY_DATA_ID(_ads129x_data.ads1296r_sw_buffer[0])) {
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[_ads129x_get_voltage] Measure ID invalid \n");
  }
  
  if(_lead_off) {
    /* Update LOFF status */
    _ads129x_data_upload[ADS129X_STAT_IDX] = IS_SET(_ads129x_data.ads1298_sw_buffer[1], ADS129X_LA_BIT);
    _ads129x_data_upload[ADS129X_STAT_IDX + 1] = IS_SET(_ads129x_data.ads1298_sw_buffer[2], ADS129X_RA_BIT);
    _ads129x_data_upload[ADS129X_STAT_IDX + 2] = IS_SET(_ads129x_data.ads1298_sw_buffer[1], ADS129X_LL_BIT);
    _ads129x_data_upload[ADS129X_STAT_IDX + 3] = IS_SET(_ads129x_data.ads1298_sw_buffer[1], ADS129X_V1_BIT);
    _ads129x_data_upload[ADS129X_STAT_IDX + 4] = IS_SET(_ads129x_data.ads1298_sw_buffer[0], ADS129X_V2_BIT);
    _ads129x_data_upload[ADS129X_STAT_IDX + 5] = IS_SET(_ads129x_data.ads1298_sw_buffer[1], ADS129X_V3_BIT);
    _ads129x_data_upload[ADS129X_STAT_IDX + 6] = IS_SET(_ads129x_data.ads1298_sw_buffer[1], ADS129X_V4_BIT);
    _ads129x_data_upload[ADS129X_STAT_IDX + 7] = IS_SET(_ads129x_data.ads1298_sw_buffer[1], ADS129X_V5_BIT); 
    _ads129x_data_upload[ADS129X_STAT_IDX + 8] = IS_SET(_ads129x_data.ads1298_sw_buffer[0], ADS129X_V6_BIT);
  } else {
    memset(&_ads129x_data_upload[ADS129X_STAT_IDX], 0, ADS129X_ELECT_NUMB - 1);
  }
  
  /* Update Pace Detection Status based on the hardware jumper */
  if(ADS129X_PACE_DEVICE == ADS129X_8) {
    _ads129x_data_upload[ADS129X_STAT_IDX + 9] = IS_SET(_ads129x_data.ads1298_sw_buffer[2], ADS129X_PACE_BIT);
  } else {
    _ads129x_data_upload[ADS129X_STAT_IDX + 9] = IS_SET(_ads129x_data.ads1296r_sw_buffer[2], ADS129X_PACE_BIT);
  }      
  
  /* Convert data of ADS1298 */
  for(int idx = ADS129X_CH1_IDX ; idx < ADS129X_8_SW_BUFFER_SIZE + 1 ; idx+=ADS129X_3BYTE) {

    value_raw = _ads129x_utils_get_uint32_from_array_of_16bit(&_ads129x_data.ads1298_sw_buffer[idx]);
    lsb = ((2*(float)ADS129X_REF)/(float)actual_gain)/(float)(pow(2, ADS129X_RES) - 1);
    
    /* Verificar se a medida é negativa */
    if(ADS129X_VERIFY_POL(value_raw)) {
      voltage = (value_raw - pow(2, ADS129X_RES)) * lsb;
    } else {
      voltage = value_raw * lsb;
    }
    
    #if ADS129X_MEAS_DEBUG
    /* Converts charge voltage to string */
    char voltage_string[10];
    memset(voltage_string, '\0', 10);
    utils_ftoa(voltage_string, (float)voltage * 1000, 0.00000001);

    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[_ads129x_get_voltage] Measure converted ADS1298: %s\n", voltage_string);
    #endif

    /* Upload to array */    
    uvoltage = voltage*ADS129x_INTEGER_CONVERT;
    utils_save_uint32_t_to_array(&_ads129x_data_upload[addr], uvoltage);    
    addr += ADS129X_4BYTE;
  }

  /* Convert data of ADS1296R */
  for(int idx = ADS129X_CH1_IDX ; idx < ADS129X_6R_SW_BUFFER_SIZE ; idx+=ADS129X_3BYTE) {

    value_raw = _ads129x_utils_get_uint32_from_array_of_16bit(&_ads129x_data.ads1296r_sw_buffer[idx]);
    
    if(addr == ADS129X_CH1_IDX) {
      lsb = ((2*(float)ADS129X_REF)/(float)_resp_gain)/(float)(pow(2, ADS129X_RES) - 1);
    } else {
      lsb = ((2*(float)ADS129X_REF)/(float)actual_gain)/(float)(pow(2, ADS129X_RES) - 1);
    }   
    
    /* Verificar se a medida é negativa */
    if(ADS129X_VERIFY_POL(value_raw)) {
      voltage = (value_raw - pow(2, ADS129X_RES)) * lsb;
    } else {
      voltage = value_raw * lsb;
    }
    
    #if ADS129X_MEAS_DEBUG
    /* Converts charge voltage to string */
    char voltage_string[10];
    memset(voltage_string, '\0', 10);
    utils_ftoa(voltage_string, (float)voltage * 1000, 0.00000001);

    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[_ads129x_get_voltage] Measure converted ADS1296R: %s\n", voltage_string);
    #endif
    
    /* Upload to array */
    uvoltage = voltage*ADS129x_INTEGER_CONVERT;
    utils_save_uint32_t_to_array(&_ads129x_data_upload[addr], uvoltage);    
    addr += ADS129X_4BYTE;
  }

}

bool _ads129x_standby(void) {

  /* Data a enviar para o ADS129x para entrar em modo STANDBY */
  static uint16_t cmd_standby[] = {ADS129X_STANDBY_CMD};
  
  /* Formalizar transferência para SPI manager */
  static nrf_spi_mngr_transfer_t const transfers = ADS129X_TRANSFER(cmd_standby, ARRAY_SIZE(cmd_standby), NULL, 0);     
 
  /* Put ADS1298 in Standby */
  nrf_gpio_pin_clear(ADS129X_8_CS_PIN);
  if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers, 1, NULL) != NRF_SUCCESS) {       
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_configs] Standby try for ADS1298 error\n");
    return false;      
  }
  nrf_gpio_pin_set(ADS129X_8_CS_PIN);
  
  /* Put ADS1296R in Standby */
  nrf_gpio_pin_clear(ADS129X_6R_CS_PIN);
  if(nrf_spi_mngr_perform(_p_nrf_spi_mngr, NULL, &transfers, 1, NULL) != NRF_SUCCESS) {     
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[ads129x_configs] Standby try for ADS1296R error\n");
    return false;      
  }
  nrf_gpio_pin_set(ADS129X_6R_CS_PIN);
  return true;

}

/**
 * Gets number from a string.
 * @param[in] array String that contains the number.
 * @return The 32-bit unsigned integer.
 */
uint32_t _ads129x_utils_get_uint32_from_array_of_16bit(uint8_t *array) {
  uint32_t u32 = 0;

  u32 |= ((uint32_t) array[0]) << 16;
  u32 |= ((uint32_t) array[1]) << 8;
  u32 |= ((uint32_t) array[2]);

  return u32;
}
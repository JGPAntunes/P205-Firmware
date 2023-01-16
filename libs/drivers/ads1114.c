/*
* @file		ads1114.c
* @date		April 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the ads1114 adc utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/

/* Interface */
#include "ads1114.h"

/* Utilities */
#include "twi_mngr.h"

/* Pheriperals */
#include "sense_library/periph/rtc.h"

/* SDK */
#include "nrf_twi_mngr.h"
#include "nrf_drv_gpiote.h"

/* Debug - libs */
#include "sense_library/utils/debug.h"

/* Utils */
#include "sense_library/utils/utils.h"

/* C standard library */
#include "math.h"

/********************************** Private ************************************/
/*
* Ponteiro para instância do TWI transaction manager
*/
static const nrf_twi_mngr_t *_p_nrf_twi_mngr;

/*
* Variável de estado do loop de inicialização
*/
static ads1114_init_states _current_state = ADS1114_INIT_ST;

/*
* Variável de estado do device que indica se o device está no barramento
*/
static bool _ads_device_detect = false;

/*
* Variável de estado de config do ads1114
*/
static bool _ads_state_config = false;

/*
* Variável de estado de execução da função ads1114_init()
*/
static bool _ads_exec_init = false;

/*
 * Instanciação do registo para leitura do ADS1114
 */
static uint8_t _ads_reg_addr_read = ADS1114_REG_READ;    

/*
 * Buffer para leitura de dados do ADS1114
 */
static uint8_t _m_buffer[ADS1114_BUFFER_SIZE];   

/*
 * Full scale range.
 */
static float _ads_fsr = ADS1114_FSR;

/*
 * Data structure to return through the function ads1114_get_data()
 */
static ads1114_data _ads1114_data;

/*
 * Variables to test ADS1114
 */
#if ADS1114_TEST 
static uint32_t _samples_count;
static float _ads1114_voltage_array [ADS1114_MAX_VOLTAGE_SAMPLES];
static float _ads1114_voltage_min = ADS1114_VCOMP_MIN;
static float _ads1114_voltage_max = ADS1114_VCOMP_MAX;
#endif

/* TWI callback functions list */
static void ads1114_readValue_cb(ret_code_t result, void *p_user_data);
static void ads1114_detectDevice_cb(ret_code_t result, void *p_user_data);
static void ads1114_config_cb(ret_code_t result, void *p_user_data);
void ads1114_data_ready_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

/* Private functions list */
bool ads1114_read(void);
float ads1114_voltage_value(int16_t adc_result);
bool ads1114_config(void);
#if ADS1114_TEST
void ads1114_voltageTest(float voltage);
#endif


/*
 * @brief Function for initializing ADS1114
 */
bool ads1114_init(void) {

  uint32_t err_code = NRF_SUCCESS;
  nrf_drv_gpiote_in_config_t in_config;

  if(_ads_exec_init) {                                                      /* Caso a função já tenha sido executada com sucesso */
    return true;
  } 
  
  switch(_current_state) {                                                  /* Configuração Inicial da driver */
    case ADS1114_INIT_ST:

      _p_nrf_twi_mngr = twi_mngr_get_instance(TWI_MNGR_CONFIG1_INSTANCE);   /* Obter ponteiro da manager TWI */

      /* Configurar GPIO DRY */
      nrf_drv_gpiote_in_config_t in_config = 
      {
        .sense = NRF_GPIOTE_POLARITY_HITOLO,                
        .pull = NRF_GPIO_PIN_PULLUP,                        
        .is_watcher = false,                                
        .hi_accuracy = false,                             
        .skip_gpio_setup = false 
      };

      /* Inicializar GPIO DRY */
      err_code = nrf_drv_gpiote_in_init(ADS1114_DRY_PIN, &in_config, ads1114_data_ready_cb);
      if(err_code == 0) {
        _current_state = ADS1114_DETECT_DEVICE_ST;
      } 
      break;
    case ADS1114_DETECT_DEVICE_ST:
      
      err_code = NRF_SUCCESS;                                               /* Dummy */

      /* Data a enviar para o ADS1114 para deteção do device */
      static uint8_t config[] = {ADS1114_REG_CONFIG, ADS1114_REG_CONFIG_MSB_D, ADS1114_REG_CONFIG_LSB_D};
     
      static nrf_twi_mngr_transfer_t const transfers[] =                    /* Formalizar escrita para TWI manager */
      {
        ADS_WRITE(config, (sizeof(config) / sizeof(config[0])))
      };
  
      /* Transação a ser realizada */
      static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
      {
        .callback            = ads1114_detectDevice_cb,                     /* Função chamada após a transação ser realizada */
        .p_user_data         = NULL,                                        
        .p_transfers         = transfers,                                   /* Ponteiro para as transferências a fazer */
        .number_of_transfers = sizeof(transfers) / sizeof(transfers[0])     /* Número de transferências */
      };

      /* Efetivar transação */
      err_code = nrf_twi_mngr_schedule(_p_nrf_twi_mngr, &transaction);
      if(err_code == 0) {
        _current_state = ADS1114_CONFIG_ST;
      } 
      break;
    case ADS1114_CONFIG_ST:

      if(_ads_device_detect) {                                              /* Caso a transferência tenha tido sucesso */
        if(ads1114_config()) {                                              /* Realizar configuração dos registos Lo_thresh e Hi_thresh */
          nrf_drv_gpiote_in_event_enable(ADS1114_DRY_PIN, true);            /* Permitir interrupções para Data Ready pin */
          _ads_exec_init = true;
          return true;
        }        
      }
      break;
    default:
      return false;
  }
  return false;    
           
}


/*
 * @brief   Function to config registers to enable data conversion ready pin function
 *
 * @retval  NRFX_SUCCESS                   If write transaction has been successfully.
 * @retval  NRF_ERROR_NO_MEM               If the queue is full.
 */
bool ads1114_config(void) {

  uint32_t err_code = NRF_SUCCESS;
  
  /* Data a enviar para o ADS1114 */
  static uint8_t config[] = {ADS1114_REG_LO_TH, ADS1114_REG_LO_TH_MSB, ADS1114_REG_LO_TH_LSB};
  static uint8_t config1[] = {ADS1114_REG_HI_TH, ADS1114_REG_HI_TH_MSB, ADS1114_REG_HI_TH_LSB};

  /* 
  * these structures have to be "static"
  * they cannot be placed on stack since the transaction is scheduled and these structures
  * most likely will be referred after this function returns
  */    
  static nrf_twi_mngr_transfer_t const transfers[] =                    /* Formalizar escrita para TWI manager */
  {
    ADS_WRITE(config, (sizeof(config) / sizeof(config[0]))),
    ADS_WRITE(config1, (sizeof(config) / sizeof(config[0]))),
  };

  /* Transação a ser realizada */
  static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
  {
    .callback            = ads1114_config_cb,                           /* Função chamada após a transação ser realizada */
    .p_user_data         = NULL,                                        
    .p_transfers         = transfers,                                   /* Ponteiro para as transferências a fazer */
    .number_of_transfers = sizeof(transfers) / sizeof(transfers[0])     /* Número de transferências */
  };

  /* Efetivar transação */
  err_code = nrf_twi_mngr_schedule(_p_nrf_twi_mngr, &transaction); 

  if(err_code == 0) {        
    return true;
  }
  return false;  

}


/*
 * @brief   Function to request a measurement to the ADS1114 (write the config to the ADS1114 register)
 *
 * @retval  NRFX_SUCCESS                   If write transaction has been successfully.
 * @retval  NRF_ERROR_NO_MEM               If the queue is full.
 */
bool ads1114_readValue(void) {

  uint32_t err_code = NRF_SUCCESS;

  /* Data a enviar para o ADS1114 */
  static uint8_t config[] = {ADS1114_REG_CONFIG, ADS1114_REG_CONFIG_MSB, ADS1114_REG_CONFIG_LSB};

  /* 
  * these structures have to be "static"
  * they cannot be placed on stack since the transaction is scheduled and these structures
  * most likely will be referred after this function returns
  */    
  static nrf_twi_mngr_transfer_t const transfers[] =                    /* Formalizar escrita para TWI manager */
  {
    ADS_WRITE(config, (sizeof(config) / sizeof(config[0])))
  };

  /* Transação a ser realizada */
  static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
  {
    .callback            = NULL,                                        /* Função chamada após a transação ser realizada */
    .p_user_data         = NULL,                                        
    .p_transfers         = transfers,                                   /* Ponteiro para as transferências a fazer */
    .number_of_transfers = sizeof(transfers) / sizeof(transfers[0])     /* Número de transferências */
  };

  /* Efetivar transação */
  err_code = nrf_twi_mngr_schedule(_p_nrf_twi_mngr, &transaction); 

  if(err_code == 0) {
    return true;
  }
  return false; 

}


/*
 * @brief Function to schedule register read from ADS1114
 */
bool ads1114_read(void) {

  uint32_t err_code = NRF_SUCCESS;

  /* 
  * these structures have to be "static"
  * they cannot be placed on stack since the transaction is scheduled and these structures
  * most likely will be referred after this function returns
  */
  static nrf_twi_mngr_transfer_t const transfers[] =
  {
    ADS_READ(&_ads_reg_addr_read, _m_buffer, ADS1114_REG_SIZE)
  };

  /* Transação a ser realizada */
  static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
  {
    .callback            = ads1114_readValue_cb,                        /* Função chamada após a transação ser realizada */
    .p_user_data         = NULL,                                    
    .p_transfers         = transfers,                                   /* Ponteiro para as transferências a fazer */
    .number_of_transfers = sizeof(transfers) / sizeof(transfers[0])     /* Número de transferências */
  };

  /* Efetivar transação */
  err_code = nrf_twi_mngr_schedule(_p_nrf_twi_mngr, &transaction); 

  if(err_code == 0) {
    return true;
  }
  return false;

}


/*
 * @brief Function to handle TWI register read callback 
 *
 * @param[in] result        Result of operation (NRF_SUCCESS on success, otherwise a relevant error code).
 * @param[in] p_user_data   Pointer to user data defined in transaction descriptor.
 */
static void ads1114_readValue_cb(ret_code_t result, void *p_user_data) {
   
  if (result != NRF_SUCCESS) {
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());  
    debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"Read registers from ADS1114 error: %d\n", result);    
    return;
  }
  _ads1114_data.timestamp = rtc_get_milliseconds();                                /* Obter timestamp da medida */

  uint16_t adc_result = ADS1114_GETVALUE(_m_buffer[0], _m_buffer[1]);
  _ads1114_data.voltage = ads1114_voltage_value (adc_result);                      /* Obter valor de tensão total */

  /* Converts charge voltage to string */
  char voltage_string[10];
  memset(voltage_string, '\0', 10);
  utils_ftoa(voltage_string, _ads1114_data.voltage, 0.0001);

  //debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  //debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[ads1114_readValue_cb] data collected; voltage = %sV \n", voltage_string);
          

  #if ADS1114_TEST
    ads1114_voltageTest(_ads1114_data.voltage);   
    _samples_count ++;
  #endif    
        
}


/*
 * @brief Function to check if driver initialization was successful 
 *
 * @return      status             Returns the status of driver
 *                                  true  - sucess
 *                                  false - failure
 */
bool ads1114_get_init_status(void) {

  return _ads_state_config;

}


/* 
 * @brief Function that returns the data structure 
 *
 * @return    voltage     Returns the data structure pointer
 */
ads1114_data *ads1114_get_data(void) {

  return &_ads1114_data;

}


/*
 * @brief Function for converting register value from ADS1114 to voltage
 *
 * @param[in]   adc_result          ADC value returned by ADC buffer.
 * @return      voltage             Returns the voltage requested
 */
float ads1114_voltage_value(int16_t adc_result) {

  return  (adc_result*(_ads_fsr / pow(2, ADS1114_RES-1)));            
}


/*
 * @brief Function to handle TWI register write callback for device detection
 *
 * @param[in] result        Result of operation (NRF_SUCCESS on success, otherwise a relevant error code).
 * @param[in] p_user_data   Pointer to user data defined in transaction descriptor.
 */
static void ads1114_detectDevice_cb(ret_code_t result, void *p_user_data) {

  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());  
  if (result != NRF_SUCCESS) {     
    debug_printf_string(DEBUG_LEVEL_1, (uint8_t*)"ADS1114 not present: %d\n", result);
    return;
  }
  _ads_device_detect = true;
  debug_print_string(DEBUG_LEVEL_1, (uint8_t*)"ADS1114 is present: \n");
  
          
}


/*
 * @brief Function to handle TWI register write callback for device config
 *
 * @param[in] result        Result of operation (NRF_SUCCESS on success, otherwise a relevant error code).
 * @param[in] p_user_data   Pointer to user data defined in transaction descriptor.
 */
static void ads1114_config_cb(ret_code_t result, void *p_user_data) {
  
  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());  
  if (result != NRF_SUCCESS) {
    debug_printf_string(DEBUG_LEVEL_1, (uint8_t*)"ADS1114 config not done: %d\n", result);    
    return;
  }
  _ads_state_config = true;
  debug_print_string(DEBUG_LEVEL_1, (uint8_t*)"ADS1114 config successful\n");
            
}

 
/*
 * @brief Function to handle pin DRY change from low to high callback for data ready detection
 *
 * @param[in] pin           Pin that triggered this event.
 * @param[in] action        Action that led to triggering this event.
 */
void ads1114_data_ready_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

  if(pin != ADS1114_DRY_PIN || action != NRF_GPIOTE_POLARITY_HITOLO) {
    return;
  }

  uint8_t read_reg_status = ads1114_read();
  if(!read_reg_status) {
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"Erro no acesso ao registo de leitura do ADS1114\n");     
  }

}


/*
 * @brief Function to test the ADC with a resquest of a number of samples
 *        defined by ADS1114_MAX_VOLTAGE_SAMPLES
 *
 * @param[in]   voltage          Voltage value returned from adcInt_voltageGet(...)
 */
#if ADS1114_TEST
void ads1114_voltageTest(float voltage) {

  /* Se o número de samples for mais que o desejado */
  if(_samples_count > ADS1114_MAX_VOLTAGE_SAMPLES) {
    return;
  }

  /* Albbergar a soma das samples */
  float voltage_sum = 0; 

  /* Guardar amostras de tensão no array */
  _ads1114_voltage_array[_samples_count] = voltage;
    
  /* Se o número de samples não for o desejado retornar */
  if(_samples_count < ADS1114_MAX_VOLTAGE_SAMPLES) {
    return;
  }

  /* Retirar o máximo e mínimo e a média das samples */
  for(int i = 0 ; i < ADS1114_MAX_VOLTAGE_SAMPLES ; i++) {

    if(_ads1114_voltage_array[i] > _ads1114_voltage_max) {                /* Retirar o máximo */
      _ads1114_voltage_max = _ads1114_voltage_array[i]; 
    }

    if(_ads1114_voltage_array[i] < _ads1114_voltage_min) {                /* Retirar o minimo */
      _ads1114_voltage_min = _ads1114_voltage_array[i];
    }

    voltage_sum += _ads1114_voltage_array[i];                            /* Soma das samples */

  }
    
  float average = voltage_sum/ADS1114_MAX_VOLTAGE_SAMPLES;              /* Retirar a média */

  /* Impressão dos resultados da medição */
  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"Resultados da medicao:\n");  
  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"  Tensao maxima: %.2f [mV] \n", _ads1114_voltage_max*1000); 
  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"  Tensao media:  %.2f [mV] \n", average*1000); 
  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"  Tensao minima: %.2f [mV] \n", _ads1114_voltage_min*1000); 
  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"Excel:\n");
  
  

  
  for(int i = 0 ; i < ADS1114_MAX_VOLTAGE_SAMPLES ; i++) {

    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"%.2f\n", _ads1114_voltage_array[i]*1000);
  }
  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"Done\n");
}
#endif


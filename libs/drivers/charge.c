/*
* @file		charge.c
* @date		April 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the Wireless and Wire charge utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/
/* Interface */
#include "charge.h"

/* Pheriperals */
#include "sense_library/periph/rtc.h"

/* SDK */
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

/* Utils */
#include "sense_library/utils/debug.h"

/* Gama libs */
#include "sense_library/protocol-gama/gama_fw/include/gama_generic.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_measurement.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_definitions.h"

/* Measurements buffer */
#include "sense_library/sensoroid/measurements_v2_manager.h"

/* Utils */
#include "sense_library/utils/debug.h"
#include "sense_library/utils/externs.h"
#include "sense_library/utils/utils.h"


/********************************** Private ************************************/
#if MICROCONTROLER_1
static app_extram_get_init_status_callback_def _app_extram_get_init_status_callback;
static app_extram_get_battery_data_callback_def _app_extram_get_battery_data_callback;
#endif /* MICROCONTROLER_1 */

#if MICROCONTROLER_2
/*
* Variável de estado da inicialização do driver de carregamento
*/
static bool _init_state = false;

/*
* Variável de timestamp de carregamento por fio
*/
static uint64_t _charge_timestamp;

/*
* Variável de timestamp para primeira medida
*/
static uint64_t _first_timestamp;

/*
* Variável de timestamp para segunda medida
*/
static uint64_t _second_timestamp;

/*
* Variável que conta quantas vezes o sinal blink (fault) foi observado
*/
static uint8_t _wave_count;

/*
* Variável que conta as execuções de código na função charge_wire_cb()
*/
static uint8_t _charge_wire_count;

/*
* Variável de estado do pin de wire status
*/
static uint8_t _wire_status;

/*
* Variável de estado do pin de wireless status
*/
static uint8_t _wireless_status;

/**
 *  Stores how many queue add attempts.
 */
static uint8_t _queue_add_attempts = 0;
#endif /* MICROCONTROLER_2 */

/*
* Variável estado do loop de sampling
*/
static charge_sampling_states _current_state;

/*
 * Buffer the measurement timestamp. 
 */
static uint64_t _last_timestamp;

/*
 * App loop the sampling timestamp. 
 */
static uint64_t _sampling_timestamp;

/*
 * Stores the sampling rate given by the upper layer.
 */
static uint64_t _sampling_rate;

/*
 * Data structure to return through the function charge_get_info()
 */
static charge_data _charge_data;

#if MICROCONTROLER_2
/* Charge callback functions list */
void charge_wire_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

/* Private functions list */
bool charge_add_measurement(charge_data* charge_data);
uint8_t charge_get_percentage_internal(float voltage);
bool charge_get_wireless_status(void);
uint8_t charge_get_wire_status(void);
#endif 

/********************************** Public ************************************/
/*
 * @brief Function for initializing Charge Driver
 *
 * @param[in] sampling_rate Charge data sampling rate.
 * @param[in] app_extram_get_init_status_callback Pointer to charge app init status function.
 * @param[in] app_extram_get_battery_data_callback Pointer to battery data function.
 */
bool charge_init(
  uint64_t                                        sampling_rate,
  app_extram_get_init_status_callback_def         app_extram_get_init_status_callback,
  app_extram_get_battery_data_callback_def        app_extram_get_battery_data_callback) {
  
  _sampling_rate = sampling_rate;
  
  #if MICROCONTROLER_1
  _current_state = CHARGE_POWER_ON;
  _app_extram_get_init_status_callback = app_extram_get_init_status_callback;
  _app_extram_get_battery_data_callback = app_extram_get_battery_data_callback;
  #endif 
 
  #if MICROCONTROLER_2
  _wire_status = CHARGE_WIRE_OFF;
  _wireless_status = CHARGE_WIRE_OFF;
  _current_state = CHARGE_POWERED_OFF;
  
  /* Configuração do PIN de status do carregamento wireless */
  nrf_gpio_cfg_input(CHARGE_WIRELESS_PIN, NRF_GPIO_PIN_PULLUP);

  /* Configuração do PIN de status do carregamento por fio */
  nrf_drv_gpiote_in_config_t in_config = 
  {
    .sense = NRF_GPIOTE_POLARITY_LOTOHI,                
    .pull = NRF_GPIO_PIN_PULLUP,                        
    .is_watcher = false,                                
    .hi_accuracy = false,                             
    .skip_gpio_setup = false 
  };

  /* Inicializar GPIOE do carregamento por fio */
  if(nrf_drv_gpiote_in_init(CHARGE_WIRE_PIN, &in_config, charge_wire_cb) != 0) {
    return false;
  }
  
  nrf_drv_gpiote_in_event_enable(CHARGE_WIRE_PIN, true);                        /* Permitir interrupções para Wire Status */
  #endif 

  return true;      
    
}


/********************************** Code bellow adapted from Energy.c ************************************/
/**
 * Charge application loop. 
 */
void charge_loop(void) {
  
  #if MICROCONTROLER_2
  /* Data sampling state machine */
  switch (_current_state) {
    case CHARGE_POWERED_OFF:

      if(saadc_init()) {        
        _current_state = CHARGE_POWER_ON;

        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[charge_loop] Powered up sucessfully\r\n");
      }

      break;   
    case CHARGE_POWER_ON: 

      if(saadc_get_init_status()) {
        _current_state = CHARGE_IDLE;     
        
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[charge_loop] Going to Idle\r\n");
      } 

      break;    
    case CHARGE_IDLE: 

      if((rtc_get_milliseconds() - _last_timestamp) > _sampling_rate) {
        if(saadc_sample()) {
          _sampling_timestamp = rtc_get_milliseconds();
          debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
          debug_print_string(DEBUG_LEVEL_0, "[charge_loop] Read started\n");
          _current_state = CHARGE_WAITING_DATA;
        } else {
          break;
        }  
      }

      break;            

    case CHARGE_WAITING_DATA:

      /* Adicionar à estrutura a percentagem da bateria */
      if(saadc_get_data()->timestamp > _last_timestamp) {
        _charge_data.voltage = (saadc_get_data()->voltage)*CHARGE_VOLTAGE_DIVIDER_VALUE;
        _charge_data.battery_percentage = charge_get_percentage_internal(_charge_data.voltage);
        
        _charge_data.last_read_timestamp = saadc_get_data()->timestamp;
        _last_timestamp = saadc_get_data()->timestamp;

        /* Converts charge voltage to string */
        char voltage_string[10];
        memset(voltage_string, '\0', 10);
        utils_ftoa(voltage_string, _charge_data.voltage, 0.001);
        
        _current_state = CHARGE_UPLOAD_DATA;

        debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
        debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[charge_loop] data collected; voltage = %sV, percentage = %d%%\n", voltage_string, _charge_data.battery_percentage);
      }

      break;
    case CHARGE_UPLOAD_DATA:
      
      /* Adicionar measurement da tensão da bateria */
      if(charge_add_measurement(&_charge_data)) {
        _current_state = CHARGE_IDLE;
      }  
          
      break;
    default: 
      break;    
  }

  /* Timeout to get data verification */
  if((_current_state == CHARGE_WAITING_DATA) && (rtc_get_milliseconds() - _sampling_timestamp > CHARGE_DATA_SAMPLING_TIMEOUT) ) {
    _current_state = CHARGE_IDLE; //CHARGE_NOP;     

    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[charge_loop] charge sample timeout expired. retry\r\n");
  }
  #endif

  #if MICROCONTROLER_1
  /* Data sampling state machine */
  switch (_current_state) {
      
    case CHARGE_POWER_ON: 

      if(_app_extram_get_init_status_callback()) {
        _current_state = CHARGE_IDLE;     
        
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[charge_loop] Requested data\r\n");
      } 

      break;    
    case CHARGE_IDLE: 

      if((rtc_get_milliseconds() - _last_timestamp) > _sampling_rate) {        
        debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_0, "[charge_loop] Read started\n");
        _current_state = CHARGE_WAITING_DATA;         
      }

      break;            

    case CHARGE_WAITING_DATA:

      /* Adicionar à estrutura a percentagem da bateria */
      if(_app_extram_get_battery_data_callback()->last_read_timestamp > _last_timestamp) {
        _charge_data.battery_mode = _app_extram_get_battery_data_callback()->battery_mode;
        
        _charge_data.last_read_timestamp = _app_extram_get_battery_data_callback()->last_read_timestamp;
        _last_timestamp = _app_extram_get_battery_data_callback()->last_read_timestamp;
       
        _current_state = CHARGE_IDLE;

        debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());
        if(_charge_data.battery_mode) {
          debug_printf_string(DEBUG_LEVEL_2, (uint8_t*)"[charge_loop] data collected; battery mode: save mode\n");
        } else {
          debug_printf_string(DEBUG_LEVEL_2, (uint8_t*)"[charge_loop] data collected; battery mode: normal mode\n");
        }     
      }

      break;
    
    default: 
      break;    
  }

  /* Timeout to get data verification */
  if((rtc_get_milliseconds() - _last_timestamp > CHARGE_DATA_SAMPLING_TIMEOUT) ) {
    _current_state = CHARGE_NOP;     

    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[CHARGE_loop] charge timeout expired. retry\r\n");
  }
  #endif  
}


/**
 * Gets battery information.
 * @return Battery information structure.
 */
charge_data* charge_get_info(void) {
      
  return &_charge_data;
  
}


/**
 * Asks if the application is running.
 * @return True if it is busy right now, false otherwise.
 */
bool charge_is_busy(void) {

  if(_current_state == CHARGE_IDLE || _current_state == CHARGE_NOP) {    
    return false;
  } else {
    return true;
  }
}


#if MICROCONTROLER_2
/**
 * Adds measurement to measurements manager.
 * @param[in] charge_data Charge information.
 */
bool charge_add_measurement(charge_data* charge_data) {
  float voltage = charge_data->voltage;

  gama_measure_format_v2_fields_t measurement_fields;
  measurement_fields.measure_type = GAMA_MEASURE_BATTERY_VOLTAGE;
  measurement_fields.sensor = ENERGY_SENSOR_SEQUENCE;
  measurement_fields.config_byte = MEASURE_VALUE_FLOAT_TYPE;
  measurement_fields.val = &voltage;

  /* Convert floats to string to be printed out. Note
   * that printed value might be slightly different from
   * the real float values in the last decimal case */
  char voltage_string[10];
  memset(voltage_string, '\0', 10);
  utils_ftoa(voltage_string, voltage, 0.001);

  if(measurements_v2_manager_add_measurement(&measurement_fields)) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_1,(uint8_t*)"[charge_add_measurement] measurement added; details: value = %sV\n", voltage_string);
    
    _queue_add_attempts = 0;
    return true;
  } else {
    if(_queue_add_attempts >= GAMA_QUEUE_ADD_MAX_ATTEMPTS) {
      _queue_add_attempts = 0;
      return true;
    }
    else {
      _queue_add_attempts++;
      return false;
    }
  }
}


/**
 * Triggers new charge collection.
 * @param[in] upload_info Tells if data needs to be uploaded.
 * @return True if collection was triggered, false otherwise.
 */
bool charge_collect_data(bool upload_info) {
  /* As charge module is critical and it's always powered on we want to block parallel reads.
   * Just wait a bit to be free again */
  if(saadc_sample() && (_current_state == CHARGE_IDLE)) {
    _current_state = CHARGE_WAITING_DATA;
    _sampling_timestamp = rtc_get_milliseconds();
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, "[charge_collect_data] Collect data request\n"); 

    return true;
  } else {
    return false;
  }
}  


/**
 * Gets batteries' rechargeable  condition.
 * @return True if battery is recheargable, false otherwise.
 */
bool charge_get_rechargeable_type(void) {

  return true;  
}


/**
 * Gets external power information.
 * @return  CHARGE_MODE_OFF (0) if charge is off
 *          CHARGE_MODE_WIRE_FAULT (1) if wire charge is in fault condition
 *          CHARGE_MODE_WIRELESS_ON (2) if wireless charge on
 *          CHARGE_MODE_WIRE_ON(3) if wire charge on          
 */
uint8_t charge_get_external_power_status(void) {

  if(charge_get_wireless_status()) {
    return CHARGE_MODE_WIRELESS_ON;
  } else if(charge_get_wire_status() == CHARGE_WIRE_ON) {
    return CHARGE_MODE_WIRE_ON;
  } else if(charge_get_wire_status() == CHARGE_WIRE_FAULT) {
    return CHARGE_MODE_WIRE_FAULT;
  } else {
    return CHARGE_MODE_OFF;
  }
  
} 


/**
 * Reads battery information.
 * @return Battery information struct.
 */
struct battery_information charge_get_battery_type_information(void) {
  
  struct battery_information battery_data = {     
    .enter_battery_saving_voltage = LITHIUM_ENTER_BATTERY_SAVING_TYPE_1,
    .leaving_battery_saving_warmup_voltage = LITHIUM_LEAVING_BATTERY_SAVING_WARMUP_TYPE_1,
    .leaving_battery_saving_voltage = LITHIUM_LEAVING_BATTERY_SAVING_TYPE_1 
  };

  return battery_data;
}


/**
 * Function to get voltage percentage of last measurement
 * @return Battery percentage.
 */
uint8_t charge_get_percentage(void) {
  uint8_t percentage = 0;

  return charge_get_percentage_internal(_charge_data.voltage);
  
}
#endif 


/* ******************************** Calbacks ********************************** */
#if MICROCONTROLER_2
/*
 * @brief Function to handle pin Wire Status change from low to high callback for fault confition wire charge.   
 *
 * @param[in] pin           Pin that triggered this event.
 * @param[in] action        Action that led to triggering this event.
 */
void charge_wire_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

  _charge_timestamp = rtc_get_milliseconds(); 

  if(_wire_status == CHARGE_WIRE_FAULT) {                                       /* Caso já esteja confimada a fault condition */
    return;
  }                           
  
  if(pin != CHARGE_WIRE_PIN || action != NRF_GPIOTE_POLARITY_LOTOHI) {
    return;
  }
  
  if(_charge_wire_count % 2 == 0) {                                             /* Caso a contagem seja par */
    _first_timestamp = _charge_timestamp;                                       /* Armazena o timestamp da primeira ascenção do pin de status */

  } else {                                                                      /* Caso a contagem seja ímpar */
    _second_timestamp = _charge_timestamp;                                      /* Armazena o timestamp da segunda ascenção do pin de status */
    
    uint64_t diff = _second_timestamp - _first_timestamp;                       /* Retirar o período da onda blink */

    if(diff > CHARGE_BLINK_BOTTOM_TIME_TH && diff < CHARGE_BLINK_UP_TIME_TH) {  /* Caso o período esteja compreendido no intervalo de segurança */
      _wave_count ++;                                                           /* Contabilizar como onda de blink */
  
      if(_wave_count == CHARGE_BLINK_NUMBER_TH) {                               /* Apenas assumir que a onda está presente após CHARGE_BLINK_NUMBER_TH vezes (segurança apenas) */
        _wire_status = CHARGE_WIRE_FAULT;                                       /* Oficializar Fault Condition */
        _charge_wire_count = 0;
        _wave_count = 0;
      }
    }
  }
  
}
#endif 

/* ******************************** Private ********************************** */
#if MICROCONTROLER_2
/*
 * @brief   Function to get the status of the Wireless Charging Chip
 *                          
 * @retval  If wireless charging is on the function will return true, otherwise false
 */
bool charge_get_wireless_status(void) {  

  /* Retirar o valor do estado do pino de estado do carregamento wireless */
  if(nrf_gpio_pin_read(CHARGE_WIRELESS_PIN) == 0) {
    _wireless_status = true;
    return true;
  } else {
    _wireless_status = false;
    return false;
  }

}


/*
 * @brief   Function to get the status of the Wire Charging Chip
 *                          
 * @retval  If wire charging is on the function will return 1, if not and in case of charge fault 255 (0xFF)
 */
uint8_t charge_get_wire_status(void) {

  uint64_t diff = rtc_get_milliseconds() - _charge_timestamp;

  if((_wire_status == CHARGE_WIRE_FAULT) && diff < CHARGE_BLINK_TIME_TH) {      /* Está em Fault Condition e Wave Blink ainda está a ser detetada */
    return _wire_status;
  } else {                                                                      /* É necessário ver se está a carregar ou não */
    /* Retirar o valor do estado do pino de estado do carregamento por fio */
    if(nrf_drv_gpiote_in_is_set(CHARGE_WIRE_PIN) == 0) {                        /* Carregamento ON */
      _wire_status = CHARGE_WIRE_ON;
      return _wire_status;       
    } else {                                                                    /* Carregamento OFF */
      _wire_status = CHARGE_WIRE_OFF;
      return _wire_status;
    }
  }
    
  return _wire_status; 

}

/**
 * Utility function to convert from voltage to percentage.
 * @note Remove this utility when battery details were being retrieved
 * from the sensor.
 * @param[in] voltage
 * @return Battery percentage.
 */
uint8_t charge_get_percentage_internal(float voltage) {
  uint8_t percentage = 0;

  /* Compute percentage value */
  percentage = (voltage - 3.5) / (4.1 - 3.5) * 100;  

  /* Check return value limits */
  if(percentage < 0.0) {
          percentage = 0;
  }
  if(percentage > 100) {
          percentage = 100;
  }

  return percentage;
}

#endif 
/*
* @file		app_bodytemp.c
* @date		April 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the app_bodytemp aplication.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/
/* Interface */
#include "app_bodytemp.h"
#if MICROCONTROLER_1
/* Libs */
#include "meas_mngr.h"

/* Utilities */
#include "sense_library/periph/rtc.h"
#include "utils.h"
#include "twi_mngr.h"

/* Debug - libs */
#include "sense_library/utils/debug.h"

/* standard library */
#include <stdint.h>
#include <stdbool.h>

/* SDK */
#include "nrf_gpio.h"

/* Measurements buffer */
#include "sense_library/sensoroid/measurements_v2_manager.h"

/* Utils */
#include "sense_library/utils/debug.h"
#include "sense_library/utils/externs.h"
#include "sense_library/utils/utils.h"

/********************************** Private ************************************/
/*
 * Buffer that stores a measurement before it's addded to the queues. 
 */
static uint8_t _data_buffer[APP_BODYTEMP_DATA_PACKET_MEASUREMENT_SIZE];   

/*
 * Buffer the measurement timestamp. 
 */
static uint64_t _last_timestamp;

/*
 * Buffer that stores a given amount of timestamps before it's addded to the queues. 
 */
static uint8_t _current_index = 0;

/*
 * Stores the current aplication state.
 */
static app_states _current_state = BODYTEMP_TWI_INITIALIZING;

/*
 * Stores the sampling rate given by the upper layer.
 */
static uint64_t _sampling_rate;

/*
 * Stores the temperature measured by NTC resistance
 */
static uint64_t _lr_temperature;

/*
 * Stores a lookup table fro the threshold temperature and LCD indicator.
 */
static int16_t _lookup_table[] = { 3398,3384,3370,3357,3343,3330,3316,
                                   3303,3289,3276,3263,3249,3236,3223,
                                   3210,3197,3184,3171,3158,3145,3133,
                                   3120,3107,3095,3082,3070,3057,3045,
                                   3032,3020,3008,2996,2984,2971,2959,
                                   2947,2935,2923,2912,2900,2888,2876,
                                   2865,2853,2842,2830,2819,2807,2796,
                                   2784,2773,2762,2751,2739,2728,2717,
                                   2706,2695,2684,2673,2663,2652,2641,
                                   2630,2620,2609,2598,2588,2577,2567,
                                   2557,2546,2536,2526,2515,2505,2495,
                                   2485,2475,2465,2455,2445,2435,2425,
                                   2415,2405,2396,2386,2376,2367,2357};

/*
 * Temperature lower threshold from 340 to 420 (temp*10).
 */
static uint16_t _lower_threshold = 0;

/*
 * Temperature upper threshold from 340 to 420 (temp*10).
 */
static uint16_t _upper_threshold = 0;

/**
 *  Stores how many queue add attempts.
 */
static uint8_t _queue_add_attempts = 0;

/********************************** Private ************************************/
void _app_body_pwr_on(void);
void _app_body_pwr_off(void);
bool _app_body_add_temperature_measurement(uint16_t ntc_data);


/********************************** Public ************************************/
/*
 * @brief Function for initializing the body temperature app. 
 *
 * @paramin sampling_rate   Value of the sampling rate in milliseconds.
 */
void app_body_temperature_init(uint64_t sampling_rate) {

  _sampling_rate = sampling_rate;
  if(_sampling_rate != 0) {
    _current_state = BODYTEMP_TWI_INITIALIZING;
  } else {
    _current_state = BODYTEMP_NOP;
  }  
  _app_body_pwr_on();
  /* To Do - verificar temperatura máxima */
  /* To Do - verificar temperatura mínima */   

  /* Config APP_BODYTEMP_PWR_EN as an output */
  nrf_gpio_cfg_output(APP_BODYTEMP_PWR_EN);

  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_1, "[app_body_temperature_init] Init started \n");
}
/* 
 * @brief Body temperature loop with the state machine
 */
void app_body_temperature_loop(void) {
  /* Aplication state machine */
  switch(_current_state) {
    case BODYTEMP_TWI_INITIALIZING:
      if(twi_mngr_get_status(TWI_MNGR_INSTANCE_ID0)) {   
        ads1114_init();
        _current_state = BODYTEMP_DRIVER_INITIALIZING;
      } else {
        return;
      }
      break;
    case BODYTEMP_DRIVER_INITIALIZING:
      if(ads1114_get_init_status()) {
        _current_state = BODYTEMP_IDLE;
      } else {
        /* Driver must be properly protected against multiple calls */
        ads1114_init();
        return;
      }
      break;
    case BODYTEMP_IDLE:      
      /* Check if it's time to measure temperature */
      if((rtc_get_milliseconds() - _last_timestamp) >  _sampling_rate) {               
        /* Start an adc measurement */
        if(ads1114_readValue()) {
          debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
          debug_print_string(DEBUG_LEVEL_0, "[app_body_temperature_loop] Read started\n");
          _current_state = BODYTEMP_WAITING_DATA;
        } else {
          return;
        }
      }
      break;
    case BODYTEMP_WAITING_DATA:
      /* Check if data as arrived */
      if(ads1114_get_data()->timestamp > _last_timestamp) {       
        uint32_t temp_res = (uint32_t)(RESISTANCE_CONVERTER(ads1114_get_data()->voltage) * APP_BODYTEMP_TEMPERATURE_FACTOR);
        _last_timestamp = rtc_get_milliseconds();
        
        /* MSB first and LSB last */
        _data_buffer[0] = CONVERT_UINT16_TO_UINT8((temp_res >> 16) & 0x0000ffff, 1);
        _data_buffer[1] = CONVERT_UINT16_TO_UINT8((temp_res >> 16) & 0x0000ffff, 0);
        _data_buffer[2] = CONVERT_UINT16_TO_UINT8(temp_res & 0x0000ffff, 1);
        _data_buffer[3] = CONVERT_UINT16_TO_UINT8(temp_res & 0x0000ffff, 0);

        /* Search the NTC resistance on the lookup table */
        uint8_t i = 0;
        while(temp_res < (_lookup_table[i] * 10)) {
          i++;
        }        
        temp_res = temp_res/APP_BODYTEMP_TEMPERATURE_FACTOR;

        /* Add temperature timestamp to send */
        utils_save_uint64_t_to_array_keep_endianness(&_data_buffer[4], ads1114_get_data()->timestamp);

        debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());//////////////////////////////////////////////////////////////////////////
        debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[app_body_temperature_loop] Timestamp:%dms\n", ads1114_get_data()->timestamp);

        /* Store the low resolution temperature */
        _data_buffer[12] = CONVERT_UINT16_TO_UINT8((i + APP_BODYTEMP_LOOKUP_TABLE_MIN_TEMPERATURE), 1);
        _data_buffer[13] = CONVERT_UINT16_TO_UINT8((i + APP_BODYTEMP_LOOKUP_TABLE_MIN_TEMPERATURE), 0);               

        /* Converts charge voltage to string */
        char temp_string[10];
        _lr_temperature = _data_buffer[12] << 8 | _data_buffer[13];
        memset(temp_string, '\0', 10);
        utils_ftoa(temp_string, (float)_lr_temperature/APP_BODYTEMP_TEMPERATURE_FACTOR, 0.01);

        debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
        debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[app_body_temperature_loop] Calculated Resistance:%d, Low Resolution temperature:%sºC\n", temp_res, temp_string);
        
        _current_state = BODYTEMP_UPLOAD_DATA; 
      }
      break;
    case BODYTEMP_UPLOAD_DATA:
      debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_0, "[app_body_temperature_loop] Let's upload\n");

      /* To Do - Adicionar alertas para a RAM */         

      /* Adicionar measurement da temperatura da NTC à RAM */
      if(meas_mngr_store_temp(&_data_buffer[4])) {
        _current_state = BODYTEMP_IDLE;
      }  

      break;  
    default:
    /* In case of NOP (No operation) */
      _app_body_pwr_off();
      return;
  }
}

/* 
 * @brief Tells the upper layer if application is still running.
 *
 * @return    True if app is running, false if it's idling.
 */
bool app_body_temperature_is_busy(void) {   
  if(_current_state == BODYTEMP_IDLE || _current_state == BODYTEMP_NOP) {
    return false;
  } else {
    return true;
  }
}

/********************************** Private ***********************************/
/* 
 * @brief Enables the power to the temperature circuit..
 */
void _app_body_pwr_on(void) {   
  nrf_gpio_pin_set(APP_BODYTEMP_PWR_EN);
}

/* 
 * @brief Disables the power to the temperature circuit..
 */
void _app_body_pwr_off(void) {   
  nrf_gpio_pin_clear(APP_BODYTEMP_PWR_EN);
}
#endif
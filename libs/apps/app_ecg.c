/*
* @file		app_ecg.c
* @date		September 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the ads129x utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/
/* Interface */
#include "app_ecg.h"

/* Pheriperals */
#include "sense_library/periph/rtc.h"

/* Utils */
#include "sense_library/utils/debug.h"

/* APPS */
#include "meas_mngr.h"

/* Utils */
#include "sense_library/utils/debug.h"
#include "sense_library/utils/utils.h"

/* C standard library */
//#include <stdint.h>
//#include <stdbool.h>
//#include <stdlib.h>
//#include "arm_math.h"
//uint16_t _idxxx = 0;

/********************************** Private ************************************/
/*
* Variável estado do loop
*/
static app_ecg_states _current_state;

/*
* Variável da ultimo timestamp de medida adquirido
*/
static uint64_t _data_timestamp;

/*
* Variável de estado da configuração inicial
*/
static bool _init_config_status;

/*
* Variável do estado do save mode
*/
static bool _save_mode;

/*
 * Callback function to check if save mode is active
 */
static power_save_callback_def _power_save_callback;

/* Private functions list */


//arm_fir_instance_f32 ssss;
//float32_t samples_to_filter[5000];
//float32_t samples_filtered[5000];

//static uint8_t _count = 0;

//#define BLOCK_SIZE            10
//#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
///* Must be a multiple of 16 */
//#define NUM_TAPS_ARRAY_SIZE              32
//#else
//#define NUM_TAPS_ARRAY_SIZE              29
//#endif
//#define NUM_TAPS              29

///* -------------------------------------------------------------------
// * Declare State buffer of size (numTaps + blockSize - 1)
// * ------------------------------------------------------------------- */
//#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
//static float32_t firStateF32[2 * BLOCK_SIZE + NUM_TAPS - 1];
//#else
//static float32_t firStateF32[BLOCK_SIZE + NUM_TAPS - 1];
//#endif 
///* ----------------------------------------------------------------------
//** FIR Coefficients buffer generated using fir1() MATLAB function.
//** fir1(28, 6/24)
//** ------------------------------------------------------------------- */
//#if defined(ARM_MATH_MVEF) && !defined(ARM_MATH_AUTOVECTORIZE)
//const float32_t firCoeffs32[NUM_TAPS_ARRAY_SIZE] = {
//  -0.0018225230f, -0.0015879294f, +0.0000000000f, +0.0036977508f, +0.0080754303f, +0.0085302217f, -0.0000000000f, -0.0173976984f,
//  -0.0341458607f, -0.0333591565f, +0.0000000000f, +0.0676308395f, +0.1522061835f, +0.2229246956f, +0.2504960933f, +0.2229246956f,
//  +0.1522061835f, +0.0676308395f, +0.0000000000f, -0.0333591565f, -0.0341458607f, -0.0173976984f, -0.0000000000f, +0.0085302217f,
//  +0.0080754303f, +0.0036977508f, +0.0000000000f, -0.0015879294f, -0.0018225230f, 0.0f,0.0f,0.0f
//};
//#else
//const float32_t firCoeffs32[NUM_TAPS_ARRAY_SIZE] = {
//  -0.0011,-0.0002,-0.0003, -0.0031, -0.0095, -0.0150, -0.0098,  0.0149,  0.0551,  0.0897,  0.0901,  0.0405, -0.0453, -0.1268,  0.8415, -0.1268, -0.0453,  0.0405,  0.0901,  0.0897,  0.0551,  0.0149, -0.0098, -0.0150, -0.0095, -0.0031, -0.0003, -0.0002, -0.001
//};
//#endif

/********************************** Public ************************************/
/*
 * @brief Function for initializing ECG App
 *
 * @param[in] power_save_callback Pointer to check if save mode is active
 */
void app_ecg_init(power_save_callback_def power_save_callback) {
  
  _power_save_callback = power_save_callback;
  _current_state = APP_ECG_POWERED_OFF;
  char *sequence[] = ADS129X_DATA_ORDER;
  meas_mngr_ecg_set_sequence(sequence, ARRAY_LEN(sequence));
     
  /* Call FIR init function to initialize the instance structure. */
  //arm_fir_init_f32(&ssss, NUM_TAPS, (float32_t *)&firCoeffs32[0], &firStateF32[0], BLOCK_SIZE);  

}


/**
 * ECG application loop. 
 */
void app_ecg_loop(void) {
    
  if(_power_save_callback()) {                          /* Check if save mode is active */
    _current_state = APP_ECG_SAVE_MODE;
    _save_mode = true;
  } else if(!_power_save_callback() && _save_mode) {    /* If save mode has become false, start sampling */
    app_ecg_start_sample();
    _current_state = APP_ECG_SAMPLING;
    _save_mode = false;
  }
  
  /* Data sampling state machine */
  switch (_current_state) {
    case APP_ECG_POWERED_OFF:

      if(ads129x_init()) {      
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[app_ecg_loop] Powered up sucessfully\r\n");
        _current_state = APP_ECG_POWER_ON;
      }

      break;   
    case APP_ECG_POWER_ON:            
      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[app_ecg_loop] Waiting for init config\r\n");
      _current_state = APP_ECG_NOP;     

      break; 
    case APP_ECG_START_SAMPLE:
      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[app_ecg_loop] Waiting for sample to start\r\n");
      _current_state = APP_ECG_NOP;   

      break;
    case APP_ECG_SAMPLING:       

      /* Verificar se existe novo data para envio */      
      if(utils_get_serial_from_array(&ads129x_get_data()[0]) > _data_timestamp) {

        uint64_t timestamp = utils_get_serial_from_array(&ads129x_get_data()[0]);
        debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
        debug_printf_string(DEBUG_LEVEL_0, "[app_ecg_loop] Send data timestamp: %d\n", timestamp);


        _current_state = APP_ECG_UPLOAD_DATA;
        _data_timestamp = rtc_get_milliseconds();
        debug_print_time(DEBUG_LEVEL_3, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_3, "[app_ecg_loop] Going to send data\n");
      }

      break;
    case APP_ECG_UPLOAD_DATA:
      
      //if(_count < 5000) {
      //  samples_to_filter[_count] = (utils_get_uint32_from_array(&ads129x_get_data()[8]))/ADS129x_INTEGER_CONVERT;
      //  _count++;
      
      //  if(_count >= 10){
      //    arm_fir_f32(&ssss, &samples_to_filter[_idxxx], samples_filtered, 10);
      //    _idxxx++;
      //  }
      //} else{
      //  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      //  debug_print_string(DEBUG_LEVEL_0, "[app_ecg_loop] Filtering Ended, Going to print\n");

      //  /* Converts charge voltage to string */
      //  char voltage_string[10];
      //  memset(voltage_string, '\0', 10);
        
      //  for(int i=0; i < 5000; i++) {
      //    memset(voltage_string, '\0', 10);
      //    utils_ftoa(voltage_string, (float)samples_to_filter[i] * 1000, 0.00000001);

      //    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      //    debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"%s\n", voltage_string);
          
      //  }


      //}
      
      /* Upload to RAM */
      //meas_mngr_store_ecg(ads129x_get_data());      
      _current_state = APP_ECG_SAMPLING;

      break;
    case APP_ECG_SAVE_MODE:
      app_ecg_stop_sample();
      _current_state = APP_ECG_NOP;
      break;    
    default: 
      break;    
  } 
}


/*
 * @brief Function to configure ECG devices
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
 * @retval                  Returns true if the config was successfull and with valid gains, or false otherwise
 */
bool app_ecg_config(bool accuracy, bool lead_off, uint8_t ecg_gain, uint8_t resp_gain, uint8_t test_mode) {
  
  uint8_t status;
  uint8_t retries = 0;
  
  /* Configurações iniciais */
  if(!_init_config_status) {     
    while(true) {
      if(ads129x_configs(accuracy, lead_off, ecg_gain, resp_gain, test_mode)) {    
        _current_state = APP_ECG_NOP;
        return true;
      }
      retries++;
        
      /* Verificar o número de tentativas */
      if(retries == APP_ECG_RETRIES_CMD) {
        return false;
      } 
    }
  }
    
  /* Precaution */
  if(_current_state == APP_ECG_SAMPLING) {
    return false;
  }

  /* User Configs */
  while(true) {
    if(ads129x_user_configs(accuracy, lead_off, ecg_gain, resp_gain, test_mode)) {    
      _current_state = APP_ECG_NOP;
      return true;
    }
    retries++;
      
    /* Verificar o número de tentativas */
    if(retries == APP_ECG_RETRIES_CMD) {
      return false;
    } 
  }
  
  return false;
}


/*
 * @brief Function to enable sampling of ECG devices 
 *                            
 * @retval                  Returns true if the start sample was successfull, otherwise false
 */
bool app_ecg_start_sample(void) {

  uint8_t retries;

  /* Precaution */
  if(_current_state == APP_ECG_SAMPLING) {
    return true;
  }

  /* Iniciar amostragem */
  while(true) {
    uint8_t status = ads129x_start_sample();
    
    /* Check start sample status */
    if(status == ADS129X_TRUE) {
      break;
    }else if(status == ADS129X_FAULT) {
      retries++;
    }
    
    /* Verificar o número de tentativas */
    if(retries == APP_ECG_RETRIES_CMD) {
      return false;
    }
  }    
  
  retries = 0;
  for(; retries < APP_ECG_RETRIES_CMD; retries++) {
    if(ads129x_start_datac()) {
      _current_state = APP_ECG_SAMPLING;
      return true;
    }
  }

}


/*
 * @brief Function to disable sampling of ECG devices 
 *                            
 * @retval                  Returns true if the disable sample was successfull, otherwise false
 */
bool app_ecg_stop_sample(void) {
    
  /* Precaution */
  if(_current_state == APP_ECG_NOP) {
    return true;
  }  
  
  for(int retries = 0 ; retries < APP_ECG_RETRIES_CMD; retries++) {
    if(ads129x_stop_datac()) {
      _current_state = APP_ECG_NOP;
      return true;
    }
  } 
  return false;

}


/*
 * @brief Function to power off ECG devices 
 *                            
 * @retval                  Returns true if the power off was successfull, otherwise false
 */
void app_ecg_power_off(void) {
  
  ads129x_uninit(); 

}


/**
 * Asks if the application is running.
 * @return True if it is busy right now, false otherwise.
 */
bool app_ecg_is_busy(void) {

  if(_current_state == APP_ECG_SAMPLING || _current_state == APP_ECG_NOP) {    
    return false;
  } else {
    return true;
  }

}
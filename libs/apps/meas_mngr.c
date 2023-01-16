/*
* @file		measurement_mngr.c
* @date		July 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the measurements manager funtions.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/
/* Interface */
#include "meas_mngr.h"

/* Driver */
#include "drivers/mc_23k640.h"

/* Utilities */

#include "utils.h"

/* Sense */
#include "sense_library/utils/debug.h"
#include "sense_library/utils/utils.h"
#include "sense_library/sensoroid/measurements_v2_manager.h"
#include "sense_library/periph/rtc.h"
/* Gama libs */
#include "sense_library/protocol-gama/gama_fw/include/gama_generic.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_measurement.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_definitions.h"

/* Standard library */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "nrf_delay.h"

/********************************** Private ************************************/
const uint8_t EXTERNAL_PROBE_SENSOR_SEQUENCE           = (16);  ///< External probe sensor sequence.

/**
 *  Stores how many queue add attempts.
 */
static uint8_t _queue_add_attempts = 0;
/*
 * 
 */
static char **_ecg_sequence;

/*
 * 
 */
static uint16_t ecg_size = 0;

/*
 * 
 */
static uint16_t ram_idx = 0;

/*
 * 
 */
static uint8_t ecg_hash[ECG_SEQUENCE_SIZE];

/*
 * 
 */
static char *_ram_sequence[] = MEASURES_CONTENT;

/*
 * 
 */
static uint8_t _ram_sequence_sizes[] = MEASURES_CONTENT_SIZE;

/*
 * 
 */
static uint8_t ram_buffer[RAM_BUFFER_SIZE] = "0";

/*
 * 
 */
static meas_mngr_states current_state;

/*
 * 
 */
 static uint8_t ram_counter = 0;

/*
 * 
 */
 static uint16_t total_sequence_size = 0;

/*
 * 
 */
static uint16_t upload_ecg_meas_idx;

/*
 * 
 */
static uint16_t upload_ecg_meas_bytes = 0;
/*
 * @brief  
 * 
 * @param[in] 
 * @retval 
 */
bool meas_mngr_add_measurement(uint8_t *value, uint8_t bytes) {

  float data = (float)utils_get_uint32_from_array(value) / 100000000;
  
  gama_measure_format_v2_fields_t measurement_fields;
  measurement_fields.measure_type = GAMA_MEASURE_ECG;
  measurement_fields.sensor = EXTERNAL_PROBE_SENSOR_SEQUENCE;
  measurement_fields.config_byte = MEASURE_VALUE_FLOAT_TYPE;
  measurement_fields.val = &data;

  /* Converts pressure to string */
  char str[10];
  memset(str, '\0', 10);
  utils_ftoa(str, data, 0.000001);

  if(measurements_v2_manager_add_measurement(&measurement_fields)) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_1,(uint8_t*)"[add_ecg_measurement] measurement added; details: Voltage = %sV\n", str);
    
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


/*
 * @brief  
 * 
 * @param[in] 
 * @retval 
 */
void meas_mngr_interrupt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
  ram_counter++;
  if(pin != MC_23k640_CS2 || action != NRF_GPIOTE_POLARITY_LOTOHI) {
    return;
  }

  if(ram_counter >= READ_RAM_THRESHOLD) {
    nrf_delay_ms(1);
    ram_counter = 0;
    uint8_t read_str[RAM_BUFFER_SIZE] = "\0";
    uint8_t rbytes = 1;
    uint64_t timestamp;
    while(rbytes) {
      rbytes = mc_23k640_read_data(read_str);
      debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"Bytes read: %d", rbytes);
      
      //debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[main] Bytes remaining: %d , Str read: %s\n\n", rbytes, read_str); 
      
      timestamp = utils_get_serial_from_array(read_str);
      //debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"Timestamp: %ld \n", timestamp);
      for(uint16_t i=0; i < total_sequence_size ; i++) {
        debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"%x", read_str[i]);
      }
      
      debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"\n\n");
      /* Clear */
      memset(read_str, '\0', RAM_BUFFER_SIZE);
    }
    meas_mngr_add_measurement(&read_str[upload_ecg_meas_bytes], _ram_sequence_sizes[upload_ecg_meas_idx]);
  }
}

/*
 * @brief  
 * 
 * @param[in] 
 * @retval 
 */
void meas_mngr_init(void) {
 
  mc_23k640_init();
   for(uint16_t i = 0 ; i < ARRAY_SIZE(_ram_sequence_sizes) ; i++) {
    total_sequence_size += _ram_sequence_sizes[i];
  }
#if MICROCONTROLER_2
 
  /* GPIO Configuration */
  nrf_drv_gpiote_in_config_t in_config = 
  {
    .sense = NRF_GPIOTE_POLARITY_LOTOHI,                
    .pull = NRF_GPIO_PIN_NOPULL,                        
    .is_watcher = false,                                
    .hi_accuracy = false,                             
    .skip_gpio_setup = false 
  };
  
  /* Calculate idx of upload ecg measurement on config.h buffer */
  for(uint8_t i = 0; i < DEVICE_MEASURES_NUMBER ; i++) {
    if(!strcmp(_ram_sequence[i], UPLOAD_ECG_MEAS)) {
      break;
    } else {
      upload_ecg_meas_bytes += i * _ram_sequence_sizes[i];
    }
  }

  /* Inicializar GPIO DRY */
  if(!nrf_drv_gpiote_in_init(MC_23k640_CS2, &in_config, meas_mngr_interrupt_handler)) {
    nrf_drv_gpiote_in_event_enable(MC_23k640_CS2, true);
    current_state = MEAS_MNGR_IDLE;
  } else{
    current_state = MEAS_MNGR_INITIALIZING;
  }

#endif 
 
#if MICROCONTROLLER_1
  current_state = MEAS_MNGR_IDLE;
#endif
}

/*
 * @brief  
 * 
 * @param[in] 
 * @retval 
 */
void meas_mngr_ecg_set_sequence(char *ecg_sequence[], uint16_t sequence_size) {
  _ecg_sequence = ecg_sequence;
  uint16_t _sequence_size = sequence_size;

  /* Calculate ecg sequence hash */
  for(int16_t seq_idx = 0 ; seq_idx < sequence_size ; seq_idx++) {
    uint16_t ecg_i = 0;

    /* Search the ecg sequence for a specific sequence, aka _ram_sequence[seq_idx] */
    while(strcmp(_ram_sequence[seq_idx], _ecg_sequence[ecg_i])) {
      ecg_i++;

      /* When we reach the end of the ecg sequence */
      if(ecg_i > sequence_size) {
        ecg_i = SEQUENCE_ERROR;
        break;
      }
    }
    if(ecg_i != SEQUENCE_ERROR) {
      ecg_hash[seq_idx] = ecg_i;

      /* Calculate ECG sequence size */
      ecg_size += _ram_sequence_sizes[seq_idx];
    }
  }
}

/* 
 * @brief
 *
 * @param[in] 
 * @retval 
 */
bool meas_mngr_store_temp(uint8_t *data) {
  uint8_t copied_bytes = 0;
  uint8_t temp_size = _ram_sequence_sizes[0] + _ram_sequence_sizes[1];
  int8_t write_retries = MAX_RAM_RETRIES;

  for(uint8_t i = 0 ; copied_bytes < temp_size ; i++) {
    ram_buffer[i] = data[i];
    copied_bytes++;
  }
  /* Write data to RAM !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! APAGAR!!!!!!!!!!!!!!!!!!!!*/
  while(!mc_23k640_write_data(data, total_sequence_size) && (write_retries >= 0)) {
    write_retries--;
  }
  memset(ram_buffer, 0, total_sequence_size);
  debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[meas_mngr_ram_write] Write with: %d retries\n", MAX_RAM_RETRIES - write_retries);
  if(write_retries <= 0) {
    return false;
  }
  return true;
}
/* 
 * @brief
 *
 * @param[in] 
 * @retval 
 */
bool meas_mngr_store_ecg(uint8_t *data) {

  uint16_t copied_bytes = 0;
  int8_t write_retries = MAX_RAM_RETRIES;

  /* Apply the hash key */
  for(uint8_t i = 0 ; copied_bytes < total_sequence_size ; i++) {
    if(ecg_hash[i] != SEQUENCE_ERROR) {
      ram_buffer[i] = data[ecg_hash[i]];
    }
    copied_bytes++;
  }
    
  /* Write data to RAM */
  while(!mc_23k640_write_data(data, total_sequence_size) && (write_retries >= 0)) {
    write_retries--;
  }
  memset(ram_buffer, 0, total_sequence_size);

  debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[meas_mngr_ram_write] Write with: %d retries\n", MAX_RAM_RETRIES - write_retries); 

   
}

void meas_mngr_loop(void) {

  switch(current_state) {
    case MEAS_MNGR_INITIALIZING:
      meas_mngr_init();
      break;
    case MEAS_MNGR_IDLE:
      
      break;
  }
}



/*
* @file         telco.c
* @date         August 2021
* @author       Modified by PFaria & JAntunes
*
* @brief        This file has the telco utilities that controls the lower hierarchy layers (cellular and simcom).
                The telco layer was adapted from Sensefinity proprietary code, so there's alot of features we don't support.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*
*  This source code was adapted from the Sensefinity company provided source code:
  /*
   * Copyright (c) 2020 Sensefinity
   * This file is subject to the Sensefinity private code license.
   */
  /**
   * @file  telco.c
   * @brief This file has the telco definitions.
   */
/* 
* 
*/

/********************************** Includes ***********************************/

/* Telecomunications are on used by MCU1 */


/* Interface */
#include "telco.h"
#if MICROCONTROLER_2

/* SDK */
#include "nrf_gpio.h"
#include "nrf_uart.h"
#include "app_uart.h"
#include "nrf_drv_uart.h"
#include "nrf_uarte.h"

/* Libs */
#include "sense_library/sensors/simcom.h"
#include "sense_library/cellular/cellular.h"
#include "sense_library/uplink/uplink.h"

/* Apps */
#include "power_management.h"

/* Periph */
#include "sense_library/periph/rtc.h"

/* Queues */
#include "sense_library/sensoroid/gama_queues.h"

/* Hello */
#include "sense_library/sensoroid(evian)/hello.h"

/* Measurements manager */
#include "sense_library/sensoroid/measurements_v2_manager.h"

/********************************** Private ************************************/
/**
 * Tells if we need to store in the queue a new power off measurement.
 */
static bool _add_power_off_measurement = false;

/**
 * Tells if we need to store in the queue a new battery saving measurement.
 */
static bool _add_battery_saving_measurement = false;

/**
 * Tells if hello with the cellular parameters was already uploaded.
 */
static bool _hello_uploaded = false;

/**
 * Tells if there is a cellular module available in the PCB.
 */
static bool _cellular_module_available = false;

/**
 * Telco UART callback function.
 * @param[in] p_event UART event that caused interruption.
 */
void telco_uart_handler(app_uart_evt_t* p_event) {
  if (p_event->evt_type == APP_UART_DATA_READY) {
    uint8_t rx_byte;
    app_uart_get(&rx_byte);
    simcom_rx_new_char(rx_byte);
  }
}

/**
 * Telco UART tx function.
 * @param[in] tx_buffer      Buffer to be transmitted.
 * @param[in] tx_buffer_size Buffer's length.
 */
void telco_uart_tx_buffer(char* tx_buffer, uint16_t tx_buffer_size) {
  for (uint16_t i = 0; i < tx_buffer_size; i++) {
    app_uart_put(tx_buffer[i]);
  }
}

/**
 * Telco UART function init.
 */
void telco_uart_init(void) {
  uint32_t err_code;

  /* SIMCom UART drivers parameters */
  const app_uart_comm_params_t uart_parameters = {
    TELCO_UART_RX_PIN_NUMBER,
    TELCO_UART_TX_PIN_NUMBER,
    TELCO_UART_RTS_PIN_NUMBER,
    TELCO_UART_CTS_PIN_NUMBER,
    TELCO_UART_PARITY,
    TELCO_UART_FLOW_CONTROL,
    TELCO_UART_BAUDRATE};

  /* SIMCom UART initialization */
  APP_UART_FIFO_INIT(&uart_parameters,
    TELCO_UART_FIFO_RX_SIZE,
    TELCO_UART_FIFO_TX_SIZE,
    telco_uart_handler,
    TELCO_UART_INTERRUPTION_PRIORITY,
    err_code);
}

/**
 * Telco UART function reset.
 */
void telco_uart_reset(void) {
  simcom_reset_variables();
  telco_uart_init();
}

/**
 * Telco UART function shutdown.
 */
void telco_uart_shutdown(void) {
  app_uart_close();
}

/**
 * @todo Implement this function later on.
 */
/**
 * Creates a registration failure measurement.
 * @param[in] current_timestamp
 * @param[in] registration_failed_time Data content to be included in the measurement.
 * @return    True if measurement was created, false otherwise.
 */
bool telco_registration_failure_create_message(uint64_t current_timestamp, uint64_t registration_failed_time) {
  return true;
}

/**
 * @todo Implement this function later on.
 */
/**
 * Creates a data session failure measurement.
 * @param[in] current_timestamp
 * @param[in] session_failure_time Data content to be included in the measurement.
 * @return    True if measurement was created, false otherwise.
 */
bool telco_session_failure_create_message(uint64_t current_timestamp, uint64_t session_failure_time) {
  return true;
}

/**
 * @todo Implement this function later on.
 */
/**
 * Creates an HTTP failure measurement.
 * @param[in] current_timestamp
 * @param[in] http_failed_time Data content to be included in the measurement.
 * @return    True if measurement was created, false otherwise.
 */
bool telco_http_failure_create_message(uint64_t current_timestamp, uint64_t http_failed_time) {
  return true;
}

/**
 * @todo Implement this function later on.
 */
/**
 * Creates an NB-IoT timer 3346 measurement.
 * @param[in] current_timestamp
 * @param[in] timer_3346_value Data content to be included in the measurement.
 * @return    True if measurement was created, false otherwise.
 */
bool telco_timer_3346_create_message(uint64_t current_timestamp, uint64_t timer_3346_value) {
  return true;
}

/**
 * Creates a hello message containing important paramenters
 * about the communication module (e.g. ICCID, IMEI, etc.).
 * @param[in] iccid          ICCID information.
 * @param[in] imei           IMEI information.
 * @param[in] imsi           IMSI information.
 * @param[in] simcom_version SIMCom module version (includes module's firmware version).
 * @param[in] current_timestamp
 */
void telco_send_hello_msg(char* iccid, char* imei, char* imsi, char* simcom_version, uint64_t current_timestamp) {
  if(!_hello_uploaded) {
    _hello_uploaded = hello_add_cellular_info_message(iccid, imei, imsi, simcom_version, current_timestamp);
  }
}

/**
 * @todo Implement this function later on.
 */
bool download_message(uint8_t* msg, uint32_t msg_size) {
  /* Not supported on SAMB board */
  /* Timestamp in message */
  timestamping_reverse_in_packet(msg, rtc_get_milliseconds());

  //node_receive_new_message(msg, msg_size);
  return true;
}

/**
 * @todo Implement this function later on if still makes sense.
 */
bool power_management_get_power_off_device_callback(void) {
  return true;
}

/**
 * @todo Implement this function later on if still makes sense.
 */
void power_management_reset_module(void) {
  power_management_set_telco_upload_finished();
}

/**
 * SIMCOM_VBAT_CTRL pin clear.
 */
void telco_simcom_vbat_ctrl_pin_clear(void) {
  /* Not supported on SAMB board */
}

/**
 * SIMCOM_VBAT_CTRL pin set.
 */
void telco_simcom_vbat_ctrl_pin_set(void) {
  /* Not supported on SAMB board */
}

/**
 * SIMCOM_VBAT_CTRL pin initialization.
 */
void telco_simcom_vbat_ctrl_pin_init(void) {
  /* Not supported on SAMB board */
}

/**
 * SIMCOM_PWR_KEY pin clear.
 */
void telco_simcom_pwr_key_pin_clear(void) {
   nrf_gpio_pin_clear(SIMCOM_PWR_KEY);
}

/**
 * SIMCOM_PWR_KEY pin set.
 */
void telco_simcom_pwr_key_pin_set(void) {
  nrf_gpio_pin_set(SIMCOM_PWR_KEY);
}

/**
 * SIMCOM_PWR_KEY pin initialization.
 */
void telco_simcom_pwr_key_pin_init(void) {
  nrf_gpio_cfg_output(SIMCOM_PWR_KEY);
  /* Start SIMCOM_PWR_KEY pin on */
  telco_simcom_pwr_key_pin_set();
}

/**
 * @brief SIMCOM_LVL_SHFT_OE pin clear.
 */
void telco_simcom_lvl_shft_oe_pin_clear(void) {
  nrf_gpio_pin_clear(SIMCOM_LVL_SHFT_OE);
}

/**
 * SIMCOM_LVL_SHFT_OE pin set.
 */
void telco_simcom_lvl_shft_oe_pin_set(void) {
  nrf_gpio_pin_set(SIMCOM_LVL_SHFT_OE);
}

/**
 * SIMCOM_LVL_SHFT_OE pin initialization.
 */
void telco_simcom_lvl_shft_oe_pin_init(void) {
  nrf_gpio_cfg_output(SIMCOM_LVL_SHFT_OE);

  /* Start SIMCOM_LVL_SHFT_OE pin off */
  telco_simcom_lvl_shft_oe_pin_clear();
}

/**
 * SIMCOM_STATUS pin initialization.
 */
void telco_simcom_status_pin_init(void) {
  /* Not supported on SAMB board (a jumper wire is needed) */  
  nrf_gpio_cfg_input(SIMCOM_STATUS, GPIO_PIN_CNF_PULL_Pullup);
}

/**
 * Read SIMCOM_STATUS pin current status.
 */
bool telco_simcom_status_pin_read(void) {
  /* Not supported on SAMB board (a jumper wire is needed) */
  uint32_t status_pin_current_status = nrf_gpio_pin_read(SIMCOM_STATUS);

  if (status_pin_current_status == 0) {
    return false;
  } else {
    return true;
  }
}

/********************************** Public *************************************/

/**
 * Telco initialization.
 */
void telco_init(void) {
  _cellular_module_available = false;
  _hello_uploaded = false;
  _add_power_off_measurement = false;
  _add_battery_saving_measurement = false;
  
  /* SIMCom UART tx function pointer setup */
  simcom_setup(telco_uart_tx_buffer);

  /* Telco UART initialization call */
  telco_uart_init();

  /* Cellular initialization */
  _cellular_module_available = cellular_setup(
      telco_registration_failure_create_message,
      telco_session_failure_create_message,
      telco_http_failure_create_message,
      telco_timer_3346_create_message,
      telco_send_hello_msg,
      telco_simcom_vbat_ctrl_pin_init, telco_simcom_vbat_ctrl_pin_clear, telco_simcom_vbat_ctrl_pin_set,
      telco_simcom_pwr_key_pin_init, 
      telco_simcom_pwr_key_pin_clear, 
      telco_simcom_pwr_key_pin_set,
      telco_simcom_lvl_shft_oe_pin_init, 
      telco_simcom_lvl_shft_oe_pin_clear, 
      telco_simcom_lvl_shft_oe_pin_set,
      telco_simcom_status_pin_init, 
      telco_simcom_status_pin_read,
      power_management_is_battery_saving_mode_active,
      rtc_get_milliseconds,
      telco_uart_reset, telco_uart_shutdown);

  /* Uplink initialization */
  uplink_setup(
      rtc_get_milliseconds(),
      cellular_loop, 
      cellular_has_data_to_upload, 
      cellular_set_has_data_to_upload,
      cellular_is_time_to_set_data_to_upload, 
      cellular_set_data_to_upload,
      cellular_has_data_to_read, 
      cellular_data_successfully_uploaded,
      cellular_has_downloaded_data, 
      cellular_get_downloaded_data, 
      cellular_set_skip_downloaded_data,
      cellular_is_powered_on,
      gama_queues_print_sequence_numbers,
      power_management_reset_module,
      power_management_get_power_off_device_callback,
      power_management_is_battery_saving_mode_active,
      download_message,
      cellular_get_current_communication_status);

  /* Cellular PSM initialization */
  cellular_set_psm_parameters(TELCO_NBIOT_PSM_ON, 
      TELCO_NBIOT_T3324, 
      TELCO_NBIOT_T3412, 
      rtc_get_milliseconds());

  /* Cellular eDRX initialization */
  cellular_set_edrx_parameters(TELCO_NBIOT_EDRX_ON, 
      TELCO_NBIOT_EDRX_PERIOD, 
      TELCO_NBIOT_PTW_PERIOD, 
      rtc_get_milliseconds());
  
}

/**
 * Telco loop.
 * param[in] current_timestamp
 */
void telco_loop(uint64_t current_timestamp) {
  /* Check if we need to add new power off measurement */
  if(_add_power_off_measurement) {
    uplink_send_power_off_message_autonomously(rtc_get_milliseconds());
    _add_power_off_measurement = false;

    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[telco_loop] Power off status measurement added.\n"); 
  }

  /* Check if we need to add new battery saving measurement */
  if(_add_battery_saving_measurement) {
    /* Only runs if we enter in saving mode */
    uplink_send_power_saving_message_autonomously(rtc_get_milliseconds());
    _add_battery_saving_measurement = false;

    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[telco_loop] Battery saving measurement added.\n");
  } 

  if(power_management_is_battery_saving_mode_active() && !uplink_is_busy()) {
    /* In case we enter in power saving and we need to send an alert to cloud */
    if(uplink_get_power_off_message_autonomously() || uplink_get_power_saving_message_autonomously()) {
      uplink_loop(current_timestamp, 
        TELCO_UPLINK_ON_QUEUE_SIZE, 
        TELCO_UPLINK_QUEUE_SIZE, 
        TELCO_UPLINK_ON_TIMEOUT, 
        TELCO_UPLINK_TIMEOUT, 
        TELCO_UPLINK_REMAIN_OPEN, 
        TELCO_UPLINK_REGISTRATION_FAILED, 
        1, 1, true);
    }
  }
  else {
    /* Normal call when we need to upload */
    uplink_loop(current_timestamp, 
      TELCO_UPLINK_ON_QUEUE_SIZE, 
      TELCO_UPLINK_QUEUE_SIZE, 
      TELCO_UPLINK_ON_TIMEOUT, 
      TELCO_UPLINK_TIMEOUT, 
      TELCO_UPLINK_REMAIN_OPEN, 
      TELCO_UPLINK_REGISTRATION_FAILED, 
      1, 1, true);
  }
   
} // end of telco loop


/**
 * Set Telco internal variable to send battery saving mode message
 * in an autonomous way.
 */
void telco_send_battery_saving_mode_message_autonomously(void) {
  _add_battery_saving_measurement = true;

  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_0,(uint8_t*)"[telco_send_battery_saving_mode_message_autonomously] Add measurement.\n");
}

/**
 * Set Telco internal variable to send device's power off message
 * in an autonomous way.
 */
void telco_send_power_off_message_autonomously(void) {
  _add_power_off_measurement = true;
  
  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_0,(uint8_t*)"[telco_send_power_off_message_autonomously] Add measurement.\n");
}

/**
 * Calls module respective function.
 * @return True if cells service is supported, false otherwise.
 */
bool telco_support_cells_service(void) {
  cellular_support_cells_service();
}

/**
 * Calls module respective function.
 * @param[in] current_timestamp
 */
void telco_collect_cells(uint64_t current_timestamp) {
  cellular_collect_cells(current_timestamp);
}

/**
 * Calls module respective function.
 * @return True if new cells information is available.
 */
bool telco_has_cells_data(void) {
  return cellular_has_cells_data();
}

/**
 * Calls module respective function.
 * @param[in] cells_info Cells data structure.
 */
void telco_read_cells_data(struct cells_data* cells_info) {
  cellular_read_cells_data(cells_info);
}

/**
 * Calls module respective function.
 * @return True if new cells information is being collected right now, false otherwise.
 */
bool telco_is_collecting_cells_sample(void) {
  return cellular_is_collecting_cells_sample();
}

/**
 * Calls module respective function.
 * @return RSSI data structure.
 */
struct rssi_data* telco_get_rssi_data(void) {
  return cellular_get_rssi_data();
}

/**
 * Asks if the telco has pending operations.
 * @return True if it is busy right now, false otherwise.
 */
bool telco_is_busy(void) {
  if(uplink_is_busy()) {
    return true;
  } else if(_add_power_off_measurement) {
    return true;
  } else if(_add_battery_saving_measurement) {
    return true;
  } else {
    return false;
  }
}
#endif
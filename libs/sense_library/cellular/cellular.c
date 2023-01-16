/**
 * Copyright (c) 2016 Sensefinity
 *
 * This file is subject to the Sensefinity private code license. See the file
 * LICENSE in the top level directory for more details.
 */

/*
 * @ingroup     app
 * @{
 *
 * @file        cellular.c
 * @brief       Implementation of the prototypes in the header file of same name
 *
 * @author      João Ambrósio <joao.ambrosio@sensefinity.com>
 *
 * @}
 */

/* Interface */
#include "cellular.h"

/* RSSI to get RSSI data structure */
#include "sense_library/apps/rssi.h"

/********************************** Private ***********************************/

static bool _psm_wakeup_now = false;

static uint64_t _psm_wakeup_timestamp = 0;

static bool _psm_is_currently_active = false;

static bool _gps_is_already_collecting = false;

static uint64_t _gps_turn_on_module_timestamp = 0;

static uint64_t _simcom_not_ready_to_rx_new_command_timestamp = 0;

static uint64_t _last_registration_timeout = RESPONSE_WAITING_TIME_3600S;

static uint64_t _last_idle_state_machine_execution_timestamp = 0;
static uint64_t _last_warm_up_state_machine_execution_timestamp = 0;
static uint64_t _last_info_state_machine_execution_timestamp = 0;
static uint64_t _last_data_session_state_machine_execution_timestamp = 0;
static uint64_t _last_http_state_machine_execution_timestamp = 0;
static uint64_t _last_cells_state_machine_execution_timestamp = 0;
static uint64_t _last_gps_state_machine_execution_timestamp = 0;

static uint64_t _last_timers_status_print_timestamp = 0;

static uint64_t _last_time_tx_unsuccessfully_done_print = 0;

static uint64_t _last_time_phase_set_timestamp = 0;

static bool _vbat_ctrl_current_state = false;

static bool _wakeup_from_psm_by_cells = false;

static registration_failure_create_message_callback_def _registration_failure_create_message_callback = NULL;
static session_failure_create_message_callback_def      _session_failure_create_message_callback = NULL;
static http_failure_create_message_callback_def         _http_failure_create_message_callback = NULL;
static timer_3346_create_message_callback_def           _timer_3346_create_message_callback = NULL;
static sensoroid_send_hello_msg_callback_def            _sensoroid_send_hello_msg_callback = NULL;

static cellular_init_vbat_ctrl_callback_def             _cellular_init_vbat_ctrl_callback = NULL;
static cellular_clear_vbat_ctrl_callback_def            _cellular_clear_vbat_ctrl_callback = NULL;
static cellular_set_vbat_ctrl_callback_def              _cellular_set_vbat_ctrl_callback = NULL;

static cellular_init_power_key_callback_def             _cellular_init_power_key_callback = NULL;
static cellular_clear_power_key_callback_def            _cellular_clear_power_key_callback = NULL;
static cellular_set_power_key_callback_def              _cellular_set_power_key_callback = NULL;

static cellular_init_lvl_sh_oe_callback_def             _cellular_init_lvl_sh_oe_callback = NULL;
static cellular_clear_lvl_sh_oe_callback_def            _cellular_clear_lvl_sh_oe_callback = NULL;
static cellular_set_lvl_sh_oe_callback_def              _cellular_set_lvl_sh_oe_callback = NULL;

static cellular_init_status_callback_def                _cellular_init_status_callback = NULL;
static cellular_read_status_callback_def                _cellular_read_status_callback = NULL;

static power_management_is_battery_saving_mode_callback_def _power_management_is_battery_saving_mode_callback = NULL;

static sysclk_get_in_ms_callback_def                    _sysclk_get_in_ms_callback = NULL;

static module_communication_init_callback_def           _module_communication_init = NULL;
static module_communication_shutdown_callback_def       _module_communication_shutdown = NULL;

static uint8_t _http_client_error_code_counter = 0;

static uint8_t _http_other_error_codes = 0;

static bool _timer_3346_msg_sent = false;

static bool _psm_configured = false;

static uint64_t _psm_print_timestamp = 0;

static uint16_t _http_code = 0;

static uint64_t _http_code_failed_timestamp = 0;

static uint64_t _http_other_bad_codes_timeout_values[HTTP_OTHER_CODES_ERROR_MAX] = {
  HTTP_OTHER_BAD_CODES_TIMEOUT_STARTING_VALUE_1,
  HTTP_OTHER_BAD_CODES_TIMEOUT_STARTING_VALUE_2,
  HTTP_OTHER_BAD_CODES_TIMEOUT_STARTING_VALUE_3,
  HTTP_OTHER_BAD_CODES_TIMEOUT_STARTING_VALUE_4,
  HTTP_OTHER_BAD_CODES_TIMEOUT_STARTING_VALUE_5
};

/* Power saving mode parameters */
static bool     _psm_on = false;
static uint8_t  _t3324_value = 0;
static uint8_t  _t3412_value = 0;

/* eDRX parameters */
static bool     _edrx_on = false;
static uint8_t  _edrx_period_value = 0;
static uint8_t  _ptw_period_value = 0;

static bool _gps_on = false;

static uint8_t _https_on = HTTPS_DEFAULT;

static bool _want_to_attach_data_session = false;

static uint64_t _last_registration_total_time = 0;

static uint64_t _power_on_number = 0;

static uint64_t _power_on_number_print_timestamp = 0;

static uint64_t _creg_searching_print_timestamp = 0;

static struct rssi_data _rssi_info;

static uint64_t _registration_failed_first_time_timestamp = 0;

static uint64_t _session_failed_first_time_timestamp = 0;

static uint64_t _http_failed_first_time_timestamp = 0;

static bool _http_action_sent = false;

static uint8_t _consecutive_noks_received = 0;

static SIMCOM_Module_Version _simcom_module_version_used = SIMCOM_UNKNOWN_VERSION;

static bool _current_registration_status_is_ok = true;

static bool _current_data_session_status_is_ok = true;

static bool _current_http_status_is_ok = true;

static bool _powering_on = false;

//TODO: Use this later. Too risky and I don't have time to test it now
static Power_On_Phases _power_on_status;

static bool _powering_off = false;

static Power_Off_Phases _power_off_status;

static uint64_t _power_on_off_timestamp = 0;

/**
 * @brief   Indicates the current cellular phase
 */
static cellular_Phases _current_phase;

/**
 * @brief   Indicates the current state of each phase
 */
static struct current_states _current_states;

/**
 * @brief   Indicates what i'm doing right now (e.g. sending or receiving)
 */
static Action_List _current_action;

/**
 * @brief   Buffer that stores the last command
 */
static char _last_command[COMMAND_SIZE];

/**
 * @brief   Detection of the command loopback on the buffer
 */
static bool _detected_command = false;

/**
 * @brief   A pointer to the expected response
 */
static char *_expected_response;

static char *_wrong_response;

/**
 * @brief   The number of attempts of the last executed command
 */
static uint16_t _command_attempts_number;

/**
 * @brief   The last timestamp that will be responsible of
 *          restarting the cellular module
 */
static uint64_t _cellular_module_restart_timestamp;

/**
 * @brief   The current timeout to restart the cellular module
 */
static uint64_t _cellular_module_restart_timeout;

/**
 * @brief   Used for cells collection
 */
static uint64_t _timestamp_cells;

/**
 * @brief   Timestamp of the last command tried
 */
static uint64_t _tx_command_unsuccessful_timestamp;

/**
 * @brief   The current timeout to force the last tx command
 */
static uint64_t _tx_command_unsuccessful_timeout;

/**
 * @brief   Records the timestamp of data session opening
 */
static uint64_t _timestamp_starting_data_session = 0;

/**
 * @brief   Timeout to remain data session open
 */
static bool _timeout_starting_data_session;

/**
 * @brief   The cellular info collected
 */
static struct cellular_data _cellular_data;

/**
 * @brief   The cells info collected
 */
static struct cells_data _cells_data;

/**
 * @brief   Indicates if it's time to collect cells info
 */
static bool _collect_cells;

/**
 * @brief   Indicates if it's time to collect GPS info
 */
static bool _collect_gps = false;

/**
 * @brief   Indicates when the module started to wait for new data to come
 */
static uint64_t http_starting_waiting_time_for_new_data = 0;

/**
 * @brief   Indicates if I should skip the rest of the downloaded data.
 */
static bool _skip_downloaded_data = false;

/**
 * @brief   Indicates if the cellular module is powered on or off
 */
static bool _powered_on;

static bool _nbiot_apn_configured = false;

/**
 * @brief   Tells if the data session is to remain open and how many time (in s).
 *            If it's equal to 0, it's always open
 */
static uint64_t _remain_data_session_open = cellular_REMAIN_DATA_SESSION_OPEN_DEFAULT;

/**
 * @brief   Save the timestamp of the last registration fail
 */
static uint64_t _registration_failed_timestamp = 0;

static uint64_t _session_failed_timestamp = 0;

/**
 * @brief   Tells the maximum time to retry a new network registration (in ms)
 */
static uint64_t _registration_failed_next_try_maximum_timeout = 
                                        REGISTRATION_FAILED_MAX_RETRY_TIMEOUT_DEFAULT;

static uint64_t _session_failed_next_try_maximum_timeout =
                                        SESSION_FAILED_MAX_RETRY_TIMEOUT_DEFAULT;

static uint16_t _machinates_version = MACHINATES_VERSION_DEFAULT;

/**
 * @brief   Tells the current time to retry a new network registration (in ms)
 */
static uint64_t _current_registration_failed_next_try_timeout = 0;

static uint64_t _current_session_failed_next_try_timeout = 0;

/**
 * @brief   Tells the time that the module took to register on the network (in ms)
 */
static uint64_t _last_time_to_register_on_the_network = 0;

/**
 * @brief   Tells if the time to register on the network was already saved or not
 */
static bool _time_to_register_on_the_network_collected = false;

/**
 * @brief   Tells if we have cells available
 */
static bool _registered_cell_on_network_collected = false;

static uint64_t _timer_timeout = 0;
static uint64_t _timer_timestamp = 0;

void cellular_set_phase(cellular_Phases new_phase, uint64_t current_timestamp) {
  _current_phase = new_phase;
  _last_time_phase_set_timestamp = current_timestamp;

  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
  debug_printf_string(DEBUG_LEVEL_2,
      (uint8_t *)"[cellular_set_phase] current phase set to %d\r\n", _current_phase);
}

uint64_t cellular_get_http_bad_code_next_try_timeout(void) {
  if ((_http_other_error_codes - 1) >= HTTP_OTHER_CODES_ERROR_MAX) {
    return _http_other_bad_codes_timeout_values[HTTP_OTHER_CODES_ERROR_MAX - 1];
  } else {
    return _http_other_bad_codes_timeout_values[(_http_other_error_codes - 1)];
  }
}

void cellular_enable_timer_callback(uint64_t timer_timeout_in_ms, uint64_t current_timestamp) {
  _timer_timeout = timer_timeout_in_ms;
  _timer_timestamp = current_timestamp;
}

void cellular_disable_timer_callback(void) {
  _timer_timeout = 0;
  _timer_timestamp = 0;
}

/**
 * @brief   Timeout expired handler
 */
void cellular_timer_expired(void) {
  cellular_disable_timer_callback();
  _current_action = SENDING_COMMAND;
}

/**
 * @brief   Tells the time to retry a new network registration (in ms)
 *            based on a incremental approach until reach the defined
 *            "_registration_failed_next_try_maximum_timeout"
 */
uint64_t cellular_get_registration_failed_next_try_timeout(void) {
  return _current_registration_failed_next_try_timeout;
}

uint64_t cellular_get_session_failed_next_try_timeout(void) {
  return _current_session_failed_next_try_timeout;
}

/**
 * @brief   Increments the time to retry a new network registration (in ms)
 *            based on a incremental approach until reach the defined
 *            "_registration_failed_next_try_maximum_timeout"
 */
void cellular_increment_registration_failed_next_try_timeout(
    uint64_t current_timestamp, uint64_t last_registration_timeout) {

  if (_current_registration_failed_next_try_timeout == 0) {
    if (last_registration_timeout > _registration_failed_next_try_maximum_timeout) {
      _current_registration_failed_next_try_timeout = _registration_failed_next_try_maximum_timeout;
    } else {
      _current_registration_failed_next_try_timeout = last_registration_timeout;
    }
  } else {
    if ((_current_registration_failed_next_try_timeout * 2) > _registration_failed_next_try_maximum_timeout) {
      _current_registration_failed_next_try_timeout = _registration_failed_next_try_maximum_timeout;
    } else {
      _current_registration_failed_next_try_timeout *= 2;
    }
  }

  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
  debug_print_string(DEBUG_LEVEL_2,
      (uint8_t *)"[cellular_increment_registration_failed_next_try_timeout] timeout is now: ");
  char timeout[10];
  memset(timeout, '\0', 10);
  utils_itoa(_current_registration_failed_next_try_timeout, timeout, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *) timeout);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *) "ms\r\n");
}

void cellular_increment_session_failed_next_try_timeout(uint64_t current_timestamp) {
  if (_current_session_failed_next_try_timeout == 0) {
    _current_session_failed_next_try_timeout = SESSION_TIMEOUT_STARTING_VALUE;
  } else {
    if ((_current_session_failed_next_try_timeout * 2) > _session_failed_next_try_maximum_timeout) {
      _current_session_failed_next_try_timeout = _session_failed_next_try_maximum_timeout;
    } else {
      _current_session_failed_next_try_timeout *= 2;
    }
  }

  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
  debug_print_string(DEBUG_LEVEL_2, 
      (uint8_t *)"[cellular_increment_session_failed_next_try_timeout] timeout is now: ");
  char timeout[10];
  memset(timeout, '\0', 10);
  utils_itoa(_current_session_failed_next_try_timeout, timeout, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)timeout);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"ms\r\n");
}

/**
 * @brief   Check if it's time to force a new tx to the module.
 *          This can happen when a "OK" response hasn't been received
 *          by the module so after a certain timeout we will force a new tx
 *          to know if the module is still there.
 */
bool cellular_is_tx_forcing_time(uint64_t current_timestamp) {
  bool ret = false;

  if (_tx_command_unsuccessful_timestamp != 0) {
    if (current_timestamp - _tx_command_unsuccessful_timestamp >= _tx_command_unsuccessful_timeout) {
      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      debug_print_string(DEBUG_LEVEL_2, 
          (uint8_t *)"[cellular_is_tx_forcing_time] forcing command tx activated (reason 1)\r\n");
      ret = true;
    }
  }

  if (_simcom_not_ready_to_rx_new_command_timestamp != 0) {
    if ((current_timestamp - _simcom_not_ready_to_rx_new_command_timestamp) >= cellular_MODULE_OK_TIMEOUT) {
      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      debug_print_string(DEBUG_LEVEL_2, 
          (uint8_t *)"[cellular_is_tx_forcing_time] forcing command tx activated (reason 2)\r\n");
      ret = true;
    }
  }

  return ret;
}

bool cellular_wakeup_from_psm(uint64_t current_timestamp) {
  if (_powered_on && _cellular_read_status_callback() && cellular_has_psm_active()) {
    debug_print_time(DEBUG_LEVEL_0, current_timestamp);
    debug_print_string(DEBUG_LEVEL_0, 
        (uint8_t *)"[cellular_wakeup_from_psm] wakeup from psm will starte now!\r\n");

    _cellular_set_power_key_callback();

    _psm_wakeup_now = true;
    _psm_wakeup_timestamp = rtc_get_milliseconds();
  } else {
    debug_print_time(DEBUG_LEVEL_0, current_timestamp);
    debug_print_string(DEBUG_LEVEL_0, 
        (uint8_t *)"[cellular_wakeup_from_psm] read status is on now so don't need to wakeup from psm\r\n");

    _psm_is_currently_active = false;
  }

  return true;
}

/**
 * @brief   Initializes all the internal variables
 */
void cellular_reset_variables(void) {
  memset(_cellular_data.downloaded_data, '\0', RX_BUFFER_SIZE);
  memset(_last_command, '\0', COMMAND_SIZE);
  cellular_set_phase(IDLE_PHASE, _sysclk_get_in_ms_callback());
  _current_states.warmup = WARMUP_IDLE_STATE;
  _current_states.info = INFO_IDLE_STATE;
  _current_states.data_session = DATA_SESSION_IDLE_STATE;
  _current_states.http = HTTP_IDLE_STATE;
  _current_states.cells = CELLS_IDLE_STATE;
  _current_states.gps = GPS_IDLE_STATE;
  _current_action = IDLE;
  _expected_response = NULL;
  _wrong_response = NULL;
  _cellular_data.has_data_to_read = false;
  _cellular_data.is_time_to_set_data_to_upload = false;
  _cellular_data.data_successfully_uploaded = false;
  _cellular_data.has_downloaded_data = false;
  _cellular_data.downloaded_data_last_read_size = 0;
  _cellular_data.downloaded_data_total_size = 0;
  _cellular_data.downloaded_data_total_read_size = 0;
  _skip_downloaded_data = false;
  _cellular_data.warmup_success = false;
  _cellular_data.info_success = false;
  _last_time_phase_set_timestamp = _sysclk_get_in_ms_callback();
  _cellular_data.http_init = false;
  _cellular_data.http_context_saved = false;
  _detected_command = false;
  _cellular_data.reg_status = 0;
  _cellular_data.rssi = 0;
  _cellular_data.ber = 0;
  _cellular_data.imsi = 0;
  _cellular_data.data_session_status = 0;
  _cellular_data.data_session_attached_status = 0;
  _cells_data.available = false;
  _timeout_starting_data_session = false;
  _timestamp_starting_data_session = 0;
  _tx_command_unsuccessful_timestamp = 0;
  _tx_command_unsuccessful_timeout = 0;
  _timestamp_cells = 0;
  /* Just to force initial power off */
  _powered_on = true;
  _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
  simcom_reset_variables();
  _cellular_data.has_data_to_upload = false;
  _want_to_attach_data_session = false;
  _cellular_data.https_set = false;
  _http_code = 0;
  _wakeup_from_psm_by_cells = false;
  _gps_is_already_collecting = false;
}

void cellular_start_power_on(uint64_t current_timestamp) {
  if (!_powering_on && !_powering_off) {
    debug_print_time(DEBUG_LEVEL_1, current_timestamp);
    debug_print_string(DEBUG_LEVEL_1, 
        (uint8_t *)"[cellular_start_power_on] power on will begin asap\r\n");

    _power_on_status = POWER_ON_BEGINNING;
    _powering_on = true;
  }
}

void cellular_start_power_off(uint64_t current_timestamp) {
  if (!_powering_off && !_powering_on) {
    debug_print_time(DEBUG_LEVEL_1, current_timestamp);
    debug_print_string(DEBUG_LEVEL_1, 
        (uint8_t *)"[cellular_start_power_off] power off will begin asap\r\n");

    _power_off_status = POWER_OFF_BEGINNING;
    _powering_off = true;
  }
}

/**
 * @brief       Power on cellular module
 * @details     Set POWER_KEY pin for one second and set VBAT_CTRL pin.
 */
//  void cellular_power_on(uint64_t current_timestamp) {
//    debug_print_time(DEBUG_LEVEL_1, current_timestamp);
//    debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[cellular_power_on] power on begin\r\n");
//
//    /* Status off? */
//    if (_cellular_read_status_callback()) {
//      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
//      debug_print_string(DEBUG_LEVEL_2,
//          (uint8_t*) "[cellular_power_on] status off so let's do power on procedure!\r\n");
//
//      //TODO: Remove blocking delays!
//      _cellular_set_vbat_ctrl_callback();
//      _vbat_ctrl_current_state = true;
//
//      _cellular_clear_power_key_callback();
//
//      _sysclk_delay_in_ms_callback(1000);
//
//      _cellular_set_power_key_callback();
//
//      _sysclk_delay_in_ms_callback(2000);
//
//      _cellular_clear_power_key_callback();
//
//      _cellular_set_lvl_sh_oe_callback();
//
//      /* While status remains off */
//      while (_cellular_read_status_callback()) {
//        ;
//      }
//    } else {
//      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
//      debug_print_string(DEBUG_LEVEL_2,
//          (uint8_t*) "[cellular_power_on] status on so skip power on procedure!\r\n");
//    }
//
//    debug_print_time(DEBUG_LEVEL_1, current_timestamp);
//    debug_print_string(DEBUG_LEVEL_1,
//        (uint8_t*) "[cellular_power_on] power on done!\r\n");
//
//    _power_on_number++;
//
//    _module_communication_init();
//
//    _powered_on = true;
//    _nbiot_apn_configured = false;
//    _psm_configured = false;
//    _http_action_sent = false;
//
//    /* Clear flags */
//    _time_to_register_on_the_network_collected = false;
//    _registered_cell_on_network_collected = false;
//
//    /* Register the timestamp */
//    _last_time_to_register_on_the_network = _sysclk_get_in_ms_callback();
//  }

//TODO: Use this later. Too risky and I don't have time to test it now
void cellular_power_on_loop(uint64_t current_timestamp) {
  switch (_power_on_status) {
    case POWER_ON_BEGINNING: {
      if (!_cellular_read_status_callback()) {
        debug_print_time(DEBUG_LEVEL_1, current_timestamp);
        debug_print_string(DEBUG_LEVEL_1, 
            (uint8_t *)"[cellular_power_on] power on started; status on so skip power on procedure\r\n");
  
        _power_on_status = POWER_ON_FINISHING;
      } else {
        debug_print_time(DEBUG_LEVEL_1, current_timestamp);
        debug_print_string(DEBUG_LEVEL_1, 
            (uint8_t *)"[cellular_power_on] power on started; status off so let's do power on procedure\r\n");
  
        _cellular_set_vbat_ctrl_callback();
        _vbat_ctrl_current_state = true;
  
        _cellular_clear_power_key_callback();
  
        _power_on_off_timestamp = current_timestamp;
        _power_on_status = POWER_ON_SET_VBAT_CTRL;
      }
  
      break;
    }
    case POWER_ON_SET_VBAT_CTRL: {
      if ((current_timestamp - _power_on_off_timestamp) >= SET_VBAT_CTRL_POWER_ON_TIMEOUT) {
        _cellular_set_power_key_callback();
  
        _power_on_off_timestamp = current_timestamp;
        _power_on_status = POWER_ON_CLEAR_POWER_KEY;
      }
  
      break;
    }
    case POWER_ON_CLEAR_POWER_KEY: {
      if ((current_timestamp - _power_on_off_timestamp) >= SET_POWER_KEY_POWER_ON_TIMEOUT) {
        _cellular_clear_power_key_callback();
        _cellular_set_lvl_sh_oe_callback();
  
        _power_on_status = POWER_ON_WAIT_FOR_STATUS;
      }
  
      break;
    }
    case POWER_ON_WAIT_FOR_STATUS: {
      if (!_cellular_read_status_callback()) {
        _power_on_status = POWER_ON_FINISHING;
      }
  
      break;
    }
    case POWER_ON_FINISHING: {
      _power_on_number++;
  
      _module_communication_init();
  
      _powered_on = true;
      _powering_on = false;
      _nbiot_apn_configured = false;
      _psm_configured = false;
      _http_action_sent = false;
      _psm_is_currently_active = false;
  
      _power_on_off_timestamp = current_timestamp;
  
      /* Clear flags */
      _time_to_register_on_the_network_collected = false;
      _registered_cell_on_network_collected = false;
      _psm_wakeup_now = false;
      _psm_wakeup_timestamp = 0;
  
      /* Register the timestamp */
      _last_time_to_register_on_the_network = _sysclk_get_in_ms_callback();
  
      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_power_on] power on ended\r\n");
  
      break;
    }
    default: {
      break;
    }
  }
}

/**
 * @brief       Power off cellular module
 * @details     Set POWER_KEY pin for one second and clear VBAT_CTRL pin.
 */
//void cellular_power_off(uint64_t current_timestamp) {
//  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
//  debug_print_string(DEBUG_LEVEL_1,
//      (uint8_t*) "[cellular_power_off] power off begin\r\n");
//
//  _module_communication_shutdown();
//
//  /* Try to do this first only to
//   * discharge the capacitors */
//  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
//  debug_print_string(DEBUG_LEVEL_2,
//      (uint8_t*) "[cellular_power_off] prevention; beginning to discharge capacitors!!!\r\n");
//
//  /* Status off? */
//  if (_cellular_read_status_callback()) {
//    _cellular_clear_vbat_ctrl_callback();
//    _vbat_ctrl_current_state = false;
//
//    /* While status remains on */
//    while (!_cellular_read_status_callback()) {
//      ;
//    }
//  } else {
//    _cellular_clear_power_key_callback();
//    _sysclk_delay_in_ms_callback(1000);
//    _cellular_set_power_key_callback();
//    _sysclk_delay_in_ms_callback(2000);
//    _cellular_clear_power_key_callback();
//
//    _cellular_clear_vbat_ctrl_callback();
//    _vbat_ctrl_current_state = false;
//
//    /* While status remains on */
//    while (!_cellular_read_status_callback()) {
//      ;
//    }
//  }
//
//  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
//  debug_print_string(DEBUG_LEVEL_2,
//      (uint8_t*) "[cellular_power_off] prevention; ending to discharge capacitors!!!\r\n");
//
//  /* Status on? */
//  if (!_cellular_read_status_callback()) {
//      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
//      debug_print_string(DEBUG_LEVEL_2,
//          (uint8_t*) "[cellular_power_off] what?! status is still on!\r\n");
//
//      //TODO: Remove blocking delays!
//      _cellular_clear_power_key_callback();
//
//      _sysclk_delay_in_ms_callback(1000);
//
//      _cellular_set_power_key_callback();
//
//      _sysclk_delay_in_ms_callback(1000);
//
//      _cellular_clear_power_key_callback();
//
//      _sysclk_delay_in_ms_callback(1000);
//      _cellular_clear_lvl_sh_oe_callback();
//      _cellular_clear_vbat_ctrl_callback();
//      _vbat_ctrl_current_state = false;
//
//      /* While status remains on */
//      while (!_cellular_read_status_callback()) {
//        ;
//      }
//  } else {
//    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
//    debug_print_string(DEBUG_LEVEL_2,
//        (uint8_t*) "[cellular_power_off] clear pins!\r\n");
//
//    _cellular_clear_power_key_callback();
//    _cellular_clear_lvl_sh_oe_callback();
//    _cellular_clear_vbat_ctrl_callback();
//    _vbat_ctrl_current_state = false;
//
//    /* Delay just in case... */
//    _sysclk_delay_in_ms_callback(1000);
//  }
//
////  simcom_shutdown(SIMCOM_UART_IRQ);
////  cellular_set_power_key();
////  sysclk_delay(1);
////  cellular_clear_power_key();
////  sysclk_delay(1);
////  cellular_clear_lvl_sh_oe();
////  cellular_clear_vbat_ctrl();
//
//  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
//  debug_print_string(DEBUG_LEVEL_1,
//      (uint8_t*) "[cellular_power_off] power off done!\r\n");
//
//  _cellular_data.reg_status = 0;
//  _powered_on = false;
//}

//TODO: Use this later. Too risky and I don't have time to test it now
/**
 * @brief     Power off cellular module
 * @details   Set POWER_KEY pin for one second and clear VBAT_CTRL pin.
 */
void cellular_power_off_loop(uint64_t current_timestamp) {
  switch (_power_off_status) {
    case POWER_OFF_BEGINNING: {
      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, 
          (uint8_t *)"[cellular_power_off] power off started; start to discharge capacitors just for prevention\r\n");
  
      _module_communication_shutdown();
  
      /* Status off? */
      if (_cellular_read_status_callback()) {
        _power_off_status = POWER_OFF_CLEAR_VBAT_CTRL;
      } else {
        _cellular_clear_power_key_callback();
  
        _power_on_off_timestamp = current_timestamp;
        _power_off_status = POWER_OFF_SET_POWER_KEY;
      }
  
      break;
    }
    case POWER_OFF_SET_POWER_KEY: {
      if ((current_timestamp - _power_on_off_timestamp) >= CLEAR_POWER_KEY_POWER_OFF_TIMEOUT) {
        _cellular_set_power_key_callback();
  
        _power_on_off_timestamp = current_timestamp;
        _power_off_status = POWER_OFF_CLEAR_POWER_KEY;
      }
  
      break;
    }
    case POWER_OFF_CLEAR_POWER_KEY: {
      if ((current_timestamp - _power_on_off_timestamp) >= SET_POWER_KEY_POWER_OFF_TIMEOUT) {
        _cellular_clear_power_key_callback();
  
        _power_on_off_timestamp = current_timestamp;
        _power_off_status = POWER_OFF_CLEAR_VBAT_CTRL;
      }
  
      break;
    }
    case POWER_OFF_CLEAR_VBAT_CTRL: {
      _cellular_clear_vbat_ctrl_callback();
      _vbat_ctrl_current_state = false;
  
      _power_off_status = POWER_OFF_WAIT_FOR_STATUS;
  
      break;
    }
  
    case POWER_OFF_WAIT_FOR_STATUS: {
      if (_cellular_read_status_callback()) {
        _power_on_off_timestamp = current_timestamp;
        _power_off_status = POWER_OFF_CLEAR_PINS;
  
        debug_print_time(DEBUG_LEVEL_1, current_timestamp);
        debug_print_string(DEBUG_LEVEL_1, 
            (uint8_t *)"[cellular_power_off] ending to discharge capacitors\r\n");
      }
      break;
    }
  
    case POWER_OFF_CLEAR_PINS: {
      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_power_off] clear pins\r\n");
  
      _cellular_clear_power_key_callback();
      _cellular_clear_lvl_sh_oe_callback();
      _cellular_clear_vbat_ctrl_callback();
      _vbat_ctrl_current_state = false;
  
      _power_off_status = POWER_OFF_CLEAR_PINS_WAITING_TIME;
      _power_on_off_timestamp = current_timestamp;
    }
  
    case POWER_OFF_CLEAR_PINS_WAITING_TIME: {
      if ((current_timestamp - _power_on_off_timestamp) >= CLEAR_PINS_WAITING_TIMEOUT) {
        _power_off_status = POWER_OFF_FINISHING;
        _power_on_off_timestamp = current_timestamp;
      }
    }
  
    case POWER_OFF_FINISHING: {
      _cellular_data.reg_status = 0;
      _powered_on = false;
      _powering_off = false;
      _power_on_off_timestamp = current_timestamp;
  
      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_power_off] power off ended\r\n");
  
      break;
    }
  
    default: {
      break;
    }
  }
}

/**
 * @brief   cellular new command sending procedure
 * @param   command
 *            New command to send.
 *          expected_response
 *            The expected response.
 *          timeout_for_expire_in_seconds
 *            Timeout in seconds to receive the expected response for the command.
 *          timeout_for_retries_in_ms
 *            Timeout to retry until the timeout_for_expire_in_seconds has been expired.
 *          force
 *            Force to send the new command.
 */
void cellular_send_new_command(char *command, char *expected_response,
    uint64_t timeout_for_expire_in_ms, uint64_t timeout_for_retries_in_ms,
    bool force, uint64_t current_timestamp, char *wrong_response) {
  /* Command first time */
  if (_command_attempts_number == 0) {
    char command_str_without_cr[strlen(command)];
    memset(command_str_without_cr, '\0', strlen(command));
    memcpy(command_str_without_cr, command, (strlen(command) - 1));

    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    debug_print_string(DEBUG_LEVEL_2, 
        (uint8_t *)"[cellular_send_new_command] first attempt for this command tx (");
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)command_str_without_cr);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)")\r\n");

    _cellular_module_restart_timestamp = _sysclk_get_in_ms_callback();
    _cellular_module_restart_timeout = timeout_for_expire_in_ms;

    _tx_command_unsuccessful_timestamp = current_timestamp;
    _tx_command_unsuccessful_timeout = timeout_for_retries_in_ms * NUMBER_OF_COMMANDS_TX_TO_FORCE;
  }

  if (simcom_ready_to_rx_new_command() || force || cellular_is_tx_forcing_time(current_timestamp)) {
    _simcom_not_ready_to_rx_new_command_timestamp = 0;

    if (_command_attempts_number > 1) {
      char command_str_without_cr[strlen(command)];
      memset(command_str_without_cr, '\0', strlen(command));
      memcpy(command_str_without_cr, command, (strlen(command) - 1));

      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_send_new_command] another tx command (");
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)command_str_without_cr);
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)")\r\n");
    }

    _tx_command_unsuccessful_timestamp = current_timestamp;

    _expected_response = expected_response;
    _wrong_response = wrong_response;
    _current_action = WAITING_RESPONSE;

    /* Store last command */
    memset(_last_command, '\0', COMMAND_SIZE);
    /* Command size to large for the buffer. No problem... let's skip it! */
    if (strlen(command) <= COMMAND_SIZE) {
      memcpy(_last_command, command, strlen(command));
      _detected_command = false;
    } else {
      _detected_command = true;
    }

    /* In the HTTP_STORE_DATA state when I send the response containing the data to
    upload to Machinates, this data will not be loopbacked to me, so it's an exception! */
    if ((_current_phase == HTTP_PHASE) && (_current_states.http == HTTP_STORE_DATA_STATE)) {
      _detected_command = true;
    }

    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    simcom_send_command(command, strlen(command), true);
    _command_attempts_number++;

    /* Enable timeout timer */
    if (timeout_for_retries_in_ms > 0) {
      cellular_enable_timer_callback(timeout_for_retries_in_ms,
          current_timestamp);
    }
    /* else if not (simcom_ready_to_rx_new_command() || force || cellular_is_tx_forcing_time(current_timestamp)) */
  } else {
    if ((_simcom_not_ready_to_rx_new_command_timestamp == 0) && (!simcom_ready_to_rx_new_command())) {
      _simcom_not_ready_to_rx_new_command_timestamp = current_timestamp;
    }

    if ((current_timestamp - _last_time_tx_unsuccessfully_done_print) >= 1000) {
      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      debug_print_string(DEBUG_LEVEL_2, 
          (uint8_t *)"[cellular_send_new_command] failed command tx. details: simcom_ready_to_rx_new_command? ");
      if (simcom_ready_to_rx_new_command()) {
        debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"true; force? ");
      } else {
        debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"false; force? ");
      }
      if (force) {
        debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"true; cellular_is_tx_forcing_time? ");
      } else {
        debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"false; cellular_is_tx_forcing_time? ");
      }
      if (cellular_is_tx_forcing_time(current_timestamp)) {
        debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"true\r\n");
      } else {
        debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"false\r\n");
      }

      _last_time_tx_unsuccessfully_done_print = current_timestamp;
    }
  }
}

void cellular_process_pending_messages_task(uint64_t current_timestamp) {
  /* Timeout expired! */
  if ( (_cellular_module_restart_timestamp != 0) && 
       ((current_timestamp - _cellular_module_restart_timestamp) > _cellular_module_restart_timeout) ) {
    if (_current_states.warmup == WARMUP_IS_REGISTERED_STATE) {
      _last_registration_total_time = (current_timestamp - _cellular_module_restart_timestamp);

      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      debug_print_string(DEBUG_LEVEL_2, 
          (uint8_t *)"[cellular_process_pending_messages_task] save last registration total time frame\r\n");
    }

    debug_print_time(DEBUG_LEVEL_0, current_timestamp);
    debug_print_string(DEBUG_LEVEL_0, 
        (uint8_t *)"[cellular_process_pending_messages_task] bad response detected\r\n");

    _current_action = BAD_RESPONSE;
  }
  /* I'm waiting for a response. */
  if (_current_action == WAITING_RESPONSE) {
    /* I got one in the module. Let's process it! */
    if (simcom_how_many_new_responses() > 0) {
      struct simcom_responses received_reponses;
      simcom_read_responses(&received_reponses);

      /* Detected the command loopback */
      if (strstr(received_reponses.responses, _last_command) != NULL) {
        _detected_command = true;
      }

      /* It was detected the right answer to my command */
      if ( _detected_command && 
           (strstr(received_reponses.responses, _expected_response) != NULL) ) {
        _current_action = GOOD_RESPONSE;
      }
      //  else if ( _detected_command && (_current_phase == CELLS_PHASE) && 
      //            (_current_states.cells == CELLS_WAKE_UP_FROM_PSM_STATE_1) ) {
      //
      //    _current_action = GOOD_RESPONSE;
      //    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      //    debug_print_string(DEBUG_LEVEL_2, 
      //        (uint8_t*) "[cellular_process_pending_messages_task] workaround for exiting quickly psm\r\n");
      //  }
      /* It was detected the wrong answer to my command */
      else if (_detected_command && (strstr(received_reponses.responses, _wrong_response) != NULL)) {
        _current_action = ERROR_RESPONSE;

        debug_print_time(DEBUG_LEVEL_2, current_timestamp);
        debug_print_string(DEBUG_LEVEL_2, 
            (uint8_t *)"[cellular_process_pending_messages_task] wrong message detected for this command\r\n");
      }
    }
  }
}

/**
 * @brief   The function that will run the warm-up phase state machine
 */
void cellular_warm_up_state_machine(uint64_t current_timestamp) {
  switch (_current_states.warmup) {
    case WARMUP_IDLE_STATE: {
      break;
    }
  
    case WARMUP_STARTING_STATE: {
      _current_states.warmup = WARMUP_AT_TEST_STATE;
      _current_action = SENDING_COMMAND;
      _command_attempts_number = 0;
  
      /* Just disable it until next command tx */
      _tx_command_unsuccessful_timestamp = 0;
  
      break;
    }
  
    case WARMUP_AT_TEST_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(AT_TEST_COMMAND(_simcom_module_version_used),
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_5S, COMMAND_RETRY_TIME_500MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        if (_simcom_module_version_used == SIMCOM_UNKNOWN_VERSION) {
          cellular_disable_timer_callback();
  
          _current_states.warmup = WARMUP_GET_GMR_PHASE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        } else {
          if ( (_simcom_module_version_used == SIMCOM_7020_VERSION) || 
               (_simcom_module_version_used == SIMCOM_7080_VERSION) ) {
            //    /* Get last completed transaction */
            //    struct simcom_responses last_completed_transaction;
            //    simcom_get_last_completed_transaction(_last_command,
            //        &last_completed_transaction);
            //
            //    if (last_completed_transaction.responses_size > 0) {
            //      char* temp;
            //      temp = strstr(last_completed_transaction.responses, TIMER_3346_RESPONSE);
            //
            //      if (temp != NULL) {
            //        temp = strchr(temp, ' ') + 1;
            //
            //        uint64_t timer_3346 = utils_atoll(temp);
            //
            //        if (!_timer_3346_msg_sent && timer_3346 != 0) {
            //          if(timer_3346_create_message(current_timestamp, timer_3346)) {
            //            _timer_3346_msg_sent = true;
            //          }
            //        }
            //      }
            //    }
  
            cellular_disable_timer_callback();
  
            if (!_nbiot_apn_configured) {
              _current_states.warmup = WARMUP_SET_APN_NBIOT_FIND_IMSI_STATE_PHASE_1;
            } else {
              _current_states.warmup = WARMUP_IS_REGISTERED_STATE;
            }
  
            _current_action = SENDING_COMMAND;
            _command_attempts_number = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
            /* else if not ( (_simcom_module_version_used == SIMCOM_7020_VERSION) || (_simcom_module_version_used == SIMCOM_7080_VERSION) ) */
          } else {
            cellular_disable_timer_callback();
  
            _current_states.warmup = WARMUP_IS_REGISTERED_STATE;
            _current_action = SENDING_COMMAND;
            _command_attempts_number = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
          }
        }
      }
      break;
    }
  
    case WARMUP_GET_GMR_PHASE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(GET_GMR_COMMAND, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S, COMMAND_RETRY_TIME_1000MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command, &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          if (strlen(_cellular_data.simcom_version) == 0) {
            char *simcom_version;
            simcom_version = strstr(last_completed_transaction.responses, GET_GMR_REVISION);
            char *simcom_version_2;
            simcom_version_2 = strstr(last_completed_transaction.responses, GET_GMR_RESPONSE);
  
            if (simcom_version != NULL) {
              simcom_version = strchr(simcom_version, ':') + 1;
  
              memset(&(_cellular_data.simcom_version), '\0', SIMCOM_VERSION_MAX_LENGTH);
  
              if (cellular_utilities_number_of_chars_until_cr(simcom_version) <= SIMCOM_VERSION_MAX_LENGTH) {
                memcpy(&(_cellular_data.simcom_version), simcom_version,
                    cellular_utilities_number_of_chars_until_cr(simcom_version));
  
                debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                debug_print_string(DEBUG_LEVEL_2, 
                    (uint8_t *)"[cellular_warm_up_state_machine] SIMCOM firmware version stored: ");
                debug_print_string(DEBUG_LEVEL_2, (uint8_t *)_cellular_data.simcom_version);
                debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\r\n");
              }
            } else if (simcom_version_2 != NULL) {
              simcom_version_2 = strchr(simcom_version_2, ' ') + 1;
  
              memset(&(_cellular_data.simcom_version), '\0', SIMCOM_VERSION_MAX_LENGTH);
  
              if (cellular_utilities_number_of_chars_until_cr(simcom_version_2) <= SIMCOM_VERSION_MAX_LENGTH) {
                memcpy(&(_cellular_data.simcom_version),
                    simcom_version_2, cellular_utilities_number_of_chars_until_cr(simcom_version_2) );
  
                debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                debug_print_string(DEBUG_LEVEL_2, 
                    (uint8_t *)"[cellular_warm_up_state_machine] SIMCOM firmware version stored: ");
                debug_print_string(DEBUG_LEVEL_2, (uint8_t *)_cellular_data.simcom_version);
                debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\r\n");
              }
            } else {
              simcom_version = strstr(last_completed_transaction.responses, GET_GMR_COMMAND);
  
              if (simcom_version != NULL) {
                simcom_version = strchr(simcom_version, '\n') + 1;
  
                memset(&(_cellular_data.simcom_version), '\0',  SIMCOM_VERSION_MAX_LENGTH);
  
                if (cellular_utilities_number_of_chars_until_cr(simcom_version) <= SIMCOM_VERSION_MAX_LENGTH) {
                  memcpy( &(_cellular_data.simcom_version), simcom_version,
                      cellular_utilities_number_of_chars_until_cr(simcom_version) );
  
                  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                  debug_print_string(DEBUG_LEVEL_2, 
                      (uint8_t *)"[cellular_warm_up_state_machine] SIMCOM firmware version stored: ");
                  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)_cellular_data.simcom_version);
                  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\r\n");
                }
              }
            }
          }
  
          char *temp_simcom7020;
          temp_simcom7020 = strstr(last_completed_transaction.responses, SIMCOM_7020_MODEL_STRING);
          char *temp_simcom7080;
          temp_simcom7080 = strstr(last_completed_transaction.responses, SIMCOM_7080_MODEL_STRING);
          char *temp_simcom800;
          temp_simcom800 = strstr(last_completed_transaction.responses, SIMCOM_800_MODEL_STRING);
          char *temp_simcom7600;
          temp_simcom7600 = strstr(last_completed_transaction.responses, SIMCOM_7600_MODEL_STRING);
          char *temp_simcom868;
          temp_simcom868 = strstr(last_completed_transaction.responses, SIMCOM_868_MODEL_STRING);
  
          if (temp_simcom800 != NULL) {
            _simcom_module_version_used = SIMCOM_800_VERSION;
  
            debug_print_time(DEBUG_LEVEL_0, current_timestamp);
            debug_print_string(DEBUG_LEVEL_0, 
                (uint8_t *)"[cellular_warm_up_state_machine] SIMCOM800 detected\r\n");
          } else if (temp_simcom7020 != NULL) {
            _simcom_module_version_used = SIMCOM_7020_VERSION;
  
            debug_print_time(DEBUG_LEVEL_0, current_timestamp);
            debug_print_string(DEBUG_LEVEL_0, 
                (uint8_t *)"[cellular_warm_up_state_machine] SIMCOM7020 detected\r\n");
          } else if (temp_simcom7080 != NULL) {
            _simcom_module_version_used = SIMCOM_7080_VERSION;
  
            debug_print_time(DEBUG_LEVEL_0, current_timestamp);
            debug_print_string(DEBUG_LEVEL_0, 
                (uint8_t *)"[cellular_warm_up_state_machine] SIMCOM7080 detected\r\n");
          } else if (temp_simcom7600 != NULL) {
            _simcom_module_version_used = SIMCOM_7600_VERSION;
  
            debug_print_time(DEBUG_LEVEL_0, current_timestamp);
            debug_print_string(DEBUG_LEVEL_0, 
                (uint8_t *)"[cellular_warm_up_state_machine] SIMCOM7600 detected\r\n");
          } else if (temp_simcom868 != NULL) {
            _simcom_module_version_used = SIMCOM_868_VERSION;
  
            debug_print_time(DEBUG_LEVEL_0, current_timestamp);
            debug_print_string(DEBUG_LEVEL_0, 
                (uint8_t *)"[cellular_warm_up_state_machine] SIMCOM868 detected\r\n");
          } else {
            debug_print_time(DEBUG_LEVEL_0, current_timestamp);
            debug_print_string(DEBUG_LEVEL_0, 
                (uint8_t *)"[cellular_warm_up_state_machine] unknown SIMCOM module\r\n");
          }
  
          if ( (_simcom_module_version_used == SIMCOM_7020_VERSION) || 
               (_simcom_module_version_used == SIMCOM_7080_VERSION) ) {
            cellular_disable_timer_callback();
  
            if (!_nbiot_apn_configured) {
              _current_states.warmup = WARMUP_SET_APN_NBIOT_FIND_IMSI_STATE_PHASE_1;
            } else {
              _current_states.warmup = WARMUP_IS_REGISTERED_STATE;
            }
            _current_action = SENDING_COMMAND;
            _command_attempts_number = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
          } else {
            cellular_disable_timer_callback();
  
            _current_states.warmup = WARMUP_IS_REGISTERED_STATE;
            _current_action = SENDING_COMMAND;
            _command_attempts_number = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
          }
        }
      }
      break;
    }
  
    case WARMUP_IS_REGISTERED_STATE: {
      if (_current_action == SENDING_COMMAND) {
        if (_registration_failed_first_time_timestamp == 0) {
          _registration_failed_first_time_timestamp = current_timestamp;
        }
  
        char *temp;
        temp = strstr(
            cellular_utilities_get_band((_cellular_data.imsi / 1000000000000),
                ((_cellular_data.imsi % 1000000000000) / 10000000000),
                _simcom_module_version_used),
            BAND_MANUAL_SELECTION_DEFAULT);
  
        if (temp != NULL) { /* currently this if means SIMCOM_7020_VERSION or SIMCOM_7080_VERSION */
          debug_print_time(DEBUG_LEVEL_2, current_timestamp);
          debug_print_string(DEBUG_LEVEL_2, 
              (uint8_t *)"[cellular_warm_up_state_machine] registration timeout is 1h\r\n");
  
          _last_registration_timeout = RESPONSE_WAITING_TIME_3600S;
  
          cellular_send_new_command(
              IS_REGISTERED_COMMAND(_simcom_module_version_used),
              IS_REGISTERED_RESPONSE(_simcom_module_version_used),
              _last_registration_timeout,
              COMMAND_RETRY_TIME_5000MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        } else { /* currently this else means not (SIMCOM_7020_VERSION or SIMCOM_7080_VERSION) */
          if ( (_simcom_module_version_used == SIMCOM_800_VERSION)  || 
               (_simcom_module_version_used == SIMCOM_7600_VERSION) || 
               (_simcom_module_version_used == SIMCOM_868_VERSION) ) {
            debug_print_time(DEBUG_LEVEL_2, current_timestamp);
            debug_print_string(DEBUG_LEVEL_2, 
                (uint8_t *)"[cellular_warm_up_state_machine] registration timeout is 15min\r\n");
  
            _last_registration_timeout = RESPONSE_WAITING_TIME_900S;
  
            cellular_send_new_command(
                IS_REGISTERED_COMMAND(_simcom_module_version_used),
                IS_REGISTERED_RESPONSE(_simcom_module_version_used),
                _last_registration_timeout,
                COMMAND_RETRY_TIME_5000MS,
                false, current_timestamp, WRONG_RESPONSE_DEFAULT);
          } else { /* Precaution else, case missing a SIMCOM version */
            debug_print_time(DEBUG_LEVEL_2, current_timestamp);
            debug_print_string(DEBUG_LEVEL_2, 
                (uint8_t *)"[cellular_warm_up_state_machine] registration timeout is 1h\r\n");
  
            _last_registration_timeout = RESPONSE_WAITING_TIME_3600S;
  
            cellular_send_new_command(
                IS_REGISTERED_COMMAND(_simcom_module_version_used),
                IS_REGISTERED_RESPONSE(_simcom_module_version_used),
                _last_registration_timeout,
                COMMAND_RETRY_TIME_5000MS,
                false, current_timestamp, WRONG_RESPONSE_DEFAULT);
          }
        }
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command,
            &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
          temp = strstr(last_completed_transaction.responses,
              IS_REGISTERED_RESPONSE(_simcom_module_version_used));
  
          uint8_t aux = 0;
  
          if (temp != NULL) {
            temp = strchr(temp, ' ') + 1;
  
            if (_psm_configured) {
              aux = utils_atoll(temp);
            }
  
            temp = strchr(temp, ',') + 1;
            _cellular_data.reg_status = utils_atoll(temp);
  
            if ((current_timestamp - _creg_searching_print_timestamp) > 5000) {
              debug_print_time(DEBUG_LEVEL_2, current_timestamp);
              debug_print_string(DEBUG_LEVEL_2, 
                  (uint8_t *)"[cellular_warm_up_state_machine] time to registration reset: ");
              char xpto[6];
              memset(xpto, '\0', 6);
              utils_itoa(
                  (_cellular_module_restart_timeout - (current_timestamp - _cellular_module_restart_timestamp)),
                  xpto, 10);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)xpto);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"ms\r\n");
  
              debug_print_time(DEBUG_LEVEL_2, current_timestamp);
              debug_print_string(DEBUG_LEVEL_2, 
                  (uint8_t *)"[cellular_warm_up_state_machine] last registration time was: ");
              memset(xpto, '\0', 6);
              utils_itoa(_last_registration_total_time, xpto, 10);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)xpto);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"s\r\n");
  
              _creg_searching_print_timestamp = current_timestamp;
            }
  
            if ( (_cellular_data.reg_status == REGISTERED_ROAMING || _cellular_data.reg_status == REGISTERED_HOME_NETWORK) && 
                 (!_psm_configured || (_psm_configured && (aux == 4))) ) {
              if ( (_registration_failed_first_time_timestamp != 0) && 
                   ((current_timestamp - _registration_failed_first_time_timestamp) >= cellular_REGISTRATION_TIMEOUT_SEND_FAILURE_MSG)) {
                _registration_failure_create_message_callback(
                    current_timestamp,
                    ((current_timestamp - _registration_failed_first_time_timestamp) / 1000));
              }
              _registration_failed_first_time_timestamp = 0;
  
              _timer_3346_msg_sent = false;
  
              /* Update flag */
              if (!_current_registration_status_is_ok) {
                _current_registration_status_is_ok = true;
              }
  
              cellular_disable_timer_callback();
  
              _current_states.warmup = WARMUP_GET_CSQ_STATE;
              _current_action = SENDING_COMMAND;
              _command_attempts_number = 0;
  
              /* Just disable it until next command tx */
              _tx_command_unsuccessful_timestamp = 0;
  
              /* Register the timeout that took the module to be registered */
              if (!_time_to_register_on_the_network_collected) {
                _last_time_to_register_on_the_network = current_timestamp - _last_time_to_register_on_the_network;
                _time_to_register_on_the_network_collected = true;
              }
            } else {
              if (_cellular_data.reg_status == NOT_SEARCHING_HOME_NETWORK) {
                ; // Doing nothing
              }
            }
          }
        }
      }
      break;
    }
  
    case WARMUP_GET_CSQ_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(GET_CSQ_COMMAND, GET_CSQ_RESPONSE,
            RESPONSE_WAITING_TIME_60S, COMMAND_RETRY_TIME_1000MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command, &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
          temp = strstr(last_completed_transaction.responses, GET_CSQ_RESPONSE);
  
          if (temp != NULL) {
            temp = strchr(temp, ' ') + 1;
            _cellular_data.rssi = utils_atoll(temp);
            temp = strchr(temp, ',') + 1;
            _cellular_data.ber = utils_atoll(temp);
  
            _rssi_info.rssi = cellular_utilities_get_dbm_from_rssi(_cellular_data.rssi);
            _rssi_info.rssi_available = true;
            _rssi_info.ber = _cellular_data.ber;
            _rssi_info.ber_available = true;
            _rssi_info.last_read_timestamp = current_timestamp;
  
            if (_cellular_data.rssi >= CSQ_MIN_SIGNAL(_simcom_module_version_used)) {
              /* Clear current registration next try timeout */
              _current_registration_failed_next_try_timeout = 0;
  
              if (!_psm_configured && cellular_has_psm_active()) {
                cellular_disable_timer_callback();
  
                _current_states.warmup = WARMUP_DISABLE_PSM;
                _current_action = SENDING_COMMAND;
                _command_attempts_number = 0;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
              } else {
                _cellular_data.warmup_success = true;
                _last_time_phase_set_timestamp = current_timestamp;
                cellular_disable_timer_callback();
  
                _current_states.warmup = WARMUP_FINISHED_STATE;
                _current_action = IDLE;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
  
                /* Just prevention between phases transition */
                _cellular_module_restart_timestamp = current_timestamp;
                _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
              }
            }
          }
        }
      }
      break;
    }
  
    case WARMUP_SET_APN_NBIOT_FIND_IMSI_STATE_PHASE_1: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(GET_IMSI_COMMAND, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command, &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
          temp = strstr(last_completed_transaction.responses, GET_IMSI_COMMAND);
  
          if (temp != NULL) {
            temp = strchr(temp, '\n') + 1;
  
            uint64_t new_imsi_read = utils_atoll(temp);
  
            /* Is it really different? */
            if (_cellular_data.imsi != new_imsi_read) {
              _cellular_data.imsi = new_imsi_read;
  
              memset(&(_cellular_data.imsi_text), '\0', IMSI_MAX_LENGTH);
              memcpy(&(_cellular_data.imsi_text), temp, cellular_utilities_number_of_chars_until_cr(temp));
  
              debug_print_time(DEBUG_LEVEL_2, current_timestamp);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_info_state_machine] new imsi: ");
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)_cellular_data.imsi_text);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\r\n");
            }
  
            if (_cellular_data.imsi > 0) {
              cellular_disable_timer_callback();
  
              _current_states.warmup = WARMUP_SET_BAND;
              _current_action = SENDING_COMMAND;
              _command_attempts_number = 0;
  
              /* Just disable it until next command tx */
              _tx_command_unsuccessful_timestamp = 0;
            }
          }
        }
      }
      break;
    }
  
    case WARMUP_SET_APN_NBIOT_DISABLE_RADIO_STATE_PHASE_4: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(DISABLE_RADIO_COMMAND, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_45S, COMMAND_RETRY_TIME_15000MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
        if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
          _current_states.warmup = WARMUP_SET_APN_NBIOT_STATE_PHASE_3;
        } else {
          _current_states.warmup = WARMUP_SET_APN_NBIOT_STATE_PHASE_2;
        }
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_PSM: {
      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, SET_PSM_COMMAND);
  
        /* Strcat T3412 value in binary */
        int c = 0, k = 0;
        for (c = 7; c >= 0; c--) {
          k = _t3412_value >> c;
          if (k & 1) {
            strcat(command, "1");
          } else {
            strcat(command, "0");
          }
        }
        strcat(command, "\",\"");
  
        /* Strcat T3324 value in binary */
        c = 0;
        k = 0;
        for (c = 7; c >= 0; c--) {
          k = _t3324_value >> c;
          if (k & 1) {
            strcat(command, "1");
          } else {
            strcat(command, "0");
          }
        }
        strcat(command, "\"\r");
  
        cellular_send_new_command(command, "+CEREG: 1",
            RESPONSE_WAITING_TIME_45S, COMMAND_RETRY_TIME_15000MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        if (cellular_has_edrx_active()) {
          _current_states.warmup = WARMUP_DISABLE_EDRX;
        } else {
          _current_states.warmup = WARMUP_IS_REGISTERED_STATE;
          _psm_configured = true;
        }
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_DISABLE_EDRX: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(DISABLE_EDRX_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_GET_EDRX_STATUS;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_GET_EDRX_STATUS: {
      if (_current_action == SENDING_COMMAND) {
        if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
          cellular_send_new_command(GET_EDRX_STATUS(_simcom_module_version_used),
              GET_EDRX_NBIOT_DEACTIVATED_RESPONSE,
              RESPONSE_WAITING_TIME_60S, COMMAND_RETRY_TIME_1000MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        } else {
          cellular_send_new_command(GET_EDRX_STATUS(_simcom_module_version_used),
              GENERIC_OK_RESPONSE,
              RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        }
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_EDRX;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_EDRX: {
      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, SET_EDRX_COMMAND(_simcom_module_version_used));
  
        if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
          /* Strcat PTW value */
          char temp_char_array[20];
          memset(temp_char_array, '\0', 20);
          utils_itoa(_ptw_period_value, temp_char_array, 10);
          strcat(command, temp_char_array);
          strcat(command, "\",\"");
  
          /* Strcat eDRX period value */
          memset(temp_char_array, '\0', 20);
          utils_itoa(_edrx_period_value, temp_char_array, 10);
          strcat(command, temp_char_array);
          strcat(command, "\"\r");
        } else {
          /* Strcat eDRX period value in binary */
          int c = 0, k = 0;
          for (c = 3; c >= 0; c--) {
            k = _edrx_period_value >> c;
            if (k & 1) {
              strcat(command, "1");
            } else {
              strcat(command, "0");
            }
          }
          strcat(command, "\",\"");
  
          /* Strcat PTW value in binary */
          c = 0;
          k = 0;
          for (c = 3; c >= 0; c--) {
            k = _ptw_period_value >> c;
            if (k & 1) {
              strcat(command, "1");
            } else {
              strcat(command, "0");
            }
          }
          strcat(command, "\"\r");
        }
  
        cellular_send_new_command(command, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_45S, COMMAND_RETRY_TIME_15000MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_GET_EDRX_PARAMETERS;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_GET_EDRX_PARAMETERS: {
      if (_current_action == SENDING_COMMAND) {
        if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
          cellular_send_new_command(GET_EDRX_PARAMETERS,
              GET_EDRX_NBIOT_ACTIVATED_RESPONSE,
              RESPONSE_WAITING_TIME_60S, COMMAND_RETRY_TIME_1000MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        } else {
          cellular_send_new_command(GET_EDRX_PARAMETERS,
              GENERIC_OK_RESPONSE,
              RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        }
      } else if (_current_action == GOOD_RESPONSE) {
        _psm_configured = true;
  
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_IS_REGISTERED_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_5: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(ENABLE_RADIO_COMMAND, ENABLE_RADIO_RESPONSE,
            RESPONSE_WAITING_TIME_45S, COMMAND_RETRY_TIME_15000MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        _nbiot_apn_configured = true;
  
        cellular_disable_timer_callback();
  
        if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
          _current_states.warmup = WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_6;
        } else {
          _current_states.warmup = WARMUP_IS_REGISTERED_STATE;
        }
  
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_6: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(CHECK_PS_SERVICE_COMMAND, "+CGATT: 1",
            RESPONSE_WAITING_TIME_900S, COMMAND_RETRY_TIME_1000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_7;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_7: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(QUERY_APN_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_8;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_8: {
      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, SET_AUTH_COMMAND_1);
        strcat(command,
            cellular_utilities_get_apn((_cellular_data.imsi / 1000000000000),
                ((_cellular_data.imsi % 1000000000000) / 10000000000),
                _simcom_module_version_used, cellular_has_psm_active()));
  
        /* Check if auth is required */
        if (cellular_utilities_auth_needed((_cellular_data.imsi / 1000000000000),
                ((_cellular_data.imsi % 1000000000000) / 10000000000),
                _simcom_module_version_used)) {
          strcat(command, "\",\"");
          strcat(command,
              cellular_utilities_get_user((_cellular_data.imsi / 1000000000000),
                  ((_cellular_data.imsi % 1000000000000) / 10000000000),
                  _simcom_module_version_used));
          strcat(command, "\",\"");
          strcat(command,
              cellular_utilities_get_pass((_cellular_data.imsi / 1000000000000),
                  ((_cellular_data.imsi % 1000000000000) / 10000000000),
                  _simcom_module_version_used));
          strcat(command, "\",3\r");
  
        } else {
          strcat(command, "\"\r");
        }
  
        cellular_send_new_command(command,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_9;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_9: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(ACTIVATE_NETWORK_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_IS_REGISTERED_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_APN_NBIOT_STATE_PHASE_2: {
      
      debug_printf_string(DEBUG_LEVEL_0,"[cellular_data_session_state_machine] MCC: %d , MNC: %d \n", 
        (uint16_t)(_cellular_data.imsi / 1000000000000), 
        (uint8_t)((_cellular_data.imsi % 1000000000000) / 10000000000));

      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, SET_AUTH_COMMAND_1);
        strcat(command,
            cellular_utilities_get_apn((_cellular_data.imsi / 1000000000000),
                ((_cellular_data.imsi % 1000000000000) / 10000000000),
                _simcom_module_version_used, cellular_has_psm_active()));
  
        /* Check if auth is required */
        if (cellular_utilities_auth_needed((_cellular_data.imsi / 1000000000000),
                ((_cellular_data.imsi % 1000000000000) / 10000000000),
                _simcom_module_version_used)) {
          strcat(command, "\",\"");
          strcat(command,
              cellular_utilities_get_user((_cellular_data.imsi / 1000000000000),
                  ((_cellular_data.imsi % 1000000000000) / 10000000000),
                  _simcom_module_version_used));
          strcat(command, "\",\"");
          strcat(command,
              cellular_utilities_get_pass((_cellular_data.imsi / 1000000000000),
                  ((_cellular_data.imsi % 1000000000000) / 10000000000),
                  _simcom_module_version_used));
          strcat(command, "\",3\r");
        } else {
          strcat(command, "\"\r");
        }
  
        cellular_send_new_command(command,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
  
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_APN_NBIOT_STATE_PHASE_4;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_APN_NBIOT_STATE_PHASE_3: {
      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, SET_APN_1_COMMAND(_simcom_module_version_used));
        strcat(command,
            cellular_utilities_get_apn( (_cellular_data.imsi / 1000000000000),
                                  ( (_cellular_data.imsi % 1000000000000) / 10000000000 ),
                                  _simcom_module_version_used, cellular_has_psm_active() ) );
  
        strcat(command, "\"\r");
        cellular_send_new_command(command,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_APN_NBIOT_STATE_PHASE_2;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_APN_NBIOT_STATE_PHASE_4: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(GET_PDP_CONTEXT_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_5;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_DISABLE_PSM: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(DISABLE_PSM_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_PSM_WAKEUP_INDICATION;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_PSM_WAKEUP_INDICATION: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(SET_PSM_WAKEUP_INDICATION,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_UNSOLICITED_PSM_CODE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_UNSOLICITED_PSM_CODE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(SET_PSM_UNSOLICITED_PSM_CODE_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_PSM;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_SET_BAND: {
      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, BAND_MANUAL_SELECTION(_simcom_module_version_used));
        if (_simcom_module_version_used == SIMCOM_7020_VERSION) {
          strcat(command,
                 cellular_utilities_get_band((_cellular_data.imsi / 1000000000000),
                  ( (_cellular_data.imsi % 1000000000000) / 10000000000),
                     _simcom_module_version_used) );
        }
        strcat(command, "\r");
  
        cellular_send_new_command(command, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_30S, COMMAND_RETRY_TIME_15000MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.warmup = WARMUP_SET_APN_NBIOT_DISABLE_RADIO_STATE_PHASE_4;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case WARMUP_FINISHED_STATE: {
      break;
    }
  
    default: {
      _current_states.warmup = WARMUP_IDLE_STATE;
      _current_action = IDLE;
    }
  }
}

/**
 * @brief   The function that will run the cells phase state machine
 */
void cellular_cells_state_machine(uint64_t current_timestamp) {
  switch (_current_states.cells) {
    case CELLS_IDLE_STATE: {
      break;
    }
  
    case CELLS_STARTING_STATE: {
      if (_simcom_module_version_used == SIMCOM_7600_VERSION) {
        cellular_disable_timer_callback();
        _timestamp_cells = _sysclk_get_in_ms_callback();
        _current_states.cells = CELLS_WAITING_FOR_COLLECTION;
        _current_action = IDLE;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
  
        /* Just prevention between phases transition */
        _cellular_module_restart_timestamp = current_timestamp;
        _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT + TIMEOUT_TO_COLLECT_CELLS;
  
        debug_print_time(DEBUG_LEVEL_2, current_timestamp);
        debug_print_string(DEBUG_LEVEL_2, 
            (uint8_t *)"[cellular_cells_state_machine] waiting before reading cells towers info\r\n");
      } else {
        if (_wakeup_from_psm_by_cells) {
          _current_states.cells = CELLS_WAKE_UP_FROM_PSM_STATE_1;
  
          debug_print_time(DEBUG_LEVEL_2, current_timestamp);
          debug_print_string(DEBUG_LEVEL_2, 
              (uint8_t *)"[cellular_cells_state_machine] we were sleeping so let's force HTTP session to keep the module up\r\n");
        } else {
          _current_states.cells = CELLS_SET_MODE_STATE;
        }
      }
      _current_action = SENDING_COMMAND;
      _command_attempts_number = 0;
  
      cellular_disable_timer_callback();
  
      /* Just disable it until next command tx */
      _tx_command_unsuccessful_timestamp = 0;
  
      break;
    }
  
    case CELLS_WAKE_UP_FROM_PSM_STATE_1: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(
            HTTP_INIT_COMMAND(_simcom_module_version_used, _machinates_version, _https_on),
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_30S,
            COMMAND_RETRY_TIME_5000MS,
            true, current_timestamp, WRONG_RESPONSE_ERROR);
      } else if (_current_action == GOOD_RESPONSE) {
        _current_states.cells = CELLS_SET_MODE_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
  
        break;
      }
      break;
    }
  
    case CELLS_SET_MODE_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(
            CELLS_SET_MODE_COMMAND_1(_simcom_module_version_used),
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
        _timestamp_cells = _sysclk_get_in_ms_callback();
        _current_states.cells = CELLS_WAITING_FOR_COLLECTION;
        _current_action = IDLE;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
  
        /* Just prevention between phases transition */
        _cellular_module_restart_timestamp = current_timestamp;
        _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT + TIMEOUT_TO_COLLECT_CELLS;
  
        debug_print_time(DEBUG_LEVEL_2, current_timestamp);
        debug_print_string(DEBUG_LEVEL_2, 
            (uint8_t *)"[cellular_cells_state_machine] waiting before reading cells towers info\r\n");
      }
      break;
    }
  
    case CELLS_WAITING_FOR_COLLECTION: {
      if ( (_timestamp_cells != 0) && 
           ( (_sysclk_get_in_ms_callback() - _timestamp_cells) > TIMEOUT_TO_COLLECT_CELLS) ) {
        _timestamp_cells = 0;
  
        _current_states.cells = CELLS_GET_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case CELLS_GET_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(CELLS_GET_COMMAND(_simcom_module_version_used),
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S, COMMAND_RETRY_TIME_2000MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(
            CELLS_GET_RESPONSE(_simcom_module_version_used),
            &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
          temp = strstr(last_completed_transaction.responses, CELLS_GET_RESPONSE(_simcom_module_version_used));
  
          if (temp != NULL) {
            if (cellular_get_simcom_version() == SIMCOM_800_VERSION) {
              temp = strchr(temp, ' ') + 1;
            }
  
            cellular_utilities_process_cells(temp, &_cells_data.cells_type,
                &_cells_data.cells_number, &_cells_data.mnc,
                &_cells_data.mcc, _cells_data.lac, _cells_data.ids,
                _cells_data.rx_levels, GAMA_CELLS_VECTOR_MAX_SIZE,
                cellular_get_simcom_version(),
                (_cellular_data.imsi / 1000000000000),
                ((_cellular_data.imsi % 1000000000000) / 10000000000));
  
            if (_wakeup_from_psm_by_cells) {
              debug_print_time(DEBUG_LEVEL_2, current_timestamp);
              debug_print_string(DEBUG_LEVEL_2, 
                  (uint8_t *)"[cellular_cells_state_machine] as we openned the HTTP session to keep module up now it's time to close it...\r\n");
  
              _current_states.cells = CELLS_WAKE_UP_FROM_PSM_STATE_2;
              _current_action = SENDING_COMMAND;
              _command_attempts_number = 0;
  
              /* Just disable it until next command tx */
              _tx_command_unsuccessful_timestamp = 0;
  
              break;
            } else {
              _cells_data.available = true;
              _cells_data.last_read_timestamp = rtc_get_milliseconds();
              _collect_cells = false;
  
              _registered_cell_on_network_collected = true;
  
              cellular_disable_timer_callback();
  
              _current_states.cells = CELLS_FINISHED_STATE;
              _current_action = IDLE;
  
              /* Just disable it until next command tx */
              _tx_command_unsuccessful_timestamp = 0;
  
              /* Just prevention between phases transition */
              _cellular_module_restart_timestamp = current_timestamp;
              _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
  
              break;
            }
          }
        }
      }
      break;
    }
  
    case CELLS_WAKE_UP_FROM_PSM_STATE_2: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_DESTROY_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_15S,
            COMMAND_RETRY_TIME_5000MS, false, current_timestamp,
            WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        _wakeup_from_psm_by_cells = false;
  
        _cells_data.available = true;
        _cells_data.last_read_timestamp = rtc_get_milliseconds();
        _collect_cells = false;
  
        _registered_cell_on_network_collected = true;
  
        cellular_disable_timer_callback();
  
        _current_states.cells = CELLS_FINISHED_STATE;
        _current_action = IDLE;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
  
        /* Just prevention between phases transition */
        _cellular_module_restart_timestamp = current_timestamp;
        _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
  
        break;
      }
      break;
    }
  
    case CELLS_FINISHED_STATE: {
      break;
    }
  
    default: {
      _current_states.cells = CELLS_IDLE_STATE;
      _current_action = IDLE;
    }
  }
}

/**
 * @brief       The function that will run the GPS phase state machine
 */
void cellular_gps_state_machine(uint64_t current_timestamp) {
  switch (_current_states.gps) {
    case GPS_IDLE_STATE: {
      break;
    }
  
    case GPS_STARTING_STATE: {
      _current_states.gps = GPS_START_MODE_STATE;
      _current_action = SENDING_COMMAND;
      _command_attempts_number = 0;
  
      /* Just disable it until next command tx */
      _tx_command_unsuccessful_timestamp = 0;
  
      break;
    }
  
    case GPS_START_MODE_STATE: {
      if (_current_action == SENDING_COMMAND) {
        if (_gps_turn_on_module_timestamp == 0) {
          _gps_turn_on_module_timestamp = current_timestamp;
        }
        /* We did the first sent. We saw that most of the times module returned
         * ok response and after that multiple error responses so let's do it
         * once and skip to RMC phase. */
        else {
          _current_action = GOOD_RESPONSE;
          break;
        }
  
        /* Check GPS rx maximum timeout just for precaution cause sometimes module
         * returns error message when trying to turn on GPS so we want to move forward and
         * try later again */
        if ( (_gps_turn_on_module_timestamp != 0) && 
             ( (current_timestamp - _gps_turn_on_module_timestamp) >= GPS_RX_VALID_FIX_MAXIMUM_TIMEOUT) ) {
          _gps_turn_on_module_timestamp = 0;
  
          _collect_gps = false;
          _gps_is_already_collecting = false;
          gps_utilities_set_gps_start_collecting_timestamp();
          gps_utilities_reset_gps_info();
  
          _current_states.gps = GPS_FINISHED_STATE;
          _current_action = IDLE;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
  
          /* Just prevention between phases transition */
          _cellular_module_restart_timestamp = current_timestamp;
          _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
  
          debug_print_time(DEBUG_LEVEL_1, current_timestamp);
          debug_print_string(DEBUG_LEVEL_1,
              (uint8_t *)"[cellular_gps_state_machine] timeout to turn on gps expired. Let's skip this sample and try again in the next one\r\n");
        } else {
          cellular_send_new_command(GPS_START_MODE_COMMAND(_simcom_module_version_used),
              GENERIC_OK_RESPONSE,
              RESPONSE_WAITING_TIME_15S, COMMAND_RETRY_TIME_5000MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        }
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _gps_turn_on_module_timestamp = 0;
  
        _current_states.gps = GPS_START_RMC_SENTENCE_MODE_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case GPS_START_RMC_SENTENCE_MODE_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(GPS_START_RMC_SENTENCE_COMMAND(_simcom_module_version_used),
            GENERIC_STAND_ALONE_RESPONSE,
            (GPS_RX_VALID_FIX_MAXIMUM_TIMEOUT + GPS_RX_VALID_FIX_GUARD_TIMEOUT), COMMAND_RETRY_TIME_5000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
  
        /* No command repetitions... Just wait for responses periodically */
        cellular_disable_timer_callback();
        _command_attempts_number = 0;
        _tx_command_unsuccessful_timestamp = 0;
  
        _current_action = STAND_ALONE_RESPONSE;
  
        _gps_is_already_collecting = true;
      } else if (_current_action == GOOD_RESPONSE) {
        _collect_gps = false;
        _gps_is_already_collecting = false;
  
        cellular_disable_timer_callback();
  
        _current_states.gps = GPS_STOP_RMC_SENTENCE_MODE_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      } else if (_current_action == STAND_ALONE_RESPONSE) {
        /* Process manually the received responses */
        if (simcom_how_many_new_responses() > 0) {
          struct simcom_responses received_reponses;
          simcom_read_responses(&received_reponses);
  
          /* Is it a RMC GPS sentence? */
          if (strstr(received_reponses.responses, GPS_RMC_SENTENCE_PREAMBLE(_simcom_module_version_used)) != NULL) {
            /* Process RMC GPS sentence and update GPS info values */
            gps_utilities_process_rmc_msg(
                strstr(received_reponses.responses, GPS_RMC_SENTENCE_PREAMBLE(_simcom_module_version_used)) );
          }
  
          /* Is it a GGA GPS sentence? */
          if (strstr(received_reponses.responses, GPS_GGA_SENTENCE_PREAMBLE(_simcom_module_version_used)) != NULL) {
            /* Process GGA GPS sentence and update GPS info values */
            gps_utilities_process_gga_msg(
                strstr(received_reponses.responses, GPS_GGA_SENTENCE_PREAMBLE(_simcom_module_version_used)) );
          }
        }
  
        /* Check GPS rx maximum timeout just for precaution. Remember
         * that I'm running stand alone, so be careful... */
        if ( (current_timestamp - gps_utilities_get_gps_info()->start_collecting_timestamp) >= 
             GPS_RX_VALID_FIX_MAXIMUM_TIMEOUT ) {
          _collect_gps = false;
          _gps_is_already_collecting = false;
          gps_utilities_set_gps_start_collecting_timestamp();
          gps_utilities_reset_gps_info();
  
          cellular_disable_timer_callback();
  
          _current_states.gps = GPS_STOP_RMC_SENTENCE_MODE_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
  
          debug_print_time(DEBUG_LEVEL_2, current_timestamp);
          debug_print_string(DEBUG_LEVEL_2, 
              (uint8_t *)"[cellular_gps_state_machine] timeout to get a valid GPS fix expired\r\n");
        }
      }
      break;
    }
  
    case GPS_STOP_RMC_SENTENCE_MODE_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(GPS_STOP_RMC_SENTENCE_COMMAND(_simcom_module_version_used),
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_15S, COMMAND_RETRY_TIME_5000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.gps = GPS_STOP_MODE_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case GPS_STOP_MODE_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(GPS_STOP_MODE_COMMAND(_simcom_module_version_used),
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_15S, COMMAND_RETRY_TIME_5000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        _gps_is_already_collecting = false;
  
        _current_states.gps = GPS_FINISHED_STATE;
        _current_action = IDLE;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
  
        /* Just prevention between phases transition */
        _cellular_module_restart_timestamp = current_timestamp;
        _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
      }
      break;
    }
  
    case GPS_FINISHED_STATE: {
      break;
    }
  
    default: {
      _current_states.gps = GPS_IDLE_STATE;
      _current_action = IDLE;
    }
  }
}

/**
 * @brief   The function that will run the info phase state machine
 */
void cellular_info_state_machine(uint64_t current_timestamp) {
  switch (_current_states.info) {
    case INFO_IDLE_STATE: {
      break;
    }
  
    case INFO_STARTING_STATE: {
      if (strlen(_cellular_data.imei) == 0) {
        _current_states.info = INFO_GET_IMEI_STATE;
      } else {
        _current_states.info = INFO_GET_ICCID_STATE;
      }
      _current_action = SENDING_COMMAND;
      _command_attempts_number = 0;
  
      /* Just disable it until next command tx */
      _tx_command_unsuccessful_timestamp = 0;
  
      break;
    }
  
    case INFO_GET_IMEI_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(GET_IMEI_COMMAND, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command, &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
          temp = strstr(last_completed_transaction.responses, GET_IMEI_COMMAND);
  
          if (temp != NULL) {
            temp = strchr(temp, '\n') + 1;
  
            char new_imei[IMEI_MAX_LENGTH];
            memset(&(new_imei), '\0', IMEI_MAX_LENGTH);
            memcpy(&(new_imei), temp, cellular_utilities_number_of_chars_until_cr(temp));
  
            if (strcmp(_cellular_data.imei, new_imei) != 0) {
              memset(&(_cellular_data.imei), '\0', IMEI_MAX_LENGTH);
              memcpy(&(_cellular_data.imei), temp, cellular_utilities_number_of_chars_until_cr(temp));
            }
  
            if (strlen(_cellular_data.imei) > 0) {
              cellular_disable_timer_callback();
  
              debug_print_time(DEBUG_LEVEL_2, current_timestamp);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_info_state_machine] imei: ");
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)_cellular_data.imei);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\r\n");
  
              _current_states.info = INFO_GET_ICCID_STATE;
              _current_action = SENDING_COMMAND;
              _command_attempts_number = 0;
  
              /* Just disable it until next command tx */
              _tx_command_unsuccessful_timestamp = 0;
            }
          }
        }
      }
      break;
    }
  
    case INFO_GET_ICCID_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(GET_ICCID_COMMAND, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command, &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
  
          if (_simcom_module_version_used == SIMCOM_7600_VERSION) {
            temp = strstr(last_completed_transaction.responses, GET_ICCID_RESPONSE);
          } else {
            temp = strstr(last_completed_transaction.responses, GET_ICCID_COMMAND);
          }
  
          if (temp != NULL) {
            if (_simcom_module_version_used == SIMCOM_7600_VERSION) {
              temp = strchr(temp, ' ') + 1;
            } else {
              temp = strchr(temp, '\n') + 1;
            }
  
            char new_iccid[ICCID_MAX_LENGTH];
            memset(&(new_iccid), '\0', ICCID_MAX_LENGTH);
            memcpy(&(new_iccid), temp, cellular_utilities_number_of_chars_until_cr(temp));
  
            if (strcmp(_cellular_data.iccid, new_iccid) != 0) {
              memset(&(_cellular_data.iccid), '\0', ICCID_MAX_LENGTH);
              memcpy(&(_cellular_data.iccid), temp, cellular_utilities_number_of_chars_until_cr(temp));
            }
  
            if (strlen(_cellular_data.iccid) > 0) {
              cellular_disable_timer_callback();
  
              debug_print_time(DEBUG_LEVEL_2, current_timestamp);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_info_state_machine] iccid: ");
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)_cellular_data.iccid);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\r\n");
  
              _current_states.info = INFO_GET_IMSI_STATE;
              _current_action = SENDING_COMMAND;
              _command_attempts_number = 0;
  
              /* Just disable it until next command tx */
              _tx_command_unsuccessful_timestamp = 0;
            }
          }
        }
      }
      break;
    }
  
    case INFO_GET_IMSI_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(GET_IMSI_COMMAND, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS, false,
            current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command, &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
          temp = strstr(last_completed_transaction.responses, GET_IMSI_COMMAND);
  
          if (temp != NULL) {
            temp = strchr(temp, '\n') + 1;
  
            uint64_t new_imsi_read = utils_atoll(temp);
  
            /* Is it really different? */
            if (_cellular_data.imsi != new_imsi_read) {
              _cellular_data.imsi = new_imsi_read;
  
              memset(&(_cellular_data.imsi_text), '\0', IMSI_MAX_LENGTH);
              memcpy(&(_cellular_data.imsi_text), temp, cellular_utilities_number_of_chars_until_cr(temp));
  
              debug_print_time(DEBUG_LEVEL_2, current_timestamp);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_info_state_machine] new imsi: ");
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)_cellular_data.imsi_text);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\r\n");
            } else {
              debug_print_time(DEBUG_LEVEL_2, current_timestamp);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_info_state_machine] old imsi: ");
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)_cellular_data.imsi_text);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\r\n");
            }
  
            if (_cellular_data.imsi > 0) {
              _cellular_data.info_success = true;
              _last_time_phase_set_timestamp = current_timestamp;
              cellular_disable_timer_callback();
  
              _sensoroid_send_hello_msg_callback(_cellular_data.iccid,
                  _cellular_data.imei, _cellular_data.imsi_text,
                  _cellular_data.simcom_version, current_timestamp);
  
              _current_states.info = INFO_FINISHED_STATE;
              _current_action = IDLE;
  
              /* Just disable it until next command tx */
              _tx_command_unsuccessful_timestamp = 0;
  
              /* Just prevention between phases transition */
              _cellular_module_restart_timestamp = current_timestamp;
              _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
            }
          }
        }
      }
      break;
    }
  
    case INFO_FINISHED_STATE: {
      break;
    }
  
    default: {
      _current_states.info = INFO_IDLE_STATE;
      _current_action = IDLE;
    }
  }
}

/**
 * @brief   The function that will run the data session phase state machine
 */
void cellular_data_session_state_machine(uint64_t current_timestamp) {
  switch (_current_states.data_session) {
    case DATA_SESSION_IDLE_STATE: {
      break;
    }
  
    case DATA_SESSION_STARTING_STATE: {
      if ( (_simcom_module_version_used == SIMCOM_7020_VERSION) || 
           (_simcom_module_version_used == SIMCOM_7080_VERSION) ) {
        _current_states.data_session = DATA_SESSION_IS_ATTACHED_TO_NETWORK_STATE;
        _want_to_attach_data_session = true;
      } else {
        _current_states.data_session = DATA_SESSION_IS_OPEN_STATE;
      }
      _current_action = SENDING_COMMAND;
      _command_attempts_number = 0;
  
      /* Prevention! */
      _cellular_data.has_data_to_read = false;
      _cellular_data.has_downloaded_data = false;
      _cellular_data.downloaded_data_last_read_size = 0;
      _cellular_data.downloaded_data_total_size = 0;
      _cellular_data.downloaded_data_total_read_size = 0;
      _skip_downloaded_data = false;
  
      /* Just disable it until next command tx */
      _tx_command_unsuccessful_timestamp = 0;
  
      break;
    }
  
    case DATA_SESSION_IS_ATTACHED_TO_NETWORK_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(DATA_SESSION_IS_ATTACHED_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S, COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command, &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
          temp = strstr(last_completed_transaction.responses, DATA_SESSION_IS_ATTACHED_COMMAND_RESPONSE);
  
          if (temp != NULL) {
            temp = strchr(temp, ' ') + 1;
            _cellular_data.data_session_attached_status = utils_atoll(temp);
  
            if (_cellular_data.data_session_attached_status == 1) {
              debug_print_time(DEBUG_LEVEL_0, current_timestamp);
              debug_print_string(DEBUG_LEVEL_0, 
                  (uint8_t *)"[cellular_data_session_state_machine] data session is attached\r\n");
  
              if (!_want_to_attach_data_session) {
                debug_print_time(DEBUG_LEVEL_0, current_timestamp);
                debug_print_string(DEBUG_LEVEL_0, 
                    (uint8_t *)"[cellular_data_session_state_machine] detach now!\r\n");
  
                cellular_disable_timer_callback();
  
                _current_states.data_session = DATA_SESSION_DETACH_TO_NETWORK_STATE;
                _current_action = SENDING_COMMAND;
                _command_attempts_number = 0;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
              } else {
                debug_print_time(DEBUG_LEVEL_0, current_timestamp);
                debug_print_string(DEBUG_LEVEL_0, 
                    (uint8_t *)"[cellular_data_session_state_machine] already attached...\r\n");
  
                cellular_disable_timer_callback();
  
                _current_states.data_session = DATA_SESSION_IS_OPEN_STATE;
                _current_action = SENDING_COMMAND;
                _command_attempts_number = 0;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
              }
            } else { /* else if not (_cellular_data.data_session_attached_status == 1) */
              debug_print_time(DEBUG_LEVEL_0, current_timestamp);
              debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"[cellular_data_session_state_machine] data session is detached\r\n");
  
              if (_want_to_attach_data_session) {
                debug_print_time(DEBUG_LEVEL_0, current_timestamp);
                debug_print_string(DEBUG_LEVEL_0, 
                    (uint8_t *)"[cellular_data_session_state_machine] attach now!\r\n");
  
                cellular_disable_timer_callback();
  
                _current_states.data_session = DATA_SESSION_ATTACH_TO_NETWORK_STATE;
                _current_action = SENDING_COMMAND;
                _command_attempts_number = 0;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
              } else {
                debug_print_time(DEBUG_LEVEL_0, current_timestamp);
                debug_print_string(DEBUG_LEVEL_0, 
                    (uint8_t *)"[cellular_data_session_state_machine] already detached...\r\n");
  
                cellular_disable_timer_callback();
  
                _current_states.data_session = DATA_SESSION_FINISHED_STATE;
                _current_action = IDLE;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
  
                /* Just prevention between phases transition */
                _cellular_module_restart_timestamp = current_timestamp;
                _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
              }
            }
          }
        }
      }
      break;
    }
  
    case DATA_SESSION_ATTACH_TO_NETWORK_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(DATA_SESSION_ATTACH_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_45S, COMMAND_RETRY_TIME_15000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.data_session = DATA_SESSION_IS_ATTACHED_TO_NETWORK_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case DATA_SESSION_DETACH_TO_NETWORK_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(DATA_SESSION_DETACH_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_1800S, COMMAND_RETRY_TIME_45000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.data_session = DATA_SESSION_IS_ATTACHED_TO_NETWORK_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case DATA_SESSION_IS_OPEN_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(
            IS_DATA_SESSION_OPENED_COMMAND(_simcom_module_version_used),
            IS_DATA_SESSION_OPENED_RESPONSE(_simcom_module_version_used),
            RESPONSE_WAITING_TIME_30S,
            COMMAND_RETRY_TIME_1000MS, false, current_timestamp,
            WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command, &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
          temp = strstr( last_completed_transaction.responses,
              IS_DATA_SESSION_OPENED_RESPONSE(_simcom_module_version_used) );
  
          if (temp != NULL) {
            temp = strchr(temp, ' ') + 1;
            if (_simcom_module_version_used != SIMCOM_7080_VERSION) {
              temp = strchr(temp, ',') + 1;
            }
            _cellular_data.data_session_status = utils_atoll(temp);
  
            /* This will check the current data session status.
                States 1 (DATA_SESSION_OPENED) and 3 (DATA_SESSION_CLOSED) are for connected and closed.
                States 0 (DATA_SESSION_OPENING) and 2 (DATA_SESSION_CLOSING) are for connecting and closing
                  so we are interested only in completed states. */
            if (_cellular_data.data_session_status == DATA_SESSION_OPENED) {
              if ( (_session_failed_first_time_timestamp != 0) && 
                   ( (current_timestamp - _session_failed_first_time_timestamp) >= cellular_SESSION_TIMEOUT_SEND_FAILURE_MSG) ) {
                _session_failure_create_message_callback(
                    current_timestamp,
                    ((current_timestamp - _session_failed_first_time_timestamp) / 1000));
              }
              _session_failed_first_time_timestamp = 0;
  
              _current_session_failed_next_try_timeout = 0;
  
              if (_timestamp_starting_data_session == 0) {
                _timestamp_starting_data_session = current_timestamp;
              }
  
              if (_cellular_data.has_data_to_upload) {
                cellular_disable_timer_callback();
  
                _current_states.data_session = DATA_SESSION_FINISHED_STATE;
                _current_action = IDLE;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
  
                /* Just prevention between phases transition */
                _cellular_module_restart_timestamp = current_timestamp;
                _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
                /* else if not (_cellular_data.has_data_to_upload) */
              } else {
                if (cellular_has_psm_active()) {
                  debug_print_time(DEBUG_LEVEL_0, current_timestamp);
                  debug_print_string(DEBUG_LEVEL_0, 
                      (uint8_t *)"[cellular_data_session_state_machine] data session will stay open because we are running psm! thank god!\r\n");
                } else if (_remain_data_session_open != 0) {
                  debug_print_time(DEBUG_LEVEL_0, current_timestamp);
                  debug_print_string(DEBUG_LEVEL_0, 
                      (uint8_t *)"[cellular_data_session_state_machine] remain data session opened for ");
                  char seconds_remaining_opened[6];
                  memset(seconds_remaining_opened, '\0', 6);
                  utils_itoa(_remain_data_session_open, seconds_remaining_opened, 10);
                  debug_print_string(DEBUG_LEVEL_0, (uint8_t *)seconds_remaining_opened);
                  debug_print_string(DEBUG_LEVEL_0, (uint8_t *)" ms\r\n");
                } else {
                  debug_print_time(DEBUG_LEVEL_0, current_timestamp);
                  debug_print_string(DEBUG_LEVEL_0, 
                      (uint8_t *)"[cellular_data_session_state_machine] remain data session will be always open\r\n");
                }
  
                cellular_disable_timer_callback();
                _current_states.data_session = DATA_SESSION_REMAIN_OPEN;
                _current_action = IDLE;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
  
                if (!cellular_has_psm_active() && _remain_data_session_open != 0) {
                  /* Just prevention between phases transition */
                  _cellular_module_restart_timestamp = current_timestamp;
                  _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT + _remain_data_session_open;
                } else {
                  /* Wait for upper layer */
                  _cellular_module_restart_timestamp = 0;
                }
              }
              /* else if not (_cellular_data.data_session_status == DATA_SESSION_OPENED) */
            } else if (_cellular_data.data_session_status == DATA_SESSION_CLOSED(_simcom_module_version_used)) {
              _timestamp_starting_data_session = 0;
  
              if (!_timeout_starting_data_session) {
                /* Just to make sure... */
                _cellular_data.http_init = false;
  
                if ( (_simcom_module_version_used == SIMCOM_7020_VERSION) || 
                     (_simcom_module_version_used == SIMCOM_7080_VERSION) ) {
                  cellular_disable_timer_callback();
  
                  _current_states.data_session = DATA_SESSION_OPENING_STATE;
                  _current_action = SENDING_COMMAND;
                  _command_attempts_number = 0;
  
                  /* Just disable it until next command tx */
                  _tx_command_unsuccessful_timestamp = 0;
                } else {
                  cellular_disable_timer_callback();
  
                  _current_states.data_session = DATA_SESSION_SET_APN_STATE;
                  _current_action = SENDING_COMMAND;
                  _command_attempts_number = 0;
  
                  /* Just disable it until next command tx */
                  _tx_command_unsuccessful_timestamp = 0;
                }
              } else {
                if ( (_simcom_module_version_used == SIMCOM_7020_VERSION) || 
                     (_simcom_module_version_used == SIMCOM_7080_VERSION) ) {
                  _want_to_attach_data_session = false;
  
                  cellular_disable_timer_callback();
  
                  _current_states.data_session = DATA_SESSION_IS_ATTACHED_TO_NETWORK_STATE;
                  _current_action = SENDING_COMMAND;
                  _command_attempts_number = 0;
  
                  /* Just disable it until next command tx */
                  _tx_command_unsuccessful_timestamp = 0;
                } else {
                  _timeout_starting_data_session =
                      false;
  
                  cellular_disable_timer_callback();
                  _current_states.data_session = DATA_SESSION_FINISHED_STATE;
                  _current_action = IDLE;
  
                  /* Just disable it until next command tx */
                  _tx_command_unsuccessful_timestamp = 0;
  
                  /* Just prevention between phases transition */
                  _cellular_module_restart_timestamp = current_timestamp;
                  _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
                }
              }
            }
          }
        }
      }
      break;
    }
  
    case DATA_SESSION_SET_APN_STATE: {
      
      debug_printf_string(DEBUG_LEVEL_0,"[cellular_data_session_state_machine] MCC: %d , MNC: %d \n", 
        (uint16_t)(_cellular_data.imsi / 1000000000000), 
        (uint8_t)((_cellular_data.imsi % 1000000000000) / 10000000000));

      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        if (_simcom_module_version_used == SIMCOM_7600_VERSION) {
          strcpy(command, SET_APN_3_COMMAND);
        } else {
          strcpy(command, SET_APN_2_COMMAND);
        }
  
        strcat(command,
            cellular_utilities_get_apn((_cellular_data.imsi / 1000000000000),
                ((_cellular_data.imsi % 1000000000000) / 10000000000),
                _simcom_module_version_used, cellular_has_psm_active()));
        strcat(command, "\"\r");
  
        cellular_send_new_command(command, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        if (cellular_utilities_auth_needed((_cellular_data.imsi / 1000000000000),
                ((_cellular_data.imsi % 1000000000000) / 10000000000),
                _simcom_module_version_used)) {
          cellular_disable_timer_callback();
  
          _current_states.data_session = DATA_SESSION_SET_USER_APN_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        } else {
          cellular_disable_timer_callback();
  
          _current_states.data_session = DATA_SESSION_OPENING_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        }
      }
      break;
    }
  
    case DATA_SESSION_SET_USER_APN_STATE: {
      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, SET_APN_USER_COMMAND);
        strcat(command,
            cellular_utilities_get_user((_cellular_data.imsi / 1000000000000),
                ((_cellular_data.imsi % 1000000000000) / 10000000000),
                _simcom_module_version_used));
        strcat(command, "\"\r");
  
        cellular_send_new_command(command, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.data_session = DATA_SESSION_SET_PASS_APN_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case DATA_SESSION_SET_PASS_APN_STATE: {
      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, SET_APN_PASS_COMMAND);
        strcat(command,
            cellular_utilities_get_pass((_cellular_data.imsi / 1000000000000),
                ((_cellular_data.imsi % 1000000000000) / 10000000000),
                _simcom_module_version_used));
        strcat(command, "\"\r");
  
        cellular_send_new_command(command, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S, COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.data_session = DATA_SESSION_OPENING_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case DATA_SESSION_OPENING_STATE: {
      if (_current_action == SENDING_COMMAND) {
        if (_session_failed_first_time_timestamp == 0) {
          _session_failed_first_time_timestamp = current_timestamp;
        }
  
        cellular_send_new_command(
            OPEN_DATA_SESSION_COMMAND(_simcom_module_version_used),
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_60S,
            COMMAND_RETRY_TIME_15000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        _timestamp_starting_data_session = current_timestamp;
  
        cellular_disable_timer_callback();
  
        /* Update flag */
        if (!_current_data_session_status_is_ok) {
          _current_data_session_status_is_ok = true;
        }
  
        _current_states.data_session = DATA_SESSION_IS_OPEN_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case DATA_SESSION_CLOSING_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(
            CLOSE_DATA_SESSION_COMMAND(_simcom_module_version_used),
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_60S,
            COMMAND_RETRY_TIME_15000MS, false, current_timestamp,
            WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.data_session = DATA_SESSION_IS_OPEN_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case DATA_SESSION_REMAIN_OPEN: {
      if (!_psm_is_currently_active && cellular_has_psm_active()) {
        _psm_is_currently_active = true;
      }
  
      if (_cellular_data.has_data_to_upload) {
        if (cellular_has_psm_active()) {
          cellular_wakeup_from_psm(current_timestamp);
        }
  
        /* Just for prevention */
        _timeout_starting_data_session = false;
  
        cellular_disable_timer_callback();
  
        _current_states.data_session = DATA_SESSION_IS_OPEN_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
  
        break;
      }

#pragma GCC diagnostic ignored "-Wtype-limits"
      if ( !cellular_has_psm_active() && 
           ( ((_remain_data_session_open > 0) && ((current_timestamp - _timestamp_starting_data_session) >= _remain_data_session_open)) || 
             (_power_management_is_battery_saving_mode_callback()) ) ) {
        _timeout_starting_data_session = true;
  
        if (_simcom_module_version_used == SIMCOM_7600_VERSION) {
          debug_print_time(DEBUG_LEVEL_2, current_timestamp);
          debug_print_string(DEBUG_LEVEL_2, 
              (uint8_t *)"[cellular_data_session_state_machine] module is going down!\r");
  
          _timeout_starting_data_session = false;
  
          cellular_disable_timer_callback();
          _current_states.data_session = DATA_SESSION_FINISHED_STATE;
          _current_action = IDLE;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
  
          /* Just prevention between phases transition */
          _cellular_module_restart_timestamp = current_timestamp;
          _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
        } else {
          cellular_disable_timer_callback();
  
          _current_states.data_session = DATA_SESSION_CLOSING_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        }
  
        break;
      }
  
      break;
    }
  
    case DATA_SESSION_FINISHED_STATE: {
      break;
    }
  
    default: {
      _current_states.data_session = DATA_SESSION_IDLE_STATE;
      _current_action = IDLE;
    }
  }
}

/**
 * @brief   The function that will run the http phase state machine
 */
void cellular_http_state_machine(uint64_t current_timestamp) {
  switch (_current_states.http) {
    case HTTP_IDLE_STATE: {
      break;
    }
  
    case HTTP_STARTING_STATE: {
      if (_http_failed_first_time_timestamp == 0) {
        _http_failed_first_time_timestamp = current_timestamp;
      }
  
      if (_simcom_module_version_used == SIMCOM_7020_VERSION) {
        if (_cellular_data.http_init) {
          cellular_disable_timer_callback();
  
          _current_states.http = HTTP_UPLOAD_DATA_STATE;
          _current_action = IDLE;
          _cellular_data.is_time_to_set_data_to_upload = true;
  
          /* Wait for upper layer */
          _cellular_module_restart_timestamp = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        } else {
          _current_states.http = HTTP_INIT_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        }
      } else if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
        if (_cellular_data.http_init) {
          cellular_disable_timer_callback();
  
          _current_states.http = HTTP_UPLOAD_DATA_STATE;
          _current_action = IDLE;
          _cellular_data.is_time_to_set_data_to_upload = true;
  
          /* Wait for upper layer */
          _cellular_module_restart_timestamp = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        } else {
          if (_https_on) {
            _current_states.http = HTTP_SSL_CONFIGURATION_STATE;
          } else {
            _current_states.http = HTTP_SET_CONNECT_SERVER_STATE;
          }
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        }

        /* If not SIMCOM_70w0_VERSION nor SIMCOM_7080_VERSION */
      } else {
        if (_cellular_data.http_init) {
          _current_states.http = HTTP_SET_VERSION_PARAM_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        } else {
          _current_states.http = HTTP_INIT_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        }
      }
  
      break;
    }
  
    case HTTP_INIT_STATE: {
      if (_current_action == SENDING_COMMAND) {
        if ( (_simcom_module_version_used == SIMCOM_7020_VERSION) || 
             (_simcom_module_version_used == SIMCOM_7080_VERSION) ) {
          /* Some ERROR returns observed for NB-IoT in this phase so
           * when timeout expires just force new command */
          cellular_send_new_command(
              HTTP_INIT_COMMAND(_simcom_module_version_used,_machinates_version, _https_on),
              GENERIC_OK_RESPONSE,
              RESPONSE_WAITING_TIME_120S,
              COMMAND_RETRY_TIME_45000MS,
              true, current_timestamp, WRONG_RESPONSE_ERROR);
        } else {
          cellular_send_new_command(
              HTTP_INIT_COMMAND(_simcom_module_version_used,_machinates_version, _https_on),
              GENERIC_OK_RESPONSE,
              RESPONSE_WAITING_TIME_2S,
              COMMAND_RETRY_TIME_250MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        }
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        if ( (_simcom_module_version_used == SIMCOM_7020_VERSION) || 
             (_simcom_module_version_used == SIMCOM_7080_VERSION) ) {
          _current_states.http = HTTP_SET_CON_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        } else {
          _cellular_data.http_init = true;
  
          _current_states.http = HTTP_SET_VERSION_PARAM_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        }
      }
      break;
    }
  
    case HTTP_SET_CON_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_CON_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_60S,
            COMMAND_RETRY_TIME_45000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        _cellular_data.http_init = true;
  
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_UPLOAD_DATA_STATE;
        _current_action = IDLE;
        _cellular_data.is_time_to_set_data_to_upload = true;
  
        /* Wait for upper layer */
        _cellular_module_restart_timestamp = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_SET_VERSION_PARAM_STATE: {
      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, HTTP_SET_VERSION_PARAM_COMMAND_1);
        strcat(command, HTTP_SET_VERSION_PARAM_COMMAND_2);
  
        char temp_char_array[20];
        memset(temp_char_array, '\0', 20);
        utils_itoa(_machinates_version, temp_char_array, 10);
  
        strcat(command, temp_char_array);
        strcat(command, HTTP_SET_VERSION_PARAM_COMMAND_3);
  
        cellular_send_new_command(command,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_15S,
            COMMAND_RETRY_TIME_5000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        if (_cellular_data.http_context_saved) {
          if (_cellular_data.https_set) {
            _current_states.http = HTTP_UPLOAD_DATA_STATE;
            _current_action = IDLE;
            _cellular_data.is_time_to_set_data_to_upload = true;
  
            /* Wait for upper layer */
            _cellular_module_restart_timestamp = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
          } else {
            _current_states.http = HTTPS_IS_ENABLED_STATE;
            _current_action = SENDING_COMMAND;
            _command_attempts_number = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
          }
        } else {
          if (_simcom_module_version_used == SIMCOM_7600_VERSION) {
            _current_states.http = HTTP_END_POINT_STATE;
          } else {
            _current_states.http = HTTP_CID_STATE;
          }
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        }
      }
      break;
    }
  
    case HTTP_CID_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(
            HTTP_CID_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S,
            COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_END_POINT_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_END_POINT_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(
            HTTP_SET_END_POINT_COMMAND_PARAM(_machinates_version,_https_on),
            GENERIC_OK_RESPONSE, RESPONSE_WAITING_TIME_2S,
            COMMAND_RETRY_TIME_250MS, false, current_timestamp,
            WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        if (_simcom_module_version_used == SIMCOM_7600_VERSION) {
          cellular_disable_timer_callback();
  
          _current_states.http = HTTP_UPLOAD_DATA_STATE;
          _current_action = IDLE;
          _cellular_data.is_time_to_set_data_to_upload = true;
  
          /* Wait for upper layer */
          _cellular_module_restart_timestamp = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        } else {
          cellular_disable_timer_callback();
  
          _current_states.http = HTTP_SAVE_CONTEXT_STATE;
  
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        }
      }
      break;
    }
  
    case HTTP_SAVE_CONTEXT_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_SAVE_HTTP_CONTEXT_COMMAND,
            GENERIC_OK_RESPONSE, RESPONSE_WAITING_TIME_2S,
            COMMAND_RETRY_TIME_250MS, false, current_timestamp,
            WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        _cellular_data.http_context_saved = true;
  
        cellular_disable_timer_callback();
  
        if (_cellular_data.https_set) {
          _current_states.http = HTTP_UPLOAD_DATA_STATE;
          _current_action = IDLE;
          _cellular_data.is_time_to_set_data_to_upload = true;
  
          /* Wait for upper layer */
          _cellular_module_restart_timestamp = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        } else {
          _current_states.http = HTTPS_IS_ENABLED_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        }
      }
      break;
    }
  
    case HTTPS_IS_ENABLED_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTPS_IS_INIT_COMMAND,
            IS_HTTPS_INIT_RESPONSE,
            RESPONSE_WAITING_TIME_2S,
            COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        /* Get last completed transaction */
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command,
            &last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
          temp = strstr(last_completed_transaction.responses,IS_HTTPS_INIT_RESPONSE);
  
          if (temp != NULL) {
            temp = strchr(temp, ' ') + 1;
            uint8_t https_status = utils_atoll(temp);
  
            if (_https_on) {
              if (https_status == HTTPS_ENABLED) {
                debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                debug_print_string(DEBUG_LEVEL_2, 
                    (uint8_t *)"[cellular_http_state_machine] https is enabled\r\n");
  
                _cellular_data.https_set = true;
  
                cellular_disable_timer_callback();
  
                _current_states.http = HTTP_UPLOAD_DATA_STATE;
                _current_action = IDLE;
                _cellular_data.is_time_to_set_data_to_upload = true;
  
                /* Wait for upper layer */
                _cellular_module_restart_timestamp = 0;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
              } else {
                debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                debug_print_string(DEBUG_LEVEL_2, 
                    (uint8_t *)"[cellular_http_state_machine] https is disabled\r\n");
  
                cellular_disable_timer_callback();
  
                _current_states.http = HTTPS_ENABLE_STATE;
                _current_action = SENDING_COMMAND;
                _command_attempts_number = 0;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
              }

              /* else if not _https_on */
            } else {
              if (https_status == HTTPS_DISABLED) {
                debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                debug_print_string(DEBUG_LEVEL_2, 
                    (uint8_t *)"[cellular_http_state_machine] https is disabled\r\n");
  
                _cellular_data.https_set = true;
  
                cellular_disable_timer_callback();
  
                _current_states.http = HTTP_UPLOAD_DATA_STATE;
                _current_action = IDLE;
                _cellular_data.is_time_to_set_data_to_upload = true;
  
                /* Wait for upper layer */
                _cellular_module_restart_timestamp = 0;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
              } else {
                debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                debug_print_string(DEBUG_LEVEL_2, 
                    (uint8_t *)"[cellular_http_state_machine] https is enabled\r\n");
  
                cellular_disable_timer_callback();
  
                _current_states.http = HTTPS_DISABLE_STATE;
                _current_action = SENDING_COMMAND;
                _command_attempts_number = 0;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
              }
            }
          }
        }
      }
      break;
    }
  
    case HTTPS_ENABLE_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTPS_ENABLE_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S,
            COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTPS_IS_ENABLED_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTPS_DISABLE_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTPS_DISABLE_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_2S,
            COMMAND_RETRY_TIME_250MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTPS_IS_ENABLED_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_UPLOAD_DATA_STATE: {
      if (_current_action == IDLE) {
        if (_cellular_data.data_to_upload_size > 0) {
          if (_simcom_module_version_used == SIMCOM_7020_VERSION) {
            cellular_disable_timer_callback();
  
            _current_states.http = HTTP_ACTION_STATE;
            _current_action = SENDING_COMMAND;
            _command_attempts_number = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
  
            /* Clear flag for this new http action */
            _skip_downloaded_data = false;
          } else if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
            cellular_disable_timer_callback();
  
            _current_action = SENDING_COMMAND;
            _command_attempts_number = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
          } else {
            cellular_disable_timer_callback();
  
            _current_action = SENDING_COMMAND;
            _command_attempts_number = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
          }
        }
      } else if (_current_action == SENDING_COMMAND) {
        if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
          char command[COMMAND_SIZE];
          memset(command, '\0', COMMAND_SIZE);
  
          strcpy(command, HTTP_SET_BODY_1_COMMAND);
  
          char temp_char_array[20];
          memset(temp_char_array, '\0', 20);
          utils_itoa(_cellular_data.data_to_upload_size, temp_char_array, 10);
  
          strcat(command, temp_char_array);
          strcat(command, HTTP_SET_BODY_2_COMMAND);
  
          cellular_send_new_command(command,
              "",
              RESPONSE_WAITING_TIME_15S,
              COMMAND_RETRY_TIME_5000MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        } else {
          char command[COMMAND_SIZE];
          memset(command, '\0', COMMAND_SIZE);
  
          strcpy(command, HTTP_UPLOAD_DATA_COMMAND);
  
          char temp_char_array[20];
          memset(temp_char_array, '\0', 20);
          utils_itoa(_cellular_data.data_to_upload_size, temp_char_array, 10);
  
          strcat(command, temp_char_array);
          strcat(command, HTTP_TIME_TO_STORE_DATA_SUB_COMMAND);
  
          cellular_send_new_command(command,
              HTTP_UPLOAD_DATA_RESPONSE,
              RESPONSE_WAITING_TIME_2S,
              COMMAND_RETRY_TIME_250MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        }
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_STORE_DATA_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_STORE_DATA_STATE: {
      if (_current_action == SENDING_COMMAND) {
        char command[TX_BUFFER_SIZE];
        memset(command, '\0', TX_BUFFER_SIZE);
  
        strcpy(command, _cellular_data.data_to_upload);
        strcat(command, "\r");
  
        cellular_send_new_command(command, GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S, 0, true, current_timestamp,
            WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_ACTION_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
  
        /* Clear flag for this new http action */
        _skip_downloaded_data = false;
      }
      break;
    }
  
    case HTTP_ACTION_STATE: {
      if (_current_action == SENDING_COMMAND) {
        if (_simcom_module_version_used == SIMCOM_7020_VERSION) {
          uint16_t machinates_version_number_max_size = 4;
          char machinates_version_number[machinates_version_number_max_size];
          memset(machinates_version_number, '\0', machinates_version_number_max_size);
          utils_itoa(_machinates_version, machinates_version_number, 10);
  
          uint16_t machinates_version_max_size = 22;
          char machinates_version[machinates_version_max_size];
          memset(machinates_version, '\0', machinates_version_max_size);
          strcpy(machinates_version, HTTP_SET_VERSION_PARAM_COMMAND_2);
          strcat(machinates_version, machinates_version_number);
          strcat(machinates_version, "\r\n");
  
          uint16_t machinates_version_in_hex_max_size = (machinates_version_max_size * 2) + 1;
          char machinates_version_in_hex[machinates_version_in_hex_max_size];
          memset(machinates_version_in_hex, '\0', machinates_version_in_hex_max_size);
  
          /* Convert string characters of version parameters into hexadecimal characters */
          base64_string_to_hex(machinates_version,
              machinates_version_in_hex,
              machinates_version_in_hex_max_size);
  
          uint16_t base_64_to_upload_in_hex_max_size = (_cellular_data.data_to_upload_size * 2) + 1;
          char base_64_to_upload_in_hex[base_64_to_upload_in_hex_max_size];
          memset(base_64_to_upload_in_hex, '\0', base_64_to_upload_in_hex_max_size);
  
          /* Convert string characters of base64 into hexadecimal characters */
          base64_string_to_hex(_cellular_data.data_to_upload,
              base_64_to_upload_in_hex,
              base_64_to_upload_in_hex_max_size);
  
          char command[(strlen(HTTP_SEND_COMMAND_1(_machinates_version)) + 
                        strlen(HTTP_SEND_COMMAND_2(_machinates_version)) + 
                        machinates_version_in_hex_max_size + 
                        base_64_to_upload_in_hex_max_size) + 2];
          memset(command, '\0', ( strlen(HTTP_SEND_COMMAND_1(_machinates_version)) + 
                                  strlen(HTTP_SEND_COMMAND_2(_machinates_version)) + 
                                  strlen(machinates_version) + 
                                  base_64_to_upload_in_hex_max_size ) );
  
          strcpy(command, HTTP_SEND_COMMAND_1(_machinates_version));
          strcat(command, machinates_version_in_hex);
          strcat(command, HTTP_SEND_COMMAND_2(_machinates_version));
          strcat(command, base_64_to_upload_in_hex);
          strcat(command, "\r");
  
          _http_action_sent = true;
  
          cellular_send_new_command(command,
              HTTP_ACTION_RESPONSE(_simcom_module_version_used),
              RESPONSE_WAITING_TIME_180S,
              COMMAND_RETRY_TIME_90000MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);

          /* else if not SIMCOM_7020_VERSION */
        } else if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
          _http_action_sent = true;
  
          cellular_send_new_command(HTTP_POST_COMMAND,
              HTTP_POST_RESPONSE,
              RESPONSE_WAITING_TIME_180S,
              COMMAND_RETRY_TIME_90000MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        } else { /* When neither SIMCOM_7020_VERSION nor SIMCOM_7080_VERSION */
          _http_action_sent = true;
  
          cellular_send_new_command(HTTP_ACTION_COMMAND,
              HTTP_ACTION_RESPONSE(_simcom_module_version_used),
              RESPONSE_WAITING_TIME_180S,
              COMMAND_RETRY_TIME_90000MS,
              false, current_timestamp, WRONG_RESPONSE_DEFAULT);
        }
      } else if (_current_action == GOOD_RESPONSE) {
        if (_simcom_module_version_used == SIMCOM_7020_VERSION) {
          /* Get last completed transaction */
          struct simcom_responses last_completed_transaction;
          simcom_get_last_completed_transaction(
              HTTP_ACTION_RESPONSE(_simcom_module_version_used),
              &last_completed_transaction);
  
          if (last_completed_transaction.responses_size > 0) {
            char *temp;
            temp = strstr(last_completed_transaction.responses,
                HTTP_ACTION_RESPONSE(_simcom_module_version_used));
  
            if (temp != NULL) {
              temp = strchr(temp, ' ') + 1;
              temp = strchr(temp, ',') + 1;
              temp = strchr(temp, ',') + 1;
              temp = strchr(temp, ',') + 1;
              uint16_t http_response_size = utils_atoll(temp);
  
              /* TODO: fill with something good */
              _http_code = 0;
  
              /* If size >= 0 we got an 200 OK response from the server */
              if (http_response_size >= 0) {
                /* Check if it's a NOK from Machinates */
                if ( (http_response_size != 6) || 
                     ( (http_response_size == 6) && (_consecutive_noks_received >= MAX_CONSECUTIVE_NOKS_RECEIVED) ) ) {
                  if ((http_response_size == 6) && (_consecutive_noks_received >= MAX_CONSECUTIVE_NOKS_RECEIVED)) {
                    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                    debug_print_string(DEBUG_LEVEL_2, 
                        (uint8_t *)"[cellular_http_state_machine] nok answers limit! let's move to another message upload\r\n");
                  }
  
                  _consecutive_noks_received = 0;
                  _cellular_data.data_successfully_uploaded = true;
                  memset(_cellular_data.data_to_upload, '\0', TX_BUFFER_SIZE);
                  _cellular_data.data_to_upload_size = 0;
                  _cellular_data.has_data_to_upload = false;
  
                  /* Update flag */
                  if (!_current_http_status_is_ok) {
                    _current_http_status_is_ok = true;
                  }
  
                  _cellular_data.downloaded_data_total_size = 0;
                  _cellular_data.downloaded_data_total_read_size = 0;
                  _cellular_data.downloaded_data_last_read_size = 0;
  
                  /* Data size is too big for one read... Discard the rest of the information.
                   * Hex has the double of the size of our base 64 string buffer and we have
                   * to discount also the '\0' terminal in the end of the buffer */
                  if (http_response_size <= ((RX_BUFFER_SIZE * 2) - 2)) {
                    temp = strchr(temp, ',') + 1;
  
                    /* The size that was asked isn't the one we received. Try again upon timeout.
                     * The calculation has a plus two because of the "\r\n" in the end */
                    if (strlen(temp) >= http_response_size) {
                      uint16_t base64_response_in_hex_max_size = http_response_size + 1;
                      char base64_response_in_hex[base64_response_in_hex_max_size];
                      memset(base64_response_in_hex, '\0', base64_response_in_hex_max_size);
  
                      memcpy(base64_response_in_hex, temp, http_response_size);
  
                      memset(_cellular_data.downloaded_data, '\0', RX_BUFFER_SIZE);
                      base64_hex_to_string(
                          base64_response_in_hex,
                          _cellular_data.downloaded_data,
                          UPLINK_RX_BUFFER_SIZE);
  
                      if (strlen(_cellular_data.downloaded_data) == (http_response_size / 2)) {
                        debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                        debug_print_string(DEBUG_LEVEL_2, 
                            (uint8_t *)"[cellular_http_state_machine] read is ok\r\n");
  
                        /* Remember that the answer in hex has the double of the
                         * size of our base 64 string buffer */
                        _cellular_data.downloaded_data_total_size = strlen(_cellular_data.downloaded_data);
                        _cellular_data.downloaded_data_last_read_size = strlen(_cellular_data.downloaded_data);
  
                        _cellular_data.has_data_to_read = true;
  
                        _cellular_data.has_downloaded_data = true;
  
                        if ( (_http_failed_first_time_timestamp != 0) && 
                             ((current_timestamp - _http_failed_first_time_timestamp) >= cellular_HTTP_TIMEOUT_SEND_FAILURE_MSG) ) {
                          _http_failure_create_message_callback(
                              current_timestamp,
                              ((current_timestamp - _http_failed_first_time_timestamp) / 1000) );
                        }
                        _http_failed_first_time_timestamp = 0;
  
                        cellular_disable_timer_callback();
  
                        _current_states.http = HTTP_READ_STATE;
                        _current_action = IDLE;
  
                        /* Just disable it until next command tx */
                        _tx_command_unsuccessful_timestamp = 0;
  
                        /* Wait for upper layer */
                        _cellular_module_restart_timestamp = 0;
                      } else { /* if not (strlen(_cellular_data.downloaded_data) == (http_response_size / 2) */
                        debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                        debug_print_string(DEBUG_LEVEL_2, 
                            (uint8_t *)"[cellular_http_state_machine] 2nd read procedure was corrupted. try again later\r\n");
  
                        _cellular_data.has_data_to_read = false;
  
                        cellular_disable_timer_callback();
  
                        _current_states.http = HTTP_WAIT_FOR_MORE_DATA_STATE;
                        _current_action = IDLE;
                        _command_attempts_number = 0;
  
                        /* Just disable it until next command tx */
                        _tx_command_unsuccessful_timestamp = 0;
  
                        /* Wait for upper layer */
                        _cellular_module_restart_timestamp = 0;
  
                        http_starting_waiting_time_for_new_data = current_timestamp;
                      }
                    } else { /* else if not (strlen(temp) >= http_response_size) */
                      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                      debug_print_string(DEBUG_LEVEL_2, 
                          (uint8_t *)"[cellular_http_state_machine] 1st read procedure was corrupted. try again later\r\n");
  
                      _cellular_data.has_data_to_read = false;
  
                      cellular_disable_timer_callback();
  
                      _current_states.http = HTTP_WAIT_FOR_MORE_DATA_STATE;
                      _current_action = IDLE;
                      _command_attempts_number = 0;
  
                      /* Just disable it until next command tx */
                      _tx_command_unsuccessful_timestamp = 0;
  
                      /* Wait for upper layer */
                      _cellular_module_restart_timestamp = 0;
  
                      http_starting_waiting_time_for_new_data = current_timestamp;
                    }
                  } else { /* else if not (http_response_size <= ((RX_BUFFER_SIZE * 2) - 2)) */
                    _cellular_data.has_data_to_read = false;
  
                    cellular_disable_timer_callback();
  
                    _current_states.http = HTTP_WAIT_FOR_MORE_DATA_STATE;
                    _current_action = IDLE;
                    _command_attempts_number = 0;
  
                    /* Just disable it until next command tx */
                    _tx_command_unsuccessful_timestamp = 0;
  
                    /* Wait for upper layer */
                    _cellular_module_restart_timestamp = 0;
  
                    http_starting_waiting_time_for_new_data = current_timestamp;
                  }

                  /* Check if it's not a NOK */
                } else {
                  if ((http_response_size == 6) && _http_action_sent) {
                    _consecutive_noks_received++;
                    _current_action = BAD_RESPONSE;
  
                    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                    debug_print_string(DEBUG_LEVEL_2, 
                        (uint8_t *)"[cellular_http_state_machine] NOK answer detected. try again later ");
                    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"(");
                    char attempts[6];
                    memset(attempts, '\0', 6);
                    utils_itoa(_consecutive_noks_received, attempts, 10);
                    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)attempts);
                    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"/5)!\r\n");
                  }
  
                  _cellular_data.has_data_to_upload = false;
                  _http_action_sent = false;
                }
              } else { /* else if not (http_response_size >= 0) */
                cellular_disable_timer_callback();
  
                _current_states.http = HTTP_BAD_CODE_STATE;
                _current_action = IDLE;
  
                /* Wait for upper layer */
                _cellular_module_restart_timestamp = 0;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
  
                debug_print_time(DEBUG_LEVEL_1, current_timestamp);
                debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_http_state_machine] bad code (");
                char code[6];
                memset(code, '\0', 6);
                utils_itoa(_http_code, code, 10);
                debug_print_string(DEBUG_LEVEL_1, (uint8_t *)code);
                debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"). let's make a decision...\r\n");
  
                break;
              }
            }
          }
        } else { /* When not SIMCOM_7020_VERSION*/
          /* Get last completed transaction */
          struct simcom_responses last_completed_transaction;
          simcom_get_last_completed_transaction(_last_command, &last_completed_transaction);
  
          if (last_completed_transaction.responses_size > 0) {
            char *temp;
            if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
              temp = strstr(last_completed_transaction.responses, HTTP_POST_RESPONSE);
            } else {
              temp = strstr(last_completed_transaction.responses,
                  HTTP_ACTION_RESPONSE(_simcom_module_version_used));
            }
  
            if (temp != NULL) {
              if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
                temp = strchr(temp, ',') + 1;
              } else {
                temp = strchr(temp, ' ') + 1;
                temp = strchr(temp, ',') + 1;
              }
              uint16_t http_code = utils_atoll(temp);
  
              _http_code = http_code;
  
              //TODO: Process other codes to do specific actions according with the received http code
              if ((http_code >= HTTP_SUCCESS_CODE_ANSWER_BEGIN) && ((http_code <= HTTP_SUCCESS_CODE_ANSWER_END))) {
                _http_client_error_code_counter = 0;
                _http_other_error_codes = 0;
  
                temp = strchr(temp, ',') + 1;
  
                /* Check if it's a NOK */
                if ( (utils_atoll(temp) != 3) || 
                     ( (utils_atoll(temp) == 3) && (_consecutive_noks_received >= MAX_CONSECUTIVE_NOKS_RECEIVED) ) ) {
                  if ( (utils_atoll(temp) == 3) && (_consecutive_noks_received >= MAX_CONSECUTIVE_NOKS_RECEIVED) ) {
                    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                    debug_print_string(DEBUG_LEVEL_2, 
                        (uint8_t *)"[cellular_http_state_machine] nok answers limit! let's move to another message upload\r\n");
                  }
  
                  _consecutive_noks_received = 0;
                  _cellular_data.downloaded_data_total_size = 0;
                  _cellular_data.downloaded_data_total_size = utils_atoll(temp);
                  _cellular_data.downloaded_data_total_read_size = 0;
                  _cellular_data.downloaded_data_last_read_size = 0;
  
                  _cellular_data.has_data_to_read = true;
                  _cellular_data.data_successfully_uploaded = true;
                  memset(_cellular_data.data_to_upload, '\0', TX_BUFFER_SIZE);
                  _cellular_data.data_to_upload_size = 0;
                  _cellular_data.has_data_to_upload = false;
  
                  /* Update flag */
                  if (!_current_http_status_is_ok) {
                    _current_http_status_is_ok = true;
                  }
  
                  if (_cellular_data.downloaded_data_total_size > 0) {
                    if ( (_http_failed_first_time_timestamp != 0) && 
                         ((current_timestamp - _http_failed_first_time_timestamp) >= cellular_HTTP_TIMEOUT_SEND_FAILURE_MSG) ) {
                      _http_failure_create_message_callback(
                          current_timestamp,
                          ((current_timestamp - _http_failed_first_time_timestamp) / 1000));
                    }
                    _http_failed_first_time_timestamp = 0;
  
                    cellular_disable_timer_callback();
  
                    _current_states.http = HTTP_READ_STATE;
                    _current_action = SENDING_COMMAND;
                    _command_attempts_number = 0;
  
                    /* Just disable it until next command tx */
                    _tx_command_unsuccessful_timestamp = 0;
                  } else {
                    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                    debug_print_string(DEBUG_LEVEL_2, 
                        (uint8_t *)"[cellular_http_state_machine] received data size = 0\r\n");
  
                    _cellular_data.has_data_to_read = false;
  
                    cellular_disable_timer_callback();
  
                    _current_states.http = HTTP_WAIT_FOR_MORE_DATA_STATE;
                    _current_action = IDLE;
                    _command_attempts_number = 0;
  
                    /* Just disable it until next command tx */
                    _tx_command_unsuccessful_timestamp = 0;
  
                    /* Wait for upper layer */
                    _cellular_module_restart_timestamp = 0;
  
                    http_starting_waiting_time_for_new_data = current_timestamp;
                  }

                  /* Check if it's not a NOK */
                } else {
                  if ((utils_atoll(temp) == 3) && _http_action_sent) {
                    _consecutive_noks_received++;
                    _current_action = BAD_RESPONSE;
  
                    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                    debug_print_string(DEBUG_LEVEL_2, 
                        (uint8_t *)"[cellular_http_state_machine] NOK answer detected. try again later ");
                    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"(");
                    char attempts[6];
                    memset(attempts, '\0', 6);
                    utils_itoa(_consecutive_noks_received, attempts, 10);
                    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)attempts);
                    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"/5)!\r\n");
                  }
  
                  _cellular_data.has_data_to_upload = false;
                  _http_action_sent = false;
                }
              } else {
                cellular_disable_timer_callback();
  
                _current_states.http = HTTP_BAD_CODE_STATE;
                _current_action = IDLE;
  
                /* Wait for upper layer */
                _cellular_module_restart_timestamp = 0;
  
                /* Just disable it until next command tx */
                _tx_command_unsuccessful_timestamp = 0;
  
                debug_print_time(DEBUG_LEVEL_1, current_timestamp);
                debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_http_state_machine] bad code (");
                char code[6];
                memset(code, '\0', 6);
                utils_itoa(_http_code, code, 10);
                debug_print_string(DEBUG_LEVEL_1, (uint8_t *)code);
                debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"). let's make a decision...\r\n");
  
                break;
              }
            }
          }
        }
      }
      //    } else if (_current_action == WAITING_RESPONSE) {
      //      /* Get last completed transaction */
      //      struct simcom_responses last_completed_transaction;
      //      simcom_get_last_completed_transaction(
      //              HTTP_ACTION_RESPONSE(_simcom_module_version_used),
      //              &last_completed_transaction);
      //
      //      if (last_completed_transaction.responses_size > 0) {
      //        debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      //        debug_print_string(DEBUG_LEVEL_2, 
      //            (uint8_t*) "[cellular_http_state_machine] we didn't receive the complete commands but... we can understand that it happened good so let's continue for now with it\r\n");
      //
      //        _current_action = GOOD_RESPONSE;
      //      }
      //    }
      break;
    }
  
    case HTTP_BAD_CODE_STATE: {
      if (_current_action == IDLE) {
        /* Search for http client error codes */
        if ( (_http_code >= HTTP_CLIENT_ERROR_CODE_ANSWER_BEGIN) && 
             (_http_code <= HTTP_CLIENT_ERROR_CODE_ANSWER_END) ) {
          if (_http_client_error_code_counter >= HTTP_CLIENT_ERROR_CODE_MAX) {
            _http_client_error_code_counter = 0;
            _http_other_error_codes = 0;
  
            debug_print_time(DEBUG_LEVEL_1, current_timestamp);
            debug_print_string(DEBUG_LEVEL_1, 
                (uint8_t *)"[cellular_http_state_machine] max attempts for http client error reached. move forward please!\r\n");
  
            _consecutive_noks_received = 0;
            _cellular_data.downloaded_data_total_size = 0;
            _cellular_data.downloaded_data_total_read_size = 0;
            _cellular_data.downloaded_data_last_read_size = 0;
            _cellular_data.has_data_to_read = false;
            _cellular_data.data_successfully_uploaded = true;
            memset(_cellular_data.data_to_upload, '\0', TX_BUFFER_SIZE);
            _cellular_data.data_to_upload_size = 0;
            _cellular_data.has_data_to_upload = false;
  
            cellular_disable_timer_callback();
  
            _current_states.http = HTTP_WAIT_FOR_MORE_DATA_STATE;
            _current_action = IDLE;
            _command_attempts_number = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
  
            /* Wait for upper layer */
            _cellular_module_restart_timestamp = 0;
  
            http_starting_waiting_time_for_new_data = current_timestamp;
  
            break;
          } else { /* else if not (_http_client_error_code_counter >= HTTP_CLIENT_ERROR_CODE_MAX) */
            _current_action = BAD_RESPONSE;
            _cellular_data.has_data_to_upload = false;
            _cellular_data.data_successfully_uploaded = false;
  
            _http_client_error_code_counter++;
            _http_other_error_codes = 0;
  
            debug_print_time(DEBUG_LEVEL_1, current_timestamp);
            debug_print_string(DEBUG_LEVEL_1, 
                (uint8_t *)"[cellular_http_state_machine] http client error code detected! let's try again with a new base64\r\n");
            debug_print_time(DEBUG_LEVEL_1, current_timestamp);
            debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_http_state_machine] attempt ");
            char attempt[6];
            memset(attempt, '\0', 6);
            utils_itoa(_http_client_error_code_counter, attempt, 10);
            debug_print_string(DEBUG_LEVEL_1, (uint8_t *)attempt);
            debug_print_string(DEBUG_LEVEL_1, (uint8_t *)" out of ");
            memset(attempt, '\0', 6);
            utils_itoa(HTTP_CLIENT_ERROR_CODE_MAX, attempt, 10);
            debug_print_string(DEBUG_LEVEL_1, (uint8_t *)attempt);
            debug_print_string(DEBUG_LEVEL_1, (uint8_t *)".\r\n");
  
            break;
          }
          /* Other http codes than >= HTTP_CLIENT_ERROR_CODE_ANSWER_BEGIN) && 
                                   <= HTTP_CLIENT_ERROR_CODE_ANSWER_END)  */
        } else {
          _current_action = BAD_RESPONSE;
          _cellular_data.has_data_to_upload = false;
          _cellular_data.data_successfully_uploaded = false;
  
          _http_client_error_code_counter = 0;
          _http_other_error_codes++;
  
          debug_print_time(DEBUG_LEVEL_1, current_timestamp);
          debug_print_string(DEBUG_LEVEL_1, 
              (uint8_t *)"[cellular_http_state_machine] other http bad code detected! let's try again soon...\r\n");
  
          break;
        }
      }
  
      break;
    }
  
    case HTTP_READ_STATE: {
      /* In this case I'm waiting for the Uplink to read the downloaded packet. 
       * After that I will perform another read */
      if (_current_action == IDLE) {
        if (!_cellular_data.has_downloaded_data) {
          if (_simcom_module_version_used == SIMCOM_7020_VERSION) {
            debug_print_time(DEBUG_LEVEL_2, current_timestamp);
            debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_http_state_machine] finished read\r\n");
  
            _cellular_data.has_data_to_read = false;
  
            _current_states.http = HTTP_WAIT_FOR_MORE_DATA_STATE;
            _current_action = IDLE;
            _command_attempts_number = 0;
  
            /* Just disable it until next command tx */
            _tx_command_unsuccessful_timestamp = 0;
  
            /* Wait for upper layer */
            _cellular_module_restart_timestamp = 0;
  
            http_starting_waiting_time_for_new_data = current_timestamp;

            /* if not SIMCOM_7020_VERSION */
          } else {
            /* Total size reached or I know through another app that
             * it's time to skip the rest of the downloaded information */
            if ( (_cellular_data.downloaded_data_total_read_size >= _cellular_data.downloaded_data_total_size) || 
                 _skip_downloaded_data ) {
  
              debug_print_time(DEBUG_LEVEL_2, current_timestamp);
              debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_http_state_machine] finished read\r\n");
  
              _cellular_data.has_data_to_read = false;
  
              _current_states.http = HTTP_WAIT_FOR_MORE_DATA_STATE;
              _current_action = IDLE;
              _command_attempts_number = 0;
  
              /* Just disable it until next command tx */
              _tx_command_unsuccessful_timestamp = 0;
  
              /* Wait for upper layer */
              _cellular_module_restart_timestamp = 0;
  
              http_starting_waiting_time_for_new_data = current_timestamp;
            } else {
              _current_states.http = HTTP_READ_STATE;
              _current_action = SENDING_COMMAND;
              _command_attempts_number = 0;
  
              /* Just disable it until next command tx */
              _tx_command_unsuccessful_timestamp = 0;
            }
          }
        }
      } else if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, HTTP_READ_COMMAND(_simcom_module_version_used));
  
        char temp_char_array[20];
        memset(temp_char_array, '\0', 20);
        utils_itoa(_cellular_data.downloaded_data_total_read_size, temp_char_array, 10);
  
        strcat(command, temp_char_array);
        strcat(command, ",");
  
        memset(temp_char_array, '\0', 20);
        if (_simcom_module_version_used == SIMCOM_7080_VERSION) {
          if ( (_cellular_data.downloaded_data_total_read_size + UPLINK_RX_BUFFER_SIZE) > 
               _cellular_data.downloaded_data_total_size ) {
            utils_itoa(_cellular_data.downloaded_data_total_size - _cellular_data.downloaded_data_total_read_size, temp_char_array, 10);
          } else {
            utils_itoa(UPLINK_RX_BUFFER_SIZE, temp_char_array, 10);
          }
        } else {
          utils_itoa(UPLINK_RX_BUFFER_SIZE, temp_char_array, 10);
        }
  
        strcat(command, temp_char_array);
        strcat(command, "\r");
  
        cellular_send_new_command(command, HTTP_READ_RESPONSE(_simcom_module_version_used),
            RESPONSE_WAITING_TIME_30S,
            COMMAND_RETRY_TIME_5000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        struct simcom_responses last_completed_transaction;
        simcom_get_last_completed_transaction(_last_command,&last_completed_transaction);
  
        if (last_completed_transaction.responses_size > 0) {
          char *temp;
          temp = strstr(last_completed_transaction.responses,
              HTTP_READ_RESPONSE(_simcom_module_version_used));
  
          if (temp != NULL) {
            /* Get downloaded data size */
            temp = strchr(temp, ' ') + 1;
            uint64_t last_read_size = utils_atoll(temp);
  
            if (last_read_size > 0) {
              /* Get downloaded data chunck */
              temp = strchr(temp, '\n') + 1;
  
              /* The size that was asked isn't the one we received. Try again upon timeout.
               * The calculation has a plus two because of the "\r\n" in the end */
              if ((strlen(temp) >= last_read_size) && (temp[last_read_size] == '\r')) {
                memset(_cellular_data.downloaded_data, '\0', RX_BUFFER_SIZE);
                memcpy(_cellular_data.downloaded_data, temp, last_read_size);
  
                /* Remove the final '\n' if present, i.e. end string in that position */
                for (uint64_t j = 0; j < last_read_size; j++) {
                  if ( (_cellular_data.downloaded_data[j] == '\n') || 
                       (_cellular_data.downloaded_data[j] == '\r') ) {
                    _cellular_data.downloaded_data[j] = '\0';
                    break;
                  }
                }
  
                if (strlen(_cellular_data.downloaded_data) == last_read_size) {
                  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_http_state_machine] read is ok\r\n");
  
                  _cellular_data.downloaded_data_last_read_size = last_read_size;
                  _cellular_data.downloaded_data_total_read_size += last_read_size;
                  _cellular_data.has_downloaded_data = true;
  
                  cellular_disable_timer_callback();
  
                  _current_states.http = HTTP_READ_STATE;
                  _current_action = IDLE;
  
                  /* Just disable it until next command tx */
                  _tx_command_unsuccessful_timestamp = 0;
  
                  /* Wait for upper layer */
                  _cellular_module_restart_timestamp = 0;
                } else {
                  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                  debug_print_string(DEBUG_LEVEL_2, 
                      (uint8_t *)"[cellular_http_state_machine] 2nd read procedure was corrupted. try again later\r\n");
                }
              } else {
                debug_print_time(DEBUG_LEVEL_2, current_timestamp);
                debug_print_string(DEBUG_LEVEL_2, 
                    (uint8_t *)"[cellular_http_state_machine] 1st read procedure was corrupted. try again later\r\n");
              }
            }
          }
        }
      }
      break;
    }
  
    case HTTP_WAIT_FOR_MORE_DATA_STATE: {
      if (_current_action == IDLE) {
        if (_cellular_data.has_data_to_upload && (_collect_cells || _collect_gps)) {
          debug_print_time(DEBUG_LEVEL_0, current_timestamp);
          debug_print_string(DEBUG_LEVEL_0, 
              (uint8_t *)"[cellular_http_state_machine] we received more data but... we have now to collect cells and gps. we will resume after that\r\n");
  
          cellular_disable_timer_callback();
  
          _current_states.http = HTTP_TERM_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
  
          break;
        }
        if (_cellular_data.has_data_to_upload) {
          debug_print_time(DEBUG_LEVEL_0, current_timestamp);
          debug_print_string(DEBUG_LEVEL_0, 
              (uint8_t *)"[cellular_http_state_machine] more data received so let's go again without closing the http session\r\n");
  
          cellular_disable_timer_callback();
  
          _current_states.http = HTTP_STARTING_STATE;
          _current_action = IDLE;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
  
          break;
        } else if ((current_timestamp - http_starting_waiting_time_for_new_data) >= HTTP_TIMEOUT_WAIT_FOR_MORE_DATA) {
          debug_print_time(DEBUG_LEVEL_0, current_timestamp);
          debug_print_string(DEBUG_LEVEL_0, 
              (uint8_t *)"[cellular_http_state_machine] we didn't receive more data. close http session please\r\n");
  
          cellular_disable_timer_callback();
  
          _current_states.http = HTTP_TERM_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
  
          break;
        }
      }
      break;
    }
  
    case HTTP_TERM_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_TERM_COMMAND(_simcom_module_version_used),
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_30S,
            COMMAND_RETRY_TIME_5000MS, false, current_timestamp,
            WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        if (_simcom_module_version_used == SIMCOM_7020_VERSION) {
          _cellular_data.http_init = false;
  
          /* Prevention! */
          _cellular_data.has_data_to_read = false;
          _cellular_data.has_downloaded_data = false;
          _cellular_data.downloaded_data_last_read_size = 0;
          _cellular_data.downloaded_data_total_size = 0;
          _cellular_data.downloaded_data_total_read_size = 0;
          _skip_downloaded_data = false;
  
          cellular_disable_timer_callback();
  
          _current_states.http = HTTP_DESTROY_STATE;
          _current_action = SENDING_COMMAND;
          _command_attempts_number = 0;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
        } else { /* Else if not SIMCOM_7020_VERSION */
          _cellular_data.http_init = false;

          /* FIR-770 - GSM Error 603 correction */
          _cellular_data.https_set =false;

          cellular_disable_timer_callback();
  
          _current_states.http = HTTP_FINISHED_STATE;
          _current_action = IDLE;
  
          /* Prevention! */
          _cellular_data.has_data_to_read = false;
          _cellular_data.has_downloaded_data = false;
          _cellular_data.downloaded_data_last_read_size = 0;
          _cellular_data.downloaded_data_total_size = 0;
          _cellular_data.downloaded_data_total_read_size = 0;
          _skip_downloaded_data = false;
  
          /* Just disable it until next command tx */
          _tx_command_unsuccessful_timestamp = 0;
  
          /* Just prevention between phases transition */
          _cellular_module_restart_timestamp = current_timestamp;
          _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
        }
      }
      break;
    }
  
    case HTTP_DESTROY_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_DESTROY_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_30S,
            COMMAND_RETRY_TIME_5000MS, false, current_timestamp,
            WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_FINISHED_STATE;
        _current_action = IDLE;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
  
        /* Just prevention between phases transition */
        _cellular_module_restart_timestamp = current_timestamp;
        _cellular_module_restart_timeout = cellular_RESTART_MODULE_DEFAULT_TIMEOUT;
      }
      break;
    }
  
    case HTTP_SSL_CONFIGURATION_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_SSL_CONFIGURATION_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_SSL_VERIFICATION_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_SSL_VERIFICATION_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_SSL_VERIFICATION_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_15S,
            COMMAND_RETRY_TIME_5000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_SET_CONNECT_SERVER_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_SET_CONNECT_SERVER_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_SET_CONNECT_SERVER_COMMAND(_machinates_version, _https_on),
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_SET_BODY_LEN_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_SET_BODY_LEN_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_SET_BODY_LEN_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_SET_HEADER_LEN_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_SET_HEADER_LEN_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_SET_HEADER_LEN_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_CONNECT_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_CONNECT_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_CONNECT_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_60S,
            COMMAND_RETRY_TIME_15000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_GET_STATUS_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_GET_STATUS_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_GET_STATUS_COMMAND,
            HTTP_GET_STATUS_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_CLEAR_HEADER_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_CLEAR_HEADER_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_CLEAR_HEADER_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_SET_HEADER_2_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_SET_HEADER_1_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_SET_CONTENT_TYPE_HEADER_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_SET_HEADER_2_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_SET_HEADER_2_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_SET_CACHE_CONTROL_HEADER_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_SET_HEADER_3_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_SET_HEADER_3_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_SET_CONNECTION_HEADER_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_SET_HEADER_4_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_SET_HEADER_4_STATE: {
      if (_current_action == SENDING_COMMAND) {
        cellular_send_new_command(HTTP_SET_ACCEPT_HEADER_COMMAND,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_SET_HEADER_5_STATE;
        _current_action = SENDING_COMMAND;
        _command_attempts_number = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_SET_HEADER_5_STATE: {
      if (_current_action == SENDING_COMMAND) {
        char command[COMMAND_SIZE];
        memset(command, '\0', COMMAND_SIZE);
  
        strcpy(command, HTTP_SET_VERSION_HEADER_COMMAND);
  
        char temp_char_array[20];
        memset(temp_char_array, '\0', 20);
        utils_itoa(_machinates_version, temp_char_array, 10);
  
        strcat(command, temp_char_array);
        strcat(command, "\"\r");
  
        cellular_send_new_command(command,
            GENERIC_OK_RESPONSE,
            RESPONSE_WAITING_TIME_10S,
            COMMAND_RETRY_TIME_2000MS,
            false, current_timestamp, WRONG_RESPONSE_DEFAULT);
      } else if (_current_action == GOOD_RESPONSE) {
        _cellular_data.http_init = true;
  
        cellular_disable_timer_callback();
  
        _current_states.http = HTTP_UPLOAD_DATA_STATE;
        _current_action = IDLE;
        _cellular_data.is_time_to_set_data_to_upload = true;
  
        /* Wait for upper layer */
        _cellular_module_restart_timestamp = 0;
  
        /* Just disable it until next command tx */
        _tx_command_unsuccessful_timestamp = 0;
      }
      break;
    }
  
    case HTTP_FINISHED_STATE: {
      break;
    }
  
    default: {
      _current_states.http = HTTP_IDLE_STATE;
      _current_action = IDLE;
    }
  }
}

/**
 * @brief   The function that will run the cellular phase machine
 */
void cellular_phase_machine(uint64_t current_timestamp) {
  if ( ( (_current_phase != IDLE_PHASE) && (_current_phase != GPS_PHASE) ) && 
         !_cellular_data.warmup_success ) {
    /* Let's starting! */
    if (_current_states.warmup == WARMUP_IDLE_STATE) {
      _current_states.warmup = WARMUP_STARTING_STATE;
    }
    cellular_warm_up_state_machine(current_timestamp);
    _last_warm_up_state_machine_execution_timestamp = current_timestamp;
    /* Finished detected, let's go idle! */
    if (_current_states.warmup == WARMUP_FINISHED_STATE) {
      _current_states.warmup = WARMUP_IDLE_STATE;
    }
    return;
  } else if ( ((_current_phase != IDLE_PHASE) && (_current_phase != GPS_PHASE) ) && 
              !_cellular_data.info_success ) {
    /* Let's starting! */
    if (_current_states.info == INFO_IDLE_STATE) {
      _current_states.info = INFO_STARTING_STATE;
    }
    cellular_info_state_machine(current_timestamp);
    _last_info_state_machine_execution_timestamp = current_timestamp;
    /* Finished detected, let's go idle! */
    if (_current_states.info == INFO_FINISHED_STATE) {
      _current_states.info = INFO_IDLE_STATE;
    }
    return;
  }

  switch (_current_phase) {
    case IDLE_PHASE: {
      _last_idle_state_machine_execution_timestamp = current_timestamp;
  
      /* Debug print */
      if ((current_timestamp - _last_timers_status_print_timestamp) > 300000) {
        if (_registration_failed_timestamp != 0) {
          debug_print_time(DEBUG_LEVEL_0, current_timestamp);
          debug_print_string(DEBUG_LEVEL_0, 
              (uint8_t *)"[cellular_phase_machine] fyi registration failed timer is still active. please wait ");
          char xpto[6];
          memset(xpto, '\0', 6);
          utils_itoa(
              (cellular_get_registration_failed_next_try_timeout() - (current_timestamp - _registration_failed_timestamp)),
              xpto, 10);
          debug_print_string(DEBUG_LEVEL_0, (uint8_t *)xpto);
          debug_print_string(DEBUG_LEVEL_0, (uint8_t *)" ms\r\n");
        }
  
        if (_session_failed_timestamp != 0) {
          debug_print_time(DEBUG_LEVEL_0, current_timestamp);
          debug_print_string(DEBUG_LEVEL_0, 
              (uint8_t *)"[cellular_phase_machine] fyi session failed timer is still active. please wait ");
          char xpto[6];
          memset(xpto, '\0', 6);
          utils_itoa(
              (cellular_get_session_failed_next_try_timeout() - (current_timestamp - _session_failed_timestamp)),
              xpto, 10);
          debug_print_string(DEBUG_LEVEL_0, (uint8_t *)xpto);
          debug_print_string(DEBUG_LEVEL_0, (uint8_t *)" ms\r\n");
        }
  
        if (_http_code_failed_timestamp != 0) {
          debug_print_time(DEBUG_LEVEL_0, current_timestamp);
          debug_print_string(DEBUG_LEVEL_0, 
              (uint8_t *)"[cellular_phase_machine] fyi http failed timer is still active. please wait ");
          char xpto[6];
          memset(xpto, '\0', 6);
          utils_itoa(
              (cellular_get_http_bad_code_next_try_timeout() - (current_timestamp - _http_code_failed_timestamp)), 
              xpto, 10);
          debug_print_string(DEBUG_LEVEL_0, (uint8_t *)xpto);
          debug_print_string(DEBUG_LEVEL_0, (uint8_t *)" ms\r\n");
        }
  
        if ( (_registration_failed_timestamp != 0) && (_session_failed_timestamp != 0) && 
             (_http_code_failed_timestamp != 0) ) {
          debug_print_time(DEBUG_LEVEL_2, current_timestamp);
          debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_phase_machine] there's no active timers now\r\n");
        }
  
        _last_timers_status_print_timestamp = current_timestamp;
      }
  
      /* Check registration timeout */
      if ( (_registration_failed_timestamp != 0) && 
           (current_timestamp - _registration_failed_timestamp >= cellular_get_registration_failed_next_try_timeout()) ) {
        _registration_failed_timestamp = 0;
        debug_print_time(DEBUG_LEVEL_0, current_timestamp);
        debug_print_string(DEBUG_LEVEL_0, 
            (uint8_t *)"[cellular_phase_machine] cellular registration failed timeout expired. let's try again!\r\n");
      }
  
      /* Check session timeout */
      if ( (_session_failed_timestamp != 0) &&
           (current_timestamp - _session_failed_timestamp >= cellular_get_session_failed_next_try_timeout()) ) {
        _session_failed_timestamp = 0;
        debug_print_time(DEBUG_LEVEL_0, current_timestamp);
        debug_print_string(DEBUG_LEVEL_0, 
            (uint8_t *)"[cellular_phase_machine] cellular session failed timeout expired. let's try again!\r\n");
      }
  
      /* Check http bad code timeout */
      if ( (_http_code_failed_timestamp != 0) && 
           (current_timestamp - _http_code_failed_timestamp >= cellular_get_http_bad_code_next_try_timeout()) ) {
        _http_code_failed_timestamp = 0;
        debug_print_time(DEBUG_LEVEL_0, current_timestamp);
        debug_print_string(DEBUG_LEVEL_0, 
            (uint8_t *)"[cellular_phase_machine] cellular http bad code timeout expired. let's try again!\r\n");
      }
  
      /* Don't power on cellular if the cellular registration or session timeout isn't expired yet.
       * Just wait for now... */
      if ( ( ((_registration_failed_timestamp != 0) || (_session_failed_timestamp != 0) || (_http_code_failed_timestamp != 0) ) && !_collect_gps) && 
           ( ( ((_session_failed_timestamp != 0) || (_http_code_failed_timestamp != 0)) && !_collect_cells ) || 
             ( (_registration_failed_timestamp != 0) && _collect_cells ) ) ) {
        /* Check power. It doens't make sense to have module powered
         * on if we are waiting for something to happen and it may take some time... */
        if (_powered_on && !_powering_on) {
          debug_print_time(DEBUG_LEVEL_2, current_timestamp);
          debug_print_string(DEBUG_LEVEL_2, 
              (uint8_t *)"[cellular_phase_machine] power off module please... we are still waiting for timers to expire. we need to save power until it happens\r\n");
  
          cellular_disable_timer_callback();
          _cellular_module_restart_timestamp = 0;
  
          //cellular_power_off(current_timestamp);
          //TODO: Use this later. Too risky and I don't have time to test it now
          cellular_start_power_off(current_timestamp);
        }
  
        /* Do nothing */
        ;
      }
      /* Don't power on cellular if the cellular data upload timeout isn't expired yet.
       * Just wait for now... */
      else if (_collect_gps) {
        debug_print_time(DEBUG_LEVEL_2, current_timestamp);
        debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_phase_machine] go gps...\r\n");
  
        if (!_powered_on && !_powering_on) {
          //cellular_power_on(current_timestamp);
          //TODO: Use this later. Too risky and I don't have time to test it now
          cellular_start_power_on(current_timestamp);
          _cellular_data.warmup_success = false;
          _cellular_data.info_success = false;
          _last_time_phase_set_timestamp = current_timestamp;
        }
        //    else if (_powered_on && _cellular_read_status_callback() && cellular_has_psm_active()) {
        //      cellular_wakeup_from_psm(current_timestamp);
        //    }
        cellular_set_phase(GPS_PHASE, current_timestamp);
      } else if (_collect_cells) {
        debug_print_time(DEBUG_LEVEL_2, current_timestamp);
        debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_phase_machine] go cells...\r\n");
  
        if (!_powered_on && !_powering_on) {
          //cellular_power_on(current_timestamp);
          //TODO: Use this later. Too risky and I don't have time to test it now
          cellular_start_power_on(current_timestamp);
          _cellular_data.warmup_success = false;
          _cellular_data.info_success = false;
          _last_time_phase_set_timestamp = current_timestamp;
        } else if ( _powered_on && _cellular_read_status_callback() && cellular_has_psm_active() ) {
          cellular_wakeup_from_psm(current_timestamp);
          _wakeup_from_psm_by_cells = true;
        }
        cellular_set_phase(CELLS_PHASE, current_timestamp);
      } else if (_current_states.data_session == DATA_SESSION_REMAIN_OPEN) {
        debug_print_time(DEBUG_LEVEL_2, current_timestamp);
        debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_phase_machine] go data remain open...\r\n");
  
        cellular_set_phase(DATA_SESSION_PHASE, current_timestamp);
        /* There is data to send do Machinates? Yes? Let's turn on the cellular! */
      } else if (_cellular_data.has_data_to_upload) {
        debug_print_time(DEBUG_LEVEL_2, current_timestamp);
        debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_phase_machine] go send data...\r\n");
  
        if (!_powered_on && !_powering_on) {
          //cellular_power_on(current_timestamp);
          //TODO: Use this later. Too risky and I don't have time to test it now
          cellular_start_power_on(current_timestamp);
          _cellular_data.warmup_success = false;
          _cellular_data.info_success = false;
          _last_time_phase_set_timestamp = current_timestamp;
        }
        cellular_set_phase(DATA_SESSION_PHASE, current_timestamp);
      } else {
        if (_powered_on && !_powering_on) {
          debug_print_time(DEBUG_LEVEL_2, current_timestamp);
          debug_print_string(DEBUG_LEVEL_2, 
              (uint8_t *)"[cellular_phase_machine] power off module please... we don't have any task to be done\r\n");
  
          cellular_disable_timer_callback();
          _cellular_module_restart_timestamp = 0;
  
          //cellular_power_off(current_timestamp);
          //TODO: Use this later. Too risky and I don't have time to test it now
          cellular_start_power_off(current_timestamp);
        }
      }
      break;
    }
  
    case DATA_SESSION_PHASE: {
      /* Let's starting! */
      if (_current_states.data_session == DATA_SESSION_IDLE_STATE) {
        if (_cellular_data.has_data_to_upload && (_collect_cells || _collect_gps)) {
          debug_print_time(DEBUG_LEVEL_0, current_timestamp);
          debug_print_string(DEBUG_LEVEL_0, 
              (uint8_t *)"[cellular_phase_machine] we received more data but... we have now to collect cells and gps. we resume after that\r\n");
  
          cellular_set_phase(IDLE_PHASE, current_timestamp);
  
          break;
        }
  
        _current_states.data_session = DATA_SESSION_STARTING_STATE;
      }
      cellular_data_session_state_machine(current_timestamp);
      _last_data_session_state_machine_execution_timestamp = current_timestamp;
      if (_current_states.data_session == DATA_SESSION_REMAIN_OPEN) {
        cellular_set_phase(IDLE_PHASE, current_timestamp);
      }
      /* Finished detected, let's go idle! */
      else if (_current_states.data_session == DATA_SESSION_FINISHED_STATE) {
        if (_cellular_data.has_data_to_upload) {
          _current_states.data_session = DATA_SESSION_IDLE_STATE;
          cellular_set_phase(HTTP_PHASE, current_timestamp);
        } else {
          _current_states.data_session = DATA_SESSION_IDLE_STATE;
          cellular_set_phase(IDLE_PHASE, current_timestamp);
        }
        _cellular_data.warmup_success = false;
        _last_time_phase_set_timestamp = current_timestamp;
      }
      break;
    }
  
    case HTTP_PHASE: {
      /* Let's starting! */
      if (_current_states.http == HTTP_IDLE_STATE) {
        _current_states.http = HTTP_STARTING_STATE;
      }
      cellular_http_state_machine(current_timestamp);
      _last_http_state_machine_execution_timestamp = current_timestamp;
      /* Finished detected, let's go idle! */
      if (_current_states.http == HTTP_FINISHED_STATE) {
        _current_states.http = HTTP_IDLE_STATE;
        cellular_set_phase(DATA_SESSION_PHASE, current_timestamp);
        _cellular_data.warmup_success = false;
        _last_time_phase_set_timestamp = current_timestamp;
      }
      break;
    }
  
    case CELLS_PHASE: {
      /* Let's starting! */
      if (_current_states.cells == CELLS_IDLE_STATE) {
        _current_states.cells = CELLS_STARTING_STATE;
      }
      cellular_cells_state_machine(current_timestamp);
      _last_cells_state_machine_execution_timestamp = current_timestamp;
      /* Finished detected, let's go idle! */
      if (_current_states.cells == CELLS_FINISHED_STATE) {
        _current_states.cells = CELLS_IDLE_STATE;
        cellular_set_phase(IDLE_PHASE, current_timestamp);
      }
      break;
    }
  
    case GPS_PHASE: {
      /* Let's starting! */
      if (_current_states.gps == GPS_IDLE_STATE) {
        _current_states.gps = GPS_STARTING_STATE;
      }
      cellular_gps_state_machine(current_timestamp);
      _last_gps_state_machine_execution_timestamp = current_timestamp;
      /* Finished detected, let's go idle! */
      if (_current_states.gps == GPS_FINISHED_STATE) {
        _current_states.gps = GPS_IDLE_STATE;
        cellular_set_phase(IDLE_PHASE, current_timestamp);
      }
      break;
    }
  
    default: {
      cellular_set_phase(IDLE_PHASE, current_timestamp);
      _current_action = IDLE;
      break;
    }
  }
}

/********************************** Public ***********************************/
// Currently returning always true, i.e., assumes the module is always present.
bool cellular_setup(
    registration_failure_create_message_callback_def  registration_failure_create_message_callback,
    session_failure_create_message_callback_def       session_failure_create_message_callback,
    http_failure_create_message_callback_def          http_failure_create_message_callback,
    timer_3346_create_message_callback_def            timer_3346_create_message_callback,
    sensoroid_send_hello_msg_callback_def             sensoroid_send_hello_msg_callback,
    cellular_init_vbat_ctrl_callback_def              cellular_init_vbat_ctrl_callback,
    cellular_clear_vbat_ctrl_callback_def             cellular_clear_vbat_ctrl_callback,
    cellular_set_vbat_ctrl_callback_def               cellular_set_vbat_ctrl_callback,
    cellular_init_power_key_callback_def              cellular_init_power_key_callback,
    cellular_clear_power_key_callback_def             cellular_clear_power_key_callback,
    cellular_set_power_key_callback_def               cellular_set_power_key_callback,
    cellular_init_lvl_sh_oe_callback_def              cellular_init_lvl_sh_oe_callback,
    cellular_clear_lvl_sh_oe_callback_def             cellular_clear_lvl_sh_oe_callback,
    cellular_set_lvl_sh_oe_callback_def               cellular_set_lvl_sh_oe_callback,
    cellular_init_status_callback_def                 cellular_init_status_callback,
    cellular_read_status_callback_def                 cellular_read_status_callback,
    power_management_is_battery_saving_mode_callback_def  power_management_is_battery_saving_mode_callback,
    sysclk_get_in_ms_callback_def                     sysclk_get_in_ms_callback,
    module_communication_init_callback_def            module_communication_init,
    module_communication_shutdown_callback_def        module_communication_shutdown) {

  _registration_failure_create_message_callback     = registration_failure_create_message_callback;
  _session_failure_create_message_callback          = session_failure_create_message_callback;
  _http_failure_create_message_callback             = http_failure_create_message_callback;
  _timer_3346_create_message_callback               = timer_3346_create_message_callback;
  _sensoroid_send_hello_msg_callback                = sensoroid_send_hello_msg_callback;

  _cellular_init_vbat_ctrl_callback                 = cellular_init_vbat_ctrl_callback;
  _cellular_clear_vbat_ctrl_callback                = cellular_clear_vbat_ctrl_callback;
  _cellular_set_vbat_ctrl_callback                  = cellular_set_vbat_ctrl_callback;

  _cellular_init_power_key_callback                 = cellular_init_power_key_callback;
  _cellular_clear_power_key_callback                = cellular_clear_power_key_callback;
  _cellular_set_power_key_callback                  = cellular_set_power_key_callback;

  _cellular_init_lvl_sh_oe_callback                 = cellular_init_lvl_sh_oe_callback;
  _cellular_clear_lvl_sh_oe_callback                = cellular_clear_lvl_sh_oe_callback;
  _cellular_set_lvl_sh_oe_callback                  = cellular_set_lvl_sh_oe_callback;

  _cellular_init_status_callback                    = cellular_init_status_callback;
  _cellular_read_status_callback                    = cellular_read_status_callback;

  _power_management_is_battery_saving_mode_callback = power_management_is_battery_saving_mode_callback;

  _sysclk_get_in_ms_callback                        = sysclk_get_in_ms_callback;

  _module_communication_init                        = module_communication_init;
  _module_communication_shutdown                    = module_communication_shutdown;

  _current_registration_status_is_ok = true;
  _current_data_session_status_is_ok = true;
  _current_http_status_is_ok = true;

  _rssi_info.rssi = 0;
  _rssi_info.rssi_available = false;
  _rssi_info.ber = 0;
  _rssi_info.ber_available = false;
  _rssi_info.last_read_timestamp = 0;

  _gps_turn_on_module_timestamp = 0;

  cellular_reset_variables();
  _cellular_init_vbat_ctrl_callback();
  _cellular_init_power_key_callback();
  _cellular_init_lvl_sh_oe_callback();
  _cellular_init_status_callback();
  _cellular_clear_vbat_ctrl_callback();
  _vbat_ctrl_current_state = false;
  _cellular_clear_lvl_sh_oe_callback();
  _cellular_clear_power_key_callback();

  /* Clear variables */
  memset(&(_cellular_data.simcom_version), '\0', SIMCOM_VERSION_MAX_LENGTH);

  /* Start module communication test to know if there is an available SIMCom module */
  cellular_start_power_off(_sysclk_get_in_ms_callback());
  //  cellular_power_on(_sysclk_get_in_ms_callback());
  //
  //  cellular_send_new_command(AT_TEST_COMMAND(_simcom_module_version_used),
  //    GENERIC_OK_RESPONSE,
  //    RESPONSE_WAITING_TIME_5S, COMMAND_RETRY_TIME_500MS, false,
  //    _sysclk_get_in_ms_callback(), WRONG_RESPONSE_DEFAULT);
  //
  //  uint64_t at_check_timestamp = _sysclk_get_in_ms_callback();
  bool ret = true;
  //
  //  while((_current_action != GOOD_RESPONSE) && ((_sysclk_get_in_ms_callback() - at_check_timestamp)) < SIMCOM_INIT_TEST_TIMEOUT) {
  //    cellular_process_pending_messages_task(_sysclk_get_in_ms_callback());
  //  }
  //
  //  /* SIMCom module is there and available! */
  //  if(_current_action == GOOD_RESPONSE) {
  //    ret = true;
  //  }
  //
  //  cellular_power_off(_sysclk_get_in_ms_callback());
  //
  //  /* Clear variables again */
  //	cellular_reset_variables();
  //	_cellular_init_vbat_ctrl_callback();
  //	_cellular_init_power_key_callback();
  //	_cellular_init_lvl_sh_oe_callback();
  //	_cellular_init_status_callback();
  //	_cellular_clear_vbat_ctrl_callback();
  //	_vbat_ctrl_current_state = false;
  //	_cellular_clear_lvl_sh_oe_callback();
  //	_cellular_clear_power_key_callback();
  //
  //	/* Clear variables */
  //	memset(&(_cellular_data.simcom_version), '\0', SIMCOM_VERSION_MAX_LENGTH);

  return ret;
}

void cellular_loop(uint64_t  current_timestamp, uint64_t  remain_data_session_open,
    uint64_t  registration_failed_next_try,     uint16_t  machinates_version,
    uint8_t   https_on, bool      gps_on) {

  if ((_timer_timeout != 0) && ((current_timestamp - _timer_timestamp) >= _timer_timeout)) {
    cellular_timer_expired();
  }

  if ((current_timestamp - _power_on_number_print_timestamp) >= PRINT_TIMEOUT) {
    cellular_print_current_internal_state(current_timestamp);
    _power_on_number_print_timestamp = current_timestamp;
  }

  if (_remain_data_session_open != remain_data_session_open) {
    _remain_data_session_open = remain_data_session_open;

    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_loop] remain data session open: ");
    if (_remain_data_session_open != 0) {
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"true (time=");
      char remain_session_open[6];
      memset(remain_session_open, '\0', 6);
      utils_itoa(_remain_data_session_open, remain_session_open, 10);
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)remain_session_open);
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"ms)\r\n");
    } else {
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"false\r\n");
    }
  }
  if (_registration_failed_next_try_maximum_timeout != registration_failed_next_try) {
    _registration_failed_next_try_maximum_timeout =
        registration_failed_next_try;

    _session_failed_next_try_maximum_timeout = registration_failed_next_try;

    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_loop] registration failed next try: ");
    char reg_failed_next_try[6];
    memset(reg_failed_next_try, '\0', 6);
    utils_itoa(_registration_failed_next_try_maximum_timeout, reg_failed_next_try, 10);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)reg_failed_next_try);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"ms\r\n");

    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_loop] session failed next try: ");
    memset(reg_failed_next_try, '\0', 6);
    utils_itoa(_session_failed_next_try_maximum_timeout, reg_failed_next_try, 10);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)reg_failed_next_try);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"ms\r\n");
  }
  if (_machinates_version != machinates_version) {
    _machinates_version = machinates_version;

    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_setup] machinates version: ");
    char machinates_version_str[6];
    memset(machinates_version_str, '\0', 6);
    utils_itoa(_machinates_version, machinates_version_str, 10);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)machinates_version_str);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"ms\r\n");
  }
  if (_https_on != https_on) {
    _https_on = https_on;

    if (_https_on == 0) {
      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_setup] https is disabled!\r\n");
    } else {
      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_setup] https is enabled!\r\n");
    }
  }
  if (_gps_on != gps_on) {
    _gps_on = gps_on;

    if (!_gps_on) {
      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_setup] gps is disabled!\r\n");
    } else {
      debug_print_time(DEBUG_LEVEL_2, current_timestamp);
      debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_setup] gps is enabled!\r\n");
    }
  }

  //TODO: Use this later. Too risky and I don't have time to test it now
  /* We are powering on */
  if (_powering_on) {
    cellular_power_on_loop(current_timestamp);
    /* Nothing to do now */
    return;
  }

  /* We are powering off */
  if (_powering_off) {
    cellular_power_off_loop(current_timestamp);
    /* Nothing to do now */
    return;
  }

  /* We need to finish PSM exit */
  if (_psm_wakeup_now) {
    if ( (_psm_wakeup_timestamp != 0) && 
         ((rtc_get_milliseconds() - _psm_wakeup_timestamp) > 1000) ) {
      _cellular_clear_power_key_callback();

      if (cellular_get_simcom_version() == SIMCOM_7080_VERSION) {
        _psm_configured = false;
        _nbiot_apn_configured = false;
      }

      debug_print_time(DEBUG_LEVEL_0, current_timestamp);
      debug_print_string(DEBUG_LEVEL_0, 
          (uint8_t *)"[cellular_wakeup_from_psm] wakeup from psm ended!\r\n");

      _psm_wakeup_timestamp = 0;
      _psm_wakeup_now = false;
    }
    return;
  }

  if ( (current_timestamp - _psm_print_timestamp) > PRINT_TIMEOUT) {
    _psm_print_timestamp = current_timestamp;

    if (_powered_on && _cellular_read_status_callback() && cellular_has_psm_active()) {
      debug_print_time(DEBUG_LEVEL_0, current_timestamp);
      debug_print_string(DEBUG_LEVEL_0, 
          (uint8_t *)"[cellular_loop] read status is off and we are in psm, so everything is ok, we are just saving battery!\r\n");
    }
  }

  /* Error response detected so let's go quick try again! */
  if (_current_action == ERROR_RESPONSE) {
    cellular_timer_expired();

    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    debug_print_string(DEBUG_LEVEL_2, 
        (uint8_t *)"[cellular_loop] error response detected! let's send command again baby!\r\n");
  }

  if ( (_current_action == BAD_RESPONSE) || 
       (_powered_on && _cellular_read_status_callback() && !cellular_has_psm_active()) ) {
    if (_powered_on && _cellular_read_status_callback()) {
      debug_print_time(DEBUG_LEVEL_0, current_timestamp);
      debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"[cellular_loop] read status is off!\r\n");
    } else {
      debug_print_time(DEBUG_LEVEL_0, current_timestamp);
      debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"[cellular_loop] bad response!\r\n");
    }

    //TODO: Finish specific behaviour! Current is restart cellular...
    cellular_disable_timer_callback();
    _cellular_module_restart_timestamp = 0;

    /* I don't have available cells for now, i.e. registration failed */
    if ( !_cellular_data.warmup_success && 
         ( _current_states.warmup == WARMUP_IS_REGISTERED_STATE || (_current_states.warmup == WARMUP_GET_CSQ_STATE) || 
         ( (cellular_get_simcom_version() == SIMCOM_7080_VERSION) && (_current_states.warmup == WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_6) ) ) ) {
      /* Update status */
      if (_current_registration_status_is_ok) {
        _current_registration_status_is_ok = false;
      }

      /* Refresh RSSI value */
      _rssi_info.rssi = RSSI_NO_CONNECTION_VALUE;
      _rssi_info.rssi_available = true;
      _rssi_info.ber_available = false;
      _rssi_info.last_read_timestamp = current_timestamp;

      /* Check if it's enable */
      if (_registration_failed_next_try_maximum_timeout != 0) {
        _registration_failed_timestamp = current_timestamp;

        /* Increments registration timeout */
        cellular_increment_registration_failed_next_try_timeout(current_timestamp, _last_registration_timeout);

        debug_print_time(DEBUG_LEVEL_0, current_timestamp);
        debug_print_string(DEBUG_LEVEL_0, 
            (uint8_t *)"[cellular_loop] cellular registration failed. Try again after ");
        char seconds_retry_again[10];
        memset(seconds_retry_again, '\0', 10);
        utils_itoa(cellular_get_registration_failed_next_try_timeout(),seconds_retry_again, 10);
        debug_print_string(DEBUG_LEVEL_0, (uint8_t *)seconds_retry_again);
        debug_print_string(DEBUG_LEVEL_0, (uint8_t *)" ms\r\n");
      }
    }

    /* I didn't open data session. Maybe I don't have more data plafond */
    if ( (_current_phase == DATA_SESSION_PHASE) && (_current_states.data_session == DATA_SESSION_OPENING_STATE) ) {
      /* Update status */
      if (_current_data_session_status_is_ok) {
        _current_data_session_status_is_ok = false;
      }

      /* Check if it's enable */
      if (_session_failed_next_try_maximum_timeout != 0) {
        _session_failed_timestamp = current_timestamp;

        /* Increments session timeout */
        cellular_increment_session_failed_next_try_timeout(current_timestamp);

        debug_print_time(DEBUG_LEVEL_0, current_timestamp);
        debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"[cellular_loop] cellular session failed. try again after ");
        char seconds_retry_again[10];
        memset(seconds_retry_again, '\0', 10);
        utils_itoa(cellular_get_session_failed_next_try_timeout(),seconds_retry_again, 10);
        debug_print_string(DEBUG_LEVEL_0, (uint8_t *)seconds_retry_again);
        debug_print_string(DEBUG_LEVEL_0, (uint8_t *)" ms\r\n");
      }
    }

    /* I didn't had an http action code */
    if ( (_current_phase == HTTP_PHASE) && (_current_states.http == HTTP_BAD_CODE_STATE) && (_http_other_error_codes > 0) ) {
      /* Update status */
      if (_current_http_status_is_ok) {
        _current_http_status_is_ok = false;
      }

      /* Check if it's enable */
      _http_code_failed_timestamp = current_timestamp;

      debug_print_time(DEBUG_LEVEL_0, current_timestamp);
      debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"[cellular_loop] cellular http bad code. try again after ");
      char seconds_retry_again[10];
      memset(seconds_retry_again, '\0', 10);
      utils_itoa(cellular_get_http_bad_code_next_try_timeout(),seconds_retry_again, 10);
      debug_print_string(DEBUG_LEVEL_0, (uint8_t *)seconds_retry_again);
      debug_print_string(DEBUG_LEVEL_0, (uint8_t *)" ms\r\n");
    }

    /*TODO: ignore variable to guarantee restart */
    //  if (_powered_on) {
    //    cellular_power_off(current_timestamp);
    //TODO: Use this later. Too risky and I don't have time to test it now
    cellular_start_power_off(current_timestamp);
    //  }
    cellular_reset_variables();

    return;
  }

  /* Message processing task */
  cellular_process_pending_messages_task(current_timestamp);

  /* Run the cellular phase machine */
  cellular_phase_machine(current_timestamp);
}

bool cellular_has_data_to_upload(void) {
  return _cellular_data.has_data_to_upload;
}

bool cellular_has_data_to_read(void) {
  return _cellular_data.has_data_to_read;
}

bool cellular_is_time_to_set_data_to_upload(void) {
  return _cellular_data.is_time_to_set_data_to_upload;
}

void cellular_set_has_data_to_upload(bool has_data_to_upload) {
  _cellular_data.has_data_to_upload = has_data_to_upload;
}

bool cellular_set_data_to_upload(char *data_to_upload, uint64_t data_to_upload_size,
    uint64_t current_timestamp) {

  if (data_to_upload_size > TX_BUFFER_SIZE) {
    debug_print_time(DEBUG_LEVEL_0, current_timestamp);
    debug_print_string(DEBUG_LEVEL_0, 
        (uint8_t *)"[cellular_set_data_to_upload] upload data exceeds tx buffer size. base64 maximum size is: ");
    char base64_maximum_size[6];
    memset(base64_maximum_size, '\0', 6);
    utils_itoa(TX_BUFFER_SIZE, base64_maximum_size, 10);
    debug_print_string(DEBUG_LEVEL_0, (uint8_t *)base64_maximum_size);
    debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"B, data to upload size is: ");
    char upload_size[6];
    memset(upload_size, '\0', 6);
    utils_itoa(data_to_upload_size, upload_size, 10);
    debug_print_string(DEBUG_LEVEL_0, (uint8_t *)upload_size);
    debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"B\r\n");

    return false;
  }

  memset(_cellular_data.data_to_upload, '\0', TX_BUFFER_SIZE);
  memcpy(_cellular_data.data_to_upload, data_to_upload, data_to_upload_size);
  _cellular_data.data_to_upload_size = data_to_upload_size;
  _cellular_data.data_successfully_uploaded = false;
  _cellular_data.is_time_to_set_data_to_upload = false;

  debug_print_time(DEBUG_LEVEL_0, current_timestamp);
  debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"[cellular_set_data_to_upload] upload data content: ");
  debug_print_string(DEBUG_LEVEL_0, (uint8_t *)data_to_upload);
  debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"\r\n");

  return true;
}

bool cellular_data_successfully_uploaded(void) {
  return _cellular_data.data_successfully_uploaded;
}

bool cellular_has_downloaded_data(void) {
  return _cellular_data.has_downloaded_data;
}

uint8_t cellular_get_downloaded_data(char *downloaded_data) {
  memset(downloaded_data, '\0', RX_BUFFER_SIZE);

  if (!_cellular_data.has_downloaded_data)
    return 0;

  memcpy(downloaded_data, _cellular_data.downloaded_data,
      _cellular_data.downloaded_data_last_read_size);

  memset(_cellular_data.downloaded_data, '\0', RX_BUFFER_SIZE);
  _cellular_data.has_downloaded_data = false;

  return _cellular_data.downloaded_data_last_read_size;
}

void cellular_collect_cells(uint64_t current_timestamp) {
  if ( (cellular_get_simcom_version() == SIMCOM_800_VERSION)  || (cellular_get_simcom_version() == SIMCOM_7020_VERSION) || 
       (cellular_get_simcom_version() == SIMCOM_7080_VERSION) || (cellular_get_simcom_version() == SIMCOM_7600_VERSION) || 
       (cellular_get_simcom_version() == SIMCOM_868_VERSION)) {
    _collect_cells = true;

    debug_print_time(DEBUG_LEVEL_0, current_timestamp);
    debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"[cellular_collect_cells] collect data now\r\n");
  }
}

bool cellular_has_cells_data(void) {
  return _cells_data.available;
}

void cellular_read_cells_data(struct cells_data *cells_info) {
  cells_info->available           = _cells_data.available;
  cells_info->cells_type          = _cells_data.cells_type;
  cells_info->cells_number        = _cells_data.cells_number;
  cells_info->mnc                 = _cells_data.mnc;
  cells_info->mcc                 = _cells_data.mcc;
  cells_info->last_read_timestamp = _cells_data.last_read_timestamp;

  uint8_t i;
  for (i = 0; i < cells_info->cells_number; i++) {
    cells_info->lac[i]            = _cells_data.lac[i];
    cells_info->ids[i]            = _cells_data.ids[i];
    cells_info->rx_levels[i]      = _cells_data.rx_levels[i];
  }

  _cells_data.available = false;
}

/* Currently not being used */
void cellular_read_cells_data_without_clear_flag(struct cells_data *cells_info) {
  cells_info->available           = _cells_data.available;
  cells_info->cells_type          = _cells_data.cells_type;
  cells_info->cells_number        = _cells_data.cells_number;
  cells_info->mnc                 = _cells_data.mnc;
  cells_info->mcc                 = _cells_data.mcc;

  uint8_t i;
  for (i = 0; i < cells_info->cells_number; i++) {
    cells_info->lac[i]            = _cells_data.lac[i];
    cells_info->ids[i]            = _cells_data.ids[i];
    cells_info->rx_levels[i]      = _cells_data.rx_levels[i];
  }
}

char *cellular_get_imei(void) {
  return _cellular_data.imei;
}

bool cellular_is_powered_on(void) {
  return _powered_on;
}

void cellular_set_skip_downloaded_data(bool skip_downloaded_data,
    uint64_t current_timestamp) {

  if (skip_downloaded_data) {
    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_set_skip_downloaded_data] enabled");
  } else {
    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_set_skip_downloaded_data] disabled");
  }

  _skip_downloaded_data = skip_downloaded_data;
}

bool cellular_has_time_took_to_register_on_network(void) {
  return _time_to_register_on_the_network_collected;
}

uint64_t cellular_get_time_took_to_register_on_network(void) {
  return _last_time_to_register_on_the_network;
}

bool cellular_has_register_cell_on_network(void) {
  return _registered_cell_on_network_collected;
}

SIMCOM_Module_Version cellular_get_simcom_version(void) {
  return _simcom_module_version_used;
}

void cellular_start_collecting_gps(uint64_t current_timestamp) {
  if ( _gps_on && 
       ((_simcom_module_version_used == SIMCOM_7600_VERSION) || (_simcom_module_version_used == SIMCOM_868_VERSION)) && 
       !_collect_gps ) {
    _collect_gps = true;

    debug_print_time(DEBUG_LEVEL_0, current_timestamp);
    debug_print_string(DEBUG_LEVEL_0, 
        (uint8_t *)"[cellular_collect_gps] start GPS data collection! maximum internal timeout is: ");
    char timeout[6];
    memset(timeout, '\0', 6);
    utils_itoa(GPS_RX_VALID_FIX_MAXIMUM_TIMEOUT, timeout, 10);
    debug_print_string(DEBUG_LEVEL_0, (uint8_t *)timeout);
    debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"ms\r\n");
  } else {
    debug_print_time(DEBUG_LEVEL_0, current_timestamp);
    debug_print_string(DEBUG_LEVEL_0, 
        (uint8_t *)"[cellular_collect_gps] not allowed. SIMCOM model doesn't support GPS or GPS is being collected right now\r\n");
  }
}

void cellular_stop_collecting_gps(uint64_t current_timestamp) {
  if ( ( (_simcom_module_version_used == SIMCOM_7600_VERSION) || (_simcom_module_version_used == SIMCOM_868_VERSION) ) &&
       _collect_gps && (_current_phase == GPS_PHASE) && (_current_states.gps == GPS_START_RMC_SENTENCE_MODE_STATE) && 
       (_current_action == STAND_ALONE_RESPONSE) ) {
    _current_action = GOOD_RESPONSE;

    debug_print_time(DEBUG_LEVEL_0, current_timestamp);
    debug_print_string(DEBUG_LEVEL_0, (uint8_t *)"[cellular_collect_gps] stop GPS collection!\r\n");
  } else {
    debug_print_time(DEBUG_LEVEL_0, current_timestamp);
    debug_print_string(DEBUG_LEVEL_0, 
        (uint8_t *)"[cellular_collect_gps] not allowed. SIMCOM model doesn't support GPS or GPS isn't being collected right now\r\n");
  }
}

bool cellular_is_collecting_gps_sample(void) {
  return _collect_gps;
}

bool cellular_is_collecting_cells_sample(void) {
  return _collect_cells;
}

struct gps_info *cellular_get_gps_info(void) {
  return gps_utilities_get_gps_info();
}

void cellular_set_psm_parameters(
    bool      psm_on, 
    uint8_t   t3324_value,
    uint8_t   t3412_value, 
    uint64_t  current_timestamp) {

  _psm_on = psm_on;

  if (_psm_on == false) {
    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_set_psm_parameters] psm is off!\r\n");
    return;
  }

  if ( ( ((t3324_value && T3324_VALUE_MASK) >> T3324_VALUE_SHIFT) >= T3324_VALUE_MIN ) && 
       ( ((t3324_value && T3324_VALUE_MASK) >> T3324_VALUE_SHIFT) <= T3324_VALUE_MAX ) ) {
    _t3324_value = t3324_value;
  } else {
    _t3324_value = (_t3324_value && T3324_VALUE_MASK);

    debug_print_string(DEBUG_LEVEL_2, 
        (uint8_t *)"[cellular_set_psm_parameters] t3324 value out of range. use minimum mask value\r\n");
  }
  if ( ( ((t3412_value && T3412_VALUE_MASK) >> T3412_VALUE_SHIFT) >= T3412_VALUE_MIN ) && 
       ( ((t3412_value && T3412_VALUE_MASK) >> T3412_VALUE_SHIFT) <= T3412_VALUE_MAX) ) {
    _t3412_value = t3412_value;
  } else {
    _t3412_value = (t3412_value && T3324_VALUE_MASK);

    debug_print_string(DEBUG_LEVEL_2, 
        (uint8_t *)"[cellular_set_psm_parameters] t3412 value out of range. use minimum mask value\r\n");
  }

  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
  debug_print_string(DEBUG_LEVEL_2, 
      (uint8_t *)"[cellular_set_psm_parameters] psm is on! set psm parameters (t3324 value: ");
  char param1[6];
  memset(param1, '\0', 6);
  utils_itoa(_t3324_value, param1, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)param1);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)", t3412 value: ");
  memset(param1, '\0', 6);
  utils_itoa(_t3412_value, param1, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)param1);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)")\r\n");
}

void cellular_set_edrx_parameters(
    bool      edrx_on, 
    uint8_t   edrx_period_value,
    uint8_t   ptw_period_value, 
    uint64_t  current_timestamp) {

  _edrx_on = edrx_on;

  if (_edrx_on == false) {
    debug_print_time(DEBUG_LEVEL_2, current_timestamp);
    debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_set_edrx_parameters] edrx is off!\r\n");
    return;
  }

  if ( (edrx_period_value >= EDRX_PERIOD_VALUE_MIN) && (edrx_period_value <= EDRX_PERIOD_VALUE_MAX) ) {
    _edrx_period_value = edrx_period_value;
  } else {
    _edrx_period_value = EDRX_PERIOD_VALUE_MIN;

    debug_print_string(DEBUG_LEVEL_2, 
        (uint8_t *)"[cellular_set_psm_parameters] eDRX period value out of range\r\n");
  }
  if ( (ptw_period_value >= PTW_PERIOD_VALUE_MIN) && (ptw_period_value <= PTW_PERIOD_VALUE_MAX) ) {
    _ptw_period_value = ptw_period_value;
  } else {
    _ptw_period_value = PTW_PERIOD_VALUE_MIN;

    debug_print_string(DEBUG_LEVEL_2, 
        (uint8_t *)"[cellular_set_psm_parameters] PTW period value out of range\r\n");
  }

  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
  debug_print_string(DEBUG_LEVEL_2, 
      (uint8_t *)"[cellular_set_edrx_parameters] edrx is on! set edrx parameters (edrx period: ");
  char param1[6];
  memset(param1, '\0', 6);
  utils_itoa(_edrx_period_value, param1, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)param1);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)", ptw period: ");
  memset(param1, '\0', 6);
  utils_itoa(_ptw_period_value, param1, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)param1);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)")\r\n");
}

bool cellular_has_psm_active(void) {
  return ( _psm_on && 
           ((_simcom_module_version_used == SIMCOM_7020_VERSION) || (_simcom_module_version_used == SIMCOM_7080_VERSION)) && 
           cellular_utilities_has_psm_mode((_cellular_data.imsi / 1000000000000), ((_cellular_data.imsi % 1000000000000) / 10000000000), _simcom_module_version_used) );
}

bool cellular_has_edrx_active(void) {
  return ( _edrx_on && cellular_has_psm_active() );
}

bool cellular_is_module_with_data_session_remaining_opened(void) {
  if (_current_states.data_session == DATA_SESSION_REMAIN_OPEN) {
    return true;
  } else {
    return false;
  }
}

void cellular_set_psm_configured(bool psm_configured, uint64_t current_timestamp) {
  debug_print_time(DEBUG_LEVEL_2, current_timestamp);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"[cellular_set_psm_configured] reset variable!\r\n");

  _psm_configured = psm_configured;
}

uint8_t cellular_get_t3324_value(void) {
  return _t3324_value;
}

uint8_t cellular_get_t3412_value(void) {
  return _t3412_value;
}

uint8_t cellular_get_edrx_period_value(void) {
  return _edrx_period_value;
}

uint8_t cellular_get_ptw_period_value(void) {
  return _ptw_period_value;
}

bool cellular_is_psm_on(void) {
  return _psm_on;
}

bool cellular_is_edrx_on(void) {
  return _edrx_on;
}

bool cellular_support_cells_service(void) {
  if ( (cellular_get_simcom_version() == SIMCOM_800_VERSION)  || (cellular_get_simcom_version() == SIMCOM_7020_VERSION) || 
       (cellular_get_simcom_version() == SIMCOM_7080_VERSION) || (cellular_get_simcom_version() == SIMCOM_7600_VERSION) || 
       (cellular_get_simcom_version() == SIMCOM_868_VERSION) ) {
    return true;
  } else {
    return false;
  }
}

bool cellular_support_gps_service(void) {
  if ( _gps_on && 
       ( (cellular_get_simcom_version() == SIMCOM_7600_VERSION) || (cellular_get_simcom_version() == SIMCOM_868_VERSION) ) ) {
    return true;
  } else {
    return false;
  }
}

uint64_t cellular_get_gps_sampling_rate(void) {
  if ( (cellular_get_simcom_version() == SIMCOM_7600_VERSION) || (cellular_get_simcom_version() == SIMCOM_868_VERSION) ) {
    return GPS_SENTENCE_SAMPLE_RATE;
  } else {
    return 0;
  }
}

uint16_t cellular_get_http_code(void) {
  return _http_code;
}

int64_t cellular_how_many_time_to_close_data_session(uint64_t current_timestamp) {
  int64_t ret = 0;

  if (_remain_data_session_open == 0) {
    return 0;
  }

  ret = _remain_data_session_open - (current_timestamp - _timestamp_starting_data_session);

  if (ret < 0) {
    ret = -1;
  }

  return ret;
}

void cellular_print_current_internal_state(uint64_t current_timestamp) {
  char aux[6];
  memset(aux, '\0', 6);

  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"##### cellular status begin #####\r\n");

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_loop] power on number: ");
  char xpto[6];
  memset(xpto, '\0', 6);
  utils_itoa(_power_on_number, xpto, 10);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)xpto);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)" times!\r\n");

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] vbat ctrl: ");
  if (_vbat_ctrl_current_state) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"true\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"false\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, 
      (uint8_t *)"[cellular_print_current_internal_state] status pin (true = off; false = true): ");
  if (_cellular_read_status_callback()) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"true\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"false\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, 
      (uint8_t *)"[cellular_print_current_internal_state] is registered? ");
  if (_cellular_data.reg_status == REGISTERED_ROAMING || _cellular_data.reg_status == REGISTERED_HOME_NETWORK) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"true\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"false\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, 
      (uint8_t *)"[cellular_print_current_internal_state] is data session open? ");
  if (_cellular_data.data_session_status == DATA_SESSION_OPENED) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"true\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"false\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] is http init? ");
  if (_cellular_data.http_init) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"true\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"false\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] has data to upload? ");
  if (_cellular_data.has_data_to_upload) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"true\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"false\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] has data to download? ");
  if (_cellular_data.has_data_to_read) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"true\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"false\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] was cells asked? ");
  if (_collect_cells) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"true\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"false\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] was gps asked? ");
  if (_collect_gps) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"true\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"false\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] iccid: ");
  if (strlen(_cellular_data.iccid) == 0) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"-\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)_cellular_data.iccid);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] imsi: ");
  if (strlen(_cellular_data.imsi_text) == 0) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"-\r\n");
  } else {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)_cellular_data.imsi_text);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] rssi: ");
  if (_cellular_data.rssi == 0) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"-\r\n");
  } else {
    memset(aux, '\0', 6);
    utils_itoa(_cellular_data.rssi, aux, 10);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)aux);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] current phase: ");
  if ( ((_current_phase != IDLE_PHASE) && (_current_phase != GPS_PHASE)) && !_cellular_data.warmup_success ) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"warmup\r\n");

    debug_print_time(DEBUG_LEVEL_1, current_timestamp);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] warmup state is: ");
    memset(aux, '\0', 6);
    utils_itoa(_current_states.warmup, aux, 10);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)aux);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");

    debug_print_time(DEBUG_LEVEL_1, current_timestamp);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] last warmup loop on: ");
    debug_print_time(DEBUG_LEVEL_1, _last_warm_up_state_machine_execution_timestamp);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");

    /* if already not ( ((_current_phase != IDLE_PHASE) && (_current_phase != GPS_PHASE)) && !_cellular_data.warmup_success ) */
  } else if ( ( (_current_phase != IDLE_PHASE) && (_current_phase != GPS_PHASE) ) && !_cellular_data.info_success ) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"info\r\n");

    debug_print_time(DEBUG_LEVEL_1, current_timestamp);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] info state is: ");
    memset(aux, '\0', 6);
    utils_itoa(_current_states.info, aux, 10);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)aux);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");

    debug_print_time(DEBUG_LEVEL_1, current_timestamp);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[cellular_print_current_internal_state] last info loop on: ");
    debug_print_time(DEBUG_LEVEL_1, _last_info_state_machine_execution_timestamp);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");
  } else {
    if (_current_phase == IDLE_PHASE) {
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"idle\r\n");

      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, 
          (uint8_t *)"[cellular_print_current_internal_state] last data session loop on: ");
      debug_print_time(DEBUG_LEVEL_1, _last_idle_state_machine_execution_timestamp);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");
    } else if (_current_phase == DATA_SESSION_PHASE) {
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"data session\r\n");

      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, 
          (uint8_t *)"[cellular_print_current_internal_state] data session state is: ");
      memset(aux, '\0', 6);
      utils_itoa(_current_states.data_session, aux, 10);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)aux);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");

      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, 
          (uint8_t *)"[cellular_print_current_internal_state] last data session loop on: ");
      debug_print_time(DEBUG_LEVEL_1, _last_data_session_state_machine_execution_timestamp);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");
    } else if (_current_phase == HTTP_PHASE) {
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"http\r\n");

      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, 
          (uint8_t *)"[cellular_print_current_internal_state] http state is: ");
      memset(aux, '\0', 6);
      utils_itoa(_current_states.http, aux, 10);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)aux);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");

      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, 
          (uint8_t *)"[cellular_print_current_internal_state] last data session loop on: ");
      debug_print_time(DEBUG_LEVEL_1, _last_http_state_machine_execution_timestamp);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");
    } else if (_current_phase == CELLS_PHASE) {
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"cells\r\n");

      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, 
          (uint8_t *)"[cellular_print_current_internal_state] cells state is: ");
      memset(aux, '\0', 6);
      utils_itoa(_current_states.cells, aux, 10);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)aux);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");

      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, 
          (uint8_t *)"[cellular_print_current_internal_state] last data session loop on: ");
      debug_print_time(DEBUG_LEVEL_1, _last_cells_state_machine_execution_timestamp);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");
    } else if (_current_phase == GPS_PHASE) {
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"gps\r\n");

      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, 
          (uint8_t *)"[cellular_print_current_internal_state] gps state is: ");
      memset(aux, '\0', 6);
      utils_itoa(_current_states.gps, aux, 10);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)aux);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");

      debug_print_time(DEBUG_LEVEL_1, current_timestamp);
      debug_print_string(DEBUG_LEVEL_1, 
          (uint8_t *)"[cellular_print_current_internal_state] last data session loop on: ");
      debug_print_time(DEBUG_LEVEL_1, _last_gps_state_machine_execution_timestamp);
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");
    } else {
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"not defined state\r\n");
    }
  }

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, 
      (uint8_t *)"[cellular_print_current_internal_state] current action is: ");
  memset(aux, '\0', 6);
  utils_itoa(_current_action, aux, 10);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)aux);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");

  debug_print_time(DEBUG_LEVEL_1, current_timestamp);
  debug_print_string(DEBUG_LEVEL_1, 
      (uint8_t *)"[cellular_print_current_internal_state] phase last time set on: ");
  debug_print_time(DEBUG_LEVEL_1, _last_time_phase_set_timestamp);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\r\n");

  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"##### cellular status end #####\r\n");
}

bool gps_is_already_collecting(void) {
  return _gps_is_already_collecting;
}

bool cellular_get_current_communication_status(void) {
  if (!_current_registration_status_is_ok || !_current_data_session_status_is_ok || !_current_http_status_is_ok) {
    return false;
  } else {
    return true;
  }
}

/**
 * Gets RSSI data structure.
 * @return The pointer to the RSSI structure.
 */
struct rssi_data *cellular_get_rssi_data(void) {
  return &_rssi_info;
}

bool cellular_psm_is_currently_active(void) {
  return _psm_is_currently_active;
}
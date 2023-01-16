/*
* @file		power_management.c
* @date		August 2021
* @author	Modified by PFaria & JAntunes
*
* @brief        This file has the Power Management utilities, with the purpose of 
*               managing, periodic voltage batery values, for switching between 
*               power saving mode and normal operation mode. It can be switched between states
*               depending in the functions used.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*
* This source code was adapted from the Sensefinity company provided source code:
  /*
   * Copyright (c) 2020 Sensefinity
   * This file is subject to the Sensefinity private code license.
   */

  /**
   * @file  power_management.c
   * @brief This file has the power management application. This application is in
   * charge of managing device power transictions.
   */
/*
*
*/

/*********************************** Includes ***********************************/

/* Interface */
#include "power_management.h"

/* To have access to NRF_POWER register */
#include "sdk_common.h"

/* Charge app to get voltage */
#include "charge.h"

/* Gama libs */
#include "sense_library/protocol-gama/gama_fw/include/gama_generic.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_measurement.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_definitions.h"

/* Measurements buffer */
#include "sense_library/sensoroid/measurements_v2_manager.h"

/* Components */
#include "sense_library/components(telco)/telco.h"
          
/* Utils */
#include "sense_library/utils/debug.h"
#include "sense_library/utils/measurements_vector.h"
#include "sense_library/utils/utils.h"
#include "sense_library/utils/externs.h"

/* Gama queues */
#include "sense_library/sensoroid/gama_queues.h"

/********************************** Definições ***********************************/
/**
 * Stores current power management state.
 */
static power_management_states _current_power_state = POWER_MANAGEMENT_WAITING_FOR_ACTION;

/**
 * Tells if the device is in power saving mode.
 */
static bool _battery_saving_mode_active = true;

#if MICROCONTROLER_2
/**
 * Indicates if device is starting or not.
 */
static bool _is_module_starting = true;

/**
 * Stores current power off reason.
 */
static power_management_power_reasons _power_off_reason = POWER_MANAGEMENT_POWER_NOT_DEFINED_REASON;

/**
 * Stores current power on reason.
 */
static power_management_power_reasons _power_on_reason = POWER_MANAGEMENT_POWER_NOT_DEFINED_REASON;

/**
 * Tells last time power off timestamp was refreshed. It is used when device enters in
 * powering off mode so that device needs to prepare this power off and it will renew timestamp
 * each time an aditional action is perform so that this power off can be extended in time until
 * device had performed all the necessary action to a safe power off.
 */
static uint64_t _power_off_last_action_time = 0;

/**
 * Stores last time debug print was executed.
 */
static uint64_t _power_management_debug_print_timestamp = 0;


/**
 * Stores last time battery info was collected.
 */
static uint64_t _last_battery_info_collection_timestamp = 0;

/**
 * Tells if battery info was collected since power on.
 */
static bool _battery_info_collected_first_time = false;

/**
 * Tells if application already asked for a new battery voltage measurement.
 */
static bool _battery_info_asked = false;

/**
 * Callback function to upload battery saving mode.
 */
static uplink_send_battery_saving_mode_message_autonomously_callback_def _uplink_send_battery_saving_mode_message_autonomously_callback = NULL;

/**
 * Stores the current battery voltage.
 */
static float _current_battery_voltage = 0;

/**
 * Voltage threshold to enter battery saving mode.
 */
static uint16_t _enter_battery_saving_voltage = 0;

/**
 * Voltage threshold to leave battery saving mode when device is warming up.
 */
static uint16_t _leaving_battery_saving_warmup_voltage = 0;

/**
 * Voltage threshold to leaving in battery saving mode.
 */
static uint16_t _leaving_battery_saving_voltage = 0;

/**
 * Tells if device's power status measurement was already added.
 */
static bool _power_status_measurement_added = false;

/**
 * Tells if device's battery saving mode measurement was already added.
 */
static bool _battery_saving_mode_measurement_added = false;

/**
 * Tells if Telco module upload finished (i.e. power off measurement upload was finished)
 * so we can move forward to shutdown device.
 */
static bool _telco_upload_finished = false;

/**
 * Stores the timestamp when the power off message was asked to be added by telco.
 */
static uint64_t _telco_upload_asked_timestamp = 0;


/**
 * Gets the telco upload timeout to upload the power off measurement.
 * @return Timeout to upload power off measurement.
 */
uint64_t get_telco_upload_timeout(void) {
  
  return POWER_MANAGEMENT_ROUTER_TELCO_TIMEOUT;

}
#endif

/**
 * Callback function to shutdown device.
 */
static shutdown_device_callback_def _shutdown_device_callback = NULL;

#if MICROCONTROLER_1
/**
 * Stores the current battery mode.
 */
static float _current_battery_mode = 0;
#endif

/**
 * Runs battery voltage check in order to know if power saving mode
 * should be activated or not.
 */
void battery_saving_mode_check(void) {
  #if MICROCONTROLER_2
  /* Prevention */
  if(_current_battery_voltage == 0) {
    return;
  }

  char battery_voltage_string[10];
  memset(battery_voltage_string, '\0', 10);
  utils_ftoa(battery_voltage_string, _current_battery_voltage, 0.001);

  debug_printf_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[battery_saving_mode_check] Current voltage = %sV\n", battery_voltage_string);

  /* Entering battery saving mode */
  if (_current_battery_voltage < _enter_battery_saving_voltage) {
     if (!_battery_saving_mode_active) {
      _battery_saving_mode_active = true;

      _uplink_send_battery_saving_mode_message_autonomously_callback();

      debug_printf_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_0, 
          (uint8_t*)"[battery_saving_mode_check] Battery saving mode is now on.\n");
    }
  }

  /* Exiting battery saving mode */
  if (_current_battery_voltage > _leaving_battery_saving_voltage) {
    if (_battery_saving_mode_active) {
      _battery_saving_mode_active = false;

      _uplink_send_battery_saving_mode_message_autonomously_callback();

      debug_printf_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_0, 
          (uint8_t*)"[battery_saving_mode_check] Battery saving mode is now off.\n");
    }
  }

  /* We are now powering on so if we are in the middle let the module power on */
  if (_is_module_starting) {
    if ( (_current_battery_voltage > _leaving_battery_saving_warmup_voltage) && 
         (_current_battery_voltage <= _leaving_battery_saving_voltage) ) {
      /* Exiting battery saving mode */
      if (_battery_saving_mode_active) {
        _battery_saving_mode_active = false;

        _uplink_send_battery_saving_mode_message_autonomously_callback();

        debug_printf_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_0, 
            (uint8_t*)"[battery_saving_mode_check] Starting check; Battery saving mode is now off.\n");
      }
    }
    _is_module_starting = false;
  }
  #endif

  #if MICROCONTROLER_1
  
  debug_printf_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  if(_battery_saving_mode_active) {
    debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[battery_saving_mode_check] Current mode = Saving Mode\n");
  } else {
    debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[battery_saving_mode_check] Current mode = Normal Mode\n");
  }

  /* Entering battery saving mode */
  if(_current_battery_mode) {
    if (!_battery_saving_mode_active) {
      _battery_saving_mode_active = true;

      debug_printf_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_0, 
          (uint8_t*)"[battery_saving_mode_check] Battery saving mode is now on.\n");
    }
  
  /* Exiting battery saving mode */
  } else {
    if (_battery_saving_mode_active) {
      _battery_saving_mode_active = false;

      debug_printf_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_0, 
          (uint8_t*)"[battery_saving_mode_check] Battery saving mode is now off.\n");
    }
  }
  #endif
}


/**
 * Runs battery voltage loop. It will collect new battery voltage information
 * and compare this information to determine if battery saving should be
 * activated or deactivated.
 */
void battery_saving_mode_loop(void) {
  #if MICROCONTROLER_2 
  /* Check if it's time to collect new battery voltage information */
  if(!_battery_info_collected_first_time || ((rtc_get_milliseconds() - _last_battery_info_collection_timestamp) >= POWER_MANAGEMENT_BATTERY_INFO_TIMEOUT)) {
    /* Get current battery info */
    charge_data *charge_info = charge_get_info();
    
    /* Check if there is a battery info available */
    if((charge_info->last_read_timestamp != 0) && ((rtc_get_milliseconds() - charge_info->last_read_timestamp) <= POWER_MANAGEMENT_BATTERY_INFO_IS_FRESH_TIMEOUT)) {
      _battery_info_asked = false;

      /* Prevention */
      if(charge_info->voltage != 0) {
        _current_battery_voltage = charge_info->voltage;
      }
      else {
        return;
      }

      if(!_battery_info_collected_first_time) {
        _battery_info_collected_first_time = true;
      }
      _last_battery_info_collection_timestamp = rtc_get_milliseconds();

      battery_saving_mode_check();
    }
    else {
      if(!_battery_info_asked) {
        _battery_info_asked = charge_collect_data(false);
      }
    }
  }
  #endif

  #if MICROCONTROLER_1
    /* Get current battery info */
    charge_data *charge_info = charge_get_info();

    _current_battery_mode = charge_info->battery_mode;
    
    battery_saving_mode_check();  
  #endif
}

#if MICROCONTROLER_2
/**
 * Adds battery saving mode. Used when device is powering on.
 */
bool battery_saving_mode_add_measurement(void) {
  uint8_t value = 0;
  if(_battery_saving_mode_active) {
    value = 1;
  }

  gama_measure_format_v2_fields_t measurement_fields;
  measurement_fields.measure_type = GAMA_MEASURE_POWER_SAVING;
  measurement_fields.sensor = MCU_SENSOR_SEQUENCE;
  measurement_fields.config_byte = MEASURE_VALUE_UINT8_TYPE;
  measurement_fields.val = &value;

  if(measurements_v2_manager_add_measurement(&measurement_fields)) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_1,
        (uint8_t*)"[battery_saving_mode_add_measurement] Measurement added; Details: value = %d\n", value);
    return true;
  } else {
    return false;
  }
}
#endif

/********************************** Public *************************************/

/**
 * Power management init and global variables initialization.
 * @param[in] initial_power_state The initial power management state that device will assume  !ONLY-uC1!
 * when main begins.
 * @param[in] shutdown_device_callback Pointer to shutdown device function. !ONLY-uC1!
 * @param[in] uplink_send_battery_saving_mode_message_autonomously_callback Pointer to trigger upload battery saving information function.
 * @param[in] enter_battery_saving_voltage Value bellow that device enters in battery saving mode.
 * @param[in] leaving_battery_saving_warmup_voltage Value above that device exits battery saving mode when warming up (i.e. powering on).
 * @param[in] leaving_battery_saving_voltage Value above that device exits battery saving mode.
 */
void power_management_init(
    power_management_states                       initial_power_state,
    shutdown_device_callback_def                  shutdown_device_callback,
    uplink_send_battery_saving_mode_message_autonomously_callback_def uplink_send_battery_saving_mode_message_autonomously_callback,
    uint16_t                                      enter_battery_saving_voltage,
    uint16_t                                      leaving_battery_saving_warmup_voltage,
    uint16_t                                      leaving_battery_saving_voltage) {
  
    
  #if MICROCONTROLER_2
  _uplink_send_battery_saving_mode_message_autonomously_callback = uplink_send_battery_saving_mode_message_autonomously_callback;  
  _enter_battery_saving_voltage = enter_battery_saving_voltage;
  _leaving_battery_saving_voltage = leaving_battery_saving_voltage;
  _leaving_battery_saving_warmup_voltage = leaving_battery_saving_warmup_voltage;
  _last_battery_info_collection_timestamp = 0;
  _battery_info_asked = false;
  _battery_info_collected_first_time = false;
  _power_off_reason = POWER_MANAGEMENT_POWER_NOT_DEFINED_REASON;
  _power_on_reason = POWER_MANAGEMENT_POWER_NOT_DEFINED_REASON;
  _power_off_last_action_time = 0;
  _power_management_debug_print_timestamp = rtc_get_milliseconds();
  _is_module_starting = true;
  _power_status_measurement_added = false;
  _battery_saving_mode_measurement_added = false;
  _telco_upload_finished = false;
  _telco_upload_asked_timestamp = 0;
    
  debug_printf_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_printf_string(DEBUG_LEVEL_1, 
      (uint8_t*)"[power_management_init] Enter battery saving voltage = %dmV; Leave battery saving warmup voltage = %dmV; Leave battery saving voltage = %dmV\n", 
      _enter_battery_saving_voltage, _leaving_battery_saving_warmup_voltage, _leaving_battery_saving_voltage);  
  #endif
    
  _shutdown_device_callback = shutdown_device_callback; 
  _current_power_state = initial_power_state;  
  _battery_saving_mode_active = true;
    
  #if MICROCONTROLER_1
  _current_battery_mode = true;
  debug_printf_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_printf_string(DEBUG_LEVEL_1, 
      (uint8_t*)"[power_management_init] \n");

  #endif 
  
}


/**
 * Power management loop in charge of execute/manage internal state machine.
 */
void power_management_loop(void) {
  
  #if MICROCONTROLER_2
  if((rtc_get_milliseconds() - _power_management_debug_print_timestamp) >= POWER_MANAGEMENT_DEBUG_PRINT_TIMEOUT) {
    _power_management_debug_print_timestamp = rtc_get_milliseconds();

    debug_printf_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_1, 
        (uint8_t*)"[power_management_loop] Current power management state: %d\n", _current_power_state);
    
    char battery_voltage_string[10];
    memset(battery_voltage_string, '\0', 10);
    utils_ftoa(battery_voltage_string, _current_battery_voltage, 0.001);

    if (_battery_saving_mode_active) {
      debug_printf_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_printf_string(DEBUG_LEVEL_1, 
          (uint8_t*)"[power_management_loop] Battery saving mode is on; Current voltage = %smV\n", battery_voltage_string);
    }
    else {
      debug_printf_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_printf_string(DEBUG_LEVEL_1, 
          (uint8_t*)"[power_management_loop] Battery saving mode is off; Current voltage = %smV\n", battery_voltage_string);
    }
  }

  switch (_current_power_state) {
    case POWER_MANAGEMENT_WAITING_FOR_ACTION: {
      break;
    }

    case POWER_MANAGEMENT_POWERED_OFF: {
      /* See you in another power on brother! */
      _shutdown_device_callback();

      break;
    }

    case POWER_MANAGEMENT_POWER_ON: {
      _current_power_state = POWER_MANAGEMENT_POWERED_ON;

      if(_power_on_reason == POWER_MANAGEMENT_POWER_BUTTON_REASON) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_1,
            (uint8_t*)"[power_management_loop] Power on reason was button so add power on measurement.\n");
        
        /* Adds power status measurement */
        if(power_management_add_power_measurement(true)) {
          _power_status_measurement_added = true;
        }
      }

      /* Adds battery saving mode measurement */
      if(battery_saving_mode_add_measurement()) {
        _battery_saving_mode_measurement_added = true;
      }

      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_1,(uint8_t*)"[power_management_loop] Move to powered on state.\n");

      break;
    }

    case POWER_MANAGEMENT_POWERED_ON: {
      /* Precaution */
      if((_power_on_reason == POWER_MANAGEMENT_POWER_BUTTON_REASON) && (!_power_status_measurement_added)) {
        if(power_management_add_power_measurement(true)) {
          _power_status_measurement_added = true;
        }
      }

      /* Precaution */
      if(!_battery_saving_mode_measurement_added) {
        if(battery_saving_mode_add_measurement()) {
          _battery_saving_mode_measurement_added = true;
        }
      }

      /* Only applied to recheargable batteries */
      if(charge_get_rechargeable_type()) {
        battery_saving_mode_loop();
      }
      else {
        if(_battery_saving_mode_active) {
          _battery_saving_mode_active = false;
        }
      }
      
      break;
    }

    case POWER_MANAGEMENT_PREPARE_POWER_OFF: {
      if(POWER_MANAGEMENT_TELCO_UPLOAD_POWER_OFF && (_power_off_reason == POWER_MANAGEMENT_POWER_BUTTON_REASON)) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_1,
            (uint8_t*)"[power_management_loop] Power off reason was button so add power off measurement.\n");

        /* Stop any critical application in case they are running */
        //apps_manager_stop();

        /* Tell telco module to upload power measurement autonomously */
        _telco_upload_finished = false;
        _telco_upload_asked_timestamp = rtc_get_milliseconds();
        telco_send_power_off_message_autonomously();
      }
      else {
        _telco_upload_finished = true;
      }

      /* Start timeout for no action */
      power_management_update_power_off_last_action_time();

      _current_power_state = POWER_MANAGEMENT_POWER_OFF;

      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_1,(uint8_t*)"[power_management_loop] Move to power off state.\n");

      break;
    }

    case POWER_MANAGEMENT_POWER_OFF: {
      /* Timeout for no action detected so let's shutdown device */
      if (rtc_get_milliseconds() - _power_off_last_action_time >= POWER_MANAGEMENT_POWER_OFF_TIMEOUT) {
        if(!_telco_upload_finished && ((rtc_get_milliseconds() - _telco_upload_asked_timestamp) >= get_telco_upload_timeout())) {
          debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
          debug_print_string(DEBUG_LEVEL_1,(uint8_t*)"[power_management_loop] Telco upload timeout reached, so let's move ahead with device power off.\n");

          _telco_upload_finished = true;
        }

        bool power_off = false;
        
        if(_power_off_reason != POWER_MANAGEMENT_POWER_BUTTON_REASON) {
          power_off = true;
        }        
        else {
          /* Check if there are no packets in the queue so that power off
           * measurement was transmitted or check if timeout has expired */
          /**
           * @todo Check with a more efficient method.
           */
          if((gama_queues_how_many_messages() == 0) || _telco_upload_finished) {
            power_off = true;
          }
        }        

        if(power_off) {
          _current_power_state = POWER_MANAGEMENT_POWERED_OFF;

          debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
          debug_print_string(DEBUG_LEVEL_1,(uint8_t*)"[power_management_loop] Move to powered off state.\n");
        }
      }

      break;
    }

    default: {
      break;
    }
  }
  #endif

  #if MICROCONTROLER_1  
  switch (_current_power_state) {
    case POWER_MANAGEMENT_WAITING_FOR_ACTION: {
      break;
    }

    case POWER_MANAGEMENT_POWERED_OFF: {
      /* See you in another power on brother! */
      _shutdown_device_callback();

      break;
    }

    case POWER_MANAGEMENT_POWER_ON: {
      _current_power_state = POWER_MANAGEMENT_POWERED_ON;
      
      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_1,(uint8_t*)"[power_management_loop] Move to powered on state.\n");

      break;
    }

    case POWER_MANAGEMENT_POWERED_ON: {
            
      battery_saving_mode_loop();    
            
      break;
    }

    case POWER_MANAGEMENT_PREPARE_POWER_OFF: {
      
      debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_1,(uint8_t*)"[power_management_loop] Move to power off state.\n");

      break;
    }

    case POWER_MANAGEMENT_POWER_OFF: {
      
      bool power_off = false;
            
      if(power_off) {
        _current_power_state = POWER_MANAGEMENT_POWERED_OFF;

        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_1,(uint8_t*)"[power_management_loop] Move to powered off state.\n");
      }
    }     

    default: {
      break;
    }
  }
  #endif
}


/**
 * Sets current power management state.
 * @param[in] new_power_state New power management state.
 */
void power_management_set_state(power_management_states new_power_state) {
  _current_power_state = new_power_state;
}


/**
 * Gets current power management state.
 * @return new_power_state New power management state.
 */
power_management_states power_management_get_state(void) {
  return _current_power_state;
}

/**
 * Tells if device is working in battery saving mode.
 * @return True if device is in battery saving mode, otherwise returns false.
 */
bool power_management_is_battery_saving_mode_active(void) {
  return _battery_saving_mode_active;
}

#if MICROCONTROLER_2
/**
 * Sets power off reason flag.
 * @param[in] power_off_reason New power off reason.
 */
void power_management_set_power_off_reason(power_management_power_reasons power_off_reason) {
  _power_off_reason = power_off_reason;
}


/**
 * Sets power on reason flag.
 * @param[in] power_on_reason New power on reason.
 */
void power_management_set_power_on_reason(power_management_power_reasons power_on_reason) {
  _power_on_reason = power_on_reason;
}


/**
 * Renew timestamp to move forward in device power off.
 */
void power_management_update_power_off_last_action_time(void) {
  _power_off_last_action_time = rtc_get_milliseconds();
}


/**
 * Sets telco upload finished flag.
 */
void power_management_set_telco_upload_finished(void) {
  _telco_upload_finished = true;
}


/**
 * Adds power on/off measurement.
 * @param[in] power_status True if device is on, false if device is off.
 * @return True if message was added, false otherwise.
 */
bool power_management_add_power_measurement(bool power_status) {
  uint8_t value = 0;
  if(power_status) {
    value = 1;
  }

  gama_measure_format_v2_fields_t measurement_fields;
  measurement_fields.measure_type = GAMA_MEASURE_POWER_ON;
  measurement_fields.sensor = MCU_SENSOR_SEQUENCE;
  measurement_fields.config_byte = MEASURE_VALUE_UINT8_TYPE;
  measurement_fields.val = &value;

  if(measurements_v2_manager_add_measurement(&measurement_fields)) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_1,(uint8_t*)"[power_status_add_measurement] Measurement added; details: value = %d\n", value);
    return true;
  } else {
    return false;
  }
}
#endif



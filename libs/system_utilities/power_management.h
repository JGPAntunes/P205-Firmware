/*
* @file		power_management.h
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
* This header code was adapted from the Sensefinity company provided header code:
  /*
   * Copyright (c) 2020 Sensefinity
   * This file is subject to the Sensefinity private code license.
   */

  /**
   * @file  power_management.h
   * @brief This header file has the power management application.  
   */
/*
*
*/

#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

/*********************************** Includes ***********************************/

/* C standard library */
#include <stdint.h>
#include <stdbool.h>

/* Config */
#include "config.h"

/********************************** Definições ***********************************/
/**
 * Power management possible internal states.
 */
typedef enum {
  POWER_MANAGEMENT_WAITING_FOR_ACTION      = (0),
  POWER_MANAGEMENT_POWERED_OFF             = (1),
  POWER_MANAGEMENT_POWER_ON                = (2),
  POWER_MANAGEMENT_POWERED_ON              = (3),
  POWER_MANAGEMENT_PREPARE_POWER_OFF       = (4),
  POWER_MANAGEMENT_POWER_OFF               = (5)
} power_management_states;

#if MICROCONTROLER_2
/**
 * Power off reasons.
 */
typedef enum {
  POWER_MANAGEMENT_POWER_NOT_DEFINED_REASON    = (0),
  POWER_MANAGEMENT_POWER_BUTTON_REASON         = (1),
  POWER_MANAGEMENT_POWER_PERIODIC_RESET_REASON = (2)
} power_management_power_reasons;

#define POWER_MANAGEMENT_POWER_OFF_TIMEOUT             (5000)   ///< Power off preparation timeout after that device will execute power off.
#define POWER_MANAGEMENT_DEBUG_PRINT_TIMEOUT           (300000) ///< Timeout that a debug sentence is printed out.

#define POWER_MANAGEMENT_TELCO_UPLOAD_POWER_OFF        (0)      ///< Upload power off status measurement when powering off device.
#define POWER_MANAGEMENT_ROUTER_TELCO_TIMEOUT          (60000)  ///< Gateway telco upload timeout.
#endif

#define POWER_MANAGEMENT_BATTERY_INFO_TIMEOUT          (300000) ///< Timeout to consider a new measurement relatively fresh (i.e. new one).
#define POWER_MANAGEMENT_BATTERY_INFO_IS_FRESH_TIMEOUT (60000)  ///< Timeout to consider a new measurement relatively fresh (i.e. new one).

/* Callback functions needed */
typedef void (*shutdown_device_callback_def)(void);                                        ///< Shutdown function callback definition.
typedef void (*uplink_send_battery_saving_mode_message_autonomously_callback_def)(void);   ///< Send battery saving mode callback definition.
#if MICROCONTROLER_2
typedef bool (*configuration_flash_page_is_busy_callback_def)(void);                       ///< Check configuration page busy callback function definition.
#endif
/********************************** Prototypes *********************************/

void power_management_init(
    power_management_states                       initial_power_state,
    shutdown_device_callback_def                  shutdown_device_callback,
    uplink_send_battery_saving_mode_message_autonomously_callback_def uplink_send_battery_saving_mode_message_autonomously_callback,
    uint16_t                                      enter_battery_saving_voltage,
    uint16_t                                      leaving_battery_saving_warmup_voltage,
    uint16_t                                      leaving_battery_saving_voltage);

void power_management_loop(void);

void power_management_set_state(power_management_states new_power_state);

power_management_states power_management_get_state(void);

bool power_management_is_battery_saving_mode_active(void);

#if MICROCONTROLER_2
void power_management_set_power_off_reason(power_management_power_reasons power_off_reason);

void power_management_set_power_on_reason(power_management_power_reasons power_on_reason);

void power_management_update_power_off_last_action_time(void);

void power_management_set_telco_upload_finished(void);

bool power_management_add_power_measurement(bool power_status);
#endif

#endif /* POWER_MANAGEMENT_H */
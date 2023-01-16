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

#ifndef APP_ECG_H_
#define APP_ECG_H_

/********************************** Includes ***********************************/
/* Drivers */
#include "ads129x.h"

/* Config */
#include "config.h"

/********************************** Definições ***********************************/
#define APP_ECG_RETRIES_CMD     3

/* Estados da aplicação loop ECG */
typedef enum {
  APP_ECG_POWERED_OFF,
  APP_ECG_POWER_ON,
  APP_ECG_START_SAMPLE,
  APP_ECG_SAMPLING,
  APP_ECG_UPLOAD_DATA,
  APP_ECG_SAVE_MODE,
  APP_ECG_NOP
} app_ecg_states;

/* Callback functions needed */
typedef bool (*power_save_callback_def)(void);   /* Powersave function callback definition. */

/********************************** Funções ***********************************/
void app_ecg_init(power_save_callback_def power_save_callback);
void app_ecg_loop(void);
bool app_ecg_config(bool accuracy, bool lead_off, uint8_t ecg_gain, uint8_t resp_gain, uint8_t test_mode);
bool app_ecg_start_sample(void);
bool app_ecg_stop_sample(void);
void app_ecg_power_off(void);
bool app_ecg_is_busy(void);


#endif /* APP_ECG_H_ */





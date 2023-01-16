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

#ifndef CHARGE_H_
#define CHARGE_H_

/********************************** Includes ***********************************/

/* SDK */
#include "nrf_gpio.h"

/* Config */
#include "config.h"

#if MICROCONTROLER_2
#include "nrf_gpio.h"
#include "saadc.h"
#endif

#if MICROCONTROLER_1
//#include "app_extram.h" /* ToDo */
#endif



/********************************** Definições ***********************************/
/* PIN */
#if MICROCONTROLER_2
#define CHARGE_WIRE_PIN                 NRF_GPIO_PIN_MAP(0, 25)             /* Wire Status Pin */
#define CHARGE_WIRELESS_PIN             NRF_GPIO_PIN_MAP(0, 26)             /* Wireless Status Pin */

#define CHARGE_BLINK_UP_TIME_TH         1200                                /* Wave Charge Wire Blink upper threshold */
#define CHARGE_BLINK_BOTTOM_TIME_TH     900                                 /* Wave Charge Wire Blink bottom threshold */
#define CHARGE_BLINK_NUMBER_TH          3                                   /* Number of waves Charge Wire Blink count threshold security */
#define CHARGE_BLINK_TIME_TH            3000                                /* Threshold for checking if blink is still occuring */

/* Estados de carregamento por fio */
typedef enum {
  CHARGE_WIRE_OFF   = (0x00),
  CHARGE_WIRE_ON    = (0x01),
  CHARGE_WIRE_FAULT = (0xFF)     
} charge_wire_states;
#endif

#define CHARGE_VOLTAGE_DIVIDER_VALUE    1.3090909

/********************************** Code bellow adapted from Energy.c ************************************/
/**
* Charge data sampling internal states.
*/
typedef enum {
  CHARGE_POWERED_OFF,
  CHARGE_POWER_ON,
  CHARGE_IDLE,
  CHARGE_WAITING_DATA,
  CHARGE_UPLOAD_DATA,
  CHARGE_NOP
} charge_sampling_states;

#define CHARGE_DATA_SAMPLING_TIMEOUT                 (2000)      ///< Timeout to get energy data from sensor.

#if MICROCONTROLER_2
/* Battery parameters for lithium (type 1) */
#define LITHIUM_VOLTAGE_MAX_TYPE_1                   (4.17)      ///< Voltage maximum in V.
#define LITHIUM_CHARGE_TERMINATION_TYPE_1            (30)        ///< Charge termination in mA.
#define LITHIUM_EMPTY_VOLTAGE_TYPE_1                 (3.35)      ///< Empty voltage in V.
#define LITHIUM_RECOVERY_VOLTAGE_TYPE_1              (3.6)       ///< Recovery voltage in V.
#define LITHIUM_CAPACITY_TYPE_1                      (1200)       ///< Capacity in mAh.
#define LITHIUM_ENTER_BATTERY_SAVING_TYPE_1          (3.400)      ///< Voltage in mV.
#define LITHIUM_LEAVING_BATTERY_SAVING_WARMUP_TYPE_1 (3.500)      ///< Voltage in mV.
#define LITHIUM_LEAVING_BATTERY_SAVING_TYPE_1        (3.700)      ///< Voltage in mV.
#define LITHIUM_CHARGED_BATTERY_TYPE_1               (4.000)      ///< Voltage in mV.

typedef enum {
  CHARGE_MODE_OFF,
  CHARGE_MODE_WIRE_FAULT,
  CHARGE_MODE_WIRELESS_ON,
  CHARGE_MODE_WIRE_ON  
} charge_modes;

/**
 * Battery's type information structure.
 */
struct battery_information {
  uint16_t enter_battery_saving_voltage;                         ///< Battery's voltage.
  uint16_t leaving_battery_saving_warmup_voltage;                ///< Battery's voltage.
  uint16_t leaving_battery_saving_voltage;                       ///< Battery's voltage.
};
#endif 

/**
 * Charge data structure uC2.
 */
typedef struct {
  #if MICROCONTROLER_2
  uint8_t battery_percentage;                         ///< Battery's percentage.  
  float voltage;                                      ///< Battery's voltage in mV.
  #endif
  #if MICROCONTROLER_1
  bool battery_mode;                                  ///< Battery's mode.  
  #endif
  uint64_t last_read_timestamp;                       ///< Refers to the last data read timestamp.
} charge_data;

/* Callback functions needed */
typedef bool (*app_extram_get_init_status_callback_def)(void);
typedef charge_data* (*app_extram_get_battery_data_callback_def)(void);

/********************************** Funções ***********************************/
bool charge_init(
  uint64_t                                        sampling_rate,
  app_extram_get_init_status_callback_def         app_extram_get_init_status_callback,
  app_extram_get_battery_data_callback_def        app_extram_get_battery_data_callback);

/********************************** Code bellow adapted from Energy.c ************************************/
void charge_loop(void);
bool charge_collect_data(bool upload_info);
charge_data* charge_get_info(void);
bool charge_is_busy(void);

#if MICROCONTROLER_2
bool charge_collect_data(bool upload_info);
bool charge_get_rechargeable_type(void);
uint8_t charge_get_external_power_status(void);
struct battery_information charge_get_battery_type_information(void);
uint8_t charge_get_percentage(void);
#endif 

#endif /* CHARGE_H_ */





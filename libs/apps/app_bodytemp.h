/*
* @file		app_bodytemp.h
* @date		June 2020
* @author	PFaria & JAntunes
*
* @brief	This is the header for body temperature application utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef APP_BODY_TEMPERATURE_H_
#define APP_BODY_TEMPERATURE_H_

/********************************** Includes ***********************************/
/* Config */
#include "config.h"

#if MICROCONTROLER_1
/* SDK */
#include "ads1114.h"

/* standard library */
#include <stdint.h>
#include <stdbool.h>

/********************************** Definitions ***********************************/
/* App states */
typedef enum {
  BODYTEMP_TWI_INITIALIZING,
  BODYTEMP_DRIVER_INITIALIZING,
  BODYTEMP_IDLE,
  BODYTEMP_WAITING_DATA,
  BODYTEMP_UPLOAD_DATA,
  BODYTEMP_NOP
} app_states;

#define APP_BODYTEMP_PWR_EN                                 (NRF_GPIO_PIN_MAP(0, 25))      ///< Telco UART Rx pin number.

#define APP_BODYTEMP_DATA_PACKET_MEASUREMENT_SIZE           14             /* Size in bytes of each measurement */

/* Analog frontend constants */
#define APP_BODYTEMP_VREF                                   3             /* Voltage reference on the wheatstone bridge */
#define APP_BODYTEMP_R_TOP_WHEATSTONE                       3160//3200          /* Lower wheatstone bridge resistor value */ 
#define APP_BODYTEMP_R_BOTTOM_WHEATSTONE                    2700          /* Upper wheatstone bridge resistor value */
#define APP_BODYTEMP_VA                                     (float)APP_BODYTEMP_VREF * APP_BODYTEMP_R_BOTTOM_WHEATSTONE / \
                                                            (APP_BODYTEMP_R_BOTTOM_WHEATSTONE + APP_BODYTEMP_R_TOP_WHEATSTONE) /* Value of VA voltage */
#define APP_BODYTEMP_LOOKUP_TABLE_MIN_TEMPERATURE           340           /* Minimum temperature */
#define APP_BODYTEMP_LOOKUP_TABLE_MAX_TEMPERATURE           430           /* Maximum temperature */
#define APP_BODYTEMP_TEMPERATURE_FACTOR                     10            /* Multiplying factor for temperature value */

/* Utility macros*/
#define RESISTANCE_CONVERTER(voltage) (APP_BODYTEMP_R_BOTTOM_WHEATSTONE * (((float)APP_BODYTEMP_VREF / (float)((float)voltage + APP_BODYTEMP_VA)) - 1))  /* Converter tensão para resistencia */


/********************************** Functions ***********************************/
void app_body_temperature_loop(void);
bool app_body_temperature_is_busy(void);
void app_body_temperature_init(uint64_t sampling_rate);

#endif 
#endif /* APP_BODYTEMP_H_ */



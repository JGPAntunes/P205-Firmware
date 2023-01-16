/*
* @file		config.h
* @date		June 2020
* @author	PFaria & JAntunes
*
* @brief	This is the header for configs destinated to the firmware in general.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef CONFIG_H
#define CONFIG_H

/********************************** Includes ***********************************/
/* C standard library */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/* ******************************* General *********************************** */
/* ***************** */
/*   Sincronização   */
/* ***************** */
/* uC1 macro for code selection */
#define MICROCONTROLER_1    0

/* uC2 macro for code selection */
#define MICROCONTROLER_2    1

/* ***************** */
/*       APPS        */
/* ***************** */
/* App Body Temp - ads1114 */
#define APP_BODY_TEMP_SAMPLING_RATE   6000    
#define APP_BODY_TEMP_UPPER_TH        380
#define APP_BODY_TEMP_LOWER_TH        350

/* App Charge */
#define APP_CHARGE_SAMPLING_RATE      15000

/* App uSD */
#define APP_USD_UPLOAD_TH             20  
#define APP_USD_PWR_EN                NRF_GPIO_PIN_MAP(0, 4)
#define APP_USD_MOSI_PIN              6
#define APP_USD_MISO_PIN              8
#define APP_USD_SCK_PIN               7
#define APP_USD_CS_PIN                5

/* ***************** */
/*       Serial      */
/* ***************** */
/* Evian */
#define DEVICE_CLOUD_ID               191909076200000068          /* ID used to communicate to the cloud */

/* ***************** */
/*      Measures     */
/* ***************** */
/* Sensors' sequence numbers list */
#define GPS_SENSOR_SEQUENCE                       1   ///< GPS sensor sequence number.
#define LABEL_SENSOR_SEQUENCE                     2   ///< Label sensor sequence number.
#define EXTERNAL_POWER_SENSOR_SEQUENCE            3   ///< External power sensor sequence number.
#define ENERGY_SENSOR_SEQUENCE                    4   ///< Energy sensor sequence number.
#define MCU_SENSOR_SEQUENCE                       5   ///< MCU sequence number. Used for measurements added by the MCU without reading from external sensors or internal registers.
#define BAROMETER_SENSOR_SEQUENCE                 6   ///< Barometer sensor sequence number.
#define HUMIDITY_SENSOR_SEQUENCE                  7   ///< Humidity sensor sequence number.
#define ACCELEROMETER_SENSOR_SEQUENCE             8   ///< Accelerometer sensor (generic) sequence number.
#define ACCELEROMETER_X_AXIS_SENSOR_SEQUENCE      9   ///< Accelerometer X axis sensor sequence number.
#define ACCELEROMETER_Y_AXIS_SENSOR_SEQUENCE      10  ///< Accelerometer Y axis sensor sequence number.
#define ACCELEROMETER_Z_AXIS_SENSOR_SEQUENCE      11  ///< Accelerometer Z axis sensor sequence number.
#define CELLULAR_MODULE_SENSOR_SEQUENCE           12  ///< Cellular module sensor sensor sequence number.
#define LUMINOSITY_SENSOR_SEQUENCE                13  ///< Lumisosity sensor sequence number.
#define EXTERNAL_FLASH_SENSOR_SEQUENCE            14  ///< External Flash sensor sequence number.
#define EXTERNAL_RTC_SENSOR_SEQUENCE              15  ///< External RTC sensor sequence number.


/* Strings macros for measurements names */                                    
#define MEAS_TIMESTAMP  "Timestamp"  
#define MEAS_TEMP       "Temperature"   
#define MEAS_RESP       "Respiration"  
#define MEAS_ECG_I      "Lead I"
#define MEAS_ECG_II     "Lead II"
#define MEAS_ECG_III    "Lead III"
#define MEAS_ECG_AVR    "Lead aVR"
#define MEAS_ECG_AVL    "Lead aVL"
#define MEAS_ECG_AVF    "Lead aVF"
#define MEAS_ECG_V1     "Lead V1"
#define MEAS_ECG_V2     "Lead V2"
#define MEAS_ECG_V3     "Lead V3"
#define MEAS_ECG_V4     "Lead V4"
#define MEAS_ECG_V5     "Lead V5"
#define MEAS_ECG_V6     "Lead V6"

/* Array with content measurements transfers */
#define MEASURES_CONTENT    \
  {                         \
    MEAS_TIMESTAMP,         \
    MEAS_TEMP,              \
    MEAS_TIMESTAMP,         \
    MEAS_RESP,              \
    MEAS_ECG_I,             \
    MEAS_ECG_II,            \
    MEAS_ECG_III,           \
    MEAS_ECG_AVR,           \
    MEAS_ECG_AVL,           \
    MEAS_ECG_AVF,           \
    MEAS_ECG_V1,            \
    MEAS_ECG_V2,            \
    MEAS_ECG_V3,            \
    MEAS_ECG_V4,            \
    MEAS_ECG_V5,            \
    MEAS_ECG_V6             \
  }
#define DEVICE_MEASURES_NUMBER         16

/* Size macros in bytes to use in MEASURES_CONTENT_SIZE */
typedef enum {                                     
  MEAS_1BYTE  = (1),    /* uint8_t */
  MEAS_2BYTE  = (2),    /* uint16_t */
  MEAS_4BYTE  = (4),    /* uint32_t */
  MEAS_8BYTE  = (8),    /* uint64_t */  
} measures_size;

/* Array with size of content measurements transfers */
#define MEASURES_CONTENT_SIZE  \
  {                           \
    MEAS_8BYTE,               \
    MEAS_2BYTE,               \
    MEAS_8BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE,               \
    MEAS_2BYTE                \
  }


/* Informação da configuração do equipamento do paciente a monitorizar */
typedef struct {
  uint8_t   mode;
  bool      ecg;
  bool      ecg_lead_off;
  uint8_t   ecg_gain;
  bool      ecg_accuracy;
  bool      pace;
  uint8_t   resp_gain;
  bool      temp;  
  uint16_t  temp_low;
  uint16_t  temp_high;
  uint16_t  heart_rate_low;
  uint16_t  heart_rate_high;
  uint16_t  resp_rate_low;
  uint16_t  resp_rate_high;  
} device_config;
#define DEVICE_CONFIGS_NUMBER         14

/* Modes to use in field mode in device_config struct */
typedef enum {                                     
  MODE_DATA_LOG,
  MODE_ALERTS,
  MODE_ALL,
} device_cfg_modes;

/* Informação do paciente a monitorizar */
typedef struct {
  uint32_t      id;
  uint8_t       *name;
  uint8_t       *gender;
  uint8_t       age;   
} patient_info;

/* ******************************** Testes *********************************** */
/* ***************** */
/*      Testes       */
/* ***************** */
/* Driver ADS1114 */
#define ADS1114_TEST        0


#endif /* CONFIG_H */
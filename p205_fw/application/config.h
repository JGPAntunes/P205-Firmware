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

/* SDK */
#include "nrf_drv_gpiote.h"

/********************************** Tests ***********************************

/* ***************** */
/*       Tests       */
/* ***************** */
/* Driver ADS1114 */
#define ADS1114_TEST        0

/* ***************** */
/*  Synchronization  */
/* ***************** */
/* uC1 macro for code selection */
#define MICROCONTROLER_1    1

/* uC2 macro for code selection */
#define MICROCONTROLER_2    0

/* ***************** */
/*       Pins        */
/* ***************** */
/* Telco */
#define TELCO_UART_RX_PIN_NUMBER          NRF_GPIO_PIN_MAP(0, 14)     /* Telco UART Rx pin number sense-8 */
#define TELCO_UART_TX_PIN_NUMBER          NRF_GPIO_PIN_MAP(0, 13)     /*  Telco UART Tx pin number sense-11 */
#define TELCO_UART_CTS_PIN_NUMBER         NRF_GPIO_PIN_MAP(0, 11)     /*  Telco UART CTS pin number sense-14 */
#define TELCO_UART_RTS_PIN_NUMBER         NRF_GPIO_PIN_MAP(0, 12)     /*  Telco UART RTS pin number sense-15 */
#define SIMCOM_PWR_KEY                    NRF_GPIO_PIN_MAP(0, 15)     /*  PWR KEY pin definition sense-16 */
#define SIMCOM_LVL_SHFT_OE                NRF_GPIO_PIN_MAP(0, 17)     /*  LVL SHFT OE pin definition sense-18 */
#define SIMCOM_STATUS                     NRF_GPIO_PIN_MAP(0, 3)      /*  STATUS pin definition sense-17 */
/* Buttons */ 
#define BTN_LEFT_PIN                      NRF_GPIO_PIN_MAP(0, 18)                     
#define BTN_RIGHT_PIN                     NRF_GPIO_PIN_MAP(0, 23)                     
#define BTN_BOTTOM_PIN                    NRF_GPIO_PIN_MAP(0, 20)                       
#define BTN_UPPER_PIN                     NRF_GPIO_PIN_MAP(0, 22)                          
#define BTN_CENTER_PIN                    NRF_GPIO_PIN_MAP(0, 19)
/* Charge */
#define CHARGE_WIRE_PIN                   NRF_GPIO_PIN_MAP(0, 25)     /* Wire Status Pin */
#define CHARGE_WIRELESS_PIN               NRF_GPIO_PIN_MAP(0, 26)     /* Wireless Status Pin */
/* SAADC */
#define SAADC_PIN_P                       NRF_SAADC_INPUT_AIN4        /* P0.28 */
/*  SSD1309 */
#define SSD1309_RES                       NRF_GPIO_PIN_MAP(0, 10)     /* Output Pin to reset LCD */
#define SSD1309_DC                        NRF_GPIO_PIN_MAP(0, 2)     /* Output Pin to choose Data or Command LCD */
#define SSD1309_CS                        NRF_GPIO_PIN_MAP(0, 9)     /* Output Pin of Chip Select of LCD */
/* ADS1114 */
#define ADS1114_DRY_PIN                   NRF_GPIO_PIN_MAP(0, 24)      /* Data ready Pin */
/* ADS129x */
#define ADS129x_PACE_RST_PIN              NRF_GPIO_PIN_MAP(0, 11)     /* Output Pin to reset latch */
#define ADS129X_DRDY                      NRF_GPIO_PIN_MAP(0, 12)     /* Input Pin para obter sincronização */
#define ADS129x_PWDN                      NRF_GPIO_PIN_MAP(0, 18)     /* Output Pin to power on/off */
/* ADS1298 */
#define ADS129x_8_CS_PIN                  NRF_GPIO_PIN_MAP(0, 16)     /* Output Pin of CS */              
/* ADS1296R */
#define ADS129x_6R_CS_PIN                 NRF_GPIO_PIN_MAP(0, 17)     /* Output Pin of CS */ 
/* SPI */
//config1
#define SPI_MNGR_CONFIG1_SCK_PIN          NRF_GPIO_PIN_MAP(0, 14)
#define SPI_MNGR_CONFIG1_MOSI_PIN         NRF_GPIO_PIN_MAP(0, 15)
#define SPI_MNGR_CONFIG1_MISO_PIN         NRF_GPIO_PIN_MAP(0, 13)

#define SPI_MNGR_CONFIG2_SCK_PIN          NRF_GPIO_PIN_MAP(0, 14)
#define SPI_MNGR_CONFIG2_MOSI_PIN         NRF_GPIO_PIN_MAP(0, 15)
#define SPI_MNGR_CONFIG2_MISO_PIN         NRF_GPIO_PIN_MAP(0, 13)

#define SPI_MNGR_CONFIG3_SCK_PIN          NRF_GPIO_PIN_MAP(0, 7)
#define SPI_MNGR_CONFIG3_MOSI_PIN         NRF_GPIO_PIN_MAP(0, 6)
#define SPI_MNGR_CONFIG3_MISO_PIN         NRF_GPIO_PIN_MAP(0, 8)
/* ***************** */
/*       Apps        */
/* ***************** */
/* App Body Temp - ads1114 */
#define APP_BODY_TEMP_SAMPLING_RATE   6000    
#define APP_BODY_TEMP_UPPER_TH        380
#define APP_BODY_TEMP_LOWER_TH        350

/* App Charge */
#define APP_CHARGE_SAMPLING_RATE      15000

/* App LCD */
#define LCD_HEIGHT                                64    /* LCD height in pixels */
#define LCD_WIDTH                                 128   /* LCD widht in pixels */
#define LCD_ROTATION                              0     /* LCD rotation used for portrait and landscape mode, static for now */
#define LCD_STATBAR_REFRESH_RATE                  200  /* LCD refresh rate for the status bar (the top bar) */
#define LCD_ACTIVE_AREA_REFRESH_RATE              500  /* LCD refresh rate for the active area */
#define LCD_INFOBAR_REFRESH_RATE                  2000  /* LCD refresh rate for the information bar */

/* App uSD */
#define APP_USD_PWR_EN                                      NRF_GPIO_PIN_MAP(0, 4)
#define APP_USD_MOSI_PIN                                    6  
#define APP_USD_MISO_PIN                                    8  
#define APP_USD_SCK_PIN                                     7 
#define APP_USD_CS_PIN                                      5  

/* App uSD */
#define USD_ACTIVE                                0

/* ***************** */
/*       Serial      */
/* ***************** */

/* Evian */
#define DEVICE_CLOUD_ID               191909076200000068          /* ID used to communicate to the cloud */


/* ***************** */
/*      Measures     */
/* ***************** */
/* Cloud measurement type */
#define GAMA_MEASURE_ECG          228

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
#define MEAS_TIMESTAMP    "Timestamp"  
#define MEAS_TEMP         "Temperature"   
#define MEAS_RESP         "Respiration"  
#define MEAS_ECG_I        "Lead I"
#define MEAS_ECG_II       "Lead II"
#define MEAS_ECG_III      "Lead III"
#define MEAS_ECG_AVR      "Lead aVR"
#define MEAS_ECG_AVL      "Lead aVL"
#define MEAS_ECG_AVF      "Lead aVF"
#define MEAS_ECG_V1       "Lead V1"
#define MEAS_ECG_V2       "Lead V2"
#define MEAS_ECG_V3       "Lead V3"
#define MEAS_ECG_V4       "Lead V4"
#define MEAS_ECG_V5       "Lead V5"
#define MEAS_ECG_V6       "Lead V6"
#define MEAS_ECG_PACE     "Pace Status"
#define MEAS_ECG_LOFF_LA  "Lead-off LA"
#define MEAS_ECG_LOFF_RA  "Lead-off RA"
#define MEAS_ECG_LOFF_LL  "Lead-off LL"
#define MEAS_ECG_LOFF_V1  "Lead-off V1"
#define MEAS_ECG_LOFF_V2  "Lead-off V2"
#define MEAS_ECG_LOFF_V3  "Lead-off V3"
#define MEAS_ECG_LOFF_V4  "Lead-off V4"
#define MEAS_ECG_LOFF_V5  "Lead-off V5"
#define MEAS_ECG_LOFF_V6  "Lead-off V6"

/* Array with content measurements transfers */
#define MEASURES_CONTENT    \
  {                         \
    MEAS_TIMESTAMP,         \
    MEAS_TEMP,              \
    MEAS_TIMESTAMP,         \
    MEAS_ECG_LOFF_LA,       \
    MEAS_ECG_LOFF_RA,       \
    MEAS_ECG_LOFF_LL,       \
    MEAS_ECG_LOFF_V1,       \
    MEAS_ECG_LOFF_V2,       \
    MEAS_ECG_LOFF_V3,       \
    MEAS_ECG_LOFF_V4,       \
    MEAS_ECG_LOFF_V5,       \
    MEAS_ECG_LOFF_V6,       \
    MEAS_ECG_PACE,          \
    MEAS_ECG_I,             \
    MEAS_ECG_II,            \
    MEAS_ECG_III,           \
    MEAS_ECG_AVR,           \
    MEAS_ECG_AVL,           \
    MEAS_ECG_AVF,           \
    MEAS_ECG_V2,            \
    MEAS_RESP,              \
    MEAS_ECG_LOFF_V3,       \
    MEAS_ECG_LOFF_V4,       \
    MEAS_ECG_LOFF_V5,       \
    MEAS_ECG_LOFF_V6        \
  }
#define DEVICE_MEASURES_NUMBER         25

/* Size macros in bytes to use in MEASURES_CONTENT_SIZE */
typedef enum {                                     
  MEAS_1BYTE  = (1),    /* uint8_t */
  MEAS_2BYTE  = (2),    /* uint16_t */
  MEAS_4BYTE  = (4),    /* uint32_t */
  MEAS_8BYTE  = (8),    /* uint64_t */  
} measures_size;

/* Array with size of content measurements transfers */
#define MEASURES_CONTENT_SIZE \
  {                           \
    MEAS_8BYTE,               \
    MEAS_4BYTE,               \
    MEAS_8BYTE,               \
    MEAS_1BYTE,               \
    MEAS_1BYTE,               \
    MEAS_1BYTE,               \
    MEAS_1BYTE,               \
    MEAS_1BYTE,               \
    MEAS_1BYTE,               \
    MEAS_1BYTE,               \
    MEAS_1BYTE,               \
    MEAS_1BYTE,               \
    MEAS_1BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE,               \
    MEAS_4BYTE                \
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

#endif /* CONFIG_H */
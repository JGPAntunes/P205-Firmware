/*
* @file		measurements_mngr.h
* @date		July 2021
* @author	PFaria & JAntunes
*
* @brief	This is the header for the measurement manager utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef MEAS_MNGR_H_
#define MEAS_MNGR_H_

/********************************** Includes ***********************************/
/* Config */
#include "config.h"

/* Standard library */
#include <stdint.h>
#include <stdbool.h>

/********************************** Definições ***********************************/

/* Struct that stores pointers to the app buffers */
typedef struct {
  uint8_t  *data;       /* Pointer to the data buffer */
  uint64_t timestamp;   /* Timestamp of packet, can be 0 if a chain of packets are sent */
  uint8_t  type;        /* Packet type */
  uint16_t nbytes;      /* Number of data bytes */
}packet_struct;

/* Struct that stores pointers to the app buffers */
typedef struct {
  uint64_t timestamp;   /* Timestamp of packet, cannot be 0 */
  uint8_t  alert_type;  /* Packet type */
}alert_packet_struct;

///* Struct that stores the queue pointers */
//struct typedef {
//  uint8_t *read_ptr;
//  uint8_t *write_ptr;

//}queue_struct;

/* App states */
typedef enum {
  MEAS_MNGR_INITIALIZING,
  MEAS_MNGR_IDLE
} meas_mngr_states;

#define MC_23k640_CS2           NRF_GPIO_PIN_MAP(0, 31)        /*  */

/* Sequence */
#define READ_RAM_THRESHOLD      8       /* Number of packets sent from MCU 1 */

/* Sequence */
#define ECG_SEQUENCE_SIZE       100     /* Maximum sequence buffer sizes */
#define RAM_BUFFER_SIZE         500     /* Maximum sequence buffer sizes */
#define SEQUENCE_ERROR          0xfafa  /* */

#define RAM_THRESHOLD           320     /* */
#define MAX_RAM_RETRIES         2       /* */

#define UPLOAD_ECG_MEAS         MEAS_ECG_III    /* */



extern const uint8_t GAMA_QUEUE_ADD_MAX_ATTEMPTS;               ///< Tells the maximum attempts to add a new packets to Gama queue.

//#define NTC_RESISTANCE_TYPE     1       /* Measure type that refers to body temperature data */
//#define CPU_TEMPERATURE_TYPE    2       /* Measure type that refers to CPU internal temperature data */
//#define BATTERY_VOLTAGE_TYPE    3       /* Measure type that refers to current battery voltage data */
//#define ECG_V1_TYPE             4       /* Measure type that refers to V1 lead data */
//#define ECG_V2_TYPE             5       /* Measure type that refers to V2 lead data */
//#define ECG_V3_TYPE             6       /* Measure type that refers to V3 lead data */
//#define ECG_V4_TYPE             7       /* Measure type that refers to V4 lead data */
//#define ECG_V5_TYPE             8       /* Measure type that refers to V5 lead data */
//#define ECG_V6_TYPE             9       /* Measure type that refers to V6 lead data */
//#define ECG_aVL_TYPE            10      /* Measure type that refers to aVL lead data */
//#define ECG_aVR_TYPE            11      /* Measure type that refers to aVR lead data */
//#define ECG_aVF_TYPE            12      /* Measure type that refers to aVF lead data */
//#define ECG_I_TYPE              13      /* Measure type that refers to I lead data */
//#define ECG_II_TYPE             14      /* Measure type that refers to II lead data */
//#define ECG_III_TYPE            15      /* Measure type that refers to III lead data */
//#define ECG_U_WAVE_TYPE         16      /* Measure type that refers to U wave duration data */
//#define ECG_P_WAVE_TYPE         17      /* Measure type that refers to P wave duration data */
//#define ECG_T_WAVE_TYPE         18      /* Measure type that refers to T wave duration lead data */
//#define ECG_PR_TYPE             19      /* Measure type that refers to PR segment duration data */
//#define ECG_ST_TYPE             20      /* Measure type that refers to ST segment duration data */
//#define ECG_QT_TYPE             21      /* Measure type that refers to QT segment duration data */
//#define ECG_QRS_TYPE            22      /* Measure type that refers to QRS segment duration data */
//#define ECG_AXIS_TYPE           23      /* Measure type that refers to axis data */
//#define ECG_HR_TYPE             24      /* Measure type that refers to heart rate data */
//#define ECG_RR_TYPE             25      /* Measure type that refers to respiratory  rate data */
//#define ECG_LPWAN_RSSI_TYPE     26     /* Measure type that refers to RA lead data */


#define UPLOAD_DATA_ID          0       /* ID that tells the SD card driver where to store data */
#define UPLOAD_ALERT_ID         1       /* ID that tells the SD card driver where to store data */

/* Alert packet types */
#define LOW_TEMP_ALERT          1       /* The lower temperature threshold has been reached */
#define HIGH_TEMP_ALERT         2       /* The lower temperature threshold has been reached */
#define BATTERY_CHRG_ALERT      3       /* Battery charging detected */
#define PACE_PULSE_ALERT        4       /* Pacemaker discharge detected */ 
#define LEAD_OFF_ALERT          5       /* Lead off detected */
#define POWER_SAVING_TYPE       XX      /* Check Sensefinity value */
#define HELLO_TYPE              XX      /* Check Sensefinity value */

#define DATA_BUFFER_SIZE                80      /* Buffer size for the periodic write on external RAM */
#define ALERT_BUFFER_SIZE               50      /* Buffer size for the alerts buffer to be written on external RAM */
#define BUFFER_SIZE_RAM_THRESHOLD       70      /* Threshold size to start an upload to RAM */
#define BUFFER_SIZE_TELCO_THRESHOLD     70      /* Threshold size to start an upload to telco */
/********************************** Funções ***********************************/

void meas_mngr_init(void);
void meas_mngr_ecg_set_sequence(char *ecg_sequence[], uint16_t sequence_size);
bool meas_mngr_store_ecg(uint8_t *data);
bool meas_mngr_store_temp(uint8_t *data);
void meas_mngr_loop(void);
#endif /* MEASUREMENTS_MNGR_H_ */


/*
* @file		app_usd.H
* @date		September 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the app_usd aplication.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef APP_USD_H_
#define APP_USD_H_

/********************************** Includes ***********************************/
/* SDK */
#include "ads1114.h"

/* Config */
#include "config.h"

/* standard library */
#include <stdint.h>
#include <stdbool.h>

/********************************** Definições ***********************************/
/* Files */
#define APP_USD_README_FILE                                 "README.TXT"
#define APP_USD_CSV_FILE                                    "DATA%d.CSV"
#define APP_USD_CONFIG_FILE                                 "CONFIG.TXT"

/* APP_USD_README_FILE */
#define APP_USD_README_ID                                   "SAMB V2.0\n"
#define APP_USD_README_STRING1                              "Signal Aquisition Medical Board\nThis is a private medical information and must be preserved\nDevelopers of SAMB:\nPedro Faria e Jorge Antunes\n"
#define APP_USD_README_STRING2                              "Device was created in 2021 by Cave Tech company with Sensefinity partnership\n" 
#define APP_USD_README_ID_SIZE                              ARRAY_SIZE(APP_USD_README_ID)
#define APP_USD_README_STRING1_SIZE                         ARRAY_SIZE(APP_USD_README_STRING1)
#define APP_USD_README_STRING2_SIZE                         ARRAY_SIZE(APP_USD_README_STRING2)    

#define APP_USD_COL1_SIZE                                   1

/* APP_USD_DATA_FILE */
#define APP_USD_DATAF_NAME                                  "DATA"
#define APP_USD_DATAF_NAME_SIZE                             11
#define APP_USD_DATAF_NAME_INIT_NUMBER                      0
#define APP_USD_DATAF_NAME_DATA_SIZE                        ARRAY_SIZE(APP_USD_DATAF_NAME)
#define APP_USD_DATAF_TITLE                                 "Document with ECG and temperature signals acquired by the SAMB V1.0 equipment\n\nThis output file is proprietary of Cave Tech company\n"
#define APP_USD_DATAF_DESCRIPTION1                          "Disclaimer: Cave Tech company is not responsible for the incorrect use of the private information of the patient\n\n\n"
#define APP_USD_DATAF_PATIENT1                              "Monitored patient information:\n"
#define APP_USD_DATAF_PATIENT2                              "ID;%d;\nName;%s;\nAge;%d;Gender;%s;\n\n\n"
#define APP_USD_DATAF_TITLE_SIZE                            ARRAY_SIZE(APP_USD_DATAF_TITLE)
#define APP_USD_DATAF_DESCRIPTION1_SIZE                     ARRAY_SIZE(APP_USD_DATAF_DESCRIPTION1)
#define APP_USD_DATAF_PATIENT1_SIZE                         ARRAY_SIZE(APP_USD_DATAF_PATIENT1)
#define APP_USD_DATAF_ID_OFFSET                             APP_USD_DATAF_TITLE_SIZE + APP_USD_DATAF_DESCRIPTION1_SIZE + APP_USD_DATAF_PATIENT1_SIZE - 3
#define APP_USD_DATAF_ID_STR                                "ID"
#define APP_USD_DATAF_NAME_STR                              "Name"
#define APP_USD_DATAF_AGE_STR                               "Age"
#define APP_USD_DATAF_GENDER_STR                            "Gender"
#define APP_USD_DATAF_ID_STR_SIZE                           ARRAY_SIZE(APP_USD_DATAF_ID_STR) + 5
#define APP_USD_DATAF_NAME_STR_SIZE                         ARRAY_SIZE(APP_USD_DATAF_NAME_STR) + 30
#define APP_USD_DATAF_AGE_STR_SIZE                          ARRAY_SIZE(APP_USD_DATAF_AGE_STR) + 2
#define APP_USD_DATAF_GENDER_STR_SIZE                       ARRAY_SIZE(APP_USD_DATAF_GENDER_STR) + 10
#define APP_USD_DATAF_PATIENT_INFO_SIZE                     APP_USD_DATAF_ID_STR_SIZE + APP_USD_DATAF_NAME_STR_SIZE + APP_USD_DATAF_AGE_STR_SIZE + APP_USD_DATAF_GENDER_STR_SIZE
#define APP_USD_PATIENT_ELEMENTS                            4
#define APP_USD_PATIENT_DELIMITER1                          ";"

/* APP_USD_CONFIG_FILE */
#define APP_USD_CFG_TITLE                                   "Configuration file, only people allowed can modify\n"
//#define APP_USD_CFG_SIZE                                    200
#define APP_USD_CFG_STR                                     "%s{%d}\n"
#define APP_USD_CFG_DELIMITER1                              "{"
#define APP_USD_CFG_DELIMITER2                              "}"
#define APP_USD_CFG_SIZE                                    322
#define APP_USD_CFG_ELEMENTS                                2
#define APP_USD_CFG_TITLE_SIZE                              ARRAY_SIZE(APP_USD_CFG_TITLE)

/* uSD device configs Identifiers */
#define APP_USD_ID1   "Mode"
#define APP_USD_ID2   "ECG"
#define APP_USD_ID3   "Leaf-off"
#define APP_USD_ID4   "ECG gain"
#define APP_USD_ID5   "ECG accuracy"
#define APP_USD_ID6   "Pace"
#define APP_USD_ID7   "Resp gain"
#define APP_USD_ID8   "Temp"
#define APP_USD_ID9   "Temp low"
#define APP_USD_ID10  "Temp high"
#define APP_USD_ID11  "Heart rate low"
#define APP_USD_ID12  "Heart rate high"
#define APP_USD_ID13  "Resp rate low"
#define APP_USD_ID14  "Resp rate high"

/* Measurments macros */
#define APP_USD_MEAS_TITLE_SIZE                             150
#define APP_USD_MEAS_BATCH_NUMBER                           20
#define APP_USD_MEAS_BATCH_SIZE                             100
#define APP_USD_MEAS_UPLOAD_MOUNT_TH                        2
#define APP_USD_SAMPLES_ADDR_OFFSET                         11
#define APP_USD_BYTES_IN_64BITS                             8

/* uSD data sampling internal states */
typedef enum {
  APP_USD_INIT,
  APP_USD_CONFIG,
  APP_USD_GET_CFG_FILE,
  APP_USD_GET_CSV_FILE,
  APP_USD_CONFIG_IDDLE,
  APP_USD_IDLE,
  APP_USD_SAVE_MODE,
  APP_USD_NOP
} app_usd_sampling_states;

/* Patient status */
typedef enum {
  APP_USD_PATIENT_ADD,
  APP_USD_PATIENT_NO_OVERWRITE,
  APP_USD_PATIENT_OVERWRITE
} app_usd_patient_state;

/* Files status */
typedef enum {
  APP_USD_CSV_OPEN,
  APP_USD_CSV_CLOSED,
  APP_USD_CFG_OPEN,
  APP_USD_CFG_CLOSED
} app_usd_files_state;


/********************************** Funções ***********************************/
void app_usd_init(uint8_t upload_meas_th);
void app_usd_loop(bool save_mode);
bool app_usd_is_busy(void);
void app_usd_add_device_config(device_config *config);
device_config *app_usd_get_device_config(void);
void app_usd_add_patient_info(patient_info *patient, bool overwrite);
patient_info *app_usd_get_patient_info(void);
bool app_usd_add_measurement(uint8_t *meas_buffer, uint8_t size);

#endif /* APP_USD_H_ */



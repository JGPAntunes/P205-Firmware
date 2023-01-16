/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/*
 * @file  cellular.h
 * @brief Implements the cellular app header.
 */

#ifndef CELLULAR_H
#define CELLULAR_H

/********************************** Includes ***********************************/

/* Standard C library */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Utils */
#include "sense_library/utils/base64.h"
#include "sense_library/utils/utils.h"
#include "cellular_utilities.h"

/* Gama protocol */
#include "sense_library/protocol-gama/gama_fw/include/gama_node_position.h"

/* Sensors */
#include "sense_library/sensors/simcom.h"

/* Uplink to get buffer sizes */
#include "sense_library/uplink/uplink.h"

/* GPS utilities */
#include "sense_library/gps/gps_utilities.h"

/************************** Relevant default definitions ***********************/
#define cellular_REMAIN_DATA_SESSION_OPEN_DEFAULT     (5000)     ///< Tells the time to remain with cellular data session open; 0 = always and forever open; x = remains open x milliseconds (x > 0) (default value).
#define REGISTRATION_FAILED_MAX_RETRY_TIMEOUT_DEFAULT (10800000) ///< If network registration fails try after the specified milliseconds (default value).
#define SESSION_FAILED_MAX_RETRY_TIMEOUT_DEFAULT		  (REGISTRATION_FAILED_MAX_RETRY_TIMEOUT_DEFAULT) ///< Same as /p REGISTRATION_FAILED_MAX_RETRY_TIMEOUT_DEFAULT but for data session (default value).
#define MACHINATES_VERSION_DEFAULT                    (1)        ///< Tells the cloud entry point to upload data (default value).
#define HTTPS_DEFAULT                                 (0)        ///< Tells if HTTPS on or off (default value). But default values will be updated according to cellular_loop https_on value.

/********************************** Other definitions ***********************************/
typedef bool (*registration_failure_create_message_callback_def)(
  uint64_t current_timestamp, uint64_t registration_failed_time);
typedef bool (*session_failure_create_message_callback_def)(
  uint64_t current_timestamp, uint64_t session_failure_time);
typedef bool (*http_failure_create_message_callback_def)(
  uint64_t current_timestamp, uint64_t http_failed_time);
typedef bool (*timer_3346_create_message_callback_def)(
  uint64_t current_timestamp, uint64_t timer_3346);
typedef void (*sensoroid_send_hello_msg_callback_def)(char* iccid, char* imei,
  char* imsi, char* simcom_version, uint64_t current_timestamp);

typedef void (*cellular_enable_timer_callback_def)(uint32_t milliseconds);
typedef void (*cellular_disable_timer_callback_def)(void);

typedef void (*cellular_init_vbat_ctrl_callback_def)(void);
typedef void (*cellular_clear_vbat_ctrl_callback_def)(void);
typedef void (*cellular_set_vbat_ctrl_callback_def)(void);

typedef void (*cellular_init_power_key_callback_def)(void);
typedef void (*cellular_clear_power_key_callback_def)(void);
typedef void (*cellular_set_power_key_callback_def)(void);

typedef void (*cellular_init_lvl_sh_oe_callback_def)(void);
typedef void (*cellular_clear_lvl_sh_oe_callback_def)(void);
typedef void (*cellular_set_lvl_sh_oe_callback_def)(void);

typedef void (*cellular_init_status_callback_def)(void);
typedef bool (*cellular_read_status_callback_def)(void);

typedef bool (*power_management_is_battery_saving_mode_callback_def)(void);

typedef uint64_t (*sysclk_get_in_ms_callback_def)(void);
typedef void (*sysclk_delay_in_ms_callback_def)(uint64_t ms_timeout);

typedef void (*module_communication_init_callback_def)(void);
typedef void (*module_communication_shutdown_callback_def)(void);

typedef enum _simcom_module_version {
  SIMCOM_UNKNOWN_VERSION  = (0),
  SIMCOM_800_VERSION      = (1),
  SIMCOM_7020_VERSION     = (2),
  SIMCOM_7600_VERSION     = (3),
  SIMCOM_868_VERSION      = (4),
  SIMCOM_7080_VERSION     = (5)
} SIMCOM_Module_Version;

#define PRINT_TIMEOUT                             (300000)

#define TX_BUFFER_SIZE                            (UPLINK_TX_BUFFER_SIZE)
#define RX_BUFFER_SIZE                            (UPLINK_RX_BUFFER_SIZE + 1)
#define COMMAND_SIZE                              (50)

#define GPS_RX_VALID_FIX_MAXIMUM_TIMEOUT          (600000)
#define GPS_RX_VALID_FIX_GUARD_TIMEOUT            (60000)

#define cellular_REGISTRATION_TIMEOUT_SEND_FAILURE_MSG  (900000)
#define cellular_SESSION_TIMEOUT_SEND_FAILURE_MSG       (900000)
#define cellular_HTTP_TIMEOUT_SEND_FAILURE_MSG          (900000)

#define TIMEOUT_TO_COLLECT_CELLS                  (15000) // Corresponds to the timeout to collect a representative amount of cells in milliseconds

#define cellular_RESTART_MODULE_DEFAULT_TIMEOUT   (15000) // In milliseconds

#define HTTP_TIMEOUT_WAIT_FOR_MORE_DATA           (3000) // In milliseconds

#define cellular_MODULE_OK_TIMEOUT                (300000) // In milliseconds

#define NUMBER_OF_COMMANDS_TX_TO_FORCE            (3)

#define IMSI_MAX_LENGTH                           (30)
#define ICCID_MAX_LENGTH                          (23)
#define IMEI_MAX_LENGTH                           (17)
#define SIMCOM_VERSION_MAX_LENGTH                 (60)

#define MAX_CONSECUTIVE_NOKS_RECEIVED             (5)

#define HTTP_CLIENT_ERROR_CODE_MAX                (5)
#define HTTP_OTHER_CODES_ERROR_MAX                (5)

#define WRONG_RESPONSE_DEFAULT                    "WRONG_ANSWER_DEFAULT"
#define WRONG_RESPONSE_ERROR                      "ERROR"

struct cellular_data {
  bool      warmup_success;
  bool      info_success;
  bool      http_init;
  bool      http_context_saved;
  bool      https_set;
  uint8_t   reg_status;
  uint8_t   rssi;
  uint8_t   ber;
  char      simcom_version[SIMCOM_VERSION_MAX_LENGTH];
  char      imei[IMEI_MAX_LENGTH];      //Identifies the device. It has 15 digits
  char      iccid[ICCID_MAX_LENGTH];    //Integrated Circuit Card Identifier. Identifies the SIM card
  char      imsi_text[IMSI_MAX_LENGTH]; //Integrated Circuit Card Identifier. Identifies the SIM card
  uint64_t  imsi;                       //The IMSI consists of a Mobile Country Code (MCC), a Mobile Network Code (MNC) and a Mobile Station Identification Number (MSIN)
  uint8_t   data_session_status;
  uint8_t   data_session_attached_status;
  bool      is_time_to_set_data_to_upload;
  bool      has_data_to_upload;
  bool      has_data_to_read;
  uint64_t  data_to_upload_size;
  char      data_to_upload[TX_BUFFER_SIZE];
  bool      data_successfully_uploaded;
  bool      has_downloaded_data;
  uint64_t  downloaded_data_last_read_size;
  uint64_t  downloaded_data_total_size;
  uint64_t  downloaded_data_total_read_size;
  char      downloaded_data[RX_BUFFER_SIZE];
};

struct cells_data {
  bool      available;
  uint8_t   cells_type;
  uint8_t   cells_number;
  uint16_t  mnc;
  uint16_t  mcc;
  uint16_t  lac[GAMA_CELLS_VECTOR_MAX_SIZE];
  uint32_t  ids[GAMA_CELLS_VECTOR_MAX_SIZE];
  uint8_t   rx_levels[GAMA_CELLS_VECTOR_MAX_SIZE];
  uint64_t  last_read_timestamp;
};

struct gps_data {
  bool      available;
};

typedef enum _power_on {
  POWER_ON_BEGINNING       = (0),
  POWER_ON_SET_VBAT_CTRL   = (1),
  POWER_ON_CLEAR_POWER_KEY = (2),
  POWER_ON_WAIT_FOR_STATUS = (3),
  POWER_ON_FINISHING       = (4)
} Power_On_Phases;

typedef enum _power_off {
  POWER_OFF_BEGINNING               = (0),
  POWER_OFF_SET_POWER_KEY           = (1),
  POWER_OFF_CLEAR_POWER_KEY         = (2),
  POWER_OFF_CLEAR_VBAT_CTRL         = (3),
  POWER_OFF_WAIT_FOR_STATUS         = (4),
  POWER_OFF_CLEAR_PINS              = (5),
  POWER_OFF_CLEAR_PINS_WAITING_TIME = (6),
  POWER_OFF_FINISHING               = (7)
} Power_Off_Phases;

//Power on timeouts
#define SET_VBAT_CTRL_POWER_ON_TIMEOUT      (1000)
#define SET_POWER_KEY_POWER_ON_TIMEOUT      (2000)

//Power off timeouts
#define CLEAR_POWER_KEY_POWER_OFF_TIMEOUT   (1000)
#define SET_POWER_KEY_POWER_OFF_TIMEOUT     (2000)
#define CLEAR_PINS_WAITING_TIMEOUT          (1000)

typedef enum _cellular_phases {
  IDLE_PHASE          = (0),
  DATA_SESSION_PHASE  = (1),
  HTTP_PHASE          = (2),
  CELLS_PHASE         = (3),
  GPS_PHASE           = (4)
} cellular_Phases;

// Note: the states defined at higher columns are exclusive to NB-IoT.
//       But those defined at lower columns are mainly for GSM
typedef enum _warmup_states {
  WARMUP_IDLE_STATE                 = (0),
  WARMUP_STARTING_STATE             = (1),
  WARMUP_AT_TEST_STATE              = (2),
  WARMUP_IS_REGISTERED_STATE        = (3),
  WARMUP_GET_CSQ_STATE              = (4),
  WARMUP_FINISHED_STATE             = (5),
  WARMUP_SET_APN_NBIOT_FIND_IMSI_STATE_PHASE_1      = (6),
  WARMUP_SET_APN_NBIOT_STATE_PHASE_2                = (7),
  WARMUP_SET_APN_NBIOT_DISABLE_RADIO_STATE_PHASE_4  = (8),
  WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_5   = (9),
  WARMUP_GET_GMR_PHASE              = (10),
  WARMUP_SET_BAND                                   = (11),
  WARMUP_SET_APN_NBIOT_STATE_PHASE_4                = (13),
  WARMUP_SET_PSM                                    = (14),
  WARMUP_SET_EDRX                                   = (15),
  WARMUP_SET_UNSOLICITED_PSM_CODE                   = (16),
  WARMUP_SET_PSM_WAKEUP_INDICATION                  = (17),
  WARMUP_DISABLE_PSM                                = (18),
  WARMUP_GET_EDRX_STATUS                            = (19),
  WARMUP_GET_EDRX_PARAMETERS                        = (20),
  WARMUP_DISABLE_EDRX                               = (21),
  WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_6   = (22),
  WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_7   = (23),
  WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_8   = (24),
  WARMUP_SET_APN_NBIOT_ENABLE_RADIO_STATE_PHASE_9   = (25),
  WARMUP_SET_APN_NBIOT_STATE_PHASE_3                = (26)
} WarmUp_States;

typedef enum _info_states {
  INFO_IDLE_STATE       = (0),
  INFO_STARTING_STATE   = (1),
  INFO_GET_IMEI_STATE   = (2),
  INFO_GET_ICCID_STATE  = (3),
  INFO_GET_IMSI_STATE   = (4),
  INFO_FINISHED_STATE   = (5)
} Info_States;

typedef enum _data_session_states {
  DATA_SESSION_IDLE_STATE                   = (0),
  DATA_SESSION_STARTING_STATE               = (1),
  DATA_SESSION_IS_OPEN_STATE                = (2),
  DATA_SESSION_SET_APN_STATE                = (3),
  DATA_SESSION_SET_USER_APN_STATE           = (4),
  DATA_SESSION_SET_PASS_APN_STATE           = (5),
  DATA_SESSION_OPENING_STATE                = (6),
  DATA_SESSION_CLOSING_STATE                = (7),
  DATA_SESSION_REMAIN_OPEN                  = (8),
  DATA_SESSION_FINISHED_STATE               = (9),
  DATA_SESSION_IS_ATTACHED_TO_NETWORK_STATE = (10),
  DATA_SESSION_ATTACH_TO_NETWORK_STATE      = (11),
  DATA_SESSION_DETACH_TO_NETWORK_STATE      = (12)
} Data_Session_States;

typedef enum _http_states {
  HTTP_IDLE_STATE               = (0),
  HTTP_STARTING_STATE           = (1),
  HTTP_INIT_STATE               = (2),
  HTTP_CID_STATE                = (3),
  HTTP_END_POINT_STATE          = (4),
  HTTP_SAVE_CONTEXT_STATE       = (5),
  HTTP_UPLOAD_DATA_STATE        = (6),
  HTTP_STORE_DATA_STATE         = (7),
  HTTP_ACTION_STATE             = (8),
  HTTP_READ_STATE               = (9),
  HTTP_TERM_STATE               = (10),
  HTTP_FINISHED_STATE           = (11),
  HTTP_DESTROY_STATE            = (12),
  HTTP_WAIT_FOR_MORE_DATA_STATE = (13),
  HTTPS_ENABLE_STATE            = (14),
  HTTPS_IS_ENABLED_STATE        = (15),
  HTTP_SET_VERSION_PARAM_STATE  = (16),
  HTTPS_DISABLE_STATE           = (17),
  HTTP_SET_CON_STATE            = (18),
  HTTP_BAD_CODE_STATE           = (19),
  HTTP_SSL_CONFIGURATION_STATE  = (20),
  HTTP_SSL_VERIFICATION_STATE   = (21),
  HTTP_SET_CONNECT_SERVER_STATE = (22),
  HTTP_SET_BODY_LEN_STATE       = (23),
  HTTP_SET_HEADER_LEN_STATE     = (24),
  HTTP_CONNECT_STATE            = (25),
  HTTP_GET_STATUS_STATE         = (26),
  HTTP_CLEAR_HEADER_STATE       = (27),
  HTTP_SET_HEADER_1_STATE       = (28),
  HTTP_SET_HEADER_2_STATE       = (29),
  HTTP_SET_HEADER_3_STATE       = (30),
  HTTP_SET_HEADER_4_STATE       = (31),
  HTTP_SET_HEADER_5_STATE       = (32)
} Http_States;

typedef enum _cells_states {
  CELLS_IDLE_STATE                = (0),
  CELLS_STARTING_STATE            = (1),
  CELLS_SET_MODE_STATE            = (2),
  CELLS_WAITING_FOR_COLLECTION    = (3),
  CELLS_GET_STATE                 = (4),
  CELLS_FINISHED_STATE            = (5),
  CELLS_WAKE_UP_FROM_PSM_STATE_1  = (6),
  CELLS_WAKE_UP_FROM_PSM_STATE_2  = (7)
} Cells_States;

typedef enum _gps_states {
  GPS_IDLE_STATE                    = (0),
  GPS_STARTING_STATE                = (1),
  GPS_START_MODE_STATE              = (2),
  GPS_START_RMC_SENTENCE_MODE_STATE = (3),
  GPS_STOP_RMC_SENTENCE_MODE_STATE  = (4),
  GPS_STOP_MODE_STATE               = (5),
  GPS_FINISHED_STATE                = (6)
} Gps_States;

struct current_states {
  WarmUp_States       warmup;
  Info_States         info;
  Data_Session_States data_session;
  Http_States         http;
  Cells_States        cells;
  Gps_States          gps;
};

typedef enum _action_list {
  IDLE                  = (0),
  SENDING_COMMAND       = (1),
  WAITING_RESPONSE      = (2),
  GOOD_RESPONSE         = (3),
  ERROR_RESPONSE        = (4),
  BAD_RESPONSE          = (5),
  STAND_ALONE_RESPONSE  = (6)
} Action_List;

#define COMMAND_RETRY_TIME_250MS          (250)
#define COMMAND_RETRY_TIME_500MS          (500)
#define COMMAND_RETRY_TIME_5000MS         (5000)
#define COMMAND_RETRY_TIME_1000MS         (1000)
#define COMMAND_RETRY_TIME_2000MS         (2000)
#define COMMAND_RETRY_TIME_5000MS         (5000)
#define COMMAND_RETRY_TIME_10000MS        (10000)
#define COMMAND_RETRY_TIME_15000MS        (15000)
#define COMMAND_RETRY_TIME_30000MS        (30000)
#define COMMAND_RETRY_TIME_45000MS        (45000)
#define COMMAND_RETRY_TIME_60000MS        (60000)
#define COMMAND_RETRY_TIME_90000MS        (90000)
#define COMMAND_RETRY_TIME_120000MS       (120000)
#define COMMAND_RETRY_TIME_180000MS       (180000)

#define RESPONSE_WAITING_TIME_1S          (1000)
#define RESPONSE_WAITING_TIME_2S          (2000)
#define RESPONSE_WAITING_TIME_4S          (4000)
#define RESPONSE_WAITING_TIME_5S          (5000)
#define RESPONSE_WAITING_TIME_10S         (10000)
#define RESPONSE_WAITING_TIME_15S         (15000)
#define RESPONSE_WAITING_TIME_30S         (30000)
#define RESPONSE_WAITING_TIME_35S         (35000)
#define RESPONSE_WAITING_TIME_45S         (45000)
#define RESPONSE_WAITING_TIME_60S         (60000)
#define RESPONSE_WAITING_TIME_90S         (90000)
#define RESPONSE_WAITING_TIME_120S        (120000)
#define RESPONSE_WAITING_TIME_180S        (180000)
#define RESPONSE_WAITING_TIME_300S        (300000)
#define RESPONSE_WAITING_TIME_900S        (900000)
#define RESPONSE_WAITING_TIME_1800S       (1800000)
#define RESPONSE_WAITING_TIME_3600S       (3600000)

//#define AT_TEST_COMMAND(simcom_version) (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? ("AT+CTRJ?\r") : ("AT\r"))
#define AT_TEST_COMMAND(simcom_version)	  (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? ("AT\r") : ("AT\r"))
#define TIMER_3346_RESPONSE               "+CTRJ: "

#define GET_GMR_COMMAND                   "AT+GMR\r"
#define GET_GMR_REVISION                  "Revision:"
#define GET_GMR_RESPONSE                  "+GMR:"

#define SIMCOM_800_MODEL_STRING           "SIM800"
#define SIMCOM_7020_MODEL_STRING          "SIM7020"
#define SIMCOM_7080_MODEL_STRING          "SIM7080"
#define SIMCOM_7600_MODEL_STRING          "SIM7600"
#define SIMCOM_868_MODEL_STRING           "SIM868"

#define IS_REGISTERED_COMMAND(simcom_version)   (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? ("AT+CEREG?\r") : ("AT+CREG?\r"))
#define IS_REGISTERED_RESPONSE(simcom_version)  (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? ("+CEREG: ") : ("+CREG: "))

#define BAND_MANUAL_SELECTION(simcom_version)   ((simcom_version == SIMCOM_7020_VERSION) ? ("AT+CBAND=") : ("AT+CBAND=ALL_MODE"))

#define SEARCHING_NETWORK                 (2)
#define NOT_SEARCHING_HOME_NETWORK        (0)
#define REGISTERED_HOME_NETWORK           (1)
#define REGISTERED_ROAMING                (5)

#define HTTPS_DISABLED                    (0)
#define HTTPS_ENABLED                     (1)

#define DISABLE_RADIO_COMMAND               "AT+CFUN=0\r"
#define DISABLE_RADIO_RESPONSE              "+CPIN: NOT READY"
#define ENABLE_RADIO_COMMAND                "AT+CFUN=1\r"
#define ENABLE_RADIO_RESPONSE               "+CPIN: READY"

#define DISABLE_PSM_COMMAND                 "AT+CPSMS=0\r"
#define SET_PSM_COMMAND                     "AT+CPSMS=1,,,\""
#define SET_EDRX_COMMAND(simcom_version)    ((simcom_version == SIMCOM_7020_VERSION) ? ("AT+CEDRXS=1,5,\"") : ("AT+CEDRX=2,1,\""))
#define SET_PSM_UNSOLICITED_PSM_CODE_COMMAND  "AT+CEREG=4\r"
#define SET_PSM_WAKEUP_INDICATION           "AT+CPSMSTATUS=1\r"
#define GET_EDRX_STATUS(simcom_version)     ((simcom_version == SIMCOM_7020_VERSION) ? ("AT+CEDRXS?\r") : ("AT+CEDRXRDP\r"))
#define GET_EDRX_PARAMETERS                 "AT+CEDRXRDP\r"
#define DISABLE_EDRX_COMMAND                "AT+CEDRXS=0\r"
#define GET_EDRX_NBIOT_ACTIVATED_RESPONSE   "+CEDRXRDP: 5"
#define GET_EDRX_NBIOT_DEACTIVATED_RESPONSE "+CEDRXRDP: 0"

#define GET_CSQ_COMMAND                     "AT+CSQ\r"
#define GET_CSQ_RESPONSE                    "+CSQ: "
#define CSQ_MIN_SIGNAL(simcom_version)      (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? (0) : (7))

#define GET_IMEI_COMMAND                    "AT+GSN\r"

#define GET_ICCID_COMMAND                   "AT+CCID\r"
#define GET_ICCID_RESPONSE                  "+CCID: "

#define GET_IMSI_COMMAND                    "AT+CIMI\r"

// 7220 AT*MCGDEFCONT command is correct! it must not be AT+MCGDEFCONT
#define SET_APN_1_COMMAND(simcom_version) ((simcom_version == SIMCOM_7020_VERSION) ? ("AT*MCGDEFCONT=\"IP\",\"") : ("AT+CGDCONT=1,\"IP\",\""))
#define SET_APN_2_COMMAND                 "AT+SAPBR=3,1,\"APN\",\""
#define SET_APN_3_COMMAND                 "AT+CGDCONT=1,\"IP\",\""

#define GET_PDP_CONTEXT_COMMAND           "AT+CGDCONT?\r"

#define SET_APN_USER_COMMAND              "AT+SAPBR=3,1,\"USER\",\""

#define SET_APN_PASS_COMMAND              "AT+SAPBR=3,1,\"PWD\",\""

#define IS_DATA_SESSION_OPENED_COMMAND(simcom_version)    (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7600_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? (IS_DATA_SESSION_OPENED_COMMAND_2(simcom_version)) : ("AT+SAPBR=2,1\r"))
#define IS_DATA_SESSION_OPENED_COMMAND_2(simcom_version)  (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7600_VERSION)) ? ("AT+CGACT?\r") : ("AT+CGATT?\r"))

#define IS_DATA_SESSION_OPENED_RESPONSE(simcom_version)   (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7600_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? (IS_DATA_SESSION_OPENED_RESPONSE_2(simcom_version)) : ("+SAPBR: "))
#define IS_DATA_SESSION_OPENED_RESPONSE_2(simcom_version) (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7600_VERSION)) ? ("+CGACT: ") : ("+CGATT: "))

#define DATA_SESSION_OPENING              (0)
#define DATA_SESSION_OPENED               (1)
#define DATA_SESSION_CLOSING              (2)

#define DATA_SESSION_IS_ATTACHED_COMMAND            ("AT+CGATT?\r")
#define DATA_SESSION_IS_ATTACHED_COMMAND_RESPONSE   ("+CGATT: ")

#define DATA_SESSION_ATTACH_COMMAND                 ("AT+CGATT=1\r")
#define DATA_SESSION_DETACH_COMMAND                 ("AT+CGATT=0\r")

#define DATA_SESSION_CLOSED(simcom_version)           (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7600_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? (0) : (3))

#define OPEN_DATA_SESSION_COMMAND(simcom_version)     (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7600_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? (OPEN_DATA_SESSION_COMMAND_2(simcom_version)) : ("AT+SAPBR=1,1\r"))
#define OPEN_DATA_SESSION_COMMAND_2(simcom_version)   (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7600_VERSION)) ? ("AT+CGACT=1,1\r") : ("AT+CGATT=1\r"))

#define CLOSE_DATA_SESSION_COMMAND(simcom_version)    (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7600_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? (CLOSE_DATA_SESSION_COMMAND_2(simcom_version)) : ("AT+SAPBR=0,1\r"))
#define CLOSE_DATA_SESSION_COMMAND_2(simcom_version)  (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7600_VERSION)) ? ("AT+CGACT=0,1\r") : ("AT+CGATT=0\r"))

#define HTTP_INIT_COMMAND(simcom_version, machinates_version, https_on)   (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? HTTP_SET_END_POINT_COMMAND_CREATE(machinates_version, https_on) : ("AT+HTTPINIT\r"))

#define HTTP_CID_COMMAND                      "AT+HTTPPARA=\"CID\",1\r"

#define HTTP_CON_COMMAND                      "AT+CHTTPCON=0\r"

#define HTTP_SET_END_POINT_COMMAND_PARAM(machinates_version, https_on)      ((https_on == 0) ? HTTP_SET_END_POINT_COMMAND_WITHOUT_HTTPS_PARAM(machinates_version) : HTTP_SET_END_POINT_COMMAND_WITH_HTTPS_PARAM(machinates_version))
#define HTTP_SET_END_POINT_COMMAND_WITHOUT_HTTPS_PARAM(machinates_version)  ((machinates_version == 0) ? ("AT+HTTPPARA=\"URL\",\"http://production.sensefinity.com/api/raw\"\r") : ("AT+HTTPPARA=\"URL\",\"http://hub.sensefinity.com\"\r"))
#define HTTP_SET_END_POINT_COMMAND_WITH_HTTPS_PARAM(machinates_version)     ((machinates_version == 0) ? ("AT+HTTPPARA=\"URL\",\"http://production.sensefinity.com/api/raw\"\r") : ("AT+HTTPPARA=\"URL\",\"https://hub.sensefinity.com\"\r"))

#define HTTP_SET_END_POINT_COMMAND_CREATE(machinates_version, https_on)     ((https_on == 0) ? HTTP_SET_END_POINT_COMMAND_WITHOUT_HTTPS_CREATE(machinates_version) : HTTP_SET_END_POINT_COMMAND_WITH_HTTPS_CREATE(machinates_version))
#define HTTP_SET_END_POINT_COMMAND_WITHOUT_HTTPS_CREATE(machinates_version) ((machinates_version == 0) ? ("AT+CHTTPCREATE=\"http://production.sensefinity.com\"\r") : ("AT+CHTTPCREATE=\"http://hub.sensefinity.com\"\r"))
#define HTTP_SET_END_POINT_COMMAND_WITH_HTTPS_CREATE(machinates_version)    ((machinates_version == 0) ? ("AT+CHTTPCREATE=\"http://production.sensefinity.com\"\r") : ("AT+CHTTPCREATE=\"https://hub.sensefinity.com\"\r"))

#define HTTP_SAVE_HTTP_CONTEXT_COMMAND        "AT+HTTPSCONT\r"

#define HTTPS_IS_INIT_COMMAND                 "AT+HTTPSSL?\r"
#define HTTPS_ENABLE_COMMAND                  "AT+HTTPSSL=1\r"
#define HTTPS_DISABLE_COMMAND                 "AT+HTTPSSL=0\r"

#define CHECK_PS_SERVICE_COMMAND              "AT+CGATT?\r"
#define QUERY_APN_COMMAND                     "AT+CGNAPN\r"
#define ACTIVATE_NETWORK_COMMAND              "AT+CNACT=0,1\r"
#define SET_AUTH_COMMAND_1                    "AT+CNCFG=1,1,\""

#define HTTP_SSL_CONFIGURATION_COMMAND        "AT+CSSLCFG=\"sslversion\",1,3\r"
#define HTTP_SSL_VERIFICATION_COMMAND         "AT+SHSSL=1,\"baidu_root_ca.cer\"\r"
#define HTTP_SET_CONNECT_SERVER_COMMAND(machinates_version, https_on)     ((https_on == 0) ? HTTP_SET_CONNECT_SERVER_COMMAND_WITHOUT_HTTPS(machinates_version) : HTTP_SET_CONNECT_SERVER_COMMAND_WITH_HTTPS(machinates_version))
#define HTTP_SET_CONNECT_SERVER_COMMAND_WITHOUT_HTTPS(machinates_version) ((machinates_version == 0) ? ("AT+SHCONF=\"URL\",\"http://production.sensefinity.com\"\r") : ("AT+SHCONF=\"URL\",\"http://hub.sensefinity.com\"\r"))
#define HTTP_SET_CONNECT_SERVER_COMMAND_WITH_HTTPS(machinates_version)    ((machinates_version == 0) ? ("AT+SHCONF=\"URL\",\"http://production.sensefinity.com\"\r") : ("AT+SHCONF=\"URL\",\"http://hub.sensefinity.com\"\r"))
#define HTTP_SET_BODY_LEN_COMMAND             "AT+SHCONF=\"BODYLEN\",1024\r"
#define HTTP_SET_HEADER_LEN_COMMAND           "AT+SHCONF=\"HEADERLEN\",350\r"
#define HTTP_CONNECT_COMMAND                  "AT+SHCONN\r"
#define HTTP_GET_STATUS_COMMAND               "AT+SHSTATE?\r"
#define HTTP_GET_STATUS_RESPONSE              "+SHSTATE: 1"
#define HTTP_CLEAR_HEADER_COMMAND             "AT+SHCHEAD\r"
#define HTTP_SET_CONTENT_TYPE_HEADER_COMMAND  "AT+SHAHEAD=\"Content-Type\",\"application/js\"\r"
#define HTTP_SET_CACHE_CONTROL_HEADER_COMMAND "AT+SHAHEAD=\"Cache-control\",\"no-cache\"\r"
#define HTTP_SET_CONNECTION_HEADER_COMMAND    "AT+SHAHEAD=\"Connection\",\"keep-alive\"\r"
#define HTTP_SET_ACCEPT_HEADER_COMMAND        "AT+SHAHEAD=\"Accept\",\"*/*\"\r"
#define HTTP_SET_VERSION_HEADER_COMMAND       "AT+SHAHEAD=\"Version\",\""
#define HTTP_SET_BODY_1_COMMAND               "AT+SHBOD="
#define HTTP_SET_BODY_2_COMMAND               ",2000\r"
#define HTTP_SET_BODY_RESPONSE                ">"
#define HTTP_POST_COMMAND                     "AT+SHREQ=\"\",3\r"
#define HTTP_POST_RESPONSE                    "+SHREQ: "

#define IS_HTTPS_INIT_RESPONSE                "+HTTPSSL: "

#define HTTPS_DISABLED                        (0)
#define HTTPS_ENABLED                         (1)

#define HTTP_SET_VERSION_PARAM_COMMAND_1      "AT+HTTPPARA=\"USERDATA\",\""
#define HTTP_SET_VERSION_PARAM_COMMAND_2      "Version: "
#define HTTP_SET_VERSION_PARAM_COMMAND_3      "\"\r"

#define HTTP_UPLOAD_DATA_COMMAND              "AT+HTTPDATA="
#define HTTP_TIME_TO_STORE_DATA_SUB_COMMAND   ",15000\r"
#define HTTP_UPLOAD_DATA_RESPONSE             "DOWNLOAD"

#define HTTP_ACTION_COMMAND                   "AT+HTTPACTION=1\r"
#define HTTP_SEND_COMMAND_1(machinates_version) ((machinates_version == 0) ? ("AT+CHTTPSEND=0,1,\"/api/raw\",") : ("AT+CHTTPSEND=0,1,\"/\","))
#define HTTP_SEND_COMMAND_2(machinates_version) ((machinates_version == 0) ? (",\"text/plain\",") : (",\"text/plain\","))

#define HTTP_ACTION_RESPONSE(simcom_version)    (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? ("+CHTTPNMIC: ") : ("+HTTPACTION: "))

/* HTTP code classes */
#define HTTP_INFO_CODE_ANSWER_BEGIN                 (100)
#define HTTP_INFO_CODE_ANSWER_END                   (103)
#define HTTP_SUCCESS_CODE_ANSWER_BEGIN              (200)
#define HTTP_SUCCESS_CODE_ANSWER_END                (226)
#define HTTP_REDIRECTION_CODE_ANSWER_BEGIN          (300)
#define HTTP_REDIRECTION_CODE_ANSWER_END            (308)
#define HTTP_CLIENT_ERROR_CODE_ANSWER_BEGIN         (400)
#define HTTP_CLIENT_ERROR_CODE_ANSWER_END           (451)
#define HTTP_SERVER_ERROR_CODE_ANSWER_BEGIN         (500)
#define HTTP_SERVER_ERROR_CODE_ANSWER_END           (511)

#define HTTP_READ_COMMAND(simcom_version)           (((simcom_version != SIMCOM_7080_VERSION)) ? "AT+HTTPREAD=" : "AT+SHREAD=")
#define HTTP_READ_RESPONSE(simcom_version)          (((simcom_version != SIMCOM_7080_VERSION)) ? "+HTTPREAD: " : "+SHREAD: ")

#define HTTP_TERM_COMMAND(simcom_version)           (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? (HTTP_TERM_COMMAND_2(simcom_version)) : ("AT+HTTPTERM\r"))
#define HTTP_TERM_COMMAND_2(simcom_version)         ((simcom_version == SIMCOM_7020_VERSION) ? ("AT+CHTTPDISCON=0\r") : ("AT+SHDISC\r"))

#define HTTP_DESTROY_COMMAND                        "AT+CHTTPDESTROY=0\r"

#define CELLS_SET_MODE_COMMAND_1(simcom_version)    (((simcom_version == SIMCOM_800_VERSION) || (simcom_version == SIMCOM_868_VERSION)) ? ("AT+CENG=3\r") : (CELLS_SET_MODE_COMMAND_2(simcom_version)))
#define CELLS_SET_MODE_COMMAND_2(simcom_version)    ((simcom_version == SIMCOM_7020_VERSION) ? ("AT+CENG=0\r") : ("AT+CENG=1\r"))

#define CELLS_GET_COMMAND(simcom_version)           ((simcom_version == SIMCOM_7600_VERSION) ? ("AT+CPSI?\r") : ("AT+CENG?\r"))
#define CELLS_GET_RESPONSE(simcom_version)          ((simcom_version == SIMCOM_7600_VERSION) ? ("+CPSI: ") : ("+CENG: "))

#define GPS_START_MODE_COMMAND(simcom_version)          ((simcom_version == SIMCOM_7600_VERSION) ? ("AT+CGPS=1\r") : ("AT+CGNSPWR=1\r"))
#define GPS_START_RMC_SENTENCE_COMMAND(simcom_version)  ((simcom_version == SIMCOM_7600_VERSION) ? ("AT+CGPSINFOCFG=1,3\r") : ("AT+CGNSTST=1\r"))
#define GPS_STOP_RMC_SENTENCE_COMMAND(simcom_version)   ((simcom_version == SIMCOM_7600_VERSION) ? ("AT+CGPSINFOCFG=0,0\r") : ("AT+CGNSTST=0\r"))
#define GPS_STOP_MODE_COMMAND(simcom_version)           ((simcom_version == SIMCOM_7600_VERSION) ? ("AT+CGPS=0\r") : ("AT+CGNSPWR=0\r"))
#define GPS_RMC_SENTENCE_PREAMBLE(simcom_version)       ((simcom_version == SIMCOM_7600_VERSION) ? ("$GPRMC") : ("$GNRMC"))
#define GPS_GGA_SENTENCE_PREAMBLE(simcom_version)       ((simcom_version == SIMCOM_7600_VERSION) ? ("$GPGGA") : ("$GNGGA"))		

#define GPS_SENTENCE_SAMPLE_RATE                    (1000)

#define GENERIC_OK_RESPONSE                         "OK"
#define GENERIC_STAND_ALONE_RESPONSE                "STANDALONERESPONSE"

/* Power saving mode definitions. Only to be read as the actual value will be in the
 * configuration line */

/* PSM - T3324 */
#define T3324_MULTIPLIER_1                          (0b00000000) //2 seconds multiplier
#define T3324_MULTIPLIER_2                          (0b00100000) //1 minute multiplier
#define T3324_MULTIPLIER_3                          (0b01000000) //6 minutes multiplier

#define T3324_VALUE_MIN                             (0)
#define T3324_VALUE_MAX                             (2)

#define T3324_VALUE_MASK                            (0b11100000)
#define T3324_VALUE_SHIFT                           (5)

/* PSM - T3412 */
#define T3412_MULTIPLIER_1                          (0b00000000) //10 minutes multiplier
#define T3412_MULTIPLIER_2                          (0b00100000) //1 hour multiplier
#define T3412_MULTIPLIER_3                          (0b01000000) //10 hours multiplier
#define T3412_MULTIPLIER_4                          (0b01100000) //2 seconds multiplier
#define T3412_MULTIPLIER_5                          (0b10000000) //30 seconds multiplier
#define T3412_MULTIPLIER_6                          (0b10100000) //1 minute multiplier
#define T3412_MULTIPLIER_7                          (0b11000000) //320 hours multiplier

#define T3412_VALUE_MIN                             (0)
#define T3412_VALUE_MAX                             (6)

#define T3412_VALUE_MASK                            (0b11100000)
#define T3412_VALUE_SHIFT                           (5)

/* eDRX */
#define EDRX_PERIOD_1                               (0b0001) //10.24 seconds
#define EDRX_PERIOD_2                               (0b0010) //20.48 seconds
#define EDRX_PERIOD_3                               (0b0011) //40.96 seconds
#define EDRX_PERIOD_4                               (0b0100) //61.44 seconds
#define EDRX_PERIOD_5                               (0b0101) //81.92 seconds
#define EDRX_PERIOD_6                               (0b0110) //102,4 seconds
#define EDRX_PERIOD_7                               (0b0111) //122.88 seconds
#define EDRX_PERIOD_8                               (0b1000) //143.36 seconds
#define EDRX_PERIOD_9                               (0b1001) //163.84 seconds
#define EDRX_PERIOD_10                              (0b1010) //327.68 seconds
#define EDRX_PERIOD_11                              (0b1011) //655.36 seconds
#define EDRX_PERIOD_12                              (0b1100) //1310.72 seconds
#define EDRX_PERIOD_13                              (0b1101) //2621.44 seconds
#define EDRX_PERIOD_14                              (0b1110) //5242.88 seconds
#define EDRX_PERIOD_15                              (0b1111) //10485.76 seconds

#define EDRX_PERIOD_VALUE_MIN                       1
#define EDRX_PERIOD_VALUE_MAX                       15

/* PTW */
#define PTW_PERIOD_1                                (0b0000) //2.56 seconds
#define PTW_PERIOD_2                                (0b0001) //5.12 seconds
#define PTW_PERIOD_3                                (0b0010) //7.68 seconds
#define PTW_PERIOD_4                                (0b0011) //10.24 seconds
#define PTW_PERIOD_5                                (0b0100) //12.8 seconds
#define PTW_PERIOD_6                                (0b0101) //15.36 seconds
#define PTW_PERIOD_7                                (0b0110) //17.92 seconds
#define PTW_PERIOD_8                                (0b0111) //20.48 seconds
#define PTW_PERIOD_9                                (0b1000) //23.04 seconds
#define PTW_PERIOD_10                               (0b1001) //25.6 seconds
#define PTW_PERIOD_11                               (0b1010) //28.16 seconds
#define PTW_PERIOD_12                               (0b1011) //30.72 seconds
#define PTW_PERIOD_13                               (0b1100) //33.28 seconds
#define PTW_PERIOD_14                               (0b1101) //35.84 seconds
#define PTW_PERIOD_15                               (0b1110) //38.4 seconds
#define PTW_PERIOD_16                               (0b1111) //40.96 seconds

#define PTW_PERIOD_VALUE_MIN                        0
#define PTW_PERIOD_VALUE_MAX                        15

#define SIMCOM_INIT_TEST_TIMEOUT                    (1000) // In seconds

/* End of power saving mode definitions */

/********************************** Prototypes ***********************************/

#define SESSION_TIMEOUT_STARTING_VALUE                  (300000)
#define HTTP_OTHER_BAD_CODES_TIMEOUT_STARTING_VALUE_1   (10000)
#define HTTP_OTHER_BAD_CODES_TIMEOUT_STARTING_VALUE_2   (300000)
#define HTTP_OTHER_BAD_CODES_TIMEOUT_STARTING_VALUE_3   (600000)
#define HTTP_OTHER_BAD_CODES_TIMEOUT_STARTING_VALUE_4   (900000)
#define HTTP_OTHER_BAD_CODES_TIMEOUT_STARTING_VALUE_5   (1800000)

uint64_t cellular_get_registration_failed_next_try_timeout(void);

uint64_t cellular_get_data_upload_failed_next_try_timeout(void);

void cellular_increment_registration_failed_next_try_timeout(
    uint64_t  current_timestamp, 
    uint64_t  last_registration_timeout);

void cellular_increment_session_failed_next_try_timeout(uint64_t current_timestamp);

void cellular_increment_data_upload_failed_next_try_timeout(void);

/**
 *  Initializes the cellular service
 */
bool cellular_setup(
    registration_failure_create_message_callback_def    registration_failure_create_message_callback,
    session_failure_create_message_callback_def         session_failure_create_message_callback,
    http_failure_create_message_callback_def            http_failure_create_message_callback,
    timer_3346_create_message_callback_def              timer_3346_create_message_callback,
    sensoroid_send_hello_msg_callback_def               sensoroid_send_hello_msg_callback,
    cellular_init_vbat_ctrl_callback_def                cellular_init_vbat_ctrl_callback,
    cellular_clear_vbat_ctrl_callback_def               cellular_clear_vbat_ctrl_callback,
    cellular_set_vbat_ctrl_callback_def                 cellular_set_vbat_ctrl_callback,
    cellular_init_power_key_callback_def                cellular_init_power_key_callback,
    cellular_clear_power_key_callback_def               cellular_clear_power_key_callback,
    cellular_set_power_key_callback_def                 cellular_set_power_key_callback,
    cellular_init_lvl_sh_oe_callback_def                cellular_init_lvl_sh_oe_callback,
    cellular_clear_lvl_sh_oe_callback_def               cellular_clear_lvl_sh_oe_callback,
    cellular_set_lvl_sh_oe_callback_def                 cellular_set_lvl_sh_oe_callback,
    cellular_init_status_callback_def                   cellular_init_status_callback,
    cellular_read_status_callback_def                   cellular_read_status_callback,
    power_management_is_battery_saving_mode_callback_def power_management_is_battery_saving_mode_callback,
    sysclk_get_in_ms_callback_def                       sysclk_get_in_ms_callback,
    module_communication_init_callback_def              module_communication_init,
    module_communication_shutdown_callback_def          module_communication_shutdown);

/**
 *  Updates the cellular service
 */
void cellular_loop(
    uint64_t    current_timestamp, 
    uint64_t    remain_data_session_open,
    uint64_t    registration_failed_next_try, 
    uint16_t    machinates_version,
    uint8_t     https_on, 
    bool        gps_on);

/**
 *    Check if there is data be sent in the buffer, i.e. it indicates if
 *    the data was already uploaded or not
 */
bool cellular_has_data_to_upload(void);

/**
 *    Check if there is data be read
 */
bool cellular_has_data_to_read(void);

/**
 *    Check if it's time to set the data to upload on the cellular module
 */
bool cellular_is_time_to_set_data_to_upload(void);

/**
 * Informs cellular module that there is something to be sent to Machinates
 * @param   has_data_to_upload
 *            The new value of has_data_to_upload.
 */
void cellular_set_has_data_to_upload(bool has_data_to_upload);

/**
 * Set the buffer with data to be uploaded
 * @param   data_to_upload
 *            Corresponds to the data that will be copied to the internal buffer.
 *          data_to_upload_size
 *            The size of the upload data.
 */
bool cellular_set_data_to_upload(
    char*     data_to_upload, 
    uint64_t  data_to_upload_size,
    uint64_t  current_timestamp);

/**
 *  Check if the last stored data packets were successfully uploaded.
 */
bool cellular_data_successfully_uploaded(void);

/**
 *  Returns the a boolean that indicates if there is data to be read.
 *
 */
bool cellular_has_downloaded_data(void);

/**
 * Read the data downloaded from Machinates
 * @details   Copies the data downloaded from Machinates to the buffer passed as argument
 * @param     downloaded_data
 *              Buffer to copy data.
 */
uint8_t cellular_get_downloaded_data(char* downloaded_data);

/**
 *  Indicates that it's time to collect cells
 */
void cellular_collect_cells(uint64_t current_timestamp);

/**
 *  Indicates if there is cells data available to be read
 */
bool cellular_has_cells_data(void);

/**
 * Read the available cells data
 * @param   cells_info
 *            Structure where the cells data will be copied for.
 */
void cellular_read_cells_data(struct cells_data* cells_info);

/**
 * Read the available cells data without clear flag
 * @param   cells_info
 *            Structure where the cells data will be copied for.
 */
void cellular_read_cells_data_without_clear_flag(struct cells_data* cells_info);

/**
 *  Returns the IMEI of the device
 */
char* cellular_get_imei(void);

/**
 *  Returns the RSSI for the connected cellular antenna
 */
int32_t cellular_get_rssi(uint64_t current_timestamp);

/**
 *  Tells if the cellular module is powered on or not
 */
bool cellular_is_powered_on(void);

/**
 *  Tells to the module to skip the rest of the downloaded information
 * @param   skip_downloaded_data
 *            The new value for _skip_downloaded_data
 */
void cellular_set_skip_downloaded_data(bool skip_downloaded_data,
                                      uint64_t current_timestamp);

bool cellular_has_time_took_to_register_on_network(void);

uint64_t cellular_get_time_took_to_register_on_network(void);

bool cellular_has_register_cell_on_network(void);

SIMCOM_Module_Version cellular_get_simcom_version(void);

void cellular_start_collecting_gps(uint64_t current_timestamp);

void cellular_stop_collecting_gps(uint64_t current_timestamp);

bool cellular_is_collecting_gps_sample(void);

bool cellular_is_collecting_cells_sample(void);

struct gps_info* cellular_get_gps_info(void);

uint64_t cellular_get_gps_rx_start_timestamp(void);

void cellular_set_psm_parameters(
    bool      psm_on, 
    uint8_t   t3324_value,
    uint8_t   t3412_value, 
    uint64_t  current_timestamp);

void cellular_set_edrx_parameters(
    bool      edrx_on, 
    uint8_t   edrx_period_value,
    uint8_t   ptw_period_value, 
    uint64_t  current_timestamp);

bool cellular_has_psm_active(void);

bool cellular_has_edrx_active(void);

bool cellular_is_module_with_data_session_remaining_opened(void);

void cellular_set_psm_configured(bool psm_configured, uint64_t current_timestamp);

uint8_t cellular_get_t3324_value(void);

uint8_t cellular_get_t3412_value(void);

uint8_t cellular_get_edrx_period_value(void);

uint8_t cellular_get_ptw_period_value(void);

bool cellular_is_psm_on(void);

bool cellular_is_edrx_on(void);

bool cellular_support_cells_service(void);

bool cellular_support_gps_service(void);

uint64_t cellular_get_gps_sampling_rate(void);

int64_t cellular_how_many_time_to_close_data_session(uint64_t current_timestamp);

void cellular_print_current_internal_state(uint64_t current_timestamp);

bool gps_is_already_collecting(void);

bool cellular_get_current_communication_status(void);

struct rssi_data* cellular_get_rssi_data(void);

bool cellular_psm_is_currently_active(void);

#endif /* CELLULAR_H */
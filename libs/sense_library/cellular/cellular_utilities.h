/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file cellular_utilities.h
 * @brief This header file has the cellular utilities including network authentication
 * definitions.
 */

#ifndef CELLULAR_UTILITIES_H
#define CELLULAR_UTILITIES_H

/********************************** Includes ***********************************/

/* Standard C library */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Libs */
#include "sense_library/utils/debug.h"

/* Cellular to get module versions */
#include "sense_library/cellular/cellular.h"

/********************************** Definitions ********************************/

#define MCC_POLAND                                    (260)               ///< MCC code.
#define MCC_LUXEMBOURG                                (270)               ///< MCC code.
#define MCC_BRAZIL                                    (724)               ///< MCC code.
#define MCC_PORTUGAL                                  (268)               ///< MCC code.
#define MCC_GERMANY                                   (262)               ///< MCC code.
#define MCC_NETHERLANDS                               (204)               ///< MCC code.
#define MCC_DENMARK                                   (238)               ///< MCC code.
#define MCC_INTERNATIONAL                             (901)               ///< MCC code.
#define MCC_TURKEY                    	              (286)               ///< MCC code.
#define MCC_UK                                        (234)               ///< MCC code.

#define MNC_POLAND_COSWITCHED                         (06)                ///< MNC code.
#define MNC_LUXEMBOURG_JOIN                           (01)                ///< MNC code.
#define MNC_BRAZIL_CLARO                              (5)                 ///< MNC code.
#define MNC_BRAZIL_OI                                 (31)                ///< MNC code.
#define MNC_BRAZIL_TIM_1                              (2)                 ///< MNC code.
#define MNC_BRAZIL_TIM_2                              (3)                 ///< MNC code.
#define MNC_BRAZIL_TIM_3                              (4)                 ///< MNC code.
#define MNC_BRAZIL_VIVO_1                             (1)                 ///< MNC code.
#define MNC_BRAZIL_VIVO_2                             (6)                 ///< MNC code.
#define MNC_BRAZIL_VIVO_3                             (10)                ///< MNC code.
#define MNC_BRAZIL_VIVO_4                             (11)                ///< MNC code.
#define MNC_BRAZIL_VIVO_5                             (19)                ///< MNC code.
#define MNC_BRAZIL_VIVO_6                             (23)                ///< MNC code.
#define MNC_BRAZIL_VODAFONE                           (18)                ///< MNC code.
#define MNC_PORTUGAL_VODAFONE                         (01)                ///< MNC code.
#define MNC_PORTUGAL_MEO                              (06)                ///< MNC code.
#define MNC_PORTUGAL_NOS                              (03)                ///< MNC code.
#define MNC_GERMANY_TELEKOM_1                         (01)                ///< MNC code.
#define MNC_GERMANY_TELEKOM_2                         (06)                ///< MNC code.
#define MNC_GERMANY_TELEKOM_3                         (78)                ///< MNC code.
#define MNC_INTERNATIONAL_TELEKOM                     (67)                ///< MNC code.
#define MNC_INTERNATIONAL_VODAFONE                    (28)                ///< MNC code.
#define MNC_INTERNATIONAL_1NCE                        (40)                ///< MNC code.
#define MNC_INTERNATIONAL_NETHERLANDS_TMOBILE         (16)                ///< MNC code.
#define MNC_TURKEY_VODAFONE                           (02)                ///< MNC code.
#define MNC_NETHERLANDS_VODAFONE                      (04)                ///< MNC code.
#define MNC_DENMARK_TELENOR                           (02)                ///< MNC code.
#define MNC_UK_RWG                                    (33)                ///< MNC code.
#define MNC_UK_JT                                     (50)                ///< MNC code. This is from Hologram Jersey Telecom. 
#define MNC_INTERNATIONAL_RWG                         (MNC_INTERNATIONAL_VODAFONE)  ///< MNC code.

#define APN_OPENROAMER                                "openroamer.com"    ///< APN string.
#define APN_JOIN                                      "joinm2m"           ///< APN string.
#define APN_BRAZIL_CLARO                              "claro.com.br"      ///< APN string.
#define APN_BRAZIL_OI                                 "gprs.oi.com.br"    ///< APN string.
#define APN_BRAZIL_TIM                                "tim.brasil.br"     ///< APN string.
#define APN_BRAZIL_VIVO                               "zap.vivo.com.br"   ///< APN string.
#define APN_BRAZIL_VODAFONE                           "vodafone.br"       ///< APN string.
#define APN_PORTUGAL_VODAFONE(simcom_version, psm)    (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? ((psm == 1) ? ("nbiot2.vodafone.pt") : ("m2m.vodafone.pt")) : ("net2.vodafone.pt"))    ///< APN string.
#define APN_PORTUGAL_MEO                              "internetm2m"       ///< APN string.
#define APN_PORTUGAL_NOS                              "internet"          ///< APN string.
#define APN_NETHERLANDS_VODAFONE                      "m2m.vodafone.pt"   ///< APN string.
#define APN_INTERNATIONAL_NETHERLANDS_TMOBILE         "cdp.iot.t-mobile.nl" ///< APN string.
#define APN_INTERNATIONAL_RWG                         "lpwa.pelion"       ///< APN string.
#define APN_DENMARK_TELENOR                           "telenor.iot"       ///< APN string.
#define APN_GERMANY_TELEKOM(simcom_version)           (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? ("internet.nbiot.telekom.de") : ("internet.m2mportal.de"))    ///< APN string.
#define APN_INTERNATIONAL_VODAFONE                    "m2m.vodafone.pt"   ///< APN string.
#define APN_INTERNATIONAL_1NCE                        "iot.1nce.net"      ///< APN string.
#define APN_TURKEY_VODAFONE                           "internet"          ///< APN string.
#define APN_UK_RWG                                    "everywhere"        ///< APN string.
#define APN_UK_JT                                     "hologram"          ///< APN string. This is from Hologram Jersey Telecom. 

#define PASS_BRAZIL_CLARO                             "claro"             ///< Password.
#define PASS_BRAZIL_OI                                "oi"                ///< Password.
#define PASS_BRAZIL_TIM                               "tim"               ///< Password.
#define PASS_BRAZIL_VIVO                              "vivo"              ///< Password.
#define PASS_BRAZIL_VODAFONE                          ""                  ///< Password.
#define PASS_PORTUGAL_VODAFONE(simcom_version)        (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? ("") : ("vodafone"))   ///< Password.
#define PASS_PORTUGAL_MEO                             ""                  ///< Password.
#define PASS_PORTUGAL_NOS                             ""                  ///< Password.
#define PASS_GERMANY_TELEKOM                          "sim"               ///< Password.
#define PASS_NETHERLANDS_VODAFONE                     "vodafone"          ///< Password.
#define PASS_INTERNATIONAL_RWG                        "streamip"          ///< Password.
#define PASS_UK_RWG                                   "secure"            ///< Password.
#define PASS_UK_JT                                    ""                  ///< Password. This is from Hologram Jersey Telecom. 

#define USER_BRAZIL_CLARO                             "claro"             ///< User.
#define USER_BRAZIL_OI                                "oi"                ///< User.
#define USER_BRAZIL_TIM                               "tim"               ///< User.
#define USER_BRAZIL_VIVO                              "vivo"              ///< User.
#define USER_BRAZIL_VODAFONE                          ""                  ///< User.
#define USER_PORTUGAL_VODAFONE(simcom_version)        (((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) ? ("") : ("vodafone"))   ///< User.
#define USER_PORTUGAL_MEO                             ""                  ///< User.
#define USER_PORTUGAL_NOS                             ""                  ///< User.
#define USER_GERMANY_TELEKOM                          "m2m"               ///< User.
#define USER_NETHERLANDS_VODAFONE                     "vodafone"          ///< User.
#define USER_INTERNATIONAL_RWG                        "streamip"          ///< Password.
#define USER_UK_RWG                                   "eesecure"          ///< User.
#define USER_UK_JT                                     ""                 ///< User. This is from Hologram Jersey Telecom. 

#define BAND_PORTUGAL_VODAFONE                        "20"                ///< Band selection.
#define BAND_PORTUGAL_MEO                             "20"                ///< Band selection.
#define BAND_GERMANY_TELEKOM                          "8"                 ///< Band selection.
#define BAND_DENMARK_TELENOR                          "20"                ///< Band selection.
#define BAND_INTERNATIONAL_NETHERLANDS_TMOBILE        "8"                 ///< Band selection.
#define BAND_MANUAL_SELECTION_DEFAULT                 "1,3,5,8,20,28"     ///< Band selection.

/********************************** Prototypes *********************************/

char* cellular_utilities_get_apn(
    uint16_t  mobile_country_code,
    uint8_t   mobile_network_code, 
    uint8_t   simcom_version, 
    bool      psm_active);

bool cellular_utilities_auth_needed(
    uint16_t  mobile_country_code,
    uint8_t   mobile_network_code, 
    uint8_t   simcom_version);

char* cellular_utilities_get_user(
    uint16_t  mobile_country_code,
    uint8_t   mobile_network_code, 
    uint8_t   simcom_version);

char* cellular_utilities_get_pass(
    uint16_t  mobile_country_code,
    uint8_t   mobile_network_code, 
    uint8_t   simcom_version);

void cellular_utilities_process_cells(
    char*     response, 
    uint8_t*  cells_type,
    uint8_t*  cells_number, 
    uint16_t* mnc, 
    uint16_t* mcc, 
    uint16_t* lac,
    uint32_t* ids, 
    uint8_t*  rx_levels, 
    uint8_t   max_cells,
    uint8_t   simcom_version, 
    uint16_t  lte_mcc, 
    uint8_t   lte_mnc);

uint8_t cellular_utilities_number_of_chars_until_cr(char* str);

char* cellular_utilities_get_band(
    uint16_t  mobile_country_code,
    uint8_t   mobile_network_code, 
    uint8_t   simcom_version);

bool cellular_utilities_has_psm_mode(
    uint16_t  mobile_country_code,
    uint8_t   mobile_network_code, 
    uint8_t   simcom_version);

int32_t cellular_utilities_get_rxl_from_dbm(int32_t dbm);

int32_t cellular_utilities_get_rssi_from_dbm(int32_t dbm);

int32_t cellular_utilities_get_dbm_from_rssi(int32_t rssi);

int32_t cellular_utilities_get_dbm_from_rxl(int32_t rxl);

int32_t cellular_utilities_get_rssi_from_rxl(int32_t rxl);

int32_t cellular_utilities_get_rxl_from_rssi(int32_t rssi);

#endif /* CELLULAR_UTILITIES_H */

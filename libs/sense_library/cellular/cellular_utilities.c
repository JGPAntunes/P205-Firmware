/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file cellular_utilities.c
 * @brief This file has the cellular utilities including network authentication
 * definitions.
 */

/********************************** Includes ***********************************/

/* Interface */
#include "cellular_utilities.h"

/* Utils */
#include "sense_library/utils/utils.h"

/********************************** Private ************************************/



/********************************** Public *************************************/

/**
 * Get APN string according with MCC and MNC.
 * @param[in] mobile_country_code
 * @param[in] mobile_network_code
 * @param[in] simcom_version
 * @param[in] psm_active
 * @return APN.
 */
char *cellular_utilities_get_apn(uint16_t mobile_country_code, uint8_t mobile_network_code, 
    uint8_t simcom_version, bool psm_active) {

  debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

  switch (mobile_country_code) {
    case MCC_POLAND: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] poland mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_POLAND_COSWITCHED: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] coswitched mnc\n");
          return APN_OPENROAMER;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return APN_OPENROAMER;
        }
      }
    }
    case MCC_LUXEMBOURG: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] luxembourg mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_LUXEMBOURG_JOIN: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] join mnc\n");
          return APN_JOIN;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return APN_JOIN;
        }
      }
    }
    case MCC_BRAZIL: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] brazil mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_BRAZIL_CLARO: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] claro mnc\n");
          return APN_BRAZIL_CLARO;
        }
        case MNC_BRAZIL_OI: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] oi mnc\n");
          return APN_BRAZIL_OI;
        }
        case MNC_BRAZIL_TIM_1: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] tim mnc\n");
          return APN_BRAZIL_TIM;
        }
        case MNC_BRAZIL_TIM_2: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] tim mnc\n");
          return APN_BRAZIL_TIM;
        }
        case MNC_BRAZIL_TIM_3: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] tim mnc\n");
          return APN_BRAZIL_TIM;
        }
        case MNC_BRAZIL_VIVO_1: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] vivo mnc\n");
          return APN_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_2: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] vivo mnc\n");
          return APN_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_3: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] vivo mnc\n");
          return APN_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_4: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] vivo mnc\n");
          return APN_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_5: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] vivo mnc\n");
          return APN_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_6: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] vivo mnc\n");
          return APN_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] vodafone mnc\n");
          return APN_BRAZIL_VODAFONE;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return APN_BRAZIL_VIVO;
        }
      }
    }
    case MCC_PORTUGAL: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] portugal mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_PORTUGAL_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] vodafone mnc\n");
          return APN_PORTUGAL_VODAFONE(simcom_version, (uint8_t)psm_active);
        }
        case MNC_PORTUGAL_MEO: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] meo mnc\n");
          return APN_PORTUGAL_MEO;
        }
        case MNC_PORTUGAL_NOS: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] nos mnc\n");
          return APN_PORTUGAL_NOS;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return APN_PORTUGAL_MEO;
        }
      }
    }
    case MCC_GERMANY: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] germany mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_GERMANY_TELEKOM_1: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] telekom mnc\n");
          return APN_GERMANY_TELEKOM(simcom_version);
        }
        case MNC_GERMANY_TELEKOM_2: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] telekom mnc\n");
          return APN_GERMANY_TELEKOM(simcom_version);
        }
        case MNC_GERMANY_TELEKOM_3: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] telekom mnc\n");
          return APN_GERMANY_TELEKOM(simcom_version);
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return APN_GERMANY_TELEKOM(simcom_version);
        }
      }
    }
    case MCC_INTERNATIONAL: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] international mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_INTERNATIONAL_TELEKOM: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] telekom mnc\n");
          return APN_GERMANY_TELEKOM(simcom_version);
        }
        //  case MNC_INTERNATIONAL_VODAFONE: {
        //    debug_print_string(DEBUG_LEVEL_2,
        //        (uint8_t*) "[cellular_utilities_get_apn] vodafone mnc\n");
        //    return APN_INTERNATIONAL_VODAFONE;
        //  }
        case MNC_INTERNATIONAL_1NCE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] 1nce mnc\n");
          return APN_INTERNATIONAL_1NCE;
        }
        case MNC_INTERNATIONAL_NETHERLANDS_TMOBILE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] t-mobile mnc\n");
          return APN_INTERNATIONAL_NETHERLANDS_TMOBILE;
        }
        case MNC_INTERNATIONAL_RWG: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] rwg mnc\n");
          return APN_INTERNATIONAL_RWG;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return APN_GERMANY_TELEKOM(simcom_version);
        }
      }
    }
    case MCC_TURKEY: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] turkey mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_TURKEY_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] vodafone mnc\n");
          return APN_TURKEY_VODAFONE;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return APN_TURKEY_VODAFONE;
        }
      }
    }
    case MCC_NETHERLANDS: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] netherlands mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_NETHERLANDS_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] vodafone mnc\n");
          return APN_NETHERLANDS_VODAFONE;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return APN_NETHERLANDS_VODAFONE;
        }
      }
    }
    case MCC_DENMARK: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] denmark mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_DENMARK_TELENOR: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] telenor mnc\n");
          return APN_DENMARK_TELENOR;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return APN_DENMARK_TELENOR;
        }
      }
    }
    case MCC_UK: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] uk mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_UK_RWG: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] rwg mnc\n");
          return APN_UK_RWG;
        }
        case MNC_UK_JT: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] hologram mnc\n");
          return APN_UK_JT;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return APN_UK_RWG;
        }
      }
    }
    default: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_apn] default mcc\n");
      return APN_JOIN;
    }
  }
}

/**
 * Tells if the current SIMCard requires authentication.
 * @param[in] mobile_country_code
 * @param[in] mobile_network_code
 * @param[in] simcom_version
 * @return True if authentication is needed, false otherwise. 
 */
bool cellular_utilities_auth_needed(uint16_t mobile_country_code,
    uint8_t mobile_network_code, uint8_t simcom_version) {

  (void)simcom_version;

  debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

  switch (mobile_country_code) {
    case MCC_POLAND: {
      switch (mobile_network_code) {
        case MNC_POLAND_COSWITCHED: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
      }
    }
    case MCC_LUXEMBOURG: {
      switch (mobile_network_code) {
        case MNC_LUXEMBOURG_JOIN: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
      }
    }
    case MCC_BRAZIL: {
      switch (mobile_network_code) {
        case MNC_BRAZIL_CLARO: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_OI: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_TIM_1: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_TIM_2: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_TIM_3: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_VIVO_1: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_VIVO_2: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_VIVO_3: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_VIVO_4: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_VIVO_5: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_VIVO_6: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_BRAZIL_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
      }
    }
    case MCC_PORTUGAL: {
      switch (mobile_network_code) {
        case MNC_PORTUGAL_VODAFONE: {
          if ((simcom_version == SIMCOM_7020_VERSION) || (simcom_version == SIMCOM_7080_VERSION)) {
            debug_print_string(DEBUG_LEVEL_2,
                (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
            return false;
          } else {
            debug_print_string(DEBUG_LEVEL_2,
                (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
            return true;
          }
        }
        case MNC_PORTUGAL_MEO: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
        case MNC_PORTUGAL_NOS: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
      }
    }
    case MCC_GERMANY: {
      switch (mobile_network_code) {
        case MNC_GERMANY_TELEKOM_1: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_GERMANY_TELEKOM_2: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        case MNC_GERMANY_TELEKOM_3: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
      }
    }
    case MCC_INTERNATIONAL: {
      switch (mobile_network_code) {
        case MNC_INTERNATIONAL_TELEKOM: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        //  case MNC_INTERNATIONAL_VODAFONE: {
        //    debug_print_string(DEBUG_LEVEL_2,
        //        (uint8_t*) "[cellular_utilities_auth_needed] need auth\n");
        //    return true;
        //  }
        case MNC_INTERNATIONAL_1NCE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
        case MNC_INTERNATIONAL_NETHERLANDS_TMOBILE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] dont' need auth\n");
          return false;
        }
        case MNC_INTERNATIONAL_RWG: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
      }
    }
    case MCC_TURKEY: {
      switch (mobile_network_code) {
        case MNC_TURKEY_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
      }
    }
    case MCC_NETHERLANDS: {
      switch (mobile_network_code) {
        case MNC_NETHERLANDS_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] need auth\n");
          return true;
        }
      }
    }
    case MCC_DENMARK: {
      switch (mobile_network_code) {
        case MNC_DENMARK_TELENOR: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return false;
        }
      }
    }
    case MCC_UK: {
      switch (mobile_network_code) {
        case MNC_UK_RWG: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return true;
        }
        case MNC_UK_JT: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return true;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
          return true;
        }
      }
    }
    default: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_auth_needed] don't need auth\n");
      return false;
    }
  }
}

/**
 * Get user parameter if the current SIMCard requires authentication.
 * @param[in] mobile_country_code
 * @param[in] mobile_network_code
 * @param[in] simcom_version
 * @return User.
 */
char *cellular_utilities_get_user(uint16_t mobile_country_code,
    uint8_t mobile_network_code, uint8_t   simcom_version) {

  switch (mobile_country_code) {
    case MCC_POLAND: {
      switch (mobile_network_code) {
        case MNC_POLAND_COSWITCHED: {
          return "";
        }
        default: {
          return "";
        }
      }
    }
    case MCC_LUXEMBOURG: {
      switch (mobile_network_code) {
        case MNC_LUXEMBOURG_JOIN: {
          return "";
        }
        default: {
          return "";
        }
      }
    }
    case MCC_BRAZIL: {
      switch (mobile_network_code) {
        case MNC_BRAZIL_CLARO: {
          return USER_BRAZIL_CLARO;
        }
        case MNC_BRAZIL_OI: {
          return USER_BRAZIL_OI;
        }
        case MNC_BRAZIL_TIM_1: {
          return USER_BRAZIL_TIM;
        }
        case MNC_BRAZIL_TIM_2: {
          return USER_BRAZIL_TIM;
        }
        case MNC_BRAZIL_TIM_3: {
          return USER_BRAZIL_TIM;
        }
        case MNC_BRAZIL_VIVO_1: {
          return USER_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_2: {
          return USER_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_3: {
          return USER_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_4: {
          return USER_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_5: {
          return USER_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_6: {
          return USER_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VODAFONE: {
          return USER_BRAZIL_VODAFONE;
        }
        default: {
          return USER_BRAZIL_VIVO;
        }
      }
    }
    case MCC_PORTUGAL: {
      switch (mobile_network_code) {
        case MNC_PORTUGAL_VODAFONE:
          return USER_PORTUGAL_VODAFONE(simcom_version);
        case MNC_PORTUGAL_MEO:
          return USER_PORTUGAL_MEO;
        case MNC_PORTUGAL_NOS:
          return USER_PORTUGAL_NOS;
        default:
          return USER_PORTUGAL_MEO;
        }
    }
    case MCC_GERMANY: {
      switch (mobile_network_code) {
        case MNC_GERMANY_TELEKOM_1:
          return USER_GERMANY_TELEKOM;
        case MNC_GERMANY_TELEKOM_2:
          return USER_GERMANY_TELEKOM;
        case MNC_GERMANY_TELEKOM_3:
          return USER_GERMANY_TELEKOM;
        default:
          return USER_GERMANY_TELEKOM;
        }
    }
    case MCC_INTERNATIONAL: {
      switch (mobile_network_code) {
        case MNC_INTERNATIONAL_TELEKOM:
          return USER_GERMANY_TELEKOM;
        //  case MNC_INTERNATIONAL_VODAFONE:
        //    return USER_NETHERLANDS_VODAFONE;
        case MNC_INTERNATIONAL_RWG:
          return USER_INTERNATIONAL_RWG;
        default:
          return USER_GERMANY_TELEKOM;
        }
    }
    case MCC_NETHERLANDS: {
      switch (mobile_network_code) {
        case MNC_NETHERLANDS_VODAFONE: {
          return USER_NETHERLANDS_VODAFONE;
        }
        default: {
          return USER_NETHERLANDS_VODAFONE;
        }
      }
    }
    case MCC_UK: {
      switch (mobile_network_code) {
        case MNC_UK_RWG: {
          return USER_UK_RWG;
        }
        case MNC_UK_JT: {
          return USER_UK_JT;
        }
        default: {
          return USER_UK_RWG;
        }
      }
    }
    default: {
      return "";
    }
  }
}

/**
 * Get password parameter if the current SIMCard requires authentication.
 * @param[in] mobile_country_code
 * @param[in] mobile_network_code
 * @param[in] simcom_version
 * @return Password.
 */
char *cellular_utilities_get_pass(uint16_t mobile_country_code,
    uint8_t mobile_network_code, uint8_t simcom_version) {

  switch (mobile_country_code) {
    case MCC_POLAND: {
      switch (mobile_network_code) {
        case MNC_POLAND_COSWITCHED: {
          return "";
        }
        default: {
          return "";
        }
      }
    }
    case MCC_LUXEMBOURG: {
      switch (mobile_network_code) {
        case MNC_LUXEMBOURG_JOIN: {
          return "";
        }
        default: {
          return "";
        }
      }
    }
    case MCC_BRAZIL: {
      switch (mobile_network_code) {
        case MNC_BRAZIL_CLARO: {
          return PASS_BRAZIL_CLARO;
        }
        case MNC_BRAZIL_OI: {
          return PASS_BRAZIL_OI;
        }
        case MNC_BRAZIL_TIM_1: {
          return PASS_BRAZIL_TIM;
        }
        case MNC_BRAZIL_TIM_2: {
          return PASS_BRAZIL_TIM;
        }
        case MNC_BRAZIL_TIM_3: {
          return PASS_BRAZIL_TIM;
        }
        case MNC_BRAZIL_VIVO_1: {
          return PASS_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_2: {
          return PASS_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_3: {
          return PASS_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_4: {
          return PASS_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_5: {
          return PASS_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VIVO_6: {
          return PASS_BRAZIL_VIVO;
        }
        case MNC_BRAZIL_VODAFONE: {
          return PASS_BRAZIL_VODAFONE;
        }
        default: {
          return PASS_BRAZIL_VIVO;
        }
      }
    }
    case MCC_PORTUGAL: {
      switch (mobile_network_code) {
        case MNC_PORTUGAL_VODAFONE: {
          return PASS_PORTUGAL_VODAFONE(simcom_version);
        }
        case MNC_PORTUGAL_MEO: {
          return PASS_PORTUGAL_MEO;
        }
        case MNC_PORTUGAL_NOS: {
          return PASS_PORTUGAL_NOS;
        }
        default: {
          return PASS_PORTUGAL_MEO;
        }
      }
    }
    case MCC_GERMANY: {
      switch (mobile_network_code) {
        case MNC_GERMANY_TELEKOM_1: {
          return PASS_GERMANY_TELEKOM;
        }
        case MNC_GERMANY_TELEKOM_2: {
          return PASS_GERMANY_TELEKOM;
        }
        case MNC_GERMANY_TELEKOM_3: {
          return PASS_GERMANY_TELEKOM;
        }
        default: {
          return PASS_GERMANY_TELEKOM;
        }
      }
    }
    case MCC_INTERNATIONAL: {
      switch (mobile_network_code) {
        case MNC_INTERNATIONAL_TELEKOM: {
          return PASS_GERMANY_TELEKOM;
        }
        //  case MNC_INTERNATIONAL_VODAFONE: {
        //    return PASS_NETHERLANDS_VODAFONE;
        //  }
        case MNC_INTERNATIONAL_RWG: {
          return PASS_INTERNATIONAL_RWG;
        }
        default: {
          return PASS_GERMANY_TELEKOM;
        }
      }
    }
    case MCC_NETHERLANDS: {
      switch (mobile_network_code) {
        case MNC_NETHERLANDS_VODAFONE: {
          return PASS_NETHERLANDS_VODAFONE;
        }
        default: {
          return PASS_NETHERLANDS_VODAFONE;
        }
      }
    }
    case MCC_UK: {
      switch (mobile_network_code) {
        case MNC_UK_RWG: {
          return PASS_UK_RWG;
        }
        case MNC_UK_JT: {
          return PASS_UK_JT;
        }
        default: {
          return PASS_UK_RWG;
        }
      }
    }
    default: {
      return "";
    }
  }
}

/**
 * This utility processes the received cells information processing each cell
 * (i.e. each line) one by one, storing each cell's parameter in the data
 * structure.
 * @param[in] response String to be processed.
 * @param[in] cells_type Tells about cells type. If it is GSM, LTE, etc.
 * @param[in] cells_number Stores the total cells number.
 * @param[in] mnc Stores each cell MNC.
 * @param[in] mcc Stores each cell MCC.
 * @param[in] lac Stores each cell LAC.
 * @param[in] ids Stores each cell ID.
 * @param[in] rx_levels Stores each cell received level.
 * @param[in] max_cells Tells how many cells we can store.
 * @param[in] simcom_version
 * @param[in] lte_mcc Stores each LTE cell MCC.
 * @param[in] lte_mnc Stores each LTE cell MNC.
 */
void cellular_utilities_process_cells(
    char *response, uint8_t *cells_type, uint8_t *cells_number, uint16_t *mnc, 
    uint16_t *mcc, uint16_t *lac, uint32_t *ids, uint8_t *rx_levels, uint8_t max_cells, 
    uint8_t simcom_version, uint16_t lte_mcc, uint8_t lte_mnc) {

  *cells_number = 0;

  if (simcom_version == SIMCOM_800_VERSION) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1,
        (uint8_t *)"[cellular_utilities_process_cells] SIM800 module\n");

    *cells_type = GAMA_POSITION_CELLS_TYPE;

    char *p = response;

    do {
      /* Precaution */
      p = strstr(p, "+CENG:");
      if (p == NULL) {
        break;
      }

      p = strchr(p, ' ');
      if (p == NULL) {
        break;
      }
      p = p + 1;
      uint8_t position = utils_atoll(p);

      if (position == 0) {
        *cells_number = 0;

        // I just want to record the mnc and the mcc for the first cell
        p = strchr(p, '"') + 1;
        *mcc = utils_atoll(p);

        p = strchr(p, ',') + 1;
        *mnc = utils_atoll(p);
      } else {
        //Skip the mnc and the mcc for the other cells!
        p = strchr(p, '"') + 1;
        p = strchr(p, ',') + 1;
      }

      p = strchr(p, ',') + 1;
      lac[*cells_number] = strtoul(p, NULL, 16);
      //utils_atoll(p);

      p = strchr(p, ',') + 1;
      ids[*cells_number] = strtoul(p, NULL, 16);
      //utils_atoll(p);

      p = strchr(p, ',') + 1;
      p = strchr(p, ',') + 1;
      //p = strchr(p, ',') + 1;
      rx_levels[*cells_number] = utils_atoll(p);

      /* Invalid ID = no more cells */
      if ((ids[*cells_number] == 0) || (ids[*cells_number] == 0xFFFF))
        break;
      else
        *cells_number = *cells_number + 1;
    } while (*cells_number < max_cells);
  }
  if (simcom_version == SIMCOM_868_VERSION) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1,
        (uint8_t *)"[cellular_utilities_process_cells] SIMCOM868 module\n");

    *cells_type = GAMA_POSITION_CELLS_TYPE;

    char *p = response;

    /* Skip initial "+CENG" */
    p = strstr(p, "+CENG:") + 1;

    do {
      /* Precaution */
      p = strstr(p, "+CENG:");
      if (p == NULL) {
        break;
      }

      p = strchr(p, ' ');
      if (p == NULL) {
        break;
      }
      p = p + 1;
      uint8_t position = utils_atoll(p);

      if (position == 0) {
        *cells_number = 0;

        //I just want to record the mnc and the mcc for the first cell
        p = strchr(p, '"') + 1;
        *mcc = utils_atoll(p);

        p = strchr(p, ',') + 1;
        *mnc = utils_atoll(p);
      } else {
        //Skip the mnc and the mcc for the other cells!
        p = strchr(p, '"') + 1;
        p = strchr(p, ',') + 1;
      }

      p = strchr(p, ',') + 1;
      lac[*cells_number] = strtoul(p, NULL, 16);
      //utils_atoll(p);

      p = strchr(p, ',') + 1;
      ids[*cells_number] = strtoul(p, NULL, 16);
      //utils_atoll(p);

      p = strchr(p, ',') + 1;
      p = strchr(p, ',') + 1;
      //p = strchr(p, ',') + 1;
      rx_levels[*cells_number] = utils_atoll(p);

      /* Invalid ID = no more cells */
      if ((ids[*cells_number] == 0) || (ids[*cells_number] == 0xFFFF))
        break;
      else
        *cells_number = *cells_number + 1;
    } while (*cells_number < max_cells);
  } else if (simcom_version == SIMCOM_7020_VERSION) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1,
        (uint8_t *)"[cellular_utilities_process_cells] SIM7020 module\n");

    *cells_type = GAMA_POSITION_CELLS_V2_TYPE;

    char *p = response;

    do {
      /* Precaution */
      p = strstr(p, "+CENG:");
      if (p == NULL) {
        break;
      }

      p = strchr(p, ' ');
      if (p == NULL) {
        break;
      }
      p = p + 1;

      if (*cells_number == 0) {
        mnc[*cells_number] = lte_mnc;
        mcc[*cells_number] = lte_mcc;
      }

      /* Locate cell ID */
      p = strchr(p, ',') + 1;
      p = strchr(p, ',') + 1;
      p = strchr(p, ',') + 2;

      /* Convert to a separate string */
      char hex_string[20];
      memset(hex_string, '\0', 20);
      hex_string[0] = '0';
      hex_string[1] = 'x';
      for (uint8_t i = 0; p[i] != '"'; i++) {
        hex_string[2 + i] = p[i];
      }
      ids[*cells_number] = strtol(hex_string, NULL, 0);

      /* Locate RSRP */
      p = strchr(p, ',') + 1;
      rx_levels[*cells_number] = cellular_utilities_get_rxl_from_dbm(atoi(p));

      /* Locate TAC */
      p = strchr(p, ',') + 1;
      p = strchr(p, ',') + 1;
      p = strchr(p, ',') + 1;
      p = strchr(p, ',') + 1;
      p = strchr(p, ',') + 2;

      /* Convert to a separate string */
      memset(hex_string, '\0', 20);
      hex_string[0] = '0';
      hex_string[1] = 'x';
      for (uint8_t i = 0; p[i] != '"'; i++) {
        hex_string[2 + i] = p[i];
      }
      lac[*cells_number] = strtol(hex_string, NULL, 0);

      /* Invalid ID = no more cells */
      if ((ids[*cells_number] == 0) || (ids[*cells_number] == 0xFFFF))
        break;
      else
        *cells_number = *cells_number + 1;
    } while (*cells_number < max_cells);
  } else if (simcom_version == SIMCOM_7080_VERSION) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1,
        (uint8_t *)"[cellular_utilities_process_cells] SIM7080 module\n");

    *cells_type = GAMA_POSITION_CELLS_V2_TYPE;

    char *p = response;
    bool gsm_profile = false;
    bool gsm_profile_detected = false;
    bool lte_profile_detected = false;

    do {
      /* Precaution */
      p = strstr(p, "+CENG:");
      if (p == NULL) {
        break;
      }

      /* Check if we have a GSM or LTE cells profile */
      char *aux_1 = strstr(p, "GSM");
      if (aux_1 != NULL) {
        gsm_profile_detected = true;
      }
      char *aux_2 = strstr(p, "LTE ");
      if (aux_2 != NULL) {
        lte_profile_detected = true;
      }

      p = strchr(p, '"');
      if (p == NULL) {
        break;
      }
      p = p + 1;

      /* Change profile? */
      if (gsm_profile_detected) {
        if (aux_1 <= p) {
          gsm_profile = true;

          debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
          debug_print_string(DEBUG_LEVEL_1,
              (uint8_t *)"[cellular_utilities_process_cells] gsm profile detected\n");
        }
      }
      if (lte_profile_detected) {
        if (aux_2 <= p) {
          gsm_profile = false;

          debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
          debug_print_string(DEBUG_LEVEL_1,
              (uint8_t *)"[cellular_utilities_process_cells] lte profile detected\n");
        }
      }

      if (*cells_number == 0) {
        mnc[*cells_number] = lte_mnc;
        mcc[*cells_number] = lte_mcc;
      }

      if (gsm_profile) {
        /* Locate Rx level */
        p = strchr(p, ',') + 1;
        rx_levels[*cells_number] = utils_atoll(p);

        /* Locate cell ID */
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;

        /* Convert to a separate string */
        char hex_string[20];
        memset(hex_string, '\0', 20);
        hex_string[0] = '0';
        hex_string[1] = 'x';
        for (uint8_t i = 0; p[i] != '"'; i++) {
          hex_string[2 + i] = p[i];
        }
        ids[*cells_number] = strtol(hex_string, NULL, 0);

        /* Locate TAC */
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;

        /* Convert to a separate string */
        memset(hex_string, '\0', 20);
        hex_string[0] = '0';
        hex_string[1] = 'x';
        for (uint8_t i = 0; p[i] != '"'; i++) {
          hex_string[2 + i] = p[i];
        }
        lac[*cells_number] = strtol(hex_string, NULL, 0);
      } else {
        /* Locate RSRP */
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        rx_levels[*cells_number] = cellular_utilities_get_rxl_from_dbm(atoi(p));

        /* Locate TAC */
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        lac[*cells_number] = atoi(p);

        /* Locate cell ID */
        p = strchr(p, ',') + 1;

        /* Convert to a separate string */
        char hex_string[20];
        memset(hex_string, '\0', 20);
        hex_string[0] = '0';
        hex_string[1] = 'x';
        for (uint8_t i = 0; p[i] != '"'; i++) {
          hex_string[2 + i] = p[i];
        }
        ids[*cells_number] = strtol(hex_string, NULL, 0);
      }

      /* Invalid ID = no more cells */
      if ((ids[*cells_number] == 0) || (ids[*cells_number] == 0xFFFF))
        break;
      else
        *cells_number = *cells_number + 1;
    } while (*cells_number < max_cells);
  } else if (simcom_version == SIMCOM_7600_VERSION) {
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1,
        (uint8_t *)"[cellular_utilities_process_cells] SIM7600 module\n");

    char *p = response;

    do {
      /* Precaution */
      p = strstr(p, "+CPSI:");
      if (p == NULL) {
        break;
      }

      p = strchr(p, ' ');
      if (p == NULL) {
        break;
      }
      p = p + 1;

      /* LTE cell tower */
      if ((p[0] == 'L') && (p[1] == 'T') && (p[2] == 'E')) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_1,
            (uint8_t *)"[cellular_utilities_process_cells] LTE cell tower format\n");

        *cells_type = GAMA_POSITION_CELLS_V2_TYPE;

        /* Locate MCC */
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;

        if (*cells_number == 0) {
          mcc[*cells_number] = utils_atoll(p);
        }

        /* Locate MNC */
        p = strchr(p, '-') + 1;

        if (*cells_number == 0) {
          mnc[*cells_number] = utils_atoll(p);
        }

        /* Locate TAC */
        p = strchr(p, ',') + 3;

        /* Convert to a separate string */
        char hex_string[20];
        memset(hex_string, '\0', 20);
        hex_string[0] = '0';
        hex_string[1] = 'x';
        for (uint8_t i = 0; p[i] != ','; i++) {
          hex_string[2 + i] = p[i];
        }
        lac[*cells_number] = strtol(hex_string, NULL, 0);

        /* Locate cell ID */
        p = strchr(p, ',') + 1;
        ids[*cells_number] = utils_atoll(p);

        /* Locate RSRP */
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;

        /* RSRP needs to be /10 because value is in -1/10 dBm */
        rx_levels[*cells_number] = cellular_utilities_get_rxl_from_dbm(
            (atoi(p) / 10));
      } else if ((p[0] == 'G') && (p[1] == 'S') && (p[2] == 'M')) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
        debug_print_string(DEBUG_LEVEL_1,
            (uint8_t *)"[cellular_utilities_process_cells] GSM cell tower format\n");

        *cells_type = GAMA_POSITION_CELLS_TYPE;

        /* Locate MCC */
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;

        if (*cells_number == 0) {
          mcc[*cells_number] = utils_atoll(p);
        }

        /* Locate MNC */
        p = strchr(p, '-') + 1;

        if (*cells_number == 0) {
          mnc[*cells_number] = utils_atoll(p);
        }

        /* Locate LAC */
        p = strchr(p, ',') + 3;

        /* Convert to a separate string */
        char hex_string[20];
        memset(hex_string, '\0', 20);
        hex_string[0] = '0';
        hex_string[1] = 'x';
        for (uint8_t i = 0; p[i] != ','; i++) {
          hex_string[2 + i] = p[i];
        }
        lac[*cells_number] = strtol(hex_string, NULL, 0);

        /* Locate cell ID */
        p = strchr(p, ',') + 1;
        ids[*cells_number] = utils_atoll(p);

        /* Locate RSRP */
        p = strchr(p, ',') + 1;
        p = strchr(p, ',') + 1;
        rx_levels[*cells_number] = cellular_utilities_get_rxl_from_dbm(
            atoi(p));
      }

      /* Invalid ID = no more cells */
      if ((ids[*cells_number] == 0) || (ids[*cells_number] == 0xFFFF))
        break;
      else
        *cells_number = *cells_number + 1;
    } while (*cells_number < max_cells);
  }

  /* Print results */
  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_1,
      (uint8_t *)"[cellular_utilities_process_cells] cells type: ");
  char char_aux[10];
  memset(char_aux, '\0', 10);
  utils_itoa(*cells_type, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"; cells processed: ");

  memset(char_aux, '\0', 10);
  utils_itoa(*cells_number, char_aux, 10);

  if (*cells_number == 0) {
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\n");
    return;
  }

  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"; MCC: ");

  memset(char_aux, '\0', 10);
  utils_itoa(*mcc, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"; MNC: ");

  memset(char_aux, '\0', 10);
  utils_itoa(*mnc, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\n");

  for (uint8_t j = 0; j < *cells_number; j++) {
    memset(char_aux, '\0', 10);
    utils_itoa(j, char_aux, 10);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"[");
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)char_aux);
    if (simcom_version == SIMCOM_800_VERSION) {
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"] LAC: ");
    } else {
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"] TAC: ");
    }

    memset(char_aux, '\0', 10);
    utils_itoa(lac[j], char_aux, 10);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)char_aux);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"; cell ID: ");

    memset(char_aux, '\0', 10);
    utils_itoa(ids[j], char_aux, 10);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)char_aux);
    if (simcom_version == SIMCOM_800_VERSION) {
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"; RSSI: ");
    } else {
      debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"; RSRP: ");
    }

    memset(char_aux, '\0', 10);
    utils_itoa(rx_levels[j], char_aux, 10);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)char_aux);
    debug_print_string(DEBUG_LEVEL_1, (uint8_t *)"\n");
  }
}

/**
 * Tells the number of chars until carriage return char.
 * @param[in] str String to be processed.
 * Number of chars
 */
uint8_t cellular_utilities_number_of_chars_until_cr(char *str) {
  uint8_t number_of_chars = 0;
  uint16_t i;
  for (i = 0; str[i] != '\r' && i < strlen(str); i++) {
    number_of_chars++;
  }
  return number_of_chars;
}

/**
 * Get the cellular band according with SIMcard profile.
 * @param[in] mobile_country_code
 * @param[in] mobile_network_code
 * @param[in] simcom_version
 * @return Cellular band to be used.
 */
char *cellular_utilities_get_band(uint16_t mobile_country_code,
    uint8_t mobile_network_code,  uint8_t  simcom_version) {

  (void)simcom_version; // Very strange code!!! What is it for?

  if ((simcom_version != SIMCOM_7020_VERSION) && (simcom_version != SIMCOM_7080_VERSION)) {
    return "";
  }

  debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());
  switch (mobile_country_code) {
    case MCC_POLAND: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_band] poland mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_POLAND_COSWITCHED: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] coswitched mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] default mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
      }
    }
    case MCC_LUXEMBOURG: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_band] luxembourg mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_LUXEMBOURG_JOIN: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] join mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_apn] default mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
      }
    }
    case MCC_BRAZIL: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_band] brazil mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_BRAZIL_CLARO: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] claro mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_OI: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] oi mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_TIM_1: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] tim mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_TIM_2: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] tim mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_TIM_3: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] tim mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_VIVO_1: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vivo mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_VIVO_2: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vivo mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_VIVO_3: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vivo mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_VIVO_4: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vivo mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_VIVO_5: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vivo mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_VIVO_6: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vivo mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_BRAZIL_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vodafone mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] default mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
      }
    }
    case MCC_PORTUGAL: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_band] portugal mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_PORTUGAL_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vodafone mnc\n");
          return BAND_PORTUGAL_VODAFONE;
        }
        case MNC_PORTUGAL_MEO: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] meo mnc\n");
          return BAND_PORTUGAL_MEO;
        }
        case MNC_PORTUGAL_NOS: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] nos mnc\n");
          return BAND_PORTUGAL_VODAFONE;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] default mnc\n");
          return BAND_PORTUGAL_VODAFONE;
        }
      }
    }
    case MCC_GERMANY: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_band] germany mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_GERMANY_TELEKOM_1: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] telekom mnc\n");
          return BAND_GERMANY_TELEKOM;
        }
        case MNC_GERMANY_TELEKOM_2: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] telekom mnc\n");
          return BAND_GERMANY_TELEKOM;
        }
        case MNC_GERMANY_TELEKOM_3: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] telekom mnc\n");
          return BAND_GERMANY_TELEKOM;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] default mnc\n");
          return BAND_GERMANY_TELEKOM;
        }
      }
    }
    case MCC_DENMARK: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_band] denmark mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());
  
      switch (mobile_network_code) {
        case MNC_DENMARK_TELENOR: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] telenor mnc\n");
          return BAND_DENMARK_TELENOR;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] default mnc\n");
          return BAND_DENMARK_TELENOR;
        }
      }
    }
    case MCC_INTERNATIONAL: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_band] international mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_INTERNATIONAL_TELEKOM: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] telekom mnc\n");
          return BAND_GERMANY_TELEKOM;
        }
        case MNC_INTERNATIONAL_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vodafone mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_INTERNATIONAL_1NCE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] 1nce mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        case MNC_INTERNATIONAL_NETHERLANDS_TMOBILE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] t-mobile mnc\n");
          return BAND_INTERNATIONAL_NETHERLANDS_TMOBILE;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] default mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
      }
    }
    case MCC_TURKEY: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_band] turkey mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());

      switch (mobile_network_code) {
        case MNC_TURKEY_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vodafone mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] default mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
      }
    }
    case MCC_NETHERLANDS: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_band] netherlands mcc\n");
      debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());
  
      switch (mobile_network_code) {
        case MNC_NETHERLANDS_VODAFONE: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] vodafone mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
        default: {
          debug_print_string(DEBUG_LEVEL_2,
              (uint8_t *)"[cellular_utilities_get_band] default mnc\n");
          return BAND_MANUAL_SELECTION_DEFAULT;
        }
      }
    }
    default: {
      debug_print_string(DEBUG_LEVEL_2,
          (uint8_t *)"[cellular_utilities_get_band] default mcc\n");
      return BAND_MANUAL_SELECTION_DEFAULT;
    }
  }
}

/**
 * Tells if power saving mode can be active according with SIMCard profile.
 * @param[in] mobile_country_code
 * @param[in] mobile_network_code
 * @param[in] simcom_version
 * @return True if power saving mode is to be activated, false otherwise.
 */
bool cellular_utilities_has_psm_mode(uint16_t  mobile_country_code,
    uint8_t   mobile_network_code,   uint8_t   simcom_version) {

  if ((simcom_version != SIMCOM_7020_VERSION) && (simcom_version != SIMCOM_7080_VERSION)) {
    return false;
  }

  switch (mobile_country_code) {
    case MCC_POLAND: {
      switch (mobile_network_code) {
        case MNC_POLAND_COSWITCHED: {
          return false;
        }
        default: {
          return false;
        }
      }
    }
    case MCC_LUXEMBOURG: {
      switch (mobile_network_code) {
        case MNC_LUXEMBOURG_JOIN: {
          return false;
        }
        default: {
          return false;
        }
      }
    }
    case MCC_BRAZIL: {
      switch (mobile_network_code) {
        case MNC_BRAZIL_CLARO: {
          return false;
        }
        case MNC_BRAZIL_OI: {
          return false;
        }
        case MNC_BRAZIL_TIM_1: {
          return false;
        }
        case MNC_BRAZIL_TIM_2: {
          return false;
        }
        case MNC_BRAZIL_TIM_3: {
          return false;
        }
        case MNC_BRAZIL_VIVO_1: {
          return false;
        }
        case MNC_BRAZIL_VIVO_2: {
          return false;
        }
        case MNC_BRAZIL_VIVO_3: {
          return false;
        }
        case MNC_BRAZIL_VIVO_4: {
          return false;
        }
        case MNC_BRAZIL_VIVO_5: {
          return false;
        }
        case MNC_BRAZIL_VIVO_6: {
          return false;
        }
        case MNC_BRAZIL_VODAFONE: {
          return false;
        }
        default: {
          return false;
        }
      }
    }
    case MCC_PORTUGAL: {
      switch (mobile_network_code) {
        case MNC_PORTUGAL_VODAFONE: {
          return true;
        }
        case MNC_PORTUGAL_MEO: {
          return false;
        }
        case MNC_PORTUGAL_NOS: {
          return false;
        }
        default: {
          return false;
        }
      }
    }
    case MCC_GERMANY: {
      switch (mobile_network_code) {
        case MNC_GERMANY_TELEKOM_1: {
          return false;
        }
        case MNC_GERMANY_TELEKOM_2: {
          return false;
        }
        case MNC_GERMANY_TELEKOM_3: {
          return false;
        }
        default: {
          return false;
        }
      }
    }
    case MCC_INTERNATIONAL: {
      switch (mobile_network_code) {
        case MNC_INTERNATIONAL_TELEKOM: {
          return false;
        }
        default: {
          return false;
        }
      }
    }
    case MCC_TURKEY: {
      switch (mobile_network_code) {
        case MNC_TURKEY_VODAFONE: {
          return false;
        }
        default: {
          return false;
        }
      }
    }
    default: {
      return false;
    }
  }
}

/**
 * Converts dBm value to a correspondent received level.
 * @param[in] dbm
 * @return Received level.
 */
int32_t cellular_utilities_get_rxl_from_dbm(int32_t dbm) {

  if (dbm == 0) {
    return 0;
  }

  int32_t base_rssi = -113;

  int32_t ret = (int32_t)(dbm - base_rssi);

  /* Print results */
  debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_2,
      (uint8_t *)"[cellular_utilities_get_rxl_from_dbm] dbm: ");
  char char_aux[10];
  memset(char_aux, '\0', 10);
  utils_itoa(dbm, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)" = rxl: ");
  memset(char_aux, '\0', 10);
  utils_itoa(ret, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\n");

  return ret;
}

/**
 * Converts dBm value to a correspondent RSSI value.
 * @param[in] dbm
 * @return RSSI value.
 */
int32_t cellular_utilities_get_rssi_from_dbm(int32_t dbm) {
  
  if (dbm == 0) {
    return 0;
  }

  int32_t base_rssi = -113;

  int32_t ret = (int32_t)cellular_utilities_get_rssi_from_rxl(dbm - base_rssi);

  /* Print results */
  debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_2,
      (uint8_t *)"[cellular_utilities_get_rxl_from_dbm] dbm: ");
  char char_aux[10];
  memset(char_aux, '\0', 10);
  utils_itoa(dbm, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)" = rssi: ");
  memset(char_aux, '\0', 10);
  utils_itoa(ret, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\n");

  return ret;
}

/**
 * Converts a RSSI value to a dBm value.
 * @param[in] rssi
 * @return dBm value.
 */
int32_t cellular_utilities_get_dbm_from_rssi(int32_t rssi) {

  if (rssi == 0) {
    return 0;
  }

  int32_t base_rssi = -113;

  int32_t ret = (int32_t)(base_rssi + cellular_utilities_get_rxl_from_rssi(rssi));

  /* Print results */
  debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_2,
      (uint8_t *)"[cellular_utilities_get_dbm_from_rssi] rssi: ");
  char char_aux[10];
  memset(char_aux, '\0', 10);
  utils_itoa(rssi, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)" = dbm: ");
  memset(char_aux, '\0', 10);
  utils_itoa(ret, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\n");

  return ret;
}

/**
 * Converts a received level value to a dBm value.
 * @param[in] rxl
 * @return dBm value.
 */
int32_t cellular_utilities_get_dbm_from_rxl(int32_t rxl) {

  if (rxl == 0) {
    return 0;
  }

  int32_t base_rssi = -113;

  int32_t ret = (int32_t)(base_rssi + rxl);

  /* Print results */
  debug_print_time(DEBUG_LEVEL_2, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_2,
      (uint8_t *)"[cellular_utilities_get_dbm_from_rxl] rxl: ");
  char char_aux[10];
  memset(char_aux, '\0', 10);
  utils_itoa(rxl, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)" = dbm: ");
  memset(char_aux, '\0', 10);
  utils_itoa(ret, char_aux, 10);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)char_aux);
  debug_print_string(DEBUG_LEVEL_2, (uint8_t *)"\n");

  return ret;
}

/**
 * Converts a received level value to a RSSI value.
 * @param[in] rxl
 * @return RSSI value.
 */
int32_t cellular_utilities_get_rssi_from_rxl(int32_t rxl) {
  return (rxl / 2);
}

/**
 * Converts a RSSI value to a received level value.
 * @param[in] rssi
 * @return Received level value.
 */
int32_t cellular_utilities_get_rxl_from_rssi(int32_t rssi) {
  return (rssi * 2);
}
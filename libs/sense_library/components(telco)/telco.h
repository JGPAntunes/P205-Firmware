/*
* @file         telco.h
* @date         August 2021
* @author       Modified by PFaria & JAntunes
*
* @brief        This file has the telco utilities that controls the lower hierarchy layers (cellular and simcom).
                The telco layer was adapted from Sensefinity proprietary code, so there's alot of features we don't support.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*
*  This source code was adapted from the Sensefinity company provided source code:
  /*
   * Copyright (c) 2020 Sensefinity
   * This file is subject to the Sensefinity private code license.
   */
  /**
   * @file  telco.h
   * @brief This header file has the telco definitions.
   */
/* 
* 
*/

#ifndef TELCO_H_
#define TELCO_H_

/********************************** Includes ***********************************/
/* Config */
#include "config.h"

#if MICROCONTROLER_2
/* Standard C library */
#include <stdint.h>
#include <stdbool.h>

/* Data structs */
#include "sense_library//apps/rssi.h"
#include "sense_library/apps/cells.h"

/********************************** Definitions ********************************/

#define TELCO_UART_PARITY                (false)                       /*  Telco UART parity enabled */
#define TELCO_UART_FLOW_CONTROL          (false)                       /*  Telco UART flow control enabled */
#define TELCO_UART_BAUDRATE              (NRF_UART_BAUDRATE_115200)    /*  Telco UART baudrate */
#define TELCO_UART_INTERRUPTION_PRIORITY (APP_IRQ_PRIORITY_LOWEST)     /*  Telco UART priority level */

#define TELCO_UART_FIFO_TX_SIZE          (1024)                        /*  UART internal FIFO Tx size */
#define TELCO_UART_FIFO_RX_SIZE          (256)                         /*  UART internal FIFO Rx size */

#define CELLULAR_STATUS_PIN_DELAY        (3000)                        /* Delay used to emulate status pin */


/* Uplink configurations */
#define TELCO_UPLINK_ON_QUEUE_SIZE          (1)       /* Activate upload thresholds */
#define TELCO_UPLINK_QUEUE_SIZE             (1)     /* Upload threshold in bytes */
#define TELCO_UPLINK_ON_TIMEOUT             (0)       /* Activate upload via timeouts */
#define TELCO_UPLINK_TIMEOUT                (0)       /* Upload timeout in ms */
#define TELCO_UPLINK_REMAIN_OPEN            (2000)    /* Time that session remains open in ms */
#define TELCO_UPLINK_REGISTRATION_FAILED    (60000)   /* Time to retry network registration attempts in ms */

/* NB-IoT configurations */
#define TELCO_NBIOT_PSM_ON                  (0)       /* Power saving mode enabled */
#define TELCO_NBIOT_T3324                   (30)      /* Active time in PSM, default:30sec */
#define TELCO_NBIOT_T3412                   (12)      /* Sleep time in PSM, default:2h */
#define TELCO_NBIOT_EDRX_ON                 (0)       /* Extended Discontinuous Reception enabled */
#define TELCO_NBIOT_EDRX_PERIOD             (2)       /* Extended Discontinuous Reception */
#define TELCO_NBIOT_PTW_PERIOD              (3)       /* Extended Discontinuous Reception */


/********************************** Prototypes *********************************/
/* Telecomunications are on used by MCU1 */

void telco_init(void);

void telco_loop(uint64_t current_timestamp);

void telco_send_battery_saving_mode_message_autonomously(void);

void telco_send_power_off_message_autonomously(void);

bool telco_support_cells_service(void);

void telco_collect_cells(uint64_t current_timestamp);

bool telco_has_cells_data(void);

void telco_read_cells_data(struct cells_data* cells_info);

bool telco_is_collecting_cells_sample(void);

struct rssi_data* telco_get_rssi_data(void);

bool telco_is_busy(void);
#endif
#endif /* TELCO_H_ */
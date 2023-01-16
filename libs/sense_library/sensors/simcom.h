/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  simcom.h
 * @brief SIMCom driver header.
 */

#ifndef SIMCOM_H
#define SIMCOM_H

/********************************** Includes ***********************************/

/* Standard C library */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Utils */
#include "sense_library/utils/utils.h"

/* Libs */
#include "sense_library/utils/debug.h"

/********************************** Includes ***********************************/

#define RX_UART_SIMCOM_BUFFER_SIZE (1024) ///< Internal rx buffer size.

/**
 * Callback definition of send new string function.
 */
typedef void (*uart_send_string_callback_def)(char *command, uint16_t command_size);

/**
 * SIMCom responses structure.
 */
struct simcom_responses {
  char responses[RX_UART_SIMCOM_BUFFER_SIZE]; ///< Internal rx buffer.
  uint16_t responses_size;                    ///< Indicates the total size in bytes of all the received answers inside the buffer.
  uint16_t responses_number;                  ///< Note that each response is a sequence of bytes that in the end has a carriage return char.
};

/********************************** Definitions ********************************/

void simcom_setup(uart_send_string_callback_def uart_send_string_callback);

void simcom_reset_variables(void);

bool simcom_ready_to_rx_new_command(void);

bool simcom_send_command(char *command, uint16_t command_size, bool force);

uint16_t simcom_how_many_new_responses(void);

void simcom_read_responses(struct simcom_responses *responses);

void simcom_get_last_completed_transaction(char *last_command, struct simcom_responses *responses);

void simcom_rx_new_char(uint8_t new_char);

#endif /* SIMCOM_H */
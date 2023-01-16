/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  sequence_number.h
 * @brief This header file has the sequence number utilities.
 */

#ifndef SEQUENCE_NUMBER_H_
#define SEQUENCE_NUMBER_H_

/********************************** Includes ***********************************/

#include <stdint.h>
#include <stdbool.h>

/********************************** Definitions ********************************/

#define SEQUENCE_NUMBER_LOCK_TIMEOUT  (5000)  ///< Maximum timeout to release lock.

/********************************** Prototypes *********************************/

void sequence_number_init(void);

bool sequence_number_get(uint16_t *sequence_number_parameter);

bool sequence_number_get_lock(void);

uint16_t sequence_number_get_after_lock(void);

void sequence_number_set(uint16_t sequence_number);

void sequence_number_free_increment(void);

void sequence_number_free_no_increment(void);

bool sequence_number_save_in_message(uint8_t *message);

#endif /* SEQUENCE_NUMBER_H_ */

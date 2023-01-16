/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file timestamp.h
 * @brief This file has the timestamp header.
 */

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

/********************************** Includes ***********************************/

/* C standard library */
#include <stdint.h>

/********************************** Definitions ********************************/

#define MAX_TIME_IN_RMQ   (10800000)  ///< Maximum time that a message stays in the reverse message queue.

/********************************** Prototypes *********************************/

long long timestamping_out_packet(uint8_t *message, long long timestamp_in_ms);

void timestamping_out_ble_packet(uint8_t *message, long long timestamp_in_ms);

void timestamping_out_exp_packet(uint8_t *message, long long timestamp_in_ms);

void timestamping_in_packet(uint8_t *message, uint64_t time);

void timestamping_ble_reset(uint8_t *message);

void timestamping_exp_reset(uint8_t *message);

void timestamping_reverse_in_packet(uint8_t *message, uint64_t time);

void timestamping_reverse_out_packet(uint8_t *message, uint64_t time);

void timestamping_set_actual_timestamp(uint8_t *message, long long timestamp_in_ms);

#endif /* TIMESTAMP_H_ */

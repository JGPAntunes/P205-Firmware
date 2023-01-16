/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file timestamp.c
 * @brief This file has the timestamp functionality.
 */

/********************************** Includes ***********************************/

/* Interface */
#include "sense_library/utils/timestamp.h"

/* Gama */
#include "sense_library/protocol-gama/gama_fw/include/gama_node_header.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_definitions.h"

/* Utils */
#include "sense_library/utils/debug.h"

/********************************** Private ************************************/

/**
 * Store last BLE timestamp so that we can rollback if message needs to stay
 * longer in the queue.
 */
static long long ble_last_sysclk_used;

/**
 * Store last expansion timestamp so that we can rollback if message needs to stay
 * longer in the queue.
 */
static long long exp_last_sysclk_used;

/********************************** Public *************************************/

/**
 * Timestamp out packet.
 * @param[in] message Message to be timestamped.
 * @param[in] timestamp_in_ms Current timestamp (in ms).
 * @return Current timestamp (/p timestamp_in_ms).
 */
long long timestamping_out_packet(uint8_t *message, long long timestamp_in_ms) {
  long long new_timestamp = 0;
  long long sysclk_time = timestamp_in_ms;
  uint64_t current_timestamp;

  current_timestamp = gama_get_node_timestamp(message);
  new_timestamp = sysclk_time - (long long)current_timestamp;
  gama_set_node_timestamp(message, (uint64_t)new_timestamp);

  return sysclk_time;
}

/**
 * BLE timestamp out packet.
 * @param[in] message Message to be timestamped.
 * @param[in] timestamp_in_ms Current timestamp (in ms).
 */
void timestamping_out_ble_packet(uint8_t *message, long long timestamp_in_ms) {
  ble_last_sysclk_used = timestamping_out_packet(message, timestamp_in_ms);
}

/**
 * Expansion layer timestamp out packet.
 * @param[in] message Message to be timestamped.
 * @param[in] timestamp_in_ms Current timestamp (in ms).
 */
void timestamping_out_exp_packet(uint8_t *message, long long timestamp_in_ms) {
  exp_last_sysclk_used = timestamping_out_packet(message, timestamp_in_ms);
}

/**
 * Timestamp in packet.
 * @param[in] message Message to be timestamped.
 * @param[in] timestamp_in_ms Current timestamp (in ms).
 */
void timestamping_in_packet(uint8_t *message, uint64_t timestamp_in_ms) {
  long long new_timestamp = 0;
  uint64_t current_timestamp;

  current_timestamp = gama_get_node_timestamp(message);
  new_timestamp = (long long)timestamp_in_ms - (long long)current_timestamp;
  gama_set_node_timestamp(message, (uint64_t)new_timestamp);
}

/**
 * BLE timestamp packet reset (rollback to the previous timestamp in).
 * @param[in] message Message to be timestamped.
 */
void timestamping_ble_reset(uint8_t *message) {
  long long old_timestamp = 0;
  uint64_t current_timestamp;

  current_timestamp = gama_get_node_timestamp(message);
  old_timestamp = ble_last_sysclk_used - (long long)current_timestamp;
  gama_set_node_timestamp(message, (uint64_t)old_timestamp);
}

/**
 * Expansion layer timestamp packet reset (rollback to the previous timestamp in).
 * @param[in] message Message to be timestamped.
 */
void timestamping_exp_reset(uint8_t *message) {
  long long old_timestamp = 0;
  uint64_t current_timestamp;

  current_timestamp = gama_get_node_timestamp(message);
  old_timestamp = exp_last_sysclk_used - (long long)current_timestamp;
  gama_set_node_timestamp(message, (uint64_t)old_timestamp);
}

/**
 * Set message in timestamp.
 * @param[in] message Message to be timestamped.
 * @param[in] timestamp_in_ms Current timestamp (in ms).
 */
void timestamping_reverse_in_packet(uint8_t *message, uint64_t timestamp_in_ms) {
  long long new_timestamp = 0;
  uint64_t msg_timestamp;

  msg_timestamp = gama_get_node_timestamp(message);

  /* Limiting message time to live */
  if(msg_timestamp > MAX_TIME_IN_RMQ) {
    msg_timestamp = MAX_TIME_IN_RMQ;
  }

  new_timestamp = (long long)timestamp_in_ms + (long long)msg_timestamp;
  gama_set_node_timestamp(message, (uint64_t)new_timestamp);
}

/**
 * Reverse message's timestamp out.
 * @param[in] message Message to be timestamped.
 * @param[in] timestamp_in_ms Current timestamp (in ms).
 */
void timestamping_reverse_out_packet(uint8_t *message, uint64_t timestamp_in_ms) {
  long long new_timestamp = 0;
  uint64_t msg_timestamp;

  msg_timestamp = gama_get_node_timestamp(message);

  if(msg_timestamp > timestamp_in_ms) {
    new_timestamp =  (long long)msg_timestamp - (long long)timestamp_in_ms;
  } 
  else {
    new_timestamp = 60000;
  }

  gama_set_node_timestamp(message, (uint64_t)new_timestamp);
}

/**
 * Set message timestamp directly with the /p timestamp_in_ms.
 * @param[in] message Message to be timestamped.
 * @param[in] timestamp_in_ms Current timestamp (in ms).
 */
void timestamping_set_actual_timestamp(uint8_t *message, long long timestamp_in_ms) {
  gama_set_node_timestamp(message, timestamp_in_ms);
}
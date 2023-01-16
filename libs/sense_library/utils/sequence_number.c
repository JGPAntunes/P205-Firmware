/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file sequence_number.c
 * @brief This file has the sequence number utilities.
 */

/********************************** Includes ***********************************/

/* Interface */
#include "sense_library/utils/sequence_number.h"

/* Libs */
#include "sense_library/utils/lock.h"
#include "sense_library/utils/debug.h"

/* Gama protocol */
#include "sense_library/protocol-gama/gama_fw/include/gama.h"

/* Utils */
#include "sense_library/utils/externs.h"

/********************************** Private ************************************/

/**
 * Current sequence number value.
 */
static uint16_t _current_sequence  = 0;

/**
 * Current sequence number value.
 */
static enum lock _sequence_number_lock = UNLOCKED;

/**
 * Tells about last time lock was acquired.
 */
static uint64_t _last_lock_acquired_timestamp = 0;

/********************************** Public *************************************/

/**
 * Sequence number initialization.
 */
void sequence_number_init(void) {
  _current_sequence = 0;
  _last_lock_acquired_timestamp = 0;
}

/**
 * Sets the current sequence number in the message's
 * sequence number field.
 * @param[in] sequence_number_parameter Pointer to the sequence number field.
 * True if set is done, false otherwise (and parameter is set with 0).
 */
bool sequence_number_get(uint16_t *sequence_number_parameter) {
  /* If some app got the lock for the sequence number we have to release it
   * if it's taking too long to unlock */
  if((_sequence_number_lock == LOCKED) && (_last_lock_acquired_timestamp >= SEQUENCE_NUMBER_LOCK_TIMEOUT)) {
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[sequence_number_get] Some app is getting lock for too long. Release it for others.\n");

    sequence_number_free_no_increment();
  }

  /* Try to reserve the lock */
  if(lock_try_acquire(&_sequence_number_lock)) {
    _last_lock_acquired_timestamp = rtc_get_milliseconds();
    *sequence_number_parameter = _current_sequence;
    return true;
  }
  else {
    *sequence_number_parameter = 0;
    return false;
  }
}

/**
 * Gets sequence number lock.
 * @return True if lock acquired, false otherwise.
 */
bool sequence_number_get_lock(void) {
  /* If some app got the lock for the sequence number we have to release it
   * if it is taking too long to unlock. */
  if((_sequence_number_lock == LOCKED) && (_last_lock_acquired_timestamp >= SEQUENCE_NUMBER_LOCK_TIMEOUT)) {
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[sequence_number_get] Some app is getting lock for too long. Release it for others.\n");

    sequence_number_free_no_increment();
  }

  /* Try to reserve the lock */
  if(lock_try_acquire(&_sequence_number_lock)) {
    _last_lock_acquired_timestamp = rtc_get_milliseconds();
    return true;
  }
  else {
    return false;
  }
}

/**
 * Gets current sequence number. Only use this if acquire lock was called
 * manually.
 * @return Current sequence number value.
 */
uint16_t sequence_number_get_after_lock(void) {
  return _current_sequence;
}

/**
 * Sets current sequence number.
 * @param[in] sequence_number New sequence number value.
 */
void sequence_number_set(uint16_t sequence_number) {
  _current_sequence = sequence_number;
}

/**
 * Increments sequence number and release internal lock.
 */
void sequence_number_free_increment(void) {
  /* Wait and reserve the lock */
  _current_sequence++;
  lock_unlock(&_sequence_number_lock);
  return;
}

/**
 * Releases internal lock without incrementing current sequence number.
 */
void sequence_number_free_no_increment(void) {
  /* Wait and reserve the lock */
  lock_unlock(&_sequence_number_lock);
  return;
}

/**
 * Updates sequence number directly in the message.
 * @param[in] message
 * Returns true if operation was successfully done, false otherwise.
 */
bool sequence_number_save_in_message(uint8_t *message) {
  /* Check data size */
  uint32_t data_size = gama_get_node_data_size(message);
  if(!data_size) {
    return false;
  }

  gama_node_fields_t gama_fields = gama_get_node_fields(message);

  /* Save current sequence_number */
  gama_fields.seq_number = _current_sequence;
  uint8_t header_size = gama_get_node_header_size(&gama_fields);
  gama_fill_node_header(&gama_fields, message, header_size);

  return true;
}

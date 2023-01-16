/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  lock.c
 * @brief This file has the lock utilities.
 */

/********************************** Includes ***********************************/

/* Interface */
#include "sense_library/utils/lock.h"

/* Utils */
#include "sense_library/utils/atomic.h"

/********************************** Private ************************************/



/********************************** Public *************************************/

/**
 * Try to acquire lock.
 * @param[in] lock
 * @return True if lock acquired, false otherwise.
 */
bool lock_try_acquire(enum lock *lock) {
  bool ret = false;

  atomic(
    if(*lock == UNLOCKED) {
      *lock = LOCKED;
      ret = true;
    }
  );

  return ret;
}

/**
 * Unlock lock.
 * @param[in] lock
 */
void lock_unlock(enum lock *lock) {
  atomic(
    *lock = UNLOCKED;
  );
}
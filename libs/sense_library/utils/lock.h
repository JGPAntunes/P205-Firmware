/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  lock.h
 * @brief This header file has the lock utilities.
 */

#ifndef LOCK_H_
#define LOCK_H_

/********************************** Includes ***********************************/

#include <stdbool.h>

/********************************** Definitions ********************************/

/**
 * Lock data structure.
 */
enum lock {
  LOCKED   = 0,
  UNLOCKED = 1
};

/********************************** Prototypes *********************************/

bool lock_try_acquire(enum lock *l);

void lock_unlock(enum lock *l);

#endif /* LOCK_H_ */
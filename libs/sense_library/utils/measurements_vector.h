/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file measurements_vector.h
 * @brief This header file has the measurements utils (e.g. mean, etc.).
 */

#ifndef MEASUREMENTS_VECTOR_H
#define MEASUREMENTS_VECTOR_H

/********************************** Includes ***********************************/

/* Standard C libraries */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/********************************** Definitions ********************************/

#define MEASUREMENTS_NUMBER_MAX     (10)    ///< Maximum number of measurements inside the buffer.

struct __attribute__ ((__packed__)) measurements_mean {
  float accumulation;                       ///< The accumulation of the various measurements.
  size_t count;                             ///< The number of received measurements.
};

struct __attribute__ ((__packed__)) measurements_median {
  float values[MEASUREMENTS_NUMBER_MAX];    ///< Measurements' buffer.
  size_t count;                             ///< Store the number of measurements inside the buffer.
};


struct __attribute__ ((__packed__)) measurements_recent {
  float last;                               ///< The last value measured.
};

struct __attribute__ ((__packed__)) measurements_max {
  float max;                                ///< The maximum measured value in a cycle.
};

/**
 * Operation type definition.
 */
enum measurements_mode {
  MEASUREMENTS_VECTOR_MEAN   = 0,
  MEASUREMENTS_VECTOR_MEDIAN = 1,
  MEASUREMENTS_VECTOR_RECENT = 2,
  MEASUREMENTS_VECTOR_MAX    = 3,
  MEASUREMENTS_VECTOR_MODE   = 4
};

/********************************** Prototypes *********************************/

/********************************** Prototypes ***********************************/

void measurements_vector_init(void *measurements_struct, enum measurements_mode mode);

void measurements_vector_add(void *measurements_struct, enum measurements_mode mode, float value);

float measurements_vector_get(void *measurements_struct, enum measurements_mode mode);

#endif /* MEASUREMENTS_VECTOR_H */

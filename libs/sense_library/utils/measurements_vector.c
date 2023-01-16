/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file measurements_vector.c
 * @brief This file has the measurements utils (e.g. mean, etc.).
 */

/********************************** Includes ***********************************/

/* Interface */
#include "measurements_vector.h"

/********************************** Private ************************************/

/**
 * Compare two float values.
 * @param[in] a Value a to be compared.
 * @param[in] b Value b to be compared.
 * @return Difference between the two values.
 */
static int compare(const void *a, const void *b) {
  return (int) (*(float*)a - *(float*)b);
}

/**
 * Gets measurements' mean.
 * @param[in] measurements The measurements' buffer.
 * @param[in] count        The measurements' count.
 * @return Mean value.
 */
float get_mean(float *measurements, size_t count) {
  if(!(count > 0)) {
    return 0;
  }
  
  if(!(measurements != NULL)) {
    return 0;
  }

  if (count == 1) {
    return *measurements;
  }

  float accumulation = 0;
  for (size_t i = 0; i < count; ++i) {
    accumulation += measurements[i];
  }

  return accumulation / count;
}

/**
 * Gets measurements' median.
 * @param[in] numbers Stored samples.
 * @param[in] size Number of measurements to process.
 * @return Median.
 */
float get_median(float *numbers, size_t size) {
  if(!(size > 0)) return 0;
  if(!(numbers != NULL)) return 0;

  if (size == 1) {
    return *numbers;
  }
  
  qsort(numbers, size, sizeof(float), compare);

  /* if the number of elements is even */
  if(!(size & 0b1)){
    /* average of both middle values */
    return (numbers[size / 2] + numbers[(size / 2) - 1]) / 2;
  }else{
    /* return middle value */
    return numbers[size / 2];
  }
}

/**
 * Gets measurements' mode.
 * @param[in] numbers
 * @param[in] size Number of measurements to process.
 * @return Median.
 */
float get_mode(float *numbers, size_t size) {
  if(size <= 0) {
    return 0;
  }
  
  if(numbers == NULL) {
    return 0;
  }

  if (size == 1) {
    return *numbers;
  }

  qsort(numbers, size, sizeof(float), compare);
  
  size_t i;
  int count, max_count;
  float mode;
  for (i = 1, count = 1, max_count = 0, mode = numbers[0]; i < size; ++i) {
    if (numbers[i - 1] != numbers[i]) {
      if (max_count < count) {
        mode = numbers[i - 1];
        max_count = count;
      }
      count = 1;
    } else {
      count++;
    }
  }
  
  if (max_count < count) {
    mode = numbers[i - 1];
  }
  
  return mode;
}

/********************************** Public *************************************/

/**
 * Measurements vector initialization.
 * @param[in] measurements_struct
 * @param[in] mode Operation mode.
 */
void measurements_vector_init(void *measurements_struct, enum measurements_mode mode){
  if(mode == MEASUREMENTS_VECTOR_RECENT) {
    struct measurements_recent *recent = measurements_struct;
    recent->last = 0;
  } else if(mode == MEASUREMENTS_VECTOR_MAX) {
    struct measurements_max *max = measurements_struct;
    max->max = 0;
  } else if(mode == MEASUREMENTS_VECTOR_MEAN) {
    struct measurements_mean *mean = measurements_struct;
    mean->accumulation = 0;
    mean->count = 0;
  } else if(mode == MEASUREMENTS_VECTOR_MEDIAN) {
    struct measurements_median *median = measurements_struct;
    for(size_t i = 0; i < MEASUREMENTS_NUMBER_MAX; i++) {
      median->values[i] = 0;
    }
    median->count = 0;
  } else if(mode == MEASUREMENTS_VECTOR_MODE) {
    struct measurements_median *mode = measurements_struct;
    for(size_t i = 0; i < MEASUREMENTS_NUMBER_MAX; i++) {
      mode->values[i] = 0;
    }
    mode->count = 0;
  }
}

/**
 * Adds new measurement to measurements' struct.
 * @param[in] measurements_struct
 * @param[in] mode  Operation mode.
 * @param[in] value New value to be added.
 */
void measurements_vector_add(void *measurements_struct, enum measurements_mode mode, float value) {
  if(mode == MEASUREMENTS_VECTOR_RECENT) {
    struct measurements_recent *recent = measurements_struct;
    recent->last = value;
  } else if(mode == MEASUREMENTS_VECTOR_MAX) {
    struct measurements_max *max = measurements_struct;
    if(value>max->max) {
      max->max = value;
    }
  } else if(mode == MEASUREMENTS_VECTOR_MEAN) {
    struct measurements_mean *mean = measurements_struct;
    mean->accumulation += value;
    mean->count++;
  } else if(mode == MEASUREMENTS_VECTOR_MEDIAN){
    struct measurements_median *median = measurements_struct;
    if(median->count < MEASUREMENTS_NUMBER_MAX) {
      median->values[median->count] = value;
      median->count++;
    }
  }else if(mode == MEASUREMENTS_VECTOR_MODE){
    struct measurements_median *mode = measurements_struct;
    if(mode->count < MEASUREMENTS_NUMBER_MAX){
      mode->values[mode->count] = value;
      mode->count++;
    }
  }
}

/**
 * Gets value.
 * @param[in] measurements_struct
 * @param[in] mode Operation mode.
 * @return Value from measurement's structure.
 */
float measurements_vector_get(void *measurements_struct, enum measurements_mode mode) {
  if(mode == MEASUREMENTS_VECTOR_RECENT) {
    struct measurements_recent *recent = measurements_struct;
    return recent->last;
  } else if (mode == MEASUREMENTS_VECTOR_MAX) {
    struct measurements_max *max = measurements_struct;
    return max->max;
  } else if (mode == MEASUREMENTS_VECTOR_MEAN) {
    struct measurements_mean *mean = measurements_struct;
    if((mean->accumulation == 0) || (mean->count == 0)) {
      return 0;
    }
    return mean->accumulation / mean->count;
  } else if (mode == MEASUREMENTS_VECTOR_MEDIAN) {
    struct measurements_median *median = measurements_struct;
    return get_median(median->values, median->count);
  } else if (mode == MEASUREMENTS_VECTOR_MODE) {
    struct measurements_median *mode = measurements_struct;
    return get_mode(mode->values, mode->count);
  }
  return 0;
}

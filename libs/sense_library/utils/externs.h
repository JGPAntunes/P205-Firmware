/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  externs.h
 * @brief This file has the declared external functions that we will need
 * in the Libs repository.
 */

#ifndef EXTERNS_H_
#define EXTERNS_H_

/********************************** Includes ***********************************/

#include "c_queue.h"

/* Config */
#include "config.h"
/********************************** Definitions ********************************/

extern const uint8_t GAMA_QUEUE_ADD_MAX_ATTEMPTS;               ///< Tells the maximum attempts to add a new packets to Gama queue.

extern const uint8_t CONFIGURATION_WORKING_MODE_POWERED_ON;     ///< Working mode on configuration.
extern const uint8_t CONFIGURATION_WORKING_MODE_POWERED_OFF;    ///< Working mode off configuration.
          
/********************************** Prototypes *********************************/

/**
 * Extern function to get device's serial number.
 * @return Device's serial number.
 */
extern uint64_t factory_get_serial_number(void);

/**
 * Extern function to get configuration working mode parameter value.
 * @return Working mode configuration parameter value.
 */
extern uint8_t configuration_get_working_mode(void);

/**
 * Extern function to set configuration working mode parameter value.
 * @param[in] working_mode New value to configuration working mode parameter.
 */
extern void configuration_set_working_mode(uint8_t working_mode);

/**
 * Extern function to get device's current timestamp.
 * @return Current device's timestamp.
 */
extern uint64_t rtc_get_milliseconds(void);

/**
 * Extern function to get a random generated number.
 * @return Random generated number.
 */
extern uint8_t utils_random_generate(void);

#endif /* EXTERNS_H_ */

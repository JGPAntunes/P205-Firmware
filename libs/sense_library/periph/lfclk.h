/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file lfclk.h
 * @brief This header file has the low frequency clock peripherical utilities.
 */

#ifndef LFCLK_H_
#define LFCLK_H_

/********************************** Includes ***********************************/

/* Standard C library */
#include <stdint.h>

/********************************** Definitions ********************************/

/* Position of BYPASS field. */
#define CLOCK_LFCLKSRC_BYPASS_Pos        (16UL)   ///< Enable or disable bypass of LFCLK crystal oscillator with external clock source (bits 16).
#define CLOCK_LFCLKSRC_BYPASS_Disabled   (0UL)    ///< Disabled bypass bit.
#define CLOCK_LFCLKSRC_BYPASS_Enabled    (1UL)    ///< Enable bypass bit.

/* Position of EXTERNAL field. */
#define CLOCK_LFCLKSRC_EXTERNAL_Pos      (17UL)   ///< Enable or disable external source for LFCLK (bits 17).
#define CLOCK_LFCLKSRC_EXTERNAL_Disabled (0UL)    ///< Disabled external source bit.
#define CLOCK_LFCLKSRC_EXTERNAL_Enabled  (1UL)    ///< Enable external source bit.

/********************************** Prototypes *********************************/

void lfclock_init(void);

#endif /* LFCLK_H_ */
/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file lfclk.c
 * @brief This file has the low frequency clock peripherical utilities.
 */

/********************************** Includes ***********************************/

/* Interface */
#include "lfclk.h"

/* SDK */
#include "nrf_drv_clock.h"

/********************************** Private ************************************/



/********************************** Public *************************************/

/**
 * Low frequency clock pre-configuration.
 */
void lfclock_init(void) {
  /* Configuring LFCLK to use an external signal */
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
   /*|
    (CLOCK_LFCLKSRC_BYPASS_Enabled << CLOCK_LFCLKSRC_BYPASS_Pos ) |
    (CLOCK_LFCLKSRC_EXTERNAL_Enabled << CLOCK_LFCLKSRC_EXTERNAL_Pos);*/

  /* Starting LFCLK */
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;

  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {
    /* Nothing to do */
  }
}
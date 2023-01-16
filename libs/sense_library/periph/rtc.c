/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file rtc.c
 * @brief This file has the RTC peripherical utilities.
 * @author João Ambrósio and Jorge Veiga
 * @date 17/12/2020
 */

/********************************** Includes ***********************************/

/* Interface */
#include "rtc.h"

/* SDK */
#include "nrf_drv_rtc.h"

/********************************** Private ************************************/

/**
 * Declaring RTC2.
 */
const nrf_drv_rtc_t _rtc = NRF_DRV_RTC_INSTANCE(2);

/**
 * RTC base counter (overflow + current counter).
 */
static volatile uint64_t _base = 0;

/**
 * Additional safe timestamp updated on every 24 bit period used
 * when "_base" was not updated and overflow occured.
 */
static uint64_t _safe_stamp = 0;

/**
 * Function for handling the RTC2 interrupts.
 * @param[in] int_type Type that cause the interruption.
 */
static void rtc_handler(nrf_drv_rtc_int_type_t int_type) {
  if (int_type == NRF_DRV_RTC_INT_OVERFLOW) {
    _base += (RTC_OVERFLOW + 1);
  }
  /* Channel 0 is triggered in the middle of 24 bit period to updated control 
   * timestamp in place where there is no risk of overflow. */
  if (int_type == NRF_DRV_RTC_INT_COMPARE0) {
    _safe_stamp = _base + nrf_drv_rtc_counter_get(&_rtc);
  }
}

/********************************** Public *************************************/

/**
 * RTC configuration.
 */
void rtc_init(void) {
  nrf_drv_rtc_config_t config = NRF_DRV_RTC_DEFAULT_CONFIG;
  config.prescaler = 0;
  config.interrupt_priority = RTC_INTERRUPT_PRIORITY;
  nrf_drv_rtc_init(&_rtc, &config, rtc_handler);

  nrfx_rtc_tick_disable(&_rtc);

  /* Enable overflow */
  nrf_drv_rtc_overflow_enable(&_rtc, true);

  /* Enable safe counter */
  nrf_drv_rtc_cc_set(&_rtc, RTC_SAFE_COUNTER_CC, (RTC_OVERFLOW >> 1), true);

  /* Power on RTC */
  nrf_drv_rtc_enable(&_rtc);
}

/**
 * RTC set counter in milliseconds.
 * @param[in] ts_now, which counts in millisenconds (e.g., External RTC uptime in milliseconds
 * @note Should be invoked once having an accurate value as is the one obtained from External RTC.
 */
void rtc_set_timer(uint64_t ts_now) {
  uint64_t t64 = RTC_MS_TO_TICKS(ts_now);

  /* Stop RTC */
  nrf_drv_rtc_disable(&_rtc);

  /* Update base base using the provided time */
  _base = t64; 
  _safe_stamp = _base;

  nrfx_rtc_tick_disable(&_rtc);

  /* Enable overflow */
  nrf_drv_rtc_overflow_enable(&_rtc, true);

  /* Enable safe counter */
  nrf_drv_rtc_cc_set(&_rtc, RTC_SAFE_COUNTER_CC, (RTC_OVERFLOW >> 1), true);

  /* Re-start RTC timer */
  nrf_drv_rtc_enable(&_rtc);
}

/**
 * Get internal RTC value.
 * @return Current RTC clock in milliseconds.
 */
uint64_t rtc_get_milliseconds(void) {
  uint64_t ticks;

  /* Read ticks */
  ticks = _base + nrf_drv_rtc_counter_get(&_rtc);

  /* It is possible that "_base" was not updated and overflow occured, in that case "ticks" will be
   * 24bit value behind. Additional timestamp updated on every 24 bit period is used to detect
   * that case. Apart from that "now" should never be behind previously read timestamp.
   */
  if (ticks < _safe_stamp) {
    ticks += (RTC_OVERFLOW + 1);
  }
  return RTC_TICKS_TO_MS(ticks);
}

/**
 * Get internal RTC value.
 * @return Current RTC clock in seconds.
 */
uint64_t rtc_get_seconds(void) {
  return RTC_MS_TO_S(rtc_get_milliseconds());
}
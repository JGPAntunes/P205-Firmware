/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  log.c
 * @brief This file has the log utilities.
 */

/********************************** Includes ***********************************/

/* Interface */
#include "log.h"

/* Standard C library */
#include <stdarg.h>

/* SDK */
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/* Libs */
#include "sense_library/utils/debug.h"

/********************************** Private ************************************/

/**
 * Callback function for print string with args.
 * @param[in] string String to be printed.
 * @param[in] args   Arguments passed to printf.
 */
void debug_printf_str(uint8_t *string, va_list args) {
#if defined(DEBUG)
  vprintf((char*)string, args);
#endif
}

/**
 * Callback function for print string.
 * @param[in] string String to be printed.
 */
void debug_print_str(uint8_t *string, ...) {
#if defined(DEBUG)
  printf((char*)string);
#endif
}

/**
 * Callback function for print char.
 * @param[in] ch Char to be printed.
 */
void debug_print_ch(uint8_t ch) {
#if defined(DEBUG)
  printf("%c", (char)ch);
#endif
}

/********************************** Public *************************************/

/**
 * Function for initializing the nRF log module.
 */
void log_init(void) {
#if defined(DEBUG) 
  NRF_LOG_INIT(NULL);
  NRF_LOG_DEFAULT_BACKENDS_INIT();

  debug_init(DEBUG_LEVEL_1, debug_printf_str, debug_print_str, debug_print_ch);
#endif
}

/**
 * Function that calls log flush.
 */
void log_flush(void) {
#if defined(DEBUG) 
  NRF_LOG_FLUSH();
#endif
}
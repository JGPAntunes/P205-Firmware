/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  debug.c
 * @brief This file has the debug utilities.
 */

/********************************** Libraries ***********************************/

#include "debug.h"
#include "utils.h"
#include <stdarg.h>

/********************************** Private ***********************************/

/**
 * Stores current debug level.
 */
static uint8_t current_debug_level = DEBUG_LEVEL_DEFAULT;

/**
 * Stores debug activated flag.
 */
static bool is_debug_active = false;

/**
 * Callback function that "printfs" a char.
 */
static Debug_Printf_String _debug_printf_string = NULL;

/**
 * Callback function that prints a string.
 */
static Debug_Print_String _debug_print_string = NULL;

/**
 * Callback function that prints a char.
 */
static Debug_Print_Char _debug_print_char = NULL;

/**
 * Tells if debug is activatedor not.
 * @return True if debug is active, false otherwise.
 */
bool is_debug_activated(void) {
  return is_debug_active;
}

/********************************** Public *************************************/

/**
 * Debug initialization.
 * param[in] debug_level         Selected debug level (lower values = more debug print details).
 * param[in] debug_printf_string Printf callback function.
 * param[in] debug_print_string  Print callback function.
 * param[in] debug_print_char    Print char callback function.
 */
void debug_init(uint8_t debug_level,
    Debug_Printf_String debug_printf_string,
    Debug_Print_String debug_print_string,
    Debug_Print_Char debug_print_char) {
  current_debug_level = debug_level;
  is_debug_active = true;

  _debug_printf_string = debug_printf_string;
  _debug_print_string = debug_print_string;
  _debug_print_char = debug_print_char;
}

/**
 * Debug printf function.
 * param[in] debug_level Selected debug level (lower values = more debug print details).
 * param[in] string      String to be printed out.
 */
void debug_printf_string(int debug_level, uint8_t *string, ...) {
  if (is_debug_active && (debug_level <= current_debug_level)) {
    va_list args;
    va_start(args, string);
    _debug_printf_string(string, args);
    va_end(args);
  }
}

/**
 * Debug print function.
 * param[in] debug_level Selected debug level (lower values = more debug print details).
 * param[in] string      String to be printed out.
 */
void debug_print_string(int debug_level, uint8_t *string, ...) {
  if (is_debug_active && (debug_level <= current_debug_level)) {
    _debug_print_string(string);
  }
}

/**
 * Debug print char function.
 * param[in] debug_level Selected debug level (lower values = more debug print details).
 * param[in] string      String to be printed out.
 */
void debug_print_char(int debug_level, uint8_t ch) {
  if (is_debug_active && (debug_level <= current_debug_level)) {
    _debug_print_char(ch);
  }
}

/**
 * Debug print internal device's timestamp function (to keep track time in debug print log).
 * param[in] debug_level         Selected debug level (lower values = more debug print details).
 * param[in] internal_time_in_ms Internal device's timestamp.
 */
void debug_print_time(int debug_level, uint64_t internal_time_in_ms) {
  if (is_debug_active && (debug_level <= current_debug_level)) {
    uint64_t internal_time_in_s = (internal_time_in_ms / 1000);
    uint64_t days, hours, minutes, seconds, milliseconds = 0;

    days = (internal_time_in_s / 86400);
    hours = ((internal_time_in_s - (days * 86400)) / 3600);
    minutes = (internal_time_in_s - (days * 86400) - (hours * 3600)) / 60;
    seconds = (internal_time_in_s - (days * 86400) - (hours * 3600) - (minutes * 60));
    milliseconds = (internal_time_in_ms - (days * 86400 * 1000) - (hours * 3600 * 1000) - (minutes * 60 * 1000) - (seconds * 1000));

    char str_aux[10];

    debug_print_string(debug_level, (uint8_t *)"[");
    memset(str_aux, '\0', 10);
    utils_itoa(days, str_aux, 10);
    debug_print_string(debug_level, (uint8_t *)str_aux);
    debug_print_string(debug_level, (uint8_t *)":");
    memset(str_aux, '\0', 10);
    utils_itoa(hours, str_aux, 10);
    debug_print_string(debug_level, (uint8_t *)str_aux);
    debug_print_string(debug_level, (uint8_t *)":");
    memset(str_aux, '\0', 10);
    utils_itoa(minutes, str_aux, 10);
    debug_print_string(debug_level, (uint8_t *)str_aux);
    debug_print_string(debug_level, (uint8_t *)":");
    memset(str_aux, '\0', 10);
    utils_itoa(seconds, str_aux, 10);
    debug_print_string(debug_level, (uint8_t *)str_aux);
    debug_print_string(debug_level, (uint8_t *)":");
    memset(str_aux, '\0', 10);
    utils_itoa(milliseconds, str_aux, 10);
    debug_print_string(debug_level, (uint8_t *)str_aux);
    debug_print_string(debug_level, (uint8_t *)"] ");
  }
}

/**
 * Debug printf internal device's timestamp function (to keep track time in debug print log).
 * param[in] debug_level         Selected debug level (lower values = more debug print details).
 * param[in] internal_time_in_ms Internal device's timestamp.
 */
void debug_printf_time(int debug_level, uint64_t internal_time_in_ms) {
  if (is_debug_active && (debug_level <= current_debug_level)) {
    uint64_t internal_time_in_s = (internal_time_in_ms / 1000);
    uint64_t days, hours, minutes, seconds, milliseconds = 0;

    days = (internal_time_in_s / 86400);
    debug_printf_string(debug_level, (uint8_t *)"[%d:", days);
    hours = ((internal_time_in_s - (days * 86400)) / 3600);
    debug_printf_string(debug_level, (uint8_t *)"%d:", hours);
    minutes = (internal_time_in_s - (days * 86400) - (hours * 3600)) / 60;
    debug_printf_string(debug_level, (uint8_t *)"%d:", minutes);
    seconds = (internal_time_in_s - (days * 86400) - (hours * 3600) - (minutes * 60));
    debug_printf_string(debug_level, (uint8_t *)"%d:", seconds);
    milliseconds = (internal_time_in_ms - (days * 86400 * 1000) - (hours * 3600 * 1000) - (minutes * 60 * 1000) - (seconds * 1000));
    debug_printf_string(debug_level, (uint8_t *)"%d] ", milliseconds);
  }
}
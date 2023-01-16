/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file debug.h
 * @brief This file has the debug header.
 */

#ifndef DEBUG_H_
#define DEBUG_H_

/********************************** Libraries ***********************************/

/* Standard C libraries */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/********************************** Definitions ********************************/

#define DEBUG_LEVEL_0         (0)             ///< Debug level (everything will be printed out).
#define DEBUG_LEVEL_1         (1)             ///< Debug level.
#define DEBUG_LEVEL_2         (2)             ///< Debug level.
#define DEBUG_LEVEL_3         (3)             ///< Debug level.
#define DEBUG_LEVEL_4         (4)             ///< Debug level (more selective one).

/* Higher level = more details */
#define DEBUG_LEVEL_DEFAULT   (DEBUG_LEVEL_1) ///< Default debug level.

typedef void (*Debug_Printf_String)(uint8_t *string, va_list args); ///< Printf callback definition.

typedef void (*Debug_Print_String)(uint8_t *string, ...);           ///< Print callback definition.

typedef void (*Debug_Print_Char)(uint8_t ch);                       ///< Print char callback definition.

/********************************** Prototypes *********************************/

void debug_init(uint8_t debug_level, Debug_Printf_String debug_printf_string, Debug_Print_String debug_print_string, Debug_Print_Char debug_print_char);

void debug_printf_string(int debug_level, uint8_t *string, ...);

void debug_print_string(int debug_level, uint8_t *string, ...);

void debug_print_char(int debug_level, uint8_t ch);

void debug_print_time(int debug_level, uint64_t internal_time_in_ms);

void debug_printf_time(int debug_level, uint64_t internal_time_in_ms);

#endif /* DEBUG_H_ */
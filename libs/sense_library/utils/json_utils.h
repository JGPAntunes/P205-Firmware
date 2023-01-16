/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  json_utils.h
 * @brief This file has the JSON utilities header.
 */

#ifndef JSON_UTILS_H_
#define JSON_UTILS_H_

/********************************** Includes ***********************************/

/* Utilities */
#include "sense_library/utils/json.h"

/* C standard library */
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/********************************** Definitions ********************************/

#define JSON_VALUE_STRING_SIZE_MAX    (50)    ///< String maximum size for JSON value.

/********************************** Prototypes *********************************/

bool json_utils_check(json_t const* json, int type);

json_t const*  json_utils_str_to_json(char *json_str, int max_fields);

int json_utils_get_int_element_value( json_t const *element_json, char *element_required );

char* json_utils_print_minified_string(json_t const* json, char* buffer);

char const* json_utils_get_value_from_path(json_t const* json, char const * json_path, ...);

jsonType_t json_utils_get_type_from_path(json_t const* json, char const * json_path, ...);

bool json_utils_set_value_from_path(char* json_string, size_t json_string_max_size, char* new_value, jsonType_t new_value_type, char const * json_path, ...);

#endif /* JSON_UTILS_H_ */

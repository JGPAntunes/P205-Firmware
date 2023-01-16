/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  json_utils.c
 * @brief This file has the JSON utilities implementation.
 */

/********************************** Includes ***********************************/

/* Interface */
#include "sense_library/utils/json_utils.h"

/* C standard library */
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Utilities */
#include "sense_library/utils/json.h"
#include "sense_library/utils/debug.h"

/********************************** Private ************************************/

#define check( x ) (!(x)) ? false : true      //< Boolean verification of an object

/********************************** Public *************************************/

/**
 * Check a JSON object and if is type is the type expected.
 * @param[in] json   JSON object to check.
 * @param[in] type   JSON string to be convertedtype that is expected to the JSON object.
 * @return true if JSON is valid, false if it is not.
 */
bool json_utils_check(json_t const* json, int type) {
  if(check(json) && check(type == json_getType(json))) {
    return true;    
  }
  return false;
}

/**
 * Creates a JSON object by a JSON string.
 * @param[in] json_str   JSON string to be converted.
 * @return The JSON object created by the JSON string, NULL if an case of an error occurred.
 */
json_t const*  json_utils_str_to_json(char *json_str, int max_fields) {
  json_t pool[max_fields];
  unsigned const qty = sizeof pool / sizeof *pool;
  json_t const* json;

  json = json_create(json_str, pool, qty);
  if(json_utils_check(json,JSON_OBJ)) {
    return json;
  }
  return NULL;
}

/**
 * Get an integer value of a known JSON object.
 * @param[in] element_json   JSON object.
 * @param[in] element_required name of the element required.
 * @return The integer value of the element required of the JSON object, 0 as a default.
 */
int json_utils_get_int_element_value( json_t const *element_json, char *element_required ) {
  json_t const *json;

  json = json_getProperty(element_json,element_required);
  if(json_utils_check(json,JSON_INTEGER)) {
    return json_getInteger(json);
  }
  return -1;
}

/**
 * Prints out the JSON object to a string in the minified form.
 * @param[in] json   JSON object to be printed out.
 * @param[in] buffer String where JSON object will be printed out.
 * @return Pointer to buffer in the case we want to print it out in the debug log.
 */
char* json_utils_print_minified_string(json_t const *json, char *buffer) {
    jsonType_t const type = json_getType(json);
    if (type != JSON_OBJ && type != JSON_ARRAY) {
        debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[json_utils_get_string] error!");
        return buffer;
    }

    strcpy(buffer, type == JSON_OBJ ? "{": "[");
    buffer += strlen(type == JSON_OBJ ? "{": "[");
//    debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "%s", type == JSON_OBJ ? "{": "[");

    json_t const* child;
    for (child = json_getChild(json); child != 0; child = json_getSibling(child)) {

        jsonType_t propertyType = json_getType(child);
        char const* name = json_getName(child);
        if (name) {
            strcpy(buffer, "\"");
            buffer += strlen("\"");

            strcpy(buffer, name);
            buffer += strlen(name);

            strcpy(buffer, "\":");
            buffer += strlen("\":");

//            debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "\"%s\":", name);
        }

        if (propertyType == JSON_OBJ || propertyType == JSON_ARRAY) {
        	buffer = json_utils_print_minified_string(child, buffer);
            if(json_getSibling(child) != 0) {
                strcpy(buffer, ",");
                buffer += strlen(",");
//                debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "%s", ",");
            }
        }

        else {
            char const* value = json_getValue(child);
            if (value) {
                bool const text = JSON_TEXT == json_getType(child);

                if(text) {
                    strcpy(buffer, "\"");
                    buffer += strlen("\"");

                    strcpy(buffer, value);
                    buffer += strlen(value);

                    strcpy(buffer, "\"");
                    buffer += strlen("\"");
                }
                else {
                    strcpy(buffer, value);
                    buffer += strlen(value);
                }
//                debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) fmt, value);

                bool const last = !json_getSibling(child);
                if (!last) {
                    strcpy(buffer, ",");
                    buffer += strlen(",");
//                    debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "%s", ",");
                }
            }
        }
    }

    strcpy(buffer, type == JSON_OBJ ? "}": "]");
    buffer += strlen(type == JSON_OBJ ? "}": "]");
//    debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "%s", type == JSON_OBJ ? "}": "]");

    return buffer;
}

/**
 * Gets value from JSON object.
 * @param[in] json      JSON to be searched.
 * @param[in] json_path Path to the parameter we want to fetch.
 * @return Value inside parameter.
 * @note When calling function end json_path with a NULL argument so that internal function
 * knows where to stop (e.g. "json_utils_get_value_from_path(json, "baromteter", "offset", NULL)").
 */
char const* json_utils_get_value_from_path(json_t const *json, char const *json_path, ...) {
    if (json == 0) {
        return NULL;
    }

    va_list args;
    va_start(args, json_path);

    while (json_path != NULL) {
    	json = json_getProperty(json, json_path);

        if (json == 0) {
            va_end(args);
            return NULL;
        }

        json_path = va_arg(args, char const*);
    }

    va_end(args);
    return json_getValue(json);
}

/**
 * Gets type from JSON object.
 * @param[in] json      JSON to be searched.
 * @param[in] json_path Path to the parameter we want to fetch.
 * @return JSON parameter's type.
 * @note When calling function end json_path with a NULL argument so that internal function
 * knows where to stop (e.g. "json_utils_get_type_from_path(json, "baromteter", "offset", NULL)").
 */
jsonType_t json_utils_get_type_from_path(json_t const *json, char const *json_path, ...) {
    if (json == 0) {
        return JSON_NULL;
    }

    va_list args;
    va_start(args, json_path);

    while (json_path != NULL) {
    	json = json_getProperty(json, json_path);

        if (json == 0) {
            va_end(args);
            return JSON_NULL;
        }

        json_path = va_arg(args, char const*);
    }

    va_end(args);
    return json_getType(json);
}

/**
 * Sets value inside JSON object.
 * @param[in] json_string          JSON to be set.
 * @param[in] json_string_max_size JSON's string maximum size to write down.
 * @param[in] new_value            New value to be set.
 * @param[in] new_value_type       JSON parameter's type.
 * @param[in] json_path            Path to the parameter we want to fetch.
 * @return JSON parameter's type.
 * @note When calling function end json_path with a NULL argument so that internal function
 * knows where to stop (e.g. "json_utils_get_type_from_path(json, "baromteter", "offset", NULL)").
 */
bool json_utils_set_value_from_path(char* json_string, size_t json_string_max_size, char* new_value, jsonType_t new_value_type_arg, char const * json_path, ...) {
  bool ret = false;

  if (strlen(json_string) == 0) {
    return ret;
  }

  int new_value_type = new_value_type_arg;

  if((new_value_type != JSON_INTEGER) && (new_value_type != JSON_REAL) && (new_value_type != JSON_BOOLEAN) && (new_value_type != JSON_TEXT)) {
    return ret;
  }

  char *string_starting_write_pointer = json_string;

  va_list args;
  va_start(args, json_path);

  while (json_path != NULL) {
    string_starting_write_pointer = strstr(string_starting_write_pointer, json_path);
    json_path = va_arg(args, char const*);
  }

  va_end(args);

  if((new_value_type == JSON_INTEGER) || (new_value_type == JSON_REAL) || (new_value_type == JSON_BOOLEAN)) {
    string_starting_write_pointer = strstr(string_starting_write_pointer, ":");
    string_starting_write_pointer++;
    
    ret = true;
  }
  else if (new_value_type == JSON_TEXT) {
    string_starting_write_pointer = strstr(string_starting_write_pointer, ":");
    string_starting_write_pointer = strstr(string_starting_write_pointer, "\"");
    string_starting_write_pointer++;
          
    ret = true;
  }

  if(ret) {
    if (string_starting_write_pointer != NULL) {
      /*char* string_ending_write_pointer = string_starting_write_pointer;
      if(strstr(string_ending_write_pointer, "}") == NULL) {
        string_ending_write_pointer = strstr(string_ending_write_pointer, "}");
      }
      else {
        string_ending_write_pointer = strstr(string_ending_write_pointer, ",");
      }*/

      char* string_ending_write_pointer = string_starting_write_pointer;
      char* aux_1 = strstr(string_ending_write_pointer, ",");
      char* aux_2 = strstr(string_ending_write_pointer, "}");
      if(aux_1 > aux_2) {
              string_ending_write_pointer = strstr(string_ending_write_pointer, "}");
      }
      else {
              string_ending_write_pointer = strstr(string_ending_write_pointer, ",");
      }
      
      size_t aux_size = strlen(string_ending_write_pointer);
      char aux_string[aux_size];
      memset(aux_string, '\0', aux_size);
      strcpy(aux_string, string_ending_write_pointer);

      /* Check final size before copy data */
      string_starting_write_pointer[0] = '\0';
      if((strlen(json_string) + strlen(new_value) + strlen(aux_string)) > json_string_max_size) {
        ret = false;
      }

      if(ret) {
        strcpy(string_starting_write_pointer, new_value);
        strcpy((string_starting_write_pointer + strlen(new_value)), aux_string);
        memset((string_starting_write_pointer + strlen(new_value) + strlen(aux_string)), '\0', json_string_max_size - strlen(json_string));
      }
    }
    else {
      ret = false;
    }
  }

  return ret;
}

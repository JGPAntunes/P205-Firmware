/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  json.h
 * @brief This file has the JSON implementation.
 * @note Licensed under the MIT License (http://opensource.org/licenses/MIT).
 * See https://github.com/rafagafe/tiny-json.
 */

#ifndef JSON_H_
#define	JSON_H_

/********************************** Includes ***********************************/

#ifdef __cplusplus
extern "C" {
#endif

/* C standard library */
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/********************************** Definitions ********************************/

#define json_containerOf( ptr, type, member ) \
  ((type*)( (char*)ptr - offsetof( type, member ) ))

/**
 * Enumeration of codes of supported JSON properties types.
 */
typedef enum {
  JSON_OBJ, JSON_ARRAY, JSON_TEXT, JSON_BOOLEAN,
  JSON_INTEGER, JSON_REAL, JSON_NULL
} jsonType_t;

/**
 * Structure to handle JSON properties. 
 */
typedef struct json_s {
  struct json_s* sibling;
  char const* name;
  union {
    char const* value;
    struct {
      struct json_s* child;
      struct json_s* last_child;
    } c;
  } u;
  jsonType_t type;
} json_t;

/** 
 * Structure to handle a heap of JSON properties.
 */
typedef struct jsonPool_s jsonPool_t;
struct jsonPool_s {
  json_t* (*init)( jsonPool_t* pool );
  json_t* (*alloc)( jsonPool_t* pool );
};

/** 
 * Get the name of a json property.
 * @param json A valid handler of a json property.
 * @retval Pointer to null-terminated if property has name.
 * @retval Null pointer if the property is unnamed. 
 */
static inline char const* json_getName( json_t const* json ) {
  return json->name;
}

/** 
 * Get the value of a json property.
 * The type of property cannot be JSON_OBJ or JSON_ARRAY.
 * @param json A valid handler of a json property.
 * @return Pointer to null-terminated string with the value. 
 */
static inline char const* json_getValue( json_t const* property ) {
  return property->u.value;
}

/**
 * Get the type of a json property.
 * @param json A valid handler of a json property.
 * @return The code of type.
 */
static inline jsonType_t json_getType( json_t const* json ) {
  return json->type;
}

/** 
 * Get the next sibling of a JSON property that is within a JSON object or array.
 * @param json A valid handler of a json property.
 * @retval The handler of the next sibling if found.
 * @retval Null pointer if the json property is the last one. 
 */
static inline json_t const* json_getSibling( json_t const* json ) {
  return json->sibling;
}

/** 
 * Get the first property of a JSON object or array.
 * @param json A valid handler of a json property.
 * Its type must be JSON_OBJ or JSON_ARRAY.
 * @retval The handler of the first property if there is.
 * @retval Null pointer if the json object has not properties.
  */
static inline json_t const* json_getChild( json_t const* json ) {
  return json->u.c.child;
}

/**
 * Get the value of a json boolean property.
 * @param property A valid handler of a json object. Its type must be JSON_BOOLEAN.
 * @return The value stdbool. 
 */
static inline bool json_getBoolean( json_t const* property ) {
  return *property->u.value == 't';
}

/** 
 * Get the value of a json integer property.
 * @param property A valid handler of a json object. Its type must be JSON_INTEGER.
 * @return The value stdint. 
 */
static inline int64_t json_getInteger( json_t const* property ) {
  return atoll( property->u.value );
}

/** 
 * Get the value of a json real property.
 * @param property A valid handler of a json object. Its type must be JSON_REAL.
 * @return The value. 
 */
static inline double json_getReal( json_t const* property ) {
  return atof( property->u.value );
}

/********************************** Prototypes *********************************/

json_t const* json_create( char* str, json_t mem[], unsigned int qty );

json_t const* json_getProperty( json_t const* obj, char const* property );

char const* json_getPropertyValue( json_t const* obj, char const* property );

json_t const* json_createWithPool( char* str, jsonPool_t* pool );

#ifdef __cplusplus
}
#endif

#endif	/* JSON_H_ */

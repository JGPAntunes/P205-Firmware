/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  base64.h
 * @brief Base64 utils header.
 */

#ifndef BASE64_H
#define BASE64_H

/********************************** Includes ***********************************/

/* C standard library */
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

/* Libs */
#include "debug.h"

/********************************** Definitions ********************************/

/**
 * Helper macro that computes the length of the resulting Base64
 * string when the data to be encoded has length (x).
 */
#define BASE64_LENGTH(x) ((x % 3) != 0 ? (((x + (3 - (x % 3))) / 3) * 4) : ((x / 3) * 4))

/**
 * Helper macro that computes the length of the resulting binary
 * string when the data to be decoded has length (x)
 */
#define BINARY_LENGTH(x) (((x) * 3) / 4)

/********************************** Prototypes *********************************/

void base64_encode(const void *input, size_t input_length, char *output, size_t *output_lenght);

void base64_decode(const char *input, size_t input_lenght, unsigned char *output, size_t *output_lenght);

bool base64_is_valid(const char *input, size_t input_lenght);

bool base64_string_to_hex(char* string, char* hexstring, int hexstring_size);

bool base64_hex_to_string(char* hexstring, char* string, int string_size);

#endif /* BASE64_H */

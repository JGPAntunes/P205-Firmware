/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file utils.h
 * @brief This file has the utilities header.
 */

#ifndef UTILS_H
#define UTILS_H

/********************************** Includes ***********************************/

/* C standard library */
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

/********************************** Definitions ********************************/

#define ARRAY_LEN(x) ((sizeof(x)) / sizeof((x)[0]))     ///< Calculates array length.

#define RANDOM_MAX (255)                                ///< Random maximum value.

/********************************** Prototypes *********************************/

char* utils_ftoa(char *s, float n, float precision);

int utils_cmp_64_with_array(uint64_t u64, uint8_t *array);

uint64_t utils_get_serial_from_array(uint8_t *array);

void utils_save_uint64_t_to_array(uint8_t *buffer, uint64_t u64);

void utils_save_uint64_t_to_array_keep_endianness(uint8_t *buffer, uint64_t u64);

void utils_invert_endianness(void *data, size_t size);

uint64_t utils_atoll(char *str);

int utils_locate_subtring(char src[], char str[]);

void utils_reverse(char str[], int len);

char* utils_itoa(int num, char* str, int base);

uint16_t utils_get_uint16_from_array(uint8_t *array);

void utils_save_uint16_t_to_array(uint8_t *message, uint16_t u16);

void utils_save_uint32_t_to_array(uint8_t *message, uint32_t u32);

uint16_t utils_get_uint16_from_uint32_t(uint32_t u32);

int utils_readline(const char *input, char *buffer);

uint64_t utils_get_uint64_from_string(const char *value);

uint64_t utils_get_int64_from_string(const char *value);

uint8_t utils_get_uint8_from_string(const char *value);

uint16_t utils_get_uint16_from_string(const char *value);

uint32_t utils_get_uint32_from_string(const char *value);

uint8_t utils_atoi(char str);

uint64_t utils_endian_swap_u64(uint64_t u);

uint16_t utils_endian_swap_u16(uint16_t u);

uint32_t utils_get_uint32_from_array(uint8_t * array);

int utils_cmp_func(const void *a, const void *b);

int8_t utils_stats_get_mode(int8_t *numbers, int size);

int8_t utils_stats_get_mean(int8_t *numbers, int size);

int8_t utils_stats_get_median(int8_t *numbers, int size);

uint8_t utils_atoi(char str);

uint64_t utils_random_beetween(uint64_t min, uint64_t max);

int8_t utils_compute_percent(float value, float v_0_percent, float v_100_percent);

void utils_strtok_n_elems(char *str,char *delimiter, int *n_elems, char elems_strs[][30]);

#endif /* UTILS_H */

/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file utils.c
 * @brief This file has the utilities functionality.
 */

/********************************** Includes ***********************************/

/* Interface */
#include "utils.h"

/* C standard library */
#include <math.h> 
#include <stdio.h>

/* Externs */
#include "externs.h"

/********************************** Private ************************************/



/********************************** Public *************************************/

/**
 * Converts a floating point number to a string.
 * @param[in] s         String to write down the float.
 * @param[in] n         Float value to be write.
 * @param[in] precision Precision that ww need in the conversion (e.g. 0.001).
 * @return Pointer to s.
 */
char* utils_ftoa(char *s, float n, float precision) {
  /* Handle special cases */
  if (isnan(n)) {
    strcpy(s, "nan");
  } else if (isinf(n)) {
    strcpy(s, "inf");
  } else if (n == 0.0) {
    strcpy(s, "0");
  } else {
    int digit, m, m1;
    char *c = s;
    int neg = (n < 0);
    if (neg) {
      n = -n;
    }

    /* Calculate magnitude */
    m = log10(n);
    int useExp = (m >= 14 || (neg && m >= 9) || m <= -9);
    if (neg) {
      *(c++) = '-';
    }

    /* Set up for scientific notation */
    if (useExp) {
      if (m < 0) {
        m -= 1.0;
      }
      n = n / pow(10.0, m);
      m1 = m;
      m = 0;
    }
    if (m < 1.0) {
      m = 0;
    }

    /* Convert the number */
    while (n > precision || m >= 0) {
      double weight = pow(10.0, m);
      if (weight > 0 && !isinf(weight)) {
        digit = floor(n / weight);
        n -= (digit * weight);
        *(c++) = '0' + digit;
      }
      if (m == 0 && n > 0) {
        *(c++) = '.';
      }
      m--;
    }
      
    if (useExp) {
      /* Convert the exponent */
      int i, j;
      *(c++) = 'e';
      if (m1 > 0) {
        *(c++) = '+';
      } else {
        *(c++) = '-';
        m1 = -m1;
      }
      m = 0;
      while (m1 > 0) {
        *(c++) = '0' + m1 % 10;
        m1 /= 10;
        m++;
      }
      c -= m;
      for (i = 0, j = m-1; i<j; i++, j--) {
        /* Swap without temporary */
        c[i] ^= c[j];
        c[j] ^= c[i];
        c[i] ^= c[j];
      }
      c += m;
    }
    *(c) = '\0';
  }
  return s;
}

/**
 * Compares number with array.
 * @param[in] u64   Unsigned 64 bits number.
 * @param[in] array Array that contains a number.
 * @return 0 if they aren't equal, other value otherwise.
 */
int utils_cmp_64_with_array(uint64_t u64, uint8_t *array) {
  uint64_t dst_64 = 0;

  dst_64  = ((uint64_t) array[0])  << 56;
  dst_64 |= ((uint64_t) array[1]) << 48;
  dst_64 |= ((uint64_t) array[2]) << 40;
  dst_64 |= ((uint64_t) array[3]) << 32;
  dst_64 |= ((uint64_t) array[4]) << 24;
  dst_64 |= ((uint64_t) array[5]) << 16;
  dst_64 |= ((uint64_t) array[6]) << 8;
  dst_64 |= ((uint64_t) array[7]);

  return ((dst_64 - u64) == 0);
}

/**
 * Converts to serial number to a 64 bit integer.
 * @param[in] array Buffer containing the serial number.
 * @return The serial number converted in an integer.
 */
uint64_t utils_get_serial_from_array(uint8_t *array) {
	uint64_t serial = 0 ;
  serial  = ((uint64_t) array[0]) << 56;
	serial |= ((uint64_t) array[1]) << 48;
	serial |= ((uint64_t) array[2]) << 40;
	serial |= ((uint64_t) array[3]) << 32;
  serial |= ((uint64_t) array[4]) << 24;
  serial |= ((uint64_t) array[5]) << 16;
  serial |= ((uint64_t) array[6]) << 8;
  serial |= ((uint64_t) array[7]);

  return serial;
}

/**
 * Save unsigned 64 bit integer to buffer.
 * @param[in] buffer
 * @param[in] u64    Number to saved.
 */
void utils_save_uint64_t_to_array(uint8_t *buffer, uint64_t u64) {
  size_t i;
  uint8_t *ptr = (uint8_t *) &u64;

  /* The u64 always starts @ offset = 9 */
  for (i =0; i < (sizeof(uint64_t) - 1); i++) {
    buffer[i] = *ptr++;
  }
}

/**
 * Save unsigned endianness 64 bit integer to buffer.
 * @param[in] buffer
 * @param[in] u64    Number to saved.
 */
void utils_save_uint64_t_to_array_keep_endianness(uint8_t *buffer, uint64_t u64) {
  int i;
  uint8_t *ptr = (uint8_t *) &u64;

  /* The u64 always starts @ offset = 9 */
  for (i = (sizeof(uint64_t) - 1); i >= 0; i--) {
    buffer[i] = *ptr++;
  }
}

/**
 * Generic function that inverts the endianness of a sequence
 * of bytes.
 * @param[in] data Pointer to the data for which endianness is to be inverted.
 * @param[in] size The size of the data for which endianness is to be inverted.
 */
void utils_invert_endianness(void *data, size_t size) {
  size_t i;
  size_t j;
  uint8_t tmp;
  uint8_t *ptr = (uint8_t *) data;

  for (i = 0; i < size / 2; ++i) {
    /* Compute swap index */
    j = size - i -1;

    /* Swap */
    tmp = ptr[i];

    ptr[i] = ptr[j];
    ptr[j] = tmp;
  }
}

/**
 * Converts string to a long long number.
 * @param[in] str String that contains the number to be converted.
 * @return The converted number
 */
uint64_t utils_atoll(char *str) {
  long long retval;
  retval = 0;

  for (; *str; str++) {
    if(*str < '0' || *str > '9') {
      break;
    }
    retval = 10 * retval + (*str - '0');
  }

  return retval;
}

/**
 * Utility function. Returns the first position of the subtring in the main string.
 * @param[in] src String to be scanned.
 * @param[in] str The substring.
 * @return The first position of str in src string or -1 otherwise.
 */
int utils_locate_subtring(char src[], char str[]) {
  int i, j, first_occurence;
  i = 0, j = 0;

  while (src[i] != '\0') {
    while (src[i] != str[0] && src[i] != '\0') {
      i++;
    }
    if (src[i] == '\0') {
      return (-1);
    }
    first_occurence = i;
    while (src[i] == str[j] && src[i] != '\0' && str[j] != '\0') {
      i++;
      j++;
    }
    if (str[j] == '\0') {
      return (first_occurence);
    }
    if (src[i] == '\0') {
      return (-1);
    }
    i = first_occurence + 1;
    j = 0;
  }
  return (-1);
}

/**
 * Reverse char buffer.
 * @param[in] str Buffer to be reversed.
 * @param[in] len Buffer's lenght.
 */
void utils_reverse(char str[], int len) {
  int start, end;
  char temp;

  for(start = 0, end = len - 1; start < end; start++, end--) {
    temp = *(str + start);
    *(str + start) = *(str + end);
    *(str + end) = temp;
  }
}

/**
 * Converts interger to string. Works for signed numbers also.
 * @param[in] num Number to be converted to string.
 * @param[in] str String buffer.
 * @param[in] base Base to be used (e.g. 10 for decimal numbers).
 */
char* utils_itoa(int num, char* str, int base) {
  int i = 0;
  bool is_negative = false;

  /* A zero is same "0" string in all base */
  if (num == 0) {
    str[i] = '0';
    str[i + 1] = '\0';
    return str;
  }

  /* Negative numbers are only handled if base is 10
   * otherwise considered unsigned number */
  if (num < 0 && base == 10) {
    is_negative = true;
    num = -num;
  }

  while (num != 0) {
    int rem = num % base;
    str[i++] = (rem > 9)? (rem-10) + 'A' : rem + '0';
    num = num/base;
  }

  /* Append negative sign for negative numbers */
  if (is_negative){
    str[i++] = '-';
  }

  str[i] = '\0';

  utils_reverse(str, i);

  return str;
}

/**
 * Gets 16-bit unsigned integer from a bit array.
 * @param[in] array Buffer that contains the 16-bit integer.
 * @return 16-bit integer inside array.
 */
uint16_t utils_get_uint16_from_array(uint8_t *array) {
	uint16_t u16 = 0 ;
  u16 |= ((uint16_t) array[0]) << 8;
  u16 |= ((uint16_t) array[1]);
  return u16;
}

/**
 * Put 16-bit unsigned integer inside a bit array.
 * @param[in] message Buffer that will store the 16-bit integer.
 * @param[in] u16   16-bit unsigned integer.
 */
void utils_save_uint16_t_to_array(uint8_t *message, uint16_t u16) {
  int i;
  uint8_t *ptr = (uint8_t *)&u16;
  for (i = (sizeof(uint16_t) - 1); i >= 0 ; i--) {
    message[i] = *ptr++;
  }
}

/**
 * Put 32-bit unsigned integer inside a bit array.
 * @param[in] message Buffer that will store the 32-bit integer.
 * @param[in] u32   32-bit unsigned integer.
 */
void utils_save_uint32_t_to_array(uint8_t *message, uint32_t u32) {
  int i;
  uint8_t *ptr = (uint8_t *)&u32;
  for (i = (sizeof(uint32_t) - 1); i >= 0; i--) {
    message[i] = *ptr++;
  }
}

/**
 * Gets a 16-bit unsigned integer from a 32-bit unsigned integer.
 * @param[in] u32   32-bit unsigned integer.
 * @return 16-bit unsigned integer.
 */
uint16_t utils_get_uint16_from_uint32_t(uint32_t u32) {
  /* "copy" uint32_t big bytes, assuming little endian */
  uint16_t u16 = 0;

  /* Pointer to u32 big byte */
  uint8_t *ptr_32 = (uint8_t*) &u32;

  /* Pointer to u16 big byte */
  uint8_t *ptr_16 = (uint8_t*) &u16;

  /* Copy first byte */
  *ptr_16 = *ptr_32;
  ptr_32++;
  ptr_16++;
  
  /* Copy second byte */
  *ptr_16 = *ptr_32;

  return u16;
}

/**
 * Splits an input string into lines.
 * @param[in] input Pointer to the input string. On the first call, this function
 * expects a valid pointer to a string. On subsequent calls, this
 * function expects a NULL pointer.
 * @param[in] buffer Output buffer for the line.
 * @return A boolean value indicating whether a new line was read.
 */
int utils_readline(const char *input, char *buffer) {
  static const char *p = NULL;

  /* Check whether we have a new string to be processed */
  if (input != NULL) {
    p = input;
  }

  /* No string to be processed */
  if (p == NULL) {
    return 0;
  }

  /* Skip non-printable characters */
  while (*p != '\0' && !isprint((int) *p)) {
    ++p;
  }

  /* Check whether we've reached the end of the string */
  if (*p == '\0') {
    p = NULL;
    return 0;
  }

  /* Copy printable characters to the buffer */
  while (*p != '\0' && isprint((int) *p)) {
    *buffer++ = *p++;
  }

  /* Finish string and return line */
  *buffer = '\0';
  
  return 1;
}

/**
 * Gets number from a string.
 * @param[in] value String that contains the number.
 * @return The 64-bit unsigned integer.
 */
uint64_t utils_get_uint64_from_string(const char *value) {
  return strtoull(value, NULL, 10);
}

/**
 * Gets number from a string.
 * @param[in] value String that contains the number.
 * @return The 64-bit signed integer.
 */
uint64_t utils_get_int64_from_string(const char *value) {
  return strtoll(value, NULL, 10);
}

/**
 * Gets number from a string.
 * @param[in] value String that contains the number.
 * @return The 8-bit unsigned integer.
 */
uint8_t utils_get_uint8_from_string(const char *value) {
  return strtoull(value, NULL, 10);
}

/**
 * Gets number from a string.
 * @param[in] value String that contains the number.
 * @return The 16-bit unsigned integer.
 */
uint16_t utils_get_uint16_from_string(const char *value) {
  return strtoull(value, NULL, 10);
}

/**
 * Gets number from a string.
 * @param[in] value String that contains the number.
 * @return The 16-bit unsigned integer.
 */
uint32_t utils_get_uint32_from_string(const char *value) {
  return strtoull(value, NULL, 10);
}

/**
 * Gets number from a string.
 * @param[in] str String that contains the number.
 * @return The 8-bit unsigned integer.
 */
uint8_t utils_atoi(char str) {
  if (str < '0' || str > '9') {
    return 0;
  } else {
    return (str - '0');
  }
}

/**
 * Return copy of a 16-bit unsigned integer argument with byte order reversed.
 * @param[in] u Number to be reversed.
 * @return Reversed number.
 */
uint16_t utils_endian_swap_u16(uint16_t u){
  uint16_t tmp = 0;

  tmp |= ((u >> 8) & (0xffull <<  0));
  tmp |= ((u << 8) & (0xffull <<  8));

  return tmp;
}

/**
 * Return copy of a 64-bit unsigned integer argument with byte order reversed.
 * @param[in] u Number to be reversed.
 * @return Reversed number.
 */
uint64_t utils_endian_swap_u64(uint64_t u) {
  uint64_t tmp = 0;

  tmp |= ((u >> 56) & (0xffull <<  0));
  tmp |= ((u >> 40) & (0xffull <<  8));
  tmp |= ((u >> 24) & (0xffull << 16));
  tmp |= ((u >>  8) & (0xffull << 24));
  tmp |= ((u <<  8) & (0xffull << 32));
  tmp |= ((u << 24) & (0xffull << 40));
  tmp |= ((u << 40) & (0xffull << 48));
  tmp |= ((u << 56) & (0xffull << 56));

  return tmp;
}

/**
 * Gets number from a string.
 * @param[in] array String that contains the number.
 * @return The 32-bit unsigned integer.
 */
uint32_t utils_get_uint32_from_array(uint8_t *array) {
  uint32_t u32 = 0;

  u32 |= ((uint32_t) array[0]) << 24;
  u32 |= ((uint32_t) array[1]) << 16;
  u32 |= ((uint32_t) array[2]) << 8;
  u32 |= ((uint32_t) array[3]);

  return u32;
}

/**
 * Compare values.
 * @param[in] a Content to be compared.
 * @param[in] b Content to be compared.
 * @return False if they are the same, true otherwise.
 */
int utils_cmp_func(const void *a, const void *b) {
  return (int) (*(int8_t*)a - *(int8_t*)b);
}

/**
 * Get mode value.
 * @param[in] numbers Numbers to do the math.
 * @param[in] size Number of samples collected.
 * @return The mode value.
 */
int8_t utils_stats_get_mode(int8_t *numbers, int size) {
  if(!(size > 0))return 0;
  if(!(numbers != NULL)) return 0;

  if (size == 1) {
    return *numbers;
  }

  qsort(numbers, size, sizeof(int8_t), utils_cmp_func);

  int i, count, max_count, mode;

  for (i = 1, count = 1, max_count = 0, mode = numbers[0]; i < size; ++i) {
    if (numbers[i - 1] != numbers[i]) {

    if (max_count < count) {
      mode = numbers[i - 1];
      max_count = count;
    }
    count = 1;
    } else {
      count++;
    }
  }
  
  if (max_count < count) {
    mode = numbers[i - 1];
  }

  return mode;
}

/**
 * Get mean value.
 * @param[in] numbers Numbers to do the math.
 * @param[in] size Number of samples collected.
 * @return The mean value.
 */
int8_t utils_stats_get_mean(int8_t *numbers, int size) {
  if(!(size > 0)) return 0;
  if(!(numbers != NULL))return 0;

  if (size == 1) {
    return *numbers;
  }

  int accum = 0;

  for (int i = 0; i < size; ++i) {
    accum += numbers[i];
  }

  return accum / size;
}

/**
 * Get median value.
 * @param[in] numbers Numbers to do the math.
 * @param[in] size Number of samples collected.
 * @return The median value.
 */
int8_t utils_stats_get_median(int8_t *numbers, int size) {
  if(!(size > 0)) return 0;
  if(!(numbers != NULL)) return 0;

  if (size == 1) {
    return *numbers;
  }

  int8_t numbers_aux[size];
  memcpy(numbers_aux, numbers, size);
  qsort(numbers_aux, size, sizeof(int8_t), utils_cmp_func);

  /* If the number of elements is even */
  if(!(size & 0b1)){
    /* Average of both middle values*/
    return (numbers_aux[size / 2] + numbers_aux[(size / 2) - 1] / 2);
  }
  else{
    /* Return middle value */
    return numbers_aux[size / 2];
  }
}

/**
 * Get random number between two numbers.
 * @param[in] min
 * @param[in] max
 * @return The random value.
 */
uint64_t utils_random_beetween(uint64_t min, uint64_t max) {
  /* Pseudo random number */
  if(min >= max) return min;
  uint64_t interval= max-min;
  if(interval < RANDOM_MAX) return utils_random_generate();

  uint64_t n_sums = interval/RANDOM_MAX;

  return utils_random_generate() * n_sums;
}

/**
 * Gets percentage.
 * @param[in] value         Value that we want to calculate the percentage.
 * @param[in] v_0_percent   0% percentage value.
 * @param[in] v_100_percent 100% percentage value.
 * @return The calculated percentage.
 */
int8_t utils_compute_percent(float value, float v_0_percent, float v_100_percent) {
  /* If value exceed min or Max return */
  if(value < v_0_percent) {
    return 0;
  }
  else if (value > v_100_percent) {
    return 100;
  }
    
  if(v_0_percent >= v_100_percent) {
    return -1;
  }

  /* Compute line parameters */
  float a = 0 - 100;
  float b = v_100_percent - v_0_percent;
  float c = ((v_0_percent - v_100_percent)* 0) + 100 * v_0_percent;

  /* Compute value respective percentage */
  float y = (-c - (a * value)) / b;

  /* Check percentage value */
  if(y < 0) {
    return 0;
  }
  if(y > 100) {
    return 100;
  }

  /* Return percentage */
  return y;
}

/**
 * Parse the entire string to a set of sub strings delimited by the delimiter string.
 * @param[in] str         String to get tokens.
 * @param[in] delimiter   Character or string delimiter.
 * @param[in] n_elems     Number of tokens found.
 * @param[in] elems_strs  Set of strings with the tokens found.
 * @return void.
 */
void utils_strtok_n_elems(char *str,char *delimiter, int *n_elems, char elems_strs[][30]) {
  char *token;
  int dim = 0;

  /* get the first token */
  token = strtok(str, delimiter);

  /* walk through other tokens */
  while( token != NULL ) {
    strcpy(elems_strs[dim++],token);
    token = strtok(NULL, delimiter);
  }
  *n_elems = dim;
}

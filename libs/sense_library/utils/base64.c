/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  base64.c
 * @brief Base64 utils. Used when uploading info to our cloud platform.
 */

/********************************** Includes ***********************************/

#include "base64.h"

/* Utils */
#include "externs.h"

/********************************** Private ************************************/

/**
 * Holds base64 encoding table.
 */
static const char *_BASE64_TABLE =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/********************************** Public ************************************/

/**
 * Single base64 character conversion.
 */
static int POS(char c) {
	if (c >= 'A' && c <= 'Z')
		return c - 'A';
	if (c >= 'a' && c <= 'z')
		return c - 'a' + 26;
	if (c >= '0' && c <= '9')
		return c - '0' + 52;
	if (c == '+')
		return 62;
	if (c == '/')
		return 63;
	if (c == '=')
		return -1;
	return -2;
}

/**
 * Converts hexadecimal to binary.
 * param[in] string
 * @return Converted binary.
 */
int hex_to_bin(const char *string) {
	int ret = 0;
	int i;
	for (i = 0; i < 2; i++) {
		char c = *string++;
		int n = 0;
		if ('0' <= c && c <= '9')
			n = c - '0';
		else if ('a' <= c && c <= 'f')
			n = 10 + c - 'a';
		else if ('A' <= c && c <= 'F')
			n = 10 + c - 'A';
		ret = n + ret * 16;
	}
	return ret;
}

/********************************** Public ************************************/

/**
 * Encodes a sequence of bytes with the provided length as a
 * base64 string.
 * @param[in] input Pointer to the data to be encoded.
 * @param[in] input_length The length of the data to be encoded, in number of bytes.
 * @param[in] output Pointer to an output array of characters where the resulting
 * Base64 string is written. The resulting string is
 * NULL-terminated. The size of the output buffer shall be at
 * least (BASE64_LENGTH(length) + 1).
 * @param[in] output_lenght Encoded data length.
 */
void base64_encode(const void *input, size_t input_length, char *output,
		size_t *output_lenght) {
	int ch;
	unsigned int i;
	const unsigned char *in = (const unsigned char *) input;
	char *temp_output = output;

	for (i = 0; i < input_length; i += 3) {
		ch = (in[i] & 0xFC) >> 2;
		*output++ = _BASE64_TABLE[ch];

		ch = (in[i] & 0x03) << 4;

		if ((i + 1) < input_length) {
			ch |= ((in[i + 1] & 0xF0) >> 4);
			*output++ = _BASE64_TABLE[ch];
			ch = (in[i + 1] & 0x0F) << 2;

			if ((i + 2) < input_length) {
				ch |= ((in[i + 2] & 0xC0) >> 6);
				*output++ = _BASE64_TABLE[ch];
				ch = in[i + 2] & 0x3F;
				*output++ = _BASE64_TABLE[ch];

			} else {
				*output++ = _BASE64_TABLE[ch];
				*output++ = '=';
			}

		} else {
			*output++ = _BASE64_TABLE[ch];
			*output++ = '=';
			*output++ = '=';
		}
	}

	/* Terminate string */
	*output = '\0';

	*output_lenght = output - temp_output;
}

/**
 * @brief Decodes a base64 string.
 * @param[in] input Data to be decoded.
 * @param[in] input_lenght Encoded data length.
 * @param[in] output Pointer to the output, i.e. the decoded data.
 * @param[in] output_lenght Decoded data length.
 */
void base64_decode(const char *input, size_t input_lenght,
		unsigned char *output, size_t *output_lenght) {
	const char *p;
	unsigned char *q;
	int n[4] = { 0, 0, 0, 0 };

	*output_lenght = 0;

	if (!base64_is_valid(input, input_lenght)) {
		return;
	}

	q = output;

	for (p = input; *p;) {
		n[0] = POS(*p++);
		n[1] = POS(*p++);
		n[2] = POS(*p++);
		n[3] = POS(*p++);

		if (n[0] == -2 || n[1] == -2 || n[2] == -2 || n[3] == -2) {
			return;
		}

		if (n[0] == -1 || n[1] == -1) {
			return;
		}

		if (n[2] == -1 && n[3] != -1) {
			return;
		}

		q[0] = (n[0] << 2) + (n[1] >> 4);

		if (n[2] != -1) {
			q[1] = ((n[1] & 15) << 4) + (n[2] >> 2);
		}

		if (n[3] != -1) {
			q[2] = ((n[2] & 3) << 6) + n[3];
		}

		q += 3;
	}

	*output_lenght = q - output - (n[2] == -1) - (n[3] == -1);

	return;
}

/**
 * @brief Decodes a base64 string.
 * @param[in] input Data to be decoded.
 * @param[in] input_lenght Encoded data length.
 * @return True in the case of a valid base64, otherwise false.
 */
bool base64_is_valid(const char *input, size_t input_lenght) {
	/* Size = 0 or size not multiple of 4 it's considered an
	 * invalid base64 input. Check also for a blank space here */
	if ((input_lenght == 0) || (input_lenght % 4 != 0)
			|| (strlen(input) < input_lenght)) {
		return false;
	}

	uint16_t ending_before_equal_char = input_lenght - 1;

	/* Look for a '=' char in the end of the base64 */
	if (input[ending_before_equal_char] == '=')
		ending_before_equal_char--;

	/* Look for a second '=' char in the end of the base64 */
	if (input[ending_before_equal_char] == '=')
		ending_before_equal_char--;

	/* Check if there is any invalid base64 char present */
	uint16_t i = 0;
	uint16_t j = 0;
	for (i = 0; i <= ending_before_equal_char; i++) {
		bool ret = false;
		/* Until strlen(_BASE64_TABLE) - 1 in to ignore the '=' char
		 that was previously scanned */
		for (j = 0; j < strlen(_BASE64_TABLE) - 1; j++) {
			if (input[i] == _BASE64_TABLE[j]) {
				ret = true;
			}
		}
		if (!ret) {
			return false;
		}
	}

	/* If I'm here all went quite good */
	return true;
}

/**
 * Convert base64 string to hex.
 * @param[in] string
 * @param[in] hexstring
 * @param[in] hexstring_size
 * @return False if some unexpected error happens, true otherwise.
 */
bool base64_string_to_hex(char* string, char* hexstring, int hexstring_size) {
	char ch;
	int i, j, len = 0;

	len = strlen(string);

	/* No space available.
	 * Count also with the '\0' in the end. */
	if (hexstring_size <= (len * 2)) {
		debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
		debug_print_string(DEBUG_LEVEL_1,
				(uint8_t*) "[base64_string_to_hex] error calculating base 64 hex\r\n");
		return false;
	}

	for (i = 0, j = 0; i < len; i++, j += 2) {
		ch = string[i];
#pragma GCC diagnostic ignored "-Wtype-limits"
		if (ch >= 0 && ch <= 0x0F) {
			hexstring[j] = 0x30;
#pragma GCC diagnostic ignored "-Wtype-limits"
			if (ch >= 0 && ch <= 9)
				hexstring[j + 1] = 0x30 + ch;
			else
				hexstring[j + 1] = 0x37 + ch;
		} else if (ch >= 0x10 && ch <= 0x1F) {
			hexstring[j] = 0x31;
			ch -= 0x10;
#pragma GCC diagnostic ignored "-Wtype-limits"
			if (ch >= 0 && ch <= 9)
				hexstring[j + 1] = 0x30 + ch;
			else
				hexstring[j + 1] = 0x37 + ch;
		} else if (ch >= 0x20 && ch <= 0x2F) {
			hexstring[j] = 0x32;
			ch -= 0x20;
#pragma GCC diagnostic ignored "-Wtype-limits"
			if (ch >= 0 && ch <= 9)
				hexstring[j + 1] = 0x30 + ch;
			else
				hexstring[j + 1] = 0x37 + ch;
		} else if (ch >= 0x30 && ch <= 0x3F) {
			hexstring[j] = 0x33;
			ch -= 0x30;
#pragma GCC diagnostic ignored "-Wtype-limits"
			if (ch >= 0 && ch <= 9)
				hexstring[j + 1] = 0x30 + ch;
			else
				hexstring[j + 1] = 0x37 + ch;
		} else if (ch >= 0x40 && ch <= 0x4F) {
			hexstring[j] = 0x34;
			ch -= 0x40;
#pragma GCC diagnostic ignored "-Wtype-limits"
			if (ch >= 0 && ch <= 9)
				hexstring[j + 1] = 0x30 + ch;
			else
				hexstring[j + 1] = 0x37 + ch;
		} else if (ch >= 0x50 && ch <= 0x5F) {
			hexstring[j] = 0x35;
			ch -= 0x50;
#pragma GCC diagnostic ignored "-Wtype-limits"
			if (ch >= 0 && ch <= 9)
				hexstring[j + 1] = 0x30 + ch;
			else
				hexstring[j + 1] = 0x37 + ch;
		} else if (ch >= 0x60 && ch <= 0x6F) {
			hexstring[j] = 0x36;
			ch -= 0x60;
#pragma GCC diagnostic ignored "-Wtype-limits"
			if (ch >= 0 && ch <= 9)
				hexstring[j + 1] = 0x30 + ch;
			else
				hexstring[j + 1] = 0x37 + ch;
#pragma GCC diagnostic ignored "-Wtype-limits"
		} else if (ch >= 0x70 && ch <= 0x7F) {
			hexstring[j] = 0x37;
			ch -= 0x70;

			if (ch >= 0 && ch <= 9)
				hexstring[j + 1] = 0x30 + ch;
			else
				hexstring[j + 1] = 0x37 + ch;
		}
	}
	hexstring[j] = 0x00;

	return true;
}

/**
 * Convert base64 hex to string.
 * @param[in] hexstring
 * @param[in] string
 * @param[in] string_size
 * @return False if some unexpected error happens, true otherwise.
 */
bool base64_hex_to_string(char* hexstring, char* string, int string_size) {
	const char *in = hexstring;
	int i, len = 0;

	len = strlen(hexstring);

	/* No space available.
	 * Count also with the '\0' in the end. */
	if (string_size <= (len / 2)) {
		debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
		debug_print_string(DEBUG_LEVEL_1,
				(uint8_t*) "[base64_hex_to_string] error calculating base 64 string\r\n");
		return false;
	}

	for (i = 0; i < len; i++) {
		string[i] = hex_to_bin(in);
		in += 2;
	}

	string[i] = 0x00;

	return true;
}

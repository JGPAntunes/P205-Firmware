/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  gps_utilities.c
 * @brief This file has the GPS utilities.
 */

/********************************** Libraries ***********************************/

/* Header file */
#include "sense_library/gps/gps_utilities.h"

/* Measurements utitity */
#include "sense_library/sensoroid/measurements_v2_manager.h"

/* Utils */
#include "sense_library/utils/debug.h"
#include "sense_library/utils/utils.h"
#include "sense_library/utils/measurements_vector.h"
#include "sense_library/utils/externs.h"

/********************************** Private ***********************************/

/**
 *  Stores how many queue add attempts.
 */
static uint8_t _queue_add_attempts = 0;

/**
 * Stores GPS current information.
 */
static struct gps_info _gps_info;

/**
 * How many GPS samples we are taking with same HDOP to do the final average.
 */
static uint8_t _average_samples_count = 0;

/**
 * Latitude samples buffer.
 */
static struct measurements_mean _latitude_buffer;

/**
 * Longitude samples buffer.
 */
static struct measurements_mean _longitude_buffer;

/**
 * Tells the statistic model to be applied.
 */
static enum measurements_mode _measuresements_statistic_model = MEASUREMENTS_VECTOR_MEAN;

/**
 * Custom function that converts next number inside string into a double.
 * @param[in] string
 * @return    Converted double.
 */
long long atoll_custom(const char *string) {
	long long longlong;

	if (*string == '-') {
		longlong = -1LL;
	} else {
		longlong = 0;
	}

	for (; *string >= '0' && *string <= '9'; string++) {
		longlong = 10 * longlong + (*string - '0');
	}

	return longlong;
}

/**
 * Custom function that converts next number inside string into a floating number.
 * @param[in] string
 * @return    Converted floating number.
 */
float atof_custom(const char *string) {
	float integer = 0, decimal = 0, divider = 1;

	for (; *string >= '0' && *string <= '9'; string++) {
		integer = integer * 10 + (*string - '0');
	}

	if (*string == '.') {
		for (string += 1; *string >= '0' && *string <= '9'; string++, divider *= .1) {
			decimal = decimal * 10 + (*string - '0');
		}
		return integer + decimal * divider;
	}

	return integer;
}

/**
 * Compares new GPS sample's HDOP value with current stored one.
 * @param[in] gps_info_aux New GPS sample received.
 * @return    True if new GPS has a better HDOP value, false otherwise.
 */
bool new_best_sample(struct gps_info gps_info_aux) {
	float diff = _gps_info.hdop - gps_info_aux.hdop;

	if (diff > 0.00) {
		return true;
	} else {
		return false;
	}
}

/**
 * Compares new GPS sample's HDOP value with current stored one.
 * @param[in] gps_info_aux New GPS sample received.
 * @return    True if new GPS has the same HDOP value, false otherwise.
 */
bool same_hdop(struct gps_info gps_info_aux) {
	float diff = _gps_info.hdop - gps_info_aux.hdop;

	if (diff == 0.00) {
		return true;
	} else {
		return false;
	}
}

/**
 * Gets NMEA checksum from the setence.
 * @param[in] nmea_data NMEA GPS sample received.
 * @return    NMEA checksum.
 */
uint32_t get_checksum(char *nmea_data) {
	uint32_t nmea_checksum = 0;
	char* temp;

	if (strstr(nmea_data, "*") == NULL) {
		return nmea_checksum;
	}

	temp = strstr(nmea_data, "*");
	temp++;

	while ((*temp != '\r') && (*temp != '\n') && (*temp != '\0')) {
		/* Get current character then increment */
		char byte = *temp++;

		/* Transform hex character to the 4bit
		 * equivalent number, using the ASCII table indexes */
		if (byte >= '0' && byte <= '9')
			byte = byte - '0';
		else if (byte >= 'a' && byte <= 'f')
			byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <= 'F')
			byte = byte - 'A' + 10;

		/* Shift 4 to make space for new digit,
		 * and add the 4 bits of the new digit */
		nmea_checksum = (nmea_checksum << 4) | (byte & 0xF);
	}

	return nmea_checksum;
}

/**
 * Calculates NMEA checksum.
 * @param[in] nmea_data NMEA GPS sample received.
 * @return    NMEA calculated checksum.
 */
uint32_t calculate_checksum(char *nmea_data) {
	uint32_t nmea_calculated_checksum = 0;
	uint32_t i;

	for (i = 0; i < strlen(nmea_data); i++) {
		if (nmea_data[i] == '$') {
			continue;
		}
		if ((nmea_data[i] == '*') || (nmea_data[i] == '\r')
				|| (nmea_data[i] == '\n')) {
			break;
		} else {
			nmea_calculated_checksum ^= nmea_data[i];
		}
	}

	return nmea_calculated_checksum;
}

/********************************** Public ***********************************/

/**
 * Creates new Gama GPS position message.
 * @param[in] gama_msg            Pointer to gama message buffer.
 * @param[in] message_node_fields Message fields to be used when creating new message.
 * @param[in] gps_sensor_seq      Sensor seq number.
 * @return    New message's lenght.
 */
uint16_t gps_add_nmea_position_message(uint8_t* gama_msg, gama_node_fields_t message_node_fields, uint8_t gps_sensor_seq) {
	(void) gps_sensor_seq;

	uint16_t nmea_message_length = 0;
	memset(gama_msg, 0x00, GAMA_MSG_MAX_SIZE);

	if (_average_samples_count > 0) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
		debug_print_string(DEBUG_LEVEL_1,
				(uint8_t*) "[gps_add_nmea_position_message] compute average\r\n");
		_gps_info.latitude = measurements_vector_get(&_latitude_buffer,
				_measuresements_statistic_model);
		_gps_info.longitude = measurements_vector_get(&_longitude_buffer,
				_measuresements_statistic_model);
	}

	gama_nmea_fields_t nmea_fields;
	memset((uint8_t*) &nmea_fields, 0, sizeof(gama_nmea_fields_t));
	nmea_fields.latitude = _gps_info.latitude;
	nmea_fields.longitude = _gps_info.longitude;
	nmea_fields.altitude = _gps_info.altitude;
	nmea_fields.nmea_config_bits.altitude = 1;
	nmea_fields.speed = _gps_info.speed;
	nmea_fields.nmea_config_bits.speed = 1;
	nmea_fields.course = _gps_info.course;
	nmea_fields.nmea_config_bits.course = 1;
	nmea_fields.satellite_count = _gps_info.satellite_number;
	nmea_fields.nmea_config_bits.satellite_count = 1;
	nmea_fields.fix_time = _gps_info.fix_time;
	nmea_fields.nmea_config_bits.fix_time = 1;
	nmea_fields.hdop = _gps_info.hdop;
  nmea_fields.nmea_config_bits.hdop = 1;
  nmea_fields.fix_count = _gps_info.valid_fix_count;
  nmea_fields.nmea_config_bits.fix_count = 1;

	/* For now don't use UTC and date, at least for now */
	nmea_fields.nmea_config_bits.utc_date = 0;

  /* Convert floats to string to be printed out. Note
   * that printed value might be slightly different from
   * the real float values in the last decimal case */
  char hdop_string[10], altitude_string[10], speed_string[10], course_string[10];
  memset(hdop_string, '\0', 10);
  memset(altitude_string, '\0', 10);
  memset(speed_string, '\0', 10);
  memset(course_string, '\0', 10);
  utils_ftoa(hdop_string, nmea_fields.hdop, 0.001);
  utils_ftoa(altitude_string, nmea_fields.altitude, 0.001);
  utils_ftoa(speed_string, nmea_fields.speed, 0.001);
  utils_ftoa(course_string, nmea_fields.course, 0.001);

  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[gps_add_nmea_position_message] message details: latitude = 0x%08x%08x; longitude = 0x%08x%08x; altitude = %sm", (unsigned) (nmea_fields.latitude >> 32), (unsigned) (nmea_fields.latitude), (unsigned) (nmea_fields.longitude >> 32), (unsigned) (nmea_fields.longitude), altitude_string);
  debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "; speed = %skn; course = %sº; hdop = %s", speed_string, course_string, hdop_string);
  debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "; satellite number = %d; fix time = %ds; fix count = %d\r\n", nmea_fields.satellite_count, nmea_fields.fix_time, nmea_fields.fix_count);

	nmea_message_length = gama_create_node_nmea_msg(&message_node_fields,
			&nmea_fields, gama_msg);

	return nmea_message_length;
}

/**
 * Creates new Gama GPS fix time message. Typically called when there was no GPS fix so we send
 * 0 as GPS fix time value.
 * @param[in] gps_sensor_seq      Sensor seq number.
 * @return    New message's lenght.
 */
bool gps_add_fix_time_measurement(uint8_t gps_sensor_seq) {
  gama_measure_format_v2_fields_t fix_time_measurement_fields;
	fix_time_measurement_fields.measure_type = GAMA_MEASURE_GPS_FIX_TIME;
	fix_time_measurement_fields.sensor = gps_sensor_seq;
	uint8_t fix_time_temp_8 = 0;
  uint32_t fix_time_temp_32 = 0;

	if (!_gps_info.fix) {
    fix_time_measurement_fields.config_byte = MEASURE_VALUE_UINT8_TYPE;
    fix_time_measurement_fields.val = &fix_time_temp_8;

    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[gps_add_fix_time_measurement] measurement added: details: fix time = %ds\r\n", fix_time_temp_8);
	} else {
    /* Let's cast to 32-bit number because GPS fix time isn't really a big number */
    fix_time_measurement_fields.config_byte = MEASURE_VALUE_UINT32_TYPE;
    fix_time_temp_32 = (int32_t)(_gps_info.fix_time);
    fix_time_measurement_fields.val = &fix_time_temp_32;

    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_printf_string(DEBUG_LEVEL_1, (uint8_t*) "[gps_add_fix_time_measurement] measurement added: details: fix time = %ds\r\n", fix_time_temp_32);
	}

  if(measurements_v2_manager_add_measurement(&fix_time_measurement_fields)) {
    _queue_add_attempts = 0;
    return true;
  } else {
    if(_queue_add_attempts >= GAMA_QUEUE_ADD_MAX_ATTEMPTS) {
      _queue_add_attempts = 0;
      return true;
    }
    else {
      _queue_add_attempts++;
      return false;
    }
  }
}

/**
 * Processes GGA GPS sentences and store the processed values in the respective structure entries.
 * @param[in] gga_sentence Pointer to the GGA sentence.
 * @return    True if it was processed correctly, false if an error ocurred during processing (e.g. invalid checksum).
 */
bool gps_utilities_process_gga_msg(char* gga_sentence) {
	struct gps_info _gps_info_aux;
	memcpy(&_gps_info_aux, &_gps_info, sizeof(_gps_info_aux));

	char* p = gga_sentence;
	unsigned int fractional_size = 0;
	uint64_t position_fractional;
  uint32_t altitude_fractional_part;

	if ((calculate_checksum(gga_sentence))
			!= (get_checksum(gga_sentence))) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
		debug_print_string(DEBUG_LEVEL_1,
				(uint8_t*) "[gps_utilities_process_gga_msg] checksum didn't match! discard this sentence\r\n");
		return false;
	}

	/* Get UTC time (hhmmss.ss) */
	p = strchr(p, ',') + 1;
	float time_f = atof_custom(p);
	uint32_t time = time_f;
	_gps_info_aux.hour = time / 10000;
	_gps_info_aux.minute = (time % 10000) / 100;
	_gps_info_aux.second = (time % 100);

	/* Latitude data in format ddmm.mmmmmm */
	p = strchr(p, ',') + 1;
	_gps_info_aux.latitude = 0;
	_gps_info_aux.latitude = atoll_custom(p);
	_gps_info_aux.latitude = ((int64_t) (_gps_info_aux.latitude
			* (int64_t) 1000000));
	p = strchr(p, '.') + 1;
	fractional_size = (unsigned int) (strchr(p, ',') - p);
	position_fractional = (atoll_custom(p));
	if (fractional_size < 2) {
		_gps_info_aux.latitude += (position_fractional * 100000uLL);
	} else if (fractional_size < 3) {
		_gps_info_aux.latitude += (position_fractional * 10000uLL);
	} else if (fractional_size < 4) {
		_gps_info_aux.latitude += (position_fractional * 1000uLL);
	} else if (fractional_size < 5) {
		_gps_info_aux.latitude += (position_fractional * 100uLL);
	} else if (fractional_size < 6) {
		_gps_info_aux.latitude += (position_fractional * 10uLL);
	} else {
		_gps_info_aux.latitude += (position_fractional * 1uLL);
	}

	/* Get latitude signal by checking West (i.e. -) and East (i.e. +) */
	p = strchr(p, ',') + 1;
	if (p[0] == 'N') {
		/* Do nothing */
	} else if (p[0] == 'S') {
		/* Invert sign */
		_gps_info_aux.latitude = _gps_info_aux.latitude * -1;
	} else {
		return false;
	}

	/* Longitude data in format ddmm.mmmmmm */
	p = strchr(p, ',') + 1;
	_gps_info_aux.longitude = atoll_custom(p);
	_gps_info_aux.longitude = _gps_info_aux.longitude * 1000000uLL;
	p = strchr(p, '.') + 1;
	fractional_size = (unsigned int) (strchr(p, ',') - p);
	position_fractional = (atoll_custom(p));
	if (fractional_size < 2) {
		_gps_info_aux.longitude += (position_fractional * 100000uLL);
	} else if (fractional_size < 3) {
		_gps_info_aux.longitude += (position_fractional * 10000uLL);
	} else if (fractional_size < 4) {
		_gps_info_aux.longitude += (position_fractional * 1000uLL);
	} else if (fractional_size < 5) {
		_gps_info_aux.longitude += (position_fractional * 100uLL);
	} else if (fractional_size < 6) {
		_gps_info_aux.longitude += (position_fractional * 10uLL);
	} else {
		_gps_info_aux.longitude += (position_fractional * 1uLL);
	}

	/* Get longitude signal by checking North (i.e. +) and South (i.e. -) */
	p = strchr(p, ',') + 1;
	/* check E/W */
	if (p[0] == 'E') {
		/* do nothing */
	} else if (p[0] == 'W') {
		_gps_info_aux.longitude = _gps_info_aux.longitude * -1;
	} else {
		return false;
	}

	/* GPS fix (0 = no fix, 1 = GPS fix, 2 = DGPS fix) */
	p = strchr(p, ',') + 1;
	/* Currently discarded */

	/* Number of satellites */
	p = strchr(p, ',') + 1;
	_gps_info_aux.satellite_number = atoll_custom(p);

	/* HDOP integer part */
	p = strchr(p, ',') + 1;
	_gps_info_aux.hdop = atof(p);

	/* Altitude (in meters) */
	p = strchr(p, ',') + 1;
	_gps_info_aux.altitude = atof(p);

	/* Ignore remaining sentence */

	/* Compare with previous GPS info */
	if (new_best_sample(_gps_info_aux)) {
		/* Reset Samples count and Average vectors */
		_average_samples_count = 0;
		measurements_vector_init(&_latitude_buffer, _measuresements_statistic_model);
		measurements_vector_init(&_longitude_buffer, _measuresements_statistic_model);

        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
		debug_print_string(DEBUG_LEVEL_1,
				(uint8_t*) "[gps_utilities_process_rmc_msg] new best hdop. this gps sentence has lower hdop. let's save this sample\r\n");

		/* Save this GPS sample */
		_gps_info.latitude = _gps_info_aux.latitude;
		_gps_info.longitude = _gps_info_aux.longitude;
		_gps_info.hdop = _gps_info_aux.hdop;
	} else if (same_hdop(_gps_info_aux)) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
		debug_print_string(DEBUG_LEVEL_1,
				(uint8_t*) "[gps_utilities_process_rmc_msg] this gps sentence has same hdop value. use it! \r\n");

		_average_samples_count++;

		if (measurements_vector_get(&_latitude_buffer, _measuresements_statistic_model)
				!= _gps_info_aux.latitude) {
			measurements_vector_add(&_latitude_buffer, _measuresements_statistic_model,
					_gps_info_aux.latitude);
		}

		if (measurements_vector_get(&_longitude_buffer, _measuresements_statistic_model)
				!= _gps_info_aux.longitude) {
			measurements_vector_add(&_longitude_buffer, _measuresements_statistic_model,
					_gps_info_aux.longitude);
		}
	} else {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
		debug_print_string(DEBUG_LEVEL_1,
				(uint8_t*) "[gps_utilities_process_rmc_msg] this gps sentence hasn't lower hdop. discard it!\r\n");
	}

	/* Save this GPS sample */
	_gps_info.satellite_number = _gps_info_aux.satellite_number;
	_gps_info.year = _gps_info_aux.year;
	_gps_info.month = _gps_info_aux.month;
	_gps_info.day = _gps_info_aux.day;
	_gps_info.hour = _gps_info_aux.hour;
	_gps_info.minute = _gps_info_aux.minute;
	_gps_info.second = _gps_info_aux.second;
	_gps_info.altitude = _gps_info_aux.altitude;
	_gps_info.course = _gps_info_aux.course;
	_gps_info.speed = _gps_info_aux.speed;

	return true;
}

/**
 * Processes RMC GPS sentences and store the processed values in the respective structure entries.
 * @param[in] rmc_sentence Pointer to the GGA sentence.
 * @return    True if it was processed correctly, false if an error ocurred during processing (e.g. invalid checksum).
 */
bool gps_utilities_process_rmc_msg(char* rmc_sentence) {
	struct gps_info _gps_info_aux;
	memcpy(&_gps_info_aux, &_gps_info, sizeof(_gps_info_aux));

	char* p = rmc_sentence;
	unsigned int fractional_size = 0;
  uint32_t speed_fractional_part, course_fractional_part;
	uint64_t position_fractional;

	if ((calculate_checksum(rmc_sentence))
			!= (get_checksum(rmc_sentence))) {
        debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
		debug_print_string(DEBUG_LEVEL_1,
				(uint8_t*) "[gps_utilities_process_gga_msg] checksum didn't match! discard this sentence\r\n");
		return false;
	}

	/* Get UTC time (hhmmss.ss) */
	p = strchr(p, ',') + 1;
	float time_f = atof_custom(p);
	uint32_t time = time_f;
	_gps_info_aux.hour = time / 10000;
	_gps_info_aux.minute = (time % 10000) / 100;
	_gps_info_aux.second = (time % 100);

	/* Fix validity */
	p = strchr(p, ',') + 1;
	if (p[0] == 'A') {
		/* First fix? */
		if (!_gps_info_aux.fix) {
			_gps_info_aux.fix = true;
			_gps_info_aux.fix_timestamp = rtc_get_milliseconds();
			_gps_info_aux.fix_time = ((_gps_info_aux.fix_timestamp
					- _gps_info_aux.start_collecting_timestamp) / 1000);
		} else {
			_gps_info_aux.fix = true;
			_gps_info_aux.fix_timestamp = rtc_get_milliseconds();
		}
	} else {
		_gps_info_aux.fix = false;
		_gps_info_aux.fix_timestamp = 0;
		_gps_info_aux.fix_time = 0;
		_gps_info_aux.valid_fix_count = 0;

		/* Return, invalid sample */
		return false;
	}

	/* Latitude data in format ddmm.mmmmmm */
	p = strchr(p, ',') + 1;
	_gps_info_aux.latitude = 0;
	_gps_info_aux.latitude = atoll_custom(p);
	_gps_info_aux.latitude = ((int64_t) (_gps_info_aux.latitude
			* (int64_t) 1000000));
	p = strchr(p, '.') + 1;
	fractional_size = (unsigned int) (strchr(p, ',') - p);
	position_fractional = (atoll_custom(p));
	if (fractional_size < 2) {
		_gps_info_aux.latitude += (position_fractional * 100000uLL);
	} else if (fractional_size < 3) {
		_gps_info_aux.latitude += (position_fractional * 10000uLL);
	} else if (fractional_size < 4) {
		_gps_info_aux.latitude += (position_fractional * 1000uLL);
	} else if (fractional_size < 5) {
		_gps_info_aux.latitude += (position_fractional * 100uLL);
	} else if (fractional_size < 6) {
		_gps_info_aux.latitude += (position_fractional * 10uLL);
	} else {
		_gps_info_aux.latitude += (position_fractional * 1uLL);
	}

	/* Get latitude signal by checking West (i.e. -) and East (i.e. +) */
	p = strchr(p, ',') + 1;
	if (p[0] == 'N') {
		/* Do nothing */
	} else if (p[0] == 'S') {
		/* Invert sign */
		_gps_info_aux.latitude = _gps_info_aux.latitude * -1;
	} else {
		return false;
	}

	/* Longitude data in format ddmm.mmmmmm */
	p = strchr(p, ',') + 1;
	_gps_info_aux.longitude = atoll_custom(p);
	_gps_info_aux.longitude = _gps_info_aux.longitude * 1000000uLL;
	p = strchr(p, '.') + 1;
	fractional_size = (unsigned int) (strchr(p, ',') - p);
	position_fractional = (atoll_custom(p));
	if (fractional_size < 2) {
		_gps_info_aux.longitude += (position_fractional * 100000uLL);
	} else if (fractional_size < 3) {
		_gps_info_aux.longitude += (position_fractional * 10000uLL);
	} else if (fractional_size < 4) {
		_gps_info_aux.longitude += (position_fractional * 1000uLL);
	} else if (fractional_size < 5) {
		_gps_info_aux.longitude += (position_fractional * 100uLL);
	} else if (fractional_size < 6) {
		_gps_info_aux.longitude += (position_fractional * 10uLL);
	} else {
		_gps_info_aux.longitude += (position_fractional * 1uLL);
	}

	/* Get longitude signal by checking North (i.e. +) and South (i.e. -) */
	p = strchr(p, ',') + 1;
	/* check E/W */
	if (p[0] == 'E') {
		/* do nothing */
	} else if (p[0] == 'W') {
		_gps_info_aux.longitude = _gps_info_aux.longitude * -1;
	} else {
		return false;
	}

	/* Speed over ground (in knots). E.g. 1.24 */
	p = strchr(p, ',') + 1;
	_gps_info_aux.speed = atof(p);

	/* Course over ground (in degrees) */
	p = strchr(p, ',') + 1;
	_gps_info_aux.course = atof(p); /* degrees */

	/* Get data (ddmmyy) */
	p = strchr(p, ',') + 1;
	uint32_t full_date = atof_custom(p);
	_gps_info_aux.day = full_date / 10000;
	_gps_info_aux.month = (full_date % 10000) / 100;
	_gps_info_aux.year = (full_date % 100);

	/* Ignore remaining sentence */

	/* Save this GPS sample */
	_gps_info.satellite_number = _gps_info_aux.satellite_number;
	_gps_info.year = _gps_info_aux.year;
	_gps_info.month = _gps_info_aux.month;
	_gps_info.day = _gps_info_aux.day;
	_gps_info.hour = _gps_info_aux.hour;
	_gps_info.minute = _gps_info_aux.minute;
	_gps_info.second = _gps_info_aux.second;
	_gps_info.altitude = _gps_info_aux.altitude;
	_gps_info.course = _gps_info_aux.course;
	_gps_info.speed = _gps_info_aux.speed;

	_gps_info.valid_fix_count++;
	_gps_info.fix = _gps_info_aux.fix;
	_gps_info.fix_timestamp = _gps_info_aux.fix_timestamp;
	_gps_info.fix_time = _gps_info_aux.fix_time;

	return true;
}

/**
 * Resets current stored GPS data.
 */
void gps_utilities_reset_gps_info(void) {
	_gps_info.fix = false;
	_gps_info.fix_timestamp = 0;
	_gps_info.fix_time = 0;
	_gps_info.valid_fix_count = 0;
	_gps_info.hdop = 255;

	_average_samples_count = 0;
	measurements_vector_init(&_latitude_buffer, _measuresements_statistic_model);
	measurements_vector_init(&_longitude_buffer, _measuresements_statistic_model);
}

/**
 * Returns current stored GPS data.
 * @return GPS data structure.
 */
struct gps_info* gps_utilities_get_gps_info(void) {
	return &_gps_info;
}

/**
 * Sets GPS starting aquisition time inside GPS internal data structure.
 */
void gps_utilities_set_gps_start_collecting_timestamp(void) {
	_gps_info.start_collecting_timestamp = rtc_get_milliseconds();
}

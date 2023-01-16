/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file  gps_utilities.h
 * @brief This file has the GPS utilities header.
 */

#ifndef GPS_UTILITIES_H_
#define GPS_UTILITIES_H_

/********************************** Includes ***********************************/

/* Standard C libraries */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

/* Gama protocol */
#include "sense_library/protocol-gama/gama_fw/include/gama_generic.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_measurement.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_definitions.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_position.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_node_definitions.h"
#include "sense_library/protocol-gama/gama_fw/include/gama_util/fixedpoint.h"

/********************************** Private ***********************************/

/**
 * Contains the data fields that can be read from the GPS sensor.
 */
struct gps_info {
	uint64_t fix_timestamp;               ///< The timestamp when it took the last valid fix.
	uint64_t start_collecting_timestamp;  ///< The timestamp when GPS sensor was turned on.
	bool fix;                             ///< True = valid fix, false = wainting for one or didn't get one.
	uint16_t fix_time;                    ///< Stores the time to get the first valid fix since powered on in seconds.
	uint8_t satellite_number;             ///< Tells how many GPS satellites in the sky.
	float hdop;                           ///< Horizontal Dilution of Precision (one of the fix quality parameters).
	uint8_t year;                         ///< Year.
	uint8_t month;                        ///< Month.
	uint8_t day;                          ///< Day.
	uint8_t hour;                         ///< Hour.
	uint8_t minute;                       ///< Minute.
	uint8_t second;                       ///< Second.
	int64_t latitude;                     ///< Latitude (could be an average of current GPS samples with same HDOP value).
	int64_t longitude;                    ///< Longitude (could be an average of current GPS samples with same HDOP value).
	float altitude;                       ///< Altitude.
	float course;                         ///< Course.
	float speed;                          ///< Speed.
	uint16_t valid_fix_count;             ///< How many valid consecutive GPS samples we are now.
	uint64_t last_read_timestamp;         ///< The timestamp of last collection.
};

/********************************** Prototypes ***********************************/

uint16_t gps_add_nmea_position_message(uint8_t* gama_msg, gama_node_fields_t message_node_fields, uint8_t gps_sensor_seq);

bool gps_add_fix_time_measurement(uint8_t gps_sensor_seq);

bool gps_utilities_process_gga_msg(char* gga_sentence);

bool gps_utilities_process_rmc_msg(char* rmc_sentence);

void gps_utilities_reset_gps_info(void);

struct gps_info* gps_utilities_get_gps_info(void);

void gps_utilities_set_gps_start_collecting_timestamp(void);

#endif /* GPS_UTILITIES_H_ */

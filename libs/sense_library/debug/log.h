/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file log.h
 * @brief This header file has the log utilities.
 * @author João Ambrósio
 * @date 17/03/2019
 */

#ifndef LOG_H_
#define LOG_H_

/********************************** Includes ***********************************/

/* Standard C library */
#include <stdint.h>

/********************************** Definitions ********************************/



/********************************** Prototypes *********************************/

void log_init(void);

void log_flush(void);

#endif /* LOG_H_ */
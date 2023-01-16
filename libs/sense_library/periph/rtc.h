/*
 * Copyright (c) 2020 Sensefinity
 * This file is subject to the Sensefinity private code license.
 */

/**
 * @file rtc.h
 * @brief This header file has the RTC peripherical utilities.
 * @author João Ambrósio and Jorge Veiga
 * @date 17/12/2020
 */

#ifndef RTC_H_
#define RTC_H_

/********************************** Includes ***********************************/

/* Standard C library */
#include <stdint.h>

/* SDK */
#include "drv_rtc.h"

/********************************** Definitions ********************************/

#define RTC_BITS                (24)                                                        ///< RTC bits number.
#define RTC_FREQ                ((float) 32768)                                             ///< RTC frequency.
#define RTC_OVERFLOW            (RTC_COUNTER_COUNTER_Msk)                                   ///< RTC overflow ticks number.
#define RTC_INTERRUPT_PRIORITY  (6)                                                         ///< RTC interrupt priority.
#define RTC_SAFE_COUNTER_CC     (0)                                                         ///< CC safe counter number.

#define RTC_PERIOD_MS           ((float) (1000 / RTC_FREQ))                                 ///< RTC period in ms.
#define RTC_MS_TO_S(ms)         (ms / 1000)                                                 ///< RTC milliseconds to seconds.
#define RTC_S_TO_MS(ms)         (s  * 1000)                                                 ///< RTC milliseconds to seconds.
#define RTC_MS_TO_US(ms)        (ms * 1000)                                                 ///< RTC seconds to milliseconds.

#define RTC_TICKS_TO_MS(t)      ((float) (t * RTC_PERIOD_MS))                               ///< RTC ticks to milliseconds utility.
#define RTC_MS_TO_TICKS(ms)     ((float) (DRV_RTC_US_TO_TICKS(RTC_MS_TO_US(ms), RTC_FREQ))) ///< RTC milliseconds to ticks utility.

/********************************** Prototypes *********************************/

void rtc_init(void);

void rtc_set_timer(uint64_t ts_now);

uint64_t rtc_get_milliseconds(void);

uint64_t rtc_get_seconds(void);

#endif /* RTC_H_ */
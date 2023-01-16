
/*
* @file		lcd_mngr.c
* @date		August 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the LCD aplication.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/
/* Interface */
#include "lcd_mngr.h"

#if MICROCONTROLER_2
/* Utilities */
#include "sense_library/periph/rtc.h"
//#include "measurement_mngr.h"
#include "utils.h"
#include "twi_mngr.h"
#include "drivers/ssd1309.h"
/* Debug - libs */
#include "sense_library/utils/debug.h"

/* standard library */
#include <stdint.h>
#include <stdbool.h>

/* SDK */
#include "nrf_gpio.h"
#include "nrf_font.h"
#include "nrf_gfx.h"
#include "app_error.h"
#include "nrf_delay.h"

/* Utils */
#include "sense_library/utils/debug.h"
#include "sense_library/utils/externs.h"
#include "sense_library/utils/utils.h"
/********************************** Private ************************************/

/* ##### Images ##### */
/* https://dot2pic.com/ Used to draw the symbol and convert to BMP */

/*
 * Battery empty image. 
 */
static uint8_t _battery0_image[] = {0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
                                    1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
                                    1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
                                    1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
                                    1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
                                    1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,
                                    1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,
                                    0,1,1,1,1,1,1,1,1,1,1,1,1,0,0}; 

/*
 * Battery 25% image. 
 */
static uint8_t _battery25_image[] = {0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
                                     1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,
                                     1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,
                                     1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,
                                     1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,
                                     1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,
                                     1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,0,0};

/*
 * Battery 50% image. 
 */
static uint8_t _battery50_image[] = {0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
                                     1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,
                                     1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,
                                     1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,
                                     1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,
                                     1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,
                                     1,1,1,1,1,1,1,0,0,0,0,0,0,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,0,0};

/*
 * Battery 75% image. 
 */
static uint8_t _battery75_image[] = {0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
                                     1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,
                                     1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,
                                     1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,
                                     1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,
                                     1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,
                                     1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,
                                     0,1,1,1,1,1,1,1,1,1,1,1,1,0,0};

/*
 * Battery full image. 
 */
static uint8_t _battery100_image[] = {0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                      1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
                                      0,1,1,1,1,1,1,1,1,1,1,1,1,0,0};

/*
 * Antenna no signal image. 
 */
static uint8_t _antenna_00_image[] = {1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,
                                      0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,
                                      0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
                                      0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,
                                      0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,
                                      0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,
                                      0,0,0,1,0,0,0,0,1,0,1,0,0,0,0,
                                      0,0,0,1,0,0,0,1,0,0,0,1,0,0,0};

/*
 * Antenna with one bar signal image. 
 */
static uint8_t _antenna_33_image[] = {1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,
                                      0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,
                                      0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
                                      0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
                                      0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,
                                      0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,
                                      0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,
                                      0,0,0,1,0,0,1,1,0,0,0,0,0,0,0};

/*
 * Antenna with two bars signal image. 
 */
static uint8_t _antenna_66_image[] = {1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,
                                      0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,
                                      0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,
                                      0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,
                                      0,0,0,1,0,0,0,0,0,1,1,0,0,0,0,
                                      0,0,0,1,0,0,1,1,0,1,1,0,0,0,0,
                                      0,0,0,1,0,0,1,1,0,1,1,0,0,0,0,
                                      0,0,0,1,0,0,1,1,0,1,1,0,0,0,0};

/*
 * Antenna with three bars signal image. 
 */
static uint8_t _antenna_100_image[] = {1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,
                                       0,1,0,1,0,1,0,0,0,0,0,0,1,1,0,
                                       0,0,1,1,1,0,0,0,0,0,0,0,1,1,0,
                                       0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,
                                       0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,
                                       0,0,0,1,0,0,1,1,0,1,1,0,1,1,0,
                                       0,0,0,1,0,0,1,1,0,1,1,0,1,1,0,
                                       0,0,0,1,0,0,1,1,0,1,1,0,1,1,0};

/*
 * Charge source, no charge image. 
 */
static uint8_t _chg_indicator_nochg_image[] ={0,0,0,0,0,0,0,0,
                                              0,0,0,0,0,0,0,0,
                                              0,0,0,0,0,0,0,0,
                                              0,0,0,0,0,0,0,0,
                                              0,0,0,0,0,0,0,0,
                                              0,0,0,0,0,0,0,0,
                                              0,0,0,0,0,0,0,0,
                                              0,0,0,0,0,0,0,0};

/*
 * Charge source, wireless charge image. 
 */
static uint8_t _chg_indicator_wireless_image[] =  {0,0,1,0,0,0,0,0,
                                                   0,1,0,0,0,0,0,0,
                                                   1,0,0,0,0,0,0,0,
                                                   1,1,1,0,0,0,0,0,
                                                   0,0,1,0,0,0,0,0,
                                                   0,1,0,0,0,0,0,0,
                                                   0,1,0,0,0,0,0,0,
                                                   1,0,0,0,0,0,0,0};

/*
 * Charge source, USB charge image. 
 */
static uint8_t _chg_indicator_usb_image[] =  {0,0,1,0,0,0,0,1,
                                              0,1,0,0,0,0,1,0,
                                              1,0,0,0,0,1,0,0,
                                              1,1,1,0,0,1,1,1,
                                              0,0,1,0,0,0,0,1,
                                              0,1,0,0,0,0,1,0,
                                              0,1,0,0,0,0,1,0,
                                              1,0,0,0,0,1,0,0}
;
/*
 * GPS off image. 
 */
static uint8_t _gps_off_image[] = {0,0,0,0,1,1,1,0,0,0,0,
                                   0,0,0,1,0,0,0,1,0,0,0,
                                   0,0,1,0,0,1,0,0,1,0,0,
                                   0,0,1,0,1,0,1,0,1,0,0,
                                   0,0,1,0,0,1,0,0,1,0,0,
                                   0,0,0,1,0,0,0,1,0,0,0,
                                   0,0,0,0,1,0,1,0,0,0,0,
                                   0,0,0,0,0,1,0,0,0,0,0};

/*
 * GPS on image (could be used to indicate when trying ot get a fix). 
 */
static uint8_t _gps_on_image[] =  {0,0,0,1,0,0,0,1,0,0,0,
                                   0,0,1,0,1,1,1,0,1,0,0,
                                   0,1,0,1,1,0,1,1,0,1,0,
                                   0,1,0,1,0,1,0,1,0,1,0,
                                   0,1,0,1,1,0,1,1,0,1,0,
                                   0,0,1,0,1,1,1,0,1,0,0,
                                   0,0,0,1,0,1,0,1,0,0,0,
                                   0,0,0,0,1,0,1,0,0,0,0};

/*
 * Homepage house image. 
 */
static uint8_t _house_image[] =       {0,0,0,1,1,0,0,0,
                                       0,0,1,1,1,1,0,0,
                                       0,1,1,1,1,1,1,0,
                                       1,1,1,1,1,1,1,1,
                                       0,0,1,1,1,1,0,0,
                                       0,0,1,1,1,1,0,0,
                                       0,0,1,1,1,1,0,0,
                                       0,0,1,0,0,1,0,0};

/*
 * Big low temperature image. 
 */
static uint8_t _temp_low_image[] = {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0};

/*
 * Big medium temperature image. 
 */
static uint8_t _temp_medium_image[] = {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0};

/*
 * Big high temperature image. 
 */
static uint8_t _temp_high_image[] = {0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,1,1,0,1,1,1,1,1,1,1,0,1,1,0,0,0,0,0,0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0};

/*
 * Last timestamp statbar printed. 
 */
static uint64_t _print_statbar_last_timestamp;

/*
 * Last timestamp init printed. 
 */
static uint64_t _printed_init_last_timestamp;

/*
 * Last timestamp infobar printed. 
 */
static uint64_t _print_infobar_last_timestamp;

/*
 * Last timestamp active area printed. 
 */
static uint64_t _printed_homepage_last_timestamp;

/*
 * Buffer that stores the plotable area (not implemented). 
 */
static uint8_t _plot_area[ACTIVE_AREA_X_SIZE][ACTIVE_AREA_Y_SIZE];

/*
 * Buffer with converted values (value -> y_pixels) (not implemented). 
 */
static uint8_t _plot_in_pixels[PLOT_MAX_X_POINTS];

/*
 * Index used to plot without blocking (probably we will stop supporting this).
 */
static uint8_t plot_print_index;

/*
 * EXTERNAL FONT_INFO struct, small font (defined in consolas_8ptFont.c, custom).
 */
extern const nrf_gfx_font_desc_t consolas_8ptFontInfo;

/*
 * EXTERNAL FONT_INFO struct, bigger font (defined in orkney8pts.c).
 */
extern const nrf_gfx_font_desc_t orkney_8ptFontInfo;

/*
 * EXTERNAL nrf_lcd_t struct with the driver pointers (defined in ssd1309.c).
 */
extern const nrf_lcd_t ssd1309_lcd_instance;

/*
 * Pointer to custom FONT_INFO struct.
 */
static const nrf_gfx_font_desc_t *small_font = &consolas_8ptFontInfo;

/*
 * Pointer to FONT_INFO struct for bigger font.
 */
static const nrf_gfx_font_desc_t *medium_font = &orkney_8ptFontInfo;

/*
 * Pointer to nrf_lcd_t struct.
 */
static const nrf_lcd_t *lcd_instance = &ssd1309_lcd_instance;

/*
 * Indicates if first init screen printed.
 */
static bool _second_screen_first_time;

/*
 * Indicates if second init screen printed.
 */
static bool _first_screen_first_time;

/*
 * Indicates if we are ploting without blocking (probably we will stop supporting this).
 */
static bool _printing_plot = false;

/*
 * Indicates if status bar screen printed.
 */
static bool _statbar_first_time = false;

/*
 * Indicates if information bar screen printed.
 */
static bool _infobar_first_time = false;

/*
 * Indicates if homepage screen printed.
 */
static bool _homepage_first_time = false;

/*
 * State machine used on loop.
 */
static lcd_display_states current_display_state;

/*
 * Indicates if main menu printed.
 */
static bool _main_menu_first_time = false;

/*
 * Indicates if ecg menu printed (check drawings on jorge's notes).
 */
static bool _ecgpage_first_time = false;

/*
 * Indicates if temperature menu printed (check drawings on jorge's notes).
 */
static bool _temppage_first_time = false;

/*
 * Indicates if dev menu printed (check drawings on jorge's notes).
 */
static bool _devpage_first_time = false;

/* ###### Callbacks passed to this app ###### */
static lcd_system_is_in_standby_callback_def _lcd_system_is_in_standby_callback;
static lcd_is_gps_active_callback_def _lcd_is_gps_active_callback;
static lcd_get_battery_percentage_callback_def _lcd_get_battery_percentage_callback;
static lcd_get_signal_percentage_callback_def _lcd_get_signal_percentage_callback;
static lcd_get_charge_source_callback_def _lcd_get_charge_source_callback;
static lcd_center_button_pressed_callback_def _lcd_center_button_pressed_callback;
static lcd_left_button_pressed_callback_def _lcd_left_button_pressed_callback;
static lcd_right_button_pressed_callback_def _lcd_right_button_pressed_callback;
static lcd_up_button_pressed_callback_def _lcd_up_button_pressed_callback;
static lcd_down_button_pressed_callback_def _lcd_down_button_pressed_callback;
static lcd_is_battery_saving_active_callback_def _lcd_is_battery_saving_active_callback;
static lcd_get_temperature_percentage_callback_def _lcd_get_temperature_percentage_callback;

/* ###### Private functions ###### */
void _lcd_print_grid(void);
void _lcd_active_area_clear(void);
void _lcd_convert_and_store(uint8_t value);
void _lcd_print_plot_noblock(void);
void _lcd_print_string_big(char *string, uint8_t x_start, uint8_t y_start);
void _lcd_print_string(char *string, uint8_t x_start, uint8_t y_start);
void _lcd_print_array_block(uint8_t *buffer, uint8_t x_start, uint8_t y_start, uint8_t x_size, uint8_t y_size);
void _lcd_print_battery(void);
void _lcd_print_antenna_signal(void);
void _lcd_print_gps(void);
void _lcd_print_big_temp(void);
void _print_first_init(void);
void _print_second_init(void);
void _lcd_page_clear(uint8_t page);
void _lcd_clear(void);
void _lcd_print_status_bar(void);
void _lcd_print_house(void);
void _lcd_print_info_bar(void);
void _lcd_reset_state_flags(void);

/* ********************************* Private *********************************** */
/*
 * @brief Print a grid to be used on menus with lists.
 */
void _lcd_print_grid(void) {

  for(int i = 2; i <= SSD1309_PAGES -3 ; i++) {
    const nrf_gfx_line_t line_plot = {0, 
                                      i * SSD1309_PAGE_LINES, 
                                      ACTIVE_AREA_X_SIZE,
                                      i * SSD1309_PAGE_LINES,
                                      LINE_THICKNESS}; 
  
    nrf_gfx_line_draw(lcd_instance, &line_plot, 1);
  }
  /* Display */
  nrf_gfx_display(lcd_instance);
}

/*
 * @brief Clear the active area.
 */
void _lcd_active_area_clear(void) {
  _lcd_page_clear(ACTIVE_AREA_PAGE1);
  _lcd_page_clear(ACTIVE_AREA_PAGE2);
  _lcd_page_clear(ACTIVE_AREA_PAGE3);
  _lcd_page_clear(ACTIVE_AREA_PAGE4);
  _lcd_page_clear(ACTIVE_AREA_PAGE5);
  _lcd_page_clear(ACTIVE_AREA_PAGE6);
}
/*
 * @brief Convert a value to y pixel and add it to bufer.
 *
 * @paramin value to be converted. 
 */
void _lcd_convert_and_store(uint8_t value) {
  if(plot_print_index == PLOT_MAX_X_POINTS) {
    _printing_plot = true;
    //_lcd_print_plot(); 
  }
  _plot_in_pixels[plot_print_index] = (value) * (ACTIVE_AREA_Y_START - (ACTIVE_AREA_Y_START + ACTIVE_AREA_Y_SIZE)) / (0xff) + ACTIVE_AREA_Y_START + ACTIVE_AREA_Y_SIZE;
  plot_print_index++;
}

/*
 * @brief Plot a graph, non blocking way. 
 */
void _lcd_print_plot_noblock(void) {
  const nrf_gfx_line_t line_plot = {plot_print_index * PLOT_TIMESTAMP, 
                                    _plot_in_pixels[plot_print_index], 
                                    (plot_print_index + 1) * PLOT_TIMESTAMP,
                                    _plot_in_pixels[plot_print_index + 1],
                                    LINE_THICKNESS};  
  nrf_gfx_line_draw(lcd_instance, &line_plot, 0);

  /* Check if we reached the end of the active area */
  if(plot_print_index == PLOT_MAX_X_POINTS) {
    memset(_plot_in_pixels, 0, ACTIVE_AREA_X_SIZE);
    plot_print_index = 0;
    _printing_plot = false;
  }
}

/*
 * @brief Print a string on a x and y with bigger font.
 *
 * @paramin string String to be printed. 
 * @paramin x_start X coordinate on where to print.
 * @paramin y_start Y coordinate on where to print.
 */
void _lcd_print_string_big(char *string, uint8_t x_start, uint8_t y_start) {
  nrf_gfx_point_t pixel = {x_start, y_start};
  nrf_gfx_print(lcd_instance, &pixel, COLOR_WHITE, string, medium_font, false);
}

/*
 * @brief Print a string on a x and y.
 *
 * @paramin string String to be printed. 
 * @paramin x_start X coordinate on where to print.
 * @paramin y_start Y coordinate on where to print.
 */
void _lcd_print_string(char *string, uint8_t x_start, uint8_t y_start) {
  nrf_gfx_point_t pixel = {x_start, y_start};
  nrf_gfx_print(lcd_instance, &pixel, COLOR_WHITE, string, small_font, false);
}

/*
 * @brief Print a unidimentional array like the symbol images.
 *
 * @paramin buffer Array to print. 
 * @paramin x_start X coordinate on where to print.
 * @paramin y_start Y coordinate on where to print.
 * @paramin x_size X size of the printed image.
 * @paramin y_size Y size of the printed image.
 */
void _lcd_print_array_block(uint8_t *buffer, uint8_t x_start, uint8_t y_start, uint8_t x_size, uint8_t y_size) {
  nrf_gfx_point_t pixel = {x_start, y_start};

  for(int i = 0 ; pixel.y < y_start + y_size ; i++) {
    nrf_gfx_point_draw(lcd_instance, &pixel, buffer[i]);
    /* Check if we need to jump a line */
    if(pixel.x == (x_size + x_start -1)) {
      pixel.y++;
      pixel.x = x_start;
    } else {
      pixel.x++;
    }
  }
  return;
}
/*
 * @brief Print the battery simbol and charge source indicator.
 */
void _lcd_print_battery(void) {
  static uint8_t *choosen_battery;
  static uint8_t *choosen_indicator;

  uint8_t battery_percent = _lcd_get_battery_percentage_callback();
  uint8_t charge_indicator = _lcd_get_charge_source_callback();

  if(battery_percent < BATTERY_0PERCENT_THOLD) {
    choosen_battery = _battery0_image;
  } else if(battery_percent < BATTERY_25PERCENT_THOLD) {
    choosen_battery = _battery25_image;
  } else if(battery_percent < BATTERY_50PERCENT_THOLD) {
    choosen_battery = _battery50_image;
  } else if(battery_percent < BATTERY_75PERCENT_THOLD) {
    choosen_battery = _battery75_image;
  } else {
    choosen_battery = _battery100_image;
  }
  if(charge_indicator == CHARGE_INDICATOR_WIRELESS) {
    choosen_indicator = _chg_indicator_wireless_image;
  } else if(charge_indicator == CHARGE_INDICATOR_USB) {
    choosen_indicator = _chg_indicator_usb_image;
  } else {
    choosen_indicator = _chg_indicator_nochg_image;
   }
   /* Print battery symbol */
  _lcd_print_array_block(choosen_battery, STATBAR_X_START, STATBAR_Y_START, STATBAR_BATTERY_X_SIZE, STATBAR_BATTERY_Y_SIZE);

  /* Print charge indicator symbol */
  _lcd_print_array_block(choosen_indicator, STATBAR_X_START + STATBAR_CHG_SOURCE_X_OFFSET, STATBAR_Y_START, STATBAR_CHG_SOURCE_X_SIZE, STATBAR_CHG_SOURCE_Y_SIZE);
}
 
/*
 * @brief Print the antenna signal intensity image.
 */
void _lcd_print_antenna_signal(void) {
  static uint8_t *choosen_antenna;
  uint8_t signal_percent = _lcd_get_battery_percentage_callback();

  if(signal_percent < ANTENNA_00PERCENT_THOLD) {
    choosen_antenna = _antenna_00_image;
  } else if(signal_percent < ANTENNA_33PERCENT_THOLD) {
    choosen_antenna = _antenna_33_image;
  } else if(signal_percent < ANTENNA_66PERCENT_THOLD) {
    choosen_antenna = _antenna_66_image;
  } else {
    choosen_antenna = _antenna_100_image;
  }
  _lcd_print_array_block(choosen_antenna, STATBAR_X_START + STATBAR_ANTENNA_X_OFFSET , STATBAR_Y_START, STATBAR_BATTERY_X_SIZE, STATBAR_BATTERY_Y_SIZE);
}

/*
 * @brief Print the GPS image.
 */
void _lcd_print_gps(void) {
  static uint8_t *choosen_gps;
  bool gps_status = _lcd_is_gps_active_callback();

  if(gps_status) {
    choosen_gps = _gps_on_image;
  } else {
    choosen_gps = _gps_off_image;
  }
  _lcd_print_array_block(choosen_gps, STATBAR_X_START + STATBAR_GPS_X_OFFSET , STATBAR_Y_START, STATBAR_GPS_X_SIZE, STATBAR_GPS_Y_SIZE);
}

/*
 * @brief Print the bigger temperature image.
 */
void _lcd_print_big_temp(void) {

  static uint8_t *choosen_temperature;
  uint8_t temperature = _lcd_get_temperature_percentage_callback();

  if(temperature > BIG_TEMP_HIGH_THOLD) {
    choosen_temperature = _temp_high_image;
  } else if(temperature < BIG_TEMP_LOW_THOLD) {
    choosen_temperature = _temp_low_image;
  } else {
    choosen_temperature = _temp_medium_image;
  }
  _lcd_print_array_block(choosen_temperature, ACTIVE_AREA_X_START + ACTIVE_AREA_TEMP_X_OFFSET , ACTIVE_AREA_Y_START + ACTIVE_AREA_TEMP_Y_OFFSET, ACTIVE_AREA_TEMP_X_SIZE, ACTIVE_AREA_TEMP_Y_SIZE);
}

/*
 * @brief Print the first init screen.
 */
void _print_first_init(void) {
  char string[30] = INIT1_STRING;
  /* Clear display */
  _lcd_clear();

  /* Print screen */
  _lcd_print_string(string, INIT1_STRING_X_START, INIT1_STRING_Y_START);
  /* Display */
  nrf_gfx_display(lcd_instance);
}

/*
 * @brief Print the second init screen.
 */
void _print_second_init(void) {
  char string[] = INIT2_STRING;
  /* Clear screen */
  _lcd_clear();

  /* Print screen */
  _lcd_print_string(string, INIT2_STRING_X_START, INIT2_STRING_Y_START);

  /* Display */
  nrf_gfx_display(lcd_instance);
}

/*
 * @brief Clear a given page on the screen.
 *
 * @paramin page Number of the page to erase  
 */
void _lcd_page_clear(uint8_t page) {
  /* Clear lines before infobar */
  if(page == INFOBAR_PAGE) {
    /* Clear a line */
    const nrf_gfx_line_t line_plot1 = {INFOBAR_X_START, 
      INFOBAR_Y_START - 2, 
      INFOBAR_X_SIZE,
      INFOBAR_Y_START - 2,
      LINE_THICKNESS}; 

    /* Clear a line */
    const nrf_gfx_line_t line_plot2 = {INFOBAR_X_START, 
      INFOBAR_Y_START - 1, 
      INFOBAR_X_SIZE,
      INFOBAR_Y_START - 1,
      LINE_THICKNESS}; 
    nrf_gfx_line_draw(lcd_instance, &line_plot1, 0);
    nrf_gfx_line_draw(lcd_instance, &line_plot2, 0);
  }
  if(page == ACTIVE_AREA_PAGE6) {
    const nrf_gfx_rect_t rect = {
      .x = 0,
      .y = page * SSD1309_PAGES,
      .width = nrf_gfx_width_get(lcd_instance),
      .height = SSD1309_PAGES - 2
    };
    nrf_gfx_rect_draw(lcd_instance, &rect, 1, 0, true);
  } else {
    const nrf_gfx_rect_t rect = {
      .x = 0,
      .y = page * SSD1309_PAGES,
      .width = nrf_gfx_width_get(lcd_instance),
      .height = SSD1309_PAGES
    };
    nrf_gfx_rect_draw(lcd_instance, &rect, 1, 0, true);
  }
  
}

/*
 * @brief Erase the whole screen.
 */
void _lcd_clear(void) {
  nrf_gfx_screen_fill(lcd_instance, 0);
}

/*
 * @brief Print the status bar.
 */
void _lcd_print_status_bar(void) {

  _lcd_page_clear(STATBAR_PAGE);

  /* Print Battery */
  _lcd_print_battery();
  
  /* Print Antenna */
  _lcd_print_antenna_signal();

  /* Print GPS */
  _lcd_print_gps();
  
  /* Display */
  nrf_gfx_display(lcd_instance);
  _print_statbar_last_timestamp = rtc_get_milliseconds();
  _statbar_first_time = true;

}

/*
 * @brief Print the homepage house.
 */
void _lcd_print_house(void) {
  static uint8_t *choosen_house;
   choosen_house = _house_image;
  _lcd_print_array_block(choosen_house, INFOBAR_X_START + INFOBAR_HOUSE_X_OFFSET , INFOBAR_Y_START, INFOBAR_HOUSE_X_SIZE, INFOBAR_HOUSE_Y_SIZE);

}
/*
 * @brief Print the info bar.
 */
void _lcd_print_info_bar(void) {
  
  /* Clear the infobar */
  _lcd_page_clear(INFOBAR_PAGE);
  
  /* Draw a line */
  const nrf_gfx_line_t line_plot = {INFOBAR_X_START, 
    INFOBAR_Y_START - 2, 
    INFOBAR_X_SIZE,
    INFOBAR_Y_START - 2,
    LINE_THICKNESS}; 
  
  nrf_gfx_line_draw(lcd_instance, &line_plot, 1);

  /* Check on what screen we are */
  switch(current_display_state) {
    case DISPLAY_HOMEPAGE:
      /* Print the house */
      _lcd_print_house();
      break;
  }
  

  /* Display */
  nrf_gfx_display(lcd_instance);
  _print_infobar_last_timestamp = rtc_get_milliseconds();
  _infobar_first_time = true;
  return;
}

/*
 * @brief Control the first time printed flags.
 */
void _lcd_reset_state_flags(void) {

  switch(current_display_state) {
    case DISPLAY_HOMEPAGE:
      _devpage_first_time = false;
      _temppage_first_time = false;
      _ecgpage_first_time = false;
      _main_menu_first_time = false;
      break;
    case DISPLAY_ECGPAGE:
      _devpage_first_time = false;
      _homepage_first_time = false;
      _temppage_first_time = false;
      _main_menu_first_time = false;
      break;
    case DISPLAY_TEMPPAGE:
      _devpage_first_time = false;
      _homepage_first_time = false;
      _ecgpage_first_time = false;
      _main_menu_first_time = false;
      break;
    case DISPLAY_DEVPAGE:
      _homepage_first_time = false;
      _temppage_first_time = false;
      _ecgpage_first_time = false;
      _main_menu_first_time = false;
      break;
    case DISPLAY_MAIN_MENU:
      _devpage_first_time = false;
      _homepage_first_time = false;
      _temppage_first_time = false;
      _ecgpage_first_time = false;
      break;
  }
}
/********************************** Public ************************************/
/*
 * @brief Initializes the driver, and stores the provided callbacks.
 *
 * @paramin lcd_system_is_in_standby_callback System status function pointer.
 * @paramin lcd_is_gps_active_callback GPS status function pointer.
 * @paramin lcd_get_battery_percentage_callback Battery percentage function pointer.
 * @paramin lcd_get_charge_source_callback Charge status function pointer.
 * @paramin lcd_get_signal_percentage_callback Signal percentage function pointer.
 * @paramin lcd_center_button_pressed_callback Center button status function pointer.
 * @paramin lcd_left_button_pressed_callback Left button status function pointer.
 * @paramin lcd_right_button_pressed_callback Right button status function pointer.
 * @paramin lcd_up_button_pressed_callback Up button status function pointer.
 * @paramin lcd_down_button_pressed_callback Down button status function pointer.
 * @paramin lcd_is_battery_saving_active_callback Battery saving mode status function pointer.
 * @paramin lcd_get_temperature_percentage_callback Temperature percentage function pointer.
 */
void lcd_init(lcd_system_is_in_standby_callback_def lcd_system_is_in_standby_callback,
              lcd_is_gps_active_callback_def lcd_is_gps_active_callback,
              lcd_get_battery_percentage_callback_def lcd_get_battery_percentage_callback,
              lcd_get_charge_source_callback_def lcd_get_charge_source_callback,
              lcd_get_signal_percentage_callback_def lcd_get_signal_percentage_callback,
              lcd_center_button_pressed_callback_def lcd_center_button_pressed_callback,
              lcd_left_button_pressed_callback_def lcd_left_button_pressed_callback,
              lcd_right_button_pressed_callback_def lcd_right_button_pressed_callback,
              lcd_up_button_pressed_callback_def lcd_up_button_pressed_callback,
              lcd_down_button_pressed_callback_def lcd_down_button_pressed_callback,
              lcd_is_battery_saving_active_callback_def lcd_is_battery_saving_active_callback,
              lcd_get_temperature_percentage_callback_def lcd_get_temperature_percentage_callback) { 
  
  /* Let's initialize the driver */
  if(nrf_gfx_init(lcd_instance) != NRF_SUCCESS) {
    debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[lcd_ui_init] error, LCD didn't initialize\n");
  }
  _printing_plot = false;
  _first_screen_first_time = false;
  _second_screen_first_time = false;
  current_display_state = LCD_DRIVER_INITIALIZING;
    
  /* Clear Display */
  _lcd_clear();
  nrf_gfx_display(lcd_instance);

  _lcd_system_is_in_standby_callback = lcd_system_is_in_standby_callback;
  _lcd_is_gps_active_callback = lcd_is_gps_active_callback;
  _lcd_get_battery_percentage_callback = lcd_get_battery_percentage_callback;
  _lcd_get_signal_percentage_callback = lcd_get_signal_percentage_callback;
  _lcd_get_charge_source_callback = lcd_get_charge_source_callback;
  _lcd_center_button_pressed_callback = lcd_center_button_pressed_callback;
  _lcd_down_button_pressed_callback = lcd_down_button_pressed_callback;
  _lcd_left_button_pressed_callback =  lcd_left_button_pressed_callback;
  _lcd_right_button_pressed_callback = lcd_right_button_pressed_callback;
  _lcd_up_button_pressed_callback = lcd_up_button_pressed_callback;
  _lcd_is_battery_saving_active_callback = lcd_is_battery_saving_active_callback;
  _lcd_get_temperature_percentage_callback = lcd_get_temperature_percentage_callback;
  return;
}

/* 
 * @brief 
 */
void lcd_loop(void) {

  switch(current_display_state) {
    case LCD_DRIVER_INITIALIZING:
      if(ssd1309_get_init_status()) {
        current_display_state = DISPLAY_INITIALIZING;
      } else {
        nrf_gfx_init(lcd_instance);
      }
      break;
    case DISPLAY_INITIALIZING:
      /* First Screen */
      if(!_first_screen_first_time) {
        _print_first_init();
        _printed_init_last_timestamp = rtc_get_milliseconds();
        _first_screen_first_time = true;
      /* Second Screen */
      } else if(((rtc_get_milliseconds() - _printed_init_last_timestamp) > INIT1_SCREEN_DELAY) && !_second_screen_first_time) {
        _print_second_init();
        _second_screen_first_time = true;
      }
      /* Check if system has initialized completely */
      if(_second_screen_first_time && _lcd_system_is_in_standby_callback()) {
        current_display_state = DISPLAY_HOMEPAGE;
        /* Clear Screen */
        _lcd_clear();

        /* Display */
        nrf_gfx_display(lcd_instance);
      }
      break;

    /* ### Homepage ### */
    case DISPLAY_HOMEPAGE:
      /* On the first run, clear display */
      if(!_homepage_first_time) {
        _lcd_active_area_clear();

        /* Print the respective infobar */
        _lcd_print_info_bar();
        _lcd_print_big_temp();
        /* Display */
        nrf_gfx_display(lcd_instance);
        _lcd_reset_state_flags();
        _printed_homepage_last_timestamp = rtc_get_milliseconds();
        _homepage_first_time = true;
      }
      if((rtc_get_milliseconds() - _printed_homepage_last_timestamp) > LCD_ACTIVE_AREA_REFRESH_RATE) {
        _printed_homepage_last_timestamp = rtc_get_milliseconds();
        _lcd_print_big_temp();
      }
      
      /* Enter main menu */
      if(_lcd_center_button_pressed_callback()) {
        current_display_state = DISPLAY_MAIN_MENU;

        /* Clear Active Area */
        _lcd_active_area_clear();
      }
      break;

    case DISPLAY_ECGPAGE:
      if(!_ecgpage_first_time) {
        _lcd_print_string_big("ECG Screen", 20, 16);

        /* Print the respective infobar */
        _lcd_print_info_bar();
        _ecgpage_first_time = true;
        _lcd_reset_state_flags();
      }
      break;
    case DISPLAY_TEMPPAGE:
      if(!_temppage_first_time) {
      
        /* Print the respective infobar */
        _lcd_print_info_bar();

        _lcd_print_string_big("TEMP Screen", 15, 16);
        _temppage_first_time = true;
        _lcd_reset_state_flags();
      }
      break;
    case DISPLAY_DEVPAGE:
      if(!_devpage_first_time) {
      
        /* Print the respective infobar */
        _lcd_print_info_bar();

        _lcd_print_string_big("DEV Screen", 20, 16);
        _devpage_first_time = true;
        _lcd_reset_state_flags();
      }
      break;
    
    /* ### Main Menu ### */
    case DISPLAY_MAIN_MENU:
      if(!_main_menu_first_time) {
      
        /* Print the respective infobar */
        _lcd_print_info_bar();

        _main_menu_first_time = true;
        _lcd_reset_state_flags();
        _lcd_print_grid();
      }

      /* Enter homepage */
      if(_lcd_center_button_pressed_callback()) {
        current_display_state = DISPLAY_HOMEPAGE;

        /* Clear Active Area */
        _lcd_active_area_clear();
      }
      break;

  }

  /* Slide to the next screen */
  if(_lcd_left_button_pressed_callback() && (current_display_state > FIRST_SCREEN) && (current_display_state <= LAST_SCREEN)) {
    current_display_state--;
    /* Clear Active Area */
    _lcd_active_area_clear();
    }

  /* Slide to the next screen */
  if(_lcd_right_button_pressed_callback() && (current_display_state >= FIRST_SCREEN) && (current_display_state < LAST_SCREEN)) {
    current_display_state++;
    /* Clear Active Area */
    _lcd_active_area_clear();
  }
  
  /* Funtionality common to all menus */
  /* Print status bar */
  if((((rtc_get_milliseconds() - _print_statbar_last_timestamp) > LCD_STATBAR_REFRESH_RATE) || !_statbar_first_time) && (_second_screen_first_time)) {
    _lcd_print_status_bar();
    
  }
  
  /* Print information bar */
  if(((rtc_get_milliseconds() - _print_infobar_last_timestamp) > LCD_INFOBAR_REFRESH_RATE) && (_second_screen_first_time)) {
    _lcd_print_info_bar();
  }
  /* Enter in power saving mode */
  if(_lcd_is_battery_saving_active_callback()) {
    /* Clear Active Area */
    _lcd_active_area_clear();
    _lcd_print_string_big("Shutting Down...", 10, 24);
   
    /* Display */
    nrf_gfx_display(lcd_instance);
    nrf_delay_us(1000000);
    _lcd_clear();

    /* Display */
    nrf_gfx_display(lcd_instance);
    ssd1309_power_mode();
  }

} /* End of loop */

/**
 * Asks if the application is running.
 * @return True if it is busy right now, false otherwise.
 */
bool lcd_is_busy(void) {
  
  if(_second_screen_first_time) {    
    return true;
  } else {
    return false;
  }
}

#endif 

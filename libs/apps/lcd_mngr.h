/*
* @file		lcd_mngr.h
* @date		August 2021
* @author	PFaria & JAntunes
*
* @brief	This is the header for LCD application.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef LCD_MNGR_H_
#define LCD_MNGR_H_

/********************************** Includes ***********************************/
/* Config */
#include "config.h"

#if MICROCONTROLER_2
/* SDK */
#include "nrf_gfx.h"

/* standard library */
#include <stdint.h>
#include <stdbool.h>

/********************************** Definitions ***********************************/
/* Application states */
typedef enum {
  LCD_DRIVER_INITIALIZING = 0,
  DISPLAY_INITIALIZING,
  DISPLAY_HOMEPAGE,
  DISPLAY_ECGPAGE,
  DISPLAY_TEMPPAGE,
  DISPLAY_DEVPAGE,
  DISPLAY_MAIN_MENU
} lcd_display_states; 

/* Screens */
#define FIRST_SCREEN                  DISPLAY_HOMEPAGE    /* First screen */
#define LAST_SCREEN                   DISPLAY_DEVPAGE     /* Last screen */

/* General active area */
#define ACTIVE_AREA_X_SIZE            SSD1309_WIDTH   /* X size of the area used by every menu, in pixels */
#define ACTIVE_AREA_Y_SIZE            48    /* Y size of the area used by every menu, in pixels */
#define ACTIVE_AREA_X_START           0     /* X value where the area used by every menu begins, in pixels */
#define ACTIVE_AREA_Y_START           15    /* Y value where the area used by every menu begins, in pixels */
#define ACTIVE_AREA_TEMP_X_SIZE       19    /* Size of the big temperature symbol on X */
#define ACTIVE_AREA_TEMP_Y_SIZE       35    /* Size of the big temperatur symbol on Y */
#define ACTIVE_AREA_TEMP_X_OFFSET     20    /* Offset from origin to print the big temperature symbol, on X */
#define ACTIVE_AREA_TEMP_Y_OFFSET     0     /* Offset from origin to print the big temperature symbol, on Y */

#define COLOR_WHITE                   1     /* Value we use to print solid color */
#define COLOR_BLACK                   0     /* Value we use to print no color */

/* Graph plot area */
#define PLOT_TIMESTAMP                4     /* Number of pixels per sample on a plot */
#define PLOT_MAX_X_POINTS             ACTIVE_AREA_X_SIZE / PLOT_TIMESTAMP  /* X size of the plotable area in pixels */
#define LINE_THICKNESS                1     /* Line thickness */

/* Big Temperature thresholds */
#define BIG_TEMP_LOW_THOLD            16     /* Used to select the Big Temperature symbol */
#define BIG_TEMP_HIGH_THOLD           40     /* Used to select the Big Temperature symbol */

/* Battery thresholds */
#define BATTERY_0PERCENT_THOLD        10     /* Used to select the battery symbol */
#define BATTERY_25PERCENT_THOLD       40     /* Used to select the battery symbol */
#define BATTERY_50PERCENT_THOLD       65     /* Used to select the battery symbol */
#define BATTERY_75PERCENT_THOLD       90     /* Used to select the battery symbol */
#define CHARGE_INDICATOR_WIRELESS     1      /* Used to select the charge source symbol */     
#define CHARGE_INDICATOR_USB          2      /* Used to select the charge source symbol */     

/* Antenna thresholds */
#define ANTENNA_00PERCENT_THOLD       5      /* Used to select the antenna symbol */
#define ANTENNA_33PERCENT_THOLD       50     /* Used to select the antenna symbol */
#define ANTENNA_66PERCENT_THOLD       75     /* Used to select the antenna symbol */

/* Initialization screen */
#define INIT1_SCREEN_DELAY            2000   /* Delay to switch from init1 screen to init2 screen */
#define INIT1_STRING_X_START          25
#define INIT1_STRING_Y_START          16
#define INIT2_STRING_X_START          20
#define INIT2_STRING_Y_START          28

/* Status bar */
#define STATBAR_X_START               0   
#define STATBAR_Y_START               0 
#define STATBAR_X_SIZE                SSD1309_WIDTH 
#define STATBAR_Y_SIZE                8
#define STATBAR_GPS_X_OFFSET          58
#define STATBAR_ANTENNA_X_OFFSET      112
#define STATBAR_CHG_SOURCE_X_OFFSET   STATBAR_BATTERY_X_SIZE + 2

#define STATBAR_BATTERY_X_SIZE        15    /* Size of the battery symbol on X */
#define STATBAR_BATTERY_Y_SIZE        8     /* Size of the battery symbol on Y */
#define STATBAR_CHG_SOURCE_X_SIZE     8    /* Size of charge source symbol on X */
#define STATBAR_CHG_SOURCE_Y_SIZE     8     /* Size of charge source symbol on Y */
#define STATBAR_ANTENNA_X_SIZE        15    /* Size of the antena symbol on X */
#define STATBAR_ANTENNA_Y_SIZE        8     /* Size of the antena symbol on Y */
#define STATBAR_GPS_X_SIZE            11    /* Size of the GPS symbol on X */
#define STATBAR_GPS_Y_SIZE            8     /* Size of the GPS symbol on X */

/* Information bar */
#define INFOBAR_X_START               0   
#define INFOBAR_Y_START               INFOBAR_PAGE * SSD1309_PAGE_LINES 
#define INFOBAR_X_SIZE                SSD1309_WIDTH 
#define INFOBAR_HOUSE_X_SIZE         8                 /* Size of the house symbol on Y */
#define INFOBAR_HOUSE_Y_SIZE         8                 /* Size of the house symbol on Y */
#define INFOBAR_HOUSE_X_OFFSET       (INFOBAR_X_SIZE / 2) - (INFOBAR_HOUSE_X_SIZE / 2)
#define INFOBAR_MAX_CHARS            (uint8_t)((SSD1309_WIDTH - INFOBAR_X_START)/SSD1309_PAGE_LINES)

/* Strings */
#define INIT2_STRING                  "Initializing..."   /* String to be written on init1 */
#define INIT1_STRING                  "SENSEFINITY \n\n\n      ISEL"   /* String to be written on init1 */
#define GPS_NAME_STRING               "GPS:"   /* GPS name string */
#define GPS_ON_STRING                 "ON"   /* GPS on string */
#define GPS_NOK_STRING                "NOK"   /* GPS not ok string */
#define GPS_OFF_STRING                "off"   /* GPS off string */
#define HOMEPAGE_INFOBAR_STRING       "HOMEPAGE"

/* Pages */ //Should be on the driver
#define STATBAR_PAGE                 0     /* Page 0 */
#define ACTIVE_AREA_PAGE1            1     /* Page 1 */
#define ACTIVE_AREA_PAGE2            2     /* Page 2 */
#define ACTIVE_AREA_PAGE3            3     /* Page 3*/
#define ACTIVE_AREA_PAGE4            4     /* Page 4 */
#define ACTIVE_AREA_PAGE5            5     /* Page 5 */
#define ACTIVE_AREA_PAGE6            6     /* Page 6 */
#define INFOBAR_PAGE                 7     /* Page 0 */

/* Callback functions needed */
typedef bool (*lcd_system_is_in_standby_callback_def)(void);            /* LCD has the system entered in standby callback function definition. */
typedef bool (*lcd_is_gps_active_callback_def)(void);                   /* LCD is GPS active callback function definition. */
typedef uint8_t (*lcd_get_battery_percentage_callback_def)(void);       /* LCD get battery percentage callback function definition. */
typedef uint8_t (*lcd_get_charge_source_callback_def)(void);            /* LCD get charge source callback function definition. */
typedef uint8_t (*lcd_get_signal_percentage_callback_def)(void);        /* LCD get signal intensity percentage callback function definition. */
typedef bool (*lcd_center_button_pressed_callback_def)(void);           /* LCD center button has been pressed callback function definition. */
typedef bool (*lcd_left_button_pressed_callback_def)(void);             /* LCD left button has been pressed callback function definition. */
typedef bool (*lcd_right_button_pressed_callback_def)(void);            /* LCD right button has been pressed callback function definition. */
typedef bool (*lcd_up_button_pressed_callback_def)(void);               /* LCD up button has been pressed callback function definition. */
typedef bool (*lcd_down_button_pressed_callback_def)(void);             /* LCD down button has been pressed callback function definition. */
typedef bool (*lcd_is_battery_saving_active_callback_def)(void);        /* LCD enter power saving mode callback function definition. */
typedef uint8_t (*lcd_get_temperature_percentage_callback_def)(void);   /* LCD enter power saving mode callback function definition. */

/********************************** Functions ***********************************/
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
              lcd_get_temperature_percentage_callback_def lcd_get_temperature_percentage_callback);
void lcd_loop(void);
bool lcd_is_busy(void);

#endif 
#endif /* LCD_MNGR_H_ */



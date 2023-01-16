/*
* @file		main.c
* @date		June 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the main entry point
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/
/* Config */
#include "config.h"

/* Drivers */
#include "charge.h"

/* Utilities */
#include "twi_mngr.h"
#include "spi_mngr.h"
#include "power_management.h" /* Based on Sensifinity Company Code */

/* Apps */
#include "meas_mngr.h"
//#include "app_timer.h"

/* SDK */
#include "nrf_drv_gpiote.h"

/* Sensefinity Includes */
/* WDT */
#include "nrf_drv_wdt.h"

/* Periph - evian */
#include "sense_library/periph/lfclk.h"
#include "sense_library/periph/rtc.h"

/* C standard library */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/* Debug - evian */
#include "sense_library/debug/log.h"

/* Debug - libs */
#include "sense_library/utils/debug.h"

/* nRF power management */
#include "nrf_pwr_mgmt.h"

#if MICROCONTROLER_1
/* Utilities */
#include "twi_mngr.h"

/* Apps */
#include "app_bodytemp.h"
#include "app_ecg.h"
#endif 

#if MICROCONTROLER_2
/* Drivers */
#include "btn.h"

/* Utilities */
#include "spi_mngr.h"

/* Apps */
#include "lcd_mngr.h"
#include "sense_library/apps/gps.h"
#include "sense_library/apps/cells.h"
#include "sense_library/apps/rssi.h"

/* Components - evian */
#include "sense_library/components(telco)/telco.h"

/* Factory page utils - evian */
#include "sense_library/sensoroid(evian)/factory.h"

/* Queues - libs */
#include "sense_library/sensoroid/gama_queues.h"

/* Sequence number - libs */
#include "sense_library/utils/sequence_number.h"

/* Hello */
#include "sense_library/sensoroid(evian)/hello.h"

/* Reset reason */
#include "sense_library/sensoroid(evian)/reset_reason.h"

/* Measurements buffer */
  #include "sense_library/sensoroid/measurements_v2_manager.h"

#endif 


/********************************** Private ************************************/
/**
 * Tells if reset reason measurement was already stored.
 */
static bool _reset_reason_measurement_added = false;

/**
 * Tells if hello message was already stored.
 */
static bool _hello_message_added = false;

/**
 * WDT channel ID.
 */
nrf_drv_wdt_channel_id m_channel_id;

/*
 *  Variable to get application iddle status
 */
static bool _apps_iddle = false;

/* ####### APAGAR!!!! ######## */
uint8_t apagar_battery_percentage = 0;
uint8_t apagar_signal_percentage = 0;
uint8_t apagar_charge_source = 0;
uint64_t apagar_battery_percentage_timestamp = 0;
uint64_t apagar_signal_percentage_timestamp = 0;
uint64_t apagar_charge_source_timestamp = 0;
uint64_t apagar_gps_status_timestamp = 0;
bool apagar_gps_status = false;

/**
 * Shutdown device utility.
 */
void device_shutdown(void) {  
  /* To Do */
}

/**
 * Put device in sleep mode.
 */
void device_sleep(void) {
  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[device_sleep] going to sleep\n");

  nrf_pwr_mgmt_run();
}

/**
 * WDT events handler. Note that the maximum amount of time we can spend
 * in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs.
 */
void wdt_event_handler(void) {
  debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[wdt_event_handler] wdt happened\n");

  return;
}

/*
 * Function to get system status 
 */
bool system_is_in_iddle(void) {
  if(_apps_iddle) {
    return true;
  }
  return false;
}

/* ******************************** Dummy ********************************** */
bool app_extram_get_init_status_callback_dummy() {
  return true;
}

#if MICROCONTROLER_1  
charge_data _charge_data_v;
charge_data *app_extram_get_battery_data_callback_dummy() {
      
  _charge_data_v.battery_mode = false;
  _charge_data_v.last_read_timestamp = rtc_get_milliseconds();  

  return &_charge_data_v;

}
#endif

uint8_t get_temperature_percentage() {
  return 30;
}

uint8_t get_signal_percentage(){
  if((rtc_get_milliseconds() - apagar_signal_percentage_timestamp) > 500) {
    apagar_signal_percentage += 10;
    apagar_signal_percentage_timestamp = rtc_get_milliseconds();
  }
  if(apagar_signal_percentage > 100) {
    apagar_signal_percentage = 0;
  }
  return apagar_signal_percentage;
}

bool is_gps_active(void) {
  if((rtc_get_milliseconds() - apagar_gps_status_timestamp) > 500) {
    apagar_gps_status = !apagar_gps_status;
    apagar_gps_status_timestamp = rtc_get_milliseconds();
  }
  return apagar_gps_status;

}


/* ******************************** Public *********************************** */
/**
 * Main entry point of the firmware program.
 */
 int main(void) {
  
  #ifdef DEBUG
    log_init();
  #endif 
  
  debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"\n\n\n\n\n");  

  /* Clock setup */
  lfclock_init();
  
  /* RTC setup */
  rtc_init();

  /* This init must be done prior to ads1114, ads129x and saadc USE */
  nrf_drv_gpiote_init();  
  
  /* This init must be done prior to any TWI/I2C use*/
  #if MICROCONTROLER_1                                    
  twi_mngr_init(TWI_MNGR_CONFIG1_INSTANCE);                 /* Init TWI for Temperature Sensor */
  #endif                                                   

  /* Add here any pior init to use */
  #if MICROCONTROLER_1
  spi_mngr_init(SPI_MNGR_CONFIG1_INSTANCE);                 /* Init SPI for ECG sensor */
  #endif 
  #if !USD_ACTIVE
  spi_mngr_init(SPI_MNGR_CONFIG3_INSTANCE);                 /* Init SPI for RAM and LDC */
  #endif 
  
  /* *** Sensefinity Company Code Init *** */
  /* WDT configuration */
  nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
  nrf_drv_wdt_init(&config, wdt_event_handler);
  nrf_drv_wdt_channel_alloc(&m_channel_id);
  nrf_drv_wdt_enable();
  
  #if MICROCONTROLER_1
  power_management_init(POWER_MANAGEMENT_WAITING_FOR_ACTION, 
    device_shutdown, 
    NULL,
    NULL,
    NULL,
    NULL);
  #endif 

  #if MICROCONTROLER_2
  /* Reads and process factory page */
  factory_init();
  //sequency_number_init();                                 /* Check if usefull !!!!!!!!!!!! */
  gama_queues_init();
  measurements_v2_manager_init();
  
  power_management_init(POWER_MANAGEMENT_WAITING_FOR_ACTION, 
    device_shutdown, 
    telco_send_battery_saving_mode_message_autonomously,
    charge_get_battery_type_information().enter_battery_saving_voltage,
    charge_get_battery_type_information().leaving_battery_saving_warmup_voltage,
    charge_get_battery_type_information().leaving_battery_saving_voltage);
  #endif 
  
  debug_printf_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_printf_string(DEBUG_LEVEL_0, (uint8_t*)"[main init] Init Main Done\n");
      
  #if MICROCONTROLER_2
  /* Adds reset reason measurement */
  if(reset_reason_add_measurement()) {
    reset_reason_clear_register();
    _reset_reason_measurement_added = true;
  }
  #endif 

  power_management_set_state(POWER_MANAGEMENT_POWER_ON);
  power_management_loop();
  
  #if MICROCONTROLER_2
  /* Create hello with basic info (such as ble address, firmware version, etc.) */
  _hello_message_added = hello_add_basic_info_message(rtc_get_milliseconds());
  
  telco_init();
  
  /* Call gps init function */
  gps_init(cellular_start_collecting_gps, 
    cellular_stop_collecting_gps, 
    cellular_get_gps_sampling_rate, 
    cellular_is_collecting_gps_sample, 
    cellular_support_gps_service);
  /* Call cells init function */
  cells_init(telco_support_cells_service,
    telco_collect_cells,
    telco_has_cells_data,
    telco_read_cells_data,
    telco_is_collecting_cells_sample);
  /* Call rssi init function */
  rssi_init(telco_get_rssi_data);
  #endif 
  
  /**************************/
  /* Insert here apps init  */
  /**************************/  
  #if MICROCONTROLER_1
  charge_init(APP_CHARGE_SAMPLING_RATE, app_extram_get_init_status_callback_dummy, app_extram_get_battery_data_callback_dummy);
  app_body_temperature_init(APP_BODY_TEMP_SAMPLING_RATE);
  app_ecg_init(power_management_is_battery_saving_mode_active);
  #endif 
  
  #if MICROCONTROLER_2    
  charge_init(APP_CHARGE_SAMPLING_RATE, NULL, NULL);
  lcd_init(system_is_in_iddle,
    is_gps_active,
    charge_get_percentage,
    charge_get_external_power_status,
    get_signal_percentage,
    btn_center_get_status,
    btn_left_get_status,
    btn_right_get_status,
    btn_upper_get_status,
    btn_bottom_get_status,
    power_management_is_battery_saving_mode_active,
    get_temperature_percentage);
  #endif   
  
  /**************************/
  /* Inicialize apps loops  */
  /**************************/
  while(charge_is_busy()) {
    charge_loop();
  }  
  meas_mngr_init();
  #if MICROCONTROLER_1
  while(app_body_temperature_is_busy()) {
    app_body_temperature_loop();
  }
  while(app_ecg_is_busy()) {
    app_ecg_loop();
  }
  #endif 
  
  #if MICROCONTROLER_2      
  while(lcd_is_busy()) {
    lcd_loop();
  } 
  #endif 
  
  _apps_iddle = true;

  debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[main] Program init ended. now going to main loop\n");

  uint64_t last_timestamp = rtc_get_milliseconds();

  /* ECG dummies */
  #if MICROCONTROLER_1
  app_ecg_config(false, false, 1, 1, ADS129X_SUPPLY);
  app_ecg_start_sample();
  #endif

  /* Main Loop */
  while(true) {
    /* Feed WDT */
    nrfx_wdt_feed();
    
    uint64_t current_timestamp = rtc_get_milliseconds();
    
    /* Power management layer loop */
    power_management_loop();    
    
    #if MICROCONTROLER_2
    /* Prevention */
    if(!_hello_message_added) {
      _hello_message_added = hello_add_basic_info_message(current_timestamp);
    }

    /* Prevention */
    if(!_reset_reason_measurement_added && reset_reason_add_measurement()) {
      reset_reason_clear_register();
      _reset_reason_measurement_added = true;
    }

    /* If there are measurements in the measurements manager
     * push it to ram gama queue if device status is ok */
    if(!measurements_v2_manager_is_empty()) {
      if(measurements_v2_manager_send_measurements()) {
        measurements_v2_manager_delete_measurements();
      }
    }    
    #endif 

    /**************************/
    /* Insert here apps loops */
    /**************************/
    charge_loop();
    meas_mngr_loop();
    #if MICROCONTROLER_1
      app_body_temperature_loop();
      app_ecg_loop();
    #endif 
    
    #if MICROCONTROLER_2    
      telco_loop(current_timestamp);                /* Telco loop */
      lcd_loop();
    #endif      
         
    /* After every minute, perform some tasks */
    if((current_timestamp - last_timestamp) >= 10000) {
      debug_print_time(DEBUG_LEVEL_0, rtc_get_milliseconds());
      debug_print_string(DEBUG_LEVEL_0, (uint8_t*)"[main] Another minute...\n");  

      last_timestamp = current_timestamp;
    }
    
    /* Sleep decision */
    /**************************/
    /* Insert here apps busy  */
    /**************************/
    if(!charge_is_busy()) {
      //device_sleep();
    }
    #if MICROCONTROLER_1
    if(!app_body_temperature_is_busy()) {
      //device_sleep();
    }
    #endif 
    
    #if MICROCONTROLER_2    
    if(!telco_is_busy() && !lcd_is_busy()) {
      //device_sleep();
    }
    #endif

  }

  /* Program execution should never reach this statement */
  return EXIT_SUCCESS;
}





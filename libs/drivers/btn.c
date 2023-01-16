/*
* @file		button.c
* @date		April 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the button utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/

/* Interface */
#include "btn.h"

/* Pheriperals */
#include "sense_library/periph/rtc.h"

/* SDK */
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"

/* Debug - libs */
#include "sense_library/utils/debug.h"

/* Utils */
#include "sense_library/utils/utils.h"

/********************************** Private ************************************/
/*
* Variável de estado de inicialização do saadc
*/
static bool _btn_state_init = false;

/*
* Variável de tentativas de inicialização da driver
*/
static uint8_t _retries = 0;

/*
* Variável que armazena o status do botão esquerdo
*/
static bool _btn_left_status;

/*
* Variável que armazena o status do botão esquerdo
*/
static bool _btn_right_status;

/*
* Variável que armazena o status do botão inferior
*/
static bool _btn_bottom_status;

/*
* Variável que armazena o status do botão superior
*/
static bool _btn_upper_status;

/*
* Variável que armazena o status do botão central
*/
static bool _btn_center_status;

/* Estrutra de configuração de GPIOTE HOTOLO */
nrf_drv_gpiote_in_config_t config_hitolo = 
{
  .sense = NRF_GPIOTE_POLARITY_HITOLO,                
  .pull = NRF_GPIO_PIN_PULLUP,                        
  .is_watcher = false,                                
  .hi_accuracy = false,                             
  .skip_gpio_setup = false 
};

/* Estrutra de configuração de GPIOTE LOTOHI */
nrf_drv_gpiote_in_config_t config_toggle = 
{
  .sense = NRF_GPIOTE_POLARITY_TOGGLE,                
  .pull = NRF_GPIO_PIN_PULLUP,                        
  .is_watcher = false,                                
  .hi_accuracy = false,                             
  .skip_gpio_setup = false 
};

/*
* Variável que armazena o timestamp de botão pressed
*/
uint64_t _btn_center_pressed_timestamp;

/*
* Variável que armazena o timestamp de botão released
*/
uint64_t _btn_center_released_timestamp;

/*
* Variável que armazena o tempo de botão premido
*/
uint64_t _btn_center_pressed_time;

/*
* Variável que armazena o estado para o toggle do botão central
*/
static bool _btn_center_toggle;

/*
* Variável que armazena o estado do botão central para desligar por timeout
*/
static bool _btn_center_early_long_status;

/* BTN callback functions list */
void btn_left_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void btn_right_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void btn_bottom_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void btn_upper_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
void btn_center_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

/* Private functions list */
#if BTN_TEST_DK
void btn_init_leds(void);
void btn_led_toggle(uint8_t led);
#endif


/* *********************************** Public *********************************** */
/*
 * @brief Function for initializing BTN driver
 *
 * @retval  True if initialization was successful, otherwise false
 */
bool btn_init(void) {
  
  /* Caso a driver já tenha sido inicializada com sucesso */
  if(_btn_state_init) {                                                             
    return true;
  }

  /* Caso o número de tentativas de inicialização tenha ultrapassado */
  if(_retries >= BTN_RETRIES) {
    return false;
  }
   
  #if BTN_TEST_DK
  btn_init_leds();
  #endif

  /* Inicialização de varáveis globais */
  _btn_left_status = BTN_NOT_PRESSED;
  _btn_right_status = BTN_NOT_PRESSED;
  _btn_bottom_status = BTN_NOT_PRESSED;
  _btn_upper_status = BTN_NOT_PRESSED;
  _btn_center_status = BTN_NOT_PRESSED;
  _btn_center_pressed_timestamp = 0;
  _btn_center_released_timestamp = 0;
  _btn_center_pressed_time = 0;  
  _btn_center_toggle = false;
  _btn_center_early_long_status = false;

  /* Inicializar GPIOES PINS */
  if(nrf_drv_gpiote_in_init(BTN_LEFT_PIN, &config_hitolo, btn_left_cb) != NRF_SUCCESS) {
    _retries++;
    return false;
  }
  if(nrf_drv_gpiote_in_init(BTN_RIGHT_PIN, &config_hitolo, btn_right_cb) != NRF_SUCCESS) {
    _retries++;
    return false;
  }
  if(nrf_drv_gpiote_in_init(BTN_BOTTOM_PIN, &config_hitolo, btn_bottom_cb) != NRF_SUCCESS) {
    _retries++;
    return false;
  }
  if(nrf_drv_gpiote_in_init(BTN_UPPER_PIN, &config_hitolo, btn_upper_cb) != NRF_SUCCESS) {
    _retries++;
    return false;
  }
  if(nrf_drv_gpiote_in_init(BTN_CENTER_PIN, &config_toggle, btn_center_cb) != NRF_SUCCESS) {
    _retries++;
    return false;
  }
  
  /* Enable PINS interrupt */
  nrf_drv_gpiote_in_event_enable(BTN_LEFT_PIN, true);
  nrf_drv_gpiote_in_event_enable(BTN_RIGHT_PIN, true);
  nrf_drv_gpiote_in_event_enable(BTN_BOTTOM_PIN, true);
  nrf_drv_gpiote_in_event_enable(BTN_UPPER_PIN, true);
  nrf_drv_gpiote_in_event_enable(BTN_CENTER_PIN, true);
  
  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[btn_init] Left Initialized\r\n");

  _btn_state_init = true;
  return true;    
           
}


/*
 * @brief Function to check if driver initialization was successful 
 *
 * @return      status             Returns the status of driver
 *                                  true  - sucess
 *                                  false - failure
 */
bool btn_get_init_status(void) {

  return _btn_state_init;

}


/*
 * @brief Function to get button left status       
 *
 * @retval                  True if button was pressed or false otherwise.
 */
bool btn_left_get_status(void) {

  if(_btn_left_status) {
    _btn_left_status = BTN_NOT_PRESSED;
    return BTN_PRESSED;
  } else {
    return BTN_NOT_PRESSED;
  }

}


/*
 * @brief Function to get button right status       
 *
 * @retval                  True if button was pressed or false otherwise.
 */
bool btn_right_get_status(void) {

  if(_btn_right_status) {
    _btn_right_status = BTN_NOT_PRESSED;
    return BTN_PRESSED;
  } else {
    return BTN_NOT_PRESSED;
  }

}


/*
 * @brief Function to get button bottom status       
 *
 * @retval                  True if button was pressed or false otherwise.
 */
bool btn_bottom_get_status(void) {

  if(_btn_bottom_status) {
    _btn_bottom_status = BTN_NOT_PRESSED;
    return BTN_PRESSED;
  } else {
    return BTN_NOT_PRESSED;
  }

}


/*
 * @brief Function to get button upper status       
 *
 * @retval                  True if button was pressed or false otherwise.
 */
bool btn_upper_get_status(void) {

  if(_btn_upper_status) {
    _btn_upper_status = BTN_NOT_PRESSED;
    return BTN_PRESSED;
  } else {
    return BTN_NOT_PRESSED;
  }

}


/*
 * @brief Function to get button center status       
 *
 * @retval                  True if button was pressed or false otherwise.
 */
bool btn_center_get_status(void) {

  if(_btn_center_status == BTN_NOT_PRESSED) {                                                   /* Botão não foi premido */
    return BTN_NOT_PRESSED;
  
  } else {                                                                                      /* Botão foi premido */

    if(_btn_center_pressed_time > BTN_DURATION_LONG_PRESS) {                                    /* Foi pressed de longa duração */
      return BTN_NOT_PRESSED;

    } else if(_btn_center_pressed_time < BTN_DURATION_SHORT_PRESS){                             /* Foi pressed de curta duração */

      _btn_center_status = BTN_NOT_PRESSED;   
      _btn_center_pressed_time = 0;      
      return BTN_PRESSED;
    }

  }  
  return BTN_NOT_PRESSED;
  
}


/*
 * @brief Function to get button center status for long press operation.    
 *
 * @retval                  True if button was pressed or false otherwise.
 */
bool btn_center_get_long_pressed_status(void) {

  /* Botão foi pressed e released */
  if(_btn_center_status == BTN_PRESSED) {  

    if(_btn_center_pressed_time > BTN_DURATION_LONG_PRESS) {                                    /* Foi pressed de longa duração */      
      _btn_center_status = BTN_NOT_PRESSED;
      _btn_center_pressed_time = 0;
      return BTN_PRESSED;
    } 
        
  }

  uint64_t btn_center_pressed_time = rtc_get_milliseconds() - _btn_center_pressed_timestamp;    /* Tempo de botão premido */
  
  /* Foi pressed de longa duração e o calback não ocorreu */
  if((_btn_center_status == BTN_NOT_PRESSED) && _btn_center_toggle) {

    if(btn_center_pressed_time > BTN_DURATION_LONG_PRESS) {   
      _btn_center_toggle = false;
      _btn_center_status = BTN_NOT_PRESSED;
      _btn_center_pressed_time = 0;
      _btn_center_early_long_status = true;      
      return BTN_PRESSED;
    }

  }


  
  /* Botão não foi pressed nem released */
  return BTN_NOT_PRESSED;

}


/* ********************************** Private *********************************** */
/*
 * @brief Function to handle pin for button left Status change from high to low callback.        
 *
 * @param[in] pin           Pin that triggered this event.
 * @param[in] action        Action that led to triggering this event.
 */
void btn_left_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

  if(pin != BTN_LEFT_PIN || action != NRF_GPIOTE_POLARITY_HITOLO) {
    return;
  }

  _btn_left_status = BTN_PRESSED;
  
  #if BTN_TEST_DK
  btn_led_toggle(LED1);
  #endif

  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[btn_left_cb] Left button pressed\r\n");

}


/*
 * @brief Function to handle pin for button right Status change from high to low callback.        
 *
 * @param[in] pin           Pin that triggered this event.
 * @param[in] action        Action that led to triggering this event.
 */
void btn_right_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

  if(pin != BTN_RIGHT_PIN || action != NRF_GPIOTE_POLARITY_HITOLO) {
    return;
  }

  _btn_right_status = BTN_PRESSED;

  #if BTN_TEST_DK
  btn_led_toggle(LED2);
  #endif

  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[btn_right_cb] Right button pressed\r\n");

}


/*
 * @brief Function to handle pin for button bottom Status change from high to low callback.        
 *
 * @param[in] pin           Pin that triggered this event.
 * @param[in] action        Action that led to triggering this event.
 */
void btn_bottom_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

  if(pin != BTN_BOTTOM_PIN || action != NRF_GPIOTE_POLARITY_HITOLO) {
    return;
  }

  _btn_bottom_status = BTN_PRESSED;

  #if BTN_TEST_DK
  btn_led_toggle(LED3);
  #endif

  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[btn_bottom_cb] Bottom button pressed\r\n");

}


/*
 * @brief Function to handle pin for button upper Status change from high to low callback.        
 *
 * @param[in] pin           Pin that triggered this event.
 * @param[in] action        Action that led to triggering this event.
 */
void btn_upper_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

  if(pin != BTN_UPPER_PIN || action != NRF_GPIOTE_POLARITY_HITOLO) {
    return;
  }

  _btn_upper_status = BTN_PRESSED;

  #if BTN_TEST_DK
  btn_led_toggle(LED4);
  #endif

  debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
  debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[btn_upper_cb] Upper button pressed\r\n");

}


/*
 * @brief Function to handle pin for button center Status change from high to low callback.        
 *
 * @param[in] pin           Pin that triggered this event.
 * @param[in] action        Action that led to triggering this event.
 */
void btn_center_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {

  if(pin != BTN_CENTER_PIN || action != NRF_GPIOTE_POLARITY_TOGGLE) {
    return;
  }  

  /* Já foi realizada a leitura como long press por timeout */
  if(_btn_center_early_long_status) {                                                           
    _btn_center_early_long_status = false;
    return;
  }
    
  if(!_btn_center_toggle) {                                                                     /* Button Pressed */
    _btn_center_toggle = true;                                 
    
    #if BTN_TEST_DK
    /* To Do */
    #endif

    _btn_center_pressed_timestamp = rtc_get_milliseconds();      

    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[btn_center_cb] Center button pressed\r\n");
  
  } else {                                                                                      /* Button Released */
                                                    
    _btn_center_toggle = false; 
    _btn_center_status = BTN_PRESSED;
    
    _btn_center_released_timestamp = rtc_get_milliseconds();
    
    _btn_center_pressed_time = _btn_center_released_timestamp - _btn_center_pressed_timestamp;  /* Tempo de botão premido */
    
    debug_print_time(DEBUG_LEVEL_1, rtc_get_milliseconds());
    debug_print_string(DEBUG_LEVEL_1, (uint8_t*) "[btn_center_cb] Center button released\r\n");
  }  

}


#if BTN_TEST_DK
/*
 * @brief Function to initialize LEDS of DK board.     
 */
void btn_init_leds(void) {
  
  /* Defenir leds como output */
  nrf_gpio_cfg_output(PIN_LED1);	
  nrf_gpio_cfg_output(PIN_LED2);
  nrf_gpio_cfg_output(PIN_LED3);
  nrf_gpio_cfg_output(PIN_LED4);

  /* Inicializar Leds desligados */
  nrf_gpio_pin_write(PIN_LED1, 1);
  nrf_gpio_pin_write(PIN_LED2, 1);
  nrf_gpio_pin_write(PIN_LED3, 1);
  nrf_gpio_pin_write(PIN_LED4, 1); 
}


/*
 * @brief Function to toogle LEDS status of DK board.  
 *
 * @param[in] led           Led to toggle.
 */
void btn_led_toggle(uint8_t led) {

  switch(led) {
    case LED1:
      nrf_gpio_pin_toggle(PIN_LED1);
      break;
    case LED2:
      nrf_gpio_pin_toggle(PIN_LED2);
      break;
    case LED3:
      nrf_gpio_pin_toggle(PIN_LED3);
      break;
    case LED4:
      nrf_gpio_pin_toggle(PIN_LED4);
      break;
    default:
      nrf_gpio_pin_write(PIN_LED1, 1);
      nrf_gpio_pin_write(PIN_LED2, 1);
      nrf_gpio_pin_write(PIN_LED3, 1);
      nrf_gpio_pin_write(PIN_LED4, 1); 
      break;
  }

}
#endif







/*
* @file		saadc.c
* @date		April 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the saadc adc utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef BTN_H_
#define BTN_H_

/********************************** Includes ***********************************/

/* SDK */
#include "nrf_drv_saadc.h"

/* Config */
#include "config.h"

/********************************** Definições ***********************************/


/* Macros Gerais */
#define BTN_RETRIES                 2                           /* Tentativas de inicialização da driver */
#define BTN_NOT_PRESSED             0
#define BTN_PRESSED                 1
#define BTN_DURATION_LONG_PRESS     2000
#define BTN_DURATION_SHORT_PRESS    700
#define BTN_TIMEOUT_RELEASED        6000

/* Macros para teste em DK */
#define BTN_TEST_DK                 1                           /* Colocar no config.h !!!!! */
#if BTN_TEST_DK
#define PIN_LED1                    NRF_GPIO_PIN_MAP(0, 17)
#define PIN_LED2                    NRF_GPIO_PIN_MAP(0, 18)
#define PIN_LED3                    NRF_GPIO_PIN_MAP(0, 19)
#define PIN_LED4                    NRF_GPIO_PIN_MAP(0, 20)

#define LED1                        1
#define LED2                        2
#define LED3                        3
#define LED4                        4
#endif

/********************************** Funções ***********************************/
bool btn_init(void);
bool btn_get_init_status(void);
bool btn_left_get_status(void);
bool btn_right_get_status(void);
bool btn_bottom_get_status(void);
bool btn_upper_get_status(void);
bool btn_center_get_status(void);
bool btn_center_get_long_pressed_status(void);

#endif /* BTN_H_ */



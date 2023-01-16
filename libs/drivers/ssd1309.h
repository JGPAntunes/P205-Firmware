/*
* @file		ssd1309.h
* @date		August 2020
* @author	PFaria & JAntunes
*
* @brief	This is the header for ssd1309 utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef SSD1309_H_
#define SSD1309_H_

/********************************** Includes ***********************************/

/* SDK */
#include "nrf_spi_mngr.h"
#include "nrf_lcd.h"

/* Config */
#include "config.h"

/********************************** Definições ***********************************/
/* Comandos */
#define SSD1309_OFF_CMD               0xAE                        /* Display OFF */
#define SSD1309_SET_CLOCKDIV_CMD      0xD5                        /* Set Display Clock Divide */
#define SSD1309_SET_CLOCKDIV_VALUE    0xF0                        /* Set Display Clock Divide - reset data */
#define SSD1309_SET_MULTIPLEX_CMD     0xA8                        /* Set Multiplex Ratio */
#define SSD1309_SET_MULTIPLEX_VALUE   0x3F                        /* Set Multiplex Ratio - reset data */
#define SSD1309_SET_DISPOFFSET_CMD    0xD3                        /* Set Display Offset */
#define SSD1309_SET_DISPOFFSET_VALUE  0x00                        /* Set Display Offset - reset data */
#define SSD1309_SET_STARTLINE_CMD     0x40                        /* Set Display Start Line - reset value */
#define SSD1309_SET_MEMORYMODE_CMD    0x20                        /* Set Memory Addressing Mode */
#define SSD1309_SET_MEMORYMODE_VALUE  0x02                        /* Set Memory Addressing Mode - reset data */
#define SSD1309_SET_SEGREMAP_CMD      0xA1                        /* Set Segment Re-map */ 
#define SSD1309_SET_COMOUTPUT_CMD     0xC8                        /* Set COM Output Scan Direction */ 
#define SSD1309_SET_COMPINS_CMD       0xDA                        /* Set COM Pins Hardware Configuration */
#define SSD1309_SET_COMPINS_VALUE     0X12                        /* Set COM Pins Hardware Configuration - reset data */
#define SSD1309_SET_CONTRAST_CMD      0x81                        /* Set Contrast Control */
#define SSD1309_SET_CONTRAST_VALUE    0x00                        /* Set Contrast Control - reset data */
#define SSD1309_SET_PRECHARGEP_CMD    0xD9                        /* Set Pre-charge Period */
#define SSD1309_SET_PRECHARGEP_VALUE  0xF1                        /* Set Pre-charge Period - reset data */
#define SSD1309_SET_VCOMH_CMD         0xDB                        /* Set VCOMH Deselect Level */
#define SSD1309_SET_VCOMH_VALUE       0x37                        /* Set VCOMH Deselect Level - reset data */
#define SSD1309_ENTIREDISP_ONRAM_CMD  0xA4                        /* Entire Display ON - output follows RAM content*/
#define SSD1309_NORLMALDISP_ON_CMD    0xA6                        /* Set Normal */
#define SSD1309_ON_CMD                0xAF                        /* Inverse Display */

#define SSD1309_ENTIREDISP_ON_CMD     0xA5                        /* Entire Display ON - ignores RAM content */
#define SSD1309_INVERSE_DISP_CMD      0xA7                        /* Set Normal */

/* Temporizações */
#define SSD1309_TON_CMD               180                         /* Tempo de espera do comando SSD1309_ON_CMD */

/* Endereços */
#define SSD1309_PAGE_0_ADDR           0xB0
#define SSD1309_PAGE_1_ADDR           0xB1
#define SSD1309_PAGE_2_ADDR           0xB2
#define SSD1309_PAGE_3_ADDR           0xB3
#define SSD1309_PAGE_4_ADDR           0xB4
#define SSD1309_PAGE_5_ADDR           0xB5
#define SSD1309_PAGE_6_ADDR           0xB6
#define SSD1309_PAGE_7_ADDR           0xB7

/* Macros Gerais */
#define SSD1309_RETRIES               4                           /* Tentativas de inicialização da driver */
#define SSD1309_HEIGHT                64
#define SSD1309_WIDTH                 128
#define SSD1309_PAGES                 8
#define SSD1309_PAGE_LINES            8
#define SSD1309_NBYTES_1              1
#define SSD1309_COLOR_ON              1
#define SSD1309_COLOR_OFF             0
#define SSD1309_REFRESH_V2            1

/* Macros utilitárias */
/* Obter endereço */
/*
* x             Horizontal coordinate of the point.
* y             Vertical coordinate of the point.
*/
#define SSD1309_GET_ADDR(x, y)         (y * SSD1309_WIDTH + x)

/* Transferência SPI */
/*
* _p_tx_data - ponteiro para o data a enviar
* _tx_length - número de bytes a enviar
* _p_rx_data - ponteiro para o data recebido
* _rx_length - número de bytes a receber
*/
#define SSD1309_TRANSFER(_p_tx_data, _tx_length, _p_rx_data, _rx_length) \
  NRF_SPI_MNGR_TRANSFER(_p_tx_data, _tx_length, _p_rx_data, _rx_length) 

/********************************** Funções ***********************************/
ret_code_t ssd1309_init(void);
bool ssd1309_get_init_status(void);
void ssd1309_pixel_draw(uint16_t x, uint16_t y, uint32_t color);
void ssd1309_refresh_lcd(void);
void ssd1309_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
void ssd1309_invert(bool invert);
void ssd1309_power_mode(void);


#endif /* SSD1309_H_ */





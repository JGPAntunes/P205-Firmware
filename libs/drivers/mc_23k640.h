/*
* @file		mc_23k640.c
* @date		September 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the saadc adc utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef MC_23K640_H_
#define MC_23K640_H_

/********************************** Includes ***********************************/

/* SDK */
#include "nrf_spi_mngr.h"
#include "nrf_drv_gpiote.h"

/* Config */
#include "config.h"

/* standard library */
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

/********************************** Definitions ***********************************/
#define MC_23k640_CS1                  NRF_GPIO_PIN_MAP(0, 30)        /* */


/* Circular buffer struct */
typedef struct {
  uint16_t  head;       /* Head index */
  uint16_t  tail;       /* Tail index */
} circular_struct;


/* Memory Macros */
#define MAX_MEMORY_ADDR                (0x1fff)    /* Maximum address */
#define CONFIG_MEMORY_ADDR             (0x1f40)    /* Address of device configuration region */
#define DATA_MEMORY_ADDR               (0x0010)    /* Address of device data region */
#define TAIL_INDEX_MEMORY_ADDR         (0x0000)    /* Address of device written bytes region */
#define HEAD_INDEX_MEMORY_ADDR         (0x0005)    /* Address of device written bytes region */
#define DATA_MEMORY_SIZE               (CONFIG_MEMORY_ADDR - DATA_MEMORY_ADDR)
/* Sizes */
#define MC_23K640_INDEX_SIZE           (2)         /*    */
#define MC_23K640_BUFFER_SIZE          (520)       /* Size in bytes of a Page */
#define MC_23K640_MAX_TRANSFER_SIZE    (500)       /* Maximum size in bytes of a dual transfer */
#define MC_23K640_SINGLE_TRANSFER_SIZE (250)       /* Size in bytes of a single transfer */

/* Commands */
#define MC_23K640_READ_CMD             (0x03)                 /* */
#define MC_23K640_WRITE_CMD            (0x02)                 /* */
#define MC_23K640_READ_STAT_CMD        (0x05)                 /* */
#define MC_23K640_WRITE_STAT_CMD       (0x01)                 /* */
#define MC_23K640_CMD_TRANSFER_SIZE    (3)                    /* */

/* Status Register */
#define MC_23K640_STAT_SIZE            (2)                   /* */
#define MC_23K640_MODE                 (0x01)                /* 0=Byte; 1=Sequential; 2=page; 3=Reserved */
#define MC_23K640_HOLD                 (0x01)                /* */
#define MC_23K640_MODE_SHIFTS          (6)                   /* */
#define MC_23K640_STAT_REG             ((MC_23K640_MODE << MC_23K640_MODE_SHIFTS) | 0x02 | MC_23K640_HOLD)               /* */

#define MC_23K640_TRANSFER(tx_data, tx_len, rx_data, rx_len) \
  NRF_SPI_MNGR_TRANSFER(tx_data, tx_len, rx_data, rx_len) 


/********************************** Functions ***********************************/
bool mc_23k640_init(void);
uint16_t mc_23k640_read_data(uint8_t *data);
uint8_t mc_23k640_write_data(uint8_t *data, uint16_t bytes);
bool mc_23k640_read_config(uint8_t *data, uint16_t nbytes);
bool mc_23k640_write_config(uint8_t *data, uint16_t nbytes);
#endif /* MC_23K640_H_ */



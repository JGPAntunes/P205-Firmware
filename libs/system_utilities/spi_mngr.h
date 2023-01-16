/*
* @file           spi_mngr.h
* @date           July 2021
* @author         PFaria & JAntunes
*
* @brief          This file has the SPI manager utilities, with the purpose of
*                 inicializing the spi peripheral.
*
* Copyright(C)    2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef SPI_MNGR_H
#define SPI_MNGR_H

/*********************************** Includes ***********************************/
/* SDK */
#include "nrf_spi_mngr.h"
/* Config */
#include "config.h"

/********************************** Definições ***********************************/
/* SPI Private Config */
/* SPI Instances */
#define SPI_MNGR_INSTANCE_ID0          0                              /* SPI0 Manager Instance */
#define SPI_MNGR_INSTANCE_ID1          1                              /* SPI1 Manager Instance - ads129X driver */
#define SPI_MNGR_INSTANCE_ID2          2                              /* SPI2 Manager Instance - lcd, ram, uSD driver */

/* SPI Max. Queue Size */
typedef enum {
  SPI_MNGR_MAX_QUEUE_SIZE0 = 10,                                      /* SPI0 Max. de transações pendentes */
  SPI_MNGR_MAX_QUEUE_SIZE1 = 10,                                      /* SPI1 Max. de transações pendentes */
  SPI_MNGR_MAX_QUEUE_SIZE2 = 10                                       /* SPI2 Max. de transações pendentes */
} spi_mngr_max_queue_size_t;

/* SPI Configs */
typedef enum {                                                        /* In case of adding more devices SPI add config macro here */
  SPI_MNGR_CONFIG1 = 1,                                               /* SPI config1 */
  SPI_MNGR_CONFIG2,                                                   /* SPI config2 */
  SPI_MNGR_CONFIG3                                                    /* SPI config3 */
} spi_mngr_configs_t;
#define SPI_MNGR_NUMBER_CONFIGS        3                              /* SPI configs number */

/* SPI User Configs */  

#define SPI_MNGR_CONFIG1_T                            \
  {                                                   \
    .sck_pin      = SPI_MNGR_CONFIG1_SCK_PIN,         \
    .mosi_pin     = SPI_MNGR_CONFIG1_MOSI_PIN,        \
    .miso_pin     = SPI_MNGR_CONFIG1_MISO_PIN,        \
    .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,         \
    .irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,  \
    .orc          = 0xFF,                             \
    .frequency    = NRF_DRV_SPI_FREQ_8M,              \
    .mode         = NRF_DRV_SPI_MODE_1,               \
    .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST   \
  }
#define SPI_MNGR_CONFIG1_INSTANCE       SPI_MNGR_INSTANCE_ID1         /* SPI Instance - ads129X driver 8M Hz config */
 
#define SPI_MNGR_CONFIG2_T                            \
  {                                                   \
    .sck_pin      = SPI_MNGR_CONFIG2_SCK_PIN,         \
    .mosi_pin     = SPI_MNGR_CONFIG2_MOSI_PIN,        \
    .miso_pin     = SPI_MNGR_CONFIG2_MISO_PIN,        \
    .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,         \
    .irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,  \
    .orc          = 0xFF,                             \
    .frequency    = NRF_DRV_SPI_FREQ_4M,              \
    .mode         = NRF_DRV_SPI_MODE_1,               \
    .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST   \
  }
#define SPI_MNGR_CONFIG2_INSTANCE       SPI_MNGR_INSTANCE_ID1         /* SPI Instance - ads129X driver 4M Hz config */

#define SPI_MNGR_CONFIG3_T                            \
  {                                                   \
    .sck_pin      = SPI_MNGR_CONFIG3_SCK_PIN,         \
    .mosi_pin     = SPI_MNGR_CONFIG3_MOSI_PIN,        \
    .miso_pin     = SPI_MNGR_CONFIG3_MISO_PIN,        \
    .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,         \
    .irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,  \
    .orc          = 0xFF,                             \
    .frequency    = NRF_DRV_SPI_FREQ_8M,              \
    .mode         = NRF_DRV_SPI_MODE_0,               \
    .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST   \
  }
#define SPI_MNGR_CONFIG3_INSTANCE       SPI_MNGR_INSTANCE_ID2         /* SPI Instance - lcd, ram, uSD driver */
  

/********************************** Funções ***********************************/
void spi_mngr_init(uint8_t spi_config);
const nrf_spi_mngr_t* spi_mngr_get_instance(uint8_t spi_instance);
bool spi_mngr_get_status(uint8_t spi_instance);

#endif /* SPI_MNGR_H */
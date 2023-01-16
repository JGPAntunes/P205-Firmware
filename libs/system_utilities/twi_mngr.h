/*
* @file		twi_mngr.h
* @date		June 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the TWI manager utilities, with the purpose of
*               inicializing the I2C peripheral.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef TWI_MNGR_H
#define TWI_MNGR_H

/*********************************** Includes ***********************************/

/* SDK */
#include "nrf_twi_mngr.h"

/* Config */
#include "config.h"

/********************************** Definições ***********************************/
/* TWI Private Config */
/* TWI Instances */
#define TWI_MNGR_INSTANCE_ID0         0                              /* TWI0 Manager Instance */
#define TWI_MNGR_INSTANCE_ID1         1                              /* TWI1 Manager Instance */

/* TWI Max. Queue Size */
typedef enum {
    TWI_MNGR_MAX_QUEUE_SIZE0 = 5,                                     /* TWI0 Max. de transações pendentes */
    TWI_MNGR_MAX_QUEUE_SIZE1 = 5                                      /* TWI1 Max. de transações pendentes */
} twi_mngr_max_queue_size_t;

/* TWI User Configs */

/* TWI Configs */
typedef enum {                                                        /* In case of adding more devices TWI add config macro here */
  TWI_MNGR_CONFIG1 = 1                                                /* TWI config1 - Temperature Driver */
} twi_mngr_configs_t;
#define TWI_MNGR_NUMBER_CONFIGS        1                              /* TWI configs number */

typedef enum {                                                        /* TWI device config 1 - ADS129x Normal Mode */
  TWI_MNGR_CONFIG1_SCL          = 26,                                 /* Device pin for SCL  */
  TWI_MNGR_CONFIG1_SDA          = 27,                                 /* Device pin for SDA */
  TWI_MNGR_CONFIG1_FREQ         = NRF_DRV_TWI_FREQ_400K,              /* Device frequency */ 
  TWI_MNGR_CONFIG1_IRQ_PRIORITY = TWI_DEFAULT_CONFIG_IRQ_PRIORITY,    /* Default IRQ priority */
  TWI_MNGR_CONFIG1_INSTANCE     = TWI_MNGR_INSTANCE_ID0               /* TWI Instance */
} twi_mngr_config1_t;

/********************************** Funções ***********************************/
void twi_mngr_init(uint8_t twi_config);
const nrf_twi_mngr_t* twi_mngr_get_instance(uint8_t twi_instance);
bool twi_mngr_get_status(uint8_t twi_instance);

#endif /* TWI_MNGR_H */
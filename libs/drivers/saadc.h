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

#ifndef SAADC_H_
#define SAADC_H_

/********************************** Includes ***********************************/

/* SDK */
#include "nrf_drv_saadc.h"

/* Config */
#include "config.h"

/********************************** Definições ***********************************/
/* Configurações do SAADC */
#define SAADC_RESOLUTION            NRF_SAADC_RESOLUTION_14BIT
#define SAADC_OVERSAMPLE            NRF_SAADC_OVERSAMPLE_DISABLED 

/* Configurações do canal analógico */
#define SAADC_ANALOG_CHANNEL        0
#define SAADC_GAIN                  NRF_SAADC_GAIN1_6
#define SAADC_VREFERENCE            NRF_SAADC_REFERENCE_INTERNAL
#define SAADC_ACQ_TIME              NRF_SAADC_ACQTIME_10US
#define SAADC_MODE                  NRF_SAADC_MODE_SINGLE_ENDED
#define SAADC_SAMPLES_IN_BUFFER     1    

/* Caracteristicas do SAADC */
#define SAADC_GAIN_VALUE            0.16666667
#define SAADC_REF                   0.6
#define SAADC_RES                   14

/* Macro para testar SAADC */
#if SAADC_TEST
#define SAADC_MAX_VOLTAGE_SAMPLES   100    
#define SAADC_VCOMP_MIN             400
#define SAADC_VCOMP_MAX             0
#endif

/* Macros utilitárias */

/* Estrutura de dados com o valor de tensão medido e o respetivo timestamp */
typedef struct {
  float voltage;
  uint64_t timestamp;
} saadc_data;  


/********************************** Funções ***********************************/
bool saadc_init(void);
bool saadc_get_init_status(void);
bool saadc_sample(void);
saadc_data *saadc_get_data(void);

#endif /* SAADC_H_ */



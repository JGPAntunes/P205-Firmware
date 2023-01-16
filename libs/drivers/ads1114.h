/*
* @file		ads1114.h
* @date		April 2020
* @author	PFaria & JAntunes
*
* @brief	This is the header for ads1114 adc utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef ADS1114_H_
#define ADS1114_H_

/********************************** Includes ***********************************/

/* SDK */
#include "nrf_twi_mngr.h"

/* Config */
#include "config.h"

/********************************** Definições ***********************************/
/* Endereços ADS1114 */
#define ADS1114_ADDR                    0x48                                /* ADDR PIN to GND */
#define ADS1114_REG_READ                0x00                                /* Endereço com conteudo de medição */
#define ADS1114_REG_CONFIG              0x01                                /* Endereço para escrever config */
#define ADS1114_REG_LO_TH               0x02                                /* Endereço para Lo_thresh */
#define ADS1114_REG_HI_TH               0x03                                /* Endereço para Hi_thresh */
#define ADS1114_REG_SIZE                2                                   /* Tamanho em bytes do registo a fazer leitura */

/* Configuração dos registos ADS1114 para deteção do device */ 
#define ADS1114_REG_CONFIG_MSB_D        0x0F                                /* Most significant byte */
#define ADS1114_REG_CONFIG_LSB_D        0xE0                                /* Least significant byte */ //Previous without DRY -> 0x17

/* Configuração dos registos ADS1114 */
/* Config register */
#define ADS1114_REG_CONFIG_MSB          0x8F                                /* Most significant byte */
#define ADS1114_REG_CONFIG_LSB          0xE0                                /* Least significant byte */ //Previous without DRY -> 0x17

/* Lo_thresh register */
#define ADS1114_REG_LO_TH_MSB           0x7F                                /* Most significant byte */
#define ADS1114_REG_LO_TH_LSB           0xFF                                /* Least significant byte */ 

/* Hi_thresh register */
#define ADS1114_REG_HI_TH_MSB           0x80                                /* Most significant byte */
#define ADS1114_REG_HI_TH_LSB           0x00                                /* Least significant byte */


/* Caracteristicas do ADS1114 */
#define ADS1114_BUFFER_SIZE             ADS1114_REG_SIZE                    /* Tamanho do buffer de receção dos dados do ADS */
#define ADS1114_RES                     16                                  /* Resolução do ADC */
#define ADS1114_FSR                     0.256                               /* Full sclare range */

/* Macro para testar ADS1114 */
#if ADS1114_TEST
#define ADS1114_MAX_VOLTAGE_SAMPLES     100    
#define ADS1114_VCOMP_MIN               400
#define ADS1114_VCOMP_MAX               0
#endif

/* Macros utilitárias */
#define ADS1114_GETVALUE(ads_hi, ads_lo) ( ((int16_t)ads_hi << 8) | ads_lo)  /* Converter 2 bytes para variavel de 16 bit */

/* Leitura TWI */
/*
* address - endereço para realizar leitura
* p_buffer - buffer de destino do conteudo
* length - número de bytes para receber na transferência
*/
#define ADS_READ(address, p_buffer, length) \
  NRF_TWI_MNGR_WRITE(ADS1114_ADDR, address, 1, NRF_TWI_MNGR_NO_STOP), \
  NRF_TWI_MNGR_READ (ADS1114_ADDR, p_buffer, length, 0)

/* Escrita TWI */
/*
* p_data - pointer para o data a ser enviado - [registo a escrever, conteudo]
* length - número de bytes para receber na transferência
*/
#define ADS_WRITE(p_data, length) \
  NRF_TWI_MNGR_WRITE(ADS1114_ADDR, p_data, length, 0) 

/* Estrutura de dados com o valor de tensão medido e o respetivo timestamp */
typedef struct {
  float voltage;
  uint64_t timestamp;
} ads1114_data;  

/* Estados da inicialização */
typedef enum {
  ADS1114_INIT_ST,
  ADS1114_DETECT_DEVICE_ST,
  ADS1114_CONFIG_ST   
} ads1114_init_states; 


/********************************** Funções ***********************************/
bool ads1114_init (void);
bool ads1114_get_init_status(void);
bool ads1114_readValue(void);
ads1114_data *ads1114_get_data(void);


#endif /* ADS1114_H_ */



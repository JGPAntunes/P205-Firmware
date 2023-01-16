/*
* @file           spi_mngr.c
* @date           July 2021
* @author         PFaria & JAntunes
*
* @brief          This file has the SPI manager utilities, with the purpose of
*                 inicializing the spi peripheral.
*
* Copyright(C)    2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/

/* Interface */
#include "spi_mngr.h"

/* SDK */
#include "nrf_spi_mngr.h"

/********************************** Definições ***********************************/
/*
* Macro que define SPI0 transaction manager instance
*/
#if SPI0_ENABLED
NRF_SPI_MNGR_DEF(_p_nrf_spi_mngr0, SPI_MNGR_MAX_QUEUE_SIZE0, SPI_MNGR_INSTANCE_ID0);
#endif

/*
* Macro que define SPI1 transaction manager instance
*/
#if SPI1_ENABLED
NRF_SPI_MNGR_DEF(_p_nrf_spi_mngr1, SPI_MNGR_MAX_QUEUE_SIZE1, SPI_MNGR_INSTANCE_ID1);
#endif

/*
* Macro que define SPI2 transaction manager instance
*/
#if SPI2_ENABLED
NRF_SPI_MNGR_DEF(_p_nrf_spi_mngr2, SPI_MNGR_MAX_QUEUE_SIZE2, SPI_MNGR_INSTANCE_ID2);
#endif

/*
* Variável de estado de inicialização do SPI_MNGR0
*/
static bool _spi_mngr0_state_init = false;

/*
* Variável de estado de inicialização do SPI_MNGR1
*/
static bool _spi_mngr1_state_init = false;

/*
* Variável de estado de inicialização do SPI_MNGR2
*/
static bool _spi_mngr2_state_init = false;

/* Private functions list */
void spi_mngr_instance_init(nrf_drv_spi_config_t spi_config, uint8_t spi_instance);


/*
* @brief Function initializing SPI instance
*
* @param[in]   spi_config             SPI instance configuration
*/
void spi_mngr_init(uint8_t spi_config) {

  nrf_drv_spi_config_t config1 = SPI_MNGR_CONFIG1_T;
  nrf_drv_spi_config_t config2 = SPI_MNGR_CONFIG2_T;
  nrf_drv_spi_config_t config3 = SPI_MNGR_CONFIG3_T;

  switch(spi_config) {
    case SPI_MNGR_CONFIG1:      
      /* Configuração 1 */
      spi_mngr_instance_init(config1, SPI_MNGR_CONFIG1_INSTANCE);    
      break;
    case SPI_MNGR_CONFIG2:
      /* Configuração 2 */
      spi_mngr_instance_init(config2, SPI_MNGR_CONFIG2_INSTANCE);    
      break;
    case SPI_MNGR_CONFIG3:
      /* Configuração 3 */
      spi_mngr_instance_init(config3, SPI_MNGR_CONFIG3_INSTANCE);    
      break;
    default:                          /* Caso adicionar mais devices, adicionar estados ao switch */
      break;
  }

}


/*
* @brief Function initializing SPI transaction manager instance
*
* @param[in]   spi_config             SPI master driver instance configuration structure
* @param[in]   spi_instance           SPI instance to inicialize
*/
void spi_mngr_instance_init(nrf_drv_spi_config_t spi_config, uint8_t spi_instance) {
  
  uint32_t err_code = NRF_SUCCESS;

  /* Inicialização do SPI0 manager instance */
  #if SPI0_ENABLED
  if(spi_instance == SPI_MNGR_INSTANCE_ID0 && !_spi_mngr0_state_init) {
    
    /* Configuração do SPI0 master drive instance*/
    nrf_drv_spi_config_t const spi_config0 = spi_config;       
    };    

    /* Efetivar a inicialização */
    err_code = nrf_spi_mngr_init(&_p_nrf_spi_mngr0, &spi_config0);

    /* Verificar inicialização */ 
    if(err_code == NRF_SUCCESS) {
      _spi_mngr0_state_init = true;
    };  
  }
  #endif

  /* Inicialização do SPI1 manager instance */
  #if SPI1_ENABLED
  if(spi_instance == SPI_MNGR_INSTANCE_ID1 && !_spi_mngr1_state_init) {
    
    /* Configuração do SPI1 master drive */
    nrf_drv_spi_config_t const spi_config1 = spi_config;   

    /* Efetivar a inicialização */
    err_code = nrf_spi_mngr_init(&_p_nrf_spi_mngr1, &spi_config1);

    /* Verificar inicialização */ 
    if(err_code == NRF_SUCCESS) {
      _spi_mngr1_state_init = true;
    }  
  }
  #endif

  /* Inicialização do SPI2 manager instance */
  #if SPI2_ENABLED
  if(spi_instance == SPI_MNGR_INSTANCE_ID2 && !_spi_mngr2_state_init) {
    
    /* Configuração do SPI2 master drive */
    nrf_drv_spi_config_t const spi_config2 = spi_config;   

    /* Efetivar a inicialização */
    err_code = nrf_spi_mngr_init(&_p_nrf_spi_mngr2, &spi_config2);

    /* Verificar inicialização */ 
    if(err_code == NRF_SUCCESS) {
      _spi_mngr2_state_init = true;
    }  
  }
  #endif

}


/*
* @brief Function to get SPI transaction manager instance pointer
*
* @param[in]   spi_instance           SPI instance to get pointer
* @return                             Returns the SPI instance pointer
*/
const nrf_spi_mngr_t* spi_mngr_get_instance(uint8_t spi_instance) {

  /* Escolha da instância correta */
  switch(spi_instance) {    
    #if SPI0_ENABLED
    case SPI_MNGR_INSTANCE_ID0:
      return &_p_nrf_spi_mngr0;
    #endif
    #if SPI1_ENABLED
    case SPI_MNGR_INSTANCE_ID1:
      return &_p_nrf_spi_mngr1;
    #endif
    #if SPI2_ENABLED
    case SPI_MNGR_INSTANCE_ID2:
      return &_p_nrf_spi_mngr2;
    #endif
    default:
      return NULL;
  }

}


/*
* @brief Function to check if SPI transaction manager instance was succesfully inicialized
*
* @param[in]   spi_instance           SPI instance to check inicialization
* @return                             Returns the status of SPI instance
*/
bool spi_mngr_get_status(uint8_t spi_instance) {

  /* Escolha da instância correta */
  switch(spi_instance) {  
    case SPI_MNGR_INSTANCE_ID0:
      return _spi_mngr0_state_init;
    case SPI_MNGR_INSTANCE_ID1:
      return _spi_mngr1_state_init;
    case SPI_MNGR_INSTANCE_ID2:
      return _spi_mngr2_state_init;
    default:
      return NULL;
  }

}


/********************************** Notes ***********************************/
/* 
*
* Sense para ativar SPI1 em no SDK:
*   NRFX_SPI_ENABLED 1
*   NRFX_SPI1_ENABLED 1
*
*   SPI_ENABLED 1
*   SPI1_ENABLED 1
*
* Conlusão, para ter dois barramentos, no sdkconfig.h é necessário:
*   NRFX_SPI2_ENABLED 0 -> 1
*   SPI2_ENABLED 0 -> 1
*
*   Por questões de partilha de endereços de memória é apenas possível utilizar,
*   1 Twi e 2 SPI em simultaneo 
*
*/


/*
* @file		twi_mngr.c
* @date		June 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the TWI manager utilities, with the purpose of
*               inicializing the I2C peripheral.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

/*********************************** Includes ***********************************/

/* Interface */
#include "twi_mngr.h"

/* SDK */
#include "nrf_twi_mngr.h"

/********************************** Definições ***********************************/
/*
* Macro que define TWI0 transaction manager instance
*/
#if TWI0_ENABLED
NRF_TWI_MNGR_DEF(_p_nrf_twi_mngr0, TWI_MNGR_MAX_QUEUE_SIZE0, TWI_MNGR_INSTANCE_ID0);
#endif

/*
* Macro que define TWI0 transaction manager instance
*/
#if TWI1_ENABLED
NRF_TWI_MNGR_DEF(_p_nrf_twi_mngr1, TWI_MNGR_MAX_QUEUE_SIZE1, TWI_MNGR_INSTANCE_ID1);
#endif

/*
* Variável de estado de inicialização do TWI_MNGR0
*/
static bool _twi_mngr0_state_init = false;

/*
* Variável de estado de inicialização do TWI_MNGR1
*/
static bool _twi_mngr1_state_init = false;

/*
* Estrutura para alocar configs para os diversos devices
*/
nrf_drv_twi_config_t _config[TWI_MNGR_NUMBER_CONFIGS];

/* Private functions list */
void twi_mngr_instance_init(nrf_drv_twi_config_t twi_config, uint8_t twi_instance);


/*
* @brief Function initializing TWI manager
*
* @param[in]   twi_config             TWI instance configuration device dedicated
*/
void twi_mngr_init(uint8_t twi_config) {

  switch(twi_config) {
    case TWI_MNGR_CONFIG1:
      /* Configuração 1 */
      _config[TWI_MNGR_CONFIG1].scl                = TWI_MNGR_CONFIG1_SCL;
      _config[TWI_MNGR_CONFIG1].sda                = TWI_MNGR_CONFIG1_SDA;
      _config[TWI_MNGR_CONFIG1].frequency          = TWI_MNGR_CONFIG1_FREQ;
      _config[TWI_MNGR_CONFIG1].interrupt_priority = TWI_MNGR_CONFIG1_IRQ_PRIORITY;                 
      _config[TWI_MNGR_CONFIG1].clear_bus_init     = TWI_DEFAULT_CONFIG_CLR_BUS_INIT;

      twi_mngr_instance_init(_config[TWI_MNGR_CONFIG1], TWI_MNGR_INSTANCE_ID0);
      break;
    default:                          /* Caso adicionar mais devices, adicionar estados ao switch */
      break;
  }    

}


/*
* @brief Function initializing TWI transaction manager instance
*
* @param[in]   twi_config             TWI master driver instance configuration structure
* @param[in]   twi_instance           TWI instance to inicialize
*/
void twi_mngr_instance_init(nrf_drv_twi_config_t twi_config, uint8_t twi_instance) {

  uint32_t err_code = NRF_SUCCESS;

  /* Configuração do TWI0 master drive instance*/
  #if TWI0_ENABLED
  if(twi_instance == TWI_MNGR_INSTANCE_ID0 && !_twi_mngr0_state_init) {
    
    nrf_drv_twi_config_t const twi_config0 = twi_config;
  
    /* Efetivar a inicialização */
    if(err_code == NRF_SUCCESS) {   
      err_code = nrf_twi_mngr_init(&_p_nrf_twi_mngr0, &twi_config0); 
    }
  
    /* Verificar inicialização */
    if(err_code == NRF_SUCCESS) {   
      _twi_mngr0_state_init = true;
    }
  }
  #endif

  /* Configuração do TWI1 master drive instance*/
  #if TWI1_ENABLED
  if(twi_instance == TWI_MNGR_INSTANCE_ID1 && !_twi_mngr1_state_init) {
    
    nrf_drv_twi_config_t const twi_config1 = twi_config;
  
    /* Efetivar a inicialização */
    if(err_code == NRF_SUCCESS) {   
      err_code = nrf_twi_mngr_init(&_p_nrf_twi_mngr1, &twi_config1); 
    }
  
    /* Verificar inicialização */
    if(err_code == NRF_SUCCESS) {   
      _twi_mngr1_state_init = true;
    }
  }
  #endif  
  
}


/*
* @brief Function to get TWI transaction manager instance pointer
*
* @param[in]   twi_instance           TWI instance to get pointer
* @return                             Returns the TWI instance pointer
*/
const nrf_twi_mngr_t *twi_mngr_get_instance(uint8_t twi_instance) {

  /* Escolha da instância correta */
  switch(twi_instance) {    
    #if TWI0_ENABLED
    case TWI_MNGR_INSTANCE_ID0:
      return &_p_nrf_twi_mngr0;
    #endif
    #if TWI1_ENABLED
    case TWI_MNGR_INSTANCE_ID1:
      return &_p_nrf_twi_mngr1;
    #endif    
    default:
      return NULL;
  }

} 


/*
* @brief Function to check if TWI transaction manager instance was succesfully inicialized
*
* @param[in]   twi_instance           TWI instance to check inicialization
* @return                             Returns the status of TWI instance
*/
bool twi_mngr_get_status(uint8_t twi_instance) {

  /* Escolha da instância correta */
  switch(twi_instance) {  
    case TWI_MNGR_INSTANCE_ID0:
      return _twi_mngr0_state_init;
    case TWI_MNGR_INSTANCE_ID1:
      return _twi_mngr1_state_init;
    default:
      return NULL;
  }

}

/********************************** Notes ***********************************/
/* 
*
* Sense para ativar TWI1 em no SDK:
*   NRFX_TWIM_ENABLED   1
*
*   NRFX_TWI_ENABLED    1
*   NRFX_TWI0_ENABLED   1
*
*   TWI_ENABLED 1
*   TWI0_ENABLED 1
*
* Conlusão, para ter 1 barramentos, no sdkconfig.h é necessário:
*   Manter as configurações
*
*/
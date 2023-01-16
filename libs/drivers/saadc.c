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

/*********************************** Includes ***********************************/

/* Interface */
#include "saadc.h"

/* Pheriperals */
#include "sense_library/periph/rtc.h"

/* SDK */
#include "nrf_drv_saadc.h"

/* C standard library */
#include "math.h"

/********************************** Private ************************************/
/*
* Variável de estado de inicialização do saadc
*/
static bool _saadc_state_init = false;

/*
* Buffer que armazena as samples do SAADC
*/
static nrf_saadc_value_t     m_buffer_pool[2][SAADC_SAMPLES_IN_BUFFER];

/*
 * Data structure to return through the function saadc_get_data()
 */
static saadc_data _saadc_data;


/*
 * Variables to test SAADC
 */
#if SAADC_TEST 
static uint32_t _samples_count;
static float _saadc_voltage_array [SAADC_MAX_VOLTAGE_SAMPLES];
static float _saadc_voltage_min = SAADC_VCOMP_MIN;
static float _saadc_voltage_max = SAADC_VCOMP_MAX;
#endif

/* SAADC callback functions list */
static void saadc_cb(nrfx_saadc_evt_t const *p_event);

/* Private functions list */
float saadc_voltage_value(uint32_t adc_result);
//#if SAADC_TEST
void saadc_voltageTest(float voltage);
//#endif


/*
 * @brief Function for initializing SAADC
 *
 * @retval  True if initialization was successful, otherwise false
 */
bool saadc_init(void) {

  if(_saadc_state_init) {                                                             /* Caso a função já tenha sido executada com sucesso */
    return true;
  }

  /* Configuração das caracteristicas do SAADC */
  nrf_drv_saadc_config_t p_config;
  p_config.resolution = (nrf_saadc_resolution_t) SAADC_RESOLUTION;      
  p_config.oversample = (nrf_saadc_oversample_t) SAADC_OVERSAMPLE;        
  p_config.interrupt_priority = NRFX_SAADC_CONFIG_IRQ_PRIORITY;                   
  p_config.low_power_mode = false; 

  /* Efetivar inicialização do SAADC */
  if(nrf_drv_saadc_init(&p_config, saadc_cb) != 0) {
    return false;
  }
          
  /* Configuração do canal analógico */
  nrf_saadc_channel_config_t config;    
  config.resistor_p = NRF_SAADC_RESISTOR_DISABLED;                                
  config.resistor_n = NRF_SAADC_RESISTOR_DISABLED;                               
  config.gain = SAADC_GAIN;                                                  
  config.reference = SAADC_VREFERENCE;                                
  config.acq_time = SAADC_ACQ_TIME;                                        
  config.mode = SAADC_MODE;                                      
  config.burst = NRF_SAADC_BURST_DISABLED;                                        
  config.pin_p = SAADC_PIN_P;                
  config.pin_n = NRF_SAADC_INPUT_DISABLED; 
  
  /* Efetivar inicialização do canal analógico */
  if(nrf_drv_saadc_channel_init(SAADC_ANALOG_CHANNEL, &config) != 0) {
    return false;
  }
  
  /* Inicialização de buffers a preencher com número de samples definido */ 
  if(nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAADC_SAMPLES_IN_BUFFER) != 0) {  /* SAADC começa a escrever para este buffer */
    return false;
  }
  
  if(nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAADC_SAMPLES_IN_BUFFER) != 0) {  /* SAADC escreve para 2º buffer quando o inicial tiver cheio */
    return false;
  }
  _saadc_state_init = true;
  return true;    
           
}


/*
 * @brief Function for request an SAADC sample
 *
 * @retval  True if request sample was successful, otherwise false
 */
bool saadc_sample(void) {        
  
  if(nrf_drv_saadc_sample() != 0) {
    return false;
  }
  return true; 
    
}


/*
 * @brief Function to handle SAADC event callback 
 *
 * @param[in] p_event       Pointer to an SAADC driver event.
 */
static void saadc_cb(nrfx_saadc_evt_t const *p_event) {

  if (p_event->type == NRF_DRV_SAADC_EVT_DONE) {

    _saadc_data.timestamp = rtc_get_milliseconds();                                     /* Obter timestamp da medida */
        
    /* Obter sample para o buffer de resultado */
    if(nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAADC_SAMPLES_IN_BUFFER) != 0) { 
      return;
    }

    /* Retirar o valor de tensão medida */
    _saadc_data.voltage = saadc_voltage_value(p_event->data.done.p_buffer[0]);

    #if SAADC_TEST
      saadc_voltageTest(_saadc_data.voltage);   
      _samples_count ++;
    #endif          
  }
    
}


/*
 * @brief Function to check if driver initialization was successful 
 *
 * @return      status             Returns the status of driver
 *                                  true  - sucess
 *                                  false - failure
 */
bool saadc_get_init_status(void) {

  return _saadc_state_init;

}


/*
 * @brief Function for converting register value from SAADC to voltage
 *
 * @param[in]   adc_result          ADC value returned by ADC buffer.
 * @return      voltage             Returns the voltage requested
 */
float saadc_voltage_value(uint32_t adc_result) {

  return  adc_result /(float)((SAADC_GAIN_VALUE/(float)SAADC_REF)*pow(2, SAADC_RES));

}


/* 
 * @brief Function that returns the data structure 
 *
 * @return    voltage     Returns the data structure pointer
 */
saadc_data *saadc_get_data(void) {

  return &_saadc_data;

}


/*
 * @brief Function to test the ADC with a resquest of a number of samples
 *        defined by SAADC_MAX_VOLTAGE_SAMPLES
 *
 * @param[in]   voltage          Voltage value returned from adcInt_voltageGet(...)
 */
#if SAADC_TEST
void saadc_voltageTest(float voltage) {

  /* Se o número de samples for mais que o desejado */
  if(_samples_count > SAADC_MAX_VOLTAGE_SAMPLES) {
    return;
  }

  /* Albbergar a soma das samples */
  float voltage_sum = 0; 

  /* Guardar amostras de tensão no array */
  _saadc_voltage_array[_samples_count] = voltage;
    
  /* Se o número de samples não for o desejado retornar */
  if(_samples_count < SAADC_MAX_VOLTAGE_SAMPLES) {
    return;
  }

  /* Retirar o máximo e mínimo e a média das samples */
  for(int i = 0 ; i < SAADC_MAX_VOLTAGE_SAMPLES ; i++) {

    if(_saadc_voltage_array[i] > _saadc_voltage_max) {                /* Retirar o máximo */
      _saadc_voltage_max = _saadc_voltage_array[i]; 
    }

    if(_saadc_voltage_array[i] < _saadc_voltage_min) {                /* Retirar o minimo */
      _saadc_voltage_min = _saadc_voltage_array[i];
    }

    voltage_sum += _saadc_voltage_array[i];                            /* Soma das samples */

  }
    
  float average = voltage_sum/SAADC_MAX_VOLTAGE_SAMPLES;              /* Retirar a média */

  /* Impressão dos resultados da medição */
  printf("Resultados da medicao:\n");
  printf("  Tensao maxima: %.2f [V] \n", _saadc_voltage_max);
  printf("  Tensao media:  %.2f [V] \n", average);
  printf("  Tensao minima: %.2f [V] \n", _saadc_voltage_min);

  printf("Excel:\n");
  for(int i = 0 ; i < SAADC_MAX_VOLTAGE_SAMPLES ; i++) {

    printf("%.2f\n", _saadc_voltage_array[i]);

  }
  printf("Done\n");

}
#endif


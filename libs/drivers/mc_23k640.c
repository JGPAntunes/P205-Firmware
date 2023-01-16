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

/*********************************** Includes ***********************************/

/* Interface */
#include "mc_23k640.h"

/* Pheriperals */
#include "sense_library/periph/rtc.h"

/* SDK */
#include "nrf_drv_saadc.h"
#include "nrf_spi_mngr.h"
#include "nrf_delay.h"

/* Utilities */
#include "spi_mngr.h"
#include "utils.h"

/* Debug - libs */
#include "sense_library/utils/debug.h"

/* C standard library */
#include "math.h"

/* Sensefinity */
#include "sense_library/utils/utils.h"

/********************************** Definitions ***********************************/
/*
* Stores the spi configuration.
*/
static const nrf_drv_spi_config_t _spi_config_8m = SPI_MNGR_CONFIG3_T;

/*
* Stores the spi instance.
*/
static const nrf_spi_mngr_t *_p_spi_mngr;

/*
* Indicates if init was successful.
*/
static bool _init_finished = false;

/*
* Indicates if init was scheduled.
*/
static bool _init_started = false;
    
/*
* General buffer used to comunicate with RAM.
*/
static uint8_t _mc_23k640_buffer[MC_23K640_BUFFER_SIZE];

/*
* Stores the number rof bytes written on RAM.
*/
static uint8_t _current_index[MC_23K640_INDEX_SIZE];

/*
* Circualr buffer structure.
*/
static circular_struct circular_buffer;

static uint16_t _last_index;


/********************************** Private ************************************/
/* Private functions list */
uint16_t circular_buffer_free_space(void);
uint16_t circular_buffer_occupied_space(void);
bool mc_23k640_read_circular_idx(void);
void mc_23k640_cs_init(void);
void mc_23k640_cs_deactivate(void);
void mc_23k640_cs_activate(void);

/*
 * @brief 
 */
uint16_t circular_buffer_free_space(void) {
  if(circular_buffer.head >= circular_buffer.tail) {
    /* Tail is before the head */
    return (DATA_MEMORY_SIZE - (circular_buffer.head - circular_buffer.tail));
  } else {
    /* Head is before the tail */
    return circular_buffer.tail - circular_buffer.head;
  }
}

/*
 * @brief 
 */
uint16_t circular_buffer_occupied_space(void){
  if(circular_buffer.head < circular_buffer.tail) {
    /* Head is before the tail */
    return DATA_MEMORY_SIZE - (circular_buffer.tail - circular_buffer.head);
  } else {
    /* Tail is before the head */
    return circular_buffer.head - circular_buffer.tail;
  }
}

/*
 * @brief Fucntion to activate the chip select.
 */
void mc_23k640_cs_activate(void) {
  /* Activate CS */
  nrf_gpio_pin_clear(MC_23k640_CS1);
}

/*
 * @brief Fucntion to initialize the chip select.
 */
void mc_23k640_cs_init(void) {
  nrf_gpio_cfg_output(MC_23k640_CS1);
  //nrf_gpio_cfg(MC_23k640_CS1, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_H0H1, NRF_GPIO_PIN_NOSENSE);

  /* Deactivate CS */
  mc_23k640_cs_deactivate();
}

/*
 * @brief Fucntion to activate the chip select.
 */
void mc_23k640_cs_deactivate(void) {
  /* Deactivate CS */
  nrf_gpio_pin_set(MC_23k640_CS1);
}

/*
 * @brief Function to read the indexes from external RAM.
 *
 * @retval True if read was successful. 
 */
bool mc_23k640_read_circular_idx(void) {
  static uint8_t tmp_buffer[2];

  /* Read cmd tail addr */
  _mc_23k640_buffer[0] = MC_23K640_READ_CMD;
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[1], TAIL_INDEX_MEMORY_ADDR);

  /* Read cmd head addr */
  _mc_23k640_buffer[3] = MC_23K640_READ_CMD;
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[4], HEAD_INDEX_MEMORY_ADDR);

  nrf_spi_mngr_transfer_t transfers[] = {                      
    MC_23K640_TRANSFER(&_mc_23k640_buffer[0], MC_23K640_CMD_TRANSFER_SIZE, NULL, 0),
    MC_23K640_TRANSFER(NULL, 0, tmp_buffer, MC_23K640_INDEX_SIZE),
    MC_23K640_TRANSFER(&_mc_23k640_buffer[3], MC_23K640_CMD_TRANSFER_SIZE, NULL, 0),
    MC_23K640_TRANSFER(NULL, 0, tmp_buffer, MC_23K640_INDEX_SIZE)
  };

  /* Read tail from RAM */
  mc_23k640_cs_activate();
  if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[0], 2, NULL) != NRF_SUCCESS) {
    mc_23k640_cs_deactivate();
    return false;
  } 
  mc_23k640_cs_deactivate();
  
  circular_buffer.tail = utils_get_uint16_from_array(tmp_buffer);

  /* Read head from RAM */
  mc_23k640_cs_activate();
  if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[2], 2, NULL) != NRF_SUCCESS) {
    mc_23k640_cs_deactivate();
    return false;
  } 
  mc_23k640_cs_deactivate();

  circular_buffer.head = utils_get_uint16_from_array(tmp_buffer);

  
  return true;
}
/********************************** Public ***********************************/
/*
 * @brief Function to initialize the external RAM.
 *
 * @retval True if init was successful.
 */
bool mc_23k640_init(void) {

  /* Return true if init was successful */
  if(_init_finished) {
    return true;
  }
  
  /* Prevent loop calls */
  if(_init_started) {
    return false;
  }


  ///* Initicialize Circular Indexes */
  circular_buffer.head = 0;
  circular_buffer.tail = 0;


  mc_23k640_cs_init();

  /* Get SPI manager */
  _p_spi_mngr = spi_mngr_get_instance(SPI_MNGR_CONFIG3_INSTANCE); 
            
#if MICROCONTROLER_2  
  /* Store status cmd and address */
  _mc_23k640_buffer[0] = MC_23K640_WRITE_STAT_CMD;
  _mc_23k640_buffer[1] = MC_23K640_STAT_REG;

  /* Store the tail */
  _mc_23k640_buffer[2] = MC_23K640_WRITE_CMD;
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[3], TAIL_INDEX_MEMORY_ADDR);
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[5], circular_buffer.tail);

  /* Store the head */
  _mc_23k640_buffer[7] = MC_23K640_WRITE_CMD;
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[8], HEAD_INDEX_MEMORY_ADDR);
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[10],circular_buffer.head);

  /* Store status cmd and address */
  _mc_23k640_buffer[12] = MC_23K640_READ_STAT_CMD;

  nrf_spi_mngr_transfer_t transfers[] = 
  {                      
    MC_23K640_TRANSFER(_mc_23k640_buffer, MC_23K640_STAT_SIZE, NULL, 0),
    MC_23K640_TRANSFER(&_mc_23k640_buffer[2], MC_23K640_INDEX_SIZE + MC_23K640_CMD_TRANSFER_SIZE, NULL, 0),
    MC_23K640_TRANSFER(&_mc_23k640_buffer[7], MC_23K640_INDEX_SIZE + MC_23K640_CMD_TRANSFER_SIZE, NULL, 0),
    MC_23K640_TRANSFER(&_mc_23k640_buffer[12], 1, NULL, 0),
    MC_23K640_TRANSFER(NULL, 0, &_mc_23k640_buffer[13], 1),
  };
  
  /* Write status to RAM */
  mc_23k640_cs_activate();
  if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[0], 1, NULL) != NRF_SUCCESS) {
    _init_started = false;
    mc_23k640_cs_deactivate();
    return false;
  } 
  mc_23k640_cs_deactivate();
  nrf_delay_ms(200);

  /* Write tail to RAM */
  mc_23k640_cs_activate();
  if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[1], 1, NULL) != NRF_SUCCESS) {
    _init_started = false;
    mc_23k640_cs_deactivate();
    return false;
  } 
  mc_23k640_cs_deactivate();
  nrf_delay_ms(200);

  /* Write head to RAM */
  mc_23k640_cs_activate();
  if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[2], 1, NULL) != NRF_SUCCESS) {
    _init_started = false;
    mc_23k640_cs_deactivate();
    return false;
  } 
  mc_23k640_cs_deactivate();

 /* Read status from RAM */
  mc_23k640_cs_activate();
  nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[3], 2, NULL);
  if(_mc_23k640_buffer[13] != MC_23K640_STAT_REG) {
    mc_23k640_cs_deactivate();
    return false;
  }
#endif
  mc_23k640_cs_deactivate();
  _init_finished = true;
  return true;
        
}

/*
 * @brief Function to write configurations in external RAM.
 *
 * @param[in] data Pointer to data buffer to be written.
 * @param[in] nbytes Number of bytes to write.
 * @retval True if write was successful, false otherwise. 
 */
bool mc_23k640_write_config(uint8_t *data, uint16_t nbytes) {

  if(!nbytes || data == NULL || nbytes > (MAX_MEMORY_ADDR - CONFIG_MEMORY_ADDR)) {
    return false;
  } 
  /* Store cmd and address */
  _mc_23k640_buffer[0] = MC_23K640_WRITE_CMD;
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[1], CONFIG_MEMORY_ADDR);

  
  /* Copy data to internal buffer */
  memcpy(&_mc_23k640_buffer[3], data, nbytes);

  nrf_spi_mngr_transfer_t transfers[] = {                      
    MC_23K640_TRANSFER(_mc_23k640_buffer, nbytes + MC_23K640_CMD_TRANSFER_SIZE, NULL, 0)
  };

  /* Write config to RAM */
  mc_23k640_cs_activate();
  if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[0], 1, NULL) != NRF_SUCCESS) {
    mc_23k640_cs_deactivate();
    return false;
  }
  mc_23k640_cs_deactivate();
  return true;
}

/*
 * @brief Function to read configurations in external RAM.
 *
 * @param[in] data Pointer to data buffer to be read.
 * @param[in] nbytes Number of bytes to read.
 * @retval True if read was successful, false otherwise. 
 */
bool mc_23k640_read_config(uint8_t *data, uint16_t nbytes) {

  if(!nbytes || data == NULL || nbytes > (MAX_MEMORY_ADDR - CONFIG_MEMORY_ADDR)) {
    return false;
  }

  /* Store CMD and adress */
  _mc_23k640_buffer[0] = MC_23K640_READ_CMD;
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[1], CONFIG_MEMORY_ADDR);


  nrf_spi_mngr_transfer_t transfers[] = {                      
    MC_23K640_TRANSFER(_mc_23k640_buffer, MC_23K640_CMD_TRANSFER_SIZE, NULL, 0),
    MC_23K640_TRANSFER(NULL, 0, data, nbytes)
  };

  /* Read config from RAM */
  mc_23k640_cs_activate();
  if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[0], 2, NULL) != NRF_SUCCESS) {
    mc_23k640_cs_deactivate();
    return false;
  } 
  mc_23k640_cs_deactivate();
  return true;
}

/*
 * @brief Function to write data in external RAM.
 *
 * @param[in] data Pointer to data buffer to be written.
 * @param[in] nbytes Number of bytes of data to be written.
 * @retval The number of bytes written. 
 */
uint8_t mc_23k640_write_data(uint8_t *data, uint16_t bytes) {
  uint8_t nbytes = bytes;
  if(!bytes || data == NULL) {
    return 0;
  }
  
  if(bytes >= MC_23K640_SINGLE_TRANSFER_SIZE) {
    nbytes = MC_23K640_SINGLE_TRANSFER_SIZE;
  } 

  /* Get current head index */
  if(!mc_23k640_read_circular_idx()) {
    return 0;
  }
  uint16_t current_index = circular_buffer.head;

  /* If we reach the end of the buffer, turn around */
  if((nbytes + circular_buffer.head >=  CONFIG_MEMORY_ADDR) && (circular_buffer.tail > nbytes)) { 
    current_index = 0;
  } else if ((nbytes + circular_buffer.head >=  CONFIG_MEMORY_ADDR) && (circular_buffer.tail <= nbytes)) {
    /* We don't have space to write */
    return 0;
  }

  ///* In case the circular buffer is full */
  //if(circular_buffer_free_space() < nbytes) {
  //  return 0;
  //}
  
  /* Write Head */
  _mc_23k640_buffer[0] = MC_23K640_WRITE_CMD;
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[1], HEAD_INDEX_MEMORY_ADDR);

  /* Write the head index on RAM */
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[3], current_index + nbytes);

  /* Write data CMD */
  _mc_23k640_buffer[5] = MC_23K640_WRITE_CMD;
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[6], DATA_MEMORY_ADDR + current_index);

  /* Copy data to internal buffer */
  memcpy(&_mc_23k640_buffer[8], data, nbytes);
  
  nrf_spi_mngr_transfer_t transfers[] = {                      
    MC_23K640_TRANSFER(_mc_23k640_buffer, MC_23K640_INDEX_SIZE + MC_23K640_CMD_TRANSFER_SIZE, NULL, 0), /* Index */
    MC_23K640_TRANSFER(&_mc_23k640_buffer[5], nbytes + MC_23K640_CMD_TRANSFER_SIZE, NULL, 0),   /* Data + cmd */
  };

  /* Write current index in RAM */
  mc_23k640_cs_activate();
  nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[0], 1, NULL);
  mc_23k640_cs_deactivate();

  /* Write data in RAM */
  mc_23k640_cs_activate();
  nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[1], 1, NULL);
  mc_23k640_cs_deactivate();
 

  /* Clear static indexes */
  circular_buffer.tail = 0;
  circular_buffer.head = 0;
  

  return nbytes;
}

/*
 * @brief Function to read data from external RAM.
 *
 * @param[in] data Pointer to data buffer to be read.
 * @retval Number of bytes read, -1 on error.
 */
uint16_t mc_23k640_read_data(uint8_t *data) {

  uint16_t remaining_bytes = 0;
  uint16_t read_size;

  if(data == NULL) {
    return 0;
  }
  /* Get current index */
  if(!mc_23k640_read_circular_idx()) {
    return 0;
  }
  uint16_t current_index = circular_buffer.tail;

  /* We're on the end of the buffer, and we have data on the beginning */
  if((CONFIG_MEMORY_ADDR - current_index) < circular_buffer_occupied_space()) {
    read_size = (CONFIG_MEMORY_ADDR - current_index) + 1;
    remaining_bytes = circular_buffer_occupied_space() - read_size;
    current_index = 0;
  } else if(MC_23K640_MAX_TRANSFER_SIZE <= circular_buffer_occupied_space()) {
    read_size = MC_23K640_MAX_TRANSFER_SIZE;
    remaining_bytes = circular_buffer_occupied_space() - read_size;
    current_index += read_size;
  } else {
    read_size = circular_buffer_occupied_space();
    current_index += read_size;
  }

  /* Store tail index */
  _mc_23k640_buffer[0] = MC_23K640_WRITE_CMD;
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[1], TAIL_INDEX_MEMORY_ADDR);

  /* Index value */
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[3], current_index);

  /* Read cmd and address */
  _mc_23k640_buffer[5] = MC_23K640_READ_CMD;
  utils_save_uint16_t_to_array(&_mc_23k640_buffer[6], DATA_MEMORY_ADDR + circular_buffer.tail);

  if(read_size <= MC_23K640_SINGLE_TRANSFER_SIZE) {
    nrf_spi_mngr_transfer_t transfers[] = {                      
      MC_23K640_TRANSFER(&_mc_23k640_buffer[0], MC_23K640_INDEX_SIZE + MC_23K640_CMD_TRANSFER_SIZE, NULL, 0), /* Write index */
      MC_23K640_TRANSFER(&_mc_23k640_buffer[5], MC_23K640_CMD_TRANSFER_SIZE, NULL, 0),                        /* Write a read cmd */
      MC_23K640_TRANSFER(NULL, 0, data, read_size)                                                            /* Read data */ 
    };

   /* Write current index in RAM */
    mc_23k640_cs_activate();
    if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[0], 1, NULL) != NRF_SUCCESS) {
      mc_23k640_cs_deactivate();
      return -1;
    } 
    mc_23k640_cs_deactivate();

    /* Read data in RAM */
    mc_23k640_cs_activate();
    if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[1], 2, NULL) != NRF_SUCCESS) {
      mc_23k640_cs_deactivate();
      return -1;
    } 

  /* In case we need to perform 2 transfers */
  } else {
    nrf_spi_mngr_transfer_t transfers[] = {                      
      MC_23K640_TRANSFER(&_mc_23k640_buffer[0], MC_23K640_INDEX_SIZE + MC_23K640_CMD_TRANSFER_SIZE, NULL, 0),   /* Write index */
      MC_23K640_TRANSFER(&_mc_23k640_buffer[5], MC_23K640_CMD_TRANSFER_SIZE, NULL, 0),                          /* Write a read cmd */
      MC_23K640_TRANSFER(NULL, 0, data, MC_23K640_SINGLE_TRANSFER_SIZE),                                        /* Read data */ 
      MC_23K640_TRANSFER(NULL, 0, &data[MC_23K640_SINGLE_TRANSFER_SIZE], read_size - MC_23K640_SINGLE_TRANSFER_SIZE)  /* Read data */ 
    };

   /* Write current index in RAM */
    mc_23k640_cs_activate();
    if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[0], 1, NULL) != NRF_SUCCESS) {
      mc_23k640_cs_deactivate();
      return -1;
    } 
    mc_23k640_cs_deactivate();

    /* Read data in RAM */
    mc_23k640_cs_activate();
    if(nrf_spi_mngr_perform(_p_spi_mngr, &_spi_config_8m, &transfers[1], 3, NULL) != NRF_SUCCESS) {
      mc_23k640_cs_deactivate();
      return -1;
    } 
  }
  mc_23k640_cs_deactivate();
  
  /* Clear static indexes */
  circular_buffer.tail = 0;
  circular_buffer.head = 0;

  /* Return the remaining data size in RAM */
  return remaining_bytes;
}
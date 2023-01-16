/*
* @file		ads129x.c
* @date		June 2021
* @author	PFaria & JAntunes
*
* @brief        This file has the ads129x adc utilities.
*
* Copyright(C)  2020-2021, PFaria & JAntunes
* All rights reserved.
*/

#ifndef ADS129X_H_
#define ADS129X_H_

/********************************** Includes ***********************************/
/* Utilities */
#include "spi_mngr.h"

/* SDK */
#include "nrf_spi_mngr.h"

/* Config */
//#include "config.h"

/********************************** Definições ***********************************/
/* Pinos ADS129x  **************** Colocar no config.h **************** */  
#define ADS129X_PACE_RST_PIN      NRF_GPIO_PIN_MAP(0, 11)     /* Output Pin to reset latch */
#define ADS129X_DRDY_PIN          12                          /* Input Pin para obter sincronização */
#define ADS129X_PWDN_PIN          NRF_GPIO_PIN_MAP(0, 18)     /* Output Pin to power on/off */

/* Pinos ADS1298 */
#define ADS129X_8_CS_PIN          NRF_GPIO_PIN_MAP(0, 16)     /* Output Pin of CS */              

/* Pinos ADS1296R */
#define ADS129X_6R_CS_PIN         NRF_GPIO_PIN_MAP(0, 17)     /* Output Pin of CS */ 
/* Pinos ADS129x  **************** Colocar no config.h **************** */  

#define ADS129X_INIT_RETIRES      3

/* ************** Comandos ************** */
/* Comandos de Sistema */
#define ADS129X_WAKEUP_CMD        0x02
#define ADS129X_STANDBY_CMD       0x04
#define ADS129X_RESET_CMD         0x06
#define ADS129X_START_CMD         0x08
#define ADS129X_STOP_CMD          0x0A

/* Comandos de DATA READ */
#define ADS129X_RDATAC_CMD        0x10
#define ADS129X_SDATAC_CMD        0x11
#define ADS129X_RDATA_CMD         0x12

/* Comandos de REGISTER READ */
#define ADS129X_RREG_1BYTE_CMD    0x20        
#define ADS129X_RREG_2BYTE_CMD    0x00
#define ADS129X_WREG_1BYTE_CMD    0x40
#define ADS129X_WREG_2BYTE_CMD    0x00


/* ************** Endereços ************** */
/* Definições do device */
#define ADS129X_REG_ID            0x00
#define ADS129X_REG_CONFIG1       0x01
#define ADS129X_REG_CONFIG2       0x02
#define ADS129X_REG_CONFIG3       0x03
#define ADS129X_REG_LOFF          0x04

/* Definições Globais a todos os canais */
#define ADS129X_CH1SET            0x05
#define ADS129X_CH2SET            0x06
#define ADS129X_CH3SET            0x07
#define ADS129X_CH4SET            0x08
#define ADS129X_CH5SET            0x09
#define ADS129X_CH6SET            0x0A
#define ADS129X_CH7SET            0x0B
#define ADS129X_CH8SET            0x0C
#define ADS129X_RLD_SENSP         0x0D
#define ADS129X_RLD_SENSN         0x0E
#define ADS129X_LOFF_SENSP        0x0F
#define ADS129X_LOFF_SENSN        0x10
#define ADS129X_LOFF_FLIP         0x11

/* Estado do Lead-off */
#define ADS129X_LOFF_STATP        0x12
#define ADS129X_LOFF_STAPN        0x13

/* GPIO e outros registos */
#define ADS129X_GPIO              0x14
#define ADS129X_PACE              0x15
#define ADS129X_RESP              0x16
#define ADS129X_CONFIG4           0x17
#define ADS129X_WCT1              0x18
#define ADS129X_WCT2              0x19


/* ************** Configurações ************** */ /* !! Verificar em caso de falha na validação !! */
#define ADS129X_CONFIG3_INIT            0xC0      /* CONFIG3 */
#define ADS129X_CONFIG1_HP_VALUE        0x86//0x86      /* CONFIG 1 - High resolution for 500 SPS */
#define ADS129X_CONFIG1_LP_VALUE        0x06      /* CONFIG 1 - Low Power for 250 SPS */
#define ADS129X_CONFIG2_VALUE           0x30      /* CONFIG 2 - Deve ser variável na config inicial */
#define ADS129X_LOFF_VALUE              0x07      /* LOFF - Predefinição DC */
typedef enum {                                    /* CHNSET - normal input - deve ser variável pelo utilizador ToDo */      
  ADS129X_CHNSET_GAIN1    = (0x01) << 4,
  ADS129X_CHNSET_GAIN2    = (0x02) << 4,
  ADS129X_CHNSET_GAIN3    = (0x03) << 4,
  ADS129X_CHNSET_GAIN4    = (0x04) << 4,
  ADS129X_CHNSET_GAIN6    = (0x05) << 4,
  ADS129X_CHNSET_GAIN8    = (0x06) << 4,
  ADS129X_CHNSET_GAIN12   = (0x00),
} ads129x_chnset_value;

#define ADS129X_CHNSET_OFF              0x81      /* CHNSET - off */
#define ADS129X_CHNSET_SUPPLY_MEAS      0x13      /* CHNSET - Supply measurement */
#define ADS129X_CHNSET_TEMP_MEAS        0x14      /* CHNSET - Temperature measurement */
#define ADS129X_CHNSET_TEST_SIG_MEAS    0x15      /* CHNSET - Test Signal measurement - GAIN = 1 */
//#define ADS129X_CHNSET_TEST_SIG_MEAS    0x35      /* CHNSET - Test Signal measurement - GAIN = 3 */
//#define ADS129X_CHNSET_TEST_SIG_MEAS    0x05      /* CHNSET - Test Signal measurement - GAIN = 6 */
#define ADS129X_RLD_SENSP_VALUE         0x00      /* RLD_SENSP */
#define ADS129X_RLD_SENSN_VALUE         0x00      /* RLD_SENSN */
#define ADS129X_LOFF_FLIP_VALUE         0x00      /* LOFF_FLIP */
#define ADS129X_LOFF_STATP_VALUE        0x00      /* LOFF_STATP - apenas de leitura */
#define ADS129X_LOFF_STATN_VALUE        0x00      /* LOFF_STATN - apenas de leitura */
#define ADS129X_GPIO_VALUE              0x0F      /* GPIO */ 
#define ADS129X_PACE_OFF_VALUE          0x00      /* PACE - desativado */
#define ADS129X_RESP_OFF_VALUE          0x20      /* RESP - ON */

/* Configurações ADS1298 */
#define ADS129X_8_ID_VALUE              0x92      /* ID */
#define ADS129X_8_CONFIG3_VALUE         0xC4//0xCC      /* CONFIG3 */  
#define ADS129X_8_LOFF_SENSP_ON_VALUE   0xFF      /* LOFF_SENSP - ON */  
#define ADS129X_8_LOFF_SENSP_OFF_VALUE  0x00      /* LOFF_SENSP - OFF */ 
#define ADS129X_8_LOFF_SENSN_ON_VALUE   0x07      /* LOFF_SENSN - ON */  
#define ADS129X_8_LOFF_SENSN_OFF_VALUE  0x00      /* LOFF_SENSN - OFF */
#define ADS129X_8_PACE_VALUE            0x19      /* PACE - ativado */ 
#define ADS129X_8_CONFIG4_ON_VALUE      0x06      /* CONFIG4 - comp. leadoff on */ 
#define ADS129X_8_CONFIG4_OFF_VALUE     0x04      /* CONFIG4 - comp. leadoff off */ 
#define ADS129X_8_WCT1_VALUE            0xEC      /* WCT1 */
#define ADS129X_8_WCT2_VALUE            0xC8      /* WCT2 */

/* Configurações ADS1296R */
#define ADS129X_6R_ID_VALUE             0xD1      /* ID */   
#define ADS129X_6R_CONFIG3_VALUE        0xC8      /* CONFIG3 */   
#define ADS129X_6R_CHNSET_SHORT_VALUE   0x01      /* CHNSET - OFF */
#define ADS129X_6R_CHNSET_OFF_VALUE     0x00      /* CHNSET - OFF */
#define ADS129X_6R_LOFF_SENSP_ON_VALUE  0x1E      /* LOFF_SENSP - ON */  
#define ADS129X_6R_LOFF_SENSP_OFF_VALUE 0x00      /* LOFF_SENSP - OFF */ 
#define ADS129X_6R_LOFF_SENSN_ON_VALUE  0x00      /* LOFF_SENSN - ON */  
#define ADS129X_6R_LOFF_SENSN_OFF_VALUE 0x00      /* LOFF_SENSN - OFF */
#define ADS129X_6R_PACE_VALUE           0x0B      /* PACE - ativado */
#define ADS129X_6R_RESP_ON_VALUE        0xE6      /* RESP - ON predefinição */
#define ADS129X_6R_CONFIG4_ON_VALUE     0x02      /* CONFIG4 - comp. leadoff on */ 
#define ADS129X_6R_CONFIG4_OFF_VALUE    0x00      /* CONFIG4 - comp. leadoff off */ 
#define ADS129X_6R_WCT1_VALUE           0x00      /* WCT1 */
#define ADS129X_6R_WCT2_VALUE           0x00      /* WCT2 */

/* Características do ADS129x */
#define ADS129X_TZERO                   0         /* Tempo de espera zero - dummy */       
#define ADS129X_OS                      1         /* Tempo de Power Up dos devices - VCAP > 1.1V */                     /* Recomendado -> 20 us */                                             
#define ADS129X_TPOR                    200       /* Tempo de Power Up dos devices - VCAP > 1.1V */                     /* Recomendado -> (2^8)*t_clk = 134,74 ms */                                             
#define ADS129X_TRST                    1         /* Tempo de envio de CMD após RST */                                   /* Recomendado -> 4*t_clk   = 2.056 us */
#define ADS129X_TSTOP                   1         /* Tempo após envio de comnado SDATAC */                              /* Recomendado -> 4*t_clk   = 2.056 us */ 
#define ADS129X_TVREF                   200       /* Tempo de envio de CMD após Ativar tensão interna de referência */  /* Recomendado -> 150 ms */   
#define ADS129X_TWAKEUP                 100       /* Tempo de envio de CMD após WAKEUP */                               /* Recomendado -> 9 ms */ 
#define ADS129X_TMAX                    0xFFFFFFFF

#define ADS129X_REG_BUFFER_SIZE         1                           /* Register Size 1 byte */
#define ADS129X_SW_BUFFER_SIZE          216/8                       /* Status Word Size 27 bytes */
#define ADS129X_8_SW_BUFFER_SIZE        ADS129X_SW_BUFFER_SIZE - 3  /* Status Word Size 27 bytes */
#define ADS129X_6R_SW_BUFFER_SIZE       ADS129X_SW_BUFFER_SIZE - 72/8 /* Status Word Size 27 bytes */
#define ADS129X_NUMBER_REG              25-1      /* Número de registos total */
#define ADS129X_8_NUMBER_REG_ECG        8         /* Número de registos total */
#define ADS129X_6_NUMBER_REG_ECG        4         /* Número de registos total */
#define ADS129X_CONFIGS_SIZE            27
#define ADS129X_USER_CONFIGS_SIZE_8     10
#define ADS129X_USER_CONFIGS_SIZE_6R    6
#define ADS129X_CHNSET_TEST_SIG_GAIN    1         /* CHNSET - Test Signal measurement - GAIN = 1 */
/* Identificação dos ganhos dos Canais analógicos */
typedef enum {
  ADS129X_GAIN1    = (1),
  ADS129X_GAIN2    = (2),
  ADS129X_GAIN3    = (3),
  ADS129X_GAIN4    = (4),
  ADS129X_GAIN6    = (6),
  ADS129X_GAIN8    = (8),
  ADS129X_GAIN12   = (12),
} ads129x_pga_gain;

/* Identificação do Test mode */
typedef enum {
  ADS129X_NORMAL    = (0),
  ADS129X_SUPPLY    = (1),
  ADS129X_TEMP      = (2),
  ADS129X_SIGNAL    = (3),
} ads129x_test_mode;
                                                              
/* ************** Macros utilitárias ************** */
#define ADS129X_REG_MASK                0x1F
/* Obter 1ºByte do comando efetivo de REGISTER READ */
/*
* Concatenar o número de registos para fazer leitura 
* n - número de registos a fazer leitura -1
*/
#define ADS129X_RREG_CMD_1BYTE_CONCAT(ads129x_reg)                        (ADS129X_RREG_1BYTE_CMD | (ads129x_reg & ADS129X_REG_MASK))

/* Obter 1ºByte do comando efetivo de REGISTER READ */
/*
* Concatenar o registo inicial para começar a fazer leitura
* ads129x_reg - registo para realizar leitura (5 bits)
*/
#define ADS129X_RREG_CMD_2BYTE_CONCAT(ads129x_n_reg)                      (ADS129X_RREG_2BYTE_CMD | (ads129x_n_reg & ADS129X_REG_MASK))

/* Obter comando de REGISTER WRITE */
/*
* Concatenar o número de registos para fazer escrita
* n - número de registos a fazer escrita -1
*/
#define ADS129X_WREG_CMD_1BYTE_CONCAT(ads129x_reg)                        (ADS129X_WREG_1BYTE_CMD | (ads129x_reg & ADS129X_REG_MASK))

/* Obter comando de REGISTER WRITE */
/*
* Concatenar o registo inicial para começar a fazer escrita
* ads129x_reg - registo para realizar escrita (5 bits)
*/
#define ADS129X_WREG_CMD_2BYTE_CONCAT(ads129x_n_reg)                      (ADS129X_WREG_2BYTE_CMD | (ads129x_n_reg & ADS129X_REG_MASK))

/* Transferência SPI */
/*
* _p_tx_data - ponteiro para o data a enviar
* _tx_length - número de bytes a enviar
* _p_rx_data - ponteiro para o data recebido
* _rx_length - número de bytes a receber
*/
#define ADS129X_TRANSFER(_p_tx_data, _tx_length, _p_rx_data, _rx_length) \
  NRF_SPI_MNGR_TRANSFER(_p_tx_data, _tx_length, _p_rx_data, _rx_length) 


/* Comando de configuração dos registos do ADS1298 */
/* 
* accuracy      - true for high resolution or false for low power resolution
* lead_off      - true for lead off detection on, or false otherwise
* chnset_value  - valor de configuração dos registos CH[1:8]SET
*/
#define ADS129X_8_CONFIG_CMD(accuracy, lead_off, chnset_value)                    \
  {                                                                               \
    ADS129X_WREG_CMD_1BYTE_CONCAT(ADS129X_REG_CONFIG1),                           \
    ADS129X_WREG_CMD_2BYTE_CONCAT(ADS129X_NUMBER_REG),                            \
    (accuracy ? ADS129X_CONFIG1_HP_VALUE : ADS129X_CONFIG1_LP_VALUE),             \
    ADS129X_CONFIG2_VALUE,                                                        \
    ADS129X_8_CONFIG3_VALUE,                                                      \
    ADS129X_LOFF_VALUE,                                                           \
    chnset_value,                                                                 \
    chnset_value,                                                                 \
    chnset_value,                                                                 \
    ADS129X_CHNSET_OFF,                                                                 \
    ADS129X_CHNSET_OFF,                                                                 \
    ADS129X_CHNSET_OFF,                                                                 \
    ADS129X_CHNSET_OFF,                                                                 \
    ADS129X_CHNSET_OFF,                                                                 \
    ADS129X_RLD_SENSP_VALUE,                                                      \
    ADS129X_RLD_SENSN_VALUE,                                                      \
    (lead_off ? ADS129X_8_LOFF_SENSP_ON_VALUE : ADS129X_8_LOFF_SENSP_OFF_VALUE),  \
    (lead_off ? ADS129X_8_LOFF_SENSN_ON_VALUE : ADS129X_8_LOFF_SENSN_OFF_VALUE),  \
    ADS129X_LOFF_FLIP_VALUE,                                                      \
    ADS129X_LOFF_STATP_VALUE,                                                     \
    ADS129X_LOFF_STATN_VALUE,                                                     \
    ADS129X_GPIO_VALUE,                                                           \
    ADS129X_PACE_OFF_VALUE,                                                             \
    ADS129X_RESP_OFF_VALUE,                                                       \
    (lead_off ? ADS129X_8_CONFIG4_ON_VALUE : ADS129X_8_CONFIG4_OFF_VALUE),        \
    ADS129X_8_WCT1_VALUE,                                                         \
    ADS129X_8_WCT2_VALUE                                                          \
  }
//ADS129X_8_PACE_VALUE,                                                         \

/* Comando de configuração dos registos do ADS1296R */
/* 
* accuracy      - true for high resolution or false for low power resolution
* lead_off      - true for lead off detection on, or false otherwise
* chnset_value  - valor de configuração dos registos CH[1:8]SET
* chnset_value  - valor de configuração dos registos CH[1:8]SET dedicado ao Resp
*/
#define ADS129X_6R_CONFIG_CMD(accuracy, lead_off, chnset_value, chnset_resp)        \
  {                                                                                 \
    ADS129X_WREG_CMD_1BYTE_CONCAT(ADS129X_REG_CONFIG1),                             \
    ADS129X_WREG_CMD_2BYTE_CONCAT(ADS129X_NUMBER_REG),                              \
    (accuracy ? ADS129X_CONFIG1_HP_VALUE : ADS129X_CONFIG1_LP_VALUE),               \
    ADS129X_CONFIG2_VALUE,                                                          \
    ADS129X_6R_CONFIG3_VALUE,                                                       \
    ADS129X_LOFF_VALUE,                                                             \
    chnset_resp,                                                                    \
    ADS129X_CHNSET_OFF,                                                                   \
    ADS129X_CHNSET_OFF,                                                                   \
    ADS129X_CHNSET_OFF,                                                                   \
    ADS129X_CHNSET_OFF,                                                                   \
    ADS129X_6R_CHNSET_SHORT_VALUE,                                                  \
    ADS129X_6R_CHNSET_OFF_VALUE,                                                    \
    ADS129X_6R_CHNSET_OFF_VALUE,                                                    \
    ADS129X_RLD_SENSP_VALUE,                                                        \
    ADS129X_RLD_SENSN_VALUE,                                                        \
    (lead_off ? ADS129X_6R_LOFF_SENSP_ON_VALUE : ADS129X_6R_LOFF_SENSP_OFF_VALUE),  \
    (lead_off ? ADS129X_6R_LOFF_SENSN_ON_VALUE : ADS129X_6R_LOFF_SENSN_OFF_VALUE),  \
    ADS129X_LOFF_FLIP_VALUE,                                                        \
    ADS129X_LOFF_STATP_VALUE,                                                       \
    ADS129X_LOFF_STATN_VALUE,                                                       \
    ADS129X_GPIO_VALUE,                                                             \
    ADS129X_PACE_OFF_VALUE,                                                         \
    ADS129X_6R_RESP_ON_VALUE,                                                       \
    (lead_off ? ADS129X_6R_CONFIG4_ON_VALUE : ADS129X_6R_CONFIG4_OFF_VALUE),        \
    ADS129X_6R_WCT1_VALUE,                                                          \
    ADS129X_6R_WCT2_VALUE                                                           \
  }

/* Estados da inicialização */
typedef enum {
  ADS129X_INIT_CONFIG_ST,
  ADS129X_POWER_INIT_ST,
  ADS129X_RESET_ST,
  ADS129X_SDATAC_ST,
  ADS129X_VOLTAGE_REF_ST,
  ADS129X_GET_ID_ST,      
} ads129x_init_states;

/* Identificação do device */
typedef enum {
  ADS129X_6R  = (6),
  ADS129X_8   = (8),  
} ads129x_devices;

/* Data Utilities */
#define ADS129X_DATA_ID               0xC0
#define ADS129X_VERIFY_DATA_ID(value) ((value & 0xF0) == ADS129X_DATA_ID) ? 1 : 0
#define ADS129X_PACE_DEVICE           ADS129X_8
#define ADS129X_LA_BIT                0
#define ADS129X_RA_BIT                4
#define ADS129X_LL_BIT                5
#define ADS129X_V1_BIT                7
#define ADS129X_V2_BIT                3
#define ADS129X_V3_BIT                5
#define ADS129X_V4_BIT                6
#define ADS129X_V5_BIT                7
#define ADS129X_V6_BIT                0
#define ADS129X_PACE_BIT              0
#define ADS129X_ELECT_NUMB            10
#define ADS129X_STAT_IDX              8
#define ADS129X_DATA_IDX              18
#define ADS129x_INTEGER_CONVERT       100000000

/* Estrutura de dados com o valor da Status Word do ADS129x e o respetivo timestamp */
typedef struct {
  uint8_t   ads1298_sw_buffer[ADS129X_SW_BUFFER_SIZE];
  uint8_t   ads1296r_sw_buffer[ADS129X_SW_BUFFER_SIZE];
  uint64_t  timestamp;
} ads129x_data; 

#define ADS129X_VERIFY_POL(value)   (value & 0x00800000) ? 1 : 0
#define ADS129X_REF                 2.4
#define ADS129X_RES                 24

/* Indíces dos buffers de dados ADS1298 e ADS1296R */ 
#define ADS129X_CH1_IDX               24/8              /* Channel 1 data */
#define ADS129X_CH2_IDX               48/8              /* Channel 2 data */
#define ADS129X_CH3_IDX               72/8              /* Channel 3 data */
#define ADS129X_CH4_IDX               96/8              /* Channel 4 data */
#define ADS129X_CH5_IDX               120/8             /* Channel 5 data */
#define ADS129X_CH6_IDX               144/8             /* Channel 6 data */
#define ADS129X_CH7_IDX               168/8             /* Channel 7 data - ADS1298 only */
#define ADS129X_CH8_IDX               192/8             /* Channel 8 data - ADS1298 only */
#define ADS129X_3BYTE                 3  
#define ADS129X_4BYTE                 4  
#define ADS129X_MEAS_DEBUG            1

/* Array with content order of ADS129x data */
#define ADS129X_DATA_ORDER  \
  {                         \
    MEAS_TIMESTAMP,         \
    MEAS_ECG_LOFF_LA,       \
    MEAS_ECG_LOFF_RA,       \
    MEAS_ECG_LOFF_LL,       \
    MEAS_ECG_LOFF_V1,       \
    MEAS_ECG_LOFF_V2,       \
    MEAS_ECG_LOFF_V3,       \
    MEAS_ECG_LOFF_V4,       \
    MEAS_ECG_LOFF_V5,       \
    MEAS_ECG_LOFF_V6,       \
    MEAS_ECG_PACE,          \
    MEAS_ECG_I,             \
    MEAS_ECG_II,            \
    MEAS_ECG_III,           \
    MEAS_ECG_AVR,           \
    MEAS_ECG_AVL,           \
    MEAS_ECG_AVF,           \
    MEAS_ECG_V2,            \
    MEAS_RESP,              \
    MEAS_ECG_V3,            \
    MEAS_ECG_V4,            \
    MEAS_ECG_V5,            \
    MEAS_ECG_V6             \
  }
#define ADS129X_DATA_SIZE   66

/* Estados das funções com temporização ou mais do que um estado */
typedef enum {
  ADS129X_TRUE,
  ADS129X_IDDLE,
  ADS129X_FAULT
} ads129x_func_status;

/********************************** Funções ***********************************/
bool ads129x_init(void);
void ads129x_uninit(void);
bool ads129x_configs(bool accuracy, bool lead_off, uint8_t ecg_gain, uint8_t resp_gain, uint8_t test_mode);
uint8_t ads129x_start_sample(void);
bool ads129x_start_datac(void);
bool ads129x_stop_datac(void);
uint8_t *ads129x_get_data(void);
bool ads129x_user_configs(bool accuracy, bool lead_off, uint8_t ecg_gain, uint8_t resp_gain, uint8_t test_mode);
void ads129x_pace_rst(bool state);

#endif /* ADS129X_H_ */



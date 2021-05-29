/********************************************************************
*                         Standard headers                          *
********************************************************************/
#ifndef NRF24201_H__
#define NRF24201_H__

#include <stdio.h>
#include <stdint.h>

/********************************************************************
*                           Local headers                           *
********************************************************************/
#include "nrf24.h"

/********************************************************************
*                       Function macro defines                      *
********************************************************************/
#define MAX_PAYLOAD_SIZE    (32)

#define NRF_STATUS_IDLE     (0 << 0)
#define NRF_STATUS_RX_DR    (1 << 6)
#define NRF_STATUS_TX_DS    (1 << 5)
#define NRF_STATUS_MAX_RT   (1 << 4)
#define NRF_STATUS_TX_FULL  (1 << 0)

/********************************************************************
*                             Typedefs                              *
********************************************************************/
typedef enum
{
    NRF_E_SUCCESS,
    NRF_E_INVALID_PARAM,
    NRF_E_NO_MEM,
    NRF_E_ALREADY_INITIALIZED,
    NRF_E_NOT_CONNECTED,
} nrf_error_t;

/* NRF24L01 Commands */
typedef enum
{
    NRF_CMD_R_REGISTER   = 0b00000000,
    NRF_CMD_W_REGISTER   = 0b00100000,
    NRF_CMD_R_RX_PAYLOAD = 0b01100001,
    NRF_CMD_W_TX_PAYLOAD = 0b10100000,
    NRF_CMD_FLUSH_TX     = 0b11100001,
    NRF_CMD_FLUSH_RX     = 0b11100010,
    NRF_CMD_R_RX_PL_WID  = 0b01100000,
    NRF_CMD_NOP          = 0b11111111
} nrf_cmd_t;

/* NRF24L01 Registers */
typedef enum
{
    NRF_REG_CONFIG      = 0x00,
    NRF_REG_EN_AA       = 0x01,
    NRF_REG_EN_RX_ADDR  = 0x02,
    NRF_REG_SETUP_AW    = 0x03,
    NRF_REG_SETUP_RETR  = 0x04,
    NRF_REG_RF_CH       = 0x05,
    NRF_REG_RF_SETUP    = 0x06,
    NRF_REG_STATUS      = 0x07,
    NRF_REG_OBSERVE_TX  = 0x08,
    NRF_REG_RX_ADDR_P0  = 0x0A,
    NRF_REG_RX_ADDR_P1,
    NRF_REG_RX_ADDR_P2,
    NRF_REG_RX_ADDR_P3,
    NRF_REG_RX_ADDR_P4,
    NRF_REG_RX_ADDR_P5,
    NRF_REG_TX_ADDR     = 0x10,
    NRF_REG_FIFO_STATUS = 0x17,
    NRF_REG_DYNPD       = 0x1C,
    NRF_REG_FEATURE     = 0x1D
} nrf_reg_t;

typedef enum
{
    NRF_PIPE_0,
    NRF_PIPE_1,
    NRF_PIPE_2,
    NRF_PIPE_3,
    NRF_PIPE_4,
    NRF_PIPE_5,
    NRF_PIPE_TX
} nrf_pipe_t;

typedef enum
{
    ADDR_SIZE_ILLEGAL,
    ADDR_SIZE_3B,
    ADDR_SIZE_4B,
    ADDR_SIZE_5B
} nrf_addr_size_t;

typedef enum
{
    NRF_MODE_POWER_DOWN,
    NRF_MODE_TX,
    NRF_MODE_RX,
} nrf_mode_t;

typedef enum
{
    NRF_CRC_DISABLE,
    NRF_CRC_1B,
    NRF_CRC_2B
} nrf_crc_t;

typedef enum
{
    NRF_DATA_RATE_1M,
    NRF_DATA_RATE_2M
} nrf_data_rate_t;

typedef enum
{
    NRF_OUTPUT_POWER_18DB,
    NRF_OUTPUT_POWER_12DB,
    NRF_OUTPUT_POWER_6DB,
    NRF_OUTPUT_POWER_0DB,
} nrf_output_power_t;

typedef void* nrf_dscr_t;
typedef void (*spi_transfer_t)(uint8_t *tx_buff, uint8_t *rx_buff, uint8_t len);

#endif /* NRF24201_H__ */
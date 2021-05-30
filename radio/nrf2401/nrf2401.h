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

/********************************************************************
*                       Function macro defines                      *
********************************************************************/
#define MAX_PAYLOAD_SIZE     (32)

#define NRF_FLAG_RX_DR    	 (1 << 6)
#define NRF_FLAG_TX_DS    	 (1 << 5)
#define NRF_FLAG_MAX_RT   	 (1 << 4)
#define NRF_FLAG_TX_FULL  	 (1 << 0)

#define NRF_FIFO_STATUS_TX_REUSE (1 << 6)
#define NRF_FIFO_STATUS_TX_FULL  (1 << 5)
#define NRF_FIFO_STATUS_TX_EMPTY (1 << 4)
#define NRF_FIFO_STATUS_RX_FULL  (1 << 1)
#define NRF_FIFO_STATUS_RX_EMPTY (1 << 0)

/********************************************************************
*                             Typedefs                              *
********************************************************************/
typedef enum
{
    NRF_E_SUCCESS,
    NRF_E_INVALID_PARAM,
    NRF_E_INTERNAL,
    NRF_E_NO_MEM,
    NRF_E_ALREADY_INITIALIZED,
    NRF_E_NOT_CONNECTED,
    NRF_E_INVALID_DATA_SIZE,
    NRF_E_FIFO_FULL,
    NRF_E_FIFO_EMPTY,
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
    NRF_CMD_ACTIVATE     = 0b01010000,
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
    NRF_REG_PAYLOAD_SIZE_P0,
    NRF_REG_PAYLOAD_SIZE_P1,
    NRF_REG_PAYLOAD_SIZE_P2,
    NRF_REG_PAYLOAD_SIZE_P3,
    NRF_REG_PAYLOAD_SIZE_P4,
    NRF_REG_PAYLOAD_SIZE_P5,
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

typedef void (*spi_transfer_t)(uint8_t *tx_buff, uint8_t *rx_buff, uint8_t len);

/********************************************************************
*                                API                                *
********************************************************************/
nrf_error_t nrf_init(spi_transfer_t spi_transfer);
nrf_error_t nrf_mode(nrf_mode_t mode);
nrf_error_t nrf_crc(nrf_crc_t crc);
nrf_error_t nrf_interrupt(bool enable);
nrf_error_t nrf_rx_pipe(nrf_pipe_t pipe, bool enable, bool ack);
nrf_error_t nrf_addr_size(nrf_addr_size_t size);
nrf_error_t nrf_retr_setup(uint8_t retr, uint8_t delay);
nrf_error_t nrf_rf_setup(uint8_t channel, nrf_data_rate_t rate, nrf_output_power_t power);
uint8_t nrf_status_get(void);
void nrf_flag_clear(uint8_t flags);
nrf_error_t nrf_addr_set(nrf_pipe_t pipe, uint8_t *addr);
nrf_error_t nrf_addr_get(nrf_pipe_t pipe, uint8_t *addr);
nrf_error_t nrf_payload_size(nrf_pipe_t pipe, uint8_t size);
uint8_t nrf_lost_packets_cnt(void);
uint8_t nrf_retr_cnt(void);
uint8_t nrf_fifo_status(void);
void nrf_fifo_flush_rx(void);
void nrf_fifo_flush_tx(void);
nrf_error_t nrf_fifo_push(uint8_t *data, uint8_t len);
nrf_error_t nrf_fifo_pop(uint8_t *data, uint8_t *len, nrf_pipe_t *pipe);
nrf_error_t nrf_dynamic_payload(nrf_pipe_t pipe, bool enable);
void nrf_feature(bool dpl, bool ack_pay, bool dyn_ack);
uint8_t nrf_read_reg(nrf_reg_t reg);

#endif /* NRF24201_H__ */

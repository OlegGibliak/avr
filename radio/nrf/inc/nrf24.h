#ifndef NRF24_H__
#define NRF24_H__

/********************************************************************
*                         Standard headers                          *
********************************************************************/
#include <stdint.h>
#include <stdbool.h>

/********************************************************************
*                           Local headers                           *
********************************************************************/

/********************************************************************
*                       Function macro defines                      *
********************************************************************/
#define MAX_PAYLOAD_SIZE    (32)

/********************************************************************
*                             Typedefs                              *
********************************************************************/
typedef enum
{
    NRF_E_SUCCESS,
    NRF_E_INVALID_PARAM,
    NRF_E_NOT_CONNECTED,
    NRF_E_INTERNAL,
    NRF_E_INVALID_SIZE,
    NRF_E_FIFO_FULL,
    NRF_E_FIFO_EMPTY,
} nrf_error_t;

typedef enum
{
    NRF_STATUS_MAX_RT = (1 << 4),
    NRF_STATUS_TX_DS  = (1 << 5),
    NRF_STATUS_RX_DR  = (1 << 6)
} nrf_status_t;

typedef enum
{
    NRF_ADDR_SIZE_3B = 1,
    NRF_ADDR_SIZE_4B,
    NRF_ADDR_SIZE_5B,
} nrf_addr_size_t;

typedef enum
{
    NRF_RX_P0,
    NRF_RX_P1,
    NRF_RX_P2,
    NRF_RX_P3,
    NRF_RX_P4,
    NRF_RX_P5,
    NRF_PIPE_TX
} nrf_pipe_t;

typedef enum
{
    NRF_AUTO_RETR_DISABLE,
    NRF_AUTO_RETR_1,
    NRF_AUTO_RETR_2,
    NRF_AUTO_RETR_3,
    NRF_AUTO_RETR_4,
    NRF_AUTO_RETR_5,
    NRF_AUTO_RETR_6,
    NRF_AUTO_RETR_7,
    NRF_AUTO_RETR_8,
    NRF_AUTO_RETR_9,
    NRF_AUTO_RETR_10,
    NRF_AUTO_RETR_11,
    NRF_AUTO_RETR_12,
    NRF_AUTO_RETR_13,
    NRF_AUTO_RETR_14,
    NRF_AUTO_RETR_15
} nrf_auto_retr_t;

typedef enum
{
    NRF_CRC_1B,
    NRF_CRC_2B,
    NRF_CRC_DISABLED
} nrf_crc_t;

typedef enum
{
    NRF_MODE_PTX,
    NRF_MODE_PRX
} nrf_mode_t;

typedef struct
{
    nrf_addr_size_t addr_size;
    nrf_auto_retr_t retr;
    uint8_t         delay;
    uint8_t         channel;
    nrf_crc_t       crc_mode;
    // nrf_tx_cb_t     tx_cb;
    // nrf_rx_cb_t     rx_cb;
} nrf_setup_t;

typedef void (*nrf_spi_t)(uint8_t *tx, uint8_t *rx, uint8_t length);


/********************************************************************
*                                API                                *
********************************************************************/
nrf_error_t  nrf_init(nrf_spi_t spi_rw);
nrf_error_t  nrf_is_chip_connected(void);
nrf_status_t nrf_status_get(void);
nrf_error_t  nrf_addr_size_set(nrf_addr_size_t addr_size);
nrf_addr_size_t  nrf_addr_size_get(void);
nrf_error_t  nrf_pipe_addr_get(nrf_pipe_t pipe, uint8_t *addr);
nrf_error_t  nrf_pipe_addr_set(nrf_pipe_t pipe, const uint8_t *addr, uint8_t addr_len);

nrf_error_t nrf_flush_tx_fifo(void);
nrf_error_t nrf_flush_rx_fifo(void);
nrf_error_t nrf_fifo_push(uint8_t *data, uint8_t len);
nrf_error_t nrf_fifo_pop(uint8_t *data, uint8_t *len, nrf_pipe_t *pipe);

void nrf_setup(nrf_setup_t *config);
nrf_error_t nrf_pipe_open(nrf_pipe_t pipe, uint8_t *addr);
void nrf_addr_get(nrf_pipe_t pipe, uint8_t *addr, uint8_t *addr_size);
void nrf_mode_set(nrf_mode_t mode);
void nrf_status_clear(nrf_status_t flags);
void nrf_observe_tx(uint8_t *retr_cnt, uint8_t *lost_cnt);
#endif /* NRF24_H__ */
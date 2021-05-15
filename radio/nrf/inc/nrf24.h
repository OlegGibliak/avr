#ifndef NRF24_H__
#define NRF24_H__

/********************************************************************
*                         Standard headers                          *
********************************************************************/
#include <stdint.h>

/********************************************************************
*                           Local headers                           *
********************************************************************/

/********************************************************************
*                       Function macro defines                      *
********************************************************************/

/********************************************************************
*                             Typedefs                              *
********************************************************************/
typedef enum
{
    NRF_E_SUCCESS,
    NRF_E_INVALID_PARAM,
    NRF_E_NOT_CONNECTED,
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
} nrf_pipe_t;

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

#endif /* NRF24_H__ */
/********************************************************************
*                         Standard headers                          *
********************************************************************/
#include <stdio.h>
#include <stdint.h>

/********************************************************************
*                           Local headers                           *
********************************************************************/
#include "nrf24.h"

/********************************************************************
*                       Function macro defines                      *
********************************************************************/
#define NRF_NOP             (0xFF)
#define NRF_DUMMY_OFFSET    (1)

/********************************************************************
*                             Typedefs                              *
********************************************************************/
/* NRF24L01 Commands */
typedef enum
{
    NRF_CMD_R_REGISTER   = 0b00000000,
    NRF_CMD_W_REGISTER   = 0b00100000,
    NRF_CMD_R_RX_PAYLOAD = 0b01100001,
    NRF_CMD_W_TX_PAYLOAD = 0b10100000,
    NRF_CMD_FLUSH_TX     = 0b11100001,
    NRF_CMD_FLUSH_RX     = 0b11100010,
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
    NRF_REG_STATUS      = 0x07,
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

/********************************************************************
*                  Static global data declarations                  *
********************************************************************/
static nrf_spi_t spi;

/********************************************************************
*                     Functions implementations                     *
********************************************************************/
static uint8_t reg_read(nrf_reg_t reg)
{
    uint8_t data[] = {NRF_CMD_R_REGISTER | reg, NRF_NOP};

    // uint8_t rx[sizeof(data)] = {0};
    spi(data, data, 2);
    return data[1];
}

static void reg_write(nrf_reg_t reg, uint8_t value)
{
    uint8_t data[] = {NRF_CMD_W_REGISTER | reg, value};
    spi(data, NULL, sizeof(data));
}

static void reg_read_bytes(nrf_reg_t reg, uint8_t *value, uint8_t len)
{
    uint8_t data[len + 1];
    data[0] = NRF_CMD_R_REGISTER | reg;

    spi(data, data, len + 1);

    for (uint32_t i = 0; i < len ; ++i)
        value[i] = data[i + 1];
}

static void reg_write_bytes(nrf_reg_t reg, const uint8_t *value, uint8_t len)
{
    uint8_t data[len + 1];
    data[0] = NRF_CMD_W_REGISTER | reg;

    for (uint32_t i = 0; i < len ; ++i)
        data[i + 1] = value[i];
    
    spi(data, NULL, len + 1);
}

/********************************************************************
*                                API                                *
********************************************************************/
nrf_error_t nrf_init(nrf_spi_t spi_rw)
{
    if (spi_rw == NULL)
    {
        return NRF_E_INVALID_PARAM;
    }
    spi = spi_rw;

    return NRF_E_SUCCESS;
}

nrf_error_t  nrf_is_chip_connected(void)
{
    uint8_t addr_size = reg_read(NRF_REG_SETUP_AW);
    if (addr_size < NRF_ADDR_SIZE_3B || addr_size > NRF_ADDR_SIZE_5B)
    {
        return NRF_E_NOT_CONNECTED;
    }

    return NRF_E_SUCCESS;
}

nrf_status_t nrf_status_get(void)
{
    uint8_t sts_reg = reg_read(NRF_REG_STATUS);

    return sts_reg & (NRF_STATUS_MAX_RT | NRF_STATUS_TX_DS | NRF_STATUS_RX_DR);
}

nrf_addr_size_t  nrf_addr_size_get(void)
{
    return reg_read(NRF_REG_SETUP_AW);
}

nrf_error_t nrf_addr_size_set(nrf_addr_size_t addr_size)
{
    if (addr_size < NRF_ADDR_SIZE_3B || addr_size > NRF_ADDR_SIZE_5B)
    {
        return NRF_E_INVALID_PARAM;
    }

    reg_write(NRF_REG_SETUP_AW, addr_size);
    return NRF_E_SUCCESS;
}

nrf_error_t nrf_pipe_addr_get(nrf_pipe_t pipe, uint8_t *addr)
{
    if (addr == NULL)
    {
        return NRF_E_INVALID_PARAM;
    }

    uint8_t addr_size = nrf_addr_size_get();
    switch (pipe)
    {
        case NRF_RX_P0: reg_read_bytes(NRF_REG_RX_ADDR_P0, addr, addr_size); break;
        case NRF_RX_P1: reg_read_bytes(NRF_REG_RX_ADDR_P1, addr, addr_size); break;
        case NRF_RX_P2: reg_read_bytes(NRF_REG_RX_ADDR_P2, addr, addr_size); break;
        case NRF_RX_P3: reg_read_bytes(NRF_REG_RX_ADDR_P3, addr, addr_size); break;
        case NRF_RX_P4: reg_read_bytes(NRF_REG_RX_ADDR_P4, addr, addr_size); break;
        case NRF_RX_P5: reg_read_bytes(NRF_REG_RX_ADDR_P5, addr, addr_size); break;    
        default:
            return NRF_E_INVALID_PARAM;
    }

    return NRF_E_SUCCESS;
}

nrf_error_t nrf_pipe_addr_set(nrf_pipe_t pipe, const uint8_t *addr, uint8_t addr_len)
{
    if (addr == NULL)
    {
        return NRF_E_INVALID_PARAM;
    }

    uint8_t addr_size = nrf_addr_size_get();
    switch (pipe)
    {
        case NRF_RX_P0: reg_write_bytes(NRF_REG_RX_ADDR_P0, addr, addr_size); break;
        case NRF_RX_P1: reg_write_bytes(NRF_REG_RX_ADDR_P1, addr, addr_size); break;
        case NRF_RX_P2: reg_write_bytes(NRF_REG_RX_ADDR_P2, addr, addr_size); break;
        case NRF_RX_P3: reg_write_bytes(NRF_REG_RX_ADDR_P3, addr, addr_size); break;
        case NRF_RX_P4: reg_write_bytes(NRF_REG_RX_ADDR_P4, addr, addr_size); break;
        case NRF_RX_P5: reg_write_bytes(NRF_REG_RX_ADDR_P5, addr, addr_size); break;    
        default:
            return NRF_E_INVALID_PARAM;
    }

    return NRF_E_SUCCESS;
}

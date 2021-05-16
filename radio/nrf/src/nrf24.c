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
#define MAX_PAYLOAD_SIZE    (32)

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

typedef struct __attribute__((packed))
{
    uint8_t prim_rx     :1; /*< RX/TX control 1: PRX, 0: PTX.*/
    uint8_t pwr_up      :1; /*< 1 - POWER UP, 0 - POWER DOWN.*/
    uint8_t crco        :1; /*< CRC encoding scheme '0' - 1 byte, '1' – 2 bytes.*/
    uint8_t en_crc      :1; /*< Enable CRC. Forced high if one of the bits in the EN_AA is high.*/
    uint8_t mask_max_rt :1; /*< Mask interrupt caused by MAX_RT 1: Interrupt not reflected on the IRQ pin
                                                                0: Reflect MAX_RT as active low interrupt on the IRQ pin.*/
    uint8_t mask_tx_ds  :1; /*< Mask interrupt caused by TX_DS 1: Interrupt not reflected on the IRQ pin
                                                                0: Reflect TX_DS as active low interrupt on the IRQpin.*/
    uint8_t mask_rx_dr  :1; /*< Mask interrupt caused by RX_DR 1: Interrupt not reflected on the IRQ pin
                                                                0: Reflect RX_DR as active low interrupt on the IRQ pin.*/
    uint8_t _rfu        :1;
} config_reg_t;

typedef struct
{
    uint8_t auto_retr_cnt   :4; /*< Auto Retransmit Coun.*/
    uint8_t auto_retr_delay :4; /*< Auto Retransmit Delay. Delay defined from end of transmission to start of next transmission. */
} setup_retr_t;

typedef struct
{
    uint8_t rx_empty :1;
    uint8_t rx_full  :1;
    uint8_t _rfu1    :2;
    uint8_t tx_empty :1;
    uint8_t tx_full  :1;
    uint8_t tx_reuse :1; /*< Used for a PTX devicePulse the rfce high for at least 10μs to Reuse last transmitted payload.
                          * TX payload reuse is active until W_TX_PAYLOAD or FLUSH TX is executed.
                          * TX_REUSE is set by the SPI command REUSE_TX_PL, and is reset by the SPI commands W_TX_PAYLOAD or FLUSH TX.*/
    uint8_t _rfu2    :1;
} nrf_fifo_info_t;

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

void nrf_setup(nrf_setup_t *config)
{
    config_reg_t reg = {.mask_max_rt = 1,
                        .mask_rx_dr  = 1,
                        .mask_tx_ds  = 1,
                        .prim_rx     = 1,
                        .pwr_up      = 1};

    if(config->crc_mode != NRF_CRC_DISABLED)
    {
        reg.en_crc = 1;
        reg.crco   = config->crc_mode;
    }
    else
    {
        reg.en_crc = 0;
    }
    reg_write_bytes(NRF_REG_CONFIG, (uint8_t *)&reg, 1);

    setup_retr_t retr = {.auto_retr_cnt   = config->retr,
                         .auto_retr_delay = config->delay};
    reg_write_bytes(NRF_REG_SETUP_RETR, (uint8_t *)&retr, 1);

    reg_write(NRF_REG_RF_CH, config->channel);

    reg_write(NRF_REG_SETUP_AW, config->addr_size);
}

void nrf_flush_tx_fifo(void)
{
    reg_write_bytes(NRF_CMD_FLUSH_TX, NULL, 0);
}

void nrf_flush_rx_fifo(void)
{
    reg_write_bytes(NRF_CMD_FLUSH_RX, NULL, 0);
}

nrf_error_t nrf_fifo_push(uint8_t *data, uint8_t len)
{
    if (len > MAX_PAYLOAD_SIZE)
    {
        return NRF_E_INVALID_SIZE;
    }
    uint8_t reg = reg_read(NRF_REG_FIFO_STATUS);
    nrf_fifo_info_t *fifo = (nrf_fifo_info_t *)&reg;
    if (fifo->tx_full)
    {
        return NRF_E_FIFO_FULL;
    }
    
    reg_write_bytes(NRF_CMD_W_TX_PAYLOAD, data, len);

    reg = reg_read(NRF_REG_FIFO_STATUS);
    return (fifo->tx_empty) ? NRF_E_INTERNAL : NRF_E_SUCCESS;
}

nrf_error_t nrf_fifo_pop(uint8_t *data, uint8_t *len)
{
    uint8_t reg = reg_read(NRF_REG_FIFO_STATUS);
    nrf_fifo_info_t *fifo = (nrf_fifo_info_t *)&reg;
    if (fifo->rx_empty)
    {
        return NRF_E_FIFO_EMPTY;
    }

    uint8_t pl_size = reg_read(NRF_CMD_R_RX_PL_WID);
    if (pl_size =0 || pl_size > MAX_PAYLOAD_SIZE)
    {
        return NRF_E_INTERNAL;
    }

    reg_read_bytes(NRF_CMD_R_RX_PAYLOAD, data, pl_size);
    *len = pl_size;
}

nrf_error_t nrf_pipe_open(nrf_pipe_t pipe, uint8_t *addr)
{
    uint8_t addr_size = reg_read(NRF_REG_SETUP_AW);
    if (addr_size < NRF_ADDR_SIZE_3B|| addr_size > NRF_ADDR_SIZE_5B)
    {
        return NRF_E_INTERNAL;
    }

    addr_size += 2;
    if (pipe == NRF_PIPE_TX)
    {
        reg_write_bytes(NRF_REG_TX_ADDR, addr, addr_size);
    }
    else if (pipe < NRF_PIPE_TX)
    {
        if (pipe == NRF_RX_P0)
        {
            //TODO
            // return NRF_E_INVALID_PARAM;
        }

        if (pipe == NRF_RX_P0 || pipe == NRF_RX_P1)
        {
            reg_write_bytes(NRF_REG_RX_ADDR_P0 + pipe, addr, addr_size);
        }
        else
        {
            reg_write_bytes(NRF_REG_RX_ADDR_P0 + pipe, addr, 1);
        }

        uint8_t en_rx_addr = (1 << pipe);
        reg_write(NRF_REG_EN_RX_ADDR, en_rx_addr);
    }
    else
    {
        return NRF_E_INVALID_PARAM;
    }
    
    return NRF_E_SUCCESS;
}

#include <stdint.h>
#include <stddef.h>
#include "nrf24_ll.h"

#include "app_timer.h"
#include "spi.h"

#define NRF24_SEND_TIMEOUT_MS   5

#define PAYLOAD_MAX_SIZE        32

typedef enum
{
    NRF_INTERRUP_ENABLE,   
    NRF_INTERRUP_DISABLE
} nrf24_interrupt_t;

typedef enum
{
    NRF_STATE_TX_READY,
    NRF_STATE_TX_TRANSMIT,
    NRF_STATE_RX_READY 
} nrf24_ll_state_t;

typedef struct
{
    nrf24_ll_state_t     state;
    app_timer_t          timer;

    nrf_rx_cb_t          rx_callback;
    nrf_tx_complete_cb_t tx_callback;
} nrf_ll_dscr_t;

typedef struct __attribute((__packed__))
{
    union
    {
        struct __attribute((__packed__))
        {
            uint8_t prim_rx     : 1;    /* RX/TX control. 1: PRX, 0: PTX. */
            uint8_t pwr_up      : 1;    /* 1: POWER UP, 0:POWER DOWN. */
            uint8_t crco        : 1;    /* CRC encoding scheme. '0' - 1 byte '1' – 2 bytes. */
            uint8_t en_crc      : 1;    /* Enable CRC. Forced high if one of the bits in the EN_AA is high */
            uint8_t mask_max_rt : 1;    /* Mask interrupt caused by MAX_RT:
                                         * 1: Interrupt not reflected on the IRQ pin.
                                         * 0: Reflect MAX_RT as active low interrupt on the IRQ pin. */
            uint8_t mask_tx_ds  : 1;    /* Mask interrupt caused by TX_DS:
                                         * 1: Interrupt not reflected on the IRQ pin
                                         * 0: Reflect TX_DS as active low interrupt on the IRQpin. */
            uint8_t mask_rx_dr  : 1;    /* Mask interrupt caused by RX_DR:
                                         * 1: Interrupt not reflected on the IRQ pin.
                                         * 0: Reflect RX_DR as active low interrupt on the IRQ pin. */
            uint8_t _rfu        : 1;    /* Reserved for future. */
        };
        uint8_t byte;
    };
} nrf24_config_t;

typedef struct __attribute((__packed__))
{
    union
    {
        struct __attribute((__packed__))
        {
            uint8_t _rfu       : 1;
            uint8_t rf_pwr     : 2;     /* Set RF output power in TX mode '00'=-18dBm '01'=-12dBm '10'=-6dBm '11'=0dBm. */
            uint8_t rf_dr_high : 1;     /* Select between the high speed data rates. This bit is don’t care if RF_DR_LOW is set.
                                         *[RF_DR_LOW, RF_DR_HIGH]:‘00’ – 1Mbps ‘01’ – 2Mbps ‘10’ – 250kbps ‘11’ – Reserved. */
            uint8_t pll_lock   : 1;     /* Force PLL lock signal. Only used in test. */
            uint8_t rf_dr_low  : 1;     /* Set RF Data Rate to 250kbps. */
            uint8_t _rfu2      : 1;
            uint8_t cont_wave  : 1;     /* Enables continuous carrier transmit when high. */
        };
        uint8_t byte;
    };
} nrf24_setup_t;

typedef struct __attribute((__packed__))
{
    union
    {
        struct __attribute((__packed__))
        {
            uint8_t aw   : 2;           /* RX/TX Address field width.
                                         * '00' - Illegal
                                         * '01' - 3 bytes
                                         * '10' - 4 bytes
                                         * '11' – 5 bytes */
            uint8_t _rfu : 6; 
        };
        uint8_t byte;
    };
} nrf24_setup_aw_t;

typedef struct __attribute((__packed__))
{
    union
    {
        struct __attribute((__packed__))
        {
            uint8_t arc : 4;           /* Auto Retransmit Count:
                                        * ‘0000’ – Re-Transmit disabled
                                        * ‘0001’ – Up to 1 Re-Transmit on fail of AA
                                        * ......
                                        * ‘1111’ – Up to 15 Re-Transmit on fail of AA */
            uint8_t ard : 4;           /* Auto Retransmit Delay:
                                        * ‘0000’ – Wait 250μS
                                        * ‘0001’ – Wait 500μS
                                        * ‘0010’ – Wait 750μS
                                        * ........
                                        * ‘1111’ – Wait 4000μS */
        };
        uint8_t byte;
    };
} nrf24_setup_retr_t;

static nrf_ll_dscr_t g_nrf_dscr;

static uint8_t nrf24_ll_read_reg(uint8_t reg)
{
    NRF24_CSN_LOW();
    uint8_t data[] = {reg, NOP};
    spi_master_send_block(data, data, sizeof(data));
    NRF24_CSN_HIGH();

    return data[1];
}

static void nrf24_ll_write_reg(uint8_t reg, uint8_t value)
{
    NRF24_CSN_LOW();
    uint8_t data[] = {reg | W_REGISTER, value};
    spi_master_send_block(data, data, sizeof(data));
    NRF24_CSN_HIGH();
}

static void nrf24_ll_read_multi(uint8_t reg, uint8_t *p_data, uint8_t len)
{
    NRF24_CSN_LOW();
    spi_master_rw(reg | R_REGISTER);
    spi_master_send_block(p_data, p_data, len);
    NRF24_CSN_HIGH();
}

static void nrf24_ll_write_multi(uint8_t reg, uint8_t *p_data, uint8_t len)
{
    NRF24_CSN_LOW();
    spi_master_rw(reg | W_REGISTER);
    spi_master_send_block(p_data, NULL, len);
    NRF24_CSN_HIGH();
}

static void nrf24_ll_interrupt(nrf24_interrupt_t enable)
{
    nrf24_config_t config = {.byte = nrf24_ll_read_reg(CONFIG)};

    config.mask_max_rt = enable;
    config.mask_tx_ds  = enable;
    config.mask_rx_dr  = enable;

    nrf24_ll_write_reg(CONFIG, config.byte);
}

static void nrf24_ll_interrupt_flags_clear(void)
{
    nrf24_status_t status = {.max_rt = 1,
                             .tx_ds  = 1,
                             .rx_dr  = 1};

    nrf24_ll_write_reg(STATUS, status.byte);
}

NRF_IRQ_HANDLER()
{
    app_timer_reschedule(&g_nrf_dscr.timer, 0);
}

static uint32_t nrf24_ll_timer_cb(void *p_context)
{
    nrf24_status_t status = nrf24_ll_status();
    nrf24_ll_interrupt_flags_clear();
    switch (g_nrf_dscr.state)
    {
        case NRF_STATE_TX_TRANSMIT:
        {
            g_nrf_dscr.state = NRF_STATE_TX_READY;
            if (g_nrf_dscr.tx_callback != NULL)
            {
                g_nrf_dscr.tx_callback(status);
            }
            break;
        }
        case NRF_STATE_RX_READY:
        {
            if (g_nrf_dscr.rx_callback != NULL)
            {
                g_nrf_dscr.rx_callback(status);
            }
            break;
        }
        default:
            break;
    }
    return APP_TIMER_STOP;
}

void nrf24_ll_init(nrf24_dev_mode_t mode, 
                    uint8_t channel, 
                    nrf24_data_rate_t data_rate,
                    uint8_t *p_tx_addr, 
                    nrf24_addr_width_t addr_width,
                    uint8_t payload_size,
                    nrf_tx_complete_cb_t tx_callback,
                    nrf_rx_cb_t rx_callback)
{
    g_nrf_dscr.timer.cb = nrf24_ll_timer_cb;
    g_nrf_dscr.timer.p_context = NULL;

    g_nrf_dscr.tx_callback = tx_callback;
    g_nrf_dscr.rx_callback = rx_callback;

    CSN_INIT();
    NRF24_CSN_HIGH();

    CE_INIT();
    // NRF24_CE_HIGH();
    NRF24_CE_LOW();

    NRF_IRQ_INIT();
    DELAY_US(150000UL);

    nrf24_ll_shock_burst(NRF_SHOCKBURST_DISABLE);
    
    nrf24_ll_interrupt_flags_clear();
    nrf24_ll_interrupt(NRF_INTERRUP_ENABLE);
    
    nrf24_ll_dev_mode(mode);

    nrf24_ll_address_width_set(addr_width);

    nrf24_ll_tx_addr_set(p_tx_addr);
    nrf24_ll_open_rx_pipe(PIPE_P0, p_tx_addr, payload_size);

    nrf24_ll_channel_set(channel);

    nrf24_ll_data_rate_set(data_rate);

    nrf24_ll_tx_fifo_flash();
    nrf24_ll_rx_fifo_flash();

    nrf24_config_t config = {.byte = nrf24_ll_read_reg(CONFIG)};
    config.pwr_up = true;
    nrf24_ll_write_reg(CONFIG, config.byte);
    
    DELAY_US(150000UL);
    NRF_IRQ_ENABLE();
}

void nrf24_ll_shock_burst(nrf24_shock_burst_t state)
{
    nrf24_ll_write_reg(EN_AA, state);
    nrf24_ll_write_reg(SETUP_RETR, state);
}

void nrf24_ll_power_set(nrf24_power_t power)
{
    nrf24_setup_t setup = {.byte = nrf24_ll_read_reg(RF_SETUP)};

    setup.rf_pwr = power;
    nrf24_ll_write_reg(RF_SETUP, setup.byte);
}

void nrf24_ll_data_rate_set(nrf24_data_rate_t data_rate)
{
    nrf24_setup_t setup = {.byte = nrf24_ll_read_reg(RF_SETUP)};

    setup.rf_dr_low  = (data_rate & 2) >> 1;
    setup.rf_dr_high = data_rate & 1;
    nrf24_ll_write_reg(RF_SETUP, setup.byte);
}

void nrf24_ll_channel_set(uint8_t channel)
{
    nrf24_ll_write_reg(RF_CH, channel);
}

void nrf24_ll_address_width_set(nrf24_addr_width_t width)
{
    nrf24_setup_aw_t setup = {.aw = width};
    nrf24_ll_write_reg(SETUP_AW, setup.byte);
}

void nrf24_ll_scan(nrf24_scan_t scan_state)
{
    (scan_state == NRF_SCAN_ON) ? NRF24_CE_HIGH() : NRF24_CE_LOW();
}

error_t nrf24_ll_dev_mode(nrf24_dev_mode_t mode)
{
    if (g_nrf_dscr.state != NRF_STATE_TX_READY &&
        g_nrf_dscr.state != NRF_STATE_RX_READY)
    {
        return ERROR_INVALID_STATE;
    }

    nrf24_config_t config = {.byte = nrf24_ll_read_reg(CONFIG)};
    config.prim_rx = mode;
    nrf24_ll_write_reg(CONFIG, config.byte);

    if (mode == NRF_MODE_TX)
    {
        g_nrf_dscr.state = NRF_STATE_TX_READY;
    }
    else
    {
        g_nrf_dscr.state = NRF_STATE_RX_READY;
    }

    return ERROR_SUCCESS;
}

void nrf24_ll_crc(nrf24_crc_enable_t enable, nrf24_crc_scheme_t num_bytes)
{
    nrf24_config_t config = {.byte = nrf24_ll_read_reg(CONFIG)};

    config.en_crc = enable;
    config.crco   = num_bytes;

    nrf24_ll_write_reg(CONFIG, config.byte);
}

uint8_t nrf24_ll_rpd(void)
{
    return nrf24_ll_read_reg(CD);
}

void nrf24_ll_open_rx_pipe(nrf24_pipe_t pipe, uint8_t *p_rx_addr, uint8_t payload_len)
{
    uint8_t en_rxaddr = nrf24_ll_read_reg(EN_RXADDR);
    en_rxaddr |= (1 << pipe);
    nrf24_ll_write_reg(EN_RXADDR, en_rxaddr);

    uint8_t addr_size = nrf24_ll_read_reg(SETUP_AW) + 2;
    nrf24_ll_write_multi(RX_ADDR_P0 + pipe, p_rx_addr, addr_size);

    nrf24_ll_write_reg(RX_PW_P0 + pipe, payload_len);
}

void nrf24_ll_close_rx_pipe(nrf24_pipe_t pipe)
{
    uint8_t en_rxaddr = nrf24_ll_read_reg(EN_RXADDR);
    en_rxaddr &= !(1 << pipe);
    nrf24_ll_write_reg(EN_RXADDR, en_rxaddr);
}

void nrf24_ll_tx_addr_set(uint8_t *p_addr)
{
    nrf24_setup_aw_t setup = {.byte = nrf24_ll_read_reg(SETUP_AW) };
    nrf24_ll_write_multi(TX_ADDR, p_addr, setup.aw + 2);
}

void nrf24_ll_tx_setup_retr(uint8_t retr_cnt, nrf24_retr_delay_t delay)
{
    nrf24_setup_retr_t retr_setup = {.arc = retr_cnt,
                                     .ard = delay};
    nrf24_ll_write_reg(SETUP_RETR, retr_setup.byte);
}

error_t nrf24_ll_transmit(void)
{
    if (g_nrf_dscr.state != NRF_STATE_TX_READY)
    {
        return ERROR_INVALID_STATE;
    }

    g_nrf_dscr.state = NRF_STATE_TX_TRANSMIT;

    g_nrf_dscr.timer.p_context = NULL;
    app_timer_reschedule(&g_nrf_dscr.timer, NRF24_SEND_TIMEOUT_MS);

    NRF24_CE_LOW();
    NRF24_CE_HIGH();

    DELAY_US(10);
    NRF24_CE_LOW();
    return ERROR_SUCCESS;
}

nrf24_status_t nrf24_ll_status(void)
{
    NRF24_CSN_LOW();
    nrf24_status_t status = {.byte = spi_master_rw(NOP)};
    NRF24_CSN_HIGH();
    
    return status;
}

nrf24_fifo_status_t nrf24_ll_fifo_status(void)
{
    nrf24_fifo_status_t status = {.byte = nrf24_ll_read_reg(FIFO_STATUS)};

    return status;
}

error_t nrf24_ll_write_tx_payload(uint8_t *p_data, uint8_t size)
{
    if (p_data == NULL)
    {
        return ERROR_NULL_PTR;
    }
    if (size > PAYLOAD_MAX_SIZE)
    {
        return ERROR_DATA_LENGTH;
    }

    nrf24_ll_write_multi(W_TX_PAYLOAD, p_data, size);

    return ERROR_SUCCESS;
}

nrf24_status_t nrf24_ll_tx_fifo_flash(void)
{
    NRF24_CSN_LOW();
    nrf24_status_t status = {.byte = spi_master_rw(FLUSH_TX)};
    NRF24_CSN_HIGH();

    return status;
}

uint8_t nrf24_ll_read_rx_payload_size(void)
{
    return nrf24_ll_read_reg(R_RX_PL_WID);
}

void nrf24_ll_read_rx_fifo(uint8_t *p_data, uint8_t size)
{
    nrf24_ll_read_multi(R_RX_PAYLOAD, p_data, size);
}

nrf24_status_t nrf24_ll_rx_fifo_flash(void)
{
    NRF24_CSN_LOW();
    nrf24_status_t status = {.byte = spi_master_rw(FLUSH_RX)};
    NRF24_CSN_HIGH();

    return status;
}
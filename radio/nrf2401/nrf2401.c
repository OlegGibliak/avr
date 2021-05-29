/********************************************************************
*                         Standard headers                          *
********************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/********************************************************************
*                           Local headers                           *
********************************************************************/
#include "nrf2401.h"

/********************************************************************
*                       Function macro defines                      *
********************************************************************/
#define REG_SIZE                (1)

/********************************************************************
*                             Typedefs                              *
********************************************************************/
typedef struct
{
    bool           initialized;
    spi_transfer_t spi;
    uint8_t        rf_channel;
} nrf_ctx_t;

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
} config_t;

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
} fifo_info_t;

typedef struct
{
    uint8_t en_dyn_ack :1; /*< Enables the W_TX_PAYLOAD_NOACK command.*/
    uint8_t en_ack_pay :1; /*< Enables Payload with ACK.*/
    uint8_t en_dpl     :1; /*< Enables Dynamic Payload Length.*/
    uint8_t _rfu       :5; /*< Reserved.*/
} feature_t;

typedef struct
{                      /*< Enable dynamic payload length.*/
    uint8_t dpl_p0 :1;
    uint8_t dpl_p1 :1;
    uint8_t dpl_p2 :1;
    uint8_t dpl_p3 :1;
    uint8_t dpl_p4 :1;
    uint8_t dpl_p5 :1;
    uint8_t _rfu   :2;
} dynpl_t;

typedef struct
{
    uint8_t arc_cnt  :4; /*< Count retransmitted packets. The counter is reset when 
                          *  transmission of a new packet starts.*/
    uint8_t plos_cnt :4; /*< Count lost packets. The counter is overflow protected to 15,
                          *  and discontinues at max until reset. The counter is reset by writing to RF_CH.*/
} observe_tx_t;

/********************************************************************
*                  Static global data declarations                  *
********************************************************************/
static nrf_ctx_t g_ctx;

/********************************************************************
*                     Functions implementations                     *
********************************************************************/
void read(nrf_reg_t reg, uint8_t *data, uint8_t len)
{
    uint8_t tx_buff[len + REG_SIZE];
    tx_buff[0] = reg | NRF_CMD_R_REGISTER;;

    g_ctx.spi(tx_buff, data, len + REG_SIZE);
}

void write(nrf_reg_t reg, uint8_t *data, uint8_t len)
{
    uint8_t tx_buff[len + REG_SIZE];
    tx_buff[0] = reg | NRF_CMD_W_REGISTER;;

    for (uint8_t i = 0; i < len; i++)
    {
        tx_buff[i + REG_SIZE] = data[i];
    }
    
    g_ctx.spi(tx_buff, NULL, len + REG_SIZE);
}

/********************************************************************
*                                API                                *
********************************************************************/
nrf_error_t nrf_init(nrf_dscr_t *dscr, spi_transfer_t spi_transfer)
{
    if (g_ctx.initialized == true)
    {
        return NRF_E_ALREADY_INITIALIZED;
    }

    g_ctx.spi = spi_transfer;

    uint8_t setup_aw;
    read(NRF_REG_SETUP_AW, &setup_aw, sizeof(setup_aw));

    if (setup_aw < ADDR_SIZE_3B || setup_aw > ADDR_SIZE_5B)
    {
        return NRF_E_NOT_CONNECTED;
    }

    read(NRF_REG_RF_CH, &g_ctx.rf_channel, sizeof(g_ctx.rf_channel));
    g_ctx.initialized = true;
    return NRF_E_SUCCESS;
}

nrf_error_t nrf_mode(nrf_mode_t mode)
{
    uint8_t config;
    read(NRF_REG_CONFIG, &config, sizeof(config));
    config_t * p_config = (config_t *)&config;

    switch (mode)
    {
        case NRF_MODE_POWER_DOWN:
            p_config->pwr_up = 0;
            break;
        case NRF_MODE_TX:
            p_config->pwr_up  = 1;
            p_config->prim_rx = 0;
            break;
        case NRF_MODE_RX:
            p_config->pwr_up  = 1;
            p_config->prim_rx = 1;
            break;
        default:
            return NRF_E_INVALID_PARAM;
    }

    write(NRF_REG_CONFIG, &config, sizeof(config));
    
    return NRF_E_SUCCESS;
}

nrf_error_t nrf_crc(nrf_crc_t crc)
{
    uint8_t config;
    read(NRF_REG_CONFIG, &config, sizeof(config));
    config_t * p_config = (config_t *)&config;

    switch (crc)
    {
        case NRF_CRC_DISABLE:
            p_config->en_crc = 0;
            break;
        case NRF_CRC_1B:
            p_config->en_crc = 1;
            p_config->crco   = 0;
            break;
        case NRF_CRC_2B:
            p_config->en_crc = 1;
            p_config->crco   = 1;
            break;
        default:
            return NRF_E_INVALID_PARAM;
    }

    write(NRF_REG_CONFIG, &config, sizeof(config));
    
    return NRF_E_SUCCESS;
}

nrf_error_t nrf_interrupt(bool enable)
{
    uint8_t config;
    read(NRF_REG_CONFIG, &config, sizeof(config));
    config_t * p_config = (config_t *)&config;

    if (enable)
    {
        p_config->mask_max_rt = 0;
        p_config->mask_rx_dr  = 0;
        p_config->mask_tx_ds  = 0;  
    }
    else
    {
        p_config->mask_max_rt = 1;
        p_config->mask_rx_dr  = 1;
        p_config->mask_tx_ds  = 1;
    }

    write(NRF_REG_CONFIG, &config, sizeof(config));
    
    return NRF_E_SUCCESS;
}

nrf_error_t nrf_rx_pipe(nrf_pipe_t pipe, bool enable, bool ack)
{
    if (pipe > NRF_PIPE_5)
    {
        return NRF_E_INVALID_PARAM;
    }

    uint8_t en_rx;
    read(NRF_REG_EN_RX_ADDR, &en_rx, sizeof(en_rx));

    if (enable)
    {
        en_rx |= (1 << pipe);

        uint8_t en_aa;
        read(NRF_REG_EN_AA, &en_aa, sizeof(en_aa));
        
        if (ack)
        {
            en_aa |= (1 << pipe);
        }
        else
        {
            en_aa &= ~(1 << pipe);
        }
        write(NRF_REG_EN_AA, &en_aa, sizeof(en_aa));
    }
    else
    {
        en_rx &= ~(1 << pipe);
    }

    write(NRF_REG_EN_AA, &en_rx, sizeof(en_rx));
    
    return NRF_E_SUCCESS;
}

nrf_error_t nrf_addr_size(nrf_addr_size_t size)
{
    if (size == ADDR_SIZE_ILLEGAL)
    {
        return NRF_E_INVALID_PARAM;
    }

    uint8_t addr_size = size;
    write(NRF_REG_SETUP_AW, &addr_size, sizeof(addr_size));

    return NRF_E_SUCCESS;

}

nrf_error_t nrf_retr_setup(uint8_t retr, uint8_t delay)
{
    if (retr > 0x0F || delay > 0x0F)
    {
        return NRF_E_INVALID_PARAM;
    }

    uint8_t setip_retr = delay << 4 | retr;
    write(NRF_REG_SETUP_RETR, &setip_retr, sizeof(setip_retr));
    
    return NRF_E_SUCCESS;
}

nrf_error_t nrf_rf_setup(uint8_t channel, nrf_data_rate_t rate, nrf_output_power_t power)
{
    if (channel > 124)
    {
        return NRF_E_INVALID_PARAM;
    }

    write(NRF_REG_RF_CH, &channel, sizeof(channel));
    g_ctx.rf_channel = channel;
    
    uint8_t rf_setup = (power << 1) | (rate << 3);
    write(NRF_REG_RF_SETUP, &rf_setup, sizeof(rf_setup));

    return NRF_E_SUCCESS;
}

uint8_t nrf_status_get(void)
{
    uint8_t status;
    read(NRF_REG_STATUS, &status, sizeof(status));
}

void nrf_flag_clear(uint8_t flags)
{
    uint8_t status;
    read(NRF_REG_STATUS, &status, sizeof(status));

    status &= ~(flags);
    write(NRF_REG_STATUS, &status, sizeof(status));
}

nrf_error_t nrf_addr_set(nrf_pipe_t pipe, uint8_t *addr)
{
    uint8_t addr_size;
    read(NRF_REG_SETUP_AW, &addr_size, sizeof(addr_size));

    if (addr_size < ADDR_SIZE_3B || addr_size > ADDR_SIZE_5B)
    {
        return NRF_E_INTERNAL;
    }

    write(NRF_REG_RX_ADDR_P0 + pipe, addr, addr_size);

    return NRF_E_SUCCESS;
}

nrf_error_t nrf_payload_size(nrf_pipe_t pipe, uint8_t size)
{
    if (size > MAX_PAYLOAD_SIZE)
    {
        return NRF_E_INVALID_PARAM;
    }

    write(NRF_REG_PAYLOAD_SIZE_P0 + pipe, &size, size);

    return NRF_E_SUCCESS;
}

uint8_t nrf_lost_packets_cnt(void)
{
    uint8_t lost;
    read(NRF_REG_OBSERVE_TX, &lost, sizeof(lost));

    /* Clear lost packets counter. */ 
    write(NRF_REG_RF_CH, &g_ctx.rf_channel, sizeof(g_ctx.rf_channel));

    return lost >> 4;
}

uint8_t nrf_retr_cnt(void)
{
    uint8_t retr;
    read(NRF_REG_OBSERVE_TX, &retr, sizeof(retr));
    return retr & 0x0F;
}

uint8_t nrf_fifo_status(void)
{
    uint8_t status;
    read(NRF_REG_FIFO_STATUS, &status, sizeof(status));
    return status;
}

void nnrf_fifo_flush_rx(void)
{
    uint8_t cmd = NRF_CMD_FLUSH_TX;
    g_ctx.spi(&cmd, NULL, sizeof(cmd));
}

void nrf_fifo_flush_tx(void)
{
    uint8_t cmd = NRF_CMD_FLUSH_RX;
    g_ctx.spi(&cmd, NULL, sizeof(cmd));
}

nrf_error_t nrf_dynamic_payload(nrf_pipe_t pipe, bool enable)
{
    if (pipe >= NRF_PIPE_TX)
    {
        return NRF_E_INVALID_PARAM;
    }

    uint8_t dp;
    read(NRF_REG_DYNPD, &dp, sizeof(dp));

    if (enable)
    {
        dp |= (1 << pipe);
    }
    else
    {
        dp &= ~(1 << pipe);
    }

    write(NRF_REG_DYNPD, &dp, sizeof(dp));

    return NRF_E_SUCCESS;
}

void nrf_feature(bool dpl, bool ack_pay, bool dyn_ack)
{
    uint8_t feature;
    read(NRF_REG_FEATURE, &feature, sizeof(feature));

    feature_t *p_feature = (feature_t *)&feature;

    if (dpl)
    {
        p_feature->en_dpl = 1;
    }

    if (ack_pay)
    {
        p_feature->en_ack_pay = 1;
    }

    if (dyn_ack)
    {
        p_feature->en_dyn_ack = 1;
    }

    write(NRF_REG_FEATURE, &feature, sizeof(feature));
}

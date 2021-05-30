#include <stdbool.h>

#include "radio.h"
#include "nrf2401.h"
#include "logger.h"
/********************************************************************
*                       Function macro defines                      *
********************************************************************/
#if 0
#define bail_required(_cond_) if ((_cond_) != NRF_E_SUCCESS) goto bail
#else
#define bail_required(_cond_)                                           \
    if (_cond_)                                                         \
    {                                                                   \
        __LOG(LOG_LEVEL_DEBUG, "%s[%u]:\r\n", __func__, __LINE__);    \
        goto bail;                                                      \
    }

#endif

#define RADIO_DATA_RATE NRF_DATA_RATE_2M
#define RADIO_POWER     NRF_OUTPUT_POWER_0DB
/********************************************************************
*                             Typedefs                              *
********************************************************************/
typedef enum
{
    RADIO_IRQ_NOT_USED,
    RADIO_IRQ_IDLE,
    RADIO_IRQ_ASSERTED,
} irg_state_t;

typedef struct
{
    ce_high_t    nrf_ce_high;
    ce_low_t     nrf_ce_low;
    delay_t      delay;
    radio_mode_t mode;
    irg_state_t  irq_state;
} radio_ctx_t;

/********************************************************************
*                  Static global data declarations                  *
********************************************************************/
static radio_ctx_t g_ctx;
/********************************************************************
*                     Functions implementations                     *
********************************************************************/

static void radio_state_pkt_sent_handle(radio_pkt_sent_t *pkt_sent)
{
    pkt_sent->retr_cnt = nrf_retr_cnt();
    nrf_flag_clear(NRF_FLAG_TX_DS);
}

static void radio_state_pkt_recv_handle(radio_pkt_received_t *pkt_recv)
{
    pkt_recv->cnt = 0;
    for (uint8_t i = 0; i < RADIO_FIFO_SIZE_MAX; ++i)
    {
        nrf_pipe_t pipe;
        if (nrf_fifo_pop(pkt_recv->pkt[i].payload, &pkt_recv->pkt[i].len, &pipe) != NRF_E_SUCCESS)
        {
            break;
        }

        nrf_addr_get(NRF_PIPE_1, pkt_recv->pkt[i].addr);
        if (pipe != NRF_PIPE_1)
        {
            nrf_addr_get(pipe, &pkt_recv->pkt[i].addr[4]);
        }

        pkt_recv->cnt++;
    }
    nrf_flag_clear(NRF_FLAG_RX_DR);
}

static void radio_state_pkt_lost_handle(radio_pkt_lost_t *lost)
{
    lost->lost_pkts = nrf_lost_packets_cnt();
    nrf_flag_clear(NRF_FLAG_MAX_RT);
}

static void radio_state_fifo_full_handle(void)
{
    nrf_fifo_flush_tx();
}

/********************************************************************
*                                API                                *
********************************************************************/
radio_error_t radio_init(spi_tx_rx_t spi, ce_high_t ce_high, ce_low_t ce_low, delay_t delay)
{
    if (spi == NULL || ce_high == NULL || ce_low == NULL)
    {
        return RADIO_E_INVALID_PARAM;
    }

    g_ctx.nrf_ce_high = ce_high;
    g_ctx.nrf_ce_low  = ce_low;
    g_ctx.delay       = delay;

    nrf_error_t sts = nrf_init(spi);
    if (sts != NRF_E_SUCCESS)
    {
        __LOG(LOG_LEVEL_DEBUG, "nrf_init [%u]:\r\n", sts);
        return sts;
    }
    return RADIO_E_SUCCESS;
}

radio_error_t radio_setup(radio_mode_t mode, uint8_t channel)
{
    nrf_fifo_flush_rx();
    nrf_fifo_flush_tx();
    nrf_flag_clear(NRF_FLAG_RX_DR | NRF_FLAG_TX_DS | NRF_FLAG_MAX_RT);

    bail_required(nrf_interrupt(false));
    g_ctx.irq_state = RADIO_IRQ_NOT_USED;
    bail_required(nrf_addr_size(ADDR_SIZE_5B));

    switch (mode)
    {
        case RADIO_MODE_SHOCKBURST:
        {
            bail_required(nrf_retr_setup(15, 10));
            break;
        }
        case RADIO_MODE_ADVERTISER:
        {
            bail_required(nrf_retr_setup(0, 0));
            break;
        }
        default:
            return RADIO_E_INVALID_PARAM;
    }

    g_ctx.mode = mode;

    bail_required(nrf_rf_setup(channel, RADIO_DATA_RATE, RADIO_POWER));
    nrf_feature(true, false, false);

    return RADIO_E_SUCCESS;

bail:
    return RADIO_E_INTERNAL;
}


radio_error_t radio_rx_pipe_open(radio_pipe_t pipe, uint8_t *pipes_addr)
{
    nrf_pipe_t nrf_pipe;
    switch (pipe)
    {
        case RADIO_PIPE_1: nrf_pipe = NRF_PIPE_1; break;
        case RADIO_PIPE_2: nrf_pipe = NRF_PIPE_2; break;
        case RADIO_PIPE_3: nrf_pipe = NRF_PIPE_3; break;
        case RADIO_PIPE_4: nrf_pipe = NRF_PIPE_4; break;
        case RADIO_PIPE_5: nrf_pipe = NRF_PIPE_5; break;
    
        default:
            return RADIO_E_INVALID_PARAM;
    }

    bool auto_ack = (g_ctx.mode == RADIO_MODE_SHOCKBURST) ? true : false;

    bail_required(nrf_addr_set(nrf_pipe, pipes_addr));
    bail_required(nrf_rx_pipe(nrf_pipe, true, auto_ack));
    bail_required(nrf_dynamic_payload(nrf_pipe, true));

    return RADIO_E_SUCCESS;

bail:
    return RADIO_E_INTERNAL;
}

radio_error_t radio_rx_pipe_close(radio_pipe_t pipe)
{
    nrf_pipe_t nrf_pipe;
    switch (pipe)
    {
        case RADIO_PIPE_1: nrf_pipe = NRF_PIPE_1; break;
        case RADIO_PIPE_2: nrf_pipe = NRF_PIPE_2; break;
        case RADIO_PIPE_3: nrf_pipe = NRF_PIPE_3; break;
        case RADIO_PIPE_4: nrf_pipe = NRF_PIPE_4; break;
        case RADIO_PIPE_5: nrf_pipe = NRF_PIPE_5; break;
    
        default:
            return RADIO_E_INVALID_PARAM;
    }

    bail_required(nrf_rx_pipe(nrf_pipe, false, false));
    
    return RADIO_E_SUCCESS;

bail:
    return RADIO_E_INTERNAL;
}

// radio_error_t radio_rx_addr(uint8_t *base_addr, uint8_t *pipes_addr)
// {
//     uint8_t addr_p1[RADIO_ADDR_SIZE];
//     for (uint8_t i = 0; i < RADIO_ADDR_BASE; i++)
//     {
//         addr_p1[i] = base_addr[i];
//     }
 
//     addr_p1[RADIO_ADDR_BASE] = pipes_addr[0];

//     bool auto_ack = (g_ctx.mode == RADIO_MODE_SHOCKBURST) ? true : false;

//     for (uint8_t i = 0; i < RADIO_MULTIRECEIVERS; i++)
//     {
//         uint8_t *p_addr = (i == 0) ? addr_p1 : &pipes_addr[i];

//         if (pipes_addr[i] != 0)
//         {
//             bail_required(nrf_addr_set(NRF_PIPE_1, p_addr));
//             bail_required(nrf_rx_pipe(NRF_PIPE_1 + i, true, auto_ack));

//             bail_required(nrf_dynamic_payload(NRF_PIPE_1 + i, true));
//         }
//     }
    
//     return RADIO_E_SUCCESS;

// bail:
//     return RADIO_E_INTERNAL;
// }

radio_error_t radio_tx_addr(uint8_t *addr)
{
    /* We have not to change tx address if we have same data in tx fifo,
     * so we need to flush it.*/
    nrf_fifo_flush_tx();

    bail_required(nrf_addr_set(NRF_PIPE_TX, addr));

    // if (g_ctx.mode == RADIO_MODE_SHOCKBURST)
    {
        bail_required(nrf_addr_set(NRF_PIPE_0, addr));
        // bail_required(nrf_rx_pipe(NRF_PIPE_0, true, true));
    }
    // else
    {
        // bail_required(nrf_rx_pipe(NRF_PIPE_0, false, false));
    }

    return RADIO_E_SUCCESS;

bail:
    return RADIO_E_INTERNAL;
}

radio_error_t radio_receive(void)
{
    if (nrf_mode(NRF_MODE_RX) != NRF_E_SUCCESS)
        return RADIO_E_INTERNAL;
    
    g_ctx.nrf_ce_high();
    return RADIO_E_SUCCESS;
}

radio_error_t radio_send(uint8_t *pkt, uint8_t pkt_len)
{
    g_ctx.nrf_ce_low();
    bail_required(nrf_mode(NRF_MODE_TX));

    bail_required(nrf_fifo_push(pkt, pkt_len));

    g_ctx.nrf_ce_high();
    g_ctx.delay(10);
    g_ctx.nrf_ce_low();

    return RADIO_E_SUCCESS;

bail:
    return RADIO_E_INTERNAL;
}

void radio_irq_handle(void)
{
    if (g_ctx.irq_state == RADIO_IRQ_IDLE)
        g_ctx.irq_state = RADIO_IRQ_ASSERTED;
}

void radio_proccess(radio_proccess_t *proc)
{
    if (g_ctx.irq_state == RADIO_IRQ_ASSERTED ||
        g_ctx.irq_state == RADIO_IRQ_NOT_USED)
    {
        uint8_t status = nrf_status_get();
        if (status & NRF_FLAG_RX_DR)
        {
            proc->state = RADIO_STATE_PKT_RECEIVED;
            radio_state_pkt_recv_handle(&proc->recv);
        }
        else if (status & NRF_FLAG_TX_DS)
        {
            __LOG(LOG_LEVEL_DEBUG, "%s[%u]: status %02X\r\n", __func__, __LINE__, status);
            proc->state = RADIO_STATE_PKT_SENT;
            radio_state_pkt_sent_handle(&proc->sent);
        }
        else if (status & NRF_FLAG_MAX_RT)
        {
             __LOG(LOG_LEVEL_DEBUG, "%s[%u]: status %02X\r\n", __func__, __LINE__, status);
            proc->state = RADIO_STATE_PKT_LOST;
            radio_state_pkt_lost_handle(&proc->lost);
        }
        else if (status & NRF_FLAG_TX_FULL)
        {
            proc->state = RADIO_STATE_FIFO_FULL;
            radio_state_fifo_full_handle();
        }
        else
        {
            proc->state = RADIO_STATE_IDLE;
        }

        g_ctx.irq_state = (g_ctx.irq_state == RADIO_IRQ_ASSERTED) ?
            RADIO_IRQ_IDLE : RADIO_IRQ_NOT_USED;
    }
}

void radio_redails(radio_details_t *details)
{
    details->config  = nrf_read_reg(NRF_REG_CONFIG);
    details->en_aa   = nrf_read_reg(NRF_REG_EN_AA);
    details->en_rx   = nrf_read_reg(NRF_REG_EN_RX_ADDR);
    details->aw      = nrf_read_reg(NRF_REG_SETUP_AW);
    details->ret     = nrf_read_reg(NRF_REG_SETUP_RETR);
    details->ch      = nrf_read_reg(NRF_REG_RF_CH);
    details->rf_setup= nrf_read_reg(NRF_REG_RF_SETUP);
    details->status  = nrf_read_reg(NRF_REG_STATUS);

    nrf_addr_get(NRF_PIPE_0, details->rx_addr_p0);
    nrf_addr_get(NRF_PIPE_1, details->rx_addr_p1);
    nrf_addr_get(NRF_PIPE_2, details->rx_addr_p2);
    nrf_addr_get(NRF_PIPE_3, details->rx_addr_p3);
    nrf_addr_get(NRF_PIPE_4, details->rx_addr_p4);
    nrf_addr_get(NRF_PIPE_5, details->rx_addr_p5);
    nrf_addr_get(NRF_PIPE_TX, details->rx_addr_tx);

    details->fifo    = nrf_read_reg(NRF_REG_FIFO_STATUS);
    details->dynpl   = nrf_read_reg(NRF_REG_DYNPD);
    details->feature = nrf_read_reg(NRF_REG_FEATURE);
    details->observe_tx = nrf_read_reg(NRF_REG_OBSERVE_TX);
}
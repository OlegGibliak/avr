/********************************************************************
*                         Standard headers                          *
********************************************************************/
#include <stdio.h>
#include <stdint.h>

/********************************************************************
*                           Local headers                           *
********************************************************************/
#include "radio.h"
#include "nrf24.h"
#include "spi.h"

/********************************************************************
*                       Function macro defines                      *
********************************************************************/

#define CE_PIN      (1)
#define CE_DDR      (DDRB)
#define CE_PORT     (PORTB)

/********************************************************************
*                             Typedefs                              *
********************************************************************/

/********************************************************************
*                  Static global data declarations                  *
********************************************************************/

/********************************************************************
*                     Functions implementations                     *
********************************************************************/
static void spi_rw(uint8_t *tx, uint8_t *rx, uint8_t length)
{
    spi_select();
    spi_master_send_block(tx, rx, length);
    spi_unselect();
}

static void ce_select(void)
{
    CE_PORT &= ~(1 << CE_PIN);
    PORTB ^= (1 << 5);
}

static void ce_unselect(void)
{
    CE_PORT |= (1 << CE_PIN);
    PORTB ^= (1 << 5);
}

static void ce_init(void)
{
    CE_PORT |= (1 << CE_PIN);
    CE_DDR  |= (1 << CE_PIN);
}

static void tx_complete(void)
{
    ce_unselect();
    nrf_mode_set(NRF_MODE_PRX);
}

/********************************************************************
*                                API                                *
********************************************************************/
#include "logger.h"
#include <util/delay.h>
static void print_detail(void)
{
    uint8_t addr[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x1};
    uint8_t addr_size = sizeof(addr);

    nrf_addr_get(NRF_RX_P0, addr, &addr_size);
    __LOG_XB(LOG_LEVEL_DEBUG, "addr P0:", addr, addr_size);

    nrf_addr_get(NRF_RX_P1, addr, &addr_size);
    __LOG_XB(LOG_LEVEL_DEBUG, "addr P1:", addr, addr_size);

    nrf_addr_get(NRF_PIPE_TX, addr, &addr_size);
    __LOG_XB(LOG_LEVEL_DEBUG, "addr TX:", addr, addr_size);
}

radio_error_t radio_init(void)
{
    ce_init();
    spi_master_init();
    nrf_init(spi_rw);

    if(nrf_is_chip_connected() == NRF_E_NOT_CONNECTED)
    {
        return RADIO_E_NOT_CONNECTED;
    }

    if (nrf_flush_tx_fifo() != NRF_E_SUCCESS)
    {
        __LOG(LOG_LEVEL_DEBUG, "Flush tx fifo fail\r\n");
    }
    
    if (nrf_flush_tx_fifo() != NRF_E_SUCCESS)
    {
        __LOG(LOG_LEVEL_DEBUG, "Flush rx fifo fail\r\n");
    }

    nrf_status_clear(NRF_STATUS_MAX_RT | NRF_STATUS_RX_DR | NRF_STATUS_TX_DS);

    nrf_setup_t config = {.addr_size = NRF_ADDR_SIZE_5B,
                          .retr      = NRF_AUTO_RETR_10,
                          .delay     = 0x0F,
                          .channel   = 40,
                          .crc_mode  = NRF_CRC_1B};
    
    nrf_setup(&config);

    print_detail();

    return RADIO_E_SUCCESS;
}

radio_error_t radio_pkt_send(uint8_t *addr, uint8_t *pkt, uint8_t len)
{
    nrf_error_t error = nrf_fifo_push(pkt, len);
    if (error != NRF_E_SUCCESS)
    {
        return RADIO_E_NO_MEM;
    }

    if (nrf_pipe_open(NRF_PIPE_TX, addr) != NRF_E_SUCCESS)
    {
        return RADIO_E_INTERNAL;
    }

    if (nrf_pipe_open(NRF_RX_P0, addr) != NRF_E_SUCCESS)
    {
        return RADIO_E_INTERNAL;
    }

    nrf_mode_set(NRF_MODE_PTX);
    ce_select();
    _delay_ms(100);
    ce_unselect();
    return RADIO_E_SUCCESS;
}

radio_error_t radio_listen(uint8_t *addr)
{
    if (nrf_pipe_open(NRF_RX_P1, addr) != NRF_E_SUCCESS)
    {
        return RADIO_E_INTERNAL;
    }

    nrf_mode_set(NRF_MODE_PRX);
    ce_unselect();
    __LOG(LOG_LEVEL_DEBUG, "Start listerning...\r\n");
    return RADIO_E_SUCCESS;
}

void radio_proccess(void)
{
    nrf_status_t status = nrf_status_get();
    if (status & NRF_STATUS_TX_DS)
    {
        __LOG(LOG_LEVEL_DEBUG, "Packet sent\r\n");
        tx_complete();
        nrf_status_clear(NRF_STATUS_TX_DS);
    }
    else if (status & NRF_STATUS_RX_DR)
    {
        uint8_t pkt[MAX_PAYLOAD_SIZE];
        uint8_t pkt_size = 0;
        nrf_pipe_t pipe;
        nrf_error_t error = nrf_fifo_pop(pkt, &pkt_size, &pipe);
        if (error != NRF_E_SUCCESS)
        {
            __LOG(LOG_LEVEL_DEBUG, "Fifo pop fail %u\r\n", error);
        }
        else
        {
            pkt[pkt_size] = '\0';
            __LOG(LOG_LEVEL_DEBUG, "%s\r\n", (char *)pkt);
        }
        nrf_status_clear(NRF_STATUS_RX_DR);
    }
    else if (status & NRF_STATUS_MAX_RT)
    {
        __LOG(LOG_LEVEL_DEBUG, "Max retransmit. Flush fifo\r\n");
        tx_complete();
        nrf_flush_tx_fifo();
        nrf_status_clear(NRF_STATUS_MAX_RT);
    }
    else if (status)
    {
        __LOG(LOG_LEVEL_DEBUG, "Unknown nrf status %02X\r\n", status);
    }
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"
#include "logger.h"
#include "radio.h"
#include "spi.h"
#include "app_timer.h"

#define CE_PIN      (1)
#define CE_DDR      (DDRB)
#define CE_PORT     (PORTB)

static uint32_t timer_callback(void *p_context);
static uint32_t timer_send_cb(void *p_context);
static void     print_details(void);

static app_timer_t timer_details = {.cb = timer_callback};
static app_timer_t timer_send = {.cb = timer_send_cb};
static radio_proccess_t proc;

static uint32_t timer_callback(void *p_context)
{
    print_details();
    return 10 * 1000;
}

static uint32_t timer_send_cb(void *p_context)
{
    // char pkt[] = "Arduino pkt";
    // uint8_t addr[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x2};
    // radio_tx_addr(addr);

    // radio_error_t sts = radio_send((uint8_t *)pkt, sizeof(pkt));
    // if (sts != RADIO_E_SUCCESS)
    // {
    //     __LOG(LOG_LEVEL_DEBUG, "Radio timer_send fail %02X\r\n", sts);
    // }
    // radio_receive();
    return 5 * 1000;
}

void spi_transfer_block(uint8_t *tx_buff, uint8_t *rx_buff, uint8_t len)
{
    spi_select();
    spi_master_send_block(tx_buff, rx_buff, len);
    spi_unselect();
}

static void ce_select(void)
{
    CE_PORT &= ~(1 << CE_PIN);
}

static void ce_unselect(void)
{
    CE_PORT |= (1 << CE_PIN);
}

static void ce_init(void)
{
    CE_PORT |= (1 << CE_PIN);
    CE_DDR  |= (1 << CE_PIN);
}

static void delay(uint32_t ms)
{
    _delay_ms(150);
}

static void print_details(void)
{
    static radio_details_t details;
    radio_redails(&details);
    __LOG(LOG_LEVEL_DEBUG,  "====+ RADIO DETAILS +====\r\n"
                            "CONFIG   [%02X] |   EN_AA  [%02X]\r\n"
                            "EN_RX    [%02X] |   AW     [%02X]\r\n"
                            "RETR     [%02X] |   CH     [%02X]\r\n"
                            "RF_SETUP [%02X] |   STATUS [%02X]\r\n"

                            "FIFO     [%02X] |   DYNPL  [%02X]\r\n"
                            "FEATURE  [%02X] |   OBS TX [%02X]\r\n",
    details.config,   details.en_aa,
    details.en_rx,    details.aw,
    details.ret,      details.ch,
    details.rf_setup, details.status,
    
    details.fifo,     details.dynpl,
    details.feature,  details.observe_tx);

    // __LOG(LOG_LEVEL_DEBUG,  "FIFO     [%02X] |   DYNPL  [%02X]\r\n"
    //                         "FEATURE  [%02X] |   OBS TX [%02X]\r\n",
    // details.fifo,
    // details.dynpl,
    // details.feature,
    // details.observe_tx);

    __LOG(LOG_LEVEL_DEBUG,  "======+ ADDRESSES +======\r\n"
                            "ADDR P0 [%02X %02X %02X %02X %02X]\r\n"
                            "ADDR P1 [%02X %02X %02X %02X %02X]\r\n"
                            "ADDR P2 [%02X]\r\n"
                            "ADDR P3 [%02X]\r\n"
                            "ADDR P4 [%02X]\r\n"
                            "ADDR P5 [%02X]\r\n"
                            "ADDR TX [%02X %02X %02X %02X %02X]\r\n",
    details.rx_addr_p0[0], details.rx_addr_p0[1], details.rx_addr_p0[2], details.rx_addr_p0[3], details.rx_addr_p0[4],
    details.rx_addr_p1[0], details.rx_addr_p1[1], details.rx_addr_p1[2], details.rx_addr_p1[3], details.rx_addr_p1[4],
    details.rx_addr_p2[0],
    details.rx_addr_p3[0],
    details.rx_addr_p4[0],
    details.rx_addr_p5[0],
    details.rx_addr_tx[0], details.rx_addr_tx[1], details.rx_addr_tx[2], details.rx_addr_tx[3], details.rx_addr_tx[4]);
}

int main()
{
    // DDRB |= (1 << 5);

    // DDRB |= (1 << 4);
    // PORTB |= (1 << 1);
    // DDRB |= (1 << 1); //CE
    
    PORTB |= (1 << 2);
    DDRB |= (1 << 2); //CS

    PORTB |= (1 << 5);
    DDRB |= (1 << 5); //SCL

    PORTB |= (1 << 3);
    DDRB |= (1 << 3); //MOSI
    
    sei();
    serial_init();
    _delay_ms(1000);
    __LOG(LOG_LEVEL_DEBUG, "Start\r\n");
    
    app_timer_init();
    app_timer_add(&timer_details, 1000);
    app_timer_add(&timer_send, 2000);
    
    ce_init();
    spi_master_init();

    radio_error_t sts;
    while (1)
    {
        sts = radio_init(spi_transfer_block,
                         ce_unselect,
                         ce_select,
                         delay);
        _delay_ms(300);

        if (sts != RADIO_E_SUCCESS)
        {
            __LOG(LOG_LEVEL_DEBUG, "Radio init fail %02X\r\n", sts);
        }
        else
        {
            break;
        }
    }

    sts = radio_setup(RADIO_MODE_SHOCKBURST, 40);
    if (sts != RADIO_E_SUCCESS)
    {
        __LOG(LOG_LEVEL_DEBUG, "Radio setup fail %02X\r\n", sts);
    }

    uint8_t rx_addr[RADIO_ADDR_SIZE] = {0xDD, 0xAD, 0xBE, 0xEF, 0x01};
    // uint8_t pipes_addr[RADIO_MULTIRECEIVERS] = {0x01, 0, 0, 0, 0};
    // sts = radio_rx_addr(base_addr, pipes_addr);
    sts = radio_rx_pipe_open(RADIO_PIPE_1, rx_addr);
    if (sts != RADIO_E_SUCCESS)
    {
        __LOG(LOG_LEVEL_DEBUG, "Radio setup rx addr %02X RADIO_PIPE_1\r\n", sts);
    }

//!!!!!!!!!!!!!!!
    rx_addr[4] = 0xDE;
    sts = radio_rx_pipe_open(RADIO_PIPE_2, &rx_addr[4]);
    if (sts != RADIO_E_SUCCESS)
    {
        __LOG(LOG_LEVEL_DEBUG, "Radio setup rx addr %02X RADIO_PIPE_\r\n", sts);
    }

    sts = radio_receive();
    if (sts != RADIO_E_SUCCESS)
    {
        __LOG(LOG_LEVEL_DEBUG, "Radio receive %02X\r\n", sts);
    }

    for(;;)
    {
        radio_proccess(&proc);
        switch (proc.state)
        {
            case RADIO_STATE_IDLE:
            {
                break;
            }
            case RADIO_STATE_PKT_RECEIVED:
            {
                for (uint8_t i = 0; i < proc.recv.cnt; ++i)
                {
                    __LOG(LOG_LEVEL_DEBUG, "PKT_RECEIVED[%d/%d]: [%02X%02X%02X%02X%02X]\r\n",
                        i+1, proc.recv.cnt,
                        proc.recv.pkt[i].addr[0],
                        proc.recv.pkt[i].addr[1],
                        proc.recv.pkt[i].addr[2],
                        proc.recv.pkt[i].addr[3],
                        proc.recv.pkt[i].addr[4]);
                    __LOG(LOG_LEVEL_DEBUG, "DATA STR: %s\r\n", (char *) proc.recv.pkt[i].payload);
                    __LOG_XB(LOG_LEVEL_DEBUG, "DATA RAW: ", proc.recv.pkt[i].payload, proc.recv.pkt[i].len);
                }
                break;
            }
            case RADIO_STATE_PKT_SENT:
            {
                __LOG(LOG_LEVEL_DEBUG, "PKT_SENT: RETR %d\r\n", proc.sent.retr_cnt);
                break;
            }
            case RADIO_STATE_PKT_LOST:
            {
                __LOG(LOG_LEVEL_DEBUG, "PKT_LOST: CNT %d\r\n", proc.lost.lost_pkts);
                break;
            }
            case RADIO_STATE_FIFO_FULL:
            {
                __LOG(LOG_LEVEL_DEBUG, "FIFO_FULL: FLUSH TX\r\n");
                break;
            }
        }

        app_timer_process();
    }
}
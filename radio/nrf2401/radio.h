/********************************************************************
*                         Standard headers                          *
********************************************************************/
#ifndef RADIO_H__
#define RADIO_H__

#include <stdio.h>
#include <stdint.h>

/********************************************************************
*                           Local headers                           *
********************************************************************/
#define RADIO_ADDR_SIZE         (5)

#define RADIO_FIFO_DATA_MAX     (32)
#define RADIO_FIFO_SIZE_MAX     (3)

/********************************************************************
*                       Function macro defines                      *
********************************************************************/

/********************************************************************
*                             Typedefs                              *
********************************************************************/
typedef enum
{
    RADIO_E_SUCCESS,
    RADIO_E_INVALID_PARAM,
    RADIO_E_CHIP_FAIL,
    RADIO_E_INTERNAL,
} radio_error_t;

typedef enum
{
    RADIO_MODE_SHOCKBURST,
    RADIO_MODE_ADVERTISER
} radio_mode_t;

typedef enum
{
    RADIO_STATE_IDLE,
    RADIO_STATE_PKT_RECEIVED,
    RADIO_STATE_PKT_SENT,
    RADIO_STATE_PKT_LOST,
    RADIO_STATE_FIFO_FULL,
} radio_state_t;

typedef enum
{
    RADIO_PIPE_1,
    RADIO_PIPE_2,
    RADIO_PIPE_3,
    RADIO_PIPE_4,
    RADIO_PIPE_5,
} radio_pipe_t;

typedef struct
{
    uint8_t config;
    uint8_t en_aa;
    uint8_t en_rx;
    uint8_t aw;
    uint8_t ret;
    uint8_t ch;
    uint8_t rf_setup;
    uint8_t status;
    uint8_t rx_addr_p0[RADIO_ADDR_SIZE];
    uint8_t rx_addr_p1[RADIO_ADDR_SIZE];
    uint8_t rx_addr_p2[1];
    uint8_t rx_addr_p3[1];
    uint8_t rx_addr_p4[1];
    uint8_t rx_addr_p5[1];
    uint8_t rx_addr_tx[RADIO_ADDR_SIZE];
    uint8_t fifo; 
    uint8_t dynpl;
    uint8_t feature;
    uint8_t observe_tx;
} radio_details_t;

typedef struct
{
    uint8_t addr[RADIO_ADDR_SIZE];
    uint8_t len;
    uint8_t payload[RADIO_FIFO_DATA_MAX];
} radio_pkt_t;

typedef struct
{
    uint8_t     cnt;
    radio_pkt_t pkt[RADIO_FIFO_SIZE_MAX];
} radio_pkt_received_t;

typedef struct
{
    uint8_t retr_cnt;
} radio_pkt_sent_t;

typedef struct
{
    uint8_t lost_pkts;
} radio_pkt_lost_t;

typedef struct
{
    radio_state_t state;
    union
    {
        radio_pkt_received_t recv;
        radio_pkt_sent_t     sent;
        radio_pkt_lost_t     lost;
    };
} radio_proccess_t;

typedef void (*spi_tx_rx_t)(uint8_t *tx_buff, uint8_t *rx_buff, uint8_t len);
typedef void (*ce_high_t)(void);
typedef void (*ce_low_t)(void);
typedef void (*delay_t)(uint32_t ms);

/********************************************************************
*                                API                                *
********************************************************************/
radio_error_t radio_init(spi_tx_rx_t spi, ce_high_t ce_high, ce_low_t ce_low,  delay_t delay);
radio_error_t radio_setup(radio_mode_t mode, uint8_t channel);

/* Base address width 4 bytes.
   5 Unique addresses pipe1...pipe5, must be unique for all pipes
   or 0 if not used. */
// radio_error_t radio_rx_addr(uint8_t *base_addr, uint8_t *pipes_addr);
radio_error_t radio_rx_pipe_open(radio_pipe_t pipe, uint8_t *pipes_addr);
radio_error_t radio_rx_pipe_close(radio_pipe_t pipe);
radio_error_t radio_tx_addr(uint8_t *addr);
radio_error_t radio_receive(void);
radio_error_t radio_send(uint8_t *pkt, uint8_t pkt_len);

void radio_irq_handle(void);
void radio_proccess(radio_proccess_t *proc);

void radio_redails(radio_details_t *details);
#endif /* RADIO_H__ */
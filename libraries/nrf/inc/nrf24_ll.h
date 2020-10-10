#ifndef NRF24_LL_H__
#define NRF24_LL_H__

#include <stdbool.h>
#include "error.h"

#ifdef ARDUINO_BOARD
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#endif /* ARDUINO_BOARD */

#ifdef ARDUINO_BOARD
#define DELAY_US(_us)       _delay_us(_us)

#define CSN_PORT            (PORTD)
#define CSN_PIN             (1 << 6) /* D6 */
#define CSN_DDR             (DDRD)

#define CSN_INIT()          (CSN_DDR |= CSN_PIN)

#define NRF24_CSN_HIGH()   (CSN_PORT |= CSN_PIN)
#define NRF24_CSN_LOW()    (CSN_PORT &= ~CSN_PIN)

#define CE_PORT            (PORTD)
#define CE_PIN             (1 << 7) /* D7 */
#define CE_DDR             (DDRD)

#define CE_INIT()          (CE_DDR |= CE_PIN)

#define NRF24_CE_HIGH()    (CE_PORT |= CE_PIN)
#define NRF24_CE_LOW()     (CE_PORT &= ~(CE_PIN))

#define NRF_IRQ_PORT       (PORTD)
#define NRF_IRQ_PIN        (1 << 2) /* D2 */
#define NRF_IRQ_DDR        (DDRD)
#define NRF_IRQ_INIT()                      \
do                                          \
{                                           \
    /* Pin as input */                      \
    NRF_IRQ_DDR &= ~(NRF_IRQ_PIN);          \
    /* External interrupt INT0 */           \
    EICRA = (EICRA & 0xFC) | (1 << ISC01);  \
} while(0)

#define NRF_IRQ_HANDLER()                   \
ISR(INT0_vect)

#define NRF_IRQ_ENABLE()    (EIMSK |= (1 << INT0))
#define NRF_IRQ_DISABLE()   (EIMSK &= ~(1 << INT0))
#endif /* ARDUINO_BOARD */

/* nRF24 Registers. */
#define CONFIG      0x00
#define EN_AA       0x01
#define EN_RXADDR   0x02
#define SETUP_AW    0x03
#define SETUP_RETR  0x04
#define RF_CH       0x05
#define RF_SETUP    0x06
#define STATUS      0x07
#define OBSERVE_TX  0x08
#define CD          0x09
#define RX_ADDR_P0  0x0A
#define RX_ADDR_P1  0x0B
#define RX_ADDR_P2  0x0C
#define RX_ADDR_P3  0x0D
#define RX_ADDR_P4  0x0E
#define RX_ADDR_P5  0x0F
#define TX_ADDR     0x10
#define RX_PW_P0    0x11
#define RX_PW_P1    0x12
#define RX_PW_P2    0x13
#define RX_PW_P3    0x14
#define RX_PW_P4    0x15
#define RX_PW_P5    0x16
#define FIFO_STATUS 0x17
#define DYNPD       0x1C
#define FEATURE     0x1D

/* nRF24 commands. */
#define R_REGISTER          0x00
#define W_REGISTER          0x20
#define R_RX_PAYLOAD        0x61
#define W_TX_PAYLOAD        0xA0
#define FLUSH_TX            0xE1
#define FLUSH_RX            0xE2
#define REUSE_TX_PL         0xE3
#define ACTIVATE            0x50
#define R_RX_PL_WID         0x60
#define W_ACK_PAYLOAD       0xA8
#define W_TX_PAYLOAD_NOACK  0x58
#define NOP                 0xFF

#define NRF24_DATA_MAX_SIZE 32

typedef struct __attribute((__packed__))
{
    union
    {
        struct __attribute((__packed__))
        {
            uint8_t tx_full : 1;    /* TX FIFO full flag. (1: TX FIFO full. 0: Available locations in TX FIFO). */
            uint8_t rx_p_no : 3;    /* Data pipe number for the payload available for reading from RX_FIFO.
                                     * (000-101: Data Pipe Number. 110: Not Used. 111: RX FIFO Empty). */
            uint8_t max_rt  : 1;    /* Maximum number of TX retransmits interrupt. Write 1 to clear bit. */
            uint8_t tx_ds   : 1;    /* Data Sent TX FIFO interrupt. */
            uint8_t rx_dr   : 1;    /* Data Ready RX FIFO interrupt. Write 1 to clear bit. */
            uint8_t _rfu    : 1;    /* Reserved for future. */
        };
        uint8_t byte;
    };
} nrf24_status_t;

typedef struct __attribute((__packed__))
{
    union
    {
        struct __attribute((__packed__))
        {
            uint8_t rx_empty : 1;   /* RX FIFO empty flag. 1: RX FIFO empty. 0: Data in RX FIFO. */
            uint8_t rx_full  : 1;   /* RX FIFO full flag. 1: RX FIFO full. 0: Available locations in RX FIFO. */
            uint8_t _rfu     : 2;   /* Reserved for future. */
            uint8_t tx_empty : 1;   /* TX FIFO empty flag. 1: TX FIFO empty. 0: Data in TX FIFO. */
            uint8_t tx_full  : 1;   /* TX FIFO full flag. 1: TX FIFO full. 0: Available loca-tions in TX FIFO. */
            uint8_t tx_reuse : 1;   /* Used for a PTX devicePulse the rfce high for at least 10μs to Reuse last transmitted payload. */
            uint8_t _rfu2    : 1;    /* Reserved for future. */
        };
        uint8_t byte;
    };
} nrf24_fifo_status_t;

typedef enum
{
    PIPE_P0,
    PIPE_P1,
    PIPE_P2,
    PIPE_P3,
    PIPE_P4,
    PIPE_P5,
} nrf24_pipe_t;

typedef enum
{
    NRF_CRC_DISABLE,
    NRF_CRC_ENABLE
} nrf24_crc_enable_t;

typedef enum
{
    NRF_CRC_1_BYTE,
    NRF_CRC_2_BYTE
} nrf24_crc_scheme_t;

typedef enum
{
    NRF_POWER_18_DB,
    NRF_POWER_12_DB,
    NRF_POWER_6_DB,
    NRF_POWER_0_DB
} nrf24_power_t;

typedef enum
{
    NRF_DATA_RATE_1_MBPS,
    NRF_DATA_RATE_2_MBPS,
    NRF_DATA_RATE_250_KBPS
} nrf24_data_rate_t;

typedef enum
{
    NRF_ADDR_WIDTH_3 = 1,
    NRF_ADDR_WIDTH_4,
    NRF_ADDR_WIDTH_5
} nrf24_addr_width_t;

typedef enum
{
    NRF_MODE_TX,
    NRF_MODE_RX
} nrf24_dev_mode_t;

typedef enum
{
    NRF_DELAY_250US,
    NRF_DELAY_500US,
    NRF_DELAY_750US,
    NRF_DELAY_1000US,
    NRF_DELAY_1250US,
    NRF_DELAY_1500US,
    NRF_DELAY_1750US,
    NRF_DELAY_2000US,
    NRF_DELAY_2250US,
    NRF_DELAY_2500US,
    NRF_DELAY_2750US,
    NRF_DELAY_3000US,
    NRF_DELAY_3250US,
    NRF_DELAY_3500US,
    NRF_DELAY_3750US,
    NRF_DELAY_4000US,
} nrf24_retr_delay_t;

typedef enum
{
    NRF_SHOCKBURST_DISABLE,
    NRF_SHOCKBURST_ENABLE
} nrf24_shock_burst_t;

typedef enum
{
    NRF_SCAN_OFF,
    NRF_SCAN_ON
} nrf24_scan_t;

typedef void (*nrf_tx_complete_cb_t)(nrf24_status_t status);
typedef void (*nrf_rx_cb_t)(nrf24_status_t status);

void nrf24_ll_init(nrf24_dev_mode_t mode, 
                    uint8_t channel, 
                    nrf24_data_rate_t data_rate,
                    uint8_t *p_tx_addr, 
                    nrf24_addr_width_t addr_width,
                    uint8_t payload_size,
                    nrf_tx_complete_cb_t tx_callback,
                    nrf_rx_cb_t rx_callback);

void    nrf24_ll_shock_burst(nrf24_shock_burst_t state);
void    nrf24_ll_power_set(nrf24_power_t power);
void    nrf24_ll_data_rate_set(nrf24_data_rate_t data_rate);
void    nrf24_ll_channel_set(uint8_t channel);
void    nrf24_ll_address_width_set(nrf24_addr_width_t width);
error_t nrf24_ll_dev_mode(nrf24_dev_mode_t mode);
void    nrf24_ll_crc(nrf24_crc_enable_t enable, nrf24_crc_scheme_t num_bytes);
uint8_t nrf24_ll_rpd(void);

void    nrf24_ll_open_rx_pipe(nrf24_pipe_t pipe, uint8_t *p_rx_addr, uint8_t payload_len);
void    nrf24_ll_close_rx_pipe(nrf24_pipe_t pipe);

void    nrf24_ll_tx_addr_set(uint8_t *p_addr);
void    nrf24_ll_tx_setup_retr(uint8_t retr_cnt, nrf24_retr_delay_t delay);

error_t nrf24_ll_transmit(void);
void    nrf24_ll_scan(nrf24_scan_t scan_state);

nrf24_status_t      nrf24_ll_status(void);
nrf24_fifo_status_t nrf24_ll_fifo_status(void);
error_t             nrf24_ll_write_tx_payload(uint8_t *p_data, uint8_t size);
nrf24_status_t      nrf24_ll_tx_fifo_flash(void);

uint8_t             nrf24_ll_read_rx_payload_size(void);
void                nrf24_ll_read_rx_fifo(uint8_t *p_data, uint8_t size);
nrf24_status_t      nrf24_ll_rx_fifo_flash(void);

// error_t nrf24_open_pipe(nrf24_pipe_t pipe, uint8_t *p_addr, bool auto_ack, uint8_t payload_len);

#endif /* NRF24_LL_H__ */
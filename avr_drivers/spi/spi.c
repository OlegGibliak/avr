#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "spi.h"
#include "logger.h"

#define NULL 0

#define DDR_SPI (DDRB)
#define DP_MOSI (3)
#define DP_MISO (4)
#define DP_SCK  (5)
#define DP_SS   (2)

typedef enum
{
    SPI_IDLE,
    SPI_MASTER_TX
} spi_state_t;

typedef struct
{
    uint8_t     *rx_buff;
    uint8_t     *tx_buff;
    uint8_t     len;
    spi_state_t state;
    spi_cb      cb;
} spi_descriptor_t;

static volatile spi_descriptor_t desc;

ISR(SPI_STC_vect)
{
    if (desc.rx_buff)
    {
        *desc.rx_buff++ = SPDR;
    }
    if (--desc.len)
    {
        SPDR = *desc.tx_buff++;
    }
    else
    {
        if (desc.cb)
        {
            desc.cb();
        }
        desc.state = SPI_IDLE;
        // PORTB |= (1 << DP_SS);
    }
}

void spi_master_init(void)
{
    /* Set MOSI and SCK output, all others input */
    DDR_SPI = (1 << DP_MOSI)|(1 << DP_SCK)|(1 << DP_SS);
    // PORTB |= (1 << DP_SS);
    /* Enable SPI, enable interrupt, Master, set clock rate fck/16 */
    SPCR = (1 << SPE)|(1 << SPIE)|(1 << MSTR)|(1 << SPR0);
}

uint8_t spi_master_rw(uint8_t value)
{
    uint8_t retval;
    while (desc.state != SPI_IDLE);
    desc.tx_buff = NULL;
    desc.rx_buff = &retval;
    desc.len     = 1;
    desc.state   = SPI_MASTER_TX;
    // PORTB &= ~(1 << DP_SS);
    SPDR = value;
    while (desc.state != SPI_IDLE);
    return retval;
}

void spi_master_send_block(uint8_t *tx_buff, uint8_t *rx_buff, uint8_t len)
{
    while (desc.state != SPI_IDLE);

    desc.tx_buff = tx_buff + 1;
    desc.rx_buff = rx_buff;
    desc.len     = len;
    desc.cb      = NULL;
    desc.state   = SPI_MASTER_TX;
    // PORTB &= ~(1 << DP_SS);
    SPDR = tx_buff[0];

    while(desc.state != SPI_IDLE);
}

void spi_master_send(uint8_t *tx_buff, uint8_t *rx_buff, uint8_t len, spi_cb cb)
{
    while (desc.state != SPI_IDLE);
    desc.tx_buff = tx_buff + 1;
    desc.rx_buff = rx_buff;
    desc.len     = len;
    desc.cb      = cb;
    desc.state   = SPI_MASTER_TX;
    SPDR = tx_buff[0];
}

uint8_t spi_is_ready(void)
{
    return desc.state == SPI_IDLE;
}

#ifndef _SPI_H
#define _SPI_H
#include <stdint.h>
#include <avr/io.h>

#define CSN_DDR         (DDRB)
#define CSN_PORT        (PORTB)
#define CSN_PIN         (2)

typedef void (*spi_cb)(void);

static inline void spi_select(void)
{
    CSN_PORT &= ~(1 << CSN_PIN);
}

static inline void spi_unselect(void)
{
    CSN_PORT |=  (1 << CSN_PIN);
}

void    spi_master_init(void);
uint8_t spi_master_rw(uint8_t value);
void    spi_master_send(uint8_t *tx_buff, uint8_t *rx_buff, uint8_t len, spi_cb cb);
void    spi_master_send_block(uint8_t *tx_buff, uint8_t *rx_buff, uint8_t len);
uint8_t spi_is_ready(void);
#endif /* _SPI_H */

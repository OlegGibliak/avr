#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"
#include "logger.h"
#include "spi.h"
#include "nrf24.h"

static void spi_rw(uint8_t *tx, uint8_t *rx, uint8_t length)
{
    spi_select();
    spi_master_send_block(tx, rx, length);
    spi_unselect();
}

int main()
{
    // DDRB |= (1 << 5);

    // DDRB |= (1 << 4);
    PORTB |= (1 << 1);
    DDRB |= (1 << 1); //CE
    
    PORTB |= (1 << 2);
    DDRB |= (1 << 2); //CS

    PORTB |= (1 << 5);
    DDRB |= (1 << 5); //SCL

    PORTB |= (1 << 3);
    DDRB |= (1 << 3); //MOSI
    


    // PORTB |= ( 1 << 1); //CE HI
    sei();

    serial_init();
    
    _delay_ms(1000);
    spi_master_init();
    nrf_init(spi_rw);
    

    
    for(;;)
    {
        __LOG(LOG_LEVEL_DEBUG, "nrf st %02X\r\n", nrf_status_get());
        PORTB ^= (1 << 5);
        _delay_ms(1000);
        // nrf_status_get();
    }
}
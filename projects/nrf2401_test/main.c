#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"
#include "logger.h"
#include "radio.h"

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
    


    // PORTB |= ( 1 << 1); //CE HI
    sei();

    serial_init();
    _delay_ms(1000);
    __LOG(LOG_LEVEL_DEBUG, "Start\r\n");
    
    radio_error_t sts = radio_init();
    if (sts != RADIO_E_SUCCESS)
    {
        __LOG(LOG_LEVEL_DEBUG, "Radio init fail %02X\r\n", sts);
    }
    uint8_t addr[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x1};

    // uint8_t buff[] = {1,2,3,4,5,6,7,8,9};
    // sts = radio_pkt_send(addr, buff, sizeof(buff));
    // if (sts != RADIO_E_SUCCESS)
    // {
    //     __LOG(LOG_LEVEL_DEBUG, "Radio send fail: %02X\r\n", sts);
    // }


    sts = radio_listen(addr);
    if (sts != RADIO_E_SUCCESS)
    {
        __LOG(LOG_LEVEL_DEBUG, "Radio listen fail: %02X\r\n", sts);
    }

    for(;;)
    {
        radio_proccess();
    }
}
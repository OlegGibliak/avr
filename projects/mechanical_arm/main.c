#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"
// #include "task_manager.h"
#include "logger.h"
#include "adc.h"

#define LEFT  (500UL)
#define CENTER (1000UL)
#define RIGHT  (2000UL)

static uint16_t adc_value;

static void servo(uint8_t pin, int dir)
{
    PORTB |= (1 << pin);
    _delay_us(dir);
    PORTB &= ~(1 << pin);
    _delay_us(dir);
    PORTB |= (1 << pin);
    _delay_us(dir);
    PORTB &= ~(1 << pin);
    _delay_us(dir);
}

void adc_cb(uint16_t value)
{
    adc_value = value;
}

int main()
{
    DDRB |= (1 << 5);

    DDRB |= (1 << 4);
    sei();

    serial_init();

    adc_init(ADC_VOLT_REF_AVCC, ACD_PRESCALER_128, adc_cb);
    adc_start(ADC_PIN_1);

    
    for(;;)
    {
        // if (adc_value)
        {
            __LOG(LOG_LEVEL_DEBUG, "%u\r\n", adc_value);
            adc_value = 0;
            adc_start(ADC_PIN_1);
        }
        static int flag;
        if (flag)
        {
            // for(uint8_t i = 10; i > 0; --i)
                servo(4, LEFT);
            flag = 0;
        }
        else
        {
            servo(4, RIGHT);
            flag = 1;
        }
        
        PORTB ^= (1 << 5);
        _delay_ms(1000);
    }
}
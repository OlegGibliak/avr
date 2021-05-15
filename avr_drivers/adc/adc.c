#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "adc.h"
#include "error.h"
#include "logger.h"

static adc_conversion_cb_t adc_cb;

ISR(ADC_vect)
{
    uint16_t adc   = ADCL;
    uint16_t value = (ADCH << 2) | (adc >> 6);

    if (adc_cb)
    {
        adc_cb(value);
    }
}

void adc_init(adc_volt_ref_t volt_ref, adc_prescaler_t prerscaler, adc_conversion_cb_t cb)
{
    adc_cb = cb;

    ADMUX = (volt_ref << 6) | (1 << ADLAR);
    ADCSRA = (1 << ADEN) | (1 << ADIE) | prerscaler;
}

void adc_set_triger(adc_trig_src_t trig_src)
{

}

void adc_start(adc_channel_t channel)
{
    // __LOG(LOG_LEVEL_DEBUG, "ADMUX %02X, ADCSRA %02X\r\n", ADMUX, ADCSRA);
    ADMUX  = (ADMUX & 0xF0) | channel;
    ADCSRA |= (1 << ADSC);
}

void adc_stop()
{

}
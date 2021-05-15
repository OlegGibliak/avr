#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer_timestamp.h"

static volatile uint32_t g_timestamp;

ISR(TIMER0_COMPA_vect)
{
    g_timestamp++;
}

void timer_timestamp_init(void)
{
    /* TCCR
     * CPU Freq 16Mhz
     * Need interval of 1Ms ==> 0,001/(1/16000000) = 16.000 ticks
     *
     * Prescaler 64 ==> resolution changes from 0,0000000625 to 0,000004
     * Need interval of 1Ms ==> 0,001/((1/16000000)*64) = 250 ticks
     */

    /* Initialize counter */
    TCNT0 = 0 ;

    /* Set Timer Interrupt Mask Register to
     * Clear Timer on Compare channel A for timer 0
     */
    TIMSK0 = (1 << OCIE0A);

    OCR0A = 250 - 1;

    /* Set prescaler to 64 ; (1 << CS01)|(1 << CS00)
     * Clear Timer on Compare (CTC) mode ; (1 << WGM02)
     */
    TCCR0A = 0 ;   
    TCCR0B |= (1 << WGM02)|(1 << CS01)|(1 << CS00) ;
}

uint32_t timer_timestamp_ms_get(void)
{
    return g_timestamp;
}
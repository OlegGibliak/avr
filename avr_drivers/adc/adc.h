#include <stdint.h>
#include "error.h"

typedef enum
{
    ADC_VOLT_REF_AREF0,     /*< AREF, internal VREF turned off.*/
    ADC_VOLT_REF_AVCC,      /*< AVCC with external capacitor at AREF pin.*/
    ADC_VOLT_REF_INTERNAL   /*< Internal 1.1V voltage reference with external capacitor at AREF pin.*/
} adc_volt_ref_t;

typedef enum
{
    ACD_PRESCALER_2 = 1,
    ACD_PRESCALER_4,
    ACD_PRESCALER_8,
    ACD_PRESCALER_16,
    ACD_PRESCALER_32,
    ACD_PRESCALER_64,
    ACD_PRESCALER_128
} adc_prescaler_t;

typedef enum
{
    ADC_TRIGGER_FREE,       /*< Free running mode.*/
    ADC_TRIGGER_AC,         /*< Analog comparator.*/
    ADC_TRIGGER_EI,         /*< External interrupt request.*/
    ADC_TRIGGER__TC0C,      /*< Timer/Counter0 compare match A.*/
    ADC_TRIGGER_TC0O,       /*< Timer/Counter0 overflow.*/
    ADC_TRIGGER_TC1C,       /*< Timer/Counter1 compare match B.*/
    ADC_TRIGGER_TC1O,       /*< Timer/Counter1 overflow.*/
    ADC_TRIGGER_TC1CE       /*< Timer/Counter1 capture event.*/
} adc_trig_src_t;

typedef enum
{
    ADC_PIN_0,                     
    ADC_PIN_1,
    ADC_PIN_2,
    ADC_PIN_3,
    ADC_PIN_4,
    ADC_PIN_5,
    ADC_PIN_6,
    ADC_PIN_7,
    ADV_TEMPERATURE
} adc_channel_t;

typedef void (*adc_conversion_cb_t)(uint16_t value);

void adc_init(adc_volt_ref_t volt_ref, adc_prescaler_t prerscaler, adc_conversion_cb_t cb);
void adc_set_triger(adc_trig_src_t trig_src);
void adc_start(adc_channel_t channel);
void adc_stop();


#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "assert.h"
#include "logger.h"
#include "led_dbg.h"

#if defined(CONFIG_ASSERT_ENABLE)
void assert_callback(uint16_t line, const char *func_name)
{
    printf("assert: %s: %u\r\n", func_name, line);
    cli();
    while(1)
    {
    	LED_DEBUG_TOOGLE();
    	_delay_ms(100);
    };
}
#endif /* CONFIG_ASSERT_ENABLE */
#ifndef LED_DEBUG_H__
#define LED_DEBUG_H__

#include <avr/io.h>
//to do #define PORT        (B)
#define LED_PORT            (PORTB)
#define LED_PIN             (5)
#define LED_DIR             (DDRB)
#define LED_PORTPIN         (PINB)

#if defined(MODULE_LED_DBG)
#define LED_DEBUG_INIT()                    \
do                                          \
{                                           \
    LED_DIR  |=  (1 << LED_PIN);            \
    LED_DEBUG_OFF();                        \
} while (0)

#define LED_DEBUG_ON()      (LED_PORT |=  (1 << LED_PIN))
#define LED_DEBUG_OFF()     (LED_PORT &= ~(1 << LED_PIN))
#define LED_DEBUG_TOOGLE()                  \
do                                          \
{                                           \
    if (LED_PORTPIN & (1 << 5))             \
        LED_DEBUG_OFF();                    \
    else                                    \
        LED_DEBUG_ON();                     \
} while (0)
#else /* MODULE_LED_DBG */
#define LED_DEBUG_INIT()
#define LED_DEBUG_ON()
#define LED_DEBUG_OFF()
#define LED_DEBUG_TOOGLE()
#endif /* MODULE_LED_DBG */
#endif /* LED_DEBUG_H__ */
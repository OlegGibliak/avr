#ifndef UTIL_H__
#define UTIL_H__

#include <avr/io.h>
#include <avr/interrupt.h>

#define INTERRUPT_ENABLE()            sei()
#define INTERRUPT_DISABLE()           cli()
#endif /* UTIL_H__ */
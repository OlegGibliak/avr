#ifndef TIMER_TIMESTAMP_H__
#define TIMER_TIMESTAMP_H__

#include <stdint.h>

void timer_timestamp_init(void);

uint32_t timer_timestamp_ms_get(void);

#endif /* TIMER_TIMESTAMP_H__ */
#ifndef TWI_H__
#define TWI_H__

#include "error.h"

typedef enum
{
    TWI_SCL_50KHZ   = 50,
    TWI_SCL_100KMHZ = 100,
    TWI_SCL_400KMHZ = 400
} twi_scl_t;

typedef void (*twi_tx_callback)(uint8_t status);
typedef void (*twi_rx_callback)(uint8_t status);

void    twi_init(twi_scl_t f_scl, twi_tx_callback tx_cb, twi_rx_callback rx_cb);
error_t twi_initialized(void);
error_t twi_send(uint8_t addr, const void* p_data, uint16_t len);
error_t twi_read(uint8_t addr, void* p_data, uint16_t len);
#endif /* TWI_H__ */

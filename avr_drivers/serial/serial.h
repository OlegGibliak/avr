#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdbool.h>
#include "error.h"

typedef void (*serial_tx_complete_cb)(void);
typedef void (*serial_rx_char_cb)(uint8_t ch);

void serial_init(void);
error_t serial_send_block(const uint8_t *data, uint8_t length);
error_t serial_send_no_block(const uint8_t *data, uint8_t length);
bool serial_ready(void);
void serial_set_tx_complete_cb(serial_tx_complete_cb cb);
void serial_set_rx_cb(serial_rx_char_cb cb);

#endif /* _SERIAL_H */

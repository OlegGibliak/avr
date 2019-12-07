#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "error.h"
#include "serial.h"
#include "logger.h"

#define SERIAL_FOSC                (F_CPU) // Clock Speed
#define SERIAL_BAUD                (9600)
#define SERIAL_MYUBRR              (SERIAL_FOSC/16/SERIAL_BAUD-1)

typedef enum
{
    SERIAL_STATE_IDLE,
    SERIAL_STATE_TX,
    SERIAL_STATE_RX
} serial_state_e;

typedef struct
{
    uint8_t               len;
    const uint8_t         *p_tx_buff;
    serial_state_e        state;
    serial_rx_char_cb     rx_cb;
    serial_tx_complete_cb tx_cb;
} serial_descriptor_t;

static volatile serial_descriptor_t m_desc;

ISR(USART_RX_vect)
{
    m_desc.state = SERIAL_STATE_RX;
    uint8_t ch = UDR0;
    if (m_desc.rx_cb)
    {
        m_desc.rx_cb(ch);
    }
    m_desc.state = SERIAL_STATE_IDLE;
}

ISR(USART_TX_vect)
{
    if (--m_desc.len)
    {
        UDR0 = *m_desc.p_tx_buff++;
    }
    else
    {
        if (m_desc.tx_cb)
        {
            m_desc.tx_cb();
        }
        m_desc.state = SERIAL_STATE_IDLE;
    }
}

/* Pablic API */
void serial_init(void)
{
    uint16_t ubrr = SERIAL_MYUBRR;
    /** Set baud rate */
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    /** Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0) | (1<<TXCIE0);
    /** Set frame format: 8data, 1stop bit */
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);
    sei();
    __LOG(LOG_LEVEL_INFO, "global interrupt enabled.\r\n");
    __LOG(LOG_LEVEL_INFO, "Serial initialization.\r\n");
}

error_t serial_send_block(const uint8_t *data, uint8_t length)
{
    if (!data)        return ERROR_NULL_PTR;
    if (length == 0)  return ERROR_DATA_LENGTH;

    while (!serial_ready());

    m_desc.len       = length;
    m_desc.p_tx_buff = data + 1;
    m_desc.state     = SERIAL_STATE_TX;
    UDR0 = data[0];

    while (!serial_ready()){};

    return ERROR_SUCCESS;
}

error_t serial_send_no_block(const uint8_t *data, uint8_t length)
{
    if (!data)        return ERROR_NULL_PTR;
    if (length == 0)  return ERROR_DATA_LENGTH;
    if (!serial_ready()) return ERROR_BUSY;

    m_desc.len       = length;
    m_desc.p_tx_buff = data + 1;
    m_desc.state     = SERIAL_STATE_TX;

    UDR0 = data[0];
    return ERROR_SUCCESS;
}

bool serial_ready(void)
{
    return m_desc.state == SERIAL_STATE_IDLE;
}

void serial_set_tx_complete_cb(serial_tx_complete_cb cb)
{
    m_desc.tx_cb = cb;
}

void serial_set_rx_cb(serial_rx_char_cb cb)
{
    m_desc.rx_cb = cb;
}

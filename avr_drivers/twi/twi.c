#include <avr/io.h>
#include <avr/interrupt.h>
#include "twi.h"
#include "logger.h"
#include "led_dbg.h"

#define TWI_READ              (1 << 0)
#define TWI_WRITE             (0 << 0)

#define TWI_START_CONDITION() (TWCR = (1 << TWINT) | (1 << TWIE) | (1 << TWEN) | (1 << TWSTA))
#define TWI_RESTART_CONDITION() TWI_START_CONDITION
#define TWI_STOP_CONDITION()  (TWCR = (1 << TWINT) | (1 << TWIE) |(1 << TWEN) | (1 << TWSTO))

typedef enum
{
    TWI_START_TRANSMITTED      = 0x08,
    TWI_R_START_TRANSMITTED    = 0x10,
    TWI_SLA_W_TRANSMITTED_ACK  = 0x18,
    TWI_SLA_W_TRANSMITTED_NACK = 0x20,
    TWI_DATA_TRANSMITTED_ACK   = 0x28,
    TWI_DATA_TRANSMITTED_NACK  = 0x30,
    TWI_ARBITRATION_LOST       = 0x38,
    TWI_SLA_R_TRANSMITTED_ACK  = 0x40,
    TWI_SLA_R_TRANSMITTED_NACK = 0x48,
    TWI_DATA_RECEIVED_ACK      = 0x50,
    TWI_DATA_RECEIVED_NACK     = 0x58
} twi_status_t;

typedef enum
{
    TWI_STATE_IDLE,
    TWI_STATE_TX,
    TWI_STATE_RX
} twi_state_t;

typedef struct
{
    twi_state_t     state;
    uint8_t         *buff;
    uint16_t        len;
    uint8_t         slave_addr;
    twi_tx_callback tx_cb;
    twi_rx_callback rx_cb;
    uint8_t         is_initialized;
} twi_descriptor_t;

/*****************************************************************************/
/*                         Static global vars                                */
/*****************************************************************************/
static volatile twi_descriptor_t m_desc = {.is_initialized = ERROR_INVALID_STATE};
/*****************************************************************************/
/*                         Static function                                   */
/*****************************************************************************/
static void error_handler(uint8_t status)
{
    switch (m_desc.state)
    {
        case TWI_STATE_TX:
            if (m_desc.tx_cb)
                m_desc.tx_cb(status);
            break;
        case TWI_STATE_RX:
            if (m_desc.rx_cb)
                m_desc.rx_cb(status);
            break;
        default:
            break;
    }
    LED_DEBUG_BLINK(2);
}

ISR(TWI_vect)
{
    switch (TWSR)
    {
        case TWI_START_TRANSMITTED:
        case TWI_R_START_TRANSMITTED:
        { 
            TWDR = (m_desc.slave_addr) | ((m_desc.state == TWI_STATE_TX) ? TWI_WRITE : TWI_READ);
            TWCR = (1 << TWINT) | (1 << TWIE) | (1 << TWEN);
            break;
        }
        case TWI_SLA_W_TRANSMITTED_ACK:
        {
            if (m_desc.len)
            {
                TWDR = *m_desc.buff++;
                m_desc.len--;
                TWCR = (1 << TWINT) | (1 << TWIE) | (1 << TWEN);
            }
            else
            {
                error_handler(TWSR);
                m_desc.state = TWI_STATE_IDLE;
            }
            break;
        }
        case TWI_DATA_TRANSMITTED_ACK:
        {
            if (m_desc.len)
            {
                TWDR = *m_desc.buff++;
                m_desc.len--;
                TWCR = (1 << TWINT) | (1 << TWIE) | (1 << TWEN);
            }
            else
            {
                if (m_desc.tx_cb)
                {
                    m_desc.tx_cb(TWSR);
                }
                TWI_STOP_CONDITION();
                m_desc.state = TWI_STATE_IDLE;
            }
            break;
        }
        case TWI_SLA_R_TRANSMITTED_ACK:
        {
            TWCR = (1 << TWINT) | (1 << TWIE) | (1 << TWEN) | ((m_desc.len == 1) ? (0 << TWEA) : (1 << TWEA));
            break;
        }
        case TWI_DATA_RECEIVED_ACK:
        {
            m_desc.buff[0] = TWDR;
            m_desc.buff++;
            m_desc.len--;
            TWCR = (1 << TWINT) | (1 << TWIE) | (1 << TWEN) | ((m_desc.len == 1) ? (0 << TWEA) : (1 << TWEA));
            break;
        }
        case TWI_DATA_RECEIVED_NACK:
        {
            m_desc.buff[0] = TWDR;
            m_desc.len--;
            if (m_desc.rx_cb)
            {
                m_desc.rx_cb(TWI_STATE_RX);
            }
            m_desc.state = TWI_STATE_IDLE;
            TWI_STOP_CONDITION();
            break;
        }
        default:
        {
            error_handler(TWSR);
            m_desc.state = TWI_STATE_IDLE;
            TWI_STOP_CONDITION();
            break;
        }
    }
}

/*****************************************************************************/
/*                         Public API                                        */
/*****************************************************************************/

void twi_init(twi_scl_t f_scl, twi_tx_callback tx_cb, twi_rx_callback rx_cb)
{
    m_desc.tx_cb = tx_cb;
    m_desc.rx_cb = rx_cb;
    TWBR = ((F_CPU/f_scl) - 16) / (2*(TWSR & 0x03));
    TWCR = (1 << TWEN) |                                        /* < Enable module           */
           (1 << TWIE) |                                        /* < Interrupt Enable        */
           (0 << TWEA);                                         /* < Disable acknowledge Bit */
    m_desc.is_initialized = ERROR_SUCCESS;
    __LOG(LOG_LEVEL_INFO, "TWI initializated.\r\n");
}

error_t twi_initialized(void)
{
    return m_desc.is_initialized;
}

error_t twi_send(uint8_t addr, const void* p_data, uint16_t len)
{
    // if (TWI_STATE_IDLE != m_desc.state)
    // {
    //     return ERROR_BUSY;
    // }
    // uint8_t *data = (uint8_t*)p_data;
    // __LOG(LOG_LEVEL_INFO, "%s len: %d d: %02X %02X\r\n", __func__, len, data[0], data[1]);
    m_desc.slave_addr = addr;
    m_desc.buff       = (uint8_t*)p_data;
    m_desc.len        = len;
    m_desc.state      = TWI_STATE_TX;
    TWI_START_CONDITION();
    while(m_desc.state != TWI_STATE_IDLE) { _delay_ms(10);}
    return ERROR_SUCCESS;
}

error_t twi_read(uint8_t addr, void* p_data, uint16_t len)
{
    if (TWI_STATE_IDLE != m_desc.state)
    {
        return ERROR_BUSY;
    }
    m_desc.slave_addr = addr;
    m_desc.buff       = p_data;
    m_desc.len        = len;
    m_desc.state      = TWI_STATE_RX;
    TWI_START_CONDITION();
    return ERROR_SUCCESS;
}

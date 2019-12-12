#include <avr/io.h>
#include "etwi.h"

#define ETWI_ENABLE_TRANSFER() (TWCR = (1 << TWINT) | (1 << TWEN))

static etwi_status_t wait(etwi_status_t expected)
{
    /* timout value should be > 280 for 50KHz Bus and 16 Mhz CPU, 
       however the start condition might need longer */
    
    volatile uint16_t cnt = 0xFFFF;
    // while(cnt--);
    // return ETWI_STATUS_SUCCESS;
    while (!(TWCR & (1 << TWINT)))
    {
        if (cnt == 0)
        {
            return ETWI_STATUS_TIMEOUT;
        }
        --cnt;
    }

    return (TWSR  == expected) ? ETWI_STATUS_SUCCESS : TWSR;
}

void etwi_init(void)
{
    TWSR = 0;
    TWBR = F_CPU/(2*100000UL)-8;

    TWCR = (1 << TWEN) |                                        /* < Enable module           */
           (0 << TWIE) |                                        /* < Interrupt Enable        */
           (0 << TWEA);            
}

etwi_status_t etwi_start(uint8_t sla_addr)
{
    register etwi_status_t status;
    /* send start */
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    status = wait(ETWI_START_TRANSMITTED);
    if (ETWI_STATUS_SUCCESS != status)
    {
        return status;
    }

    /* set slave address */
    TWDR = sla_addr;
    /* enable sla transfer */
    ETWI_ENABLE_TRANSFER();

    status = wait(ETWI_SLA_W_TRANSMITTED_ACK);
    if (ETWI_STATUS_SUCCESS != status)
    {
        return status;
    }
    return ETWI_STATUS_SUCCESS;
}

etwi_status_t etwi_send_array(const uint8_t *p_data, uint16_t len)
{
    register etwi_status_t status;

    for(uint16_t i = 0; i < len; ++i)
    {
        TWDR = p_data[i];
        ETWI_ENABLE_TRANSFER();

        status = wait(ETWI_DATA_TRANSMITTED_ACK);
        if (ETWI_STATUS_SUCCESS != status)
        {
            return status;
        }
    }
    return ETWI_STATUS_SUCCESS;
}

void etwi_stop()
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
    /* write stop */
    wait(1 << TWSTO);
}
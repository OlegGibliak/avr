#ifndef ETWI_H__
#define ETWI_H__

typedef enum
{
    ETWI_STATUS_SUCCESS         = 0x00,
    ETWI_STATUS_TIMEOUT         = 0x01,
    ETWI_STATUS_BUS_ERROR       = 0x02,
    ETWI_START_TRANSMITTED      = 0x08,
    ETWI_R_START_TRANSMITTED    = 0x10,
    ETWI_SLA_W_TRANSMITTED_ACK  = 0x18,
    ETWI_SLA_W_TRANSMITTED_NACK = 0x20,
    ETWI_DATA_TRANSMITTED_ACK   = 0x28,
    ETWI_DATA_TRANSMITTED_NACK  = 0x30,
    ETWI_ARBITRATION_LOST       = 0x38,
    ETWI_SLA_R_TRANSMITTED_ACK  = 0x40,
    ETWI_SLA_R_TRANSMITTED_NACK = 0x48,
    ETWI_DATA_RECEIVED_ACK      = 0x50,
    ETWI_DATA_RECEIVED_NACK     = 0x58
} etwi_status_t;

void etwi_init(void);
etwi_status_t etwi_start(uint8_t sla_addr);
etwi_status_t etwi_send_array(const uint8_t *p_data, uint16_t len);
void etwi_stop(void);

#endif /* ETWI_H__ */
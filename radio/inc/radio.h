#ifndef RADIO_H__
#define RADIO_H__
/********************************************************************
*                         Standard headers                          *
********************************************************************/
#include <stdint.h>

/********************************************************************
*                           Local headers                           *
********************************************************************/

/********************************************************************
*                       Function macro defines                      *
********************************************************************/
#define RADIO_STATE_IRQ
#define RADIO_STATE_POLLING

/********************************************************************
*                             Typedefs                              *
********************************************************************/
typedef enum
{
    RADIO_E_SUCCESS,
    RADIO_E_NOT_CONNECTED,
    RADIO_E_INVALID_CONFIG,
    RADIO_E_INTERNAL,
    RADIO_E_NO_MEM,
} radio_error_t;

/********************************************************************
*                                API                                *
********************************************************************/
radio_error_t radio_init(void);
radio_error_t radio_pkt_send(uint8_t *addr, uint8_t *pkt, uint8_t len);
radio_error_t radio_listen(uint8_t *addr);
void radio_proccess(void);
#endif /* RADIO_H__ */
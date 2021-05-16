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
} radio_error_t;

/********************************************************************
*                                API                                *
********************************************************************/

#endif /* RADIO_H__ */
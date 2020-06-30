/********************************************************************
*                         Standard headers                          *
********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/********************************************************************
*                           Local headers                           *
********************************************************************/
#include "app_timer.h"
#include "error.h"
#include "timer_timestamp.h"

/********************************************************************
*                             Typedefs                              *
********************************************************************/
/* Timer context structure. */
typedef struct
{
    app_timer_t *p_head;
} timer_desc_t;

/********************************************************************
*                  Static global data declarations                  *
********************************************************************/
/* Timer context declaration. */
static timer_desc_t g_app_timer_desc;

/********************************************************************
*                     Functions implementations                     *
********************************************************************/
void app_timer_process(void)
{
    if (g_app_timer_desc.p_head == NULL)
    {
        return;
    }

    while(g_app_timer_desc.p_head != NULL)
    {
        uint32_t timestamp_now = timer_timestamp_ms_get();

        /* timestamp after time now */
        if (((int32_t)(g_app_timer_desc.p_head->timestamp - timestamp_now)) > 0)
        {
            return;
        }

        app_timer_t *p_timer    = g_app_timer_desc.p_head;
        g_app_timer_desc.p_head = p_timer->p_next;

        uint32_t next_timeout_ms = APP_TIMER_STOP;

        if (p_timer->cb != NULL)
        {
            next_timeout_ms = p_timer->cb(p_timer->p_context);
        }

        if (next_timeout_ms != APP_TIMER_STOP)
        {
            app_timer_add(p_timer, next_timeout_ms);
        }
    }
}

/********************************************************************
*                                 Api                               *
********************************************************************/
void app_timer_init(void)
{
    memset(&g_app_timer_desc, 0, sizeof(g_app_timer_desc));
    timer_timestamp_init();
}

error_t app_timer_add(app_timer_t *p_timer, uint32_t time_ms)
{
    uint32_t timestamp_now = timer_timestamp_ms_get();
    
    p_timer->timestamp = timestamp_now + time_ms;
    
    if (g_app_timer_desc.p_head == NULL ||
        ((int32_t)(g_app_timer_desc.p_head->timestamp - p_timer->timestamp)) > 0)
    {

        p_timer->p_next = g_app_timer_desc.p_head;
        g_app_timer_desc.p_head = p_timer;
    }
    else
    {
        app_timer_t *p_iter = g_app_timer_desc.p_head;
        while(p_iter->p_next != NULL &&
              ((int32_t)(p_timer->timestamp - p_iter->p_next->timestamp)) > 0)
        {
            p_iter = p_iter->p_next;
        }

        p_timer->p_next = p_iter->p_next;
        p_iter->p_next  = p_timer;

    }

    return ERROR_SUCCESS;
}

void app_timer_remove(app_timer_t *p_timer)
{
    app_timer_t *p_iter = g_app_timer_desc.p_head;
    app_timer_t *p_prev = NULL;

    while(p_iter != NULL && p_iter != p_timer)
    {
        p_prev = p_iter;
        p_iter = p_iter->p_next;
    }

    if (p_iter != NULL)
    {
        if(p_prev == NULL)
        {
            g_app_timer_desc.p_head = p_iter->p_next;
        }
        else
        {
            p_prev->p_next = p_iter->p_next;
        }
    }
}

void app_timer_reschedule(app_timer_t *p_timer, uint32_t time_ms)
{
    app_timer_remove(p_timer);
    app_timer_add(p_timer, time_ms);
}
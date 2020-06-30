#ifndef APP_TIMER_H__
#define APP_TIMER_H__

/********************************************************************
*                         Standard headers                          *
********************************************************************/
#include <stdint.h>

/********************************************************************
*                           Local headers                           *
********************************************************************/
#include "error.h"

/********************************************************************
*                       Constant macro defines                      *
********************************************************************/
#define APP_TIMER_STOP             ((uint32_t)(-1))

/********************************************************************
* @brief      Function type for the timeout callback.
*
* @param[in]  p_context       Pointer to the data provided when starting the timer.
*
* @returns    uint32_t        Next timeout in ms.
*
* @note
********************************************************************/
typedef uint32_t (*app_timer_callback_t)(void *p_context);

/* Timer structure for schedulable timers. */
typedef struct app_timer_s
{
    uint32_t                        timestamp;  /* The timestamp at which to trigger. */
    app_timer_callback_t            cb;         /* Callback function for a call when the timer is triggered. */
    void                            *p_context; /* Pointer to the data. */
    struct app_timer_s              *p_next;    /* Pointer to next timer in linked list. Only for internal usage. */
} app_timer_t;

/********************************************************************
* @brief      Initializes the timer module.
*
* @param      void.
*
* @returns    void.
*
* @note
********************************************************************/
void app_timer_init(void);

/********************************************************************
* @brief      Adds a timer into the list of timers.
*
* @param[in]  p_timer           Pointer to a allocated timer.
* @param[in]  time_ms           Time in ms when the timer should start.
*
* @returns    E_INTERNAL_ERROR  Internal error
* @returns    E_SUCCESS         Timer was added.
*
* @note The structure parameters must not be changed after
* the structure has been added.
********************************************************************/
error_t app_timer_add(app_timer_t *p_timer, uint32_t time_ms);

/********************************************************************
* @brief      Removes a timer from the list of timers.
*
* @param[in]  p_timer           Pointer to a allocated timer.
*
* @returns    void.
*
* @note
********************************************************************/
void app_timer_remove(app_timer_t *p_timer);

/********************************************************************
* @brief      Reschedules a previously added timer.
*
* @param[in]  p_timer           Pointer to a allocated timer.
* @param[in]  time_ms           Time in ms when the timer should start.
*
* @returns    void.
*
* @note
********************************************************************/
void app_timer_reschedule(app_timer_t *p_timer, uint32_t time_ms);

void app_timer_process(void);

void print_timers();
#endif /* APP_TIMER_H__ */
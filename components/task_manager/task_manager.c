#include <avr/io.h>
#include <avr/interrupt.h>
#include "task_manager.h"
#include "assert.h"
#include "list.h"

#define TIMER_INTERRUPT_ENABLE() 		(TIMSK0  = (1 << OCIE0A))
#define TIMER_INTERRUPT_DISABLE()		(TIMSK0  = 0)

typedef enum
{
	TASK_STATE_IDLE,
	TASK_STATE_RUN,
} task_state_t;

typedef struct
{
	task_state_t state;
	task_func_t  cb;
	void         *p_param;
	uint16_t     delay;
	uint16_t     period;
} task_t;

/***********************************************************/
/*                    Static global vars                   */
/***********************************************************/
LIST_INSTANCE(task_list, sizeof(task_t), TASK_MANAGER_MAX_TASK_NUM);

/***********************************************************/
/*                    Privat function                      */
/***********************************************************/
ISR(TIMER0_COMPA_vect)
{
    FOREACH(&task_list, p_iter)
    {
    	task_t* p_task = list_item_data_get(p_iter);
    	if (p_task->delay == 0)
    	{
    		p_task->state = TASK_STATE_RUN;
    	}
    	else
    	{
    		p_task->delay--;
    	}
    }
}

/***********************************************************/
/*                    Public API                           */
/***********************************************************/
void task_manager_init(void)
{
	TIMSK0  = (1 << OCIE0A);                /* < Timer Output Compare Match A Interrupt Enable */
    TCNT0   = 0;
    OCR0A   = 250;
    TCCR0A  = (1 << WGM01); 				/* < Timer mode is CTC */
    TCCR0B  = (1 << CS01) | (1 << CS00);    /* < Timer prescaler is 64 */
    /* Should be 1 ms. */
}

error_t task_create(task_func_t task_cb, void *p_param, uint16_t delay, uint16_t period)
{
	task_t new_task = {.state   = TASK_STATE_IDLE,
	                   .cb      = task_cb,
	                   .p_param = p_param,
	                   .delay   = delay,
	                   .period  = period};
	return list_push_back(&task_list, &new_task);
}

void task_proccess(void)
{
	FOREACH(&task_list, p_iter)
    {
    	task_t* p_task = list_item_data_get(p_iter);
    	if (p_task->state == TASK_STATE_RUN)
    	{
    		p_task->cb(p_task->p_param);
    		if (p_task->period)
    		{
    			p_task->delay = p_task->period;
    			p_task->state = TASK_STATE_IDLE;
    		}
    		else
    		{
    			TIMER_INTERRUPT_DISABLE();
    			ASSERT(list_remove(&task_list, p_iter) == ERROR_SUCCESS);
    			TIMER_INTERRUPT_ENABLE();
    		}
    	}
    }
}

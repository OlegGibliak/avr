#include <string.h>
#include "fifo.h"

fifo_status_t fifo_push(fifo_t *p_fifo, const void *p_elem)
{
	if (p_fifo->p_tail)
	{
		uint8_t *p_end_of_pool = p_fifo->p_mem_pool + (p_fifo->el_size * p_fifo->el_num);
		if (p_fifo->p_tail > p_fifo->p_head)
		{
			if (p_fifo->p_tail + p_fifo->el_size != p_end_of_pool)
			{
				p_fifo->p_tail = p_fifo->p_tail + p_fifo->el_size;
				memcpy(p_fifo->p_tail, p_elem, p_fifo->el_size);
			}
			else
			{
				if ((p_fifo->p_mem_pool != p_fifo->p_head))
				{
					p_fifo->p_tail = p_fifo->p_mem_pool;
					memcpy(p_fifo->p_tail, p_elem, p_fifo->el_size);
				}
				else
				{
					return FIFO_STATUS_NO_MEM;
				}
			}
		}
		else if ((p_fifo->p_tail + p_fifo->el_size) < p_fifo->p_head)
		{
			p_fifo->p_tail = p_fifo->p_tail + p_fifo->el_size;
			memcpy(p_fifo->p_tail, p_elem, p_fifo->el_size);
		}
		else  if (p_fifo->p_tail == p_fifo->p_head)
		{
			p_fifo->p_tail = ((p_fifo->p_tail + p_fifo->el_size) < p_end_of_pool) ?
				   			 (p_fifo->p_tail + p_fifo->el_size) : p_fifo->p_mem_pool;
			memcpy(p_fifo->p_tail, p_elem, p_fifo->el_size);
		}
		else /* (p_fifo->p_tail + p_fifo->el_size) == p_fifo->p_head */
		{
			return FIFO_STATUS_NO_MEM;
		}
	}
	else
	{
		p_fifo->p_tail = p_fifo->p_head = p_fifo->p_mem_pool;
		memcpy(p_fifo->p_tail, p_elem, p_fifo->el_size);
	}
#ifdef FIFO_DEBUG_ENABLE
	p_fifo->size++;
#endif /* FIFO_DEBUG_ENABLE */
	return FIFO_STATUS_SUCCESS;
}

void* fifo_pop(fifo_t *p_fifo)
{
	if (!p_fifo->p_head)
	{
		return NULL;
	}
	void *p_item = p_fifo->p_head;
	if (p_fifo->p_head == p_fifo->p_tail)
	{
		p_fifo->p_tail = p_fifo->p_head = NULL;
	}
	else
	{
		uint8_t *p_end_of_pool = p_fifo->p_mem_pool + (p_fifo->el_size * p_fifo->el_num);
		p_fifo->p_head = ((p_fifo->p_head + p_fifo->el_size) < p_end_of_pool) ?
                     (p_fifo->p_head + p_fifo->el_size) : p_fifo->p_mem_pool;
    }
#ifdef FIFO_DEBUG_ENABLE
	p_fifo->size--;
#endif /* FIFO_DEBUG_ENABLE */
    return p_item;
}

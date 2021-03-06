#include <stdio.h>
#include <string.h>
#include "list.h"

typedef enum
{
    LIST_STATE_FREE,
    LIST_STATE_COMMITED
} list_node_state_t;

/*******************************************************************/
/*                       Static function                           */
/*******************************************************************/
static inline list_node_t* mem_alloc(list_t *p_list)
{
	uint8_t el_struct_size = p_list->element_size + sizeof(list_node_t);

	for (uint8_t *p_item = (uint8_t*)p_list->p_mem_pool;
		 p_item < ((uint8_t*)p_list->p_mem_pool + p_list->element_max_num * el_struct_size);
		 p_item += el_struct_size)
	{
		list_node_t *p_elem = (list_node_t*)p_item;
		if (LIST_STATE_FREE == p_elem->state)
		{
			p_elem->state = LIST_STATE_COMMITED;
			return p_elem;
		}
	}
	return NULL;
}

static inline void mem_free(list_node_t *p_remove)
{
	p_remove->state  = LIST_STATE_FREE;
	p_remove->p_next = NULL;
}
/*******************************************************************/
/*                       Public api                                */
/*******************************************************************/
void* list_front(list_t *p_list)
{
	return (p_list->p_head) ? p_list->p_head->data : NULL;
}

void* list_back(list_t *p_list)
{
	list_node_t *p_last = NULL;
	FOREACH(p_list, p_iter)
	{
		p_last = p_iter;
	}
    return p_last;
}

error_t list_push_front(list_t *p_list, void *p_data)
{
	if (NULL == p_list || NULL == p_data)
		return ERROR_INVALID_PARAM;

	list_node_t *p_new = mem_alloc(p_list);
	if (NULL == p_new)
		return ERROR_NO_MEM;

	memcpy(p_new->data, p_data, p_list->element_size);
	p_new->p_next      = p_list->p_head;
	p_list->p_head     = p_new;
	p_list->size++;
	return ERROR_SUCCESS;
}

error_t list_push_back(list_t *p_list, void *p_data)
{
	if (NULL == p_list || NULL == p_data)
		return ERROR_INVALID_PARAM;

	list_node_t *p_new = mem_alloc(p_list);
	if (NULL == p_new)
		return ERROR_NO_MEM;

	memcpy(p_new->data, p_data, p_list->element_size);

	list_node_t *p_last = NULL;
	FOREACH(p_list, p_iter)
	{
		p_last = p_iter;
	}

	if (p_last)
	{
		p_last->p_next = p_new;
	}
	else
	{
		p_list->p_head = p_new;
	}
	p_list->size++;
	return ERROR_SUCCESS;
}

error_t list_pop_front(list_t *p_list)
{
	if (NULL == p_list)
		return ERROR_INVALID_PARAM;

	if (NULL == p_list->p_head)
		return ERROR_SUCCESS;

	list_node_t *p_remove = p_list->p_head;
	p_list->p_head        = p_list->p_head->p_next;
	mem_free(p_remove);
	p_list->size--;
	return ERROR_SUCCESS;
}

error_t list_pop_back(list_t *p_list)
{
	if (NULL == p_list)
		return ERROR_INVALID_PARAM;

	if (NULL == p_list->p_head)
	{
		return ERROR_SUCCESS;
	}

	list_node_t *p_entry = p_list->p_head;
	list_node_t *p_prev  = NULL;

	while (p_entry->p_next)
	{
		p_prev  = p_entry;
		p_entry = p_entry->p_next;
	}

	if (NULL != p_prev)
	{
		p_prev->p_next = NULL;
	}
	else
	{
		p_list->p_head = NULL;
	}
	mem_free(p_entry);
	p_list->size--;
	return ERROR_SUCCESS;
}

error_t list_remove_if(list_t *p_list, void *p_remove, predicate_func_t predicate)
{
	if (NULL == p_list || NULL == predicate)
	{
		return ERROR_INVALID_PARAM;
	}

	list_node_t *p_entry = p_list->p_head;
	list_node_t *p_prev  = NULL;

	while (p_entry)
	{
		if (predicate(p_entry->data, p_remove))
		{
			if (p_prev)
			{
				p_prev->p_next = p_entry->p_next;
			}
			else
			{
				p_list->p_head = p_entry->p_next;
			}
			mem_free(p_entry);
			p_list->size--;
			return ERROR_SUCCESS;
		}
		p_prev  = p_entry;
		p_entry = p_entry->p_next;
	}
	return ERROR_NOT_FOUND;
}

error_t list_remove(list_t *p_list, void *p_remove)
{
	if (NULL == p_list || NULL == p_remove)
	{
		return ERROR_INVALID_PARAM;
	}

	list_node_t  *p_entry = p_list->p_head;
	list_node_t *p_prev  = NULL;

	while (p_entry)
	{
		if (p_entry == p_remove)
		{
			if (p_prev)
			{
				p_prev->p_next = p_entry->p_next;
			}
			else
			{
				p_list->p_head = p_entry->p_next;
			}
			mem_free(p_entry);
			p_list->size--;
			return ERROR_SUCCESS;
		}
		p_prev  = p_entry;
		p_entry = p_entry->p_next;
	}
	return ERROR_NOT_FOUND;
}

void* list_item_data_get(list_node_t *p_item)
{
	return (void*)p_item->data;
}
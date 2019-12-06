#ifndef LIST_H__
#define LIST_H__

#include <stdint.h>

#define LIST_STATUS_BASE                (40)

typedef enum
{
    LIST_STATUS_SUCCESS       = 0,
    LIST_STATUS_NO_MEM        = LIST_STATUS_BASE,
    LIST_STATUS_INVALID_PARAM,
    LIST_STATUS_NOT_FOUND
} list_status_t;

typedef struct list_node_t
{
    uint8_t            state;
    struct list_node_t *p_next;
    uint8_t            data[];
} list_node_t;

typedef struct
{
    uint8_t     element_size;
    uint8_t     element_max_num;
    uint8_t     size;
    list_node_t *p_head;
    list_node_t *p_mem_pool;
} list_t;

#define LIST_INSTANCE(_list_name, _el_size, _el_max_num)                                    \
    static uint8_t  _list_name##_mem_pool[(_el_size + sizeof(list_node_t)) * _el_max_num];  \
    static list_t   _list_name  = {                                                         \
        .element_size    = _el_size,                                                        \
        .element_max_num = _el_max_num,                                                     \
        .p_head          = NULL,                                                            \
        .p_mem_pool      = (list_node_t*)&(_list_name##_mem_pool)};

#define FOREACH(_list, _iter)                                                   \
for (list_node_t *_iter = _list->p_head; _iter != NULL; _iter = _iter->p_next)

void* list_front(list_t *p_list);
void* list_back(list_t *p_list);

list_status_t list_push_front(list_t *p_list, void *p_data);
list_status_t list_push_back(list_t *p_list, void *p_data);
list_status_t list_pop_front(list_t *p_list);
list_status_t list_pop_back(list_t *p_list);

#endif /* FIFO_H__ */

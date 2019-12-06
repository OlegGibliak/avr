#ifndef FIFO_H__
#define FIFO_H__

#include <stdint.h>

#define FIFO_DEBUG_ENABLE
#define FIFO_STATUS_BASE                                            (40)

typedef enum
{
    FIFO_STATUS_SUCCESS = 0,
    FIFO_STATUS_NO_MEM  = FIFO_STATUS_BASE,
    FIFO_STATUS_ENPTY
} fifo_status_t;

typedef struct
{
    uint8_t el_size;
    uint8_t el_num;
    uint8_t *p_head;
    uint8_t *p_tail;
    uint8_t *p_mem_pool;
#ifdef FIFO_DEBUG_ENABLE
    int     size;
#endif /* FIFO_DEBUG_ENABLE */
} fifo_t;

#define FIFO_INSTANCE_CREATE(_name, _el_size, _el_num)              \
    static uint8_t m_##_name##_mem_pool[_el_size * _el_num];        \
    static fifo_t _name = {                                         \
        .el_size    = _el_size,                                     \
        .el_num     = _el_num,                                      \
        .p_head     = NULL,                                         \
        .p_tail     = NULL,                                         \
        .p_mem_pool = m_##_name##_mem_pool};

fifo_status_t fifo_push(fifo_t *p_fifo, const void *p_elem);
void*         fifo_pop(fifo_t *p_fifo);

#endif /* FIFO_H__ */

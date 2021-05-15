#ifndef TASK_MANAGER_H__
#define TASK_MANAGER_H__

#include "error.h"
#define TASK_MANAGER_MAX_TASK_NUM				(10)

typedef void (*task_func_t)(void *p_param);

void task_manager_init(void);
error_t task_create(task_func_t task_cb, void *p_param, uint16_t delay, uint16_t period);
void task_proccess(void);

#endif /* TASK_MANAGER_H__ */
#include "serial.h"
#include "util.h"
#include "led_dbg.h"
#include "assert.h"
#include "ssd1306.h"
#include "task_manager.h"
#include "logger.h"

static void task_inside(void * p_param)
{
	__LOG(LOG_LEVEL_INFO, "%s\r\n", __func__);
}

static void task_sys(void *p_param)
{
	uint16_t param = (uint16_t)p_param;
	__LOG(LOG_LEVEL_INFO, "%s param: %X\r\n", __func__, param);
	task_create(task_inside, 0, 500, 0);
}

static void task_sys_led_on(void *p_param)
{
	LED_DEBUG_ON();
	__LOG(LOG_LEVEL_INFO, "%s\r\n", __func__);
}

static void task_sys_led_off(void *p_param)
{
	LED_DEBUG_OFF();
	__LOG(LOG_LEVEL_INFO, "%s\r\n", __func__);
}

int main()
{
	LED_DEBUG_INIT();
	INTERRUPT_ENABLE();
	serial_init();
	// ssd1306_init();
	// ssd1306_draw_pixel(10, 10);
	// ssd1306_update();

	task_manager_init();
	void *p_param = (void*)0xABCD;
	task_create(task_sys, p_param, 100, 5000);
	task_create(task_sys_led_on,  0, 0, 2000);
	task_create(task_sys_led_off, 0, 1000, 2000);
	for(;;)
	{
		task_proccess();		
	}
}
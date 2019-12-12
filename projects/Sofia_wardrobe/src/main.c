#include "serial.h"
#include "util.h"
#include "led_dbg.h"
#include "assert.h"
#include "ssd1306.h"
#include "task_manager.h"
#include "logger.h"

#include <stdlib.h>

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

static void task_draw_pixel(void *p_param)
{
	__LOG(LOG_LEVEL_INFO, "%s\r\n", __func__);
	// static uint8_t x, y;
	// x = rand() % 128;
	// y = rand() % 32;
	// ssd1306_draw_pixel(x, y);
	// ssd1306_update();
}
int main()
{
	LED_DEBUG_INIT();
	INTERRUPT_ENABLE();
	serial_init();
	ssd1306_init();
	
	ssd1306_putc(0, 0, 'M');
	ssd1306_rect(50, 5, 50, 20);
	ssd1306_update();

	task_manager_init();
	task_create(task_sys_led_on,  0, 0, 2000);
	task_create(task_sys_led_off, 0, 1000, 2000);
	task_create(task_draw_pixel,  0, 0, 5000);
	for(;;)
	{
		task_proccess();		
	}
}
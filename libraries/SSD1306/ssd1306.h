#ifndef SSD1306__H__
#define SSD1306__H__

#include <stdint.h>

#define SSD1306_WIDTH 		(128)
#define SSD1306_HEIGHT		(32)

void ssd1306_init(void);
void ssd1306_update(void);
void ssd1306_draw_pixel(uint8_t x, uint8_t y);

#endif /* SSD1306__H__ */
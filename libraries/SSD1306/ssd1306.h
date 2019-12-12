#ifndef SSD1306__H__
#define SSD1306__H__

#include <stdint.h>

#define SSD1306_WIDTH 		(128)
#define SSD1306_HEIGHT		(32)

void ssd1306_init(void);
void ssd1306_update(void);
void ssd1306_draw_pixel(uint8_t x, uint8_t y);
void ssd1306_line_v(uint8_t x, uint8_t y, uint8_t height);
void ssd1306_line_h(uint8_t x, uint8_t y, uint8_t width);
void ssd1306_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void ssd1306_putc(uint8_t x, uint8_t y, char c);

#endif /* SSD1306__H__ */
#include "ssd1306.h"
#include "twi.h"
#include "assert.h"
#include "logger.h"
#include <stddef.h>

#define SSD1306_ADDRESS                         (0b01111000) /* (0b01111010) */
#define SSD1306_SLA_W                           (SSD1306_ADDRESS)

#define SSD1306_CONTROL_BYTE_COM                (0b10000000)
#define SSD1306_CONTROL_BYTE_DAT                (0b11000000)
#define SSD1306_CONTROL_BYTE_DATS               (0b01000000)

#define CONTROL_BYTE_SIZE                       (1)

#define SSD1306_WRITE_COMMAND(_cmd)				ssd1306_write_single_byte(SSD1306_CONTROL_BYTE_COM, _cmd)
#define SSD1306_WRITE_DATA(_data)               ssd1306_write_single_byte(SSD1306_CONTROL_BYTE_DAT, _data)
#define SSD1306_WRITE_DATA_ARR(_p_data, _len)   ssd1306_write_multi_byte(_p_data, _len)

#define SSD1306_BUFF_SIZE (SSD1306_WIDTH * SSD1306_HEIGHT / 8)
/*****************************************************************************/
/*                         Static global vars                                */
/*****************************************************************************/
static uint8_t display_buff[SSD1306_BUFF_SIZE + CONTROL_BYTE_SIZE];
static uint8_t *mp_display_buff = display_buff + CONTROL_BYTE_SIZE;
/*****************************************************************************/
/*                         Static function                                   */
/*****************************************************************************/
static inline void ssd1306_write_single_byte(uint8_t co_byte, uint8_t cmd)
{
	uint8_t tx_buff[CONTROL_BYTE_SIZE + sizeof(cmd)] = {co_byte, cmd};
	ASSERT(twi_send(SSD1306_SLA_W, tx_buff, sizeof(tx_buff)) == ERROR_SUCCESS);
}

static inline void ssd1306_write_multi_byte(uint8_t *p_data, uint16_t len)
{
	ASSERT(twi_send(SSD1306_SLA_W, p_data, len) == ERROR_SUCCESS);
}

/*****************************************************************************/
/*                         Public API                                        */
/*****************************************************************************/

void ssd1306_init(void)
{
	if (twi_initialized() != ERROR_SUCCESS)
	{
		twi_init(TWI_SCL_50KHZ, NULL, NULL);
	}

	/* A little delay */
	uint32_t p = 2500;
	while(p>0)
		p--;

	/* Init LCD */
	SSD1306_WRITE_COMMAND(0xAE); //display off
	SSD1306_WRITE_COMMAND(0x20); //Set Memory Addressing Mode   
	SSD1306_WRITE_COMMAND(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	SSD1306_WRITE_COMMAND(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	SSD1306_WRITE_COMMAND(0xC8); //Set COM Output Scan Direction
	SSD1306_WRITE_COMMAND(0x00); //---set low column address
	SSD1306_WRITE_COMMAND(0x10); //---set high column address
	SSD1306_WRITE_COMMAND(0x40); //--set start line address
	SSD1306_WRITE_COMMAND(0x81); //--set contrast control register
	SSD1306_WRITE_COMMAND(0xFF);
	SSD1306_WRITE_COMMAND(0xA1); //--set segment re-map 0 to 127
	SSD1306_WRITE_COMMAND(0xA6); //--set normal display
	SSD1306_WRITE_COMMAND(0xA8); //--set multiplex ratio(1 to 64)
	SSD1306_WRITE_COMMAND(0x3F); //
	SSD1306_WRITE_COMMAND(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	SSD1306_WRITE_COMMAND(0xD3); //-set display offset
	SSD1306_WRITE_COMMAND(0x00); //-not offset
	SSD1306_WRITE_COMMAND(0xD5); //--set display clock divide ratio/oscillator frequency
	SSD1306_WRITE_COMMAND(0xF0); //--set divide ratio
	SSD1306_WRITE_COMMAND(0xD9); //--set pre-charge period
	SSD1306_WRITE_COMMAND(0x22); //
	SSD1306_WRITE_COMMAND(0xDA); //--set com pins hardware configuration
	SSD1306_WRITE_COMMAND(0x02);
	SSD1306_WRITE_COMMAND(0xDB); //--set vcomh
	SSD1306_WRITE_COMMAND(0x20); //0x20,0.77xVcc
	SSD1306_WRITE_COMMAND(0x8D); //--set DC-DC enable
	SSD1306_WRITE_COMMAND(0x14); //
	SSD1306_WRITE_COMMAND(0xAF); //--turn on SSD1306 panel
	
    __LOG(LOG_LEVEL_INFO, "ssd1306 initializated.\r\n");
}

void ssd1306_update(void)
{
    display_buff[0] = SSD1306_CONTROL_BYTE_DATS;
    SSD1306_WRITE_DATA_ARR(display_buff, sizeof(display_buff));
}
void ssd1306_draw_pixel(uint8_t x, uint8_t y)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        return;
    }

    mp_display_buff[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
}

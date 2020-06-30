#include <avr/pgmspace.h>   
#include <stddef.h>
#include "ssd1306.h"
#include "twi.h"
#include "assert.h"
#include "logger.h"
#include "font.h"

#define SSD1306_ADDRESS                         (0b01111000) /* (0b01111010) */
#define SSD1306_SLA_W                           (SSD1306_ADDRESS)

#define SSD1306_CONTROL_BYTE_COMS               (0b00000000)
#define SSD1306_CONTROL_BYTE_COM                (0b10000000)
#define SSD1306_CONTROL_BYTE_DAT                (0b11000000)
#define SSD1306_CONTROL_BYTE_DATS               (0b01000000)

#define CONTROL_BYTE_SIZE                       (1)
#define SSD1306_BUFF_SIZE                       (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

#define SSD1306_WRITE(_p_data, _len) twi_send(SSD1306_ADDRESS, _p_data, _len)

#define SSD1306_SETCONTRAST                     0x81
#define SSD1306_DISPLAYALLON_RESUME             0xA4
#define SSD1306_DISPLAYALLON                    0xA5
#define SSD1306_NORMALDISPLAY                   0xA6
#define SSD1306_INVERTDISPLAY                   0xA7
#define SSD1306_DISPLAYOFF                      0xAE
#define SSD1306_DISPLAYON                       0xAF
#define SSD1306_SETDISPLAYOFFSET                0xD3
#define SSD1306_SETCOMPINS                      0xDA
#define SSD1306_SETVCOMDETECT                   0xDB
#define SSD1306_SETDISPLAYCLOCKDIV              0xD5
#define SSD1306_SETPRECHARGE                    0xD9
#define SSD1306_SETMULTIPLEX                    0xA8
#define SSD1306_SETLOWCOLUMN                    0x00
#define SSD1306_SETHIGHCOLUMN                   0x10
#define SSD1306_SETSTARTLINE                    0x40
#define SSD1306_MEMORYMODE                      0x20
#define SSD1306_COLUMNADDR                      0x21
#define SSD1306_PAGEADDR                        0x22
#define SSD1306_COMSCANINC                      0xC0
#define SSD1306_COMSCANDEC                      0xC8
#define SSD1306_SEGREMAP                        0xA0
#define SSD1306_CHARGEPUMP                      0x8D
#define SSD1306_EXTERNALVCC                     0x01
#define SSD1306_SWITCHCAPVCC                    0x02
#define SSD1306_DEACTIVATE_SCROLL               0x2E

/*****************************************************************************/
/*                         Static global vars                                */
/*****************************************************************************/
static uint8_t display_buff[SSD1306_BUFF_SIZE + CONTROL_BYTE_SIZE];
static uint8_t *mp_display_buff = display_buff + CONTROL_BYTE_SIZE;

/*****************************************************************************/
/*                         Static function                                   */
/*****************************************************************************/

/* Init sequence for 128x32 OLED module */
const uint8_t init_sequence[] PROGMEM = {
    SSD1306_CONTROL_BYTE_COMS,
    SSD1306_DISPLAYOFF,         //display off
    SSD1306_MEMORYMODE,         //Set Memory Addressing Mode   
    0x00,                       //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    0xB0,                       //Set Page Start Address for Page Addressing Mode,0-7
    0xC8,                       //Set COM Output Scan Direction
    0x00,                       //---set low column address
    0x10,                       //---set high column address
    SSD1306_SETSTARTLINE,       //--set start line address
    SSD1306_SETCONTRAST,        //--set contrast control register
    0x7F,                       //
    0xA1,                       //--set segment re-map 0 to 127
    SSD1306_NORMALDISPLAY,      //--set normal display
    SSD1306_SETMULTIPLEX,       //--set multiplex ratio(1 to 64)
    0x3F,                       //
    SSD1306_DISPLAYALLON_RESUME,//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    SSD1306_SETDISPLAYOFFSET,   //-set display offset
    0x00,                       //-not offset
    SSD1306_SETDISPLAYCLOCKDIV, //--set display clock divide ratio/oscillator frequency
    0xF0,                       //--set divide ratio
    SSD1306_SETPRECHARGE,       //--set pre-charge period
    0x22,                       //
    SSD1306_SETCOMPINS,         //--set com pins hardware configuration
    0x02,                       //
    SSD1306_SETVCOMDETECT,      //--set vcomh
    0x20,                       //0x20,0.77xVcc
    SSD1306_CHARGEPUMP,         //--set DC-DC enable
    0x14,                       //
    SSD1306_DISPLAYON           //
};


/*****************************************************************************/
/*                         Public API                                        */
/*****************************************************************************/
void ssd1306_init(void)
{
	if (twi_initialized() != ERROR_SUCCESS)
	{
		twi_init(TWI_SCL_100KHZ, NULL, NULL);
	}

	/* A little delay */
    uint32_t delay = 0xFFFFFFFF;
    while(delay--);


    uint8_t commandSequence[sizeof(init_sequence)];
    for (uint8_t i = 0; i < sizeof (init_sequence); i++)
    {
        commandSequence[i] = (pgm_read_byte(&init_sequence[i]));
    }

    SSD1306_WRITE(commandSequence, sizeof(commandSequence));
    ssd1306_update();	
}

void ssd1306_update(void)
{    
    display_buff[0] = SSD1306_CONTROL_BYTE_DATS;
    SSD1306_WRITE(display_buff, sizeof(display_buff));
}

void ssd1306_draw_pixel(uint8_t x, uint8_t y)
{
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT)
    {
        return;
    }
    
    mp_display_buff[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
}

void ssd1306_line_v(uint8_t x, uint8_t y, uint8_t height)
{
    for (uint8_t i = 0; i < height; ++i)
    {
        ssd1306_draw_pixel(x, y + i);
    }
}
void ssd1306_line_h(uint8_t x, uint8_t y, uint8_t width)
{
    for (uint8_t i = 0; i < width; ++i)
    {
        ssd1306_draw_pixel(x + i, y);
    }
}
void ssd1306_rect(uint8_t x, uint8_t y, uint8_t width, uint8_t height)
{
    ssd1306_line_h(x, y, width);
    ssd1306_line_v(x, y, height);
    ssd1306_line_v(x + width-1, y, height);
    ssd1306_line_h(x, y + height-1, width);
}

void ssd1306_putc(uint8_t x, uint8_t y, char c)
{
    char index = c - ' ';
    if((index < 0) || (index > '~' - ' '))
    {
        return;
    }

    uint8_t map[CHAR_WIDTH];
    char_map_get(index, map);
    for (uint8_t i = 0; i < CHAR_WIDTH; ++i)
    {
        mp_display_buff[(x + i) + (y / 8) * SSD1306_WIDTH] = map[i];
    }
}
#include "ws2812b.h"

#if defined (WS2812_USE_SPI)
	#if defined(SPI_ENABLED)
		#error "SPI has already been used."
	#else
			
	#endif
#else
	#error "The software ws2812b protocol doesn't implemented."
#endif /* !defined(SPI_ENABLED) && defined (WS2812_USE_SPI) */

#define DDR_SPI (DDRB)
#define DP_MOSI (3)

void ws2812b_init(void)
{
	DDR_SPI = (1 << DP_MOSI)
}
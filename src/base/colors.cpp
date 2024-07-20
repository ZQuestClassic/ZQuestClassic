#include "base/colors.h"

byte nes_pal[] =
{
	31,31,31,                                                 //  0
	0, 0,63,                                                  //  1
	0, 0,47,                                                  //  2
	17,10,47,                                                 //  3
	37, 0,33,                                                 //  4
	42, 0, 8,                                                 //  5
	42, 4, 0,                                                 //  6
	34, 5, 0,                                                 //  7
	20,12, 0,                                                 //  8
	0,30, 0,                                                  //  9
	0,26, 0,                                                  // 10
	0,22, 0,                                                  // 11
	0,16,22,                                                  // 12
	0, 0, 0,                                                  // 13 0D
	0, 0, 0,                                                  // 14 0E
	0, 0, 0,                                                  // 15 0F
	47,47,47,                                                 // 16
	0,30,62,                                                  // 17
	0,22,62,                                                  // 18
	26,17,63,                                                 // 19
	54, 0,51,                                                 // 20
	57, 0,22,                                                 // 21
	62,14, 0,                                                 // 22
	57,23, 4,                                                 // 23
	43,31, 0,                                                 // 24
	0,46, 0,                                                  // 25
	0,42, 0,                                                  // 26
	0,42,17,                                                  // 27
	0,34,34,                                                  // 28
	0, 0, 0,                                                  // 29
	0, 0, 0,                                                  // 30
	0, 0, 0,                                                  // 31
	62,62,62,                                                 // 32
	15,47,63,                                                 // 33
	26,34,63,                                                 // 34
	38,30,62,                                                 // 35
	62,30,62,                                                 // 36
	62,22,38,                                                 // 37
	62,30,22,                                                 // 38
	63,40,17,                                                 // 39
	62,46, 0,                                                 // 40
	46,62, 6,                                                 // 41
	22,54,21,                                                 // 42
	22,62,38,                                                 // 43
	0,58,54,                                                  // 44
	30,30,30,                                                 // 45
	0, 0, 0,                                                  // 46
	0, 0, 0,                                                  // 47
	63,63,63,                                                 // 48 !
	41,57,63,                                                 // 49
	52,52,62,                                                 // 50
	54,46,62,                                                 // 51
	62,46,62,                                                 // 52
	62,41,48,                                                 // 53
	60,52,44,                                                 // 54
	63,56,42,                                                 // 55
	62,54,30,                                                 // 56
	54,62,30,                                                 // 57
	46,62,46,                                                 // 58
	46,62,54,                                                 // 59
	0,63,63,                                                  // 60
	62,54,62,                                                 // 61
	0,54,50,                                                  // 62
	31,63,63                                                  // 63
};

// Global pointer to current color data used by "pal.cc" routines.
// Must be allocated to pdTOTAL*16 bytes before calling init_colordata().
extern byte *colordata;

/* create_zc_trans_table:
  *  Constructs a translucency color table for the specified palette. The
  *  r, g, and b parameters specifiy the solidity of each color component,
  *  ranging from 0 (totally transparent) to 255 (totally solid). If the
  *  callback function is not NULL, it will be called 256 times during the
  *  calculation, allowing you to display a progress indicator.
  */
void create_zc_trans_table(COLOR_MAP *table, AL_CONST PALETTE pal_8bit, int32_t r, int32_t g, int32_t b)
{
	int32_t tmp[768], *q;
	int32_t x, y, i, j, k;
	uint8_t *p;
	RGB c;

	// Allegro has been modified to use an 8 bit palette, but this method still uses 6 bit.
	PALETTE pal;
	for (int i = 0; i < 256; i++)
	{
		pal[i] = pal_8bit[i];
		pal[i].r /= 4;
		pal[i].g /= 4;
		pal[i].b /= 4;
	}
	
	for(x=0; x<256; x++)
	{
		tmp[x*3]   = pal[x].r * (255-r) / 255;
		tmp[x*3+1] = pal[x].g * (255-g) / 255;
		tmp[x*3+2] = pal[x].b * (255-b) / 255;
	}
	
	for(x=0; x<PAL_SIZE; x++)
	{
		i = pal[x].r * r / 255;
		j = pal[x].g * g / 255;
		k = pal[x].b * b / 255;
		
		p = table->data[x];
		q = tmp;
		
		if(rgb_map)
		{
			for(y=0; y<PAL_SIZE; y++)
			{
				c.r = i + *(q++);
				c.g = j + *(q++);
				c.b = k + *(q++);
				p[y] = rgb_map->data[c.r>>1][c.g>>1][c.b>>1];
			}
		}
		else
		{
			for(y=0; y<PAL_SIZE; y++)
			{
				c.r = i + *(q++);
				c.g = j + *(q++);
				c.b = k + *(q++);
				p[y] = bestfit_color(pal, c.r, c.g, c.b);
			}
		}
	}
}

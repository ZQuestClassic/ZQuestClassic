#ifndef _FONTS_H_
#define _FONTS_H_
#include "base/zdefs.h"

extern ALLEGRO_FONT* a5font;
extern FONT* a4fonts[font_max];
extern ALLEGRO_FONT* a5fonts[font_max];
void initFonts();
FONT* get_zc_font(int index);
ALLEGRO_FONT* get_zc_font_a5(int32_t index);
char const* get_zc_fontname(int32_t index);
bool isBrokenFont(int32_t index);
void init_custom_fonts();

enum
{
	CFONT_DLG,
	CFONT_GUI,
	CFONT_TITLE,
	CFONT_FAVCMD,
	CFONT_TEXTBOX,
	CFONT_TTIP,
	CFONT_INFO,
	CFONT_MAX
};
FONT* get_custom_font(int cfont);
ALLEGRO_FONT* get_custom_font_a5(int cfont);
#endif


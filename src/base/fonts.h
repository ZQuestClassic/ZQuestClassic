#ifndef _FONTS_H_
#define _FONTS_H_
#include "base/zdefs.h"

extern FONT *nfont, *nfont2, *zfont, *z3font, *z3smallfont, *deffont, *lfont, *lfont_l, *pfont, *mfont, *ztfont, *sfont, *sfont2, *sfont3, *spfont, *ssfont1, *ssfont2, *ssfont3, *ssfont4, *gblafont,
		*goronfont, *zoranfont, *hylian1font, *hylian2font, *hylian3font, *hylian4font, *gboraclefont, *gboraclepfont, *dsphantomfont, *dsphantompfont,
		//New fonts for 2.54+
		*atari800font, *acornfont, *adosfont, *baseallegrofont, *apple2font, *apple280colfont, *apple2gsfont,
		*aquariusfont, *atari400font, *c64font, *c64hiresfont, *cgafont, *cocofont, *coco2font,
		*coupefont, *cpcfont, *fantasyfont, *fdskanafont, *fdslikefont, *fdsromanfont, *finalffont,
		*futharkfont, *gaiafont, *hirafont, *jpfont, *kongfont, *manafont, *mlfont, *motfont,
		*msxmode0font, *msxmode1font, *petfont, *pstartfont, *saturnfont, *scififont, *sherwoodfont,
		*sinqlfont, *spectrumfont, *speclgfont, *ti99font, *trsfont, *z2font, *zxfont, *lisafont;
extern ALLEGRO_FONT* a5font;
extern ALLEGRO_FONT* a5fonts[font_max];
void initFonts();
FONT* get_zc_font(int32_t index);
ALLEGRO_FONT* get_zc_font_a5(int32_t index);
char const* get_zc_fontname(int32_t index);
bool isBrokenFont(int32_t index);
void init_custom_fonts();

enum
{
	CFONT_DLG,
	CFONT_TITLE,
	CFONT_FAVCMD,
	CFONT_GUI,
	CFONT_TEXTBOX,
	CFONT_TTIP,
	CFONT_INFO,
	CFONT_MAX
};
FONT* get_custom_font(int cfont);
ALLEGRO_FONT* get_custom_font_a5(int cfont);
#endif


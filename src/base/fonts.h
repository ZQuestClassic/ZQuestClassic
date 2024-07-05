#ifndef FONTS_H_
#define FONTS_H_

#include "base/zc_alleg.h"
#include "base/ints.h"
#include "base/headers.h"

enum
{
	font_zfont, /* 24, 32, 26, 5 */
	font_z3font,
	font_z3smallfont,
	font_deffont,
	font_lfont,
	font_lfont_l,
	font_pfont,
	font_mfont,
	font_ztfont,
	font_sfont,
	font_sfont2,
	font_spfont,
	font_ssfont1,
	font_ssfont2,
	font_ssfont3,
	font_ssfont4,
	font_gblafont,
	font_goronfont,
	font_zoranfont,
	font_hylian1font,
	font_hylian2font,
	font_hylian3font,
	font_hylian4font,
	font_gboraclefont,
	font_gboraclepfont,
	font_dsphantomfont,
	font_dsphantompfont,

	font_atari800font,
	font_acornfont,
	font_adosfont,
	font_baseallegrofont,
	font_apple2font,
	font_apple280colfont,
	font_apple2gsfont,
	font_aquariusfont,
	font_atari400font,
	font_c64font,
	font_c64hiresfont,
	font_cgafont,
	font_cocofont,
	font_coco2font,
	font_coupefont,
	font_cpcfont,
	font_fantasyfont,
	font_fdskanafont,
	font_fdslikefont,
	font_fdsromanfont,
	font_finalffont,
	font_futharkfont,
	font_gaiafont,
	font_hirafont,
	font_jpfont,
	font_kongfont,
	font_manafont,
	font_mlfont,
	font_motfont,
	font_msxmode0font,
	font_msxmode1font,
	font_petfont,
	font_pstartfont,
	font_saturnfont,
	font_scififont,
	font_sherwoodfont,
	font_sinqlfont,
	font_spectrumfont,
	font_speclgfont,
	font_ti99font,
	font_trsfont,
	font_z2font,
	font_zxfont,
	font_lisafont,
	font_nfont,
	font_sfont3,
	font_cv3,
	font_ctrig,
	font_nfont2,
	font_bak,
	font_gunstar,
	font_smw_credits,
	font_wl4,
	font_bsz,
	font_bsz_prop,
	font_ff6,
	font_evo_eden,
	font_smt,
	font_actraiser,
	font_bak_runes,
	font_bak_small,
	font_disorient,
	font_doom,
	font_dracula,
	font_ejim,
	font_fallout,
	font_gradius,
	font_lamu_msx,
	font_megaman,
	font_wingdings,
	font_pkmn2,
	font_smrpg,
	font_undertale,
	font_smw,

	font_max
};

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
	FONTPREF_PLAYER,
	FONTPREF_COMPACT,
	FONTPREF_LARGE,
	NUM_FONTPREFS
};
uint get_font_prefid();
string const& get_font_prefix(optional<uint> prefid = nullopt);
string get_font_cfgname(bool scale, uint indx, optional<uint> prefid = nullopt);
int get_def_fontid(uint indx);

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


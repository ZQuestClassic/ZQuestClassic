
#include "base/fonts.h"
#include "fontsdat.h"
#include "base/zapp.h"
#include "base/zsys.h"
#include "drawing.h"
#include <fmt/format.h>
extern DATAFILE *fontsdata;
extern bool is_compact;
using namespace util;

std::string msgfont_str[font_max];

const char *font_output_strs[] =
{
	" !\"#$%&'()*+,-./",
	"0123456789:;<=>?",
	"@ABCDEFGHIJKLMNO",
	"PQRSTUVWXYZ[\\]^_",
	"`abcdefghijklmno",
	"pqrstuvwxyz{|}~"
};

FONT* a4fonts[font_max] = {nullptr};
FONT* customfonts[CFONT_MAX] = {nullptr};
FONT* deffonts[CFONT_MAX] = {nullptr};
int fontscales[CFONT_MAX] = {0};
ALLEGRO_FONT* a5fonts[font_max] = {nullptr};
ALLEGRO_FONT* customfonts_a5[CFONT_MAX] = {nullptr};
ALLEGRO_FONT* deffonts_a5[CFONT_MAX] = {nullptr};
ALLEGRO_FONT* a5font = nullptr;

FONT *get_zc_font(int index)
{
	if(unsigned(index) >= font_max)
		index = font_zfont;
	return a4fonts[index];
}

ALLEGRO_FONT* get_zc_font_a5(int32_t index)
{
	if(unsigned(index) >= font_max)
		index = font_zfont;
	return a5fonts[index];
}

char const* get_zc_fontname(int32_t index)
{
	if(unsigned(index) >= font_max) return "Unknown Font?";
	return msgfont_str[index].c_str();
}

//Illegible / incomplete / non-english fonts
bool isBrokenFont(int32_t index)
{
	switch(index)
	{
		case font_ztfont:
		case font_ssfont1:
		case font_ssfont2:
		case font_ssfont4:
		case font_goronfont:
		case font_zoranfont:
    	case font_hylian1font:
    	case font_hylian2font:
    	case font_hylian3font:
    	case font_hylian4font:
    	case font_cocofont:
    	case font_fdskanafont:
    	case font_futharkfont:
    	case font_hirafont:
    	case font_jpfont:
			return true;
	}
	return false;
}

static PALETTE fontpal;
static bool did_init_fontpal;
void init_fontpal()
{
	if(did_init_fontpal) return;
	
	memset(fontpal, 32, sizeof(PALETTE));
	fontpal[0].r = 63;
	fontpal[0].g = 0;
	fontpal[0].b = 63;
	fontpal[1].r = 63;
	fontpal[1].g = 63;
	fontpal[1].b = 63;
	fontpal[255].r = 0;
	fontpal[255].g = 0;
	fontpal[255].b = 0;
	
	did_init_fontpal = true;
}
BITMAP* get_font_bitmap(FONT* savefont)
{
	init_fontpal();
	
	int len = 0;
	for(auto* c : font_output_strs)
	{
		int nl = text_length(savefont, c);
		if(nl > len)
			len = nl;
	}
	int spacing = 1;
	len += spacing*17;
	
	int fh = text_height(savefont);
	int hei = (spacing*7)+(fh*6);
	
	BITMAP* bmp = create_bitmap_ex(8, len, hei);
	clear_to_color(bmp, 255);
	for(int row = 0; row < 6; ++row)
	{
		int y = spacing + (row*(fh+spacing));
		int x = spacing;
		for(int col = 0; col < 16; ++col)
		{
			char bf[] = {font_output_strs[row][col], 0};
			
			textout_ex(bmp, savefont, bf, x, y, 1, 0);
			
			x += text_length(savefont, bf)+spacing;
		}
	}
	return bmp;
}
void save_font(char const* path, int fontid)
{
	BITMAP* bmp = get_font_bitmap(get_zc_font(fontid));
	save_bitmap(path, bmp, fontpal);
	destroy_bitmap(bmp);
}

int fcolor_rgb(RGB col)
{
	int diff_0 = (63-col.r)+col.g+(63-col.b);
	int diff_1 = (63-col.r)+(63-col.g)+(63-col.b);
	int diff_255 = col.r+col.g+col.b;
	if(diff_0 < diff_1)
	{
		if(diff_0 < diff_255)
			return 0;
		return 255;
	}
	if(diff_1 < diff_255)
		return 1;
	return 255;
}
BITMAP* __load_fontpal_bmp(char const* path)
{
	PALETTE pal;
	BITMAP* bmp = load_bitmap(path,pal);
	std::vector<byte> scolors = getColors(bmp,3);
	std::vector<byte> dcolors;
	for(byte c : scolors)
		dcolors.push_back(fcolor_rgb(pal[c]));
	replColors(bmp,scolors,dcolors);
	return bmp;
}

FONT* __load_a4_font(BITMAP* bmp)
{
	PALETTE oldpal;
	get_palette(oldpal);
	
	init_fontpal();
	zc_set_palette(fontpal);
	
	FONT* newfont = grab_font_from_bitmap(bmp);
	
	zc_set_palette(oldpal);
	return newfont;
}
FONT* __load_a4_font(char const* path)
{
	BITMAP* bmp = __load_fontpal_bmp(path);
	
	FONT* newfont = __load_a4_font(bmp);
	
	destroy_bitmap(bmp);
	return newfont;
}
ALLEGRO_FONT* __load_a5_font(BITMAP* bmp)
{
	PALETTE oldpal;
	get_palette(oldpal);
	
	init_fontpal();
	zc_set_palette(fontpal);
	
	all_set_transparent_palette_index(0);
	ALLEGRO_BITMAP* a5bmp = all_get_a5_bitmap(bmp);
	
	int ranges[] = {32, 126}; //space to tilde
	ALLEGRO_FONT* a5font = al_grab_font_from_bitmap(a5bmp, 1, ranges);
	
	al_destroy_bitmap(a5bmp);
	zc_set_palette(oldpal);
	return a5font;
}
ALLEGRO_FONT* __load_a5_font(char const* path)
{
	BITMAP* bmp = __load_fontpal_bmp(path);
	
	ALLEGRO_FONT* a5font = __load_a5_font(bmp);
	
	destroy_bitmap(bmp);
	return a5font;
}
static int fontsdat_end = 0;
static void initfont(int fontind, int font_dat_ind, char const* str)
{
	msgfont_str[fontind] = str;
	if(font_dat_ind >= fontsdat_end) return; //invalid access
	a4fonts[fontind] = (FONT*)fontsdata[font_dat_ind].dat;
}
static void initfont(int fontind, FONT* f, char const* str)
{
	a4fonts[fontind] = f;
	msgfont_str[fontind] = str;
}
void initFonts()
{
	for(int q = 0; q < font_max; ++q)
	{
		msgfont_str[q].clear();
		a4fonts[q] = nullptr;
		if(a5fonts[q])
			al_destroy_font(a5fonts[q]);
		a5fonts[q] = nullptr;
	}
	for(int q = 0;;++q)
	{
		if(fontsdata[q].type == DAT_END)
		{
			fontsdat_end = q;
			break;
		}
	}
	initfont(font_deffont, font, "Allegro Default");
	initfont(font_nfont, FONT_GUI_PROP, "NFont");
	initfont(font_nfont2, FONT_GUI_MONO, "NFont2");
	initfont(font_pfont, FONT_8xPROP_THIN, "GUI Font Narrow");
	initfont(font_lfont, FONT_LARGEPROP, "GUI Font Bold");
	initfont(font_lfont_l, FONT_LARGEPROP_L, "GUI Font");
	initfont(font_zfont, FONT_NES, "Zelda NES");
	initfont(font_z3font, FONT_Z3, "Link to the Past");
	initfont(font_z3smallfont, FONT_Z3SMALL, "LttP Small");
	initfont(font_mfont, FONT_MATRIX, "Zelda NES (Matrix)");
	initfont(font_ztfont, FONT_ZTIME, "BS Time (Incomplete)");
	initfont(font_sfont, FONT_6x6, "Small");
	initfont(font_sfont2, FONT_6x4, "Small 2");
	initfont(font_sfont3, FONT_12x8, "Small 3");
	initfont(font_spfont, FONT_6xPROP, "S. Proportional");
	initfont(font_ssfont1, FONT_SUBSCREEN1, "SS 1 (Numerals)");
	initfont(font_ssfont2, FONT_SUBSCREEN2, "SS 2 (Incomplete)");
	initfont(font_ssfont3, FONT_SUBSCREEN3, "SS 3");
	initfont(font_ssfont4, FONT_SUBSCREEN4, "SS 4 (Numerals)");
	initfont(font_gblafont, FONT_GB_LA, "Link's Awakening");
	initfont(font_goronfont, FONT_GORON, "Goron");
	initfont(font_zoranfont, FONT_ZORAN, "Zoran");
	initfont(font_hylian1font, FONT_HYLIAN1, "Hylian 1");
	initfont(font_hylian2font, FONT_HYLIAN2, "Hylian 2");
	initfont(font_hylian3font, FONT_HYLIAN3, "Hylian 3");
	initfont(font_hylian4font, FONT_HYLIAN4, "Hylian 4");
	initfont(font_gboraclefont, FONT_GB_ORACLE, "Oracle");
	initfont(font_gboraclepfont, FONT_GB_ORACLE_P, "Oracle Proportional");
	initfont(font_dsphantomfont, FONT_DS_PHANTOM, "Phantom");
	initfont(font_dsphantompfont, FONT_DS_PHANTOM_P, "Phantom Proportional");
	initfont(font_atari800font, FONT_A80080C, "Atari 800");
	initfont(font_acornfont, FONT_ACORN, "Acorn");
	initfont(font_adosfont, FONT_ADOS, "ADOS");
	initfont(font_baseallegrofont, FONT_ALLEGRO, "Allegro");
	initfont(font_apple2font, FONT_APPLE2, "Apple II");
	initfont(font_apple280colfont, FONT_APPLE280, "Apple II 80 Column");
	initfont(font_apple2gsfont, FONT_APPLE2GS, "Apple IIgs");
	initfont(font_aquariusfont, FONT_AQUA, "Aquarius");
	initfont(font_atari400font, FONT_ATARI400, "Atari 400");
	initfont(font_c64font, FONT_C64, "C64");
	initfont(font_c64hiresfont, FONT_C64HR, "C64 HiRes");
	initfont(font_cgafont, FONT_CGA, "IBM CGA");
	initfont(font_cocofont, FONT_COCO, "COCO Mode I");
	initfont(font_coco2font, FONT_COCO2, "COCO Mode II");
	initfont(font_coupefont, FONT_COUPE, "Coupe");
	initfont(font_cpcfont, FONT_CPC, "Amstrad CPC");
	initfont(font_fantasyfont, FONT_FANTASY, "Fantasy Letters");
	initfont(font_fdskanafont, FONT_FDS_KANA, "FDS Katakana");
	initfont(font_fdslikefont, FONT_FDSLIKE, "FDSesque");
	initfont(font_fdsromanfont, FONT_FDSROMAN, "FDS Roman");
	initfont(font_finalffont, FONT_FF, "FF");
	initfont(font_futharkfont, FONT_FUTHARK, "Elder Futhark");
	initfont(font_gaiafont, FONT_GAIA, "Gaia");
	initfont(font_hirafont, FONT_HIRA, "Hira");
	initfont(font_jpfont, FONT_JP, "JP Unsorted");
	initfont(font_kongfont, FONT_KONG, "Kong");
	initfont(font_manafont, FONT_MANA, "Mana");
	initfont(font_mlfont, FONT_MARIOLAND, "Mario");
	initfont(font_motfont, FONT_MOT, "Mot CPU");
	initfont(font_msxmode0font, FONT_MSX0, "MSX Mode 0");
	initfont(font_msxmode1font, FONT_MSX1, "MSX Mode 1");
	initfont(font_petfont, FONT_PET, "PET");
	initfont(font_pstartfont, FONT_PRESTRT, "Homebrew");
	initfont(font_saturnfont, FONT_SATURN, "Mr. Saturn");
	initfont(font_scififont, FONT_SCIFI, "Sci-Fi");
	initfont(font_sherwoodfont, FONT_SHERWOOD, "Sherwood");
	initfont(font_sinqlfont, FONT_SINQL, "Sinclair QL");
	initfont(font_spectrumfont, FONT_SPEC, "Spectrum");
	initfont(font_speclgfont, FONT_SPECLG, "Spectrum Large");
	initfont(font_ti99font, FONT_TI99, "TI99");
	initfont(font_trsfont, FONT_TRS, "TRS");
	initfont(font_z2font, FONT_Z2, "Zelda 2");
	initfont(font_zxfont, FONT_ZX, "ZX");
	initfont(font_lisafont, FONT_LISA, "Lisa");
	initfont(font_cv3, FONT_CV3, "CV 3");
	initfont(font_ctrig, FONT_CTRIG, "Chrono");
	initfont(font_bak, FONT_BAK, "Betrayal at Krondor");
	initfont(font_gunstar, FONT_GUNSTAR, "Gunstar Heroes");
	initfont(font_smw_credits, FONT_SMW_CREDITS, "SMW Credits");
	initfont(font_wl4, FONT_WL4, "WLand 4");
	initfont(font_bsz, FONT_BSZ, "BSZ");
	initfont(font_bsz_prop, FONT_BSZ_PROP, "BSZ Proportional");
	initfont(font_ff6, FONT_FF6, "FF6");
	initfont(font_evo_eden, FONT_EVO_EDEN, "EVO Search for Eden");
	initfont(font_smt, FONT_SMT, "SMT Font");
	
	font = a4fonts[font_nfont];
	
	memset(customfonts, 0, sizeof(customfonts));
	memset(customfonts_a5, 0, sizeof(customfonts));
	bool fontfail = false;
	for(int q = 0; q < font_max; ++q)
	{
		if(!a4fonts[q])
		{
			Z_error("Failed to load font '%s'!\n", msgfont_str[q].c_str());
			fontfail = true;
			continue;
		}
		BITMAP* bmp = get_font_bitmap(get_zc_font(q));
		a5fonts[q] = __load_a5_font(bmp);
		destroy_bitmap(bmp);
	}
	a5font = a5fonts[font_nfont];
	
	if(fontfail)
	{
		Z_error_fatal("Failed to load fonts.dat!\n");
	}
	
	init_custom_fonts();
}

FONT* load_cfont(char const* name)
{
	char path[512];
	char pref[16];
	
	if(is_compact)
		strcpy(pref, "compact");
	else
		strcpy(pref, "large");
	
	sprintf(path, "customfonts/%s_%s.bmp", pref, name);
	
	if(!exists(path))
		return nullptr;
	FONT* f = nullptr;
	try
	{
		f = __load_a4_font(path);
	} catch(std::exception){}
	if(!f)
		zprint2("Error loading font: '%s'\n", path);
	return f;
}

ALLEGRO_FONT* load_cfont_a5(char const* name)
{
	char path[512];
	char pref[16];
	
	if(is_compact)
		strcpy(pref, "compact");
	else
		strcpy(pref, "large");
	
	sprintf(path, "customfonts/%s_%s.bmp", pref, name);
	
	if(!exists(path))
		return nullptr;
	ALLEGRO_FONT* f = nullptr;
	try
	{
		f = __load_a5_font(path);
	} catch(std::exception){}
	if(!f)
		zprint2("Error loading a5 font: '%s'\n", path);
	return f;
}

FONT* scale_font(FONT* f, int scale)
{
	BITMAP* bmp = get_font_bitmap(f);
	BITMAP* scaledbmp = create_bitmap_ex(8,bmp->w*scale,bmp->h*scale);
	stretch_blit(bmp, scaledbmp, 0, 0, bmp->w, bmp->h, 0, 0, scaledbmp->w, scaledbmp->h);
	FONT* newfont = __load_a4_font(scaledbmp);
	destroy_bitmap(bmp);
	destroy_bitmap(scaledbmp);
	return newfont;
}

ALLEGRO_FONT* scale_font_a5(FONT* f, int scale)
{
	BITMAP* bmp = get_font_bitmap(f);
	BITMAP* scaledbmp = create_bitmap_ex(8,bmp->w*scale,bmp->h*scale);
	stretch_blit(bmp, scaledbmp, 0, 0, bmp->w, bmp->h, 0, 0, scaledbmp->w, scaledbmp->h);
	ALLEGRO_FONT* newfont = __load_a5_font(scaledbmp);
	destroy_bitmap(bmp);
	destroy_bitmap(scaledbmp);
	return newfont;
}

int dlgfontheight;
void init_custom_fonts()
{
	font = a4fonts[font_nfont];
	
	char pref[16];
	if(is_compact)
		strcpy(pref, "compact");
	else
		strcpy(pref, "large");
	
	char buf[512];
	int deffont_ids[CFONT_MAX] = {font_lfont_l,font_lfont,font_pfont,font_nfont,font_sfont3,font_lfont,font_lfont_l};
	char const* _font_titles[CFONT_MAX] = {"dialog", "gui", "title", "favcmd", "textbox", "ttip", "info"};
	bool use_custom_fonts = zc_get_config("gui","custom_fonts",1, App::zquest);
	for(int q = 0; q < CFONT_MAX; ++q)
	{
		sprintf(buf, "font_%s_%s", pref, _font_titles[q]);
		deffont_ids[q] = zc_get_config("ZQ_GUI", buf, deffont_ids[q], App::zquest);
		sprintf(buf, "fontscale_%s_%s", pref, _font_titles[q]);
		fontscales[q] = zc_get_config("ZQ_GUI", buf, 1, App::zquest);
		if(unsigned(deffont_ids[q]) >= font_max)
			deffont_ids[q] = font_lfont_l;
		deffonts[q] = get_zc_font(deffont_ids[q]);
		deffonts_a5[q] = a5fonts[deffont_ids[q]];
		if(customfonts[q])
		{
			destroy_font(customfonts[q]);
			customfonts[q] = nullptr;
		}
		if(customfonts_a5[q])
		{
			al_destroy_font(customfonts_a5[q]);
			customfonts_a5[q] = nullptr;
		}
		if(use_custom_fonts)
		{
			customfonts[q] = load_cfont(_font_titles[q]);
			customfonts_a5[q] = load_cfont_a5(_font_titles[q]);
		}
		
		//If a customfonts slot is empty, use it to scale a normal font, if needed.
		if(customfonts[q] || customfonts_a5[q]) continue;
		if(fontscales[q] <= 1) continue;
		customfonts[q] = scale_font(deffonts[q], fontscales[q]);
		customfonts_a5[q] = scale_font_a5(deffonts[q], fontscales[q]);
	}
	dlgfontheight = text_height(get_custom_font(CFONT_DLG));
}

FONT* get_custom_font(int cfont)
{
	if(unsigned(cfont) >= CFONT_MAX)
		return a4fonts[font_lfont_l];
	if(customfonts[cfont])
		return customfonts[cfont];
	return deffonts[cfont];
}
ALLEGRO_FONT* get_custom_font_a5(int cfont)
{
	if(unsigned(cfont) >= CFONT_MAX)
		return a5fonts[font_lfont_l];
	if(zc_get_config("gui","custom_fonts",1,App::zquest) && customfonts_a5[cfont])
		return customfonts_a5[cfont];
	return deffonts_a5[cfont];
}


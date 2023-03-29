
#include "base/fonts.h"
#include "fontsdat.h"
#include "base/zapp.h"
#include "drawing.h"
#include <fmt/format.h>
extern DATAFILE *fontsdata;
extern bool is_compact;
using namespace util;

const char *msgfont_str[font_max] =
{
	"Zelda NES", "Link to the Past", "LttP Small", "Allegro Default", "GUI Font Bold", "GUI Font", "GUI Font Narrow", "Zelda NES (Matrix)", "BS Time (Incomplete)", "Small", "Small 2",
	"S. Proportional", "SS 1 (Numerals)", "SS 2 (Incomplete)", "SS 3", "SS 4 (Numerals)", "Link's Awakening", "Goron", "Zoran", "Hylian 1", "Hylian 2",
	"Hylian 3", "Hylian 4", "Oracle", "Oracle Proportional", "Phantom", "Phantom Proportional",
	"Atari 800", 
	"Acorn",
	"ADOS",
	"Allegro",
	"Apple II",
	"Apple II 80 Column",
	"Apple IIgs",
	"Aquarius",
	"Atari 400",
	"C64",
	"C64 HiRes",
	"IBM CGA",
	"COCO Mode I",
	"COCO Mode II",
	"Coupe",
	"Amstrad CPC",
	"Fantasy Letters",
	"FDS Katakana",
	"FDSesque",
	"FDS Roman",
	"FF",
	"Elder Futhark",
	"Gaia",
	"Hira",
	"JP Unsorted",
	"Kong",
	"Mana",
	"Mario",
	"Mot CPU",
	"MSX Mode 0",
	"MSX Mode 1",
	"PET",
	"Homebrew",
	"Mr. Saturn",
	"Sci-Fi",
	"Sherwood",
	"Sinclair QL",
	"Spectrum",
	"Spectrum Large",
	"TI99",
	"TRS",
	"Zelda 2",
	"ZX",
	"Lisa",
	"nfont",
	"Small 3",
	"CV 3",
	"Chrono"
};

const char *font_output_strs[] =
{
	" !\"#$%&'()*+,-./",
	"0123456789:;<=>?",
	"@ABCDEFGHIJKLMNO",
	"PQRSTUVWXYZ[\\]^_",
	"`abcdefghijklmno",
	"pqrstuvwxyz{|}~"
};

FONT* a4fonts[font_max];
FONT* customfonts[CFONT_MAX];
FONT* deffonts[CFONT_MAX];
int fontscales[CFONT_MAX];
ALLEGRO_FONT* a5fonts[font_max];
ALLEGRO_FONT* customfonts_a5[CFONT_MAX];
ALLEGRO_FONT* deffonts_a5[CFONT_MAX];
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
	return msgfont_str[index];
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
void initFonts()
{
	a4fonts[font_deffont] = font;
	a4fonts[font_nfont] = (FONT*)fontsdata[FONT_GUI_PROP].dat;
	a4fonts[font_nfont2] = (FONT*)fontsdata[FONT_GUI_MONO].dat;
	a4fonts[font_pfont] = (FONT*)fontsdata[FONT_8xPROP_THIN].dat;
	a4fonts[font_lfont] = (FONT*)fontsdata[FONT_LARGEPROP].dat;
	a4fonts[font_lfont_l] = (FONT*)fontsdata[FONT_LARGEPROP_L].dat;
	a4fonts[font_zfont] = (FONT*)fontsdata[FONT_NES].dat;
	a4fonts[font_z3font] = (FONT*)fontsdata[FONT_Z3].dat;
	a4fonts[font_z3smallfont] = (FONT*)fontsdata[FONT_Z3SMALL].dat;
	a4fonts[font_mfont] = (FONT*)fontsdata[FONT_MATRIX].dat;
	a4fonts[font_ztfont] = (FONT*)fontsdata[FONT_ZTIME].dat;
	a4fonts[font_sfont] = (FONT*)fontsdata[FONT_6x6].dat;
	a4fonts[font_sfont2] = (FONT*)fontsdata[FONT_6x4].dat;
	a4fonts[font_sfont3] = (FONT*)fontsdata[FONT_12x8].dat;
	a4fonts[font_spfont] = (FONT*)fontsdata[FONT_6xPROP].dat;
	a4fonts[font_ssfont1] = (FONT*)fontsdata[FONT_SUBSCREEN1].dat;
	a4fonts[font_ssfont2] = (FONT*)fontsdata[FONT_SUBSCREEN2].dat;
	a4fonts[font_ssfont3] = (FONT*)fontsdata[FONT_SUBSCREEN3].dat;
	a4fonts[font_ssfont4] = (FONT*)fontsdata[FONT_SUBSCREEN4].dat;
	a4fonts[font_gblafont] = (FONT*)fontsdata[FONT_GB_LA].dat;
	a4fonts[font_goronfont] = (FONT*)fontsdata[FONT_GORON].dat;
	a4fonts[font_zoranfont] = (FONT*)fontsdata[FONT_ZORAN].dat;
	a4fonts[font_hylian1font] = (FONT*)fontsdata[FONT_HYLIAN1].dat;
	a4fonts[font_hylian2font] = (FONT*)fontsdata[FONT_HYLIAN2].dat;
	a4fonts[font_hylian3font] = (FONT*)fontsdata[FONT_HYLIAN3].dat;
	a4fonts[font_hylian4font] = (FONT*)fontsdata[FONT_HYLIAN4].dat;
	a4fonts[font_gboraclefont] = (FONT*)fontsdata[FONT_GB_ORACLE].dat;
	a4fonts[font_gboraclepfont] = (FONT*)fontsdata[FONT_GB_ORACLE_P].dat;
	a4fonts[font_dsphantomfont] = (FONT*)fontsdata[FONT_DS_PHANTOM].dat;
	a4fonts[font_dsphantompfont] = (FONT*)fontsdata[FONT_DS_PHANTOM_P].dat;
	a4fonts[font_atari800font] = (FONT*)fontsdata[FONT_A80080C].dat;
	a4fonts[font_acornfont] = (FONT*)fontsdata[FONT_ACORN].dat;
	a4fonts[font_adosfont] = (FONT*)fontsdata[FONT_ADOS].dat;
	a4fonts[font_baseallegrofont] = (FONT*)fontsdata[FONT_ALLEGRO].dat;
	a4fonts[font_apple2font] = (FONT*)fontsdata[FONT_APPLE2].dat;
	a4fonts[font_apple280colfont] = (FONT*)fontsdata[FONT_APPLE280].dat;
	a4fonts[font_apple2gsfont] = (FONT*)fontsdata[FONT_APPLE2GS].dat;
	a4fonts[font_aquariusfont] = (FONT*)fontsdata[FONT_AQUA].dat;
	a4fonts[font_atari400font] = (FONT*)fontsdata[FONT_ATARI400].dat;
	a4fonts[font_c64font] = (FONT*)fontsdata[FONT_C64].dat;
	a4fonts[font_c64hiresfont] = (FONT*)fontsdata[FONT_C64HR].dat;
	a4fonts[font_cgafont] = (FONT*)fontsdata[FONT_CGA].dat;
	a4fonts[font_cocofont] = (FONT*)fontsdata[FONT_COCO].dat;
	a4fonts[font_coco2font] = (FONT*)fontsdata[FONT_COCO2].dat;
	a4fonts[font_coupefont] = (FONT*)fontsdata[FONT_COUPE].dat;
	a4fonts[font_cpcfont] = (FONT*)fontsdata[FONT_CPC].dat;
	a4fonts[font_fantasyfont] = (FONT*)fontsdata[FONT_FANTASY].dat;
	a4fonts[font_fdskanafont] = (FONT*)fontsdata[FONT_FDS_KANA].dat;
	a4fonts[font_fdslikefont] = (FONT*)fontsdata[FONT_FDSLIKE].dat;
	a4fonts[font_fdsromanfont] = (FONT*)fontsdata[FONT_FDSROMAN].dat;
	a4fonts[font_finalffont] = (FONT*)fontsdata[FONT_FF].dat;
	a4fonts[font_futharkfont] = (FONT*)fontsdata[FONT_FUTHARK].dat;
	a4fonts[font_gaiafont] = (FONT*)fontsdata[FONT_GAIA].dat;
	a4fonts[font_hirafont] = (FONT*)fontsdata[FONT_HIRA].dat;
	a4fonts[font_jpfont] = (FONT*)fontsdata[FONT_JP].dat;
	a4fonts[font_kongfont] = (FONT*)fontsdata[FONT_KONG].dat;
	a4fonts[font_manafont] = (FONT*)fontsdata[FONT_MANA].dat;
	a4fonts[font_mlfont] = (FONT*)fontsdata[FONT_MARIOLAND].dat;
	a4fonts[font_motfont] = (FONT*)fontsdata[FONT_MOT].dat;
	a4fonts[font_msxmode0font] = (FONT*)fontsdata[FONT_MSX0].dat;
	a4fonts[font_msxmode1font] = (FONT*)fontsdata[FONT_MSX1].dat;
	a4fonts[font_petfont] = (FONT*)fontsdata[FONT_PET].dat;
	a4fonts[font_pstartfont] = (FONT*)fontsdata[FONT_PRESTRT].dat;
	a4fonts[font_saturnfont] = (FONT*)fontsdata[FONT_SATURN].dat;
	a4fonts[font_scififont] = (FONT*)fontsdata[FONT_SCIFI].dat;
	a4fonts[font_sherwoodfont] = (FONT*)fontsdata[FONT_SHERWOOD].dat;
	a4fonts[font_sinqlfont] = (FONT*)fontsdata[FONT_SINQL].dat;
	a4fonts[font_spectrumfont] = (FONT*)fontsdata[FONT_SPEC].dat;
	a4fonts[font_speclgfont] = (FONT*)fontsdata[FONT_SPECLG].dat;
	a4fonts[font_ti99font] = (FONT*)fontsdata[FONT_TI99].dat;
	a4fonts[font_trsfont] = (FONT*)fontsdata[FONT_TRS].dat;
	a4fonts[font_z2font] = (FONT*)fontsdata[FONT_Z2].dat;
	a4fonts[font_zxfont] = (FONT*)fontsdata[FONT_ZX].dat;
	a4fonts[font_lisafont] = (FONT*)fontsdata[FONT_LISA].dat;
	a4fonts[font_cv3] = (FONT*)fontsdata[FONT_CV3].dat;
	a4fonts[font_ctrig] = (FONT*)fontsdata[FONT_CTRIG].dat;
	
	font = a4fonts[font_nfont];
	
	memset(customfonts, 0, sizeof(customfonts));
	memset(customfonts_a5, 0, sizeof(customfonts));
	
	for(int q = 0; q < font_max; ++q)
	{
		BITMAP* bmp = get_font_bitmap(get_zc_font(q));
		a5fonts[q] = __load_a5_font(bmp);
		destroy_bitmap(bmp);
	}
	a5font = a5fonts[font_nfont];
	
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


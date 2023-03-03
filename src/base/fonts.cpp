
#include "base/fonts.h"
#include "fontsdat.h"
#include "base/zapp.h"
#include <fmt/format.h>
extern DATAFILE *fontsdata;
extern bool is_compact;
using namespace util;

FONT    *nfont, *nfont2, *zfont, *z3font, *z3smallfont, *deffont, *lfont, *lfont_l, *pfont, *mfont, *ztfont, *sfont, *sfont2, *sfont3, *spfont, *ssfont1, *ssfont2, *ssfont3, *ssfont4, *gblafont,
		*goronfont, *zoranfont, *hylian1font, *hylian2font, *hylian3font, *hylian4font, *gboraclefont, *gboraclepfont, *dsphantomfont, *dsphantompfont,
		//New fonts for 2.54+
		*atari800font, *acornfont, *adosfont, *baseallegrofont, *apple2font, *apple280colfont, *apple2gsfont,
		*aquariusfont, *atari400font, *c64font, *c64hiresfont, *cgafont, *cocofont, *coco2font,
		*coupefont, *cpcfont, *fantasyfont, *fdskanafont, *fdslikefont, *fdsromanfont, *finalffont,
		*futharkfont, *gaiafont, *hirafont, *jpfont, *kongfont, *manafont, *mlfont, *motfont,
		*msxmode0font, *msxmode1font, *petfont, *pstartfont, *saturnfont, *scififont, *sherwoodfont,
		*sinqlfont, *spectrumfont, *speclgfont, *ti99font, *trsfont, *z2font, *zxfont, *lisafont;
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
	"Small 3"
};

const char *msgfont_int_str[font_max] =
{
	"zfont",
	"z3font",
	"z3smallfont",
	"deffont",
	"lfont",
	"lfont_l",
	"pfont",
	"mfont",
	"ztfont",
	"sfont",
	"sfont2",
	"spfont",
	"ssfont1",
	"ssfont2",
	"ssfont3",
	"ssfont4",
	"gblafont",
	"goronfont",
	"zoranfont",
	"hylian1font",
	"hylian2font",
	"hylian3font",
	"hylian4font",
	"gboraclefont",
	"gboraclepfont",
	"dsphantomfont",
	"dsphantompfont",

	"atari800font",  
	"acornfont",  
	"adosfont",  
	"baseallegrofont",  
	"apple2font",  
	"apple280colfont",  
	"apple2gsfont",
	"aquariusfont",  
	"atari400font",  
	"c64font",  
	"c64hiresfont",  
	"cgafont",  
	"cocofont",
	"coco2font",
	"coupefon",
	"cpcfon",
	"fantasyfon",
	"fdskanafon",
	"fdslikefon",
	"fdsromanfon",
	"finalffont",
	"futharkfont",
	"gaiafont",
	"hirafont",
	"jpfont",
	"kongfont",
	"manafont",
	"mlfont",
	"motfont",
	"msxmode0font",
	"msxmode1font",
	"petfont",
	"pstartfont",
	"saturnfont",
	"scififont",
	"sherwoodfont",
	"sinqlfont",
	"spectrumfont",
	"speclgfont",
	"ti99font",
	"trsfont",
	"z2font",
	"zxfont",
	"lisafont",
	"nfont",
	"sfont3"
};

FONT* customfonts[CFONT_MAX];
FONT* deffonts[CFONT_MAX];
ALLEGRO_FONT* a5fonts[font_max];
ALLEGRO_FONT* customfonts_a5[CFONT_MAX];
ALLEGRO_FONT* deffonts_a5[CFONT_MAX];
ALLEGRO_FONT* a5font = nullptr;

FONT *get_zc_font(int32_t index)
{
    switch(index)
    {
		default:
			return zfont;
			
		case font_z3font:
			return z3font;
			
		case font_z3smallfont:
			return z3smallfont;
			
		case font_deffont:
			return deffont;
			
		case font_lfont:
			return lfont;
			
		case font_lfont_l:
			return lfont_l;
			
		case font_pfont:
			return pfont;
			
		case font_mfont:
			return mfont;
			
		case font_ztfont:
			return ztfont;
			
		case font_sfont:
			return sfont;
			
		case font_sfont2:
			return sfont2;
			
		case font_spfont:
			return spfont;
			
		case font_ssfont1:
			return ssfont1;
			
		case font_ssfont2:
			return ssfont2;
			
		case font_ssfont3:
			return ssfont3;
			
		case font_ssfont4:
			return ssfont4;
			
		case font_gblafont:
			return gblafont;
			
		case font_goronfont:
			return goronfont;
			
		case font_zoranfont:
			return zoranfont;
			
		case font_hylian1font:
			return hylian1font;
			
		case font_hylian2font:
			return hylian2font;
			
		case font_hylian3font:
			return hylian3font;
			
		case font_hylian4font:
			return hylian4font;
			
		case font_gboraclefont:
			return gboraclefont;
			
		case font_gboraclepfont:
			return gboraclepfont;
			
		case font_dsphantomfont:
			return dsphantomfont;
			
		case font_dsphantompfont:
			return dsphantompfont;
		case font_atari800font: return atari800font;
		case font_acornfont: return acornfont;
		case font_adosfont: return adosfont;
		case font_baseallegrofont: return  baseallegrofont;  
		case font_apple2font: return apple2font;
		case font_apple280colfont: return apple280colfont;   
		case font_apple2gsfont: return  apple2gsfont;
		case font_aquariusfont: return  aquariusfont;  
		case font_atari400font: return  atari400font;  
		case font_c64font: return c64font;   
		case font_c64hiresfont: return c64hiresfont;   
		case font_cgafont: return cgafont;   
		case font_cocofont: return cocofont;
		case font_coco2font: return coco2font;
		case font_coupefon: return  coupefont;
		case font_cpcfon: return  cpcfont;
		case font_fantasyfon: return  fantasyfont;
		case font_fdskanafon: return  fdskanafont;
		case font_fdslikefon: return  fdslikefont;
		case font_fdsromanfon: return fdsromanfont; 
		case font_finalffont: return finalffont; 
		case font_futharkfont: return  futharkfont;
		case font_gaiafont: return gaiafont; 
		case font_hirafont: return hirafont; 
		case font_jpfont: return jpfont; 
		case font_kongfont: return  kongfont;
		case font_manafont: return manafont; 
		case font_mlfont: return  mlfont;
		case font_motfont: return motfont;
		case font_msxmode0font: return  msxmode0font;
		case font_msxmode1font: return  msxmode1font;
		case font_petfont: return  petfont;
		case font_pstartfont: return  pstartfont;
		case font_saturnfont: return  saturnfont;
		case font_scififont: return  scififont;
		case font_sherwoodfont: return sherwoodfont;
		case font_sinqlfont: return  sinqlfont;
		case font_spectrumfont: return  spectrumfont;
		case font_speclgfont: return  speclgfont;
		case font_ti99font: return  ti99font;
		case font_trsfont: return  trsfont;
		case font_z2font: return  z2font;
		case font_zxfont: return zxfont;
		case font_lisafont: return lisafont;
		case font_nfont: return nfont;
		case font_sfont3: return sfont3;
    }
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
    	case font_fdskanafon:
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
	char font_output_strs[14][17];
	char c = ' ';
	for(int q = 0; q < 14; ++q)
	{
		for(int ind = 0; ind < 16; ++ind)
		{
			font_output_strs[q][ind] = c++;
		}
		font_output_strs[q][16] = 0;
	}
	for(auto* c : font_output_strs)
	{
		int nl = text_length(savefont, c);
		if(nl > len)
			len = nl;
	}
	int spacing = 1;
	len += spacing*17;
	
	int num_rows = 14;
	int fh = text_height(savefont);
	int hei = (spacing*(num_rows+1))+(fh*num_rows);
	
	BITMAP* bmp = create_bitmap_ex(8, len, hei);
	clear_to_color(bmp, 255);
	for(int row = 0; row < num_rows; ++row)
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
ALLEGRO_FONT* __load_a5_font(BITMAP* bmp)
{
	PALETTE oldpal;
	get_palette(oldpal);
	
	init_fontpal();
	zc_set_palette(fontpal);
	
	all_set_transparent_palette_index(0);
	ALLEGRO_BITMAP* a5bmp = all_get_a5_bitmap(bmp);
	
	int ranges[] = {32, 255}; //space to end
	ALLEGRO_FONT* a5font = al_grab_font_from_bitmap(a5bmp, 1, ranges);
	
	al_destroy_bitmap(a5bmp);
	zc_set_palette(oldpal);
	return a5font;
}
ALLEGRO_FONT* __load_a5_font(char const* path)
{
	init_fontpal();
	
	BITMAP* bmp = load_bitmap(path,fontpal);
	
	ALLEGRO_FONT* a5font = __load_a5_font(bmp);
	
	destroy_bitmap(bmp);
	return a5font;
}
void initFonts()
{
	deffont=font;
	nfont = (FONT*)fontsdata[FONT_GUI_PROP].dat;
	nfont2 = (FONT*)fontsdata[FONT_GUI_MONO].dat;
	font = nfont;
	pfont = (FONT*)fontsdata[FONT_8xPROP_THIN].dat;
	lfont = (FONT*)fontsdata[FONT_LARGEPROP].dat;
	lfont_l = (FONT*)fontsdata[FONT_LARGEPROP_L].dat;
	zfont = (FONT*)fontsdata[FONT_NES].dat;
	z3font = (FONT*)fontsdata[FONT_Z3].dat;
	z3smallfont = (FONT*)fontsdata[FONT_Z3SMALL].dat;
	mfont = (FONT*)fontsdata[FONT_MATRIX].dat;
	ztfont = (FONT*)fontsdata[FONT_ZTIME].dat;
	sfont = (FONT*)fontsdata[FONT_6x6].dat;
	sfont2 = (FONT*)fontsdata[FONT_6x4].dat;
	sfont3 = (FONT*)fontsdata[FONT_12x8].dat;
	spfont = (FONT*)fontsdata[FONT_6xPROP].dat;
	ssfont1 = (FONT*)fontsdata[FONT_SUBSCREEN1].dat;
	ssfont2 = (FONT*)fontsdata[FONT_SUBSCREEN2].dat;
	ssfont3 = (FONT*)fontsdata[FONT_SUBSCREEN3].dat;
	ssfont4 = (FONT*)fontsdata[FONT_SUBSCREEN4].dat;
	gblafont = (FONT*)fontsdata[FONT_GB_LA].dat;
	goronfont = (FONT*)fontsdata[FONT_GORON].dat;
	zoranfont = (FONT*)fontsdata[FONT_ZORAN].dat;
	hylian1font = (FONT*)fontsdata[FONT_HYLIAN1].dat;
	hylian2font = (FONT*)fontsdata[FONT_HYLIAN2].dat;
	hylian3font = (FONT*)fontsdata[FONT_HYLIAN3].dat;
	hylian4font = (FONT*)fontsdata[FONT_HYLIAN4].dat;
	gboraclefont = (FONT*)fontsdata[FONT_GB_ORACLE].dat;
	gboraclepfont = (FONT*)fontsdata[FONT_GB_ORACLE_P].dat;
	dsphantomfont = (FONT*)fontsdata[FONT_DS_PHANTOM].dat;
	dsphantompfont = (FONT*)fontsdata[FONT_DS_PHANTOM_P].dat;
	atari800font=(FONT*)fontsdata[FONT_ZZ_ATARU800].dat;  
	acornfont=(FONT*)fontsdata[FONT_ZZ_ACORN].dat;  
	adosfont=(FONT*)fontsdata[FONT_ZZ_ADOS].dat;  
	baseallegrofont=(FONT*)fontsdata[FONT_ZZ_ALLEGRO].dat;  
	apple2font=(FONT*)fontsdata[FONT_ZZ_APPLE2].dat;  
	apple280colfont=(FONT*)fontsdata[FONT_ZZ_APPLE280].dat;  
	apple2gsfont=(FONT*)fontsdata[FONT_ZZ_APPLE2GS].dat;
	aquariusfont=(FONT*)fontsdata[FONT_ZZ_AQUA].dat;  
	atari400font=(FONT*)fontsdata[FONT_ZZ_ATARI400].dat;  
	c64font=(FONT*)fontsdata[FONT_ZZ_C64].dat;  
	c64hiresfont=(FONT*)fontsdata[FONT_ZZ_C64HI].dat;  
	cgafont=(FONT*)fontsdata[FONT_ZZ_CGA].dat;  
	cocofont=(FONT*)fontsdata[FONT_ZZ_COCO].dat; 
	coco2font=(FONT*)fontsdata[FONT_ZZ_COCO2].dat;
	coupefont=(FONT*)fontsdata[FONT_ZZ_COUPE].dat;  
	cpcfont=(FONT*)fontsdata[FONT_ZZ_CPC].dat;  
	fantasyfont=(FONT*)fontsdata[FONT_ZZ_FANTASY].dat;  
	fdskanafont=(FONT*)fontsdata[FONT_ZZ_FDS_KANA].dat;  
	fdslikefont=(FONT*)fontsdata[FONT_ZZ_FDSLIKE].dat;  
	fdsromanfont=(FONT*)fontsdata[FONT_ZZ_FDSROMAN].dat;  
	finalffont=(FONT*)fontsdata[FONT_ZZ_FF].dat;
	futharkfont=(FONT*)fontsdata[FONT_ZZ_FUTHARK].dat;  
	gaiafont=(FONT*)fontsdata[FONT_ZZ_GAIA].dat;  
	hirafont=(FONT*)fontsdata[FONT_ZZ_HIRA].dat;  
	jpfont=(FONT*)fontsdata[FONT_ZZ_JP].dat;  
	kongfont=(FONT*)fontsdata[FONT_ZZ_KONG].dat;  
	manafont=(FONT*)fontsdata[FONT_ZZ_MANA].dat;  
	mlfont=(FONT*)fontsdata[FONT_ZZ_MARIOLAND].dat;  
	motfont=(FONT*)fontsdata[FONT_ZZ_MOT].dat;
	msxmode0font=(FONT*)fontsdata[FONT_ZZ_MSX0].dat;  
	msxmode1font=(FONT*)fontsdata[FONT_ZZ_MSX1].dat;  
	petfont=(FONT*)fontsdata[FONT_ZZ_PET].dat;  
	pstartfont=(FONT*)fontsdata[FONT_ZZ_PRESTRT].dat;  
	saturnfont=(FONT*)fontsdata[FONT_ZZ_SATURN].dat;  
	scififont=(FONT*)fontsdata[FONT_ZZ_SCIFI].dat;  
	sherwoodfont=(FONT*)fontsdata[FONT_ZZ_SHERWOOD].dat;
	sinqlfont=(FONT*)fontsdata[FONT_ZZ_SINQL].dat;  
	spectrumfont=(FONT*)fontsdata[FONT_ZZ_SPEC].dat;  
	speclgfont=(FONT*)fontsdata[FONT_ZZ_SPECLG].dat;  
	ti99font=(FONT*)fontsdata[FONT_ZZ_TI99].dat;  
	trsfont=(FONT*)fontsdata[FONT_ZZ_TRS].dat;  
	z2font=(FONT*)fontsdata[FONT_ZZ_ZELDA2].dat;  
	zxfont=(FONT*)fontsdata[FONT_ZZ_ZX].dat; 
	lisafont=(FONT*)fontsdata[FONT_ZZZ_LISA].dat;
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
	FONT* f = load_font(path,nullptr,nullptr);
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
	ALLEGRO_FONT* f = __load_a5_font(path);
	if(!f)
		zprint2("Error loading a5 font: '%s'\n", path);
	return f;
}

void init_custom_fonts()
{
	font = nfont;
	
	char pref[16];
	if(is_compact)
		strcpy(pref, "compact");
	else
		strcpy(pref, "large");
	
	char buf[512];
	int deffont_ids[CFONT_MAX];
	sprintf(buf, "font_%s_%s", pref, "dialog");
	deffont_ids[CFONT_DLG] = zc_get_config("ZQ_GUI", buf, font_lfont_l, App::zquest);
	sprintf(buf, "font_%s_%s", pref, "title");
	deffont_ids[CFONT_TITLE] = zc_get_config("ZQ_GUI", buf, font_lfont, App::zquest);
	sprintf(buf, "font_%s_%s", pref, "favcmd");
	deffont_ids[CFONT_FAVCMD] = zc_get_config("ZQ_GUI", buf, font_pfont, App::zquest);
	sprintf(buf, "font_%s_%s", pref, "gui");
	deffont_ids[CFONT_GUI] = zc_get_config("ZQ_GUI", buf, font_nfont, App::zquest);
	sprintf(buf, "font_%s_%s", pref, "textbox");
	deffont_ids[CFONT_TEXTBOX] = zc_get_config("ZQ_GUI", buf, font_sfont3, App::zquest);
	sprintf(buf, "font_%s_%s", pref, "ttip");
	deffont_ids[CFONT_TTIP] = zc_get_config("ZQ_GUI", buf, font_lfont, App::zquest);
	sprintf(buf, "font_%s_%s", pref, "info");
	deffont_ids[CFONT_INFO] = zc_get_config("ZQ_GUI", buf, font_lfont_l, App::zquest);
	
	for(int q = 0; q < CFONT_MAX; ++q)
	{
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
	}
	if(zc_get_config("gui","custom_fonts",1, App::zquest))
	{
		customfonts[CFONT_DLG] = load_cfont("dialog");
		customfonts[CFONT_TITLE] = load_cfont("title");
		customfonts[CFONT_FAVCMD] = load_cfont("favcmd");
		customfonts[CFONT_GUI] = load_cfont("gui");
		customfonts[CFONT_TEXTBOX] = load_cfont("textbox");
		customfonts[CFONT_TTIP] = load_cfont("ttip");
		customfonts[CFONT_INFO] = load_cfont("info");
		
		customfonts_a5[CFONT_DLG] = load_cfont_a5("dialog");
		customfonts_a5[CFONT_TITLE] = load_cfont_a5("title");
		customfonts_a5[CFONT_FAVCMD] = load_cfont_a5("favcmd");
		customfonts_a5[CFONT_GUI] = load_cfont_a5("gui");
		customfonts_a5[CFONT_TEXTBOX] = load_cfont_a5("textbox");
		customfonts_a5[CFONT_TTIP] = load_cfont_a5("ttip");
		customfonts_a5[CFONT_INFO] = load_cfont_a5("info");
	}
}

FONT* get_custom_font(int cfont)
{
	if(unsigned(cfont) >= CFONT_MAX)
		return lfont_l;
	if(zc_get_config("gui","custom_fonts",1,App::zquest) && customfonts[cfont])
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


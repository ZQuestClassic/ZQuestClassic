
#include "fonts.h"
#include "fontsdat.h"
extern DATAFILE *fontsdata;
FONT    *nfont, *nfont2, *zfont, *z3font, *z3smallfont, *deffont, *lfont, *lfont_l, *pfont, *mfont, *ztfont, *sfont, *sfont2, *sfont3, *spfont, *ssfont1, *ssfont2, *ssfont3, *ssfont4, *gblafont,
		*goronfont, *zoranfont, *hylian1font, *hylian2font, *hylian3font, *hylian4font, *gboraclefont, *gboraclepfont, *dsphantomfont, *dsphantompfont,
		//New fonts for 2.54+
		*atari800font, *acornfont, *adosfont, *baseallegrofont, *apple2font, *apple280colfont, *apple2gsfont,
		*aquariusfont, *atari400font, *c64font, *c64hiresfont, *cgafont, *cocofont, *coco2font,
		*coupefont, *cpcfont, *fantasyfont, *fdskanafont, *fdslikefont, *fdsromanfont, *finalffont,
		*futharkfont, *gaiafont, *hirafont, *jpfont, *kongfont, *manafont, *mlfont, *motfont,
		*msxmode0font, *msxmode1font, *petfont, *pstartfont, *saturnfont, *scififont, *sherwoodfont,
		*sinqlfont, *spectrumfont, *speclgfont, *ti99font, *trsfont, *z2font, *zxfont, *lisafont;

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
}

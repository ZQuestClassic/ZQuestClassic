#include "base/zdefs.h"
#include "jwin.h"
#include "base/zapp.h"
#include "dialog/info.h"
#include <sstream>
#include "zcmusic.h"

extern byte quest_rules[QUESTRULES_NEW_SIZE];

using std::string;
using std::ostringstream;
using namespace util;

extern PALETTE RAMpal;
extern bool update_hw_pal;

bool global_z3_scrolling_extended_height_mode = !true;
viewport_t viewport = {0};
int32_t global_z3_cur_scr_drawing = -1;

volatile bool close_button_quit = false;

int next_script_data_debug_id = 0;

const char months[13][13] =
{ 
	"Nonetober", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
};

char *VerStr(int32_t version)
{
    static char ver_str[15];
    sprintf(ver_str,"v%d.%02X",version>>8,version&0xFF);
    return ver_str;
}

RGB _RGB(byte *si)
{
	RGB x;
	x.r = si[0];
	x.g = si[1];
	x.b = si[2];
	x.filler=0;
	return x;
}

RGB _RGB(int32_t r,int32_t g,int32_t b)
{
	RGB x;
	x.r = r;
	x.g = g;
	x.b = b;
	x.filler=0;
	return x;
}

RGB invRGB(RGB s)
{
	RGB x;
	x.r = 63-s.r;
	x.g = 63-s.g;
	x.b = 63-s.b;
	x.filler=0;
	return x;
}

RGB mixRGB(int32_t r1,int32_t g1,int32_t b1,int32_t r2,int32_t g2,int32_t b2,int32_t ratio)
{
	RGB x;
	x.r = (r1*(64-ratio) + r2*ratio) >> 6;
	x.g = (g1*(64-ratio) + g2*ratio) >> 6;
	x.b = (b1*(64-ratio) + b2*ratio) >> 6;
	x.filler=0;
	return x;
}

static char themefile[2048] = {0};
char tmp_themefile[2048] = {0};
char const* get_themefile()
{
	return themefile;
}
void set_theme(char const* fpath)
{
	strcpy(themefile, fpath);
	reset_theme();
}
void reset_theme()
{
	load_themefile(themefile);
}
void load_themefile(char const* fpath)
{
	load_themefile(fpath, RAMpal);
}
void load_themefile(char const* fpath, PALETTE pal)
{
	zc_push_config();
	zc_config_file(fpath);
	pal[dvc(1)] = _RGB(zc_get_config_basic("Theme","dvc1_r",4),zc_get_config_basic("Theme","dvc1_g",38),zc_get_config_basic("Theme","dvc1_b",46)); //box fg is text
	pal[dvc(2)] = _RGB(zc_get_config_basic("Theme","dvc2_r",(16*63/255)), zc_get_config_basic("Theme","dvc2_g",(10*63/255)), zc_get_config_basic("Theme","dvc2_b",0));
	pal[dvc(3)] = _RGB(zc_get_config_basic("Theme","dvc3_r",17),zc_get_config_basic("Theme","dvc3_g",20),zc_get_config_basic("Theme","dvc3_b",20)); //slate
	pal[dvc(4)] = _RGB(zc_get_config_basic("Theme","dvc4_r",13),zc_get_config_basic("Theme","dvc4_g",14),zc_get_config_basic("Theme","dvc4_b",14)); //menu background
	pal[dvc(5)] = _RGB(zc_get_config_basic("Theme","dvc5_r",0),zc_get_config_basic("Theme","dvc5_g",0),zc_get_config_basic("Theme","dvc5_b",0));//menu text bg
	pal[dvc(6)] = _RGB(zc_get_config_basic("Theme","dvc6_r",13),zc_get_config_basic("Theme","dvc6_g",14),zc_get_config_basic("Theme","dvc6_b",14));//menu selected text
	pal[dvc(7)] = _RGB(zc_get_config_basic("Theme","dvc7_r",42),zc_get_config_basic("Theme","dvc7_g",60),zc_get_config_basic("Theme","dvc7_b",48));
	pal[dvc(8)] = _RGB(zc_get_config_basic("Theme","dvc8_r",6),zc_get_config_basic("Theme","dvc8_g",49),zc_get_config_basic("Theme","dvc8_b",35));//highlight on selected menu text
					   
	jwin_pal[jcBOX]	=dvc(zc_get_config_basic("Theme","jcbox",4));
	jwin_pal[jcLIGHT]  =dvc(zc_get_config_basic("Theme","jclight",5));
	jwin_pal[jcMEDLT]  =dvc(zc_get_config_basic("Theme","jcmedlt",4));
	jwin_pal[jcMEDDARK]=dvc(zc_get_config_basic("Theme","jcmeddark",3));
	jwin_pal[jcDARK]   =dvc(zc_get_config_basic("Theme","jcdark",2));
	jwin_pal[jcBOXFG]  =dvc(zc_get_config_basic("Theme","jcboxfg",1));
	jwin_pal[jcTITLEL] =dvc(zc_get_config_basic("Theme","jctitlel",3));
	jwin_pal[jcTITLER] =dvc(zc_get_config_basic("Theme","jctitler",5));
	jwin_pal[jcTITLEFG]=dvc(zc_get_config_basic("Theme","jctitlefg",7));
	jwin_pal[jcTEXTBG] =dvc(zc_get_config_basic("Theme","jctextbg",5));
	jwin_pal[jcTEXTFG] =dvc(zc_get_config_basic("Theme","jctextfg",1));
	jwin_pal[jcSELBG]  =dvc(zc_get_config_basic("Theme","jcselbg",8));
	jwin_pal[jcSELFG]  =dvc(zc_get_config_basic("Theme","jcselfg",6));
	jwin_pal[jcCURSORMISC] = dvc(zc_get_config_basic("Theme","jccursormisc",1));
	jwin_pal[jcCURSOROUTLINE] = dvc(zc_get_config_basic("Theme","jccursoroutline",2));
	jwin_pal[jcCURSORLIGHT] = dvc(zc_get_config_basic("Theme","jccursorlight",3));
	jwin_pal[jcCURSORDARK] = dvc(zc_get_config_basic("Theme","jccursordark",5));
	jwin_pal[jcALT_TEXTFG] = dvc(zc_get_config_basic("Theme","jc_alt_textfg",r_dvc(jwin_pal[jcMEDDARK])));
	jwin_pal[jcALT_TEXTBG] = dvc(zc_get_config_basic("Theme","jc_alt_textbg",r_dvc(jwin_pal[jcTEXTFG])));
	jwin_pal[jcDISABLED_FG] = dvc(zc_get_config_basic("Theme","jc_disabled_fg",r_dvc(jwin_pal[jcMEDDARK])));
	jwin_pal[jcDISABLED_BG] = dvc(zc_get_config_basic("Theme","jc_disabled_bg",r_dvc(jwin_pal[jcBOX])));
	
	zc_pop_config();
	
    gui_bg_color=jwin_pal[jcBOX];
    gui_fg_color=jwin_pal[jcBOXFG];
    jwin_set_colors(jwin_pal);
	update_hw_pal = true;
}

void save_themefile(char const* fpath)
{
	save_themefile(fpath, RAMpal);
}
void save_themefile(char const* fpath, PALETTE pal)
{
	zc_push_config();
	zc_config_file(fpath);
	zc_set_config_basic("Theme","dvc1_r",pal[dvc(1)].r); zc_set_config_basic("Theme","dvc1_g",pal[dvc(1)].g); zc_set_config_basic("Theme","dvc1_b",pal[dvc(1)].b);
	zc_set_config_basic("Theme","dvc2_r",pal[dvc(2)].r); zc_set_config_basic("Theme","dvc2_g",pal[dvc(2)].g); zc_set_config_basic("Theme","dvc2_b",pal[dvc(2)].b);
	zc_set_config_basic("Theme","dvc3_r",pal[dvc(3)].r); zc_set_config_basic("Theme","dvc3_g",pal[dvc(3)].g); zc_set_config_basic("Theme","dvc3_b",pal[dvc(3)].b);
	zc_set_config_basic("Theme","dvc4_r",pal[dvc(4)].r); zc_set_config_basic("Theme","dvc4_g",pal[dvc(4)].g); zc_set_config_basic("Theme","dvc4_b",pal[dvc(4)].b);
	zc_set_config_basic("Theme","dvc5_r",pal[dvc(5)].r); zc_set_config_basic("Theme","dvc5_g",pal[dvc(5)].g); zc_set_config_basic("Theme","dvc5_b",pal[dvc(5)].b);
	zc_set_config_basic("Theme","dvc6_r",pal[dvc(6)].r); zc_set_config_basic("Theme","dvc6_g",pal[dvc(6)].g); zc_set_config_basic("Theme","dvc6_b",pal[dvc(6)].b);
	zc_set_config_basic("Theme","dvc7_r",pal[dvc(7)].r); zc_set_config_basic("Theme","dvc7_g",pal[dvc(7)].g); zc_set_config_basic("Theme","dvc7_b",pal[dvc(7)].b);
	zc_set_config_basic("Theme","dvc8_r",pal[dvc(8)].r); zc_set_config_basic("Theme","dvc8_g",pal[dvc(8)].g); zc_set_config_basic("Theme","dvc8_b",pal[dvc(8)].b);
	zc_set_config_basic("Theme","jcbox",r_dvc(jwin_pal[jcBOX]));
	zc_set_config_basic("Theme","jclight",r_dvc(jwin_pal[jcLIGHT]));
	zc_set_config_basic("Theme","jcmedlt",r_dvc(jwin_pal[jcMEDLT]));
	zc_set_config_basic("Theme","jcmeddark",r_dvc(jwin_pal[jcMEDDARK]));
	zc_set_config_basic("Theme","jcdark",r_dvc(jwin_pal[jcDARK]));
	zc_set_config_basic("Theme","jcboxfg",r_dvc(jwin_pal[jcBOXFG]));
	zc_set_config_basic("Theme","jctitlel",r_dvc(jwin_pal[jcTITLEL]));
	zc_set_config_basic("Theme","jctitler",r_dvc(jwin_pal[jcTITLER]));
	zc_set_config_basic("Theme","jctitlefg",r_dvc(jwin_pal[jcTITLEFG]));
	zc_set_config_basic("Theme","jctextbg",r_dvc(jwin_pal[jcTEXTBG]));
	zc_set_config_basic("Theme","jctextfg",r_dvc(jwin_pal[jcTEXTFG]));
	zc_set_config_basic("Theme","jcselbg",r_dvc(jwin_pal[jcSELBG]));
	zc_set_config_basic("Theme","jcselfg",r_dvc(jwin_pal[jcSELFG]));
	zc_set_config_basic("Theme","jccursormisc",r_dvc(jwin_pal[jcCURSORMISC]));
	zc_set_config_basic("Theme","jccursoroutline",r_dvc(jwin_pal[jcCURSOROUTLINE]));
	zc_set_config_basic("Theme","jccursorlight",r_dvc(jwin_pal[jcCURSORLIGHT]));
	zc_set_config_basic("Theme","jccursordark",r_dvc(jwin_pal[jcCURSORDARK]));
	zc_set_config_basic("Theme","jc_alt_textfg",r_dvc(jwin_pal[jcALT_TEXTFG]));
	zc_set_config_basic("Theme","jc_alt_textbg",r_dvc(jwin_pal[jcALT_TEXTBG]));
	zc_set_config_basic("Theme","jc_disabled_fg",r_dvc(jwin_pal[jcDISABLED_FG]));
	zc_set_config_basic("Theme","jc_disabled_bg",r_dvc(jwin_pal[jcDISABLED_BG]));
	zc_pop_config();
}

const char* get_app_theme_filename()
{
	switch (get_app_id()) {
		case App::zelda: return "_custom_zc.ztheme";
		case App::zquest: return "_custom_zq.ztheme";
		case App::launcher: return "_custom_zcl.ztheme";
		default: return "_custom.ztheme";
	}
}

void load_udef_colorset(App a)
{
	load_udef_colorset(a, RAMpal);
}
void load_udef_colorset(App a, PALETTE pal)
{
	char const* darkthemename = "themes/dark.ztheme";
	char const* tfnm = zc_get_config("Theme", "theme_filename", "-", a);
	bool defaulted_theme = !(tfnm[0]&&tfnm[0]!='-');
	strcpy(tmp_themefile, defaulted_theme ? darkthemename : tfnm);
	
	fix_filename_case(tmp_themefile);
	fix_filename_slashes(tmp_themefile);
	if(defaulted_theme &&
		(zc_get_config("Theme", "dvc1_r", 1, a)
		!= zc_get_config("Theme", "dvc1_r", 2, a)))
	{
		//Write these back to the custom theme file
		strcpy(tmp_themefile, get_app_theme_filename());
		load_themefile(get_config_file_name(a), pal);
		save_themefile(tmp_themefile, pal);
	}
	else load_themefile(tmp_themefile, pal);
	if (defaulted_theme)
	{
		zc_set_config("Theme", "theme_filename", tmp_themefile);
	}
}

void load_colorset(int32_t colorset)
{
	load_colorset(colorset, RAMpal);
}
void load_colorset(int32_t colorset, PALETTE pal)
{
	bool udef = false;
	switch(colorset)
	{
		case 1:  //Windows 98
		{
			pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
			pal[dvc(2)] = _RGB(128*63/255, 128*63/255, 128*63/255);
			pal[dvc(3)] = _RGB(192*63/255, 192*63/255, 192*63/255);
			pal[dvc(4)] = _RGB(223*63/255, 223*63/255, 223*63/255);
			pal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
			pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
			pal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
			pal[dvc(8)] = _RGB(0*63/255,   0*63/255,  80*63/255);
			
			byte palrstart=  0*63/255, palrend=166*63/255,
				 palgstart=  0*63/255, palgend=202*63/255,
				 palbstart=128*63/255, palbend=240*63/255,
				 paldivs=7;
				 
			for(int32_t i=0; i<paldivs; i++)
			{
				pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
			}
			
			jwin_pal[jcBOX]	=dvc(3);
			jwin_pal[jcLIGHT]  =dvc(5);
			jwin_pal[jcMEDLT]  =dvc(4);
			jwin_pal[jcMEDDARK]=dvc(2);
			jwin_pal[jcDARK]   =dvc(1);
			jwin_pal[jcBOXFG]  =dvc(1);
			jwin_pal[jcTITLEL] =dvc(9);
			jwin_pal[jcTITLER] =dvc(15);
			jwin_pal[jcTITLEFG]=dvc(7);
			jwin_pal[jcTEXTBG] =dvc(5);
			jwin_pal[jcTEXTFG] =dvc(1);
			jwin_pal[jcSELBG]  =dvc(8);
			jwin_pal[jcSELFG]  =dvc(6);
		}
		break;
		
		case 2:  //Windows 99
		{
			pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
			pal[dvc(2)] = _RGB(64*63/255,  64*63/255,  64*63/255);
			pal[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
			pal[dvc(4)] = _RGB(192*63/255, 192*63/255, 192*63/255);
			pal[dvc(5)] = _RGB(223*63/255, 223*63/255, 223*63/255);
			pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 255*63/255);
			pal[dvc(7)] = _RGB(255*63/255, 255*63/255, 225*63/255);
			pal[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
			pal[dvc(9)] = _RGB(0*63/255,   0*63/255,  80*63/255);
			
			byte palrstart=  0*63/255, palrend=166*63/255,
				 palgstart=  0*63/255, palgend=202*63/255,
				 palbstart=128*63/255, palbend=240*63/255,
				 paldivs=6;
				 
			for(int32_t i=0; i<paldivs; i++)
			{
				pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
			}
			
			jwin_pal[jcBOX]	=dvc(4);
			jwin_pal[jcLIGHT]  =dvc(6);
			jwin_pal[jcMEDLT]  =dvc(5);
			jwin_pal[jcMEDDARK]=dvc(3);
			jwin_pal[jcDARK]   =dvc(2);
			jwin_pal[jcBOXFG]  =dvc(1);
			jwin_pal[jcTITLEL] =dvc(10);
			jwin_pal[jcTITLER] =dvc(15);
			jwin_pal[jcTITLEFG]=dvc(8);
			jwin_pal[jcTEXTBG] =dvc(6);
			jwin_pal[jcTEXTFG] =dvc(1);
			jwin_pal[jcSELBG]  =dvc(9);
			jwin_pal[jcSELFG]  =dvc(7);
		}
		break;
		
		case 3:  //Windows 2000 Blue
		{
			pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
			pal[dvc(2)] = _RGB(16*63/255,  15*63/255, 116*63/255);
			pal[dvc(3)] = _RGB(82*63/255,  80*63/255, 182*63/255);
			pal[dvc(4)] = _RGB(162*63/255, 158*63/255, 250*63/255);
			pal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
			pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 127*63/255);
			pal[dvc(7)] = _RGB(255*63/255, 225*63/255,  63*63/255);
			pal[dvc(8)] = _RGB(0*63/255,   0*63/255,  80*63/255);
			
			byte palrstart=  0*63/255, palrend=162*63/255,
				 palgstart=  0*63/255, palgend=158*63/255,
				 palbstart= 80*63/255, palbend=250*63/255,
				 paldivs=7;
				 
			for(int32_t i=0; i<paldivs; i++)
			{
				pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
			}
			
			jwin_pal[jcBOX]	=dvc(4);
			jwin_pal[jcLIGHT]  =dvc(5);
			jwin_pal[jcMEDLT]  =dvc(4);
			jwin_pal[jcMEDDARK]=dvc(3);
			jwin_pal[jcDARK]   =dvc(2);
			jwin_pal[jcBOXFG]  =dvc(1);
			jwin_pal[jcTITLEL] =dvc(9);
			jwin_pal[jcTITLER] =dvc(15);
			jwin_pal[jcTITLEFG]=dvc(7);
			jwin_pal[jcTEXTBG] =dvc(5);
			jwin_pal[jcTEXTFG] =dvc(1);
			jwin_pal[jcSELBG]  =dvc(8);
			jwin_pal[jcSELFG]  =dvc(6);
		}
		break;
		
		case 687:  //Windows 2000 Gold (6-87 was the North American release date of LoZ)
		{
			pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
			pal[dvc(2)] = _RGB(64*63/255,  64*63/255,  43*63/255);
			pal[dvc(3)] = _RGB(170*63/255, 154*63/255,  96*63/255);
			pal[dvc(4)] = _RGB(223*63/255, 200*63/255, 128*63/255); // Old Gold
			pal[dvc(5)] = _RGB(240*63/255, 223*63/255, 136*63/255);
			pal[dvc(6)] = _RGB(255*63/255, 223*63/255, 128*63/255);
			pal[dvc(7)] = _RGB(255*63/255, 223*63/255, 128*63/255);
			pal[dvc(8)] = _RGB(255*63/255, 225*63/255, 160*63/255);
			pal[dvc(9)] = _RGB(80*63/255,  80*63/255,   0*63/255);
			
			byte palrstart=128*63/255, palrend=240*63/255,
				 palgstart=128*63/255, palgend=202*63/255,
				 palbstart=  0*63/255, palbend=166*63/255,
				 paldivs=6;
				 
			for(int32_t i=0; i<paldivs; i++)
			{
				pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
			}
			
			jwin_pal[jcBOX]	=dvc(4);
			jwin_pal[jcLIGHT]  =dvc(6);
			jwin_pal[jcMEDLT]  =dvc(5);
			jwin_pal[jcMEDDARK]=dvc(3);
			jwin_pal[jcDARK]   =dvc(2);
			jwin_pal[jcBOXFG]  =dvc(1);
			jwin_pal[jcTITLEL] =dvc(10);
			jwin_pal[jcTITLER] =dvc(15);
			jwin_pal[jcTITLEFG]=dvc(8);
			jwin_pal[jcTEXTBG] =dvc(6);
			jwin_pal[jcTEXTFG] =dvc(1);
			jwin_pal[jcSELBG]  =dvc(9);
			jwin_pal[jcSELFG]  =dvc(7);
		}
		break;
		
		case 4104:  //Windows 2000 Easter (4-1-04 is April Fools Day, the date of this release)
		{
			pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
			pal[dvc(2)] = _RGB(64*63/255,  64*63/255,  64*63/255);
			pal[dvc(3)] = _RGB(128*63/255, 128*63/255, 128*63/255);
			pal[dvc(4)] = _RGB(252*63/255, 186*63/255, 188*63/255);
			pal[dvc(5)] = _RGB(254*63/255, 238*63/255, 238*63/255);
			pal[dvc(6)] = _RGB(244*63/255, 243*63/255, 161*63/255);
			pal[dvc(7)] = _RGB(120*63/255, 173*63/255, 189*63/255);
			pal[dvc(8)] = _RGB(220*63/255, 183*63/255, 227*63/255);
			
			byte palrstart=244*63/255, palrend=220*63/255,
				 palgstart=243*63/255, palgend=183*63/255,
				 palbstart=161*63/255, palbend=227*63/255,
				 paldivs=7;
				 
			for(int32_t i=0; i < paldivs; i++)
			{
				pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
			}
			
			jwin_pal[jcBOX]	=dvc(4);
			jwin_pal[jcLIGHT]  =dvc(5);
			jwin_pal[jcMEDLT]  =dvc(4);
			jwin_pal[jcMEDDARK]=dvc(3);
			jwin_pal[jcDARK]   =dvc(2);
			jwin_pal[jcBOXFG]  =dvc(7);
			jwin_pal[jcTITLEL] =dvc(9);
			jwin_pal[jcTITLER] =dvc(15);
			jwin_pal[jcTITLEFG]=dvc(7);
			jwin_pal[jcTEXTBG] =dvc(5);
			jwin_pal[jcTEXTFG] =dvc(7);
			jwin_pal[jcSELBG]  =dvc(8);
			jwin_pal[jcSELFG]  =dvc(6);
		}
		break;
		
		case 2019:  //2.55 DARK Theme
		{
		   
			pal[dvc(1)] = _RGB(4,38,46); //box fg is text
			pal[dvc(2)] = _RGB(16*63/255, 10*63/255, 0*63/255);
			pal[dvc(3)] = _RGB(17,20,20); //slate
			pal[dvc(4)] = _RGB(13,14,14); //menu background
			pal[dvc(5)] = _RGB(0,0,0);//menu text bg
			pal[dvc(6)] = _RGB(13,14,14);//menu selected text
			pal[dvc(7)] = _RGB(42,60,48);
			pal[dvc(8)] = _RGB(6,49,35);//highlight on selected menu text
			
			byte palrstart= 10*63/255, palrend=166*63/255,
				 palgstart= 36*63/255, palgend=202*63/255,
				 palbstart=106*63/255, palbend=240*63/255,
				 paldivs=7;
		   
			jwin_pal[jcBOX]	=dvc(4);
			jwin_pal[jcLIGHT]  =dvc(5);
			jwin_pal[jcMEDLT]  =dvc(4);
			jwin_pal[jcMEDDARK]=dvc(3);
			jwin_pal[jcDARK]   =dvc(2);
			jwin_pal[jcBOXFG]  =dvc(1);
			jwin_pal[jcTITLEL] =dvc(3);
			jwin_pal[jcTITLER] =dvc(5);
			jwin_pal[jcTITLEFG]=dvc(7);
			jwin_pal[jcTEXTBG] =dvc(5);
			jwin_pal[jcTEXTFG] =dvc(1);
			jwin_pal[jcSELBG]  =dvc(8);
			jwin_pal[jcSELFG]  =dvc(6);
		}
		break;
		
		case 99:  //User Defined
		{
			udef = true;
			load_udef_colorset(App::undefined, pal);
			strcpy(themefile, tmp_themefile);
		}
		break;
		
		case 201018:  //20-oct-2018, PureZC Expo
		{
			//16,10,0; dark chocolate
			//39,19,0 chocolate
			//63,23,0 orange
			//46,32,4 tan
			pal[dvc(1)] = _RGB(63,23,0); //box fg is text
			pal[dvc(2)] = _RGB(16*63/255, 10*63/255, 0*63/255);
			pal[dvc(3)] = _RGB(39,19,0);
		   // pal[dvc(4)] = _RGB(212*63/255, 208*63/255, 200*63/255);
			pal[dvc(4)] = _RGB(16,10,0); //menu background
			pal[dvc(5)] = _RGB(0,0,0);
			//pal[dvc(5)] = _RGB(63*63/255, 23*63/255, 0*63/255);
			//pal[dvc(5)] = _
			pal[dvc(6)] = _RGB(0,0,0);
			pal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
			pal[dvc(8)] = _RGB(63,49,0);
			
			byte palrstart= 10*63/255, palrend=166*63/255,
				 palgstart= 36*63/255, palgend=202*63/255,
				 palbstart=106*63/255, palbend=240*63/255,
				 paldivs=7;
			/*	 
			for(int32_t i=0; i<paldivs; i++)
			{
				pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
			}
			*/
			jwin_pal[jcBOX]	=dvc(4);
			jwin_pal[jcLIGHT]  =dvc(5);
			jwin_pal[jcMEDLT]  =dvc(4);
			jwin_pal[jcMEDDARK]=dvc(3);
			jwin_pal[jcDARK]   =dvc(2);
			jwin_pal[jcBOXFG]  =dvc(1);
			jwin_pal[jcTITLEL] =dvc(3);
			jwin_pal[jcTITLER] =dvc(3);
			jwin_pal[jcTITLEFG]=dvc(7);
			jwin_pal[jcTEXTBG] =dvc(5);
			jwin_pal[jcTEXTFG] =dvc(1);
			jwin_pal[jcSELBG]  =dvc(8);
			jwin_pal[jcSELFG]  =dvc(6);
		}
		break;
		
		default:  //Windows 2000
		{
			pal[dvc(1)] = _RGB(0*63/255,   0*63/255,   0*63/255);
			pal[dvc(2)] = _RGB(66*63/255,  65*63/255,  66*63/255);
			pal[dvc(3)] = _RGB(132*63/255, 130*63/255, 132*63/255);
			pal[dvc(4)] = _RGB(212*63/255, 208*63/255, 200*63/255);
			pal[dvc(5)] = _RGB(255*63/255, 255*63/255, 255*63/255);
			pal[dvc(6)] = _RGB(255*63/255, 255*63/255, 225*63/255);
			pal[dvc(7)] = _RGB(255*63/255, 225*63/255, 160*63/255);
			pal[dvc(8)] = _RGB(0*63/255,   0*63/255,  80*63/255);
			
			byte palrstart= 10*63/255, palrend=166*63/255,
				 palgstart= 36*63/255, palgend=202*63/255,
				 palbstart=106*63/255, palbend=240*63/255,
				 paldivs=7;
				 
			for(int32_t i=0; i<paldivs; i++)
			{
				pal[dvc(15-paldivs+1)+i].r = palrstart+((palrend-palrstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].g = palgstart+((palgend-palgstart)*i/(paldivs-1));
				pal[dvc(15-paldivs+1)+i].b = palbstart+((palbend-palbstart)*i/(paldivs-1));
			}
			
			jwin_pal[jcBOX]	=dvc(4);
			jwin_pal[jcLIGHT]  =dvc(5);
			jwin_pal[jcMEDLT]  =dvc(4);
			jwin_pal[jcMEDDARK]=dvc(3);
			jwin_pal[jcDARK]   =dvc(2);
			jwin_pal[jcBOXFG]  =dvc(1);
			jwin_pal[jcTITLEL] =dvc(9);
			jwin_pal[jcTITLER] =dvc(15);
			jwin_pal[jcTITLEFG]=dvc(7);
			jwin_pal[jcTEXTBG] =dvc(5);
			jwin_pal[jcTEXTFG] =dvc(1);
			jwin_pal[jcSELBG]  =dvc(8);
			jwin_pal[jcSELFG]  =dvc(6);
		}
		break;
	}
	
	if(!udef) //Auto-fill the indexes that aren't used in old styles
	{
		jwin_pal[jcCURSORMISC] = dvc(1);
		jwin_pal[jcCURSOROUTLINE] = dvc(2);
		jwin_pal[jcCURSORLIGHT] = dvc(3);
		jwin_pal[jcCURSORDARK] = dvc(5);
		jwin_pal[jcALT_TEXTFG] = jwin_pal[jcMEDDARK];
		jwin_pal[jcALT_TEXTBG] = jwin_pal[jcTEXTFG];
		jwin_pal[jcDISABLED_FG] = jwin_pal[jcMEDDARK];
		jwin_pal[jcDISABLED_BG] = jwin_pal[jcBOX];
	}
	
    gui_bg_color=jwin_pal[jcBOX];
    gui_fg_color=jwin_pal[jcBOXFG];
    
    jwin_set_colors(jwin_pal);
}

//MsgStr stuff
// Copy everything except listpos
MsgStr& MsgStr::operator=(MsgStr const& other)
{
	copyText(other);
	copyStyle(other);
	return *this;
}

// Copy text data - just s and nextstring
void MsgStr::copyText(MsgStr const& other)
{
	s.resize(other.s.size());
	s.assign(other.s.c_str());
	nextstring=other.nextstring;
}

// Copy style data - everything except s, nextstring, and listpos
void MsgStr::copyStyle(MsgStr const& other)
{
	tile=other.tile;
	cset=other.cset;
	trans=other.trans;
	font=other.font;
	x=other.x;
	y=other.y;
	w=other.w;
	h=other.h;
	sfx=other.sfx;
	vspace=other.vspace;
	hspace=other.hspace;
	stringflags=other.stringflags;
	for(int32_t q = 0; q < 4; ++q)
	{
		margins[q] = other.margins[q];
	}
	portrait_tile=other.portrait_tile;
	portrait_cset=other.portrait_cset;
	portrait_x=other.portrait_x;
	portrait_y=other.portrait_y;
	portrait_tw=other.portrait_tw;
	portrait_th=other.portrait_th;
	shadow_type=other.shadow_type;
	shadow_color=other.shadow_color;
	drawlayer=other.drawlayer;
}

void MsgStr::copyAll(MsgStr const& other)
{
	copyText(other);
	copyStyle(other);
	listpos = other.listpos;
}

void MsgStr::clear()
{
	s = "";
	s.shrink_to_fit();
	nextstring = 0;
	tile=0;
	cset=0;
	trans=0;
	font=0;
	x=0;
	y=0;
	w=0;
	h=0;
	sfx=0;
	vspace=0;
	hspace=0;
	stringflags=0;
	for(int32_t q = 0; q < 4; ++q)
	{
		margins[q] = 0;
	}
	portrait_tile=0;
	portrait_cset=0;
	portrait_x=0;
	portrait_y=0;
	portrait_tw=0;
	portrait_th=0;
	shadow_type=0;
	shadow_color=0;
	listpos=0;
	drawlayer=6;
}

bool valid_str(char const* ptr, char cancel)
{
	return ptr && ptr[0] && ptr[0] != cancel;
}

direction X_DIR(int32_t dir)
{
	dir = NORMAL_DIR(dir);
	if(dir < 0) return dir_invalid;
	return xDir[dir];
}
direction Y_DIR(int32_t dir)
{
	dir = NORMAL_DIR(dir);
	if(dir < 0) return dir_invalid;
	return yDir[dir];
}
direction XY_DIR(int32_t xdir, int32_t ydir)
{
	if(X_DIR(xdir) < 0) return NORMAL_DIR(ydir);
	if(Y_DIR(ydir) < 0) return NORMAL_DIR(xdir);
	switch(X_DIR(xdir))
	{
		case right:
			switch(Y_DIR(ydir))
			{
				case up: return r_up;
				case down: return r_down;
			}
			break;
		case left:
			switch(Y_DIR(ydir))
			{
				case up: return l_up;
				case down: return l_down;
			}
			break;
	}
	return dir_invalid;
}
direction GET_XDIR(zfix const& sign)
{
	if(sign < 0) return left;
	if(sign) return right;
	return dir_invalid;
}
direction GET_YDIR(zfix const& sign)
{
	if(sign < 0) return up;
	if(sign) return down;
	return dir_invalid;
}
direction GET_DIR(zfix const& dx, zfix const& dy)
{
	return XY_DIR(GET_XDIR(dx), GET_YDIR(dy));
}

direction XY_DELTA_TO_DIR(int32_t dx, int32_t dy)
{
	if (dx == 0 && dy == 0) return dir_invalid;
	if (dx == 1 && dy == 0) return right;
	if (dx == -1 && dy == 0) return left;
	if (dx == 0 && dy == 1) return down;
	if (dx == 0 && dy == -1) return up;
	return dir_invalid;
}

string get_dbreport_string()
{
	std::stringstream oss;
	
	char buf[256];
	
	oss << "```\n"
		<< ZQ_EDITOR_NAME
		<< "\nVersion: " << ZQ_EDITOR_V << " " << ALPHA_VER_STR
		<< "\nTag: " << getReleaseTag()
		<< "\nBuild: " << VERSION_BUILD;
		
	sprintf(buf,"Build Date: %s %s, %d at @ %s %s", dayextension(BUILDTM_DAY).c_str(),
		(char*)months[BUILDTM_MONTH], BUILDTM_YEAR, __TIME__, __TIMEZONE__);
	
	oss << "\n" << buf
		<< "\nDev Signoff: " << DEV_SIGNOFF
		<< "\nQR:" << get_qr_hexstr(quest_rules, true, false)
		<< "\n```";
	return oss.str();
}

string get_qr_hexstr(byte* qrs, bool hash, bool disctags)
{
	if(!qrs) qrs = quest_rules;
	std::ostringstream oss;
	if(hash)
	{
		if(disctags)
			oss << "`";
		oss << "##";
	}
	for (size_t i = 0; i < QUESTRULES_NEW_SIZE; ++i)
	{
		char hex_buf[3];
		sprintf(hex_buf, "%02X", qrs[i]);
		oss << hex_buf;
	}
	if(hash)
	{
		oss << "##";
		if(disctags)
			oss << "`";
	}
	return oss.str();
}

bool clipboard_has_text()
{
	return al_clipboard_has_text(all_get_display());
}
bool get_al_clipboard(std::string& clipboard)
{
	if(!clipboard_has_text()) return false;
	clipboard = al_get_clipboard_text(all_get_display());
	return true;
}
void set_al_clipboard(std::string const& clipboard)
{
	al_set_clipboard_text(all_get_display(), clipboard.c_str());
}

bool load_qr_hexstr(string hexstr)
{
	//Strings with '##' delimiting start/end of substring are valid
	size_t startpos = hexstr.find("##");
	if(startpos != string::npos)
	{
		startpos += 2; //cut out the '##'
		size_t endpos = hexstr.find("##", startpos); //find an end terminator
		if(endpos == string::npos)
			hexstr = hexstr.substr(startpos);
		else hexstr = hexstr.substr(startpos, endpos-startpos);
	}
	if(hexstr.size() != QUESTRULES_NEW_SIZE*2)
	{
		if(hexstr.find_first_not_of("0123456789ABCDEFabcdef") != string::npos)
		{
			return false;
		}
		else zprint2("Detected bad hexstr length '%d != %d'!\n", hexstr.size(), QUESTRULES_NEW_SIZE * 2);
	}
	size_t len = hexstr.size() / 2;
	if(len > QUESTRULES_NEW_SIZE) len = QUESTRULES_NEW_SIZE;
	for(size_t i = 0; i < len; ++i)
	{
		char hex_buf[3];
		sprintf(hex_buf, "%s", hexstr.substr(i*2,2).c_str());
		quest_rules[i] = (byte)zc_xtoi(hex_buf);
	}
	InfoDialog("QRs Loaded", "Quest Rules have been loaded from the clipboard").show();
	return true;
}
bool load_qr_hexstr_clipboard()
{
	if(!clipboard_has_text()) return false;
	char* clipboardstr = al_get_clipboard_text(all_get_display());
	if(!clipboardstr) return false;
	string str(clipboardstr);
	bool ret = load_qr_hexstr(str);
	al_free(clipboardstr);
	return true;
}

bool load_dev_info(string& devstr)
{
	if(devstr.empty())
		return false;
	if(!load_qr_hexstr(devstr))
		return false;
	removechar(devstr, '\r');
	size_t startchop = devstr.find("```\n");
	if(startchop == string::npos)
		startchop = 0;
	size_t endchop = devstr.find("\nQR:", startchop);
	InfoDialog("DEV About Data", devstr.substr(startchop,endchop-startchop)).show();
	return true;
}
bool load_dev_info_clipboard()
{
	if(!clipboard_has_text()) return false;
	char* clipboardstr = al_get_clipboard_text(all_get_display());
	if(!clipboardstr) return false;
	string str(clipboardstr);
	bool ret = load_dev_info(str);
	al_free(clipboardstr);
	return true;
}

string generate_zq_about()
{
	char buf1[256];
	std::ostringstream oss;
	sprintf(buf1,"%s (%s), Version: %s", ZQ_EDITOR_NAME,PROJECT_NAME,ZQ_EDITOR_V);
	oss << buf1 << '\n';
	sprintf(buf1,"Tag: %s", getReleaseTag());
	oss << buf1 << '\n';
	sprintf(buf1, "%s, Build %d", ALPHA_VER_STR, VERSION_BUILD);
	oss << buf1 << '\n';
	sprintf(buf1,"Build Date: %s %s, %d at @ %s %s", dayextension(BUILDTM_DAY).c_str(), (char*)months[BUILDTM_MONTH], BUILDTM_YEAR, __TIME__, __TIMEZONE__);
	oss << buf1 << '\n';
	sprintf(buf1, "Built By: %s", DEV_SIGNOFF);
	oss << buf1 << '\n';
	
	return oss.str();
}

combo_pool& combo_pool::operator=(combo_pool const& other)
{
	clear();
	for(cpool_entry const& cp : other.combos)
	{
		add(cp.cid, cp.cset, cp.quant);
	}
	recalc();
	return *this;
}
void combo_pool::push(int32_t cid, int8_t cs, word q) //add a combo with quantity
{
	if(!q) return;
	totalweight += q;
	for(cpool_entry& cp : combos)
	{
		if(cp.cid == cid && cp.cset == cs)
		{
			cp.quant += q;
			return;
		}
	}
	combos.emplace_back(cid,cs,q);
}
void combo_pool::add(int32_t cid, int8_t cs, word q) //add a new combo entry
{
	totalweight += q;
	combos.emplace_back(cid,cs,q);
}
void combo_pool::swap(size_t ind1, size_t ind2) //Swap 2 combos
{
	if(ind1 >= combos.size()) return;
	if(ind2 >= combos.size()) return;
	
	auto it1 = combos.begin();
	for(size_t q = 0; q < ind1 && it1 != combos.end(); ++q)
		++it1;
	if(it1 == combos.end()) return;
	
	auto it2 = combos.begin();
	for(size_t q = 0; q < ind2 && it2 != combos.end(); ++q)
		++it2;
	if(it2 == combos.end()) return;
	
	cpool_entry cp1 = *it1;
	cpool_entry cp2 = *it2;
	it1 = combos.insert(it1, cp2);
	it1 = combos.erase(++it1);
	//it2 is invalidated, find it again?
	it2 = combos.begin();
	for(size_t q = 0; q < ind2 && it2 != combos.end(); ++q)
		++it2;
	if(it2 == combos.end()) assert(false); //should never happen
	it2 = combos.insert(it2, cp1);
	it2 = combos.erase(++it2);
}
void combo_pool::erase(size_t ind) //Remove a combo
{
	if(ind >= combos.size()) return;
	auto it = combos.begin();
	for(size_t q = 0; q < ind && it != combos.end(); ++q)
		++it;
	if(it == combos.end()) return;
	totalweight -= it->quant;
	combos.erase(it);
}
void combo_pool::trim() //Trim any invalid entries or unneeded space
{
	for(auto it = combos.begin(); it != combos.end();)
	{
		if (!it->valid()) //0 quant or bad cid, remove
		{
			it = combos.erase(it);
			continue;
		}
		
		cpool_entry* entry = &(*it);
		cpool_entry* canon_entry = get(entry->cid, entry->cset);
		if(canon_entry != entry) //Duplicate entry, merge them
		{
			canon_entry->quant += entry->quant;
			it = combos.erase(it);
			continue;
		}
		++it;
	}
	recalc();
}
cpool_entry const* combo_pool::get_ind(size_t index) const
{
	if(index >= combos.size()) return nullptr;
	return &(combos.at(index));
}
cpool_entry const* combo_pool::get_w(size_t weight_index) const
{
	if(!combos.size() || totalweight < 1
		|| weight_index >= size_t(totalweight))
		return nullptr;
	size_t curweight = 0;
	for(cpool_entry const& cp : combos)
	{
		curweight += cp.quant;
		if(weight_index < curweight)
			return &cp;
	}
	return nullptr; //Error?
}
cpool_entry const* combo_pool::pick() const
{
	if(totalweight < 1)
		return nullptr;
	return get_w(zc_rand(totalweight-1));
}
static bool load_entry(cpool_entry const* entry, int32_t& cid, int8_t& cs)
{
	if(entry)
	{
		cid = entry->cid;
		if(entry->cset > -1)
		{
			cs = entry->cset;
		}
		return true;
	}
	return false;
}
bool combo_pool::get_ind(int32_t& cid, int8_t& cs, size_t index) const
{
	return load_entry(get_ind(index), cid, cs);
}
bool combo_pool::get_w(int32_t& cid, int8_t& cs, size_t weight_index) const
{
	return load_entry(get_w(weight_index), cid, cs);
}
bool combo_pool::pick(int32_t& cid, int8_t& cs) const
{
	return load_entry(pick(), cid, cs);
}
void combo_pool::recalc()
{
	totalweight = 0;
	for(cpool_entry const& cp : combos)
	{
		totalweight += cp.quant;
	}
}
cpool_entry* combo_pool::get(int32_t cid, int8_t cs)
{
	for(cpool_entry& cp : combos)
	{
		if(cp.cid == cid && cp.cset == cs)
		{
			return &cp;
		}
	}
	return nullptr;
}


bool zquestheader::is_legacy() const
{
	return new_version_id_main < 2 || (new_version_id_main == 2 && new_version_id_second < 55);
}

int8_t zquestheader::getAlphaState() const
{
	if(new_version_id_release) return 3;
	else if(new_version_id_gamma) return 2;
	else if(new_version_id_beta) return 1;
	else if(new_version_id_alpha) return 0;
	return -1;
}

char const* zquestheader::getAlphaStr(bool ignoreNightly) const
{
	static char buf[40] = "";
	char format[20] = "%s";
	if(!ignoreNightly && new_version_is_nightly) strcpy(format, "Nightly (%s)");
	if(new_version_id_release) sprintf(buf, format, "Release");
	else if(new_version_id_gamma) sprintf(buf, format, "Gamma");
	else if(new_version_id_beta) sprintf(buf, format, "Beta");
	else if(new_version_id_alpha) sprintf(buf, format, "Alpha");
	else sprintf(buf, format, "Unknown");
	return buf;
}

int32_t zquestheader::getAlphaVer() const
{
	if(new_version_id_release) return new_version_id_release;
	else if(new_version_id_gamma) return new_version_id_gamma;
	else if(new_version_id_beta) return new_version_id_beta;
	else if(new_version_id_alpha) return new_version_id_alpha;
	return 0;
}

char const* zquestheader::getAlphaVerStr() const
{
	static char buf[40] = "";
	if(new_version_is_nightly)
	{
		if(getAlphaVer() < 0)
			sprintf(buf, "Nightly (%s ?\?)", getAlphaStr(true));
		else sprintf(buf, "Nightly (%s %d/%d)", getAlphaStr(true), getAlphaVer()-1, getAlphaVer());
	}
	else
	{
		if(getAlphaVer() < 0)
			sprintf(buf, "%s ?\?", getAlphaStr(true));
		else sprintf(buf, "%s %d", getAlphaStr(true), getAlphaVer());
	}
	return buf;
}

char const* zquestheader::getVerStr() const
{
	static char buf[80] = "";
	if(is_legacy())
	{
		switch(zelda_version)
		{
			case 0x254:
				sprintf(buf, "2.54 Build %d", build);
				break;
			case 0x250:
			{
				switch(build)
				{
					case 19:
						strcpy(buf, "2.50.0, Gamma 1"); break;
					case 20:
						strcpy(buf, "2.50.0, Gamma 2"); break;
					case 21:
						strcpy(buf, "2.50.0, Gamma 3"); break;
					case 22:
						strcpy(buf, "2.50.0, Gamma 4"); break;
					case 23:
						strcpy(buf, "2.50.0, Gamma 5"); break;
					case 24:
						strcpy(buf, "2.50.0, Release"); break;
					case 25:
						strcpy(buf, "2.50.1, Gamma 1"); break;
					case 26:
						strcpy(buf, "2.50.1, Gamma 2"); break;
					case 27: 
						strcpy(buf, "2.50.1, Gamma 3"); break;
					case 28:
						strcpy(buf, "2.50.1, Release"); break;
					case 29:
						strcpy(buf, "2.50.2, Release"); break;
					case 30:
						strcpy(buf, "2.50.3, Gamma 1"); break;
					case 31:
						strcpy(buf, "2.53.0, Prior to Gamma 3"); break;
					case 32:
						strcpy(buf, "2.53.0"); break;
					case 33:
						strcpy(buf, "2.53.1"); break;
					default:
						sprintf(buf, "?%x?, Build %d", zelda_version, build); break;
				}
				break;
			}
			case 0x211:
				sprintf(buf, "2.11, Beta %d", build);
				break;
			case 0x210:
			{
				if(build)
					sprintf(buf, "2.10.x Beta/Build %d", build);
				else strcpy(buf, "2.10.x");
				break;
			}
			case 0:
				buf[0] = 0;
				break;
			default:
				sprintf(buf, "Unknown version: '%X, build %d'", zelda_version, build);
				break;
		}
	}
	else if(new_version_id_fourth > 0)
		sprintf(buf, "%d.%d.%d.%d %s", new_version_id_main, new_version_id_second,
			new_version_id_third, new_version_id_fourth, getAlphaVerStr());
	else sprintf(buf, "%d.%d.%d %s", new_version_id_main, new_version_id_second,
			new_version_id_third, getAlphaVerStr());
	return buf;
}

int32_t zquestheader::compareDate() const
{
	zprint2("Comparing dates: '%04d-%02d-%02d %02d:%02d', '%04d-%02d-%02d %02d:%02d'\n",
		new_version_id_date_year, new_version_id_date_month, new_version_id_date_day,
		new_version_id_date_hour, new_version_id_date_minute,
		BUILDTM_YEAR, BUILDTM_MONTH, BUILDTM_DAY, BUILDTM_HOUR, BUILDTM_MINUTE);
	//!TODO handle timezones (build_timezone, __TIMEZONE__)
	if(new_version_id_date_year > BUILDTM_YEAR)
		return 1;
	if(new_version_id_date_year < BUILDTM_YEAR)
		return -1;
	if(new_version_id_date_month > BUILDTM_MONTH)
		return 1;
	if(new_version_id_date_month < BUILDTM_MONTH)
		return -1;
	if(new_version_id_date_day > BUILDTM_DAY)
		return 1;
	if(new_version_id_date_day < BUILDTM_DAY)
		return -1;
	#define BUILDTIME_FUZZ 15
	int32_t time_minutes = (new_version_id_date_hour*60)+new_version_id_date_minute;
	int32_t btm_minutes = (BUILDTM_HOUR*60)+BUILDTM_MINUTE;
	if(time_minutes > btm_minutes+BUILDTIME_FUZZ)
		return 1;
	if(time_minutes < btm_minutes-BUILDTIME_FUZZ)
		return -1;
	return 0;
}

int32_t zquestheader::compareVer() const
{
	if(new_version_id_main > V_ZC_FIRST)
		return 1;
	if(new_version_id_main < V_ZC_FIRST)
		return -1;
	if(new_version_id_second > V_ZC_SECOND)
		return 1;
	if(new_version_id_second < V_ZC_SECOND)
		return -1;
	if(new_version_id_third > V_ZC_THIRD)
		return 1;
	if(new_version_id_third < V_ZC_THIRD)
		return -1;
	if(new_version_id_fourth > V_ZC_FOURTH)
		return 1;
	if(new_version_id_fourth < V_ZC_FOURTH)
		return -1;
	return 0;
}

int8_t getProgramAlphaState()
{
	if(V_ZC_RELEASE) return 3;
	else if(V_ZC_GAMMA) return 2;
	else if(V_ZC_BETA) return 1;
	else if(V_ZC_ALPHA) return 0;
	return -1;
}

int32_t getProgramAlphaVer()
{
	if(V_ZC_RELEASE) return V_ZC_RELEASE;
	else if(V_ZC_GAMMA) return V_ZC_GAMMA;
	else if(V_ZC_BETA) return V_ZC_BETA;
	else if(V_ZC_ALPHA) return V_ZC_ALPHA;
	return 0;
}

char const* getProgramAlphaStr(bool ignoreNightly = false)
{
	static char buf[60] = "";
	char format[20] = "%s";
	if(!ignoreNightly && ZC_IS_NIGHTLY) strcpy(format, "Nightly (%s)");
	if(V_ZC_RELEASE) sprintf(buf, format, "Release");
	else if(V_ZC_GAMMA) sprintf(buf, format, "Gamma");
	else if(V_ZC_BETA) sprintf(buf, format, "Beta");
	else if(V_ZC_ALPHA) sprintf(buf, format, "Alpha");
	else sprintf(buf, format, "Unknown");
	return buf;
}

char const* getProgramAlphaVerStr()
{
	static char buf[100] = "";
	if(ZC_IS_NIGHTLY)
	{
		if(getProgramAlphaVer() < 0)
			sprintf(buf, "Nightly (%s ?\?)", getProgramAlphaStr(true));
		else sprintf(buf, "Nightly (%s %d/%d)", getProgramAlphaStr(true), getProgramAlphaVer()-1, getProgramAlphaVer());
	}
	else
	{
		if(getProgramAlphaVer() < 0)
			sprintf(buf, "%s ?\?", getProgramAlphaStr(true));
		else sprintf(buf, "%s %d", getProgramAlphaStr(true), getProgramAlphaVer());
	}
	return buf;
}

char const* getProgramVerStr()
{
	static char buf[120] = "";
	if(V_ZC_FOURTH > 0)
		sprintf(buf, "%d.%d.%d.%d %s", V_ZC_FIRST, V_ZC_SECOND,
			V_ZC_THIRD, V_ZC_FOURTH, getProgramAlphaVerStr());
	else sprintf(buf, "%d.%d.%d %s", V_ZC_FIRST, V_ZC_SECOND,
			V_ZC_THIRD, getProgramAlphaVerStr());
	return buf;
}

char const* getReleaseTag()
{
#ifdef RELEASE_TAG
	return RELEASE_TAG;
#else
	return getProgramVerStr();
#endif
}

//double ddir=atan2(double(fakey-(Hero.y)),double(Hero.x-fakex));
double WrapAngle( double radians ) 
{
	while (radians <= -PI) radians += (PI*2);
	while (radians > PI) radians -= (PI*2);
	return radians;
}
double WrapDegrees( double degrees )
{
	while (degrees <= -180.0) degrees += 360.0;
	while (degrees > 180.0) degrees -= 360.0;
	return degrees;
}

double DegreesToRadians(double d)
{
	double dvs = PI/180.0;
	return d*dvs;
}

double RadiansToDegrees(double rad)
{
	double dvs = 180.0/PI;
	return rad*dvs;
}

double DirToRadians(int d)
{
	switch(d)
	{
		case up:
			return DegreesToRadians(270);
		case down:
			return DegreesToRadians(90);
		case left:
			return DegreesToRadians(180);
		case right:
			return 0;
		case 4:
			return DegreesToRadians(225);
		case 5:
			return DegreesToRadians(315);
		case 6:
			return DegreesToRadians(135);
		case 7:
			return DegreesToRadians(45);
	}
	return 0;
}

double DirToDegrees(int d)
{
	switch(d)
	{
		case up:
			return 270;
		case down:
			return 90;
		case left:
			return 180;
		case right:
			return 0;
		case 4:
			return 225;
		case 5:
			return 315;
		case 6:
			return 135;
		case 7:
			return 45;
	}
	return 0;
}
int32_t AngleToDir(double ddir)
{
	int32_t lookat=0;
	
	if((ddir<=(((-5)*PI)/8))&&(ddir>(((-7)*PI)/8)))
	{
		lookat=l_up;
	}
	else if((ddir<=(((-3)*PI)/8))&&(ddir>(((-5)*PI)/8)))
	{
		lookat=up;
	}
	else if((ddir<=(((-1)*PI)/8))&&(ddir>(((-3)*PI)/8)))
	{
		lookat=r_up;
	}
	else if((ddir<=(((1)*PI)/8))&&(ddir>(((-1)*PI)/8)))
	{
		lookat=right;
	}
	else if((ddir<=(((3)*PI)/8))&&(ddir>(((1)*PI)/8)))
	{
		lookat=r_down;
	}
	else if((ddir<=(((5)*PI)/8))&&(ddir>(((3)*PI)/8)))
	{
		lookat=down;
	}
	else if((ddir<=(((7)*PI)/8))&&(ddir>(((5)*PI)/8)))
	{
		lookat=l_down;
	}
	else
	{
		lookat=left;
	}
	return lookat;
}
int32_t AngleToDir4(double ddir)
{
	int32_t lookat=0;
	
	if(ddir <= 135.0 && ddir > 45.0)
	{
		lookat = down;
	}
	else if(ddir <= 45.0 && ddir > -45.0)
	{
		lookat = right;
	}
	else if(ddir <= -45.0 && ddir > -135.0)
	{
		lookat = up;
	}
	else
	{
		lookat = left;
	}
	return lookat;
}
int32_t AngleToDir4Rad(double ddir)
{
	int32_t lookat=0;
	ddir = RadiansToDegrees(ddir);
	
	if(ddir <= 135.0 && ddir > 45.0)
	{
		lookat = down;
	}
	else if(ddir <= 45.0 && ddir > -45.0)
	{
		lookat = right;
	}
	else if(ddir <= -45.0 && ddir > -135.0)
	{
		lookat = up;
	}
	else
	{
		lookat = left;
	}
	return lookat;
}


bool isNextType(int32_t type)
{
	switch(type)
	{
		case cLIFTSLASHNEXT:
		case cLIFTSLASHNEXTSPECITEM:
		case cLIFTSLASHNEXTITEM:
		case cDIGNEXT:
		case cLIFTNEXT:
		case cLIFTNEXTITEM:
		case cLIFTNEXTSPECITEM:
		case cSLASHNEXT:
		case cBUSHNEXT:
		case cTALLGRASSNEXT:
		case cSLASHNEXTITEM:
		case cSLASHNEXTTOUCHY:
		case cSLASHNEXTITEMTOUCHY:
		case cBUSHNEXTTOUCHY:
		{
			return true;
		}
		default: return false;
	}
}
bool isWarpType(int32_t type)
{
	switch(type)
	{
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
			return true;
	}
	return false;
}
int32_t getWarpLetter(int32_t type)
{
	switch(type)
	{
		case cSTAIR: case cSWIMWARP: case cDIVEWARP: case cPIT:
		case cAWARPA: case cSWARPA:
			return 0;
		case cSTAIRB: case cSWIMWARPB: case cDIVEWARPB: case cPITB:
		case cAWARPB: case cSWARPB:
			return 1;
		case cSTAIRC: case cSWIMWARPC: case cDIVEWARPC: case cPITC:
		case cAWARPC: case cSWARPC:
			return 2;
		case cSTAIRD: case cSWIMWARPD: case cDIVEWARPD: case cPITD:
		case cAWARPD: case cSWARPD:
			return 3;
		case cSTAIRR: case cPITR: case cAWARPR: case cSWARPR:
			return 4;
	}
	return -1;
}
int32_t simplifyWarpType(int32_t type)
{
	switch(type)
	{
		case cSTAIR: case cSTAIRB: case cSTAIRC: case cSTAIRD: case cSTAIRR:
			return cSTAIR;
		case cSWIMWARP: case cSWIMWARPB: case cSWIMWARPC: case cSWIMWARPD:
			return cSWIMWARP;
		case cDIVEWARP: case cDIVEWARPB: case cDIVEWARPC: case cDIVEWARPD:
			return cDIVEWARP;
		case cPIT: case cPITB: case cPITC: case cPITD: case cPITR:
			return cPIT;
		case cAWARPA: case cAWARPB: case cAWARPC: case cAWARPD: case cAWARPR:
			return cAWARPA;
		case cSWARPA: case cSWARPB: case cSWARPC: case cSWARPD: case cSWARPR:
			return cSWARPA;
	}
	return 0;
}
bool isStepType(int32_t type)
{
	switch(type)
	{
		case cSTEP: case cSTEPSAME:
		case cSTEPALL: case cSTEPCOPY:
			return true;
	}
	return false;
}
bool isDamageType(int32_t type)
{
	switch(type)
	{
		case cDAMAGE1: case cDAMAGE2: case cDAMAGE3: case cDAMAGE4:
		case cDAMAGE5: case cDAMAGE6: case cDAMAGE7:
			return true;
	}
	return false;
}

void zinitdata::clear()
{
	memset(items,0,sizeof(items));
	hc = 0;
	start_heart = 0;
	cont_heart = 0;
	hcp = 0;
	hcp_per_hc = 4;
	keys = 0;
	rupies = 0;
	triforce = 0;
	memset(map,0,sizeof(map));
	memset(compass,0,sizeof(compass));
	memset(boss_key,0,sizeof(boss_key));
	memset(misc,0,sizeof(misc));
	last_map = 0;
	last_screen = 0;
	max_magic = 0;
	magic = 0;
	bomb_ratio = 4;
	msg_more_x = 0;
	msg_more_y = 0;
	msg_more_is_offset = 0;
	subscreen = 0;
	start_dmap = 0;
	heroAnimationStyle = 0;
	memset(level_keys,0,sizeof(level_keys));
	ss_grid_x = 0;
	ss_grid_y = 0;
	ss_grid_xofs = 0;
	ss_grid_yofs = 0;
	ss_grid_color = 0;
	ss_bbox_1_color = 0;
	ss_bbox_2_color = 0;
	ss_flags = 0;
	
	subscreen_style = 0;
	usecustomsfx = 0;
	max_rupees = 255;
	max_keys = 255;
	gravity = 0;
	gravity2 = 0;
	terminalv = 0;
	msg_speed = 0;
	transition_type = 0;
	jump_hero_layer_threshold = 0;
	hero_swim_speed = 0;
	
	bombs = 0;
	super_bombs = 0;
	max_bombs = 0;
	max_sbombs = 0;
	arrows = 0;
	max_arrows = 0;
	heroStep = 0;
	subscrSpeed = 0;
	heroSideswimUpStep = 0;
	heroSideswimSideStep = 0;
	heroSideswimDownStep = 0;
	
	exitWaterJump = 0;

	hp_per_heart = 0;
	magic_per_block = 0;
	hero_damage_multiplier = 0;
	ene_damage_multiplier = 0;
	
	memset(scrcnt,0,sizeof(scrcnt));
	memset(scrmaxcnt,0,sizeof(scrmaxcnt));
	
	swimgravity = 0;
	
	dither_type = 0;
	dither_arg = 0;
	dither_percent = 0;
	def_lightrad = 0;
	transdark_percent = 0;
	darkcol = 0;
	
	bunny_ltm = 0;
	switchhookstyle = 0;
	
	magicdrainrate = 0;
	
	memset(gen_doscript,0,sizeof(gen_doscript));
	memset(gen_exitState,0,sizeof(gen_exitState));
	memset(gen_reloadState,0,sizeof(gen_reloadState));
	memset(gen_initd,0,sizeof(gen_initd));
	memset(gen_dataSize,0,sizeof(gen_dataSize));
	for(auto q = 0; q < NUMSCRIPTSGENERIC; ++q)
	{
		gen_data[q].clear();
	}
	memset(gen_eventstate,0,sizeof(gen_eventstate));
}

void zinitdata::copy(zinitdata const& other)
{
	memcpy(items,other.items,sizeof(items));
	hc = other.hc;
	start_heart = other.start_heart;
	cont_heart = other.cont_heart;
	hcp = other.hcp;
	hcp_per_hc = other.hcp_per_hc;
	keys = other.keys;
	rupies = other.rupies;
	triforce = other.triforce;
	memcpy(map,other.map,sizeof(map));
	memcpy(compass,other.compass,sizeof(compass));
	memcpy(boss_key,other.boss_key,sizeof(boss_key));
	memcpy(misc,other.misc,sizeof(misc));
	last_map = other.last_map;
	last_screen = other.last_screen;
	max_magic = other.max_magic;
	magic = other.magic;
	bomb_ratio = other.bomb_ratio;
	msg_more_x = other.msg_more_x;
	msg_more_y = other.msg_more_y;
	msg_more_is_offset = other.msg_more_is_offset;
	subscreen = other.subscreen;
	start_dmap = other.start_dmap;
	heroAnimationStyle = other.heroAnimationStyle;
	memcpy(level_keys,other.level_keys,sizeof(level_keys));
	ss_grid_x = other.ss_grid_x;
	ss_grid_y = other.ss_grid_y;
	ss_grid_xofs = other.ss_grid_xofs;
	ss_grid_yofs = other.ss_grid_yofs;
	ss_grid_color = other.ss_grid_color;
	ss_bbox_1_color = other.ss_bbox_1_color;
	ss_bbox_2_color = other.ss_bbox_2_color;
	ss_flags = other.ss_flags;
	
	subscreen_style = other.subscreen_style;
	usecustomsfx = other.usecustomsfx;
	max_rupees = other.max_rupees;
	max_keys = other.max_keys;
	gravity = other.gravity;
	gravity2 = other.gravity2;
	terminalv = other.terminalv;
	msg_speed = other.msg_speed;
	transition_type = other.transition_type;
	jump_hero_layer_threshold = other.jump_hero_layer_threshold;
	hero_swim_speed = other.hero_swim_speed;
	
	bombs = other.bombs;
	super_bombs = other.super_bombs;
	max_bombs = other.max_bombs;
	max_sbombs = other.max_sbombs;
	arrows = other.arrows;
	max_arrows = other.max_arrows;
	heroStep = other.heroStep;
	subscrSpeed = other.subscrSpeed;
	heroSideswimUpStep = other.heroSideswimUpStep;
	heroSideswimSideStep = other.heroSideswimSideStep;
	heroSideswimDownStep = other.heroSideswimDownStep;
	
	exitWaterJump = other.exitWaterJump;

	hp_per_heart = other.hp_per_heart;
	magic_per_block = other.magic_per_block;
	hero_damage_multiplier = other.hero_damage_multiplier;
	ene_damage_multiplier = other.ene_damage_multiplier;
	
	memcpy(scrcnt,other.scrcnt,sizeof(scrcnt));
	memcpy(scrmaxcnt,other.scrmaxcnt,sizeof(scrmaxcnt));
	
	swimgravity = other.swimgravity;
	
	dither_type = other.dither_type;
	dither_arg = other.dither_arg;
	dither_percent = other.dither_percent;
	def_lightrad = other.def_lightrad;
	transdark_percent = other.transdark_percent;
	darkcol = other.darkcol;
	
	bunny_ltm = other.bunny_ltm;
	switchhookstyle = other.switchhookstyle;
	
	magicdrainrate = other.magicdrainrate;
	
	memcpy(gen_doscript,other.gen_doscript,sizeof(gen_doscript));
	memcpy(gen_exitState,other.gen_exitState,sizeof(gen_exitState));
	memcpy(gen_reloadState,other.gen_reloadState,sizeof(gen_reloadState));
	memcpy(gen_initd,other.gen_initd,sizeof(gen_initd));
	memcpy(gen_dataSize,other.gen_dataSize,sizeof(gen_dataSize));
	for(auto q = 0; q < NUMSCRIPTSGENERIC; ++q)
	{
		gen_data[q].clear();
		if(other.gen_data[q].size())
			gen_data[q] = other.gen_data[q];
	}
	memcpy(gen_eventstate,other.gen_eventstate,sizeof(gen_eventstate));
}



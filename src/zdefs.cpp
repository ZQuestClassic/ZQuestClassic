
#include "zdefs.h"
#include "jwin.h"
extern PALETTE RAMpal;
extern bool update_hw_pal;

const char months[13][13] =
{ 
	"Nonetober", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
};

char *VerStr(int32_t version)
{
    static char ver_str[12];
    sprintf(ver_str,"v%d.%02X",version>>8,version&0xFF);
    return ver_str;
}

void set_config_standard()
{
	flush_config_file();
	set_config_file(STANDARD_CFG);
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
	push_config_state();
	set_config_file(fpath);
	pal[dvc(1)] = _RGB(zc_get_config("Theme","dvc1_r",4),zc_get_config("Theme","dvc1_g",38),zc_get_config("Theme","dvc1_b",46)); //box fg is text
	pal[dvc(2)] = _RGB(zc_get_config("Theme","dvc2_r",(16*63/255)), zc_get_config("Theme","dvc2_g",(10*63/255)), zc_get_config("Theme","dvc2_b",0));
	pal[dvc(3)] = _RGB(zc_get_config("Theme","dvc3_r",17),zc_get_config("Theme","dvc3_g",20),zc_get_config("Theme","dvc3_b",20)); //slate
	pal[dvc(4)] = _RGB(zc_get_config("Theme","dvc4_r",13),zc_get_config("Theme","dvc4_g",14),zc_get_config("Theme","dvc4_b",14)); //menu background
	pal[dvc(5)] = _RGB(zc_get_config("Theme","dvc5_r",0),zc_get_config("Theme","dvc5_g",0),zc_get_config("Theme","dvc5_b",0));//menu text bg
	pal[dvc(6)] = _RGB(zc_get_config("Theme","dvc6_r",13),zc_get_config("Theme","dvc6_g",14),zc_get_config("Theme","dvc6_b",14));//menu selected text
	pal[dvc(7)] = _RGB(zc_get_config("Theme","dvc7_r",42),zc_get_config("Theme","dvc7_g",60),zc_get_config("Theme","dvc7_b",48));
	pal[dvc(8)] = _RGB(zc_get_config("Theme","dvc8_r",6),zc_get_config("Theme","dvc8_g",49),zc_get_config("Theme","dvc8_b",35));//highlight on selected menu text
					   
	jwin_pal[jcBOX]	=dvc(zc_get_config("Theme","jcbox",4));
	jwin_pal[jcLIGHT]  =dvc(zc_get_config("Theme","jclight",5));
	jwin_pal[jcMEDLT]  =dvc(zc_get_config("Theme","jcmedlt",4));
	jwin_pal[jcMEDDARK]=dvc(zc_get_config("Theme","jcmeddark",3));
	jwin_pal[jcDARK]   =dvc(zc_get_config("Theme","jcdark",2));
	jwin_pal[jcBOXFG]  =dvc(zc_get_config("Theme","jcboxfg",1));
	jwin_pal[jcTITLEL] =dvc(zc_get_config("Theme","jctitlel",3));
	jwin_pal[jcTITLER] =dvc(zc_get_config("Theme","jctitler",5));
	jwin_pal[jcTITLEFG]=dvc(zc_get_config("Theme","jctitlefg",7));
	jwin_pal[jcTEXTBG] =dvc(zc_get_config("Theme","jctextbg",5));
	jwin_pal[jcTEXTFG] =dvc(zc_get_config("Theme","jctextfg",1));
	jwin_pal[jcSELBG]  =dvc(zc_get_config("Theme","jcselbg",8));
	jwin_pal[jcSELFG]  =dvc(zc_get_config("Theme","jcselfg",6));
	jwin_pal[jcCURSORMISC] = dvc(zc_get_config("Theme","jccursormisc",1));
	jwin_pal[jcCURSOROUTLINE] = dvc(zc_get_config("Theme","jccursoroutline",2));
	jwin_pal[jcCURSORLIGHT] = dvc(zc_get_config("Theme","jccursorlight",3));
	jwin_pal[jcCURSORDARK] = dvc(zc_get_config("Theme","jccursordark",5));
	pop_config_state();
	
    gui_bg_color=jwin_pal[jcBOX];
    gui_fg_color=jwin_pal[jcBOXFG];
    gui_mg_color=jwin_pal[jcMEDDARK];
    jwin_set_colors(jwin_pal);
	update_hw_pal = true;
}

void save_themefile(char const* fpath)
{
	save_themefile(fpath, RAMpal);
}
void save_themefile(char const* fpath, PALETTE pal)
{
	push_config_state();
	set_config_file(fpath);
	zc_set_config("Theme","dvc1_r",pal[dvc(1)].r); zc_set_config("Theme","dvc1_g",pal[dvc(1)].g); zc_set_config("Theme","dvc1_b",pal[dvc(1)].b);
	zc_set_config("Theme","dvc2_r",pal[dvc(2)].r); zc_set_config("Theme","dvc2_g",pal[dvc(2)].g); zc_set_config("Theme","dvc2_b",pal[dvc(2)].b);
	zc_set_config("Theme","dvc3_r",pal[dvc(3)].r); zc_set_config("Theme","dvc3_g",pal[dvc(3)].g); zc_set_config("Theme","dvc3_b",pal[dvc(3)].b);
	zc_set_config("Theme","dvc4_r",pal[dvc(4)].r); zc_set_config("Theme","dvc4_g",pal[dvc(4)].g); zc_set_config("Theme","dvc4_b",pal[dvc(4)].b);
	zc_set_config("Theme","dvc5_r",pal[dvc(5)].r); zc_set_config("Theme","dvc5_g",pal[dvc(5)].g); zc_set_config("Theme","dvc5_b",pal[dvc(5)].b);
	zc_set_config("Theme","dvc6_r",pal[dvc(6)].r); zc_set_config("Theme","dvc6_g",pal[dvc(6)].g); zc_set_config("Theme","dvc6_b",pal[dvc(6)].b);
	zc_set_config("Theme","dvc7_r",pal[dvc(7)].r); zc_set_config("Theme","dvc7_g",pal[dvc(7)].g); zc_set_config("Theme","dvc7_b",pal[dvc(7)].b);
	zc_set_config("Theme","dvc8_r",pal[dvc(8)].r); zc_set_config("Theme","dvc8_g",pal[dvc(8)].g); zc_set_config("Theme","dvc8_b",pal[dvc(8)].b);
	zc_set_config("Theme","jcbox",r_dvc(jwin_pal[jcBOX]));
	zc_set_config("Theme","jclight",r_dvc(jwin_pal[jcLIGHT]));
	zc_set_config("Theme","jcmedlt",r_dvc(jwin_pal[jcMEDLT]));
	zc_set_config("Theme","jcmeddark",r_dvc(jwin_pal[jcMEDDARK]));
	zc_set_config("Theme","jcdark",r_dvc(jwin_pal[jcDARK]));
	zc_set_config("Theme","jcboxfg",r_dvc(jwin_pal[jcBOXFG]));
	zc_set_config("Theme","jctitlel",r_dvc(jwin_pal[jcTITLEL]));
	zc_set_config("Theme","jctitler",r_dvc(jwin_pal[jcTITLER]));
	zc_set_config("Theme","jctitlefg",r_dvc(jwin_pal[jcTITLEFG]));
	zc_set_config("Theme","jctextbg",r_dvc(jwin_pal[jcTEXTBG]));
	zc_set_config("Theme","jctextfg",r_dvc(jwin_pal[jcTEXTFG]));
	zc_set_config("Theme","jcselbg",r_dvc(jwin_pal[jcSELBG]));
	zc_set_config("Theme","jcselfg",r_dvc(jwin_pal[jcSELFG]));
	zc_set_config("Theme","jccursormisc",r_dvc(jwin_pal[jcCURSORMISC]));
	zc_set_config("Theme","jccursoroutline",r_dvc(jwin_pal[jcCURSOROUTLINE]));
	zc_set_config("Theme","jccursorlight",r_dvc(jwin_pal[jcCURSORLIGHT]));
	zc_set_config("Theme","jccursordark",r_dvc(jwin_pal[jcCURSORDARK]));
	pop_config_state();
}

void load_udef_colorset(char const* fpath)
{
	load_udef_colorset(fpath, RAMpal);
}
void load_udef_colorset(char const* fpath, PALETTE pal)
{
	push_config_state();
	set_config_file(fpath);
	char const* darkthemename = "themes/dark.ztheme";
	char const* tfnm = zc_get_config("Theme", "theme_filename", "-");
	bool defaulted_theme = !(tfnm[0]&&tfnm[0]!='-');
	strcpy(tmp_themefile, defaulted_theme ? darkthemename : tfnm);
	
	fix_filename_case(tmp_themefile);
	fix_filename_slashes(tmp_themefile);
	if(defaulted_theme
		&& get_config_int("Theme","dvc1_r",4)==get_config_int("Theme","dvc1_r",5))
	{
		//Write these back to the custom theme file
		#ifdef IS_ZQUEST
		strcpy(tmp_themefile, "_custom_zq.ztheme");
		#elif defined(IS_PLAYER)
		strcpy(tmp_themefile, "_custom_zc.ztheme");
		#elif defined(IS_LAUNCHER)
		strcpy(tmp_themefile, "_custom_zcl.ztheme");
		#else
		strcpy(tmp_themefile, "_custom.ztheme");
		#endif
		load_themefile(STANDARD_CFG, pal);
		save_themefile(tmp_themefile, pal);
	}
	else load_themefile(tmp_themefile, pal);
	if (defaulted_theme)
		zc_set_config("Theme", "theme_filename", tmp_themefile);
	pop_config_state();
}

void load_colorset(int32_t colorset)
{
	load_colorset(colorset, RAMpal);
}
void load_colorset(int32_t colorset, PALETTE pal)
{
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
			jwin_pal[jcCURSORMISC] = dvc(1);
			jwin_pal[jcCURSOROUTLINE] = dvc(2);
			jwin_pal[jcCURSORLIGHT] = dvc(3);
			jwin_pal[jcCURSORDARK] = dvc(5);
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
			jwin_pal[jcCURSORMISC] = dvc(1);
			jwin_pal[jcCURSOROUTLINE] = dvc(2);
			jwin_pal[jcCURSORLIGHT] = dvc(3);
			jwin_pal[jcCURSORDARK] = dvc(5);
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
			jwin_pal[jcCURSORMISC] = dvc(1);
			jwin_pal[jcCURSOROUTLINE] = dvc(2);
			jwin_pal[jcCURSORLIGHT] = dvc(3);
			jwin_pal[jcCURSORDARK] = dvc(5);
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
			jwin_pal[jcCURSORMISC] = dvc(1);
			jwin_pal[jcCURSOROUTLINE] = dvc(2);
			jwin_pal[jcCURSORLIGHT] = dvc(3);
			jwin_pal[jcCURSORDARK] = dvc(5);
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
			jwin_pal[jcCURSORMISC] = dvc(1);
			jwin_pal[jcCURSOROUTLINE] = dvc(2);
			jwin_pal[jcCURSORLIGHT] = dvc(3);
			jwin_pal[jcCURSORDARK] = dvc(5);
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
			jwin_pal[jcCURSORMISC] = dvc(1);
			jwin_pal[jcCURSOROUTLINE] = dvc(2);
			jwin_pal[jcCURSORLIGHT] = dvc(3);
			jwin_pal[jcCURSORDARK] = dvc(5);
		}
		break;
		
		case 99:  //User Defined
		{
			load_udef_colorset(STANDARD_CFG, pal);
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
			jwin_pal[jcCURSORMISC] = dvc(1);
			jwin_pal[jcCURSOROUTLINE] = dvc(2);
			jwin_pal[jcCURSORLIGHT] = dvc(3);
			jwin_pal[jcCURSORDARK] = dvc(5);
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
			jwin_pal[jcCURSORMISC] = dvc(1);
			jwin_pal[jcCURSOROUTLINE] = dvc(2);
			jwin_pal[jcCURSORLIGHT] = dvc(3);
			jwin_pal[jcCURSORDARK] = dvc(5);
		}
		break;
	}
	
    gui_bg_color=jwin_pal[jcBOX];
    gui_fg_color=jwin_pal[jcBOXFG];
    gui_mg_color=jwin_pal[jcMEDDARK];
    
    jwin_set_colors(jwin_pal);
}

int32_t zc_get_config(char const* header, char const* name, int32_t default_val)
{
	int32_t ret = get_config_int(header,name,default_val);
	if(ret == default_val) //Might have been defaulted, so write it back
		set_config_int(header, name, ret);
	return ret;
}
double zc_get_config(char const* header, char const* name, double default_val)
{
	double ret = get_config_float(header,name,default_val);
	if(ret == default_val) //Might have been defaulted, so write it back
		set_config_float(header, name, ret);
	return ret;
}
char const* zc_get_config(char const* header, char const* name, char const* default_val)
{
	char const* ret = get_config_string(header,name,default_val);
	if(ret==default_val) //Defaulted, so write it back
	{
		if(default_val[0]) //Writing back the empty string destroys the value?? -Em
			set_config_string(header, name, default_val);
	}
	return ret;
}
void zc_set_config(char const* header, char const* name, int32_t val)
{
	set_config_int(header,name,val);
}
void zc_set_config(char const* header, char const* name, double default_val)
{
	set_config_float(header, name, default_val);
}
void zc_set_config(char const* header, char const* name, char const* val)
{
	set_config_string(header,name,val);
}

int32_t zc_get_config(char const* cfg_file, char const* header, char const* name, int32_t default_val)
{
	push_config_state();
	set_config_file(cfg_file);
	int32_t ret = get_config_int(header,name,default_val);
	if(ret == default_val) //Might have been defaulted, so write it back
		set_config_int(header, name, ret);
	pop_config_state();
	return ret;
}
double zc_get_config(char const* cfg_file, char const* header, char const* name, double default_val)
{
	push_config_state();
	set_config_file(cfg_file);
	double ret = get_config_float(header,name,default_val);
	if(ret == default_val) //Might have been defaulted, so write it back
		set_config_float(header, name, ret);
	pop_config_state();
	return ret;
}
char const* zc_get_config(char const* cfg_file, char const* header, char const* name, char const* default_val)
{
	push_config_state();
	set_config_file(cfg_file);
	char const* ret = get_config_string(header,name,default_val);
	if(ret==default_val) //Defaulted, so write it back
	{
		if(default_val[0]) //Writing back the empty string destroys the value?? -Em
			set_config_string(header, name, default_val);
	}
	pop_config_state();
	return ret;
}
void zc_set_config(char const* cfg_file, char const* header, char const* name, int32_t val)
{
	push_config_state();
	set_config_file(cfg_file);
	set_config_int(header,name,val);
	pop_config_state();
}
void zc_set_config(char const* cfg_file, char const* header, char const* name, double default_val)
{
	push_config_state();
	set_config_file(cfg_file);
	set_config_float(header, name, default_val);
	pop_config_state();
}
void zc_set_config(char const* cfg_file, char const* header, char const* name, char const* val)
{
	push_config_state();
	set_config_file(cfg_file);
	set_config_string(header,name,val);
	pop_config_state();
}

void process_killer::kill(uint32_t exitcode)
{
	if(process_handle)
	{
		TerminateProcess(process_handle, exitcode);
		process_handle = NULL;
	}
}

process_killer launch_process(char const* relative_path)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	GetStartupInfo(&si);
	char path[MAX_PATH];
	strcpy(path, relative_path);
	CreateProcess(NULL,path,NULL,NULL,FALSE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&pi);
	return process_killer(pi.hProcess);
}


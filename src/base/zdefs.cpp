#include "base/zdefs.h"
#include "base/version.h"
#include "gui/jwin_a5.h"
#include "base/zapp.h"
#include "dialog/info.h"
#include <sstream>
#include "sound/zcmusic.h"
#include <fmt/format.h>
#include "base/qrs.h"
#include "base/md5.h"
#include <fstream>

using std::string;
using std::ostringstream;
using namespace util;

extern PALETTE RAMpal;
extern bool update_hw_pal;

volatile bool close_button_quit = false, exiting_program = false, dialog_open_quit = false;

const char months[13][13] =
{ 
	"Nonetober", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
};

char *VerStrFromHex(int32_t version)
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

void convertRGB(RGB& c)
{
	c.r = _rgb_scale_6[c.r];
	c.g = _rgb_scale_6[c.g];
	c.b = _rgb_scale_6[c.b];
}

RGB invRGB(RGB s)
{
	RGB x;
	x.r = 255-s.r;
	x.g = 255-s.g;
	x.b = 255-s.b;
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
	load_themefile(fpath, RAMpal, jwin_a5_colors);
}
#define VER_ZTHEME 1
static void update_theme(int fromver)
{
	if(fromver < 1)
	{
		RGB cols[9];
		cols[1] = _RGB(zc_get_config_basic("Theme","dvc1_r",4),zc_get_config_basic("Theme","dvc1_g",38),zc_get_config_basic("Theme","dvc1_b",46)); //box fg is text
		cols[2] = _RGB(zc_get_config_basic("Theme","dvc2_r",16), zc_get_config_basic("Theme","dvc2_g",10), zc_get_config_basic("Theme","dvc2_b",0));
		cols[3] = _RGB(zc_get_config_basic("Theme","dvc3_r",17),zc_get_config_basic("Theme","dvc3_g",20),zc_get_config_basic("Theme","dvc3_b",20)); //slate
		cols[4] = _RGB(zc_get_config_basic("Theme","dvc4_r",13),zc_get_config_basic("Theme","dvc4_g",14),zc_get_config_basic("Theme","dvc4_b",14)); //menu background
		cols[5] = _RGB(zc_get_config_basic("Theme","dvc5_r",0),zc_get_config_basic("Theme","dvc5_g",0),zc_get_config_basic("Theme","dvc5_b",0));//menu text bg
		cols[6] = _RGB(zc_get_config_basic("Theme","dvc6_r",13),zc_get_config_basic("Theme","dvc6_g",14),zc_get_config_basic("Theme","dvc6_b",14));//menu selected text
		cols[7] = _RGB(zc_get_config_basic("Theme","dvc7_r",42),zc_get_config_basic("Theme","dvc7_g",60),zc_get_config_basic("Theme","dvc7_b",48));
		cols[8] = _RGB(zc_get_config_basic("Theme","dvc8_r",6),zc_get_config_basic("Theme","dvc8_g",49),zc_get_config_basic("Theme","dvc8_b",35));//highlight on selected menu text
		
		for(int q = 1; q <= 8; ++q)
		{
			convertRGB(cols[q]);

			//Clear the old color vars
			zc_set_config_basic("Theme",fmt::format("dvc{}_r",q).c_str(),(char*)nullptr);
			zc_set_config_basic("Theme",fmt::format("dvc{}_g",q).c_str(),(char*)nullptr);
			zc_set_config_basic("Theme",fmt::format("dvc{}_b",q).c_str(),(char*)nullptr);
			//Add the new hex var
			int hexval = ((cols[q].r) << 16) | ((cols[q].g) << 8) | (cols[q].b);
			zc_set_config_basic_hex("Theme",fmt::format("color_{}",q).c_str(),hexval);
		}
	}
	zc_set_config_basic("Theme","ztheme_ver",VER_ZTHEME);
}
struct PaletteDefault
{
	virtual int get() const {return v ? *v : r_dvc(jwin_pal[*i]);}
	static PaletteDefault val(int v)
	{
		return PaletteDefault(v, nullopt);
	}
	static PaletteDefault indx(int v)
	{
		return PaletteDefault(nullopt, v);
	}
private:
	optional<int> v, i;
	PaletteDefault(optional<int> v, optional<int> ind) : v(v), i(ind) {}
};
const char* t_cfg_name[jcMAX] =
{
	"jcbox","jclight","jcmedlt","jcmeddark","jcdark","jcboxfg","jctitlel",
	"jctitler", "jctitlefg", "jctextbg", "jctextfg", "jcselbg","jcselfg",
	"jccursormisc", "jccursoroutline", "jccursorlight", "jccursordark",
	"jc_alt_textfg", "jc_alt_textbg", "jc_disabled_fg", "jc_disabled_bg"
};
const PaletteDefault t_cfg_def[jcMAX] =
{
	#define _PD PaletteDefault
	_PD::val(4),_PD::val(5),_PD::val(4),_PD::val(3),_PD::val(2),_PD::val(1),_PD::val(3),
	_PD::val(5),_PD::val(7),_PD::val(5),_PD::val(1),_PD::val(8),_PD::val(6),
	_PD::val(1),_PD::val(2),_PD::val(3),_PD::val(5),
	_PD::indx(jcMEDDARK),_PD::indx(jcTEXTFG),_PD::indx(jcBOX),_PD::indx(jcMEDDARK)
};
void load_themefile(char const* fpath, PALETTE pal, ALLEGRO_COLOR* colors)
{
	zc_push_config();
	zc_config_file(fpath);
	
	int themever = zc_get_config_basic("Theme","ztheme_ver",0);
	if(themever < VER_ZTHEME)
		update_theme(themever);
	
	for(int q = 1; q <= 8; ++q)
	{
		int hexval = zc_get_config_basic_hex("Theme",fmt::format("color_{}",q).c_str(),0xFF00FF);
		int r = (hexval>>16)&0xFF;
		int g = (hexval>>8)&0xFF;
		int b = (hexval>>0)&0xFF;
		pal[dvc(q)] = _RGB(r,g,b);
		colors[q] = al_map_rgb(r,g,b);
	}
	
	for(int q = 0; q < jcMAX; ++q)
	{
		int v = zc_get_config_basic("Theme",t_cfg_name[q],t_cfg_def[q].get());
		if(v < 1 || v > 8)
		{
			v = vbound(v,1,8);
			zc_set_config_basic("Theme",t_cfg_name[q],v);
		}
		jwin_pal[q] = dvc(v);
	}
	
	zc_pop_config();
	
    gui_bg_color=jwin_pal[jcBOX];
    gui_fg_color=jwin_pal[jcBOXFG];
    jwin_set_colors(jwin_pal);
	update_hw_pal = true;
}

void save_themefile(char const* fpath)
{
	save_themefile(fpath, RAMpal, jwin_a5_colors);
}
void save_themefile(char const* fpath, PALETTE pal, ALLEGRO_COLOR* colors)
{
	zc_push_config();
	zc_config_file(fpath);
	
	zc_set_config_basic("Theme","ztheme_ver",VER_ZTHEME);
	unsigned char r,g,b;
	int hexval;
	for(int q = 1; q <= 8; ++q)
	{
		al_unmap_rgb(colors[q],&r,&g,&b);
		hexval = (r<<16)|(g<<8)|(b);
		zc_set_config_basic_hex("Theme",fmt::format("color_{}",q).c_str(), hexval);
	}
	
	for(int q = 0; q < jcMAX; ++q)
	{
		zc_set_config_basic("Theme",t_cfg_name[q],r_dvc(jwin_pal[q]));
	}
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
	load_udef_colorset(a, RAMpal, jwin_a5_colors);
}
void load_udef_colorset(App a, PALETTE pal, ALLEGRO_COLOR* colors)
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
		load_themefile(get_config_file_name(a), pal, colors);
		save_themefile(tmp_themefile, pal, colors);
	}
	else load_themefile(tmp_themefile, pal, colors);
	if (defaulted_theme)
	{
		zc_set_config("Theme", "theme_filename", tmp_themefile);
	}
}

void load_colorset(int32_t colorset)
{
	load_colorset(colorset, RAMpal, jwin_a5_colors);
}
void load_colorset(int32_t colorset, PALETTE pal, ALLEGRO_COLOR* colors)
{
	bool udef = false;
	switch(colorset)
	{
		case 1:  //Windows 98
		{
			pal[dvc(1)] = _RGB(0,   0,   0);
			pal[dvc(2)] = _RGB(128, 128, 128);
			pal[dvc(3)] = _RGB(192, 192, 192);
			pal[dvc(4)] = _RGB(223, 223, 223);
			pal[dvc(5)] = _RGB(255, 255, 255);
			pal[dvc(6)] = _RGB(255, 255, 225);
			pal[dvc(7)] = _RGB(255, 225, 160);
			pal[dvc(8)] = _RGB(0,   0,  80);
			
			byte palrstart=  0, palrend=166,
				 palgstart=  0, palgend=202,
				 palbstart=128, palbend=240,
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
			pal[dvc(1)] = _RGB(0,   0,   0);
			pal[dvc(2)] = _RGB(64,  64,  64);
			pal[dvc(3)] = _RGB(128, 128, 128);
			pal[dvc(4)] = _RGB(192, 192, 192);
			pal[dvc(5)] = _RGB(223, 223, 223);
			pal[dvc(6)] = _RGB(255, 255, 255);
			pal[dvc(7)] = _RGB(255, 255, 225);
			pal[dvc(8)] = _RGB(255, 225, 160);
			pal[dvc(9)] = _RGB(0,   0,  80);
			
			byte palrstart=  0, palrend=166,
				 palgstart=  0, palgend=202,
				 palbstart=128, palbend=240,
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
			pal[dvc(1)] = _RGB(0,   0,   0);
			pal[dvc(2)] = _RGB(16,  15, 116);
			pal[dvc(3)] = _RGB(82,  80, 182);
			pal[dvc(4)] = _RGB(162, 158, 250);
			pal[dvc(5)] = _RGB(255, 255, 255);
			pal[dvc(6)] = _RGB(255, 255, 127);
			pal[dvc(7)] = _RGB(255, 225,  63);
			pal[dvc(8)] = _RGB(0,   0,  80);
			
			byte palrstart=  0, palrend=162,
				 palgstart=  0, palgend=158,
				 palbstart= 80, palbend=250,
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
			pal[dvc(1)] = _RGB(0,   0,   0);
			pal[dvc(2)] = _RGB(64,  64,  43);
			pal[dvc(3)] = _RGB(170, 154,  96);
			pal[dvc(4)] = _RGB(223, 200, 128); // Old Gold
			pal[dvc(5)] = _RGB(240, 223, 136);
			pal[dvc(6)] = _RGB(255, 223, 128);
			pal[dvc(7)] = _RGB(255, 223, 128);
			pal[dvc(8)] = _RGB(255, 225, 160);
			pal[dvc(9)] = _RGB(80,  80,   0);
			
			byte palrstart=128, palrend=240,
				 palgstart=128, palgend=202,
				 palbstart=  0, palbend=166,
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
			pal[dvc(1)] = _RGB(0,   0,   0);
			pal[dvc(2)] = _RGB(64,  64,  64);
			pal[dvc(3)] = _RGB(128, 128, 128);
			pal[dvc(4)] = _RGB(252, 186, 188);
			pal[dvc(5)] = _RGB(254, 238, 238);
			pal[dvc(6)] = _RGB(244, 243, 161);
			pal[dvc(7)] = _RGB(120, 173, 189);
			pal[dvc(8)] = _RGB(220, 183, 227);
			
			byte palrstart=244, palrend=220,
				 palgstart=243, palgend=183,
				 palbstart=161, palbend=227,
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
			pal[dvc(2)] = _RGB(16, 10, 0);
			pal[dvc(3)] = _RGB(17,20,20); //slate
			pal[dvc(4)] = _RGB(13,14,14); //menu background
			pal[dvc(5)] = _RGB(0,0,0);//menu text bg
			pal[dvc(6)] = _RGB(13,14,14);//menu selected text
			pal[dvc(7)] = _RGB(42,60,48);
			pal[dvc(8)] = _RGB(6,49,35);//highlight on selected menu text
			for (int i = dvc(1); i <= dvc(8); i++)
				convertRGB(pal[i]);

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
			load_udef_colorset(App::undefined, pal, colors);
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
			pal[dvc(2)] = _RGB(16, 10, 0);
			pal[dvc(3)] = _RGB(39,19,0);
			pal[dvc(4)] = _RGB(16,10,0); //menu background
			pal[dvc(5)] = _RGB(0,0,0);
			pal[dvc(6)] = _RGB(0,0,0);
			pal[dvc(7)] = _RGB(255, 225, 160);
			pal[dvc(8)] = _RGB(63,49,0);
			for (int i = dvc(1); i <= dvc(8); i++)
				convertRGB(pal[i]);
			
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
			pal[dvc(1)] = _RGB(0,   0,   0);
			pal[dvc(2)] = _RGB(66,  65,  66);
			pal[dvc(3)] = _RGB(132, 130, 132);
			pal[dvc(4)] = _RGB(212, 208, 200);
			pal[dvc(5)] = _RGB(255, 255, 255);
			pal[dvc(6)] = _RGB(255, 255, 225);
			pal[dvc(7)] = _RGB(255, 225, 160);
			pal[dvc(8)] = _RGB(0,   0,  80);
			
			byte palrstart= 10, palrend=166,
				 palgstart= 36, palgend=202,
				 palbstart=106, palbend=240,
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
		for(int q = jcCURSORMISC; q <= jcDISABLED_BG; ++q)
			jwin_pal[jcCURSORMISC] = dvc(t_cfg_def[q].get());
		
		for(int q = 1; q <= 8; ++q)
		{
			colors[q] = a5color(pal[dvc(q)]);
		}
	}
	
    gui_bg_color=jwin_pal[jcBOX];
    gui_fg_color=jwin_pal[jcBOXFG];
    
    jwin_set_colors(jwin_pal);
}

string get_dbreport_string()
{
	std::stringstream oss;
	
	char buf[256];
	
	oss << "```\n"
		<< "ZQuest Classic Editor"
		<< "\nVersion: " << getVersionString();
		
	sprintf(buf,"Build Date: %s %s, %d at @ %s %s", dayextension(BUILDTM_DAY).c_str(),
		(char*)months[BUILDTM_MONTH], BUILDTM_YEAR, __TIME__, __TIMEZONE__);
	
	oss << "\n" << buf
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
	unpack_qrs();
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
	return ret;
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
	return ret;
}

string generate_zq_about()
{
	char buf1[256];
	std::ostringstream oss;
	oss << "ZQuest Classic Editor" << '\n';
	sprintf(buf1,"Version: %s", getVersionString());
	oss << buf1 << '\n';
	sprintf(buf1,"Build Date: %s %s, %d at @ %s %s", dayextension(BUILDTM_DAY).c_str(), (char*)months[BUILDTM_MONTH], BUILDTM_YEAR, __TIME__, __TIMEZONE__);
	oss << buf1 << '\n';
	
	return oss.str();
}

bool zquestheader::is_legacy() const
{
	return version_major < 2 ||
		(version_major == 2 && version_minor < 55) ||
		(version_major == 2 && version_minor == 55 && new_version_id_release == 0);
}

int8_t zquestheader::getAlphaState() const
{
	if (version_major >= 3)
		return version_patch == 0 ? 3 : 0;

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

	if (version_major >= 3)
		return zelda_version_string;

	if(is_legacy())
	{
		switch(zelda_version)
		{
			case 0x255:
				if(new_version_id_fourth > 0)
					sprintf(buf, "%d.%d.%d.%d %s", version_major, version_minor,
						version_patch, new_version_id_fourth, getAlphaVerStr());
				else sprintf(buf, "%d.%d.%d %s", version_major, version_minor,
						version_patch, getAlphaVerStr());
				break;
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
			case 0x192:
				sprintf(buf, "1.92, Beta %d", build);
				break;
			case 0x190:
			case 0x188:
			case 0x187:
			case 0x186:
			case 0x185:
			case 0x184:
			case 0x183:
			case 0x182:
			case 0x181:
			case 0x180:
				if (build)
					sprintf(buf, "1.%x, Beta/Build %d", zelda_version - 0x100, build);
				else
					sprintf(buf, "1.%x", zelda_version - 0x100);
				break;
			case 0:
				buf[0] = 0;
				break;
			default:
				sprintf(buf, "Unknown version: '%X, build %d'", zelda_version, build);
				break;
		}
	}
	else sprintf(buf, "%d.%d.%d", version_major, version_minor, version_patch);
	return buf;
}

char const* zquestheader::getVerCmpStr() const
{
	static char buf[256];
	int cmp = compareDate();
	sprintf(buf, "'%04d-%02d-%02d %02d:%02d' %s '%04d-%02d-%02d %02d:%02d'\n",
		new_version_id_date_year, new_version_id_date_month, new_version_id_date_day,
		new_version_id_date_hour, new_version_id_date_minute,
		cmp < 0 ? "<" : (cmp ? ">" : "=="),
		BUILDTM_YEAR, BUILDTM_MONTH, BUILDTM_DAY, BUILDTM_HOUR, BUILDTM_MINUTE);
	return buf;
}

int32_t zquestheader::compareDate() const
{
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
	auto version = getVersion();
	if(version_major > version.major)
		return 1;
	if(version_major < version.major)
		return -1;
	if(version_minor > version.minor)
		return 1;
	if(version_minor < version.minor)
		return -1;
	if(version_patch > version.patch)
		return 1;
	if(version_patch < version.patch)
		return -1;
	if(new_version_id_fourth > 0)
		return 1;
	if(new_version_id_fourth < 0)
		return -1;
	return 0;
}

// Returns -1 if zquestheader version is older than given version, 1 if newer, 0 if same.
//
// For example, to check if the currently loaded quest is older than 2.55.9:
//
//    if (QHeader.compareVer(2, 55, 9) < 0)
//
// or at least as new as 2.55.9:
//
//    if (QHeader.compareVer(2, 55, 9) >= 0)
//
int32_t zquestheader::compareVer(int major, int minor, int patch) const
{
	if(version_major > major)
		return 1;
	if(version_major < major)
		return -1;
	if(version_minor > minor)
		return 1;
	if(version_minor < minor)
		return -1;
	if(version_patch > patch)
		return 1;
	if(version_patch < patch)
		return -1;
	return 0;
}

std::string zquestheader::hash() const
{
	if (!_hash.empty())
		return _hash;

	cvs_MD5Context ctx;
	cvs_MD5Init(&ctx);
	size_t buffer_size = 1<<20; // 1 MB
	char *buffer = new char[buffer_size];

	std::ifstream fin(filename, std::ifstream::binary);
	while (fin)
	{
		fin.read(buffer, buffer_size);
		size_t count = fin.gcount();
		if (!count)
			break;
		cvs_MD5Update(&ctx, (const uint8_t*)buffer, count);
	}

	uint8_t md5sum[16];
	cvs_MD5Final(md5sum, &ctx);
	_hash = util::make_hex_string(std::begin(md5sum), std::end(md5sum));
	delete[] buffer;

	return _hash;
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
bool isConveyorType(int32_t type)
{
	switch(type)
	{
		case cCVUP: case cCVDOWN: case cCVLEFT: case cCVRIGHT:
			return true;
	}
	return false;
}
bool isChestType(int32_t type)
{
	switch(type)
	{
		case cCHEST: case cLOCKEDCHEST: case cBOSSCHEST:
		case cCHEST2: case cLOCKEDCHEST2: case cBOSSCHEST2:
			return true;
	}
	return false;
}

int size_and_pos::tw() const
{
	return w*xscale;
}
int size_and_pos::th() const
{
	return h*yscale;
}
int size_and_pos::cx() const
{
	return x < 0 ? x : (x+tw()/2);
}
int size_and_pos::cy() const
{
	return y < 0 ? y : (y+th()/2);
}

void size_and_pos::clear()
{
	*this = size_and_pos();
}
bool size_and_pos::rect(int mx, int my) const
{
	if(x < 0 || y < 0 || w < 0 || h < 0)
		return false;
	auto sw = w * xscale;
	auto sh = h * yscale;
	if(fw > -1 && fh > -1)
		if(mx >= x+fw && my >= y+fh)
			return false;
	return isinRect(mx,my,x,y,x+sw-1,y+sh-1);
}
int size_and_pos::rectind(int mx, int my) const
{
	if(!rect(mx,my)) return -1; //not in rect
	//Where in rect?
	mx -= x;
	my -= y;
	auto row = (my / yscale);
	auto col = (mx / xscale);
	int ind = col + (row * w);
	return ind;
}
void size_and_pos::set(int nx, int ny, int nw, int nh)
{
	x = nx; y = ny;
	w = nw; h = nh;
}
void size_and_pos::set(int nx, int ny, int nw, int nh, int xs, int ys)
{
	x = nx; y = ny;
	w = nw; h = nh;
	xscale = xs; yscale = ys;
}
static size_and_pos nilsqr;
static size_and_pos tempsqr;
size_and_pos const& size_and_pos::subsquare(int ind) const
{
	if(w < 1 || h < 1)
		return nilsqr;
	return subsquare(ind%w, ind/w);
}
size_and_pos const& size_and_pos::subsquare(int col, int row) const
{
	if(w < 1 || h < 1)
		return nilsqr;
	int x2 = x+(col*xscale);
	int y2 = y+(row*yscale);
	if(fw > -1 && fh > -1 && x2 >= x+fw && y2 >= y+fh)
		return nilsqr;
	tempsqr.clear();
	tempsqr.set(x2,y2,xscale,yscale);
	return tempsqr;
}
size_and_pos const& size_and_pos::rel_subsquare(int nx, int ny, int ind) const
{
	if(w < 1 || h < 1)
		return nilsqr;
	return rel_subsquare(nx, ny, ind%w, ind/w);
}
size_and_pos const& size_and_pos::rel_subsquare(int nx, int ny, int col, int row) const
{
	if(w < 1 || h < 1)
		return nilsqr;
	int x2 = (col*xscale);
	int y2 = (row*yscale);
	if(fw > -1 && fh > -1 && x2 >= fw && y2 >= fh)
		return nilsqr;
	tempsqr.clear();
	tempsqr.set(nx+x2,ny+y2,xscale,yscale);
	return tempsqr;
}
size_and_pos::size_and_pos(int nx, int ny, int nw, int nh, int xsc, int ysc, int fw, int fh)
	: x(nx), y(ny), w(nw), h(nh), xscale(xsc), yscale(ysc), fw(fw), fh(fh)
{}

const int BUILDTM_YEAR = (
    __DATE__[7] == '?' ? 1900
    : (((__DATE__[7] - '0') * 1000 )
    + (__DATE__[8] - '0') * 100
    + (__DATE__[9] - '0') * 10
    + __DATE__[10] - '0'));

const int BUILDTM_MONTH = (
    __DATE__ [2] == '?' ? 1
    : __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? 1 : 6)
    : __DATE__ [2] == 'b' ? 2
    : __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? 3 : 4)
    : __DATE__ [2] == 'y' ? 5
    : __DATE__ [2] == 'l' ? 7
    : __DATE__ [2] == 'g' ? 8
    : __DATE__ [2] == 'p' ? 9
    : __DATE__ [2] == 't' ? 10
    : __DATE__ [2] == 'v' ? 11
    : 12);

const int BUILDTM_DAY = (
    __DATE__[4] == '?' ? 1
    : ((__DATE__[4] == ' ' ? 0 :
    ((__DATE__[4] - '0') * 10)) + __DATE__[5] - '0'));

const int BUILDTM_HOUR = (
	(__TIME__[0]-'0')*10 +
	(__TIME__[1]-'0'));

const int BUILDTM_MINUTE = (
	(__TIME__[3]-'0')*10 +
	(__TIME__[4]-'0'));

const int BUILDTM_SECOND = (
	(__TIME__[6]-'0')*10 +
	(__TIME__[7]-'0'));


size_t datcnt = 0;
void dat_callback(DATAFILE*)
{
	++datcnt;
}
DATAFILE* load_datafile_count(const char* path, size_t& sz)
{
	datcnt = 0;
	DATAFILE* ret = load_datafile_callback(path,dat_callback);
	if(ret)
		sz = datcnt-1;
	else sz = 0;
	return ret;
}

int combo_trigger_flag_to_secret_combo_index(int flag)
{
	int ft = -1;

	switch(flag)
	{
	case mfANYFIRE:
		ft=sBCANDLE;
		break;
		
	case mfSTRONGFIRE:
		ft=sRCANDLE;
		break;
		
	case mfMAGICFIRE:
		ft=sWANDFIRE;
		break;
		
	case mfDIVINEFIRE:
		ft=sDIVINEFIRE;
		break;
		
	case mfARROW:
		ft=sARROW;
		break;
		
	case mfSARROW:
		ft=sSARROW;
		break;
		
	case mfGARROW:
		ft=sGARROW;
		break;
		
	case mfSBOMB:
		ft=sSBOMB;
		break;
		
	case mfBOMB:
		ft=sBOMB;
		break;
		
	case mfBRANG:
		ft=sBRANG;
		break;
		
	case mfMBRANG:
		ft=sMBRANG;
		break;
		
	case mfFBRANG:
		ft=sFBRANG;
		break;
		
	case mfWANDMAGIC:
		ft=sWANDMAGIC;
		break;
		
	case mfREFMAGIC:
		ft=sREFMAGIC;
		break;
		
	case mfREFFIREBALL:
		ft=sREFFIREBALL;
		break;
		
	case mfSWORD:
		ft=sSWORD;
		break;
		
	case mfWSWORD:
		ft=sWSWORD;
		break;
		
	case mfMSWORD:
		ft=sMSWORD;
		break;
		
	case mfXSWORD:
		ft=sXSWORD;
		break;
		
	case mfSWORDBEAM:
		ft=sSWORDBEAM;
		break;
		
	case mfWSWORDBEAM:
		ft=sWSWORDBEAM;
		break;
		
	case mfMSWORDBEAM:
		ft=sMSWORDBEAM;
		break;
		
	case mfXSWORDBEAM:
		ft=sXSWORDBEAM;
		break;
		
	case mfHOOKSHOT:
		ft=sHOOKSHOT;
		break;
		
	case mfWAND:
		ft=sWAND;
		break;
		
	case mfHAMMER:
		ft=sHAMMER;
		break;
		
	case mfSTRIKE:
		ft=sSTRIKE;
		break;
	
	case mfSECRETSNEXT:
		ft=sSECNEXT;
		break;
	}

	return ft;
}

bool runscript_do_earlyret(int runscript_val)
{
	switch(runscript_val)
	{
		case RUNSCRIPT_SELFDELETE:
		case RUNSCRIPT_SELFREMOVE:
			return true;
	}
	return false;
}

savedportal::savedportal()
{
	uid = nextuid++;
}

const char* ScriptTypeToString(ScriptType type)
{
	static const char script_types[][19] =
	{
		"none", "global", "ffc", "screendata", "hero", "item", "lweapon", "npc", "subscreen",
		"eweapon", "dmapdata", "itemsprite", "dmapdata (AS)", "dmapdata (PS)", "combodata", "dmapdata (MAP)",
		"generic", "generic (FRZ)", "subscreen (engine)"
	};
	return script_types[(int)type];
}

bool valid_str(char const* ptr, char cancel)
{
	return ptr && ptr[0] && ptr[0] != cancel;
}

void guydata::clear()
{
	*this = guydata();
}


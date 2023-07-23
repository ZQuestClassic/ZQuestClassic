//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  title.cc
//
//  Title screen and intro for zelda.cc
//  Also has game loading and select screen code.
//
//--------------------------------------------------------

#include <filesystem>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <memory>
#include "base/zc_alleg.h"
#include "base/qrs.h"

#include "base/zdefs.h"
#include "zc/zelda.h"
#include "base/zsys.h"
#include "qst.h"
#include "tiles.h"
#include "base/colors.h"
#include "pal.h"
#include "base/gui.h"
#include "sprite.h"
#include "subscr.h"
//#include "jwin.h"
#include "base/jwinfsel.h"
#include "zc/title.h"
#include "gamedata.h"
#include "zc/hero.h"
#include "zc/ffscript.h"
#include "zc/render.h"

#ifdef __EMSCRIPTEN__
#include "base/emscripten_utils.h"
#endif

#ifdef _MSC_VER
#define strupr _strupr
#define stricmp _stricmp
#endif

extern int32_t loadlast;
extern int32_t skipcont;
extern int32_t skipicon;
extern FFScript FFCore;
extern ZModule zcm; //modules
extern zcmodule moduledata;
//extern byte refresh_select_screen;
bool load_custom_game(int32_t file);

struct savedicon
{
	byte loaded;
	byte ring;
	byte pal[4][48];
	byte icon[4][128];
};

savedicon iconbuffer[15];

static bool chosecustomquest = false;

extern char runningItemScripts[256];

/********************************/
/*****   NES title screen   *****/
/********************************/

#define BKGDATA 224*256/32

static byte titlepal[24] =
{
	0x0F,0x00,0x10, 0x17,0x27,0x0F, 0x08,0x1A,0x28, 0x30,0x3B,0x22,
	0x30,0x3B,0x16, 0x17,0x27,0x0F, 0x08,0x1A,0x28, 0x30,0x3B,0x22
};

static byte darkpal1[24] =
{
	0x0F,0x0F,0x00, 0x01,0x11,0x0F, 0x0C,0x01,0x02, 0x00,0x01,0x0C,
	0x00,0x01,0x0C, 0x01,0x11,0x0F, 0x0C,0x01,0x02, 0x00,0x01,0x0C
};

static byte darkpal2[24] =
{
	0x0F,0x0F,0x00, 0x01,0x11,0x0F, 0x0F,0x0C,0x01, 0x01,0x0C,0x0F,
	0x01,0x0C,0x0F, 0x01,0x11,0x0F, 0x0F,0x0C,0x01, 0x01,0x0C,0x0F
};

static byte darkpal3[24] =
{
	0x0F,0x0F,0x0F, 0x01,0x0F,0x0F, 0x0F,0x0C,0x0F, 0x0F,0x0C,0x0F,
	0x0F,0x0C,0x0F, 0x01,0x0F,0x0F, 0x0F,0x0C,0x0F, 0x0F,0x0C,0x0F
};

static byte itemspal[24] =
{
	0x30,0x30,0x30, 0x21,0x30,0x30, 0x16,0x30,0x30, 0x29,0x1A,0x09,
	0x29,0x37,0x17, 0x02,0x22,0x30, 0x16,0x27,0x30, 0x0B,0x1B,0x2B
};

static void loadtitlepal(int32_t clear,byte *dataofs,int32_t shift)
{
	for(int32_t i=0; i<4; i++)
	{
		RAMpal[CSET(i)+shift] = NESpal(clear);
		
		for(int32_t c=1; c<4; c++)
			RAMpal[CSET(i)+c+shift] = NESpal(*dataofs++);
	}
	
	for(int32_t i=6; i<10; i++)
	{
		RAMpal[CSET(i)+shift] = NESpal(clear);
		
		for(int32_t c=1; c<4; c++)
			RAMpal[CSET(i)+c+shift] = NESpal(*dataofs++);
	}
	
	refreshpal=true;
}

static byte tricolor[] = {0x27,0x37,0x27,0x17,0x07,0x17};
static byte tridelay[] = {6,12,6,6,12,16};
static word dusktime[] = {522,522+8,522+14,522+19,522+23,522+26,522+28,522+30};
static byte duskcolor[]= {0x39,0x31,0x3C,0x3B,0x2C,0x1C,0x02,0x0C};
static byte wave[3]= {};

static void cyclewaves()
{
	for(int32_t i=0; i<3; i++)
	{
		wave[i]+=2;
		
		if(wave[i]==50)
			wave[i]=0;
			
		int32_t y=wave[i]+170;
		
		if(wave[i]<8)
		{
			overtile16(framebuf,204,80,y,4,0);
			overtile16(framebuf,205,96,y,4,0);
		}
		
		
		else if(wave[i]<16)
		{
			overtile16(framebuf,206,80,y,4,0);
			overtile16(framebuf,207,96,y,4,0);
		}
		else
		{
			overtile16(framebuf,208,80,y,4,0);
			overtile16(framebuf,209,96,y,4,0);
		}
	}
}

static byte tri,fcnt;

static void mainscreen(int32_t f)
{
	if(f>=1010)
		return;
		
	if(f==0)
	{
		blit((BITMAP*)datafile[BMP_TITLE_NES].dat,scrollbuf,0,0,0,0,256,224);
		blit(scrollbuf,framebuf,0,0,0,0,256,224);
		char tbuf[2048] = {0}; char tbuf2[2048] = {0};
	char copyrbuf[2][2048] = { {0}, {0} };
	//const char *copy_year = (char *)moduledata.copyright_strings[2];
	//const char *copy_s0 =  (char *)moduledata.copyright_strings[0];
	//const char *copy_s1 =  (char *)moduledata.copyright_strings[1];
	if ( moduledata.copyright_strings[0][0] != 0 ) 
	{
		strcpy(tbuf,"(C)");
		strcat(tbuf,moduledata.copyright_strings[0]);
	}
	if ( moduledata.copyright_strings[1][0] != 0 ) 
	{	
		strcpy(tbuf2,"(C)");
		strcat(tbuf2,moduledata.copyright_strings[2]);
		strcat(tbuf2," ");
		strcat(tbuf2,moduledata.copyright_strings[1]);
	}
	sprintf(copyrbuf[0],tbuf,0xBB);
	sprintf(copyrbuf[1],tbuf2,0xBC);
	al_trace("Original title screen. Font: %d\n",moduledata.copyright_string_vars[titleScreenMAIN+0]);
	al_trace("Original title screen. X: %d\n",moduledata.copyright_string_vars[titleScreenMAIN+1]);
	al_trace("Original title screen. Y: %d\n",moduledata.copyright_string_vars[titleScreenMAIN+2]);
	al_trace("Original title screen. col: %d\n",moduledata.copyright_string_vars[titleScreenMAIN+3]);
	al_trace("Original title screen. sz: %d\n",moduledata.copyright_string_vars[titleScreenMAIN+4]);
		
	al_trace("Original title screen. Font2: %d\n",moduledata.copyright_string_vars[titleScreenMAIN+5]);
	al_trace("Original title screen. X2: %d\n",moduledata.copyright_string_vars[titleScreenMAIN+6]);
	al_trace("Original title screen. Y2: %d\n",moduledata.copyright_string_vars[titleScreenMAIN+7]);
	al_trace("Original title screen. col2: %d\n",moduledata.copyright_string_vars[titleScreenMAIN+8]);
	al_trace("Original title screen. sz2: %d\n",moduledata.copyright_string_vars[titleScreenMAIN+9]);
	   // sprintf(tbuf, "%c1986 NINTENDO", 0xBB);
		//sprintf(tbuf, "%c" (char *)copy_s0 , 0xBB);
		//sprintf(tbuf, "%c" (char *)copy_s0 , 0xBB);
		//textout_ex(framebuf,get_zc_font(font_zfont),tbuf,moduledata.copyright_string_vars[1],moduledata.copyright_string_vars[2],moduledata.copyright_string_vars[3],moduledata.copyright_string_vars[4]);
		//al_trace("Font for copyright string 0 is set to: %d",moduledata.copyright_string_vars[0]);
	textout_ex(framebuf,(moduledata.copyright_string_vars[titleScreenMAIN+0] > 0 ? get_zc_font(moduledata.copyright_string_vars[titleScreenMAIN+0]) : get_zc_font(font_zfont)),copyrbuf[0],moduledata.copyright_string_vars[titleScreenMAIN+1],moduledata.copyright_string_vars[titleScreenMAIN+2],moduledata.copyright_string_vars[titleScreenMAIN+3],moduledata.copyright_string_vars[4]);
	   // sprintf(tbuf, "%c" (char *)copy_year (char *)copy_s1, 0xBC);
	 //   sprintf(tbuf, "%c" COPYRIGHT_YEAR " AG", 0xBC);
		//tbuf[0]=(char)0xBC;
	   // textout_ex(framebuf,get_zc_font(font_zfont),tbuf,moduledata.copyright_string_vars[6],moduledata.copyright_string_vars[7],moduledata.copyright_string_vars[8],moduledata.copyright_string_vars[9]);
		textout_ex(framebuf,(moduledata.copyright_string_vars[titleScreenMAIN+5] > 0 ? get_zc_font(moduledata.copyright_string_vars[titleScreenMAIN+5]) : get_zc_font(font_zfont)),copyrbuf[1],moduledata.copyright_string_vars[titleScreenMAIN+6],moduledata.copyright_string_vars[titleScreenMAIN+7],moduledata.copyright_string_vars[titleScreenMAIN+8],moduledata.copyright_string_vars[titleScreenMAIN+9]);
	}
	if ( !moduledata.animate_NES_title ) //if we aren't disabling this
	{
		if(f<554+192+10)
		{
		blit(scrollbuf,framebuf,80,160,80,160,32,64);
		
		if(f&8)
		{
			puttile16(framebuf,200,80,160,4,0);
			puttile16(framebuf,201,96,160,4,0);
		}
		else
		{
			puttile16(framebuf,202,80,160,4,0);
			puttile16(framebuf,203,96,160,4,0);
		}
		
		cyclewaves();
		}
		
		if(f<58*9)
		++fcnt;
		
		if(fcnt==tridelay[tri])
		{
		fcnt=0;
		tri = tri<5 ? tri+1 : 0;
		RAMpal[CSET(2)+2]=NESpal(tricolor[tri]);
		refreshpal=true;
		}
		
		for(int32_t i=0; i<8; i++)
		{
		if(f==dusktime[i])
			loadtitlepal(duskcolor[i],titlepal,4);
		}
		
		if(f==554)
		loadtitlepal(0x0F,darkpal1,4);
		
		if(f==554+192)
		loadtitlepal(0x0F,darkpal2,4);
		
		if(f==554+192+6)
		loadtitlepal(0x0F,darkpal3,4);
		
		if(f==554+192+10)
		{
		clear_bitmap(framebuf);
		clear_bitmap(scrollbuf);
		}
		
		if(f==554+192+10+193)
		loadtitlepal(0x0F,itemspal,0);
	}
}

void putstring(int32_t x,int32_t y,const char* str,int32_t cset)
{
	textout_ex(scrollbuf,get_zc_font(font_zfont),str,x,y,(cset<<CSET_SHFT)+1,0);
}

//#define ii    11
#define ii    14
//static byte vine[5] = { 2,3,6,7,10 };
static byte vine[5] = { 3,6,7,10,11 };

static void storyscreen(int32_t f)
{
	if(f<1010)  return;
	
	if(f>=1804) return;
	
	if(f==1010)
	{
		puttile8(scrollbuf,vine[4],16,232,3,1);
		puttile8(scrollbuf,vine[2],24,232,3,1);
		puttile8(scrollbuf,vine[3],32,232,3,1);
		putstring(48,232,"THE LEGEND OF ZELDA",2);
		puttile8(scrollbuf,vine[3],208,232,3,0);
		puttile8(scrollbuf,vine[2],216,232,3,1);
		puttile8(scrollbuf,vine[3],224,232,3,1);
		puttile8(scrollbuf,vine[4],232,232,3,1);
	}
	
	if(f==16*3 +1010)
		putstring(32,232,"MANY  YEARS  AGO  PRINCE",0);
		
	if(f==16*4 +1010)
	{
		puttile8(scrollbuf,ii,104,232,2,0);
		puttile8(scrollbuf,ii,168,232,2,0);
	}
	
	if(f==16*5 +1010)
	{
		putstring(32,232,"DARKNESS",0);
		putstring(120,232,"GANNON",2);
		putstring(184,232,"STOLE",0);
	}
	
	if(f==16*7 +1010)
	{
		putstring(32,232,"ONE OF THE",0);
		putstring(120,232,"TRIFORCE",1);
		putstring(192,232,"WITH",0);
	}
	
	if(f==16*9 +1010)
	{
		putstring(32,232,"POWER.    PRINCESS",0);
		putstring(184,232,"ZELDA",2);
	}
	
	if(f==16*11 +1010)
	{
		putstring(32,232,"HAD  ONE OF THE",0);
		putstring(160,232,"TRIFORCE",1);
	}
	
	if(f==16*13 +1010)
		putstring(32,232,"WITH WISDOM. SHE DIVIDED",0);
		
	if(f==16*14 +1010)
	{
		puttile8(scrollbuf,ii,88,232,1,0);
		puttile8(scrollbuf,ii,112,232,1,0);
	}
	
	if(f==16*15 +1010)
	{
		putstring(32,232,"IT INTO    UNITS TO HIDE",0);
		putstring(104,232,"8",1);
	}
	
	if(f==16*16 +1010)
	{
		puttile8(scrollbuf,ii,96,232,2,0);
		puttile8(scrollbuf,ii,160,232,2,0);
	}
	
	if(f==16*17 +1010)
	{
		putstring(32,232,"IT FROM",0);
		putstring(112,232,"GANNON",2);
		putstring(176,232,"BEFORE",0);
	}
	
	if(f==16*19 +1010)
		putstring(32,232,"SHE WAS CAPTURED.",0);
		
	if(f==16*20 +1010)
	{
		puttile8(scrollbuf,ii,136,232,2,0);
		puttile8(scrollbuf,ii,160,232,2,0);
	}
	
	if(f==16*21 +1010)
		putstring(48,232,"GO FIND THE  8 UNITS",2);
		
	if(f==16*22 +1010)
	{
		puttile8(scrollbuf,ii,48,232,3,0);
		puttile8(scrollbuf,ii,96,232,3,0);
	}
	
	if(f==16*23 +1010)
	{
		putstring(64,232,"HERO",3);
		putstring(112,232,"TO SAVE HER.",2);
	}
	
	if(f==16*25 +1010)
	{
		puttile8(scrollbuf,vine[4],16,232,3,1);
		
		for(int32_t x=24; x<232; x+=16)
		{
			puttile8(scrollbuf,vine[2],x,232,3,0);
			puttile8(scrollbuf,vine[3],x+8,232,3,0);
		}
		
		puttile8(scrollbuf,vine[4],232,232,3,1);
	}
	
	
	if((f>1010) && (f <= 16*24 + 1010))
	{
		if(((f-1010)&15) == 0)
		{
			if((f-1010)&16)
			{
				puttile8(scrollbuf,vine[0],16,232,3,0);
				puttile8(scrollbuf,vine[1],232,232,3,0);
			}
			else
			{
				puttile8(scrollbuf,vine[1],16,232,3,0);
				puttile8(scrollbuf,vine[0],232,232,3,0);
			}
		}
	}
	
	blit(scrollbuf,framebuf,0,8,0,0,256,224);
	
	if((f>=1010) && (f<= 16*26 + 1010) && (f&1))
		blit(scrollbuf,scrollbuf,0,8,0,7,256,248);
		
}


static int32_t trstr;
static byte tr_items[] =
{
	iHeart,iHeartC,iFairyMoving,iClock,iRupy,i5Rupies,iBPotion,iRPotion,iLetter,iBait,iSword,iWSword,iMSword,iShield,iBrang,iMBrang,iBombs,iBow,iArrow,iSArrow,iBCandle,iRCandle,
	iBRing,iRRing,iBracelet,iWhistle,iRaft,iLadder,iWand,iBook,iKey,iMKey,iMap,iCompass,iTriforce
};

static const char* treasure_str[] =
{
	"ALL OF TREASURES",
	"   HEART     CONTAINER     ",
	"                HEART      ",
	"   FAIRY        CLOCK      ",
	"                           ",
	"   RUPY       5 RUPIES     ",
	"                           ",
	"LIFE POTION   2ND POTION   ",
	"                           ",
	"  LETTER        FOOD       ",
	"                           ",
	"   SWORD        WHITE      ",
	"                SWORD      ",
	"  MAGICAL      MAGICAL     ",
	"   SWORD        SHIELD     ",
	" BOOMERANG     MAGICAL     ",
	"              BOOMERANG    ",
	"   BOMB          BOW       ",
	"                           ",
	"   ARROW        SILVER     ",
	"                ARROW      ",
	"   BLUE          RED       ",
	"   CANDLE       CANDLE     ",
	"   BLUE          RED       ",
	"   RING          RING      ",
	"   POWER       RECORDER    ",
	" BRACELET                  ",
	"   RAFT       STEPLADDER   ",
	"                           ",
	"  MAGICAL      BOOK OF     ",
	"    ROD         MAGIC      ",
	"    KEY        MAGICAL     ",
	"                 KEY       ",
	"    MAP        COMPASS     ",
	"                           ",
	"                           ",
	"        TRIFORCE           "
};

static void treasures(int32_t f)
{
	if(f<1804) return;
	
	if(f>4492) return;
	
	if(f == 1804)
	
	{
		for(int32_t x=0; x<48; x+=16)
		{
			puttile8(scrollbuf,vine[2],x,232,3,0);
			puttile8(scrollbuf,vine[3],x+8,232,3,0);
			puttile8(scrollbuf,vine[2],x+208,232,3,0);
			puttile8(scrollbuf,vine[3],x+216,232,3,0);
		}
		
		puttile8(scrollbuf,vine[4],48,232,3,1);
		puttile8(scrollbuf,vine[4],200,232,3,1);
		putstring(64,232,treasure_str[trstr++],0);
	}
	
	int32_t y = (1820 + 96 + 448 - f) >>1;
	y += f&1;
	
	for(int32_t i=0; i<34; i+=2)
	{
		if((y>=0)&&(y<240))
		{
			if(i==2)                                              // fairy
				puttile16(scrollbuf,((f&4)>>2)+63,68,y,8,0);
			else
				//      putitem(scrollbuf,68,y,tr_items[i],lens_hint_item[tr_items[i]][0], lens_hint_item[tr_items[i]][1], 0);
				//    putitem(scrollbuf,172,y,tr_items[i+1],lens_hint_item[tr_items[i+1]][0],lens_hint_item[tr_items[i+1]][1], 0);
				putitem(scrollbuf,68,y,tr_items[i]);
				
			putitem(scrollbuf,172,y,tr_items[i+1]);
		}
		
		y+=64;
	}
	
	y+=8;
	
	if((y>=0)&&(y<240))
		putitem(scrollbuf,120,y,iTriforce);
		
	y+=80;
	
	if(y>=0)
		for(int32_t i=177; i<=217; i+=20)
		{
			if(y<240)
			{
				puttile16(scrollbuf,i,104,y,6,0);
				puttile16(scrollbuf,i+1,120,y,6,0);
				puttile16(scrollbuf,i+2,136,y,6,0);
			}
			
			y+=16;
		}
		
	if(y<240) //Hero holding item
		puttile16(scrollbuf,29,120,y,6,0);
		
	if(f < 18*8*16 + 1820+128)
	{
		if(((f-1820)&15)==0)
		{
			int32_t ax=(f-1820)>>4;
			
			if((ax&0xF8) && ((ax&7)<2))
				putstring(32,232,treasure_str[trstr++],0);
		}
	}
	
	blit(scrollbuf,framebuf,0,8,0,0,256,224);
}

static void NES_titlescreen()
{
	FFCore.kb_typing_mode = false;
	FFCore.skip_ending_credits = 0;
	/*if ( FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME )
	{
	Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
	midi_volume = FFCore.usr_midi_volume;
//	master_volume(-1,FFCore.usr_midi_volume);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME )
	{
	digi_volume = FFCore.usr_digi_volume;
	//master_volume((int32_t)(FFCore.usr_digi_volume),1);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME )
	{
	emusic_volume = (int32_t)FFCore.usr_music_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME )
	{
	sfx_volume = (int32_t)FFCore.usr_sfx_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
	pan_style = (int32_t)FFCore.usr_panstyle;
	}
	*/
	for ( int32_t q = 0; q < 256; q++ ) runningItemScripts[q] = 0; //Clear scripts that were running before. 

	int32_t f=0;
	bool done=false;
	wave[0]=0;
	wave[1]=16;
	wave[2]=32;
	tri=0;
	fcnt=0;
	trstr=0;
	zc_set_palette(black_palette);
	try_zcmusic((char*)moduledata.base_NSF_file,moduledata.title_track, ZC_MIDI_TITLE);
	clear_bitmap(screen);
	clear_bitmap(framebuf);
	init_NES_mode();
	reset_items(true, &QHeader);
	CSET_SIZE=4;
	CSET_SHFT=2;
	loadtitlepal(0x36,titlepal,4);
	ALLOFF();
	
	do
	{
		mainscreen(f);
		storyscreen(f);
		treasures(f);
		
		if((f>1676)&&(f<=4492)&&(f&1))
			blit(scrollbuf,scrollbuf,0,8,0,7,256,248);
			
		if(f>=4492)
			putitem(framebuf,120,24,iTriforce);
			
		++f;
		
		if((f>4750&&midi_pos<0) || f>6000)
		{
			wave[0]=0;
			wave[1]=16;
			wave[2]=32;
			tri=fcnt=trstr=f=0;
			clear_bitmap(framebuf);
			loadtitlepal(0x36,titlepal,4);
			music_stop();
			try_zcmusic((char*)moduledata.base_NSF_file,moduledata.title_track, ZC_MIDI_TITLE);
		}
		
		advanceframe(true);
		load_control_state();
		
		if(rSbtn())
			done=true;
	}
	while(!done && !Quit);
	
	
	
	music_stop();
	clear_to_color(screen,BLACK);
	CSET_SIZE=16;
	CSET_SHFT=4;
}

/************************************/
/********  DX title screen  *********/
/************************************/

static void DX_mainscreen(int32_t f)
{
	FFCore.kb_typing_mode = false;
	/*if ( FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME )
	{
	Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
	midi_volume = FFCore.usr_midi_volume;
//	master_volume(-1,FFCore.usr_midi_volume);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME )
	{
	digi_volume = FFCore.usr_digi_volume;
	//master_volume((int32_t)(FFCore.usr_digi_volume),1);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME )
	{
	emusic_volume = (int32_t)FFCore.usr_music_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME )
	{
	sfx_volume = (int32_t)FFCore.usr_sfx_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
	pan_style = (int32_t)FFCore.usr_panstyle;
	}
	*/
	FFCore.skip_ending_credits = 0;
	for ( int32_t q = 0; q < 256; q++ ) runningItemScripts[q] = 0; //Clear scripts that were running before. 
	
	set_uformat(U_ASCII);
	
	static int32_t pic=0;
	//char tbuf[80];
	char tbuf[2048] = {0}; char tbuf2[2048] = {0};
	char copyrbuf[2][2048] = { {0}, {0} };
	
	if(f>=1010)
		return;
		
	DATAFILE *dat = (DATAFILE*)datafile[TITLE_DX].dat;
	BITMAP *bmp;
	
	if(f==0)
	{
		copy_pal((RGB*)dat[TITLE_DX_PAL_1].dat,RAMpal);
		refreshpal=true;
	}
	
	if(f<680+256 && (f&15)==0)
	{
		bmp = (BITMAP*)dat[pic<4 ? pic : 6-pic].dat;
		pic = (pic+1)%6;
		blit(bmp,framebuf, 0,0, 0,0, 256,224);
		//    text_mode(-1);
		/*
		sprintf(tbuf, "%c1986 Nintendo",0xBB);
		//tbuf[0]=0xBB;
		textout_ex(framebuf,font,tbuf,46,138,255,-1);
		sprintf(tbuf, "%c" COPYRIGHT_YEAR " Armageddon Games",0xBC);
		//tbuf[0]=0xBC;
		textout_ex(framebuf,font,tbuf,46,146,255,-1);
		
		*/
		  
	//const char *copy_year = (char *)moduledata.copyright_strings[2];
	//const char *copy_s0 =  (char *)moduledata.copyright_strings[0];
	//const char *copy_s1 =  (char *)moduledata.copyright_strings[1];
	if ( moduledata.copyright_strings[0][0] != 0 ) 
	{
		strcpy(tbuf,"(C)");
		strcat(tbuf,moduledata.copyright_strings[0]);
	}
	if ( moduledata.copyright_strings[1][0] != 0 ) 
	{	
		strcpy(tbuf2,"(C)");
		strcat(tbuf2,moduledata.copyright_strings[2]);
		strcat(tbuf2," ");
		strcat(tbuf2,moduledata.copyright_strings[1]);
	}
	/*    
	strcpy(tbuf,"(C)");
	strcat(tbuf,moduledata.copyright_strings[0]);
	strcpy(tbuf2,"(C)");
	strcat(tbuf2,moduledata.copyright_strings[2]);
	strcat(tbuf2," ");
	strcat(tbuf2,moduledata.copyright_strings[1]);
	*/
	sprintf(copyrbuf[0],tbuf,0xBB);
	sprintf(copyrbuf[1],tbuf2,0xBC);
	
	al_trace("2.10 title screen. Font: %d\n",moduledata.copyright_string_vars[titleScreen210+0]);
	al_trace("2.10 title screen. X: %d\n",moduledata.copyright_string_vars[titleScreen210+1]);
	al_trace("2.10 title screen. Y: %d\n",moduledata.copyright_string_vars[titleScreen210+2]);
	al_trace("2.10 title screen. col: %d\n",moduledata.copyright_string_vars[titleScreen210+3]);
	al_trace("2.10 title screen. sz: %d\n",moduledata.copyright_string_vars[titleScreen210+4]);
		
	al_trace("2.10 title screen. Font2: %d\n",moduledata.copyright_string_vars[titleScreen210+5]);
	al_trace("2.10 title screen. X2: %d\n",moduledata.copyright_string_vars[titleScreen210+6]);
	al_trace("2.10 title screen. Y2: %d\n",moduledata.copyright_string_vars[titleScreen210+7]);
	al_trace("2.10 title screen. col2: %d\n",moduledata.copyright_string_vars[titleScreen210+8]);
	al_trace("2.10 title screen. sz2: %d\n",moduledata.copyright_string_vars[titleScreen210+9]);
	   // sprintf(tbuf, "%c1986 NINTENDO", 0xBB);
		//sprintf(tbuf, "%c" (char *)copy_s0 , 0xBB);
		//sprintf(tbuf, "%c" (char *)copy_s0 , 0xBB);
		//textout_ex(framebuf,get_zc_font(font_zfont),tbuf,moduledata.copyright_string_vars[1],moduledata.copyright_string_vars[2],moduledata.copyright_string_vars[3],moduledata.copyright_string_vars[4]);
		al_trace("Font for copyright string 0 is set to: %d",moduledata.copyright_string_vars[0]);
	textout_ex(framebuf,(moduledata.copyright_string_vars[titleScreen210+0] > 0 ? get_zc_font(moduledata.copyright_string_vars[titleScreen210+0]) : get_zc_font(font_zfont)),copyrbuf[0],moduledata.copyright_string_vars[titleScreen210+1],moduledata.copyright_string_vars[titleScreen210+2],moduledata.copyright_string_vars[titleScreen210+3],moduledata.copyright_string_vars[titleScreen210+4]);
	   // sprintf(tbuf, "%c" (char *)copy_year (char *)copy_s1, 0xBC);
	 //   sprintf(tbuf, "%c" COPYRIGHT_YEAR " AG", 0xBC);
		//tbuf[0]=(char)0xBC;
	   // textout_ex(framebuf,get_zc_font(font_zfont),tbuf,moduledata.copyright_string_vars[6],moduledata.copyright_string_vars[7],moduledata.copyright_string_vars[8],moduledata.copyright_string_vars[9]);
		textout_ex(framebuf,(moduledata.copyright_string_vars[titleScreen210+5] > 0 ? get_zc_font(moduledata.copyright_string_vars[titleScreen210+5]) : get_zc_font(font_zfont)),copyrbuf[1],moduledata.copyright_string_vars[titleScreen210+6],moduledata.copyright_string_vars[titleScreen210+7],moduledata.copyright_string_vars[titleScreen210+8],moduledata.copyright_string_vars[titleScreen210+9]);
   
		//    text_mode(0);
	}
	
	if(f>=680 && f<680+256 && (f%3)==0)
	{
		fade_interpolate((RGB*)dat[TITLE_DX_PAL_1].dat,black_palette,RAMpal,
						 (f-680)>>2,0,255);
		refreshpal=true;
	}
	
	if(f==680+256)
	{
		clear_bitmap(framebuf);
		clear_bitmap(scrollbuf);
	}
	
	if(f==680+256+2)
		loadtitlepal(0x0F,itemspal,0);
}

static void DX_titlescreen()
{
	//  JGMOD *yea;
	FFCore.kb_typing_mode = false;
	/*
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME )
	{
	Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
	midi_volume = FFCore.usr_midi_volume;
//	master_volume(-1,FFCore.usr_midi_volume);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME )
	{
	digi_volume = FFCore.usr_digi_volume;
	//master_volume((int32_t)(FFCore.usr_digi_volume),1);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME )
	{
	emusic_volume = (int32_t)FFCore.usr_music_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME )
	{
	sfx_volume = (int32_t)FFCore.usr_sfx_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
	pan_style = (int32_t)FFCore.usr_panstyle;
	}
	*/
	FFCore.skip_ending_credits = 0;
	for ( int32_t q = 0; q < 256; q++ ) runningItemScripts[q] = 0; //Clear scripts that were running before. 

	int32_t f=0;
	bool done=false;
	trstr=0;
	zc_set_palette(black_palette);
	
	try_zcmusic((char*)moduledata.base_NSF_file,moduledata.title_track, ZC_MIDI_TITLE);
	clear_to_color(screen,BLACK);
	clear_bitmap(framebuf);
	init_NES_mode();
	reset_items(true, &QHeader);
	CSET_SIZE=4;
	CSET_SHFT=2;
	ALLOFF();
	clear_keybuf();
	
	do
	{
		DX_mainscreen(f);
		storyscreen(f);
		treasures(f);
		
		if((f>1676)&&(f<=4492)&&(f&1))
			blit(scrollbuf,scrollbuf,0,8,0,7,256,248);
			
		if(f>=4492)
			putitem(framebuf,120,24,iTriforce);
			
		++f;
		
		if((f>4750&&midi_pos<0) || f>6000)
		{
			trstr=f=0;
			clear_bitmap(framebuf);
			music_stop();
			try_zcmusic((char*)moduledata.base_NSF_file,moduledata.title_track, ZC_MIDI_TITLE);
		}
		
		advanceframe(true);
		load_control_state();
		
		if(rSbtn())
			done=true;
	}
	while(!done && !Quit);
	
	music_stop();
	//  stop_mod();
	
	clear_to_color(screen,BLACK);
	CSET_SIZE=16;
	CSET_SHFT=4;
}

/*************************************/
/********  2.5 title screen  *********/
/*************************************/

static void v25_mainscreen(int32_t f)
{
	FFCore.kb_typing_mode = false;
	/*
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME )
	{
	Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
	midi_volume = FFCore.usr_midi_volume;
//	master_volume(-1,FFCore.usr_midi_volume);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME )
	{
	digi_volume = FFCore.usr_digi_volume;
	//master_volume((int32_t)(FFCore.usr_digi_volume),1);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME )
	{
	emusic_volume = (int32_t)FFCore.usr_music_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME )
	{
	sfx_volume = (int32_t)FFCore.usr_sfx_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
	pan_style = (int32_t)FFCore.usr_panstyle;
	}
	*/
	FFCore.skip_ending_credits = 0;
	for ( int32_t q = 0; q < 256; q++ ) runningItemScripts[q] = 0; //Clear scripts that were running before. 
	
	set_uformat(U_ASCII);
	
	static int32_t pic=0;
	//char tbuf[80];
	char tbuf[2048] = {0}; char tbuf2[2048] = {0};
	char copyrbuf[2][2048] = { {0}, {0} };
	if(f>=1010)
		return;
		
	DATAFILE *dat = (DATAFILE*)datafile[TITLE_25].dat;
	BITMAP *bmp;
	
	if(f==0)
	{
		copy_pal((RGB*)dat[TITLE_25_PAL_1].dat,RAMpal);
		refreshpal=true;
	}
	
	if(f<680+256 && (f&15)==0)
	{  
		bmp = (BITMAP*)dat[pic<5 ? pic : 8-pic].dat;
		pic = (pic+1)%8;
		blit(bmp,framebuf, 0,0, 0,0, 256,224);
		//    text_mode(-1);
		/*
		sprintf(tbuf, "%c1986 Nintendo",0xBB);
		//tbuf[0]=0xBB;
		textout_ex(framebuf,font,tbuf,80,134,255,-1);
		sprintf(tbuf, "%c" COPYRIGHT_YEAR " Armageddon Games",0xBC);
		//tbuf[0]=0xBC;
		textout_ex(framebuf,font,tbuf,80,142,255,-1);
		*/
		
	//const char *copy_year = (char *)moduledata.copyright_strings[2];
	//const char *copy_s0 =  (char *)moduledata.copyright_strings[0];
	//const char *copy_s1 =  (char *)moduledata.copyright_strings[1];
	if ( moduledata.copyright_strings[0][0] != 0 ) 
	{
		strcpy(tbuf,"(C)");
		strcat(tbuf,moduledata.copyright_strings[0]);
	}
	if ( moduledata.copyright_strings[1][0] != 0 ) 
	{	
		strcpy(tbuf2,"(C)");
		strcat(tbuf2,moduledata.copyright_strings[2]);
		strcat(tbuf2," ");
		strcat(tbuf2,moduledata.copyright_strings[1]);
	}
	/*
	strcpy(tbuf,"(C)");
	strcat(tbuf,moduledata.copyright_strings[0]);
	strcpy(tbuf2,"(C)");
	strcat(tbuf2,moduledata.copyright_strings[2]);
	strcat(tbuf2," ");
	strcat(tbuf2,moduledata.copyright_strings[1]);
	*/
	sprintf(copyrbuf[0],tbuf,0xBB);
	sprintf(copyrbuf[1],tbuf2,0xBC);
		
	// al_trace("2.50 title screen. Font: %d\n",moduledata.copyright_string_vars[titleScreen250+0]);
	// al_trace("2.50 title screen. X: %d\n",moduledata.copyright_string_vars[titleScreen250+1]);
	// al_trace("2.50 title screen. Y: %d\n",moduledata.copyright_string_vars[titleScreen250+2]);
	// al_trace("2.50 title screen. col: %d\n",moduledata.copyright_string_vars[titleScreen250+3]);
	// al_trace("2.50 title screen. sz: %d\n",moduledata.copyright_string_vars[titleScreen250+4]);
		
	// al_trace("2.50 title screen. Font2: %d\n",moduledata.copyright_string_vars[titleScreen250+5]);
	// al_trace("2.50 title screen. X2: %d\n",moduledata.copyright_string_vars[titleScreen250+6]);
	// al_trace("2.50 title screen. Y2: %d\n",moduledata.copyright_string_vars[titleScreen250+7]);
	// al_trace("2.50 title screen. col2: %d\n",moduledata.copyright_string_vars[titleScreen250+8]);
	// al_trace("2.50 title screen. sz2: %d\n",moduledata.copyright_string_vars[titleScreen250+9]);
	   // sprintf(tbuf, "%c1986 NINTENDO", 0xBB);
		//sprintf(tbuf, "%c" (char *)copy_s0 , 0xBB);
		//sprintf(tbuf, "%c" (char *)copy_s0 , 0xBB);
		//textout_ex(framebuf,get_zc_font(font_zfont),tbuf,moduledata.copyright_string_vars[1],moduledata.copyright_string_vars[2],moduledata.copyright_string_vars[3],moduledata.copyright_string_vars[4]);
		//al_trace("Font for copyright string 0 is set to: %d",moduledata.copyright_string_vars[0]);
	textout_ex(framebuf,(moduledata.copyright_string_vars[titleScreen250+0] > 0 ? get_zc_font(moduledata.copyright_string_vars[titleScreen250+0]) : get_zc_font(font_zfont)),copyrbuf[0],moduledata.copyright_string_vars[titleScreen250+1],moduledata.copyright_string_vars[titleScreen250+2],moduledata.copyright_string_vars[titleScreen250+3],moduledata.copyright_string_vars[titleScreen250+4]);
	   // sprintf(tbuf, "%c" (char *)copy_year (char *)copy_s1, 0xBC);
	 //   sprintf(tbuf, "%c" COPYRIGHT_YEAR " AG", 0xBC);
		//tbuf[0]=(char)0xBC;
	   // textout_ex(framebuf,get_zc_font(font_zfont),tbuf,moduledata.copyright_string_vars[6],moduledata.copyright_string_vars[7],moduledata.copyright_string_vars[8],moduledata.copyright_string_vars[9]);
		textout_ex(framebuf,(moduledata.copyright_string_vars[titleScreen250+5] > 0 ? get_zc_font(moduledata.copyright_string_vars[titleScreen250+5]) : get_zc_font(font_zfont)),copyrbuf[1],moduledata.copyright_string_vars[titleScreen250+6],moduledata.copyright_string_vars[titleScreen250+7],moduledata.copyright_string_vars[titleScreen250+8],moduledata.copyright_string_vars[titleScreen250+9]);
   
		//    text_mode(0);
	}
	
	
	
	if(f>=680 && f<680+256 && (f%3)==0)
	{
		fade_interpolate((RGB*)dat[TITLE_25_PAL_1].dat,black_palette,RAMpal,
						 (f-680)>>2,0,255);
		refreshpal=true;
	}
	
	if(f==680+256)
	{
		clear_bitmap(framebuf);
		clear_bitmap(scrollbuf);
	}
	
	if(f==680+256+2)
		loadtitlepal(0x0F,itemspal,0);
}

static void v25_titlescreen()
{
	FFCore.kb_typing_mode = false;
	/*
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME )
	{
	Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
	midi_volume = FFCore.usr_midi_volume;
//	master_volume(-1,FFCore.usr_midi_volume);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME )
	{
	digi_volume = FFCore.usr_digi_volume;
	//master_volume((int32_t)(FFCore.usr_digi_volume),1);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME )
	{
	emusic_volume = (int32_t)FFCore.usr_music_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME )
	{
	sfx_volume = (int32_t)FFCore.usr_sfx_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
	pan_style = (int32_t)FFCore.usr_panstyle;
	}
	*/
	FFCore.skip_ending_credits = 0;
	for ( int32_t q = 0; q < 256; q++ ) runningItemScripts[q] = 0; //Clear scripts that were running before. 

	//  JGMOD *yea;
	int32_t f=0;
	bool done=false;
	trstr=0;
	zc_set_palette(black_palette);
	
	clear_to_color(screen,BLACK);
	clear_bitmap(framebuf);
	init_NES_mode();
	reset_items(true, &QHeader);
	CSET_SIZE=4;
	CSET_SHFT=2;
	ALLOFF();
	clear_keybuf();
	try_zcmusic((char*)moduledata.base_NSF_file,moduledata.title_track, ZC_MIDI_TITLE);
	
	do
	{
		v25_mainscreen(f);
		storyscreen(f);
		treasures(f);
		
		if((f>1676)&&(f<=4492)&&(f&1))
			blit(scrollbuf,scrollbuf,0,8,0,7,256,248);
			
		if(f>=4492)
			putitem(framebuf,120,24,iTriforce);
			
		++f;
		
		if((f>4750&&midi_pos<0) || f>6000)
		{
			trstr=f=0;
			clear_bitmap(framebuf);
			music_stop();
			try_zcmusic((char*)moduledata.base_NSF_file,moduledata.title_track, ZC_MIDI_TITLE);
		}
		
		advanceframe(true);
		load_control_state();
		
		if(rSbtn())
			done=true;
	}
	while(!done && !Quit);
	
	music_stop();
	//  stop_mod();
	
	clear_to_color(screen,BLACK);
	CSET_SIZE=16;
	CSET_SHFT=4;
}

/***********************************/
/****  Game Selection Screens  *****/
/***********************************/

// first the game saving & loading system

static const char *SAVE_HEADER = "Zelda Classic Save File";
extern char *SAVE_FILE;

int32_t readsaves(gamedata *savedata, PACKFILE *f)
{
	FFCore.kb_typing_mode = false;
	if (get_bit(quest_rules, qr_OLD_SCRIPT_VOLUME))
	{
		if (FFCore.coreflags & FFCORE_SCRIPTED_MIDI_VOLUME)
		{
			Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
			midi_volume = FFCore.usr_midi_volume;
			//	master_volume(-1,FFCore.usr_midi_volume);
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_DIGI_VOLUME)
		{
			digi_volume = FFCore.usr_digi_volume;
			//master_volume((int32_t)(FFCore.usr_digi_volume),1);
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_MUSIC_VOLUME)
		{
			emusic_volume = (int32_t)FFCore.usr_music_volume;
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_SFX_VOLUME)
		{
			sfx_volume = (int32_t)FFCore.usr_sfx_volume;
		}
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
		pan_style = (int32_t)FFCore.usr_panstyle;
	}
	FFCore.skip_ending_credits = 0;
	//word item_count;
	word qstpath_len=0;
	word save_count=0;
	char name[9]={0};
	byte tempbyte = 0;
	int16_t tempshort = 0;
	//  int32_t templong;
	word tempword = 0;
	word tempword2 = 0;
	word tempword3 = 0;
	word tempword4 = 0;
	word tempword5 = 0;
	dword tempdword = 0;
	int32_t templong = 0;
	int32_t section_id=0;
	word section_version=0;
	word section_cversion=0;
	dword section_size = 0;
	
	//section id
	if(!p_mgetl(&section_id,f,true))
	{
		return 1;
	}
	
	//section version info
	if(!p_igetw(&section_version,f,true))
	{
		return 2;
	}
	
	if(!p_igetw(&section_cversion,f,true))
	{
		return 3;
	}
	
	if(section_version < 11) //Sorry!
	{
		//Currently unsupported
		return 1;
	}
	
	//section size
	if(!p_igetl(&section_size,f,true))
	{
		return 4;
	}
	
	if(!p_igetw(&save_count,f,true))
	{
		return 5;
	}
	
	// Excess saves would get deleted, so...
	if(standalone_mode && save_count>1)
	{
		enter_sys_pal();
		jwin_alert("Invalid save file",
				   "This save file cannot be",
				   "used in standalone mode.",
				   "",
				   "OK",NULL,'o',0,get_zc_font(font_lfont));
		exit(0);
	}
	else if(!standalone_mode && save_count==1)
	{
		enter_sys_pal();
		if(jwin_alert3("Standalone save file",
					   "This save file was created in standalone mode.",
					   "If you continue, you will no longer be able",
					   "to use it in standalone mode. Continue anyway?",
					   "No","Yes",NULL, 'n','y', 0, get_zc_font(font_lfont))!=2)
		{
			exit(0);
		}
		exit_sys_pal();
	}
	
	for(int32_t i=0; i<save_count; i++)
	{
		if(!pfread(name,9,f,true))
		{
			return 6;
		}
		
		savedata[i].set_name(name);
		
		if(!p_getc(&tempbyte,f,true))
		{
			return 7;
		}
		
		savedata[i].set_quest(tempbyte);
		
		if(section_version<3)
		{
			if(!p_igetw(&tempword,f,true))
			{
				return 8;
			}
			
			savedata[i].set_counter(tempword, 0);
			savedata[i].set_dcounter(tempword, 0);
			
			if(!p_igetw(&tempword,f,true))
			{
				return 9;
			}
			
			savedata[i].set_maxcounter(tempword, 0);
			
			if(!p_igetw(&tempshort,f,true))
			{
				return 10;
			}
			
			savedata[i].set_dcounter(tempshort, 1);
			
			if(!p_igetw(&tempword,f,true))
			{
				return 11;
			}
			
			savedata[i].set_counter(tempword, 1);
			
			if(!p_igetw(&tempword,f,true))
			{
				return 12;
			}
			
			savedata[i].set_counter(tempword, 3);
			savedata[i].set_dcounter(tempword, 3);
		}
		
		if(!p_igetw(&tempword,f,true))
		{
			return 13;
		}
		
		savedata[i].set_deaths(tempword);
		
		if(section_version<3)
		{
			if(!p_getc(&tempbyte,f,true))
			{
				return 14;
			}
			
			savedata[i].set_counter(tempbyte, 5);
			savedata[i].set_dcounter(tempbyte, 5);
			
			if(!p_getc(&tempbyte,f,true))
			{
				return 15;
			}
			
			savedata[i].set_maxcounter(tempbyte, 2);
		}
		
		if(section_version<4)
		{
			if(!p_getc(&tempbyte,f,true))
			{
				return 16;
			}
			
			savedata[i].set_wlevel(tempbyte);
		}
		
		if(!p_getc(&tempbyte,f,true))
		{
			return 17;
		}
		
		if (section_version < 24) tempbyte = (tempbyte ? DIDCHEAT_BIT : 0);
		savedata[i]._cheat = tempbyte;
		
		char temp;
		
		for(int32_t j=0; j<MAXITEMS; j++) // why not MAXITEMS ?
		{
			if(!p_getc(&temp, f, true))
				return 18;
				
			savedata[i].set_item_no_flush(j, (temp != 0));
		}
		
		size_t versz = section_version<31 ? 9 : 16;
		if(!pfread(savedata[i].version,versz,f,true))
		{
			return 20;
		}
		
		if(!pfread(savedata[i].title,sizeof(savedata[i].title),f,true))
		{
			return 21;
		}
		
		if(!p_getc(&tempbyte,f,true))
		{
			return 22;
		}
		
		savedata[i].set_hasplayed(tempbyte);
		
		if(!p_igetl(&tempdword,f,true))
		{
			return 23;
		}
		
		savedata[i].set_time(tempdword);
		
		if(!p_getc(&tempbyte,f,true))
		{
			return 24;
		}
		
		savedata[i].set_timevalid(tempbyte);
		
		if(section_version <= 5)
		{
			for(int32_t j=0; j<OLDMAXLEVELS; ++j)
			{
				if(!p_getc(&(savedata[i].lvlitems[j]),f,true))
				{
					return 25;
				}
			}
		}
		else
		{
			for(int32_t j=0; j<MAXLEVELS; ++j)
			{
				if(!p_getc(&(savedata[i].lvlitems[j]),f,true))
				{
					return 25;
				}
			}
		}
		
		if(section_version<4)
		{
			if(!p_getc(&tempbyte,f,true))
			{
				return 26;
			}
			
			savedata[i].set_HCpieces(tempbyte);
		}
		
		if(!p_getc(&tempbyte,f,true))
		{
			return 27;
		}
		
		savedata[i].set_continue_scrn(tempbyte);
		
		if(section_version <= 5)
		{
			if(!p_getc(&tempbyte,f,true))
			{
				return 28;
			}
			
			savedata[i].set_continue_dmap(tempbyte);
		}
		else
		{
			if(!p_igetw(&tempword,f,true))
			{
				return 28;
			}
			
			savedata[i].set_continue_dmap(tempword);
		}
		
		if(section_version<3)
		{
			if(!p_igetw(&tempword,f,true))
			{
				return 29;
			}
			
			savedata[i].set_counter(tempword, 4);
			
			if(!p_igetw(&tempword,f,true))
			{
				return 30;
			}
			
			savedata[i].set_maxcounter(tempword, 4);
			
			if(!p_igetw(&tempshort,f,true))
			{
				return 31;
			}
			
			savedata[i].set_dcounter(tempshort, 4);
		}
		
		if(section_version<4)
		{
			if(!p_getc(&tempbyte,f,true))
			{
				return 32;
			}
			
			savedata[i].set_magicdrainrate(tempbyte);
			
			if(!p_getc(&tempbyte,f,true))
			{
				return 33;
			}
			
			savedata[i].set_canslash(tempbyte);
		}
		
		if(section_version <= 5)
		{
			for(int32_t j=0; j<OLDMAXDMAPS; ++j)
			{
				if(!p_getc(&(savedata[i].visited[j]),f,true))
				{
					return 34;
				}
			}
			
			for(int32_t j=0; j<OLDMAXDMAPS*64; ++j)
			{
				byte tempBMaps[OLDMAXDMAPS*64] = {0};
				for(int32_t j=0; j<OLDMAXDMAPS*64; ++j)
				{
					if(!p_getc(&(tempBMaps[j]),f,true))
					{
						return 35;
					}
				}
				std::fill(savedata[i].bmaps, savedata[i].bmaps + MAXDMAPS*128, 0);
				for(int32_t dm = 0; dm < OLDMAXDMAPS; ++dm)
				{
					for(int32_t scr = 0; scr < 128; ++scr)
					{
						int32_t di = (dm<<7) + (scr & 0x70) + (scr&15)-(DMaps[dm].type==dmOVERW ? 0 : DMaps[dm].xoff); //New Calculation
						if(((unsigned)((scr&15)-DMaps[dm].xoff)) > 7) 
							continue;
						int32_t si = ((dm-1)<<6) + ((scr>>4)<<3) + ((scr&15)-DMaps[dm].xoff); //Old Calculation
						if(si < 0)
						{
							savedata[i].bmaps[di] = savedata[i].visited[512+si]&0x8F; //Replicate bug; OOB indexes
							continue;
						}
						savedata[i].bmaps[di] = tempBMaps[si]&0x8F;
					}
				}
			}
		}
		else
		{
			for(int32_t j=0; j<MAXDMAPS; ++j)
			{
				if(!p_getc(&(savedata[i].visited[j]),f,true))
				{
					return 34;
				}
			}
			
			if(section_version < 17)
			{
				byte tempBMaps[MAXDMAPS*64] = {0};
				for(int32_t j=0; j<MAXDMAPS*64; ++j)
				{
					if(!p_getc(&(tempBMaps[j]),f,true))
					{
						return 35;
					}
				}
				std::fill(savedata[i].bmaps, savedata[i].bmaps + MAXDMAPS*128, 0);
				for(int32_t dm = 0; dm < MAXDMAPS; ++dm)
				{
					for(int32_t scr = 0; scr < 128; ++scr)
					{
						int32_t di = (dm<<7) + (scr & 0x70) + (scr&15)-(DMaps[dm].type==dmOVERW ? 0 : DMaps[dm].xoff); //New Calculation
						if(((unsigned)((scr&15)-DMaps[dm].xoff)) > 7) 
							continue;
						int32_t si = ((dm-1)<<6) + ((scr>>4)<<3) + ((scr&15)-DMaps[dm].xoff); //Old Calculation
						if(si < 0)
						{
							savedata[i].bmaps[di] = savedata[i].visited[512+si]&0x8F; //Replicate bug; OOB indexes
							continue;
						}
						savedata[i].bmaps[di] = tempBMaps[si]&0x8F;
					}
				}
			}
			else
			{
				for(int32_t j=0; j<MAXDMAPS*128; ++j)
				{
					if(!p_getc(&(savedata[i].bmaps[j]),f,true))
					{
						return 35;
					}
				}
			}
		}
		
		for(int32_t j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
		{
			if(!p_igetw(&savedata[i].maps[j],f,true))
			{
				return 36;
			}
		}
		
		for(int32_t j=0; j<MAXMAPS2*MAPSCRSNORMAL; ++j)
		{
			if(!p_getc(&(savedata[i].guys[j]),f,true))
			{
				return 37;
			}
		}
		
		if(!p_igetw(&qstpath_len,f,true))
		{
			return 38;
		}
		
		if(!pfread(savedata[i].qstpath,qstpath_len,f,true))
		{
			return 39;
		}
		
		if(standalone_mode && strcmp(savedata[i].qstpath, standalone_quest)!=0)
		{
			enter_sys_pal();
			jwin_alert("Invalid save file",
					   "This save file is for",
					   "a different quest.",
					   "",
					   "OK",NULL,'o',0,get_zc_font(font_lfont));
			exit(0);
		}
		
		// Convert path separators so save files work across platforms (hopefully)
		regulate_path(savedata[i].qstpath);
		
		savedata[i].qstpath[qstpath_len]=0;
		
		if(!pfread(savedata[i].icon,sizeof(savedata[i].icon),f,true))
		{
			return 40;
		}
		
		if(!pfread(savedata[i].pal,sizeof(savedata[i].pal),f,true))
		{
			return 41;
		}
		
		if(section_version <= 5)
		{
			for(int32_t j=0; j<OLDMAXLEVELS; ++j)
			{
				if(!p_getc(&(savedata[i].lvlkeys[j]),f,true))
				{
					return 42;
				}
			}
		}
		else
		{
			for(int32_t j=0; j<MAXLEVELS; ++j)
			{
				if(!p_getc(&(savedata[i].lvlkeys[j]),f,true))
				{
					return 42;
				}
			}
		}
		
		if(section_version>1)
		{
			if(section_version <= 5)
			{
				for(int32_t j=0; j<OLDMAXDMAPS*64; j++)
				{
					for(int32_t k=0; k<8; k++)
					{
						if(!p_igetl(&savedata[i].screen_d[j][k],f,true))
						{
							return 43;
						}
					}
				}
			}
			else if(section_version < 10)
			{
				for(int32_t j=0; j<MAXDMAPS*64; j++)
				{
					for(int32_t k=0; k<8; k++)
					{
						if(!p_igetl(&savedata[i].screen_d[j][k],f,true))
						{
							return 43;
						}
					}
				}
			}
			else
			{
				for(int32_t j=0; j<MAX_MI; j++)
				{
					for(int32_t k=0; k<8; k++)
					{
						if(!p_igetl(&savedata[i].screen_d[j][k],f,true))
						{
							return 43;
						}
					}
				}
			}
			if ( section_version >= 12 && FFCore.getQuestHeaderInfo(vZelda) >= 0x253 || section_version >= 16)
			/* 2.53.1 also have a v12 for this section. 
			I needed to path this to ensure that the s_v is specific to the build.
			I also skipped 13 to 15 so that 2.53.1 an use these if needed with the current patch. -Z
			*/
			{
				for(int32_t j=0; j<MAX_SCRIPT_REGISTERS; j++)
				{
				if(!p_igetl(&savedata[i].global_d[j],f,true))
				{
					return 45;
				}
				}
			}
			else
			{
				for(int32_t j=0; j<256; j++)
				{
					if(!p_igetl(&savedata[i].global_d[j],f,true))
					{
						return 45;
					}
				}
			}
		}
		
		if(section_version>2)
		{
			for(int32_t j=0; j<32; j++)
			{
				if(!p_igetw(&tempword,f,true))
				{
					return 46;
				}
				
				savedata[i].set_counter(tempword, j);
				
				if(!p_igetw(&tempword,f,true))
				{
					return 47;
				}
				
				savedata[i].set_maxcounter(tempword, j);
				
				if(!p_igetw(&tempshort,f,true))
				{
					return 48;
				}
				
				savedata[i].set_dcounter(tempshort, j);
			}
		}
		
		if(section_version>19)
		{
			for(int32_t j=0; j<256; j++)
			{
				if(!p_igetl(&templong,f,true))
				{
					return 49;
				}
				
				savedata[i].set_generic(templong, j);
			}
		}
		else if(section_version>3)
		{
			for(int32_t j=0; j<256; j++)
			{
				if(!p_getc(&tempbyte,f,true))
				{
					return 49;
				}
				
				savedata[i].set_generic(tempbyte, j);
			}
		}
		
		if(section_version>6)
		{
			if(!p_getc(&tempbyte, f, true))
			{
				return 50;
			}
			
			savedata[i].awpn = tempbyte;
			
			if(!p_getc(&tempbyte, f, true))
			{
				return 51;
			}
			
			savedata[i].bwpn = tempbyte;
		}
		else
		{
			savedata[i].awpn = 0;
			savedata[i].bwpn = 0;
		}
		
		//First we get the size of the vector
		if(!p_igetl(&tempdword, f, true))
			return 53;
			
		if(tempdword != 0) //Might not be any at all
		{
			//Then we allocate the vector
			savedata[i].globalRAM.resize(tempdword);
			
			for(dword j = 0; j < savedata[i].globalRAM.size(); j++)
			{
				ZScriptArray& a = savedata[i].globalRAM[j];
				
				//We get the size of each container
				if(!p_igetl(&tempdword, f, true))
					return 54;
					
				//We allocate the container
				a.Resize(tempdword);
				
				//And then fill in the contents
				for(dword k = 0; k < a.Size(); k++)
					if(!p_igetl(&(a[k]), f, true))
						return 55;
			}
		}
		if((section_version > 11 && FFCore.getQuestHeaderInfo(vZelda) < 0x255) || (section_version > 15 && FFCore.getQuestHeaderInfo(vZelda) >= 0x255))
		{
			if(!p_igetw(&tempword2, f, true))
			{
				return 56;
			}
			
			savedata[i].forced_awpn = tempword2;
			
			if(!p_igetw(&tempword3, f, true))
			{
				return 57;
			}
			
			savedata[i].forced_bwpn = tempword3;
		}
		else
		{
			savedata[i].forced_awpn = -1;
			savedata[i].forced_bwpn = -1;
		}
		if (section_version > 17)
		{
			
			if(!p_getc(&tempbyte, f, true))
			{
				return 58;
			}
			
			savedata[i].xwpn = tempbyte;
			
			if(!p_getc(&tempbyte, f, true))
			{
				return 59;
			}
			
			savedata[i].ywpn = tempbyte;
		
			
			if(!p_igetw(&tempword3, f, true))
			{
				return 60;
			}
			
			savedata[i].forced_xwpn = tempword3;
			
			if(!p_igetw(&tempword4, f, true))
			{
				return 61;
			}
			
			savedata[i].forced_ywpn = tempword4;
		}
		else
		{
			savedata[i].xwpn = 0;
			savedata[i].ywpn = 0;
			savedata[i].forced_xwpn = -1;
			savedata[i].forced_ywpn = -1;
		}
		
		if(section_version >= 19)
		{
			for(int32_t j=0; j<MAXLEVELS; ++j)
			{
				if(!p_igetl(&(savedata[i].lvlswitches[j]),f,true))
				{
					return 62;
				}
			}
		}
		else
		{
			std::fill(savedata[i].lvlswitches, savedata[i].lvlswitches+MAXLEVELS, 0);
		}
		if(section_version >= 21)
		{
			for(int32_t j=0; j<MAXITEMS; ++j)
			{
				if(!p_getc(&(savedata[i].item_messages_played[j]),f,true))
				{
					return 63;
				}
			}
		}
		else 
		{
			std::fill(savedata[i].item_messages_played, savedata[i].item_messages_played+MAXITEMS, 0);
		}
		if(section_version >= 22)
		{
			for(int32_t j=0; j<256; ++j)
			{
				if(!p_getc(&(savedata[i].bottleSlots[j]),f,true))
				{
					return 64;
				}
			}
		}
		else 
		{
			memset(savedata[i].bottleSlots, 0, sizeof(savedata[i].bottleSlots));
		}
		if(section_version >= 23)
		{
			if(!p_igetw(&(savedata[i].saved_mirror_portal.destdmap), f, true))
			{
				return 65;
			}
			if(!p_igetw(&(savedata[i].saved_mirror_portal.srcdmap), f, true))
			{
				return 66;
			}
			if(!p_getc(&(savedata[i].saved_mirror_portal.srcscr),f,true))
			{
				return 67;
			}
			if(section_version >= 32)
			{
				if(!p_getc(&(savedata[i].saved_mirror_portal.destscr),f,true))
				{
					return 67;
				}
			}
			else savedata[i].saved_mirror_portal.destscr = savedata[i].saved_mirror_portal.srcscr;
			if(!p_igetl(&(savedata[i].saved_mirror_portal.x), f, true))
			{
				return 68;
			}
			if(!p_igetl(&(savedata[i].saved_mirror_portal.y), f, true))
			{
				return 69;
			}
			if(!p_getc(&(savedata[i].saved_mirror_portal.sfx),f,true))
			{
				return 70;
			}
			if(!p_igetl(&(savedata[i].saved_mirror_portal.warpfx), f, true))
			{
				return 71;
			}
			if(!p_igetw(&(savedata[i].saved_mirror_portal.spr), f, true))
			{
				return 72;
			}
		}
		else savedata[i].saved_mirror_portal.clear();
		
		savedata[i].clear_genscript();
		
		word num_gen_scripts;
		if(section_version >= 25)
		{
			byte dummybyte;
			if(!p_igetw(&num_gen_scripts, f, true))
			{
				return 73;
			}
			for(size_t q=0; q<num_gen_scripts; q++)
			{
				if(!p_getc(&dummybyte,f, true))
					return 74;
				savedata[i].gen_doscript[q] = dummybyte!=0;
				if(!p_igetw(&(savedata[i].gen_exitState[q]),f, true))
					return 75;
				if(!p_igetw(&(savedata[i].gen_reloadState[q]),f, true))
					return 76;
				for(size_t ind = 0; ind < 8; ++ind)
					if(!p_igetl(&(savedata[i].gen_initd[q][ind]),f, true))
						return 77;
				int32_t sz;
				if(!p_igetl(&sz,f, true))
					return 78;
				savedata[i].gen_dataSize[q] = sz;
				savedata[i].gen_data[q].resize(sz, 0);
				for(auto ind = 0; ind < sz; ++ind)
					if(!p_igetl(&(savedata[i].gen_data[q][ind]),f, true))
						return 79;
			}
		}
		
		if(section_version >= 26)
		{
			for(int32_t j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
			{
				if(!p_igetl(&(savedata[i].xstates[j]),f,true))
				{
					return 78;
				}
			}
		}
		else
		{
			std::fill(savedata[i].xstates, savedata[i].xstates+(MAXMAPS2*MAPSCRSNORMAL), 0);
		}
		
		std::fill(savedata[i].gen_eventstate, savedata[i].gen_eventstate+NUMSCRIPTSGENERIC, 0);
		if(section_version >= 27)
		{
			for(size_t q=0; q<num_gen_scripts; q++)
			{
				if(!p_igetl(&savedata[i].gen_eventstate[q],f, true))
					return 78;
			}
		}
		if(section_version >= 28)
		{
			for(size_t q = 0; q < NUM_GSWITCHES; ++q)
			{
				if(!p_igetl(&savedata[i].gswitch_timers[q],f, true))
					return 79;
			}
		}
		else
		{
			std::fill(savedata[i].gswitch_timers, savedata[i].gswitch_timers+NUM_GSWITCHES, 0);
		}
		if(section_version >= 29)
		{
			word replay_path_len;
			if(!p_igetw(&replay_path_len, f, true))
				return 80;

			auto buf = std::make_unique<char[]>(replay_path_len + 1);
			buf[replay_path_len] = '\0';
			if (!pfread(buf.get(), replay_path_len, f, true))
				return 81;
			savedata[i].replay_file = buf.get();

			// TODO why doesn't this work?
			// savedata[i].replay_file.reserve(replay_path_len + 1);
			// if (!pfread(savedata[i].replay_file.data(), replay_path_len, f, true))
			// 	return 81;
		}
		else
		{
			savedata[i].replay_file = "";
		}
		if(section_version >= 30)
		{
			uint32_t sz;
			if(!p_igetl(&sz,f,true))
				return 83;
			for(uint32_t objind = 0; objind < sz; ++objind)
			{
				saved_user_object& s_ob = savedata[i].user_objects.emplace_back();
				if(!p_igetl(&s_ob.object_index,f,true))
					return 84;
				//user_object
				user_object& obj = s_ob.obj;
				if(!p_getc(&tempbyte,f,true))
					return 85;
				obj.reserved = tempbyte!=0;
				//Don't need to save owned_type,owned_i?
				uint32_t datsz;
				if(!p_igetl(&datsz,f,true))
					return 86;
				for(uint32_t q = 0; q < datsz; ++q)
				{
					if(!p_igetl(&templong,f,true))
						return 87;
					obj.data.push_back(templong);
				}
				if(!p_igetl(&obj.owned_vars,f,true))
					return 88;
				//scr_func_exec
				scr_func_exec& exec = obj.destruct;
				if(!p_igetl(&exec.pc,f,true))
					return 89;
				if(!p_igetl(&exec.thiskey,f,true))
					return 90;
				if(!p_igetl(&exec.type,f,true))
					return 91;
				if(!p_igetl(&exec.i,f,true))
					return 92;
				if(!p_igetw(&exec.script,f,true))
					return 93;
				if(!p_getwstr(&exec.name,f,true))
					return 94;
				//array data map
				auto& map = s_ob.held_arrays;
				uint32_t arrcount;
				if(!p_igetl(&arrcount,f,true))
					return 95;
				for(uint32_t ind = 0; ind < arrcount; ++ind)
				{
					int32_t arr_index;
					if(!p_igetl(&arr_index,f,true))
						return 96;
					
					uint32_t arrsz;
					if(!p_igetl(&arrsz,f,true))
						return 97;
					ZScriptArray zsarr;
					zsarr.Resize(arrsz);
					for(uint32_t q = 0; q < arrsz; ++q)
					{
						if(!p_igetl(&templong,f,true))
							return 98;
						zsarr[q] = templong;
					}
					map[arr_index] = zsarr;
				}
			}
		}
		if(section_version >= 32)
		{
			uint32_t sz;
			if(!p_igetl(&sz,f,true))
				return 99;
			for(uint32_t q = 0; q < sz; ++q)
			{
				savedportal& p = savedata[i].user_portals.emplace_back();
				if(!p_igetw(&(p.destdmap), f, true))
					return 100;
				if(!p_igetw(&(p.srcdmap), f, true))
					return 101;
				if(!p_getc(&(p.srcscr),f,true))
					return 102;
				if(!p_getc(&(p.destscr),f,true))
					return 103;
				if(!p_igetl(&(p.x), f, true))
					return 104;
				if(!p_igetl(&(p.y), f, true))
					return 105;
				if(!p_getc(&(p.sfx),f,true))
					return 106;
				if(!p_igetl(&(p.warpfx), f, true))
					return 107;
				if(!p_igetw(&(p.spr), f, true))
					return 108;
			}
		}
	}
	
	
	return 0;
}

void set_up_standalone_save()
{
	char *fn=get_filename(standalone_quest);
	saves[0].set_name(fn);
	
	qstpath=(char*)malloc(2048);
	strncpy(qstpath, standalone_quest, 2047);
	qstpath[2047]='\0';
	chosecustomquest=true;
	load_custom_game(0);
	
	// Why does the continue screen need set when
	// everything else gets set automatically?
	saves[0].set_continue_dmap(0);
	saves[0].set_continue_scrn(0xFF);
	saves[0].set_hasplayed(false);
	
	load_game_icon_to_buffer(false, 0);
	load_game_icon(saves, true, 0);
}

int32_t init_saves()
{
	if(saves == NULL)
	{
		saves = new gamedata[MAXSAVES];
		for (int i = 0; i < MAXSAVES; i++)
			saves[i].Clear();
		
		if(saves==NULL)
			return 1;
	}
	return 0;
}

// call once at startup
int32_t load_savedgames()
{
	FFCore.kb_typing_mode = false;
	FFCore.skip_ending_credits = 0;
	char *fname = SAVE_FILE;
	char *iname = (char *)malloc(2048);
	int32_t ret;
	PACKFILE *f=NULL;
	FILE *f2=NULL;
	char tmpfilename[L_tmpnam];
	temp_name(tmpfilename);
//  const char *passwd = datapwd;

	int32_t save_ret = init_saves();
	if(save_ret) return save_ret;
	enter_sys_pal();
	
	// see if it's there
	if(!exists(fname))
	{
		goto newdata;
	}
	
	if(file_size_ex_password(fname, "") == 0)
	{
		if(errno==0) // No error, file's empty
		{
			goto init;
		}
		else // Error...
		{
			goto cantopen;
		}
	}
	
	// decode to temp file
#ifdef __EMSCRIPTEN__
    if (em_is_lazy_file(fname))
    {
        em_fetch_file(fname);
    }
#endif
	ret = decode_file_007(fname, tmpfilename, SAVE_HEADER, ENC_METHOD_MAX-1, strstr(fname, ".dat#")!=NULL, "");
	
	if(ret)
	{
		goto cantopen;
	}
	
	fname = tmpfilename;
	
	// load the games
	f = pack_fopen_password(fname, F_READ_PACKED, "");
	
	if(!f)
		goto cantopen;
		
	if(readsaves(saves,f)!=0)
		goto reset;
		
		strcpy(iname, zc_get_config("SAVEFILE","save_filename","zc.sav"));
	
	for(int32_t i=0; iname[i]!='\0'; iname[i]=='.'?iname[i]='\0':i++)
	{
		/* do nothing */
	}
	
	strcat(iname,".icn");
	
	if(!exists(iname))
	{
		byte *di2 = (byte *)iconbuffer;
		
		for(dword i=0; i<sizeof(savedicon)*MAXSAVES; i++)
			*(di2++) = 0;
	}
	else
	{
		f2=fopen(iname,"rb");
		byte *di2 = (byte *)iconbuffer;
		
		for(dword i=0; (i<sizeof(savedicon)*MAXSAVES)&&!feof(f2); i++)
			*(di2++) = fgetc(f2);
			
		fclose(f2);
	}
	
	//Load game icons
	for(int32_t i=0; i<MAXSAVES; i++)
	{
		byte showmetadata = zc_get_config("zeldadx","print_metadata_for_each_save_slot",0);
		//zprint2("Reading Save Slot %d\n", i);
		
		if(strlen(saves[i].qstpath))
		{
			if(skipicon)
			{
				for(int32_t j=0; j<128; j++)
				{
					saves[i].icon[j]=0;
				}
				
				for(int32_t j=0; j<48; j++)
				{
					saves[i].pal[j]=0;
				}
			}
			else
			{
				if(!iconbuffer[i].loaded || zc_get_config("zeldadx","reload_game_icons",0))
				{
					int32_t ret2 = load_quest(saves+i, false, showmetadata);
					
					if(ret2 == qe_OK)
					{
						int32_t ring=0;
						flushItemCache();
						int32_t maxringid = getHighestLevelOfFamily(saves+i, itemsbuf, itype_ring);
						
						if(maxringid != -1)
						{
							ring = itemsbuf[maxringid].fam_type;
						}
						
						if (ring > 0) --ring;
						iconbuffer[i].ring = zc_min(ring, 3);
						reload_icon_buffer(i);
						//load_game_icon_to_buffer(false,i);
						load_game_icon(saves+i, true, i);
					}
				}
			}
		}
	}
	
	pack_fclose(f);
	delete_file(tmpfilename);
	free(iname);
	exit_sys_pal();
	return 0;
	
newdata:
	if(standalone_mode)
		goto init;

	#ifdef __EMSCRIPTEN__
		goto init;
	#endif

	if(jwin_alert("Can't Find Saved Game File",
				  "The save file could not be found.",
				  "Create a new file from scratch?",
				  "Warning: Doing so will erase any previous saved games!",
				  "OK","Cancel",13,27,get_zc_font(font_lfont))!=1)
	{
		exit(1);
	}
	
	Z_message("Save file not found.  Creating new save file.");
	goto init;
	
cantopen:
	{
		char buf[256];
		snprintf(buf, 256, "still can't be opened, you'll need to delete %s.", SAVE_FILE);
		jwin_alert("Can't Open Saved Game File",
				   "The save file was found, but could not be opened. Wait a moment",
				   "and try again. If this problem persists, reboot. If the file",
				   buf,
				   "OK",NULL,'o',0,get_zc_font(font_lfont));
	}
	abort();
	
reset:
	if(jwin_alert3("Can't Open Saved Game File",
				   "Unable to read the save file.",
				   "Create a new file from scratch?",
				   "Warning: Doing so will erase any previous saved games!",
				   "No","Yes",NULL,'n','y',0,get_zc_font(font_lfont))!=2)
	{
		abort();
	}
	
	if(f)
		pack_fclose(f);
		
	delete_file(tmpfilename);
	Z_message("Format error.  Resetting game data... ");
	
init:
	for(int32_t i=0; i<MAXSAVES; i++)
		saves[i].Clear();
		
	memset(iconbuffer, 0, sizeof(savedicon)*MAXSAVES);
	
	if(standalone_mode)
		set_up_standalone_save();
		
	free(iname);
	exit_sys_pal();
	return 0;
}


int32_t writesaves(gamedata *savedata, PACKFILE *f)
{
	int32_t section_id=ID_SAVEGAME;
	int32_t section_version=V_SAVEGAME;
	int32_t section_cversion=CV_SAVEGAME;
	int32_t section_size=0;
	
	//section id
	if(!p_mputl(section_id,f))
	{
		return 1;
	}
	
	//section version info
	if(!p_iputw(section_version,f))
	{
		return 2;
	}
	
	if(!p_iputw(section_cversion,f))
	{
		return 3;
	}
	
	//section size
	if(!p_iputl(section_size,f))
	{
		return 4;
	}
	
	//word item_count=iMax;
	word qstpath_len=0;
	
	if(!p_iputw(MAXSAVES,f))
	{
		return 5;
	}
	
	for(int32_t i=0; i<MAXSAVES; i++)
	{
		qstpath_len=(word)strlen(savedata[i].qstpath);
		
		if(!pfwrite(savedata[i].get_name(),9,f))
		{
			return 6;
		}
		
		if(!p_putc(savedata[i].get_quest(),f))
		{
			return 7;
		}
		
		if(!p_iputw(savedata[i].get_deaths(),f))
		{
			return 13;
		}
		
		if(!p_putc(savedata[i]._cheat,f))
		{
			return 17;
		}
		
		for(int32_t j=0; j<MAXITEMS; j++)
		{
			if(!p_putc(savedata[i].get_item(j) ? 1 : 0,f))
				return 18;
		}
		
		if(!pfwrite(savedata[i].version,16,f))
		{
			return 20;
		}
		
		if(!pfwrite(savedata[i].title,sizeof(savedata[i].title),f))
		{
			return 21;
		}
		
		if(!p_putc(savedata[i].get_hasplayed(),f))
		{
			return 22;
		}
		
		if(!p_iputl(savedata[i].get_time(),f))
		{
			return 23;
		}
		
		if(!p_putc(savedata[i].get_timevalid(),f))
		{
			return 24;
		}
		
		if(!pfwrite(savedata[i].lvlitems,MAXLEVELS,f))
		{
			return 25;
		}
		
		if(!p_putc(savedata[i].get_continue_scrn(),f))
		{
			return 27;
		}
		
		if(!p_iputw(savedata[i].get_continue_dmap(),f))
		{
			return 28;
		}
		
		if(!pfwrite(savedata[i].visited,MAXDMAPS,f))
		{
			return 34;
		}
		
		if(!pfwrite(savedata[i].bmaps,MAXDMAPS*128,f))
		{
			return 35;
		}
		
		for(int32_t j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
		{
			if(!p_iputw(savedata[i].maps[j],f))
			{
				return 36;
			}
		}
		
		if(!pfwrite(savedata[i].guys,MAXMAPS2*MAPSCRSNORMAL,f))
		{
			return 37;
		}
		
		if(!p_iputw(qstpath_len,f))
		{
			return 38;
		}
		
		if(!pfwrite(savedata[i].qstpath,qstpath_len,f))
		{
			return 39;
		}
		
		if(!pfwrite(savedata[i].icon,sizeof(savedata[i].icon),f))
		{
			return 40;
		}
		
		if(!pfwrite(savedata[i].pal,sizeof(savedata[i].pal),f))
		{
			return 41;
		}
		
		if(!pfwrite(savedata[i].lvlkeys,MAXLEVELS,f))
		{
			return 42;
		}
		
		for(int32_t j=0; j<MAX_MI; j++)
		{
			for(int32_t k=0; k<8; k++)
			{
				if(!p_iputl(savedata[i].screen_d[j][k],f))
				{
					return 43;
				}
			}
		}
		
		for(int32_t j=0; j<MAX_SCRIPT_REGISTERS; j++)
		{
			if(!p_iputl(savedata[i].global_d[j],f))
			{
				return 44;
			}
		}
		
		for(int32_t j=0; j<32; j++)
		{
			if(!p_iputw(savedata[i].get_counter(j), f))
			{
				return 45;
			}
			
			if(!p_iputw(savedata[i].get_maxcounter(j), f))
			{
				return 46;
			}
			
			if(!p_iputw(savedata[i].get_dcounter(j), f))
			{
				return 47;
			}
		}
		
		for(int32_t j=0; j<256; j++)
		{
			if(!p_iputl(savedata[i].get_generic(j), f))
			{
				return 48;
			}
		}
		
		if(!p_putc(savedata[i].awpn, f))
		{
			return 49;
		}
		
		if(!p_putc(savedata[i].bwpn, f))
		{
			return 50;
		}
		
		//First we put the size of the vector
		if(!p_iputl(savedata[i].globalRAM.size(), f))
			return 51;
			
		for(dword j = 0; j < savedata[i].globalRAM.size(); j++)
		{
			ZScriptArray& a = savedata[i].globalRAM[j];
			
			//Then we put the size of each container
			if(!p_iputl(a.Size(), f))
				return 52;
				
			//Followed by its contents
			for(dword k = 0; k < a.Size(); k++)
				if(!p_iputl(a[k], f))
					return 53;
		}
		if(!p_iputw(savedata[i].forced_awpn, f))
		{
			return 54;
		}
		
		if(!p_iputw(savedata[i].forced_bwpn, f))
		{
			return 55;
		}
	
		if(!p_iputw(savedata[i].forced_xwpn, f))
		{
			return 56;
		}
		
		if(!p_iputw(savedata[i].forced_ywpn, f))
		{
			return 57;
		}
		if(!p_putc(savedata[i].xwpn, f))
		{
			return 58;
		}
		if(!p_putc(savedata[i].ywpn, f))
		{
			return 59;
		}
		if(!pfwrite(savedata[i].lvlswitches,MAXLEVELS*sizeof(int32_t),f))
		{
			return 60;
		}
		if(!pfwrite(savedata[i].item_messages_played,MAXITEMS*sizeof(bool),f))
		{
			return 61;
		}
		if(!pfwrite(savedata[i].bottleSlots,256*sizeof(byte),f))
		{
			return 62;
		}
		if(!p_iputw(savedata[i].saved_mirror_portal.destdmap, f))
		{
			return 63;
		}
		if(!p_iputw(savedata[i].saved_mirror_portal.srcdmap, f))
		{
			return 64;
		}
		if(!p_putc(savedata[i].saved_mirror_portal.srcscr,f))
		{
			return 65;
		}
		if(!p_putc(savedata[i].saved_mirror_portal.destscr,f))
		{
			return 109;
		}
		if(!p_iputl(savedata[i].saved_mirror_portal.x, f))
		{
			return 66;
		}
		if(!p_iputl(savedata[i].saved_mirror_portal.y, f))
		{
			return 67;
		}
		if(!p_putc(savedata[i].saved_mirror_portal.sfx,f))
		{
			return 68;
		}
		if(!p_iputl(savedata[i].saved_mirror_portal.warpfx, f))
		{
			return 69;
		}
		if(!p_iputw(savedata[i].saved_mirror_portal.spr, f))
		{
			return 70;
		}
		
		if(!p_iputw(NUMSCRIPTSGENERIC,f))
		{
			new_return(71);
		}
		save_genscript(savedata[i]); //read the values into the save object
		for(size_t q=0; q<NUMSCRIPTSGENERIC; q++)
        {
			if(!p_putc(savedata[i].gen_doscript[q] ? 1 : 0,f))
				return 72;
			if(!p_iputw(savedata[i].gen_exitState[q],f))
				return 73;
			if(!p_iputw(savedata[i].gen_reloadState[q],f))
				return 74;
			for(size_t ind = 0; ind < 8; ++ind)
				if(!p_iputl(savedata[i].gen_initd[q][ind],f))
					return 75;
			int32_t sz = savedata[i].gen_dataSize[q];
			if(!p_iputl(sz,f))
				return 76;
			for(auto ind = 0; ind < sz; ++ind)
				if(!p_iputl(savedata[i].gen_data[q][ind],f))
					return 77;
        }
		
		for(int32_t j=0; j<MAXMAPS2*MAPSCRSNORMAL; j++)
		{
			if(!p_iputl(savedata[i].xstates[j],f))
			{
				return 78;
			}
		}
		
		for(auto q = 0; q < NUMSCRIPTSGENERIC; ++q)
		{
			if(!p_iputl(savedata[i].gen_eventstate[q],f))
			{
				return 79;
			}
		}
		if(!pfwrite(savedata[i].gswitch_timers,NUM_GSWITCHES*sizeof(int32_t),f))
		{
			return 80;
		}
		if (!p_iputw(savedata[i].replay_file.length(), f))
			return 81;
		if (!pfwrite((void*)savedata[i].replay_file.c_str(), savedata[i].replay_file.length(), f))
			return 82;
		uint32_t sz = savedata[i].user_objects.size();
		if(!p_iputl(sz,f))
			return 83;
		for(saved_user_object const& s_ob : savedata[i].user_objects)
		{
			if(!p_iputl(s_ob.object_index,f))
				return 84;
			//user_object
			user_object const& obj = s_ob.obj;
			if(!p_putc(obj.reserved?1:0,f))
				return 85;
			//Don't need to save owned_type,owned_i?
			uint32_t datsz = obj.data.size();
			if(!p_iputl(datsz,f))
				return 86;
			for(uint32_t q = 0; q < datsz; ++q)
				if(!p_iputl(obj.data.at(q),f))
					return 87;
			if(!p_iputl(obj.owned_vars,f))
				return 88;
			//scr_func_exec
			scr_func_exec const& exec = obj.destruct;
			if(!p_iputl(exec.pc,f))
				return 89;
			if(!p_iputl(exec.thiskey,f))
				return 90;
			if(!p_iputl((int)exec.type,f))
				return 91;
			if(!p_iputl(exec.i,f))
				return 92;
			if(!p_iputw(exec.script,f))
				return 93;
			if(!p_putwstr(exec.name,f))
				return 94;
			auto& map = s_ob.held_arrays;
			uint32_t arrcount = map.size();
			if(!p_iputl(arrcount,f))
				return 95;
			for(auto it = map.begin(); it != map.end(); ++it)
			{
				auto& pair = *it;
				if(!p_iputl(pair.first,f))
					return 96;
				auto& zsarr = pair.second;
				uint32_t arrsz = zsarr.Size();
				if(!p_iputl(arrsz,f))
					return 97;
				for(uint32_t ind = 0; ind < arrsz; ++ind)
				{
					if(!p_iputl(zsarr[ind],f))
						return 98;
				}
			}
		}
		sz = savedata[i].user_portals.size();
		if(!p_iputl(sz,f))
			return 99;
		for(savedportal const& p : savedata[i].user_portals)
		{
			if(!p_iputw(p.destdmap, f))
				return 100;
			if(!p_iputw(p.srcdmap, f))
				return 101;
			if(!p_putc(p.srcscr,f))
				return 102;
			if(!p_putc(p.destscr,f))
				return 103;
			if(!p_iputl(p.x, f))
				return 104;
			if(!p_iputl(p.y, f))
				return 105;
			if(!p_putc(p.sfx,f))
				return 106;
			if(!p_iputl(p.warpfx, f))
				return 107;
			if(!p_iputw(p.spr, f))
				return 108;
		}
	}
	
	return 0;
}

static int32_t do_save_games()
{
	if (disable_save_to_disk || saves==NULL)
		return 1;
	
	// Not sure why this happens, but apparently it does...
	for(int32_t i=0; i<MAXSAVES; i++)
	{
		for(int32_t j=0; j<48; j++)
		{
			saves[i].pal[j]&=63;
		}
	}
	
	char tmpfilename[L_tmpnam];
	temp_name(tmpfilename);
	
	PACKFILE *f = pack_fopen_password(tmpfilename, F_WRITE_PACKED, "");
	
	if(!f)
	{
		delete_file(tmpfilename);
		return 2;
	}
	
	if(writesaves(saves, f)!=0)
	{
		pack_fclose(f);
		delete_file(tmpfilename);
		return 4;
	}
	
	pack_fclose(f);
	int32_t ret = encode_file_007(tmpfilename, SAVE_FILE, 0x413F0000 + (frame&0xffff), SAVE_HEADER, ENC_METHOD_MAX-1);
	
	if(ret)
		ret += 100;
		
	delete_file(tmpfilename);
	
	FILE *f2=NULL;
	char *iname = (char *)malloc(2048);
	strcpy(iname, SAVE_FILE);
	
	for(int32_t i=0; iname[i]!='\0'; iname[i]=='.'?iname[i]='\0':i++)
	{
		/* do nothing */
	}
	
	strcat(iname,".icn");
	
	f2=fopen(iname,"wb");
	byte *di2 = (byte *)iconbuffer;
	
	for(dword i=0; (i<sizeof(savedicon)*MAXSAVES); i++)
		fputc(*(di2++),f2);
		
	fclose(f2);
	free(iname);

#ifdef __EMSCRIPTEN__
	em_sync_fs();
#endif

	return ret;
}

int32_t save_savedgames()
{
	Saving = true;
	render_zc();
	int32_t result = do_save_games();
	Saving = false;
	return result;
}

void load_game_icon(gamedata *g, bool, int32_t index)
{
	//We need an override that fixes the palette here to prevent monochrome overwriting it. -Z
	int32_t i=iconbuffer[index].ring; //
	
	byte *si = iconbuffer[index].icon[i];
	
	for(int32_t j=0; j<128; j++)
	{
		g->icon[j] = *(si++);
	}
	
	si = iconbuffer[index].pal[i];
	
	for(int32_t j=0; j<48; j++)
	{
		g->pal[j] = *(si++);
	}
}

void reload_icon_buffer(int32_t index)
{
	int32_t t=0;
	
	for(int32_t i=0; i<4; i++)
	{
		t = QMisc.icons[i];
		
		if(t<0 || t>=NEWMAXTILES)
		{
			t=0;
		}
		
		int32_t tileind = t ? t : 28;
		
		byte *si = newtilebuf[tileind].data;
		
		if(newtilebuf[tileind].format==tf8Bit)
		{
			for(int32_t j=0; j<128; j++)
			{
				iconbuffer[index].icon[i][j] =0;
			}
		}
		else
		{
			for(int32_t j=0; j<128; j++)
			{
				iconbuffer[index].icon[i][j] = *(si++);
			}
		}
		
		if(t)
		{
			si = colordata + CSET(pSprite(i+spICON1))*3;
		}
		else
		{
			if(i)
			{
				si = colordata + CSET(pSprite(i-1+spBLUE))*3;
			}
			else
			{
				si = colordata + CSET(6)*3;
			}
		}
		
		if(newtilebuf[tileind].format==tf8Bit)
		{
			for(int32_t j=0; j<48; j++)
			{
				iconbuffer[index].pal[i][j] = 0;
			}
		}
		else
		{
			for(int32_t j=0; j<48; j++)
			{
				iconbuffer[index].pal[i][j] = *(si++);
			}
		}
	}
	
	iconbuffer[index].loaded=1;
}

void load_game_icon_to_buffer(bool forceDefault, int32_t index)
{
	int32_t ring=0;
	
	if(!forceDefault)
	{
		flushItemCache();
		int32_t maxringid = getHighestLevelOfFamily(&zinit, itemsbuf, itype_ring);
		
		if(maxringid != -1)
		{
			ring = itemsbuf[maxringid].fam_type;
		}
	}
	
	//blue rings now start at level 2 for some reason, account for that -DD
	if (ring > 0) --ring;
	iconbuffer[index].ring = zc_min(ring, 3);
	
	reload_icon_buffer(index);
}

void load_game_icon_to_buffer_manual(bool forceDefault, int32_t index, int32_t ring_value)
{
	iconbuffer[index].ring = zc_min(ring_value, 3);
	
	//reload_icon_buffer(index);
}

static void select_mode()
{
	textout_ex(scrollbuf,get_zc_font(font_zfont),"REGISTER YOUR NAME",48,152,1,0);
	textout_ex(scrollbuf,get_zc_font(font_zfont),"COPY FILE",48,168,1,0);
	textout_ex(scrollbuf,get_zc_font(font_zfont),"DELETE FILE",48,184,1,0);
}

static void register_mode()
{
	textout_ex(scrollbuf,get_zc_font(font_zfont),"REGISTER YOUR NAME",48,152,CSET(2)+3,0);
}

static void copy_mode()
{
	textout_ex(scrollbuf,get_zc_font(font_zfont),"COPY FILE",48,168,CSET(2)+3,0);
}

static void delete_mode()
{
	textout_ex(scrollbuf,get_zc_font(font_zfont),"DELETE FILE",48,184,CSET(2)+3,0);
}

static void framerect(BITMAP* dest, int32_t x, int32_t y, int32_t w, int32_t h, int32_t c)
{
	BITMAP* temp = create_bitmap_ex(8, dest->w, dest->h);
	clear_bitmap(temp);
	rect(temp, x, y, x+w-1, y+h-1,c);
	rect(temp, x+1, y+1, x+w-2, y+h-2,c);
	temp->line[y][x] = 0;
	temp->line[y][x+w-1] = 0;
	temp->line[y+h-1][x] = 0;
	temp->line[y+h-1][x+w-1] = 0;
	masked_blit(temp, dest, x, y, x, y, w, h);
	destroy_bitmap(temp);
}

static void selectscreen()
{
	FFCore.kb_typing_mode = false;
	if (get_bit(quest_rules, qr_OLD_SCRIPT_VOLUME))
	{
		if (FFCore.coreflags & FFCORE_SCRIPTED_MIDI_VOLUME)
		{
			Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
			midi_volume = FFCore.usr_midi_volume;
			//master_volume(-1,FFCore.usr_midi_volume);
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_DIGI_VOLUME)
		{
			digi_volume = FFCore.usr_digi_volume;
			//master_volume((int32_t)(FFCore.usr_digi_volume),1);
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_MUSIC_VOLUME)
		{
			emusic_volume = (int32_t)FFCore.usr_music_volume;
		}
		if (FFCore.coreflags & FFCORE_SCRIPTED_SFX_VOLUME)
		{
			sfx_volume = (int32_t)FFCore.usr_sfx_volume;
		}
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
		pan_style = (int32_t)FFCore.usr_panstyle;
	}
	FFCore.skip_ending_credits = 0;
	//  text_mode(0);
	init_NES_mode();
	loadfullpal();
	loadlvlpal(1);
	Bwpn = 0, Awpn = 0; //the subsreen values
	clear_bitmap(scrollbuf);
	
	framerect(scrollbuf, 27, 51, 26*8-6, 20*8-6,0x03);
	
	textout_ex(scrollbuf,get_zc_font(font_zfont),"- S E L E C T -",64,24,1,0); //could be in module at some point
	textout_ex(scrollbuf,get_zc_font(font_zfont)," NAME ",80,48,1,0);
	textout_ex(scrollbuf,get_zc_font(font_zfont)," LIFE ",152,48,1,0);
	select_mode();
	RAMpal[CSET(9)+1]=NESpal(0x15);
	RAMpal[CSET(9)+2]=NESpal(0x27);
	RAMpal[CSET(9)+3]=NESpal(0x30);
	RAMpal[CSET(13)+1]=NESpal(0x30);
}

static byte left_arrow_str[] = {132,0};
static byte right_arrow_str[] = {133,0};

static int32_t savecnt;

static void list_save(int32_t save_num, int32_t ypos)
{
	bool r = refreshpal;
	
	if(save_num<savecnt)
	{
		game->set_maxlife(saves[save_num].get_maxlife());
		game->set_life(saves[save_num].get_maxlife());
		game->set_hp_per_heart(saves[save_num].get_hp_per_heart());
		
		//wpnsbuf[iwQuarterHearts].tile = moduledata.select_screen_tiles[sels_heart_tile];
		//Setting the cset does nothing, because it lifemeter() uses overtile8()
		//Modules should set the cset manually. 
		//wpnsbuf[iwQuarterHearts].csets = moduledata.select_screen_tile_csets[sels_heart_tilettile_cset];
		
		//boogie!
		lifemeter(framebuf,144,ypos+((game->get_maxlife()>16*(saves[save_num].get_hp_per_heart()))?8:0),8,0);
		textout_ex(framebuf,get_zc_font(font_zfont),saves[save_num].get_name(),72,ypos+16,1,0);
		
		if(saves[save_num].get_quest())
			textprintf_ex(framebuf,get_zc_font(font_zfont),72,ypos+24,1,0,"%5d",saves[save_num].get_deaths());
			
		if ( moduledata.select_screen_tiles[draw_hero_first]) 
			overtile16(framebuf,moduledata.select_screen_tiles[sels_herotile],48,ypos+17,((unsigned)moduledata.select_screen_tile_csets[sels_hero_cset] < 15 ) ? moduledata.select_screen_tile_csets[sels_hero_cset] : (save_num%3)+10,0); 

		if(saves[save_num].get_quest()==1)
		{
			//hardcoded quest icons -- move to module
			//overtile16(framebuf,41,56,ypos+14,9,0);             //put sword on second quests
			if ( moduledata.select_screen_tiles[sels_tile_questicon_1A] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_1A],moduledata.select_screen_tiles[sels_tile_questicon_1A_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_1A_cset],0);   
			if ( moduledata.select_screen_tiles[sels_tile_questicon_1B] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_1B],moduledata.select_screen_tiles[sels_tile_questicon_1B_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_1B_cset],0);   
		}
		if(saves[save_num].get_quest()==2)
		{
		   //hardcoded quest icons -- move to module
			//overtile16(framebuf,41,56,ypos+14,9,0);             //put sword on second quests
			if ( moduledata.select_screen_tiles[sels_tile_questicon_2A] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_2A],moduledata.select_screen_tiles[sels_tile_questicon_2A_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_2A_cset],0);   
			if ( moduledata.select_screen_tiles[sels_tile_questicon_2B] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_2B],moduledata.select_screen_tiles[sels_tile_questicon_2B_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_2B_cset],0);   
		}
			
			
		if(saves[save_num].get_quest()==3)
		{
			//overtile16(framebuf,41,56,ypos+14,9,0);             //put sword on second quests
			//overtile16(framebuf,41,41,ypos+14,9,0);             //put sword on third quests
		
			if ( moduledata.select_screen_tiles[sels_tile_questicon_3A] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_3A],moduledata.select_screen_tiles[sels_tile_questicon_3A_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_3A_cset],0);   
			if ( moduledata.select_screen_tiles[sels_tile_questicon_3B] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_3B],moduledata.select_screen_tiles[sels_tile_questicon_3B_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_3B_cset],0);   
		}
		
		if(saves[save_num].get_quest()==4)
		{
			//overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
			//overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
			if ( moduledata.select_screen_tiles[sels_tile_questicon_4A] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_4A],moduledata.select_screen_tiles[sels_tile_questicon_4A_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_4A_cset],0);   
			if ( moduledata.select_screen_tiles[sels_tile_questicon_4B] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_4B],moduledata.select_screen_tiles[sels_tile_questicon_4B_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_4B_cset],0);   
		}

		if(saves[save_num].get_quest()==5)
		{
			//overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
			//overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
			if ( moduledata.select_screen_tiles[sels_tile_questicon_5A] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_5A],moduledata.select_screen_tiles[sels_tile_questicon_5A_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_5A_cset],0);   
			if ( moduledata.select_screen_tiles[sels_tile_questicon_5B] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_5B],moduledata.select_screen_tiles[sels_tile_questicon_5B_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_5B_cset],0);   
		}
		
		if(saves[save_num].get_quest()==6)
		{
			//overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
			//overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
			if ( moduledata.select_screen_tiles[sels_tile_questicon_6A] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_6A],moduledata.select_screen_tiles[sels_tile_questicon_6A_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_6A_cset],0);   
			if ( moduledata.select_screen_tiles[sels_tile_questicon_6B] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_6B],moduledata.select_screen_tiles[sels_tile_questicon_6B_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_6B_cset],0);   
		}

		if(saves[save_num].get_quest()==7)
		{
			//overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
			//overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
			if ( moduledata.select_screen_tiles[sels_tile_questicon_7A] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_7A],moduledata.select_screen_tiles[sels_tile_questicon_7A_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_7A_cset],0);   
			if ( moduledata.select_screen_tiles[sels_tile_questicon_7B] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_7B],moduledata.select_screen_tiles[sels_tile_questicon_7B_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_7B_cset],0);   
		}
		if(saves[save_num].get_quest()==8)
		{
			//overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
			//overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
			if ( moduledata.select_screen_tiles[sels_tile_questicon_8A] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_8A],moduledata.select_screen_tiles[sels_tile_questicon_8A_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_8A_cset],0);   
			if ( moduledata.select_screen_tiles[sels_tile_questicon_8B] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_8B],moduledata.select_screen_tiles[sels_tile_questicon_8B_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_8B_cset],0);   
		}
		
		if(saves[save_num].get_quest()==9)
		{
			//overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
			//overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
			if ( moduledata.select_screen_tiles[sels_tile_questicon_9A] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_9A],moduledata.select_screen_tiles[sels_tile_questicon_9A_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_9A_cset],0);   
			if ( moduledata.select_screen_tiles[sels_tile_questicon_9B] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_9B],moduledata.select_screen_tiles[sels_tile_questicon_9B_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_9B_cset],0);   
		}
		if(saves[save_num].get_quest()==10)
		{
			//overtile16(framebuf,176,52,ypos+14,0,1);             //dust pile
			//overtile16(framebuf,175,52,ypos+14,9,0);             //triforce
			if ( moduledata.select_screen_tiles[sels_tile_questicon_10A] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_10A],moduledata.select_screen_tiles[sels_tile_questicon_10A_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_10A_cset],0);   
			if ( moduledata.select_screen_tiles[sels_tile_questicon_10B] > 0 )
			overtile16(framebuf,moduledata.select_screen_tiles[sels_tile_questicon_10B],moduledata.select_screen_tiles[sels_tile_questicon_10B_X],ypos+14,moduledata.select_screen_tile_csets[sels_tile_questicon_10B_cset],0);   
		}

		textprintf_ex(framebuf,get_zc_font(font_zfont),72,ypos+16,1,0,"%s",saves[save_num].get_name());
	}
	
	byte *hold = newtilebuf[0].data;
	byte holdformat=newtilebuf[0].format;
	newtilebuf[0].format=tf4Bit;
	newtilebuf[0].data = saves[save_num].icon;
	overtile16(framebuf,(moduledata.select_screen_tiles[sels_herotile] > 1 && saves[save_num].get_quest() > 0 && saves[save_num].get_quest() < 255 ) ? moduledata.select_screen_tiles[sels_herotile] : 0,48,ypos+17,
	((unsigned)moduledata.select_screen_tile_csets[sels_hero_cset] < 15 && saves[save_num].get_quest() > 0 && saves[save_num].get_quest() < 255 ) ? (unsigned)moduledata.select_screen_tile_csets[sels_hero_cset] < 15 :
	(save_num%3)+10,0);               //hero
	newtilebuf[0].format=holdformat;
	newtilebuf[0].data = hold;
	
	hold = colordata;
	colordata = saves[save_num].pal;
	//if ( moduledata.select_screen_tile_csets[change_cset_on_quest_3] ) loadpalset((save_num%3)+10,0); //quest number changes the palette -- move to module
	loadpalset((save_num%3)+10,0); //quest number changes the palette -- move to module?
	colordata = hold;
	
	textout_ex(framebuf,get_zc_font(font_zfont),"-",136,ypos+16,1,0);
	
	refreshpal = r;
}

static void list_saves()
{
	// Fourth Quest turns the menu red.
	bool red = false;
	
	for(int32_t i=0; i<savecnt; i++)
		if(saves[i].get_quest()==4)
			red = true;
			
	loadpalset(0,red ? pSprite(spPILE) : 0);
	
	for(int32_t i=0; i<3; i++)
	{
		list_save(listpos+i,i*24+56);
	}
	
	// Draw the arrows above the lifemeter!
	if(savecnt>3)
	{
		if(listpos>=3)
			textout_ex(framebuf,get_zc_font(font_zfont),(char *)left_arrow_str,96,60,3,0);
			
		if(listpos+3<savecnt)
			textout_ex(framebuf,get_zc_font(font_zfont),(char *)right_arrow_str,176,60,3,0);
			
		textprintf_ex(framebuf,get_zc_font(font_zfont),112,60,3,0,"%2d - %-2d",listpos+1,listpos+3);
	}
	
	
}

static void draw_cursor(int32_t pos,int32_t mode)
{
	int32_t cs = 0;
	//al_trace( "moduledata.select_screen_tile_csets[sels_cusror_cset] is: %d\n", moduledata.select_screen_tile_csets[sels_cusror_cset]);
	if ( (unsigned)moduledata.select_screen_tile_csets[sels_cusror_cset] < 15 ) cs = moduledata.select_screen_tile_csets[sels_cusror_cset];
	else cs = (mode==3)?13:9;
	//al_trace("select screen cursor cset is: %d\n", cs);
	
	if(pos<3)
		overtile8(framebuf,moduledata.select_screen_tiles[sels_cursor_tile],40,pos*24+77,cs,0);
	else
		overtile8(framebuf,moduledata.select_screen_tiles[sels_cursor_tile],40,(pos-3)*16+153,cs,0);
}

static bool register_name()
{
	if(savecnt>=MAXSAVES)
		return false;
		
	if ( moduledata.refresh_title_screen ) //refresh
	{
		selectscreen();
		moduledata.refresh_title_screen = 0;
	}
	int32_t NameEntryMode2=NameEntryMode;
	
	saves[savecnt].set_maxlife(3*16);
	saves[savecnt].set_life(3*16);
	saves[savecnt].set_maxbombs(8);
	saves[savecnt].set_continue_dmap(0);
	saves[savecnt].set_continue_scrn(0xFF);
	
	int32_t s=savecnt;
	++savecnt;
	listpos=(s/3)*3;
//  clear_bitmap(framebuf);
	rectfill(framebuf,32,56,223,151,0);
	list_saves();
	blit(framebuf,scrollbuf,0,0,0,0,256,224);
	
	int32_t pos=s%3;
	int32_t y=((NameEntryMode2>0)?0:(pos*24))+72;
	int32_t x=0;
	int32_t spos=0;
	char name[9];
	
	memset(name,0,9);
	register_mode();
	clear_keybuf();
	SystemKeys=(NameEntryMode2>0);
	refreshpal=true;
	bool done=false;
	bool cancel=false;

	if (!load_qstpath.empty()) {
		std::string filename = get_filename(load_qstpath.c_str());
		filename.erase(remove(filename.begin(), filename.end(), ' '), filename.end());
		auto len = filename.find(".qst", 0);
		len = zc_min(len, 8);
		strcpy(name, filename.substr(0, len).c_str());
		x = strlen(name);
	}
	
	int32_t letter_grid_x=(NameEntryMode2==2)?34:44;
	int32_t letter_grid_y=120;
	int32_t letter_grid_offset=(NameEntryMode2==2)?10:8;
	int32_t letter_grid_width=(NameEntryMode2==2)?16:11;
	int32_t letter_grid_height=(NameEntryMode2==2)?6:4;
	int32_t letter_grid_spacing=(NameEntryMode2==2)?12:16;
	
	const char *simple_grid="ABCDEFGHIJKLMNOPQRSTUVWXYZ-.,!'&.0123456789 ";
	const char *complete_grid=" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ";
	
	if(NameEntryMode2>0)
	{
		//int32_t pos=file%3;
		BITMAP *info = create_bitmap_ex(8,168,32);
		clear_bitmap(info);
		blit(framebuf,info,40,pos*24+70,0,0,168,26);
		rectfill(info,40,0,168,1,0);
		rectfill(info,0,24,39,25,0);
		rectfill(info,0,0,7,15,0);
		rectfill(framebuf,40,64,216,192,0);
		rectfill(framebuf,96,60,183,67,0);
		
		int32_t i=pos*24+70;
		
		do
		{
			blit(info,framebuf,0,0,40,i,168,32);
			advanceframe(true);
			i-=pos+pos;
		}
		while(pos && i>=70);
		
		clear_bitmap(framebuf);
		framerect(framebuf, 27, 51, 26*8-6, 8*8-6,0x03);
		textout_ex(framebuf,get_zc_font(font_zfont)," NAME ",80,48,1,0);
		textout_ex(framebuf,get_zc_font(font_zfont)," LIFE ",152,48,1,0);
		
		blit(info,framebuf,0,0,40,70,168,32);
		destroy_bitmap(info);
		
		framerect(framebuf, letter_grid_x-letter_grid_offset+3, letter_grid_y-letter_grid_offset+3,
			((NameEntryMode2==2)?26:23)*8-6, ((NameEntryMode2==2)?11:9)*8-6,0x03);
		
		if(NameEntryMode2==1)
		{
			textout_ex(framebuf,get_zc_font(font_zfont),"A B C D E F G H I J K",letter_grid_x,letter_grid_y,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"L M N O P Q R S T U V",letter_grid_x,letter_grid_y+16,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"W X Y Z - . , ! ' & .",letter_grid_x,letter_grid_y+32,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"0 1 2 3 4 5 6 7 8 9  ",letter_grid_x,letter_grid_y+48,1,-1);
		}
		else
		{
			textout_ex(framebuf,get_zc_font(font_zfont),"   \"  $  &  (  *  ,  .",letter_grid_x,   letter_grid_y,   1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "!  #  %  '  )  +  -  /",letter_grid_x+12,letter_grid_y,   1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"0  2  4  6  8  :  <  >", letter_grid_x,   letter_grid_y+12,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "1  3  5  7  9  ;  =  ?",letter_grid_x+12,letter_grid_y+12,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"@  B  D  F  H  J  L  N", letter_grid_x,   letter_grid_y+24,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "A  C  E  G  I  K  M  O",letter_grid_x+12,letter_grid_y+24,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"P  R  T  V  X  Z  \\  ^",letter_grid_x,   letter_grid_y+36,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "Q  S  U  W  Y  [  ]  _",letter_grid_x+12,letter_grid_y+36,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"`  b  d  f  h  j  l  n", letter_grid_x,   letter_grid_y+48,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "a  c  e  g  i  k  m  o",letter_grid_x+12,letter_grid_y+48,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont),"p  r  t  v  x  z  |  ~", letter_grid_x,   letter_grid_y+60,1,-1);
			textout_ex(framebuf,get_zc_font(font_zfont), "q  s  u  w  y  {  }",   letter_grid_x+12,letter_grid_y+60,1,-1);
		}
		
		advanceframe(true);
		blit(framebuf,scrollbuf,0,0,0,0,256,224);
		
	}
	
	int32_t grid_x=0;
	int32_t grid_y=0;
	
	
	do
	{
		if(NameEntryMode2>0)
		{
			spos = grid_y*letter_grid_width+grid_x;
			load_control_state();
			
			if(rLeft())
			{
				--grid_x;
				
				if(grid_x<0)
				{
					grid_x=letter_grid_width-1;
					--grid_y;
					
					if(grid_y<0)
					{
						grid_y=letter_grid_height-1;
					}
				}
				
				sfx(WAV_CHIME);
			}
			else if(rRight())
			{
				++grid_x;
				
				if(grid_x>=letter_grid_width)
				{
					grid_x=0;
					++grid_y;
					
					if(grid_y>=letter_grid_height)
					{
						grid_y=0;
					}
				}
				
				sfx(WAV_CHIME);
			}
			else if(rUp())
			{
				--grid_y;
				
				if(grid_y<0)
				{
					grid_y=letter_grid_height-1;
				}
				
				sfx(WAV_CHIME);
			}
			else if(rDown())
			{
				++grid_y;
				
				if(grid_y>=letter_grid_height)
				{
					grid_y=0;
				}
				
				sfx(WAV_CHIME);
			}
			else if(rBbtn())
			{
				if(x<8 && name[zc_min(x,7)])
				{
					++x;
					sfx(WAV_CHIME);
				}
				
				if(x>=8)
				{
					x=0;
				}
			}
			else if(rAbtn())
			{
				name[zc_min(x,7)]=(NameEntryMode2==2)?complete_grid[spos]:simple_grid[spos];
				++x;
				
				if(x>=8)
				{
					x=0;
				}
				
				sfx(WAV_PLACE);
			}
			else if(rSbtn())
			{
				done=true;
				int32_t ltrs=0;
				
				for(int32_t i=0; i<8; i++)
				{
					if(name[i]!=' ' && name[i]!=0)
					{
						++ltrs;
					}
				}
				
				if(!ltrs)
				{
					cancel=true;
				}
			}
			
		}
		else
		{
#ifdef __EMSCRIPTEN__
			// Allow gamepad to submit name.
			poll_joystick();
			load_control_state();
			if(rSbtn())
			{
				done = true;
				break;
			}
#endif

			if(keypressed())
			{
				int32_t k=readkey();
				
				if(isprint(k&255))
				{
					name[zc_min(x,7)]=k&0xFF;
					
					if(x<8)
					{
						++x;
					}
					
					sfx(WAV_PLACE);
				}
				else
				{
					switch(k>>8)
					{
					case KEY_LEFT:
						if(x>0)
						{
							if(x==8)
							{
								x=6;
							}
							else
							{
								--x;
							}
							
							sfx(WAV_CHIME);
						}
						
						break;
						
					case KEY_RIGHT:
						if(x<8 && name[zc_min(x,7)])
						{
							++x;
							sfx(WAV_CHIME);
						}
						
						break;
						
					case KEY_ENTER:
					case KEY_ENTER_PAD:
					{
						done=true;
						int32_t ltrs=0;
						
						for(int32_t i=0; i<8; i++)
						{
							if(name[i]!=' ' && name[i]!=0)
							{
								++ltrs;
							}
						}
						
						if(!ltrs)
						{
							cancel=true;
						}
					}
					break;
					
					case KEY_BACKSPACE:
						if(x>0)
						{
							--x;
							
							for(int32_t i=zc_min(x,7); i<8; i++)
							{
								name[i]=name[i+1];
							}
							
							sfx(WAV_OUCH);
						}
						
						break;
						
					case KEY_DEL:
						for(int32_t i=zc_min(x,7); i<8; i++)
						{
							name[i]=name[i+1];
						}
						
						sfx(WAV_OUCH);
						break;
						
					case KEY_ESC:
						x=-1;
						done=true;
						
						while(key[KEY_ESC])
						{
							poll_keyboard();
							/* do nothing */
							rest(1);
						}
						
						break;
					}
				}
			}
		}
		
		saves[s].set_name(name);
		blit(scrollbuf,framebuf,0,0,0,0,256,224);
//    list_saves();
		list_save(s,56+((NameEntryMode2>0)?0:(pos*24)));
		
		int32_t x2=letter_grid_x + grid_x*letter_grid_spacing;
		int32_t y2=letter_grid_y + grid_y*letter_grid_spacing;
		
		if(frame&8)
		{
			int32_t tx=(zc_min(x,7)<<3)+72;
			
			for(int32_t dy=0; dy<8; dy++)
			{
				for(int32_t dx=0; dx<8; dx++)
				{
					if(framebuf->line[y+dy][tx+dx]==0)
					{
						framebuf->line[y+dy][tx+dx]=CSET(9)+1;
					}
					
					if(NameEntryMode2>0)
					{
						if(framebuf->line[y2+dy][x2+dx]==0)
						{
							framebuf->line[y2+dy][x2+dx]=CSET(9)+1;
						}
					}
				}
			}
		}
		
		draw_cursor((NameEntryMode2>0)?0:pos,0);
		advanceframe(true);
		/*
		  if(rBbtn())
		  {
		  x=-1;
		  done=true;
		  }
		  */
	}
	while(!done && !Quit);
	
	if(x<0 || cancel)
	{
		done=false;
	}
	
	if(done)
	{
		int32_t quest=1;
		char buf[9];
		strcpy(buf,name);
		strupr(buf);
		
		for ( byte q = 1; q < moduledata.max_quest_files; q++)
		{
			if(!stricmp(buf,moduledata.skipnames[q]))
			{
				zprint2("Quest %d '%s'\n",q+1,moduledata.skipnames[q]);
				quest=q+1;
				break;
			}
		}
		/*
		if(!stricmp(buf,moduledata.skipnames[1]))
			quest=2;
			
		if(!stricmp(buf,moduledata.skipnames[2]))
			quest=3;
			
		if(!stricmp(buf,moduledata.skipnames[3]))
			quest=4;
		   
		if(!stricmp(buf,moduledata.skipnames[4])) // This is what BigJoe wanted. I have no problem with it.
			quest=5;
		*/
		saves[s].set_quest(quest);
		game->qstpath[0] = 0;
		byte qst_num = byte(quest-1);
		
		int32_t ret = (qst_num < moduledata.max_quest_files) ? load_quest(saves+s) : qe_no_qst;
		
		if(ret==qe_OK)
		{
			flushItemCache();
			//messy hack to get this to work, since game is not yet initialized -DD
			gamedata *oldgame = game;
			game = saves+s;
			saves[s].set_maxlife(zinit.hc*zinit.hp_per_heart);
			saves[s].set_life(zinit.hc*zinit.hp_per_heart);
			saves[s].set_hp_per_heart(zinit.hp_per_heart);
			//saves[s].items[itype_ring]=0;
			removeItemsOfFamily(&saves[s], itemsbuf, itype_ring);
			int32_t maxringid = getHighestLevelOfFamily(&zinit, itemsbuf, itype_ring);
			
			if(maxringid != -1)
				getitem(maxringid, true);
				
			//      game->set_maxbombs(&saves[s], zinit.max_bombs);
			selectscreen();                                       // refresh palette
			game = oldgame;
			ringcolor(false);
			load_game_icon_to_buffer(false,s);
			load_game_icon(saves+s, true, s);
		}
		else
		{
			ringcolor(true);
			load_game_icon(saves+s, true, s);
		}
		
		//setPackfilePassword(NULL);
		saves[s].set_timevalid(1);
	}
	
	if(x<0 || cancel)
	{
		for(int32_t i=s; i<MAXSAVES-1; i++)
			saves[i]=saves[i+1];
			
		saves[MAXSAVES-1].Clear();
		--savecnt;
		
		if(listpos>savecnt-1)
			listpos=zc_max(listpos-3,0);
	}
	
	SystemKeys=true;
	selectscreen();
	list_saves();
	select_mode();
	return done;
}

static bool copy_file(int32_t file)
{
	if(savecnt<MAXSAVES && file<savecnt)
	{
		saves[savecnt]=saves[file];
		if (!saves[savecnt].replay_file.empty())
		{
			if (std::filesystem::exists(saves[file].replay_file))
			{
				std::string new_replay_path = create_replay_path_for_save(&saves[savecnt]);
				saves[savecnt].replay_file = new_replay_path;
				std::filesystem::copy(saves[file].replay_file, new_replay_path);
			}
			else
			{
				Z_error("Error copying replay file - %s not found", saves[file].replay_file.c_str());
				saves[savecnt].replay_file = "";
			}
		}
		iconbuffer[savecnt]=iconbuffer[file];
		++savecnt;
		listpos=((savecnt-1)/3)*3;
		sfx(WAV_SCALE);
		select_mode();
		return true;
	}
	
	return false;
}

static bool delete_save(int32_t file)
{
	if(file<savecnt)
	{
		for(int32_t i=file; i<MAXSAVES-1; i++)
		{
			saves[i]=saves[i+1];
			iconbuffer[i]=iconbuffer[i+1];
		}
		
		saves[MAXSAVES-1].Clear();
		--savecnt;
		
		if(listpos>savecnt-1)
			listpos=zc_max(listpos-3,0);
			
		sfx(WAV_OUCH);
		select_mode();
		return true;
	}
	
	return false;
}

/** game mode stuff **/

DIALOG gamemode_dlg[] =
{
	// (dialog proc)      (x)   (y)   (w)   (h)   (fg)     (bg)     (key)    (flags)     (d1)           (d2)     (dp)                            (dp2)  (dp3)
	{ jwin_win_proc,       40,   44,  240,  180,   0,       0,       0,       D_EXIT,     0,             0, (void *) "Select Custom Quest", NULL,  NULL },
	// 1
	{ jwin_button_proc,   205,   76,   61,   21,   0,       0,       'b',     D_EXIT,     0,             0, (void *) "&Browse",             NULL,  NULL },
	{ jwin_textbox_proc,   55,   78,  140,   16,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	{ jwin_text_proc,      76,  106,   64,    8,   vc(0),   vc(11),  0,       0,          0,             0, (void *) "Info:",               NULL,  NULL },
	{ jwin_textbox_proc,   76,  118,  168,   60,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL },
	// 5
	{ jwin_button_proc,    90,  191,   61,   21,   0,       0,       'k',     D_EXIT,     0,             0, (void *) "O&K",                 NULL,  NULL },
	{ jwin_button_proc,   170,  191,   61,   21,   0,       0,       27,      D_EXIT,     0,             0, (void *) "Cancel",              NULL,  NULL },
	{ d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
	{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};


static int32_t get_quest_info(zquestheader *header,char *str)
{
	if(strlen(get_filename(qstpath)) == 0)
	{
		str[0]=0;
		return 0;
	}

	int32_t error;
	PACKFILE* f = open_quest_file(&error, qstpath, false);

	if (!f)
	{
		strcpy(str,"Error: Unable to open file");
		return 0;
	}

	int32_t ret = readheader(f, header, true);
	pack_fclose(f);
	clear_quest_tmpfile();

	switch(ret)
	{
		case 0:
			break;
			
		case qe_invalid:
			strcpy(str,"Error: Invalid quest file");
			return 0;
			
		case qe_version:
			strcpy(str,"Error: Invalid version");
			return 0;
			
		case qe_obsolete:
			strcpy(str,"Error: Obsolete version");
			return 0;
			
		case qe_silenterr:
			return 0;
	}
	
	strcpy(str,"Title:\n");
	strcat(str,header->title);
	strcat(str,"\n\nAuthor:\n");
	strcat(str,header->author);

	return 1;
}

bool load_custom_game(int32_t file)
{
	if(!saves[file].get_hasplayed())
	{
		if(chosecustomquest)
		{
			clear_to_color(screen,BLACK);
			saves[file].set_quest(0xFF);
			char temppath[2048];
			memset(temppath, 0, 2048);
			zc_make_relative_filename(temppath, qstdir, qstpath, 2047);
			
			if(temppath[0]==0)  //can't make relative, go absolute
			{
				sprintf(saves[file].qstpath, "%s", qstpath);
			}
			else
			{
				sprintf(saves[file].qstpath, "%s", temppath);
			}
			
			load_quest(saves+file);
			
			saves[file].set_maxlife(zinit.hc*zinit.hp_per_heart);
			saves[file].set_life(zinit.hc*zinit.hp_per_heart);
			saves[file].set_hp_per_heart(zinit.hp_per_heart);
			flushItemCache();
			
			//messy hack to get this to work properly since game is not initialized -DD
			gamedata *oldgame = game;
			game = saves+file;
			int32_t maxringid = getHighestLevelOfFamily(&zinit, itemsbuf, itype_ring);
			
			if(maxringid != -1)
				getitem(maxringid, true);
				
			rest(200); // Formerly 1000 -L
			ringcolor(false);
			load_game_icon_to_buffer(false,file);
			load_game_icon(game,false,file);
			game = oldgame;
			chosecustomquest = false;
			return true;
		}
	}
	
	return false;
}

int32_t custom_game(int32_t file)
{
	zquestheader h;
	char infostr[200];
	char path[2048];
	int32_t ret=0; 
	int32_t focus_obj = 1; //Fixes the issue where the button tied to the enter key is stuck on 'browse'.
	
	if(is_relative_filename(saves[file].qstpath))
	{
		sprintf(qstpath,"%s%s",qstdir,saves[file].qstpath);
	}
	else
	{
		sprintf(qstpath,"%s", saves[file].qstpath);
	}
	char relpath[2048];
	relativize_path(relpath, qstpath);
	
	gamemode_dlg[0].dp2 = get_zc_font(font_lfont);
	gamemode_dlg[2].dp = relpath;//get_filename(qstpath);
	
	if(get_quest_info(&h,infostr)==0)
	{
		gamemode_dlg[4].dp = infostr;
		gamemode_dlg[5].flags = D_DISABLED;
	}
	else
	{
		gamemode_dlg[4].dp = infostr;
		gamemode_dlg[5].flags = D_EXIT;
	}
	
	if(byte(saves[file].get_quest()-1) < moduledata.max_quest_files)
		strcpy(qstpath,qstdir);
	
	gamemode_dlg[2].d1 = gamemode_dlg[4].d1 = 0;
	gamemode_dlg[2].d2 = gamemode_dlg[4].d2 = 0;
	enter_sys_pal();
	
	clear_keybuf();
	
	large_dialog(gamemode_dlg);
   
	bool customized = false;
	while((ret=zc_popup_dialog(gamemode_dlg,focus_obj))==1)
	{
		blit(screen,tmp_scr,scrx,scry,0,0,320,240);
		
		int32_t  sel=0;
		static EXT_LIST list[] =
		{
			{ (char *)"ZC Quests (*.qst)", (char *)"qst" },
			{ NULL,                        NULL }
		};
		
		strcpy(path, qstpath);
		
		if(jwin_file_browse_ex("Load Quest", path, list, &sel, 2048, -1, -1, get_zc_font(font_lfont)))
		{
			customized = true;
			//      strcpy(qstpath, path);
			replace_extension(qstpath,path,"qst",2047);
			gamemode_dlg[2].dp = get_filename(qstpath);
			
			if(get_quest_info(&h,infostr)==0)
			{
				gamemode_dlg[4].dp = infostr;
				gamemode_dlg[5].flags = D_DISABLED;
				focus_obj = 1;
			}
			else
			{
				gamemode_dlg[4].dp = infostr;
				gamemode_dlg[5].flags = D_EXIT;
				focus_obj = 5;
			}
			
			gamemode_dlg[2].d1 = gamemode_dlg[4].d1 = 0;
			gamemode_dlg[2].d2 = gamemode_dlg[4].d2 = 0;
		}
		
		blit(tmp_scr,screen,0,0,scrx,scry,320,240);
	}
	if(!customized) strcpy(qstpath, relpath);
	
	exit_sys_pal();
	key[KEY_ESC]=0;
	chosecustomquest = (ret==5) && customized;
	return (int32_t)chosecustomquest;
}

static int32_t game_details(int32_t file)
{

	al_trace("Running game_details(int32_t file)\n");
	int32_t pos=file%3;
	
	if(saves[file].get_quest()==0)
		return 0;
		
	BITMAP *info = create_bitmap_ex(8,168,32);
	clear_bitmap(info);
	blit(framebuf,info,40,pos*24+70,0,0,168,26);
	rectfill(info,40,0,168,1,0);
	rectfill(info,0,24,39,25,0);
	rectfill(info,0,0,7,15,0);
	rectfill(framebuf,40,64,216,192,0);
	rectfill(framebuf,96,60,183,67,0);
	
	int32_t i=pos*24+70;
	
	do
	{
		blit(info,framebuf,0,0,40,i,168,32);
		advanceframe(true);
		i-=pos+pos;
	}
	while(pos && i>=70);
	
	destroy_bitmap(info);
	
	textout_ex(framebuf,get_zc_font(font_zfont),"GAME TYPE",40,104,3,0);
	textout_ex(framebuf,get_zc_font(font_zfont),"QUEST",40,112,3,0);
	textout_ex(framebuf,get_zc_font(font_zfont),"STATUS",40,120,3,0);
	
	if(saves[file].get_quest()<0xFF)
	{
		textout_ex(framebuf,get_zc_font(font_zfont),"Normal Game",120,104,1,0);
		textprintf_ex(framebuf,get_zc_font(font_zfont),120,112,1,0,"%s Quest",
					  ordinal(saves[file].get_quest()));
	}
	else
	{
		textout_ex(framebuf,get_zc_font(font_zfont),"Custom Quest",120,104,1,0);
		textprintf_ex(framebuf,get_zc_font(font_zfont),120,112,1,0,"%s",
					  get_filename(saves[file].qstpath));
	}
	
	if(!saves[file].get_hasplayed())
		textout_ex(framebuf,get_zc_font(font_zfont),"Empty Game",120,120,1,0);
	else if(!saves[file].get_timevalid())
		textout_ex(framebuf,get_zc_font(font_zfont),"Time Unknown",120,120,1,0);
	else
		textout_ex(framebuf,get_zc_font(font_zfont),time_str_med(saves[file].get_time()),120,120,1,0);
		
	if(saves[file].did_cheat())
		textout_ex(framebuf,get_zc_font(font_zfont),"Used Cheats",120,128,1,0);
		
	textout_ex(framebuf,get_zc_font(font_zfont),"START: PLAY GAME",56,152,1,0);
	textout_ex(framebuf,get_zc_font(font_zfont),"    B: CANCEL",56,168,1,0);
	
	if(!saves[file].get_hasplayed())
		textout_ex(framebuf,get_zc_font(font_zfont),"    A: CUSTOM QUEST",56,184,1,0);
		
	while(!Quit)
	{
		advanceframe(true);
		load_control_state();
		
		if(rBbtn())
		{
			blit(scrollbuf,framebuf,0,0,0,0,256,224);
			return 0;
		}
		
		if(rSbtn())
		{
			blit(framebuf,scrollbuf,0,0,0,0,256,224);
			return 1;
		}
		
		if(rAbtn() && !saves[file].get_hasplayed())
		{
			(void)custom_game(file);
		}
		
		if(chosecustomquest && load_custom_game(file))
		{
			selectscreen();
			return 0;
		}
	}
	
	return 0;
}

static int32_t saveslot = -1;

int32_t getsaveslot()
{
	if(saveslot >= 0 && (!saves[saveslot].get_quest() || saves[saveslot].get_hasplayed()))
	{
		return -1;
	}
	
	return saveslot;
}

static void select_game(bool skip = false)
{
	if(standalone_mode || skip)
		return;
		
	int32_t pos = zc_max(zc_min(currgame-listpos,3),0);
	int32_t mode = 0;
	
	//kill_sfx();
	
	//  text_mode(0);
	selectscreen();
	
	savecnt=0;
	
	while(savecnt < MAXSAVES && saves[savecnt].get_quest()>0)
		++savecnt;
		
	if(savecnt == 0)
		pos=3;
		
	bool done=false;
	refreshpal=true;
	bool popup_choose_quest = false;
	do
	{
		if ( moduledata.refresh_title_screen ) //refresh
		{
			selectscreen();
			moduledata.refresh_title_screen = 0;
		}
		sfxdat=1;
		blit(scrollbuf,framebuf,0,0,0,0,256,224);
		list_saves();
		draw_cursor(pos,mode);
		advanceframe(true);
		load_control_state();
		saveslot = pos + listpos;

		if(!load_qstpath.empty())
		{
			if (register_name())
			{
				currgame = savecnt - 1;
				loadlast = currgame + 1;
				strcpy(qstpath, load_qstpath.c_str());
				chosecustomquest = true;
				load_custom_game(currgame);
				save_savedgames();
				break;
			}
			else
			{
				load_qstpath = "";
			}
		}
		
		if(popup_choose_quest)
		{
			custom_game(saveslot);
			popup_choose_quest = false;
		}
		
		if(rSbtn())
			switch(pos)
			{
			case 3:
				if(!register_name())
					pos = 3;
				else
				{
					pos = (savecnt-1)%3;
					
					popup_choose_quest = true;
				}
				refreshpal=true;
				break;
				
			case 4:
				if(savecnt && savecnt<MAXSAVES)
				{
					mode=2;
					pos=0;
					copy_mode();
				}
				
				refreshpal=true;
				break;
				
			case 5:
				if(savecnt)
				{
					mode=3;
					pos=0;
					delete_mode();
				}
				
				refreshpal=true;
				break;
				
			default:
				switch(mode)
				{
				case 0:
					currgame=saveslot;
					loadlast=currgame+1;
					
					if(saves[currgame].get_quest())
						done=true;
						
					break;
					
				case 2:
					if(copy_file(saveslot))
					{
						mode=0;
						pos=(savecnt-1)%3;
						refreshpal=true;
					}
					
					break;
					
				case 3:
					if(delete_save(saveslot))
					{
						mode=0;
						pos=3;
						refreshpal=true;
					}
					
					break;
				}
			}
			
		if(rUp())
		{
			--pos;
			
			if(pos<0)
				pos=(mode)?2:5;
				
			sfx(WAV_CHIME);
		}
		
		if(rDown())
		{
			++pos;
			
			if(pos>((mode)?2:5))
				pos=0;
				
			sfx(WAV_CHIME);
		}
		
		if(rLeft() && listpos>2)
		{
			listpos-=3;
			sfx(WAV_CHIME);
			refreshpal=true;
		}
		
		if(rRight() && listpos+3<savecnt)
		{
			listpos+=3;
			sfx(WAV_CHIME);
			refreshpal=true;
		}
		
		if(rBbtn() && mode)
		{
			if(mode==2) pos=4;
			
			if(mode==3) pos=5;
			
			mode=0;
			select_mode();
		}
		
		if(rAbtn() && !mode && pos<3)
		{
			if(game_details(saveslot))
			{
				currgame=saveslot;
				loadlast=currgame+1;
				
				if(saves[currgame].get_quest())
					done=true;
			}
		}
		
		if(chosecustomquest)
		{
			load_custom_game(saveslot);
			selectscreen();
		}
	}
	while(!Quit && !done);
	
	saveslot = -1;
}

/**************************************/
/****  Main title screen routine  *****/
/**************************************/

void titlescreen(int32_t lsave)
{
	int32_t q=Quit;
	
	Quit=0;
	Playing=Paused=false;
	FFCore.kb_typing_mode = false;
/*
if ( FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME )
	{
	Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
	midi_volume = FFCore.usr_midi_volume;
//	master_volume(-1,FFCore.usr_midi_volume);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME )
	{
	digi_volume = FFCore.usr_digi_volume;
	//master_volume((int32_t)(FFCore.usr_digi_volume),1);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME )
	{
	emusic_volume = (int32_t)FFCore.usr_music_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME )
	{
	sfx_volume = (int32_t)FFCore.usr_sfx_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
	pan_style = (int32_t)FFCore.usr_panstyle;
	}	
	*/
	FFCore.skip_ending_credits = 0;
	
	if(q==qCONT)
	{
		cont_game();
		return;
	}
	
	if (replay_get_mode() == ReplayMode::Record)
	{
		replay_save();
		replay_stop();
	}
	
	if(q==qRESET && !skip_title)
	{
		show_subscreen_dmap_dots=true;
		show_subscreen_numbers=true;
		show_subscreen_items=true;
		show_subscreen_life=true;
		
		switch(title_version)
		{
		case 2:
			v25_titlescreen();
			break;
			
		case 1:
			DX_titlescreen();
			break;
			
		case 0:
		default:
			NES_titlescreen();
			break;
		}
	}
	
	if(!Quit)
	{
		if(lsave<1)
		{
			if(slot_arg)
			{
				currgame = slot_arg2-1;
				savecnt=0;
				
				while(savecnt < MAXSAVES && saves[savecnt].get_quest() > 0)
					++savecnt;
					
				if(currgame > savecnt-1)
				{
					slot_arg = 0;
					currgame = 0;
					select_game(q==qRELOAD);
				}
				
				slot_arg = 0;
				//game->get_quest(&saves[currgame]);
				//select_game();
			}
			else
			{
				select_game(q==qRELOAD);
			}
		}
		else
		{
			currgame = lsave-1;
			
			if(!saves[currgame].get_quest())
			{
				select_game();
			}
		}
	}
	
	if(!Quit)
	{
		init_game();
	}
}

void game_over(int32_t type)
{
	FFCore.kb_typing_mode = false; 
	/*
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME )
	{
	Z_scripterrlog("Trying to restore master MIDI volume to: %d\n", FFCore.usr_midi_volume);
	midi_volume = FFCore.usr_midi_volume;
//	master_volume(-1,FFCore.usr_midi_volume);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME )
	{
	digi_volume = FFCore.usr_digi_volume;
	//master_volume((int32_t)(FFCore.usr_digi_volume),1);
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME )
	{
	emusic_volume = (int32_t)FFCore.usr_music_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME )
	{
	sfx_volume = (int32_t)FFCore.usr_sfx_volume;
	}
	if ( FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE )
	{
	pan_style = (int32_t)FFCore.usr_panstyle;
	}
	*/
	FFCore.skip_ending_credits = 0;
	kill_sfx();
	music_stop();
	clear_bitmap(screen);
	clear_a5_bmp(rti_infolayer.bitmap);
	//clear_to_color(screen,SaveScreenSettings[SAVESC_BACKGROUND]);
	loadfullpal();
	
	//if(get_qr(qr_INSTANT_RESPAWN))
	//{	zprint2("Reloading/n");
	//	Quit = qRELOAD;
	//	return;
	//}
	
	if(Quit==qGAMEOVER)
		jukebox(SaveScreenSettings[SAVESC_MIDI] + (ZC_MIDI_COUNT - 1));
		
	Quit=0;
	
	clear_to_color(framebuf,SaveScreenSettings[SAVESC_BACKGROUND]);
	//  text_mode(-1);
	
	//Setting the colour via the array isn't working. Perhaps misc colours need to be assigned to the array in init.
	textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_CONTINUE],88,72,( SaveScreenSettings[SAVESC_TEXT_CONTINUE_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_CONTINUE_COLOUR] : QMisc.colors.msgtext) ,-1);
	//WTF! Setting this in zq Init() didn't work?! -Z
	if(!type)
	{
		textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVE],88,96,( SaveScreenSettings[SAVESC_TEXT_SAVE_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVE_COLOUR] : QMisc.colors.msgtext),-1);
		textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_RETRY],88,120,( SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] : QMisc.colors.msgtext),-1);
	}
	else
		textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_RETRY],88,96,( SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] : QMisc.colors.msgtext),-1);
		
	int32_t pos = 0;
	int32_t f=-1;
	//  int32_t htile = QHeader.old_dat_flags[ZQ_TILES] ? 2 : 0;
	int32_t htile = SaveScreenSettings[SAVESC_USETILE];
	int32_t curcset = SaveScreenSettings[SAVESC_CURSOR_CSET];
	bool done=false;

	if (replay_version_check(7))
	{
		zc_readrawkey(Skey, true);
	}
	else
	{
		do {
			load_control_state();
		}
		while(getInput(btnS, true, false, true));//rSbtn
	}
	
	do
	{
		load_control_state();
		
		if(f==-1)
		{
			if(getInput(btnUp, true, false, true))//rUp
			{
				sfx(SaveScreenSettings[SAVESC_CUR_SOUND]);
				pos=(pos==0)?2:pos-1;
				
				if(type)
				{
					if(pos==1) pos--;
				}
			}
			
			if(getInput(btnDown, true, false, true))//rDown
			{
				sfx(SaveScreenSettings[SAVESC_CUR_SOUND]);
				pos=(pos+1)%3;
				
				if(type)
				{
					if(pos==1) pos++;
				}
			}
			
			if(getInput(btnS, true, false, true)) ++f;//rSbtn
		}
		
		if(f>=0)
		{
			if(++f == 65)
				done=true;
				
			if(!(f&3))
			{
				bool flash = (f&4)!=0;
				
				switch(pos)
				{
				case 0:
					textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_CONTINUE],88,72,(flash ? ( SaveScreenSettings[SAVESC_TEXT_CONTINUE_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_CONTINUE_FLASH]
						: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_CONTINUE_COLOUR] > 0 ? 
							SaveScreenSettings[SAVESC_TEXT_CONTINUE_COLOUR] : QMisc.colors.msgtext)),-1);
					break;
					
				case 1:
					textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVE],88,96,(flash ? ( SaveScreenSettings[SAVESC_TEXT_SAVE_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVE_FLASH]
						: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_SAVE_COLOUR] > 0 ? 
							SaveScreenSettings[SAVESC_TEXT_SAVE_COLOUR] : QMisc.colors.msgtext)),-1);
					break;
					
				case 2:
					if(!type)
						textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_RETRY],88,120,(flash ? ( SaveScreenSettings[SAVESC_TEXT_RETRY_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_RETRY_FLASH]
							: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] > 0 ? 
								SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] : QMisc.colors.msgtext)),-1);
					else textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_RETRY],88,96,(flash ? ( SaveScreenSettings[SAVESC_TEXT_RETRY_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_RETRY_FLASH]
						: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] > 0 ? 
							SaveScreenSettings[SAVESC_TEXT_RETRY_COLOUR] : QMisc.colors.msgtext)),-1);
					
					break;
				}
			}
		}
		
		rectfill(framebuf,72,72,79,127,SaveScreenSettings[SAVESC_BACKGROUND]);
		overtile8(framebuf,htile,72,pos*(type?12:24)+72,curcset,SaveScreenSettings[SAVESC_CUR_FLIP]);
		advanceframe(true);
	}
	while(!Quit && !done);

	if (replay_is_debug())
		replay_step_comment("game_over selection made");
	
	reset_combo_animations();
	reset_combo_animations2();
	clear_bitmap(framebuf);
	advanceframe(true);
	
	if(done)
	{
		// This is always the last step before a game save replay is stopped. On replay_continue,
		// the frame_count is set to this step's frame + 1 to continue the recording–so this comment
		// is super important.
		replay_step_comment("game_over");

		if(pos)
		{
			if(standalone_mode && !skip_title)
			{
				Quit=qRESET;
			}
			else
			{
				Quit=qQUIT;
			}
		}
		else
		{
			Quit=qCONT;
		}
		
		//Quit = pos ? ((standalone_mode && skip_title) ? qRESET : qQUIT) : qCONT;
		if(pos==1&&(!type))
		{
			//run save scripts
			FFCore.runOnSaveEngine();
			setMonochrome(false); //Clear monochrome before drawing the file select.
			doClearTint();
			
			game->save_user_objects();
			saves[currgame]=*game;
			
			int32_t ring=0;
			flushItemCache();
			int32_t maxringid = getHighestLevelOfFamily(game, itemsbuf, itype_ring);
			
			if(maxringid != -1)
			{
				ring = itemsbuf[maxringid].fam_type;
			}
			
			if (ring > 0) --ring;
			iconbuffer[currgame].ring = zc_min(ring, 3);
			
			load_game_icon(saves+currgame,false,currgame);
			save_savedgames();
			if (replay_get_mode() == ReplayMode::Record) replay_save();
		}
	}
}

void save_game(bool savepoint)
{
	//run save scripts
	FFCore.runOnSaveEngine();
	if(savepoint)
	{
		game->set_continue_scrn(homescr);
		game->set_continue_dmap(currdmap);
		lastentrance_dmap = currdmap;
		lastentrance = game->get_continue_scrn();
	}
	
	game->save_user_objects();
	saves[currgame]=*game;
	
	flushItemCache();
	
	if(zqtesting_mode) return;
	
	int32_t maxringid = getHighestLevelOfFamily(game, itemsbuf, itype_ring);
	
	int32_t ring=0;
	if(maxringid != -1)
	{
		ring = itemsbuf[maxringid].fam_type;
	}
	
	if (ring > 0) --ring;
	iconbuffer[currgame].ring = zc_min(ring, 3);
	load_game_icon(saves+currgame,false,currgame);
	save_savedgames();
	if (replay_get_mode() == ReplayMode::Record) replay_save();
}

bool save_game(bool savepoint, int32_t type)
{
	kill_sfx();
	//music_stop();
	clear_bitmap(screen);
	clear_a5_bmp(rti_infolayer.bitmap);
	//clear_to_color(screen,SaveScreenSettings[SAVESC_BACKGROUND]);
	loadfullpal();
	
	//  int32_t htile = QHeader.old_dat_flags[ZQ_TILES] ? 2 : 0;
	int32_t htile = SaveScreenSettings[SAVESC_USETILE];
	int32_t curcset = SaveScreenSettings[SAVESC_CURSOR_CSET];
	bool done=false;
	bool didsaved=false;
	FFCore.kb_typing_mode = false;
	do
	{
		int32_t pos = 0;
		int32_t f=-1;
		bool done2=false;
		clear_to_color(framebuf,SaveScreenSettings[SAVESC_BACKGROUND]);
		
		//  text_mode(-1);
		if(type)
		{
		//Migrate this to use SaveScreenColours[SAVESC_TEXT] and set that to a default
		//of QMisc.colors.msgtext when loading the quest in the loadquest function
		//for quests with a version < 0x254! -Z
			textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVEQUIT],88,72,( SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_COLOUR] : QMisc.colors.msgtext),-1);
		}
		else
		{
			textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVE2],88,72,( SaveScreenSettings[SAVESC_TEXT_SAVE2_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVE2_COLOUR] : QMisc.colors.msgtext),-1);
		}
		
		textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_DONTSAVE],88,96,( SaveScreenSettings[SAVESC_TEXT_DONTSAVE_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_DONTSAVE_COLOUR] : QMisc.colors.msgtext),-1);
		textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_QUIT],88,120,( SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext),-1);
		
		rUp(); rDown(); rSbtn(); //eat inputs
		do
		{
			load_control_state();
			
			if(f==-1)
			{
				if(getInput(btnUp, true, false, true))//rUp
				{
					sfx(SaveScreenSettings[SAVESC_CUR_SOUND]);
					pos=(pos==0)?2:pos-1;
				}
				
				if(getInput(btnDown, true, false, true))//rDown
				{
					sfx(SaveScreenSettings[SAVESC_CUR_SOUND]);
					pos=(pos+1)%3;
				}
				
				if(getInput(btnS, true, false, true)) ++f;//rSbtn
			}
			
			if(f>=0)
			{
				if(++f == 65)
					done2=true;
					
				if(!(f&3))
				{
					bool flash = (f&4)!=0;
					
					switch(pos)
					{
					case 0:
						if(type)
							textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVEQUIT],88,72,(flash ? ( SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_FLASH]
								: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_COLOUR] > 0 ? 
									SaveScreenSettings[SAVESC_TEXT_SAVEQUIT_COLOUR] : QMisc.colors.msgtext)),-1);
						else textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_SAVE2],88,72,(flash ? ( SaveScreenSettings[SAVESC_TEXT_SAVE2_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_SAVE2_FLASH]
								: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_SAVE2_COLOUR] > 0 ? 
									SaveScreenSettings[SAVESC_TEXT_SAVE2_COLOUR] : QMisc.colors.msgtext)),-1);
						
						break;
						
					case 1:
						textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_DONTSAVE],88,96,(flash ? ( SaveScreenSettings[SAVESC_TEXT_DONTSAVE_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_DONTSAVE_FLASH]
							: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_DONTSAVE_COLOUR] > 0 ? 
								SaveScreenSettings[SAVESC_TEXT_DONTSAVE_COLOUR] : QMisc.colors.msgtext)),-1);
						break;
						
					case 2:
						textout_ex(framebuf,get_zc_font(font_zfont),SaveScreenText[SAVESC_QUIT],88,120,(flash ? ( SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH]
							: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? 
								SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext)),-1);
						break;
					}
				}
			}
			
			rectfill(framebuf,72,72,79,127,SaveScreenSettings[SAVESC_BACKGROUND]);
			overtile8(framebuf,htile,72,pos*24+72,curcset,SaveScreenSettings[SAVESC_CUR_FLIP]);
			advanceframe(true);
		}
		while(!Quit && !done2);
		
		//reset_combo_animations();
		clear_bitmap(framebuf);
		//advanceframe();
		
		if(done2)
		{
			//Quit = (pos==2) ? qQUIT : 0;
			if(pos==1||pos==0) done=true;
			
			if(pos==0)
			{
				//run save scripts
				FFCore.runOnSaveEngine();
				if(savepoint)
				{
					game->set_continue_scrn(homescr);
					game->set_continue_dmap(currdmap);
					lastentrance_dmap = currdmap;
					lastentrance = game->get_continue_scrn();
				}
				
				game->save_user_objects();
				saves[currgame]=*game;
				
				int32_t ring=0;
				flushItemCache();
				int32_t maxringid = getHighestLevelOfFamily(game, itemsbuf, itype_ring);
				
				if(maxringid != -1)
				{
					ring = itemsbuf[maxringid].fam_type;
				}
				
				if (ring > 0) --ring;
				iconbuffer[currgame].ring = zc_min(ring, 3);
				load_game_icon(saves+currgame,false,currgame);
				save_savedgames();
				if (replay_get_mode() == ReplayMode::Record) replay_save();
				didsaved=true;
				
				if(type)
				{
					Quit = qQUIT;
					done=true;
					skipcont=1;
				}
			}
			
			if(pos==2)
			{
				clear_to_color(framebuf,SaveScreenSettings[SAVESC_BACKGROUND]);
				//  text_mode(-1);
				textout_ex(framebuf,get_zc_font(font_zfont),"ARE YOU SURE?",88,72,( SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext),-1);
				textout_ex(framebuf,get_zc_font(font_zfont),"YES",88,96,( SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext),-1);
				textout_ex(framebuf,get_zc_font(font_zfont),"NO",88,120,( SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext),-1);
				int32_t pos2=0;
				int32_t g=-1;
				bool done3=false;
				
				rUp(); rDown(); rSbtn(); //eat inputs
				do
				{
					load_control_state();
					
					if(g==-1)
					{
						if(getInput(btnUp, true, false, true))//rUp
						{
							sfx(WAV_CHINK);
							pos2=(pos2==0)?1:pos2-1;
						}
						
						if(getInput(btnDown, true, false, true))//rDown
						{
							sfx(WAV_CHINK);
							pos2=(pos2+1)%2;
						}
						
						if(getInput(btnS, true, false, true)) ++g;//rSbtn
					}
					
					if(g>=0)
					{
						if(++g == 65)
							done3=true;
							
						if(!(g&3))
						{
							bool flash = (g&4)!=0;
							
							switch(pos2)
							{
							case 0:
								textout_ex(framebuf,get_zc_font(font_zfont),"YES",88,96,(flash ? ( SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH]
									: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? 
										SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext)),-1);
								break;
								
							case 1:
								textout_ex(framebuf,get_zc_font(font_zfont),"NO",88,120,(flash ? ( SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH] > 0 ? SaveScreenSettings[SAVESC_TEXT_QUIT_FLASH]
									: QMisc.colors.caption) : (SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] > 0 ? 
										SaveScreenSettings[SAVESC_TEXT_QUIT_COLOUR] : QMisc.colors.msgtext)),-1);
								break;
								//case 2: textout_ex(framebuf,get_zc_font(font_zfont),"QUIT",88,120,c,-1);   break;
							}
						}
					}
					
					
					rectfill(framebuf,72,72,79,127,0);
					puttile8(framebuf,htile,72,pos2*24+96,1,0);
					advanceframe(true);
				}
				while(!Quit && !done3);
				
				clear_bitmap(framebuf);
				
				if(pos2==0)
				{
					Quit = qQUIT;
					done=true;
					skipcont=1;
				}
			}
		}
	}
	while(!Quit && !done);
	
	ringcolor(false);
	loadlvlpal(DMaps[currdmap].color);
	
	if(darkroom)
	{
		if(get_qr(qr_FADE))
		{
			interpolatedfade();
		}
		else
		{
			loadfadepal((DMaps[currdmap].color)*pdLEVEL+poFADE3);
		}
	}
	
	return didsaved;
}

//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_misc.cc
//
//  Misc. stuff for ZQuest.
//
//--------------------------------------------------------

//INLINE void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }
//INLINE void SCRFIX() {}

#include "precompiled.h" //always first

#include "zq_misc.h"
#include "zquestdat.h"
#include "zquest.h"
#include "base/colors.h"
#include "qst.h"
#include "base/zsys.h"
#include "zq_class.h"
#include "dialog/info.h"
#include "dialog/about.h"
#include "jwin_a5.h"
#include <string.h>
#include <stdio.h>
#include <sstream>

#include "metadata/metadata.h"

#ifdef _MSC_VER
#define strupr _strupr
#define stricmp _stricmp
#endif

extern int32_t prv_mode;
extern void dopreview();
extern int32_t jwin_pal[jcMAX];


const char *imgstr[ftMAX] =
{
    "Not loaded", "Binary/ROM", "Image", "ZC Tiles",
    "ZC Tiles", "ZC Tiles", "ZC Tiles", "ZC Tiles"
};

int32_t filetype(const char *path)
{
    if(path==NULL || strlen(get_filename(path))==0)
        return 0;

    char ext[40];
    strcpy(ext,get_extension(path));
    strupr(ext);

    for(int32_t i=0; i<ssfmtMAX; ++i)
    {
        if(stricmp(ext,snapshotformat_str[i][1])==0) return ftBMP;
    }

    if(stricmp(ext,"til")==0) return ftTIL;

    if(stricmp(ext,"zgp")==0) return ftZGP;

    if(stricmp(ext,"qsu")==0) return ftQSU;

    if(stricmp(ext,"zqt")==0) return ftZQT;

    if(stricmp(ext,"qst")==0) return ftQST;

    if(stricmp(ext,"dat")==0) return 0;

    if(stricmp(ext,"htm")==0) return 0;

    if(stricmp(ext,"html")==0) return 0;

    if(stricmp(ext,"txt")==0) return 0;

    if(stricmp(ext,"zip")==0) return 0;

    return ftBIN;
}

int32_t cursorColor(int32_t col)
{
	switch(col)
	{
		case dvc(1):
		case dvc(4):
			return jwin_pal[jcCURSORMISC];
		case dvc(2):
			return jwin_pal[jcCURSOROUTLINE];
		case dvc(3):
			return jwin_pal[jcCURSORLIGHT];
		case dvc(5):
			return jwin_pal[jcCURSORDARK];
	}
	return col;
}

void load_mice()
{
	scare_mouse();
	set_mouse_sprite(NULL);
	int32_t sz = vbound(int32_t(16*(zc_get_config("zquest","cursor_scale_large",1.5))),16,80);
	for(int32_t i=0; i<MOUSE_BMP_MAX; i++)
	{
		for(int32_t j=0; j<4; j++)
		{
			if(mouse_bmp[i][j]) destroy_bitmap(mouse_bmp[i][j]);
			if(mouse_bmp_1x[i][j]) destroy_bitmap(mouse_bmp_1x[i][j]);
			mouse_bmp[i][j] = create_bitmap_ex(8,sz,sz);
			mouse_bmp_1x[i][j] = create_bitmap_ex(8,16,16);
			BITMAP* tmpbmp = create_bitmap_ex(8,16,16);
			BITMAP* subbmp = create_bitmap_ex(8,16,16);
			clear_bitmap(tmpbmp);
			clear_bitmap(subbmp);
			blit((BITMAP*)zcdata[BMP_MOUSEZQ].dat,tmpbmp,i*17+1,j*17+1,0,0,16,16);
			for(int32_t x = 0; x < 16; ++x)
			{
				for(int32_t y = 0; y < 16; ++y)
				{
					putpixel(subbmp, x, y, cursorColor(getpixel(tmpbmp, x, y)));
				}
			}
			if(sz!=16)
				stretch_blit(subbmp, mouse_bmp[i][j], 0, 0, 16, 16, 0, 0, sz, sz);
			else
				blit(subbmp, mouse_bmp[i][j], 0, 0, 0, 0, 16, 16);
			blit(subbmp, mouse_bmp_1x[i][j], 0, 0, 0, 0, 16, 16);
			destroy_bitmap(tmpbmp);
			destroy_bitmap(subbmp);
		}
	}
	restore_mouse();
	unscare_mouse();
}

void load_icons()
{
    for(int32_t i=0; i<ICON_BMP_MAX; i++)
    {
        for(int32_t j=0; j<4; j++)
        {
            icon_bmp[i][j] = create_bitmap_ex(8,16,16);
            blit((BITMAP*)zcdata[BMP_ICONS].dat,icon_bmp[i][j],i*17+1,j*17+1,0,0,16,16);
        }
    }
}

void load_selections()
{
    for(int32_t i=0; i<2; i++)
    {
        select_bmp[i] = create_bitmap_ex(8,16,16);
        //  blit((BITMAP*)zcdata[BMP_SELECT].dat,select_bmp[i],i*17+1,1,0,0,16,16);
        blit((BITMAP*)zcdata[BMP_SELECT].dat,select_bmp[i],i*17+1,1,0,0,16,16);
    }
}

void load_arrows()
{
    for(int32_t i=0; i<MAXARROWS; i++)
    {
        arrow_bmp[i] = create_bitmap_ex(8,16,16);
		BITMAP* tmpbmp = create_bitmap_ex(8,16,16);
        blit((BITMAP*)zcdata[BMP_ARROWS].dat,tmpbmp,i*17+1,1,0,0,16,16);
		for(int32_t x = 0; x < 16; ++x)
		{
			for(int32_t y = 0; y < 16; ++y)
			{
				putpixel(arrow_bmp[i], x, y, cursorColor(getpixel(tmpbmp, x, y)));
			}
		}
		destroy_bitmap(tmpbmp);
    }
}

void dump_pal()
{
    for(int32_t i=0; i<256; i++)
        rectfill(screen,(i&63)<<2,(i&0xFC0)>>4,((i&63)<<2)+3,((i&0xFC0)>>4)+3,i);
}

int32_t wrap(int32_t x,int32_t low,int32_t high)
{
    while(x<low)
        x+=high-low+1;

    while(x>high)
        x-=high-low+1;

    return x;
}

bool readfile(const char *path,void *buf,int32_t count)
{
    PACKFILE *f=pack_fopen_password(path,F_READ,"");

    if(!f)
        return 0;

    bool good=pfread(buf,count,f,true);
    pack_fclose(f);
    return good;
}

bool writefile(const char *path,void *buf,int32_t count)
{
    PACKFILE *f=pack_fopen_password(path,F_WRITE,"");

    if(!f)
        return 0;

    bool good=pfwrite(buf,count,f);
    pack_fclose(f);
    return good;
}

/* dotted_rect: (from allegro's guiproc.c)
  *  Draws a dotted rectangle, for showing an object has the input focus.
  */
void dotted_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t fg, int32_t bg)
{
    int32_t x = ((x1+y1) & 1) ? 1 : 0;
    int32_t c;

    /* two loops to avoid bank switches */
    for(c=x1; c<=x2; c++)
    {
        putpixel(screen, c, y1, (((c+y1) & 1) == x) ? fg : bg);
    }

    for(c=x1; c<=x2; c++)
    {
        putpixel(screen, c, y2, (((c+y2) & 1) == x) ? fg : bg);
    }

    for(c=y1+1; c<y2; c++)
    {
        putpixel(screen, x1, c, (((c+x1) & 1) == x) ? fg : bg);
        putpixel(screen, x2, c, (((c+x2) & 1) == x) ? fg : bg);
    }
}

void reset_pal_cycling();
void cycle_palette();

void load_cset(RGB *pal,int32_t cset_index,int32_t dataset)
{
    byte *si = colordata + CSET(dataset)*3;

    for(int32_t i=0; i<16; i++)
    {
        pal[CSET(cset_index)+i] = _RGB(si);
        si+=3;
    }
}

void set_pal()
{
    set_palette_range(RAMpal,0,0xE0,true);
}

void loadlvlpal(int32_t level)
{
	Color=level;

	// full pal
	for(int32_t i=0; i<0xE0; i++)
		RAMpal[i] = _RGB(colordata+i*3);

	// level pal
	byte *si = colordata + CSET(level*pdLEVEL+poLEVEL)*3;

	for(int32_t i=0; i<16*3; i++)
	{
		RAMpal[CSET(2)+i] = _RGB(si);
		si+=3;
	}

	for(int32_t i=0; i<16; i++)
	{
		RAMpal[CSET(9)+i] = _RGB(si);
		si+=3;
	}
	
	if (get_bit(quest_rules, qr_CSET1_LEVEL))
	{
		si = colordata + CSET(level*pdLEVEL+poNEWCSETS)*3;
		for(int32_t i=0; i<16; i++)
		{
			RAMpal[CSET(1)+i] = _RGB(si);
			si+=3;
		}
	}
	if (get_bit(quest_rules, qr_CSET5_LEVEL))
	{
		si = colordata + CSET(level*pdLEVEL+poNEWCSETS+1)*3;
		for(int32_t i=0; i<16; i++)
		{
			RAMpal[CSET(5)+i] = _RGB(si);
			si+=3;
		}
	}
	if (get_bit(quest_rules, qr_CSET7_LEVEL))
	{
		si = colordata + CSET(level*pdLEVEL+poNEWCSETS+2)*3;
		for(int32_t i=0; i<16; i++)
		{
			RAMpal[CSET(7)+i] = _RGB(si);
			si+=3;
		}
	}
	if (get_bit(quest_rules, qr_CSET8_LEVEL))
	{
		si = colordata + CSET(level*pdLEVEL+poNEWCSETS+3)*3;
		for(int32_t i=0; i<16; i++)
		{
			RAMpal[CSET(8)+i] = _RGB(si);
			si+=3;
		}
	}

	reset_pal_cycling();
	set_pal();
}

void loadfadepal(int32_t dataset)
{
    byte *si = colordata + CSET(dataset)*3;

    for(int32_t i=0; i<16*3; i++)
    {
        RAMpal[CSET(2)+i] = _RGB(si);
        si+=3;
    }

    set_pal();
}

ALLEGRO_COLOR real_lc1(int pal)
{
	return a5color(_RGB(colordata+(CSET(pal*pdLEVEL+poLEVEL)+2)*3));
}
ALLEGRO_COLOR real_lc2(int pal)
{
	return a5color(_RGB(colordata+(CSET(pal*pdLEVEL+poLEVEL)+16+1)*3));
}

void refresh_pal()
{
    loadlvlpal(Color);
    set_palette(RAMpal);
}

char ns_string[4];

// Mirrored in hero.cpp
const char *roomtype_string[MAXROOMTYPES] =
{
    "(None)","Special Item","Pay for Info","Secret Money","Gamble",
    "Door Repair","Red Potion or Heart Container","Feed the Goriya","Triforce Check",
    "Potion Shop","Shop","More Bombs","Leave Money or Life","10 Rupees",
    "3-Stair Warp","Ganon","Zelda", "-<item pond>", "1/2 Magic Upgrade", "Learn Slash", "More Arrows","Take One Item"
};

const char *catchall_string[MAXROOMTYPES] =
{
    "Generic Catchall","Special Item","Info Type","Amount","Generic Catchall","Repair Fee","Generic Catchall","Generic Catchall","Generic Catchall","Shop Type",
    "Shop Type","Price","Price","Generic Catchall","Warp Ring","Generic Catchall","Generic Catchall", "Generic Catchall", "Generic Catchall",
    "Generic Catchall", "Price","Shop Type","Bottle Shop Type"
};

const char *warptype_string[MAXWARPTYPES] =
{
    "Cave/Item Cellar","Passageway","Entrance/Exit","Scrolling Warp","Insta-Warp","Insta-Warp with Blackout","Insta-Warp with Opening Wipe","Insta-Warp with Zap Effects", "Insta-Warp with Wave Effects", "Cancel Warp"
    //  "Cave/Item Cellar","Passageway","Entrance/Exit","Scrolling Warp","Standard","Cancel Warp"," "," ", " ", " "
};

const char *warpeffect_string[MAXWARPEFFECTS] =
{
    "Instant", "Circle", "Oval", "Triangle", "Super Mario All-Stars", "Curtains (Smooth)", "Curtains (Stepped)", "Mosaic",
    "Wave White", "Wave Black", "Fade White", "Fade Black", "Global Opening/Closing", "Destination Default"
};

const char *flag_string[MAXFLAGS] =
{
    "  0 (None)",
    "  1 Push Block (Vertical, Trigger)",
    "  2 Push Block (4-Way, Trigger)",
    "  3 Whistle Trigger",
    "  4 Burn Trigger (Any)",
    "  5 Arrow Trigger (Any)",
    "  6 Bomb Trigger (Any)",
    "  7 Fairy Ring (Life)",
    "  8 Raft Path",
    "  9 Armos -> Secret",
    " 10 Armos/Chest -> Item",
    " 11 Bomb (Super)",
    " 12 Raft Branch",
    " 13 Dive -> Item",
    " 14 Lens Marker",
    " 15 Zelda (Win Game)",
    " 16 Secret Tile 0",
    " 17 Secret Tile 1",
    " 18 Secret Tile 2",
    " 19 Secret Tile 3",
    " 20 Secret Tile 4",
    " 21 Secret Tile 5",
    " 22 Secret Tile 6",
    " 23 Secret Tile 7",
    " 24 Secret Tile 8",
    " 25 Secret Tile 9",
    " 26 Secret Tile 10",
    " 27 Secret Tile 11",
    " 28 Secret Tile 12",
    " 29 Secret Tile 13",
    " 30 Secret Tile 14",
    " 31 Secret Tile 15",
    " 32 Trap (Horizontal, Line of Sight)",
    " 33 Trap (Vertical, Line of Sight)",
    " 34 Trap (4-Way, Line of Sight)",
    " 35 Trap (Horizontal, Constant)",
    " 36 Trap (Vertical, Constant)",
    " 37 Enemy 0",
    " 38 Enemy 1",
    " 39 Enemy 2",
    " 40 Enemy 3",
    " 41 Enemy 4",
    " 42 Enemy 5",
    " 43 Enemy 6",
    " 44 Enemy 7",
    " 45 Enemy 8",
    " 46 Enemy 9",
    " 47 Push Block (Horiz, Once, Trigger)",
    " 48 Push Block (Up, Once, Trigger)",
    " 49 Push Block (Down, Once, Trigger)",
    " 50 Push Block (Left, Once, Trigger)",
    " 51 Push Block (Right, Once, Trigger)",
    " 52 Push Block (Vert, Once)",
    " 53 Push Block (Horizontal, Once)",
    " 54 Push Block (4-Way, Once)",
    " 55 Push Block (Up, Once)",
    " 56 Push Block (Down, Once)",
    " 57 Push Block (Left, Once)",
    " 58 Push Block (Right, Once)",
    " 59 Push Block (Vertical, Many)",
    " 60 Push Block (Horizontal, Many)",
    " 61 Push Block (4-Way, Many)",
    " 62 Push Block (Up, Many)",
    " 63 Push Block (Down, Many)",
    " 64 Push Block (Left, Many)",
    " 65 Push Block (Right, Many)",
    " 66 Block Trigger",
    " 67 No Push Blocks",
    " 68 Boomerang Trigger (Any)",
    " 69 Boomerang Trigger (Magic +)",
    " 70 Boomerang Trigger (Fire)",
    " 71 Arrow Trigger (Silver +)",
    " 72 Arrow Trigger (Golden)",
    " 73 Burn Trigger (Red Candle +)",
    " 74 Burn Trigger (Wand Fire)",
    " 75 Burn Trigger (Din's Fire)",
    " 76 Magic Trigger (Wand)",
    " 77 Magic Trigger (Reflected)",
    " 78 Fireball Trigger (Reflected)",
    " 79 Sword Trigger (Any)",
    " 80 Sword Trigger (White +)",
    " 81 Sword Trigger (Magic +)",
    " 82 Sword Trigger (Master)",
    " 83 Sword Beam Trigger (Any)",
    " 84 Sword Beam Trigger (White +)",
    " 85 Sword Beam Trigger (Magic +)",
    " 86 Sword Beam Trigger (Master)",
    " 87 Hookshot Trigger",
    " 88 Wand Trigger",
    " 89 Hammer Trigger",
    " 90 Strike Trigger",
    " 91 Block Hole (Block -> Next)",
    " 92 Fairy Ring (Magic)",
    " 93 Fairy Ring (All)",
    " 94 Trigger -> Self Only",
    " 95 Trigger -> Self, Secret Tiles",
    " 96 No Enemies",
    " 97 No Ground Enemies",
    " 98 General Purpose 1 (Scripts)",
    " 99 General Purpose 2 (Scripts)",
    "100 General Purpose 3 (Scripts)",
    "101 General Purpose 4 (Scripts)",
    "102 General Purpose 5 (Scripts)",
    "103 Raft Bounce",
     "104 Pushed",
    "105 General Purpose 6 (Scripts)",
    "106 General Purpose 7 (Scripts)",
    "107 General Purpose 8 (Scripts)",
    "108 General Purpose 9 (Scripts)",
    "109 General Purpose 10 (Scripts)",
    "110 General Purpose 11 (Scripts)",
    "111 General Purpose 12 (Scripts)",
    "112 General Purpose 13 (Scripts)",
    "113 General Purpose 14 (Scripts)",
    "114 General Purpose 15 (Scripts)",
    "115 General Purpose 16 (Scripts)",
    "116 General Purpose 17 (Scripts)",
    "117 General Purpose 18 (Scripts)",
    "118 General Purpose 19 (Scripts)",
    "119 General Purpose 20 (Scripts)",
    "120 Pit or Hole (Scripted)",
    "121 Pit or Hole, Fall Down Floor (Scripted)",
    "122 Fire or Lava (Scripted)",
    "123 Ice (Scripted)",
    "124 Ice, Damaging (Scripted)",
    "125 Damage-1 (Scripted)",
    "126 Damage-2 (Scripted)",
    "127 Damage-4 (Scripted)",
    "128 Damage-8 (Scripted)",
    "119 Damage-16 (Scripted)",
    "130 Damage-32 (Scripted)",
    "131 Freeze Screen (Unimplemented)",
    "132 Freeze Screen, Except FFCs (Unimplemented)",
    "133 Freeze FFCs Only (Unimplemented)",
    "134 Trigger LW_SCRIPT1 (Unimplemented)",
    "135 Trigger LW_SCRIPT2 (Unimplemented)",
    "136 Trigger LW_SCRIPT3 (Unimplemented)",
    "137 Trigger LW_SCRIPT4 (Unimplemented)",
    "138 Trigger LW_SCRIPT5 (Unimplemented)",
    "139 Trigger LW_SCRIPT6 (Unimplemented)",
    "140 Trigger LW_SCRIPT7 (Unimplemented)",
    "141 Trigger LW_SCRIPT8 (Unimplemented)",
    "142 Trigger LW_SCRIPT9 (Unimplemented)",
    "143 Trigger LW_SCRIPT10 (Unimplemented)",
    "144 Dig Spot (Scripted)",
    "145 Dig Spot, Next (Scripted)",
    "146 Dig Spot, Special Item (Scripted)",
    "147 Pot, Slashable (Scripted)",
    "148 Pot, Liftable (Scripted)",
    "149 Pot, Slash or Lift (Scripted)",
    "150 Rock, Lift Normal (Scripted)",
    "151 Rock, Lift Heavy (Scripted)",
    "152 Dropset Item (Scripted)",
    "153 Special Item (Scripted)",
    "154 Drop Key (Scripted)",
    "155 Drop level-Specific Key (Scripted)",
    "156 Drop Compass (Scripted)",
    "157 Drop Map (Scripted)",
    "158 Drop Bosskey (Scripted)",
    "159 Spawn NPC (Scripted)",
    "160 SwitchHook Spot (Scripted)",
    "161 Sideview Ladder",
    "162 Sideview Platform","163 Spawn No Enemies","164 Spawn All Enemies","165 Secrets->Next","166 No Mirroring","167 Unsafe Ground","168 mf168","169 mf169",
    "170 mf170","171 mf171","172 mf172","173 mf173","174 mf174","175 mf175","176 mf176","177 mf177","178 mf178","179 mf179",
    "180 mf180","181 mf181","182 mf182","183 mf183","184 mf184","185 mf185","186 mf186","187 mf187","188 mf188","189 mf189",
    "190 mf190","191 mf191","192 mf192","193 mf193","194 mf194","195 mf195","196 mf196","197 mf197","198 mf198","199 mf199",
    "200 mf200","201 mf201","202 mf202","203 mf203","204 mf204","205 mf205","206 mf206","207 mf207","208 mf208","209 mf209",
    "210 mf210","211 mf211","212 mf212","213 mf213","214 mf214","215 mf215","216 mf216","217 mf217","218 mf218","219 mf219",
    "220 mf220","221 mf221","222 mf222","223 mf223","224 mf224","225 mf225","226 mf226","227 mf227","228 mf228","229 mf229",
    "230 mf230","231 mf231","232 mf232","233 mf233","234 mf234","235 mf235","236 mf236","237 mf237","238 mf238","239 mf239",
    "240 mf240","241 mf241","242 mf242","243 mf243","244 mf244","245 mf245","246 mf246","247 mf247","248 mf248","249 mf249",
    "250 mf250","251 mf251","252 mf252","253 mf253","254 mf254",
    "255 Extended (Extended Flag Editor)"
};

// eMAXGUYS is defined in zdefs.h
// Strings with a trailing space will not appear in the ZQ editor.
// Remove the trailing space (e.g. "Ghini (L2, Magic) " become "Ghini (L2, Magic)"
// to make them visible and editable. -Z
// Add a trailing space to make any invisible (hidden) in the editor.
// This is what is used by build_bie_list() in zquest.cpp to generate the enemy lists! -Z
const char *old_guy_string[OLDMAXGUYS] =
{
    "(None)","Abei","Ama","Merchant","Moblin","Fire","Fairy","Goriya","Zelda","Abei 2","Empty","","","","","","","","","",
    // 020
    "Octorok (L1, Slow)","Octorok (L2, Slow)","Octorok (L1, Fast)","Octorok (L2, Fast)","Tektite (L1)",
    // 025
    "Tektite (L2)","Leever (L1)","Leever (L2)","Moblin (L1)","Moblin (L2)",
    // 030
    "Lynel (L1)","Lynel (L2)","Peahat (L1)","Zora","Rock",
    // 035
    "Ghini (L1, Normal)","Ghini (L1, Phantom)","Armos","Keese (CSet 7)","Keese (CSet 8)",
    // 040
    "Keese (CSet 9)","Stalfos (L1)","Gel (L1, Normal)","Zol (L1, Normal)","Rope (L1)",
    // 045
    "Goriya (L1)","Goriya (L2)","Trap (4-Way)","Wall Master","Darknut (L1)",
    // 050
    "Darknut (L2)","Bubble (Sword, Temporary Disabling)","Vire (Normal)","Like Like","Gibdo",
    // 055
    "Pols Voice (Arrow)","Wizzrobe (Teleporting)","Wizzrobe (Floating)","Aquamentus (Facing Left)","Moldorm",
    // 060
    "Dodongo","Manhandla (L1)","Gleeok (1 Head)","Gleeok (2 Heads)","Gleeok (3 Heads)",
    // 065
    "Gleeok (4 Heads)","Digdogger (1 Kid)","Digdogger (3 Kids)","Digdogger Kid (1)","Digdogger Kid (2)",
    // 070
    "Digdogger Kid (3)","Digdogger Kid (4)","Gohma (L1)","Gohma (L2)","Lanmola (L1)",
    // 075
    "Lanmola (L2)","Patra (L1, Big Circle)","Patra (L1, Oval)","Ganon","Stalfos (L2)",
    // 080
    "Rope (L2)","Bubble (Sword, Permanent Disabling)","Bubble (Sword, Re-enabling)","Shooter (Fireball)","Item Fairy ",
    // 085
    "Fire","Octorok (Magic)", "Darknut (Death Knight)", "Gel (L1, Tribble)", "Zol (L1, Tribble)",
    // 090
    "Keese (Tribble)", "Vire (Tribble)", "Darknut (Splitting)", "Aquamentus (Facing Right)", "Manhandla (L2)",
    // 095
    "Trap (Horizontal, Line of Sight)", "Trap (Vertical, Line of Sight)", "Trap (Horizontal, Constant)", "Trap (Vertical, Constant)", "Wizzrobe (Fire)",
    // 100
    "Wizzrobe (Wind)", "Ceiling Master ", "Floor Master ", "Patra (BS Zelda)", "Patra (L2)",
    // 105
    "Patra (L3)", "Bat", "Wizzrobe (Bat)", "Wizzrobe (Bat 2) ", "Gleeok (Fire, 1 Head)",
    // 110
    "Gleeok (Fire, 2 Heads)",  "Gleeok (Fire, 3 Heads)","Gleeok (Fire, 4 Heads)", "Wizzrobe (Mirror)", "Dodongo (BS Zelda)",
    // 115
    "Dodongo (Fire) ","Trigger", "Bubble (Item, Temporary Disabling)", "Bubble (Item, Permanent Disabling)", "Bubble (Item, Re-enabling)",
    // 120
    "Stalfos (L3)", "Gohma (L3)", "Gohma (L4)", "NPC 1 (Standing)", "NPC 2 (Standing)",
    // 125
    "NPC 3 (Standing)", "NPC 4 (Standing)", "NPC 5 (Standing)", "NPC 6 (Standing)", "NPC 1 (Walking)",
    // 130
    "NPC 2 (Walking)", "NPC 3 (Walking)", "NPC 4 (Walking)", "NPC 5 (Walking)", "NPC 6 (Walking)",
    // 135
    "Boulder", "Goriya (L3)", "Leever (L3)", "Octorok (L3, Slow)", "Octorok (L3, Fast)",
    // 140
    "Octorok (L4, Slow)", "Octorok (L4, Fast)", "Trap (8-Way)", "Trap (Diagonal)", "Trap (/, Constant)",
    // 145
    "Trap (/, Line of Sight)", "Trap (\\, Constant)", "Trap (\\, Line of Sight)", "Trap (CW, Constant)", "Trap (CW, Line of Sight)",
    // 150
    "Trap (CCW, Constant)", "Trap (CCW, Line of Sight)", "Wizzrobe (Summoner)", "Wizzrobe (Ice)", "Shooter (Magic)",
    // 155
    "Shooter (Rock)", "Shooter (Spear)", "Shooter (Sword)", "Shooter (Fire)", "Shooter (Fire 2)",
    // 160
    "Bombchu", "Gel (L2, Normal)", "Zol (L2, Normal)", "Gel (L2, Tribble)", "Zol (L2, Tribble)",
    // 165
    "Tektite (L3)", "Spinning Tile (Combo)", "Spinning Tile (Enemy Sprite)", "Lynel (L3)", "Peahat (L2)",
    // 170
    "Pols Voice (Magic)", "Pols Voice (Whistle)", "Darknut (Mirror)", "Ghini (L2, Fire)", "Ghini (L2, Magic)",
    // 175
    "Grappler Bug (HP)", "Grappler Bug (MP)"
};

char *guy_string[eMAXGUYS];

const char *enetype_string[eeMAX] =
{
    "-Guy",
    "Walking Enemy",
    "-Unused",
    "Tektite",
    "Leever",
    "Peahat",
    "Zora",
    "Rock",
    "Ghini",
    "-Unused",
    "Keese",
    "-Unused",
    "-Unused",
    "-Unused",
    "-Unused",
    "Trap",
    "Wall Master",
    "-Unused",
    "-Unused",
    "-Unused",
    "-Unused",
    "Wizzrobe",
    "Aquamentus",
    "Moldorm",
    "Dodongo",
    "Manhandla",
    "Gleeok",
    "Digdogger",
    "Gohma",
    "Lanmola",
    "Patra",
    "Ganon",
    "Projectile Shooter",
    "-Unused",
    "-Unused",
    "-Unused",
    "-Unused",
    "Spin Tile",
    "(None)",
    "-Fairy",
    "Other (Floating)",
    "Other",
    "-max250",
    "Custom 01", "Custom 02", "Custom 03", "Custom 04", "Custom 05",
    "Custom 06", "Custom 07", "Custom 08", "Custom 09", "Custom 10",
    "Custom 11", "Custom 12", "Custom 13", "Custom 14", "Custom 15",
    "Custom 16", "Custom 17", "Custom 18", "Custom 19", "Custom 20",
    "Friendly NPC 01", "Friendly NPC 02", "Friendly NPC 03", "Friendly NPC 04",
    "Friendly NPC 05", "Friendly NPC 06", "Friendly NPC 07",
    "Friendly NPC 08", "Friendly NPC 09", "Friendly NPC 10"
};

const char *eneanim_string[aMAX] =
{
    "(None)",
    "Flip",
    "-Unused",
    "2-Frame",
    "-Unused",
    "Octorok (NES)",
    "Tektite (NES)",
    "Leever (NES)",
    "Walker",
    "Zora (NES)",
    "Zora (4-Frame)",
    "Ghini",
    "Armos (NES)",
    "Rope",
    "Wall Master (NES)",
    "Wall Master (4-Frame)",
    "Darknut (NES)",
    "Vire",
    "3-Frame",
    "Wizzrobe (NES)",
    "Aquamentus",
    "Dodongo (NES)",
    "Manhandla",
    "Gleeok",
    "Digdogger",
    "Gohma",
    "Lanmola",
    "2-Frame Flying",
    "4-Frame 4-Dir + Tracking",
    "4-Frame 8-Dir + Tracking",
    "4-Frame 4-Dir + Firing",
    "4-Frame 4-Dir",
    "4-Frame 8-Dir + Firing",
    "Armos (4-Frame)",
    "4-Frame Flying 4-Dir",
    "4-Frame Flying 8-Dir",
    "-Unused",
    "4-Frame 8-Dir Big",
    "Tektite (4-Frame)",
    "3-Frame 4-Dir",
    "2-Frame 4-Dir",
    "Leever (4-Frame)",
    "2-Frame 4-Dir + Tracking",
    "Wizzrobe (4-Frame)",
    "Dodongo (4-Frame)",
    "Dodongo BS (4-Frame)",
    "4-Frame Flying 8-Dir + Firing",
    "4-Frame Flying 4-Dir + Firing",
    "4-Frame",
    "Ganon",
    "2-Frame Big",
    "4-Frame 8-Dir Big + Tracking", 
    "4-Frame 4-Dir Big + Tracking", 
    "4-Frame 8-Dir Big + Firing",
    "4-Frame 4-Dir Big", 
    "4-Frame 4-Dir Big + Firing"
};

/*
char *itemset_string[isMAX] =
{
	"(None)",
	"Default",
	"Bombs",
	"Rupees",
	"Life",
	"Bombs 100%",
	"Super Bombs 100%",
	"Magic",
	"Magic + Bombs",
	"Magic + Rupees",
	"Magic + Life",
	"Magic 2"
};
*/

const char *eweapon_string[wMax-wEnemyWeapons] =
{
    "(None)",
    "Fireball",
    "Arrow",
    "Boomerang",
    "Sword",
    "Rock",
    "Magic",
    "Bomb Blast",
    "Super Bomb Blast",
    "Lit Bomb",
    "Lit Super Bomb",
    "Fire Trail",
    "Flame",
    "Wind",
    "Flame 2",
    "-Flame 2 Trail <unused>",
    "-Ice <unused>",
    "Fireball (Rising)"
};

const char *walkmisc1_string[e1tLAST] =
{
    "1 Shot", "1 (End-Halt)", "Rapid-Fire", "1 (Fast)", "1 (Slanted)", "3 Shots", "4 Shots", "5 Shots", "3 (Fast)", "Breath", "8 Shots", "Summon", "Summon (Layer)"
};
const char *walkmisc2_string[e2tTRIBBLE+1] =
{
    "Normal", "Split On Hit", "Split On Death", "8 Shots", "Explode", "Tribble"
};

const char *walkmisc7_string[e7tEATHURT+1] =
{
    "None", "Temp. Jinx", "Perm. Jinx", "Cure Jinx", "Lose Magic", "Lose Rupees", "Drunk", "Eat (Items)", "Eat (Magic)", "Eat (Rupees)", "Eat (Damage)"
};

const char *walkmisc9_string[e9tARMOS+1] =
{
    "Normal", "Rope", "Vire", "Pols Voice", "Armos"
};

const char *patramisc28_string[patratLAST] =
{
    "1 Shot", "1 (Fast)", "3 Shots", "3 (Fast)", "5 Shots", "5 (Fast)", "4 (Cardinal)", "4 (Diagonal)", "4 (Card/Diag Random)", "8 Shots", "Breath", "Stream"
};

const char *pattern_string[MAXPATTERNS] =
{
    "Spawn (Classic)", "Enter from Sides (Consecutive)", "Enter from Sides (Random)", "Fall From Ceiling (Classic)", "Fall From Ceiling (Random)", "Spawn (Random)", "Spawn No Enemies"
};

const char *short_pattern_string[MAXPATTERNS] =
{
    "Spawn (C)", "Sides", "Sides (R)", "Ceiling (C)", "Ceiling (R)", "Spawn (R)", "No Spawning"
};

const char *midi_string[MAXCUSTOMMIDIS_ZQ] =
{
    "(None)",
    "Overworld",
    "Dungeon",
    "Level 9",
};

const char *screen_midi_string[MAXCUSTOMMIDIS_ZQ+1] =
{
    "Use DMap MIDI",
    "(None)",
    "Overworld",
    "Dungeon",
    "Level 9",
};

void domouse();
void init_doorcombosets();

int32_t onNew();
int32_t PickRuleset();
int32_t PickRuleTemplate();
int32_t onOpen();
int32_t onOpen2();
int32_t onRevert();
int32_t onSave();
int32_t onSaveAs();
int32_t onQuestTemplates();

int32_t onUndo();
int32_t onCopy();
int32_t onPaste();
int32_t onPasteAll();
int32_t onPasteToAll();
int32_t onPasteAllToAll();
int32_t onDelete();
int32_t onDeleteMap();

int32_t onTemplate();
int32_t onDoors();
int32_t onCSetFix();
int32_t onFlags();
int32_t onShowPal();
int32_t onReTemplate();

int32_t playTune();
int32_t playMIDI();
int32_t stopMIDI();
int32_t onKeyFile();

int32_t onUp();
int32_t onDown();
int32_t onLeft();
int32_t onRight();
int32_t onPgUp();
int32_t onPgDn();
int32_t onIncreaseCSet();
int32_t onDecreaseCSet();

int32_t  onHelp();
void doHelp(int32_t bg,int32_t fg);

int32_t onScrData();
int32_t onGuy();
int32_t onEndString();
int32_t onString();
int32_t onRType();
int32_t onCatchall();
int32_t onItem();
int32_t onWarp();
int32_t onWarp2();
int32_t onPath();
int32_t onEnemies();
int32_t onEnemyFlags();
int32_t onUnderCombo();
int32_t onSecretCombo();

int32_t onHeader();
int32_t onAnimationRules();
int32_t onComboRules();
int32_t onItemRules();
int32_t onEnemyRules();
int32_t onFixesRules();
int32_t onMiscRules();
int32_t onCompatRules();
int32_t onRules2();
int32_t onCheats();
int32_t onStrings();
int32_t onDmaps();
int32_t onTiles();
int32_t onCombos();
int32_t onMidis();
int32_t onShopTypes();
int32_t onInfoTypes();
int32_t onWarpRings();
int32_t onWhistle();
int32_t onMiscColors();
int32_t onMapStyles();
int32_t onTemplates();
int32_t onDoorCombos();
int32_t onTriPieces();
int32_t onIcons();
int32_t onInit();
int32_t onLayers();
int32_t onScreenPalette();

int32_t onColors_Main();
int32_t onColors_Levels();
int32_t onColors_Sprites();

int32_t onImport_Map();
int32_t onImport_DMaps();
int32_t onImport_Msgs();
int32_t onImport_Combos();
int32_t onImport_Tiles();
int32_t onImport_Subscreen();
int32_t onImport_Pals();
int32_t onImport_ZGP();
int32_t onImport_ZQT();
int32_t onImport_UnencodedQuest();

int32_t onExport_Map();
int32_t onExport_DMaps();
int32_t onExport_Msgs();
int32_t onExport_MsgsText();
int32_t onExport_Combos();
int32_t onExport_Tiles();
int32_t onExport_Subscreen();
int32_t onExport_Pals();
int32_t onExport_ZGP();
int32_t onExport_ZQT();
int32_t onExport_UnencodedQuest();

int32_t onGotoMap();

int32_t onViewPic();
int32_t onViewMap();
int32_t onComboPage();

int32_t onDefault_Pals();
int32_t onDefault_Tiles();
int32_t onDefault_Combos();
int32_t onDefault_Sprites();
int32_t onDefault_MapStyles();

int32_t onCustomItems();
int32_t onCustomWpns();
int32_t onCustomHero();
int32_t onCustomGuys();

int32_t onTest();
int32_t onTestOptions();

int32_t onOptions();

void draw_checkbox(BITMAP *dest,int32_t x,int32_t y,int32_t bg,int32_t fg,bool value);
void draw_layerradio(BITMAP *dest,int32_t x,int32_t y,int32_t bg,int32_t fg,int32_t value);
void KeyFileName(char *kfname);


extern int32_t draw_mode;
extern int32_t alias_origin;

int32_t onSpacebar()
{
    if(draw_mode==3)
    {
        alias_origin=(alias_origin+1)%4;
        return D_O_K;
    }

    combo_cols=!combo_cols;
    return D_O_K;
}

int32_t onClearQuestFilepath()
{
	if(jwin_alert3(
			"Clear Quest Path",
			"Clear the current default filepath?",
			NULL,
			NULL,
		 "&Yes",
		"&No",
		NULL,
		'y',
		'n',
		0,
		lfont) == 1)
	{
		ZQ_ClearQuestPath();
		save_config_file();
		return D_O_K;
	}
	else return D_O_K;

}

char const* getSnapName()
{
	static char snapbuf[200];
	int32_t num = 0;
	
	do
	{
		sprintf(snapbuf, "%szquest_screen%05d.%s", get_snap_str(), ++num, snapshotformat_str[SnapshotFormat][1]);
	}
	while(num<99999 && exists(snapbuf));
	
	return snapbuf;
}

int32_t onSnapshot()
{
    blit(screen,screen2,0,0,0,0,zq_screen_w,zq_screen_h);
    PALETTE RAMpal2;
    get_palette(RAMpal2);
    save_bitmap(getSnapName(),screen2,RAMpal2);
    return D_O_K;
}

int32_t onMapscrSnapshot()
{
	int32_t x = showedges?16:0;
	int32_t y = showedges?16:0;

	PALETTE usepal;
	get_palette(usepal);

	bool useflags = (key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]); //Only use visibility flags (flags, walkability, etc) if CTRL is held
	int32_t misal = ShowMisalignments; //Store misalignments, so it can be disabled, and restored after.
	ShowMisalignments = 0;

	BITMAP *panorama = create_bitmap_ex(8,256,176);
	Map.setCurrScr(Map.getCurrScr());                                 // to update palette
	clear_to_color(panorama,vc(0));

	Map.draw(panorama, 0, 0, useflags?Flags:0, -1, -1);
	
	save_bitmap(getSnapName(),panorama,usepal);
	destroy_bitmap(panorama);

	ShowMisalignments = misal; //Restore misalignments.

	return D_O_K;
}


int32_t gocnt=0;

void go()
{
    switch(gocnt)
    {
    case 0:
        scare_mouse();
        blit(screen,menu1,0,0,0,0,zq_screen_w,zq_screen_h);
        unscare_mouse();
        break;

    case 1:
        scare_mouse();
        blit(screen,menu3,0,0,0,0,zq_screen_w,zq_screen_h);
        unscare_mouse();
        break;

    default:
        return;
    }

    ++gocnt;
}

void comeback()
{
    switch(gocnt)
    {
    case 1:
        scare_mouse();
        blit(menu1,screen,0,0,0,0,zq_screen_w,zq_screen_h);
        unscare_mouse();
        break;

    case 2:
        scare_mouse();
        blit(menu3,screen,0,0,0,0,zq_screen_w,zq_screen_h);
        unscare_mouse();
        break;

    default:
        return;
    }

    --gocnt;
}

int32_t checksave()
{
    if(saved)
        return 1;

    char buf[256+20];
    char *name = get_filename(filepath);

    if(name[0]==0)
        sprintf(buf,"Save this quest file?");
    else
        sprintf(buf,"Save changes to %s?",name);

    switch(jwin_alert3("ZQuest",buf,NULL,NULL,"&Yes","&No","Cancel",'y','n',27,lfont))
    {
    case 1:
        onSave();
        return 1;

    case 2:
        return 1;
    }

    return 0;
}

int32_t onExit()
{
#ifdef __EMSCRIPTEN__
	return D_O_K;
#endif

    restore_mouse();

    if(checksave()==0)
        return D_O_K;

    if(jwin_alert("ZQuest","Really want to quit?", NULL, NULL, "&Yes", "&No", 'y', 'n', lfont) == 2)
        return D_O_K;

    return D_CLOSE;
}

int32_t onAbout()
{
    char buf1[80]={0};
	
    if(get_debug())
    {
		char buf2[80]={0};
		char buf3[80]={0};
#if V_ZC_ALPHA
        {
            sprintf(buf1,"ZQuest %s Alpha Build %d - DEBUG",ZQ_EDITOR_V, VERSION_BUILD);
        }
#elif V_ZC_BETA
        {
            sprintf(buf1,"ZQuest %s Beta Build %d - DEBUG",ZQ_EDITOR_V, VERSION_BUILD);
        }
#elif V_ZC_GAMMA
        {
            sprintf(buf1,"ZQuest %s Gamma Build %d - DEBUG",ZQ_EDITOR_V, VERSION_BUILD);
        }
#else
        {
            sprintf(buf1,"ZQuest %s Build %d - DEBUG",ZQ_EDITOR_V, VERSION_BUILD);
        }
#endif
        sprintf(buf2,"Tag: %s", getReleaseTag());
        sprintf(buf3,"This qst file: %04X",header.internal&0xFFFF);
        InfoDialog("About ZQuest", { buf1, buf2, buf3 }).show();
    }
    else
    {
        AboutDialog("About ZQuest", generate_zq_about()).show();
    }

    return D_O_K;
}

int32_t onShowWalkability()
{
    Flags^=cWALK;
    refresh(rMAP+rMENU);
    return D_O_K;
}

int32_t onPreviewMode()
{
	prv_mode=(prv_mode+1)%2;

	if(prv_mode)
	{
		Map.set_prvscr(Map.getCurrMap(),Map.getCurrScr());
		
		zoomed_minimap = false;
		
		bool tempcb=ComboBrush!=0;
		ComboBrush=0;
		restore_mouse();
		clear_tooltip();
		clear_tooltip2();
		dopreview();
		ComboBrush=tempcb;
	}
	return D_O_K;
}

int32_t onShowFlags()
{
    Flags^=cFLAGS;
    refresh(rMAP);
    return D_O_K;
}

int32_t onP()
{
    if(prv_mode)
    {
        Map.set_prvfreeze(((Map.get_prvfreeze()+1)%2));
    }

    return D_O_K;
}

int32_t onShowComboInfoCSet()
{
    if(Flags&cCSET)
    {
        Flags ^= cCSET;
        Flags |= cCTYPE;
    }
    else if(Flags&cCTYPE)
    {
        Flags ^= cCTYPE;
    }
    else
    {
        Flags |= cCSET;
    }

    refresh(rMAP);
    return D_O_K;
}

int32_t onShowCSet()
{
    Flags^=cCSET;
    Flags&=~cCTYPE;
    refresh(rMAP);
    return D_O_K;
}

int32_t onShowCType()
{
    Flags^=cCTYPE;
    Flags&=~cCSET;
    refresh(rMAP);
    return D_O_K;
}

extern MENU view_menu[];
int32_t onShowDarkness()
{
	if(get_bit(quest_rules,qr_NEW_DARKROOM))
	{
		Flags ^= cNEWDARK;
		refresh(rALL);
	}
	else
	{
		refresh(rALL);
		update_hw_screen(true);
		if(get_bit(quest_rules,qr_FADE))
		{
			int32_t last = CSET(5)-1;

			if(get_bit(quest_rules,qr_FADECS5))
				last += 16;

			byte *si = colordata + CSET(Color*pdLEVEL+poFADE1)*3;

			for(int32_t i=0; i<16; i++)
			{
				int32_t light = si[0]+si[1]+si[2];
				si+=3;
				fade_interpolate(RAMpal,black_palette,RAMpal,light?32:64,CSET(2)+i,CSET(2)+i);
			}

			fade_interpolate(RAMpal,black_palette,RAMpal,64,CSET(3),last);
			set_palette(RAMpal);

			readkey();

			load_cset(RAMpal,5,5);
			loadlvlpal(Color);
		}
		else
		{
			loadfadepal(Color*pdLEVEL+poFADE3);
			readkey();
			loadlvlpal(Color);
		}
	}

	return D_O_K;
}

int32_t onM()
{
    return D_O_K;
}

int32_t onJ()
{
    return D_O_K;
}

int32_t theFlagColor = 0;
void setFlagColor()
{
	setFlagColor(Flag);
}
void setFlagColor(int32_t c)
{
	theFlagColor = c%16;
    RAMpal[dvc(0)]=RAMpal[vc(c%16)];
    set_palette_range(RAMpal,dvc(0),dvc(0),false);
}

int32_t onIncreaseFlag()
{
	do
	{
		Flag=(Flag+1);

		if(Flag==mfMAX)
		{
			Flag=0;
		}
	} while(!ZI.isUsableMapFlag(Flag));

    setFlagColor();
    refresh(rMENU);
    return D_O_K;
}

int32_t onDecreaseFlag()
{
	do
	{
		if(Flag==0)
		{
			Flag=mfMAX;
		}

		Flag=(Flag-1);
	} while(!ZI.isUsableMapFlag(Flag));
	
    setFlagColor();
    refresh(rMENU);
    return D_O_K;
}

int32_t on0();
int32_t on1();
int32_t on2();
int32_t on3();
int32_t on4();
int32_t on5();
int32_t on6();
int32_t on7();
int32_t on8();
int32_t on9();
int32_t on10();
int32_t on11();
int32_t on12();
int32_t on13();
int32_t on14();

int32_t onToggleDarkness();
int32_t onIncMap();
int32_t onDecMap();

int32_t onDumpScr();

// these are here so that copy_dialog won't choke when compiling zquest
int32_t d_jbutton_proc(int32_t, DIALOG*, int32_t)
{
    return D_O_K;
}

int32_t d_listen_proc(int32_t, DIALOG*, int32_t)
{
    return D_O_K;
}

int32_t d_savemidi_proc(int32_t, DIALOG*, int32_t)
{
    return D_O_K;
}

//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zdefs.h
//
//  Data formats, definitions, and a few small functions
//  for zelda.cc and zquest.cc
//
//--------------------------------------------------------

#ifndef _ZDEFS_H_
#define _ZDEFS_H_

#define DEVLEVEL 0
#define COLLECT_SCRIPT_ITEM_ZERO -32767

//DEVLEVEL 1 = extra debug tools
//DEVLEVEL 2 = force enable cheats
//DEVLEVEL 3 ?? (Seems to do nothing special)
//DEVLEVEL 4 = ignore quest passwords

#ifdef _DEBUG
#if DEVLEVEL < 4
#undef DEVLEVEL
#define DEVLEVEL 4
#endif
#endif

#if DEVLEVEL > 0
#define DEVLOGGING	dev_logging
#define DEVDEBUG	dev_debug
#define DEVTIMESTAMP dev_timestmp
#else
#define DEVLOGGING	 false
#define DEVDEBUG	 false
#define DEVTIMESTAMP false
#endif


#define NUMSCRIPTFFC			512
#define NUMSCRIPTFFCOLD			256
#define NUMSCRIPTITEM			256
#define NUMSCRIPTGUYS			256
#define NUMSCRIPTWEAPONS		256
#define NUMSCRIPTGLOBAL			8
#define NUMSCRIPTGLOBAL255OLD	7
#define NUMSCRIPTGLOBAL253		4
#define NUMSCRIPTGLOBALOLD		3
#define NUMSCRIPTHEROOLD		3
#define NUMSCRIPTPLAYER			5
#define NUMSCRIPTSCREEN			256
#define NUMSCRIPTSDMAP			256
#define NUMSCRIPTSITEMSPRITE	256
#define NUMSCRIPTSCOMBODATA		512
#define NUMSCRIPTSGENERIC       512

//Conditional Debugging Compilation
//Script related
#define _FFDEBUG
//#define _FFDISSASSEMBLY
//#define _FFONESCRIPTDISSASSEMBLY

//Other
//#define _SKIPPASSWORDCHECK

/*
  //DOS Graphics Modes
  GFX_TEXT
  GFX_AUTODETECT
  GFX_AUTODETECT_FULLSCREEN
  GFX_AUTODETECT_WINDOWED
  GFX_SAFE
  GFX_VGA
  GFX_MODEX
  GFX_VESA1
  GFX_VESA2B
  GFX_VESA2L
  GFX_VESA3
  GFX_VBEAF

  //Windows Graphics Modes
  GFX_TEXT
  GFX_AUTODETECT
  GFX_AUTODETECT_FULLSCREEN
  GFX_AUTODETECT_WINDOWED
  GFX_SAFE
  GFX_DIRECTX
  GFX_DIRECTX_ACCEL
  GFX_DIRECTX_SOFT
  GFX_DIRECTX_SAFE
  GFX_DIRECTX_WIN
  GFX_DIRECTX_OVL
  GFX_GDI

  //Linux Graphics Modes
  GFX_TEXT
  GFX_AUTODETECT
  GFX_AUTODETECT_FULLSCREEN
  GFX_AUTODETECT_WINDOWED
  GFX_SAFE
  GFX_FBCON
  GFX_VBEAF
  GFX_SVGALIB
  GFX_VGA
  GFX_MODEX

  //X-Window Graphics Modes
  GFX_TEXT
  GFX_AUTODETECT
  GFX_AUTODETECT_FULLSCREEN
  GFX_AUTODETECT_WINDOWED
  GFX_SAFE
  GFX_XWINDOWS
  GFX_XWINDOWS_FULLSCREEN
  GFX_XDGA2
  GFX_XDGA2_SOFT

  //MacOS X Drivers
  GFX_TEXT
  GFX_AUTODETECT
  GFX_AUTODETECT_FULLSCREEN
  GFX_AUTODETECT_WINDOWED
  GFX_SAFE
  GFX_QUARTZ_FULLSCREEN
  GFX_QUARTZ_WINDOW
  */

#include <cstdio>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <vector>
#include <set>
#include <assert.h>
#include <string>
#include "base/ints.h"

//Common struct array element sizes-Z
#define INITIAL_A 2
#define INITIAL_D 8
#define FFSCRIPT_MISC 32
// Max number of ffcs for a single screen.
#define MAXFFCS 128
#define MAX_FFCID (region_scr_count * MAXFFCS - 1)

#define MAX_SIGNED_32 (2147483647)
#define MIN_SIGNED_32 (-2147483647-1)
#define MAX_DWORD dword(-1)
#define MIN_DWORD 0

#include "metadata/metadata.h"
#include "base/zc_alleg.h"
#include "gamedata.h"
#include "base/zc_array.h"
#include "base/random.h"
#include "base/util.h"
#include "base/process_management.h"
#include "base/render.h"
#include "zconfig.h"
#include "user_object.h"
#include "base/mapscr.h"
#include "base/handles.h"
#include "dcheck.h"

#define ZELDA_VERSION       0x0255                         //version of the program
#define ZC_VERSION 25500 //Version ID for ZScript Game->Version
#define VERSION_BUILD       61                             //V_BUILD build number of this version
//31 == 2.53.0 , leaving 32-39 for bugfixes, and jumping to 40. 
//#define ZELDA_VERSION_STR   "AEternal (v2.55) Alpha 37"                    //version of the program as presented in text
//#define IS_BETA             -39                       //is this a beta? (1: beta, -1: alpha)
//#define VERSION_BETA        39	
//#define DATE_STR            "19th October, 2019, 12:18GMT"
//__DATE__ and __TIME__ macros can simplify this, in the future. 
//#define ZELDA_ABOUT_STR 	    "ZC Player 'AEternal', Alpha 37"
#define COPYRIGHT_YEAR      "2019"                          //shown on title screen and in ending

#define MIN_VERSION         0x0184

#define ZELDADAT_VERSION      0x0211                        //version of zelda.dat
#define ZELDADAT_BUILD        17                            //build of zelda.dat
#define SFXDAT_VERSION        0x0211                        //version of sfx.dat
#define SFXDAT_BUILD          15                            //build of sfx.dat
#define FONTSDAT_VERSION      0x0253                        //version of fonts.dat
#define FONTSDAT_BUILD        30                            //build of fonts.dat
#define QSTDAT_VERSION        0x0253                        //version of qst.dat
#define QSTDAT_BUILD          30                            //build of qst.dat
#define ZQUESTDAT_VERSION     0x0211                        //version of zquest.dat
#define ZQUESTDAT_BUILD       18                            //build of zquest.dat

#define MAX_INTERNAL_QUESTS 	5

#define BITS_SP	10
#define MAX_SCRIPT_REGISTERS (1<<BITS_SP)
#define MAX_SCRIPT_REGISTERS_250 256

#define DCHECK_RANGE_INCLUSIVE(x, low, high) DCHECK(x >= low && x <= high)
#define DCHECK_RANGE_EXCLUSIVE(x, low, high) DCHECK(x >= low && x < high)
#define DCHECK_LAYER_ZERO_INDEX(l) DCHECK(l >= 0 && l < 7)
#define DCHECK_LAYER_NEG1_INDEX(l) DCHECK(l >= -1 && l < 6)

enum {ENC_METHOD_192B104=0, ENC_METHOD_192B105, ENC_METHOD_192B185, ENC_METHOD_211B9, ENC_METHOD_211B18, ENC_METHOD_MAX};

//Moved these OS-dependent defs from 'ffasm.cpp', to be global.
#ifdef ALLEGRO_MACOSX
#define strnicmp strncasecmp
#endif

#ifdef ALLEGRO_MACOSX
#define strnicmp strncasecmp
#endif

#ifdef ALLEGRO_LINUX
#define strnicmp strncasecmp
#endif

#ifdef _MSC_VER
#define stricmp _stricmp
#define strnicmp _strnicmp
#define unlink _unlink
#endif

#ifdef _WIN32
#define PATH_SLASH '\\'
#define WRONG_PATH_SLASH '/'
#else
#define PATH_SLASH '/'
#define WRONG_PATH_SLASH '\\'
#endif

#define PI 3.14159265358979323846

//#define HP_PER_HEART          16 //We should make this a global quest setting.
//#define MAGICPERBLOCK         32
//#define DAMAGE_MULTIPLIER     2 //We should make this a global quest setting.

#define ZC_ID(a,b,c,d)  (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))

//ID values of the different section types
#define ID_HEADER         ZC_ID('H','D','R',' ')              //quest header
#define ID_RULES          ZC_ID('R','U','L','E')              //quest rules
#define ID_STRINGS        ZC_ID('S','T','R',' ')              //message strings
#define ID_MISC           ZC_ID('M','I','S','C')              //misc data
#define ID_TILES          ZC_ID('T','I','L','E')              //tiles
#define ID_COMBOS         ZC_ID('C','M','B','O')              //combos
#define ID_CSETS          ZC_ID('C','S','E','T')              //csets (and pal names?)
#define ID_MAPS           ZC_ID('M','A','P',' ')              //maps
#define ID_DMAPS          ZC_ID('D','M','A','P')              //dmaps
#define ID_DOORS          ZC_ID('D','O','O','R')              //door combo sets
#define ID_ITEMS          ZC_ID('I','T','E','M')              //items
#define ID_WEAPONS        ZC_ID('W','P','N',' ')              //weapons
#define ID_COLORS         ZC_ID('M','C','L','R')              //misc. colors
#define ID_ICONS          ZC_ID('I','C','O','N')              //save game icons
#define ID_GRAPHICSPACK   ZC_ID('G','P','A','K')              //graphics pack header
#define ID_INITDATA       ZC_ID('I','N','I','T')              //initialization data
#define ID_GUYS           ZC_ID('G','U','Y',' ')              //guys
#define ID_MIDIS          ZC_ID('M','I','D','I')              //midis
#define ID_CHEATS         ZC_ID('C','H','T',' ')              //cheats
#define ID_SAVEGAME       ZC_ID('S','V','G','M')              //save game data (used in the save game file)
#define ID_COMBOALIASES   ZC_ID('C','M','B','A')              //combo alias
#define ID_HEROSPRITES    ZC_ID('L','I','N','K')              //Hero sprites
#define ID_SUBSCREEN      ZC_ID('S','U','B','S')              //subscreen data
#define ID_ITEMDROPSETS   ZC_ID('D','R','O','P')              //item drop set tables
#define ID_FAVORITES      ZC_ID('F','A','V','S')              //favorite combos and combo aliases
#define ID_FFSCRIPT       ZC_ID('F','F','S','C')              //ff scripts data
#define ID_SFX            ZC_ID('S','F','X',' ')              //sfx data
#define ID_ZINFO          ZC_ID('Z','I','N','F')              //ZInfo data

//Version number of the different section types
#define V_HEADER           8
#define V_RULES           17
#define V_STRINGS         10
#define V_MISC            15
#define V_TILES            3 //2 is a int32_t, max 214500 tiles (ZScript upper limit)
#define V_COMBOS          41
#define V_CSETS            5 //palette data
#define V_MAPS            25
#define V_DMAPS            17
#define V_DOORS            1
#define V_ITEMS           57
#define V_WEAPONS          8
#define V_COLORS           4 //Misc Colours
#define V_ICONS            10 //Game Icons
#define V_GRAPHICSPACK     1
#define V_INITDATA        34
#define V_GUYS            47
#define V_MIDIS            4
#define V_CHEATS           1
#define V_SAVEGAME        32
#define V_COMBOALIASES     4
#define V_HEROSPRITES      16
#define V_SUBSCREEN        7
#define V_ITEMDROPSETS     2
#define V_FFSCRIPT         21
#define V_SFX              8
#define V_FAVORITES        3

#define V_COMPATRULE       46
#define V_ZINFO            3

//= V_SHOPS is under V_MISC

/*
  * Compatible version number of the different section types
  * Basically, the last version number that this section type
  * is just an extension of (ie. new variables are stuck on the end)
  * instead of a complete rewrite (or variables added to the middle).
  * If this and the version number are the same, then this is
  * a total reworking of the section and probably won't be able
  * to be read by anything that was written for a previous version.
  */
#define CV_HEADER          3
#define CV_RULES           1
#define CV_STRINGS         2
#define CV_MISC            7
#define CV_TILES           1
#define CV_COMBOS          1
#define CV_CSETS           1
#define CV_MAPS            9
#define CV_DMAPS           1
#define CV_DOORS           1
#define CV_ITEMS          15
#define CV_WEAPONS         1
#define CV_COLORS          1
#define CV_ICONS           1
#define CV_GRAPHICSPACK    1
#define CV_INITDATA       15
#define CV_GUYS            4
#define CV_MIDIS           3
#define CV_CHEATS          1
#define CV_SAVEGAME        5
#define CV_COMBOALIASES    1
#define CV_HEROSPRITES     1
#define CV_SUBSCREEN       3
#define CV_ITEMDROPSETS    1
#define CV_FFSCRIPT        1
#define CV_SFX             5
#define CV_FAVORITES       1
#define CV_ZINFO           0


// Loose Object Version Metadata
// If the version is 0, it is ther original.
// 'Higher Versions' use negative numbers, so -1 is newer than 0, -2 is newer than -1.
// Sorry for the ugly hack, but it maintains full compatibility through
// Packfile magic. -Z
#define V_ZCOMBO -1
#define V_ZMOD -1
#define V_ZTHEME -1
#define V_ZASM -1
#define V_ZTILE -1
#define V_ZTILESET -1
#define V_ZDMAP -1
#define V_ZALIAS -1
#define V_ZNPC -1
#define V_ZITEM -1
#define V_ZWPNSPR -1

void zprint(const char * const format,...);
void zprint2(const char * const format,...);

extern int32_t curr_tb_page;
extern int32_t original_playing_field_offset;
extern int32_t playing_field_offset;
extern int32_t passive_subscreen_height;
extern int32_t passive_subscreen_offset;

extern int32_t CSET_SIZE;
extern int32_t CSET_SHFT;

extern int32_t readsize, writesize;
extern bool fake_pack_writing;

extern volatile bool close_button_quit;

// system colors
#define lc1(x) ((x)+192)                                    // offset to 'level bg color' x (row 12)
#define lc2(x) ((x)+208)                                    // offset to 'level fg color' x (row 13)
#define vc(x)  ((x)+224)                                    // offset to 'VGA color' x (row 14)
#define dvc(x) ((x)+240)                                    // offset to dark 'VGA color' x (row 15)
#define r_lc1(x) ((x)-192)
#define r_lc2(x) ((x)-208)
#define r_vc(x)  ((x)-224)
#define r_dvc(x) ((x)-240)
#define BLACK         253
#define WHITE         254

#define LARGE_W       912
#define LARGE_H       684

#define BYTE_FILTER 0xFF
#define DIAG_TO_SIDE		0.7071
#define SIDE_TO_DIAG		1.4142
#define STEP_DIAGONAL(s)	(s*DIAG_TO_SIDE)

#define SINGLE_TILE_SIZE    128
#define TILES_PER_ROW       20
#define TILE_ROWS_PER_PAGE  13
#define TILES_PER_PAGE      (TILES_PER_ROW*TILE_ROWS_PER_PAGE)

#define TILEROW(tile)		((tile)/TILES_PER_ROW)
#define TILECOL(tile)		((tile)%TILES_PER_ROW)
#define TILEPAGE(tile)		((tile)/TILES_PER_PAGE)

#define TILE_PAGES          825
#define TILE_PAGES_ZC250    252 //2.50.x

#define OLDMAXTILES         (TILES_PER_PAGE*6)              // 1560 tiles
#define NEWMAXTILES         (TILES_PER_PAGE*TILE_PAGES)     // 214500 tiles
#define ZC250MAXTILES         (TILES_PER_PAGE*TILE_PAGES_ZC250)     // 32760 tiles

#define MAXTILEROWS         (TILE_ROWS_PER_PAGE*TILE_PAGES) //Last row that we can show when trying to grab tiles from  .QST file. -Z

#define NEWTILE_SIZE2       (NEWMAXTILES*SINGLE_TILE_SIZE)  // 27456000 bytes 
#define ZC250TILESIZE       (ZC250MAXTILES*SINGLE_TILE_SIZE)  // 4193280 bytes (new packed format, 6 pages)
#define OLDTILE_SIZE2       (OLDMAXTILES*SINGLE_TILE_SIZE)  // 199680 bytes (new packed format, 6 pages)
// 133120 bytes (new packed format, 4 pages)
#define OLDTILE_SIZE        (TILES_PER_PAGE*4*SINGLE_TILE_SIZE)
//#define NEWTILE_SIZE      (260*6*128) // 199680 bytes (new packed format, 6 pages)
//#define OLDTILE_SIZE      (260*4*128) // 133120 bytes (new packed format, 4 pages)
#define TILEBUF_SIZE        (320*480)                       // 153600 bytes (old unpacked format)

#define COMBOS_PER_ROW      20

//TODO Turn this into an init data var, and allow editing it per-weapon! -Em
#define DEFAULT_FIRE_LIGHT_RADIUS 40

#define WRAP_CS2(cs,cs2) (get_bit(quest_rules,qr_OLDCS2)?((cs+cs2+16)%16):((cs+cs2+14)%14))

#define XOR(a,b) (!(a) != !(b))
#define CLEAR_LOW_BITS(x, b) ((x) & ~((1<<(b)) - 1))
#define TRUNCATE_TILE(x) CLEAR_LOW_BITS(x, 4)
#define TRUNCATE_HALF_TILE(x) CLEAR_LOW_BITS(x, 3)

//#define MAGICDRAINRATE  2

// quest stuff
#define ZQ_TILES        0
#define ZQ_MIDIS2       1                                   //4 bytes
#define ZQ_CHEATS2       5
#define ZQ_MAXDATA      20
#define WAV_COUNT       256

#define MAXSCREENS 128
#define MAXCUSTOMMIDIS192b177 32                                  // uses bit string for midi flags, so 32 bytes
#define MAXCUSTOMMIDIS        252                                 // uses bit string for midi flags, so 32 bytes
#define MIDIFLAGS_SIZE  ((MAXCUSTOMMIDIS+7)>>3)
#define MAXCUSTOMTUNES        252
//Midi offsets
//The offset from dmap/mapscr-> midi/screen_midi to currmidi
#define MIDIOFFSET_DMAP		(ZC_MIDI_COUNT-4)
#define MIDIOFFSET_MAPSCR	(ZC_MIDI_COUNT-4)
//The offset from currmidi to ZScript MIDI values
#define MIDIOFFSET_ZSCRIPT	(ZC_MIDI_COUNT-1)
//Use together as `(MIDIOFFSET_DMAP-MIDIOFFSET_ZSCRIPT)` to go from `dmap` directly to `zscript`


#define MAXMUSIC              256                                 // uses bit string for music flags, so 32 bytes
#define MUSICFLAGS_SIZE       MAXMUSIC>>3

#define MAXMAPS2        255                                 // 4 times the old number
//#define MAXMAPS         16
#define MAPSCRSNORMAL   128
#define MAPSCRS192b136  132
#define MAPSCRS         136
#define TEMPLATES         8
#define TEMPLATE        131
#define TEMPLATE2       132

#define MAXQTS           256
#define MAXMSGS          65535
#define MAXDOORCOMBOSETS 256
#define MAXDMAPS         512                                 //this and
#define MAXLEVELS        512								 //this should be the same number (was 32)
#define OLDMAXLEVELS	 256
#define OLDMAXDMAPS		 256
#define MAXITEMS         256
#define MAXWPNS          256
#define OLDBETAMAXGUYS   256								//max 2.5 guys through beta 20
#define MAXGUYS          512
#define MAXITEMDROPSETS  256
#define COMBOS_PER_PAGE  256
#define COMBO_PAGES      255
#define MAXCOMBOS        COMBO_PAGES*COMBOS_PER_PAGE
#define BOUND_COMBO(c)	vbound(c, 0, MAXCOMBOS)
#define MAXSUBSCREENITEMS	256
#define MAXCUSTOMSUBSCREENS 128
#define MAXNPCS	512

#define MAXFAVORITECOMMANDS 64
#define MAXFAVORITECOMBOS 300
#define MAXFAVORITECOMBOALIASES MAXFAVORITECOMBOS

#define FAVORITECOMBO_PER_ROW 30

#define PALNAMESIZE     17
// mapscr "valid" byte
#define mVALID          0x01
#define mVERSION        0x80

// lvlitems flags
#define liTRIFORCE      1
#define liMAP           2
#define liCOMPASS       4
#define liBOSS          8
#define liBOSSKEY       16


// door codes        meaning: type | shows on subscreen map
//                       bit: 4321    0
#define dWALL           0                                   // 0000    0
#define dOPEN           1                                   // 0000    1
#define dLOCKED         2                                   // 0001    0
#define dUNLOCKED       3                                   // 0001    1
#define dSHUTTER        4                                   // 0010    0
#define dBOMB           6                                   // 0011    0
#define dBOMBED         7                                   // 0011    1
#define dWALK           8                                   // 0100    0

#define dBOSS           10                                  // 0101    0
#define dOPENBOSS       11                                  // 0101    1

#define dOPENSHUTTER    12                                  // 0110    0    // special case (don't show on map)
#define d1WAYSHUTTER    14                                  // 0111    0    // never opens

// screen flags
#define fSHUTTERS       1
#define fITEM           2 // Enemies->Item
#define fDARK           4 //'S.Flags1' Dark Room
#define fROAR           8 // DEFUNCT
#define fWHISTLE        16 //'
#define fLADDER         32 //'S.Flags2' Toggle  'Allow Ladder' --??? Is that right?
#define fMAZE           64 //'S.Flags 1' Use Maze path
#define fSEA            128 // DEFUNCT

// flags2
#define wfUP            1
#define wfDOWN          2
#define wfLEFT          4
#define wfRIGHT         8
#define fSECRET         16  // play "secret" sfx upon entering this screen
#define fAIRCOMBOS      32  //'S.Flags2' Combos affect midair Hero
#define fFLOATTRAPS     64 //'E.Flags' Traps ignore walkability. 
#define fCLEARSECRET    128 // clear all enemies to trigger secret entrance (Enemies->Secret)

// flags3
#define fHOLDITEM         1 //'S.Flags1' Hold Up Item
#define fCYCLEONINIT      2 //'S.Flags2' Cycle combos on screen init
#define fINVISROOM        4 //
#define fINVISHERO        8 //'S.Flags1' Invisible Hero
#define fNOSUBSCR         16 //'S.Flags1' No subscreen
#define fIWARPFULLSCREEN  32                                // instawarps affect all sprites --is this 'Sprites carry over in warps???' -Z
#define fNOSUBSCROFFSET   64  //'S.Flags1' ...but don't offset screen     // don't offset the screen when fNOSUBSCR is true
#define fENEMIESRETURN    128 //'E.Flags' Enemies always return                             // enemies always return

//flags4
#define fOVERHEADTEXT     1
#define fITEMWARP         2
#define fTIMEDDIRECT      4    //'T.Warp' Timed Warps are Direct                             //Specifies timed pit warp
#define fDISABLETIME      8    //'T.Warp' Secrets Disable timed warp ??
#define fENEMYSCRTPERM    16   //'E.Flags' Enemies->Secret is Permanent
#define fNOITEMRESET      32
#define fSAVEROOM		  64  //'S.Flags2' Save point->Continue here 
#define fAUTOSAVE		  128 //'S.Flags2' Save game on entry

//flags5
#define fRANDOMTIMEDWARP  1 //'T.Warp' Timed warp is random
#define fDAMAGEWITHBOOTS  2
#define fDIRECTAWARP	  4 //'S.Flags1' Auto warps are direct
#define fDIRECTSWARP	  8 //'S.Flags1' Sensitive warps are direct
#define fTEMPSECRETS	  16 //'S.Flags1' Secrets are temporary
// what is going on with 32?
#define fTOGGLEDIVING   64 //'S.Flags2' Toggle no Diving
#define fNOFFCARRYOVER  128 //'S.Flags2' No FFC Carryover

//flags6
#define fCAVEROOM	  1 //'S.Flags1' Treat as Interior screen.
#define fDUNGEONROOM	  2 //'S.Flags1' Treat as NES dungeon room
#define fTRIGGERFPERM     4 //'S.Flags1' Hit all triggers->Perm Secret
#define fCONTINUEHERE	  8 //'S.Flags 2' Continue Here
#define fNOCONTINUEHERE   16 //'S.Flags2' No continue here after warp
#define fTRIGGERF1631	  32 //'S.Flags1' Hit all triggers 16-31
#define fTOGGLERINGDAMAGE 64 //'S.Flags2' Toggle Rings Affect Combos
#define fWRAPAROUNDFF	  128 //'S.Flags2' FF Combos Wrap Around

//flags7
#define fLAYER3BG       1 //'S.Flags1' Layer 3 is background
#define fLAYER2BG       2 //'S.Flags1' Layer 2 is background
#define fITEMFALLS	4 //'S.Flags1' Item falls from ceiling
#define fSIDEVIEW       8 //'S.Flags1' Sideview Gravity
#define fNOHEROMARK     16 //'S.Flags1' No Hero marker in Minimap
#define fSPECITEMMARK   32
#define fWHISTLEPAL     64 //'S.Flags2' Whistle->Palette change. 
#define fWHISTLEWATER   128 //'S.Flags2' Whistle->Dry lake

//flags8
// 1 - 16 are General Purpose (Scripts)
#define fGENERALSCRIPT1 	0x01 //'S.Flags2' General Use 1 (Scripts)
#define fGENERALSCRIPT2 	0x02 //'S.Flags2' General Use 2 (Scripts)
#define fGENERALSCRIPT3 	0x04 //'S.Flags2' General Use 3 (Scripts)
#define fGENERALSCRIPT4 	0x08 //'S.Flags2' General Use 4 (Scripts)
#define fGENERALSCRIPT5 	0x10 //'S.Flags2' General Use 5 (Scripts)
#define fMAZEvSIDEWARP 		0x20 //'S.Flags1' Maze overrides side warps
#define fSECRETITEM 		0x40 //'S.Flags3' Secrets->Item
#define fITEMSECRET 		0x80 //'S.Flags3' Item->Secret

//flags9
#define fITEMSECRETPERM 	0x01 //'S.Flags3' Item->Secret is Permanent
#define fITEMRETURN	 	0x02 //'S.Flags3' Item always returns
#define fBELOWRETURN	 	0x04 //'S.Flags3' Special Item always returns
#define fDARK_DITHER        0x08 //'S.Flags1' ...dithered dark
#define fDARK_TRANS         0x10 //'S.Flags1' ...transparent dark
#define fDISABLE_MIRROR     0x20 //'S.Flags1' Disable Magic Mirror
#define fENEMY_WAVES     0x40 //'E.Flags' Chain 'Enemies->' triggers

//flags10 - ENTIRE FLAGS10 RESERVED FOR Z3 SCROLLING! Please don't use :)
#define fZ3_SCROLLING_WHEN  0x01
// ----

//lens layer effects
#define llNORMAL        0
#define llLENSHIDES     8
#define llLENSSHOWS     16

// sprite drawing flag bits
#define sprdrawflagALWAYSOLDDRAWS 1

// enemy flags
#define efZORA          1
#define efTRAP4         2
#define efTRAP2         4
#define efROCKS         8
#define efFIREBALLS     16
#define efLEADER        32
#define efCARRYITEM     64
#define efBOSS          128

// item "pick up" flags
#define ipBIGRANGE      1                                   // Collision rectangle is large
#define ipHOLDUP        2                                   // Hero holds up item when he gets it
#define ipONETIME       4                                   // Getting this item sets mITEM
#define ipDUMMY         8                                   // Dummy item.  Can't get this.
#define ipCHECK         16                                  // Check restrictions (money in a shop, etc.)
#define ipMONEY         32                                  // This item causes money loss (bomb upgrade, swindle room, etc.)
#define ipFADE          64                                  // Blinks if fadeclk>0
#define ipENEMY         128                                 // Enemy is carrying it around
#define ipTIMER         256                                 // Disappears after a while
#define ipBIGTRI        512                                 // Large collision rectangle (used for large triforce)
#define ipNODRAW        1024                                // Don't draw this (for underwater items)
#define ipONETIME2      2048                                // Getting this item sets mSPECIALITEM
#define ipSECRETS       4096                                // Trigger Secrets when picked up
#define ipCANGRAB       8192                                // Always grabbable to hookshot/arrows/brang

//We have definitions for this below, but this is for scripted stuff in 2,54. 
enum { warpfxNONE, warpfxBLACKOUT, warpfxWIPE, warpfxSCROLL, warpfxZAP, warpfxZAPIN, warpfxZAPOUT, warpfxWAVY, 
	warpfxWAVYIN, warpfxWAVYOUT, warpfxWAVYNOHERO, warpfxWAVYINNOHERO, warpfxWAVYOUTNOHERO, 
		warpfxLAST};
	
//wipe types - mosaic should be one of them. 
enum { wipeNONE, wipeCOOL, wipeBLACK, wipeBSZELDA, wipeTRIANGLE, wipeCIRCLE, wipeALLSTARS, wipeLAST };

enum
{
    wtCAVE, wtPASS, wtEXIT, wtSCROLL, wtIWARP, wtIWARPBLK, wtIWARPOPEN,
    wtIWARPZAP, wtIWARPWAVE, wtNOWARP, wtWHISTLE, wtMAX
};

enum
{
    weINSTANT, weCIRCLE, weOVAL, weTRIANGLE, weSMAS, weBLINDS_SMOOTH, weBLINDS_STEPPED, weMOSAIC, weWAVE_WHITE, weWAVE_BLACK, weFADE_WHITE, weFADE_BLACK, weDEFAULT_OC, weDEST_DEFAULT, weMAX
};
// sprite palettes
enum
{
    spAQUA, spGLEEOK, spDIG, spGANON, spBROWN, spPILE, spBLUE, spRED,
    spGOLD, spICON1, spICON2, spICON3, spICON4, spGLEEOKF, spFROZEN
};

// dmap types
enum { dmDNGN, dmOVERW, dmCAVE, dmBSOVERW, dmMAX };

// dmap type bit masks (?)
#define dmfCONTINUE      128
#define dmfTYPE          127

// map flags
enum
{	//These are the COMBO flags. -Z
	//0
	mfNONE, 	mfPUSHUD, 	mfPUSH4, 	mfWHISTLE, 	mfANYFIRE, 
	//5
	mfARROW, 	mfBOMB, 	mfFAIRY, 	mfRAFT, 	mfARMOS_SECRET, 
	//10
	mfARMOS_ITEM, 	mfSBOMB, 	mfRAFT_BRANCH, 	mfDIVE_ITEM,	mfLENSMARKER, 
	//15
	mfZELDA,	mfSECRETS01, 	mfSECRETS02, 	mfSECRETS03, 	mfSECRETS04,       
	//20
	mfSECRETS05, 	mfSECRETS06, 	mfSECRETS07, 	mfSECRETS08,	mfSECRETS09, 
	//25
	mfSECRETS10, 	mfSECRETS11, 	mfSECRETS12,	mfSECRETS13, 	mfSECRETS14, 
	//30
	mfSECRETS15, 	mfSECRETS16,	mfTRAP_H, 	mfTRAP_V, 	mfTRAP_4, 
	//35
	mfTRAP_LR, 	mfTRAP_UD,	mfENEMY0, 	mfENEMY1, 	mfENEMY2, 
	//40
	mfENEMY3, 	mfENEMY4,	mfENEMY5, 	mfENEMY6, 	mfENEMY7, 
	//45
	mfENEMY8, 	mfENEMY9,	mfPUSHLR, 	mfPUSHU, 	mfPUSHD, 
	//50
	mfPUSHL, 	mfPUSHR,	mfPUSHUDNS, 	mfPUSHLRNS, 	mfPUSH4NS, 
	//55
	mfPUSHUNS,	mfPUSHDNS, 	mfPUSHLNS, 	mfPUSHRNS, 	mfPUSHUDINS,
	//60
	mfPUSHLRINS, 	mfPUSH4INS, 	mfPUSHUINS,	mfPUSHDINS, 	mfPUSHLINS, 
	//65
	mfPUSHRINS,	mfBLOCKTRIGGER,	mfNOBLOCKS, 	mfBRANG, 	mfMBRANG,
	//70
	mfFBRANG, 	mfSARROW, 	mfGARROW, 	mfSTRONGFIRE, 	mfMAGICFIRE, 
	//75
	mfDIVINEFIRE,	mfWANDMAGIC, 	mfREFMAGIC, 	mfREFFIREBALL, 	mfSWORD, 
	//80
	mfWSWORD,	mfMSWORD, 	mfXSWORD, 	mfSWORDBEAM, 	mfWSWORDBEAM, 
	//85
	mfMSWORDBEAM,	mfXSWORDBEAM, 	mfHOOKSHOT, 	mfWAND, 	mfHAMMER, 
	//90
	mfSTRIKE, 	mfBLOCKHOLE,	mfMAGICFAIRY, 	mfALLFAIRY, 	mfSINGLE, 
	//95
	mfSINGLE16,	mfNOENEMY, 	mfNOGROUNDENEMY, mfSCRIPT1, 	mfSCRIPT2, 
	//100
	mfSCRIPT3, 	mfSCRIPT4, 	mfSCRIPT5,	mfRAFT_BOUNCE, 	mfPUSHED,
	//105
	mfSCRIPT6, 	mfSCRIPT7, 	mfSCRIPT8, 	mfSCRIPT9, 	mfSCRIPT10, 
	//110
	mfSCRIPT11, 	mfSCRIPT12, 	mfSCRIPT13, 	mfSCRIPT14, 	mfSCRIPT15,
	//115
	mfSCRIPT16, 	mfSCRIPT17, 	mfSCRIPT18, 	mfSCRIPT19, 	mfSCRIPT20,
	//120
	mfPITHOLE, 	mfPITFALLFLOOR,	mfLAVA, 	mfICE, 		mfICEDAMAGE, 
	//125
	mfDAMAGE1, 	mfDAMAGE2, 	mfDAMAGE4, 	mfDAMAGE8, 	mfDAMAGE16, 
	//130
	mfDAMAGE32,	mfFREEZEALL, 	mfFREZEALLANSFFCS, mfFREEZEFFCSOLY, mfSCRITPTW1TRIG, 
	//135
	mfSCRITPTW2TRIG, mfSCRITPTW3TRIG, mfSCRITPTW4TRIG, mfSCRITPTW5TRIG, mfSCRITPTW6TRIG, 
	//140
	mfSCRITPTW7TRIG, mfSCRITPTW8TRIG, mfSCRITPTW9TRIG, mfSCRITPTW10TRIG, mfTROWEL, 
	//145
	mfTROWELNEXT, mfTROWELSPECIALITEM,mfSLASHPOT, 	mfLIFTPOT,	mfLIFTORSLASH, 
	//150
	mfLIFTROCK, 	mfLIFTROCKHEAVY, mfDROPITEM, 	mfSPECIALITEM, 	mfDROPKEY, 
	//155
	mfDROPLKEY, 	mfDROPCOMPASS, 	mfDROPMAP, 	mfDROPBOSSKEY, mfSPAWNNPC, 
	//160
	mfSWITCHHOOK, 	mfSIDEVIEWLADDER, mfSIDEVIEWPLATFORM, mfNOENEMYSPAWN, mfENEMYALL,
	//165
	mfSECRETSNEXT, mfNOMIRROR, mfUNSAFEGROUND, mf168, mf169, mf170, mf171, mf172, mf173, mf174,
    mf175, mf176, mf177, mf178, mf179, mf180, mf181, mf182, mf183, mf184, mf185, mf186, mf187, 
    mf188, mf189, mf190, mf191, mf192, mf193, mf194, mf195, mf196, mf197, mf198, mf199, mf200,
    mf201, mf202, mf203, mf204, mf205, mf206, mf207, mf208, mf209, mf210, mf211, mf212, mf213,
    mf214, mf215, mf216, mf217, mf218, mf219, mf220, mf221, mf222, mf223, mf224, mf225, mf226, 
    mf227, mf228, mf229, mf230, mf231, mf232, mf233, mf234, mf235, mf236, mf237, mf238, mf239,
    mf240, mf241, mf242, mf243, mf244, mf245, mf246, mf247, mf248, mf249, mf250, mf251, mf252, mf253, mf254, 
    mfEXTENDED, mfMAX //256 for array sizing
    
    //mfSLASHPOTSPECIALITEM, //stack flags to make a drop or special item
     //mfLIFTPOTSPECIAL, 
};

// combo types
//combo name strings come from defdata
enum
{
    cNONE, cSTAIR, cCAVE, cWATER, cARMOS, 
	//5
	cGRAVE, cDOCK, cUNDEF, cPUSH_WAIT, cPUSH_HEAVY, 
	//10
	cPUSH_HW, cL_STATUE, cR_STATUE, cWALKSLOW, cCVUP, 
	//15
	cCVDOWN, cCVLEFT, cCVRIGHT, cSWIMWARP, cDIVEWARP,
	//20
	cLADDERHOOKSHOT, cTRIGNOFLAG, cTRIGFLAG, cZELDA, cSLASH, 
	//25
	cSLASHITEM, cPUSH_HEAVY2, cPUSH_HW2, cPOUND, cHSGRAB, 
	//30
	cHSBRIDGE, cDAMAGE1, cDAMAGE2, cDAMAGE3, cDAMAGE4, 
	//35
	cC_STATUE, cTRAP_H, cTRAP_V, cTRAP_4, cTRAP_LR, 
	//40
	cTRAP_UD, cPIT, cHOOKSHOTONLY, cOVERHEAD, cNOFLYZONE, 
	//45
	cMIRROR, cMIRRORSLASH, cMIRRORBACKSLASH, cMAGICPRISM, cMAGICPRISM4,
	//50
	cMAGICSPONGE, cCAVE2, cEYEBALL_A, cEYEBALL_B, cNOJUMPZONE, 
	//55
	cBUSH, cFLOWERS, cTALLGRASS, cSHALLOWWATER, cLOCKBLOCK, 
	//60
	cLOCKBLOCK2, cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2, cLADDERONLY, cBSGRAVE,
	//65
	cCHEST, cCHEST2, cLOCKEDCHEST, cLOCKEDCHEST2, cBOSSCHEST, 
	//70
	cBOSSCHEST2, cRESET, cSAVE, cSAVE2, /*cVERTICAL,*/ cCAVEB, 
	//75
	cCAVEC, cCAVED, cSTAIRB, cSTAIRC, cSTAIRD, 
	//80
	cPITB, cPITC, cPITD, cCAVE2B, cCAVE2C, 
	//85
	cCAVE2D, cSWIMWARPB, cSWIMWARPC, cSWIMWARPD, cDIVEWARPB, 
	//90
	cDIVEWARPC, cDIVEWARPD, cSTAIRR, cPITR, cAWARPA, 
	//95
	cAWARPB, cAWARPC, cAWARPD, cAWARPR, cSWARPA, 
	//100
	cSWARPB, cSWARPC, cSWARPD, cSWARPR, cSTRIGNOFLAG, 
	//105
	cSTRIGFLAG, cSTEP, cSTEPSAME, cSTEPALL, cSTEPCOPY, 
	//110
	cNOENEMY, cBLOCKARROW1, cBLOCKARROW2, cBLOCKARROW3, cBLOCKBRANG1, 
	//115
	cBLOCKBRANG2, cBLOCKBRANG3, cBLOCKSBEAM, cBLOCKALL, cBLOCKFIREBALL, 
	//120
	cDAMAGE5, cDAMAGE6, cDAMAGE7, cCHANGE/**DEPRECATED**/, cSPINTILE1, 
	//125
	cSPINTILE2, cSCREENFREEZE, cSCREENFREEZEFF, cNOGROUNDENEMY, cSLASHNEXT, 
	//130
	cSLASHNEXTITEM, cBUSHNEXT, cSLASHTOUCHY, cSLASHITEMTOUCHY, cBUSHTOUCHY, 
	//135
	cFLOWERSTOUCHY, cTALLGRASSTOUCHY, cSLASHNEXTTOUCHY, cSLASHNEXTITEMTOUCHY, cBUSHNEXTTOUCHY, 
	//140
	cEYEBALL_4, cTALLGRASSNEXT, cSCRIPT1, cSCRIPT2, cSCRIPT3, 
	//145
	cSCRIPT4, cSCRIPT5, cSCRIPT6, cSCRIPT7, cSCRIPT8, 
	//150
	cSCRIPT9, cSCRIPT10, cSCRIPT11, cSCRIPT12, cSCRIPT13, 
	//155
	cSCRIPT14, cSCRIPT15, cSCRIPT16, cSCRIPT17, cSCRIPT18, 
	//160
	cSCRIPT19, cSCRIPT20, cTRIGGERGENERIC, cPITFALL, cSTEPSFX,
	//165
	cBRIDGE, cSIGNPOST, cCSWITCH, cCSWITCHBLOCK, cTORCH,
	//170
	cSPOTLIGHT, cGLASS, cLIGHTTARGET, cSWITCHHOOK, cBUTTONPROMPT,
	//175
	cCUSTOMBLOCK, cSHOOTER, cSLOPE, cCUTSCENETRIG, cPUSHBLOCK,
	//180
	cICY,
    cMAX,
	// ! potential new stuff that I might decide it is worth adding. 
    //Five additional user script types, 
    
    //165
    cDIG, cDIGNEXT, cDIGITEM, //Dig a hole, dig a hole.
    
      /* pot, or rock:
		lift and slash combos need a 'break' sprite set in the combo editor. 
		The lifted object also needs a 'damage enemies' value, so that when thrown
		it makes an lweapon. The leweapon type, and damage should be inputs
		in the combo editor. -1 to damage means enemies ignore it. 
		-1 to weapon means that itdoesn't generate a weapon.
	
		The default type should be LW_POT, which we need to add, or LW_ROCK. 
		We should add both of these. 
		
		These should also include a break sound, a throw sound, and possibly other
		attributes such as throw distance. Sideview behaviour must be very carefully 
		defined. -Z
		
	*/
    //168
    //normal (use undercombo), lift and generate drop item, lift and generate special item (screen item)
    cLIFT, cLIFTITEM, cLIFTSPECITER, 
    //171
    //lift and show next combo, same plus item drop, same with room item
    cLIFTNEXT, cLIFTNEXTITEM, cLIFTNEXTSPECITEM,
    //174
    //lift or slash (do not change combo; use undercombo), ...and make drop item, ...and make room item
    cLIFTSLASH, cLIFTSLAHITEM, cLIFTSLASHSPECITEM, 
    //177
    //lift or shash, then change the combo at its position to the 'next combo', ...
	//...and make drop, ...and make screen room item
    cLIFTSLASHNEXT, cLIFTSLASHNEXTITEM, cLIFTSLASHNEXTSPECITEM, 
    //180

    cBREAKAWAYFLOOR, //as combo cycling to pit, but it makes a sound? Might be useful? 
    //181
    cFREEZEFFCONLY, //freezes only scripts
    //182
    // cSYS183, cSYS184, cSYS185, cSYS186, cSYS187, cSYS188, cSYS189, cSYS190,
    // cSYS191, cSYS192, cSYS193, cSYS194, cSYS195, cSYS196, cSYS197, cSYS198, cSYS199, cSYS200, 
    // cSYS201, cSYS202, cSYS203, cSYS204, cSYS205, cSYS206, cSYS207, cSYS208, cSYS209, cSYS210,
    // cSYS211, cSYS212, cSYS213, cSYS214, cSYS215, cSYS216, cSYS217, cSYS218, cSYS219, cSYS220,
    // cSYS221, cSYS222, cSYS223, cSYS224, cSYS225, cSYS226, cSYS227, cSYS228, cSYS229, cSYS230, 
    // cSYS231, cSYS232, cSYS233, cSYS234, cSYS235, cSYS236, cSYS237, cSYS238, cSYS239, cSYS240, 
    // cSYS241, cSYS242, cSYS243, cSYS244, cSYS245, cSYS246, cSYS247, cSYS248, cSYS249, cSYS250,
    // cSYS251, cSYS252, cSYS253, cSYS254,
    //Should be 255
    cEXPANDED=255, //Set to this, and then select an expansion[0] type for the 'oombo editor'.
    
    //2.54, these would need filepack changes. Needs V_COMBOS.
    cMAX250 //Old max, to make filepack handling easier. 
    
    //These need to be in a new, index of expansion[]. 
    //Let's use expansion[0] for now. 
    
    //cMAX
};

#define PITFALL_FALL_FRAMES 70
#define WATER_DROWN_FRAMES 64

//Combo editor and additional system combos for combo type 'expanded'.

//We could also change .type from a byte to a word, and typecast (byte)combo.type for old quests. 
//That would make 2.50 exporting harder, though. 
enum 
{
	cUSER000, cUSER001, cUSER002, cUSER003, cUSER004, cUSER005, cUSER006, cUSER007, cUSER008, cUSER009,
    cUSER010, cUSER011, cUSER012, cUSER013, cUSER014, cUSER015, cUSER016, cUSER017, cUSER018, cUSER019,
    cUSER020, cUSER021, cUSER022, cUSER023, cUSER024, cUSER025, cUSER026, cUSER027, cUSER028, cUSER029,
    cUSER030, cUSER031, cUSER032, cUSER033, cUSER034, cUSER035, cUSER036, cUSER037, cUSER038, cUSER039,
    cUSER040, cUSER041, cUSER042, cUSER043, cUSER044, cUSER045, cUSER046, cUSER047, cUSER048, cUSER049,
    cUSER050, cUSER051, cUSER052, cUSER053, cUSER054, cUSER055, cUSER056, cUSER057, cUSER058, cUSER059,
    cUSER060, cUSER061, cUSER062, cUSER063, cUSER064, cUSER065, cUSER066, cUSER067, cUSER068, cUSER069,
    cUSER070, cUSER071, cUSER072, cUSER073, cUSER074, cUSER075, cUSER076, cUSER077, cUSER078, cUSER079,
    cUSER080, cUSER081, cUSER082, cUSER083, cUSER084, cUSER085, cUSER086, cUSER087, cUSER088, cUSER089,
    cUSER090, cUSER091, cUSER092, cUSER093, cUSER094, cUSER095, cUSER096, cUSER097, cUSER098, cUSER099,
    cUSER100, cUSER101, cUSER102, cUSER103, cUSER104, cUSER105, cUSER106, cUSER107, cUSER108, cUSER109,
    cUSER110, cUSER111, cUSER112, cUSER113, cUSER114, cUSER115, cUSER116, cUSER117, cUSER118, cUSER119,
    cUSER120, cUSER121, cUSER122, cUSER123, cUSER124, cUSER125, cUSER126, cUSER127,
    
    cSYSTEM000, cSYSTEM001, cSYSTEM002, cSYSTEM003, cSYSTEM004, cSYSTEM005, cSYSTEM006, cSYSTEM007, cSYSTEM008, cSYSTEM009,
    cSYSTEM010, cSYSTEM011, cSYSTEM012, cSYSTEM013, cSYSTEM014, cSYSTEM015, cSYSTEM016, cSYSTEM017, cSYSTEM018, cSYSTEM019,
    cSYSTEM020, cSYSTEM021, cSYSTEM022, cSYSTEM023, cSYSTEM024, cSYSTEM025, cSYSTEM026, cSYSTEM027, cSYSTEM028, cSYSTEM029,
    cSYSTEM030, cSYSTEM031, cSYSTEM032, cSYSTEM033, cSYSTEM034, cSYSTEM035, cSYSTEM036, cSYSTEM037, cSYSTEM038, cSYSTEM039,
    cSYSTEM040, cSYSTEM041, cSYSTEM042, cSYSTEM043, cSYSTEM044, cSYSTEM045, cSYSTEM046, cSYSTEM047, cSYSTEM048, cSYSTEM049,
    cSYSTEM050, cSYSTEM051, cSYSTEM052, cSYSTEM053, cSYSTEM054, cSYSTEM055, cSYSTEM056, cSYSTEM057, cSYSTEM058, cSYSTEM059,
    cSYSTEM060, cSYSTEM061, cSYSTEM062, cSYSTEM063, cSYSTEM064, cSYSTEM065, cSYSTEM066, cSYSTEM067, cSYSTEM068, cSYSTEM069,
    cSYSTEM070, cSYSTEM071, cSYSTEM072, cSYSTEM073, cSYSTEM074, cSYSTEM075, cSYSTEM076, cSYSTEM077, cSYSTEM078, cSYSTEM079,
    cSYSTEM080, cSYSTEM081, cSYSTEM082, cSYSTEM083, cSYSTEM084, cSYSTEM085, cSYSTEM086, cSYSTEM087, cSYSTEM088, cSYSTEM089,
    cSYSTEM090, cSYSTEM091, cSYSTEM092, cSYSTEM093, cSYSTEM094, cSYSTEM095, cSYSTEM096, cSYSTEM097, cSYSTEM098, cSYSTEM099,
    cSYSTEM100, cSYSTEM101, cSYSTEM102, cSYSTEM103, cSYSTEM104, cSYSTEM105, cSYSTEM106, cSYSTEM107, cSYSTEM108, cSYSTEM109,
    cSYSTEM110, cSYSTEM111, cSYSTEM112, cSYSTEM113, cSYSTEM114, cSYSTEM115, cSYSTEM116, cSYSTEM117, cSYSTEM118, cSYSTEM119,
    cSYSTEM120, cSYSTEM121, cSYSTEM122, cSYSTEM123, cSYSTEM124, cSYSTEM125, cSYSTEM126, cSYSTEM127,
    cEXPANDED_MAX	
};

#define lensflagSHOWHINTS = 0x0010;
#define lensflagHIDESECRETS = 0x0020;
#define lensflagNOXRAY = 0x0400;
#define lensflagSHOWRAFTPATHS = 0x0800;
#define lensflagSHOWINVISENEMIES = 0x1000;


enum 
{
	USR_MIDI_DEATH, USR_MIDI_GAMEOVER, USR_MIDI_OVERWORLD, USR_MIDI_DUNGEON, USR_MIDI_LEVELNINE, USR_MIDI_MAX 
	
}; //Obviously, the overworld, dungeon and L9 midis don;t need to be here, but we can define some user space for o
//	otherwise hardcoded sounds, and use these instead of hardcoded SFX if they are set somehow. 

#define QUESTRULES_SIZE 20
#define QUESTRULES_NEW_SIZE 100
#define QR_SZ QUESTRULES_NEW_SIZE
#define EXTRARULES_SIZE 1

// "quest rules" flags (bit numbers in bit string)
enum
{
    // 1st byte out of 20
    qr_SOLIDBLK, qr_NOTMPNORET, qr_ALWAYSRET, qr_MEANTRAPS,
    qr_BSZELDA, qr_FADE, qr_FADECS5, qr_FASTDNGN,
    // 2
    qr_NOLEVEL3FIX, qr_COOLSCROLL, qr_NOSCROLL, qr_4TRI,
    qr_EXPLICIT_RAFTING, qr_FASTFILL, qr_CAVEEXITNOSTOPMUSIC, qr_3TRI,
    // 3
    qr_TIME, qr_FREEFORM, qr_KILLALL, qr_NOFLICKER,
    qr_CONTFULL_DEP/*DEPRECATED*/, qr_RLFIX, qr_LENSHINTS, /*DEPRECATED*/ 
    qr_HERODUNGEONPOSFIX,
    // 4
    qr_HOLDITEMANIMATION, qr_HESITANTPUSHBLOCKS, qr_HIDECARRIEDITEMS, qr_FFCSCROLL,
    qr_RAFTLENS, /*DEPRECATED*/  
	qr_SMOOTHVERTICALSCROLLING, qr_WHIRLWINDMIRROR, qr_NOFLASHDEATH,
    // 5
    qr_HOLDNOSTOPMUSIC, qr_FIREPROOFHERO, qr_OUCHBOMBS, qr_NOCLOCKS_DEP/*DEPRECATED*/,
    qr_TEMPCLOCKS_DEP/*DEPRECATED*/, qr_BRKBLSHLDS_DEP/*DEPRECATED*/, qr_BRKNSHLDTILES, qr_MEANPLACEDTRAPS,
    // 6
    qr_PHANTOMPLACEDTRAPS, qr_ALLOWFASTMSG, qr_LINKEDCOMBOS/*DEPRECATED*/, qr_NOGUYFIRES,
    qr_HEARTRINGFIX, qr_NOHEARTRING, qr_OVERWORLDTUNIC, qr_SWORDWANDFLIPFIX,
    // 7
    qr_ENABLEMAGIC, qr_MAGICWAND_DEP/*DEPRECATED*/, qr_MAGICCANDLE_DEP/*DEPRECATED*/, qr_MAGICBOOTS_DEP/*DEPRECATED*/,
    qr_NONBUBBLEMEDICINE, qr_NONBUBBLEFAIRIES, qr_NONBUBBLETRIFORCE, qr_NEWENEMYTILES,
    // 8
    qr_NOROPE2FLASH_DEP/*DEPRECATED*/, qr_NOBUBBLEFLASH_DEP/*DEPRECATED*/, qr_GHINI2BLINK_DEP, qr_BITMAPOFFSETFIX, /* UNIMPLEMENTED */
    qr_PHANTOMGHINI2_DEP/*DEPRECATED*/, qr_Z3BRANG_HSHOT, qr_NOITEMMELEE/*DEPRECATED*/, qr_SHADOWS,
    // 9
    qr_TRANSSHADOWS, qr_QUICKSWORD, qr_BOMBHOLDFIX, qr_EXPANDEDLTM,
    qr_NOPOTIONCOMBINE_DEP/*DEPRECATED*/, qr_HEROFLICKER, qr_SHADOWSFLICKER, qr_WALLFLIERS,
    // 10
    qr_NOBOMBPALFLASH, qr_HEARTSREQUIREDFIX, qr_PUSHBLOCKCSETFIX, qr_TRANSLUCENTDIVINEPROTECTIONROCKET_DEP/*DEPRECATED*/,
    qr_FLICKERINGDIVINEPROTECTIONROCKET_DEP/*DEPRECATED*/, qr_CMBCYCLELAYERS, qr_DMGCOMBOPRI, qr_WARPSIGNOREARRIVALPOINT,
    // 11
    qr_LTTPCOLLISION, qr_LTTPWALK, qr_SLOWENEMYANIM_DEP/*DEPRECATED*/, qr_TRUEARROWS,
    qr_NOSAVE, qr_NOCONTINUE, qr_QUARTERHEART, qr_NOARRIVALPOINT,
    // 12
    qr_NOGUYPOOF, qr_ALLOWMSGBYPASS, qr_NODIVING, qr_LAYER12UNDERCAVE,
    qr_NOSCROLLCONTINUE, qr_SMARTSCREENSCROLL, qr_RINGAFFECTDAMAGE/*DEPRECATED*/, qr_ALLOW10RUPEEDROPS_DEP/*DEPRECATED*/,
    // 13
    qr_TRAPPOSFIX, qr_TEMPCANDLELIGHT, qr_REDPOTIONONCE_DEP/*DEPRECATED*/, qr_OLDSTYLEWARP,
    qr_NOBORDER, qr_OLDTRIBBLES_DEP/*DEPRECATED*/, qr_REFLECTROCKS /*DEPRECATED*/, qr_OLDPICKUP,
    // 14
    qr_ENEMIESZAXIS, qr_SAFEENEMYFADE, qr_MORESOUNDS, qr_BRANGPICKUP,
    qr_HEARTPIECEINCR_DEP/*DEPRECATED*/, qr_ITEMSONEDGES, qr_EATSMALLSHIELD_DEP/*DEPRECATED*/, qr_MSGFREEZE,
    // 15
    qr_SLASHFLIPFIX/*DEPRECATED*/, qr_FIREMAGICSPRITE_DEP/*DEPRECATED*/, qr_SLOWCHARGINGWALK/*DEPRECATED*/, qr_NOWANDMELEE/*DEPRECATED*/,
    qr_SLOWBOMBFUSES_DEP/*DEPRECATED*/, qr_SWORDMIRROR, qr_SELECTAWPN, qr_LENSSEESENEMIES, /*DEPRECATED*/ 
    // 16
    qr_INSTABURNFLAGS, qr_DROWN, qr_MSGDISAPPEAR, qr_SUBSCREENOVERSPRITES,
    qr_BOMBDARKNUTFIX, qr_LONGBOMBBOOM_DEP/*DEPRECATED*/, qr_OFFSETEWPNCOLLISIONFIX, qr_DMGCOMBOLAYERFIX,
    // 17
    qr_ITEMSINPASSAGEWAYS, qr_LOG, qr_FIREPROOFHERO2, qr_NOITEMOFFSET,
    qr_ITEMBUBBLE, qr_GOTOLESSNOTEQUAL /* Compatibility */, qr_LADDERANYWHERE, qr_HOOKSHOTLAYERFIX,
    // 18
    qr_REPLACEOPENDOORS /* Compatibility */, qr_OLDLENSORDER /* Compatibility */, qr_NOFAIRYGUYFIRES /* Compatibility */, qr_SCRIPTERRLOG,
    qr_TRIGGERSREPEAT /* Compatibility */, qr_ENEMIESFLICKER, qr_OVALWIPE, qr_TRIANGLEWIPE,
    // 19
    qr_SMASWIPE, qr_NOSOLIDDAMAGECOMBOS /* Compatibility */, qr_SHOPCHEAT, qr_HOOKSHOTDOWNBUG /* Compatibility */,
    qr_OLDHOOKSHOTGRAB /* Compatibility */, qr_PEAHATCLOCKVULN /* Compatibility */, qr_VERYFASTSCROLLING, qr_OFFSCREENWEAPONS /* Compatibility */,
    // 20
    qr_BROKENSTATUES /* Compatibility */, qr_BOMBCHUSUPERBOMB,
    qr_ITEMPICKUPSETSBELOW /* Compatibility */,
    
    qr_GANONINTRO, //bugfix//
    qr_MELEEMAGICCOST/*DEPRECATED*/,
    qr_OLDMIRRORCOMBOS,
    qr_BROKENBOOKCOST,
    qr_OLDSIDEVIEWSPIKES,
    //21
	qr_OLDINFMAGIC/* Compatibility */, //Infinite magic prevents items from draining rupees
	qr_NEVERDISABLEAMMOONSUBSCREEN, qr_ITEMSCRIPTSKEEPRUNNING,
	qr_SCRIPTSRUNINHEROSTEPFORWARD, /*qr_SCRIPTDRAWSINCANCELWARP,*/ qr_FIXSCRIPTSDURINGSCROLLING, qr_SCRIPTDRAWSINWARPS,
	qr_DYINGENEMYESDONTHURTHERO, //t.b.a
	qr_SIDEVIEWTRIFORCECELLAR,
	//22
	qr_OUTOFBOUNDSENEMIES,
	qr_EPILEPSY,
	qr_SCRIPT_FRIENDLY_ENEMY_TYPES,
	qr_SMARTDROPS,
	qr_NO_L_R_BUTTON_INVENTORY_SWAP,
	qr_USE_EX1_EX2_INVENTORYSWAP,
	qr_NOFASTMODE,
	qr_OLD_F6 /* Compatibility */,
	//23
	qr_BROKEN_ASKIP_Y_FRAMES /* Compatibility */, qr_ENEMY_BROKEN_TOP_HALF_SOLIDITY /* Compatibility */, qr_SIDEVIEWLADDER_FACEUP, qr_ITEMS_IGNORE_SIDEVIEW_PLATFORMS,
	qr_DOWN_FALL_THROUGH_SIDEVIEW_PLATFORMS, qr_SIDEVIEW_FALLTHROUGH_USES_DRUNK, qr_DOWN_DOESNT_GRAB_LADDERS, qr_DOWNJUMP_FALL_THROUGH_SIDEVIEW_PLATFORMS,
	//24
	qr_OLD_SIDEVIEW_CEILING_COLLISON /* Compatibility */, qr_0AFRAME_ITEMS_IGNORE_AFRAME_CHANGES /* Compatibility */, qr_OLD_ENEMY_KNOCKBACK_COLLISION /* Compatibility */, qr_FADEBLACKWIPE,
	qr_PASSIVE_SUBSCRIPT_RUNS_DURING_ACTIVE_SUBSCRIPT, qr_DMAP_ACTIVE_RUNS_DURING_ACTIVE_SUBSCRIPT, qr_ANIMATECUSTOMWEAPONS, qr_WEAPONSMOVEOFFSCREEN,
	//25
	qr_CHECKSCRIPTWEAPONOFFSCREENCLIP, qr_SHORTDGNWALK, qr_SCRIPT_WEAPONS_UNIQUE_SPRITES, qr_ANGULAR_REFLECTED_WEAPONS,
	qr_MIRRORS_USE_WEAPON_CENTER, qr_CUSTOMCOMBOSLAYERS1AND2, qr_BUSHESONLAYERS1AND2, qr_NEW_HERO_MOVEMENT,
	//26
	qr_DISABLE_4WAY_GRIDLOCK, qr_NEW_COMBO_ANIMATION, qr_OLD_STRING_EDITOR_MARGINS, qr_NO_OVERWRITING_HOPPING,
	qr_DUNGEON_DMAPS_PERM_SECRETS, qr_STRING_FRAME_OLD_WIDTH_HEIGHT, qr_NO_SCROLL_WHILE_IN_AIR, qr_INSTANT_RESPAWN,
	//27
	qr_BROKEN_OVERWORLD_MINIMAP, qr_NO_STUNLOCK, qr_GREATER_MAX_TIME, qr_BROKEN_RING_POWER,
	qr_NO_OVERWORLD_MAP_CHARTING, qr_DUNGEONS_USE_CLASSIC_CHARTING, qr_ARROWS_ALWAYS_PENETRATE, qr_SWORDBEAMS_ALWAYS_PENETRATE, 
	//28
	qr_SET_XBUTTON_ITEMS, qr_SMARTER_WATER, qr_NO_HOPPING, qr_NO_SOLID_SWIM, 
	qr_WATER_ON_LAYER_1, qr_WATER_ON_LAYER_2, qr_SHALLOW_SENSITIVE, qr_SMARTER_SMART_SCROLL,
	//29
	qr_INSTANT_CONTINUE, qr_NONHEAVY_BLOCKTRIGGER_PERM, qr_SET_YBUTTON_ITEMS, qr_CORRECTED_EW_BRANG_ANIM,
	qr_SCRIPT_WRITING_HEROSTEP_DOESNT_CARRY_OVER, qr_OVERHEAD_COMBOS_L1_L2, qr_ALLOW_EDITING_COMBO_0, qr_OLD_CHEST_COLLISION,
	//30
	qr_AUTOCOMBO_LAYER_1, qr_AUTOCOMBO_LAYER_2, qr_TRUEFIXEDBOMBSHIELD, qr_BOMBSPIERCESHIELD,
	qr_BROKEN_HORIZONTAL_WEAPON_ANIM, qr_NEW_DARKROOM, qr_NEWDARK_L6, qr_ENEMIES_SECRET_ONLY_16_31,
	//31
	qr_SCREEN80_OWN_MUSIC, qr_OLDCS2, qr_HARDCODED_ENEMY_ANIMS, qr_OLD_ITEMDATA_SCRIPT_TIMING,
	qr_SIDESWIM, qr_SIDESWIMDIR, qr_PUSHBLOCK_LAYER_1_2, qr_NEWDARK_SCROLLEDGE,
	//32
	qr_STEPTEMP_SECRET_ONLY_16_31, qr_ALLTRIG_PERMSEC_NO_TEMP, qr_HARDCODED_LITEM_LTMS, qr_NO_BOTTLE_IF_ANY_COUNTER_FULL,
	qr_LIGHTBEAM_TRANSPARENT, qr_CANDLES_SHARED_LIMIT, qr_OLD_RESPAWN_POINTS, qr_HOOKSHOTALLLAYER,
	//33
	qr_ANONE_NOANIM, qr_BLOCKHOLE_SAME_ONLY, qr_SWITCHOBJ_RUN_SCRIPT, qr_ITEMCOMBINE_NEW_PSTR,
	qr_ITEMCOMBINE_CONTINUOUS, qr_SCC_ITEM_COMBINES_ITEMS, qr_SCROLLING_KILLS_CHARGE, qr_CUSTOMWEAPON_IGNORE_COST,
	//34
	qr_BLOCKS_DONT_LOCK_OTHER_LAYERS, qr_SCC_GOTO_RESPECTS_CONTFLAG, qr_BROKEN_KEEPOLD_FLAG, qr_KEEPOLD_APPLIES_RETROACTIVELY,
	qr_PASSIVE_ITEM_SCRIPT_ONLY_HIGHEST, qr_OLD_HALF_MAGIC, qr_LEVEL_RESTART_CONT_POINT, qr_SUBSCR_OLD_SELECTOR,
	//35
	qr_OLD_FAIRY_LIMIT, qr_FAIRYDIR, qr_ARROWCLIP, qr_CONT_SWORD_TRIGGERS, 
	qr_OLD_210_WATER, qr_8WAY_SHOT_SFX, qr_COPIED_SWIM_SPRITES, qr_WRONG_BRANG_TRAIL_DIR,
	//36
	qr_192b163_WARP, qr_210_WARPRETURN, qr_LESS_AWFUL_SIDESPIKES, qr_OLD_LADDER_ITEM_SIDEVIEW,
	qr_OLD_SECRETMONEY, qr_OLD_DOORREPAIR, qr_OLD_POTION_OR_HC, qr_NO_LANMOLA_RINGLEADER,
	//37
	qr_HARDCODED_BS_PATRA, qr_PATRAS_USE_HARDCODED_OFFSETS, qr_BROKEN_BIG_ENEMY_ANIMATION, qr_BROKEN_ATTRIBUTE_31_32,
	qr_CAN_PLACE_GANON, qr_CAN_PLACE_TRAPS, qr_FIND_CLOSEST_BAIT, qr_OLD_FLAMETRAIL_DURATION,
	//38
	qr_PERMANENT_WHISTLE_SECRETS, qr_SMARTER_DROPS, qr_WAVY_NO_EPILEPSY, qr_WAVY_NO_EPILEPSY_2,
	qr_DYING_ENEMIES_IGNORE_STUN, qr_SHOP_ITEMS_VANISH, qr_ANGULAR_REFLECT_BROKEN, qr_CSET1_LEVEL,
	//39
	qr_CSET5_LEVEL, qr_CSET7_LEVEL, qr_CSET8_LEVEL, qr_FADECS1,
	qr_FADECS7, qr_FADECS8, qr_FADECS9, qr_UNBEATABLES_DONT_KEEP_DEAD,
	//40
	qr_NO_LEAVE_ONE_ENEMY_ALIVE_TRICK, qr_OLD_BRIDGE_COMBOS, qr_BROKEN_Z3_ANIMATION, qr_OLD_TILE_INITIALIZATION,
	qr_FLUCTUATING_ENEMY_JUMP, qr_SPRITE_JUMP_IS_TRUNCATED, qr_BUGGY_BUGGY_SLASH_TRIGGERS, qr_OLD_DRAWOFFSET,
	//41
	qr_BROKEN_DRAWSCREEN_FUNCTIONS, qr_ENEMY_DROPS_USE_HITOFFSETS, qr_BROKEN_ITEM_CARRYING, qr_LEEVERS_DONT_OBEY_STUN,
	qr_QUAKE_STUNS_LEEVERS, qr_GANON_CANT_SPAWN_ON_CONTINUE, qr_WIZZROBES_DONT_OBEY_STUN, qr_OLD_BUG_NET,
	//42
	qr_MANHANDLA_BLOCK_SFX, qr_GRASS_SENSITIVE, qr_BETTER_RAFT, qr_BETTER_RAFT_2,
	qr_RAFT_SOUND, qr_WARPS_RESTART_DMAPSCRIPT, qr_DMAP_0_CONTINUE_BUG, qr_SCRIPT_WARPS_DMAP_SCRIPT_TOGGLE,
	//43
	qr_OLD_SCRIPTED_KNOCKBACK, qr_OLD_KEESE_Z_AXIS, qr_POLVIRE_NO_SHADOW, qr_CONVEYORS_L1_L2,
	qr_CUSTOMCOMBOS_EVERY_LAYER, qr_SUBSCR_BACKWARDS_ID_ORDER, qr_FASTCOUNTERDRAIN, qr_OLD_LOCKBLOCK_COLLISION,
	//44
	qr_DECO_2_YOFFSET, qr_SCREENSTATE_80s_BUG, qr_AUTOCOMBO_ANY_LAYER, qr_GOHMA_UNDAMAGED_BUG,
	qr_FFCPRELOAD_BUGGED_LOAD, qr_SWITCHES_AFFECT_MOVINGBLOCKS, qr_BROKEN_GETPIXEL_VALUE, qr_NO_LIFT_SPRITE,
	//45
	qr_OLD_SIDEVIEW_LANDING_CODE, qr_OLD_FFC_SPEED_CAP, qr_OLD_WIZZROBE_SUBMERGING, qr_SPARKLES_INHERIT_PROPERTIES,
	qr_BROKENHITBY, qr_ENEMIES_DONT_SCRIPT_FIRST_FRAME,
	
	//50
	qr_OLD_FFC_FUNCTIONALITY = 50*8, qr_OLD_SHALLOW_SFX, qr_BUGGED_LAYERED_FLAGS, qr_HARDCODED_FFC_BUSH_DROPS,
	qr_POUNDLAYERS1AND2, qr_MOVINGBLOCK_FAKE_SOLID, qr_NEW_HERO_MOVEMENT2, qr_CARRYABLE_NO_ACROSS_SCREEN,
	//51
	qr_NO_SCROLL_WHILE_CARRYING, qr_HELD_BOMBS_EXPLODE, qr_BROKEN_MOVING_BOMBS, qr_OLD_BOMB_HITBOXES,
	qr_SCROLLWARP_NO_RESET_FRAME, qr_BROKEN_RAFT_SCROLL,
	//60
	//70
	
	//ZScript Parser //room for 20 of these
	//80
	qr_PARSER_250DIVISION = 80*8, qr_PARSER_NO_LOGGING, qr_PARSER_SHORT_CIRCUIT, qr_PARSER_BOOL_TRUE_DECIMAL,
	qr_SPRITEXY_IS_FLOAT, qr_PARSER_TRUE_INT_SIZE,
	qr_WPNANIMFIX, /* Not Implemented : This was in 2.50.2, but never used. */ 
	qr_NOSCRIPTSDURINGSCROLL, /* Not Implemented : This was in 2.50.2, but never used. */
	//81
	qr_OLDSPRITEDRAWS, qr_WEAPONSHADOWS, qr_ITEMSHADOWS, qr_OLDEWPNPARENT,
	qr_OLDCREATEBITMAP_ARGS, qr_OLDQUESTMISC, qr_PARSER_FORCE_INLINE, qr_CLEARINITDONSCRIPTCHANGE,
	//82
	qr_NOFFCWAITDRAW, qr_NOITEMWAITDRAW, qr_TRACESCRIPTIDS, qr_FIXDRUNKINPUTS,
	qr_PARSER_BINARY_32BIT, qr_ALWAYS_DEALLOCATE_ARRAYS, qr_ONDEATH_RUNS_AFTER_DEATH_ANIM, qr_DISALLOW_SETTING_RAFTING,
	//83
	qr_WEAPONS_EXTRA_FRAME, qr_250WRITEEDEFSCRIPT, qr_SETENEMYWEAPONSPRITESONWPNCHANGE, qr_BROKENCHARINTDRAWING,
	qr_WRITING_NPC_WEAPON_UNIQUE_SPRITES, qr_COMBOSCRIPTS_LAYER_0, qr_COMBOSCRIPTS_LAYER_1, qr_COMBOSCRIPTS_LAYER_2,
	//84
	qr_COMBOSCRIPTS_LAYER_3, qr_COMBOSCRIPTS_LAYER_4, qr_COMBOSCRIPTS_LAYER_5, qr_COMBOSCRIPTS_LAYER_6,
	qr_OLD_INIT_SCRIPT_TIMING, qr_DO_NOT_DEALLOCATE_INIT_AND_SAVELOAD_ARRAYS, qr_BITMAP_AND_FILESYSTEM_PATHS_ALWAYS_RELATIVE, qr_PARSER_STRINGSWITCH_INSENSITIVE,
	//85
	qr_OLD_SLASHNEXT_SECRETS, qr_STEP_IS_FLOAT, qr_OLDMAPDATAFLAGS, qr_OLD_PRINTF_ARGS,
	qr_PASSIVE_SUBSCRIPT_RUNS_WHEN_GAME_IS_FROZEN, qr_WRITE_ENTRYPOINTS_AFFECTS_HEROCLASS, qr_LOG_INVALID_UID_LOAD, qr_COMBODATA_INITD_MULT_TENK,
	//86
	qr_ZS_NO_NEG_ARRAY,
	
    qr_MAX
};

// Unsaved compatibility rules
enum extraRules
{
    er_BITMAPOFFSET, //to be deprecated by qr_BITMAPOFFSETFIX
	er_SHORTDGNWALK,
	er_MAGICCOSTSWORD, //BEAMS Only
	er_BROKENCHARINTDRAWING, //pre-2.53 fix to DrawInteger() and DrarCharacter() -Z
	er_SETENEMYWEAPONSPRITESONWPNCHANGE, //er_OLDSIDEVIEWSPIKES,
	er_MAX
};

// room types
enum
{
    rNONE, rSP_ITEM, rINFO, rMONEY, rGAMBLE, rREPAIR, rRP_HC, rGRUMBLE,
    rTRIFORCE, rP_SHOP, rSHOP, rBOMBS, rSWINDLE, r10RUPIES, rWARP,
    rGANON, rZELDA, rITEMPOND, rMUPGRADE, rLEARNSLASH, rARROWS, rTAKEONE,
	rBOTTLESHOP,
    rMAX
};

// directions
enum direction { dir_invalid = -1, up, down, left, right, l_up, r_up, l_down, r_down };
const direction oppositeDir[]= {down, up, right, left, r_down, l_down, r_up, l_up};
const direction normalDir[]={up,down,left,right,l_up,r_up,l_down,r_down,up,r_up,right,r_down,down,l_down,left,l_up};
const direction xDir[] = { dir_invalid,dir_invalid,left,right,left,right,left,right };
const direction yDir[] = { up,down,dir_invalid,dir_invalid,up,up,down,down };
direction X_DIR(int32_t dir);
direction Y_DIR(int32_t dir);
direction XY_DIR(int32_t xdir, int32_t ydir);
direction XY_DELTA_TO_DIR(int32_t dx, int32_t dy); // TODO ! remove? is this just XY_DIR?
direction GET_XDIR(zfix const& sign);
direction GET_YDIR(zfix const& sign);
direction GET_DIR(zfix const& dx, zfix const& dy);
#define NORMAL_DIR(dir)    ((dir >= 0 && dir < 16) ? normalDir[dir] : dir_invalid)

// refill stuff
enum { REFILL_NONE, REFILL_FAIRYDONE, REFILL_LIFE, REFILL_MAGIC, REFILL_ALL};
#define REFILL_FAIRY -1

// 2.55 lighting stuff
enum { pal_litDEFAULT, pal_litOVERRIDE, pal_litRESET, pal_litSET, pal_litRESETONLY };

//Z-axis related
#define FEATHERJUMP 80

//other
#define MAXDRUNKCLOCK          500

enum
{
    dBUSHLEAVES, dFLOWERCLIPPINGS, dGRASSCLIPPINGS, dHAMMERSMACK,
    dTALLGRASS, dRIPPLES, dDIVINEPROTECTIONSHIELD, dHOVER, dMAXDECORATIONS
};

// items
enum                                                        // value matters because of current quest file data
{
    iRupy, i5Rupies, iHeart, iBombs, iClock,
    iSword, iWSword, iMSword, iShield, iKey,
    // 10
    iBCandle, iRCandle, iLetter, iArrow, iSArrow,
    iBow, iBait, iBRing, iRRing, iBracelet,
    // 20
    iTriforce, iMap, iCompass, iBrang, iMBrang,
    iWand, iRaft,iLadder,iHeartC, iBPotion,
    // 30
    iRPotion, iWhistle,iBook, iMKey, iFairyMoving,
    iFBrang, iXSword, iMShield, i20Rupies, i50Rupies,
    // 40
    i200Rupies, iWallet500, iWallet999, iPile, iBigTri,
    iSelectA, iMisc1, iMisc2, iSBomb, iHCPiece,
    // 50
    iAmulet, iFlippers, iHookshot, iLens, iHammer,
    iBoots, iL2Bracelet, iGArrow, iMagicC, iSMagic,
    // 60
    iLMagic, iGRing, iKillAll, iL2Amulet, iDivineFire,
    iDivineEscape, iDivineProtection, iBossKey, iBow2, iFairyStill,
    // 70
    i1ArrowAmmo, i5ArrowAmmo, i10ArrowAmmo, i30ArrowAmmo, iQuiver,
    iQuiverL2, iQuiverL3, i1BombAmmo, i4BombAmmo, i8BombAmmo,
    // 80
    i30BombAmmo, iBombBag, iBombBagL2, iBombBagL3, iLevelKey,
    iSelectB, i10Rupies, i100Rupies, iCByrna, iLongshot,
    // 90
    iLetterUsed,iRocsFeather,iHoverBoots,iSShield,iSpinScroll,
    iCrossScroll,iQuakeScroll,iL2QuakeScroll,iL2SpinScroll,iWhispRing,
    // 100
    iL2WhispRing, iChargeRing, iL2ChargeRing, iPerilScroll, iWalletL3,
    iQuiverL4, iBombBagL4, iOldGlove, iL2Ladder,iWealthMedal,
    // 110
    iL2WealthMedal, iL3WealthMedal,iHeartRing, iL2HeartRing, iL3HeartRing,
    iMagicRing, iL2MagicRing, iL3MagicRing, iL4MagicRing, iStoneAgony,
    // 120
    iStompBoots, iPerilRing, iWhimsicalRing,
    iCustom1, iCustom2, iCustom3, iCustom4, iCustom5,
    iCustom6, iCustom7, iCustom8, iCustom9, iCustom10,
    iCustom11, iCustom12, iCustom13, iCustom14, iCustom15,
    iCustom16, iCustom17, iCustom18, iCustom19, iCustom20,
    iLast,
    iMax=256
};

// Shield projectile blocking
#define shROCK		0x001
#define shARROW		0x002
#define shBRANG		0x004
#define shFIREBALL	0x008
#define shSWORD		0x010
#define shMAGIC		0x020
#define shFLAME		0x040
#define shSCRIPT	0x080
#define shFIREBALL2	0x100 // Boss fireball, not ewFireball2
#define shLIGHTBEAM 0x200 //Light puzzle beams


// item sets
enum
{
    isNONE, isDEFAULT, isBOMBS, isMONEY, isLIFE, isBOMB100, isSBOMB100,
    isMAGIC, isMAGICBOMBS, isMAGICMONEY, isMAGICLIFE, isMAGIC2, isTALLGRASS, isMAX
};

// weapons (in qst data)
enum
{
    wSWORD, wWSWORD, wMSWORD, wXSWORD, wBRANG,
    wMBRANG, wFBRANG, wBOMB, wSBOMB, wBOOM,
// 10
    wARROW, wSARROW, wFIRE, wWIND, wBAIT,
    wWAND, wMAGIC, ewFIREBALL, ewROCK, ewARROW,
// 20
    ewSWORD, ewMAGIC, iwSpawn, iwDeath, iwSwim,
    wHAMMER, wHSHEAD, wHSCHAIN_H, wHSHANDLE, wSSPARKLE,
// 30
    wGSPARKLE, wMSPARKLE, wFSPARKLE, iwHammerSmack, wGARROW,
    ewFLAME, ewWIND, iwMMeter, wDIVINEFIRE1A, wDIVINEFIRE1B,
// 40
    wDIVINEFIRES1A, wDIVINEFIRES1B, wHSCHAIN_V, iwMore, iwBossMarker,
    iwHeroSlash, wSWORDSLASH, wWSWORDSLASH, wMSWORDSLASH, wXSWORDSLASH,
// 50
    iwShadow, iwLargeShadow, iwBushLeaves, iwFlowerClippings, iwGrassClippings,
    iwTallGrass, iwRipples, iwNPCs, wDIVINEPROTECTION1A, wDIVINEPROTECTION1B,
// 60
    wDIVINEPROTECTIONS1A, wDIVINEPROTECTIONS1B, wDIVINEPROTECTION2A, wDIVINEPROTECTION2B, wDIVINEPROTECTIONS2A,
    wDIVINEPROTECTIONS2B, iwDivineProtectionShieldFront, iwDivineProtectionShieldBack, iwSubscreenVine, wCBYRNA,
// 70
    wCBYRNASLASH, wLSHEAD, wLSCHAIN_H, wLSHANDLE, wLSCHAIN_V,
    wSBOOM, ewBOMB, ewSBOMB, ewBOOM, ewSBOOM,
// 80
    ewFIRETRAIL, ewFLAME2, ewFLAME2TRAIL, ewICE, iwHover,
    wFIREMAGIC, iwQuarterHearts, wCBYRNAORB, //iwSideLadder, iwSideRaft,
    
    wLast, wSCRIPT, wMAX=256
};

//Types of hit detection
#define HIT_BY_NPC 0
#define HIT_BY_EWEAPON 1
#define HIT_BY_LWEAPON 2
#define HIT_BY_FFC 3

#define HIT_BY_NPC_UID 4
#define HIT_BY_EWEAPON_UID 5
#define HIT_BY_LWEAPON_UID 6
#define HIT_BY_FFC_UID 7

#define HIT_BY_LWEAPON_PARENT_ID 8
#define HIT_BY_LWEAPON_PARENT_FAMILY 9

#define HIT_BY_NPC_TYPE 10
#define HIT_BY_EWEAPON_TYPE 11
#define HIT_BY_LWEAPON_TYPE 12
#define HIT_BY_NPC_ENGINE_UID 13
#define HIT_BY_EWEAPON_ENGINE_UID 14
#define HIT_BY_LWEAPON_ENGINE_UID 15
#define HIT_BY_NPC_ID 16


#define NUM_HIT_TYPES_USED 17

//triggerflags[0]

#define combotriggerSWORD        0x00000001
#define combotriggerSWORDBEAM    0x00000002
#define combotriggerBRANG        0x00000004
#define combotriggerBOMB         0x00000008
#define combotriggerSBOMB        0x00000010
#define combotriggerLITBOMB      0x00000020
#define combotriggerLITSBOMB     0x00000040
#define combotriggerARROW        0x00000080
#define combotriggerFIRE         0x00000100
#define combotriggerWHISTLE      0x00000200
#define combotriggerBAIT         0x00000400
#define combotriggerWAND         0x00000800
#define combotriggerMAGIC        0x00001000
#define combotriggerWIND         0x00002000
#define combotriggerREFMAGIC     0x00004000
#define combotriggerREFFIREBALL  0x00008000
#define combotriggerREFROCK      0x00010000
#define combotriggerHAMMER       0x00020000
#define combotriggerRESETANIM    0x00040000
#define combotriggerINVERTPROX   0x00080000
#define combotriggerBTN_TOP      0x00100000
#define combotriggerBTN_BOTTOM   0x00200000
#define combotriggerBTN_LEFT     0x00400000
#define combotriggerBTN_RIGHT    0x00800000
#define combotriggerINVERTMINMAX 0x01000000
#define combotriggerSTEP         0x02000000
#define combotriggerSTEPSENS     0x04000000
#define combotriggerSHUTTER      0x08000000
#define combotriggerCMBTYPEFX    0x10000000
#define combotriggerONLYGENTRIG  0x20000000
#define combotriggerKILLWPN      0x40000000
#define combotriggerEWFIREBALL   0x80000000

//triggerflags[1]
#define combotriggerHOOKSHOT     0x00000001
#define combotriggerSPARKLE      0x00000002
#define combotriggerBYRNA        0x00000004
#define combotriggerREFBEAM      0x00000008
#define combotriggerSTOMP        0x00000010
#define combotriggerSCRIPT01     0x00000020
#define combotriggerSCRIPT02     0x00000040
#define combotriggerSCRIPT03     0x00000080
#define combotriggerSCRIPT04     0x00000100
#define combotriggerSCRIPT05     0x00000200
#define combotriggerSCRIPT06     0x00000400
#define combotriggerSCRIPT07     0x00000800
#define combotriggerSCRIPT08     0x00001000
#define combotriggerSCRIPT09     0x00002000
#define combotriggerSCRIPT10     0x00004000
#define combotriggerAUTOMATIC    0x00008000
#define combotriggerSECRETS      0x00010000
#define combotriggerINVERTITEM   0x00020000
#define combotriggerCONSUMEITEM  0x00040000
#define combotriggerCOUNTERGE    0x00080000
#define combotriggerCOUNTERLT    0x00100000
#define combotriggerCOUNTEREAT   0x00200000
#define combotriggerCTRNONLYTRIG 0x00400000
#define combotriggerLIGHTON      0x00800000
#define combotriggerLIGHTOFF     0x01000000
#define combotriggerPUSH         0x02000000
#define combotriggerLENSON       0x04000000
#define combotriggerLENSOFF      0x08000000
#define combotriggerEWARROW      0x10000000
#define combotriggerEWBRANG      0x20000000
#define combotriggerEWSWORD      0x40000000
#define combotriggerEWROCK       0x80000000

//triggerflags[2]
#define combotriggerEWSCRIPT01     0x00000001
#define combotriggerEWSCRIPT02     0x00000002
#define combotriggerEWSCRIPT03     0x00000004
#define combotriggerEWSCRIPT04     0x00000008
#define combotriggerEWSCRIPT05     0x00000010
#define combotriggerEWSCRIPT06     0x00000020
#define combotriggerEWSCRIPT07     0x00000040
#define combotriggerEWSCRIPT08     0x00000080
#define combotriggerEWSCRIPT09     0x00000100
#define combotriggerEWSCRIPT10     0x00000200
#define combotriggerEWMAGIC        0x00000400
#define combotriggerEWBBLAST       0x00000800
#define combotriggerEWSBBLAST      0x00001000
#define combotriggerEWLITBOMB      0x00002000
#define combotriggerEWLITSBOMB     0x00004000
#define combotriggerEWFIRETRAIL    0x00008000
#define combotriggerEWFLAME        0x00010000
#define combotriggerEWWIND         0x00020000
#define combotriggerEWFLAME2       0x00040000
#define combotriggerSPCITEM        0x00080000
#define combotriggerEXSTITEM       0x00100000
#define combotriggerEXSTENEMY      0x00200000
#define combotriggerAUTOGRABITEM   0x00400000
#define combotriggerENEMIESKILLED  0x00800000
#define combotriggerSECRETSTR      0x01000000
#define combotriggerTHROWN         0x02000000
#define combotriggerQUAKESTUN      0x04000000
#define combotriggerSQUAKESTUN     0x08000000
#define combotriggerANYFIRE        0x10000000
#define combotriggerSTRONGFIRE     0x20000000
#define combotriggerMAGICFIRE      0x40000000
#define combotriggerDIVINEFIRE     0x80000000

//triggerflags[3]
#define combotriggerTRIGLEVELSTATE      0x00000001
#define combotriggerLEVELSTATE          0x00000002
#define combotriggerTRIGGLOBALSTATE     0x00000004
#define combotriggerGLOBALSTATE         0x00000008
#define combotriggerKILLENEMIES         0x00000010
#define combotriggerCLEARENEMIES        0x00000020
#define combotriggerCLEARLWEAPONS       0x00000040
#define combotriggerCLEAREWEAPONS       0x00000080
#define combotriggerIGNITE_ANYFIRE      0x00000100
#define combotriggerIGNITE_STRONGFIRE   0x00000200
#define combotriggerIGNITE_MAGICFIRE    0x00000400
#define combotriggerIGNITE_DIVINEFIRE   0x00000800
#define combotriggerSEPARATEWEAPON      0x00001000
#define combotriggerTGROUP_CONTRIB      0x00002000
#define combotriggerTGROUP_LESS         0x00004000
#define combotriggerTGROUP_GREATER      0x00008000
#define combotriggerPUSHEDTRIG          0x00010000

#define ctrigNONE          0x00
#define ctrigIGNORE_SIGN   0x01
#define ctrigSECRETS       0x02
#define ctrigSWITCHSTATE   0x04

// weapon types in game engine
enum
{
    // 0
    wNone,wSword,wBeam,wBrang,
    wBomb,wSBomb,wLitBomb,wLitSBomb,
    // 8
    wArrow,wFire,wWhistle,wBait,
    wWand,wMagic,wCatching,wWind,
    // 16
    wRefMagic,wRefFireball,wRefRock, wHammer,
    wHookshot, wHSHandle, wHSChain, wSSparkle,
    // 24
    wFSparkle, wSmack, wPhantom, wCByrna,
	//28
    wRefBeam, wStomp,
	//30
    lwMax,
    // Dummy weapons - must be between lwMax and wEnemyWeapons!
	//31
    wScript1, wScript2, wScript3, wScript4,
	//35
    wScript5, wScript6, wScript7, wScript8,
	//39
    wScript9, wScript10, wIce, wFlame, //ice rod, fire rod
    wSound, // -Z: sound + defence split == digdogger, sound + one hit kill == pols voice -Z
	wThrown, wPot, //Thrown pot or rock -Z //Just gonna use a single 'wThrown' -Em
	wLit, //Lightning or Electric -Z
	wBombos, wEther, wQuake,// -Z
	wSword180, wSwordLA,
	wBugNet,
    // Enemy weapons
    wEnemyWeapons=128,
    //129
    ewFireball,ewArrow,ewBrang,ewSword,
    ewRock,ewMagic,ewBomb,ewSBomb,
    //137
    ewLitBomb,ewLitSBomb,ewFireTrail,ewFlame,
    ewWind,ewFlame2,ewFlame2Trail,
    //145
    ewIce,ewFireball2,
    wMax
};

enum defWpnSprite
{
	//0
	ws_0,
	wsBeam = 0,
	wsRefBeam = 1,
	wsSword2 = 1,
	wsSword3,
	wsSword4,
	wsBrang,
	wsBrang2,
	wsBrang3,
	wsBomb,
	wsSBomb,
	wsBombblast,
	//10
	wsArrow,
	wsArrow2,
	wsFire,
	wsWind,
	wsBait,
	wsWandHandle,
	wsMagic,
	wsRefFireball,
	wsFireball = 17,
	wsFireball2 = 17,
	wsRock,
	wsEArrow,
	//20
	wsEBeam,
	wsRefMagic,
	wsEMagic = 21,
	wsSpawn,
	wsDeath,
	wsUnused24,
	wsHammer,
	wsHookshotHead,
	wsHookshotChainH, 
	wsHookshotHandle,
	wsFireSparkle, //silver sparkle
	wsSilverSparkle = 29, //silver sparkle
	//30
	wsGoldSparkle,
	wsBrang2Sparkle,
	wsBrang3Sparkle,
	wsHammerSmack,
	wArrow3,
	wsEFire,
	wsEWind,
	wsMagicGauge,
	wsDFireFalling,
	wsDFireRising,
	//40
	wsDFireTrailRising,
	wsDFireTrailFalling,
	wsHookshotChainV,
	wsMore,
	wsUnused44,
	wsUnused45,
	wsSword1Slash,
	wsSword2Slash,
	wsSword3Slash,
	wsSword4Slash,
	//50
	wsShadow,
	wsShadowLarge,
	wsBushLeaves,
	wsFlowerClippings,
	wsGrassGlippings,
	wsTallGrass,
	wsRipples,
	wsUnused57,
	wsNayLoveLeft,
	wsNayLoveLeftReturn,
	//60
	wsNayLoveTrailLeft,
	wsNayLoveTrailLeftReturn,
	wsNayLoveRight,
	wsNayLoveRightReturn,
	wsNayLoveTrailRight,
	wsNayLoveTrailRightReturn,
	wsNayLoveShieldFront,
	wsNayLoveShieldBack,
	wsSubscreenVine,
	wsByrnaCane,
	//70
	wsByrnaSlash,
	wsLongshotHead,
	wsLongshotChainH,
	wsLongshotHandle,
	wsLongshotChainV,
	wsSbombblast,
	wsEBomb,
	wsESbomb,
	wsEBombblast,
	wsESbombblast,
	//80
	wsEFiretrail,
	wsEFire2,
	wsEFiretrail2,
	wsIce,
	wsHoverboots,
	wsMagicFire,
	wsQuarterHeartPieces,
	wsByrnaBeam,
	wsFirework,
	wsLast
};

// phantom weapon types
enum
{
    pDIVINEFIREROCKET, pDIVINEFIREROCKETRETURN, pDIVINEFIREROCKETTRAIL, pDIVINEFIREROCKETTRAILRETURN, pMESSAGEMORE,
    pDIVINEPROTECTIONROCKET1, pDIVINEPROTECTIONROCKETRETURN1, pDIVINEPROTECTIONROCKETTRAIL1, pDIVINEPROTECTIONROCKETTRAILRETURN1,
    pDIVINEPROTECTIONROCKET2, pDIVINEPROTECTIONROCKETRETURN2, pDIVINEPROTECTIONROCKETTRAIL2, pDIVINEPROTECTIONROCKETTRAILRETURN2
};

enum
{
    eNONE=0,
    gNONE=0, gABEI, gAMA, gDUDE, gMOBLIN,
    gFIRE, gFAIRY, gGORIYA, gZELDA, gABEI2,
    //10
    gEMPTY, gDUMMY1, gDUMMY2, gDUMMY3, gDUMMY4,
    gDUMMY5, gDUMMY6, gDUMMY7, gDUMMY8, gDUMMY9,
    //20
    eSTART=20,
    eOCTO1S=20, eOCTO2S, eOCTO1F, eOCTO2F, eTEK1,
    eTEK2, eLEV1, eLEV2, eMOBLIN1, eMOBLIN2,
    //30
    eLYNEL1, eLYNEL2, ePEAHAT, eZORA, eROCK,
    eGHINI1, eGHINI2, eARMOS, eKEESE1, eKEESE2,
    //40
    eKEESE3, eSTALFOS, eGEL, eZOL, eROPE,
    eGORIYA1, eGORIYA2, eTRAP, eWALLM, eDKNUT1,
    //50
    eDKNUT2, eBUBBLEST, eVIRE, eLIKE, eGIBDO,
    ePOLSV, eWIZ1, eWIZ2, eRAQUAM, eMOLDORM,
    //60
    eDODONGO, eMANHAN, eGLEEOK1, eGLEEOK2, eGLEEOK3,
    eGLEEOK4, eDIG1, eDIG3, eDIGPUP1, eDIGPUP2,
    //70
    eDIGPUP3, eDIGPUP4, eGOHMA1, eGOHMA2, eCENT1,
    eCENT2, ePATRA1, ePATRA2, eGANON, eSTALFOS2,
    //80
    eROPE2, eBUBBLESP, eBUBBLESR, eSHOOTFBALL, eITEMFAIRY,
    eFIRE, eOCTO5, eDKNUT5, eGELTRIB, eZOLTRIB,
    //90
    eKEESETRIB, eVIRETRIB, eDKNUT3, eLAQUAM, eMANHAN2,
    eTRAP_H, eTRAP_V, eTRAP_LR, eTRAP_UD, eFWIZ,
    //100
    eWWIZ, eCEILINGM, eFLOORM, ePATRABS, ePATRAL2,
    ePATRAL3, eBAT, eBATROBE, eBATROBEKING, eGLEEOK1F,
    //110
    eGLEEOK2F, eGLEEOK3F, eGLEEOK4F, eMWIZ, eDODONGOBS,
    eDODONGOF, eTRIGGER, eBUBBLEIT, eBUBBLEIP, eBUBBLEIR,
    //120
    eSTALFOS3, eGOHMA3, eGOHMA4, eNPCSTAND1, eNPCSTAND2,
    eNPCSTAND3, eNPCSTAND4, eNPCSTAND5, eNPCSTAND6, eNPCWALK1,
    //130
    eNPCWALK2, eNPCWALK3, eNPCWALK4, eNPCWALK5, eNPCWALK6,
    eBOULDER, eGORIYA3, eLEV3, eOCTO3S, eOCTO3F,
    //140
    eOCTO4S, eOCTO4F, eTRAP_8WAY, eTRAP_DIAGONAL, eTRAP_SLASH_C,
    eTRAP_SLASH_LOS, eTRAP_BACKSLASH_C, eTRAP_BACKSLASH_LOS, eTRAP_CW_C, eTRAP_CW_LOS,
    //150
    eTRAP_CCW_C, eTRAP_CCW_LOS, eSUMMONER, eIWIZ, eSHOOTMAGIC,
    eSHOOTROCK, eSHOOTSPEAR, eSHOOTSWORD, eSHOOTFLAME, eSHOOTFLAME2,
    //160
    eBOMBCHU, eFGEL, eFZOL, eFGELTRIB, eFZOLTRIB,
    eTEK3, eSPINTILE1, eSPINTILE2, eLYNEL3, eFPEAHAT,
    //170
    eMPOLSV, eWPOLSV, eDKNUT4, eFGHINI, eMGHINI,
    eGRAPBUGHP, eGRAPBUGMP, e177,
    
    eMAXGUYS = MAXGUYS
};

#define OLDMAXGUYS	e177

// enemy families
enum
{
    eeGUY=0, eeWALK,
    eeSHOOT/*DEPRECATED*/,
    eeTEK, eeLEV, eePEAHAT, eeZORA, eeROCK,
	//8
    eeGHINI, eeARMOS/*DEPRECATED*/, eeKEESE, eeGEL/*DEPRECATED*/, eeZOL/*DEPRECATED*/, eeROPE/*DEPRECATED*/, eeGORIYA/*DEPRECATED*/, eeTRAP,
	//16
    eeWALLM, eeBUBBLE/*DEPRECATED*/, eeVIRE/*DEPRECATED*/, eeLIKE/*DEPRECATED*/, eePOLSV/*DEPRECATED*/, eeWIZZ, eeAQUA, eeMOLD,
	//24
    eeDONGO, eeMANHAN, eeGLEEOK, eeDIG, eeGHOMA, eeLANM, eePATRA, eeGANON,
	//32
    eePROJECTILE, eeGELTRIB/*DEPRECATED*/, eeZOLTRIB/*DEPRECATED*/, eeVIRETRIB/*DEPRECATED*/, eeKEESETRIB/*DEPRECATED*/, eeSPINTILE, eeNONE,
	//39
    eeFAIRY, eeFIRE, eeOTHER, eeMAX250, //eeFire is Other (Floating), eeOther is Other in the Editor.
    eeSCRIPT01, eeSCRIPT02, eeSCRIPT03, eeSCRIPT04, eeSCRIPT05, eeSCRIPT06, eeSCRIPT07, eeSCRIPT08, eeSCRIPT09, eeSCRIPT10,
    eeSCRIPT11, eeSCRIPT12, eeSCRIPT13, eeSCRIPT14, eeSCRIPT15, eeSCRIPT16, eeSCRIPT17, eeSCRIPT18, eeSCRIPT19, eeSCRIPT20,
    eeFFRIENDLY01, eeFFRIENDLY02, eeFFRIENDLY03, eeFFRIENDLY04, eeFFRIENDLY05, eeFFRIENDLY06, eeFFRIENDLY07, eeFFRIENDLY08,
    eeFFRIENDLY09, eeFFRIENDLY10,
    eeMAX
};

// enemy animation styles
enum
{
    aNONE, aFLIP, aUNUSED1, a2FRM, aUNUSED2,
    aOCTO, aTEK, aLEV, aWALK, aZORA,
    aNEWZORA, aGHINI, aARMOS, aROPE, aWALLM,
    aNEWWALLM, aDWALK, aVIRE, a3FRM, aWIZZ,
    aAQUA, aDONGO, aMANHAN, aGLEEOK, aDIG,
    aGHOMA, aLANM, a2FRMPOS, a4FRM4EYE,a4FRM8EYE,
    a4FRM4DIRF, a4FRM4DIR, a4FRM8DIRF, aARMOS4, a4FRMPOS4DIR,
    a4FRMPOS8DIR, aUNUSED3, a4FRM8DIRB, aNEWTEK, a3FRM4DIR,
    a2FRM4DIR, aNEWLEV, a2FRM4EYE, aNEWWIZZ, aNEWDONGO,
    aDONGOBS, a4FRMPOS8DIRF, a4FRMPOS4DIRF, a4FRMNODIR, aGANON, a2FRMB, 
    a4FRM8EYEB, a4FRM4EYEB, a4FRM8DIRFB, a4FRM4DIRB, a4FRM4DIRFB, aMAX
};
// deprecated styles
#define aFLIPSLOW 1
#define a2FRMSLOW 3
#define aNEWDWALK 42
#define aNEWPOLV 39
#define a4FRM3TRAP 36

// Enemy misc1 types
enum { e1tNORMAL, e1tEACHTILE, e1tCONSTANT, e1tHOMINGBRANG=2, e1tFAST, e1tSLANT, e1t3SHOTS, e1t4SHOTS, e1t5SHOTS, e1t3SHOTSFAST, e1tFIREOCTO, e1t8SHOTS, e1tSUMMON, e1tSUMMONLAYER, e1tLAST };
// Enemy misc2 types
enum { e2tNORMAL, e2tSPLITHIT, e2tSPLIT, e2tFIREOCTO, e2tBOMBCHU, e2tTRIBBLE, e2tLAST };
#define e2tKEESETRIB 1

// Enemy misc7 types
enum { e7tNORMAL, e7tTEMPJINX, e7tPERMJINX, e7tUNJINX, e7tTAKEMAGIC, e7tTAKERUPEES, e7tDRUNK,
// all from this point involve engulfing
       e7tEATITEMS, e7tEATMAGIC, e7tEATRUPEES, e7tEATHURT,
// all from this point involve dragging
       e7tWALLMASTER, e7tLAST
     };

// Enemy misc8 types
enum { e8tSWORD, e8tITEM, e8tALL, e8tLAST };

// Enemy misc9 types
// Walker
enum { e9tNORMAL, e9tROPE, e9tVIRE, e9tPOLSVOICE, e9tARMOS,
// remainder unimplemented
       e9tLEEVER, e9tZ3LEEVER, e9tZ3WALK, e9tZ3STALFOS, e9tLAST
     };

enum { patrat1SHOT, patrat1SHOTFAST, patrat3SHOT, patrat3SHOTFAST, patrat5SHOT, patrat5SHOTFAST, patrat4SHOTCARD, patrat4SHOTDIAG, patrat4SHOTRAND, patrat8SHOT, patratBREATH, patratSTREAM, patratLAST };

/*
// Floater
enum { e9tPEAHAT=1, e9tGHINI };
// Teleporter
enum { e9tBATROBE=1, e9tZ3WIZZROBE };
// Traps
enum { e9tAUTOTRAP=1 };
// Moldorm
enum {  e9tMOLDORM=1, e9tVLANMOLA, e9tVMOLDORM, e9tZ3MOLDORM, //restricted to walkable combos
	e9tTAIL, //only tail is vulnerable
	e9tMINIMOLD, //doesn't segment
	e9tBIGTAIL //doesn't segment, only tail is vulnerable
     };
*/

          //Preparation for new defences. -Z
     
     
enum
{
	edefBRANG, 	edefBOMB, 	edefSBOMB, 	edefARROW, 	edefFIRE, 	//04
	edefWAND, 	edefMAGIC, 	edefHOOKSHOT, 	edefHAMMER, 	edefSWORD, 	//09
	edefBEAM, 	edefREFBEAM, 	edefREFMAGIC,	edefREFBALL, 	edefREFROCK,	//14
	edefSTOMP, 	edefBYRNA, 	edefSCRIPT, 	edefLAST250, 	edefQUAKE, 	//19
	edefSCRIPT01, 	edefSCRIPT02,	edefSCRIPT03,	edefSCRIPT04,	edefSCRIPT05,	//24
	edefSCRIPT06, 	edefSCRIPT07,	edefSCRIPT08,	edefSCRIPT09,	edefSCRIPT10,	//29
	edefICE,	edefBAIT, 	edefWIND,	edefSPARKLE,	edefSONIC,	//34
	edefWhistle,	edefSwitchHook,	edefTHROWN,	edefRES008,	edefRES009,	//39
	edefRES010,	//x40
	edefLAST255 //41
	/*
	edef42,	edefETHER, 	edefBOMBOS,	edefPOT,	edefTHROWNROCK,	//46
	edefELECTRIC,	edefSHIELD,	edefTROWEL,	edefSPINATTK,	edefZ3SWORD,	//51
	edefLASWORD,	//x52
	edefLASTEND  //53*/
    // Reserved for future use.
	 //edefSCRIPT used to be unused edefSPIN

}; 

#define edefLAST 19 
#define edefSCRIPTDEFS_MAX 9 //for 2.future compatibility

//Old 2.future compat rubbish for quest loading. -Z
enum
{
	scriptDEF1, scriptDEF2, scriptDEF3, scriptDEF4, scriptDEF5, scriptDEF6, scriptDEF7,
	scriptDEF8, scriptDEF9, scriptDEF10, scriptDEFLAST
};

//2.50.x last defense, used for filepack loading. 

// New defence outcomes. 
enum
{
    edNORMAL, // : IMPLEMENTED : Take damage (or stun)
    edHALFDAMAGE, // : IMPLEMENTED : Take half damage
    edQUARTDAMAGE, // : IMPLEMENTED : Take 0.25 damage
    edSTUNONLY, // : IMPLEMENTED : Stun instead of taking damage.
    edSTUNORCHINK, // : IMPLEMENTED : If damage > 0, stun instead. Else, bounce off.
    edSTUNORIGNORE, // : IMPLEMENTED : If damage > 0, stun instead. Else, ignore.
    edCHINKL1, // : IMPLEMENTED : Bounces off, plays SFX_CHINK
    edCHINKL2, // : IMPLEMENTED : Bounce off unless damage >= 2
    edCHINKL4, //: IMPLEMENTED : Bounce off unless damage >= 4
    edCHINKL6, // : IMPLEMENTED : Bounce off unless damage >= 6
    edCHINKL8, // : IMPLEMENTED : Bounce off unless damage >= 8
    edCHINK, // : IMPLEMENTED : Bounces off, plays SFX_CHINK
    edIGNOREL1, // : IMPLEMENTED : Ignore unless damage > 1.
    edIGNORE, // : IMPLEMENTED : Do Nothing
    ed1HKO, // : IMPLEMENTED : One-hit knock-out
	edCHINKL10, //: IMPLEMENTED : If damage is less than 10
	ed2x, // : IMPLEMENTED : Double damage.
	ed3x, // : IMPLEMENTED : Triple Damage.
	ed4x, // : IMPLEMENTED : 4x damage.
	edHEAL, // : IMPLEMENTED : Gain the weapon damage in HP.
	edTRIGGERSECRETS, // : IMPLEMENTED : Triggers screen secrets.
	edFREEZE, //Freeze solid
	edMSG_NOT_ENABLED, //A message for 'The following are not yet enabled.
	edMSG_LINE, //An entry for the hiriz line in THE zq PULLDOWN
	edLEVELDAMAGE, //Damage * item level
	edLEVELREDUCTION, //Damage / item level
	
	edSPLIT, //: IMPLEMENTED : causes the enemy to split if it has a split attribute
	edREPLACE, //replaced by next in list?
	edLEVELCHINK2, //If item level is < 2: This needs a weapon variable that is set by 
	edLEVELCHINK3, //If item level is < 3: the item that generates it (itemdata::level stored to
	edLEVELCHINK4, //If item level is < 4: weapon::level, or something similar; then a check to
	edLEVELCHINK5, //If item level is < 5: read weapon::level in hit detection. 
	edSHOCK, //buzz blob
	edEXPLODESMALL, //: IMPLEMENTED : ew bombblast
	edEXPLODELARGE, //: IMPLEMENTED : super bomb blast
	edSTONE, //deadrock
	
	edBREAKSHIELD, //break the enemy shield
	edRESTORESHIELD, //if the enemy had a shield, reset it
	edSPECIALDROP, //but where to define it?
	edINCREASECOUNTER, //but where to define the counter
	edREDUCECOUNTER, //same problem
	edEXPLODEHARMLESS, //: IMPLEMENTED : boss death explosion; needs different sprites?
	edKILLNOSPLIT, //If sufficient damage to kill it, a splitting enemy just dies.
	edTRIBBLE, //Tribbles on hit. 
	edFIREBALL, //Makes a 1x1 fireball; Z3 Gibdo
	edFIREBALLLARGE, //Makes a 3x3  Z3 Gibdo for large npcs. 
	edSUMMON, //: IMPLEMENTED : Summons a number of enemies as defined by the summon properties of the npc. 
	//edSAVE, edRETRY, edCRASHZC // Sanity Check Required. -Z
	edWINGAME, //Wand of Gamelon. 
	edJUMP, //Z3 stalfos
	edEATHERO, //-G //Is this practical? We need specisal npc mvoement for it. -Z
	edSHOWMESSAGE, //Shows a ZString when hit. e.g., Z3 Ganon
	edSWITCH, //Switch places with the player, as a switchhook does
	
    edLAST
};
#define edX edIGNORE // Shorthand
// Remaining 16 reserved for future use.


// enemy patters
enum { pRANDOM, pSIDES, pSIDESR, pCEILING, pCEILINGR, pRANDOMR, pNOSPAWN };

enum { tfInvalid=0, tf4Bit, tf8Bit, tf16Bit, tf24Bit, tf32Bit, tfMax };

struct size_and_pos
{
	int x = -1, y = -1;
	int w = -1, h = -1;
	int xscale = 1, yscale = 1;
	int fw = -1, fh = -1;
	
	int data[8] = {0};
	
	//Get virtual values
	int tw() const;
	int th() const;
	int cx() const;
	int cy() const;
	
	void clear(); //Clear to default vals
	
	bool rect(int mx, int my) const; //Check rect collision
	int rectind(int mx, int my) const; //Check scaled collision
	
	//Set coord values
	void set(int nx, int ny, int nw, int nh);
	void set(int nx, int ny, int nw, int nh, int xs, int ys);
	
	size_and_pos const& subsquare(int ind) const;
	size_and_pos const& subsquare(int col, int row) const;
	size_and_pos const& rel_subsquare(int x, int y, int ind) const;
	size_and_pos const& rel_subsquare(int x, int y, int col, int row) const;
	size_and_pos(int nx = -1, int ny = -1, int nw = -1, int nh = -1, int xsc = 1, int ysc = 1, int fw = -1, int fh = -1);
};

#define HOTKEY_FLAG_FILTER (KB_SHIFT_FLAG|KB_CTRL_FLAG|KB_ALT_FLAG)
struct Hotkey
{
	int modflag[2];
	int hotkey[2];
	#undef check
	bool check(int k,int shifts,bool exact=false);
	int getval() const;
	void setval(int val);
	void setval(int ind,int k,int shifts);
	void setval(int k,int shifts,int k2,int shifts2);
	std::string get_name(int ind);
	bool operator==(Hotkey const& other);
	bool operator!=(Hotkey const& other);
};
std::string get_keystr(int key);
bool is_modkey(int c);
int get_mods(int mask = HOTKEY_FLAG_FILTER);

//#define OLDITEMCNT i90
//#define OLDWPNCNT  w84
#define ITEMCNT   iMax
#define WPNCNT    wMAX

struct tiledata
{
    byte format;
    byte *data;
};


//Weapon editor

//Weapon clocks
enum
{ 
	wpnclkFRAMECOUNT, wpnclkMAGICUSAGE = 9 
};



//Weapon Types
enum
{
	weaptypeNONE, weaptypeSWORD, weaptypeSWORDBEAM, weaptypeBRANG, weaptypeBOMBBLAST,
	weaptypeSBOMBBLAST, weaptypeBOMB, weaptypeSBOMB, weaptypeARROW, weaptypeFIRE,
	weaptypeWHISTLE, weaptypeBAIT, weaptypeWAND, weaptypeMAGIC, weaptypeCANDLE,
	weaptypeWIND, weaptypeREFMAGIC, weaptypeREFFIREBALL, weaptypeREFROCK, weaptypeHAMMER,
	weaptypeHOOKSHOT, weaptype21, weaptype22, weaptypeSPARKLE, weaptype24,
	weaptype25, weaptypeBYRNA, weaptypeREFBEAM, weaptype28, weaptype29,
	weaptypeSCRIPT1, weaptypeSCRIPT2, weaptypeSCRIPT3, weaptypeSCRIPT4, weaptypeSCRIPT5,
	weaptypeSCRIPT6, weaptypeSCRIPT7, weaptypeSCRIPT8, weaptypeSCRIPT9, weaptypeSCRIPT10
};

//Defence types
enum
{
	weapdefNONE, weapdefSWORD, weapdefSWORDBEAM, weapdefBRANG, weapdefBOMBBLAST,
	weapdefSBOMBBLAST, weapdefBOMB, weapdefSBOMB, weapdefARROW, weapdefFIRE,
	weapdefWHISTLE, weapdefBAIT, weapdefWAND, weapdefMAGIC, weapdefCANDLE,
	weapdefWIND, weapdefREFMAGIC, weapdefREFFIREBALL, weapdefREFROCK, weapdefHAMMER,
	weapdefHOOKSHOT, weapdef21, weapdef22, weapdefSPARKLE, weapdef24,
	weapdef25, weapdefBYRNA, weapdefREFBEAM, weapdef28, weapdef29,
	weapdefSCRIPT1, weapdefSCRIPT2, weapdefSCRIPT3, weapdefSCRIPT4, weapdefSCRIPT5,
	weapdefSCRIPT6, weapdefSCRIPT7, weapdefSCRIPT8, weapdefSCRIPT9, weapdefSCRIPT10 
};
	

#define ITEM_MOVEMENT_PATTERNS 10

	//Move pattern array indices
enum{
	wpnmovePATTERN, wmoveARG1, wmoveARG2, wmoveARG3, wmoveARG4, wmoveDUPLICATES, wmoveOTHER1
};
	

//Movement patterns
enum
{
	wmovepatternNONE, wmovepatternLINE, wmovepatternSINE, wmovepatternCOSINE, wmovepatternCIRCLE, wmovepatternARC, wmovepatternPATTERN_A, 
	wmovepatternPATTERN_B, wmovepatternPATTERN_C, wmovepatternPATTERN_D, wmovepatternPATTERN_E, wmovepatternPATTERN_F
};

struct wpndata
{
	int32_t tile;
	byte misc;                                                // 000bvhff (vh:flipping, f:flash (1:NES, 2:BSZ))
	byte csets;                                               // ffffcccc (f:flash cset, c:cset)
	byte frames;                                              // animation frame count
	byte speed;                                               // animation speed
	byte type;                                                // used by certain weapons
	word script;
};

#define WF_AUTOFLASH  0x01
#define WF_2PFLASH    0x02
#define WF_HFLIP      0x04
#define WF_VFLIP      0x08
#define WF_BEHIND     0x10 //Weapon renders behind other sprites

struct quest_template
{
    char name[31];
    char path[2048];
    //311 bytes
};

struct item_drop_object
{
    char name[64];
    word item[10];
    word chance[11]; //0=none
};

#define guy_bhit        0x00000001
#define guy_invisible   0x00000002
#define guy_neverret    0x00000004
#define guy_doesntcount 0x00000008

#define guy_fadeflicker 0x00000010
#define guy_fadeinstant 0x00000020
/*
#define inv_bomb        0x00000040
#define inv_sbomb       0x00000080

#define inv_arrow       0x00000100
#define inv_L2arrow     0x00000200
#define inv_fire        0x00000400
#define inv_wand        0x00000800

#define inv_magic       0x00001000
#define inv_hookshot    0x00002000
#define inv_hammer      0x00004000
#define inv_L3brang     0x00008000

#define inv_L1sword     0x00010000
#define inv_L3sword     0x00020000
#define inv_L1beam      0x00040000
#define inv_L3beam      0x00080000

#define inv_refbeam     0x00100000
#define inv_refmagic    0x00200000
#define inv_refball     0x00400000
#define inv_extra       0x00800000
*/
#define inv_front       0x01000000
#define inv_left        0x02000000
#define inv_right       0x04000000
#define inv_back        0x08000000

#define guy_bkshield    0x10000000 // Shield can't be broken
//#define guy_mirror      0x20000000 // Shield is mirrored
//#define weak_L3brang    0x40000000

#define lens_only       0x80000000

#define guy_flashing    0x00000001
#define eneflag_zora    0x00000002
#define eneflag_rock    0x00000004
#define eneflag_trap    0x00000008

#define cmbflag_trph    0x00000010
#define cmbflag_trpv    0x00000020
#define cmbflag_trp4    0x00000040
#define cmbflag_trplr   0x00000080

#define cmbflag_trpud   0x00000100
#define eneflag_trp2    0x00000200
#define eneflag_fire    0x00000400
#define cmbflag_armos   0x00000800

#define cmbflag_ghini   0x00001000
#define eneflag_ganon   0x00002000
#define guy_blinking    0x00004000
#define guy_transparent 0x00008000

#define guy_ignoretmpnr 0x00010000

// Old flags
#define weak_arrow		0x20000000
#define guy_superman    0x00000008
#define guy_sbombonly   0x00000010

//FF combo flags

#define ffOVERLAY       0x00000001
#define ffTRANS         0x00000002
#define ffSOLID         0x00000004
#define ffCARRYOVER     0x00000008
#define ffSTATIONARY    0x00000010
#define ffCHANGER       0x00000020 //Is a changer
#define ffPRELOAD       0x00000040 //Script is run before screen appears.
#define ffLENSVIS       0x00000080 //Invisible, but not to the Lens of Truth.
#define ffSCRIPTRESET	0x00000100 //Script resets when carried over.
#define ffETHEREAL      0x00000200 //Does not occlude combo and flags on the screen
#define ffIGNOREHOLDUP  0x00000400 //Updated even while Hero is holding an item
#define ffIGNORECHANGER  0x00000800 //Ignore changers
#define ffIMPRECISIONCHANGER  0x00001000 //Ignore changers
#define ffLENSINVIS		0x00002000 //Visible, but not to the Lens of Truth

//FF combo changer flags

#define ffSWAPNEXT      0x80000000 //Swap speed with next FFC
#define ffSWAPPREV      0x40000000 //Swap speed with prev. FFC
#define ffCHANGENEXT    0x20000000 //Increase combo ID
#define ffCHANGEPREV    0x10000000 //Decrease combo ID
#define ffCHANGETHIS    0x08000000 //Change combo/cset to this
#define ffCHANGESPEED   0x04000000 //Change speed to this (default, not implemented yet)


//Guydata Enemy Editor Size Panel FLags
#define guyflagOVERRIDE_TILE_WIDTH	0x00000001
#define guyflagOVERRIDE_TILE_HEIGHT	0x00000002
#define guyflagOVERRIDE_HIT_WIDTH	0x00000004
#define guyflagOVERRIDE_HIT_HEIGHT	0x00000008
#define guyflagOVERRIDE_HIT_Z_HEIGHT	0x00000010
#define guyflagOVERRIDE_HIT_X_OFFSET	0x00000020
#define guyflagOVERRIDE_HIT_Y_OFFSET	0x00000040
#define guyflagOVERRIDE_DRAW_X_OFFSET	0x00000080
#define guyflagOVERRIDE_DRAW_Y_OFFSET	0x00000100
#define guyflagOVERRIDE_DRAW_Z_OFFSET	0x00000200

#define MAX_NPC_ATRIBUTES 31


struct guydata
{
    dword flags;
    dword flags2;
    int32_t  tile;
    byte  width;
    byte  height; //0=striped, 1+=rectangular
    int32_t  s_tile; //secondary (additional) tile(s)
    byte  s_width;
    byte  s_height;  //0=striped, 1+=rectangular
    int32_t  e_tile;
    byte  e_width;
    byte  e_height;
    
    int16_t hp;
    
    int16_t  family, cset, anim, e_anim, frate, e_frate;
    int16_t  dp, wdp, weapon;
    
    int16_t  rate, hrate, step, homing, grumble, item_set;
    int32_t   misc1, misc2, misc3, misc4, misc5, misc6, misc7, misc8, misc9, misc10, misc11, misc12, misc13, misc14, misc15;
    int16_t  bgsfx, bosspal, extend;
    byte defense[edefLAST255];
   // byte scriptdefense[
    //  int16_t  startx, starty;
    //  int16_t  foo1,foo2,foo3,foo4,foo5,foo6;
    byte  hitsfx, deadsfx;
    //Add all new guydata variables after this point, if you do not want to edit defdata to fit.
    //Adding earlier will offset defdata arrays. -Z
    
    //2.6 enemy editor tile and hit sizes. -Z
    int32_t xofs,yofs,zofs; //saved to the packfile, so I am using int32_t. I can typecast to fix and back in the functions. 
    // no hzofs - it's always equal to zofs.
    int32_t hxofs,hyofs,hxsz,hysz,hzsz;
    int32_t txsz,tysz;
    byte scriptdefense[scriptDEFLAST]; //old 2.future quest file crossover support. 
    int32_t wpnsprite; //wpnsprite is new for 2.6 -Z
    int32_t SIZEflags;; //Flags for size panel offsets. The user must enable these to override defaults. 
    int32_t frozentile, frozencset, frozenclock;
    int16_t frozenmisc[10];
    //v 34
    int16_t firesfx; //weapon fire (attack) sound
    //expanded ->Attributes[] to size of 32.
    int32_t misc16, misc17, misc18, misc19, misc20, misc21, misc22, misc23, 
	misc24, misc25, misc26, misc27, misc28, misc29, misc30, misc31, misc32;
    int32_t movement[32]; //Reserved for npc movement types and args. 
    int32_t new_weapon[32]; //Reserved for weapon patterns and args.
    int32_t initD[8], initA[2];
    
    word script; //For future npc action scripts. 
    //int16_t parentCore; //Probably not needed here. -Z
    int32_t editorflags;
	dword moveflags;
    
    char initD_label[8][65];
    char weapon_initD_label[8][65];
    
    word weaponscript;
    int32_t weap_initiald[INITIAL_D];
    byte weap_initiala[INITIAL_A];
    
	byte spr_shadow, spr_death, spr_spawn;
	
#define ENEMY_FLAG1   0x01
#define ENEMY_FLAG2   0x02
#define ENEMY_FLAG3     0x04
#define ENEMY_FLAG4     0x08
#define ENEMY_FLAG5     0x10
#define ENEMY_FLAG6     0x20
#define ENEMY_FLAG7     0x40
#define ENEMY_FLAG8     0x80
#define ENEMY_FLAG9     0x100
#define ENEMY_FLAG10     0x200
#define ENEMY_FLAG11     0x400
#define ENEMY_FLAG12     0x800
#define ENEMY_FLAG13     0x1000
#define ENEMY_FLAG14     0x2000
#define ENEMY_FLAG15     0x4000
#define ENEMY_FLAG16     0x8000
    
};
//Moveflags
#define FLAG_OBEYS_GRAV               0x00000001
#define FLAG_CAN_PITFALL              0x00000002
#define FLAG_CAN_PITWALK              0x00000004
#define FLAG_CAN_WATERDROWN           0x00000008
#define FLAG_CAN_WATERWALK            0x00000010
#define FLAG_ONLY_WATERWALK           0x00000020 //Only walks on water
#define FLAG_ONLY_SHALLOW_WATERWALK   0x00000040 //Only walks on shallow water
#define FLAG_ONLY_PITWALK             0x00000080 //Only walks on pitfalls
#define FLAG_NO_FAKE_Z                0x00000100
#define FLAG_NO_REAL_Z                0x00000200
#define FLAG_USE_FAKE_Z               0x00000400
#define FLAG_IGNORE_SOLIDITY          0x00000800
#define FLAG_IGNORE_BLOCKFLAGS        0x00001000
#define FLAG_IGNORE_SCREENEDGE        0x00002000
#define FLAG_USE_NEW_MOVEMENT         0x00004000
#define FLAG_NOT_PUSHABLE             0x00008000

#define LIFTFL_DIS_SHIELD             0x00000001
#define LIFTFL_DIS_ITEMS              0x00000002
#define NUM_LIFTFL 2

#define MAX_PC dword(-1)
class refInfo
{
public:
	//word script; //script number
	dword pc; //current command offset
	
	int32_t d[8]; //d registers
	int32_t a[2]; //a regsisters (reference to another ffc on screen)
	uint32_t sp; //stack pointer for current script
	dword scriptflag; //stores whether various operations were true/false etc.
	
	uint16_t ffcref;
	int32_t idata;
	dword itemref, guyref, lwpn, ewpn;
	dword screenref, npcdataref, bitmapref, spritesref, dmapsref, zmsgref, shopsref, untypedref;
	int32_t mapsref;
	//to implement
	dword dropsetref, pondref, warpringref, doorsref, zcoloursref, rgbref, paletteref, palcycleref, tunesref;
	dword gamedataref, cheatsref; 
	dword fileref, subscreenref, comboidref, directoryref, rngref, stackref, paldataref;
	dword bottletyperef, bottleshopref, genericdataref;
	int32_t combosref, comboposref;
	int32_t portalref, saveportalref;
	//byte ewpnclass, lwpnclass, guyclass; //Not implemented
	
	//byte ewpnclass, lwpnclass, guyclass; //Not implemented
	
	int32_t switchkey; //used for switch statements
	dword thiskey, thiskey2; //used for user class 'this' pointers
	dword waitframes; //wait multiple frames in a row
	dword wait_index; // nth WaitX instruction (0 being pc 0) last execution stopped at. for jit only
	
	void Clear()
	{
		*this = refInfo();
	}
};


//Build date info
extern const int BUILDTM_YEAR;
extern const int BUILDTM_MONTH;
extern const int BUILDTM_DAY;
extern const int BUILDTM_HOUR;
extern const int BUILDTM_MINUTE;
extern const int BUILDTM_SECOND;

// The version of the ZASM engine a script was compiled for
// NOT the same as V_FFSCRIPT, which is the version of the packfile format
// where the scripts are serialized
#define ZASM_VERSION        3

enum class ScriptType {
	None,
	Global,
	FFC,
	Screen,
	Player,
	Item,
	Lwpn,
	NPC,
	Subscreen,
	Ewpn,
	DMap,
	ItemSprite,
	ActiveSubscreen,
	PassiveSubscreen,
	Combo,
	OnMap,
	Generic,
	GenericFrozen,
	First = Global,
	Last = GenericFrozen,
};
const char* ScriptTypeToString(ScriptType type);

#define ZMETA_AUTOGEN		0x01
#define ZMETA_DISASSEMBLED	0x02
#define ZMETA_IMPORTED		0x04

#define SCRIPT_FORMAT_DEFAULT		0
#define SCRIPT_FORMAT_INVALID		1
#define SCRIPT_FORMAT_DISASSEMBLED	2
#define SCRIPT_FORMAT_ZASM			3

#define METADATA_V			5
#define V_COMPILER_FIRST	BUILDTM_YEAR
#define V_COMPILER_SECOND	BUILDTM_MONTH
#define V_COMPILER_THIRD	BUILDTM_DAY
#define V_COMPILER_FOURTH	BUILDTM_HOUR
#define ZMETA_NULL_TYPE		1
struct zasm_meta
{
	word zasm_v;
	word meta_v;
	word ffscript_v;
	ScriptType script_type;
	std::string run_idens[8];
	byte run_types[8];
	byte flags;
	word compiler_v1, compiler_v2, compiler_v3, compiler_v4;
	std::string script_name;
	std::string author;
	std::string attributes[10];
	std::string attribytes[8];
	std::string attrishorts[8];
	std::string usrflags[16];
	std::string attributes_help[10];
	std::string attribytes_help[8];
	std::string attrishorts_help[8];
	std::string usrflags_help[16];
	std::string initd[8];
	std::string initd_help[8];
	int8_t initd_type[8];
	
	void setFlag(byte flag)
	{
		switch(flag)
		{
			case ZMETA_DISASSEMBLED:
				flags &= ~ZMETA_IMPORTED;
				flags |= ZMETA_DISASSEMBLED;
				break;
			case ZMETA_IMPORTED:
				flags &= ~ZMETA_DISASSEMBLED;
				flags |= ZMETA_IMPORTED;
				break;
			default:
				flags |= flag;
		}
	}
	bool valid() const
	{
		return zasm_v >= 2 && meta_v >= 1 && ffscript_v >= 16;
	}
	void zero()
	{
		zasm_v = 0;
		meta_v = 0;
		ffscript_v = 0;
		script_type = ScriptType::None;
		flags = 0;
		compiler_v1 = 0;
		compiler_v2 = 0;
		compiler_v3 = 0;
		compiler_v4 = 0;
		for(int32_t q = 0; q < 16; ++q)
		{
			usrflags[q].clear();
			usrflags_help[q].clear();
			if(q > 9) continue;
			attributes[q].clear();
			attributes_help[q].clear();
			if(q > 7) continue;
			initd[q].clear();
			initd_help[q].clear();
			initd_type[q] = -1;
			run_idens[q].clear();
			run_types[q] = ZMETA_NULL_TYPE;
			attribytes[q].clear();
			attribytes_help[q].clear();
			attrishorts[q].clear();
			attrishorts_help[q].clear();
		}
		script_name.clear();
		author.clear();
	}
	void autogen(bool clears = true)
	{
		if(clears) zero();
		zasm_v = ZASM_VERSION;
		meta_v = METADATA_V;
		ffscript_v = V_FFSCRIPT;
		flags = ZMETA_AUTOGEN;
		compiler_v1 = V_COMPILER_FIRST;
		compiler_v2 = V_COMPILER_SECOND;
		compiler_v3 = V_COMPILER_THIRD;
		compiler_v4 = V_COMPILER_FOURTH;
	}
	zasm_meta()
	{
		zero();
	}
	~zasm_meta()
	{
		
	}
	zasm_meta& operator=(zasm_meta const& other)
	{
		zasm_v = other.zasm_v;
		meta_v = other.meta_v;
		ffscript_v = other.ffscript_v;
		script_type = other.script_type;
		for(auto q = 0; q < 16; ++q)
		{
			usrflags[q] = other.usrflags[q];
			usrflags_help[q] = other.usrflags_help[q];
			if(q > 9) continue;
			attributes[q] = other.attributes[q];
			attributes_help[q] = other.attributes_help[q];
			if(q > 7) continue;
			initd[q] = other.initd[q];
			initd_help[q] = other.initd_help[q];
			initd_type[q] = other.initd_type[q];
			run_idens[q] = other.run_idens[q];
			run_types[q] = other.run_types[q];
			attribytes[q] = other.attribytes[q];
			attribytes_help[q] = other.attribytes_help[q];
			attrishorts[q] = other.attrishorts[q];
			attrishorts_help[q] = other.attrishorts_help[q];
			if(q > 3) continue;
		}
		flags = other.flags;
		compiler_v1 = other.compiler_v1;
		compiler_v2 = other.compiler_v2;
		compiler_v3 = other.compiler_v3;
		compiler_v4 = other.compiler_v4;
		script_name = other.script_name;
		author = other.author;
		return *this;
	}
	bool operator==(zasm_meta const& other) const
	{
		if(zasm_v != other.zasm_v) return false;
		if(meta_v != other.meta_v) return false;
		if(ffscript_v != other.ffscript_v) return false;
		if(script_type != other.script_type) return false;
		if(flags != other.flags) return false;
		if(compiler_v1 != other.compiler_v1) return false;
		if(compiler_v2 != other.compiler_v2) return false;
		if(compiler_v3 != other.compiler_v3) return false;
		if(compiler_v4 != other.compiler_v4) return false;
		for(auto q = 0; q < 16; ++q)
		{
			if(usrflags[q].compare(other.usrflags[q]))
				return false;
			if(usrflags_help[q].compare(other.usrflags_help[q]))
				return false;
			if(q > 9) continue;
			if(attributes[q].compare(other.attributes[q]))
				return false;
			if(attributes_help[q].compare(other.attributes_help[q]))
				return false;
			if(q > 7) continue;
			if(initd[q].compare(other.initd[q]))
				return false;
			if(initd_help[q].compare(other.initd_help[q]))
				return false;
			if(initd_type[q] != other.initd_type[q])
				return false;
			if(run_idens[q].compare(other.run_idens[q]))
				return false;
			if(run_types[q] != other.run_types[q])
				return false;
			if(attribytes[q].compare(other.attribytes[q]))
				return false;
			if(attribytes_help[q].compare(other.attribytes_help[q]))
				return false;
			if(attrishorts[q].compare(other.attrishorts[q]))
				return false;
			if(attrishorts_help[q].compare(other.attrishorts_help[q]))
				return false;
		}
		if(script_name.compare(other.script_name))
			return false;
		if(author.compare(other.author))
			return false;
		return true;
	}
	bool operator!=(zasm_meta const& other) const
	{
		return !(*this == other);
	}
	
	bool parse_meta(const char *buffer);
	std::string get_meta() const;
};
ScriptType get_script_type(std::string const& name);
std::string get_script_name(ScriptType type);

struct ffscript
{
    word command;
    int32_t arg1;
    int32_t arg2;
	std::vector<int32_t> *vecptr;
	std::string *strptr;
	ffscript()
	{
		command = 0xFFFF;
		arg1 = 0;
		arg2 = 0;
		vecptr = nullptr;
		strptr = nullptr;
	}
	~ffscript()
	{
		if(vecptr)
		{
			delete vecptr;
			vecptr = nullptr;
		}
		if(strptr)
		{
			delete strptr;
			strptr = nullptr;
		}
	}
	void give(ffscript& other)
	{
		other.command = command;
		other.arg1 = arg1;
		other.arg2 = arg2;
		other.vecptr = vecptr;
		other.strptr = strptr;
		vecptr = nullptr;
		strptr = nullptr;
		clear();
	}
	void clear()
	{
		command = 0xFFFF;
		arg1 = 0;
		arg2 = 0;
		if(vecptr)
		{
			delete vecptr;
			vecptr = nullptr;
		}
		if(strptr)
		{
			delete strptr;
			strptr = nullptr;
		}
	}
	void copy(ffscript& other)
	{
		other.clear();
		other.command = command;
		other.arg1 = arg1;
		other.arg2 = arg2;
		if(vecptr)
		{
			other.vecptr = new std::vector<int32_t>();
			for(int32_t val : *vecptr)
				other.vecptr->push_back(val);
		}
		if(strptr)
		{
			other.strptr = new std::string();
			for(char c : *strptr)
				other.strptr->push_back(c);
		}
	}
	
	bool operator==(ffscript const& other) const
	{
		//Compare primitive members
		if(command != other.command) return false;
		if(arg1 != other.arg1) return false;
		if(arg2 != other.arg2) return false;
		//Check for pointer existence differences
		if((vecptr==nullptr)!=(other.vecptr==nullptr)) return false;
		if((strptr==nullptr)!=(other.strptr==nullptr)) return false;
		//If both have a pointer, compare pointer size/contents
		if(vecptr)
		{
			if(vecptr->size() != other.vecptr->size())
				return false;
			if((*vecptr) != (*other.vecptr))
				return false;
		}
		if(strptr)
		{
			if(strptr->size() != other.strptr->size())
				return false;
			if(strptr->compare(*other.strptr))
				return false;
		}
		return true;
	}
	bool operator!=(ffscript const& other) const
	{
		return !(*this == other);
	}
};

extern int next_script_data_debug_id;

struct script_data
{
	ffscript* zasm;
	zasm_meta meta;
	int debug_id;
	
	void null_script()
	{
		if(zasm)
			delete[] zasm;
		zasm = new ffscript[1];
		zasm[0].clear();
	}
	
	bool valid() const
	{
		return (zasm && zasm[0].command != 0xFFFF);
	}
	
	void disable()
	{
		if(zasm)
			zasm[0].clear();
	}
	
	uint32_t size() const
	{
		if(zasm)
		{
			for(uint32_t q = 0;;++q)
			{
				if(zasm[q].command == 0xFFFF)
					return q+1;
			}
		}
		return 0;
	}
	
	void set(script_data const& other)
	{
		if(zasm)
			delete[] zasm;
		if(other.size())
		{
            zasm = new ffscript[other.size()];
			for(size_t q = 0; q < other.size(); ++q)
			{
				other.zasm[q].copy(zasm[q]);
			}
		}
		else
		{
			zasm = NULL;
			null_script();
		}
		meta = other.meta;
	}
	
	script_data(int32_t cmds) : zasm(NULL)
	{
		debug_id = next_script_data_debug_id++;
		if(cmds > 0)
		{
			zasm = new ffscript[cmds];
			for(int32_t q = 0; q < cmds; ++q)
				zasm[q].clear();
		}
		else
			null_script();
	}
	
	script_data() : zasm(NULL)
	{
		debug_id = next_script_data_debug_id++;
		null_script();
	}
	
	script_data(script_data const& other) : zasm(NULL)
	{
		set(other);
	}
	
	~script_data()
	{
		if(zasm)
			delete[] zasm;
	}
	
	void transfer(script_data& other)
	{
		other.meta = meta;
		if(other.zasm)
			delete[] other.zasm;
		other.zasm = zasm;
		zasm = NULL;
		null_script();
	}
	
	script_data& operator=(script_data const& other)
	{
		set(other);
		return *this;
	}
	
	bool equal_zasm(script_data const& other) const
	{
		if(valid() != other.valid()) return false;
		auto sz = size();
		auto othersz = other.size();
		if(sz != othersz) return false;
		for(auto q = 0; q < sz; ++q)
		{
			if(zasm[q] != other.zasm[q]) return false;
		}
		return true;
	}
	
	bool operator==(script_data const& other) const
	{
		if(meta != other.meta) return false;
		return equal_zasm(other);
	}
	
	bool operator!=(script_data const& other) const
	{
		return !(*this == other);
	}
	
};

struct script_command
{
    char name[64];
    byte args;
    byte arg1_type; //0=reg, 1=val;
    byte arg2_type; //0=reg, 1=val;
    byte arr_type; //0x1 = string, 0x2 = array
};

struct script_variable
{
    char name[64];
    int32_t id;
    word maxcount;
    byte multiple;
};

//Sprite boundary array indices
enum
{
	spriteremovalY1, spriteremovalY2, spriteremovalX1, spriteremovalX2, spriteremovalZ1, spriteremovalZ2
};


enum
{
    sBCANDLE, sARROW, sBOMB, sSTAIRS, sSECRET01, sSECRET02, sSECRET03,
    sSECRET04, sSECRET05, sSECRET06, sSECRET07, sSECRET08, sSECRET09,
    sSECRET10, sSECRET11, sSECRET12, sSECRET13, sSECRET14, sSECRET15,
    sSECRET16, sRCANDLE, sWANDFIRE, sDIVINEFIRE, sSARROW, sGARROW,
    sSBOMB, sBRANG, sMBRANG, sFBRANG, sWANDMAGIC, sREFMAGIC, sREFFIREBALL,
    sSWORD, sWSWORD, sMSWORD, sXSWORD, sSWORDBEAM, sWSWORDBEAM,
    sMSWORDBEAM, sXSWORDBEAM, sHOOKSHOT, sWAND, sHAMMER, sSTRIKE, sSECNEXT
};

int combo_trigger_flag_to_secret_combo_index(int flag);

struct comboclass
{
    char  name[64];                       //  name
    byte  block_enemies;                  //  a (bit flags, 1=ground, 2=jump, 4=fly)
    byte  block_hole;                     //  b
    byte  block_trigger;                  //  c
    byte  block_weapon[32];               //  d (ID of lweapons/eweapons to block)
    int16_t conveyor_x_speed;               //  e
    int16_t conveyor_y_speed;               //  f
    word  create_enemy;                   //  g
    byte  create_enemy_when;              //  h
    int32_t  create_enemy_change;            //  i
    byte  directional_change_type;        //  j
    int32_t  distance_change_tiles;          //  k
    int16_t dive_item;                      //  l
    byte  dock;                           //  m
    byte  fairy;                          //  n
    byte  ff_combo_attr_change;           //  o
    int32_t  foot_decorations_tile;          //  p
    byte  foot_decorations_type;          //  q
    byte  hookshot_grab_point;            //  r
    byte  ladder_pass;                    //  s
    byte  lock_block_type;                //  t
    int32_t  lock_block_change;              //  u
    byte  magic_mirror_type;              //  v
    int16_t modify_hp_amount;               //  w
    byte  modify_hp_delay;                //  x
    byte  modify_hp_type;                 //  y
    int16_t modify_mp_amount;               //  z
    byte  modify_mp_delay;                // aa
    byte  modify_mp_type;                 // ab
    byte  no_push_blocks;                 // ac
    byte  overhead;                       // ad
    byte  place_enemy;                    // ae
    byte  push_direction;                 // af
    byte  push_weight;                    // ag
    byte  push_wait;                      // ah
    byte  pushed;                         // ai
    byte  raft;                           // aj
    byte  reset_room;                     // ak
    byte  save_point_type;                // al
    byte  screen_freeze_type;             // am
    byte  secret_combo;                   // an
    byte  singular;                       // ao
    byte  slow_movement;                  // ap
    byte  statue_type;                    // aq
    byte  step_type;                      // ar
    int32_t  step_change_to;                 // as
    byte  strike_weapons[32];             // at
    int32_t  strike_remnants;                // au
    byte  strike_remnants_type;           // av
    int32_t  strike_change;                  // aw
    int16_t strike_item;                    // ax
    int16_t touch_item;                     // ay
    byte  touch_stairs;                   // az
    byte  trigger_type;                   // ba
    byte  trigger_sensitive;              // bb
    byte  warp_type;                      // bc
    byte  warp_sensitive;                 // bd
    byte  warp_direct;                    // be
    byte  warp_location;                  // bf
    byte  water;                          // bg
    byte  whistle;                        // bh
    byte  win_game;                       // bi
    byte  block_weapon_lvl;               // bj (maximum item level to block)
	byte pit;                             // bk
};

enum {cfOFFSET, cfMAX};

#define NUM_COMBO_ATTRIBUTES 4

///user flags
#define cflag1 0x01
#define cflag2 0x02
#define cflag3 0x04
#define cflag4 0x08
#define cflag5 0x010
#define cflag6 0x020
#define cflag7 0x040
#define cflag8 0x080
#define cflag9 0x0100
#define cflag10 0x0200
#define cflag11 0x0400
#define cflag12 0x0800
#define cflag13 0x01000
#define cflag14 0x02000
#define cflag15 0x04000
#define cflag16 0x08000

struct newcombo
{
    int32_t tile;
    byte flip;
    byte walk = 0xF0;
    byte type;
    byte csets;
    byte frames;
    byte speed;
    word nextcombo;
    byte nextcset;
    byte flag;
    byte skipanim;
    word nexttimer;
    byte skipanimy;
    byte animflags;
    byte expansion[6];
	int32_t attributes[NUM_COMBO_ATTRIBUTES]; // combodata->Attributes[] and Screen->GetComboAttribute(pos, indx) / SetComboAttribute(pos, indx)
	int32_t usrflags; // combodata->Flags and Screen->ComboFlags[pos]
	int16_t genflags; // general flags
	int32_t triggerflags[6];
	int32_t triggerlevel;
	byte triggerbtn;
	byte triggeritem;
	byte trigtimer;
	byte trigsfx;
	int32_t trigchange;
	word trigprox;
	byte trigctr;
	int32_t trigctramnt;
	byte triglbeam;
	int8_t trigcschange;
	int16_t spawnitem;
	int16_t spawnenemy;
	int8_t exstate = -1;
	int32_t spawnip;
	byte trigcopycat;
	byte trigcooldown;
	byte trig_lstate, trig_gstate;
	int32_t trig_statetime;
	word trig_genscr;
	byte trig_group;
	word trig_group_val;
	byte liftflags;
	byte liftlvl;
	byte liftsfx;
	byte liftitm;
	byte liftgfx;
	word liftcmb, liftundercmb;
	byte liftcs, liftundercs;
	byte liftsprite;
	byte liftdmg;
	int16_t liftbreaksprite = -1;
	byte liftbreaksfx;
	byte lifthei = 8;
	byte lifttime = 16;
	byte lift_parent_item;
	word prompt_cid;
	byte prompt_cs;
	int16_t prompt_x = 12;
	int16_t prompt_y = -8;
	std::string label;
	byte attribytes[8];
	int16_t attrishorts[8];
	word script;
	int32_t initd[2];
	int32_t o_tile;
	byte cur_frame;
	byte aclk;
	byte speed_mult = 1;
	byte speed_div = 1;
	zfix speed_add;
	
	void set_tile(int32_t newtile)
	{
		o_tile = newtile;
		tile = newtile;
	}
	
	void clear()
	{
		*this = newcombo();
	}
	
	bool is_blank(bool ignoreEff = false)
	{
		if(tile) return false;
		if(flip) return false;
		if(walk&0xF) return false;
		if(!ignoreEff && (walk&0xF0)!=0xF0) return false;
		if(type) return false;
		if(csets) return false;
		if(frames) return false;
		if(speed) return false;
		if(nextcombo) return false;
		if(nextcset) return false;
		if(flag) return false;
		if(skipanim) return false;
		if(nexttimer) return false;
		if(skipanimy) return false;
		if(animflags) return false;
		for(auto q = 0; q < 6; ++q)
			if(expansion[q]) return false;
		for(auto q = 0; q < NUM_COMBO_ATTRIBUTES; ++q)
			if(attributes[q]) return false;
		if(usrflags) return false;
		if(genflags) return false;
		for(auto q = 0; q < 6; ++q)
			if(triggerflags[q]) return false;
		if(triggerlevel) return false;
		if(triggerbtn) return false;
		if(triggeritem) return false;
		if(trigtimer) return false;
		if(trigsfx) return false;
		if(trigchange) return false;
		if(trigprox) return false;
		if(trigctr) return false;
		if(trigctramnt) return false;
		if(triglbeam) return false;
		if(trigcschange) return false;
		if(spawnitem) return false;
		if(spawnenemy) return false;
		if(exstate > -1) return false;
		if(spawnip) return false;
		if(trigcopycat) return false;
		if(trigcooldown) return false;
		if(trig_lstate) return false;
		if(trig_gstate) return false;
		if(trig_statetime) return false;
		if(trig_genscr) return false;
		if(trig_group) return false;
		if(trig_group_val) return false;
		if(!label.empty()) return false;
		for(auto q = 0; q < 8; ++q)
			if(attribytes[q]) return false;
		for(auto q = 0; q < 8; ++q)
			if(attrishorts[q]) return false;
		if(script) return false;
		for(auto q = 0; q < 2; ++q)
			if(initd[q]) return false;
		if(o_tile) return false;
		if(cur_frame) return false;
		if(aclk) return false;
		
		if(liftcmb) return false;
		if(liftundercmb) return false;
		if(liftcs) return false;
		if(liftundercs) return false;
		if(liftdmg) return false;
		if(liftlvl) return false;
		if(liftitm) return false;
		if(liftflags) return false;
		if(liftgfx) return false;
		if(liftsprite) return false;
		if(liftsfx) return false;
		if(liftbreaksprite != -1) return false;
		if(liftbreaksfx) return false;
		if(lifthei != 8) return false;
		if(lifttime != 16) return false;
		if(lift_parent_item) return false;
		if(prompt_cid) return false;
		if(prompt_cs) return false;
		if(prompt_x != 12) return false;
		if(prompt_y != -8) return false;
		
		if(speed_mult != 1) return false;
		if(speed_div != 1) return false;
		if(speed_add) return false;
		return true;
	}
	
	int each_tile(std::function<bool(int32_t)> proc) const;
};

#define AF_FRESH          0x01
#define AF_CYCLE          0x02
#define AF_CYCLENOCSET    0x04
#define AF_TRANSPARENT    0x08

#define LF_LIFTABLE       0x01
#define LF_DROPSET        0x02
#define LF_DROPONLIFT     0x04
#define LF_SPECIALITEM    0x08
#define LF_NOUCSET        0x10
#define LF_NOWPNCMBCSET   0x20
#define LF_BREAKONSOLID   0x40

struct tiletype
{
    byte bitplanes;
    byte *data;
};

struct ZCHEATS
{
    dword flags;
    char  codes[4][41];
    //168
};

#define QH_IDSTR    "AG Zelda Classic Quest File\n "
#define QH_NEWIDSTR "AG ZC Enhanced Quest File\n   "
#define ENC_STR     "Zelda Classic Quest File"

struct zquestheader
{
    char  id_str[31];
    int16_t zelda_version;
    word  internal;
    byte  quest_number;
    byte  old_rules[2];
    byte  old_map_count;
    char  old_str_count;
    byte  data_flags[ZQ_MAXDATA];
    byte  old_rules2[2];
    char  old_options;
    char  version[17];
    char  title[65];
    char  author[65];
    //byte  padding;
    //  int16_t pwdkey;
    bool  dirty_password;
    char  password[256];
    uint8_t pwd_hash[16];
    char  minver[17];
    byte  build;
    byte  use_keyfile;
    byte  old_foo[9];
    byte  old_rules3[2];
    byte  old_rules4[2];
    byte  old_rules5[2];
    byte  old_rules6[2];
    byte  old_rules7[2];
    byte  old_rules8[2];
    byte  old_rules9[2];
    byte  old_rules10[2];
    byte  old_midi_flags[MIDIFLAGS_SIZE];
    //304
    byte  old_foo2[18];
    // No one used custom quest templates, so we stopped supporting it.
    char  templatepath[2048];
    int32_t new_version_id_main;
    int32_t new_version_id_second;
    int32_t new_version_id_third;
    int32_t new_version_id_fourth;
    int32_t new_version_id_alpha;
    int32_t new_version_id_beta;
    int32_t new_version_id_gamma;
    int32_t new_version_id_release;
	bool new_version_is_nightly;
    word new_version_id_date_year;
    byte new_version_id_date_month;
    byte new_version_id_date_day;
    byte new_version_id_date_hour;
    byte new_version_id_date_minute;
    char new_version_devsig[256];
    char new_version_compilername[256];
    char new_version_compilerversion[256];
    char product_name[1024];
    byte compilerid;
    int32_t compilerversionnumber_first;
    int32_t compilerversionnumber_second;
    int32_t compilerversionnumber_third;
    int32_t compilerversionnumber_fourth;
    word developerid;
    char made_in_module_name[1024];
    char build_datestamp[256];
    char build_timestamp[256];
    char build_timezone[6];
    //made in module_name
    
	bool external_zinfo;
	
	
	bool is_legacy() const;
	int8_t getAlphaState() const;
	char const* getAlphaStr(bool ignoreNightly = false) const;
	int32_t getAlphaVer() const;
	char const* getAlphaVerStr() const;
	char const* getVerStr() const;
	char const* getVerCmpStr() const;
	int32_t compareDate() const;
	int32_t compareVer() const;
};

int8_t getProgramAlphaState();
char const* getProgramAlphaVerStr();
char const* getProgramVerStr();
char const* getReleaseTag();

enum { msLINKED };

#define MAX_SCC_ARG 65023
/* Note: Printable ASCII begins at 32 and ends at 126, inclusive. */
#define MSGC_COLOUR            1    // 2 args (cset,swatch)
#define MSGC_SPEED             2    // 1 arg  (speed)
#define MSGC_GOTOIFGLOBAL      3    // 3 args (register, val, newtring)
#define MSGC_GOTOIFRAND        4    // 2 args (factor, newstring)
#define MSGC_GOTOIF            5    // 2 args (itemid, newstring)
#define MSGC_GOTOIFCTR         6    // 3 args (counter, val, newstring)
#define MSGC_GOTOIFCTRPC       7    // 3 args (counter, val, newstring)
#define MSGC_GOTOIFTRI         8    // 2 args (level, newstring)
#define MSGC_GOTOIFTRICOUNT    9    // 2 args (tricount, newstring)
#define MSGC_CTRUP            10    // 2 args (counter, val)
#define MSGC_CTRDN            11    // 2 args (counter, val)
#define MSGC_CTRSET           12    // 2 args (counter, val)
#define MSGC_CTRUPPC          13    // 2 args (counter, val)
#define MSGC_CTRDNPC          14    // 2 args (counter, val)
#define MSGC_CTRSETPC         15    // 2 args (counter, val)
#define MSGC_GIVEITEM         16    // 1 arg  (itemid)
#define MSGC_TAKEITEM         17    // 1 arg  (itemid)
#define MSGC_WARP             18    // 6 args (dmap, screen, x, y, effect, sound
#define MSGC_SETSCREEND       19    // 4 args (dmap, screen, reg, value)
#define MSGC_SFX              20    // 1 arg  (sfx)
#define MSGC_MIDI             21    // 1 arg  (midi)
#define MSGC_NAME             22    // 0 args
#define MSGC_GOTOIFCREEND     23    // 5 args (dmap, screen, reg, val, newstring)
#define MSGC_CHANGEPORTRAIT   24    // not implemented
#define MSGC_NEWLINE          25    // 0 args
#define MSGC_SHDCOLOR         26    // 2 args (cset,swatch)
#define MSGC_SHDTYPE          27    // 1 arg  (type)
#define MSGC_DRAWTILE         28    // 5 args (tile, cset, wid, hei, flip)
#define MSGC_ENDSTRING        29    // 0 args
#define MSGC_WAIT_ADVANCE     30    // 0 args
//31
//32
//33-127 are ascii chars, unusable
#define MSGC_SETUPMENU        128    // 5 args (tile, cset, wid, hei, flip)
#define MSGC_MENUCHOICE       129    // 5 args (pos, upos, dpos, lpos, rpos)
#define MSGC_RUNMENU          130    // 0 args
#define MSGC_GOTOMENUCHOICE   131    // 2 args (pos, newstring)
#define MSGC_TRIGSECRETS      132    // 1 arg (perm)
#define MSGC_SETSCREENSTATE   133    // 2 args (ind, state)
#define MSGC_SETSCREENSTATER  134    // 4 args (map, screen, ind, state)
#define MSGC_FONT             135    // 1 args (font)
#define MSGC_RUN_FRZ_GENSCR   136    // 2 args (script num, force_redraw)
//137+

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

#define MSGSIZE 144
#define MSG_NEW_SIZE 8192
#define MSGBUF_SIZE (MSG_NEW_SIZE*8)

#define STRINGFLAG_WRAP			0x01
#define STRINGFLAG_CONT			0x02
#define STRINGFLAG_CENTER		0x04
#define STRINGFLAG_RIGHT		0x08
#define STRINGFLAG_FULLTILE		0x10
#define STRINGFLAG_TRANS_BG		0x20
#define STRINGFLAG_TRANS_FG		0x40

struct MsgStr
{
	std::string s;
	word nextstring;
	int32_t tile;
	byte cset;
	bool trans;
	byte font;
	int16_t x;
	int16_t y;   // y position of message boxes.
	uint16_t w;
	uint16_t h;
	byte sfx; // either WAV_MSG or something else.
	word listpos;
	byte vspace;
	byte hspace;
	byte stringflags;
	int16_t margins[4];
	int32_t portrait_tile;
	byte portrait_cset;
	byte portrait_x;
	byte portrait_y;
	byte portrait_tw;
	byte portrait_th;
	byte shadow_type;
	byte shadow_color;
	byte drawlayer;
	
	// Copy everything except listpos
	MsgStr& operator=(MsgStr const& other);
	// Copy text data - just s and nextstring
	void copyText(MsgStr const& other);
	// Copy style data - everything except s, nextstring, and listpos
	void copyStyle(MsgStr const& other);
	void copyAll(MsgStr const& other);
	void clear();
};

enum {dt_pass=0, dt_lock, dt_shut, dt_boss, dt_olck, dt_osht, dt_obos, dt_wall, dt_bomb, dt_walk, dt_max};
enum {df_walktrans=0};

struct DoorComboSet
{
    char name[21];
    //byte padding;
    //22
    word doorcombo_u[9][4];                                   //[door type][door combo]
    byte doorcset_u[9][4];                                    //[door type][door combo]
    word doorcombo_d[9][4];                                   //[door type][door combo]
    byte doorcset_d[9][4];                                    //[door type][door combo]
    //238 (216)
    word doorcombo_l[9][6];                                   //[door type][door combo]
    byte doorcset_l[9][6];                                    //[door type][door combo]
    word doorcombo_r[9][6];                                   //[door type][door combo]
    byte doorcset_r[9][6];                                    //[door type][door combo]
    //562 (324)
    word bombdoorcombo_u[2];                                  //rubble
    byte bombdoorcset_u[2];                                   //rubble
    word bombdoorcombo_d[2];                                  //rubble
    byte bombdoorcset_d[2];                                   //rubble
    //574 (12)
    word bombdoorcombo_l[3];                                  //rubble
    byte bombdoorcset_l[3];                                   //rubble
    //byte padding;
    word bombdoorcombo_r[3];                                  //rubble
    byte bombdoorcset_r[3];                                   //rubble
    //byte padding;
    //594 (18)
    word walkthroughcombo[4];                                 //[n, s, e, w]
    byte walkthroughcset[4];                                  //[n, s, e, w]
    
    byte flags[2];
    byte expansion[30];
    //638 (44)
};

struct dmap
{
    byte map;
    word level;
    char xoff;
    byte compass;
    word color;
    byte midi;
    byte cont;
    byte type;
    byte grid[8];
	// [8 rows][half byte for each screen in a row]
	// Covers an entire map.
	// Currently the only valid nibble values are 0-9.
	// 0 indicates that screen is not a scrolling region.
	// Positive values indicate a contiguous scrolling region.
	// Currently, scrolling regions MUST be rectangles and have no holes.
	// Indexes can be repeated - they currently don't hold any special meaning.
	byte region_indices[8][8];
    char name[21];
    char title[21];
    char intro[73];
    int32_t minimap_1_tile;                                      //before getting map
    byte minimap_1_cset;                                      //cset for minimap 1
    int32_t minimap_2_tile;                                      //after getting map
    byte minimap_2_cset;                                      //cset for minimap 2
    int32_t largemap_1_tile;                                     //large map
    byte largemap_1_cset;                                     //cset for large
    int32_t largemap_2_tile;                                     //large map
    byte largemap_2_cset;                                     //cset for large
    char tmusic[56];
    byte tmusictrack;
    byte active_subscreen;
    byte passive_subscreen;
    byte disableditems[iMax];
    int32_t flags;
    char sideview;
    word script;
    int32_t initD[8];
    char initD_label[8][65];
	word active_sub_script;
	word passive_sub_script;
	int32_t sub_initD[8];
	char sub_initD_label[8][65];
	word onmap_script;
	int32_t onmap_initD[8];
	char onmap_initD_label[8][65];
	int16_t mirrorDMap;
};

// DMap flags
#define dmfCAVES            0x000001
#define dmf3STAIR           0x000002
#define dmfWHIRLWIND        0x000004
#define dmfGUYCAVES         0x000008
#define dmfNOCOMPASS        0x000010
#define dmfWAVY             0x000020
#define dmfWHIRLWINDRET     0x000040
#define dmfALWAYSMSG        0x000080
#define dmfVIEWMAP          0x000100
#define dmfDMAPMAP          0x000200
#define dmfMINIMAPCOLORFIX  0x000400
#define dmfSCRIPT1          0x000800
#define dmfSCRIPT2          0x001000
#define dmfSCRIPT3          0x002000
#define dmfSCRIPT4          0x004000
#define dmfSCRIPT5          0x008000
#define dmfSIDEVIEW         0x010000
#define dmfLAYER3BG         0x020000
#define dmfLAYER2BG         0x040000
#define dmfNEWCELLARENEMIES 0x080000
#define dmfBUNNYIFNOPEARL   0x100000
#define dmfMIRRORCONTINUE   0x200000
#define dmfZ3_RESERVERD_1   0x400000
#define dmfZ3_RESERVERD_2   0x800000


#define OLDMAXCOMBOALIASES 256
#define MAX250COMBOALIASES 2048
#define MAXCOMBOALIASES 8192
#define MAXCOMBOPOOLS 8192

struct combo_alias
{
    combo_alias()
    {
        memset(this, 0, sizeof(combo_alias));
        combos = new word[1];
        csets = new byte[1];
        combos[0] = 0;
        csets[0] = 0;
    }
    
    byte width;      // Max 15
    byte height;     // Max 10
    byte layermask;  // Specifies layers to be drawn
    word combo;      // Optional thumbnail combo for the alias list
    word cset;
    word *combos;    // Dynamic array. Will be stored in quest.
    byte *csets;
};

struct cpool_entry
{
	int32_t cid;
	int8_t cset;
	word quant;
	void clear()
	{
		cid = -1;
		cset = -1;
		quant = 0;
	}
	bool valid() const
	{
		return quant > 0 && unsigned(cid) < MAXCOMBOS;
	}
	cpool_entry() { clear(); }
	cpool_entry(int32_t data,int8_t cs,word q) :
		cid(data), cset(cs), quant(q)
	{}
};
struct combo_pool
{
	std::vector<cpool_entry> combos;
	
	combo_pool() : totalweight(0)
	{}
	combo_pool& operator=(combo_pool const& other);
	void push(int32_t cid, int8_t cs, word q=1); //add a quantity of a combo entry
	void add(int32_t cid, int8_t cs, word q=1); //add a new combo entry
	void swap(size_t ind1, size_t ind2);
	void erase(size_t ind); //Remove a combo
	void trim(); //Trim any invalid entries
	cpool_entry const* get_ind(size_t index) const;
	cpool_entry const* get_w(size_t weight_index) const;
	cpool_entry const* get_w_wrap(size_t weight_index) const;
	cpool_entry const* pick() const;
	bool get_ind(int32_t& cid, int8_t& cs, size_t index) const;
	bool get_w(int32_t& cid, int8_t& cs, size_t weight_index) const;
	bool get_w_wrap(int32_t& cid, int8_t& cs, size_t weight_index) const;
	bool pick(int32_t& cid, int8_t& cs) const;
	void clear()
	{
		combos.clear();
		combos.shrink_to_fit();
		totalweight = 0;
	}
	void recalc();
	size_t getTotalWeight() const
	{
		return totalweight;
	}
	bool valid() const
	{
		return totalweight > 0;
	}
private:
	//Does not need saving
	size_t totalweight;
	
	cpool_entry* get(int32_t cid, int8_t cs); //get a combo existing in the list
};

struct shoptype
{
    char name[32];
    byte item[3];
    byte hasitem[3];
    word price[3];
    word str[3];  //item info string
    //10
};

struct bottleshoptype
{
	char name[32];
	byte fill[3];
	word comb[3];
	byte cset[3];
	word price[3];
	word str[3];
    void clear()
    {
        memset(name, 0, sizeof(name));
        memset(fill, 0, sizeof(fill));
        memset(comb, 0, sizeof(comb));
        memset(cset, 0, sizeof(cset));
        memset(price, 0, sizeof(price));
        memset(str, 0, sizeof(str));
    }
};

struct bottletype
{
    char name[32];
    char counter[3];
    word amount[3];
    byte flags;
    byte next_type;
#define BTFLAG_PERC0       0x01
#define BTFLAG_PERC1       0x02
#define BTFLAG_PERC2       0x04
#define BTFLAG_CURESWJINX  0x10
#define BTFLAG_CUREITJINX  0x20
#define BTFLAG_ALLOWIFFULL 0x40
#define BTFLAG_AUTOONDEATH 0x80
    void clear()
    {
        memset(name, 0, sizeof(name));
        memset(counter, -1, sizeof(counter));
        memset(amount, 0, sizeof(amount));
        flags = 0;
        next_type = 0;
    }
};

struct pondtype
{
    byte olditem[3];
    byte newitem[3];
    byte fairytile;
    byte aframes;
    byte aspeed;
    word msg[15];
    byte foo[32];
    //72
};

struct infotype
{
    char name[32];
    word str[3];
    //byte padding;
    word price[3];
    //14
};

struct warpring
{
    char name[32];
    word dmap[9];
    byte scr[9];
    byte size;
    //18
};

struct zcolors
{
    byte text, caption;
    byte overw_bg, dngn_bg;
    byte dngn_fg, cave_fg;
    byte bs_dk, bs_goal;
    byte compass_lt, compass_dk;
    //10
    byte subscr_bg, subscr_shadow, triframe_color;
    byte bmap_bg,bmap_fg;
    byte hero_dot;
    //15
    byte triforce_cset;
    byte triframe_cset;
    byte overworld_map_cset;
    byte dungeon_map_cset;
    byte blueframe_cset;
    //20
    int32_t triforce_tile;
    int32_t triframe_tile;
    int32_t overworld_map_tile;
    int32_t dungeon_map_tile;
    int32_t blueframe_tile;
    //40
    int32_t HCpieces_tile;
    byte HCpieces_cset;
    byte msgtext;
    
    byte foo[6];
    //52
    byte foo2[256];
    
    
    //308 bytes
};

struct palcycle
{
    byte first,count,speed;
    //3
};

enum miscsprite
{
	sprFALL,
	sprDROWN,
	sprLAVADROWN,
	sprSWITCHPOOF,
	sprMAX = 256
};
enum miscsfx
{
	sfxBUSHGRASS,
	sfxSWITCHED,
	sfxLOWHEART,
	sfxERROR,
	sfxHURTPLAYER,
	sfxHAMMERPOUND,
	sfxSUBSCR_ITEM_ASSIGN,
	sfxSUBSCR_CURSOR_MOVE,
	sfxREFILL,
	sfxDRAIN,
	sfxMAX = 256
};

#define NUM_SHOPS 256
#define NUM_INFOS 256
#define NUM_PAL_CYCLES 256
#define NUM_WARP_RINGS 9

struct miscQdata
{
    shoptype shop[NUM_SHOPS];
    //160 (160=10*16)
    infotype info[NUM_INFOS];
    //384 (224=14*16)
    warpring warp[NUM_WARP_RINGS];
    //528 (144=18*8)
    palcycle cycles[NUM_PAL_CYCLES][3];
    //2832 (2304=3*256*3)
    //2850 (18=2*2)
    byte     triforce[8];                                     // positions of triforce pieces on subscreen
    //2858 (8)
    zcolors  colors;
    //3154 (296)
    int32_t     icons[4];
    //3162 (8=2*4)
    //pondtype pond[16];
    //4314 (1152=72*16)
    word endstring;
    //  byte dummy;  // left over from a word
    //word expansion[98];
    //4512
    int32_t questmisc[32]; //Misc init values for the user. Used by scripts.
    char questmisc_strings[32][128]; //needs to be memset then data allocated from IntiData
	//We probably want a way to access these in ZScript by their string, or to get the strings stored.
    int32_t zscript_last_compiled_version;
	byte sprites[sprMAX];
	
	bottletype bottle_types[64];
	bottleshoptype bottle_shop_types[256];
	
	byte miscsfx[sfxMAX];
};

#define MFORMAT_MIDI 0
#define MFORMAT_NSF  1

//tune flags
#define tfDISABLESAVE    1

class zctune
{

public:

    char title[36];
    //20
    int32_t start;
    int32_t loop_start;
    int32_t loop_end;
    //32
    int16_t loop;
    int16_t volume;
    byte flags;
    // 37
    void *data;
    // 41
    
    byte format;
    
    zctune()
    {
        data = NULL;
        format = MFORMAT_MIDI;
        reset();
    }
    
    zctune(char _title[36], int32_t _start, int32_t _loop_start, int32_t _loop_end, int16_t _loop,int16_t _volume, void *_data, byte _format)
        : start(_start), loop_start(_loop_start), loop_end(_loop_end), loop(_loop), volume(_volume), data(_data), format(_format), flags(0)
    {
        //memcpy(title, _title, 20); //NOT SAFE for int16_t strings
        strncpy(title, _title, 36);
    }
    
    void clear()
    {
        memset(title,0,36);
        start = loop_start = loop_end = 0;
        loop = volume = flags = 0;
        data = NULL;
    }
    
    void copyfrom(zctune z)
    {
        start = z.start;
        loop_start = z.loop_start;
        loop_end = z.loop_end;
        loop = z.loop;
        flags = z.flags;
        volume = z.volume;
        //memcpy(title, z.title,20); //NOT SAFE for int16_t title strings
        strncpy(title, z.title, 36);
        data = z.data;
    }
    
    void reset()
    {
        title[0]=0;
        loop=1;
        volume=144;
        start=0;
        loop_start=-1;
        loop_end=-1;
        flags=0;
        
        if(data) switch(format)
            {
            case MFORMAT_MIDI:
                destroy_midi((MIDI*) data);
                break;
                
            default:
                break;
            }
            
        data = NULL;
    }
    
};

/*typedef struct zcmidi_ // midi or other sound format (nsf ...)
{
  char title[20];
  //20
  int32_t start;
  int32_t loop_start;
  int32_t loop_end;
  //32
  int16_t loop;
  int16_t volume;
  //36
  byte format;
  MIDI *midi;
  //41
} zcmidi_;
*/

/*typedef struct emusic
{
  char title[20];
  char filename[256];
} emusic;
*/

enum // used for gamedata ITEMS
{
	// 0
	itype_sword, itype_brang, itype_arrow, itype_candle, itype_whistle,
	itype_bait, itype_letter, itype_potion, itype_wand, itype_ring,
	itype_wallet, itype_amulet, itype_shield, itype_bow, itype_raft,
	itype_ladder, itype_book, itype_magickey, itype_bracelet, itype_flippers,
	// 20
	itype_boots, itype_hookshot, itype_lens, itype_hammer, itype_divinefire,
	itype_divineescape, itype_divineprotection, itype_bomb, itype_sbomb, itype_clock,
	itype_key, itype_magiccontainer, itype_triforcepiece, itype_map, itype_compass,
	itype_bosskey, itype_quiver, itype_lkey, itype_cbyrna, itype_rupee,
	// 40
	itype_arrowammo, itype_fairy, itype_magic, itype_heart, itype_heartcontainer,
	itype_heartpiece, itype_killem, itype_bombammo, itype_bombbag, itype_rocs,
	itype_hoverboots, itype_spinscroll,itype_crossscroll, itype_quakescroll,itype_whispring,
	itype_chargering, itype_perilscroll, itype_wealthmedal,itype_heartring,itype_magicring,
	// 60
	itype_spinscroll2, itype_quakescroll2, itype_agony, itype_stompboots, itype_whimsicalring,
	itype_perilring, itype_misc,
	// 67
	itype_custom1, itype_custom2, itype_custom3, itype_custom4, itype_custom5,
	itype_custom6, itype_custom7, itype_custom8, itype_custom9, itype_custom10,
	itype_custom11, itype_custom12, itype_custom13, itype_custom14, itype_custom15,
	itype_custom16, itype_custom17, itype_custom18, itype_custom19, itype_custom20,
	// 87
	itype_bowandarrow, itype_letterpotion,
	itype_last, 
	itype_script1 = 256, //Scripted Weapons
	itype_script2, itype_script3, itype_script4, itype_script5, itype_script6, itype_script7, itype_script8, itype_script9, itype_script10,
	itype_icerod, itype_atkring, itype_lantern, itype_pearl, itype_bottle, itype_bottlefill, itype_bugnet,
	itype_mirror, itype_switchhook, itype_itmbundle, itype_progressive_itm, itype_note, itype_refill,
	itype_liftglove,
	/*
	itype_templast,
	itype_ether, itype_bombos, itype_quake, 
	itype_powder,
	itype_trowel,
	itype_instrument,
	itype_sword180,
	itype_sword_gb,
	itype_firerod,
	itype_scripted_001 = 400, 
	itype_scripted_002,
	itype_scripted_003,
	itype_scripted_004,
	itype_scripted_005,
	itype_scripted_006,
	itype_scripted_007,
	itype_scripted_008,
	itype_scripted_009,
	itype_scripted_010,
	*/

	
	
	itype_max=512
};

#define itype_max_zc250 255 //Last in the 2.50.x lists. 

enum {i_sword=1, i_wsword, i_msword, i_xsword, imax_sword};
enum {i_wbrang=1, i_mbrang, i_fbrang, imax_brang};
enum {i_warrow=1, i_sarrow, i_garrow, imax_arrow};
enum {i_bcandle=1, i_rcandle, imax_candle};
enum {i_recorder=1, imax_whistle};
enum {i_bait=1, imax_bait};
enum {i_letter=1, i_letter_used, imax_letter};
enum {i_bpotion=1, i_rpotion, imax_potion};
enum {i_wand=1, imax_wand};
enum {i_bring=2, i_rring, i_gring, imax_ring};
enum {i_swallet=1, i_lwallet, i_mwallet, imax_wallet};
enum {i_amulet1=1, i_amulet2, imax_amulet};
enum {i_smallshield=1, i_largeshield, i_mirrorshield, imax_shield};
enum {i_shortbow=1, i_longbow, imax_bow};
enum {i_raft=1, imax_raft};
enum {i_ladder=1, i_ladder2, imax_ladder};
enum {i_book=1, imax_book};
enum {i_magickey=1, imax_magickey};
enum {i_oldglove=1, i_bracelet1, i_bracelet2, imax_bracelet};
enum {i_flippers=1, imax_flippers};
enum {i_boots=1, imax_boots};
enum {i_hookshot=1, i_longshot, imax_hookshot};
enum {i_lens=1, imax_lens};
enum {i_hammer=1, imax_hammer};
enum {i_divinefire=1, imax_divinefire};
enum {i_divineescape=1, imax_divineescape};
enum {i_divineprotection=1, imax_divineprotection};
enum {i_quiver=1, i_quiverl2, i_quiverl3, i_quiverl4, imax_quiver};
enum {i_cbyrna=1, imax_cbyrna};
enum {i_rocs=1, imax_rocs};
enum {i_hoverboots=1, imax_hoverboots};
enum {i_spinscroll1=1, i_spinscroll2, imax_spinscroll};
enum {i_crossscroll=1, imax_crossscroll};
enum {i_quakescroll1=1, i_quakescroll2, imax_quakescroll};
enum {i_whispring1=1, i_whispring2, imax_whispring};
enum {i_chargering1=1, i_chargering2, imax_chargering};
enum {i_perilscroll=1, imax_perilscroll};
enum {i_shieldscroll=1, imax_shieldscroll};
enum {i_wealthmedal=1, i_wealthmedal2, i_wealthmedal3, imax_wealthmedal};
enum {i_heartring=1, i_heartring2, i_heartring3, imax_heartring};
enum {i_magicring=1, i_magicring2, i_magicring3, i_magicring4, imax_magicring};
enum {i_rupee=1, i_5rupee, i_10rupee, i_20rupee, i_50rupee, i_100rupee, i_200rupee, imax_rupee};
enum {i_arrowa=1, i_5arrowa, i_10arrowa, i_30arrowa, imax_arrowa};
enum {i_bomba=1, i_4bomba, i_8bomba, i_30bomba, imax_bomba};
enum {i_bomb = 1, imax_bomb};
enum {i_sbomb = 1, imax_sbomb};
enum {i_bombbag1=1, i_bombbag2, i_bombbag3, i_bombbag4, imax_bombbag};

//enum {i_clock=1, imax_clock};

enum generic_ind
{
	genHCP, genMDRAINRATE, genCANSLASH, genWLEVEL,
	genHCP_PER_HC, genCONTHP, genCONTHP_IS_PERC, genHP_PER_HEART,
	genMP_PER_BLOCK, genHERO_DMG_MULT, genENE_DMG_MULT,
	genDITH_TYPE, genDITH_ARG, genDITH_PERC, genLIGHT_RAD,genTDARK_PERC,genDARK_COL,
	genWATER_GRAV, genSIDESWIM_UP, genSIDESWIM_SIDE, genSIDESWIM_DOWN, genSIDESWIM_JUMP,
	genBUNNY_LTM, genSWITCHSTYLE, genLAST,
	genMAX = 256
};
enum glow_shape
{
	glshapeCIRC, glshapeCONE,
	glshapeMAX
};
enum
{
	crNONE = -1,
	crLIFE, crMONEY, crBOMBS, crARROWS, crMAGIC,
	crKEYS, crSBOMBS, crCUSTOM1, crCUSTOM2, crCUSTOM3,
	crCUSTOM4, crCUSTOM5, crCUSTOM6, crCUSTOM7, crCUSTOM8,
	crCUSTOM9, crCUSTOM10, crCUSTOM11, crCUSTOM12, crCUSTOM13,
	crCUSTOM14, crCUSTOM15, crCUSTOM16, crCUSTOM17, crCUSTOM18,
	crCUSTOM19, crCUSTOM20, crCUSTOM21, crCUSTOM22, crCUSTOM23,
	crCUSTOM24, crCUSTOM25, MAX_COUNTERS
};

#define MAX_SAVED_PORTALS 10000
struct savedportal
{
	int16_t destdmap = -1;
	int16_t srcdmap = -1;
	byte srcscr;
	byte destscr;
	int32_t x;
	int32_t y;
	byte sfx;
	int32_t warpfx;
	int16_t spr;
	bool deleting;
	
	int32_t getUID(){return uid;}
	
	savedportal();
	void clear()
	{
		*this = savedportal();
	}
	
private:
	int32_t uid;
	inline static int32_t nextuid = 1;
};

#define DIDCHEAT_BIT 0x80
#define NUM_GSWITCHES 256
#define MAX_MI (MAXDMAPS*MAPSCRSNORMAL)
struct gamedata
{
	//private:
	char  _name[9];
	byte  _quest;
	//10
	//word _life,_maxlife;
	//int16_t _drupy;
	//word _rupies,_arrows,_maxarrows,
	word _deaths;
	//20
	//byte  _keys,_maxbombs,
	byte  /*_wlevel,*/_cheat;
	//24
	bool  item[MAXITEMS];
	byte  items_off[MAXITEMS];
	//280
	word _maxcounter[MAX_COUNTERS];	// 0 - life, 1 - rupees, 2 - bombs, 3 - arrows, 4 - magic, 5 - keys, 6-super bombs
	word _counter[MAX_COUNTERS];
	int16_t _dcounter[MAX_COUNTERS];
	
	char  version[17];
	char  title[65];
	//354
	byte  _hasplayed;
	//byte  padding;
	//356
	dword _time;
	//360
	byte  _timevalid;
	byte  lvlitems[MAXLEVELS];
	byte  lvlkeys[MAXLEVELS];
	dword lvlswitches[MAXLEVELS];
	//byte  _HCpieces;
	byte  _continue_scrn;
	word  _continue_dmap;
	//620
	/*word  _maxmagic, _magic;
	int16_t _dmagic;*/
	//byte  _magicdrainrate;
	//byte  _canslash;                                           //Hero slashes instead of stabs.
	int32_t _generic[genMAX];	// Generic gamedata. See enum above this struct for indexes.
	//byte  padding[2];
	//636
	byte  visited[MAXDMAPS];
	//892 (256)
	byte  bmaps[MAXDMAPS*128];                                 // the dungeon progress maps
	//17276 (16384)
	word  maps[MAXMAPS2*MAPSCRSNORMAL];                       // info on map changes, items taken, etc.
	//82556 (65280)
	byte  guys[MAXMAPS2*MAPSCRSNORMAL];                       // guy counts (though dungeon guys are reset on entry)
	//115196 (32640)
	char  qstpath[2048];
	byte  icon[128];
	byte  pal[48];
	bool item_messages_played[MAXITEMS];  //Each field is set when an item pickup message plays the first time per session
	int32_t  screen_d[MAX_MI][8];                // script-controlled screen variables
	int32_t  global_d[MAX_SCRIPT_REGISTERS];                                      // script-controlled global variables
	std::vector< ZCArray <int32_t> > globalRAM;
	
	byte awpn, bwpn, xwpn, ywpn;											// Currently selected weapon slots
	int16_t forced_awpn = -1, forced_bwpn = -1, forced_xwpn = -1, forced_ywpn = -1;
	bool isclearing; // The gamedata is being cleared
	//115456 (260)
	byte bottleSlots[256];
	
	savedportal saved_mirror_portal;
	
	byte swim_mult = 1, swim_div = 1;
	
	bool gen_doscript[NUMSCRIPTSGENERIC];
	word gen_exitState[NUMSCRIPTSGENERIC];
	word gen_reloadState[NUMSCRIPTSGENERIC];
	int32_t gen_initd[NUMSCRIPTSGENERIC][8];
	int32_t gen_dataSize[NUMSCRIPTSGENERIC];
	std::vector<int32_t> gen_data[NUMSCRIPTSGENERIC];
	uint32_t gen_eventstate[NUMSCRIPTSGENERIC];
	
	uint32_t xstates[MAXMAPS2*MAPSCRSNORMAL];
	
	int32_t gswitch_timers[NUM_GSWITCHES];

	std::string replay_file;
	std::vector<saved_user_object> user_objects;
	std::vector<savedportal> user_portals;
	
	void Clear();
	void Copy(const gamedata& g);
	void clear_genscript();
	
	void save_user_objects();
	void load_user_objects();
	
	char *get_name();
	void set_name(const char *n);
	
	byte get_quest();
	void set_quest(byte q);
	void change_quest(int16_t q);
	
	word get_counter(byte c);
	void set_counter(word change, byte c);
	void change_counter(int16_t change, byte c);
	
	word get_maxcounter(byte c);
	void set_maxcounter(word change, byte c);
	void change_maxcounter(int16_t change, byte c);
	
	int16_t get_dcounter(byte c);
	void set_dcounter(int16_t change, byte c);
	void change_dcounter(int16_t change, byte c);
	
	word get_life();
	void set_life(word l);
	void change_life(int16_t l);
	
	word get_maxlife();
	void set_maxlife(word m);
	void change_maxlife(int16_t m);
	
	int16_t get_drupy();
	void set_drupy(int16_t d);
	void change_drupy(int16_t d);
	
	word get_rupies();
	word get_spendable_rupies();
	void set_rupies(word r);
	void change_rupies(int16_t r);
	
	word get_maxarrows();
	void set_maxarrows(word a);
	void change_maxarrows(int16_t a);
	
	word get_arrows();
	void set_arrows(word a);
	void change_arrows(int16_t a);
	
	word get_deaths();
	void set_deaths(word d);
	void change_deaths(int16_t d);
	
	word get_keys();
	void set_keys(word k);
	void change_keys(int16_t k);
	
	word get_bombs();
	void set_bombs(word k);
	void change_bombs(int16_t k);
	
	word get_maxbombs();
	void set_maxbombs(word b, bool setSuperBombs=true);
	void change_maxbombs(int16_t b);
	
	word get_sbombs();
	void set_sbombs(word k);
	void change_sbombs(int16_t k);
	
	word get_wlevel();
	void set_wlevel(word l);
	void change_wlevel(int16_t l);
	
	byte get_cheat();
	void set_cheat(byte c);
	void did_cheat(bool set);
	bool did_cheat();
	
	byte get_hasplayed();
	void set_hasplayed(byte p);
	void change_hasplayed(int16_t p);
	
	dword get_time();
	void set_time(dword t);
	void change_time(int64_t t);
	
	byte get_timevalid();
	void set_timevalid(byte t);
	void change_timevalid(int16_t t);
	
	byte get_HCpieces();
	void set_HCpieces(byte p);
	void change_HCpieces(int16_t p);
	
	byte get_hcp_per_hc();
	void set_hcp_per_hc(byte val);
	
	byte get_cont_hearts();
	void set_cont_hearts(byte val);
	
	bool get_cont_percent();
	void set_cont_percent(bool ispercent);
	
	byte get_hp_per_heart();
	void set_hp_per_heart(byte val);
	
	byte get_mp_per_block();
	void set_mp_per_block(byte val);
	
	byte get_hero_dmgmult();
	void set_hero_dmgmult(byte val);
	
	byte get_ene_dmgmult();
	void set_ene_dmgmult(byte val);
	
	byte get_dither_type();
	void set_dither_type(byte val);
	
	byte get_dither_arg();
	void set_dither_arg(byte val);

	byte get_dither_perc();
	void set_dither_perc(byte val);

	byte get_light_rad();
	void set_light_rad(byte val);
	
	byte get_transdark_perc();
	void set_transdark_perc(byte val);
	
	byte get_darkscr_color();
	void set_darkscr_color(byte val);
	
	int32_t get_watergrav();
	void set_watergrav(int32_t val);
	
	int32_t get_sideswim_up();
	void set_sideswim_up(int32_t val);
	
	int32_t get_sideswim_side();
	void set_sideswim_side(int32_t val);
	
	int32_t get_sideswim_down();
	void set_sideswim_down(int32_t val);
	
	int32_t get_sideswim_jump();
	void set_sideswim_jump(int32_t val);
	
	int32_t get_bunny_ltm();
	void set_bunny_ltm(int32_t val);
	
	byte get_switchhookstyle();
	void set_switchhookstyle(byte val);
	
	byte get_continue_scrn();
	void set_continue_scrn(byte s);
	void change_continue_scrn(int16_t s);
	
	word get_continue_dmap();
	void set_continue_dmap(word d);
	void change_continue_dmap(int16_t d);
	
	word get_maxmagic();
	void set_maxmagic(word m);
	void change_maxmagic(int16_t m);
	
	word get_magic();
	void set_magic(word m);
	void change_magic(int16_t m);
	
	int16_t get_dmagic();
	void set_dmagic(int16_t d);
	void change_dmagic(int16_t d);
	
	byte get_magicdrainrate();
	void set_magicdrainrate(byte r);
	void change_magicdrainrate(int16_t r);
	
	byte get_canslash();
	void set_canslash(byte s);
	void change_canslash(int16_t s);
	
	int32_t get_generic(byte c);
	void set_generic(int32_t change, byte c);
	void change_generic(int32_t change, byte c);
	
	byte get_lkeys();
	
	void set_item(int32_t id, bool value);
	void set_item_no_flush(int32_t id, bool value);
	inline bool get_item(int32_t id)
	{
		if ( ((unsigned)id) >= MAXITEMS ) return false;
			return item[id];
	}
	
	byte get_bottle_slot(dword slot)
	{
		if(slot > 255) return 0;
		return bottleSlots[slot];
	}
	void set_bottle_slot(dword slot, byte val)
	{
		if(slot > 255) return;
		if(val > 64) val = 0;
		bottleSlots[slot] = val;
	}
	
	int32_t fillBottle(byte val);
	bool canFillBottle();
	
	void set_portal(int16_t destdmap, int16_t srcdmap, byte scr, int32_t x, int32_t y, byte sfx, int32_t weffect, int16_t psprite);
	void load_portal();
	void clear_portal(int32_t);
	
	void load_portals();
	savedportal* getSavedPortal(int32_t uid);

	bool should_show_time();
};

// "initialization data" flags (bit numbers in bit string)
// These are DEFUNCT, it seems.
enum
{
    idE_RAFT, idE_LADDER, idE_BOOK, idE_KEY,
    idE_FLIPPERS, idE_BOOTS, idE_HOVERBOOTS, idE_MAX
};
enum
{
    idI_WAND, idI_LETTER, idI_LENS, idI_HOOKSHOT,
    idI_BAIT, idI_HAMMER, idI_CBYRNA, idI_ROCS, idI_MAX
};
enum { idI_DFIRE, idI_FWIND, idI_NLOVE, idI_MAX2 };
enum { idM_CONTPERCENT, idM_DOUBLEMAGIC, idM_CANSLASH, idM_MAX };
enum
{
    idBP_WOODENPERCENT, idBP_WHITEPERCENT,
    idBP_MAGICPERCENT, idBP_MASTERPERCENT, idBP_MAX
};

struct zinitdata
{
	bool items[256];
	//94
	byte hc;
	word start_heart, cont_heart;
	byte hcp, hcp_per_hc, keys;
	word rupies;
	byte triforce;                                            // bit flags
	byte map[64];
	byte compass[64];
	byte boss_key[64];
	byte misc[16];
	// byte sword_hearts[4];
	byte last_map;                                            //last map worked on
	//220
	byte last_screen;                                         //last screen worked on
	word max_magic;
	word magic;
	byte bomb_ratio;	// ratio of super bombs to bombs
	byte msg_more_x, msg_more_y, msg_more_is_offset;
	byte subscreen;
	word start_dmap;
	byte heroAnimationStyle;
	//238
	//byte expansion[98];
	//336 bytes total
	byte level_keys[MAXLEVELS];
	int32_t ss_grid_x;
	int32_t ss_grid_y;
	int32_t ss_grid_xofs;
	int32_t ss_grid_yofs;
	int32_t ss_grid_color;
	int32_t ss_bbox_1_color;
	int32_t ss_bbox_2_color;
	int32_t ss_flags;
	byte subscreen_style;
	byte usecustomsfx;
	word max_rupees, max_keys;
	byte gravity; //Deprecated!
	int32_t gravity2; //Bumping this up to an int32_t.
	word terminalv;
	byte msg_speed;
	byte transition_type; // Can't edit, yet.
	byte jump_hero_layer_threshold; // Hero is drawn above layer 3 if z > this.
	byte hero_swim_speed;
	
	word bombs, super_bombs, max_bombs, max_sbombs, arrows, max_arrows, heroStep, subscrSpeed, heroSideswimUpStep, heroSideswimSideStep, heroSideswimDownStep;
	
	int32_t exitWaterJump;

	byte hp_per_heart, magic_per_block, hero_damage_multiplier, ene_damage_multiplier;
	
	word scrcnt[25], scrmaxcnt[25]; //Script counter start/max -Em
	
	int32_t swimgravity;
	
	byte dither_type, dither_arg, dither_percent, def_lightrad, transdark_percent, darkcol;
	
	int32_t bunny_ltm;
	byte switchhookstyle;
	
	byte magicdrainrate;
	
	byte hero_swim_mult = 2, hero_swim_div = 3;
	
	bool gen_doscript[NUMSCRIPTSGENERIC];
	word gen_exitState[NUMSCRIPTSGENERIC];
	word gen_reloadState[NUMSCRIPTSGENERIC];
	int32_t gen_initd[NUMSCRIPTSGENERIC][8];
	int32_t gen_dataSize[NUMSCRIPTSGENERIC];
	std::vector<int32_t> gen_data[NUMSCRIPTSGENERIC];
	uint32_t gen_eventstate[NUMSCRIPTSGENERIC];
	
	void clear_genscript()
	{
		memset(gen_doscript, 0, sizeof(gen_doscript));
		memset(gen_exitState, 0, sizeof(gen_exitState));
		memset(gen_reloadState, 0, sizeof(gen_reloadState));
		memset(gen_eventstate, 0, sizeof(gen_eventstate));
		memset(gen_initd, 0, sizeof(gen_initd));
		memset(gen_dataSize, 0, sizeof(gen_dataSize));
		for(size_t q = 0; q < NUMSCRIPTSGENERIC; ++q)
		{
			gen_data[q].clear();
			gen_data[q].resize(0);
		}
	}
	
	void clear();
	void copy(zinitdata const& other);
	
	zinitdata(){clear();}
	zinitdata(zinitdata const& other)
	{
		copy(other);
	}
	zinitdata& operator=(zinitdata const& other)
	{
		copy(other);
		return *this;
	}
};

struct zcmap
{
	byte tileWidth;
	byte tileHeight;
	word subaWidth;
	word subaHeight;
	word subpWidth;
	word subpHeight;
	word scrResWidth;
	word scrResHeight;
	word viewWidth;
	word viewHeight;
	word viewX;
	word viewY;
	bool subaTrans;
	bool subpTrans;
};

enum controls //Args for 'getInput()'
{
	//control_state indeces
	btnUp, btnDown, btnLeft, btnRight, btnA, btnB, btnS, btnL, btnR, btnP, btnEx1, btnEx2, btnEx3, btnEx4, btnAxisUp, btnAxisDown, btnAxisLeft, btnAxisRight,
	//Other controls
	btnM, btnF12, btnF11, btnF5, btnQ, btnI,
	btnLast,
};

#define INT_BTN_A   0x01
#define INT_BTN_B   0x02
#define INT_BTN_L   0x04
#define INT_BTN_R   0x08
#define INT_BTN_EX1 0x10
#define INT_BTN_EX2 0x20
#define INT_BTN_EX3 0x40
#define INT_BTN_EX4 0x80

///////////////
/// MODULES ///
///////////////

// TODO: we would like to remove the module system / loading from datafiles eventually.

enum { zelda_dat, zquest_dat, fonts_dat, sfx_dat };

enum {
    sels_tile_frame, sels_tile_questicon_1A, sels_tile_questicon_1B, sels_tile_questicon_2A,
    sels_tile_questicon_2B, sels_tile_questicon_3A, sels_tile_questicon_3B, sels_tile_questicon_4A, 
    sels_tile_questicon_4B, sels_tile_questicon_5A, sels_tile_questicon_5B, sels_tile_questicon_6A, 
    sels_tile_questicon_6B, sels_tile_questicon_7A, sels_tile_questicon_7B, sels_tile_questicon_8A, 
    sels_tile_questicon_8B, sels_tile_questicon_9A, sels_tile_questicon_9B, sels_tile_questicon_10A, 
    sels_tile_questicon_10B, 
    //x positions
    sels_tile_questicon_1A_X, sels_tile_questicon_1B_X, sels_tile_questicon_2A_X, sels_tile_questicon_2B_X,
    sels_tile_questicon_3A_X, sels_tile_questicon_3B_X, sels_tile_questicon_4A_X, sels_tile_questicon_4B_X, 
    sels_tile_questicon_5A_X, sels_tile_questicon_5B_X, sels_tile_questicon_6A_X, sels_tile_questicon_6B_X, 
    sels_tile_questicon_7A_X, sels_tile_questicon_7B_X, sels_tile_questicon_8A_X, sels_tile_questicon_8B_X, 
    sels_tile_questicon_9A_X, sels_tile_questicon_9B_X, sels_tile_questicon_10A_X, sels_tile_questicon_10B_X,
	
	
    sels_cursor_tile, sels_heart_tile, sels_herotile, draw_hero_first,
    sels_tile_LAST
};

enum {
    sels_tile_frame_cset, sels_tile_questicon_1A_cset, sels_tile_questicon_1B_cset, sels_tile_questicon_2A_cset,
    sels_tile_questicon_2B_cset, sels_tile_questicon_3A_cset, sels_tile_questicon_3B_cset, sels_tile_questicon_4A_cset, 
    sels_tile_questicon_4B_cset, sels_tile_questicon_5A_cset, sels_tile_questicon_5B_cset, sels_tile_questicon_6A_cset, 
    sels_tile_questicon_6B_cset, sels_tile_questicon_7A_cset, sels_tile_questicon_7B_cset, sels_tile_questicon_8A_cset, 
    sels_tile_questicon_8B_cset, sels_tile_questicon_9A_cset, sels_tile_questicon_9B_cset, sels_tile_questicon_10A_cset, 
    sels_tile_questicon_10B_cset, change_cset_on_quest_3, 
	sels_cusror_cset, sels_heart_tilettile_cset, sels_hero_cset,
	
	sels_tile_cset_LAST
	
};

struct zcmodule
{
	char module_name[2048]; //filepath for current zcmodule file
	char quests[10][255]; //first five quests, filenames
	char skipnames[10][255]; //name entry passwords
	char datafiles[5][255]; //qst.dat, zquest.dat, fonts.dat, sfx.dat, zelda.dat

	byte old_quest_serial_flow; //Do we go from 3rd to 5th, 
	//and from 5th to 4th, or just 1->2->3->4->5
	//If this is 0, we do quests in strict order.
	//if it is 1, then we use the old hardcoded quest flow.
	
	byte max_quest_files;
	word startingdmap[10];
	word startingscreen[10];
	int32_t title_track, tf_track, gameover_track, ending_track, dungeon_track, overworld_track, lastlevel_track;
	
	char enem_type_names[eeMAX][255];
	char enem_anim_type_names[aMAX][255];
	char roomtype_names[rMAX][255];
	char walkmisc7_names[e7tEATHURT+1][255];
	char walkmisc9_names[e9tARMOS+1][255];
	char guy_type_names[gDUMMY1][255];
	char enemy_weapon_names[wMax-wEnemyWeapons][255];
	char enemy_scriptweaponweapon_names[10][255];
	char player_weapon_names[wIce+1][255];
	
	char base_NSF_file[1024];
	char copyright_strings[3][2048];
	int32_t copyright_string_vars[10*3]; //font, 104,136,13,-1
	char animate_NES_title;
	char delete_quest_data_on_wingame[20]; //Do we purge items, scripts, and other data when moving to the next quest?
		
	int32_t select_screen_tiles[sels_tile_LAST];
	char select_screen_tile_csets[sels_tile_cset_LAST];
	byte refresh_title_screen;
	
	//to add, and init
	//word startingdmap, startingscreen;
	//char enemy_script_weapon_names[10][255];
	
	char moduletitle[255];
	byte modver_1, modver_2, modver_3, modver_4, modbuild, modbeta;
	byte modmonth, modday, modhour, modminute;
	word modyear;
	char moduleauthor[255];
	char moduleinfo0[255];
	char moduleinfo1[255];
	char moduleinfo2[255];
	char moduleinfo3[255];
	char moduleinfo4[255];
	char moduletimezone[7]; //supports fiveb char abbreviations, and UTC+ or UTC- nn. 
	//char module_base_nsf[255];
	
}; //zcmodule

#include "zinfo.h"

#define titleScreen250 0
#define titleScreen210 10
#define titleScreenMAIN 20

/******************/
/**  Misc Stuff  **/
/******************/

/*#undef  max
#undef  min*/
#define zc_max(a,b)  ((a)>(b)?(a):(b))
#define zc_min(a,b)  ((a)<(b)?(a):(b))

//GameFlags
#define GAMEFLAG_TRYQUIT            0x01
#define GAMEFLAG_SCRIPTMENU_ACTIVE  0x02
#define GAMEFLAG_F6SCRIPT_ACTIVE    0x04
#define GAMEFLAG_RESET_GAME_LOOP    0x08
#define GAMEFLAG_NO_F6              0x10

#define DCLICK_START      0
#define DCLICK_RELEASE    1
#define DCLICK_AGAIN      2
#define DCLICK_NOT        3

template <class T>
INLINE T sign(T a)
{
    return T(a < 0 ? -1: 1);
}

template <class T>
INLINE T sign2(T a)
{
	if (a == 0) return 0;
    return T(a < 0 ? -1: 1);
}

//#ifndef NOZSWAP
template <class T>
static INLINE void zc_swap(T &a,T &b)
{
    T c = a;
    a = b;
    b = c;
}
//#endif

template <class T>
static INLINE bool is_between(T a, T b, T c, bool inclusive)
{
    if(a>b&&a<c)
    {
        return true;
    }
    
    if(inclusive&&(a==b||a==c))
    {
        return true;
    }
    
    return false;
}

#define NEWALLEGRO

INLINE bool pfwrite(void *p,int32_t n,PACKFILE *f)
{
    bool success=true;
    
    if(!fake_pack_writing)
    {
        success=(pack_fwrite(p,n,f)==n);
    }
    
    if(success)
    {
        writesize+=n;
    }
    
    return success;
}

INLINE bool pfread(void *p,int32_t n,PACKFILE *f,bool keepdata)
{
    bool success;
    
    if(keepdata==true)
    {
        success=(pack_fread(p,n,f)==n);
        
        if(success)
        {
            readsize+=n;
        }
        
        return success;
    }
    else
    {
        success=(pack_fseek(f,n)==0);
        
        if(success)
        {
            readsize+=n;
        }
        
        return success;
    }
}

INLINE bool p_getc(void *p,PACKFILE *f,bool keepdata)
{
    uint8_t *cp = (uint8_t *)p;
    int32_t c;
    
    if(!f) return false;
    
#ifdef NEWALLEGRO
    
    if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
    
#else
    
    if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
    
#endif
    
    if(pack_feof(f))
    {
        return false;
    }
    
    c = pack_getc(f);
    
    if(pack_ferror(f))
    {
        return false;
    }
    
    if(keepdata==true)
    {
        *cp = c;
    }
    
    readsize+=1;
    return true;
}

INLINE bool p_putc(int32_t c,PACKFILE *f)
{
    bool success=true;
    
    if(!fake_pack_writing)
    {
        if(!f) return false;
        
#ifdef NEWALLEGRO
        
        if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
        
#else
        
        if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
        
#endif
        
        pack_putc(c,f);
        success=(pack_ferror(f)==0);
    }
    
    if(success)
    {
        writesize+=1;
    }
    
    return success;
}

INLINE bool p_igetw(void *p,PACKFILE *f,bool keepdata)
{
    int16_t *cp = (int16_t *)p;
    int32_t c;
    
    if(!f) return false;
    
#ifdef NEWALLEGRO
    
    if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
    
#else
    
    if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
    
#endif
    
    if(pack_feof(f))
    {
        return false;
    }
    
    c = pack_igetw(f);
    
    if(pack_ferror(f))
    {
        return false;
    }
    
    if(keepdata==true)
    {
        *cp = c;
    }
    
    readsize+=2;
    return true;
}

INLINE bool p_iputw(int32_t c,PACKFILE *f)
{
    bool success=true;
    
    if(!fake_pack_writing)
    {
        if(!f) return false;
        
#ifdef NEWALLEGRO
        
        if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
        
#else
        
        if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
        
#endif
        
        pack_iputw(c,f);
        success=(pack_ferror(f)==0);
    }
    
    if(success)
    {
        writesize+=2;
    }
    
    return success;
}

INLINE bool p_igetl(void *p,PACKFILE *f,bool keepdata)
{
    dword *cp = (dword *)p;
    int32_t c;
    
    if(!f) return false;
    
#ifdef NEWALLEGRO
    
    if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
    
#else
    
    if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
    
#endif
    
    if(pack_feof(f))
    {
        return false;
    }
    
    c = pack_igetl(f);
    
    if(pack_ferror(f))
    {
        return false;
    }
    
    if(keepdata==true)
    {
        *cp = c;
    }
    
    readsize+=4;
    return true;
}

INLINE bool p_igetzf(void *p,PACKFILE *f,bool keepdata)
{
    zfix *cp = (zfix *)p;
    int32_t c;
    
    if(!f) return false;
    
#ifdef NEWALLEGRO
    
    if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
    
#else
    
    if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
    
#endif
    
    if(pack_feof(f))
    {
        return false;
    }
    
    c = pack_igetl(f);
    
    if(pack_ferror(f))
    {
        return false;
    }
    
    if(keepdata==true)
    {
        *cp = zslongToFix(c);
    }
    
    readsize+=4;
    return true;
}

INLINE bool p_igetd(void *p, PACKFILE *f, bool keepdata)
{
    int32_t temp;
    bool result = p_igetl(&temp,f,keepdata);
    *(int32_t *)p=(int32_t)temp;
    return result;
}

INLINE bool p_igetf(void *p,PACKFILE *f,bool keepdata)
{
    if(!f) return false;
    
#ifdef NEWALLEGRO
    
    if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
    
#else
    
    if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
    
#endif
    
    if(pack_feof(f))
    {
        return false;
    }
    
    byte tempfloat[sizeof(float)];
    
    if(!pfread(tempfloat,sizeof(float),f,true))
        return false;
        
    if(keepdata)
    {
        memset(p, 0,sizeof(float));
#ifdef ALLEGRO_MACOSX
        
        for(int32_t i=0; i<(int32_t)sizeof(float); i++)
        {
            ((byte *)p)[i] = tempfloat[i];
        }
        
#else
        
        for(int32_t i=0; i<(int32_t)sizeof(float); i++)
        {
            ((byte *)p)[sizeof(float)-i-1] = tempfloat[i];
        }
        
#endif
    }
    
    readsize += sizeof(float);
    return true;
}

INLINE bool p_iputl(int32_t c,PACKFILE *f)
{
    bool success=true;
    
    if(!fake_pack_writing)
    {
        if(!f) return false;
        
#ifdef NEWALLEGRO
        
        if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
        
#else
        
        if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
        
#endif
        
        pack_iputl(c,f);
        success=(pack_ferror(f)==0);
    }
    
    if(success)
    {
        writesize+=4;
    }
    
    return success;
}

INLINE bool p_iputzf(zfix const& c,PACKFILE *f)
{
    bool success=true;
    
    if(!fake_pack_writing)
    {
        if(!f) return false;
        
#ifdef NEWALLEGRO
        
        if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
        
#else
        
        if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
        
#endif
        
        pack_iputl(c.getZLong(),f);
        success=(pack_ferror(f)==0);
    }
    
    if(success)
    {
        writesize+=4;
    }
    
    return success;
}

INLINE bool p_mgetw(void *p,PACKFILE *f,bool keepdata)
{
    int16_t *cp = (int16_t *)p;
    int32_t c;
    
    if(!f) return false;
    
#ifdef NEWALLEGRO
    
    if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
    
#else
    
    if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
    
#endif
    
    if(pack_feof(f))
    {
        return false;
    }
    
    c = pack_mgetw(f);
    
    if(pack_ferror(f))
    {
        return false;
    }
    
    if(keepdata==true)
    {
        *cp = c;
    }
    
    readsize+=2;
    return true;
}

INLINE bool p_mputw(int32_t c,PACKFILE *f)
{
    bool success=true;
    
    if(!fake_pack_writing)
    {
        if(!f) return false;
        
#ifdef NEWALLEGRO
        
        if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
        
#else
        
        if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
        
#endif
        
        pack_mputw(c,f);
        success=(pack_ferror(f)==0);
    }
    
    if(success)
    {
        writesize+=2;
    }
    
    return success;
}

INLINE bool p_mgetl(void *p,PACKFILE *f,bool keepdata)
{
    dword *cp = (dword *)p;
    int32_t c;
    
    if(!f) return false;
    
#ifdef NEWALLEGRO
    
    if(f->normal.flags&PACKFILE_FLAG_WRITE) return false;     //must not be writing to file
    
#else
    
    if(f->flags&PACKFILE_FLAG_WRITE) return false;            //must not be writing to file
    
#endif
    
    if(pack_feof(f))
    {
        return false;
    }
    
    c = pack_mgetl(f);
    
    if(pack_ferror(f))
    {
        return false;
    }
    
    if(keepdata==true)
    {
        *cp = c;
    }
    
    readsize+=4;
    return true;
}

INLINE bool p_mputl(int32_t c,PACKFILE *f)
{
    bool success=true;
    
    if(!fake_pack_writing)
    {
        if(!f) return false;
        
#ifdef NEWALLEGRO
        
        if(!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false;  //must be writing to file
        
#else
        
        if(!(f->flags&PACKFILE_FLAG_WRITE)) return false;         //must be writing to file
        
#endif
        
        pack_mputl(c,f);
        success=(pack_ferror(f)==0);
    }
    
    if(success)
    {
        writesize+=4;
    }
    
    return success;
}

INLINE bool p_getcstr(std::string *str, PACKFILE *f, bool keepdata)
{
	if(keepdata)
		str->clear();
	byte sz = 0;
	if(!p_getc(&sz,f,keepdata))
		return false;
	if(sz) //string found
	{
		char dummy;
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getc(&dummy,f,keepdata))
				return false;
			if(keepdata)
				str->push_back(dummy);
		}
	}
	return true;
}
INLINE bool p_putcstr(std::string const& str, PACKFILE *f)
{
	byte sz = byte(zc_min(255,str.size()));
	if(!p_putc(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_putc(str.at(q),f))
				return false;
		}
	}
	return true;
}
INLINE bool p_getwstr(std::string *str, PACKFILE *f, bool keepdata)
{
	if(keepdata)
		str->clear();
	word sz = 0;
	if(!p_igetw(&sz,f,keepdata))
		return false;
	if(sz) //string found
	{
		char dummy;
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_getc(&dummy,f,keepdata))
				return false;
			if(keepdata)
				str->push_back(dummy);
		}
	}
	return true;
}
INLINE bool p_putwstr(std::string const& str, PACKFILE *f)
{
	word sz = word(zc_min(65535,str.size()));
	if(!p_iputw(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!p_putc(str.at(q),f))
				return false;
		}
	}
	return true;
}

template<typename T>
INLINE bool p_getcvec(std::vector<T> *vec, PACKFILE *f, bool keepdata)
{
	if(keepdata)
		vec->clear();
	byte sz = 0;
	if(!p_getc(&sz,f,keepdata))
		return false;
	if(sz) //vec found
	{
		T dummy;
		for(size_t q = 0; q < sz; ++q)
		{
			if(!pfread(&dummy,sizeof(T),f,keepdata))
				return false;
			if(keepdata)
				vec->push_back(dummy);
		}
	}
	return true;
}
template<typename T>
INLINE bool p_putcvec(std::vector<T> const& vec, PACKFILE *f)
{
	byte sz = byte(zc_min(255,vec.size()));
	if(!p_putc(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!pfwrite(&(vec.at(q)),sizeof(T),f))
				return false;
		}
	}
	return true;
}
template<typename T>
INLINE bool p_getwvec(std::vector<T> *vec, PACKFILE *f, bool keepdata)
{
	if(keepdata)
		vec->clear();
	word sz = 0;
	if(!p_igetw(&sz,f,keepdata))
		return false;
	if(sz) //vec found
	{
		T dummy;
		for(size_t q = 0; q < sz; ++q)
		{
			if(!pfread(&dummy,sizeof(T),f,keepdata))
				return false;
			if(keepdata)
				vec->push_back(dummy);
		}
	}
	return true;
}
template<typename T>
INLINE bool p_putwvec(std::vector<T> const& vec, PACKFILE *f)
{
	word sz = word(zc_min(65535,vec.size()));
	if(!p_iputw(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!pfwrite(&(vec.at(q)),sizeof(T),f))
				return false;
		}
	}
	return true;
}
template<typename T>
INLINE bool p_getlvec(std::vector<T> *vec, PACKFILE *f, bool keepdata)
{
	if(keepdata)
		vec->clear();
	dword sz = 0;
	if(!p_igetl(&sz,f,keepdata))
		return false;
	if(sz) //vec found
	{
		T dummy;
		for(size_t q = 0; q < sz; ++q)
		{
			if(!pfread(&dummy,sizeof(T),f,keepdata))
				return false;
			if(keepdata)
				vec->push_back(dummy);
		}
	}
	return true;
}
template<typename T>
INLINE bool p_putlvec(std::vector<T> const& vec, PACKFILE *f)
{
	dword sz = vec.size();
	if(!p_iputl(sz,f))
		return false;
	if(sz)
	{
		for(size_t q = 0; q < sz; ++q)
		{
			if(!pfwrite((void*)&(vec.at(q)), sizeof(T), f))
				return false;
		}
	}
	return true;
}

INLINE bool isinRect(int32_t x,int32_t y,int32_t rx1,int32_t ry1,int32_t rx2,int32_t ry2)
{
    return x>=rx1 && x<=rx2 && y>=ry1 && y<=ry2;
}

INLINE void SCRFIX()
{
    putpixel(screen,0,0,getpixel(screen,0,0));
}

// ack no, inline doesn't work this way -DD
//INLINE int32_t new_return(int32_t x) { fake_pack_writing=false; return x; }
#define new_return(x) {assert(x == 0); fake_pack_writing = false; return x; }

extern void flushItemCache();
extern void removeFromItemCache(int32_t itemclass);

#define GLOBAL_SCRIPT_INIT 			0
#define GLOBAL_SCRIPT_GAME			1
#define GLOBAL_SCRIPT_END			2
#define GLOBAL_SCRIPT_ONSAVELOAD	3
#define GLOBAL_SCRIPT_ONLAUNCH		4
#define GLOBAL_SCRIPT_ONCONTGAME	5
#define GLOBAL_SCRIPT_F6			6
#define GLOBAL_SCRIPT_ONSAVE		7

#define SCRIPT_PLAYER_INIT 1
#define SCRIPT_PLAYER_ACTIVE 2
#define SCRIPT_PLAYER_DEATH 3
#define SCRIPT_PLAYER_WIN 4

//Hero Internal Flags
#define LF_PAID_SWORD_COST		0x01
#define LF_PAID_WAND_COST		0x02
#define LF_PAID_CBYRNA_COST		0x04

#define RUNSCRIPT_OK			0
#define RUNSCRIPT_ERROR			1
#define RUNSCRIPT_SELFDELETE	2
#define RUNSCRIPT_STOPPED		3
#define RUNSCRIPT_SELFREMOVE	4

bool runscript_do_earlyret(int runscript_val);

#define CHAS_ATTRIB   0x01
#define CHAS_FLAG     0x02
#define CHAS_TRIG     0x04
#define CHAS_ANIM     0x08
#define CHAS_SCRIPT   0x10
#define CHAS_BASIC    0x20
#define CHAS_LIFT     0x40
#define CHAS_GENERAL  0x80

#define SCRHAS_ROOMDATA  0x00000001
#define SCRHAS_ITEM      0x00000002
#define SCRHAS_TWARP     0x00000004
#define SCRHAS_SWARP     0x00000008
#define SCRHAS_WARPRET   0x00000010
#define SCRHAS_LAYERS    0x00000020
#define SCRHAS_MAZE      0x00000040
#define SCRHAS_D_S_U     0x00000080
#define SCRHAS_FLAGS     0x00000100
#define SCRHAS_ENEMY     0x00000200
#define SCRHAS_CARRY     0x00000400
#define SCRHAS_SCRIPT    0x00000800
#define SCRHAS_UNUSED    0x00001000
#define SCRHAS_SECRETS   0x00002000
#define SCRHAS_COMBOFLAG 0x00004000
#define SCRHAS_MISC      0x00008000

#define until(n) while(!(n))
#define unless(n) if(!(n))
#define SETFLAG(v, fl, b)	if(b) v |= (fl); else v &= ~(fl)

struct viewport_t
{
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
	int32_t yofs;

	bool intersects_with(int x, int y, int w, int h) const;
	bool contains_point(int x, int y) const;
	int32_t left() const;
	int32_t right() const;
	int32_t top() const;
	int32_t bottom() const;
};

enum class ViewportMode
{
	CenterAndBound = 0,
	Center = 1,
	Script = 2,
	Max = Script,
};

enum //Mapscr hardcodes for temp mapscrs
{
	MAPSCR_SCROLL6 = -14,
	MAPSCR_SCROLL5,
	MAPSCR_SCROLL4,
	MAPSCR_SCROLL3,
	MAPSCR_SCROLL2,
	MAPSCR_SCROLL1,
	MAPSCR_SCROLL0,
	MAPSCR_TEMP6,
	MAPSCR_TEMP5,
	MAPSCR_TEMP4,
	MAPSCR_TEMP3,
	MAPSCR_TEMP2,
	MAPSCR_TEMP1,
	MAPSCR_TEMP0
};

//MIDI States
enum { midissuspNONE, midissuspHALTED, midissuspRESUME };

enum swStyle //Switchhook animation styles
{
	swPOOF, swFLICKER, swRISE
};

enum //Special hardcoded draw layers
{
	SPLAYER_PLAYER_DRAW = -200000, //The player, when this occurs changes based on various conditions.
	SPLAYER_EWEAP_BEHIND_DRAW, //Eweapons with 'Behind = true'
	SPLAYER_EWEAP_FRONT_DRAW, //Eweapons with 'Behind = false'
	SPLAYER_LWEAP_BEHIND_DRAW, //Lweapons with 'Behind = true'
	SPLAYER_LWEAP_FRONT_DRAW, //Lweapons with 'Behind = false'
	SPLAYER_LWEAP_ABOVE_DRAW, //Lweapons with (Z+FakeZ) > Init Data 'Jump Layer Height', IF the player is also above this height.
	SPLAYER_CHAINLINK_DRAW, //Hookshot chain links
	SPLAYER_NPC_DRAW, //Enemies
	SPLAYER_NPC_ABOVEPLAYER_DRAW, //Enemies 'grabbing' the player, or with a higher Z than the player
	SPLAYER_NPC_AIRBORNE_DRAW, //Flying enemies, or enemies in the Z axis (amount required QR dependant)
	SPLAYER_ITEMSPRITE_DRAW, //Itemsprites
	SPLAYER_FAIRYITEM_DRAW, //Moving fairies
	SPLAYER_PUSHBLOCK, //Pushable Blocks
	SPLAYER_MOVINGBLOCK, //Moving Pushable Blocks
	SPLAYER_OVERHEAD_CMB, //Overhead combos
	SPLAYER_OVERHEAD_FFC, //Overhead ffcs
	SPLAYER_DARKROOM_UNDER, //Under dark room darkness
	SPLAYER_DARKROOM_OVER, //Over dark room darkness
};

//

extern const char months[13][13];
char *VerStr(int32_t version);

RGB _RGB(byte *si);
RGB _RGB(int32_t r,int32_t g,int32_t b);
RGB invRGB(RGB s);
RGB mixRGB(int32_t r1,int32_t g1,int32_t b1,int32_t r2,int32_t g2,int32_t b2,int32_t ratio);

extern char tmp_themefile[2048];
char const* get_themefile();
void set_theme(char const* fpath);
void reset_theme();
void load_themefile(char const* fpath, PALETTE pal, ALLEGRO_COLOR* colors);
void load_themefile(char const* fpath);
void save_themefile(char const* fpath, PALETTE pal, ALLEGRO_COLOR* colors);
void save_themefile(char const* fpath);
void load_udef_colorset(App a, PALETTE pal, ALLEGRO_COLOR* colors);
void load_udef_colorset(App a);
void load_colorset(int32_t colorset, PALETTE pal, ALLEGRO_COLOR* colors);
void load_colorset(int32_t colorset);

void update_hw_screen(bool force = false);

bool valid_str(char const* ptr, char cancel = 0);

std::string get_dbreport_string();
std::string get_qr_hexstr(byte* qrs = NULL, bool hash = false, bool disctags = true);
bool clipboard_has_text();
bool get_al_clipboard(std::string& clipboard);
void set_al_clipboard(std::string const& clipboard);
bool load_qr_hexstr(std::string hexstr);
bool load_qr_hexstr_clipboard();
bool load_dev_info(std::string const& devstr);
bool load_dev_info_clipboard();
std::string generate_zq_about();

void enter_sys_pal();
void exit_sys_pal();

extern bool global_z3_scrolling_extended_height_mode;
extern viewport_t viewport;

enum {nswapDEC, nswapHEX, nswapLDEC, nswapLHEX, nswapBOOL, nswapMAX};


double WrapAngle(double radians);
double WrapDegrees(double degrees);
double DegreesToRadians(double deg);
double RadiansToDegrees(double rad);
double DirToRadians(int dir);
double DirToDegrees(int dir);
int32_t AngleToDir(double radians);
int32_t AngleToDir4(double degrees);
int32_t AngleToDir4Rad(double radians);

bool isNextType(int32_t type);
bool isWarpType(int32_t type);
int32_t getWarpLetter(int32_t type);
int32_t simplifyWarpType(int32_t type);
bool isStepType(int32_t type);
bool isDamageType(int32_t type);
bool isConveyorType(int32_t type);
bool isChestType(int32_t type);

DATAFILE* load_datafile_count(const char* path, size_t& sz);

enum
{
	ZSD_NONE = -1,
	ZSD_NPC, ZSD_LWPN, ZSD_EWPN, ZSD_ITSPR, ZSD_COMBODATA,
	ZSD_COMBOPOS, ZSD_FFC,
	NUM_ZSD
};

#define FONTSDAT_CNT (FONT_ZX+1)

#define SMART_WRAP(x, mod) ((x) < 0 ? (((mod)-(-(x)%(mod)))%(mod)) : ((x)%(mod)))
#define MEMCPY_ARR(dest,src) memcpy(dest,src,sizeof(dest))

#undef cmb1
#undef cmb2
#undef cmb3
#undef cmb4
#undef cmb5
#undef cmb6
#undef cmb7
#undef cmb8
#undef cmb9
#undef cmb10
#undef cmb11
#undef cmb12
#undef cmb13
#undef cmb14
#undef cmb15
#undef cmb16

#undef rad1
#undef rad2
#undef rad3
#undef rad4
#undef rad5
#undef rad6
#undef rad7
#undef rad8
#undef rad9
#undef rad10
#undef rad11
#undef rad12
#undef rad13
#undef rad14
#undef rad15
#undef rad16

#endif                                                      //_ZDEFS_H_


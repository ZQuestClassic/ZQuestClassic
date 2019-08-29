//modified isdungeon() in maps.c
//modified edge_of_dmap() in link.c

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





#ifndef _ZDEFS_H_
#define _ZDEFS_H_

#include <math.h>
#include "zc_alleg.h"
#include <string.h>

#define ZELDA_VERSION       0x0211                          //version of the program
#define VERSION_BUILD       13                              //build number of this version
#define IS_BETA             1                               //is this a beta?
#define DATE_STR            "January 1, 2006"

#define MIN_VERSION         0x0184

#define ZELDADAT_VERSION      0x0211                        //version of zelda.dat
#define ZELDADAT_BUILD        4                             //build of zelda.dat
#define SFXDAT_VERSION        0x0211                        //version of sfx.dat
#define SFXDAT_BUILD          13                            //build of sfx.dat
#define FONTSDAT_VERSION      0x0211                        //version of fonts.dat
#define FONTSDAT_BUILD        12                            //build of fonts.dat
#define QSTDAT_VERSION        0x0211                        //version of qst.dat
#define QSTDAT_BUILD          0                             //build of qst.dat
#define ZQUESTDAT_VERSION     0x0211                        //version of zquest.dat
#define ZQUESTDAT_BUILD       13                            //build of zquest.dat

enum {ENC_METHOD_192B104=0, ENC_METHOD_192B105, ENC_METHOD_192B185, ENC_METHOD_211B9, ENC_METHOD_MAX};


#ifdef ALLEGRO_MACOSX
#define KEY_ZC_LCONTROL KEY_COMMAND
#define KEY_ZC_RCONTROL KEY_COMMAND
#else
#define KEY_ZC_LCONTROL KEY_LCONTROL
#define KEY_ZC_RCONTROL KEY_RCONTROL
#endif

/*
  #ifndef ALLEGRO_WIP_VERSION
  #define KEY_SLASH_PAD KEY_SLASH2
  #endif
  */

#ifdef ALLEGRO_DOS
//already defined in DOS
#elif defined(ALLEGRO_WINDOWS)
#define PI 3.14159265358979323846
#elif defined(ALLEGRO_MACOSX)
#define PI 3.14159265358979323846
#elif defined(ALLEGRO_LINUX)
#define PI 3.14159265358979323846
#endif

#define QUEST_COUNT           3
#define HP_PER_HEART          16
#define DAMAGE_MULTIPLIER     2


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
#define ID_LINKSPRITES    ZC_ID('L','I','N','K')              //Link sprites
#define ID_SUBSCREEN      ZC_ID('S','U','B','S')              //quest header
#define ID_ITEMDROPSET    ZC_ID('D','R','O','P')              //quest header
#define ID_FFSCRIPT       ZC_ID('F','F','S','C')              //quest header
#define ID_SFX			  ZC_ID('S','F','X',' ')			  //sfx data

//Version number of the different section types
#define V_HEADER          3
#define V_RULES           1
#define V_STRINGS         1
#define V_MISC            3
#define V_TILES           1
#define V_COMBOS          6
#define V_CSETS           1
#define V_MAPS            11
#define V_DMAPS           2
#define V_DOORS           1
#define V_ITEMS           2
#define V_WEAPONS         3
#define V_COLORS          1
#define V_ICONS           1
#define V_GRAPHICSPACK    1
#define V_INITDATA        9
#define V_GUYS            3
#define V_MIDIS           1
#define V_CHEATS          1
#define V_SAVEGAME        4
#define V_COMBOALIASES    1
#define V_LINKSPRITES     2
#define V_SUBSCREEN       2
#define V_ITEMDROPSET     1
#define V_FFSCRIPT	      4
#define V_SFX             1

/*
  * Compatible version number of the different section types
  * Basically, the last version number that this section type
  * is just an extension of (ie. new variables are stuck on the end)
  * instead of a complete rewrite (or variables added to the middle).
  * If this and the version number are the same, then this is
  * a total reworking of the section and probably won't be able
  * to be read by anything that was written for a previous version.
  */
#define CV_HEADER         3
#define CV_RULES          1
#define CV_STRINGS        1
#define CV_MISC           3
#define CV_TILES          1
#define CV_COMBOS         1
#define CV_CSETS          1
#define CV_MAPS           9
#define CV_DMAPS          1
#define CV_DOORS          1
#define CV_ITEMS          1
#define CV_WEAPONS        1
#define CV_COLORS         1
#define CV_ICONS          1
#define CV_GRAPHICSPACK   1
#define CV_INITDATA       7
#define CV_GUYS           3
#define CV_MIDIS          1
#define CV_CHEATS         1
#define CV_SAVEGAME       1
#define CV_COMBOALIASES   1
#define CV_LINKSPRITES    1
#define CV_SUBSCREEN      2
#define CV_ITEMDROPSET    1
#define CV_FFSCRIPT       1
#define CV_SFX            1

extern int curr_tb_page;
extern bool triplebuffer_not_available;
extern int playing_field_offset;
extern int passive_subscreen_height;
extern int passive_subscreen_offset;

extern int CSET_SIZE;
extern int CSET_SHFT;

typedef unsigned char        byte;                               //0-                       255  ( 8 bits)
typedef unsigned short       word;                               //0-                    65,535  (16 bits)
typedef unsigned long        dword;                              //0-             4,294,967,295  (32 bits)
typedef unsigned long long   qword;                              //0-18,446,744,073,709,551,616  (64 bits)

extern int readsize, writesize;

// system colors
#define lc1(x) ((x)+192)                                    // offset to 'level bg color' x (row 12)
#define lc2(x) ((x)+208)                                    // offset to 'level fg color' x (row 13)
#define vc(x)  ((x)+224)                                    // offset to 'VGA color' x (row 14)
#define dvc(x) ((x)+240)                                    // offset to dark 'VGA color' x (row 15)
#define BLACK         253
#define WHITE         254

#define BYTE_FILTER 0xFF

#define SINGLE_TILE_SIZE    128
#define TILES_PER_ROW       20
#define TILE_ROWS_PER_PAGE  13
#define TILES_PER_PAGE      (TILES_PER_ROW*TILE_ROWS_PER_PAGE)
#define TILE_PAGES          252
#define OLDMAXTILES         (TILES_PER_PAGE*6)              // 1560 tiles
#define NEWMAXTILES         (TILES_PER_PAGE*TILE_PAGES)     // 32760 tiles
#define NEWTILE_SIZE2       (NEWMAXTILES*SINGLE_TILE_SIZE)  // 4193280 bytes (new packed format, 6 pages)
#define OLDTILE_SIZE2       (OLDMAXTILES*SINGLE_TILE_SIZE)  // 199680 bytes (new packed format, 6 pages)
// 133120 bytes (new packed format, 4 pages)
#define OLDTILE_SIZE        (TILES_PER_PAGE*4*SINGLE_TILE_SIZE)
//#define NEWTILE_SIZE      (260*6*128) // 199680 bytes (new packed format, 6 pages)
//#define OLDTILE_SIZE      (260*4*128) // 133120 bytes (new packed format, 4 pages)
#define TILEBUF_SIZE        (320*480)                       // 153600 bytes (old unpacked format)

#define COMBOS_PER_ROW      20

//#define MAGICDRAINRATE  2

// quest stuff
#define ZQ_TILES        0
#define ZQ_MIDIS2       1                                   //4 bytes
#define ZQ_CHEATS2       5
#define ZQ_MAXDATA      20
#define WAV_COUNT       128

#define MAXSCREENS 128
#define MAXCUSTOMMIDIS192b177 32                                  // uses bit string for midi flags, so 32 bytes
#define MAXCUSTOMMIDIS        252                                 // uses bit string for midi flags, so 32 bytes
#define MIDIFLAGS_SIZE  ((MAXCUSTOMMIDIS+7)>>3)

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

#define MAXQTS          256
#define MAXMSGS       65535
#define MAXDOORCOMBOSETS 256
#define MAXDMAPS        256                                 //this and
#define MAXLEVELS       256                                 //this should be the same number (was 32)
#define MAXITEMS        256
#define MAXWPNS         256
#define MAXGUYS         256
#define COMBOS_PER_PAGE 256
#define COMBO_PAGES     255
#define MAXCOMBOS       COMBO_PAGES*COMBOS_PER_PAGE
#define MAXSUBSCREENITEMS	256
#define MAXCUSTOMSUBSCREENS 128

#define MAGICPERBLOCK   32
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
#define fITEM           2
#define fDARK           4
#define fROAR           8
#define fWHISTLE        16
#define fLADDER         32
#define fMAZE           64
#define fSEA            128

// flags2
#define wfUP            1
#define wfDOWN          2
#define wfLEFT          4
#define wfRIGHT         8
#define fSECRET         16                                  // play "secret" sfx upon entering this screen
#define fVADER          32                                  // play "vader" sfx instead of "roar" sfx
#define fFLOATTRAPS     64
#define fCLEARSECRET    128                                 // clear all enemies to trigger secret entrance

// flags3
#define fHOLDITEM         1
#define fDODONGO          2                                 // play dodongo sfx instead of roar sfx
#define fINVISROOM        4
#define fINVISLINK        8
#define fNOSUBSCR         16
#define fIWARPFULLSCREEN  32                                // instawarps affect all sprites
#define fNOSECRETSOUND    64                                // never play secret sound on this screen
#define fENEMIESRETURN    128                               // enemies always return

//flags4
#define fOVERHEADTEXT     1
#define fITEMWARP         2
#define fTIMEDDIRECT      4                                 //Specifies timed pit warp
#define fDISABLETIME      8
#define fENEMYSCRTPERM    16
#define fNOITEMRESET      32
#define fSAVEROOM		  64
#define fAUTOSAVE		  128

//flags5
#define fRANDOMTIMEDWARP  1
#define fDAMAGEWITHBOOTS  2
#define fDIRECTAWARP	  4
#define fDIRECTSWARP	  8
#define fTEMPSECRETS	  16
// what is going on with 32?
#define fTOGGLEDIVING   64
#define fNOFFCARRYOVER  128

//flags6
#define fCAVEROOM		  1
#define fDUNGEONROOM	  2
#define fTRIGGERFPERM     4
#define fCONTINUEHERE	  8
#define fNOCONTINUEHERE   16
#define fTRIGGERF1631	  32
#define fTOGGLERINGDAMAGE 64
#define fWRAPAROUNDFF	  128

//flags7
#define fLAYER3BG		  1
#define fLAYER2BG		  2


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
#define ipHOLDUP        2                                   // Link holds up item when he gets it
#define ipONETIME       4                                   // Getting this item sets the map flag
#define ipDUMMY         8                                   // Dummy item.  Can't get this.
#define ipCHECK         16                                  // Check restrictions (money in a shop, etc.)
#define ipMONEY         32                                  // This item causes money loss (bomb upgrade, swindle room, etc.)
#define ipFADE          64                                  // Blinks if fadeclk>0
#define ipENEMY         128                                 // Enemy is carrying it around
#define ipTIMER         256                                 // Disappears after a while
#define ipBIGTRI        512                                 // Large collision rectangle (used for large triforce)
#define ipNODRAW        1024                                // Don't draw this (for underwater items)
#define ipTMPFLASH      2048                                // not used (delete this?)

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

// dmap flags
#define dmfCONTINUE      128
#define dmfTYPE          127

// map flags
enum
{
  mfNONE, mfPUSHUD, mfPUSH4, mfWHISTLE, mfBCANDLE, mfARROW, mfBOMB, mfFAIRY,
  mfRAFT, mfARMOS_SECRET, mfARMOS_ITEM, mfSBOMB, mfRAFT_BRANCH, mfDIVE_ITEM,
  mfNONAME, mfZELDA,
  mfSECRETS01, mfSECRETS02, mfSECRETS03, mfSECRETS04,       /*16*/
  mfSECRETS05, mfSECRETS06, mfSECRETS07, mfSECRETS08,
  mfSECRETS09, mfSECRETS10, mfSECRETS11, mfSECRETS12,
  mfSECRETS13, mfSECRETS14, mfSECRETS15, mfSECRETS16,
  mfTRAP_H, mfTRAP_V, mfTRAP_4, mfTRAP_LR, mfTRAP_UD,       /*32*/
  mfENEMY0, mfENEMY1, mfENEMY2, mfENEMY3, mfENEMY4,         /*37*/
  mfENEMY5, mfENEMY6, mfENEMY7, mfENEMY8, mfENEMY9,
  mfPUSHLR, mfPUSHU, mfPUSHD, mfPUSHL, mfPUSHR,
  mfPUSHUDNS, mfPUSHLRNS, mfPUSH4NS, mfPUSHUNS,
  mfPUSHDNS, mfPUSHLNS, mfPUSHRNS, mfPUSHUDINS,
  mfPUSHLRINS, mfPUSH4INS, mfPUSHUINS,
  mfPUSHDINS, mfPUSHLINS, mfPUSHRINS,
  mfBLOCKTRIGGER, mfNOBLOCKS, mfBRANG, mfMBRANG,
  mfFBRANG, mfSARROW, mfGARROW, mfRCANDLE, mfWANDFIRE, mfDINSFIRE,
  mfWANDMAGIC, mfREFMAGIC, mfREFFIREBALL, mfSWORD, mfWSWORD,
  mfMSWORD, mfXSWORD, mfSWORDBEAM, mfWSWORDBEAM, mfMSWORDBEAM,
  mfXSWORDBEAM, mfHOOKSHOT, mfWAND, mfHAMMER, mfSTRIKE, mfBLOCKHOLE,
  mfMAGICFAIRY, mfALLFAIRY, mfSINGLE, mfSINGLE16, 
  mfNOENEMY, mfICE, mfMAX, mfPUSHED
};

// combo types
enum
{
  cNONE, cSTAIR, cCAVE, cWATER, cARMOS, cGRAVE, cDOCK,
  cUNDEF, cPUSH_WAIT, cPUSH_HEAVY, cPUSH_HW, cL_STATUE, cR_STATUE,
  cWALKSLOW, cCVUP, cCVDOWN, cCVLEFT, cCVRIGHT, cSWIMWARP, cDIVEWARP,
  cLADDERHOOKSHOT, cTRIGNOFLAG, cTRIGFLAG, cZELDA, cSLASH, cSLASHITEM,
  cPUSH_HEAVY2, cPUSH_HW2, cPOUND, cHSGRAB, cHSBRIDGE, cDAMAGE1,
  cDAMAGE2, cDAMAGE3, cDAMAGE4, cC_STATUE, cTRAP_H, cTRAP_V, cTRAP_4,
  cTRAP_LR, cTRAP_UD, cPIT, cHOOKSHOTONLY, cOVERHEAD, cNOFLYZONE, cMIRROR,
  cMIRRORSLASH, cMIRRORBACKSLASH, cMAGICPRISM, cMAGICPRISM4,
  cMAGICSPONGE, cCAVE2, cEYEBALL_A, cEYEBALL_B, cNOJUMPZONE, cBUSH,
  cFLOWERS, cTALLGRASS, cSHALLOWWATER, cLOCKBLOCK, cLOCKBLOCK2,
  cBOSSLOCKBLOCK, cBOSSLOCKBLOCK2, cLADDERONLY, cBSGRAVE,
  cCHEST, cCHEST2, cLOCKEDCHEST, cLOCKEDCHEST2, cBOSSCHEST, cBOSSCHEST2,
  cRESET, cSAVE, cSAVE2, /*cVERTICAL,*/ cCAVEB, cCAVEC, cCAVED,
  cSTAIRB, cSTAIRC, cSTAIRD, cPITB, cPITC, cPITD,
  cCAVE2B, cCAVE2C, cCAVE2D, cSWIMWARPB, cSWIMWARPC, cSWIMWARPD,
  cDIVEWARPB, cDIVEWARPC, cDIVEWARPD, cSTAIRR, cPITR,
  cAWARPA, cAWARPB, cAWARPC, cAWARPD, cAWARPR,
  cSWARPA, cSWARPB, cSWARPC, cSWARPD, cSWARPR, cSTRIGNOFLAG, cSTRIGFLAG,
  cSTEP, cSTEPSAME, cSTEPALL, cSTEPCOPY, cNOENEMY, cBLOCKARROW1, cBLOCKARROW2,
  cBLOCKARROW3, cBLOCKBRANG1, cBLOCKBRANG2, cBLOCKBRANG3, cBLOCKSBEAM, cBLOCKALL,
  cBLOCKFIREBALL, cDAMAGE5, cDAMAGE6, cDAMAGE7, cCHANGE, cSPINTILE1, cSPINTILE2,
  cSCREENFREEZE, cSCREENFREEZEFF, cICE, cSLASHNEXT, cSLASHNEXTITEM, cBUSHNEXT,
  cMAX
};

#define QUESTRULES_SIZE 20

// "quest rules" flags (bit numbers in bit string)
enum
{
  qr_SOLIDBLK, qr_NOTMPNORET, qr_ALWAYSRET, qr_MEANTRAPS,
  qr_BSZELDA, qr_FADE, qr_FADECS5, qr_FASTDNGN,
  //1
  qr_NOLEVEL3FIX, qr_COOLSCROLL, qr_999R, qr_4TRI,
  qr_24HC, qr_FASTFILL, qr_VIEWMAP, qr_3TRI,
  //2
  qr_TIME, qr_FREEFORM, qr_KILLALL, qr_NOFLICKER,
  qr_CONTFULL, qr_RLFIX, qr_LENSHINTS, qr_LINKDUNGEONPOSFIX,
  //3
  qr_HOLDITEMANIMATION, qr_HESITANTPUSHBLOCKS, qr_HIDECARRIEDITEMS, qr_SASPARKLES,
  qr_GASPARKLES, qr_MBSPARKLES, qr_FBSPARKLES, qr_NOFLASHDEATH,
  //4
  qr_KEEPOLDITEMS, qr_FIREPROOFLINK, qr_OUCHBOMBS, qr_NOCLOCKS,
  qr_TEMPCLOCKS, qr_BRKBLSHLDS, qr_BRKNSHLDTILES, qr_MEANPLACEDTRAPS,
  //5
  qr_PHANTOMPLACEDTRAPS, qr_ALLOWFASTMSG, qr_LINKEDCOMBOS, qr_NOGUYFIRES,
  qr_HEARTRINGFIX, qr_NOHEARTRING, qr_DODONGOCOLORFIX, qr_SWORDWANDFLIPFIX,
  //6
  qr_ENABLEMAGIC, qr_MAGICWAND, qr_MAGICCANDLE, qr_MAGICBOOTS,
  qr_NONBUBBLEMEDICINE, qr_NONBUBBLEFAIRIES, qr_NONBUBBLETRIFORCE, qr_NEWENEMYTILES,
  //7
  qr_NOROPE2FLASH, qr_NOBUBBLEFLASH, qr_GHINI2BLINK, qr_WPNANIMFIX,
  qr_PHANTOMGHINI2, qr_Z3BRANG_HSHOT, qr_NOITEMMELEE, qr_SHADOWS,
  //8
  qr_TRANSSHADOWS, qr_QUICKSWORD, qr_BOMBHOLDFIX, qr_EXPANDEDLTM,
  qr_NOPOTIONCOMBINE, qr_LINKFLICKER, qr_SHADOWSFLICKER, qr_WALLFLIERS,
  //9
  qr_NOBOMBPALFLASH, qr_TRANSLUCENTNAYRUSLOVESHIELD, qr_FLICKERINGNAYRUSLOVESHIELD, qr_TRANSLUCENTNAYRUSLOVEROCKET,
  qr_FLICKERINGNAYRUSLOVEROCKET, qr_CMBCYCLELAYERS, qr_DMGCOMBOPRI, qr_WARPSIGNOREARRIVALPOINT,
  //10
  qr_LTTPCOLLISION, qr_LTTPWALK, qr_SLOWENEMYANIM, qr_TRUEARROWS, qr_NOSAVE, qr_NOCONTINUE,
  qr_QUARTERHEART, qr_NOARRIVALPOINT, qr_NOGUYPOOF, qr_ALLOWMSGBYPASS,
  //11  
  qr_NODIVING, qr_LAYER12UNDERCAVE, qr_NOSCROLLCONTINUE, qr_SMARTSCREENSCROLL,
  qr_RINGAFFECTDAMAGE, qr_ALLOW10RUPEEDROPS, qr_TRAPPOSFIX, qr_TEMPCANDLELIGHT,
  //12
  qr_REDPOTIONONCE, qr_OLDSTYLEWARP, qr_NOBORDER,qr_OLDTRIBBLES,
  
  qr_MAX
};

// room types
enum
{
  rNONE, rSP_ITEM, rINFO, rMONEY, rGAMBLE, rREPAIR, rRP_HC, rGRUMBLE,
  rTRIFORCE, rP_SHOP, rSHOP, rBOMBS, rSWINDLE, r10RUPIES, rWARP,
  rGANON, rZELDA, rITEMPOND, rMUPGRADE, rLEARNSLASH, rARROWS,
  rMAX
};

// directions
enum { up, down, left, right, l_up, r_up, l_down, r_down };

// refill stuff
enum { REFILL_LIFE, REFILL_MAGIC, REFILL_ALL};
enum { REFILL_BPOTION, REFILL_RPOTION, REFILL_FAIRY, REFILL_TRIFORCE};

// magic rates
#define LENSDRAINAMOUNT          2
#define LENSDRAINSPEED           1
#define BYRNADRAINAMOUNT         2
#define BYRNADRAINSPEED          .5
// It's a secret to everybody! ;)
#define WANDDRAINAMOUNT          8
#define CANDLEDRAINAMOUNT        4
#define DINSFIREDRAINAMOUNT     32
#define FARORESWINDDRAINAMOUNT  32
#define NAYRUSLOVEDRAINAMOUNT   64
#define BOOTSDRAINAMOUNT         1
#define BOOTSDRAINSPEED          1

//other
#define MAXDRUNKCLOCK 500

enum
{
  dBUSHLEAVES, dFLOWERCLIPPINGS, dGRASSCLIPPINGS, dHAMMERSMACK,
  dTALLGRASS, dRIPPLES, dNAYRUSLOVESHIELD, dMAXDECORATIONS
};

// items
enum                                                        // value matters because of current quest file data
{
  iRupy, i5Rupies, iHeart, iBombs, iClock,
  iSword, iWSword, iMSword, iShield, iKey,
  iBCandle, iRCandle, iLetter, iArrow, iSArrow,             /*10*/
  iBow, iBait, iBRing, iRRing, iBracelet,
  iTriforce, iMap, iCompass, iBrang, iMBrang,               /*20*/
  iWand, iRaft,iLadder,iHeartC, iBPotion,
  iRPotion, iWhistle,iBook, iMKey, iFairyMoving,            /*30*/
  iFBrang, iXSword, iMShield, i20Rupies, i50Rupies,
  i200Rupies, iWallet500, iWallet999, iPile, iBigTri,       /*40*/
  iSelectA, iMisc1, iMisc2, iSBomb, iHCPiece,
  iAmulet, iFlippers, iHookshot, iLens, iHammer,            /*50*/
  iBoots, iL2Bracelet, iGArrow, iMagicC, iSMagic,
  iLMagic, iGRing, iKillAll, iL2Amulet, iDinsFire,          /*60*/
  iFaroresWind, iNayrusLove, iBossKey, iBow2, iFairyStill,
  i1ArrowAmmo, i5ArrowAmmo, i10ArrowAmmo, i30ArrowAmmo, iQuiver,
  iQuiverL2, iQuiverL3, i1BombAmmo, i4BombAmmo, i8BombAmmo,
  i30BombAmmo, iBombBag, iBombBagL2, iBombBagL3,
  iLevelKey, iSelectB, i10Rupies, i100Rupies, iCByrna, iLongshot,
  i90,
  iMax=256
};

// item sets
enum
{
  isNONE, isDEFAULT, isBOMBS, isMONEY, isLIFE, isBOMB100, isSBOMB100,
  isMAGIC, isMAGICBOMBS, isMAGICMONEY, isMAGICLIFE, isMAGIC2
};

// weapons (in qst data)
enum
{
  wSWORD,wWSWORD,wMSWORD,wXSWORD,wBRANG,wMBRANG,wFBRANG,wBOMB,
  wSBOMB,wBOOM,wARROW,wSARROW,wFIRE,wWIND,wBAIT,wWAND,wMAGIC,
  ewFIREBALL,ewROCK,ewARROW,ewSWORD,ewMAGIC,iwSpawn,iwDeath,iwSwim,
  wHAMMER, wHSHEAD, wHSCHAIN_H, wHSHANDLE, wSSPARKLE, wGSPARKLE,
  wMSPARKLE, wFSPARKLE, iwHammerSmack, wGARROW, ewFLAME, ewWIND, iwMMeter,
  wDINSFIRE1A, wDINSFIRE1B, wDINSFIRES1A, wDINSFIRES1B, wHSCHAIN_V,
  iwMore, iwBossMarker, iwLinkSlash, wSWORDSLASH,wWSWORDSLASH,
  wMSWORDSLASH,wXSWORDSLASH,iwShadow,iwLargeShadow,iwBushLeaves,
  iwFlowerClippings, iwGrassClippings, iwTallGrass, iwRipples, iwNPCs,
  wNAYRUSLOVE1A, wNAYRUSLOVE1B, wNAYRUSLOVES1A, wNAYRUSLOVES1B,
  wNAYRUSLOVE2A, wNAYRUSLOVE2B, wNAYRUSLOVES2A, wNAYRUSLOVES2B,
  iwNayrusLoveShieldFront, iwNayrusLoveShieldBack, iwSubscreenVine, wCBYRNA, wCBYRNASLASH,
  wLSHEAD, wLSCHAIN_H, wLSHANDLE, wLSCHAIN_V, wSBOOM, ewBOMB, ewSBOMB,
  ewBOOM, ewSBOOM, ewFIRETRAIL, ewFLAME2, ewFLAME2TRAIL, ewICE, w84, wMAX=256
};

// weapon types in game engine
enum
{
  wNone,wSword,wBeam,wBrang,wBomb,wSBomb,wLitBomb,wLitSBomb,wArrow,
  wFire,wWhistle,wBait,wWand,wMagic,wCatching,wWind,wRefMagic,wRefFireball,
  wHammer, wHookshot,
  wHSHandle, wHSChain, wSSparkle, wGSparkle, wMSparkle, wFSparkle,
  wSmack, wGArrow, ewFlame, ewWind, wPhantom, wCByrna,
  wEnemyWeapons,
  ewFireball,ewArrow,ewBrang,ewSword,ewRock,ewMagic,ewBomb,ewSBomb,
  ewLitBomb,ewLitSBomb,ewFireTrail,ewFlame2,ewFlame2Trail,ewIce
};

// phantom weapon types
enum
{
  pDINSFIREROCKET, pDINSFIREROCKETRETURN, pDINSFIREROCKETTRAIL, pDINSFIREROCKETTRAILRETURN, pMESSAGEMORE,
  pNAYRUSLOVEROCKET1, pNAYRUSLOVEROCKETRETURN1, pNAYRUSLOVEROCKETTRAIL1, pNAYRUSLOVEROCKETTRAILRETURN1,
  pNAYRUSLOVEROCKET2, pNAYRUSLOVEROCKETRETURN2, pNAYRUSLOVEROCKETTRAIL2, pNAYRUSLOVEROCKETTRAILRETURN2
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
  eGRAPBUGHP, eGRAPBUGMP, 
  
  eMAXGUYS
};

// enemy families
enum
{
  eeGUY, eeWALK, eeSHOOT, eeTEK, eeLEV, eePEAHAT, eeZORA, eeROCK,
  eeGHINI, eeARMOS, eeKEESE, eeGEL, eeZOL, eeROPE, eeGORIYA, eeTRAP,
  eeWALLM, eeBUBBLE, eeVIRE, eeLIKE, eePOLSV, eeWIZZ, eeAQUA, eeMOLD,
  eeDONGO, eeMANHAN, eeGLEEOK, eeDIG, eeGHOMA, eeLANM, eePATRA, eeGANON,
  eePROJECTILE, eeGELTRIB, eeZOLTRIB, eeVIRETRIB, eeKEESETRIB, eeSPINTILE, eeNONE
};

// enemy animation styles
enum
{
  aNONE, aFLIP, aFLIPSLOW, a2FRM, a2FRMSLOW, aOCTO, aTEK, aLEV, aWALK, aZORA, aNEWZORA, aGHINI,
  aARMOS, aROPE, aWALLM, aNEWWALLM, aDWALK, aVIRE, a3FRM, aWIZZ, aAQUA,
  aDONGO, aMANHAN, aGLEEOK, aDIG, aGHOMA, aLANM, a2FRMPOS, a4FRM4EYE,
  a4FRM8EYE, a4FRM4DIRF, a4FRM4DIR, a4FRM8DIRF, aARMOS4, a4FRMPOS4DIR, a4FRMPOS8DIR,
  a4FRM3TRAP, a4FRM8DIRB, aNEWTEK, aNEWPOLV, a2FRM4DIR, aNEWLEV, aNEWDWALK,
  aNEWWIZZ, aNEWDONGO, aDONGOBS, a4FRMPOS8DIRF, a4FRMPOS4DIRF, a4FRMNODIR, aGANON, a2FRMB
};

// enemy patters
enum { pRANDOM, pSIDES };

enum { tfInvalid=0, tf4Bit, tf8Bit, tf16Bit, tf24Bit, tf32Bit, tfMax };

#define OLDITEMCNT i90
#define OLDWPNCNT  w84
#define ITEMCNT   iMax
#define WPNCNT    wMAX

typedef struct tiledata
{
  byte format;
  byte *data;
} tiledata;

typedef struct itemdata
{
  word tile;
  byte misc;                                                // 0000vhtf (vh:flipping, t:two hands, f:flash)
  byte csets;                                               // ffffcccc (f:flash cset, c:cset)
  byte frames;                                              // animation frame count
  byte speed;                                               // animation speed
  byte delay;                                               // extra delay factor (-1) for first frame
  long ltm;                                                 // Link Tile Modifier
  byte family;												// What family the item is in
  byte fam_type;											// What type in this family the item is
  byte set_gamedata;										// Whether this item sets the corresponding gamedata value or not
  word script;												// Which script the item is using
  char count;
  word amount;
  short setmax;
  word max;
  byte playsound;
  word collect_script;
//  byte exp[10];                                             // not used
  long initiald[8];
  byte initiala[2];
} itemdata;

typedef struct wpndata
{
  word tile;
  byte misc;                                                // 0000vhff (vh:flipping, f:flash (1:NES, 2:BSZ))
  byte csets;                                               // ffffcccc (f:flash cset, c:cset)
  byte frames;                                              // animation frame count
  byte speed;                                               // animation speed
  byte type;                                                // used by certain weapons
//  byte wpn_type;
//  word script;
//  byte exp;                                                 // not used
} wpndata;

typedef struct quest_template
{
  char name[31];
  char path[2048];
  //311 bytes
} quest_template;

typedef struct item_drop_object
{
  char name[64];
  byte items;
  word item[10];
  word chance[11]; //0=none
} item_drop_object;

#define guy_bhit        0x00000001
#define guy_invisible   0x00000002
#define guy_neverret    0x00000004
#define guy_superman    0x00000008

#define guy_sbombonly   0x00000010
#define inv_brang       0x00000020
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

#define inv_front       0x01000000
#define inv_left        0x02000000
#define inv_right       0x04000000
#define inv_back        0x08000000

#define weak_whistle    0x10000000
#define weak_arrow      0x20000000
#define weak_L3brang    0x40000000
#define lens_only       0x80000000

#define guy_flashing    0x00000001

//FF combo flags

#define ffOVERLAY       0x00000001
#define ffTRANS         0x00000002
#define ffSOLID         0x00000004
#define ffCARRYOVER     0x00000008
#define ffSTATIONARY    0x00000010

//FF combo changer flags

#define ffSWAPNEXT      0x80000000
#define ffSWAPPREV      0x40000000
#define ffCHANGENEXT    0x20000000
#define ffCHANGEPREV    0x10000000 
#define ffCHANGETHIS    0x08000000

typedef struct guydata
{
  dword flags;
  dword flags2;
  word  tile;
  byte  width;
  byte  height; //0=striped, 1+=rectangular
  word  s_tile; //secondary (additional) tile(s)
  byte  s_width;
  byte  s_height;  //0=striped, 1+=rectangular
  word  e_tile;
  byte  e_width;
  byte  e_height;

  short hp;

  short  family, cset, anim, e_anim, frate, e_frate;
  short  dp, wdp, weapon;

  short  rate, hrate, step, homing, grumble;
  short  item_set, misc1, misc2, misc3, misc4, misc5, misc6, misc7, misc8, misc9, misc10, bgsfx, bosspal;

  //  short  startx, starty;
  //  short  foo1,foo2,foo3,foo4,foo5,foo6;
  // 56 bytes
} guydata;

typedef struct mapscr
{
  byte valid;
  byte guy;
  word str;
  byte room;
  byte item;
  byte tilewarptype[4];
  word door_combo_set;
  byte warpreturnx[4];
  byte warpreturny[4];
  byte warpreturnc;
  byte stairx;
  byte stairy;
  byte itemx;
  byte itemy;
  byte color;
  byte enemyflags;
  byte door[4];
  byte tilewarpdmap[4];
  byte tilewarpscr[4];
  byte exitdir;
  word enemy[10];
  byte pattern;
  byte sidewarptype[4];
  byte warparrivalx;
  byte warparrivaly;
  byte path[4];
  byte sidewarpscr[4];
  byte sidewarpdmap[4];
  byte sidewarpindex;
  word undercombo;
  byte undercset;
  word catchall;
  byte flags;
  byte flags2;
  byte flags3;
  byte flags4;
  byte flags5;
  byte flags6;
  byte flags7;
  byte flags8;
  byte flags9;
  byte flags10;
  byte csensitive;
  word noreset;
  word nocarry;
  byte layermap[6];
  byte layerscreen[6];
  //  byte layerxsize[6];
  //  byte layerxspeed[6];
  //  byte layerxdelay[6];
  //  byte layerysize[6];
  //  byte layeryspeed[6];
  //  byte layerydelay[6];
  byte layeropacity[6];
  word timedwarptics;
  byte nextmap;
  byte nextscr;
  word secretcombo[128];
  byte secretcset[128];
  byte secretflag[128];
  word data[16*11];
  byte sflag[16*11];
  byte cset[16*11];
  dword numff;
  word ffdata[32];
  byte ffcset[32];
  word ffdelay[32];
  long ffx[32];
  long ffy[32];
  long ffxdelta[32];
  long ffydelta[32];
  long ffxdelta2[32];
  long ffydelta2[32];
  dword ffflags[32];
  byte ffwidth[32];
  byte ffheight[32];
  byte fflink[32];
  word ffscript[32];
  long d[32][8];
  long a[32][2];
  word pc[32];
  dword scriptflag[32];
  byte sp[32]; //stack pointer
  //long stack[32][256]; //stacks
  byte itemref[32];
  byte ffcref[32];
  byte itemclass[32];
  byte lwpnref[32];
  byte lwpnclass[32];
  byte ewpnref[32];
  byte ewpnclass[32];
  byte guyref[32];
  byte guyclass[32];
  long map_stack[256];
  long map_d[8];
  word map_pc;
  dword map_scriptflag;
  byte map_sp;
  byte map_itemref;
  byte map_itemclass;
  byte map_lwpnref;
  byte map_lwpnclass;
  byte map_ewpnref;
  byte map_ewpnclass;
  byte map_guyref;
  byte map_guyclass;
  byte map_ffcref;
  word script_entry;
  word script_occupancy;
  word script_exit;

  // for importing older quests...
  byte old_cpage;
  short screen_midi;

} mapscr;

typedef struct ffscript
{
	word command;
	long arg1;
	long arg2;
} ffscript;

enum
{
  sBCANDLE, sARROW, sBOMB, sSTAIRS, sSECRET01, sSECRET02, sSECRET03,
  sSECRET04, sSECRET05, sSECRET06, sSECRET07, sSECRET08, sSECRET09,
  sSECRET10, sSECRET11, sSECRET12, sSECRET13, sSECRET14, sSECRET15,
  sSECRET16, sRCANDLE, sWANDFIRE, sDINSFIRE, sSARROW, sGARROW,
  sSBOMB, sBRANG, sMBRANG, sFBRANG, sWANDMAGIC, sREFMAGIC, sREFFIREBALL,
  sSWORD, sWSWORD, sMSWORD, sXSWORD, sSWORDBEAM, sWSWORDBEAM,
  sMSWORDBEAM, sXSWORDBEAM, sHOOKSHOT, sWAND, sHAMMER, sSTRIKE
};

enum {cfOFFSET, cfMAX};

typedef struct newcombo
{
  word tile;
  byte flip;
  byte walk;
  byte type;
  byte csets;
  word foo;                                                 //do not change!  used for positioning!  no idea why.
  byte frames;
  byte speed;
  word nextcombo;
  byte nextcset;
  byte flag;
  byte skipanim;
  word nexttimer;
  byte skipanimy;
  byte freshanim;
  byte expansion[6];
  //24
} newcombo;

typedef struct tiletype
{
  byte bitplanes;
  byte *data;
} tiletype;

typedef struct ZCHEATS
{
  dword flags;
  char  codes[4][41];
  //168
} ZCHEATS;

#define QH_IDSTR    "AG Zelda Classic Quest File\n "
#define QH_NEWIDSTR "AG ZC Enhanced Quest File\n   "
#define ENC_STR     "Zelda Classic Quest File"

typedef struct zquestheader
{
  char  id_str[31];
  //32
  short zelda_version;
  word  internal;
  byte  quest_number;
  byte  old_rules[2];
  byte  old_map_count;
  char  old_str_count;
  //41
  byte  data_flags[ZQ_MAXDATA];
  byte  old_rules2[2];
  char  old_options;
  char  version[9];
  //73
  char  title[65];
  char  author[65];
  //byte  padding;
  //204
  //  short pwdkey;
  bool  dirty_password;
  char  password[256];
  unsigned char pwd_hash[16];
  //236
  char  minver[9];
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
  char  templatepath[2048];
  //602
} zquestheader;

enum { msLINKED };

typedef struct MsgStr
{
  char s[73];
  //byte padding
  //74
  word nextstring;
  word expansion[16];
  //108
} MsgStr;

enum {dt_pass=0, dt_lock, dt_shut, dt_boss, dt_olck, dt_osht, dt_obos, dt_wall, dt_bomb, dt_walk, dt_max};
enum {df_walktrans=0};

typedef struct DoorComboSet
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
} DoorComboSet;

typedef struct dmap
{
  byte map;
  byte level;
  char xoff;
  byte compass;
  byte color;
  byte midi;
  byte cont;
  byte type;
  //8
  byte grid[8];
  //16
  char name[21];
  char title[21];
  char intro[73];
  //byte padding;
  //132
  word minimap_1_tile;                                      //before getting map
  byte minimap_1_cset;                                      //cset for minimap 1
                                                            //byte padding;
  word minimap_2_tile;                                      //after getting map
  byte minimap_2_cset;                                      //cset for minimap 2
                                                            //byte padding;
                                                            //140
  word largemap_1_tile;                                     //large map
  byte largemap_1_cset;                                     //cset for large
                                                            //byte padding;
  word largemap_2_tile;                                     //large map
  byte largemap_2_cset;                                     //cset for large
  char tmusic[56];
  byte tmusictrack;
  byte active_subscreen;
  byte passive_subscreen;
  int emusic;
  //byte padding;
  //204
} dmap;

#define MAXCOMBOALIASES 256

typedef struct combo_alias
{
  byte width;      // Max 15
  byte height;     // Max 10
  byte layermask;  // Specifies layers to be drawn
  word combo;      // Optional thumbnail combo for the alias list
  word cset;
  word *combos;    // Dynamic array. Will be stored in quest.
  byte *csets;
} combo_alias;

typedef struct shoptype
{
  byte item[3];
  byte d1;
  word price[3];
  //10
} shoptype;

typedef struct pondtype
{
  byte olditem[3];
  byte d1;
  byte newitem[3];
  byte fairytile;
  byte aframes;
  byte aspeed;
  word msg[15];
  byte foo[32];
  //72
} pondtype;

typedef struct infotype192b145
{
  byte str[3];
  byte d1;
  word price[3];
  //10
} infotype192b145;

typedef struct infotype
{
  word str[3];
  byte d1;
  //byte padding;
  word price[3];
  //14
} infotype;

typedef struct warpring
{
  byte dmap[8];
  byte scr[8];
  byte size;
  byte d1;
  //18
} warpring;

typedef struct windwarp
{
  byte dmap;
  byte scr;

  //2
} windwarp;

typedef struct zcolors
{
  byte text, caption;
  byte overw_bg, dngn_bg;
  byte dngn_fg, cave_fg;
  byte bs_dk, bs_goal;
  byte compass_lt, compass_dk;
  //10
  byte subscr_bg, subscr_shadow, triframe_color;
  byte bmap_bg,bmap_fg;
  byte link_dot;
  //15
  byte triforce_cset;
  byte triframe_cset;
  byte overworld_map_cset;
  byte dungeon_map_cset;
  byte blueframe_cset;
  //20
  word triforce_tile;
  word triframe_tile;
  word overworld_map_tile;
  word dungeon_map_tile;
  word blueframe_tile;
  //30
  word HCpieces_tile;
  byte HCpieces_cset;
  byte foo[7];
  //40
  byte foo2[256];
  //296 bytes
} zcolors;

typedef struct palcycle
{
  byte first,count,speed;
  //3
} palcycle;

typedef struct miscQdata
{
  shoptype shop[16];
  //160 (160=10*16)
  infotype info[16];
  //384 (224=14*16)
  warpring warp[8];
  //528 (144=18*8)
  palcycle cycles[256][3];
  //2832 (2304=3*256*3)
  windwarp wind[9];                                         // destination of whirlwind for each level
                                                            //2850 (18=2*2)
  byte     triforce[8];                                     // positions of triforce pieces on subscreen
                                                            //2858 (8)
  zcolors  colors;
  //3154 (296)
  word     icons[4];
  //3162 (8=2*4)
  pondtype pond[16];
  //4314 (1152=72*16)
  word endstring;
  //  byte dummy;  // left over from a word
  word expansion[98];
  //4512
} miscQdata;

typedef struct zcmidi
{
  char title[20];
  //20
  long start;
  long loop_start;
  long loop_end;
  //32
  short loop;
  short volume;
  //36
  MIDI *midi;
  //40
} zcmidi;

typedef struct emusic
{
  char title[20];
  char filename[256];
} emusic;

enum // used for gamedata ITEMS
{
  itype_sword, itype_brang, itype_arrow, itype_candle, itype_whistle,
  itype_bait, itype_letter, itype_potion, itype_wand, itype_ring,
  itype_wallet, itype_amulet, itype_shield, itype_bow, itype_raft,
  itype_ladder, itype_book, itype_magickey, itype_bracelet,
  itype_flippers, itype_boots, itype_hookshot, itype_lens, itype_hammer,
  itype_dinsfire, itype_faroreswind, itype_nayruslove, itype_bomb,
  itype_sbomb, itype_clock, itype_key, itype_magiccontainer,
  itype_triforcepiece, itype_map, itype_compass, itype_bosskey,
  itype_quiver, itype_lkey, itype_cbyrna, 
  itype_rupee, itype_arrowammo, itype_fairy, itype_magic, itype_heart,
  itype_heartcontainer, itype_heartpiece, itype_killem, itype_bombammo, itype_bombbag,
  itype_max=255
};

enum {i_sword=1, i_wsword, i_msword, i_xsword, imax_sword};
enum {i_wbrang=1, i_mbrang, i_fbrang, imax_brang};
enum {i_warrow=1, i_sarrow, i_garrow, imax_arrow};
enum {i_bcandle=1, i_rcandle, imax_candle};
enum {i_recorder=1, imax_whistle};
enum {i_bait=1, imax_bait};
enum {i_letter=1, i_letter_used, imax_letter};
enum {i_bpotion=1, i_rpotion, imax_potion};
enum {i_wand=1, imax_wand};
enum {i_bring=1, i_rring, i_gring, imax_ring};
enum {i_swallet=1, i_lwallet, imax_wallet};
enum {i_amulet1=1, i_amulet2, imax_amulet};
enum {i_smallshield=1, i_largeshield, i_mirrorshield, imax_shield};
enum {i_shortbow=1, i_longbow, imax_bow};
enum {i_raft=1, imax_raft};
enum {i_ladder=1, imax_ladder};
enum {i_book=1, imax_book};
enum {i_magickey=1, imax_magickey};
enum {i_bracelet1=1, i_bracelet2, imax_bracelet};
enum {i_flippers=1, imax_flippers};
enum {i_boots=1, imax_boots};
enum {i_hookshot=1, i_longshot, imax_hookshot};
enum {i_lens=1, imax_lens};
enum {i_hammer=1, imax_hammer};
enum {i_dinsfire=1, imax_dinsfire};
enum {i_faroreswind=1, imax_faroreswind};
enum {i_nayruslove=1, imax_nayruslove};
enum {i_quiver=1, i_quiverl2, i_quiverl3, imax_quiver};
enum {i_cbyrna=1, imax_cbyrna};
enum {i_rupee=1, i_5rupee, i_10rupee, i_20rupee, i_50rupee, i_100rupee, i_200rupee, imax_rupee};
enum {i_arrowa=1, i_5arrowa, i_10arrowa, i_30arrowa, imax_arrowa};
enum {i_bomba=1, i_4bomba, i_8bomba, i_30bomba, imax_bomba};
enum {i_bombbag1=1, i_bombbag2, i_bombbag3, imax_bombbag}; 

//enum {i_clock=1, imax_clock};

typedef struct gamedata
{
  char  _name[9];
  byte  _quest;
  //10
  //word _life,_maxlife;
  //short _drupy;
  //word _rupies,_arrows,_maxarrows,
  word _deaths;
  //20
  //byte  _keys,_maxbombs,
  byte  /*_wlevel,*/_cheat;
  //24
  byte  items[MAXITEMS];
  byte  items_off[MAXITEMS];
  //280
  word _maxcounter[32];	// 0 - life, 1 - rupees, 2 - bombs, 3 - arrows, 4 - magic, 5 - keys, 6-super bombs
  word _counter[32];
  short _dcounter[32];

  char  version[9];
  char  title[65];
  //354
  byte  _hasplayed;
  //byte  padding;
  //356
  dword _time;
  //360
  byte  _timevalid;
  byte  lvlitems[256];
  byte  lvlkeys[256];
  //byte  _HCpieces;
  byte  _continue_scrn;
  byte  _continue_dmap;
  //620
  /*word  _maxmagic, _magic;
  short _dmagic;*/
  //byte  _magicdrainrate;
  //byte  _canslash;                                           //Link slashes instead of stabs.
  byte _generic[256];	// Generic gamedata. 0 - Heart pieces, 1- magicdrainrate, 2-canslash, 3-wlevel
                                                             //byte  padding[2];
                                                             //636
  byte  visited[MAXDMAPS];
  //892 (256)
  byte  bmaps[MAXDMAPS*64];                                 // the dungeon progress maps
                                                            //17276 (16384)
  word  maps[MAXMAPS2*128];                                 // info on map changes, items taken, etc.
                                                            //82556 (65280)
  byte  guys[MAXMAPS2*128];                                 // guy counts (though dungeon guys are reset on entry)
                                                            //115196 (32640)
  char  qstpath[2048];
  byte  icon[128];
  byte  pal[48];
  long  screen_d[MAXDMAPS*64][8];                           // script-controlled screen variables
  long  global_d[256];                                      // script-controlled global variables
  //115456 (260)
} gamedata;


// "initialization data" flags (bit numbers in bit string)
enum
{
  idE_RAFT, idE_LADDER, idE_BOOK, idE_KEY,
  idE_FLIPPERS, idE_BOOTS, idE_MAX
};
enum
{
  idI_WAND, idI_LETTER, idI_LENS, idI_HOOKSHOT,
  idI_BAIT, idI_HAMMER, idI_CBYRNA, idI_MAX
};
enum { idI_DFIRE, idI_FWIND, idI_NLOVE, idI_MAX2 };
enum { idM_CONTPERCENT, idM_DOUBLEMAGIC, idM_CANSLASH, idM_MAX };
enum
{
  idBP_WOODENPERCENT, idBP_WHITEPERCENT,
  idBP_MAGICPERCENT, idBP_MASTERPERCENT, idBP_MAX
};

typedef struct zinitdata
{
  byte raft, ladder, book, key, flippers, boots;
  byte ring, sword, shield, wallet, bracelet, amulet, bow;
  byte quiver;
  byte more_eq[32];
  //45
  byte candle, boomerang, arrow, potion, whistle, bombs, super_bombs;
  byte wand, letter, lens, hookshot, bait, hammer, dins_fire, farores_wind;
  byte nayrus_love, cloak, cbyrna;
  byte more_items[32];
  //94
  byte hc, start_heart, cont_heart, hcp, max_bombs, keys;
  byte arrows, max_arrows;
  word rupies;
  byte triforce;                                            // bit flags
  byte map[32];
  byte compass[32];
  byte boss_key[32];
  byte misc[16];
  byte sword_hearts[4];
  byte last_map;                                            //last map worked on
                                                            //220
  byte last_screen;                                         //last screen worked on
  byte max_magic;
  byte magic;
  byte beam_hearts[4];
  byte beam_percent;                                        //bit flags
                                                            //228
  byte beam_power[4];
  byte moving_fairy_hearts;
  byte moving_fairy_heart_percent;
  byte stationary_fairy_hearts;
  byte stationary_fairy_heart_percent;
  byte moving_fairy_magic;
  byte moving_fairy_magic_percent;
  byte stationary_fairy_magic;
  byte stationary_fairy_magic_percent;
  byte blue_potion_hearts;
  byte blue_potion_heart_percent;
  byte red_potion_hearts;
  byte red_potion_heart_percent;
  byte blue_potion_magic;
  byte blue_potion_magic_percent;
  byte red_potion_magic;
  byte red_potion_magic_percent;
  byte hookshot_links;
  byte hookshot_length;
  byte longshot_links;
  byte longshot_length;
  byte msg_more_x, msg_more_y;
  byte subscreen;
  byte start_dmap;
  byte linkanimationstyle;
  //238
  //byte expansion[98];
  //336 bytes total
  byte level_keys[256];
  int ss_grid_x;
  int ss_grid_y;
  int ss_grid_xofs;
  int ss_grid_yofs;
  int ss_grid_color;
  int ss_bbox_1_color;
  int ss_bbox_2_color;
  int ss_flags;
  byte subscreen_style;
  byte usecustomsfx;
  word max_rupees, max_keys;
} zinitdata;


/******************/
/**  Misc Stuff  **/
/******************/

#undef  max
#undef  min
#define max(a,b)  ((a)>(b)?(a):(b))
#define min(a,b)  ((a)<(b)?(a):(b))

#define DCLICK_START      0
#define DCLICK_RELEASE    1
#define DCLICK_AGAIN      2
#define DCLICK_NOT        3

template <class T>
  static INLINE void swap(T &a,T &b)
{
  T c = a;
  a = b;
  b = c;
}

#define NEWALLEGRO

INLINE bool pfwrite(void *p,long n,PACKFILE *f)
{
  bool success=(pack_fwrite(p,n,f)==n);
  if (success)
  {
    writesize+=n;
  }
  return success;
}

INLINE bool pfread(void *p,long n,PACKFILE *f,bool keepdata)
{
  bool success;
  if (keepdata==true)
  {
    success=(pack_fread(p,n,f)==n);
    if (success)
    {
      readsize+=n;
    }
    return success;
  }
  else
  {
    bool success=(pack_fseek(f,n)==0);
    if (success)
    {
      readsize+=n;
    }
    return success;
  }
}

INLINE bool p_getc(void *p,PACKFILE *f,bool keepdata)
{
  unsigned char *cp = (unsigned char *)p;
  int c;
  if (!f) return false;
#ifdef NEWALLEGRO
  if (f->normal.flags&PACKFILE_FLAG_WRITE) return false;    //must not be writing to file
#else
  if (f->flags&PACKFILE_FLAG_WRITE) return false;           //must not be writing to file
#endif

  if (pack_feof(f))
  {
    return false;
  }
  c = pack_getc(f);
  if (pack_ferror(f))
  {
    return false;
  }
  if (keepdata==true)
  {
    *cp = c;
  }
  readsize+=1;
  return true;
}

INLINE bool p_putc(int c,PACKFILE *f)
{
  if (!f) return false;
#ifdef NEWALLEGRO
  if (!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false; //must be writing to file
#else
  if (!(f->flags&PACKFILE_FLAG_WRITE)) return false;        //must be writing to file
#endif

  pack_putc(c,f);
  bool success=(pack_ferror(f)==0);
  if (success)
  {
    writesize+=1;
  }
  return success;
}

INLINE bool p_igetw(void *p,PACKFILE *f,bool keepdata)
{
  short *cp = (short *)p;
  int c;
  if (!f) return false;
#ifdef NEWALLEGRO
  if (f->normal.flags&PACKFILE_FLAG_WRITE) return false;    //must not be writing to file
#else
  if (f->flags&PACKFILE_FLAG_WRITE) return false;           //must not be writing to file
#endif

  if (pack_feof(f))
  {
    return false;
  }
  c = pack_igetw(f);
  if (pack_ferror(f))
  {
    return false;
  }
  if (keepdata==true)
  {
    *cp = c;
  }
  readsize+=2;
  return true;
}

INLINE bool p_iputw(int c,PACKFILE *f)
{
  if (!f) return false;
#ifdef NEWALLEGRO
  if (!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false; //must be writing to file
#else
  if (!(f->flags&PACKFILE_FLAG_WRITE)) return false;        //must be writing to file
#endif

  pack_iputw(c,f);
  bool success=(pack_ferror(f)==0);
  if (success)
  {
    writesize+=2;
  }
  return success;
}

INLINE bool p_igetl(void *p,PACKFILE *f,bool keepdata)
{
  dword *cp = (dword *)p;
  long c;
  if (!f) return false;
#ifdef NEWALLEGRO
  if (f->normal.flags&PACKFILE_FLAG_WRITE) return false;    //must not be writing to file
#else
  if (f->flags&PACKFILE_FLAG_WRITE) return false;           //must not be writing to file
#endif

  if (pack_feof(f))
  {
    return false;
  }
  c = pack_igetl(f);
  if (pack_ferror(f))
  {
    return false;
  }
  if (keepdata==true)
  {
    *cp = c;
  }
  readsize+=4;
  return true;
}

INLINE bool p_igetd(void *p, PACKFILE *f, bool keepdata)
{
	long temp;
	bool result = p_igetl(&temp,f,keepdata);
	*(int *)p=(int)temp;
	return result;
}

INLINE bool p_iputl(long c,PACKFILE *f)
{
  if (!f) return false;
#ifdef NEWALLEGRO
  if (!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false; //must be writing to file
#else
  if (!(f->flags&PACKFILE_FLAG_WRITE)) return false;        //must be writing to file
#endif

  pack_iputl(c,f);
  bool success=(pack_ferror(f)==0);
  if (success)
  {
    writesize+=4;
  }
  return success;
}

INLINE bool p_mgetw(void *p,PACKFILE *f,bool keepdata)
{
  short *cp = (short *)p;
  int c;
  if (!f) return false;
#ifdef NEWALLEGRO
  if (f->normal.flags&PACKFILE_FLAG_WRITE) return false;    //must not be writing to file
#else
  if (f->flags&PACKFILE_FLAG_WRITE) return false;           //must not be writing to file
#endif

  if (pack_feof(f))
  {
    return false;
  }
  c = pack_mgetw(f);
  if (pack_ferror(f))
  {
    return false;
  }
  if (keepdata==true)
  {
    *cp = c;
  }
  readsize+=2;
  return true;
}

INLINE bool p_mputw(int c,PACKFILE *f)
{
  if (!f) return false;
#ifdef NEWALLEGRO
  if (!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false; //must be writing to file
#else
  if (!(f->flags&PACKFILE_FLAG_WRITE)) return false;        //must be writing to file
#endif

  pack_mputw(c,f);
  bool success=(pack_ferror(f)==0);
  if (success)
  {
    writesize+=2;
  }
  return success;
}

INLINE bool p_mgetl(void *p,PACKFILE *f,bool keepdata)
{
  dword *cp = (dword *)p;
  long c;
  if (!f) return false;
#ifdef NEWALLEGRO
  if (f->normal.flags&PACKFILE_FLAG_WRITE) return false;    //must not be writing to file
#else
  if (f->flags&PACKFILE_FLAG_WRITE) return false;           //must not be writing to file
#endif

  if (pack_feof(f))
  {
    return false;
  }
  c = pack_mgetl(f);
  if (pack_ferror(f))
  {
    return false;
  }
  if (keepdata==true)
  {
    *cp = c;
  }
  readsize+=4;
  return true;
}

INLINE bool p_mputl(long c,PACKFILE *f)
{
  if (!f) return false;
#ifdef NEWALLEGRO
  if (!(f->normal.flags&PACKFILE_FLAG_WRITE)) return false; //must be writing to file
#else
  if (!(f->flags&PACKFILE_FLAG_WRITE)) return false;        //must be writing to file
#endif

  pack_mputl(c,f);
  bool success=(pack_ferror(f)==0);
  if (success)
  {
    writesize+=4;
  }
  return success;
}

INLINE bool isinRect(int x,int y,int rx1,int ry1,int rx2,int ry2)
{
  return x>=rx1 && x<=rx2 && y>=ry1 && y<=ry2;
}

INLINE void SCRFIX() { putpixel(screen,0,0,getpixel(screen,0,0)); }
#endif                                                      //_ZDEFS_H_

#define NUMSCRIPTFFC 512
#define NUMSCRIPTITEM 256
#define NUMSCRIPTGUYS 256
#define NUMSCRIPTWEAPONS 256
#define NUMSCRIPTGLOBAL 3
#define NUMSCRIPTLINK 3
#define NUMSCRIPTSCREEN 256

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

//Conditional Debugging Compilation
//Script related
#define _FFDEBUG
//#define _SCRIPT_COUNTER
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

#include <math.h>
#include <string.h>
#include <vector>
#include <set>
#include <assert.h>
#include "zc_alleg.h"
#include "gamedata.h"
#include "zc_array.h"

#define ZELDA_VERSION       0x0250  //version of the program
#define ZC_VERSION 25300 //Version ID for ZScript Game->Version
#define ABOUT_VERSION 0x0253
#define VERSION_BUILD       33                             //build number of this version
#define ZELDA_VERSION_STR   "Omnius, 2.53 Release 2"               //version of the program as presented in text
#define IS_BETA             0                        //is this a beta? (1: beta, -1: alpha)
#define DATE_STR            "17th April, 2019"
#define ZELDA_ABOUT_STR 	    "Zelda Classic 'Omnius' Release 2"
#define COPYRIGHT_YEAR      "2019"     
                     //shown on title screen and in ending

#define MIN_VERSION         0x0184

#define ZELDADAT_VERSION      0x0211                        //version of zelda.dat
#define ZELDADAT_BUILD        17                            //build of zelda.dat
#define SFXDAT_VERSION        0x0211                        //version of sfx.dat
#define SFXDAT_BUILD          15                            //build of sfx.dat
#define FONTSDAT_VERSION      0x0211                        //version of fonts.dat
#define FONTSDAT_BUILD        18                            //build of fonts.dat
#define QSTDAT_VERSION        0x0253                        //version of qst.dat
#define QSTDAT_BUILD          30                            //build of qst.dat
#define ZQUESTDAT_VERSION     0x0211                        //version of zquest.dat
#define ZQUESTDAT_BUILD       18                            //build of zquest.dat

enum {ENC_METHOD_192B104=0, ENC_METHOD_192B105, ENC_METHOD_192B185, ENC_METHOD_211B9, ENC_METHOD_211B18, ENC_METHOD_MAX};

#ifdef ALLEGRO_DOS
//already defined in DOS
/*
#elif defined(ALLEGRO_WINDOWS)
#define PI 3.14159265358979323846
#elif defined(ALLEGRO_MACOSX)
#define PI 3.14159265358979323846
#elif defined(ALLEGRO_LINUX)
*/
#else
#define PI 3.14159265358979323846
#endif

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
#define ID_SUBSCREEN      ZC_ID('S','U','B','S')              //subscreen data
#define ID_ITEMDROPSETS   ZC_ID('D','R','O','P')              //item drop set tables
#define ID_FAVORITES      ZC_ID('F','A','V','S')              //favorite combos and combo aliases
#define ID_FFSCRIPT       ZC_ID('F','F','S','C')              //ff scripts data
#define ID_SFX            ZC_ID('S','F','X',' ')              //sfx data

//Version number of the different section types
#define V_HEADER           3
#define V_RULES           13
#define V_STRINGS          5
#define V_MISC             7
#define V_TILES            1
#define V_COMBOS           7
#define V_CSETS            4
#define V_MAPS            18
#define V_DMAPS            9
#define V_DOORS            1
#define V_ITEMS           25
#define V_WEAPONS          6
#define V_COLORS           2
#define V_ICONS            1
#define V_GRAPHICSPACK     1
#define V_INITDATA        18
#define V_GUYS            24
#define V_MIDIS            4
#define V_CHEATS           1
#define V_SAVEGAME        11
#define V_COMBOALIASES     2
#define V_LINKSPRITES      5
#define V_SUBSCREEN        6
#define V_ITEMDROPSETS     2
#define V_FFSCRIPT         6
#define V_SFX              6
#define V_FAVORITES        1

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
#define CV_GUYS            3
#define CV_MIDIS           3
#define CV_CHEATS          1
#define CV_SAVEGAME        5
#define CV_COMBOALIASES    1
#define CV_LINKSPRITES     1
#define CV_SUBSCREEN       3
#define CV_ITEMDROPSETS    1
#define CV_FFSCRIPT        1
#define CV_SFX             5
#define CV_FAVORITES       1

extern int curr_tb_page;
extern bool triplebuffer_not_available;
extern int original_playing_field_offset;
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
extern bool fake_pack_writing;

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
#define WAV_COUNT       256

#define MAXSCREENS 128
#define MAXCUSTOMMIDIS192b177 32                                  // uses bit string for midi flags, so 32 bytes
#define MAXCUSTOMMIDIS        252                                 // uses bit string for midi flags, so 32 bytes
#define MIDIFLAGS_SIZE  ((MAXCUSTOMMIDIS+7)>>3)
#define MAXCUSTOMTUNES        252

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
#define MAXSUBSCREENITEMS	256
#define MAXCUSTOMSUBSCREENS 128
#define MAXFFCS			 32

#define MAXFAVORITECOMMANDS 8
#define MAXFAVORITECOMBOS 100
#define MAXFAVORITECOMBOALIASES 100

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
#define fROAR           8 // DEFUNCT
#define fWHISTLE        16
#define fLADDER         32
#define fMAZE           64
#define fSEA            128 // DEFUNCT

// flags2
#define wfUP            1
#define wfDOWN          2
#define wfLEFT          4
#define wfRIGHT         8
#define fSECRET         16  // play "secret" sfx upon entering this screen
#define fAIRCOMBOS      32
#define fFLOATTRAPS     64
#define fCLEARSECRET    128 // clear all enemies to trigger secret entrance (Enemies->Secret)

// flags3
#define fHOLDITEM         1
#define fCYCLEONINIT      2
#define fINVISROOM        4
#define fINVISLINK        8
#define fNOSUBSCR         16
#define fIWARPFULLSCREEN  32                                // instawarps affect all sprites
#define fNOSUBSCROFFSET   64                                // don't offset the screen when fNOSUBSCR is true
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
#define fLAYER3BG       1
#define fLAYER2BG       2
#define fITEMFALLS		  4
#define fSIDEVIEW       8
#define fNOLINKMARK     16
#define fSPECITEMMARK   32
#define fWHISTLEPAL     64
#define fWHISTLEWATER   128

//flags8
// 1 - 16 are General Purpose (Scripts)
#define fMAZEvSIDEWARP 32

//lens layer effects
#define llNORMAL        0
#define llLENSHIDES     8
#define llLENSSHOWS     16

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
#define ipONETIME       4                                   // Getting this item sets mITEM
#define ipONETIME2      2048                                // Getting this item sets mBELOW
#define ipDUMMY         8                                   // Dummy item.  Can't get this.
#define ipCHECK         16                                  // Check restrictions (money in a shop, etc.)
#define ipMONEY         32                                  // This item causes money loss (bomb upgrade, swindle room, etc.)
#define ipFADE          64                                  // Blinks if fadeclk>0
#define ipENEMY         128                                 // Enemy is carrying it around
#define ipTIMER         256                                 // Disappears after a while
#define ipBIGTRI        512                                 // Large collision rectangle (used for large triforce)
#define ipNODRAW        1024                                // Don't draw this (for underwater items)


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
{
    mfNONE, mfPUSHUD, mfPUSH4, mfWHISTLE, mfBCANDLE, mfARROW, mfBOMB, mfFAIRY,
    mfRAFT, mfARMOS_SECRET, mfARMOS_ITEM, mfSBOMB, mfRAFT_BRANCH, mfDIVE_ITEM,
    mfLENSMARKER, mfZELDA,
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
    mfNOENEMY, mfNOGROUNDENEMY, mfSCRIPT1, mfSCRIPT2, mfSCRIPT3, mfSCRIPT4, mfSCRIPT5,
    mfRAFT_BOUNCE, mfMAX, mfPUSHED
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
    cBLOCKFIREBALL, cDAMAGE5, cDAMAGE6, cDAMAGE7, cCHANGE/**DEPRECATED**/, cSPINTILE1, cSPINTILE2,
    cSCREENFREEZE, cSCREENFREEZEFF, cNOGROUNDENEMY, cSLASHNEXT, cSLASHNEXTITEM, cBUSHNEXT,
    cSLASHTOUCHY, cSLASHITEMTOUCHY, cBUSHTOUCHY, cFLOWERSTOUCHY, cTALLGRASSTOUCHY,
    cSLASHNEXTTOUCHY, cSLASHNEXTITEMTOUCHY, cBUSHNEXTTOUCHY, cEYEBALL_4, cTALLGRASSNEXT,
    cSCRIPT1, cSCRIPT2, cSCRIPT3, cSCRIPT4, cSCRIPT5, cMAX
};

#define QUESTRULES_SIZE 20
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
    qr_CONTFULL_DEP/*DEPRECATED*/, qr_RLFIX, qr_LENSHINTS, qr_LINKDUNGEONPOSFIX,
    // 4
    qr_HOLDITEMANIMATION, qr_HESITANTPUSHBLOCKS, qr_HIDECARRIEDITEMS, qr_FFCSCROLL,
    qr_RAFTLENS, qr_SMOOTHVERTICALSCROLLING, qr_WHIRLWINDMIRROR, qr_NOFLASHDEATH,
    // 5
    qr_HOLDNOSTOPMUSIC, qr_FIREPROOFLINK, qr_OUCHBOMBS, qr_NOCLOCKS_DEP/*DEPRECATED*/,
    qr_TEMPCLOCKS_DEP/*DEPRECATED*/, qr_BRKBLSHLDS_DEP/*DEPRECATED*/, qr_BRKNSHLDTILES, qr_MEANPLACEDTRAPS,
    // 6
    qr_PHANTOMPLACEDTRAPS, qr_ALLOWFASTMSG, qr_LINKEDCOMBOS/*DEPRECATED*/, qr_NOGUYFIRES,
    qr_HEARTRINGFIX, qr_NOHEARTRING, qr_OVERWORLDTUNIC, qr_SWORDWANDFLIPFIX,
    // 7
    qr_ENABLEMAGIC, qr_MAGICWAND_DEP/*DEPRECATED*/, qr_MAGICCANDLE_DEP/*DEPRECATED*/, qr_MAGICBOOTS_DEP/*DEPRECATED*/,
    qr_NONBUBBLEMEDICINE, qr_NONBUBBLEFAIRIES, qr_NONBUBBLETRIFORCE, qr_NEWENEMYTILES,
    // 8
    qr_NOROPE2FLASH_DEP/*DEPRECATED*/, qr_NOBUBBLEFLASH_DEP/*DEPRECATED*/, qr_GHINI2BLINK_DEP, qr_BITMAPOFFSETFIX, //qr_WPNANIMFIX /* UNIMPLEMENTED */,
    qr_PHANTOMGHINI2_DEP/*DEPRECATED*/, qr_Z3BRANG_HSHOT, qr_NOITEMMELEE, qr_SHADOWS,
    // 9
    qr_TRANSSHADOWS, qr_QUICKSWORD, qr_BOMBHOLDFIX, qr_EXPANDEDLTM,
    qr_NOPOTIONCOMBINE_DEP/*DEPRECATED*/, qr_LINKFLICKER, qr_SHADOWSFLICKER, qr_WALLFLIERS,
    // 10
    qr_NOBOMBPALFLASH, qr_HEARTSREQUIREDFIX, qr_PUSHBLOCKCSETFIX, qr_TRANSLUCENTNAYRUSLOVEROCKET_DEP/*DEPRECATED*/,
    qr_FLICKERINGNAYRUSLOVEROCKET_DEP/*DEPRECATED*/, qr_CMBCYCLELAYERS, qr_DMGCOMBOPRI, qr_WARPSIGNOREARRIVALPOINT,
    // 11
    qr_LTTPCOLLISION, qr_LTTPWALK, qr_SLOWENEMYANIM_DEP/*DEPRECATED*/, qr_TRUEARROWS,
    qr_NOSAVE, qr_NOCONTINUE, qr_QUARTERHEART, qr_NOARRIVALPOINT,
    // 12
    qr_NOGUYPOOF, qr_ALLOWMSGBYPASS, qr_NODIVING, qr_LAYER12UNDERCAVE,
    qr_NOSCROLLCONTINUE, qr_SMARTSCREENSCROLL, qr_RINGAFFECTDAMAGE, qr_ALLOW10RUPEEDROPS_DEP/*DEPRECATED*/,
    // 13
    qr_TRAPPOSFIX, qr_TEMPCANDLELIGHT, qr_REDPOTIONONCE_DEP/*DEPRECATED*/, qr_OLDSTYLEWARP,
    qr_NOBORDER, qr_OLDTRIBBLES_DEP/*DEPRECATED*/, qr_REFLECTROCKS /*DEPRECATED*/, qr_OLDPICKUP,
    // 14
    qr_ENEMIESZAXIS, qr_SAFEENEMYFADE, qr_MORESOUNDS, qr_BRANGPICKUP,
    qr_HEARTPIECEINCR_DEP/*DEPRECATED*/, qr_ITEMSONEDGES, qr_EATSMALLSHIELD_DEP/*DEPRECATED*/, qr_MSGFREEZE,
    // 15
    qr_SLASHFLIPFIX, qr_FIREMAGICSPRITE_DEP/*DEPRECATED*/, qr_SLOWCHARGINGWALK, qr_NOWANDMELEE,
    qr_SLOWBOMBFUSES_DEP/*DEPRECATED*/, qr_SWORDMIRROR, qr_SELECTAWPN, qr_LENSSEESENEMIES,
    // 16
    qr_INSTABURNFLAGS, qr_DROWN, qr_MSGDISAPPEAR, qr_SUBSCREENOVERSPRITES,
    qr_BOMBDARKNUTFIX, qr_LONGBOMBBOOM_DEP/*DEPRECATED*/, qr_OFFSETEWPNCOLLISIONFIX, qr_DMGCOMBOLAYERFIX,
    // 17
    qr_ITEMSINPASSAGEWAYS, qr_LOG, qr_FIREPROOFLINK2, qr_NOITEMOFFSET,
    qr_ITEMBUBBLE, qr_GOTOLESSNOTEQUAL /* Compatibility */, qr_LADDERANYWHERE, qr_HOOKSHOTLAYERFIX,
    // 18
    qr_REPLACEOPENDOORS /* Compatibility */, qr_OLDLENSORDER /* Compatibility */, qr_NOFAIRYGUYFIRES /* Compatibility */, qr_SCRIPTERRLOG,
    qr_TRIGGERSREPEAT /* Compatibility */, qr_ENEMIESFLICKER, qr_OVALWIPE, qr_TRIANGLEWIPE,
    // 19
    qr_SMASWIPE, qr_NOSOLIDDAMAGECOMBOS /* Compatibility */, qr_SHOPCHEAT, qr_HOOKSHOTDOWNBUG /* Compatibility */,
    qr_OLDHOOKSHOTGRAB /* Compatibility */, qr_PEAHATCLOCKVULN /* Compatibility */, qr_VERYFASTSCROLLING, qr_OFFSCREENWEAPONS /* Compatibility */,
    // 20
    qr_BROKENSTATUES /* Compatibility */, qr_BOMBCHUSUPERBOMB,
    //qr_NOSCRIPTSDURINGSCROLL /* Not Implemented */, 
    qr_ITEMPICKUPSETSBELOW /* Compatibility */,
    qr_NOGANONINTRO, //bugfix, Ganon DMap Intro
    qr_MELEEMAGICCOST,
    qr_OLDMIRRORCOMBOS,
    qr_BROKENBOOKCOST,
    qr_OLDSIDEVIEWSPIKES, //last bit possible
    qr_MAX
};

// Unsaved compatibility rules
// Unsaved compatibility rules
enum extraRules
{
    er_BITMAPOFFSET, er_SHORTDGNWALK,
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
    rMAX
};

// directions
enum direction { up, down, left, right, l_up, r_up, l_down, r_down };
const direction oppositeDir[]= {down, up, right, left, r_down, l_down, r_up, l_up};
// refill stuff
enum { REFILL_NONE, REFILL_FAIRYDONE, REFILL_LIFE, REFILL_MAGIC, REFILL_ALL};
#define REFILL_FAIRY -1

//Z-axis related
#define FEATHERJUMP 80

//other
#define MAXDRUNKCLOCK          500

enum
{
    dBUSHLEAVES, dFLOWERCLIPPINGS, dGRASSCLIPPINGS, dHAMMERSMACK,
    dTALLGRASS, dRIPPLES, dNAYRUSLOVESHIELD, dHOVER, dMAXDECORATIONS
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
    iLMagic, iGRing, iKillAll, iL2Amulet, iDinsFire,
    iFaroresWind, iNayrusLove, iBossKey, iBow2, iFairyStill,
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

// item sets
enum
{
    isNONE, isDEFAULT, isBOMBS, isMONEY, isLIFE, isBOMB100, isSBOMB100,
    isMAGIC, isMAGICBOMBS, isMAGICMONEY, isMAGICLIFE, isMAGIC2, isTALLGRASS, isMAX
};

//emulation patches
enum 
{ 
	emuITEMPERSEG, emuGRIDCOLLISION, emuOLDTRIBBLES, emu190LINKSPRITES, emuCOPYSWIMSPRITES, emu210WINDROBES,
	emu250DMAPINTOREPEAT, emuFIXTRIFORCECELLAR, emuNOFLIPFIRETRAIL, emuSWORDTRIGARECONTINUOUS, emu8WAYSHOTSFX, emu210BOMBCHU, emuLAST
		
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
    ewFLAME, ewWIND, iwMMeter, wDINSFIRE1A, wDINSFIRE1B,
// 40
    wDINSFIRES1A, wDINSFIRES1B, wHSCHAIN_V, iwMore, iwBossMarker,
    iwLinkSlash, wSWORDSLASH, wWSWORDSLASH, wMSWORDSLASH, wXSWORDSLASH,
// 50
    iwShadow, iwLargeShadow, iwBushLeaves, iwFlowerClippings, iwGrassClippings,
    iwTallGrass, iwRipples, iwNPCs, wNAYRUSLOVE1A, wNAYRUSLOVE1B,
// 60
    wNAYRUSLOVES1A, wNAYRUSLOVES1B, wNAYRUSLOVE2A, wNAYRUSLOVE2B, wNAYRUSLOVES2A,
    wNAYRUSLOVES2B, iwNayrusLoveShieldFront, iwNayrusLoveShieldBack, iwSubscreenVine, wCBYRNA,
// 70
    wCBYRNASLASH, wLSHEAD, wLSCHAIN_H, wLSHANDLE, wLSCHAIN_V,
    wSBOOM, ewBOMB, ewSBOMB, ewBOOM, ewSBOOM,
// 80
    ewFIRETRAIL, ewFLAME2, ewFLAME2TRAIL, ewICE, iwHover,
    wFIREMAGIC, iwQuarterHearts, wCBYRNAORB, //iwSideLadder, iwSideRaft,
    
    wLast, wMAX=256
};

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
    wRefBeam, wStomp,
    lwMax,
    // Dummy weapons - must be between lwMax and wEnemyWeapons!
    wScript1, wScript2, wScript3, wScript4,
    wScript5, wScript6, wScript7, wScript8,
    wScript9, wScript10,
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
    eeGHINI, eeARMOS/*DEPRECATED*/, eeKEESE, eeGEL/*DEPRECATED*/, eeZOL/*DEPRECATED*/, eeROPE/*DEPRECATED*/, eeGORIYA/*DEPRECATED*/, eeTRAP,
    eeWALLM, eeBUBBLE/*DEPRECATED*/, eeVIRE/*DEPRECATED*/, eeLIKE/*DEPRECATED*/, eePOLSV/*DEPRECATED*/, eeWIZZ, eeAQUA, eeMOLD,
    eeDONGO, eeMANHAN, eeGLEEOK, eeDIG, eeGHOMA, eeLANM, eePATRA, eeGANON,
    eePROJECTILE, eeGELTRIB/*DEPRECATED*/, eeZOLTRIB/*DEPRECATED*/, eeVIRETRIB/*DEPRECATED*/, eeKEESETRIB/*DEPRECATED*/, eeSPINTILE, eeNONE,
    eeFAIRY, eeFIRE, eeOTHER,
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
    aDONGOBS, a4FRMPOS8DIRF, a4FRMPOS4DIRF, a4FRMNODIR, aGANON, a2FRMB, aMAX
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
enum
{
    edefBRANG, edefBOMB, edefSBOMB, edefARROW, edefFIRE, edefWAND,
    edefMAGIC, edefHOOKSHOT, edefHAMMER, edefSWORD, edefBEAM, edefREFBEAM, edefREFMAGIC,
    edefREFBALL, edefREFROCK, edefSTOMP, edefBYRNA, edefSCRIPT /* used to be unused edefSPIN */,
    // Reserved for future use.
    edefQUAKE,
    edefLAST
};


enum
{
    edNORMAL, // Take damage (or stun)
    edHALFDAMAGE, // Take half damage
    edQUARTDAMAGE, // Take 0.25 damage
    edSTUNONLY, // Stun instead of taking damage.
    edSTUNORCHINK, // If damage > 0, stun instead. Else, bounce off.
    edSTUNORIGNORE, // If damage > 0, stun instead. Else, ignore.
    edCHINKL1, // Bounces off, plays SFX_CHINK
    edCHINKL2, // Bounce off unless damage >= 2
    edCHINKL4, // Bounce off unless damage >= 4
    edCHINKL6, // Bounce off unless damage >= 6
    edCHINKL8, // Bounce off unless damage >= 8
    edCHINK, // Bounces off, plays SFX_CHINK
    edIGNOREL1, // Ignore unless damage > 1.
    edIGNORE, // Do Nothing
    ed1HKO, // One-hit knock-out
    edLAST
};
#define edX edIGNORE // Shorthand
// Remaining 16 reserved for future use.

// enemy patters
enum { pRANDOM, pSIDES, pSIDESR, pCEILING, pCEILINGR, pRANDOMR };

enum { tfInvalid=0, tf4Bit, tf8Bit, tf16Bit, tf24Bit, tf32Bit, tfMax };

struct size_and_pos
{
    int x;
    int y;
    int w;
    int h;
};

//#define OLDITEMCNT i90
//#define OLDWPNCNT  w84
#define ITEMCNT   iMax
#define WPNCNT    wMAX

struct tiledata
{
    byte format;
    byte *data;
};

struct itemdata
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
    byte power;	// Damage, height, etc.
    word flags;
#define ITEM_GAMEDATA    0x0001  // Whether this item sets the corresponding gamedata value or not
#define ITEM_EDIBLE      0x0002  // can be eaten by Like Like
#define ITEM_COMBINE     0x0004  // blue potion + blue potion = red potion
#define ITEM_DOWNGRADE   0x0008
#define ITEM_FLAG1   0x0010
#define ITEM_FLAG2   0x0020
#define ITEM_KEEPOLD     0x0040
#define ITEM_RUPEE_MAGIC 0x0080
#define ITEM_UNUSED       0x0100
#define ITEM_GAINOLD     0x0200
#define ITEM_FLAG3     0x0400
#define ITEM_FLAG4     0x0800
#define ITEM_FLAG5     0x1000
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
    byte wpn;
    byte wpn2;
    byte wpn3;
    byte wpn4;
    byte wpn5;
    byte wpn6;
    byte wpn7;
    byte wpn8;
    byte wpn9;
    byte wpn10;
    byte pickup_hearts;
    long misc1;
    long misc2;
    long misc3;
    long misc4;
    long misc5;
    long misc6;
    long misc7;
    long misc8;
    long misc9;
    long misc10;
    byte magic; // Magic usage!
    byte usesound;
};

struct wpndata
{
    word tile;
    byte misc;                                                // 000bvhff (vh:flipping, f:flash (1:NES, 2:BSZ))
    byte csets;                                               // ffffcccc (f:flash cset, c:cset)
    byte frames;                                              // animation frame count
    byte speed;                                               // animation speed
    byte type;                                                // used by certain weapons
//  byte wpn_type;
    word script;
//  byte exp;                                                 // not used
};

#define		WF_BEHIND			0x10	//Weapon renders behind other sprites

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
#define ffIGNOREHOLDUP  0x00000400 //Updated even while Link is holding an item

//FF combo changer flags

#define ffSWAPNEXT      0x80000000 //Swap speed with next FFC
#define ffSWAPPREV      0x40000000 //Swap speed with prev. FFC
#define ffCHANGENEXT    0x20000000 //Increase combo ID
#define ffCHANGEPREV    0x10000000 //Decrease combo ID
#define ffCHANGETHIS    0x08000000 //Change combo/cset to this
#define ffCHANGESPEED   0x04000000 //Change speed to this (default, not implemented yet)

struct guydata
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
    
    short  rate, hrate, step, homing, grumble, item_set;
    long   misc1, misc2, misc3, misc4, misc5, misc6, misc7, misc8, misc9, misc10, misc11, misc12, misc13, misc14, misc15;
    short  bgsfx, bosspal, extend;
    byte defense[edefLAST];
    //  short  startx, starty;
    //  short  foo1,foo2,foo3,foo4,foo5,foo6;
    byte  hitsfx, deadsfx;
};

class refInfo
{
public:
    //word script; //script number
    dword pc; //current command offset
    
    long d[8]; //d registers
    long a[2]; //a regsisters (reference to another ffc on screen)
    byte sp; //stack pointer for current script
    dword scriptflag; //stores whether various operations were true/false etc.
    
    byte ffcref, idata; //current object pointers
    dword itemref, guyref, lwpn, ewpn;
    
    //byte ewpnclass, lwpnclass, guyclass; //Not implemented
    
    void Clear()
    {
        pc = 0, sp = 0, scriptflag = 0;
        ffcref = 0, idata = 0, itemref = 0, guyref = 0, lwpn = 0, ewpn = 0;
        memset(d, 0, 8 * sizeof(long));
        a[0] = a[1] = 0;
    }
    
    refInfo()
    {
        Clear();
    }
    
    refInfo(const refInfo &copy)
    {
        *this = copy;
    }
    
    refInfo &operator = (const refInfo &rhs)
    {
        pc = rhs.pc, sp = rhs.sp, scriptflag = rhs.scriptflag;
        ffcref = rhs.ffcref, idata = rhs.idata;
        itemref = rhs.itemref, guyref = rhs.guyref, lwpn = rhs.lwpn, ewpn = rhs.ewpn;
        memcpy(d, rhs.d, 8 * sizeof(long));
        memcpy(a, rhs.a, 2 * sizeof(long));
        return *this;
    }
};


struct mapscr
{
    byte valid;
    byte guy;
    word str;
    byte room;
    byte item;
    byte hasitem;
    byte tilewarptype[4];
    byte tilewarpoverlayflags;
    word door_combo_set;
    byte warpreturnx[4];
    byte warpreturny[4];
    word warpreturnc;
    byte stairx;
    byte stairy;
    byte itemx;
    byte itemy;
    word color;
    byte enemyflags;
    byte door[4];
    word tilewarpdmap[4];
    byte tilewarpscr[4];
    byte exitdir;
    word enemy[10];
    byte pattern;
    byte sidewarptype[4];
    byte sidewarpoverlayflags;
    byte warparrivalx;
    byte warparrivaly;
    byte path[4];
    byte sidewarpscr[4];
    word sidewarpdmap[4];
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
    // you're listening to ptr radio, the sounds of insane. ;)
    std::vector<word> data;
    std::vector<byte> sflag;
    std::vector<byte> cset;
    word viewX;
    word viewY;
    byte scrWidth;
    byte scrHeight;
    
    //Why doesn't ffc get to be its own class?
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
    
    //ffc script attachments
    word ffscript[32];
    long initd[32][8];
    long inita[32][2];
    bool initialized[32];
    
    /*long d[32][8];
    long a[32][2];
    word pc[32];
    dword scriptflag[32];
    byte sp[32]; //stack pointer
    byte ffcref[32];
    dword itemref[32];
    byte itemclass[32];
    dword lwpnref[32];
    dword ewpnref[32];
    dword guyref[32];*/
    //byte lwpnclass[32]; Not implemented
    //byte ewpnclass[32]; Not implemented
    //byte guyclass[32]; Not implemented
    
    /*long map_stack[256];
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
    byte map_ffcref;*/ //All this is trash because we don't have map scripts, waste of memory
    word script_entry;
    word script_occupancy;
    word script_exit;
    
    byte oceansfx;
    byte bosssfx;
    byte secretsfx;
    byte holdupsfx;
    
    // for importing older quests...
    byte old_cpage;
    short screen_midi;
    byte lens_layer;
    
    
    void zero_memory()
    {
        //oh joy, this will be fun...
        valid=0;
        guy=0;
        str=0;
        room=0;
        item=0;
        hasitem=0;
        tilewarpoverlayflags=0;
        door_combo_set=0;
        warpreturnc=0;
        stairx=0;
        stairy=0;
        itemx=0;
        itemy=0;
        color=0;
        enemyflags=0;
        
        exitdir=0;
        pattern=0;
        sidewarpoverlayflags=0;
        warparrivalx=0;
        warparrivaly=0;
        
        sidewarpindex=0;
        undercombo=0;
        undercset=0;
        catchall=0;
        flags=0;
        flags2=0;
        flags3=0;
        flags4=0;
        flags5=0;
        flags6=0;
        flags7=0;
        flags8=0;
        flags9=0;
        flags10=0;
        csensitive=0;
        noreset=0;
        nocarry=0;
        timedwarptics=0;
        nextmap=0;
        nextscr=0;
        
        viewX=0;
        viewY=0;
        scrWidth=0;
        scrHeight=0;
        numff=0;
        
        for(int i(0); i<4; i++)
        {
            door[i]=0;
            tilewarpdmap[i]=0;
            tilewarpscr[i]=0;
            tilewarptype[i]=0;
            warpreturnx[i]=0;
            warpreturny[i]=0;
            path[i]=0;
            sidewarpscr[i]=0;
            sidewarpdmap[i]=0;
            sidewarptype[i]=0;
        }
        
        for(int i(0); i<10; i++)
            enemy[i]=0;
            
        for(int i(0); i<128; i++)
        {
            secretcombo[i]=0;
            secretcset[i]=0;
            secretflag[i]=0;
        }
        
        for(int i(0); i<6; i++)
        {
            layermap[i]=0;
            layerscreen[i]=0;
            layeropacity[i]=0;
        }
        
        for(int i(0); i<32; i++)
        {
            for(int j(0); j<8; j++)
            {
                //d[i][j]=0;
                initd[i][j]=0;
            }
            
            for(int j(0); j<2; j++)
            {
                inita[i][j]=0;
                //a[i][j]=0;
            }
            
            initialized[i]=0;
            /*pc[i]=0;
            scriptflag[i]=0;
            sp[i]=0;
            ffcref[i]=0;
            itemref[i]=0;
            itemclass[i]=0;*/
            ffdata[i]=0;
            ffcset[i]=0;
            ffdelay[i]=0;
            ffx[i]=0;
            ffy[i]=0;
            ffxdelta[i]=0;
            ffydelta[i]=0;
            ffxdelta2[i]=0;
            ffydelta2[i]=0;
            ffflags[i]=0;
            ffwidth[i]=0;
            ffheight[i]=0;
            fflink[i]=0;
            ffscript[i]=0;
        }
        
        /*	  for(int i(0);i<256;i++)
        	  {
        	   map_stack[i]=0;
        	  }
        	   for(int i(0);i<8;i++)
        	  {
        	   map_d[i]=0;
        	  }
           map_pc=0;
           map_scriptflag=0;
           map_sp=map_itemref=map_itemclass=0;
           map_lwpnref=0;
           map_lwpnclass=0;
           map_ewpnref=0;
           map_ewpnclass=0;
           map_guyref=0;
           map_guyclass=0;
           map_ffcref=0;*/
        script_entry=0;
        script_occupancy=0;
        script_exit=0;
        oceansfx=0;
        bosssfx=0;
        secretsfx=0;
        holdupsfx=0;
        lens_layer=0;
        
        data.assign(176,0);
        sflag.assign(176,0);
        cset.assign(176,0);
        //data.assign(data.size(),0);
        //sflag.assign(sflag.size(),0);
        //cset.assign(cset.size(),0);
    }
    
    
    mapscr()
    {
        data.resize(176,0);
        sflag.resize(176,0);
        cset.resize(176,0);
        zero_memory();
    }
    
};

struct ffscript
{
    word command;
    long arg1;
    long arg2;
    char *ptr;
};


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

struct comboclass
{
    char  name[64];                       //  name
    byte  block_enemies;                  //  a (bit flags, 1=ground, 2=jump, 4=fly)
    byte  block_hole;                     //  b
    byte  block_trigger;                  //  c
    byte  block_weapon[32];               //  d (ID of lweapons/eweapons to block)
    short conveyor_x_speed;               //  e
    short conveyor_y_speed;               //  f
    word  create_enemy;                   //  g
    byte  create_enemy_when;              //  h
    long  create_enemy_change;            //  i
    byte  directional_change_type;        //  j
    long  distance_change_tiles;          //  k
    short dive_item;                      //  l
    byte  dock;                           //  m
    byte  fairy;                          //  n
    byte  ff_combo_attr_change;           //  o
    long  foot_decorations_tile;          //  p
    byte  foot_decorations_type;          //  q
    byte  hookshot_grab_point;            //  r
    byte  ladder_pass;                    //  s
    byte  lock_block_type;                //  t
    long  lock_block_change;              //  u
    byte  magic_mirror_type;              //  v
    short modify_hp_amount;               //  w
    byte  modify_hp_delay;                //  x
    byte  modify_hp_type;                 //  y
    short modify_mp_amount;               //  z
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
    long  step_change_to;                 // as
    byte  strike_weapons[32];             // at
    long  strike_remnants;                // au
    byte  strike_remnants_type;           // av
    long  strike_change;                  // aw
    short strike_item;                    // ax
    short touch_item;                     // ay
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
};

enum {cfOFFSET, cfMAX};

struct newcombo
{
    word tile;
    byte flip;
    byte walk;
    byte type;
    byte csets;
    word foo; // used in zq_tiles for some reason. May be redundant. -- L.
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
    //24
};

#define AF_FRESH 1
#define	AF_CYCLE 2

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
};

enum { msLINKED };

/* Note: Printable ASCII begins at 32 and ends at 126, inclusive. */
#define MSGC_COLOUR	1  // 2 args
#define MSGC_SPEED	2  // 1 arg
#define MSGC_GOTOIFGLOBAL 3 // 3 args
#define MSGC_GOTOIFRAND 4 // 2 args
#define MSGC_GOTOIF     5  // 2 args
#define MSGC_GOTOIFCTR  6  // 3 args
#define MSGC_GOTOIFCTRPC 7  // 3 args
#define MSGC_GOTOIFTRI  8  // 2 args
#define MSGC_GOTOIFTRICOUNT  9  // 2 args
#define MSGC_CTRUP     10 // 2 args
#define MSGC_CTRDN     11 // 2 args
#define MSGC_CTRSET    12 // 2 args
#define MSGC_CTRUPPC     13 // 2 args
#define MSGC_CTRDNPC     14 // 2 args
#define MSGC_CTRSETPC    15 // 2 args
#define MSGC_GIVEITEM    16 // 1 arg
#define MSGC_TAKEITEM    17 // 1 arg
#define MSGC_SFX	20 // 1 arg
#define MSGC_MIDI	21 // 1 arg
#define MSGC_NAME	22 // 0 args, disabled
#define MSGC_NEWLINE	25 // 0 args
#define MSGC_GOTOIFYN   30 // 0 args, disabled

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
    font_max
};

#define MSGSIZE 144

#define STRINGFLAG_WRAP			1
#define STRINGFLAG_CONT			2
#define STRINGFLAG_CENTER		4
#define STRINGFLAG_RIGHT		8

struct MsgStr
{
    char s[MSGSIZE+1];
    word nextstring;
    word tile;
    byte cset;
    bool trans;
    byte font;
    short x;
    short y;   // y position of message boxes.
    unsigned short w;
    unsigned short h;
    byte sfx; // either WAV_MSG or something else.
    word listpos;
    byte vspace;
    byte hspace;
    byte stringflags;
    
    // Copy everything except listpos
    MsgStr& operator=(MsgStr &other)
    {
        strncpy(s, other.s, MSGSIZE+1);
        nextstring=other.nextstring;
        copyStyle(other);
        return *this;
    }
    
    // Copy style data - everything except s, nextstring, and listpos
    void copyStyle(MsgStr& other)
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
    }
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
    // int emusic;
    //byte padding;
    //204
    byte disableditems[iMax];
    // 460
    long flags;
};

// DMap flags
#define dmfCAVES			0x0001
#define dmf3STAIR			0x0002
#define dmfWHIRLWIND		0x0004
#define dmfGUYCAVES			0x0008
#define dmfNOCOMPASS		0x0010
#define dmfWAVY 			0x0020
#define dmfWHIRLWINDRET		0x0040
#define dmfALWAYSMSG   		0x0080
#define dmfVIEWMAP       	0x0100
#define dmfDMAPMAP         	0x0200
#define dmfMINIMAPCOLORFIX 	0x0400
#define dmfSCRIPT1 			0x0800
#define dmfSCRIPT2 			0x1000
#define dmfSCRIPT3 			0x2000
#define dmfSCRIPT4 			0x4000
#define dmfSCRIPT5 			0x8000

#define OLDMAXCOMBOALIASES 256
#define MAXCOMBOALIASES 2048

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

struct shoptype
{
    char name[32];
    byte item[3];
    byte hasitem[3];
    word price[3];
    //10
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
    byte msgtext;
    byte foo[6];
    //40
    byte foo2[256];
    //296 bytes
};

struct palcycle
{
    byte first,count,speed;
    //3
};

struct miscQdata
{
    shoptype shop[256];
    //160 (160=10*16)
    infotype info[256];
    //384 (224=14*16)
    warpring warp[9];
    //528 (144=18*8)
    palcycle cycles[256][3];
    //2832 (2304=3*256*3)
    //2850 (18=2*2)
    byte     triforce[8];                                     // positions of triforce pieces on subscreen
    //2858 (8)
    zcolors  colors;
    //3154 (296)
    word     icons[4];
    //3162 (8=2*4)
    //pondtype pond[16];
    //4314 (1152=72*16)
    word endstring;
    //  byte dummy;  // left over from a word
    //word expansion[98];
    //4512
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
    long start;
    long loop_start;
    long loop_end;
    //32
    short loop;
    short volume;
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
    
    zctune(char _title[36], long _start, long _loop_start, long _loop_end, short _loop,short _volume, void *_data, byte _format)
        : start(_start), loop_start(_loop_start), loop_end(_loop_end), loop(_loop), volume(_volume), data(_data), format(_format)
    {
        //memcpy(title, _title, 20); //NOT SAFE for short strings
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
        //memcpy(title, z.title,20); //NOT SAFE for short title strings
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
  long start;
  long loop_start;
  long loop_end;
  //32
  short loop;
  short volume;
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
    itype_boots, itype_hookshot, itype_lens, itype_hammer, itype_dinsfire,
    itype_faroreswind, itype_nayruslove, itype_bomb, itype_sbomb, itype_clock,
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
    itype_last, itype_max=255
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
enum {i_dinsfire=1, imax_dinsfire};
enum {i_faroreswind=1, imax_faroreswind};
enum {i_nayruslove=1, imax_nayruslove};
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

struct gamedata
{
    //private:
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
    bool  item[MAXITEMS];
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
    byte  lvlitems[MAXLEVELS];
    byte  lvlkeys[MAXLEVELS];
    //byte  _HCpieces;
    byte  _continue_scrn;
    word  _continue_dmap;
    //620
    /*word  _maxmagic, _magic;
    short _dmagic;*/
    //byte  _magicdrainrate;
    //byte  _canslash;                                           //Link slashes instead of stabs.
    byte _generic[256];	// Generic gamedata. 0 - Heart pieces, 1- magicdrainrate, 2-canslash, 3-wlevel,
    // 4- HC Pieces per container  5- Continue hearts  6- Continue percent (1=yes)
    //byte  padding[2];
    //636
    byte  visited[MAXDMAPS];
    //892 (256)
    byte  bmaps[MAXDMAPS*64];                                 // the dungeon progress maps
    //17276 (16384)
    word  maps[MAXMAPS2*MAPSCRSNORMAL];                       // info on map changes, items taken, etc.
    //82556 (65280)
    byte  guys[MAXMAPS2*MAPSCRSNORMAL];                       // guy counts (though dungeon guys are reset on entry)
    //115196 (32640)
    char  qstpath[2048];
    byte  icon[128];
    byte  pal[48];
    long  screen_d[MAXDMAPS*MAPSCRSNORMAL][8];                // script-controlled screen variables
    long  global_d[256];                                      // script-controlled global variables
    std::vector< ZCArray <long> > globalRAM;
    
    byte awpn, bwpn;											// Currently selected weapon slots
    
    bool isclearing; // The gamedata is being cleared
    //115456 (260)
    
    // member functions
    // public:
    gamedata()
    {
        Clear();
    }
    
    ~gamedata()
    {}
    
    void Clear();
    void Copy(const gamedata& g);
    
    gamedata &operator = (const gamedata& data)
    {
        this->Copy(data);
        this->globalRAM=data.globalRAM;
        return *this;
    }
    
    char *get_name();
    void set_name(char *n);
    
    byte get_quest();
    void set_quest(byte q);
    void change_quest(short q);
    
    word get_counter(byte c);
    void set_counter(word change, byte c);
    void change_counter(short change, byte c);
    
    word get_maxcounter(byte c);
    void set_maxcounter(word change, byte c);
    void change_maxcounter(short change, byte c);
    
    short get_dcounter(byte c);
    void set_dcounter(short change, byte c);
    void change_dcounter(short change, byte c);
    
    word get_life();
    void set_life(word l);
    void change_life(short l);
    
    word get_maxlife();
    void set_maxlife(word m);
    void change_maxlife(short m);
    
    short get_drupy();
    void set_drupy(short d);
    void change_drupy(short d);
    
    word get_rupies();
    word get_spendable_rupies();
    void set_rupies(word r);
    void change_rupies(short r);
    
    word get_maxarrows();
    void set_maxarrows(word a);
    void change_maxarrows(short a);
    
    word get_arrows();
    void set_arrows(word a);
    void change_arrows(short a);
    
    word get_deaths();
    void set_deaths(word d);
    void change_deaths(short d);
    
    word get_keys();
    void set_keys(word k);
    void change_keys(short k);
    
    word get_bombs();
    void set_bombs(word k);
    void change_bombs(short k);
    
    word get_maxbombs();
    void set_maxbombs(word b, bool setSuperBombs=true);
    void change_maxbombs(short b);
    
    word get_sbombs();
    void set_sbombs(word k);
    void change_sbombs(short k);
    
    word get_wlevel();
    void set_wlevel(word l);
    void change_wlevel(short l);
    
    byte get_cheat();
    void set_cheat(byte c);
    void change_cheat(short c);
    
    byte get_hasplayed();
    void set_hasplayed(byte p);
    void change_hasplayed(short p);
    
    dword get_time();
    void set_time(dword t);
    void change_time(long long t);
    
    byte get_timevalid();
    void set_timevalid(byte t);
    void change_timevalid(short t);
    
    byte get_HCpieces();
    void set_HCpieces(byte p);
    void change_HCpieces(short p);
    
    byte get_hcp_per_hc();
    void set_hcp_per_hc(byte val);
    
    byte get_cont_hearts();
    void set_cont_hearts(byte val);
    
    bool get_cont_percent();
    void set_cont_percent(bool ispercent);
    
    byte get_continue_scrn();
    void set_continue_scrn(byte s);
    void change_continue_scrn(short s);
    
    word get_continue_dmap();
    void set_continue_dmap(word d);
    void change_continue_dmap(short d);
    
    word get_maxmagic();
    void set_maxmagic(word m);
    void change_maxmagic(short m);
    
    word get_magic();
    void set_magic(word m);
    void change_magic(short m);
    
    short get_dmagic();
    void set_dmagic(short d);
    void change_dmagic(short d);
    
    byte get_magicdrainrate();
    void set_magicdrainrate(byte r);
    void change_magicdrainrate(short r);
    
    byte get_canslash();
    void set_canslash(byte s);
    void change_canslash(short s);
    
    short get_generic(byte c);
    void set_generic(byte change, byte c);
    void change_generic(short change, byte c);
    
    byte get_lkeys();
    
    void set_item(int id, bool value);
    void set_item_no_flush(int id, bool value);
    inline bool get_item(int id)
    {
        return item[id];
    }
    
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
    byte bombs, super_bombs;
    bool items[256];
    //94
    byte hc;
    word start_heart, cont_heart;
    byte hcp, hcp_per_hc, max_bombs, keys;
    byte arrows, max_arrows;
    word rupies;
    byte triforce;                                            // bit flags
    byte map[64];
    byte compass[64];
    byte boss_key[64];
    byte misc[16];
//  byte sword_hearts[4];
    byte last_map;                                            //last map worked on
    //220
    byte last_screen;                                         //last screen worked on
    word max_magic;
    word magic;
    byte bomb_ratio;	// ratio of super bombs to bombs
    byte msg_more_x, msg_more_y, msg_more_is_offset;
    byte subscreen;
    word start_dmap;
    byte linkanimationstyle;
    //238
    //byte expansion[98];
    //336 bytes total
    byte level_keys[MAXLEVELS];
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
    byte gravity;
    word terminalv;
    byte msg_speed;
    byte transition_type; // Can't edit, yet.
    byte jump_link_layer_threshold; // Link is drawn above layer 3 if z > this.
    byte link_swim_speed;
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


/******************/
/**  Misc Stuff  **/
/******************/

/*#undef  max
#undef  min*/
#define zc_max(a,b)  ((a)>(b)?(a):(b))
#define zc_min(a,b)  ((a)<(b)?(a):(b))

#define DCLICK_START      0
#define DCLICK_RELEASE    1
#define DCLICK_AGAIN      2
#define DCLICK_NOT        3

template <class T>
INLINE T sign(T a)
{
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

INLINE bool pfwrite(void *p,long n,PACKFILE *f)
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

INLINE bool pfread(void *p,long n,PACKFILE *f,bool keepdata)
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
    unsigned char *cp = (unsigned char *)p;
    int c;
    
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

INLINE bool p_putc(int c,PACKFILE *f)
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
    short *cp = (short *)p;
    int c;
    
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

INLINE bool p_iputw(int c,PACKFILE *f)
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
    long c;
    
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

INLINE bool p_igetd(void *p, PACKFILE *f, bool keepdata)
{
    long temp;
    bool result = p_igetl(&temp,f,keepdata);
    *(int *)p=(int)temp;
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
        
        for(int i=0; i<(int)sizeof(float); i++)
        {
            ((byte *)p)[i] = tempfloat[i];
        }
        
#else
        
        for(int i=0; i<(int)sizeof(float); i++)
        {
            ((byte *)p)[sizeof(float)-i-1] = tempfloat[i];
        }
        
#endif
    }
    
    readsize += sizeof(float);
    return true;
}

INLINE bool p_iputl(long c,PACKFILE *f)
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

INLINE bool p_mgetw(void *p,PACKFILE *f,bool keepdata)
{
    short *cp = (short *)p;
    int c;
    
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

INLINE bool p_mputw(int c,PACKFILE *f)
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
    long c;
    
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

INLINE bool p_mputl(long c,PACKFILE *f)
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

INLINE bool isinRect(int x,int y,int rx1,int ry1,int rx2,int ry2)
{
    return x>=rx1 && x<=rx2 && y>=ry1 && y<=ry2;
}

INLINE void SCRFIX()
{
    putpixel(screen,0,0,getpixel(screen,0,0));
}

// ack no, inline doesn't work this way -DD
//INLINE int new_return(int x) { fake_pack_writing=false; return x; }
#define new_return(x) {assert(x == 0); fake_pack_writing = false; return x; }

//some methods for dealing with items
int getItemFamily(itemdata *items, int item);
void removeItemsOfFamily(gamedata *g, itemdata *items, int family);
void removeItemsOfFamily(zinitdata *i, itemdata *items, int family);
void removeLowerLevelItemsOfFamily(gamedata *g, itemdata *items, int family, int level);
int getHighestLevelOfFamily(zinitdata *source, itemdata *items, int family);
int getHighestLevelOfFamily(gamedata *source, itemdata *items, int family, bool checkenabled = false);
int getItemID(itemdata *items, int family, int level);
int getCanonicalItemID(itemdata *items, int family);
int getItemIDPower(itemdata *items, int family, int power);
void addOldStyleFamily(zinitdata *dest, itemdata *items, int family, char levels);
int computeOldStyleBitfield(zinitdata *source, itemdata *items, int family);

extern void flushItemCache();
extern void removeFromItemCache(int itemid);
#define NUMSCRIPTFFC		512
#define NUMSCRIPTFFCOLD		256
#define NUMSCRIPTITEM		256
#define NUMSCRIPTGUYS		256
#define NUMSCRIPTWEAPONS	256
#define NUMSCRIPTGLOBAL		4
#define NUMSCRIPTGLOBALOLD	3
#define NUMSCRIPTLINK		3
#define NUMSCRIPTSCREEN		256

#define GLOBAL_SCRIPT_INIT 		0
#define GLOBAL_SCRIPT_GAME		1
#define GLOBAL_SCRIPT_END		2
#define GLOBAL_SCRIPT_CONTINUE 	3


#endif                                                      //_ZDEFS_H_

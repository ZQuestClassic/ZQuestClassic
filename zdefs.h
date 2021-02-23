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

#define ZELDA_VERSION   0x0192 //version of the program
#define VERSION_BUILD   114     //build number of this version
#define IS_BETA         1      //is this a beta?
#define DATE_STR        "April 20, 2001"

#define MIN_VERSION     0x0170
#define DATA_VERSION    0x0190 //version of zelda.dat

#ifndef ALLEGRO_WIP_VERSION
  #define KEY_SLASH_PAD KEY_SLASH2
#endif


/*
#define ZELDA_VERSION   0x0184
#define DATE_STR        "July 21, 2000"

#define MIN_VERSION     0x0170
#define DATA_VERSION    0x0176
*/

//comment out the next line to enable file saving
//#define _CRASHONSAVE_

#define QUEST_COUNT           4
#define HP_PER_HEART          16
#define DAMAGE_MULTIPLIER     2

extern int CSET_SIZE;
extern int CSET_SHFT;


typedef unsigned char   byte;    //0-255         ( 8 bits)
typedef unsigned short  word;    //0-65535       (16 bits)
typedef unsigned long   dword;   //0-4294967295  (32 bits)


#define SINGLE_TILE_SIZE 128
#define TILES_PER_PAGE  260
#define TILE_PAGES      126
#define OLDMAXTILES     TILES_PER_PAGE*6      // 1560 tiles
#define NEWMAXTILES     TILES_PER_PAGE*TILE_PAGES      // 32760 tiles
#define NEWTILE_SIZE2   NEWMAXTILES*SINGLE_TILE_SIZE  // 4193280 bytes (new packed format, 6 pages)
#define OLDTILE_SIZE2   OLDMAXTILES*SINGLE_TILE_SIZE  // 199680 bytes (new packed format, 6 pages)
#define OLDTILE_SIZE    TILES_PER_PAGE*4*SINGLE_TILE_SIZE  // 133120 bytes (new packed format, 4 pages)
//#define NEWTILE_SIZE    260*6*128  // 199680 bytes (new packed format, 6 pages)
//#define OLDTILE_SIZE    260*4*128  // 133120 bytes (new packed format, 4 pages)
#define TILEBUF_SIZE    320*480    // 153600 bytes (old unpacked format)

//#define MAGICDRAINRATE  2

// quest stuff
#define ZQ_TILES        0
#define ZQ_MIDIS        1       // 4 bytes
#define ZQ_CHEATS       5
#define ZQ_MAXDATA      20

#define MAXMIDIS        32      // uses bit string for midi flags, so 4 bytes


#define MAXMAPS         64     // 4 times the old number
//#define MAXMAPS         16
#define MAPSCRS         132
#define TEMPLATE        131

#define MAXMSGS         255
#define MAXDMAPS        256  //this and
#define MAXLEVELS       256  //this should be the same number (was 32)
#define MAXITEMS        256
#define MAXWPNS         256
#define MAXGUYS         256
#define MAXCOMBOS       16384

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



// misc item flags
#define iSHIELD         1
#define iBOW            2
#define iRAFT           4
#define iLADDER         8
#define iBOOK           16
#define iMKEY           32
#define iBRACELET       64
#define iMSHIELD        128


// misc2
#define iCROSS          1
#define iFLIPPERS       2
#define iBOOTS          4
#define iL2BRACELET     8
#define iHOOKSHOT       16
#define iLENS           32
#define iHAMMER         64
#define iQUIVER         128


// door codes        meaning: type | shows on subscreen map
//                       bit:  321    0

#define dWALL           0  //  000    0
#define dOPEN           1  //  000    1
#define dLOCKED         2  //  001    0
#define dUNLOCKED       3  //  001    1
#define dSHUTTER        4  //  010    0
#define dBOMB           6  //  011    0
#define dBOMBED         7  //  011    1
#define dWALK           8  //  100    0

#define dOPENSHUTTER    12 //  110    0    // special case (don't show on map)
#define d1WAYSHUTTER    14 //  111    0    // never opens


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
#define fSECRET         16    // play "secret" sfx upon entering this screen
#define fVADER          32    // play "vader" sfx instead of "roar" sfx
#define fFLOATTRAPS     64
#define fCLEARSECRET    128   // clear all enemies to trigger secret entrance


// flags3
#define fHOLDITEM       1
#define fDODONGO        2     // play dodongo sfx instead of roar sfx
#define fINVISROOM      4


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
#define ipBIGRANGE      1
#define ipHOLDUP        2
#define ipONETIME       4
#define ipDUMMY         8
#define ipCHECK         16
#define ipMONEY         32
#define ipFADE          64
#define ipENEMY         128   // enemy is carrying it around
#define ipTIMER         256
#define ipBIGTRI        512
#define ipNODRAW        1024
#define ipTMPFLASH      2048


enum { wtCAVE, wtPASS, wtEXIT, wtSCROLL, wtIWARP, wtIWARPBLK, wtIWARPOPEN,
       wtIWARPZAP, wtNOWARP, wtWHISTLE, wtMAX };

// sprite palettes
enum { spAQUA, spGLEEOK, spDIG, spGANON, spBROWN, spPILE, spBLUE, spRED,
       spGOLD, spICON1, spICON2, spICON3, spICON4, spGLEEOKF, spFROZEN };

// dmap types
enum { dmDNGN, dmOVERW, dmCAVE, dmBSOVERW, dmMAX };

// dmap flags
#define dmfCONTINUE      128
#define dmfTYPE          127

// map flags
enum { mfNONE, mfPUSH2, mfPUSH4, mfWHISTLE, mfBURN, mfARROW, mfBOMB, mfFAIRY,
       mfRAFT, mfARMOS_SECRET, mfARMOS_ITEM, mfSBOMB, mfRAFT_BRANCH, mfDIVE_ITEM,
       mfNONAME, mfZELDA,
/*16*/ mfSECRETS01, mfSECRETS02, mfSECRETS03, mfSECRETS04,
       mfSECRETS05, mfSECRETS06, mfSECRETS07, mfSECRETS08,
       mfSECRETS09, mfSECRETS10, mfSECRETS11, mfSECRETS12,
       mfSECRETS13, mfSECRETS14, mfSECRETS15, mfSECRETS16,
/*32*/ mfTRAP_H, mfTRAP_V, mfTRAP_4, mfTRAP_LR, mfTRAP_UD,
/*37*/ mfENEMY0, mfENEMY1, mfENEMY2, mfENEMY3, mfENEMY4,
       mfENEMY5, mfENEMY6, mfENEMY7, mfENEMY8, mfENEMY9, mfMAX};

// combo types
enum { cNONE, cSTAIR, cCAVE, cWATER, cARMOS, cGRAVE, cDOCK,
       cUNDEF, cPUSH_WAIT, cPUSH_HEAVY, cPUSH_HW, cL_STATUE, cR_STATUE,
       cWALKSLOW, cCVUP, cCVDOWN, cCVLEFT, cCVRIGHT, cSWIMWARP, cDIVEWARP,
       cLADDERONLY, cTRIGNOFLAG, cTRIGFLAG, cZELDA, cSLASH, cSLASHITEM,
       cPUSH_HEAVY2, cPUSH_HW2, cPOUND, cHSGRAB, cHSBRIDGE, cDAMAGE1,
       cDAMAGE2, cDAMAGE3, cDAMAGE4, cC_STATUE, cTRAP_H, cTRAP_V, cTRAP_4,
       cTRAP_LR, cTRAP_UD, cPIT, cHOOKSHOT, cOVERHEAD, cNOFLYZONE, cMIRROR,
       cMIRRORSLASH, cMIRRORBACKSLASH, cMAGICPRISM, cMAGICPRISM4,
       cMAGICSPONGE, cCAVE2, cMAX };

// "quest rules" flags (bit numbers in bit string)
enum { qr1_SOLIDBLK, qr1_NOTMPNORET, qr1_ALWAYSRET, qr1_MEANTRAPS,
       qr1_BSZELDA, qr1_FADE, qr1_FADECS5, qr1_FASTDNGN, qr1_NOLEVEL3FIX,
       qr1_COOLSCROLL, qr1_999R, qr1_4TRI, qr1_24HC, qr1_FASTFILL,
       qr1_VIEWMAP, qr1_3TRI, qr1_MAX };

// "quest rules two" flags (bit numbers in bit string)
enum { qr2_TIME, qr2_NEWSUBSCR, qr2_KILLALL, qr2_NOFLICKER, qr2_CONTFULL,
       qr2_RLFIX, qr2_NOSBEAM, qr2_NOWSBEAM, qr2_NOMSBEAM, qr2_NOXSBEAM,
       qr2_BEAMHALFPWR, qr2_SASPARKLES, qr2_GASPARKLES, qr2_MBSPARKLES,
       qr2_FBSPARKLES, qr2_NOFLASHDEATH, qr2_MAX };

// "quest rules three" flags (bit numbers in bit string)
enum { qr3_REPAIRFIX, qr3_FIREPROOFLINK, qr3_OUCHBOMBS, qr3_NOCLOCKS,
       qr3_TEMPCLOCKS, qr3_NOSHIELDS, qr3_NOUNDERCOMBOS, qr3_MEANPLACEDTRAPS,
       qr3_PHANTOMPLACEDTRAPS, qr3_ALLOWFASTMSG, qr3_LINKEDCOMBOS, qr3_NOGUYFIRES,
       qr3_HEARTRINGFIX, qr3_NOHEARTRING, qr3_DODONGOCOLORFIX, qr3_SWORDWANDFLIPFIX, qr3_MAX };

// "quest rules four" flags (bit numbers in bit string)
enum { qr4_ENABLEMAGIC, qr4_MAGICWAND, qr4_MAGICCANDLE, qr4_MAGICBOOTS,
       qr4_NONBUBBLEMEDICINE, qr4_NONBUBBLEFAIRIES, qr4_NONBUBBLETRIFORCE, qr4_NEWENEMYTILES,
       qr4_NOROPE2FLASH, qr4_NOBUBBLEFLASH, qr4_GHINI2BLINK, qr4_WPNANIMFIX,
       qr4_13, qr4_14, qr4_15, qr4_16, qr4_MAX };

// "quest rules five" flags (bit numbers in bit string)
enum { qr5_1, qr5_2, qr5_3, qr5_4,
       qr5_5, qr5_6, qr5_7, qr5_8,
       qr5_9, qr5_10, qr5_11, qr5_12,
       qr5_13, qr5_14, qr5_15, qr5_16, qr5_MAX };

// "quest rules six" flags (bit numbers in bit string)
enum { qr6_1, qr6_2, qr6_3, qr6_4,
       qr6_5, qr6_6, qr6_7, qr6_8,
       qr6_9, qr6_10, qr6_11, qr6_12,
       qr6_13, qr6_14, qr6_15, qr6_16, qr6_MAX };

// "quest rules seven" flags (bit numbers in bit string)
enum { qr7_1, qr7_2, qr7_3, qr7_4,
       qr7_5, qr7_6, qr7_7, qr7_8,
       qr7_9, qr7_10, qr7_11, qr7_12,
       qr7_13, qr7_14, qr7_15, qr7_16, qr7_MAX };

// "quest rules eight" flags (bit numbers in bit string)
enum { qr8_1, qr8_2, qr8_3, qr8_4,
       qr8_5, qr8_6, qr8_7, qr8_8,
       qr8_9, qr8_10, qr8_11, qr8_12,
       qr8_13, qr8_14, qr8_15, qr8_16, qr8_MAX };

// "quest rules nine" flags (bit numbers in bit string)
enum { qr9_1, qr9_2, qr9_3, qr9_4,
       qr9_5, qr9_6, qr9_7, qr9_8,
       qr9_9, qr9_10, qr9_11, qr9_12,
       qr9_13, qr9_14, qr9_15, qr9_16, qr9_MAX };

// "quest rules ten" flags (bit numbers in bit string)
enum { qr10_1, qr10_2, qr10_3, qr10_4,
       qr10_5, qr10_6, qr10_7, qr10_8,
       qr10_9, qr10_10, qr10_11, qr10_12,
       qr10_13, qr10_14, qr10_15, qr10_16, qr10_MAX };

// room types
enum { rNONE, rSP_ITEM, rINFO, rMONEY, rGAMBLE, rREPAIR, rRP_HC, rGRUMBLE,
       rTRIFORCE, rP_SHOP, rSHOP, rBOMBS, rSWINDLE, r10RUPIES, rWARP,
       rGANON, rZELDA, rITEMPOND, rMUPGRADE,
       rMAX };

// directions
enum { up, down, left, right, l_up, r_up, l_down, r_down };

// refill stuff
enum { REFILL_LIFE, REFILL_MAGIC, REFILL_ALL};
enum { REFILL_POTION, REFILL_FAIRY, REFILL_TRIFORCE};

// magic rates
#define LENSDRAINAMOUNT      2
#define LENSDRAINSPEED       1
#define WANDDRAINAMOUNT      8
#define CANDLEDRAINAMOUNT    4
#define DINSFIREDRAINAMOUNT  32
#define BOOTSDRAINAMOUNT     1
#define BOOTSDRAINSPEED      1

// items
enum { // value matters because of current quest file data
       iRupy, i5Rupies, iHeart, iBombs, iClock,
       iSword, iWSword, iMSword, iShield, iKey,
/*10*/ iBCandle, iRCandle, iLetter, iArrow, iSArrow,
       iBow, iBait, iBRing, iRRing, iBracelet,
/*20*/ iTriforce, iMap, iCompass, iBrang, iMBrang,
       iWand, iRaft,iLadder,iHeartC, iBPotion,
/*30*/ iRPotion, iWhistle,iBook, iMKey, iFairy,
       iFBrang, iXSword, iMShield, i20Rupies, i50Rupies,
/*40*/ i200Rupies, iWallet500, iWallet999, iPile, iBigTri,
       iSelect, iMisc1, iMisc2, iSBomb, iHCPiece,
/*50*/ iCross, iFlippers, iHookshot, iLens, iHammer,
       iBoots, iL2Bracelet, iGArrow, iMagicC, iSMagic,
/*60*/ iLMagic, iGRing, iMax };


// item sets
enum { isNONE, isDEFAULT, isBOMBS, isMONEY, isLIFE, isBOMB100, isSBOMB100,
       isMAGIC, isMAGICBOMBS, isMAGICMONEY, isMAGICLIFE, isMAGIC2};


// weapons (in qst data)
enum { wSWORD,wWSWORD,wMSWORD,wXSWORD,wBRANG,wMBRANG,wFBRANG,wBOMB,
       wSBOMB,wBOOM,wARROW,wSARROW,wFIRE,wWIND,wBAIT,wWAND,wMAGIC,
       ewFIREBALL,ewROCK,ewARROW,ewSWORD,ewMAGIC,iwSpawn,iwDeath,iwSwim,
       wHAMMER, wHSHEAD, wHSCHAIN, wHSHANDLE, wSSPARKLE, wGSPARKLE,
       wMSPARKLE, wFSPARKLE, wSMACK, wGARROW, ewFLAME, ewWIND, iwMMeter,
       wDINSFIRE1A, wDINSFIRE1B, wDINSFIRES1A, wDINSFIRES1B, wMAX };

// weapon types in game engine
enum { wNone,wSword,wBeam,wBrang,wBomb,wSBomb,wLitBomb,wLitSBomb,wArrow,
       wFire,wWhistle,wBait,wWand,wMagic,wCatching,wWind,wRefMagic,wRefFireball,
       wEnemyWeapons,
       ewFireball,ewArrow,ewBrang,ewSword,ewRock,ewMagic, wHammer, wHookshot,
       wHSHandle, wHSChain, wSSparkle, wGSparkle, wMSparkle, wFSparkle,
       wSmack, wGArrow, ewFlame, ewWind, wPhantom };

enum { gABEI=1,gAMA,gDUDE,gMOBLIN,gFIRE,
       gFAIRY,gGORIYA,gZELDA,gDABEI,

       eROCTO1=10,eBOCTO1,eROCTO2,eBOCTO2,eRTEK,
       eBTEK,eRLEV,eBLEV,eRMOLBLIN,eBMOLBLIN,
/*20*/ eRLYNEL,eBLYNEL,ePEAHAT,eZORA,eROCK,
       eGHINI1,eGHINI2,eARMOS,

	                       eKEESE1,eKEESE2,
/*30*/ eKEESE3,eSTALFOS,eGEL,eZOL,eROPE,
       eRGORIYA,eBGORIYA,eTRAP,eWALLM,eRDKNUT,
/*40*/ eBDKNUT,eBUBBLE,eVIRE,eLIKE,eGIBDO,
       ePOLSV,eRWIZ,eBWIZ,

	                   eRAQUAM,eMOLDORM,
/*50*/ eDODONGO,eMANHAN,eGLEEOK1,eGLEEOK2,eGLEEOK3,eGLEEOK4,
       eDIG1,eDIG3,eDIGPUP,eRGOHMA,eBGOHMA,eRCENT,
/*60*/ eBCENT,ePATRA1,ePATRA2,eGANON,
                                      eSTALFOS2,
       eROPE2,eRBUBBLE,eBBUBBLE,eFBALL,eITEMFAIRY,
/*70*/ eFIRE,eCOCTO,eDKNIGHT, eGELTRIB, eZOLTRIB,
       eKEESETRIB, eVIRETRIB, eSDKNUT, eLAQUAM, eMANHAN2,
/*80*/ eTRAP_H, eTRAP_V, eTRAP_LR, eTRAP_UD, eFWIZ,
       eWWIZ, eCEILINGM, eFLOORM, ePATRABS, ePATRAL2,
/*90*/ ePATRAL3, eBAT, eBATROBE, eBATROBEKING, eGLEEOK1F, eGLEEOK2F,
       eGLEEOK3F, eGLEEOK4F, eMWIZ, eDODONGOBS, eDODONGOF, eTRIGGER,
       eMAXGUYS };


// enemy families
enum { eeGUY, eeWALK, eeSHOOT, eeTEK, eeLEV, eePEAHAT, eeZORA, eeROCK,
       eeGHINI, eeARMOS, eeKEESE, eeGEL, eeZOL, eeROPE, eeGORIYA, eeTRAP,
       eeWALLM, eeBUBBLE, eeVIRE, eeLIKE, eePOLSV, eeWIZZ, eeAQUA, eeMOLD,
       eeDONGO, eeMANHAN, eeGLEEOK, eeDIG, eeGHOMA, eeLANM, eePATRA, eeGANON,
       eeFBALL, eeGELTRIB, eeZOLTRIB, eeVIRETRIB, eeKEESETRIB, eeNONE };

// enemy animation styles
enum { aNONE, aFLIP, a2FRM, aOCTO, aTEK, aLEV, aWALK, aZORA, aGHINI, aARMOS,
       aROPE, aWALLM, aDWALK, aVIRE, a3FRM, aWIZZ, aAQUA, aDONGO, aMANHAN,
       aGLEEOK, aDIG, aGHOMA, aLANM, aPHAT, aKEESE, aGANON };

// enemy patters
enum { pRANDOM, pSIDES };

typedef struct itemdata {
  word tile;
  byte misc;     // 0000vhtf (vh:flipping, t:two hands, f:flash)
  byte csets;    // ffffcccc (f:flash cset, c:cset)
  byte frames;   // animation frame count
  byte speed;    // animation speed
  byte delay;    // extra delay factor (-1) for first frame
  byte exp;      // not used
  // 8 bytes
} itemdata;


typedef struct wpndata {
  word tile;
  byte misc;     // 0000vhff (vh:flipping, f:flash (1:NES, 2:BSZ))
  byte csets;    // ffffcccc (f:flash cset, c:cset)
  byte frames;   // animation frame count
  byte speed;    // animation speed
  byte type;     // used by certain weapons
  byte exp;      // not used
  // 8 bytes
} wpndata;



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


typedef struct guydata {
  dword flags;
  word  tile;
  short hp;

  short  family, cset, anim, frate;
  short  dp, wdp, weapon;

  short  rate, hrate, step, homing, grumble;
  short  item_set, misc1, misc2, bosspal;

  short  startx, starty;
  short  foo1,foo2,foo3,foo4,foo5,foo6;
// 56 bytes
} guydata;

/*
typedef struct guydata {
  dword flags;
  word  tile;
  short hp;

  byte  family, cset, anim, frate;
  byte  dp, wdp, weapon;

  byte  rate, hrate, step, homing, grumble;
  byte  item_set, misc1, misc2, bosspal;

  byte  startx, starty;
  byte  foo1,foo2,foo3,foo4,foo5,foo6;
// 32 bytes
} guydata;
*/


typedef struct oldmapscr {
// 48-byte header (old format)
  byte valid,guy,str,room,        item,__catchall,warptype,__flags;
  byte warpx,warpy,stairx,stairy, itemx,itemy,color,enemyflags;
  byte door[4], warpdmap,warpscr,exitdir,__flags2;
  byte enemy[10],pattern,warptype2,warpx2,warpy2,path[4];
  byte warpscr2,warpdmap2;
  word under;
  byte cpage,__secret;
// 40-byte expansion
  word catchall;
  byte flags,flags2,flags3;
  byte extra[15];
  byte secret[20];
// 352 bytes of screen data
  word data[16*11];
// 440 bytes total
} oldmapscr;


typedef struct mapscr192b9 {
// 48-byte header (old format)
  byte valid,guy,str,room,        item,_FOO1_,warptype,_FOO2_;
  byte warpx,warpy,stairx,stairy, itemx,itemy,color,enemyflags;
  byte door[4], warpdmap,warpscr,exitdir,_FOO3_;
  byte enemy[10],pattern,warptype2,warpx2,warpy2,path[4];
  byte warpscr2,warpdmap2;
  word under;
  byte cpage,_FOO4_;
// 40-byte expansion
  word catchall;
  byte flags,flags2,flags3;
  byte extra[15];
  byte secret[20];
// 352 bytes of screen data
  word data[16*11];
// 440 bytes total
} mapscr192b9;

typedef struct mapscr192b20 {
// 48-byte header (old format)
  byte valid,guy,str,room,        item,_FOO1_,warptype,_FOO2_;
  byte warpx,warpy,stairx,stairy, itemx,itemy,color,enemyflags;
  byte door[4], warpdmap,warpscr,exitdir,_FOO3_;
  word enemy[10];
  byte pattern,warptype2,warpx2,warpy2,path[4];
  byte warpscr2,warpdmap2;
  word under;
  byte cpage,_FOO4_;
// 40-byte expansion
  word catchall;
  byte flags,flags2,flags3;
  byte extra[15];
  byte secret[20];
// 352 bytes of screen data
  word data[16*11];
// 440 bytes total
} mapscr192b20;

typedef struct mapscr192b23 {
// 48-byte header (old format)
  byte valid,guy,str,room,        item,_FOO1_,warptype,_FOO2_;
  byte warpx,warpy,stairx,stairy, itemx,itemy,color,enemyflags;
  byte door[4], warpdmap,warpscr,exitdir,_FOO3_;
  word enemy[10];
  byte pattern,warptype2,warpx2,warpy2,path[4];
  byte warpscr2,warpdmap2;
  word under;
  byte cpage,_FOO4_;
// 40-byte expansion
  word catchall;
  byte flags,flags2,flags3;
  byte extra[15];
  byte secret[20];
// 352 bytes of screen data
  word data[16*11];
  int  sflag[16*11];
// 616 bytes total
} mapscr192b23;


typedef struct mapscr192b97 {
// 48-byte header (old format)
  byte valid,guy,str,room,        item,_FOO1_,warptype,_FOO2_;
  byte warpx,warpy,stairx,stairy, itemx,itemy,color,enemyflags;
  byte door[4], warpdmap,warpscr,exitdir,_FOO3_;
  word enemy[10];
  byte pattern,warptype2,warpx2,warpy2,path[4];
  byte warpscr2,warpdmap2;
  word under;
  byte cpage,_FOO4_;
// 40-byte expansion
  word catchall;
  byte flags,flags2,flags3;
  byte groundovermap;
  byte groundoverscreen;
  byte skyovermap;
  byte skyoverscreen;
  byte extra[11];
  byte secret[20];
// 352 bytes of screen data
  word data[16*11];
  byte sflag[16*11];
// 616 bytes total
} mapscr192b97;

typedef struct mapscr {
// 48-byte header (old format)
  byte valid,guy,str,room,        item,_FOO1_,warptype,_FOO2_;
  byte warpx,warpy,stairx,stairy, itemx,itemy,color,enemyflags;
  byte door[4], warpdmap,warpscr,exitdir,_FOO3_;
  word enemy[10];
  byte pattern,warptype2,warpx2,warpy2,path[4];
  byte warpscr2,warpdmap2;
  word under;
  byte cpage,_FOO4_;
// 40-byte expansion
  word catchall;
  byte flags,flags2,flags3;
  byte layermap[6];
  byte layerscreen[6];
  byte extra[32];
  byte secret[20];
// 352 bytes of screen data
  word data[16*11];
  byte sflag[16*11];
  byte sflag2[16*11];
// 616 bytes total
} mapscr;


typedef struct oldcombo {
  word tile;
  byte flip;
  byte walk;
  byte type;
  byte csets;
  word foo;
} oldcombo;

#define COMBO_FOO2S 6

typedef struct newcombo {
  word tile;
  byte flip;
  byte walk;
  byte type;
  byte csets;
  word foo; //do not change!  used for positioning!  no idea why.
  byte frames;
  byte speed;
  word drawtile;
  word foo2[COMBO_FOO2S];
} newcombo;


typedef struct ZCHEATS {
  dword flags;
  char  codes[4][41];
} ZCHEATS;


/*
typedef struct oldcombo {
  word tile;
  byte flip;
  byte walk;
  byte type;
  byte attr;
  word e[5];
} oldcombo;
*/


#define QH_IDSTR "AG Zelda Classic Quest File\n "
#define ENC_STR  "Zelda Classic Quest File"

typedef struct zquestheader {
 char  id_str[31];
 short zelda_version;
 word  internal;
 byte  quest_number;
 byte  rules[2];
 byte  map_count;
 char  str_count;
 byte  data_flags[ZQ_MAXDATA];
 byte  rules2[2];
 char  options;
 char  version[9];
 char  title[65];
 char  author[65];
 short pwdkey;
 char  password[30];
 char  minver[9];
 byte  build;
 byte  foo[10];
 byte  rules3[2];
 byte  rules4[2];
 byte  rules5[2];
 byte  rules6[2];
 byte  rules7[2];
 byte  rules8[2];
 byte  rules9[2];
 byte  rules10[2];
 byte  foo2[50];

 // 306 bytes
} zquestheader;


typedef struct oldheader {
 char  id_str[31];
 short zelda_version;
 short internal;
 byte  quest_number;
 byte  rules[2];
 byte  map_count;
 char  str_count;
 byte  data_flags[ZQ_MAXDATA];
 byte  rules2[2];
 char  options;
 char  version[9];
 char  title[65];
 char  author[65];
 short pwdkey;
 char  password[30];
 // 236 bytes
} oldheader;



typedef struct MsgStr {
 char s[73];
 byte d1,d2,d3;
// 76 bytes total
} MsgStr;

/*
typedef struct MsgStr {
 char s[65536];
 byte d1,d2,d3;
// 76 bytes total
} MsgStr;
*/


typedef struct dmap {
 byte map;
 byte level;
 char xoff;
 byte compass;
 byte color;
 byte midi;
 byte cont;
 byte type;
 byte grid[8];
 char name[21];
 char title[21];
 char intro[73];
// 16 bytes total
} dmap;



typedef struct shoptype {
 byte item[3];
 byte d1;
 word price[3];
 // 10 bytes
} shoptype;

typedef struct pondtype {
 byte olditem[3];
 byte d1;
 byte newitem[3];
 byte fairytile;
 byte aframes;
 byte aspeed;
 byte msg[15];
 // 25 bytes (previously 11)
} pondtype;

typedef struct infotype {
 byte str[3];
 byte d1;
 word price[3];
 // 10 bytes
} infotype;

typedef struct warpring {
 byte dmap[8];
 byte scr[8];
 byte size;
 byte d1;
} warpring;

typedef struct windwarp {
 byte dmap;
 byte scr;
} windwarp;


typedef struct zcolors
{
  byte text, caption;
  byte overw_bg, dngn_bg;
  byte dngn_fg, cave_fg;
  byte bs_dk, bs_goal;
  byte compass_lt, compass_dk;
  // 10
  byte subscr_bg, triframe_color;
  byte link_dot;
  byte bmap_bg,bmap_fg;
  // 15
  byte triforce_cset;
  byte triframe_cset;
  byte overworld_map_cset;
  byte dungeon_map_cset;
  byte blueframe_cset;
  // 20
  word triforce_tile;
  word triframe_tile;
  word overworld_map_tile;
  word dungeon_map_tile;
  word blueframe_tile;
  // 30
  word HCpieces_tile;
  byte HCpieces_cset;
  byte foo[7];
  // 40 bytes
} zcolors;


typedef struct palcycle {
 byte first,count,speed;
} palcycle;


typedef struct miscQdata192b29 {
 shoptype shop[16];
 infotype info[16];
 warpring warp[8];
 palcycle cycles[16][3];
 windwarp wind[9];      // destination of whirlwind for each level
 byte     triforce[8];  // positions of triforce pieces on subscreen
 zcolors  colors;
 word     icons[3];
} miscQdata192b29;

typedef struct miscQdata192b72 {
 shoptype shop[16];
 infotype info[16];
 warpring warp[8];
 palcycle cycles[16][3];
 windwarp wind[9];      // destination of whirlwind for each level
 byte     triforce[8];  // positions of triforce pieces on subscreen
 zcolors  colors;
 word     icons[3];
 pondtype pond[16];
 byte endstring;
 byte dummy;  // left over from a word
 word expansion[99];
} miscQdata192b72;

typedef struct miscQdata {
 shoptype shop[16];
 infotype info[16];
 warpring warp[8];
 palcycle cycles[256][3];
 windwarp wind[9];      // destination of whirlwind for each level
 byte     triforce[8];  // positions of triforce pieces on subscreen
 zcolors  colors;
 word     icons[4];
 pondtype pond[16];
 byte endstring;
 byte dummy;  // left over from a word
 word expansion[98];
} miscQdata;

typedef struct music {
 char title[20];
 long start;
 long loop_start;
 long loop_end;
 short loop;
 short volume;
 MIDI *midi;
} music;


typedef struct gamedata {
  char  name[9];
  byte  quest,extra[16];
  short life,maxlife,drupy,rupies,deaths;
  byte  sword,misc,brang,bombs,arrow,candle,whistle,bait,letter,potion,wand,
        ring,keys,maxbombs,wlevel,cheat,wallet;
  char  version[9];
  char  title[65];
  byte  hasplayed;
  dword time;
  byte  timevalid;
  byte  sbombs;
  byte  lvlitems[32];
  byte  HCpieces;
  byte  misc2;
  byte  continue_dmap;
  int   maxmagic;
  int   magic;
  int   dmagic;
  byte  magicdrainrate;
  byte  expansion[99];
  // 256 bytes so far ??

  byte  visited[MAXDMAPS];
  byte  bmaps[MAXDMAPS*64]; // the dungeon progress maps
  byte  maps[MAXMAPS*128];  // info on map changes, items taken, etc.
  byte  guys[MAXMAPS*128];  // guy counts (though dungeon guys are reset on entry)
  char  qstpath[84];
  byte  icon[128];
  byte  pal[48];
} gamedata;

typedef struct oldgamedata {
  char  name[9];
  byte  quest,extra[16];
  short life,maxlife,drupy,rupies,deaths;
  byte  sword,misc,brang,bombs,arrow,candle,whistle,bait,letter,potion,wand,
        ring,keys,maxbombs,wlevel,cheat,wallet;
  char  version[9];
  char  title[65];
  byte  hasplayed;
  dword time;
  byte  timevalid;
  byte  sbombs;
  byte  lvlitems[MAXLEVELS];
  byte  HCpieces;
  byte  misc2;
  byte  expansion[88];
  // 256 bytes so far

  byte  bmaps[MAXDMAPS*64]; // the dungeon progress maps
  byte  maps[MAXMAPS*128];  // info on map changes, items taken, etc.
  byte  guys[MAXMAPS*128];  // guy counts (though dungeon guys are reset on entry)
  char  qstpath[84];
  byte  icon[128];
  byte  pal[48];
} oldgamedata;


// "initialization data" flags (bit numbers in bit string)
enum { idE_RAFT, idE_LADDER, idE_BOOK, idE_KEY,
       idE_FLIPPERS, idE_BOOTS, idE_MAX };
enum { idI_WAND, idI_LETTER, idI_LENS, idI_HOOKSHOT,
       idI_BAIT, idI_HAMMER, idI_MAX };
enum { idM_CONTPERCENT, idM_DOUBLEMAGIC, idM_MAX };

typedef struct zinitdata {
byte ring, sword, shield, wallet, bracelet, amulet, bow;
byte candle, boomerang, arrow, potion, whistle, bombs, super_bombs;
byte equipment, items;
byte hc, start_heart, cont_heart, hcp, max_bombs, keys;
word rupies;
byte triforce; // bitflags
// 24 bytes
byte map[32];
byte compass[32];
byte misc[16];
// 89 bytes
byte swordhearts[4];
byte lastmap;  //last map worked on
byte lastscreen;  //last screen worked on
byte maxmagic;
byte magic;
byte expansion[142];
// 256 bytes total
} zinitdata;


/******************/
/**  Misc Stuff  **/
/******************/


#define max(a,b)  ((a)>(b)?(a):(b))
#define min(a,b)  ((a)<(b)?(a):(b))


template <class T>
static inline void swap(T &a,T &b)
{
  T c = a;
  a = b;
  b = c;
}


inline bool pfwrite(void *p,long n,PACKFILE *f)
{
  return pack_fwrite(p,n,f)==n;
}


inline bool pfread(void *p,long n,PACKFILE *f)
{
  return pack_fread(p,n,f)==n;
}


inline bool isinRect(int x,int y,int rx1,int ry1,int rx2,int ry2)
{
  return x>=rx1 && x<=rx2 && y>=ry1 && y<=ry2;
}



#endif //_ZDEFS_H_




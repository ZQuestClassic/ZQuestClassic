/*
  zdefs.h
  Data formats, definitions, and a few small functions for zelda.cc
  and zquest.cc
  Jeremy Craner, 1999
*/


#ifndef _ZDEFS_H_
#define _ZDEFS_H_

#define ZELDA_VERSION  0x0100
#define MIN_VERSION    0x0100
#define DATA_VERSION   0x0099

#define QUEST_COUNT  3
short min_version[QUEST_COUNT] = { 0x0100, 0x0100, 0x0100 };


#ifndef byte
typedef unsigned char byte;
#endif

#ifndef word
typedef unsigned short word;
#endif

#ifndef dword
typedef unsigned long dword;
#endif


#define TILEBUF_SIZE    320*480


// quest stuff
#define ZQ_MAXDATA      20

#define ZQ_TILES        0
#define ZQ_MIDIS        1       // uses bit string for midi flags

#define MAXMIDIS        16      // uses bit string for midi flags, so 2 bytes
#define MAXMAPS         10
#define MAXMSGS         128
#define MAXDMAPS        32

#define TEMPLATE        MAXMAPS*130



// mapscr "valid" byte
#define mVALID          0x01
#define mVERSION        0x80

// tile offsets
#define DOT             11


// palette stuff
int CSET_SIZE = 16;     // this is only changed to 4 in the NES title screen
int CSET_SHFT = 4;      // log2 of CSET_SIZE
#define CSET(x)         ((x)<<CSET_SHFT)
#define csBOSS          14


// lvlitems flags
#define liTRIFORCE      1
#define liMAP           2
#define liCOMPASS       4
#define liBOSS          8



// misc item flags
#define iSHIELD         1
#define iBOW            2
#define iRAFT           4
#define iLADDER         8
#define iBOOK           16
#define iMKEY           32
#define iBRACELET       64



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


// sprite palettes
enum { spAQUA, spGLEEOK, spDIG, spGANON, spBROWN, spPILE };

// dmap types
enum { dmDNGN, dmOVERW, dmCAVE, dmBSDNGN, dmBSOVERW, dmBSCAVE, dmMAX };

// map flags
enum { mf0, mfPUSH2, mfPUSH4, mfBURN, mfBURN_S, mfBOMB, mfBOMB_C,
       mfFAIRY, mfRAFT, mfARMOS_STAIR, mfARMOS_ITEM, mfZELDA=15 };

// combo types
enum { cNONE, cSTAIR, cCAVE, cWATER, cARMOS, cGRAVE, cDOCK,
       cUNDEF, cPUSH_WAIT, cPUSH_HEAVY, cPUSH_HW, cL_STATUE, cR_STATUE,
       cMAX };


// room types
enum { rSP_ITEM=1, rINFO, rMONEY, rGAMBLE, rREPAIR, rRP_HC, rGRUMBLE,
       rTRIFORCE, rP_SHOP, rSHOP, rBOMBS, rSWINDLE, r10RUPIES, rWARP,
       rGANON, rZELDA,
       rMAX };

enum { up, down, left, right, l_up, r_up, l_down, r_down };

enum { iRupy,i5Rupies,iHeart,iBombs,iClock,
       iSword,iWSword,iMSword,iShield,iKey,
       iBCandle,iRCandle,iLetter,iArrow,iSArrow,
       iBow,iBait,iBRing,iRRing,iBracelet,
       iTriforce,iMap,iCompass,iBrang,iMBrang,
       iWand,iRaft,iLadder,iHeartC,iBPotion,
       iRPotion,iWhistle,iBook,iMKey,iFairy,
       iFBrang,iXSword,iMShield,i20Rupies,i50Rupies,
       iPile,iBigTri,iMax };

enum { wNone,wSword,wBeam,wBrang,wBomb,wArrow,wFire,wWhistle,wBait,
       wWand,wMagic,wCatching,wLitBomb,wWind,
       wEnemyWeapons,
       ewFireball,ewArrow,ewBrang,ewSword,ewRock,ewMagic };

enum { gABEI=1,gAMA,gDUDE,gMOBLIN,gFIRE,
       gFAIRY,gGORIYA,gZELDA,

       eROCTO1=10,eBOCTO1,eROCTO2,eBOCTO2,eRTEK,
       eBTEK,eRLEV,eBLEV,eRMOLBLIN,eBMOLBLIN,
/*20*/ eRLYNEL,eBLYNEL,ePEAHAT,eZORA,eROCK,
       eGHINI1,eGHINI2,eARMOS,

	                       eKEESE1,eKEESE2,
/*30*/ eKEESE3,eSTALFOS,eGEL,eZOL,eROPE,
       eRGORIYA,eBGORIYA,eTRAP,eWALLM,eRDKNUT,
/*40*/ eBDKNUT,eBUBBLE,eVIRE,eLIKE,eGIBDO,
       ePOLSV,eRWIZ,eBWIZ,

	                   eAQUAM,eMOLDORM,
/*50*/ eDODONGO,eMANHAN,eGLEEOK2,eGLEEOK3,eGLEEOK4,
       eDIG1,eDIG3,eRGOHMA,eBGOHMA,eRCENT,
/*60*/ eBCENT,ePATRA1,ePATRA2,eGANON,
                                      eSTALFOS2,
       eROPE2,eRBUBBLE,eBBUBBLE,eGLEEOK5,eGLEEOK6,
       eGLEEOK7,eGLEEOK8,

       eMAXGUYS,eFBALL,eITEMFAIRY,eFIRE };



typedef struct mapscr {
// 48-byte header
  byte valid,guy,str,room,        item,catchall,warptype,flags;
  byte warpx,warpy,stairx,stairy, itemx,itemy,color,enemyflags;
  byte door[4], warpdmap,warpscr,exitdir,flags2;
  byte enemy[10],pattern, warptype2,warpx2,warpy2,path[4];
  byte warpscr2,warpdmap2;
  word under;
  byte unused_data[2];
// 352 bytes of screen data
  word data[16*11];
// 400 bytes total
} mapscr;



typedef struct combo {
  word tile;
  byte flip;
  byte walk;
  byte type;
  byte attr;
  word e[5];
} combo;


#define QH_IDSTR "AG Zelda Classic Quest File\n "

typedef struct zquestheader {
 char  id_str[31];
 short zelda_version;
 short internal;
 byte  quest_number;
 char  foo[2];        // unused
 char  map_count;
 char  str_count;
 byte  data_flags[ZQ_MAXDATA];
 char  foo2[3];       // unused
 char  version[9];
 char  title[65];
 char  author[65];
 short pwdkey;
 char  password[30];
} zquestheader;



typedef struct MsgStr {
 char s[73];
 byte d1,d2,d3;
// 76 bytes total
} MsgStr;




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
// 16 bytes total
} dmap;



typedef struct shoptype {
 byte item[3];
 byte price[3];
 byte d1,d2;
} shoptype;

typedef struct infotype {
 byte str[3];
 byte price[3];
 byte d1,d2;
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


typedef struct zcolors {
 byte text,caption;
 byte triforce,triframe;
 byte overw;
 byte map;
 byte dngn_bg, dngn_fg;
 byte cave_bg, cave_fg;
 byte bs_lt, bs_med, bs_dk, bs_goal;
// 14 bytes
} zcolors;


typedef struct miscQdata {
 shoptype shop[16];
 infotype info[16];
 warpring warp[16];
 windwarp wind[9];      // destination of whirlwind for each level
 byte     triforce[8];  // positions of triforce pieces on subscreen
 zcolors  colors;
 byte     misc[32];
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



//******************
// other misc stuff
//******************


#ifndef max
#define max(a,b)  ((a)>(b)?(a):(b))
#endif

#ifndef min
#define min(a,b)  ((a)<(b)?(a):(b))
#endif

int vbound(int x,int low,int high)
{
 if(x<low) x=low;
 if(x>high) x=high;
 return x;
}

inline bool isinRect(int x,int y,int rx1,int ry1,int rx2,int ry2)
{
 return x>=rx1 && x<=rx2 && y>=ry1 && y<=ry2;
}

#ifndef swap
template <class T>
inline void swap(T &a,T &b)
{
 T c = a;
 a = b;
 b = c;
}
#endif

int used_switch(int argc,char *argv[],char *s)
{
 // assumes a switch won't be in argv[0]
 for(int i=1; i<argc; i++)
  if(strcmp(argv[i],s)==0)
    return i;
 return 0;
}


char zeldapwd[8]  = { 'j'+11,'i'+22,'e'+33,'r'+44,'u'+55,0+66,'x'+77,'c'+88 };
char zquestpwd[8] = { 'c'+11,'a'+22,'o'+33,'c'+44,'a'+55,'o'+66,0+77,'k'+88 };
char datapwd[8]   = { 'l'+11,'o'+22,'n'+33,'g'+44,'t'+55,'a'+66,'n'+77,0+88 };

void resolve_password(char *pwd)
{
  for(int i=0; i<8; i++)
    pwd[i]-=(i+1)*11;
}



void set_bit(byte *bitstr,int bit,byte val)
{
  bitstr += bit>>3;
  byte mask = 1 << (bit&7);

  if(val)
    *bitstr |= mask;
  else
    *bitstr &= ~mask;
}


int get_bit(byte *bitstr,int bit)
{
  bitstr += bit>>3;
  return ((*bitstr) >> (bit&7))&1;
}



#endif //_ZDEFS_H_


/*** end of zdefs.h ***/


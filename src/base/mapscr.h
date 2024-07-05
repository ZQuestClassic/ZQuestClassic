#ifndef MAPSCR_H_
#define MAPSCR_H_

#include "base/zdefs.h"
#include "ffc.h"
#include <vector>

// door codes        meaning: type | shows on subscreen map
//                       bit: 4321    0
#define dWALL           0     // 0000    0
#define dOPEN           1     // 0000    1
#define dLOCKED         2     // 0001    0
#define dUNLOCKED       3     // 0001    1
#define dSHUTTER        4     // 0010    0
#define dBOMB           6     // 0011    0
#define dBOMBED         7     // 0011    1
#define dWALK           8     // 0100    0

#define dBOSS           10    // 0101    0
#define dOPENBOSS       11    // 0101    1

#define dOPENSHUTTER    12    // 0110    0    // special case (don't show on map)
#define d1WAYSHUTTER    14    // 0111    0    // never opens
#define dNONE           16    // 1000    0    // No 'dungeon door' at all

struct mapscr
{
	byte valid;
	byte guy;
	int32_t guytile = -1;
	byte guycs;
	word roomflags;
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
	byte door[4] = {dNONE,dNONE,dNONE,dNONE}; //need to add a dmapscreendoor command.
	word tilewarpdmap[4];
	byte tilewarpscr[4];
	byte exitdir;
	word enemy[10]; //GetSetScreenEnemies()
	byte pattern;
	byte sidewarptype[4] = { wtSCROLL,wtSCROLL,wtSCROLL,wtSCROLL };
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
	byte csensitive = 1;
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
	byte layeropacity[6] = {255, 255, 255, 255, 255, 255};
	word timedwarptics;
	byte nextmap;
	byte nextscr;
	word secretcombo[128];
	byte secretcset[128];
	byte secretflag[128];
	
	word data[176];
	byte sflag[176];
	byte cset[176];
	
	byte entry_x, entry_y; //Where Hero entered the screen. Used for pits, and to prevent water walking. -Z
	
	ffcdata ffcs[MAXFFCS];
	word numFFC();
	void ffcCountMarkDirty();
	
	byte ffEffectWidth(size_t ind) const;
	void ffEffectWidth(size_t ind, byte val);
	byte ffEffectHeight(size_t ind) const;
	void ffEffectHeight(size_t ind, byte val);
	byte ffTileWidth(size_t ind) const;
	void ffTileWidth(size_t ind, byte val);
	byte ffTileHeight(size_t ind) const;
	void ffTileHeight(size_t ind, byte val);
	
	word script_entry;
	word script_occupancy;
	word script_exit;
	
	byte oceansfx;
	byte bosssfx;
	byte secretsfx = 27;
	byte holdupsfx = 20;
	
	// for importing older quests...
	byte old_cpage;
	int16_t screen_midi = -1;
	byte lens_layer;
	byte lens_show, lens_hide;
	
	//Currently unused
	int32_t npcstrings[10];
	int16_t new_items[10];
	int16_t new_item_x[10];
	int16_t new_item_y[10];
	
	
	word script;
	int32_t screeninitd[8];
	byte preloadscript;
	
	byte hidelayers;
	byte hidescriptlayers;
	
	string usr_notes;
	
	void zero_memory();

private:
	word num_ffcs;
	bool ffc_count_dirty = true;
};

#define RFL_ALWAYS_GUY 0x0001
#define RFL_GUYFIRES   0x0002

// mapscr "valid" byte
#define mVALID          0x01
#define mVERSION        0x80

// screen flags
#define fSHUTTERS           0x01
#define fITEM               0x02
#define fDARK               0x04
#define fROAR               0x08 // DEFUNCT
#define fWHISTLE            0x10
#define fLADDER             0x20
#define fMAZE               0x40
#define fSEA                0x80 // DEFUNCT

// flags2
#define wfUP                0x01 // What's this?
#define wfDOWN              0x02 // What's this?
#define wfLEFT              0x04 // What's this?
#define wfRIGHT             0x08 // What's this?
#define fSECRET             0x10
#define fAIRCOMBOS          0x20
#define fFLOATTRAPS         0x40
#define fCLEARSECRET        0x80

// flags3
#define fHOLDITEM           0x01
#define fCYCLEONINIT        0x02
#define fINVISROOM          0x04
#define fINVISHERO          0x08
#define fNOSUBSCR           0x10
#define fIWARPFULLSCREEN    0x20
#define fNOSUBSCROFFSET     0x40
#define fENEMIESRETURN      0x80

//flags4
#define fOVERHEADTEXT       0x01 // What's this?
#define fITEMWARP           0x02 // What's this?
#define fTIMEDDIRECT        0x04
#define fDISABLETIME        0x08
#define fENEMYSCRTPERM      0x10
#define fNOITEMRESET        0x20 // What's this?
#define fSAVEROOM           0x40
#define fAUTOSAVE           0x80

//flags5
#define fRANDOMTIMEDWARP    0x01
#define fDAMAGEWITHBOOTS    0x02
#define fDIRECTAWARP        0x04
#define fDIRECTSWARP        0x08
#define fTEMPSECRETS        0x10
// what is going on with 0x20?
#define fTOGGLEDIVING       0x40
#define fNOFFCARRYOVER      0x80

//flags6
#define fCAVEROOM           0x01
#define fDUNGEONROOM        0x02
#define fTRIGGERFPERM       0x04
#define fCONTINUEHERE       0x08
#define fNOCONTINUEHERE     0x10
#define fTRIGGERF1631       0x20
#define fTOGGLERINGDAMAGE   0x40
#define fWRAPAROUNDFF       0x80

//flags7
#define fLAYER3BG           0x01
#define fLAYER2BG           0x02
#define fITEMFALLS          0x04
#define fSIDEVIEW           0x08
#define fNOHEROMARK         0x10
#define fSPECITEMMARK       0x20 // What's this?
#define fWHISTLEPAL         0x40
#define fWHISTLEWATER       0x80

//flags8
// 1 - 16 are General Purpose (Scripts)
#define fGENERALSCRIPT1     0x01
#define fGENERALSCRIPT2     0x02
#define fGENERALSCRIPT3     0x04
#define fGENERALSCRIPT4     0x08
#define fGENERALSCRIPT5     0x10
#define fMAZEvSIDEWARP      0x20
#define fSECRETITEM         0x40
#define fITEMSECRET         0x80

//flags9
#define fITEMSECRETPERM     0x01
#define fITEMRETURN         0x02
#define fBELOWRETURN        0x04
#define fDARK_DITHER        0x08
#define fDARK_TRANS         0x10
#define fDISABLE_MIRROR     0x20
#define fENEMY_WAVES        0x40

//flags10 - ENTIRE FLAGS10 RESERVED FOR Z3 SCROLLING! Please don't use :)
#define fZ3_SCROLLING_WHEN  0x01
// ----

// enemy flags
#define efZORA          1
#define efTRAP4         2
#define efTRAP2         4
#define efROCKS         8
#define efFIREBALLS     16
#define efLEADER        32
#define efCARRYITEM     64
#define efBOSS          128

//lens layer effects
#define llNORMAL        0
#define llLENSHIDES     8
#define llLENSSHOWS     16

//States
#define mDOOR_UP         0x0001 // only dungeons use this
#define mDOOR_DOWN       0x0002 //^
#define mDOOR_LEFT       0x0004 //^
#define mDOOR_RIGHT      0x0008 //^

#define mITEM            0x0010 // item (main screen)
#define mSPECIALITEM     0x0020 // special item (underground)
#define mNEVERRET        0x0040 // enemy never returns
#define mTMPNORET        0x0080 // enemies don't return until you leave the dungeon

#define mLOCKBLOCK       0x0100 // if the lockblock on the screen has been triggered
#define mBOSSLOCKBLOCK   0x0200 // if the bosslockblock on the screen has been triggered
#define mCHEST           0x0400 // if the unlocked check on this screen has been opened
#define mLOCKEDCHEST     0x0800 // if the locked chest on this screen has been opened

#define mBOSSCHEST       0x1000 // if the boss chest on this screen has been opened
#define mSECRET          0x2000 // only overworld and caves use this
#define mVISITED         0x4000 // only overworld uses this
#define mLIGHTBEAM       0x8000 // light beam triggers completed

#define mNORESET         0x3F3F // all 'no reset' flags set
#define mNOCARRYOVER     0x3F30 // all 'no carryover' flags set

#define mMAXIND          16

// room types
enum
{
	rNONE, rSP_ITEM, rINFO, rMONEY, rGAMBLE, rREPAIR, rRP_HC, rGRUMBLE,
	rTRIFORCE, rP_SHOP, rSHOP, rBOMBS, rSWINDLE, r10RUPIES, rWARP,
	rGANON, rZELDA, rITEMPOND, rMUPGRADE, rLEARNSLASH, rARROWS, rTAKEONE,
	rBOTTLESHOP,
	rMAX
};

extern std::vector<mapscr> TheMaps;
extern std::vector<word>   map_autolayers;
extern word map_count;

#endif


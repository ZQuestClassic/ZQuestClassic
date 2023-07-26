#ifndef _MAPSCR_H_
#define _MAPSCR_H_

#include "base/zdefs.h"
#include "ffc.h"

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
	byte door[4]; //need to add a dmapscreendoor command.
	word tilewarpdmap[4];
	byte tilewarpscr[4];
	byte exitdir;
	word enemy[10]; //GetSetScreenEnemies()
	byte pattern; // Enemy loading pattern.
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

// screen flags
#define fSHUTTERS           0x01
#define fITEM               0x02
#define fDARK               0x04 //'S.Flags1' Dark Room
#define fROAR               0x08 // DEFUNCT
#define fWHISTLE            0x10 //'
#define fLADDER             0x20 //'S.Flags2' Toggle  'Allow Ladder' --??? Is that right?
#define fMAZE               0x40 //'S.Flags 1' Use Maze path
#define fSEA                0x80 // DEFUNCT

// flags2
#define wfUP                0x01
#define wfDOWN              0x02
#define wfLEFT              0x04
#define wfRIGHT             0x08
#define fSECRET             0x10  // play "secret" sfx upon entering this screen
#define fAIRCOMBOS          0x20  //'S.Flags2' Combos affect midair Hero
#define fFLOATTRAPS         0x40 //'E.Flags' Traps ignore walkability.
#define fCLEARSECRET        0x80 // clear all enemies to trigger secret entrance (Enemies->Secret)

// flags3
#define fHOLDITEM           0x01 //'S.Flags1' Hold Up Item
#define fCYCLEONINIT        0x02 //'S.Flags2' Cycle combos on screen init
#define fINVISROOM          0x04 //
#define fINVISHERO          0x08 //'S.Flags1' Invisible Hero
#define fNOSUBSCR           0x10 //'S.Flags1' No subscreen
#define fIWARPFULLSCREEN    0x20                                // instawarps affect all sprites --is this 'Sprites carry over in warps???' -Z
#define fNOSUBSCROFFSET     0x40  //'S.Flags1' ...but don't offset screen     // don't offset the screen when fNOSUBSCR is true
#define fENEMIESRETURN      0x80 //'E.Flags' Enemies always return                             // enemies always return

//flags4
#define fOVERHEADTEXT       0x01
#define fITEMWARP           0x02
#define fTIMEDDIRECT        0x04    //'T.Warp' Timed Warps are Direct                             //Specifies timed pit warp
#define fDISABLETIME        0x08    //'T.Warp' Secrets Disable timed warp ??
#define fENEMYSCRTPERM      0x10   //'E.Flags' Enemies->Secret is Permanent
#define fNOITEMRESET        0x20
#define fSAVEROOM           0x40  //'S.Flags2' Save point->Continue here
#define fAUTOSAVE           0x80 //'S.Flags2' Save game on entry

//flags5
#define fRANDOMTIMEDWARP    0x01 //'T.Warp' Timed warp is random
#define fDAMAGEWITHBOOTS    0x02
#define fDIRECTAWARP        0x04 //'S.Flags1' Auto warps are direct
#define fDIRECTSWARP        0x08 //'S.Flags1' Sensitive warps are direct
#define fTEMPSECRETS        0x10 //'S.Flags1' Secrets are temporary
// what is going on with 0x20?
#define fTOGGLEDIVING       0x40 //'S.Flags2' Toggle no Diving
#define fNOFFCARRYOVER      0x80 //'S.Flags2' No FFC Carryover

//flags6
#define fCAVEROOM           0x01 //'S.Flags1' Treat as Interior screen.
#define fDUNGEONROOM        0x02 //'S.Flags1' Treat as NES dungeon room
#define fTRIGGERFPERM       0x04 //'S.Flags1' Hit all triggers->Perm Secret
#define fCONTINUEHERE       0x08 //'S.Flags 2' Continue Here
#define fNOCONTINUEHERE     0x10 //'S.Flags2' No continue here after warp
#define fTRIGGERF1631       0x20 //'S.Flags1' Hit all triggers 0x10-31
#define fTOGGLERINGDAMAGE   0x40 //'S.Flags2' Toggle Rings Affect Combos
#define fWRAPAROUNDFF       0x80 //'S.Flags2' FF Combos Wrap Around

//flags7
#define fLAYER3BG           0x01 //'S.Flags1' Layer 3 is background
#define fLAYER2BG           0x02 //'S.Flags1' Layer 2 is background
#define fITEMFALLS          0x04 //'S.Flags1' Item falls from ceiling
#define fSIDEVIEW           0x08 //'S.Flags1' Sideview Gravity
#define fNOHEROMARK         0x10 //'S.Flags1' No Hero marker in Minimap
#define fSPECITEMMARK       0x20
#define fWHISTLEPAL         0x40 //'S.Flags2' Whistle->Palette change.
#define fWHISTLEWATER       0x80 //'S.Flags2' Whistle->Dry lake

//flags8
// 1 - 16 are General Purpose (Scripts)
#define fGENERALSCRIPT1     0x01 //'S.Flags2' General Use 1 (Scripts)
#define fGENERALSCRIPT2     0x02 //'S.Flags2' General Use 2 (Scripts)
#define fGENERALSCRIPT3     0x04 //'S.Flags2' General Use 3 (Scripts)
#define fGENERALSCRIPT4     0x08 //'S.Flags2' General Use 4 (Scripts)
#define fGENERALSCRIPT5     0x10 //'S.Flags2' General Use 5 (Scripts)
#define fMAZEvSIDEWARP      0x20 //'S.Flags1' Maze overrides side warps
#define fSECRETITEM         0x40 //'S.Flags3' Secrets->Item
#define fITEMSECRET         0x80 //'S.Flags3' Item->Secret

//flags9
#define fITEMSECRETPERM     0x01 //'S.Flags3' Item->Secret is Permanent
#define fITEMRETURN         0x02 //'S.Flags3' Item always returns
#define fBELOWRETURN        0x04 //'S.Flags3' Special Item always returns
#define fDARK_DITHER        0x08 //'S.Flags1' ...dithered dark
#define fDARK_TRANS         0x10 //'S.Flags1' ...transparent dark
#define fDISABLE_MIRROR     0x20 //'S.Flags1' Disable Magic Mirror
#define fENEMY_WAVES        0x40 //'E.Flags' Chain 'Enemies->' triggers

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

// room types
enum
{
	rNONE, rSP_ITEM, rINFO, rMONEY, rGAMBLE, rREPAIR, rRP_HC, rGRUMBLE,
	rTRIFORCE, rP_SHOP, rSHOP, rBOMBS, rSWINDLE, r10RUPIES, rWARP,
	rGANON, rZELDA, rITEMPOND, rMUPGRADE, rLEARNSLASH, rARROWS, rTAKEONE,
	rBOTTLESHOP,
	rMAX
};

#endif


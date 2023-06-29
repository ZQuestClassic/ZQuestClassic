#ifndef _MAPSCR_H_
#define _MAPSCR_H_

#include "base/zdefs.h"
#include "ffc.h"

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

#endif

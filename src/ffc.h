#ifndef _FFC_H_
#define _FFC_H_

#include "zfix.h"
#include "solidobject.h"

//x = ffx
//y = ffy
//vx = ffxdelta
//vy = ffydelta
//ax = ffxdelta2
//ay = ffydelta2
//flags  = ffflags
//data = ffdata
//delay = ffdelay
//cset = ffcset
//link = fflink
//script = ffscript
//ffwidth, ffheight?

class ffcdata : public solid_object
{
public:
	zfix ax, ay;
	dword flags;
	word data, delay;
	byte cset, link, txsz,tysz;
	word script;
	int32_t initd[INITIAL_D];
	int32_t inita[INITIAL_A];
	bool initialized;
	
	ffcdata();
	~ffcdata();
	ffcdata(ffcdata const& other);
	virtual void copy(ffcdata const& other);
	void changerCopy(ffcdata& other, int32_t i = -1, int32_t j = -1);
	ffcdata& operator=(ffcdata const& other);
	void clear();
	virtual bool setSolid(bool set);
	virtual void updateSolid();
	void setLoaded(bool set);
	bool getLoaded() const;
	//Overload to do damage to Hero on pushing them
	virtual void doContactDamage(int32_t hdir);
private:
	bool loaded;
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
	byte door[4]; //need to add a dmapscreendoor command.
	word tilewarpdmap[4];
	byte tilewarpscr[4];
	byte exitdir;
	word enemy[10]; //GetSetScreenEnemies()
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
	
	std::vector<word> data;
	std::vector<byte> sflag;
	std::vector<byte> cset;
	word viewX;
	word viewY;
	byte scrWidth;
	byte scrHeight;
	
	byte entry_x, entry_y; //Where Hero entered the screen. Used for pits, and to prevent water walking. -Z
	
	ffcdata ffcs[MAXFFCS];
	
	byte ffEffectWidth(size_t ind) const
	{
		return (byte)ffcs[ind].hxsz;
	}
	void ffEffectWidth(size_t ind, byte val)
	{
		ffcs[ind].hxsz = val;
	}
	byte ffEffectHeight(size_t ind) const
	{
		return (byte)ffcs[ind].hysz;
	}
	void ffEffectHeight(size_t ind, byte val)
	{
		ffcs[ind].hysz = val;
	}
	byte ffTileWidth(size_t ind) const
	{
		return ffcs[ind].txsz;
	}
	void ffTileWidth(size_t ind, byte val)
	{
		ffcs[ind].txsz = val;
	}
	byte ffTileHeight(size_t ind) const
	{
		return ffcs[ind].tysz;
	}
	void ffTileHeight(size_t ind, byte val)
	{
		ffcs[ind].tysz = val;
	}
	
	word script_entry;
	word script_occupancy;
	word script_exit;
	
	byte oceansfx;
	byte bosssfx;
	byte secretsfx;
	byte holdupsfx;
	
	// for importing older quests...
	byte old_cpage;
	int16_t screen_midi;
	byte lens_layer;
	
	//Currently unused
	int32_t npcstrings[10];
	int16_t new_items[10];
	int16_t new_item_x[10];
	int16_t new_item_y[10];
	
	
	word script;
	int32_t screeninitd[8];
	byte screen_waitdraw;
	byte preloadscript;
	uint32_t ffcswaitdraw;
	byte screendatascriptInitialised;
	
	byte hidelayers;
	byte hidescriptlayers;
	byte doscript;
	
	void zero_memory();
	
	mapscr()
	{
		data.resize(176,0);
		sflag.resize(176,0);
		cset.resize(176,0);
		zero_memory();
	}
	void copy(mapscr const& other);
	mapscr(mapscr const& other);
	mapscr& operator=(mapscr const& other);
};

#endif


#ifndef _ZC_TILES_H_
#define _ZC_TILES_H_

#include "base/zc_alleg.h"
#include "base/zdefs.h"
#include "base/combo.h"

//extern byte *tilebuf;
extern tiledata *newtilebuf, *grabtilebuf;
extern int32_t animated_combo_table[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern int32_t animated_combo_table4[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern int32_t animated_combos;
extern int32_t animated_combo_table2[MAXCOMBOS][2];             //[0]=position in act2, [1]=original tile
extern int32_t animated_combo_table24[MAXCOMBOS][2];            //[0]=combo, [1]=clock
extern int32_t animated_combos2;
extern bool blank_tile_table[NEWMAXTILES];                  //keeps track of blank tiles
extern bool used_tile_table[NEWMAXTILES];                   //keeps track of used tiles
extern bool blank_tile_quarters_table[NEWMAXTILES*4];       //keeps track of blank tile quarters

// in tiles.cc
extern byte unpackbuf[256];
extern const char *tileformat_string[tfMax];
extern comboclass   *combo_class_buf;
extern bool zq_allow_tile_draw_cache;

void register_blank_tiles();
void register_blank_tiles(int32_t max);
int32_t count_tiles(tiledata *buf);
word count_combos();
void setup_combo_animations();
void reset_combo_animation(newcombo &cmb);
void reset_combo_animation(int32_t c);
void reset_combo_animations();
void setup_combo_animations2();
void reset_combo_animation2(int32_t c);
void reset_combo_animations2();
bool combocheck(const newcombo& cdata);
void animate(newcombo& cdata, bool forceNextFrame = false);
void animate_combos();
bool isonline(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3);
void reset_tile(tiledata *buf, int32_t t, int32_t format);
//void clear_tile(tiledata *buf, word tile);
void clear_tiles(tiledata *buf);
void overlay_tile(tiledata *buf,int32_t dest,int32_t src,int32_t cs,bool backwards);
bool copy_tile(tiledata *buf, int32_t src, int32_t dest, bool swap);
bool write_tile(tiledata *buf, BITMAP* src, int32_t dest, int32_t x, int32_t y, bool is8bit, bool overlay);
void unpack_tile(tiledata *buf, int32_t tile, int32_t flip, bool force);

void pack_tile(tiledata *buf, byte *src,int32_t tile);
void pack_tiledata(byte *dest, byte *src, byte format);
void load_tile(byte* buf, int tile);
void load_minitile(byte* buf, int tile, int mini);
int32_t rotate_value(int32_t flip);
byte rotate_walk(byte v);
byte rotate_cset(byte v);

void puttile8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void oldputtile8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void overtile8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void puttile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void oldputtile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void overtile16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip);
void overtile16_scale(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int dw, int dh);
void drawtile16_cs2(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t cset[],int32_t flip,bool over);
void overtileblock16(BITMAP* _Dest, int32_t tile, int32_t x, int32_t y, int32_t w, int32_t h, int32_t color, int32_t flip, byte skiprows=0);

extern int combotile_override_x, combotile_override_y;
extern int combotile_add_x, combotile_add_y;
int32_t combo_tile(const newcombo &c, int32_t x, int32_t y);
int32_t combo_tile(int32_t cmbdat, int32_t x, int32_t y);

void putcombo(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset);
void overcombo(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset);
void overcomboblock(BITMAP *dest, int32_t x, int32_t y, int32_t cmbdat, int32_t cset, int32_t w, int32_t h);
void overcombo2(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset);

void puttiletranslucent8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity);
void overtiletranslucent8(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity);
void puttiletranslucent16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity);
void overtiletranslucent16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t opacity);
void overtilecloaked16(BITMAP* dest,int32_t tile,int32_t x,int32_t y,int32_t flip);
void draw_cloaked_sprite(BITMAP* dest,BITMAP* src,int32_t x,int32_t y);

void putblocktranslucent8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask,int32_t opacity);
void overblocktranslucent8(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t csets[],int32_t flip,int32_t mask,int32_t opacity);

void putcombotranslucent(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset,int32_t opacity);
void overcombotranslucent(BITMAP* dest,int32_t x,int32_t y,int32_t cmbdat,int32_t cset,int32_t opacity);
void overcomboblocktranslucent(BITMAP *dest, int32_t x, int32_t y, int32_t cmbdat, int32_t cset, int32_t w, int32_t h, int32_t opacity);

bool is_valid_format(byte format);
int32_t tilesize(byte format);
int32_t comboa_lmasktotal(byte layermask);

struct BaseTileRef
{
	string name;
	uint w, h; //should never be 0
	bool no_move;
	int xoff, yoff;
	vector<std::tuple<int,int,int>> extra_rects;
	int offset() const
	{
		return xoff+yoff*TILES_PER_ROW;
	}
	virtual int32_t getTile() const = 0;
	virtual void addTile(int32_t offs) = 0;
	virtual void setTile(int32_t val) = 0;
	virtual void forEach(std::function<void(int32_t)> proc) const = 0;
	BaseTileRef(string name = "")
		: name(name), w(1), h(1), no_move(false), xoff(0), yoff(0), extra_rects()
	{}
	BaseTileRef(uint w, uint h, string name = "")
		: name(name), w(zc_max(1,w)), h(zc_max(1,h)), no_move(false), xoff(0), yoff(0), extra_rects()
	{}
};
struct TileRefPtr : public BaseTileRef
{
	int32_t* tile;
	int32_t getTile() const override {return *tile;}
	void addTile(int32_t offs) override {*tile += offs;}
	void setTile(int32_t val) override {*tile = val;}
	void forEach(std::function<void(int32_t)> proc) const override;
	TileRefPtr()
		: BaseTileRef(), tile(nullptr)
	{}
	TileRefPtr(int32_t* tile, string name = "")
		: BaseTileRef(name), tile(tile)
	{}
	TileRefPtr(int32_t* tile, uint w, uint h, string name = "")
		: BaseTileRef(w,h,name), tile(tile)
	{}
};
struct TileRefPtr10k : public BaseTileRef
{
	int32_t* tile;
	int32_t getTile() const override {return *tile / 10000;}
	void addTile(int32_t offs) override {*tile += offs * 10000;}
	void setTile(int32_t val) override {*tile = val * 10000;}
	void forEach(std::function<void(int32_t)> proc) const override;
	TileRefPtr10k()
		: BaseTileRef(), tile(nullptr)
	{}
	TileRefPtr10k(int32_t* tile, string name = "")
		: BaseTileRef(name), tile(tile)
	{}
	TileRefPtr10k(int32_t* tile, uint w, uint h, string name = "")
		: BaseTileRef(w,h,name), tile(tile)
	{}
};
struct TileRefCombo : public BaseTileRef
{
	newcombo* combo;
	int32_t getTile() const override {return combo->o_tile;}
	void addTile(int32_t offs) override {combo->set_tile(combo->o_tile + offs);}
	void setTile(int32_t val) override {combo->set_tile(val);}
	void forEach(std::function<void(int32_t)> proc) const override;
	TileRefCombo()
		: BaseTileRef(), combo(nullptr)
	{}
	TileRefCombo(newcombo* combo, string name = "")
		: BaseTileRef(name), combo(combo)
	{}
	TileRefCombo(newcombo* combo, uint w, uint h, string name = "")
		: BaseTileRef(w,h,name), combo(combo)
	{}
};
/* TODO: SCCs?
struct TileRefSCC : public BaseTileRef
{
	int32_t getTile() const override {assert(false); return 0;}
	void addTile(int32_t offs) override {assert(false);}
	void setTile(int32_t val) override {assert(false);}
	void forEach(std::function<void(int32_t)> proc) const override {}
	TileRefSCC()
		: BaseTileRef()
	{}
	TileRefSCC(string name = "")
		: BaseTileRef(name)
	{}
	TileRefSCC(uint w, uint h, string name = "")
		: BaseTileRef(w,h,name)
	{}
};*/

struct TileMoveList
{
	vector<std::unique_ptr<BaseTileRef>> move_refs;
	bitstring_long move_bits;
	string msg;
	
	TileMoveList() = default;
	TileMoveList(string msg) : msg(msg) {}
	
	template<class... Args>
	TileRefPtr* add_tile(int32_t* tile, Args&&... args)
	{
		if(!tile || !*tile)
			return nullptr;
		return (TileRefPtr*)move_refs.emplace_back(std::make_unique<TileRefPtr>(tile, args...)).get();
	}
	template<class... Args>
	TileRefPtr10k* add_tile_10k(int32_t* tile, Args&&... args)
	{
		if(!tile || !*tile)
			return nullptr;
		return (TileRefPtr10k*)move_refs.emplace_back(std::make_unique<TileRefPtr10k>(tile, args...)).get();
	}
	template<class... Args>
	TileRefCombo* add_combo(newcombo* combo, Args&&... args)
	{
		if(!combo || !combo->o_tile)
			return nullptr;
		return (TileRefCombo*)move_refs.emplace_back(std::make_unique<TileRefCombo>(combo, args...)).get();
	}
	
	
	#ifdef IS_EDITOR
	bool process(bool rect, bool is_dest, int _l, int _t, int _w, int _h, int _first, int _last);
	void add_diff(int diff);
	#endif
};

#endif                                                      // _ZC_TILES_H_
 

//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_tiles.cc
//
//  Tile editor stuff, etc., for ZQuest.
//
//--------------------------------------------------------

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <string.h>
#include <cmath>
#include <fmt/format.h>

#include "base/gui.h"
#include "zquestdat.h"
#include "zq_tiles.h"
#include "zquest.h"
#include "tiles.h"
#include "zq_misc.h"
#include "zq_class.h"
#include "base/zsys.h"
#include "base/colors.h"
#include "qst.h"
#include "jwin_a5.h"
#include "base/jwinfsel.h"
#include "zc_custom.h"
#include "questReport.h"
#include "dialog/info.h"
#include "dialog/scaletile.h"
#include "dialog/alert.h"
#include "drawing.h"
#include "colorname.h"
#include "zq/render.h"

extern zcmodule moduledata;

#ifdef _MSC_VER
#define stricmp _stricmp
#endif

#define HIDE_USED (show_only_unused_tiles&1)
#define HIDE_UNUSED (show_only_unused_tiles&2)
#define HIDE_BLANK (show_only_unused_tiles&4)
#define HIDE_8BIT_MARKER (show_only_unused_tiles&8)

extern void large_dialog(DIALOG *d);
static void massRecolorReset4Bit();
static void massRecolorReset8Bit();
static bool massRecolorSetup(int32_t cset);
static void massRecolorApply(int32_t tile);
extern int32_t last_droplist_sel;

int32_t ex=0;
int32_t nextcombo_fake_click=0;
int32_t tthighlight = 1;
int32_t showcolortip = 1;

tiledata     *newundotilebuf;
newcombo     *undocombobuf;

BITMAP *selection_pattern;
byte selection_grid[18][18];
byte selection_anchor=0;

enum {selection_mode_normal, selection_mode_add, selection_mode_subtract, selection_mode_exclude};
BITMAP *selecting_pattern;
int32_t selecting_x1, selecting_x2, selecting_y1, selecting_y2;

extern int32_t bidcomboscripts_cnt;
extern script_struct bidcomboscripts[NUMSCRIPTSCOMBODATA]; 

BITMAP *intersection_pattern;

byte relational_template[48][4]=
{
	{  0,  0,  0,  0 },
	{  0,  1,  0,  0 },
	{  1,  0,  0,  0 },
	{  1,  1,  0,  0 },
	{  0,  0,  1,  0 },
	{  0,  1,  1,  0 },
	{  1,  0,  1,  0 },
	{  1,  1,  1,  0 },
	{  0,  0,  0,  1 },
	{  0,  1,  0,  1 },
	{  1,  0,  0,  1 },
	{  1,  1,  0,  1 },
	{  0,  0,  1,  1 },
	{  0,  1,  1,  1 },
	{  1,  0,  1,  1 },
	{  1,  1,  1,  1 },
	{  0,  2,  0,  2 },
	{  1,  2,  0,  2 },
	{  0,  2,  1,  2 },
	{  1,  2,  1,  2 },
	{  3,  3,  0,  0 },
	{  3,  3,  1,  0 },
	{  3,  3,  0,  1 },
	{  3,  3,  1,  1 },
	{  3,  4,  0,  2 },
	{  3,  4,  1,  2 },
	{  2,  0,  2,  0 },
	{  2,  1,  2,  0 },
	{  2,  0,  2,  1 },
	{  2,  1,  2,  1 },
	{  2,  2,  2,  2 },
	{  4,  3,  2,  0 },
	{  4,  3,  2,  1 },
	{  4,  4,  2,  2 },
	{  0,  0,  3,  3 },
	{  0,  1,  3,  3 },
	{  1,  0,  3,  3 },
	{  1,  1,  3,  3 },
	{  0,  2,  3,  4 },
	{  1,  2,  3,  4 },
	{  3,  3,  3,  3 },
	{  3,  4,  3,  4 },
	{  2,  0,  4,  3 },
	{  2,  1,  4,  3 },
	{  2,  2,  4,  4 },
	{  4,  3,  4,  3 },
	{  4,  4,  4,  4 },
	{  5,  5,  5,  5 }
};

byte dungeon_carving_template[96][4]=
{
	{  0,  0,  0,  0 },
	{  0,  1,  0,  0 },
	{  1,  0,  0,  0 },
	{  1,  1,  0,  0 },
	{  0,  0,  1,  0 },
	{  0,  1,  1,  0 },
	{  1,  0,  1,  0 },
	{  1,  1,  1,  0 },
	{  0,  0,  0,  1 },
	{  0,  1,  0,  1 },
	{  1,  0,  0,  1 },
	{  1,  1,  0,  1 },
	{  0,  0,  1,  1 },
	{  0,  1,  1,  1 },
	{  1,  0,  1,  1 },
	{  1,  1,  1,  1 },
	{  0,  2,  0,  2 },
	{  1,  2,  0,  2 },
	{  0,  2,  1,  2 },
	{  1,  2,  1,  2 },
	{  3,  3,  0,  0 },
	{  3,  3,  1,  0 },
	{  3,  3,  0,  1 },
	{  3,  3,  1,  1 },
	{  3,  4,  0,  2 },
	{  3,  4,  1,  2 },
	{  2,  0,  2,  0 },
	{  2,  1,  2,  0 },
	{  2,  0,  2,  1 },
	{  2,  1,  2,  1 },
	{  2,  2,  2,  2 },
	{  4,  3,  2,  0 },
	{  4,  3,  2,  1 },
	{  4,  4,  2,  2 },
	{  0,  0,  3,  3 },
	{  0,  1,  3,  3 },
	{  1,  0,  3,  3 },
	{  1,  1,  3,  3 },
	{  0,  2,  3,  4 },
	{  1,  2,  3,  4 },
	{  3,  3,  3,  3 },
	{  3,  4,  3,  4 },
	{  2,  0,  4,  3 },
	{  2,  1,  4,  3 },
	{  2,  2,  4,  4 },
	{  4,  3,  4,  3 },
	{  4,  4,  4,  4 },
	{  5,  5,  5,  5 },
	
	{  5,  5,  5,  5 },
	{  6,  6,  6,  6 },
	{  7,  7,  7,  7 },
	{  7,  6,  7,  6 },
	{  8,  8,  8,  8 },
	{ 16,  6,  8, 15 },
	{  7,  7,  8,  8 },
	{  7,  6,  8, 15 },
	{  9,  9,  9,  9 },
	{  6,  6,  9,  9 },
	{  7, 17, 14,  9 },
	{  7,  6, 14,  9 },
	{  8,  9,  8,  9 },
	{ 16,  6,  8,  9 },
	{  7, 17,  8,  9 },
	{  7,  6,  8,  9 },
	{ 10, 10, 10, 10 },
	{  7, 10, 14, 10 },
	{ 16, 10,  8, 10 },
	{  7, 10,  8, 10 },
	{ 11, 11, 11, 11 },
	{ 11, 11,  8, 15 },
	{ 11, 11, 14,  9 },
	{ 11, 11,  8,  9 },
	{ 14, 14, 14, 14 },
	{ 14, 14,  8, 14 },
	{ 12, 12, 12, 12 },
	{ 12,  6, 12, 15 },
	{ 12, 17, 12,  9 },
	{ 12,  6, 12,  9 },
	{ 12, 10, 12, 10 },
	{ 15, 15, 15, 15 },
	{ 15, 15, 15,  9 },
	{ 15, 14, 12, 10 },
	{ 13, 13, 13, 13 },
	{ 16,  6, 13, 13 },
	{  7, 17, 13, 13 },
	{  7,  6, 13, 13 },
	{ 16, 16, 16, 16 },
	{  7, 16, 16, 16 },
	{ 11, 11, 13, 13 },
	{ 11, 14, 13, 16 },
	{ 17, 17, 17, 17 },
	{ 17,  6, 17, 17 },
	{ 12, 10, 17, 16 },
	{ 15, 11, 17, 13 },
	{ 15, 14, 17, 16 },
	{ 18, 18, 18, 18 }
};

struct tile_move_data
{
	int32_t copies;
	int32_t dest_first;
	int32_t dest_last;
	int32_t src_first;
	int32_t src_last;
	int32_t dest_top;
	int32_t dest_bottom;
	int32_t src_top;
	int32_t src_bottom;
	int32_t src_left, src_right;
	int32_t src_width, src_height;
	int32_t dest_left, dest_right;
	int32_t dest_width, dest_height;
	int32_t rows, cols;
	bool rect, move;
	
	tile_move_data()
	{
		copies = dest_first = dest_last = src_first = src_last = dest_top =
			dest_bottom = src_top = src_bottom = src_left = src_right =
			src_width = src_height = dest_left = dest_right = dest_width =
			dest_height = rows = cols = 0;
		rect = move = false;
	}
	
	tile_move_data(tile_move_data const& other)
	{
		copy(other);
	}
	
	tile_move_data& operator=(tile_move_data const& other)
	{
		copy(other);
		return *this;
	}

	void copy(tile_move_data const& other)
	{
		copies = other.copies;
		dest_first = other.dest_first;
		dest_last = other.dest_last;
		src_first = other.src_first;
		src_last = other.src_last;
		dest_top = other.dest_top;
		dest_bottom = other.dest_bottom;
		src_top = other.src_top;
		src_bottom = other.src_bottom;
		src_left = other.src_left;
		src_right = other.src_right;
		src_width = other.src_width;
		src_height = other.src_height;
		dest_left = other.dest_left;
		dest_right = other.dest_right;
		dest_width = other.dest_width;
		dest_height = other.dest_height;
		rows = other.rows;
		cols = other.cols;
		rect = other.rect;
		move = other.move;
	}
	
	void flip()
	{
		zc_swap(src_first, dest_first);
		zc_swap(src_last, dest_last);
		zc_swap(src_top, dest_top);
		zc_swap(src_bottom, dest_bottom);
		zc_swap(src_left, dest_left);
		zc_swap(src_right, dest_right);
		zc_swap(src_width, dest_width);
		zc_swap(src_height, dest_height);
	}
};
bool do_movetile_united(tile_move_data const& tmd);
static tile_move_data* last_tile_move = NULL;


struct combo_move_data
{
	int32_t tile, tile2, copy1, copycnt;
	combo_move_data() : tile(0), tile2(0), copy1(0), copycnt(0){}
	combo_move_data(combo_move_data const& other)
	{
		copy(other);
	}
	combo_move_data& operator=(combo_move_data const& other)
	{
		copy(other);
		return *this;
	}
	void copy(combo_move_data const& other)
	{
		tile = other.tile;
		tile2 = other.tile2;
		copy1 = other.copy1;
		copycnt = other.copycnt;
	}
	
	void flip()
	{
		int32_t tcnt = tile2-tile+1;
		int32_t cpy2 = copy1+copycnt-1;
		zc_swap(tile,copy1);
		tile2 = cpy2;
		copycnt = tcnt;
	}
};
void do_movecombo(combo_move_data const& cmd);
static combo_move_data* last_combo_move = NULL;

/*********************************/
/*****    Tiles & Combos    ******/
/*********************************/

void merge_tiles(int32_t dest_tile, int32_t src_quarter1, int32_t src_quarter2, int32_t src_quarter3, int32_t src_quarter4)
{
	int32_t size=tilesize(newtilebuf[dest_tile].format)>>4;
	int32_t size2=size>>1;
	
	if(newtilebuf[dest_tile].data==NULL)
	{
		reset_tile(newtilebuf, dest_tile, newtilebuf[src_quarter1>>2].format);
	}
	
	int32_t i=0;
	
	if((dest_tile<<2)+i!=src_quarter1)
	{
		for(int32_t j=0; j<8; ++j)
		{
			memcpy(&(newtilebuf[dest_tile].data[((j+((i&2)<<2))*size)+((i&1)*size2)]), &(newtilebuf[src_quarter1>>2].data[((j+((src_quarter1&2)<<2))*size)+((src_quarter1&1)*size2)]), size2);
		}
	}
	
	i=1;
	
	if((dest_tile<<2)+i!=src_quarter2)
	{
		for(int32_t j=0; j<8; ++j)
		{
			memcpy(&(newtilebuf[dest_tile].data[((j+((i&2)<<2))*size)+((i&1)*size2)]), &(newtilebuf[src_quarter2>>2].data[((j+((src_quarter2&2)<<2))*size)+((src_quarter2&1)*size2)]), size2);
		}
	}
	
	i=2;
	
	if((dest_tile<<2)+i!=src_quarter3)
	{
		for(int32_t j=0; j<8; ++j)
		{
			memcpy(&(newtilebuf[dest_tile].data[((j+((i&2)<<2))*size)+((i&1)*size2)]), &(newtilebuf[src_quarter3>>2].data[((j+((src_quarter3&2)<<2))*size)+((src_quarter3&1)*size2)]), size2);
		}
	}
	
	i=3;
	
	if((dest_tile<<2)+i!=src_quarter4)
	{
		for(int32_t j=0; j<8; ++j)
		{
			memcpy(&(newtilebuf[dest_tile].data[((j+((i&2)<<2))*size)+((i&1)*size2)]), &(newtilebuf[src_quarter4>>2].data[((j+((src_quarter4&2)<<2))*size)+((src_quarter4&1)*size2)]), size2);
		}
	}
}

static void make_combos(int32_t startTile, int32_t endTile, int32_t cs)
{
	 al_trace("inside make_combos()\n");
	int32_t startCombo=0;
	
	if(!select_combo_2(startCombo,cs))
		return;
	
	int32_t temp=combobuf[startCombo].o_tile;
	combobuf[startCombo].set_tile(startTile);
	
	if(!edit_combo(startCombo, false, cs))
	{
		combobuf[startCombo].set_tile(temp);
		return;
	}
	
	go_combos();
	
	for(int32_t i=0; i<=endTile-startTile; i++)
	{
		combobuf[startCombo+i]=combobuf[startCombo];
		combobuf[startCombo+i].set_tile(startTile+i);
	}
	
	setup_combo_animations();
	setup_combo_animations2();
}

static void make_combos_rect(int32_t top, int32_t left, int32_t numRows, int32_t numCols, int32_t cs)
{
	//al_trace("inside make_combos_rect()\n");
	int32_t startCombo=0;
	
	if(!select_combo_2(startCombo, cs))
		return;
	
	int32_t startTile=top*TILES_PER_ROW+left;
	int32_t temp=combobuf[startCombo].o_tile;
	combobuf[startCombo].set_tile(startTile);
	
	if(!edit_combo(startCombo, false, cs))
	{
		al_trace("make_combos_rect() early return\n");
		combobuf[startCombo].set_tile(temp);
		return;
	}
	
	bool smartWrap=false;
	if(numCols!=4 && numRows>1)
	{
		char buf[64];
		if(numCols<4)
			sprintf(buf, "Limit to %d column%s?", numCols, numCols==1 ? "" : "s");
		else
			sprintf(buf, "Fit to 4 columns?"); // Meh, whatever.
		int32_t ret=jwin_alert("Wrapping", buf, NULL, NULL, "&Yes", "&No", 'y', 'n', lfont);
		if(ret==1)
			smartWrap=true;
	}
		
	go_combos();
	
	int32_t combo=startCombo-1;
	for(int32_t row=0; row<numRows; row++)
	{
		for(int32_t col=0; col<numCols; col++)
		{
			int32_t tile=startTile+row*TILES_PER_ROW+col;
			if(smartWrap)
				// Add 4 per row, and another numRows*4 for every 4 columns
				// (col&0xFC==col/4*4), and then the column %4
				combo=startCombo+4*row+(col&0xFC)*numRows+col%4;
			else
				combo++;
			
			combobuf[combo]=combobuf[startCombo];
			combobuf[combo].set_tile(tile);
		}
	}
	
	setup_combo_animations();
	setup_combo_animations2();
}

int32_t d_combo_proc(int32_t msg,DIALOG *d,int32_t c);

static bool nogotiles = false;
static bool nogocombos = false;

void go_tiles()
{
	if(nogotiles) return;
	if(last_tile_move)
	{
		delete last_tile_move;
		last_tile_move = NULL;
	}
	for(int32_t i=0; i<NEWMAXTILES; ++i)
	{
		newundotilebuf[i].format=newtilebuf[i].format;
		
		if(newundotilebuf[i].data!=NULL)
		{
			free(newundotilebuf[i].data);
		}
		
		newundotilebuf[i].data=(byte *)malloc(tilesize(newundotilebuf[i].format));
		
		if(newundotilebuf[i].data==NULL)
		{
			Z_error_fatal("Unable to initialize undo tile #%ld.\n", i);
			exit(1);
		}
		
		memcpy(newundotilebuf[i].data,newtilebuf[i].data,tilesize(newundotilebuf[i].format));
	}
	
	/*
	  int32_t *si = (int32_t*)tilebuf;
	  int32_t *di = (int32_t*)undotilebuf;
	  for(int32_t i=0; i<NEWTILE_SIZE2/4; i++)
	  *(di++) = *(si++);
	  */
}

void go_slide_tiles(int32_t columns, int32_t rows, int32_t top, int32_t left)
{
	for(int32_t c=0; c<columns; c++)
	{
		for(int32_t r=0; r<rows; r++)
		{
			int32_t t=((top+r)*TILES_PER_ROW)+left+c;
			newundotilebuf[t].format=newtilebuf[t].format;
			
			if(newundotilebuf[t].data!=NULL)
			{
				free(newundotilebuf[t].data);
			}
			
			newundotilebuf[t].data=(byte *)malloc(tilesize(newundotilebuf[t].format));
			
			if(newundotilebuf[t].data==NULL)
			{
				Z_error_fatal("Unable to initialize undo tile #%ld.\n", t);
				exit(1);
			}
			
			memcpy(newundotilebuf[t].data,newtilebuf[t].data,tilesize(newundotilebuf[t].format));
		}
	}
}

void comeback_tiles()
{
	if(last_tile_move && last_tile_move->move)
	{
		last_tile_move->flip();
		bool t = nogotiles;
		nogotiles = true;
		do_movetile_united(*last_tile_move);
		nogotiles = t;
		delete last_tile_move;
		last_tile_move = NULL;
	}
	for(dword i=0; i<NEWMAXTILES; ++i)
	{
		newtilebuf[i].format=newundotilebuf[i].format;
		
		if(newtilebuf[i].data!=NULL)
		{
			free(newtilebuf[i].data);
		}
		
		newtilebuf[i].data=(byte *)malloc(tilesize(newtilebuf[i].format));
		
		if(newtilebuf[i].data==NULL)
		{
			Z_error_fatal("Unable to initialize tile #%ld.\n", i);
			exit(1);
		}
		
		memcpy(newtilebuf[i].data,newundotilebuf[i].data,tilesize(newtilebuf[i].format));
	}
	
	/*
	  int32_t *si = (int32_t*)undotilebuf;
	  int32_t *di = (int32_t*)tilebuf;
	  for(int32_t i=0; i<NEWTILE_SIZE2/4; i++)
	  *(di++) = *(si++);
	  */
	register_blank_tiles();
	register_used_tiles();
}

void go_combos()
{
	if(nogocombos) return;
	if(last_combo_move)
	{
		delete last_combo_move;
		last_combo_move = NULL;
	}
	newcombo *si = combobuf;
	newcombo *di = undocombobuf;
	
	for(int32_t i=0; i<MAXCOMBOS; i++)
		*(di++) = *(si++);
}

void comeback_combos()
{
	if(last_combo_move)
	{
		last_combo_move->flip();
		bool t = nogocombos;
		nogocombos = true;
		do_movecombo(*last_combo_move);
		nogocombos = t;
		delete last_combo_move;
		last_combo_move = NULL;
	}
	newcombo *si = undocombobuf;
	newcombo *di = combobuf;
	
	for(int32_t i=0; i<MAXCOMBOS; i++)
		*(di++) = *(si++);
}


void little_x(BITMAP *dest, int32_t x, int32_t y, int32_t c, int32_t s)
{
	line(dest,x,y,x+s,y+s,c);
	line(dest,x+s,y,x,y+s,c);
}
void little_x(BITMAP *dest, int32_t x, int32_t y, int32_t c, int32_t w, int32_t h)
{
	line(dest,x,y,x+w,y+h,c);
	line(dest,x+w,y,x,y+h,c);
}

enum {gm_light, gm_dark, gm_max};
int32_t gridmode=gm_light;

bool has_selection()
{
	for(int32_t i=1; i<17; ++i)
	{
		for(int32_t j=1; j<17; ++j)
		{
			if(selection_grid[i][j])
			{
				return true;
			}
		}
	}
	
	return false;
}

void draw_selection_outline(BITMAP *dest, int32_t x, int32_t y, int32_t scale2)
{
	drawing_mode(DRAW_MODE_COPY_PATTERN, selection_pattern, selection_anchor>>3, 0);
	
	for(int32_t i=1; i<18; ++i)
	{
		for(int32_t j=1; j<18; ++j)
		{
			//  zoomtile16(screen2,tile,79,31,cs,flip,8);
			if(selection_grid[i-1][j]!=selection_grid[i][j])
			{
				_allegro_vline(dest, x+((i-1)*scale2), y+((j-1)*scale2), y+(j*scale2), 255);
			}
			
			if(selection_grid[i][j-1]!=selection_grid[i][j])
			{
				_allegro_hline(dest, x+((i-1)*scale2), y+((j-1)*scale2), x+(i*scale2), 255);
			}
		}
	}
	
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	//  selection_anchor=(selection_anchor+1)%64;
}
void draw_selection_outline_a5(int32_t x, int32_t y, int32_t scale2)
{
	for(int32_t i=1; i<18; ++i)
	{
		for(int32_t j=1; j<18; ++j)
		{
			if(selection_grid[i-1][j]!=selection_grid[i][j])
			{
				int x1 = x+((i-1)*scale2);
				int y1 = y+((j-1)*scale2);
				int y2 = y+(j*scale2)+1;
				al_draw_vline(x1, y1, y2, AL5_BLACK);
				al_draw_vline(x1+1, y1, y2, AL5_WHITE);
				al_draw_vline(x1-1, y1, y2, AL5_WHITE);
			}
			if(selection_grid[i][j-1]!=selection_grid[i][j])
			{
				int x1 = x+((i-1)*scale2);
				int y1 = y+((j-1)*scale2);
				int x2 = x+(i*scale2);
				al_draw_vline(x1, y1, x2, AL5_BLACK);
				al_draw_vline(x1, y1+1, x2, AL5_WHITE);
				al_draw_vline(x1, y1-1, x2, AL5_WHITE);
			}
		}
	}
}

bool is_selecting()
{
	return (selecting_x1>-1&&selecting_x2>-1&&selecting_y1>-1&&selecting_y2>-1);
}

void draw_selecting_outline(BITMAP *dest, int32_t x, int32_t y, int32_t scale2)
{
	int32_t x1=zc_min(selecting_x1,selecting_x2);
	int32_t x2=zc_max(selecting_x1,selecting_x2);
	int32_t y1=zc_min(selecting_y1,selecting_y2);
	int32_t y2=zc_max(selecting_y1,selecting_y2);
	
//  rect(dest, x+(x1*scale2), y+(y1*scale2), x+((x2+1)*scale2), y+((y2+1)*scale2), 255);
	for(int32_t i=1; i<18; ++i)
	{
		for(int32_t j=1; j<18; ++j)
		{
			drawing_mode(DRAW_MODE_COPY_PATTERN, selecting_pattern, selection_anchor>>3, 0);
			
			if(((j>=y1+1)&&(j<=y2+1))&&((i==x1+1)||(i==x2+2)))
			{
				if(selection_grid[i-1][j]!=selection_grid[i][j])
				{
					drawing_mode(DRAW_MODE_COPY_PATTERN, intersection_pattern, selection_anchor>>3, 0);
				}
				
				_allegro_vline(dest, x+((i-1)*scale2), y+((j-1)*scale2), y+(j*scale2), 255);
			}
			
			if(((i>=x1+1)&&(i<=x2+1))&&((j==y1+1)||(j==y2+2)))
			{
				if(selection_grid[i][j-1]!=selection_grid[i][j])
				{
					drawing_mode(DRAW_MODE_COPY_PATTERN, intersection_pattern, selection_anchor>>3, 0);
				}
				
				_allegro_hline(dest, x+((i-1)*scale2), y+((j-1)*scale2), x+(i*scale2), 255);
			}
		}
	}
	
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	//  selection_anchor=(selection_anchor+1)%64;
}
void draw_selecting_outline_a5(int32_t x, int32_t y, int32_t scale2)
{
	int32_t x1=zc_min(selecting_x1,selecting_x2);
	int32_t x2=zc_max(selecting_x1,selecting_x2);
	int32_t y1=zc_min(selecting_y1,selecting_y2);
	int32_t y2=zc_max(selecting_y1,selecting_y2);
	for(int32_t i=1; i<18; ++i)
	{
		for(int32_t j=1; j<18; ++j)
		{
			if(((j>=y1+1)&&(j<=y2+1))&&((i==x1+1)||(i==x2+2)))
			{
				int x1 = x+((i-1)*scale2);
				int y1 = y+((j-1)*scale2);
				int y2 = y+(j*scale2)+1;
				al_draw_vline(x1, y1, y2, AL5_BLACK);
				al_draw_vline(x1+1, y1, y2, AL5_WHITE);
				al_draw_vline(x1-1, y1, y2, AL5_WHITE);
			}
			
			if(((i>=x1+1)&&(i<=x2+1))&&((j==y1+1)||(j==y2+2)))
			{
				int x1 = x+((i-1)*scale2);
				int y1 = y+((j-1)*scale2);
				int x2 = x+(i*scale2);
				al_draw_vline(x1, y1, x2, AL5_BLACK);
				al_draw_vline(x1, y1+1, x2, AL5_WHITE);
				al_draw_vline(x1, y1-1, x2, AL5_WHITE);
			}
		}
	}
}

void unfloat_selection();
bool floating_sel = false;
byte floatsel[256];
byte undofloatsel[256];
bool undo_is_floatsel = false;


void add_color_to_selection(int32_t color)
{
	unfloat_selection();
	for(int32_t i=1; i<17; ++i)
	{
		for(int32_t j=1; j<17; ++j)
		{
			if(unpackbuf[((j-1)<<4)+(i-1)]==color)
			{
				selection_grid[i][j]=1;
			}
		}
	}
}

void remove_color_from_selection(int32_t color)
{
	unfloat_selection();
	for(int32_t i=1; i<17; ++i)
	{
		for(int32_t j=1; j<17; ++j)
		{
			if(unpackbuf[((j-1)<<4)+(i-1)]==color)
			{
				selection_grid[i][j]=0;
			}
		}
	}
}

void intersect_color_with_selection(int32_t color)
{
	unfloat_selection();
	for(int32_t i=1; i<17; ++i)
	{
		for(int32_t j=1; j<17; ++j)
		{
			if((unpackbuf[((j-1)<<4)+(i-1)]==color)&&(selection_grid[i][j]==1))
			{
				selection_grid[i][j]=1;
			}
			else
			{
				selection_grid[i][j]=0;
			}
		}
	}
}

bool is_in_selection(int32_t x, int32_t y)
{
	x %= 16; y %= 16;
	if(x < 0) x = (16 - abs(x));
	if(y < 0) y = (16 - abs(y));
	return (!has_selection()||(selection_grid[x+1][y+1]!=0));
}

void zoomtile16(int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t m)
{
	al_draw_filled_rectangle(x,y,x+(16*m)+1,y+(16*m)+1,gridmode==gm_light?AL5_LGRAY:AL5_DGRAY);
	cset <<= 4;
	
	if(newtilebuf[tile].format>tf4Bit)
		cset=0;
	
	unpack_tile(newtilebuf, tile, 0, false);
	byte *si = unpackbuf;
	
	for(int32_t cy=0; cy<16; cy++)
	{
		for(int32_t cx=0; cx<16; cx++)
		{
			int32_t dx = ((flip&1)?15-cx:cx)*m;
			int32_t dy = ((flip&2)?15-cy:cy)*m;
			byte col = (floating_sel && floatsel[cx+(cy<<4)]) ? floatsel[cx+(cy<<4)] : *si;
			ALLEGRO_COLOR a5c = a5color(col+cset);
			al_draw_filled_rectangle(x+dx+1,y+dy+1,x+dx+m,y+dy+m,a5c);
			
			if(col==0)
			{
				int tx = x+dx+m/4, ty = y+dy+m/4;
				al_draw_x(tx,ty,tx+m/2,ty+m/2,getHighlightColor(a5c),1);
			}
			++si;
		}
	}
	
	if(has_selection()||is_selecting())
	{
		selection_anchor=(selection_anchor+1)%64;
		
		if(has_selection()||is_selecting())
		{
			draw_selection_outline_a5(x, y, m);
		}
		
		if(is_selecting())
		{
			draw_selecting_outline_a5(x, y, m);
		}
	}
}

void draw_text_button(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t bg,int32_t fg,int32_t flags,bool jwin)
{
	if(!jwin)
	{
		if(flags&D_SELECTED)
		{
			zc_swap(fg,bg);
		}
		
		rect(dest,x+1,y+1,x+w-1,y+h-1,fg);
		rectfill(dest,x+1,y+1,x+w-3,y+h-3,bg);
		rect(dest,x,y,x+w-2,y+h-2,fg);
		textout_centre_ex(dest,font,text,(x+x+w)>>1,((y+y+h)>>1)-4,fg,-1);
	}
	else
	{
		jwin_draw_text_button(dest, x, y, w, h, text, flags, true);
	}
}

void draw_layer_button(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t flags)
{
	if(flags&D_SELECTED)
	{
		rect(dest, x, y, x+w-1, y+h-1, jwin_pal[jcDARK]);
		++x;
		++y;
		--w;
		--h;
	}
	rectfill(dest,x+1,y+1,x+w-3,y+h-3,jwin_pal[(flags&D_SELECTED ? jcMEDDARK : jcBOX)]);
	jwin_draw_frame(dest, x, y, w, h, (flags&D_SELECTED ? FR_DARK : FR_BOX));
	
	//Forcibly fit the text within the button
	char buf[512] = {0};
	strcpy(buf, text);
	
	bool dis = (flags&D_DISABLED);
	auto hei = text_height(font);
	auto len = text_length(font,buf);
	auto borderwid = 8;
	if(len > w - borderwid + (dis ? 1 : 0))
	{
		auto ind = strlen(buf) - 1;
		auto dotcount = 0;
		while(len > w - borderwid + (dis ? 1 : 0))
		{
			if(dotcount >= 2)
				buf[ind+2] = 0;
			else ++dotcount;
			buf[ind--] = '.';
			len = text_length(font,buf);
		}
	}
	if(dis)
	{
		++len; ++hei;
	}
	BITMAP* tmp = create_bitmap_ex(8,len,hei);
	clear_bitmap(tmp);
	if(dis)
	{
		textout_ex(tmp,font,buf,1,1,jwin_pal[jcLIGHT],-1);
		textout_ex(tmp,font,buf,0,0,jwin_pal[jcDISABLED_FG],-1);
	}
	else
		textout_ex(tmp,font,buf,0,0,jwin_pal[jcBOXFG],-1);
	auto tx = x+((w-len)/2);
	auto ty = y+((h-hei)/2);
	if(len > w-borderwid)
	{
		tx = x+borderwid/2;
		len = w-borderwid;
	}
	if(hei > h-borderwid)
	{
		ty = y+borderwid/2;
		hei = h-borderwid;
	}
	masked_blit(tmp,dest, 0,0, tx,ty, len, hei);
	destroy_bitmap(tmp);
}

bool do_layer_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,const char *text, int32_t flags, bool toggleflag)
{
	bool over=false;
	
	while(gui_mouse_b())
	{
		//vsync();
		if(mouse_in_rect(x,y,w,h))
		{
			if(!over)
			{
				vsync();
				scare_mouse();
				draw_layer_button(screen, x, y, w, h, text, flags^D_SELECTED);
				unscare_mouse();
				over=true;
				
				update_hw_screen();
			}
		}
		else
		{
			if(over)
			{
				vsync();
				scare_mouse();
				draw_layer_button(screen, x, y, w, h, text, flags);
				unscare_mouse();
				over=false;
				
				update_hw_screen();
			}
		}
		rest(1);
	}
	
	if(over)
	{
		vsync();
		scare_mouse();
		draw_layer_button(screen, x, y, w, h, text, toggleflag ? flags^D_SELECTED : flags);
		unscare_mouse();
		
		update_hw_screen();
	}
	
	return over;
}

bool do_text_button(int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t bg,int32_t fg,bool jwin)
{
	bool over=false;
	
	while(gui_mouse_b())
	{
		custom_vsync();
		
		if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+w-1,y+h-1))
		{
			if(!over)
			{
				scare_mouse();
				draw_text_button(screen,x,y,w,h,text,fg,bg,D_SELECTED,jwin);
				unscare_mouse();
				over=true;
			}
		}
		else
		{
			if(over)
			{
				scare_mouse();
				draw_text_button(screen,x,y,w,h,text,fg,bg,0,jwin);
				unscare_mouse();
				over=false;
			}
		}
	}
	
	return over;
}

bool do_text_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t bg,int32_t fg,bool jwin, bool sel)
{
	bool over=false;
	
	while(gui_mouse_b())
	{
		custom_vsync();
		
		if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+w-1,y+h-1))
		{
			if(!over)
			{
				scare_mouse();
				draw_text_button(screen,x,y,w,h,text,fg,bg,sel?0:D_SELECTED,jwin);
				unscare_mouse();
				over=true;
			}
		}
		else
		{
			if(over)
			{
				scare_mouse();
				draw_text_button(screen,x,y,w,h,text,fg,bg,sel?D_SELECTED:0,jwin);
				unscare_mouse();
				over=false;
			}
		}
	}
	
	if(over)
	{
		custom_vsync();
		scare_mouse();
		draw_text_button(screen,x,y,w,h,text,fg,bg,sel?0:D_SELECTED,jwin);
		unscare_mouse();
	}
	
	return over;
}

void draw_graphics_button(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,BITMAP *bmp,BITMAP *bmp2,int32_t bg,int32_t fg,int32_t flags,bool jwin,bool overlay)
{
	if(!jwin)
	{
		if(flags&D_SELECTED)
		{
			zc_swap(fg,bg);
		}
		
		rect(dest,x+1,y+1,x+w-1,y+h-1,fg);
		rectfill(dest,x+1,y+1,x+w-3,y+h-3,bg);
		rect(dest,x,y,x+w-2,y+h-2,fg);
		int32_t g = (flags & D_SELECTED) ? 1 : 0;
		
		if(overlay)
		{
			masked_blit(bmp, dest, 0, 0, x+w/2+g, y+h/2-bmp->h/2+g, bmp->h, bmp->w);
		}
		else
		{
			blit(bmp, dest, 0, 0, x+w/2+g, y+h/2-bmp->h/2+g, bmp->h, bmp->w);
		}
	}
	else
	{
		jwin_draw_graphics_button(dest, x, y, w, h, bmp, bmp2, flags, false, overlay);
	}
}

bool do_graphics_button(int32_t x,int32_t y,int32_t w,int32_t h,BITMAP *bmp,BITMAP *bmp2,int32_t bg,int32_t fg,bool jwin,bool overlay)
{
	bool over=false;
	
	while(gui_mouse_b())
	{
		custom_vsync();
		
		if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+w-1,y+h-1))
		{
			if(!over)
			{
				scare_mouse();
				draw_graphics_button(screen,x,y,w,h,bmp,bmp2,fg,bg,D_SELECTED,jwin,overlay);
				unscare_mouse();
				over=true;
			}
		}
		else
		{
			if(over)
			{
				scare_mouse();
				draw_graphics_button(screen,x,y,w,h,bmp,bmp2,fg,bg,0,jwin,overlay);
				unscare_mouse();
				over=false;
			}
		}
	}
	
	return over;
}

bool do_graphics_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,BITMAP *bmp,BITMAP *bmp2,int32_t bg,int32_t fg,bool jwin,bool overlay)
{
	bool over=false;
	
	while(gui_mouse_b())
	{
		custom_vsync();
		
		if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+w-1,y+h-1))
		{
			if(!over)
			{
				scare_mouse();
				draw_graphics_button(screen,x,y,w,h,bmp,bmp2,fg,bg,D_SELECTED,jwin,overlay);
				unscare_mouse();
				over=true;
			}
		}
		else
		{
			if(over)
			{
				scare_mouse();
				draw_graphics_button(screen,x,y,w,h,bmp,bmp2,fg,bg,0,jwin,overlay);
				unscare_mouse();
				over=false;
			}
		}
	}
	
	if(over)
	{
		custom_vsync();
		scare_mouse();
		draw_graphics_button(screen,x,y,w,h,bmp,bmp2,fg,bg,0,jwin,overlay);
		unscare_mouse();
	}
	
	return over;
}
//    circle(BITMAP *bmp, int32_t x, int32_t y, int32_t radius, int32_t color);
//    circlefill(BITMAP *bmp, int32_t x, int32_t y, int32_t radius, int32_t color);

void draw_layerradio(BITMAP *dest,int32_t x,int32_t y,int32_t bg,int32_t fg, int32_t value)
{
	//these are here to bypass compiler warnings about unused arguments
	bg=bg;
	fg=fg;
	
	int32_t r, center;
	
	for(int32_t k=0; k<7; k++)
	{
		if((k==0)||(Map.CurrScr()->layermap[k-1]))
		{
			//      circle(dest, x+(k*25)+4, y+4, 4, fg);
			//      circlefill(dest, x+(k*25)+4, y+4, 2, (value==k)?fg:bg);
			//*
			r = 9/2;
			
			center = x+(k*25)+r;
			rectfill(dest, x+(k*25), y, x+(k*25)+9-1, y+9-1, jwin_pal[jcBOX]);
			
			circlefill(dest, center, y+r, r, jwin_pal[jcLIGHT]);
			arc(dest, center, y+r, itofix(32), itofix(160), r, jwin_pal[jcMEDDARK]);
			circlefill(dest, center, y+r, r-1, jwin_pal[jcMEDLT]);
			arc(dest, center, y+r, itofix(32), itofix(160), r-1, jwin_pal[jcDARK]);
			circlefill(dest, center, y+r, r-2, jwin_pal[jcLIGHT]);
			
			if(value==k)
			{
				circlefill(dest, center, y+r, r-3, jwin_pal[jcDARK]);
			}
			
			//*/
		}
	}
}

void do_layerradio(BITMAP *dest,int32_t x,int32_t y,int32_t bg,int32_t fg,int32_t &value)
{
	while(gui_mouse_b())
	{
		custom_vsync();
		
		for(int32_t k=0; k<7; k++)
		{
			if((k==0)||(Map.CurrScr()->layermap[k-1]))
			{
				//if on radio button
				if(isinRect(gui_mouse_x(),gui_mouse_y(),x+(k*25),y,x+(k*25)+8,y+8))
				{
					value=k;
					scare_mouse();
					draw_layerradio(dest,x,y,bg,fg,value);
					refresh(rMENU);
					unscare_mouse();
				}
			}
		}
	}
}

void draw_checkbox(BITMAP *dest,int32_t x,int32_t y,int32_t sz,int32_t bg,int32_t fg, bool value)
{
	draw_checkbox(dest,x,y,sz,sz,bg,fg,value);
}
void draw_checkbox(BITMAP *dest,int32_t x,int32_t y,int32_t wid,int32_t hei,int32_t bg,int32_t fg, bool value)
{
	//these are here to bypass compiler warnings about unused arguments
	bg=bg;
	fg=fg;
	
	//  rect(dest,x, y, x+8, y+8, fg);
	//  line(dest,x+1,y+1,x+7,y+7,value?fg:bg);
	//  line(dest,x+1,y+7,x+7,y+1,value?fg:bg);
	
	jwin_draw_frame(dest, x, y, wid, hei, FR_DEEP);
	rectfill(dest, x+2, y+2, x+wid-3, y+hei-3, jwin_pal[jcTEXTBG]);
	
	if(value)
	{
		line(dest, x+2, y+2, x+wid-3, y+hei-3, jwin_pal[jcTEXTFG]);
		line(dest, x+2, y+hei-3, x+wid-3, y+2, jwin_pal[jcTEXTFG]);
	}
}



bool do_checkbox(BITMAP *dest,int32_t x,int32_t y,int32_t sz,int32_t bg,int32_t fg,int32_t &value)
{
	return do_checkbox(dest,x,y,sz,sz,bg,fg,value);
}
bool do_checkbox(BITMAP *dest,int32_t x,int32_t y,int32_t wid,int32_t hei,int32_t bg,int32_t fg,int32_t &value)
{
	bool over=false;
	
	while(gui_mouse_b())
	{
		custom_vsync();
		
		if(isinRect(gui_mouse_x(),gui_mouse_y(),x,y,x+wid-1,y+hei-1))               //if on checkbox
		{
			if(!over)                                             //if wasn't here before
			{
				scare_mouse();
				value=!value;
				draw_checkbox(dest,x,y,wid,hei,bg,fg,value!=0);
				unscare_mouse();
				over=true;
			}
		}
		else                                                    //if not on checkbox
		{
			if(over)                                              //if was here before
			{
				scare_mouse();
				value=!value;
				draw_checkbox(dest,x,y,wid,hei,bg,fg,value!=0);
				unscare_mouse();
				over=false;
			}
		}
	}
	
	return over;
}

//*************** tile flood fill stuff **************

byte tf_c;
byte tf_u;

void tile_floodfill_rec(int32_t x,int32_t y)
{
	if(is_in_selection(x,y))
	{
		while(x>0 && (unpackbuf[(y<<4)+x-1] == tf_u))
			--x;
			
		while(x<=15 && (unpackbuf[(y<<4)+x] == tf_u))
		{
			if(is_in_selection(x,y))
			{
				unpackbuf[(y<<4)+x] = tf_c;
			}
			
			if(y>0 && (unpackbuf[((y-1)<<4)+x] == tf_u))
				tile_floodfill_rec(x,y-1);
				
			if(y<15 && (unpackbuf[((y+1)<<4)+x] == tf_u))
				tile_floodfill_rec(x,y+1);
				
			++x;
		}
	}
}

void tile_floodfill(int32_t tile,int32_t x,int32_t y,byte c)
{
	if(is_in_selection(x,y))
	{
		if(floating_sel)
		{
			memcpy(unpackbuf, floatsel, 256);
		}
		else unpack_tile(newtilebuf, tile, 0, false);
		tf_c = c;
		tf_u = unpackbuf[(y<<4)+x];
		
		if(tf_u != tf_c)
			tile_floodfill_rec(x,y);
		if(floating_sel)
		{
			memcpy(floatsel, unpackbuf, 256);
		}
		else pack_tile(newtilebuf,unpackbuf,tile);
	}
}

//***************** tile editor  stuff *****************
size_and_pos ok_button(302,562,71,21);
size_and_pos cancel_button(376,562,71,21);
size_and_pos edit_button(550,562,86,21);
size_and_pos hlcbox(742,392,16,16);
size_and_pos hov_prev(742,338,50,50);
size_and_pos cpalette_4(648,416,4,4,64,64);
size_and_pos cpalette_8(648,416,16,14,16,16);
size_and_pos fg_prev(648,316,50,50);
size_and_pos bg_prev(648+30,316+30,50,50);
size_and_pos zoomtile(124,32,16,16,32,32);
size_and_pos prev_til_1(648,31,96,96);
size_and_pos prev_til_2(752,31,96,96);
size_and_pos prev_til_3(648,135,96,96);
size_and_pos prev_til_4(752,135,96,96);
size_and_pos status_info(648,308-(4*8),1,4,1,8);
size_and_pos hover_info(742,338-(3*8),1,3,1,8);
size_and_pos color_info(4,189,1,1,116,8);
size_and_pos color_info_btn(24,189,96,21);
size_and_pos tool_btns(22,29,2,4,39,39);
size_and_pos x_btn(890,5,15,13);
size_and_pos info_btn(872,5,15,13);

int32_t c1=1;
int32_t c2=0;
//int32_t bgc=dvc(4+5);
//int32_t bgc=vc(1);
//enum { t_pen, t_fill, t_recolor, t_eyedropper, t_move, t_select, t_wand, t_max };
int32_t floating_tile = -1;
int32_t tool = t_pen;
int32_t old_tool = -1;
int32_t tool_cur = -1;
int32_t select_mode = 0;
int32_t drawing=0;


void update_tool_cursor()
{
	show_mouse(NULL);
	
	int32_t temp_mouse_x=gui_mouse_x();
	int32_t temp_mouse_y=gui_mouse_y();
	
	int32_t type=0;
	
	if(has_selection())
	{
		switch(tool)
		{
		case t_select:
		case t_wand:
			type+=select_mode;
			break;
		}
	}
	
	if(isinRect(temp_mouse_x,temp_mouse_y,zoomtile.x,zoomtile.y-(tool==t_fill ? (14) : 0),zoomtile.x+(zoomtile.w*zoomtile.xscale)-2,zoomtile.y+(zoomtile.h*zoomtile.yscale)-2-(tool==t_fill ? (14) : 0))) //inside the zoomed tile window
	{
		if(tool_cur==-1)
		{
			set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][type]);
			
			switch(tool)
			{
			case t_fill:
				set_mouse_sprite_focus(1, 14);
				break;
				
			case t_move:
				set_mouse_sprite_focus(8, 8);
				break;
				
			default:
				set_mouse_sprite_focus(1, 1);
				break;
			}
		}
		
		tool_cur=tool;
	}
	else if(tool_cur != -1)
	{
		set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
		tool_cur = -1;
	}
	show_mouse(screen);
}

void draw_edit_scr(int32_t tile,int32_t flip,int32_t cs,byte *oldtile, bool create_tbar)
{
	PALETTE tpal;
	static BITMAP *preview_bmp = create_bitmap_ex(8, 64, 64);
	al_lock_bitmap(al_get_target_bitmap(),ALLEGRO_PIXEL_FORMAT_ANY,ALLEGRO_LOCK_READWRITE);
	clear_a5_bmp(AL5_INVIS);
	clear_zqdialog_a4();
	al_unlock_bitmap(al_get_target_bitmap());
	
	jwin_draw_win_a5(0, 0, zq_screen_w-1, zq_screen_h-1, FR_WIN);
	
	jwin_draw_titlebar_a5(0, 3, zq_screen_w-6, 18, "", true, true);
	
	jwin_textout_a5(get_custom_font_a5(CFONT_TITLE),jwin_a5_pal(jcTITLEFG),5,5,0,fmt::format("Tile Editor ({})",tile).c_str());
	
	clear_to_color(preview_bmp, 0);
	
	zc_swap(oldtile,newtilebuf[tile].data); //Put oldtile in the tile buffer
	jwin_draw_win_a5(prev_til_1.x-2,prev_til_1.y-2, prev_til_1.w+4, prev_til_1.h+4, FR_DEEP);
	puttile16(preview_bmp,tile,0,0,cs,flip);
	stretch_blit(preview_bmp, screen, 0, 0, 16, 16, prev_til_1.x, prev_til_1.y, prev_til_1.w, prev_til_1.h);
	
	clear_to_color(preview_bmp, 0);
	jwin_draw_win_a5(prev_til_2.x-2,prev_til_2.y-2, prev_til_2.w+4, prev_til_2.h+4, FR_DEEP);
	overtile16(preview_bmp,tile,0,0,cs,flip);
	masked_stretch_blit(preview_bmp, screen, 0, 0, 16, 16, prev_til_2.x, prev_til_2.y, prev_til_2.w, prev_til_2.h);
	zc_swap(oldtile,newtilebuf[tile].data); //Swap the real tile back to the buffer
	
	unpack_tile(newtilebuf, tile, 0, false);
	if(floating_sel)
		for(auto q = 0; q < 256; ++q)
			if(floatsel[q])
				unpackbuf[q] = floatsel[q];
	byte tmptile[256];
	byte *tmpptr = tmptile;
	zc_swap(tmpptr,newtilebuf[tile].data); //Put temp data in the tile buffer
	pack_tile(newtilebuf,unpackbuf,tile);
	clear_to_color(preview_bmp, 0);
	
	jwin_draw_win_a5(prev_til_3.x-2,prev_til_3.y-2, prev_til_3.w+4, prev_til_3.h+4, FR_DEEP);
	puttile16(preview_bmp,tile,0,0,cs,flip);
	stretch_blit(preview_bmp, screen, 0, 0, 16, 16, prev_til_3.x, prev_til_3.y, prev_til_3.w, prev_til_3.h);
	
	clear_to_color(preview_bmp, 0);
	jwin_draw_win_a5(prev_til_4.x-2,prev_til_4.y-2, prev_til_4.w+4, prev_til_4.h+4, FR_DEEP);
	overtile16(preview_bmp,tile,0,0,cs,flip);
	masked_stretch_blit(preview_bmp, screen, 0, 0, 16, 16, prev_til_4.x, prev_til_4.y, prev_til_4.w, prev_til_4.h);
	
	//Color info
	{
		auto fh = color_info.yscale = text_height(font);
		int y = color_info.y;
		int rx = color_info.x+color_info.xscale;
		color_info.h = 1;
		if(showcolortip)
		{
			gui_textout_ln_a5(a5font,"Colors:",rx,y,jwin_a5_pal(jcBOXFG),jwin_a5_pal(jcBOX),2);
			auto str = get_tile_colornames(tile,cs);
			size_t pos = 0;
			char buf[512] = {0};
			char cbuf[16] = {0};
			while(pos < str.size())
			{
				y += fh;
				if(y+fh > zq_screen_h)
					break; //Out of space!
				auto endpos = str.find_first_of('\n',pos);
				
				if(endpos == std::string::npos)
				{
					strcpy(buf,str.substr(pos).c_str());
					pos = str.size();
				}
				else
				{
					strcpy(buf,str.substr(pos,endpos-pos+1).c_str());
					pos = endpos+1;
				}
				//Ensure the name fits horizontally
				if(gui_text_width_a5(a5font,buf) > color_info.xscale)
				{
					size_t pos = 0;
					for(; buf[pos]; ++pos)
					{
						if(buf[pos] == ':')
						{
							strcpy(cbuf, buf+pos);
							buf[pos] = 0;
							break;
						}
					}
					size_t clen = text_length(font,cbuf);
					size_t dotlen = text_length(font,"..");
					
					while(pos > 0 && (dotlen+clen+text_length(font,buf) > color_info.xscale))
						buf[--pos] = 0;
					while(buf[pos] == ' ')
						buf[pos] = 0;
					strcat(buf,"..");
					strcat(buf,cbuf);
				}
				gui_textout_ln_a5(a5font,buf,rx,y,jwin_a5_pal(jcBOXFG),
					jwin_a5_pal(jcBOX),2);
				++color_info.h;
			}
			jwin_draw_frame_a5(color_info.x-2,color_info.y-2,color_info.tw()+4,color_info.th()+4,FR_DEEP);
		}
		else
		{
			jwin_draw_text_button_a5(color_info_btn.x,color_info_btn.y,color_info_btn.w,
				color_info_btn.h,"Show Colors",0,false);
		}
	}
	
	zc_swap(tmpptr,newtilebuf[tile].data); //Swap the real tile back to the buffer
	
	jwin_draw_win_a5(zoomtile.x-3, zoomtile.y-3, (zoomtile.w*zoomtile.xscale)+5, (zoomtile.h*zoomtile.yscale)+5, FR_DEEP);
	zoomtile16(tile,zoomtile.x-1,zoomtile.y-1,cs,flip,zoomtile.xscale);
	
	if(floating_sel)
		jwin_textout_a5(a5font,jwin_a5_pal(jcBOXFG),status_info.x,status_info.y+0,0,"Floating selection");
	jwin_textout_a5(a5font,jwin_a5_pal(jcBOXFG),status_info.x,status_info.y+(1*status_info.yscale),0,fmt::format("Tile: {}",tile).c_str());
	if(newtilebuf[tile].format==tf8Bit)
		jwin_textout_a5(a5font,jwin_a5_pal(jcBOXFG),status_info.x,status_info.y+(2*status_info.yscale),0,"8-bit");
	else
		jwin_textout_a5(a5font,jwin_a5_pal(jcBOXFG),status_info.x,status_info.y+(2*status_info.yscale),0,fmt::format("CSet: {}",cs).c_str());
	
	PALETTE temppal;
	
	bool is8b = newtilebuf[tile].format > tf4Bit;
	int hlthick = 4;
	int extraborder = is8b ? 8 : 0;
	int borderthick = hlthick+extraborder;
	int csofs = is8b ? 0 : CSET(cs);
	if(is8b)
	{
		al_draw_filled_rectangle(cpalette_8.x-2-extraborder, cpalette_8.y-2-extraborder, cpalette_8.x+cpalette_8.tw()+2*(2+extraborder), cpalette_8.y+cpalette_8.th()+2*(2+extraborder), jwin_a5_pal(jcBOX));
		jwin_draw_win_a5(cpalette_8.x-2, cpalette_8.y-2, cpalette_8.tw()+4, cpalette_8.th()+4, FR_DEEP);
		
		for(int32_t i=0; i<cpalette_8.w*cpalette_8.h; ++i)
		{
			size_and_pos const& s = cpalette_8.subsquare(i);
			al_draw_filled_rectangle(s.x,s.y,s.x+s.w,s.y+s.h,a5color(i));
		}
		
		int xx = cpalette_8.x+1, yy = cpalette_8.y+1;
		al_draw_x(xx,yy,xx+(cpalette_8.xscale-4),yy+(cpalette_8.yscale-4),getHighlightColor(a5color(0)),1);
	}
	else
	{
		al_draw_filled_rectangle(cpalette_4.x-2-borderthick, cpalette_4.y-2-borderthick, cpalette_4.x+cpalette_4.tw()+2*(2+borderthick), cpalette_4.y+cpalette_4.th()+2*(2+borderthick), jwin_a5_pal(jcBOX));
		jwin_draw_win_a5(cpalette_4.x-2, cpalette_4.y-2, (cpalette_4.xscale*cpalette_4.w)+4, (cpalette_4.yscale*cpalette_4.h)+4, FR_DEEP);
		
		for(int32_t i=0; i<cpalette_4.w*cpalette_4.h; i++)
		{
			size_and_pos const& s = cpalette_4.subsquare(i);
			al_draw_filled_rectangle(s.x,s.y,s.x+s.w,s.y+s.h,a5color(CSET(cs)+i));
		}
		
		int xx = cpalette_4.x+1, yy = cpalette_4.y+1;
		al_draw_x(xx,yy,xx+(cpalette_4.xscale-4),yy+(cpalette_4.yscale-4),getHighlightColor(a5color(CSET(cs))),0);
	}
	
	ALLEGRO_COLOR rc1 = a5color(c1+csofs);
	ALLEGRO_COLOR rc2 = a5color(c2+csofs);
	
	al_draw_filled_rectangle(bg_prev.x, bg_prev.y, bg_prev.x+bg_prev.w, bg_prev.y+bg_prev.h, jwin_a5_pal(jcTEXTFG));
	al_draw_filled_rectangle(bg_prev.x+1, bg_prev.y+1, bg_prev.x+bg_prev.w-1, bg_prev.y+bg_prev.h-1, jwin_a5_pal(jcTEXTBG));
	al_draw_filled_rectangle(bg_prev.x+3, bg_prev.y+3, bg_prev.x+bg_prev.w-3, bg_prev.y+bg_prev.h-3, rc2);
	if(c2==0)
		al_draw_x(bg_prev.x+1, bg_prev.y+1, bg_prev.x+bg_prev.w-1, bg_prev.y+bg_prev.h-1,getHighlightColor(rc2),1);
	
	al_draw_filled_rectangle(fg_prev.x, fg_prev.y, fg_prev.x+fg_prev.w, fg_prev.y+fg_prev.h, jwin_a5_pal(jcTEXTFG));
	al_draw_filled_rectangle(fg_prev.x+1, fg_prev.y+1, fg_prev.x+fg_prev.w-1, fg_prev.y+fg_prev.h-1, jwin_a5_pal(jcTEXTBG));
	al_draw_filled_rectangle(fg_prev.x+3, fg_prev.y+3, fg_prev.x+fg_prev.w-3, fg_prev.y+fg_prev.h-3, rc1);
	if(c1==0)
		al_draw_x(fg_prev.x+1, fg_prev.y+1, fg_prev.x+fg_prev.w-1, fg_prev.y+fg_prev.h-1,getHighlightColor(rc1),1);
	
	jwin_draw_text_button_a5(ok_button.x,ok_button.y,ok_button.w,ok_button.h,"OK",0,false);
	jwin_draw_text_button_a5(cancel_button.x,cancel_button.y,cancel_button.w,cancel_button.h,"Cancel",0,false);
	jwin_draw_text_button_a5(edit_button.x,edit_button.y,edit_button.w,edit_button.h,"Edit Pal",0,false);
	draw_checkbox_a5(hlcbox.x, hlcbox.y, hlcbox.w, hlcbox.h, tthighlight);
	gui_textout_ln_a5(a5font,"Highlight Hover",hlcbox.x+hlcbox.w+2,hlcbox.y+hlcbox.h/2-al_get_font_line_height(a5font)/2,jwin_a5_pal(jcBOXFG),jwin_a5_pal(jcBOX),0);
	
	//tool buttons
	for(int32_t toolbtn = 0; toolbtn < t_max; ++toolbtn)
	{
		auto bmp = toolbtn+MOUSE_BMP_SWORD;
		int col = toolbtn%tool_btns.w;
		int row = toolbtn/tool_btns.w;
		
		jwin_draw_button_a5(tool_btns.x+(col*tool_btns.xscale),tool_btns.y+(row*tool_btns.yscale),tool_btns.xscale,tool_btns.yscale,tool==toolbtn?2:0,0);
		masked_stretch_blit(mouse_bmp_1x[bmp][0],screen,0,0,16,16,tool_btns.x+(col*tool_btns.xscale)+3+(tool==toolbtn?1:0),tool_btns.y+3+(row*tool_btns.yscale)+(tool==toolbtn?1:0),tool_btns.xscale-7,tool_btns.yscale-7);
	}
	
	//coordinates
	{
		auto mx = gui_mouse_x();
		auto my = gui_mouse_y();
		int32_t ind = zoomtile.rectind(mx,my);
		int32_t temp_x=ind%zoomtile.w;
		int32_t temp_y=ind/zoomtile.w;
		int color = -1;
		
		if(ind > -1)
		{
			char xbuf[16];
			sprintf(xbuf, "x: %d", temp_x);
			jwin_textout_a5(a5font,jwin_a5_pal(jcBOXFG),status_info.x,status_info.y+(3*status_info.yscale),0,xbuf);
			jwin_textout_a5(a5font,jwin_a5_pal(jcBOXFG),status_info.x+text_length(font,xbuf)+8,status_info.y+(3*status_info.yscale),0,fmt::format("y: {}",temp_y).c_str());
			unpack_tile(newtilebuf, tile, 0, false);
			byte *si = unpackbuf;
			si+=ind;
			color = *si;
		}
		else if(fg_prev.rect(mx,my))
			color = c1;
		else if(bg_prev.rect(mx,my))
			color = c2;
		else color = (is8b ? cpalette_8 : cpalette_4).rectind(mx,my);
		if(color > -1)
		{
			get_palette(tpal);
			char separator = ' ';
			char buf[512] = {0};
			
			int realcol = color+(is8b?0:CSET(cs));
			ALLEGRO_COLOR rcol = a5color(realcol);
			bool xcolor = (is8b ? realcol == 0 : (realcol%16)==0);
			auto& c = tpal[realcol];
			ALLEGRO_COLOR hlcol = getHighlightColor(rcol);
			ALLEGRO_COLOR cleanhl = AL5_PINK;
			
			if(tthighlight)
			{
				size_and_pos const& mainsqr = is8b ? cpalette_8 : cpalette_4;
				size_and_pos const& csqr = mainsqr.subsquare(color);
				
				if(is8b)
				{
					highlight_sqr(cleanhl, csqr.x, mainsqr.y, csqr.w, mainsqr.h*mainsqr.yscale, hlthick); //column
					highlight_sqr(cleanhl, mainsqr.x, csqr.y, mainsqr.w*mainsqr.xscale, csqr.h, hlthick); //row
				}
				highlight_sqr(cleanhl, csqr.x-borderthick, csqr.y-borderthick, csqr.w+borderthick*2, csqr.h+borderthick*2, hlthick); //square hl
				al_draw_filled_rectangle(csqr.x-extraborder, csqr.y-extraborder, csqr.x+csqr.w+extraborder, csqr.y+csqr.h+extraborder, rcol); //square color
				if(xcolor)
				{
					int xx = csqr.x-extraborder+4, yy = csqr.y-extraborder+4;
					al_draw_x(xx,yy,xx+(csqr.w+(extraborder*2)-8),yy+(csqr.h+(extraborder*2)-8),hlcol,1);
				}
				highlight_sqr(hlcol, csqr.x-extraborder, csqr.y-extraborder, csqr.w+extraborder*2, csqr.h+extraborder*2, 1); //highlight border
			}
			
			sprintf(buf, "%02d %02d %02d %c(0x%02X %d)",c.r,c.g,c.b,separator,realcol,color);
			gui_textout_ln_a5(a5font,buf,hover_info.x,hover_info.y+(2*hover_info.yscale),jwin_a5_pal(jcBOXFG),jwin_a5_pal(jcBOX),0);
			
			strcpy(buf, get_color_name(realcol, is8b).c_str());
			gui_textout_ln_a5(a5font,buf,hover_info.x,hover_info.y+(1*hover_info.yscale),jwin_a5_pal(jcBOXFG),jwin_a5_pal(jcBOX),0);
			
			sprintf(buf, "#%02X%02X%02X", tpal[realcol].r*4,tpal[realcol].g*4,tpal[realcol].b*4);
			gui_textout_ln_a5(a5font,buf,hover_info.x,hover_info.y+(0),jwin_a5_pal(jcBOXFG),jwin_a5_pal(jcBOX),0);
			
			al_draw_filled_rectangle(hov_prev.x, hov_prev.y, hov_prev.x+hov_prev.w, hov_prev.y+hov_prev.h, jwin_a5_pal(jcTEXTFG));
			al_draw_filled_rectangle(hov_prev.x+1, hov_prev.y+1, hov_prev.x+hov_prev.w-1, hov_prev.y+hov_prev.h-1, jwin_a5_pal(jcTEXTBG));
			al_draw_filled_rectangle(hov_prev.x+3, hov_prev.y+3, hov_prev.x+hov_prev.w-3, hov_prev.y+hov_prev.h-3, rcol);
			if(xcolor)
				al_draw_x(hov_prev.x+1, hov_prev.y+1, hov_prev.x+hov_prev.w-1, hov_prev.y+hov_prev.h-1,hlcol,1);
		}
	}
	
}

void normalize(int32_t tile,int32_t tile2, bool rect_sel, int32_t flip)
{
	unfloat_selection();
	if(tile>tile2)
	{
		zc_swap(tile, tile2);
	}
	
	int32_t left=zc_min(TILECOL(tile), TILECOL(tile2));
	int32_t columns=zc_max(TILECOL(tile), TILECOL(tile2))-left+1;
	
	int32_t start=tile;
	int32_t end=tile2;
	
	// Might have top-right and bottom-left corners selected...
	if(rect_sel && TILECOL(tile)>TILECOL(tile2))
	{
		start=tile-(TILECOL(tile)-TILECOL(tile2));
		end=tile2+(TILECOL(tile)-TILECOL(tile2));
	}
	
	for(int32_t temptile=start; temptile<=end; temptile++)
	{
		if(!rect_sel || ((TILECOL(temptile)>=left) && (TILECOL(temptile)<=left+columns-1)))
		{
			unpack_tile(newtilebuf, temptile, 0, false);
			
			if(flip&1)
			{
				for(int32_t y=0; y<16; y++)
				{
					for(int32_t x=0; x<8; x++)
					{
						zc_swap(unpackbuf[(y<<4)+x],unpackbuf[(y<<4)+15-x]);
					}
				}
			}
			
			if(flip&2)
			{
				for(int32_t y=0; y<8; y++)
				{
					for(int32_t x=0; x<16; x++)
					{
						zc_swap(unpackbuf[(y<<4)+x],unpackbuf[((15-y)<<4)+x]);
					}
				}
			}
			
			pack_tile(newtilebuf,unpackbuf,temptile);
		}
	}
}

void rotate_tile(int32_t tile, bool backward)
{
	unfloat_selection();
	unpack_tile(newtilebuf, tile, 0, false);
	byte tempunpackbuf[256];
	byte tempx, tempy;
	
	for(tempx=0; tempx<16; tempx++)
	{
		for(tempy=0; tempy<16; tempy++)
		{
			if(!backward)
			{
				tempunpackbuf[(tempy<<4)+tempx]=unpackbuf[((15-tempx)<<4)+tempy];
			}
			else
			{
				tempunpackbuf[((15-tempx)<<4)+tempy]=unpackbuf[(tempy<<4)+tempx];
			}
		}
	}
	
	pack_tile(newtilebuf,tempunpackbuf,tile);
}

static int32_t undocount=128;
byte undotile[256];

void wrap_tile(int32_t tile, int32_t vertical, int32_t horizontal, bool clear)
{
	byte buf[256];
	
	for(int32_t i=0; i<undocount; i++)
	{
		newtilebuf[tile].data[i]=undotile[i];
	}
	
	if(!(horizontal||vertical))
	{
		return;
	}
	
	unpack_tile(newtilebuf, tile, 0, true);
	
	for(int32_t i=0; i<256; i++)
	{
		auto shift_ind = ((i+horizontal)&0x0F)|((i+(vertical*0x10))&0xF0);
		buf[shift_ind] = unpackbuf[i];
	}
	
	if(clear)
	{
		for(int32_t r=0; r<abs(vertical); r++)
		{
			for(int32_t c=0; c<16; c++)
			{
				buf[(vertical>0?r:15-r)*16+c]=0;
			}
		}
		
		for(int32_t r=0; r<16; r++)
		{
			for(int32_t c=0; c<abs(horizontal); c++)
			{
				buf[r*16+(horizontal>0?c:15-c)]=0;
			}
		}
	}
	
	pack_tile(newtilebuf,buf,tile);
}

void wrap_sel_tile(int32_t vertical, int32_t horizontal)
{
	byte buf[256];
	
	if(!(horizontal||vertical))
	{
		return;
	}
	
	memset(buf,0,256);
	
	for(int32_t i=0; i<256; i++)
	{
		if(is_in_selection(i%16,i/16))
		{
			auto shift_ind = ((i+horizontal)&0x0F)|((i+(vertical*0x10))&0xF0);
			buf[shift_ind] = floatsel[i];
		}
	}
	
	memcpy(floatsel,buf,256);
}

void float_selection(int32_t tile, bool clear)
{
	if(floating_sel) return;
	floating_sel = true;
	floating_tile = tile;
	
	unpack_tile(newtilebuf, tile, 0, true);
	
	for(auto q = 0; q < 256; ++q)
	{
		if(is_in_selection(q%16,q/16))
		{
			floatsel[q] = unpackbuf[q];
			unpackbuf[q] = clear ? 0 : c2;
		}
		else floatsel[q] = 0;
	}
	
	pack_tile(newtilebuf,unpackbuf,tile);
}

void unfloat_selection()
{
	if(!floating_sel) return;
	floating_sel = false;
	
	unpack_tile(newtilebuf, floating_tile, 0, true);
	
	for(auto q = 0; q < 256; ++q)
	{
		if(floatsel[q])
		{
			unpackbuf[q] = floatsel[q];
		}
	}
	
	pack_tile(newtilebuf,unpackbuf,floating_tile);
	floating_tile = -1;
}

void shift_tile_colors(int32_t tile, int32_t amount, bool ignore_transparent)
{
	if(floating_sel)
	{
		for(auto q = 0; q < 256; ++q)
		{
			if(ignore_transparent && floatsel[q]==0)
				continue;
			floatsel[q]=wrap(floatsel[q]+amount, 0, newtilebuf[tile].format==tf8Bit ? 191 : 15);
		}
		return;
	}
	
	byte buf[256];
	unpack_tile(newtilebuf, tile, 0, true);
	
	for(int32_t i=0; i<256; i++)
	{
		buf[i]=unpackbuf[i];
		
		if(!is_in_selection(i&0x0F, (i&0xF0)>>4))
			continue;
			
		if(ignore_transparent)
		{
			if(buf[i]==0)
				continue;
				
			buf[i]=wrap(buf[i]+amount, 1, newtilebuf[tile].format==tf8Bit ? 191 : 15);
		}
		else // Don't ignore transparent
			buf[i]=wrap(buf[i]+amount, 0, newtilebuf[tile].format==tf8Bit ? 191 : 15);
	}
	
	pack_tile(newtilebuf,buf,tile);
}

void clear_selection_grid()
{
	unfloat_selection();
	for(int32_t x=0; x<18; ++x)
	{
		for(int32_t y=0; y<18; ++y)
		{
			selection_grid[x][y]=0;
		}
	}
}

void invert_selection_grid()
{
	unfloat_selection();
	for(int32_t x=1; x<17; ++x)
	{
		for(int32_t y=1; y<17; ++y)
		{
			selection_grid[x][y]=selection_grid[x][y]?0:1;
		}
	}
}

void shift_selection_grid(int32_t xoffs, int32_t yoffs)
{
	byte local_grid[16][16];
	memset(local_grid, 0, sizeof(local_grid));
	for(auto x = 0; x < 16; ++x)
	{
		for(auto y = 0; y < 16; ++y)
		{
			auto offs_x = (x+xoffs)%16, offs_y = (y+yoffs)%16;
			if(offs_x < 0) offs_x = (16 - abs(offs_x));
			if(offs_y < 0) offs_y = (16 - abs(offs_y));
			local_grid[offs_x][offs_y] = selection_grid[x+1][y+1]?1:0;
		}
	}
	for(auto x = 0; x < 16; ++x)
	{
		for(auto y = 0; y < 16; ++y)
		{
			selection_grid[x+1][y+1] = local_grid[x][y]?1:0;
		}
	}
}

void show_edit_tile_help()
{
	InfoDialog("Help: Tile Editor", "Hotkeys:"
		"\nF1: This Help Dialog"
		"\nEnter: Unfloat Sel / OK | Esc: Unfloat Sel / Cancel"
		"\nDelete: Clear Tile/Sel | Ctrl+Delete: Clear Tile"
		"\nA: Sel All | D: Unselect Sel | I: Invert Sel"
		"\nH/V: Flip | (Shift+)R: Rotate"
		"\n+/-: Change CSet | Ctrl +/-: Shift Colors"
		"\n(Ctrl+)S: Swap Colors | U/Ctrl+Z: Undo"
		"\nF12: Screenshot (whole screen)"
		"\nArrows: Shift Pixel | Ctrl+Arrows: Change Tile"
		"\nWhen not on Select tools, hold to swap:"
		"\nCtrl - Fill | Alt - Grab | Ctrl+Alt - Recolor").show();
}

void edit_tile(int32_t tile,int32_t flip,int32_t &cs)
{
	popup_zqdialog_start_a5(0,0,LARGE_W,LARGE_H);
	popup_zqdialog_start(0,0,LARGE_W,LARGE_H,0xFF);
	
	RenderTreeItem* ttip = add_dlg_layer_a4(0xFF);
	RenderTreeItem* ttip_hl = add_dlg_layer();
	
	FONT* oldfont = font;
	ALLEGRO_FONT* oldfont_a5 = a5font;
	font = get_custom_font(CFONT_DLG);
	a5font = get_custom_font_a5(CFONT_DLG);
	edit_button.h = ok_button.h = cancel_button.h = 12+text_height(font);
	status_info.yscale = text_height(font);
	status_info.y = 308-(status_info.h*status_info.yscale);
	hover_info.yscale = status_info.yscale;
	hover_info.y = 338-(hover_info.h*hover_info.yscale);
	undocount = tilesize(newtilebuf[tile].format);
	clear_selection_grid();
	selecting_x1=selecting_x2=selecting_y1=selecting_y2=-1;
	
	tthighlight = zc_get_config("ZQ_GUI","tile_edit_fancyhighlight",1);
	showcolortip = zc_get_config("ZQ_GUI","tile_edit_colornames",1);
	
	PALETTE tpal;
	byte oldtile[256];
	
	memset(&tpal, 0, sizeof(PALETTE));
	memset(oldtile, 0, 256);
	
	for(int32_t i=0; i<undocount; i++)
	{
		oldtile[i]=undotile[i]=newtilebuf[tile].data[i];
	}
	byte undoselgrid[16][16];
	for(auto x = 0; x < 16; ++x)
		for(auto y = 0; y < 16; ++y)
			undoselgrid[x][y] = selection_grid[x+1][y+1];
	for(auto q = 0; q < 256; ++q)
	{
		floatsel[q] = 0;
		undofloatsel[q] = 0;
		floating_sel = false;
		undo_is_floatsel = false;
	}
	
	int32_t tile_x=-1, tile_y=-1;
	int32_t temp_x=-1, temp_y=-1;
	bool bdown=false;
	int32_t done=0;
	drawing=0;
	tool_cur = -1;
	
	get_palette(tpal);
	
	draw_edit_scr(tile,flip,cs,oldtile, true);
	update_tool_cursor();
	
	update_hw_screen(true);
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	int32_t move_origin_x=-1, move_origin_y=-1;
	int32_t prev_x=-1, prev_y=-1;
	
	
	
	byte selection_pattern_source[8][8]=
	{
		{1, 1, 1, 1, 0, 0, 0, 0},
		{1, 1, 1, 0, 0, 0, 0, 1},
		{1, 1, 0, 0, 0, 0, 1, 1},
		{1, 0, 0, 0, 0, 1, 1, 1},
		{0, 0, 0, 0, 1, 1, 1, 1},
		{0, 0, 0, 1, 1, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 1, 1, 0, 0, 0},
	};
	
	byte selecting_pattern_source[8][8]=
	{
		{1, 1, 0, 0, 0, 0, 1, 1},
		{1, 0, 0, 0, 0, 1, 1, 1},
		{0, 0, 0, 0, 1, 1, 1, 1},
		{0, 0, 0, 1, 1, 1, 1, 0},
		{0, 0, 1, 1, 1, 1, 0, 0},
		{0, 1, 1, 1, 1, 0, 0, 0},
		{1, 1, 1, 1, 0, 0, 0, 0},
		{1, 1, 1, 0, 0, 0, 0, 1},
	};
	
	byte intersection_pattern_source[8][8]=
	{
		{0, 0, 1, 1, 0, 0, 1, 1},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{1, 1, 0, 0, 1, 1, 0, 0},
		{1, 0, 0, 1, 1, 0, 0, 1},
		{0, 0, 1, 1, 0, 0, 1, 1},
		{0, 1, 1, 0, 0, 1, 1, 0},
		{1, 1, 0, 0, 1, 1, 0, 0},
		{1, 0, 0, 1, 1, 0, 0, 1},
	};
	
	selection_pattern=create_bitmap_ex(8, 8, 8);
	
	for(int32_t x=0; x<8; ++x)
	{
		for(int32_t y=0; y<8; ++y)
		{
			selection_pattern->line[y][x]=selection_pattern_source[x][y]?vc(0):vc(15);
		}
	}
	
	selecting_pattern=create_bitmap_ex(8, 8, 8);
	
	for(int32_t x=0; x<8; ++x)
	{
		for(int32_t y=0; y<8; ++y)
		{
			selecting_pattern->line[y][x]=selecting_pattern_source[x][y]?vc(0):vc(15);
		}
	}
	
	intersection_pattern=create_bitmap_ex(8, 8, 8);
	
	for(int32_t x=0; x<8; ++x)
	{
		for(int32_t y=0; y<8; ++y)
		{
			intersection_pattern->line[y][x]=intersection_pattern_source[x][y]?vc(0):vc(15);
		}
	}

	do
	{
		int32_t temp_mouse_x=gui_mouse_x();
		int32_t temp_mouse_y=gui_mouse_y();
		rest(4);
		bool did_wand_select=false;
		
		if((tooltip_trigger.x>-1&&tooltip_trigger.y>-1)&&(!tooltip_trigger.rect(temp_mouse_x,temp_mouse_y)))
		{
			clear_tooltip();
		}
		
		if(keypressed())
		{
			bool ctrl = key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL];
			switch(readkey()>>8)
			{
				case KEY_F1:
					show_edit_tile_help();
					break;
				case KEY_ENTER_PAD:
				case KEY_ENTER:
					if(floating_sel)
						unfloat_selection();
					else done=2;
					break;
					
				case KEY_ESC:
					if(floating_sel)
						unfloat_selection();
					else done=1;
					break;
				
				case KEY_DEL:
				{
					unpack_tile(newtilebuf, tile, 0, false);
					bool all = key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL] || !has_selection();
					bool canDel = false;
					if(all)
					{
						//Check all
						for(auto q = 0; q < 256; ++q)
							if(unpackbuf[q])
							{
								canDel = true;
								break;
							}
					}
					else
					{
						//Check selection
						for(auto x = 0; x < 16; ++x)
							for(auto y = 0; y < 16; ++y)
								if(is_in_selection(x,y))
									if(unpackbuf[(y<<4)+x])
									{
										canDel = true;
										break;
									}
					}
					if(!canDel) break; //don't delete (and thus reset undo) if nothing would change!
					
					for(int32_t i=0; i<undocount; i++)
					{
						undotile[i]=newtilebuf[tile].data[i];
					}
					for(auto x = 0; x < 16; ++x)
						for(auto y = 0; y < 16; ++y)
							undoselgrid[x][y] = selection_grid[x+1][y+1];
					for(auto q = 0; q < 256; ++q)
						undofloatsel[q] = floatsel[q];
					undo_is_floatsel = floating_sel;
					
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL] || !has_selection())
					{
						//Delete all
						for(auto q = 0; q < 256; ++q)
						{
							unpackbuf[q] = 0;
							floatsel[q] = 0;
						}
					}
					else
					{
						//Delete selection
						for(auto x = 0; x < 16; ++x)
							for(auto y = 0; y < 16; ++y)
							{
								if(floating_sel)
								{
									floatsel[x+(y<<4)] = 0;
								}
								else if(is_in_selection(x,y))
								{
									unpackbuf[(y<<4)+x] = 0;
								}
							}
					}
					pack_tile(newtilebuf, unpackbuf, tile);
				}
				break;
					
				case KEY_A:
					clear_selection_grid();
					invert_selection_grid();
					break;
					
				case KEY_D:
					clear_selection_grid();
					break;
					
				case KEY_I:
					invert_selection_grid();
					break;
					
				case KEY_H:
					flip^=1;
					normalize(tile,tile,0,flip);
					flip=0;
					break;
					
				case KEY_V:
					flip^=2;
					normalize(tile,tile,0,flip);
					flip=0;
					break;
					
				case KEY_F12:
					onSnapshot();
					break;
					
				case KEY_R:
				{
					go_tiles();
					rotate_tile(tile,(key[KEY_LSHIFT] || key[KEY_RSHIFT]));
					saved=false;
					break;
				}
				
				case KEY_EQUALS:
				case KEY_PLUS_PAD:
				{
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL] ||
							key[KEY_ALT] || key[KEY_ALTGR])
					{
						for(int32_t i=0; i<undocount; i++)
							undotile[i]=newtilebuf[tile].data[i];
						for(auto x = 0; x < 16; ++x)
							for(auto y = 0; y < 16; ++y)
								undoselgrid[x][y] = selection_grid[x+1][y+1];
						for(auto q = 0; q < 256; ++q)
							undofloatsel[q] = floatsel[q];
						undo_is_floatsel = floating_sel;
							
						if(key[KEY_ALT] || key[KEY_ALTGR])
							shift_tile_colors(tile, 16, false);
						else
							shift_tile_colors(tile, 1, key[KEY_LSHIFT] || key[KEY_RSHIFT]);
					}
					else
						cs = (cs<13) ? cs+1:0;
					break;
				}
				
				case KEY_MINUS:
				case KEY_MINUS_PAD:
				{
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL] ||
							key[KEY_ALT] || key[KEY_ALTGR])
					{
						for(int32_t i=0; i<undocount; i++)
							undotile[i]=newtilebuf[tile].data[i];
						for(auto x = 0; x < 16; ++x)
							for(auto y = 0; y < 16; ++y)
								undoselgrid[x][y] = selection_grid[x+1][y+1];
						for(auto q = 0; q < 256; ++q)
							undofloatsel[q] = floatsel[q];
						undo_is_floatsel = floating_sel;
							
						if(key[KEY_ALT] || key[KEY_ALTGR])
							shift_tile_colors(tile, -16, false);
						else
							shift_tile_colors(tile, -1, key[KEY_LSHIFT] || key[KEY_RSHIFT]);
					}
					else
						cs = (cs>0) ? cs-1:13;
					break;
				}
				
				case KEY_SPACE:
					gridmode=(gridmode+1)%gm_max;
					break;
				
				case KEY_Z:
					if(!ctrl)
						break;
					//Ctrl+Z == undo
				[[fallthrough]];
				case KEY_U:
					for(int32_t i=0; i<undocount; i++)
						zc_swap(undotile[i],newtilebuf[tile].data[i]);
					
					for(auto x = 0; x < 16; ++x)
						for(auto y = 0; y < 16; ++y)
							zc_swap(selection_grid[x+1][y+1], undoselgrid[x][y]);
						
					for(auto q = 0; q < 256; ++q)
						zc_swap(undofloatsel[q], floatsel[q]);
					zc_swap(undo_is_floatsel, floating_sel);
					break;
					
				case KEY_S:
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
					{
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
						}
						for(auto x = 0; x < 16; ++x)
							for(auto y = 0; y < 16; ++y)
								undoselgrid[x][y] = selection_grid[x+1][y+1];
						for(auto q = 0; q < 256; ++q)
							undofloatsel[q] = floatsel[q];
						undo_is_floatsel = floating_sel;
						
						unpack_tile(newtilebuf, tile, 0, false);
						
						if(has_selection())
						{
							for(int32_t i=0; i<256; i++)
							{
								if(!is_in_selection(i%16,i/16))
									continue;
								if(unpackbuf[i]==c1)
								{
									unpackbuf[i]=c2;
								}
								else if(unpackbuf[i]==c2)
								{
									unpackbuf[i]=c1;
								}
								if(floating_sel)
								{
									if(floatsel[i]==c1)
									{
										floatsel[i]=c2;
									}
									else if(floatsel[i]==c2)
									{
										floatsel[i]=c1;
									}
								}
							}
						}
						else
						{
							for(int32_t i=0; i<256; i++)
							{
								if(unpackbuf[i]==c1)
								{
									unpackbuf[i]=c2;
								}
								else if(unpackbuf[i]==c2)
								{
									unpackbuf[i]=c1;
								}
							}
						}
						
						pack_tile(newtilebuf, unpackbuf,tile);
					}
					
					zc_swap(c1,c2);
					break;
					
				case KEY_UP:
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
					{
						unfloat_selection();
						tile=zc_max(0,tile-TILES_PER_ROW);
						undocount = tilesize(newtilebuf[tile].format);
						
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
							oldtile[i]=undotile[i];
						}
					}
					else
					{
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
						}
						for(auto x = 0; x < 16; ++x)
							for(auto y = 0; y < 16; ++y)
								undoselgrid[x][y] = selection_grid[x+1][y+1];
						for(auto q = 0; q < 256; ++q)
							undofloatsel[q] = floatsel[q];
						undo_is_floatsel = floating_sel;
						if(has_selection())
						{
							float_selection(tile,key[KEY_LSHIFT]||key[KEY_RSHIFT]);
							wrap_sel_tile(-1, 0);
							shift_selection_grid(0, -1);
						}
						else wrap_tile(tile, -1, 0, false);
					}
					break;
					
				case KEY_DOWN:
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
					{
						unfloat_selection();
						tile=zc_min(tile+TILES_PER_ROW,NEWMAXTILES-1);
						undocount = tilesize(newtilebuf[tile].format);
						
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
							oldtile[i]=undotile[i];
						}
					}
					else
					{
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
						}
						for(auto x = 0; x < 16; ++x)
							for(auto y = 0; y < 16; ++y)
								undoselgrid[x][y] = selection_grid[x+1][y+1];
						for(auto q = 0; q < 256; ++q)
							undofloatsel[q] = floatsel[q];
						undo_is_floatsel = floating_sel;
						if(has_selection())
						{
							float_selection(tile,key[KEY_LSHIFT]||key[KEY_RSHIFT]);
							wrap_sel_tile(1, 0);
							shift_selection_grid(0, 1);
						}
						else wrap_tile(tile, 1, 0, false);
					}
					break;
					
				case KEY_LEFT:
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
					{
						unfloat_selection();
						tile=zc_max(0,tile-1);
						undocount = tilesize(newtilebuf[tile].format);
						
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
							oldtile[i]=undotile[i];
						}
					}
					else
					{
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
						}
						for(auto x = 0; x < 16; ++x)
							for(auto y = 0; y < 16; ++y)
								undoselgrid[x][y] = selection_grid[x+1][y+1];
						for(auto q = 0; q < 256; ++q)
							undofloatsel[q] = floatsel[q];
						undo_is_floatsel = floating_sel;
						if(has_selection())
						{
							float_selection(tile,key[KEY_LSHIFT]||key[KEY_RSHIFT]);
							wrap_sel_tile(0, -1);
							shift_selection_grid(-1, 0);
						}
						else wrap_tile(tile, 0, -1, false);
					}
					break;
					
				case KEY_RIGHT:
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
					{
						unfloat_selection();
						tile=zc_min(tile+1, NEWMAXTILES-1);
						undocount = tilesize(newtilebuf[tile].format);
						
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
							oldtile[i]=undotile[i];
						}
					}
					else
					{
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
						}
						for(auto x = 0; x < 16; ++x)
							for(auto y = 0; y < 16; ++y)
								undoselgrid[x][y] = selection_grid[x+1][y+1];
						for(auto q = 0; q < 256; ++q)
							undofloatsel[q] = floatsel[q];
						undo_is_floatsel = floating_sel;
						if(has_selection())
						{
							float_selection(tile,key[KEY_LSHIFT]||key[KEY_RSHIFT]);
							wrap_sel_tile(0, 1);
							shift_selection_grid(1, 0);
						}
						else wrap_tile(tile, 0, 1, false);
					}
					break;
			}
			clear_keybuf();
		}
		
		if(!gui_mouse_b())
		{
			if(is_selecting())
			{
				unfloat_selection();
				int32_t x1=zc_min(selecting_x1,selecting_x2);
				int32_t x2=zc_max(selecting_x1,selecting_x2);
				int32_t y1=zc_min(selecting_y1,selecting_y2);
				int32_t y2=zc_max(selecting_y1,selecting_y2);
				
				if(select_mode==0)
				{
					clear_selection_grid();
				}
				
				for(int32_t x=x1; x<=x2; ++x)
				{
					for(int32_t y=y1; y<=y2; ++y)
					{
						selection_grid[x+1][y+1]=((select_mode<2)?(1):(((select_mode==2)?(0):(selection_grid[x+1][y+1]))));
					}
				}
				
				if(select_mode==3)
				{
					for(int32_t y=0; y<16; ++y)
					{
						for(int32_t x=0; x<x1; ++x)
						{
							selection_grid[x+1][y+1]=0;
						}
						
						for(int32_t x=x2+1; x<16; ++x)
						{
							selection_grid[x+1][y+1]=0;
						}
					}
					
					for(int32_t x=x1; x<=x2; ++x)
					{
						for(int32_t y=0; y<y1; ++y)
						{
							selection_grid[x+1][y+1]=0;
						}
						
						for(int32_t y=y2+1; y<16; ++y)
						{
							selection_grid[x+1][y+1]=0;
						}
					}
				}
			}
			
			selecting_x1=selecting_x2=selecting_y1=selecting_y2=-1;
			did_wand_select=false;
		}
		
		bool alt=(key[KEY_ALT]||key[KEY_ALTGR]);
		bool shift=(key[KEY_LSHIFT] || key[KEY_RSHIFT]);
		bool ctrl=(key[KEY_LCONTROL] || key[KEY_RCONTROL]);
		static int32_t last_tool_val = 0;
		
		if(tool==t_select||tool==t_wand)
		{
			if(!drawing)
			{
				int32_t type=0;
				
				if(has_selection())
				{
					if(shift)
					{
						type+=1;
					}
					
					if(alt)
					{
						type+=2;
					}
				}
				
				if(type!=select_mode)
				{
					select_mode=type;
					
					if(isinRect(temp_mouse_x,temp_mouse_y-(tool==t_fill ? (14) : 0),zoomtile.x,zoomtile.y,zoomtile.x+(zoomtile.w*zoomtile.xscale)-2,zoomtile.y+(zoomtile.h*zoomtile.yscale)-2-(tool==t_fill ? (14) : 0))) //inside the zoomed tile window
					{
						set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][type]);
					}
				}
			}
		}
		else if(alt||ctrl)
		{
			if(old_tool==-1)
			{
				old_tool = tool;
				tool_cur = -1;
			}
			if(alt&&ctrl)
				tool = t_recolor;
			else if(alt)
				tool = t_eyedropper;
			else tool = t_fill;
		}
		else if(old_tool!=-1)
		{
			tool = old_tool;
			old_tool = -1;
			tool_cur = -1;
		}
		if(last_tool_val != tool)
		{
			tool_cur = -1;
			update_tool_cursor();
			last_tool_val = tool;
		}
		
		if(!bdown)
		{
			move_origin_x=prev_x=(temp_mouse_x-zoomtile.x)/zoomtile.xscale;
			move_origin_y=prev_y=(temp_mouse_y-zoomtile.y)/zoomtile.yscale;
		}
		
		if(gui_mouse_b()==1 && !bdown) //pressed the left mouse button
		{
			if(isinRect(temp_mouse_x,temp_mouse_y,zoomtile.x,zoomtile.y-(tool==t_fill ? (14) : 0),zoomtile.x+(zoomtile.w*zoomtile.xscale)-2,zoomtile.y+(zoomtile.h*zoomtile.yscale)-2-(tool==t_fill ? (14) : 0))) //inside the zoomed tile window
			{
				if(tool==t_move || tool==t_fill)
				{
					scare_mouse();
					
					if(tool==t_fill)
					{
						set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][0]);
						set_mouse_sprite_focus(1, 14);
					}
					else
					{
						set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][1]);
						set_mouse_sprite_focus(8, 8);
					}
					
					unscare_mouse();
					move_origin_x=prev_x=(temp_mouse_x-zoomtile.x)/zoomtile.xscale;
					move_origin_y=prev_y=(temp_mouse_y-zoomtile.y)/zoomtile.yscale;
				}
				
				for(int32_t i=0; i<undocount; i++)
				{
					undotile[i]=newtilebuf[tile].data[i];
				}
				for(auto x = 0; x < 16; ++x)
					for(auto y = 0; y < 16; ++y)
						undoselgrid[x][y] = selection_grid[x+1][y+1];
				for(auto q = 0; q < 256; ++q)
					undofloatsel[q] = floatsel[q];
				undo_is_floatsel = floating_sel;
				
				drawing=1;
			}
			
			if(ok_button.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_text_button_a5(ok_button.x,ok_button.y,ok_button.w,ok_button.h,"OK"))
				{
					done=2;
				}
			}
			
			if(cancel_button.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_text_button_a5(cancel_button.x,cancel_button.y,cancel_button.w,cancel_button.h,"Cancel"))
				{
					done=1;
				}
			}
			
			if(edit_button.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_text_button_a5(edit_button.x,edit_button.y,edit_button.w,edit_button.h,"Edit Pal"))
				{
					popup_menu(colors_menu,edit_button.x+2,edit_button.y-40);
					get_palette(tpal);
				}
			}
			
			if(showcolortip)
			{
				if(color_info.rect(temp_mouse_x,temp_mouse_y))
				{
					showcolortip = 0;
					zc_set_config("ZQ_GUI","tile_edit_colornames",0);
				}
			}
			else
			{
				if(color_info_btn.rect(temp_mouse_x,temp_mouse_y))
				{
					if(do_text_button_a5(color_info_btn.x,color_info_btn.y,color_info_btn.w,color_info_btn.h,"Show Colors"))
					{
						showcolortip = 1;
						zc_set_config("ZQ_GUI","tile_edit_colornames",1);
					}
				}
			}
			
			if(hlcbox.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_checkbox_a5(hlcbox.x,hlcbox.y,hlcbox.w,hlcbox.h,tthighlight))
				{
					zc_set_config("ZQ_GUI","tile_edit_fancyhighlight",tthighlight);
				}
			}
			
			switch(newtilebuf[tile].format)
			{
				case tf4Bit:
				{
					auto ind = cpalette_4.rectind(temp_mouse_x,temp_mouse_y);
					if(ind > -1)
					{
						c1 = ind;
					}
					break;
				}
				case tf8Bit:
				{
					auto ind = cpalette_8.rectind(temp_mouse_x,temp_mouse_y);
					if(ind > -1)
					{
						c1 = ind;
					}
					break;
				}
			}
			
			
			int32_t newtool = tool_btns.rectind(temp_mouse_x,temp_mouse_y);
			if(newtool > -1 && newtool < t_max)
			{
				tool=newtool;
			}
			
			if(x_btn.rect(temp_mouse_x,temp_mouse_y))
			{
				if(jwin_do_x_button_a5(x_btn.x, x_btn.y))
				{
					done=1;
				}
			}
			if(info_btn.rect(temp_mouse_x,temp_mouse_y))
			{
				if(jwin_do_question_button_a5(info_btn.x, info_btn.y))
				{
					show_edit_tile_help();
				}
			}
			
			bdown=true;
		}
		
		if(gui_mouse_b()&2 && !bdown) //pressed the right mouse button
		{
			if(isinRect(temp_mouse_x,temp_mouse_y,zoomtile.x,zoomtile.y-(tool==t_fill ? (14) : 0),zoomtile.x+(zoomtile.w*zoomtile.xscale)-2,zoomtile.y+(zoomtile.h*zoomtile.yscale)-2-(tool==t_fill ? (14) : 0))) //inside the zoomed tile window
			{
				if(tool==t_move || tool==t_fill)
				{
					scare_mouse();
					
					if(tool==t_fill)
					{
						set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][0]);
						set_mouse_sprite_focus(1, 14);
					}
					else
					{
						set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][1]);
						set_mouse_sprite_focus(8, 8);
					}
					
					unscare_mouse();
					move_origin_x=prev_x=(temp_mouse_x-zoomtile.x)/zoomtile.xscale;
					move_origin_y=prev_y=(temp_mouse_y-zoomtile.y)/zoomtile.yscale;
				}
				
				for(int32_t i=0; i<undocount; i++)
				{
					undotile[i]=newtilebuf[tile].data[i];
				}
				for(auto x = 0; x < 16; ++x)
					for(auto y = 0; y < 16; ++y)
						undoselgrid[x][y] = selection_grid[x+1][y+1];
				for(auto q = 0; q < 256; ++q)
					undofloatsel[q] = floatsel[q];
				undo_is_floatsel = floating_sel;
				
				drawing=2;
			}
			
			switch(newtilebuf[tile].format)
			{
				case tf4Bit:
				{
					auto ind = cpalette_4.rectind(temp_mouse_x,temp_mouse_y);
					if(ind > -1)
					{
						c2 = ind;
					}
					break;
				}
				case tf8Bit:
				{
					auto ind = cpalette_8.rectind(temp_mouse_x,temp_mouse_y);
					if(ind > -1)
					{
						c2 = ind;
					}
					break;
				}
			}
			
			bdown=true;
		}
		
		if(bdown&&!gui_mouse_b())  //released the buttons
		{
			if(isinRect(temp_mouse_x,temp_mouse_y,zoomtile.x,zoomtile.y-(tool==t_fill ? (14) : 0),zoomtile.x+(zoomtile.w*zoomtile.xscale)-2,zoomtile.y+(zoomtile.h*zoomtile.yscale)-2-(tool==t_fill ? (14) : 0))) //inside the zoomed tile window
			{
				if(tool==t_move || tool==t_fill)
				{
					scare_mouse();
					set_mouse_sprite(mouse_bmp[MOUSE_BMP_SWORD+tool][0]);
					
					if(tool==t_fill)
					{
						set_mouse_sprite_focus(1, 14);
					}
					else
					{
						set_mouse_sprite_focus(8, 8);
					}
					
					unscare_mouse();
				}
			}
		}
		
		if(drawing && zoomtile.rect(temp_mouse_x,temp_mouse_y)) //inside the zoomed tile window
		{
			int32_t ind = zoomtile.rectind(temp_mouse_x,temp_mouse_y);
			int32_t x=ind%zoomtile.w;
			int32_t y=ind/zoomtile.w;
			
			switch(tool)
			{
				case t_pen:
					if(flip&1) x=15-x;
					
					if(flip&2) y=15-y;
					
					if(is_in_selection(x,y))
					{
						if(floating_sel)
						{
							floatsel[(y<<4)+x]=(drawing==1)?c1:c2;
						}
						else
						{
							unpack_tile(newtilebuf, tile, 0, false);
							unpackbuf[((y<<4)+x)]=(drawing==1)?c1:c2;
							pack_tile(newtilebuf, unpackbuf,tile);
						}
					}
					break;
					
				case t_fill:
					if(is_in_selection(x,y))
					{
						tile_floodfill(tile,x,y,(drawing==1)?c1:c2);
						drawing=0;
					}
					break;
					
				case t_recolor:
					if(is_in_selection(x,y))
					{
						if(floating_sel)
						{
							tf_u = floatsel[(y<<4)+x];
							for(int32_t i=0; i<256; i++)
							{
								if(is_in_selection(i&15,i>>4))
								{
									if(floatsel[i]==tf_u)
									{
										floatsel[i]=(drawing==1)?c1:c2;
									}
								}
							}
						}
						else
						{
							unpack_tile(newtilebuf, tile, 0, false);
							tf_u = unpackbuf[(y<<4)+x];
							
							for(int32_t i=0; i<256; i++)
							{
								if(is_in_selection(i&15,i>>4))
								{
									if(unpackbuf[i]==tf_u)
									{
										unpackbuf[i]=(drawing==1)?c1:c2;
									}
								}
							}
							
							pack_tile(newtilebuf, unpackbuf,tile);
						}
						drawing=0;
					}
					break;
					
				case t_eyedropper:
					if(floating_sel)
						memcpy(unpackbuf, floatsel, 256);
					else unpack_tile(newtilebuf, tile, 0, false);
					
					if(gui_mouse_b()&1)
					{
						c1=unpackbuf[((y<<4)+x)];
					}
					
					if(gui_mouse_b()&2)
					{
						c2=unpackbuf[((y<<4)+x)];
					}
					break;
					
				case t_move:
					if((prev_x!=x)||(prev_y!=y))
					{
						if(has_selection())
						{
							float_selection(tile,key[KEY_LSHIFT]||key[KEY_RSHIFT]);
							wrap_sel_tile(y-prev_y, x-prev_x);
							shift_selection_grid(x-prev_x, y-prev_y);
						}
						else wrap_tile(tile, y-move_origin_y, x-move_origin_x, drawing==2);
						prev_x=x;
						prev_y=y;
					}
					break;
					
				case t_select:
					unfloat_selection();
					if(flip&1) x=15-x;
					
					if(flip&2) y=15-y;
					
					if(selecting_x1==-1||selecting_y1==-1)
					{
						selecting_x1=x;
						selecting_y1=y;
					}
					else
					{
						selecting_x2=x;
						selecting_y2=y;
					}
					break;
					
				case t_wand:
					unfloat_selection();
					if(flip&1) x=15-x;
					
					if(flip&2) y=15-y;
					
					switch(select_mode)
					{
						case 0:
							clear_selection_grid();
							add_color_to_selection(unpackbuf[((y<<4)+x)]);
							break;
							
						case 1:
							add_color_to_selection(unpackbuf[((y<<4)+x)]);
							break;
							
						case 2:
							remove_color_from_selection(unpackbuf[((y<<4)+x)]);
							break;
							
						case 3:
							intersect_color_with_selection(unpackbuf[((y<<4)+x)]);
							break;
					}
					
					drawing=0;
					break;
			}
		}
		
		if(gui_mouse_b()==0)
		{
			bdown=false;
			drawing=0;
		}
		
		temp_x=(gui_mouse_x()-zoomtile.x)/zoomtile.xscale;
		temp_y=(gui_mouse_y()-zoomtile.y)/zoomtile.yscale;
		
		tile_x=temp_x;
		tile_y=temp_y;
		
		const char *toolnames[t_max]=
		{
			"Pen\nDraw Single Pixels", "Fill\nCtrl", "Replace Color\nCtrl+Alt", "Grab Color\nAlt", "Move\nMove Selections", "Select Pixels", "Select Color"
		};
		
		int32_t toolbtn = tool_btns.rectind(temp_mouse_x,temp_mouse_y);
		if(toolbtn > -1 && toolbtn < t_max)
		{
			int32_t column = toolbtn%tool_btns.w;
			int32_t row = toolbtn/tool_btns.w;
			
			update_tooltip(temp_mouse_x,temp_mouse_y,tool_btns.x+(column*tool_btns.xscale),tool_btns.y+(row*tool_btns.yscale),tool_btns.xscale,tool_btns.yscale, toolnames[toolbtn]);
		}
		/* Highlight the hovered pixel? Eh, maybe too much?
		int32_t hov_pix = zoomtile.rectind(temp_mouse_x,temp_mouse_y);
		if(hov_pix > -1)
		{
			int32_t column = hov_pix%zoomtile.w;
			int32_t row = hov_pix/zoomtile.w;
			
			update_tooltip(temp_mouse_x,temp_mouse_y,zoomtile.x+(column*zoomtile.xscale),zoomtile.y+(row*zoomtile.yscale),zoomtile.xscale,zoomtile.yscale, NULL);
		}*/
		
		draw_edit_scr(tile,flip,cs,oldtile, false);
		update_tool_cursor();
		
		draw_ttips(ttip,ttip_hl);
		update_hw_screen(true);
	}
	while(!done);
	
	unfloat_selection();
	clear_selection_grid();
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	if(done==1)
	{
		for(int32_t i=0; i<undocount; i++)
		{
			newtilebuf[tile].data[i]=oldtile[i];
		}
	}
	else
	{
		byte *buf = new byte[undocount];
		
		// put back old tile
		for(int32_t i=0; i<undocount; i++)
		{
			buf[i] = newtilebuf[tile].data[i];
			newtilebuf[tile].data[i] = oldtile[i];
		}
		
		// go
		go_tiles();
		
		// replace old tile with new one again
		for(int32_t i=0; i<undocount; i++)
		{
			newtilebuf[tile].data[i] = buf[i];
		}
		
		//   usetiles=true;
		saved=false;
		
		if(buf!=NULL)
		{
			delete[] buf;
		}
	}
	
	set_mouse_sprite(mouse_bmp[MOUSE_BMP_NORMAL][0]);
	register_blank_tiles();
	register_used_tiles();
	clear_tooltip();
	destroy_bitmap(selection_pattern);
	destroy_bitmap(selecting_pattern);
	destroy_bitmap(intersection_pattern);
	font = oldfont;
	a5font = oldfont_a5;
	popup_zqdialog_end();
	popup_zqdialog_end_a5();
}

/*  Grab Tile Code  */

enum recolorState { rcNone, rc4Bit, rc8Bit };

void *imagebuf=NULL;
int32_t imagesize=0;
int32_t tilecount=0;
int32_t  imagetype=0;
int32_t imagex,imagey,selx,sely;
int32_t bp=4,grabmode=16,romofs=0,romtilemode=0, romtilecols=8;
bool nesmode=false;
int32_t grabmask=0;
recolorState recolor=rcNone;
PALETTE imagepal;

/* bestfit_color:
  *  Searches a palette for the color closest to the requested R, G, B value.
  */
int32_t bestfit_cset_color(int32_t cs, int32_t r, int32_t g, int32_t b)
{
	int32_t bestMatch = 0; // Color with the lowest total difference so far
	float bestTotalDiff = 100000; // Total difference between requested color and bestMatch
	float bestHighDiff = 100000; // Greatest difference of R, G, B between requested color and bestMatch
	
	for(int32_t i = 0; i < CSET_SIZE; i++)
	{
		byte *rgbByte;
		
		// This seems to be right...
		if(cs==2 || cs==3 || cs==4)
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + (cs-2) + pdFULL) + i) * 3;
		else if(cs==9)
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + 3 + pdFULL) + i) * 3;
		else if(cs==1&&get_bit(quest_rules, qr_CSET1_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS) + i) * 3;
		else if(cs==5&&get_bit(quest_rules, qr_CSET5_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS + 1) + i) * 3;
		else if(cs==7&&get_bit(quest_rules, qr_CSET7_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS + 2) + i) * 3;
		else if(cs==8&&get_bit(quest_rules, qr_CSET8_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS + 3) + i) * 3;
		else
			rgbByte = colordata + (CSET(cs)+i)*3;
			
		int32_t dr=r-*rgbByte;
		int32_t dg=g-*(rgbByte+1);
		int32_t db=b-*(rgbByte+2);
		
		// Track both the total color difference and the single greatest
		// difference of R, G, B. The idea is that it's better to have
		// two or three small differences than one big one.
		// The differences are multiplied by different numbers to account
		// for the differences in perceived brightness of the three colors.
		float totalDiff = sqrt(dr*dr*0.241 + dg*dg*0.691 + db*db*0.068);
		float highDiff = zc_max(zc_max(sqrt(dr*dr*0.241), sqrt(dg*dg*0.691)), sqrt(db*db*0.068));
		
		// Perfect match? Just stop here.
		if(totalDiff==0)
			return i;
			
		if(totalDiff < bestTotalDiff || // Best match so far?
				(totalDiff == bestTotalDiff && highDiff < bestHighDiff)) // Equally good match with lower high difference?
		{
			bestMatch=i;
			bestTotalDiff=totalDiff;
			bestHighDiff=highDiff;
		}
	}
	
	return bestMatch;
}

// Same as the above, but draws from all colors in CSets 0-11.
int32_t bestfit_cset_color_8bit(int32_t r, int32_t g, int32_t b)
{
	int32_t bestMatch = 0;
	float bestTotalDiff = 100000;
	float bestHighDiff = 100000;
	
	for(int32_t i = 0; i < 192; i++) // 192 colors in CSets 0-11
	{
		byte *rgbByte;
		
		int32_t cs=i>>4;
		if(cs==2 || cs==3 || cs==4)
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + (cs-2) + pdFULL) + (i%16)) * 3;
		else if(cs==9)
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + 3 + pdFULL) + (i%16)) * 3;
		else if(cs==1&&get_bit(quest_rules, qr_CSET1_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS) + (i%16)) * 3;
		else if(cs==5&&get_bit(quest_rules, qr_CSET5_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS + 1) + (i%16)) * 3;
		else if(cs==7&&get_bit(quest_rules, qr_CSET7_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS + 2) + (i%16)) * 3;
		else if(cs==8&&get_bit(quest_rules, qr_CSET8_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS + 3) + (i%16)) * 3;
		else
			rgbByte = colordata + i * 3;
			
		int32_t dr=r-*rgbByte;
		int32_t dg=g-*(rgbByte+1);
		int32_t db=b-*(rgbByte+2);
		
		float totalDiff = sqrt(dr*dr*0.241 + dg*dg*0.691 + db*db*0.068);
		float highDiff = zc_max(zc_max(sqrt(dr*dr*0.241), sqrt(dg*dg*0.691)), sqrt(db*db*0.068));
		
		if(totalDiff==0) // Perfect match?
			return i;
			
		if(totalDiff < bestTotalDiff || // Best match so far?
		   (totalDiff == bestTotalDiff && highDiff < bestHighDiff)) // Equally good match with lower high difference?
		{
			bestMatch=i;
			bestTotalDiff=totalDiff;
			bestHighDiff=highDiff;
		}
	}
	
	return bestMatch;
}

byte cset_reduce_table[PAL_SIZE];

void calc_cset_reduce_table(PALETTE pal, int32_t cs)
{
	for(int32_t i=0; i<PAL_SIZE; i++)
	{
		cset_reduce_table[i]=(bestfit_cset_color(cs, pal[i].r, pal[i].g, pal[i].b)&0x0F);
	}
}

void calc_cset_reduce_table_8bit(PALETTE pal)
{
	for(int32_t i=0; i<PAL_SIZE; i++)
	{
		cset_reduce_table[i]=bestfit_cset_color_8bit(pal[i].r, pal[i].g, pal[i].b);
	}
}

void puttileROM(BITMAP *dest,int32_t x,int32_t y,byte *src,int32_t cs)
{
	//storage space for the grabbed image
	byte buf[64];
	memset(buf,0,64);
	byte *oldsrc=src;
	
	//for 8 lines in the source image...
	for(int32_t line=0; line<(nesmode?4:8); line++)
	{
		//bx is the pixel at the start of a line in the storage buffer
		int32_t  bx=line<<(nesmode?4:3);
		//b is a byte in the source image (either an entire line in 1bp or the start of a line in others)
		byte b=src[(bp&1)?line:line<<1];
		
		//fill the storage buffer with data from the source image
		for(int32_t i=7; i>=0; --i)
		{
			buf[bx+i] = (b&1)+(cs<<4);
			b>>=1;
		}
	}
	
	++src;
	
	for(int32_t p=1; p<bp; p++)
	{
		for(int32_t line=0; line<(nesmode?4:8); line++)
		{
			int32_t  bx=line<<(nesmode?4:3);
			byte b=src[(bp&1)?line:line<<1];
			
			for(int32_t i=7; i>=0; --i)
			{
				if(nesmode)
				{
					buf[bx+8+i] = (b&1)+(cs<<4);
				}
				else
				{
					buf[bx+i] |= (b&1)<<p;
				}
				
				b>>=1;
			}
		}
		
		if(p&1)
		{
			src+=15;
		}
		else
		{
			++src;
		}
	}
	
	
	if(nesmode)
	{
		src=oldsrc;
		
		for(int32_t counter=0; counter<2; ++counter, ++src)
		{
			//for 8 lines in the source image...
			for(int32_t line=0; line<4; line++)
			{
				//bx is the pixel at the start of a line in the storage buffer
				int32_t  bx=line<<4;
				//b is a byte in the source image (either an entire line in 1bp or the start of a line in others)
				byte b=src[(line+4)<<1];
				
				//fill the storage buffer with data from the source image
				for(int32_t i=7; i>=0; --i)
				{
					//        buf[bx+i] = (b&1)+(cs<<4);
					buf[bx+(counter<<3)+i] |= (b&1)<<1;
					b>>=1;
				}
			}
		}
	}
	
	int32_t c=0;
	
	switch(romtilemode)
	{
	case 0:
	case 1:
	case 2:
		for(int32_t j=0; j<8; j++)
		{
			for(int32_t i=0; i<8; i++)
			{
				putpixel(dest,x+i,y+j,buf[c++]);
			}
		}
		
		break;
		
	case 3:
		for(int32_t j=0; j<4; j++)
		{
			for(int32_t i=0; i<16; i++)
			{
				putpixel(dest,x+i,y+j,buf[c++]);
			}
		}
		
		break;
	}
}

const char *file_type[ftMAX]=
{
	"None", "BIN", "BMP", "TIL", "ZGP", "QSU", "ZQT", "QST"
};

void draw_grab_window()
{
	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-25-6)>>1;
	scare_mouse();
	jwin_draw_win(screen, window_xofs, window_yofs, w+6+6, h+25+6, FR_WIN);
	jwin_draw_frame(screen, window_xofs+4, window_yofs+23, w+2+2, h+2+2-(82*2),  FR_DEEP);
	
	FONT *oldfont = font;
	font = lfont;
	jwin_draw_titlebar(screen, window_xofs+3, window_yofs+3, w+6, 18, "Grab Tile(s)", true);
	font=oldfont;
	unscare_mouse();
	return;
}

void draw_grab_scr(int32_t tile,int32_t cs,byte *newtile,int32_t black,int32_t white, int32_t width, int32_t height, byte *newformat)
{
	width=width;
	height=height;
	white=white; // happy birthday compiler
	
	int32_t yofs=0;
	//clear_to_color(screen2,bg);
	rectfill(screen2, 0, 0, 319, 159, black);
	//jwin_draw_win(screen2, 0, 160, 320, 80, FR_WIN);
	
	rectfill(screen2,0,160,319,239,jwin_pal[jcBOX]);
	_allegro_hline(screen2, 0, 158, 319, jwin_pal[jcMEDLT]);
	_allegro_hline(screen2, 0, 159, 319, jwin_pal[jcLIGHT]);
	yofs=3;
	
	// text_mode(-1);
	int32_t tileromcolumns=20;
	
	switch(imagetype)
	{
	case ftBMP:
		if(recolor==rcNone)
		{
			blit((BITMAP*)imagebuf,screen2,imagex<<4,imagey<<4,0,0,320,160);
		}
		else
		{
			int32_t maxy=zc_min(160,((BITMAP*)imagebuf)->h);
			int32_t maxx=zc_min(320,((BITMAP*)imagebuf)->w);
			
			for(int32_t y=0; y<maxy; y++)
			{
				if((imagey<<4)+y>=((BITMAP*)imagebuf)->h)
				{
					break;
				}
				
				for(int32_t x=0; x<maxx; x++)
				{
					if((imagex<<4)+x>=((BITMAP*)imagebuf)->w)
					{
						break;
					}
					
					if(recolor==rc8Bit)
						screen2->line[y][x]=cset_reduce_table[((BITMAP*)imagebuf)->line[(imagey<<4)+y][(imagex<<4)+x]];
					else
						screen2->line[y][x]=(cset_reduce_table[((BITMAP*)imagebuf)->line[(imagey<<4)+y][(imagex<<4)+x]])+(cs<<4);
				}
			}
		}
		
		break;
		
	case ftZGP:
	case ftQST:
	case ftZQT:
	case ftQSU:
	case ftTIL:
	{
		tiledata *hold = newtilebuf;
		newtilebuf = grabtilebuf;
		//fixme
	imagey = vbound(imagey, 0, MAXTILEROWS); //fixed -Z This can no longer crash if you scroll past the end of the tile pages. 6th June, 2020
		int32_t t=imagey*TILES_PER_ROW;
		
		for(int32_t i=0; i<200; i++)                              // 10 rows, down to y=160
		{
			if(t <= tilecount)
			{
				puttile16(screen2,t,(i%TILES_PER_ROW)<<4,(i/TILES_PER_ROW)<<4,cs,0);
			}
			
			++t;
		}
		
		newtilebuf = hold;
		//fixme
	}
	break;
	
	case ftBIN:
	{
		int32_t ofs = (tileromcolumns*imagex + imagey) * 128*bp + romofs;
		byte *buf = (byte*)imagebuf;
		
		switch(romtilemode)
		{
		case 0:
			for(int32_t y=0; y<160; y+=8)
			{
				for(int32_t x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=8)
				{
					puttileROM(screen2,x,y,buf+ofs,cs);
					ofs+=8*bp;
				}
			}
			
			for(int32_t y=0; y<160; y+=8)
			{
				for(int32_t x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=8)
				{
					puttileROM(screen2,x+128,y,buf+ofs,cs);
					ofs+=8*bp;
				}
			}
			
			break;
			
		case 1:
			for(int32_t y=0; y<160; y+=16)
			{
				for(int32_t x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=8)
				{
					puttileROM(screen2,x,y,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x,y+8,buf+ofs,cs);
					ofs+=8*bp;
				}
			}
			
			for(int32_t y=0; y<160; y+=16)
			{
				for(int32_t x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=8)
				{
					puttileROM(screen2,x+128,y,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x+128,y+8,buf+ofs,cs);
					ofs+=8*bp;
				}
			}
			
			break;
			
		case 2:
			for(int32_t y=0; y<160; y+=16)
			{
				for(int32_t x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=16)
				{
					puttileROM(screen2,x,y,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x+8,y,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x,y+8,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x+8,y+8,buf+ofs,cs);
					ofs+=8*bp;
				}
			}
			
			for(int32_t y=0; y<160; y+=16)
			{
				for(int32_t x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=16)
				{
					puttileROM(screen2,x+128,y,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x+136,y,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x+128,y+8,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x+136,y+8,buf+ofs,cs);
					ofs+=8*bp;
				}
			}
			
			break;
			
		case 3:
			for(int32_t y=0; y<160; y+=16)
			{
				for(int32_t x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=16)
				{
					puttileROM(screen2,x,y,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x,y+4,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x,y+8,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x,y+12,buf+ofs,cs);
					ofs+=8*bp;
				}
			}
			
			for(int32_t y=0; y<160; y+=16)
			{
				for(int32_t x=0; ((x<128)&&(ofs<=imagesize-8*bp)); x+=16)
				{
					puttileROM(screen2,x+128,y,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x+128,y+4,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x+128,y+8,buf+ofs,cs);
					ofs+=8*bp;
					puttileROM(screen2,x+128,y+12,buf+ofs,cs);
					ofs+=8*bp;
				}
			}
			
			break;
		}
	}
	break;
	}
	
	tiledata hold;
	
	if(is_valid_format(newtilebuf[0].format))
	{
		hold.format = newtilebuf[0].format;
		hold.data = (byte *)malloc(tilesize(hold.format));
		memcpy(hold.data, newtilebuf[0].data, tilesize(hold.format));
	}
	else
	{
		hold.format=tfInvalid;
		hold.data=NULL;
	}
	
	newtilebuf[0].format=newformat[0];
	
	if(newtilebuf[0].data!=NULL)
	{
		free(newtilebuf[0].data);
	}
	
	if(is_valid_format(newtilebuf[0].format))
	{
		newtilebuf[0].data = (byte *)malloc(tilesize(newtilebuf[0].format));
		
		for(int32_t i=0; i<tilesize(newtilebuf[0].format); i++)
		{
			newtilebuf[0].data[i]=newtile[i];
		}
	}
	else
	{
		newtilebuf[0].data=NULL;
	}
	
	puttile16(screen2,0,208,168+yofs,cs,0);
	overtile16(screen2,0,232,168+yofs,cs,0);
	newtilebuf[0].format=hold.format;
	
	if(newtilebuf[0].data!=NULL)
	{
		free(newtilebuf[0].data);
	}
	
	if(is_valid_format(newtilebuf[0].format))
	{
		newtilebuf[0].data = (byte *)malloc(tilesize(newtilebuf[0].format));
		
		for(int32_t i=0; i<newtilebuf[0].format*128; i++)
		{
			newtilebuf[0].data[i]=hold.data[i];
		}
	}
	else
	{
		newtilebuf[0].data=NULL;
	}
	
	if(hold.data!=NULL)
	{
		free(hold.data);
	}
	
	puttile16(screen2,tile,208,192+yofs,cs,0);
	overtile16(screen2,tile,232,192+yofs,cs,0);
	
	rectfill(screen2,184,168+yofs,191,175+yofs,grabmask&1?vc(12):vc(7));
	rectfill(screen2,192,168+yofs,199,175+yofs,grabmask&2?vc(12):vc(7));
	rectfill(screen2,184,176+yofs,191,183+yofs,grabmask&4?vc(12):vc(7));
	rectfill(screen2,192,176+yofs,199,183+yofs,grabmask&8?vc(12):vc(7));
	
	// rect(screen2,183,167,200,184,dvc(7*2));
	// rect(screen2,207,167,224,184,dvc(7*2));
	// rect(screen2,231,167,248,184,dvc(7*2));
	// rect(screen2,207,191,224,208,dvc(7*2));
	// rect(screen2,231,191,248,208,dvc(7*2));
	
	/*
	  rect(screen2,183,167,200,184,vc(14));
	  rect(screen2,207,167,224,184,vc(14));
	  rect(screen2,231,167,248,184,vc(14));
	  rect(screen2,207,191,224,208,vc(14));
	  rect(screen2,231,191,248,208,vc(14));
	*/
	jwin_draw_frame(screen2,182,166+yofs,20,20,FR_DEEP);
	jwin_draw_frame(screen2,206,166+yofs,20,20,FR_DEEP);
	jwin_draw_frame(screen2,230,166+yofs,20,20,FR_DEEP);
	jwin_draw_frame(screen2,206,190+yofs,20,20,FR_DEEP);
	jwin_draw_frame(screen2,230,190+yofs,20,20,FR_DEEP);
	int32_t window_xofs=(zq_screen_w-640-12)>>1;
	int32_t window_yofs=(zq_screen_h-480-25-6)>>1;
	int32_t screen_xofs=window_xofs+6;
	int32_t screen_yofs=window_yofs+25;
	int32_t mul = 2;
	
	yofs=16;
	
	custom_vsync();
	scare_mouse();
	
	stretch_blit(screen2,screen,0,0,320,240,screen_xofs,screen_yofs,640,480);
	
	// Suspend the current font while draw_text_button does its work
	FONT* oldfont = font;
	
	font = lfont_l;
		
	// Interface
	switch(imagetype)
	{
	case 0:
		textprintf_ex(screen,font,window_xofs+8*mul,window_yofs+(216+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%s",imgstr[imagetype]);
		break;
		
	case ftBMP:
	{
		textprintf_ex(screen,font,window_xofs+8*mul,window_yofs+(216+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%s  %dx%d",imgstr[imagetype],((BITMAP*)imagebuf)->w,((BITMAP*)imagebuf)->h);
		draw_text_button(screen,window_xofs+117*mul,window_yofs+(192+yofs)*mul,int32_t(61*(1.5)),int32_t(20*(1.5)),"Recolor",vc(1),vc(14),0,true);
		break;
	}
	
	case ftZGP:
	case ftQST:
	case ftZQT:
	case ftQSU:
	case ftTIL:
	case ftBIN:
		textprintf_ex(screen,lfont_l,window_xofs+8*mul,window_yofs+(216+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%s  %d KB",imgstr[imagetype],imagesize>>10);
		break;
	}
	
	textprintf_ex(screen,font,window_xofs+8*mul,window_yofs+(168+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"sel: %d %d",selx,sely);
	textprintf_ex(screen,font,window_xofs+8*mul,window_yofs+(176+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"pos: %d %d",imagex,imagey);
	
	if(bp==8)
		textprintf_ex(screen,font,window_xofs+8*mul,window_yofs+(192+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"8-bit");
	else
		textprintf_ex(screen,font,window_xofs+8*mul,window_yofs+(192+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"cset: %d",cs);
	textprintf_ex(screen,font,window_xofs+8*mul,window_yofs+(200+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"step: %d",grabmode);
	
	if(imagetype==ftBIN)
	{
		textprintf_ex(screen,font,window_xofs+104*mul,window_yofs+(192+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"bp:  %d%s",bp,nesmode?" (NES)":"");
		textprintf_ex(screen,font,window_xofs+104*mul,window_yofs+(200+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"ofs: %Xh",romofs);
		textprintf_ex(screen,font,window_xofs+104*mul,window_yofs+(208+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"m: %d",romtilemode);
	}
	
	textprintf_ex(screen,font,window_xofs+8*mul,window_yofs+(224+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%s",imagepath);
//  rectfill(screen2,256,224,319,231,black);
	draw_text_button(screen,window_xofs+255*mul,window_yofs+(168+yofs)*mul,int32_t(61*(1.5)),int32_t(20*(1.5)),"OK",vc(1),vc(14),0,true);
	draw_text_button(screen,window_xofs+255*mul,window_yofs+(192+yofs)*mul,int32_t(61*(1.5)),int32_t(20*(1.5)),"Cancel",vc(1),vc(14),0,true);
	draw_text_button(screen,window_xofs+255*mul,window_yofs+(216+yofs)*mul,int32_t(61*(1.5)),int32_t(20*(1.5)),"File",vc(1),vc(14),0,true);
	draw_text_button(screen,window_xofs+117*mul,window_yofs+(166+yofs)*mul,int32_t(61*(1.5)),int32_t(20*(1.5)),"Leech",vc(1),vc(14),0,true);
	
	//int32_t rectw = 16*mul;
	//rect(screen,selx+screen_xofs,sely+screen_yofs,selx+screen_xofs+((width-1)*rectw)+rectw-1,sely+screen_yofs+((height-1)*rectw)+rectw-1,white);
	unscare_mouse();
	SCRFIX();
	font = oldfont;
}

RGB_MAP rgb_table;
COLOR_MAP imagepal_table;


extern void return_RAMpal_color(AL_CONST PALETTE pal, int32_t x, int32_t y, RGB *rgb)
{
	//these are here to bypass compiler warnings about unused arguments
	x=x;
	
	rgb->r = pal[y].r;
	rgb->g = pal[y].g;
	rgb->b = pal[y].b;
}


void load_imagebuf()
{
	PACKFILE *f;
	//cache QRS
	//byte cached_rules[QUESTRULES_NEW_SIZE] = { 0 };
	//for ( int32_t q = 0; q < QUESTRULES_NEW_SIZE; ++q )
	// { 
	//	cached_rules[q] = quest_rules[q];
	// }
	bool compressed=false;
	bool encrypted=false;
	tiledata *hold=newtilebuf;
	zquestheader tempheader;
	memset(&tempheader, 0, sizeof(zquestheader));
	
	if(imagebuf)
	{
		switch(imagetype)
		{
		case ftBMP:
			destroy_bitmap((BITMAP*)imagebuf);
			break;
			
		case ftZGP:
		case ftQST:
		case ftZQT:
		case ftQSU:
		case ftTIL:
			clear_tiles(grabtilebuf);
			break;
			
		case ftBIN:
			free(imagebuf);
			break;
		}
		
		imagebuf=NULL;
	}
	
	selx=sely=romofs=0;
	bp=4;
	imagetype=filetype(imagepath);
	
	dword section_id;
	dword section_version;
	dword section_cversion;
	
	switch(imagetype)
	{
	case ftBMP:
	packfile_password("");
		memset(imagepal, 0, sizeof(PALETTE));
		imagebuf = load_bitmap(imagepath,imagepal);
		imagesize = file_size_ex_password(imagepath,"");
		tilecount=0;
		create_rgb_table(&rgb_table, imagepal, NULL);
		rgb_map = &rgb_table;
		create_color_table(&imagepal_table, RAMpal, return_RAMpal_color, NULL);
		
		if(!imagebuf)
		{
			imagetype=0;
		}
		
		break;
		
	case ftBIN:
	packfile_password("");
		imagesize = file_size_ex_password(imagepath, "");
		tilecount=0;
		
		if(imagesize)
		{
			imagebuf = malloc(imagesize);
			
			if(!readfile(imagepath,imagebuf,imagesize))
			{
				free(imagebuf);
				imagesize=0;
				imagetype=0;
			}
		}
		
		break;
		
	case ftTIL:
	packfile_password("");
		imagesize = file_size_ex_password(imagepath,"");
		f = pack_fopen_password(imagepath,F_READ,"");
		
		if(!f)
		{
			goto error;
		}
		
		if(!p_mgetl(&section_id,f,true))
		{
			goto error;
		}
		
		if(section_id==ID_TILES)
		{
			if(readtiles(f, grabtilebuf, NULL, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES, false, true)==0)
			{
				goto error;
			}
		}
		
error:
		pack_fclose(f);
		tilecount=count_tiles(grabtilebuf);
		break;
		
	case ftZGP:
	packfile_password("");
		imagesize = file_size_ex_password(imagepath, "");
		f=pack_fopen_password(imagepath,F_READ,"");
		
		if(!f)
		{
			goto error2;
		}
		
		if(!p_mgetl(&section_id,f,true))
		{
			goto error2;
		}
		
		if(section_id!=ID_GRAPHICSPACK)
		{
			goto error2;
		}
		
		//section version info
		if(!p_igetw(&section_version,f,true))
		{
			goto error2;
		}
		
		if(!p_igetw(&section_cversion,f,true))
		{
			goto error2;
		}
		
		//tiles
		if(!p_mgetl(&section_id,f,true))
		{
			goto error2;
		}
		
		if(section_id==ID_TILES)
		{
			if(readtiles(f, grabtilebuf, NULL, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES, false, true)!=0)
			{
				goto error2;
			}
		}
		
error2:
		pack_fclose(f);
		tilecount=count_tiles(grabtilebuf);
		break;
		
	case ftQST:
	packfile_password("");
		encrypted=true;
		
	case ftZQT:
	packfile_password("");
		compressed=true;
		
	case ftQSU:
	packfile_password("");
		imagesize = file_size_ex_password(imagepath, encrypted ? datapwd : "");
		newtilebuf=grabtilebuf;
		byte skip_flags[4];
		
		for(int32_t i=0; i<skip_max; ++i)
		{
			set_bit(skip_flags,i,1);
		}
		
		set_bit(skip_flags,skip_tiles,0);
		set_bit(skip_flags,skip_header,0);
		//if(encrypted)
		//	  setPackfilePassword(datapwd);
		loadquest(imagepath,&tempheader,&misc,customtunes,false,compressed,encrypted,true,skip_flags);
		//loadquest(imagepath,&tempheader,&misc,customtunes,false,compressed,encrypted,false,skip_flags);
	//fails to keep quest password data / header
		
		if(encrypted&&compressed)
		{
			if(quest_access(imagepath, &tempheader, compressed) != 1)
			{
				imagetype=0;
				imagesize=0;
				clear_tiles(grabtilebuf);
				chop_path(imagepath);
			}
		}
		
		//setPackfilePassword(NULL);
		newtilebuf=hold;
		tilecount=count_tiles(grabtilebuf);
		break;
	}
	
	rgb_map = &zq_rgb_table;
	//restore cashed QRs / rules
	
	//for ( int32_t q = 0; q < QUESTRULES_NEW_SIZE; ++q )
	// { 
	//	quest_rules[q] = cached_rules[q];
	// }
}

static char bitstrbuf[32];
bool leeching_from_tiles=false;

const char *bitlist(int32_t index, int32_t *list_size)
{
	int32_t imported=2;
	
	if(index>=0)
	{
		bound(index,0,leeching_from_tiles?2:1);
		
		if(index==imported)
		{
			sprintf(bitstrbuf,"Imported");
		}
		else
		{
			sprintf(bitstrbuf,"%d",4<<index);
		}
		
		return bitstrbuf;
	}
	
	*list_size=leeching_from_tiles?3:2;
	return NULL;
}

static ListData bit_list(bitlist, &font);

static DIALOG leech_dlg[] =
{
	/* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
	{ jwin_win_proc,       8,    20-4,   303+1,  216-42+1,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Leech Options", NULL, NULL },
	{ d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
	// 2
	{ jwin_button_proc,     180,  210-42-4,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
	{ jwin_button_proc,     80,   210-42-4,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
	{ d_keyboard_proc,   0,    0,    0,    0,    0,       0,      0,       0,          KEY_F1,        0, (void *) onHelp, NULL, NULL },
	// 5
	{ jwin_text_proc,       14,   49-4,  176,   8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "Update Status Every:  ", NULL, NULL },
	{ jwin_edit_proc,      114,   45-4,   36,   16,    vc(12),  vc(1),  0,       0,          5,             0,       NULL, NULL, NULL },
	{ jwin_radio_proc,     155,   49-4,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Tiles", NULL, NULL },
	{ jwin_radio_proc,     200,   49-4,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Seconds", NULL, NULL },
	//9
	{ jwin_frame_proc,      14,   63-2,   176+70,  50+30,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          FR_ETCHED,             0,       NULL, NULL, NULL },
	{ jwin_text_proc,       14+8, 60-2,   80,   8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) " Duplicates ", NULL, NULL },
	
	{ jwin_check_proc,       20,   70,  168,   8+1,    vc(15),  vc(1),  0,       0,          1,             0, (void *) "Only check new tiles", NULL, NULL },
	{ jwin_text_proc,       20,   90,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "Normal:", NULL, NULL },
	{ jwin_text_proc,       20,  100,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "Horizontal Flip:", NULL, NULL },
	{ jwin_text_proc,       20,  110,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "Vertical Flip:", NULL, NULL },
	{ jwin_text_proc,       20,  120,   72,   8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "Horizontal/Vertical Flip:", NULL, NULL },
	//16
	{ jwin_radio_proc,      144,  90,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },
	{ jwin_radio_proc,      184,  90,   56+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },
	{ jwin_radio_proc,      224,  90,   72+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },
	
	{ jwin_radio_proc,      144, 100,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	{ jwin_radio_proc,      184, 100,   56+1,   8+1,    vc(14),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	{ jwin_radio_proc,      224, 100,   72+1,   8+1,    vc(14),  vc(1),  0,       0,          2,             0,       NULL, NULL, NULL },
	
	{ jwin_radio_proc,      144, 110,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_radio_proc,      184, 110,   56+1,   8+1,    vc(14),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	{ jwin_radio_proc,      224, 110,   72+1,   8+1,    vc(14),  vc(1),  0,       0,          3,             0,       NULL, NULL, NULL },
	
	{ jwin_radio_proc,      144, 120,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },
	{ jwin_radio_proc,      184, 120,   56+1,   8+1,    vc(14),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },
	{ jwin_radio_proc,      224, 120,   72+1,   8+1,    vc(14),  vc(1),  0,       0,          4,             0,       NULL, NULL, NULL },
	
	{ jwin_ctext_proc,      144+4,  80,   64+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Ignore", NULL, NULL },
	{ jwin_ctext_proc,      184+4,  80,   56+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Check", NULL, NULL },
	{ jwin_ctext_proc,      224+4,  80,   72+1,   8+1,    vc(14),  vc(1),  0,       0,          0,             0, (void *) "Discard", NULL, NULL },
	{ jwin_droplist_proc,   76,   145,   80,   16,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       0,          1,             0, (void *) &bit_list, NULL, NULL },
	{ jwin_text_proc,       14,   149,  60,   8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "Color Depth: ", NULL, NULL },
	{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

bool leech_tiles(tiledata *dest,int32_t start,int32_t cs)
{
	bool shift=true; // fix this!
	int32_t cst=0;
	int32_t currtile=start;
	int32_t height=0, width=0;
	byte *testtile = new byte[tilesize(tf32Bit)];
	byte imported_format=0;
	char updatestring[6];
	bool canadd;
	bool temp_canadd;
	bool duplicate;
	int32_t total_duplicates_found=0, total_duplicates_discarded=0;
	int32_t duplicates_found[4]=                                  //, duplicates_discarded[4]={0,0,0,0};
	{
		0,0,0,0
	};
	BITMAP *status;
	status = create_bitmap_ex(8,240,140);
	clear_bitmap(status);
	sprintf(updatestring, "%d", LeechUpdate);
	leech_dlg[0].dp2=lfont;
	leech_dlg[6].dp=updatestring;
	
	leech_dlg[10].flags=(OnlyCheckNewTilesForDuplicates!=0) ? D_SELECTED : 0;
	
	for(int32_t i=0; i<2; i++)
	{
		leech_dlg[i+7].flags=0;
	}
	
	leech_dlg[7+((LeechUpdateTiles==0) ? 1 : 0)].flags=D_SELECTED;
	
	for(int32_t i=0; i<12; i++)
	{
		leech_dlg[i+16].flags=0;
	}
	
	for(int32_t i=0; i<4; i++)
	{
		leech_dlg[(DuplicateAction[i])+16+(i*3)].flags=D_SELECTED;
	}
	
	leech_dlg[31].d1=0;
	
	large_dialog(leech_dlg);
		
	int32_t ret = zc_popup_dialog(leech_dlg,3);
	
	if(ret==2)
	{
		delete[] testtile;
		return false;
	}
	
	int32_t cdepth=leech_dlg[31].d1+1;
	int32_t newformat=0;
	auto lu = atoi(updatestring);
	auto lut = (leech_dlg[7].flags&D_SELECTED)?1:0;
	if(LeechUpdate!=lu)
	{
		LeechUpdate=lu;
		zc_set_config("zquest","leech_update",LeechUpdate);
	}
	if(LeechUpdateTiles!=lut)
	{
		LeechUpdateTiles=lut;
		zc_set_config("zquest","leech_update_tiles",LeechUpdateTiles);
	}
	
	int32_t old_dupe[4];
	for(int32_t j=0; j<4; j++)
	{
		old_dupe[j] = DuplicateAction[j];
		for(int32_t i=0; i<3; i++)
		{
			if(leech_dlg[i+16+(j*3)].flags&D_SELECTED)
			{
				DuplicateAction[j]=i;
			}
		}
	}
	if(old_dupe[0] != DuplicateAction[0])
		zc_set_config("zquest","normal_duplicate_action",DuplicateAction[0]);
	if(old_dupe[1] != DuplicateAction[1])
		zc_set_config("zquest","horizontal_duplicate_action",DuplicateAction[1]);
	if(old_dupe[2] != DuplicateAction[2])
		zc_set_config("zquest","vertical_duplicate_action",DuplicateAction[2]);
	if(old_dupe[3] != DuplicateAction[3])
		zc_set_config("zquest","both_duplicate_action",DuplicateAction[3]);
	
	auto ocntfd = leech_dlg[10].flags&D_SELECTED?1:0;
	if(OnlyCheckNewTilesForDuplicates!=ocntfd)
	{
		OnlyCheckNewTilesForDuplicates=ocntfd;
		zc_set_config("zquest","only_check_new_tiles_for_duplicates",ocntfd);
	}
	
	leeching_from_tiles=false;
	
	switch(imagetype)
	{
	case ftBIN:
		width=imagesize/128;
		height=1;
		break;
		
	case ftZGP:
	case ftQST:
	case ftZQT:
	case ftQSU:
	case ftTIL:
		leeching_from_tiles=true;
		width=count_tiles(grabtilebuf);
		height=1;
		break;
		
	case ftBMP:
		width=((((BITMAP*)imagebuf)->w)+15)/16;
		height=((((BITMAP*)imagebuf)->h)+15)/16;
		break;
	}
	
	if(currtile+(width*height)>NEWMAXTILES)
	{
		if(jwin_alert("Confirm Truncation","Too many tiles.","Truncation may occur.",NULL,"&OK","&Cancel",'o','c',lfont)==2)
		{
			delete[] testtile;
			return false;
		}
	}
	
	go_tiles();
	saved=false;
	
	//  usetiles=true;
	for(int32_t ty=0; ty<height; ty++)                            //for every row
	{
		for(int32_t tx=0; tx<width; tx++)                           //for every column (tile)
		{
			if((((ty*width)+tx)%zc_max(LeechUpdate, 1))==0)                  //update status
			{
				FONT *oldfont = font;
				static BITMAP *tbar = create_bitmap_ex(8,240-6, 18);
				static bool created_tbar=false;
				jwin_draw_win(status, 0, 0, 240, 140, FR_WIN);
				
				if(created_tbar)
				{
					blit(tbar, status, 0, 0, 3, 3, 240-6, 18);
				}
				else
				{
					font = lfont;
					jwin_draw_titlebar(tbar, 0, 0, 240-6, 18, "Leech Status", false);
					font = oldfont;
					created_tbar=true;
					blit(tbar, status, 0, 0, 3, 3, 320-6, 18);
				}
				
				textprintf_centre_ex(status,font,120,24,jwin_pal[jcTEXTFG],-1,"Checking %d of %d",((ty*width)+tx), (width*height));
				textprintf_centre_ex(status,font,120,34,jwin_pal[jcTEXTFG],-1,"%d tiles imported",currtile-start);
				jwin_draw_frame(status, 40, 49, 160, 70, FR_ETCHED);
				textprintf_centre_ex(status,font,120,46,jwin_pal[jcTEXTFG],jwin_pal[jcBOX]," Duplicates ");
				textprintf_centre_ex(status,font,120,56,jwin_pal[jcTEXTFG],-1,"%d/%d found/discarded",total_duplicates_found, total_duplicates_discarded);
				textprintf_centre_ex(status,font,120,76,jwin_pal[jcTEXTFG],-1,"%d normal %s",duplicates_found[0],((DuplicateAction[0]<2)?"found":"discarded"));
				textprintf_centre_ex(status,font,120,86,jwin_pal[jcTEXTFG],-1,"%d flipped (h) %s",duplicates_found[1],((DuplicateAction[1]<2)?"found":"discarded"));
				textprintf_centre_ex(status,font,120,96,jwin_pal[jcTEXTFG],-1,"%d flipped (v) %s",duplicates_found[2],((DuplicateAction[2]<2)?"found":"discarded"));
				textprintf_centre_ex(status,font,120,106,jwin_pal[jcTEXTFG],-1,"%d flipped (hv) %s",duplicates_found[3],((DuplicateAction[3]<2)?"found":"discarded"));
				textprintf_centre_ex(status,font,120,128,jwin_pal[jcTEXTFG],-1,"Press any key to stop.");
				scare_mouse();
				blit(status,screen,0, 0, 40, 20, 240, 140);
				unscare_mouse();
				SCRFIX();
			}
			
			canadd=true;
			
			if(currtile>=NEWMAXTILES)                             //if we've maxed out on our tiles...
			{
				delete[] testtile;
				return true;
			}
			
			switch(imagetype)
			{
			case ftBIN:
				break;
				
			case ftZGP:
			case ftQST:
			case ftZQT:
			case ftQSU:
			case ftTIL:
				memset(testtile, 0, tilesize(tf32Bit));
				imported_format=grabtilebuf[tx].format;
				
				switch(cdepth)
				{
				case 1:  //4-bit
					newformat=tf4Bit;
					
					switch(imported_format)
					{
					case tf4Bit:
						memcpy(testtile,grabtilebuf[tx].data,tilesize(imported_format));
						break;
						
					case tf8Bit:
						for(int32_t y=0; y<16; y++)                           //snag a tile
						{
							for(int32_t x=0; x<16; x+=2)
							{
								testtile[(y*8)+(x/2)]=
									(grabtilebuf[tx].data[y*16+x]&15)+
									((grabtilebuf[tx].data[y*16+x+1]&15)<<4);
							}
						}
						
						break;
					}
					
					break;
					
				case 2:  //8-bit
					newformat=tf8Bit;
					
					switch(imported_format)
					{
					case tf4Bit:
						unpack_tile(grabtilebuf, tx, 0, true);
						cst = cs&15;
						cst <<= CSET_SHFT;
						
						for(int32_t i=0; i<256; i++)
						{
							if(!shift||unpackbuf[i]!=0)
							{
								unpackbuf[i]+=cst;
							}
						}
						
						pack_tiledata(testtile, unpackbuf, tf8Bit);
						break;
						
					case tf8Bit:
						memcpy(testtile,grabtilebuf[tx].data,tilesize(imported_format));
						break;
					}
					
					break;
					
				case 3:  //original tile's bit depth
					newformat=imported_format;
					memcpy(testtile,grabtilebuf[tx].data,tilesize(imported_format));
					break;
				}
				
				break;
				
			case ftBMP:
				newformat=cdepth;
				
				for(int32_t y=0; y<16; y++)                           //snag a tile
				{
					for(int32_t x=0; x<16; x+=2)
					{
						switch(cdepth)
						{
						case tf4Bit:
							testtile[(y*8)+(x/2)]=
								(getpixel(((BITMAP*)imagebuf),(tx*16)+x,(ty*16)+y)&15)+
								((getpixel(((BITMAP*)imagebuf),(tx*16)+x+1,(ty*16)+y)&15)<<4);
							break;
							
						case tf8Bit:
							testtile[(y*16)+x]=getpixel(((BITMAP*)imagebuf),(tx*16)+x,(ty*16)+y);
							testtile[(y*16)+x+1]=getpixel(((BITMAP*)imagebuf),(tx*16)+x+1,(ty*16)+y);
							break;
						}
					}
				}
				
				break;
			}
			
			if(DuplicateAction[0]+DuplicateAction[1]+DuplicateAction[2]+DuplicateAction[3]>0)
			{
				temp_canadd=true;
				
				//check all tiles before this one
				for(int32_t checktile=((OnlyCheckNewTilesForDuplicates!=0)?start:0); ((temp_canadd==true)&&(checktile<currtile)); checktile++)
				{
					for(int32_t flipping=0; ((temp_canadd==true)&&(flipping<4)); ++flipping)
					{
						if(DuplicateAction[flipping]>0)
						{
							if(keypressed())
							{
								delete[] testtile;
								return true;
							}
							
							duplicate=(newformat==imported_format);
							
							if(duplicate)
							{
								switch(flipping)
								{
								case 0:                                     //normal
									if(dest[checktile].data!=NULL)
									{
										for(int32_t y=0; ((duplicate==true)&&(y<16)); y++)
										{
											for(int32_t x=0; ((duplicate==true)&&(x<16)); x+=3-newformat)
											{
												//                        if ((dest[(checktile*128)+(y*8)+(x/2)])!=(testtile[(y*8)+(x/2)]))
												if((dest[checktile].data[(y*8*newformat)+(x/(3-newformat))])!=(newformat==tf4Bit?(testtile[(y*8)+(x/2)]):(testtile[(y*16)+x])))
												{
													duplicate=false;
												}
											}
										}
									}
									
									break;
									
								case 1:                                     //horizontal
									if(dest[checktile].data!=NULL)
									{
										for(int32_t y=0; ((duplicate==true)&&(y<16)); y++)
										{
											for(int32_t x=0; ((duplicate==true)&&(x<16)); x+=3-newformat)
											{
												//                        if ((dest[(checktile*128)+(y*8)+((14-x)/2)])!=(((testtile[(y*8)+(x/2)]&15)<<4)+((testtile[(y*8)+(x/2)]>>4)&15)))
												if((dest[checktile].data[(y*8*newformat)+(14+(newformat-1)-x)/(3-newformat)])!=(newformat==tf4Bit?(((testtile[(y*8)+(x/2)]&15)<<4)+((testtile[(y*8)+(x/2)]>>4)&15)):(testtile[(y*16)+x])))
												{
													duplicate=false;
												}
											}
										}
									}
									
									break;
									
								case 2:                                     //vertical
									if(dest[checktile].data!=NULL)
									{
										for(int32_t y=0; ((duplicate==true)&&(y<16)); y++)
										{
											for(int32_t x=0; ((duplicate==true)&&(x<16)); x+=3-newformat)
											{
												//                      if ((dest[(checktile*128)+((15-y)*8)+(x/2)])!=(testtile[(y*8)+(x/2)]))
												if((dest[checktile].data[((15-y)*8*newformat)+(x/(3-newformat))])!=(newformat==tf4Bit?(testtile[(y*8)+(x/2)]):(testtile[(y*16)+x])))
												{
													duplicate=false;
												}
											}
										}
									}
									
									break;
									
								case 3:                                     //both
									if(dest[checktile].data!=NULL)
									{
										for(int32_t y=0; ((duplicate==true)&&(y<16)); y++)
										{
											for(int32_t x=0; ((duplicate==true)&&(x<16)); x+=3-newformat)
											{
												//                      if ((dest[(checktile*128)+((15-y)*8)+((14-x)/2)])!=(((testtile[(y*8)+(x/2)]&15)<<4)+((testtile[(y*8)+(x/2)]>>4)&15)))
												if((dest[checktile].data[((15-y)*8*newformat)+((14+(newformat-1)-x)/(3-newformat))])!=(newformat==tf4Bit?(((testtile[(y*8)+(x/2)]&15)<<4)+((testtile[(y*8)+(x/2)]>>4)&15)):testtile[(y*16)+x]))
												{
													duplicate=false;
												}
											}
										}
									}
									
									break;
								}
							}
							
							if(duplicate==true)
							{
								++duplicates_found[flipping];
								++total_duplicates_found;
								
								if(DuplicateAction[flipping]>1)
								{
									++total_duplicates_discarded;
									temp_canadd=false;
								}
							}
						}
						
						canadd=canadd&&temp_canadd;
					}
				}
			}
			
//      dest[currtile].format=(cdepth==3?imported_format:cdepth);
			dest[currtile].format=newformat;
			
			if(dest[currtile].data!=NULL)
			{
				free(dest[currtile].data);
			}
			
			dest[currtile].data=(byte *)malloc(tilesize(dest[currtile].format));
			
			if(dest[currtile].data==NULL)
			{
				Z_error_fatal("Unable to initialize tile #%d.\n", currtile);
			}
			
			if(canadd==true)
			{
				/*
				  for(int32_t y=0; y<16; y++)
				  {
				  for(int32_t x=0; x<8; x++)
				  {
				  dest[currtile].data[(y*8)+x]=testtile[(y*8)+x];
				  }
				  }
				  */
				memcpy(dest[currtile].data, testtile, tilesize(dest[currtile].format));
				++currtile;
			}
		}
	}
	
	destroy_bitmap(status);
	delete[] testtile;
	return true;
}

void grab(byte(*dest)[256],byte *def, int32_t width, int32_t height, int32_t oformat, byte *newformat)
{
	// Not too sure what's going on with the format stuff here...
	byte defFormat=(bp==8) ? tf8Bit : tf4Bit;
	byte format=defFormat;
	int32_t stile = ((imagey*TILES_PER_ROW)+imagex)+(((sely/16)*TILES_PER_ROW)+(selx/16));
	
	switch(imagetype)
	{
	case ftZGP:
	case ftQST:
	case ftZQT:
	case ftQSU:
	case ftTIL:
	case ftBIN:
	case ftBMP:
		for(int32_t ty=0; ty<height; ty++)
		{
			for(int32_t tx=0; tx<width; tx++)
			{
				format=defFormat;
				
				switch(imagetype)
				{
				case ftZGP:
				case ftQST:
				case ftZQT:
				case ftQSU:
				case ftTIL:
					format=grabtilebuf[stile+((ty*TILES_PER_ROW)+tx)].format;
					break;
				}
				
				for(int32_t y=0; y<16; y++)
				{
					for(int32_t x=0; x<16; x+=2)
					{
						if(y<8 && x<8 && grabmask&1)
						{
							switch(oformat)
							{
							case tf4Bit:
								switch(format)
								{
								case tf4Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=def[(y*8)+(x/2)];
									break;
									
								case tf8Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*8)+(x/2)]&15;
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*8)+(x/2)]>>4;
									break;
								}
								
								break;
								
							case tf8Bit:
								switch(format)
								{
								case tf4Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=(def[(y*16)+(x)]&15)+(def[(y*16)+(x+1)]<<4);
									break;
									
								case tf8Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*16)+(x)];
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*16)+(x+1)];
									break;
								}
								
								break;
							}
						}
						else if(y<8 && x>7 && grabmask&2)
						{
							switch(oformat)
							{
							case tf4Bit:
								switch(format)
								{
								case tf4Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=def[(y*8)+(x/2)];
									break;
									
								case tf8Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*8)+(x/2)]&15;
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*8)+(x/2)]>>4;
									break;
								}
								
								break;
								
							case tf8Bit:
								switch(format)
								{
								case tf4Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=(def[(y*16)+(x)]&15)+(def[(y*16)+(x+1)]<<4);
									break;
									
								case tf8Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*16)+(x)];
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*16)+(x+1)];
									break;
								}
								
								break;
							}
						}
						else if(y>7 && x<8 && grabmask&4)
						{
							switch(oformat)
							{
							case tf4Bit:
								switch(format)
								{
								case tf4Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=def[(y*8)+(x/2)];
									break;
									
								case tf8Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*8)+(x/2)]&15;
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*8)+(x/2)]>>4;
									break;
								}
								
								break;
								
							case tf8Bit:
								switch(format)
								{
								case tf4Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=(def[(y*16)+(x)]&15)+(def[(y*16)+(x+1)]<<4);
									break;
									
								case tf8Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*16)+(x)];
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*16)+(x+1)];
									break;
								}
								
								break;
							}
						}
						else if(y>7 && x>7 && grabmask&8)
						{
							switch(oformat)
							{
							case tf4Bit:
								switch(format)
								{
								case tf4Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=def[(y*8)+(x/2)];
									break;
									
								case tf8Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*8)+(x/2)]&15;
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*8)+(x/2)]>>4;
									break;
								}
								
								break;
								
							case tf8Bit:
								switch(format)
								{
								case tf4Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=(def[(y*16)+(x)]&15)+(def[(y*16)+(x+1)]<<4);
									break;
									
								case tf8Bit:
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*16)+(x)];
									dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*16)+(x+1)];
									break;
								}
								
								break;
							}
						}
						else
						{
							switch(format)
							{
							case tf8Bit:
								dest[(ty*TILES_PER_ROW)+tx][(y*16)+x]=getpixel(screen2,(tx*16)+x+selx,(ty*16)+y+sely);
								dest[(ty*TILES_PER_ROW)+tx][(y*16)+x+1]=getpixel(screen2,(tx*16)+x+1+selx,(ty*16)+y+sely);
								newformat[(ty*TILES_PER_ROW)+tx] = tf8Bit;
								break;
								
							case tf4Bit:
							default:
								dest[(ty*TILES_PER_ROW)+tx][(y*8)+(x/2)]=(getpixel(screen2,(tx*16)+x+selx,(ty*16)+y+sely)&15)+((getpixel(screen2,(tx*16)+x+1+selx,(ty*16)+y+sely)&15)<<4);
								newformat[(ty*TILES_PER_ROW)+tx] = tf4Bit;
								break;
							}
						}
					}
				}
			}
		}
		
		break;
		
	default:
		for(int32_t i=0; i<200; i++)
		{
			for(int32_t j=0; j<256; j++)
			{
				dest[i][j]=0;
			}
			
			newformat[i] = tf4Bit;
		}
		
		break;
	}
}

//Grabber is not grabbing to tile pages beyond pg. 252 right now. -ZX 18th June, 2019 
void grab_tile(int32_t tile,int32_t &cs)
{
	popup_zqdialog_start(0,0,LARGE_W,LARGE_H,0xFF);
	int32_t window_xofs=(zq_screen_w-640-12)>>1;
	int32_t window_yofs=(zq_screen_h-480-25-6)>>1;
	int32_t screen_xofs=window_xofs+6;
	int32_t screen_yofs=window_yofs+25;
	int32_t panel_yofs=0;
	int32_t bwidth = 61*1.5;
	int32_t bheight = 20*1.5;
	int32_t button_x = 255*2;
	int32_t grab_ok_button_y = 168*2 + 16 + 26;
	int32_t leech_button_x = 117*2;
	int32_t leech_button_y = 166*2 + 16 + 26;
	int32_t grab_cancel_button_y = 192*2 + 16 + 26;
	int32_t file_button_y = 216*2 + 16 + 26;
	int32_t rec_button_x = 117*2;
	int32_t rec_button_y = 192*2 + 16 + 26;
	
	int32_t screen_y1 = 24;
	int32_t screen_y2 = screen_y1+320-1;
	
	int32_t crect_x = 184+190;
	int32_t crect_y = 168*2 + 32;
	int32_t crect_w = 8*2;
	int32_t crect_h = 8*2;
	
	int32_t xrect_x = 640 + 12 - 21;
	int32_t xrect_y = 5;
	
	byte newtile[200][256];
	BITMAP *screen3=create_bitmap_ex(8, zq_screen_w, zq_screen_h);
	clear_bitmap(screen3);
	byte newformat[200];
	
	memset(newtile, 0, 200*256);
	memset(newformat, 0, 200);
	
	static EXT_LIST list[] =
	{
		{ (char *)"All Files (*.*)",                   NULL },
		{ (char *)"Bitmap Image (*.bmp)", (char *)"bmp" },
		{ (char *)"GIF Image (*.gif)", (char *)"gif" },
		{ (char *)"JPEG Image (*.jpg, *.jpeg)", (char *)"jpg,jpeg" },
		{ (char *)"ZC Tile Export (*.til)", (char *)"til" },
		{ (char *)"ZC Quest Template (*.zqt)", (char *)"zqt" },
		{ (char *)"ZC Quest (*.qst)", (char *)"qst" },
		{ (char *)"ZC Graphics Pack (*.zgp)", (char *)"zgp" },
		{ (char *)"ZC Unencoded Quest (*.qsu)", (char *)"qsu" },
		{ (char *)"NES ROM Image (*.nes)", (char *)"nes" },
		{ (char *)"SNES ROM Image (*.smc)", (char *)"smc" },
		{ (char *)"Gameboy ROM Image (*.gb)", (char *)"gb" },
		{ (char *)"Gameboy Advance ROM Image (*.gba)", (char *)"gba" },
		{ NULL,                                        NULL }
	};
	
	
	memset(cset_reduce_table, 0, 256);
	memset(col_diff,0,3*128);
	bool bdown=false;
	int32_t done=0;
	int32_t pal=0;
	int32_t f=0;
	int32_t black=vc(0),white=vc(15);
	int32_t selwidth=1, selheight=1;
	int32_t selx2=0, sely2=0;
	bool xreversed=false, yreversed=false;
	bool doleech=false, dofile=false, dopal=false;
	
	int32_t jwin_pal2[jcMAX];
	memcpy(jwin_pal2, jwin_pal, sizeof(int32_t)*jcMAX);
	
	
	if(imagebuf==NULL)
		load_imagebuf();
		
	calc_cset_reduce_table(imagepal, cs);
	calc_cset_reduce_table_8bit(imagepal);
	draw_grab_window();
	draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
	grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
	draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	do
	{
		rest(4);
		bool redraw=false;
		
		if(keypressed())
		{
			redraw=true;
			
			switch(readkey()>>8)
			{
			case KEY_F:
				dofile=true;
				break;
				
			case KEY_L:
				doleech=true;
				break;
				
			case KEY_P:
				if(imagetype==ftBMP)
				{
					dopal=true;
					recolor=rcNone;
					calc_cset_reduce_table(imagepal, cs);
				}
				
				break;
				
			case KEY_ESC:
				done=1;
				break;
				
			case KEY_ENTER_PAD:
			case KEY_ENTER:
				done=2;
				break;
				
			case KEY_DOWN:
				if(key[KEY_ZC_LCONTROL]||key[KEY_ZC_LCONTROL]) sely=zc_min(sely+1,144);
				else ++imagey;
				
				break;
				
			case KEY_UP:
				if(key[KEY_ZC_LCONTROL]||key[KEY_ZC_LCONTROL]) sely=zc_max(sely-1,0);
				else --imagey;
				
				break;
				
			case KEY_RIGHT:
				if(key[KEY_ZC_LCONTROL]||key[KEY_ZC_LCONTROL]) selx=zc_min(selx+1,304);
				else ++imagex;
				
				break;
				
			case KEY_LEFT:
				if(key[KEY_ZC_LCONTROL]||key[KEY_ZC_LCONTROL]) selx=zc_max(selx-1,0);
				else --imagex;
				
				break;
				
			case KEY_PGDN:
				imagey+=10;
				break;
				
			case KEY_PGUP:
				imagey-=10;
				break;
				
			case KEY_HOME:
				imagex=imagey=0;
				break;
				
			case KEY_EQUALS:
			case KEY_PLUS_PAD:
				cs = (cs<13) ? cs+1:0;
				if(recolor==rc4Bit)
					calc_cset_reduce_table(imagepal, cs);
				break;
				
			case KEY_MINUS:
			case KEY_MINUS_PAD:
				cs = (cs>0) ? cs-1:13;
				if(recolor==rc4Bit)
					calc_cset_reduce_table(imagepal, cs);
				break;
				
			case KEY_S:
				if(grabmode==1) grabmode=8;
				else if(grabmode==8) grabmode=16;
				else grabmode=1;
				
				break;
				
			case KEY_1:
				if(recolor==rc8Bit)
					recolor=rcNone;
				//imagex=(imagex*bp)>>3;
				bp=1;
				//imagex<<=3;
				nesmode=false;
				break;
				
			case KEY_2:
				if(recolor==rc8Bit)
					recolor=rcNone;
				//imagex=(imagex*bp)>>3;
				bp=2;
				//imagex<<=2;
				nesmode=false;
				break;
				
			case KEY_N:
				if(recolor==rc8Bit)
					recolor=rcNone;
				//imagex=(imagex*bp)>>3;
				bp=2;
				//imagex<<=2;
				nesmode=true;
				break;
				
			case KEY_4:
				if(recolor==rc8Bit)
					recolor=rcNone;
				//imagex=(imagex*bp)>>3;
				bp=4;
				//imagex<<=1;
				nesmode=false;
				break;
				
			case KEY_8:
				//imagex=(imagex*bp)>>3;
				bp=8;
				break;
				
			case KEY_B:
				if(bp==2&&!nesmode)
				{
					nesmode=true;
				}
				else
				{
					nesmode=false;
					bp<<=1;
					
					if(bp==16)
					{
						bp=1;
						//imagex<<=3;
					}
					else
					{
						//imagex>>=1;
					}
				}
				
				break;
				
			case KEY_M:
				romtilemode=(romtilemode+1)%4;
				break;
				
			case KEY_Z:
				if(romofs>0) --romofs;
				
				break;
				
			case KEY_X:
				++romofs;
				break;
				
			case KEY_R:
				if(pal)
				{
					dopal=true;
				}
				
				if(recolor!=rcNone)
					recolor=rcNone;
				else if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
				{
					bp=8;
					recolor=rc8Bit;
					calc_cset_reduce_table_8bit(imagepal);
				}
				else
				{
					if(bp==8)
						bp=4;
					recolor=rc4Bit;
					calc_cset_reduce_table(imagepal, cs);
				}
				break;
				
			default:
				redraw=false;
			}
			
			clear_keybuf();
			
			if(imagex<0) imagex=0;
			
			if(imagey<0) imagey=0;
			
			draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
			grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
		}
		
		//boogie!
		if(gui_mouse_b()==1 && !bdown)
		{
			if(isinRect(gui_mouse_x(),gui_mouse_y(),window_xofs + xrect_x, window_yofs + xrect_y, window_xofs + xrect_x + 15, window_yofs + xrect_y + 13))
			{
				if(do_x_button(screen, window_xofs+xrect_x, window_yofs+xrect_y))
				{
					done=1;
				}
			}
			
			if(!bdown)
			{
				bool regrab=false;
				bdown=true;
				int32_t x=gui_mouse_x()-window_xofs;
				int32_t y=gui_mouse_y()-window_yofs;
				// Large Mode: change font temporarily
				FONT* oldfont = font;
				
				font = lfont_l;
				
				if(y>=screen_y1 && y<=screen_y2)
				{
					while(gui_mouse_b())
					{
						x=(gui_mouse_x()-screen_xofs) / 2;
						y=(gui_mouse_y()-screen_yofs) / 2;
						
						if(!(key[KEY_LSHIFT] || key[KEY_RSHIFT]))
						{
							selx=vbound((x/grabmode)*grabmode,0,304);
							sely=vbound((y/grabmode)*grabmode,0,144);
							selx2=selx;
							sely2=sely;
							selwidth=1;
							selheight=1;
							xreversed=false;
							yreversed=false;
						}
						else
						{
							if(xreversed)
							{
								zc_swap(selx, selx2);
								xreversed=false;
							}
							
							if(yreversed)
							{
								zc_swap(sely, sely2);
								yreversed=false;
							}
							
							selx2=vbound((x/grabmode)*grabmode,0,304);
							sely2=vbound((y/grabmode)*grabmode,0,144);
							selwidth=1+(abs(selx2-selx))/16;
							selheight=1+(abs(sely2-sely))/16;
							
							if(selx2<selx)
							{
								zc_swap(selx, selx2);
								xreversed=true;
							}
							
							if(sely2<sely)
							{
								zc_swap(sely, sely2);
								yreversed=true;
							}
						}
						
						//selx*=mul;
						//sely*=mul;
						//         grab(newtile,tilebuf+(tile<<7), 1, 1);
						grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
						draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
					}
				}
				else if(isinRect(x,y,button_x,grab_ok_button_y,button_x+bwidth,grab_ok_button_y+bheight))
				{
					if(do_text_button(button_x+window_xofs,grab_ok_button_y+(76),bwidth,bheight,"OK",vc(1),vc(14),true))
						done=2;
				}
				else if(isinRect(x,y,leech_button_x,leech_button_y,leech_button_x+bwidth,leech_button_y+bheight))
				{
					if(do_text_button(leech_button_x +window_xofs,leech_button_y +(76),bwidth,bheight,"Leech",vc(1),vc(14),true))
					{
						doleech=true;
					}
				}
				else if(isinRect(x,y,button_x,grab_cancel_button_y,button_x+bwidth,grab_cancel_button_y+bheight))
				{
					if(do_text_button(button_x +window_xofs,grab_cancel_button_y +(76),bwidth,bheight,"Cancel",vc(1),vc(14),true))
						done=1;
				}
				else if(isinRect(x,y,button_x,file_button_y,button_x+bwidth,file_button_y+bheight))
				{
					if(do_text_button(button_x +window_xofs,file_button_y+(76),bwidth,bheight,"File",vc(1),vc(14),true))
					{
						dofile=true;
					}
				}
				else if(imagetype == ftBMP && isinRect(x,y,rec_button_x, rec_button_y, rec_button_x+bwidth, rec_button_y+bheight))
				{
					if(do_text_button(rec_button_x+window_xofs,rec_button_y+(76),bwidth,bheight,"Recolor",vc(1),vc(14),true))
					{
						if(pal)
						{
							dopal = true;
						}
						
						if(recolor!=rcNone)
							recolor=rcNone;
						else if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
						{
							bp=8;
							recolor=rc8Bit;
							calc_cset_reduce_table_8bit(imagepal);
						}
						else
						{
							if(bp==8)
								bp=4;
							recolor=rc4Bit;
							calc_cset_reduce_table(imagepal, cs);
						}
						redraw=true;
					}
				}
				else if(isinRect(x,y+panel_yofs,crect_x,crect_y,crect_x+(16),crect_y+crect_h-1))
				{
					regrab=true;
					grabmask^=1;
				}
				else if(isinRect(x,y+panel_yofs,crect_x+crect_w,crect_y,crect_x+(32)-1,crect_y+crect_h-1))
				{
					regrab=true;
					grabmask^=2;
				}
				else if(isinRect(x,y+panel_yofs,crect_x,crect_y+crect_h,crect_x+(16)-1,crect_y+crect_h+crect_h-1))
				{
					regrab=true;
					grabmask^=4;
				}
				else if(isinRect(x,y+panel_yofs,crect_x+crect_w,crect_y+crect_h,crect_x+(32)-1,crect_y+crect_h+crect_h-1))
				{
					regrab=true;
					grabmask^=8;
				}
				
				if(regrab)
				{
					//       grab(newtile,tilebuf+(tile<<7), 1, 1);
					grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
					redraw=true;
				}
				
				font = oldfont;
			}
		}
		
		if(gui_mouse_b()==0)
			bdown=false;
			
		if(dofile)
		{
		
			if(getname_nogo("Load File",NULL,list,imagepath,true))
			{
				set_palette(RAMpal);
				pal=0;
				white=vc(15);
				black=vc(0);
				strcpy(imagepath,temppath);
				load_imagebuf();
				imagex=imagey=0;
				calc_cset_reduce_table(imagepal, cs);
				draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
				//           grab(newtile,tilebuf+(tile<<7), 1, 1);
				grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
			}
			
			while(key[KEY_ESC])
			{
				poll_keyboard();
				/* do nothing */
				rest(1);
			}
			
			clear_keybuf();
			dofile=false;
			redraw=true;
		}
		
		if(doleech)
		{
			if(leech_tiles(newtilebuf,tile,cs))
			{
				done=1;
			}
			else
			{
				while(key[KEY_ESC])
				{
					poll_keyboard();
					/* do nothing */
					rest(1);
				}
				
				clear_keybuf();
				redraw=true;
			}
			
			doleech=false;
		}
		
		if(dopal)
		{
			pal^=1;
			
			if(pal)
			{
				get_bw(imagepal,black,white);
				
				jwin_pal[jcBOX]    =imagepal_table.data[0][jwin_pal[jcBOX]];
				jwin_pal[jcLIGHT]  =imagepal_table.data[0][jwin_pal[jcLIGHT]];
				jwin_pal[jcMEDLT]  =imagepal_table.data[0][jwin_pal[jcMEDLT]];
				jwin_pal[jcMEDDARK]=imagepal_table.data[0][jwin_pal[jcMEDDARK]];
				jwin_pal[jcDARK]   =imagepal_table.data[0][jwin_pal[jcDARK]];
				jwin_pal[jcBOXFG]  =imagepal_table.data[0][jwin_pal[jcBOXFG]];
				jwin_pal[jcTITLEL] =imagepal_table.data[0][jwin_pal[jcTITLEL]];
				jwin_pal[jcTITLER] =imagepal_table.data[0][jwin_pal[jcTITLER]];
				jwin_pal[jcTITLEFG]=imagepal_table.data[0][jwin_pal[jcTITLEFG]];
				jwin_pal[jcTEXTBG] =imagepal_table.data[0][jwin_pal[jcTEXTBG]];
				jwin_pal[jcTEXTFG] =imagepal_table.data[0][jwin_pal[jcTEXTFG]];
				jwin_pal[jcSELBG]  =imagepal_table.data[0][jwin_pal[jcSELBG]];
				jwin_pal[jcSELFG]  =imagepal_table.data[0][jwin_pal[jcSELFG]];
				gui_bg_color=jwin_pal[jcBOX];
				gui_fg_color=jwin_pal[jcBOXFG];
				jwin_set_colors(jwin_pal);
			}
			else
			{
				white=vc(15);
				black=vc(0);
				
				memcpy(jwin_pal, jwin_pal2, sizeof(int32_t)*jcMAX);
				gui_bg_color=jwin_pal[jcBOX];
				gui_fg_color=jwin_pal[jcBOXFG];
				jwin_set_colors(jwin_pal);
			}
			
			set_palette_range(pal?imagepal:RAMpal,0,255,false);
			
			dopal=false;
			redraw=true;
		}
		
		if(redraw)
		{
			draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
		}
		else
		{
			custom_vsync();
		}
		
		if((f%8)==0)
		{
			stretch_blit(screen2,screen3,0, 0, zq_screen_w, zq_screen_h, 0, 0, zq_screen_w*2, zq_screen_h*2);
				
			int32_t selxl = selx* 2;
			int32_t selyl = sely* 2;
			int32_t w = 32;
			
			if(f&8)
			{
				rect(screen3,selxl,selyl,selxl+((selwidth-1)*w)+(w-1),selyl+((selheight-1)*w)+(w-1),white);
			}
			
			scare_mouse();
			blit(screen3,screen,selxl,selyl,selxl+screen_xofs,selyl+screen_yofs,selwidth*w,selheight*w);
			unscare_mouse();
		}
		
		//    SCRFIX();
		++f;
		
	}
	while(!done);
	
	memcpy(jwin_pal, jwin_pal2, sizeof(int32_t)*jcMAX);
	gui_bg_color=jwin_pal[jcBOX];
	gui_fg_color=jwin_pal[jcBOXFG];
	jwin_set_colors(jwin_pal);
	
	
	if(done==2)
	{
		go_tiles();
		saved=false;
			
		//   usetiles=true;
		for(int32_t y=0; y<selheight; y++)
		{
			for(int32_t x=0; x<selwidth; x++)
			{
				int32_t temptile=tile+((TILES_PER_ROW*y)+x);
				int32_t format=(bp==8) ? tf8Bit : tf4Bit;
				
				if(newtilebuf[temptile].data!=NULL)
					free(newtilebuf[temptile].data);
				
				newtilebuf[temptile].format=format;
				newtilebuf[temptile].data=(byte *)malloc(tilesize(format));
				
				//newtilebuf[temptile].format=newformat[(TILES_PER_ROW*y)+x];
				
				
				if(newtilebuf[temptile].data==NULL)
				{
					Z_error_fatal("Unable to initialize tile #%d.\n", temptile);
					break;
				}
				
				for(int32_t i=0; i<((format==tf8Bit) ? 256 : 128); i++)
				{
					newtilebuf[temptile].data[i] = newtile[(TILES_PER_ROW*y)+x][i];
				}
			}
		}
	}
	
	destroy_bitmap(screen3);
	
	if(pal)
		set_palette(RAMpal);
	
	recolor=rcNone;
	calc_cset_reduce_table(imagepal, cs);
	register_blank_tiles();
	popup_zqdialog_end();
}

void al5_invalid(int x, int y, int sz)
{
	if(InvalidStatic)
		draw_static(x,y,sz,sz);
	else
	{
		al_draw_filled_rectangle(x, y, x+sz, y+sz, AL5_BLACK);
		al_draw_rectangle(x+2, y+2, x+sz-2, y+sz-2, AL5_WHITE, 0);
		al_draw_x(x+2, y+2, x+sz-2, y+sz-2, AL5_WHITE, 0);
	}
}

int32_t show_only_unused_tiles=4; //1 bit: hide used, 2 bit: hide unused, 4 bit: hide blank
bool tile_is_used(int32_t tile)
{
	return used_tile_table[tile];
}
void draw_tiles(int32_t first,int32_t cs, int32_t f)
{
	draw_tiles(screen2, first, cs, f, true);
}
void draw_tiles(BITMAP* dest,int32_t first,int32_t cs, int32_t f, bool large, bool true_empty)
{
	int32_t screen_xofs=6;
	int32_t screen_yofs=25;
	clear_bitmap(dest);
	BITMAP *buf = create_bitmap_ex(8,16,16);
	
	int32_t w = 16;
	int32_t h = 16;
	
	if(large)
	{
		w *=2;
		h *=2;
	}
	
	for(int32_t i=0; i<TILES_PER_PAGE; i++)                       // 13 rows, leaving 32 pixels from y=208 to y=239
	{
		int32_t x = (i%TILES_PER_ROW)<<4;
		int32_t y = (i/TILES_PER_ROW)<<4;
		int a5x = (x*2) + screen_xofs;
		int a5y = (y*2) + screen_yofs;
		int32_t l = 16;
		
		if(large)
		{
			x*=2;
			y*=2;
			l*=2;
		}
		
		l-=2;
		
		if((HIDE_USED && tile_is_used(first+i) && !blank_tile_table[first+i])   // 1 bit: hide used
				|| (HIDE_UNUSED && !tile_is_used(first+i) && !blank_tile_table[first+i]) // 2 bit: hide unused
				|| (HIDE_BLANK && blank_tile_table[first+i]))	// 4 bit: hide blank
		{
			if(!true_empty) //Use pure color 0; no effects
			{
				if(a4_bmp_active())
					rectfill(dest,x,y,x+l+1,y+l+1,get_zqdialog_a4_clear_color());
				al5_invalid(a5x,a5y,l+2);
			}
		}
		else
		{
			puttile16(buf,first+i,0,0,cs,0);
			stretch_blit(buf,dest,0,0,16,16,x,y,w,h);
		}
		
		if((f%32)<=16 && large && !HIDE_8BIT_MARKER && newtilebuf[first+i].format==tf8Bit)
		{
			textprintf_ex(dest,z3smallfont,(x)+l-3,(y)+l-3,vc(int32_t((f%32)/6)+10),-1,"8");
		}
	}
	
	destroy_bitmap(buf);
}

void tile_info_0(int32_t tile,int32_t tile2,int32_t cs,int32_t copy,int32_t copycnt,int32_t page,bool rect_sel)
{
	int32_t a5x=6;
	int32_t a5y=25;
	int32_t yofs=3;
	BITMAP *buf = create_bitmap_ex(8,16,16);
	char cbuf[32];
	int32_t mul = 2;
	ALLEGRO_FONT *tfont = get_zc_font_a5(font_lfont_l);
	
	if(a4_bmp_active())
		rectfill(screen2,0,416,640-1,480,get_zqdialog_a4_clear_color());
	al_draw_filled_rectangle(a5x,a5y+416,a5x+640-1,a5y+480,jwin_a5_pal(jcBOX));
	al_draw_hline(a5x, a5y+(210*2)-2, a5x+(320*2)-1, jwin_a5_pal(jcMEDLT));
	al_draw_hline(a5x, a5y+(210*2)-1, a5x+(320*2)-1, jwin_a5_pal(jcLIGHT));
	
	// Copied tile and numbers
	jwin_draw_frame_a5(a5x+(34*mul)-2,a5y+((216*mul)+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	int32_t coldiff=TILECOL(copy)-TILECOL(copy+copycnt-1);
	if(copy>=0)
	{
		puttile16(buf,rect_sel&&coldiff>0?copy-coldiff:copy,0,0,cs,0);
		stretch_blit(buf,screen2,0,0,16,16,34*mul,216*mul+yofs,16*mul,16*mul);
		
		if(copycnt>1)
		{
			sprintf(cbuf,"%d-",copy);
			jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+28*mul,a5y+(216*mul)+yofs,ALLEGRO_ALIGN_RIGHT,cbuf,jwin_a5_pal(jcBOX));
			sprintf(cbuf,"%d",copy+copycnt-1);
			jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+24*mul,a5y+(224*mul)+yofs,ALLEGRO_ALIGN_RIGHT,cbuf,jwin_a5_pal(jcBOX));
		}
		else
		{
			sprintf(cbuf,"%d",copy);
			jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+24*mul,a5y+(220*mul)+yofs,ALLEGRO_ALIGN_RIGHT,cbuf,jwin_a5_pal(jcBOX));
		}
	}
	else // No tiles copied
	{
		al5_invalid(a5x+34*mul, a5y+(216*mul)+yofs, 16*mul);
	}
	
	
	// Current tile
	jwin_draw_frame_a5(a5x+(104*mul)-2,a5y+(216*mul+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	puttile16(buf,tile,0,0,cs,0);
	stretch_blit(buf,screen2,0,0,16,16,104*mul,216*mul+yofs,16*mul,16*mul);
	
	// Current selection mode
	jwin_draw_frame_a5(a5x+(127*mul)-2,a5y+(216*mul+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	stretch_blit(select_bmp[rect_sel?1:0],screen2,0,0,16,16,127*mul,216*mul+yofs,16*mul,16*mul);
	
	if(tile>tile2)
	{
		zc_swap(tile,tile2);
	}
	
	char tbuf[8];
	tbuf[0]=0;
	
	if(tile2!=tile)
	{
		sprintf(tbuf,"-%d",tile2);
	}
	
	// Current tile and CSet text
	sprintf(cbuf, "CSet: %d", cs);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+55*mul,a5y+216*mul+yofs,ALLEGRO_ALIGN_LEFT,cbuf,jwin_a5_pal(jcBOX));
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+99*mul,a5y+216*mul+yofs,ALLEGRO_ALIGN_RIGHT,"Tile:",jwin_a5_pal(jcBOX));
	sprintf(cbuf, "%d%s", tile, tbuf);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+99*mul,a5y+224*mul+yofs,ALLEGRO_ALIGN_RIGHT,cbuf,jwin_a5_pal(jcBOX));
	
	ALLEGRO_FONT *tf = a5font;
	a5font = tfont;
	
	jwin_draw_text_button_a5(a5x+150*mul,a5y+213*mul+yofs,28*mul,21*mul,"&Grab",0,false);
	jwin_draw_text_button_a5(a5x+(150+28)*mul,a5y+213*mul+yofs,28*mul,21*mul,"&Edit",0,false);
	jwin_draw_text_button_a5(a5x+(150+28*2)*mul,a5y+213*mul+yofs,28*mul,21*mul,"Export",0,false);
	jwin_draw_text_button_a5(a5x+(150+28*3)*mul,a5y+213*mul+yofs,28*mul,21*mul,"Recolor",0,false);
	jwin_draw_text_button_a5(a5x+(150+28*4)*mul,a5y+213*mul+yofs,28*mul,21*mul,"Done",0,false);
	
	draw_arrow_a5(jwin_a5_pal(jcBOXFG),a5x+609,a5y+430+yofs,5,true,false);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+293*mul,a5y+220*mul+yofs,ALLEGRO_ALIGN_LEFT,"PG:",jwin_a5_pal(jcBOX));
	sprintf(cbuf, "%d",page);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+(305*mul+4),a5y+220*mul+yofs,ALLEGRO_ALIGN_CENTRE,cbuf,jwin_a5_pal(jcBOX));
	draw_arrow_a5(jwin_a5_pal(jcBOXFG),a5x+609,a5y+460+yofs,5,false,false);
	
	a5font = tf;
	
	scare_mouse();
	blit(screen2,screen,0,0,a5x,a5y,640,480);
	unscare_mouse();
	update_hw_screen();
	SCRFIX();
	destroy_bitmap(buf);
}

void tile_info_1(int32_t oldtile,int32_t oldflip,int32_t oldcs,int32_t tile,int32_t flip,int32_t cs,int32_t copy,int32_t page, bool always_use_flip)
{
	int32_t a5x=6;
	int32_t a5y=25;
	int32_t yofs=3;
	BITMAP *buf = create_bitmap_ex(8,16,16);
	char cbuf[32];
	int32_t mul = 2;
	ALLEGRO_FONT *tfont = get_zc_font_a5(font_lfont_l);
	
	if(a4_bmp_active())
		rectfill(screen2,0,416,640-1,480,get_zqdialog_a4_clear_color());
	al_draw_hline(a5x, a5y+(210*2)-2, a5x+(320*2)-1, jwin_a5_pal(jcMEDLT));
	al_draw_hline(a5x, a5y+(210*2)-1, a5x+(320*2)-1, jwin_a5_pal(jcLIGHT));
	
	jwin_draw_frame_a5(a5x+(124*mul)-2,a5y+((216*mul)+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	
	if(copy>=0)
	{
		puttile16(buf,copy,0,0,cs,flip);
		stretch_blit(buf,screen2,0,0,16,16,124*mul,216*mul+yofs,16*mul,16*mul);
	}
	else
	{
		al5_invalid(a5x+124*mul, a5y+(216*mul)+yofs, 16*mul);
	}
	
	jwin_draw_frame_a5(a5x+(8*mul)-2,a5y+(216*mul+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	puttile16(buf,oldtile,0,0, oldcs, oldflip);
	stretch_blit(buf,screen2,0,0,16,16,8*mul,216*mul+yofs,16*mul,16*mul);
	
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+56*mul,a5y+212*mul+yofs,ALLEGRO_ALIGN_RIGHT,"Old Tile:",jwin_a5_pal(jcBOX));
	sprintf(cbuf, "%d",oldtile);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+60*mul,a5y+212*mul+yofs,ALLEGRO_ALIGN_LEFT,cbuf,jwin_a5_pal(jcBOX));
	
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+56*mul,a5y+220*mul+yofs,ALLEGRO_ALIGN_RIGHT,"CSet:",jwin_a5_pal(jcBOX));
	sprintf(cbuf, "%d",oldcs);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+60*mul,a5y+220*mul+yofs,ALLEGRO_ALIGN_LEFT,cbuf,jwin_a5_pal(jcBOX));
	
	if(oldflip > 0 || always_use_flip)  // Suppress Flip for this usage
	{
		jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+56*mul,a5y+228*mul+yofs,ALLEGRO_ALIGN_RIGHT,"Flip:",jwin_a5_pal(jcBOX));
		sprintf(cbuf, "%d",oldflip);
		jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+60*mul,a5y+228*mul+yofs,ALLEGRO_ALIGN_LEFT,cbuf,jwin_a5_pal(jcBOX));
	}
	
	jwin_draw_frame_a5(a5x+(148*mul)-2,a5y+(216*mul+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	puttile16(buf,tile,0,0, cs,
			  (oldflip>0 || always_use_flip)?flip:0); // Suppress Flip for this usage
	stretch_blit(buf,screen2,0,0,16,16,148*mul,216*mul+yofs,16*mul,16*mul);
	
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+201*mul,a5y+212*mul+yofs,ALLEGRO_ALIGN_RIGHT,"New Tile:",jwin_a5_pal(jcBOX));
	sprintf(cbuf, "%d",tile);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+205*mul,a5y+212*mul+yofs,ALLEGRO_ALIGN_LEFT,cbuf,jwin_a5_pal(jcBOX));
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+201*mul,a5y+220*mul+yofs,ALLEGRO_ALIGN_RIGHT,"CSet:",jwin_a5_pal(jcBOX));
	sprintf(cbuf, "%d",cs);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+205*mul,a5y+220*mul+yofs,ALLEGRO_ALIGN_LEFT,cbuf,jwin_a5_pal(jcBOX));
	
	if(oldflip > 0 || always_use_flip)  // Suppress Flip for this usage
	{
		jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+201*mul,a5y+228*mul+yofs,ALLEGRO_ALIGN_RIGHT,"Flip:",jwin_a5_pal(jcBOX));
		sprintf(cbuf, "%d",flip);
		jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+205*mul,a5y+228*mul+yofs,ALLEGRO_ALIGN_LEFT,cbuf,jwin_a5_pal(jcBOX));
	}
	
	draw_arrow_a5(jwin_a5_pal(jcBOXFG),a5x+609,a5y+430+yofs,5,true,false);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+293*mul,a5y+220*mul+yofs,ALLEGRO_ALIGN_LEFT,"PG:",jwin_a5_pal(jcBOX));
	sprintf(cbuf, "%d",page);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+(305*mul+4),a5y+220*mul+yofs,ALLEGRO_ALIGN_CENTRE,cbuf,jwin_a5_pal(jcBOX));
	draw_arrow_a5(jwin_a5_pal(jcBOXFG),a5x+609,a5y+460+yofs,5,false,false);
	
	scare_mouse();
	blit(screen2,screen,0,0,a5x,a5y,640,480);
	unscare_mouse();
	update_hw_screen();
	SCRFIX();
	destroy_bitmap(buf);
}
/*
void reset_tile(tiledata *buf, int32_t t, int32_t format=1)
{
  buf[t].format=format;
  if (buf[t].data!=NULL)
  {
	free(buf[t].data);
  }
  buf[t].data=(byte *)malloc(tilesize(buf[t].format));
  if (buf[t].data==NULL)
  {
	Z_error_fatal("Unable to initialize tile #%d.\n", t);
  }
  for(int32_t i=0; i<tilesize(buf[t].format); i++)
  {
	buf[t].data[i]=0;
  }
}
*/

int32_t hide_used()
{
	show_only_unused_tiles ^= 1;
	return D_O_K;
}
int32_t hide_unused()
{
	show_only_unused_tiles ^= 2;
	return D_O_K;
}
int32_t hide_blank()
{
	show_only_unused_tiles ^= 4;
	return D_O_K;
}
int32_t hide_8bit_marker()
{
	show_only_unused_tiles ^= 8;
	return D_O_K;
}

static MENU select_tile_view_menu[] =
{
	{ (char *)"Hide Used",   hide_used,   NULL, 0, NULL },
	{ (char *)"Hide Unused", hide_unused,   NULL, 0, NULL },
	{ (char *)"Hide Blank",  hide_blank,   NULL, 0, NULL },
	{ (char *)"Hide 8-bit marker",  hide_8bit_marker,   NULL, 0, NULL },
	{ NULL,                  NULL,  NULL, 0, NULL }
};

static MENU select_tile_rc_menu[] =
{
	{ (char *)"Copy",    NULL,  NULL, 0, NULL },
	{ (char *)"Paste",   NULL,  NULL, 0, NULL },
	{ (char *)"Move",    NULL,  NULL, 0, NULL },
	{ (char *)"Clear",  NULL,  NULL, 0, NULL },
	{ (char *)"",        NULL,  NULL, 0, NULL },
	{ (char *)"Edit",    NULL,  NULL, 0, NULL },
	{ (char *)"Grab",    NULL,  NULL, 0, NULL },
	{ (char *)"Scale",  NULL,  NULL, 0, NULL },
	{ (char *)"Color Depth",  NULL,  NULL, 0, NULL },
	{ (char *)"",        NULL,  NULL, 0, NULL },
	{ (char *)"Blank?",  NULL,  NULL, 0, NULL },
	{ (char *)"",        NULL,  NULL, 0, NULL },
	{ (char *)"View\t ", NULL,  select_tile_view_menu, 0, NULL },
	{ (char *)"Overlay",  NULL,  NULL, 0, NULL },
	{ (char *)"H-Flip",  NULL,  NULL, 0, NULL },
	{ (char *)"V-Flip",  NULL,  NULL, 0, NULL },
	{ (char *)"Create Combos",  NULL,  NULL, 0, NULL },
	{ (char *)"Insert",  NULL,  NULL, 0, NULL },
	{ NULL,              NULL,  NULL, 0, NULL }
};

static MENU select_combo_rc_menu[] =
{
	{ (char *)"Copy",    NULL,  NULL, 0, NULL },
	{ (char *)"Paste",   NULL,  NULL, 0, NULL },
	{ (char *)"Swap",    NULL,  NULL, 0, NULL },
	{ (char *)"Delete",  NULL,  NULL, 0, NULL },
	{ (char *)"",        NULL,  NULL, 0, NULL },
	{ (char *)"Edit",    NULL,  NULL, 0, NULL },
	{ (char *)"Insert New",  NULL,  NULL, 0, NULL },
	{ (char *)"Remove",  NULL,  NULL, 0, NULL },
	{ (char *)"",        NULL,  NULL, 0, NULL },
	{ (char *)"Locations",  NULL,  NULL, 0, NULL },
	{ NULL,              NULL,  NULL, 0, NULL }
};

//returns the row the tile is in on its page
int32_t tile_page_row(int32_t tile)
{
	return TILEROW(tile)-(TILEPAGE(tile)*TILE_ROWS_PER_PAGE);
}

enum {ti_none, ti_encompass, ti_broken};

//striped check and striped selection
int32_t move_intersection_ss(newcombo &cmb, int32_t selection_first, int32_t selection_last, int32_t offset = 0)
{
	int32_t cmb_first = cmb.o_tile;
	int32_t cmb_last = cmb.o_tile;
	do
	{
		cmb_last = cmb.tile;
		animate(cmb, true);
	}
	while(cmb.tile != cmb.o_tile);
	reset_combo_animation(cmb);
	cmb_first += offset;
	cmb_last += offset;
	
	if(cmb_first > selection_last || cmb_last < selection_first)
		return ti_none;
	if(cmb_first >= selection_first && cmb_last <= selection_last)
		return ti_encompass;
	
	do
	{
		if(cmb.tile+offset >= selection_first && cmb.tile+offset <= selection_last)
		{
			reset_combo_animation(cmb);
			return ti_broken; //contained, but non-encompassing.
		}
		animate(cmb, true);
	}
	while(cmb.tile != cmb.o_tile);
	reset_combo_animation(cmb);
	return ti_none;
}
int32_t move_intersection_ss(int32_t check_first, int32_t check_last, int32_t selection_first, int32_t selection_last)
{
	// if selection is before or after check...
	if((check_first>selection_last)||(selection_first>check_last))
	{
		return ti_none;
	}
	
	// if selection envelopes check
	if((selection_first<=check_first)&&(selection_last>=check_last))
	{
		return ti_encompass;  //encompass
	}
	
	//everything else is a break
	return ti_broken;  //intersect
}



//rectangular check and striped selection
int32_t move_intersection_rs(int32_t check_left, int32_t check_top, int32_t check_width, int32_t check_height, int32_t selection_first, int32_t selection_last)
{
	int32_t ret1=-1, ret2=-1;
	
	for(int32_t i=0; i<check_height; ++i)
	{
		int32_t check_first=((check_top+i)*TILES_PER_ROW)+check_left;
		int32_t check_last=check_first+check_width-1;
		ret2=move_intersection_ss(check_first, check_last, selection_first, selection_last);
		
		if(ret2==ti_broken)
		{
			return ti_broken;
		}
		
		ret1=(ret2==ti_encompass?ti_encompass:ret1);
	}
	
	if(ret1==ti_encompass)
	{
		if((TILEROW(selection_first)<=check_top) &&
				(TILEROW(selection_last)>=(check_top+check_height-1)))
		{
			return ti_encompass;
		}
		else
		{
			return ti_broken;
		}
	}
	
	return ti_none;
}


//striped check and rectangular selection
int32_t move_intersection_sr(newcombo &cmb, int32_t selection_left, int32_t selection_top, int32_t selection_width, int32_t selection_height, int32_t offset = 0)
{
	if(selection_width < TILES_PER_ROW)
	{
		int32_t cmb_first = cmb.o_tile;
		int32_t cmb_last = cmb.o_tile;
		do
		{
			cmb_last = cmb.tile;
			animate(cmb, true);
		}
		while(cmb.tile != cmb.o_tile);
		reset_combo_animation(cmb);
		cmb_first += offset;
		cmb_last += offset;
		
		if((TILEROW(cmb_first)>=selection_top) &&
				(TILEROW(cmb_last)<=selection_top+selection_height-1) &&
				(TILECOL(cmb_first)>=selection_left) &&
				(TILECOL(cmb_last)<=TILECOL(selection_left+selection_width-1)))
		{
			return ti_encompass;
		}
		else if((cmb_last<selection_top*TILES_PER_ROW+selection_left) ||
				(cmb_first>(selection_top+selection_height-1)*TILES_PER_ROW+selection_left+selection_width-1))
		{
			return ti_none;
		}
		
		if(TILEROW(cmb_first) == TILEROW(cmb_last))
		{
			int32_t firstcol = TILECOL(cmb_first);
			int32_t lastcol = TILECOL(cmb_last);
			
			if(lastcol < selection_left || firstcol >= selection_left+selection_width)
				return ti_none;
			else //handle skip x
			{
				do
				{
					if(TILECOL(cmb.tile) >= selection_left && TILECOL(cmb.tile) <= selection_left+selection_width)
					{
						reset_combo_animation(cmb);
						return ti_broken;
					}
					animate(cmb, true);
				}
				while(cmb.tile != cmb.o_tile);
				reset_combo_animation(cmb);
				return ti_none;
			}
		}
		else //multi-row combo...
		{
			int32_t row = TILEROW(cmb_first);
			
			do
			{
				if(row < selection_top || row > selection_top+selection_height-1)
				{
					//This row isn't in the selection; skip to next row
					do
					{
						animate(cmb,true);
						if(cmb.tile == cmb.o_tile) return ti_none; //reached end
					}
					while(TILEROW(cmb.tile) == row);
					row = TILEROW(cmb.tile);
					continue;
				}
				
				//This row IS in the selection; check each tile.
				do
				{
					if(TILECOL(cmb.tile) >= selection_left && TILECOL(cmb.tile) <= selection_left+selection_width-1)
					{
						reset_combo_animation(cmb);
						return ti_broken;
					}
					animate(cmb, true);
					if(cmb.tile == cmb.o_tile) return ti_none; //reached end
				}
				while(TILEROW(cmb.tile) == row);
				row = TILEROW(cmb.tile);
			}
			while(cmb.tile != cmb.o_tile);
			
			return ti_none; //...Theoretically unreachable, but if it DOES get here, it's done.
		}
	}
	
	return move_intersection_ss(cmb, selection_top*TILES_PER_ROW+selection_left, (selection_top+selection_height-1)*TILES_PER_ROW+selection_left+selection_width-1, offset);
}
int32_t move_intersection_sr(int32_t check_first, int32_t check_last, int32_t selection_left, int32_t selection_top, int32_t selection_width, int32_t selection_height)
{
	if(selection_width < TILES_PER_ROW)
	{
		if((check_last-check_first+1<=selection_width) &&
				(TILEROW(check_first)>=selection_top) &&
				(TILEROW(check_last)<=selection_top+selection_height-1) &&
				(TILECOL(check_first)>=selection_left) &&
				(TILECOL(check_last)<=TILECOL(selection_left+selection_width-1)))
		{
			return ti_encompass;
		}
		else if((check_last<selection_top*TILES_PER_ROW+selection_left) ||
				(check_first>(selection_top+selection_height-1)*TILES_PER_ROW+selection_left+selection_width-1))
		{
			return ti_none;
		}
		
		//else if (selection_top*TILES_PER_ROW+selection_left<check_first && (selection_top+1)*TILES_PER_ROW+selection_left>check_last)
		
		//one last base case: the strip we're interested in only lies along one row
		if(check_first/TILES_PER_ROW == check_last/TILES_PER_ROW)
		{
			int32_t cfcol = check_first%TILES_PER_ROW;
			int32_t clcol = check_last%TILES_PER_ROW;
			
			if(clcol < selection_left || cfcol >= selection_left+selection_width)
				return ti_none;
			else
				return ti_broken;
		}
		else
		{
			//recursively cut the strip into substrips which lie entirely on one row
			int32_t currow = check_first/TILES_PER_ROW;
			int32_t endrow = check_last/TILES_PER_ROW;
			int32_t accum = 0;
			accum |= move_intersection_sr(check_first,(currow+1)*TILES_PER_ROW-1,selection_left,selection_top,selection_width,selection_height);
			
			for(++currow; currow<endrow; currow++)
			{
				accum |= move_intersection_sr(currow*TILES_PER_ROW,(currow+1)*TILES_PER_ROW-1,selection_left,selection_top,selection_width,selection_height);
			}
			
			accum |= move_intersection_sr(currow*TILES_PER_ROW, check_last,selection_left,selection_top,selection_width,selection_height);
			
			if(accum > 0)
				return ti_broken;
				
			return ti_none;
		}
	}
	
	return move_intersection_ss(check_first, check_last, selection_top*TILES_PER_ROW+selection_left, (selection_top+selection_height-1)*TILES_PER_ROW+selection_left+selection_width-1);
}

//rectangular check and rectangular selection
int32_t move_intersection_rr(int32_t check_left, int32_t check_top, int32_t check_width, int32_t check_height, int32_t selection_left, int32_t selection_top, int32_t selection_width, int32_t selection_height)
{
	if((check_left>=selection_left) &&
			(check_left+check_width<=selection_left+selection_width) &&
			(check_top>=selection_top) &&
			(check_top+check_height<=selection_top+selection_height))
	{
		return ti_encompass;
	}
	else
	{
		for(int32_t i=check_top; i<check_top+check_height; ++i)
		{
			if(move_intersection_rs(selection_left, selection_top, selection_width, selection_height, i*TILES_PER_ROW+check_left, i*TILES_PER_ROW+check_left+check_width-1)!=ti_none)
			{
				return ti_broken;
			}
		}
	}
	
	return ti_none;
}




static DIALOG tile_move_list_dlg[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
	{ jwin_win_proc,      0,   0,   254,  178,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,      NULL, NULL, NULL },
	{ jwin_ctext_proc,   127,  24,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "", NULL, NULL },
	{ jwin_ctext_proc,   127,  34,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "", NULL, NULL },
	{ jwin_ctext_proc,   127,  44,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "", NULL, NULL },
	{ jwin_textbox_proc,  12,   54,   231,  96,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,      NULL, NULL, NULL },
	{ jwin_button_proc,   57,   153,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
	{ jwin_button_proc,  137,   153,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
	{ d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
	{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};


typedef struct move_tiles_item
{
	const char *name;
	int32_t tile;
	int32_t width;
	int32_t height;
} move_tiles_item;

/*move_tiles_item subscreen_items[1]=
{
  { "Tile Block",             0,  0,  0, },
};*/

move_tiles_item map_styles_items[6]=
{
	{ "Frame",                  0,  2,  2 },
	{ "Heart Container Piece",  0,  1,  1 },
	{ "Triforce Fragment",      0, -1, -1 },
	{ "Triforce Frame",         0, -1, -1 },
	{ "Overworld Map",          0,  5,  3 },
	{ "Dungeon Map",            0,  5,  3 },
};

move_tiles_item dmap_map_items[4]=
{
	{ "Minimap (Empty)",        0,  5,  3 },
	{ "Minimap (Filled)",       0,  5,  3 },
	{ "Large Map (Empty)",      0, -1,  5 },
	{ "Large Map (Filled)",     0, -1,  5 },
};

enum
{
	//0
	hspr_walk_up, hspr_walk_down, hspr_walk_left, hspr_walk_right,
	//4
	hspr_slash_up, hspr_slash_down, hspr_slash_left, hspr_slash_right,
	//8
	hspr_stab_up, hspr_stab_down, hspr_stab_left, hspr_stab_right,
	//12
	hspr_pound_up, hspr_pound_down, hspr_pound_left, hspr_pound_right,
	//16
	hspr_holdland_1, hspr_holdland_2,
	hspr_casting,
	//19
	hspr_float_up, hspr_float_down, hspr_float_left, hspr_float_right,
	//23
	hspr_swim_up, hspr_swim_down, hspr_swim_left, hspr_swim_right,
	//27
	hspr_dive_up, hspr_dive_down, hspr_dive_left, hspr_dive_right,
	//31
	hspr_holdwater_1, hspr_holdwater_2,
	//33
	hspr_jump_up, hspr_jump_down, hspr_jump_left, hspr_jump_right,
	//37
	hspr_charge_up, hspr_charge_down, hspr_charge_left, hspr_charge_right,
	//41
	hspr_slash_2_up, hspr_slash_2_down, hspr_slash_2_left, hspr_slash_2_right,
	//45
	hspr_falling_up, hspr_falling_down, hspr_falling_left, hspr_falling_right,
	//49
	hspr_lifting_up, hspr_lifting_down, hspr_lifting_left, hspr_lifting_right,
	//53
	hspr_liftwalk_up, hspr_liftwalk_down, hspr_liftwalk_left, hspr_liftwalk_right,
	//57
	hspr_drown_up, hspr_drown_down, hspr_drown_left, hspr_drown_right,
	//61
	hspr_lavadrown_up, hspr_lavadrown_down, hspr_lavadrown_left, hspr_lavadrown_right,
	//65
	hspr_sideswim_up, hspr_sideswim_down, hspr_sideswim_left, hspr_sideswim_right,
	//69
	hspr_sideslash_up, hspr_sideslash_down, hspr_sideslash_left, hspr_sideslash_right,
	//73
	hspr_sidestab_up, hspr_sidestab_down, hspr_sidestab_left, hspr_sidestab_right,
	//77
	hspr_sidepound_up, hspr_sidepound_down, hspr_sidepound_left, hspr_sidepound_right,
	//81
	hspr_sidecharge_up, hspr_sidecharge_down, hspr_sidecharge_left, hspr_sidecharge_right,
	//85
	hspr_holdsidewater_1, hspr_holdsidewater_2,
	hspr_sideswimcasting, hspr_sidedrowning,
	num_hspr
};

move_tiles_item hero_sprite_items[num_hspr]=
{
	//0
	{ "Walk (Up)",                0,  0,  0 },
	{ "Walk (Down)",              0,  0,  0 },
	{ "Walk (Left)",              0,  0,  0 },
	{ "Walk (Right)",             0,  0,  0 },
	//4
	{ "Slash (Up)",               0,  0,  0 },
	{ "Slash (Down)",             0,  0,  0 },
	{ "Slash (Left)",             0,  0,  0 },
	{ "Slash (Right)",            0,  0,  0 },
	//8
	{ "Stab (Up)",                0,  0,  0 },
	{ "Stab (Down)",              0,  0,  0 },
	{ "Stab (Left)",              0,  0,  0 },
	{ "Stab (Right)",             0,  0,  0 },
	//12
	{ "Pound (Up)",               0,  0,  0 },
	{ "Pound (Down)",             0,  0,  0 },
	{ "Pound (Left)",             0,  0,  0 },
	{ "Pound (Right)",            0,  0,  0 },
	//16
	{ "Hold (Land, One Hand)",    0,  0,  0 },
	{ "Hold (Land, Two Hands)",   0,  0,  0 },
	{ "Cast",                     0,  0,  0 },
	//19
	{ "Float (Up)",               0,  0,  0 },
	{ "Float (Down)",             0,  0,  0 },
	{ "Float (Left)",             0,  0,  0 },
	{ "Float (Right)",            0,  0,  0 },
	//23
	{ "Swim (Up)",                0,  0,  0 },
	{ "Swim (Down)",              0,  0,  0 },
	{ "Swim (Left)",              0,  0,  0 },
	{ "Swim (Right)",             0,  0,  0 },
	//27
	{ "Dive (Up)",                0,  0,  0 },
	{ "Dive (Down)",              0,  0,  0 },
	{ "Dive (Left)",              0,  0,  0 },
	{ "Dive (Right)",             0,  0,  0 },
	//31
	{ "Hold (Water, One Hand)",   0,  0,  0 },
	{ "Hold (Water, Two Hands)",  0,  0,  0 },
	//33
	{ "Jump (Up)",                0,  0,  0 },
	{ "Jump (Down)",              0,  0,  0 },
	{ "Jump (Left)",              0,  0,  0 },
	{ "Jump (Right)",             0,  0,  0 },
	//37
	{ "Charge (Up)",              0,  0,  0 },
	{ "Charge (Down)",            0,  0,  0 },
	{ "Charge (Left)",            0,  0,  0 },
	{ "Charge (Right)",           0,  0,  0 },
	//41
	{ "Slash 2 (Up)",             0,  0,  0 },
	{ "Slash 2 (Down)",           0,  0,  0 },
	{ "Slash 2 (Left)",           0,  0,  0 },
	{ "Slash 2 (Right)",          0,  0,  0 },
	//45
	{ "Falling (Up)",             0,  0,  0 },
	{ "Falling (Down)",           0,  0,  0 },
	{ "Falling (Left)",           0,  0,  0 },
	{ "Falling (Right)",          0,  0,  0 },
	//49
	{ "Lifting (Up)",             0,  0,  0 },
	{ "Lifting (Down)",           0,  0,  0 },
	{ "Lifting (Left)",           0,  0,  0 },
	{ "Lifting (Right)",          0,  0,  0 },
	//53
	{ "LiftWalk (Up)",            0,  0,  0 },
	{ "LiftWalk (Down)",          0,  0,  0 },
	{ "LiftWalk (Left)",          0,  0,  0 },
	{ "LiftWalk (Right)",         0,  0,  0 },
	//57
	{ "Drown (Up)",               0,  0,  0 },
	{ "Drown (Down)",             0,  0,  0 },
	{ "Drown (Left)",             0,  0,  0 },
	{ "Drown (Right)",            0,  0,  0 },
	//61
	{ "LavaDrown (Up)",           0,  0,  0 },
	{ "LavaDrown (Down)",         0,  0,  0 },
	{ "LavaDrown (Left)",         0,  0,  0 },
	{ "LavaDrown (Right)",        0,  0,  0 },
	//65
	{ "SideSwim (Up)",            0,  0,  0 },
	{ "SideSwim (Down)",          0,  0,  0 },
	{ "SideSwim (Left)",          0,  0,  0 },
	{ "SideSwim (Right)",         0,  0,  0 },
	//69
	{ "SideSlash (Up)",           0,  0,  0 },
	{ "SideSlash (Down)",         0,  0,  0 },
	{ "SideSlash (Left)",         0,  0,  0 },
	{ "SideSlash (Right)",        0,  0,  0 },
	//73
	{ "SideStab (Up)",            0,  0,  0 },
	{ "SideStab (Down)",          0,  0,  0 },
	{ "SideStab (Left)",          0,  0,  0 },
	{ "SideStab (Right)",         0,  0,  0 },
	//77
	{ "SidePound (Up)",           0,  0,  0 },
	{ "SidePound (Down)",         0,  0,  0 },
	{ "SidePound (Left)",         0,  0,  0 },
	{ "SidePound (Right)",        0,  0,  0 },
	//81
	{ "SideCharge (Up)",          0,  0,  0 },
	{ "SideCharge (Down)",        0,  0,  0 },
	{ "SideCharge (Left)",        0,  0,  0 },
	{ "SideCharge (Right)",       0,  0,  0 },
	//85
	{ "Hold (SideWater, One Hand)",   0,  0,  0 },
	{ "Hold (SideWater, Two Hands)",  0,  0,  0 },
	{ "SideSwim Casting",             0,  0,  0 },
	{ "SideDrown",                    0,  0,  0 },
};

int32_t quick_select_3(int32_t a, int32_t b, int32_t c, int32_t d)
{
	return a==0?b:a==1?c:d;
}

void setup_hero_sprite_items()
{
	int32_t a_style=(zinit.heroAnimationStyle);
	
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[i].tile=walkspr[i][spr_tile]-(walkspr[i][spr_extend]<2?0:1)-(walkspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[i].width=(walkspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, (i==0?1:2), 3, 9) + (walkspr[i][spr_extend]<2?0:1);
		hero_sprite_items[i].height=walkspr[i][spr_extend]<2?1:2;
	}
	
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[4+i].tile=slashspr[i][spr_tile]-(slashspr[i][spr_extend]<2?0:1)-(slashspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[4+i].width=(slashspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 1, 1, 6) + (slashspr[i][spr_extend]<2?0:1);;
		hero_sprite_items[4+i].height=slashspr[i][spr_extend]<2?1:2;
	}
	
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[8+i].tile=stabspr[i][spr_tile]-(stabspr[i][spr_extend]<2?0:1)-(stabspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[8+i].width=(stabspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 1, 1, 3) + (stabspr[i][spr_extend]<2?0:1);;
		hero_sprite_items[8+i].height=stabspr[i][spr_extend]<2?1:2;
	}
	
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[12+i].tile=poundspr[i][spr_tile]-(poundspr[i][spr_extend]<2?0:1)-(poundspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[12+i].width=(poundspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 1, 1, 3) + (poundspr[i][spr_extend]<2?0:1);;
		hero_sprite_items[12+i].height=poundspr[i][spr_extend]<2?1:2;
	}
	
	for(int32_t i=0; i<2; ++i)
	{
		hero_sprite_items[16+i].tile=holdspr[0][i][spr_tile]-(holdspr[0][i][spr_extend]<2?0:1)-(holdspr[0][i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[16+i].width=(holdspr[0][i][spr_extend]<2?1:2) + (holdspr[0][i][spr_extend]<2?0:1);;
		hero_sprite_items[16+i].height=holdspr[0][i][spr_extend]<2?1:2;
	}
	
	hero_sprite_items[18].tile=castingspr[spr_tile]-(castingspr[spr_extend]<2?0:1)-(castingspr[spr_extend]<1?0:TILES_PER_ROW);
	hero_sprite_items[18].width=(castingspr[spr_extend]<2?1:2) + (castingspr[spr_extend]<2?0:1);;
	hero_sprite_items[18].height=castingspr[spr_extend]<2?1:2;
	
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[19+i].tile=floatspr[i][spr_tile]-(floatspr[i][spr_extend]<2?0:1)-(floatspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[19+i].width=(floatspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 4) + (floatspr[i][spr_extend]<2?0:1);;
		hero_sprite_items[19+i].height=floatspr[i][spr_extend]<2?1:2;
	}
	
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[23+i].tile=swimspr[i][spr_tile]-(swimspr[i][spr_extend]<2?0:1)-(swimspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[23+i].width=(swimspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 4) + (swimspr[i][spr_extend]<2?0:1);;
		hero_sprite_items[23+i].height=swimspr[i][spr_extend]<2?1:2;
	}
	
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[27+i].tile=divespr[i][spr_tile]-(divespr[i][spr_extend]<2?0:1)-(divespr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[27+i].width=(divespr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 4) + (divespr[i][spr_extend]<2?0:1);;
		hero_sprite_items[27+i].height=divespr[i][spr_extend]<2?1:2;
	}
	
	for(int32_t i=0; i<2; ++i)
	{
		hero_sprite_items[31+i].tile=holdspr[1][i][spr_tile]-(holdspr[1][i][spr_extend]<2?0:1)-(holdspr[1][i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[31+i].width=(holdspr[1][i][spr_extend]<2?1:2) + (holdspr[1][i][spr_extend]<2?0:1);;
		hero_sprite_items[31+i].height=holdspr[1][i][spr_extend]<2?1:2;
	}
	
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[33+i].tile=jumpspr[i][spr_tile]-(jumpspr[i][spr_extend]<2?0:1)-(jumpspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[33+i].width=(jumpspr[i][spr_extend]<2?1:2) * 3 + (jumpspr[i][spr_extend]<2?0:1);
		hero_sprite_items[33+i].height=jumpspr[i][spr_extend]<2?1:2;
	}
	
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[37+i].tile=chargespr[i][spr_tile]-(chargespr[i][spr_extend]<2?0:1)-(chargespr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[37+i].width=(chargespr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 9) + (chargespr[i][spr_extend]<2?0:1);
		hero_sprite_items[37+i].height=chargespr[i][spr_extend]<2?1:2;
	}
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[41+i].tile=revslashspr[i][spr_tile]-(revslashspr[i][spr_extend]<2?0:1)-(revslashspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[41+i].width=(revslashspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 1, 1, 6) + (revslashspr[i][spr_extend]<2?0:1);;
		hero_sprite_items[41+i].height=revslashspr[i][spr_extend]<2?1:2;
	}
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[45+i].tile=fallingspr[i][spr_tile]-(fallingspr[i][spr_extend]<2?0:1)-(fallingspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[45+i].width=(fallingspr[i][spr_extend]<2?1:2) * 7;
		hero_sprite_items[45+i].height=fallingspr[i][spr_extend]<2?1:2;
	}
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[49+i].tile=liftingspr[i][spr_tile]-(liftingspr[i][spr_extend]<2?0:1)-(liftingspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[49+i].width=(liftingspr[i][spr_extend]<2?1:2) * liftingspr[i][spr_frames] + (liftingspr[i][spr_extend]<2?0:1);
		hero_sprite_items[49+i].height=liftingspr[i][spr_extend]<2?1:2;
	}
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[53+i].tile=liftingwalkspr[i][spr_tile]-(liftingwalkspr[i][spr_extend]<2?0:1)-(liftingwalkspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[53+i].width=(liftingwalkspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, (i==0?1:2), 3, 9) + (liftingwalkspr[i][spr_extend]<2?0:1);
		hero_sprite_items[53+i].height=liftingwalkspr[i][spr_extend]<2?1:2;
	}
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[57+i].tile=drowningspr[i][spr_tile]-(drowningspr[i][spr_extend]<2?0:1)-(drowningspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[57+i].width=(drowningspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 3);
		hero_sprite_items[57+i].height=drowningspr[i][spr_extend]<2?1:2;
	}
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[61+i].tile=drowning_lavaspr[i][spr_tile]-(drowning_lavaspr[i][spr_extend]<2?0:1)-(drowning_lavaspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[61+i].width=(drowning_lavaspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 3);
		hero_sprite_items[61+i].height=drowning_lavaspr[i][spr_extend]<2?1:2;
	}
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[61+i].tile=sideswimspr[i][spr_tile]-(sideswimspr[i][spr_extend]<2?0:1)-(sideswimspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[61+i].width=(sideswimspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 3);
		hero_sprite_items[61+i].height=sideswimspr[i][spr_extend]<2?1:2;
	}
	//69
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[69+i].tile=sideswimslashspr[i][spr_tile]-(sideswimslashspr[i][spr_extend]<2?0:1)-(sideswimslashspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[69+i].width=(sideswimslashspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 1, 1, 6) + (sideswimslashspr[i][spr_extend]<2?0:1);;
		hero_sprite_items[69+i].height=sideswimslashspr[i][spr_extend]<2?1:2;
	}
	//73
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[73+i].tile=sideswimstabspr[i][spr_tile]-(sideswimstabspr[i][spr_extend]<2?0:1)-(sideswimstabspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[73+i].width=(sideswimstabspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 1, 1, 3) + (sideswimstabspr[i][spr_extend]<2?0:1);;
		hero_sprite_items[73+i].height=sideswimstabspr[i][spr_extend]<2?1:2;
	}
	//77
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[77+i].tile=sideswimpoundspr[i][spr_tile]-(sideswimpoundspr[i][spr_extend]<2?0:1)-(sideswimpoundspr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[77+i].width=(sideswimpoundspr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 1, 1, 3) + (sideswimpoundspr[i][spr_extend]<2?0:1);;
		hero_sprite_items[77+i].height=sideswimpoundspr[i][spr_extend]<2?1:2;
	}
	//81
	for(int32_t i=0; i<4; ++i)
	{
		hero_sprite_items[81+i].tile=sideswimchargespr[i][spr_tile]-(sideswimchargespr[i][spr_extend]<2?0:1)-(sideswimchargespr[i][spr_extend]<1?0:TILES_PER_ROW);
		hero_sprite_items[81+i].width=(sideswimchargespr[i][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 9) + (sideswimchargespr[i][spr_extend]<2?0:1);
		hero_sprite_items[81+i].height=sideswimchargespr[i][spr_extend]<2?1:2;
	}
	//85
	hero_sprite_items[85].tile=sideswimholdspr[spr_hold1][spr_tile]-(sideswimholdspr[spr_hold1][spr_extend]<2?0:1)-(sideswimholdspr[spr_hold1][spr_extend]<1?0:TILES_PER_ROW);
	hero_sprite_items[85].width=sideswimholdspr[spr_hold1][spr_extend]<2?1:2;
	hero_sprite_items[85].height=sideswimholdspr[spr_hold1][spr_extend]<2?1:2;
	hero_sprite_items[86].tile=sideswimholdspr[spr_hold2][spr_tile]-(sideswimholdspr[spr_hold2][spr_extend]<2?0:1)-(sideswimholdspr[spr_hold2][spr_extend]<1?0:TILES_PER_ROW);
	hero_sprite_items[86].width=sideswimholdspr[spr_hold2][spr_extend]<2?1:2;
	hero_sprite_items[86].height=sideswimholdspr[spr_hold2][spr_extend]<2?1:2;
	hero_sprite_items[87].tile=sideswimcastingspr[spr_tile]-(sideswimcastingspr[spr_extend]<2?0:1)-(sideswimcastingspr[spr_extend]<1?0:TILES_PER_ROW);
	hero_sprite_items[87].width=(sideswimcastingspr[spr_extend]<2?1:2) + (sideswimcastingspr[spr_extend]<2?0:1);;
	hero_sprite_items[87].height=sideswimcastingspr[spr_extend]<2?1:2;
	hero_sprite_items[88].tile=sidedrowningspr[down][spr_tile]-(sidedrowningspr[down][spr_extend]<2?0:1)-(sidedrowningspr[down][spr_extend]<1?0:TILES_PER_ROW);
	hero_sprite_items[88].width=(sidedrowningspr[down][spr_extend]<2?1:2) * quick_select_3(a_style, 2, 3, 3);
	hero_sprite_items[88].height=sidedrowningspr[down][spr_extend]<2?1:2;
}

void register_used_tiles()
{
	bool ignore_frames=false;
	
	for(int32_t t=0; t<NEWMAXTILES; ++t)
	{
		used_tile_table[t]=false;
	}
	reset_combo_animations();
	reset_combo_animations2();
	for(int32_t u=0; u<MAXCOMBOS; u++)
	{
		/* This doesn't account for ASkipX, or ASkipY... Time to rewrite.
		for(int32_t t=zc_max(combobuf[u].o_tile,0); t<zc_min(combobuf[u].o_tile+zc_max(combobuf[u].frames,1),NEWMAXTILES); ++t)
		{
			used_tile_table[t]=true;
		} */
		do
		{
			used_tile_table[combobuf[u].tile] = true;
			animate(combobuf[u], true);
		}
		while(combobuf[u].tile != combobuf[u].o_tile);
	}
	
	for(int32_t u=0; u<iLast; u++)
	{
		for(int32_t t=zc_max(itemsbuf[u].tile,0); t<zc_min(itemsbuf[u].tile+zc_max(itemsbuf[u].frames,1),NEWMAXTILES); ++t)
		{
			used_tile_table[t]=true;
		}
	}
	
	bool BSZ2=get_bit(quest_rules,qr_BSZELDA)!=0;
	
	for(int32_t u=0; u<wLast; u++)
	{
		int32_t m=0;
		ignore_frames=false;
		
		switch(u)
		{
		case wSWORD:
		case wWSWORD:
		case wMSWORD:
		case wXSWORD:
			m=3+((wpnsbuf[u].type==3)?1:0);
			break;
			
		case wSWORDSLASH:
		case wWSWORDSLASH:
		case wMSWORDSLASH:
		case wXSWORDSLASH:
			m=4;
			break;
			
		case iwMMeter:
			m=9;
			break;
			
		case wBRANG:
		case wMBRANG:
		case wFBRANG:
			m=BSZ2?1:3;
			break;
			
		case wBOOM:
		case wSBOOM:
		case ewBOOM:
		case ewSBOOM:
			ignore_frames=true;
			m=2;
			break;
			
		case wWAND:
			m=1;
			break;
			
		case wMAGIC:
			m=1;
			break;
			
		case wARROW:
		case wSARROW:
		case wGARROW:
		case ewARROW:
			m=1;
			break;
			
		case wHAMMER:
			m=8;
			break;
			
		case wHSHEAD:
			m=1;
			break;
			
		case wHSCHAIN_H:
			m=1;
			break;
			
		case wHSCHAIN_V:
			m=1;
			break;
			
		case wHSHANDLE:
			m=1;
			break;
			
		case iwDeath:
			m=BSZ2?4:2;
			break;
			
		case iwSpawn:
			m=3;
			break;
			
		default:
			m=0;
			break;
		}
		
		for(int32_t t=zc_max(wpnsbuf[u].tile,0); t<zc_min(wpnsbuf[u].tile+zc_max((ignore_frames?0:wpnsbuf[u].frames),1)+m,NEWMAXTILES); ++t)
		{
			used_tile_table[t]=true;
		}
		
		used_tile_table[54]=true;
		used_tile_table[55]=true;
	}
	
	setup_hero_sprite_items();
	
//  i=move_intersection_rs(TILECOL(hero_sprite_items[u].tile), TILEROW(hero_sprite_items[u].tile), hero_sprite_items[u].width, hero_sprite_items[u].height, selection_first, selection_last);
	for(int32_t u=0; u<num_hspr; u++)
	{
		for(int32_t r=zc_max(TILEROW(hero_sprite_items[u].tile),0); r<zc_min(TILEROW(hero_sprite_items[u].tile)+zc_max(hero_sprite_items[u].height,1),TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
		{
			for(int32_t c=zc_max(TILECOL(hero_sprite_items[u].tile),0); c<zc_min(TILECOL(hero_sprite_items[u].tile)+zc_max(hero_sprite_items[u].width,1),TILES_PER_ROW); ++c)
			{
				used_tile_table[(r*TILES_PER_ROW)+c]=true;
			}
		}
	}
	
	BSZ2=(zinit.subscreen>2);
	map_styles_items[0].tile=misc.colors.blueframe_tile;
	map_styles_items[1].tile=misc.colors.HCpieces_tile;
	map_styles_items[1].width=zinit.hcp_per_hc;
	map_styles_items[2].tile=misc.colors.triforce_tile;
	map_styles_items[2].width=BSZ2?2:1;
	map_styles_items[2].height=BSZ2?3:1;
	map_styles_items[3].tile=misc.colors.triframe_tile;
	map_styles_items[3].width=BSZ2?7:6;
	map_styles_items[3].height=BSZ2?7:3;
	map_styles_items[4].tile=misc.colors.overworld_map_tile;
	map_styles_items[5].tile=misc.colors.dungeon_map_tile;
	
	for(int32_t u=0; u<6; u++)
	{
		for(int32_t r=zc_max(TILEROW(map_styles_items[u].tile),0); r<zc_min(TILEROW(map_styles_items[u].tile)+zc_max(map_styles_items[u].height,1),TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
		{
			for(int32_t c=zc_max(TILECOL(map_styles_items[u].tile),0); c<zc_min(TILECOL(map_styles_items[u].tile)+zc_max(map_styles_items[u].width,1),TILES_PER_ROW); ++c)
			{
				used_tile_table[(r*TILES_PER_ROW)+c]=true;
			}
		}
	}
	
	for(int32_t u=0; u<4; u++)
	{
		for(int32_t t=zc_max(misc.icons[u],0); t<zc_min(misc.icons[u]+1,NEWMAXTILES); ++t)
		{
			used_tile_table[t]=true;
		}
	}
	
	BSZ2=(zinit.subscreen>2);
	
	for(int32_t d=0; d<MAXDMAPS; d++)
	{
		dmap_map_items[0].tile=DMaps[d].minimap_1_tile;
		dmap_map_items[1].tile=DMaps[d].minimap_2_tile;
		dmap_map_items[2].tile=DMaps[d].largemap_1_tile;
		dmap_map_items[2].width=BSZ2?7:9;
		dmap_map_items[3].tile=DMaps[d].largemap_2_tile;
		dmap_map_items[3].width=BSZ2?7:9;
		
		for(int32_t u=0; u<4; u++)
		{
			for(int32_t r=zc_max(TILEROW(dmap_map_items[u].tile),0); r<zc_min(TILEROW(dmap_map_items[u].tile)+zc_max(dmap_map_items[u].height,1),TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
			{
				for(int32_t c=zc_max(TILECOL(dmap_map_items[u].tile),0); c<zc_min(TILECOL(dmap_map_items[u].tile)+zc_max(dmap_map_items[u].width,1),TILES_PER_ROW); ++c)
				{
					used_tile_table[(r*TILES_PER_ROW)+c]=true;
				}
			}
		}
	}
	
	bool newtiles=get_bit(quest_rules,qr_NEWENEMYTILES)!=0;
	int32_t u;
	
	for(u=0; u<eMAXGUYS; u++)
	{
		bool darknut=false;
		int32_t gleeok=0;
		
		switch(u)
		{
		case eDKNUT1:
		case eDKNUT2:
		case eDKNUT3:
		case eDKNUT5:
			darknut=true;
			break;
		}
		
		if(u>=eGLEEOK1 && u<=eGLEEOK4)
		{
			gleeok=1;
		}
		else if(u>=eGLEEOK1F && u<=eGLEEOK4F)
		{
			gleeok=2;
		}
		
		if(newtiles)
		{
			if(guysbuf[u].e_tile==0)
			{
				continue;
			}
			
			if(guysbuf[u].e_height==0)
			{
				for(int32_t t=zc_max(guysbuf[u].e_tile,0); t<zc_min(guysbuf[u].e_tile+zc_max(guysbuf[u].e_width, 0),NEWMAXTILES); ++t)
				{
					used_tile_table[t]=true;
				}
			}
			else
			{
				for(int32_t r=zc_max(TILEROW(guysbuf[u].e_tile),0); r<zc_min(TILEROW(guysbuf[u].e_tile)+zc_max(guysbuf[u].e_height,1),TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
				{
					for(int32_t c=zc_max(TILECOL(guysbuf[u].e_tile),0); c<zc_min(TILECOL(guysbuf[u].e_tile)+zc_max(guysbuf[u].e_width,1),TILES_PER_ROW); ++c)
					{
						used_tile_table[(r*TILES_PER_ROW)+c]=true;
					}
				}
			}
			
			if(darknut)
			{
				for(int32_t r=zc_max(TILEROW(guysbuf[u].e_tile+120),0); r<zc_min(TILEROW(guysbuf[u].e_tile+120)+zc_max(guysbuf[u].e_height,1),TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
				{
					for(int32_t c=zc_max(TILECOL(guysbuf[u].e_tile+120),0); c<zc_min(TILECOL(guysbuf[u].e_tile+120)+zc_max(guysbuf[u].e_width,1),TILES_PER_ROW); ++c)
					{
						used_tile_table[(r*TILES_PER_ROW)+c]=true;
					}
				}
			}
			else if(u==eGANON)
			{
				for(int32_t r=zc_max(TILEROW(guysbuf[u].e_tile),0); r<zc_min(TILEROW(guysbuf[u].e_tile)+4,TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
				{
					for(int32_t c=zc_max(TILECOL(guysbuf[u].e_tile),0); c<zc_min(TILECOL(guysbuf[u].e_tile)+20,TILES_PER_ROW); ++c)
					{
						used_tile_table[(r*TILES_PER_ROW)+c]=true;
					}
				}
			}
			else if(gleeok)
			{
				for(int32_t j=0; j<4; ++j)
				{
					for(int32_t r=zc_max(TILEROW(guysbuf[u].e_tile+8)+(j<<1)+(gleeok>1?1:0),0); r<zc_min(TILEROW(guysbuf[u].e_tile+8)+(j<<1)+(gleeok>1?1:0)+1,TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
					{
						for(int32_t c=zc_max(TILECOL(guysbuf[u].e_tile+(gleeok>1?-4:8)),0); c<zc_min(TILECOL(guysbuf[u].e_tile+(gleeok>1?-4:8))+4,TILES_PER_ROW); ++c)
						{
							used_tile_table[(r*TILES_PER_ROW)+c]=true;
						}
					}
				}
				
				int32_t c3=TILECOL(guysbuf[u].e_tile)+(gleeok>1?-12:0);
				int32_t r3=TILEROW(guysbuf[u].e_tile)+(gleeok>1?17:8);
				
				for(int32_t r=zc_max(r3,0); r<zc_min(r3+3,TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
				{
					for(int32_t c=zc_max(c3,0); c<zc_min(c3+20,TILES_PER_ROW); ++c)
					{
						used_tile_table[(r*TILES_PER_ROW)+c]=true;
					}
				}
				
				for(int32_t r=zc_max(r3+3,0); r<zc_min(r3+3+6,TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
				{
					for(int32_t c=zc_max(c3,0); c<zc_min(c3+16,TILES_PER_ROW); ++c)
					{
						used_tile_table[(r*TILES_PER_ROW)+c]=true;
					}
				}
			}
		}
		else
		{
			if(guysbuf[u].tile==0)
			{
				continue;
			}
			
			if(guysbuf[u].height==0)
			{
				for(int32_t t=zc_max(guysbuf[u].tile,0); t<zc_min(guysbuf[u].tile+zc_max(guysbuf[u].width, 0),NEWMAXTILES); ++t)
				{
					used_tile_table[t]=true;
				}
			}
			else
			{
				for(int32_t r=zc_max(TILEROW(guysbuf[u].tile),0); r<zc_min(TILEROW(guysbuf[u].tile)+zc_max(guysbuf[u].height,1),TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
				{
					for(int32_t c=zc_max(TILECOL(guysbuf[u].tile),0); c<zc_min(TILECOL(guysbuf[u].tile)+zc_max(guysbuf[u].width,1),TILES_PER_ROW); ++c)
					{
						used_tile_table[(r*TILES_PER_ROW)+c]=true;
					}
				}
			}
			
			if(guysbuf[u].s_tile!=0)
			{
				if(guysbuf[u].s_height==0)
				{
					for(int32_t t=zc_max(guysbuf[u].s_tile,0); t<zc_min(guysbuf[u].s_tile+zc_max(guysbuf[u].s_width, 0),NEWMAXTILES); ++t)
					{
						used_tile_table[t]=true;
					}
				}
				else
				{
					for(int32_t r=zc_max(TILEROW(guysbuf[u].s_tile),0); r<zc_min(TILEROW(guysbuf[u].s_tile)+zc_max(guysbuf[u].s_height,1),TILE_ROWS_PER_PAGE*TILE_PAGES); ++r)
					{
						for(int32_t c=zc_max(TILECOL(guysbuf[u].s_tile),0); c<zc_min(TILECOL(guysbuf[u].s_tile)+zc_max(guysbuf[u].s_width,1),TILES_PER_ROW); ++c)
						{
							used_tile_table[(r*TILES_PER_ROW)+c]=true;
						}
					}
				}
			}
		}
	}
}

bool overlay_tiles(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move, int32_t cs, bool backwards)
{
	bool ctrl=(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]);
	bool copied=false;
	copied=overlay_tiles_united(tile,tile2,copy,copycnt,rect_sel,move,cs,backwards);
	
	if(copied)
	{
		saved=false;
	}
	
	return copied;
}

bool overlay_tiles_mass(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move, int32_t cs, bool backwards)
{
	bool ctrl=(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]);
	bool copied=false;
	copied=overlay_tile_united_mass(tile,tile2,copy,copycnt,rect_sel,move,cs,backwards);
	
	if(copied)
	{
		if(!ctrl)
		{
			copy=-1;
			tile2=tile;
		}
		
		saved=false;
	}
	
	return copied;
}

void handle_hero_sprite_move(bool* move_hero_sprites_list, int32_t diff)
{
	for(size_t u=0; u<num_hspr; ++u)
	{
		if(move_hero_sprites_list[u])
		{
			switch(u)
			{
				case hspr_walk_up: case hspr_walk_down:
				case hspr_walk_left: case hspr_walk_right:
					walkspr[u][spr_tile]+=diff;
					break;
					
				case hspr_slash_up: case hspr_slash_down:
				case hspr_slash_left: case hspr_slash_right:
					slashspr[u-hspr_slash_up][spr_tile]+=diff;
					break;
					
				case hspr_stab_up: case hspr_stab_down:
				case hspr_stab_left: case hspr_stab_right:
					stabspr[u-hspr_stab_up][spr_tile]+=diff;
					break;
					
				case hspr_pound_up: case hspr_pound_down:
				case hspr_pound_left: case hspr_pound_right:
					poundspr[u-hspr_pound_up][spr_tile]+=diff;
					break;
					
				case hspr_holdland_1: case hspr_holdland_2:
					holdspr[0][u-hspr_holdland_1][spr_tile]+=diff;
					break;
					
				case hspr_casting:
					castingspr[spr_tile]+=diff;
					break;
					
				case hspr_float_up: case hspr_float_down:
				case hspr_float_left: case hspr_float_right:
					floatspr[u-hspr_float_up][spr_tile]+=diff;
					break;
					
				case hspr_swim_up: case hspr_swim_down:
				case hspr_swim_left: case hspr_swim_right: 
					swimspr[u-hspr_swim_up][spr_tile]+=diff;
					break;
					
				case hspr_dive_up: case hspr_dive_down:
				case hspr_dive_left: case hspr_dive_right:
					divespr[u-hspr_dive_up][spr_tile]+=diff;
					break;
					
				case hspr_holdwater_1: case hspr_holdwater_2:
					holdspr[1][u-hspr_holdwater_1][spr_tile]+=diff;
					break;
					
				case hspr_jump_up: case hspr_jump_down:
				case hspr_jump_left: case hspr_jump_right:
					jumpspr[u-hspr_jump_up][spr_tile]+=diff;
					break;
					
				case hspr_charge_up: case hspr_charge_down:
				case hspr_charge_left: case hspr_charge_right:
					chargespr[u-hspr_charge_up][spr_tile]+=diff;
					break;
				case hspr_slash_2_up: case hspr_slash_2_down:
				case hspr_slash_2_left: case hspr_slash_2_right:
					revslashspr[u-hspr_slash_2_up][spr_tile]+=diff;
					break;
				case hspr_falling_up: case hspr_falling_down:
				case hspr_falling_left: case hspr_falling_right:
					fallingspr[u-hspr_falling_up][spr_tile]+=diff;
					break;
				case hspr_lifting_up: case hspr_lifting_down:
				case hspr_lifting_left: case hspr_lifting_right:
					liftingspr[u-hspr_lifting_up][spr_tile]+=diff;
					break;
				case hspr_liftwalk_up: case hspr_liftwalk_down:
				case hspr_liftwalk_left: case hspr_liftwalk_right:
					liftingwalkspr[u-hspr_liftwalk_up][spr_tile]+=diff;
					break;
				case hspr_drown_up: case hspr_drown_down:
				case hspr_drown_left: case hspr_drown_right:
					drowningspr[u-hspr_drown_up][spr_tile]+=diff;
					break;
				case hspr_lavadrown_up: case hspr_lavadrown_down:
				case hspr_lavadrown_left: case hspr_lavadrown_right:
					drowning_lavaspr[u-hspr_lavadrown_up][spr_tile]+=diff;
					break;
				case hspr_sideswim_up: case hspr_sideswim_down:
				case hspr_sideswim_left: case hspr_sideswim_right:
					sideswimspr[u-hspr_sideswim_up][spr_tile]+=diff;
					break;
				case hspr_sideslash_up: case hspr_sideslash_down:
				case hspr_sideslash_left: case hspr_sideslash_right:
					sideswimslashspr[u-hspr_sideslash_up][spr_tile]+=diff;
					break;
				case hspr_sidestab_up: case hspr_sidestab_down:
				case hspr_sidestab_left: case hspr_sidestab_right:
					sideswimstabspr[u-hspr_sidestab_up][spr_tile]+=diff;
					break;
				case hspr_sidepound_up: case hspr_sidepound_down:
				case hspr_sidepound_left: case hspr_sidepound_right:
					sideswimpoundspr[u-hspr_sidepound_up][spr_tile]+=diff;
					break;
				case hspr_sidecharge_up: case hspr_sidecharge_down:
				case hspr_sidecharge_left: case hspr_sidecharge_right:
					sideswimchargespr[u-hspr_sidecharge_up][spr_tile]+=diff;
					break;
				case hspr_holdsidewater_1: case hspr_holdsidewater_2: 
					sideswimholdspr[u-hspr_holdsidewater_1][spr_tile]+=diff;
					break;
				case hspr_sideswimcasting:
					sideswimcastingspr[spr_tile]+=diff;
					break;
				case hspr_sidedrowning: 
					sidedrowningspr[down][spr_tile]+=diff;
					break;
			}
		}
	}
}

bool overlay_tiles_united(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect, bool move, int32_t cs, bool backwards)
{
	bool alt=(key[KEY_ALT]||key[KEY_ALTGR]);
	bool shift=(key[KEY_LSHIFT] || key[KEY_RSHIFT]);
	bool ignore_frames=false;
	
	// if tile>tile2 then swap them
	if(tile>tile2)
	{
		zc_swap(tile, tile2);
	}
	
	// alt=copy from right
	// shift=copy from bottom
	
	int32_t copies=copycnt;
	int32_t dest_first=tile;
	int32_t dest_last=tile2;
	int32_t src_first=copy;
	int32_t src_last=copy+copies-1;
	
	int32_t dest_top=0;
	int32_t dest_bottom=0;
	int32_t src_top=0;
	int32_t src_bottom=0;
	int32_t src_left=0, src_right=0;
	int32_t src_width=0, src_height=0;
	int32_t dest_left=0, dest_right=0;
	int32_t dest_width=0, dest_height=0;
	int32_t rows=0, cols=0;
	
	if(rect)
	{
		dest_top=TILEROW(dest_first);
		dest_bottom=TILEROW(dest_last);
		src_top=TILEROW(src_first);
		src_bottom=TILEROW(src_last);
		
		src_left= zc_min(TILECOL(src_first),TILECOL(src_last));
		src_right=zc_max(TILECOL(src_first),TILECOL(src_last));
		src_first=(src_top  * TILES_PER_ROW)+src_left;
		src_last= (src_bottom*TILES_PER_ROW)+src_right;
		
		dest_left= zc_min(TILECOL(dest_first),TILECOL(dest_last));
		dest_right=zc_max(TILECOL(dest_first),TILECOL(dest_last));
		dest_first=(dest_top  * TILES_PER_ROW)+dest_left;
		dest_last= (dest_bottom*TILES_PER_ROW)+dest_right;
		
		//if no dest range set, then set one
		if((dest_first==dest_last)&&(src_first!=src_last))
		{
			if(alt)
			{
				dest_left=dest_right-(src_right-src_left);
			}
			else
			{
				dest_right=dest_left+(src_right-src_left);
			}
			
			if(shift)
			{
				dest_top=dest_bottom-(src_bottom-src_top);
			}
			else
			{
				dest_bottom=dest_top+(src_bottom-src_top);
			}
			
			dest_first=(dest_top  * TILES_PER_ROW)+dest_left;
			dest_last= (dest_bottom*TILES_PER_ROW)+dest_right;
		}
		else
		{
			if(dest_right-dest_left<src_right-src_left) //destination is shorter than source
			{
				if(alt) //copy from right tile instead of left
				{
					src_left=src_right-(dest_right-dest_left);
				}
				else //copy from left tile
				{
					src_right=src_left+(dest_right-dest_left);
				}
			}
			else if(dest_right-dest_left>src_right-src_left)  //destination is longer than source
			{
				if(alt) //copy from right tile instead of left
				{
					dest_left=dest_right-(src_right-src_left);
				}
				else //copy from left tile
				{
					dest_right=dest_left+(src_right-src_left);
				}
			}
			
			if(dest_bottom-dest_top<src_bottom-src_top) //destination is shorter than source
			{
				if(shift) //copy from bottom tile instead of top
				{
					src_top=src_bottom-(dest_bottom-dest_top);
				}
				else //copy from top tile
				{
					src_bottom=src_top+(dest_bottom-dest_top);
				}
			}
			else if(dest_bottom-dest_top>src_bottom-src_top)  //destination is longer than source
			{
				if(shift) //copy from bottom tile instead of top
				{
					dest_top=dest_bottom-(src_bottom-src_top);
				}
				else //copy from top tile
				{
					dest_bottom=dest_top+(src_bottom-src_top);
				}
			}
			
			src_first=(src_top  * TILES_PER_ROW)+src_left;
			src_last= (src_bottom*TILES_PER_ROW)+src_right;
			dest_first=(dest_top  * TILES_PER_ROW)+dest_left;
			dest_last= (dest_bottom*TILES_PER_ROW)+dest_right;
		}
		
		cols=src_right-src_left+1;
		rows=src_bottom-src_top+1;
		
		dest_width=dest_right-dest_left+1;
		dest_height=dest_bottom-dest_top+1;
		src_width=src_right-src_left+1;
		src_height=src_bottom-src_top+1;
		
	}
	else  //!rect
	{
		//if no dest range set, then set one
		if((dest_first==dest_last)&&(src_first!=src_last))
		{
			if(alt)
			{
				dest_first=dest_last-(src_last-src_first);
			}
			else
			{
				dest_last=dest_first+(src_last-src_first);
			}
		}
		else
		{
			if(dest_last-dest_first<src_last-src_first) //destination is shorter than source
			{
				if(alt) //copy from last tile instead of first
				{
					src_first=src_last-(dest_last-dest_first);
				}
				else //copy from first tile
				{
					src_last=src_first+(dest_last-dest_first);
				}
			}
			else if(dest_last-dest_first>src_last-src_first)  //destination is longer than source
			{
				if(alt) //copy from last tile instead of first
				{
					dest_first=dest_last-(src_last-src_first);
				}
				else //copy from first tile
				{
					dest_last=dest_first+(src_last-src_first);
				}
			}
		}
		
		copies=dest_last-dest_first+1;
	}
	
	
	
	char buf[80], buf2[80], buf3[80], buf4[80];
	sprintf(buf, " ");
	sprintf(buf2, " ");
	sprintf(buf3, " ");
	sprintf(buf4, " ");
	
	// warn if range extends beyond last tile
	sprintf(buf4, "Some tiles will not be %s", move?"moved.":"copied.");
	
	if(dest_last>=NEWMAXTILES)
	{
		sprintf(buf4, "%s operation cancelled.", move?"Move":"Copy");
		jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", NULL, 'o', 0, lfont);
		return false;
//fix this below to allow the operation to complete with a modified start or end instead of just cancelling
		//if (jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", "&Cancel", 'o', 'c', lfont)==2)
		// {
		//  return false;
		// }
	}
	
	char *tile_move_list_text = new char[65535];
	char temptext[80];
	
	sprintf(buf, "Destination Warning");
	tile_move_list_dlg[0].dp=buf;
	tile_move_list_dlg[0].dp2=lfont;
	bool found;
	bool flood;
	
	int32_t i;
	bool *move_combo_list = new bool[MAXCOMBOS];
	bool *move_items_list = new bool[iMax];
	bool *move_weapons_list = new bool[wMAX];
	bool move_hero_sprites_list[num_hspr];
	bool move_mapstyles_list[6];
	//bool move_subscreenobjects_list[MAXCUSTOMSUBSCREENS*MAXSUBSCREENITEMS];
	bool move_game_icons_list[4];
	bool move_dmap_maps_list[MAXDMAPS][4];
	//    bool move_enemies_list[eMAXGUYS];  //to be implemented once custom enemies are in
	
	// warn if paste overwrites other defined tiles or
	// if delete erases other defined tiles
	int32_t selection_first=0, selection_last=0, selection_left=0, selection_top=0, selection_width=0, selection_height=0;
	bool done = false;
	
	for(int32_t q=0; q<2 && !done; ++q)
	{
	
		switch(q)
		{
		case 0:
			selection_first=dest_first;
			selection_last=dest_last;
			selection_left=dest_left;
			selection_top=dest_top;
			selection_width=dest_width;
			selection_height=dest_height;
			break;
			
		case 1:
			selection_first=src_first;
			selection_last=src_last;
			selection_left=src_left;
			selection_top=src_top;
			selection_width=src_width;
			selection_height=src_height;
			break;
		}
		
		if(move||q==0)
		{
			//check combos
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				
				for(int32_t u=0; u<MAXCOMBOS; u++)
				{
					move_combo_list[u]=false;
					
					if(rect)
					{
						i = move_intersection_sr(combobuf[u], selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i = move_intersection_ss(combobuf[u], selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(combobuf[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%d\n", u);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_combo_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following combos");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check items
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_bii_list(false);
				
				for(int32_t u=0; u<iMax; u++)
				{
					move_items_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_sr(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+zc_max(itemsbuf[bii[u].i].frames,1)-1, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+zc_max(itemsbuf[bii[u].i].frames,1)-1, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(itemsbuf[bii[u].i].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", bii[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_items_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following items");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check weapons/misc
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_biw_list();
				bool BSZ2=get_bit(quest_rules,qr_BSZELDA)!=0;
				
				for(int32_t u=0; u<wMAX; u++)
				{
					ignore_frames=false;
					move_weapons_list[u]=false;
					int32_t m=0;
					
					switch(biw[u].i)
					{
					case wSWORD:
					case wWSWORD:
					case wMSWORD:
					case wXSWORD:
						m=3+((wpnsbuf[biw[u].i].type==3)?1:0);
						break;
						
					case wSWORDSLASH:
					case wWSWORDSLASH:
					case wMSWORDSLASH:
					case wXSWORDSLASH:
						m=4;
						break;
						
					case iwMMeter:
						m=9;
						break;
						
					case wBRANG:
					case wMBRANG:
					case wFBRANG:
						m=BSZ2?1:3;
						break;
						
					case wBOOM:
					case wSBOOM:
					case ewBOOM:
					case ewSBOOM:
						ignore_frames=true;
						m=2;
						break;
						
					case wWAND:
						m=1;
						break;
						
					case wMAGIC:
						m=1;
						break;
						
					case wARROW:
					case wSARROW:
					case wGARROW:
					case ewARROW:
						m=1;
						break;
						
					case wHAMMER:
						m=8;
						break;
						
					case wHSHEAD:
						m=1;
						break;
						
					case wHSCHAIN_H:
						m=1;
						break;
						
					case wHSCHAIN_V:
						m=1;
						break;
						
					case wHSHANDLE:
						m=1;
						break;
						
					case iwDeath:
						m=BSZ2?4:2;
						break;
						
					case iwSpawn:
						m=3;
						break;
						
					default:
						m=0;
						break;
					}
					
					if(rect)
					{
						i=move_intersection_sr(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+zc_max((ignore_frames?0:wpnsbuf[biw[u].i].frames),1)-1+m, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+zc_max((ignore_frames?0:wpnsbuf[biw[u].i].frames),1)-1+m, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(wpnsbuf[biw[u].i].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", biw[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_weapons_list[u]=true;
						}
					}
					
					if((u==3)||(u==9))
					{
						if(rect)
						{
							i=move_intersection_sr(54, 55, selection_left, selection_top, selection_width, selection_height);
						}
						else
						{
							i=move_intersection_ss(54, 55, selection_first, selection_last);
						}
						
						if(i!=ti_none)
						{
							sprintf(temptext, "%s Impact (not shown in sprite list)\n", (u==3)?"Arrow":"Boomerang");
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following weapons");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check Player sprites
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				setup_hero_sprite_items();
				
				for(int32_t u=0; u<num_hspr; u++)
				{
					move_hero_sprites_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_rr(TILECOL(hero_sprite_items[u].tile), TILEROW(hero_sprite_items[u].tile), hero_sprite_items[u].width, hero_sprite_items[u].height, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_rs(TILECOL(hero_sprite_items[u].tile), TILEROW(hero_sprite_items[u].tile), hero_sprite_items[u].width, hero_sprite_items[u].height, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(hero_sprite_items[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", hero_sprite_items[u].name);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_hero_sprites_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following Player sprites");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "sprites will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done =true;
						}
					}
				}
			}
			
			//Check subscreen objects
			//Tried to have a go at this but I think it's a bit too complicated for me at the moment.
			//Might come back to it another time and see what I can do ~Joe123
			/*if(!done){
				 for(int32_t u=0;u<MAXCUSTOMSUBSCREENS;u++){
					 if(!custom_subscreen[u].ss_type) continue;
					 for(int32_t v=0;v<MAXSUBSCREENITEMS;v++){
						  if(custom_subscreen[u].objects[v].type != ssoTILEBLOCK) continue;
						  subscreen_items[0].tile = custom_subscreen[u].objects[v].d1;
						  subscreen_items[0].width = custom_subscreen[u].objects[v].w;
						  subscreen_items[0].height = custom_subscreen[u].objects[v].h;
					 }
				 }
			}*/
			
			//check map styles
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				bool BSZ2=(zinit.subscreen>2);
				map_styles_items[0].tile=misc.colors.blueframe_tile;
				map_styles_items[1].tile=misc.colors.HCpieces_tile;
				map_styles_items[1].width=zinit.hcp_per_hc;
				map_styles_items[2].tile=misc.colors.triforce_tile;
				map_styles_items[2].width=BSZ2?2:1;
				map_styles_items[2].height=BSZ2?3:1;
				map_styles_items[3].tile=misc.colors.triframe_tile;
				map_styles_items[3].width=BSZ2?7:6;
				map_styles_items[3].height=BSZ2?7:3;
				map_styles_items[4].tile=misc.colors.overworld_map_tile;
				map_styles_items[5].tile=misc.colors.dungeon_map_tile;
				
				for(int32_t u=0; u<6; u++)
				{
					move_mapstyles_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_rr(TILECOL(map_styles_items[u].tile), TILEROW(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_rs(TILECOL(map_styles_items[u].tile), TILEROW(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(map_styles_items[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", map_styles_items[u].name);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_mapstyles_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following map style");
					
					if(move)
					{
						sprintf(buf3, "items will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "items will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check game icons
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				const char *icon_title[4]=
				{
					"No Ring / Green Ring", "Blue Ring", "Red Ring", "Golden Ring"
				};
				
				for(int32_t u=0; u<4; u++)
				{
					move_game_icons_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_sr(misc.icons[u], misc.icons[u], selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(misc.icons[u], misc.icons[u], selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(misc.icons[u]!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", icon_title[u]);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_game_icons_list[u]=true;
						}
					}
				}
				
				if(rect)
				{
					i=move_intersection_sr(41, 41, selection_left, selection_top, selection_width, selection_height);
				}
				else
				{
					i=move_intersection_ss(41, 41, selection_first, selection_last);
				}
				
				if((i!=ti_none)) // &&(41!=0))  //this is for when the quest sword can change
				{
					sprintf(temptext, "Quest Sword");
					
					if(strlen(tile_move_list_text)<65000)
					{
						strcat(tile_move_list_text, temptext);
					}
					else
					{
						if(!flood)
						{
							strcat(tile_move_list_text, "...\n...\n...\nmany others");
							flood=true;
						}
					}
					
					found=true;
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following quest icons");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be overwritten by this process.  Proceed?");
						sprintf(buf4, " ");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check dmap maps
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				bool BSZ2=(zinit.subscreen>2);
				
				for(int32_t t=0; t<MAXDMAPS; t++)
				{
					dmap_map_items[0].tile=DMaps[t].minimap_1_tile;
					dmap_map_items[1].tile=DMaps[t].minimap_2_tile;
					dmap_map_items[2].tile=DMaps[t].largemap_1_tile;
					dmap_map_items[2].width=BSZ2?7:9;
					dmap_map_items[3].tile=DMaps[t].largemap_2_tile;
					dmap_map_items[3].width=BSZ2?7:9;
					
					for(int32_t u=0; u<4; u++)
					{
						move_dmap_maps_list[t][u]=false;
						
						if(rect)
						{
							i=move_intersection_rr(TILECOL(dmap_map_items[u].tile), TILEROW(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, selection_left, selection_top, selection_width, selection_height);
						}
						else
						{
							i=move_intersection_rs(TILECOL(dmap_map_items[u].tile), TILEROW(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, selection_first, selection_last);
						}
						
						if((i!=ti_none)&&(dmap_map_items[u].tile!=0))
						{
							if(i==ti_broken || q==0)
							{
								sprintf(temptext, "DMap %d %s\n", t, dmap_map_items[u].name);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
							else if(i==ti_encompass)
							{
								move_dmap_maps_list[t][u]=true;
							}
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following dmap-specific");
					
					if(move)
					{
						sprintf(buf3, "subscreen maps will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "subscreen maps will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check enemies
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_bie_list(false);
				bool newtiles=get_bit(quest_rules,qr_NEWENEMYTILES)!=0;
				int32_t u;
				
				for(u=0; u<eMAXGUYS; u++)
				{
					const guydata& enemy=guysbuf[bie[u].i];
					bool darknut=false;
					int32_t gleeok=0;
					
					if(enemy.family==eeWALK && ((enemy.flags&(inv_back|inv_front|inv_left|inv_right))!=0))
						darknut=true;
					else if(enemy.family==eeGLEEOK)
					{
						// Not certain this is the right thing to check...
						if(enemy.misc3==0)
							gleeok=1;
						else
							gleeok=2;
					}
					
					// Dummied out enemies
					if(bie[u].i>=eOCTO1S && bie[u].i<e177)
					{
						if(old_guy_string[bie[u].i][strlen(old_guy_string[bie[u].i])-1]==' ')
						{
							continue;
						}
					}
					
					if(newtiles)
					{
						if(guysbuf[bie[u].i].e_tile==0)
						{
							continue;
						}
						
						if(guysbuf[bie[u].i].e_height==0)
						{
							if(rect)
							{
								i=move_intersection_sr(guysbuf[bie[u].i].e_tile, guysbuf[bie[u].i].e_tile+zc_max(guysbuf[bie[u].i].e_width-1, 0), selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_ss(guysbuf[bie[u].i].e_tile, guysbuf[bie[u].i].e_tile+zc_max(guysbuf[bie[u].i].e_width-1, 0), selection_first, selection_last);
							}
						}
						else
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_first, selection_last);
							}
						}
						
						if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
						{
							sprintf(temptext, "%s\n", bie[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						
						if(darknut)
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile+120), TILEROW(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile+120), TILEROW(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_first, selection_last);
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s (broken shield)\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
						else if(enemy.family==eeGANON && i==ti_none)
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile)+2, 20, 4, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile)+2, 20, 4, selection_first, selection_last);
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
						else if(gleeok && i==ti_none)
						{
							for(int32_t j=0; j<4 && i==ti_none; ++j)
							{
								if(rect)
								{
									i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), TILEROW(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), TILEROW(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, selection_first, selection_last);
								}
							}
							
							if(i==ti_none)
							{
								int32_t c=TILECOL(guysbuf[bie[u].i].e_tile)+(gleeok>1?-12:0);
								int32_t r=TILEROW(guysbuf[bie[u].i].e_tile)+(gleeok>1?17:8);
								
								if(rect)
								{
									i=move_intersection_rr(c, r, 20, 3, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(c, r, 20, 3, selection_first, selection_last);
								}
								
								if(i==ti_none)
								{
									if(rect)
									{
										i=move_intersection_rr(c, r+3, 16, 6, selection_left, selection_top, selection_width, selection_height);
									}
									else
									{
										i=move_intersection_rs(c, r+3, 16, 6, selection_first, selection_last);
									}
								}
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
					}
					else
					{
						if(guysbuf[bie[u].i].tile==0)
						{
							continue;
						}
						else if(guysbuf[bie[u].i].height==0)
						{
							if(rect)
							{
								i=move_intersection_sr(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+zc_max(guysbuf[bie[u].i].width-1, 0), selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_ss(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+zc_max(guysbuf[bie[u].i].width-1, 0), selection_first, selection_last);
							}
						}
						else
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].tile), TILEROW(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].tile), TILEROW(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, selection_first, selection_last);
							}
						}
						
						if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
						{
							sprintf(temptext, "%s\n", bie[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						
						if(guysbuf[bie[u].i].s_tile!=0)
						{
							if(guysbuf[bie[u].i].s_height==0)
							{
								if(rect)
								{
									i=move_intersection_sr(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+zc_max(guysbuf[bie[u].i].s_width-1, 0), selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_ss(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+zc_max(guysbuf[bie[u].i].s_width-1, 0), selection_first, selection_last);
								}
							}
							else
							{
								if(rect)
								{
									i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].s_tile), TILEROW(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].s_tile), TILEROW(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, selection_first, selection_last);
								}
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s (%s)\n", bie[u].s, darknut?"broken shield":"secondary tiles");
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following enemies");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
		}
	}
	
	//
	// copy tiles and delete if needed (move)
	
	if(!done)
	{
		go_tiles();
		
		int32_t diff=dest_first-src_first;
		
		if(rect)
		{
			for(int32_t r=0; r<rows; ++r)
			{
				for(int32_t c=0; c<cols; ++c)
				{
					int32_t dt=(dest_first+((r*TILES_PER_ROW)+c));
					int32_t st=(src_first+((r*TILES_PER_ROW)+c));
					
					if(dt>=NEWMAXTILES)
						continue;
					
			overlay_tile(newtilebuf,dt,st,cs,backwards);
			
				}
			}
		}
		else
		{
			for(int32_t c=0; c<copies; ++c)
			{
				int32_t dt=(dest_first+c);
				int32_t st=(src_first+c);
				
				if(dt>=NEWMAXTILES)
					continue;
		
		overlay_tile(newtilebuf,dt,st,cs,backwards);
		
				if(move)
				{
					if(st<dest_first||st>(dest_first+c-1))
						reset_tile(newtilebuf, st, tf4Bit);
				}
			}
		}
		
		if(move)
		{
			for(int32_t u=0; u<MAXCOMBOS; u++)
			{
				if(move_combo_list[u])
				{
					combobuf[u].tile+=diff;
				}
			}
			
			for(int32_t u=0; u<iMax; u++)
			{
				if(move_items_list[u])
				{
					itemsbuf[bii[u].i].tile+=diff;
				}
			}
			
			for(int32_t u=0; u<wMAX; u++)
			{
				if(move_weapons_list[u])
				{
					wpnsbuf[biw[u].i].tile+=diff;
				}
			}
			
			handle_hero_sprite_move(move_hero_sprites_list,diff);
			
			for(int32_t u=0; u<6; u++)
			{
				if(move_mapstyles_list[u])
				{
					switch(u)
					{
					case 0:
						misc.colors.blueframe_tile+=diff;
						break;
						
					case 1:
						misc.colors.HCpieces_tile+=diff;
						break;
						
					case 2:
						misc.colors.triforce_tile+=diff;
						break;
						
					case 3:
						misc.colors.triframe_tile+=diff;
						break;
						
					case 4:
						misc.colors.overworld_map_tile+=diff;
						break;
						
					case 5:
						misc.colors.dungeon_map_tile+=diff;
						break;
					}
				}
			}
			
			for(int32_t u=0; u<4; u++)
			{
				if(move_game_icons_list[u])
				{
					misc.icons[u]+=diff;
				}
			}
			
			for(int32_t t=0; t<MAXDMAPS; t++)
			{
				for(int32_t u=0; u<4; u++)
				{
					move_dmap_maps_list[t][u]=false;
					
					if(move_dmap_maps_list[t][u])
					{
						switch(u)
						{
						case 0:
							DMaps[t].minimap_1_tile+=diff;
							break;
							
						case 1:
							DMaps[t].minimap_2_tile+=diff;
							break;
							
						case 2:
							DMaps[t].largemap_1_tile+=diff;
							break;
							
						case 3:
							DMaps[t].largemap_2_tile+=diff;
							break;
						}
					}
				}
			}
		}
	}
	
	//now that tiles have moved, fix these buffers -DD
	register_blank_tiles();
	register_used_tiles();
	
	delete[] tile_move_list_text;
	delete[] move_combo_list;
	delete[] move_items_list;
	delete[] move_weapons_list;
	
	if(done)
		return false;
		
	return true;
}
//

bool overlay_tile_united_mass(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect, bool move, int32_t cs, bool backwards)
{
	bool alt=(key[KEY_ALT]||key[KEY_ALTGR]);
	bool shift=(key[KEY_LSHIFT] || key[KEY_RSHIFT]);
	bool ignore_frames=false;
	
	// if tile>tile2 then swap them
	if(tile>tile2)
	{
		zc_swap(tile, tile2);
	}
	
	// alt=copy from right
	// shift=copy from bottom
	
	int32_t copies=copycnt;
	int32_t dest_first=tile;
	int32_t dest_last=tile2;
	int32_t src_first=copy;
	int32_t src_last=copy+copies-1;
	
	int32_t dest_top=0;
	int32_t dest_bottom=0;
	int32_t src_top=0;
	int32_t src_bottom=0;
	int32_t src_left=0, src_right=0;
	int32_t src_width=0, src_height=0;
	int32_t dest_left=0, dest_right=0;
	int32_t dest_width=0, dest_height=0;
	int32_t rows=0, cols=0;
	
	if(rect)
	{
		dest_top=TILEROW(dest_first);
		dest_bottom=TILEROW(dest_last);
		src_top=TILEROW(src_first);
		src_bottom=TILEROW(src_last);
		
		src_left= zc_min(TILECOL(src_first),TILECOL(src_last));
		src_right=zc_max(TILECOL(src_first),TILECOL(src_last));
		src_first=(src_top  * TILES_PER_ROW)+src_left;
		src_last= (src_bottom*TILES_PER_ROW)+src_right;
		
		dest_left= zc_min(TILECOL(dest_first),TILECOL(dest_last));
		dest_right=zc_max(TILECOL(dest_first),TILECOL(dest_last));
		dest_first=(dest_top  * TILES_PER_ROW)+dest_left;
		dest_last= (dest_bottom*TILES_PER_ROW)+dest_right;
		
		//if no dest range set, then set one
		if((dest_first==dest_last)&&(src_first!=src_last))
		{
			if(alt)
			{
				dest_left=dest_right-(src_right-src_left);
			}
			else
			{
				dest_right=dest_left+(src_right-src_left);
			}
			
			if(shift)
			{
				dest_top=dest_bottom-(src_bottom-src_top);
			}
			else
			{
				dest_bottom=dest_top+(src_bottom-src_top);
			}
			
			dest_first=(dest_top  * TILES_PER_ROW)+dest_left;
			dest_last= (dest_bottom*TILES_PER_ROW)+dest_right;
		}
		else
		{
			if(dest_right-dest_left<src_right-src_left) //destination is shorter than source
			{
				if(alt) //copy from right tile instead of left
				{
					src_left=src_right-(dest_right-dest_left);
				}
				else //copy from left tile
				{
					src_right=src_left+(dest_right-dest_left);
				}
			}
			else if(dest_right-dest_left>src_right-src_left)  //destination is longer than source
			{
				if(alt) //copy from right tile instead of left
				{
					dest_left=dest_right-(src_right-src_left);
				}
				else //copy from left tile
				{
					dest_right=dest_left+(src_right-src_left);
				}
			}
			
			if(dest_bottom-dest_top<src_bottom-src_top) //destination is shorter than source
			{
				if(shift) //copy from bottom tile instead of top
				{
					src_top=src_bottom-(dest_bottom-dest_top);
				}
				else //copy from top tile
				{
					src_bottom=src_top+(dest_bottom-dest_top);
				}
			}
			else if(dest_bottom-dest_top>src_bottom-src_top)  //destination is longer than source
			{
				if(shift) //copy from bottom tile instead of top
				{
					dest_top=dest_bottom-(src_bottom-src_top);
				}
				else //copy from top tile
				{
					dest_bottom=dest_top+(src_bottom-src_top);
				}
			}
			
			src_first=(src_top  * TILES_PER_ROW)+src_left;
			src_last= (src_bottom*TILES_PER_ROW)+src_right;
			dest_first=(dest_top  * TILES_PER_ROW)+dest_left;
			dest_last= (dest_bottom*TILES_PER_ROW)+dest_right;
		}
		
		cols=src_right-src_left+1;
		rows=src_bottom-src_top+1;
		
		dest_width=dest_right-dest_left+1;
		dest_height=dest_bottom-dest_top+1;
		src_width=src_right-src_left+1;
		src_height=src_bottom-src_top+1;
		
	}
	else  //!rect
	{
		//if no dest range set, then set one
		if((dest_first==dest_last)&&(src_first!=src_last))
		{
			if(alt)
			{
				dest_first=dest_last-(src_last-src_first);
			}
			else
			{
				dest_last=dest_first+(src_last-src_first);
			}
		}
		else
		{
			if(dest_last-dest_first<src_last-src_first) //destination is shorter than source
			{
				if(alt) //copy from last tile instead of first
				{
					src_first=src_first;
				}
				else //copy from first tile
				{
					src_last=src_first;
				}
			}
			else if(dest_last-dest_first>src_last-src_first)  //destination is longer than source
			{
				if(alt) //copy from last tile instead of first
				{
					dest_first=dest_last-(src_last-src_first);
				}
				else //copy from first tile
				{
					dest_last=dest_first+(src_last-src_first);
				}
			}
		}
		
		copies=dest_last-dest_first+1;
	}
	
	
	
	char buf[80], buf2[80], buf3[80], buf4[80];
	sprintf(buf, " ");
	sprintf(buf2, " ");
	sprintf(buf3, " ");
	sprintf(buf4, " ");
	
	// warn if range extends beyond last tile
	sprintf(buf4, "Some tiles will not be %s", move?"moved.":"copied.");
	
	if(dest_last>=NEWMAXTILES)
	{
		sprintf(buf4, "%s operation cancelled.", move?"Move":"Copy");
		jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", NULL, 'o', 0, lfont);
		return false;
//fix this below to allow the operation to complete with a modified start or end instead of just cancelling
		//if (jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", "&Cancel", 'o', 'c', lfont)==2)
		// {
		//  return false;
		// }
	}
	
	char *tile_move_list_text = new char[65535];
	char temptext[80];
	
	sprintf(buf, "Destination Warning");
	tile_move_list_dlg[0].dp=buf;
	tile_move_list_dlg[0].dp2=lfont;
	bool found;
	bool flood;
	
	int32_t i;
	bool *move_combo_list = new bool[MAXCOMBOS];
	bool *move_items_list = new bool[iMax];
	bool *move_weapons_list = new bool[wMAX];
	bool move_hero_sprites_list[num_hspr];
	bool move_mapstyles_list[6];
	//bool move_subscreenobjects_list[MAXCUSTOMSUBSCREENS*MAXSUBSCREENITEMS];
	bool move_game_icons_list[4];
	bool move_dmap_maps_list[MAXDMAPS][4];
	//    bool move_enemies_list[eMAXGUYS];  //to be implemented once custom enemies are in
	
	// warn if paste overwrites other defined tiles or
	// if delete erases other defined tiles
	int32_t selection_first=0, selection_last=0, selection_left=0, selection_top=0, selection_width=0, selection_height=0;
	bool done = false;
	
	for(int32_t q=0; q<2 && !done; ++q)
	{
	
		switch(q)
		{
		case 0:
			selection_first=dest_first;
			selection_last=dest_last;
			selection_left=dest_left;
			selection_top=dest_top;
			selection_width=dest_width;
			selection_height=dest_height;
			break;
			
		case 1:
			selection_first=src_first;
			selection_last=src_last;
			selection_left=src_left;
			selection_top=src_top;
			selection_width=src_width;
			selection_height=src_height;
			break;
		}
		
		if(move||q==0)
		{
			//check combos
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				
				for(int32_t u=0; u<MAXCOMBOS; u++)
				{
					move_combo_list[u]=false;
					
					if(rect)
					{
						i = move_intersection_sr(combobuf[u], selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i = move_intersection_ss(combobuf[u], selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(combobuf[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%d\n", u);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_combo_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following combos");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check items
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_bii_list(false);
				
				for(int32_t u=0; u<iMax; u++)
				{
					move_items_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_sr(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+zc_max(itemsbuf[bii[u].i].frames,1)-1, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+zc_max(itemsbuf[bii[u].i].frames,1)-1, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(itemsbuf[bii[u].i].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", bii[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_items_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following items");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check weapons/misc
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_biw_list();
				bool BSZ2=get_bit(quest_rules,qr_BSZELDA)!=0;
				
				for(int32_t u=0; u<wMAX; u++)
				{
					ignore_frames=false;
					move_weapons_list[u]=false;
					int32_t m=0;
					
					switch(biw[u].i)
					{
					case wSWORD:
					case wWSWORD:
					case wMSWORD:
					case wXSWORD:
						m=3+((wpnsbuf[biw[u].i].type==3)?1:0);
						break;
						
					case wSWORDSLASH:
					case wWSWORDSLASH:
					case wMSWORDSLASH:
					case wXSWORDSLASH:
						m=4;
						break;
						
					case iwMMeter:
						m=9;
						break;
						
					case wBRANG:
					case wMBRANG:
					case wFBRANG:
						m=BSZ2?1:3;
						break;
						
					case wBOOM:
					case wSBOOM:
					case ewBOOM:
					case ewSBOOM:
						ignore_frames=true;
						m=2;
						break;
						
					case wWAND:
						m=1;
						break;
						
					case wMAGIC:
						m=1;
						break;
						
					case wARROW:
					case wSARROW:
					case wGARROW:
					case ewARROW:
						m=1;
						break;
						
					case wHAMMER:
						m=8;
						break;
						
					case wHSHEAD:
						m=1;
						break;
						
					case wHSCHAIN_H:
						m=1;
						break;
						
					case wHSCHAIN_V:
						m=1;
						break;
						
					case wHSHANDLE:
						m=1;
						break;
						
					case iwDeath:
						m=BSZ2?4:2;
						break;
						
					case iwSpawn:
						m=3;
						break;
						
					default:
						m=0;
						break;
					}
					
					if(rect)
					{
						i=move_intersection_sr(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+zc_max((ignore_frames?0:wpnsbuf[biw[u].i].frames),1)-1+m, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+zc_max((ignore_frames?0:wpnsbuf[biw[u].i].frames),1)-1+m, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(wpnsbuf[biw[u].i].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", biw[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_weapons_list[u]=true;
						}
					}
					
					if((u==3)||(u==9))
					{
						if(rect)
						{
							i=move_intersection_sr(54, 55, selection_left, selection_top, selection_width, selection_height);
						}
						else
						{
							i=move_intersection_ss(54, 55, selection_first, selection_last);
						}
						
						if(i!=ti_none)
						{
							sprintf(temptext, "%s Impact (not shown in sprite list)\n", (u==3)?"Arrow":"Boomerang");
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following weapons");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check Player sprites
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				setup_hero_sprite_items();
				
				for(int32_t u=0; u<num_hspr; u++)
				{
					move_hero_sprites_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_rr(TILECOL(hero_sprite_items[u].tile), TILEROW(hero_sprite_items[u].tile), hero_sprite_items[u].width, hero_sprite_items[u].height, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_rs(TILECOL(hero_sprite_items[u].tile), TILEROW(hero_sprite_items[u].tile), hero_sprite_items[u].width, hero_sprite_items[u].height, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(hero_sprite_items[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", hero_sprite_items[u].name);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_hero_sprites_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following Player sprites");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "sprites will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done =true;
						}
					}
				}
			}
			
			//Check subscreen objects
			//Tried to have a go at this but I think it's a bit too complicated for me at the moment.
			//Might come back to it another time and see what I can do ~Joe123
			/*if(!done){
				 for(int32_t u=0;u<MAXCUSTOMSUBSCREENS;u++){
					 if(!custom_subscreen[u].ss_type) continue;
					 for(int32_t v=0;v<MAXSUBSCREENITEMS;v++){
						  if(custom_subscreen[u].objects[v].type != ssoTILEBLOCK) continue;
						  subscreen_items[0].tile = custom_subscreen[u].objects[v].d1;
						  subscreen_items[0].width = custom_subscreen[u].objects[v].w;
						  subscreen_items[0].height = custom_subscreen[u].objects[v].h;
					 }
				 }
			}*/
			
			//check map styles
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				bool BSZ2=(zinit.subscreen>2);
				map_styles_items[0].tile=misc.colors.blueframe_tile;
				map_styles_items[1].tile=misc.colors.HCpieces_tile;
				map_styles_items[1].width=zinit.hcp_per_hc;
				map_styles_items[2].tile=misc.colors.triforce_tile;
				map_styles_items[2].width=BSZ2?2:1;
				map_styles_items[2].height=BSZ2?3:1;
				map_styles_items[3].tile=misc.colors.triframe_tile;
				map_styles_items[3].width=BSZ2?7:6;
				map_styles_items[3].height=BSZ2?7:3;
				map_styles_items[4].tile=misc.colors.overworld_map_tile;
				map_styles_items[5].tile=misc.colors.dungeon_map_tile;
				
				for(int32_t u=0; u<6; u++)
				{
					move_mapstyles_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_rr(TILECOL(map_styles_items[u].tile), TILEROW(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_rs(TILECOL(map_styles_items[u].tile), TILEROW(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(map_styles_items[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", map_styles_items[u].name);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_mapstyles_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following map style");
					
					if(move)
					{
						sprintf(buf3, "items will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "items will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check game icons
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				const char *icon_title[4]=
				{
					"No Ring / Green Ring", "Blue Ring", "Red Ring", "Golden Ring"
				};
				
				for(int32_t u=0; u<4; u++)
				{
					move_game_icons_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_sr(misc.icons[u], misc.icons[u], selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(misc.icons[u], misc.icons[u], selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(misc.icons[u]!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", icon_title[u]);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_game_icons_list[u]=true;
						}
					}
				}
				
				if(rect)
				{
					i=move_intersection_sr(41, 41, selection_left, selection_top, selection_width, selection_height);
				}
				else
				{
					i=move_intersection_ss(41, 41, selection_first, selection_last);
				}
				
				if((i!=ti_none)) // &&(41!=0))  //this is for when the quest sword can change
				{
					sprintf(temptext, "Quest Sword");
					
					if(strlen(tile_move_list_text)<65000)
					{
						strcat(tile_move_list_text, temptext);
					}
					else
					{
						if(!flood)
						{
							strcat(tile_move_list_text, "...\n...\n...\nmany others");
							flood=true;
						}
					}
					
					found=true;
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following quest icons");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be overwritten by this process.  Proceed?");
						sprintf(buf4, " ");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check dmap maps
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				bool BSZ2=(zinit.subscreen>2);
				
				for(int32_t t=0; t<MAXDMAPS; t++)
				{
					dmap_map_items[0].tile=DMaps[t].minimap_1_tile;
					dmap_map_items[1].tile=DMaps[t].minimap_2_tile;
					dmap_map_items[2].tile=DMaps[t].largemap_1_tile;
					dmap_map_items[2].width=BSZ2?7:9;
					dmap_map_items[3].tile=DMaps[t].largemap_2_tile;
					dmap_map_items[3].width=BSZ2?7:9;
					
					for(int32_t u=0; u<4; u++)
					{
						move_dmap_maps_list[t][u]=false;
						
						if(rect)
						{
							i=move_intersection_rr(TILECOL(dmap_map_items[u].tile), TILEROW(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, selection_left, selection_top, selection_width, selection_height);
						}
						else
						{
							i=move_intersection_rs(TILECOL(dmap_map_items[u].tile), TILEROW(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, selection_first, selection_last);
						}
						
						if((i!=ti_none)&&(dmap_map_items[u].tile!=0))
						{
							if(i==ti_broken || q==0)
							{
								sprintf(temptext, "DMap %d %s\n", t, dmap_map_items[u].name);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
							else if(i==ti_encompass)
							{
								move_dmap_maps_list[t][u]=true;
							}
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following dmap-specific");
					
					if(move)
					{
						sprintf(buf3, "subscreen maps will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "subscreen maps will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check enemies
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_bie_list(false);
				bool newtiles=get_bit(quest_rules,qr_NEWENEMYTILES)!=0;
				int32_t u;
				
				for(u=0; u<eMAXGUYS; u++)
				{
					const guydata& enemy=guysbuf[bie[u].i];
					bool darknut=false;
					int32_t gleeok=0;
					
					if(enemy.family==eeWALK && ((enemy.flags&(inv_back|inv_front|inv_left|inv_right))!=0))
						darknut=true;
					else if(enemy.family==eeGLEEOK)
					{
						// Not certain this is the right thing to check...
						if(enemy.misc3==0)
							gleeok=1;
						else
							gleeok=2;
					}
					
					// Dummied out enemies
					if(bie[u].i>=eOCTO1S && bie[u].i<e177)
					{
						if(old_guy_string[bie[u].i][strlen(old_guy_string[bie[u].i])-1]==' ')
						{
							continue;
						}
					}
					
					if(newtiles)
					{
						if(guysbuf[bie[u].i].e_tile==0)
						{
							continue;
						}
						
						if(guysbuf[bie[u].i].e_height==0)
						{
							if(rect)
							{
								i=move_intersection_sr(guysbuf[bie[u].i].e_tile, guysbuf[bie[u].i].e_tile+zc_max(guysbuf[bie[u].i].e_width-1, 0), selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_ss(guysbuf[bie[u].i].e_tile, guysbuf[bie[u].i].e_tile+zc_max(guysbuf[bie[u].i].e_width-1, 0), selection_first, selection_last);
							}
						}
						else
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_first, selection_last);
							}
						}
						
						if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
						{
							sprintf(temptext, "%s\n", bie[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						
						if(darknut)
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile+120), TILEROW(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile+120), TILEROW(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_first, selection_last);
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s (broken shield)\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
						else if(enemy.family==eeGANON && i==ti_none)
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile)+2, 20, 4, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile)+2, 20, 4, selection_first, selection_last);
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
						else if(gleeok && i==ti_none)
						{
							for(int32_t j=0; j<4 && i==ti_none; ++j)
							{
								if(rect)
								{
									i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), TILEROW(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), TILEROW(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, selection_first, selection_last);
								}
							}
							
							if(i==ti_none)
							{
								int32_t c=TILECOL(guysbuf[bie[u].i].e_tile)+(gleeok>1?-12:0);
								int32_t r=TILEROW(guysbuf[bie[u].i].e_tile)+(gleeok>1?17:8);
								
								if(rect)
								{
									i=move_intersection_rr(c, r, 20, 3, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(c, r, 20, 3, selection_first, selection_last);
								}
								
								if(i==ti_none)
								{
									if(rect)
									{
										i=move_intersection_rr(c, r+3, 16, 6, selection_left, selection_top, selection_width, selection_height);
									}
									else
									{
										i=move_intersection_rs(c, r+3, 16, 6, selection_first, selection_last);
									}
								}
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
					}
					else
					{
						if(guysbuf[bie[u].i].tile==0)
						{
							continue;
						}
						else if(guysbuf[bie[u].i].height==0)
						{
							if(rect)
							{
								i=move_intersection_sr(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+zc_max(guysbuf[bie[u].i].width-1, 0), selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_ss(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+zc_max(guysbuf[bie[u].i].width-1, 0), selection_first, selection_last);
							}
						}
						else
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].tile), TILEROW(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].tile), TILEROW(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, selection_first, selection_last);
							}
						}
						
						if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
						{
							sprintf(temptext, "%s\n", bie[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						
						if(guysbuf[bie[u].i].s_tile!=0)
						{
							if(guysbuf[bie[u].i].s_height==0)
							{
								if(rect)
								{
									i=move_intersection_sr(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+zc_max(guysbuf[bie[u].i].s_width-1, 0), selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_ss(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+zc_max(guysbuf[bie[u].i].s_width-1, 0), selection_first, selection_last);
								}
							}
							else
							{
								if(rect)
								{
									i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].s_tile), TILEROW(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].s_tile), TILEROW(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, selection_first, selection_last);
								}
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s (%s)\n", bie[u].s, darknut?"broken shield":"secondary tiles");
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following enemies");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
		}
	}
	
	//
	// copy tiles and delete if needed (move)
	
	if(!done)
	{
		go_tiles();
		
		int32_t diff=dest_first-src_first;
		
		if(rect)
		{
			for(int32_t r=0; r<rows; ++r)
			{
				for(int32_t c=0; c<cols; ++c)
				{
					int32_t dt=(dest_first+((r*TILES_PER_ROW)+c));
					int32_t st=(src_first);
					
					if(dt>=NEWMAXTILES)
						continue;
					
			overlay_tile(newtilebuf,dt,st,cs,backwards);
			
				}
			}
		}
		else
		{
			for(int32_t c=0; c<copies; ++c)
			{
				int32_t dt=(dest_first+c);
				int32_t st=(src_first);
				
				if(dt>=NEWMAXTILES)
					continue;
		
		overlay_tile(newtilebuf,dt,st,cs,backwards);
		
				if(move)
				{
					if(st<dest_first||st>(dest_first+c-1))
						reset_tile(newtilebuf, st, tf4Bit);
				}
			}
		}
		
		if(move)
		{
			for(int32_t u=0; u<MAXCOMBOS; u++)
			{
				if(move_combo_list[u])
				{
					combobuf[u].tile+=diff;
				}
			}
			
			for(int32_t u=0; u<iMax; u++)
			{
				if(move_items_list[u])
				{
					itemsbuf[bii[u].i].tile+=diff;
				}
			}
			
			for(int32_t u=0; u<wMAX; u++)
			{
				if(move_weapons_list[u])
				{
					wpnsbuf[biw[u].i].tile+=diff;
				}
			}
			
			handle_hero_sprite_move(move_hero_sprites_list,diff);
			
			for(int32_t u=0; u<6; u++)
			{
				if(move_mapstyles_list[u])
				{
					switch(u)
					{
					case 0:
						misc.colors.blueframe_tile+=diff;
						break;
						
					case 1:
						misc.colors.HCpieces_tile+=diff;
						break;
						
					case 2:
						misc.colors.triforce_tile+=diff;
						break;
						
					case 3:
						misc.colors.triframe_tile+=diff;
						break;
						
					case 4:
						misc.colors.overworld_map_tile+=diff;
						break;
						
					case 5:
						misc.colors.dungeon_map_tile+=diff;
						break;
					}
				}
			}
			
			for(int32_t u=0; u<4; u++)
			{
				if(move_game_icons_list[u])
				{
					misc.icons[u]+=diff;
				}
			}
			
			for(int32_t t=0; t<MAXDMAPS; t++)
			{
				for(int32_t u=0; u<4; u++)
				{
					move_dmap_maps_list[t][u]=false;
					
					if(move_dmap_maps_list[t][u])
					{
						switch(u)
						{
						case 0:
							DMaps[t].minimap_1_tile+=diff;
							break;
							
						case 1:
							DMaps[t].minimap_2_tile+=diff;
							break;
							
						case 2:
							DMaps[t].largemap_1_tile+=diff;
							break;
							
						case 3:
							DMaps[t].largemap_2_tile+=diff;
							break;
						}
					}
				}
			}
		}
	}
	
	//now that tiles have moved, fix these buffers -DD
	register_blank_tiles();
	register_used_tiles();
	
	delete[] tile_move_list_text;
	delete[] move_combo_list;
	delete[] move_items_list;
	delete[] move_weapons_list;
	
	if(done)
		return false;
		
	return true;
}
//
bool do_movetile_united(tile_move_data const& tmd)
{
	bool ignore_frames=false;
	char buf[80], buf2[80], buf3[80], buf4[80];
	sprintf(buf, " ");
	sprintf(buf2, " ");
	sprintf(buf3, " ");
	sprintf(buf4, " ");
	
	// warn if range extends beyond last tile
	sprintf(buf4, "Some tiles will not be %s", tmd.move?"moved.":"copied.");
	
	if(tmd.dest_last>=NEWMAXTILES)
	{
		sprintf(buf4, "%s operation cancelled.", tmd.move?"Move":"Copy");
		jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", NULL, 'o', 0, lfont);
		return false;
//fix this below to allow the operation to complete with a modified start or end instead of just cancelling
		//if (jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", "&Cancel", 'o', 'c', lfont)==2)
		// {
		//  return false;
		// }
	}
	
	char *tile_move_list_text = new char[65535];
	char temptext[80];
	
	sprintf(buf, "Destination Warning");
	tile_move_list_dlg[0].dp=buf;
	tile_move_list_dlg[0].dp2=lfont;
	bool found;
	bool flood;
	
	int32_t i;
	bool *move_combo_list = new bool[MAXCOMBOS];
	bool *move_items_list = new bool[iMax];
	bool *move_weapons_list = new bool[wMAX];
	bool *move_enemy_list = new bool[eMAXGUYS];
	bool move_hero_sprites_list[num_hspr];
	bool move_mapstyles_list[6];
	//bool move_subscreenobjects_list[MAXCUSTOMSUBSCREENS*MAXSUBSCREENITEMS];
	bool move_game_icons_list[4];
	bool move_dmap_maps_list[MAXDMAPS][4];
	//    bool move_enemies_list[eMAXGUYS];  //to be implemented once custom enemies are in
	
	// warn if paste overwrites other defined tiles or
	// if delete erases other defined tiles
	int32_t selection_first=0, selection_last=0, selection_left=0, selection_top=0, selection_width=0, selection_height=0;
	bool done = false;
	bool first = true;
	bool newtiles=get_bit(quest_rules,qr_NEWENEMYTILES)!=0;
	int32_t diff = 0;
	for(int32_t q=tmd.move?1:0; q>=0 && !done; --q)
	{
		switch(q)
		{
			case 0:
				if(tmd.move)
					diff = tmd.dest_first-tmd.src_first;
				selection_first=tmd.dest_first;
				selection_last=tmd.dest_last;
				selection_left=tmd.dest_left;
				selection_top=tmd.dest_top;
				selection_width=tmd.dest_width;
				selection_height=tmd.dest_height;
				break;
				
			case 1: case 2:
				selection_first=tmd.src_first;
				selection_last=tmd.src_last;
				selection_left=tmd.src_left;
				selection_top=tmd.src_top;
				selection_width=tmd.src_width;
				selection_height=tmd.src_height;
				break;
		}
		
		{
			//check combos
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				
				for(int32_t u=0; u<MAXCOMBOS; u++)
				{
					if(first) move_combo_list[u]=false;
					else if(move_combo_list[u]) continue;
					
					if(tmd.rect)
					{
						i=move_intersection_sr(combobuf[u], selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(combobuf[u], selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(combobuf[u].o_tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%d\n", u);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if (i == ti_encompass)
						{
							move_combo_list[u] = true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following combos");
					
					if(tmd.move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check items
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_bii_list(false);
				
				for(int32_t u=0; u<iMax; u++)
				{
					if(first) move_items_list[u]=false;
					else if(move_items_list[u]) continue;
					
					if(tmd.rect)
					{
						i=move_intersection_sr(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+zc_max(itemsbuf[bii[u].i].frames,1)-1, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+zc_max(itemsbuf[bii[u].i].frames,1)-1, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(itemsbuf[bii[u].i].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", bii[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_items_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following items");
					
					if(tmd.move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check weapons/misc
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_biw_list();
				bool BSZ2=get_bit(quest_rules,qr_BSZELDA)!=0;
				
				for(int32_t u=0; u<wMAX; u++)
				{
					ignore_frames=false;
					if(first) move_weapons_list[u]=false;
					else if(move_weapons_list[u]) continue;
					
					int32_t m=0;
					
					switch(biw[u].i)
					{
					case wSWORD:
					case wWSWORD:
					case wMSWORD:
					case wXSWORD:
						m=3+((wpnsbuf[biw[u].i].type==3)?1:0);
						break;
						
					case wSWORDSLASH:
					case wWSWORDSLASH:
					case wMSWORDSLASH:
					case wXSWORDSLASH:
						m=4;
						break;
						
					case iwMMeter:
						m=9;
						break;
						
					case wBRANG:
					case wMBRANG:
					case wFBRANG:
						m=BSZ2?1:3;
						break;
						
					case wBOOM:
					case wSBOOM:
					case ewBOOM:
					case ewSBOOM:
						ignore_frames=true;
						m=2;
						break;
						
					case wWAND:
						m=1;
						break;
						
					case wMAGIC:
						m=1;
						break;
						
					case wARROW:
					case wSARROW:
					case wGARROW:
					case ewARROW:
						m=1;
						break;
						
					case wHAMMER:
						m=8;
						break;
						
					case wHSHEAD:
						m=1;
						break;
						
					case wHSCHAIN_H:
						m=1;
						break;
						
					case wHSCHAIN_V:
						m=1;
						break;
						
					case wHSHANDLE:
						m=1;
						break;
						
					case iwDeath:
						m=BSZ2?4:2;
						break;
						
					case iwSpawn:
						m=3;
						break;
						
					default:
						m=0;
						break;
					}
					
					if(tmd.rect)
					{
						i=move_intersection_sr(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+zc_max((ignore_frames?0:wpnsbuf[biw[u].i].frames),1)-1+m, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+zc_max((ignore_frames?0:wpnsbuf[biw[u].i].frames),1)-1+m, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(wpnsbuf[biw[u].i].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", biw[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_weapons_list[u]=true;
						}
					}
					
					if((u==3)||(u==9))
					{
						if(tmd.rect)
						{
							i=move_intersection_sr(54, 55, selection_left, selection_top, selection_width, selection_height);
						}
						else
						{
							i=move_intersection_ss(54, 55, selection_first, selection_last);
						}
						
						if(i!=ti_none)
						{
							sprintf(temptext, "%s Impact (not shown in sprite list)\n", (u==3)?"Arrow":"Boomerang");
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following weapons");
					
					if(tmd.move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check Player sprites
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				setup_hero_sprite_items();
				
				for(int32_t u=0; u<num_hspr; u++)
				{
					if(first) move_hero_sprites_list[u]=false;
					else if(move_hero_sprites_list[u]) continue;
					
					if(tmd.rect)
					{
						i=move_intersection_rr(TILECOL(hero_sprite_items[u].tile), TILEROW(hero_sprite_items[u].tile), hero_sprite_items[u].width, hero_sprite_items[u].height, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_rs(TILECOL(hero_sprite_items[u].tile), TILEROW(hero_sprite_items[u].tile), hero_sprite_items[u].width, hero_sprite_items[u].height, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(hero_sprite_items[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", hero_sprite_items[u].name);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_hero_sprites_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following Player sprites");
					
					if(tmd.move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "sprites will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done =true;
						}
					}
				}
			}
			
			//Check subscreen objects
			//Tried to have a go at this but I think it's a bit too complicated for me at the moment.
			//Might come back to it another time and see what I can do ~Joe123
			/*if(!done){
				 for(int32_t u=0;u<MAXCUSTOMSUBSCREENS;u++){
					 if(!custom_subscreen[u].ss_type) continue;
					 for(int32_t v=0;v<MAXSUBSCREENITEMS;v++){
						  if(custom_subscreen[u].objects[v].type != ssoTILEBLOCK) continue;
						  subscreen_items[0].tile = custom_subscreen[u].objects[v].d1;
						  subscreen_items[0].width = custom_subscreen[u].objects[v].w;
						  subscreen_items[0].height = custom_subscreen[u].objects[v].h;
					 }
				 }
			}*/
			
			//check map styles
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				bool BSZ2=(zinit.subscreen>2);
				map_styles_items[0].tile=misc.colors.blueframe_tile;
				map_styles_items[1].tile=misc.colors.HCpieces_tile;
				map_styles_items[1].width=zinit.hcp_per_hc;
				map_styles_items[2].tile=misc.colors.triforce_tile;
				map_styles_items[2].width=BSZ2?2:1;
				map_styles_items[2].height=BSZ2?3:1;
				map_styles_items[3].tile=misc.colors.triframe_tile;
				map_styles_items[3].width=BSZ2?7:6;
				map_styles_items[3].height=BSZ2?7:3;
				map_styles_items[4].tile=misc.colors.overworld_map_tile;
				map_styles_items[5].tile=misc.colors.dungeon_map_tile;
				
				for(int32_t u=0; u<6; u++)
				{
					if(first) move_mapstyles_list[u]=false;
					else if(move_mapstyles_list[u]) continue;
					
					if(tmd.rect)
					{
						i=move_intersection_rr(TILECOL(map_styles_items[u].tile), TILEROW(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_rs(TILECOL(map_styles_items[u].tile), TILEROW(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(map_styles_items[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", map_styles_items[u].name);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_mapstyles_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following map style");
					
					if(tmd.move)
					{
						sprintf(buf3, "items will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "items will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check game icons
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				const char *icon_title[4]=
				{
					"No Ring / Green Ring", "Blue Ring", "Red Ring", "Golden Ring"
				};
				
				for(int32_t u=0; u<4; u++)
				{
					if(first) move_game_icons_list[u]=false;
					else if(move_game_icons_list[u]) continue;
					
					if(tmd.rect)
					{
						i=move_intersection_sr(misc.icons[u], misc.icons[u], selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(misc.icons[u], misc.icons[u], selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(misc.icons[u]!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", icon_title[u]);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_game_icons_list[u]=true;
						}
					}
				}
				
				if(tmd.rect)
				{
					i=move_intersection_sr(41, 41, selection_left, selection_top, selection_width, selection_height);
				}
				else
				{
					i=move_intersection_ss(41, 41, selection_first, selection_last);
				}
				
				if((i!=ti_none)) // &&(41!=0))  //this is for when the quest sword can change
				{
					sprintf(temptext, "Quest Sword");
					
					if(strlen(tile_move_list_text)<65000)
					{
						strcat(tile_move_list_text, temptext);
					}
					else
					{
						if(!flood)
						{
							strcat(tile_move_list_text, "...\n...\n...\nmany others");
							flood=true;
						}
					}
					
					found=true;
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following quest icons");
					
					if(tmd.move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be overwritten by this process.  Proceed?");
						sprintf(buf4, " ");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check dmap maps
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				bool BSZ2=(zinit.subscreen>2);
				
				for(int32_t t=0; t<MAXDMAPS; t++)
				{
					dmap_map_items[0].tile=DMaps[t].minimap_1_tile;
					dmap_map_items[1].tile=DMaps[t].minimap_2_tile;
					dmap_map_items[2].tile=DMaps[t].largemap_1_tile;
					dmap_map_items[2].width=BSZ2?7:9;
					dmap_map_items[3].tile=DMaps[t].largemap_2_tile;
					dmap_map_items[3].width=BSZ2?7:9;
					
					for(int32_t u=0; u<4; u++)
					{
						if(first) move_dmap_maps_list[t][u]=false;
						else if(move_dmap_maps_list[t][u]) continue;
						
						if(tmd.rect)
						{
							i=move_intersection_rr(TILECOL(dmap_map_items[u].tile), TILEROW(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, selection_left, selection_top, selection_width, selection_height);
						}
						else
						{
							i=move_intersection_rs(TILECOL(dmap_map_items[u].tile), TILEROW(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, selection_first, selection_last);
						}
						
						if((i!=ti_none)&&(dmap_map_items[u].tile!=0))
						{
							if(i==ti_broken || q==0)
							{
								sprintf(temptext, "DMap %d %s\n", t, dmap_map_items[u].name);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
							else if(i==ti_encompass)
							{
								move_dmap_maps_list[t][u]=true;
							}
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following dmap-specific");
					
					if(tmd.move)
					{
						sprintf(buf3, "subscreen maps will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "subscreen maps will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check enemies
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_bie_list(false);
				int32_t u;
				
				for(u=0; u<eMAXGUYS; u++)
				{
					if(first) move_enemy_list[u] = false;
					else if(move_enemy_list[u]) continue;
					const guydata& enemy=guysbuf[bie[u].i];
					bool darknut=false;
					int32_t gleeok=0;
					
					if(enemy.family==eeWALK && ((enemy.flags&(inv_back|inv_front|inv_left|inv_right))!=0))
						darknut=true;
					else if(enemy.family==eeGLEEOK)
					{
						// Not certain this is the right thing to check...
						if(enemy.misc3==0)
							gleeok=1;
						else
							gleeok=2;
					}
					
					// Dummied out enemies
					if(bie[u].i>=eOCTO1S && bie[u].i<e177)
					{
						if(old_guy_string[bie[u].i][strlen(old_guy_string[bie[u].i])-1]==' ')
						{
							continue;
						}
					}
					
					if(newtiles)
					{
						if(guysbuf[bie[u].i].e_tile==0)
						{
							continue;
						}
						
						if(guysbuf[bie[u].i].e_height==0)
						{
							if(tmd.rect)
							{
								i=move_intersection_sr(guysbuf[bie[u].i].e_tile, guysbuf[bie[u].i].e_tile+zc_max(guysbuf[bie[u].i].e_width-1, 0), selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_ss(guysbuf[bie[u].i].e_tile, guysbuf[bie[u].i].e_tile+zc_max(guysbuf[bie[u].i].e_width-1, 0), selection_first, selection_last);
							}
						}
						else
						{
							if(tmd.rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_first, selection_last);
							}
						}
						
						if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
						{
							sprintf(temptext, "%s\n", bie[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i == ti_encompass)
							move_enemy_list[u] = true;
						
						if(darknut)
						{
							bool did_move = move_enemy_list[u];
							if(first) move_enemy_list[u] = false;
							if(tmd.rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile+120), TILEROW(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile+120), TILEROW(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_first, selection_last);
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s (broken shield)\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
							else if(i == ti_encompass && did_move)
								move_enemy_list[u] = true;
						}
						else if(enemy.family==eeGANON && i!=ti_broken)
						{
							bool did_move = move_enemy_list[u];
							if(first) move_enemy_list[u] = false;
							if(tmd.rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile)+2, 20, 4, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile)+2, 20, 4, selection_first, selection_last);
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
							else if(i == ti_encompass && did_move)
								move_enemy_list[u] = true;
						}
						else if(gleeok && i!=ti_broken)
						{
							bool did_move = move_enemy_list[u];
							if(first) move_enemy_list[u] = false;
							for(int32_t j=0; j<4 && i==ti_none; ++j)
							{
								if(tmd.rect)
								{
									i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), TILEROW(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), TILEROW(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, selection_first, selection_last);
								}
							}
							
							if(i==ti_none)
							{
								int32_t c=TILECOL(guysbuf[bie[u].i].e_tile)+(gleeok>1?-12:0);
								int32_t r=TILEROW(guysbuf[bie[u].i].e_tile)+(gleeok>1?17:8);
								
								if(tmd.rect)
								{
									i=move_intersection_rr(c, r, 20, 3, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(c, r, 20, 3, selection_first, selection_last);
								}
								
								if(i==ti_none)
								{
									if(tmd.rect)
									{
										i=move_intersection_rr(c, r+3, 16, 6, selection_left, selection_top, selection_width, selection_height);
									}
									else
									{
										i=move_intersection_rs(c, r+3, 16, 6, selection_first, selection_last);
									}
								}
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
							else if(i == ti_encompass && did_move)
								move_enemy_list[u] = true;
						}
					}
					else
					{
						if(guysbuf[bie[u].i].tile==0)
						{
							continue;
						}
						else if(guysbuf[bie[u].i].height==0)
						{
							if(tmd.rect)
							{
								i=move_intersection_sr(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+zc_max(guysbuf[bie[u].i].width-1, 0), selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_ss(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+zc_max(guysbuf[bie[u].i].width-1, 0), selection_first, selection_last);
							}
						}
						else
						{
							if(tmd.rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].tile), TILEROW(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].tile), TILEROW(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, selection_first, selection_last);
							}
						}
						
						if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
						{
							sprintf(temptext, "%s\n", bie[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i == ti_encompass)
							move_enemy_list[u] = true;
						
						if(guysbuf[bie[u].i].s_tile!=0)
						{
							bool did_move = move_enemy_list[u];
							if(first) move_enemy_list[u] = false;
							if(guysbuf[bie[u].i].s_height==0)
							{
								if(tmd.rect)
								{
									i=move_intersection_sr(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+zc_max(guysbuf[bie[u].i].s_width-1, 0), selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_ss(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+zc_max(guysbuf[bie[u].i].s_width-1, 0), selection_first, selection_last);
								}
							}
							else
							{
								if(tmd.rect)
								{
									i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].s_tile), TILEROW(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].s_tile), TILEROW(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, selection_first, selection_last);
								}
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s (%s)\n", bie[u].s, darknut?"broken shield":"secondary tiles");
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
							else if(i == ti_encompass && did_move)
								move_enemy_list[u] = true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following enemies");
					
					if(tmd.move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
		}
		first = false;
	}
	
	//
	// copy tiles and delete if needed (tmd.move)
	
	if(!done)
	{
		go_tiles();
		
		int32_t diff=tmd.dest_first-tmd.src_first;
		
		if(tmd.rect)
		{
			for(int32_t r=0; r<tmd.rows; ++r)
			{
				for(int32_t c=0; c<tmd.cols; ++c)
				{
					int32_t dt=(tmd.dest_first+((r*TILES_PER_ROW)+c));
					int32_t st=(tmd.src_first+((r*TILES_PER_ROW)+c));
					
					if(dt>=NEWMAXTILES)
						continue;
						
					reset_tile(newtilebuf, dt, newundotilebuf[st].format);
					
					for(int32_t j=0; j<tilesize(newundotilebuf[st].format); j++)
					{
						newtilebuf[dt].data[j]=newundotilebuf[st].data[j];
					}
					
					if(tmd.move)
					{
						if((st<tmd.dest_first||st>tmd.dest_first+((tmd.rows-1)*TILES_PER_ROW)+(tmd.cols-1)))
							reset_tile(newtilebuf, st, tf4Bit);
						else
						{
							int32_t destLeft=tmd.dest_first%TILES_PER_ROW;
							int32_t destRight=(tmd.dest_first+tmd.cols-1)%TILES_PER_ROW;
							if(destLeft<=destRight)
							{
								if(st%TILES_PER_ROW<destLeft || st%TILES_PER_ROW>destRight)
									reset_tile(newtilebuf, st, tf4Bit);
							}
							else // Wrapped around
							{
								if(st%TILES_PER_ROW<destLeft && st%TILES_PER_ROW>destRight)
									reset_tile(newtilebuf, st, tf4Bit);
							}
						}
					}
				}
			}
		}
		else
		{
			for(int32_t c=0; c<tmd.copies; ++c)
			{
				int32_t dt=(tmd.dest_first+c);
				int32_t st=(tmd.src_first+c);
				
				if(dt>=NEWMAXTILES)
					continue;
					
				reset_tile(newtilebuf, dt, newundotilebuf[st].format);
				
				for(int32_t j=0; j<tilesize(newundotilebuf[st].format); j++)
				{
					newtilebuf[dt].data[j]=newundotilebuf[st].data[j];
				}
				
				if(tmd.move)
				{
					if(st<tmd.dest_first||st>(tmd.dest_first+c-1))
						reset_tile(newtilebuf, st, tf4Bit);
				}
			}
		}
		
		if(tmd.move)
		{
			for(int32_t u=0; u<MAXCOMBOS; u++)
			{
				if(move_combo_list[u])
				{
					combobuf[u].set_tile(combobuf[u].o_tile+diff);
				}
			}
			
			for(int32_t u=0; u<iMax; u++)
			{
				if(move_items_list[u])
				{
					itemsbuf[bii[u].i].tile+=diff;
				}
			}
			
			for(int32_t u=0; u<wMAX; u++)
			{
				if(move_weapons_list[u])
				{
					wpnsbuf[biw[u].i].tile+=diff;
				}
			}
			
			handle_hero_sprite_move(move_hero_sprites_list,diff);
			
			for(int32_t u=0; u<6; u++)
			{
				if(move_mapstyles_list[u])
				{
					switch(u)
					{
					case 0:
						misc.colors.blueframe_tile+=diff;
						break;
						
					case 1:
						misc.colors.HCpieces_tile+=diff;
						break;
						
					case 2:
						misc.colors.triforce_tile+=diff;
						break;
						
					case 3:
						misc.colors.triframe_tile+=diff;
						break;
						
					case 4:
						misc.colors.overworld_map_tile+=diff;
						break;
						
					case 5:
						misc.colors.dungeon_map_tile+=diff;
						break;
					}
				}
			}
			
			for(int32_t u=0; u<4; u++)
			{
				if(move_game_icons_list[u])
				{
					misc.icons[u]+=diff;
				}
			}
			
			for(int32_t t=0; t<MAXDMAPS; t++)
			{
				for(int32_t u=0; u<4; u++)
				{
					move_dmap_maps_list[t][u]=false;
					
					if(move_dmap_maps_list[t][u])
					{
						switch(u)
						{
						case 0:
							DMaps[t].minimap_1_tile+=diff;
							break;
							
						case 1:
							DMaps[t].minimap_2_tile+=diff;
							break;
							
						case 2:
							DMaps[t].largemap_1_tile+=diff;
							break;
							
						case 3:
							DMaps[t].largemap_2_tile+=diff;
							break;
						}
					}
				}
			}
		
			for(int32_t u=0; u<eMAXGUYS; u++)
			{
				if(move_enemy_list[u])
				{
					guydata& enemy=guysbuf[bie[u].i];
					if(newtiles)
						enemy.e_tile += diff;
					else
					{
						enemy.tile += diff;
						if(enemy.s_tile)
							enemy.s_tile += diff;
					}
				}
			}
		}
	}
	
	//now that tiles have moved, fix these buffers -DD
	register_blank_tiles();
	register_used_tiles();
	
	delete[] tile_move_list_text;
	delete[] move_combo_list;
	delete[] move_items_list;
	delete[] move_weapons_list;
	delete[] move_enemy_list;
	
	if(done)
		return false;
		
	return true;
}

bool copy_tiles_united(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect, bool move)
{
	bool alt=(key[KEY_ALT]||key[KEY_ALTGR]);
	bool shift=(key[KEY_LSHIFT] || key[KEY_RSHIFT]);
	
	// if tile>tile2 then swap them
	if(tile>tile2)
	{
		zc_swap(tile, tile2);
	}
	
	// alt=copy from right
	// shift=copy from bottom
	tile_move_data tmd;
	
	tmd.copies=copycnt;
	tmd.dest_first=tile;
	tmd.dest_last=tile2;
	tmd.src_first=copy;
	tmd.src_last=copy+tmd.copies-1;
	tmd.rect = rect;
	tmd.move = move;
	
	if(rect)
	{
		tmd.dest_top=TILEROW(tmd.dest_first);
		tmd.dest_bottom=TILEROW(tmd.dest_last);
		tmd.src_top=TILEROW(tmd.src_first);
		tmd.src_bottom=TILEROW(tmd.src_last);
		
		tmd.src_left= zc_min(TILECOL(tmd.src_first),TILECOL(tmd.src_last));
		tmd.src_right=zc_max(TILECOL(tmd.src_first),TILECOL(tmd.src_last));
		tmd.src_first=(tmd.src_top  * TILES_PER_ROW)+tmd.src_left;
		tmd.src_last= (tmd.src_bottom*TILES_PER_ROW)+tmd.src_right;
		
		tmd.dest_left= zc_min(TILECOL(tmd.dest_first),TILECOL(tmd.dest_last));
		tmd.dest_right=zc_max(TILECOL(tmd.dest_first),TILECOL(tmd.dest_last));
		tmd.dest_first=(tmd.dest_top  * TILES_PER_ROW)+tmd.dest_left;
		tmd.dest_last= (tmd.dest_bottom*TILES_PER_ROW)+tmd.dest_right;
		
		//if no dest range set, then set one
		if((tmd.dest_first==tmd.dest_last)&&(tmd.src_first!=tmd.src_last))
		{
			if(alt)
			{
				tmd.dest_left=tmd.dest_right-(tmd.src_right-tmd.src_left);
			}
			else
			{
				tmd.dest_right=tmd.dest_left+(tmd.src_right-tmd.src_left);
			}
			
			if(shift)
			{
				tmd.dest_top=tmd.dest_bottom-(tmd.src_bottom-tmd.src_top);
			}
			else
			{
				tmd.dest_bottom=tmd.dest_top+(tmd.src_bottom-tmd.src_top);
			}
			
			tmd.dest_first=(tmd.dest_top  * TILES_PER_ROW)+tmd.dest_left;
			tmd.dest_last= (tmd.dest_bottom*TILES_PER_ROW)+tmd.dest_right;
		}
		else
		{
			if(tmd.dest_right-tmd.dest_left<tmd.src_right-tmd.src_left) //destination is shorter than source
			{
				if(alt) //copy from right tile instead of left
				{
					tmd.src_left=tmd.src_right-(tmd.dest_right-tmd.dest_left);
				}
				else //copy from left tile
				{
					tmd.src_right=tmd.src_left+(tmd.dest_right-tmd.dest_left);
				}
			}
			else if(tmd.dest_right-tmd.dest_left>tmd.src_right-tmd.src_left)  //destination is longer than source
			{
				if(alt) //copy from right tile instead of left
				{
					tmd.dest_left=tmd.dest_right-(tmd.src_right-tmd.src_left);
				}
				else //copy from left tile
				{
					tmd.dest_right=tmd.dest_left+(tmd.src_right-tmd.src_left);
				}
			}
			
			if(tmd.dest_bottom-tmd.dest_top<tmd.src_bottom-tmd.src_top) //destination is shorter than source
			{
				if(shift) //copy from bottom tile instead of top
				{
					tmd.src_top=tmd.src_bottom-(tmd.dest_bottom-tmd.dest_top);
				}
				else //copy from top tile
				{
					tmd.src_bottom=tmd.src_top+(tmd.dest_bottom-tmd.dest_top);
				}
			}
			else if(tmd.dest_bottom-tmd.dest_top>tmd.src_bottom-tmd.src_top)  //destination is longer than source
			{
				if(shift) //copy from bottom tile instead of top
				{
					tmd.dest_top=tmd.dest_bottom-(tmd.src_bottom-tmd.src_top);
				}
				else //copy from top tile
				{
					tmd.dest_bottom=tmd.dest_top+(tmd.src_bottom-tmd.src_top);
				}
			}
			
			tmd.src_first=(tmd.src_top  * TILES_PER_ROW)+tmd.src_left;
			tmd.src_last= (tmd.src_bottom*TILES_PER_ROW)+tmd.src_right;
			tmd.dest_first=(tmd.dest_top  * TILES_PER_ROW)+tmd.dest_left;
			tmd.dest_last= (tmd.dest_bottom*TILES_PER_ROW)+tmd.dest_right;
		}
		
		tmd.cols=tmd.src_right-tmd.src_left+1;
		tmd.rows=tmd.src_bottom-tmd.src_top+1;
		
		tmd.dest_width=tmd.dest_right-tmd.dest_left+1;
		tmd.dest_height=tmd.dest_bottom-tmd.dest_top+1;
		tmd.src_width=tmd.src_right-tmd.src_left+1;
		tmd.src_height=tmd.src_bottom-tmd.src_top+1;
		
	}
	else  //!rect
	{
		//if no dest range set, then set one
		if((tmd.dest_first==tmd.dest_last)&&(tmd.src_first!=tmd.src_last))
		{
			if(alt)
			{
				tmd.dest_first=tmd.dest_last-(tmd.src_last-tmd.src_first);
			}
			else
			{
				tmd.dest_last=tmd.dest_first+(tmd.src_last-tmd.src_first);
			}
		}
		else
		{
			if(tmd.dest_last-tmd.dest_first<tmd.src_last-tmd.src_first) //destination is shorter than source
			{
				if(alt) //copy from last tile instead of first
				{
					tmd.src_first=tmd.src_last-(tmd.dest_last-tmd.dest_first);
				}
				else //copy from first tile
				{
					tmd.src_last=tmd.src_first+(tmd.dest_last-tmd.dest_first);
				}
			}
			else if(tmd.dest_last-tmd.dest_first>tmd.src_last-tmd.src_first)  //destination is longer than source
			{
				if(alt) //copy from last tile instead of first
				{
					tmd.dest_first=tmd.dest_last-(tmd.src_last-tmd.src_first);
				}
				else //copy from first tile
				{
					tmd.dest_last=tmd.dest_first+(tmd.src_last-tmd.src_first);
				}
			}
		}
		
		tmd.copies=tmd.dest_last-tmd.dest_first+1;
	}
	
	bool ret = do_movetile_united(tmd);
	if(ret)
	{
		if(last_tile_move)
			delete last_tile_move;
		last_tile_move = new tile_move_data(tmd);
	}
	return ret;
}

//

bool copy_tiles_united_floodfill(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect, bool move)
{
	bool ignore_frames=false;
	
	// if tile>tile2 then swap them
	if(tile>tile2)
	{
		zc_swap(tile, tile2);
	}
		
	tile_move_data tmd;
	tmd.copies=copycnt;
	tmd.dest_first=tile;
	tmd.dest_last=tile2;
	tmd.src_first=copy;
	tmd.src_last=copy+tmd.copies-1;
	
	
	
	if(rect)
	{
		tmd.dest_top=TILEROW(tmd.dest_first);
		tmd.dest_bottom=TILEROW(tmd.dest_last);
		//tmd.src_top=TILEROW(tmd.src_first);
		//tmd.src_bottom=TILEROW(tmd.src_last);
		
		//tmd.src_left= zc_min(TILECOL(tmd.src_first),TILECOL(tmd.src_last));
		//tmd.src_right=zc_max(TILECOL(tmd.src_first),TILECOL(tmd.src_last));
		//tmd.src_first=(tmd.src_top  * TILES_PER_ROW)+tmd.src_left;
		//tmd.src_last= (tmd.src_bottom*TILES_PER_ROW)+tmd.src_right;
		
		tmd.dest_left= zc_min(TILECOL(tmd.dest_first),TILECOL(tmd.dest_last));
		tmd.dest_right=zc_max(TILECOL(tmd.dest_first),TILECOL(tmd.dest_last));
		tmd.dest_first=(tmd.dest_top  * TILES_PER_ROW)+tmd.dest_left;
		tmd.dest_last= (tmd.dest_bottom*TILES_PER_ROW)+tmd.dest_right;
		
		
		
		
		tmd.dest_width=tmd.dest_right-tmd.dest_left;
		tmd.dest_height=tmd.dest_bottom-tmd.dest_top;
		
	tmd.cols=tmd.dest_width+1;
		tmd.rows=tmd.dest_height+1;
	
	al_trace("tmd.rows: %d\n", tmd.rows);
	al_trace("tmd.cols: %d\n", tmd.cols);

		
	}
	else  //!rect
	{
		tmd.copies=tmd.dest_last-tmd.dest_first+1;
	}
	
	
	
	char buf[80], buf2[80], buf3[80], buf4[80];
	sprintf(buf, " ");
	sprintf(buf2, " ");
	sprintf(buf3, " ");
	sprintf(buf4, " ");
	
	// warn if range extends beyond last tile
	sprintf(buf4, "Some tiles will not be %s", move?"moved.":"copied.");
	
	if(tmd.dest_last>=NEWMAXTILES)
	{
		sprintf(buf4, "%s operation cancelled.", move?"Move":"Copy");
		jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", NULL, 'o', 0, lfont);
		return false;
		//fix this below to allow the operation to complete with a modified start or end instead of just cancelling
		//if (jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", "&Cancel", 'o', 'c', lfont)==2)
		// {
		//  return false;
		// }
	}
	
	char *tile_move_list_text = new char[65535];
	char temptext[80];
	
	sprintf(buf, "Destination Warning");
	tile_move_list_dlg[0].dp=buf;
	tile_move_list_dlg[0].dp2=lfont;
	bool found;
	bool flood;
	
	int32_t i;
	bool *move_combo_list = new bool[MAXCOMBOS];
	bool *move_items_list = new bool[iMax];
	bool *move_weapons_list = new bool[wMAX];
	bool move_hero_sprites_list[num_hspr];
	bool move_mapstyles_list[6];
	//bool move_subscreenobjects_list[MAXCUSTOMSUBSCREENS*MAXSUBSCREENITEMS];
	bool move_game_icons_list[4];
	bool move_dmap_maps_list[MAXDMAPS][4];
	//    bool move_enemies_list[eMAXGUYS];  //to be implemented once custom enemies are in
	
	// warn if paste overwrites other defined tiles or
	// if delete erases other defined tiles
	int32_t selection_first=0, selection_last=0, selection_left=0, selection_top=0, selection_width=0, selection_height=0;
	bool done = false;
	
	for(int32_t q=0; q<2 && !done; ++q)
	{
	
		switch(q)
		{
		case 0:
			selection_first=tmd.dest_first;
			selection_last=tmd.dest_last;
			selection_left=tmd.dest_left;
			selection_top=tmd.dest_top;
			selection_width=tmd.dest_width;
			selection_height=tmd.dest_height;
			break;
			
		case 1:
			selection_first=tmd.src_first;
			selection_last=tmd.src_last;
			selection_left=tmd.src_left;
			selection_top=tmd.src_top;
			selection_width=tmd.src_width;
			selection_height=tmd.src_height;
			break;
		}
		
		if(move||q==0)
		{
			//check combos
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				
				for(int32_t u=0; u<MAXCOMBOS; u++)
				{
					move_combo_list[u]=false;
					
					if(rect)
					{
						i = move_intersection_sr(combobuf[u], selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i = move_intersection_ss(combobuf[u], selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(combobuf[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%d\n", u);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_combo_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following combos");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check items
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_bii_list(false);
				
				for(int32_t u=0; u<iMax; u++)
				{
					move_items_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_sr(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+zc_max(itemsbuf[bii[u].i].frames,1)-1, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+zc_max(itemsbuf[bii[u].i].frames,1)-1, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(itemsbuf[bii[u].i].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", bii[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_items_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following items");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check weapons/misc
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_biw_list();
				bool BSZ2=get_bit(quest_rules,qr_BSZELDA)!=0;
				
				for(int32_t u=0; u<wMAX; u++)
				{
					ignore_frames=false;
					move_weapons_list[u]=false;
					int32_t m=0;
					
					switch(biw[u].i)
					{
					case wSWORD:
					case wWSWORD:
					case wMSWORD:
					case wXSWORD:
						m=3+((wpnsbuf[biw[u].i].type==3)?1:0);
						break;
						
					case wSWORDSLASH:
					case wWSWORDSLASH:
					case wMSWORDSLASH:
					case wXSWORDSLASH:
						m=4;
						break;
						
					case iwMMeter:
						m=9;
						break;
						
					case wBRANG:
					case wMBRANG:
					case wFBRANG:
						m=BSZ2?1:3;
						break;
						
					case wBOOM:
					case wSBOOM:
					case ewBOOM:
					case ewSBOOM:
						ignore_frames=true;
						m=2;
						break;
						
					case wWAND:
						m=1;
						break;
						
					case wMAGIC:
						m=1;
						break;
						
					case wARROW:
					case wSARROW:
					case wGARROW:
					case ewARROW:
						m=1;
						break;
						
					case wHAMMER:
						m=8;
						break;
						
					case wHSHEAD:
						m=1;
						break;
						
					case wHSCHAIN_H:
						m=1;
						break;
						
					case wHSCHAIN_V:
						m=1;
						break;
						
					case wHSHANDLE:
						m=1;
						break;
						
					case iwDeath:
						m=BSZ2?4:2;
						break;
						
					case iwSpawn:
						m=3;
						break;
						
					default:
						m=0;
						break;
					}
					
					if(rect)
					{
						i=move_intersection_sr(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+zc_max((ignore_frames?0:wpnsbuf[biw[u].i].frames),1)-1+m, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+zc_max((ignore_frames?0:wpnsbuf[biw[u].i].frames),1)-1+m, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(wpnsbuf[biw[u].i].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", biw[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_weapons_list[u]=true;
						}
					}
					
					if((u==3)||(u==9))
					{
						if(rect)
						{
							i=move_intersection_sr(54, 55, selection_left, selection_top, selection_width, selection_height);
						}
						else
						{
							i=move_intersection_ss(54, 55, selection_first, selection_last);
						}
						
						if(i!=ti_none)
						{
							sprintf(temptext, "%s Impact (not shown in sprite list)\n", (u==3)?"Arrow":"Boomerang");
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following weapons");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check Player sprites
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				setup_hero_sprite_items();
				
				for(int32_t u=0; u<num_hspr; u++)
				{
					move_hero_sprites_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_rr(TILECOL(hero_sprite_items[u].tile), TILEROW(hero_sprite_items[u].tile), hero_sprite_items[u].width, hero_sprite_items[u].height, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_rs(TILECOL(hero_sprite_items[u].tile), TILEROW(hero_sprite_items[u].tile), hero_sprite_items[u].width, hero_sprite_items[u].height, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(hero_sprite_items[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", hero_sprite_items[u].name);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_hero_sprites_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following Player sprites");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "sprites will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done =true;
						}
					}
				}
			}
			
			//Check subscreen objects
			//Tried to have a go at this but I think it's a bit too complicated for me at the moment.
			//Might come back to it another time and see what I can do ~Joe123
			/*if(!done){
				 for(int32_t u=0;u<MAXCUSTOMSUBSCREENS;u++){
					 if(!custom_subscreen[u].ss_type) continue;
					 for(int32_t v=0;v<MAXSUBSCREENITEMS;v++){
						  if(custom_subscreen[u].objects[v].type != ssoTILEBLOCK) continue;
						  subscreen_items[0].tile = custom_subscreen[u].objects[v].d1;
						  subscreen_items[0].width = custom_subscreen[u].objects[v].w;
						  subscreen_items[0].height = custom_subscreen[u].objects[v].h;
					 }
				 }
			}*/
			
			//check map styles
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				bool BSZ2=(zinit.subscreen>2);
				map_styles_items[0].tile=misc.colors.blueframe_tile;
				map_styles_items[1].tile=misc.colors.HCpieces_tile;
				map_styles_items[1].width=zinit.hcp_per_hc;
				map_styles_items[2].tile=misc.colors.triforce_tile;
				map_styles_items[2].width=BSZ2?2:1;
				map_styles_items[2].height=BSZ2?3:1;
				map_styles_items[3].tile=misc.colors.triframe_tile;
				map_styles_items[3].width=BSZ2?7:6;
				map_styles_items[3].height=BSZ2?7:3;
				map_styles_items[4].tile=misc.colors.overworld_map_tile;
				map_styles_items[5].tile=misc.colors.dungeon_map_tile;
				
				for(int32_t u=0; u<6; u++)
				{
					move_mapstyles_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_rr(TILECOL(map_styles_items[u].tile), TILEROW(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_rs(TILECOL(map_styles_items[u].tile), TILEROW(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(map_styles_items[u].tile!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", map_styles_items[u].name);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_mapstyles_list[u]=true;
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following map style");
					
					if(move)
					{
						sprintf(buf3, "items will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "items will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check game icons
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				const char *icon_title[4]=
				{
					"No Ring / Green Ring", "Blue Ring", "Red Ring", "Golden Ring"
				};
				
				for(int32_t u=0; u<4; u++)
				{
					move_game_icons_list[u]=false;
					
					if(rect)
					{
						i=move_intersection_sr(misc.icons[u], misc.icons[u], selection_left, selection_top, selection_width, selection_height);
					}
					else
					{
						i=move_intersection_ss(misc.icons[u], misc.icons[u], selection_first, selection_last);
					}
					
					if((i!=ti_none)&&(misc.icons[u]!=0))
					{
						if(i==ti_broken || q==0)
						{
							sprintf(temptext, "%s\n", icon_title[u]);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						else if(i==ti_encompass)
						{
							move_game_icons_list[u]=true;
						}
					}
				}
				
				if(rect)
				{
					i=move_intersection_sr(41, 41, selection_left, selection_top, selection_width, selection_height);
				}
				else
				{
					i=move_intersection_ss(41, 41, selection_first, selection_last);
				}
				
				if((i!=ti_none)) // &&(41!=0))  //this is for when the quest sword can change
				{
					sprintf(temptext, "Quest Sword");
					
					if(strlen(tile_move_list_text)<65000)
					{
						strcat(tile_move_list_text, temptext);
					}
					else
					{
						if(!flood)
						{
							strcat(tile_move_list_text, "...\n...\n...\nmany others");
							flood=true;
						}
					}
					
					found=true;
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following quest icons");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be overwritten by this process.  Proceed?");
						sprintf(buf4, " ");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check dmap maps
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				bool BSZ2=(zinit.subscreen>2);
				
				for(int32_t t=0; t<MAXDMAPS; t++)
				{
					dmap_map_items[0].tile=DMaps[t].minimap_1_tile;
					dmap_map_items[1].tile=DMaps[t].minimap_2_tile;
					dmap_map_items[2].tile=DMaps[t].largemap_1_tile;
					dmap_map_items[2].width=BSZ2?7:9;
					dmap_map_items[3].tile=DMaps[t].largemap_2_tile;
					dmap_map_items[3].width=BSZ2?7:9;
					
					for(int32_t u=0; u<4; u++)
					{
						move_dmap_maps_list[t][u]=false;
						
						if(rect)
						{
							i=move_intersection_rr(TILECOL(dmap_map_items[u].tile), TILEROW(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, selection_left, selection_top, selection_width, selection_height);
						}
						else
						{
							i=move_intersection_rs(TILECOL(dmap_map_items[u].tile), TILEROW(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, selection_first, selection_last);
						}
						
						if((i!=ti_none)&&(dmap_map_items[u].tile!=0))
						{
							if(i==ti_broken || q==0)
							{
								sprintf(temptext, "DMap %d %s\n", t, dmap_map_items[u].name);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
							else if(i==ti_encompass)
							{
								move_dmap_maps_list[t][u]=true;
							}
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following dmap-specific");
					
					if(move)
					{
						sprintf(buf3, "subscreen maps will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "subscreen maps will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
			
			//check enemies
			if(!done)
			{
				//this is here to allow this section to fold
				tile_move_list_text[0]=0;
				found=false;
				flood=false;
				build_bie_list(false);
				bool newtiles=get_bit(quest_rules,qr_NEWENEMYTILES)!=0;
				int32_t u;
				
				for(u=0; u<eMAXGUYS; u++)
				{
					const guydata& enemy=guysbuf[bie[u].i];
					bool darknut=false;
					int32_t gleeok=0;
					
					if(enemy.family==eeWALK && ((enemy.flags&(inv_back|inv_front|inv_left|inv_right))!=0))
						darknut=true;
					else if(enemy.family==eeGLEEOK)
					{
						// Not certain this is the right thing to check...
						if(enemy.misc3==0)
							gleeok=1;
						else
							gleeok=2;
					}
					
					// Dummied out enemies
					if(bie[u].i>=eOCTO1S && bie[u].i<e177)
					{
						if(old_guy_string[bie[u].i][strlen(old_guy_string[bie[u].i])-1]==' ')
						{
							continue;
						}
					}
					
					if(newtiles)
					{
						if(guysbuf[bie[u].i].e_tile==0)
						{
							continue;
						}
						
						if(guysbuf[bie[u].i].e_height==0)
						{
							if(rect)
							{
								i=move_intersection_sr(guysbuf[bie[u].i].e_tile, guysbuf[bie[u].i].e_tile+zc_max(guysbuf[bie[u].i].e_width-1, 0), selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_ss(guysbuf[bie[u].i].e_tile, guysbuf[bie[u].i].e_tile+zc_max(guysbuf[bie[u].i].e_width-1, 0), selection_first, selection_last);
							}
						}
						else
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_first, selection_last);
							}
						}
						
						if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
						{
							sprintf(temptext, "%s\n", bie[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						
						if(darknut)
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile+120), TILEROW(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile+120), TILEROW(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, selection_first, selection_last);
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s (broken shield)\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
						else if(enemy.family==eeGANON && i==ti_none)
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile)+2, 20, 4, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile)+2, 20, 4, selection_first, selection_last);
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
						else if(gleeok && i==ti_none)
						{
							for(int32_t j=0; j<4 && i==ti_none; ++j)
							{
								if(rect)
								{
									i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), TILEROW(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), TILEROW(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, selection_first, selection_last);
								}
							}
							
							if(i==ti_none)
							{
								int32_t c=TILECOL(guysbuf[bie[u].i].e_tile)+(gleeok>1?-12:0);
								int32_t r=TILEROW(guysbuf[bie[u].i].e_tile)+(gleeok>1?17:8);
								
								if(rect)
								{
									i=move_intersection_rr(c, r, 20, 3, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(c, r, 20, 3, selection_first, selection_last);
								}
								
								if(i==ti_none)
								{
									if(rect)
									{
										i=move_intersection_rr(c, r+3, 16, 6, selection_left, selection_top, selection_width, selection_height);
									}
									else
									{
										i=move_intersection_rs(c, r+3, 16, 6, selection_first, selection_last);
									}
								}
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s\n", bie[u].s);
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
					}
					else
					{
						if(guysbuf[bie[u].i].tile==0)
						{
							continue;
						}
						else if(guysbuf[bie[u].i].height==0)
						{
							if(rect)
							{
								i=move_intersection_sr(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+zc_max(guysbuf[bie[u].i].width-1, 0), selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_ss(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+zc_max(guysbuf[bie[u].i].width-1, 0), selection_first, selection_last);
							}
						}
						else
						{
							if(rect)
							{
								i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].tile), TILEROW(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, selection_left, selection_top, selection_width, selection_height);
							}
							else
							{
								i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].tile), TILEROW(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, selection_first, selection_last);
							}
						}
						
						if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
						{
							sprintf(temptext, "%s\n", bie[u].s);
							
							if(strlen(tile_move_list_text)<65000)
							{
								strcat(tile_move_list_text, temptext);
							}
							else
							{
								if(!flood)
								{
									strcat(tile_move_list_text, "...\n...\n...\nmany others");
									flood=true;
								}
							}
							
							found=true;
						}
						
						if(guysbuf[bie[u].i].s_tile!=0)
						{
							if(guysbuf[bie[u].i].s_height==0)
							{
								if(rect)
								{
									i=move_intersection_sr(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+zc_max(guysbuf[bie[u].i].s_width-1, 0), selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_ss(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+zc_max(guysbuf[bie[u].i].s_width-1, 0), selection_first, selection_last);
								}
							}
							else
							{
								if(rect)
								{
									i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].s_tile), TILEROW(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, selection_left, selection_top, selection_width, selection_height);
								}
								else
								{
									i=move_intersection_rs(TILECOL(guysbuf[bie[u].i].s_tile), TILEROW(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, selection_first, selection_last);
								}
							}
							
							if(((q==1) && i==ti_broken) || (q==0 && i!=ti_none))
							{
								sprintf(temptext, "%s (%s)\n", bie[u].s, darknut?"broken shield":"secondary tiles");
								
								if(strlen(tile_move_list_text)<65000)
								{
									strcat(tile_move_list_text, temptext);
								}
								else
								{
									if(!flood)
									{
										strcat(tile_move_list_text, "...\n...\n...\nmany others");
										flood=true;
									}
								}
								
								found=true;
							}
						}
					}
				}
				
				if(found)
				{
					sprintf(buf2, "The tiles used by the following enemies");
					
					if(move)
					{
						sprintf(buf3, "will be partially cleared by the move.");
						sprintf(buf4, "Proceed?");
					}
					else
					{
						sprintf(buf3, "will be partially or completely");
						sprintf(buf4, "overwritten by this process.  Proceed?");
					}
					
					tile_move_list_dlg[1].dp=buf2;
					tile_move_list_dlg[2].dp=buf3;
					tile_move_list_dlg[3].dp=buf4;
					tile_move_list_dlg[4].dp=tile_move_list_text;
					tile_move_list_dlg[4].d2=0;
					
					if(TileProtection)
					{
						large_dialog(tile_move_list_dlg);
							
						int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
						position_mouse_z(0);
						
						if(ret!=5)
						{
							done = true;
						}
					}
				}
			}
		}
	}
	
	//
	// copy tiles and delete if needed (move)
	
	if(!done)
	{
		go_tiles();
		
		int32_t diff=tmd.dest_first-tmd.src_first;
		
		if(rect)
		{
		al_trace("floodfill, rect\n");
		al_trace("tmd.rows: %d\n", tmd.rows);
		al_trace("tmd.cols: %d\n", tmd.cols);
			for(int32_t r=0; r<tmd.rows; ++r)
			{
				for(int32_t c=0; c<tmd.cols; ++c)
				{
					int32_t dt=(tmd.dest_first+((r*TILES_PER_ROW)+c));
					//int32_t st=(tmd.src_first+((r*TILES_PER_ROW)+c));
					
					if(dt>=NEWMAXTILES)
						continue;
						
					reset_tile(newtilebuf, dt, newundotilebuf[copy].format);
					
					for(int32_t j=0; j<tilesize(newundotilebuf[copy].format); j++)
					{
						newtilebuf[dt].data[j]=newundotilebuf[copy].data[j];
					}
				}
			}
		}
		else
		{
			for(int32_t c=0; c<tmd.copies; ++c)
			{
				int32_t dt=(tmd.dest_first+c);
				int32_t st=(tmd.src_first+c);
				
				if(dt>=NEWMAXTILES)
					continue;
					
				reset_tile(newtilebuf, dt, newundotilebuf[copy].format);
				
				for(int32_t j=0; j<tilesize(newundotilebuf[copy].format); j++)
				{
					newtilebuf[dt].data[j]=newundotilebuf[copy].data[j];
				}
				
			}
		}
	}
	
	//now that tiles have moved, fix these buffers -DD
	register_blank_tiles();
	register_used_tiles();
	
	delete[] tile_move_list_text;
	delete[] move_combo_list;
	delete[] move_items_list;
	delete[] move_weapons_list;
	
	if(done)
		return false;
		
	return true;
}
//

bool copy_tiles_floodfill(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move)
{
	al_trace("Floodfill Psste\n");
	bool ctrl=(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]);
	bool copied=false;
	copied=copy_tiles_united_floodfill(tile,tile2,copy,copycnt,rect_sel,move);
	
	if(copied)
	{
		if(!ctrl)
		{
			copy=-1;
			tile2=tile;
		}
		
		saved=false;
	}
	
	return copied;
}

bool copy_tiles(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move)
{
	bool ctrl=(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]);
	bool copied=false;
	copied=copy_tiles_united(tile,tile2,copy,copycnt,rect_sel,move);
	
	if(copied)
	{
		if(!ctrl)
		{
			copy=-1;
			tile2=tile;
		}
		
		saved=false;
	}
	
	return copied;
}

bool scale_tiles(int32_t &tile, int32_t &tile2, int32_t &cs)
{
	// if tile>tile2 then swap them
	if(tile>tile2)
	{
		zc_swap(tile, tile2);
	}
	int32_t src_top = TILEROW(tile);
	int32_t src_bottom = TILEROW(tile2);
	int32_t src_left = zc_min(TILECOL(tile),TILECOL(tile2));
	int32_t src_right = zc_max(TILECOL(tile),TILECOL(tile2));
	int32_t src_first = (src_top  * TILES_PER_ROW)+src_left;
	int32_t src_last = (src_bottom*TILES_PER_ROW)+src_right;
	
	int32_t src_width = src_right-src_left+1,
		src_height = src_bottom-src_top+1;
	int32_t dest_width = src_width, dest_height = src_height;
	ScaleTileDialog(&dest_width, &dest_height).show();
	if(dest_width == src_width && dest_height == src_height) return false; //no scaling
	dest_width = vbound(dest_width, 1, 20);
	dest_height = vbound(dest_height, 1, 20);
	
	int32_t dest_top = src_top;
	int32_t dest_bottom = src_top+dest_height-1;
	int32_t dest_left = src_left;
	int32_t dest_right = src_left+dest_width-1;
	int32_t dest_first = src_first;
	int32_t dest_last = (dest_bottom*TILES_PER_ROW)+dest_right;
	
	
	if(dest_last>=NEWMAXTILES)
	{
		InfoDialog("Destination Error", "The destination extends beyond the last available tile row. Scale operation cancelled.").show();
		return false;
	}
	
	//{ Overwrite warnings
	char buf[80], buf2[80], buf3[80], buf4[80];
	sprintf(buf, " ");
	sprintf(buf2, " ");
	sprintf(buf3, " ");
	sprintf(buf4, " ");
	char *tile_move_list_text = new char[65535];
	char temptext[80];
	
	sprintf(buf, "Destination Warning");
	tile_move_list_dlg[0].dp=buf;
	tile_move_list_dlg[0].dp2=lfont;
	bool found;
	bool flood;
	
	int32_t i;
	bool done = false;
	bool ignore_frames=false;
	
	//check combos
	if(!done)
	{
		//this is here to allow this section to fold
		tile_move_list_text[0]=0;
		found=false;
		flood=false;
		
		for(int32_t u=0; u<MAXCOMBOS; u++)
		{
			i=move_intersection_sr(combobuf[u], dest_left, dest_top, dest_width, dest_height);
			
			if((i!=ti_none)&&(combobuf[u].o_tile!=0))
			{
				sprintf(temptext, "%d\n", u);
				
				if(strlen(tile_move_list_text)<65000)
				{
					strcat(tile_move_list_text, temptext);
				}
				else
				{
					if(!flood)
					{
						strcat(tile_move_list_text, "...\n...\n...\nmany others");
						flood=true;
					}
				}
				
				found=true;
			}
		}
		
		if(found)
		{
			sprintf(buf2, "The tiles used by the following combos");
			sprintf(buf3, "will be partially or completely");
			sprintf(buf4, "overwritten by this process.  Proceed?");
			
			tile_move_list_dlg[1].dp=buf2;
			tile_move_list_dlg[2].dp=buf3;
			tile_move_list_dlg[3].dp=buf4;
			tile_move_list_dlg[4].dp=tile_move_list_text;
			tile_move_list_dlg[4].d2=0;
			
			if(TileProtection)
			{
				large_dialog(tile_move_list_dlg);
					
				int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
				position_mouse_z(0);
				
				if(ret!=5)
				{
					done = true;
				}
			}
		}
	}
	
	//check items
	if(!done)
	{
		//this is here to allow this section to fold
		tile_move_list_text[0]=0;
		found=false;
		flood=false;
		build_bii_list(false);
		
		for(int32_t u=0; u<iMax; u++)
		{
			i=move_intersection_sr(itemsbuf[bii[u].i].tile, itemsbuf[bii[u].i].tile+zc_max(itemsbuf[bii[u].i].frames,1)-1, dest_left, dest_top, dest_width, dest_height);
			
			if((i!=ti_none)&&(itemsbuf[bii[u].i].tile!=0))
			{
				sprintf(temptext, "%s\n", bii[u].s);
				
				if(strlen(tile_move_list_text)<65000)
				{
					strcat(tile_move_list_text, temptext);
				}
				else
				{
					if(!flood)
					{
						strcat(tile_move_list_text, "...\n...\n...\nmany others");
						flood=true;
					}
				}
				
				found=true;
			}
		}
		
		if(found)
		{
			sprintf(buf2, "The tiles used by the following items");
			sprintf(buf3, "will be partially or completely");
			sprintf(buf4, "overwritten by this process.  Proceed?");
			
			tile_move_list_dlg[1].dp=buf2;
			tile_move_list_dlg[2].dp=buf3;
			tile_move_list_dlg[3].dp=buf4;
			tile_move_list_dlg[4].dp=tile_move_list_text;
			tile_move_list_dlg[4].d2=0;
			
			if(TileProtection)
			{
				large_dialog(tile_move_list_dlg);
					
				int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
				position_mouse_z(0);
				
				if(ret!=5)
				{
					done = true;
				}
			}
		}
	}
	
	//check weapons/misc
	if(!done)
	{
		//this is here to allow this section to fold
		tile_move_list_text[0]=0;
		found=false;
		flood=false;
		build_biw_list();
		bool BSZ2=get_bit(quest_rules,qr_BSZELDA)!=0;
		
		for(int32_t u=0; u<wMAX; u++)
		{
			ignore_frames=false;
			int32_t m=0;
			
			switch(biw[u].i)
			{
			case wSWORD:
			case wWSWORD:
			case wMSWORD:
			case wXSWORD:
				m=3+((wpnsbuf[biw[u].i].type==3)?1:0);
				break;
				
			case wSWORDSLASH:
			case wWSWORDSLASH:
			case wMSWORDSLASH:
			case wXSWORDSLASH:
				m=4;
				break;
				
			case iwMMeter:
				m=9;
				break;
				
			case wBRANG:
			case wMBRANG:
			case wFBRANG:
				m=BSZ2?1:3;
				break;
				
			case wBOOM:
			case wSBOOM:
			case ewBOOM:
			case ewSBOOM:
				ignore_frames=true;
				m=2;
				break;
				
			case wWAND:
				m=1;
				break;
				
			case wMAGIC:
				m=1;
				break;
				
			case wARROW:
			case wSARROW:
			case wGARROW:
			case ewARROW:
				m=1;
				break;
				
			case wHAMMER:
				m=8;
				break;
				
			case wHSHEAD:
				m=1;
				break;
				
			case wHSCHAIN_H:
				m=1;
				break;
				
			case wHSCHAIN_V:
				m=1;
				break;
				
			case wHSHANDLE:
				m=1;
				break;
				
			case iwDeath:
				m=BSZ2?4:2;
				break;
				
			case iwSpawn:
				m=3;
				break;
				
			default:
				m=0;
				break;
			}
			
			i=move_intersection_sr(wpnsbuf[biw[u].i].tile, wpnsbuf[biw[u].i].tile+zc_max((ignore_frames?0:wpnsbuf[biw[u].i].frames),1)-1+m, dest_left, dest_top, dest_width, dest_height);
			
			if((i!=ti_none)&&(wpnsbuf[biw[u].i].tile!=0))
			{
				sprintf(temptext, "%s\n", biw[u].s);
					
				if(strlen(tile_move_list_text)<65000)
				{
					strcat(tile_move_list_text, temptext);
				}
				else
				{
					if(!flood)
					{
						strcat(tile_move_list_text, "...\n...\n...\nmany others");
						flood=true;
					}
				}
					
				found=true;
			}
			
			if((u==3)||(u==9))
			{
				i=move_intersection_sr(54, 55, dest_left, dest_top, dest_width, dest_height);
				
				if(i!=ti_none)
				{
					sprintf(temptext, "%s Impact (not shown in sprite list)\n", (u==3)?"Arrow":"Boomerang");
					
					if(strlen(tile_move_list_text)<65000)
					{
						strcat(tile_move_list_text, temptext);
					}
					else
					{
						if(!flood)
						{
							strcat(tile_move_list_text, "...\n...\n...\nmany others");
							flood=true;
						}
					}
					
					found=true;
				}
			}
		}
		
		if(found)
		{
			sprintf(buf2, "The tiles used by the following weapons");
			sprintf(buf3, "will be partially or completely");
			sprintf(buf4, "overwritten by this process.  Proceed?");
			
			tile_move_list_dlg[1].dp=buf2;
			tile_move_list_dlg[2].dp=buf3;
			tile_move_list_dlg[3].dp=buf4;
			tile_move_list_dlg[4].dp=tile_move_list_text;
			tile_move_list_dlg[4].d2=0;
			
			if(TileProtection)
			{
				large_dialog(tile_move_list_dlg);
					
				int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
				position_mouse_z(0);
				
				if(ret!=5)
				{
					done = true;
				}
			}
		}
	}
	
	//check Player sprites
	if(!done)
	{
		//this is here to allow this section to fold
		tile_move_list_text[0]=0;
		found=false;
		flood=false;
		setup_hero_sprite_items();
		
		for(int32_t u=0; u<num_hspr; u++)
		{
			i=move_intersection_rr(TILECOL(hero_sprite_items[u].tile), TILEROW(hero_sprite_items[u].tile), hero_sprite_items[u].width, hero_sprite_items[u].height, dest_left, dest_top, dest_width, dest_height);
			
			if((i!=ti_none)&&(hero_sprite_items[u].tile!=0))
			{
				sprintf(temptext, "%s\n", hero_sprite_items[u].name);
				
				if(strlen(tile_move_list_text)<65000)
				{
					strcat(tile_move_list_text, temptext);
				}
				else
				{
					if(!flood)
					{
						strcat(tile_move_list_text, "...\n...\n...\nmany others");
						flood=true;
					}
				}
				
				found=true;
			}
		}
		
		if(found)
		{
			sprintf(buf2, "The tiles used by the following Player sprites");
			sprintf(buf3, "sprites will be partially or completely");
			sprintf(buf4, "overwritten by this process.  Proceed?");
			
			tile_move_list_dlg[1].dp=buf2;
			tile_move_list_dlg[2].dp=buf3;
			tile_move_list_dlg[3].dp=buf4;
			tile_move_list_dlg[4].dp=tile_move_list_text;
			tile_move_list_dlg[4].d2=0;
			
			if(TileProtection)
			{
				large_dialog(tile_move_list_dlg);
					
				int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
				position_mouse_z(0);
				
				if(ret!=5)
				{
					done =true;
				}
			}
		}
	}
		
	//check map styles
	if(!done)
	{
		//this is here to allow this section to fold
		tile_move_list_text[0]=0;
		found=false;
		flood=false;
		bool BSZ2=(zinit.subscreen>2);
		map_styles_items[0].tile=misc.colors.blueframe_tile;
		map_styles_items[1].tile=misc.colors.HCpieces_tile;
		map_styles_items[1].width=zinit.hcp_per_hc;
		map_styles_items[2].tile=misc.colors.triforce_tile;
		map_styles_items[2].width=BSZ2?2:1;
		map_styles_items[2].height=BSZ2?3:1;
		map_styles_items[3].tile=misc.colors.triframe_tile;
		map_styles_items[3].width=BSZ2?7:6;
		map_styles_items[3].height=BSZ2?7:3;
		map_styles_items[4].tile=misc.colors.overworld_map_tile;
		map_styles_items[5].tile=misc.colors.dungeon_map_tile;
		
		for(int32_t u=0; u<6; u++)
		{
			i=move_intersection_rr(TILECOL(map_styles_items[u].tile), TILEROW(map_styles_items[u].tile), map_styles_items[u].width, map_styles_items[u].height, dest_left, dest_top, dest_width, dest_height);
			
			if((i!=ti_none)&&(map_styles_items[u].tile!=0))
			{
				sprintf(temptext, "%s\n", map_styles_items[u].name);
				
				if(strlen(tile_move_list_text)<65000)
				{
					strcat(tile_move_list_text, temptext);
				}
				else
				{
					if(!flood)
					{
						strcat(tile_move_list_text, "...\n...\n...\nmany others");
						flood=true;
					}
				}
				
				found=true;
			}
		}
		
		if(found)
		{
			sprintf(buf2, "The tiles used by the following map style");
			sprintf(buf3, "items will be partially or completely");
			sprintf(buf4, "overwritten by this process.  Proceed?");
			
			tile_move_list_dlg[1].dp=buf2;
			tile_move_list_dlg[2].dp=buf3;
			tile_move_list_dlg[3].dp=buf4;
			tile_move_list_dlg[4].dp=tile_move_list_text;
			tile_move_list_dlg[4].d2=0;
			
			if(TileProtection)
			{
				large_dialog(tile_move_list_dlg);
					
				int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
				position_mouse_z(0);
				
				if(ret!=5)
				{
					done = true;
				}
			}
		}
	}
	
	//check game icons
	if(!done)
	{
		//this is here to allow this section to fold
		tile_move_list_text[0]=0;
		found=false;
		flood=false;
		const char *icon_title[4]=
		{
			"No Ring / Green Ring", "Blue Ring", "Red Ring", "Golden Ring"
		};
		
		for(int32_t u=0; u<4; u++)
		{
			i=move_intersection_sr(misc.icons[u], misc.icons[u], dest_left, dest_top, dest_width, dest_height);
			
			if((i!=ti_none)&&(misc.icons[u]!=0))
			{
				sprintf(temptext, "%s\n", icon_title[u]);
				
				if(strlen(tile_move_list_text)<65000)
				{
					strcat(tile_move_list_text, temptext);
				}
				else
				{
					if(!flood)
					{
						strcat(tile_move_list_text, "...\n...\n...\nmany others");
						flood=true;
					}
				}
				
				found=true;
			}
		}
		
		i=move_intersection_sr(41, 41, dest_left, dest_top, dest_width, dest_height);
		
		if((i!=ti_none)) // &&(41!=0))  //this is for when the quest sword can change
		{
			sprintf(temptext, "Quest Sword");
			
			if(strlen(tile_move_list_text)<65000)
			{
				strcat(tile_move_list_text, temptext);
			}
			else
			{
				if(!flood)
				{
					strcat(tile_move_list_text, "...\n...\n...\nmany others");
					flood=true;
				}
			}
			
			found=true;
		}
		
		if(found)
		{
			sprintf(buf2, "The tiles used by the following quest icons");
			sprintf(buf3, "will be overwritten by this process.  Proceed?");
			sprintf(buf4, " ");
			
			tile_move_list_dlg[1].dp=buf2;
			tile_move_list_dlg[2].dp=buf3;
			tile_move_list_dlg[3].dp=buf4;
			tile_move_list_dlg[4].dp=tile_move_list_text;
			tile_move_list_dlg[4].d2=0;
			
			if(TileProtection)
			{
				large_dialog(tile_move_list_dlg);
					
				int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
				position_mouse_z(0);
				
				if(ret!=5)
				{
					done = true;
				}
			}
		}
	}
	
	//check dmap maps
	if(!done)
	{
		//this is here to allow this section to fold
		tile_move_list_text[0]=0;
		found=false;
		flood=false;
		bool BSZ2=(zinit.subscreen>2);
		
		for(int32_t t=0; t<MAXDMAPS; t++)
		{
			dmap_map_items[0].tile=DMaps[t].minimap_1_tile;
			dmap_map_items[1].tile=DMaps[t].minimap_2_tile;
			dmap_map_items[2].tile=DMaps[t].largemap_1_tile;
			dmap_map_items[2].width=BSZ2?7:9;
			dmap_map_items[3].tile=DMaps[t].largemap_2_tile;
			dmap_map_items[3].width=BSZ2?7:9;
			
			for(int32_t u=0; u<4; u++)
			{
				i=move_intersection_rr(TILECOL(dmap_map_items[u].tile), TILEROW(dmap_map_items[u].tile), dmap_map_items[u].width, dmap_map_items[u].height, dest_left, dest_top, dest_width, dest_height);
				
				if((i!=ti_none)&&(dmap_map_items[u].tile!=0))
				{
					sprintf(temptext, "DMap %d %s\n", t, dmap_map_items[u].name);
					
					if(strlen(tile_move_list_text)<65000)
					{
						strcat(tile_move_list_text, temptext);
					}
					else
					{
						if(!flood)
						{
							strcat(tile_move_list_text, "...\n...\n...\nmany others");
							flood=true;
						}
					}
					
					found=true;
				}
			}
		}
		
		if(found)
		{
			sprintf(buf2, "The tiles used by the following dmap-specific");
			sprintf(buf3, "subscreen maps will be partially or completely");
			sprintf(buf4, "overwritten by this process.  Proceed?");
			
			tile_move_list_dlg[1].dp=buf2;
			tile_move_list_dlg[2].dp=buf3;
			tile_move_list_dlg[3].dp=buf4;
			tile_move_list_dlg[4].dp=tile_move_list_text;
			tile_move_list_dlg[4].d2=0;
			
			if(TileProtection)
			{
				large_dialog(tile_move_list_dlg);
					
				int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
				position_mouse_z(0);
				
				if(ret!=5)
				{
					done = true;
				}
			}
		}
	}
	
	//check enemies
	if(!done)
	{
		//this is here to allow this section to fold
		tile_move_list_text[0]=0;
		found=false;
		flood=false;
		build_bie_list(false);
		bool newtiles=get_bit(quest_rules,qr_NEWENEMYTILES)!=0;
		int32_t u;
		
		for(u=0; u<eMAXGUYS; u++)
		{
			const guydata& enemy=guysbuf[bie[u].i];
			bool darknut=false;
			int32_t gleeok=0;
			
			if(enemy.family==eeWALK && ((enemy.flags&(inv_back|inv_front|inv_left|inv_right))!=0))
				darknut=true;
			else if(enemy.family==eeGLEEOK)
			{
				// Not certain this is the right thing to check...
				if(enemy.misc3==0)
					gleeok=1;
				else
					gleeok=2;
			}
			
			// Dummied out enemies
			if(bie[u].i>=eOCTO1S && bie[u].i<e177)
			{
				if(old_guy_string[bie[u].i][strlen(old_guy_string[bie[u].i])-1]==' ')
				{
					continue;
				}
			}
			
			if(newtiles)
			{
				if(guysbuf[bie[u].i].e_tile==0)
				{
					continue;
				}
				
				if(guysbuf[bie[u].i].e_height==0)
				{
					i=move_intersection_sr(guysbuf[bie[u].i].e_tile, guysbuf[bie[u].i].e_tile+zc_max(guysbuf[bie[u].i].e_width-1, 0), dest_left, dest_top, dest_width, dest_height);
				}
				else
				{
					i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, dest_left, dest_top, dest_width, dest_height);
				}
				
				if(i!=ti_none)
				{
					sprintf(temptext, "%s\n", bie[u].s);
					
					if(strlen(tile_move_list_text)<65000)
					{
						strcat(tile_move_list_text, temptext);
					}
					else
					{
						if(!flood)
						{
							strcat(tile_move_list_text, "...\n...\n...\nmany others");
							flood=true;
						}
					}
					
					found=true;
				}
				
				if(darknut)
				{
					i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile+120), TILEROW(guysbuf[bie[u].i].e_tile+120), guysbuf[bie[u].i].e_width, guysbuf[bie[u].i].e_height, dest_left, dest_top, dest_width, dest_height);
					
					if(i!=ti_none)
					{
						sprintf(temptext, "%s (broken shield)\n", bie[u].s);
						
						if(strlen(tile_move_list_text)<65000)
						{
							strcat(tile_move_list_text, temptext);
						}
						else
						{
							if(!flood)
							{
								strcat(tile_move_list_text, "...\n...\n...\nmany others");
								flood=true;
							}
						}
						
						found=true;
					}
				}
				else if(enemy.family==eeGANON && i==ti_none)
				{
					i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile), TILEROW(guysbuf[bie[u].i].e_tile)+2, 20, 4, dest_left, dest_top, dest_width, dest_height);
					
					if(i!=ti_none)
					{
						sprintf(temptext, "%s\n", bie[u].s);
						
						if(strlen(tile_move_list_text)<65000)
						{
							strcat(tile_move_list_text, temptext);
						}
						else
						{
							if(!flood)
							{
								strcat(tile_move_list_text, "...\n...\n...\nmany others");
								flood=true;
							}
						}
						
						found=true;
					}
				}
				else if(gleeok && i==ti_none)
				{
					for(int32_t j=0; j<4 && i==ti_none; ++j)
					{
						i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].e_tile+(gleeok>1?-4:8)), TILEROW(guysbuf[bie[u].i].e_tile+8)+(j<<1)+(gleeok>1?1:0), 4, 1, dest_left, dest_top, dest_width, dest_height);
					}
					
					if(i==ti_none)
					{
						int32_t c=TILECOL(guysbuf[bie[u].i].e_tile)+(gleeok>1?-12:0);
						int32_t r=TILEROW(guysbuf[bie[u].i].e_tile)+(gleeok>1?17:8);
						
						i=move_intersection_rr(c, r, 20, 3, dest_left, dest_top, dest_width, dest_height);
						
						if(i==ti_none)
						{
							i=move_intersection_rr(c, r+3, 16, 6, dest_left, dest_top, dest_width, dest_height);
						}
					}
					
					if(i!=ti_none)
					{
						sprintf(temptext, "%s\n", bie[u].s);
						
						if(strlen(tile_move_list_text)<65000)
						{
							strcat(tile_move_list_text, temptext);
						}
						else
						{
							if(!flood)
							{
								strcat(tile_move_list_text, "...\n...\n...\nmany others");
								flood=true;
							}
						}
						
						found=true;
					}
				}
			}
			else
			{
				if(guysbuf[bie[u].i].tile==0)
				{
					continue;
				}
				else if(guysbuf[bie[u].i].height==0)
				{
					i=move_intersection_sr(guysbuf[bie[u].i].tile, guysbuf[bie[u].i].tile+zc_max(guysbuf[bie[u].i].width-1, 0), dest_left, dest_top, dest_width, dest_height);
				}
				else
				{
					i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].tile), TILEROW(guysbuf[bie[u].i].tile), guysbuf[bie[u].i].width, guysbuf[bie[u].i].height, dest_left, dest_top, dest_width, dest_height);
				}
				
				if(i!=ti_none)
				{
					sprintf(temptext, "%s\n", bie[u].s);
					
					if(strlen(tile_move_list_text)<65000)
					{
						strcat(tile_move_list_text, temptext);
					}
					else
					{
						if(!flood)
						{
							strcat(tile_move_list_text, "...\n...\n...\nmany others");
							flood=true;
						}
					}
					
					found=true;
				}
				
				if(guysbuf[bie[u].i].s_tile!=0)
				{
					if(guysbuf[bie[u].i].s_height==0)
					{
						i=move_intersection_sr(guysbuf[bie[u].i].s_tile, guysbuf[bie[u].i].s_tile+zc_max(guysbuf[bie[u].i].s_width-1, 0), dest_left, dest_top, dest_width, dest_height);
					}
					else
					{
						i=move_intersection_rr(TILECOL(guysbuf[bie[u].i].s_tile), TILEROW(guysbuf[bie[u].i].s_tile), guysbuf[bie[u].i].s_width, guysbuf[bie[u].i].s_height, dest_left, dest_top, dest_width, dest_height);
					}
					
					if(i!=ti_none)
					{
						sprintf(temptext, "%s (%s)\n", bie[u].s, darknut?"broken shield":"secondary tiles");
						
						if(strlen(tile_move_list_text)<65000)
						{
							strcat(tile_move_list_text, temptext);
						}
						else
						{
							if(!flood)
							{
								strcat(tile_move_list_text, "...\n...\n...\nmany others");
								flood=true;
							}
						}
						
						found=true;
					}
				}
			}
		}
		
		if(found)
		{
			sprintf(buf2, "The tiles used by the following enemies");
			sprintf(buf3, "will be partially or completely");
			sprintf(buf4, "overwritten by this process.  Proceed?");
			
			tile_move_list_dlg[1].dp=buf2;
			tile_move_list_dlg[2].dp=buf3;
			tile_move_list_dlg[3].dp=buf4;
			tile_move_list_dlg[4].dp=tile_move_list_text;
			tile_move_list_dlg[4].d2=0;
			
			if(TileProtection)
			{
				large_dialog(tile_move_list_dlg);
					
				int32_t ret=zc_popup_dialog(tile_move_list_dlg,2);
				position_mouse_z(0);
				
				if(ret!=5)
				{
					done = true;
				}
			}
		}
	}
	//}
	
	if(!done)
	{
		go_tiles();
		
		int32_t diff=dest_first-src_first;
		BITMAP *srcbmp = create_bitmap_ex(8,src_width*16,src_height*16),
			*destbmp = create_bitmap_ex(8,dest_width*16,dest_height*16);
		clear_bitmap(srcbmp); clear_bitmap(destbmp);
		overtileblock16(srcbmp, src_first, 0, 0, src_width, src_height, cs, 0);
		bool is8bit = newtilebuf[src_first].format == tf8Bit;
		stretch_blit(srcbmp, destbmp, 0, 0, srcbmp->w, srcbmp->h,
			0, 0, destbmp->w, destbmp->h);
		int32_t mhei = zc_max(src_height,dest_height),
			mwid = zc_max(src_width, dest_width);
		for(int32_t r=0; r<mhei; ++r)
		{
			for(int32_t c=0; c<mwid; ++c)
			{
				int32_t dt=(dest_first+((r*TILES_PER_ROW)+c));
				
				if(dt>=NEWMAXTILES)
					continue;
				if(r < dest_height && c < dest_width)
				{
					write_tile(newtilebuf, destbmp, dt, c*16, r*16, is8bit, false);
				}
				else reset_tile(newtilebuf, dt, tf4Bit);
			}
		}
	}
	
	register_blank_tiles();
	register_used_tiles();
	return !done;
}

void copy_combos(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool masscopy)
{
	//these 2 shouldn't be needed, but just to be safe...
	reset_combo_animations();
	reset_combo_animations2();
	
	if(tile2<tile)
	{
		zc_swap(tile,tile2);
	}
	
	if(!masscopy)
	{
		if(tile==copy)
		{
			copy=-1;
			tile2=tile;
			return;
		}
		
		// go_combos(); // commented because caller does it for us
		//if copying to an earlier combo, copy from left to right
		//otherwise, copy from right to left
		for(int32_t t=(tile<copy)?0:(copycnt-1); (tile<copy)?(t<copycnt):(t>=0); (tile<copy)?(t++):(t--))
		{
			if(tile+t < MAXCOMBOS)
			{
				combobuf[tile+t]=combobuf[copy+t];
			}
		}
		
		copy=-1;
		tile2=tile;
		saved=false;
	}
	else
	{
		// go_combos();
		int32_t src=copy, dest=tile;
		
		do
		{
			combobuf[dest]=combobuf[src];
			++src;
			++dest;
			
			if((src-copy)==copycnt) src=copy;
		}
		while(dest<=tile2);
		
		copy=-1;
		tile2=tile;
		saved=false;
	}
	
	setup_combo_animations();
	setup_combo_animations2();
	return;
}

void do_movecombo(combo_move_data const& cmd)
{
	reset_combo_animations();
	reset_combo_animations2();
	go_combos();
	
	for(int32_t t=(cmd.tile<cmd.copy1)?0:(cmd.copycnt-1); (cmd.tile<cmd.copy1)?(t<cmd.copycnt):(t>=0); (cmd.tile<cmd.copy1)?(t++):(t--))
	{
		if(cmd.tile+t < MAXCOMBOS)
		{
			combobuf[cmd.tile+t]=combobuf[cmd.copy1+t];
			clear_combo(cmd.copy1+t);
		}
	}
	int32_t diff = cmd.tile - cmd.copy1;
	for(int32_t i=0; i<map_count && i<MAXMAPS2; i++)
	{
		for(int32_t j=0; j<MAPSCRS; j++)
		{
			mapscr& scr = TheMaps[i*MAPSCRS+j];
			for(int32_t k=0; k<176; k++)
			{
				if((scr.data[k]>=cmd.copy1)&&(scr.data[k]<cmd.copy1+cmd.copycnt))
				{
					scr.data[k] += diff;
				}
			}
			
			for(int32_t k=0; k<128; k++)
			{
				if((scr.secretcombo[k]>=cmd.copy1)&& (scr.secretcombo[k]<cmd.copy1+cmd.copycnt))
				{
					scr.secretcombo[k] += diff;
				}
			}
			
			if((scr.undercombo>=cmd.copy1)&&(scr.undercombo<cmd.copy1+cmd.copycnt))
			{
				scr.undercombo += diff;
			}
			
			word maxffc = scr.numFFC();
			for(word k=0; k<maxffc; k++)
			{
				ffcdata& ffc = scr.ffcs[k];
				if((ffc.getData() >= cmd.copy1) && (ffc.getData() < cmd.copy1+cmd.copycnt)
					&& (ffc.getData() != 0) && (ffc.getData()+diff!=0))
				{
					ffc.incData(diff);
				}
			}
		}
	}
	
	for(int32_t i=0; i<MAXDOORCOMBOSETS; i++)
	{
		for(int32_t j=0; j<9; j++)
		{
			if(j<4)
			{
				if((DoorComboSets[i].walkthroughcombo[j]>=cmd.copy1)&&(DoorComboSets[i].walkthroughcombo[j]<cmd.copy1+cmd.copycnt))
				{
					DoorComboSets[i].walkthroughcombo[j] += diff;
				}
				
				if(j<3)
				{
					if(j<2)
					{
						if((DoorComboSets[i].bombdoorcombo_u[j]>=cmd.copy1)&&(DoorComboSets[i].bombdoorcombo_u[j]<cmd.copy1+cmd.copycnt))
						{
							DoorComboSets[i].bombdoorcombo_u[j] += diff;
						}
						
						if((DoorComboSets[i].bombdoorcombo_d[j]>=cmd.copy1)&&(DoorComboSets[i].bombdoorcombo_d[j]<cmd.copy1+cmd.copycnt))
						{
							DoorComboSets[i].bombdoorcombo_d[j] += diff;
						}
					}
					
					if((DoorComboSets[i].bombdoorcombo_l[j]>=cmd.copy1)&&(DoorComboSets[i].bombdoorcombo_l[j]<cmd.copy1+cmd.copycnt))
					{
						DoorComboSets[i].bombdoorcombo_l[j] += diff;
					}
					
					if((DoorComboSets[i].bombdoorcombo_r[j]>=cmd.copy1)&&(DoorComboSets[i].bombdoorcombo_r[j]<cmd.copy1+cmd.copycnt))
					{
						DoorComboSets[i].bombdoorcombo_r[j] += diff;
					}
				}
			}
			
			for(int32_t k=0; k<6; k++)
			{
				if(k<4)
				{
					if((DoorComboSets[i].doorcombo_u[j][k]>=cmd.copy1)&&(DoorComboSets[i].doorcombo_u[j][k]<cmd.copy1+cmd.copycnt))
					{
						DoorComboSets[i].doorcombo_u[j][k] += diff;
					}
					
					if((DoorComboSets[i].doorcombo_d[j][k]>=cmd.copy1)&&(DoorComboSets[i].doorcombo_d[j][k]<cmd.copy1+cmd.copycnt))
					{
						DoorComboSets[i].doorcombo_d[j][k] += diff;
					}
				}
				
				if((DoorComboSets[i].doorcombo_l[j][k]>=cmd.copy1)&&(DoorComboSets[i].doorcombo_l[j][k]<cmd.copy1+cmd.copycnt))
				{
					DoorComboSets[i].doorcombo_l[j][k] += diff;
				}
				
				if((DoorComboSets[i].doorcombo_r[j][k]>=cmd.copy1)&&(DoorComboSets[i].doorcombo_r[j][k]<cmd.copy1+cmd.copycnt))
				{
					DoorComboSets[i].doorcombo_r[j][k] += diff;
				}
			}
		}
	}
	
	for(int32_t i=0; i<MAXCOMBOS; i++)
	{
		newcombo& cmb = combobuf[i];
		if(cmb.nextcombo && (cmb.nextcombo>=cmd.copy1)&&(cmb.nextcombo<cmd.copy1+cmd.copycnt))
		{
			cmb.nextcombo += diff;
		}
		if(cmb.liftcmb && (cmb.liftcmb>=cmd.copy1)&&(cmb.liftcmb<cmd.copy1+cmd.copycnt))
		{
			cmb.liftcmb += diff;
		}
		if(cmb.liftundercmb && (cmb.liftundercmb>=cmd.copy1)&&(cmb.liftundercmb<cmd.copy1+cmd.copycnt))
		{
			cmb.liftundercmb += diff;
		}
		if(cmb.prompt_cid && (cmb.prompt_cid>=cmd.copy1)&&(cmb.prompt_cid<cmd.copy1+cmd.copycnt))
		{
			cmb.prompt_cid += diff;
		}
	}
	for(auto q = 0; q < MAXCOMBOPOOLS; ++q)
	{
		combo_pool& pool = combo_pools[q];
		for(cpool_entry& cp : pool.combos)
		{
			if(cp.cid && (cp.cid >= cmd.copy1) && (cp.cid < cmd.copy1+cmd.copycnt))
			{
				cp.cid += diff;
			}
		}
	}
	
	for(int32_t i=0; i<MAXCOMBOALIASES; i++)
	{
		//dimensions are 1 less than you would expect -DD
		int32_t count=(comboa_lmasktotal(combo_aliases[i].layermask)+1)*(combo_aliases[i].width+1)*(combo_aliases[i].height+1);
		
		for(int32_t j=0; j<count; j++)
		{
		
			if((combo_aliases[i].combos[j]>=cmd.copy1)&&(combo_aliases[i].combos[j]<cmd.copy1+cmd.copycnt)&&(combo_aliases[i].combos[j]!=0))
			{
				combo_aliases[i].combos[j] += diff;
			}
		}
	}
	
	for(int32_t i=0; i<MAXFAVORITECOMBOS; i++)
	{
		if(favorite_combos[i]>=cmd.copy1 && favorite_combos[i]<cmd.copy1+cmd.copycnt)
			favorite_combos[i] += diff;
	}
	
	for(auto q = 0; q < 256; ++q)
	{
		for(auto p = 0; p < 3; ++p)
		{
			if(misc.bottle_shop_types[q].comb[p] >= cmd.copy1 && misc.bottle_shop_types[q].comb[p] < cmd.copy1+cmd.copycnt)
				misc.bottle_shop_types[q].comb[p] += diff;
		}
	}
	
	setup_combo_animations();
	setup_combo_animations2();
	saved=false;
}

void move_combos(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt)
{
	if(tile2<tile)
	{
		zc_swap(tile,tile2);
	}
	
	if(tile==copy)
	{
		copy=-1;
		tile2=tile;
		return;
	}
	
	combo_move_data cmd;
	cmd.tile = tile;
	cmd.tile2 = tile2;
	cmd.copy1 = copy;
	cmd.copycnt = copycnt;
	
	do_movecombo(cmd);
	if(last_combo_move)
		delete last_combo_move;
	last_combo_move = new combo_move_data(cmd);
	copy=-1;
	tile2=tile;
}

void do_delete_tiles(int32_t firsttile, int32_t lasttile, bool rect_sel)
{
	if(firsttile > lasttile)
		zc_swap(firsttile,lasttile);
	int32_t coldiff = 0;
	if(rect_sel && TILECOL(firsttile)>TILECOL(lasttile))
	{
		coldiff=TILECOL(firsttile)-TILECOL(lasttile);
		firsttile-=coldiff;
		lasttile+=coldiff;
	}
	for(int32_t t=firsttile; t<=lasttile; ++t)
		if(!rect_sel ||
				((TILECOL(t)>=TILECOL(firsttile)) &&
				 (TILECOL(t)<=TILECOL(lasttile))))
			reset_tile(newtilebuf, t, tf4Bit);
	saved=false;
	register_blank_tiles();
}

void delete_tiles(int32_t &tile,int32_t &tile2,bool rect_sel)
{
	char buf[40];
	
	if(tile==tile2)
	{
		sprintf(buf,"Delete tile %d?",tile);
	}
	else
	{
		sprintf(buf,"Delete tiles %d-%d?",zc_min(tile,tile2),zc_max(tile,tile2));
	}
	
	if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
	{
		int32_t firsttile=zc_min(tile,tile2), lasttile=zc_max(tile,tile2), coldiff=0;
		
		go_tiles();
		
		//if copying to an earlier tile, copy from left to right
		//otherwise, copy from right to left
		do_delete_tiles(firsttile, lasttile, rect_sel);
				
		tile=tile2=zc_min(tile,tile2);
		saved=false;
		register_blank_tiles();
	}
}

void overlay_tile2(int32_t dest,int32_t src,int32_t cs,bool backwards)
{
	byte buf[256];
	go_tiles();
	
	unpack_tile(newtilebuf, dest, 0, false);
	
	for(int32_t i=0; i<256; i++)
		buf[i] = unpackbuf[i];
		
	unpack_tile(newtilebuf, src, 0, false);
	
	if(newtilebuf[src].format>tf4Bit)
	{
		cs=0;
	}
	
	cs &= 15;
	cs <<= CSET_SHFT;
	
	for(int32_t i=0; i<256; i++)
	{
		if(backwards)
		{
			if(!buf[i])
			{
				buf[i] = unpackbuf[i]+cs;
			}
		}
		else
		{
			if(unpackbuf[i])
			{
				buf[i] = unpackbuf[i]+cs;
			}
		}
	}
	
	pack_tile(newtilebuf, buf,dest);
	saved=false;
}

void mass_overlay_tile(int32_t dest1, int32_t dest2, int32_t src, int32_t cs, bool backwards, bool rect_sel)
{
	//byte buf[256];
	go_tiles();
	
	/*unpack_tile(newtilebuf, src, 0, false);
	
	for(int32_t i=0; i<256; i++)
		buf[i] = unpackbuf[i];
		
	if(newtilebuf[src].format>tf4Bit)
	{
		cs=0;
	}
	
	
	cs &= 15;
	cs <<= CSET_SHFT;
	*/
	if(!rect_sel)
	{
		for(int32_t d=dest1; d <= dest2; ++d)
		{
			/*unpack_tile(newtilebuf, d, 0, false);
			
			for(int32_t i=0; i<256; i++)
			{
				if(!backwards)
				{
					if(!buf[i])
					{
						buf[i] = unpackbuf[i] + cs;
					}
				}
				else
				{
					if(unpackbuf[i])
					{
						buf[i] = unpackbuf[i] + cs;
					}
				}
			}
			
			pack_tile(newtilebuf, buf,d);
		*/
		
		overlay_tile(newtilebuf,d,src,cs,backwards);
			
			if(!blank_tile_table[src])
			{
				blank_tile_table[d]=false;
			}
		}
	}
	else
	{
		int32_t rmin=zc_min(TILEROW(dest1),TILEROW(dest2));
		int32_t rmax=zc_max(TILEROW(dest1),TILEROW(dest2));
		int32_t cmin=zc_min(TILECOL(dest1),TILECOL(dest2));
		int32_t cmax=zc_max(TILECOL(dest1),TILECOL(dest2));
		int32_t d=0;
		
		for(int32_t j=cmin; j<=cmax; ++j)
		{
			for(int32_t k=rmin; k<=rmax; ++k)
			{
				d=j+TILES_PER_ROW*k;
				/*unpack_tile(newtilebuf, d, 0, false);
				
				for(int32_t i=0; i<256; i++)
				{
					if(!backwards)
					{
						if(!buf[i])
						{
							buf[i] = unpackbuf[i] + cs;
						}
					}
					else
					{
						if(unpackbuf[i])
						{
							buf[i] = unpackbuf[i] + cs;
						}
					}
				}
			
				pack_tile(newtilebuf, buf,d);
				*/
		
		overlay_tile(newtilebuf,d,src,cs,backwards);
		
				if(!blank_tile_table[src])
				{
					blank_tile_table[d]=false;
				}
			}
		}
	}
	
	return;
}

void sel_tile(int32_t &tile, int32_t &tile2, int32_t &first, int32_t type, int32_t s)
{
	tile+=s;
	bound(tile,0,NEWMAXTILES-1);
	
	if(type!=0 || !(key[KEY_LSHIFT] || key[KEY_RSHIFT]))
		tile2 = tile;
		
	first = tile - (tile%TILES_PER_PAGE);
}

void convert_tile(int32_t t, int32_t bp2, int32_t cs, bool shift, bool alt)
{
	int32_t cst;
	
	switch(bp2)
	{
	case tf4Bit:
		switch(newtilebuf[t].format)
		{
		case tf4Bit:
			//already in the right format
			break;
			
		case tf8Bit:
			unpack_tile(newtilebuf, t, 0, true);
			
			if(alt)  //reduce
			{
				for(int32_t i=0; i<256; i++)
				{
					if(!shift||unpackbuf[i]!=0)
					{
						unpackbuf[i]=(cset_reduce_table[unpackbuf[i]]);
					}
				}
			}
			else //truncate
			{
				for(int32_t i=0; i<256; i++)
				{
					unpackbuf[i]&=15;
				}
			}
			
			reset_tile(newtilebuf, t, tf4Bit);
			pack_tile(newtilebuf, unpackbuf, t);
			break;
		}
		
		break;
		
	case tf8Bit:
		switch(newtilebuf[t].format)
		{
		case tf4Bit:
			unpack_tile(newtilebuf, t, 0, true);
			cst = cs&15;
			cst <<= CSET_SHFT;
			
			for(int32_t i=0; i<256; i++)
			{
				if(!shift||unpackbuf[i]!=0)
				{
					unpackbuf[i]+=cst;
				}
			}
			
			reset_tile(newtilebuf, t, tf8Bit);
			pack_tile(newtilebuf, unpackbuf, t);
			break;
			
		case tf8Bit:
			//already in the right format
			break;
		}
		
		break;
	}
}

static DIALOG create_relational_tiles_dlg[] =
{
	// (dialog proc)       (x)   (y)    (w)     (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
	{ jwin_win_proc,         0,    0,   160,     92,   vc(0),              vc(11),           0,       D_EXIT,     0,             0, (void *) "Tile Setup", NULL, NULL },
	{ jwin_rtext_proc,      74,   28,    48,      8,   jwin_pal[jcBOXFG],  jwin_pal[jcBOX],  0,       0,          0,             0, (void *) "Frames:", NULL, NULL },
	{ jwin_edit_proc,       78,   24,    48,     16,   0,                  0,                0,       0,          6,             0,       NULL, NULL, NULL },
	{ jwin_radio_proc,       8,   44,    64,      9,   vc(14),             vc(1),            0,       D_SELECTED, 0,             0, (void *) "Relational", NULL, NULL },
	{ jwin_radio_proc,      68,   44,    64,      9,   vc(14),             vc(1),            0,       0,          0,             0, (void *) "Dungeon Carving", NULL, NULL },
	{ jwin_button_proc,     10,   66,    61,     21,   vc(0),              vc(11),           13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
	{ jwin_button_proc,     90,   66,    61,     21,   vc(0),              vc(11),           27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
	{ d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
	{ NULL,                  0,    0,     0,      0,   0,                  0,                0,       0,          0,             0,       NULL, NULL,  NULL }
};

void draw_tile_list_window()
{
	int32_t w = 640;
	int32_t h = 480;
	
	if(a4_bmp_active())
		clear_to_color(screen, get_zqdialog_a4_clear_color()); //!TODO Remove when a5 dialog done - Clear a4 screen layer
	jwin_draw_win_a5(0, 0, w+6+6, h+25+6, FR_WIN);
	jwin_draw_frame_a5(4, 23, w+2+2, h+4+2-64, FR_DEEP);
	
	ALLEGRO_FONT *oldfont = a5font;
	a5font = get_custom_font_a5(CFONT_TITLE);
	jwin_draw_titlebar_a5(3, 3, w+6, 18, "Select Tile", true);
	a5font=oldfont;
}

void show_blank_tile(int32_t t)
{
	char tbuf[80], tbuf2[80], tbuf3[80];
	sprintf(tbuf, "Tile is%s blank.", blank_tile_table[t]?"":" not");
	sprintf(tbuf2, "%c %c", blank_tile_quarters_table[t*4]?'X':'-', blank_tile_quarters_table[(t*4)+1]?'X':'-');
	sprintf(tbuf3, "%c %c", blank_tile_quarters_table[(t*4)+2]?'X':'-', blank_tile_quarters_table[(t*4)+3]?'X':'-');
	jwin_alert("Blank Tile Information",tbuf,tbuf2,tbuf3,"&OK",NULL,13,27,lfont);
}

void do_convert_tile(int32_t tile, int32_t tile2, int32_t cs, bool rect_sel, bool fourbit, bool shift, bool alt)
{
	char buf[80];
	sprintf(buf, "Do you want to convert the selected %s to %d-bit color?", tile==tile2?"tile":"tiles",fourbit?4:8);
	
	if(jwin_alert("Convert Tile?",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
	{
		go_tiles();
		saved=false;
		
		if(fourbit)
		{
			memset(cset_reduce_table, 0, 256);
			memset(col_diff,0,3*128);
			calc_cset_reduce_table(RAMpal, cs);
		}
		
		int32_t firsttile=zc_min(tile,tile2), lasttile=zc_max(tile,tile2), coldiff=0;
		
		if(rect_sel && TILECOL(firsttile)>TILECOL(lasttile))
		{
			coldiff=TILECOL(firsttile)-TILECOL(lasttile);
			firsttile-=coldiff;
			lasttile+=coldiff;
		}
		
		for(int32_t t=firsttile; t<=lasttile; t++)
			if(!rect_sel ||
					((TILECOL(t)>=TILECOL(firsttile)) &&
					 (TILECOL(t)<=TILECOL(lasttile))))
				convert_tile(t, fourbit?tf4Bit:tf8Bit, cs, shift, alt);
				
		tile=tile2=zc_min(tile,tile2);
	}
}


int32_t readtilefile(PACKFILE *f)
{
	dword section_version=0;
	dword section_cversion=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	al_trace("readoneweapon section_version: %d\n", section_version);
	al_trace("readoneweapon section_cversion: %d\n", section_cversion);

	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .ztile packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_TILES ) || ( section_version == V_TILES && section_cversion < CV_TILES ) )
	{
		al_trace("Cannot read .ztile packfile made using V_TILES (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .ztile packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	int32_t index = 0;
	int32_t count = 0;
	
	//tile id
	if(!p_igetl(&index,f,true))
	{
		return 0;
	}
	al_trace("Reading tile: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f,true))
	{
		return 0;
	}
	al_trace("Reading tile: count(%d)\n", count);
	
	
	

	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		byte *temp_tile = new byte[tilesize(tf32Bit)];
		byte format=tf4Bit;
		memset(temp_tile, 0, tilesize(tf32Bit));
		if(!p_getc(&format,f,true))
		{
			delete[] temp_tile;
			return 0;
		}

				
		if(!pfread(temp_tile,tilesize(format),f,true))
		{
			delete[] temp_tile;
			return 0;
		}
				
		reset_tile(newtilebuf, index+(tilect), format);
		memcpy(newtilebuf[index+(tilect)].data,temp_tile,tilesize(newtilebuf[index+(tilect)].format));
		delete[] temp_tile;
	}
	
	
	//::memcpy(&(newtilebuf[tile_index]),&temptile,sizeof(tiledata));
	
	register_blank_tiles();
	register_used_tiles();
			
	return 1;
	
}

int32_t readtilefile_to_location(PACKFILE *f, int32_t start, int32_t skip)
{
	dword section_version=0;
	dword section_cversion=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	al_trace("readoneweapon section_version: %d\n", section_version);
	al_trace("readoneweapon section_cversion: %d\n", section_cversion);

	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .ztile packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_TILES ) || ( section_version == V_TILES && section_cversion < CV_TILES ) )
	{
		al_trace("Cannot read .ztile packfile made using V_TILES (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .ztile packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	int32_t index = 0;
	int32_t count = 0;
	
	//tile id
	if(!p_igetl(&index,f,true))
	{
		return 0;
	}
	al_trace("Reading tile: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f,true))
	{
		return 0;
	}
	al_trace("Reading tile: count(%d)\n", count);
	

	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		byte *temp_tile = new byte[tilesize(tf32Bit)];
		byte format=tf4Bit;
		memset(temp_tile, 0, tilesize(tf32Bit));
		if(!p_getc(&format,f,true))
		{
			delete[] temp_tile;
			return 0;
		}

				
		if(!pfread(temp_tile,tilesize(format),f,true))
		{
			delete[] temp_tile;
			return 0;
		}
				
		reset_tile(newtilebuf, start+(tilect), format);
		if ( skip )
		{
			if ( (start+(tilect)) < skip ) 
			{
				delete[] temp_tile;
				continue;
			}
			
		}
		if ( start+(tilect) < NEWMAXTILES )
		{
			memcpy(newtilebuf[start+(tilect)].data,temp_tile,tilesize(newtilebuf[start+(tilect)].format));
		}
		delete[] temp_tile;
		
	}
	
	
	//::memcpy(&(newtilebuf[tile_index]),&temptile,sizeof(tiledata));
	
	register_blank_tiles();
	register_used_tiles();
			
	return 1;
	
}


int32_t readtilefile_to_location(PACKFILE *f, int32_t start)
{
	dword section_version=0;
	dword section_cversion=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	al_trace("readoneweapon section_version: %d\n", section_version);
	al_trace("readoneweapon section_cversion: %d\n", section_cversion);

	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .ztile packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_TILES ) || ( section_version == V_TILES && section_cversion < CV_TILES ) )
	{
		al_trace("Cannot read .ztile packfile made using V_TILES (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .ztile packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	int32_t index = 0;
	int32_t count = 0;
	
	//tile id
	if(!p_igetl(&index,f,true))
	{
		return 0;
	}
	al_trace("Reading tile: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f,true))
	{
		return 0;
	}
	al_trace("Reading tile: count(%d)\n", count);
	
	
	

	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		byte *temp_tile = new byte[tilesize(tf32Bit)];
		byte format=tf4Bit;
		memset(temp_tile, 0, tilesize(tf32Bit));
		
		if(!p_getc(&format,f,true))
		{
			delete[] temp_tile;
			return 0;
		}

				
		if(!pfread(temp_tile,tilesize(format),f,true))
		{
			delete[] temp_tile;
			return 0;
		}
				
		reset_tile(newtilebuf, start+(tilect), format);
		if ( start+(tilect) < NEWMAXTILES )
		{
			memcpy(newtilebuf[start+(tilect)].data,temp_tile,tilesize(newtilebuf[start+(tilect)].format));
		}
		delete[] temp_tile;
	}
	
	
	//::memcpy(&(newtilebuf[tile_index]),&temptile,sizeof(tiledata));
	
	register_blank_tiles();
	register_used_tiles();
			
	return 1;
	
}
int32_t writetilefile(PACKFILE *f, int32_t index, int32_t count)
{
	dword section_version=V_TILES;
	dword section_cversion=CV_TILES;
	int32_t zversion = ZELDA_VERSION;
	int32_t zbuild = VERSION_BUILD;
	
	if(!p_iputl(zversion,f))
	{
		return 0;
	}
	if(!p_iputl(zbuild,f))
	{
		return 0;
	}
	if(!p_iputw(section_version,f))
	{
		return 0;
	}
	
	if(!p_iputw(section_cversion,f))
	{
		return 0;
	}
	
	//start tile id
	if(!p_iputl(index,f))
	{
		return 0;
	}
	
	//count
	if(!p_iputl(count,f))
	{
		return 0;
	}
	
	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		//al_trace("Tile id: %d\n",index+(tilect));
		if(!p_putc(newtilebuf[index+(tilect)].format,f))
		{
			return 0;
		}
		//al_trace("Tile format: %d\n", newtilebuf[index+(tilect)].format);
		if(!pfwrite(newtilebuf[index+(tilect)].data,tilesize(newtilebuf[index+(tilect)].format),f))
		{
			return 0;
		}
	}
	
	return 1;
	
}

static int32_t _selected_tile=-1, _selected_tcset=-1;
int32_t select_tile(int32_t &tile,int32_t &flip,int32_t type,int32_t &cs,bool edit_cs,int32_t exnow, bool always_use_flip)
{
	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-31)>>1;
	popup_zqdialog_start_a5(window_xofs,window_yofs,w+12,h+31);
	popup_zqdialog_start(window_xofs,window_yofs,w+12,h+31,0xFF);
	reset_combo_animations();
	reset_combo_animations2();
	bound(tile,0,NEWMAXTILES-1);
	ex=exnow;
	int32_t done=0;
	int32_t oflip=flip;
	int32_t otile=tile;
	int32_t ocs=cs;
	int32_t first=(tile/TILES_PER_PAGE)*TILES_PER_PAGE; //first tile on the current page
	int32_t copy=-1;
	int32_t tile2=tile,copycnt=0;
	int32_t tile_clicked=-1;
	bool rect_sel=true;
	bound(first,0,(TILES_PER_PAGE*TILE_PAGES)-1);
	position_mouse_z(0);
	
	go();
	
	register_used_tiles();
	int32_t screen_xofs=6;
	int32_t screen_yofs=25;
	int32_t panel_yofs=3;
	int32_t mul = 2;
	FONT *tfont = lfont_l;
	ALLEGRO_FONT *tfont_a5 = get_zc_font_a5(font_lfont_l);
	
	draw_tile_list_window();
	int32_t f=0;
	draw_tiles(first,cs,f);
	
	if(type==0)
	{
		tile_info_0(tile,tile2,cs,copy,copycnt,first/TILES_PER_PAGE,rect_sel);
	}
	else
	{
		tile_info_1(otile,oflip,ocs,tile,flip,cs,copy,first/TILES_PER_PAGE, always_use_flip);
	}
	
	go_tiles();
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	bool bdown=false;
	
	#define FOREACH_START(_t) \
	{ \
		int32_t _first, _last; \
		if(is_rect) \
		{ \
			_first=top*TILES_PER_ROW+left; \
			_last=_first+rows*TILES_PER_ROW|+columns-1; \
		} \
		else \
		{ \
			_first=zc_min(tile, tile2); \
			_last=zc_max(tile, tile2); \
		} \
		for(int32_t _t=_first; _t<=_last; _t++) \
		{ \
			if(is_rect) \
			{ \
				int32_t row=TILEROW(_t); \
				if(row<top || row>=top+rows) \
					continue; \
				int32_t col=TILECOL(_t); \
				if(col<left || col>=left+columns) \
					continue; \
			} \
		
	#define FOREACH_END\
		} \
	}
	
	bool did_snap = false;
	do
	{
		rest(4);
		int32_t top=TILEROW(zc_min(tile, tile2));
		int32_t left=zc_min(TILECOL(tile), TILECOL(tile2));
		int32_t rows=TILEROW(zc_max(tile, tile2))-top+1;
		int32_t columns=zc_max(TILECOL(tile), TILECOL(tile2))-left+1;
		bool is_rect=(rows==1)||(columns==TILES_PER_ROW)||rect_sel;
		bool redraw=false;
		
		if(mouse_z!=0)
		{
			sel_tile(tile,tile2,first,type,((mouse_z/abs(mouse_z))*(-1)*TILES_PER_PAGE));
			position_mouse_z(0);
			redraw=true;
		}
		
		if(keypressed())
		{
			switch(readkey()>>8)
			{
				case KEY_ENTER_PAD:
				case KEY_ENTER:
					done=2;
					break;
				
				case KEY_ESC:
					done=1;
					break;
				
				case KEY_F1:
					onHelp();
					break;
				
				case KEY_EQUALS:
				case KEY_PLUS_PAD:
				{
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL] ||
							key[KEY_ALT] || key[KEY_ALTGR])
					{
						FOREACH_START(t)
							if(key[KEY_ALT] || key[KEY_ALTGR])
								shift_tile_colors(t, 16, false);
							else
								shift_tile_colors(t, 1, key[KEY_LSHIFT] || key[KEY_RSHIFT]);
						FOREACH_END
						
						register_blank_tiles();
					}
					else if(edit_cs)
						cs = (cs<13) ? cs+1:0;
						
					redraw=true;
					break;
				}
				
				case KEY_Z:
				case KEY_F12:
				{
					if(!did_snap)
					{
						//Export tile page as screenshot
						PALETTE temppal;
						get_palette(temppal);
						BITMAP *tempbmp=create_bitmap_ex(8,16*TILES_PER_ROW, 16*TILE_ROWS_PER_PAGE);
						draw_tiles(tempbmp,first,cs,f,false,true);
						save_bitmap(getSnapName(), tempbmp, RAMpal);
						destroy_bitmap(tempbmp);
						
						redraw = true;
						did_snap = true;
					}
					break;
				}
				
				case KEY_S:
				{
					if(!getname("Save ZTILE(.ztile)", "ztile", NULL,datapath,false))
						break;   
					PACKFILE *f=pack_fopen_password(temppath,F_WRITE, "");
					if(!f) break;
					al_trace("Saving tile: %d\n", tile);
					writetilefile(f,tile,1);
					pack_fclose(f);
					break;
				}
				case KEY_L:
				{
					if(!getname("Load ZTILE(.ztile)", "ztile", NULL,datapath,false))
						break;   
					PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
					if(!f) break;
					al_trace("Saving tile: %d\n", tile);
					if (!readtilefile(f))
					{
						al_trace("Could not read from .ztile packfile %s\n", temppath);
						jwin_alert("ZTILE File: Error","Could not load the specified Tile.",NULL,NULL,"O&K",NULL,'k',0,lfont);
					}
					else
					{
						jwin_alert("ZTILE File: Success!","Loaded the source tiles to your tile sheets!",NULL,NULL,"O&K",NULL,'k',0,lfont);
					}
				
					pack_fclose(f);
					//register_blank_tiles();
					//register_used_tiles();
					redraw=true;
					break;
				}
				case KEY_MINUS:
				case KEY_MINUS_PAD:
				{
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL] ||
							key[KEY_ALT] || key[KEY_ALTGR])
					{
						FOREACH_START(t)
							if(key[KEY_ALT] || key[KEY_ALTGR])
								shift_tile_colors(t, -16, false);
							else
								shift_tile_colors(t, -1, key[KEY_LSHIFT] || key[KEY_RSHIFT]);
						FOREACH_END
						
						register_blank_tiles();
					}
					else if(edit_cs)
						cs = (cs>0) ? cs-1:13;
						
					redraw=true;
					break;
				}
				
				case KEY_UP:
				{
					switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
					{
						case 3:  //ALT and CTRL
						case 2:  //ALT
							if(is_rect)
							{
								saved=false;
								go_slide_tiles(columns, rows, top, left);
								int32_t bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
								bool same = true;
								
								for(int32_t d=0; d<columns; d++)
								{
									for(int32_t s=0; s<rows; s++)
									{
										int32_t t=((top+s)*TILES_PER_ROW)+left+d;
										
										if(newtilebuf[t].format!=bitcheck) same = false;
									}
								}
								
								if(!same) break;
								
								for(int32_t c=0; c<columns; c++)
								{
									for(int32_t r=0; r<rows; r++)
									{
										int32_t temptile=((top+r)*TILES_PER_ROW)+left+c;
										qword *src_pixelrow=(qword*)(newundotilebuf[temptile].data+(8*bitcheck));
										qword *dest_pixelrow=(qword*)(newtilebuf[temptile].data);
										
										for(int32_t pixelrow=0; pixelrow<16*bitcheck; pixelrow++)
										{
											if(pixelrow==15*bitcheck)
											{
												int32_t srctile=temptile+TILES_PER_ROW;
												if(srctile>=NEWMAXTILES)
													srctile-=rows*TILES_PER_ROW;
												src_pixelrow=(qword*)(newtilebuf[srctile].data);
											}
											
											*dest_pixelrow=*src_pixelrow;
											dest_pixelrow++;
											src_pixelrow++;
										}
									}
									
									qword *dest_pixelrow=(qword*)(newtilebuf[((top+rows-1)*TILES_PER_ROW)+left+c].data+(120*bitcheck));
									
									for(int32_t b=0; b<bitcheck; b++,dest_pixelrow++)
									{
										if((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
										{
											*dest_pixelrow=0;
										}
										else
										{
											qword *src_pixelrow=(qword*)(newundotilebuf[(top*TILES_PER_ROW)+left+c].data+(8*b));
											*dest_pixelrow=*src_pixelrow;
										}
									}
								}
							}
							
							register_blank_tiles();
							redraw=true;
							break;
							
						case 1:  //CTRL
						case 0:  //None
							sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-1*(tile_page_row(tile)*TILES_PER_ROW):-TILES_PER_ROW);
							redraw=true;
							
						default: //Others
							break;
					}
				}
				break;
				
				case KEY_DOWN:
				{
					switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
					{
						case 3:  //ALT and CTRL
						case 2:  //ALT
							if(is_rect)
							{
								saved=false;
								go_slide_tiles(columns, rows, top, left);
								int32_t bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
								bool same = true;
								
								for(int32_t c=0; c<columns; c++)
								{
									for(int32_t r=0; r<rows; r++)
									{
										int32_t t=((top+r)*TILES_PER_ROW)+left+c;
										
										if(newtilebuf[t].format!=bitcheck) same = false;
									}
								}
								
								if(!same) break;
								
								for(int32_t c=0; c<columns; c++)
								{
									for(int32_t r=rows-1; r>=0; r--)
									{
										int32_t temptile=((top+r)*TILES_PER_ROW)+left+c;
										qword *src_pixelrow=(qword*)(newundotilebuf[temptile].data+(112*bitcheck)+(8*(bitcheck-1)));
										qword *dest_pixelrow=(qword*)(newtilebuf[temptile].data+(120*bitcheck)+(8*(bitcheck-1)));
										
										for(int32_t pixelrow=(8<<bitcheck)-1; pixelrow>=0; pixelrow--)
										{
											if(pixelrow<bitcheck)
											{
												int32_t srctile=temptile-TILES_PER_ROW;
												if(srctile<0)
													srctile+=rows*TILES_PER_ROW;
												qword *tempsrc=(qword*)(newtilebuf[srctile].data+(120*bitcheck)+(8*pixelrow));
												*dest_pixelrow=*tempsrc;
												//*dest_pixelrow=0;
											}
											else
											{
												*dest_pixelrow=*src_pixelrow;
											}
											
											dest_pixelrow--;
											src_pixelrow--;
										}
									}
									
									qword *dest_pixelrow=(qword*)(newtilebuf[(top*TILES_PER_ROW)+left+c].data);
									qword *src_pixelrow=(qword*)(newundotilebuf[((top+rows-1)*TILES_PER_ROW)+left+c].data+(120*bitcheck));
									
									for(int32_t b=0; b<bitcheck; b++)
									{
										if((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
										{
											*dest_pixelrow=0;
										}
										else
										{
											*dest_pixelrow=*src_pixelrow;
										}
										
										dest_pixelrow++;
										src_pixelrow++;
									}
								}
							}
							
							register_blank_tiles();
							redraw=true;
							break;
							
						case 1:  //CTRL
						case 0:  //None
							sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?((TILE_ROWS_PER_PAGE-1)-tile_page_row(tile))*TILES_PER_ROW:TILES_PER_ROW);
							redraw=true;
							
						default: //Others
							break;
					}
				}
				break;
				
				case KEY_LEFT:
				{
					switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
					{
						case 3:  //ALT and CTRL
						case 2:  //ALT
							if(is_rect)
							{
								saved=false;
								go_slide_tiles(columns, rows, top, left);
								int32_t bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
								bool same = true;
								
								for(int32_t c=0; c<columns; c++)
								{
									for(int32_t r=0; r<rows; r++)
									{
										int32_t t=((top+r)*TILES_PER_ROW)+left+c;
										
										if(newtilebuf[t].format!=bitcheck) same = false;
									}
								}
								
								if(!same) break;
								
								for(int32_t r=0; r<rows; r++)
								{
									for(int32_t c=0; c<columns; c++)
									{
										int32_t temptile=((top+r)*TILES_PER_ROW)+left+c;
										byte *dest_pixelrow=(newtilebuf[temptile].data);
										
										for(int32_t pixelrow=0; pixelrow<16; pixelrow++)
										{
		#ifdef ALLEGRO_LITTLE_ENDIAN
										
											//if(bitcheck==tf4Bit)
											// {
											for(int32_t p=0; p<(8*bitcheck)-1; p++)
											{
												if(bitcheck==tf4Bit)
												{
													*dest_pixelrow=*dest_pixelrow>>4;
													*dest_pixelrow|=(*(dest_pixelrow+1)<<4);
													
													if(p==6) *(dest_pixelrow+1)=*(dest_pixelrow+1)>>4;
												}
												else
												{
													*dest_pixelrow=*(dest_pixelrow+1);
												}
												
												dest_pixelrow++;
											}
											
		#else
											
											for(int32_t p=0; p<(8*bitcheck)-1; p++)
											{
												if(bitcheck==tf4Bit)
												{
													*dest_pixelrow=*dest_pixelrow<<4;
													*dest_pixelrow|=(*(dest_pixelrow+1)>>4);
											
													if(p==6) *(dest_pixelrow+1)=*(dest_pixelrow+1)<<4;
												}
												else
												{
													*dest_pixelrow=*(dest_pixelrow+1);
												}
											
												dest_pixelrow++;
											}
											
		#endif
											
											if(c==columns-1)
											{
												if(!(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
												{
													byte *tempsrc=(newundotilebuf[((top+r)*TILES_PER_ROW)+left].data+(pixelrow*8*bitcheck));
		#ifdef ALLEGRO_LITTLE_ENDIAN
													
													if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
													else *dest_pixelrow=*tempsrc;
													
		#else
													
													if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
													else *dest_pixelrow=*tempsrc;
													
		#endif
												}
											}
											else
											
											{
												byte *tempsrc=(newtilebuf[temptile+1].data+(pixelrow*8*bitcheck));
		#ifdef ALLEGRO_LITTLE_ENDIAN
												
												if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
												else *dest_pixelrow=*tempsrc;
												
		#else
												
												if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
												else *dest_pixelrow=*tempsrc;
												
		#endif
											}
											
											dest_pixelrow++;
										}
									}
								}
								
								register_blank_tiles();
								redraw=true;
							}
							
							break;
							
						case 1:  //CTRL
						case 0:  //None
							sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-(tile%TILES_PER_ROW):-1);
							redraw=true;
							
						default: //Others
							break;
					}
				}
				break;
			
				case KEY_RIGHT:
				{
					switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
					{
						case 3:  //ALT and CTRL
						case 2:  //ALT
							if(is_rect)
							{
								saved=false;
								go_slide_tiles(columns, rows, top, left);
								int32_t bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
								bool same = true;
								
								for(int32_t c=0; c<columns; c++)
								{
									for(int32_t r=0; r<rows; r++)
									{
										int32_t t=((top+r)*TILES_PER_ROW)+left+c;
										
										if(newtilebuf[t].format!=bitcheck) same = false;
									}
								}
								
								if(!same) break;
								
								for(int32_t r=0; r<rows; r++)
								{
									for(int32_t c=columns-1; c>=0; c--)
									{
										int32_t temptile=((top+r)*TILES_PER_ROW)+left+c;
										byte *dest_pixelrow=(newtilebuf[temptile].data)+(128*bitcheck)-1;
										
										for(int32_t pixelrow=15; pixelrow>=0; pixelrow--)
										{
		#ifdef ALLEGRO_LITTLE_ENDIAN
										
											//*dest_pixelrow=(*dest_pixelrow)<<4;
											for(int32_t p=0; p<(8*bitcheck)-1; p++)
											{
												if(bitcheck==tf4Bit)
												{
													*dest_pixelrow=*dest_pixelrow<<4;
													*dest_pixelrow|=(*(dest_pixelrow-1)>>4);
													
													if(p==6) *(dest_pixelrow-1)=*(dest_pixelrow-1)<<4;
												}
												else
												{
													*dest_pixelrow=*(dest_pixelrow-1);
												}
												
												dest_pixelrow--;
											}
											
		#else
											
											for(int32_t p=0; p<(8*bitcheck)-1; p++)
											{
												if(bitcheck==tf4Bit)
												{
													*dest_pixelrow=*dest_pixelrow>>4;
													*dest_pixelrow|=(*(dest_pixelrow-1)<<4);
											
													if(p==6) *(dest_pixelrow-1)=*(dest_pixelrow-1)>>4;
												}
												else
												{
													*dest_pixelrow=*(dest_pixelrow-1);
												}
											
												dest_pixelrow--;
											}
											
		#endif
											
											if(c==0)
											{
												if(!(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
												{
													byte *tempsrc=(newundotilebuf[(((top+r)*TILES_PER_ROW)+left+columns-1)].data+(pixelrow*8*bitcheck)+(8*bitcheck)-1);
		#ifdef ALLEGRO_LITTLE_ENDIAN
													
													if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
													else *dest_pixelrow=*tempsrc;
													
		#else
													
													if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
													else *dest_pixelrow=*tempsrc;
													
		#endif
												}
											}
											else
											{
												byte *tempsrc=(newtilebuf[temptile-1].data+(pixelrow*8*bitcheck)+(8*bitcheck)-1);
		#ifdef ALLEGRO_LITTLE_ENDIAN
												
												// (*dest_pixelrow)|=((*(dest_pixelrow-16))&0xF000000000000000ULL)>>60;
												if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
												else *dest_pixelrow=*tempsrc;
												
		#else
												
												if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
												else *dest_pixelrow=*tempsrc;
												
		#endif
											}
											
											dest_pixelrow--;
										}
									}
								}
								
								register_blank_tiles();
								redraw=true;
							}
							
							break;
							
						case 1:  //CTRL
						case 0:  //None
							sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?(TILES_PER_ROW)-(tile%TILES_PER_ROW)-1:1);
							redraw=true;
							
						default: //Others
							break;
					}
				}
				break;
			
				case KEY_PGUP:
					sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-1*(TILEROW(tile)*TILES_PER_ROW):-TILES_PER_PAGE);
					redraw=true;
					break;
				
				case KEY_PGDN:
					sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?((TILE_PAGES*TILE_ROWS_PER_PAGE)-TILEROW(tile)-1)*TILES_PER_ROW:TILES_PER_PAGE);
					redraw=true;
					break;
				
				case KEY_HOME:
					sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-(tile):-(tile%TILES_PER_PAGE));
					redraw=true;
					break;
				
				case KEY_END:
					sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?(TILE_PAGES)*(TILES_PER_PAGE)-tile-1:(TILES_PER_PAGE)-(tile%TILES_PER_PAGE)-1);
					redraw=true;
					break;
				
				case KEY_P:
				{
					int32_t whatPage = gettilepagenumber("Goto Page", (PreFillTileEditorPage?(first/TILES_PER_PAGE):0));
					
					if(whatPage >= 0)
						sel_tile(tile,tile2,first,type,((whatPage-TILEPAGE(tile))*TILE_ROWS_PER_PAGE)*TILES_PER_ROW);
						
					break;
				}
				
				case KEY_O:
					if(type==0 && copy>=0)
					{
						go_tiles();
						
						if(key[KEY_LSHIFT] ||key[KEY_RSHIFT])
						{
							mass_overlay_tile(zc_min(tile,tile2),zc_max(tile,tile2),copy,cs,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]), rect_sel);
							saved=false;
						}
						else
						{
							saved = !overlay_tiles(tile,tile2,copy,copycnt,rect_sel,false,cs,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]));
							//overlay_tile(newtilebuf,tile,copy,cs,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]));
						}
						
						saved=false;
						redraw=true;
					}
					
					break;
				
				case KEY_E:
					if(type==0)
					{
						edit_tile(tile,flip,cs);
						draw_tile_list_window();
						redraw=true;
					}
					
					break;
				
				case KEY_G:
					if(type==0)
					{
						grab_tile(tile,cs);
						draw_tile_list_window();
						redraw=true;
					}
					
					break;
				
				case KEY_C:
					copy=zc_min(tile,tile2);
					copycnt=abs(tile-tile2)+1;
					redraw=true;
					break;
				
				case KEY_X:
					if(type==2)
					{
						ex=(ex+1)%3;
					}
					
					break;
				
				//usetiles=true;
				case KEY_R:
					if(type==2)
						break;
					if(type==1)
					{
						flip = rotate_value(flip);
						redraw=true;
						break;
					}
						
					go_tiles();
					
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
					{
						bool go=false;
						if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
							go=true;
						else if(massRecolorSetup(cs))
							go=true;
						
						if(go)
						{
							FOREACH_START(t)
								massRecolorApply(t);
							FOREACH_END
							
							register_blank_tiles();
						}
					}
					else
					{
						FOREACH_START(t)
							rotate_tile(t,(key[KEY_LSHIFT] || key[KEY_RSHIFT]));
						FOREACH_END
					}
					
					redraw=true;
					saved=false;
					break;
				
				case KEY_SPACE:
					rect_sel=!rect_sel;
					copy=-1;
					redraw=true;
					break;
				
				//     case KEY_N:     go_tiles(); normalize(tile,tile2,flip); flip=0; redraw=true; saved=false; usetiles=true; break;
				case KEY_H:
					flip^=1;
					go_tiles();
					
					if(type==0)
					{
						normalize(tile,tile2,rect_sel,flip);
						flip=0;
					}
					
					redraw=true;
					break;
				
				
				case KEY_V:
					if(copy==-1)
					{
						if(type!=2)
						{
							flip^=2;
							go_tiles();
							
							if(type==0)
							{
								normalize(tile,tile2,rect_sel,flip);
								flip=0;
							}
						}
					}
					else
					{
						bool alt=(key[KEY_ALT] || key[KEY_ALTGR]);
						go_tiles();
						saved = !copy_tiles(tile,tile2,copy,copycnt,rect_sel,false);
					}
					
					redraw=true;
					break;
				
				case KEY_F:
					if(copy==-1)
					{
						break;
					}
					else
					{
						go_tiles();
						{
							saved = !copy_tiles_floodfill(tile,tile2,copy,copycnt,rect_sel,false);
						}
					}
					
					redraw=true;
					break;
			
				case KEY_DEL:
					delete_tiles(tile,tile2,rect_sel);
					redraw=true;
					break;
				
				case KEY_U:
				{
					if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
					{
						//Only toggle the first 2 bits!
						show_only_unused_tiles = (show_only_unused_tiles&~3) | (((show_only_unused_tiles&3)+1)%4);
					}
					else
					{
						comeback_tiles();
					}
					
					redraw=true;
				}
				break;
				
				case KEY_8:
				case KEY_8_PAD:
					hide_8bit_marker();
					break;
				
				case KEY_I: //insert tiles
					if(type==0)
					{
						bool warn = (rect_sel
							&& ((tile/20)!=(tile2/20))
							&& !(tile%20==0&&tile2%20==19));
						int32_t z=zc_min(tile,tile2);
						int32_t count = abs(tile-tile2) + 1;
						tile=z;
						tile2=NEWMAXTILES;
						copy = tile + count;
						copycnt = NEWMAXTILES-copy;
						
						if(key[KEY_LSHIFT]||key[KEY_RSHIFT]) //Remove
						{
							char buf[64];
							
							if(count>1)
								sprintf(buf,"Remove tiles %d - %d?",tile, copy-1);
							else
								sprintf(buf,"Remove tile %d?",tile);
								
							AlertDialog("Remove Tiles", std::string(buf)
								+"\nThis will offset the tiles that follow!"
								+(warn?"\nRemoving tiles ignores rectangular selections!":""),
								[&](bool ret,bool)
								{
									if(ret)
									{
										go_tiles();
										if(copy_tiles(tile,tile2,copy,copycnt,false,true))
										{
											redraw=true;
											saved=false;
										}
									}
								}).show();
						}
						else
						{
							char buf[64];
							
							if(count>1)
								sprintf(buf,"Insert %d blank tiles?",count);
							else
								sprintf(buf,"Insert a blank tile?");
								
							AlertDialog("Insert Tiles", std::string(buf)
								+"\nThis will offset the tiles that follow!"
								+(warn?"\nInserting tiles ignores rectangular selections!":""),
								[&](bool ret,bool)
								{
									if(ret)
									{
										go_tiles();
										if(copy_tiles(copy,tile2,tile,copycnt,false,true))
										{
											redraw=true;
											saved=false;
										}
									}
								}).show();
						}
						
						copy=-1;
						tile2=tile=z;
					}
					break;
				case KEY_M:
					if(type==0)
					{
						if((copy!=-1)&&(copy!=zc_min(tile,tile2)))
						{
							go_tiles();
							if(copy_tiles(tile,tile2,copy,copycnt,rect_sel,true))
								saved=false;
						}
						else if(copy==-1)
						{
							// I don't know what this was supposed to be doing before.
							// It didn't work in anything like a sensible way.
							if(rect_sel)
							{
								make_combos_rect(top, left, rows, columns, cs);
							}
							else
							{
								make_combos(zc_min(tile, tile2), zc_max(tile, tile2), cs);
							}
						}
						
						redraw=true;
					}
					break;
				
				case KEY_D:
				{
					int32_t frames=1;
					char buf[80];
					sprintf(buf, "%d", frames);
					create_relational_tiles_dlg[0].dp2=lfont;
					create_relational_tiles_dlg[2].dp=buf;
					
					large_dialog(create_relational_tiles_dlg);
						
					int32_t ret=zc_popup_dialog(create_relational_tiles_dlg,2);
					
					if(ret==5)
					{
						frames=zc_max(atoi(buf),1);
						bool same = true;
						int32_t bitcheck=newtilebuf[tile].format;
						
						for(int32_t t=1; t<frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?6:19); ++t)
						{
							if(newtilebuf[tile+t].format!=bitcheck) same = false;
						}
						
						if(!same)
						{
							jwin_alert("Error","The source tiles are not","in the same format.",NULL,"&OK",NULL,13,27,lfont);
							break;
						}
						
						if(tile+(frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?48:96))>NEWMAXTILES)
						{
							jwin_alert("Error","Too many tiles will be created",NULL,NULL,"&OK",NULL,13,27,lfont);
							break;
						}
						
						for(int32_t i=frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?6:19); i<(frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?48:96)); ++i)
						{
							reset_tile(newtilebuf, tile+i, bitcheck);
						}
						
						if(create_relational_tiles_dlg[3].flags&D_SELECTED)
						{
							for(int32_t i=create_relational_tiles_dlg[3].flags&D_SELECTED?47:95; i>0; --i)
							{
								for(int32_t j=0; j<frames; ++j)
								{
									merge_tiles(tile+(i*frames)+j, (tile+(relational_template[i][0]*frames)+j)<<2, ((tile+(relational_template[i][1]*frames)+j)<<2)+1, ((tile+(relational_template[i][2]*frames)+j)<<2)+2, ((tile+(relational_template[i][3]*frames)+j)<<2)+3);
								}
							}
						}
						else
						{
							for(int32_t i=create_relational_tiles_dlg[3].flags&D_SELECTED?47:95; i>0; --i)
							{
								for(int32_t j=0; j<frames; ++j)
								{
									merge_tiles(tile+(i*frames)+j, (tile+(dungeon_carving_template[i][0]*frames)+j)<<2, ((tile+(dungeon_carving_template[i][1]*frames)+j)<<2)+1, ((tile+(dungeon_carving_template[i][2]*frames)+j)<<2)+2, ((tile+(dungeon_carving_template[i][3]*frames)+j)<<2)+3);
								}
							}
						}
					}
					register_blank_tiles();
					register_used_tiles();
					redraw=true;
					saved=false;
					break;
				}
				
				case KEY_B:
				{
					bool shift=(key[KEY_LSHIFT] || key[KEY_RSHIFT]);
					bool control=(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]);
					bool alt=(key[KEY_ALT] || key[KEY_ALTGR]);
					
					do_convert_tile(tile, tile2, cs, rect_sel, control, shift, alt);
					register_blank_tiles();
				}
				break;
			}
			
			clear_keybuf();
		}
		
		if(!(key[KEY_Z] || key[KEY_F12]))
			did_snap = false;
		
		if(gui_mouse_b()&1)
		{
			if(isinRect(gui_mouse_x(),gui_mouse_y(),w + 12 - 21, 5, w +12 - 21 + 15, 5 + 13))
			{
				if(do_x_button(screen,w+12 - 21, 5))
				{
					done=1;
				}
			}
			
			int32_t x=gui_mouse_x()-screen_xofs;
			int32_t y=gui_mouse_y()-screen_yofs;
			
			if(y>=0 && y<208*mul)
			{
				x=zc_min(zc_max(x,0),(320*mul)-1);
				int32_t t = (y>>(5))*TILES_PER_ROW + (x>>(5)) + first;
				
				if(type==0 && (key[KEY_LSHIFT] || key[KEY_RSHIFT]))
				{
					tile2=t;
				}
				else
				{
					tile=tile2=t;
				}
				
				if(tile_clicked!=t)
				{
					dclick_status=DCLICK_NOT;
				}
				else if(dclick_status == DCLICK_AGAIN)
				{
					while(gui_mouse_b())
					{
						/* do nothing */
						rest(1);
					}
					
					if(((y>>(5))*TILES_PER_ROW + (x>>(5)) + first)!=t)
					{
						dclick_status=DCLICK_NOT;
					}
					else
					{
						if(type==0)
						{
							edit_tile(tile,flip,cs);
							draw_tile_list_window();
							redraw=true;
						}
						else
						{
							done=2;
						}
					}
				}
				
				tile_clicked=t;
			}
			else if(x>300*mul && !bdown)
			{
				if(y<224*mul && first>0)
				{
					first-=TILES_PER_PAGE;
					redraw=true;
				}
				
				if(y>=224*mul && first<TILES_PER_PAGE*(TILE_PAGES-1))
				{
					first+=TILES_PER_PAGE;
					redraw=true;
				}
				
				bdown=true;
			}
			
			ALLEGRO_FONT *tf = a5font;
			a5font = tfont_a5;
			if(type==1||type==2)
			{
				if(!bdown && isinRect(x,y,8*mul,216*mul+panel_yofs,23*mul,231*mul+panel_yofs))
					done=1;
					
				if(!bdown && isinRect(x,y,148*mul,216*mul+panel_yofs,163*mul,231*mul+panel_yofs))
					done=2;
			}
			else if(!bdown && isinRect(x,y,127*mul,216*mul+panel_yofs,(127+15)*mul,(216+15)*mul+panel_yofs))
			{
				rect_sel=!rect_sel;
				copy=-1;
				redraw=true;
			}
			else if(!bdown && isinRect(x,y,150*mul,213*mul+panel_yofs,(150+28)*mul,(213+21)*mul+panel_yofs))
			{
				if(do_text_button_a5(150*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"&Grab"))
				{
					a5font = tf;
					grab_tile(tile,cs);
					draw_tile_list_window();
					position_mouse_z(0);
					redraw=true;
				}
			}
			else if(!bdown && isinRect(x,y,(150+28)*mul,213*mul+panel_yofs,(150+28*2)*mul,(213+21)*mul+panel_yofs+21))
			{
				if(do_text_button_a5((150+28)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"&Edit"))
				{
					a5font = tf;
					edit_tile(tile,flip,cs);
					draw_tile_list_window();
					redraw=true;
				}
			}
			else if(!bdown && isinRect(x,y,(150+28*2)*mul,213*mul+panel_yofs,(150+28*3)*mul,(213+21)*mul+panel_yofs))
			{
				if(do_text_button_a5((150+28*2)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"Export"))
				{
					strcpy(datapath, "tileset.png");
					if(getname("Export Tile Page (.png)","png",NULL,datapath,true))
					{
						PALETTE temppal;
						get_palette(temppal);
						BITMAP *tempbmp=create_bitmap_ex(8,16*TILES_PER_ROW, 16*TILE_ROWS_PER_PAGE);
						draw_tiles(tempbmp,first,cs,f,false,true);
						save_bitmap(temppath, tempbmp, RAMpal);
						destroy_bitmap(tempbmp);
					}
				}
			}
			else if(!bdown && isinRect(x,y,(150+28*3)*mul,213*mul+panel_yofs,(150+28*4)*mul,(213+21)*mul+panel_yofs))
			{
				if(do_text_button_a5((150+28*3)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"Recolor"))
				{
					if(massRecolorSetup(cs))
					{
						go_tiles();
						
						FOREACH_START(t)
							massRecolorApply(t);
						FOREACH_END
						
						register_blank_tiles();
					}
				}
			}
			else if(!bdown && isinRect(x,y,(150+28*4)*mul,213*mul+panel_yofs,(150+28*5)*mul,(213+21)*mul+panel_yofs))
			{
				if(do_text_button_a5((150+28*4)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"Done"))
				{
					done=1;
				}
			}
			
			a5font = tf;
			bdown=true;
		}
		
		bool r_click = false;
		
		if(gui_mouse_b()&2 && !bdown && type==0)
		{
			int32_t x=(gui_mouse_x()-screen_xofs);//&0xFF0;
			int32_t y=(gui_mouse_y()-screen_yofs);//&0xF0;
			
			if(y>=0 && y<208*mul)
			{
				x=zc_min(zc_max(x,0),(320*mul)-1);
				int32_t t = ((y)>>(5))*TILES_PER_ROW + ((x)>>(5)) + first;
				
				if(t<zc_min(tile,tile2) || t>zc_max(tile,tile2))
					tile=tile2=t;
			}
			
			bdown = r_click = true;
			f=8;
		}
		
		if(gui_mouse_b()==0)
			bdown=false;
			
		position_mouse_z(0);
		
REDRAW:

		if((f%16)==0 || InvalidStatic)
			redraw=true;
			
		if(redraw)
		{
			draw_tiles(first,cs,f);
		}
		if(f&8)
		{
			if(rect_sel)
			{
				for(int32_t i=zc_min(TILEROW(tile),TILEROW(tile2))*TILES_PER_ROW+
						  zc_min(TILECOL(tile),TILECOL(tile2));
						i<=zc_max(TILEROW(tile),TILEROW(tile2))*TILES_PER_ROW+
						zc_max(TILECOL(tile),TILECOL(tile2)); i++)
				{
					if(i>=first && i<first+TILES_PER_PAGE &&
							TILECOL(i)>=zc_min(TILECOL(tile),TILECOL(tile2)) &&
							TILECOL(i)<=zc_max(TILECOL(tile),TILECOL(tile2)))
					{
						int32_t x=TILECOL(i)<<(5);
						int32_t y=TILEROW(i-first)<<(5);
						rect(screen2,x,y,x+(16*mul)-1,y+(16*mul)-1,vc(15));
					}
				}
			}
			else
			{
				for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
				{
					if(i>=first && i<first+TILES_PER_PAGE)
					{
						int32_t x=TILECOL(i)<<(5);
						int32_t y=TILEROW(i-first)<<(5);
						rect(screen2,x,y,x+(16*mul)-1,y+(16*mul)-1,vc(15));
					}
				}
			}
		}
		
		if(type==0)
			tile_info_0(tile,tile2,cs,copy,copycnt,first/TILES_PER_PAGE,rect_sel);
		else
			tile_info_1(otile,oflip,ocs,tile,flip,cs,copy,first/TILES_PER_PAGE, always_use_flip);
			
		if(type==2)
		{
			char cbuf[16];
			sprintf(cbuf, "E&xtend: %s",ex==2 ? "32x32" : ex==1 ? "32x16" : "16x16");
			gui_textout_ln(screen, lfont_l, (uint8_t *)cbuf, (235*mul)+screen_xofs, (212*mul)+screen_yofs+panel_yofs, jwin_pal[jcBOXFG],jwin_pal[jcBOX],0);
		}
		
		++f;
		
		if(r_click)
		{
			select_tile_rc_menu[1].flags = (copy==-1) ? D_DISABLED : 0;
			select_tile_rc_menu[2].flags = (copy==-1) ? D_DISABLED : 0;
			select_tile_view_menu[0].flags = HIDE_USED ? D_SELECTED : 0;
			select_tile_view_menu[1].flags = HIDE_UNUSED ? D_SELECTED : 0;
			select_tile_view_menu[2].flags = HIDE_BLANK ? D_SELECTED : 0;
			select_tile_view_menu[3].flags = HIDE_8BIT_MARKER ? D_SELECTED : 0;
			select_tile_rc_menu[7].flags = (type!=0) ? D_DISABLED : 0;
			int32_t m = popup_menu(select_tile_rc_menu,gui_mouse_x(),gui_mouse_y());
			redraw=true;
			
			switch(m)
			{
				case 0:
					copy=zc_min(tile,tile2);
					copycnt=abs(tile-tile2)+1;
					break;
					
				case 2:
				case 1:
				{
					bool b = copy_tiles(tile,tile2,copy,copycnt,rect_sel,(m==2));
					if(saved) saved = !b;
					break;
				}
				case 7:
				{
					bool b = scale_tiles(tile, tile2,cs);
					if(saved) saved = !b;
					break;
				}
					
				case 3:
					delete_tiles(tile,tile2,rect_sel);
					break;
					
				case 5:
					edit_tile(tile,flip,cs);
					draw_tile_list_window();
					break;
					
				case 8:
				{
					do_convert_tile(tile,tile2,cs,rect_sel,(newtilebuf[tile].format!=tf4Bit),false,false);
					break;
				}
				
				case 6:
					grab_tile(tile,cs);
					draw_tile_list_window();
					position_mouse_z(0);
					break;
					
				case 10:
					show_blank_tile(tile);
					break;
				
				case 13: //overlay
					overlay_tile(newtilebuf,tile,copy,cs,0);
					break;
				
				case 14: //h-flip
				{
					flip^=1;
					go_tiles();
					
					if(type==0)
					{
						normalize(tile,tile2,rect_sel,flip);
						flip=0;
					}
					
					redraw=true;   
					break;
				}
				
				case 15: //h-flip
				{
					if(copy==-1)
					{
						if(type!=2)
						{
							flip^=2;
							go_tiles();
							
							if(type==0)
							{
								normalize(tile,tile2,rect_sel,flip);
								flip=0;
							}
						}
					}
					else
					{
						go_tiles();
						saved=!copy_tiles(tile,tile2,copy,copycnt,rect_sel,false);
					}
					
					redraw=true;
					break;
				}
				
				
				case 16: //mass combo
				{
					if(type==0)
					{
						//al_trace("mass combo key pressed, copy == %d\n",copy);
						if((copy!=-1)&&(copy!=zc_min(tile,tile2)))
						{
							go_tiles();
							saved=!copy_tiles(tile,tile2,copy,copycnt,rect_sel,true);
						}
						else if(copy==-1)
						{
							// I don't know what this was supposed to be doing before.
							// It didn't work in anything like a sensible way.
							if(rect_sel)
							{
								make_combos_rect(top, left, rows, columns, cs);
							}
							else
							{
								make_combos(zc_min(tile, tile2), zc_max(tile, tile2), cs);
							}
						}
						
						redraw=true;
					}
				}
				break;
				
				case 17:
					if(type==0)
					{
						bool warn = (rect_sel
							&& ((tile/20)!=(tile2/20))
							&& !(tile%20==0&&tile2%20==19));
						int32_t z=zc_min(tile,tile2);
						int32_t count = abs(tile-tile2) + 1;
						tile=z;
						tile2=NEWMAXTILES;
						copy = tile + count;
						copycnt = NEWMAXTILES-copy;
						
						if(key[KEY_LSHIFT]||key[KEY_RSHIFT]) //Remove
						{
							char buf[64];
							
							if(count>1)
								sprintf(buf,"Remove tiles %d - %d?",tile, copy-1);
							else
								sprintf(buf,"Remove tile %d?",tile);
								
							AlertDialog("Remove Tiles", std::string(buf)
								+"\nThis will offset the tiles that follow!"
								+(warn?"\nRemoving tiles ignores rectangular selections!":""),
								[&](bool ret,bool)
								{
									if(ret)
									{
										go_tiles();
										if(copy_tiles(tile,tile2,copy,copycnt,false,true))
										{
											redraw=true;
											saved=false;
										}
									}
								}).show();
						}
						else
						{
							char buf[64];
							
							if(count>1)
								sprintf(buf,"Insert %d blank tiles?",count);
							else
								sprintf(buf,"Insert a blank tile?");
								
							AlertDialog("Insert Tiles", std::string(buf)
								+"\nThis will offset the tiles that follow!"
								+(warn?"\nInserting tiles ignores rectangular selections!":""),
								[&](bool ret,bool)
								{
									if(ret)
									{
										go_tiles();
										if(copy_tiles(copy,tile2,tile,copycnt,false,true))
										{
											redraw=true;
											saved=false;
										}
									}
								}).show();
						}
						
						copy=-1;
						tile2=tile=z;
					}
					break;
					
				default:
					redraw=false;
					break;
			}
			
			r_click = false;
			goto REDRAW;
		}
		update_hw_screen();
	}
	while(!done);
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	comeback();
	register_blank_tiles();
	register_used_tiles();
	setup_combo_animations();
	setup_combo_animations2();
	int32_t ret = done-1;
	if(ret)
	{
		_selected_tile = tile;
		_selected_tcset = cs;
	}
	
	popup_zqdialog_end();
	popup_zqdialog_end_a5();
	return ret;
}
int32_t select_tile_2(int32_t &tile,int32_t &flip,int32_t type,int32_t &cs,bool edit_cs,int32_t exnow, bool always_use_flip)
{
	if(_selected_tile > -1)
	{
		tile = _selected_tile;
		cs = _selected_tcset;
	}
	int32_t ret = select_tile(tile,flip,type,cs,edit_cs,exnow,always_use_flip);
	if(_selected_tile < 0)
	{
		_selected_tile = tile;
		_selected_tcset = cs;
	}
	return ret;
}

int32_t onTiles()
{
	static int32_t t=0;
	int32_t f=0;
	int32_t c=CSet;
	reset_pal_cycling();
//  loadlvlpal(Map.CurrScr()->color);
	rebuild_trans_table();
	select_tile(t,f,0,c,true);
	refresh(rALL);
	return D_O_K;
}

void draw_combo(BITMAP *dest, int32_t x,int32_t y,int32_t c,int32_t cs)
{
	if(c<MAXCOMBOS)
	{
		/*BITMAP *buf = create_bitmap_ex(8,16,16);
		put_combo(buf,0,0,c,cs,0,0);
		stretch_blit(buf,dest,0,0,16,16,x,y,32,32);
		destroy_bitmap(buf);*/
		put_combo(dest,x,y,c,cs,0,0);
	}
	else
	{
		rectfill(dest,x,y,x+32-1,y+32-1,0);
	}
}

void draw_combos(int32_t page,int32_t cs,bool cols)
{
	int32_t screen_xofs=6;
	int32_t screen_yofs=25;
	clear_bitmap(screen2);
	BITMAP *buf = create_bitmap_ex(8,16,16);
	
	int32_t w = 32;
	int32_t h = 32;
	int32_t mul = 2;
	
	if(cols==false)
	{
		for(int32_t i=0; i<256; i++)                                // 13 rows, leaving 32 pixels from y=208 to y=239
		{
			int32_t x = (i%COMBOS_PER_ROW)<<5;
			int32_t y = (i/COMBOS_PER_ROW)<<5;
			
			draw_combo(buf,0,0,i+(page<<8),cs);
			stretch_blit(buf,screen2,0,0,16,16,x,y,w,h);
		}
	}
	else
	{
		int32_t c = 0;
		
		for(int32_t i=0; i<256; i++)
		{
			int32_t x = (i%COMBOS_PER_ROW)<<5;
			int32_t y = (i/COMBOS_PER_ROW)<<5;
			draw_combo(buf,0,0,c+(page<<8),cs);
			stretch_blit(buf,screen2,0,0,16,16,x,y,w,h);
			++c;
			
			if((i&3)==3)
				c+=48;
				
			if((i%COMBOS_PER_ROW)==(COMBOS_PER_ROW-1))
				c-=256;
		}
	}
	
	for(int32_t x=(64*mul); x<(320*mul); x+=(64*mul))
	{
		_allegro_vline(screen2,x,0,(208*mul)-1,vc(15));
		_allegro_vline(screen2,x-1,0,(208*mul)-1,vc(15));
	}
	
	destroy_bitmap(buf);
}

void combo_info(int32_t tile,int32_t tile2,int32_t cs,int32_t copy,int32_t copycnt,int32_t page,int32_t buttons)
{
	int32_t a5x=6;
	int32_t a5y=25;
	int32_t yofs=3;
	BITMAP *buf = create_bitmap_ex(8,16,16);
	char cbuf[32];
	int32_t mul = 2;
	ALLEGRO_FONT *tfont = get_zc_font_a5(font_lfont_l);
	
	if(a4_bmp_active())
		rectfill(screen2,0,416,640-1,480,get_zqdialog_a4_clear_color());
	al_draw_filled_rectangle(a5x,a5y+416,a5x+640-1,a5y+480,jwin_a5_pal(jcBOX));
	al_draw_hline(a5x, a5y+(210*2)-2, a5x+(320*2)-1, jwin_a5_pal(jcMEDLT));
	al_draw_hline(a5x, a5y+(210*2)-1, a5x+(320*2)-1, jwin_a5_pal(jcLIGHT));
	
	jwin_draw_frame_a5(a5x+(31*mul)-2,a5y+((216*mul)+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	
	if(copy>=0)
	{
		put_combo(buf,0,0,copy,cs,0,0);
		stretch_blit(buf,screen2,0,0,16,16,31*mul,216*mul+yofs,16*mul,16*mul);
		
		if(copycnt>1)
		{
			sprintf(cbuf,"%d-",copy);
			jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+28*mul,a5y+(216*mul)+yofs,ALLEGRO_ALIGN_RIGHT,cbuf,jwin_a5_pal(jcBOX));
			sprintf(cbuf,"%d",copy+copycnt-1);
			jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+24*mul,a5y+(224*mul)+yofs,ALLEGRO_ALIGN_RIGHT,cbuf,jwin_a5_pal(jcBOX));
		}
		else
		{
			sprintf(cbuf,"%d",copy);
			jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+24*mul,a5y+(220*mul)+yofs,ALLEGRO_ALIGN_RIGHT,cbuf,jwin_a5_pal(jcBOX));
		}
	}
	else
	{
		al5_invalid(a5x+31*mul, a5y+(216*mul)+yofs, 16*mul);
	}
	
	jwin_draw_frame_a5(a5x+(53*mul)-2,a5y+(216*mul)+yofs-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	put_combo(buf,0,0,tile,cs,0,0);
	stretch_blit(buf,screen2,0,0,16,16,53*mul,216*mul+yofs,16*mul,16*mul);
	
	if(tile>tile2)
	{
		zc_swap(tile,tile2);
	}
	
	char tbuf[8];
	tbuf[0]=0;
	
	if(tile2!=tile)
	{
		sprintf(tbuf,"-%d",tile2);
	}
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+99*mul,a5y+216*mul+yofs,ALLEGRO_ALIGN_RIGHT,"Combo:",jwin_a5_pal(jcBOX));
	sprintf(cbuf, "%d%s", tile, tbuf);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+99*mul,a5y+224*mul+yofs,ALLEGRO_ALIGN_RIGHT,cbuf,jwin_a5_pal(jcBOX));
	
	if(tile2==tile)
	{
		int32_t nextcombo=combobuf[tile].nextcombo;
		int32_t nextcset=(combobuf[tile].animflags & AF_CYCLENOCSET) ? cs : combobuf[tile].nextcset;
		jwin_draw_frame_a5(a5x+(136*mul)-2,a5y+(216*mul)+yofs-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
		
		if(nextcombo>0)
		{
			put_combo(buf,0,0,nextcombo,nextcset,0,0);
			stretch_blit(buf,screen2,0,0,16,16,136*mul,216*mul+yofs,16*mul,16*mul);
		}
		else
		{
			al5_invalid(a5x+(136*mul), a5y+(216*mul)+yofs, 16*mul);
		}
		
		jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+132*mul,a5y+216*mul+yofs,ALLEGRO_ALIGN_RIGHT,"Cycle:",jwin_a5_pal(jcBOX));
		sprintf(cbuf,"%d",nextcombo);
		jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+132*mul,a5y+224*mul+yofs,ALLEGRO_ALIGN_RIGHT,cbuf,jwin_a5_pal(jcBOX));
	}
	
	ALLEGRO_FONT *tf = a5font;
	a5font = tfont;
	
	if(buttons&2)
	{
		jwin_draw_text_button_a5(a5x+((202)*mul),a5y+(213*mul)+yofs,(44*mul),(21*mul),"&Edit",0,false);
	}
	
	if(buttons&4)
	{
		jwin_draw_text_button_a5(a5x+((247)*mul),a5y+(213*mul)+yofs,(44*mul),(21*mul),"Done",0,false);
	}
	
	draw_arrow_a5(jwin_a5_pal(jcBOXFG),a5x+609,a5y+430+yofs,5,true,false);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+293*mul,a5y+220*mul+yofs,ALLEGRO_ALIGN_LEFT,"PG:",jwin_a5_pal(jcBOX));
	sprintf(cbuf, "%d",page);
	jwin_textout_a5(tfont,jwin_a5_pal(jcBOXFG),a5x+(305*mul+4),a5y+220*mul+yofs,ALLEGRO_ALIGN_CENTRE,cbuf,jwin_a5_pal(jcBOX));
	draw_arrow_a5(jwin_a5_pal(jcBOXFG),a5x+609,a5y+460+yofs,5,false,false);
	
	a5font = tf;
	
	scare_mouse();
	blit(screen2,screen,0,0,a5x,a5y,640,480);
	unscare_mouse();
	update_hw_screen();
	SCRFIX();
	destroy_bitmap(buf);
}

void sel_combo(int32_t &tile, int32_t &tile2, int32_t s, bool cols)
{
	int32_t page = tile&0xFF00;
	tile &= 0xFF;
	
	if(!cols)
		tile += s;
	else
	{
		if(s==-COMBOS_PER_ROW)
			tile-=4;
			
		if(s==COMBOS_PER_ROW)
			tile+=4;
			
		if(s==-1)
			tile-=1;
			
		if(s==1)
			tile+=1;
	}
	
	/*
	  if(s==1)
	  {
	  if((tile&3)==3)
	  tile+=48;
	  else
	  ++tile;
	  }
	  if(s==-1)
	  {
	  if((tile&3)==0)
	  tile-=48;
	  else
	  --tile;
	  }
	  }
	  */
	bound(tile,0,255);
	tile += page;
	
	if(!(key[KEY_LSHIFT] || key[KEY_RSHIFT]))
		tile2 = tile;
}

void draw_combo_list_window()
{
	int32_t w = 640;
	int32_t h = 480;
	
	if(a4_bmp_active())
		clear_to_color(screen, get_zqdialog_a4_clear_color()); //!TODO Remove when a5 dialog done - Clear a4 screen layer
	jwin_draw_win_a5(0, 0, w+6+6, h+25+6, FR_WIN);
	jwin_draw_frame_a5(4, 23, w+2+2, h+4+2-64, FR_DEEP);
	
	ALLEGRO_FONT *oldfont = a5font;
	a5font = get_custom_font_a5(CFONT_TITLE);
	jwin_draw_titlebar_a5(3, 3, w+6, 18, "Select Combo", true);
	a5font=oldfont;
}


static int32_t _selected_combo=-1, _selected_cset=-1;
bool select_combo_2(int32_t &cmb,int32_t &cs)
{
	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-31)>>1;
	popup_zqdialog_start_a5(window_xofs,window_yofs,w+12,h+31);
	popup_zqdialog_start(window_xofs,window_yofs,w+12,h+31,0xFF);
	reset_combo_animations();
	reset_combo_animations2();
	// static int32_t cmb=0;
	int32_t page=cmb>>8;
	int32_t tile2=cmb;
	int32_t done=0;
	int32_t tile_clicked=-1;
	int32_t t2;
	// int32_t cs = CSet;
	int32_t copy=-1;
	int32_t copycnt=0;
	
	position_mouse_z(0);
		
	go();
	int32_t screen_xofs=6;
	int32_t screen_yofs=25;
	int32_t panel_yofs=3;
	int32_t mul = 2;
	FONT *tfont = lfont_l;
	
	draw_combo_list_window();
	draw_combos(page,cs,combo_cols);
	combo_info(cmb,tile2,cs,copy,copycnt,page,4);
	unscare_mouse();
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	bool bdown=false;
	int32_t f=0;
	
	do
	{
		rest(4);
		bool redraw=false;
		
		if(mouse_z<0)
		{
			if(page<COMBO_PAGES-1)
			{
				++page;
				cmb=tile2=(page<<8)+(cmb&0xFF);
			}
			
			position_mouse_z(0);
			redraw=true;
		}
		else if(mouse_z>0)
		{
			if(page>0)
			{
				--page;
				cmb=tile2=(page<<8)+(cmb&0xFF);
			}
			
			position_mouse_z(0);
			redraw=true;
		}
		
		if(keypressed())
		{
			switch(readkey()>>8)
			{
			case KEY_DEL:
				cmb=0;
				done=2;
				break;
				
			case KEY_ENTER_PAD:
			case KEY_ENTER:
				done=2;
				break;
				
			case KEY_ESC:
				done=1;
				break;
				
			case KEY_F1:
				onHelp();
				break;
				
			case KEY_SPACE:
				combo_cols=!combo_cols;
				redraw=true;
				break;
				
			case KEY_EQUALS:
			case KEY_PLUS_PAD:
				cs = (cs<13) ? cs+1:0;
				redraw=true;
				break;
				
			case KEY_MINUS:
			case KEY_MINUS_PAD:
				cs = (cs>0) ? cs-1:13;
				redraw=true;
				break;
				
			case KEY_UP:
				sel_combo(cmb,tile2,-COMBOS_PER_ROW,combo_cols);
				redraw=true;
				break;
				
			case KEY_DOWN:
				sel_combo(cmb,tile2,COMBOS_PER_ROW,combo_cols);
				redraw=true;
				break;
				
			case KEY_LEFT:
				sel_combo(cmb,tile2,-1,combo_cols);
				redraw=true;
				break;
				
			case KEY_RIGHT:
				sel_combo(cmb,tile2,1,combo_cols);
				redraw=true;
				break;
				
			case KEY_PGUP:
				if(page>0)
				{
					--page;
					cmb=tile2=(page<<8)+(cmb&0xFF);
				}
				
				redraw=true;
				break;
				
			case KEY_PGDN:
				if(page<COMBO_PAGES-1)
				{
					++page;
					cmb=tile2=(page<<8)+(cmb&0xFF);
				}
				
				redraw=true;
				break;
				
			case KEY_P:
			{
				int32_t choosepage=getnumber("Goto Page", (PreFillComboEditorPage?page:0));
				
				if(!cancelgetnum)
					page=(zc_min(choosepage,COMBO_PAGES-1));
					
				cmb=tile2=(page<<8)+(cmb&0xFF);
				redraw=true;
				break;
			}
			}
			
			clear_keybuf();
		}
		
		if(gui_mouse_b()&1)
		{
			if(isinRect(gui_mouse_x(),gui_mouse_y(),w + 12 - 21, 5, w +12 - 21 + 15, 5 + 13))
			{
				if(do_x_button(screen, w+12 - 21, 5))
				{
					done=1;
				}
			}
			
			int32_t x=gui_mouse_x()-screen_xofs;
			int32_t y=gui_mouse_y()-screen_yofs;
			
			if(y>=0 && y<208*mul)
			{
				x=zc_min(zc_max(x,0),(320*mul)-1);
				int32_t t;
				
				if(!combo_cols)
				{
					t = (y>>5)*COMBOS_PER_ROW + (x>>5);
				}
				else
				{
					t = ((x>>7)*52) + ((x>>5)&3) + ((y>>5)<<2);
				}
				
				bound(t,0,255);
				t+=page<<8;
				cmb=tile2=t;
				
				if(tile_clicked!=t)
				{
					dclick_status=DCLICK_NOT;
				}
				else if(dclick_status == DCLICK_AGAIN)
				{
					while(gui_mouse_b())
					{
						/* do nothing */
					}
					
					if(!combo_cols)
					{
						t2 = (y>>5)*COMBOS_PER_ROW + (x>>5);
					}
					else
					{
						t2 = ((x>>7)*52) + ((x>>5)&3) + ((y>>5)<<2);
					}
					
					if(t2!=t)
					{
						dclick_status=DCLICK_NOT;
					}
					else
					{
						done=2;
					}
				}
				
				tile_clicked=t;
			}
			else if(y>=(208*mul) && x>(300*mul) && !bdown)
			{
				if(y<(224*mul)+panel_yofs && page>0)
				{
					--page;
					redraw=true;
				}
				
				if(y>=(224*mul)+panel_yofs && page<COMBO_PAGES-1)
				{
					++page;
					redraw=true;
				}
				
				bdown=true;
			}
			
			if(!bdown && isinRect(x,y,(247*mul),(213*mul),((247+44)*mul),((213+21)*mul)))
			{
				FONT *tf = font;
				font = tfont;
				
				if(do_text_button((247*mul)+screen_xofs,(213*mul)+screen_yofs+panel_yofs,(44*mul),(21*mul),"Done",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
				{
					done=2;
				}
				
				font = tf;
			}
			
			bdown=true;
		}
		
		bool r_click = false;
		
		if(gui_mouse_b()&2 && !bdown)
		{
			int32_t x=gui_mouse_x()+screen_xofs;
			int32_t y=gui_mouse_y()+screen_yofs;
			
			if(y>=0 && y<208*mul)
			{
				x=zc_min(zc_max(x,0),(320*mul)-1);
				int32_t t;
				
				if(!combo_cols)
					t = (y>>5)*COMBOS_PER_ROW + (x>>5);
				else
					t = ((x>>7)*52) + ((x>>5)&3) + ((y>>5)<<2);
					
				bound(t,0,255);
				t+=page<<8;
				
				if(t<zc_min(cmb,tile2) || t>zc_max(cmb,tile2))
					cmb=tile2=t;
			}
			
			bdown = r_click = true;
			f=8;
		}
		
		if(gui_mouse_b()==0)
			bdown=false;
			
		if(redraw)
			draw_combos(page,cs,combo_cols);
			
		combo_info(cmb,tile2,cs,copy,copycnt,page,4);
		
		if(f&8)
		{
			int32_t x,y;
			scare_mouse();
			
			for(int32_t i=zc_min(cmb,tile2); i<=zc_max(cmb,tile2); i++)
			{
				if((i>>8)==page)
				{
					int32_t t=i&255;
					
					if(!combo_cols)
					{
						x=(t%COMBOS_PER_ROW)<<5;
						y=(t/COMBOS_PER_ROW)<<5;
					}
					else
					{
						x=((t&3) + ((t/52)<<2)) << 5;
						y=((t%52)>>2) << 5;
					}
					
					rect(screen,x+screen_xofs,y+screen_yofs,x+screen_xofs+(16*mul)-1,y+screen_yofs+(16*mul)-1,vc(15));
				}
			}
			
			unscare_mouse();
			SCRFIX();
		}
		
		++f;
		
	}
	while(!done);
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	comeback();
	setup_combo_animations();
	setup_combo_animations2();
	
	bool ret = done==2;
	if(ret)
	{
		_selected_combo = cmb;
		_selected_cset = cs;
	}
	
	popup_zqdialog_end();
	popup_zqdialog_end_a5();
	return ret;
}

bool select_combo_3(int32_t &cmb,int32_t &cs)
{
	if(_selected_combo < 0)
	{
		_selected_combo = Combo;
		_selected_cset = CSet;
	}
	cmb = _selected_combo;
	cs = _selected_cset;
	return select_combo_2(cmb,cs);
}

static DIALOG advpaste_dlg[] =
{
	/* (dialog proc)     (x)   (y)    (w)   (h)    (fg)      (bg)     (key)    (flags)       (d1)           (d2)      (dp) */
	{ jwin_win_proc,         0,    0,   200,  161,   vc(14),   vc(1),       0,     D_EXIT,       0,             0, (void *) "Advanced Paste", NULL, NULL },
	{ jwin_button_proc,     27,   130,  61,   21,   vc(14),  vc(1),  'k',     D_EXIT,     0,             0, (void *) "O&K", NULL, NULL },
	{ jwin_button_proc,     112,  130,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
	
	{ jwin_check_proc,		 10,	   30,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Tile", NULL, NULL },
	{ jwin_check_proc,		 10,	   40,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Cset2", NULL, NULL },
	{ jwin_check_proc,		 10,	   50,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Walkability", NULL, NULL },
	{ jwin_check_proc,		 10,	   60,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Animation", NULL, NULL },
	{ jwin_check_proc,		 10,	   70,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Type", NULL, NULL },
	{ jwin_check_proc,		 10,	   80,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Inherent Flag", NULL, NULL },
	{ jwin_check_proc,		 10,	   90,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Attribytes", NULL, NULL },
	{ jwin_check_proc,		 10,	  100,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Attrishorts", NULL, NULL },
	{ jwin_check_proc,		 10,	  110,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Attributes", NULL, NULL },
	{ jwin_check_proc,		 10,	  120,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Flags", NULL, NULL },
	{ jwin_check_proc,		110,	   30,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Label", NULL, NULL },
	{ jwin_check_proc,		110,	   40,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Triggered By", NULL, NULL },
	{ jwin_check_proc,		110,	   50,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Script", NULL, NULL },
	{ jwin_check_proc,		110,	   60,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Effect", NULL, NULL },
	{ jwin_check_proc,		110,	   70,	33,		9,	vc(14),	 vc(1),	  0,		0,				1,			0,	(void*) "Lift", NULL, NULL },
	
	{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t advpaste(int32_t tile, int32_t tile2, int32_t copy)
{
	advpaste_dlg[0].dp2=lfont;
	
	large_dialog(advpaste_dlg);
	
	int32_t ret = zc_popup_dialog(advpaste_dlg,-1);
	
	if(ret!=1) return ret;
	
	// save original in case it's in paste range
	newcombo combo=combobuf[copy];
	
	for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); ++i)
	{
		if(advpaste_dlg[3].flags & D_SELECTED)   // tile
		{
			combobuf[i].tile=combo.tile;
			combobuf[i].o_tile=combo.o_tile;
			combobuf[i].flip=combo.flip;
			setup_combo_animations();
			setup_combo_animations2();
		}
		
		if(advpaste_dlg[4].flags & D_SELECTED)   // cset2
		{
			combobuf[i].csets=combo.csets;
		}
		
		if(advpaste_dlg[5].flags & D_SELECTED)   // walk
		{
			combobuf[i].walk=(combobuf[i].walk&0xF0) | (combo.walk&0x0F);
		}
		
		if(advpaste_dlg[6].flags & D_SELECTED)   // anim
		{
			combobuf[i].frames=combo.frames;
			combobuf[i].speed=combo.speed;
			combobuf[i].nextcombo=combo.nextcombo;
			combobuf[i].nextcset=combo.nextcset;
			combobuf[i].skipanim=combo.skipanim;
			combobuf[i].nexttimer=combo.nexttimer;
			combobuf[i].skipanimy=combo.skipanimy;
			combobuf[i].animflags=combo.animflags;
		}
		
		if(advpaste_dlg[7].flags & D_SELECTED)   // type
		{
			combobuf[i].type=combo.type;
		}
		
		if(advpaste_dlg[8].flags & D_SELECTED)   // flag
		{
			combobuf[i].flag=combo.flag;
		}
		
		if(advpaste_dlg[9].flags & D_SELECTED)   // attribytes
		{
			for(int32_t q = 0; q < 8; ++q)
				combobuf[i].attribytes[q] = combo.attribytes[q];
		}
		
		if(advpaste_dlg[10].flags & D_SELECTED)   // attribytes
		{
			for(int32_t q = 0; q < 8; ++q)
				combobuf[i].attrishorts[q] = combo.attrishorts[q];
		}
		
		if(advpaste_dlg[11].flags & D_SELECTED)   // attributes
		{
			for(int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; ++q)
				combobuf[i].attributes[q] = combo.attributes[q];
		}
		
		if(advpaste_dlg[12].flags & D_SELECTED)   // flags
		{
			combobuf[i].usrflags = combo.usrflags;
		}
		
		if(advpaste_dlg[13].flags & D_SELECTED)   // label
		{
			for(int32_t q = 0; q < 11; ++q)
				combobuf[i].label[q] = combo.label[q];
		}
		
		if(advpaste_dlg[14].flags & D_SELECTED)   // triggered by
		{
			for(int32_t q = 0; q < 3; ++q)
				combobuf[i].triggerflags[q] = combo.triggerflags[q];
			combobuf[i].triggerlevel = combo.triggerlevel;
			combobuf[i].triggerbtn = combo.triggerbtn;
			combobuf[i].triggeritem = combo.triggeritem;
			combobuf[i].trigtimer = combo.trigtimer;
			combobuf[i].trigsfx = combo.trigsfx;
			combobuf[i].trigchange = combo.trigchange;
			combobuf[i].trigprox = combo.trigprox;
			combobuf[i].trigctr = combo.trigctr;
			combobuf[i].trigctramnt = combo.trigctramnt;
			combobuf[i].triglbeam = combo.triglbeam;
			combobuf[i].trigcschange = combo.trigcschange;
			combobuf[i].spawnitem = combo.spawnitem;
			combobuf[i].spawnenemy = combo.spawnenemy;
			combobuf[i].exstate = combo.exstate;
			combobuf[i].spawnip = combo.spawnip;
			combobuf[i].trigcopycat = combo.trigcopycat;
			combobuf[i].trigcooldown = combo.trigcooldown;
		}
		
		if(advpaste_dlg[15].flags & D_SELECTED)   // script
		{
			combobuf[i].script = combo.script;
			for(int32_t q = 0; q < 2; ++q)
				combobuf[i].initd[q] = combo.initd[q];
		}
		
		if(advpaste_dlg[16].flags & D_SELECTED)   // effect
		{
			combobuf[i].walk=(combobuf[i].walk&0x0F) | (combo.walk&0xF0);
		}
		
		if(advpaste_dlg[17].flags & D_SELECTED)   // lift
		{
			combobuf[i].liftcmb = combo.liftcmb;
			combobuf[i].liftundercmb = combo.liftundercmb;
			combobuf[i].liftcs = combo.liftcs;
			combobuf[i].liftundercs = combo.liftundercs;
			combobuf[i].liftdmg = combo.liftdmg;
			combobuf[i].liftlvl = combo.liftlvl;
			combobuf[i].liftitm = combo.liftitm;
			combobuf[i].liftflags = combo.liftflags;
			combobuf[i].liftgfx = combo.liftgfx;
			combobuf[i].liftsprite = combo.liftsprite;
			combobuf[i].liftsfx = combo.liftsfx;
			combobuf[i].liftbreaksprite = combo.liftbreaksprite;
			combobuf[i].liftbreaksfx = combo.liftbreaksfx;
			combobuf[i].lifthei = combo.lifthei;
			combobuf[i].lifttime = combo.lifttime;
		}
	}
	
	return ret;
}

int32_t combo_screen(int32_t pg, int32_t tl)
{
	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-31)>>1;
	popup_zqdialog_start_a5(window_xofs,window_yofs,w+12,h+31);
	popup_zqdialog_start(window_xofs,window_yofs,w+12,h+31,0xFF);
	reset_combo_animations();
	reset_combo_animations2();
	static int32_t tile=0;
	static int32_t page=0;
	
	if(pg>-1)
		page = pg;
		
	if(tl>-1)
		tile = tl;
		
	int32_t tile2=tile;
	int32_t done=0;
	int32_t cs = CSet;
	int32_t copy=-1;
	int32_t copycnt=0;
	
	int32_t tile_clicked=-1;
	int32_t t2;
	
	bool masscopy;
	
	go();
	int32_t screen_xofs=6;
	int32_t screen_yofs=25;
	int32_t panel_yofs=3;
	int32_t mul = 2;
	FONT *tfont = lfont_l;
	
	draw_combo_list_window();
	draw_combos(page,cs,combo_cols);
	combo_info(tile,tile2,cs,copy,copycnt,page,6);
	unscare_mouse();
	go_combos();
	position_mouse_z(0);
	
	while(gui_mouse_b())
	{
		/* do nothing */
	}
	
	bool bdown=false;
	int32_t f=0;
	
	do
	{
		rest(4);
		bool redraw=false;
		
		if(mouse_z<0)
		{
			if(page<COMBO_PAGES-1)
			{
				++page;
				tile=tile2=(page<<8)+(tile&0xFF);
			}
			
			position_mouse_z(0);
			redraw=true;
		}
		else if(mouse_z>0)
		{
			if(page>0)
			{
				--page;
				tile=tile2=(page<<8)+(tile&0xFF);
			}
			
			position_mouse_z(0);
			redraw=true;
		}
		
		if(keypressed())
		{
			switch(readkey()>>8)
			{
			case KEY_ENTER_PAD:
			case KEY_ENTER:
				done=2;
				break;
				
			case KEY_ESC:
				done=1;
				break;
				
			case KEY_F1:
				onHelp();
				break;
				
			case KEY_SPACE:
				combo_cols=!combo_cols;
				redraw=true;
				break;
				
			case KEY_EQUALS:
			case KEY_PLUS_PAD:
				if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
				{
					int32_t amnt = (key[KEY_LSHIFT] || key[KEY_RSHIFT]) ?
							   ((key[KEY_ALT] || key[KEY_ALTGR]) ? TILES_PER_PAGE*10 : TILES_PER_ROW)
							   : ((key[KEY_ALT] || key[KEY_ALTGR]) ? TILES_PER_PAGE : 1);
					
					for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); ++i)
					{
						combobuf[i].set_tile(wrap(combobuf[i].tile + amnt,
												0, NEWMAXTILES-1));
					}
					
					setup_combo_animations();
					redraw=true;
				}
				else
				{
					cs = (cs<13)  ? cs+1:0;
					redraw=true;
				}
				
				break;
				
			case KEY_MINUS:
			case KEY_MINUS_PAD:
				if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
				{
					int32_t amnt = (key[KEY_LSHIFT] || key[KEY_RSHIFT]) ?
							   ((key[KEY_ALT] || key[KEY_ALTGR]) ? TILES_PER_PAGE*10 : TILES_PER_ROW)
							   : ((key[KEY_ALT] || key[KEY_ALTGR]) ? TILES_PER_PAGE : 1);
					
					for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); ++i)
					{
						combobuf[i].set_tile(wrap(combobuf[i].tile - amnt,
												0, NEWMAXTILES-1));
					}
					
					setup_combo_animations();
					redraw=true;
				}
				else
				{
					cs = (cs>0) ? cs-1:13;
					redraw=true;
				}
				
				break;
				
			case KEY_UP:
				sel_combo(tile,tile2,-COMBOS_PER_ROW,combo_cols);
				redraw=true;
				break;
				
			case KEY_DOWN:
				sel_combo(tile,tile2,COMBOS_PER_ROW,combo_cols);
				redraw=true;
				break;
				
			case KEY_LEFT:
				sel_combo(tile,tile2,-1,combo_cols);
				redraw=true;
				break;
				
			case KEY_RIGHT:
				sel_combo(tile,tile2,1,combo_cols);
				redraw=true;
				break;
				
			case KEY_PGUP:
				if(page>0)
				{
					--page;
					tile=tile2=(page<<8)+(tile&0xFF);
				}
				
				redraw=true;
				break;
				
			case KEY_PGDN:
				if(page<COMBO_PAGES-1)
				{
					++page;
					tile=tile2=(page<<8)+(tile&0xFF);
				}
				
				redraw=true;
				break;
				
			case KEY_A:
			{
				tile=(page<<8);
				tile2=(page<<8)+(0xFF);
			}
			
			redraw=true;
			break;
			
			case KEY_P:
			{
				int32_t choosepage = getnumber("Goto Page", (PreFillComboEditorPage?page:0));
				
				if(!cancelgetnum)
					page=(zc_min(choosepage,COMBO_PAGES-1));
					
				tile=tile2=(page<<8)+(tile&0xFF);
				redraw=true;
			}
			break;
			
			case KEY_U:
				comeback_combos();
				redraw=true;
				break;
				
			case KEY_E:
				go_combos();
				edit_combo(tile,false,cs);
				redraw=true;
				setup_combo_animations();
				setup_combo_animations2();
				break;
				
			case KEY_C:
				go_combos();
				copy=zc_min(tile,tile2);
				copycnt=abs(tile-tile2)+1;
				redraw=true;
				break;
				
			case KEY_H:
				for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
				{
					combobuf[i].flip^=1;
					byte w2=combobuf[i].walk;
					combobuf[i].walk=((w2& ~0x33)>>2 | (w2&0x33)<<2);
					w2=combobuf[i].csets;
					combobuf[i].csets= (((w2& ~0x50)>>1 | (w2&0x50)<<1) & ~0x0F) | (w2 & 0x0F);
				}
				
				redraw=true;
				saved=false;
				break;
				
			case KEY_M:
				if((copy!=-1)&&(copy!=zc_min(tile,tile2)))
				{
					move_combos(tile,tile2,copy,copycnt);
					saved=false;
				}
				
				redraw=true;
				break;
				
			case KEY_S:
				tile=tile2=zc_min(tile,tile2);
				
				if(copy>=0 && tile!=copy)
				{
					go_combos();
					
					for(int32_t i=0; i<copycnt; i++)
					{
						zc_swap(combobuf[copy+i],combobuf[tile+i]);
					}
					
					saved=false;
					setup_combo_animations();
					setup_combo_animations2();
				}
				
				redraw=true;
				copy=-1;
				break;
				
			case KEY_V:
				if((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]) && copy != -1)
				{
					if(advpaste(tile, tile2, copy)==1)
					{
						saved=false;
						redraw=true;
						copy=-1;
					}
					
					break;
				}
				
				masscopy=(key[KEY_LSHIFT] || key[KEY_RSHIFT])?1:0;
				
				if(copy==-1)
				{
					go_combos();
					
					for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
					{
						combobuf[i].flip^=2;
						byte w2=combobuf[i].walk;
						combobuf[i].walk=(w2&0x55)<<1 | (w2& ~0x55)>>1;
						w2=combobuf[i].csets;
						combobuf[i].csets= (((w2&0x30)<<2 | (w2& ~0x30)>>2) & ~0x0F) | (w2 & 0x0F);
					}
					
					saved=false;
				}
				else
				{
					go_combos();
					copy_combos(tile,tile2,copy,copycnt,masscopy);
					setup_combo_animations();
					setup_combo_animations2();
					saved=false;
				}
				
				redraw=true;
				break;
			case KEY_R:
				for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
				{
					combobuf[i].flip = rotate_value(combobuf[i].flip);
					combobuf[i].walk = rotate_walk(combobuf[i].walk);
					combobuf[i].csets = rotate_cset(combobuf[i].csets);
				}
				
				redraw=true;
				saved=false;
				break;
				
			case KEY_I:
			{
				// rev.1509; Can now insert/remove all selected combos
				int32_t z=tile;
				int32_t numSelected = abs(tile-tile2) + 1;
				tile=zc_min(tile,tile2);
				tile2=MAXCOMBOS;
				copy = tile + numSelected; // copy=tile+1;
				copycnt = MAXCOMBOS-tile-numSelected; // copycnt=MAXCOMBOS-tile;
				
				if(key[KEY_LSHIFT]||key[KEY_RSHIFT])
				{
					char buf[64];
					
					if(numSelected>1)
						sprintf(buf,"Remove combos %d - %d?",tile, copy-1);
					else
						sprintf(buf,"Remove combo %d?",tile);
						
					if(jwin_alert("Confirm Remove",buf,"This will offset all of the combos that follow!",NULL,"&Yes","&No",'y','n',lfont)==1)
					{
						move_combos(tile,tile2,copy, copycnt);
						//don't allow the user to undo; quest combo references are incorrect -DD
						go_combos();
						redraw=true;
						saved=false;
					}
				}
				else
				{
					char buf[64];
					
					if(numSelected>1)
						sprintf(buf,"Insert %d blank combos?",numSelected);
					else
						sprintf(buf,"Insert a blank combo?");
						
					if(jwin_alert("Confirm Insert",buf,"This will offset all of the combos that follow!",NULL,"&Yes","&No",'y','n',lfont)==1)
					{
						move_combos(copy,tile2,tile, copycnt);
						go_combos();
						redraw=true;
						saved=false;
					}
				}
				
				copy=-1;
				tile2=tile=z;
			}
			break;
			
			case KEY_DEL:
			{
				char buf[40];
				
				if(tile==tile2)
				{
					sprintf(buf,"Delete combo %d?",tile);
				}
				else
				{
					sprintf(buf,"Delete combos %d-%d?",zc_min(tile,tile2),zc_max(tile,tile2));
				}
				
				if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
				{
					go_combos();
					
					for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
					{
						clear_combo(i);
					}
					
					tile=tile2=zc_min(tile,tile2);
					redraw=true;
					saved=false;
					setup_combo_animations();
					setup_combo_animations2();
				}
			}
			break;
			}
			
			clear_keybuf();
		}
		
		if(gui_mouse_b()&1)
		{
			if(isinRect(gui_mouse_x(),gui_mouse_y(),w + 12 - 21, 5, w +12 - 21 + 15, 5 + 13))
			{
				if(do_x_button(screen, w+12 - 21, 5))
				{
					done=1;
				}
			}
			
			int32_t x=gui_mouse_x()-screen_xofs;
			int32_t y=gui_mouse_y()-screen_yofs;
			
			if(y>=0 && y<(208*mul))
			{
				x=zc_min(zc_max(x,0),(320*mul)-1);
				int32_t t;
				
				if(!combo_cols)
				{
					t = (y>>5)*COMBOS_PER_ROW + (x>>5);
				}
				else
				{
					t = ((x>>7)*52) + ((x>>5)&3) + ((y>>5)<<2);
				}
				
				bound(t,0,255);
				t+=page<<8;
				
				if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
				{
					tile2=t;
				}
				else
				{
					tile=tile2=t;
				}
				
				if(tile_clicked!=t)
				{
					dclick_status=DCLICK_NOT;
				}
				else if(dclick_status == DCLICK_AGAIN)
				{
					while(gui_mouse_b())
					{
						/* do nothing */
						rest(1);
					}
					
					if(!combo_cols)
					{
						t2 = (y>>4)*COMBOS_PER_ROW + (x>>4);
					}
					else
					{
						t2 = ((x>>6)*52) + ((x>>4)&3) + ((y>>4)<<2);
					}
					
					bound(t2,0,255);
					t2+=page<<8;
					
					if(t2!=t)
					{
						dclick_status=DCLICK_NOT;
					}
					else
					{
						go_combos();
						edit_combo(tile,false,cs);
						redraw=true;
						setup_combo_animations();
						setup_combo_animations2();
					}
				}
				
				tile_clicked=t;
			}
			else if(x>(300*mul) && !bdown)
			{
				if(y<(224*mul)+panel_yofs && page>0)
				{
					--page;
					redraw=true;
				}
				
				if(y>=(224*mul)+panel_yofs && page<COMBO_PAGES-1)
				{
					++page;
					redraw=true;
				}
				
				bdown=true;
			}
			
			if(!bdown && isinRect(x,y,(202*mul),(213*mul)+panel_yofs,(202+44)*mul,(213+21)*mul))
			{
				FONT *tf = font;
				font = tfont;
				
				if(do_text_button((202*mul)+screen_xofs,(213*mul)+screen_yofs+panel_yofs,(44*mul),(21*mul),"Edit",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
				{
					font = tf;
					edit_combo(tile,false,cs);
					redraw=true;
				}
				
				font = tf;
			}
			else if(!bdown && isinRect(x,y,(247*mul),(213*mul)+panel_yofs,(247+44)*mul,(213+21)*mul))
			{
				FONT *tf = font;
				font = tfont;
				
				if(do_text_button((247*mul)+screen_xofs,(213*mul)+screen_yofs+panel_yofs,(44*mul),(21*mul),"Done",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
				{
					done=1;
				}
				
				font = tf;
			}
			
			bdown=true;
		}
		
		bool r_click = false;
		
		if(gui_mouse_b()&2 && !bdown)
		{
			int32_t x=gui_mouse_x()-screen_xofs;
			int32_t y=gui_mouse_y()-screen_yofs;
			
			if(y>=0 && y<(208*mul))
			{
				x=zc_min(zc_max(x,0),(320*mul)-1);
				int32_t t;
				
				if(!combo_cols)
				{
					t = (y>>5)*COMBOS_PER_ROW + (x>>5);
				}
				else
				{
					t = ((x>>7)*52) + ((x>>5)&3) + ((y>>5)<<2);
				}
				
				bound(t,0,255);
				t+=page<<8;
				
				if(t<zc_min(tile,tile2) || t>zc_max(tile,tile2))
				{
					tile=tile2=t;
				}
			}
			
			bdown = r_click = true;
			f=8;
		}
		
REDRAW:

		if(gui_mouse_b()==0)
		{
			bdown=false;
		}
		
		if(redraw)
		{
			draw_combos(page,cs,combo_cols);
		}
		
		combo_info(tile,tile2,cs,copy,copycnt,page,6);
		
		if(f&8)
		{
			int32_t x,y;
			scare_mouse();
			
			for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
			{
				if((i>>8)==page)
				{
					int32_t t=i&255;
					
					if(!combo_cols)
					{
						x=(t%COMBOS_PER_ROW)<<5;
						y=(t/COMBOS_PER_ROW)<<5;
					}
					else
					{
						x=((t&3) + ((t/52)<<2)) << 5;
						y=((t%52)>>2) << 5;
					}
					
					rect(screen,x+screen_xofs,y+screen_yofs,x+screen_xofs+(16*mul)-1,y+screen_yofs+(16*mul)-1,vc(15));
				}
			}
			
			unscare_mouse();
			SCRFIX();
		}
		
		++f;
		
		//Seriously? There is duplicate code for the r-click menu? -Gleeok
		if(r_click)
		{
			int32_t m = popup_menu(select_combo_rc_menu,gui_mouse_x(),gui_mouse_y());
			redraw=true;
			
			switch(m)
			{
			case 0:
				go_combos();
				copy=zc_min(tile,tile2);
				copycnt=abs(tile-tile2)+1;
				break;
				
			case 1:
				if((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]) && copy != -1)
				{
					if(advpaste(tile, tile2, copy)==1)
					{
						saved=false;
						redraw=true;
						copy=-1;
					}
					
					break;
				}
				
				masscopy=(key[KEY_LSHIFT] || key[KEY_RSHIFT])?1:0;
				
				if(copy==-1)
				{
					go_combos();
					
					for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
					{
						combobuf[i].flip^=2;
						byte w2=combobuf[i].walk;
						combobuf[i].walk=(w2&5)<<1 | (w2& ~5)>>1;
						w2=combobuf[i].csets;
						combobuf[i].csets=(w2&0x30)<<2 | (w2& ~0x30)>>2;
					}
					
					saved=false;
				}
				else
				{
					go_combos();
					copy_combos(tile,tile2,copy,copycnt,masscopy);
					setup_combo_animations();
					setup_combo_animations2();
					saved=false;
				}
				
				redraw=true;
				break;
				
			case 2:
			{
				tile=tile2=zc_min(tile,tile2);
				
				if(copy>=0 && tile!=copy)
				{
					go_combos();
					
					for(int32_t i=0; i<copycnt; i++)
					{
						zc_swap(combobuf[copy+i],combobuf[tile+i]);
					}
					
					saved=false;
					setup_combo_animations();
					setup_combo_animations2();
				}
				
				redraw=true;
				copy=-1;
			}
			break;
			
			case 3:
			{
				char buf[40];
				
				if(tile==tile2)
					sprintf(buf,"Delete combo %d?",tile);
				else
					sprintf(buf,"Delete combos %d-%d?",zc_min(tile,tile2),zc_max(tile,tile2));
					
				if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',lfont)==1)
				{
					go_combos();
					
					for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
						clear_combo(i);
						
					tile=tile2=zc_min(tile,tile2);
					redraw=true;
					saved=false;
				}
			}
			break;
			
			case 5:
				go_combos();
				edit_combo(tile,false,cs);
				break;
				
			case 6:
			case 7:
			{
				int32_t z=tile;
				tile=zc_min(tile,tile2);
				tile2=MAXCOMBOS;
				copy=tile+1;
				copycnt=MAXCOMBOS-tile;
				
				if(m==7)
				{
					char buf[40];
					sprintf(buf,"Remove combo %d?",tile);
					
					if(jwin_alert("Confirm Remove",buf,"This will offset all of the combos that follow!",NULL,"&Yes","&No",'y','n',lfont)==1)
					{
						move_combos(tile,tile2,copy, copycnt);
					}
					else break;
				}
				else
				{
					move_combos(copy,tile2,tile, copycnt);
				}
				
				copy=-1;
				tile2=tile=z;
				
				//don't allow the user to undo; quest combo references are incorrect -DD
				go_combos();
				
				redraw=true;
				saved=false;
			}
			break;
			
			case 9:
			{
				int32_t z = Combo;
				Combo = tile;
				onComboLocationReport();
				Combo = z;
			}
			break;
			
			default:
				redraw=false;
				break;
			}
			
			r_click = false;
			goto REDRAW;
		}
		
	}
	while(!done);
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	comeback();
	setup_combo_animations();
	setup_combo_animations2();
	_selected_combo = tile;
	_selected_cset = cs;
	popup_zqdialog_end();
	popup_zqdialog_end_a5();
	return done-1;
}

int32_t onCombos()
{
	// reset_combo_animations();
	combo_screen(-1,-1);
	// setup_combo_animations();
	refresh(rALL);
	return D_O_K;
}

int32_t d_ctile_proc(int32_t msg,DIALOG *d,int32_t c)
{
	//these are here to bypass compiler warnings about unused arguments
	d=d;
	c=c;
	
	if(msg==MSG_CLICK)
	{
		int32_t t=curr_combo.o_tile;
		int32_t f=curr_combo.flip;
		
		if(select_tile(t,f,1,CSet,true,0,true))
		{
			curr_combo.tile=t;
			curr_combo.o_tile=t;
			curr_combo.flip=f;
			return D_REDRAW;
		}
	}
	
	return D_O_K;
}

int32_t d_combo_loader(int32_t msg,DIALOG *d,int32_t c)
{
	//these are here to bypass compiler warnings about unused arguments
	c=c;
	
	if(msg==MSG_DRAW)
	{
		FONT *f = lfont_l;
		textprintf_ex(screen,f,d->x,d->y,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Tile:");
		textprintf_ex(screen,f,d->x+((1.5)*36),d->y,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",curr_combo.o_tile);
		textprintf_ex(screen,f,d->x,d->y+(14),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Flip:");
		textprintf_ex(screen,f,d->x+((1.5)*36),d->y+(14),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",curr_combo.flip);
		textprintf_ex(screen,f,d->x,d->y+(36),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet2:");
	}
	
	return D_O_K;
}

int32_t click_d_ctile_proc()
{
	d_ctile_proc(MSG_CLICK,NULL,0);
	return D_REDRAW;
}

int32_t click_d_combo_proc();

const char *comboscriptdroplist(int32_t index, int32_t *list_size)
{
	if(index<0)
	{
		*list_size = bidcomboscripts_cnt;
		return NULL;
	}
	
	return bidcomboscripts[index].first.c_str();
}
ListData comboscript_list(comboscriptdroplist, &font);

bool call_combo_editor(int32_t);
bool edit_combo(int32_t c,bool freshen,int32_t cs)
{
	FONT* ofont = font;
	//CSet = cs;
	reset_combo_animations();
	reset_combo_animations2();
	bool edited = call_combo_editor(c);
	font = ofont;
	
	if(freshen)
	{
		refresh(rALL);
	}
	
	setup_combo_animations();
	setup_combo_animations2();
	
	return edited;
}

int32_t d_itile_proc(int32_t msg,DIALOG *d,int32_t)
{
	switch(msg)
	{
	case MSG_CLICK:
	{
		int32_t cs = d->d2;
		int32_t f  = 0;
		
		if(select_tile(d->d1,f,1,cs,true))
		{
			int32_t ok=1;
			
			if(newtilebuf[d->d1].format==tf8Bit)
				jwin_alert("Warning",
						   "You have selected an 8-bit tile.",
						   "It will not be drawn correctly",
						   "on the file select screen.",
						   "&OK",NULL,'o',0,lfont);
						   
			return D_REDRAW;
		}
	}
	break;
	
	case MSG_DRAW:
		d->w = 32+4;
		d->h = 32+4;
		
		BITMAP *buf = create_bitmap_ex(8,16,16);
		BITMAP *bigbmp = create_bitmap_ex(8,d->w,d->h);
		
		if(buf && bigbmp)
		{
			clear_bitmap(buf);
			overtile16(buf,d->d1,0,0,d->fg,0);
			stretch_blit(buf, bigbmp, 0,0, 16, 16, 2, 2, d->w-4, d->h-4);
			destroy_bitmap(buf);
			jwin_draw_frame(bigbmp,0, 0, d->w,d->h, FR_DEEP);
			blit(bigbmp,screen,0,0,d->x-1,d->y-1,d->w,d->h);
			destroy_bitmap(bigbmp);
		}
		
		break;
	}
	
	return D_O_K;
}

static DIALOG icon_dlg[] =
{
	/* (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp) */
	{ jwin_win_proc,       70,   70,   170,  104,   vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Game Icons", NULL, NULL },
	{ d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
	{ d_itile_proc,      108+3,  112,  20,   20,   0,       0,      0,       0,          0,             6,       NULL, NULL, NULL },
	{ d_itile_proc,      138+3,  112,  20,   20,   0,       0,      0,       0,          0,             7,       NULL, NULL, NULL },
	{ d_itile_proc,      168+3,  112,  20,   20,   0,       0,      0,       0,          0,             8,       NULL, NULL, NULL },
	{ d_itile_proc,      198+3,  112,  20,   20,   0,       0,      0,       0,          0,             9,       NULL, NULL, NULL },
	{ jwin_button_proc,     90,   145,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
	{ jwin_button_proc,     170,  145,  61,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
	{ jwin_text_proc,     108+11,    98,    8,      9,    0,          0,           0,    0,          0,    0, (void *) "0",        NULL,   NULL                },
	{ jwin_text_proc,     138+11,    98,    8,      9,    0,          0,           0,    0,          0,    0, (void *) "1",        NULL,   NULL                },
	{ jwin_text_proc,     168+11,    98,    8,      9,    0,          0,           0,    0,          0,    0, (void *) "2",        NULL,   NULL                },
	{ jwin_text_proc,     198+11,    98,    8,      9,    0,          0,           0,    0,          0,    0, (void *) "3+",        NULL,   NULL                },
	{ jwin_text_proc,     88,      98,    12,      9,    0,          0,           0,    0,          0,    0, (void *) "Ring:",        NULL,   NULL                },
	{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

int32_t onIcons()
{
	PALETTE pal;
	//  pal = RAMpal;
	memcpy(pal,RAMpal,sizeof(RAMpal));
	icon_dlg[0].dp2=lfont;
	
	for(int32_t i=0; i<4; i++)
	{
		icon_dlg[i+2].d1 = misc.icons[i];
		icon_dlg[i+2].fg = i+6;
		load_cset(pal, i+6, pSprite(i+spICON1));
	}
	
	set_palette(pal);
	
	large_dialog(icon_dlg);
		
	int32_t ret = zc_popup_dialog(icon_dlg,7);
	
	if(ret==6)
	{
		for(int32_t i=0; i<4; i++)
		{
			if(misc.icons[i] != icon_dlg[i+2].d1)
			{
				misc.icons[i] = icon_dlg[i+2].d1;
				saved=false;
			}
		}
	}
	
	set_palette(RAMpal);
	return D_O_K;
}

// Identical to jwin_frame_proc, but is treated differently by large_dialog()
int32_t d_comboframe_proc(int32_t msg, DIALOG *d, int32_t c)
{
	//these are here to bypass compiler warnings about unused arguments
	c=c;
	
	if(msg == MSG_DRAW)
	{
		jwin_draw_frame(screen, d->x, d->y, d->w, d->h, d->d1);
	}
	
	return D_O_K;
}

int32_t d_combo_proc(int32_t msg,DIALOG *d,int32_t c)
{
	//these are here to bypass compiler warnings about unused arguments
	c=c;
	
	switch(msg)
	{
	case MSG_CLICK:
	{
		if(d->flags&D_NOCLICK)
		{
			break;
		}
		
		int32_t ret = (d->flags & D_EXIT) ? D_CLOSE : D_O_K;
		int32_t combo2;
		int32_t cs;
		
		if(key[KEY_LSHIFT])
		{
			if(gui_mouse_b()&1)
			{
				d->d1++;
				
				if(d->d1>=MAXCOMBOS) d->d1=0;
			}
			else if(gui_mouse_b()&2)
			{
				d->d1--;
				
				if(d->d1<0) d->d1=MAXCOMBOS-1;
			}
			
			return ret|D_REDRAW;
		}
		else if(key[KEY_RSHIFT])
		{
			if(gui_mouse_b()&1)
			{
				d->fg++;
				
				if(d->fg>11) d->fg=0;
			}
			else if(gui_mouse_b()&2)
			{
				d->fg--;
				
				if(d->fg<0) d->fg=11;
			}
			
			return ret|D_REDRAW;
		}
		else if(key[KEY_ALT])
		{
			if(gui_mouse_b()&1)
			{
				d->d1 = Combo;
				d->fg = CSet;
			}
			
			return ret|D_REDRAW;
		}
		else if(gui_mouse_b()&2||nextcombo_fake_click==2)  //right mouse button
		{
			if(d->d1==0&&d->fg==0&&!(gui_mouse_b()&1))
			{
				return ret;
			}
			
			d->d1=0;
			d->fg=0;
			return ret|D_REDRAW;
		}
		else if(gui_mouse_b()&1||nextcombo_fake_click==1)  //left mouse button
		{
			combo2=d->d1;
			cs=d->fg;
			
			if(select_combo_2(combo2, cs))
			{
				d->d1=combo2;
				d->fg=cs;
			}
			
			return ret|D_REDRAW;
		}
		else
		{
			return ret|D_REDRAWME;
		}
	}
	
	break;
	
	case MSG_DRAW:
		d->w = 32;
		d->h = 32;
		
		BITMAP *buf = create_bitmap_ex(8,16,16);
		BITMAP *bigbmp = create_bitmap_ex(8,d->w,d->h);
		
		if(buf && bigbmp)
		{
			clear_bitmap(buf);
			
			if(d->d1==-1) // Display curr_combo instead of combobuf
			{
				newcombo *hold = combobuf;
				combobuf = &curr_combo;
				putcombo(buf,0,0,0,d->fg);
				combobuf = hold;
			}
			else if(d->d1)
			{
				putcombo(buf,0,0,d->d1,d->fg);
			}
			
			stretch_blit(buf, bigbmp, 0,0, 16, 16, 0, 0, d->w, d->h);
			destroy_bitmap(buf);
			blit(bigbmp,screen,0,0,d->x-1,d->y-1,d->w,d->h);
			destroy_bitmap(bigbmp);
		}
	}
	
	return D_O_K;
}

void center_zq_tiles_dialog()
{
	jwin_center_dialog(advpaste_dlg);
}


// Hey, let's have a few hundred more lines of code, why not.

#define MR_4BIT 0
#define MR_8BIT 1

static byte massRecolorSrc4Bit[16]={ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
static byte massRecolorDest4Bit[16]={ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
static word massRecolor8BitCSets=0; // Which CSets are affected? One bit each.

static byte massRecolorSrc8Bit[16]={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static byte massRecolorDest8Bit[16]={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static int32_t massRecolorDraggedColor=-1;
static int32_t massRecolorCSet;
static bool massRecolorIgnoreBlank=true;
static byte massRecolorType=MR_4BIT;

// Shows the sets of colors to replace from/to.
// D_CSET: Colors are 0-15 within the current CSet rather than absolute.
// D_SETTABLE: Colors can be dragged and dropped onto this one.
#define D_CSET D_USER
#define D_SETTABLE (D_USER<<1)
int32_t d_mr_cset_proc(int32_t msg, DIALOG* d, int32_t)
{
	BITMAP* bmp=screen;
	int32_t colorWidth=(d->w-4)/16;
	byte* colors=static_cast<byte*>(d->dp);
	
	switch(msg)
	{
	case MSG_DRAW:
		{
			jwin_draw_frame(bmp, d->x, d->y, d->w, d->h, FR_DEEP);
			
			int32_t baseColor=((d->flags&D_CSET)!=0) ? massRecolorCSet*16 : 0;
			for(int32_t c=0; c<16; c++)
			{
				rectfill(bmp,
				  d->x+2+c*colorWidth, d->y+2,
				  d->x+2+((c+1)*colorWidth)-1, d->y+2+d->h-5,
				  baseColor+colors[c]);
			}
		}
		break;
		
	case MSG_LPRESS:
		{
			int32_t x=(gui_mouse_x()-(d->x+2))/colorWidth;
			
			if(x >= 0 && x < 16) //sanity check!
			{
				massRecolorDraggedColor=colors[x];
			}
		}
		break;
		
	case MSG_LRELEASE: // This isn't exactly right, but it'll do...
		if((d->flags&D_SETTABLE)!=0 && massRecolorDraggedColor>=0)
		{
			int32_t x=(gui_mouse_x()-(d->x+2))/colorWidth;
			if(x >= 0 && x < 16) //sanity check!
			{
				colors[x]=massRecolorDraggedColor;
				d->flags|=D_DIRTY;
			}
		}
		massRecolorDraggedColor=-1;
		break;
	}
	
	return D_O_K;
}

// Used for the full palette in 8-bit mode.
static int32_t d_mr_palette_proc(int32_t msg, DIALOG* d, int32_t)
{
	BITMAP* bmp=screen;
	int32_t colorWidth=(d->w-4)/16;
	int32_t colorHeight=(d->h-4)/12;
	
	switch(msg)
	{
	case MSG_DRAW:
		{
			jwin_draw_frame(bmp, d->x, d->y, d->w, d->h, FR_DEEP);
			for(int32_t cset=0; cset<=11; cset++)
			{
				for(int32_t color=0; color<16; color++)
				{
					rectfill(bmp,
					  d->x+2+color*colorWidth,
					  d->y+2+cset*colorHeight,
					  d->x+2+((color+1)*colorWidth)-1,
					  d->y+2+((cset+1)*colorHeight)-1,
					  cset*16+color);
				}
			}
		}
		break;
		
	case MSG_LPRESS:
		{
			int32_t cset=(gui_mouse_y()-(d->y+2))/colorHeight;
			int32_t color=(gui_mouse_x()-(d->x+2))/colorWidth;
			massRecolorDraggedColor=cset*16+color;
		}
		break;
	}
	
	return D_O_K;
}

static DIALOG recolor_4bit_dlg[] =
{
	// (dialog proc)         (x)   (y) (w)  (h)    (fg)   (bg) (key)            (flags) (d1) (d2)  (dp)
	{ jwin_win_proc,           0,   0, 216, 224,      0,     0,    0,            D_EXIT,   0,   0, (void *) "Recolor setup", NULL, NULL },
	
	// 1
	{ jwin_text_proc,         12,  32, 176,   8, vc(15), vc(1),    0,                 0,   0,   0, (void *) "From", NULL, NULL },
	{ d_mr_cset_proc,         10,  42, 196,  16,      0,     0,    0,            D_CSET,   0,   0, (void *)massRecolorSrc4Bit, NULL, NULL },
	{ jwin_text_proc,         12,  60, 176,   8, vc(15), vc(1),    0,                 0,   0,   0, (void *) "To", NULL, NULL },
	{ d_mr_cset_proc,         10,  70, 196,  16,      0,     0,    0, D_CSET|D_SETTABLE,   0,   0, (void *)massRecolorDest4Bit, NULL, NULL },
	
	// 5
	{ jwin_text_proc,         12,  96, 176,   8, vc(15), vc(1),    0,                 0,   0,   0, (void *) "Apply to which CSets in 8-bit tiles?", NULL, NULL },
	{ jwin_check_proc,        12, 112, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "0", NULL, NULL },
	{ jwin_check_proc,        36, 112, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "1", NULL, NULL },
	{ jwin_check_proc,        60, 112, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "2", NULL, NULL },
	{ jwin_check_proc,        84, 112, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "3", NULL, NULL },
	{ jwin_check_proc,       108, 112, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "4", NULL, NULL },
	{ jwin_check_proc,       132, 112, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "5", NULL, NULL },
	{ jwin_check_proc,        12, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "6", NULL, NULL },
	{ jwin_check_proc,        36, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "7", NULL, NULL },
	{ jwin_check_proc,        60, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "8", NULL, NULL },
	{ jwin_check_proc,        84, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "9", NULL, NULL },
	{ jwin_check_proc,       108, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "10", NULL, NULL },
	{ jwin_check_proc,       132, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "11", NULL, NULL },
	
	// 18
	{ jwin_check_proc,        12, 144, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "Ignore blank tiles", NULL, NULL },
	{ jwin_func_button_proc,  14, 160,  60,  20, vc(14), vc(1),    0,                 0,   0,   0, (void *) "Reset", NULL, (void*)massRecolorReset4Bit },
	{ jwin_button_proc,       82, 160, 120,  20, vc(14), vc(1),    0,            D_EXIT,   0,   0, (void *) "Switch to 8-bit mode", NULL, NULL },
	{ jwin_button_proc,       44, 188,  60,  20, vc(14), vc(1),    0,            D_EXIT,   0,   0, (void *) "OK", NULL, NULL },
	{ jwin_button_proc,      112, 188,  60,  20, vc(14), vc(1),    0,            D_EXIT,   0,   0, (void *) "Cancel", NULL, NULL },
	
	{ NULL,                    0,   0,   0,   0,   0,        0,    0,                 0,   0,   0, NULL, NULL,  NULL }
};

#define MR4_SRC_COLORS 2
#define MR4_DEST_COLORS 4
#define MR4_8BIT_EFFECT_START 6
#define MR4_IGNORE_BLANK 18
#define MR4_RESET 19
#define MR4_SWITCH 20
#define MR4_OK 21
#define MR4_CANCEL 22

static DIALOG recolor_8bit_dlg[] =
{
	// (dialog proc)         (x)  (y)  (w)  (h)   (fg)    (bg) (key)     (flags) (d1) (d2) (dp)
	{ jwin_win_proc,           0,   0, 288, 224,     0,      0,    0,     D_EXIT,   0,  0, (void *) "Recolor setup", NULL, NULL },
	
	// 1
	{ jwin_text_proc,         12,  32, 176,   8, vc(15), vc(1),    0,          0,   0,  0, (void *) "From", NULL, NULL },
	{ d_mr_cset_proc,         10,  42, 132,  12,      0,     0,    0, D_SETTABLE,   0,  0, (void *)massRecolorSrc8Bit, NULL, NULL },
	{ jwin_text_proc,         12,  60, 176,   8, vc(15), vc(1),    0,          0,   0,  0, (void *) "To", NULL, NULL },
	{ d_mr_cset_proc,         10,  70, 132,  12,      0,     0,    0, D_SETTABLE,   0,  0, (void *)massRecolorDest8Bit, NULL, NULL },
	{ d_mr_palette_proc,     144,  32, 132, 100, vc(15), vc(1),    0,          0,   0,  0, (void *) NULL, NULL, NULL },
	
	// 6
	{ jwin_check_proc,        12, 144, 168,   8, vc(15), vc(1),    0,          0,   1,  0, (void *) "Ignore blank tiles", NULL, NULL },
	{ jwin_func_button_proc,  50, 160,  60,  20, vc(14), vc(1),    0,          0,   0,  0, (void *) "Reset", NULL, (void*)massRecolorReset8Bit },
	{ jwin_button_proc,      118, 160, 120,  20, vc(14), vc(1),    0,     D_EXIT,   0,  0, (void *) "Switch to 4-bit mode", NULL, NULL },
	{ jwin_button_proc,       80, 188,  60,  20, vc(14), vc(1),    0,     D_EXIT,   0,  0, (void *) "OK", NULL, NULL },
	{ jwin_button_proc,      148, 188,  60,  20, vc(14), vc(1),    0,     D_EXIT,   0,  0, (void *) "Cancel", NULL, NULL },
	
	{ NULL,                    0,   0,   0,   0,      0,     0,    0,          0,   0,  0,       NULL,                           NULL,  NULL }
};

#define MR8_SRC_COLORS 2
#define MR8_DEST_COLORS 4
#define MR8_PALETTE 5
#define MR8_IGNORE_BLANK 6
#define MR8_RESET 7
#define MR8_SWITCH 8
#define MR8_OK 9
#define MR8_CANCEL 10

static void massRecolorInit(int32_t cset)
{
	massRecolorDraggedColor=-1;
	massRecolorCSet=cset;
	
	recolor_4bit_dlg[0].dp2=lfont;
	recolor_8bit_dlg[0].dp2=lfont;
	
	for(int32_t i=0; i<=11; i++)
	{
		if((massRecolor8BitCSets&(1<<i))!=0)
			recolor_4bit_dlg[MR4_8BIT_EFFECT_START+i].flags|=D_SELECTED;
		else
			recolor_4bit_dlg[MR4_8BIT_EFFECT_START+i].flags&=~D_SELECTED;
	}
	
	if(massRecolorIgnoreBlank)
	{
		recolor_4bit_dlg[MR4_IGNORE_BLANK].flags|=D_SELECTED;
		recolor_8bit_dlg[MR8_IGNORE_BLANK].flags|=D_SELECTED;
	}
	else
	{
		recolor_4bit_dlg[MR4_IGNORE_BLANK].flags&=~D_SELECTED;
		recolor_8bit_dlg[MR8_IGNORE_BLANK].flags&=~D_SELECTED;
	}
	
	large_dialog(recolor_4bit_dlg);
	large_dialog(recolor_8bit_dlg);
	
	// Quick fix for large_dialog() screwing these up. It's ugly. Whatever.
	if((recolor_4bit_dlg[MR4_DEST_COLORS].w-4)%4!=0)
	{
		recolor_4bit_dlg[MR4_SRC_COLORS].x++;
		recolor_4bit_dlg[MR4_SRC_COLORS].w-=2;
		recolor_4bit_dlg[MR4_DEST_COLORS].x++;
		recolor_4bit_dlg[MR4_DEST_COLORS].w-=2;
		
		recolor_8bit_dlg[MR8_SRC_COLORS].x++;
		recolor_8bit_dlg[MR8_SRC_COLORS].w-=2;
		recolor_8bit_dlg[MR8_DEST_COLORS].x++;
		recolor_8bit_dlg[MR8_DEST_COLORS].w-=2;
		recolor_8bit_dlg[MR8_PALETTE].x++;
		recolor_8bit_dlg[MR8_PALETTE].w-=2;
		recolor_8bit_dlg[MR8_PALETTE].y++;
		recolor_8bit_dlg[MR8_PALETTE].h-=2;
	}
}

static void massRecolorApplyChanges()
{
	massRecolor8BitCSets=0;
	for(int32_t i=0; i<=11; i++)
	{
		if((recolor_4bit_dlg[MR4_8BIT_EFFECT_START+i].flags&D_SELECTED)!=0)
			massRecolor8BitCSets|=1<<i;
	}
	
	if(massRecolorType==MR_4BIT)
		massRecolorIgnoreBlank=(recolor_4bit_dlg[MR4_IGNORE_BLANK].flags&D_SELECTED)!=0;
	else
		massRecolorIgnoreBlank=(recolor_8bit_dlg[MR8_IGNORE_BLANK].flags&D_SELECTED)!=0;
}

static bool massRecolorSetup(int32_t cset)
{
	massRecolorInit(cset);
	
	// Remember the current colors in case the user cancels.
	int32_t oldDest4Bit[16], oldSrc8Bit[16], oldDest8Bit[16];
	for(int32_t i=0; i<16; i++)
	{
		oldDest4Bit[i]=massRecolorDest4Bit[i];
		oldSrc8Bit[i]=massRecolorSrc8Bit[i];
		oldDest8Bit[i]=massRecolorDest8Bit[i];
	}
	
	byte type=massRecolorType;
	int32_t ret;
	do
	{
		if(type==MR_4BIT)
		{
			ret=zc_popup_dialog(recolor_4bit_dlg, MR4_OK);
			if(ret==MR4_SWITCH)
				type=MR_8BIT;
		}
		else
		{
			ret=zc_popup_dialog(recolor_8bit_dlg, MR8_OK);
			if(ret==MR8_SWITCH)
				type=MR_4BIT;
		}
	} while(ret==MR4_SWITCH || ret==MR8_SWITCH);
	
	if(ret!=MR4_OK && ret!=MR8_OK) // Canceled
	{
		for(int32_t i=0; i<16; i++)
		{
			massRecolorDest4Bit[i]=oldDest4Bit[i];
			massRecolorSrc8Bit[i]=oldSrc8Bit[i];
			massRecolorDest8Bit[i]=oldDest8Bit[i];
		}
		return false;
	}
	
	// OK
	massRecolorType=type;
	massRecolorApplyChanges();
	return true;
}

static void massRecolorApply4Bit(int32_t tile)
{
	byte buf[256];
	unpack_tile(newtilebuf, tile, 0, true);
	
	if(newtilebuf[tile].format==tf4Bit)
	{
		for(int32_t i=0; i<256; i++)
			buf[i]=massRecolorDest4Bit[unpackbuf[i]];
	}
	else // 8-bit
	{
		for(int32_t i=0; i<256; i++)
		{
			word cset=unpackbuf[i]>>4;
			if((massRecolor8BitCSets&(1<<cset))!=0) // Recolor this CSet?
			{
				word color=unpackbuf[i]&15;
				buf[i]=(cset<<4)|massRecolorDest4Bit[color];
			}
			else
				buf[i]=unpackbuf[i];
		}
	}
	
	pack_tile(newtilebuf, buf, tile);
}

static void massRecolorApply8Bit(int32_t tile)
{
	byte buf[256];
	unpack_tile(newtilebuf, tile, 0, true);
	
	for(int32_t i=0; i<256; i++)
	{
		byte color=unpackbuf[i];
		for(int32_t j=0; j<16; j++)
		{
			if(massRecolorSrc8Bit[j]==color)
			{
				color=massRecolorDest8Bit[j];
				break;
			}
		}
		buf[i]=color;
	}
	
	pack_tile(newtilebuf, buf, tile);
}

static void massRecolorApply(int32_t tile)
{
	if(massRecolorIgnoreBlank && blank_tile_table[tile])
		return;
	
	if(massRecolorType==MR_4BIT)
		massRecolorApply4Bit(tile);
	else // 8-bit
	{
		if(newtilebuf[tile].format==tf4Bit)
			return;
		massRecolorApply8Bit(tile);
	}
}

static void massRecolorReset4Bit()
{
	for(int32_t i=0; i<16; i++)
		massRecolorDest4Bit[i]=i;
	recolor_4bit_dlg[MR4_DEST_COLORS].flags|=D_DIRTY;
}

static void massRecolorReset8Bit()
{
	for(int32_t i=0; i<16; i++)
	{
		massRecolorSrc8Bit[i]=0;
		massRecolorDest8Bit[i]=0;
	}
	
	recolor_8bit_dlg[MR8_SRC_COLORS].flags|=D_DIRTY;
	recolor_8bit_dlg[MR8_DEST_COLORS].flags|=D_DIRTY;
}

void center_zq_tiles_dialogs()
{
	jwin_center_dialog(create_relational_tiles_dlg);
	jwin_center_dialog(icon_dlg);
	jwin_center_dialog(leech_dlg);
	jwin_center_dialog(tile_move_list_dlg);
	jwin_center_dialog(recolor_4bit_dlg);
	jwin_center_dialog(recolor_8bit_dlg);
}

//.ZCOMBO

int32_t readcombo_loop(PACKFILE* f, word section_version, newcombo& temp_combo);
int32_t writecombo_loop(PACKFILE *f, word section_version, newcombo const& tmp_cmb);

int32_t readcombofile_old(PACKFILE *f, int32_t skip, byte nooverwrite, int32_t zversion,
	dword section_version, dword section_cversion, int32_t index, int32_t count)
{
	newcombo temp_combo;
	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		temp_combo.clear();
		if(!p_igetw(&temp_combo.tile,f,true))
		{
			return 0;
		}
		temp_combo.o_tile = temp_combo.tile;
			
		if(!p_getc(&temp_combo.flip,f,true))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.walk,f,true))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.type,f,true))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.csets,f,true))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.frames,f,true))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.speed,f,true))
		{
			return 0;
		}
			
		if(!p_igetw(&temp_combo.nextcombo,f,true))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.nextcset,f,true))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.flag,f,true))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.skipanim,f,true))
		{
			return 0;
		}
			
		if(!p_igetw(&temp_combo.nexttimer,f,true))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.skipanimy,f,true))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.animflags,f,true))
		{
			return 0;
		}
		
		//2.55 starts here
		if ( zversion >= 0x255 )
		{
			if  ( section_version >= 12 )
			{
				for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
				{
					if(!p_igetl(&temp_combo.attributes[q],f,true))
					{
						return 0;
					}
				}
				if(!p_igetl(&temp_combo.usrflags,f,true))
				{
						return 0;
				}	 
				for ( int32_t q = 0; q < 3; q++ ) 
				{
					if(!p_igetl(&temp_combo.triggerflags[q],f,true))
					{
						return 0;
					}
				}
				   
				if(!p_igetl(&temp_combo.triggerlevel,f,true))
				{
						return 0;
				}
				if(section_version >= 22)
				{
					if(!p_getc(&temp_combo.triggerbtn,f,true))
					{
						return 0;
					}
				}
				if(section_version < 23)
				{
					switch(temp_combo.type) //combotriggerCMBTYPEFX now required for combotype-specific effects
					{
						case cSCRIPT1: case cSCRIPT2: case cSCRIPT3: case cSCRIPT4: case cSCRIPT5:
						case cSCRIPT6: case cSCRIPT7: case cSCRIPT8: case cSCRIPT9: case cSCRIPT10:
						case cTRIGGERGENERIC: case cCSWITCH:
							temp_combo.triggerflags[0] |= combotriggerCMBTYPEFX;
					}
				}
				if(section_version >= 24)
				{
					if(!p_getc(&temp_combo.triggeritem,f,true))
					{
						return 0;
					}
					if(!p_getc(&temp_combo.trigtimer,f,true))
					{
						return 0;
					}
				}
				if(section_version >= 25)
				{
					if(!p_getc(&temp_combo.trigsfx,f,true))
					{
						return 0;
					}
				}
				else
				{
					switch(temp_combo.type)
					{
						case cLOCKBLOCK: case cBOSSLOCKBLOCK:
							if(!(temp_combo.usrflags & cflag3))
								temp_combo.attribytes[3] = WAV_DOOR;
							temp_combo.usrflags &= ~cflag3;
							break;
					}
				}
				if(section_version < 26)
				{
					if(temp_combo.type == cARMOS)
					{
						if(temp_combo.usrflags & cflag1)
							temp_combo.usrflags |= cflag3;
					}
				}
				if(section_version >= 27)
				{
					if(!p_igetl(&temp_combo.trigchange,f,true))
					{
						return qe_invalid;
					}
				}
				else
				{
					if(temp_combo.triggerflags[0] & 0x00040000) //'next'
						temp_combo.trigchange = 1;
					else if(temp_combo.triggerflags[0] & 0x00080000) //'prev'
						temp_combo.trigchange = -1;
					else temp_combo.trigchange = 0;
					temp_combo.triggerflags[0] &= ~(0x00040000|0x00080000);
				}
				if(section_version >= 29)
				{
					if(!p_igetw(&temp_combo.trigprox,f,true))
					{
						return qe_invalid;
					}
					if(!p_getc(&temp_combo.trigctr,f,true))
					{
						return qe_invalid;
					}
					if(!p_igetl(&temp_combo.trigctramnt,f,true))
					{
						return qe_invalid;
					}
				}
				else
				{
					temp_combo.trigprox = 0;
					temp_combo.trigctr = 0;
					temp_combo.trigctramnt = 0;
				}
				if(section_version >= 30)
				{
					if(!p_getc(&temp_combo.triglbeam,f,true))
					{
						return qe_invalid;
					}
				}
				else temp_combo.triglbeam = 0;
				if(section_version >= 31)
				{
					if(!p_getc(&temp_combo.trigcschange,f,true))
					{
						return qe_invalid;
					}
					if(!p_igetw(&temp_combo.spawnitem,f,true))
					{
						return qe_invalid;
					}
					if(!p_igetw(&temp_combo.spawnenemy,f,true))
					{
						return qe_invalid;
					}
					if(!p_getc(&temp_combo.exstate,f,true))
					{
						return qe_invalid;
					}
					if(!p_igetl(&temp_combo.spawnip,f,true))
					{
						return qe_invalid;
					}
					if(!p_getc(&temp_combo.trigcopycat,f,true))
					{
						return qe_invalid;
					}
				}
				else
				{
					temp_combo.trigcschange = 0;
					temp_combo.spawnitem = 0;
					temp_combo.spawnenemy = 0;
					temp_combo.exstate = -1;
					temp_combo.spawnip = 0;
					temp_combo.trigcopycat = 0;
				}
				if(section_version >= 32)
				{
					if(!p_getc(&temp_combo.trigcooldown,f,true))
					{
						return qe_invalid;
					}
				}
				else
				{
					temp_combo.trigcooldown = 0;
				}
				for ( int32_t q = 0; q < 11; q++ ) 
				{
					if(!p_getc(&temp_combo.label[q],f,true))
					{
						return 0;
					}
				}
			}
			if  ( section_version >= 13 )
			{
				for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
				{
					if(!p_getc(&temp_combo.attribytes[q],f,true))
					{
						return 0;
					}
				}
				
			}
		}
		
		if ( !(skip && (index+(tilect-1)) < skip) ) //is -1 still needed here?
		{
			if ( !nooverwrite || combobuf[index+tilect].is_blank() )
			{
				memcpy(&combobuf[index+(tilect)],&temp_combo,sizeof(newcombo));
			}
		}
	}
	
	return 1;
}

int32_t readcombofile(PACKFILE *f, int32_t skip, byte nooverwrite, int32_t start)
{
	dword section_version=0;
	dword section_cversion=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zcombo packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_COMBOS ) || ( section_version == V_COMBOS && section_cversion > CV_COMBOS ) )
	{
		al_trace("Cannot read .zcombo packfile made using V_COMBOS (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zcombo packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	int32_t index = 0;
	int32_t count = 0;
	
	//tile id
	if(!p_igetl(&index,f,true))
	{
		return 0;
	}
	if(start > -1) index = start;
	// al_trace("Reading combo: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f,true))
	{
		return 0;
	}
	// al_trace("Reading combo: count(%d)\n", count);
	reset_combo_animations();
	reset_combo_animations2();
	
	if(section_version < 33)
		return readcombofile_old(f,skip,nooverwrite,zversion,section_version,section_cversion,index,count);
	
	newcombo temp_combo;
	size_t end = index+count;
	for ( size_t q = index; q < end; q++ )
	{
		auto ret = readcombo_loop(f,section_version,temp_combo);
		if(ret) return 0;
		
		if ( !(skip && q-1 < skip) )
		{
			if ( !nooverwrite || combobuf[q].is_blank() )
			{
				memcpy(&combobuf[q],&temp_combo,sizeof(newcombo));
			}
		}
	}
	
	return 1;
}
int32_t readcombofile_to_location(PACKFILE *f, int32_t start, byte nooverwrite, int32_t skip)
{
	return readcombofile(f,skip,nooverwrite,start);
}
int32_t writecombofile(PACKFILE *f, int32_t index, int32_t count)
{
	dword section_version=V_COMBOS;
	dword section_cversion=CV_COMBOS;
	int32_t zversion = ZELDA_VERSION;
	int32_t zbuild = VERSION_BUILD;
	
	if(!p_iputl(zversion,f))
	{
		return 0;
	}
	if(!p_iputl(zbuild,f))
	{
		return 0;
	}
	if(!p_iputw(section_version,f))
	{
		return 0;
	}
	
	if(!p_iputw(section_cversion,f))
	{
		return 0;
	}
	
	//start tile id
	if(!p_iputl(index,f))
	{
		return 0;
	}
	
	//count
	if(!p_iputl(count,f))
	{
		return 0;
	}
	reset_combo_animations();
	reset_combo_animations2();
	size_t end = index+count;
	for(size_t q = index; q < end; ++q)
	{
		if(writecombo_loop(f, section_version, combobuf[q]))
			return 0;
	}
	
	return 1;
	
}

//.ZALIAS


//.ZALIAS

int32_t readcomboaliasfile(PACKFILE *f)
{
	dword section_version=0;
	dword section_cversion=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	word tempword = 0;
	
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	al_trace("readoneweapon section_version: %d\n", section_version);
	al_trace("readoneweapon section_cversion: %d\n", section_cversion);

	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zalias packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_COMBOALIASES ) || ( section_version == V_COMBOALIASES && section_cversion > CV_COMBOALIASES ) )
	{
		al_trace("Cannot read .zalias packfile made using V_COMBOALIASES (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zalias packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	int32_t index = 0;
	int32_t count = 0;
	int32_t count2 = 0;
	byte tempcset = 0;
	
	//tile id
	if(!p_igetl(&index,f,true))
	{
		return 0;
	}
	al_trace("Reading combo: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f,true))
	{
		return 0;
	}
	al_trace("Reading combo: count(%d)\n", count);
	
	combo_alias temp_alias;
	memset(&temp_alias, 0, sizeof(temp_alias));

	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		memset(&temp_alias, 0, sizeof(temp_alias));
		if(!p_igetw(&temp_alias.combo,f,true))
			{
				return 0;
			}
			
			if(!p_getc(&temp_alias.cset,f,true))
			{
				return 0;
			}
			
			
		
		if(!p_igetl(&count2,f,true))
			{
				return 0;
			}
		al_trace("Read, Combo alias count is: %d\n", count2);
			if(!p_getc(&temp_alias.width,f,true))
			{
				return 0;
			}
			
			if(!p_getc(&temp_alias.height,f,true))
			{
				return 0;
			}
			
			if(!p_getc(&temp_alias.layermask,f,true))
			{
				return 0;
			}
			//These values are flexible, and may differ in size, so we delete them 
		//and recreate them at the correct size on the pointer. 
		delete[] temp_alias.combos;
		temp_alias.combos = new word[count2];
		delete[] temp_alias.csets;
		temp_alias.csets = new byte[count2];
			for(int32_t k=0; k<count2; k++)
			{
				if(!p_igetw(&tempword,f,true))
				{
			//al_trace("Could not reas alias.combos[%d]\n",k);
					return 0;
				}
		else
		{
			//al_trace("Read Combo Alias Combo [%d] as: %d\n", k, tempword);
			
			
			//al_trace("tempword is: %d\n", tempword);
			temp_alias.combos[k] = tempword;
			//al_trace("Combo Alias Combo [%d] is: %d\n", k, temp_alias.combos[k]);
		}
			}
		//al_trace("Read alias combos.\n");
			
			for(int32_t k=0; k<count2; k++)
			{
				if(!p_getc(&tempcset,f,true))
				//if(!p_getc(&temp_alias.csets[k],f,true))
		{
					return 0;
				}
		else
		{
			//al_trace("Read Combo Alias CSet [%d] as: %d\n", k, tempcset);
			
			temp_alias.csets[k] = tempcset;
			//al_trace("Combo Alias CSet [%d] is: %d\n", k, temp_alias.csets[k]);
		}
			}
		//al_trace("Read alias csets.\n");
		//al_trace("About to memcpy a combo alias\n");
		memcpy(&combo_aliases[index+(tilect)],&temp_alias,sizeof(combo_alias));
	}
	
	//::memcpy(&(newtilebuf[tile_index]),&temptile,sizeof(tiledata));
	
			
	return 1;
	
}

int32_t readcomboaliasfile_to_location(PACKFILE *f, int32_t start)
{
	dword section_version=0;
	dword section_cversion=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	al_trace("readcomboaliasfile_to_location section_version: %d\n", section_version);
	al_trace("readcomboaliasfile_to_location section_cversion: %d\n", section_cversion);

	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zalias packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_COMBOALIASES ) || ( section_version == V_COMBOALIASES && section_cversion > CV_COMBOALIASES ) )
	{
		al_trace("Cannot read .zalias packfile made using V_COMBOALIASES (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zalias packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	int32_t index = 0;
	int32_t count = 0;
	int32_t count2 = 0;
	byte tempcset = 0;
	word tempword = 0;
	
	
	//tile id
	if(!p_igetl(&index,f,true))
	{
		return 0;
	}
	al_trace("Reading tile: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f,true))
	{
		return 0;
	}
	al_trace("Reading tile: count(%d)\n", count);
	
	
	combo_alias temp_alias;
	memset(&temp_alias, 0, sizeof(temp_alias)); 

	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		memset(&temp_alias, 0, sizeof(temp_alias));
		if(!p_igetw(&temp_alias.combo,f,true))
			{
				return 0;
			}
			
			if(!p_getc(&temp_alias.cset,f,true))
			{
				return 0;
			}
			
			int32_t count2 = 0;
		
		if(!p_igetl(&count2,f,true))
			{
				return 0;
			}
		
			if(!p_getc(&temp_alias.width,f,true))
			{
				return 0;
			}
			
			if(!p_getc(&temp_alias.height,f,true))
			{
				return 0;
			}
			
			if(!p_getc(&temp_alias.layermask,f,true))
			{
				return 0;
			}
		//These values are flexible, and may differ in size, so we delete them 
		//and recreate them at the correct size on the pointer. 
			delete[] temp_alias.combos;
		temp_alias.combos = new word[count2];
		delete[] temp_alias.csets;
		temp_alias.csets = new byte[count2];
		
			for(int32_t k=0; k<count2; k++)
			{
				if(!p_igetw(&tempword,f,true))
				{
					return 0;
				}
		else
		{
			temp_alias.combos[k] = tempword;
		}
			}
			
			for(int32_t k=0; k<count2; k++)
			{
				if(!p_getc(&tempcset,f,true))
				{
					return 0;
				}
		else
		{
			temp_alias.csets[k] = tempcset;
		}
			}
		
		
		if ( start+(tilect) < MAXCOMBOALIASES )
		{
			memcpy(&combo_aliases[start+(tilect)],&temp_alias,sizeof(temp_alias));
		}
	}
	
	
	//::memcpy(&(newtilebuf[tile_index]),&temptile,sizeof(tiledata));
	
			
	return 1;
	
}
int32_t writecomboaliasfile(PACKFILE *f, int32_t index, int32_t count)
{
	al_trace("Running writecomboaliasfile\n");
	dword section_version=V_COMBOALIASES;
	dword section_cversion=CV_COMBOALIASES;
	int32_t zversion = ZELDA_VERSION;
	int32_t zbuild = VERSION_BUILD;
	
	if(!p_iputl(zversion,f))
	{
		return 0;
	}
	if(!p_iputl(zbuild,f))
	{
		return 0;
	}
	if(!p_iputw(section_version,f))
	{
		return 0;
	}
	
	if(!p_iputw(section_cversion,f))
	{
		return 0;
	}
	
	//start tile id
	if(!p_iputl(index,f))
	{
		return 0;
	}
	
	//count
	if(!p_iputl(count,f))
	{
		return 0;
	}
	
	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
	
		if(!p_iputw(combo_aliases[index+(tilect)].combo,f))
			{
				return 0;
			}
			
			if(!p_putc(combo_aliases[index+(tilect)].cset,f))
			{
				return 0;
			}
			
			int32_t count2 = ((combo_aliases[index+(tilect)].width+1)*(combo_aliases[index+(tilect)].height+1))*(comboa_lmasktotal(combo_aliases[index+(tilect)].layermask)+1);
			
		if(!p_iputl(count2,f))
			{
				return 0;
			}
		al_trace("Write`, Combo alias count is: %d\n", count2);
		
			if(!p_putc(combo_aliases[index+(tilect)].width,f))
			{
				return 0;
			}
			
			if(!p_putc(combo_aliases[index+(tilect)].height,f))
			{
				return 0;
			}
			
			if(!p_putc(combo_aliases[index+(tilect)].layermask,f))
			{
				return 0;
			}
			
			for(int32_t k=0; k<count2; k++)
			{
				if(!p_iputw(combo_aliases[index+(tilect)].combos[k],f))
				{
					return 0;
				}
			}
			
			for(int32_t k=0; k<count2; k++)
			{
				if(!p_putc(combo_aliases[index+(tilect)].csets[k],f))
				{
					return 0;
				}
			}
	}
	
	return 1;
	
}

int32_t select_dmap_tile(int32_t &tile,int32_t &flip,int32_t type,int32_t &cs,bool edit_cs,int32_t exnow, bool always_use_flip)
{
	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-31)>>1;
	popup_zqdialog_start_a5(window_xofs,window_yofs,w+12,h+31);
	popup_zqdialog_start(window_xofs,window_yofs,w+12,h+31,0xFF);
	reset_combo_animations();
	reset_combo_animations2();
	bound(tile,0,NEWMAXTILES-1);
	ex=exnow;
	tile = DMapEditorLastMaptileUsed;
	int32_t done=0;
	int32_t oflip=flip;
	int32_t otile=tile;
	int32_t ocs=cs;
	int32_t first=(tile/TILES_PER_PAGE)*TILES_PER_PAGE; //first tile on the current page
	int32_t copy=-1;
	int32_t tile2=tile,copycnt=0;
	int32_t tile_clicked=-1;
	bool rect_sel=true;
	bound(first,0,(TILES_PER_PAGE*TILE_PAGES)-1);
	position_mouse_z(0);
	
	go();
	
	register_used_tiles();
	int32_t screen_xofs=6;
	int32_t screen_yofs=25;
	int32_t panel_yofs=3;
	int32_t mul = 2;
	FONT *tfont = lfont_l;
	
	draw_tile_list_window();
	int32_t f=0;
	draw_tiles(first,cs,f);
	
	if(type==0)
	{
		tile_info_0(tile,tile2,cs,copy,copycnt,first/TILES_PER_PAGE,rect_sel);
	}
	else
	{
		tile_info_1(otile,oflip,ocs,tile,flip,cs,copy,first/TILES_PER_PAGE, always_use_flip);
	}
	
	go_tiles();
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	bool bdown=false;
	
	#define FOREACH_START_DMAPTILE(_t) \
	{ \
		int32_t _first, _last; \
		if(is_rect) \
		{ \
			_first=top*TILES_PER_ROW+left; \
			_last=_first+rows*TILES_PER_ROW|+columns-1; \
		} \
		else \
		{ \
			_first=zc_min(tile, tile2); \
			_last=zc_max(tile, tile2); \
		} \
		for(int32_t _t=_first; _t<=_last; _t++) \
		{ \
			if(is_rect) \
			{ \
				int32_t row=TILEROW(_t); \
				if(row<top || row>=top+rows) \
					continue; \
				int32_t col=TILECOL(_t); \
				if(col<left || col>=left+columns) \
					continue; \
			} \
		
	#define FOREACH_DMAPTILE_END\
		} \
	}
	
	bool did_snap = false;
	do
	{
		rest(4);
		int32_t top=TILEROW(zc_min(tile, tile2));
		int32_t left=zc_min(TILECOL(tile), TILECOL(tile2));
		int32_t rows=TILEROW(zc_max(tile, tile2))-top+1;
		int32_t columns=zc_max(TILECOL(tile), TILECOL(tile2))-left+1;
		bool is_rect=(rows==1)||(columns==TILES_PER_ROW)||rect_sel;
		bool redraw=false;
		
		if(mouse_z!=0)
		{
			sel_tile(tile,tile2,first,type,((mouse_z/abs(mouse_z))*(-1)*TILES_PER_PAGE));
			position_mouse_z(0);
			redraw=true;
		}
		
		if(keypressed())
		{
			switch(readkey()>>8)
			{
			case KEY_ENTER_PAD:
			case KEY_ENTER:
				done=2;
				break;
				
			case KEY_ESC:
				done=1;
				break;
				
			case KEY_F1:
				onHelp();
				break;
				
			case KEY_EQUALS:
			case KEY_PLUS_PAD:
			{
				if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL] ||
						key[KEY_ALT] || key[KEY_ALTGR])
				{
					FOREACH_START_DMAPTILE(t)
						if(key[KEY_ALT] || key[KEY_ALTGR])
							shift_tile_colors(t, 16, false);
						else
							shift_tile_colors(t, 1, key[KEY_LSHIFT] || key[KEY_RSHIFT]);
					FOREACH_DMAPTILE_END
					
					register_blank_tiles();
				}
				else if(edit_cs)
					cs = (cs<13) ? cs+1:0;
					
				redraw=true;
				break;
			}
			case KEY_Z:
			case KEY_F12:
			{
				if(!did_snap)
				{
					onSnapshot();
					redraw = true;
					did_snap = true;
				}
				break;
			}
			case KEY_S:
			{
				if(!getname("Save ZTILE(.ztile)", "ztile", NULL,datapath,false))
					break;   
				PACKFILE *f=pack_fopen_password(temppath,F_WRITE, "");
				if(!f) break;
				al_trace("Saving tile: %d\n", tile);
				writetilefile(f,tile,1);
				pack_fclose(f);
				break;
			}
			case KEY_L:
			{
				if(!getname("Load ZTILE(.ztile)", "ztile", NULL,datapath,false))
					break;   
				PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
				if(!f) break;
				al_trace("Saving tile: %d\n", tile);
				if (!readtilefile(f))
				{
					al_trace("Could not read from .ztile packfile %s\n", temppath);
					jwin_alert("ZTILE File: Error","Could not load the specified Tile.",NULL,NULL,"O&K",NULL,'k',0,lfont);
				}
				else
				{
					jwin_alert("ZTILE File: Success!","Loaded the source tiles to your tile sheets!",NULL,NULL,"O&K",NULL,'k',0,lfont);
				}
			
				pack_fclose(f);
				//register_blank_tiles();
				//register_used_tiles();
				redraw=true;
				break;
			}
			case KEY_MINUS:
			case KEY_MINUS_PAD:
			{
				if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL] ||
						key[KEY_ALT] || key[KEY_ALTGR])
				{
					FOREACH_START_DMAPTILE(t)
						if(key[KEY_ALT] || key[KEY_ALTGR])
							shift_tile_colors(t, -16, false);
						else
							shift_tile_colors(t, -1, key[KEY_LSHIFT] || key[KEY_RSHIFT]);
					FOREACH_DMAPTILE_END
					
					register_blank_tiles();
				}
				else if(edit_cs)
					cs = (cs>0) ? cs-1:13;
					
				redraw=true;
				break;
			}
			
			case KEY_UP:
			{
				switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
				{
				case 3:  //ALT and CTRL
				case 2:  //ALT
					if(is_rect)
					{
						saved=false;
						go_slide_tiles(columns, rows, top, left);
						int32_t bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
						bool same = true;
						
						for(int32_t d=0; d<columns; d++)
						{
							for(int32_t s=0; s<rows; s++)
							{
								int32_t t=((top+s)*TILES_PER_ROW)+left+d;
								
								if(newtilebuf[t].format!=bitcheck) same = false;
							}
						}
						
						if(!same) break;
						
						for(int32_t c=0; c<columns; c++)
						{
							for(int32_t r=0; r<rows; r++)
							{
								int32_t temptile=((top+r)*TILES_PER_ROW)+left+c;
								qword *src_pixelrow=(qword*)(newundotilebuf[temptile].data+(8*bitcheck));
								qword *dest_pixelrow=(qword*)(newtilebuf[temptile].data);
								
								for(int32_t pixelrow=0; pixelrow<16*bitcheck; pixelrow++)
								{
									if(pixelrow==15*bitcheck)
									{
										int32_t srctile=temptile+TILES_PER_ROW;
										if(srctile>=NEWMAXTILES)
											srctile-=rows*TILES_PER_ROW;
										src_pixelrow=(qword*)(newtilebuf[srctile].data);
									}
									
									*dest_pixelrow=*src_pixelrow;
									dest_pixelrow++;
									src_pixelrow++;
								}
							}
							
							qword *dest_pixelrow=(qword*)(newtilebuf[((top+rows-1)*TILES_PER_ROW)+left+c].data+(120*bitcheck));
							
							for(int32_t b=0; b<bitcheck; b++,dest_pixelrow++)
							{
								if((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
								{
									*dest_pixelrow=0;
								}
								else
								{
									qword *src_pixelrow=(qword*)(newundotilebuf[(top*TILES_PER_ROW)+left+c].data+(8*b));
									*dest_pixelrow=*src_pixelrow;
								}
							}
						}
					}
					
					register_blank_tiles();
					redraw=true;
					break;
					
				case 1:  //CTRL
				case 0:  //None
					sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-1*(tile_page_row(tile)*TILES_PER_ROW):-TILES_PER_ROW);
					redraw=true;
					
				default: //Others
					break;
				}
			}
			break;
			
			case KEY_DOWN:
			{
				switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
				{
				case 3:  //ALT and CTRL
				case 2:  //ALT
					if(is_rect)
					{
						saved=false;
						go_slide_tiles(columns, rows, top, left);
						int32_t bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
						bool same = true;
						
						for(int32_t c=0; c<columns; c++)
						{
							for(int32_t r=0; r<rows; r++)
							{
								int32_t t=((top+r)*TILES_PER_ROW)+left+c;
								
								if(newtilebuf[t].format!=bitcheck) same = false;
							}
						}
						
						if(!same) break;
						
						for(int32_t c=0; c<columns; c++)
						{
							for(int32_t r=rows-1; r>=0; r--)
							{
								int32_t temptile=((top+r)*TILES_PER_ROW)+left+c;
								qword *src_pixelrow=(qword*)(newundotilebuf[temptile].data+(112*bitcheck)+(8*(bitcheck-1)));
								qword *dest_pixelrow=(qword*)(newtilebuf[temptile].data+(120*bitcheck)+(8*(bitcheck-1)));
								
								for(int32_t pixelrow=(8<<bitcheck)-1; pixelrow>=0; pixelrow--)
								{
									if(pixelrow<bitcheck)
									{
										int32_t srctile=temptile-TILES_PER_ROW;
										if(srctile<0)
											srctile+=rows*TILES_PER_ROW;
										qword *tempsrc=(qword*)(newtilebuf[srctile].data+(120*bitcheck)+(8*pixelrow));
										*dest_pixelrow=*tempsrc;
										//*dest_pixelrow=0;
									}
									else
									{
										*dest_pixelrow=*src_pixelrow;
									}
									
									dest_pixelrow--;
									src_pixelrow--;
								}
							}
							
							qword *dest_pixelrow=(qword*)(newtilebuf[(top*TILES_PER_ROW)+left+c].data);
							qword *src_pixelrow=(qword*)(newundotilebuf[((top+rows-1)*TILES_PER_ROW)+left+c].data+(120*bitcheck));
							
							for(int32_t b=0; b<bitcheck; b++)
							{
								if((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
								{
									*dest_pixelrow=0;
								}
								else
								{
									*dest_pixelrow=*src_pixelrow;
								}
								
								dest_pixelrow++;
								src_pixelrow++;
							}
						}
					}
					
					register_blank_tiles();
					redraw=true;
					break;
					
				case 1:  //CTRL
				case 0:  //None
					sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?((TILE_ROWS_PER_PAGE-1)-tile_page_row(tile))*TILES_PER_ROW:TILES_PER_ROW);
					redraw=true;
					
				default: //Others
					break;
				}
			}
			break;
			
			case KEY_LEFT:
			{
				switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
				{
				case 3:  //ALT and CTRL
				case 2:  //ALT
					if(is_rect)
					{
						saved=false;
						go_slide_tiles(columns, rows, top, left);
						int32_t bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
						bool same = true;
						
						for(int32_t c=0; c<columns; c++)
						{
							for(int32_t r=0; r<rows; r++)
							{
								int32_t t=((top+r)*TILES_PER_ROW)+left+c;
								
								if(newtilebuf[t].format!=bitcheck) same = false;
							}
						}
						
						if(!same) break;
						
						for(int32_t r=0; r<rows; r++)
						{
							for(int32_t c=0; c<columns; c++)
							{
								int32_t temptile=((top+r)*TILES_PER_ROW)+left+c;
								byte *dest_pixelrow=(newtilebuf[temptile].data);
								
								for(int32_t pixelrow=0; pixelrow<16; pixelrow++)
								{
#ifdef ALLEGRO_LITTLE_ENDIAN
								
									//if(bitcheck==tf4Bit)
									// {
									for(int32_t p=0; p<(8*bitcheck)-1; p++)
									{
										if(bitcheck==tf4Bit)
										{
											*dest_pixelrow=*dest_pixelrow>>4;
											*dest_pixelrow|=(*(dest_pixelrow+1)<<4);
											
											if(p==6) *(dest_pixelrow+1)=*(dest_pixelrow+1)>>4;
										}
										else
										{
											*dest_pixelrow=*(dest_pixelrow+1);
										}
										
										dest_pixelrow++;
									}
									
#else
									
									for(int32_t p=0; p<(8*bitcheck)-1; p++)
									{
										if(bitcheck==tf4Bit)
										{
											*dest_pixelrow=*dest_pixelrow<<4;
											*dest_pixelrow|=(*(dest_pixelrow+1)>>4);
									
											if(p==6) *(dest_pixelrow+1)=*(dest_pixelrow+1)<<4;
										}
										else
										{
											*dest_pixelrow=*(dest_pixelrow+1);
										}
									
										dest_pixelrow++;
									}
									
#endif
									
									if(c==columns-1)
									{
										if(!(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
										{
											byte *tempsrc=(newundotilebuf[((top+r)*TILES_PER_ROW)+left].data+(pixelrow*8*bitcheck));
#ifdef ALLEGRO_LITTLE_ENDIAN
											
											if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
											else *dest_pixelrow=*tempsrc;
											
#else
											
											if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
											else *dest_pixelrow=*tempsrc;
											
#endif
										}
									}
									else
									
									{
										byte *tempsrc=(newtilebuf[temptile+1].data+(pixelrow*8*bitcheck));
#ifdef ALLEGRO_LITTLE_ENDIAN
										
										if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
										else *dest_pixelrow=*tempsrc;
										
#else
										
										if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
										else *dest_pixelrow=*tempsrc;
										
#endif
									}
									
									dest_pixelrow++;
								}
							}
						}
						
						register_blank_tiles();
						redraw=true;
					}
					
					break;
					
				case 1:  //CTRL
				case 0:  //None
					sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-(tile%TILES_PER_ROW):-1);
					redraw=true;
					
				default: //Others
					break;
				}
			}
			break;
			
			case KEY_RIGHT:
			{
				switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?1:0))
				{
				case 3:  //ALT and CTRL
				case 2:  //ALT
					if(is_rect)
					{
						saved=false;
						go_slide_tiles(columns, rows, top, left);
						int32_t bitcheck = newtilebuf[((top)*TILES_PER_ROW)+left].format;
						bool same = true;
						
						for(int32_t c=0; c<columns; c++)
						{
							for(int32_t r=0; r<rows; r++)
							{
								int32_t t=((top+r)*TILES_PER_ROW)+left+c;
								
								if(newtilebuf[t].format!=bitcheck) same = false;
							}
						}
						
						if(!same) break;
						
						for(int32_t r=0; r<rows; r++)
						{
							for(int32_t c=columns-1; c>=0; c--)
							{
								int32_t temptile=((top+r)*TILES_PER_ROW)+left+c;
								byte *dest_pixelrow=(newtilebuf[temptile].data)+(128*bitcheck)-1;
								
								for(int32_t pixelrow=15; pixelrow>=0; pixelrow--)
								{
#ifdef ALLEGRO_LITTLE_ENDIAN
								
									//*dest_pixelrow=(*dest_pixelrow)<<4;
									for(int32_t p=0; p<(8*bitcheck)-1; p++)
									{
										if(bitcheck==tf4Bit)
										{
											*dest_pixelrow=*dest_pixelrow<<4;
											*dest_pixelrow|=(*(dest_pixelrow-1)>>4);
											
											if(p==6) *(dest_pixelrow-1)=*(dest_pixelrow-1)<<4;
										}
										else
										{
											*dest_pixelrow=*(dest_pixelrow-1);
										}
										
										dest_pixelrow--;
									}
									
#else
									
									for(int32_t p=0; p<(8*bitcheck)-1; p++)
									{
										if(bitcheck==tf4Bit)
										{
											*dest_pixelrow=*dest_pixelrow>>4;
											*dest_pixelrow|=(*(dest_pixelrow-1)<<4);
									
											if(p==6) *(dest_pixelrow-1)=*(dest_pixelrow-1)>>4;
										}
										else
										{
											*dest_pixelrow=*(dest_pixelrow-1);
										}
									
										dest_pixelrow--;
									}
									
#endif
									
									if(c==0)
									{
										if(!(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]))
										{
											byte *tempsrc=(newundotilebuf[(((top+r)*TILES_PER_ROW)+left+columns-1)].data+(pixelrow*8*bitcheck)+(8*bitcheck)-1);
#ifdef ALLEGRO_LITTLE_ENDIAN
											
											if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
											else *dest_pixelrow=*tempsrc;
											
#else
											
											if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
											else *dest_pixelrow=*tempsrc;
											
#endif
										}
									}
									else
									{
										byte *tempsrc=(newtilebuf[temptile-1].data+(pixelrow*8*bitcheck)+(8*bitcheck)-1);
#ifdef ALLEGRO_LITTLE_ENDIAN
										
										// (*dest_pixelrow)|=((*(dest_pixelrow-16))&0xF000000000000000ULL)>>60;
										if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc>>4;
										else *dest_pixelrow=*tempsrc;
										
#else
										
										if(bitcheck==tf4Bit) *dest_pixelrow|=*tempsrc<<4;
										else *dest_pixelrow=*tempsrc;
										
#endif
									}
									
									dest_pixelrow--;
								}
							}
						}
						
						register_blank_tiles();
						redraw=true;
					}
					
					break;
					
				case 1:  //CTRL
				case 0:  //None
					sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?(TILES_PER_ROW)-(tile%TILES_PER_ROW)-1:1);
					redraw=true;
					
				default: //Others
					break;
				}
			}
			break;
			
			case KEY_PGUP:
				sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-1*(TILEROW(tile)*TILES_PER_ROW):-TILES_PER_PAGE);
				redraw=true;
				break;
				
			case KEY_PGDN:
				sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?((TILE_PAGES*TILE_ROWS_PER_PAGE)-TILEROW(tile)-1)*TILES_PER_ROW:TILES_PER_PAGE);
				redraw=true;
				break;
				
			case KEY_HOME:
				sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?-(tile):-(tile%TILES_PER_PAGE));
				redraw=true;
				break;
				
			case KEY_END:
				sel_tile(tile,tile2,first,type,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])?(TILE_PAGES)*(TILES_PER_PAGE)-tile-1:(TILES_PER_PAGE)-(tile%TILES_PER_PAGE)-1);
				redraw=true;
				break;
				
			case KEY_P:
			{
				int32_t whatPage = gettilepagenumber("Goto Page", (PreFillTileEditorPage?(first/TILES_PER_PAGE):0));
				
				if(whatPage >= 0)
					sel_tile(tile,tile2,first,type,((whatPage-TILEPAGE(tile))*TILE_ROWS_PER_PAGE)*TILES_PER_ROW);
					
				break;
			}
			
			case KEY_O:
				if(type==0 && copy>=0)
				{
					go_tiles();
					
					if(key[KEY_LSHIFT] ||key[KEY_RSHIFT])
					{
						mass_overlay_tile(zc_min(tile,tile2),zc_max(tile,tile2),copy,cs,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]), rect_sel);
						saved=false;
					}
					else
					{
						saved = !overlay_tiles(tile,tile2,copy,copycnt,rect_sel,false,cs,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]));
						//overlay_tile(newtilebuf,tile,copy,cs,(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]));
					}
					
					saved=false;
					redraw=true;
				}
				
				break;
				
			case KEY_E:
				if(type==0)
				{
					edit_tile(tile,flip,cs);
					draw_tile_list_window();
					redraw=true;
				}
				
				break;
				
			case KEY_G:
				if(type==0)
				{
					grab_tile(tile,cs);
					draw_tile_list_window();
					redraw=true;
				}
				
				break;
				
			case KEY_C:
				copy=zc_min(tile,tile2);
				copycnt=abs(tile-tile2)+1;
				redraw=true;
				break;
				
			case KEY_X:
				if(type==2)
				{
					ex=(ex+1)%3;
				}
				
				break;
				
				//usetiles=true;
			case KEY_R:
				if(type==2)
					break;
					
				go_tiles();
				
				if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
				{
					bool go=false;
					if(key[KEY_LSHIFT] || key[KEY_RSHIFT])
						go=true;
					else if(massRecolorSetup(cs))
						go=true;
					
					if(go)
					{
						FOREACH_START_DMAPTILE(t)
							massRecolorApply(t);
						FOREACH_DMAPTILE_END
						
						register_blank_tiles();
					}
				}
				else
				{
					FOREACH_START_DMAPTILE(t)
						rotate_tile(t,(key[KEY_LSHIFT] || key[KEY_RSHIFT]));
					FOREACH_DMAPTILE_END
				}
				
				redraw=true;
				saved=false;
				break;
				
			case KEY_SPACE:
				rect_sel=!rect_sel;
				copy=-1;
				redraw=true;
				break;
				
				//     case KEY_N:     go_tiles(); normalize(tile,tile2,flip); flip=0; redraw=true; saved=false; usetiles=true; break;
			case KEY_H:
				flip^=1;
				go_tiles();
				
				if(type==0)
				{
					normalize(tile,tile2,rect_sel,flip);
					flip=0;
				}
				
				redraw=true;
				break;
				
			case KEY_V:
				if(copy==-1)
				{
					if(type!=2)
					{
						flip^=2;
						go_tiles();
						
						if(type==0)
						{
							normalize(tile,tile2,rect_sel,flip);
							flip=0;
						}
					}
				}
				else
				{
					bool alt=(key[KEY_ALT] || key[KEY_ALTGR]);
			go_tiles();
					saved = !copy_tiles(tile,tile2,copy,copycnt,rect_sel,false);
				}
				
				redraw=true;
				break;
				
		
		
		case KEY_F:
				if(copy==-1)
				{
					break;
				}
				else
				{
			go_tiles();
			{
				saved = !copy_tiles_floodfill(tile,tile2,copy,copycnt,rect_sel,false);
			}
				}
				
				redraw=true;
				break;
		
			case KEY_DEL:
				delete_tiles(tile,tile2,rect_sel);
				redraw=true;
				break;
				
			case KEY_U:
			{
				if(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL])
				{
					//Only toggle the first 2 bits!
					show_only_unused_tiles = (show_only_unused_tiles&~3) | (((show_only_unused_tiles&3)+1)%4);
				}
				else
				{
					comeback_tiles();
				}
				
				redraw=true;
			}
			break;
			
			case KEY_8:
			case KEY_8_PAD:
				hide_8bit_marker();
				break;
			
			case KEY_M:
			//al_trace("mass combo key pressed, type == %d\n",type);
				if(type==0)
				{
			 //al_trace("mass combo key pressed, copy == %d\n",copy);
					if((copy!=-1)&&(copy!=zc_min(tile,tile2)))
					{
						go_tiles();
						saved=!copy_tiles(tile,tile2,copy,copycnt,rect_sel,true);
					}
					else if(copy==-1)
					{
						// I don't know what this was supposed to be doing before.
						// It didn't work in anything like a sensible way.
						if(rect_sel)
			{
							make_combos_rect(top, left, rows, columns, cs);
			}
						else
			{
							make_combos(zc_min(tile, tile2), zc_max(tile, tile2), cs);
			}
					}
					
					redraw=true;
				}
				
				break;
				
			case KEY_D:
			{
				int32_t frames=1;
				char buf[80];
				sprintf(buf, "%d", frames);
				create_relational_tiles_dlg[0].dp2=lfont;
				create_relational_tiles_dlg[2].dp=buf;
				
				large_dialog(create_relational_tiles_dlg);
					
				int32_t ret=zc_popup_dialog(create_relational_tiles_dlg,2);
				
				if(ret==5)
				{
					frames=zc_max(atoi(buf),1);
					bool same = true;
					int32_t bitcheck=newtilebuf[tile].format;
					
					for(int32_t t=1; t<frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?6:19); ++t)
					{
						if(newtilebuf[tile+t].format!=bitcheck) same = false;
					}
					
					if(!same)
					{
						jwin_alert("Error","The source tiles are not","in the same format.",NULL,"&OK",NULL,13,27,lfont);
						break;
					}
					
					if(tile+(frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?48:96))>NEWMAXTILES)
					{
						jwin_alert("Error","Too many tiles will be created",NULL,NULL,"&OK",NULL,13,27,lfont);
						break;
					}
					
					for(int32_t i=frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?6:19); i<(frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?48:96)); ++i)
					{
						reset_tile(newtilebuf, tile+i, bitcheck);
					}
					
					if(create_relational_tiles_dlg[3].flags&D_SELECTED)
					{
						for(int32_t i=create_relational_tiles_dlg[3].flags&D_SELECTED?47:95; i>0; --i)
						{
							for(int32_t j=0; j<frames; ++j)
							{
								merge_tiles(tile+(i*frames)+j, (tile+(relational_template[i][0]*frames)+j)<<2, ((tile+(relational_template[i][1]*frames)+j)<<2)+1, ((tile+(relational_template[i][2]*frames)+j)<<2)+2, ((tile+(relational_template[i][3]*frames)+j)<<2)+3);
							}
						}
					}
					else
					{
						for(int32_t i=create_relational_tiles_dlg[3].flags&D_SELECTED?47:95; i>0; --i)
						{
							for(int32_t j=0; j<frames; ++j)
							{
								merge_tiles(tile+(i*frames)+j, (tile+(dungeon_carving_template[i][0]*frames)+j)<<2, ((tile+(dungeon_carving_template[i][1]*frames)+j)<<2)+1, ((tile+(dungeon_carving_template[i][2]*frames)+j)<<2)+2, ((tile+(dungeon_carving_template[i][3]*frames)+j)<<2)+3);
							}
						}
					}
				}
			}
			
			register_blank_tiles();
			register_used_tiles();
			redraw=true;
			saved=false;
			break;
			
			case KEY_B:
			{
				bool shift=(key[KEY_LSHIFT] || key[KEY_RSHIFT]);
				bool control=(key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL]);
				bool alt=(key[KEY_ALT] || key[KEY_ALTGR]);
				
				do_convert_tile(tile, tile2, cs, rect_sel, control, shift, alt);
				register_blank_tiles();
			}
			break;
			}
			clear_keybuf();
		}
		
		if(!(key[KEY_Z] || key[KEY_F12]))
			did_snap = false;
		
		if(gui_mouse_b()&1)
		{
			if(isinRect(gui_mouse_x(),gui_mouse_y(), w + 12 - 21, 5, w +12 - 21 + 15, 5 + 13))
			{
				if(do_x_button(screen, w+12 - 21, 5))
				{
					done=1;
				}
			}
			
			int32_t x=gui_mouse_x()-screen_xofs;
			int32_t y=gui_mouse_y()-screen_yofs;
			
			if(y>=0 && y<208*mul)
			{
				x=zc_min(zc_max(x,0),(320*mul)-1);
				int32_t t = (y>>5)*TILES_PER_ROW + (x>>5) + first;
				
				if(type==0 && (key[KEY_LSHIFT] || key[KEY_RSHIFT]))
				{
					tile2=t;
				}
				else
				{
					tile=tile2=t;
				}
				
				if(tile_clicked!=t)
				{
					dclick_status=DCLICK_NOT;
				}
				else if(dclick_status == DCLICK_AGAIN)
				{
					while(gui_mouse_b())
					{
						/* do nothing */
					}
					
					if(((y>>5)*TILES_PER_ROW + (x>>5) + first)!=t)
					{
						dclick_status=DCLICK_NOT;
					}
					else
					{
						if(type==0)
						{
							edit_tile(tile,flip,cs);
							draw_tile_list_window();
							redraw=true;
						}
						else
						{
							done=2;
						}
					}
				}
				
				tile_clicked=t;
			}
			else if(x>300*mul && !bdown)
			{
				if(y<224*mul && first>0)
				{
					first-=TILES_PER_PAGE;
					redraw=true;
				}
				
				if(y>=224*mul && first<TILES_PER_PAGE*(TILE_PAGES-1))
				{
					first+=TILES_PER_PAGE;
					redraw=true;
				}
				
				bdown=true;
			}
			
			if(type==1||type==2)
			{
				if(!bdown && isinRect(x,y,8*mul,216*mul+panel_yofs,23*mul,231*mul+panel_yofs))
					done=1;
					
				if(!bdown && isinRect(x,y,148*mul,216*mul+panel_yofs,163*mul,231*mul+panel_yofs))
					done=2;
			}
			else if(!bdown && isinRect(x,y,127*mul,216*mul+panel_yofs,(127+15)*mul,(216+15)*mul+panel_yofs))
			{
				rect_sel=!rect_sel;
				copy=-1;
				redraw=true;
			}
			else if(!bdown && isinRect(x,y,150*mul,213*mul+panel_yofs,(150+28)*mul,(213+21)*mul+panel_yofs))
			{
				FONT *tf = font;
				font = tfont;
				
				if(do_text_button(150*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"&Grab",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
				{
					font = tf;
					grab_tile(tile,cs);
					draw_tile_list_window();
					position_mouse_z(0);
					redraw=true;
				}
				
				font = tf;
			}
			else if(!bdown && isinRect(x,y,(150+28)*mul,213*mul+panel_yofs,(150+28*2)*mul,(213+21)*mul+panel_yofs+21))
			{
				FONT *tf = font;
				font = tfont;
				
				if(do_text_button((150+28)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"&Edit",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
				{
					font = tf;
					edit_tile(tile,flip,cs);
					draw_tile_list_window();
					redraw=true;
				}
				
				font = tf;
			}
			else if(!bdown && isinRect(x,y,(150+28*2)*mul,213*mul+panel_yofs,(150+28*3)*mul,(213+21)*mul+panel_yofs))
			{
				FONT *tf = font;
				font = tfont;
				
				if(do_text_button((150+28*2)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"Export",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
				{
					if(getname("Export Tile Page (.png)","png",NULL,datapath,false))
					{
						PALETTE temppal;
						get_palette(temppal);
						BITMAP *tempbmp=create_bitmap_ex(8,16*TILES_PER_ROW, 16*TILE_ROWS_PER_PAGE);
						draw_tiles(tempbmp,first,cs,f,false,true);
						save_bitmap(temppath, tempbmp, RAMpal);
						destroy_bitmap(tempbmp);
					}
				}
				
				font = tf;
			}
			else if(!bdown && isinRect(x,y,(150+28*3)*mul,213*mul+panel_yofs,(150+28*4)*mul,(213+21)*mul+panel_yofs))
			{
				FONT *tf = font;
				font = tfont;
				
				if(do_text_button((150+28*3)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"Recolor",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
				{
					if(massRecolorSetup(cs))
					{
						go_tiles();
						
						FOREACH_START_DMAPTILE(t)
							massRecolorApply(t);
						FOREACH_DMAPTILE_END
						
						register_blank_tiles();
					}
				}
				
				font = tf;
			}
			else if(!bdown && isinRect(x,y,(150+28*4)*mul,213*mul+panel_yofs,(150+28*5)*mul,(213+21)*mul+panel_yofs))
			{
				FONT *tf = font;
				font = tfont;
				
				if(do_text_button((150+28*4)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"Done",jwin_pal[jcBOXFG],jwin_pal[jcBOX],true))
				{
					done=1;
				}
				
				font = tf;
			}
			
			bdown=true;
		}
		
		bool r_click = false;
		
		if(gui_mouse_b()&2 && !bdown && type==0)
		{
			int32_t x=(gui_mouse_x()-screen_xofs);//&0xFF0;
			int32_t y=(gui_mouse_y()-screen_yofs);//&0xF0;
			
			if(y>=0 && y<208*mul)
			{
				x=zc_min(zc_max(x,0),(320*mul)-1);
				int32_t t = ((y)>>5)*TILES_PER_ROW + ((x)>>5) + first;
				
				if(t<zc_min(tile,tile2) || t>zc_max(tile,tile2))
					tile=tile2=t;
			}
			
			bdown = r_click = true;
			f=8;
		}
		
		if(gui_mouse_b()==0)
			bdown=false;
			
		position_mouse_z(0);
		
REDRAW_DMAP_SELTILE:

		if((f%16)==0 || InvalidStatic)
			redraw=true;
			
		if(redraw)
		{
			draw_tiles(first,cs,f);
		}
		if(f&8)
		{
			if(rect_sel)
			{
				for(int32_t i=zc_min(TILEROW(tile),TILEROW(tile2))*TILES_PER_ROW+
						  zc_min(TILECOL(tile),TILECOL(tile2));
						i<=zc_max(TILEROW(tile),TILEROW(tile2))*TILES_PER_ROW+
						zc_max(TILECOL(tile),TILECOL(tile2)); i++)
				{
					if(i>=first && i<first+TILES_PER_PAGE &&
							TILECOL(i)>=zc_min(TILECOL(tile),TILECOL(tile2)) &&
							TILECOL(i)<=zc_max(TILECOL(tile),TILECOL(tile2)))
					{
						int32_t x=(i%TILES_PER_ROW)<<5;
						int32_t y=((i-first)/TILES_PER_ROW)<<5;
						rect(screen2,x,y,x+(16*mul)-1,y+(16*mul)-1,vc(15));
					}
				}
			}
			else
			{
				for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
				{
					if(i>=first && i<first+TILES_PER_PAGE)
					{
						int32_t x=TILECOL(i)<<5;
						int32_t y=TILEROW(i-first)<<5;
						rect(screen2,x,y,x+(16*mul)-1,y+(16*mul)-1,vc(15));
					}
				}
			}
		}
		
		if(type==0)
			tile_info_0(tile,tile2,cs,copy,copycnt,first/TILES_PER_PAGE,rect_sel);
		else
			tile_info_1(otile,oflip,ocs,tile,flip,cs,copy,first/TILES_PER_PAGE, always_use_flip);
			
		if(type==2)
		{
			char cbuf[16];
			sprintf(cbuf, "E&xtend: %s",ex==2 ? "32x32" : ex==1 ? "32x16" : "16x16");
			gui_textout_ln(screen, lfont_l, (uint8_t *)cbuf, (235*mul)+screen_xofs, (212*mul)+screen_yofs+panel_yofs, jwin_pal[jcBOXFG],jwin_pal[jcBOX],0);
		}
		
		++f;
		
		if(r_click)
		{
			select_tile_rc_menu[1].flags = (copy==-1) ? D_DISABLED : 0;
			select_tile_rc_menu[2].flags = (copy==-1) ? D_DISABLED : 0;
			select_tile_view_menu[0].flags = HIDE_USED ? D_SELECTED : 0;
			select_tile_view_menu[1].flags = HIDE_UNUSED ? D_SELECTED : 0;
			select_tile_view_menu[2].flags = HIDE_BLANK ? D_SELECTED : 0;
			select_tile_view_menu[3].flags = HIDE_8BIT_MARKER ? D_SELECTED : 0;
			select_tile_rc_menu[7].flags = (type!=0) ? D_DISABLED : 0;
			int32_t m = popup_menu(select_tile_rc_menu,gui_mouse_x(),gui_mouse_y());
			redraw=true;
			
			switch(m)
			{
			case 0:
				copy=zc_min(tile,tile2);
				copycnt=abs(tile-tile2)+1;
				break;
				
			case 2:
			case 1:
			{
				bool b = copy_tiles(tile,tile2,copy,copycnt,rect_sel,(m==2));
				if(saved) saved = !b;
				break;
			}
			case 7:
			{
				bool b = scale_tiles(tile, tile2,cs);
				if(saved) saved = !b;
				break;
			}
				
			case 3:
				delete_tiles(tile,tile2,rect_sel);
				break;
				
			case 5:
				edit_tile(tile,flip,cs);
				draw_tile_list_window();
				break;
				
			case 8:
			{
				do_convert_tile(tile,tile2,cs,rect_sel,(newtilebuf[tile].format!=tf4Bit),false,false);
				break;
			}
			
			case 6:
				grab_tile(tile,cs);
				draw_tile_list_window();
				position_mouse_z(0);
				break;
				
			case 10:
				show_blank_tile(tile);
				break;
		
		case 13: //overlay
			overlay_tile(newtilebuf,tile,copy,cs,0);
			break;
		
		case 14: //h-flip
		{
		flip^=1;
		go_tiles();
		
		if(type==0)
		{
			normalize(tile,tile2,rect_sel,flip);
			flip=0;
		}
		
		redraw=true;   
		break;
			
		  }
		  
		  case 15: //h-flip
		  {
			if(copy==-1)
			{
				if(type!=2)
				{
				flip^=2;
				go_tiles();
				
				if(type==0)
				{
					normalize(tile,tile2,rect_sel,flip);
					flip=0;
				}
				}
			}
			else
			{
				go_tiles();
				saved=!copy_tiles(tile,tile2,copy,copycnt,rect_sel,false);
			}
			
			redraw=true;
		break;
			
		  }
			
		
			case 16: //mass combo
			{
				if(type==0)
				{
					//al_trace("mass combo key pressed, copy == %d\n",copy);
					if((copy!=-1)&&(copy!=zc_min(tile,tile2)))
					{
						go_tiles();
						saved=!copy_tiles(tile,tile2,copy,copycnt,rect_sel,true);
					}
					else if(copy==-1)
					{
						// I don't know what this was supposed to be doing before.
						// It didn't work in anything like a sensible way.
						if(rect_sel)
						{
							make_combos_rect(top, left, rows, columns, cs);
						}
						else
						{
							make_combos(zc_min(tile, tile2), zc_max(tile, tile2), cs);
						}
					}
					redraw=true;
				}
			}
			break;
				
			case 17:
				if(type==0)
				{
					bool warn = (rect_sel
						&& ((tile/20)!=(tile2/20))
						&& !(tile%20==0&&tile2%20==19));
					int32_t z=zc_min(tile,tile2);
					int32_t count = abs(tile-tile2) + 1;
					tile=z;
					tile2=NEWMAXTILES;
					copy = tile + count;
					copycnt = NEWMAXTILES-copy;
					
					if(key[KEY_LSHIFT]||key[KEY_RSHIFT]) //Remove
					{
						char buf[64];
						
						if(count>1)
							sprintf(buf,"Remove tiles %d - %d?",tile, copy-1);
						else
							sprintf(buf,"Remove tile %d?",tile);
							
						AlertDialog("Remove Tiles", std::string(buf)
							+"\nThis will offset the tiles that follow!"
							+(warn?"\nRemoving tiles ignores rectangular selections!":""),
							[&](bool ret,bool)
							{
								if(ret)
								{
									go_tiles();
									if(copy_tiles(tile,tile2,copy,copycnt,false,true))
									{
										redraw=true;
										saved=false;
									}
								}
							}).show();
					}
					else
					{
						char buf[64];
						
						if(count>1)
							sprintf(buf,"Insert %d blank tiles?",count);
						else
							sprintf(buf,"Insert a blank tile?");
							
						AlertDialog("Insert Tiles", std::string(buf)
							+"\nThis will offset the tiles that follow!"
							+(warn?"\nInserting tiles ignores rectangular selections!":""),
							[&](bool ret,bool)
							{
								if(ret)
								{
									go_tiles();
									if(copy_tiles(copy,tile2,tile,copycnt,false,true))
									{
										redraw=true;
										saved=false;
									}
								}
							}).show();
					}
					
					copy=-1;
					tile2=tile=z;
				}
				break;
				
			default:
				redraw=false;
				break;
			}
			
			r_click = false;
			goto REDRAW_DMAP_SELTILE;
		}
		
	}
	while(!done);
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	comeback();
	register_blank_tiles();
	register_used_tiles();
	setup_combo_animations();
	setup_combo_animations2();
	popup_zqdialog_end();
	popup_zqdialog_end_a5();
	return tile+1;
}


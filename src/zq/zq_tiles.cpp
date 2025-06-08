#include <cstring>
#include <cmath>

#include "base/files.h"
#include "base/qrs.h"
#include "base/dmap.h"
#include "base/cpool.h"
#include "base/autocombo.h"
#include "base/packfile.h"
#include "base/gui.h"
#include "base/combo.h"
#include "base/msgstr.h"
#include "base/zdefs.h"
#include "new_subscr.h"
#include "subscr.h"
#include "zq/zq_tiles.h"
#include "zq/zquest.h"
#include "tiles.h"
#include "zq/zq_misc.h"
#include "zq/zq_class.h"
#include "base/zsys.h"
#include "base/colors.h"
#include "base/qst.h"
#include "gui/jwin.h"
#include <base/new_menu.h>
#include "base/jwinfsel.h"
#include "hero_tiles.h"
#include "zq/questReport.h"
#include "dialog/info.h"
#include "dialog/scaletile.h"
#include "dialog/rotatetile.h"
#include "dialog/alert.h"
#include "drawing.h"
#include "colorname.h"
#include "zq/render.h"
#include "zinfo.h"
#include <fmt/format.h>
#include <functional>
#include "zq/moveinfo.h"
using std::set;


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
extern int32_t TilePgCursorCol, CmbPgCursorCol;

int32_t ex=0;
int32_t nextcombo_fake_click=0;
int32_t invcol=0;
int32_t tthighlight = 1;
int32_t showcolortip = 1;
int32_t show_quartgrid = 0, hide_grid = 0;

tiledata     *newundotilebuf;
std::vector<newcombo> undocombobuf;

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

bool do_movecombo(combo_move_data const& cmd, ComboMoveUndo& on_undo, bool is_undoing = false);
static optional<ComboMoveUndo> last_combo_move_list;
static optional<TileMoveUndo> last_tile_move_list;

int refl_flags = 0;
enum
{
	REFL_90CW, REFL_HFLIP,
	REFL_90CCW, REFL_VFLIP,
	REFL_180, REFL_DBLFLIP,
	REFL_MAX
};
const char *reflbtn_names[] =
{
	"90 CW", "HFlip",
	"90 CCW", "VFlip",
	"180 Rot", "Diag Flip"
};
int bgmode = 0, xmode = 0;
const char *bgmodebtn_names[] =
{
	"BG Color 0", "BG Trans."
};
const char *xmodebtn_names[] =
{
	"X", "No X"
};
enum
{
	XMODE_X, XMODE_NOX,
	XMODE_MAX
};
enum
{
	BGMODE_0, BGMODE_TRANSP,
	BGMODE_MAX
};

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
		int32_t ret=jwin_alert("Wrapping", buf, NULL, NULL, "&Yes", "&No", 'y', 'n', get_zc_font(font_lfont));
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
	last_tile_move_list = nullopt;
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
		}
		
		memcpy(newundotilebuf[i].data,newtilebuf[i].data,tilesize(newundotilebuf[i].format));
	}
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
			}
			
			memcpy(newundotilebuf[t].data,newtilebuf[t].data,tilesize(newundotilebuf[t].format));
		}
	}
}

void comeback_tiles()
{
	if(last_tile_move_list)
	{
		last_tile_move_list->undo();
		last_tile_move_list = nullopt;
	}
	for(dword i=0; i<NEWMAXTILES; ++i)
	{
		if(newtilebuf[i].format != newundotilebuf[i].format || !newtilebuf[i].data)
		{
			newtilebuf[i].format = newundotilebuf[i].format;
			
			if(newtilebuf[i].data!=NULL)
				free(newtilebuf[i].data);
			newtilebuf[i].data=(byte *)malloc(tilesize(newtilebuf[i].format));
			if(newtilebuf[i].data==NULL)
				Z_error_fatal("Unable to initialize tile #%ld.\n", i);
		}
		
		memcpy(newtilebuf[i].data,newundotilebuf[i].data,tilesize(newtilebuf[i].format));
	}
	
	register_blank_tiles();
	register_used_tiles();
}

void go_combos()
{
	if(nogocombos) return;
	last_combo_move_list = nullopt;

	undocombobuf = combobuf;
}

void comeback_combos()
{
	if(last_combo_move_list)
	{
		last_combo_move_list->undo();
		last_combo_move_list = nullopt;
	}

	combobuf = undocombobuf;
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
				vline(dest, x+((i-1)*scale2), y+((j-1)*scale2), y+(j*scale2), 255);
			}
			
			if(selection_grid[i][j-1]!=selection_grid[i][j])
			{
				hline(dest, x+((i-1)*scale2), y+((j-1)*scale2), x+(i*scale2), 255);
			}
		}
	}
	
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	//  selection_anchor=(selection_anchor+1)%64;
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
				
				vline(dest, x+((i-1)*scale2), y+((j-1)*scale2), y+(j*scale2), 255);
			}
			
			if(((i>=x1+1)&&(i<=x2+1))&&((j==y1+1)||(j==y2+2)))
			{
				if(selection_grid[i][j-1]!=selection_grid[i][j])
				{
					drawing_mode(DRAW_MODE_COPY_PATTERN, intersection_pattern, selection_anchor>>3, 0);
				}
				
				hline(dest, x+((i-1)*scale2), y+((j-1)*scale2), x+(i*scale2), 255);
			}
		}
	}
	
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
	//  selection_anchor=(selection_anchor+1)%64;
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

void zoomtile16(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t m)
{
	//  rectfill(dest,x,y,x+(16*m),y+(16*m),gridmode==gm_light?jwin_pal[jcMEDLT]:jwin_pal[jcDARK]);
	int gridcol = gridmode==gm_light?vc(7):vc(8);
	
	cset <<= 4;
	if(newtilebuf[tile].format>tf4Bit)
		cset=0;
	
	int g = hide_grid ? 1 : 0;
	byte transp_col = (bgmode == BGMODE_TRANSP ? jwin_pal[jcBOX] : 0+cset);
	rectfill(dest,x,y,x+(16*m)+g,y+(16*m)+g,transp_col);
	
	unpack_tile(newtilebuf, tile, 0, false);
	byte *si = unpackbuf;
	for(int32_t cy=0; cy<16; cy++)
	{
		for(int32_t cx=0; cx<16; cx++)
		{
			byte col = (floating_sel && floatsel[cx+(cy<<4)]) ? floatsel[cx+(cy<<4)] : *si;
			int32_t dx = ((flip&1)?15-cx:cx)*m;
			int32_t dy = ((flip&2)?15-cy:cy)*m;
			if(col)
				rectfill(dest,x+dx,y+dy,x+dx+m-1,y+dy+m-1,col+cset);
			
			if(!col && xmode == XMODE_X)
				little_x(dest,x+dx+m/4,y+dy+m/4,invcol,m/2);
			
			++si;
		}
	}
	
	if(!hide_grid)
	{
		for(int cx = 0; cx <= 16; ++cx)
			vline(dest,x+(cx*m),y,y+(16*m)-1,gridcol);
		for(int cy = 0; cy <= 16; ++cy)
			hline(dest,x,y+(cy*m),x+(16*m)-1,gridcol);
	}
	
	if(show_quartgrid)
	{
		int offs = (8*m);
		const int RAD = 3;
		rectfill(dest,x+offs-RAD,y,x+offs+RAD,y+(16*m),gridcol);
		rectfill(dest,x,y+offs-RAD,x+(16*m),y+offs+RAD,gridcol);
	}
	
	if(has_selection()||is_selecting())
	{
		selection_anchor=(selection_anchor+1)%64;
		
		if(has_selection()||is_selecting())
			draw_selection_outline(dest, x, y, m);
		
		if(is_selecting())
			draw_selecting_outline(dest, x, y, m);
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

void draw_icon_button(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,int icon,int32_t bg,int32_t fg,int32_t flags,bool jwin)
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
		jwin_draw_icon(dest,x+w/2,y+h/2,fg,icon,icon_proportion(icon,w,h),true);
	}
	else
	{
		jwin_draw_icon_button(dest, x, y, w, h, icon, flags, true);
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
				draw_layer_button(screen, x, y, w, h, text, flags^D_SELECTED);
				over=true;
				
				update_hw_screen();
			}
		}
		else
		{
			if(over)
			{
				vsync();
				draw_layer_button(screen, x, y, w, h, text, flags);
				over=false;
				
				update_hw_screen();
			}
		}
		rest(1);
	}
	
	if(over)
	{
		vsync();
		draw_layer_button(screen, x, y, w, h, text, toggleflag ? flags^D_SELECTED : flags);
		
		update_hw_screen();
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
				draw_text_button(screen,x,y,w,h,text,fg,bg,sel?0:D_SELECTED,jwin);
				over=true;
			}
		}
		else
		{
			if(over)
			{
				draw_text_button(screen,x,y,w,h,text,fg,bg,sel?D_SELECTED:0,jwin);
				over=false;
			}
		}
	}
	
	if(over)
	{
		custom_vsync();
		draw_text_button(screen,x,y,w,h,text,fg,bg,sel?0:D_SELECTED,jwin);
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
				draw_graphics_button(screen,x,y,w,h,bmp,bmp2,fg,bg,D_SELECTED,jwin,overlay);
				over=true;
			}
		}
		else
		{
			if(over)
			{
				draw_graphics_button(screen,x,y,w,h,bmp,bmp2,fg,bg,0,jwin,overlay);
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
				draw_graphics_button(screen,x,y,w,h,bmp,bmp2,fg,bg,D_SELECTED,jwin,overlay);
				over=true;
			}
		}
		else
		{
			if(over)
			{
				draw_graphics_button(screen,x,y,w,h,bmp,bmp2,fg,bg,0,jwin,overlay);
				over=false;
			}
		}
	}
	
	if(over)
	{
		custom_vsync();
		draw_graphics_button(screen,x,y,w,h,bmp,bmp2,fg,bg,0,jwin,overlay);
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
					draw_layerradio(dest,x,y,bg,fg,value);
					refresh(rMENU);
				}
			}
		}
	}
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
size_and_pos cpalette_8(648,416,16,14,16,18);
size_and_pos fg_prev(648,316,50,50);
size_and_pos bg_prev(648+30,316+30,50,50);
size_and_pos zoomtile(124,32,16,16,32,32);
size_and_pos prev_til_1(648,31,96,96);
size_and_pos prev_til_2(752,31,96,96);
size_and_pos prev_til_3(648,135,96,96);
size_and_pos prev_til_4(752,135,96,96);
size_and_pos ref_til(14,189,96,96);
size_and_pos status_info(648,308-(4*8),1,4,1,8);
size_and_pos hover_info(742,338-(3*8),1,3,1,8);
size_and_pos color_info(4,294,1,1,116,8);
size_and_pos color_info_btn(24,294,96,21);
size_and_pos tool_btns(22,29,2,4,39,39);
size_and_pos x_btn(890,5,15,13);
size_and_pos info_btn(872,5,15,13);
size_and_pos hidegrid_cbox(124,552,16,16);
size_and_pos quartgrid_cbox(124,572,16,16);
size_and_pos reflbtn_grid(124,610,2,3,71,21);
size_and_pos xmodebtn_grid(300,610,1,2,90,21);
size_and_pos bgmodebtn_grid(390,610,1,2,90,21);

int32_t c1=1;
int32_t c2=0;
int32_t floating_tile = -1;
int32_t tool = t_pen;
int32_t old_tool = -1;
int32_t tool_cur = -1;
int32_t select_mode = 0;
int32_t drawing=0;
int32_t reftile = 0;

bool qgrid_tool(int tool)
{
	switch(tool)
	{
		case t_pen:
		case t_fill:
		case t_recolor:
		case t_wand:
			return true;
	}
	return false;
}

void set_tool_sprite(int tool, int type)
{
	int spr = ZQM_NORMAL;
	switch(tool)
	{
		case t_pen: spr = ZQM_SWORD; break;
		case t_fill: spr = ZQM_POTION; break;
		case t_recolor: spr = ZQM_WAND; break;
		case t_eyedropper: spr = ZQM_LENS; break;
		case t_move: spr = ZQM_GLOVE_OPEN+type; break;
		case t_select: spr = ZQM_HOOK_PLAIN+type; break;
		case t_wand: spr = ZQM_SEL_WAND_PLAIN+type; break;
	}
	MouseSprite::set(spr);
}
void update_tool_cursor()
{
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
			set_tool_sprite(tool,type);
		
		tool_cur=tool;
	}
	else if(tool_cur != -1)
	{
		MouseSprite::set(ZQM_NORMAL);
		tool_cur = -1;
	}
}

void draw_edit_scr(int32_t tile,int32_t flip,int32_t cs,byte *oldtile,bool create_tbar)
{
	PALETTE tpal;
	static BITMAP *tbar = create_bitmap_ex(8,zq_screen_w-6, 18);
	static BITMAP *preview_bmp = create_bitmap_ex(8, 64, 64);
	jwin_draw_win(screen2, 0, 0, zq_screen_w, zq_screen_h, FR_WIN);
	
	if(!create_tbar)
	{
		blit(tbar, screen2, 0, 0, 3, 3, zq_screen_w-6, 18);
	}
	else
	{
		jwin_draw_titlebar(tbar, 0, 0, zq_screen_w-6, 18, "", true, true);
		blit(tbar, screen2, 0, 0, 3, 3, zq_screen_w-6, 18);
	}
	
	textprintf_ex(screen2,get_zc_font(font_lfont),5,5,jwin_pal[jcTITLEFG],-1,"Tile Editor (%d)",tile);
	
	clear_to_color(preview_bmp, 0);
	
	zc_swap(oldtile,newtilebuf[tile].data); //Put oldtile in the tile buffer
	jwin_draw_win(screen2, prev_til_1.x-2,prev_til_1.y-2, prev_til_1.w+4, prev_til_1.h+4, FR_DEEP);
	puttile16(preview_bmp,tile,0,0,cs,flip);
	stretch_blit(preview_bmp, screen2, 0, 0, 16, 16, prev_til_1.x, prev_til_1.y, prev_til_1.w, prev_til_1.h);
	
	clear_to_color(preview_bmp, 0);
	jwin_draw_win(screen2, prev_til_2.x-2,prev_til_2.y-2, prev_til_2.w+4, prev_til_2.h+4, FR_DEEP);
	overtile16(preview_bmp,tile,0,0,cs,flip);
	masked_stretch_blit(preview_bmp, screen2, 0, 0, 16, 16, prev_til_2.x, prev_til_2.y, prev_til_2.w, prev_til_2.h);
	zc_swap(oldtile,newtilebuf[tile].data); //Swap the real tile back to the buffer
	
	unpack_tile(newtilebuf, tile, 0, true);
	if(floating_sel)
		for(auto q = 0; q < 256; ++q)
			if(floatsel[q])
				unpackbuf[q] = floatsel[q];
	byte tmptile[256];
	byte *tmpptr = tmptile;
	zc_swap(tmpptr,newtilebuf[tile].data); //Put temp data in the tile buffer
	pack_tile(newtilebuf,unpackbuf,tile);
	clear_to_color(preview_bmp, 0);
	
	jwin_draw_win(screen2, prev_til_3.x-2,prev_til_3.y-2, prev_til_3.w+4, prev_til_3.h+4, FR_DEEP);
	puttile16(preview_bmp,tile,0,0,cs,flip);
	stretch_blit(preview_bmp, screen2, 0, 0, 16, 16, prev_til_3.x, prev_til_3.y, prev_til_3.w, prev_til_3.h);
	
	clear_to_color(preview_bmp, 0);
	jwin_draw_win(screen2, prev_til_4.x-2,prev_til_4.y-2, prev_til_4.w+4, prev_til_4.h+4, FR_DEEP);
	overtile16(preview_bmp,tile,0,0,cs,flip);
	masked_stretch_blit(preview_bmp, screen2, 0, 0, 16, 16, prev_til_4.x, prev_til_4.y, prev_til_4.w, prev_til_4.h);
	
	if(reftile > 0)
	{
		clear_to_color(preview_bmp, 0);
		jwin_draw_win(screen2, ref_til.x-2,ref_til.y-2, ref_til.w+4, ref_til.h+4, FR_DEEP);
		overtile16(preview_bmp,reftile,0,0,cs,flip);
		masked_stretch_blit(preview_bmp, screen2, 0, 0, 16, 16, ref_til.x, ref_til.y, ref_til.w, ref_til.h);
	}
	
	//Color info
	{
		color_info.h = 1;
		if(showcolortip)
		{
			auto fh = color_info.yscale = text_height(font);
			int ty = color_info.y;
			if(reftile <= 0)
				ty -= ref_til.h + 8;
			int y = ty;
			int rx = color_info.x+color_info.xscale;
			gui_textout_ln(screen2,font,(unsigned char*)"Colors:",
				rx,y,jwin_pal[jcBOXFG],jwin_pal[jcBOX],2);
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
				if(text_length(font,buf) > color_info.xscale)
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
				gui_textout_ln(screen2,font,(unsigned char const*)buf,
					rx,y,jwin_pal[jcBOXFG],jwin_pal[jcBOX],2);
				++color_info.h;
			}
			jwin_draw_frame(screen2,color_info.x-2,ty-2,(color_info.w*color_info.xscale)+4,(color_info.h*color_info.yscale)+4,FR_DEEP);
		}
		else
		{
			int ty = color_info_btn.y;
			if(reftile <= 0)
				ty -= ref_til.h + 8;
			draw_text_button(screen2,color_info_btn.x,ty,color_info_btn.w,color_info_btn.h,
				"Show Colors",vc(1),vc(14),0,true);
		}
	}
	
	zc_swap(tmpptr,newtilebuf[tile].data); //Swap the real tile back to the buffer
	
	jwin_draw_win(screen2, zoomtile.x-3, zoomtile.y-3, (zoomtile.w*zoomtile.xscale)+5, (zoomtile.h*zoomtile.yscale)+5, FR_DEEP);
	zoomtile16(screen2,tile,zoomtile.x-1,zoomtile.y-1,cs,flip,zoomtile.xscale);
	
	if(floating_sel)
		textprintf_ex(screen2,font,status_info.x,status_info.y+0,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Floating selection");
	textprintf_ex(screen2,font,status_info.x,status_info.y+(1*status_info.yscale),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"tile: %d",tile);
	if(newtilebuf[tile].format==tf8Bit)
		textprintf_ex(screen2,font,status_info.x,status_info.y+(2*status_info.yscale),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"8-bit");
	else
		textprintf_ex(screen2,font,status_info.x,status_info.y+(2*status_info.yscale),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"cset: %d",cs);
	
	PALETTE temppal;
	
	//palette and mouse
	switch(newtilebuf[tile].format)
	{
		case tf4Bit:
			jwin_draw_win(screen2, cpalette_4.x-2, cpalette_4.y-2, (cpalette_4.xscale*cpalette_4.w)+4, (cpalette_4.yscale*cpalette_4.h)+4, FR_DEEP);
			get_palette(temppal);
			
			for(int32_t i=0; i<cpalette_4.w*cpalette_4.h; i++)
			{
				size_and_pos const& s = cpalette_4.subsquare(i);
				rectfill(screen2,s.x,s.y,s.x+s.w-1,s.y+s.h-1,CSET(cs)+i);
			}
			
			little_x(screen2,cpalette_4.x+1,cpalette_4.y+1,invcol,cpalette_4.xscale-5,cpalette_4.yscale-5);
			break;
			
		case tf8Bit:
			jwin_draw_win(screen2, cpalette_8.x-2, cpalette_8.y-2, (cpalette_8.xscale*cpalette_8.w)+4, (cpalette_8.yscale*cpalette_8.h)+4, FR_DEEP);
			
			for(int32_t i=0; i<cpalette_8.w*cpalette_8.h; ++i)
			{
				size_and_pos const& s = cpalette_8.subsquare(i);
				rectfill(screen2,s.x,s.y,s.x+s.w-1,s.y+s.h-1,i);
			}
			
			little_x(screen2,cpalette_8.x+1,cpalette_8.y+1,invcol,cpalette_8.xscale-5,cpalette_8.yscale-5);
			break;
	}
	
	rect(screen2, bg_prev.x, bg_prev.y, bg_prev.x+bg_prev.w-1, bg_prev.y+bg_prev.h-1, jwin_pal[jcTEXTFG]);
	rectfill(screen2, bg_prev.x+1, bg_prev.y+1, bg_prev.x+bg_prev.w-2, bg_prev.y+bg_prev.h-2, jwin_pal[jcTEXTBG]);
	rectfill(screen2, bg_prev.x+3, bg_prev.y+3, bg_prev.x+bg_prev.w-4, bg_prev.y+bg_prev.h-4, c2+((newtilebuf[tile].format==tf4Bit)?CSET(cs):0));
	
	if(c2==0)
	{
		little_x(screen2, bg_prev.x+1, bg_prev.y+1, invcol, bg_prev.w-2, bg_prev.h-2);
	}
	
	rect(screen2, fg_prev.x, fg_prev.y, fg_prev.x+fg_prev.w-1, fg_prev.y+fg_prev.h-1, jwin_pal[jcTEXTFG]);
	rectfill(screen2, fg_prev.x+1, fg_prev.y+1, fg_prev.x+fg_prev.w-2, fg_prev.y+fg_prev.h-2, jwin_pal[jcTEXTBG]);
	rectfill(screen2, fg_prev.x+3, fg_prev.y+3, fg_prev.x+fg_prev.w-4, fg_prev.y+fg_prev.h-4, c1+((newtilebuf[tile].format==tf4Bit)?CSET(cs):0));
	
	if(c1==0)
	{
		little_x(screen2, fg_prev.x+1, fg_prev.y+1, invcol, fg_prev.w-2, fg_prev.h-2);
	}
	
	draw_text_button(screen2,ok_button.x,ok_button.y,ok_button.w,ok_button.h,"OK",vc(1),vc(14),0,true);
	draw_text_button(screen2,cancel_button.x,cancel_button.y,cancel_button.w,cancel_button.h,"Cancel",vc(1),vc(14),0,true);
	draw_text_button(screen2,edit_button.x,edit_button.y,edit_button.w,edit_button.h,"Edit Pal",vc(1),vc(14),0,true);
	draw_checkbox(screen2,hlcbox.x, hlcbox.y, hlcbox.w, hlcbox.h, tthighlight);
	gui_textout_ln(screen2,font,(unsigned char*)"Highlight Hover",hlcbox.x+hlcbox.w+2,hlcbox.y+hlcbox.h/2-text_height(font)/2,jwin_pal[jcBOXFG],jwin_pal[jcBOX],0);
	
	draw_checkbox(screen2,quartgrid_cbox.x, quartgrid_cbox.y, quartgrid_cbox.w, quartgrid_cbox.h, show_quartgrid);
	gui_textout_ln(screen2,font,(unsigned char*)"Quarter Grid",quartgrid_cbox.x+quartgrid_cbox.w+2,quartgrid_cbox.y+quartgrid_cbox.h/2-text_height(font)/2,jwin_pal[jcBOXFG],jwin_pal[jcBOX],0);
	
	draw_checkbox(screen2,hidegrid_cbox.x, hidegrid_cbox.y, hidegrid_cbox.w, hidegrid_cbox.h, hide_grid);
	gui_textout_ln(screen2,font,(unsigned char*)"Hide Grid",hidegrid_cbox.x+hidegrid_cbox.w+2,hidegrid_cbox.y+hidegrid_cbox.h/2-text_height(font)/2,jwin_pal[jcBOXFG],jwin_pal[jcBOX],0);
	
	bool qgrd = show_quartgrid && qgrid_tool(tool);
	gui_textout_ln(screen2,font,(unsigned char*)"Quarter-Grid Draw Modes", reflbtn_grid.x, reflbtn_grid.y-text_height(font)-4,jwin_pal[jcBOXFG],jwin_pal[jcBOX],qgrd?0:D_DISABLED);
	for(int q = 0; q < REFL_MAX; ++q)
	{
		auto& sqr = reflbtn_grid.subsquare(q);
		draw_text_button(screen2,sqr.x,sqr.y,sqr.w,sqr.h,reflbtn_names[q],vc(1),vc(14),qgrd ? ((refl_flags&(1<<q)) ? D_SELECTED : 0) : D_DISABLED,true);
	}
	gui_textout_ln(screen2,font,(unsigned char*)"Transparent Mode", xmodebtn_grid.x, xmodebtn_grid.y-text_height(font)-4,jwin_pal[jcBOXFG],jwin_pal[jcBOX],0);
	for(int q = 0; q < XMODE_MAX; ++q)
	{
		auto& sqr = xmodebtn_grid.subsquare(q);
		draw_text_button(screen2,sqr.x,sqr.y,sqr.w,sqr.h,xmodebtn_names[q],vc(1),vc(14),(xmode == q) ? D_SELECTED : 0,true);
	}
	for(int q = 0; q < BGMODE_MAX; ++q)
	{
		auto& sqr = bgmodebtn_grid.subsquare(q);
		draw_text_button(screen2,sqr.x,sqr.y,sqr.w,sqr.h,bgmodebtn_names[q],vc(1),vc(14),(bgmode == q) ? D_SELECTED : 0,true);
	}
	
	//tool buttons
	for(int32_t toolbtn = 0; toolbtn < t_max; ++toolbtn)
	{
		auto bmp = toolbtn+MOUSE_BMP_SWORD;
		int col = toolbtn%tool_btns.w;
		int row = toolbtn/tool_btns.w;
		
		jwin_draw_button(screen2,tool_btns.x+(col*tool_btns.xscale),tool_btns.y+(row*tool_btns.yscale),tool_btns.xscale,tool_btns.yscale,tool==toolbtn?2:0,0);
		masked_stretch_blit(mouse_bmp_1x[bmp][0],screen2,0,0,16,16,tool_btns.x+(col*tool_btns.xscale)+3+(tool==toolbtn?1:0),tool_btns.y+3+(row*tool_btns.yscale)+(tool==toolbtn?1:0),tool_btns.xscale-7,tool_btns.yscale-7);
	}
	
	//coordinates
	{
		auto mx = gui_mouse_x();
		auto my = gui_mouse_y();
		int32_t ind = zoomtile.rectind(mx,my);
		int32_t temp_x=ind%zoomtile.w;
		int32_t temp_y=ind/zoomtile.w;
		int color = -1;
		
		bool is8b = newtilebuf[tile].format > tf4Bit;
		if(ind > -1)
		{
			char xbuf[16];
			sprintf(xbuf, "x: %d", temp_x);
			textprintf_ex(screen2,font,status_info.x,status_info.y+(3*status_info.yscale),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%s",xbuf);
			textprintf_ex(screen2,font,status_info.x+text_length(font,xbuf)+8,status_info.y+(3*status_info.yscale),jwin_pal[jcBOXFG],jwin_pal[jcBOX],"y: %d",temp_y);
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
			bool xcolor = (is8b ? realcol == 0 : (realcol%16)==0);
			auto& c = tpal[realcol];
			
			if(tthighlight)
			{
				size_and_pos const& mainsqr = is8b ? cpalette_8 : cpalette_4;
				size_and_pos const& csqr = mainsqr.subsquare(color);
				
				int hlcol = getHighlightColor(tpal[realcol]);
				int hlthick = 4;
				int extraborder = is8b ? 8 : 0;
				int borderthick = hlthick+extraborder;
				
				if(is8b)
				{
					highlight_sqr(screen2, 0xED, csqr.x, mainsqr.y, csqr.w, mainsqr.h*mainsqr.yscale, hlthick); //column
					highlight_sqr(screen2, 0xED, mainsqr.x, csqr.y, mainsqr.w*mainsqr.xscale, csqr.h, hlthick); //row
				}
				highlight_sqr(screen2, 0xED, csqr.x-borderthick, csqr.y-borderthick, csqr.w+borderthick*2, csqr.h+borderthick*2, hlthick); //square hl
				rectfill(screen2, csqr.x-extraborder, csqr.y-extraborder, csqr.x+csqr.w-1+extraborder, csqr.y+csqr.h-1+extraborder, realcol); //square color
				if(xcolor)
					little_x(screen2, csqr.x-extraborder+4, csqr.y-extraborder+4, invcol, csqr.w+(extraborder*2)-8, csqr.h+(extraborder*2)-8); //transparent X
				highlight_sqr(screen2, hlcol, csqr.x-extraborder, csqr.y-extraborder, csqr.w+extraborder*2, csqr.h+extraborder*2, 1); //highlight border
			}
			
			sprintf(buf, "%02d %02d %02d %c(0x%02X %d)",c.r,c.g,c.b,separator,realcol,color);
			gui_textout_ln(screen2,font,(unsigned char*)buf,hover_info.x,hover_info.y+(2*hover_info.yscale),jwin_pal[jcBOXFG],jwin_pal[jcBOX],0);
			
			strcpy(buf, get_color_name(realcol, is8b).c_str());
			gui_textout_ln(screen2,font,(unsigned char*)buf,hover_info.x,hover_info.y+(1*hover_info.yscale),jwin_pal[jcBOXFG],jwin_pal[jcBOX],0);
			
			sprintf(buf, "#%02X%02X%02X", tpal[realcol].r,tpal[realcol].g,tpal[realcol].b);
			gui_textout_ln(screen2,font,(unsigned char*)buf,hover_info.x,hover_info.y+(0),jwin_pal[jcBOXFG],jwin_pal[jcBOX],0);
			
			rect(screen2, hov_prev.x, hov_prev.y, hov_prev.x+hov_prev.w-1, hov_prev.y+hov_prev.h-1, jwin_pal[jcTEXTFG]);
			rectfill(screen2, hov_prev.x+1, hov_prev.y+1, hov_prev.x+hov_prev.w-2, hov_prev.y+hov_prev.h-2, jwin_pal[jcTEXTBG]);
			rectfill(screen2, hov_prev.x+3, hov_prev.y+3, hov_prev.x+hov_prev.w-4, hov_prev.y+hov_prev.h-4, realcol);
			if(xcolor)
				little_x(screen2, hov_prev.x+1, hov_prev.y+1, invcol, hov_prev.w-2, hov_prev.h-2);
		}
	}
	
	custom_vsync();
	blit(screen2,screen,0,0,0,0,zq_screen_w,zq_screen_w);
	update_tool_cursor();
	SCRFIX();
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
			unpack_tile(newtilebuf, temptile, 0, true);
			
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
	unpack_tile(newtilebuf, tile, 0, true);
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
			floatsel[q]=wrap(floatsel[q]+amount, 0, newtilebuf[tile].format==tf8Bit ? 0xDF : 0xF);
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
				
			buf[i]=wrap(buf[i]+amount, 1, newtilebuf[tile].format==tf8Bit ? 0xDF : 0xF);
		}
		else // Don't ignore transparent
			buf[i]=wrap(buf[i]+amount, 0, newtilebuf[tile].format==tf8Bit ? 0xDF : 0xF);
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

static int move_origin_x=-1, move_origin_y=-1;
static int prev_x=-1, prev_y=-1;
bool __pixel_draw(int x, int y, int tile, int flip)
{
	bool ret = false;
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
				ret = true;
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
					if(tf_u != ((drawing==1)?c1:c2))
					{
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
				}
				ret = true;
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
			
			ret = true;
			break;
	}
	return ret;
}
void edit_tile(int32_t tile,int32_t flip,int32_t &cs)
{
	popup_zqdialog_start();
	FONT* oldfont = font;
	font = get_custom_font(CFONT_DLG);
	edit_button.h = ok_button.h = cancel_button.h = 12+text_height(font);
	status_info.yscale = text_height(font);
	status_info.y = 308-(status_info.h*status_info.yscale);
	hover_info.yscale = status_info.yscale;
	hover_info.y = 338-(hover_info.h*hover_info.yscale);
	go();
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
	
	if(newtilebuf[tile].format==tf4Bit)
	{
		invcol=makecol8(255 - tpal[CSET(cs)].r, 255 - tpal[CSET(cs)].g, 255 - tpal[CSET(cs)].b);
	}
	else
	{
		invcol=makecol8(255 - tpal[0].r, 255 - tpal[0].g, 255 - tpal[0].b);
	}
	
	custom_vsync();
	zc_set_palette(tpal);
	draw_edit_scr(tile,flip,cs,oldtile, true);
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	move_origin_x=-1;
	move_origin_y=-1;
	prev_x=-1;
	prev_y=-1;
	
	
	
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
		HANDLE_CLOSE_ZQDLG();
		if(exiting_program) break;
		int32_t temp_mouse_x=gui_mouse_x();
		int32_t temp_mouse_y=gui_mouse_y();
		rest(4);
		bool redraw=false;
		bool did_wand_select=false;
		
		if(keypressed())
		{
			bool ctrl = CHECK_CTRL_CMD;
			int k = readkey()>>8;
			switch(k)
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
					else if(has_selection())
						clear_selection_grid();
					else done=1;
					break;
				
				case KEY_DEL:
				{
					unpack_tile(newtilebuf, tile, 0, false);
					bool all = CHECK_CTRL_CMD || !has_selection();
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
					
					if(CHECK_CTRL_CMD || !has_selection())
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
					redraw=true;
				}
				break;
					
				case KEY_A:
					clear_selection_grid();
					invert_selection_grid();
					redraw=true;
					break;
					
				case KEY_D:
					clear_selection_grid();
					redraw=true;
					break;
					
				case KEY_I:
					invert_selection_grid();
					redraw=true;
					break;
					
				case KEY_H:
					flip^=1;
					normalize(tile,tile,0,flip);
					flip=0;
					redraw=true;
					break;
					
				case KEY_V:
					flip^=2;
					normalize(tile,tile,0,flip);
					flip=0;
					redraw=true;
					break;
					
				case KEY_F12:
					onSnapshot();
					break;
					
				case KEY_R:
				{
					//if(CHECK_CTRL_CMD))
					// {
					//do_recolor(tile); redraw=true; saved=false;
					// }
					//else
					// {
					go_tiles();
					rotate_tile(tile,(key[KEY_LSHIFT] || key[KEY_RSHIFT]));
					redraw=true;
					saved=false;
					break;
				}
				
				case KEY_EQUALS:
				case KEY_PLUS_PAD:
				{
					if(CHECK_CTRL_CMD ||
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
						
					redraw=true;
					break;
				}
				
				case KEY_MINUS:
				case KEY_MINUS_PAD:
				{
					if(CHECK_CTRL_CMD ||
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
						
					redraw=true;
					break;
				}
				
				case KEY_SPACE:
					gridmode=(gridmode+1)%gm_max;
					redraw=true;
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
					
					redraw=true;
					break;
					
				case KEY_S:
					if(CHECK_CTRL_CMD)
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
					redraw=true;
					break;
					
				case KEY_UP:
					if(CHECK_CTRL_CMD)
					{
						unfloat_selection();
						tile=zc_max(0,tile-TILES_PER_ROW);
						undocount = tilesize(newtilebuf[tile].format);
						
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
							oldtile[i]=undotile[i];
						}
						
						redraw=true;
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
						redraw=true;
					}
					break;
					
				case KEY_DOWN:
					if(CHECK_CTRL_CMD)
					{
						unfloat_selection();
						tile=zc_min(tile+TILES_PER_ROW,NEWMAXTILES-1);
						undocount = tilesize(newtilebuf[tile].format);
						
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
							oldtile[i]=undotile[i];
						}
						
						redraw=true;
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
						redraw=true;
					}
					break;
					
				case KEY_LEFT:
					if(CHECK_CTRL_CMD)
					{
						unfloat_selection();
						tile=zc_max(0,tile-1);
						undocount = tilesize(newtilebuf[tile].format);
						
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
							oldtile[i]=undotile[i];
						}
						
						redraw=true;
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
						redraw=true;
					}
					break;
					
				case KEY_RIGHT:
					if(CHECK_CTRL_CMD)
					{
						unfloat_selection();
						tile=zc_min(tile+1, NEWMAXTILES-1);
						undocount = tilesize(newtilebuf[tile].format);
						
						for(int32_t i=0; i<undocount; i++)
						{
							undotile[i]=newtilebuf[tile].data[i];
							oldtile[i]=undotile[i];
						}
						
						redraw=true;
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
						redraw=true;
					}
					break;
				case KEY_0: case KEY_1: case KEY_2: case KEY_3:
				case KEY_4: case KEY_5: case KEY_6: case KEY_7:
				case KEY_8: case KEY_9:
				case KEY_0_PAD: case KEY_1_PAD: case KEY_2_PAD: case KEY_3_PAD:
				case KEY_4_PAD: case KEY_5_PAD: case KEY_6_PAD: case KEY_7_PAD:
				case KEY_8_PAD: case KEY_9_PAD:
				{
					int t = k - ((k>KEY_9) ? KEY_1_PAD : KEY_1);
					if(unsigned(t) < t_max)
					{
						if(old_tool != -1)
							old_tool = t;
						else tool = t;
					}
					break;
				}
				case KEY_TAB:
				{
					if(key_shifts & KB_CTRL_CMD_FLAG)
					{
						xmode = (xmode+1)%XMODE_MAX;
						if(!xmode)
							bgmode = (bgmode+1)%BGMODE_MAX;
					}
					else if(key_shifts & KB_SHIFT_FLAG)
						hide_grid = !hide_grid;
					else show_quartgrid = !show_quartgrid;
					redraw = true;
					break;
				}
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
		bool ctrl=CHECK_CTRL_CMD;
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
						set_tool_sprite(tool,type);
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
			redraw = true;
		}
		if(last_tool_val != tool)
		{
			redraw = true;
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
					set_tool_sprite(tool,1);
					
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
				if(do_text_button(ok_button.x,ok_button.y,ok_button.w,ok_button.h,"OK"))
				{
					done=2;
				}
			}
			
			if(cancel_button.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_text_button(cancel_button.x,cancel_button.y,cancel_button.w,cancel_button.h,"Cancel"))
				{
					done=1;
				}
			}
			
			if(edit_button.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_text_button(edit_button.x,edit_button.y,edit_button.w,edit_button.h,"Edit Pal"))
				{
					colors_menu.pop(edit_button.x+2,edit_button.y-40);
					get_palette(tpal);
					
					if(newtilebuf[tile].format==tf4Bit)
					{
						invcol=makecol8(255 - tpal[CSET(cs)].r, 255 - tpal[CSET(cs)].g, 255 - tpal[CSET(cs)].b);
					}
					else
					{
						invcol=makecol8(255 - tpal[0].r, 255 - tpal[0].g, 255 - tpal[0].b);
					}
					
					redraw=true;
				}
			}
			
			int sqr_clicked;
			if(show_quartgrid && qgrid_tool(tool))
			{
				sqr_clicked = reflbtn_grid.rectind(temp_mouse_x,temp_mouse_y);
				if(sqr_clicked > -1)
				{
					auto& sqr = reflbtn_grid.subsquare(sqr_clicked);
					if(do_text_button(sqr.x,sqr.y,sqr.w,sqr.h,reflbtn_names[sqr_clicked]))
						refl_flags ^= (1<<sqr_clicked);
				}
			}
			sqr_clicked = xmodebtn_grid.rectind(temp_mouse_x,temp_mouse_y);
			if(sqr_clicked > -1)
			{
				auto& sqr = xmodebtn_grid.subsquare(sqr_clicked);
				if(do_text_button(sqr.x,sqr.y,sqr.w,sqr.h,xmodebtn_names[sqr_clicked]))
					xmode = sqr_clicked;
			}
			sqr_clicked = bgmodebtn_grid.rectind(temp_mouse_x,temp_mouse_y);
			if(sqr_clicked > -1)
			{
				auto& sqr = bgmodebtn_grid.subsquare(sqr_clicked);
				if(do_text_button(sqr.x,sqr.y,sqr.w,sqr.h,bgmodebtn_names[sqr_clicked]))
					bgmode = sqr_clicked;
			}
			
			if(showcolortip)
			{
				auto oy = color_info.y;
				if(reftile <= 0)
					color_info.y -= ref_til.h + 8;
				if(color_info.rect(temp_mouse_x,temp_mouse_y))
				{
					showcolortip = 0;
					zc_set_config("ZQ_GUI","tile_edit_colornames",0);
				}
				color_info.y = oy;
			}
			else
			{
				auto oy = color_info_btn.y;
				if(reftile <= 0)
					color_info_btn.y -= ref_til.h + 8;
				if(color_info_btn.rect(temp_mouse_x,temp_mouse_y))
				{
					if(do_text_button(color_info_btn.x,color_info_btn.y,color_info_btn.w,color_info_btn.h,"Show Colors"))
					{
						showcolortip = 1;
						zc_set_config("ZQ_GUI","tile_edit_colornames",1);
						redraw=true;
					}
				}
				color_info_btn.y = oy;
			}
			
			if(hlcbox.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_checkbox(screen2,hlcbox.x,hlcbox.y,hlcbox.w,hlcbox.h,tthighlight))
				{
					zc_set_config("ZQ_GUI","tile_edit_fancyhighlight",tthighlight);
					redraw=true;
				}
			}
			if(quartgrid_cbox.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_checkbox(screen2,quartgrid_cbox.x,quartgrid_cbox.y,quartgrid_cbox.w,quartgrid_cbox.h,show_quartgrid))
					redraw=true;
			}
			if(hidegrid_cbox.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_checkbox(screen2,hidegrid_cbox.x,hidegrid_cbox.y,hidegrid_cbox.w,hidegrid_cbox.h,hide_grid))
					redraw=true;
			}
			
			switch(newtilebuf[tile].format)
			{
				case tf4Bit:
				{
					auto ind = cpalette_4.rectind(temp_mouse_x,temp_mouse_y);
					if(ind > -1)
					{
						c1 = ind;
						redraw=true;
					}
					break;
				}
				case tf8Bit:
				{
					auto ind = cpalette_8.rectind(temp_mouse_x,temp_mouse_y);
					if(ind > -1)
					{
						c1 = ind;
						redraw=true;
					}
					break;
				}
			}
			
			
			int32_t newtool = tool_btns.rectind(temp_mouse_x,temp_mouse_y);
			if(newtool > -1 && newtool < t_max)
			{
				tool=newtool;
				redraw=true;
			}
			
			if(x_btn.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_x_button(screen, x_btn.x, x_btn.y))
				{
					done=1;
				}
			}
			if(info_btn.rect(temp_mouse_x,temp_mouse_y))
			{
				if(do_question_button(screen, info_btn.x, info_btn.y))
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
					set_tool_sprite(tool,1);
					
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
						redraw=true;
					}
					break;
				}
				case tf8Bit:
				{
					auto ind = cpalette_8.rectind(temp_mouse_x,temp_mouse_y);
					if(ind > -1)
					{
						c2 = ind;
						redraw=true;
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
					set_tool_sprite(tool,0);
				}
			}
		}
		
		if(drawing && zoomtile.rect(temp_mouse_x,temp_mouse_y)) //inside the zoomed tile window
		{
			int32_t ind = zoomtile.rectind(temp_mouse_x,temp_mouse_y);
			int32_t x=ind%zoomtile.w;
			int32_t y=ind/zoomtile.w;
			
			bool reset_draw = false;
			
			if(__pixel_draw(x,y,tile,flip))
				reset_draw = true;
			if(show_quartgrid)
			{
				auto tmp_sel_mode = select_mode;
				if(tool == t_wand && select_mode == 0)
					select_mode = 1;
				if(qgrid_tool(tool))
				{
					if(refl_flags & (1<<REFL_HFLIP))
						if(__pixel_draw(15-x,y,tile,flip))
							reset_draw = true;
					if(refl_flags & (1<<REFL_VFLIP))
						if(__pixel_draw(x,15-y,tile,flip))
							reset_draw = true;
					//Diagonal flip and 180° rotation are the same!
					if(refl_flags & ((1<<REFL_DBLFLIP)|(1<<REFL_180)))
						if(__pixel_draw(15-x,15-y,tile,flip))
							reset_draw = true;
					if(refl_flags & (1<<REFL_90CW))
						if(__pixel_draw(15-y,x,tile,flip))
							reset_draw = true;
					if(refl_flags & (1<<REFL_90CCW))
						if(__pixel_draw(y,15-x,tile,flip))
							reset_draw = true;
				}
				select_mode = tmp_sel_mode;
			}
			
			if(reset_draw)
				drawing = 0;
			redraw=true;
		}
		
		if(gui_mouse_b()==0)
		{
			bdown=false;
			drawing=0;
		}
		
		temp_x=(gui_mouse_x()-zoomtile.x)/zoomtile.xscale;
		temp_y=(gui_mouse_y()-zoomtile.y)/zoomtile.yscale;
		
		{
			tile_x=temp_x;
			tile_y=temp_y;
			redraw=true;
		}
		
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
			redraw=true;
		}
		/* Highlight the hovered pixel? Eh, maybe too much?
		int32_t hov_pix = zoomtile.rectind(temp_mouse_x,temp_mouse_y);
		if(hov_pix > -1)
		{
			int32_t column = hov_pix%zoomtile.w;
			int32_t row = hov_pix/zoomtile.w;
			
			update_tooltip(temp_mouse_x,temp_mouse_y,zoomtile.x+(column*zoomtile.xscale),zoomtile.y+(row*zoomtile.yscale),zoomtile.xscale,zoomtile.yscale, NULL);
			redraw=true;
		}*/
		
		if(redraw)
		{
			custom_vsync();
			draw_edit_scr(tile,flip,cs,oldtile, false);
		}
		else
		{
			bool hs=has_selection();
			
			if(hs)
			{
				zoomtile16(screen2,tile,zoomtile.x-1,zoomtile.y-1,cs,flip,zoomtile.xscale);
			}
			
			custom_vsync();
			
			if(hs)
			{
				// blit(screen2, screen, 79, 31, 79, 31, 129, 129);
				blit(screen2, screen, zoomtile.x-1,zoomtile.y-1, zoomtile.x-1,zoomtile.y-1, (zoomtile.w*zoomtile.xscale)+1, (zoomtile.h*zoomtile.yscale)+1);
			}
			
			update_tool_cursor();
			SCRFIX();
		}
		
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
		
		saved=false;
		
		if(buf!=NULL)
		{
			delete[] buf;
		}
	}
	
	MouseSprite::set(ZQM_NORMAL);
	register_blank_tiles();
	register_used_tiles();
	clear_tooltip();
	comeback();
	destroy_bitmap(selection_pattern);
	destroy_bitmap(selecting_pattern);
	destroy_bitmap(intersection_pattern);
	font = oldfont;
	popup_zqdialog_end();
}

/*  Grab Tile Code  */

enum recolorState { rcNone, rc4Bit, rc8Bit };

BITMAP* original_imagebuf_bitmap=NULL;
void *imagebuf=NULL;
int32_t imagebuf_bitmap_scale=0;
#define IMAGEBUF_SCALE (imagebuf_bitmap_scale > 0 ? imagebuf_bitmap_scale : 1.0 / -imagebuf_bitmap_scale)
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
		else if(cs==1&&get_qr(qr_CSET1_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS) + i) * 3;
		else if(cs==5&&get_qr(qr_CSET5_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS + 1) + i) * 3;
		else if(cs==7&&get_qr(qr_CSET7_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS + 2) + i) * 3;
		else if(cs==8&&get_qr(qr_CSET8_LEVEL))
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
		else if(cs==1&&get_qr(qr_CSET1_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS) + (i%16)) * 3;
		else if(cs==5&&get_qr(qr_CSET5_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS + 1) + (i%16)) * 3;
		else if(cs==7&&get_qr(qr_CSET7_LEVEL))
			rgbByte = colordata + (CSET((Map.CurrScr()->color) * pdLEVEL + poNEWCSETS + 2) + (i%16)) * 3;
		else if(cs==8&&get_qr(qr_CSET8_LEVEL))
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
	int w = 640;
	int h = 480;
	int window_xofs=0;//(zq_screen_w-w-12)>>1;
	int window_yofs=0;//(zq_screen_h-h-25-6)>>1;
	jwin_draw_win(screen, window_xofs, window_yofs, w+6+6, h+25+6, FR_WIN);
	jwin_draw_frame(screen, window_xofs+4, window_yofs+23, w+2+2, h+2+2-(79*2),  FR_DEEP);
	
	FONT *oldfont = font;
	font = get_zc_font(font_lfont);
	jwin_draw_titlebar(screen, window_xofs+3, window_yofs+3, w+6, 18, "Grab Tile(s)", true);
	font=oldfont;
}

void draw_grab_scr(int32_t tile,int32_t cs,byte *newtile,int32_t black,int32_t white, int32_t width, int32_t height, byte *newformat)
{
	width=width;
	height=height;
	white=white; // happy birthday compiler
	
	int32_t yofs=0;
	//clear_to_color(screen2,bg);
	rectfill(screen2, 0, 0, 319, 159, black);
	rectfill(screen2,0,162,319,239,jwin_pal[jcBOX]);
	hline(screen2, 0, 160, 319, jwin_pal[jcMEDLT]);
	hline(screen2, 0, 161, 319, jwin_pal[jcLIGHT]);
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
	bool holdblank = blank_tile_table[0];
	
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
	blank_tile_table[0] = false;
	
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
	blank_tile_table[0] = holdblank;
	
	if(newtilebuf[0].data!=NULL)
	{
		free(newtilebuf[0].data);
	}
	
	if(is_valid_format(newtilebuf[0].format))
	{
		newtilebuf[0].data = (byte *)malloc(tilesize(newtilebuf[0].format));
		
		for(int32_t i=0; i<256; i++)
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
	int32_t screen_xofs=6;
	int32_t screen_yofs=25;
	int winh = 511;
	int32_t mul = 2;
	
	yofs=16;
	
	custom_vsync();
	
	stretch_blit(screen2,screen,0,0,320,240,screen_xofs,screen_yofs,640,480);
	
	// Suspend the current font while draw_text_button does its work
	FONT* oldfont = font;
	
	font = get_zc_font(font_lfont_l);
	
	int txt_x = 8*mul;
	int rbtn_x = 255*mul;
	int max_fpath_wid = rbtn_x-2-txt_x;
	int max_fpath_wid2 = max_fpath_wid-text_length(font,"... ");
	// Interface
	switch(imagetype)
	{
	case 0:
		textprintf_ex(screen,font,txt_x,(216+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%s",imgstr[imagetype]);
		break;
		
	case ftBMP:
	{
		std::string text = fmt::format("{}  {}x{}, {:.2g}x   zoom with , and .", imgstr[imagetype], original_imagebuf_bitmap->w, original_imagebuf_bitmap->h, IMAGEBUF_SCALE);
		int text_x = txt_x;
		int text_y = (216 + yofs) * mul;
		// TODO: can almost use this, but drawing is offset. prob cuz drawing to a different bitmap than the normal screen bitmap? idk
		// int text_w = text_length(font, text.c_str());
		// int text_h = text_height(font);
		// static int grab_scale_tooltip_id = ttip_register_id();
		// ttip_install(grab_scale_tooltip_id, "zoom with , and .", text_x, text_y, text_w, text_h, text_x, text_y - 40);
		textprintf_ex(screen, font, text_x, text_y, jwin_pal[jcTEXTFG], jwin_pal[jcBOX], "%s", text.c_str());

		draw_text_button(screen,117*mul,(192+yofs)*mul,int32_t(61*(1.5)),int32_t(20*(1.5)),"Recolor",vc(1),vc(14),0,true);
		break;
	}
	
	case ftZGP:
	case ftQST:
	case ftZQT:
	case ftQSU:
	case ftTIL:
	case ftBIN:
		textprintf_ex(screen,get_zc_font(font_lfont_l),txt_x,(216+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"%s  %d KB",imgstr[imagetype],imagesize>>10);
		break;
	}
	
	textprintf_ex(screen,font,txt_x,(168+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"sel: %d %d",selx,sely);
	textprintf_ex(screen,font,txt_x,(176+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"pos: %d %d",imagex,imagey);
	
	if(bp==8)
		textprintf_ex(screen,font,txt_x,(192+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"8-bit");
	else
		textprintf_ex(screen,font,txt_x,(192+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"cset: %d",cs);
	textprintf_ex(screen,font,txt_x,(200+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"step: %d",grabmode);
	
	if(imagetype==ftBIN)
	{
		textprintf_ex(screen,font,104*mul,(192+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"bp:  %d%s",bp,nesmode?" (NES)":"");
		textprintf_ex(screen,font,104*mul,(200+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"ofs: %Xh",romofs);
		textprintf_ex(screen,font,104*mul,(208+yofs)*mul,jwin_pal[jcTEXTFG],jwin_pal[jcBOX],"m: %d",romtilemode);
	}
	
	int fpath_y = (224+yofs)*mul;
	if(text_length(font,imagepath) <= max_fpath_wid)
		textout_ex(screen,font,imagepath,txt_x,fpath_y,jwin_pal[jcTEXTFG],jwin_pal[jcBOX]);
	else
	{
		char buf[2052] = {0};
		strncpy(buf,imagepath,2048);
		int len = strlen(buf);
		char *ptr = buf;
		char *endptr = buf+len;
		char *it = endptr;
		int tmpy = fpath_y;
		int tmph = text_height(font)+1;
		while(true)
		{
			if(tmpy+tmph > (winh-2))
				break; //Out of space!
			char c = *it;
			bool end = !c;
			*it = 0;
			int newlen = text_length(font,ptr);
			if(newlen <= (end ? max_fpath_wid : max_fpath_wid2))
			{
				if(end) //No stored character, string ended
				{
					textout_ex(screen,font,ptr,txt_x,tmpy,jwin_pal[jcTEXTFG],jwin_pal[jcBOX]);
					break;
				}
				char t[5];
				t[0] = c;
				for(int q = 1; q < 5; ++q)
					t[q] = it[q];
				strcpy(it,"...");
				textout_ex(screen,font,ptr,txt_x,tmpy,jwin_pal[jcTEXTFG],jwin_pal[jcBOX]);
				for(int q = 0; q < 5; ++q)
					it[q] = t[q];
				tmpy += tmph;
				ptr = it;
				it = endptr;
			}
			else
			{
				*it = c;
				--it;
			}
		}
	}
	draw_text_button(screen,rbtn_x,(168+yofs)*mul,int32_t(61*(1.5)),int32_t(20*(1.5)),"OK",vc(1),vc(14),0,true);
	draw_text_button(screen,rbtn_x,(192+yofs)*mul,int32_t(61*(1.5)),int32_t(20*(1.5)),"Cancel",vc(1),vc(14),0,true);
	draw_text_button(screen,rbtn_x,(216+yofs)*mul,int32_t(61*(1.5)),int32_t(20*(1.5)),"File",vc(1),vc(14),0,true);
	draw_text_button(screen,117*mul,(166+yofs)*mul,int32_t(61*(1.5)),int32_t(20*(1.5)),"Leech",vc(1),vc(14),0,true);
	
	//int32_t rectw = 16*mul;
	//rect(screen,selx+screen_xofs,sely+screen_yofs,selx+screen_xofs+((width-1)*rectw)+rectw-1,sely+screen_yofs+((height-1)*rectw)+rectw-1,white);
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
	zquestheader tempheader{};
	
	if(imagebuf)
	{
		switch(imagetype)
		{
		case ftBMP:
			if (original_imagebuf_bitmap != imagebuf)
				destroy_bitmap((BITMAP*)imagebuf);
			destroy_bitmap(original_imagebuf_bitmap);
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
		original_imagebuf_bitmap=NULL;
	}
	
	selx=sely=romofs=0;
	bp=4;
	imagetype=filetype(imagepath);
	
	dword section_id;
	word section_version;
	
	switch(imagetype)
	{
	case ftBMP:
	packfile_password("");
		memset(imagepal, 0, sizeof(PALETTE));
		original_imagebuf_bitmap = load_bitmap(imagepath,imagepal);
		imagesize = file_size_ex_password(imagepath,"");
		tilecount=0;
		create_rgb_table(&rgb_table, imagepal, NULL);
		rgb_map = &rgb_table;
		create_color_table(&imagepal_table, RAMpal, return_RAMpal_color, NULL);
		
		if(!original_imagebuf_bitmap)
		{
			imagetype=0;
		}
		else
		{
			imagebuf = original_imagebuf_bitmap;
			imagebuf_bitmap_scale = 1;
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
		
		if(!p_mgetl(&section_id,f))
		{
			goto error;
		}
		
		if(section_id==ID_TILES)
		{
			if(readtiles(f, grabtilebuf, NULL, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES, false)==0)
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
		
		if(!p_mgetl(&section_id,f))
		{
			goto error2;
		}
		
		if(section_id!=ID_GRAPHICSPACK)
		{
			goto error2;
		}
		
		//section version info
		if(!p_igetw(&section_version,f))
		{
			goto error2;
		}
		
		if(!read_deprecated_section_cversion(f))
		{
			goto error2;
		}
		
		//tiles
		if(!p_mgetl(&section_id,f))
		{
			goto error2;
		}
		
		if(section_id==ID_TILES)
		{
			if(readtiles(f, grabtilebuf, NULL, ZELDA_VERSION, VERSION_BUILD, 0, NEWMAXTILES, false)!=0)
			{
				goto error2;
			}
		}
		
error2:
		pack_fclose(f);
		tilecount=count_tiles(grabtilebuf);
		break;
		
	case ftQST:
		encrypted=true;
	case ftZQT:
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
		int ret = loadquest(imagepath,&tempheader,&QMisc,customtunes,true,skip_flags);
		if (ret)
		{
			imagetype=0;
			imagesize=0;
			clear_tiles(grabtilebuf);
			chop_path(imagepath);
		}
		
		if (!ret && encrypted && compressed)
		{
			if(quest_access(imagepath, &tempheader) != 1)
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
	leech_dlg[0].dp2=get_zc_font(font_lfont);
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
		
	int32_t ret = do_zqdialog(leech_dlg,3);
	
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
		if(jwin_alert("Confirm Truncation","Too many tiles.","Truncation may occur.",NULL,"&OK","&Cancel",'o','c',get_zc_font(font_lfont))==2)
		{
			delete[] testtile;
			return false;
		}
	}
	
	go_tiles();
	saved=false;
	
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
					font = get_zc_font(font_lfont);
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
				blit(status,screen,0, 0, 40, 20, 240, 140);
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
						testtile[(y*16)+x]=getpixel(((BITMAP*)imagebuf),(tx*16)+x,(ty*16)+y);
						testtile[(y*16)+x+1]=getpixel(((BITMAP*)imagebuf),(tx*16)+x+1,(ty*16)+y);
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
												if((dest[checktile].data[(y*8*newformat)+(x/(3-newformat))])!=testtile[(y*16)+x])
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
												if((dest[checktile].data[(y*8*newformat)+(14+(newformat-1)-x)/(3-newformat)])!=testtile[(y*16)+x])
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
												if((dest[checktile].data[((15-y)*8*newformat)+(x/(3-newformat))])!=testtile[(y*16)+x])
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
												if((dest[checktile].data[((15-y)*8*newformat)+((14+(newformat-1)-x)/(3-newformat))])!=testtile[(y*16)+x])
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

				bool ever_did_unmasked = false;

				for(int32_t y=0; y<16; y++)
				{
					for(int32_t x=0; x<16; x+=2)
					{
						bool masked = (y<8 && x<8 && grabmask&1) || (y<8 && x>7 && grabmask&2) || (y>7 && x<8 && grabmask&4) || (y>7 && x>7 && grabmask&8);
						if (masked)
						{
							dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x)]=def[(y*16)+(x)];
							dest[(ty*TILES_PER_ROW)+tx][(y*16)+(x+1)]=def[(y*16)+(x+1)];
						}
						else
						{
							dest[(ty*TILES_PER_ROW)+tx][(y*16)+x]=getpixel(screen2,(tx*16)+x+selx,(ty*16)+y+sely);
							dest[(ty*TILES_PER_ROW)+tx][(y*16)+x+1]=getpixel(screen2,(tx*16)+x+1+selx,(ty*16)+y+sely);
							ever_did_unmasked = true;
						}
						if (format == tf4Bit)
						{
							dest[(ty*TILES_PER_ROW)+tx][(y*16)+x] &= 15;
							dest[(ty*TILES_PER_ROW)+tx][(y*16)+x+1] &= 15;
						}
					}
				}

				if (ever_did_unmasked)
					newformat[(ty*TILES_PER_ROW)+tx] = format;
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

static void scale_imagebuf_bitmap()
{
	imagebuf_bitmap_scale = std::clamp(imagebuf_bitmap_scale, -10, 10);

	float scale = IMAGEBUF_SCALE;
	int nw = original_imagebuf_bitmap->w * scale;
	int nh = original_imagebuf_bitmap->h * scale;
	if (nw <= 0 || nh <= 0)
		return;

	BITMAP* scaled_bmp = create_bitmap_ex(8, nw, nh);
	if (!scaled_bmp)
		return;

	stretch_blit(original_imagebuf_bitmap, scaled_bmp, 0, 0, original_imagebuf_bitmap->w, original_imagebuf_bitmap->h, 0, 0, nw, nh);
	if (imagebuf != original_imagebuf_bitmap)
		destroy_bitmap((BITMAP*)imagebuf);
	imagebuf = scaled_bmp;
}

//Grabber is not grabbing to tile pages beyond pg. 252 right now. -ZX 18th June, 2019 
void grab_tile(int32_t tile,int32_t &cs)
{
	zq_allow_tile_draw_cache = true;

	int window_w = 640+6+6, window_h = 480+25+6;
	int window_x=(zq_screen_w-window_w)/2;
	int window_y=(zq_screen_h-window_h)/2;
	popup_zqdialog_start(window_x,window_y,window_w,window_h,-1);
	int window_xofs = 0;
	int screen_xofs=6;
	int screen_yofs=25;
	int panel_yofs=0;
	int bwidth = 61*1.5;
	int bheight = 20*1.5;
	int button_x = 255*2;
	int grab_ok_button_y = 168*2 + 32;
	int leech_button_x = 117*2;
	int leech_button_y = 166*2 + 32;
	int grab_cancel_button_y = 192*2 + 32;
	int file_button_y = 216*2 + 32;
	int rec_button_x = 117*2;
	int rec_button_y = 192*2 + 32;
	
	int screen_y1 = 24;
	int screen_y2 = screen_y1+320-1;
	
	int crect_x = 184+190;
	int crect_y = 168*2 + 32;
	int crect_w = 8*2;
	int crect_h = 8*2;
	
	int xrect_x = 640 + 12 - 21;
	int xrect_y = 5;
	
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
	int done=0;
	int pal=0;
	int f=0;
	int black=vc(0),white=vc(15);
	int selwidth=1, selheight=1;
	int selx2=0, sely2=0;
	bool xreversed=false, yreversed=false;
	bool doleech=false, dofile=false, dopal=false;
	
	int jwin_pal2[jcMAX];
	memcpy(jwin_pal2, jwin_pal, sizeof(int)*jcMAX);
	
	
	if(imagebuf==NULL)
		load_imagebuf();
		
	calc_cset_reduce_table(imagepal, cs);
	calc_cset_reduce_table_8bit(imagepal);
	draw_grab_window();
	draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
	grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	do
	{
		HANDLE_CLOSE_ZQDLG();
		if(exiting_program) break;
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
				if(CHECK_CTRL_CMD) sely=zc_min(sely+1,144);
				else ++imagey;
				
				break;
				
			case KEY_UP:
				if(CHECK_CTRL_CMD) sely=zc_max(sely-1,0);
				else --imagey;
				
				break;
				
			case KEY_RIGHT:
				if(CHECK_CTRL_CMD) selx=zc_min(selx+1,304);
				else ++imagex;
				
				break;
				
			case KEY_LEFT:
				if(CHECK_CTRL_CMD) selx=zc_max(selx-1,0);
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
				cs = (cs>0)  ? cs-1:13;
				if(recolor==rc4Bit)
					calc_cset_reduce_table(imagepal, cs);
				break;
				
			case KEY_S:
				if(grabmode==1) grabmode=8;
				else if(grabmode==8) grabmode=16;
				else grabmode=1;
				
				break;
			
			case KEY_COMMA:
			if (imagetype == ftBMP)
			{
				imagebuf_bitmap_scale--;
				if (imagebuf_bitmap_scale == 0)
					imagebuf_bitmap_scale = -2;
				scale_imagebuf_bitmap();
			}
			break;
			case KEY_STOP:
			if (imagetype == ftBMP)
			{
				imagebuf_bitmap_scale++;
				if (imagebuf_bitmap_scale == -1)
					imagebuf_bitmap_scale = 1;
				scale_imagebuf_bitmap();
			}
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
			int x=gui_mouse_x();
			int y=gui_mouse_y();
			if(isinRect(x,y,xrect_x, xrect_y, xrect_x + 15, xrect_y + 13))
				if(do_x_button(screen, xrect_x, xrect_y))
					done=1;
			
			if(!bdown)
			{
				bool regrab=false;
				bdown=true;
				FONT* oldfont = font;
				font = get_zc_font(font_lfont_l);
				
				if(y>=screen_y1 && y<=screen_y2)
				{
					do
					{
						HANDLE_CLOSE_ZQDLG();
						if(exiting_program) break;
						int x = (gui_mouse_x()-screen_xofs) / 2;
						int y = (gui_mouse_y()-screen_yofs) / 2;
						
						int ox=selx,oy=sely,ow=selwidth,oh=selheight;
						
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
						
						bool changed = (ox!=selx || oy!=sely || ow!=selwidth || oh!=selheight);
						bool redraw = changed || !(f%8);
						
						if(redraw)
						{
							draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
							if(changed)
								grab(newtile,newtilebuf[tile].data, selwidth, selheight, newtilebuf[tile].format, newformat);
							if(f&8)
							{
								static const int w = 32;
								rect(screen,(selx*2)+screen_xofs,(sely*2)+screen_yofs,(selx*2)+screen_xofs+((selwidth-1)*w)+(w-1),(sely*2)+screen_yofs+((selheight-1)*w)+(w-1),white);
							}
						}
						else custom_vsync();
						
						++f;
					}
					while(gui_mouse_b());
				}
				else if(isinRect(x,y,button_x,grab_ok_button_y,button_x+bwidth,grab_ok_button_y+bheight))
				{
					if(do_text_button(button_x,grab_ok_button_y,bwidth,bheight,"OK"))
						done=2;
				}
				else if(isinRect(x,y,leech_button_x,leech_button_y,leech_button_x+bwidth,leech_button_y+bheight))
				{
					if(do_text_button(leech_button_x,leech_button_y,bwidth,bheight,"Leech"))
					{
						doleech=true;
					}
				}
				else if(isinRect(x,y,button_x,grab_cancel_button_y,button_x+bwidth,grab_cancel_button_y+bheight))
				{
					if(do_text_button(button_x,grab_cancel_button_y,bwidth,bheight,"Cancel"))
						done=1;
				}
				else if(isinRect(x,y,button_x,file_button_y,button_x+bwidth,file_button_y+bheight))
				{
					if(do_text_button(button_x,file_button_y,bwidth,bheight,"File"))
					{
						dofile=true;
					}
				}
				else if(imagetype == ftBMP && isinRect(x,y,rec_button_x, rec_button_y, rec_button_x+bwidth, rec_button_y+bheight))
				{
					if(do_text_button(rec_button_x,rec_button_y,bwidth,bheight,"Recolor"))
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
					draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
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
			if (prompt_for_existing_file_compat("Load File", "", list, imagepath, true))
			{
				zc_set_palette(RAMpal);
				pal=0;
				white=vc(15);
				black=vc(0);
				strcpy(imagepath,temppath);
				load_imagebuf();
				imagex=imagey=0;
				calc_cset_reduce_table(imagepal, cs);
				draw_grab_scr(tile,cs,newtile[0],black,white, selwidth, selheight, newformat);
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
				
				memcpy(jwin_pal, jwin_pal2, sizeof(int)*jcMAX);
				gui_bg_color=jwin_pal[jcBOX];
				gui_fg_color=jwin_pal[jcBOXFG];
				jwin_set_colors(jwin_pal);
			}
			
			zc_set_palette_range(pal?imagepal:RAMpal,0,255,false);
			
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
				
			int selxl = selx* 2;
			int selyl = sely* 2;
			int w = 32;
			
			if(f&8)
				rect(screen3,selxl,selyl,selxl+((selwidth-1)*w)+(w-1),selyl+((selheight-1)*w)+(w-1),white);
			
			blit(screen3,screen,selxl,selyl,selxl+screen_xofs,selyl+screen_yofs,selwidth*w,selheight*w);
		}
		
		//    SCRFIX();
		++f;
		
	}
	while(!done);
	
	memcpy(jwin_pal, jwin_pal2, sizeof(int)*jcMAX);
	gui_bg_color=jwin_pal[jcBOX];
	gui_fg_color=jwin_pal[jcBOXFG];
	jwin_set_colors(jwin_pal);
	
	
	if(done==2)
	{
		go_tiles();
		saved=false;
			
		for(int y=0; y<selheight; y++)
		{
			for(int x=0; x<selwidth; x++)
			{
				int temptile=tile+((TILES_PER_ROW*y)+x);
				int format=(bp==8) ? tf8Bit : tf4Bit;
				
				if(newtilebuf[temptile].data!=NULL)
					free(newtilebuf[temptile].data);
				
				newtilebuf[temptile].format=format;
				newtilebuf[temptile].data=(byte *)malloc(tilesize(format));
				
				if(newtilebuf[temptile].data==NULL)
				{
					Z_error_fatal("Unable to initialize tile #%d.\n", temptile);
					break;
				}
				
				for(int i=0; i<256; i++)
				{
					// newtilebuf[temptile].data[i] = cset_reduce_table[newtile[(TILES_PER_ROW*y)+x][i]];
					newtilebuf[temptile].data[i] = newtile[(TILES_PER_ROW*y)+x][i];
				}

				// unpackbuf[i]=(cset_reduce_table[unpackbuf[i]]);
			}
		}
	}
	
	destroy_bitmap(screen3);
	
	if(pal)
		zc_set_palette(RAMpal);
	
	recolor=rcNone;
	calc_cset_reduce_table(imagepal, cs);
	register_blank_tiles();
	popup_zqdialog_end();

	zq_allow_tile_draw_cache = false;
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
				if (InvalidBG == 2)
				{
					draw_checkerboard(dest, x, y, w);
				}
				else if(InvalidBG == 1)
				{
					for(int32_t dy=0; dy<=l+1; dy++)
					{
						for(int32_t dx=0; dx<=l+1; dx++)
						{
							dest->line[dy+(y)][dx+(x)]=vc((((zc_oldrand()%100)/50)?0:8)+(((zc_oldrand()%100)/50)?0:7));
						}
					}
				}
				else
				{
					for(int32_t dy=0; dy<=l+1; dy++)
					{
						for(int32_t dx=0; dx<=l+1; dx++)
						{
							dest->line[dy+(y)][dx+(x)]=vc(0);
						}
					}
					rect(dest, (x)+1,(y)+1, (x)+l, (y)+l, vc(15));
					line(dest, (x)+1,(y)+1, (x)+l, (y)+l, vc(15));
					line(dest, (x)+1,(y)+l, (x)+l, (y)+1,  vc(15));
				}
			}
		}
		else
		{
			puttile16(buf,first+i,0,0,cs,0);
			stretch_blit(buf,dest,0,0,16,16,x,y,w,h);
		}
		
		if((f%32)<=16 && large && !HIDE_8BIT_MARKER && newtilebuf[first+i].format==tf8Bit)
		{
			textprintf_ex(dest,get_zc_font(font_z3smallfont),(x)+l-3,(y)+l-3,vc(int32_t((f%32)/6)+10),-1,"8");
		}
	}
	
	destroy_bitmap(buf);
}

void tile_info_0(int32_t tile,int32_t tile2,int32_t cs,int32_t copy,int32_t copycnt,int32_t page,bool rect_sel)
{
	int32_t yofs=3;
	BITMAP *buf = create_bitmap_ex(8,16,16);
	int32_t mul = 2;
	FONT *tfont = get_zc_font(font_pfont);
	
	rectfill(screen2,0,210*2,(320*2)-1,(240*2),jwin_pal[jcBOX]);
	hline(screen2, 0, (210*2)-2, (320*2)-1, jwin_pal[jcMEDLT]);
	hline(screen2, 0, (210*2)-1, (320*2)-1, jwin_pal[jcLIGHT]);
	tfont = get_zc_font(font_lfont_l);
	
	// Copied tile and numbers
	jwin_draw_frame(screen2,(34*mul)-2,((216*mul)+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	int32_t coldiff=TILECOL(copy)-TILECOL(copy+copycnt-1);
	if(copy>=0)
	{
		puttile16(buf,rect_sel&&coldiff>0?copy-coldiff:copy,0,0,cs,0);
		stretch_blit(buf,screen2,0,0,16,16,34*mul,216*mul+yofs,16*mul,16*mul);
		
		if(copycnt>1)
		{
			textprintf_right_ex(screen2,tfont,28*mul,(216*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d-",copy);
			textprintf_right_ex(screen2,tfont,24*mul,(224*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",copy+copycnt-1);
		}
		else
		{
			textprintf_right_ex(screen2,tfont,24*mul,(220*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",copy);
		}
	}
	else // No tiles copied
	{
		if (InvalidBG == 2)
		{
			draw_checkerboard(screen2, 34 * mul, 216 * mul + yofs, 16 * mul);
		}
		else if(InvalidBG == 1)
		{
			for(int32_t dy=0; dy<16*mul; dy++)
			{
				for(int32_t dx=0; dx<16*mul; dx++)
				{
					screen2->line[(216*mul+yofs+dy)][36*mul+dx]=vc((((zc_oldrand()%100)/50)?0:8)+(((zc_oldrand()%100)/50)?0:7));
				}
			}
		}
		else
		{
			rectfill(screen2, 34*mul, (216*mul)+yofs, (34+15)*mul, ((216+15)*mul)+yofs, vc(0));
			rect(screen2, 34*mul, (216*mul)+yofs, (34+15)*mul, ((216+15)*mul)+yofs, vc(15));
			line(screen2, 34*mul, (216*mul)+yofs, (34+15)*mul, ((216+15)*mul)+yofs, vc(15));
			line(screen2, 34*mul, ((216+15)*mul)+yofs, (34+15)*mul, (216*mul)+yofs, vc(15));
		}
	}
	
	
	// Current tile
	jwin_draw_frame(screen2,(104*mul)-2,(216*mul+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	puttile16(buf,tile,0,0,cs,0);
	stretch_blit(buf,screen2,0,0,16,16,104*mul,216*mul+yofs,16*mul,16*mul);
	
	// Current selection mode
	jwin_draw_frame(screen2,(127*mul)-2,(216*mul+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
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
	textprintf_ex(screen2,tfont,55*mul,216*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"cs: %d",cs);
	textprintf_right_ex(screen2,tfont,99*mul,216*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"tile:");
	textprintf_right_ex(screen2,tfont,99*mul,224*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d%s",tile,tbuf);
	
	FONT *tf = font;
	font = tfont;
	
	draw_text_button(screen2,150*mul,213*mul+yofs,28*mul,21*mul,"&Grab",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
	draw_text_button(screen2,(150+28)*mul,213*mul+yofs,28*mul,21*mul,"&Edit",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
	draw_text_button(screen2,(150+28*2)*mul,213*mul+yofs,28*mul,21*mul,"Export",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
	draw_text_button(screen2,(150+28*3)*mul,213*mul+yofs,28*mul,21*mul,"Recolor",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
	draw_text_button(screen2,(150+28*4)*mul,213*mul+yofs,28*mul,21*mul,"Done",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
	
	jwin_draw_icon(screen2,(305*mul+4),220*mul-6+yofs,jwin_pal[jcBOXFG],BTNICON_ARROW_UP,6,true);
	textprintf_ex(screen2,tfont,293*mul,220*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"p:");
	textprintf_centre_ex(screen2,tfont,(305*mul+4),220*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",page);
	jwin_draw_icon(screen2,(305*mul+4),228*mul+3+yofs,jwin_pal[jcBOXFG],BTNICON_ARROW_DOWN,6,true);
	
	font = tf;

	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-25-6)>>1;
	int32_t screen_xofs=window_xofs+6;
	int32_t screen_yofs=window_yofs+25;
	
	custom_vsync();
	blit(screen2,screen,0,0,screen_xofs,screen_yofs,w,h);
	SCRFIX();
	destroy_bitmap(buf);
}

void tile_info_1(int32_t oldtile,int32_t oldflip,int32_t oldcs,int32_t tile,int32_t flip,int32_t cs,int32_t copy,int32_t page, bool always_use_flip)
{
	int32_t yofs=3;
	BITMAP *buf = create_bitmap_ex(8,16,16);
	int32_t mul = 2;
	FONT *tfont = get_zc_font(font_pfont);
	
	rectfill(screen2,0,210*2,(320*2)-1,(240*2),jwin_pal[jcBOX]);
	hline(screen2, 0, (210*2)-2, (320*2)-1, jwin_pal[jcMEDLT]);
	hline(screen2, 0, (210*2)-1, (320*2)-1, jwin_pal[jcLIGHT]);
	tfont = get_zc_font(font_lfont_l);
	
	jwin_draw_frame(screen2,(124*mul)-2,((216*mul)+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	
	if(copy>=0)
	{
		puttile16(buf,copy,0,0,cs,flip);
		stretch_blit(buf,screen2,0,0,16,16,124*mul,216*mul+yofs,16*mul,16*mul);
	}
	else
	{
		if (InvalidBG == 2)
		{
			draw_checkerboard(screen2, 124 * mul, 216 * mul + yofs, 16 * mul);
		}
		else if(InvalidBG == 1)
		{
			for(int32_t dy=0; dy<16*mul; dy++)
			{
				for(int32_t dx=0; dx<16*mul; dx++)
				{
					screen2->line[216*mul+yofs+dy][124*mul+dx]=vc((((zc_oldrand()%100)/50)?0:8)+(((zc_oldrand()%100)/50)?0:7));
				}
			}
		}
		else
		{
			rectfill(screen2, 124*mul, (216*mul)+yofs, (124+15)*mul, ((216+15)*mul)+yofs, vc(0));
			rect(screen2, 124*mul, (216*mul)+yofs, (124+15)*mul, ((216+15)*mul)+yofs, vc(15));
			line(screen2, 124*mul, (216*mul)+yofs, (124+15)*mul, ((216+15)*mul)+yofs, vc(15));
			line(screen2, 124*mul, ((216+15)*mul)+yofs, (124+15)*mul, (216*mul)+yofs, vc(15));
		}
	}
	
	jwin_draw_frame(screen2,(8*mul)-2,(216*mul+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	puttile16(buf,oldtile,0,0, oldcs, oldflip);
	stretch_blit(buf,screen2,0,0,16,16,8*mul,216*mul+yofs,16*mul,16*mul);
	
	textprintf_right_ex(screen2,tfont,56*mul,212*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Old Tile:");
	textprintf_ex(screen2,tfont,60*mul,212*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",oldtile);
	
	textprintf_right_ex(screen2,tfont,56*mul,220*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet:");
	textprintf_ex(screen2,tfont,60*mul,220*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",oldcs);
	
	if(oldflip > 0 || always_use_flip)  // Suppress Flip for this usage
	{
		textprintf_right_ex(screen2,tfont,56*mul,228*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Flip:");
		textprintf_ex(screen2,tfont,60*mul,228*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",oldflip);
	}
	
	jwin_draw_frame(screen2,(148*mul)-2,(216*mul+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	puttile16(buf,tile,0,0, cs,
			  (oldflip>0 || always_use_flip)?flip:0); // Suppress Flip for this usage
	stretch_blit(buf,screen2,0,0,16,16,148*mul,216*mul+yofs,16*mul,16*mul);
	
	textprintf_right_ex(screen2,tfont,201*mul,212*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"New Tile:");
	textprintf_ex(screen2,tfont,205*mul,212*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",tile);
	textprintf_right_ex(screen2,tfont,201*mul,220*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"CSet:");
	textprintf_ex(screen2,tfont,205*mul,220*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",cs);
	
	if(oldflip > 0 || always_use_flip)  // Suppress Flip for this usage
	{
		textprintf_right_ex(screen2,tfont,201*mul,228*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Flip:");
		textprintf_ex(screen2,tfont,205*mul,228*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",flip);
	}
	
	jwin_draw_icon(screen2,(305*mul+4),220*mul-6+yofs,jwin_pal[jcBOXFG],BTNICON_ARROW_UP,6,true);
	textprintf_ex(screen2,tfont,293*mul,220*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"p:");
	textprintf_centre_ex(screen2,tfont,309*mul,220*mul+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",page);
	jwin_draw_icon(screen2,(305*mul+4),228*mul+3+yofs,jwin_pal[jcBOXFG],BTNICON_ARROW_DOWN,6,true);
	

	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-25-6)>>1;
	int32_t screen_xofs=window_xofs+6;
	int32_t screen_yofs=window_yofs+25;
	
	custom_vsync();
	blit(screen2,screen,0,0,screen_xofs,screen_yofs,w,h);
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

enum
{
	MENUID_SELTILE_VIEW_HIDE_USED,
	MENUID_SELTILE_VIEW_HIDE_UNUSED,
	MENUID_SELTILE_VIEW_HIDE_BLANK,
	MENUID_SELTILE_VIEW_HIDE_8BIT,
};
static NewMenu select_tile_view_menu
{
	{ "Hide Used", hide_used, MENUID_SELTILE_VIEW_HIDE_USED },
	{ "Hide Unused", hide_unused, MENUID_SELTILE_VIEW_HIDE_UNUSED },
	{ "Hide Blank", hide_blank, MENUID_SELTILE_VIEW_HIDE_BLANK },
	{ "Hide 8-bit marker", hide_8bit_marker, MENUID_SELTILE_VIEW_HIDE_8BIT },
};

static std::function<void(int)> select_tile_color_depth_cb;

static void set_tile_color_depth_4()
{
	select_tile_color_depth_cb(tf4Bit);
}
static void set_tile_color_depth_8()
{
	select_tile_color_depth_cb(tf8Bit);
}
enum
{
	MENUID_SELTILE_COLOR_DEPTH_4_BIT,
	MENUID_SELTILE_COLOR_DEPTH_8_BIT,
};
static NewMenu select_tile_color_depth_menu
{
	{ "4-bit", set_tile_color_depth_4, MENUID_SELTILE_COLOR_DEPTH_4_BIT },
	{ "8-bit", set_tile_color_depth_8, MENUID_SELTILE_COLOR_DEPTH_8_BIT },
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




static DIALOG move_textbox_list_dlg[] =
{
	// (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
	{ jwin_win_proc,      0,   0,   300,  212,  vc(14),  vc(1),  0,       D_EXIT,          0,             0,      NULL, NULL, NULL },
	{ jwin_ctext_proc,   150,  18,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "", NULL, NULL },
	{ jwin_ctext_proc,   150,  28,     0,  8,    vc(15),  vc(1),  0,       0,          0,             0, (void *) "", NULL, NULL },
	{ jwin_textbox_proc,  12,   40,   277,  138,   jwin_pal[jcTEXTFG],  jwin_pal[jcTEXTBG],  0,       D_EXIT,     0,             0,      NULL, NULL, NULL },
	{ jwin_button_proc,   80,   185,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "OK", NULL, NULL },
	{ jwin_button_proc,  160,   185,  61,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
	{ d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
	{ NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

bool popup_move_textbox_dlg(string const& msg, char* textbox, char const* title)
{
	char buf1[512] = {0};
	char buf2[512] = {0};
	large_dialog(move_textbox_list_dlg);
	DIALOG& tbox = move_textbox_list_dlg[3];
	{
		FONT* f = tbox.dp2 ? (FONT*)tbox.dp2 : get_custom_font(CFONT_GUI);
		int indx = 0, word_indx = 0;
		for(char c : msg)
		{
			if(c == ' ' || c == '\n')
				word_indx = indx;
			buf1[indx++] = c;
			if(c == '\n' || text_length(f, buf1) >= tbox.w)
			{
				buf1[word_indx] = 0;
				strcpy(buf2, msg.c_str()+word_indx+1);
				break;
			}
		}
	}
	
	move_textbox_list_dlg[0].dp = (void*)title;
	move_textbox_list_dlg[0].dp2 = get_zc_font(font_lfont);
	move_textbox_list_dlg[1].dp = buf1;
	move_textbox_list_dlg[2].dp = buf2;
	tbox.dp = textbox;
	tbox.d2 = 0;
	auto tby = tbox.y;
	auto tbh = tbox.h;
	if(!buf2[0])
	{
		auto diff = move_textbox_list_dlg[2].h;
		tbox.y -= diff;
		tbox.h += diff;
	}
	
	int32_t ret=do_zqdialog(move_textbox_list_dlg,2);
	position_mouse_z(0);
	tbox.y = tby;
	tbox.h = tbh;
	
	return ret == 4;
}

int32_t quick_select_3(int32_t a, int32_t b, int32_t c, int32_t d)
{
	return a==0?b:a==1?c:d;
}

bool TileMoveList::process(std::unique_ptr<BaseTileRef>& ref, TileMoveProcess const& proc, bool is_dest)
{
	TileRefCombo* combo_ref = dynamic_cast<TileRefCombo*>(ref.get());
	int i = ti_none;
	auto t = ref->getTile() + ref->offset();
	
	if(combo_ref)
	{
		if(proc.rect)
			i=move_intersection_sr(*combo_ref->combo, proc._l, proc._t, proc._w, proc._h);
		else i=move_intersection_ss(*combo_ref->combo, proc._first, proc._last);
	}
	else if(proc.rect)
	{
		if(ref->h > 1)
			i=move_intersection_rr(TILECOL(t), TILEROW(t), ref->w, ref->h, proc._l, proc._t, proc._w, proc._h);
		else i=move_intersection_sr(t, t+ref->w-1, proc._l, proc._t, proc._w, proc._h);
	}
	else
	{
		if(ref->h > 1)
			i=move_intersection_rs(TILECOL(t), TILEROW(t), ref->w, ref->h, proc._first, proc._last);
		else i=move_intersection_ss(t, t+ref->w-1, proc._first, proc._last);
	}
	
	bool in = i != ti_none, out = i != ti_encompass;
	for(size_t q = 0; !(in&&out) && q < ref->extra_rects.size(); ++q)
	{
		auto [ex_t,ex_w,ex_h] = ref->extra_rects[q];
		if(proc.rect)
			i = move_intersection_rr(TILECOL(t+ex_t), TILEROW(t+ex_t), ex_w, ex_h, proc._l, proc._t, proc._w, proc._h);
		else i = move_intersection_rs(TILECOL(t+ex_t), TILEROW(t+ex_t), ex_w, ex_h, proc._first, proc._last);
		if(i != ti_none)
			in = true;
		if(i != ti_encompass)
			out = true;
	}
	i = in ? (out ? ti_broken : ti_encompass) : ti_none;
	
	if(i != ti_none && ref->getTile() != 0)
	{
		if(mode == Mode::CHECK_ALL)
		{
			move_refs.emplace_back(std::move(ref));
			return true;
		}
		else if(i==ti_broken || is_dest || (i==ti_encompass && ref->no_move))
		{
			if(warning_flood || warning_list.tellp() >= 65000)
			{
				if(!warning_flood)
					warning_list << "...\n...\n...\nmany others";
				warning_flood = true;
			}
			else
				warning_list << ref->name << '\n';
		}
		else if(i==ti_encompass)
		{
			move_refs.emplace_back(std::move(ref));
			return true;
		}
	}
	return false;
}

bool TileMoveList::check_prot()
{
	if(!TileProtection)
		return true;
	auto ret = !warning_list.tellp() || popup_move_textbox_dlg(msg, warning_list.str().data(), "Tile Warning");
	
	warning_flood = false;
	warning_list.clear();
	
	return ret;
}

void TileMoveList::add_diff(int diff)
{
	for(auto& ref : move_refs)
		ref->addTile(diff);
}

//from 'combo.h'
bool ComboMoveList::process(std::unique_ptr<BaseComboRef>& ref, ComboMoveProcess const& proc, bool is_dest)
{
	int i = ti_none;
	auto c = ref->getCombo();
	
	if(ref->no_move)
		processed_combos[c] = true;
	else processed_combos[c]; //inserts element if does not exist
	i = move_intersection_ss(c, c, proc._first, proc._last);
	
	if(i != ti_none && ref->getCombo() != 0)
	{
		if(i==ti_broken || is_dest || (i==ti_encompass && ref->no_move))
		{
			if(ComboProtection)
			{
				if(warning_flood || warning_list.tellp() >= 65000)
				{
					if(!warning_flood)
						warning_list << "...\n...\n...\nmany others";
					warning_flood = true;
				}
				else
					warning_list << ref->name << '\n';
			}
		}
		else if(i==ti_encompass)
		{
			move_refs.emplace_back(std::move(ref));
			return true;
		}
	}
	return false;
}

bool ComboMoveList::check_prot()
{
	if(!ComboProtection)
		return true;
	vector<set<int> const*> subset = combo_links.subset(processed_combos);
	bool subset_header = false;
	for(int q = 0; q < 2; ++q)
	{
		bool is_dest = (q==1);
		if(!is_dest && !source_process)
			continue;
		ComboMoveProcess const& proc = is_dest ? dest_process : *source_process;
		for(auto it = subset.begin(); it != subset.end();)
		{
			auto s = *it;
			if(warning_flood || warning_list.tellp() >= 65000)
			{
				if(!warning_flood)
					warning_list << "...\n...\n...\nmany others";
				warning_flood = true;
				break;
			}
			set<int> in_set, out_set;
			bool no_move = is_dest;
			for(int c : *s)
			{
				int i = move_intersection_ss(c, c, proc._first, proc._last);
				if(i != ti_none)
					in_set.insert(c);
				if(i != ti_encompass)
					out_set.insert(c);
				if(!no_move)
				{
					auto it = processed_combos.find(c);
					if(it != processed_combos.end() && it->second)
						no_move = true;
				}
			}
			int i = in_set.empty() ? ti_none : (out_set.empty() ? ti_encompass : ti_broken);
			if(i == ti_encompass && !no_move)
			{
				it = subset.erase(it);
				continue;
			}
			if(i == ti_none)
			{
				++it;
				continue;
			}
			
			if(!subset_header)
			{
				subset_header = true;
				warning_list << "===== Broken Relative Combo Groups =====\n";
			}
			bool comma = false;
			warning_list << "In(";
			for(int c : in_set)
			{
				if(comma)
					warning_list << ",";
				else comma = true;
				warning_list << c;
			}
			warning_list << "),Out(";
			comma = false;
			for(int c : out_set)
			{
				if(comma)
					warning_list << ",";
				else comma = true;
				warning_list << c;
			}
			warning_list << ")\n";
			it = subset.erase(it);
		}
	}
	auto ret = !warning_list.tellp() || popup_move_textbox_dlg(msg, warning_list.str().data(), "Combo Warning");
	
	processed_combos.clear();
	warning_flood = false;
	warning_list.clear();
	
	return ret;
}

void ComboMoveList::add_diff(int diff)
{
	for(auto& ref : move_refs)
		ref->addCombo(diff);
}

static void collect_subscreen_tiles(SubscrWidget& widget, TileMoveList& list)
{
	if (auto w = dynamic_cast<SW_2x2Frame*>(&widget))
	{
		list.add_tile(&w->tile, 2, 2, "2x2 Frame");
	}
	else if (auto w = dynamic_cast<SW_TriFrame*>(&widget))
	{
		list.add_tile(&w->frame_tile, 6, 3, "McGuffin Frame - Frame");
		list.add_tile(&w->piece_tile, "McGuffin Frame - Piece");
	}
	else if (auto w = dynamic_cast<SW_McGuffin*>(&widget))
	{
		list.add_tile(&w->tile, "McGuffin Piece");
	}
	else if (auto w = dynamic_cast<SW_TileBlock*>(&widget))
	{
		list.add_tile(&w->tile, w->w, w->h, "TileBlock");
	}
	else if (auto w = dynamic_cast<SW_MiniTile*>(&widget))
	{
		if (w->tile == -1)
			return;

		list.add_tile(&w->tile, "MiniTile");
	}
	else if (auto w = dynamic_cast<SW_GaugePiece*>(&widget))
	{
		int fr = w->frames ? w->frames : 1;
		fr = fr * (1+(w->get_per_container()/(w->unit_per_frame+1)));
		if(!(w->flags&SUBSCR_GAUGE_FULLTILE))
			fr = (fr/4_zf).getCeil();

		for(auto q = 0; q < 4; ++q)
		{
			list.add_tile(&w->mts[q].mt_tile, fr, 1, fmt::format("Gauge Tile {}", q));
		}
	}
}

static void collect_subscreen_tiles(SubscrPage& page, TileMoveList& list)
{
	for(auto q = 0; q < page.contents.size(); ++q)
	{
		size_t indx = list.move_refs.size();
		collect_subscreen_tiles(*page.contents[q], list);
		for(; indx < list.move_refs.size(); ++indx)
		{
			auto& ref = list.move_refs[indx];
			ref->name = fmt::format("Widget {} - {}", q, ref->name);
		}
	}
}

static void collect_subscreen_tiles(ZCSubscreen& subscreen, TileMoveList& list)
{
	for (auto q = 0; q < subscreen.pages.size(); ++q)
	{
		size_t indx = list.move_refs.size();
		collect_subscreen_tiles(subscreen.pages[q], list);
		for(; indx < list.move_refs.size(); ++indx)
		{
			auto& ref = list.move_refs[indx];
			ref->name = fmt::format("Page {} - {}", q, ref->name);
		}
	}
}

bool _handle_tile_move(TileMoveProcess dest_process, optional<TileMoveProcess> source_process, int diff, TileMoveUndo* on_undo = nullptr, std::function<void(int32_t)> every_proc = nullptr, TileMoveList::Mode mode = TileMoveList::Mode::MOVE)
{
	bool BSZ2 = get_qr(qr_BSZELDA);
	bool move = source_process.has_value();
	TileMoveUndo local_undo;
	TileMoveUndo& storage = on_undo ? *on_undo : local_undo;
	auto& vec = storage.vec;
	storage.diff = diff;
	storage.state = false;
	
	//Combos
	{
		auto& movelist = vec.emplace_back(std::make_unique<TileMoveList>(
			dest_process, source_process, mode,
			move
			? "The tiles used by the following combos will be partially cleared by the move."
			: "The tiles used by the following combos will be partially or completely overwritten by this process."
			));
		for(int32_t q = 0; q < MAXCOMBOS; ++q)
		{
			auto& cmb = combobuf[q];
			auto lbl = fmt::format("Combo {}{}", q, cmb.label.empty() ? ""
				: fmt::format(" ({})", cmb.label));
			movelist->add_combo(&cmb, lbl);
			
			//type-specific
			char const* type_name = ZI.getComboTypeName(cmb.type);
			switch(cmb.type)
			{
				case cSPOTLIGHT:
				{
					if(!(cmb.usrflags & cflag1))
						break;
					movelist->add_tile_10k(&cmb.attributes[0], 16, 1, fmt::format("{} - Type '{}' - Beam Tiles", lbl, type_name));
					break;
				}
			}
		}
		if(!every_proc && !movelist->check_prot())
			return false;
	}
	//Items
	{
		auto& movelist = vec.emplace_back(std::make_unique<TileMoveList>(
			dest_process, source_process, mode,
			move
			? "The tiles used by the following items will be partially cleared by the move."
			: "The tiles used by the following items will be partially or completely overwritten by this process."
			));
		build_bii_list(false);
		for(int32_t u=0; u<MAXITEMS; u++)
		{
			auto id = bii[u].i;
			itemdata& itm = itemsbuf[id];
			if(itm.family == itype_bottle)
			{
				vector<std::tuple<int,int,int>> rects;
				auto fr = itm.frames;
				for(int q = 0; q < NUM_BOTTLE_TYPES; ++q)
				{
					bottletype const& bt = QMisc.bottle_types[q];
					if(bt.is_blank())
						continue;
					rects.emplace_back(fr+q*fr, fr, 1);
				}
				movelist->add_tile(&itm.tile, fr, 1, fmt::format("Item {}", id),
					false, 0, 0, rects);
			}
			else movelist->add_tile(&itm.tile, itm.frames, 1, fmt::format("Item {}", id));
		}
		if(!every_proc && !movelist->check_prot())
			return false;
	}
	//Weapon sprites
	{
		auto& movelist = vec.emplace_back(std::make_unique<TileMoveList>(
			dest_process, source_process, mode,
			move
			? "The tiles used by the following weapons will be partially cleared by the move."
			: "The tiles used by the following weapons will be partially or completely overwritten by this process."
			));
		build_biw_list();
		
		for(int32_t u=0; u<MAXWPNS; u++)
		{
			bool ignore_frames=false;
			int32_t m=0;
			
			auto id = biw[u].i;
			auto& wpn = wpnsbuf[id];
			
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
				if(get_qr(qr_HARDCODED_ENEMY_ANIMS))
				{
					ignore_frames = true;
					m=BSZ2?4:2;
				}
				break;
				
			case iwSpawn:
				if(get_qr(qr_HARDCODED_ENEMY_ANIMS))
				{
					ignore_frames = true;
					m=3;
				}
				break;
			}
			
			movelist->add_tile(&wpn.tile, zc_max((ignore_frames?0:wpn.frames),1)+m,
				1, fmt::format("{} {}", biw[u].s, id));
			
			//Tile 54+55 are "Impact (not shown in sprite list)", for u==3 "Arrow" and u==9 "Boomerang"
			//...these can't be updated by a move.
			if((u==3)||(u==9))
			{
				static int32_t impact_tiles[2] = {54,54};
				auto& tile = impact_tiles[u==3 ? 0 : 1];
				tile = 54; //dummy tile, ensure it's correct
				movelist->add_tile(&tile, 2, 1,
					fmt::format("{} Impact (not shown in sprite list)",(u==3)?"Arrow":"Boomerang"),
					true);
			}
		}
		if(!every_proc && !movelist->check_prot())
			return false;
	}
	//Hero sprites
	{
		auto& movelist = vec.emplace_back(std::make_unique<TileMoveList>(
			dest_process, source_process, mode,
			move
			? "The tiles used by the following Hero sprites will be partially cleared by the move."
			: "The tiles used by the following Hero sprites will be partially or completely overwritten by this process."
			));
		{
			int32_t a_style=(zinit.heroAnimationStyle);
			#define ADD_HERO_SPRITE(ref_sprite, frames, name) \
			do \
			{ \
				movelist->add_tile(&ref_sprite[spr_tile], \
					(ref_sprite[spr_extend] < 2 ? 1 : 2) * frames, \
					ref_sprite[spr_extend] < 1 ? 1 : 2, \
					name, false, \
					ref_sprite[spr_extend] < 2 ? 0 : -1, \
					ref_sprite[spr_extend] < 1 ? 0 : -1); \
			} while(false)
			// + (ref_sprite[spr_extend] < 2 ? 0 : 1) //this was on some of the 'width's before... but doesn't make sense?
			
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(walkspr[i], quick_select_3(a_style, (i==0?1:2), 3, 9), fmt::format("Walking ({})", dirstr_proper[i]));
			}
			
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(slashspr[i], quick_select_3(a_style, 1, 1, 6), fmt::format("Slashing ({})", dirstr_proper[i]));
			}
			
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(stabspr[i], quick_select_3(a_style, 1, 1, 3), fmt::format("Stabbing ({})", dirstr_proper[i]));
			}
			
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(poundspr[i], quick_select_3(a_style, 1, 1, 3), fmt::format("Pounding ({})", dirstr_proper[i]));
			}
			
			for(int32_t i=0; i<2; ++i)
			{
				ADD_HERO_SPRITE(holdspr[0][i], 1, fmt::format("Hold (Land, {}-hand)", i+1));
			}
			
			ADD_HERO_SPRITE(castingspr, 1, "Casting");
			
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(floatspr[i], quick_select_3(a_style, 2, 3, 4), fmt::format("Floating ({})", dirstr_proper[i]));
			}
			
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(swimspr[i], quick_select_3(a_style, 2, 3, 4), fmt::format("Swimming ({})", dirstr_proper[i]));
			}
			
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(divespr[i], quick_select_3(a_style, 2, 3, 4), fmt::format("Diving ({})", dirstr_proper[i]));
			}
			
			for(int32_t i=0; i<2; ++i)
			{
				ADD_HERO_SPRITE(holdspr[1][i], 1, fmt::format("Hold (Water, {}-hand)", i));
			}
			
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(jumpspr[i], 3, fmt::format("Jumping ({})", dirstr_proper[i]));
			}
			
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(chargespr[i], quick_select_3(a_style, 2, 3, 9), fmt::format("Charging ({})", dirstr_proper[i]));
			}
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(revslashspr[i], quick_select_3(a_style, 1, 1, 6), fmt::format("Slash 2 ({})", dirstr_proper[i]));
			}
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(fallingspr[i], 7, fmt::format("Falling ({})", dirstr_proper[i]));
			}
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(liftingspr[i], liftingspr[i][spr_frames], fmt::format("Lifting ({})", dirstr_proper[i]));
			}
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(liftingwalkspr[i], quick_select_3(a_style, (i==0?1:2), 3, 9), fmt::format("Lift-Walking ({})", dirstr_proper[i]));
			}
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(drowningspr[i], quick_select_3(a_style, 2, 3, 3), fmt::format("Drowning ({})", dirstr_proper[i]));
			}
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(drowning_lavaspr[i], quick_select_3(a_style, 2, 3, 3), fmt::format("Lava Drowning ({})", dirstr_proper[i]));
			}
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(sideswimspr[i], quick_select_3(a_style, 2, 3, 3), fmt::format("Side-Swimming ({})", dirstr_proper[i]));
			}
			//69
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(sideswimslashspr[i], quick_select_3(a_style, 1, 1, 6), fmt::format("Side-Swim Slash ({})", dirstr_proper[i]));
			}
			//73
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(sideswimstabspr[i], quick_select_3(a_style, 1, 1, 3), fmt::format("Side-Swim Stab ({})", dirstr_proper[i]));
			}
			//77
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(sideswimpoundspr[i], quick_select_3(a_style, 1, 1, 3), fmt::format("Side-Swim Pound ({})", dirstr_proper[i]));
			}
			//81
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(sideswimchargespr[i], quick_select_3(a_style, 2, 3, 9), fmt::format("Side-Swim Charging ({})", dirstr_proper[i]));
			}
			//85
			ADD_HERO_SPRITE(sideswimholdspr[spr_hold1], 1, "Hold (Side-Water, 1-hand)");
			ADD_HERO_SPRITE(sideswimholdspr[spr_hold2], 1, "Hold (Side-Water, 2-hand)");
			ADD_HERO_SPRITE(sideswimcastingspr, 1, "Side-Swim Casting");
			for(int32_t i=0; i<4; ++i)
			{
				ADD_HERO_SPRITE(sidedrowningspr[i], quick_select_3(a_style, 2, 3, 3), fmt::format("Side-Swim Drowning ({})", dirstr_proper[i]));
			}
			//91
		}
		if(!every_proc && !movelist->check_prot())
			return false;
	}
	//Map Styles
	{
		auto& movelist = vec.emplace_back(std::make_unique<TileMoveList>(
			dest_process, source_process, mode,
			move
			? "The tiles used by the following map styles will be partially cleared by the move."
			: "The tiles used by the following map styles will be partially or completely overwritten by this process."
			));
		movelist->add_tile(&QMisc.colors.blueframe_tile, 2, 2, "Frame");
		movelist->add_tile(&QMisc.colors.HCpieces_tile, zinit.hcp_per_hc, 1, "Heart Container Piece");
		movelist->add_tile(&QMisc.colors.triforce_tile, BSZ2?2:1, BSZ2?3:1, "McGuffin Fragment");
		movelist->add_tile(&QMisc.colors.triframe_tile, BSZ2?7:6, BSZ2?7:3, "McGuffin Frame");
		movelist->add_tile(&QMisc.colors.overworld_map_tile, 5, 3, "Overworld Map");
		movelist->add_tile(&QMisc.colors.dungeon_map_tile, 5, 3, "Dungeon Map");
		if(!every_proc && !movelist->check_prot())
			return false;
	}
	//Game Icons
	{
		auto& movelist = vec.emplace_back(std::make_unique<TileMoveList>(
			dest_process, source_process, mode,
			move
			? "The tiles used by the following game icons will be partially cleared by the move."
			: "The tiles used by the following game icons will be partially or completely overwritten by this process."
			));
		for(int32_t u=0; u<4; u++)
			movelist->add_tile(&QMisc.icons[u], fmt::format("Game Icon {}", u));
		if(!every_proc && !movelist->check_prot())
			return false;
	}
	//DMaps
	{
		auto& movelist = vec.emplace_back(std::make_unique<TileMoveList>(
			dest_process, source_process, mode,
			move
			? "The tiles used by the following dmaps will be partially cleared by the move."
			: "The tiles used by the following dmaps will be partially or completely overwritten by this process."
			));
		for(int32_t u=0; u<MAXDMAPS; u++)
		{
			auto& dm = DMaps[u];
			movelist->add_tile(&dm.minimap_1_tile, 5, 3, fmt::format("DMap {} - Minimap (Empty)", u));
			movelist->add_tile(&dm.minimap_2_tile, 5, 3, fmt::format("DMap {} - Minimap (Filled)", u));
			movelist->add_tile(&dm.largemap_1_tile, BSZ2?7:9, 5, fmt::format("DMap {} - Large Map (Empty)", u));
			movelist->add_tile(&dm.largemap_2_tile, BSZ2?7:9, 5, fmt::format("DMap {} - Large Map (Filled)", u));
		}
		if(!every_proc && !movelist->check_prot())
			return false;
	}
	//Enemies
	{
		auto& movelist = vec.emplace_back(std::make_unique<TileMoveList>(
			dest_process, source_process, mode,
			move
			? "The tiles used by the following enemies will be partially cleared by the move."
			: "The tiles used by the following enemies will be partially or completely overwritten by this process."
			));
		build_bie_list(false);
		bool newtiles=get_qr(qr_NEWENEMYTILES)!=0;
		for(int u=0; u<eMAXGUYS; u++)
		{
			guydata& enemy=guysbuf[bie[u].i];
			bool darknut=false;
			bool gleeok=false;
			
			if(enemy.family==eeWALK && ((enemy.flags&(guy_shield_back|guy_shield_front|guy_shield_left|guy_shield_right))!=0))
				darknut=true;
			else if(enemy.family==eeGLEEOK)
				gleeok=true;
			else if (enemy.family == eePATRA)
			{
				if (!get_qr(qr_PATRAS_USE_HARDCODED_OFFSETS))
				{
					darknut=true; //uses the same logic no need for separate variables!
				}
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
				if(enemy.e_tile==0)
				{
					continue;
				}
				
				vector<std::tuple<int,int,int>> rects;
				
				if(darknut) //or anything that uses S. Tile for with new tiles
				{
					if (enemy.s_tile != 0)
					{
						movelist->add_tile(&enemy.s_tile, enemy.s_width, enemy.s_height, fmt::format("Enemy {} ({}) 'Special'", u, bie[u].s));
					}
				}
				else if (gleeok)
				{
					for (int32_t j = 0; j < enemy.attributes[4]; ++j)
					{
						rects.emplace_back(enemy.attributes[5] + (enemy.attributes[6]*j), 4, 1);
					}
					rects.emplace_back(enemy.attributes[7], 4, 1);
					rects.emplace_back(enemy.attributes[8], 4, 1);
				}
				movelist->add_tile(&enemy.e_tile, enemy.e_width, enemy.e_height, fmt::format("Enemy {} ({}) 'New'", u, bie[u].s),
					false, 0, 0, rects);
				
			}
			else
			{
				if(enemy.tile==0)
				{
					continue;
				}
				movelist->add_tile(&enemy.tile, enemy.width, enemy.height, fmt::format("Enemy {} ({}) 'Old'", u, bie[u].s));
				
				if(enemy.s_tile!=0)
				{
					movelist->add_tile(&enemy.s_tile, enemy.s_width, enemy.s_height, fmt::format("Enemy {} ({}) 'Special'", u, bie[u].s));
				}
			}
		}
		if(!every_proc && !movelist->check_prot())
			return false;
	}
	//Subscreens
	{
		auto& movelist = vec.emplace_back(std::make_unique<TileMoveList>(
			dest_process, source_process, mode,
			move
			? "The tiles used by the following subscreen widgets will be partially cleared by the move."
			: "The tiles used by the following subscreen widgets will be partially or completely overwritten by this process."
			));
		
		for(auto q = 0; q < subscreens_active.size(); ++q)
		{
			size_t indx = movelist->move_refs.size();
			collect_subscreen_tiles(subscreens_active[q], *movelist.get());
			for(; indx < movelist->move_refs.size(); ++indx)
			{
				auto& ref = movelist->move_refs[indx];
				ref->name = fmt::format("Active Subscr {} - {}", q, ref->name);
			}
		}
		for(auto q = 0; q < subscreens_passive.size(); ++q)
		{
			size_t indx = movelist->move_refs.size();
			collect_subscreen_tiles(subscreens_passive[q], *movelist.get());
			for(; indx < movelist->move_refs.size(); ++indx)
			{
				auto& ref = movelist->move_refs[indx];
				ref->name = fmt::format("Passive Subscr {} - {}", q, ref->name);
			}
		}
		for(auto q = 0; q < subscreens_overlay.size(); ++q)
		{
			size_t indx = movelist->move_refs.size();
			collect_subscreen_tiles(subscreens_overlay[q], *movelist.get());
			for(; indx < movelist->move_refs.size(); ++indx)
			{
				auto& ref = movelist->move_refs[indx];
				ref->name = fmt::format("Overlay Subscr {} - {}", q, ref->name);
			}
		}
		if(!every_proc && !movelist->check_prot())
			return false;
	}
	//Strings
	{
		auto& movelist = vec.emplace_back(std::make_unique<TileMoveList>(
			dest_process, source_process, mode,
			move
			? "The tiles used by the following strings will be partially cleared by the move."
			: "The tiles used by the following strings will be partially or completely overwritten by this process."
			));
		for(size_t q = 0; q < msg_count; ++q)
		{
			MsgStr& str = MsgStrings[q];
			std::string text = str.serialize();
			bool fulltile = str.stringflags & STRINGFLAG_FULLTILE;
			movelist->add_tile(&str.tile, fulltile ? (str.w/16_zf).getCeil() : 2,
				fulltile ? (str.h/16_zf).getCeil() : 2, fmt::format("{} (BG): '{}'", q, util::snip(text,100)));
			movelist->add_tile(&str.portrait_tile, str.portrait_tw, str.portrait_th,
				fmt::format("{} (Port.): '{}'", q, util::snip(text,100)));
		}
		if(!every_proc && !movelist->check_prot())
			return false;
	}
	
	if(source_process) //Apply the 'diff' value to all moved tiles
		storage.redo();
	if(every_proc)
		for(auto &list : vec)
			for(auto &ref : list->move_refs)
				ref->forEach(every_proc);
	return true;
}
bool handle_tile_move(TileMoveProcess dest_process)
{
	return _handle_tile_move(dest_process, nullopt, 0);
}
bool handle_tile_move(TileMoveProcess dest_process, TileMoveProcess source_process, int diff, TileMoveUndo& on_undo)
{
	return _handle_tile_move(dest_process, source_process, diff, &on_undo);
}
void for_every_used_tile(std::function<void(int32_t)> proc)
{
	reset_combo_animations();
	reset_combo_animations2();
	TileMoveProcess all_tiles {.rect = false, ._first = 0, ._last = NEWMAXTILES-1};
	_handle_tile_move(all_tiles, nullopt, 0, nullptr, proc, TileMoveList::Mode::CHECK_ALL);
}

bool _handle_combo_move(ComboMoveProcess dest_process, optional<ComboMoveProcess> source_process, int diff, ComboMoveUndo* on_undo)
{
	bool BSZ2 = get_qr(qr_BSZELDA);
	bool move = source_process.has_value();
	ComboMoveUndo local_undo;
	ComboMoveUndo& storage = on_undo ? *on_undo : local_undo;
	auto& vec = storage.vec;
	auto& combo_links = storage.combo_links;
	storage.diff = diff;
	storage.state = false;
	//Combo relative links
	{
		for(int32_t q = 0; q < MAXCOMBOS; ++q)
		{
			newcombo& cmb = combobuf[q];
			for(size_t idx = 0; idx < cmb.triggers.size(); ++idx)
			{
				auto& trig = cmb.triggers[idx];
				if(trig.trigchange)
					combo_links.add_to(q, q+trig.trigchange);
			}
			bool next = cmb.flag == mfSECRETSNEXT;
			switch(cmb.type)
			{
				case cPOUND:
				case cLOCKBLOCK: case cLOCKBLOCK2:
				case cBOSSLOCKBLOCK: case cBOSSLOCKBLOCK2:
				case cCHEST: case cCHEST2:
				case cLOCKEDCHEST: case cLOCKEDCHEST2:
				case cBOSSCHEST: case cBOSSCHEST2:
				case cSTEP: case cSTEPSAME: case cSTEPALL: case cSTEPCOPY:
				case cSLASHNEXT: case cSLASHNEXTITEM: case cBUSHNEXT:
				case cSLASHNEXTTOUCHY: case cSLASHNEXTITEMTOUCHY: case cBUSHNEXTTOUCHY:
				case cTALLGRASSNEXT: case cCRUMBLE:
					next = true;
					break;
				case cCSWITCH: case cCSWITCHBLOCK:
					combo_links.add_to(q, q+cmb.attributes[0]);
					break;
				case cLIGHTTARGET:
					if(cmb.usrflags & cflag1)
						combo_links.add_to(q, q-1);
					else next = true;
					break;
				case cSTEPSFX:
					if((cmb.usrflags&(cflag1|cflag3)) == cflag1)
						next = true;
					break;
			}
			if(next)
				combo_links.add_to(q, q+1);
		}
	}
	
	//This function is expensive! Any optimizations possible should be made. -Em
	
	//OPT: Check for a 0-val preemptively, to avoid processing the fmt::format strings
	#define ADDC(ptr, str) \
	if(*ptr) movelist->add_combo(ptr, ComboProtection ? str : "");
	#define ADDC_10k(ptr, str) \
	if(*ptr) movelist->add_combo_10k(ptr, ComboProtection ? str : "");
	//Combos
	{
		auto& movelist = vec.emplace_back(std::make_unique<ComboMoveList>(
			combo_links, dest_process, source_process,
			move
			? "The combos used by the following combos will be partially cleared by the move."
			: "The combos used by the following combos will be partially or completely overwritten by this process."
			));
		for(int32_t q = 0; q < MAXCOMBOS; ++q)
		{
			newcombo& cmb = combobuf[q];
			auto lbl = fmt::format("{}{}", q, cmb.label.empty() ? ""
				: fmt::format(" ({})", cmb.label));
			ADDC(&cmb.nextcombo, fmt::format("{} - Combo Cycle", lbl));
			ADDC(&cmb.liftcmb, fmt::format("{} - Lift Combo", lbl));
			ADDC(&cmb.liftundercmb, fmt::format("{} - Lift Undercombo", lbl));
			for(auto& trig : cmb.triggers)
				ADDC(&trig.prompt_cid, fmt::format("{} - Triggers ButtonPrompt", lbl));
			
			//type-specific
			char const* type_name = ZI.getComboTypeName(cmb.type);
			switch(cmb.type)
			{
				case cLOCKEDCHEST: case cBOSSCHEST:
					if(cmb.usrflags & cflag13)
						ADDC_10k(&cmb.attributes[2], fmt::format("{} - Type '{}' - Locked Prompt", lbl, type_name));
				[[fallthrough]];
				case cCHEST:
					if(cmb.usrflags & cflag13)
						ADDC_10k(&cmb.attributes[1], fmt::format("{} - Type '{}' - Prompt", lbl, type_name));
					break;
				case cLOCKBLOCK: case cBOSSLOCKBLOCK:
					if(cmb.usrflags & cflag13)
					{
						ADDC_10k(&cmb.attributes[1], fmt::format("{} - Type '{}' - Prompt", lbl, type_name));
						ADDC_10k(&cmb.attributes[2], fmt::format("{} - Type '{}' - Locked Prompt", lbl, type_name));
					}
					break;
				case cSIGNPOST:
					if(cmb.usrflags & cflag13)
						ADDC_10k(&cmb.attributes[1], fmt::format("{} - Type '{}' - Prompt", lbl, type_name));
					break;
				case cBUTTONPROMPT:
					if(cmb.usrflags & cflag13)
						ADDC_10k(&cmb.attributes[0], fmt::format("{} - Type '{}' - Prompt", lbl, type_name));
					break;
			}
		}
		
		if(!movelist->check_prot())
			return false;
	}
	//Door Combo Sets
	{
		auto& movelist = vec.emplace_back(std::make_unique<ComboMoveList>(
			combo_links, dest_process, source_process,
			move
			? "The combos used by the following screens will be partially cleared by the move."
			: "The combos used by the following screens will be partially or completely overwritten by this process."
			));
		static const char* door_names[9] = {
			"Wall", "Locked", "Shuttered", "Boss", "Bombed", "Open", "Unlocked", "Open Shuttered", "Open Boss"
		};
		for(int32_t i=0; i<MAXDOORCOMBOSETS; i++)
		{
			auto& dcs = DoorComboSets[i];
			auto& name = DoorComboSetNames[i];			
			for(int32_t j=0; j<9; j++)
			{
				if(j<4)
				{
					ADDC(&dcs.walkthroughcombo[j], fmt::format("{} ({}): Walk-Through {}", i, name, j));
					
					if(j<3)
					{
						if(j<2)
						{
							ADDC(&dcs.bombdoorcombo_u[j], fmt::format("{} ({}): Unused? bombdoorcombo_u {}", i, name, j));
							ADDC(&dcs.bombdoorcombo_d[j], fmt::format("{} ({}): Unused? bombdoorcombo_d {}", i, name, j));
						}
						ADDC(&dcs.bombdoorcombo_l[j], fmt::format("{} ({}): Unused? bombdoorcombo_l {}", i, name, j));
						ADDC(&dcs.bombdoorcombo_r[j], fmt::format("{} ({}): Unused? bombdoorcombo_r {}", i, name, j));
					}
				}
				
				for(int32_t k=0; k<6; k++)
				{
					if(k<4)
					{
						ADDC(&dcs.doorcombo_u[j][k], fmt::format("{} ({}): Top, {} #{}", i, name, door_names[j], k));
						ADDC(&dcs.doorcombo_d[j][k], fmt::format("{} ({}): Bottom, {} #{}", i, name, door_names[j], k));
					}
					
					ADDC(&dcs.doorcombo_l[j][k], fmt::format("{} ({}): Left, {} #{}", i, name, door_names[j], k));
					ADDC(&dcs.doorcombo_r[j][k], fmt::format("{} ({}): Right, {} #{}", i, name, door_names[j], k));
				}
			}
		}
		
		if(!movelist->check_prot())
			return false;
	}
	//Combo Pools
	{
		auto& movelist = vec.emplace_back(std::make_unique<ComboMoveList>(
			combo_links, dest_process, source_process,
			move
			? "The combos used by the following combo pools will be partially cleared by the move."
			: "The combos used by the following combo pools will be partially or completely overwritten by this process."
			));
		for(auto q = 0; q < MAXCOMBOPOOLS; ++q)
		{
			combo_pool& pool = combo_pools[q];
			int idx = 0;
			for(cpool_entry& cp : pool.combos)
				ADDC(&cp.cid, fmt::format("{} index {}", q, idx++));
		}
		
		if(!movelist->check_prot())
			return false;
	}
	//Auto Combos
	{
		auto& movelist = vec.emplace_back(std::make_unique<ComboMoveList>(
			combo_links, dest_process, source_process,
			move
			? "The combos used by the following autocombos will be partially cleared by the move."
			: "The combos used by the following autocombos will be partially or completely overwritten by this process."
			));
		for (auto q = 0; q < MAXAUTOCOMBOS; ++q)
		{
			combo_auto& cauto = combo_autos[q];
			int idx = 0;
			for (autocombo_entry& ac : cauto.combos)
				ADDC(&ac.cid, fmt::format("{} index {}", q, idx++));
			ADDC(&cauto.cid_erase, fmt::format("{} Erase Combo", q));
			ADDC(&cauto.cid_display, fmt::format("{} Display Combo", q));
		}
		
		if(!movelist->check_prot())
			return false;
	}
	//Combo Aliases
	{
		auto& movelist = vec.emplace_back(std::make_unique<ComboMoveList>(
			combo_links, dest_process, source_process,
			move
			? "The combos used by the following aliases will be partially cleared by the move."
			: "The combos used by the following aliases will be partially or completely overwritten by this process."
			));
		for(int32_t i=0; i<MAXCOMBOALIASES; i++)
		{
			//dimensions are 1 less than you would expect -DD
			int32_t count=(comboa_lmasktotal(combo_aliases[i].layermask)+1)*(combo_aliases[i].width+1)*(combo_aliases[i].height+1);
			
			for(int32_t j=0; j<count; j++)
			{
				ADDC(&combo_aliases[i].combos[j], fmt::format("{} index {}", i, j));
			}
		}
		
		if(!movelist->check_prot())
			return false;
	}
	//Favorite Combos
	{
		auto& movelist = vec.emplace_back(std::make_unique<ComboMoveList>(
			combo_links, dest_process, source_process,
			move
			? "The combos used by the following favorite combos will be partially cleared by the move."
			: "The combos used by the following favorite combos will be partially or completely overwritten by this process."
			));
		for(int32_t i=0; i<MAXFAVORITECOMBOS; i++)
		{
			if(favorite_combo_modes[i] != dm_normal) //don't hit pools/aliases/autos, only combos!
				continue;
			ADDC(&favorite_combos[i], fmt::format("Favorite {}", i));
		}
		
		if(!movelist->check_prot())
			return false;
	}
	//Bottle Shops
	{
		auto& movelist = vec.emplace_back(std::make_unique<ComboMoveList>(
			combo_links, dest_process, source_process,
			move
			? "The combos used by the following bottle shops will be partially cleared by the move."
			: "The combos used by the following bottle shops will be partially or completely overwritten by this process."
			));
		for(auto q = 0; q < 256; ++q)
			for(auto p = 0; p < 3; ++p)
				ADDC(&QMisc.bottle_shop_types[q].comb[p], fmt::format("{} slot {}", q, p));
		
		if(!movelist->check_prot())
			return false;
	}
	//Screens //EXPENSIVE! DO THIS LAST!
	{
		auto& movelist = vec.emplace_back(std::make_unique<ComboMoveList>(
			combo_links, dest_process, source_process,
			move
			? "The combos used by the following screens will be partially cleared by the move."
			: "The combos used by the following screens will be partially or completely overwritten by this process."
			));		
		
		for(int32_t i=0; i<map_count && i<MAXMAPS; i++)
		{
			for(int32_t j=0; j<MAPSCRS; j++)
			{
				mapscr& scr = TheMaps[i*MAPSCRS+j];
				
				if(!(scr.valid&mVALID))
					continue;
				
				ADDC(&scr.undercombo, fmt::format("{}x{:02X} - UnderCombo", i, j));

				// Specifying the exact position is too expensive - too much string creation.
				std::string data_str = ComboProtection ? fmt::format("{}x{:02X} - Combo", i, j) : "";
				for(int32_t k=0; k<176; k++)
					ADDC(&scr.data[k], data_str);
				
				for(int32_t k=0; k<128; k++)
					ADDC(&scr.secretcombo[k], fmt::format("{}x{:02X} - SecretCombo {}", i, j, k));
				
				word maxffc = scr.numFFC();
				for(word k=0; k<maxffc; k++)
				{
					ffcdata& ffc = scr.ffcs[k];
					ADDC(&ffc.data, fmt::format("{}x{:02X} - FFC {}", i, j, k+1));
				}
			}
		}
		
		if(!movelist->check_prot())
			return false;
	}
	if(source_process) //Apply the 'diff' value to all moved combos
		storage.redo();
	return true;
}

bool handle_combo_move(ComboMoveProcess dest_process)
{
	return _handle_combo_move(dest_process, nullopt, 0, nullptr);
}
bool handle_combo_move(ComboMoveProcess dest_process, ComboMoveProcess source_process, int diff, ComboMoveUndo& on_undo)
{
	return _handle_combo_move(dest_process, source_process, diff, &on_undo);
}
void register_used_tiles()
{
	memset(used_tile_table, 0, sizeof(used_tile_table));
	for_every_used_tile([&](int tile)
		{
			used_tile_table[tile] = true;
		});
}

bool overlay_tiles(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move, int32_t cs, bool backwards)
{
	bool ctrl=(CHECK_CTRL_CMD);
	bool copied=false;
	copied=overlay_tiles_united(tile,tile2,copy,copycnt,rect_sel,move,cs,backwards);
	
	if(copied)
	{
		saved=false;
	}
	
	return copied;
}

bool overlay_tiles_united(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect, bool move, int32_t cs, bool backwards)
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
	
	
	
	char buf2[80], buf3[80], buf4[80];
	sprintf(buf2, " ");
	sprintf(buf3, " ");
	sprintf(buf4, " ");
	
	// warn if range extends beyond last tile
	sprintf(buf4, "Some tiles will not be %s", move?"moved.":"copied.");
	
	if(dest_last>=NEWMAXTILES)
	{
		sprintf(buf4, "%s operation cancelled.", move?"Move":"Copy");
		jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", NULL, 'o', 0, get_zc_font(font_lfont));
		return false;
//fix this below to allow the operation to complete with a modified start or end instead of just cancelling
		//if (jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", "&Cancel", 'o', 'c', get_zc_font(font_lfont))==2)
		// {
		//  return false;
		// }
	}
	
	
	TileMoveUndo on_undo;
	// Overwrite warnings
	TileMoveProcess dest{rect, dest_left, dest_top, dest_width, dest_height, dest_first, dest_last};
	if(move)
	{
		TileMoveProcess src{rect, src_left, src_top, src_width, src_height, src_first, src_last};
		if(!handle_tile_move(dest, src, dest_first-src_first, on_undo))
			return false;
	}
	else
	{
		if(!handle_tile_move(dest))
			return false;
	}
	// copy tiles and delete if needed (move)
	
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
	}
	
	//now that tiles have moved, fix these buffers -DD
	register_blank_tiles();
	register_used_tiles();
	
	if(move)
		last_tile_move_list = std::move(on_undo);
	return true;
}
//
bool do_movetile_united(tile_move_data const& tmd)
{
	char buf2[80], buf3[80], buf4[80];
	sprintf(buf2, " ");
	sprintf(buf3, " ");
	sprintf(buf4, " ");
	
	// warn if range extends beyond last tile
	sprintf(buf4, "Some tiles will not be %s", tmd.move?"moved.":"copied.");
	
	if(tmd.dest_last>=NEWMAXTILES)
	{
		sprintf(buf4, "%s operation cancelled.", tmd.move?"Move":"Copy");
		jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", NULL, 'o', 0, get_zc_font(font_lfont));
		return false;
	}
	
	TileMoveUndo on_undo;
	// Overwrite warnings
	TileMoveProcess dest{tmd.rect, tmd.dest_left, tmd.dest_top, tmd.dest_width, tmd.dest_height, tmd.dest_first, tmd.dest_last};
	if(tmd.move)
	{
		TileMoveProcess src{tmd.rect, tmd.src_left, tmd.src_top, tmd.src_width, tmd.src_height, tmd.src_first, tmd.src_last};
		if(!handle_tile_move(dest, src, tmd.dest_first-tmd.src_first, on_undo))
			return false;
	}
	else
	{
		if(!handle_tile_move(dest))
			return false;
	}
	
	// copy tiles and delete if needed (tmd.move)
	{
		go_tiles();
		
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
	}
	
	//now that tiles have moved, fix these buffers -DD
	register_blank_tiles();
	register_used_tiles();
	
	if(tmd.move)
		last_tile_move_list = std::move(on_undo);
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
	
	return do_movetile_united(tmd);
}

//

bool copy_tiles_united_floodfill(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect, bool move)
{
	assert(!move); //not implemented
	
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
	
	
	
	char buf2[80], buf3[80], buf4[80];
	sprintf(buf2, " ");
	sprintf(buf3, " ");
	sprintf(buf4, " ");
	
	// warn if range extends beyond last tile
	sprintf(buf4, "Some tiles will not be %s", move?"moved.":"copied.");
	
	if(tmd.dest_last>=NEWMAXTILES)
	{
		sprintf(buf4, "%s operation cancelled.", move?"Move":"Copy");
		jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", NULL, 'o', 0, get_zc_font(font_lfont));
		return false;
		//fix this below to allow the operation to complete with a modified start or end instead of just cancelling
		//if (jwin_alert("Destination Error", "The destination extends beyond", "the last available tile row.", buf4, "&OK", "&Cancel", 'o', 'c', get_zc_font(font_lfont))==2)
		// {
		//  return false;
		// }
	}
	
	TileMoveUndo on_undo;
	// Overwrite warnings
	TileMoveProcess dest{tmd.rect, tmd.dest_left, tmd.dest_top, tmd.dest_width, tmd.dest_height, tmd.dest_first, tmd.dest_last};
	if(tmd.move)
	{
		TileMoveProcess src{tmd.rect, tmd.src_left, tmd.src_top, tmd.src_width, tmd.src_height, tmd.src_first, tmd.src_last};
		if(!handle_tile_move(dest, src, tmd.dest_first-tmd.src_first, on_undo))
			return false;
	}
	else
	{
		if(!handle_tile_move(dest))
			return false;
	}
	
	// copy tiles and delete if needed (move)
	
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
		
	if(tmd.move)
		last_tile_move_list = std::move(on_undo);
	return true;
}
//

bool copy_tiles_floodfill(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move)
{
	al_trace("Floodfill Psste\n");
	bool ctrl=(CHECK_CTRL_CMD);
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
	bool ctrl=(CHECK_CTRL_CMD);
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

bool scale_or_rotate_tiles(int32_t &tile, int32_t &tile2, int32_t &cs, bool rotate)
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
	zfix dest_rot = 0_zf;
	if(rotate)
		RotateTileDialog(&dest_width, &dest_height, &dest_rot).show();
	else
		ScaleTileDialog(&dest_width, &dest_height).show();
	if (rotate)
	{
		if (dest_rot == 0) return false;
	}
	else
	{
		if (dest_width == src_width && dest_height == src_height) return false; //no scaling
	}
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
	
	// Overwrite warnings
	if(!handle_tile_move({true, dest_left, dest_top, dest_width, dest_height, dest_first, dest_last}))
		return false;
	
	//Do the rotate
	{
		go_tiles();
		
		int32_t diff=dest_first-src_first;
		BITMAP *srcbmp = create_bitmap_ex(8,src_width*16,src_height*16),
			*destbmp = create_bitmap_ex(8,dest_width*16,dest_height*16);
		clear_bitmap(srcbmp); clear_bitmap(destbmp);
		overtileblock16(srcbmp, src_first, 0, 0, src_width, src_height, cs, 0);
		bool is8bit = newtilebuf[src_first].format == tf8Bit;
		if (rotate)
		{
			rotate_sprite(destbmp, srcbmp, 0, 0, ftofix(dest_rot * 0.7111111111111));
		}
		else
		{
			stretch_blit(srcbmp, destbmp, 0, 0, srcbmp->w, srcbmp->h,
				0, 0, destbmp->w, destbmp->h);
		}
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
	return true;
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
	
	auto first = tile;
	auto last = masscopy ? tile2 : first + copycnt-1;
	if(!handle_combo_move({first,last}))
		return;
	
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
}

bool do_movecombo(combo_move_data const& cmd, ComboMoveUndo& on_undo, bool is_undoing)
{
	reset_combo_animations();
	reset_combo_animations2();
	go_combos();
	
	auto diff = cmd.tile - cmd.copy1;
	if(is_undoing)
		on_undo.undo();
	else if(!handle_combo_move({cmd.tile,cmd.tile+cmd.copycnt-1},{cmd.copy1,cmd.copy1+cmd.copycnt-1}, diff, on_undo))
		return false;
	
	for(int32_t t=(cmd.tile<cmd.copy1)?0:(cmd.copycnt-1); (cmd.tile<cmd.copy1)?(t<cmd.copycnt):(t>=0); (cmd.tile<cmd.copy1)?(t++):(t--))
	{
		if(cmd.tile+t < MAXCOMBOS)
		{
			combobuf[cmd.tile+t]=combobuf[cmd.copy1+t];
			clear_combo(cmd.copy1+t);
		}
	}
	
	setup_combo_animations();
	setup_combo_animations2();
	saved=false;
	return true;
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
	
	ComboMoveUndo on_undo;
	if(!do_movecombo(cmd, on_undo))
		return;
	last_combo_move_list = std::move(on_undo);
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
	
	if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',get_zc_font(font_lfont))==1)
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
	
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-25-6)>>1;
	jwin_draw_win(screen, window_xofs, window_yofs, w+6+6, h+25+6, FR_WIN);
	jwin_draw_frame(screen, window_xofs+4, window_yofs+23, w+2+2, h+4+2-64,  FR_DEEP);
	
	FONT *oldfont = font;
	font = get_zc_font(font_lfont);
	jwin_draw_titlebar(screen, window_xofs+3, window_yofs+3, w+6, 18, "Select Tile", true);
	font=oldfont;
	return;
}

void show_blank_tile(int32_t t)
{
	char tbuf[80], tbuf2[80], tbuf3[80];
	sprintf(tbuf, "Tile is%s blank.", blank_tile_table[t]?"":" not");
	sprintf(tbuf2, "%c %c", blank_tile_quarters_table[t*4]?'X':'-', blank_tile_quarters_table[(t*4)+1]?'X':'-');
	sprintf(tbuf3, "%c %c", blank_tile_quarters_table[(t*4)+2]?'X':'-', blank_tile_quarters_table[(t*4)+3]?'X':'-');
	jwin_alert("Blank Tile Information",tbuf,tbuf2,tbuf3,"&OK",NULL,13,27,get_zc_font(font_lfont));
}

static void do_convert_tile(int32_t tile, int32_t tile2, int32_t cs, bool rect_sel, int format, bool shift, bool alt, bool skip_prompt = false)
{
	int num_bits;
	if (format == tf4Bit)
		num_bits = 4;
	else if (format == tf8Bit)
		num_bits = 8;
	else assert(false);

	char buf[80];
	sprintf(buf, "Do you want to convert the selected %s to %d-bit color?", tile==tile2?"tile":"tiles",num_bits);
	
	if (skip_prompt || jwin_alert("Convert Tile?",buf,NULL,NULL,"&Yes","&No",'y','n',get_zc_font(font_lfont))==1)
	{
		go_tiles();
		saved=false;
		
		if(format == tf4Bit)
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
				convert_tile(t, format, cs, shift, alt);

		tile=tile2=zc_min(tile,tile2);
	}
}


int32_t readtilefile(PACKFILE *f)
{
	dword section_version=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	
	if(!p_igetl(&zversion,f))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f))
	{
		return 0;
	}
	if(!read_deprecated_section_cversion(f))
	{
		return 0;
	}
	al_trace("readoneweapon section_version: %d\n", section_version);

	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .ztile packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_TILES ))
	{
		al_trace("Cannot read .ztile packfile made using V_TILES (%d)\n", section_version);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .ztile packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	int32_t index = 0;
	int32_t count = 0;
	
	//tile id
	if(!p_igetl(&index,f))
	{
		return 0;
	}
	al_trace("Reading tile: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f))
	{
		return 0;
	}
	al_trace("Reading tile: count(%d)\n", count);
	
	
	

	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		byte *temp_tile = new byte[tilesize(tf32Bit)];
		byte format=tf4Bit;
		memset(temp_tile, 0, tilesize(tf32Bit));
		if(!p_getc(&format,f))
		{
			delete[] temp_tile;
			return 0;
		}

				
		if(!pfread(temp_tile,tilesize(format),f))
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
	int32_t zversion = 0;
	int32_t zbuild = 0;
	
	if(!p_igetl(&zversion,f))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f))
	{
		return 0;
	}
	if(!read_deprecated_section_cversion(f))
	{
		return 0;
	}
	al_trace("readoneweapon section_version: %d\n", section_version);

	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .ztile packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_TILES ))
	{
		al_trace("Cannot read .ztile packfile made using V_TILES (%d)\n", section_version);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .ztile packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	int32_t index = 0;
	int32_t count = 0;
	
	//tile id
	if(!p_igetl(&index,f))
	{
		return 0;
	}
	al_trace("Reading tile: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f))
	{
		return 0;
	}
	al_trace("Reading tile: count(%d)\n", count);
	

	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		byte *temp_tile = new byte[tilesize(tf32Bit)];
		byte format=tf4Bit;
		memset(temp_tile, 0, tilesize(tf32Bit));
		if(!p_getc(&format,f))
		{
			delete[] temp_tile;
			return 0;
		}

				
		if(!pfread(temp_tile,tilesize(format),f))
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
	int32_t zversion = 0;
	int32_t zbuild = 0;
	
	if(!p_igetl(&zversion,f))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f))
	{
		return 0;
	}
	if(!read_deprecated_section_cversion(f))
	{
		return 0;
	}
	al_trace("readoneweapon section_version: %d\n", section_version);

	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .ztile packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_TILES ))
	{
		al_trace("Cannot read .ztile packfile made using V_TILES (%d)\n", section_version);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .ztile packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	int32_t index = 0;
	int32_t count = 0;
	
	//tile id
	if(!p_igetl(&index,f))
	{
		return 0;
	}
	al_trace("Reading tile: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f))
	{
		return 0;
	}
	al_trace("Reading tile: count(%d)\n", count);
	
	
	

	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		byte *temp_tile = new byte[tilesize(tf32Bit)];
		byte format=tf4Bit;
		memset(temp_tile, 0, tilesize(tf32Bit));
		
		if(!p_getc(&format,f))
		{
			delete[] temp_tile;
			return 0;
		}

				
		if(!pfread(temp_tile,tilesize(format),f))
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
	
	if(!write_deprecated_section_cversion(section_version,f))
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
		if(!p_putc(newtilebuf[index+(tilect)].format,f))
		{
			return 0;
		}
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
	popup_zqdialog_start();
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
	reftile = 0;
	int32_t tile_clicked=-1;
	bool rect_sel=true;
	bound(first,0,(TILES_PER_PAGE*TILE_PAGES)-1);
	position_mouse_z(0);
	
	go();
	
	register_used_tiles();
	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-25-6)>>1;
	int32_t screen_xofs=window_xofs+6;
	int32_t screen_yofs=window_yofs+25;
	int32_t panel_yofs=3;
	int32_t mul = 2;
	FONT *tfont = get_zc_font(font_lfont_l);
	
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
	int otl = tile, otl2 = tile2;
	do
	{
		HANDLE_CLOSE_ZQDLG();
		if(exiting_program) break;
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
					if(CHECK_CTRL_CMD ||
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
					if(!prompt_for_new_file_compat("Save ZTILE(.ztile)", "ztile", NULL,datapath,false))
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
					if(!prompt_for_existing_file_compat("Load ZTILE(.ztile)", "ztile", NULL,datapath,false))
						break;   
					PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
					if(!f) break;
					al_trace("Saving tile: %d\n", tile);
					if (!readtilefile(f))
					{
						al_trace("Could not read from .ztile packfile %s\n", temppath);
						jwin_alert("ZTILE File: Error","Could not load the specified Tile.",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
					}
					else
					{
						jwin_alert("ZTILE File: Success!","Loaded the source tiles to your tile sheets!",NULL,NULL,"O&K",NULL,'k',0,get_zc_font(font_lfont));
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
					if(CHECK_CTRL_CMD ||
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
						cs = (cs>0)  ? cs-1:13;
						
					redraw=true;
					break;
				}
				
				case KEY_UP:
				{
					switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((CHECK_CTRL_CMD)?1:0))
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

								// This used to do something. Too lazy to remove.
								// Can probably remove the above "same" check too.
								bitcheck = 2;
								
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
										if((CHECK_CTRL_CMD))
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
							sel_tile(tile,tile2,first,type,(CHECK_CTRL_CMD)?-1*(tile_page_row(tile)*TILES_PER_ROW):-TILES_PER_ROW);
							redraw=true;
							
						default: //Others
							break;
					}
				}
				break;
				
				case KEY_DOWN:
				{
					switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((CHECK_CTRL_CMD)?1:0))
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

								// This used to do something. Too lazy to remove.
								// Can probably remove the above "same" check too.
								bitcheck = 2;
								
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
										if((CHECK_CTRL_CMD))
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
							sel_tile(tile,tile2,first,type,(CHECK_CTRL_CMD)?((TILE_ROWS_PER_PAGE-1)-tile_page_row(tile))*TILES_PER_ROW:TILES_PER_ROW);
							redraw=true;
							
						default: //Others
							break;
					}
				}
				break;
				
				case KEY_LEFT:
				{
					switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((CHECK_CTRL_CMD)?1:0))
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

								// This used to do something. Too lazy to remove.
								// Can probably remove the above "same" check too.
								bitcheck = 2;
								
								for(int32_t r=0; r<rows; r++)
								{
									for(int32_t c=0; c<columns; c++)
									{
										int32_t temptile=((top+r)*TILES_PER_ROW)+left+c;
										byte *dest_pixelrow=(newtilebuf[temptile].data);
										
										for(int32_t pixelrow=0; pixelrow<16; pixelrow++)
										{
											for(int32_t p=0; p<(8*bitcheck)-1; p++)
											{
												*dest_pixelrow=*(dest_pixelrow+1);
												dest_pixelrow++;
											}
											
											if(c==columns-1)
											{
												if(!(CHECK_CTRL_CMD))
												{
													byte *tempsrc=(newundotilebuf[((top+r)*TILES_PER_ROW)+left].data+(pixelrow*8*bitcheck));
													*dest_pixelrow=*tempsrc;
												}
											}
											else
											
											{
												byte *tempsrc=(newtilebuf[temptile+1].data+(pixelrow*8*bitcheck));
												*dest_pixelrow=*tempsrc;
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
							sel_tile(tile,tile2,first,type,(CHECK_CTRL_CMD)?-(tile%TILES_PER_ROW):-1);
							redraw=true;
							
						default: //Others
							break;
					}
				}
				break;
			
				case KEY_RIGHT:
				{
					switch(((key[KEY_ALT] || key[KEY_ALTGR])?2:0)+((CHECK_CTRL_CMD)?1:0))
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

								// This used to do something. Too lazy to remove.
								// Can probably remove the above "same" check too.
								bitcheck = 2;
								
								for(int32_t r=0; r<rows; r++)
								{
									for(int32_t c=columns-1; c>=0; c--)
									{
										int32_t temptile=((top+r)*TILES_PER_ROW)+left+c;
										byte *dest_pixelrow=(newtilebuf[temptile].data)+(128*bitcheck)-1;
										
										for(int32_t pixelrow=15; pixelrow>=0; pixelrow--)
										{
											for(int32_t p=0; p<(8*bitcheck)-1; p++)
											{
												*dest_pixelrow=*(dest_pixelrow-1);
												dest_pixelrow--;
											}
											
											if(c==0)
											{
												if(!(CHECK_CTRL_CMD))
												{
													byte *tempsrc=(newundotilebuf[(((top+r)*TILES_PER_ROW)+left+columns-1)].data+(pixelrow*8*bitcheck)+(8*bitcheck)-1);
													*dest_pixelrow=*tempsrc;
												}
											}
											else
											{
												byte *tempsrc=(newtilebuf[temptile-1].data+(pixelrow*8*bitcheck)+(8*bitcheck)-1);
												*dest_pixelrow=*tempsrc;
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
							sel_tile(tile,tile2,first,type,(CHECK_CTRL_CMD)?(TILES_PER_ROW)-(tile%TILES_PER_ROW)-1:1);
							redraw=true;
							
						default: //Others
							break;
					}
				}
				break;
			
				case KEY_PGUP:
					sel_tile(tile,tile2,first,type,(CHECK_CTRL_CMD)?-1*(TILEROW(tile)*TILES_PER_ROW):-TILES_PER_PAGE);
					redraw=true;
					break;
				
				case KEY_PGDN:
					sel_tile(tile,tile2,first,type,(CHECK_CTRL_CMD)?((TILE_PAGES*TILE_ROWS_PER_PAGE)-TILEROW(tile)-1)*TILES_PER_ROW:TILES_PER_PAGE);
					redraw=true;
					break;
				
				case KEY_HOME:
					sel_tile(tile,tile2,first,type,(CHECK_CTRL_CMD)?-(tile):-(tile%TILES_PER_PAGE));
					redraw=true;
					break;
				
				case KEY_END:
					sel_tile(tile,tile2,first,type,(CHECK_CTRL_CMD)?(TILE_PAGES)*(TILES_PER_PAGE)-tile-1:(TILES_PER_PAGE)-(tile%TILES_PER_PAGE)-1);
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
							mass_overlay_tile(zc_min(tile,tile2),zc_max(tile,tile2),copy,cs,(CHECK_CTRL_CMD), rect_sel);
							saved=false;
						}
						else
						{
							saved = !overlay_tiles(tile,tile2,copy,copycnt,rect_sel,false,cs,(CHECK_CTRL_CMD));
							//overlay_tile(newtilebuf,tile,copy,cs,(CHECK_CTRL_CMD));
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
					
					if(CHECK_CTRL_CMD)
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
					if(type==0 && (key[KEY_LSHIFT]||key[KEY_RSHIFT]))
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
					delete_tiles(tile,tile2,rect_sel);
					redraw=true;
					break;
				
				case KEY_U:
				{
					if(CHECK_CTRL_CMD)
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
					create_relational_tiles_dlg[0].dp2=get_zc_font(font_lfont);
					create_relational_tiles_dlg[2].dp=buf;
					
					large_dialog(create_relational_tiles_dlg);
						
					int32_t ret=do_zqdialog(create_relational_tiles_dlg,2);
					
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
							jwin_alert("Error","The source tiles are not","in the same format.",NULL,"&OK",NULL,13,27,get_zc_font(font_lfont));
							break;
						}
						
						if(tile+(frames*(create_relational_tiles_dlg[3].flags&D_SELECTED?48:96))>NEWMAXTILES)
						{
							jwin_alert("Error","Too many tiles will be created",NULL,NULL,"&OK",NULL,13,27,get_zc_font(font_lfont));
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
					bool control=(CHECK_CTRL_CMD);
					bool alt=(key[KEY_ALT] || key[KEY_ALTGR]);
					int format = control ? tf4Bit : tf8Bit;
					
					do_convert_tile(tile, tile2, cs, rect_sel, format, shift, alt);
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
			if(isinRect(gui_mouse_x(),gui_mouse_y(),window_xofs + w + 12 - 21, window_yofs + 5, window_xofs + w +12 - 21 + 15, window_yofs + 5 + 13))
			{
				if(do_x_button(screen, w+12+window_xofs - 21, 5+window_yofs))
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
				
				if(do_text_button(150*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"&Grab"))
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
				
				if(do_text_button((150+28)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"&Edit"))
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
				
				if(do_text_button((150+28*2)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"Export"))
				{
					std::string initial_path = "tileset.png";
					if (strlen(datapath))
						initial_path = fmt::format("{}/{}", datapath, initial_path);
					if(prompt_for_new_file_compat("Export Tile Page (.png)","png",NULL,initial_path,true))
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
				
				if(do_text_button((150+28*3)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"Recolor"))
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
				
				font = tf;
			}
			else if(!bdown && isinRect(x,y,(150+28*4)*mul,213*mul+panel_yofs,(150+28*5)*mul,(213+21)*mul+panel_yofs))
			{
				FONT *tf = font;
				font = tfont;
				
				if(do_text_button((150+28*4)*mul+screen_xofs,213*mul+screen_yofs+panel_yofs,28*mul,21*mul,"Done"))
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

		if((f%8)==0 || InvalidBG == 1)
			redraw=true;
		if(otl != tile || otl2 != tile2)
		{
			otl = tile;
			otl2 = tile2;
			redraw = true;
		}
			
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
						safe_rect(screen2,x,y,x+(16*mul)-1,y+(16*mul)-1,vc(TilePgCursorCol),2);
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
						safe_rect(screen2,x,y,x+(16*mul)-1,y+(16*mul)-1,vc(TilePgCursorCol),2);
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
			gui_textout_ln(screen, get_zc_font(font_lfont_l), (uint8_t *)cbuf, (235*mul)+screen_xofs, (212*mul)+screen_yofs+panel_yofs, jwin_pal[jcBOXFG],jwin_pal[jcBOX],0);
		}
		
		++f;
		
		if(r_click)
		{
			select_tile_view_menu.select_uid(MENUID_SELTILE_VIEW_HIDE_USED, HIDE_USED);
			select_tile_view_menu.select_uid(MENUID_SELTILE_VIEW_HIDE_UNUSED, HIDE_UNUSED);
			select_tile_view_menu.select_uid(MENUID_SELTILE_VIEW_HIDE_BLANK, HIDE_BLANK);
			select_tile_view_menu.select_uid(MENUID_SELTILE_VIEW_HIDE_8BIT, HIDE_8BIT_MARKER);

			int current_tile_format = MENUID_SELTILE_COLOR_DEPTH_4_BIT;
			if (newtilebuf[tile].format == tf8Bit)
				current_tile_format = MENUID_SELTILE_COLOR_DEPTH_8_BIT;
			select_tile_color_depth_menu.select_only_uid(current_tile_format);
			select_tile_color_depth_cb = [&](int format){
				if (newtilebuf[tile].format == format)
					return;

				bool skip_prompt = true;
				do_convert_tile(tile, tile2, cs, rect_sel, format, false, false, skip_prompt);
			};

			NewMenu rcmenu
			{
				{ "Copy", [&]()
					{
						copy = zc_min(tile,tile2);
						copycnt = abs(tile-tile2)+1;
					} },
				{ "Paste", [&]()
					{
						bool b = copy_tiles(tile, tile2, copy, copycnt, rect_sel, false);
						if(saved) saved = !b;
					}, nullopt, copy < 0 },
				{ "Move", [&]()
					{
						bool b = copy_tiles(tile, tile2, copy, copycnt, rect_sel, true);
						if(saved) saved = !b;
					}, nullopt, copy < 0 },
				{ "Clear", [&]()
					{
						delete_tiles(tile, tile2, rect_sel);
					} },
				{ "Set as Reference", [&]()
					{
						reftile = tile;
					} },
				{},
				{ "Edit", [&]()
					{
						edit_tile(tile, flip, cs);
						draw_tile_list_window();
					} },
				{ "Grab", [&]()
					{
						grab_tile(tile, cs);
						draw_tile_list_window();
						position_mouse_z(0);
					} },
				{ "Scale", [&]()
					{
						bool b = scale_or_rotate_tiles(tile, tile2, cs, false);
						if(saved) saved = !b;
					}, nullopt, type != 0 },
				{ "Angular Rotation", [&]()
					{
						bool b = scale_or_rotate_tiles(tile, tile2, cs, true);
						if(saved) saved = !b;
					}, nullopt, type != 0 },
				{ "Color Depth", &select_tile_color_depth_menu },
				{},
				{ "Blank?", [&]()
					{
						show_blank_tile(tile);
					} },
				{},
				{ "View ", &select_tile_view_menu },
				{ "Overlay", [&]()
					{
						overlay_tile(newtilebuf, tile, copy, cs, 0);
					} },
				{ "H-Flip", [&]()
					{
						flip ^= 1;
						go_tiles();
						
						if(type == 0)
						{
							normalize(tile, tile2, rect_sel, flip);
							flip = 0;
						}
					} },
				{ "V-Flip", [&]()
					{
						flip ^= 2;
						go_tiles();
						
						if(type == 0)
						{
							normalize(tile, tile2, rect_sel, flip);
							flip = 0;
						}
					} },
				{ "Create Combos", [&]()
					{
						if(rect_sel)
							make_combos_rect(top, left, rows, columns, cs);
						else
							make_combos(zc_min(tile, tile2), zc_max(tile, tile2), cs);
					}, nullopt, type != 0 },
				{ "Insert", [&]()
					{
						bool warn = (rect_sel
							&& ((tile/20)!=(tile2/20))
							&& !(tile%20==0&&tile2%20==19));
						int32_t z = zc_min(tile, tile2);
						int32_t count = abs(tile-tile2) + 1;
						tile = z;
						tile2 = NEWMAXTILES;
						copy = tile + count;
						copycnt = NEWMAXTILES-copy;
						
						string msg;
						
						if(count>1)
							msg = fmt::format("Insert {} blank tiles?",count);
						else
							msg = "Insert a blank tile?";
							
						AlertDialog("Insert Tiles", msg
							+"\nThis will offset the tiles that follow!"
							+(warn?"\nInserting tiles ignores rectangular selections!":""),
							[&](bool ret,bool)
							{
								if(ret)
								{
									go_tiles();
									if(copy_tiles(copy, tile2, tile, copycnt, false, true))
										saved = false;
								}
							}).show();
						
						copy=-1;
						tile2=tile=z;
					}, nullopt, type != 0 },
				{ "Remove", [&]()
					{
						bool warn = (rect_sel
							&& ((tile/20)!=(tile2/20))
							&& !(tile%20==0&&tile2%20==19));
						int32_t z = zc_min(tile, tile2);
						int32_t count = abs(tile-tile2) + 1;
						tile = z;
						tile2 = NEWMAXTILES;
						copy = tile + count;
						copycnt = NEWMAXTILES-copy;
						
						string msg;
						
						if(count>1)
							msg = fmt::format("Remove tiles {} - {}?", tile, copy-1);
						else
							msg = fmt::format("Remove tile {}?", tile);
						
						AlertDialog("Remove Tiles", msg
							+"\nThis will offset the tiles that follow!"
							+(warn?"\nRemoving tiles ignores rectangular selections!":""),
							[&](bool ret,bool)
							{
								if(ret)
								{
									go_tiles();
									if(copy_tiles(tile, tile2, copy, copycnt, false, true))
										saved = false;
								}
							}).show();
						
						copy=-1;
						tile2=tile=z;
					}, nullopt, type != 0 },
			};
			rcmenu.pop(window_mouse_x(),window_mouse_y());
			redraw = true;
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
	return onGotoTiles(-1);
}

int32_t onGotoTiles(int32_t startfrom)
{
	static int32_t t = 0;
	if (startfrom > -1)
		t = startfrom;
	int32_t f = 0;
	int32_t c = CSet;
	reset_pal_cycling();
	//  loadlvlpal(Map.CurrScr()->color);
	rebuild_trans_table();
	select_tile(t, f, 0, c, true);
	refresh(rALL);
	return D_O_K;
}

int32_t combopage_animate = 1;
void draw_combo(BITMAP *dest, int x,int y,int c,int cs,bool animate)
{
	if(unsigned(c)<MAXCOMBOS)
	{
		newcombo& cmb = combobuf[c];
		int t = cmb.tile;
		if(!animate)
			cmb.tile = cmb.o_tile;
		put_combo(dest,x,y,c,cs,0,0);
		cmb.tile = t;
	}
	else
	{
		rectfill(dest,x,y,x+32-1,y+32-1,0);
	}
}

void draw_combos(int32_t page,int32_t cs,bool cols)
{
	clear_bitmap(screen2);
	BITMAP *buf = create_bitmap_ex(8,16,16);
	
	int32_t w = 32;
	int32_t h = 32;
	int32_t mul = 2;
	
	int32_t window_xofs=(zq_screen_w-640-12)>>1;
	int32_t window_yofs=(zq_screen_h-480-25-6)>>1;
	int32_t screen_xofs=window_xofs+6;
	int32_t screen_yofs=window_yofs+25;
	
	if(cols==false)
	{
		for(int32_t i=0; i<256; i++)                                // 13 rows, leaving 32 pixels from y=208 to y=239
		{
			int32_t x = (i%COMBOS_PER_ROW)*w;
			int32_t y = (i/COMBOS_PER_ROW)*h;
			
			combotile_override_x = x+screen_xofs+(w-16)/2;
			combotile_override_y = y+screen_yofs+(h-16)/2;
			draw_combo(buf,0,0,i+(page<<8),cs,combopage_animate);
			stretch_blit(buf,screen2,0,0,16,16,x,y,w,h);
		}
	}
	else
	{
		int32_t c = 0;
		
		for(int32_t i=0; i<256; i++)
		{
			int32_t x = (i%COMBOS_PER_ROW)*w;
			int32_t y = (i/COMBOS_PER_ROW)*h;
			
			combotile_override_x = x+screen_xofs+(w-16)/2;
			combotile_override_y = y+screen_yofs+(h-16)/2;
			draw_combo(buf,0,0,c+(page<<8),cs,combopage_animate);
			stretch_blit(buf,screen2,0,0,16,16,x,y,w,h);
			++c;
			
			if((i&3)==3)
				c+=48;
				
			if((i%COMBOS_PER_ROW)==(COMBOS_PER_ROW-1))
				c-=256;
		}
	}
	combotile_override_x = combotile_override_y = -1;
	
	for(int32_t x=(64*mul); x<(320*mul); x+=(64*mul))
	{
		vline(screen2,x,0,(208*mul)-1,vc(15));
	}
	
	destroy_bitmap(buf);
}

void combo_info(int32_t tile,int32_t tile2,int32_t cs,int32_t copy,int32_t copycnt,int32_t page,int32_t buttons)
{
	int32_t yofs=3;
	static BITMAP *buf = create_bitmap_ex(8,16,16);
	int32_t mul = 2;
	FONT *tfont = get_zc_font(font_lfont_l);
	
	rectfill(screen2,0,210*2,(320*2)-1,(240*2)-1,jwin_pal[jcBOX]);
	hline(screen2, 0, (210*2)-2, (320*2)-1, jwin_pal[jcMEDLT]);
	hline(screen2, 0, (210*2)-1, (320*2)-1, jwin_pal[jcLIGHT]);
	
	jwin_draw_frame(screen2,(31*mul)-2,((216*mul)+yofs)-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	
	if(copy>=0)
	{
		put_combo(buf,0,0,copy,cs,0,0);
		stretch_blit(buf,screen2,0,0,16,16,31*mul,216*mul+yofs,16*mul,16*mul);
		
		if(copycnt>1)
		{
			textprintf_right_ex(screen2,tfont,28*mul,(216*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d-",copy);
			textprintf_right_ex(screen2,tfont,24*mul,(224*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",copy+copycnt-1);
		}
		else
		{
			textprintf_right_ex(screen2,tfont,24*mul,(220*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",copy);
		}
	}
	else
	{
		if (InvalidBG == 2)
		{
			draw_checkerboard(screen2, 31 * mul, 216 * mul + yofs, 16 * mul);
		}
		else if(InvalidBG == 1)
		{
			for(int32_t dy=0; dy<16*mul; dy++)
			{
				for(int32_t dx=0; dx<16*mul; dx++)
				{
					screen2->line[(216*mul)+yofs+dy][(31*mul)+dx]=vc((((zc_oldrand()%100)/50)?0:8)+(((zc_oldrand()%100)/50)?0:7));
				}
			}
		}
		else
		{
			rectfill(screen2, (31*mul), (216*mul)+yofs, (31*mul)+31, (216*mul)+yofs+31, vc(0));
			rect(screen2, (31*mul), (216*mul)+yofs, (31*mul)+31, (216*mul)+yofs+31, vc(15));
			line(screen2, (31*mul), (216*mul)+yofs, (31*mul)+31, (216*mul)+yofs+31, vc(15));
			line(screen2, (31*mul), (216*mul)+yofs+31, (31*mul)+31, (216*mul)+yofs, vc(15));
		}
	}
	
	jwin_draw_frame(screen2,(53*mul)-2,(216*mul)+yofs-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
	put_combo(buf,0,0,tile,cs,0,0);
	stretch_blit(buf,screen2,0,0,16,16,53*mul,216*mul+yofs,16*mul,16*mul);
	
	if(tile>tile2)
	{
		zc_swap(tile,tile2);
	}
	
	char cbuf[8];
	cbuf[0]=0;
	
	if(tile2!=tile)
	{
		sprintf(cbuf,"-%d",tile2);
	}
	
	textprintf_ex(screen2,tfont,(73*mul),(216*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"combo: CSet %d", cs);
	textprintf_ex(screen2,tfont,(73*mul),(224*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d%s",tile,cbuf);
	
	if(tile2==tile)
	{
		int32_t nextcombo=combobuf[tile].nextcombo;
		int32_t nextcset=(combobuf[tile].animflags & AF_CYCLENOCSET) ? cs : combobuf[tile].nextcset;
		jwin_draw_frame(screen2,(136*mul)-2,(216*mul)+yofs-2,(16*mul)+4,(16*mul)+4,FR_DEEP);
		
		if(nextcombo>0 && !(combobuf[tile].animflags & AF_CYCLEUNDERCOMBO))
		{
			put_combo(buf,0,0,nextcombo,nextcset,0,0);
			stretch_blit(buf,screen2,0,0,16,16,136*mul,216*mul+yofs,16*mul,16*mul);
		}
		else
		{
			if (InvalidBG == 2)
			{
				draw_checkerboard(screen2, 136 * mul, 216 * mul + yofs, 16 * mul);
			}
			else if(InvalidBG == 1)
			{
				for(int32_t dy=0; dy<16*mul; dy++)
				{
					for(int32_t dx=0; dx<16*mul; dx++)
					{
						screen2->line[(216*mul)+yofs+dy][(136*mul)+dx]=vc((((zc_oldrand()%100)/50)?0:8)+(((zc_oldrand()%100)/50)?0:7));
					}
				}
			}
			else
			{
				rectfill(screen2, (136*mul), (216*mul)+yofs, (136*mul)+31, (216*mul)+yofs+31, vc(0));
				rect(screen2, (136*mul), (216*mul)+yofs, (136*mul)+31, (216*mul)+yofs+31, vc(15));
				line(screen2, (136*mul), (216*mul)+yofs, (136*mul)+31, (216*mul)+yofs+31, vc(15));
				line(screen2, (136*mul), (216*mul)+yofs+31, (136*mul)+31, (216*mul)+yofs, vc(15));
			}
		}
		
		textprintf_right_ex(screen2,tfont,(132*mul),(216*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Cycle:");
		textprintf_right_ex(screen2,tfont,(132*mul),(224*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",nextcombo);
	}
	
	
	FONT *tf = font;
	font = tfont;
	
	draw_checkbox(screen2,320,440+yofs,16,combopage_animate);
	textprintf_ex(screen2,tfont,320+18,440+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"Animate");
	
	if(buttons&2)
	{
		draw_text_button(screen2,404,426+yofs,88,42,"Edit",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
	}
	
	if(buttons&4)
	{
		draw_text_button(screen2,494,426+yofs,88,42,"Done",jwin_pal[jcBOXFG],jwin_pal[jcBOX],0,true);
	}
	
	font = tf;
	
	jwin_draw_icon(screen2,(305*mul+4),220*mul-6+yofs,jwin_pal[jcBOXFG],BTNICON_ARROW_UP,6,true);
	textprintf_ex(screen2,tfont,(293*mul),(220*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"p:");
	textprintf_centre_ex(screen2,tfont,(309*mul),(220*mul)+yofs,jwin_pal[jcBOXFG],jwin_pal[jcBOX],"%d",page);
	jwin_draw_icon(screen2,(305*mul+4),228*mul+3+yofs,jwin_pal[jcBOXFG],BTNICON_ARROW_DOWN,6,true);

	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-25-6)>>1;
	int32_t screen_xofs=window_xofs+6;
	int32_t screen_yofs=window_yofs+25;
	
	custom_vsync();
	blit(screen2,screen,0,0,screen_xofs,screen_yofs,w,h);
	SCRFIX();
	//destroy_bitmap(buf);
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
	int32_t window_xofs=0;
	int32_t window_yofs=0;
	int32_t w = 640;
	int32_t h = 480;
	
	window_xofs=(zq_screen_w-w-12)>>1;
	window_yofs=(zq_screen_h-h-25-6)>>1;
	jwin_draw_win(screen, window_xofs, window_yofs, w+6+6, h+25+6, FR_WIN);
	jwin_draw_frame(screen, window_xofs+4, window_yofs+23, w+2+2, h+4+2-64,  FR_DEEP);
	FONT *oldfont = font;
	font = get_zc_font(font_lfont);
	jwin_draw_titlebar(screen, window_xofs+3, window_yofs+3, w+6, 18, "Select Combo", true);
	font=oldfont;
}


static int32_t _selected_combo=-1, _selected_cset=-1;
bool select_combo_2(int32_t &cmb,int32_t &cs)
{
	popup_zqdialog_start();
	reset_combo_animations();
	reset_combo_animations2();
	combopage_animate = zc_get_config("ZQ_GUI","combopage_animate",1);
	// static int32_t cmb=0;
	int32_t page=cmb>>8;
	int32_t tile2=cmb;
	int32_t done=0;
	int32_t tile_clicked=-1;
	int32_t t2;
	int32_t copy=-1;
	int32_t copycnt=0;
	
	position_mouse_z(0);
		
	go();
	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-25-6)>>1;
	int32_t screen_xofs=window_xofs+6;
	int32_t screen_yofs=window_yofs+25;
	int32_t panel_yofs=3;
	int32_t mul = 2;
	FONT *tfont = get_zc_font(font_lfont_l);
	
	draw_combo_list_window();
	draw_combos(page,cs,true);
	combo_info(cmb,tile2,cs,copy,copycnt,page,4);
	
	while(gui_mouse_b())
	{
		/* do nothing */
		rest(1);
	}
	
	bool bdown=false;
	int32_t f=0;
	int otl = cmb, otl2 = tile2;
	
	do
	{
		HANDLE_CLOSE_ZQDLG();
		if(exiting_program) break;
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
			
			case KEY_EQUALS:
			case KEY_PLUS_PAD:
				cs = (cs<13) ? cs+1:0;
				redraw=true;
				break;
				
			case KEY_MINUS:
			case KEY_MINUS_PAD:
				cs = (cs>0)  ? cs-1:13;
				redraw=true;
				break;
				
			case KEY_UP:
				sel_combo(cmb,tile2,-COMBOS_PER_ROW,true);
				redraw=true;
				break;
				
			case KEY_DOWN:
				sel_combo(cmb,tile2,COMBOS_PER_ROW,true);
				redraw=true;
				break;
				
			case KEY_LEFT:
				sel_combo(cmb,tile2,-1,true);
				redraw=true;
				break;
				
			case KEY_RIGHT:
				sel_combo(cmb,tile2,1,true);
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
			if(isinRect(gui_mouse_x(),gui_mouse_y(),window_xofs + w + 12 - 21, window_yofs + 5, window_xofs + w +12 - 21 + 15, window_yofs + 5 + 13))
			{
				if(do_x_button(screen, w+12+window_xofs - 21, 5+window_yofs))
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
				
				t = ((x>>7)*52) + ((x>>5)&3) + ((y>>5)<<2);
				
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
					
					t2 = ((x>>7)*52) + ((x>>5)&3) + ((y>>5)<<2);
					
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
				
				if(do_text_button((247*mul)+screen_xofs,(213*mul)+screen_yofs+panel_yofs,(44*mul),(21*mul),"Done"))
				{
					done=2;
				}
				
				font = tf;
			}
			else if(!bdown && isinRect(x,y,320,440+panel_yofs,320+16,440+panel_yofs+16))
			{
				FONT *tf = font;
				font = tfont;
				
				//do_scheckbox(screen2,320,440+panel_yofs,16,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],combopage_animate,screen_xofs,screen_yofs);
				combopage_animate = combopage_animate ? 0 : 1;
				zc_set_config("ZQ_GUI","combopage_animate",combopage_animate);
				redraw = true;
				
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
		
		if((f%8) || InvalidBG == 1)
			redraw = true;
		if(otl != cmb || otl2 != tile2)
		{
			otl = cmb;
			otl2 = tile2;
			redraw = true;
		}
		
		if(redraw || combopage_animate)
			draw_combos(page,cs,true);
			
		combo_info(cmb,tile2,cs,copy,copycnt,page,4);
		
		if(f&8)
		{
			int32_t x,y;
			
			for(int32_t i=zc_min(cmb,tile2); i<=zc_max(cmb,tile2); i++)
			{
				if((i>>8)==page)
				{
					int32_t t=i&255;
					
					x=((t&3) + ((t/52)<<2)) << 5;
					y=((t%52)>>2) << 5;
					
					safe_rect(screen,x+screen_xofs,y+screen_yofs,x+screen_xofs+(16*mul)-1,y+screen_yofs+(16*mul)-1,vc(CmbPgCursorCol),2);
				}
			}
			
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

bool advpaste(int32_t tile, int32_t tile2, int32_t copy)
{
	static bitstring pasteflags;
	static const vector<CheckListInfo> advp_names =
	{
		{ "Tile" },
		{ "CSet2" },
		{ "Solidity" },
		{ "Animation" },
		{ "Type" },
		{ "Inherent Flag" },
		{ "Attribytes" },
		{ "Attrishorts" },
		{ "Attributes" },
		{ "Flags", "The 16 Flags on the 'Flags' tab" },
		{ "Gen. Flags", "The 2 'General Flags' on the 'Flags' tab" },
		{ "Label" },
		{ "Script" },
		{ "Effect" },
		{ "Triggers Tab" },
		{ "Lifting Tab" },
		{ "Gen: Movespeed", "The Movespeed related values from the 'General' tab" },
		{ "Gen: SFX", "The SFX related values from the 'General' tab" },
		{ "Gen: Sprites", "The Sprites related values from the 'General' tab" },
	};
	if(!call_checklist_dialog("Advanced Paste",advp_names,pasteflags))
		return false;
	
	//Paste to each combo in the range
	for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); ++i)
	{
		combobuf[i].advpaste(combobuf[copy], pasteflags);
	}
	
	if(pasteflags.get(CMB_ADVP_TILE)) //reset animations if needed
	{
		setup_combo_animations();
		setup_combo_animations2();
	}
	
	return true;
}

int32_t combo_screen(int32_t pg, int32_t tl)
{
	popup_zqdialog_start();
	reset_combo_animations();
	reset_combo_animations2();
	combopage_animate = zc_get_config("ZQ_GUI","combopage_animate",1);
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
	int32_t w = 640;
	int32_t h = 480;
	int32_t window_xofs=(zq_screen_w-w-12)>>1;
	int32_t window_yofs=(zq_screen_h-h-25-6)>>1;
	int32_t screen_xofs=window_xofs+6;
	int32_t screen_yofs=window_yofs+25;
	int32_t panel_yofs=3;
	int32_t mul = 2;
	FONT *tfont = get_zc_font(font_lfont_l);
	
	draw_combo_list_window();
	draw_combos(page,cs,true);
	combo_info(tile,tile2,cs,copy,copycnt,page,6);
	go_combos();
	position_mouse_z(0);
	
	while(gui_mouse_b())
	{
		/* do nothing */
	}
	
	bool bdown=false;
	int32_t f=0;
	int otl = tile, otl2 = tile2;
	
	do
	{
		HANDLE_CLOSE_ZQDLG();
		if(exiting_program) break;
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
			
			case KEY_EQUALS:
			case KEY_PLUS_PAD:
				if(CHECK_CTRL_CMD)
				{
					int32_t amnt = (key[KEY_LSHIFT] || key[KEY_RSHIFT]) ?
							   ((key[KEY_ALT] || key[KEY_ALTGR]) ? TILES_PER_PAGE*10 : TILES_PER_ROW)
							   : ((key[KEY_ALT] || key[KEY_ALTGR]) ? TILES_PER_PAGE : 1);
					
					for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); ++i)
					{
						combobuf[i].set_tile(wrap(combobuf[i].o_tile + amnt,
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
				if(CHECK_CTRL_CMD)
				{
					int32_t amnt = (key[KEY_LSHIFT] || key[KEY_RSHIFT]) ?
							   ((key[KEY_ALT] || key[KEY_ALTGR]) ? TILES_PER_PAGE*10 : TILES_PER_ROW)
							   : ((key[KEY_ALT] || key[KEY_ALTGR]) ? TILES_PER_PAGE : 1);
					
					for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); ++i)
					{
						combobuf[i].set_tile(wrap(combobuf[i].o_tile - amnt,
												0, NEWMAXTILES-1));
					}
					
					setup_combo_animations();
					redraw=true;
				}
				else
				{
					cs = (cs>0)  ? cs-1:13;
					redraw=true;
				}
				
				break;
				
			case KEY_UP:
				sel_combo(tile,tile2,-COMBOS_PER_ROW,true);
				redraw=true;
				break;
				
			case KEY_DOWN:
				sel_combo(tile,tile2,COMBOS_PER_ROW,true);
				redraw=true;
				break;
				
			case KEY_LEFT:
				sel_combo(tile,tile2,-1,true);
				redraw=true;
				break;
				
			case KEY_RIGHT:
				sel_combo(tile,tile2,1,true);
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
				if((CHECK_CTRL_CMD) && copy != -1)
				{
					if(advpaste(tile, tile2, copy))
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
						
					if(jwin_alert("Confirm Remove",buf,"This will offset all of the combos that follow!",NULL,"&Yes","&No",'y','n',get_zc_font(font_lfont))==1)
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
						
					if(jwin_alert("Confirm Insert",buf,"This will offset all of the combos that follow!",NULL,"&Yes","&No",'y','n',get_zc_font(font_lfont))==1)
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
				
				if(jwin_alert("Confirm Delete",buf,NULL,NULL,"&Yes","&No",'y','n',get_zc_font(font_lfont))==1)
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
			if(isinRect(gui_mouse_x(),gui_mouse_y(),window_xofs + w + 12 - 21, window_yofs + 5, window_xofs + w +12 - 21 + 15, window_yofs + 5 + 13))
			{
				if(do_x_button(screen, w+12+window_xofs - 21, 5+window_yofs))
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
				
				t = ((x>>7)*52) + ((x>>5)&3) + ((y>>5)<<2);
				
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
					
					t2 = ((x>>6)*52) + ((x>>4)&3) + ((y>>4)<<2);
					
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
				
				if(do_text_button((202*mul)+screen_xofs,(213*mul)+screen_yofs+panel_yofs,(44*mul),(21*mul),"Edit"))
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
				
				if(do_text_button((247*mul)+screen_xofs,(213*mul)+screen_yofs+panel_yofs,(44*mul),(21*mul),"Done"))
				{
					done=1;
				}
				
				font = tf;
			}
			else if(!bdown && isinRect(x,y,320,440+panel_yofs,320+16,440+panel_yofs+16))
			{
				FONT *tf = font;
				font = tfont;
				
				//do_scheckbox(screen2,320,440+panel_yofs,16,jwin_pal[jcTEXTBG],jwin_pal[jcTEXTFG],combopage_animate,screen_xofs,screen_yofs);
				combopage_animate = combopage_animate ? 0 : 1;
				zc_set_config("ZQ_GUI","combopage_animate",combopage_animate);
				redraw = true;
				
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
				
				t = ((x>>7)*52) + ((x>>5)&3) + ((y>>5)<<2);
				
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
			bdown=false;
		
		if((f%8) || InvalidBG == 1)
			redraw = true;
		if(otl != tile || otl2 != tile2)
		{
			otl = tile;
			otl2 = tile2;
			redraw = true;
		}
		
		if(redraw || combopage_animate)
			draw_combos(page,cs,true);
		
		combo_info(tile,tile2,cs,copy,copycnt,page,6);
		
		if(f&8)
		{
			int32_t x,y;
			
			for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
			{
				if((i>>8)==page)
				{
					int32_t t=i&255;
					
					x=((t&3) + ((t/52)<<2)) << 5;
					y=((t%52)>>2) << 5;
					
					safe_rect(screen,x+screen_xofs,y+screen_yofs,x+screen_xofs+(16*mul)-1,y+screen_yofs+(16*mul)-1,vc(CmbPgCursorCol),2);
				}
			}
			
			SCRFIX();
		}
		
		++f;
		
		//Seriously? There is duplicate code for the r-click menu? -Gleeok
		if(r_click)
		{
			NewMenu rcmenu
			{
				{ "Copy", [&]()
					{
						go_combos();
						copy=zc_min(tile,tile2);
						copycnt=abs(tile-tile2)+1;
					} },
				{ "Paste", [&]()
					{
						if((CHECK_CTRL_CMD) && copy != -1)
						{
							if(advpaste(tile, tile2, copy))
							{
								saved=false;
								redraw=true;
								copy=-1;
							}
							return;
						}
						
						masscopy=(key[KEY_LSHIFT] || key[KEY_RSHIFT])?1:0;
						
						if(copy>-1)
						{
							go_combos();
							copy_combos(tile,tile2,copy,copycnt,masscopy);
							setup_combo_animations();
							setup_combo_animations2();
							saved=false;
						}
					} },
				{ "Adv. Paste", [&]()
					{
						if(copy > -1)
						{
							if(advpaste(tile, tile2, copy))
							{
								saved=false;
								redraw=true;
								copy=-1;
							}
						}
					} },
				{ "Swap", [&]()
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
						copy=-1;
					} },
				{ "Delete", [&]()
					{
						string msg;
						
						if(tile==tile2)
							msg = fmt::format("Delete combo {}?",tile);
						else
							msg = fmt::format("Delete combos {}-{}?",zc_min(tile,tile2),zc_max(tile,tile2));
						bool didconfirm = false;
						AlertDialog("Confirm Delete",msg,
							[&](bool ret,bool)
							{
								if(ret)
									didconfirm = true;
							}).show();
						if(didconfirm)
						{
							go_combos();
							
							for(int32_t i=zc_min(tile,tile2); i<=zc_max(tile,tile2); i++)
								clear_combo(i);
								
							tile=tile2=zc_min(tile,tile2);
							saved=false;
						}
					} },
				{},
				{ "Edit", [&]()
					{
						go_combos();
						edit_combo(tile,false,cs);
					} },
				{ "Insert", [&]()
					{
						int z = tile;
						int count = abs(tile-tile2)+1;
						tile = zc_min(tile,tile2);
						tile2 = MAXCOMBOS;
						copy = tile+count;
						copycnt = MAXCOMBOS-tile-count;
						
						string msg;
						
						if(count>1)
							msg = fmt::format("Insert combos {} - {}?"
								" This will offset all of the combos that follow!",tile, copy-1);
						else
							msg = fmt::format("Insert combo {}?"
								" This will offset all of the combos that follow!",tile);
						
						bool didconfirm = false;
						AlertDialog("Confirm Insert",msg,
							[&](bool ret,bool)
							{
								if(ret)
									didconfirm = true;
							}).show();
						if(didconfirm)
							move_combos(copy, tile2, tile, copycnt);
						else return;
						
						copy = -1;
						tile2 = tile = z;
						
						//don't allow the user to undo; quest combo references are incorrect -DD
						go_combos();
						saved = false;
					} },
				{ "Remove", [&]()
					{
						int z = tile;
						int count = abs(tile-tile2)+1;
						tile = zc_min(tile,tile2);
						tile2 = MAXCOMBOS;
						copy = tile+count;
						copycnt = MAXCOMBOS-tile-count;
						
						string msg;
						
						if(count>1)
							msg = fmt::format("Remove combos {} - {}?"
								" This will offset all of the combos that follow!",tile, copy-1);
						else
							msg = fmt::format("Remove combo {}?"
								" This will offset all of the combos that follow!",tile);
						
						bool didconfirm = false;
						AlertDialog("Confirm Remove",msg,
							[&](bool ret,bool)
							{
								if(ret)
									didconfirm = true;
							}).show();
						if(didconfirm)
							move_combos(tile, tile2, copy, copycnt);
						else return;
						
						copy = -1;
						tile2 = tile = z;
						
						//don't allow the user to undo; quest combo references are incorrect -DD
						go_combos();
						saved = false;
					} },
				{},
				{ "Locations", [&]()
					{
						int32_t z = Combo;
						Combo = tile;
						onComboLocationReport();
						Combo = z;
					} },
			};
			rcmenu.pop(window_mouse_x(),window_mouse_y());
			redraw = true;
			r_click = false;
			goto REDRAW;
		}
		
	}
	while(!done);
	
	while(gui_mouse_b())
		rest(1);
	comeback();
	setup_combo_animations();
	setup_combo_animations2();
	_selected_combo = tile;
	_selected_cset = cs;
	popup_zqdialog_end();
	return done-1;
}

int32_t onCombos()
{
	combo_screen(-1,-1);
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
		FONT *f = get_zc_font(font_lfont_l);
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
						   "&OK",NULL,'o',0,get_zc_font(font_lfont));
						   
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
	icon_dlg[0].dp2=get_zc_font(font_lfont);
	
	for(int32_t i=0; i<4; i++)
	{
		icon_dlg[i+2].d1 = QMisc.icons[i];
		icon_dlg[i+2].fg = i+6;
		load_cset(pal, i+6, pSprite(i+spICON1));
	}
	
	zc_set_palette(pal);
	
	large_dialog(icon_dlg);
		
	int32_t ret = do_zqdialog(icon_dlg,7);
	
	if(ret==6)
	{
		for(int32_t i=0; i<4; i++)
		{
			if(QMisc.icons[i] != icon_dlg[i+2].d1)
			{
				QMisc.icons[i] = icon_dlg[i+2].d1;
				saved=false;
			}
		}
	}
	
	zc_set_palette(RAMpal);
	return D_O_K;
}

// Identical to jwin_frame_proc, but is treated differently by large_dialog()
int32_t d_comboframe_proc(int32_t msg, DIALOG *d, int32_t)
{
	if(msg == MSG_DRAW)
	{
		jwin_draw_frame(screen, d->x, d->y, d->w, d->h, d->d1);
	}
	
	return D_O_K;
}

int32_t d_combo_proc(int32_t msg,DIALOG *d,int32_t)
{
	switch(msg)
	{
		case MSG_CLICK:
		{
			if((d->flags&D_NOCLICK))
				break;
			
			int32_t ret = (d->flags & D_EXIT) ? D_CLOSE : D_O_K;
			int32_t combo2;
			int32_t cs;
			
			if(CHECK_ALT) //place selected cmb/cs
			{
				if(gui_mouse_b()&1)
				{
					if(!CHECK_SHIFT)
						d->d1 = Combo;
					d->fg = CSet;
				}
				
				return ret|D_REDRAW;
			}
			else if(gui_mouse_b()&2||nextcombo_fake_click==2)  //clear to 0/0
			{
				d->d1=0;
				d->fg=0;
				return ret|D_REDRAW;
			}
			else if(gui_mouse_b()&1||nextcombo_fake_click==1)  //popup combo picker
			{
				combo2=d->d1;
				cs=d->fg;
				
				if((CHECK_CTRL_CMD ? select_combo_3 : select_combo_2)(combo2, cs))
				{
					d->d1=combo2;
					d->fg=cs;
				}
				
				return ret|D_REDRAW;
			}
			else return ret|D_REDRAWME;
		}
		break;
		
		case MSG_DRAW:
		{
			d->w = 32;
			d->h = 32;
			
			BITMAP *buf = create_bitmap_ex(8,16,16);
			BITMAP *bigbmp = create_bitmap_ex(8,d->w,d->h);
			
			if(buf && bigbmp)
			{
				clear_bitmap(buf);
				
				if(d->d1==-1) // Display curr_combo instead of combobuf
				{
					newcombo hold = combobuf[0];
					combobuf[0] = curr_combo;
					putcombo(buf,0,0,0,d->fg);
					combobuf[0] = hold;
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
		break;
	}
	return D_O_K;
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
	int colorWidth=(d->w-4)/16;
	d->h = 4+(colorWidth*14);
	int colorHeight=colorWidth;
	
	switch(msg)
	{
	case MSG_DRAW:
		{
			jwin_draw_frame(bmp, d->x, d->y, d->w, d->h, FR_DEEP);
			for(int cset=0; cset<=13; cset++)
			{
				for(int color=0; color<16; color++)
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
			int cset=(gui_mouse_y()-(d->y+2))/colorHeight;
			int color=(gui_mouse_x()-(d->x+2))/colorWidth;
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
	{ jwin_check_proc,       156, 112, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "6", NULL, NULL },
	{ jwin_check_proc,        12, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "7", NULL, NULL },
	{ jwin_check_proc,        36, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "8", NULL, NULL },
	{ jwin_check_proc,        60, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "9", NULL, NULL },
	{ jwin_check_proc,        84, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "10", NULL, NULL },
	{ jwin_check_proc,       108, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "11", NULL, NULL },
	{ jwin_check_proc,       132, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "12", NULL, NULL },
	{ jwin_check_proc,       156, 128, 168,   8, vc(15), vc(1),    0,                 0,   1,   0, (void *) "13", NULL, NULL },
	
	// 20
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
#define MR4_IGNORE_BLANK 20
#define MR4_RESET 21
#define MR4_SWITCH 22
#define MR4_OK 23
#define MR4_CANCEL 24

static DIALOG recolor_8bit_dlg[] =
{
	// (dialog proc)         (x)  (y)  (w)  (h)   (fg)    (bg) (key)     (flags) (d1) (d2) (dp)
	{ jwin_win_proc,           0,   0, 288, 224,     0,      0,    0,     D_EXIT,   0,  0, (void *) "Recolor setup", NULL, NULL },
	
	// 1
	{ jwin_text_proc,         12,  32, 176,   8, vc(15), vc(1),    0,          0,   0,  0, (void *) "From", NULL, NULL },
	{ d_mr_cset_proc,         10,  42, 132,  12,      0,     0,    0, D_SETTABLE,   0,  0, (void *)massRecolorSrc8Bit, NULL, NULL },
	{ jwin_text_proc,         12,  60, 176,   8, vc(15), vc(1),    0,          0,   0,  0, (void *) "To", NULL, NULL },
	{ d_mr_cset_proc,         10,  70, 132,  12,      0,     0,    0, D_SETTABLE,   0,  0, (void *)massRecolorDest8Bit, NULL, NULL },
	{ d_mr_palette_proc,     144,  32, 132, 150, vc(15), vc(1),    0,          0,   0,  0, (void *) NULL, NULL, NULL },
	
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
	
	recolor_4bit_dlg[0].dp2=get_zc_font(font_lfont);
	recolor_8bit_dlg[0].dp2=get_zc_font(font_lfont);
	
	for(int32_t i=0; i<=13; i++)
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
	for(int32_t i=0; i<=13; i++)
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
		HANDLE_CLOSE_ZQDLG();
		if(exiting_program) break;
		if(type==MR_4BIT)
		{
			ret=do_zqdialog(recolor_4bit_dlg, MR4_OK);
			if(ret==MR4_SWITCH)
				type=MR_8BIT;
		}
		else
		{
			ret=do_zqdialog(recolor_8bit_dlg, MR8_OK);
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
	jwin_center_dialog(move_textbox_list_dlg);
	jwin_center_dialog(recolor_4bit_dlg);
	jwin_center_dialog(recolor_8bit_dlg);
}

//.ZCOMBO

int32_t readcombo_loop(PACKFILE* f, word section_version, newcombo& temp_combo);
int32_t writecombo_loop(PACKFILE *f, word section_version, newcombo const& tmp_cmb);

int32_t readcombofile_old(PACKFILE *f, int32_t skip, byte nooverwrite, int32_t zversion,
	dword section_version, int32_t index, int32_t count)
{
	newcombo temp_combo;
	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		temp_combo.clear();
		combo_trigger& temp_trigger = temp_combo.triggers.emplace_back();
		if(!p_igetw(&temp_combo.tile,f))
		{
			return 0;
		}
		temp_combo.o_tile = temp_combo.tile;
			
		if(!p_getc(&temp_combo.flip,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.walk,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.type,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.csets,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.frames,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.speed,f))
		{
			return 0;
		}
			
		if(!p_igetw(&temp_combo.nextcombo,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.nextcset,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.flag,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.skipanim,f))
		{
			return 0;
		}
			
		if(!p_igetw(&temp_combo.nexttimer,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.skipanimy,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_combo.animflags,f))
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
					if(!p_igetl(&temp_combo.attributes[q],f))
					{
						return 0;
					}
				}
				if(!p_igetl(&temp_combo.usrflags,f))
				{
						return 0;
				}	 
				for ( int32_t q = 0; q < 3; q++ ) 
				{
					if(!p_igetl(&temp_trigger.triggerflags[q],f))
					{
						return 0;
					}
				}
				   
				if(!p_igetl(&temp_trigger.triggerlevel,f))
				{
						return 0;
				}
				if(section_version >= 22)
				{
					if(!p_getc(&temp_trigger.triggerbtn,f))
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
							temp_trigger.triggerflags[0] |= combotriggerCMBTYPEFX;
					}
				}
				if(section_version >= 24)
				{
					if(!p_getc(&temp_trigger.triggeritem,f))
					{
						return 0;
					}
					if(!p_getc(&temp_trigger.trigtimer,f))
					{
						return 0;
					}
				}
				if(section_version >= 25)
				{
					if(!p_getc(&temp_trigger.trigsfx,f))
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
					if(!p_igetl(&temp_trigger.trigchange,f))
					{
						return qe_invalid;
					}
				}
				else
				{
					if(temp_trigger.triggerflags[0] & 0x00040000) //'next'
						temp_trigger.trigchange = 1;
					else if(temp_trigger.triggerflags[0] & 0x00080000) //'prev'
						temp_trigger.trigchange = -1;
					else temp_trigger.trigchange = 0;
					temp_trigger.triggerflags[0] &= ~(0x00040000|0x00080000);
				}
				if(section_version >= 29)
				{
					if(!p_igetw(&temp_trigger.trigprox,f))
					{
						return qe_invalid;
					}
					if(!p_getc(&temp_trigger.trigctr,f))
					{
						return qe_invalid;
					}
					if(!p_igetl(&temp_trigger.trigctramnt,f))
					{
						return qe_invalid;
					}
				}
				else
				{
					temp_trigger.trigprox = 0;
					temp_trigger.trigctr = 0;
					temp_trigger.trigctramnt = 0;
				}
				if(section_version >= 30)
				{
					if(!p_getc(&temp_trigger.triglbeam,f))
					{
						return qe_invalid;
					}
				}
				else temp_trigger.triglbeam = 0;
				if(section_version >= 31)
				{
					if(!p_getc(&temp_trigger.trigcschange,f))
					{
						return qe_invalid;
					}
					if(!p_igetw(&temp_trigger.spawnitem,f))
					{
						return qe_invalid;
					}
					if(!p_igetw(&temp_trigger.spawnenemy,f))
					{
						return qe_invalid;
					}
					if(!p_getc(&temp_trigger.exstate,f))
					{
						return qe_invalid;
					}
					if(!p_igetl(&temp_trigger.spawnip,f))
					{
						return qe_invalid;
					}
					if(!p_getc(&temp_trigger.trigcopycat,f))
					{
						return qe_invalid;
					}
				}
				else
				{
					temp_trigger.trigcschange = 0;
					temp_trigger.spawnitem = 0;
					temp_trigger.spawnenemy = 0;
					temp_trigger.exstate = -1;
					temp_trigger.spawnip = 0;
					temp_trigger.trigcopycat = 0;
				}
				if(section_version >= 32)
				{
					if(!p_getc(&temp_trigger.trigcooldown,f))
					{
						return qe_invalid;
					}
				}
				else
				{
					temp_trigger.trigcooldown = 0;
				}
				char label[12];
				label[11] = '\0';
				for ( int32_t q = 0; q < 11; q++ ) 
				{
					if(!p_getc(&label[q],f))
					{
						return 0;
					}
				}
				temp_combo.label = label;
			}
			if  ( section_version >= 13 )
			{
				for ( int32_t q = 0; q < NUM_COMBO_ATTRIBUTES; q++ )
				{
					if(!p_getc(&temp_combo.attribytes[q],f))
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
				combobuf[index+(tilect)] = temp_combo;
			}
		}
	}
	
	return 1;
}

int32_t readcombofile(PACKFILE *f, int32_t skip, byte nooverwrite, int32_t start)
{
	dword section_version=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	
	if(!p_igetl(&zversion,f))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f))
	{
		return 0;
	}
	if(!read_deprecated_section_cversion(f))
	{
		return 0;
	}
	
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zcombo packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_COMBOS ))
	{
		al_trace("Cannot read .zcombo packfile made using V_COMBOS (%d)\n", section_version);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zcombo packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	int32_t index = 0;
	int32_t count = 0;
	
	//tile id
	if(!p_igetl(&index,f))
	{
		return 0;
	}
	if(start > -1) index = start;
	
	//tile count
	if(!p_igetl(&count,f))
	{
		return 0;
	}
	reset_combo_animations();
	reset_combo_animations2();
	
	if(section_version < 33)
		return readcombofile_old(f,skip,nooverwrite,zversion,section_version,index,count);
	
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
				combobuf[q] = temp_combo;
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
	
	if(!write_deprecated_section_cversion(section_version,f))
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
	int32_t zversion = 0;
	int32_t zbuild = 0;
	word tempword = 0;
	
	if(!p_igetl(&zversion,f))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f))
	{
		return 0;
	}
	if(!read_deprecated_section_cversion(f))
	{
		return 0;
	}
	al_trace("readoneweapon section_version: %d\n", section_version);

	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zalias packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_COMBOALIASES ))
	{
		al_trace("Cannot read .zalias packfile made using V_COMBOALIASES (%d)\n", section_version);
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
	if(!p_igetl(&index,f))
	{
		return 0;
	}
	al_trace("Reading combo: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f))
	{
		return 0;
	}
	al_trace("Reading combo: count(%d)\n", count);
	
	combo_alias temp_alias;

	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		temp_alias.clear();
		if(!p_igetw(&temp_alias.combo,f))
			{
				return 0;
			}
			
			if(!p_getc(&temp_alias.cset,f))
			{
				return 0;
			}
			
			
		
		if(!p_igetl(&count2,f))
			{
				return 0;
			}
		al_trace("Read, Combo alias count is: %d\n", count2);
			if(!p_getc(&temp_alias.width,f))
			{
				return 0;
			}
			
			if(!p_getc(&temp_alias.height,f))
			{
				return 0;
			}
			
			if(!p_getc(&temp_alias.layermask,f))
			{
				return 0;
			}
			//These values are flexible, and may differ in size, so we delete them 
			//and recreate them at the correct size on the pointer.
			temp_alias.combos.clear();
			temp_alias.csets.clear();
			for(int32_t k=0; k<count2; k++)
			{
				if(!p_igetw(&tempword,f))
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
				if(!p_getc(&tempcset,f))
				//if(!p_getc(&temp_alias.csets[k],f))
		{
					return 0;
				}
		else
		{
			
			temp_alias.csets[k] = tempcset;
		}
			}
		combo_aliases[index+(tilect)] = temp_alias;
	}
	
	//::memcpy(&(newtilebuf[tile_index]),&temptile,sizeof(tiledata));
	
			
	return 1;
	
}

int32_t readcomboaliasfile_to_location(PACKFILE *f, int32_t start)
{
	dword section_version=0;
	int32_t zversion = 0;
	int32_t zbuild = 0;
	
	if(!p_igetl(&zversion,f))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f))
	{
		return 0;
	}
	if(!read_deprecated_section_cversion(f))
	{
		return 0;
	}
	al_trace("readcomboaliasfile_to_location section_version: %d\n", section_version);

	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zalias packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	else if ( ( section_version > V_COMBOALIASES ))
	{
		al_trace("Cannot read .zalias packfile made using V_COMBOALIASES (%d)\n", section_version);
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
	if(!p_igetl(&index,f))
	{
		return 0;
	}
	al_trace("Reading tile: index(%d)\n", index);
	
	//tile count
	if(!p_igetl(&count,f))
	{
		return 0;
	}
	al_trace("Reading tile: count(%d)\n", count);
	
	
	combo_alias temp_alias;

	for ( int32_t tilect = 0; tilect < count; tilect++ )
	{
		temp_alias.clear();
		if(!p_igetw(&temp_alias.combo,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_alias.cset,f))
		{
			return 0;
		}
			
		int32_t count2 = 0;
		
		if(!p_igetl(&count2,f))
		{
			return 0;
		}
		
		if(!p_getc(&temp_alias.width,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_alias.height,f))
		{
			return 0;
		}
			
		if(!p_getc(&temp_alias.layermask,f))
		{
			return 0;
		}
		//These values are flexible, and may differ in size, so we delete them 
		//and recreate them at the correct size on the pointer.
		temp_alias.combos.clear();
		temp_alias.csets.clear();
		
		for(int32_t k=0; k<count2; k++)
		{
			if(!p_igetw(&tempword,f))
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
			if(!p_getc(&tempcset,f))
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
			combo_aliases[start + (tilect)] = temp_alias;
		}
	}
	return 1;
}
int32_t writecomboaliasfile(PACKFILE *f, int32_t index, int32_t count)
{
	al_trace("Running writecomboaliasfile\n");
	dword section_version=V_COMBOALIASES;
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
	
	if(!write_deprecated_section_cversion(section_version,f))
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

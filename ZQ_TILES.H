//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zq_tiles.cc
//
//  Tile editor stuff, etc., for ZQuest.
//
//--------------------------------------------------------

#ifndef _ZQ_TILES_H_
#define _ZQ_TILES_H_

#include "zdefs.h"

#include "zc_alleg.h"

/*********************************/
/*****    Tiles & Combos    ******/
/*********************************/

int d_combo_proc(int msg,DIALOG *d,int c);
void go_tiles();
void comeback_tiles();
void go_combos();
void comeback_combos();
void little_x(BITMAP *dest, int x, int y, int c, int s);
void zoomtile16(BITMAP *dest,int tile,int x,int y,int cset,int flip,int m);
void draw_button(BITMAP *dest,int x,int y,int w,int h,char *text,int bg,int fg,int flags,bool jwin);
bool do_button(int x,int y,int w,int h,char *text,int bg,int fg,bool jwin);
bool do_button_reset(int x,int y,int w,int h,char *text,int bg,int fg,bool jwin);
void draw_layerradio(BITMAP *dest,int x,int y,int bg,int fg, int value);
void do_layerradio(BITMAP *dest,int x,int y,int bg,int fg,int &value);
void draw_checkbox(BITMAP *dest,int x,int y,int bg,int fg, bool value);
bool do_checkbox(BITMAP *dest,int x,int y,int bg,int fg,int &value);

//*************** tile flood fill stuff **************

//extern byte tf_c;
//extern byte tf_u;

void tile_floodfill_rec(int x,int y);
void tile_floodfill(int tile,int x,int y,byte c);

//***************** tile editor  stuff *****************

//extern int c1;
//extern int c2;
//extern int bgc;
enum { t_pen, t_brush, t_recolor, t_eyedropper, t_move, t_select, t_max };
//extern int tool;
//extern int tool_cur;
//extern int drawing;

void update_tool_cursor();
void draw_edit_scr(int tile,int flip,int cs,byte *oldtile);
void normalize(int tile,int tile2, bool rect_sel, int flip);
void rotate_tile(int tile, bool backward);
void wrap_tile(int tile, int offset);
void edit_tile(int tile,int flip,int &cs);

/*  Grab Tile Code  */

//extern void *imagebuf;
//extern long imagesize;
//extern int  imagetype;
//extern int imagex,imagey,selx,sely;
//extern int bp,grabmode,romofs,romtilemode, romtilecols;
//extern int grabmask;
//extern PALETTE imagepal;

void puttileROM(BITMAP *dest,int x,int y,byte *src,int cs);
void draw_grab_scr(int tile,int cs,byte *newtile,int bg,int white,int txt, int width, int height);
void load_imagebuf();
void leech_tiles(byte *dest,int start);
void grab(byte *dest,byte *def, int width, int height);
void grab_tile(int tile,int &cs);
void draw_tiles(int first,int cs);
int tile_col(int tile);
int tile_row(int tile);
int tile_page(int tile);
int tile_page_row(int tile);
void tile_info_0(int tile,int tile2,int cs,int copy,int copycnt,int page,bool rect_sel);
void tile_info_1(int oldtile,int oldflip,int oldcs,int tile,int flip,int cs,int copy,int page);
void reset_tile(int t);
void copy_tiles(int &tile,int &tile2,int &copy,int &copycnt, bool rect_sel, bool move);
void copy_combos(int &tile,int &tile2,int &copy,int &copycnt);
void move_combos(int &tile,int &tile2,int &copy,int &copycnt);
void delete_tiles(int &tile,int &tile2,bool rect_sel);
void overlay_tile(int dest,int src);
void sel_tile(int &tile, int &tile2, int &first, int type, int s);
int select_tile(int &tile,int &flip,int type,int &cs,bool edit_cs);
int onTiles();
void draw_combo(int x,int y,int c,int cs);
void draw_combos(int page,int cs,bool cols);
void combo_info(int tile,int tile2,int cs,int copy,int copycnt,int page,int buttons);
void sel_combo(int &tile, int &tile2, int s, bool cols);

//extern word ctable[MAXCOMBOS];

int select_combo_2(int &tile,int &cs);
int combo_screen();
int onCombos();

//extern int edit_combo_cset;

int d_ctile_proc(int msg,DIALOG *d,int c);
int d_combo_loader(int msg,DIALOG *d,int c);

typedef struct combotype_struct
{
  char *s;
  int i;
} combotype_struct;

//extern combotype_struct bict[cMAX];
//extern int bict_cnt;

void build_bict_list();
char *combotypelist(int index, int *list_size);

int click_d_ctile_proc();
int onCmb_dlg_h();
int onCmb_dlg_v();
int onCmb_dlg_r();
void edit_combo(int c,bool freshen,int cs);
int d_itile_proc(int msg,DIALOG *d,int c);
int onIcons();
void center_zq_tiles_dialogs();
int d_combo_proc(int msg,DIALOG *d,int c);
#endif

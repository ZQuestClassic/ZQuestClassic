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
extern tiledata     *newundotilebuf;
extern newcombo     *undocombobuf;
extern byte cset_reduce_table[PAL_SIZE];
void calc_cset_reduce_table(PALETTE pal, int cs);

void register_used_tiles();
int d_comboframe_proc(int msg, DIALOG *d, int c);
int d_combo_proc(int msg,DIALOG *d,int c);
void go_tiles();
void go_slide_tiles(int columns, int rows, int top, int left);
void comeback_tiles();
void go_combos();
void comeback_combos();
void little_x(BITMAP *dest, int x, int y, int c, int s);
void zoomtile16(BITMAP *dest,int tile,int x,int y,int cset,int flip,int m);
void draw_text_button(BITMAP *dest,int x,int y,int w,int h,const char *text,int bg,int fg,int flags,bool jwin);
bool do_text_button(int x,int y,int w,int h,const char *text,int bg,int fg,bool jwin);
bool do_text_button_reset(int x,int y,int w,int h,const char *text,int bg,int fg,bool jwin);
void draw_graphics_button(BITMAP *dest,int x,int y,int w,int h,BITMAP *bmp,BITMAP *bmp2,int bg,int fg,int flags,bool jwin,bool overlay);
bool do_graphics_button(int x,int y,int w,int h,BITMAP *bmp,BITMAP *bmp2,int bg,int fg,bool jwin,bool overlay);
bool do_graphics_button_reset(int x,int y,int w,int h,BITMAP *bmp,BITMAP *bmp2,int bg,int fg,bool jwin,bool overlay);
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
enum { t_pen, t_fill, t_recolor, t_eyedropper, t_move, t_select, t_wand, t_max };
//extern int tool;
//extern int tool_cur;
//extern int drawing;

void update_tool_cursor();
void draw_edit_scr(int tile,int flip,int cs,byte *oldtile, bool create_tbar);
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
void draw_grab_scr(int tile,int cs,byte *newtile,int black,int white,int width, int height, byte *newformat);
void load_imagebuf();
bool leech_tiles(tiledata *dest,int start,int cs);
void grab(byte (*dest)[256],byte *def, int width, int height, int oformat, byte *newformat);
void grab_tile(int tile,int &cs);
void draw_tiles(int first,int cs, int f);
int tile_col(int tile);
int tile_row(int tile);
int tile_page(int tile);
int tile_page_row(int tile);
void tile_info_0(int tile,int tile2,int cs,int copy,int copycnt,int page,bool rect_sel);
void tile_info_1(int oldtile,int oldflip,int oldcs,int tile,int flip,int cs,int copy,int page);
//void reset_tile(tiledata *buf, int t, int format);
bool copy_tiles(int &tile,int &tile2,int &copy,int &copycnt, bool rect_sel, bool move);
bool copy_tiles_united(int &tile,int &tile2,int &copy,int &copycnt, bool rect_sel, bool move);
void copy_combos(int &tile,int &tile2,int &copy,int &copycnt, bool masscopy);
void move_combos(int &tile,int &tile2,int &copy,int &copycnt);
void delete_tiles(int &tile,int &tile2,bool rect_sel);
void overlay_tile2(int dest,int src,int cs,bool backwards);
void sel_tile(int &tile, int &tile2, int &first, int type, int s);
int select_tile(int &tile,int &flip,int type,int &cs,bool edit_cs, int exnow=0);
int onTiles();
void draw_combo(BITMAP *dest, int x,int y,int c,int cs);
void draw_combos(int page,int cs,bool cols);
void combo_info(int tile,int tile2,int cs,int copy,int copycnt,int page,int buttons);
void sel_combo(int &tile, int &tile2, int s, bool cols);

//extern word ctable[MAXCOMBOS];

int select_combo_2(int &tile,int &cs);
int combo_screen(int pg, int tl);
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
const char *combotypelist(int index, int *list_size);

int click_d_ctile_proc();
int onCmb_dlg_h();
int onCmb_dlg_v();
int onCmb_dlg_r();
bool edit_combo(int c,bool freshen,int cs);
int d_itile_proc(int msg,DIALOG *d,int c);
int onIcons();
void center_zq_tiles_dialogs();
int d_combo_proc(int msg,DIALOG *d,int c);
void center_zq_tiles_dialog();
#endif
 

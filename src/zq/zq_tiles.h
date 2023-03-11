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

#include "base/zdefs.h"

#include "base/zc_alleg.h"

/*********************************/
/*****    Tiles & Combos    ******/
/*********************************/
extern tiledata     *newundotilebuf;
extern newcombo     *undocombobuf;
extern byte cset_reduce_table[PAL_SIZE];
void calc_cset_reduce_table(PALETTE pal, int32_t cs);

void register_used_tiles();

int32_t readcomboaliasfile(PACKFILE *f);
int32_t readcomboaliasfile_to_location(PACKFILE *f, int32_t start);
int32_t writecomboaliasfile(PACKFILE *f, int32_t index, int32_t count);

int32_t readtilefile(PACKFILE *f);
int32_t writetilefile(PACKFILE *f, int32_t index, int32_t count);
int32_t readtilefile_to_location(PACKFILE *f, int32_t start);
int32_t readtilefile_to_location(PACKFILE *f, int32_t start, int32_t skip);

int32_t readcombofile(PACKFILE *f, int32_t skip, byte nooverwrite, int32_t start = -1);
int32_t readcombofile_to_location(PACKFILE *f, int32_t start, byte nooverwrite, int32_t skip);
int32_t writecombofile(PACKFILE *f, int32_t index, int32_t count);

int32_t d_comboframe_proc(int32_t msg, DIALOG *d, int32_t c);
int32_t d_combo_proc(int32_t msg,DIALOG *d,int32_t c);
void go_tiles();
void go_slide_tiles(int32_t columns, int32_t rows, int32_t top, int32_t left);
void comeback_tiles();
void go_combos();
void comeback_combos();
void little_x(BITMAP *dest, int32_t x, int32_t y, int32_t c, int32_t s);
void zoomtile16(BITMAP *dest,int32_t tile,int32_t x,int32_t y,int32_t cset,int32_t flip,int32_t m);
void draw_text_button(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t bg,int32_t fg,int32_t flags,bool jwin);
void draw_layer_button(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t flags);
bool do_layer_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,const char *text, int32_t flags, bool toggleflag=false);
bool do_text_button(int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t bg,int32_t fg,bool jwin);
bool do_text_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,const char *text,int32_t bg,int32_t fg,bool jwin,bool sel=false);
void draw_graphics_button(BITMAP *dest,int32_t x,int32_t y,int32_t w,int32_t h,BITMAP *bmp,BITMAP *bmp2,int32_t bg,int32_t fg,int32_t flags,bool jwin,bool overlay);
bool do_graphics_button(int32_t x,int32_t y,int32_t w,int32_t h,BITMAP *bmp,BITMAP *bmp2,int32_t bg,int32_t fg,bool jwin,bool overlay);
bool do_graphics_button_reset(int32_t x,int32_t y,int32_t w,int32_t h,BITMAP *bmp,BITMAP *bmp2,int32_t bg,int32_t fg,bool jwin,bool overlay);
void draw_layerradio(BITMAP *dest,int32_t x,int32_t y,int32_t bg,int32_t fg, int32_t value);
void do_layerradio(BITMAP *dest,int32_t x,int32_t y,int32_t bg,int32_t fg,int32_t &value);
void draw_checkbox(BITMAP *dest,int32_t x,int32_t y,int32_t wid,int32_t hei,int32_t bg,int32_t fg, bool value);
void draw_checkbox(BITMAP *dest,int32_t x,int32_t y,int32_t sz,int32_t bg,int32_t fg, bool value);
bool do_checkbox(BITMAP *dest,int32_t x,int32_t y,int32_t wid,int32_t hei,int32_t bg,int32_t fg,int32_t &value);
bool do_checkbox(BITMAP *dest,int32_t x,int32_t y,int32_t sz,int32_t bg,int32_t fg,int32_t &value);

//*************** tile flood fill stuff **************

//extern byte tf_c;
//extern byte tf_u;

void tile_floodfill_rec(int32_t x,int32_t y);
void tile_floodfill(int32_t tile,int32_t x,int32_t y,byte c);

//***************** tile editor  stuff *****************

//extern int32_t c1;
//extern int32_t c2;
//extern int32_t bgc;
enum { t_pen, t_fill, t_recolor, t_eyedropper, t_move, t_select, t_wand, t_max };
//extern int32_t tool;
//extern int32_t tool_cur;
//extern int32_t drawing;

void update_tool_cursor();
void draw_edit_scr(int32_t tile,int32_t flip,int32_t cs,byte *oldtile, bool create_tbar);
void normalize(int32_t tile,int32_t tile2, bool rect_sel, int32_t flip);
void rotate_tile(int32_t tile, bool backward);
void wrap_tile(int32_t tile, int32_t offset);
void shift_tile_colors(int32_t tile, int32_t amount, bool ignore_transparent);
void edit_tile(int32_t tile,int32_t flip,int32_t &cs);

/*  Grab Tile Code  */

//extern void *imagebuf;
//extern int32_t imagesize;
//extern int32_t  imagetype;
//extern int32_t imagex,imagey,selx,sely;
//extern int32_t bp,grabmode,romofs,romtilemode, romtilecols;
//extern int32_t grabmask;
//extern PALETTE imagepal;

void puttileROM(BITMAP *dest,int32_t x,int32_t y,byte *src,int32_t cs);
void draw_grab_scr(int32_t tile,int32_t cs,byte *newtile,int32_t black,int32_t white,int32_t width, int32_t height, byte *newformat);
void load_imagebuf();
bool leech_tiles(tiledata *dest,int32_t start,int32_t cs);
void grab(byte(*dest)[256],byte *def, int32_t width, int32_t height, int32_t oformat, byte *newformat);
void grab_tile(int32_t tile,int32_t &cs);
void draw_tiles(int32_t first,int32_t cs, int32_t f);
void draw_tiles(BITMAP* dest,int32_t first,int32_t cs, int32_t f,bool large,bool true_empty = false);
int32_t tile_col(int32_t tile);
int32_t tile_row(int32_t tile);
int32_t tile_page(int32_t tile);
int32_t tile_page_row(int32_t tile);
void tile_info_0(int32_t tile,int32_t tile2,int32_t cs,int32_t copy,int32_t copycnt,int32_t page,bool rect_sel);
void tile_info_1(int32_t oldtile,int32_t oldflip,int32_t oldcs,int32_t tile,int32_t flip,int32_t cs,int32_t copy,int32_t page, bool always_use_flip=false);
//void reset_tile(tiledata *buf, int32_t t, int32_t format);
bool copy_tiles(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move);
bool copy_tiles_united(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move);
bool copy_tiles_floodfill(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move);
bool copy_tiles_united_floodfill(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect, bool move);
bool overlay_tiles_united(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect, bool move, int32_t cs, bool backwards);
bool overlay_tiles(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move, int32_t cs, bool backwards);
bool overlay_tiles_mass(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect_sel, bool move, int32_t cs, bool backwards);
bool overlay_tile_united_mass(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool rect, bool move, int32_t cs, bool backwards);
void copy_combos(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt, bool masscopy);
void move_combos(int32_t &tile,int32_t &tile2,int32_t &copy,int32_t &copycnt);
void delete_tiles(int32_t &tile,int32_t &tile2,bool rect_sel);
void overlay_tile2(int32_t dest,int32_t src,int32_t cs,bool backwards);
void sel_tile(int32_t &tile, int32_t &tile2, int32_t &first, int32_t type, int32_t s);
int32_t select_tile(int32_t &tile,int32_t &flip,int32_t type,int32_t &cs,bool edit_cs, int32_t exnow=0, bool always_use_flip=false);
int32_t select_tile_2(int32_t &tile,int32_t &flip,int32_t type,int32_t &cs,bool edit_cs, int32_t exnow=0, bool always_use_flip=false);
int32_t select_dmap_tile(int32_t &tile,int32_t &flip,int32_t type,int32_t &cs,bool edit_cs, int32_t exnow=0, bool always_use_flip=false);
int32_t onTiles();
void draw_combo(BITMAP *dest, int32_t x,int32_t y,int32_t c,int32_t cs);
void draw_combos(int32_t page,int32_t cs,bool cols);
void combo_info(int32_t tile,int32_t tile2,int32_t cs,int32_t copy,int32_t copycnt,int32_t page,int32_t buttons);
void sel_combo(int32_t &tile, int32_t &tile2, int32_t s, bool cols);

bool select_combo_2(int32_t &cmb,int32_t &cs);
bool select_combo_3(int32_t &cmb,int32_t &cs);
int32_t combo_screen(int32_t pg, int32_t tl);
int32_t onCombos();

//extern int32_t edit_combo_cset;

int32_t d_ctile_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t d_combo_loader(int32_t msg,DIALOG *d,int32_t c);

int32_t click_d_ctile_proc();
int32_t onCmb_dlg_h();
int32_t onCmb_dlg_v();
int32_t onCmb_dlg_r();
bool edit_combo(int32_t c,int32_t cs);
int32_t d_itile_proc(int32_t msg,DIALOG *d,int32_t c);
int32_t onIcons();
void center_zq_tiles_dialogs();
int32_t d_combo_proc(int32_t msg,DIALOG *d,int32_t c);
void center_zq_tiles_dialog();
#endif


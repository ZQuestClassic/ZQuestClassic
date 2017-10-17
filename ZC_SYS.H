//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zc_sys.h
//
//  System functions, input handlers, GUI stuff, etc.
//  for Zelda Classic.
//
//--------------------------------------------------------

#ifndef _ZC_SYS_H_
#define _ZC_SYS_H_

#include "zdefs.h"

extern MENU the_menu[];

bool game_vid_mode(int mode,int wait);
void Z_init_timers();
void Z_init_sound();

void load_game_configs();
void save_game_configs();

void draw_lens_under();
void draw_lens_over();
void fix_menu();
void f_Quit(int type);
void advanceframe();
void updatescr();
void syskeys();
void System();
void system_pal();
void game_pal();
int  onSave();
int  onQuit();
int  onReset();
int  onExit();
void fix_dialogs();

int onCheatBombs();

int  next_press_key();
int  next_press_btn();
bool joybtn(int b);
bool ReadKey(int k);
void eat_buttons();

bool Up();
bool Down();
bool Left();
bool Right();
bool DrunkUp();
bool DrunkDown();
bool DrunkLeft();
bool DrunkRight();

bool rUp();
bool rDown();
bool rLeft();
bool rRight();
bool DrunkrUp();
bool DrunkrDown();
bool DrunkrLeft();
bool DrunkrRight();

bool cAbtn();
bool cBbtn();
bool cSbtn();
bool cMbtn();
bool cLbtn();
bool cRbtn();
bool cPbtn();
bool DrunkcAbtn();
bool DrunkcBbtn();
bool DrunkcSbtn();
bool DrunkcMbtn();
bool DrunkcLbtn();
bool DrunkcRbtn();
bool DrunkcPbtn();

bool rAbtn();
bool rBbtn();
bool rSbtn();
bool rMbtn();
bool rLbtn();
bool rRbtn();
bool rPbtn();
bool DrunkrAbtn();
bool DrunkrBbtn();
bool DrunkrSbtn();
bool DrunkrMbtn();
bool DrunkrLbtn();
bool DrunkrRbtn();
bool DrunkrPbtn();

int after_time();

enum {bosCIRCLE=0, bosOVAL, bosTRIANGLE, bosSMAS, bosMAX};

void go();
void comeback();
void dump_pal(BITMAP *dest);
void show_paused();
void show_fps();
bool game_vid_mode(int mode,int wait);
void init_NES_mode();
  
extern int black_opening_count;
extern int black_opening_x,black_opening_y;
extern int black_opening_shape;

void zapout();
void zapin();
void wavyout();
void wavyin();
void blackscr(int fcnt,bool showsubscr);
void black_opening(BITMAP *dest,int x,int y,int a,int max_a);
void close_black_opening(int x, int y, bool wait);
void open_black_opening(int x, int y, bool wait);
void openscreen();
int  TriforceCount();

bool item_disabled(int item_type, int item);
bool can_use_item(int item_type, int item);
bool has_item(int item_type, int item);
int high_item(int jmax, int item_type, bool consecutive, int itemcluster, bool usecluster);
int current_item(int item_type, bool consecutive);
int high_flag(int i, int item_type, bool consecutive);
int item_tile_mod();
int dmap_tile_mod();

void jukebox(int index);
void jukebox(int index,int loop);
void play_DmapMusic();
void music_pause();
void music_resume();
void music_stop();
void master_volume(int dv,int mv);
int  sfx_count();
void sfx_cleanup();
bool sfx_init(int index);
void sfx(int index,int pan,bool loop);
void cont_sfx(int index);
void stop_sfx(int index);
void adjust_sfx(int index,int pan,bool loop);
void pause_sfx(int index);
void resume_sfx(int index);
void pause_all_sfx();
void resume_all_sfx();
void stop_sfx(int index);
void kill_sfx();
int  pan(int x);
void zc_putpixel(int layer, int x, int y, int cset, int color, int timer);
#endif                                                      // _ZC_SYS_H_

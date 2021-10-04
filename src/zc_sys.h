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
#include "jwinfsel.h"
extern MENU the_player_menu[];
extern MENU the_player_menu2[];
extern MENU the_player_menu_zc_on_left[];
extern MENU the_player_menu_zc_on_left2[];

void large_dialog(DIALOG *d);
void large_dialog(DIALOG *d, float RESIZE_AMT);

bool game_vid_mode(int mode,int wait);
int Z_init_timers();
void Z_remove_timers();
void Z_init_sound();

void load_game_configs();
void save_game_configs();
int zc_load_zmod_module_file();
bool zc_getname(const char *prompt,const char *ext,EXT_LIST *list,const char *def,bool usefilename);
bool zc_getname_nogo(const char *prompt,const char *ext,EXT_LIST *list,const char *def,bool usefilename);

extern bool midi_paused;
#ifdef _WIN32
void do_DwmFlush();
#endif

void draw_lens_under(BITMAP *dest, bool layer);
void draw_lens_over();
void fix_menu();
int onFullscreenMenu();
void f_Quit(int type);
void advanceframe(bool allowwavy, bool sfxcleaup = true, bool allowF6Script = true);
void updatescr(bool allowwavy);
void syskeys();
void checkQuitKeys();
bool CheatModifierKeys();
void System();
void system_pal();
void switch_out_callback();
void switch_in_callback();
void game_pal();
int  onSave();
int  onQuit();
int  onTryQuitMenu();
int  onTryQuit(bool inMenu = false);
int  onReset();
int  onExit();
void fix_dialogs();
int onMIDIPatch();

int onCheatBombs();
int onCheatArrows();

int  next_press_key();
int  next_press_btn();
bool joybtn(int b);
bool zc_readkey(int k, bool ignoreDisable = false);
bool zc_getkey(int k, bool ignoreDisable = false);
bool zc_readrawkey(int k, bool ignoreDisable = false);
bool zc_getrawkey(int k, bool ignoreDisable = false);
void update_keys();
bool zc_disablekey(int k, bool val);
void eat_buttons();

extern bool control_state[18];
extern bool disable_control[18];
extern bool drunk_toggle_state[11];
extern bool disabledKeys[127];
extern bool KeyInput[127]; //ZScript 'Input->Key[]'
extern bool KeyPress[127]; //ZScript 'Input->KeyPress[]'
extern bool key_truestate[127]; //Internal, used for ZScript 'Input->KeyPress[]'
extern bool button_press[18];
extern int cheat_modifier_keys[4]; //two options each, default either control and either shift
extern const char *qst_dir_name;
extern char qst_files_path[2048];

extern bool button_hold[18];

void load_control_state();
extern int sfx_voice[WAV_COUNT];

bool getInput(int btn, bool press = false, bool drunk = false, bool ignoreDisable = false, bool eatEntirely = false);
bool getIntBtnInput(byte intbtn, bool press = false, bool drunk = false, bool ignoreDisable = false, bool eatEntirely = false);
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
bool cEx1btn();
bool cEx2btn();
bool cEx3btn();
bool cEx4btn();
bool AxisUp();
bool AxisDown();
bool AxisLeft();
bool AxisRight();
bool DrunkcAbtn();
bool DrunkcBbtn();
bool DrunkcEx1btn();
bool DrunkcEx2btn();
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
bool rEx1btn();
bool rEx2btn();
bool rEx3btn();
bool rEx4btn();
bool rAxisUp();
bool rAxisDown();
bool rAxisLeft();
bool rAxisRight();
bool DrunkrAbtn();
bool DrunkrBbtn();
bool DrunkrSbtn();
bool DrunkrMbtn();
bool DrunkrLbtn();
bool DrunkrRbtn();
bool DrunkrPbtn();
bool DrunkrEx1btn();
bool DrunkrEx2btn();
bool DrunkrEx3btn();
bool DrunkrEx4btn();

int after_time();

enum {bosCIRCLE=0, bosOVAL, bosTRIANGLE, bosSMAS, bosFADEBLACK, bosMAX};

void go();
void comeback();
void dump_pal(BITMAP *dest);
void show_paused(BITMAP *target);
void show_fps(BITMAP *target);
void show_saving(BITMAP *target);
bool game_vid_mode(int mode,int wait);
void init_NES_mode();

extern int black_opening_count;
extern int black_opening_x,black_opening_y;
extern int black_opening_shape;

void zapout();
void zapin();
void wavyout(bool showlink);
void wavyin();
void blackscr(int fcnt,bool showsubscr);
void black_opening(BITMAP *dest,int x,int y,int a,int max_a);
void black_fade(int fadeamnt);
void close_black_opening(int x, int y, bool wait, int shape = -1);
void open_black_opening(int x, int y, bool wait, int shape = -1);
void openscreen(int shape = -1);
void closescreen(int shape = -1);
int  TriforceCount();

bool item_disabled(int item);
bool can_use_item(int item_type, int item);
bool has_item(int item_type, int item);
//int high_item(int jmax, int item_type, bool consecutive, int itemcluster, bool usecluster);
int current_item(int item_type);
int current_item(int item_type, bool checkenabled);
int current_item_power(int item_type);
int current_item_id(int item_type, bool checkmagic = true);
int high_flag(int i, int item_type, bool consecutive);
int item_tile_mod(bool);
int dmap_tile_mod();

bool try_zcmusic(char *filename, int track, int midi);
bool try_zcmusic_ex(char *filename, int track, int midi);
int get_zcmusicpos();
void set_zcmusicpos(int position);
void set_zcmusicspeed(int speed);
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
void sfx(int index,int pan,bool loop, bool restart = true);
bool sfx_allocated(int index);
void cont_sfx(int index);
void stop_sfx(int index);
void adjust_sfx(int index,int pan,bool loop);
void pause_sfx(int index);
void resume_sfx(int index);
void pause_all_sfx();
void resume_all_sfx();
void stop_sfx(int index);
void stop_item_sfx(int family);
void kill_sfx();
int  pan(int x);
int  onSetSnapshotFormat();
void zc_putpixel(int layer, int x, int y, int cset, int color, int timer);
int onKeyboardEntry();
int onLetterGridEntry();
int onExtLetterGridEntry();

#ifdef ALLEGRO_LINUX_GP2X
extern int midi_strict;
#endif
#ifdef ALLEGRO_MACOSX
extern int midi_strict; //L
#endif
#endif                                                      // _ZC_SYS_H_


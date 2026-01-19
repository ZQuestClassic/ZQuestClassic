#ifndef ZC_SYS_H_
#define ZC_SYS_H_

#include "base/zdefs.h"
#include "base/jwinfsel.h"
#include "base/zsys.h"
#include "sound/zcmusic.h"
#include "advanced_music.h"
#include "zcsfx.h"

extern bool is_sys_pal;

void zc_exit(int code);
void render_zc();

bool flash_reduction_enabled(bool check_qr = true);

void large_dialog(DIALOG *d);
void large_dialog(DIALOG *d, float RESIZE_AMT);

int32_t Z_init_timers();
void Z_remove_timers();
void Z_init_sound();

void load_game_configs();
void save_game_configs();

#ifdef _WIN32
void do_DwmFlush();
#endif

void draw_lens_under(BITMAP *dest, bool layer);
void draw_lens_over(BITMAP *dest);
void fix_menu();
int32_t onFullscreenMenu();
void f_Quit(int32_t type);
void enqueue_qr_change(int qr, bool value);
void process_enqueued_qr_changes();
void advanceframe(bool allowwavy, bool sfxcleaup = true, bool allowF6Script = true);
void updatescr(bool allowwavy);
void syskeys();
void checkQuitKeys();
bool CheatModifierKeys();
void System();
void system_pal(bool force = false);
void switch_out_callback();
void switch_in_callback();
void game_pal();
int32_t  onSave();
int32_t  onQuit();
int32_t  onTryQuitMenu();
int32_t  onTryQuit(bool inMenu = false);
int32_t  onReset();
int32_t  onExit();
void fix_dialogs();
int32_t onPauseInBackground();

int32_t onCheatBombs();
int32_t onCheatArrows();

int32_t  next_press_key();
int32_t  next_joy_input(bool buttons);
int32_t button_pressed();
bool joybtn(int stick_idx, int b);
bool zc_readkey(int32_t k, bool ignoreDisable = false);
bool zc_getkey(int32_t k, bool ignoreDisable = false);
bool zc_readrawkey(int32_t k, bool ignoreDisable = false);
bool zc_getrawkey(int32_t k, bool ignoreDisable = false);
bool zc_get_system_key(int32_t k);
bool zc_read_system_key(int32_t k);
bool is_system_key(int32_t k);
void update_system_keys();
void update_keys();
void eat_buttons();

extern bool raw_control_state[controls::btnLast];
extern bool control_state[controls::btnLast];
extern bool disable_control[controls::btnLast];
extern bool drunk_toggle_state[11];
extern bool disabledKeys[127];
extern bool KeyInput[127]; //ZScript 'Input->Key[]'
extern bool KeyPress[127]; //ZScript 'Input->KeyPress[]'
extern bool key_current_frame[127];
extern bool key_previous_frame[127];
extern bool button_press[controls::btnLast];
extern const char *qst_dir_name;
extern char qst_files_path[2048];

extern bool button_hold[controls::btnLast];

void load_control_state();

bool getInput(int32_t btn, int input_flags = 0);

// returns {kb_pressed, joy_pressed} to differentiate kb 'esc' from gamepad 'menu'
std::pair<bool, bool> menu_buttons(bool just_pressed);
// calls the other function, and returns the || of the result (so, if EITHER is pressed)
bool menu_pressed(bool just_pressed);

byte getIntBtnInput(byte intbtn, int input_flags = 0);
byte checkIntBtnVal(byte intbtn, byte vals);

enum {bosCIRCLE=0, bosOVAL, bosTRIANGLE, bosSMAS, bosFADEBLACK, bosMAX};

void dump_pal(BITMAP *dest);
extern int game_mouse_index;
bool sys_mouse();
bool game_mouse();
void custom_mouse(BITMAP* bmp, int fx, int fy, bool sys_recolor, bool user_scale);
void recolor_mouse(BITMAP* bmp);
bool game_vid_mode(int32_t mode,int32_t wait);
void init_NES_mode();

extern int32_t black_opening_count;
extern int32_t black_opening_x,black_opening_y;
extern int32_t black_opening_shape;

void zapout();
void zapin();
void wavyout(bool showhero);
void wavyin();
void blackscr(int32_t fcnt,bool showsubscr);
void black_opening(BITMAP *dest,int32_t x,int32_t y,int32_t a,int32_t max_a);
void black_fade(int32_t fadeamnt);
void close_black_opening(int32_t x, int32_t y, bool wait, int32_t shape = -1);
void open_black_opening(int32_t x, int32_t y, bool wait, int32_t shape = -1);
void openscreen(int32_t shape = -1);
void closescreen(int32_t shape = -1);
int32_t  TriforceCount();

bool item_disabled(int32_t item);
bool can_use_item(int32_t item_type, int32_t item);
bool has_item(int32_t item_type, int32_t item);

int current_item(int itemtype, bool checkmagic = true, bool jinx_check = false, bool check_bunny = true);
int current_item_power(int itemtype, bool checkmagic = true, bool jinx_check = false, bool check_bunny = true);
int current_item_id(int item_type, bool checkmagic = true, bool jinx_check = false, bool check_bunny = true);
int32_t heart_container_id();
int32_t high_flag(int32_t i, int32_t item_type, bool consecutive);
void cache_tile_mod_clear();
int32_t item_tile_mod();
int32_t bunny_tile_mod();

int32_t get_emusic_volume();
int32_t get_zcmusicpos();
void set_zcmusicpos(int32_t position);
void set_zcmusicspeed(int32_t speed);
int32_t get_zcmusiclen();
void set_zcmusicloop(double start, double end);
void jukebox(int32_t index);
void jukebox(int32_t index,int32_t loop);
void music_pause();
void music_resume();
void music_stop();
void master_volume(int32_t dv,int32_t mv);

void stop_item_sfx(int32_t family);
int32_t  pan(int32_t x);
int32_t  onSetSnapshotFormat(SnapshotType format);
int32_t onSetBottom8Pixels(int option);
void updateShowBottomPixels();
int32_t onKeyboardEntry();
int32_t onLetterGridEntry();
int32_t onExtLetterGridEntry();


int32_t onShowLayer0();
int32_t onShowLayer1();
int32_t onShowLayer2();
int32_t onShowLayer3();
int32_t onShowLayer4();
int32_t onShowLayer5();
int32_t onShowLayer6();
int32_t onShowLayerO();
int32_t onShowLayerP();
int32_t onShowLayerS();
int32_t onShowLayerF();
int32_t onShowLayerW();
int32_t onShowLayerE();
int32_t onShowFFScripts();
int32_t onShowHitboxes();
int32_t onShowInfoOpacity();


#ifdef ALLEGRO_LINUX_GP2X
extern int32_t midi_strict;
#endif
#ifdef ALLEGRO_MACOSX
extern int32_t midi_strict; //L
#endif
#endif                                                      // _ZC_SYS_H_

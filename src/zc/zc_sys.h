#ifndef ZC_SYS_H_
#define ZC_SYS_H_

#include "base/zdefs.h"
#include "base/jwinfsel.h"
#include "base/zsys.h"
#include "sound/zcmusic.h"

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
void draw_lens_over();
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
bool joybtn(int32_t b);
const char* joybtn_name(int32_t b);
const char* joystick_name(int32_t s);
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

#define ZC_CONTROL_STATES 18
extern bool raw_control_state[ZC_CONTROL_STATES];
extern bool control_state[ZC_CONTROL_STATES];
extern bool disable_control[ZC_CONTROL_STATES];
extern bool drunk_toggle_state[11];
extern bool disabledKeys[127];
extern bool KeyInput[127]; //ZScript 'Input->Key[]'
extern bool KeyPress[127]; //ZScript 'Input->KeyPress[]'
extern bool key_current_frame[127];
extern bool key_previous_frame[127];
extern bool button_press[ZC_CONTROL_STATES];
extern int32_t cheat_modifier_keys[4]; //two options each, default either control and either shift
extern const char *qst_dir_name;
extern char qst_files_path[2048];

extern bool button_hold[ZC_CONTROL_STATES];

void load_control_state();
extern int32_t sfx_voice[WAV_COUNT];

bool getInput(int32_t btn, bool press = false, bool drunk = false, bool ignoreDisable = false, bool eatEntirely = false, bool peek = false);
byte getIntBtnInput(byte intbtn, bool press = false, bool drunk = false, bool ignoreDisable = false, bool eatEntirely = false, bool peek = false);
byte checkIntBtnVal(byte intbtn, byte vals);

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

enum uKey
{
	ukey_a, ukey_b, ukey_s, ukey_l, ukey_r, ukey_p, ukey_ex1, ukey_ex2, ukey_ex3, ukey_ex4,
	ukey_du, ukey_dd, ukey_dl, ukey_dr, ukey_mod1a, ukey_mod1b, ukey_mod2a, ukey_mod2b,
	num_ukey
};
void load_ukeys(int32_t* arr);
std::string get_ukey_name(int32_t k);

int32_t after_time();

enum {bosCIRCLE=0, bosOVAL, bosTRIANGLE, bosSMAS, bosFADEBLACK, bosMAX};

void go();
void comeback();
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
void play_DmapMusic();
void music_pause();
void music_resume();
void music_stop();
void master_volume(int32_t dv,int32_t mv);
int32_t  sfx_count();
void sfx_cleanup();
SAMPLE* sfx_get_sample(int32_t index);
bool sfx_init(int32_t index);
void sfx(int32_t index,int32_t pan,bool loop, bool restart = true, int32_t vol = 1000000, int32_t freq = -1);
int32_t sfx_get_default_freq(int32_t index);
int32_t sfx_get_length(int32_t index);
bool sfx_allocated(int32_t index);
void cont_sfx(int32_t index);
void stop_sfx(int32_t index);
void adjust_sfx(int32_t index,int32_t pan,bool loop);
void pause_sfx(int32_t index);
void resume_sfx(int32_t index);
void pause_all_sfx();
void resume_all_sfx();
void stop_sfx(int32_t index);
void stop_item_sfx(int32_t family);
void kill_sfx();
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

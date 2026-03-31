#ifndef ZALLEG_ZALLEG_H_
#define ZALLEG_ZALLEG_H_

#include "allegro/base.h"
#include "allegro/datafile.h"
#include "allegro/file.h"
#include "allegro/palette.h"
#include "base/zapp.h"
#include "zalleg/pal_tables.h"
#include "zsyssimple.h"
#include <allegro.h>
#include <allegro/internal/aintern.h>
#include <a5alleg.h>
#include <cstdint>
#include <string>
#include <utility>

#ifdef ALLEGRO_WINDOWS
#include <winalleg.h>
#endif

#if !defined(ALLEGRO_MACOSX)
#define KEY_ZC_LCONTROL KEY_LCONTROL
#define KEY_ZC_RCONTROL KEY_RCONTROL
#define CHECK_CTRL_CMD (key[KEY_LCONTROL] || key[KEY_RCONTROL])
#define KB_CTRL_CMD_FLAG KB_CTRL_FLAG
#else
#define KEY_ZC_LCONTROL KEY_COMMAND
#define KEY_ZC_RCONTROL KEY_COMMAND
#define CHECK_CTRL_CMD key[KEY_COMMAND]
#define KB_CTRL_CMD_FLAG KB_COMMAND_FLAG
#endif
#define CHECK_SHIFT (key[KEY_LSHIFT] || key[KEY_RSHIFT])
#define CHECK_ALT (key[KEY_ALT] || key[KEY_ALTGR])

// https://www.allegro.cc/forums/thread/613716
#ifdef ALLEGRO_LEGACY_MSVC
   #include <limits.h>
   #ifdef PATH_MAX
      #undef PATH_MAX
   #endif
   #define PATH_MAX MAX_PATH
#endif

struct DATAFILE;
enum App;

extern DATAFILE *sfxdata;
extern bool sound_was_installed;

void zalleg_setup_allegro(App id, int argc, char **argv);
void zalleg_create_window(const char* title, int gfx_mode, int v_width, int v_height, int saved_window_width, int saved_window_height, int max_scale = 3);
void zalleg_wait_for_all_keys_up();
std::pair<int, int> zalleg_get_default_display_size(int base_width, int base_height, int saved_width, int saved_height, int max_scale = 3);
void zalleg_process_display_events();
PACKFILE* zalleg_pack_fopen_password(const char *filename, const char *mode, const char *password);
uint64_t zalleg_file_size_ex_password(const char *filename, const char *password);
bool zalleg_alleg4_save_bitmap(BITMAP* bitmap, int scale, const char* filename, AL_CONST RGB *pal = nullptr);
int32_t zalleg_save_midi(const char *filename, MIDI *midi);
void zc_trace_clear();
void safe_al_trace(const char* str);
void safe_al_trace(std::string const& str);

#endif

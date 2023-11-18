#ifndef _ZC_ALLEG_H_
#define _ZC_ALLEG_H_

#include <allegro.h>
#include <allegro/internal/aintern.h>
#include <a5alleg.h>
#include "base/zfix.h"

#ifdef ALLEGRO_WINDOWS
#include <winalleg.h>
#endif

#if !defined(ALLEGRO_MACOSX)
#define KEY_ZC_LCONTROL KEY_LCONTROL
#define KEY_ZC_RCONTROL KEY_RCONTROL
#define CHECK_CTRL_CMD (key[KEY_LCONTROL] || key[KEY_RCONTROL])
#else
#define KEY_ZC_LCONTROL KEY_COMMAND
#define KEY_ZC_RCONTROL KEY_COMMAND
#define CHECK_CTRL_CMD key[KEY_COMMAND]
#endif
#define CHECK_SHIFT (key[KEY_LSHIFT] || key[KEY_RSHIFT])

// https://www.allegro.cc/forums/thread/613716
#ifdef ALLEGRO_LEGACY_MSVC
   #include <limits.h>
   #ifdef PATH_MAX
      #undef PATH_MAX
   #endif
   #define PATH_MAX MAX_PATH
#endif

PACKFILE *pack_fopen_password(const char *filename, const char *mode, const char *password);
uint64_t file_size_ex_password(const char *filename, const char *password);

bool alleg4_save_bitmap(BITMAP* bitmap, int scale, const char* filename, AL_CONST RGB *pal = nullptr);

#endif

//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zc_alleg.h
//
//--------------------------------------------------------

#ifndef _ZC_ALLEG_H_
#define _ZC_ALLEG_H_

#define DEBUGMODE

#include <allegro.h>
#include <allegro/internal/aintern.h>
#include <a5alleg.h>
#include "zfix.h"

#ifdef ALLEGRO_WINDOWS
#include <winalleg.h>
#endif

#if !defined(ALLEGRO_MACOSX)
#define KEY_ZC_LCONTROL KEY_LCONTROL
#define KEY_ZC_RCONTROL KEY_RCONTROL
#else
#define KEY_ZC_LCONTROL KEY_COMMAND
#define KEY_ZC_RCONTROL KEY_COMMAND
#endif

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

#endif

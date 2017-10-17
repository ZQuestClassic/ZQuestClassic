//--------------------------------------------------------
//  Zelda Classic
//  by Jeremy Craner, 1999-2000
//
//  zc_alleg.h
//
//  For debugging...
//
//--------------------------------------------------------

#ifndef _ZC_ALLEG_H_
#define _ZC_ALLEG_H_

#define DEBUGMODE
#define ALLEGRO_NO_COMPATIBILITY
#include <allegro.h>
#include <allegro/internal/aintern.h>

#ifdef _ZQUEST_SCALE_
#undef SCREEN_W
#undef SCREEN_H
#define SCREEN_W (screen ? screen->w : 0)
#define SCREEN_H (screen ? screen->h : 0)
#endif

#endif


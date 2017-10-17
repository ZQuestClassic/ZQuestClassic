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

  #ifdef __cplusplus
  INLINE fix abs(fix f) {
    fix t;
    if (f < 0)
    {
      t.v = -f.v;
    }
    else
    {
      t.v = f.v;
    }
    return t;
  }
  #endif

  #ifdef _ZQUEST_SCALE_
    #undef SCREEN_W
    #undef SCREEN_H
    #define SCREEN_W (screen ? screen->w : 0)
    #define SCREEN_H (screen ? screen->h : 0)
    #define popup_dialog popup_zqdialog
	#define do_dialog do_zqdialog
  #endif

  #ifdef ALLEGRO_WINDOWS
    #include <winalleg.h>
    #define YIELD() Sleep(10)
  #else
    #if defined(ALLEGRO_UNIX)||defined(ALLEGRO_LINUX)||defined(ALLEGRO_MACOSX)
      #include <sys/time.h>
      INLINE void YIELD(void)
      {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1;
        select(0, NULL, NULL, NULL, &tv);
      }
    #else
      #define YIELD() yield_timeslice()
    #endif
  #endif

  #if !defined(ALLEGRO_MACOSX)
    #define KEY_ZC_LCONTROL KEY_LCONTROL
    #define KEY_ZC_RCONTROL KEY_RCONTROL
  #else
    #define KEY_ZC_LCONTROL KEY_COMMAND
    #define KEY_ZC_RCONTROL KEY_COMMAND
  
  #endif

#endif

 

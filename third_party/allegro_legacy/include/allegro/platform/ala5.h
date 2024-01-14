/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      MacOS X specific header defines.
 *
 *      By Angelo Mottola.
 *
 *      See readme.txt for copyright information.
 */


#ifndef ALA5_H
#define ALA5_H


#ifndef SCAN_DEPEND
   #include <allegro5/allegro5.h>
   #include <allegro5/allegro_image.h>
   #include <allegro5/allegro_font.h>
   #include <allegro5/allegro_primitives.h>
   #include <allegro5/allegro_audio.h>
   #include <allegro5/allegro_acodec.h>
#endif

AL_LEGACY_VAR(ALLEGRO_DISPLAY *, _a5_display);
AL_LEGACY_VAR(bool, _a5_display_switched_out);

/* System driver */
#define SYSTEM_ALLEGRO_5        AL_ID('A','5',' ',' ')
AL_LEGACY_VAR(SYSTEM_DRIVER, system_allegro_5);

/* Timer driver */
#define TIMERDRV_ALLEGRO_5      AL_ID('A','5','T',' ')
AL_LEGACY_VAR(TIMER_DRIVER, timerdrv_allegro_5);

/* Keyboard driver */
#define KEYBOARD_ALLEGRO_5      AL_ID('A','5','K','B')
AL_LEGACY_VAR(KEYBOARD_DRIVER, keyboard_allegro_5);

/* Mouse driver */
#define MOUSE_ALLEGRO_5         AL_ID('A','5','M',' ')
AL_LEGACY_VAR(MOUSE_DRIVER, mouse_allegro_5);

/* Gfx drivers */
#define GFX_ALLEGRO_5           AL_ID('A','5','D',' ')
AL_LEGACY_VAR(GFX_DRIVER, gfx_allegro_5);

/* Digital sound drivers */
#define DIGI_ALLEGRO_5          AL_ID('A','5','A',' ')
AL_LEGACY_VAR(DIGI_DRIVER, digi_allegro_5);

/* MIDI music drivers */
#define MIDI_ALLEGRO_5         AL_ID('A','5','M','M')
//#define MIDI_QUICKTIME          AL_ID('Q','T','M',' ')
AL_LEGACY_VAR(MIDI_DRIVER, midi_allegro_5);
//AL_LEGACY_VAR(MIDI_DRIVER, midi_quicktime);

/* Joystick drivers */
#define JOYSTICK_ALLEGRO_5      AL_ID('A','5','J',' ')
AL_LEGACY_VAR(JOYSTICK_DRIVER, joystick_allegro_5);

#define GFX_DRIVER_ALLEGRO_5                                            \
   { GFX_ALLEGRO_5,            &display_allegro_5,            TRUE },	\

#define DIGI_DRIVER_ALLEGRO_5                                           \
   {  DIGI_ALLEGRO_5,  &digi_allegro_5,      TRUE  },

#define KEYBOARD_DRIVER_ALLEGRO_5                                           \
   {  KEYBOARD_ALLEGRO_5,  &keyboard_allegro_5,      TRUE  },

#define MIDI_DRIVER_ALLEGRO_5                                                \
   {  MIDI_ALLEGRO_5,       &midi_allegro_5,      TRUE  },

#define JOYSTICK_DRIVER_ALLEGRO_5                                       \
   {  JOYSTICK_ALLEGRO_5, &joystick_allegro_5,  TRUE  },

#endif

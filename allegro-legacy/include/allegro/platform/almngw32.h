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
 *      Configuration defines for use with Mingw32.
 *
 *      By Michael Rickmann.
 *
 *      Native build version by Henrik Stokseth.
 *
 *      See readme.txt for copyright information.
 */


#ifndef SCAN_DEPEND
   #include <io.h>
   #include <fcntl.h>
   #include <direct.h>
   #include <malloc.h>
#endif


/* a static auto config */
/* older mingw's don't seem to have inttypes.h */
/* #define ALLEGRO_LEGACY_HAVE_INTTYPES_H */
#define ALLEGRO_LEGACY_HAVE_STDINT_H	1


/* describe this platform */
#ifdef ALLEGRO_LEGACY_STATICLINK
   #define ALLEGRO_LEGACY_PLATFORM_STR  "MinGW32.s"
#else
   #define ALLEGRO_LEGACY_PLATFORM_STR  "MinGW32"
#endif

#define ALLEGRO_LEGACY_WINDOWS
#define ALLEGRO_LEGACY_I386
#define ALLEGRO_LEGACY_LITTLE_ENDIAN
#define ALLEGRO_LEGACY_USE_CONSTRUCTOR
#define ALLEGRO_LEGACY_MULTITHREADED

#ifdef ALLEGRO_LEGACY_USE_CONSOLE
   #define ALLEGRO_LEGACY_CONSOLE_OK
   #define ALLEGRO_LEGACY_NO_MAGIC_MAIN
#endif


/* describe how function prototypes look to MINGW32 */
#if (defined ALLEGRO_LEGACY_STATICLINK) || (defined ALLEGRO_LEGACY_SRC)
   #define _AL_LEGACY_DLL
#else
   #define _AL_LEGACY_DLL   __declspec(dllimport)
#endif

#define AL_LEGACY_VAR(type, name)                   extern _AL_LEGACY_DLL type name
#define AL_LEGACY_ARRAY(type, name)                 extern _AL_LEGACY_DLL type name[]
#define AL_LEGACY_FUNC(type, name, args)            extern type name args
#define AL_LEGACY_METHOD(type, name, args)          type (*name) args
#define AL_LEGACY_FUNCPTR(type, name, args)         extern _AL_LEGACY_DLL type (*name) args


/* windows specific defines */

#if (defined ALLEGRO_LEGACY_SRC)
/* pathches to handle DX7 headers on a win9x system */

/* should WINNT be defined on win9x systems? */
#ifdef WINNT
   #undef WINNT
#endif

/* defined in windef.h */
#ifndef HMONITOR_DECLARED
   #define HMONITOR_DECLARED 1
#endif

#endif /* ALLEGRO_LEGACY_SRC */

/* another instance of missing constants in the mingw32 headers */
#ifndef ENUM_CURRENT_SETTINGS
   #define ENUM_CURRENT_SETTINGS       ((DWORD)-1)
#endif

/* describe the asm syntax for this platform */
#define ALLEGRO_LEGACY_ASM_PREFIX    "_"

/* arrange for other headers to be included later on */
#define ALLEGRO_LEGACY_EXTRA_HEADER     "allegro/platform/alwin.h"
#define ALLEGRO_LEGACY_INTERNAL_HEADER  "allegro/platform/aintwin.h"
#define ALLEGRO_LEGACY_ASMCAPA_HEADER   "obj/mingw32/asmcapa.h"

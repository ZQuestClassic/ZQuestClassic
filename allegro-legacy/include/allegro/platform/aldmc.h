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
 *      Configuration defines for use with Digital Mars C compiler.
 *
 *      By Matthew Leverton.
 *
 *      See readme.txt for copyright information.
 */


#ifndef SCAN_DEPEND
   #include <io.h>
   #include <fcntl.h>
#endif

#include <stdint.h>


/* a static auto config */
#define ALLEGRO_LEGACY_HAVE_INTTYPES_H
#define ALLEGRO_LEGACY_HAVE_STDINT_H

#define LONG_LONG long long

/* describe this platform */
#ifdef ALLEGRO_LEGACY_STATICLINK
   #define ALLEGRO_LEGACY_PLATFORM_STR  "DMC.s"
#else
   #define ALLEGRO_LEGACY_PLATFORM_STR  "DMC"
#endif

#define ALLEGRO_LEGACY_WINDOWS
#define ALLEGRO_LEGACY_I386
#define ALLEGRO_LEGACY_LITTLE_ENDIAN
#define ALLEGRO_LEGACY_MULTITHREADED

#ifdef ALLEGRO_LEGACY_USE_CONSOLE
   #define ALLEGRO_LEGACY_CONSOLE_OK
   #define ALLEGRO_LEGACY_NO_MAGIC_MAIN
#endif


/* describe how function prototypes look to DMC */
#if defined ALLEGRO_LEGACY_STATICLINK
   #define _AL_LEGACY_DLL
#elif defined ALLEGRO_LEGACY_SRC
   #define _AL_LEGACY_DLL   __declspec(dllexport)
#else
   #define _AL_LEGACY_DLL   __declspec(dllimport)
#endif

#define AL_LEGACY_VAR(type, name)                   extern _AL_LEGACY_DLL type name
#define AL_LEGACY_ARRAY(type, name)                 extern _AL_LEGACY_DLL type name[]
#define AL_LEGACY_FUNC(type, name, args)            extern type name args
#define AL_LEGACY_METHOD(type, name, args)          type (*name) args
#define AL_LEGACY_FUNCPTR(type, name, args)         extern _AL_LEGACY_DLL type (*name) args


/* Windows specific defines */

#if (defined ALLEGRO_LEGACY_SRC)

#if (!defined S_IRUSR) && (!defined SCAN_DEPEND)
   #define S_IRUSR   S_IREAD
   #define S_IWUSR   S_IWRITE
#endif

typedef unsigned long   _fsize_t;

struct _wfinddata_t {
   unsigned attrib;
   time_t   time_create;         /* -1 for FAT file systems */
   time_t   time_access;         /* -1 for FAT file systems */
   time_t   time_write;
   _fsize_t size;
   wchar_t  name[260];           /* may include spaces. */
};

#endif /* ALLEGRO_LEGACY_SRC */

/* describe the asm syntax for this platform */
#define ALLEGRO_LEGACY_ASM_PREFIX    "_"

/* arrange for other headers to be included later on */
#define ALLEGRO_LEGACY_EXTRA_HEADER     "allegro/platform/alwin.h"
#define ALLEGRO_LEGACY_INTERNAL_HEADER  "allegro/platform/aintwin.h"
#define ALLEGRO_LEGACY_ASMCAPA_HEADER   "obj/dmc/asmcapa.h"

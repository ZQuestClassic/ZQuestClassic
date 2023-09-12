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
 *      Configuration defines for use with Borland C++Builder.
 *
 *      By Greg Hackmann.
 *
 *      See readme.txt for copyright information.
 */


#ifdef ALLEGRO_LEGACY_SRC
   #error Currently BCC32 cannot build the library
#endif

#ifndef SCAN_DEPEND
   #include <io.h>
   #include <fcntl.h>
   #include <direct.h>
   #include <malloc.h>
#endif


#pragma warn -8004  /* unused assigned value         */
#pragma warn -8008  /* condition always met          */
#pragma warn -8057  /* unused parameter              */
#pragma warn -8066  /* unreachable code              */


/* describe this platform */
#define ALLEGRO_LEGACY_PLATFORM_STR  "BCC32"
#define ALLEGRO_LEGACY_WINDOWS
#define ALLEGRO_LEGACY_I386
#define ALLEGRO_LEGACY_LITTLE_ENDIAN
#define ALLEGRO_LEGACY_GUESS_INTTYPES_OK
   /* TODO: check if BCC has inttypes.h and/or stdint.h */
#define ALLEGRO_LEGACY_MULTITHREADED

#ifdef ALLEGRO_LEGACY_USE_CONSOLE
   #define ALLEGRO_LEGACY_CONSOLE_OK
   #define ALLEGRO_LEGACY_NO_MAGIC_MAIN
#endif


/* describe how function prototypes look to BCC32 */
#if defined ALLEGRO_LEGACY_STATICLINK
   #define _AL_LEGACY_DLL
#elif defined ALLEGRO_LEGACY_SRC
   #define _AL_LEGACY_DLL   __declspec(dllexport)
#else
   #define _AL_LEGACY_DLL   __declspec(dllimport)
#endif

#define AL_LEGACY_VAR(type, name)             extern _AL_LEGACY_DLL type name
#define AL_LEGACY_ARRAY(type, name)           extern _AL_LEGACY_DLL type name[]
#define AL_LEGACY_FUNC(type, name, args)      _AL_LEGACY_DLL type __cdecl name args
#define AL_LEGACY_METHOD(type, name, args)    type (__cdecl *name) args
#define AL_LEGACY_FUNCPTR(type, name, args)   extern _AL_LEGACY_DLL type (__cdecl *name) args


#define END_OF_INLINE(name)
#define AL_LEGACY_INLINE(type, name, args, code)    extern __inline type __cdecl name args code END_OF_INLINE(name)

#define INLINE       __inline

#define LONG_LONG    __int64
#define int64_t      signed __int64
#define uint64_t     unsigned __int64

#define AL_CONST     const

/* windows specific defines */
#ifdef NONAMELESSUNION
   #undef NONAMELESSUNION
#endif
/* This fixes 99.999999% of Borland C++Builder's problems with structs. */

/* arrange for other headers to be included later on */
#define ALLEGRO_LEGACY_EXTRA_HEADER     "allegro/platform/alwin.h"
#define ALLEGRO_LEGACY_INTERNAL_HEADER  "allegro/platform/aintwin.h"

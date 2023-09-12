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
 *      Configuration defines for use with PSP.
 *
 *      By diedel.
 *
 *      See readme.txt for copyright information.
 */


#ifndef ALPSPCFG_H
#define ALPSPCFG_H

#ifndef SCAN_DEPEND
   #include <fcntl.h>
   #include <unistd.h>
#endif


#ifndef ALLEGRO_LEGACY_NO_MAGIC_MAIN
   #define ALLEGRO_LEGACY_MAGIC_MAIN
   #define main _mangled_main
   #undef END_OF_MAIN
   #define END_OF_MAIN() void *_mangled_main_address = (void *) _mangled_main;
#else
   #undef END_OF_MAIN
   #define END_OF_MAIN() void *_mangled_main_address;
#endif


/* Provide implementations of missing definitions */
#define dup(X)	(fcntl(X, F_DUPFD, 0))


/* TODO: Use the configure script. */
/* A static auto config */
//#define ALLEGRO_LEGACY_HAVE_LIBPTHREAD 1
#define ALLEGRO_LEGACY_HAVE_DIRENT_H   1
#define ALLEGRO_LEGACY_HAVE_INTTYPES_H 1
#define ALLEGRO_LEGACY_HAVE_STDINT_H   1
#define ALLEGRO_LEGACY_HAVE_SYS_TIME_H 1
#define ALLEGRO_LEGACY_HAVE_SYS_STAT_H 1

/* Describe this platform */
#define ALLEGRO_LEGACY_PLATFORM_STR  "PlayStation Portable"
#define ALLEGRO_LEGACY_USE_CONSTRUCTOR

#define ALLEGRO_LEGACY_LITTLE_ENDIAN

/* Exclude ASM */
#ifndef ALLEGRO_LEGACY_NO_ASM
   #define ALLEGRO_LEGACY_NO_ASM
#endif

/* Arrange for other headers to be included later on */
#define ALLEGRO_LEGACY_EXTRA_HEADER     "allegro/platform/alpsp.h"


#endif

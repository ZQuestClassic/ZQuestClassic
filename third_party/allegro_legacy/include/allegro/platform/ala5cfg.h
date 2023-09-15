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
 *      Configuration defines for use with BeOS.
 *
 *      By Jason Wilkins.
 *
 *      See readme.txt for copyright information.
 */


//#include <fcntl.h>
//#include <unistd.h>

/* provide implementations of missing functions */
/*#define ALLEGRO_LEGACY_NO_STRICMP
#define ALLEGRO_LEGACY_NO_STRLWR
#define ALLEGRO_LEGACY_NO_STRUPR */

/* a static auto config */
//#define ALLEGRO_LEGACY_HAVE_DIRENT_H   1
//#define ALLEGRO_LEGACY_HAVE_INTTYPES_H 1       /* TODO: check this */
//#define ALLEGRO_LEGACY_HAVE_STDINT_H   1       /* TODO: check this */
//#define ALLEGRO_LEGACY_HAVE_SYS_TIME_H 1

/* describe this platform */
#define ALLEGRO_LEGACY_PLATFORM_STR "Allegro 5"
/*#define ALLEGRO_LEGACY_LITTLE_ENDIAN
#define ALLEGRO_LEGACY_CONSOLE_OK
#define ALLEGRO_LEGACY_USE_CONSTRUCTOR
#define ALLEGRO_LEGACY_MULTITHREADED */

/* arrange for other headers to be included later on */
#define ALLEGRO_LEGACY_EXTRA_HEADER     "allegro/platform/ala5.h"
#define ALLEGRO_LEGACY_INTERNAL_HEADER  "allegro/platform/ainta5.h"
//#define ALLEGRO_LEGACY_ASMCAPA_HEADER   "obj/beos/asmcapa.h"

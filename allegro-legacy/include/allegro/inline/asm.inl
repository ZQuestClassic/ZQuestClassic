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
 *      Imports asm definitions of various inline functions.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */

#ifdef __cplusplus
   extern "C" {
#endif


#ifndef ALLEGRO_LEGACY_NO_ASM

#if (defined ALLEGRO_LEGACY_GCC) && (defined ALLEGRO_LEGACY_I386)

   /* use i386 asm, GCC syntax */
   #include "allegro/platform/al386gcc.h"

#elif (defined ALLEGRO_LEGACY_MSVC) && (defined ALLEGRO_LEGACY_I386)

   /* use i386 asm, MSVC syntax */
   #include "allegro/platform/al386vc.h"

#elif (defined ALLEGRO_LEGACY_WATCOM) && (defined ALLEGRO_LEGACY_I386)

   /* use i386 asm, Watcom syntax */
   #include "allegro/platform/al386wat.h"

#else

   /* asm not supported */
   #define ALLEGRO_LEGACY_NO_ASM

#endif

#endif

/* Define ALLEGRO_LEGACY_USE_C for backwards compatibility. It should not be used
 * anywhere else in the sources for now.
 */
#ifdef ALLEGRO_LEGACY_NO_ASM
#define ALLEGRO_LEGACY_USE_C
#endif

#ifdef __cplusplus
   }
#endif


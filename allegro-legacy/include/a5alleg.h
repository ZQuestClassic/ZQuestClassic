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
 *      The Be Allegro Header File.
 *
 *      Aren't you glad you used the BeOS header file?
 *      Don't you wish everybody did?
 *
 *      By Jason Wilkins.
 *
 *      See readme.txt for copyright information.
 */

#ifndef A5_ALLEGRO_LEGACY_H
#define A5_ALLEGRO_LEGACY_H

AL_LEGACY_FUNC(ALLEGRO_DISPLAY *, all_get_display, (void));
AL_LEGACY_FUNC(ALLEGRO_BITMAP *, all_get_a5_bitmap, (BITMAP * bp));
AL_LEGACY_FUNC(void, all_render_a5_bitmap, (BITMAP * bp, ALLEGRO_BITMAP * a5bp));
AL_LEGACY_FUNC(void, all_render_screen, (void));
AL_LEGACY_FUNC(void, all_disable_threaded_display, (void));

#endif /* ifndef A5_ALLEGRO_LEGACY_H */

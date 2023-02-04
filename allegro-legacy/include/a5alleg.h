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

#ifdef __cplusplus
extern "C" {
#endif

AL_LEGACY_FUNC(ALLEGRO_DISPLAY *, all_get_display, (void));
AL_LEGACY_FUNC(ALLEGRO_BITMAP *, all_get_a5_bitmap, (BITMAP * bp));
AL_LEGACY_FUNC(void, all_render_a5_bitmap, (BITMAP * bp, ALLEGRO_BITMAP * a5bp));
AL_LEGACY_FUNC(void, all_render_screen, (void));
AL_LEGACY_FUNC(void, all_disable_threaded_display, (void));
// local edit
AL_LEGACY_FUNC(void, all_process_display_events, ());
AL_LEGACY_FUNC(void, all_set_scale, (int scale));
AL_LEGACY_FUNC(void, all_set_force_integer_scale, (bool force));
AL_LEGACY_FUNC(int, all_get_scale, (void));
AL_LEGACY_FUNC(void, all_get_display_transform, (int* out_native_width, int* out_native_height, int* out_display_width, int* out_display_height, int* out_offset_x, int* out_offset_y, double* out_scale));
AL_LEGACY_FUNC(void, all_set_fullscreen_flag, (bool fullscreen));
AL_LEGACY_FUNC(bool, all_get_fullscreen_flag, (void));
AL_LEGACY_FUNC(void, all_set_display_flags, (int flags));
AL_LEGACY_FUNC(int, all_get_display_flags, (void));
AL_LEGACY_FUNC(void, all_set_bitmap_flags, (int flags));
AL_LEGACY_FUNC(int, all_get_bitmap_flags, (void));

// local edit
AL_LEGACY_FUNC(void, all_lock_screen, (void));
AL_LEGACY_FUNC(void, all_unlock_screen, (void));
AL_LEGACY_FUNC(void, all_set_transparent_palette_index, (int));
AL_LEGACY_FUNC(void, all_keyboard_queue_register_event_source, (ALLEGRO_EVENT_SOURCE *));

#ifdef __cplusplus
}
#endif

#endif /* ifndef A5_ALLEGRO_LEGACY_H */

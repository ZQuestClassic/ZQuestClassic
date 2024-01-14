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
 *      Stuff for BeOS.
 *
 *      By Jason Wilkins.
 *
 *      See readme.txt for copyright information.
 */

#include <stdio.h>
#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/ainta5.h"
#include "allegro/platform/ala5.h"
#include "a5alleg.h"

static int _a5_display_switch_mode = SWITCH_PAUSE;

ALLEGRO_DISPLAY * _a5_display = NULL;
bool _a5_display_switched_out = false;
void (*_a5_close_button_proc)(void) = NULL;

static int a5_sys_init(void)
{
    int ret = al_init();
    return !ret;
}

static void a5_sys_exit(void)
{
    al_uninstall_system();
}

static void a5_sys_get_executable_name(char *output, int size)
{
    ALLEGRO_PATH * path;
    bool already_initialized = al_is_system_installed();
    char * mod_output = NULL;

    if(!already_initialized)
    {
        al_init();
    }
    path = al_get_standard_path(ALLEGRO_EXENAME_PATH);
    if(path)
    {
        strcpy(output, al_path_cstr(path, '/'));
        al_destroy_path(path);
        mod_output = ustrstr(output, ".app/");
        if(mod_output)
        {
            strcpy(mod_output, ".app");
        }
        if(!already_initialized)
        {
            al_uninstall_system();
        }
    }
    else
    {
        // local edit
        output[0] = '\0';
    }
}

static void a5_sys_set_window_title(AL_CONST char * name)
{
    if(_a5_display)
    {
        al_set_window_title(_a5_display, name);
    }
}

static int a5_sys_set_close_button_callback(void (*proc)(void))
{
   _a5_close_button_proc = proc;

   return 0;
}

static void a5_sys_message(AL_CONST char * msg)
{
    printf("%s\n", msg);
}

static int a5_sys_set_display_switch_mode(int mode)
{
    _a5_display_switch_mode = mode;
    return 0;
}

static int a5_sys_desktop_color_depth(void)
{
    return 32;
}

static int a5_sys_get_desktop_resolution(int *width, int *height)
{
    ALLEGRO_MONITOR_INFO info;

    al_get_monitor_info(0, &info);
    if(width)
    {
        *width = info.x2 - info.x1;
    }
    if(height)
    {
        *height = info.y2 - info.y1;
    }
    return 0;
}

static void a5_sys_get_gfx_safe_mode(int *driver, struct GFX_MODE *mode)
{
   *driver = GFX_ALLEGRO_5;
   mode->width = 640;
   mode->height = 480;
   mode->bpp = 8;
}

static void a5_sys_yield_timeslice(void)
{
    al_rest(0.001);
}

static void * a5_sys_create_mutex(void)
{
    return al_create_mutex_recursive();
}

static void a5_sys_destroy_mutex(void * handle)
{
    al_destroy_mutex(handle);
}

static void a5_sys_lock_mutex(void *handle)
{
    al_lock_mutex(handle);
}

static void a5_sys_unlock_mutex(void *handle)
{
    al_unlock_mutex(handle);
}

SYSTEM_DRIVER system_allegro_5 = {
   SYSTEM_ALLEGRO_5,
   empty_string,
   empty_string,
   "Allegro 5 System",
   a5_sys_init,
   a5_sys_exit,
   a5_sys_get_executable_name,
   NULL, //be_sys_find_resource,
   a5_sys_set_window_title,
   a5_sys_set_close_button_callback,
   a5_sys_message,
   NULL,  // AL_LEGACY_METHOD(void, assert, (char *msg));
   NULL,  // AL_LEGACY_METHOD(void, save_console_state, (void));
   NULL,  // AL_LEGACY_METHOD(void, restore_console_state, (void));
   NULL,  // AL_LEGACY_METHOD(struct BITMAP *, create_bitmap, (int color_depth, int width, int height));
   NULL,  // AL_LEGACY_METHOD(void, created_bitmap, (struct BITMAP *bmp));
   NULL,  // AL_LEGACY_METHOD(struct BITMAP *, create_sub_bitmap, (struct BITMAP *parent, int x, int y, int width, int height));
   NULL,  // AL_LEGACY_METHOD(void, created_sub_bitmap, (struct BITMAP *bmp, struct BITMAP *parent));
   NULL,  // AL_LEGACY_METHOD(int, destroy_bitmap, (struct BITMAP *bitmap));
   NULL,  // AL_LEGACY_METHOD(void, read_hardware_palette, (void));
   NULL,  // AL_LEGACY_METHOD(void, set_palette_range, (struct RGB *p, int from, int to, int vsync));
   NULL,  // AL_LEGACY_METHOD(struct GFX_VTABLE *, get_vtable, (int color_depth));
   a5_sys_set_display_switch_mode,
   NULL,  // AL_LEGACY_METHOD(void, display_switch_lock, (int lock));
   a5_sys_desktop_color_depth,
   a5_sys_get_desktop_resolution,
   a5_sys_get_gfx_safe_mode,
   a5_sys_yield_timeslice,
   a5_sys_create_mutex,
   a5_sys_destroy_mutex,
   a5_sys_lock_mutex,
   a5_sys_unlock_mutex,
   NULL,  // AL_LEGACY_METHOD(_DRIVER_INFO *, gfx_drivers, (void));
   NULL,  // AL_LEGACY_METHOD(_DRIVER_INFO *, digi_drivers, (void));
   NULL,  // AL_LEGACY_METHOD(_DRIVER_INFO *, midi_drivers, (void));
   NULL,  // AL_LEGACY_METHOD(_DRIVER_INFO *, keyboard_drivers, (void));
   NULL,  // AL_LEGACY_METHOD(_DRIVER_INFO *, mouse_drivers, (void));
   NULL,  // AL_LEGACY_METHOD(_DRIVER_INFO *, joystick_drivers, (void));
   NULL   // AL_LEGACY_METHOD(_DRIVER_INFO *, timer_drivers, (void));
};

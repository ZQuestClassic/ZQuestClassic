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

#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/ainta5.h"
#include "allegro/platform/ala5.h"
// local edit
#include "a5alleg.h"

static ALLEGRO_THREAD * a5_mouse_thread = NULL;
static int a5_last_mouse_x = -1;
static int a5_last_mouse_y = -1;
static int prevx = -1;
static int prevy = -1;
static int prevz = -1;
static bool mouse_hidden = false;
static bool have_touch_input = false;

static void * a5_mouse_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
    ALLEGRO_EVENT_QUEUE * queue;
    ALLEGRO_EVENT event;
    ALLEGRO_TIMEOUT timeout;

    queue = al_create_event_queue();
    if(!queue)
    {
        return NULL;
    }
    al_register_event_source(queue, al_get_mouse_event_source());
    if (have_touch_input)
        al_register_event_source(queue, al_get_touch_input_event_source());
    while(!al_get_thread_should_stop(thread))
    {
#ifdef ALLEGRO_LEGACY_CLOSE_THREADS
        al_init_timeout(&timeout, 0.1);
        if (al_wait_for_event_until(queue, &event, &timeout))
#else
        al_wait_for_event(queue, &event);
#endif
        {
            switch(event.type)
            {
                // local edit
                case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
                {
                    _mouse_on = -1;
                }
                case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
                {
                    _mouse_on = 0;
                    break;
                }
                case ALLEGRO_EVENT_MOUSE_AXES:
                {
                    _mouse_x = event.mouse.x;
                    _mouse_y = event.mouse.y;
                    _mouse_z = event.mouse.z;

                    if (prevx != _mouse_x || prevy != _mouse_y || prevz != _mouse_z) {
                        prevx = event.mouse.x;
                        prevy = event.mouse.y;
                        prevz = event.mouse.z;
                    }
                    break;
                }
                case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                {
                    _mouse_b |= 1 << (event.mouse.button - 1);
                    break;
                }
                case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
                {
                    _mouse_b &= ~(1 << (event.mouse.button - 1));
                    break;
                }

                // local edit
                // emulate mouse click on touch.
                case ALLEGRO_EVENT_TOUCH_BEGIN:
                case ALLEGRO_EVENT_TOUCH_MOVE:
                case ALLEGRO_EVENT_TOUCH_END:
                {
                    if (event.touch.primary)
                    {
                        _mouse_x = event.touch.x;
                        _mouse_y = event.touch.y;
                        if (event.type == ALLEGRO_EVENT_TOUCH_BEGIN)
                        {
                            _mouse_b |= 1;
                        }
                        else if (event.type == ALLEGRO_EVENT_TOUCH_END)
                        {
                            _mouse_b &= ~1;
                        }
                    }
                    break;
                }
            }
            _handle_mouse_input();
        }
    }
    al_destroy_event_queue(queue);
    return NULL;
}

static int a5_mouse_init(void)
{
    if(!al_install_mouse())
    {
        return -1;
    }
    have_touch_input = al_install_touch_input();

    // if(_a5_display)
    // {
    //     al_hide_mouse_cursor(_a5_display);
    // }
    a5_mouse_thread = al_create_thread(a5_mouse_thread_proc, NULL);
    al_start_thread(a5_mouse_thread);
    return 0;
}

static void a5_mouse_exit(void)
{
#ifdef ALLEGRO_LEGACY_CLOSE_THREADS
    al_destroy_thread(a5_mouse_thread);
    a5_mouse_thread = NULL;
#endif
    al_uninstall_mouse();
}

static void a5_mouse_position(int x, int y)
{
    // al_set_mouse_xy(_a5_display, x, y);
}

static void a5_mouse_get_mickeys(int * x, int * y)
{
    if(x)
    {
        if(a5_last_mouse_x < 0)
        {
            a5_last_mouse_x = _mouse_x;
        }
        *x = _mouse_x - a5_last_mouse_x;
        a5_last_mouse_x = _mouse_x;
    }
    if(y)
    {
        if(a5_last_mouse_y < 0)
        {
            a5_last_mouse_y = _mouse_y;
        }
        *y = _mouse_y - a5_last_mouse_y;
        a5_last_mouse_y = _mouse_y;
    }
}

static void a5_mouse_enable_hardware_cursor(int mode)
{
    static bool is_enabled = true;
    if (mode != is_enabled && _a5_display)
    {
        if (mode)
            al_show_mouse_cursor(_a5_display);
        else
            al_hide_mouse_cursor(_a5_display);
        is_enabled = mode;
    }
}

static int a5_mouse_select_system_cursor(const int cursor)
{
    if(!_a5_display)
    {
        return 1;
    }
    switch(cursor)
    {
        case MOUSE_CURSOR_NONE:
        {
            a5_mouse_enable_hardware_cursor(0);
            mouse_hidden = true;
            break;
        }
        case MOUSE_CURSOR_ALLEGRO:
        case MOUSE_CURSOR_ARROW:
        {
          al_show_mouse_cursor(_a5_display);
            al_set_system_mouse_cursor(_a5_display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
            mouse_hidden = false;
            break;
        }
        case MOUSE_CURSOR_BUSY:
        {
          al_show_mouse_cursor(_a5_display);
            al_set_system_mouse_cursor(_a5_display, ALLEGRO_SYSTEM_MOUSE_CURSOR_BUSY);
            mouse_hidden = false;
            break;
        }
        case MOUSE_CURSOR_EDIT:
        {
          al_show_mouse_cursor(_a5_display);
          al_set_system_mouse_cursor(_a5_display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
            al_set_system_mouse_cursor(_a5_display, ALLEGRO_SYSTEM_MOUSE_CURSOR_EDIT);
            mouse_hidden = false;
            break;
        }
        case MOUSE_CURSOR_QUESTION:
        {
            al_show_mouse_cursor(_a5_display);
            al_set_system_mouse_cursor(_a5_display, ALLEGRO_SYSTEM_MOUSE_CURSOR_QUESTION);
            mouse_hidden = false;
            break;
        }
    }
    return 1;
}

MOUSE_DRIVER mouse_allegro_5 = {
   MOUSE_ALLEGRO_5,		// int id;
   empty_string,	// char *name;
   empty_string,	// char *desc;
   "Allegro 5 Mouse",		// char *ascii_name;
   a5_mouse_init, //be_mouse_init,	// AL_LEGACY_METHOD(int, init, (void));
   a5_mouse_exit, //be_mouse_exit,	// AL_LEGACY_METHOD(void, exit, (void));
   NULL,		// AL_LEGACY_METHOD(void, poll, (void));
   NULL,		// AL_LEGACY_METHOD(void, timer_poll, (void));
   a5_mouse_position, //be_mouse_position,	// AL_LEGACY_METHOD(void, position, (int x, int y));
   NULL, //be_mouse_set_range,	// AL_LEGACY_METHOD(void, set_range, (int x1, int y1, int x2, int y2));
   NULL, //be_mouse_set_speed,	// AL_LEGACY_METHOD(void, set_speed, (int xspeed, int yspeed));
   a5_mouse_get_mickeys, //be_mouse_get_mickeys,// AL_LEGACY_METHOD(void, get_mickeys, (int *mickeyx, int *mickeyy));
   NULL,                // AL_LEGACY_METHOD(int,  analyse_data, (const char *buffer, int size));
   a5_mouse_enable_hardware_cursor,                // AL_LEGACY_METHOD(void,  enable_hardware_cursor, (AL_CONST int mode));
   a5_mouse_select_system_cursor                 // AL_LEGACY_METHOD(int,  select_system_cursor, (AL_CONST int cursor));
};

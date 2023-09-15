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

#define _A5_KEYBOARD_BUFFER_SIZE 256

static ALLEGRO_THREAD * a5_keyboard_thread = NULL;
static ALLEGRO_EVENT_QUEUE * queue = NULL;
static int a5_keyboard_keycode_map[256];

// local edit
static void update_key_shifts(ALLEGRO_EVENT* event) {
    _key_shifts = 0;
    if ((ALLEGRO_KEYMOD_SHIFT & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_SHIFT_FLAG;
    }
    if ((ALLEGRO_KEYMOD_CTRL & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_CTRL_FLAG;
    }
    if ((ALLEGRO_KEYMOD_ALT & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_ALT_FLAG;
    }
    if ((ALLEGRO_KEYMOD_LWIN & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_LWIN_FLAG;
    }
    if ((ALLEGRO_KEYMOD_RWIN & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_RWIN_FLAG;
    }
    if ((ALLEGRO_KEYMOD_MENU & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_MENU_FLAG;
    }
    if ((ALLEGRO_KEYMOD_COMMAND & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_COMMAND_FLAG;
    }
    if ((ALLEGRO_KEYMOD_SCROLLLOCK & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_SCROLOCK_FLAG;
    }
    if ((ALLEGRO_KEYMOD_NUMLOCK & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_NUMLOCK_FLAG;
    }
    if ((ALLEGRO_KEYMOD_CAPSLOCK & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_CAPSLOCK_FLAG;
    }
    if ((ALLEGRO_KEYMOD_INALTSEQ & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_INALTSEQ_FLAG;
    }
    if ((ALLEGRO_KEYMOD_ACCENT1 & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_ACCENT1_FLAG;
    }
    if ((ALLEGRO_KEYMOD_ACCENT2 & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_ACCENT2_FLAG;
    }
    if ((ALLEGRO_KEYMOD_ACCENT3 & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_ACCENT3_FLAG;
    }
    if ((ALLEGRO_KEYMOD_ACCENT4 & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_ACCENT4_FLAG;
    }
}


static void * a5_keyboard_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
    ALLEGRO_EVENT event;
    ALLEGRO_TIMEOUT timeout;

    al_register_event_source(queue, al_get_keyboard_event_source());
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
                case ALLEGRO_EVENT_KEY_DOWN:
                {
                    update_key_shifts(&event);
                    if(event.keyboard.keycode >= ALLEGRO_KEY_MODIFIERS)
                    {
                        _handle_key_press(0, a5_keyboard_keycode_map[event.keyboard.keycode]);
                    }
                    break;
                }
                case ALLEGRO_EVENT_KEY_UP:
                {
                    update_key_shifts(&event);
                    _handle_key_release(a5_keyboard_keycode_map[event.keyboard.keycode]);
                    break;
                }
                case ALLEGRO_EVENT_KEY_CHAR:
                {
                    update_key_shifts(&event);
                    if(event.keyboard.unichar >= 0)
                    {
                        // local edit
                        if ((ALLEGRO_KEYMOD_ALT & event.keyboard.modifiers) != 0) {
                            _handle_key_press(0, event.keyboard.keycode);
                        } else {
                            _handle_key_press(event.keyboard.unichar, event.keyboard.keycode);
                        }
                    }
                    break;
                }
            }
        }
    }
    al_destroy_event_queue(queue);
    return NULL;
}

#include <stdio.h>

static int a5_keyboard_init(void)
{
    int i;

    if(!al_install_keyboard())
    {
        return -1;
    }

    /* set up key code map */
    for(i = 0; i <= ALLEGRO_KEY_COMMAND; i++)
    {
        a5_keyboard_keycode_map[i] = i;
    }
    a5_keyboard_keycode_map[ALLEGRO_KEY_LSHIFT] = KEY_LSHIFT;
    a5_keyboard_keycode_map[ALLEGRO_KEY_RSHIFT] = KEY_RSHIFT;
    a5_keyboard_keycode_map[ALLEGRO_KEY_LCTRL] = KEY_LCONTROL;
    a5_keyboard_keycode_map[ALLEGRO_KEY_RCTRL] = KEY_RCONTROL;
    a5_keyboard_keycode_map[ALLEGRO_KEY_ALT] = KEY_ALT;
    a5_keyboard_keycode_map[ALLEGRO_KEY_ALTGR] = KEY_ALTGR;
    a5_keyboard_keycode_map[ALLEGRO_KEY_LWIN] = KEY_LWIN;
    a5_keyboard_keycode_map[ALLEGRO_KEY_RWIN] = KEY_RWIN;
    a5_keyboard_keycode_map[ALLEGRO_KEY_MENU] = KEY_MENU;
    a5_keyboard_keycode_map[ALLEGRO_KEY_SCROLLLOCK] = KEY_SCRLOCK;
    a5_keyboard_keycode_map[ALLEGRO_KEY_NUMLOCK] = KEY_NUMLOCK;
    a5_keyboard_keycode_map[ALLEGRO_KEY_CAPSLOCK] = KEY_CAPSLOCK;

    queue = al_create_event_queue();
    if(!queue)
    {
        return -1;
    }

    a5_keyboard_thread = al_create_thread(a5_keyboard_thread_proc, NULL);
    al_start_thread(a5_keyboard_thread);

    return 0;
}

static void a5_keyboard_exit(void)
{
#ifdef ALLEGRO_LEGACY_CLOSE_THREADS
    al_destroy_thread(a5_keyboard_thread);
    a5_keyboard_thread = NULL;
#endif

    al_uninstall_keyboard();
}

static void a5_keyboard_set_leds(int flags)
{
    int a5_flags = 0;

    if(flags & KB_SCROLOCK_FLAG)
    {
        a5_flags |= ALLEGRO_KEYMOD_SCROLLLOCK;
    }
    if(flags & KB_CAPSLOCK_FLAG)
    {
        a5_flags |= ALLEGRO_KEYMOD_CAPSLOCK;
    }
    if(flags & KB_NUMLOCK_FLAG)
    {
        a5_flags |= ALLEGRO_KEYMOD_NUMLOCK;
    }
    if(flags < 0)
    {
        al_set_keyboard_leds(-1);
    }
    else
    {
        al_set_keyboard_leds(a5_flags);
    }
}

// local edit
void all_keyboard_queue_register_event_source(ALLEGRO_EVENT_SOURCE * evtsrc)
{
    al_register_event_source(queue, evtsrc);
}

KEYBOARD_DRIVER keyboard_allegro_5 = {
   KEYBOARD_ALLEGRO_5,
   empty_string,
   empty_string,
   "Allegro 5 Keyboard",
   TRUE,
   a5_keyboard_init, //be_key_init,
   a5_keyboard_exit, //be_key_exit,
   NULL,
   a5_keyboard_set_leds, //be_key_set_leds,
   NULL, //be_key_set_rate,
   NULL, //be_key_wait_for_input,
   NULL, //be_key_stop_waiting_for_input,
   NULL,
   NULL
};

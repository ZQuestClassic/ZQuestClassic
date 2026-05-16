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
#include "allegro/keyboard.h"
#include "allegro/platform/ainta5.h"
#include "allegro/platform/ala5.h"

#define _A5_KEYBOARD_BUFFER_SIZE 256

static ALLEGRO_THREAD * a5_keyboard_thread = NULL;
static ALLEGRO_EVENT_QUEUE * queue = NULL;
static int a5_keyboard_keycode_map[256];

// local edit
static void update_key_shifts(ALLEGRO_EVENT* event) {
    _key_shifts = 0;

    // local edit - On Windows, AltGr is reported as Ctrl+Alt. If we propagate
    // KB_CTRL_FLAG in that case, MSG_CHAR handlers like jwin_edit_proc will
    // suppress character input (treating it as a Ctrl shortcut) and break
    // typing characters that require AltGr (e.g. '$' on Swedish QWERTY).
    bool is_ctrl_mod = (event->keyboard.modifiers & ALLEGRO_KEYMOD_CTRL) != 0;
    bool is_alt_mod = (event->keyboard.modifiers & ALLEGRO_KEYMOD_ALT) != 0;
    bool is_altgr_mod = (event->keyboard.modifiers & ALLEGRO_KEYMOD_ALTGR) != 0;
    bool is_altgr = is_altgr_mod || (is_ctrl_mod && is_alt_mod);

    if ((ALLEGRO_KEYMOD_SHIFT & event->keyboard.modifiers) != 0) {
        _key_shifts |= KB_SHIFT_FLAG;
    }
    if (is_ctrl_mod && !is_altgr) {
        _key_shifts |= KB_CTRL_FLAG;
    }
    if (is_alt_mod || is_altgr_mod) {
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
            // This helps for understanding how to make key press events for allegro 4:
            // https://github.com/liballeg/allegro4/blob/master/src/win/wkeybd.c#L292
            //
            // event.keyboard.keycode (Allegro 5) -> KEY_X (Allegro 4 Scancode)
            //    This represents the PHYSICAL KEY index.
            //    Ex: ALLEGRO_KEY_Z (26) maps to KEY_Z. 
            //    Used for checking key[] array, moving game characters (WASD), etc.
            //
            // event.keyboard.unichar
            //    This represents the LOGICAL CHARACTER (ASCII/Unicode).
            //    Ex: 'z' (122) or 'Z' (90).
            //    Used for typing names, chat boxes, etc.
            switch(event.type)
            {
                case ALLEGRO_EVENT_KEY_DOWN:
                {
                    // local edit - prevent the zscript debugger window from triggering keyboard events.
                    if (event.keyboard.display != all_get_display()) break;

                    update_key_shifts(&event);
                    _handle_key_press(-1, a5_keyboard_keycode_map[event.keyboard.keycode]);
                    break;
                }
                case ALLEGRO_EVENT_KEY_UP:
                {
                    // local edit - prevent the zscript debugger window from triggering keyboard events.
                    if (event.keyboard.display != all_get_display()) break;

                    update_key_shifts(&event);
                    _handle_key_release(a5_keyboard_keycode_map[event.keyboard.keycode]);
                    break;
                }
                case ALLEGRO_EVENT_KEY_CHAR:
                {
                    // local edit - prevent the zscript debugger window from triggering keyboard events.
                    if (event.keyboard.display != all_get_display()) break;

                    update_key_shifts(&event);
                    if(event.keyboard.unichar >= 0)
                    {
                        // local edit - don't break AltGr key modifiers.
                        bool is_alt = (event.keyboard.modifiers & ALLEGRO_KEYMOD_ALT) != 0;
                        bool is_ctrl = (event.keyboard.modifiers & ALLEGRO_KEYMOD_CTRL) != 0;
                        bool is_altgr = (event.keyboard.modifiers & ALLEGRO_KEYMOD_ALTGR) != 0;
                        bool is_cmd = (event.keyboard.modifiers & ALLEGRO_KEYMOD_COMMAND) != 0;

                        int key = event.keyboard.keycode;
                        int unichar = event.keyboard.unichar;
                        int scancode = a5_keyboard_keycode_map[key];

                        // If AltGr is held, it is character input.
                        // On Windows, AltGr often reports as Ctrl+Alt.
                        if (is_altgr || (is_alt && is_ctrl)) {
                            _handle_key_press(unichar, scancode);
                        }
                        // If Alt or Cmd is held (without Ctrl), it might be a shortcut or a Mac Option-character.
                        else if (is_alt || is_cmd) {
                            bool is_shortcut = true;

                            // Heuristic: If the produced character is different from the natural
                            // key character, it's likely intentional character input (like Mac Option-key).
                            if (key >= ALLEGRO_KEY_A && key <= ALLEGRO_KEY_Z) {
                                if (unichar != (97 + key - ALLEGRO_KEY_A) &&
                                    unichar != (65 + key - ALLEGRO_KEY_A))
                                    is_shortcut = false;
                            }
                            else if (key >= ALLEGRO_KEY_0 && key <= ALLEGRO_KEY_9) {
                                if (unichar != (48 + key - ALLEGRO_KEY_0))
                                    is_shortcut = false;
                            }
                            else if (unichar >= 128) {
                                // Any non-ASCII character produced with Alt/Cmd is likely intentional.
                                is_shortcut = false;
                            }

                            if (is_shortcut) {
                                _handle_key_press(0, scancode);
                            } else {
                                _handle_key_press(unichar, scancode);
                            }
                        }
                        else {
                            // Normal typing (no Alt/Cmd) or Ctrl-shortcuts (which don't usually produce unichars).
                            _handle_key_press(unichar, scancode);
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

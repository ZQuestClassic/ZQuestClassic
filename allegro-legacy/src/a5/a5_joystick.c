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
 *      Joystick driver for BeOS.
 *
 *      By Angelo Mottola.
 *
 *      See readme.txt for copyright information.
 */

#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/ainta5.h"
#include "allegro/platform/ala5.h"

static ALLEGRO_THREAD * a5_joystick_thread = NULL;

static int a5_get_joystick(ALLEGRO_JOYSTICK * joystick)
{
    int i;

    for(i = 0; i < al_get_num_joysticks(); i++)
    {
        if(joystick == al_get_joystick(i))
        {
            return i;
        }
    }
    return -1;
}

static int stick_to_btn[10];

static void a5_reconfigure_joysticks()
{
    ALLEGRO_JOYSTICK * joystick;
    int i, j, k;

    memset(stick_to_btn, 0, sizeof(stick_to_btn));

    num_joysticks = al_get_num_joysticks();
    for(i = 0; i < num_joysticks; i++)
    {
        joystick = al_get_joystick(i);
        if(joystick)
        {
            /* top level */
            joy[i].flags = JOYFLAG_DIGITAL | JOYFLAG_ANALOGUE;

            /* buttons */
            joy[i].num_buttons = al_get_joystick_num_buttons(joystick);
            for(j = 0; j < joy[i].num_buttons; j++)
            {
                joy[i].button[j].name = al_get_joystick_button_name(joystick, j);
            }

            /* sticks */
            joy[i].num_sticks = al_get_joystick_num_sticks(joystick);
            for(j = 0; j < joy[i].num_sticks; j++)
            {
                joy[i].stick[j].name = al_get_joystick_stick_name(joystick, j);
                joy[i].stick[j].flags = JOYFLAG_DIGITAL | JOYFLAG_ANALOGUE | JOYFLAG_SIGNED;
                joy[i].stick[j].num_axis = al_get_joystick_num_axes(joystick, j);
                for(k = 0; k < joy[i].stick[j].num_axis; k++)
                {
                    joy[i].stick[j].axis[k].name = al_get_joystick_axis_name(joystick, j, k);
                }

                // local edit - treat trigger-like sticks as a button.
                // This is quite hacky, and is only done so that the rest of the control code
                // can deal in only buttons. Ideally, the control dialog would be able to select
                // a stick and a given threshold to treat as a button press.
                if (joy[i].stick[j].num_axis == 1)
                {
                    stick_to_btn[j] = joy[i].num_buttons;
                    joy[i].button[joy[i].num_buttons].name = joy[i].stick[j].name;
                    joy[i].num_buttons++;
                }
            }
        }
    }
}

static void * a5_joystick_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
    ALLEGRO_EVENT_QUEUE * queue;
    ALLEGRO_EVENT event;
    ALLEGRO_TIMEOUT timeout;
    int i;

    queue = al_create_event_queue();
    if(!queue)
    {
        return NULL;
    }
    al_register_event_source(queue, al_get_joystick_event_source());
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
                case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN:
                {
                    i = a5_get_joystick(event.joystick.id);
                    if(i >= 0)
                    {
                        joy[i].button[event.joystick.button].b = 1;
                    }
                    break;
                }
                case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP:
                {
                    i = a5_get_joystick(event.joystick.id);
                    if(i >= 0)
                    {
                        joy[i].button[event.joystick.button].b = 0;
                    }
                    break;
                }
                case ALLEGRO_EVENT_JOYSTICK_AXIS:
                {
                    i = a5_get_joystick(event.joystick.id);
                    if(i >= 0)
                    {
                        if(event.joystick.pos < -0.5)
                        {
                            joy[i].stick[event.joystick.stick].axis[event.joystick.axis].d1 = 1;
                            joy[i].stick[event.joystick.stick].axis[event.joystick.axis].d2 = 0;
                        }
                        else if(event.joystick.pos > 0.5)
                        {
                            joy[i].stick[event.joystick.stick].axis[event.joystick.axis].d1 = 0;
                            joy[i].stick[event.joystick.stick].axis[event.joystick.axis].d2 = 1;
                        }
                        else
                        {
                            joy[i].stick[event.joystick.stick].axis[event.joystick.axis].d1 = 0;
                            joy[i].stick[event.joystick.stick].axis[event.joystick.axis].d2 = 0;
                        }
                        joy[i].stick[event.joystick.stick].axis[event.joystick.axis].pos = event.joystick.pos * 128.0;

                        // local edit - treat like a button
                        if (joy[i].stick[event.joystick.stick].num_axis == 1)
                        {
                            int btn = stick_to_btn[event.joystick.stick];
                            joy[i].button[btn].b = event.joystick.pos > 0.1;
                        }
                    }
                    break;
                }
                // local edit
                case ALLEGRO_EVENT_JOYSTICK_CONFIGURATION:
                {
                    al_reconfigure_joysticks();
                    a5_reconfigure_joysticks();
                    break;
                }
            }
        }
    }
    al_destroy_event_queue(queue);
    return NULL;
}

static int a5_joystick_init(void)
{
    if(!al_install_joystick())
    {
        return -1;
    }
    a5_joystick_thread = al_create_thread(a5_joystick_thread_proc, NULL);
    if(!a5_joystick_thread)
    {
        al_uninstall_joystick();
        return -1;
    }
    a5_reconfigure_joysticks();
    al_start_thread(a5_joystick_thread);
    return 0;
}

static void a5_joystick_exit(void)
{
#ifdef ALLEGRO_LEGACY_CLOSE_THREADS
    al_destroy_thread(a5_joystick_thread);
    a5_joystick_thread = NULL;
#endif
    al_uninstall_joystick();
}

static int a5_joystick_poll(void)
{
    return 0;
}

JOYSTICK_DRIVER joystick_allegro_5 =
{
   JOYSTICK_ALLEGRO_5,        // int  id;
   empty_string,         // AL_CONST char *name;
   empty_string,         // AL_CONST char *desc;
   "Allegro 5 Joystick",      // AL_CONST char *ascii_name;
   a5_joystick_init, //be_joy_init,          // AL_LEGACY_METHOD(int, init, (void));
   a5_joystick_exit, //be_joy_exit,          // AL_LEGACY_METHOD(void, exit, (void));
   a5_joystick_poll, //be_joy_poll,          // AL_LEGACY_METHOD(int, poll, (void));
   NULL,                 // AL_LEGACY_METHOD(int, save_data, (void));
   NULL,                 // AL_LEGACY_METHOD(int, load_data, (void));
   NULL,                 // AL_LEGACY_METHOD(AL_CONST char *, calibrate_name, (int n));
   NULL                  // AL_LEGACY_METHOD(int, calibrate, (int n));
};

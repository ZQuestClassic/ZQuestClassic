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
#include "allegro/platform/ala5.h"

#define _A5_MAX_TIMERS 32

// local edit
// One thread serves every legacy timer. Each install_int gets its own allegro 5
// timer, all registered on one event queue, and the thread dispatches each tick
// to the callback whose timer fired. (Upstream ran a thread per timer.)
// A thread per timer also meant a callback removing itself - the MIDI player
// does that when a song loops - had nothing to join; here remove_int only stops
// that timer, and the thread is only joined by a5_timer_exit.
typedef struct
{
    ALLEGRO_TIMER * timer;
    void (*timer_proc)(void);
    void (*param_timer_proc)(void * data);
    void * data;

} _A5_TIMER_DATA;

static _A5_TIMER_DATA a5_timer_data[_A5_MAX_TIMERS];

static ALLEGRO_THREAD * a5_timer_thread = NULL;
static ALLEGRO_EVENT_QUEUE * a5_timer_queue = NULL;
static ALLEGRO_EVENT_SOURCE a5_timer_stop_source;

// Serializes the callbacks against install/remove. Recursive, because a
// callback may install or remove timers (including itself).
static ALLEGRO_MUTEX * timers_mutex;

int timer_is_installed();
static void * a5_timer_proc(ALLEGRO_THREAD * thread, void * data)
{
    ALLEGRO_EVENT event;
    double cur_time, prev_time, diff_time;
    int i;

    prev_time = al_get_time();
    while(!al_get_thread_should_stop(thread))
    {
        al_wait_for_event(a5_timer_queue, &event);
        if(event.type != ALLEGRO_EVENT_TIMER)
            continue; // Woken by _a5_stop_thread.

        cur_time = al_get_time();
        diff_time = cur_time - prev_time;
        prev_time = cur_time;

        al_lock_mutex(timers_mutex);
        for(i = 0; i < _A5_MAX_TIMERS; i++)
        {
            if(a5_timer_data[i].timer != event.timer.source)
                continue;
            if(a5_timer_data[i].param_timer_proc)
            {
                a5_timer_data[i].param_timer_proc(a5_timer_data[i].data);
            }
            else if(a5_timer_data[i].timer_proc)
            {
                a5_timer_data[i].timer_proc();
            }
            break;
        }
        al_unlock_mutex(timers_mutex);

        if (timer_is_installed())
            _handle_timer_tick(MSEC_TO_TIMER(diff_time * 1000.0));
    }
    return NULL;
}

static int a5_timer_init(void)
{
    memset(a5_timer_data, 0, sizeof(a5_timer_data));
    timers_mutex = al_create_mutex_recursive();
    a5_timer_queue = al_create_event_queue();
    a5_timer_thread = al_create_thread(a5_timer_proc, NULL);
    if(!timers_mutex || !a5_timer_queue || !a5_timer_thread)
    {
        return -1;
    }
    al_init_user_event_source(&a5_timer_stop_source);
    al_register_event_source(a5_timer_queue, &a5_timer_stop_source);
    al_start_thread(a5_timer_thread);
    return 0;
}

// The thread must be joined before allegro 5 shuts down (see _a5_stop_thread),
// and before remove_timer destroys the mutex that _handle_timer_tick locks.
static void a5_timer_exit(void)
{
    int i;

    _a5_stop_thread(a5_timer_thread, &a5_timer_stop_source);
    al_destroy_thread(a5_timer_thread);
    a5_timer_thread = NULL;

    for(i = 0; i < _A5_MAX_TIMERS; i++)
    {
        if(a5_timer_data[i].timer)
        {
            al_destroy_timer(a5_timer_data[i].timer);
        }
    }
    memset(a5_timer_data, 0, sizeof(a5_timer_data));

    al_destroy_event_queue(a5_timer_queue);
    a5_timer_queue = NULL;
    al_destroy_user_event_source(&a5_timer_stop_source);
    al_destroy_mutex(timers_mutex);
    timers_mutex = NULL;
}

static double a5_get_timer_speed(long speed)
{
    return (double)speed / (float)TIMERS_PER_SECOND;
}

// Finds an unused slot and gives it a (stopped) timer at the given speed.
static _A5_TIMER_DATA * a5_get_free_timer_data(long speed)
{
    int i;

    for(i = 0; i < _A5_MAX_TIMERS; i++)
    {
        _A5_TIMER_DATA * timer_data = &a5_timer_data[i];
        if(timer_data->timer_proc || timer_data->param_timer_proc)
            continue;

        if(!timer_data->timer)
        {
            timer_data->timer = al_create_timer(a5_get_timer_speed(speed));
            if(!timer_data->timer)
                return NULL;
            al_register_event_source(a5_timer_queue, al_get_timer_event_source(timer_data->timer));
        }
        else
        {
            al_set_timer_speed(timer_data->timer, a5_get_timer_speed(speed));
        }
        return timer_data;
    }

    return NULL;
}

static int a5_timer_install_int(void (*proc)(void), long speed)
{
    int i;
    int result = -1;
    _A5_TIMER_DATA * timer_data;

    al_lock_mutex(timers_mutex);

    for(i = 0; i < _A5_MAX_TIMERS; i++)
    {
        if(proc == a5_timer_data[i].timer_proc)
        {
            al_set_timer_speed(a5_timer_data[i].timer, a5_get_timer_speed(speed));
            al_unlock_mutex(timers_mutex);
            return 0;
        }
    }

    timer_data = a5_get_free_timer_data(speed);
    if(timer_data)
    {
        timer_data->timer_proc = proc;
        al_start_timer(timer_data->timer);
        result = 0;
    }

    al_unlock_mutex(timers_mutex);
    return result;
}

static void a5_timer_remove_int(void (*proc)(void))
{
    int i;

    al_lock_mutex(timers_mutex);

    for(i = 0; i < _A5_MAX_TIMERS; i++)
    {
        if(proc == a5_timer_data[i].timer_proc)
        {
            al_stop_timer(a5_timer_data[i].timer);
            a5_timer_data[i].timer_proc = NULL;
            break;
        }
    }

    al_unlock_mutex(timers_mutex);
}

static int a5_timer_install_param_int(void (*proc)(void * data), void * param, long speed)
{
    int i;
    int result = -1;
    _A5_TIMER_DATA * timer_data;

    al_lock_mutex(timers_mutex);

    for(i = 0; i < _A5_MAX_TIMERS; i++)
    {
        if(proc == a5_timer_data[i].param_timer_proc && param == a5_timer_data[i].data)
        {
            al_set_timer_speed(a5_timer_data[i].timer, a5_get_timer_speed(speed));
            al_unlock_mutex(timers_mutex);
            return 0;
        }
    }

    timer_data = a5_get_free_timer_data(speed);
    if(timer_data)
    {
        timer_data->param_timer_proc = proc;
        timer_data->data = param;
        al_start_timer(timer_data->timer);
        result = 0;
    }

    al_unlock_mutex(timers_mutex);
    return result;
}

static void a5_timer_remove_param_int(void (*proc)(void * data), void * param)
{
    int i;

    al_lock_mutex(timers_mutex);

    for(i = 0; i < _A5_MAX_TIMERS; i++)
    {
        if(proc == a5_timer_data[i].param_timer_proc && param == a5_timer_data[i].data)
        {
            al_stop_timer(a5_timer_data[i].timer);
            a5_timer_data[i].param_timer_proc = NULL;
            a5_timer_data[i].data = NULL;
            break;
        }
    }

    al_unlock_mutex(timers_mutex);
}

static void a5_timer_rest(unsigned int time, void (*callback)(void))
{
    double start_time = al_get_time();
    if(callback)
    {
        while(al_get_time() - start_time < (double)time / 1000.0)
        {
            callback();
        }
    }
    else
    {
        al_rest((double)time / 1000.0);
    }
}

TIMER_DRIVER timer_allegro5 = {
   TIMERDRV_ALLEGRO_5,		// int id;
   empty_string,	// char *name;
   empty_string,	// char *desc;
   "Allegro 5 Timer",		// char *ascii_name;
   a5_timer_init,	// AL_LEGACY_METHOD(int, init, (void));
   a5_timer_exit,	// AL_LEGACY_METHOD(void, exit, (void));
   a5_timer_install_int, 		// AL_LEGACY_METHOD(int, install_int, (AL_LEGACY_METHOD(void, proc, (void)), long speed));
   a5_timer_remove_int,		// AL_LEGACY_METHOD(void, remove_int, (AL_LEGACY_METHOD(void, proc, (void))));
   a5_timer_install_param_int,		// AL_LEGACY_METHOD(int, install_param_int, (AL_LEGACY_METHOD(void, proc, (void *param)), void *param, long speed));
   a5_timer_remove_param_int,		// AL_LEGACY_METHOD(void, remove_param_int, (AL_LEGACY_METHOD(void, proc, (void *param)), void *param));
   NULL,		// AL_LEGACY_METHOD(int, can_simulate_retrace, (void));
   NULL,		// AL_LEGACY_METHOD(void, simulate_retrace, (int enable));
   a5_timer_rest,	// AL_LEGACY_METHOD(void, rest, (long time, AL_LEGACY_METHOD(void, callback, (void))));
};

_DRIVER_INFO _timer_driver_list[] = {
   {TIMERDRV_ALLEGRO_5, &timer_allegro5, TRUE},
   {0, NULL, 0}
};

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

typedef struct
{
    ALLEGRO_TIMER * timer;
    void (*timer_proc)(void);
    void (*param_timer_proc)(void * data);
    void * data;

} _A5_TIMER_DATA;

static _A5_TIMER_DATA * a5_timer_data[_A5_MAX_TIMERS];

// local edit
static ALLEGRO_TIMER * a5_timer;
static ALLEGRO_THREAD * a5_timer_thread;
static ALLEGRO_EVENT_QUEUE * a5_timer_queue;
static ALLEGRO_MUTEX * timers_mutex;

static double a5_get_timer_speed(long speed)
{
    return (double)speed / (float)TIMERS_PER_SECOND;
}

static _A5_TIMER_DATA * a5_create_timer_data(void)
{
    _A5_TIMER_DATA * timer_data;

    timer_data = malloc(sizeof(_A5_TIMER_DATA));
    if(timer_data)
    {
        memset(timer_data, 0, sizeof(_A5_TIMER_DATA));
    }
    return timer_data;
}

// local edit
static _A5_TIMER_DATA * a5_get_free_timer_data(void)
{
    for(int i = 0; i < _A5_MAX_TIMERS; i++)
    {
        if (a5_timer_data[i] == NULL)
        {
            a5_timer_data[i] = a5_create_timer_data();
            return a5_timer_data[i];
        }

        if (a5_timer_data[i]->timer_proc == NULL && a5_timer_data[i]->param_timer_proc == NULL)
        {
            return a5_timer_data[i];
        }
    }

    return NULL;
}

static void a5_destroy_timer_data(_A5_TIMER_DATA * timer_data)
{
    if(timer_data->timer)
    {
        al_destroy_timer(timer_data->timer);
    }
    free(timer_data);
}

static void * a5_timer_proc(ALLEGRO_THREAD * thread, void * unused)
{
    ALLEGRO_TIMER* timer;
    ALLEGRO_EVENT event;
    double cur_time, prev_time = 0.0, diff_time;
    _A5_TIMER_DATA * timer_data = NULL;

    timer = al_create_timer(a5_get_timer_speed(0x8000));
    cur_time = al_get_time();

    while(!al_get_thread_should_stop(thread))
    {
        al_wait_for_event(a5_timer_queue, &event);

        al_lock_mutex(timers_mutex);
        timer_data = NULL;
        for(int i = 0; i < _A5_MAX_TIMERS && a5_timer_data[i]; i++)
        {
            if (a5_timer_data[i]->timer == event.timer.source) {
                timer_data = a5_timer_data[i];
                break;
            }
        }

        if(timer_data)
        {
            cur_time = al_get_time();
            diff_time = cur_time - prev_time;
            prev_time = cur_time;
            if(timer_data->param_timer_proc)
            {
                timer_data->param_timer_proc(timer_data->data);
            }
            else if(timer_data->timer_proc)
            {
                timer_data->timer_proc();
            }
            _handle_timer_tick(MSEC_TO_TIMER(diff_time * 1000.0));
        }

        al_unlock_mutex(timers_mutex);
    }
    return NULL;
}

static int a5_timer_init(void)
{
    a5_timer_queue = al_create_event_queue();
    if(!a5_timer_queue)
    {
        return 1;
    }

    timers_mutex = al_create_mutex_recursive();
    a5_timer_thread = al_create_thread(a5_timer_proc, NULL);

    al_start_thread(a5_timer_thread);
    return 0;
}

static void a5_timer_exit(void)
{
    int i;

    al_destroy_event_queue(a5_timer_queue);
    // For some reason on Mac this will hang program exit.
#ifndef __APPLE__
    al_destroy_thread(a5_timer_thread);
#endif

    for(i = 0; i < _A5_MAX_TIMERS && a5_timer_data[i]; i++)
    {
        a5_destroy_timer_data(a5_timer_data[i]);
        a5_timer_data[i] = NULL;
    }
}

// local edit
static int _a5_timer_install_int(void (*proc)(void), long speed)
{
    int i;

    for(i = 0; i < _A5_MAX_TIMERS && a5_timer_data[i]; i++)
    {
        if(proc == a5_timer_data[i]->timer_proc)
        {
            al_set_timer_speed(a5_timer_data[i]->timer, a5_get_timer_speed(speed));
            return 0;
        }
    }

    _A5_TIMER_DATA* timer_data = a5_get_free_timer_data();
    if (!timer_data) return -1;
    if (!timer_data->timer) timer_data->timer = al_create_timer(a5_get_timer_speed(speed));
    else al_set_timer_speed(timer_data->timer, a5_get_timer_speed(speed));
    if (!timer_data->timer) return -1;

    timer_data->timer_proc = proc;
    al_start_timer(timer_data->timer);
    al_register_event_source(a5_timer_queue, al_get_timer_event_source(timer_data->timer));
    return 0;
}

static int a5_timer_install_int(void (*proc)(void), long speed)
{
    al_lock_mutex(timers_mutex);
    int result = _a5_timer_install_int(proc, speed);
    al_unlock_mutex(timers_mutex);
    return result;
}

// local edit
static void a5_timer_remove_int(void (*proc)(void))
{
    int i, j;

    al_lock_mutex(timers_mutex);

    for(i = 0; i < _A5_MAX_TIMERS && a5_timer_data[i]; i++)
    {
        if(proc == a5_timer_data[i]->timer_proc)
        {
            al_unregister_event_source(a5_timer_queue, al_get_timer_event_source(a5_timer_data[i]->timer));
            al_stop_timer(a5_timer_data[i]->timer);
            a5_timer_data[i]->timer_proc = NULL;
            break;
        }
    }

    al_unlock_mutex(timers_mutex);
}

// local edit
static int _a5_timer_install_param_int(void (*proc)(void * data), void * param, long speed)
{
    int i;

    for(i = 0; i < _A5_MAX_TIMERS && a5_timer_data[i]; i++)
    {
        if(proc == a5_timer_data[i]->param_timer_proc && param == a5_timer_data[i]->data)
        {
            a5_timer_data[i]->data = param;
            al_set_timer_speed(a5_timer_data[i]->timer, a5_get_timer_speed(speed));
            return 0;
        }
    }

    _A5_TIMER_DATA* timer_data = a5_get_free_timer_data();
    if (!timer_data) return -1;
    if (!timer_data->timer) timer_data->timer = al_create_timer(a5_get_timer_speed(speed));
    else al_set_timer_speed(timer_data->timer, a5_get_timer_speed(speed));
    if (!timer_data->timer) return -1;

    timer_data->param_timer_proc = proc;
    timer_data->data = param;
    al_start_timer(timer_data->timer);
    al_register_event_source(a5_timer_queue, al_get_timer_event_source(timer_data->timer));

    return 0;
}

static int a5_timer_install_param_int(void (*proc)(void * data), void * param, long speed)
{
    al_lock_mutex(timers_mutex);
    int result = _a5_timer_install_param_int(proc, param, speed);
    al_unlock_mutex(timers_mutex);
    return result;
}

static void a5_timer_remove_param_int(void (*proc)(void * data), void * param)
{
    int i, j;

    al_lock_mutex(timers_mutex);

    for(i = 0; i < _A5_MAX_TIMERS && a5_timer_data[i]; i++)
    {
        // local edit
        if(proc == a5_timer_data[i]->param_timer_proc && param == a5_timer_data[i]->data)
        {
            al_unregister_event_source(a5_timer_queue, al_get_timer_event_source(a5_timer_data[i]->timer));
            al_stop_timer(a5_timer_data[i]->timer);
            a5_timer_data[i]->param_timer_proc = NULL;
            a5_timer_data[i]->data = NULL;
            break;
        }
    }

    al_unlock_mutex(timers_mutex);
}

static void a5_timer_rest(unsigned int time, void (*callback)(void))
{
    double start_time = al_get_time();
    double current_time = start_time;
    if(callback)
    {
        while(current_time - start_time < (double)time / 1000.0)
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

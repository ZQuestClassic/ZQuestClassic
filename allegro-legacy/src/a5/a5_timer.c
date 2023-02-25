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

    ALLEGRO_THREAD * thread;
    ALLEGRO_TIMER * timer;
    void (*timer_proc)(void);
    void (*param_timer_proc)(void * data);
    void * data;

} _A5_TIMER_DATA;

static _A5_TIMER_DATA * a5_timer_data[_A5_MAX_TIMERS];

static ALLEGRO_MUTEX * timers_mutex;

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

static _A5_TIMER_DATA * a5_get_free_timer_data(void)
{
    int i;

    for(i = 0; i < _A5_MAX_TIMERS; i++)
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
    if(timer_data->thread)
    {
        al_destroy_thread(timer_data->thread);
    }
    if(timer_data->timer)
    {
        al_destroy_timer(timer_data->timer);
    }
    free(timer_data);
}

static void * a5_timer_proc(ALLEGRO_THREAD * thread, void * data)
{
    ALLEGRO_EVENT_QUEUE * queue;
    ALLEGRO_EVENT event;
    ALLEGRO_TIMEOUT timeout;
    double cur_time, prev_time = 0.0, diff_time;
    _A5_TIMER_DATA * timer_data = (_A5_TIMER_DATA *)data;

    queue = al_create_event_queue();
    if(!queue)
    {
        return NULL;
    }
    al_register_event_source(queue, al_get_timer_event_source(timer_data->timer));
    al_start_timer(timer_data->timer);
    while(!al_get_thread_should_stop(thread))
    {
#ifdef ALLEGRO_LEGACY_CLOSE_THREADS
        al_init_timeout(&timeout, 0.1);
        // TODO: why does this hog so much CPU?!
        if (al_wait_for_event_until(queue, &event, &timeout))
#else
        al_wait_for_event(queue, &event);
#endif
        // if(al_wait_for_event_until(queue, &event, &timeout))
        {
            cur_time = al_get_time();
            diff_time = cur_time - prev_time;
            prev_time = al_get_time();
            al_lock_mutex(timers_mutex);
            if(timer_data->param_timer_proc)
            {
                timer_data->param_timer_proc(timer_data->data);
            }
            else if(timer_data->timer_proc)
            {
                timer_data->timer_proc();
            }
            al_unlock_mutex(timers_mutex);
            if (timer_is_installed())
                _handle_timer_tick(MSEC_TO_TIMER(diff_time * 1000.0));
        }
    }
    al_stop_timer(timer_data->timer);
    al_destroy_event_queue(queue);
    return NULL;
}

static int a5_timer_init(void)
{
    timers_mutex = al_create_mutex_recursive();
    return 0;
}

static void a5_timer_exit(void)
{
    // Trying to destroy threads on exit just hangs everything :/
#ifdef ALLEGRO_LEGACY_CLOSE_THREADS
    int i;

    for(i = 0; i < _A5_MAX_TIMERS && a5_timer_data[i]; i++)
    {
        a5_destroy_timer_data(a5_timer_data[i]);
    }

#endif
}

static double a5_get_timer_speed(long speed)
{
    return (double)speed / (float)TIMERS_PER_SECOND;
}

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
    if (!timer_data->thread) timer_data->thread = al_create_thread(a5_timer_proc, timer_data);
    if (!timer_data->timer) timer_data->timer = al_create_timer(a5_get_timer_speed(speed));
    else al_set_timer_speed(timer_data->timer, a5_get_timer_speed(speed));
    if (!timer_data->thread || !timer_data->timer) return -1;

    timer_data->timer_proc = proc;
    al_start_thread(timer_data->thread);
    al_start_timer(timer_data->timer);
    return 0;
}

static int a5_timer_install_int(void (*proc)(void), long speed)
{
    al_lock_mutex(timers_mutex);
    int result = _a5_timer_install_int(proc, speed);
    al_unlock_mutex(timers_mutex);
    return result;
}

static void a5_timer_remove_int(void (*proc)(void))
{
    int i, j;

    al_lock_mutex(timers_mutex);

    for(i = 0; i < _A5_MAX_TIMERS && a5_timer_data[i]; i++)
    {
        if(proc == a5_timer_data[i]->timer_proc)
        {
            al_stop_timer(a5_timer_data[i]->timer);
            a5_timer_data[i]->timer_proc = NULL;
            break;
        }
    }

    al_unlock_mutex(timers_mutex);
}

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
    if (!timer_data->thread) timer_data->thread = al_create_thread(a5_timer_proc, timer_data);
    if (!timer_data->timer) timer_data->timer = al_create_timer(a5_get_timer_speed(speed));
    else al_set_timer_speed(timer_data->timer, speed);
    if (!timer_data->thread || !timer_data->timer) return -1;

    timer_data->param_timer_proc = proc;
    timer_data->data = param;
    al_start_thread(timer_data->thread);
    al_start_timer(timer_data->timer);

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
        if(proc == a5_timer_data[i]->param_timer_proc && param == a5_timer_data[i]->data)
        {
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

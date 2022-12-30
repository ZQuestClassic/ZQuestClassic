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

#define _A5_SOUND_BUFFERS        2
#define _A5_SOUND_BUFFER_SIZE 1024
#define _A5_SOUND_FREQUENCY  44100
#define _A5_SOUND_DEPTH ALLEGRO_AUDIO_DEPTH_INT16
#define _A5_SOUND_CHANNELS ALLEGRO_CHANNEL_CONF_2

static ALLEGRO_THREAD * a5_sound_thread = NULL;
static ALLEGRO_AUDIO_STREAM * a5_sound_stream = NULL;
static ALLEGRO_MUTEX * a5_sound_mutex = NULL;

static void * a5_sound_thread_proc(ALLEGRO_THREAD * thread, void * data)
{
    ALLEGRO_EVENT_QUEUE * queue;
    ALLEGRO_EVENT event;
    ALLEGRO_TIMEOUT timeout;
    void * fragment;
    bool fragments_done = false;

    a5_sound_stream = al_create_audio_stream(_A5_SOUND_BUFFERS, _A5_SOUND_BUFFER_SIZE, _A5_SOUND_FREQUENCY, _A5_SOUND_DEPTH, _A5_SOUND_CHANNELS);
    if(!a5_sound_stream)
    {
        return NULL;
    }
    if(!al_attach_audio_stream_to_mixer(a5_sound_stream, al_get_default_mixer()))
    {
        al_destroy_audio_stream(a5_sound_stream);
        return NULL;
    }
    queue = al_create_event_queue();
    if(!queue)
    {
        al_destroy_audio_stream(a5_sound_stream);
        return NULL;
    }
    al_set_audio_stream_playing(a5_sound_stream, true);
    al_register_event_source(queue, al_get_audio_stream_event_source(a5_sound_stream));
    while(!al_get_thread_should_stop(thread))
    {
#ifdef ALLEGRO_LEGACY_CLOSE_THREADS
        if (al_wait_for_event_until(queue, &event, &timeout))
#else
        al_wait_for_event(queue, &event);
#endif
        {
            switch(event.type)
            {
                case ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT:
                {
                    fragments_done = false;
                    while(!fragments_done)
                    {
                        fragment = al_get_audio_stream_fragment(a5_sound_stream);
                        if(fragment)
                        {
                            // local edit
                            _mix_some_samples((uintptr_t)fragment, 0, TRUE);
                            al_set_audio_stream_fragment(a5_sound_stream, fragment);
                        }
                        else
                        {
                            fragments_done = true;
                        }
                    }
                    break;
                }
            }
        }
    }
    al_destroy_event_queue(queue);
    al_destroy_audio_stream(a5_sound_stream);
    return NULL;
}

static int a5_sound_detect(int input)
{
    return TRUE;
}

static int a5_sound_init(int input, int voices)
{
    digi_allegro_5.voices = voices;
    _sound_stereo = 1;
    _sound_freq = _A5_SOUND_FREQUENCY;
    _sound_bits = 16;

    if(_mixer_init(_A5_SOUND_BUFFER_SIZE * (_sound_bits / 8), _sound_freq, _sound_stereo, ((_sound_bits == 16) ? 1 : 0), &digi_allegro_5.voices) != 0)
    {
        return -1;
    }
    if(!al_install_audio())
    {
        return -1;
    }
    if(!al_reserve_samples(4))
    {
        al_uninstall_audio();
        return -1;
    }
    a5_sound_mutex = al_create_mutex();
    if(!a5_sound_mutex)
    {
        al_uninstall_audio();
        return -1;
    }
    a5_sound_thread = al_create_thread(a5_sound_thread_proc, NULL);
    if(!a5_sound_thread)
    {
        al_uninstall_audio();
        return -1;
    }
    al_start_thread(a5_sound_thread);
    return 0;
}

static void a5_sound_exit(int input)
{
#ifdef ALLEGRO_LEGACY_CLOSE_THREADS
    al_destroy_thread(a5_sound_thread);
    a5_sound_thread = NULL;
#endif
    al_uninstall_audio();
}

static int a5_sound_set_mixer_volume(int volume)
{
    if(al_set_audio_stream_gain(a5_sound_stream, (float)volume / 255.0))
    {
        return 0;
    }
    return -1;
}

static int a5_sound_get_mixer_volume(void)
{
    return al_get_audio_stream_gain(a5_sound_stream) * 255.0;
}

static int a5_sound_buffer_size(void)
{
    return _A5_SOUND_BUFFER_SIZE * al_get_audio_depth_size(_A5_SOUND_DEPTH) * 2;
}

static void * a5_sound_lock_voice(int voice, int start, int end)
{
    al_lock_mutex(a5_sound_mutex);
    return NULL;
}

static void a5_sound_unlock_voice(int voice)
{
    al_unlock_mutex(a5_sound_mutex);
}

DIGI_DRIVER digi_allegro_5 =
{
   DIGI_ALLEGRO_5,
   empty_string,
   empty_string,
   "Allegro 5 Sound",
   0,
   0,
   MIXER_MAX_SFX,
   MIXER_DEF_SFX,

   a5_sound_detect,
   a5_sound_init,
   a5_sound_exit,
   a5_sound_set_mixer_volume,
   a5_sound_get_mixer_volume,

   a5_sound_lock_voice,
   a5_sound_unlock_voice,
   a5_sound_buffer_size,
   _mixer_init_voice,
   _mixer_release_voice,
   _mixer_start_voice,
   _mixer_stop_voice,
   _mixer_loop_voice,

   _mixer_get_position,
   _mixer_set_position,

   _mixer_get_volume,
   _mixer_set_volume,
   _mixer_ramp_volume,
   _mixer_stop_volume_ramp,

   _mixer_get_frequency,
   _mixer_set_frequency,
   _mixer_sweep_frequency,
   _mixer_stop_frequency_sweep,

   _mixer_get_pan,
   _mixer_set_pan,
   _mixer_sweep_pan,
   _mixer_stop_pan_sweep,

   _mixer_set_echo,
   _mixer_set_tremolo,
   _mixer_set_vibrato,
   0, 0,
   0,
   0,
   0,
   0,
   0,
   0
};

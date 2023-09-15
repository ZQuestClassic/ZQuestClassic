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
 *      MIDI driver for BeOS.
 *
 *      By Angelo Mottola.
 *
 *      See readme.txt for copyright information.
 */

#include "allegro.h"
#include "allegro/internal/aintern.h"
#include "allegro/platform/ainta5.h"
#include "allegro/platform/ala5.h"
#include "midia5/midia5.h"

static MIDIA5_OUTPUT_HANDLE * a5_midi_output_handle = NULL;
static int a5_midi_output_volume;

static int a5_midi_detect(int input)
{
    if(input)
    {
        return 0;
    }
    else
    {
        if(midia5_get_output_device_count() > 0)
        {
            return 1;
        }
    }
    return 0;
}

static int get_midi_device(void)
{
	int device_count;
	int i;

	device_count = midia5_get_output_device_count();

	/* look for FLUID Synth */
	for(i = 0; i < device_count; i++)
	{
		if(!memcmp(midia5_get_output_device_name(i), "FLUID", 5))
		{
			return i;
		}
	}
	if(device_count <= 0)
	{
		return -1;
	}
	return 0;
}

static int a5_midi_init(int input, int voices)
{
    int midi_device;

    if(input)
    {
        return -1;
    }
    else
    {
        midi_device = get_midi_device();
        if(midi_device >= 0)
        {
            a5_midi_output_handle = midia5_create_output_handle(midi_device);
            if(a5_midi_output_handle)
            {
                return 0;
            }
        }
    }
    return -1;
}

static void a5_midi_exit(int input)
{
    if(!input)
    {
        if(a5_midi_output_handle)
        {
            midia5_destroy_output_handle(a5_midi_output_handle);
            a5_midi_output_handle = NULL;
        }
    }
}

static int a5_midi_set_mixer_volume(int volume)
{
    a5_midi_output_volume = volume;
    return midia5_set_output_gain(a5_midi_output_handle, a5_midi_output_volume / 255.0);
}

static int a5_midi_get_mixer_volume(void)
{
    return a5_midi_output_volume;
}

static void a5_raw_midi(int data)
{
    midia5_send_data(a5_midi_output_handle, data);
}

MIDI_DRIVER midi_allegro_5 =
{
   MIDI_ALLEGRO_5,               /* driver ID code */
   empty_string,            /* driver name */
   empty_string,            /* description string */
   "Allegro 5 MIDI",             /* ASCII format name string */
   16,                      /* available voices */
   0,                       /* voice number offset */
   28,                      /* maximum voices we can support */
   0,                       /* default number of voices to use */
   10, 10,                  /* reserved voice range */
   a5_midi_detect, //be_midi_detect,          /* AL_LEGACY_METHOD(int,  detect, (int input)); */
   a5_midi_init, //be_midi_init,            /* AL_LEGACY_METHOD(int,  init, (int input, int voices)); */
   a5_midi_exit, //be_midi_exit,            /* AL_LEGACY_METHOD(void, exit, (int input)); */
   a5_midi_set_mixer_volume, //be_midi_set_mixer_volume,/* AL_LEGACY_METHOD(int,  set_mixer_volume, (int volume)); */
   a5_midi_get_mixer_volume, //be_midi_get_mixer_volume,/* AL_LEGACY_METHOD(int,  get_mixer_volume, (void)); */
   a5_raw_midi,                    /* AL_LEGACY_METHOD(void, raw_midi, (int data)); */
   _dummy_load_patches,     /* AL_METHOD(int,  load_patches, (AL_CONST char *patches, AL_CONST char *drums)); */
   _dummy_adjust_patches,   /* AL_METHOD(void, adjust_patches, (AL_CONST char *patches, AL_CONST char *drums)); */
   _dummy_key_on,           /* AL_METHOD(void, key_on, (int inst, int note, int bend, int vol, int pan)); */
   _dummy_noop1,            /* AL_METHOD(void, key_off, (int voice)); */
   _dummy_noop2,            /* AL_METHOD(void, set_volume, (int voice, int vol)); */
   _dummy_noop3,            /* AL_METHOD(void, set_pitch, (int voice, int note, int bend)); */
   _dummy_noop2,            /* AL_METHOD(void, set_pan, (int voice, int pan)); */
   _dummy_noop2,            /* AL_METHOD(void, set_vibrato, (int voice, int amount)); */
};

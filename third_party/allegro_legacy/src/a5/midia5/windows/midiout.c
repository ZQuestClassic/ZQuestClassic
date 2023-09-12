#include <windows.h>
#include <mmsystem.h>
#include "../midia5.h"

typedef struct
{

    HMIDIOUT output_device;
	MIDIOUTCAPS output_device_caps;

    int midi_message;
	int midi_message_length;
	int midi_message_pos;

} MIDIA5_PLATFORM_DATA;

int _midia5_get_platform_output_device_count(void)
{
    return 1;
}

const char * _midia5_get_platform_output_device_name(int device)
{
    return "Windows MIDI Mapper";
}

void * _midia5_init_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp, int device)
{
    MIDIA5_PLATFORM_DATA * cm_data;

    cm_data = malloc(sizeof(MIDIA5_PLATFORM_DATA));
    if(cm_data)
    {
        if(midiOutOpen(&cm_data->output_device, MIDI_MAPPER, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
    	{
            free(cm_data);
            return NULL;
        }
    }
    return cm_data;
}

void _midia5_free_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp)
{
    MIDIA5_PLATFORM_DATA * cm_data = (MIDIA5_PLATFORM_DATA *)hp->platform_data;

    midiOutClose(cm_data->output_device);
    free(cm_data);
}

void _midia5_platform_send_data(MIDIA5_OUTPUT_HANDLE * hp, int data)
{
    MIDIA5_PLATFORM_DATA * cm_data = (MIDIA5_PLATFORM_DATA *)hp->platform_data;
    int message_length[8] = {3, 3, 3, 3, 2, 2, 3, 0};

	if(data >= 0x80)
	{
		cm_data->midi_message_length = message_length[(data >> 4) & 0x07];
		cm_data->midi_message = 0;
		cm_data->midi_message_pos = 0;
	}
	if(cm_data->midi_message_length > 0)
	{
		cm_data->midi_message |= ((unsigned long)data) << (cm_data->midi_message_pos * 8);
		cm_data->midi_message_pos++;
		if(cm_data->midi_message_pos == cm_data->midi_message_length)
		{
			midiOutShortMsg(cm_data->output_device, cm_data->midi_message);
		}
	}
}

void _midia5_platform_reset_output_device(MIDIA5_OUTPUT_HANDLE * hp)
{
	MIDIA5_PLATFORM_DATA * cm_data = (MIDIA5_PLATFORM_DATA *)hp->platform_data;

    midiOutReset(cm_data->output_device);
}

bool _midia5_platform_set_output_gain(MIDIA5_OUTPUT_HANDLE * hp, float gain)
{
    MIDIA5_PLATFORM_DATA * cm_data = (MIDIA5_PLATFORM_DATA *)hp->platform_data;

	return false;
}

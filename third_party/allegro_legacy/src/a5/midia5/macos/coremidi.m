#include <AVFoundation/AVFoundation.h>
#include "../midia5.h"

typedef struct
{

    AUGraph graph;
    AudioUnit synth_unit;
    int command, data_pos, data_buffer[2];

} MIDIA5_COREMIDI_DATA;

int _midia5_get_platform_output_device_count(void)
{
	return 1;
}

const char * _midia5_get_platform_output_device_name(int device)
{
	return "CoreMIDI Software Synthesizer";
}

static void _setup_output_platform_data(MIDIA5_COREMIDI_DATA * cm_data)
{
    AudioComponentDescription desc;
    AUNode synth_node, output_node;
    UInt32 quality, reverb_type;
    int reverb;
    struct
    {
        UInt32 type;
        char *name;
    } reverb_info[6] = {
        { kReverbRoomType_SmallRoom, "small room" },
        { kReverbRoomType_MediumRoom, "medium room" },
        { kReverbRoomType_LargeRoom, "large room" },
        { kReverbRoomType_MediumHall, "medium hall" },
        { kReverbRoomType_LargeHall, "large hall" },
        { kReverbRoomType_Plate, "plate" }
    };

    NewAUGraph(&cm_data->graph);

    desc.componentType = kAudioUnitType_MusicDevice;
    desc.componentSubType = kAudioUnitSubType_DLSSynth;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;
    AUGraphAddNode(cm_data->graph, &desc, &synth_node);

    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;
    AUGraphAddNode(cm_data->graph, &desc, &output_node);

    AUGraphConnectNodeInput(cm_data->graph, synth_node, 0, output_node, 0);

    AUGraphOpen(cm_data->graph);
    AUGraphInitialize(cm_data->graph);

    AUGraphNodeInfo(cm_data->graph, synth_node, NULL, &cm_data->synth_unit);

    quality = 127;
    AudioUnitSetProperty(cm_data->synth_unit, kAudioUnitProperty_RenderQuality, kAudioUnitScope_Output, 0, &quality, sizeof(quality));

    // local edit - reverb sound awful.
    // reverb = 5;
    // reverb_type = reverb_info[5].type;
    // AudioUnitSetProperty(cm_data->synth_unit, kAudioUnitProperty_ReverbRoomType, kAudioUnitScope_Output, 0, &reverb_type, sizeof(reverb_type));

    AUGraphStart(cm_data->graph);

    cm_data->command = -1;
}

void _unsetup_output_platform_data(MIDIA5_COREMIDI_DATA * cm_data)
{
    AUGraphStop(cm_data->graph);
    AUGraphUninitialize(cm_data->graph);
    AUGraphClose(cm_data->graph);
    DisposeAUGraph(cm_data->graph);
}

void * _midia5_init_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp, int device)
{
    MIDIA5_COREMIDI_DATA * cm_data;

    cm_data = malloc(sizeof(MIDIA5_COREMIDI_DATA));
    if(cm_data)
    {
        _setup_output_platform_data(cm_data);
    }
    return cm_data;
}

void _midia5_free_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp)
{
    MIDIA5_COREMIDI_DATA * cm_data = (MIDIA5_COREMIDI_DATA *)hp->platform_data;

    _unsetup_output_platform_data(cm_data);
    free(cm_data);
}

void _midia5_platform_send_data(MIDIA5_OUTPUT_HANDLE * hp, int data)
{
    MIDIA5_COREMIDI_DATA * cm_data = (MIDIA5_COREMIDI_DATA *)hp->platform_data;

    if(cm_data->command == -1)
    {
        cm_data->data_buffer[0] = cm_data->data_buffer[1] = 0;
        cm_data->data_pos = 0;
        cm_data->command = data;
        return;
    }
    cm_data->data_buffer[cm_data->data_pos++] = data;
    if(((cm_data->data_pos == 1) && (((cm_data->command >> 4) == 0xC) || ((cm_data->command >> 4) == 0xD))) || (cm_data->data_pos == 2))
    {
        MusicDeviceMIDIEvent(cm_data->synth_unit, cm_data->command, cm_data->data_buffer[0], cm_data->data_buffer[1], 0);
        cm_data->command = -1;
    }
}

void _midia5_platform_reset_output_device(MIDIA5_OUTPUT_HANDLE * hp)
{
    MIDIA5_COREMIDI_DATA * cm_data = (MIDIA5_COREMIDI_DATA *)hp->platform_data;
    _unsetup_output_platform_data(cm_data);
    _setup_output_platform_data(cm_data);
}

bool _midia5_platform_set_output_gain(MIDIA5_OUTPUT_HANDLE * hp, float gain)
{
    MIDIA5_COREMIDI_DATA * cm_data = (MIDIA5_COREMIDI_DATA *)hp->platform_data;

    return AudioUnitSetParameter(cm_data->synth_unit, kAudioUnitParameterUnit_LinearGain, kAudioUnitScope_Output, 0, gain, 0);
}

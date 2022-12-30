#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include "../midia5.h"

typedef struct
{

	/* sequencer data */
	snd_seq_t * sequencer;
	snd_seq_addr_t addr;
	int port;

	/* MIDI command data */
	int command_step;
	int command_type;
	int command_channel;
	int command_data[16];

} MIDIA5_ALSA_DATA;

int _midia5_get_platform_output_device_count(void)
{
	snd_seq_t *seq;
	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;
	int count = 0;

	snd_seq_open(&seq, "default", SND_SEQ_OPEN_OUTPUT, 0);
	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	while(snd_seq_query_next_client(seq, cinfo) >= 0)
	{
		int client = snd_seq_client_info_get_client(cinfo);

		snd_seq_port_info_set_client(pinfo, client);
		snd_seq_port_info_set_port(pinfo, -1);
		while(snd_seq_query_next_port(seq, pinfo) >= 0)
		{
			/* port must understand MIDI messages */
			if(!(snd_seq_port_info_get_type(pinfo) & SND_SEQ_PORT_TYPE_MIDI_GENERIC))
			{
				continue;
			}
			/* we need both WRITE and SUBS_WRITE */
			if((snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) != (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE))
			{
				continue;
			}
			count++;
		}
	}
	snd_seq_close(seq);
	return count;
}

static char _midia5_platform_output_device_name_buffer[1024];

const char * _midia5_get_platform_output_device_name(int device)
{
	snd_seq_t *seq;
	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;
	int count = 0;

	strcpy(_midia5_platform_output_device_name_buffer, "");

	if (snd_seq_open(&seq, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
		// local edit
		fprintf(stderr, "Error opening ALSA sequencer.\n");
		return _midia5_platform_output_device_name_buffer;
	}
	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);

	snd_seq_client_info_set_client(cinfo, -1);
	while(snd_seq_query_next_client(seq, cinfo) >= 0)
	{
		int client = snd_seq_client_info_get_client(cinfo);

		snd_seq_port_info_set_client(pinfo, client);
		snd_seq_port_info_set_port(pinfo, -1);
		while(snd_seq_query_next_port(seq, pinfo) >= 0)
		{
			/* port must understand MIDI messages */
			if(!(snd_seq_port_info_get_type(pinfo) & SND_SEQ_PORT_TYPE_MIDI_GENERIC))
			{
				continue;
			}
			/* we need both WRITE and SUBS_WRITE */
			if((snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) != (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE))
			{
				continue;
			}
			if(count == device)
			{
				sprintf(_midia5_platform_output_device_name_buffer, "%s:%d", snd_seq_client_info_get_name(cinfo), snd_seq_port_info_get_port(pinfo));
			}
			count++;
		}
	}
	snd_seq_close(seq);
	return _midia5_platform_output_device_name_buffer;
}

void * _midia5_init_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp, int device)
{
	MIDIA5_ALSA_DATA * cm_data;

	cm_data = malloc(sizeof(MIDIA5_ALSA_DATA));
	if(cm_data)
	{
		cm_data->command_step = 0;
		if(snd_seq_open(&cm_data->sequencer, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0)
		{
			fprintf(stderr, "Error opening ALSA sequencer.\n");
			free(cm_data);
			return NULL;
	 	}
		snd_seq_set_client_name(cm_data->sequencer, "MIDIA5");
		if(snd_seq_parse_address(cm_data->sequencer, &cm_data->addr, midia5_get_output_device_name(device)) < 0)
		{
			free(cm_data);
			printf("Failed to open MIDI device: %s!\n", midia5_get_output_device_name(device));
			return NULL;
		}
		cm_data->port = snd_seq_create_simple_port(cm_data->sequencer, "MIDIA5", 0, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
		if(cm_data->port < 0)
		{
			printf("Failed to create MIDI port!\n");
			free(cm_data);
			return NULL;
		}
		if(snd_seq_connect_to(cm_data->sequencer, 0, cm_data->addr.client, cm_data->addr.port) < 0)
		{
			free(cm_data);
			printf("Failed to open MIDI device!\n");
			return NULL;
		}
	}
	return cm_data;
}

void _midia5_free_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp)
{
	MIDIA5_ALSA_DATA * cm_data = (MIDIA5_ALSA_DATA *)hp->platform_data;

	snd_seq_disconnect_from(cm_data->sequencer, 0, cm_data->addr.client, cm_data->addr.port);
	snd_seq_delete_simple_port(cm_data->sequencer, cm_data->port);
	snd_seq_close(cm_data->sequencer);
	free(cm_data);
}

static int get_alsa_pitch_bend_value(int d1, int d2)
{
	return ((d1 & 127) | ((d2 & 127) << 7)) - 0x2000;
}

void _midia5_platform_send_data(MIDIA5_OUTPUT_HANDLE * hp, int data)
{
	MIDIA5_ALSA_DATA * cm_data = (MIDIA5_ALSA_DATA *)hp->platform_data;
	snd_seq_event_t ev;

	switch(cm_data->command_step)
	{
		case 0:
		{
			cm_data->command_type = data & 0xF0;
			cm_data->command_channel = data & 0x0F;
			switch(cm_data->command_type)
			{
				case 0x80:
				case 0x90:
				case 0xA0:
				case 0xB0:
				case 0xE0:
				case 0xC0:
				case 0xD0:
				{
					cm_data->command_step = 1;
					break;
				}
			}
			break;
		}
		case 1:
		{
			cm_data->command_data[0] = data;
			switch(cm_data->command_type)
			{
				case 0x80:
				case 0x90:
				case 0xA0:
				case 0xB0:
				case 0xE0:
				{
					cm_data->command_step = 2;
					break;
				}
				case 0xC0:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_pgmchange(&ev, cm_data->command_channel, cm_data->command_data[0]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
				case 0xD0:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_chanpress(&ev, cm_data->command_channel, cm_data->command_data[0]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
			}
			break;
		}
		case 2:
		{
			cm_data->command_data[1] = data;
			switch(cm_data->command_type)
			{
				case 0x80:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_noteoff(&ev, cm_data->command_channel, cm_data->command_data[0], cm_data->command_data[1]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
				case 0x90:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_noteon(&ev, cm_data->command_channel, cm_data->command_data[0], cm_data->command_data[1]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
				case 0xA0:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_keypress(&ev, cm_data->command_channel, cm_data->command_data[0], cm_data->command_data[1]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
				case 0xB0:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_controller(&ev, cm_data->command_channel, cm_data->command_data[0], cm_data->command_data[1]);
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
				case 0xE0:
				{
					snd_seq_ev_clear(&ev);
					snd_seq_ev_set_pitchbend(&ev, cm_data->command_channel, get_alsa_pitch_bend_value(cm_data->command_data[0], cm_data->command_data[1]));
					snd_seq_ev_set_direct(&ev);
					ev.dest = cm_data->addr;
					snd_seq_event_output_direct(cm_data->sequencer, &ev);
					snd_seq_drain_output(cm_data->sequencer);
					cm_data->command_step = 0;
					break;
				}
			}
			break;
		}
	}
}

void _midia5_platform_reset_output_device(MIDIA5_OUTPUT_HANDLE * hp)
{
	int i, j;

	for(i = 0; i < 0xF; i++)
	{
		for(j = 0; j < 128; j++)
		{
			midia5_send_data(hp, 0x80 | i);
			midia5_send_data(hp, j);
			midia5_send_data(hp, 127);
		}
	}
}

bool _midia5_platform_set_output_gain(MIDIA5_OUTPUT_HANDLE * hp, float gain)
{
	return false;
}

#include <stddef.h>

#include "../midia5.h"

/* THIS IS A PLACEHOLDER FILE
 * Output-less MIDI backend, for systems without a usable MIDI output API: the ALSA sequencer
 * backend is Linux-only, so the BSDs and other Unixes use this one. Reporting zero output
 * devices makes Allegro's MIDI driver behave as if no synthesizer were available (the same
 * thing that happens on Linux when no sequencer ports exist).
 *
 * Entire file is a local edit.
 */

int _midia5_get_platform_output_device_count(void)
{
	return 0;
}

const char * _midia5_get_platform_output_device_name(int device)
{
	(void)device;
	return NULL;
}

void * _midia5_init_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp, int device)
{
	(void)hp;
	(void)device;
	return NULL;
}

void _midia5_free_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp)
{
	(void)hp;
}

void _midia5_platform_send_data(MIDIA5_OUTPUT_HANDLE * hp, int data)
{
	(void)hp;
	(void)data;
}

void _midia5_platform_reset_output_device(MIDIA5_OUTPUT_HANDLE * hp)
{
	(void)hp;
}

bool _midia5_platform_set_output_gain(MIDIA5_OUTPUT_HANDLE * hp, float gain)
{
	(void)hp;
	(void)gain;
	return false;
}

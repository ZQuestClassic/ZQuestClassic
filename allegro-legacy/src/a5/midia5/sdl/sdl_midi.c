#include <SDL2/SDL_mixer.h>
// #include <timidity.h>

// entire file is a local edit
// also, it's not implemented

#include "../midia5.h"

int _midia5_get_platform_output_device_count(void)
{
	return 1;
}

const char * _midia5_get_platform_output_device_name(int device)
{
	return "SDL Midi";
}

void * _midia5_init_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp, int device)
{
	return NULL;
}

void _midia5_free_output_platform_data(MIDIA5_OUTPUT_HANDLE * hp)
{
}

void _midia5_platform_send_data(MIDIA5_OUTPUT_HANDLE * hp, int data)
{
}

void _midia5_platform_reset_output_device(MIDIA5_OUTPUT_HANDLE * hp)
{
}

bool _midia5_platform_set_output_gain(MIDIA5_OUTPUT_HANDLE * hp, float gain)
{
	return false;
}

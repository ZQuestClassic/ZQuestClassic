#ifndef MIDIA5_H
#define MIDIA5_H

#include <allegro5/allegro5.h>

typedef struct
{

	/* midi event velocities will be multiplied by this during input/output */
	float gain;
	void * platform_data;

} MIDIA5_OUTPUT_HANDLE;

int midia5_get_output_device_count(void);
const char * midia5_get_output_device_name(int device);

MIDIA5_OUTPUT_HANDLE * midia5_create_output_handle(int device);
void midia5_destroy_output_handle(MIDIA5_OUTPUT_HANDLE * hp);

void midia5_send_data(MIDIA5_OUTPUT_HANDLE * hp, int data);
void midia5_reset_output_device(MIDIA5_OUTPUT_HANDLE * hp);
bool midia5_set_output_gain(MIDIA5_OUTPUT_HANDLE * hp, float gain);

#endif

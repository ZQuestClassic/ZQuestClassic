#include "sound/zcmixer.h"
#include <allegro5/allegro_audio.h>
#include <algorithm>

ZCMIXER* zcmixer_create()
{
	ZCMIXER *mix = NULL;
	if ((mix = (ZCMIXER*)malloc(sizeof(ZCMIXER))) == NULL)
		return NULL;

	mix->fadeinframes = 0;
	mix->fadeinmaxframes = 0;
	mix->fadeindelay = 0;
	mix->fadeoutframes = 0;
	mix->fadeoutmaxframes = 0;

	mix->newtrack = NULL;
	mix->oldtrack = NULL;

	return mix;
}

void zcmixer_update(ZCMIXER* mix, int32_t basevol, int32_t uservol, bool oldscriptvol)
{
	if (mix == NULL)
		return;

	if (mix->fadeinframes)
	{
		if (mix->fadeindelay)
		{
			--mix->fadeindelay;
			if (mix->newtrack != NULL)
			{
				zcmusic_play(mix->newtrack, 0);
			}
		}
		else
		{
			--mix->fadeinframes;
			if (mix->newtrack != NULL)
			{
				int32_t pct = std::clamp(int32_t((uint64_t(mix->fadeinframes) * 10000) / uint64_t(mix->fadeinmaxframes)), 0, 10000);
				mix->newtrack->fadevolume = 10000 - pct;
				int32_t temp_volume = basevol;
				if (!oldscriptvol)
					temp_volume = (basevol * uservol) / 10000 / 100;
				temp_volume = (temp_volume * mix->newtrack->fadevolume) / 10000;
				zcmusic_play(mix->newtrack, temp_volume);
				if (mix->fadeinframes == 0)
				{
					mix->newtrack->fadevolume = 10000;
				}
			}
		}
	}
	if(mix->fadeoutframes)
	{
		if (mix->fadeoutdelay)
			--mix->fadeoutdelay;
		else
			--mix->fadeoutframes;
		if (mix->oldtrack != NULL)
		{
			int32_t pct = 0;
			if(mix->fadeoutframes > 0)
				pct = std::clamp(int32_t((uint64_t(mix->fadeoutframes) * 10000) / uint64_t(mix->fadeoutmaxframes)), 0, 10000);
			mix->oldtrack->fadevolume = pct;
			int32_t temp_volume = basevol;
			if (!oldscriptvol)
				temp_volume = (basevol * uservol) / 10000 / 100;
			temp_volume = (temp_volume * mix->oldtrack->fadevolume) / 10000;
			zcmusic_play(mix->oldtrack, temp_volume);
		}
		if (mix->fadeoutframes == 0)
		{
			zcmusic_stop(mix->oldtrack);
			zcmusic_unload_file(mix->oldtrack);
		}
	}
}

void zcmixer_exit(ZCMIXER* &mix)
{
	if (mix == NULL) 
		return;

	zcmusic_unload_file(mix->oldtrack);
	// newtrack is just zcmusic

	free(mix);
	mix = NULL;
}

#include "music_playback.h"
#include "sound/zcmusic.h"
#include "sound/zcmixer.h"
#include "base/general.h"
#include "play_midi.h"
#include "base/qrs.h"
#include "zc/zc_sys.h"

extern ZCMUSIC* zcmusic;
extern ZCMIXER* zcmixer;

// Run an NSF, or a MIDI if the NSF is missing somehow.
bool try_zcmusic(const char* filename, char* path, int32_t track, int32_t midi, int32_t vol, int32_t fadeoutframes)
{
	ZCMUSIC* newzcmusic = zcmusic_load_for_quest(filename, path);

	// Found it
	if (newzcmusic != NULL)
	{
		newzcmusic->fadevolume = 10000;
		newzcmusic->fadeoutframes = fadeoutframes;

		zcmixer->newtrack = newzcmusic;

		zcmusic_stop(zcmusic);
		zcmusic_unload_file(zcmusic);
		zc_stop_midi();

		zcmusic = newzcmusic;
		zcmusic_play(zcmusic, vol);

		if (track > 0)
			zcmusic_change_track(zcmusic, track);

		return true;
	}
	#ifndef IS_EDITOR
	// Not found, play MIDI - unless this was called by a script (yay, magic numbers)
	else if (midi > -1000)
		jukebox(midi);
	#endif

	return false;
}

bool play_enh_music_crossfade(char* name, char* path, int32_t track, int32_t vol, int32_t fadeinframes, int32_t fadeoutframes, int32_t fademiddleframes, int32_t startpos, bool revertonfail)
{
	double fadeoutpct = 1.0;
	// If there was an old fade going, use that as a multiplier for the new fade out
	if (zcmixer->newtrack != NULL)
	{
		fadeoutpct = double(zcmixer->newtrack->fadevolume) / 10000.0;
	}

	ZCMUSIC* oldold = zcmixer->oldtrack;
	bool ret = false;

	if (name == NULL)
	{
		// Pass currently playing music off to the mixer
		zcmixer->oldtrack = zcmusic;
		// Do not play new music
		zcmusic = NULL;
		zcmixer->newtrack = NULL;

		zcmixer->fadeinframes = fadeinframes;
		zcmixer->fadeinmaxframes = fadeinframes;
		zcmixer->fadeoutframes = zc_max(fadeoutframes * fadeoutpct, 1);
		zcmixer->fadeoutmaxframes = fadeoutframes;
		if (fademiddleframes < 0)
		{
			zcmixer->fadeindelay = 0;
			zcmixer->fadeoutdelay = -fademiddleframes;
		}
		else
		{
			zcmixer->fadeindelay = fademiddleframes;
			zcmixer->fadeoutdelay = 0;
		}
		if (zcmixer->oldtrack != NULL)
			zcmixer->oldtrack->fadevolume = 10000;
		if (zcmixer->newtrack != NULL)
			zcmixer->newtrack->fadevolume = 0;
	}
	else // Pointer to a string..
	{
		// Pass currently playing music to the mixer
		zcmixer->oldtrack = zcmusic;
		zcmusic = NULL;
		zcmixer->newtrack = NULL;

		ret = try_zcmusic(name, path, track, -1000, vol, fadeoutframes);
		// If new music was found
		if (ret)
		{
			// New music fades in
			if (zcmusic != NULL)
				zcmusic->fadevolume = 0;

			zcmixer->newtrack = zcmusic;
			zcmixer->fadeinframes = fadeinframes;
			zcmixer->fadeinmaxframes = fadeinframes;
			zcmixer->fadeoutframes = zc_max(fadeoutframes * fadeoutpct, 1);
			zcmixer->fadeoutmaxframes = fadeoutframes;
			if (fademiddleframes < 0)
			{
				zcmixer->fadeindelay = 0;
				zcmixer->fadeoutdelay = -fademiddleframes;
			}
			else
			{
				zcmixer->fadeindelay = fademiddleframes;
				zcmixer->fadeoutdelay = 0;
			}
			if (startpos > 0)
				zcmusic_set_curpos(zcmixer->newtrack, startpos);
			if (zcmixer->oldtrack != NULL)
				zcmixer->oldtrack->fadevolume = 10000;
			if (zcmixer->newtrack != NULL)
				zcmixer->newtrack->fadevolume = 0;
		}
		else if (revertonfail)
		{
			// Switch back to the old music
			zcmusic = zcmixer->oldtrack;
			zcmixer->newtrack = NULL;
			zcmixer->oldtrack = NULL;
		}
	}

	// If there was already an old track playing, stop it
	if (oldold != NULL)
	{
		// Don't allow it to null both tracks if running twice in a row
		if (zcmixer->newtrack == NULL && zcmixer->oldtrack == NULL)
		{
			zcmixer->oldtrack = oldold;

			if (oldold->fadeoutframes > 0)
			{
				zcmixer->fadeoutframes = zc_max(oldold->fadeoutframes * fadeoutpct, 1);
				zcmixer->fadeoutmaxframes = oldold->fadeoutframes;
				if (zcmixer->oldtrack != NULL)
					zcmixer->oldtrack->fadevolume = 10000;
				oldold->fadeoutframes = 0;
			}
		}
		else
		{
			zcmusic_stop(oldold);
			zcmusic_unload_file(oldold);
			oldold = NULL;
		}
	}

	return ret;
}

#include "zc/scripting/types/audio.h"

#include "advanced_music.h"
#include "base/check.h"
#include "components/zasm/defines.h"
#include "core/qrs.h"
#include "zc/ffscript.h"
#include "zc/scripting/arrays.h"
#include "zc/zelda.h"

namespace {

void do_setMIDI_volume(int32_t m)
{
	master_volume(-1,(vbound(m,0,255)));
}
void do_setMusic_volume(int32_t m)
{
	emusic_volume = vbound(m,0,255);
}
void do_setDIGI_volume(int32_t m)
{
	master_volume((vbound(m,0,255)),-1);
}
void do_setSFX_volume(int32_t m)
{
	sfx_volume = m;
}
void do_setSFX_pan(int32_t m)
{
	pan_style = vbound(m,0,3);
}
int32_t do_getMIDI_volume()
{
	return ((int32_t)midi_volume);
}
int32_t do_getMusic_volume()
{
	return ((int32_t)emusic_volume);
}
int32_t do_getDIGI_volume()
{
	return ((int32_t)digi_volume);
}
int32_t do_getSFX_volume()
{
	return ((int32_t)sfx_volume);
}
int32_t do_getSFX_pan()
{
	return ((int32_t)pan_style);
}

}

int32_t audio_get_register(int32_t reg)
{
	int32_t ret = 0;

	switch (reg)
	{
		case AUDIOPAN:
		{
			ret = do_getSFX_pan() * 10000;
			break;
		}
		case ENGINE_MUSIC_ACTIVE:
			ret = engine_music_active ? 10000 : 0;
			break;
		case GETMIDI:
			ret=(currmidi-MIDIOFFSET_ZSCRIPT)*10000;
			break;
		case MUSICUPDATECOND:
		{
			ret = ((byte)FFCore.music_update_cond) * 10000;
			break;
		}
		case NUM_MUSICS:
			ret = quest_music.size() * 10000;
			break;

		default:
			NOTREACHED();
	}

	return ret;
}

void audio_set_register(int32_t reg, int32_t value)
{
	switch (reg)
	{
		case AUDIOPAN:
		{
			if ( !(FFCore.coreflags&FFCORE_SCRIPTED_PANSTYLE) ) 
			{
				FFCore.usr_panstyle = do_getSFX_pan();
				FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_PANSTYLE,true);
			}
			do_setSFX_pan(value/10000);
			break;
		}
		case MUSICUPDATECOND:
		{
			FFCore.music_update_cond = vbound(value / 10000, 0, 255);
			break;
		}

		default:
			NOTREACHED();
	}
}

// Audio arrays.

static ArrayRegistrar AUDIOVOLUME_registrar(AUDIOVOLUME, []{
	static ScriptingArray_GlobalComputed<int> impl(
		[](int){ return 4; },
		[](int, int index){
			switch (index)
			{
				case 0: //midi volume
					return do_getMIDI_volume();
				case 1: //digi volume
					return do_getDIGI_volume();
				case 2: //emh music volume
					return do_getMusic_volume();
				case 3: //sfx volume
					return do_getSFX_volume();
				default: NOTREACHED();
			}
		},
		[](int, int index, int value){
			if (!get_qr(qr_OLD_SCRIPT_VOLUME))
				return false;

			switch (index)
			{
				case 0: //midi volume
				{
					if ( !(FFCore.coreflags&FFCORE_SCRIPTED_MIDI_VOLUME) ) 
					{
						FFCore.usr_midi_volume = do_getMIDI_volume();
						FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_MIDI_VOLUME,true);
					}
					do_setMIDI_volume(value);
					break;
				}
				case 1: //digi volume
				{
					if ( !(FFCore.coreflags&FFCORE_SCRIPTED_DIGI_VOLUME) ) 
					{
						FFCore.usr_digi_volume = do_getDIGI_volume();
						FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_DIGI_VOLUME,true);
					}
					do_setDIGI_volume(value);
					break;
				}
				case 2: //emh music volume
				{
					if ( !(FFCore.coreflags&FFCORE_SCRIPTED_MUSIC_VOLUME) ) 
					{
						FFCore.usr_music_volume = do_getMusic_volume();
						FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_MUSIC_VOLUME,true);
					}
					do_setMusic_volume(value);
					break;
				}
				case 3: //sfx volume
				{
					if ( !(FFCore.coreflags&FFCORE_SCRIPTED_SFX_VOLUME) ) 
					{
						FFCore.usr_sfx_volume = do_getSFX_volume();
						FFCore.SetFFEngineFlag(FFCORE_SCRIPTED_SFX_VOLUME,true);
					}
					do_setSFX_volume(value);
					break;
				}
				default: NOTREACHED();
			}

			return true;
		}
	);
	impl.compatSetDefaultValue(-10000);
	impl.setMul10000(true);
	return &impl;
}());

static ArrayRegistrar MUSICUPDATEFLAGS_registrar(MUSICUPDATEFLAGS, []{
	static ScriptingArray_GlobalComputed<bool> impl(
		[](int){ return 2; },
		[](int, int index) -> bool {
			return (FFCore.music_update_flags >> index) & 1;
		},
		[](int, int index, bool value){
			SETFLAG(FFCore.music_update_flags, 1 << index, value);
			return true;
		}
	);
	impl.setMul10000(true);
	return &impl;
}());

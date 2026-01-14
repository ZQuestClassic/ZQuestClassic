#include "zc/scripting/types/musicdata.h"
#include "zc/scripting/arrays.h"

void playLevelMusic();
void music_stop();

extern refInfo *ri;
extern int32_t sarg1;
extern int32_t sarg2;
extern int32_t sarg3;

AdvancedMusic* checkMusic(int32_t ref, bool skipError)
{
	if (unsigned(ref-1) >= quest_music.size())
	{
		if (!skipError)
			scripting_log_error_with_context("Invalid {} using UID = {}", "musicdata", ref);
		return nullptr;
	}
	return &quest_music[ref-1];
}

std::optional<int32_t> musicdata_get_register(int32_t reg)
{
	int32_t ret = 0;
	int ref = GET_REF(musicref);
	switch (reg)
	{
		case MUSICDATA_ID:
		{
			ret = ref * 10000; // intentionally lacks checks; valid to call on NULL pointers and special pointers
			break;
		}
		case MUSICDATA_IS_ACTIVE:
		{
			if (ref == 0)
				ret = 0;
			else if (auto* amus = checkMusic(ref))
				ret = amus->is_playing() ? 10000 : 0; // intentionally lacks checks; valid to call on NULL pointers and special pointers
			break;
		}
		case MUSICDATA_MIDI:
		{
			if (ref == 0)
				break; // return w/o error message
			if (auto* amus = checkMusic(ref))
				ret = amus->midi * 10000L;
			break;
		}
		case MUSICDATA_IS_ENHANCED:
		{
			if (ref == 0)
				break;
			if (auto* amus = checkMusic(ref))
				ret = amus->enhanced.is_empty() ? 0 : 10000L;
			break;
		}
		case MUSICDATA_TRACK:
		{
			if (ref == 0)
				break; // return w/o error message
			if (auto* amus = checkMusic(ref))
				ret = amus->enhanced.track * 10000L;
			break;
		}
		case MUSICDATA_LOOPSTART:
		{
			if (ref == 0)
				break; // return w/o error message
			if (auto* amus = checkMusic(ref))
				ret = amus->enhanced.loop_start;
			break;
		}
		case MUSICDATA_LOOPEND:
		{
			if (ref == 0)
				break; // return w/o error message
			if (auto* amus = checkMusic(ref))
				ret = amus->enhanced.loop_end;
			break;
		}
		case MUSICDATA_XFADEIN:
		{
			if (ref == 0)
				break; // return w/o error message
			if (auto* amus = checkMusic(ref))
				ret = amus->enhanced.xfade_in * 10000L;
			break;
		}
		case MUSICDATA_XFADEOUT:
		{
			if (ref == 0)
				break; // return w/o error message
			if (auto* amus = checkMusic(ref))
				ret = amus->enhanced.xfade_out * 10000L;
			break;
		}

		default: return std::nullopt;
	}

	return ret;
}
bool musicdata_set_register(int32_t reg, int32_t value)
{
	int ref = GET_REF(musicref);
	switch (reg)
	{
		case MUSICDATA_MIDI:
		{
			if (auto* amus = checkMusic(ref))
			{
				auto v = value / 10000;
				if (amus->midi != v)
				{
					amus->midi = v;
					if (amus->is_playing() && !zcmusic) // changing currently-playing midi
					{
						bool ema = engine_music_active;
						amus->play();
						engine_music_active = ema;
					}
				}
			}
			break;
		}
		case MUSICDATA_TRACK:
		{
			if (auto* amus = checkMusic(ref))
			{
				auto v = value / 10000;
				if (amus->enhanced.track != v)
				{
					amus->enhanced.track = v;
					if (amus->is_playing() && zcmusic) // changing currently-playing track
					{
						bool ema = engine_music_active;
						amus->play();
						engine_music_active = ema;
					}
				}
			}
			break;
		}
		case MUSICDATA_LOOPSTART:
		{
			if (auto* amus = checkMusic(ref))
			{
				if (amus->enhanced.loop_start != value)
				{
					amus->enhanced.loop_start = value;
					if (amus->is_playing() && zcmusic) // changing currently-playing loop
						zcmusic_set_loop(zcmusic, (amus->enhanced.loop_start / 10000.0), (amus->enhanced.loop_end / 10000.0));
				}
			}
			break;
		}
		case MUSICDATA_LOOPEND:
		{
			if (auto* amus = checkMusic(ref))
			{
				if (amus->enhanced.loop_end != value)
				{
					amus->enhanced.loop_end = value;
					if (amus->is_playing() && zcmusic) // changing currently-playing loop
						zcmusic_set_loop(zcmusic, (amus->enhanced.loop_start / 10000.0), (amus->enhanced.loop_end / 10000.0));
				}
			}
			break;
		}
		case MUSICDATA_XFADEIN:
		{
			if (auto* amus = checkMusic(ref))
				amus->enhanced.xfade_in = value / 10000;
			break;
		}
		case MUSICDATA_XFADEOUT:
		{
			if (auto* amus = checkMusic(ref))
			{
				auto v = value / 10000;
				if (amus->enhanced.xfade_out != v)
				{
					amus->enhanced.xfade_out = v;
					if (amus->is_playing() && zcmusic) // changing currently-playing fadeout
						zcmusic->fadeoutframes = v;
				}
			}
			break;
		}
		
		default: return false;
	}
	return true;
}
std::optional<int32_t> musicdata_run_command(word command)
{
	extern ScriptType curScriptType;
	extern word curScriptNum;
	extern int32_t curScriptIndex;

	ScriptType type = curScriptType;
	int32_t i = curScriptIndex;
	int ref = command == PLAY_MUSIC ? get_register(sarg1) : GET_REF(musicref);
	switch (command)
	{
		case MUSICDATA_GETPATH:
		{
			string str;
			if (ref != 0) // ref 0 will return an empty path without error
			{
				if (auto* amus = checkMusic(ref))
					str = amus->enhanced.path;
				else break;
			}
			auto arrayptr = get_register(sarg1);
			if (ArrayH::setArray(arrayptr, str, true) == SH::_Overflow)
				scripting_log_error_with_context("Array supplied not large enough, and couldn't be resized!");
			break;
		}
		case MUSICDATA_GETNAME:
		{
			string str;
			if (ref != 0) // ref 0 will return an empty name without error
			{
				if (auto* amus = checkMusic(ref))
					str = amus->name;
				else break;
			}
			auto arrayptr = get_register(sarg1);
			if (ArrayH::setArray(arrayptr, str, true) == SH::_Overflow)
				scripting_log_error_with_context("Array supplied not large enough, and couldn't be resized!");
			break;
		}
		case MUSICDATA_SETPATH:
		{
			if (auto* amus = checkMusic(ref))
			{
				auto arrayptr = get_register(sarg1);
				string newpath;
				ArrayH::getString(arrayptr, newpath);
				if (newpath != amus->enhanced.path)
				{
					amus->enhanced.path = newpath;
					if (amus->is_playing() && (zcmusic || !newpath.empty())) // changing enhanced music of currently playing music
					{
						bool ema = engine_music_active;
						amus->play();
						engine_music_active = ema;
					}
				}
			}
			break;
		}
		case MUSICDATA_SETNAME:
		{
			if (auto* amus = checkMusic(ref))
			{
				auto arrayptr = get_register(sarg1);
				ArrayH::getString(arrayptr, amus->name, 512);
			}
			break;
		}
		case PLAY_MUSIC:
		{
			if (ref == 0) // null kills music
				music_stop();
			else if (ref == -1) // -1 plays current engine music directly
				playLevelMusic();
			else if (auto* amus = checkMusic(ref))
				amus->play();
			break;
		}
		
		default: return std::nullopt;
	}

	return RUNSCRIPT_OK;
}

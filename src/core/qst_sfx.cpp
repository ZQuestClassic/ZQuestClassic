#include "core/qrs.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zalleg/zalleg.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

const char *old_sfx_string[Z35] =
{
    "Arrow", "Sword beam", "Bomb blast", "Boomerang",  "Subscreen cursor",
    "Shield is hit", "Item chime",  "Roar (Dodongo, Gohma)", "Shutter", "Enemy dies",
    "Enemy is hit", "Low hearts warning", "Fire", "Ganon's fanfare", "Boss is hit", "Hammer",
    "Hookshot", "Message", "Hero is hit", "Item fanfare", "Bomb placed", "Item pickup",
    "Refill", "Roar (Aquamentus, Gleeok, Ganon)", "Item pickup 2", "Ocean ambience",
    "Secret chime", "Hero dies", "Stairs", "Sword", "Roar (Manhandla, Digdogger, Patra)",
    "Wand magic", "Whistle", "Zelda's fanfare", "Charging weapon", "Charging weapon 2",
    "Divine Fire", "Enemy falls from ceiling", "Divine Escape", "Fireball", "Tall Grass slashed",
    "Pound pounded", "Hover Boots", "Ice magic", "Jump", "Lens of Truth off", "Lens of Truth on",
    "Divine Protection shield", "Divine Protection 2", "Push block", "Rock", "Spell rocket down",
    "Spell rocket up", "Sword spin attack", "Splash", "Summon magic", "Sword tapping",
    "Sword tapping (secret)", "Whistle whirlwind", "Cane of Byrna orbit"
};

namespace {

int32_t readsfx_old(PACKFILE *f, word s_version)
{
	int32_t dummy;
	//int32_t ret;
	const size_t WAV_COUNT = 256;
	SAMPLE samples[WAV_COUNT] = {0};
	
	/* HIGHLY UNORTHODOX UPDATING THING, by L
	 * This fixes quests made before revision 411 (such as the 'Lost Isle Build'),
	 * where the meaning of GOTOLESS changed. It also coincided with V_SFX
	 * changing from 1 to 2.
	 */
	if(s_version < 2)
		set_qr(qr_GOTOLESSNOTEQUAL,1);
		
	/* End highly unorthodox updating thing */
	
	int32_t wavcount = WAV_COUNT;
	
	if(s_version < 6)
		wavcount = 128;
		
	uint8_t tempflag[WAV_COUNT>>3];
	
	if(s_version < 4)
	{
		memset(tempflag, 0xFF, WAV_COUNT>>3);
	}
	else
	{
		if(s_version < 6)
			memset(tempflag, 0, WAV_COUNT>>3);
			
		for(int32_t i=0; i<(wavcount>>3); i++)
		{
			p_getc(&tempflag[i], f);
		}
		
	}
	
	string sfx_string[256];
	if(s_version>4)
	{
		for(int32_t i=1; i<256; i++)
		{
			if(i < Z35)
				sfx_string[i] = old_sfx_string[i-1];
			else sfx_string[i] = fmt::format("s{:03}", i);
			
			if(i>=wavcount)
				continue;
			if(get_bit(tempflag, i-1))
			{
				char tempname[37];
				tempname[sizeof(tempname) - 1] = '\0';

				if(!pfread(tempname, 36, f))
				{
					return qe_invalid;
				}

				sfx_string[i] = tempname;
			}
			else
			{
				if(i < Z35)
					sfx_string[i] = old_sfx_string[i-1];
				else sfx_string[i] = fmt::format("s{:03}", i);
			}
		}
	}
	else
	{
		for(int32_t i=1; i<256; i++)
		{
			if(i < Z35)
				sfx_string[i] = old_sfx_string[i-1];
			else sfx_string[i] = fmt::format("s{:03}", i);
		}
	}
	
	//finally...  section data
	for(int32_t i=1; i<wavcount; i++)
	{
		SAMPLE& temp_sample = samples[i];
		temp_sample.loop_start = 0;
		temp_sample.loop_end = 0;
		temp_sample.param = 0;
		temp_sample.data = nullptr;
		if(get_bit(tempflag, i-1))
		{
			if(!p_igetl(&dummy,f))
			{
				return qe_invalid;
			}
			
			(temp_sample.bits) = dummy;
			
			if(!p_igetl(&dummy,f))
			{
				return qe_invalid;
			}
			
			(temp_sample.stereo) = dummy;
			
			if(!p_igetl(&dummy,f))
			{
				return qe_invalid;
			}
			
			(temp_sample.freq) = dummy;
			
			if(!p_igetl(&dummy,f))
			{
				return qe_invalid;
			}
			
			(temp_sample.priority) = dummy;
			
			if(!p_igetl(&(temp_sample.len),f))
			{
				return qe_invalid;
			}
			
			if(!p_igetl(&(temp_sample.loop_start),f))
			{
				return qe_invalid;
			}
			
			if(!p_igetl(&(temp_sample.loop_end),f))
			{
				return qe_invalid;
			}
			
			if(!p_igetl(&(temp_sample.param),f))
			{
				return qe_invalid;
			}

			auto len = (temp_sample.bits==8?1:2)*(temp_sample.stereo==0?1:2)*temp_sample.len;
#ifdef ZC_FUZZ
	const int32_t sfx_limit = 100000;
			if (len < 0 || len > sfx_limit)
			{
				return qe_invalid;
			}
#endif
			temp_sample.data = calloc(len,1);
			
			if(s_version < 3)
				len = (temp_sample.bits==8?1:2)*temp_sample.len;
				
			//old-style, non-portable loading (Bad Allegro! Bad!!) -DD
			if(s_version < 2)
			{
				if(!pfread(temp_sample.data, len,f))
				{
					return qe_invalid;
				}
			}
			else
			{
				//re-endianfy the data
				int32_t wordstoread = len / sizeof(word);
				
				for(int32_t j=0; j<wordstoread; j++)
				{
					word temp;
					
					if(!p_igetw(&temp, f))
					{
						return qe_invalid;
					}
					
					((word *)temp_sample.data)[j] = temp;
				}
			}
		}
		else if(i < Z35)
		{
			SAMPLE* datsamp = (SAMPLE*)(sfxdata[i].dat);
			memcpy(&temp_sample, datsamp, sizeof(SAMPLE));
			set_bit(tempflag, i-1, 1);
			int32_t len = (temp_sample.bits==8?1:2)*(temp_sample.stereo==0?1:2)*temp_sample.len;
			temp_sample.data = calloc(len,1);
			memcpy(temp_sample.data, datsamp->data, len);
		}
		else continue;
		
		
		// unsure how to handle this, or if it needs to be handled?
		// int32_t len2 = (temp_sample.bits==8?1:2)*(temp_sample.stereo==0?1:2)*temp_sample.len;
		// samples[i].data = calloc(len2,1);
		// int32_t cpylen = len2;
		// if(s_version<3)
		// {
			// cpylen = (temp_sample.bits==8?1:2)*temp_sample.len;
			// al_trace("WARNING: Quest SFX %d is in stereo, and may be corrupt.\n",i);
		// }
		// memcpy(samples[i].data,temp_sample.data,cpylen);
		// free(temp_sample.data);
	}
	
	int blanks = 0;
	quest_sounds.reserve(wavcount);
	for (uint q = 1; q < wavcount; ++q)
	{
		if (get_bit(tempflag, q - 1))
		{
			while (quest_sounds.size() < q)
				quest_sounds.emplace_back();
			auto& sound = quest_sounds[q - 1];
			sound = ZCSFX(samples[q]);
			if (sound.is_invalid())
				return qe_invalid;
			sound.sfx_name = sfx_string[q];
		}
		else if (q - 1 < quest_sounds.size())
			quest_sounds[q - 1].clear();
	}
	quest_sounds.shrink_to_fit();
	
	sfxdat = 0;
	return 0;
}

} // end namespace

void setupsfx()
{
	quest_sounds.clear();
	quest_sounds.reserve(Z35-1);
    for(size_t q = 1; q < Z35; ++q)
    {
		SAMPLE *temp_sample = (SAMPLE *)sfxdata[q].dat;
		auto& sound = quest_sounds.emplace_back(*temp_sample);
		sound.sfx_name = old_sfx_string[q-1];
    }
}

int32_t readsfx(PACKFILE *f, zquestheader *)
{
	int32_t dummy;
	word s_version = 0;
	
	if(!p_igetw(&s_version,f))
		return qe_invalid;

	if (s_version > V_SFX)
		return qe_version;
	
	FFCore.quest_format[vSFX] = s_version;
	
	if(!read_deprecated_section_cversion(f))
		return qe_invalid;
	
	//section size
	if(!p_igetl(&dummy,f))
		return qe_invalid;
	
	if (s_version < 9)
		return readsfx_old(f, s_version);
	
	word count;
	if (!p_igetw(&count, f))
		return qe_invalid;
	
	quest_sounds.clear();
	quest_sounds.reserve(count);
	for (word q = 0; q < count; ++q)
	{
		ZCSFX& sound = quest_sounds.emplace_back();
		if (auto ret = sound.read(f, s_version))
			return ret;
	}
	
	sfxdat = 0;
	return 0;
}

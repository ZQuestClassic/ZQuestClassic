#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern const char *old_weapon_string[wLast];

namespace {

void reset_weaponname(int32_t i)
{
	sprite_data_buf[i].name = i < wLast
		? old_weapon_string[i]
		: fmt::format("zz{:03}", i);
}

} // end namespace

int32_t read_single_spritedata(PACKFILE *f, zquestheader *Header, word s_version, word index)
{
	static sprite_data _nil_sprite;
	
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_weapons);
	sprite_data& sprite_ref = should_skip ? _nil_sprite : sprite_data_buf[index];
	string oldname = sprite_ref.name;
	sprite_ref = sprite_data();
	
	byte tempbyte;
	word tempword;
	if (s_version >= 9)
	{
		if (!p_getcstr(&sprite_ref.name, f))
			return qe_invalid;
	}
	else sprite_ref.name = oldname;
	
	if (s_version < 8)
	{
		if (!p_igetw(&tempword, f))
			return qe_invalid;
		if (s_version < 7)
			sprite_ref.tile = tempword;
	}

	if(!p_getc(&sprite_ref.misc,f))
		return qe_invalid;
	
	if(!p_getc(&sprite_ref.csets,f))
		return qe_invalid;
	
	if(!p_getc(&sprite_ref.frames,f))
		return qe_invalid;
	
	if(!p_getc(&sprite_ref.speed,f))
		return qe_invalid;
	
	if(!p_getc(&sprite_ref.type,f))
		return qe_invalid;

	if ( s_version >= 7 )
	{
		if(!p_igetw(&sprite_ref.script,f))
			return qe_invalid;
		if(!p_igetl(&sprite_ref.tile,f))
			return qe_invalid;
	}
	
	if(Header->zelda_version < 0x193)
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
	
	if(s_version < 6)
		SETFLAG(sprite_ref.misc, WF_BEHIND, index == ewFIRETRAIL);
	return 0;
}

int32_t readweapons(PACKFILE *f, zquestheader *Header)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_weapons);

    word weapons_to_read=256;
    int32_t dummy;
    word s_version=0;
    
    
    if(Header->zelda_version < 0x186)
    {
        weapons_to_read=64;
    }
    
    if(Header->zelda_version < 0x185)
    {
        weapons_to_read=32;
    }
    
    if(Header->zelda_version > 0x192)
    {
        weapons_to_read=0;
        
        //section version info
        if(!p_igetw(&s_version,f))
        {
            return qe_invalid;
        }

		if (s_version > V_WEAPONS)
			return qe_version;
	
	FFCore.quest_format[vWeaponSprites] = s_version;
        
        if(!read_deprecated_section_cversion(f))
        {
            return qe_invalid;
        }
        
        //section size
        if(!p_igetl(&dummy,f))
        {
            return qe_invalid;
        }
        
        //finally...  section data
        if(!p_igetw(&weapons_to_read,f))
        {
            return qe_invalid;
        }

        if (weapons_to_read > MAXSPRITES)
        {
            return qe_invalid;
        }
    }
    
	if (!should_skip)
	{
		sprite_data_buf.clear();
		sprite_data_buf.reserve(weapons_to_read);
	}
	if (s_version <= 2)
	{
		if (!should_skip)
			for(int32_t i=0; i<256; i++)
				reset_weaponname(i);
	}
    else if(s_version < 9)
    {
        for(int32_t i=0; i<weapons_to_read; i++)
        {
            char tempname[65] = {0};
            
            if(!pfread(tempname, 64, f))
            {
                return qe_invalid;
            }
            
			if (!should_skip)
				sprite_data_buf[i].name = tempname;
        }
        
		if (!should_skip)
		{
			if(s_version<4)
			{
				sprite_data_buf[iwHover].name = old_weapon_string[iwHover];
				sprite_data_buf[wFIREMAGIC].name = old_weapon_string[wFIREMAGIC];
			}
			
			if(s_version<5)
			{
				sprite_data_buf[iwQuarterHearts].name = old_weapon_string[iwQuarterHearts];
			}
        }
    }
    
	for(int32_t i=0; i<weapons_to_read; i++)
	{
		if (auto ret = read_single_spritedata(f, Header, s_version, i))
			return ret;
	}

	if (should_skip)
		return 0;
    
	if(s_version<2)
	{
		sprite_data_buf[wSBOOM] = sprite_data_buf.get(wBOOM);
	}
	
	if(s_version<5)
	{
		sprite_data_buf[iwQuarterHearts].tile = 1;
		sprite_data_buf[iwQuarterHearts].csets = 1;
	}
	
	if(Header->zelda_version < 0x176)
	{
		auto& spawn_item = itemsbuf[iMisc1];
		auto& death_item = itemsbuf[iMisc2];
		sprite_data_buf[iwSpawn].load_item(spawn_item);
		sprite_data_buf[iwDeath].load_item(death_item);
		spawn_item.clear();
		death_item.clear();
	}
	
	if((Header->zelda_version < 0x192)||
			((Header->zelda_version == 0x192)&&(Header->build<129)))
	{
		sprite_data_buf[wHSCHAIN_V] = sprite_data_buf.get(wHSCHAIN_H);
	}
	
	if((Header->zelda_version < 0x210))
	{
		sprite_data_buf[wLSHEAD] = sprite_data_buf.get(wHSHEAD);
		sprite_data_buf[wLSCHAIN_H] = sprite_data_buf.get(wHSCHAIN_H);
		sprite_data_buf[wLSHANDLE] = sprite_data_buf.get(wHSHANDLE);
		sprite_data_buf[wLSCHAIN_V] = sprite_data_buf.get(wHSCHAIN_V);
	}
	
	sprite_data_buf.normalize();
    
    return 0;
}

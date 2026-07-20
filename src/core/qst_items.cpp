#include "core/qrs.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern dword loading_tileset_flags;
extern const char *old_item_string[iLast];
extern byte deprecated_rules[QUESTRULES_NEW_SIZE];

namespace {

void reset_itemname(int32_t id)
{
	if(id < iLast)
		itemsbuf[id].name = old_item_string[id];
	else
		itemsbuf[id].name = fmt::format("zz{:03}", id);
}

int32_t read_single_item_old(PACKFILE *f, word s_version, word index, word version, word build)
{
    byte padding, tempbyte;
    word dummy_word;
	dword tempdword;
	
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_items);
	itemdata tempitem = itemdata();
	reset_itembuf(&tempitem, index);
	tempitem.name = itemsbuf.get(index).name;
	
	if ( s_version > 35 ) //expanded tiles	
	{    
		if(!p_igetl(&tempitem.tile,f))
		{
			return qe_invalid;
		}
	}
	else
	{
		if(!p_igetw(&tempitem.tile,f))
		{
			return qe_invalid;
		}
	}
	
	if(!p_getc(&tempitem.misc_flags,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&tempitem.csets,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&tempitem.frames,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&tempitem.speed,f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&tempitem.delay,f))
	{
		return qe_invalid;
	}
	
	if(version < 0x193)
	{
		if(!p_getc(&padding,f))
		{
			return qe_invalid;
		}
		
		if((version < 0x192)||((version == 0x192)&&(build<186)))
		{
			if (should_skip)
				return 0;

			switch(index)
			{
			case iShield:
				tempitem.ltm=get_qr(qr_BSZELDA)?-12:10;
				break;
				
			case iMShield:
				tempitem.ltm=get_qr(qr_BSZELDA)?-6:-10;
				break;
				
			default:
				tempitem.ltm=0;
				break;
			}
			
			tempitem.count=-1;
			tempitem.flags=item_none;
			tempitem.wpn_sprites[0]=tempitem.wpn_sprites[1]=tempitem.wpn_sprites[2]=tempitem.wpn_sprites[2]=tempitem.pickup_hearts=
											tempitem.misc1=tempitem.misc2=tempitem.usesound=0;
			tempitem.type=0xFF;
			tempitem.playsound=WAV_SCALE;
			reset_itembuf(&tempitem,index);
			
			itemsbuf[index] = tempitem;
			
			return 0;
		}
	}
	
	if(!p_igetl(&tempitem.ltm,f))
	{
		return qe_invalid;
	}
	
	if(version < 0x193)
	{
		for(int32_t q=0; q<12; q++)
		{
			if(!p_getc(&padding,f))
			{
				return qe_invalid;
			}
		}
	}
	
	if(s_version>1)
	{
		if ( s_version >= 31 )
		{
			if(!p_igetl(&tempitem.type,f))
			{
				return qe_invalid;
			}    
		}
		else
		{		    
			if(!p_getc(&tempitem.type,f))
			{
				return qe_invalid;
			}
		}
		if(s_version < 16)
			if(tempitem.type == 0xFF)
				tempitem.type = itype_misc;
				
		if(!p_getc(&tempitem.level,f))
		{
			return qe_invalid;
		}
		
		if(s_version>5)
		{
			if(s_version>=31)
			{
				if(!p_igetl(&tempitem.power,f))
				{
					return qe_invalid;
				}
			}
			else
			{
				if(!p_getc(&tempitem.power,f))
				{
				return qe_invalid;
				}
			}
					
			//converted flags from 16b to 32b -Z
			if ( s_version < 41 )
			{
				if(!p_igetw(&tempitem.flags,f))
				{
					return qe_invalid;
				}
			}
			else
			{
				if(!p_igetl(&tempitem.flags,f))
				{
					return qe_invalid;
				}
			}
		}
		else
		{
			//tempitem.power = tempitem.fam_type;
			char tempchar;
			
			if(!p_getc(&tempchar,f))
			{
				return qe_invalid;
			}
			
			if (tempchar) tempitem.flags |= item_gamedata;
		}
		
		if(!p_igetw(&tempitem.scrconfig.script,f))
		{
			return qe_invalid;
		}
		
		if(s_version<=3)
		{
			if(tempitem.scrconfig.script > NUMSCRIPTITEM)
				tempitem.scrconfig.script = 0;
		}
		
		if(!p_getc(&tempitem.count,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&tempitem.amount,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&tempitem.collect_scrconfig.script,f))
		{
			return qe_invalid;
		}
		
		if(s_version<=3)
		{
			if(tempitem.collect_scrconfig.script > NUMSCRIPTITEM)
				tempitem.collect_scrconfig.script = 0;
		}
		
		if(!p_igetw(&tempitem.setmax,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&tempitem.max,f))
		{
			return qe_invalid;
		}
		
		if (s_version >= 66)
		{
			if(!p_igetw(&tempitem.playsound,f))
				return qe_invalid;
		}
		else
		{
			if(!p_getc(&tempbyte,f))
				return qe_invalid;
			tempitem.playsound = tempbyte;
		}
		
		for(int32_t j=0; j<8; j++)
		{
			if(!p_igetl(&tempitem.scrconfig.run_args[j],f))
				return qe_invalid;
			tempitem.collect_scrconfig.run_args[j] = tempitem.scrconfig.run_args[j];
			tempitem.sprite_scrconfig.run_args[j] = tempitem.scrconfig.run_args[j];
		}
		
		for(int32_t j=0; j<2; j++)
		{
			byte temp;
			if(!p_getc(&temp,f))
			{
				return qe_invalid;
			}
		}
		
		if(s_version>4)
		{
			if(s_version>5)
			{
				int count = s_version >= 15 ? 10 : 4;
				for (int q = 0; q < count; ++q)
				{
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					tempitem.wpn_sprites[q] = tempbyte;
				}
				
				if(!p_getc(&tempitem.pickup_hearts,f))
				{
					return qe_invalid;
				}
				
				if(s_version<15)
				{
					if(!p_igetw(&dummy_word,f))
					{
						return qe_invalid;
					}
					
					tempitem.misc1=dummy_word;
					
					if(!p_igetw(&dummy_word,f))
					{
						return qe_invalid;
					}
					
					tempitem.misc2=dummy_word;
				}
				else
				{
					if(!p_igetl(&tempitem.misc1,f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&tempitem.misc2,f))
					{
						return qe_invalid;
					}
					
					// Version 24: sh_ice -> sh_script; previously, all shields could block script weapons
					if(s_version<24)
					{
						if(tempitem.type==itype_shield)
						{
							tempitem.misc1|=sh_script;
						}
					}
				}
				
				if(s_version < 53)
				{
					byte tempbyte;
					if(!p_getc(&tempbyte,f))
					{
						return qe_invalid;
					}
					tempitem.cost_amount[0] = tempbyte;
				}
				else
				{
					for(auto q = 0; q < 2; ++q)
					{
						if(!p_igetw(&tempitem.cost_amount[q],f))
						{
							return qe_invalid;
						}
					}
				}
			}
			else
			{
				char tempchar;
				
				if(!p_getc(&tempchar,f))
				{
					return qe_invalid;
				}
				
				if (tempchar) tempitem.flags |= item_edible;
			}
			
			// June 2007: more misc. attributes
			if(s_version>=12)
			{
				if(s_version<15)
				{
					if(!p_igetw(&dummy_word,f))
					{
						return qe_invalid;
					}
					
					tempitem.misc3=dummy_word;
					
					if(!p_igetw(&dummy_word,f))
					{
						return qe_invalid;
					}
					
					tempitem.misc4=dummy_word;
				}
				else
				{
					if(!p_igetl(&tempitem.misc3,f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&tempitem.misc4,f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&tempitem.misc5,f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&tempitem.misc6,f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&tempitem.misc7,f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&tempitem.misc8,f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&tempitem.misc9,f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&tempitem.misc10,f))
					{
						return qe_invalid;
					}
				}
				
				
				if (s_version >= 66)
				{
					if(!p_igetw(&tempitem.usesound,f))
						return qe_invalid;
					if(!p_igetw(&tempitem.usesound2,f))
						return qe_invalid;
				}
				else
				{
					if(!p_getc(&tempbyte,f))
						return qe_invalid;
					tempitem.usesound = tempbyte;
					if (s_version >= 49)
					{
						if(!p_getc(&tempbyte,f))
							return qe_invalid;
						tempitem.usesound2 = tempbyte;
					}
					else tempitem.usesound2 = 0;
				}
				if(s_version < 50 && tempitem.type == itype_mirror)
				{
					//Split continue/dmap warp effect/sfx, port for old
					tempitem.misc2 = tempitem.misc1;
					tempitem.usesound2 = tempitem.usesound;
				}
			}
		}
	
		if ( s_version >= 26 )  //! New itemdata vars for weapon editor. -Z
		{			// temp.useweapon, temp.usedefence, temp.weaprange, temp.weap_pattern[ITEM_MOVEMENT_PATTERNS]
			if(s_version < 63)
			{
				if(!p_getc(&tempitem.weap_data.imitate_weapon,f))
				{
					return qe_invalid;
				}
				if(!p_getc(&tempitem.weap_data.default_defense,f))
				{
					return qe_invalid;
				}
			}
			if(!p_igetl(&tempitem.weaprange,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempitem.weapduration,f))
			{
				return qe_invalid;
			}
			for ( int32_t q = 0; q < ITEM_MOVEMENT_PATTERNS; q++ )
			{
				if(!p_igetl(&tempitem.weap_pattern[q],f))
				{
					return qe_invalid;
				}
			}
		}
		
		if ( s_version >= 27 )  //! New itemdata vars for weapon editor. -Z
		{			// temp.useweapon, temp.usedefence, temp.weaprange, temp.weap_pattern[ITEM_MOVEMENT_PATTERNS]
			if(!p_igetl(&tempitem.duplicates,f))
			{
				return qe_invalid;
			}
			if(s_version < 63)
				for ( int32_t q = 0; q < INITIAL_D; q++ )
					if(!p_igetl(&tempitem.weap_data.scrconfig.run_args[q],f))
						return qe_invalid;
			for ( int32_t q = 0; q < 2; q++ )
			{
				byte temp;
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
			}
			
			if(!p_getc(&tempitem.drawlayer,f))
			{
				return qe_invalid;
			}
			
			
			if(!p_igetl(&tempitem.hxofs,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempitem.hyofs,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempitem.hxsz,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempitem.hysz,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempitem.hzsz,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempitem.xofs,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempitem.yofs,f))
			{
				return qe_invalid;
			}
			if(s_version < 63)
			{
				if(!p_igetl(&tempitem.weap_data.hxofs,f))
					return qe_invalid;
				if(!p_igetl(&tempitem.weap_data.hyofs,f))
					return qe_invalid;
				if(!p_igetl(&tempitem.weap_data.hxsz,f))
					return qe_invalid;
				if(!p_igetl(&tempitem.weap_data.hysz,f))
					return qe_invalid;
				if(!p_igetl(&tempitem.weap_data.hzsz,f))
					return qe_invalid;
				if(!p_igetl(&tempitem.weap_data.xofs,f))
					return qe_invalid;
				if(!p_igetl(&tempitem.weap_data.yofs,f))
					return qe_invalid;
			}
			if(s_version < 63)
				if(!p_igetw(&tempitem.weap_data.scrconfig.script,f))
					return qe_invalid;
			if(!p_igetl(&tempitem.wpnsprite,f))
			{
				return qe_invalid;
			}
			auto num_cost_tmr = (s_version > 52 ? 2 : 1);
			for(auto q = 0; q < num_cost_tmr; ++q)
			{
				if(!p_igetl(&tempitem.magiccosttimer[q],f))
				{
					return qe_invalid;
				}
			}
			for(auto q = num_cost_tmr; q < 2; ++q)
				tempitem.magiccosttimer[q] = 0;
		}
		if ( s_version >= 28 )  //! New itemdata vars for weapon editor. -Z
		{
			//Item Size FLags, TileWidth, TileHeight
			if(!p_igetl(&tempitem.overrideFLAGS,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempitem.tilew,f))
			{
				return qe_invalid;
			}
			if(!p_igetl(&tempitem.tileh,f))
			{
				return qe_invalid;
			}
		}
		if ( s_version >= 29 && s_version < 63)  //! More new vars. 
		{
			if(!p_igetl(&tempitem.weap_data.override_flags,f))
				return qe_invalid;
			if(!p_igetl(&tempitem.weap_data.tilew,f))
				return qe_invalid;
			if(!p_igetl(&tempitem.weap_data.tileh,f))
				return qe_invalid;
		}
		if ( s_version >= 30 )  //! More new vars. 
		{
			//Pickup Type
			if(!p_igetl(&tempitem.pickup,f))
			{
				return qe_invalid;
			}
		}
		if ( s_version >= 32 )  //! More new vars. 
		{
			//Pickup Type
			if(!p_igetw(&tempitem.pstring,f))
			{
				return qe_invalid;
			}
		}
		if ( s_version >= 33 )  //! More new vars. 
		{
			//Pickup Type
			if(!p_igetw(&tempitem.pickup_string_flags,f))
			{
				return qe_invalid;
			}
		}
		if ( s_version >= 34 )  //! cost counter
		{
			if(s_version < 53)
			{
				if(!p_getc(&tempitem.cost_counter[0],f))
				{
					return qe_invalid;
				}
			}
			else
			{
				for(auto q = 0; q < 2; ++q)
				{
					if(!p_getc(&tempitem.cost_counter[q],f))
					{
						return qe_invalid;
					}
				}
			}
		}
		if ( s_version >= 44 )  //! sprite scripts
		{
			for ( int32_t q = 0; q < 8; q++ )
			{
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&padding,f))
					{
						return qe_invalid;
					} 
				}
				if(s_version < 63)
					for ( int32_t w = 0; w < 65; w++ )
						if(!p_getc(&padding,f))
							return qe_invalid;
				for ( int32_t w = 0; w < 65; w++ )
				{
					if(!p_getc(&padding,f))
					{
						return qe_invalid;
					} 
				}
				if(!p_igetl(&tempdword,f))
				{
					return qe_invalid;
				}
				
			}
			for ( int32_t q = 0; q < 2; q++ )
			{
				byte temp;
				if(!p_getc(&temp,f))
				{
					return qe_invalid;
				}
			}
			//Pickup Type
			if(!p_igetw(&tempitem.sprite_scrconfig.script,f))
			{
				return qe_invalid;
			}
		}
		if ( s_version >= 48 )  //! pickup flags
		{
			if(!p_getc(&(tempitem.pickupflag),f))
			{
				return qe_invalid;
			}
		}
		if ( s_version >= 57 )
		{
			if(!p_getcstr(&tempitem.display_name,f))
				return qe_invalid;
		}
	}
	else
	{
		tempitem.count=-1;
		tempitem.type=itype_misc;
		tempitem.flags=item_none;
		tempitem.wpn_sprites[0]=tempitem.wpn_sprites[1]=tempitem.wpn_sprites[2]=tempitem.pickup_hearts=tempitem.misc1=tempitem.misc2=tempitem.usesound=0;
		tempitem.playsound=WAV_SCALE;
		reset_itembuf(&tempitem,index);
	}
	
	if(s_version >= 58 && s_version < 63)
	{
		for(int q = 0; q < WPNSPR_MAX; ++q)
		{
			if(!p_getc(&tempbyte,f))
				return qe_invalid;
			tempitem.weap_data.burnsprs[q] = tempbyte;
			if(s_version >= 59)
				if(!p_getc(&tempitem.weap_data.light_rads[q],f))
					return qe_invalid;
		}
	}
	
	if ( s_version >= 60 )
	{
		if ( s_version >= 65 )
		{
			if(!p_igetw(&tempitem.pickup_litems,f))
				return qe_invalid;
		}
		else
		{
			if(!p_getc(&padding,f))
				return qe_invalid;
			tempitem.pickup_litems = word(padding);
		}
		if(!p_igetw(&tempitem.pickup_litem_level,f))
			return qe_invalid;
	}
	
	if ( s_version >= 62 )
	{
		if (!p_igetl(&tempitem.moveflags, f))
			return qe_invalid;
		if(s_version < 63)
			if (!p_igetl(&tempitem.weap_data.moveflags, f))
				return qe_invalid;
	}
	else
	{
		tempitem.moveflags = (move_obeys_grav | move_can_pitfall);
		switch(tempitem.type)
		{
			case itype_divinefire:
				if(!(tempitem.flags & item_flag3))
					break;
			[[fallthrough]];
			case itype_bomb: case itype_sbomb:
			case itype_bait: case itype_liftglove:
			case itype_candle: case itype_book:
				tempitem.weap_data.moveflags = (move_obeys_grav | move_can_pitfall);
				break;
			default:
				tempitem.weap_data.moveflags = move_none;
				break;
		}
	}
	
	if(s_version >= 63)
	{
		if(auto ret = read_weap_data(tempitem.weap_data, f))
			return ret;
	}
	else
	{
		SETFLAG(tempitem.weap_data.wflags, WFLAG_UPDATE_IGNITE_SPRITE, tempitem.flags & item_burning_sprites);
		switch(tempitem.type)
		{
			case itype_liftglove:
				tempitem.weap_data.wflags = WFLAG_BREAK_WHEN_LANDING;
				break;
			case itype_bomb: case itype_sbomb:
				// Moving these over and removing them from itemdata
				if(tempitem.flags & item_flag3)
					tempitem.weap_data.wflags |= WFLAG_STOP_WHEN_LANDING;
				if(tempitem.flags & item_flag5)
					tempitem.weap_data.wflags |= WFLAG_STOP_WHEN_HIT_SOLID;
				tempitem.flags &= ~(item_flag3|item_flag5);
				if(tempitem.misc4)
				{
					tempitem.weap_data.lift_level = tempitem.misc4;
					tempitem.weap_data.lift_time = tempitem.misc5;
					tempitem.weap_data.lift_height = tempitem.misc6;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
				}
				break;
		}
	}
	
	if (s_version >= 64)
	{
		if (!p_igetl(&tempitem.cooldown, f))
			return qe_invalid;
	}
	
	if (!should_skip)
	{
		if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
		{
			tempitem.scrconfig.clear();
			tempitem.weap_data.scrconfig.clear();
		}
		itemsbuf[index] = tempitem;
	}
	return 0;
}

int32_t read_items_old(PACKFILE *f, word s_version, word version, word build)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_items);

    word items_to_read = 256;
    
    if(version < 0x186)
    {
        items_to_read=64;
    }
    
    if(version > 0x192)
    {
        items_to_read=0;
		
        //finally...  section data
        if(!p_igetw(&items_to_read,f))
        {
            return qe_invalid;
        }

        if (items_to_read > MAXITEMS)
        {
            return qe_invalid;
        }
    }
    
    
	if (!should_skip)
	{
		itemsbuf.clear();
		// default any items that might be needed for hardcoded behavior,
		// which will be missed in the reading code (>= items_to_read).
		itemsbuf.reserve(zc_max(items_to_read, iLast));
		for (int q = items_to_read; q < iLast; ++q)
		{ // likely only runs version < 0x186?
			itemdata& id = itemsbuf[q];
			reset_itembuf(&id, q);
		}
	}
    if(s_version>1)
    {
        for(int32_t i=0; i<items_to_read; i++)
        {
            char tempname[64];
            
            if(!pfread(tempname, 64, f))
                return qe_invalid;
            
			tempname[63] = 0;
			if (!should_skip)
				itemsbuf[i].name = tempname;
        }
    }
    else if (!should_skip)
    {
		for(int32_t i=0; i<256; i++)
			reset_itemname(i);
    }
    
    for (int32_t i=0; i<items_to_read; i++)
		if (auto ret = read_single_item_old(f, s_version, i, version, build))
			return ret;

	if (!should_skip)
		for(word i = 0; i < itemsbuf.capacity(); ++i)
			update_old_item(s_version, i, version, build);
	
	return 0;
}

} // end namespace

void update_old_item(word s_version, word index, word version, word build)
{
	itemdata& tempitem = itemsbuf[index];
	
	if (s_version < 67)
	{
		//Account for older quests that didn't have an actual item for the used letter
		if(s_version < 2 && index==iLetterUsed)
		{
			reset_itembuf(&tempitem, iLetterUsed);
			tempitem.name = old_item_string[index];
			auto const& letter = get_item_data(iLetter);
			tempitem.tile = letter.tile;
			tempitem.csets = letter.csets;
			tempitem.misc_flags = letter.misc_flags;
			tempitem.frames = letter.frames;
			tempitem.speed = letter.speed;
			tempitem.ltm = letter.ltm;
		}
		
		if(s_version < 3)
		{
			switch(index)
			{
				case iRocsFeather:
				case iHoverBoots:
				case iSpinScroll:
				case iL2SpinScroll:
				case iCrossScroll:
				case iQuakeScroll:
				case iL2QuakeScroll:
				case iWhispRing:
				case iL2WhispRing:
				case iChargeRing:
				case iL2ChargeRing:
				case iPerilScroll:
				case iWalletL3:
				case iQuiverL4:
				case iBombBagL4:
				case iBracelet:
				case iL2Bracelet:
				case iOldGlove:
				case iL2Ladder:
				case iWealthMedal:
				case iL2WealthMedal:
				case iL3WealthMedal:
					reset_itembuf(&tempitem, index);
					tempitem.name = old_item_string[index];
					break;
					
				case iSShield:
					reset_itembuf(&tempitem, index);
					tempitem.name = old_item_string[index];
					break;
			}
		}
		
		if(s_version < 5)
		{
			switch(index)
			{
				case iHeartRing:
				case iL2HeartRing:
				case iL3HeartRing:
				case iMagicRing:
				case iL2MagicRing:
				case iL3MagicRing:
				case iL4MagicRing:
					reset_itembuf(&tempitem, index);
					tempitem.name = old_item_string[index];
					break;
			}
		}
		
		if(s_version < 6)  // April 2007: Advanced item editing capabilities.
		{
			if(index !=iBPotion && index !=iRPotion)
				if (get_bit(deprecated_rules,32)) tempitem.flags |= item_keep_old;
				
			switch(index)
			{
				case iTriforce:
					tempitem.level=1;
					break;
					
				case iBigTri:
					tempitem.level=0;
					break;
					
				case iBombs:
					tempitem.level=i_bomb;
					tempitem.power=4;
					tempitem.wpn_sprites[0]=wBOMB;
					tempitem.wpn_sprites[1]=wBOOM;
					tempitem.misc1 = 50;
					
					if(get_bit(deprecated_rules,qr_SLOWBOMBFUSES_DEP)) tempitem.misc1 = 200;
					
					break;
					
				case iSBomb:
					tempitem.level=i_sbomb;
					tempitem.power=16;
					tempitem.wpn_sprites[0]=wSBOMB;
					tempitem.wpn_sprites[1]=wSBOOM;
					tempitem.misc1 = 50;
					
					if(get_bit(deprecated_rules,qr_SLOWBOMBFUSES_DEP)) tempitem.misc1 = 400;
					
					break;
					
				case iBook:
					if(get_bit(deprecated_rules, qr_FIREMAGICSPRITE_DEP))
						tempitem.wpn_sprites[0] = wFIREMAGIC;
						
					break;
					
				case iSArrow:
					tempitem.wpn_sprites[1] = get_bit(deprecated_rules,27) ? wSSPARKLE : 0; //qr_SASPARKLES
					tempitem.power=4;
					tempitem.flags|=item_gamedata;
					tempitem.wpn_sprites[0]=wSARROW;
					break;
					
				case iGArrow:
					tempitem.wpn_sprites[1] = get_bit(deprecated_rules,28) ? wGSPARKLE : 0; //qr_GASPARKLES
					tempitem.power=8;
					tempitem.flags|=(item_gamedata|item_flag1);
					tempitem.wpn_sprites[0]=wGARROW;
					break;
					
				case iBrang:
					tempitem.power=0;
					tempitem.wpn_sprites[0]=wBRANG;
					tempitem.misc1=36;
					break;
					
				case iMBrang:
					tempitem.wpn_sprites[1] = get_bit(deprecated_rules,29) ? wMSPARKLE : 0; //qr_MBSPARKLES
					tempitem.power=0;
					tempitem.wpn_sprites[0]=wMBRANG;
					break;
					
				case iFBrang:
					tempitem.wpn_sprites[2] = get_bit(deprecated_rules,30) ? wFSPARKLE : 0; //qr_FBSPARKLES
					tempitem.power=2;
					tempitem.wpn_sprites[0]=wFBRANG;
					break;
					
				case iBoots:
					tempitem.cost_amount[0] = get_bit(deprecated_rules,qr_MAGICBOOTS_DEP) ? 1 : 0;
					tempitem.power=7;
					break;
					
				case iWand:
					tempitem.cost_amount[0] = get_bit(deprecated_rules,qr_MAGICWAND_DEP) ? 8 : 0;
					tempitem.power=2;
					tempitem.wpn_sprites[0]=wWAND;
					tempitem.wpn_sprites[2]=wMAGIC;
					break;
					
				case iBCandle:
					tempitem.cost_amount[0] = get_bit(deprecated_rules,qr_MAGICCANDLE_DEP) ? 4 : 0;
					tempitem.power=1;
					tempitem.flags|=(item_gamedata|item_flag1);
					tempitem.wpn_sprites[2]=wFIRE;
					break;
					
				case iRCandle:
					tempitem.cost_amount[0] = get_bit(deprecated_rules,qr_MAGICCANDLE_DEP) ? 4 : 0;
					tempitem.power=1;
					tempitem.wpn_sprites[2]=wFIRE;
					break;
					
				case iSword:
					tempitem.power=1;
					tempitem.flags|= item_flag4 |item_flag2;
					tempitem.wpn_sprites[0]=tempitem.wpn_sprites[2]=wSWORD;
					tempitem.wpn_sprites[1]=wSWORDSLASH;
					break;
					
				case iWSword:
					tempitem.power=2;
					tempitem.flags|= item_flag4 |item_flag2;
					tempitem.wpn_sprites[0]=tempitem.wpn_sprites[2]=wWSWORD;
					tempitem.wpn_sprites[1]=wWSWORDSLASH;
					break;
					
				case iMSword:
					tempitem.power=4;
					tempitem.flags|= item_flag4 |item_flag2;
					tempitem.wpn_sprites[0]=tempitem.wpn_sprites[2]=wMSWORD;
					tempitem.wpn_sprites[1]=wMSWORDSLASH;
					break;
					
				case iXSword:
					tempitem.power=8;
					tempitem.flags|= item_flag4 |item_flag2;
					tempitem.wpn_sprites[0]=tempitem.wpn_sprites[2]=wXSWORD;
					tempitem.wpn_sprites[1]=wXSWORDSLASH;
					break;
					
				case iDivineProtection:
					tempitem.flags |= get_bit(deprecated_rules,qr_FLICKERINGDIVINEPROTECTIONROCKET_DEP) ? item_flag1 : item_none;
					tempitem.flags |= get_bit(deprecated_rules,qr_TRANSLUCENTDIVINEPROTECTIONROCKET_DEP) ? item_flag2 : item_none;
					tempitem.wpn_sprites[0]=wDIVINEPROTECTION1A;
					tempitem.wpn_sprites[1]=wDIVINEPROTECTION1B;
					tempitem.wpn_sprites[2]=wDIVINEPROTECTIONS1A;
					tempitem.wpn_sprites[3]=wDIVINEPROTECTIONS1B;
					tempitem.wpn_sprites[5]=wDIVINEPROTECTION2A;
					tempitem.wpn_sprites[6]=wDIVINEPROTECTION2B;
					tempitem.wpn_sprites[7]=wDIVINEPROTECTIONS2A;
					tempitem.wpn_sprites[8]=wDIVINEPROTECTIONS2B;
					tempitem.wpn_sprites[4] = iwDivineProtectionShieldFront;
					tempitem.wpn_sprites[9] = iwDivineProtectionShieldBack;
					tempitem.misc1=512;
					tempitem.cost_amount[0]=64;
					break;
					
				case iLens:
					tempitem.misc1=60;
					tempitem.flags |= get_qr(qr_ENABLEMAGIC) ? item_none : item_rupee_magic;
					tempitem.cost_amount[0] = get_qr(qr_ENABLEMAGIC) ? 2 : 1;
					break;
					
				case iArrow:
					tempitem.power=2;
					tempitem.wpn_sprites[0]=wARROW;
					break;
					
				case iHoverBoots:
					tempitem.misc1=45;
					tempitem.wpn_sprites[0]=iwHover;
					break;
					
				case iDivineFire:
					tempitem.power=8;
					tempitem.wpn_sprites[0]=wDIVINEFIRE1A;
					tempitem.wpn_sprites[1]=wDIVINEFIRE1B;
					tempitem.wpn_sprites[2]=wDIVINEFIRES1A;
					tempitem.wpn_sprites[3]=wDIVINEFIRES1B;
					tempitem.misc1 = 32;
					tempitem.misc2 = 200;
					tempitem.cost_amount[0]=32;
					break;
					
				case iDivineEscape:
					tempitem.cost_amount[0]=32;
					break;
					
				case iHookshot:
					tempitem.power=0;
					tempitem.flags&=~item_flag1;
					tempitem.wpn_sprites[0]=wHSHEAD;
					tempitem.wpn_sprites[1]=wHSCHAIN_H;
					tempitem.wpn_sprites[3]=wHSHANDLE;
					tempitem.wpn_sprites[2]=wHSCHAIN_V;
					tempitem.misc1=50;
					tempitem.misc2=100;
					break;
					
				case iLongshot:
					tempitem.power=0;
					tempitem.flags&=~item_flag1;
					tempitem.wpn_sprites[0]=wLSHEAD;
					tempitem.wpn_sprites[1]=wLSCHAIN_H;
					tempitem.wpn_sprites[3]=wLSHANDLE;
					tempitem.wpn_sprites[2]=wLSCHAIN_V;
					tempitem.misc1=99;
					tempitem.misc2=100;
					break;
					
				case iHammer:
					tempitem.power=4;
					tempitem.wpn_sprites[0]=wHAMMER;
					tempitem.wpn_sprites[1]=iwHammerSmack;
					break;
					
				case iCByrna:
					tempitem.power=1;
					tempitem.wpn_sprites[0]=wCBYRNA;
					tempitem.wpn_sprites[1]=wCBYRNASLASH;
					tempitem.wpn_sprites[2]=wCBYRNAORB;
					tempitem.misc1=4;
					tempitem.misc2=16;
					tempitem.misc3=1;
					tempitem.cost_amount[0]=1;
					break;
					
				case iWhistle:
					tempitem.wpn_sprites[0]=wWIND;
					tempitem.misc1=3;
					tempitem.flags|=item_flag1;
					break;
					
				case iBRing:
					tempitem.power=2;
					tempitem.misc1=spBLUE;
					break;
					
				case iRRing:
					tempitem.power=4;
					tempitem.misc1=spRED;
					break;
					
				case iGRing:
					tempitem.power=8;
					tempitem.misc1=spGOLD;
					break;
					
				case iSpinScroll:
					tempitem.power = 2;
					tempitem.misc1 = 1;
					break;
					
				case iL2SpinScroll:
					tempitem.type=itype_spinscroll2;
					tempitem.level=1;
					tempitem.cost_amount[0]=8;
					tempitem.power=2;
					tempitem.misc1 = 20;
					break;
					
				case iQuakeScroll:
					tempitem.misc1=0x10;
					tempitem.misc2=64;
					break;
					
				case iL2QuakeScroll:
					tempitem.type=itype_quakescroll2;
					tempitem.level=1;
					tempitem.power = 2;
					tempitem.misc1=0x20;
					tempitem.misc2=192;
					tempitem.cost_amount[0]=8;
					break;
					
				case iChargeRing:
					tempitem.misc1=64;
					tempitem.misc2=128;
					break;
					
				case iL2ChargeRing:
					tempitem.misc1=32;
					tempitem.misc2=64;
					break;
					
				case iOldGlove:
					tempitem.flags |= item_flag1;
					
					//fallthrough
				case iBombBagL4:
				case iWalletL3:
				case iQuiverL4:
				case iBracelet:
					tempitem.power = 1;
					break;
					
				case iL2Bracelet:
					tempitem.power = 2;
					break;
					
				case iMKey:
					tempitem.power=0xFF;
					tempitem.flags |= item_flag1;
					break;
			}
		}
		
		if(s_version < 7)
		{
			switch(index)
			{
				case iStoneAgony:
				case iStompBoots:
				case iPerilRing:
				case iWhimsicalRing:
				{
					reset_itembuf(&tempitem, index);
					tempitem.name = old_item_string[index];
					break;
				}
			}
		}
		
		if(s_version < 8) // May 2007: Some corrections.
		{
			switch(index)
			{
				case iMShield:
					tempitem.misc1|=sh_flame;
					tempitem.misc2|=sh_fireball|sh_magic;
					
					if(get_qr(qr_SWORDMIRROR))
					{
						tempitem.misc2 |= sh_sword;
					}
					
					// fallthrough
				case iShield:
					tempitem.misc1|=sh_fireball|sh_sword|sh_magic;
					
					// fallthrough
				case iSShield:
					tempitem.misc1|=sh_rock|sh_arrow|sh_brang|sh_script;
					
					if(get_bit(deprecated_rules,102))  //qr_REFLECTROCKS
					{
						tempitem.misc2 |= sh_rock;
					}
					
					break;
					
				case iWhispRing:
					tempitem.power=1;
					tempitem.flags|=item_gamedata|item_flag1;
					tempitem.misc1 = 3;
					break;
					
				case iL2WhispRing:
					tempitem.power=0;
					tempitem.flags|=item_gamedata|item_flag1;
					tempitem.misc1 = 3;
					break;
					
				case iL2Ladder:
				case iBow:
				case iCByrna:
					tempitem.power = 1;
					break;
			}
		}
		
		if(s_version < 9 && index ==iClock)
		{
			tempitem.misc1 = get_bit(deprecated_rules, qr_TEMPCLOCKS_DEP) ? 256 : 0;
		}
		
		//add the misc flag for bomb
		if(s_version < 10 && tempitem.type == itype_bomb)
		{
			tempitem.flags = (tempitem.flags & ~item_flag1) | (get_qr(qr_LONGBOMBBOOM_DEP) ? item_flag1 : item_none);
		}
		
		if(s_version < 11 && tempitem.type == itype_triforcepiece)
		{
			tempitem.flags = (tempitem.level ? item_gamedata : item_none);
			tempitem.playsound = (tempitem.level ? WAV_SCALE : WAV_CLEARED);
		}
		
		if(s_version < 12) // June 2007: More Misc. attributes.
		{
			switch(index)
			{
			case iFBrang:
				tempitem.misc4 |= sh_fireball|sh_sword|sh_magic;
				
				//fallthrough
			case iMBrang:
				tempitem.misc3 |= sh_sword|sh_magic;
				
				//fallthrough
			case iHookshot:
			case iLongshot:
				//fallthrough
				tempitem.misc3 |= sh_fireball;
				
			case iBrang:
				tempitem.misc3 |= sh_brang|sh_rock|sh_arrow;
				break;
			}
			
			switch(tempitem.type)
			{
			case itype_hoverboots:
				tempitem.usesound = WAV_ZN1HOVER;
				break;
				
			case itype_wand:
				tempitem.usesound = WAV_WAND;
				break;
				
			case itype_book:
				tempitem.usesound = WAV_FIRE;
				break;
				
			case itype_arrow:
				tempitem.usesound = WAV_ARROW;
				break;
				
			case itype_hookshot:
				tempitem.usesound = WAV_HOOKSHOT;
				break;
				
			case itype_brang:
				tempitem.usesound = WAV_BRANG;
				break;
				
			case itype_shield:
				tempitem.usesound = WAV_CHINK;
				break;
				
			case itype_sword:
				tempitem.usesound = WAV_SWORD;
				break;
				
			case itype_whistle:
				tempitem.usesound = WAV_WHISTLE;
				break;
				
			case itype_hammer:
				tempitem.usesound = WAV_HAMMER;
				break;
				
			case itype_divinefire:
				tempitem.usesound = WAV_ZN1DIVINEFIRE;
				break;
				
			case itype_divineescape:
				tempitem.usesound = WAV_ZN1DIVINEESCAPE;
				break;
				
			case itype_divineprotection:
				tempitem.usesound = WAV_ZN1DIVINEPROTECTION1;
				break;
				
			case itype_bomb:
			case itype_sbomb:
			case itype_quakescroll:
			case itype_quakescroll2:
				tempitem.usesound = WAV_BOMB;
				break;
				
			case itype_spinscroll:
			case itype_spinscroll2:
				tempitem.usesound = WAV_ZN1SPINATTACK;
				break;
			}
		}
		
		if(s_version < 13) // July 2007
		{
			if(tempitem.type == itype_whistle)
			{
				tempitem.misc1 = (tempitem.power==2 ? 4 : 3);
				tempitem.power = 1;
				tempitem.flags|=item_flag1;
			}
			else if(tempitem.type == itype_wand)
				tempitem.flags|=item_flag1;
			else if(tempitem.type == itype_book)
			{
				tempitem.flags|=item_flag1;
				tempitem.power = 2;
			}
		}
		
		if(s_version < 14) // August 2007
		{
			if(tempitem.type == itype_fairy)
			{
				tempitem.usesound = WAV_SCALE;
				
				if(tempitem.level)
					tempitem.misc3=50;
			}
			else if(tempitem.type == itype_potion)
			{
				tempitem.flags |= item_gain_old;
			}
		}
		
		if(s_version < 17) // November 2007
		{
			if(tempitem.type == itype_candle && !tempitem.wpn_sprites[2])
			{
				tempitem.wpn_sprites[2] = wFIRE;
			}
			else if(tempitem.type == itype_arrow && tempitem.power>4)
			{
				tempitem.flags|=item_flag1;
			}
		}
		
		if(s_version < 18) // New Year's Eve 2007
		{
			if(tempitem.type == itype_whistle)
				tempitem.misc2 = 8; // Use the Whistle warp ring
			else if(tempitem.type == itype_bait)
				tempitem.misc1 = 768; // Frames until it goes
			else if(tempitem.type == itype_triforcepiece)
			{
				if(tempitem.flags & item_gamedata)
				{
					tempitem.misc2 = 1; // Cutscene 1
					tempitem.flags |= item_flag1; // Side Warp Out
				}
			}
		}
		
		if(s_version < 19)  // January 2008
		{
			if(tempitem.type == itype_divineprotection)
			{
				if (get_bit(deprecated_rules,qr_NOBOMBPALFLASH+1)) tempitem.flags |= item_flag3;
				if (get_bit(deprecated_rules,qr_NOBOMBPALFLASH+2)) tempitem.flags |= item_flag4;
			}
		}
		
		if(s_version < 20)  // October 2008
		{
			if(tempitem.type == itype_divineprotection)
			{
				tempitem.wpn_sprites[5]=wDIVINEPROTECTION2A;
				tempitem.wpn_sprites[6]=wDIVINEPROTECTION2B;
				tempitem.wpn_sprites[7]=wDIVINEPROTECTIONS2A;
				tempitem.wpn_sprites[8]=wDIVINEPROTECTIONS2B;
				tempitem.wpn_sprites[4] = iwDivineProtectionShieldFront;
				tempitem.wpn_sprites[9] = iwDivineProtectionShieldBack;
			}
		}
		
		if(s_version < 21)  // November 2008
		{
			if(tempitem.flags & 0x0100)  // item_slash
			{
				tempitem.flags &= ~item_unused;
				
				if(tempitem.type == itype_sword ||
						tempitem.type == itype_wand ||
						tempitem.type == itype_candle ||
						tempitem.type == itype_cbyrna)
				{
					tempitem.flags |= item_flag4;
				}
			}
		}
		
		if(s_version < 22)  // September 2009
		{
			if(tempitem.type == itype_sbomb || tempitem.type == itype_bomb)
			{
				tempitem.misc3 = tempitem.power/2;
			}
		}
		
		if(s_version < 23)    // March 2011
		{
			if(tempitem.type == itype_divinefire)
				tempitem.wpn_sprites[4] = wFIRE;
			else if(tempitem.type == itype_book)
				tempitem.wpn_sprites[1] = wFIRE;
		}
		
		// Version 25: Bomb bags were acting as though "super bombs also" was checked
		// whether it was or not, and a lot of existing quests depended on the
		// incorrect behavior.
		if(s_version < 25)    // January 2012
		{
			if(tempitem.type == itype_bombbag)
				tempitem.flags |= item_flag1;
				
			if(tempitem.type == itype_divinefire)
				tempitem.flags |= item_flag3; // Sideview gravity flag
		}
		
		if (version < 0x250 && (version == 0x250 && build < 30)) // < 2.53
		{
			if (tempitem.type == itype_triforcepiece)
			{
				if (tempitem.flags & item_gamedata)
					tempitem.flags |= item_flag8;
			}
		}
		
		if( version < 0x254) //Nuke greyed-out flags/values from <=2.53, in case they are used in 2.54/2.55
		{
			switch(tempitem.type)
			{
				case itype_sword:
				{
					tempitem.flags &= ~(item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 3; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_brang:
				{
					tempitem.flags &= ~(item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 3; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_arrow:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 3; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_candle:
				{
					tempitem.flags &= ~ (item_flag3 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 3; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_whistle:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 1; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_bait:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 1; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_letter:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_potion:
				{
					tempitem.flags &= ~ (item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_wand:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 3; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_ring:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_wallet:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_amulet:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_shield:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_bow:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_raft:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_ladder:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_book:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 2; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_magickey:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_bracelet:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_flippers:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_boots:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_hookshot:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 4; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_lens:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_hammer:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 2; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_divinefire:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 5; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_divineescape:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_divineprotection:
				{
					tempitem.flags &= ~ (item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					break;
				}
				case itype_bomb:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 2; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_sbomb:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 2; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_clock:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_key:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_magiccontainer:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_triforcepiece:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_map:	case itype_compass:	case itype_bosskey:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_quiver:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_lkey:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_cbyrna:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag5);
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 5; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_rupee: case itype_arrowammo:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_fairy:
				{
					tempitem.flags &= ~ (item_flag3 | item_flag4 | item_flag5);
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_magic: case itype_heart: case itype_heartcontainer: case itype_heartpiece: case itype_killem: case itype_bombammo:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_bombbag:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_rocs:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_hoverboots:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 1; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_spinscroll:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_crossscroll:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_quakescroll:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_whispring:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_chargering:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_perilscroll:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_wealthmedal:
				{
					tempitem.flags &= ~ (item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_heartring:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_magicring:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_spinscroll2:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_quakescroll2:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_agony:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_stompboots:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_whimsicalring:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_perilring:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
				case itype_custom1: case itype_custom2: case itype_custom3: case itype_custom4: case itype_custom5:
				case itype_custom6: case itype_custom7: case itype_custom8: case itype_custom9: case itype_custom10:
				case itype_custom11: case itype_custom12: case itype_custom13: case itype_custom14: case itype_custom15:
				case itype_custom16: case itype_custom17: case itype_custom18: case itype_custom19: case itype_custom20:
				case itype_bowandarrow: case itype_letterpotion: case itype_misc:
				{
					tempitem.flags &= ~ (item_flag1 | item_flag2 | item_flag3 | item_flag4 | item_flag5);
					tempitem.misc1 = 0;
					tempitem.misc2 = 0;
					tempitem.misc3 = 0;
					tempitem.misc4 = 0;
					tempitem.misc5 = 0;
					tempitem.misc6 = 0;
					tempitem.misc7 = 0;
					tempitem.misc8 = 0;
					tempitem.misc9 = 0;
					tempitem.misc10 = 0;
					for(int q = 0; q < 10; ++q)
						tempitem.wpn_sprites[q] = 0;
					break;
				}
			}
		}
		//Port quest rules to items
		if( s_version <= 31) 
		{
			if(tempitem.type == itype_bomb)
			{
				if ( get_qr(qr_OUCHBOMBS) )  tempitem.flags |= item_flag2;
				else tempitem.flags &= ~ item_flag2;
			}
			else if(tempitem.type == itype_sbomb)
			{
				if ( get_qr(qr_OUCHBOMBS) )  tempitem.flags |= item_flag2;
				else tempitem.flags &= ~ item_flag2;
			}
			
			else if(tempitem.type == itype_brang)
			{
				if ( get_qr(qr_BRANGPICKUP) )  tempitem.flags |= item_flag4;
				else tempitem.flags &= ~ item_flag4;
			}	
			else if(tempitem.type == itype_wand)
			{
				if ( get_qr(qr_NOWANDMELEE) )  tempitem.flags |= item_flag3;
				else tempitem.flags &= ~ item_flag3;
			}
		}
		
		//Port quest rules to items
		if( s_version <= 37) 
		{
			if(tempitem.type == itype_flippers)
			{
				if ( (get_qr(qr_NODIVING)) ) tempitem.flags |= item_flag1;
				else tempitem.flags &= ~ item_flag1;
			}
			else if(tempitem.type == itype_sword)
			{
				if ( (get_qr(qr_QUICKSWORD)) ) tempitem.flags |= item_flag5;
				else tempitem.flags &= ~ item_flag5;
			}
			else if(tempitem.type == itype_wand)
			{
				if ( (get_qr(qr_QUICKSWORD)) ) tempitem.flags |= item_flag5;
				else tempitem.flags &= ~ item_flag5;
			}
			else if(tempitem.type == itype_book || tempitem.type == itype_candle)
			{
				//@Emily: What was qrFIREPROOFHERO2 again, and does that also need to enable this?
				if ( (get_qr(qr_FIREPROOFHERO)) ) tempitem.flags |= item_flag3;
				else tempitem.flags &= ~ item_flag3;
			}
		}
		
		if( s_version < 38)
		{
			if(tempitem.type == itype_brang || tempitem.type == itype_hookshot)
			{
				if(get_qr(qr_BRANGPICKUP)) tempitem.flags |= item_flag4;
				else tempitem.flags &= ~item_flag4;
				
				if(get_qr(qr_Z3BRANG_HSHOT)) tempitem.flags |= item_flag5 | item_flag6;
				else tempitem.flags &= ~(item_flag5|item_flag6);
			} 
			else if(tempitem.type == itype_arrow)
			{
				if(get_qr(qr_BRANGPICKUP)) tempitem.flags |= item_flag4;
				else tempitem.flags &= ~item_flag4;
				
				if(get_qr(qr_Z3BRANG_HSHOT)) tempitem.flags &= ~item_flag2;
				else tempitem.flags |= item_flag2;
			}
		}
		
		if( s_version < 39)
		{
			if(tempitem.type == itype_divinefire || tempitem.type == itype_book || tempitem.type == itype_candle)
			{
				if(get_qr(qr_TEMPCANDLELIGHT)) tempitem.flags |= item_flag5;
				else tempitem.flags &= ~item_flag5;
			}
			else if(tempitem.type == itype_potion)
			{
				if(get_qr(qr_NONBUBBLEMEDICINE))
				{
					tempitem.flags &= ~(item_flag3|item_flag4);
				}
				else
				{
					tempitem.flags |= item_flag3;
					if(get_qr(qr_ITEMBUBBLE))tempitem.flags |= item_flag4;
					else tempitem.flags &= ~item_flag4;
				}
			}
			else if(tempitem.type == itype_triforcepiece)
			{
				if(get_qr(qr_NONBUBBLETRIFORCE))
				{
					tempitem.flags |= item_flag3;
					if(get_qr(qr_ITEMBUBBLE))tempitem.flags |= item_flag4;
					else tempitem.flags &= ~item_flag4;
				}
				else
				{
					tempitem.flags &= ~(item_flag3|item_flag4);
				}
			}
		}
		
		if( s_version < 40)
		{
			if(tempitem.type == itype_ring || tempitem.type == itype_perilring)
			{
				if(get_qr(qr_RINGAFFECTDAMAGE))tempitem.flags |= item_flag1;
				else tempitem.flags &= ~item_flag1;
			} 
			else if(tempitem.type == itype_candle || tempitem.type == itype_sword || tempitem.type == itype_wand || tempitem.type == itype_cbyrna)
			{
				if(get_qr(qr_SLASHFLIPFIX))tempitem.flags |= item_flag8;
				else tempitem.flags &= ~item_flag8;
			}
			if(tempitem.type == itype_sword || tempitem.type == itype_wand || tempitem.type == itype_hammer)
			{
				if(get_qr(qr_NOITEMMELEE))tempitem.flags |= item_flag7;
				else tempitem.flags &= ~item_flag7;
			} 
			else if(tempitem.type == itype_cbyrna)
			{
				tempitem.flags |= item_flag7;
			}
		}
		
		if( s_version < 41 )
		{
			if(tempitem.type == itype_sword)
			{
				if(get_qr(qr_SWORDMIRROR))tempitem.flags |= item_flag9;
				else tempitem.flags &= ~item_flag9;
				
				if(get_qr(qr_SLOWCHARGINGWALK))tempitem.flags |= item_flag10;
				else tempitem.flags &= ~item_flag10;
			}
		}
		
		if( s_version < 42 )
		{
			if(tempitem.type == itype_wand)
			{
				if(get_qr(qr_NOWANDMELEE))tempitem.flags |= item_flag3;
				else tempitem.flags &= ~item_flag3;
				
				tempitem.flags &= ~item_flag6;
			} 
			else if(tempitem.type == itype_hammer)
			{
				tempitem.flags &= ~item_flag3;
			} 
			else if(tempitem.type == itype_cbyrna)
			{
				tempitem.flags |= item_flag3;
				
				tempitem.flags &= ~item_flag6;
			} 
			else if(tempitem.type == itype_sword)
			{
				if(get_qr(qr_MELEEMAGICCOST))tempitem.flags |= item_flag6;
				else tempitem.flags &= ~item_flag6;
			}
		}
		
		if( s_version < 43 )
		{
			if(tempitem.type == itype_whistle)
			{
				if(get_qr(qr_WHIRLWINDMIRROR))tempitem.flags |= item_flag3;
				else tempitem.flags &= ~item_flag3;
			}
		}
		
		if( s_version < 45 )
		{
			if(tempitem.type == itype_flippers)
			{
				tempitem.misc1 = 50; //Dive length, default 50 frames -V
				tempitem.misc2 = 30; //Dive cooldown, default 30 frames -V
			}
		}
		
		if( s_version < 46 )
		{
			if(tempitem.type == itype_raft)
			{
				tempitem.misc1 = 1; //Rafting speed modifier; default 1. Negative slows, positive speeds.
			}
		}
		if ( s_version < 34 )  //! set the default counter for older quests. 
		{
			if ( (tempitem.flags & item_rupee_magic) )
			{
				tempitem.cost_counter[0] = 1;
			}
			else 
			{
				if(get_qr(qr_ENABLEMAGIC))
					tempitem.cost_counter[0] = 4;
				else
				{
					tempitem.cost_amount[0] = 0;
					tempitem.cost_counter[0] = -1;
				}
			}
		}
		
		if ( s_version < 35 ) //new Lens of Truth flags		
		{
			if ( tempitem.type == itype_lens )
			{
				if ( get_qr(qr_RAFTLENS) ) 
				{
					tempitem.flags |= item_flag4;
				}
				if ( get_qr(qr_LENSHINTS) ) 
				{
					tempitem.flags |= item_flag1;
				}
				if ( get_qr(qr_LENSSEESENEMIES) ) 
				{
					tempitem.flags |= item_flag5;
				}
			}
		}
		if ( s_version < 44 ) //InitD Labels and Sprite Script Data
		{
			tempitem.sprite_scrconfig.clear();
		}
		if ( s_version < 47 ) //InitD Labels and Sprite Script Data
		{
			tempitem.pickupflag = 0;
		}
		
		if( s_version < 51 )
		{
			if( tempitem.type == itype_candle )
			{
				tempitem.misc4 = 50; //Step speed
			}
		}
		
		if( s_version < 52 )
		{
			if( tempitem.type == itype_shield )
				tempitem.flags |= item_flag1; //'Block Front' flag
		}
		if(s_version < 53)
		{
			switch(tempitem.type)
			{
				case itype_arrow:
					tempitem.cost_counter[1] = crARROWS;
					tempitem.cost_amount[1] = 1;
					break;
				case itype_bomb:
					tempitem.cost_counter[1] = crBOMBS;
					tempitem.cost_amount[1] = 1;
					break;
				case itype_sbomb:
					tempitem.cost_counter[1] = crSBOMBS;
					tempitem.cost_amount[1] = 1;
					break;
				default:
					tempitem.cost_counter[1] = crNONE;
					tempitem.cost_amount[1] = 0;
			}
			tempitem.magiccosttimer[1] = 0;
		}
		if( s_version < 54 )
		{
			if( tempitem.type == itype_flippers )
				tempitem.misc3 = INT_BTN_A; //'Block Front' flag
		}
		if(s_version < 55)
		{
			switch(tempitem.type)
			{
				case itype_spinscroll:
				case itype_quakescroll:
					tempitem.usesound2 = WAV_ZN1CHARGE;
					break;
				case itype_spinscroll2:
				case itype_quakescroll2:
					tempitem.usesound2 = WAV_ZN1CHARGE2;
					break;
			}
		}
		if(s_version < 56)
		{
			switch(tempitem.type)
			{
				case itype_divinefire:
					SETFLAG(tempitem.flags, item_flag9, version < 0x255); //Strong Fire
					SETFLAG(tempitem.flags, item_flag10, version < 0x250); //Magic Fire
					tempitem.flags |= item_flag11; //Divine Fire
					break;
				case itype_candle:
					SETFLAG(tempitem.flags, item_flag9, tempitem.level > 1); //Strong Fire
					tempitem.flags &= ~item_flag10; //Magic Fire
					tempitem.flags &= ~item_flag11; //Divine Fire
					break;
				case itype_book:
					tempitem.flags |= item_flag9; //Strong Fire
					tempitem.flags |= item_flag10; //Magic Fire
					tempitem.flags &= ~item_flag11; //Divine Fire
					break;
			}
		}
		if (s_version < 61)
		{
			switch (tempitem.type)
			{
				case itype_sword:
					tempitem.usesound2 = WAV_BEAM;
					break;
			}
		}
		
	}
	
	if(tempitem.level == 0)  // Always do this
		tempitem.level = 1;
}

int32_t read_single_item(PACKFILE *f, word s_version, word index, word version, word build)
{
	if (s_version < 67)
		return read_single_item_old(f, s_version, index, version, build);
	static itemdata _nil_item;
	
	byte tempbyte;
	dword tempdword;
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_items);
	itemdata& item_ref = should_skip ? _nil_item : itemsbuf[index];
	item_ref = itemdata();
	reset_itembuf(&item_ref, index);
	
	if (!p_getcstr(&item_ref.name, f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.tile,f))
		return qe_invalid;
	
	if(!p_getc(&item_ref.misc_flags,f))
		return qe_invalid;
	
	if(!p_getc(&item_ref.csets,f))
		return qe_invalid;
	
	if(!p_getc(&item_ref.frames,f))
		return qe_invalid;
	
	if(!p_getc(&item_ref.speed,f))
		return qe_invalid;
	
	if(!p_getc(&item_ref.delay,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.ltm,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.type,f))
		return qe_invalid;
	
	if(!p_getc(&item_ref.level,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.power,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.flags,f))
		return qe_invalid;
	
	if (s_version < 71)
		if(!p_igetw(&item_ref.scrconfig.script,f))
			return qe_invalid;
	
	if(!p_getc(&item_ref.count,f))
		return qe_invalid;
	
	if(!p_igetw(&item_ref.amount,f))
		return qe_invalid;
	
	if (s_version < 71)
		if(!p_igetw(&item_ref.collect_scrconfig.script,f))
			return qe_invalid;
	
	if(!p_igetw(&item_ref.setmax,f))
		return qe_invalid;
	
	if(!p_igetw(&item_ref.max,f))
		return qe_invalid;
	
	if(!p_igetw(&item_ref.playsound,f))
		return qe_invalid;
	
	if (s_version < 71)
	{
		for (size_t q = 0; q < 8; ++q)
		{
			if(!p_igetl(&item_ref.scrconfig.run_args[q], f))
				return qe_invalid;
			item_ref.collect_scrconfig.run_args[q] = item_ref.scrconfig.run_args[q];
			item_ref.sprite_scrconfig.run_args[q] = item_ref.scrconfig.run_args[q];
		}
	}
	
	if (s_version < 68)
	{
		for (int q = 0; q < 10; ++q)
		{
			if(!p_getc(&tempbyte,f))
				return qe_invalid;
			item_ref.wpn_sprites[q] = tempbyte;
		}
	}
	else
	{
		for (int q = 0; q < 10; ++q)
			if(!p_igetw(&item_ref.wpn_sprites[q],f))
				return qe_invalid;
	}
	
	if(!p_getc(&item_ref.pickup_hearts,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.misc1,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.misc2,f))
		return qe_invalid;
	
	for(size_t q = 0; q < 2; ++q)
		if(!p_igetw(&item_ref.cost_amount[q],f))
			return qe_invalid;
	
	if(!p_igetl(&item_ref.misc3,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.misc4,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.misc5,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.misc6,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.misc7,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.misc8,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.misc9,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.misc10,f))
		return qe_invalid;
	
	if(!p_igetw(&item_ref.usesound,f))
		return qe_invalid;
	if(!p_igetw(&item_ref.usesound2,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.weaprange,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.weapduration,f))
		return qe_invalid;
	for (size_t q = 0; q < ITEM_MOVEMENT_PATTERNS; ++q)
		if(!p_igetl(&item_ref.weap_pattern[q],f))
			return qe_invalid;
	
	if(!p_igetl(&item_ref.duplicates,f))
		return qe_invalid;
	if(!p_getc(&item_ref.drawlayer,f))
		return qe_invalid;
	
	if(!p_igetl(&item_ref.hxofs,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.hyofs,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.hxsz,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.hysz,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.hzsz,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.xofs,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.yofs,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.wpnsprite,f))
		return qe_invalid;
	for(size_t q = 0; q < 2; ++q)
		if(!p_igetl(&item_ref.magiccosttimer[q],f))
			return qe_invalid;
	
	if(!p_igetl(&item_ref.overrideFLAGS,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.tilew,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.tileh,f))
		return qe_invalid;
	if(!p_igetl(&item_ref.pickup,f))
		return qe_invalid;
	if(!p_igetw(&item_ref.pstring,f))
		return qe_invalid;
	if(!p_igetw(&item_ref.pickup_string_flags,f))
		return qe_invalid;
	for(size_t q = 0; q < 2; ++q)
		if(!p_getc(&item_ref.cost_counter[q],f))
			return qe_invalid;
		
	if (s_version < 71)
	{
		for ( int32_t q = 0; q < 8; q++ )
		{
			for ( int32_t w = 0; w < 65; w++ )
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
			for ( int32_t w = 0; w < 65; w++ )
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
			if(!p_igetl(&tempdword,f))
				return qe_invalid;
		}
		if(!p_igetw(&item_ref.sprite_scrconfig.script,f))
			return qe_invalid;
	}
	else
	{
		if (!p_getvar(&item_ref.scrconfig, f))
			return qe_invalid;
		if (!p_getvar(&item_ref.collect_scrconfig, f))
			return qe_invalid;
		if (!p_getvar(&item_ref.sprite_scrconfig, f))
			return qe_invalid;
	}
	
	if(!p_getc(&(item_ref.pickupflag),f))
		return qe_invalid;
	if(!p_getcstr(&item_ref.display_name,f))
		return qe_invalid;
	
	if(!p_igetw(&item_ref.pickup_litems,f))
		return qe_invalid;
	if(!p_igetw(&item_ref.pickup_litem_level,f))
		return qe_invalid;
	
	if (!p_igetl(&item_ref.moveflags, f))
		return qe_invalid;

	if(auto ret = read_weap_data(item_ref.weap_data, f))
		return ret;
	
	if (!p_igetl(&item_ref.cooldown, f))
		return qe_invalid;

	if (s_version >= 70)
	{
		if (!p_igetl(&item_ref.viewport_suspend_range, f))
			return qe_invalid;
		if (!p_igetl(&item_ref.viewport_despawn_range, f))
			return qe_invalid;
	}

	if (!should_skip)
	{
		if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
		{
			item_ref.scrconfig.clear();
			item_ref.weap_data.scrconfig.clear();
		}
	}
	return 0;
}

int32_t readitems(PACKFILE *f, word version, word build)
{
	if (version <= 0x192)
		return read_items_old(f, 0, version, build);
	
	word s_version;
	int32_t dummy;
	if(!p_igetw(&s_version,f))
		return qe_invalid;
	if (s_version > V_ITEMS)
		return qe_version;
	
	FFCore.quest_format[vItems] = s_version;
		
	if(!read_deprecated_section_cversion(f))
		return qe_invalid;
	
	//section size
	if(!p_igetl(&dummy,f))
		return qe_invalid;
	
	if (s_version < 67)
		return read_items_old(f, s_version, version, build);
	
	word items_to_read = 0;
	if (!p_igetw(&items_to_read, f))
		return qe_invalid;
	if (items_to_read > MAXITEMS)
		return qe_invalid;
	itemsbuf.clear();
	itemsbuf.reserve(items_to_read);

	for (word q = 0; q < items_to_read; ++q)
		if (auto ret = read_single_item(f, s_version, q, version, build))
			return ret;
	for (word q = 0; q < items_to_read; ++q)
		update_old_item(s_version, q, version, build);
	
	itemsbuf.normalize();
	
	return 0;
}

#include "core/qrs.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern dword loading_tileset_flags;
extern byte deprecated_rules[QUESTRULES_NEW_SIZE];

int32_t read_one_dmap(PACKFILE* f, zquestheader *Header, int s_version, int index)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_dmaps);
	dmap tempDMap;
	static AdvancedMusic tempMusic;
	char legacy_title[22];
	byte padding;
	
	if (s_version < 25)
		tempMusic.clear();
	
	if (!should_skip)
	{
		DMaps[index].clear();
		DMaps[index].type |= dmCAVE;
	}
	
	{
		tempDMap.clear();
		snprintf(legacy_title, sizeof(legacy_title), "                    ");
		snprintf(tempDMap.intro, sizeof(tempDMap.intro), "                                                                        ");
		
		if(!p_getc(&tempDMap.map,f))
		{
			return qe_invalid;
		}
		
		if(s_version <= 4)
		{
			byte tempbyte;
			
			if(!p_getc(&tempbyte,f))
			{
				return qe_invalid;
			}
			
			tempDMap.level=(word)tempbyte;
		}
		else
		{
			if(!p_igetw(&tempDMap.level,f))
			{
				return qe_invalid;
			}
		}
		
		if(!p_getc(&tempDMap.xoff,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&tempDMap.compass,f))
		{
			return qe_invalid;
		}
		
		if(s_version > 8) // February 2009
		{
			if(!p_igetw(&tempDMap.color,f))
			{
				return qe_invalid;
			}
		}
		else
		{
			byte tempbyte;
			
			if(!p_getc(&tempbyte,f))
			{
				return qe_invalid;
			}
			
			tempDMap.color = (word)tempbyte;
		}
		
		if (s_version < 25)
		{
			if(!p_getc(&padding,f))
				return qe_invalid;
			tempMusic.midi = convert_from_old_midi_id(padding + (MIDIOFFSET_DMAP-MIDIOFFSET_ZSCRIPT));
		}
		
		if(!p_getc(&tempDMap.cont,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&tempDMap.type,f))
		{
			return qe_invalid;
		}
		
		if((tempDMap.type & dmfTYPE) == dmOVERW &&
		   (!Header || Header->zelda_version >= 0x210)) // Not sure exactly when this changed
			tempDMap.xoff = 0;
		
		for(int32_t j=0; j<8; j++)
		{
			if(!p_getc(&tempDMap.grid[j],f))
			{
				return qe_invalid;
			}
		}
		
		if(Header && ((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<41))))
		{
			if(tempDMap.level>0&&tempDMap.level<10)
			{
				snprintf(legacy_title, sizeof(legacy_title), "LEVEL-%d             ", tempDMap.level);
			}
			tempDMap.title.assign(legacy_title);
			
			if(index==0 && Header->zelda_version <= 0x190)
			{
				tempDMap.cont = std::max((int)tempDMap.cont - tempDMap.xoff, 0);
				tempDMap.compass = std::max((int)tempDMap.compass - tempDMap.xoff, 0);
			}
			
			//forgotten -DD
			if(tempDMap.level==0)
			{
				tempDMap.flags=dmfCAVES|dmf3STAIR|dmfWHIRLWIND|dmfGUYCAVES;
			}
		}
		else
		{
			if(!p_getstr(tempDMap.name,sizeof(DMaps[0].name) - 1,f))
			{
				return qe_invalid;
			}
			
			if(s_version<20)
			{
				if (!p_getstr(legacy_title, sizeof(legacy_title) - 1, f))
				{
					return qe_invalid;
				}
				tempDMap.title.assign(legacy_title);
			}
			else
			{
				if (!p_getwstr(&tempDMap.title, f))
				{
					return qe_invalid;
				}
			}
			
			if(!p_getstr(tempDMap.intro,sizeof(DMaps[0].intro)-1,f))
			{
				return qe_invalid;
			}
			
			if(Header && ((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<152))))
			{
				if ((tempDMap.type & dmfTYPE) == dmOVERW) tempDMap.flags = dmfCAVES | dmf3STAIR | dmfWHIRLWIND | dmfGUYCAVES;
				DMaps[index] = tempDMap;
				
				return 0;
			}
			
			if(Header && (Header->zelda_version < 0x193))
			{
				if(!p_getc(&padding,f))
				{
					return qe_invalid;
				}
			}
			if ( s_version >= 11 )
			{
				if(!p_igetl(&tempDMap.minimap_tile[0],f))
				{
					return qe_invalid;
				}
			}
			else
			{
				if(!p_igetw(&tempDMap.minimap_tile[0],f))
				{
					return qe_invalid;
				}
			}
			
			if(!p_getc(&tempDMap.minimap_cset[0],f))
			{
				return qe_invalid;
			}
			
			if(Header && (Header->zelda_version < 0x193))
			{
				if(!p_getc(&padding,f))
				{
					return qe_invalid;
				}
			}
			
			if ( s_version >= 11 )
			{
				if(!p_igetl(&tempDMap.minimap_tile[1],f))
				{
					return qe_invalid;
				}
			}
			else
			{
				if(!p_igetw(&tempDMap.minimap_tile[1],f))
				{
					return qe_invalid;
				}
			}
			if(!p_getc(&tempDMap.minimap_cset[1],f))
			{
				return qe_invalid;
			}
			
			if(Header && (Header->zelda_version < 0x193))
			{
				if(!p_getc(&padding,f))
				{
					return qe_invalid;
				}
			}
			
			if ( s_version >= 11 )
			{
				if(!p_igetl(&tempDMap.largemap_tile[0],f))
				{
					return qe_invalid;
				}
			}
			else
			{
				if(!p_igetw(&tempDMap.largemap_tile[0],f))
				{
					return qe_invalid;
				}
			}
			
			if(!p_getc(&tempDMap.largemap_cset[0],f))
			{
				return qe_invalid;
			}
			
			if(Header && (Header->zelda_version < 0x193))
			{
			
				if(!p_getc(&padding,f))
				{
					return qe_invalid;
				}
			}
			
			if ( s_version >= 11 )
			{
				if(!p_igetl(&tempDMap.largemap_tile[1],f))
				{
					return qe_invalid;
				}
			}
			else
			{
				if(!p_igetw(&tempDMap.largemap_tile[1],f))
				{
					return qe_invalid;
				}
			}
			if(!p_getc(&tempDMap.largemap_cset[1],f))
			{
				return qe_invalid;
			}
			
			if (s_version < 25)
			{
				char tmusic[57] = {0};
				if(!p_getstr(tmusic,sizeof(tmusic)-1,f))
					return qe_invalid;
				tempMusic.enhanced.path = tmusic;
			}
		}
		
		if(s_version>1)
		{
			if (s_version < 25)
				if(!p_getc(&tempMusic.enhanced.track,f))
					return qe_invalid;
			
			if(!p_getc(&tempDMap.active_subscreen,f))
			{
				return qe_invalid;
			}
			
			if(!p_getc(&tempDMap.passive_subscreen,f))
			{
				return qe_invalid;
			}
		}
		
		if (s_version >= 26)
		{
			if (!p_getbitstr(&tempDMap.disabled_items, f))
				return qe_invalid;
		}
		else if (s_version > 2)
		{
			byte di[32];
			
			if(!pfread(&di, 32, f)) return qe_invalid;
			
			for(int32_t j=0; j<256; j++)
				tempDMap.disabled_items.set(j, di[j/8] & (1 << (j%8)));
		}
		
		if(s_version >= 6)
		{
			if(!p_igetl(&tempDMap.flags,f))
			{
				return qe_invalid;
			}
		}
		else if(s_version>3)
		{
			char temp;
			
			if(!p_getc(&temp,f))
			{
				return qe_invalid;
			}
			
			tempDMap.flags = temp;
		}
		else if(tempDMap.level==0 && ((Header->zelda_version < 0x211) || ((Header->zelda_version == 0x211) && (Header->build<18))))
		{
			tempDMap.flags=dmfCAVES|dmf3STAIR|dmfWHIRLWIND|dmfGUYCAVES;
		}
		else
			tempDMap.flags=0;
			
		if(s_version<7)
		{
			if(tempDMap.level==0 && get_bit(deprecated_rules,14))
				tempDMap.flags|= dmfVIEWMAP;
		}
		
		if(s_version<8)
		{
			if(tempDMap.level==0 && (tempDMap.type&dmfTYPE)==dmDNGN)
			{
				tempDMap.type &= ~dmDNGN;
				tempDMap.type |= dmCAVE;
			}
			else if((tempDMap.type&dmfTYPE)==dmCAVE)
			{
				tempDMap.flags |= dmfMINIMAPCOLORFIX;
			}
		}
		
		if(Header && ((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>=41)))
				&& (Header->zelda_version < 0x193))
		{
			if(!p_getc(&padding,f))
			{
				return qe_invalid;
			}
		}
	
		if(s_version >= 10)
		{
			if(!p_getc(&tempDMap.sideview,f))
			{
				return qe_invalid;
			}
		}
		if(s_version < 10) tempDMap.sideview = 0;
		
		//Dmap Scripts
		if (s_version >= 27)
		{
			if (!p_getvar(&tempDMap.active_scrconfig, f))
				return qe_invalid;
			if (!p_getvar(&tempDMap.active_sub_scrconfig, f))
				return qe_invalid;
			if (!p_getvar(&tempDMap.passive_sub_scrconfig, f))
				return qe_invalid;
			if (!p_getvar(&tempDMap.onmap_scrconfig, f))
				return qe_invalid;
		}
		else
		{
			if (s_version >= 12)
			{
				if (!p_igetw(&tempDMap.active_scrconfig.script,f))
					return qe_invalid;
				for ( int32_t q = 0; q < 8; q++ )
				{
					if(!p_igetl(&tempDMap.active_scrconfig.run_args[q],f))
						return qe_invalid;
				}
			}
			else
			{
				tempDMap.active_scrconfig.clear();
			}
			
			if(s_version >= 13)
			{
				for ( int32_t q = 0; q < 8; q++ )
					for ( int32_t w = 0; w < 65; w++ )
						if (!p_getc(&padding,f))
							return qe_invalid;
			}
			else
			{
				tempDMap.active_scrconfig.script = 0;
			}
			if(s_version >= 14)
			{
				if(!p_igetw(&tempDMap.active_sub_scrconfig.script,f))
					return qe_invalid;
				if(!p_igetw(&tempDMap.passive_sub_scrconfig.script,f))
					return qe_invalid;
				// These index the fixed-size dmapscripts[NUMSCRIPTSDMAP] table; a
				// file-controlled out-of-range value would cause an out-of-bounds read
				// and wild-pointer dereference in consumers, so sanitize here.
				if(tempDMap.active_sub_scrconfig.script >= NUMSCRIPTSDMAP)
					tempDMap.active_sub_scrconfig.script = 0;
				if(tempDMap.passive_sub_scrconfig.script >= NUMSCRIPTSDMAP)
					tempDMap.passive_sub_scrconfig.script = 0;
				for ( int32_t q = 0; q < 8; ++q )
				{
					if(!p_igetl(&tempDMap.active_sub_scrconfig.run_args[q],f))
						return qe_invalid;
					tempDMap.passive_sub_scrconfig.run_args[q] = tempDMap.active_sub_scrconfig.run_args[q];
				}
				for(int32_t q = 0; q < 8; ++q)
					for ( int32_t w = 0; w < 65; ++w )
						if(!p_getc(&padding,f))
							return qe_invalid;
			}
			else
			{
				tempDMap.active_sub_scrconfig.clear();
				tempDMap.passive_sub_scrconfig.clear();
			}
			if(s_version >= 15)
			{
				if(!p_igetw(&tempDMap.onmap_scrconfig.script,f))
					return qe_invalid;
				for ( int32_t q = 0; q < 8; ++q )
					if(!p_igetl(&tempDMap.onmap_scrconfig.run_args[q],f))
						return qe_invalid;
				for(int32_t q = 0; q < 8; ++q)
					for ( int32_t w = 0; w < 65; ++w )
						if(!p_getc(&padding,f))
							return qe_invalid;
			}
			else
			{
				tempDMap.onmap_scrconfig.clear();
			}
		}
		if(s_version >= 16)
		{
			if(!p_igetw(&tempDMap.mirrorDMap,f))
			{
				return qe_invalid;
			}
		}
		else
		{
			tempDMap.mirrorDMap = -1;
		}

		// Enhanced music loop points
		if (s_version >= 18 && s_version < 25)
		{
			if (!p_igetl(&tempMusic.enhanced.loop_start, f))
				return qe_invalid;
			if (!p_igetl(&tempMusic.enhanced.loop_end, f))
				return qe_invalid;
			if (!p_igetl(&tempMusic.enhanced.xfade_in, f))
				return qe_invalid;
			if (!p_igetl(&tempMusic.enhanced.xfade_out, f))
				return qe_invalid;
		}
		
		if(s_version >= 19)
			if(!p_getc(&tempDMap.overlay_subscreen, f))
				return qe_invalid;

		if (s_version >= 20)
		{
			if (!p_igetl(&tempDMap.intro_string_id, f))
				return qe_invalid;
		}
		else
			tempDMap.intro_string_id = 0;

		if(s_version == 21)
		{
			static regions_data tmp_rd;
			regions_data& rd = should_skip ? tmp_rd : Regions[tempDMap.map];
			for(int32_t j=0; j<8; j++)
            {
                for(int32_t k=0; k<8; k++)
                {
					if(!p_getc(&rd.region_ids[j][k],f))
					{
						return qe_invalid;
					}
				}
			}
		}

		if (s_version > 22 && (tempDMap.flags & dmfCUSTOM_GRAVITY))
		{
			if (!p_igetzf(&tempDMap.dmap_gravity, f))
				return qe_invalid;
			if (!p_igetzf(&tempDMap.dmap_terminal_v, f))
				return qe_invalid;
		}
		if(s_version > 23)
		{
			if(!p_igetw(&tempDMap.map_subscreen, f))
				return qe_invalid;
			if(!p_getc(&tempDMap.floor, f))
				return qe_invalid;
		}
		if (s_version >= 25)
		{
			if (!p_igetw(&tempDMap.music, f))
				return qe_invalid;
		}
		else if (!should_skip) // add an AdvancedMusic to the quest
		{
			if (tempMusic.is_empty())
				tempDMap.music = 0;
			else
			{
				tempMusic.name = fmt::format("DMap {}", index);
				auto& ref = quest_music.emplace_back(tempMusic);
				tempDMap.music = word(quest_music.size());
				ref.id = tempDMap.music;
			}
		}

		if (!should_skip)
		{
			if(loading_tileset_flags & TILESET_CLEARMAPS)
				tempDMap.map = 0;
			if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
			{
				tempDMap.active_scrconfig.clear();
				tempDMap.active_sub_scrconfig.clear();
				tempDMap.passive_sub_scrconfig.clear();
				tempDMap.onmap_scrconfig.clear();
			}
			DMaps[index] = tempDMap;
		}
	}
	return 0;
}

int32_t readdmaps(PACKFILE *f, zquestheader *Header, word, word, word start_dmap, word max_dmaps)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_dmaps);

	word dmapstoread=0;
	
	int32_t dummy;
	word s_version=0;

	if (!should_skip && s_version == 21)
		Regions = {};

	Header->is_z3 = false;
	if(!Header || Header->zelda_version > 0x192)
	{
		//section version info
		if(!p_igetw(&s_version,f))
		{
			return qe_invalid;
		}

		if (s_version > V_DMAPS)
			return qe_version;

		Header->is_z3 = s_version >= 22;
		FFCore.quest_format[vDMaps] = s_version;
		
		
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
		if(!p_igetw(&dmapstoread,f))
		{
			return qe_invalid;
		}
	}
	else
	{
		if((Header->zelda_version < 0x192)||
				((Header->zelda_version == 0x192)&&(Header->build<5)))
		{
			dmapstoread=32;
		}
		else if(s_version <= 4)
		{
			dmapstoread=OLDMAXDMAPS;
		}
		else
		{
			dmapstoread=MAXDMAPS;
		}
	}
	
	dmapstoread = zc_min(dmapstoread, max_dmaps);
	dmapstoread = zc_min(dmapstoread, MAXDMAPS-start_dmap);
	
	if (s_version < 25) // reserve space for the advanced music that will be created
		quest_music.reserve(quest_music.size() + dmapstoread);
	
	for(int i = start_dmap; i < dmapstoread + start_dmap; ++i)
	{
		if (int ret = read_one_dmap(f, Header, s_version, i))
			return ret;
	}
	
	return 0;
}

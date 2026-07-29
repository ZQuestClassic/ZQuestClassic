#include "core/misctypes.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;

namespace {

// Initializes miscQdata to the defaults used by the original NES quest
// (modules/classic/title_gfx.dat). Older quest formats only contain a subset of
// the misc data, so the rest must be initialized here; previously readmisc
// relied on title_gfx.dat having been loaded into QMisc beforehand to supply
// these baseline values.
void init_nes_misc_data(miscQdata *misc)
{
	*misc = {};

	for (auto& bt : misc->bottle_types)
		bt.clear();
	for (auto& bst : misc->bottle_shop_types)
		bst.clear();

	// Default sound effects. These mirror the legacy defaults applied below when
	// reading quest formats that predate the miscsfx section.
	misc->miscsfx[sfxBUSHGRASS] = WAV_ZN1GRASSCUT;
	misc->miscsfx[sfxLOWHEART] = WAV_ER;
	misc->miscsfx[sfxHURTPLAYER] = WAV_OUCH;
	misc->miscsfx[sfxHAMMERPOUND] = WAV_ZN1HAMMERPOST;
	misc->miscsfx[sfxSUBSCR_ITEM_ASSIGN] = WAV_PLACE;
	misc->miscsfx[sfxSUBSCR_CURSOR_MOVE] = WAV_CHIME;
	misc->miscsfx[sfxREFILL] = WAV_MSG;
	misc->miscsfx[sfxDRAIN] = WAV_MSG;
	misc->miscsfx[sfxTAP] = WAV_ZN1TAP;
	misc->miscsfx[sfxTAP_HOLLOW] = WAV_ZN1TAP2;

	misc->zscript_last_compiled_version = -1;
}

} // end namespace

int32_t readmisc(PACKFILE *f, zquestheader *Header, miscQdata *Misc)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_misc);

	word maxinfos=256;
	word maxshops=256;
	word shops=16, infos=16, warprings=8, palcycles=256, windwarps=9, triforces=8, icons=4;
	word ponds=16, pondsize=72, expansionsize=98*2;
	byte tempbyte, padding;
	miscQdata temp_misc;
	word s_version=0;
	word swaptmp;
	int32_t tempsize=0;

	init_nes_misc_data(&temp_misc);

	for(int32_t i=0; i<maxshops; ++i)
	{
		memset(&temp_misc.shop, 0, sizeof(shoptype)*256);
	}
	
	for(int32_t i=0; i<maxinfos; ++i)
	{
		memset(&temp_misc.info, 0, sizeof(infotype)*256);
	}

	memset(&temp_misc.warp, 0, sizeof(temp_misc.warp));
	for (auto& sm : temp_misc.save_menus)
		sm.clear();
	
	if(Header->zelda_version > 0x192)
	{
		//section version info
		if(!p_igetw(&s_version,f))
		{
			return qe_invalid;
		}

		if (s_version > V_MISC)
			return qe_version;
		
		FFCore.quest_format[vMisc] = s_version;
		
		if(!read_deprecated_section_cversion(f))
		{
			return qe_invalid;
		}
		
		
		//section size
		if(!p_igetl(&tempsize,f))
		{
			return qe_invalid;
		}
	}
	
	//finally...  section data
	readsize=0;
	
	//shops
	if(Header->zelda_version > 0x192)
	{
		if(!p_igetw(&shops,f))
		{
			return qe_invalid;
		}
	}

	if (shops > NUM_SHOPS)
	{
		return qe_invalid;
	}
	
	for(int32_t i=0; i<shops; i++)
	{
		if(s_version > 6)
		{
			if(!p_getstr(temp_misc.shop[i].name,sizeof(temp_misc.shop[i].name)-1,f))
			{
				return qe_invalid;
			}
		}
		
		for(int32_t j=0; j<3; j++)
		{
			if (s_version >= 20)
			{
				if(!p_igetw(&temp_misc.shop[i].item[j], f))
				{
					return qe_invalid;
				}
			}
			else if(!p_getc(&temp_misc.shop[i].item[j], f))
			{
				return qe_invalid;
			}
			
			if(s_version < 4)
			{
				temp_misc.shop[i].hasitem[j] = (temp_misc.shop[i].item[j] == 0) ? 0 : 1;
			}
		}
		
		if(Header->zelda_version < 0x193)
		{
			if(!p_getc(&tempbyte,f))
			{
				return qe_invalid;
			}
		}
		
		for(int32_t j=0; j<3; j++)
		{
			if(!p_igetw(&temp_misc.shop[i].price[j],f))
			{
				return qe_invalid;
			}
		}
		
		if (s_version > 3)
		{
			for(int32_t j=0; j<3; j++)
			{
				if(!p_getc(&temp_misc.shop[i].hasitem[j],f))
					return qe_invalid;
			}
		}
	
	/*
	if(s_version < 8)
		{
			for(int32_t j=0; j<3; j++)
			{
				(&temp_misc.shop[i].str[j])=0; //initialise.
			}
		}
	*/
	}
	
	//filter all the 0 items to the end (yeah, bubble sort; sue me)
	for(int32_t i=0; i<maxshops; ++i)
	{
		auto& shop = temp_misc.shop[i];
		for(int32_t j=0; j<3-1; j++)
		{
			for(int32_t k=0; k<2-j; k++)
			{
				if(!shop.hasitem[k])
				{
					zc_swap(shop.item[k], shop.item[k+1]);
					zc_swap(shop.price[k], shop.price[k+1]);
					zc_swap(shop.hasitem[k], shop.hasitem[k+1]);
				}
			}
		}
	}
	
	//infos
	if(Header->zelda_version > 0x192)
	{
		if(!p_igetw(&infos,f))
		{
			return qe_invalid;
		}
	}

	if (infos > NUM_INFOS)
	{
		return qe_invalid;
	}
	

	for(int32_t i=0; i<infos; i++)
	{
		if(s_version > 6)
		{
			if(!p_getstr(temp_misc.info[i].name,sizeof(temp_misc.info[i].name)-1,f))
			{
				return qe_invalid;
			}
		}
		
		for(int32_t j=0; j<3; j++)
		{
			if((Header->zelda_version < 0x192)||
					((Header->zelda_version == 0x192)&&(Header->build<146)))
			{
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
				}
				
				temp_misc.info[i].str[j]=tempbyte;
			}
			else
			{
				if(!p_igetw(&temp_misc.info[i].str[j],f))
				{
					return qe_invalid;
				}
			}
		}
		
		if(Header->zelda_version < 0x193)
		{
			if(!p_getc(&tempbyte,f))
			{
				return qe_invalid;
			}
		}
		
		if((Header->zelda_version == 0x192)&&(Header->build>145))
		{
			if(!p_getc(&padding,f))
			{
				return qe_invalid;
			}
		}
		
		for(int32_t j=0; j<3; j++)
		{
			if(!p_igetw(&temp_misc.info[i].price[j],f))
			{
				return qe_invalid;
			}
		}
	}
	
	//filter all the 0 strings to the end (yeah, bubble sort; sue me)
	for(int32_t i=0; i<maxinfos; ++i)
	{
		for(int32_t j=0; j<3-1; j++)
		{
			for(int32_t k=0; k<2-j; k++)
			{
				if(temp_misc.info[i].str[k]==0)
				{
					swaptmp = temp_misc.info[i].str[k];
					temp_misc.info[i].str[k] = temp_misc.info[i].str[k+1];
					temp_misc.info[i].str[k+1] = swaptmp;
					swaptmp = temp_misc.info[i].price[k];
					temp_misc.info[i].price[k] = temp_misc.info[i].price[k+1];
					temp_misc.info[i].price[k+1] = swaptmp;
				}
			}
		}
	}
	
	
	//warp rings
	if(s_version > 5)
		warprings++;
		
	if(Header->zelda_version > 0x192)
	{
		if(!p_igetw(&warprings,f))
		{
			return qe_invalid;
		}

		if (warprings > NUM_WARP_RINGS)
		{
			// return qe_invalid;
			// Note: we can't actually fail here because for some reason, some quest files have more than the max
			// number of possible warp rings. Some examples of this are: demosp253.qst, yuurand.qst
			// So instead below we disable `keepdata` when reading the bad warp ring data, so no memory is corrupted.
		}
	}
	
	for(int32_t i=0; i<warprings; i++)
	{
		// See above comment on the `warprings` range check.
		bool keepdata = i < NUM_WARP_RINGS;

		for(int32_t j=0; j<8+((s_version > 5)?1:0); j++)
		{
			if(s_version <= 3)
			{
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
				}

				if (keepdata)
					temp_misc.warp[i].dmap[j]=(word)tempbyte;
			}
			else
			{
				word tempword;
				if(!p_igetw(&tempword,f))
				{
					return qe_invalid;
				}

				if (keepdata)
					temp_misc.warp[i].dmap[j] = tempword;
			}
		}
		
		for(int32_t j=0; j<8+((s_version > 5)?1:0); j++)
		{
			if(!p_getc(&tempbyte,f))
			{
				return qe_invalid;
			}
			if (keepdata)
				temp_misc.warp[i].scr[j] = tempbyte;
		}

		if(!p_getc(&tempbyte,f))
		{
			return qe_invalid;
		}
		if (keepdata)
			temp_misc.warp[i].size = tempbyte;
		
		if(Header->zelda_version < 0x193)
		{
			if(!p_getc(&tempbyte,f))
			{
				return qe_invalid;
			}
		}
	}
	
	//palette cycles
	if(Header->zelda_version < 0x193)                         //in 1.93+, palette cycling is saved with the palettes
	{
		for(int32_t i=0; i<256; i++)
		{
			for(int32_t j=0; j<3; j++)
			{
				temp_misc.cycles[i][j].first=0;
				temp_misc.cycles[i][j].count=0;
				temp_misc.cycles[i][j].speed=0;
			}
		}
		
		if((Header->zelda_version < 0x192)||
				((Header->zelda_version == 0x192)&&(Header->build<73)))
		{
			palcycles=16;
		}
		
		for(int32_t i=0; i<palcycles; i++)
		{
			for(int32_t j=0; j<3; j++)
			{
				if(!p_getc(&temp_misc.cycles[i][j].first,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&temp_misc.cycles[i][j].count,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&temp_misc.cycles[i][j].speed,f))
				{
					return qe_invalid;
				}
			}
		}
	}
	
	//Wind warps are now just another warp ring.
	if(s_version <= 5)
	{
		if(Header->zelda_version > 0x192)
		{
			if(!p_igetw(&windwarps,f))
			{
				return qe_invalid;
			}
		}

		if (windwarps > NUM_WARP_RINGS)
		{
			return qe_invalid;
		}
		
		for(int32_t i=0; i<windwarps; i++)
		{
			if(s_version <= 3)
			{
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
				}
				
				temp_misc.warp[8].dmap[i]=tempbyte;
			}
			else
			{
				if(!p_igetw(&temp_misc.warp[8].dmap[i],f))
				{
					return qe_invalid;
				}
			}
			
			if(!p_getc(&temp_misc.warp[8].scr[i],f))
			{
				return qe_invalid;
			}
			
			temp_misc.warp[8].size = 9;
			
			if(s_version == 5)
			{
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
				}
			}
		}
	}
	
	
	//triforce pieces
	for(int32_t i=0; i<triforces; i++)
	{
		if(!p_getc(&temp_misc.triforce[i],f))
		{
			return qe_invalid;
		}
	}
	
	//misc color data
	if(s_version<3)
	{
		if(!p_getc(&temp_misc.colors.text,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.caption,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.overw_bg,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.dngn_bg,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.dngn_fg,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.cave_fg,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.bs_dk,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.bs_goal,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.compass_lt,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.compass_dk,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.subscr_bg,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.triframe_color,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.hero_dot,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.bmap_bg,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.bmap_fg,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.triforce_cset,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.triframe_cset,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.overworld_map_cset,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.dungeon_map_cset,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.blueframe_cset,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&temp_misc.colors.triforce_tile,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&temp_misc.colors.triframe_tile,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&temp_misc.colors.overworld_map_tile,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&temp_misc.colors.dungeon_map_tile,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&temp_misc.colors.blueframe_tile,f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&temp_misc.colors.HCpieces_tile,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&temp_misc.colors.HCpieces_cset,f))
		{
			return qe_invalid;
		}
		
		temp_misc.colors.msgtext = 0x01;
		
		if(Header->zelda_version < 0x193)
		{
			for(int32_t i=0; i<7; i++)
			{
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
				}
			}
		}
		
		if((Header->zelda_version == 0x192)&&(Header->build>145))
		{
			for(int32_t i=0; i<256; i++)
			{
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
				}
			}
		}
		
		if(s_version>1)
		{
			if(!p_getc(&temp_misc.colors.subscr_shadow,f))
			{
				return qe_invalid;
			}
		}
		
		//save game icons
		if((Header->zelda_version < 0x192)||
				((Header->zelda_version == 0x192)&&(Header->build<73)))
		{
			icons=3;
		}
		
		for(int32_t i=0; i<icons; i++)
		{
			if(!p_igetw(&temp_misc.icons[i],f))
			{
				return qe_invalid;
			}
		}
	}
	
	if((Header->zelda_version < 0x192)||
			((Header->zelda_version == 0x192)&&(Header->build<30)))
	{
		*Misc = temp_misc;
		
		return 0;
	}
	
	//pond information
	if(Header->zelda_version < 0x193)
	{
		if((Header->zelda_version == 0x192)&&(Header->build<146))
		{
			pondsize=25;
		}
		
		for(int32_t i=0; i<ponds; i++)
		{
			for(int32_t j=0; j<pondsize; j++)
			{
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
					
				}
			}
		}
	}
	
	//end string
	if((Header->zelda_version < 0x192)||
			((Header->zelda_version == 0x192)&&(Header->build<146)))
	{
		if(!p_getc(&tempbyte,f))
		{
			return qe_invalid;
		}
		
		temp_misc.endstring=tempbyte;
		
		if(!p_getc(&tempbyte,f))
		{
			return qe_invalid;
		}
	}
	else
	{
		if(!p_igetw(&temp_misc.endstring,f))
		{
			return qe_invalid;
		}
	}
	
	//expansion
	if(Header->zelda_version < 0x193)
	{
		if((Header->zelda_version == 0x192)&&(Header->build<73))
		{
			expansionsize=99*2;
		}
		
		for(int32_t i=0; i<expansionsize; i++)
		{
			if(!p_getc(&tempbyte,f))
			{
				return qe_invalid;
			}
		}
	}
	//shops v8
	
	
	if(s_version >= 8)
	{
		for(int32_t i=0; i<shops; i++)
		{
			for(int32_t j=0; j<3; j++)
			{
				if(!p_igetw(&temp_misc.shop[i].str[j],f))
					return qe_invalid;
			}
		}
	}
	
	memset(&temp_misc.questmisc, 0, sizeof(int32_t)*32);
	memset(&temp_misc.zscript_last_compiled_version, 0, sizeof(int32_t));
	
	//v9 includes quest misc[32]
	// ... this has been deprecated (2024)
	if(s_version >= 9)
	{
		for ( int32_t q = 0; q < 32; q++ ) 
		{
			if(!p_igetl(&temp_misc.questmisc[q],f))
						return qe_invalid;
		}
		// this was string labels
		if (pack_fseek(f, 32 * 128))
			return qe_invalid;
	}
	
	if(s_version >= 11 )
	{
		if(!p_igetl(&temp_misc.zscript_last_compiled_version,f))
			return qe_invalid;
	}
	else if(s_version < 11 )
	{
		temp_misc.zscript_last_compiled_version = -1;
	}
	
	FFCore.quest_format[vLastCompile] = temp_misc.zscript_last_compiled_version;
	
	if(s_version >= 21)
	{
		for(int32_t q = 0; q < sprMAX; ++q)
		{
			if(!p_igetw(&temp_misc.sprites[q],f))
				return qe_invalid;
		}
	}
	else if(s_version >= 12)
	{
		byte spr;
		for(int32_t q = 0; q < sprMAX; ++q)
		{
			if(!p_getc(&spr,f))
				return qe_invalid;
			temp_misc.sprites[q] = spr;
		}
	}
	else
	{
		memset(&(temp_misc.sprites), 0, sizeof(temp_misc.sprites));
		//temp_misc.sprites[sprFALL] = ;
	}
	
	if(s_version >= 13)
	{
		for(size_t q = 0; q < 64; ++q)
		{
			bottletype* bt = &(temp_misc.bottle_types[q]);
            if (!p_getstr(bt->name, sizeof(bt->name)-1, f))
                return qe_invalid;
			for(size_t j = 0; j < 3; ++j)
			{
                if (!p_getc(&(bt->counter[j]), f))
                    return qe_invalid;
                if (!p_igetw(&(bt->amount[j]), f))
                    return qe_invalid;
			}
            if (!p_getc(&(bt->flags), f))
                return qe_invalid;
            if (!p_getc(&(bt->next_type), f))
                return qe_invalid;
		}
		for(size_t q = 0; q < 256; ++q)
		{
			bottleshoptype* bst = &(temp_misc.bottle_shop_types[q]);
            if (!pfread(bst->name, sizeof(bst->name)-1, f))
                return qe_invalid;
			for(size_t j = 0; j < 3; ++j)
			{
                if (!p_getc(&(bst->fill[j]), f))
                    return qe_invalid;
                if (!p_igetw(&(bst->comb[j]), f))
                    return qe_invalid;
                if (!p_getc(&(bst->cset[j]), f))
                    return qe_invalid;
                if (!p_igetw(&(bst->price[j]), f))
                    return qe_invalid;
                if (!p_igetw(&(bst->str[j]), f))
                    return qe_invalid;
			}
		}
	}
	else
	{
		for(size_t q = 0; q < 64; ++q)
			temp_misc.bottle_types[q].clear();
		for(size_t q = 0; q < 256; ++q)
			temp_misc.bottle_shop_types[q].clear();
	}
	
	if(s_version >= 14)
	{
		if (s_version < 19)
		{
			byte msfx;
			for(int32_t q = 0; q < sfxMAX; ++q)
			{
				if(!p_getc(&msfx,f))
					return qe_invalid;
				temp_misc.miscsfx[q] = msfx;
			}
		}
		else
		{
			word msfx;
			for(int32_t q = 0; q < sfxMAX; ++q)
			{
				if(!p_igetw(&msfx,f))
					return qe_invalid;
				temp_misc.miscsfx[q] = msfx;
			}
		}
	}
	else
	{
		memset(&(temp_misc.miscsfx), 0, sizeof(temp_misc.miscsfx));
		temp_misc.miscsfx[sfxBUSHGRASS] = WAV_ZN1GRASSCUT;
		temp_misc.miscsfx[sfxLOWHEART] = WAV_ER;
	}
	if(s_version < 15)
	{
		temp_misc.miscsfx[sfxHURTPLAYER] = WAV_OUCH;
		temp_misc.miscsfx[sfxHAMMERPOUND] = WAV_ZN1HAMMERPOST;
		temp_misc.miscsfx[sfxSUBSCR_ITEM_ASSIGN] = WAV_PLACE;
		temp_misc.miscsfx[sfxSUBSCR_CURSOR_MOVE] = WAV_CHIME;
		temp_misc.miscsfx[sfxREFILL] = WAV_MSG;
		temp_misc.miscsfx[sfxDRAIN] = WAV_MSG;
	}
	if(s_version < 16)
	{
		temp_misc.miscsfx[sfxTAP] = WAV_ZN1TAP;
		temp_misc.miscsfx[sfxTAP_HOLLOW] = WAV_ZN1TAP2;
	}
	if (s_version >= 17)
	{
		byte save_menu_count = 0;
		if (!p_getc(&save_menu_count, f))
			return qe_invalid;
		if (save_menu_count > NUM_SAVE_MENUS) // file-controlled; guard against OOB write into save_menus
			return qe_invalid;
		for(size_t q = 0; q < save_menu_count; ++q)
		{
			SaveMenu& menu = temp_misc.save_menus[q];
			menu.clear();
			
			byte menu_empty;
			if (!p_getc(&menu_empty, f))
				return qe_invalid;
			if (menu_empty)
				continue;
			
			if (!p_getcstr(&menu.name, f))
				return qe_invalid;
			
			if (!p_igetw(&menu.flags, f))
				return qe_invalid;
			
			if (!p_igetl(&menu.cursor_tile, f))
				return qe_invalid;
			
			if (!p_getc(&menu.cursor_cset, f))
				return qe_invalid;
			
			if (s_version < 19)
			{
				if (!p_getc(&tempbyte, f))
					return qe_invalid;
				menu.cursor_sfx = tempbyte;
				if (!p_getc(&tempbyte, f))
					return qe_invalid;
				menu.choose_sfx = tempbyte;
			}
			else
			{
				if (!p_igetw(&menu.cursor_sfx, f))
					return qe_invalid;
				if (!p_igetw(&menu.choose_sfx, f))
					return qe_invalid;
			}
			
			if (!p_getc(&menu.bg_color, f))
				return qe_invalid;
			
			if (!p_getc(&menu.hspace, f))
				return qe_invalid;
			
			if (!p_getc(&menu.vspace, f))
				return qe_invalid;
			
			if (!p_getc(&menu.opt_x, f))
				return qe_invalid;
			
			if (!p_getc(&menu.opt_y, f))
				return qe_invalid;
			
			if (!p_getc(&menu.text_align, f))
				return qe_invalid;
			
			if (!p_getc(&menu.textbox_align, f))
				return qe_invalid;
			
			if (!p_igetw(&menu.close_frames, f))
				return qe_invalid;
			
			if (!p_getc(&menu.close_flash_rate, f))
				return qe_invalid;
			
			if (s_version < 18)
			{
				int16_t midi;
				if (!p_igetw(&midi, f))
					return qe_invalid;
				if (should_skip)
					menu.music = 0;
				else menu.music = find_or_make_midi_music(convert_from_old_midi_id(midi));
			}
			else if (!p_igetw(&menu.music, f))
				return qe_invalid;
			
			if (!p_igetl(&menu.bg_tile, f))
				return qe_invalid;
			
			if (!p_getc(&menu.bg_cset, f))
				return qe_invalid;
			
			if (!p_getc(&menu.bg_tw, f))
				return qe_invalid;
			
			if (!p_getc(&menu.bg_th, f))
				return qe_invalid;
			
			byte opt_count;
			if (!p_getc(&opt_count, f))
				return qe_invalid;
			menu.options.resize(opt_count);
			
			for (size_t q = 0; q < opt_count; ++q)
			{
				SaveMenuOption& opt = menu.options[q];
				
				if (!p_getcstr(&opt.text, f))
					return qe_invalid;
				
				if (!p_igetw(&opt.flags, f))
					return qe_invalid;
				
				if (!p_getc(&opt.color, f))
					return qe_invalid;
				
				if (!p_getc(&opt.picked_color, f))
					return qe_invalid;
				
				if (!p_igetl(&opt.font, f))
					return qe_invalid;
				
				if (s_version >= 22)
				{
					if (!p_getvar(&opt.gen_scrconfig, f))
						return qe_invalid;
				}
				else
				{
					if (!p_igetw(&opt.gen_scrconfig.script, f))
						return qe_invalid;
				}
			}
		}
		
		if (!p_getc(&temp_misc.savemenu_game_over, f))
			return qe_invalid;
		if (!p_getc(&temp_misc.savemenu_f6, f))
			return qe_invalid;
	}
	
	if (!should_skip)
		*Misc = temp_misc;
	
	return 0;
}

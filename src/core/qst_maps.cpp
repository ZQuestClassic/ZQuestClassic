#include "core/qrs.h"
#include "core/qst.h"
#include "zalleg/packfile.h"
#include "zc/ffscript.h"

extern const byte* legacy_skip_flags;
extern dword loading_tileset_flags;
extern bool fixffcs;
extern std::vector<byte> old_combo_pages;

namespace {

int32_t doortranslations_u[9][4]=
{
    {37,38,53,54},
    {37,38,39,40},
    {37,38,55,56},
    {37,38,39,40},
    {37,38,53,54},
    {37,38,53,54},
    {37,38,53,54},
    {7,8,23,24},
    {7,8,41,42}
};

int32_t doortranslations_d[9][4]=
{
    {117,118,133,134},
    {135,136,133,134},
    {119,120,133,134},
    {135,136,133,134},
    {117,118,133,134},
    {117,118,133,134},
    {117,118,133,134},
    {151,152,167,168},
    {137,138,167,168},
};

//enum {dt_pass=0, dt_lock, dt_shut, dt_boss, dt_olck, dt_osht, dt_obos, dt_wall, dt_bomb, dt_walk, dt_max};
int32_t doortranslations_l[9][6]=
{
    {66,67,82,83,98,99},
    {66,68,82,84,98,100},
    {66,69,82,85,98,101},
    {66,68,82,84,98,100},
    {66,67,82,83,98,99},
    {66,67,82,83,98,99},
    {66,67,82,83,98,99},
    {64,65,80,81,96,97},
    {64,65,80,114,96,97},
};

int32_t doortranslations_r[9][6]=
{

    {76,77,92,93,108,109},
    {75,77,91,93,107,109},
    {74,77,90,93,106,109},
    {75,77,91,93,107,109},
    {76,77,92,93,108,109},
    {76,77,92,93,108,109},
    {76,77,92,93,108,109},
    {78,79,94,95,110,111},
    {78,79,125,95,110,111},
};

int32_t tdcmbdat(int32_t map, int32_t scr, int32_t pos)
{
	byte old_cpage = old_combo_pages[map*MAPSCRS+scr];
    return (TheMaps[map*MAPSCRS+TEMPLATE].data[pos]&0xFF) + (old_cpage<<8);
}

int32_t tdcmbcset(int32_t map, int32_t scr, int32_t pos)
{
    //these are here to bypass compiler warnings about unused arguments
    map=map;
    scr=scr;
    pos=pos;
    
    //what does this function do?
    //  return TheMaps[map*MAPSCRS+TEMPLATE].cset[pos];
    return 2;
}

int32_t tcmbdat2(int32_t map, int32_t scr, int32_t pos)
{
	byte old_cpage = old_combo_pages[map*MAPSCRS+scr];
    return (TheMaps[map*MAPSCRS+TEMPLATE2].data[pos]&0xFF) + (old_cpage<<8);
}

int32_t tcmbcset2(int32_t map, int32_t pos)
{

    return TheMaps[map*MAPSCRS+TEMPLATE2].cset[pos];
}

int32_t tcmbflag2(int32_t map, int32_t pos)
{
    return TheMaps[map*MAPSCRS+TEMPLATE2].sflag[pos];
}

int32_t MakeDoors(int32_t map, int32_t scr)
{
    if(!(TheMaps[map*MAPSCRS+scr].valid&mVALID))
    {
        return 0;
    }
    
    DoorComboSet tempdcs{};
    
    //up
    for(int32_t i=0; i<9; i++)
    {
        for(int32_t j=0; j<4; j++)
        {
            tempdcs.doorcombo_u[i][j]=tdcmbdat(map,scr,doortranslations_u[i][j]);
            tempdcs.doorcset_u[i][j]=tdcmbcset(map,scr,doortranslations_u[i][j]);
        }
    }
    
    tempdcs.bombdoorcombo_u[0]=tdcmbdat(map,scr,57);
    tempdcs.bombdoorcset_u[0]=tdcmbcset(map,scr,57);
    tempdcs.bombdoorcombo_u[1]=tdcmbdat(map,scr,58);
    tempdcs.bombdoorcset_u[1]=tdcmbcset(map,scr,58);
    tempdcs.walkthroughcombo[0]=tdcmbdat(map,scr,34);
    tempdcs.walkthroughcset[0]=tdcmbdat(map,scr,34);
    
    //down
    for(int32_t i=0; i<9; i++)
    {
        for(int32_t j=0; j<4; j++)
        {
            tempdcs.doorcombo_d[i][j]=tdcmbdat(map,scr,doortranslations_d[i][j]);
            tempdcs.doorcset_d[i][j]=tdcmbcset(map,scr,doortranslations_d[i][j]);
        }
    }
    
    tempdcs.bombdoorcombo_d[0]=tdcmbdat(map,scr,121);
    
    tempdcs.bombdoorcset_d[0]=tdcmbcset(map,scr,121);
    tempdcs.bombdoorcombo_d[1]=tdcmbdat(map,scr,122);
    tempdcs.bombdoorcset_d[1]=tdcmbcset(map,scr,122);
    tempdcs.walkthroughcombo[1]=tdcmbdat(map,scr,34);
    tempdcs.walkthroughcset[1]=tdcmbdat(map,scr,34);
    
    //left
    //        TheMaps[i*MAPSCRS+j].warpdmap=TheOldMap.warpdmap;
    for(int32_t i=0; i<9; i++)
    {
        for(int32_t j=0; j<6; j++)
        {
            tempdcs.doorcombo_l[i][j]=tdcmbdat(map,scr,doortranslations_l[i][j]);
            tempdcs.doorcset_l[i][j]=tdcmbcset(map,scr,doortranslations_l[i][j]);
        }
    }
    
    for(int32_t j=0; j>6; j++)
    {
        if((j!=2)&&(j!=3))
        {
            tempdcs.doorcombo_l[dt_bomb][j]=TheMaps[map*MAPSCRS+scr].data[doortranslations_l[dt_bomb][j]];
            tempdcs.doorcset_l[dt_bomb][j]=TheMaps[map*MAPSCRS+scr].cset[doortranslations_l[dt_bomb][j]];
        }
    }
    
    tempdcs.bombdoorcombo_l[0]=0;
    tempdcs.bombdoorcset_l[0]=tdcmbcset(map,scr,115);
    tempdcs.bombdoorcombo_l[1]=tdcmbdat(map,scr,115);
    tempdcs.bombdoorcset_l[1]=tdcmbcset(map,scr,115);
    tempdcs.bombdoorcombo_l[2]=0;
    tempdcs.bombdoorcset_l[2]=tdcmbcset(map,scr,115);
    tempdcs.walkthroughcombo[2]=tdcmbdat(map,scr,34);
    tempdcs.walkthroughcset[2]=tdcmbdat(map,scr,34);
    
    //right
    for(int32_t i=0; i<9; i++)
    {
        for(int32_t j=0; j<6; j++)
        {
            tempdcs.doorcombo_r[i][j]=tdcmbdat(map,scr,doortranslations_r[i][j]);
            tempdcs.doorcset_r[i][j]=tdcmbcset(map,scr,doortranslations_r[i][j]);
        }
    }
    
    for(int32_t j=0; j>6; j++)
    {
        if((j!=2)&&(j!=3))
        {
            tempdcs.doorcombo_r[dt_bomb][j]=TheMaps[map*MAPSCRS+scr].data[doortranslations_r[dt_bomb][j]];
            tempdcs.doorcset_r[dt_bomb][j]=TheMaps[map*MAPSCRS+scr].cset[doortranslations_r[dt_bomb][j]];
        }
    }
    
    tempdcs.bombdoorcombo_r[0]=0;
    tempdcs.bombdoorcset_r[0]=tdcmbcset(map,scr,124);
    tempdcs.bombdoorcombo_r[1]=tdcmbdat(map,scr,124);
    tempdcs.bombdoorcset_r[1]=tdcmbcset(map,scr,124);
    tempdcs.bombdoorcombo_r[2]=0;
    tempdcs.bombdoorcset_r[2]=tdcmbcset(map,scr,124);
    tempdcs.walkthroughcombo[3]=tdcmbdat(map,scr,34);
    tempdcs.walkthroughcset[3]=tdcmbdat(map,scr,34);
    
    int32_t k;
    
    for(k=0; k<door_combo_set_count; k++)
    {
        if(DoorComboSets[k]==tempdcs)
        {
            break;
        }
    }

    if (k >= DoorComboSets.size())
    {
        return 0;
    }

    if(k==door_combo_set_count)
    {
        DoorComboSets[k]=tempdcs;
        DoorComboSetNames[k] = fmt::format("Door Combo Set {}", k);
        ++door_combo_set_count;
    }
    
    return k;
    /*
      doorcombo_u[9][4];
      doorcset_u[9][4];
      doorcombo_d[9][4];
      doorcset_d[9][4];
      doorcombo_l[9][6];
      doorcset_l[9][6];
      doorcombo_r[9][6];
      doorcset_r[9][6];
      bombdoorcombo_u[2];
      bombdoorcset_u[2];
      bombdoorcombo_d[2];
      bombdoorcset_d[2];
      bombdoorcombo_l[3];
      bombdoorcset_l[3];
      bombdoorcombo_r[3];
      bombdoorcset_r[3];
      walkthroughcombo[4];
      walkthroughcset[4];
      */
}

int32_t readmapscreen_old(PACKFILE *f, zquestheader *Header, mapscr *temp_mapscr, word version, int scrind, bool keep_music)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_maps);

	byte tempbyte, padding;
	word wpadding;
	int32_t extras, secretcombos;
	if(!p_getc(&(temp_mapscr->valid),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(temp_mapscr->guy),f))
		return qe_invalid;
	temp_mapscr->guytile = -1; //signal to use default guy values
	SETFLAG(temp_mapscr->roomflags,RFL_ALWAYS_GUY,temp_mapscr->guy==gFAIRY);
	SETFLAG(temp_mapscr->roomflags,RFL_GUYFIRES,temp_mapscr->guy!=gFAIRY || !get_qr(qr_NOFAIRYGUYFIRES));
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<146)))
	{
		if(!p_getc(&tempbyte,f))
		{
			return qe_invalid;
		}
		
		temp_mapscr->str=tempbyte;
	}
	else
	{
		if(!p_igetw(&(temp_mapscr->str),f))
		{
			return qe_invalid;
		}
	}
	
	if(!p_getc(&(temp_mapscr->room),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&tempbyte,f))
		return qe_invalid;
	temp_mapscr->item = tempbyte;
	
	if(Header->zelda_version < 0x211 || (Header->zelda_version == 0x211 && Header->build < 14))
	{
		temp_mapscr->hasitem = (temp_mapscr->item != 0) ? 1 : 0;
	}
	else
	{
		if(!p_getc(&(temp_mapscr->hasitem),f))
			return qe_invalid;
	}
	
	if((Header->zelda_version < 0x192)||
		((Header->zelda_version == 0x192)&&(Header->build<154)))
	{
		if(!p_getc(&tempbyte,f))
		{
			return qe_invalid;
		}
	}
	
	if(!p_getc(&(temp_mapscr->tilewarptype[0]),f))
	{
		return qe_invalid;
	}
	
	if(Header->zelda_version < 0x193)
	{
		if(!p_getc(&tempbyte,f))
		{
			return qe_invalid;
		}
	}
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
	{
		for(int32_t i=1; i<4; i++)
		{
			if(!p_getc(&(temp_mapscr->tilewarptype[i]),f))
			{
				return qe_invalid;
			}
		}
	}
	else
	{
		temp_mapscr->tilewarptype[1]=0;
		temp_mapscr->tilewarptype[2]=0;
		temp_mapscr->tilewarptype[3]=0;
	}
	
	if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>153)))
	{
		if(!p_igetw(&(temp_mapscr->door_combo_set),f))
		{
			return qe_invalid;
		}
	}
	
	if(!p_getc(&(temp_mapscr->warpreturnx[0]),f))
	{
		return qe_invalid;
	}
	
	temp_mapscr->warpreturnx[1]=0;
	temp_mapscr->warpreturnx[2]=0;
	temp_mapscr->warpreturnx[3]=0;
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
	{
		for(int32_t i=1; i<4; i++)
		{
			if(!p_getc(&(temp_mapscr->warpreturnx[i]),f))
			{
				return qe_invalid;
			}
		}
	}
	
	if(!p_getc(&(temp_mapscr->warpreturny[0]),f))
	{
		return qe_invalid;
	}
	
	temp_mapscr->warpreturny[1]=0;
	temp_mapscr->warpreturny[2]=0;
	temp_mapscr->warpreturny[3]=0;
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
	{
		for(int32_t i=1; i<4; i++)
		{
			if(!p_getc(&(temp_mapscr->warpreturny[i]),f))
			{
				return qe_invalid;
			}
		}
		
		if(version>=18)
		{
			if(!p_igetw(&temp_mapscr->warpreturnc,f))
			{
				return qe_invalid;
			}
		}
		else
		{
			byte temp;
			
			if(!p_getc(&temp,f))
			{
				return qe_invalid;
			}
			
			temp_mapscr->warpreturnc=temp<<8|temp;
		}
	}
	
	if(!p_getc(&(temp_mapscr->stairx),f))
	
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(temp_mapscr->stairy),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(temp_mapscr->itemx),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(temp_mapscr->itemy),f))
	{
		return qe_invalid;
	}
	
	if(version > 15) // February 2009
	{
		if(!p_igetw(&(temp_mapscr->color),f))
		{
			return qe_invalid;
		}
	}
	else
	{
		if(!p_getc(& tempbyte,f))
		{
			return qe_invalid;
		}
		
		temp_mapscr->color = (word) tempbyte;
	}
	
	if(!p_getc(&(temp_mapscr->flags11),f))
	{
		return qe_invalid;
	}
	
	for(int32_t k=0; k<4; k++)
	{
		if(!p_getc(&(temp_mapscr->door[k]),f))
		{
			return qe_invalid;
			
		}
	}
	
	if(version <= 11)
	{
		if(!p_getc(&(tempbyte),f))
		{
			return qe_invalid;
		}
		
		temp_mapscr->tilewarpdmap[0]=(word)tempbyte;
		
		if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
		{
			for(int32_t i=1; i<4; i++)
			{
				if(!p_getc(&(tempbyte),f))
				{
					return qe_invalid;
				}
				
				temp_mapscr->tilewarpdmap[i]=(word)tempbyte;
			}
		}
		else
		{
			temp_mapscr->tilewarpdmap[1]=0;
			temp_mapscr->tilewarpdmap[2]=0;
			temp_mapscr->tilewarpdmap[3]=0;
		}
	}
	else
	{
		for(int32_t i=0; i<4; i++)
		{
			if(!p_igetw(&(temp_mapscr->tilewarpdmap[i]),f))
			{
				return qe_invalid;
			}
		}
	}
	
	if(!p_getc(&(temp_mapscr->tilewarpscr[0]),f))
	{
		return qe_invalid;
	}
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
	{
		for(int32_t i=1; i<4; i++)
		{
			if(!p_getc(&(temp_mapscr->tilewarpscr[i]),f))
			{
				return qe_invalid;
			}
		}
	}
	else
	{
		temp_mapscr->tilewarpscr[1]=0;
		temp_mapscr->tilewarpscr[2]=0;
		temp_mapscr->tilewarpscr[3]=0;
	}
	
	if(version >= 15)
	{
		if(!p_getc(&(temp_mapscr->tilewarpoverlayflags),f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_mapscr->tilewarpoverlayflags=0;
	}
	
	if(!p_getc(&(temp_mapscr->exitdir),f))
	{
		return qe_invalid;
	}
	
	if(Header->zelda_version < 0x193)
	{
		if(!p_getc(&tempbyte,f))
		{
			return qe_invalid;
		}
		
	}
	
	if((Header->zelda_version == 0x192)&&(Header->build>145)&&(Header->build<154))
	{
		if(!p_getc(&padding,f))
		{
			return qe_invalid;
		}
	}
	
	for(int32_t k=0; k<10; k++)
	{
		if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<10)))
		{
			if(!p_getc(&tempbyte,f))
			{
				return qe_invalid;
			}
			
			temp_mapscr->enemy[k]=tempbyte;

			// 76 is the highest enemy id possible to set in 1.90. Anything higher must have come
			// from corrupting when 1.90 saved the quest.
			// https://discord.com/channels/876899628556091432/1287580827164737658
			if (Header->zelda_version <= 0x190)
			{
				if (temp_mapscr->enemy[k] > 76)
				{
					temp_mapscr->enemy[k] = 0;
				}
			}
		}
		else
		{
			if(!p_igetw(&(temp_mapscr->enemy[k]),f))
			{
				return qe_invalid;
			}
		}
		
		if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<108)))
		{
			//using enumerations here is dangerous
			//very easy to break old quests -DD
			if(temp_mapscr->enemy[k]>=57)  //old eGOHMA1
			{
				temp_mapscr->enemy[k]+=5;
			}
			else if(temp_mapscr->enemy[k]>=52)  //old eGLEEOK2
			{
				temp_mapscr->enemy[k]+=1;
			}
		}

		if(version < 9)
		{
			if(temp_mapscr->enemy[k]>0)
			{
				temp_mapscr->enemy[k]+=10;
			}
		}
		//don't read in any invalid data
		if ( ((unsigned)temp_mapscr->enemy[k]) > MAXGUYS )
		{
			al_trace("Tried to read an invalid enemy ID (%d) for enemy[%d]. This has been cleared to 0.\n", temp_mapscr->enemy[k], k);
			temp_mapscr->enemy[k] = 0;
		}
	}
	
	if(!p_getc(&(temp_mapscr->pattern),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(temp_mapscr->sidewarptype[0]),f))
	{
		return qe_invalid;
	}
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
	{
		for(int32_t i=1; i<4; i++)
		{
			if(!p_getc(&(temp_mapscr->sidewarptype[i]),f))
			{
				return qe_invalid;
			}
		}
	}
	else
	{
		temp_mapscr->sidewarptype[1]=0;
		temp_mapscr->sidewarptype[2]=0;
		temp_mapscr->sidewarptype[3]=0;
	}
	
	if(version >= 15)
	{
		if(!p_getc(&(temp_mapscr->sidewarpoverlayflags),f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_mapscr->sidewarpoverlayflags=0;
	}
	
	if(!p_getc(&(temp_mapscr->warparrivalx),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(temp_mapscr->warparrivaly),f))
	{
		return qe_invalid;
	}
	
	for(int32_t k=0; k<4; k++)
	{
		if(!p_getc(&(temp_mapscr->path[k]),f))
		{
			return qe_invalid;
		}
	}
	
	if(!p_getc(&(temp_mapscr->sidewarpscr[0]),f))
	{
		return qe_invalid;
	}
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
	{
		for(int32_t i=1; i<4; i++)
		{
			if(!p_getc(&(temp_mapscr->sidewarpscr[i]),f))
			{
				return qe_invalid;
			}
		}
	}
	else
	{
		temp_mapscr->sidewarpscr[1]=0;
		temp_mapscr->sidewarpscr[2]=0;
		temp_mapscr->sidewarpscr[3]=0;
	}
	
	if(version <= 11)
	{
		if(!p_getc(&(tempbyte),f))
		{
			return qe_invalid;
		}
		
		temp_mapscr->sidewarpdmap[0]=(word)tempbyte;
		
		if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
		{
			for(int32_t i=1; i<4; i++)
			{
				if(!p_getc(&(tempbyte),f))
				{
					return qe_invalid;
				}
				
				temp_mapscr->sidewarpdmap[i]=(word)tempbyte;
			}
		}
		else
		{
			temp_mapscr->sidewarpdmap[1]=0;
			temp_mapscr->sidewarpdmap[2]=0;
			temp_mapscr->sidewarpdmap[3]=0;
		}
	}
	else
	{
		for(int32_t i=0; i<4; i++)
		{
			if(!p_igetw(&(temp_mapscr->sidewarpdmap[i]),f))
			{
				return qe_invalid;
			}
		}
	}
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
	{
		if(!p_getc(&(temp_mapscr->sidewarpindex),f))
		{
			return qe_invalid;
		}
	}
	else temp_mapscr->sidewarpindex = 0;
	
	if(!p_igetw(&(temp_mapscr->undercombo),f))
	{
		return qe_invalid;
	}
	
	byte old_combo_page = 0;
	if(Header->zelda_version < 0x193)
	{
		if (!p_getc(&old_combo_page, f))
		{
			return qe_invalid;
		}

		if (!should_skip)
		{
			old_combo_pages[scrind] = old_combo_page;
		}
	}
	
	if(!p_getc(&(temp_mapscr->undercset),f))                //recalculated for older quests
	{
		return qe_invalid;
	}
	
	if(!p_igetw(&(temp_mapscr->catchall),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(temp_mapscr->flags),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(temp_mapscr->flags2),f))
	{
		return qe_invalid;
	}
	
	if(!p_getc(&(temp_mapscr->flags3),f))
	{
		return qe_invalid;
	}
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>1)))
		//if (version>2)
	{
		if(!p_getc(&(temp_mapscr->flags4),f))
		{
			return qe_invalid;
		}
	}
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>7)))
	{
		if(!p_getc(&(temp_mapscr->flags5),f))
		{
			return qe_invalid;
		}
		
		if(!p_igetw(&wpadding,f))
			return qe_invalid;
		temp_mapscr->noreset = wpadding | mLIGHTBEAM | mTMPNORET | mVISITED;
		if(!p_igetw(&wpadding,f))
			return qe_invalid;
		temp_mapscr->nocarry = wpadding | mLIGHTBEAM | mTMPNORET | mVISITED |
			mDOOR_UP | mDOOR_DOWN | mDOOR_LEFT | mDOOR_RIGHT | mNEVERRET | mNO_ENEMIES_RETURN;
		
		if(temp_mapscr->flags5&32)
		{
			temp_mapscr->flags5 &= ~32;
			temp_mapscr->noreset |= 48;
		}
		
		if(version<8)
		{
			if(temp_mapscr->noreset&1)
			{
				temp_mapscr->noreset|=8192;
			}
			
			if(temp_mapscr->nocarry&1)
			{
				temp_mapscr->nocarry|=8192;
				temp_mapscr->nocarry&=~1;
			}
		}
	}
	else
	{
		temp_mapscr->flags5 = 0;
		temp_mapscr->noreset = mLIGHTBEAM | mTMPNORET | mVISITED;
		temp_mapscr->nocarry = mLIGHTBEAM | mTMPNORET | mVISITED |
			mDOOR_UP | mDOOR_DOWN | mDOOR_LEFT | mDOOR_RIGHT | mNEVERRET | mNO_ENEMIES_RETURN;
	}
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>9)))
	{
		if(!p_getc(&(temp_mapscr->flags6),f))
		{
			return qe_invalid;
		}
	}
	
	if(version>5)
	{
		if(!p_getc(&(temp_mapscr->flags7),f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&(temp_mapscr->flags8),f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&(temp_mapscr->flags9),f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&(temp_mapscr->flags10),f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&(temp_mapscr->csensitive),f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_mapscr->csensitive=1;
	}
	
	if(version<14) // August 2007: screen SFX added
	{
		if(temp_mapscr->flags&8) //fROAR
		{
			temp_mapscr->bosssfx=
				(temp_mapscr->flags3&2) ? WAV_DODONGO :  // fDODONGO
				(temp_mapscr->flags2&32) ? WAV_VADER : // fVADER
				WAV_ROAR;
		}
		
		if(temp_mapscr->flags&128) //fSEA_SFX
		{
			temp_mapscr->oceansfx=WAV_SEA;
		}
		
		temp_mapscr->secretsfx = (temp_mapscr->flags3&64) //fNOSECRETSOUND
			? 0 : WAV_SECRET;
		
		temp_mapscr->flags3 &= ~66; //64|2
		temp_mapscr->flags2 &= ~32;
		temp_mapscr->flags &= ~136; // 128|8
	}
	else
	{
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		temp_mapscr->oceansfx = tempbyte;
		
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		temp_mapscr->bosssfx = tempbyte;
		
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		temp_mapscr->secretsfx = tempbyte;
	}
	
	if(version<15) // October 2007: another SFX
	{
		temp_mapscr->holdupsfx=WAV_PICKUP;
	}
	else
	{
		if(!p_getc(&tempbyte,f))
			return qe_invalid;
		temp_mapscr->holdupsfx = tempbyte;
	}
	
	
	if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>97)))
	{
		for(int32_t k=0; k<6; k++)
		{
			if(!p_getc(&(temp_mapscr->layermap[k]),f))
			{
				return qe_invalid;
			}
		}
		
		for(int32_t k=0; k<6; k++)
		{
			if(!p_getc(&(temp_mapscr->layerscreen[k]),f))
			{
				return qe_invalid;
			}
		}
	}
	else if((Header->zelda_version == 0x192)&&(Header->build>23)&&(Header->build<98))
	{
		if(!p_getc(&(temp_mapscr->layermap[2]),f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&(temp_mapscr->layerscreen[2]),f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&(temp_mapscr->layermap[4]),f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&(temp_mapscr->layerscreen[4]),f))
		
		{
			return qe_invalid;
		}
	}
	
	if((Header->zelda_version == 0x192)&&(Header->build>149))
	{
		for(int32_t k=0; k<6; k++)
		{
			if(!p_getc(&tempbyte,f))                          //layerxsize
			{
				return qe_invalid;
			}
		}
		
		for(int32_t k=0; k<6; k++)
		{
			if(!p_getc(&tempbyte,f))                          //layerxspeed
			{
				return qe_invalid;
			}
		}
		
		for(int32_t k=0; k<6; k++)
		{
			if(!p_getc(&tempbyte,f))                          //layerxdelay
			{
				return qe_invalid;
			}
		}
		
		for(int32_t k=0; k<6; k++)
		{
			if(!p_getc(&tempbyte,f))                          //layerysize
			{
				return qe_invalid;
			}
		}
		
		for(int32_t k=0; k<6; k++)
		{
			if(!p_getc(&tempbyte,f))                          //layeryspeed
			{
				return qe_invalid;
			}
		}
		
		for(int32_t k=0; k<6; k++)
		{
			if(!p_getc(&tempbyte,f))                          //layerydelay
			{
				return qe_invalid;
			}
		}
	}
	
	if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>149)))
	{
		for(int32_t k=0; k<6; k++)
		{
			if(!p_getc(&(temp_mapscr->layeropacity[k]),f))
			{
				return qe_invalid;
			}
		}
	}
	
	if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>153)))
	{
		if((Header->zelda_version == 0x192)&&(Header->build>153))
		{
			if(!p_getc(&padding,f))
			{
				return qe_invalid;
			}
		}
		
		if(!p_igetw(&(temp_mapscr->timedwarptics),f))
		{
			return qe_invalid;
		}
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<24)))
	{
		extras=15;
	}
	else if(((Header->zelda_version == 0x192)&&(Header->build<98)))
	{
		extras=11;
	}
	else if((Header->zelda_version == 0x192)&&(Header->build<150))
	{
		extras=32;
	}
	else if((Header->zelda_version == 0x192)&&(Header->build<154))
	{
		extras=64;
	}
	else if(Header->zelda_version < 0x193)
	{
		extras=62;
	}
	else
	
	{
		extras=0;
	}
	
	for(int32_t k=0; k<extras; k++)
	{
		if(!p_getc(&tempbyte,f))                            //extra[k]
		{
			return qe_invalid;
		}
	}
	
	if((Header->zelda_version > 0x211)||((Header->zelda_version == 0x211)&&(Header->build>2)))
		//if (version>3)
	{
		if(!p_getc(&(temp_mapscr->nextmap),f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&(temp_mapscr->nextscr),f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_mapscr->nextmap=0;
		temp_mapscr->nextscr=0;
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<137)))
	{
		secretcombos=20;
	}
	else if((Header->zelda_version == 0x192)&&(Header->build<154))
	{
		secretcombos=256;
	}
	else
	{
		secretcombos=128;
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<154)))
	{
		for(int32_t k=0; k<secretcombos; k++)
		{
			if(!p_getc(&tempbyte,f))
			{
				return qe_invalid;
			}
			
			if(k<128)
			{
				temp_mapscr->secretcombo[k]=tempbyte;
			}
		}
	}
	else
	{
		for(int32_t k=0; k<128; k++)
		{
			if(!p_igetw(&(temp_mapscr->secretcombo[k]),f))
			{
				return qe_invalid;
			}
			
		}
	}
	
	if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>153)))
	{
		for(int32_t k=0; k<128; k++)
		{
			if(!p_getc(&(temp_mapscr->secretcset[k]),f))
			{
				return qe_invalid;
			}
		}
		
		for(int32_t k=0; k<128; k++)
		{
			if(!p_getc(&(temp_mapscr->secretflag[k]),f))
			{
				return qe_invalid;
			}
		}
	}
	
	if((Header->zelda_version == 0x192)&&(Header->build>97)&&(Header->build<154))
	{
		if(!p_getc(&padding,f))
		{
			return qe_invalid;
		}
	}
	
	for(int32_t k=0; k<176; k++)
	{
		if(!p_igetw(&(temp_mapscr->data[k]),f))
		{
			return qe_invalid;
		}
	}
	
	if((Header->zelda_version == 0x192)&&(Header->build>20)&&(Header->build<24))
	{
		if(!p_getc(&padding,f))
		{
			return qe_invalid;
		}
		
		if(!p_getc(&padding,f))
		{
			return qe_invalid;
		}
	}
	
	if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>20)))
	{
		for(int32_t k=0; k<176; k++)
		{
			if(!p_getc(&(temp_mapscr->sflag[k]),f))
			{
				return qe_invalid;
			}
			
			if((Header->zelda_version == 0x192)&&(Header->build<24))
			{
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
				}
				
				if(!p_getc(&tempbyte,f))
				{
					return qe_invalid;
				}
			}
		}
	}
	
	if((Header->zelda_version > 0x192)||((Header->zelda_version == 0x192)&&(Header->build>97)))
	{
		for(int32_t k=0; k<176; k++)
		{
		
			if(!p_getc(&(temp_mapscr->cset[k]),f))
			{
				return qe_invalid;
			}
		}
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<154)))
	{
		temp_mapscr->undercset=(temp_mapscr->undercombo>>8)&7;
		temp_mapscr->undercombo=(temp_mapscr->undercombo&0xFF)+(old_combo_page<<8);
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<137)))
	{
		temp_mapscr->secretcombo[sSBOMB]=temp_mapscr->secretcombo[sBOMB];
		temp_mapscr->secretcombo[sRCANDLE]=temp_mapscr->secretcombo[sBCANDLE];
		temp_mapscr->secretcombo[sWANDFIRE]=temp_mapscr->secretcombo[sBCANDLE];
		temp_mapscr->secretcombo[sDIVINEFIRE]=temp_mapscr->secretcombo[sBCANDLE];
		temp_mapscr->secretcombo[sSARROW]=temp_mapscr->secretcombo[sARROW];
		temp_mapscr->secretcombo[sGARROW]=temp_mapscr->secretcombo[sARROW];
	}
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<154)))
	{
		for(int32_t k=0; k<176; k++)
		{
			if((Header->zelda_version == 0x192)&&(Header->build>149))
			{
				if((Header->zelda_version == 0x192)&&(Header->build!=153))
				{
					temp_mapscr->cset[k]=((temp_mapscr->data[k]>>8)&7);
				}
			}
			else
			{
				if((Header->zelda_version < 0x192)||
						((Header->zelda_version == 0x192)&&(Header->build<21)))
				{
					temp_mapscr->sflag[k]=(temp_mapscr->data[k]>>11);
				}
				
				temp_mapscr->cset[k]=((temp_mapscr->data[k]>>8)&7);
			}
			
			temp_mapscr->data[k]=(temp_mapscr->data[k]&0xFF)+(old_combo_page<<8);
		}
	}
	
	/*if(version>12)
	{
	  if(!p_getc(&(temp_mapscr->scrWidth),f))
	  {
		return qe_invalid;
	  }
	if(!p_getc(&(temp_mapscr->scrHeight),f))
	  {
		return qe_invalid;
	  }
	}*/
	
	if(version>4)
	{
		int16_t m;
		if(!p_igetw(&m, f))
			return qe_invalid;
		if (m <= 0)
			temp_mapscr->music = vbound(m, 0, -1);
		else if (!keep_music) // can't safely convert without modifying quest_music, so just use -1
			temp_mapscr->music = -1;
		else temp_mapscr->music = find_or_make_midi_music(convert_from_old_midi_id(m + (MIDIOFFSET_MAPSCR - MIDIOFFSET_ZSCRIPT),true));
	}
	else
	{
		temp_mapscr->music = -1;
	}
	
	if(version>=17)
	{
		if(!p_getc(&(temp_mapscr->lens_layer),f))
		{
			return qe_invalid;
		}
	}
	else
	{
		temp_mapscr->lens_layer = llNORMAL;
	}
	
	if(version>6)
	{
		dword bits;
		if(!p_igetl(&bits,f))
		{
			return qe_invalid;
		}
		
		int32_t m;
		float tempfloat;
		word tempw;
		temp_mapscr->ffcCountMarkDirty();
		temp_mapscr->ffcs.clear();
		temp_mapscr->resizeFFC(std::bit_width(bits));
		for(m=0; m<32; m++)
		{
			if((bits>>m)&1)
			{
				ffcdata& tempffc = temp_mapscr->ffcs[m];
				if(!p_igetw(&tempw,f))
				{
					return qe_invalid;
				}
				tempffc.data = tempw;
				
				if(!p_getc(&(tempffc.cset),f))
				{
					return qe_invalid;
				}
				
				if(!p_igetw(&(tempffc.delay),f))
				{
					return qe_invalid;
				}
				
				if(version < 9)
				{
					if(!p_igetf_DO_NOT_USE(&tempfloat,f))
					{
						return qe_invalid;
					}
					
					tempffc.x=zslongToFix(int32_t(tempfloat*10000));
					
					if(!p_igetf_DO_NOT_USE(&tempfloat,f))
					{
						return qe_invalid;
					}
					
					tempffc.y=zslongToFix(int32_t(tempfloat*10000));
					
					if(!p_igetf_DO_NOT_USE(&tempfloat,f))
					{
						return qe_invalid;
					}
					
					tempffc.vx=zslongToFix(int32_t(tempfloat*10000));
					
					if(!p_igetf_DO_NOT_USE(&tempfloat,f))
					{
						return qe_invalid;
					}
					
					tempffc.vy=zslongToFix(int32_t(tempfloat*10000));
					
					if(!p_igetf_DO_NOT_USE(&tempfloat,f))
					{
						return qe_invalid;
					}
					
					tempffc.ax=zslongToFix(int32_t(tempfloat*10000));
					
					if(!p_igetf_DO_NOT_USE(&tempfloat,f))
					{
						return qe_invalid;
					}
					
					tempffc.ay=zslongToFix(int32_t(tempfloat*10000));
				}
				else
				{
					if(!p_igetzf(&(tempffc.x),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetzf(&(tempffc.y),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetzf(&(tempffc.vx),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetzf(&(tempffc.vy),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetzf(&(tempffc.ax),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetzf(&(tempffc.ay),f))
					{
						return qe_invalid;
					}
				}
				
				if(!p_getc(&(tempffc.link),f))
				{
					return qe_invalid;
				}
				
				if(version>7)
				{
					if(!p_getc(&tempbyte,f))
					{
						return qe_invalid;
					}
			
					tempffc.hit_width = (tempbyte&0x3F)+1;
					tempffc.txsz = (tempbyte>>6)+1;
					
					if(!p_getc(&tempbyte,f))
					{
						return qe_invalid;
					}
			
					tempffc.hit_height = (tempbyte&0x3F)+1;
					tempffc.tysz = (tempbyte>>6)+1;
					
					if(!p_igetl(&(tempffc.flags),f))
					{
						return qe_invalid;
					}
				}
				else
				{
					tempffc.hit_width=16;
					tempffc.hit_height=16; 
					tempffc.txsz=1;
					tempffc.tysz=1;
					tempffc.flags=ffc_none;
				}
		
				tempffc.updateSolid();
			
				
				if(Header->zelda_version == 0x211 || (Header->zelda_version == 0x250 && Header->build<20))
				{
					tempffc.flags|=ffc_ignoreholdup;
				}
				
				if(version>9)
				{
					if(!p_igetw(&(tempffc.scrconfig.script),f))
					{
						return qe_invalid;
					}
				}
				else
				{
					tempffc.scrconfig.script=0;
				}
				
				if(version>10)
				{
					if(!p_igetl(&(tempffc.scrconfig.run_args[0]),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&(tempffc.scrconfig.run_args[1]),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&(tempffc.scrconfig.run_args[2]),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&(tempffc.scrconfig.run_args[3]),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&(tempffc.scrconfig.run_args[4]),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&(tempffc.scrconfig.run_args[5]),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&(tempffc.scrconfig.run_args[6]),f))
					{
						return qe_invalid;
					}
					
					if(!p_igetl(&(tempffc.scrconfig.run_args[7]),f))
					{
						return qe_invalid;
					}
					
					if(!p_getc(&(tempbyte),f))
					{
						return qe_invalid;
					}
					if(!p_getc(&(tempbyte),f))
					{
						return qe_invalid;
					}
				}
				
				if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
				{
					tempffc.scrconfig.clear();
				}
				if(version <= 11)
				{
					fixffcs=true;
				}
			}
		}

		temp_mapscr->shrinkToFitFFCs();
	}

	
	//add in the new whistle flags
	if(version<13)
	{
		if(temp_mapscr->flags & fWHISTLE)
		{
			temp_mapscr->flags7 |= (fWHISTLEPAL | fWHISTLEWATER);
		}
	}
	
	//2.55 starts here
	if ( version >= 19 && Header->zelda_version > 0x253 )
	{
		// mapscr fields that were never used, so are now removed:
		// int32_t npcstrings[10];
		// int16_t new_items[10];
		// int16_t new_item_x[10];
		// int16_t new_item_y[10];
		if (pack_fseek(f, 100))
		{
			return qe_invalid;
		}
	}

	if ( version >= 20 && Header->zelda_version > 0x253 )
	{
	if(!p_igetw(&(temp_mapscr->scrconfig.script),f))
	{
		return qe_invalid;
	} 
	for ( int32_t q = 0; q < 8; q++)
	{
		if(!p_igetl(&(temp_mapscr->scrconfig.run_args[q]),f))
		{
			return qe_invalid;
		}
	}		
	}
	if ( version < 20 )
	{
		temp_mapscr->scrconfig.clear();
	}
	if ( version >= 21 && Header->zelda_version > 0x253 )
	{
	if(!p_getc(&(temp_mapscr->preloadscript),f))
	{
		return qe_invalid;
	}       
	}
	if ( version < 21 )
	{
	temp_mapscr->preloadscript = 0;    
	}
	
	if ( version >= 22 && Header->zelda_version > 0x253 ) //26th June, 2019; Layer Visibility
	{
	if(!p_getc(&(temp_mapscr->hidelayers ),f))
	{
		return qe_invalid;
	} 
	if(!p_getc(&(temp_mapscr->hidescriptlayers  ),f))
	{
		return qe_invalid;
	}      
	}
	if ( version < 22 )
	{
	temp_mapscr->hidelayers = 0;    
	temp_mapscr->hidescriptlayers = 0;    
	}
	
	//Dodongos in 2.10 used the boss roar, not the dodongo sound. -Z
	//May be any version before 2.11. -Z
	/* --not the roar, the HIT SFX
	if ( Header->zelda_version <= 0x210 ) 
	{
	if ( temp_mapscr->bosssfx == WAV_DODONGO ) 
	{
		temp_mapscr->bosssfx = WAV_ROAR;
	}
	}
	*/
	for(int32_t k=0; k<4; k++)
	{
		if(temp_mapscr->door[k] == dNONE)
			temp_mapscr->door[k] = dWALL;
	}
	
	return 0;
}

} // end namespace


int32_t readmapscreen(PACKFILE *f, zquestheader *Header, mapscr *temp_mapscr, word version, int scrind, bool keep_music)
{
	if(version < 23)
	{
		auto ret = readmapscreen_old(f,Header,temp_mapscr,version,scrind,keep_music);
		if(ret) return ret;
	}
	else
	{
		byte tempbyte;
		if(!p_getc(&(temp_mapscr->valid),f))
			return qe_invalid;
		if(!(temp_mapscr->valid & mVALID))
		{
			int map = scrind/MAPSCRS;
			int screen = scrind%MAPSCRS;
			if(version > 25 && scrind > -1 && map < map_infos.size())
			{
				auto const& mapinf = map_infos[map];
				//Empty screen, apply defaults
				for(int q = 0; q < 6; ++q)
				{
					auto layermap = mapinf.autolayers[q];
					temp_mapscr->layermap[q] = layermap;
					if(layermap)
						temp_mapscr->layerscreen[q] = screen;
				}
				temp_mapscr->color = mapinf.autopalette;
			}
			return 0;
		}
		uint32_t scr_has_flags;
		if(!p_igetl(&scr_has_flags,f))
			return qe_invalid;
		
		if(scr_has_flags & SCRHAS_ROOMDATA)
		{
			if(!p_getc(&(temp_mapscr->guy),f))
				return qe_invalid;
			if(version > 26)
			{
				if(!p_igetl(&(temp_mapscr->guytile),f))
					return qe_invalid;
				if(!p_getc(&(temp_mapscr->guycs),f))
					return qe_invalid;
				if(!p_igetw(&(temp_mapscr->roomflags),f))
					return qe_invalid;
			}
			else
			{
				temp_mapscr->guytile = -1; //signal to use default guy values
				SETFLAG(temp_mapscr->roomflags,RFL_ALWAYS_GUY,temp_mapscr->guy==gFAIRY);
				SETFLAG(temp_mapscr->roomflags,RFL_GUYFIRES,temp_mapscr->guy!=gFAIRY || !get_qr(qr_NOFAIRYGUYFIRES));
			}
			if(!p_igetw(&(temp_mapscr->str),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->room),f))
				return qe_invalid;
			if(!p_igetw(&(temp_mapscr->catchall),f))
				return qe_invalid;
		}
		if(scr_has_flags & SCRHAS_ITEM)
		{
			if (version < 39)
			{
				if(!p_getc(&tempbyte, f))
					return qe_invalid;
				temp_mapscr->item = tempbyte;
			}
			else
			{
				if(!p_igetw(&temp_mapscr->item,f))
					return qe_invalid;
			}
			if(!p_getc(&(temp_mapscr->hasitem),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->itemx),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->itemy),f))
				return qe_invalid;
		}
		if(scr_has_flags & (SCRHAS_SWARP|SCRHAS_TWARP))
		{
			if(!p_igetw(&temp_mapscr->warpreturnc,f))
				return qe_invalid;
		}
		if(scr_has_flags & SCRHAS_TWARP)
		{
			for(int32_t i=0; i<4; i++)
			{
				if(!p_getc(&(temp_mapscr->tilewarptype[i]),f))
					return qe_invalid;
			}
			for(int32_t i=0; i<4; i++)
			{
				if(!p_igetw(&(temp_mapscr->tilewarpdmap[i]),f))
					return qe_invalid;
			}
			for(int32_t i=0; i<4; i++)
			{
				if(!p_getc(&(temp_mapscr->tilewarpscr[i]),f))
					return qe_invalid;
			}
			if(!p_getc(&(temp_mapscr->tilewarpoverlayflags),f))
				return qe_invalid;
		}
		if(scr_has_flags & SCRHAS_SWARP)
		{
			for(int32_t i=0; i<4; i++)
			{
				if(!p_getc(&(temp_mapscr->sidewarptype[i]),f))
					return qe_invalid;
			}
			for(int32_t i=0; i<4; i++)
			{
				if(!p_igetw(&(temp_mapscr->sidewarpdmap[i]),f))
					return qe_invalid;
			}
			for(int32_t i=0; i<4; i++)
			{
				if(!p_getc(&(temp_mapscr->sidewarpscr[i]),f))
					return qe_invalid;
			}
			if(!p_getc(&(temp_mapscr->sidewarpoverlayflags),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->sidewarpindex),f))
				return qe_invalid;
		}
		if(scr_has_flags & SCRHAS_WARPRET)
		{
			for(int32_t i=0; i<4; i++)
			{
				if(!p_getc(&(temp_mapscr->warpreturnx[i]),f))
					return qe_invalid;
			}
			for(int32_t i=0; i<4; i++)
			{
				if(!p_getc(&(temp_mapscr->warpreturny[i]),f))
					return qe_invalid;
			}
			if(!p_getc(&(temp_mapscr->warparrivalx),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->warparrivaly),f))
				return qe_invalid;
		}
		if(scr_has_flags & SCRHAS_LAYERS)
		{
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&(temp_mapscr->layermap[k]),f))
					return qe_invalid;
			}
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&(temp_mapscr->layerscreen[k]),f))
					return qe_invalid;
			}
			for(int32_t k=0; k<6; k++)
			{
				if(!p_getc(&(temp_mapscr->layeropacity[k]),f))
					return qe_invalid;
			}
			if(!p_getc(&(temp_mapscr->hidelayers),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->hidescriptlayers),f))
				return qe_invalid;
		}
		if(scr_has_flags & SCRHAS_MAZE)
		{
			for(int32_t k=0; k<4; k++)
			{
				if(!p_getc(&(temp_mapscr->path[k]),f))
					return qe_invalid;
			}
			if(!p_getc(&(temp_mapscr->exitdir),f))
				return qe_invalid;
			if (version >= 32)
			{
				if (!p_getc(&temp_mapscr->maze_transition_wipe, f))
					return qe_invalid;
			}
		}
		if(scr_has_flags & SCRHAS_D_S_U)
		{
			if(!p_igetw(&(temp_mapscr->door_combo_set),f))
				return qe_invalid;
			for(int32_t k=0; k<4; k++)
			{
				if(!p_getc(&(temp_mapscr->door[k]),f))
					return qe_invalid;
				if(version < 29 && temp_mapscr->door[k] == dNONE)
					temp_mapscr->door[k] = dWALL;
			}
			
			if(!p_getc(&(temp_mapscr->stairx),f))
				return qe_invalid;
			
			if(!p_getc(&(temp_mapscr->stairy),f))
				return qe_invalid;
			if(!p_igetw(&(temp_mapscr->undercombo),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->undercset),f))
				return qe_invalid;
		}
		else if(version < 29)
		{
			for(int k = 0; k < 4; ++k)
				temp_mapscr->door[k] = dWALL;
		}
		if(scr_has_flags & SCRHAS_FLAGS)
		{
			if(!p_getc(&(temp_mapscr->flags),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->flags2),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->flags3),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->flags4),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->flags5),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->flags6),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->flags7),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->flags8),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->flags9),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->flags10),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->flags11),f))
				return qe_invalid;
		}
		if(scr_has_flags & SCRHAS_ENEMY)
		{
			for(int32_t k=0; k<10; k++)
			{
				if(!p_igetw(&(temp_mapscr->enemy[k]),f))
					return qe_invalid;
				if (unsigned(temp_mapscr->enemy[k]) > MAXGUYS)
					temp_mapscr->enemy[k] = 0;
			}
			if(!p_getc(&(temp_mapscr->pattern),f))
				return qe_invalid;
		}
		if(scr_has_flags & SCRHAS_CARRY)
		{
			if(version < 34)
			{
				word tmpw;
				if(!p_igetw(&tmpw,f))
					return qe_invalid;
				temp_mapscr->noreset = tmpw | mLIGHTBEAM | mTMPNORET | mVISITED;
				if(!p_igetw(&tmpw,f))
					return qe_invalid;
				temp_mapscr->nocarry = tmpw | mLIGHTBEAM | mTMPNORET | mVISITED |
					mDOOR_UP | mDOOR_DOWN | mDOOR_LEFT | mDOOR_RIGHT | mNEVERRET | mNO_ENEMIES_RETURN;
			}
			else
			{
				if(!p_igetl(&(temp_mapscr->noreset),f))
					return qe_invalid;
				if(!p_igetl(&(temp_mapscr->nocarry),f))
					return qe_invalid;
				if(!p_igetl(&(temp_mapscr->exstate_reset),f))
					return qe_invalid;
				if(!p_igetl(&(temp_mapscr->exstate_carry),f))
					return qe_invalid;
			}
			if(!p_getc(&(temp_mapscr->nextmap),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->nextscr),f))
				return qe_invalid;
		}
		else
		{
			if(version < 34)
			{
				temp_mapscr->noreset = mLIGHTBEAM | mTMPNORET | mVISITED;
				temp_mapscr->nocarry = mLIGHTBEAM | mTMPNORET | mVISITED |
					mDOOR_UP | mDOOR_DOWN | mDOOR_LEFT | mDOOR_RIGHT | mNEVERRET | mNO_ENEMIES_RETURN;
			}
		}
		if(scr_has_flags & SCRHAS_SCRIPT)
		{
			if (version < 41)
			{
				if (!p_igetw(&(temp_mapscr->scrconfig.script),f))
					return qe_invalid;
				if (!p_getc(&(temp_mapscr->preloadscript),f))
					return qe_invalid;
				for (int32_t q = 0; q < 8; q++)
				{
					if(!p_igetl(&(temp_mapscr->scrconfig.run_args[q]),f))
						return qe_invalid;
				}
			}
			else
			{
				if (!p_getvar(&(temp_mapscr->scrconfig),f))
					return qe_invalid;
				if (!p_getc(&(temp_mapscr->preloadscript),f))
					return qe_invalid;
			}
		}
		if(scr_has_flags & SCRHAS_SECRETS)
		{
			for(int32_t k=0; k<128; k++)
			{
				if(!p_igetw(&(temp_mapscr->secretcombo[k]),f))
					return qe_invalid;
			}
			for(int32_t k=0; k<128; k++)
			{
				if(!p_getc(&(temp_mapscr->secretcset[k]),f))
					return qe_invalid;
			}
			for(int32_t k=0; k<128; k++)
			{
				if(!p_getc(&(temp_mapscr->secretflag[k]),f))
					return qe_invalid;
			}
		}
		if(scr_has_flags & SCRHAS_COMBOFLAG)
		{
			for(int32_t k=0; k<176; ++k)
			{
				if(!p_igetw(&(temp_mapscr->data[k]),f))
					return qe_invalid;
			}
			for(int32_t k=0; k<176; ++k)
			{
				if(!p_getc(&(temp_mapscr->sflag[k]),f))
					return qe_invalid;
			}
			for(int32_t k=0; k<176; ++k)
			{
				if(!p_getc(&(temp_mapscr->cset[k]),f))
					return qe_invalid;
			}
		}
		if(scr_has_flags & SCRHAS_MISC)
		{
			if(!p_igetw(&(temp_mapscr->color),f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->csensitive),f))
				return qe_invalid;
			if (version < 38)
			{
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				temp_mapscr->oceansfx = tempbyte;
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				temp_mapscr->bosssfx = tempbyte;
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				temp_mapscr->secretsfx = tempbyte;
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				temp_mapscr->holdupsfx = tempbyte;
			}
			else
			{
				if(!p_igetw(&(temp_mapscr->oceansfx),f))
					return qe_invalid;
				if(!p_igetw(&(temp_mapscr->bosssfx),f))
					return qe_invalid;
				if(!p_igetw(&(temp_mapscr->secretsfx),f))
					return qe_invalid;
				if(!p_igetw(&(temp_mapscr->holdupsfx),f))
					return qe_invalid;
			}
			if(!p_igetw(&(temp_mapscr->timedwarptics),f))
				return qe_invalid;
			if (version < 37)
			{
				int16_t m;
				if(!p_igetw(&m, f))
					return qe_invalid;
				if (m <= 0)
					temp_mapscr->music = vbound(m, 0, -1);
				else if (!keep_music) // can't safely convert without modifying quest_music, so just use -1
					temp_mapscr->music = -1;
				else temp_mapscr->music = find_or_make_midi_music(convert_from_old_midi_id(m + (MIDIOFFSET_MAPSCR - MIDIOFFSET_ZSCRIPT), true));
			}
			else if (!p_igetl(&(temp_mapscr->music), f))
				return qe_invalid;
			if(!p_getc(&(temp_mapscr->lens_layer),f))
				return qe_invalid;
			if(version > 27)
			{
				if(!p_getc(&(temp_mapscr->lens_show),f))
					return qe_invalid;
				if(!p_getc(&(temp_mapscr->lens_hide),f))
					return qe_invalid;
			}
		}
		else
		{
			temp_mapscr->music = -1;
			temp_mapscr->csensitive = 1;
		}
		//FFC
		bool old_ff = version < 25;
		dword bits = 0;
		word numffc = 32;
		if(old_ff)
		{
			if(!p_igetl(&bits,f))
				return qe_invalid;
		}
		else
		{
			if(!p_igetw(&numffc,f) || numffc > MAXFFCS)
				return qe_invalid;
		}

		temp_mapscr->ffcCountMarkDirty();
		temp_mapscr->ffcs.clear();
		temp_mapscr->resizeFFC(numffc);

		word tempw;
		static ffcdata nil_ffc;
		for(word m = 0; m < numffc; ++m)
		{
			if(old_ff && !(bits & (1<<m))) continue;

			ffcdata& tempffc = (m < MAXFFCS)
				? temp_mapscr->ffcs[m]
				: nil_ffc; //sanity

			if(!p_igetw(&tempw,f))
				return qe_invalid;
			if(!old_ff && !tempw) //empty ffc, nothing more to load
				continue;
			tempffc.data = tempw;
			
			if(!p_getc(&(tempffc.cset),f))
				return qe_invalid;
			if(!p_igetw(&(tempffc.delay),f))
				return qe_invalid;
			if(!p_igetzf(&(tempffc.x),f))
				return qe_invalid;
			if(!p_igetzf(&(tempffc.y),f))
				return qe_invalid;
			if(!p_igetzf(&(tempffc.vx),f))
				return qe_invalid;
			if(!p_igetzf(&(tempffc.vy),f))
				return qe_invalid;
			if(!p_igetzf(&(tempffc.ax),f))
				return qe_invalid;
			if(!p_igetzf(&(tempffc.ay),f))
				return qe_invalid;
			if(!p_getc(&(tempffc.link),f))
				return qe_invalid;
			if(version < 24)
			{
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				tempffc.hit_width = (tempbyte&0x3F)+1;
				tempffc.txsz = (tempbyte>>6)+1;
				if(!p_getc(&tempbyte,f))
					return qe_invalid;
				tempffc.hit_height = (tempbyte&0x3F)+1;
				tempffc.tysz = (tempbyte>>6)+1;
			}
			else
			{
				if(!p_igetl(&(tempffc.hit_width),f))
					return qe_invalid;
				if(!p_igetl(&(tempffc.hit_height),f))
					return qe_invalid;
				if(!p_getc(&(tempffc.txsz),f))
					return qe_invalid;
				if(!p_getc(&(tempffc.tysz),f))
					return qe_invalid;
			}
			if(!p_igetl(&(tempffc.flags),f))
				return qe_invalid;
			tempffc.updateSolid();
			if (version < 41)
			{
				if(!p_igetw(&(tempffc.scrconfig.script),f))
					return qe_invalid;
				for(auto q = 0; q < 8; ++q)
				{
					if(!p_igetl(&(tempffc.scrconfig.run_args[q]),f))
						return qe_invalid;
				}
			}
			else if (!p_getvar(&tempffc.scrconfig, f))
				return qe_invalid;
			if(version < 33)
			{
				if(!p_getc(&(tempbyte),f))
					return qe_invalid;
				if(!p_getc(&(tempbyte),f))
					return qe_invalid;
			}
			else if(!p_getc(&(tempffc.layer),f))
				return qe_invalid;

			if(version >= 40)
			{
				if(!p_igetl(&(tempffc.viewport_suspend_range),f))
					return qe_invalid;
				if(!p_igetl(&(tempffc.viewport_despawn_range),f))
					return qe_invalid;
			}

			if(loading_tileset_flags & TILESET_CLEARSCRIPTS)
			{
				tempffc.scrconfig.clear();
			}
		}
		//END FFC
		if(version > 29)
			if(!p_getlstr(&temp_mapscr->usr_notes, f))
				return qe_invalid;
		
		if (version >= 35 && (temp_mapscr->flags10 & fSCREEN_GRAVITY))
		{
			if (!p_igetzf(&temp_mapscr->screen_gravity, f))
				return qe_invalid;
			if (!p_igetzf(&temp_mapscr->screen_terminal_v, f))
				return qe_invalid;
		}
	}

	temp_mapscr->shrinkToFitFFCs();

	return 0;
}

int32_t readmaps(PACKFILE *f, zquestheader *Header)
{
	bool should_skip = legacy_skip_flags && get_bit(legacy_skip_flags, skip_maps);
	int32_t screen=0;
	
	word version=0;
	dword dummy;
	int32_t screens_to_read;
	
	mapscr temp_mapscr{};
	word temp_map_count;
	dword section_size;
	
	if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<137)))
	{
		screens_to_read=MAPSCRS192b136;
	}
	else
	{
		screens_to_read=MAPSCRS;
	}
	
	if(Header->zelda_version > 0x192)
	{
		//section version info
		if(!p_igetw(&version,f))
		{
			return qe_invalid;
		}

		if (version > V_MAPS)
			return qe_version;
	
		FFCore.quest_format[vMaps] = version;
		
		if(!p_igetw(&dummy,f))
		{
			return qe_invalid;
		}
		
		//section size
		if(!p_igetl(&section_size,f))
		{
			return qe_invalid;
		}
		
		//finally...  section data
		if(!p_igetw(&temp_map_count,f))
		{
			return 5;
		}
	}
	else
	{
		temp_map_count=map_count;
	}

	if (temp_map_count > MAXMAPS)
	{
		return qe_invalid;
	}

	if (!should_skip)
	{
		const int32_t _mapsSize = MAPSCRS*temp_map_count;
		TheMaps.clear();
		TheMaps.resize(_mapsSize);
		old_combo_pages.clear();
		old_combo_pages.resize(_mapsSize);
		map_infos.clear();
		map_infos.resize(temp_map_count);
		if(version >= 31)
			Regions = {};
	}
	
	for(int32_t i=0; i<temp_map_count && i<MAXMAPS; i++)
	{
		byte valid=1;
		if(version > 22)
		{
			if(!p_getc(&valid,f))
				return qe_invalid;
		}
		if(valid)
		{
			if (version > 25)
			{
				auto& mapinf = map_infos[i];
				for(int q = 0; q < 6; ++q)
				{
					if(!p_igetw(&mapinf.autolayers[q],f))
						return qe_invalid;
				}
				if (version >= 36)
					if(!p_igetw(&mapinf.autopalette,f))
						return qe_invalid;
			}

			if (version >= 31)
			{
				static regions_data tmp_rd;
				regions_data& rd = should_skip ? tmp_rd : Regions[i];
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
		}
		for(int32_t j=0; j<screens_to_read; j++)
		{
			screen=i*MAPSCRS+j;
			mapscr* scr = should_skip ? &temp_mapscr : &TheMaps[screen];
			scr->map = i;
			scr->screen = j;
			if(valid)
				readmapscreen(f, Header, scr, version, screen, !should_skip);
			else if (!should_skip)
				scr->zero_memory();
		}

		if (should_skip)
			continue;
		
		if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<137)))
		{
			int32_t index = (i*MAPSCRS+132);
			
			TheMaps[index]=TheMaps[index-1];
			
			MEMCPY_ARR(TheMaps[i*MAPSCRS+132].data, TheMaps[i*MAPSCRS+131].data);
			MEMCPY_ARR(TheMaps[i*MAPSCRS+132].sflag, TheMaps[i*MAPSCRS+131].sflag);
			MEMCPY_ARR(TheMaps[i*MAPSCRS+132].cset, TheMaps[i*MAPSCRS+131].cset);
			
			for(int32_t j=133; j<MAPSCRS; j++)
			{
				screen=i*MAPSCRS+j;
				
				TheMaps[screen].zero_memory();
				TheMaps[screen].valid = mVERSION;
				TheMaps[screen].music = -1;
				TheMaps[screen].csensitive = 1;
			}
		}
		
		if((Header->zelda_version < 0x192)||((Header->zelda_version == 0x192)&&(Header->build<154)))
		{
			for(int32_t j=0; j<MAPSCRS; j++)
			{
				screen=i*MAPSCRS+j;
				TheMaps[screen].door_combo_set=MakeDoors(i, j);
				
				for(int32_t k=0; k<128; k++)
				{
					// secretcombo is a file-controlled position into the TEMPLATE2
					// screen's 176-entry cset/sflag/data arrays; bound it to avoid OOB reads.
					word pos = TheMaps[screen].secretcombo[k];
					if(pos >= 176)
						pos = 0;
					TheMaps[screen].secretcset[k]=tcmbcset2(i, pos);
					TheMaps[screen].secretflag[k]=tcmbflag2(i, pos);
					TheMaps[screen].secretcombo[k]=tcmbdat2(i, j, pos);
				}
			}
		}
	}
	map_count = temp_map_count;
	return 0;
}

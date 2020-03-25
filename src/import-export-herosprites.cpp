int readherosprites(PACKFILE *f)
{
	dword section_version=0;
	dword section_cversion=0;
	int zversion = 0;
	int zbuild = 0;
	int spritetype = 0;
	int ret = 1;
	
	if(!p_igetl(&zversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&zbuild,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_version,f,true))
	{
		return 0;
	}
	if(!p_igetw(&section_cversion,f,true))
	{
		return 0;
	}
	if(!p_igetl(&spritetype,f,true))
	{
		return 0;
	}
	
	
	if(section_version>=0)
	{
		word tile, tile2;
		byte flip, extend, dummy_byte;
		int keepdata = 1;
        
		for(int i=0; i<4; i++)
		{
			if(!p_igetw(&tile,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&flip,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&extend,f,keepdata))
			{
				return 0;
			}
            
			if(keepdata)
			{
				walkspr[i][spr_tile]=(int)tile;
				walkspr[i][spr_flip]=(int)flip;
				walkspr[i][spr_extend]=(int)extend;
			}
		}
        
		for(int i=0; i<4; i++)
		{
			if(!p_igetw(&tile,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&flip,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&extend,f,keepdata))
			{
				return 0;
			}
            
			if(keepdata)
			{
				stabspr[i][spr_tile]=(int)tile;
				stabspr[i][spr_flip]=(int)flip;
				stabspr[i][spr_extend]=(int)extend;
			}
		}
        
		for(int i=0; i<4; i++)
		{
			if(!p_igetw(&tile,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&flip,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&extend,f,keepdata))
			{
				return 0;
			}
            
			if(keepdata)
			{
				slashspr[i][spr_tile]=(int)tile;
				slashspr[i][spr_flip]=(int)flip;
				slashspr[i][spr_extend]=(int)extend;
			}
		}
        
		for(int i=0; i<4; i++)
		{
			if(!p_igetw(&tile,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&flip,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&extend,f,keepdata))
			{
				return 0;
			}
            
			if(keepdata)
			{
				floatspr[i][spr_tile]=(int)tile;
				floatspr[i][spr_flip]=(int)flip;
				floatspr[i][spr_extend]=(int)extend;
			}
		}
        
		if(section_version>1)
		{
			for(int i=0; i<4; i++)
			{
				if(!p_igetw(&tile,f,keepdata))
				{
					return 0;
				}
                
				if(!p_getc(&flip,f,keepdata))
				{
					return 0;
				}
                
				if(!p_getc(&extend,f,keepdata))
				{
					return 0;
				}
                
				if(keepdata)
				{
					swimspr[i][spr_tile]=(int)tile;
					swimspr[i][spr_flip]=(int)flip;
					swimspr[i][spr_extend]=(int)extend;
				}
			}
		}
        
		for(int i=0; i<4; i++)
		{
			if(!p_igetw(&tile,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&flip,f,keepdata))
			{
				return 0;
			}
		    
			if(!p_getc(&extend,f,keepdata))
			{
				return 0;
			}
		    
			if(keepdata)
			{
				divespr[i][spr_tile]=(int)tile;
				divespr[i][spr_flip]=(int)flip;
				divespr[i][spr_extend]=(int)extend;
			}
		}
        
		for(int i=0; i<4; i++)
		{
			if(!p_igetw(&tile,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&flip,f,keepdata))
			{
				return 0;
			}
            
			if(!p_getc(&extend,f,keepdata))
			{
				return 0;
			}
            
			if(keepdata)
			{
				poundspr[i][spr_tile]=(int)tile;
				poundspr[i][spr_flip]=(int)flip;
				poundspr[i][spr_extend]=(int)extend;
			}
		}
        
		if(!p_igetw(&tile,f,keepdata))
		{
			return 0;
		}
        
		flip=0;
        
		if(section_version>0)
		{
			if(!p_getc(&flip,f,keepdata))
			{
				return 0;
			}
		}
        
		if(!p_getc(&extend,f,keepdata))
		{
			return 0;
		}
        
		if(keepdata)
		{
			castingspr[spr_tile]=(int)tile;
			castingspr[spr_flip]=(int)flip;
			castingspr[spr_extend]=(int)extend;
		}
        
		if(section_version>0)
		{
			for(int i=0; i<2; i++)
			{
				for(int j=0; j<2; j++)
				{
					if(!p_igetw(&tile,f,keepdata))
					{
						return 0;
					}
                    
					if(!p_getc(&flip,f,keepdata))
					{
						return 0;
					}
                    
					if(!p_getc(&extend,f,keepdata))
					{
						return 0;
					}
                    
					if(keepdata)
					{
						holdspr[i][j][spr_tile]=(int)tile;
						holdspr[i][j][spr_flip]=(int)flip;
						holdspr[i][j][spr_extend]=(int)extend;
					}
				}
			}
		}
		else
		{
			for(int i=0; i<2; i++)
			{
				if(!p_igetw(&tile,f,keepdata))
				{
					return 0;
				}
                
				if(!p_igetw(&tile2,f,keepdata))
				{
					return 0;
				}
                
				if(!p_getc(&extend,f,keepdata))
				{
					return 0;
				}
                
				if(keepdata)
				{
					holdspr[i][spr_hold1][spr_tile]=(int)tile;
					holdspr[i][spr_hold1][spr_flip]=(int)flip;
					holdspr[i][spr_hold1][spr_extend]=(int)extend;
					holdspr[i][spr_hold2][spr_tile]=(int)tile2;
					holdspr[i][spr_hold1][spr_flip]=(int)flip;
					holdspr[i][spr_hold2][spr_extend]=(int)extend;
				}
			}
		}
        
		if(section_version>2)
		{
			for(int i=0; i<4; i++)
			{
				if(!p_igetw(&tile,f,keepdata))
				{
					return 0;
				}
                
				if(!p_getc(&flip,f,keepdata))
				{
					return 0;
				}
                
				if(!p_getc(&extend,f,keepdata))
				{
					return 0;
				}
                
				if(keepdata)
				{
					jumpspr[i][spr_tile]=(int)tile;
					jumpspr[i][spr_flip]=(int)flip;
					jumpspr[i][spr_extend]=(int)extend;
				}
			}
		}
        
		if(section_version>3)
		{
			for(int i=0; i<4; i++)
			{
				if(!p_igetw(&tile,f,keepdata))
				{
					return 0;
				}
                
				if(!p_getc(&flip,f,keepdata))
				{
					return 0;
				}
                
				if(!p_getc(&extend,f,keepdata))
				{
					return 0;
				}
                
				if(keepdata)
				{
					chargespr[i][spr_tile]=(int)tile;
					chargespr[i][spr_flip]=(int)flip;
					chargespr[i][spr_extend]=(int)extend;
				}
			}
		}
        
		if(section_version>4)
		{
			if(!p_getc(&dummy_byte,f,keepdata))
			{
				return 0;
			}
            
			if(keepdata)
			{
				zinit.link_swim_speed=(byte)dummy_byte;
			}
		}
	}
	return ret;
}


int writezdoorsets(PACKFILE *f, int first = 0, int count = door_combo_set_count)
{
	dword section_version=V_DOORS;
	dword section_cversion=CV_DOORS;
	int zversion = ZELDA_VERSION;
	int zbuild = VERSION_BUILD;
	int doorscount = door_combo_set_count;
	int firstset = first;
	int lastset = count;
	
	if(!p_iputl(zversion,f))
	{
		return 0;
	}
	if(!p_iputl(zbuild,f))
	{
		return 0;
	}
	if(!p_iputw(section_version,f))
	{
		return 0;
	}
	if(!p_iputw(section_cversion,f))
	{
		return 0;
	}
	if(!p_iputl(doorscount,f))
	{
		return 0;
	}
	if(!p_iputl(firstset,f))
	{
		return 0;
	}
	if(!p_iputl(lastset,f))
	{
		return 0;
	}
	//if the params passed would be invalid:
	if ( firstset < 0 || firstset >= doorscount )
	{
		firstset = 0;
	}
	if ( lastset < 1 || lastset >= doorscount )
	{
		lastset = doorscount;
	}
	else
	{
		lastset = firstset + count;
	}
	//end params sanity guard
	
	//doorset data
	for(int i=firstset; i<lastset; ++i)
        {
		al_trace("Door writecycle %d\n", i);
		//name
		if(!pfwrite(&DoorComboSets[i].name,sizeof(DoorComboSets[0].name),f))
		{
			return 0;
		}
		//up door
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<4; k++)
			{
				if(!p_iputw(DoorComboSets[i].doorcombo_u[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<4; k++)
			{
				if(!p_putc(DoorComboSets[i].doorcset_u[j][k],f))
				{
					return 0;
				}
			}
		}
		//down door
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<4; k++)
			{
				if(!p_iputw(DoorComboSets[i].doorcombo_d[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<4; k++)
			{
				if(!p_putc(DoorComboSets[i].doorcset_d[j][k],f))
				{
					return 0;
				}
			}
		}
		//left door
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<6; k++)
			{
				if(!p_iputw(DoorComboSets[i].doorcombo_l[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<6; k++)
			{
				if(!p_putc(DoorComboSets[i].doorcset_l[j][k],f))
				{
					return 0;
				}
			}
		}
		//right door
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<6; k++)
			{
				if(!p_iputw(DoorComboSets[i].doorcombo_r[j][k],f))
				{
					return 0;
				}
			}
		}
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<6; k++)
			{
				if(!p_putc(DoorComboSets[i].doorcset_r[j][k],f))
				{
					return 0;
				}
			}
		}
		//up bomb rubble
		for(int j=0; j<2; j++)
		{
			if(!p_iputw(DoorComboSets[i].bombdoorcombo_u[j],f))
			{
				return 0;
			}
		}
		for(int j=0; j<2; j++)
		{
			if(!p_putc(DoorComboSets[i].bombdoorcset_u[j],f))
			{
				return 0;
			}
		}
		//down bomb rubble
		for(int j=0; j<2; j++)
		{
			if(!p_iputw(DoorComboSets[i].bombdoorcombo_d[j],f))
			{
				return 0;
			}
		}
		for(int j=0; j<2; j++)
		{
			if(!p_putc(DoorComboSets[i].bombdoorcset_d[j],f))
			{
				return 0;
			}
		}
		//left bomb rubble
		for(int j=0; j<3; j++)
		{
			if(!p_iputw(DoorComboSets[i].bombdoorcombo_l[j],f))
			{
				return 0;
			}
		}
		for(int j=0; j<3; j++)
		{
			if(!p_putc(DoorComboSets[i].bombdoorcset_l[j],f))
			{
				return 0;
			}
		}
		//right bomb rubble
		for(int j=0; j<3; j++)
		{
			if(!p_iputw(DoorComboSets[i].bombdoorcombo_r[j],f))
			{
				return 0;
			}
		}
		for(int j=0; j<3; j++)
		{
			if(!p_putc(DoorComboSets[i].bombdoorcset_r[j],f))
			{
				return 0;
			}
		}
		//walkthrough stuff
		for(int j=0; j<4; j++)
		{
			if(!p_iputw(DoorComboSets[i].walkthroughcombo[j],f))
			{
				return 0;
			}
		}
		for(int j=0; j<4; j++)
		{
			if(!p_putc(DoorComboSets[i].walkthroughcset[j],f))
			{
				return 0;
			}
		}
		//flags
		for(int j=0; j<2; j++)
		{
			if(!p_putc(DoorComboSets[i].flags[j],f))
			{
				return 0;
			}
		}
        }
	return 1;
}

///

int saveherosprites(PACKFILE *f, int type = 0)
{

	dword section_id=ID_LINKSPRITES;
	dword section_version=V_LINKSPRITES;
	dword section_cversion=CV_LINKSPRITES;
	dword section_size=0;
	int spritetype = type;
    
	//section version info
	if(!p_iputw(V_ZELDA,f))
	{
		return 0;
	}
	if(!p_iputw(V_BUILD,f))
	{
		return 0;
	}
    
	//section version info
	if(!p_iputw(section_version,f))
	{
		return 0;
	}
    
	if(!p_iputw(section_cversion,f))
	{
		return 0;
	}
	
	if(!p_iputl(spritetype,f))
	{
		return 0;
	}
    
        //finally...  section data
        for(int i=0; i<4; i++)
        {
		if(!p_iputw((word)walkspr[i][spr_tile],f))
		{
			return 0;
		}
            
		if(!p_putc((byte)walkspr[i][spr_flip],f))
		{
			return 0;
		}
            
		if(!p_putc((byte)walkspr[i][spr_extend],f))
		{
			return 0;
		}
	}
        
        for(int i=0; i<4; i++)
        {
		if(!p_iputw((word)stabspr[i][spr_tile],f))
		{
			return 0;
		}
            
		if(!p_putc((byte)stabspr[i][spr_flip],f))
		{
			return 0;
		}
            
		if(!p_putc((byte)stabspr[i][spr_extend],f))
		{
			return 0;
		}
	}
        
	for(int i=0; i<4; i++)
	{
		if(!p_iputw((word)slashspr[i][spr_tile],f))
		{
			return 0;
		}
    
	if(!p_putc((byte)slashspr[i][spr_flip],f))
	{
		return 0;
	}
    
	if(!p_putc((byte)slashspr[i][spr_extend],f))
	{
		return 0;
	}
}

	for(int i=0; i<4; i++)
	{
		if(!p_iputw((word)floatspr[i][spr_tile],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)floatspr[i][spr_flip],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)floatspr[i][spr_extend],f))
		{
			return 0;
		}
	}

	for(int i=0; i<4; i++)
	{
		if(!p_iputw((word)swimspr[i][spr_tile],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)swimspr[i][spr_flip],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)swimspr[i][spr_extend],f))
		{
			return 0;
		}
	}

	for(int i=0; i<4; i++)
	{
		if(!p_iputw((word)divespr[i][spr_tile],f))
		{
			return 0;
		}
		
		if(!p_putc((byte)divespr[i][spr_flip],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)divespr[i][spr_extend],f))
		{
			return 0;
		}
	}

	for(int i=0; i<4; i++)
	{
		if(!p_iputw((word)poundspr[i][spr_tile],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)poundspr[i][spr_flip],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)poundspr[i][spr_extend],f))
		{
			return 0;
		}
	}

	if(!p_iputw((word)castingspr[spr_tile],f))
	{
		return 0;
	}

	if(!p_putc((byte)castingspr[spr_flip],f))
	{
		return 0;
	}

	if(!p_putc((byte)castingspr[spr_extend],f))
	{
		return 0;
	}

	for(int i=0; i<2; i++)
	{
		for(int j=0; j<2; j++)
		{
			if(!p_iputw((word)holdspr[i][j][spr_tile],f))
			{
				return 0;
			}
	
			if(!p_putc((byte)holdspr[i][j][spr_flip],f))
			{
				return 0;
			}
	
			if(!p_putc((byte)holdspr[i][j][spr_extend],f))
			{
				return 0;
			}
		}
	}

	for(int i=0; i<4; i++)
	{
		if(!p_iputw((word)jumpspr[i][spr_tile],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)jumpspr[i][spr_flip],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)jumpspr[i][spr_extend],f))
		{
			return 0;
		}
	}

	for(int i=0; i<4; i++)
	{
		if(!p_iputw((word)chargespr[i][spr_tile],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)chargespr[i][spr_flip],f))
		{
			return 0;
		}
    
		if(!p_putc((byte)chargespr[i][spr_extend],f))
		{
			return 0;
		}
	}

	if(!p_putc((byte)zinit.link_swim_speed,f))
	{
		return 0;
	}
        
	return 1;
}


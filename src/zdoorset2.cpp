int writezdoorsets(PACKFILE *f, int first, int count)
{
	dword section_version=0;
	dword section_cversion=0;
	int zversion = 0;
	int zbuild = 0;
	int doorscount = 0;
	DoorComboSet tempDoorComboSet;
	memset(tempDoorComboSet, 0, sizeof(DoorComboSet));
	int lastset = 0;
	int firstset = 0;
	int last = 0;
	
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
	if(!p_igetl(&doorscount,f,true))
	{
		return 0;
	}
	if(!p_igetl(&firstset,f,true))
	{
		return 0;
	}
	if(!p_igetl(&last,f,true))
	{
		return 0;
	}
	//if the params passed would be invalid:
	if ( first < firstset || first >= door_combo_set_count ) 
	{
		first = firstset;
	}
	if ( lastset < 1 || lastset > count )
	{
		lastset = doorscount;
	}
	else
	{
		lastset = firstset + count;
	}
	//end params sanity guard
	if ( zversion > ZELDA_VERSION )
	{
		al_trace("Cannot read .zdoorset packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_DOORS ) || ( section_version == V_DOORS && section_cversion > CV_DOORS ) )
	{
		al_trace("Cannot read .zdoorset packfile made using V_DOORS (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zdoorset packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
	}
	
	//section data for doors
	for(int i=firstset; i<lastset; ++i)
	{
		//Clear per set
		memset(tempDoorComboSet, 0, sizeof(DoorComboSet));
		//name
		if(!pfread(&tempDoorComboSet.name,sizeof(tempDoorComboSet.name),f,true))
		{
			return 0;
		}
		//up door
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<4; k++)
			{
				if(!p_igetw(&tempDoorComboSet.doorcombo_u[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<4; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_u[j][k],f,true))
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
				if(!p_igetw(&tempDoorComboSet.doorcombo_d[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<4; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_d[j][k],f,true))
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
				if(!p_igetw(&tempDoorComboSet.doorcombo_l[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<6; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_l[j][k],f,true))
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
				if(!p_igetw(&tempDoorComboSet.doorcombo_r[j][k],f,true))
				{
					return 0;
				}
			}
		}
		for(int j=0; j<9; j++)
		{
			for(int k=0; k<6; k++)
			{
				if(!p_getc(&tempDoorComboSet.doorcset_r[j][k],f,true))
				{
					return 0;
				}
			}
		}
		//up bomb rubble
		for(int j=0; j<2; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_u[j],f,true))
			{
				return 0;
			}
		}
		for(int j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_u[j],f,true))
			{
				return 0;
			}
		}
		//down bomb rubble
		for(int j=0; j<2; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_d[j],f,true))
			{
				return 0;
			}
		}
		for(int j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_d[j],f,true))
			{
				return 0;
			}
		}
		//left bomb rubble
		for(int j=0; j<3; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_l[j],f,true))
			{
				return 0;
			}
		}
		for(int j=0; j<3; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_l[j],f,true))
			{
				return 0;
			}
		}
		//right bomb rubble
		for(int j=0; j<3; j++)
		{
			if(!p_igetw(&tempDoorComboSet.bombdoorcombo_r[j],f,true))
			{
				return 0;
			}
		}
		for(int j=0; j<3; j++)
		{
			if(!p_getc(&tempDoorComboSet.bombdoorcset_r[j],f,true))
			{
				return 0;
			}
		}
		//walkthrough stuff
		for(int j=0; j<4; j++)
		{
			if(!p_igetw(&tempDoorComboSet.walkthroughcombo[j],f,true))
			{
				return 0;
			}
		}
		for(int j=0; j<4; j++)
		{
			if(!p_getc(&tempDoorComboSet.walkthroughcset[j],f,true))
			{
				return 0;
			}
		}
		//flags
		for(int j=0; j<2; j++)
		{
			if(!p_getc(&tempDoorComboSet.flags[j],f,true))
			{
				return 0;
			}
		}
		memcpy(&DoorComboSets[i], &tempDoorComboSet, sizeof(tempDoorComboSet));
	}
	return 1;	
}


int writezdoorsets(PACKFILE *f, int index, int first = 0, int count)
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


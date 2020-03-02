//zq_files

int readzdoorsets(PACKFILE *f, int first, int count)
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
		al_trace("Cannot read .zdoors packfile made in ZC version (%x) in this version of ZC (%x)\n", zversion, ZELDA_VERSION);
		return 0;
	}
	
	else if ( ( section_version > V_DOORS ) || ( section_version == V_DOORS && section_cversion > CV_DOORS ) )
	{
		al_trace("Cannot read .zdoors packfile made using V_DOORS (%d) subversion (%d)\n", section_version, section_cversion);
		return 0;
		
	}
	else
	{
		al_trace("Reading a .zdoors packfile made in ZC Version: %x, Build: %d\n", zversion, zbuild);
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


//zquest.cpp

static DIALOG save_doorset_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,      0,   0,   120,  100,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Save Doorset", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    //for future tabs
    { d_dummy_proc,         120,  128,  80+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         120,  128,  80+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },
    //4
    {  jwin_text_proc,        10,    28,     20,      8,    vc(11),     vc(1),      0,    0,          0,    0, (void *) "First",               NULL,   NULL  },
    { jwin_edit_proc,          55,     26,    40,     16,    vc(12),                 vc(1),                   0,       0,          63,    0,  NULL,                                           NULL,   NULL                  },
    //6
    {  jwin_text_proc,        10,    46,     20,      8,    vc(11),     vc(1),      0,    0,          0,    0, (void *) "Count",               NULL,   NULL  },
    { jwin_edit_proc,          55,     44,    40,     16,    vc(12),                 vc(1),                   0,       0,          63,    0,  NULL,                                           NULL,   NULL                  },
    //8
    { jwin_button_proc,   15,   72,  36,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "Save", NULL, NULL },
    { jwin_button_proc,   69,  72,  36,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};


void do_exportoorset(const char *prompt,int initialval)
{
	char firstdoor[8], doorct[8];
	int first_doorset_id = 0; int the_doorset_count = 1;
	sprintf(firstdoor,"%d",0);
	sprintf(doorct,"%d",1);
	//int ret;
	save_doorset_dlg[0].dp2 = lfont;
	
	sprintf(firstdoor,"%d",0);
	sprintf(doorct,"%d",1);
	
	save_doorset_dlg[5].dp = firstdoor;
	save_doorset_dlg[7].dp = doorct;
	
	if(is_large)
		large_dialog(save_doorset_dlg);
	
	int ret = zc_popup_dialog(save_doorset_dlg,-1);
	jwin_center_dialog(save_doorset_dlg);
	
	if(ret == 8) //OK
	{
		/* sanity bounds
		first_doorset_id = vbound(atoi(firstdoor), 0, (MAXCOMBOS-1));
		the_doorset_count = vbound(atoi(doorct), 1, (MAXCOMBOS-1)-first_doorset_id);
		*/
		if(getname("Save ZDOORS(.zdoors)", "zdoors", NULL,datapath,false))
		{  
			char name[256];
			extract_name(temppath,name,FILENAMEALL);
			PACKFILE *f=pack_fopen_password(temppath,F_WRITE, "");
			if(f)
			{
				al_trace("Saving doorsets %d to %d: %d\n", first_doorset_id, first_doorset_id+(the_doorset_count-1));
				writezdoorsets(f,first_doorset_id,the_doorset_count);
				pack_fclose(f);
				char tmpbuf[512]={0};
				sprintf(tmpbuf,"Saved %s",name);
				jwin_alert("Success!",tmpbuf,NULL,NULL,"O&K",NULL,'k',0,lfont);
			}
		}
	}
}

static DIALOG load_doorset_dlg[] =
{
    // (dialog proc)     (x)   (y)   (w)   (h)   (fg)     (bg)    (key)    (flags)     (d1)           (d2)     (dp)
    { jwin_win_proc,      0,   0,   120,  100,  vc(14),  vc(1),  0,       D_EXIT,          0,             0, (void *) "Door Set (Range)", NULL, NULL },
    { d_timer_proc,         0,    0,     0,    0,    0,       0,       0,       0,          0,          0,         NULL, NULL, NULL },
    //for future tabs
    { d_dummy_proc,         120,  128,  80+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },
    { d_dummy_proc,         120,  128,  80+1,   8+1,    vc(14),  vc(1),  0,       0,          1,             0,       NULL, NULL, NULL },
    //4
    {  jwin_text_proc,        10,    28,     20,      8,    vc(11),     vc(1),      0,    0,          0,    0, (void *) "First:",               NULL,   NULL  },
    { jwin_edit_proc,          55,     26,    40,     16,    vc(12),                 vc(1),                   0,       0,          63,    0,  NULL,                                           NULL,   NULL                  },
    //6
    {  jwin_text_proc,        10,    46,     20,      8,    vc(11),     vc(1),      0,    0,          0,    0, (void *) "Count",               NULL,   NULL  },
    { jwin_edit_proc,          55,     44,    40,     16,    vc(12),                 vc(1),                   0,       0,          63,    0,  NULL,                                           NULL,   NULL                  },
    //8
    { jwin_button_proc,   15,   72,  36,   21,   vc(14),  vc(1),  13,      D_EXIT,     0,             0, (void *) "Load", NULL, NULL },
    { jwin_button_proc,   69,  72,  36,   21,   vc(14),  vc(1),  27,      D_EXIT,     0,             0, (void *) "Cancel", NULL, NULL },
    //{ jwin_check_proc,        10,     46,     95,      9,    vc(14),                 vc(1),                   0,       0,           1,    0, (void *) "Don't Overwrite",                      NULL,   NULL                  },
    
    { NULL,                 0,    0,    0,    0,   0,       0,       0,       0,          0,             0,       NULL,                           NULL,  NULL }
};

void do_importdoorset(const char *prompt,int initialval)
{
	
	char firstdoor[8], doorct[8];;
	int first_doorset_id = 0; int the_doorset_count = 1;
	sprintf(firstdoor,"%d",0);
	sprintf(doorct,"%d",1);
		//int ret;
	
	save_doorset_dlg[0].dp2 = lfont;
	
	sprintf(firstdoor,"%d",0);
	sprintf(doorct,"%d",1);
	
	load_doorset_dlg[5].dp = firstdoor;
	load_doorset_dlg[7].dp = doorct;
	
	byte nooverwrite = 0;
	
	if(is_large)
		large_dialog(load_doorset_dlg);
	
	int ret = zc_popup_dialog(load_doorset_dlg,-1);
	jwin_center_dialog(load_doorset_dlg);
	
	if(ret == 8) //OK
	{
		//if (load_doorset_dlg[10].flags & D_SELECTED) nooverwrite = 1;
	
		//al_trace("Nooverwrite is: %d\n", nooverwrite);
		//sanity bound
		//first_doorset_id = vbound(atoi(firstdoor), 0, (MAXCOMBOS-1));
		//the_doorset_count = vbound(atoi(doorct), 1, NEWMAXTILES-first_doorset_id);
		if(getname("Load ZDOORS(.zdoors)", "zdoors", NULL,datapath,false))
		{  
			char name[256];
			extract_name(temppath,name,FILENAMEALL);
			PACKFILE *f=pack_fopen_password(temppath,F_READ, "");
			if(f)
			{
				
				if (!readzdoorsets(f,first_doorset_id,the_doorset_count))
				{
					al_trace("Could not read from .zdoors packfile %s\n", name);
					jwin_alert("ZDOORS File: Error","Could not load the specified doorsets.",NULL,NULL,"O&K",NULL,'k',0,lfont);
				}
				else
				{
					jwin_alert("ZDOORS File: Success!","Loaded the source doorsets!",NULL,NULL,"O&K",NULL,'k',0,lfont);
					saved=false;
				}
				pack_fclose(f);
			}
		}
	}
}


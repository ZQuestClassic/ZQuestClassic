char weapon_initd_labels[8][65];
	char weap_initdvals[8][13];
	
	//Weapon Scripts for lweapons
    int j = 0; build_belweapons_list(); //lweapon scripts lister
	
	for(j = 0; j < bilweapons_cnt; j++)
	{
		if(bilweapons[j].second == guysbuf[index].weaponscript -1)
		{
			enedata_dlg[292].d1 = j; 
			break;
		}
	}
    
	for ( int q = 0; q < 8; q++ )
	{
	    
		sprintf(weap_initdvals[q],"%.4f",guysbuf[index].weap_initiald[q]/10000.0);
	 
		enedata_dlg[283+q].dp = weap_initdvals[q];
	}
    
	for ( int q = 0; q < 8; q++ )
	{
		//al_trace("Enemy Editor: InitD[%d] string for the npc is: %s\n", q, guysbuf[index].initD_label[q]);
		strcpy(weapon_initd_labels[q], guysbuf[index].weapon_initD_label[q]);
		enedata_dlg[275+q].dp = weapon_initd_labels[q];
		//sprintf();
	    
	}
	
	
	
	/////////////
	
	//begin lweapon scripts
        test.weaponscript = bieweapons[enedata_dlg[292].d1].second + 1; 
	for ( int q = 0; q < 8; q++ )
	{
		test.weap_initiald[q] = vbound(atoi(weap_initdvals[q])*10000,-2147483647, 2147483647);
		strcpy(test.weapon_initD_label[q], weapon_initd_labels[q]);
	}
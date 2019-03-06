char weapon_initd_labels[8][65];
	char initdvals[8][13];
	
	//Weapon Scripts for lweapons
    int j = 0; build_bidmaps_list(); //lweapon scripts lister
	
	for(j = 0; j < bidmaps_cnt; j++)
	{
		if(bidmaps[j].second == DMaps[index].script -1)
		{
			editdmap_dlg[147].d1 = j; 
			break;
		}
	}
    
	for ( int q = 0; q < 8; q++ )
	{
	    
		sprintf(initdvals[q],"%.4f",DMaps[index].initD[q]/10000.0);
	 
		editdmap_dlg[138+q].dp = weap_initdvals[q];
	}
    
	
	test.weaponscript = bilweapons[editdmap_dlg[292].d1].second + 1; 
	for ( int q = 0; q < 8; q++ )
	{
		test.weap_initiald[q] = vbound(atoi(weap_initdvals[q])*10000,-2147483647, 2147483647);
		strcpy(test.weapon_initD_label[q], weapon_initd_labels[q]);
	}
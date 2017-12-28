//Graphics->Colors[] Misc Colours, byte[29]
case GFXCOLOURS:				
{
	switch(ri->d[0] / 10000)
	{
		case 0: ret = ((int)zcolors.text) * 10000; break;
		case 1: ret = ((int)zcolors.caption) * 10000; break;
		case 2: ret = ((int)zcolors.overw_bg) * 10000; break;
		case 3: ret = ((int)zcolors.dngn_bg) * 10000; break;
		case 4: ret = ((int)zcolors.dngn_fg) * 10000; break;
		case 5: ret = ((int)zcolors.bs_dk) * 10000; break;
		case 6: ret = ((int)zcolors.bs_goal) * 10000; break;
		case 7: ret = ((int)zcolors.compass_lt) * 10000; break;
		case 8: ret = ((int)zcolors.compass_dk) * 10000; break;
		case 9: ret = ((int)zcolors.subscr_bg) * 10000; break;
		case 10: ret = ((int)zcolors.subscr_shadow) * 10000; break;
		case 11: ret = ((int)zcolors.triframe_color) * 10000; break;
		case 12: ret = ((int)zcolors.bmap_bg) * 10000; break;
		case 13: ret = ((int)zcolors.bmap_fg) * 10000; break;
		case 14: ret = ((int)zcolors.link_dot) * 10000; break;
		case 15: ret = ((int)zcolors.triforce_cset) * 10000; break;
		case 16: ret = ((int)zcolors.triframe_cset) * 10000; break;
		case 17: ret = ((int)zcolors.overworld_map_cset) * 10000; break;
		case 18: ret = ((int)zcolors.dungeon_map_cset) * 10000; break;
		case 19: ret = ((int)zcolors.blueframe_cset) * 10000; break;
		case 20: ret = ((int)zcolors.triforce_tile) * 10000; break;
		case 21: ret = ((int)zcolors.triframe_tile) * 10000; break;
		case 22: ret = ((int)zcolors.overworld_map_tile) * 10000; break;
		case 23: ret = ((int)zcolors.dungeon_map_tile) * 10000; break;
		case 24: ret = ((int)zcolors.blueframe_tile) * 10000; break;
		case 25: ret = ((int)zcolors.caption) * 10000; break;
		case 26: ret = ((int)zcolors.HCpieces_tile) * 10000; break;
		case 27: ret = ((int)zcolors.HCpieces_cset) * 10000; break;
		case 18: ret = ((int)zcolors.msgtext) * 10000; break;
		default: 
		{
			Z_scripterrlog("Invalid Array Index passed to Graphics->%s: %d\n", indx, "Colors[]");
			ret = -1; 
			break;
		}
	}
	break;
}

//dropsets

//item_drop_sets[MAXITEMDROPSETS];

//needs to be a function. 
case DROPDATANAME:	//CHAR[64]
{
	ret = -1; break; //not implemented
}
case DROPDATAITEM:	//W, 10 INDICES
{
	int ref = ri->dropsetref; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 9 ) 
	{ 
		Z_scripterrlog("Invalid Array Index passed to dropdata->%s: %d\n", indx, "Item[]"); 
		ret = -1;
	} 
	else if ( ref < 0 || ref > ( MAXITEMDROPSETS-1 ) )
	{
		Z_scripterrlog("Invalid Dropset ID passed to dropdata->%s: %d\n", indx, "Item[]"); 
		ret = -1;
	}
	else ret = ((int)item_drop_sets[ref].item[indx] * 10000;
	break;
}
case DROPDATACHANCE:	//W, 11 INDICES
{
	int ref = ri->dropsetref; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 10 ) 
	{ 
		Z_scripterrlog("Invalid Array Index passed to dropdata->%s: %d\n", indx, "Chance[]"); 
		ret = -1;
	} 
	else if ( ref < 0 || ref > ( MAXITEMDROPSETS-1 ) )
	{
		Z_scripterrlog("Invalid Dropset ID passed to dropdata->%s: %d\n", indx, "Item[]"); 
		ret = -1;
	}
	else ret = ((int)item_drop_sets[ref].chance[indx] * 10000;
	break;
}


//miscQdata
//this should not be a pointer object. These belong under Game-> 
case QUESTTRIFORCE:		//C, 8 INDICES
{
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 7 ) 
	{ 
		Z_scripterrlog("Invalid Array Index passed to Game->Triforce[]: %d\n", indx);
	} 
	else
	{
		ret = ((int)(misc->triforce[indx]) * 10000); 
	}	
	break;
}
case QUESTENDSTRING:		//W, STRING EDITOR VALUE
{
	ret = ((int)(misc->endstring) * 10000);	break;
}
case QUESTICON:		//W, 4 INDICES
{
	
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 3 ) 
	{ 
		Z_scripterrlog("Invalid Array Index passed to Game->Icons[]: %d\n", indx);
	} 
	else
	{
		ret = ((int)(misc->icons[indx])) * 10000; 
	}	
	break;
	
}

//warpring
//case WARPRINGNAME: 	//CHAR[32]
case WARPRINGDMAP:	//W, 9 INDICES
{
	int ref = ri->warpringref; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 9 ) 
	{ 
		Z_scripterrlog("Invalid Array Index passed to dropdata->%s: %d\n", indx, "Item[]"); 
	} 
	ret = ((int)misc->warp[ref].dmap[indx]) * 10000;
	break;
}
	
}
case WARPRINGSCREEN:	//W, 9 INDICES
{
	int ref = ri->warpringref; 
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 9 ) 
	{ 
		Z_scripterrlog("Invalid Array Index passed to dropdata->%s: %d\n", indx, "Item[]"); 
	} 
	ret = ((int)misc->warp[ref].scr[indx]) * 10000;
	break;
}
case WARPRINGSIZE:	//C
{
	int ref = ri->warpringref; 
	ret = ((int)misc->warp[ref].size) * 10000;
	break;
}

//zcheats

/*

//ZCHEATS
case CHEATFLAGS:	//W
case CHEATCODELV1	//CHAR[41], LET'S USE A DIRECT STRING?
case CHEATCODELV2	//CHAR[41], LET'S USE A DIRECT STRING?
case CHEATCODELV3	//CHAR[41], LET'S USE A DIRECT STRING?
case CHEATCODELV4	//CHAR[41], LET'S USE A DIRECT STRING?
*/

//pal cycles
case PALCYCLEFIRST:	//C
{
	int ref = ri->palcycleref; //max 256 of these.
	if ( ref < 0 || ref > 255 ) 
	{
		Z_scripterrlog("Invalid Palette Cycle ID passed to palcycle->%s: %d\n", ref, "First"); 
		ret = -1;
	} 
	else ret = ((int)misc->cycles[ref].first) * 10000;
	break;
}
case PALCYCLECOUNT:	//C
{
	int ref = ri->palcycleref;
	if ( ref < 0 || ref > 255 ) 
	{
		Z_scripterrlog("Invalid Palette Cycle ID passed to palcycle->%s: %d\n", ref, "Count"); 
		ret = -1;
	} 
	else ret = ((int)misc->cycles[ref].count) * 10000;
	break;
}

case PALCYCLESPEED:	//C
{
	int ref = ri->palcycleref; 
	if ( ref < 0 || ref > 255 ) 
	{
		Z_scripterrlog("Invalid Palette Cycle ID passed to palcycle->%s: %d\n", ref, "Speed"); 
		ret = -1;
	} 
	else ret = ((int)misc->cycles[ref].speed) * 10000;
	break;
}

//tunes
//ZCTUNETITLE	//CHAR[36]
case ZCTUNESTART:	//LONG
{
	int ref = ri->tunesref; 
	ret = ((int)zctune[ref].start) * 10000;
	break;
}	
	
case ZCTUNELOOPSTART:	//LONG
{
	int ref = ri->tunesref; 
	ret = ((int)zctune[ref].loop_start) * 10000;
	break;
}

case ZCTUNELOOPEND:	//LONG
{
	int ref = ri->tunesref; 
	ret = ((int)zctune[ref].loop_end) * 10000;
	break;
}

case ZCTUNELOOP:	//SHORT
{
	int ref = ri->tunesref; 
	ret = ((int)zctune[ref].loop) * 10000;
	break;
}

case ZCTUNEVOLUME:	//SHORT
{
	int ref = ri->tunesref; 
	ret = ((int)zctune[ref].volume) * 10000;
	break;
}
case ZCTUNEFLAGS:	//C
{
	int ref = ri->tunesref; 
	ret = ((int)zctune[ref].flags) * 10000;
	break;
}

case ZCTUNEFORMAT:	//C --READ ONLY IS PROBABLY BEST
{
	int ref = ri->tunesref; 
	ret = ((int)zctune[ref].format) * 10000;
	break;
}

//quest info
//GAMEQUESTVERSION	//CHAR[9]
//GAMEQUESTTITLE		//CHAR[65]

//zcolors --these should all be palette indices, not raw RGB data
//this also does not need a ref...
//they may as well be ZColors->member

//Put these under Graphics-> for now. 

//in fact, this would probably be better as an array! Game->ZColors[], except that it has two arrays in it. 
case ZCOLOURTEXT:	//C
{
	//int ref = ri->zcoloursref; 
	ret = ((int)zcolors.text) * 10000;
	break;
}
case ZCOLOURCAPTION:	//C
{
	//int ref = ri->zcoloursref; 
	ret = ((int)zcolors.caption) * 10000;
	break;
}

case ZCOLOUROVERWORLDBG:			//C
{
	ret = ((int)zcolors.overw_bg) * 10000;
	break;
}

case ZCOLOURDUNGEONBG:			//C
{
	ret = ((int)zcolors.dngn_bg) * 10000;
	break;
}
case ZCOLOURDUNGEONFG:			//C
{
	ret = ((int)zcolors.dngn_fg) * 10000;
	break;
}
case ZCOLOURCAVEFG:				//C
{
	ret = ((int)zcolors.bs_dk) * 10000;
	break;
}
case ZCOLOURBSDARK:				//C
{
	ret = ((int)zcolors.bs_goal) * 10000;
	break;
}
case ZCOLOURBSGOAL:				//C
{
	ret = ((int)zcolors.compass_lt) * 10000;
	break;
}
case ZCOLOURCOMPASSLIGHT:			//C
{
	ret = ((int)zcolors.compass_dk) * 10000;
	break;
}
case ZCOLOURCOMPASSDARK:		//C
{
	ret = ((int)zcolors.subscr_bg) * 10000;
	break;
}
case ZCOLOURSUBSCREENBG:			//C
{
	ret = ((int)zcolors.subscr_shadow) * 10000;
	break;
}
case ZCOLOURSUBSCREENSHADOW:			//C
{
	ret = ((int)zcolors.triframe_color) * 10000;
	break;
}
case ZCOLOURTRIFRAME:				//C
{
	ret = ((int)zcolors.bmap_bg) * 10000;
	break;
}
case ZCOLOURBIGMAPBG:				//C
{
	ret = ((int)zcolors.bmap_fg) * 10000;
	break;
}
case ZCOLOURBIGMAPFG:				//C
{
	ret = ((int)zcolors.link_dot) * 10000;
	break;
}
case ZCOLOURLINKDOT:				//C
{
	ret = ((int)zcolors.triforce_cset) * 10000;
	break;
}
case ZCOLOURTRIFORCECSET:			//C
{
	ret = ((int)zcolors.triframe_cset) * 10000;
	break;
}
case ZCOLOURTRIFRAMECSET:			//C
{
	ret = ((int)zcolors.overworld_map_cset) * 10000;
	break;
}
case ZCOLOUROVERWORLDMAPCSET:			//C
{
	ret = ((int)zcolors.dungeon_map_cset) * 10000;
	break;
}
case ZCOLOURDUNGEONMAPCSET:			//C
{
	ret = ((int)zcolors.blueframe_cset) * 10000;
	break;
}
case ZCOLOURBLUEFRAMECSET:			//C
{
	ret = ((int)zcolors.triforce_tile) * 10000;
	break;
}
case ZCOLOURTRIFORCETILE:			//W
{
	ret = ((int)zcolors.triframe_tile) * 10000;
	break;
}
case ZCOLOURTRIFRAMETILE:			//W
{
	ret = ((int)zcolors.overworld_map_tile) * 10000;
	break;
}
case ZCOLOUROVERWORLDMAPTILE:			//W
{
	ret = ((int)zcolors.dungeon_map_tile) * 10000;
	break;
}
case ZCOLOURDUNGEONMAPTILE:			//W
{
	ret = ((int)zcolors.blueframe_tile) * 10000;
	break;
}
case ZCOLOURBLUEFRAMETILE:			//W
{
	ret = ((int)zcolors.caption) * 10000;
	break;
}
case ZCOLOURHCPIECETILE:			//W
{
	ret = ((int)zcolors.HCpieces_tile) * 10000;
	break;
}
case ZCOLOURHCPIECECSET:			//C
{
	ret = ((int)zcolors.HCpieces_cset) * 10000;
	break;
}
case ZCOLOURMSGTEXT:				//C
{
	ret = ((int)zcolors.msgtext) * 10000;
	break;
}
case ZCOLOURFOO:				//C, 6 INDICES
{
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 5 ) 
	{ 
		Z_scripterrlog("Invalid Array Index passed to Colors->%s: %d\n", indx, "Foo[]"); 
	} 
	ret = ((int)zcolors.Foo[indx]) * 10000;
	break;
}
case ZCOLOURFOO2:				//C, 256 INDICES
{
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || indx > 255 ) 
	{ 
		Z_scripterrlog("Invalid Array Index passed to Colors->%s: %d\n", indx, "Foo2[]"); 
	} 
	ret = ((int)zcolors.Foo2[indx]) * 10000;
	break;
}
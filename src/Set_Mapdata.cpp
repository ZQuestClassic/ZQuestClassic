case MAPDATAVALID:		SET_MAPDATA_VAR_BYTE(valid, "Valid"); break;		//b
case MAPDATAGUY: 		SET_MAPDATA_VAR_BYTE(guy, "Guy"); break;		//b
case MAPDATASTRING:		SET_MAPDATA_VAR_INT32(str, "String"); break;		//w
case MAPDATAROOM: 		SET_MAPDATA_VAR_BYTE(room, "RoomType");	break;		//b
case MAPDATAITEM: 		SET_MAPDATA_VAR_BYTE(item, "Item"); break;		//b
case MAPDATAHASITEM: 		SET_MAPDATA_VAR_BYTE(hasitem, "HasItem"); break;	//b
case MAPDATATILEWARPTYPE: 	SET_MAPDATA_BYTE_INDEX(tilewarptype, "TileWarpType"); break;	//b, 4 of these
case MAPDATATILEWARPOVFLAGS: 	SET_MAPDATA_VAR_BYTE(tilewarpoverlayflags, "TileWarpOverlayFlags"); break;	//b, tilewarpoverlayflags
case MAPDATADOORCOMBOSET: 	SET_MAPDATA_VAR_INT32(door_combo_set, "DoorComboSet"); break;	//w
case MAPDATAWARPRETX:	 	SET_MAPDATA_BYTE_INDEX(warpreturnx, "WarpReturnX"); break;	//b, 4 of these
case MAPDATAWARPRETY:	 	SET_MAPDATA_BYTE_INDEX(warpreturny, "WarpReturnY"); break;	//b, 4 of these
case MAPDATAWARPRETURNC: 	SET_MAPDATA_VAR_INT32(warpreturnc, "WarpReturnC"); break;	//w
case MAPDATASTAIRX: 		SET_MAPDATA_VAR_BYTE(stairx, "StairsX"); break;	//b
case MAPDATASTAIRY: 		SET_MAPDATA_VAR_BYTE(stairy, "StairsY"); break;	//b
case MAPDATAITEMX:		SET_MAPDATA_VAR_BYTE(itemx, "ItemX"); break; //itemx
case MAPDATAITEMY:		SET_MAPDATA_VAR_BYTE(itemy, "ItemY"); break;	//itemy
case MAPDATACOLOUR: 		SET_MAPDATA_VAR_INT32(color, "CSet"); break;	//w
case MAPDATAENEMYFLAGS: 	SET_MAPDATA_VAR_BYTE(enemyflags, "EnemyFlags");	break;	//b
case MAPDATADOOR: 		SET_MAPDATA_BYTE_INDEX(door, "Door"); break;	//b, 4 of these
case MAPDATATILEWARPDMAP: 	SET_MAPDATA_VAR_INDEX32(tilewarpdmap, "TileWarpDMap"); break;	//w, 4 of these
case MAPDATATILEWARPSCREEN: 	SET_MAPDATA_BYTE_INDEX(tilewarpscr, "TileWarpScreen"); break;	//b, 4 of these
case MAPDATAEXITDIR: 		SET_MAPDATA_VAR_BYTE(exitdir, "ExitDir"); break;	//b
case MAPDATAENEMY: 		SET_MAPDATA_VAR_INDEX32(enemy, "Enemy"); break;	//w, 10 of these
case MAPDATAPATTERN: 		SET_MAPDATA_VAR_BYTE(pattern, "Pattern"); break;	//b
case MAPDATASIDEWARPTYPE: 	SET_MAPDATA_BYTE_INDEX(sidewarptype, "SideWarpType"); break;	//b, 4 of these
case MAPDATASIDEWARPOVFLAGS: 	SET_MAPDATA_VAR_BYTE(sidewarpoverlayflags, "SideWarpOverlayFlags"); break;	//b
case MAPDATAWARPARRIVALX: 	SET_MAPDATA_VAR_BYTE(warparrivalx, "WarpArrivalX"); break;	//b
case MAPDATAWARPARRIVALY: 	SET_MAPDATA_VAR_BYTE(warparrivaly, "WarpArrivalY"); break;	//b
case MAPDATAPATH: 		SET_MAPDATA_BYTE_INDEX(path, "MazePath"); break;	//b, 4 of these
case MAPDATASIDEWARPSC: 	SET_MAPDATA_BYTE_INDEX(sidewarpscr, "SideWarpScreen"); break;	//b, 4 of these
case MAPDATASIDEWARPDMAP: 	SET_MAPDATA_VAR_INDEX32(sidewarpdmap, "SideWarpDMap"); break;	//w, 4 of these
case MAPDATASIDEWARPINDEX: 	SET_MAPDATA_VAR_BYTE(sidewarpindex, "SideWarpIndex"); break;	//b
case MAPDATAUNDERCOMBO: 	SET_MAPDATA_VAR_INT32(undercombo, "Undercombo"); break;	//w
case MAPDATAUNDERCSET:	 	SET_MAPDATA_VAR_BYTE(undercset,	"UnderCSet"); break; //b
case MAPDATACATCHALL:	 	SET_MAPDATA_VAR_INT32(catchall,	"Catchall"); break; //W

case MAPDATACSENSITIVE: 	SET_MAPDATA_VAR_BYTE(csensitive, "CSensitive"); break;	//B
case MAPDATANORESET: 		SET_MAPDATA_VAR_INT32(noreset, "NoReset"); break;	//W
case MAPDATANOCARRY: 		SET_MAPDATA_VAR_INT32(nocarry, "NoCarry"); break;	//W
case MAPDATALAYERMAP:	 	SET_MAPDATA_BYTE_INDEX(layermap, "LayerMap"); break;	//B, 6 OF THESE
case MAPDATALAYERSCREEN: 	SET_MAPDATA_BYTE_INDEX(layerscreen, "LayerScreen"); break;	//B, 6 OF THESE
case MAPDATALAYEROPACITY: 	SET_MAPDATA_BYTE_INDEX(layeropacity, "LayerOpacity"); break;	//B, 6 OF THESE
case MAPDATATIMEDWARPTICS: 	SET_MAPDATA_VAR_INT32(timedwarptics, "TimedWarpTimer"); break;	//W
case MAPDATANEXTMAP: 		SET_MAPDATA_VAR_BYTE(nextmap, "NextMap"); break;	//B
case MAPDATANEXTSCREEN: 	SET_MAPDATA_VAR_BYTE(nextscr, "NextScreen"); break;	//B
case MAPDATASECRETCOMBO: 	SET_MAPDATA_VAR_INDEX32(secretcombo, "SecretCombo"); break;	//W, 128 OF THESE
case MAPDATASECRETCSET: 	SET_MAPDATA_BYTE_INDEX(secretcset, "SecretCSet"); break;	//B, 128 OF THESE
case MAPDATASECRETFLAG: 	SET_MAPDATA_BYTE_INDEX(secretflag, "SecretFlags"); break;	//B, 128 OF THESE
case MAPDATAVIEWX: 		SET_MAPDATA_VAR_INT32(viewX, "ViewX"); break;	//W
case MAPDATAVIEWY: 		SET_MAPDATA_VAR_INT32(viewY, "ViewY"); break; //W
case MAPDATASCREENWIDTH: 	SET_MAPDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
case MAPDATASCREENHEIGHT: 	SET_MAPDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
case MAPDATAENTRYX: 		SET_MAPDATA_VAR_BYTE(entry_x, "EntryX"); break;	//B
case MAPDATAENTRYY: 		SET_MAPDATA_VAR_BYTE(entry_y, "EntryY"); break;	//B
case MAPDATANUMFF: 		SET_MAPDATA_VAR_INT16(numff, "NumFFCs"); break;	//INT16
case MAPDATAFFDATA: 		SET_MAPDATA_VAR_INDEX32(ffdata,	"FFCData"); break;	//W, 32 OF THESE
case MAPDATAFFCSET: 		SET_MAPDATA_BYTE_INDEX(ffcset,	"FFCCSet"); break;	//B, 32
case MAPDATAFFDELAY: 		SET_MAPDATA_VAR_INDEX32(ffdelay, "FFCDelay"); break;	//W, 32
case MAPDATAFFX: 		SET_MAPDATA_VAR_INDEX32(ffx, "FFCX"); break;	//INT32, 32 OF THESE
case MAPDATAFFY: 		SET_MAPDATA_VAR_INDEX32(ffy, "FFCY"); break;	//..
case MAPDATAFFXDELTA:	 	SET_MAPDATA_VAR_INDEX32(ffxdelta, "FFCVx"); break;	//..
case MAPDATAFFYDELTA: 		SET_MAPDATA_VAR_INDEX32(ffydelta, "FFCVy"); break;	//..
case MAPDATAFFXDELTA2:	 	SET_MAPDATA_VAR_INDEX32(ffxdelta2, "FFCAx"); break;	//..
case MAPDATAFFYDELTA2:	 	SET_MAPDATA_VAR_INDEX32(ffydelta2, "FFCAy"); break;	//..
case MAPDATAFFFLAGS: 		SET_MAPDATA_VAR_INDEX16(ffflags, "FFCFlags"); break;	//INT16, 23 OF THESE
//Height and With are Or'd together, and need to be separate:
/*
 //TileWidth ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref] & ~63) | (((value/10000)-1)&63);
*/
case MAPDATAFFWIDTH: 		
{
	mapscreen *m = ri->mapsref;
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || index > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCTileWidth[]: %d\n", indx);
		break;
	}
	if ( (value/10000) < 0 || (value/10000) > 4 ) 
	{
		Z_scripterrlog("Invalid WIDTH value passed to MapData->FFCTileWidth[]: %d\n", value);
		break;
	}
	m->ffwidth[indx] = ( m->ffwidth[indx]&~63) | vbound( (((value/10000)-1)&63), (0&63), (214747&63) ); 
	break;
}	


//SET_MAPDATA_BYTE_INDEX(ffwidth, "FFCTileWidth");	//B, 32 OF THESE
case MAPDATAFFHEIGHT:	 	
{
	mapscreen *m = ri->mapsref;
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || index > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCTileHeight[]: %d\n", indx);
		break;
	}
	if ( (value/10000) < 0 || (value/10000) > 4 ) 
	{
		Z_scripterrlog("Invalid WIDTH value passed to MapData->FFCTileHeight[]: %d\n", value);
		break;
	}
	m->ffheight[indx] = ( m->ffheight[indx]&~63) | vbound( (((value/10000)-1)&63), (0&63), (4&63) ); 
	break;
	
}

//EffectWidth tmpscr->ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref]&63) | ((((value/10000)-1)&3)<<6);

//SET_MAPDATA_BYTE_INDEX(ffheight, "FFCTileHeight"	//B, 32 OF THESE
case MAPDATAFFEFFECTWIDTH: 		
{ 
	mapscreen *m = ri->mapsref;
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || index > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCEffectWidth[]: %d\n", indx);
		break;
	}
	if ( (value/10000) < 0 ) 
	{
		Z_scripterrlog("Invalid WIDTH value passed to MapData->FFCEffectWidth[]: %d\n", value);
		break;
	}
	m->ffwidth[indx] = ( m->ffwidth[indx]&63) | vbound( ((((value/10000)-1)&3)<<6), ((((0)-1)&3)<<6), ((((214747)&3)<<6)) );
	break;
}


//SET_MAPDATA_BYTE_INDEX(ffwidth, "FFCEffectWidth");	//B, 32 OF THESE
case MAPDATAFFEFFECTHEIGHT:
{ 
	mapscreen *m = ri->mapsref;
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || index > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCEffectHeight[]: %d\n", indx);
		break;
	}
	if ( (value/10000) < 0 ) 
	{
		Z_scripterrlog("Invalid HEIGHT value passed to MapData->FFCEffectHeight[]: %d\n", value);
		break;
	}
	m->ffheight[indx] = ( m->ffheight[indx]&63) | vbound( ((((value/10000)-1)&3)<<6), ((((0)-1)&3)<<6), ((((214747)&3)<<6)) );
	break;
}	
	
//SET_MAPDATA_BYTE_INDEX(ffheight, "FFCEffectHeight"	//B, 32 OF THESE	

case MAPDATAFFLINK: 		SET_MAPDATA_BYTE_INDEX(fflink, "FFCLink"); break;	//B, 32 OF THESE
case MAPDATAFFSCRIPT:	 	SET_MAPDATA_VAR_INDEX32(ffscript, "FFCScript"); break;	//W, 32 OF THESE
case MAPDATAINTID: 	 //Same form as SetScreenD()
	//SetFFCInitD(ffindex, d, value)
{
	mapscreen *m = ri->mapsref;
	//int ffindex = ri->d[0]/10000;
	//int d = ri->d[1]/10000;
	//int v = (value/10000);
	m->initd[(ri->d[0]/10000)][(ri->d[1]/10000)] = (value/10000);
	break;
}	
	

//initd	//INT32 , 32 OF THESE, EACH WITH 10 INDICES. 


case MAPDATAINITA: 		
	//same form as SetScreenD
{
	mapscreen *m = ri->mapsref;
	//int ffindex = ri->d[0]/10000;
	//int d = ri->d[1]/10000;
	//int v = (value/10000);
	m->inita[(ri->d[0]/10000)][(ri->d[1]/10000)] = (value/10000);
	break;
}	
	
/* Getters
case MAPDATAINTID: 	 //Same form as SetDMapScreenD()
	//GetFFCInitD(ffindex, d)
{
	mapscreen *m = ri->mapsref;
	//int ffindex = ri->d[0]/10000;
	//int d = ri->d[1]/10000;
	ret = (m->initd[(ri->d[0]/10000)][ri->d[1]/10000]);
	break;
}	
	

//initd	//INT32 , 32 OF THESE, EACH WITH 10 INDICES. 


case MAPDATAINITA: 		
	//same form as SetDMapScreenD
{
	mapscreen *m = ri->mapsref;
	//int ffindex = ri->d[0]/10000;
	//int d = ri->d[1]/10000;
	ret = (m->inita[(ri->d[0]/10000)][ri->d[1]/10000]);
	break;
}
*

/*

//int GetFFCInitD(mapscr, int,int,int)
    {
        int id = memberids["GetFFCInitD"];
	int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(MAPDATAINTID)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
	
    }
    
     //void SetFFCInitD(mapsc, int,int,int,int)
    {
        int id = memberids["SetFFCInitD"];
	int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(MAPDATAINTID), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    
    //int GetFFCInitA(mapscr, int,int,int)
    {
        int id = memberids["GetFFCInitD"];
	int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(MAPDATAINTIA)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
	
    }
    
     //void SetFFCInitA(mapsc, int,int,int,int)
    {
        int id = memberids["SetFFCInitD"];
	int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(MAPDATAINTIA), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    
    

*/
	//inita	//INT32, 32 OF THESE, EACH WITH 2
case MAPDATAFFINITIALISED: 	SET_MAPDATA_BOOL_INDEX(initialized, "FFCRunning"); break;	//BOOL, 32 OF THESE
case MAPDATASCRIPTENTRY: 	SET_MAPDATA_VAR_INT32(script_entry, "ScriptEntry"); break;	//W
case MAPDATASCRIPTOCCUPANCY: 	SET_MAPDATA_VAR_INT32(script_occupancy,	"ScriptOccupancy");  break;//W
case MAPDATASCRIPTEXIT: 	SET_MAPDATA_VAR_INT32(script_exit, "ExitScript"); break;	//W
case MAPDATAOCEANSFX:	 	SET_MAPDATA_VAR_BYTE(oceansfx, "OceanSFX"); break;	//B
case MAPDATABOSSSFX: 		SET_MAPDATA_VAR_BYTE(bosssfx, "BossSFX"); break;	//B
case MAPDATASECRETSFX:	 	SET_MAPDATA_VAR_BYTE(secretsfx, "SecretSFX"); break;	//B
case MAPDATAHOLDUPSFX:	 	SET_MAPDATA_VAR_BYTE(holdupsfx,	"ItemSFX"); break; //B
case MAPDATASCREENMIDI: 	SET_MAPDATA_VAR_INT16(screen_midi, "MIDI"); break;	//SHORT, OLD QUESTS ONLY?
case MAPDATALENSLAYER:	 	SET_MAPDATA_VAR_BYTE(lens_layer, "LensLayer"); break;	//B, OLD QUESTS ONLY?
	

case MAPDATAFLAGS: 
{
	int flagid = ri->d[0])/10000;
	mapscreen *m = ri->mapsref; 
	//bool valtrue = ( value ? 10000 : 0);
	switch(flagid)
	{
		case 0: m->flags = (value / 10000); break;
		case 1: m->flags1 = (value / 10000); break;
		case 2: m->flags2 = (value / 10000); break;
		case 3: m->flags3 = (value / 10000); break;
		case 4: m->flags4 = (value / 10000); break;
		case 5: m->flags5 = (value / 10000); break;
		case 6: m->flags6 = (value / 10000); break;
		case 7: m->flags7 = (value / 10000); break;
		case 8: m->flags8 = (value / 10000); break;
		case 9: m->flags9 = (value / 10000); break;
		case 10: m->flags10 = (value / 10000); break;
		default:
		{
			Z_scripterrlog("Invalid index passed to mapdata->flags[]: %d\n", flagid); 
			break;
			
		}
	}
	break;
	//SET_MAPDATA_BYTE_INDEX	//B, 11 OF THESE, flags, flags2-flags10
}

/* Probably needs to be a function?
SetFlags(int flagset, int flag, bool state)
case MAPDATAFLAGS: 
{
	int flagid = ri->d[0])/10000;
	mapscreen *m = ri->mapsref; 
	bool valtrue = ( value ? 10000 : 0);
	switch(flagid)
	{
		case 0:
		{
			if ( valtrue ) m->flags |= 
			else ( 
		case 1: flags2
		case 10: flags10
	}
	//SET_MAPDATA_BYTE_INDEX	//B, 11 OF THESE, flags, flags2-flags10

 value ? tmpscr->ffflags[ri->ffcref] |=   1<<((ri->d[0])/10000)
                : tmpscr->ffflags[ri->ffcref] &= ~(1<<((ri->d[0])/10000));
}
*/

case MAPDATAMISCD:
int indx = ri->d[0])/10000;
mapscreen *m = ri->mapsref; 
int FFScript::get_screen_d(long index1, long index2)
{
    if(index2 < 0 || index2 > 7)
    {
        Z_scripterrlog("You were trying to reference an out-of-bounds array index for a screen's D[] array (%ld); valid indices are from 0 to 7.\n", index1);
        break;
    }
    else {
	    game->screen_d[ri->mapsref][indx] = val/10000;
	    break;
    }
}
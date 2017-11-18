case MAPDATAVALID:		GET_MAPDATA_VAR_BYTE(valid, "Valid"); break;		//b
case MAPDATAGUY: 		GET_MAPDATA_VAR_BYTE(guy, "Guy"); break;		//b
case MAPDATASTRING:		GET_MAPDATA_VAR_INT32(str, "String"); break;		//w
case MAPDATAROOM: 		GET_MAPDATA_VAR_BYTE(room, "RoomType");	break;		//b
case MAPDATAITEM: 		GET_MAPDATA_VAR_BYTE(item, "Item"); break;		//b
case MAPDATAHASITEM: 		GET_MAPDATA_VAR_BYTE(hasitem, "HasItem"); break;	//b
case MAPDATATILEWARPTYPE: 	GET_MAPDATA_BYTE_INDEX(tilewarptype, "TileWarpType"); break;	//b, 4 of these
case MAPDATATILEWARPOVFLAGS: 	GET_MAPDATA_VAR_BYTE(tilewarpoverlayflags, "TileWarpOverlayFlags"); break;	//b, tilewarpoverlayflags
case MAPDATADOORCOMBOSET: 	GET_MAPDATA_VAR_INT32(door_combo_set, "DoorComboSet"); break;	//w
case MAPDATAWARPRETX:	 	GET_MAPDATA_BYTE_INDEX(warpreturnx, "WarpReturnX"); break;	//b, 4 of these
case MAPDATAWARPRETY:	 	GET_MAPDATA_BYTE_INDEX(warpreturny, "WarpReturnY"); break;	//b, 4 of these
case MAPDATAWARPRETURNC: 	GET_MAPDATA_VAR_INT32(warpreturnc, "WarpReturnC"); break;	//w
case MAPDATASTAIRX: 		GET_MAPDATA_VAR_BYTE(stairx, "StairsX"); break;	//b
case MAPDATASTAIRY: 		GET_MAPDATA_VAR_BYTE(stairy, "StairsY"); break;	//b
case MAPDATAITEMX:		GET_MAPDATA_VAR_BYTE(itemx, "ItemX"); break; //itemx
case MAPDATAITEMY:		GET_MAPDATA_VAR_BYTE(itemy, "ItemY"); break;	//itemy
case MAPDATACOLOUR: 		GET_MAPDATA_VAR_INT32(color, "CSet"); break;	//w
case MAPDATAENEMYFLAGS: 	GET_MAPDATA_VAR_BYTE(enemyflags, "EnemyFlags");	break;	//b
case MAPDATADOOR: 		GET_MAPDATA_BYTE_INDEX(door, "Door"); break;	//b, 4 of these
case MAPDATATILEWARPDMAP: 	GET_MAPDATA_VAR_INDEX32(tilewarpdmap, "TileWarpDMap"); break;	//w, 4 of these
case MAPDATATILEWARPSCREEN: 	GET_MAPDATA_BYTE_INDEX(tilewarpscr, "TileWarpScreen"); break;	//b, 4 of these
case MAPDATAEXITDIR: 		GET_MAPDATA_VAR_BYTE(exitdir, "ExitDir"); break;	//b
case MAPDATAENEMY: 		GET_MAPDATA_VAR_INDEX32(enemy, "Enemy"); break;	//w, 10 of these
case MAPDATAPATTERN: 		GET_MAPDATA_VAR_BYTE(pattern, "Pattern"); break;	//b
case MAPDATASIDEWARPTYPE: 	GET_MAPDATA_BYTE_INDEX(sidewarptype, "SideWarpType"); break;	//b, 4 of these
case MAPDATASIDEWARPOVFLAGS: 	GET_MAPDATA_VAR_BYTE(sidewarpoverlayflags, "SideWarpOverlayFlags"); break;	//b
case MAPDATAWARPARRIVALX: 	GET_MAPDATA_VAR_BYTE(warparrivalx, "WarpArrivalX"); break;	//b
case MAPDATAWARPARRIVALY: 	GET_MAPDATA_VAR_BYTE(warparrivaly, "WarpArrivalY"); break;	//b
case MAPDATAPATH: 		GET_MAPDATA_BYTE_INDEX(path, "MazePath"); break;	//b, 4 of these
case MAPDATASIDEWARPSC: 	GET_MAPDATA_BYTE_INDEX(sidewarpscr, "SideWarpScreen"); break;	//b, 4 of these
case MAPDATASIDEWARPDMAP: 	GET_MAPDATA_VAR_INDEX32(sidewarpdmap, "SideWarpDMap"); break;	//w, 4 of these
case MAPDATASIDEWARPINDEX: 	GET_MAPDATA_VAR_BYTE(sidewarpindex, "SideWarpIndex"); break;	//b
case MAPDATAUNDERCOMBO: 	GET_MAPDATA_VAR_INT32(undercombo, "Undercombo"); break;	//w
case MAPDATAUNDERCSET:	 	GET_MAPDATA_VAR_BYTE(undercset,	"UnderCSet"); break; //b
case MAPDATACATCHALL:	 	GET_MAPDATA_VAR_INT32(catchall,	"Catchall"); break; //W

case MAPDATACSENSITIVE: 	GET_MAPDATA_VAR_BYTE(csensitive, "CSensitive"); break;	//B
case MAPDATANORESET: 		GET_MAPDATA_VAR_INT32(noreset, "NoReset"); break;	//W
case MAPDATANOCARRY: 		GET_MAPDATA_VAR_INT32(nocarry, "NoCarry"); break;	//W
case MAPDATALAYERMAP:	 	GET_MAPDATA_BYTE_INDEX(layermap, "LayerMap"); break;	//B, 6 OF THESE
case MAPDATALAYERSCREEN: 	GET_MAPDATA_BYTE_INDEX(layerscreen, "LayerScreen"); break;	//B, 6 OF THESE
case MAPDATALAYEROPACITY: 	GET_MAPDATA_BYTE_INDEX(layeropacity, "LayerOpacity"); break;	//B, 6 OF THESE
case MAPDATATIMEDWARPTICS: 	GET_MAPDATA_VAR_INT32(timedwarptics, "TimedWarpTimer"); break;	//W
case MAPDATANEXTMAP: 		GET_MAPDATA_VAR_BYTE(nextmap, "NextMap"); break;	//B
case MAPDATANEXTSCREEN: 	GET_MAPDATA_VAR_BYTE(nextscr, "NextScreen"); break;	//B
case MAPDATASECRETCOMBO: 	GET_MAPDATA_VAR_INDEX32(secretcombo, "SecretCombo"); break;	//W, 128 OF THESE
case MAPDATASECRETCSET: 	GET_MAPDATA_BYTE_INDEX(secretcset, "SecretCSet"); break;	//B, 128 OF THESE
case MAPDATASECRETFLAG: 	GET_MAPDATA_BYTE_INDEX(secretflag, "SecretFlags"); break;	//B, 128 OF THESE
case MAPDATAVIEWX: 		GET_MAPDATA_VAR_INT32(viewX, "ViewX"); break;	//W
case MAPDATAVIEWY: 		GET_MAPDATA_VAR_INT32(viewY, "ViewY"); break; //W
case MAPDATASCREENWIDTH: 	GET_MAPDATA_VAR_BYTE(scrWidth, "Width"); break;	//B
case MAPDATASCREENHEIGHT: 	GET_MAPDATA_VAR_BYTE(scrHeight,	"Height"); break;	//B
case MAPDATAENTRYX: 		GET_MAPDATA_VAR_BYTE(entry_x, "EntryX"); break;	//B
case MAPDATAENTRYY: 		GET_MAPDATA_VAR_BYTE(entry_y, "EntryY"); break;	//B
case MAPDATANUMFF: 		GET_MAPDATA_VAR_INT16(numff, "NumFFCs"); break;	//INT16
case MAPDATAFFDATA: 		GET_MAPDATA_VAR_INDEX32(ffdata,	"FFCData"); break;	//W, 32 OF THESE
case MAPDATAFFCSET: 		GET_MAPDATA_BYTE_INDEX(ffcset,	"FFCCSet"); break;	//B, 32
case MAPDATAFFDELAY: 		GET_MAPDATA_VAR_INDEX32(ffdelay, "FFCDelay"); break;	//W, 32
case MAPDATAFFX: 		GET_MAPDATA_VAR_INDEX32(ffx, "FFCX"); break;	//INT32, 32 OF THESE
case MAPDATAFFY: 		GET_MAPDATA_VAR_INDEX32(ffy, "FFCY"); break;	//..
case MAPDATAFFXDELTA:	 	GET_MAPDATA_VAR_INDEX32(ffxdelta, "FFCVx"); break;	//..
case MAPDATAFFYDELTA: 		GET_MAPDATA_VAR_INDEX32(ffydelta, "FFCVy"); break;	//..
case MAPDATAFFXDELTA2:	 	GET_MAPDATA_VAR_INDEX32(ffxdelta2, "FFCAx"); break;	//..
case MAPDATAFFYDELTA2:	 	GET_MAPDATA_VAR_INDEX32(ffydelta2, "FFCAy"); break;	//..
case MAPDATAFFFLAGS: 		GET_MAPDATA_VAR_INDEX16(ffflags, "FFCFlags"); break;	//INT16, 23 OF THESE
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
		ret = -10000;
		break;
	}
	ret = ((m->ffwidth[indx]&0x3F)+1)*10000;
	break;
}	


//GET_MAPDATA_BYTE_INDEX(ffwidth, "FFCTileWidth");	//B, 32 OF THESE
case MAPDATAFFHEIGHT:	 	
{
	mapscreen *m = ri->mapsref;
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || index > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCTileHeight[]: %d\n", indx);
		ret = -10000;
		break;
	}
	ret=((m->ffheight[indx]&0x3F)+1)*10000;
	break;
	
}

//EffectWidth tmpscr->ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref]&63) | ((((value/10000)-1)&3)<<6);

//GET_MAPDATA_BYTE_INDEX(ffheight, "FFCTileHeight"	//B, 32 OF THESE
case MAPDATAFFEFFECTWIDTH: 		
{ 
	mapscreen *m = ri->mapsref;
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || index > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCEffectWidth[]: %d\n", indx);
		ret = -10000;
		break;
	}
	ret=((m->ffwidth[indx]>>6)+1)*10000;
	break;
}


//GET_MAPDATA_BYTE_INDEX(ffwidth, "FFCEffectWidth");	//B, 32 OF THESE
case MAPDATAFFEFFECTHEIGHT:
{ 
	mapscreen *m = ri->mapsref;
	int indx = ri->d[0] / 10000; 
	if ( indx < 0 || index > 31 ) 
	{
		Z_scripterrlog("Invalid FFC Index passed to MapData->FFCEffectHeight[]: %d\n", indx);
		ret = -10000;
		break;
	}
	ret=((m->ffheight[indx]>>6)+1)*10000;
	break;
}	
	
//GET_MAPDATA_BYTE_INDEX(ffheight, "FFCEffectHeight"	//B, 32 OF THESE	

case MAPDATAFFLINK: 		GET_MAPDATA_BYTE_INDEX(fflink, "FFCLink"); break;	//B, 32 OF THESE
case MAPDATAFFSCRIPT:	 	GET_MAPDATA_VAR_INDEX32(ffscript, "FFCScript"); break;	//W, 32 OF THESE
case MAPDATAINTID: 	 //Same form as SetScreenD()
	//SetFFCInitD(ffindex, d, value)
{
	mapscreen *m = ri->mapsref;
	//int ffindex = ri->d[0]/10000;
	//int d = ri->d[1]/10000;
	//int v = (value/10000);
	ret = (m->initd[(ri->d[0]/10000)][(ri->d[1]/10000)]) * 10000;
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
	ret = (m->inita[(ri->d[0]/10000)][(ri->d[1]/10000)]) * 10000;
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
case MAPDATAFFINITIALISED: 	GET_MAPDATA_BOOL_INDEX(initialized, "FFCRunning"); break;	//BOOL, 32 OF THESE
case MAPDATASCRIPTENTRY: 	GET_MAPDATA_VAR_INT32(script_entry, "ScriptEntry"); break;	//W
case MAPDATASCRIPTOCCUPANCY: 	GET_MAPDATA_VAR_INT32(script_occupancy,	"ScriptOccupancy");  break;//W
case MAPDATASCRIPTEXIT: 	GET_MAPDATA_VAR_INT32(script_exit, "ExitScript"); break;	//W
case MAPDATAOCEANSFX:	 	GET_MAPDATA_VAR_BYTE(oceansfx, "OceanSFX"); break;	//B
case MAPDATABOSSSFX: 		GET_MAPDATA_VAR_BYTE(bosssfx, "BossSFX"); break;	//B
case MAPDATASECRETSFX:	 	GET_MAPDATA_VAR_BYTE(secretsfx, "SecretSFX"); break;	//B
case MAPDATAHOLDUPSFX:	 	GET_MAPDATA_VAR_BYTE(holdupsfx,	"ItemSFX"); break; //B
case MAPDATASCREENMIDI: 	GET_MAPDATA_VAR_INT16(screen_midi, "MIDI"); break;	//SHORT, OLD QUESTS ONLY?
case MAPDATALENSLAYER:	 	GET_MAPDATA_VAR_BYTE(lens_layer, "LensLayer"); break;	//B, OLD QUESTS ONLY?
	

case MAPDATAFLAGS: 
{
	int flagid = ri->d[0])/10000;
	mapscreen *m = ri->mapsref; 
	//bool valtrue = ( value ? 10000 : 0);
	switch(flagid)
	{
		case 0: ret = (m->flags * 10000); break;
		case 1: ret = (m->flags1 * 10000); break;
		case 2: ret = (m->flags2 * 10000); break;
		case 3: ret = (m->flags3 * 10000); break;
		case 4: ret = (m->flags4 * 10000); break;
		case 5: ret = (m->flags5 * 10000); break;
		case 6: ret = (m->flags6 * 10000); break;
		case 7: ret = (m->flags7 * 10000); break;
		case 8: ret = (m->flags8 * 10000); break;
		case 9: ret = (m->flags9 * 10000); break;
		case 10: ret = (m->flags10 * 10000); break;
		default:
		{
			Z_scripterrlog("Invalid index passed to mapdata->flags[]: %d\n", flagid); 
			ret = -10000;
			break;
			
		}
	}
	break;
	//GET_MAPDATA_BYTE_INDEX	//B, 11 OF THESE, flags, flags2-flags10
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
	//GET_MAPDATA_BYTE_INDEX	//B, 11 OF THESE, flags, flags2-flags10

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
        
	    ret = -10000;
	    break;
    }
    else {
	    ret = (game->screen_d[ri->mapsref][indx]) * 10000;
	    break;
    }
}
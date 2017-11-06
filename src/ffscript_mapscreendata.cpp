case MAPDATAVALID:		SET_MAPDATA_VAR_BYTE(valid, "Valid");	//b
case MAPDATAGUY: 		SET_MAPDATA_VAR_BYTE(guy, "Guy");	//b
case MAPDATASTRING:		SET_MAPDATA_VAR_INT32(str, "String");	//w
case MAPDATAROOM: 		SET_MAPDATA_VAR_BYTE(room, "RoomType");	//b
case MAPDATAITEM: 		SET_MAPDATA_VAR_BYTE(item, "Item");	//b
case MAPDATAHASITEM: 		SET_MAPDATA_VAR_BYTE(hasitem, "HasItem");	//b
case MAPDATATILEWARPTYPE: 	SET_MAPDATA_BYTE_INDEX(tilewarptype, "TileWarpType");	//b, 4 of these
case MAPDATATILEWARPOVFLAGS: 	SET_MAPDATA_VAR_BYTE(tilewarpoverlayflags, "TileWarpOverlayFlags");	//b, tilewarpoverlayflags
case MAPDATADOORCOMBOSET: 	SET_MAPDATA_VAR_INT32(door_combo_set, "DoorComboSet");	//w
case MAPDATAWARPRETX:	 	SET_MAPDATA_BYTE_INDEX(warpreturnx, "WarpReturnX");	//b, 4 of these
case MAPDATAWARPRETY:	 	SET_MAPDATA_BYTE_INDEX(warpreturny, "WarpReturnY");	//b, 4 of these
case MAPDATAWARPRETURNC: 	SET_MAPDATA_VAR_INT32(warpreturnc, "WarpReturnC");	//w
case MAPDATASTAIRX: 		SET_MAPDATA_VAR_BYTE(stairx, "StairsX");	//b
case MAPDATASTAIRY: 		SET_MAPDATA_VAR_BYTE(stairy, "StairsY");	//b
case MAPDATAITEMX:		SET_MAPDATA_VAR_BYTE(itemx, "ItemX");		//itemx
case MAPDATAITEMY:		SET_MAPDATA_VAR_BYTE(itemy, "ItemY");		//itemy
case MAPDATACOLOUR: 		SET_MAPDATA_VAR_INT32(color, "CSet");	//w
case MAPDATAENEMYFLAGS: 	SET_MAPDATA_VAR_BYTE(enemyflags, "EnemyFlags");	//b
case MAPDATADOOR: 		SET_MAPDATA_BYTE_INDEX(door, "Door");	//b, 4 of these
case MAPDATATILEWARPDMAP: 	SET_MAPDATA_VAR_INDEX32(tilewarpdmap, "TileWarpDMap");	//w, 4 of these
case MAPDATATILEWARPSCREEN: 	SET_MAPDATA_BYTE_INDEX(tilewarpscr, "TileWarpScreen");	//b, 4 of these
case MAPDATAEXITDIR: 		SET_MAPDATA_VAR_BYTE(exitdir, "ExitDir");	//b
case MAPDATAENEMY: 		SET_MAPDATA_VAR_INDEX32(enemy, "Enemy");	//w, 10 of these
case MAPDATAPATTERN: 		SET_MAPDATA_VAR_BYTE(pattern, "Pattern");	//b
case MAPDATASIDEWARPTYPE: 	SET_MAPDATA_BYTE_INDEX(sidewarptype, "SideWarpType");	//b, 4 of these
case MAPDATASIDEWARPOVFLAGS: 	SET_MAPDATA_VAR_BYTE(sidewarpoverlayflags, "SideWarpOverlayFlags");	//b
case MAPDATAWARPARRIVALX: 	SET_MAPDATA_VAR_BYTE(warparrivalx, "WarpArrivalX");	//b
case MAPDATAWARPARRIVALY: 	SET_MAPDATA_VAR_BYTE(warparrivaly, "WarpArrivalY");	//b
case MAPDATAPATH: 		SET_MAPDATA_BYTE_INDEX(path, "MazePath");	//b, 4 of these
case MAPDATASIDEWARPSC: 	SET_MAPDATA_BYTE_INDEX(sidewarpscr, "SideWarpScreen");	//b, 4 of these
case MAPDATASIDEWARPDMAP: 	SET_MAPDATA_VAR_INDEX32(sidewarpdmap, "SideWarpDMap");	//w, 4 of these
case MAPDATASIDEWARPINDEX: 	SET_MAPDATA_VAR_BYTE(sidewarpindex, "SideWarpIndex");	//b
case MAPDATAUNDERCOMBO: 	SET_MAPDATA_VAR_INT32(undercombo, "Undercombo");	//w
case MAPDATAUNDERCSET:	 	SET_MAPDATA_VAR_BYTE(undercset,	"UnderCSet");//b
case MAPDATACATCHALL:	 	SET_MAPDATA_VAR_INT32(catchall,	"Catchall");//W

case MAPDATACSENSITIVE: 	SET_MAPDATA_VAR_BYTE(csensitive, "CSensitive");	//B
case MAPDATANORESET: 		SET_MAPDATA_VAR_INT32(noreset, "NoReset");	//W
case MAPDATANOCARRY: 		SET_MAPDATA_VAR_INT32(nocarry, "NoCarry");	//W
case MAPDATALAYERMAP:	 	SET_MAPDATA_BYTE_INDEX(layermap, "LayerMap");	//B, 6 OF THESE
case MAPDATALAYERSCREEN: 	SET_MAPDATA_BYTE_INDEX(layerscreen, "LayerScreen");	//B, 6 OF THESE
case MAPDATALAYEROPACITY: 	SET_MAPDATA_BYTE_INDEX(layeropacity, "LayerOpacity");	//B, 6 OF THESE
case MAPDATATIMEDWARPTICS: 	SET_MAPDATA_VAR_INT32(timedwarptics, "TimedWarpTimer");	//W
case MAPDATANEXTMAP: 		SET_MAPDATA_VAR_BYTE(nextmap, "NextMap");	//B
case MAPDATANEXTSCREEN: 	SET_MAPDATA_VAR_BYTE(nextscr, "NextScreen");	//B
case MAPDATASECRETCOMBO: 	SET_MAPDATA_VAR_INDEX32(secretcombo, "SecretCombo");	//W, 128 OF THESE
case MAPDATASECRETCSET: 	SET_MAPDATA_BYTE_INDEX(secretcset, "SecretCSet");	//B, 128 OF THESE
case MAPDATASECRETFLAG: 	SET_MAPDATA_BYTE_INDEX(secretflag, "SecretFlags");	//B, 128 OF THESE
case MAPDATAVIEWX: 		SET_MAPDATA_VAR_INT32(viewX, "ViewX");	//W
case MAPDATAVIEWY: 		SET_MAPDATA_VAR_INT32(viewY, "ViewY"); //W
case MAPDATASCREENWIDTH: 	SET_MAPDATA_VAR_BYTE(scrWidth, "Width");	//B
case MAPDATASCREENHEIGHT: 	SET_MAPDATA_VAR_BYTE(scrHeight,	"Height");	//B
case MAPDATAENTRYX: 		SET_MAPDATA_VAR_BYTE(entry_x, "EntryX");	//B
case MAPDATAENTRYY: 		SET_MAPDATA_VAR_BYTE(entry_y, "EntryY");	//B
case MAPDATANUMFF: 		SET_MAPDATA_VAR_INT16(numff, "NumFFCs");	//INT16
case MAPDATAFFDATA: 		SET_MAPDATA_VAR_INDEX32(ffdata,	"FFCData");	//W, 32 OF THESE
case MAPDATAFFCSET: 		SET_MAPDATA_BYTE_INDEX(ffcset,	"FFCCSet");	//B, 32
case MAPDATAFFDELAY: 		SET_MAPDATA_VAR_INDEX32(ffdelay, "FFCDelay");	//W, 32
case MAPDATAFFX: 		SET_MAPDATA_VAR_INDEX32(ffx, "FFCX");	//INT32, 32 OF THESE
case MAPDATAFFY: 		SET_MAPDATA_VAR_INDEX32(ffy, "FFCY");	//..
case MAPDATAFFXDELTA:	 	SET_MAPDATA_VAR_INDEX32(ffxdelta, "FFCVx");	//..
case MAPDATAFFYDELTA: 		SET_MAPDATA_VAR_INDEX32(ffydelta, "FFCVy");	//..
case MAPDATAFFXDELTA2:	 	SET_MAPDATA_VAR_INDEX32(ffxdelta2, "FFCAx");	//..
case MAPDATAFFYDELTA2:	 	SET_MAPDATA_VAR_INDEX32(ffydelta2, "FFCAy");	//..
case MAPDATAFFFLAGS: 		SET_MAPDATA_VAR_INDEX16(ffflags, "FFCFlags");	//INT16, 23 OF THESE
//Height and With are Or'd together, and need to be separate:
/*
 //Width ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref] & ~63) | (((value/10000)-1)&63);
 //EffectWidth tmpscr->ffwidth[ri->ffcref]= (tmpscr->ffwidth[ri->ffcref]&63) | ((((value/10000)-1)&3)<<6);

*/
case MAPDATAFFWIDTH: 		SET_MAPDATA_BYTE_INDEX(ffwidth, "FFCTileWidth");	//B, 32 OF THESE
case MAPDATAFFHEIGHT:	 	SET_MAPDATA_BYTE_INDEX(ffheight, "FFCTileHeight"	//B, 32 OF THESE
case MAPDATAFFEFFECTWIDTH: 		SET_MAPDATA_BYTE_INDEX(ffwidth, "FFCEffectWidth");	//B, 32 OF THESE
case MAPDATAFFEFFECTHEIGHT:	 	SET_MAPDATA_BYTE_INDEX(ffheight, "FFCEffectHeight"	//B, 32 OF THESE	

case MAPDATAFFLINK: 		SET_MAPDATA_BYTE_INDEX(fflink, "FFCLink");	//B, 32 OF THESE
case MAPDATAFFSCRIPT:	 	SET_MAPDATA_VAR_INDEX32(ffscript, "FFCScript");	//W, 32 OF THESE
//case MAPDATAINTID: 		initd	//INT32 , 32 OF THESE, EACH WITH 10 INDICES. 
//case MAPDATAINITA: 		inita	//INT32, 32 OF THESE, EACH WITH 2
case MAPDATAFFINITIALISED: 	SET_MAPDATA_BOOL_INDEX(initialized, "FFCRunning");	//BOOL, 32 OF THESE
case MAPDATASCRIPTENTRY: 	SET_MAPDATA_VAR_INT32(script_entry, "ScriptEntry");	//W
case MAPDATASCRIPTOCCUPANCY: 	SET_MAPDATA_VAR_INT32(script_occupancy,	"ScriptOccupancy"); //W
case MAPDATASCRIPTEXIT: 	SET_MAPDATA_VAR_INT32(script_exit, "ExitScript");	//W
case MAPDATAOCEANSFX:	 	SET_MAPDATA_VAR_BYTE(oceansfx, "OceanSFX");	//B
case MAPDATABOSSSFX: 		SET_MAPDATA_VAR_BYTE(bosssfx, "BossSFX");	//B
case MAPDATASECRETSFX:	 	SET_MAPDATA_VAR_BYTE(secretsfx, "SecretSFX");	//B
case MAPDATAHOLDUPSFX:	 	SET_MAPDATA_VAR_BYTE(holdupsfx,	"ItemSFX");//B
case MAPDATASCREENMIDI: 	SET_MAPDATA_VAR_INT16(screen_midi, "MIDI");	//SHORT, OLD QUESTS ONLY?
case MAPDATALENSLAYER:	 	SET_MAPDATA_VAR_BYTE(lens_layer, "LensLayer");	//B, OLD QUESTS ONLY?
	

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
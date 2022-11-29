import "std.zh"

//int vol[4];

global script a{
	void run(){

		//npcdata nd;
		itemdata id = Game->LoadItemData(13);
		TraceS("Pickup string for wooden arrows: ");
		Trace(id->PString);
		int walkingtile[2] = { 11720, 11721 };
		Link->ScriptTile = walkingtile[0];
		int frame = -1;

		npcdata nd = Game->LoadNPCData(1); //Loadnpcdata is broken?
//invalid index 2 passed to a local array size of 2 invalid zasm command 578 reached
		messagedata md = Game->LoadMessageData(1);
//messagedata is also broken, invalid pointer 0 pased to array
//! This must be something screwy in the parser?!
		dmapdata dmd = Game->LoadDMapData(0);
		TraceS("messagedata->Font: "); Trace(md->Font);
		TraceS("dmapdata->Level: "); Trace(dmd->Level);


//		TraceS("MIDI Volume: "); Trace(Audio->Volume[VOL_MIDI]);
//		TraceS("DIGI Volume: "); Trace(Audio->Volume[VOL_DIGI]);
//		TraceS("Music Volume: "); Trace(Audio->Volume[VOL_MUSIC]);
//		TraceS("SFX Volume: "); Trace(Audio->Volume[VOL_SFX]);
//		TraceS("PanStyle: "); Trace(Audio->PanStyle);
		while(true){
			if ( ++frame >= 12 ) frame = 0;

			if ( Link->InputLeft )
			{
				if (frame < 6) Link->ScriptTile = walkingtile[1];
				else Link->ScriptTile = walkingtile[0];
			}
			if ( Link->InputRight )
			{
				if (frame < 6) Link->ScriptTile = walkingtile[1];
				else Link->ScriptTile = walkingtile[0];
			}
			if ( Link->InputUp )
			{
				if (frame < 6) Link->ScriptTile = walkingtile[1];
				else Link->ScriptTile = walkingtile[0];
			}
			if ( Link->InputDown )
			{
				if (frame < 6) Link->ScriptTile = walkingtile[1];
				else Link->ScriptTile = walkingtile[0];
			}
			if ( Link->PressEx3 ) { Link->ScriptTile = Clamp((Link->ScriptTile-1), 0, MAX_TILES); TraceS("ScriptTile: "); Trace(Link->ScriptTile); }
			if ( Link->PressEx4 ) { Link->ScriptTile = Clamp((Link->ScriptTile+1), 0, MAX_TILES); TraceS("ScriptTile: "); Trace(Link->ScriptTile); }

			if ( Link->PressEx1 ) { Link->ScriptFlip = Clamp((Link->ScriptFlip-1), -1, 7); TraceS("ScriptFlip: "); Trace(Link->ScriptFlip); }
			if ( Link->PressEx2 ) { Link->ScriptFlip = Clamp((Link->ScriptFlip+1), -1, 7); TraceS("ScriptFlip: "); Trace(Link->ScriptFlip); }
			switch(Link->Dir)
			{
				case DIR_UP:
					Link->ScriptFlip = FLIP_NONE;
					break;

				case DIR_DOWN:
					Link->ScriptFlip = FLIP_VERTICAL;
					break;
				case DIR_LEFT:
					Link->ScriptFlip = ROT_ACW;
					break;
				case DIR_RIGHT:
					Link->ScriptFlip = ROT_CW;
					break;

				default: break;
			}
/*
			if ( Input->Key[KEY_F] ){
				lweapon l = Screen->CreateLWeapon(LW_BOMBBLAST);
				l->X = Link->X + 16; l->Y = Link->Y +16;
				l->Parent = 36; TraceS("l->Parent: "); Trace(l->Parent);
				l->UseSprite(18);

			}
			if ( Input->Key[KEY_G] ) { Audio->Volume[VOL_MIDI] = Clamp((Audio->Volume[VOL_MIDI]-1), 0, 255);

TraceS("MIDI Volume: "); Trace(Audio->Volume[VOL_MIDI]); }

			if ( Input->Key[KEY_H] ) {
Audio->Volume[VOL_MIDI] = Clamp((Audio->Volume[VOL_MIDI]+1), 0, 255);

TraceS("MIDI Volume: "); Trace(Audio->Volume[VOL_MIDI]); }

			if ( Input->Key[KEY_J] ) { Audio->Volume[VOL_SFX] = Clamp((Audio->Volume[VOL_SFX]-1), 0, 255);
TraceS("SFX Volume: ");
Trace(Audio->Volume[VOL_SFX]); }
			if ( Input->Key[KEY_K] ) {  Audio->Volume[VOL_SFX] = Clamp((Audio->Volume[VOL_SFX]+1), 0, 255);
TraceS("SFX Volume: "); Trace(Audio->Volume[VOL_SFX]); }


			if ( Input->Key[KEY_Y] ) { Audio->Volume[VOL_MUSIC] = Clamp((Audio->Volume[VOL_MUSIC]-1), 0, 255);
TraceS("Enhanced Music Volume: ");
Trace(Audio->Volume[VOL_MUSIC]); }
			if ( Input->Key[KEY_U] ) {  Audio->Volume[VOL_MUSIC] = Clamp((Audio->Volume[VOL_MUSIC]+1), 0, 255);
TraceS("Enhanced Music Volume: "); Trace(Audio->Volume[VOL_MUSIC]); }

			if ( Input->Key[KEY_I] ) { Audio->Volume[VOL_DIGI] = Clamp((Audio->Volume[VOL_DIGI]-1), 0, 255);
TraceS("Enhanced DIGI Volume: ");
Trace(Audio->Volume[VOL_DIGI]); }
			if ( Input->Key[KEY_O] ) {  Audio->Volume[VOL_DIGI] = Clamp((Audio->Volume[VOL_DIGI]+1), 0, 255);
TraceS("Enhanced DIGI Volume: "); Trace(Audio->Volume[VOL_DIGI]); }

			if ( Input->Key[KEY_N] ) { Audio->PanStyle = Clamp((Audio->PanStyle-1), 0, 3);
TraceS("Pan Style: ");
Trace(Audio->PanStyle); }
			if ( Input->Key[KEY_M] ) {  Audio->PanStyle = Clamp((Audio->PanStyle+1), 0, 3);
TraceS("Pan Style: "); Trace(Audio->PanStyle); }

*/

			Waitdraw(); Waitframe();
		}
	}
}

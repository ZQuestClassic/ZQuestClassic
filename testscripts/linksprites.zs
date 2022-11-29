import "std.zh"


int tileframe;

//Base tiles for use in stacking equipment. These will be copied to the drawn tiles, and the equipment overlaid.
const int BASE_LINKTILE_WALK_UP		=
const int BASE_LINKTILE_WALK_UP_B	=
const int BASE_LINKTILE_WALK_DOWN	=
const int BASE_LINKTILE_WALK_DOWN_B	=
const int BASE_LINKTILE_WALK_LEFT	=
const int BASE_LINKTILE_WALK_LEFT_B	=
const int BASE_LINKTILE_WALK_RIGHT	=
const int BASE_LINKTILE_WALK_UP_RIGHT	=
const int BASE_LINKTILE_ATTACK_UP	=
const int BASE_LINKTILE_ATTACK_DOWN	=
const int BASE_LINKTILE_ATTACK_LEFT	=
const int BASE_LINKTILE_ATTACK_RIGHT	=
const int BASE_LINKTILE_SWIM_UP		=
const int BASE_LINKTILE_SWIM_UP_B	=
const int BASE_LINKTILE_SWIM_DOWN	=
const int BASE_LINKTILE_SWIM_DOWN_B	=
const int BASE_LINKTILE_SWIM_LEFT	=
const int BASE_LINKTILE_SWIM_LEFT_B	=
const int BASE_LINKTILE_SWIM_RIGHT	=
const int BASE_LINKTILE_SWIM_UP_RIGHT	=

const int BASE_LINKTILE_DIVE_UP		=
const int BASE_LINKTILE_DIVE_UP_B	=
const int BASE_LINKTILE_DIVE_DOWN	=
const int BASE_LINKTILE_DIVE_DOWN_B	=
const int BASE_LINKTILE_DIVE_LEFT	=
const int BASE_LINKTILE_DIVE_LEFT_B	=
const int BASE_LINKTILE_DIVE_RIGHT	=
const int BASE_LINKTILE_DIVE_UP_RIGHT	=

const int BASE_LINKTILE_HOLD1LAND	=
const int BASE_LINKTILE_HOLD2LAND	=
const int BASE_LINKTILE_HOLD1WATER	=
const int BASE_LINKTILE_HOLD2WATER	=
const int BASE_LINKTILE_CASTFRAME_1	=
const int BASE_LINKTILE_CASTFRAME_2	=
const int BASE_LINKTILE_CASTFRAME_3	=
const int BASE_LINKTILE_CASTFRAME_4	=

void OverlayEquipment()
{

	//Armour and clothing
	//Capes if facing up
	//Shield Tiles if not facing up
}



//These tiles will be drawn
const int LINKTILE_WALK_UP		=
const int LINKTILE_WALK_UP_B		=
const int LINKTILE_WALK_DOWN		=
const int LINKTILE_WALK_DOWN_B		=
const int LINKTILE_WALK_LEFT		=
const int LINKTILE_WALK_LEFT_B		=
const int LINKTILE_WALK_RIGHT		=
const int LINKTILE_WALK_UP_RIGHT	=
const int LINKTILE_ATTACK_UP		=
const int LINKTILE_ATTACK_DOWN		=
const int LINKTILE_ATTACK_LEFT		=
const int LINKTILE_ATTACK_RIGHT		=
const int LINKTILE_SWIM_UP		=
const int LINKTILE_SWIM_UP_B		=
const int LINKTILE_SWIM_DOWN		=
const int LINKTILE_SWIM_DOWN_B		=
const int LINKTILE_SWIM_LEFT		=
const int LINKTILE_SWIM_LEFT_B		=
const int LINKTILE_SWIM_RIGHT		=
const int LINKTILE_SWIM_UP_RIGHT	=

const int LINKTILE_DIVE_UP		=
const int LINKTILE_DIVE_UP_B		=
const int LINKTILE_DIVE_DOWN		=
const int LINKTILE_DIVE_DOWN_B		=
const int LINKTILE_DIVE_LEFT		=
const int LINKTILE_DIVE_LEFT_B		=
const int LINKTILE_DIVE_RIGHT		=
const int LINKTILE_DIVE_UP_RIGHT	=

const int LINKTILE_HOLD1LAND		=
const int LINKTILE_HOLD2LAND		=
const int LINKTILE_HOLD1WATER		=
const int LINKTILE_HOLD2WATER		=
const int LINKTILE_CASTFRAME_1		=
const int LINKTILE_CASTFRAME_2		=
const int LINKTILE_CASTFRAME_3		=
const int LINKTILE_CASTFRAME_4		=

//Runs the timer and resets it between actions.
void TileFrame()
{
	//No!
	//if ( ++tileframe >= 12 ) tileframe = 0;
	switch(Link->Action)
	case LA_WALKING:
	{
		if ( ++tileframe >= 12 ) tileframe = 0;
	}
	//What about swimming?
	case LA_SWIMMING: //LA_SWIMMING is false when standing still, IIRC
	{
		( ++tileframe >= 12 ) tileframe = 0;
	}
	default: tileframe = 0;
}

//const int FLIP_LEFT_RIGHT_LINKTILE = 0;

void SetLinkSprite()
{
	int walktiles[] = {
		//dir tile 1, tile 2
		LINKTILE_WALK_UP, LINKTILE_WALK_UP_B, //Up
		LINKTILE_WALK_DOWN, LINKTILE_WALK_DOWN_B, //down
		LINKTILE_WALK_LEFT, LINKTILE_WALK_LEFT_B, //left
		LINKTILE_WALK_RIGHT, LINKTILE_WALK_UP_RIGHT //right
	};
	int attacktiles[] = {
		//dir tile 1, tile 2
		LINKTILE_ATTACK_UP,
		LINKTILE_ATTACK_DOWN,
		LINKTILE_ATTACK_LEFT,
		LINKTILE_ATTACK_RIGHT,
	};
	int swimtiles[] = {
		//dir tile 1, tile 2
		LINKTILE_SWIM_UP, LINKTILE_SWIM_UP_B, //Up
		LINKTILE_SWIM_DOWN, LINKTILE_SWIM_DOWN_B, //down
		LINKTILE_SWIM_LEFT, LINKTILE_SWIM_LEFT_B, //left
		LINKTILE_SWIM_RIGHT, LINKTILE_SWIM_UP_RIGHT //right
	};
	int diveiles[] = {
		//dir tile 1, tile 2
		LINKTILE_DIVE_UP, LINKTILE_DIVE_UP_B, //Up
		LINKTILE_DIVE_DOWN, LINKTILE_DIVE_DOWN_B, //down
		LINKTILE_DIVE_LEFT, LINKTILE_DIVE_LEFT_B, //left
		LINKTILE_DIVE_RIGHT, LINKTILE_DIVE_UP_RIGHT //right
	};
	int holdtiles[] = {LINKTILE_HOLD1LAND, LINKTILE_HOLD2LAND, LINKTILE_HOLD1WATER, LINKTILE_HOLD2WATER };
	int castingtiles[] = {LINKTILE_CASTFRAME_1, LINKTILE_CASTFRAME_2, LINKTILE_CASTFRAME_3, LINKTILE_CASTFRAME_4 };

	//int spr;
	int tile = 0; int flip = 0;
	switch(Link->Action)
	{
		case LA_NONE:
		{
			tile = walktiles[(Link->Dir *2)]; flip = 0;
			break;
		}
		case LA_WALKING:
		{
			tile = walktiles[(Link->Dir *2) + Cond( (tileframe < 6), 0, 1 ) ]; flip = 0;
			break;
		}
		case LA_ATTACKING:
		{
			tile = attacktiles[Link->Dir]; flip = 0;
			break;
		}
		case LA_CHARGING:
		{
			//Are ther emultiple frames to this?
			tile = attacktiles[Link->Dir]; flip = 0;
			break;
		}
		case LA_SPINNING:
		{
			tile = attacktiles[Link->Dir]; flip = 0;
			break;
			/*
			switch(Link->Dir)
			{

				case DIR_UP:
				{
					tile = attacktiles[DIR_UP];
					flip = 0;
					break;
				}
				case DIR_DOWN:
				{
					tile = attacktiles[DIR_DOWN];
					flip = 0;
					break;
				}
				case DIR_LEFT:
				{
					tile = attacktiles[DIR_LEFT];
					flip = 0;
					break;
				}
				case DIR_RIGHT:
				{
					tile = attacktiles[DIR_LEFT];
					flip = 1;
					break;
				}

			}
			break;
			*/
		}
		case LA_SWIMMING:
		{
			tile = swimtiles[(Link->Dir *2) + Cond( (tileframe < 6), 0, 1 ) ]; flip = 0;
			break;
		}
		case LA_DIVING:
		{
			tile = divetiles[(Link->Dir *2) + Cond( (tileframe < 6), 0, 1 ) ]; flip = 0;
			break;
		}
		case LA_DROWNING:
		{
			tile = divetiles[(Link->Dir *2) + Cond( (tileframe < 6), 0, 1 ) ]; flip = 0;
			break;
		}
		case LA_HOLD1LAND:
			tile = holdtiles[0]; break; flip = 0;
		case LA_HOLD2LAND:
			tile = holdtiles[1]; break; flip = 0;
		case LA_HOLD1WATER:
			tile = holdtiles[3]; break; flip = 0;
		case LA_HOLD2WATER:
			tile = holdtiles[4]; break; flip = 0;
		case LA_RAFTING:
			tile = walktiles[(Link->Dir *2)]; break; flip = 0;
		case LA_GOTHURTLAND:
			tile = walktiles[(Link->Dir *2)]; break; flip = 0;
		case LA_GOTHURTWATER:
			tile = swimtiles[(Link->Dir *2)]; flip = 0;
		case LA_INWIND:
			tile = walktiles[(Link->Dir *2)]; break; flip = 0;
		case LA_SCROLLING:
			tile = walktiles[(Link->Dir *2)]; break; flip = 0;
		case LA_WINNING:
			tile = holdtiles[1]; break; flip = 0;
		case LA_HOPPING:
		case LA_CAVEWALKUP:
			tile = walktiles[DIR_UP + Cond( (tileframe < 6), 0, 1 ) ]; break; flip = 0;
		case LA_CAVEWALKDOWN:
			tile = walktiles[(DIR_DOWN*2) + Cond( (tileframe < 6), 0, 1 ) ]; break; flip = 0;

		case LA_DYING:
		{
			switch(tileframe)
			{
				case 0:
				case 1:
				case 2:
				{
					tile = walktiles[0]; flip = 0; break;
				}
				case 3:
				case 4:
				case 5:
				{
					tile = walktiles[2]; flip = 0; break;
				}
				case 6:
				case 7:
				case 8:
				{
					tile = walktiles[4]; flip = 0; break;
				}
				case 9:
				case 10:
				case 11:
				{
					tile = walktiles[6]; flip = 0; break;
				}
			}
			break;
		}
		case LA_CASTING:
		{
			switch(tileframe)
			{
				case 0:
				case 1:
				case 2:
				{
					tile = castingtiles[0]; flip = 0; break;
				}
				case 3:
				case 4:
				case 5:
				{
					tile = castingtiles[1]; flip = 0; break;
				}
				case 6:
				case 7:
				case 8:
				{
					tile = castingtiles[2]; flip = 0; break;
				}
				case 9:
				case 10:
				case 11:
				{
					tile = castingtiles[3]; flip = 0; break;
				}
			}
			break;
		}
		case LA_FROZEN:
		{
			tile = walktiles[(Link->Dir *2)]; flip = 0;
		}
		default:
			tile = walktiles[(Link->Dir *2)]; break; flip = 0;
	}
	Link->ScriptTile = tile;
	Link->ScriptFlip = flip;
}

global script a{
	void run(){
		int walkingtile[2] = { 11720, 11721 };
		Link->ScriptTile = walkingtile[0];
		int frame = -1;

		//Deprecated
		//TraceS("MIDI Volume: "); Trace(Audio->Volume[VOL_MIDI]);
		//TraceS("DIGI Volume: "); Trace(Audio->Volume[VOL_DIGI]);
		//TraceS("Music Volume: "); Trace(Audio->Volume[VOL_MUSIC]);
		//TraceS("SFX Volume: "); Trace(Audio->Volume[VOL_SFX]);
		TraceS("PanStyle: "); Trace(Audio->PanStyle);
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
			if ( Input->Key[KEY_F] ){
				lweapon l = Screen->CreateLWeapon(LW_BOMBBLAST);
				l->X = Link->X + 16; l->Y = Link->Y +16;
				l->Parent = 36; TraceS("l->Parent: "); Trace(l->Parent);
				l->UseSprite(18);

			}
			/*
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

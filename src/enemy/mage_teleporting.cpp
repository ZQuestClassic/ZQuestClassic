#include "mage_teleporting.h"
#include "../link.h"
#include <algorithm>

MageTeleporting::MageTeleporting(enemy const & other, bool newScriptUID, bool clearParentUID):
	enemy(other),
	currAnimState(animState::NORMAL)
{
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));

		scrmem->scriptData = other.scrmem->scriptData;
	}
	else
		scrmem = nullptr;

	for(int32_t i = 0; i < edefLAST255; ++i)
		defense[i]=other.defense[i];
	for(int32_t q = 0; q < 10; ++q)
		frozenmisc[q] = other.frozenmisc[q];
	for(int32_t q = 0; q < NUM_HIT_TYPES_USED; ++q)
		hitby[q] = other.hitby[q];

	if(newScriptUID)
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs.
	if(clearParentUID)
		parent_script_UID = 0;
	for(int32_t q = 0; q < 32; ++q)
		movement[q] = other.movement[q];
	for(int32_t q = 0; q < 32; ++q)
		new_weapon[q] = other.new_weapon[q];

	for(int32_t q = 0; q < 8; ++q)
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for(int32_t q = 0; q < 2; ++q)
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

MageTeleporting::MageTeleporting(zfix x, zfix y, int32_t id, int32_t clk):
	enemy(x, y, id, clk),
	currAnimState(animState::NORMAL)
{
	hxofs = 1000;
	fading = fade_invisible;
	// Set clk to just before the 'reappear' threshold
	clk = zc_min(
		clk+(146+zc_max(0, dmisc5))+14,
		(146+zc_max(0, dmisc5))-1);
	currAnimState = animState::NORMAL;
	frate = 1200+146; //1200 = 20 seconds

	SIZEflags = d->SIZEflags;
	if(SIZEflags&guyflagOVERRIDE_TILE_WIDTH && txsz > 0 )
	{
		txsz = d->txsz;
		if(txsz > 1)
			extend = 3;
	}

	if(SIZEflags&guyflagOVERRIDE_TILE_HEIGHT && tysz > 0)
	{
		tysz = d->tysz;
		if(tysz > 1)
			extend = 3;
	}
	if((SIZEflags & guyflagOVERRIDE_HIT_WIDTH) && hxsz >= 0)
		hxsz = d->hxsz;
	if((SIZEflags & guyflagOVERRIDE_HIT_HEIGHT) && hysz >= 0)
		hysz = d->hysz;
	if((SIZEflags & guyflagOVERRIDE_HIT_Z_HEIGHT) && hzsz >= 0)
		hzsz = d->hzsz;
	if(SIZEflags & guyflagOVERRIDE_HIT_X_OFFSET)
		hxofs = d->hxofs;
	if(SIZEflags & guyflagOVERRIDE_HIT_Y_OFFSET)
		hyofs = d->hyofs;
	if(SIZEflags & guyflagOVERRIDE_DRAW_X_OFFSET)
		xofs = d->xofs;
	if(SIZEflags & guyflagOVERRIDE_DRAW_Y_OFFSET)
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += 56 ; //this offset fixes yofs not plaing properly. -Z
	}

	if(SIZEflags & guyflagOVERRIDE_DRAW_Z_OFFSET)
		zofs = d->zofs;
}

bool MageTeleporting::animate(int32_t index)
{
	if(fallclk || drownclk)
		return enemy::animate(index);
	if(dying)
		return Dead(index);
	if(clk == 0)
		removearmos(x,y);

	if(watch)
	{
		fading = 0;
		hxofs = 0;
		return enemy::animate(index);
	}

	switch(clk)
	{
	case 0: // Teleport and start appearing
		if(!tryTeleport())
			// Couldn't find anywhere to appear? Just die.
			return true;
		faceHero();
		fading = fade_flicker;
		hxofs = 0;
		break;

	case 64: // Stop flickering, switch to charging animation
		fading = 0;
		currAnimState = animState::CHARGING;
		break;

	case 73: // Switch to firing animation frame, but don't fire quite yet
		currAnimState = animState::FIRING;
		break;

	case 83: // NOW fire
		eWpn.fire();
		break;

	case 119: // Back to charging animation
		currAnimState = animState::CHARGING;
		break;

	case 128: // Start flickering
		fading = fade_flicker;
		currAnimState = animState::NORMAL;
		break;

	case 146: // Disappear
		fading = fade_invisible;
		hxofs = 1000;
		[[fallthrough]];

	default: // And stay gone until clk rolls over
		if(clk >= (146+zc_max(0, dmisc5)))
			clk = -1;

	break;
	}

	return enemy::animate(index);
}

bool MageTeleporting::tryTeleport()
{
	if(dmisc2 == 0) // Single shot
	{
		// Mages' Misc4 controls whether mages can teleport on top of
		// solid combos, but should not appear on dungeon walls.
		// 1.84, and probably 1.90 mages should NEVER appear
		// in dungeon walls.-Z (1.84 confirmed, 15th January, 2019 by
		// Chris Miller).
		if(FFCore.getQuestHeaderInfo(vZelda) <= 0x190)
		{
			teleportAligned(false);
			return true;
		}
		else if(id == eWWIZ &&
			(FFCore.getQuestHeaderInfo(vZelda) == 0x210 || FFCore.getQuestHeaderInfo(vZelda) == 0x192) &&
			FFCore.emulation[emu210WINDROBES])
		{
			//2.10 Windrobe
			//randomise location and face the hero
			return teleportRandomly();
		}
		else
		{
			teleportAligned(dmisc4 != 0);
			return true;
		}
	}
	else
		return teleportRandomly();
}

bool MageTeleporting::teleportRandomly()
{
	for(int8_t i = 0; i < 160; ++i)
	{
		if(isdungeon())
		{
			x = (rand()%12+2)*16;
			y = (rand()%7+2)*16;
		}
		else
		{
			x = (rand()%14+1)*16;
			y = (rand()%9+1)*16;
		}

		if(!m_walkflag(x, y, spw_door, dir) && (abs(x-Link.getX()) >= 32 || abs(y-Link.getY()) >= 32))
			return true;
	}

	return false;
}

void MageTeleporting::teleportAligned(bool solidOK)
{
	int32_t hx = std::clamp(((int32_t)Link.getX())&0xF0, 32, 208);
	int32_t hy = std::clamp(((int32_t)Link.getY())&0xF0, 32, 128);
	int32_t checkPos = rand()%23;
	bool placed = false;

	// Each of the tiles aligned with the hero and not at the edge of the screen
	// are checked - that's 9 vertically and 14 horizontally. 23 positions,
	// with one duplicated. Cycle through them and take the first one that's
	// far enough from the hero and the mage can stand on.
	int32_t numTries;
	for(numTries = 0; true; ++numTries)
	{
		if(checkPos<14)
		{
			x = (checkPos<<4)+16;
			y = hy;
		}
		else
		{
			x = hx;
			y = ((checkPos-14)<<4)+16;
		}

		// If the position's at least a tile away from the hero and the mage
		// can stand there, take it.
		if(abs(hx-x) > 16 || abs(hy-y) > 16)
		{
			// Red mages should be able to appear on water, but not other
			// solid combos; however, they could appear on solid combos in 2.10,
			// and some quests depend on that.
			if((solidOK || !m_walkflag(x, y,  spw_water, dir)) && !flyerblocked(x, y, spw_floater))
			break;
		}

		// Too many tries? Give up and take the first one that's not
		// at the edge of the screen.
		if(numTries >= 22 && !(x < 32 || y < 32 || x >= 224 || y >= 144))
			break;

		checkPos = (checkPos+3)%23;
	}

	// XXX Unused?
	clk2 = numTries;
}

void MageTeleporting::faceHero()
{
	if(abs(x-Link.getX()) < abs(y-Link.getY()))
	{
		if(y < Link.getY())
			dir = down;
		else
			dir = up;
	}
	else
	{
		if(x < Link.getX())
			dir = right;
		else
			dir = left;
	}
}

void MageTeleporting::draw(BITMAP *dest)
{
	switch(currAnimState)
	{
	case animState::NORMAL:
		dummy_bool[1] = false;
		dummy_bool[2] = false;
		break;

	case animState::CHARGING:
		dummy_bool[1] = true;
		dummy_bool[2] = false;
		break;

	case animState::FIRING:
		dummy_bool[1] = false;
		dummy_bool[2] = true;
		break;
	}
	update_enemy_frame();
	// XXX Does not setting dummy_bool back to its previous values right here
	// cause any problems? If so, it's not immediately obvious.
	enemy::draw(dest);
}

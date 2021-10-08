#include "mage_floating.h"
#include "../ffscript.h"
#include "../zelda.h"

MageFloating::MageFloating(const enemy& other, bool newScriptUID, bool clearParentUID):
	enemy(other),
    currAction(*((action*)&misc)),
    actionTimer(clk3),
    shotTimer(0)
{
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(long));

		scrmem->scriptData = other.scrmem->scriptData;
	}
	else
		scrmem = nullptr;

	for(int i = 0; i < edefLAST255; ++i)
		defense[i] = other.defense[i];
	for(int q = 0; q < 10; ++q)
		frozenmisc[q] = other.frozenmisc[q];
	for(int q = 0; q < NUM_HIT_TYPES_USED; ++q)
		hitby[q] = other.hitby[q];

	if(newScriptUID)
		script_UID = FFCore.GetScriptObjectUID(UID_TYPE_NPC); //This is used by child npcs.
	if(clearParentUID)
		parent_script_UID = 0;

	for(int q = 0; q < 32; ++q)
		movement[q] = other.movement[q];
	for(int q = 0; q < 32; ++q)
		new_weapon[q] = other.new_weapon[q];

	for(int q = 0; q < 8; ++q)
	{
		initD[q] = other.initD[q];
		weap_initiald[q] = other.weap_initiald[q];
	}
	for(int q = 0; q < 2; ++q)
	{
		initA[q] = other.initA[q];
		weap_initiala[q] = other.weap_initiala[q];
	}
}

MageFloating::MageFloating(zfix x, zfix y, int id, int clk):
	enemy(x, y, id, clk),
	currAction(*((action*)&misc)),
	actionTimer(clk3),
	shotTimer(0)
{
	dir = (loadside == right) ? right : left;
	currAction = action::INIT;

	SIZEflags = d->SIZEflags;
	if((SIZEflags & guyflagOVERRIDE_TILE_WIDTH) && txsz > 0 )
	{
		txsz = d->txsz;
		if(txsz > 1)
			extend = 3;
	}

	if((SIZEflags & guyflagOVERRIDE_TILE_HEIGHT) && tysz > 0)
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

bool MageFloating::animate(int index)
{
	if(fallclk || drownclk)
		return enemy::animate(index);
	if(dying)
		return Dead(index);
	if(clk == 0)
		removearmos(x,y);
	if(clk < 0 || dying || stunclk || watch || ceiling || frozenclock)
		return enemy::animate(index);

	// When the timer's run out or it can't keep going,
	// it's time to try a different action.
	if(actionTimer <= 0 ||
		((actionTimer&31) == 0 && !canmove(dir, 1_x, spw_door, false) && currAction == action::WALKING))
	{
		fix_coords();

		switch(currAction)
		{
		case action::PHASING:
			// Just moved a full tile. If the new position is walkable,
			// switch to walking. Otherwise, turn if needed, move
			// another tile, then check again.
			if(!m_walkflag(x, y, spw_door, dir))
				currAction = action::WALKING;
			else
			{
				if(!canmove(dir, 1_x, spw_wizzrobe, false))
					turn(0);
				actionTimer = 16;
			}
			break;

		case action::PAUSING:
			if(maybeJump())
				break;
			[[fallthrough]];

		case action::JUMPING:
			// Either stopped while walking or just finished jumping.
			// Adjust direction in case it's diagonal, then start walking.
			dir &= 3;
			currAction = action::WALKING;
			[[fallthrough]];

		case action::WALKING:
			// Can't go any farther, timer ran out, or just started walking.
			// In any case, pick a direction and get moving.
			turn(64);
			[[fallthrough]];

		default: // Initialize (up to three times, for whatever reason)
			if(!canmove(dir, 1_x, spw_door, false))
			{
				// Can't walk. Phase if possible, turn if not.
				if(canmove(dir, 15_x, spw_wizzrobe, false))
				{
					currAction = action::PHASING;
					actionTimer = 16;
				}
				else
				{
					turn(64);
					currAction = action::WALKING;
					actionTimer = 32;
				}
			}
			else
				actionTimer = 32;

			break;
		}

		// misc is currAction, so INIT -> INIT_2 -> INIT_3 -> WALKING
		// unless it was set to something else above.
		// It should never be negative after that.
		if(misc < 0)
			++misc;
	}

	--actionTimer;

	// It could just call move directly here, but scripts
	// can observe the step change.
	switch(currAction)
	{
	case action::PHASING:
	case action::JUMPING:
		step = 1_x;
		break;

	case action::PAUSING:
		step = 0_x;
		break;

	default:
		step = 0.5_x;
		break;
	}

	move(step);

	if(readyToFire())
	{
		eWpn.fire();
		shotTimer = 30;
	}
	if(shotTimer > 0)
		--shotTimer;

	if(currAction == action::WALKING && (rand()&127) == 0)
		currAction = action::PAUSING;

	if(currAction == action::PAUSING && actionTimer == 4)
		fix_coords();

	return enemy::animate(index);
}

bool MageFloating::readyToFire() const
{
	if(currAction == action::WALKING && actionTimer == 28)
	{
		if(dmisc2 != 1) // Not 8 shots
			return lined_up(8, false) == dir;
		else
			return rand()%500 >= 400;
	}
	return false;
}

void MageFloating::turn(int homing)
{
	// Mages shouldn't move to the edge of the screen;
	// if they're already there, they should move toward the center
	if(x < 32)
		dir = right;
	else if(x >= 224)
		dir = left;
	else if(y < 32)
		dir = down;
	else if(y >= 144)
		dir = up;
	else
		newdir(4, homing, spw_wizzrobe);
}

static inline bool canJumpTo(zfix x, zfix y)
{
	return x >= 32_x && x <= 208_x && y >= 32_x && y <= 128_x;
}

bool MageFloating::maybeJump()
{
	int jx = x;
	int jy = y;
	int jdir = -1;

	switch(rand()&7)
	{
	case 0:
		if(canJumpTo(x-32, y-32))
			// dir>7 is used instead of l_up, etc. because the next direction
			// after the jump is decided by dir&3.
			jdir = 15;
		break;

	case 1:
		if(canJumpTo(x+32, y-32))
			jdir = 9;
		break;

	case 2:
		if(canJumpTo(x+32, y+32))
			jdir = 11;
		break;

	case 3:
		if(canJumpTo(x-32, y+32))
			jdir = 13;
		break;

	default:
		break;
	}

	if(jdir >= 0)
	{
		dir = jdir;
		currAction = action::JUMPING;
		actionTimer = 32;
		return true;
	}
	else
		return false;
}

void MageFloating::draw(BITMAP *dest)
{
	if(currAction==action::JUMPING || currAction==action::PHASING)
	{
		// Might be invisible due to flickering
		if((actionTimer&1) && hp > 0 && !watch && !stunclk && !frozenclock)
			return;
	}

	dummy_int[1] = shotTimer;
	update_enemy_frame();
	// XXX Does not setting dummy_int back to its previous value right here
	// cause any problems? If so, it's not immediately obvious.
	enemy::draw(dest);
}

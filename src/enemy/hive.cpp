#include "hive.h"
#include "../link.h"

static direction angleToDir(double angle)
{
	if(angle <= -5*PI/8 && angle > -7*PI/8)
		return l_down;
	else if(angle <= -3*PI/8 && angle > -5*PI/8)
		return left;
	else if(angle <= -1*PI/8 && angle > -3*PI/8)
		return l_up;
	else if(angle <= 1*PI/8 && angle > -1*PI/8)
		return up;
	else if(angle <= 3*PI/8 && angle > 1*PI/8)
		return r_up;
	else if(angle <= 5*PI/8 && angle > 3*PI/8)
		return right;
	else if(angle <= 7*PI/8 && angle > 5*PI/8)
		return r_down;
	else
		return down;
}

int Hive::numOrbiters(guydata& data)
{
	if(data.misc10 == 1)
		// Big - outer orbiters only
		return data.misc1;
	else
		// Small - inner and outer
		return data.misc1+data.misc2;
}

// Hive ========================================================================

Hive::Hive(zfix _x, zfix _y, int id, int clk):
	enemy(_x, _y, id, clk),
	mainTimer(clk2),
	outerRingCount(dmisc1),
	innerRingCount(dmisc2),
	patternCounter(0)
{
	if((editorflags&ENEMY_FLAG5) == 0)
	{
		x = 128_x;
		y = 48_x;
	}

	dir = (rand()&7)+8;
	if(dmisc6 < 1) // Make sure pattern timer is valid
		dmisc6 = 1;

	if(isBig())
	{
		step = 0.25_x;
		innerRingCount = 0; // Big hives don't have inner rings
		dmisc5 = 0;  // or shoot projectiles
		timerLimit = 90;
		hxsz = 32;
		hxofs = -8;
	}
	else
		timerLimit = 84;

	orbiters.reserve(outerRingCount+innerRingCount);

	SIZEflags = d->SIZEflags;
	if((SIZEflags & guyflagOVERRIDE_TILE_WIDTH) && txsz > 0)
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

HiveOrbiter* Hive::createOrbiter()
{
	HiveOrbiter* orbiter;
	if(orbiters.size() < outerRingCount) // Outer
		orbiter = new HiveOrbiter(id|0x1000, this, orbiters.size(), false);
	else // Inner
		orbiter = new HiveOrbiter(id|0x1000, this, orbiters.size()-outerRingCount, true);
	orbiters.push_back(orbiter);
	return orbiter;
}

void Hive::orbiterDied(HiveOrbiter* orbiter)
{
	if(orbiter->inner)
		--innerRingCount;
	else
		--outerRingCount;

	for(int i = 0; i < orbiters.size(); ++i)
	{
		if(orbiter == orbiters[i])
		{
			orbiters.erase(orbiters.begin()+i);
			break;
		}
	}
}

bool Hive::animate(int index)
{
	if(dying)
	{
		for(auto* orbiter: orbiters)
		{
			orbiter->parent = nullptr;
			orbiter->hp = -1000;
		}
		orbiters.clear();
		return Dead(index);
	}

	if(clk == 0)
		removearmos(x,y);

	variable_walk_8(rate, homing, hrate, spw_floater);

	++mainTimer;
	if(mainTimer == timerLimit)
	{
		mainTimer = 0;

		if(patternCounter)
			--patternCounter;
		else
		{
			if(misc%dmisc6 == 0)
				patternCounter = dmisc7;
		}

		++misc;
	}

	if(dmisc5 == 1 && (zc_oldrand()&127) == 0)
		eWpn.fire();

	return enemy::animate(index);
}

int Hive::defend(int wpnId, int *power, int edef)
{
	int ret = enemy::defend(wpnId, power, edef);

	if(ret<0 && (outerRingCount || innerRingCount))
		return 0;

	return ret;
}

int Hive::defendNew(int wpnId, int *power, int edef, byte unblockable)
{
	int ret = enemy::defendNew(wpnId, power, edef, unblockable);

	if(ret < 0 && (outerRingCount || innerRingCount))
		return 0;

	return ret;
}

void Hive::draw(BITMAP *dest)
{
	tile=o_tile;


	if(isBig())
	{
		if(get_bit(quest_rules, qr_NEWENEMYTILES))
		{
			static constexpr int tileOffset[8] = { 0, 8, 40, 48, 80, 88, 120, 128 };
			flip = 0;
			int tileDir = angleToDir(atan2(double(y-(Link.y)), double(Link.x-x)));
			tile += tileOffset[tileDir];

			tile += (2*(clk&3));
			xofs -= 8;
			yofs -= 8;
			drawblock(dest, 15);
			xofs += 8;
			yofs += 8;
		}
		else
		{
			flip = clk&1;
			xofs -= 8;
			yofs -= 8;
			enemy::draw(dest);
			xofs += 16;
			enemy::draw(dest);
			yofs += 16;
			enemy::draw(dest);
			xofs -= 16;
			enemy::draw(dest);
			xofs += 8;
			yofs -= 8;
		}
	}
	else
	{
		update_enemy_frame();
		enemy::draw(dest);
	}
}

// Hive orbiter ================================================================

HiveOrbiter::HiveOrbiter(int id, Hive* parent, int pos, bool inner):
	enemy(parent->x, parent->y, id, -((pos*21)>>1)-1),
	inner(inner),
	parent(parent)
{
	misc = pos;
	hp = dmisc3;
	cs = dmisc9;
	item_set = 0;
	mainguy = false;
	count_enemy = false;
	flags &= ~guy_neverret;
	bgsfx = -1;
	deadsfx = WAV_EDEAD;
	hitsfx = WAV_EHIT;
	isCore = false;
	parent_script_UID = parent->script_UID;

	double numOrbiters = inner ? dmisc2 : dmisc1;
	relOffset = double(misc)/numOrbiters;
	absOffset = relOffset*PI*2;

	if(inner)
	{
		if(get_bit(quest_rules, qr_NEWENEMYTILES))
		{
			if(dmisc5==1)
				// The center eye shoots projectiles;
				// make room for its firing tiles
				o_tile = parent->o_tile+120;
			else
				// The center eyes does not shoot; use
				// the next two tile rows for inner eyes.
				o_tile = parent->o_tile+40;
		}
		else
			o_tile = parent->o_tile+1;
	}
	else
	{
		if(get_bit(quest_rules, qr_NEWENEMYTILES))
			o_tile = parent->o_tile+dmisc8;
		else
			o_tile = parent->o_tile+1;
	}

	if(isBig())
	{
		hxsz = 16;
		hysz = 16;
	}
	else
	{
		hxsz = 12;
		hysz = 12;
		hxofs = 2;
		hyofs = 2;
	}
	yofs = playing_field_offset;

	if((SIZEflags & guyflagOVERRIDE_TILE_WIDTH) && txsz > 0)
	{
		txsz = parent->txsz;
		if(txsz > 1)
			extend = 3;
	}
	if((SIZEflags & guyflagOVERRIDE_TILE_HEIGHT) && tysz > 0)
	{
		tysz = parent->tysz;
		if(tysz > 1)
			extend = 3;
	}
	if((SIZEflags & guyflagOVERRIDE_HIT_WIDTH) && hxsz >= 0)
		hxsz = parent->hxsz;
	if((SIZEflags & guyflagOVERRIDE_HIT_HEIGHT) && hysz >= 0)
		hysz = parent->hysz;
	if((SIZEflags & guyflagOVERRIDE_HIT_Z_HEIGHT) && hzsz >= 0)
		hzsz = parent->hzsz;
	if(SIZEflags & guyflagOVERRIDE_HIT_X_OFFSET)
		hxofs = parent->hxofs;
	if(SIZEflags & guyflagOVERRIDE_HIT_Y_OFFSET)
		hyofs = parent->hyofs;
	if(SIZEflags & guyflagOVERRIDE_DRAW_X_OFFSET)
		xofs = parent->xofs;
	if(SIZEflags & guyflagOVERRIDE_DRAW_Y_OFFSET)
		yofs = parent->yofs; //This seems to be setting to +48 or something with any value set?! -Z
	if(SIZEflags & guyflagOVERRIDE_DRAW_Z_OFFSET)
		zofs = parent->zofs;
}

bool HiveOrbiter::animate(int index)
{
	if(dying)
	{
		if(parent)
		{
			parent->orbiterDied(this);
			parent = nullptr;
		}
		return Dead(index);
	}

	if(inner)
		superman = parent->outerRingCount > 0;

	if(dmisc4 == 0 && !isBig())
		positionBigCircle();
	else
		positionOval();

	if(inner && dmisc5 == 2 && (zc_oldrand()&127) == 0)
		eWpn.fire();

	return enemy::animate(index);
}

void HiveOrbiter::positionBigCircle()
{
	double orbit = inner ? 14.0 : 28.0;
	double a2 = PI*(parent->mainTimer-parent->timerLimit*relOffset)/(parent->timerLimit/2)+PI/2;
	double offsetX, offsetY;

	if(parent->patternCounter>0)
	{
		offsetX = (cos(a2)*2-sin(absOffset))*orbit;
		offsetY = (-sin(a2)*2+cos(absOffset))*orbit;
	}
	else
	{
		offsetX = cos(a2)*orbit;
		offsetY = -sin(a2)*orbit;
	}

	dir = angleToDir(atan2(offsetY, offsetX));

	x = parent->x+offsetX;
	if(inner)
		y = parent->y-offsetY;
	else
		y = parent->y+offsetY;
}

void HiveOrbiter::positionOval()
{
	double orbit;
	if(isBig())
		orbit = 45;
	else
		orbit = inner ? 21.0 : 42.0;

	double a2 = PI*(parent->mainTimer-parent->timerLimit*relOffset)/(parent->timerLimit/2)+PI/2;
	double circleX = cos(a2)*orbit;
	double circleY = -sin(a2)*orbit;
	double offsetX, offsetY;

	offsetX = circleX;
	if(parent->patternCounter > 0)
		offsetY = (-sin(a2)-cos(absOffset))*orbit/2.0;
	else
		offsetY = circleY;

	x = parent->x+offsetX;
	if(inner)
	{
		y = parent->y-offsetY;
		dir = angleToDir(atan2(circleY, -circleX));
	}
	else
	{
		y = parent->y+offsetY;
		dir = angleToDir(atan2(circleY, circleX));
	}
}

void HiveOrbiter::draw(BITMAP *dest)
{
	if(get_bit(quest_rules, qr_NEWENEMYTILES))
	{
		static constexpr int tileOffset[8] = { 0, 4, 8, 12, 20, 24, 28, 32 };
		flip = 0;
		tile = o_tile+tileOffset[dir&7];
		if(isBig())
			tile += (clk&6)>>1;
		else
			tile += clk&3;
	}
	else
	{
		if(isBig())
			tile = o_tile+(clk&4) ? 1 : 0;
		else
			tile = o_tile+((clk&2)>>1);
	}

	if(clk >= 0)
		enemy::draw(dest);
}

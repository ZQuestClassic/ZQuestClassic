#include "../ffscript.h"
#include "unicorn.h"
#include "../zelda.h"

Unicorn::Unicorn(const enemy& other, bool newScriptUID, bool clearParentUID):
	enemy(other),
	walkTimer(((Unicorn&)other).walkTimer),
	shotTimer(clk3)
{
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS * sizeof(int32_t));

		scrmem->scriptData = other.scrmem->scriptData;
	}
	else
		scrmem = nullptr;

	for(int32_t i = 0; i < edefLAST255; i++)
		defense[i] = other.defense[i];
	for(int32_t q = 0; q < 10; q++)
		frozenmisc[q] = other.frozenmisc[q];
	for(int32_t q = 0; q < NUM_HIT_TYPES_USED; q++)
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

Unicorn::Unicorn(zfix _x, zfix _y, int32_t id, int32_t clk):
	enemy(_x, _y, id, clk),
	facingDir(dmisc1 ? facing::RIGHT : facing::LEFT),
	walkTimer(clk),
	shotTimer(clk3)
{
	clk2 = 0;
	shotTimer = 32;
	dir = left;
	yofs = playing_field_offset+1;

	if((editorflags&ENEMY_FLAG5) == 0)
	{
		x = (facingDir==facing::RIGHT) ? 64 : 176;
		y = 64;
	}

	if(facingDir == facing::RIGHT && !get_bit(quest_rules,qr_NEWENEMYTILES))
		flip = 1;

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

bool Unicorn::animate(int32_t index)
{
	if(dying)
		return Dead(index);
	if(clk == 0)
		removearmos(x, y);

	--shotTimer;
	if(shotTimer == 0)
		eWpn.fire();
	// shotTimer affects animation - 32 frames of firing face
	// before actually firing
	if(shotTimer < -80 && (rand()&63) == 0)
		shotTimer = 32;

	// Every 64 frames, maybe turn around.
	if(((walkTimer+1)&63) == 0)
	{
		if(tooFarLeft())
			dir = right;
		else if(tooFarRight())
			dir = left;
		else
		{
			int d2 = (rand()%3)+1;
			if(d2 >= left)
				dir = d2;
		}
	}

	// Walk one pixel every 8 frames. walkTimer is negative while spawning.
	if(walkTimer >= -1 && ((walkTimer+1)&7) == 0)
	{
		if(dir == left)
			x -= 1;
		else
			x += 1;
	}

	walkTimer = (walkTimer+1)%256;

	return enemy::animate(index);
}

void Unicorn::draw(BITMAP *dest)
{
	if(get_bit(quest_rules,qr_NEWENEMYTILES))
	{
		xofs=(facingDir == facing::RIGHT ? -16 : 0);
		if(do_animation)
		{
			tile = o_tile+((clk&24)>>2);
			if(shotTimer > 0)
				tile += 40;
			else if(shotTimer > -32)
				tile += 80;
		}

		if(dying)
		{
			xofs = 0;
			enemy::draw(dest);
		}
		else
			drawblock(dest, 15);
	}
	else
	{
		int xblockofs=(facingDir == facing::RIGHT) ? -16 : 16;
		xofs = 0;

		if(clk < 0 || dying)
		{
			enemy::draw(dest);
			return;
		}
		if(do_animation)
		{
			// face (0=firing, 2=resting)
			tile = o_tile+((shotTimer > 0) ? 0 : 2);
			enemy::draw(dest);
			// tail
			tile = o_tile+((clk&16) ? 1 : 3);
			xofs = xblockofs;
			enemy::draw(dest);
			// body
			yofs += 16;
			xofs = 0;
			tile = o_tile+((clk&16) ? 20 : 22);
			enemy::draw(dest);
			xofs = xblockofs;
			tile = o_tile+((clk&16) ? 21 : 23);
			enemy::draw(dest);
			yofs -= 16;
		}
		else
			enemy::draw(dest);
	}
}

bool Unicorn::hit(weapon *w)
{
	if((w->scriptcoldet&1) == 0 || w->fallclk || w->drownclk)
		return false;

	switch(w->id)
	{
		case wBeam:
		case wRefBeam:
		case wMagic:
		hysz = 32;
	}

	bool ret = (dying || hclk>0) ? false : sprite::hit(w);
	hysz = 16;
	return ret;
}

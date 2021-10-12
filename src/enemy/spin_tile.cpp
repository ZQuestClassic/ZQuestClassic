#include "spin_tile.h"
#include "../ffscript.h"
#include "../link.h"
#include "../util.h"

SpinTile::SpinTile(const enemy& other, bool newScriptUID, bool clearParentUID):
	enemy(other)
{
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS*sizeof(long));
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else
		scrmem = nullptr;

	for(int i = 0; i < edefLAST255; ++i)
		defense[i] = other.defense[i];
	for(int q = 0; q < 10; ++q)
		frozenmisc[q] = other.frozenmisc[q];
	for(int q = 0; q < NUM_HIT_TYPES_USED; +q)
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

SpinTile::SpinTile(zfix x, zfix y, int id, int comboTile):
	enemy(x, y, id&0xFFF, 0)
{
	if(comboTile > 0)
	{
		// comboTile > 0 when created by a Spinning Tile combo
		o_tile = comboTile;
		cs = id>>12;
	}

	step = 0;
	mainguy = false;
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

bool SpinTile::animate(int index)
{
	if(fallclk || drownclk)
		return enemy::animate(index);
	if(dying)
		return Dead(index);

	if(clk == 0)
		removearmos(x,y);

	++misc;

	if(misc == 96)
	{
		angular = true;
		angle = atan2(double(Link.y-y), double(Link.x-x));
		dir = util::angleToDir8(angle+PI/2);
		step = zslongToFix(dstep*100);
	}

	if(y > 186 || y <= -16 || x > 272 || x <= -16)
		kickbucket();

	sprite::move(step);
	return enemy::animate(index);
}

void SpinTile::draw(BITMAP *dest)
{
	update_enemy_frame();
	y -= misc>>4;
	yofs += 2;
	enemy::draw(dest);
	yofs -= 2;
	y += misc>>4;
}

void SpinTile::drawshadow(BITMAP *dest, bool translucent)
{
	flip = 0;
	shadowtile = wpnsbuf[spr_shadow].newtile+clk%4;
	yofs += 4;
	if(!shadow_overpit(this))
		enemy::drawshadow(dest, translucent);
	yofs -= 4;
}

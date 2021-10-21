#include "falling_rock.h"
#include "ffscript.h"
#include "zelda.h"
#include <algorithm>

FallingRock::FallingRock(zfix x, zfix y, int32_t id):
	enemy(x, y, id, 0), // No spawn animation,
	fallTimer(-14),
	bounceTimer(0),
	drawDir(up)
{
	mainguy = false;
	fallTimer = -14;
	if(isBig())
	{
		hxofs = -10;
		hyofs = -10;
		hxsz = 36;
		hysz = 36;
		hzsz = 16;
	}
	else
	{
		hxofs = -2;
		hyofs = -2;
		hxsz = 20;
		hysz = 20;
	}

	if(d->SIZEflags&guyflagOVERRIDE_HIT_X_OFFSET)
		hxofs = d->hxofs;
	if(d->SIZEflags&guyflagOVERRIDE_HIT_Y_OFFSET)
		hyofs = d->hyofs;
	if((d->SIZEflags&guyflagOVERRIDE_HIT_WIDTH) && d->hxsz >= 0)
		hxsz = d->hxsz;
	if((d->SIZEflags&guyflagOVERRIDE_HIT_HEIGHT) && d->hysz >= 0)
		hysz = d->hysz;

	if((d->SIZEflags&guyflagOVERRIDE_TILE_WIDTH) && d->txsz > 0)
	{
		txsz = d->txsz;
		if(txsz > 1)
			extend = 3; //! Don;t forget to set extend if the tilesize is > 1.
	}
	if((d->SIZEflags&guyflagOVERRIDE_TILE_HEIGHT) && d->tysz > 0)
	{
		tysz = d->tysz;
		if(tysz > 1)
			extend = 3;
	}
	if((d->SIZEflags&guyflagOVERRIDE_HIT_Z_HEIGHT) && d->hzsz >= 0)
		hzsz = d->hzsz;
	if(d->SIZEflags&guyflagOVERRIDE_DRAW_X_OFFSET)
		xofs = d->xofs;
	if(d->SIZEflags&guyflagOVERRIDE_DRAW_Y_OFFSET)
	{
		yofs = d->yofs; //This seems to be setting to +48 or something with any value set?! -Z
		yofs += 56; //this offset fixes yofs not plaing properly. -Z
	}

	if(d->SIZEflags&guyflagOVERRIDE_DRAW_Z_OFFSET)
		zofs = d->zofs;
}

FallingRock::FallingRock(const enemy& other, bool newScriptUID, bool clearParentUID):
	enemy(other),
	fallTimer(((FallingRock&)other).fallTimer),
	bounceTimer(((FallingRock&)other).bounceTimer),
	drawDir(((FallingRock&)other).drawDir)
{
	if(other.scrmem)
	{
		alloc_scriptmem();
		memcpy(scrmem->stack, other.scrmem->stack, MAX_SCRIPT_REGISTERS*sizeof(int32_t));
		scrmem->scriptData = other.scrmem->scriptData;
	}
	else
		scrmem = nullptr;

	for(int32_t i = 0; i < edefLAST255; ++i)
		defense[i] = other.defense[i];
	for(int32_t q = 0; q < 10; ++q)
		frozenmisc[q] = other.frozenmisc[q];
	for(int32_t q = 0; q < NUM_HIT_TYPES_USED; ++q )
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

bool FallingRock::animate(int32_t index)
{
	if(fallclk || drownclk)
		return enemy::animate(index);
	if(dying)
		return Dead(index);

	if(clk == 0)
		removearmos(x, y);

	++fallTimer;
	if(fallTimer == 0) // Move to top, but don't fall yet
	{
		x = zc_oldrand()&0xF0;
		y = isBig() ? -32 : 0;
		bounceTimer = 0;
		fallTimer = zc_oldrand()&15;
	}

	else if(fallTimer > 16) // Falling
	{
		if(bounceTimer <= 0) // Start bouncing
		{
			// Using non-standard directions because why not.
			// 1 is right, 0 is left.
			// Can't change this on the off chance scripts depend on it.
			if(x < 32)
				dir = 1;
			else if(x > 208)
				dir = 0;
			else
				dir = zc_oldrand()&1;
		}

		if(bounceTimer < 29)
		{
			x += dir ? 1 : -1;

			if(bounceTimer < 2)
			{
				y -= 2;
				drawDir = dir ? r_up : l_up;
			}
			else if(bounceTimer < 5)
			{
				--y;
				drawDir = dir ? r_up : l_up;
			}
			else if(bounceTimer < 8)
			{
				drawDir = dir ? right : left;
			}
			else if(bounceTimer < 11)
			{
				++y;
				drawDir = dir ? r_down : l_down;
			}
			else
			{
				y += 2;
				drawDir = dir ? r_down : l_down;
			}

			++bounceTimer;
		}
		else if(y < 176)
			bounceTimer = 0; // Bounce again
		else
			fallTimer = -(zc_oldrand()&63); // Start falling again after a bit
	}

	return enemy::animate(index);
}

int32_t FallingRock::takehit(weapon*)
{
	return 0;
}

void FallingRock::draw(BITMAP* dest)
{
	if(fallTimer >= 0 || fallclk || drownclk)
	{
		auto tempDir = dir;
		dir = drawDir;
		update_enemy_frame();
		if(isBig())
		{
			xofs -= 8;
			yofs -= 8;
			drawblock(dest, 15);
			xofs += 8;
			yofs += 8;
		}
		else
			enemy::draw(dest);
		dir = tempDir;
	}
}

void FallingRock::drawshadow(BITMAP *dest, bool translucent)
{
	if(fallTimer >= 0 && !shadow_overpit(this))
	{
		auto tempY = yofs;
		if(isBig())
		{
			int frame = (clk*8/frate)&~1;
			shadowtile = wpnsbuf[spr_shadow].newtile+frame;

			yofs += std::clamp<int>(bounceTimer, 0, 29-bounceTimer);
			yofs += 8;
			xofs -= 8;
			enemy::drawshadow(dest, translucent);
			xofs += 16;
			++shadowtile;
			enemy::drawshadow(dest, translucent);
			yofs += 16;
			shadowtile += 20;
			enemy::drawshadow(dest, translucent);
			xofs -= 16;
			--shadowtile;
			enemy::drawshadow(dest, translucent);
			xofs += 8;
		}
		else
		{
			int frame = 0;
			if(get_bit(quest_rules, qr_NEWENEMYTILES))
			{
				int frameDiv = frate/4;
				frame = (frameDiv == 0) ? 0 : clk/frameDiv;
			}
			else if(clk >= frate/2)
				frame = 1;
			shadowtile = wpnsbuf[spr_shadow].newtile+frame;

			yofs += std::clamp<int>(bounceTimer, 0, 29-bounceTimer);
			yofs += 8;
			enemy::drawshadow(dest, translucent);
		}
		yofs=tempY;
	}
}

#include "land_worm.h"
#include "link.h"
#include <algorithm>

// Controller ==================================================================

int32_t LandWorm::numSegments(const guydata& data)
{
	return std::clamp<int32_t>(data.misc1, 1, MAX_SEGMENTS);
}

LandWorm::LandWorm(zfix x, zfix y, int32_t id):
	enemy(x, y, id, 0),
	segmentsCreated(0),
	lastSegmentCreated(nullptr),
	moveHistory(std::make_shared<std::deque<direction>>())
{
	if(!(editorflags&ENEMY_FLAG5))
	{
		x = 64;
		y = 80;
		adjustSpawnPoint();
	}

	superman = 1;
	fading = fade_invisible;
	hxofs = 1000;
	item_set = 0;
}

void LandWorm::adjustSpawnPoint()
{
	if(!m_walkflag_simple(x, y))
		return;

	zfix oldX = x, oldY = y;

	// We'll check each tile in squares radiating outward from the original
	// spawn location.
	for(zfix dist = 16_x; dist < 240_x; dist += 16)
	{
		for(y = oldY-dist; y <= oldY+dist; y += 16_x)
		{
			if(y != oldY)
			{
				// Above or below previous tiles checked. Try the whole line
				// from left to right.
				for(x = oldX-dist; x <= oldX+dist; x += 16_x)
				{
					if(!m_walkflag_simple(x, y))
						return;
				}
			}
			else
			{
				// y == oldY. Just check the far left and right; we've already
				// covered the tiles in between.
				if(!m_walkflag_simple(oldX-dist, y))
				{
					x = oldX-dist;
					return;
				}
				else if(!m_walkflag_simple(oldX+dist, y))
				{
					x = oldX+dist;
					return;
				}
			}
		}
	}

	// Couldn't find a valid spot; just go back to the original location.
	x = oldX;
	y = oldY;
}

LandWormSegment* LandWorm::createSegment()
{
	int32_t clk = -(segmentsCreated<<dmisc2);
	++segmentsCreated;
	if(segmentsCreated > 1)
	{
		auto* newSegment = new LandWormSegment(x, y, id|0x2000, clk, moveHistory);
		lastSegmentCreated->nextSegment = newSegment;
		newSegment->prevSegment = lastSegmentCreated;
		lastSegmentCreated = newSegment;
		return lastSegmentCreated;
	}
	else
	{
		lastSegmentCreated = new LandWormSegment(x, y, id|0x1000, clk, moveHistory);
		return lastSegmentCreated;
	}
}

bool LandWorm::animate(int32_t index)
{
	if(clk == 0)
		removearmos(x, y);

	// We want the controller to continue to exist for script compatibility,
	// but it doesn't actually need to do anything. To reduce the chances of
	// errors from raw pointers and script weirdness, we'll use the number of
	// references to moveHistory to track whether any segments are still alive.
	if(moveHistory.use_count() == 1)
	{
		setmapflag(mTMPNORET);
		stop_bgsfx(index);
		return true;
	}

	// This was here to prevent scripts from killing the controller.
	// That's less important now, but some scripts might still rely on it.
	hp = 1;
	return enemy::animate(index);
}

// Segment =====================================================================

LandWormSegment::LandWormSegment(zfix _x, zfix _y, int32_t id, int32_t clk,
	std::shared_ptr<std::deque<direction>> moveHistory):
		enemy(_x, _y, id, clk),
		prevSegment(nullptr),
		nextSegment(nullptr),
		moveTimer(0),
		moveIndex(-1),
		moveHistory(moveHistory)
{
	hxofs = 1000;
	hxsz = 8;
	mainguy = false;
	count_enemy = isHead();
	bgsfx = -1;
	isCore = false;
	flags &= ~guy_neverret;

	if(!isHead())
	{
		if(dmisc3 == 0)
			item_set = 0;
		else if(FFCore.getQuestHeaderInfo(vZelda) >= 0x210 && FFCore.emulation[emuITEMPERSEG])
			item_set = 0;
	}
}

bool LandWormSegment::animate(int32_t index)
{
	if(hp<=0 && (prevSegment || nextSegment))
		moveToEnd();

	if(dying)
	{
		xofs = 0;
		return Dead(index);
	}

	if(clk >= 0)
	{
		hxofs = 4;
		if(!watch)
			walk();
	}

	return enemy::animate(index);
}

void LandWormSegment::walk()
{
	if(moveTimer <= 0)
	{
		fix_coords(true);
		if(isHead())
		{
			newdir(rate, homing, spw_none);
			moveHistory->push_front((direction)dir);
		}
		else
		{
			if(moveIndex < 0)
				moveIndex = moveHistory->size()-1;
			dir = (*moveHistory)[moveIndex];

		}
		// Remove elements as they're no longer needed.
		// Actually, this will be larger than necessary once a segment
		// is deleted, but it's sufficient to keep the deque from growing.
		if(isTail())
			moveHistory->pop_back();

		if(step == 0)
			moveTimer = 0;
		else
			moveTimer = 16_x/step;
	}

	--moveTimer;
	move(step);
}

void LandWormSegment::moveToEnd()
{
	if(!nextSegment) // Already at the end?
	{
		if(prevSegment)
		{
			prevSegment->nextSegment = nullptr;
			prevSegment = nullptr;
		}
		return;
	}
	else // Not at the end; push hp and hclk to the tail
	{
		auto* curr = this;
		auto* next = nextSegment;
		while(next)
		{
			std::swap(curr->hp, next->hp);
			std::swap(curr->hclk, next->hclk);
			curr = next;
			next = next->nextSegment;
		}

		curr->nextSegment = nullptr;
		hclk = 33; // And don't immediately get hit again
	}
}

int32_t LandWormSegment::takehit(weapon *w)
{
	if(enemy::takehit(w))
		// Wait a frame before checking sword attacks again
		return (w->id == wSBomb) ? 1 : 2;

	return 0;
}

void LandWormSegment::draw(BITMAP *dest)
{
	tile = o_tile;

	if(get_bit(quest_rules, qr_NEWENEMYTILES))
	{
		flip = 0;
		if(!isHead())
		{
			if(isTail())
				tile+=40;
			else
				tile+=20;
		}
		int32_t frameDiv = frate/4;
		int32_t frame = (frameDiv == 0) ? 0 : clk/frameDiv;
		tile += (dir&3)*4;
		tile += frame;
	}
	else
	{
		if(!isHead())
			++tile;
	}

	if(clk >= 0)
		enemy::draw(dest);
}

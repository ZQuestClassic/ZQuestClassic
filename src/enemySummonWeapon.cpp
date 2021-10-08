#include "enemySummonWeapon.h"
#include "guys.h"
#include "link.h" // declares guys and Link
#include "maps.h"
#include "sfx.h"

EnemySummonWeapon::EnemySummonWeapon(enemy& owner, guydata& data):
	owner(owner),
	type(EnemySummonWeapon::summonType::FROM_LAYER),
	enemyToSummon(0),
	sfx(WAV_FIRE)
{
	init(data);
}

EnemySummonWeapon::EnemySummonWeapon(enemy& owner, const EnemySummonWeapon& other):
	owner(owner),
	type(other.type),
	enemyToSummon(other.enemyToSummon),
	sfx(other.sfx)
{}

void EnemySummonWeapon::init(guydata& data)
{
	switch(data.family)
	{
	case eeWIZZ:
		if(data.misc2 == 2)
		{
			type=summonType::SINGLE;
			enemyToSummon = data.misc3;
		}
		else
			type=summonType::FROM_LAYER;
	}

	// XXX It looks like batrobes always used WAV_FIRE before.
	// What should be done here?
	sfx = get_bit(quest_rules, qr_MORESOUNDS) ? WAV_ZN1SUMMON : WAV_FIRE;
}

void EnemySummonWeapon::summon() const
{
	switch(type)
	{
	case summonType::SINGLE:
	{
		int existing = 0;

		for(int i = 0; i < guys.Count(); ++i)
		{
			if((((enemy*)guys.spr(i))->id) == enemyToSummon)
			++existing;
		}

		if(existing <= 40)
		{
			int numToSummon = (rand()%3)+1;
			for(int i = 0; i < numToSummon; ++i)
				spawnEnemy(enemyToSummon, owner.x, owner.y);
		}

		sfx.play(owner.x);
		break;
	}
	case summonType::FROM_LAYER:
	{
		if(count_layer_enemies() == 0)
			break;
		if(guys.Count() >= 200)
			break;

		int numToSummon = (rand()%3)+1;
		bool summoned = false;

		for(int i = 0; i < numToSummon; ++i)
		{
			// TODO It shouldn't be necessary to adjust the return value.
			int id = vbound(random_layer_enemy(), eSTART, eMAXGUYS-1);
			for(int i = 0; i < 20; ++i)
			{
				int x = 16*((rand()%12)+2);
				int y = 16*((rand()%7)+2);

				if(!owner.m_walkflag(x, y, 0, owner.dir) &&
					(abs(x-Link.getX()) >= 32 || abs(y-Link.getY()) >= 32))
				{
					spawnEnemy(id, zfix(x), zfix(y),
						get_bit(quest_rules,qr_ENEMIESZAXIS) ? 64_x : 0_x);

					summoned=true;
					break;
				}
			}
		}

		if(summoned)
			sfx.play(owner.x);
		break;
	}
	}
}

void EnemySummonWeapon::spawnEnemy(int id, zfix x, zfix y, zfix z) const
{
	// XXX Why is clk -10?
	// XXX This always marks the last enemy
	auto index = guys.Count();
	auto added = addchild(x, y, z, id, -10, owner.script_UID);
	((enemy*)guys.spr(index))->count_enemy = false;
}

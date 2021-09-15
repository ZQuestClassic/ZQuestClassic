#ifndef ZC_ENEMYSUMMONWEAPON_H
#define ZC_ENEMYSUMMONWEAPON_H

#include "sound.h"
#include "zfix.h"
class enemy;
struct guydata;

class EnemySummonWeapon
{
public:
	EnemySummonWeapon(enemy& owner, guydata& data);
	EnemySummonWeapon(enemy& owner, const EnemySummonWeapon& other);
	void summon() const;

private:
	enum class Type: bool { single, fromLayer };

	enemy& owner;
	Type type;
	int enemyToSummon;
	Sound sfx;

	void init(guydata& data);
	void spawnEnemy(int id, zfix x, zfix y, zfix z = 0_x) const;
};

#endif

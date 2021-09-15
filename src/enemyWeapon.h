#ifndef ZC_ENEMYWEAPON_H
#define ZC_ENEMYWEAPON_H

#include "enemyProjectileWeapon.h"
#include "enemySummonWeapon.h"
#include "zfix.h"
class enemy;
struct guydata;

class EnemyWeapon
{
public:
	EnemyWeapon(enemy& owner, guydata& data);
	EnemyWeapon(enemy& owner, const EnemyWeapon& other);
	void fire(zfix xOffset = 0_x, zfix yOffset = 0_x) const;

private:
	enum class Type: char
	{
		none, projectile, summon
	};

	Type type;
	union WeaponUnion
	{
		EnemyProjectileWeapon proj;
		EnemySummonWeapon summon;
		int dummy;

		WeaponUnion(EnemyWeapon::Type type, enemy& owner, guydata& data);
		WeaponUnion(EnemyWeapon::Type type, enemy& owner,
			const WeaponUnion& other);
	} wpn;

	friend Type getType(guydata&);
};

#endif

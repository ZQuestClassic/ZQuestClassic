#ifndef ZC_ENEMYPROJECTILEWEAPON_H
#define ZC_ENEMYPROJECTILEWEAPON_H

#include "sound.h"
#include "zfix.h"
class enemy;
struct guydata;
class weapon;

class EnemyProjectileWeapon
{
public:
	EnemyProjectileWeapon(enemy& owner, guydata& data);
	EnemyProjectileWeapon(enemy& owner, const EnemyProjectileWeapon& other);
	void fire(zfix xOffset = 0_x, zfix yOffset = 0_x) const;

private:
	enum class attackPattern: char {
		NONE, BASIC, FIRE_MAGE, UNICORN
	};

	enemy& owner;
	attackPattern pattern;
	short wpn, damage;
	int wpnSubtype;
	Sound sfx;

	void init(guydata& data);

	/* Fires a single weapon. */
	weapon& spawnWeapon(zfix x, zfix y, zfix z, int dir) const;
};

#endif

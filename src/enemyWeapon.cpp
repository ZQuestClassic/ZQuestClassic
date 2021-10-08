#include "enemyWeapon.h"
#include "zdefs.h"
#include <new>

EnemyWeapon::attackType getType(guydata& data)
{
	// XXX Is this right? This is how it was in 2.55, but check 2.50.
	// Did summoners not work with no weapon set?
	if(data.weapon == 0)
		return EnemyWeapon::attackType::NONE;

	switch(data.family)
	{
	case eeAQUA:
		return EnemyWeapon::attackType::PROJECTILE;

	case eeWIZZ:
		if(data.misc2 < 2)
			return EnemyWeapon::attackType::PROJECTILE;
		else
			return EnemyWeapon::attackType::SUMMON;
	}

	return EnemyWeapon::attackType::NONE;
}

EnemyWeapon::EnemyWeapon(enemy& owner, guydata& data):
	type(getType(data)),
	wpn(WeaponUnion(type, owner, data))
{}

EnemyWeapon::EnemyWeapon(enemy& owner, const EnemyWeapon& other):
	type(other.type),
	wpn(other.type, owner, other.wpn)
{}

EnemyWeapon::WeaponUnion::WeaponUnion(EnemyWeapon::attackType type, enemy& owner, guydata& data):
	dummy(0)
{
	if(type == EnemyWeapon::attackType::PROJECTILE)
		new(&proj) EnemyProjectileWeapon(owner, data);
	else if(type == EnemyWeapon::attackType::SUMMON)
		new(&summon) EnemySummonWeapon(owner, data);
}

EnemyWeapon::WeaponUnion::WeaponUnion(EnemyWeapon::attackType type, enemy& owner,
	const EnemyWeapon::WeaponUnion& other):
		dummy(0)
{
	if(type == EnemyWeapon::attackType::PROJECTILE)
		new(&proj) EnemyProjectileWeapon(owner, other.proj);
	else if(type == EnemyWeapon::attackType::SUMMON)
		new(&summon) EnemySummonWeapon(owner, other.summon);
}

void EnemyWeapon::fire(zfix xOffset, zfix yOffset) const
{
	switch(type)
	{
	case attackType::PROJECTILE:
		wpn.proj.fire(xOffset, yOffset);
		break;

	case attackType::SUMMON:
		wpn.summon.summon();
		break;

	default:
		break;
	}
}

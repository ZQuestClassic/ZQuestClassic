#include "enemyProjectileWeapon.h"
#include "guys.h"
#include "link.h" // for Ewpns
#include "sfx.h"
#include "weapons.h"
#include "zdefs.h"

static int defaultSound(int wpn)
{
	switch(wpn)
	{
	case ewFireTrail:
	case ewFlame:
	case ewFlame2Trail:
	case ewFlame2:
		return WAV_FIRE;

	case ewWind:
	case ewMagic:
		return WAV_WAND;

	case ewIce:
		return WAV_ZN1ICE;

	case ewRock:
		if(get_bit(quest_rules, qr_MORESOUNDS))
            return WAV_ZN1ROCK;
        break;

	case ewFireball2:
	case ewFireball:
		if(get_bit(quest_rules, qr_MORESOUNDS))
            return WAV_ZN1FIREBALL;
        break;
	}

	return -1;
}

static int fireWizzrobeDefaultSound(int wpn)
{
    switch(wpn)
    {
        case ewFireball:
        case ewFireball2:
        case ewArrow:
            return 40;

        case ewBrang:
            return 4;

        case ewSword:
            return 20;

        case ewRock:
            return 51;

        case ewMagic:
            return 32;

        case ewBomb:
        case ewSBomb:
            return 3;

        case ewLitBomb:
        case ewLitSBomb:
            return 21;

        case ewFireTrail:
        case ewFlame:
        case ewFlame2:
        case ewFlame2Trail:
            return 13;

        case ewWind:
            return 32;

        case ewIce:
            return 44;

        default:
            return WAV_FIRE;
    }
}

EnemyProjectileWeapon::EnemyProjectileWeapon(enemy& owner, guydata& data):
    owner(owner),
    attackType(AttackType::none),
    wpn(data.weapon),
    damage(data.wdp),
    type(0),
    sfx(0)
{
    init(data);
}

EnemyProjectileWeapon::EnemyProjectileWeapon(enemy& owner, const EnemyProjectileWeapon& other):
    owner(owner),
    attackType(other.attackType),
    wpn(other.wpn),
    damage(other.damage),
    type(other.type),
    sfx(other.sfx)
{}

void EnemyProjectileWeapon::init(guydata& data)
{
    if(data.weapon==0)
    {
        attackType=AttackType::none;
        return;
    }

    switch(data.family)
    {
    case eeAQUA:
        attackType=AttackType::aquamentus;
        type=2;
        sfx=defaultSound(data.weapon);
        break;

    case eeWIZZ:
        if(data.misc2==0)
        {
            attackType=AttackType::wizzrobe;
            sfx=WAV_WAND;
        }
        else if(data.misc2==1)
        {
            attackType=AttackType::fireWizzrobe;
            if(FFCore.emulation[emu8WAYSHOTSFX])
                sfx=WAV_FIRE;
            else
                sfx=fireWizzrobeDefaultSound(data.weapon);
        }
        break;

    default:
        attackType=AttackType::none;
    }
}

void EnemyProjectileWeapon::fire(zfix xOffset, zfix yOffset) const
{
    zfix x=owner.x+xOffset;
    zfix y=owner.y+yOffset;

    switch(attackType)
    {
    case AttackType::aquamentus:
        // Aquamentus shots drift in the specified direction.
        fireDirectional(x, y, owner.z, up);
        fireDirectional(x, y, owner.z, 8);
        fireDirectional(x, y, owner.z, down);
        sfx.play(x);
        break;

    case AttackType::wizzrobe:
        fireDirectional(x, y, owner.z, owner.dir);
        sfx.play(x);
        break;

    case AttackType::fireWizzrobe:
        fireDirectional(x, y, owner.z, up).moveflags&=~FLAG_CAN_PITFALL;
        fireDirectional(x, y, owner.z, down).moveflags&=~FLAG_CAN_PITFALL;
        fireDirectional(x, y, owner.z, left).moveflags&=~FLAG_CAN_PITFALL;
        fireDirectional(x, y, owner.z, right).moveflags&=~FLAG_CAN_PITFALL;
        fireDirectional(x, y, owner.z, l_up).moveflags&=~FLAG_CAN_PITFALL;
        fireDirectional(x, y, owner.z, r_up).moveflags&=~FLAG_CAN_PITFALL;
        fireDirectional(x, y, owner.z, l_down).moveflags&=~FLAG_CAN_PITFALL;
        fireDirectional(x, y, owner.z, r_down).moveflags&=~FLAG_CAN_PITFALL;
        sfx.play(x);
        break;

    case AttackType::none:
        break;
    }
}

weapon& EnemyProjectileWeapon::fireDirectional(zfix x, zfix y, zfix z, int dir) const
{
    weapon* newWeapon=new weapon(
        x, y, z, wpn, type, damage, dir, -1, owner.getUID(), false);
    Ewpns.add(newWeapon);
    return *newWeapon;
}

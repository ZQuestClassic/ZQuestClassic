#include "enemyProjectileWeapon.h"
#include "guys.h"
#include "link.h" // for Ewpns
#include "sfx.h"
#include "weapons.h"
#include "zdefs.h"

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

    // Get the default sound for the weapon - might be changed later
    switch(data.weapon)
    {
        // XXX These were taken from the fire wizzrobe attack.
        // They mey not be right in general - at least the default.
        case ewFireball: sfx=40; break;
        case ewArrow: sfx=40; break;
        case ewBrang: sfx=4; break;
        case ewSword: sfx=20; break;
        case ewRock: sfx=51; break;
        case ewMagic: sfx=32; break;
        case ewBomb: sfx=3; break;
        case ewSBomb: sfx=3; break;
        case ewLitBomb: sfx=21; break;
        case ewLitSBomb: sfx=21; break;
        case ewFireTrail: sfx=13; break;
        case ewFlame: sfx=13; break;
        case ewWind: sfx=32; break;
        case ewFlame2: sfx=13; break;
        case ewFlame2Trail: sfx=13; break;
        case ewIce: sfx=44; break;
        case ewFireball2: sfx=40; break;
        default: sfx=WAV_FIRE;  break;
    }

    switch(data.family)
    {
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

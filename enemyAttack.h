#ifndef _ZC_ENEMYATTACK_H_
#define _ZC_ENEMYATTACK_H_

#include "sfxClass.h"
class enemy;
class weapon;

enum fireType
{
    ft_1Shot, ft_1ShotSlanted, ft_3Shots,ft_4Shots,
    ft_5Shots, ft_8Shots, ft_aquamentus, ft_gohma1Shot,
    ft_gohma3Shots, ft_breath, ft_breathAimed, ft_summon,
    ft_summonLayer
};

class EnemyAttack
{
public:
    EnemyAttack(int weaponID, int type, int power, fireType fType, const SFX& sfx);
    
    /// Fire the weapon or begin the breath attack.
    void activate();
    
    /// Activate breath attack with a specified duration.
    inline void setBreathTimer(int value) { breathTimer=value; }
    
    /// Update the active breath attack. isActive() should be checked first.
    void update();
    
    /// Returns true if this is a breath attack and it's currently firing.
    // Maybe it should also be true if a boomerang is out...
    bool isActive();
    
    /// Set the enemy associated with this attack.
    inline void setOwner(enemy* o) { owner=o; }
    
    inline void setXOffset(int x) { xOffset=x; }
    
    inline void setYOffset(int y) { yOffset=y; }
    
private:
    enemy* owner;
    int weaponID; // Doubles as summoned enemy ID
    int wpnType;
    int power; // Doubles as max number of enemies to summon
    fireType fType;
    int breathTimer;
    SFX sfx;
    int xOffset, yOffset;
    
    /// Creates a single enemy weapon.
    void fire(int dir, int type);
    
    /// Creates an enemy weapon at the beginning of Ewpns.
    weapon* fireFront(int dir, int type);
    
    /// Updates fire breath attack.
    weapon* fireBreath();
    
    /// Updates aimed breath attack.
    weapon* fireAimedBreath();
};

#endif


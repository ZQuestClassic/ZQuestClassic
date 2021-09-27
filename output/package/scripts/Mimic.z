// import "std.zh"
// import "string.zh"
// import "ghost.zh"


// How long to wait before firing again
const int GOR_FIRE_WAIT_TIME=30;

// Direction reversal settings
const int GOR_REVERSE_UD=1;
const int GOR_REVERSE_LR=2;
const int GOR_REVERSE_ALL=3;

// ffc->Misc[] index
const int GOR_IDX_FIRE_TIMER=0;

// npc->Attributes[] indices
const int GOR_ATTR_REVERSE=0;
const int GOR_ATTR_SPRITE_OVERRIDE=1;
const int GOR_ATTR_SOUND_OVERRIDE=2;
const int GOR_ATTR_IGNORE_CHARGE=3;
const int GOR_ATTR_COMBO=10;

ffc script Goriya_LttP
{
    void run(int enemyID)
    {
        npc ghost;
        int standCombo;
        int walkCombo;

        bool reverseLR;
        bool reverseUD;
        float step;
        bool ignoreCharge;

        int prevLinkX;
        int prevLinkY;
        float stepX;
        float stepY;

        int xDiff;
        int yDiff;

        bool fire;
        int weapon;
        int damage;
        int sprite;
        int sound;

        // Initialize
        ghost=Ghost_InitAutoGhost(this, enemyID);
        Ghost_SetFlag(GHF_NORMAL);
        Ghost_SetFlag(GHF_4WAY);
        Ghost_SpawnAnimationPuff(this, ghost);

        // Combos
        standCombo=ghost->Attributes[GOR_ATTR_COMBO];
        walkCombo=standCombo+4;

        // Weapon
        weapon=WeaponTypeToID(ghost->Weapon);
        damage=ghost->WeaponDamage;

        // Weapon sprite override
        if(ghost->Attributes[GOR_ATTR_SPRITE_OVERRIDE]>0)
            sprite=ghost->Attributes[GOR_ATTR_SPRITE_OVERRIDE];
        else
            sprite=GetDefaultEWeaponSprite(weapon);

        // Weapon sound override
        if(ghost->Attributes[GOR_ATTR_SOUND_OVERRIDE]>0)
            sound=ghost->Attributes[GOR_ATTR_SOUND_OVERRIDE];
        else
            sound=GetDefaultEWeaponSound(weapon);

        // Movement modifiers
        reverseUD=ghost->Attributes[GOR_ATTR_REVERSE]==GOR_REVERSE_UD ||
                  ghost->Attributes[GOR_ATTR_REVERSE]==GOR_REVERSE_ALL;
        reverseLR=ghost->Attributes[GOR_ATTR_REVERSE]==GOR_REVERSE_LR ||
                  ghost->Attributes[GOR_ATTR_REVERSE]==GOR_REVERSE_ALL;
        step=ghost->Step/150;
        if(step==0)
            step=1;

        ignoreCharge=ghost->Attributes[GOR_ATTR_IGNORE_CHARGE]!=0;

        // Link's previous position
        prevLinkX=Link->X;
        prevLinkY=Link->Y;

        while(true)
        {
            // Link isn't walking; match his direction, but don't do anything else
            Ghost_Data=standCombo;
            while(!(Link->Action==LA_WALKING || (Link->Action==LA_CHARGING && !ignoreCharge)) ||
                  (Link->X==prevLinkX && Link->Y==prevLinkY))
            {
                if(reverseLR && (Link->Dir==DIR_LEFT || Link->Dir==DIR_RIGHT))
                    Ghost_Dir=OppositeDir(Link->Dir);
                else if(reverseUD && (Link->Dir==DIR_UP || Link->Dir==DIR_DOWN))
                    Ghost_Dir=OppositeDir(Link->Dir);
                else
                    Ghost_Dir=Link->Dir;

                prevLinkX=Link->X;
                prevLinkY=Link->Y;

                GorWaitframe(this, ghost);
                if(Ghost_WasFrozen())
                {
                    prevLinkX=Link->X;
                    prevLinkY=Link->Y;
                }
            }

            // Link is walking; match his movement
            Ghost_Data=walkCombo;

            while((Link->Action==LA_WALKING || (Link->Action==LA_CHARGING && !ignoreCharge)) &&
                  (Link->X!=prevLinkX || Link->Y!=prevLinkY))
            {
                // Figure out where to move
                stepX=Link->X-prevLinkX;
                stepY=Link->Y-prevLinkY;

                // Adjust speed
                stepX*=step;
                stepY*=step;

                // Reversed?
                if(reverseLR)
                    stepX*=-1;
                if(reverseUD)
                    stepY*=-1;

                // Face the right way
                if(reverseLR && (Link->Dir==DIR_LEFT || Link->Dir==DIR_RIGHT))
                    Ghost_ForceDir(OppositeDir(Link->Dir));
                else if(reverseUD && (Link->Dir==DIR_UP || Link->Dir==DIR_DOWN))
                    Ghost_ForceDir(OppositeDir(Link->Dir));
                else
                    Ghost_ForceDir(Link->Dir);

                Ghost_MoveXY(stepX, stepY, 2);

                // Fire if facing Link and a weapon is set
                if(weapon>0 && this->Misc[GOR_IDX_FIRE_TIMER]==0)
                {
                    xDiff=Link->X-Ghost_X;
                    yDiff=Link->Y-Ghost_Y;
                    fire=false;

                    if(Ghost_Dir==DIR_UP)
                    {
                        if(yDiff<-16 && Abs(xDiff)<16)
                            fire=true;
                    }
                    else if(Ghost_Dir==DIR_DOWN)
                    {
                        if(yDiff>16 && Abs(xDiff)<16)
                           fire=true;
                    }
                    else if(Ghost_Dir==DIR_LEFT)
                    {
                        if(xDiff<-16 && Abs(yDiff)<16)
                            fire=true;
                    }
                    else // Right
                    {
                        if(xDiff>16 && Abs(yDiff)<16)
                            fire=true;
                    }

                    if(fire)
                    {
                        FireNonAngularEWeapon(ghost->Weapon, Ghost_X, Ghost_Y,
                                              Ghost_Dir, 300, damage, sprite,
                                              sound, EWF_ROTATE);
                        this->Misc[GOR_IDX_FIRE_TIMER]=GOR_FIRE_WAIT_TIME;
                    }
                }

                prevLinkX=Link->X;
                prevLinkY=Link->Y;

                GorWaitframe(this, ghost);
                if(Ghost_WasFrozen())
                {
                    prevLinkX=Link->X;
                    prevLinkY=Link->Y;
                }
            }
        }
    }

    void GorWaitframe(ffc this, npc ghost)
    {
        if(this->Misc[GOR_IDX_FIRE_TIMER]>0)
            this->Misc[GOR_IDX_FIRE_TIMER]--;

        Ghost_Waitframe(this, ghost, true, true);
    }
}
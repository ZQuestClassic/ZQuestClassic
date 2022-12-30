// import "std.zh"
// import "string.zh"
// import "ghost.zh"

const int BUZZ_WAIT_TIME = 60;
const int BUZZ_WALK_TIME = 60;
const int BUZZ_MIN_WALKS = 1;
const int BUZZ_MAX_WALKS = 8;
const int BUZZ_SHOCK_TIME = 30;

// npc->Attributes[] indices
const int BUZZ_ATTR_SWORD_LEVEL = 0;
const int BUZZ_ATTR_SFX_SHOCK = 1;
const int BUZZ_ATTR_SHOCK_SPRITE = 2;

// ffc->Misc[] index
const int BUZZ_IDX_SHOCKING = 0;

ffc script BuzzBlob
{
    void run(int enemyID)
    {
        npc ghost;
        int swordDefense;
        int step;
        int angle;
        int haltRate;
        int randomRate;
        int homing;
        int hunger;
        lweapon bait;
        
        // Initialize
        ghost=Ghost_InitAutoGhost(this, enemyID);
        Ghost_SetFlag(GHF_SET_DIRECTION);
        Ghost_SetFlag(GHF_KNOCKBACK);
        Ghost_SpawnAnimationPuff(this, ghost);
        
        randomRate=ghost->Rate;
        haltRate=ghost->Haltrate;
        homing=ghost->Homing;
        step=ghost->Step/100;
        angle=45*Rand(8);
        hunger=ghost->Hunger;
        swordDefense=ghost->Defense[NPCD_SWORD];
        
        while(true)
        {
            // Walk around
            do
            {
                // Change direction
                if(Rand(16)<randomRate)
                {
                    // Go for bait?
                    bait=LoadLWeaponOf(LW_BAIT);
                    if(bait->isValid() && Rand(4)<hunger)
                        angle=45*Round(ArcTan(bait->X-Ghost_X, bait->Y-Ghost_Y)/(PI/4));
                    // Aim at Link?
                    else if(Rand(256)<homing)
                        angle=45*Round(ArcTan(Link->X-Ghost_X, Link->Y-Ghost_Y)/(PI/4));
                    else
                        angle=45*Rand(8);
                }
                
                // Move
                for(int i=0; i<BUZZ_WALK_TIME; i++)
                {
                    Ghost_MoveAtAngle(angle, step, step);
                    BuzzWaitframe(this, ghost, swordDefense);
                }
            } while(Rand(16)>=haltRate);
            
            // Stand still
            BuzzWaitframes(this, ghost, swordDefense, BUZZ_WAIT_TIME);
        }
    }
    
    void BuzzWaitframe(ffc this, npc ghost, int swordDefense)
    {
        bool weakSword;
        
        // Is Link's sword too weak to hurt the blob?
        if(CurrentSwordLevel()<ghost->Attributes[BUZZ_ATTR_SWORD_LEVEL])
        {
            weakSword=true;
            ghost->Defense[NPCD_SWORD]=NPCDT_IGNORE;
        }
        else
        {
            weakSword=false;
            ghost->Defense[NPCD_SWORD]=swordDefense;
        }
        
        // If Link's sword is too strong, just wait
        if(!weakSword)
            Ghost_Waitframe(this, ghost, true, true);
        // Same thing if another Buzz Blob is already shocking Link
        else if(AlreadyBeingShocked(this))
            Ghost_Waitframe(this, ghost, true, true);
        // If stunned or a clock is active, set sword defense to normal and wait to recover
        else if(ghost->Stun>0 || ClockIsActive())
        {
            ghost->Defense[NPCD_SWORD]=swordDefense;
            
            while(ghost->Stun>0 || ClockIsActive())
                Ghost_Waitframe(this, ghost, true, true);
            
            ghost->Defense[NPCD_SWORD]=NPCDT_IGNORE;
        }
        
        // Otherwise, check for sword collisions
        else
        {
            lweapon sword;
            for(int i=Screen->NumLWeapons(); i>0; i--)
            {
                sword=Screen->LoadLWeapon(i);
                if(sword->ID!=LW_SWORD)
                    continue;
                    
                // Found the sword
                if(!Collision(sword, ghost))
                    break;
                
                // Got hit by the sword; hurt Link
                int oldLinkX;
                int oldLinkY;
                bool oldLinkInvis;
                bool oldLinkColl;
                bool alive=true;
                
                // Set this so other Buzz Blobs don't activate
                this->Misc[BUZZ_IDX_SHOCKING]=1;
                
                // Remember Link's data
                oldLinkX=Link->X;
                oldLinkY=Link->Y;
                oldLinkInvis=Link->Invisible;
                oldLinkColl=Link->CollDetection;
                
                // Flash
                Ghost_Data++;
                Ghost_StartFlashing();
                
                // Hurt Link, hide him, draw the shock graphic, and play the sound
                Link->HP-=4*ghost->WeaponDamage;
                
                Link->Invisible=true;
                Link->CollDetection=false;
                
                lweapon graphic=Screen->CreateLWeapon(LW_SCRIPT1);
                graphic->UseSprite(ghost->Attributes[BUZZ_ATTR_SHOCK_SPRITE]);
                graphic->X=Link->X;
                graphic->Y=Link->Y;
                graphic->CollDetection=false;
                graphic->DeadState=BUZZ_SHOCK_TIME;
                
                Game->PlaySound(ghost->Attributes[BUZZ_ATTR_SFX_SHOCK]);
                Screen->Quake=Max(Screen->Quake, 30);
                
                for(int j=0; j<BUZZ_SHOCK_TIME; j++)
                {
                    NoAction();
                    Link->X=oldLinkX;
                    Link->Y=oldLinkY;
                    
                    // Don't quit on death here; it would leave Link invisible
                    if(alive)
                        alive=Ghost_Waitframe(this, ghost, true, false);
                    else
                        Waitframe();
                    
                    // Make Link visible again if he died
                    if(Link->HP<=0)
                        Link->Invisible=oldLinkInvis;
                }
                
                // Unhide Link, stop flashing
                Link->Invisible=oldLinkInvis;
                Link->CollDetection=oldLinkColl;
                this->Misc[BUZZ_IDX_SHOCKING]=0;
                Ghost_Data--;
                
                if(!alive)
                    Quit();
                
                break;
            }
            Ghost_Waitframe(this, ghost, true, true);
        }
    }
    
    void BuzzWaitframes(ffc this, npc ghost, int swordDefense, int numFrames)
    {
        for(; numFrames>0; numFrames--)
            BuzzWaitframe(this, ghost, swordDefense);
    }
    
    // Get the level of Link's current sword
    int CurrentSwordLevel()
    {
        int weapon;
        itemdata id;
        
        // Check A first
        weapon=GetEquipmentA();
        id=Game->LoadItemData(weapon);
        if(id->Family==IC_SWORD)
            return id->Level;
        
        // No sword on A; try B
        weapon=GetEquipmentB();
        id=Game->LoadItemData(weapon);
        if(id->Family==IC_SWORD)
            return id->Level;
        
        // It's possible no sword is equipped; maybe the player used the sword
        // and then immediately changed weapons. In that case, find the highest
        // level sword Link has and assume that's what was used.
        
	    int ret = -1;
	    int curlevel = -1;
	    
	    // Check each item
	    for(int i = 0; i < 256; i++)
	    {
	        // Does Link have the current item?
		    if(!Link->Item[i])
		        continue;
	        
	        // Is it a sword?
		    id = Game->LoadItemData(i);
		    if(id->Family != IC_SWORD)
			    continue;
		    
		    // Check its level
		    if(id->Level > curlevel)
		        curlevel = id->Level;
	    }
	    
	    return curlevel;
    }
    
    bool AlreadyBeingShocked(ffc this)
    {
        ffc otherBB;
        
        // Check each other FFC
        for(int i=1; i<=32; i++)
        {
            otherBB=Screen->LoadFFC(i);
            
            if(otherBB==this)
                continue;
            
            // Right script and Misc[] index set? Then it's shocking Link.
            if(otherBB->Script==this->Script &&
               otherBB->Misc[BUZZ_IDX_SHOCKING]!=0)
                return true;
        }
        
        return false;
    }
}
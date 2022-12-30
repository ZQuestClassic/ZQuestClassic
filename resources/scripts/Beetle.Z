const int HARDHAT_ATTR_MIN_STEP = 0;
const int HARDHAT_ATTR_MAX_STEP = 1;

ffc script HardhatBeetle
{
    void run(int enemyID)
    {
        //Initialize
        npc ghost = Ghost_InitAutoGhost(this, enemyID);
        Ghost_SetFlag(GHF_STUN);
        Ghost_SetFlag(GHF_CLOCK);

        //Movement Variables
        int minStep = Ghost_GetAttribute(ghost, HARDHAT_ATTR_MIN_STEP, 67);
        int maxStep = Ghost_GetAttribute(ghost, HARDHAT_ATTR_MAX_STEP, 125);
        int ss = Rand(minStep,maxStep);
        int counter = 0;
        Trace(minStep);
        Trace(maxStep);

        //Knockback Variables
        int knockbackCounter;
        int knockbackAngle;

        //Spawn Animation
        Ghost_SpawnAnimationPuff(this, ghost);

        //Behavior Loop
        do
        {
            //Knockback
            if(Ghost_GotHit())
            {
                int xDiff = Abs(Link->X-Ghost_X)<<0;
                int yDiff = Abs(Link->Y-Ghost_Y)<<0;
                if(xDiff < (Ghost_TileWidth+1)*16 && yDiff < (Ghost_TileHeight+1)*16)
                {
                    knockbackCounter = __GH_KNOCKBACK_TIME;
                    knockbackAngle = Angle(Link->X, Link->Y, Ghost_X, Ghost_Y);
                }
                ss = minStep;
            }
            if(knockbackCounter!=0)
            {
                knockbackCounter--;
                int dir = AngleDir8(knockbackAngle);
                if(Ghost_CanMove(dir, __GH_KNOCKBACK_STEP/2, __GH_DEFAULT_IMPRECISION))
                   Ghost_Move(dir, __GH_KNOCKBACK_STEP/2, __GH_DEFAULT_IMPRECISION);
                else
                    knockbackCounter = 0;
            }

            //Chase Link         
            else
            {
                if(ss < maxStep)
                {
                    counter = (counter + 1)%24;
                    if(counter==0) 
                        ss = Clamp(ss+(maxStep-minStep)/24, minStep, maxStep);
                }
                int angle = Angle(Ghost_X, Ghost_Y, Link->X, Link->Y);
                if(Link->Action == LA_GOTHURTLAND && LinkCollision(ghost))
                    Link->HitDir = AngleDir4(angle);
                if(!Ghost_CanMove(AngleDir8(angle), ss/100, 2)) ss = minStep;
                Ghost_MoveTowardLink(ss/100, 2);
            }
        } while(Ghost_Waitframe(this, ghost, true, true));
    }
}
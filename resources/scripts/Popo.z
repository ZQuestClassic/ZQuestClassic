// import "std.zh"
// import "string.zh"
// import "ghost.zh"


// Wait time between movement
const int POPO_MIN_WAIT_TIME = 90;
const int POPO_MAX_WAIT_TIME = 240;

// Number of jumps before stopping
const int POPO_MIN_JUMPS = 8;
const int POPO_MAX_JUMPS = 12;

ffc script Popo
{
    void run(int enemyID)
    {
        npc ghost;
        int numJumps;
        int waitTime;
        float angle;
        float step;
        int homing;

        // Initialize
        ghost=Ghost_InitAutoGhost(this, enemyID, GHF_NORMAL);
        step=ghost->Step/100;
        homing=ghost->Homing;

        while(true)
        {
            // Wait before moving; slow animation for most of that time
            waitTime=Rand(POPO_MIN_WAIT_TIME, POPO_MAX_WAIT_TIME);
            ghost->ASpeed*=2;
            for(int i=0; i<waitTime; i++)
            {
                if(i==Floor(2*waitTime/3))
                    ghost->ASpeed/=2;
                Ghost_Waitframe(this, ghost, true, true);
            }

            // Pick angle - aim for Link based on homing factor
            if(Rand(256)<homing)
                angle=ArcTan(Link->X-Ghost_X, Link->Y-Ghost_Y)*57.2958;
            else
                angle=Rand(360);

            // Jump across the ground
            numJumps=Rand(POPO_MIN_JUMPS, POPO_MAX_JUMPS);

            for(int i=0; i<numJumps; i++)
            {
                Ghost_Jump=0.5;
                do
                {
                    Ghost_MoveAtAngle(angle, step, 3);
                    Ghost_Waitframe(this, ghost, true, true);
                } while(Ghost_Z>0);

                Ghost_Waitframes(this, ghost, true, true, 12);
            }
        }
    }
}
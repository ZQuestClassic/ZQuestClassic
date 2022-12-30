ffc script Ropa
{
	void run(int enemyID)
	{
		//Init
		npc ghost = Ghost_InitAutoGhost(this, enemyID);
		ghost->Extend = 3;
		Ghost_SetFlag(GHF_KNOCKBACK_4WAY);
		Ghost_SetFlag(GHF_STUN);
		Ghost_SetFlag(GHF_CLOCK);
		Ghost_Transform(this, ghost, -1, -1, -1, 2);
		Ghost_SetHitOffsets(ghost, 8, 8, 0, 0);

		//Declarations
		float step = ghost->Step/100;
		int counter = 0;

		//Spawn Animation
		Ghost_SpawnAnimationPuff(this, ghost);

		//Behavior Loop
		do
		{
			if(Ghost_Z == 0)
			{
				if(counter==0)
				{
					Ghost_Jump = 2;
					counter = Rand( 8)<<4;
				}
				else counter--;
			}
			else if(!Ghost_GotHit()) Ghost_MoveTowardLink(step, 0);
		} while(Ghost_Waitframe(this, ghost, true, true));
	}
}
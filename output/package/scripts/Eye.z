//Eyegore Script
//Attribute 1: The distance in pixels at which the enemy detects Link
//Attribute 2: How long the enemy moves for
//Attribute 3: How long the enemy pauses after moving before moving again
//Attribute 11: The first of 7 combos. Up, Down, Left, Right, Eye Closed, Eye Opening, Eye Open
//Attribute 12: The slot this script is loaded into

ffc script Eyegore{
	void run(int enemyid){
		npc ghost = Ghost_InitAutoGhost(this, enemyid);
		Ghost_SetFlag(GHF_NORMAL);
		Ghost_SetFlag(GHF_REDUCED_KNOCKBACK);
		int DetectDist = Ghost_GetAttribute(ghost, 0, 32);
		int MovementTime = Ghost_GetAttribute(ghost, 1, 240);
		int CooldownPause = Ghost_GetAttribute(ghost, 2, 120);
		int Combo = ghost->Attributes[10];
		int Defenses[18];
		Ghost_StoreDefenses(ghost, Defenses);
		Ghost_Data = Combo + 4;
		Ghost_SetAllDefenses(ghost, NPCDT_BLOCK);
		ghost->Defense[NPCD_FIRE] = NPCDT_IGNORE;
		int Counter = -1;
		while(true){
			//Wait for Link to come into range
			while(Distance(CenterX(ghost), CenterY(ghost), CenterLinkX(), CenterLinkY())>DetectDist){
				Ghost_Waitframe(this, ghost);
			}
			//Open eye and become vulnerable
			Ghost_Data = Combo + 5;
			Ghost_Waitframes(this, ghost, 20);
			Ghost_Data = Combo + 6;
			Ghost_Waitframes(this, ghost, 20);
			Ghost_Data = Combo;
			Ghost_SetFlag(GHF_4WAY);
			Ghost_SetDefenses(ghost, Defenses);
			//Move about like a normal enemy
			for(int i=0; i<MovementTime; i++){
				Counter = Ghost_ConstantWalk4(Counter, ghost->Step, ghost->Rate, ghost->Homing, ghost->Hunger);
				Ghost_Waitframe(this, ghost);
			}
			//Close eye and set defenses back
			Ghost_Data = Combo + 6;
			Ghost_UnsetFlag(GHF_4WAY);
			Ghost_SetAllDefenses(ghost, NPCDT_BLOCK);
			ghost->Defense[NPCD_FIRE] = NPCDT_IGNORE;
			Ghost_Waitframes(this, ghost, 20);
			Ghost_Data = Combo + 5;
			Ghost_Waitframes(this, ghost, 20);
			Ghost_Data = Combo + 4;
			//Pause before opening eye again
			Ghost_Waitframes(this, ghost, CooldownPause);
		}
	}
}
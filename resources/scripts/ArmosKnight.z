// import "std.zh"
// import "string.zh"
// import "ghost.zh"

// Time before coming to life
const int AK_INITIAL_WAIT_TIME = 180;
const int AK_RUMBLE_TIME = 60;

// First phase movement
const int AK_PART_1_WAIT_TIME = 150;
const int AK_PART_1_TOTAL_CIRCLE_TIME = 510;
const int AK_PART_1_REDUCE_RADIUS_TIME = 300;
const int AK_PART_1_EXPAND_RADIUS_TIME = 390;
const int AK_PART_1_STEP = 1;
const int AK_PART_1_ROTATE_SPEED = 1;

// Second phase movement
const int AK_PART_2_WAIT_TIME = 30;
const int AK_PART_2_STEP = 3.5;
const int AK_PART_2_RISE_DIVISOR = 4;
const int AK_PART_2_FALL_STEP = 6;

// ffc->Misc[] indices
const int AK_IDX_LAST_ONE = 0;
const int AK_IDX_PLAY_SOUND = 1;
const int AK_IDX_DYING = 2;

// npc->Attributes[] indices
const int AK_ATTR_NUM_KNIGHTS = 0;
const int AK_ATTR_SECOND_CSET = 1;
const int AK_ATTR_SFX_STOMP = 2;
const int AK_ATTR_SFX_JUMP = 3;
const int AK_ATTR_SFX_RUMBLE = 4;
const int AK_ATTR_EXPLODE = 5;

ffc script ArmosKnights
{
	void run(int enemyID, int knightID, int numKnights, int rowX)
	{
		// The placed enemy is a dummy that acts as a supervisor;
		// it'll set up the rest of the FFCs and give them nonzero knightIDs
		if(knightID==0)
		SupervisorRun(this, enemyID);
		else
		KnightRun(this, enemyID, knightID-1, numKnights, rowX);
	}

	// Supervisor mode ------------------------

	void SupervisorRun(ffc this, int enemyID)
	{
		npc ghost;
		int combo;
		int cset;
		int numKnights;
		int numFFCs;
		int ffcID;
		int numAlive;
		ffc knightFFC[6];
		int startX[6]; // Starting positions
		int startY[6];
		int rowX[6]; // Initial positions when jumping downward in a row
		bool alive[6]; // Whether each knight is still alive
		bool playSound;
		int i;

		ghost=Ghost_InitAutoGhost(this, enemyID);
		ghost->CollDetection=false;
		ghost->SFX=0;

		combo=Ghost_Data;
		cset=Ghost_CSet;
		this->Data=0;

		numKnights=Ghost_GetAttribute(ghost, AK_ATTR_NUM_KNIGHTS, 1, 1, 6);

		// Get FFCs for knights
		numFFCs=0;
		ffcID=0;

		while(numFFCs<numKnights)
		{
			ffcID=FindUnusedFFC(ffcID);

			if(ffcID==0)
			break;

			knightFFC[numFFCs]=Screen->LoadFFC(ffcID);
			numFFCs++;
		}

		// Reduce the number of knights if too few FFCs were available
		numKnights=Min(numKnights, numFFCs);

		// Find where the knights go based on how many there are
		GetStartingPositions(numKnights, startX, startY);
		GetRowPositions(numKnights, rowX);

		// Set up the FFCs; no enemies or scripts yet
		for(i=0; i<numKnights; i++)
		{
			knightFFC[i]->Data=combo;
			knightFFC[i]->CSet=cset;
			knightFFC[i]->TileWidth=2;
			knightFFC[i]->TileHeight=2;
			knightFFC[i]->X=startX[i];
			knightFFC[i]->Y=startY[i];
			knightFFC[i]->Flags[FFCF_OVERLAY]=true;
		}

		// Let the FFCs sit there a moment, then rumble

		Waitframes(AK_INITIAL_WAIT_TIME);

		Game->PlaySound(ghost->Attributes[AK_ATTR_SFX_RUMBLE]);
		for(i=0; i<AK_RUMBLE_TIME; i++)
		{
			for(int j=0; j<numKnights; j++)
			knightFFC[j]->X=startX[j]+Rand(3)-1; // Starting position +/- 1
			Waitframe();
		}

		// Set up the knights' scripts
		numAlive=numKnights;
		for(i=0; i<numKnights; i++)
		{
			knightFFC[i]->Script=this->Script;
			knightFFC[i]->InitD[0]=enemyID;
			knightFFC[i]->InitD[1]=i+1;
			knightFFC[i]->InitD[2]=numKnights;
			knightFFC[i]->InitD[3]=rowX[i];
			alive[i]=true;
		}

		// The knights will move on their own; just watch to see when they die
		while(numAlive>1)
		{
			Waitframe();
			for(i=0; i<numKnights; i++)
			{
				if(!alive[i])
				continue;

				// If a knight is dying, don't count it as dead unless there's at least one other;
				// if they all died at once here, the second phase of the fight wouldn't happen
				if(knightFFC[i]->Misc[AK_IDX_DYING]!=0 && numAlive>1)
				{
					alive[i]=false;
					numAlive--;
				}
				// Check if a landing sound should be played;
				// It's the supervisor that does this so the sound only plays once per jump
				else if(knightFFC[i]->Misc[AK_IDX_PLAY_SOUND]==1)
				playSound=true;
			}

			if(playSound)
			{
				Game->PlaySound(ghost->Attributes[AK_ATTR_SFX_STOMP]);
				playSound=false;
			}
		}

		// Only one is left; tell it to switch behavior
		for(i=0; i<numKnights; i++)
		{
			if(alive[i])
			{
				knightFFC[i]->Misc[AK_IDX_LAST_ONE]=1;
				break;
			}
		}

		// Then just wait for it to die
		knightFFC[i]->Misc[AK_IDX_DYING]=0;
		while(knightFFC[i]->Misc[AK_IDX_DYING]==0)
		Waitframe();

		// And kill the supervisor
		ghost->X=1024;
		ghost->Y=1024;
		ghost->ItemSet=0;
		this->Data=0;
		Quit();
	}

	// Determine the initial position of each knight based on the total number
	void GetStartingPositions(int numKnights, int startX, int startY)
	{
		if(numKnights==1)
		{
			startX[0]=112;
			startY[0]=72;
		}
		else if(numKnights==2)
		{
			startX[0]=64;
			startY[0]=72;

			startX[1]=160;
			startY[1]=72;
		}
		else if(numKnights==3)
		{
			startX[0]=112;
			startY[0]=48;

			startX[1]=160;
			startY[1]=96;

			startX[2]=64;
			startY[2]=96;
		}
		else if(numKnights==4)
		{
			startX[0]=64;
			startY[0]=48;

			startX[1]=160;
			startY[1]=48;

			startX[2]=160;
			startY[2]=96;

			startX[3]=64;
			startY[3]=96;
		}
		else if(numKnights==5)
		{
			startX[0]=112;
			startY[0]=48;

			startX[1]=160;
			startY[1]=48;

			startX[2]=140;
			startY[2]=96;

			startX[3]=80;
			startY[3]=96;

			startX[4]=64;
			startY[4]=48;
		}
		else // 6
		{
			startX[0]=112;
			startY[0]=48;

			startX[1]=160;
			startY[1]=48;

			startX[2]=160;
			startY[2]=96;

			startX[3]=112;
			startY[3]=96;

			startX[4]=64;
			startY[4]=96;

			startX[5]=64;
			startY[5]=48;
		}
	}

	// Determine each knight's X position when jumping in a row
	void GetRowPositions(int numKnights, int rowX)
	{
		// If there's only one, this won't be used, anyway
		if(numKnights==2)
		{
			rowX[0]=64;
			rowX[1]=160;
		}
		else if(numKnights==3)
		{
			rowX[0]=56;
			rowX[1]=112;
			rowX[2]=168;
		}
		else if(numKnights==4)
		{
			rowX[0]=40;
			rowX[1]=88;
			rowX[2]=136;
			rowX[3]=184;
		}
		else if(numKnights==5)
		{
			rowX[0]=32;
			rowX[1]=72;
			rowX[2]=112;
			rowX[3]=152;
			rowX[4]=192;
		}
		else // 6
		{
			rowX[0]=32;
			rowX[1]=64;
			rowX[2]=96;
			rowX[3]=128;
			rowX[4]=160;
			rowX[5]=192;
		}
	}

	// Knight mode ------------------------

	void KnightRun(ffc this, int enemyID, int position, int numKnights, int rowX)
	{
		npc ghost;
		int maxHP;
		int cset2;
		float radius;
		float targetAngle;
		float angleStep;
		float targetX;
		float targetY;
		bool endPart1;
		int i;

		ghost=Ghost_InitCreate(this, enemyID);
		Ghost_SetFlag(GHF_FAKE_Z);
		Ghost_SetFlag(GHF_KNOCKBACK_4WAY);
		Ghost_SetFlag(GHF_REDUCED_KNOCKBACK);
		maxHP=Ghost_HP;

		cset2=ghost->Attributes[AK_ATTR_SECOND_CSET];

		// First stage - other knights still alive
		endPart1=false;
		angleStep=AK_PART_1_ROTATE_SPEED;
		radius=40;

		while(true)
		{
			// Move into position and hold it briefly
			targetAngle=270+(position/numKnights)*360;
			targetX=112+radius*Cos(targetAngle);
			targetY=72+radius*Sin(targetAngle);

			for(i=0; i<AK_PART_1_WAIT_TIME && !endPart1; i++)
			endPart1=AKPart1Waitframe(this, ghost, targetX, targetY);

			if(endPart1)
			break;

			// Jump in a circle for a while
			for(i=0; i<AK_PART_1_TOTAL_CIRCLE_TIME && !endPart1; i++)
			{
				// Change radius at certain points
				if(i==AK_PART_1_REDUCE_RADIUS_TIME)
				radius=24;
				else if(i==AK_PART_1_EXPAND_RADIUS_TIME)
				radius=40;

				// Rotate until the radius has been decreased and restored
				if(i<=AK_PART_1_EXPAND_RADIUS_TIME)
				{
					targetAngle+=angleStep;
					targetX=112+radius*Cos(targetAngle);
					targetY=72+radius*Sin(targetAngle);
				}

				endPart1=AKPart1Waitframe(this, ghost, targetX, targetY);
			}

			if(endPart1)
			break;

			// Line up and jump down the screen
			targetX=rowX;
			targetY=16;
			for(i=0; i<AK_PART_1_WAIT_TIME && !endPart1; i++)
			endPart1=AKPart1Waitframe(this, ghost, targetX, targetY);

			if(endPart1)
			break;

			targetY=112;

			// Moving 112-16=96 pixels, so wait exactly long enough for that
			for(i=96/AK_PART_1_STEP; i>0 && !endPart1; i--)
			endPart1=AKPart1Waitframe(this, ghost, targetX, targetY);

			if(endPart1)
			break;

			// Rotate the opposite direction next time
			angleStep*=-1;
		}

		// Part 1's ended, so this is the last knight; change color, restore HP, change behavior
		Ghost_CSet=cset2;
		Ghost_HP=maxHP;

		// Finish falling...
		while(Ghost_Z>0)
		AKPart2Waitframe(this, ghost, Ghost_X, Ghost_Y);

		// Change Z handling...
		Ghost_SetFlag(GHF_NO_FALL);
		Ghost_UnsetFlag(GHF_FAKE_Z);

		while(true)
		{
			// Move to directly above Link
			targetX=Link->X-8;
			targetY=Link->Y-16;

			Game->PlaySound(ghost->Attributes[AK_ATTR_SFX_JUMP]);

			while(!(Ghost_X==targetX && Ghost_Y==targetY))
			AKPart2Waitframe(this, ghost, targetX, targetY);

			// Hold it for a moment
			for(i=0; i<AK_PART_2_WAIT_TIME; i++)
			AKPart2Waitframe(this, ghost, targetX, targetY);

			// Fall
			while(Ghost_Z>0)
			{
				Ghost_Z-=AK_PART_2_FALL_STEP;
				AKPart2Waitframe(this, ghost, targetX, targetY);
			}

			Game->PlaySound(ghost->Attributes[AK_ATTR_SFX_STOMP]);

			// And wait for a moment before repeating
			for(i=0; i<AK_PART_2_WAIT_TIME; i++)
			AKPart2Waitframe(this, ghost, targetX, targetY);
		}
	}

	// Waitframe used in the first phase of the fight. Returns true if this
	// is the last knight and it's time to start the next phase.
	bool AKPart1Waitframe(ffc this, npc ghost, float targetX, float targetY)
	{
		// Move toward where the knight's supposed to be
		if(Distance(Ghost_X, Ghost_Y, targetX, targetY)<AK_PART_1_STEP)
		{
			Ghost_X=targetX;
			Ghost_Y=targetY;
		}
		else
		{
			float angle=ArcTan(targetX-Ghost_X, targetY-Ghost_Y)*57.2958;
			Ghost_X+=AK_PART_1_STEP*Cos(angle);
			Ghost_Y+=AK_PART_1_STEP*Sin(angle);
		}

		// If on the ground, jump and tell the supervisor to play the bounce sound
		if(Ghost_Z==0)
		{
			Ghost_Jump=2;
			this->Misc[AK_IDX_PLAY_SOUND]=1;
		}
		else if(this->Misc[AK_IDX_PLAY_SOUND]==1)
		this->Misc[AK_IDX_PLAY_SOUND]=0;

		// Don't automatically clear and quit on death; if the last two died
		// at the same time, the second phase of the battle wouldn't happen
		if(!Ghost_Waitframe(this, ghost, false, false))
		{
			// Wait a frame to let the supervisor check
			this->Misc[AK_IDX_DYING]=1;
			Ghost_HP=1;
			Ghost_Waitframe(this, ghost, true, true);

			// If this isn't the last knight, it can die now
			if(this->Misc[AK_IDX_LAST_ONE]==0)
			{
				Ghost_HP=0;
				Ghost_Waitframe(this, ghost, true, true);
			}
			else
			return true;
		}

		return this->Misc[AK_IDX_LAST_ONE]!=0;
	}

	// Waitframe used in the second phase of the fight.
	void AKPart2Waitframe(ffc this, npc ghost, float targetX, float targetY)
	{
		if(Ghost_X!=targetX || Ghost_Y!=targetY)
		{
			float dist=Distance(Ghost_X, Ghost_Y, targetX, targetY);

			if(dist<AK_PART_2_STEP)
			{
				Ghost_X=targetX;
				Ghost_Y=targetY;
				Ghost_Z+=Sqrt(dist)/AK_PART_2_RISE_DIVISOR;
			}
			else
			{
				float angle=ArcTan(targetX-Ghost_X, targetY-Ghost_Y)*57.2958;
				Ghost_X+=AK_PART_2_STEP*Cos(angle);
				Ghost_Y+=AK_PART_2_STEP*Sin(angle);
				Ghost_Z+=Sqrt(dist)/AK_PART_2_RISE_DIVISOR;
			}
		}

		if(!Ghost_Waitframe(this, ghost, ghost->Attributes[AK_ATTR_EXPLODE]==0,
					ghost->Attributes[AK_ATTR_EXPLODE]==0))
		{
			this->Misc[AK_IDX_DYING]=1;
			Ghost_DeathAnimation(this, ghost, ghost->Attributes[AK_ATTR_EXPLODE]);
			Quit();
		}
	}
}
//Creates a cucco that hops around at random directions
//REQUIRES GHOST.ZH TO USE
//
//This script is designed to use the AutoGhost feature
//To use this, set up the various direction combos for the cucco in your combo page as follows
//--UP/LEFT,DOWN/LEFT,UP/RIGHT,DOWN/RIGHT--
//
//Note the first combo of that list
//
//Next, set up an enemy with the following attributes
//Type - Other
//Misc 1 - Minimum times the cucco will hop
//Misc 2 - Maximum times the cucco will hop
//Misc 3 - Jump Height (Reccomended - 10)
//Misc 4 - Max Speed (Reccomended - 5)
//Misc 5 - Minimum pause time betweeen each hop phase)
//Misc 6 - Maximum pause time betweeen each hop phase)
//Misc 11 - Combo for UP/LEFT direction of the cucco
//Misc 12 - The script slot the cucco script is in

ffc script Cucco{
	void run(int enemyID){
	
		npc ghost = Ghost_InitAutoGhost(this,enemyID); //Start the ghost enemy
		ghost->CollDetection = false; //Turn off collisions
		Ghost_SpawnAnimationPuff(this,ghost); //Make the poof appear for enemy spawning
		
		int jumpCounter; //Used to indicate how many times the cucco has jumped
		int counter = 1;
		
		int jumpNumMin = ghost->Attributes[0]; //Set the min ammount of times it will hop
		int jumpNumMax = ghost->Attributes[1]; //Set the max ammount of times it will hop
		int jumpHeight = ghost->Attributes[2] * 0.1; //Set the jump height
		int speed = ghost->Attributes[3] * 0.1; //Set the max speed
		int pauseMin = ghost->Attributes[4]; //Minimum pause time after hopping
		int pauseMax = ghost->Attributes[5]; //Max pause time after hopping
		
		int origCombo = Ghost_Data; //Store the original combo
		int pause = Rand(pauseMin,pauseMax); //Used to store pause
		float jump = Rand(jumpNumMin,jumpNumMax); //Used to store how many jumps
				
		bool move = true; //Used to tell the cucco to move or not
		bool random = false; //Used to see if the velocity has been randomized
		
		Ghost_SetFlag(GHF_FAKE_Z);
		
		while(true){
			//Change combo according to speed
			//Right Down
			if(Ghost_Vx > 0 && Ghost_Vy > 0) Ghost_Data = origCombo + 3;
			//Right Up
			else if(Ghost_Vx > 0 && Ghost_Vy < 0) Ghost_Data = origCombo + 2;
			//Left Down
			else if(Ghost_Vx < 0 && Ghost_Vy > 0) Ghost_Data = origCombo + 1;
			//Left Up
			else if(Ghost_Vx < 0 && Ghost_Vy < 0) Ghost_Data = origCombo;
				
			if(move){
				//Randomize the cucco movement
				if(!random){
					Ghost_Vx = Rand(-1*speed,(speed));
					Ghost_Vy = Rand(-1*speed,(speed));
					random = true;
				}
				//If the cucco is on the ground and hasn't jumped so many times,
				//Add more Z velocity
				if(Ghost_Z == 0 && jumpCounter <= jump){
					Ghost_Jump = jumpHeight;				
					jumpCounter++;
				}
				//If it HAS jumped three times, time to pause
				else if(jumpCounter == jump){
					jumpCounter = 0;
					move = false;
					random = false;
					Ghost_Vx = 0;
					Ghost_Vy = 0;
					pause = Rand(pauseMin,pauseMax);
				}
			}
			else{
				//Once paused, it'll wait a little before moving again
				counter++;
				if(counter%pause == 0){
					move = true;
					jump = Rand(jumpNumMin,jumpNumMax);
				}
			}
			Ghost_Waitframe(this,ghost,true,true);
		}
	}
}
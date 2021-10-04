//////////////////////////////////
/// BobOmb for 2.55            ///
/// By: ZoriaRPG               ///
/// 5th May, 2019              ///
/// v0.3                       ///
//////////////////////////////////

//v0.2 - Fixed a typo in isValid()
//v0.3 - Trivial performance edit on timer. 

//const int NPC_BOBOMB = 179; //Moved to bobomb.ID

//Use on a walking enemy with every defence set to Stun
npc script bobomb
{
	const int TIMER 	= 14; //Misc Index to store Timer
	const int CSET 		= 13; //Misc Index to store CSet
	const int FLASH_CSET 	= 8; //Some defaults. IDK if I should auto-assign them.
	const int DURATION 	= 120; //Some defaults. IDK if I should auto-assign them.
	const int ID 		= 179; //The enemy ID. Call as bobomb.ID elsewhere, if needed. 
	//Misc[] index of timer, misc index to store base cset, value fo flash cset, duration of timer
	void run(int misc_timer, int misc_cset, int flash_cset)
	{
		itemdata id = Game->LoadItemData(I_BOMB); 
		int pow = (this->WeaponDamage > 0) ? (this->WeaponDamage) : id->Power;
		
		//Auto-assign if the user forgot to set them.
		misc_timer = ( misc_timer > 0 ) ? misc_timer : TIMER;
		misc_cset = ( misc_cset > 0 ) ? misc_cset : CSET;
		flash_cset = ( flash_cset > 0 ) ? flash_cset : FLASH_CSET;
		this->Misc[misc_timer] = -1;
		while(this->isValid())
		{
			if ( this->Stun > 1 ) 
			{
				//check its timer
				
				switch(this->Misc[misc_timer])
				{
					case -1: 
					{
						//uninitialised
						this->Misc[misc_timer] = 0;
						continue; //initialise and move on to case 0. 
						//This is to ensure that the effect does not repeat. 
					}
					case 0:
					{
						//its not yet flashing, so set its timer.
						this->Misc[misc_timer] = DURATION;  //This is the duration of the flash, not the explosion. 
						this->Misc[misc_cset] = this->CSet;
						break;
					}
					default:
					{
						--this->Misc[misc_timer];
						if ( !(this->Misc[misc_timer] % 5) ) 
						{
							//Make it flash
							this->CSet = flash_cset;
						}
						else this->CSet = this->Misc[misc_cset];
						break;
					}
				}
				
			}
			if ( this->Stun == 1 ) 
			{
				//time to explode
				eweapon e = Screen->CreateEWeapon(EW_SBOMBBLAST);
				e->Damage = pow;
				e->X = this->X; e->Y = this->Y;
				this->HP = -9999;
			}
			
			Waitframe();
		}
	}
}


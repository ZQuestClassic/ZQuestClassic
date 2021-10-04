/////////////////////////////////////////////////////
/// Daira NPC Script for 2.55                     ///
/// v0.3                                          ///
/// 7th Febryary, 2021                            ///
///                                               ///
/// Original Daira by: Tamamo, and NightmareJames ///
/// This version, by: ZoriaRPG                    ///
/////////////////////////////////////////////////////
npc script DairaNPC
{
	void run()
	{

		//Original Tile
		int OTile = this->OriginalTile;

		//Movement Variables
		float counter = -1;
		int step = this->Step;
		int rate = this->Rate;
		int homing = this->Homing;
		int hunger = this->Hunger;
		
		//Weapon Variables
		eweapon wpn;
		int damage = this->WeaponDamage;
		int attackTimer;
		int attackWait; //Time between swings. 
		bool throw = (this->BFlags[4]); //Behavioural FLag 4
		//bool shielded = this->Attributes[0]&4;
		int wSprite = this->WeaponSprite; //Ghost_GetAttribute(ghost,1,-1);

		//Behavior
		while(this->isValid())
		{

			//Attacking
			if(!attackTimer) //Attacks when Link is in Range, and within Line of Sight
			{
				if ( this->LinkInRange(80) )
				{
					bool LoS = false;
					switch(this->Dir)
					{
						
						case DIR_UP:
						{
							if ( Link->Y > this->Y ) break; //Facing Link
							if ( Abs(Link->X - this->X) <= 16 ) LoS = true; break;
						}
						case DIR_DOWN:
						{
							if ( Link->Y < this->Y ) break;
							if ( Abs(Link->X - this->X) <= 16 ) LoS = true; break;
						}
						case DIR_LEFT:
						{
							if ( Link->X > this->X ) break;
							if ( Abs(Link->Y - this->Y) <= 16 ) LoS = true; break;
						}
						case DIR_RIGHT:
						{
							if ( Link->X < this->X ) break;
							if ( Abs(Link->Y - this->Y) <= 16 ) LoS = true; break;
						}
						default: break;
					}
						
					if ( LoS )
					{
						attackWait--;
						if(attackWait<=0)
						{
							attackTimer = 21;
							attackWait = 60;
						}
					}
				}
			}
			if(attackTimer>0) //Attacking?
			{
				if(this->OriginalTile == OTile)
				{
					wpn=Screen->CreateEWeapon(EW_BEAM);
					wpn->Dir = this->Dir;
					wpn->Step = this->Step;
					wpn->UseSprite(wSprite);
					wpn->Damage=damage;
					wpn->Angular=false;
					wpn->Script = 1; //daira axe
					wpn->CSet = this->CSet;
				}
				else if(!wpn->isValid())
				{
					this->OriginalTile = OTile;
					Waitframe();
					attackTimer = 0;
					continue;
				}
				this->OriginalTile = OTile + 20*Div(28-attackTimer, 7);
				wpn->Tile = this->OriginalTile+this->Dir+16;
				wpn->X = this->X;
				wpn->Y = this->Y;
				wpn->DeadState = WDS_ALIVE;
				if(attackTimer>14)
				{
					if((this->Dir == DIR_UP || this->Dir == DIR_DOWN))
						wpn->Y -= 16;
					else wpn->X -= InFrontX(this->Dir, 0);
				}
				else if(attackTimer>7)
				{
					if(this->Dir == DIR_UP || this->Dir == DIR_DOWN)
					{
						if(this->Dir == DIR_UP )
							wpn->Y -= 16;
					}
					else
					{
						wpn->X += InFrontX(this->Dir, 0);
					}
				}
				else
				{
					wpn->X += InFrontX(this->Dir,0);
					wpn->Y += InFrontY(this->Dir,0);
				}
				//the weapon hit boxes are small.
				SetWpnHitOffsets(wpn, Div(28-attackTimer, 7), this->Dir);
				attackTimer--;
			}
			else if(wpn->isValid())
			{
				this->OriginalTile = OTile;
				if(throw)
				{
					wpn=Screen->CreateEWeapon(EW_BEAM);
					wpn->Dir = this->Dir;
					wpn->Step= 200;
					wpn->UseSprite(wSprite);
					wpn->Damage=damage;
					wpn->Angular=false;
					wpn->Script = 1; //daira axe
					
					this->OriginalTile = this->OriginalTile + 80 + (this->Dir*4);
				}
				else wpn->DeadState = WDS_DEAD;
			}
			if(this->HP <= 0)
			{
				wpn->DeadState = WDS_DEAD;
				Quit();
			}
			Waitframe();
		}
	}
	void SetWpnHitOffsets(eweapon wpn, int frame, int dir)
	{
		//Set Default Collision Properties
		wpn->CollDetection = true;
		wpn->HitXOffset = 0;
		wpn->HitYOffset = 0;
		wpn->HitWidth = 16;
		wpn->HitHeight = 16;

		//Modify it based off attack frame.
		switch(frame)
		{
			case 1:
			{
				if(dir == DIR_UP || dir == DIR_DOWN)
				{
					wpn->CollDetection = false;
					return;
				}
				else
					wpn->HitWidth = 8;
				if(dir == DIR_RIGHT)
					wpn->HitXOffset = 8;
				break;
			}
			case 2:
			{
				if(dir == DIR_UP || dir==DIR_DOWN)
				{
					wpn->HitHeight = 8;
					return;
				}
				else
					wpn->HitWidth = 8;
				if(dir == DIR_LEFT)
				wpn->HitXOffset = 8; 
				break;
			}
			case 3:
			{
				wpn->HitXOffset = Cond(dir==DIR_LEFT, 8, 0);
				wpn->HitYOffset = Cond(dir==DIR_UP, 8, 0);
				wpn->HitWidth = Cond(dir==DIR_LEFT||dir==DIR_RIGHT, 8, 16);
				wpn->HitHeight = Cond(dir==DIR_UP||dir==DIR_DOWN, 8, 16);
				break;
			}
			default: //Out of range.
			{
				wpn->DeadState = WDS_DEAD;
				return;
			}
		}
	}
}

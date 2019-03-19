//Spawn A Weapon on the Death of Another Weapon
//'Replace on Death'
//By ZoriaRPG
//19th March, 2019

//v0.1 : Initial
//v0.2 : Fixed weapon orientations.


lweapon script ReplaceOnDeath
{
	void run(int newWeaponType, int requireItem, int requireCounter, int CounterAmount, bool flipOnDeath, bool newDirOnDeath, int newDir, int newScript) //could be flags here
	{
		
		itemdata bomb; bool found; bool super; int x; int y; int pow;
		if ( requireItem > 0 )
		{
			bomb = Game->LoadItemData(Link->ItemA);
			if ( bomb->ID == requireItem ) found = true;
			if ( !found )
			{
				bomb = Game->LoadItemData(Link->ItemB);
				if ( bomb->Family == IC_BOMB ) { found = true; }
			}
			if ( !found ) Quit(); //quit if bombs not in a slot
		}
		
		pow = bomb->Power;
		if ( requireCounter )
		{
			if ( Game->Counter[requireCounter] < CounterAmount ) Quit(); //quit if not enough bombs
				else Game->Counter[requireCounter] -=CounterAmount;
		}
		
		
		while(this->isValid()) 
		{ 
			x = this->X; y = this->Y; 
			
			if ( this->DeadState != WDS_ALIVE ) //When the arrow itself dies, make an explosion. 
			{
				lweapon boom = Screen->CreateLWeapon(newWeaponType);
				boom->Damage = pow;
				boom->X = x;
				boom->Y = y;
				boom->Step = this->Step;
				boom->Dir = this->Dir;
				if ( flipOnDeath ) { boom->Dir = reversedirection(this->Dir); boom->Flip = AdjustFlip(this); } //ReflectWeapon(boom, this->Angular);
				else if ( newDirOnDeath ) { boom->Dir = newDir; boom->Flip = AdjustFlip(this); }
				boom->Damage = this->Damage; //need a new damage attribute; reduce param count to accomplish this
				if ( Abs(newScript) < 511 ) boom->Script = newScript;
				Quit(); //Spawn only one weapon. 
			}
			
			Waitframe(); 
		}
		
	}
	//Returns the reverse f any of the eight cardinal directions.
	int reversedirection(int dir) 
	{
		switch(dir)
		{
			case DIR_UP: return DIR_DOWN; break;
			case DIR_DOWN: return DIR_UP; break;
			case DIR_LEFT: return DIR_RIGHT; break;
			case DIR_RIGHT: return DIR_LEFT; break;
		}
		//if ( dir == DIR_LEFT) return DIR_RIGHT;
		//if ( dir == DIR_DOWN) return DIR_UP;
		//if ( dir == DIR_UP) return DIR_DOWN;
		//if ( dir == DIR_RIGHT) return DIR_LEFT;
		///if ( dir == DIR_LEFTUP) return DIR_RIGHTDOWN;
		//if ( dir == DIR_RIGHTDOWN) return DIR_LEFTUP;
		//if ( dir == DIR_LEFTDOWN) return DIR_RIGHTUP;
		//if ( dir == DIR_RIGHTUP) return DIR_LEFTDOWN;
	}
	/*
	int FlipRev(lweapon l){
		if ( l->Dir == DIR_DOWN || l->Dir == DIR_UP && !l->Flip ) return 2;
		if ( l->Dir == DIR_DOWN || l->Dir == DIR_UP && l->Flip ) return 0; 
		if ( l->Dir == DIR_LEFT || l->Dir == DIR_RIGHT && !l->Flip ) return 1;
		if ( l->Dir == DIR_LEFT || l->Dir == DIR_RIGHT && l->Flip ) return 0;
		return -1;
	}
	*/
	int AdjustFlip(lweapon l)
	{
		switch(l->Dir)
		{
			case DIR_DOWN: return 0; break;
			case DIR_UP: return 2; break;
			case DIR_LEFT: return 4; break;
			case DIR_RIGHT: return 7; break;
		}
	}
	lweapon ReflectWeapon(lweapon l, bool angular)
	{
		l->Dir = reversedirection(l->Dir);
		//l->Flip = FlipRev(l);
		l->Angular = angular;
		if ( angular ) 
		{
			if (l->Angle >= 180 ) l->Angle -= 180;
			else l->Angle += 180;
		}
	}
}
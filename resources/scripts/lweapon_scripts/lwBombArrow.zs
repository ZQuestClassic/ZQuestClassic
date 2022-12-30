//Bomb Arrow Script for 2.55
//v0.2
//By ZoriaRPG
//19th March, 2019

//v0.1 : Initial
//v0.2 : Fix comppile, and fix how we check if the arrow is dead to create the explosion.
//     : We cannot run the explosion after while(this->isValid()), because the script runs out of scope if the
//     : sprite is no longer on the screen. Thus, we check its deadstate, while it is valid. 

lweapon script bombarrow
{
	void run()
	{
		
		itemdata bomb; bool found; bool super; int x; int y; int pow;
		bomb = Game->LoadItemData(Link->ItemA);
		if ( bomb->Family == IC_BOMB ) found = true;
		if ( bomb->Family == IC_SBOMB ) { found = true; super = true; }
		if ( !found )
		{
			bomb = Game->LoadItemData(Link->ItemB);
			if ( bomb->Family == IC_BOMB ) { found = true; }
			if ( bomb->Family == IC_SBOMB ) { found = true; super = true; }
		}
		if ( !found ) Quit(); //quit if bombs not in a slot
		else
		{
			pow = bomb->Power;
			if ( super )
			{
				if ( !Game->Counter[CR_SBOMBS] ) Quit(); //quit if not enough bombs
				else --Game->Counter[CR_SBOMBS];
			}
			else
			{
				if ( !Game->Counter[CR_BOMBS] ) Quit(); //quit if not enough bombs
				else --Game->Counter[CR_BOMBS];
			}
		}
		
		
		while(this->isValid()) 
		{ 
			x = this->X; y = this->Y; 
			
			if ( this->DeadState != WDS_ALIVE ) //When the arrow itself dies, make an explosion. 
			{
				lweapon boom = Screen->CreateLWeapon( (( super ) ? LW_SBOMBBLAST : LW_BOMBBLAST ));
				boom->Damage = pow;
				boom->X = x;
				boom->Y = y;
			}
			
			Waitframe(); 
		}
		
	}
}
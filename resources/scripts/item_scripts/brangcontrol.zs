item script brangcontrol
{
	void run()
	{
		lweapon child; currentbutton;
		for ( int q = Screen->NumLWeapons(); q > 0; --q ) //match the child weapon of this item to it
		{
			lweapon temp = Screen->LoadLWeapon(q);
			if ( temp->Parent == this->ID )
			{
				child = temp; break;
			}
		}
		if ( !child ) Quit();
		while(child->isValid())
		{
			if ( Hero->ItemA == this->ID ) 
			{
				currentbutton = CB_A;
			}
			else if ( Hero->ItemB == this->ID ) 
			{
				currentbutton = CB_B;
			}
			else currentbutton = 0;
			
			if ( currentbutton && Input->Button[currentbutton] )
			{
				//holding down the item button
				if ( Input->Button[CB_UP] ) 
				{
					if ( Input->Button[CB_LEFT] ) 
					{
						child->Dir = DIR_LEFTUP;
					}
					else if ( Input->Button[CB_RIGHT] ) 
					{
						child->Dir = DIR_RIGHTUP;
					}
					else child->Dir = DIR_UP;
				}
				else if ( Input->Button[CB_DOWN] ) 
				{
					if ( Input->Button[CB_LEFT] ) 
					{
						child->Dir = DIR_LEFTDOWN;
					}
					else if ( Input->Button[CB_RIGHT] ) 
					{
						child->Dir = DIR_RIGHTDOWN;
					}
					else child->Dir = DIR_DOWN;
			`	}
				else if ( Input->Button[CB_RIGHT] )  	//'else if', so that we don't clash. 
				{					//Won't conflict with combining with up or down.
					child->Dir = DIR_RIGHT;
				}
				else if ( Input->Button[CB_LEFT] )  	//'else if', so that we don't clash
				{					//Won't conflict with combining with up or down.
					child->Dir = DIR_LEFT;
				}
			}
			Waitframe();
		}
	}
}
			
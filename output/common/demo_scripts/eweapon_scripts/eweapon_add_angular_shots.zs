eweapon script addtwoangularshots
{
	void run(bool shieldcanblock)
	{
		int dirs[2];
		switch(this->Dir)
		{
			case DIR_UP:
			{
				dirs[0] = DIR_LEFTUP;
				dirs[1] = DIR_RIGHTUP;
				break;
			}
			case DIR_DOWN:
			{
				dirs[0] = DIR_LEFTDOWN;
				dirs[1] = DIR_RIGHTDOWN;
				break;
			}
			case DIR_LEFT:
			{
				dirs[0] = DIR_LEFTDOWN;
				dirs[1] = DIR_LEFTUP;
				break;
			}
			case DIR_RIGHT:
			{
				dirs[0] = DIR_RIGHTDOWN;
				dirs[1] = DIR_RIGHTUP;
				break;
			}
			case DIR_RIGHTUP:
			{
				dirs[0] = DIR_RIGHT;
				dirs[1] = DIR_UP;
				break;
			}
			case DIR_RIGHTDOWN:
			{
				dirs[0] = DIR_RIGHT;
				dirs[1] = DIR_DOWN;
				break;
			}
			case DIR_LEFTUP:
			{
				dirs[0] = DIR_LEFT;
				dirs[1] = DIR_UP;
				break;
			}
			case DIR_LEFTDOWN:
			{
				dirs[0] = DIR_LEFT;
				dirs[1] = DIR_DOWN;
				break;
			}
			default: 
			{
				printf("Invalid Dir\n");
				Quit();
			}
		}
		eweapon shots[2];
		for ( int q = 0; q < 2; ++q )
		{
			shots[q] = Duplicate(this);
			shots[q]->Script = 0; //Don't split infinitely.
			if ( this->Angular )
			{
				switch (q)
				{
					case 0:
					shots[q]->Angle = WrapDegrees(this->Angle-45); break;
					case 1:
					shots[q]->Angle = WrapDegrees(this->Angle+45); break;
				}
				
			}
			
			else shots[q]->Dir = dirs[q];
			//unless ( blockable && (this->Level&1)) shots[q]->Level |= 1;
			//unless ( blockable ) shots[q]->Level |= 1;
			//else shots[q]->Level &= ~1;
			if ( (this->Level&1) )
			{
				if ( shieldcanblock ) shots[q]->Level &= ~1;
			}
		}
	}
}
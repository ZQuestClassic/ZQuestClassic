//Zig-Zag Weapon pattern.
//ZoriaRPG, 18th March, 2019
//v0.2

//v0.1 : Initial
//v0.2 : Added pixels in distance of each movement as InitD[2]
eweapon script eZigZagwhoneedssine
{
	void run(int dist, int speed, int px)
	{
		int frame = 1;
		int baseline;
		dist = (dist < 1) ? 12 : dist;
		speed = (speed < 1) ? 1 : speed;
		px = ( px < 1 ) ? 1 : px;
		//initial direction from baseline
		bool initial = ( Rand(0,1) ) ? true : false;
		switch(this->Dir)
		{
			case DIR_UP: baseline = this->X+(this->TileWidth*0.5); break;
			case DIR_LEFT: baseline = this->Y+(this->TileHeight*0.5); break;
			case DIR_DOWN: baseline = this->X+(this->TileWidth*0.5)+1; break;
			case DIR_RIGHT: baseline = this->Y+(this->TileHeight*0.5)+1; break;
				
		}
		while(this->isValid())
		{
			++frame;
			switch(this->Dir)
			{
				case DIR_UP:
				case DIR_DOWN:
				{
					if ( initial )
					{
						if ( !(frame%speed) )
						{
							if ((baseline - this->X) < dist ) 
							{
								this->X-=px;
							}
							else initial = !initial;
						}
					}
					else
					{
						if ( !(frame%speed) )
						{
							if ( (this->X - baseline) < dist ) 
							{ 
								this->X+=px;
							}
							else initial = !initial;
						}
					}
					break;	
				}
				
				case DIR_LEFT:
				case DIR_RIGHT:
				{
					if ( initial )
					{
						if ( !(frame%speed) )
						{
							if ((baseline - this->Y) < dist ) 
							{
								this->Y-=px;
							}
							else initial = !initial;
						}
					}
					else
					{
						if ( !(frame%speed) )
						{
							if ( (this->Y - baseline) < dist ) 
							{ 
								this->Y+=px;
							}
							else initial = !initial;
						}
					}
					break;	
				}
			}
			Waitframe();
		}
	}
}
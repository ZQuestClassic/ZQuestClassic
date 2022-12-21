lweapon script lZigZagwhoneedssine
{
	void run(int dist, int speed)
	{
		int frame = 1;
		int baseline;
		dist = (dist < 1) ? 12 : dist;
		speed = (speed < 1) ? 1 : speed;
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
								--this->X;
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
								++this->X;
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
								--this->Y;
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
								++this->Y;
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
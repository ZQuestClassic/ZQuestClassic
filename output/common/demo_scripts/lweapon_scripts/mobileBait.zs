lweapon script MovingBait
{
	void run(int minTime, int maxTime, int minDist, int maxDist, int step_delay)
	{
		int clk = Rand(minTime, maxTime);
		int dist; int dir;
		
		minTime = ( minTime > 0 ) ? minTime : 10;
		maxTime = ( maxTime > 0 ) ? maxTime : 100;
		minDist = ( minDist > 0 ) ? minDist : 8;
		maxDist = ( maxDist > 0 ) ? maxDist : 30;
		step_delay = ( step_delay > 0 ) ? step_delay : 2;
		
		while(this->isValid())
		{
			--clk;
			if ( clk < 1 )
			{
				clk = Rand(minTime, maxTime);
				dist = Rand(minDist, maxDist);
				dir = Rand(0,3);
				//move the bait
				for ( int q = dist; q > 0; --q )
				{
					switch(dir)
					{
						case DIR_UP: --this->Y; break;
						case DIR_DOWN: ++this->Y; break;
						case DIR_LEFT: --this->X; break;
						case DIR_RIGHT: ++this->X; break;
					}
					Waitframes(step_delay);
				}
			}
			Waitframe();
		}
	}
}
				
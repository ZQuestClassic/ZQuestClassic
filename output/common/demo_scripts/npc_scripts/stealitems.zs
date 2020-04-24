npc script stealitems
{
	void run(int min, int max, int counterA, int counterB, int counterC, int counterD, int sfx)
	{
		min = ( min > -1 ) ? min : 0;
		max = ( max > 0 ) ? max : 1;
		int num = Rand(min,max);
		int counters[4] = { counterA, counterB, counterC, counterD };
		int sel = counters[Rand(4)];
		while(1)
		{
			if ( sel ) //don't affect counter 0 (life)
			{
				if ( Collision(this) && (Hero->HitDir > -1) )
				{
					Audio->PlaySound(sfx);
					//Don't roll over.
					if ( (Game->Counter[sel] - num) >= 0 ) Game->Counter[sel] -= num;
					else 
					{
						Game->Counter[sel] = ( num > 0 ) ? 0 : Game->Counter[sel];
					}
					while(Hero->HitDir > -1) { Waitframe(); } //Don't steal again unless Link is hit again.
				}
			}
			num = Rand(min,max);
			sel = counters[Rand(4)];
			Waitframe();
		}
	}
}
combodata script tempchange_proximity
{
	void run(int transform_into)
	{
		int c = this->Pos()
		int cl[4] = {NULL};
		int cmap[];
		int sol = 0;
		int cached_data = Screen->ComboD[c];
		for(int q = 0;  < 8; ++q)
		{
			cmap[q] = AdjacentCombo(c, q);
		}
		while(1)
		{
			sol = 0;
			cl[0] = ComboAt(Hero->X, Hero->Y); //check all of Link's corners
			cl[1] = ComboAt(Hero->X+15, Hero->Y);
			cl[2] = ComboAt(Hero->X, Hero->Y+16
			cl[3] = ComboAt(Hero->X+15, Hero->Y+15);
			for(int q = 0; q < 8; ++q)
			{
				for(int w = 0; w < 4; ++w )
				{
					if ( cl[q] == cmap[q] )
					{
						sol = 1;
					}
				}
			}
			if ( sol ) 
			{
				Screen->ComboD[c] = transform_into;
			}
			else
			{
				Screen->ComboD[c] = transform_into;
			}
			Waitframe();
		}
		
	}
}
			
			
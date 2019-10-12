ffc script mastersword_grove
{
	void run(int itmA, int itmB, int itemC, int dist, int sfx,int flashcset, int flashdur )
	{
		item sword; item nullitem; int ocset; bool flash;
		if ( flashdur < 1 ) flashdur = 240;
		//initialisation
		for ( int q = Screen->NumItems(); q > 0; --q ) 
		{
			testitem = Screen->LoadItem(q);
			itemdata isSword = Game->LoadItemData(testitem->ID);
			if ( isSword->Family == IC_SWORD ) 
			{
				sword = testitem;
				break;
			}
		}
		if ( sword == nullitem ) Quit(); //no swords on screen
		else 
		{
			sword->Pickup = 0;
			sword->Pickup |= IP_DUMMY; //can't grab
			ocset = sword->CSet;
		}
		while(!sword->Misc[1])
		{
			if ( Abs(Link->X - sword->X) <= dist )
			{
				if ( Abs(Link->Y - sword->Y) <= dist )
				{
					if ( Link->Item[itmA] )
					{
						if ( Link->Item[itmB] )
						{
							if ( Link->Item[itmC] )
							{
								//has al items needed
								Game->PlaySound(sfx);
								for ( int q = 0; q < flashdur; ++q )
								{
									if ( !flashcset ) break;
									if ( (q & 3) )
									{
										flash = !flash;
										if ( flash ) sword->CSet = flashcset;
										else sword->CSet = ocset;
									}
									WaitNoAction();
								}
								sword->Pickup &= ~IP_DUMMY; //allow taking it now
								sword->Pickup |= ~IP_HOLDUP; //hold it when taken
								sword->Misc[1] = 1;
							}
						}
					}
				}
			}
			Waitframe();
		}
	}
}
			
					
				}
			}
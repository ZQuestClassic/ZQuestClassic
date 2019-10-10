///////////////////////////////////////
/// Very Simple 'Master Sword Grove ///
/// v1.0 - 10th october, 2019       ///
/// By: ZoriaRPG                    ///
///////////////////////////////////////
// D0 : 1st pendant Item ID
// D1 : 2nd pendant Item ID
// D2 : 3rd pendant Item ID
// D3 : Proximity, in pixels befre checking pendants.
// D4: Sound to play if hero has all pendants and can pick up sword. 
// D5: CSet to flash sword.
// D6: Duration of flash. 
// D7: Screen D register
ffc script mastersword_grove
{
	void run(int itmA, int itmB, int itemC, int dist, int sfx,int flashcset, int flashdur )
	{
		if ( (Screen->D[register]&1) ) Quit(); //Already did animation and unlocked sword. 
		item sword; item nullitem; int ocset; bool flash;
		item pends[3];
		pends[0] = Screen->CreateItem(itmA); 
		pends[1] = Screen->CreateItem(itmA); 
		pends[2] = Screen->CreateItem(itmA); 
		
		pends[0]->Pickup |= IP_DUMMY;
		pends[1]->Pickup |= IP_DUMMY;
		pends[2]->Pickup |= IP_DUMMY;
		
		pends[0]->X = 1;
		pends[1]->X = 1;
		pends[2]->X = 1;
		pends[0]->Y = 1;
		pends[1]->Y = 1;
		pends[2]->Y = 1;
		
		pends[0]->DrawYOffset = -32768;
		pends[1]->DrawYOffset = -32768;
		pends[2]->DrawYOffset = -32768;
		
		
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
								//show pendants
								pends[0]->X = sword->X;
								pends[1]->X = sword->X-30;
								pends[2]->X = sword->X+30;
								pends[0]->Y = sword->Y-30
								pends[1]->Y = sword->Y;
								pends[2]->Y = sword->Y;
								
								pends[0]->DrawYOffset = 0;
								pends[1]->DrawYOffset = 0;
								pends[2]->DrawYOffset = 0;
								for ( int q = 0; q < flashdur; ++q )
								{
									if ( !flashcset ) break;
									if ( (q & 3) )
									{
										flash = !flash;
										if ( flash ) sword->CSet = flashcset;
										else sword->CSet = ocset;
									}
									if ( (q%2) )
									{
										//move pendants
										--pends[0]->Y;
										--pends[1]->X;
										++pends[1]->X;
									}
									WaitNoAction();
								}
								sword->Pickup &= ~IP_DUMMY; //allow taking it now
								sword->Pickup |= ~IP_HOLDUP; //hold it when taken
								
								Remove(pends[0]);
								Remove(pends[1]);
								Remove(pends[2]);
								Screen->D[register] |= 1;
								
								sword->Misc[1] = 1; //exit loop
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
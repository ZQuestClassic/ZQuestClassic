#include "std.zh"
hero script kb_riddle
{
	const int SFX_WON = 20;
	const int SFX_LOSS = 28;
	const int BUFFER_SIZE = 20;
	void run()
	{

		int StringBuffer[BUFFER_SIZE];
		int lower[BUFFER_SIZE];
		TraceS("Riddle Game String Processor:\n");
		Game->TypingMode = true;
		int index; int won;

		while(1)
		{
			while(!Input->Key[KEY_ENTER])
			{
				for(int i = KEY_A; i<=KEY_SPACE; ++i)
				{

					if(Input->ReadKey[i])
					{
						if(i == KEY_BACKSPACE && index > 0)
						{
							--index;
							StringBuffer[index] = 0;
						}
						else
						{
							if ( index < ( BUFFER_SIZE-1 ) )
							{
								LogPrint("KeyToChar is: %d \n", KeyToChar(i));
								StringBuffer[index] = KeyToChar(i);
								++index;
							}
						}

						TraceS(StringBuffer);
					}
				}
				Screen->DrawString(6, 8, 20, 2, 1, -1, 0, "I am a word that indicates cardinal directions.", 128);
				Screen->DrawString(6, 8, 40, 2, 1, -1, 0, "Type, then press ENTER to guess...", 128);
				Screen->DrawString(6, 20, 60, 0, 1, -1, 0, StringBuffer, 128);
				Waitframe();
			}
			Screen->DrawString(6, 8, 20, 2, 1, -1, 0, "I am a word that indicates cardinal directions.", 128);
			Screen->DrawString(6, 8, 40, 2, 1, -1, 0, "Type, then press ENTER to guess...", 128);
			Screen->DrawString(6, 20, 60, 0, 1, -1, 0, StringBuffer, 128);


			strcpy(lower, StringBuffer);
			TraceNL(); TraceS(StringBuffer); TraceNL();
			TraceS(lower); TraceNL();
			//convert to lowercase as a precaution
			utol(lower);
			TraceS(lower); TraceNL();
			//process
			if ( !(strcmp(lower, "news")) || !(strcmp(lower, "sewn")) )
			{
				won = 1;
			}
			else won = 2;
			if ( won == 1 )
			{
				Game->TypingMode = false;
				Game->PlaySound(SFX_WON);
				while(!Input->ReadKey[KEY_Q])
				{
					Screen->DrawString(6, 8, 20, 0, 1, -1, 0, "You win! Press Q to exit.", 128);
					++Game->NumDeaths;
					Waitframe();
				}
				Game->End();
			}
			else if ( won == 2 )
			{
				won = 0;

				Game->PlaySound(SFX_LOSS);

				while(!Input->ReadKey[KEY_Y])
				{
					Screen->DrawString(6, 8, 20, 0, 1, -1, 0, "Fail. Press Y to try again.", 128);
					Waitframe();
				}

				for ( int q = 0; q < 20; ++q ) StringBuffer[q] = 0;
				index = 0;
			}
			Waitframe();
		}
		Game->TypingMode = false;
	}
}
#include "std.zh"
hero script kb_read_test
{
	const int BUFFER_SIZE = 500;
	void run()
	{

		int StringBuffer[BUFFER_SIZE];
		TraceS("Input String Test:\n");
		Game->TypingMode = true;
		int index;
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
                        Waitframe();
		}
		TraceNL();
		StringBuffer[index+1] = '\n';
		TraceS(StringBuffer);
		Game->TypingMode = false;
	}
}
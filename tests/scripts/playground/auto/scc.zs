#include "std.zh"
#include "auto/test_runner.zs"

generic script scc
{
	const int SCC_COLOR = 1;
	const int SCC_SPEED = 2;
	const int SCC_GOTO_IF_COUNTER = 6;
	const int SCC_COUNTER_SET = 12;
	const int SCC_SFX = 20;
	const int SCC_NEWLINE = 25;

	int scc(int code)
	{
		return code + 1;
	}

	messagedata setMessage(int id, char32[] str)
	{
		messagedata msg = Game->LoadMessageData(id);
		msg->Set(str);
		msg->X = 24;
		msg->Y = 32;
		msg->Sound = 0;
		msg->Next = 0;
		return msg;
	}

	void playMessage(char32[] str)
	{
		setMessage(1, str);
		playMessage(1);
	}

	void playMessage(int id)
	{
		Screen->Message(id);
		while (Screen->ShowingMessage)
		{
			WaitTo(SCR_TIMING_POST_DRAW);
			Input->Button[CB_A] = true;
			Waitframe();
		}
	}

	void run()
	{
		Test::Init();
		Game->FFRules[qr_ALLOWFASTMSG] = false;
		char32 s[1000];

		playMessage("hello world");

		for (int i = 32; i <= 126; i++)
		{
			char32 s2[1];
			sprintf(s2, "%c", scc(i));
			// TODO: strcat should grow strings if needed.
			// TODO: would be nice to support `str += otherstring`, or `str += char`.
			strcat(s, s2);
		}
		playMessage(s);

		sprintf(s, "hello %cworld", scc(SCC_NEWLINE));
		playMessage(s);

		sprintf(s, "hello %c%c%cworld", scc(SCC_COLOR), scc(1), scc(2));
		for (int i = 0; i <= 50; i++)
		{
			char32 s2[1];
			sprintf(s2, "%c%c%c!", scc(SCC_COLOR), scc(0), scc(i % 10));
			strcat(s, s2);
		}
		playMessage(s);

		sprintf(s, "sounds....%c%c..............%c%c..........done",
			scc(SCC_SFX), scc(1),
			scc(SCC_SFX), scc(2)
		);
		playMessage(s);

		sprintf(s, "counter set%c%c%c check%c%c%c%c",
			scc(SCC_COUNTER_SET), scc(1), scc(10),
			scc(SCC_GOTO_IF_COUNTER), scc(1), scc(10), scc(2)
		);
		setMessage(1, s)->Next = 3;
		setMessage(2, "true...");
		setMessage(3, "false...");
		playMessage(1); // true

		sprintf(s, "counter set%c%c%c check%c%c%c%c",
			scc(SCC_COUNTER_SET), scc(1), scc(10),
			scc(SCC_GOTO_IF_COUNTER), scc(1), scc(100), scc(2));
		setMessage(1, s)->Next = 3;
		setMessage(2, "true...");
		setMessage(3, "false...");
		playMessage(1); // false

		sprintf(s, "speed%c%c.......%c%c............%c%c....",
			scc(SCC_SPEED), scc(0), 
			scc(SCC_SPEED), scc(10),
			scc(SCC_SPEED), scc(60)
		);
		playMessage(s);
	}
}

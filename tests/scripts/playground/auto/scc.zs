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
		Game->FFRules[qr_OLD_SCRIPTS_MESSAGE_DATA_BINARY_ENCODING] = true;
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

		Game->FFRules[qr_OLD_SCRIPTS_MESSAGE_DATA_BINARY_ENCODING] = false;

		playMessage("speed\\Speed\\0\\ .......\\Speed\\10\\ ............\\Speed\\60\\ ....");

		playMessage("Hey. \\ForceDelay\\120\\ Go away.");

		playMessage("Life: \\Counter\\0\\ /\\MaxCounter\\0\\");

		playMessage("Take this!\\CollectItem\\10\\");
		Waitframes(60 * 3);

		setMessage(1, "Take this!\\CollectItem\\11\\")->Next = 2;
		setMessage(2, "Have a \\TextColor\\3\\8\\ nice day!\\CollectItem\\12\\");
		playMessage(1);
		Waitframes(60 * 5);

		// RunFrozenGenericScript accepts up to 8 optional InitD args. The target
		// script copies its InitD into Data[] so we can verify what it received.
		{
			int target = Game->GetGenericScript("scc_frozen_target");
			genericdata gd = Game->LoadGenericData(target);
			gd->DataSize = 8;
			for (int i = 0; i < 8; i++)
				gd->Data[i] = -1;

			char32 msg[200];
			sprintf(msg, "Running script!\\RunFrozenGenericScript\\%d\\0\\11\\22\\33\\", target);
			playMessage(msg);

			gd = Game->LoadGenericData(target);
			int d0 = gd->Data[0];
			int d1 = gd->Data[1];
			int d2 = gd->Data[2];
			int d3 = gd->Data[3];
			int d7 = gd->Data[7];
			Test::AssertEqual(d0, 11);
			Test::AssertEqual(d1, 22);
			Test::AssertEqual(d2, 33);
			// Unspecified trailing InitD default to 0.
			Test::AssertEqual(d3, 0);
			Test::AssertEqual(d7, 0);
		}

		Test::End();
	}
}

// Target for the RunFrozenGenericScript test above. Records the InitD it was
// invoked with into its Data[] array.
generic script scc_frozen_target
{
	void run()
	{
		this->DataSize = 8;
		for (int i = 0; i < 8; i++)
			this->Data[i] = this->InitD[i];
	}
}

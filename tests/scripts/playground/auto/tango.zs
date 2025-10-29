#include "auto/test_runner.zs"
#include "../headers/examples/tango_example.zs"

generic script tango
{
	void runTangoLoop(int frames)
	{
		while (frames-- > 0)
		{
			Tango_Update1();
			WaitTo(SCR_TIMING_POST_GLOBAL_WAITDRAW);
			Tango_Update2();
			Waitframe();
		}

		WaitTo(SCR_TIMING_POST_DRAW);
		Input->Button[CB_A] = true;
		Waitframe();
	}

	void showMenu()
	{
		Game->SetMessage(1, "Pick one!@26@choice(1)Option A@26@choice(2)Option B@domenu()@26@if(@equal(@chosen 1) @append(2))@else(@append(3))");
		ShowMessage(1);
		runTangoLoop(60 * 3);
	}

	int scc(int code)
	{
		return code + 1;
	}

	void showSCC()
	{
		const int SCC_COLOR = 1;
		const int SCC_NEWLINE = 25;

		char32 s[0];
		sprintf(s, "hello %c%c%cworld!", scc(SCC_COLOR), scc(1), scc(2));
		int slot = ShowString(s);
		sprintf(s, "%cHow are you?", scc(SCC_NEWLINE));
		Tango_AppendString(slot, s);
		runTangoLoop(60 * 3);
	}

	void run()
	{
		Test::Init();

		SetUpTQSStyle();
		Tango_Start();

		// The tango update function checks the current screen, and when it differs clears all
		// active slots. So gotta run it first before using ShowMessage for the first time.
		runTangoLoop(1);

		showMenu();
		Waitframes(20);
		showSCC();
		Waitframes(20);

		Test::End();
	}
}

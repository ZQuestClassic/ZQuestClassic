#include "auto/test_runner.zs"
#include "std.zh"

// Screen->ZapOut() runs generic scripts from inside the command. The running script's state
// (position, registers, stack) used to be left as the last of those scripts' when the command
// returned, which mixed up which script ran which code: here, the generic helper went on to run
// this test's code from its start.

int bug_nested_script_state_starts;
int bug_nested_script_state_x;
int bug_nested_script_state_y;
bool bug_nested_script_state_done;

ffc script bug_nested_script_state_ffc
{
	void run()
	{
		int x = 1234;
		int y = 5678;
		Screen->ZapOut();
		bug_nested_script_state_x = x;
		bug_nested_script_state_y = y;
		bug_nested_script_state_done = true;
		Screen->ZapIn();
	}
}

// Waits for the start and the end of every frame, so it is due to run when ZapOut() runs the
// generic scripts.
generic script bug_nested_script_state_helper
{
	void run()
	{
		int a = 111;
		int b = 222;
		while (true)
		{
			a = b + a - 222;
			WaitTo(SCR_TIMING_END_FRAME);
			WaitTo(SCR_TIMING_START_FRAME);
		}
	}
}

generic script bug_nested_script_state
{
	void run()
	{
		++bug_nested_script_state_starts;
		Test::Init();

		Game->LoadGenericData(Game->GetGenericScript("bug_nested_script_state_helper"))->Running = true;
		Waitframe();

		RunFFCScriptOrQuit(Game->GetFFCScript("bug_nested_script_state_ffc"), {0});
		for (int i = 0; i < 300 && !bug_nested_script_state_done; ++i)
			Waitframe();

		Test::Assert(bug_nested_script_state_done, "ffc script finished");
		Test::AssertEqual(bug_nested_script_state_starts, 1, "test started once");
		Test::AssertEqual(bug_nested_script_state_x, 1234, "ffc local after ZapOut");
		Test::AssertEqual(bug_nested_script_state_y, 5678, "ffc local after ZapOut");

		Test::End();
	}
}

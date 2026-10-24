#include "std.zh"
#include "auto/test_runner.zs"

// The "set screen state" SCCs (133, 134) stopped reading their arguments as
// soon as one was invalid, so the remaining argument bytes were run as other
// commands or printed as text. Message strings here are written in 2.55's
// binary encoding, where each code and argument is stored as its value + 1.
generic script bug_scc_screen_state_bad_args
{
	void run()
	{
		Test::Init();

		messagedata msg = Game->LoadMessageData(1);

		// SCC 134 with an invalid map (200), screen 11, state 11, on. Before the fix,
		// the leftover state bytes ran as SCC 11 (Counter Down) on rupees, by the
		// value of the 'X' that follows.
		char32 bad_map[] = {135, 201, 12, 12, 2, 'X', 0};
		Game->Counter[CR_RUPEES] = 100;
		msg->Set(bad_map);
		Screen->Message(1);
		Waitframes(60);
		Test::AssertEqual(Game->Counter[CR_RUPEES], 100, "SCC 134 with an invalid map");

		// SCC 133 with an invalid state (20), on, followed by a real SCC 11 that takes
		// 50 rupees. Before the fix, the on/off byte ran as SCC 11 instead and ate the
		// real SCC's code as its counter.
		char32 bad_state[] = {134, 21, 12, 12, 2, 51, 0};
		Game->Counter[CR_RUPEES] = 100;
		msg->Set(bad_state);
		Screen->Message(1);
		Waitframes(60);
		Test::AssertEqual(Game->Counter[CR_RUPEES], 50, "SCC 133 with an invalid state");

		Test::End();
	}
}

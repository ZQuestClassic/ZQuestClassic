#include "auto/test_runner.zs"

// Some engine variables (CURSCR, GAMETIME, etc.) change value across a
// Waitframe(). The ZASM optimizer and JIT must not cache such a read and
// reuse it after the Waitframe(), nor treat two reads separated only by a
// Waitframe() as the same value (CSE across the frame boundary).
//
// Game->Time (GAMETIME) advances by one every frame, and Waitframe() is
// inlined to a bare WAITFRAME opcode, so both reads land in the same basic
// block - the exact shape that would tempt a value-propagation pass.
//
// No such miscompile ever existed - this test was written to confirm that
// suspicion was wrong, and stays as a regression guard.
generic script bug_opt_var_across_waitframe
{
	void run()
	{
		Test::Init();

		long before = Game->Time;
		Waitframe();
		long after = Game->Time;
		// A miscompile that cached the read would make these equal.
		Test::Assert(after != before, "Game->Time must change across a Waitframe");
		Test::Assert(after > before, "Game->Time must advance across a Waitframe");

		// Repeat, so the bad pattern must be optimized away twice.
		long before2 = Game->Time;
		Waitframe();
		long after2 = Game->Time;
		Test::Assert(after2 != before2, "Game->Time must change across a Waitframe (2)");
		Test::Assert(after2 > before2, "Game->Time must advance across a Waitframe (2)");

		// CURSCR (Game->CurScreen) is the variable that motivated this test. A
		// Player->Warp() only schedules the warp (sets Hero.ffwarp); CurScreen
		// doesn't change until the following Waitframe() executes the warp. So
		// the read after the Warp() call and the read after the Waitframe() are
		// separated only by the bare WAITFRAME opcode - the same basic block,
		// the same caching/CSE hazard as above.
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		Game->LoadMapData(map, 1)->Valid = 1;
		Player->Warp(Test::TestingDmap, 1);
		int scrBefore = Game->CurScreen;
		Waitframe();
		int scrAfter = Game->CurScreen;
		Test::Assert(scrBefore != scrAfter, "CurScreen must change across the warp's Waitframe");
		Test::AssertEqual(scrBefore, 0, "CurScreen should still be the old screen before the warp Waitframe");
		Test::AssertEqual(scrAfter, 1, "CurScreen should be the warp destination after the Waitframe");

		Test::End();
	}
}

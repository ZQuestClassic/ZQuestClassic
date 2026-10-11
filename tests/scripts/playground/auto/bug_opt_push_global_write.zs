// Regression test for a ZASM optimizer bug in the `setr_pushr` pass. Assigning
// a global variable and then passing it to a built-in function that takes its
// arguments on the stack compiles to:
//
//   SETR  GD1  D2     the assignment
//   PUSHR GD1         the argument
//
// The pass rewrote this to `PUSHR D2`, which pushes the right argument but
// drops the assignment, so the global kept its old value. Quests hit this with
// the common enhanced music script:
//
//   G = Game->GetCurDMap();
//   Game->GetDMapMusicFilename(G, buf);
//
// Passes with -no-optimize-zasm.

#include "auto/test_runner.zs"

int g_bug_opt_push_global_write;

generic script bug_opt_push_global_write
{
	void run()
	{
		Test::Init();

		int x = Game->GetCurDMap() + 3;
		g_bug_opt_push_global_write = 100;
		g_bug_opt_push_global_write = x;
		int d = Distance(g_bug_opt_push_global_write, 0, 0, 0);
		Test::AssertEqual(g_bug_opt_push_global_write, x);
		Test::AssertEqual(d, x);

		g_bug_opt_push_global_write = x + 1;
		int a = ArcTan(g_bug_opt_push_global_write, 0);
		Test::AssertEqual(g_bug_opt_push_global_write, x + 1);
		Test::AssertEqual(a, 0);

		char32 buf[256];
		g_bug_opt_push_global_write = Game->GetCurDMap();
		Game->GetDMapMusicFilename(g_bug_opt_push_global_write, buf);
		Test::AssertEqual(g_bug_opt_push_global_write, Game->GetCurDMap());

		Test::End();
	}
}

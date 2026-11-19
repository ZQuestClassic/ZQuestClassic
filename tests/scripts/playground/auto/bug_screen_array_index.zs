#include "std.zh"
#include "auto/test_runner.zs"

// Screen->SecretCombo[], Enemy[], TileWarpDMap[] and SideWarpDMap[] didn't check their index, so a
// script could read (and for SecretCombo[], write) past the ends of the screen's arrays.

generic script bug_screen_array_index
{
	void run()
	{
		Test::Init();

		int secret = Screen->SecretCombo[127];
		Screen->SecretCombo[128] = 5;
		Screen->SecretCombo[-1] = 5;
		Test::AssertEqual(Screen->SecretCombo[127], secret, "Screen->SecretCombo[127]");
		Test::AssertEqual(Screen->SecretCombo[128], -1, "Screen->SecretCombo[128]");
		Test::AssertEqual(Screen->SecretCombo[-1], -1, "Screen->SecretCombo[-1]");

		Test::AssertEqual(Screen->Enemy[10], -1, "Screen->Enemy[10]");
		Test::AssertEqual(Screen->TileWarpDMap[4], -1, "Screen->TileWarpDMap[4]");
		Test::AssertEqual(Screen->SideWarpDMap[4], -1, "Screen->SideWarpDMap[4]");

		Test::End();
	}
}

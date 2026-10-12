// Regression test for experimental ZASM optimizer passes treating a read of
// Game->AllocateBitmap() (the ALLOCATEBITMAPR register) like a plain value,
// though every read allocates:
//
//   propagate_values  `arr[i] = Game->AllocateBitmap();` pushes the new bitmap,
//                     pops it into D2 and stores D2. The store was rewritten to
//                     read ALLOCATEBITMAPR directly, which allocated a second
//                     bitmap: the array got that one, and the first one leaked.
//   dead_code         `Game->AllocateBitmap();` with the result unused was
//                     removed, so no bitmap was allocated. The same went for
//                     Game->CreateBitmap() and Screen->CreateLWeaponDx(), whose
//                     reads also create something (and Game->IncrementQuest(),
//                     not tested here since it ends the quest).
//
// Both shifted the ids of every bitmap allocated afterwards. Stellar Seas hits
// the first one.
//
// Passes with -no-optimize-zasm.

#include "auto/test_runner.zs"

generic script bug_opt_allocate_bitmap
{
	void run()
	{
		Test::Init();

		bitmap arr[4];
		bitmap a = Game->AllocateBitmap();
		int i = 1;
		arr[i] = Game->AllocateBitmap();
		bitmap c = Game->AllocateBitmap();
		Test::AssertEqual(<int>(arr[i]) - <int>a, 0.0001);
		Test::AssertEqual(<int>c - <int>a, 0.0002);

		Game->AllocateBitmap();
		bitmap e = Game->AllocateBitmap();
		Test::AssertEqual(<int>e - <int>c, 0.0002);

		Game->CreateBitmap(16, 16);
		bitmap g = Game->AllocateBitmap();
		Test::AssertEqual(<int>g - <int>e, 0.0002);

		int n = Screen->NumLWeapons();
		Screen->CreateLWeaponDx(LW_SCRIPT1, 0);
		Test::AssertEqual(Screen->NumLWeapons(), n + 1);

		Test::End();
	}
}

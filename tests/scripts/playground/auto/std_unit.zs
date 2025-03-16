#include "std.zh"
#include "auto/test_runner.zs"

generic script std_unit
{
	void run()
	{
		int screen = 0;
		Test::Init(screen);

		Test::AssertEqual(AdjacentCombo(0, DIR_RIGHT), 1);
		Test::AssertEqual(AdjacentCombo(0, DIR_DOWN), 16);
		Test::AssertEqual(AdjacentCombo(0, DIR_DOWNRIGHT), 17);
		Test::AssertEqual(AdjacentCombo(0, DIR_UPRIGHT), -1);
		Test::AssertEqual(AdjacentCombo(0, DIR_LEFT), -1);
		Test::AssertEqual(AdjacentCombo(9999, DIR_LEFT), -1);

		Test::AssertEqual(ComboAdjust(0, 16, 0), 1);
		Test::AssertEqual(ComboAdjust(1, 32, 32), 35);
		Test::AssertEqual(ComboAdjust(0, -16, 0), 0); // clamped

		Test::loadRegion(screen, 3);

		Test::AssertEqual(AdjacentCombo(0, DIR_RIGHT), 1);
		Test::AssertEqual(AdjacentCombo(0, DIR_DOWN), 16);
		Test::AssertEqual(AdjacentCombo(0, DIR_DOWNRIGHT), 17);
		Test::AssertEqual(AdjacentCombo(175, DIR_RIGHT), ComboAt(ComboX(175) + 16, ComboY(175) + 0));
		Test::AssertEqual(AdjacentCombo(175, DIR_DOWN), ComboAt(ComboX(175), ComboY(175) + 16));
		Test::AssertEqual(AdjacentCombo(175, DIR_DOWNRIGHT), ComboAt(ComboX(175) + 16, ComboY(175) + 16));
		Test::AssertEqual(AdjacentCombo(0, DIR_UPRIGHT), -1);
		Test::AssertEqual(AdjacentCombo(0, DIR_LEFT), -1);
		Test::AssertEqual(AdjacentCombo(9999, DIR_LEFT), -1);

		Test::AssertEqual(ComboAdjust(0, 16, 0), 1);
		Test::AssertEqual(ComboAdjust(175, -16, 0), 174);
		Test::AssertEqual(ComboAdjust(175, 16, 0), 336);
		Test::AssertEqual(ComboAdjust(175, 32, 32), 721);
		Test::AssertEqual(ComboAdjust(0, -16, 0), 0); // clamped

		Test::reset(screen);

		Test::End();
	}
}

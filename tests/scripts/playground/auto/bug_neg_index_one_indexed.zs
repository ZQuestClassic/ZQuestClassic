// Regression test for negative indexing of 1-indexed internal arrays. The
// wrap-around formula reused the 0-indexed math (size + index) instead of
// (size + index + 1), so for a 1-indexed array (valid 1..size) index -1
// mapped to size-1 (the second-to-last element) instead of size (the last),
// and -size errored instead of mapping to 1. Screen->LayerOpacity is such a
// 1-indexed internal array (size 6).

#include "auto/test_runner.zs"

generic script bug_neg_index_one_indexed
{
	void run()
	{
		Test::Init();

		// Give the first and last layers distinct values so the wrap target
		// is unambiguous.
		Screen->LayerOpacity[1] = 50;
		Screen->LayerOpacity[5] = 30;
		Screen->LayerOpacity[6] = 100;

		// -1 must wrap to the last element (6), not 5.
		Test::AssertEqual(Screen->LayerOpacity[-1], Screen->LayerOpacity[6]);
		// -6 must wrap to the first element (1).
		Test::AssertEqual(Screen->LayerOpacity[-6], Screen->LayerOpacity[1]);

		Test::End();
	}
}

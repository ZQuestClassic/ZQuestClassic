// Regression test for a ZASM optimizer bug in how `reduce_comparisons` folds
// a bool-cast comparison (comparing two bools with == or !=) whose operands
// are both known constants. When the result is stored as an int it is
// 10000/0, but the fold produced 1/0, so a comparison that used the result
// afterwards went the wrong way:
//
//   <int>((<int>(x < y) != 2) == true) > 0.5    folded to false
//
// `<int>(x < y) != 2` is always true, so the optimizer knows its value, and
// `== true` is then a bool-cast compare of two constants.
//
// The operands are runtime values, so the compiler folds nothing itself.
// Passes with -no-optimize-zasm.

#include "auto/test_runner.zs"

generic script bug_opt_bool_compare_seti
{
	int classify(int x, int y)
	{
		int r = 0;
		if (<int>((<int>(x < y) != 2) == true) > 0.5) r |= 1;
		if (<int>((<int>(x < y) != 2) != false) > 0.5) r |= 2;
		if (<int>((<int>(x < y) != 2) == false) > 0.5) r |= 4;
		if (<int>((<int>(x < y) != 2) == true) == 1) r |= 8;
		return r;
	}

	void run()
	{
		Test::Init();

		Test::AssertEqual(classify(1, 2), 1 | 2 | 8);
		Test::AssertEqual(classify(2, 1), 1 | 2 | 8);

		for (int i = 0; i < 2; i++)
		{
			int x = i;
			int y = 1 - i;
			// The same fold in expression (ternary) position.
			int v = <int>((<int>(x < y) != 2) == true) > 0.5 ? 1 : 0;
			Test::AssertEqual(v, 1);
			int w = <int>((<int>(x < y) != 2) == true);
			Test::AssertEqual(w, 1);
		}

		Test::End();
	}
}

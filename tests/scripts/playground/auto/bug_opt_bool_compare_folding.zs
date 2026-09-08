// Regression test for ZASM optimizer bugs in how `reduce_comparisons` folds a
// comparison whose left side is itself a comparison result. The folding
// treated the inner result as if it were 1/0, but a comparison result is
// 10000/0 (and a bool-cast compare against `true` compares truthiness):
//
//   (x < y) == true      folded to a constant false (the branch never ran)
//   <int>(x < y) != 2    folded to !(x < y) (it is always true)
//   <int>(x != 0) == 1   folded to a constant false (the `!= 0` step lost its
//                        value scale)
//
// A related crash: when such a chain folded to a constant while the result
// register was still needed afterwards, the pass indexed one before the end
// of an empty instruction list.
//
// The operands are runtime values, so the compiler folds nothing itself.
// Passes with -no-optimize-zasm.

#include "auto/test_runner.zs"

generic script bug_opt_bool_compare_folding
{
	int classify(int x, int y)
	{
		int r = 0;
		if ((x < y) == true) r |= 1;
		if ((x < y) != true) r |= 2;
		if ((x < y) == false) r |= 4;
		if (<int>(x < y) != 2) r |= 8;
		if (<int>(x < y) == 2) r |= 16;
		if (<int>(x != 0) == 1) r |= 32;
		if (<int>(x == 0) == 1) r |= 64;
		if (<int>(x < y) == 1) r |= 128;
		if (<int>(x < y) > 0.5) r |= 256;
		if (<bool>x == true) r |= 512;
		if ((x < y) == (y > x)) r |= 1024;
		return r;
	}

	void run()
	{
		Test::Init();

		int vals[] = {-3, 0, 2, 5};
		// classify() for each (x, y = next value) pair, computed by hand.
		int expected[] = {
			1 | 8 | 32 | 128 | 256 | 512 | 1024, // x=-3 y=0:  x<y, x!=0
			1 | 8 | 64 | 128 | 256 | 1024,       // x=0  y=2:  x<y, x==0
			1 | 8 | 32 | 128 | 256 | 512 | 1024, // x=2  y=5:  x<y, x!=0
			2 | 4 | 8 | 32 | 512 | 1024,         // x=5  y=-3: !(x<y), x!=0
		};
		for (int i = 0; i < 4; i++)
		{
			int x = vals[i];
			int y = vals[(i + 1) % 4];
			Test::AssertEqual(classify(x, y), expected[i]);

			// The same folds in expression (ternary) position.
			int v1 = (x < y) == true ? 1 : 0;
			int v2 = <int>(x != 0) == 1 ? 1 : 0;
			int v3 = <int>(x < y) != 2 ? 1 : 0;
			Test::AssertEqual(v1, x < y ? 1 : 0);
			Test::AssertEqual(v2, x != 0 ? 1 : 0);
			Test::AssertEqual(v3, 1);

			// The comparison result stays needed after a chain that folds to a
			// constant (the crash case): `t` is read after the `if`.
			bool t = (x < y) == true;
			if (<int>t == 0.0001) Test::Fail("a 10000/0 result can never equal 0.0001");
			Test::AssertEqual(t, x < y);
		}

		Test::End();
	}
}

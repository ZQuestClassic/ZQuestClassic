// Regression test for a web (wasm) JIT bug: dividing by a runtime zero. The
// interpreter (and the native JITs) produce sign(dividend) * MAX_SIGNED_32,
// so a negative dividend gives -214748.3647. The wasm JIT always produced the
// positive value.
//
// Passes with -no-jit; the operands are runtime values so nothing folds away.

#include "auto/test_runner.zs"

generic script bug_jit_div_zero_sign
{
	void run()
	{
		Test::Init();

		int vals[] = {-3, 3, 0, -0.0001, 214748.3647, -214748.3648};
		int zero = vals[2];
		int expected[] = {-214748.3647, 214748.3647, 214748.3647, -214748.3647, 214748.3647, -214748.3647};
		for (int i = 0; i < 6; i++)
			Test::AssertEqual(vals[i] / zero, expected[i]);

		// Constant dividend, runtime zero divisor.
		Test::AssertEqual(-5 / zero, -214748.3647);
		Test::AssertEqual(5 / zero, 214748.3647);

		Test::End();
	}
}

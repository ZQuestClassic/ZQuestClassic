// Regression test for a JIT bug: modulo by a constant whose raw fixed-point
// value is a power of two (2L, 4L, ... as longs, or 0.0002, 0.0004, ...,
// 6.5536 as ints). The native JITs lowered `x % 2^k` to `x & (2^k - 1)`,
// which only agrees with the interpreter's C `%` for non-negative x. A
// negative dividend got a wrong (non-negative) remainder.
//
// Passes with -no-jit; the dividends are runtime values so nothing folds away.

#include "auto/test_runner.zs"

generic script bug_jit_mod_pow2_negative
{
	void run()
	{
		Test::Init();

		long ls[] = {-3L, -4L, -5L, -1L, 3L, 0L, -100L};
		// C remainder: sign follows the dividend.
		long expected4[] = {-3L, 0L, -1L, -1L, 3L, 0L, 0L};
		long expected2[] = {-1L, 0L, -1L, -1L, 1L, 0L, 0L};
		long expected16[] = {-3L, -4L, -5L, -1L, 3L, 0L, -4L};
		for (int i = 0; i < 7; i++)
		{
			Test::AssertEqual(ls[i] % 4L, expected4[i]);
			Test::AssertEqual(ls[i] % 2L, expected2[i]);
			Test::AssertEqual(ls[i] % 16L, expected16[i]);
		}

		// Same divisors, but reached via an int with a fractional constant
		// (6.5536 is 65536 raw).
		int xs[] = {-1.5, -6.5536, -7, 1.5};
		int expected[] = {-1.5, 0, -0.4464, 1.5};
		for (int i = 0; i < 4; i++)
			Test::AssertEqual(xs[i] % 6.5536, expected[i]);

		// Register-register form (MODR) for comparison - never used the mask.
		long divs[] = {4L, 2L, 16L};
		for (int i = 0; i < 7; i++)
		{
			Test::AssertEqual(ls[i] % divs[0], expected4[i]);
			Test::AssertEqual(ls[i] % divs[1], expected2[i]);
			Test::AssertEqual(ls[i] % divs[2], expected16[i]);
		}

		Test::End();
	}
}

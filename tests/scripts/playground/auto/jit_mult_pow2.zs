// Regression test for the strength_reduce optimizer pass: MULTV reg, 2^k ->
// LSHIFTV32 reg, k (and that a non-power-of-2 multiply is left alone). Operands
// are runtime so they aren't constant-folded. Runs correctly with the optimizer
// on (strength-reduced to a shift) and off (plain MULTV); both are exercised in
// CI. Covers negatives for sign correctness.

#include "auto/test_runner.zs"

generic script jit_mult_pow2
{
	void run()
	{
		Test::Init();

		int v[2];
		v[0] = 7;
		v[1] = -7;
		int a = v[0]; // 7
		int b = v[1]; // -7

		Test::AssertEqual(a * 2, 14);
		Test::AssertEqual(a * 4, 28);
		Test::AssertEqual(a * 8, 56);
		Test::AssertEqual(a * 16, 112);
		Test::AssertEqual(b * 2, -14);
		Test::AssertEqual(b * 8, -56);
		Test::AssertEqual(a * 3, 21); // non-power-of-2, stays a multiply
		Test::AssertEqual(a * 1, 7);  // * 1.0 (folded by the compiler)
		Test::AssertEqual(a + 0, 7);  // + 0 (folded by the compiler)

		Test::End();
	}
}

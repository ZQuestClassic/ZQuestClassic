// Regression test for JIT-compiled shift / bitwise ops. Operands come from a
// runtime array so the expressions aren't constant-folded and actually execute
// the JIT codegen. Covers:
//   - immediate shift counts (LSHIFTV/RSHIFTV) and register counts (LSHIFTR/RSHIFTR)
//   - bitwise NOT (BITNOT) and XOR by immediate / register (XORV/XORR)
//   - negative values to check arithmetic (sign-extending) right shifts.

#include "auto/test_runner.zs"

generic script jit_shifts
{
	void run()
	{
		Test::Init();

		int vals[3];
		vals[0] = 5;
		vals[1] = -8;
		vals[2] = 2;
		int a = vals[0]; // 5
		int n = vals[1]; // -8
		int s = vals[2]; // 2 (runtime shift count)

		// Immediate shift counts -> LSHIFTV / RSHIFTV.
		Test::AssertEqual(a << 2, 20); // LSHIFTV
		Test::AssertEqual(a << 0, 5);  // LSHIFTV, no-op shift count
		Test::AssertEqual(a >> 1, 2);  // RSHIFTV
		Test::AssertEqual(n >> 1, -4); // RSHIFTV, arithmetic (sign-extending)
		Test::AssertEqual(n >> 2, -2); // -8 >> 2 = -2

		// Register shift counts -> LSHIFTR / RSHIFTR.
		Test::AssertEqual(a << s, 20); // LSHIFTR
		Test::AssertEqual(a >> s, 1);  // RSHIFTR (5 >> 2 = 1)
		Test::AssertEqual(n >> s, -2); // RSHIFTR, arithmetic (-8 >> 2 = -2)

		// Bitwise NOT.
		Test::AssertEqual(~a, -6);     // BITNOT
		Test::AssertEqual(~n, 7);      // BITNOT of a negative

		// Bitwise XOR by immediate (XORV) and register (XORR).
		Test::AssertEqual(a ^ 12, 9);  // XORV (5 ^ 12 = 9)
		Test::AssertEqual(a ^ n, -3);  // XORR (5 ^ -8 = -3)

		// `long` operands exercise the raw 32-bit variants (no fixed-point scaling
		// on the value). The shift count is still a fixed-point int (`s`), matching
		// the interpreter's do_lshift32 (which divides the count by 10000).
		// LSHIFTR32 / RSHIFTR32 / XORR32 / XORV32.
		long lvals[2];
		lvals[0] = 5L;
		lvals[1] = -8L;
		long la = lvals[0]; // 5
		long ln = lvals[1]; // -8
		Test::AssertEqual(la << s, 20L); // LSHIFTR32
		Test::AssertEqual(ln >> s, -2L); // RSHIFTR32, arithmetic
		Test::AssertEqual(la ^ ln, -3L); // XORR32
		Test::AssertEqual(la ^ 12L, 9L); // XORV32

		Test::End();
	}
}

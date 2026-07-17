// Modulo of the minimum integer by -1 used to crash. The naive 32-bit idiv
// instruction (in both the interpreter and the JIT) overflows because the
// quotient INT_MIN / -1 is not representable in 32 bits, even though the
// remainder x % -1 is always 0.
//
// The operands are read from a runtime array so the compiler can't fold the
// modulo at compile time (folding INT_MIN % -1 in the host compiler would
// itself crash).
//
// Modulo by the smallest positive value (raw 1) is also covered: the AArch64
// JIT emitted it as a bitwise AND with 0, which is not an encodable logical
// immediate, so compilation of the whole function failed.

#include "auto/test_runner.zs"
#include "std.zh"

generic script bug_mod_int_min
{
	void run()
	{
		Test::Init();

		int values[3];
		values[0] = MIN_FLOAT; // raw INT_MIN
		values[1] = -0.0001;   // raw -1
		values[2] = 5.5;       // raw 55000

		Test::AssertEqual(values[0] % values[1], 0); // MODR (register % register)
		Test::AssertEqual(values[0] % -0.0001, 0);   // MODV (register % constant)

		// Anything % raw 1 is 0.
		Test::AssertEqual(values[2] % 0.0001, 0);
		Test::AssertEqual((-values[2]) % 0.0001, 0);

		Test::End();
	}
}

// Modulo of the minimum integer by -1 used to crash. The naive 32-bit idiv
// instruction (in both the interpreter and the JIT) overflows because the
// quotient INT_MIN / -1 is not representable in 32 bits, even though the
// remainder x % -1 is always 0.
//
// The operands are read from a runtime array so the compiler can't fold the
// modulo at compile time (folding INT_MIN % -1 in the host compiler would
// itself crash).

#include "auto/test_runner.zs"
#include "std.zh"

generic script bug_mod_int_min
{
	void run()
	{
		Test::Init();

		int values[2];
		values[0] = MIN_FLOAT; // raw INT_MIN
		values[1] = -0.0001;   // raw -1

		Test::AssertEqual(values[0] % values[1], 0); // MODR (register % register)
		Test::AssertEqual(values[0] % -0.0001, 0);   // MODV (register % constant)

		Test::End();
	}
}

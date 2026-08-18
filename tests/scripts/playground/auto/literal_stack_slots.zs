#include "auto/test_runner.zs"
#include "std.zh"

// A string/array literal only lives until the end of the statement that uses
// it, so literal stack slots are recycled between statements. Previously every
// literal occurrence permanently grew the enclosing function's stack frame:
// this function's frame would be ~30 words instead of ~6, and recursing it
// would overflow the stack.
int literal_heavy(int n)
{
	int total = 0;
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	total += SizeOfArray("abcd");
	total += SizeOfArray({1, 2, 3});
	if (n > 0)
		return literal_heavy(n - 1) + total;
	return total;
}

generic script literal_stack_slots
{
	void run()
	{
		Test::Init();

		// 12 * 5 + 12 * 3 = 96 per call, 301 calls.
		Test::AssertEqual(literal_heavy(300), 28896);

		Test::End();
	}
}

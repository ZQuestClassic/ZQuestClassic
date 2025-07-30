#include "auto/test_runner.zs"

void call_recursive(int i)
{
	if (i > 0)
		call_recursive(i - 1);
	Trace(i);
}

generic script stack_limit
{
	void run()
	{
		Test::Init();

		Waitframe();

		// OK.
		Trace("call_recursive: 100");
		call_recursive(100);

		// This should overflow.
		Trace("call_recursive: 10000");
		call_recursive(10000);

		// This won't be hit (but `Test::End()` will still be called by TestRunner).
		Test::Fail("expected overflow");
	}
}

#include "auto/test_runner.zs"

int call_recursive_forever_count;

void call_recursive_forever()
{
	Trace(call_recursive_forever_count++);
	call_recursive_forever();
}

generic script call_limit
{
	void run()
	{
		Test::Init();

		Waitframe();

		call_recursive_forever();

		// This won't be hit (but `Test::End()` will still be called by TestRunner).
		Test::Fail("expected recursion error");
	}
}

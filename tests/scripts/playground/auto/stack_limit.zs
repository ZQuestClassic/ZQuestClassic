#include "auto/test_runner.zs"

void call_recursive(int i)
{
	// Just increase the stack size a bit.
	int var1 = i;
	int var2 = i;
	int var3 = i;
	int var4 = i;
	int var5 = i;

	if (i > 0)
		call_recursive(i - 1);

	// Same as tracing "i". Just trying to avoid all the local variables being optimized out.
	Trace(var1 + var2 + var3 + var4 + var5 - i * 4);
}

generic script stack_limit
{
	void run()
	{
		Test::Init();

		Waitframe();

		// OK. Deep enough that it would not have fit in the old 1024 word
		// stack (and, under jit, uses more nested calls than its old native
		// call stack could hold).
		Trace("call_recursive: 300");
		call_recursive(300);

		// This should overflow.
		Trace("call_recursive: 10000");
		call_recursive(10000);

		// This won't be hit (but `Test::End()` will still be called by TestRunner).
		Test::Fail("expected overflow");
	}
}

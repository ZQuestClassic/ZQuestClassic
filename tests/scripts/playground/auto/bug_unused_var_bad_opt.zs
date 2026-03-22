// https://discord.com/channels/876899628556091432/1484846091500257280

#include "auto/test_runner.zs"

generic script bug_unused_var_bad_opt
{
	void run()
	{
		Test::Init();

		int test_array[3];
		test_array[0] = 10;
		test_array[1] = 20;
		test_array[2] = 30;

		int sum = 0;

		// The compiler allocates stack slots for the child scope:
		// - Slot N   : __LOOP_ARR (the array reference)
		// - Slot N+1 : __LOOP_ITER (the index counter)
		// - Slot N+2 : val (the actual value)
		for (val : test_array)
			sum += val;

		// An unused variable declared after the loop was triggering a bad optimization,
		// overwritting the slot that `sum` is at.
		int unused_trigger = 99;

		Test::AssertEqual(sum, 60);

		Test::End();
	}
}

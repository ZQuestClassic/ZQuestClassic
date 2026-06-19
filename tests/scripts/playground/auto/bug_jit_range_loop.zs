// Regression test for a JIT bug. A range loop with a constant start and a
// runtime end compiles to PUSHR(end); COMPAREV2; GOTOCMP (the "is the range
// empty?" guard). The JIT kept the pushed end value in its stack cache across
// the COMPAREV2, so the following GOTOCMP's cache flush emitted a stack-bounds
// cmp (and a stack-pointer add) that clobbered the comparison's flags. The
// guard then branched on garbage and skipped the loop body entirely, even for
// a valid (non-empty) range.
//
// This only reproduces with the ZASM optimizer disabled; with it enabled the
// redundant push is removed before the JIT runs. CI runs the playground replays
// a second time with -no-optimize-zasm (see ci.py / test.yml), which is what
// actually exercises this bug.
//
// The end is read from a runtime array so it isn't a compile-time constant.

#include "auto/test_runner.zs"

generic script bug_jit_range_loop
{
	void run()
	{
		Test::Init();

		int values[1];
		values[0] = 10; // runtime end: the range [5, 10) is valid (5 iterations)

		int count = 0;
		loop (i in [5, values[0]))
			count++;

		Test::AssertEqual(count, 5); // valid range: body must run for 5, 6, 7, 8, 9
		Test::End();
	}
}

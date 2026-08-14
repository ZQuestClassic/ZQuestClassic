#include "auto/test_runner.zs"
#include "std.zh"

// Coverage for the `Trace` overloads added in 2.55.16 (d6ea5f4205): passing a
// string or a boolean directly. The traced lines are recorded as replay
// comments, so assert mode verifies the exact output on every run.
generic script trace_types
{
	void run()
	{
		Test::Init();

		Trace("Trace with a string argument");
		Trace(true);
		Trace(false);
		TraceB(true);
		TraceB(false);
		// The numeric overloads, for contrast.
		Trace(42);
		Trace(1.5);

		Test::End();
	}
}

// Regression test for script errors raised from JIT-compiled code naming a
// stale command. Compiled code never sets the command an error is reported
// under, so after a Waitframe() an error from the next compiled command was
// logged as "Waitframe(), sprite::X | ..." under the JIT, but as "sprite::X | ..."
// by the interpreter. The error is recorded in the replay, so the replay checks
// it (the script itself always passes).

#include "auto/test_runner.zs"

generic script bug_jit_stale_error_context
{
	void run()
	{
		Test::Init();

		Waitframe();
		npc n;
		Trace(n->X);

		Test::End();
	}
}

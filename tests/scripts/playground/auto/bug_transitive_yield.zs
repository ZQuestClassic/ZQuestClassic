#include "auto/test_runner.zs"

// A function that yields only TRANSITIVELY - its own body has no Waitframe(), it
// just calls a helper that does - must still be compiled with yield (suspend/
// resume) support. The wasm JIT groups every function that "may yield" (directly
// or transitively) into one unit, discovering transitive yielders by walking the
// call graph (ZasmFunction::called_by_functions). If that call graph isn't built,
// run() below is misclassified as non-yielding: its call to waitFrames() is
// lowered as a plain native call that cannot suspend across the helper's
// Waitframe, so Game->Time never advances (and the script can't resume).
generic script bug_transitive_yield
{
	void waitFrames(int n)
	{
		for (int i = 0; i < n; i++)
			Waitframe();
	}

	void run()
	{
		Test::Init();

		// run() contains no Waitframe of its own; it only yields through waitFrames().
		long before = Game->Time;
		waitFrames(5);
		long after = Game->Time;
		Test::Assert(after > before, "Game->Time must advance across a transitive yield");

		// Do it again through a deeper chain (run -> outer -> waitFrames).
		long before2 = Game->Time;
		waitFramesIndirect(5);
		long after2 = Game->Time;
		Test::Assert(after2 > before2, "Game->Time must advance across a nested transitive yield");

		Test::End();
	}

	void waitFramesIndirect(int n)
	{
		// Neither this nor run() waits directly; only waitFrames() does.
		waitFrames(n);
	}
}

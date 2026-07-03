// Exercises the wasm JIT's structured control flow on both sides of the
// yielder boundary, with checksums verifying the structured loops compute the
// same values as the interpreter/x64.
//
// Two distinct mechanisms are covered (see jit_wasm.cpp):
//
// 1. The INLINE loops in run(): run() waits, so it is inlined into the
//    yielder - and a loop written directly in its body, containing no
//    suspension, is exactly what detect_yielder_regions compiles as a real
//    wasm loop nested inside the dispatch (-no-jit-wasm-structured-yielder
//    turns that off).
//
// 2. The hot*() helper functions: they never wait, so each compiles as its
//    own wasm function with structured control flow
//    (-no-jit-wasm-structured turns that off). Covers loop-carried state
//    across back-edges, a nested loop, an early break, and an early return.
//
// The same code runs before and after Waitframes, so both the first-entry
// path and the resumed path execute structured code.

#include "auto/test_runner.zs"

generic script jit_yielder_hot_loop
{
	int hot(int iters)
	{
		int acc = 0;
		for (int i = 0; i < iters; i++)
			acc = (acc + i * 3 + 1) % 65537;
		return acc;
	}

	int hot_nested(int outer, int inner)
	{
		int acc = 0;
		for (int i = 0; i < outer; i++)
		{
			for (int j = 0; j < inner; j++)
				acc = (acc + i + j) % 12007;
		}
		return acc;
	}

	int hot_break(int iters, int stop_at)
	{
		int acc = 0;
		for (int i = 0; i < iters; i++)
		{
			if (acc == stop_at)
				break;
			acc = (acc + 7) % 1000;
		}
		return acc;
	}

	int hot_return(int iters)
	{
		int acc = 1;
		for (int i = 0; i < iters; i++)
		{
			acc = (acc * 3) % 2039;
			if (acc == 1)
				return i; // early return from inside the loop
		}
		return -1;
	}

	void run()
	{
		Test::Init();

		// Inline loops: these live in the yielder itself (structured as
		// dispatch-nested regions). Loop-carried state, a nested loop, and a
		// conditional break.
		int inline_a = 0;
		for (int i = 0; i < 20000; i++)
			inline_a = (inline_a + i * 3 + 1) % 65537;

		int inline_b = 0;
		for (int i = 0; i < 150; i++)
		{
			for (int j = 0; j < 150; j++)
				inline_b = (inline_b + i + j) % 12007;
		}

		Waitframe();

		// The same inline loop resumed on a later frame.
		int inline_c = 0;
		for (int i = 0; i < 20000; i++)
			inline_c = (inline_c + i * 3 + 1) % 65537;

		int inline_d = 0;
		for (int i = 0; i < 1000; i++)
		{
			if (inline_d == 994)
				break;
			inline_d = (inline_d + 7) % 1000;
		}

		// Function-based loops: structured non-yielding wasm functions,
		// called from the yielder.
		int a = hot(20000);
		int b = hot_nested(150, 150);
		Waitframe();
		int c = hot(20000);
		int d = hot_break(1000, 994);
		int e = hot_return(5000);
		Waitframe();

		Test::AssertEqual(inline_a, 64302);
		Test::AssertEqual(inline_b, 2547);
		Test::AssertEqual(inline_c, inline_a); // deterministic across frames
		Test::AssertEqual(inline_d, 994);
		Test::AssertEqual(a, inline_a); // inline and function agree
		Test::AssertEqual(b, 2547);
		Test::AssertEqual(c, a);
		Test::AssertEqual(d, 994);
		Test::AssertEqual(e, 1018);

		Test::End();
	}
}

// Regression test for the JIT's per-run stack-overflow bounds check.
//
// Each level of recurse() allocates a wide frame of zero-initialized locals,
// which the ZASM optimizer collapses into a single PUSHARGSV. The JIT checks
// stack bounds once per contiguous push run, but used to count a run's
// leading PUSHARGS as one slot instead of its full width - so the recursion
// below, on the level that crosses the stack limit, wrote far out of bounds
// (crashing the process) instead of exiting with the "Stack overflow!" error.
//
// The warmup loop makes recurse() hot so the deep recursion actually runs
// jitted (compiled functions are only adopted at a later script entry).

#include "auto/test_runner.zs"

int depth;

void recurse(int n)
{
	int v00; int v01; int v02; int v03; int v04; int v05; int v06; int v07; int v08; int v09;
	int v10; int v11; int v12; int v13; int v14; int v15; int v16; int v17; int v18; int v19;
	int v20; int v21; int v22; int v23; int v24; int v25; int v26; int v27; int v28; int v29;
	int v30; int v31; int v32; int v33; int v34; int v35; int v36; int v37; int v38; int v39;
	int v40; int v41; int v42; int v43; int v44; int v45; int v46; int v47; int v48; int v49;
	int v50; int v51; int v52; int v53; int v54; int v55; int v56; int v57; int v58; int v59;
	int v60; int v61; int v62; int v63; int v64; int v65; int v66; int v67; int v68; int v69;
	int v70; int v71; int v72; int v73; int v74; int v75; int v76; int v77; int v78; int v79;
	int v80; int v81; int v82; int v83; int v84; int v85; int v86; int v87; int v88; int v89;
	int v90; int v91; int v92; int v93; int v94; int v95; int v96; int v97; int v98; int v99;

	depth++;
	if (n > 0)
		recurse(n - 1);

	// Keep the locals from being optimized out (never actually runs).
	if (depth < 0)
		Trace(v00 + v01 + v02 + v03 + v04 + v05 + v06 + v07 + v08 + v09
			+ v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19
			+ v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29
			+ v30 + v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39
			+ v40 + v41 + v42 + v43 + v44 + v45 + v46 + v47 + v48 + v49
			+ v50 + v51 + v52 + v53 + v54 + v55 + v56 + v57 + v58 + v59
			+ v60 + v61 + v62 + v63 + v64 + v65 + v66 + v67 + v68 + v69
			+ v70 + v71 + v72 + v73 + v74 + v75 + v76 + v77 + v78 + v79
			+ v80 + v81 + v82 + v83 + v84 + v85 + v86 + v87 + v88 + v89
			+ v90 + v91 + v92 + v93 + v94 + v95 + v96 + v97 + v98 + v99);
}

generic script bug_jit_stack_overflow
{
	void run()
	{
		Test::Init();

		// Heat recurse() past the JIT hot-call threshold, with frames in
		// between so the compiled version gets adopted before the deep call.
		for (int i = 0; i < 15; i++)
		{
			recurse(1);
			Waitframe();
		}

		// Recurse until the data stack overflows (each level consumes 100+
		// slots, so this crosses MAX_STACK_SIZE long before the call limit).
		recurse(10000);

		// This won't be hit (but `Test::End()` will still be called by TestRunner).
		Test::Fail("expected stack overflow");
	}
}

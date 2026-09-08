// Regression test for a JIT bug: a switch on a string key. The compiler
// stores the key in the SWITCHKEY register, then compares it against each
// case label with STRCMPR. The native JITs keep SWITCHKEY in a host register
// and never write it to the interpreter's copy, but STRCMPR is not a compiled
// command - it runs in the interpreter, which reads the stale interpreter
// copy. Every string switch therefore fell through to its default case when
// the enclosing function was jitted.
//
// Passes with -no-jit; the keys are runtime values so nothing folds away.

#include "auto/test_runner.zs"

generic script bug_jit_string_switch
{
	int classify(char32[] s)
	{
		switch (s)
		{
			case "apple": return 1;
			case "banana": return 2;
			case "cherry": return 3;
			default: return 0;
		}
	}

	int classify_nodefault(char32[] s)
	{
		int r = -1;
		switch (s)
		{
			case "left": r = 10; break;
			case "right": r = 20; break;
		}
		return r;
	}

	void run()
	{
		Test::Init();

		char32 a[] = "apple";
		char32 b[] = "banana";
		char32 c[] = "cherry";
		char32 d[] = "durian";
		Test::AssertEqual(classify(a), 1);
		Test::AssertEqual(classify(b), 2);
		Test::AssertEqual(classify(c), 3);
		Test::AssertEqual(classify(d), 0);

		char32 l[] = "left";
		char32 r[] = "right";
		char32 u[] = "up";
		Test::AssertEqual(classify_nodefault(l), 10);
		Test::AssertEqual(classify_nodefault(r), 20);
		Test::AssertEqual(classify_nodefault(u), -1);

		// Same thing, inline (no call in between the key store and the compares).
		int hits = 0;
		for (int i = 0; i < 3; i++)
		{
			char32 key[8];
			if (i == 0) strcpy(key, "one");
			else if (i == 1) strcpy(key, "two");
			else strcpy(key, "three");
			switch (key)
			{
				case "one": hits += 1; break;
				case "two": hits += 10; break;
				case "three": hits += 100; break;
			}
		}
		Test::AssertEqual(hits, 111);

		Test::End();
	}
}

#include "auto/test_runner.zs"
#include "std.zh"

// Repro for engine string helpers using 16-bit loop indices (`for(word i = ...)`
// in ArrayH::getString): reading a string longer than 65535 chars wraps the
// index back to 0, so content past the 16-bit boundary is never seen and the
// null terminator is never reached.
generic script bug_string_16bit_index
{
	void run()
	{
		Test::Init();

		// Strings that differ only past the 16-bit index boundary.
		char32 a[65538];
		char32 b[65538];
		for (int i = 0; i < 65536; i++)
		{
			a[i] = 'A';
			b[i] = 'A';
		}
		a[65536] = 'B';
		b[65536] = 'C';
		// [65537] stays 0, terminating both strings.

		Test::AssertEqual(strcmp(a, a), 0);
		Test::AssertEqual(strcmp(a, b), -1);
		Test::AssertEqual(strcmp(b, a), 1);

		Test::End();
	}
}

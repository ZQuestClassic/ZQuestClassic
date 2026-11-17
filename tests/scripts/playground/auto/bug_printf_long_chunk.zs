#include "auto/test_runner.zs"

// Repro for printf/sprintf formatting text in 256 character chunks: a chunk
// that filled all 256 characters was not terminated, so formatting it read
// past the end of the chunk's buffer and appended whatever followed it in
// memory.
generic script bug_printf_long_chunk
{
	void run()
	{
		Test::Init();

		// 300 characters with no format arguments: one full 256 character
		// chunk, then the rest.
		char32 fmt[301];
		for (int i = 0; i < 300; i++)
			fmt[i] = 'a' + (i % 26);

		char32 out[1024];
		sprintf(out, fmt);
		Test::AssertEqual(strlen(out), 300);
		Test::AssertEqual(strcmp(out, fmt), 0);

		// The same, with an argument after the first chunk.
		fmt[290] = '%';
		fmt[291] = 'd';
		sprintf(out, fmt, 7);
		Test::AssertEqual(strlen(out), 299);
		Test::AssertEqual(out[290], '7');
		Test::AssertEqual(out[291], fmt[292]);

		Test::End();
	}
}

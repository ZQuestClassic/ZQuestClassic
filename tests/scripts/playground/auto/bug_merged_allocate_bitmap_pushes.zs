// Regression test for consecutive pushes of Game->AllocateBitmap() being
// merged into one. The merged push (PUSHARGSR / PUSHVARGSR) reads its register
// once, but every read of ALLOCATEBITMAPR allocates a new bitmap:
//
//   distinct(Game->AllocateBitmap(), Game->AllocateBitmap(), 0)
//
// compiled to `PUSHARGSR ALLOCATEBITMAPR 2`, so both arguments were the same
// bitmap and only one was allocated. `printf`-style arguments did the same.
// The compiler made this merge, and so did the ZASM optimizer's
// `conseq_additive` pass for scripts compiled without it.

#include "auto/test_runner.zs"

generic script bug_merged_allocate_bitmap_pushes
{
	// `unused` keeps the bitmap pushes from being last before the call, which the
	// optimizer never merges.
	bool distinct(bitmap a, bitmap b, int unused)
	{
		return a != b;
	}

	void run()
	{
		Test::Init();

		Test::Assert(distinct(Game->AllocateBitmap(), Game->AllocateBitmap(), 0), "function args got the same bitmap");

		char32 buf[64];
		sprintf(buf, "%l %l", <int>(Game->AllocateBitmap()), <int>(Game->AllocateBitmap()));
		char32 second[64];
		int len = ilen(buf);
		for (int i = len + 1; buf[i]; ++i)
			second[i - len - 1] = buf[i];
		Test::Assert(atoi(second) != atoi(buf), "printf args got the same bitmap");

		Test::End();
	}
}

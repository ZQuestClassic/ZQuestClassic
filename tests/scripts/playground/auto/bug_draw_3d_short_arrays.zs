#include "auto/test_runner.zs"

// Quad3D and Triangle3D made room for only as many values as the arrays they were given held. With
// arrays one value short in total, they wrote (and drew from) one value past the end of that room.

generic script bug_draw_3d_short_arrays
{
	void run()
	{
		Test::Init();

		// 11 + 8 + 4 + 2 = 25 values, where Quad3D uses 26.
		int quad_pos[] = {16, 16, 0, 48, 16, 0, 48, 48, 0, 16, 48};
		int quad_uv[] = {0, 0, 16, 0, 16, 16, 0, 16};
		int quad_csets[] = {0, 0, 0, 0};
		int quad_size[] = {1, 1};
		Screen->Quad3D(6, quad_pos, quad_uv, quad_csets, quad_size, 0, 1, PT_TEXTURE);

		// 8 + 6 + 3 + 2 = 19 values, where Triangle3D uses 20.
		int tri_pos[] = {64, 16, 0, 96, 16, 0, 64, 48};
		int tri_uv[] = {0, 0, 16, 0, 0, 16};
		int tri_csets[] = {0, 0, 0};
		int tri_size[] = {1, 1};
		Screen->Triangle3D(6, tri_pos, tri_uv, tri_csets, tri_size, 0, 1, PT_TEXTURE);

		Waitframe();

		Test::End();
	}
}

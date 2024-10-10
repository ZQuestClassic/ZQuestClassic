#include "std.zh"
#include "auto/test_runner.zs"

generic script bug_object_arrays
{
	void run()
	{
		Test::Init();

		Waitframe();

		itemsprite item = Screen->CreateItem(I_HEARTCONTAINER);
		Test::Assert(item);

		// TODO: fix this bug.
		// item[] items = {item};
		// Test::Assert(items);
		// Test::Assert(items[0]);
		// Test::AssertEqual(items[0], item);

		// int[] arr = {1337};
		// Test::AssertEqual(arr[0], 1337);
	}
}

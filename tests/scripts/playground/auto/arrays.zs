#include "std.zh"
#include "auto/test_runner.zs"

generic script arrays
{
	int a[] = {1, 2};
	int b[1];
	char32[] hello = "world";

	void run()
	{
		Test::Init();

		Waitframe();

		Test::Assert(a);
		Test::AssertEqual(a[0], 1);
		Test::AssertEqual(a[1], 2);

		Test::Assert(b);
		Test::AssertEqual(b[0], 0);
		b[0] = 1337;
		Test::AssertEqual(b[0], 1337);

		int c[1];
		Test::Assert(c);
		Test::AssertEqual(c[0], 0);
		c[0] = 123;
		Test::AssertEqual(c[0], 123);

		itemsprite item = Screen->CreateItem(I_HEARTCONTAINER);
		Test::Assert(item);

		// Currently, there is a difference between array declarations:
		int numbers_arr[] = {1337};
		{
			// And array pointers:
			int[] numbers = numbers_arr;
			Test::AssertEqual(numbers[0], 1337);
			// Only declarations should be freed at the end of their scope.
			// This difference will go away when arrays become managed by the GC.
		}
		Test::AssertEqual(numbers_arr[0], 1337);

		item[] items1 = {item};
		Test::Assert(items1);
		Test::Assert(items1[0]);
		Test::AssertEqual(items1[0], item);

		item items2[] = {item};
		Test::Assert(items2);
		Test::Assert(items2[0]);
		Test::AssertEqual(items2[0], item);

		item items3[1];
		items3[0] = item;
		Test::AssertEqual(items3[0], item);

		TraceS(hello);
		char32[] hello2 = "world2";
		TraceS(hello2);
		char32 hello3[] = "world3";
		TraceS(hello3);

		int string_format[80] = "some string: %s\n";
		printf(string_format, "name");
	}
}

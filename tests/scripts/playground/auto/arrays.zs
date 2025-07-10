#include "std.zh"
#include "auto/test_runner.zs"

class CarArrays
{
	int doors[4];
	int[] wheels;
}

bool ArrayContains(int[] arr, int v)
{
	for (x : arr)
		if (x == v) return true;
	return false;
}

generic script arrays
{
	int a[] = {1, 2};
	int b[1];
	char32[] hello = "world\n";

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

		int numbers_arr[] = {1337};
		{
			int[] numbers = numbers_arr;
			Test::AssertEqual(numbers[0], 1337);
		}
		Test::AssertEqual(numbers_arr[0], 1337);

		numbers_arr = {1338};
		{
			int[] numbers = numbers_arr;
			Test::AssertEqual(numbers[0], 1338);

			numbers_arr = {1339};
		}
		Test::AssertEqual(numbers_arr[0], 1339);

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
		char32[] hello2 = "world2\n";
		TraceS(hello2);
		char32 hello3[] = "world3\n";
		TraceS(hello3);

		int string_format[80] = "some string: %s\n";
		printf(string_format, "name");

		int nums1[] = {1, 2, 3};
		int nums2[] = {4, 5};
		ArrayCopy(nums2, nums1);
		Test::AssertEqual(nums2[0], 1);
		Test::AssertEqual(nums2[1], 2);

		auto car = new CarArrays();
		Test::AssertEqual(SizeOfArray(car->doors), 4);
		Test::AssertEqual(SizeOfArray(car->wheels), 0);

		// Without an initializer, this doesn't create an array.
		int[] nullArray;
		Test::AssertEqual(nullArray, NULL);

		// Internal arrays.

		Test::AssertEqual(SizeOfArray(Input->Button), CB_MAX);

		bool buttons[] = Input->Button;
		Test::AssertEqual(SizeOfArray(buttons), CB_MAX);

		auto ComboD = Screen->ComboD;
		Screen->ComboD[0] = 100;
		ComboD[0]++;
		Test::AssertEqual(ComboD[0], 101);
		// TODO: negative indices currently do not work for internal arrays.
		// Need to:
		// 1. implement in each IScriptingArray's getElement/setElement
		// 2. figure out what to do about "index bounding" internal arrays.
		//    Maybe add a QR for neg. index internal arrays, and have that
		//    avoid the existing index bounding behavior.
		// ComboD[175] = 102;
		// Test::AssertEqual(ComboD[-1], 102);
		ResizeArray(ComboD, 12); // Logs an error.
		Test::AssertEqual(SizeOfArray(ComboD), 176);

		for (int i = 0; i < 176; i++)
			ComboD[i] = i;
		int index = 0;
		for (combo : ComboD)
			Test::AssertEqual(combo, index++);
		index = 0;
		for (combo : Screen->ComboD)
			Test::AssertEqual(combo, index++);
		Test::AssertEqual(index, 176);
		Test::Assert(ArrayContains(Screen->ComboD, 175));
		Test::Assert(!ArrayContains(Screen->ComboD, 176));

		// Same object pointer.
		Test::AssertEqual(Screen->ComboD, Screen->ComboD);
		Test::AssertEqual(Screen->ComboD, ComboD);

		auto weapon = Screen->CreateLWeapon(1);
		auto wpnInitD = weapon->InitD;
		wpnInitD[1] = 1;
		Test::AssertEqual(wpnInitD[1], 1);
		Test::AssertEqual(weapon->InitD[1], 1);

		Test::Assert(IsValidArray(wpnInitD));
		weapon->Remove();
		Test::Assert(!IsValidArray(wpnInitD));
		wpnInitD[0] = 0; // Logs an error.
	}
}

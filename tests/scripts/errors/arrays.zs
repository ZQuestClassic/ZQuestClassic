void main()
{
	// OK
	int[] arr = {1};

	// Warn
	int arr2 = {1};
	int[] arr3 = 1;
	// Error.
	{
		#option LEGACY_ARRAYS off

		int arr2 = {1};
		int[] arr3 = 1;
	}
	// OK.
	{
		#option LEGACY_ARRAYS on

		int arr2 = {1};
		int[] arr3 = 1;
	}

	// Calls fnOverloaded(int[] arr)
	fnOverloaded(arr);
	fnOverloaded({1});

	// Calls fnOverloaded(int arr)
	int num = 1;
	fnOverloaded(num);
	fnOverloaded(1);

	fnOldStyleArray(arr);
	fnOldStyleArray(arr2);
	fnOldStyleArray({1});
	fnNewStyleArray(arr);
	fnNewStyleArray(arr2);
	fnNewStyleArray({1});

	int[] arr_multi[] = {arr, arr};
	arr_multi[2] = arr_multi;

	// Legacy arrays for reference counted objects is always an error, regardless of LEGACY_ARRAYS.
	bitmap bitmaps_bad_arr = {new bitmap()};
	bitmap[] bitmaps_good_arr = {new bitmap()};
	fnNewStyleArrayObjects(bitmaps_bad_arr);
	// OK.
	fnNewStyleArrayObjects(bitmaps_good_arr);

	// lweapon is not a reference counted object, so this type mismatch defaults to a warning.
	lweapon weapons_legacy_arr = {NULL};

	SizeOfArray(arr); // Exact match.
	SizeOfArray(arr2); // int casts to int[] (no warning)

	// OK.
	// https://discord.com/channels/876899628556091432/1295149938031722547
	ffc f = Screen->FFCs[0];
}

void fnOverloaded(int[] arr)
{
	printf("fnOverloaded(int[] arr)", arr[0]);
}

void fnOverloaded(int num)
{
	printf("fnOverloaded(int num)", num);
}

void fnOldStyleArray(int arr)
{
	printf("fnOldStyleArray(int arr)", arr[0]);
	// No warning.
	{
		#option LEGACY_ARRAYS on

		arr[0];
	}
	// Errors.
	{
		#option LEGACY_ARRAYS off

		arr[0];
	}
}

void fnNewStyleArray(int[] arr)
{
	printf("fnNewStyleArray(int[] arr)", arr[0]);
}

void fnNewStyleArrayObjects(bitmap[] arr)
{
	printf("fnNewStyleArrayObjects(bitmap[] arr)", arr[0]);
}

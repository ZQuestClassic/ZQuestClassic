void test()
{
	int arr[] = {1,2,3};
	int[] arr2[] = {arr,arr};
	npc[] enemies = Screen->NPCs;

	// Due to legacy array type conversion, the default here is to warn.
	arr2[0] = 5;
	arr[0] = arr2;
	arr2[2] = arr2;
	int[] arr3_bad = {arr,arr};
	{
		#option LEGACY_ARRAYS off

		// Error.
		arr2[0] = 5;
		arr[0] = arr2;
		arr2[2] = arr2;
		int[] arr3_bad = {arr,arr};
	}
	{
		#option LEGACY_ARRAYS on

		// OK.
		arr2[0] = 5;
		arr[0] = arr2;
		arr2[2] = arr2;
		int[] arr3_bad = {arr,arr};
	}

	//Errors
	arr2[1] = enemies;
	arr[0] = enemies[0];
	arr2[1][0] = enemies[1];

	//OK
	arr[0] = <int>arr2;
	untyped foo = arr;
	foo = arr2;
	foo = enemies;
	arr2[1] = {2,3,4};
}

void arraycat<T>(T[] dest, T[] vals)
{
	int indx = SizeOfArray(dest);
	ResizeArray(dest, indx+SizeOfArray(vals));
	for(v : vals)
		dest[indx++] = v;
}

void append<T>(T[] arr, T val)
{
	ArrayPushBack(arr,val);
}

void print<T>(T v)
{
	printf("Val: %d\n", v);
}
void print<T>(T[] arr)
{
	printf("Arr[]: %ad\n", arr);
}
void print<T>(T[][] arr)
{
	printf("Arr[][]: %aad\n", arr);
}

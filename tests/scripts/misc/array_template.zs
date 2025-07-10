ffc script TestArrays
{
	void run()
	{
		int v1[] = {1,2,3};
		int v2[] = {4,5,6};
		int v3[] = {7,8,9};
		int[] vs[] = {v1,v2,v3};
		print(vs);

		int[] vs2[] = {{1,2,3},{4,5,6},{7,8,9}};
		print(vs2);

		int buf[0];
		for(arr : vs)
			arraycat(buf,arr);
		print(buf);

		loop(q : -10..=-1, -1)
			append(buf,q);
		print(buf);

		int[] v4 = {10,11,12};
		append(vs,v4);
		print(vs);

		auto arr[] = {1,2,3};
		int[] arrptr = arr;

		auto n = Screen->NPCs;
		npc[] n2 = n; //!TODO Should this be valid? Casts 'const npc[]' to 'npc[]', removing qualifications...

		auto mixed[] = {1, n[0], 2L, 'c'};
		lweapon[] mixed_ptr = mixed; //Asserting that 'mixed' is of type 'untyped[]', and can cast to unrelated type 'lweapon[]'
	}
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
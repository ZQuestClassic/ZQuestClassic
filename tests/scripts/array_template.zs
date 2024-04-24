ffc script TestArrays
{
	void run()
	{
		int v1[] = {1,2,3};
		int v2[] = {4,5,6};
		int v3[] = {7,8,9};
		int[] vs[] = {v1,v2,v3}; //!TODO This should be able to take literals, if arrays become gc-managed
		print(vs);

		int buf[0];
		for(arr : vs)
			arraycat(buf,arr);
		print(buf);

		loop(q : -10..=-1, -1)
			append(buf,q);
		print(buf);

		int[] v4 = {10,11,12}; //!TODO This should be able to take literals, if arrays become gc-managed
		append(vs,v4);
		print(vs);
	}
}

void arraycat(T[] dest, T[] vals)
{
	int indx = SizeOfArray(dest);
	ResizeArray(dest, indx+SizeOfArray(vals));
	for(v : vals)
		dest[indx++] = v;
}

void append(T[] arr, T val)
{
	ArrayPushBack(arr,val);
}

void print(T v)
{
	printf("Val: %d\n", v);
}
void print(T[] arr)
{
	printf("Arr[]: %ad\n", arr);
}
void print(T[][] arr)
{
	printf("Arr[][]: %aad\n", arr);
}
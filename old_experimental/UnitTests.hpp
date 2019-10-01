



void UnitTest_PODArray()
{
	PODArray<int> a;

	a.Allocate(20);
	Assert(a.Capacity() == 20);
	Assert(a.Size() == 0);

	for(u32 i(0); i != 20; ++i)
		a.Add(i);

	Assert(a.Full());

	for(u32 i(0); i != 20; i+=2)
		a.Remove(19-i);

	for(u32 i(0); i != 10; ++i)
		Assert(a[i] == i*2);

	a.Reallocate(40);
	Assert(a.Capacity() == 40);

	for(u32 i(0); i != 10; ++i)
		Assert(a[i] == i*2);

	a.Clear();
	Assert(a.Size() == 0);

	int ca[18] = { 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

	a.Insert(0, 18);
	a.InsertRange(1, ca, 18);
	a.RemoveFromEnd();

	Assert(a.Size() == 18);
	Assert(a.Capacity() == 40);

	for(u32 i(0); i != 18; ++i)
		Assert(a[i] == 18-i);

	a.RemoveRange(1, 16);
	Assert(a.Size() == 2);
	Assert(a[0] == 18);
	Assert(a[1] == 1);

	a.Free();
}


void RunAllUnitTests()
{
	UnitTest_PODArray();

}



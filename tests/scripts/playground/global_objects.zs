#include "std.zh"

class Thing
{
	Thing otherThing;
	Counter someCounters[5];
	bitmap a_bitmap;
	Counter not_globalized;
}

class Counter
{
	int field;
}

Thing globalThing;
Thing thingNotGlobalized;

ffc script GlobalObject
{
	void run()
	{
		if (!globalThing)
		{
			globalThing = new Thing();
			globalThing->otherThing = new Thing();
			globalThing->someCounters[0] = new Counter();
			globalThing->otherThing->someCounters[0] = new Counter();
			GlobalObject(globalThing);
			GlobalObject(globalThing->otherThing);
			GlobalObject(globalThing->someCounters[0]);
			GlobalObject(globalThing->otherThing->someCounters[0]);
			globalThing->a_bitmap = Game->CreateBitmap();
			globalThing->not_globalized = new Counter();
			globalThing->not_globalized->field = 1337;

			thingNotGlobalized = new Thing();
		}

		globalThing->someCounters[0]->field += 1;
		globalThing->otherThing->someCounters[0]->field -= 1;

		printf("globalThing->someCounters[0]->field %d\n", globalThing->someCounters[0]->field);
		printf("globalThing->otherThing->someCounters[0]->field %d\n", globalThing->otherThing->someCounters[0]->field);
		// These should be non-zero only on the first load.
		printf("globalThing->a_bitmap->isAllocated() %d\n", globalThing->a_bitmap->isAllocated() ? 1 : 0);
		printf("globalThing->not_globalized->field %d\n", globalThing->not_globalized ? globalThing->not_globalized->field : 0);
		printf("thingNotGlobalized %d\n", thingNotGlobalized ? 1 : 0);
	}
}

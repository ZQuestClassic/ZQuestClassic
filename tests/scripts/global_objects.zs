#include "std.zh"

class Thing
{
	Thing otherThing;
	Counter someCounters[5];
}

class Counter
{
	int field;
}

Thing globalThing;

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
		}

		globalThing->someCounters[0]->field += 1;
		globalThing->otherThing->someCounters[0]->field -= 1;

		printf("globalThing->someCounters[0]->field %d\n", globalThing->someCounters[0]->field);
		printf("globalThing->otherThing->someCounters[0]->field %d\n", globalThing->otherThing->someCounters[0]->field);
	}
}

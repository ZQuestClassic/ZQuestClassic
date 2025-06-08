#include "auto/test_runner.zs"

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
Thing[] globalThings = {NULL};
Thing thingNotGlobalized;
Thing[] thingsNotGlobalized = {NULL};

int untypedThingGlobalized;
int untypedThingNotGlobalized;

int numTimesLoaded = 0;

generic script global_objects
{
	void run()
	{
		bool firstTime = numTimesLoaded == 0;
		if (firstTime) Test::Init();

		numTimesLoaded++;

		Thing thing; // declared here so that the assign to untypedThingNotGlobalized does not cause the GC to nuke it.

		if (!globalThing)
		{
			Test::Assert(firstTime, "firstTime");

			globalThing = new Thing();
			globalThing->otherThing = new Thing();
			globalThing->someCounters[0] = new Counter();
			globalThing->otherThing->someCounters[0] = new Counter();
			globalThings[0] = globalThing;
			GlobalObject(globalThing);
			GlobalObject(globalThing->otherThing);
			GlobalObject(globalThing->someCounters[0]);
			GlobalObject(globalThing->otherThing->someCounters[0]);
			globalThing->a_bitmap = new bitmap();
			globalThing->not_globalized = new Counter();
			globalThing->not_globalized->field = 1337;

			thingNotGlobalized = new Thing();
			thingsNotGlobalized[0] = thingNotGlobalized;

			thing = new Thing();
			thing->otherThing = new Thing();
			thing->someCounters[0] = new Counter();
			thing->otherThing->someCounters[0] = new Counter();
			untypedThingGlobalized = <int>thing;
			GlobalObject(thing);

			thing = new Thing();
			thing->otherThing = new Thing();
			thing->someCounters[0] = new Counter();
			thing->otherThing->someCounters[0] = new Counter();
			untypedThingNotGlobalized = <int>thing;
		}

		// These objects are all referenced from a global variable, or a globalized object.
		// Therefore the field variables should be incremented/decremented by 1 per load.

		globalThing->someCounters[0]->field += 1;
		globalThing->otherThing->someCounters[0]->field -= 1;

		Test::AssertEqual(globalThing->someCounters[0]->field, numTimesLoaded, "globalThing->someCounters[0]->field");
		Test::AssertEqual(globalThing->otherThing->someCounters[0]->field, -numTimesLoaded, "globalThing->otherThing->someCounters[0]->field");

		<Thing>(untypedThingGlobalized)->someCounters[0]->field += 1;
		<Thing>(untypedThingGlobalized)->otherThing->someCounters[0]->field -= 1;

		Test::AssertEqual(<Thing>(untypedThingGlobalized)->someCounters[0]->field, numTimesLoaded, "untypedThingGlobalized->someCounters[0]->field");
		Test::AssertEqual(<Thing>(untypedThingGlobalized)->otherThing->someCounters[0]->field, -numTimesLoaded, "untypedThingGlobalized->otherThing->someCounters[0]->field");

		// Should be valid only on the first load.
		Test::AssertEqual(globalThing->a_bitmap != NULL, firstTime, "globalThing->a_bitmap != NULL");
		Test::AssertEqual(IsValidArray(<Thing>(untypedThingNotGlobalized)->someCounters), firstTime, "IsValidArray(<Thing>(untypedThingNotGlobalized)->someCounters)");

		// Should always be valid - they are held by a global variable.
		Test::Assert(globalThing->not_globalized != NULL, "globalThing->not_globalized != NULL");
		Test::Assert(globalThing, "globalThing");
		Test::Assert(globalThings[0], "globalThings[0]");
		Test::Assert(thingNotGlobalized, "thingNotGlobalized");
		Test::Assert(thingsNotGlobalized[0], "thingsNotGlobalized[0]");

		if (numTimesLoaded <= 3)
		{
			// Avoid "continue", since that doesn't do a full reload from the save file.
			Game->Save();
			Game->Reload();
		}

		Test::End();
	}
}

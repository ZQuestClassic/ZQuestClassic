#include "auto/test_runner.zs"

// Freeing an object releases the objects it holds, then frees those only it held. If one of those
// ran a garbage collection in its destructor, the collection found the object being freed (no
// longer reachable) and released what it held a second time. A child that stored itself somewhere
// in its destructor was then freed anyway, leaving that reference dangling.

class GcInDtorParent
{
	GcInDtorChild child;
}

class GcInDtorChild
{
	int value;

	~GcInDtorChild()
	{
		gc_in_dtor_keeper = this;
		GC();
	}
}

class GcInDtorFiller
{
	int value;
}

GcInDtorChild gc_in_dtor_keeper;

generic script bug_gc_in_child_destructor
{
	void run()
	{
		Test::Init();

		// Screen->D[] lets go of its object after it is overwritten. A variable lets go before,
		// which kept the parent reachable during the collection.
		StoreParent();
		Screen->D[0] = 0;
		Test::Assert(gc_in_dtor_keeper != NULL, "child stored itself");

		// Allocate enough to reuse the id of a freed object.
		for (int i = 0; i < 10; i++)
		{
			GcInDtorFiller other = new GcInDtorFiller();
			other->value = 100 + i;
		}

		Test::AssertEqual(gc_in_dtor_keeper->value, 7);

		Test::End();
	}

	void StoreParent()
	{
		GcInDtorParent parent = new GcInDtorParent();
		parent->child = new GcInDtorChild();
		parent->child->value = 7;
		Screen->D[0] = parent;
	}
}

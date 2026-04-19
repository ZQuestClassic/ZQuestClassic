// https://discord.com/channels/876899628556091432/1495510911660855306

#include "auto/test_runner.zs"

int dtor_runs = 0;

class DestructorTest
{
	int val;

	DestructorTest()
	{
		val = 42;
	}

	~DestructorTest()
	{
		// Create a temporary reference to 'this'.
		// Before the engine fix, passing 'this' to Ping() bumped the ref count
		// to 1. Returning from Ping() dropped it to 0, triggering an early,
		// re-entrant memory free.
		Ping(this);
		dtor_runs++;
	}
}

void Ping(DestructorTest obj)
{
	// Accessing the object ensures the engine pushes it onto the stack,
	// temporarily incrementing the reference count.
	int x = obj->val;
	Test::AssertEqual(x, obj->val);
}

generic script bug_dtor_reentrancy
{
	void run()
	{
		Test::Init();

		dtor_runs = 0;
		TriggerDestructor();
		Test::AssertEqual(dtor_runs, 1, "Destructor successfully completed without UAF crash");

		Test::End();
	}

	void TriggerDestructor()
	{
		auto obj = new DestructorTest();
		// 'obj' goes out of scope here. The reference count drops to 0,
		// which invokes delete_script_object and ~DestructorTest().
	}
}

#include "std.zh"
#include "auto/test_runner.zs"

// Tests that a generic script surviving Game->Continue keeps the objects
// referenced by its local variables. The engine used to release all script
// stack references on continue, even though surviving generic scripts keep
// their stacks - deleting objects their locals still pointed to.

int gccont_count = 0;

class Tracked
{
	Tracked()
	{
		gccont_count++;
	}

	~Tracked()
	{
		gccont_count--;
	}
}

int gccont_holder_started = 0;
int gccont_holder_ready = 0;
int gccont_holder_done = 0;
int gccont_verify_now = 0;
int gccont_do_continue = 0;
int gccont_phase = 0;

// Holds an object in a local variable across the continue. Nothing relaunches
// this script, so it survives the continue with its stack intact.
generic script gc_continue_holder
{
	void run()
	{
		gccont_holder_started++;
		if (gccont_holder_started > 1)
		{
			Test::Fail("holder was relaunched; expected it to survive the continue");
			return;
		}

		Tracked t = new Tracked();
		Waitframe(); // Drain the autorelease pool; only the local retains the object.
		Test::AssertEqual(RefCount(t), 1L, "RefCount(t) before continue");
		gccont_holder_ready = 1;

		while (gccont_verify_now == 0)
			Waitframe();

		Test::AssertEqual(gccont_count, 1, "object alive after continue");
		Test::AssertEqual(RefCount(t), 1L, "RefCount(t) after continue");
		gccont_holder_done = 1;
		while (true) Waitframe();
	}
}

// Triggers the continue. Game->Continue() ends the calling script.
generic script gc_continue_trigger
{
	void run()
	{
		while (gccont_do_continue == 0)
			Waitframe();
		Game->Continue();
	}
}

generic script garbage_collection_continue
{
	void run()
	{
		Test::Init();

		if (gccont_phase == 0)
		{
			gccont_phase = 1;
			Game->LoadGenericData(Game->GetGenericScript("gc_continue_holder"))->Running = true;
			Game->LoadGenericData(Game->GetGenericScript("gc_continue_trigger"))->Running = true;

			while (gccont_holder_ready == 0)
				Waitframe();
			gccont_do_continue = 1;
			// The continue relaunches this script (the test runner re-triggers it).
			while (true) Waitframe();
		}

		// Relaunched after the continue.
		gccont_verify_now = 1;
		for (int i = 0; i < 120 && gccont_holder_done == 0; i++)
			Waitframe();
		Test::AssertEqual(gccont_holder_done, 1, "holder verified after continue");

		Test::End();
	}
}

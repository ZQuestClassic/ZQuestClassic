#include "std.zh"
#include "auto/test_runner.zs"

// Tests that saving and reloading (Game->Save + Game->Reload) correctly
// restores script objects: identity, cycles, untyped-array object tracking,
// reconstructed reference counts, global-flagged objects with zero references,
// and the zeroing of non-restorable object types (bitmap, randgen, etc).

class GCSLNode
{
	int tag;
	GCSLNode next;
}

class GCSLBox
{
	int items[0];
	bitmap bmp;
}

GCSLNode gcsl_a;
GCSLNode[] gcsl_arr;
untyped gcsl_mixed[2];
untyped gcsl_mixed2[2];
bitmap gcsl_bmp;
randgen gcsl_rng;
GCSLBox gcsl_box;
int[][] gcsl_nested;
int gcsl_launder;
int gcsl_phase = 0;

generic script garbage_collection_save_load
{
	void run()
	{
		Test::Init();

		if (gcsl_phase == 0)
		{
			GCSLNode a = new GCSLNode();
			a->tag = 7;
			GCSLNode b = new GCSLNode();
			b->tag = 8;
			a->next = b;
			b->next = a; // cycle
			gcsl_a = a;
			gcsl_arr = {a, b};
			gcsl_mixed[0] = b;
			gcsl_mixed[1] = 42;
			gcsl_mixed2[0] = new bitmap(); // non-restorable object in an untyped array
			gcsl_mixed2[1] = a;
			gcsl_bmp = new bitmap();       // non-restorable object in a global variable
			gcsl_rng = new randgen();
			gcsl_box = new GCSLBox();
			ArrayPushBack(gcsl_box->items, 1337);
			gcsl_box->bmp = new bitmap();  // non-restorable object in a class member
			gcsl_nested = {{1}, {2}};
			// A global-flagged object referenced only through a laundered int.
			GCSLNode floater = new GCSLNode();
			floater->tag = 9;
			gcsl_launder = <int>floater;
			GlobalObject(floater);

			gcsl_phase = 1;
			Game->Save();
			Game->Reload();
			// Game->Reload ends this script; the reload relaunches it.
		}
		else
		{
			// Object graph structure and identity.
			Test::AssertEqual(gcsl_a->tag, 7, "gcsl_a->tag");
			Test::AssertEqual(gcsl_a->next->tag, 8, "gcsl_a->next->tag");
			Test::Assert(gcsl_a->next->next == gcsl_a, "cycle identity");
			Test::Assert(gcsl_arr[0] == gcsl_a, "gcsl_arr[0] identity");
			Test::Assert(gcsl_arr[1] == gcsl_a->next, "gcsl_arr[1] identity");
			Test::Assert(gcsl_mixed[0] == gcsl_a->next, "gcsl_mixed[0] identity");
			Test::AssertEqual(<int>gcsl_mixed[1], 42, "gcsl_mixed[1]");
			Test::Assert(gcsl_mixed2[1] == gcsl_a, "gcsl_mixed2[1] identity");

			// Reference counts, reconstructed on load.
			// a: gcsl_a global + b->next member + gcsl_arr[0] + gcsl_mixed2[1]
			Test::AssertEqual(RefCount(gcsl_a), 4L, "RefCount(a)");
			// b: a->next member + gcsl_arr[1] + gcsl_mixed[0]
			Test::AssertEqual(RefCount(gcsl_a->next), 3L, "RefCount(b)");
			Test::AssertEqual(RefCount(gcsl_arr), 1L, "RefCount(gcsl_arr)");

			// Non-restorable object types are zeroed.
			Test::Assert(gcsl_bmp == NULL, "bitmap global is NULL");
			Test::Assert(gcsl_rng == NULL, "randgen global is NULL");
			Test::AssertEqual(<int>gcsl_mixed2[0], 0, "bitmap in untyped array zeroed");
			Test::Assert(gcsl_box->bmp == NULL, "bitmap member is NULL");

			// Class member arrays.
			Test::AssertEqual(gcsl_box->items[0], 1337, "box items[0]");
			Test::AssertEqual(RefCount(gcsl_box->items), 1L, "RefCount(box items)");

			// Nested arrays.
			Test::AssertEqual(gcsl_nested[0][0], 1, "nested value");
			Test::AssertEqual(RefCount(gcsl_nested[1]), 1L, "RefCount(nested[1])");

			// Global-flagged objects survive with zero counted references.
			Test::AssertEqual(RefCount(gcsl_launder), 0L, "floater RefCount");
			Test::AssertEqual((<GCSLNode>gcsl_launder)->tag, 9, "floater tag");

			// A full GC does not delete anything restored.
			GC();
			Test::AssertEqual(gcsl_a->tag, 7, "post-GC gcsl_a->tag");
			Test::AssertEqual(gcsl_a->next->tag, 8, "post-GC b tag");
			Test::AssertEqual((<GCSLNode>gcsl_launder)->tag, 9, "post-GC floater tag");

			Test::End();
		}
	}
}

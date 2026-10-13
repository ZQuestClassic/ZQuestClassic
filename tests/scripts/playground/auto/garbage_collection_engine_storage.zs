#include "std.zh"
#include "auto/test_runner.zs"

// Tests objects stored in engine storage that outlives scripts: Screen->D[],
// Screen->Data[] / mapdata->Data[] and genericdata->Data[]. Like global variables,
// they retain their objects - each slot of the storage separately (ex. every
// screen has its own D[]). See also garbage_collection_save_load.zs.

int gces_dtors;

class GCESNode
{
	int tag;

	GCESNode(int tag)
	{
		this->tag = tag;
	}

	~GCESNode()
	{
		gces_dtors++;
	}
}

// Its destructor stores it back into Screen->D[].
class GCESSticky
{
	int tag;
	bool stored;

	GCESSticky(int tag)
	{
		this->tag = tag;
	}

	~GCESSticky()
	{
		if (stored)
			return;
		stored = true;
		Screen->D[4] = this;
	}
}

generic script garbage_collection_engine_storage
{
	void run()
	{
		Test::Init();

		// Screen->D[] keeps its object while the player is on other screens. It used
		// to be tracked once for every screen, so the object was freed by the next GC.
		Screen->D[0] = new GCESNode(1);
		Test::AssertEqual(RefCount(<GCESNode>(Screen->D[0])), 1L, "Screen->D: RefCount");
		Player->Warp(Test::TestingDmap, 1);
		Waitframe();
		Screen->D[0] = 7; // Another screen's D[0].
		GC();
		Test::AssertEqual(gces_dtors, 0, "Screen->D: object kept on another screen");
		Player->Warp(Test::TestingDmap, 0);
		Waitframe();
		{
			GCESNode node = Screen->D[0];
			Test::AssertEqual(node->tag, 1, "Screen->D: object after returning");
			Test::AssertEqual(RefCount(node), 2L, "Screen->D: RefCount after returning");
		}

		// Storing a number releases the object.
		Screen->D[0] = 5;
		Test::AssertEqual(gces_dtors, 1, "Screen->D: released when overwritten");

		// So does Game->SetScreenD, which writes the same storage.
		Screen->D[1] = new GCESNode(2);
		Game->SetScreenD(Game->CurScreen, 1, 6);
		Test::AssertEqual(gces_dtors, 2, "Screen->D: released by SetScreenD");
		Test::AssertEqual(Screen->D[1], 6, "Screen->D: SetScreenD value");

		// Storing the slot's own object again keeps it.
		Screen->D[2] = new GCESNode(3);
		Screen->D[2] = <GCESNode>(Screen->D[2]);
		Test::AssertEqual(gces_dtors, 2, "Screen->D: self-assignment");
		Screen->D[2] = 0;
		Test::AssertEqual(gces_dtors, 3, "Screen->D: released after self-assignment");

		// Screen->Data[] and mapdata->Data[] of the same screen share their slots.
		Screen->DataSize = 2;
		Screen->Data[1] = new GCESNode(4);
		mapdata md = Game->LoadMapData(Game->CurMap, Game->CurScreen);
		{
			GCESNode node = md->Data[1];
			Test::AssertEqual(node->tag, 4, "Screen->Data: read through mapdata");
			Test::AssertEqual(RefCount(node), 2L, "Screen->Data: one reference for both");
		}
		GC();
		Test::AssertEqual(gces_dtors, 3, "Screen->Data: object kept by GC");
		// Shrinking releases the objects in the removed slots.
		Screen->DataSize = 1;
		Test::AssertEqual(gces_dtors, 4, "Screen->Data: released when shrunk");
		// Growing again doesn't bring back the released object.
		Screen->DataSize = 2;
		Test::AssertEqual(<long>(Screen->Data[1]), 0L, "Screen->Data: empty after growing again");

		genericdata gd = Game->LoadGenericData(Game->GetGenericScript("garbage_collection_engine_storage"));
		gd->DataSize = 1;
		gd->Data[0] = new GCESNode(5);
		GC();
		Test::AssertEqual(gces_dtors, 4, "genericdata->Data: object kept by GC");
		gd->DataSize = 0;
		Test::AssertEqual(gces_dtors, 5, "genericdata->Data: released when shrunk");

		// Copying from the storage knows which elements are objects.
		Screen->D[3] = new GCESNode(6);
		untyped copy[8];
		ArrayCopy(copy, Screen->D);
		Screen->D[3] = 0;
		Test::AssertEqual(gces_dtors, 5, "ArrayCopy: copy retains the object");
		Test::AssertEqual((<GCESNode>copy[3])->tag, 6, "ArrayCopy: copied object");

		// mapdata->D[] can't hold objects, but on a DMap whose number is its map's
		// number it writes the same storage as Screen->D[] - so it can overwrite an
		// object there. It used to leave the object retained (leaked), and the GC
		// treated the number written over it as an object.
		Player->Warp(1, 0); // DMap 1 shows map 1.
		Waitframe();
		mapdata same = Game->LoadMapData(Game->CurMap, Game->CurScreen);
		same->D[7] = 3;
		Test::AssertEqual(<long>(Screen->D[7]), 3L, "mapdata->D: same storage as Screen->D here");
		Screen->D[6] = new GCESNode(7);
		same->D[6] = 2;
		Test::AssertEqual(gces_dtors, 6, "mapdata->D: released the object it overwrote");
		GC();
		Screen->D[6] = 0;
		Test::AssertEqual(gces_dtors, 6, "mapdata->D: no double release");

		// A destructor run by overwriting a slot can store an object in that same slot.
		// The overwrite used to happen after the destructor, clobbering the object.
		Screen->D[4] = new GCESSticky(9);
		Game->SetScreenD(Game->CurScreen, 4, 1);
		GC();
		{
			GCESSticky sticky = Screen->D[4];
			Test::AssertEqual(sticky->tag, 9, "Screen->D: object stored by a destructor");
			Test::AssertEqual(RefCount(sticky), 2L, "Screen->D: object stored by a destructor is retained");
		}
		Screen->D[4] = 0;

		Test::End();
	}
}

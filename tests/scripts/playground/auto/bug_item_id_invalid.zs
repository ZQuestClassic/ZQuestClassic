#include "std.zh"
#include "auto/test_runner.zs"

// Setting an item's ID to an invalid item used to be accepted, and the item's data was then looked
// up by it (for example when moving the item).

generic script bug_item_id_invalid
{
	void run()
	{
		Test::Init();

		item it = Screen->CreateItem(I_HEARTCONTAINER);
		it->ID = 10000;
		Test::AssertEqual(it->ID, I_HEARTCONTAINER);
		it->Y = 40;
		Waitframe();

		Test::End();
	}
}

#include "std.zh"
#include "auto/test_runner.zs"

// Only one item is checked for pickup per frame. A dummy item (which can never
// be picked up) earlier in the item list used to be the one checked, so a real
// item at the same spot could never be collected.
generic script bug_dummy_item_blocks_pickup
{
	void run()
	{
		Test::Init();

		itemsprite dummy = Screen->CreateItem(I_RUPEE1);
		dummy->X = Hero->X;
		dummy->Y = Hero->Y;
		dummy->Pickup = IP_DUMMY;

		itemsprite real = Screen->CreateItem(I_RUPEE1);
		real->X = Hero->X;
		real->Y = Hero->Y;

		Waitframes(10);

		Test::Assert(!real->isValid(), "item at the same spot as a dummy item was not picked up");
		Test::Assert(dummy->isValid(), "dummy item was picked up");

		Test::End();
	}
}

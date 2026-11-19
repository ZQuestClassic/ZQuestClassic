// shopdata's arrays (Item[], HasItem[], Price[] and String[]) checked the index given to them,
// but not the shopdata itself. A script can make a shopdata out of any number, and for one that
// is neither a shop nor an info shop they read and wrote memory past the shops. Now such a
// shopdata's arrays have nothing in them.

#include "auto/test_runner.zs"

generic script bug_shopdata_invalid_pointer
{
	void checkInvalid(shopdata shop)
	{
		shop->Item[0] = 7;
		shop->HasItem[1] = true;
		shop->Price[2] = 7;
		shop->String[0] = 7;

		Test::AssertEqual(shop->Item[0], -1);
		Test::AssertEqual(shop->Price[2], -1);
		Test::AssertEqual(shop->String[0], -1);
	}

	void run()
	{
		Test::Init();

		shopdata real = Game->LoadShopData(0);
		real->Item[0] = 5;
		real->Price[2] = 50;

		// Before the first shop, and after the last info shop.
		checkInvalid(<shopdata>(-1));
		checkInvalid(<shopdata>(20000));

		// Real shops still work, and weren't written by the above.
		Test::AssertEqual(real->Item[0], 5);
		Test::AssertEqual(real->Price[2], 50);
		shopdata info = Game->LoadInfoShopData(0);
		info->Price[1] = 20;
		Test::AssertEqual(info->Price[1], 20);

		Test::End();
	}
}

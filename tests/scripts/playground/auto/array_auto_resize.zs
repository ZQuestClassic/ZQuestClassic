// Internal functions that write into a script array grow the array when it is
// too small, instead of truncating the result and logging an error. Plain
// out-of-bounds writes still do not resize.
//
// https://discord.com/channels/876899628556091432/1484109453132562592

#include "auto/test_runner.zs"

generic script array_auto_resize
{
	// Fills [small] via the same call as [big] and checks that it grew to
	// exactly fit the same string.
	void checkGrown(char32[] small, char32[] big, char32[] what)
	{
		Test::AssertEqual(strcmp(small, big), 0, what);
		Test::AssertEqual(SizeOfArray(small), strlen(big) + 1, what);
	}

	void run()
	{
		Test::Init();

		itemdata sword = Game->LoadItemData(I_SWORD1);

		// A large enough buffer is left alone (never shrunk).
		char32 name[256];
		sword->GetName(name);
		Test::Assert(strlen(name) > 0, "sword has a name");
		Test::AssertEqual(SizeOfArray(name), 256);

		char32 small_name[1];
		sword->GetName(small_name);
		checkGrown(small_name, name, "itemdata->GetName");

		// Exactly one short: no room for the null terminator.
		char32 short_name[1];
		ResizeArray(short_name, strlen(name));
		sword->GetName(short_name);
		checkGrown(short_name, name, "itemdata->GetName (one short)");

		char32 shown[256];
		sword->GetShownName(shown);
		char32 small_shown[2];
		sword->GetShownName(small_shown);
		checkGrown(small_shown, shown, "itemdata->GetShownName");

		sword->SetDisplayName("A Display Name");
		char32 small_display[3];
		sword->GetDisplayName(small_display);
		Test::AssertEqual(strcmp(small_display, "A Display Name"), 0);
		Test::AssertEqual(SizeOfArray(small_display), 15);

		// dmapdata
		dmapdata dm = Game->LoadDMapData(Test::TestingDmap);
		dm->SetName("Resizing DMap");
		char32 small_dmap[1];
		dm->GetName(small_dmap);
		Test::AssertEqual(strcmp(small_dmap, "Resizing DMap"), 0);
		Test::AssertEqual(SizeOfArray(small_dmap), 14);
		char32 small_dmap2[1];
		Game->GetDMapName(Test::TestingDmap, small_dmap2);
		checkGrown(small_dmap2, small_dmap, "Game->GetDMapName");

		// messages
		messagedata msg = Game->LoadMessageData(1);
		msg->Set("A message that is longer than the buffer");
		char32 msg_big[512];
		msg->Get(msg_big);
		char32 msg_small[1];
		msg->Get(msg_small);
		checkGrown(msg_small, msg_big, "messagedata->Get");
		char32 msg_small2[4];
		Game->GetMessage(1, msg_small2);
		checkGrown(msg_small2, msg_big, "Game->GetMessage");

		// string functions
		char32 cat[3] = "ab";
		strcat(cat, "cdef");
		Test::AssertEqual(strcmp(cat, "abcdef"), 0, "strcat");
		Test::AssertEqual(SizeOfArray(cat), 7, "strcat");

		char32 cpy[1];
		strcpy(cpy, "hello world");
		Test::AssertEqual(strcmp(cpy, "hello world"), 0, "strcpy");
		Test::AssertEqual(SizeOfArray(cpy), 12, "strcpy");

		char32 itoa_big[64];
		itoa(itoa_big, 12345);
		char32 itoa_small[1];
		Test::AssertEqual(itoa(itoa_small, 12345), 5, "itoa");
		checkGrown(itoa_small, itoa_big, "itoa");

		itoacat(itoa_small, 67);
		Test::AssertEqual(strcmp(itoa_small, "1234567"), 0, "itoacat");
		Test::AssertEqual(SizeOfArray(itoa_small), 8, "itoacat");

		char32 xtoa_big[64];
		xtoa(xtoa_big, 48879);
		char32 xtoa_small[1];
		Test::AssertEqual(xtoa(xtoa_small, 48879), strlen(xtoa_big), "xtoa");
		checkGrown(xtoa_small, xtoa_big, "xtoa");

		char32 fmt[1];
		Test::AssertEqual(sprintf(fmt, "%d-%s", 42, "abc"), 6, "sprintf");
		Test::AssertEqual(strcmp(fmt, "42-abc"), 0, "sprintf");
		Test::AssertEqual(SizeOfArray(fmt), 7, "sprintf");

		// Graphics->ConvertFromRGB needs 4 values for CMYK.
		Game->FFRules[qr_SCRIPTS_6_BIT_COLOR] = false;
		int cmyk[1];
		Graphics->ConvertFromRGB(cmyk, Graphics->CreateRGB(255, 0, 0), CSPACE_CMYK);
		Test::AssertEqual(SizeOfArray(cmyk), 4, "ConvertFromRGB");
		Test::AssertEqual(cmyk[0], 0);
		Test::AssertEqual(cmyk[1], 1);
		Test::AssertEqual(cmyk[2], 1);
		Test::AssertEqual(cmyk[3], 0);

		// A plain out-of-bounds write is an error, and does not resize.
		int plain[2];
		int i = 5;
		plain[i] = 1;
		Test::AssertEqual(SizeOfArray(plain), 2, "plain OOB write");

		Test::End();
	}
}

#include "auto/test_runner.zs"
#include "std.zh"

// Repro for DMap titles and intros with bytes 128-255 that aren't valid UTF-8
// drawing '^' (or the wrong character) for each such byte. Quests made before
// 2.55-alpha-117 use these bytes for the Zelda NES font's symbols, such as
// hearts (0x83) and arrows (0x84, 0x85). There are no assertions: the replay's
// screen checksums cover how the title and intro are drawn, whatever draws
// the text.
//
// The intro is drawn one character at a time with "%c" (see run_intro), which
// went wrong differently than other text: 0xA8 drew as '('.
generic script bug_text_latin1_dmap_intro
{
	void run()
	{
		Test::Init();

		dmapdata dm = Game->LoadDMapData(Test::TestingDmap);

		char32 title[] = "x Hearts x          ";
		title[0] = 0x83;
		title[9] = 0x83;
		dm->SetTitle(title);

		char32 intro[73];
		for (int i = 0; i < 72; ++i)
			intro[i] = ' ';
		intro[72] = 0;
		// Line 1: arrows and symbols, as in the intro of a 2.50 quest.
		char32 line1[] = { 0x84, 0x84, 0x84, ' ', 0xA8, ' ', 0xA3, ' ', 0x82, ' ', 0x85, 0x85, 0x85 };
		for (int i = 0; i < SizeOfArray(line1); ++i)
			intro[5 + i] = line1[i];
		char32 line2[] = "WELCOME";
		for (int i = 0; i < 7; ++i)
			intro[24 + 8 + i] = line2[i];
		dm->SetIntro(intro);

		// Show the intro again when re-entering the DMap, in the old style that
		// draws DMap::Intro (instead of a message string).
		Game->FFRules[qr_OLD_DMAP_INTRO_STRINGS] = true;
		dm->Flagset[DMFS_ALWAYSINTROMSG] = 1;
		Hero->Warp(Test::TestingDmap, 0);

		// Long enough for the whole intro to be typed out (one character every
		// 6 frames, skipping runs of spaces).
		Waitframes(180);
	}
}

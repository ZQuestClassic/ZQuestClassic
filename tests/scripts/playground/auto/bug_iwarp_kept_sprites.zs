#include "std.zh"
#include "auto/test_runner.zs"

// An insta-warp from a script that kept the sprites (WARP_FLAG_DONT_CLEAR_SPRITES) left each one
// on its old screen, which was no longer loaded, so drawing a kept enemy crashed.
generic script bug_iwarp_kept_sprites
{
	void run()
	{
		Test::Init();

		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		Game->LoadMapData(map, 2)->Valid = 1;

		npc n = Screen->CreateNPC(NPC_DARKNUT1);
		n->X = 120;
		n->Y = 32;
		n->Script = Game->GetNPCScript("bug_iwarp_kept_sprites_npc");
		Waitframe();
		Test::AssertEqual(n->Misc[0], 0, "screen before warping");

		Hero->WarpEx(WT_IWARP, Test::TestingDmap, 2, Hero->X, Hero->Y, WARPEFFECT_NONE, 0, WARP_FLAG_DONT_CLEAR_SPRITES | WARP_FLAG_DONT_RESET_DM_SCRIPT);
		while (Game->GetCurScreen() != 2)
			Waitframe();
		Waitframe();

		Test::Assert(n->isValid(), "npc kept");
		Test::AssertEqual(n->Misc[0], 2, "screen after warping");

		Test::End();
	}
}

npc script bug_iwarp_kept_sprites_npc
{
	void run()
	{
		while (true)
		{
			this->Misc[0] = Screen->Index;
			Waitframe();
		}
	}
}

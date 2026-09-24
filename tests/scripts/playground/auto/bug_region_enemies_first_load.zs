// Regression test for a region screen spawning no enemies when returning to a
// recently visited region, if that screen never came into view during the
// previous visit. The "recently visited" check was done for the region as a
// whole, so the screen was treated as visited with all its enemies killed.

#include "std.zh"
#include "auto/test_runner.zs"

generic script bug_region_enemies_first_load
{
	int countEnemiesSpawnedOn(int screen)
	{
		int count = 0;
		for (int i = 1; i <= Screen->NumNPCs(); i++)
		{
			if (Screen->LoadNPC(i)->SpawnScreen == screen)
				count++;
		}
		return count;
	}

	void run()
	{
		Test::Init();

		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		mapdata scr = Game->LoadMapData(map, 1);
		for (int i = 0; i < 3; i++)
			scr->Enemy[i] = NPC_GEL;

		// Enter a 2x2 region at screen 0. Screen 1 is to the right, out of
		// view, so its enemies don't load.
		Test::loadRegion(0, 2);
		Waitframes(60);
		Test::AssertEqual(countEnemiesSpawnedOn(1), 0, "screen 1 should not load while out of view");

		// Leave the region and come back.
		Player->Warp(Test::TestingDmap, 0x22);
		Waitframes(2);
		Player->Warp(Test::TestingDmap, 0);
		Waitframes(2);

		// Bring screen 1 into view.
		Hero->X = 256 + 120;
		Waitframes(60);
		Test::AssertEqual(countEnemiesSpawnedOn(1), 3, "screen 1 should spawn its enemies");

		Test::End();
	}
}

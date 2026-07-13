// Regression test for an out-of-bounds read via sprite->SpawnScreen. The
// setter stored value/10000 straight into the uint8_t screen_spawned with
// no validation. Consumers such as enemy::draw call get_scr(screen_spawned),
// which indexes temporary_screens[screen*7] (valid screens 0..135), so an
// out-of-range SpawnScreen read out of bounds (and a negative value wrapped
// in the uint8_t). The setter now rejects any screen that isn't in the
// current region, leaving screen_spawned at its valid value.

#include "auto/test_runner.zs"
#include "std.zh"

generic script bug_spawn_screen_oob
{
	void run()
	{
		Test::Init();

		// The enemy spawns on the current screen (0), a valid region screen.
		npc enemy = Screen->CreateNPC(NPC_ARMOS);
		enemy->Step = 0;
		Test::AssertEqual(enemy->SpawnScreen, 0);

		// Out-of-range and negative screens are rejected; screen_spawned is
		// left unchanged (still the valid spawn screen) rather than stored.
		enemy->SpawnScreen = 200;
		Test::AssertEqual(enemy->SpawnScreen, 0);
		enemy->SpawnScreen = -50;
		Test::AssertEqual(enemy->SpawnScreen, 0);
		enemy->SpawnScreen = 999999;
		Test::AssertEqual(enemy->SpawnScreen, 0);

		// The enemy still has a valid screen_spawned, so it draws without
		// crashing (this is where the out-of-bounds read previously fired).
		Waitframe();
		Waitframe();

		Test::End();
	}
}

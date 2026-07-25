// https://discord.com/channels/876899628556091432/1530144181274542211
//
// An lweapon drawing relative to itself via DRAW_ORIGIN_SPRITE queues its
// last draw command on the same frame the weapon despawns off the edge of
// the screen. That command used to log "Ignoring draw command using
// DRAW_ORIGIN_SPRITE with non-existent sprite uid" when it rendered; now it
// is skipped silently (the warning still fires if a script draws relative to
// an already-deleted sprite). The replay's recorded comments lock the
// absence of the error.

#include "std.zh"
#include "auto/test_runner.zs"

lweapon script bug_draworigin_despawn_particle
{
	void run()
	{
		Screen->DrawOrigin = DRAW_ORIGIN_SPRITE;
		Screen->DrawOriginTarget = this;
		while (true)
		{
			Screen->FastTile(4, 0, 0, this->Tile, this->CSet);
			Waitframe();
		}
	}
}

generic script bug_draworigin_despawn
{
	void run()
	{
		Test::Init();

		lweapon w = Screen->CreateLWeapon(LW_ARROW);
		w->X = 24;
		w->Y = 80;
		w->Dir = DIR_LEFT;
		w->Step = 400;
		w->Script = Game->GetLWeaponScript("bug_draworigin_despawn_particle");

		for (int i = 0; i < 30; ++i)
			Waitframe();

		// The weapon must actually have despawned for this test to mean anything.
		Test::AssertEqual(Screen->NumLWeapons(), 0);

		Test::End();
	}
}

#include "std.zh"
#include "auto/test_runner.zs"

// A bomb's blast used to keep the lit bomb's 'No Collision When Still' weapon
// flag. A blast never moves, so a flagged bomb went off harmlessly. The flag
// is meant to let a bomb lie on the ground without hurting anyone until it
// detonates, so the blast must still hit.
generic script bug_bomb_blast_no_coll_when_still
{
	void run()
	{
		Test::Init();

		int hp = Hero->HP;

		// A lit bomb sitting on the player, flagged not to collide while still.
		eweapon bomb = Screen->CreateEWeapon(EW_BOMB);
		bomb->X = Hero->X;
		bomb->Y = Hero->Y;
		bomb->Step = 0;
		bomb->Damage = 4;
		bomb->Flags[WFLAG_NO_COLL_WHEN_STILL] = true;

		// The lit bomb lies still on the player: no harm done.
		Waitframes(20);
		Test::AssertEqual(Hero->HP, hp, "still lit bomb does not hit");

		// Running out the timeout lights the fuse; the bomb becomes a blast.
		bomb->Timeout = 1;
		Waitframes(60);
		Test::Assert(Hero->HP < hp, "bomb blast hits despite 'No Collision When Still'");

		Test::End();
	}
}

#include "auto/test_runner.zs"

// An enemy firing a weapon that wasn't created (it isn't an enemy weapon) used to change the last
// eweapon instead - which was null when there were none.

generic script bug_breath_invalid_weapon
{
	void run()
	{
		Test::Init();

		npc n = Screen->CreateNPC(NPC_DARKNUT1);
		n->Weapon = LW_SWORD;
		Screen->ClearSprites(SL_EWPNS);
		n->BreathAttack(false);
		Test::AssertEqual(Screen->NumEWeapons(), 0);

		Test::End();
	}
}

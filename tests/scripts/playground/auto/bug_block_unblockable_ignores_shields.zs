#include "std.zh"
#include "auto/test_runner.zs"

// A weapon set to bypass 'block' defense types (UNBLOCK_NORM) also went
// straight through enemy shield flags, as if it had UNBLOCK_SHLD.
generic script bug_block_unblockable_ignores_shields
{
	// Fires an arrow at the enemy and returns how much HP it lost.
	int shootArrow(npc n, int unblockable)
	{
		int hp = n->HP;
		lweapon w = Screen->CreateLWeapon(LW_ARROW);
		w->X = n->X - 32;
		w->Y = n->Y;
		w->Dir = DIR_RIGHT;
		w->Step = 300;
		w->Damage = 4;
		w->Unblockable = unblockable;
		Waitframes(30);
		if (w->isValid())
			Remove(w);
		printf("unblockable %d: damage %d\n", unblockable, hp - n->HP);
		return hp - n->HP;
	}

	void run()
	{
		Test::Init();

		npc n = Screen->CreateNPC(NPC_DARKNUT1);
		n->X = Hero->X + 64;
		n->Y = Hero->Y;
		n->HP = 1000;
		n->Step = 0;
		n->Haltrate = 0;
		n->Defense[NPCD_ARROW] = NPCDT_NONE;
		// Shielded on every side, so it doesn't matter which way it faces.
		n->Flags[NPCF_SHIELD_FRONT] = true;
		n->Flags[NPCF_SHIELD_BACK] = true;
		n->Flags[NPCF_SHIELD_LEFT] = true;
		n->Flags[NPCF_SHIELD_RIGHT] = true;
		// Wait out the spawn animation, during which enemies can't be hit at all.
		Waitframes(60);

		Test::AssertEqual(shootArrow(n, UNBLOCK_NONE), 0, "normal arrow is blocked by the shield");
		Test::AssertEqual(shootArrow(n, UNBLOCK_NORM), 0, "arrow bypassing 'block' defenses is still blocked by the shield");
		Test::Assert(shootArrow(n, UNBLOCK_SHLD) > 0, "arrow bypassing shields hits");

		Test::End();
	}
}

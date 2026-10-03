#include "std.zh"
#include "auto/test_runner.zs"

// A script-type lweapon whose Weapon is set to a bomb blast counts as a bomb
// for secret flags and combo triggers, but it never blew open NES bomb doors.
// Only the engine's own blast did that, on a detonation timer that a script
// weapon never runs.
generic script bug_script_weapon_bomb_door
{
	void run()
	{
		Test::Init();

		Screen->Door[DIR_UP] = D_BOMB;
		Test::AssertEqual(Screen->Door[DIR_UP], D_BOMB, "door starts as a bomb wall");

		// A scripted blast sitting in front of the top door.
		lweapon blast = Screen->CreateLWeapon(LW_SCRIPT1);
		blast->Weapon = LW_BOMBBLAST;
		blast->X = 120;
		blast->Y = 16;

		Waitframes(5);
		Test::AssertEqual(Screen->Door[DIR_UP], D_BOMBED, "scripted bomb blast opens the top door");

		Test::End();
	}
}

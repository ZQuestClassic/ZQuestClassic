#include "std.zh"
#include "auto/test_runner.zs"

// Writing to npc->Shield[] never changed the enemy's shield flags, so scripts
// could only read them.
generic script bug_npc_shield_write
{
	void run()
	{
		Test::Init();

		npc n = Screen->CreateNPC(NPC_OCTOROCK1S);
		for (int i = 0; i < 5; i++)
			Test::AssertEqual(n->Shield[i], false, "octorok starts without shields");

		for (int i = 0; i < 5; i++)
		{
			n->Shield[i] = true;
			Test::AssertEqual(n->Shield[i], true, "shield set");
			n->Shield[i] = false;
			Test::AssertEqual(n->Shield[i], false, "shield cleared");
		}

		Test::End();
	}
}

// npc->Flags[] and npcdata->Flags[] writes to index 31 and above hit the
// wrong bits: setting NPCF_ONLY_LENS (31) also set every flag above it
// (translucent, flickering, ...), and indexes 32+ wrote a low bit instead of
// the intended one.
// https://discord.com/channels/876899628556091432/1530032203625402439

#include "std.zh"
#include "auto/test_runner.zs"

generic script bug_npc_flags_high_bits
{
	void run()
	{
		Test::Init();

		npc enemy = Screen->CreateNPC(NPC_ARMOS);

		// Start from a clean slate (a fresh npc inherits its npcdata flags).
		for (int i = 0; i < NPCF_MAX; i++)
			enemy->Flags[i] = false;
		for (int i = 0; i < NPCF_MAX; i++)
			Test::AssertEqual(enemy->Flags[i], false);

		// Setting each flag sets exactly that flag...
		for (int i = 0; i < NPCF_MAX; i++)
		{
			enemy->Flags[i] = true;
			for (int j = 0; j < NPCF_MAX; j++)
				Test::AssertEqual(enemy->Flags[j], j <= i);
		}

		// ...and clearing each flag clears exactly that flag.
		for (int i = 0; i < NPCF_MAX; i++)
		{
			enemy->Flags[i] = false;
			for (int j = 0; j < NPCF_MAX; j++)
				Test::AssertEqual(enemy->Flags[j], j > i);
		}

		// npcdata->Flags[] goes through the same code and had the same bug.
		npcdata data = Game->LoadNPCData(NPC_ARMOS);
		bool prev = data->Flags[15]; // Where a write to index 47 used to land.
		data->Flags[NPCF_TRANSLUCENT] = true;
		Test::AssertEqual(data->Flags[NPCF_TRANSLUCENT], true);
		Test::AssertEqual(data->Flags[NPCF_ONLY_LENS], false);
		Test::AssertEqual(data->Flags[15], prev);
		data->Flags[NPCF_TRANSLUCENT] = false;

		Test::End();
	}
}

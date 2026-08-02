// Every array on an npcdata read and wrote enemy 0's data instead of the
// npcdata the script actually loaded, because the npcdata array registers were
// missing their REFNPCDATA dependency. Scalar members were unaffected, so
// `npcdata->Attributes[]` disagreed with `npcdata->Anim` about which enemy it
// was talking about.
// https://discord.com/channels/876899628556091432/1533266319401681006

#include "std.zh"
#include "auto/test_runner.zs"

generic script bug_npcdata_arrays_wrong_ref
{
	void run()
	{
		Test::Init();

		// An enemy that ships with "Hammer Can Break Shield" set, which is what
		// the report was about.
		npcdata darknut = Game->LoadNPCData(NPC_DARKNUT1);
		Test::AssertEqual(darknut->Flags[NPCF_SHIELD_BREAKABLE], true);
		Test::AssertEqual(darknut->Shield[4], true);

		// The bug always read npcdata 0, so any two npcdata looked identical.
		// Write through one and check the other doesn't see it.
		npcdata a = Game->LoadNPCData(NPC_OCTOROCK1F);
		npcdata b = Game->LoadNPCData(NPC_TEKTITE1);

		a->Flags[NPCF_ISINVISIBLE] = true;
		b->Flags[NPCF_ISINVISIBLE] = false;
		Test::AssertEqual(a->Flags[NPCF_ISINVISIBLE], true);
		Test::AssertEqual(b->Flags[NPCF_ISINVISIBLE], false);
		a->Flags[NPCF_ISINVISIBLE] = false;

		// Same for the other npcdata arrays.
		a->Attributes[0] = 11;
		b->Attributes[0] = 22;
		Test::AssertEqual(a->Attributes[0], 11);
		Test::AssertEqual(b->Attributes[0], 22);

		a->Defense[0] = 1;
		b->Defense[0] = 2;
		Test::AssertEqual(a->Defense[0], 1);
		Test::AssertEqual(b->Defense[0], 2);

		a->MoveFlags[MV_OBEYS_GRAVITY] = true;
		b->MoveFlags[MV_OBEYS_GRAVITY] = false;
		Test::AssertEqual(a->MoveFlags[MV_OBEYS_GRAVITY], true);
		Test::AssertEqual(b->MoveFlags[MV_OBEYS_GRAVITY], false);

		// messagedata's two arrays were missing the same dependency.
		messagedata m1 = Game->LoadMessageData(1);
		messagedata m2 = Game->LoadMessageData(2);
		m1->Margins[0] = 3;
		m2->Margins[0] = 7;
		Test::AssertEqual(m1->Margins[0], 3);
		Test::AssertEqual(m2->Margins[0], 7);

		Test::End();
	}
}

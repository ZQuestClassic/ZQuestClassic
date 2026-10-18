// Light beams from spotlights reach light targets in every direction, through
// mirrors and magic prisms, and a beam that loops back into a prism doesn't
// freeze the game.

#include "std.zh"
#include "auto/test_runner.zs"

generic script lightbeams
{
	const int SPOT_FFC = 3600;
	const int SPOT_COMBO = 3601;
	const int MIRROR_SLASH = 3602;
	const int MIRROR_BACKSLASH = 3603;
	const int PRISM = 3604;
	const int PRISM4 = 3605;
	// A target becomes the next combo once it's lit.
	const int TARGET = 3606;
	const int TARGET_LIT = 3607;

	void run()
	{
		Test::Init();
		setup_combo(SPOT_FFC, CT_SPOTLIGHT);
		Game->LoadComboData(SPOT_FFC)->UserFlags = 1 << 1; // fires from an FFC
		setup_combo(SPOT_COMBO, CT_SPOTLIGHT);
		setup_combo(MIRROR_SLASH, CT_MIRRORSLASH);
		setup_combo(MIRROR_BACKSLASH, CT_MIRRORBACKSLASH);
		setup_combo(PRISM, CT_MAGICPRISM);
		setup_combo(PRISM4, CT_MAGICPRISM4WAY);
		setup_combo(TARGET, CT_LIGHT_TARGET);
		setup_combo(TARGET_LIT, CT_NONE);
		Hero->X = 240;
		Hero->Y = 160;

		// An FFC spotlight in the middle, aimed in each direction.
		int dx[] = {0, 0, -1, 1};
		int dy[] = {-1, 1, 0, 0};
		for (int dir = 0; dir < 4; dir++)
		{
			clear();
			int target = pos(7 + dx[dir] * 4, 5 + dy[dir] * 4);
			Screen->ComboD[target] = TARGET;
			spot_ffc(7, 5, dir);
			Waitframes(4);
			Test::AssertEqual(Screen->ComboD[target], TARGET_LIT, "an FFC beam reaches its target");
		}

		// A combo spotlight.
		clear();
		Game->LoadComboData(SPOT_COMBO)->Attribytes[0] = DIR_RIGHT;
		Screen->ComboD[pos(2, 5)] = SPOT_COMBO;
		Screen->ComboD[pos(9, 5)] = TARGET;
		Waitframes(4);
		Test::AssertEqual(Screen->ComboD[pos(9, 5)], TARGET_LIT, "a combo beam reaches its target");

		// A loop of mirrors through a 4-way prism: the prism's upward branch
		// comes back around into it.
		//   ..../.....\
		//   .>..4...../
		clear();
		Screen->ComboD[pos(4, 1)] = MIRROR_SLASH;
		Screen->ComboD[pos(10, 1)] = MIRROR_BACKSLASH;
		Screen->ComboD[pos(10, 5)] = MIRROR_SLASH;
		Screen->ComboD[pos(4, 5)] = PRISM4;
		Screen->ComboD[pos(0, 5)] = TARGET;
		Screen->ComboD[pos(4, 10)] = TARGET;
		spot_ffc(1, 5, DIR_RIGHT);
		Waitframes(4);
		Test::AssertEqual(Screen->ComboD[pos(0, 5)], TARGET_LIT, "the loop's left branch is lit");
		Test::AssertEqual(Screen->ComboD[pos(4, 10)], TARGET_LIT, "the loop's down branch is lit");

		// With the broken hitbox rule, light targets send the beam back the way
		// it came, into a 3-way prism.
		clear();
		Game->FFRules[qr_BROKEN_LIGHTBEAM_HITBOX] = true;
		Screen->ComboD[pos(6, 5)] = PRISM;
		Screen->ComboD[pos(0, 5)] = TARGET;
		Screen->ComboD[pos(10, 5)] = TARGET;
		Screen->ComboD[pos(6, 2)] = TARGET;
		Screen->ComboD[pos(6, 8)] = TARGET;
		spot_ffc(2, 5, DIR_RIGHT);
		Waitframes(4);
		Test::AssertEqual(Screen->ComboD[pos(0, 5)], TARGET_LIT, "the left target is lit");
		Test::AssertEqual(Screen->ComboD[pos(10, 5)], TARGET_LIT, "the right target is lit");
		Test::AssertEqual(Screen->ComboD[pos(6, 2)], TARGET_LIT, "the top target is lit");
		Test::AssertEqual(Screen->ComboD[pos(6, 8)], TARGET_LIT, "the bottom target is lit");
		Game->FFRules[qr_BROKEN_LIGHTBEAM_HITBOX] = false;

		Test::End();
	}

	int pos(int x, int y)
	{
		return x + y * 16;
	}

	void setup_combo(int id, int type)
	{
		combodata cd = Game->LoadComboData(id);
		cd->Type = type;
		cd->Walk = 0;
		cd->UserFlags = 0;
		for (int i = 0; i < 8; i++)
			cd->Attribytes[i] = 0;
	}

	void spot_ffc(int x, int y, int dir)
	{
		Game->LoadComboData(SPOT_FFC)->Attribytes[0] = dir;
		ffc f = Screen->LoadFFC(1);
		f->EffectWidth = 16;
		f->EffectHeight = 16;
		f->X = x * 16;
		f->Y = y * 16;
		f->Data = SPOT_FFC;
	}

	void clear()
	{
		Screen->LoadFFC(1)->Data = 0;
		for (int i = 0; i < 176; i++)
			Screen->ComboD[i] = 0;
		Waitframes(2);
	}
}

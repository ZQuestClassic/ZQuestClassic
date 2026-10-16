namespace Test
{
	const int TestingDmap = 5;

	void Init(int screen = 0)
	{
		printf("[Test] started: dmap %d screen %d\n", TestingDmap, screen);
		if (screen != 0)
		{
			Player->Warp(TestingDmap, screen);
			Waitframe();
		}
	}

	void End()
	{
		printf("[Test] done\n");
		Game->Exit();
	}

	void AssertEqual<T>(T actual, T expected)
	{
		if (expected != actual)
		{
			printf("[Test] failed assertion: expected %d but got %d\n", expected, actual);
			Game->Exit();
		}
	}

	void AssertEqual<T>(T actual, T expected, char32[] error)
	{
		if (expected != actual)
		{
			printf("[Test] failed assertion: expected %d but got %d. %s\n", expected, actual, error);
			Game->Exit();
		}
	}

	void Assert<T>(T actual)
	{
		if (!actual)
			Fail("");
	}

	void Assert<T>(T actual, char32[] error)
	{
		if (!actual)
			Fail(error);
	}
	
	void Fail(char32[] error)
	{
		if (strlen(error) > 0)
			printf("[Test] failed test: %s\n", error);
		else
			printf("[Test] failed test\n");
		Game->Exit();
	}

	void loadRegion(int screen, int size)
	{
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		for (int x = 0; x < size; x++)
		{
			for (int y = 0; y < size; y++)
			{
				mapdata scr = Game->LoadMapData(map, screen + x + y*16);
				scr->Valid = 1;
				scr->RegionID = 1;
			}
		}
		Player->Warp(Test::TestingDmap, screen);
		Waitframe();
	}

	void reset(int screen)
	{
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		ClearRegion(map);
		Player->Warp(Test::TestingDmap, 0);
		Waitframe();
	}
}

dmapdata script TestRunner
{
	void run()
	{
		char32 name[100];
		Game->GetSaveName(name);
		printf("[Test] Running %s\n", name);

		int script_id = Game->GetGenericScript(name);
		if (script_id == -1)
		{
			printf("[Test] failed to start - could not find test script\n");
			Game->Exit();
		}

		genericdata gd = Game->LoadGenericData(script_id);
		gd->Running = true;
		while (gd->Running)
		{
			Test::DrawLabel();
			Waitframe();
		}
		Test::End();
	}
}

// Helpers for tests that build their combos and sprites at runtime. These live
// after the scripts above so that adding to them doesn't shift the line numbers
// in every existing replay's traces.
namespace Test
{
	// Turns a combo into a blank one of the given type, with a visible tile.
	combodata setupCombo(int cid, int type)
	{
		combodata cd = Game->LoadComboData(cid);
		cd->Type = type;
		cd->Tile = Game->LoadComboData(12)->Tile;
		cd->Walk = 0;
		for (int i = 0; i < 16; i++)
			cd->Flags[i] = false;
		for (int i = 0; i < 24; i++)
			cd->Attributes[i] = 0;
		// Dropping the triggers resets every field of any the test adds back.
		cd->NumTriggers = 0;
		return cd;
	}

	int pos(int col, int row)
	{
		return col + row * 16;
	}

	int countLWeapons(int id)
	{
		int count = 0;
		for (int i = 1; i <= Screen->NumLWeapons; i++)
		{
			if (Screen->LoadLWeapon(i)->ID == id)
				count++;
		}
		return count;
	}

	int countEWeapons(int id)
	{
		int count = 0;
		for (int i = 1; i <= Screen->NumEWeapons; i++)
		{
			if (Screen->LoadEWeapon(i)->ID == id)
				count++;
		}
		return count;
	}

	int countNPCs(int id)
	{
		int count = 0;
		for (int i = 1; i <= Screen->NumNPCs; i++)
		{
			if (Screen->LoadNPC(i)->ID == id)
				count++;
		}
		return count;
	}

	void clearWeapons()
	{
		for (int i = Screen->NumLWeapons; i > 0; i--)
			Screen->LoadLWeapon(i)->Remove();
		for (int i = Screen->NumEWeapons; i > 0; i--)
			Screen->LoadEWeapon(i)->Remove();
	}

	void clearNPCs()
	{
		for (int i = Screen->NumNPCs; i > 0; i--)
			Screen->LoadNPC(i)->Remove();
	}

	// A line of text drawn at the top of the screen while a test runs, so that
	// its replay can be followed by eye. Tests set it with Label and the
	// TestRunner script draws it every frame. Arrays are garbage collected, so
	// holding the caller's string (even a local or a literal) keeps it alive.
	char32[] labelText;

	void Label(char32[] what)
	{
		labelText = what;
	}

	void DrawLabel()
	{
		if (!labelText)
			return;
		Screen->DrawOrigin = DRAW_ORIGIN_PLAYING_FIELD;
		Screen->DrawString(7, 2, 2, FONT_Z3SMALL, 0x01, 0x0F, TF_NORMAL, labelText, OP_OPAQUE);
	}
}

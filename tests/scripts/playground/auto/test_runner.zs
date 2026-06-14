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

	void AssertEqual(untyped actual, untyped expected)
	{
		if (expected != actual)
		{
			printf("[Test] failed assertion: expected %d but got %d\n", expected, actual);
			Game->Exit();
		}
	}

	void AssertEqual(untyped actual, untyped expected, char32[] error)
	{
		if (expected != actual)
		{
			printf("[Test] failed assertion: expected %d but got %d. %s\n", expected, actual, error);
			Game->Exit();
		}
	}

	void Assert(untyped actual)
	{
		if (!actual)
			Fail("");
	}

	void Assert(untyped actual, char32[] error)
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

	void reset(int screen)
	{
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
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
			Waitframe();
		}

		Test::End();
	}
}

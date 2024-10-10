namespace Test
{
	void Init(int dmap = 5, int screen = 15)
	{
		printf("[Test] started: dmap %d screen %d\n", dmap, screen);
		if (dmap != 5 || screen != 15)
			Player->Warp(dmap, screen);
	}

	void AssertEqual<T>(T actual, T expected)
	{
		if (expected != actual)
		{
			printf("[Test] failed assertion: expected %d but got %d\n", expected, actual);
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
}

screendata script TestRunner
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

		printf("[Test] done\n");
		Game->Exit();
	}
}

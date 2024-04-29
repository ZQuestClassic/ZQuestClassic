#option NO_ERROR_HALT on

class Object {
}

global script Global
{
    void run()
    {
		Object obj = new Object();

		// Invalid.
        int number = obj;
		obj = number;
		int numbers[] = {obj};
		Object objects[] = numbers;
		Object objects2[] = new Object();
		int numbers2[] = {obj, <untyped>1};

		// OK.
		int numbers3[] = {obj, 1};
		Object objects3[] = {obj};
		auto objects4[] = objects3;
		auto numbers4[] = {1, 2, 3};
		untyped mixed1[] = {1, obj, 3};
		auto numbers5[10] = {4, 5, 6};
		auto enemies1 = Screen->NPCs; //is 'const npc[]'
		auto enemies2[] = {enemies1[0], enemies1[2], enemies1[4]}; //is 'npc[]', despite 'const npc[]' elements
		enemies2[2] = enemies1[1];
		const auto enemies3[] = enemies2;
		npc enemy = enemies3[0]; //assert the 'npc' typing passed through correctly
		auto enemy2 = enemies1[0]; //despite 'enemies1[0]' being 'const npc', is 'npc'. 'const' is only absorbed for arrays.
		enemy2 = enemy;
		const auto enemy3 = enemy2; //This adds the 'const' forcibly
		
		// Invalid.
		enemy3 = enemy;
		numbers3 = objects4;
		enemies1[0] = NULL;
		enemies3[2] = enemies1[3];
		auto[][] enemies4 = enemies1[0];
		auto[][][][] enemies5 = enemies1[2];
		auto[][] numbers6 = {1, 2, 3};
		auto x;
    }
}

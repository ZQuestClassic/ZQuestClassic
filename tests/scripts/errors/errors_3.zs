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
		Object objects2[] = new Object(); // TODO: this should be invalid.

		// OK.
		int numbers2[] = {obj, <untyped>1};
		Object objects3[] = {obj};
		auto objects4[] = objects3; // TODO: this should be valid.
		auto numbers3[] = {1, 2, 3}; // TODO: this should be valid.

		// Invalid.
		numbers3 = objects4; // TODO: this should be invalid.
    }
}

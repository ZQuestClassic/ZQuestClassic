#option NO_ERROR_HALT on

class Object {
}

global script Global
{
    void run()
    {
		Object obj = new Object();
		int numbers[] = {1, 2, 3};

		// Invalid.
		Object n1 = ArrayPopBack(numbers);
		ArrayPushBack(numbers, obj);
		ArrayPushBack(numbers, "");
		ArrayPushBack(numbers, 1L); // TODO: this should be invalid (else: we perform an implicit cast and multiply by 10000...)
		Object choice_1 = Choose(1, 2, 3);
		auto choice_2 = Choose(1, 2, obj);
		auto max_1 = Max(1, 2, obj);
		auto max_2 = Max(1, obj);
		auto max_3 = Max(1);

		SizeOfArray(Hero); // TODO: this should warn since the type was coerced to an array (LEGACY_ARRAYS).

		// OK.
		Object objects2[] = {new Object()};
		Object o = ArrayPopBack(objects2);
		int n2 = ArrayPopBack(numbers);
		ArrayPushBack(numbers, 1);
		ArrayPushBack(numbers, <untyped>obj);
		int choice_3 = Choose(1, 2, 3);
		auto choice_4 = Choose(1, 2, 3);
		auto max_2 = Max(1, 2);
		max_2 = Max(1, 2, 3);
		max_2 = Max(1, 2, 3, Max(3, 4));
    }

	void old_ptrs(char32 ptr)
	{
		// For compatability with old scripts, the template deduction code allows
		// matching char32 as an array type by default.

		// OK
		ArrayPushBack(ptr, 1);
		ResizeArray(ptr, 1);

		// OK
		{
			#option LEGACY_ARRAYS on

			ArrayPushBack(ptr, 1);
			ResizeArray(ptr, 1);
		}

		// Errors
		{
			#option LEGACY_ARRAYS off

			ArrayPushBack(ptr, 1);
			ResizeArray(ptr, 1);
		}
	}
}

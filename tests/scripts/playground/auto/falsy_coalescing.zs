#include "auto/test_runner.zs"

generic script falsy_coalescing
{
	int global_side_effect_count = 0;

	int trigger_side_effect(int return_val)
	{
		global_side_effect_count++;
		return return_val;
	}

	void run()
	{
		Test::Init();

		Waitframe();

		// Basics.
		Test::AssertEqual(1 ?: 2, 1);
        Test::AssertEqual(0 ?: 2, 2);
        Test::AssertEqual(3 ?: 8, 3);
        Test::AssertEqual(0 ?: 0 ?: 4, 4);
        Test::AssertEqual(0 ?: 5 ?: 2, 5);

		// Operator precedence.
		// Should parse as (0) ?: (2 + 3), NOT (0 ?: 2) + 3
		Test::AssertEqual(0 ?: 2 + 3, 5); 
		// Should parse as (1 + 0) ?: 3, NOT 1 + (0 ?: 3)
		Test::AssertEqual(1 + 0 ?: 3, 1);

        int x = 6;
        x ?:= 8;
		Test::AssertEqual(x, 6);

        x = 0;
        x ?:= 7;
		Test::AssertEqual(x, 7);

        sprite spr;
		Test::AssertEqual((spr ?: Hero)->X, Hero->X);

		// Short-circuit evaluation.
		global_side_effect_count = 0;
		int a = 5 ?: trigger_side_effect(10);
		if (global_side_effect_count != 0)
			Test::Fail("Right side evaluated when left was truthy!");

		a = 0 ?: trigger_side_effect(10);
		if (global_side_effect_count != 1)
			Test::Fail("Right side failed to evaluate when left was falsy!");

		// Assignment short-circuiting.
		global_side_effect_count = 0;
		x = 6;
		x ?:= trigger_side_effect(8);
		if (global_side_effect_count != 0)
			Test::Fail("Assignment right side evaluated when left was truthy!");
		Test::AssertEqual(x, 6);

		x = 0;
		x ?:= trigger_side_effect(7);
		if (global_side_effect_count != 1)
			Test::Fail("Assignment right side failed to evaluate when left was falsy!");
		Test::AssertEqual(x, 7);

		// L-value single evaluation.
		global_side_effect_count = 0;
		int arr[3];
		arr[trigger_side_effect(0)] ?:= 99;
		// TODO: this doesn't actually work. And neither would `arr[trigger_side_effect(0)] += 99`.
		// if (global_side_effect_count > 1)
		// 	Test::Fail("L-value was evaluated multiple times during assignment!");
		Test::AssertEqual(arr[0], 99);

		Test::End();
	}
}

#include "std.zh"
#include "auto/test_runner.zs"

generic script setup_before
{
	const int CONST = 11037;
	int a = 1;
	int b = a;
	static int c = 2;
	static int d = c;
	void run()
	{
		Test::Assert(this);
		test(1, 1);
		static_test(2, 2);
		a = 5;
		b = 8;
		c = 42;
		d = 420;
		test(5, 8);
		static_test(42, 420);
		Test::AssertEqual(CONST, 11037);
	}
	void test(int ex_a, int ex_b)
	{
		// Test::Assert(this); // Error: can't access instance vars without instance
		Test::AssertEqual(a, ex_a);
		Test::AssertEqual(b, ex_b);
	}
	static void static_test(int ex_c, int ex_d)
	{
		// Test::Assert(this); // Error: can't access instance vars without instance
		Test::AssertEqual(c, ex_c);
		Test::AssertEqual(d, ex_d);
	}
}

generic script setup_after
{
	#option DEFAULT_STATIC_SCRIPT_MEMBERS off
	const int CONST = 11037;
	int a = 1;
	int b = 1; // can't assign to 'a' anymore!
	static int c = 2;
	static int d = c;
	void run()
	{
		Test::Assert(this);
		test(1, 1);
		static_test(2, 2);
		a = 5;
		b = 8;
		c = 42;
		d = 420;
		test(5, 8);
		static_test(42, 420);
		Test::AssertEqual(CONST, 11037);
	}
	void test(int ex_a, int ex_b)
	{
		Test::Assert(this); // can now access 'this' in non-static script-scope functions!
		Test::AssertEqual(a, ex_a);
		Test::AssertEqual(b, ex_b);
	}
	static void static_test(int ex_c, int ex_d)
	{
		// Test::Assert(this); // Error: can't access instance vars without instance
		Test::AssertEqual(c, ex_c);
		Test::AssertEqual(d, ex_d);
	}
}

void test_before()
{
	setup_before.test(5, 8);
	setup_before.static_test(42, 420);
	setup_before.a = 7;
	setup_before.b = 42;
	setup_before.c = 3;
	setup_before.d = 9;
	setup_before.test(7, 42);
	setup_before.static_test(3, 9);
	Test::AssertEqual(setup_before.CONST, 11037);
}

void test_after()
{
	// setup_after.test(5, 8);
	setup_after.static_test(42, 420);
	// setup_after.a = 7;
	// setup_after.b = 42;
	setup_after.c = 3;
	setup_after.d = 9;
	// setup_after.test(7, 42);
	setup_after.static_test(3, 9);
	Test::AssertEqual(setup_after.CONST, 11037); // constants can be statically accessed
}

generic script script_instance_vars
{
	void run()
	{
		Test::Init();
		Waitframe();
		
		int before = CheckGenericScript("setup_before");
		int after = CheckGenericScript("setup_after");
		
		Test::Assert(before);
		Test::Assert(after);
		
		auto before_gd = RunGenericScript(before);
		while (before_gd->Running)
			Waitframe();
		test_before();
		
		auto after_gd = RunGenericScript(after);
		while (after_gd->Running)
			Waitframe();
		test_after();
	}
}

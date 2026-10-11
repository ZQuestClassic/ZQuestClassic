#include "std.zh"
#include "auto/test_runner.zs"

// Tests user-defined template functions (ex: `T f<T>(T x)`).

// Copies `x` into a local of type T, then reassigns it.
T tmpl_copy<T>(T x)
{
	T copy = x;
	copy = x;
	return copy;
}

// Copies the first variadic argument into a local of type T, then reassigns it.
T tmpl_copy_first<T>(...T[] args)
{
	T copy = args[0];
	copy = args[0];
	return copy;
}

class TemplateMember
{
	int value;

	void set<T>(T x)
	{
		value = <int>x;
	}

	TemplateMember self<T>(T unused)
	{
		return this;
	}
}

T tmpl_add<T>(T x, T amount = 2)
{
	return x + amount;
}

// Copies `arr[0]` into a local of type T, then reassigns it to `arr[1]`.
T tmpl_first<T>(T[] arr)
{
	T copy = arr[0];
	copy = arr[1];
	return copy;
}

// Copies the first element of the first variadic array into a local of type T, then
// reassigns it to the second.
T tmpl_first_of_first<T>(...T[][] args)
{
	T copy = args[0][0];
	copy = args[0][1];
	return copy;
}

generic script templates
{
	void run()
	{
		Test::Init();

		{
			const int CONST_VAL = 5;
			// Passing a constant used to bind T to `const int`, which made the
			// template's own locals of type T constant (a compile error).
			Test::AssertEqual(tmpl_copy(CONST_VAL), 5, "T bound from a constant");
			Test::AssertEqual(tmpl_copy(7), 7, "T bound from a literal");
			Test::AssertEqual(tmpl_copy_first(CONST_VAL, 6), 5, "T bound from a constant vararg");
		}

		{
			const int CONST_ARR[] = {3, 4};
			// The array's elements being constant used to bind T to `const int`.
			Test::AssertEqual(tmpl_first(CONST_ARR), 4, "T bound from a constant array");
			Test::AssertEqual(tmpl_first_of_first(CONST_ARR), 4, "T bound from a constant vararg array");
		}

		{
			// Member template functions used to be analyzed outside of their class,
			// so they could not use its members (or `this`).
			TemplateMember m = new TemplateMember();
			m->set(42);
			Test::AssertEqual(m->value, 42, "member template writes a member");
			Test::AssertEqual(m->self(1)->value, 42, "member template returns this");
		}

		{
			// A default value for a parameter of type T used to be rejected.
			Test::AssertEqual(tmpl_add(1), 3, "T-typed default parameter");
			Test::AssertEqual(tmpl_add(1, 5), 6, "T-typed default parameter, given");
		}

		Test::End();
	}
}

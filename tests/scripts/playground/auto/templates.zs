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

T tmpl_peek<T>(stack<T> s)
{
	return s->PeekBack();
}

T tmpl_peek_const<T>(const stack<T> s)
{
	return s->PeekBack();
}

void tmpl_push<T>(stack<T> s, T value)
{
	s->PushBack(value);
}

stack<T> tmpl_make_stack<T>(T value)
{
	stack<T> s = new stack<T>();
	s->PushBack(value);
	return s;
}

T tmpl_peek_first<T>(stack<T>[] stacks)
{
	return stacks[0]->PeekBack();
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

// Overloads where the template's body only compiles for class types, so calls that
// resolve to the plain function must not instantiate the template.
int tmpl_overload(int x)
{
	return 1;
}

int tmpl_overload<T>(T x)
{
	return x->value;
}

int tmpl_overload_mismatch(float x, int y)
{
	return 2;
}

int tmpl_overload_mismatch<T>(T x, TemplateMember y)
{
	return x->value;
}

// Constructors calling another constructor of the class, where one of them is a template.
class TemplateCtor
{
	int value;

	TemplateCtor()
	{
		TemplateCtor(5);
	}

	TemplateCtor<T>(T x)
	{
		value = <int>x;
	}

	TemplateCtor<T>(T x, T y)
	{
		TemplateCtor(<int>x, <int>y, 0);
	}

	TemplateCtor(int a, int b, int c)
	{
		value = a + b + c;
	}
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

		{
			// Template functions taking a class template of T (ex: `stack<T>`)
			// could never be called.
			stack<int> si = new stack<int>();
			tmpl_push(si, 5);
			Test::AssertEqual(tmpl_peek(si), 5, "stack<T> parameter");
			Test::AssertEqual(tmpl_peek_const(si), 5, "const stack<T> parameter");
			const stack<int> csi = si;
			Test::AssertEqual(tmpl_peek_const(csi), 5, "const stack<T> parameter, const argument");
			stack<int> made = tmpl_make_stack(7);
			Test::AssertEqual(made->PeekBack(), 7, "stack<T> return type");
			stack<int> stacks[1];
			stacks[0] = si;
			Test::AssertEqual(tmpl_peek_first(stacks), 5, "stack<T>[] parameter");
			stack plain = new stack();
			plain->PushBack(3);
			Test::AssertEqual(<int>(tmpl_peek(plain)), 3, "plain stack binds T to untyped");
		}

		{
			// A template overload losing to another function (or not matching the call
			// at all) used to have its body compiled anyway, which failed here since
			// `int` has no `value`.
			TemplateMember m = new TemplateMember();
			m->set(9);
			Test::AssertEqual(tmpl_overload(5), 1, "plain overload beats template");
			Test::AssertEqual(tmpl_overload(m), 9, "template overload");
			Test::AssertEqual(tmpl_overload_mismatch(1, 2), 2, "template overload doesn't match");
			Test::AssertEqual(tmpl_overload_mismatch(m, m), 9, "template overload matches");
		}

		{
			// Calling a template constructor from another constructor used to fail to
			// compile.
			Test::AssertEqual(new TemplateCtor()->value, 5, "constructor calls template constructor");
			Test::AssertEqual(new TemplateCtor(2, 3)->value, 5, "template constructor calls constructor");
		}

		Test::End();
	}
}

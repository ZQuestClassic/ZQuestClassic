#include "std.zh"
#include "auto/test_runner.zs"

int widget_count = 0;

class Widget
{
	Widget() { widget_count++; }
	~Widget() { widget_count--; }
}

// A user-defined varargs function whose element type is an object. The vargs
// are materialized into a script array via do_varg_makearray.
Widget[] collect(...Widget[] items)
{
	return items;
}

// Same, but the element type is untyped: the materialized array holds a mix of
// objects and plain ints, and must retain only the object elements.
untyped[] collectUntyped(...untyped[] items)
{
	return items;
}

int sum(...int[] args)
{
	int sum = 0;
	for (int i = 0; i < SizeOfArray(args); i++)
		sum += args[i];
	return sum;
}

int sumAndMultiply(int multiplier, ...int[] args)
{
	int sum = 0;
	for (int i = 0; i < SizeOfArray(args); i++)
		sum += args[i];
	return sum * multiplier;
}

class TestClass
{
	int sum(...int[] args)
	{
		int sum = 0;
		for (int i = 0; i < SizeOfArray(args); i++)
			sum += args[i];
		return sum;
	}

	int sumAndMultiply(int multiplier, ...int[] args)
	{
		int sum = 0;
		for (int i = 0; i < SizeOfArray(args); i++)
			sum += args[i];
		return sum * multiplier;
	}
}

int nop(int val)
{
	return Choose(val); // attempt to clobber vargs
}

int wait_op(int val)
{
	Waitframe();
	return Choose(val); // attempt to clobber vargs
}

generic script varargs
{
	void run()
	{
		Test::Init();

		Waitframe();

		Test::AssertEqual(sum(), 0);
		Test::AssertEqual(sum(1), 1);
		Test::AssertEqual(sum(1, 2), 3);
		Test::AssertEqual(sum(1, 2, 3), 6);

		Test::AssertEqual(sumAndMultiply(1), 0);
		Test::AssertEqual(sumAndMultiply(1, 2, 3), 5);
		Test::AssertEqual(sumAndMultiply(2, 2, 3), 10);

		auto c = new TestClass();

		Test::AssertEqual(c->sum(), 0);
		Test::AssertEqual(c->sum(1), 1);
		Test::AssertEqual(c->sum(1, 2), 3);
		Test::AssertEqual(c->sum(1, 2, 3), 6);

		Test::AssertEqual(c->sumAndMultiply(1), 0);
		Test::AssertEqual(c->sumAndMultiply(1, 2, 3), 5);
		Test::AssertEqual(c->sumAndMultiply(2, 2, 3), 10);

		Test::AssertEqual(Max(1, 2), 2);
		Test::AssertEqual(Max(1, 2, 3), 3);
		Test::AssertEqual(Max(5, 1, 2, 3), 5);
		Test::AssertEqual(Max(5, 1, Min(100, 99), 3), 99);
		
		nest();
		objectVargs();
		untypedVargs();

		Test::End();
	}
	// Object-typed varargs are materialized into a script array. That array must
	// retain its object elements, so they survive after every other reference
	// (the local variable and the autorelease pool) is gone.
	void objectVargs()
	{
		Waitframe();

		{
			Widget[] arr;
			{
				Widget w = new Widget();
				arr = collect(w);
				Test::AssertEqual(widget_count, 1, "alive right after collect");
				Test::AssertEqual(SizeOfArray(arr), 1, "array size");
			}
			// `w` is out of scope now. Drain the autorelease pool.
			Waitframe();
			// Only `arr` should be keeping the element alive.
			Test::AssertEqual(widget_count, 1, "array must retain its element");
			Test::AssertEqual(RefCount(arr[0]), 1L, "element retained solely by array");
		}

		// `arr` is gone; the element should now be freed exactly once.
		Waitframe();
		Test::AssertEqual(widget_count, 0, "cleaned up with no double-free");
	}
	// Untyped varargs materialize into an `untyped[]` array holding a mix of
	// objects and plain ints. The array must retain only its object elements
	// (tracked per-position), not the ints.
	void untypedVargs()
	{
		Waitframe();

		{
			untyped[] arr;
			{
				Widget w = new Widget();
				arr = collectUntyped(w, 42);
				Test::AssertEqual(widget_count, 1, "alive right after collectUntyped");
				Test::AssertEqual(SizeOfArray(arr), 2, "array size");
			}
			// `w` is out of scope now. Drain the autorelease pool.
			Waitframe();
			// Only `arr` should be keeping the object element alive.
			Test::AssertEqual(widget_count, 1, "untyped array must retain its object element");
			Test::AssertEqual(RefCount(arr[0]), 1L, "object element retained solely by array");
			Test::AssertEqual(arr[1], 42, "int element preserved");
		}

		// `arr` is gone; the element should now be freed exactly once.
		Waitframe();
		Test::AssertEqual(widget_count, 0, "cleaned up with no double-free");
	}
	void nest()
	{
		// don't warn about `Untype()` being deprecated
		#option WARN_DEPRECATED off
		
		// test various levels of nesting
		char32 buf[1];
		sprintf(buf, "%d, %d, %d",
			sum(Max(1, -50), Min(1, 20), Max(3, 1)),
			Min(Max(-50, Untype(Untype(Untype(1)))), nop(nop(nop(20)))),
			Max(1, Min(20, 500)));
		Test::AssertEqual(strcmp(buf, "5, 1, 20"), 0, buf);
		
		sprintf(buf, "%s, %ad", "FooBar", {1,2,3,4});
		Test::AssertEqual(strcmp(buf, "FooBar, { 1, 2, 3, 4 }"), 0, buf);
		sprintf(buf, "%s, %ad", "FooBar", {1,2,nop(3),4});
		Test::AssertEqual(strcmp(buf, "FooBar, { 1, 2, 3, 4 }"), 0, buf);
		
		// ERROR: The string literal gets autoreleased before the sprintf call
		// sprintf(buf, "%s, %d, %d, %d, %s", "foo", 5, wait_op(42), 10, "bar");
		// for now, store the string in a variable to work around
		char32 fmt[] = "%s, %d, %d, %d, %s";
		sprintf(buf, fmt, "foo", 5, wait_op(42), 10, "bar");
		Test::AssertEqual(strcmp(buf, "foo, 5, 42, 10, bar"), 0, buf);
	}
}

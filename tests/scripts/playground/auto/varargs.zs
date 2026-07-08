#include "auto/test_runner.zs"

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

		Test::End();
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

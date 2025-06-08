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
	}
}

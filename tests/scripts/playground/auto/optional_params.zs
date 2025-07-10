#include "auto/test_runner.zs"

generic script optional_params
{
	int A = 1337;

	int fn1(int b, int a = A) {
		return a;
	}

	int fn2(int a = A + 100) {
		return a;
	}

	int fn3(int b, int a = 1) {
		return a;
	}

	int fn4(int a = Player->X) {
		return a;
	}

	int fn5(int a = Player->X + 1) {
		return a;
	}

	void fn6(char32[] a = "fn6 default") {
		Trace(a);
	}

	char32[] fn7(char32[] a = "fn7 default") {
		return a;
	}

	int fn8(int a = fn2()) {
		return a;
	}

	void run()
	{
		Test::Init();

		Test::AssertEqual(fn1(0), 1337);
		A += 1;
		Test::AssertEqual(fn1(0), 1338);
		Test::AssertEqual(fn1(0, 2), 2);

		Test::AssertEqual(fn2(), A + 100);
		Test::AssertEqual(fn2(1), 1);

		Test::AssertEqual(fn3(0), 1);
		Test::AssertEqual(fn3(0, 2), 2);

		Player->X = 123;
		Test::AssertEqual(fn4(), 123);
		Player->X = 10;
		Test::AssertEqual(fn4(), 10);
		Test::AssertEqual(fn4(2), 2);

		Test::AssertEqual(fn5(), 11);

		// TODO: for some reason this prints an empty string on windows 32bit.
		// fn6();
		fn6("fn6 not default");

		Trace(fn7());
		Trace(fn7("fn7 not default"));

		// Different objects.
		auto def1 = fn7();
		auto def2 = fn7();
		Test::Assert(def1 != def2);

		Test::AssertEqual(fn8(), A + 100);
		Test::AssertEqual(fn8(123), 123);
	}
}

// https://discord.com/channels/876899628556091432/1477128675546763294

#include "auto/test_runner.zs"

class empty_ctor
{
	int X, Y;

	empty_ctor()
	{
	}
}

class default_ctor
{
	int X;

	void setX()
	{
		X = 123;
	}
}

generic script bug_empty_constructor
{
	void run()
	{
		Test::Init();

		auto v1 = new empty_ctor();
		Test::Assert(v1);
		v1->X = 8;
        v1->Y = 8;
		Test::AssertEqual(v1->X, 8);
		Test::AssertEqual(v1->Y, 8);

		auto v2 = new default_ctor();
		Test::Assert(v2);
		Test::AssertEqual(v2->X, 0);
		v2->setX();
		Test::AssertEqual(v2->X, 123);

		Test::End();
	}
}

// https://discord.com/channels/876899628556091432/1444414136409919508

#include "auto/test_runner.zs"

class foo
{
	int value;

	foo()
	{
		value = 3;
		clobber(this);
		Test::AssertEqual(value, 3, "in ctor");
	}

	foo waitAndClobber()
	{
		value = 3;
		Waitframe();
		Test::AssertEqual(value, 3, "in waitAndClobber");
		return this;
	}
}

void clobber(foo f)
{
	Trace("clobber");
}

generic script bug_object_deleted_during_constructor
{
	void run()
	{
		Test::Init();

		auto f = new foo();
		Test::AssertEqual(f->value, 3, "after ctor");

		Test::AssertEqual(new foo()->value, 3, "unnamed");

		Test::AssertEqual(new foo()->waitAndClobber()->value, 3, "after waitAndClobber");

		Test::End();
	}
}

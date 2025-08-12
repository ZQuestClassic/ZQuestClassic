// https://discord.com/channels/876899628556091432/1403787778461794354

#include "auto/test_runner.zs"

class Outer0
{
    InnerDefaultCtorNoFields b;
    Outer0()
    {
        b = new InnerDefaultCtorNoFields();
    }
}

class InnerDefaultCtorNoFields
{
}

class Outer1
{
	InnerDefaultCtor b;
	Outer1()
	{
		b = new InnerDefaultCtor();
		b->val += 123;
	}
}

class InnerDefaultCtor
{
	int val;
}

class Outer2
{
	InnerUserCtor b;
	Outer2()
	{
		b = new InnerUserCtor();
		b->val += 123;
	}
}

class InnerUserCtor
{
	int val;

	InnerUserCtor()
	{
		val = 1;
	}
}

generic script bug_nested_class_constructors
{
	void run()
	{
		Test::Init();

		// auto o0 = new Outer0();

		auto o1 = new Outer1();
		Test::AssertEqual(o1->b->val, 123);

		auto o2 = new Outer2();
		Test::AssertEqual(o2->b->val, 124);

		Test::End();
	}
}

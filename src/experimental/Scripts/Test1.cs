
#include "Defines.h"


enum Test1Enum
{
	A,
	B,
	C,
	D
}


class Test1
{
	int a;
	int b;

	Test1()
	{
		a = 10;
		b = 20;
	}

	void run()
	{
		Test1 test1;// = new Test1(); // no value assign
		test1.a = b;
		test1.b = a;

		//@this = test1; // not lvalue

		//Print(a); //todo
		//Print(b);
	}
}





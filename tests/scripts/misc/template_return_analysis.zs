// A template function's instantiations get the same return analysis as a regular
// function: Tmpl<int> and Plain should compile to the same code. Neither should store
// `unused`, and neither should have an epilogue after calling A, which never returns.
// (A is only found to never return on a second pass over the functions, since it is
// analyzed before B.)

void A()
{
	B();
}

void B()
{
	while (true)
		Waitframe();
}

void Tmpl<T>(T x)
{
	int unused = 5;
	A();
}

void Plain(int x)
{
	int unused = 5;
	A();
}

ffc script template_return_analysis
{
	void run(int which)
	{
		if (which)
			Tmpl(1);
		else
			Plain(1);
	}
}

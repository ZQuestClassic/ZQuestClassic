ffc script foo
{
	int a = 1;
	int b = a;
	static int c = 2;
	static int d = c;
	void run()
	{
		Trace(this);
		print();
		static_print();
		a = 5;
		b = 8;
		c = 42;
		d = 420;
		Trace(a);
		Trace(b);
		Trace(c);
		Trace(d);
	}
	void print()
	{
		// Trace(this); // Error: can't access instance vars without instance
		Trace(a);
		Trace(b);
	}
	static void static_print()
	{
		// Trace(this); // Error: can't access instance vars without instance
		Trace(c);
		Trace(d);
	}
}

ffc script bar
{
	void run()
	{
		foo.print();
		foo.static_print();
		foo.a = 7;
		foo.b = 42;
		foo.c = 3;
		foo.d = 9;
		Trace(foo.a);
		Trace(foo.b);
		Trace(foo.c);
		Trace(foo.d);
	}
}


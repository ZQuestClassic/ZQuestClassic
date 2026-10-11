// A template function that calls itself with an ever-growing type would
// instantiate forever. This used to hang the compiler.
void grow<T>(T x, int n)
{
	if (n > 0)
		grow({x}, n - 1);
}

// The nesting limit only applies to template instantiations - this is fine.
void deep(int[][][][][][][][][][][][][][][][][][][][] x) {}

generic script errors_templates
{
	void run()
	{
		grow(1, 3);
		int[][][][][][][][][][][][][][][][][][][][] x;
		deep(x);
	}
}

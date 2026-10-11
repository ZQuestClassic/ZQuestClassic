// A template function growing several types independently instantiates every
// combination of them. This used to run out of memory (and appear to hang) long
// before any one type got too deeply nested.
void growEach<T, U, V, W>(T x, U y, V z, W w, int n)
{
	if (n > 0)
	{
		growEach({x}, y, z, w, n - 1);
		growEach(x, {y}, z, w, n - 1);
		growEach(x, y, {z}, w, n - 1);
		growEach(x, y, z, {w}, n - 1);
	}
}

// Same, but with a non-template overload that matches some of the calls once the
// template can't be instantiated any more. This used to crash the compiler.
void growEachWithFallback(int[][] x, untyped y, untyped z, untyped w) {}
void growEachWithFallback<T, U, V, W>(T x, U y, V z, W w)
{
	growEachWithFallback({x}, y, z, w);
	growEachWithFallback(x, {y}, z, w);
	growEachWithFallback(x, y, {z}, w);
	growEachWithFallback(x, y, z, {w});
}

generic script errors_template_instantiations
{
	void run()
	{
		growEach(1, 2, 3, 4, 3);
		growEachWithFallback(1, 2, 3, 4);
	}
}

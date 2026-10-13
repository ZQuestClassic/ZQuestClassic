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

// Same as growing one type too deep (see errors_templates.zs), but with a
// non-template overload that matches the call once the template can't be
// instantiated any more. This used to crash the compiler. (An object type, since
// LEGACY_ARRAYS would let the overload take an `int` right away.)
void growWithFallback(bitmap[][][][][][][][][][][][][][][][][] x) {}
void growWithFallback<T>(T x)
{
	growWithFallback({x});
}

generic script errors_template_instantiations
{
	void run()
	{
		growEach(1, 2, 3, 4, 3);
		bitmap b;
		growWithFallback(b);
	}
}

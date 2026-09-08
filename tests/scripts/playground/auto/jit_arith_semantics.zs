// Differential test of the script arithmetic the JITs compile natively. Every
// value below is computed from runtime operands (so neither the compiler nor
// the ZASM optimizer folds it) and printed; the recorded replay holds the
// interpreter's results, so any JIT lowering that disagrees with the
// interpreter fails the replay. Covers signed division/modulo (including
// power-of-two divisors and runtime zero divisors), 64-bit multiply
// intermediates, the fixed-point bitwise ops, shifts by in-range counts,
// Abs/Min/Max/Floor/Ceiling, comparisons, switch dispatch on odd keys, nested
// switches with calls, and bool equality across differently-encoded truthy
// values.
//
// Only defined behavior is exercised: shift counts stay within 0..31.

#include "auto/test_runner.zs"

generic script jit_arith_semantics
{
	void p(char32[] label, int v) { printf("P %s = %d\n", label, v); }
	void p(char32[] label, bool v) { printf("P %s = %d\n", label, <int>v); }
	void pl(char32[] label, long v) { printf("P %s = %ld\n", label, v); }

	int passthru(int v) { return v; }

	void run()
	{
		Test::Init();

		int vals[] = {-3, -1.5, -0.0001, 0, 0.0001, 1.5, 3, 7.25, -7.25, 214748.3647, -214748.3648, 100000, -100000, 0.5, -0.5, 2, -2, 65536, -65536};
		int n = 19;
		int shift_counts[] = {0, 1, 3, 7, 31};
		for (int i = 0; i < n; i++)
		{
			int x = vals[i];
			printf("--- x = %d\n", x);
			p("x % 2", x % 2);
			p("x % 0.0002", x % 0.0002);
			p("x % 6.5536", x % 6.5536);
			p("x % -2", x % -2);
			p("x % -0.0001", x % -0.0001);
			p("x % 0.0001", x % 0.0001);
			p("x % 3", x % 3);
			p("x % 1.5", x % 1.5);
			p("x / 3", x / 3);
			p("x / -3", x / -3);
			p("x / 0.0001", x / 0.0001);
			p("x / 0.5", x / 0.5);
			p("3 / x", 3 / x);
			p("x * 3", x * 3);
			p("x * -1.5", x * -1.5);
			p("x * 214748.3647", x * 214748.3647);
			p("x & 3", x & 3);
			p("x & -2", x & -2);
			p("x & 0.5", x & 0.5);
			p("x | 3", x | 3);
			p("x | -2", x | -2);
			p("x ^ 3", x ^ 3);
			p("x ^ -1", x ^ -1);
			p("~x", ~x);
			p("x << 1", x << 1);
			p("x << 3", x << 3);
			p("x >> 1", x >> 1);
			p("x >> 3", x >> 3);
			p("x >> 31", x >> 31);
			p("Abs(x)", Abs(x));
			p("Min(x, 1)", Min(x, 1));
			p("Max(x, 1)", Max(x, 1));
			p("Min(x, -1)", Min(x, -1));
			p("Max(x, -1)", Max(x, -1));
			p("Floor(x)", Floor(x));
			p("Ceiling(x)", Ceiling(x));
			p("x != 0", x != 0);
			p("!x", !x);
			p("(bool)x == true", <bool>x == true);
			p("-x", -x);
			p("3 - x", 3 - x);
			int y = vals[(i + 5) % n];
			p("x % y", x % y);
			p("x / y", x / y);
			p("x * y", x * y);
			p("x & y", x & y);
			p("x | y", x | y);
			p("x ^ y", x ^ y);
			p("Min(x,y)", Min(x, y));
			p("Max(x,y)", Max(x, y));
			p("x < y", x < y);
			p("x <= y", x <= y);
			p("x == y", x == y);
			p("x != y", x != y);
			p("x - y", x - y);
			p("x + y", x + y);
			p("x % 2 == y % 2", x % 2 == y % 2);
			int zero = vals[3];
			p("x / zero", x / zero);
			p("x % zero", x % zero);
			p("-3 / zero", -3 / zero);
			for (int s = 0; s < 5; s++)
			{
				int c = shift_counts[s];
				p("x << c", x << c);
				p("x >> c", x >> c);
				p("1 << c", 1 << c);
				p("-8 >> c", -8 >> c);
			}
		}

		long lvals[] = {-3L, -1L, 0L, 1L, 3L, 65536L, -65536L, 2147483647L, -2147483648L, 12345678L, -12345678L};
		int ln = 11;
		for (int i = 0; i < ln; i++)
		{
			long x = lvals[i];
			printf("--- lx = %ld\n", x);
			pl("lx % 2L", x % 2L);
			pl("lx % 4L", x % 4L);
			pl("lx % 65536L", x % 65536L);
			pl("lx % -4L", x % -4L);
			pl("lx % 3L", x % 3L);
			pl("lx % 1L", x % 1L);
			pl("lx % -1L", x % -1L);
			pl("lx / 3L", x / 3L);
			pl("lx / -1L", x / -1L);
			pl("lx * 3L", x * 3L);
			pl("lx * lx", x * x);
			pl("lx & 3L", x & 3L);
			pl("lx & -4L", x & -4L);
			pl("lx | 5L", x | 5L);
			pl("lx ^ -1L", x ^ -1L);
			pl("~lx", ~x);
			pl("lx << 1L", x << 1L);
			pl("lx << 31L", x << 31L);
			pl("lx >> 1L", x >> 1L);
			pl("lx >> 31L", x >> 31L);
			long y = lvals[(i + 3) % ln];
			pl("lx % ly", x % y);
			pl("lx / ly", x / y);
			pl("lx & ly", x & y);
			pl("lx | ly", x | y);
			pl("lx ^ ly", x ^ y);
			pl("Abs(lx)", Abs(x));
			long lzero = lvals[2];
			pl("lx / lzero", x / lzero);
			pl("lx % lzero", x % lzero);
		}

		// Switch dispatch with odd keys.
		int keys[] = {-3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1.5, -1.5, 0.0001, -0.0001, 214748.3647, -214748.3648, 100, -100, 2.9999, 3.0001};
		for (int i = 0; i < 24; i++)
		{
			int k = keys[i];
			int r = 0;
			switch (k)
			{
				case 0: r = 1; break;
				case 1: r = 2; break;
				case 2: r = 3; break;
				case 3: r = 4; break;
				case 4: r = 5; break;
				case 5: r = 6; break;
				case 6: r = 7; break;
				case 7: r = 8; break;
				default: r = -1; break;
			}
			int r2 = 0;
			switch (k)
			{
				case -3: r2 = 1; break;
				case -2: r2 = 2; break;
				case -1: r2 = 3; break;
				case 0: r2 = 4; break;
				case 1: r2 = 5; break;
				case 2: r2 = 6; break;
				case 3: r2 = 7; break;
				case 4: r2 = 8; break;
			}
			int r3 = 0;
			switch (k)
			{
				case 0...3: r3 = 1; break;
				case 5...7: r3 = 2; break;
				case 100: r3 = 3; break;
				case -100...-50: r3 = 4; break;
				default: r3 = -1;
			}
			int r4 = 0;
			switch (k)
			{
				case 1000: r4 = 1; break;
				case 2000: r4 = 2; break;
				case 3000: r4 = 3; break;
				case 0: r4 = 4; break;
				case 1: r4 = 5; break;
			}
			printf("SW k=%d %d %d %d %d\n", k, r, r2, r3, r4);
		}

		// Nested switch inside a loop, with a call between key store and cases.
		int acc = 0;
		for (int i = 0; i < 4; i++)
		{
			switch (passthru(i))
			{
				case 0:
					switch (passthru(i + 1))
					{
						case 1: acc += 1; break;
						default: acc += 1000; break;
					}
					break;
				case 1: acc += 10; break;
				case 2:
					switch (passthru(i + 1))
					{
						case 3: acc += 100; break;
						default: acc += 1000; break;
					}
					break;
				default: acc += 10000; break;
			}
		}
		printf("NESTED acc=%d\n", acc);

		// Bool equality with different raw truthy values.
		bool b1 = vals[6] != 0;
		bool b2 = <bool>vals[6];
		bool b3 = true;
		printf("BOOL %d %d %d eq12=%d eq13=%d eq23=%d ne12=%d\n", b1, b2, b3, b1 == b2, b1 == b3, b2 == b3, b1 != b2);
		int tv = vals[6];
		bool b4 = tv;
		printf("BOOL2 %d %d %d\n", b4, b4 == b3, b4 == (tv > 0));

		Test::End();
	}
}

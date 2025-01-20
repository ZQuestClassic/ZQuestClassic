namespace A
{
	enum Natural
	{
		A, B, C, D, E
	};
	CONST_ASSERT(A == 0 && B == 1 && C == 2 && D == 3 && E == 4);
}

namespace B
{
	@Increment(2)
	enum Even
	{
		A, B, C, D, E,
	};
	CONST_ASSERT(A == 0 && B == 2 && C == 4 && D == 6 && E == 8);
}

namespace C
{
	enum = long
	{
		A, B, C, D, E,
	};
	CONST_ASSERT(A == 0L && B == 1L && C == 2L && D == 3L && E == 4L);
}

namespace D
{
	@Increment(0.5)
	enum
	{
		A, B, C, D, E,
	};
	CONST_ASSERT(A == 0.0 && B == 0.5 && C == 1.0 && D == 1.5 && E == 2.0);
}

global script Global
{
    void run()
    {
        printf("%aad\n", {{A::A,A::B,A::C,A::D,A::E,},{B::A,B::B,B::C,B::D,B::E,},{C::A,C::B,C::C,C::D,C::E,},{D::A,D::B,D::C,D::D,D::E,},});
    }
}

enum SomeBitflags
{
	A, B, C, D, E
};

void test()
{
	AnimationBitflags f1 = AF_FRESH | AF_CYCLE;
	f1 = AF_FRESH + AF_CYCLE;
	f1 = AF_FRESH - AF_CYCLE;
	f1 += AF_FRESH;
	f1 = AF_FRESH | ~AF_CYCLE;
	f1 = ~f1|f1&AF_FRESH;
	// Error.
	f1 = HIT_BY_NPC_ID;
	// Error.
	f1 = AF_FRESH | HIT_BY_NPC_ID;
	// Error.
	f1 = AF_FRESH + HIT_BY_NPC_ID;
	// Error.
	f1 += HIT_BY_NPC_ID;

	auto f2 = AF_CYCLE|AF_CYCLE;
	f2 |= AF_FRESH;
	// Error.
	f2 = HIT_BY_NPC_ID;

	AnimationBitflags f3 = AF_CYCLE | <AnimationBitflags>AF_FRESH;
	f3 = AF_CYCLE | <AnimationBitflags>(1);
	// Error.
	f3 = AF_CYCLE | <BlockBitflags>(1);

	auto x = A::A | A::B;
	// Error: Cannot cast from bitmap to int. Just showing that above resolved to int, not the enum.
	x = new bitmap();

	bitmap b;
	// Error.
	b | b;
}

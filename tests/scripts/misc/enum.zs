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

namespace E
{
	@Bitflags("int")
	enum SomeBitflags
	{
		A, B, C, D, E
	};
}

global script Global
{
    void run()
    {
        printf("%aad\n", {{A::A,A::B,A::C,A::D,A::E,},{B::A,B::B,B::C,B::D,B::E,},{C::A,C::B,C::C,C::D,C::E,},{D::A,D::B,D::C,D::D,D::E,},{E::A,E::B,E::C,E::D,E::E,},});
    }
}

@Bitflags("int")
enum AnimationBitflags
{
	AF_FRESH,
	AF_CYCLE,
	AF_CYCLENOCSET,
	AF_TRANSPARENT,
	AF_CYCLEUNDERCOMBO,
};

enum HitIndex
{
	HIT_BY_NPC_ID,
};

void test()
{
	AnimationBitflags f1 = AF_FRESH | AF_CYCLE;
	f1 = AF_FRESH & AF_CYCLE;
	f1 = AF_FRESH ^ AF_CYCLE;
	f1 = AF_FRESH + AF_CYCLE;
	f1 = AF_FRESH - AF_CYCLE;
	f1 += AF_FRESH;
	f1 = AF_FRESH | ~AF_CYCLE;
	f1 = ~f1|f1&AF_FRESH;
	// Error.
	f1 = HIT_BY_NPC_ID;
	// Error.
	f1 = AF_FRESH * AF_CYCLE;
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
	f3 = AF_CYCLE | <E::SomeBitflags>(1);

	auto x = HIT_BY_NPC_ID | HIT_BY_NPC_ID;
	// Error: Cannot cast from bitmap to int. Just showing that above resolved to int, not the enum.
	x = new bitmap();

	bitmap b;
	// Error.
	b | b;
}

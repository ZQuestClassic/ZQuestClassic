@Bitflags("int")
enum BadlyDefinedBitflags
{
	F0 = 0,
	F1, // Still starts at 1.
	F2 = 0b100,
	F3,
	F4 = 0b110,
	F5 = 0b001,
	F6, // 0b010
	F7, // 0b100
	F8 = F1|F2,
	// Error.
	F9,
	F10,
};

@Bitflags("long")
enum BadlyDefinedBitflagsL
{
	LF0 = 0L,
	LF1, // Still starts at 1.
	LF2 = 0b100L,
	LF3,
	LF4 = 0b110L,
	LF5 = 0b001L,
	LF6, // 0b010L
	LF7, // 0b100L
	LF8 = LF1|LF2,
	// Error.
	LF9,
	LF10,
};

// Exercises @ExportDropdown / @ExportBitflags / @ExportLongBitflags /
// @ExportEnum / @ExportEngineValue metadata. Verified by parser_test.cpp.

enum Fruit
{
	FRUIT_APPLE,
	FRUIT_BANANA,
	FRUIT_GRAPE = 6,
	FRUIT_DUPE = 6, // duplicate value: the first declared name should win
};

@Bitflags("int")
enum MoveFlags
{
	MOVE_FLAG_UP    = 0x01,
	MOVE_FLAG_DOWN  = 0x02,
	MOVE_FLAG_ALL   = 0x03, // multi-bit: not displayable, skipped
	MOVE_FLAG_LEFT  = 0x04,
};

enum Lonely { LONELY_ONLY = 5 }; // the common prefix is the whole name

// The prefix is derived from the first two names; later names may not share
// it and must be left untouched (a shorter name used to crash the compiler).
enum Mixed
{
	VEG_POTATO,
	VEG_TOMATO,
	Q,
	ANIMAL_DOG,
};

generic script exports_test
{
	// Declared out of alphabetical order on purpose: instance ids must
	// follow declaration order, not name order.
	@Export("Zed"),
	@ExportDropdown("Apple", "Banana", 6, "Grape")
	nonstatic int z_dropdown = 1;

	@Export("Flags"),
	@ExportBitflags("Up", "Down", 16, "Big")
	nonstatic int a_flags = 3;

	@Export("Long Flags"),
	@ExportLongBitflags("First", "Second")
	nonstatic long m_longflags = 1L;

	@Export("Fruit"),
	@ExportEnum(Fruit)
	nonstatic int fruit = 0;

	@Export("Fruit Raw"),
	@ExportEnum(Fruit, EXPDISP_RAW)
	nonstatic int fruit_raw = 0;

	@Export("Move"),
	@ExportEnum(MoveFlags)
	nonstatic int move = 0;

	@Export("Lonely"),
	@ExportEnum(Lonely)
	nonstatic int lonely = 5;

	@Export("Tile"),
	@ExportEngineValue("Tile")
	nonstatic int tile = 0;

	@Export("Tile CSet"),
	@ExportEngineValue("Tile CSet")
	nonstatic int tile_cset = 0;

	@Export("Mixed"),
	@ExportEnum(Mixed)
	nonstatic int mixed = 0;

	void run()
	{
		// Keep the variables alive.
		Trace(z_dropdown + a_flags + fruit + fruit_raw + move + lonely + tile + tile_cset + mixed);
		Trace(m_longflags);
	}
}

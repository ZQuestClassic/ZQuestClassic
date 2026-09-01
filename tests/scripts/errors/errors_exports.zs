// Error handling for the custom export annotations.

#option NO_ERROR_HALT on

enum PlainEnum { PE_A, PE_B };

ffc script BadExports
{
	// Requires @Export
	@ExportDropdown("A")
	nonstatic int no_export;

	// Conflicts with @ExportRange
	@Export("Conflict"),
	@ExportRange(0, 10),
	@ExportDropdown("A", "B")
	nonstatic int conflict;

	// Two custom export types are mutually exclusive
	@Export("Two Customs"),
	@ExportDropdown("A"),
	@ExportBitflags("B")
	nonstatic int two_customs;

	// Zero params
	@Export("Empty Dropdown"),
	@ExportDropdown()
	nonstatic int empty_dd;

	// Two numbers in a row
	@Export("Two Numbers"),
	@ExportDropdown(1, 2, "A")
	nonstatic int two_nums;

	// Last param must be a string
	@Export("Trailing Number"),
	@ExportDropdown("A", 5)
	nonstatic int trailing_num;

	// Duplicate value
	@Export("Duplicate Value"),
	@ExportDropdown(0, "A", 0, "B")
	nonstatic int dupe_val;

	// Non-power-of-2 flag value
	@Export("Bad Flag"),
	@ExportBitflags(5, "A")
	nonstatic int bad_flag;

	// Non-power-of-2 decimal flag value
	@Export("Decimal Flag"),
	@ExportBitflags(2.5, "A")
	nonstatic int decimal_flag;

	// Custom exports are incompatible with bool
	@Export("Bool Flags"),
	@ExportBitflags("A")
	nonstatic bool bool_flags;

	// More names than available bits (18)
	@Export("Too Many Flags"),
	@ExportBitflags("F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
		"F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18")
	nonstatic int too_many;

	// Unknown engine value name
	@Export("Bogus Engine"),
	@ExportEngineValue("Bogus")
	nonstatic int bogus_engine;

	// Engine value conflicts with an explicit @Export button type
	@Export("Btn Conflict", "some help text", "H"),
	@ExportEngineValue("Tile")
	nonstatic int btn_conflict;

	// Engine value is incompatible with bool
	@Export("Bool Engine"),
	@ExportEngineValue("Color")
	nonstatic bool bool_engine;

	// Display mode contains unknown bits
	@Export("Bad Display"),
	@ExportEnum(PlainEnum, 0x40L)
	nonstatic int bad_display;

	// Zero params
	@Export("Empty Enum"),
	@ExportEnum()
	nonstatic int empty_enum;

	void run() {}
}

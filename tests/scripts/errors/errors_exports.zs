// Error handling for the custom export annotations.

#option NO_ERROR_HALT on

enum PlainEnum { PE_A, PE_B };

@Bitflags("long")
enum LongFlags { LF_A, LF_B };

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

	// Only the bool error, not a second one about the value convention
	@Export("Bool Long Flags"),
	@ExportLongBitflags("A")
	nonstatic bool bool_long_flags;

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

	// Engine value is incompatible with long (stored unscaled, but the
	// editor treats engine values as int)
	@Export("Long Engine"),
	@ExportEngineValue("Tile")
	nonstatic long long_engine;

	// 'int' bitflags on a 'long' variable (values differ by 10000x)
	@Export("Int Flags Long Var"),
	@ExportBitflags("Up", "Down")
	nonstatic long int_flags_long_var;

	// 'long' bitflags on an 'int' variable
	@Export("Long Flags Int Var"),
	@ExportLongBitflags("First", "Second")
	nonstatic int long_flags_int_var;

	// 'int' dropdown values on a 'long' variable
	@Export("Int Dropdown Long Var"),
	@ExportDropdown(0, "Zero", 1, "One")
	nonstatic long int_dd_long_var;

	// A dropdown with no values on a 'long' variable
	@Export("No Values Long Var"),
	@ExportDropdown("Zero", "One")
	nonstatic long no_vals_long_var;

	// A 'long' bitflags enum on an 'int' variable
	@Export("Long Enum Int Var"),
	@ExportEnum(LongFlags)
	nonstatic int long_enum_int_var;

	// An 'int' enum on a 'long' variable
	@Export("Int Enum Long Var"),
	@ExportEnum(PlainEnum)
	nonstatic long int_enum_long_var;

	// 'int' range bounds on a 'long' variable
	@Export("Int Range Long Var"),
	@ExportRange(0, 100)
	nonstatic long int_range_long_var;

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

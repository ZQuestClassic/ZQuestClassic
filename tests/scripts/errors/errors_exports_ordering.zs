// Export errors reported after the Registration pass. These need a file with
// no Registration-pass errors, since any error aborts before the next pass.

const int NOT_AN_ENUM = 5;

ffc script BadExportOrdering
{
	// Param must name an enum type. Registration skips unresolved names
	// (the enum may not be registered yet), so this errors in Analysis.
	@Export("Not An Enum"),
	@ExportEnum(NOT_AN_ENUM)
	nonstatic int not_an_enum;

	// 'Tile CSet' / 'Combo CSet' engine exports must directly follow a
	// variable exported with the matching 'Tile' / 'Combo' engine export.
	@Export("Orphan CSet"),
	@ExportEngineValue("Tile CSet")
	nonstatic int orphan_cset;

	@Export("Some Combo"),
	@ExportEngineValue("Combo")
	nonstatic int some_combo;

	// Not directly after the 'Combo' export (a plain export sits between).
	@Export("Gap")
	nonstatic int gap;

	@Export("Gapped CSet"),
	@ExportEngineValue("Combo CSet")
	nonstatic int gapped_cset;

	void run() {}
}

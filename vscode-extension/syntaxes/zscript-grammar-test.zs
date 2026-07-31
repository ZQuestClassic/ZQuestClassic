// Grammar test for zscript.tmLanguage.json.
//
// Each tested line is preceded by a marker: `//! valid`, `//! invalid`, or
// `//! invalid-single` (flags as exactly ONE token, not a valid prefix plus
// stranded junk). Check with:
//
//     npm run check-zscript-grammar
//
// "invalid" is an expected result, not a failure -- the grammar deliberately
// flags things that aren't real annotations or options.

// ---------------------------------------------------------------------------
// Annotations: known names must match as whole words. A known annotation that
// is a prefix of a longer name (@Export / @ExportRange, @Flag1 / @Flag12)
// must never win partially and strand the rest of the name.
// ---------------------------------------------------------------------------

//! valid
@Author
//! valid
@InitScript
//! valid
@AlwaysRunEndpoint

// Export is a prefix of ExportRange; both must match whole.
//! valid
@Export
//! valid
@ExportRange
// Not a real annotation: must flag as ONE token, not `@Export` + `[Foo]`.
//! invalid-single
@ExportFoo

// Flags run 0-15, so single-digit names are prefixes of two-digit ones.
//! valid
@Flag0
//! valid
@Flag9
//! valid
@Flag10
//! valid
@Flag12
//! valid
@Flag15
// 16 is out of range: one invalid token, not `@Flag1` + `[6]`.
//! invalid-single
@Flag16
//! valid
@FlagHelp0
//! valid
@FlagHelp15

// The remaining annotation families.
//! valid
@Attribute0
//! valid
@Attribute9
// Attribute (unlike Attribyte/Attrishort) accepts two digits, up to
// NUM_ZMETA_ATTRIBUTES (24) exclusive.
//! valid
@Attribute10
//! valid
@Attribute23
//! invalid-single
@Attribute24
//! valid
@AttributeHelp3
//! valid
@AttributeHelp23
//! valid
@Attribyte0
//! valid
@Attribyte7
//! valid
@AttrishortHelp7
//! valid
@InitD0
//! valid
@InitD7
//! valid
@InitDHelp2
//! valid
@InitDType5
//! valid
@ExportInitD0
//! valid
@ExportInitD7

// Enum annotations.
//! valid
@Increment
//! valid
@Bitflags

// Out-of-range and unknown names still flag.
//! invalid-single
@Attribyte8
//! invalid-single
@ExportInitD8
//! invalid-single
@Bogus

// ---------------------------------------------------------------------------
// Preprocessor: known option names highlight, unknown option names are
// flagged, and the arguments of other directives are left alone.
// ---------------------------------------------------------------------------

//! valid
#option SHORT_CIRCUIT on
//! valid
#option HEADER_GUARD off
//! valid
#option WARN_DEPRECATED warn
//! valid
#option ON_MISSING_RETURN error
//! valid
#option LEGACY_ARRAYS default
//! valid
#option TRUNCATE_DIVISION_BY_LITERAL_BUG inherit
//! valid
#option DEFAULT_STATIC_SCRIPT_MEMBERS off

//! invalid
#option NOT_A_REAL_OPTION on
// A former option the compiler no longer accepts (see CompileOption.xtable);
// it flags like any other unknown name.
//! invalid
#option FORCE_INLINE on

// Other directives must NOT have their argument flagged -- the option rules
// are anchored to the literal `option`.
//! valid
#define SOME_MACRO 5
//! valid
#include "std.zh"
//! valid
#includepath "scripts/"
//! valid
#includeif SOME_MACRO "extra.zh"
//! valid
#ignore error
//! valid
#IGNORE WARNING

// ---------------------------------------------------------------------------
// Identifiers must tokenize as whole words -- no partial matches, nothing
// flagged.
// ---------------------------------------------------------------------------

//! valid
int plain = 1;
//! valid
int _leadingUnderscore = 2;
//! valid
int with_digits_123 = 3;
//! valid
int trailing_ = 4;
// Identifiers that merely CONTAIN keywords must not be split apart.
//! valid
int interior = 5;
//! valid
int iffy = 6;
//! valid
int static_thing = 7;
//! valid
int forloop = 8;
//! valid
int classy = 9;

// ---------------------------------------------------------------------------
// General coverage: literals, operators, and script structure.
// ---------------------------------------------------------------------------

//! valid
const int HEX = 0xFF;
//! valid
const int BIN = 0b1010;
//! valid
const int OCT = 0o17;
//! valid
const int LONG = 42L;
//! valid
const int DECIMAL = 1.2345;
//! valid
char32 c = 'a';
//! valid
bool flags = true && false || !true;
//! valid
int shifted = (1 << 4) | (255 >> 2) & ~0;

/* Block comments should span lines
   without leaking into the next rule. */

//! valid
ffc script GrammarCoverage
{
	//! valid
	static int sharedCount;
	//! valid
	nonstatic int perInstanceFrames;

	//! valid
	void run()
	{
		//! valid
		this->Data = ++perInstanceFrames;
		//! valid
		Screen->DrawInteger(2, 4, 8, 0, -1, -1, sharedCount, 0, 128);
		//! valid
		Trace("a string with \"escapes\" and // not-a-comment");
		//! valid
		while (true) { Waitframe(); }
	}
}

// Deprecated and illegal constructs the grammar flags on purpose.
//! invalid
zstruct Deprecated;
//! invalid
int bad === 1;

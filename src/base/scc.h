#ifndef ZC_SCC_H_
#define ZC_SCC_H_

#include "base/general.h"

#include <optional>
#include <vector>
#include <string_view>

#define MIN_SCC_ARG (-214748)
#define MAX_SCC_ARG 214748

/* Note: Printable ASCII begins at 32 and ends at 126, inclusive. */
#define MSGC_COLOUR                1    // 2 args (cset,swatch)
#define MSGC_SPEED                 2    // 1 arg  (speed)
#define MSGC_GOTOIFSCREEND          3    // 3 args (register, val, newstring)
#define MSGC_GOTOIFRAND            4    // 2 args (factor, newstring)
#define MSGC_GOTOIFITEM                5    // 2 args (itemid, newstring)
#define MSGC_GOTOIFCTR             6    // 3 args (counter, val, newstring)
#define MSGC_GOTOIFCTRPC           7    // 3 args (counter, val, newstring)
#define MSGC_GOTOIFTRI             8    // 2 args (level, newstring)
#define MSGC_GOTOIFTRICOUNT        9    // 2 args (tricount, newstring)
#define MSGC_CTRUP                 10   // 2 args (counter, val)
#define MSGC_CTRDN                 11   // 2 args (counter, val)
#define MSGC_CTRSET                12   // 2 args (counter, val)
#define MSGC_CTRUPPC               13   // 2 args (counter, val)
#define MSGC_CTRDNPC               14   // 2 args (counter, val)
#define MSGC_CTRSETPC              15   // 2 args (counter, val)
#define MSGC_GIVEITEM              16   // 1 arg  (itemid)
#define MSGC_TAKEITEM              17   // 1 arg  (itemid)
#define MSGC_WARP                  18   // 6 args (dmap, screen, x, y, effect, sound
#define MSGC_SETSCREEND            19   // 4 args (dmap, screen, reg, value)
#define MSGC_SFX                   20   // 1 arg  (sfx)
#define MSGC_MIDI                  21   // 1 arg  (midi)
#define MSGC_NAME                  22   // 0 args
#define MSGC_GOTOIFCREEND          23   // 5 args (dmap, screen, reg, val, newstring)
#define MSGC_CHANGEPORTRAIT        24   // 6 args (tile, cset, x, y, twid, thei)
#define MSGC_NEWLINE               25   // 0 args
#define MSGC_SHDCOLOR              26   // 2 args (cset,swatch)
#define MSGC_SHDTYPE               27   // 1 arg  (type)
#define MSGC_DRAWTILE              28   // 5 args (tile, cset, wid, hei, flip)
#define MSGC_ENDSTRING             29   // 0 args
#define MSGC_WAIT_ADVANCE          30   // 0 args
//32-126 are ascii chars, so cannot use 31 for SCC code (since 31 + 1 = space).
//127 unused
#define MSGC_SETUPMENU             128  // 5 args (tile, cset, wid, hei, flip)
#define MSGC_MENUCHOICE            129  // 5 args (pos, upos, dpos, lpos, rpos)
#define MSGC_RUNMENU               130  // 0 args
#define MSGC_GOTOMENUCHOICE        131  // 2 args (pos, newstring)
#define MSGC_TRIGSECRETS           132  // 1 arg (perm)
#define MSGC_SETSCREENSTATE        133  // 2 args (state, value)
#define MSGC_SETSCREENSTATER       134  // 4 args (map, screen, state, value)
#define MSGC_FONT                  135  // 1 arg (font)
#define MSGC_RUN_FRZ_GENSCR        136  // 2 args (script num, force_redraw)
#define MSGC_TRIG_CMB_COPYCAT      137  // 1 arg (copycat id)
// Added in 3.0+
#define MSGC_GOTOIFGLOBALSTATE     138  // 3 args (index, value, newstring)
#define MSGC_GOTOIFSCREENSTATE     139  // 3 args (flag, value, newstring)
#define MSGC_GOTOIFANYSCREENSTATE  140  // 5 args (map, screen, flag, value, newstring)
#define MSGC_GOTOIFSCREENEXSTATE   141  // 3 args (state, value, newstring)
#define MSGC_GOTOIFANYSCREENEXSTATE 142 // 5 args (map, screen, state, value, newstring)
#define MSGC_GOTOIFLEVELSTATE      143  // 4 args (level, flag, value, newstring)
#define MSGC_GOTOIFLEVELITEM       144  // 4 args (level, flag, value, newstring)
#define MSGC_GOTOIFSECRETS         145  // 1 arg (newstring)
#define MSGC_SETSCREENEXSTATE      146  // 2 args (state, value)
#define MSGC_SETANYSCREENEXSTATE   147  // 4 args (map, screen, state, value)
#define MSGC_SETGLOBALSTATE        148  // 2 args (index, value)
#define MSGC_SETLEVELSTATE         149  // 3 args (level, flag, value)
#define MSGC_SETLEVELITEM          150  // 3 args (level, flag, value)
#define MSGC_COUNTER               151  // 1 arg (ctr)
#define MSGC_MAXCOUNTER            152  // 1 arg (ctr)
#define MSGC_KILLHERO              153  // 1 arg (bypass_revive)
#define MSGC_COLLECTITEM           154  // 1 arg (itemid)
#define MSGC_DELAY                 155  // 1 arg (frames)
#define MSGC_FORCE_DELAY           156  // 1 arg (frames)
#define MSGC_GOTOIFBOTTLE          157  // 3 args (quantity, fill type, newstring)
#define MSGC_CHANGEBOTTLE          158  // 3 args (quantity, old fill type, new fill type)
#define MSGC_MUSIC                 159  // 1 arg (music)
#define MSGC_MUSIC_REFRESH         160  // 1 arg (music refresh)

#define MAX_SCC_ARG_COUNT 6

struct StringCommand
{
	word start; // starting index in source string
	byte length; // length in source string
	byte code;
	byte num_args;
	int32_t args[MAX_SCC_ARG_COUNT];
};

// `segment_types` is the order of each part of the strings. `literals` and `commands` should be
// traversed in increasing order based on the values in `segment_types`.
//
// Invalid segments are also stored in literals, but serialization must ignore escaping rules. This
// allows bad commands to roundtrip without losing information. Otherwise, a command like `\2\1`
// (which is missing a trailing slash, and thus interpreted as a literal) would incorrectly be saved
// as `\\2\\1`.
struct ParsedMsgStr
{
	// Literals are stored in literals, and commands are stored in commands.
	enum class SegmentType {Invalid, Literal, Command};

	std::vector<std::string_view> literals;
	std::vector<StringCommand> commands;
	std::vector<SegmentType> segment_types;

	std::string serialize() const;
};

std::optional<const char*> get_scc_command_name(int code);
std::optional<int> get_scc_command_num_args(int code);
value_and_warnings<ParsedMsgStr> parse_ascii_msg_str(const std::string& str);
value_and_warnings<ParsedMsgStr> parse_legacy_binary_msg_str(const std::string& str);

#endif

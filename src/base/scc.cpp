#include "base/scc.h"
#include <map>
#include <optional>

static std::map<int, int> legacy_scc_num_args = {
	{MSGC_COLOUR, 2}, // (cset,swatch)
	{MSGC_SPEED, 1}, //  (speed)
	{MSGC_GOTOIFGLOBAL, 3}, // (register, val, newtring)
	{MSGC_GOTOIFRAND, 2}, // (factor, newstring)
	{MSGC_GOTOIF, 2}, // (itemid, newstring)
	{MSGC_GOTOIFCTR, 3}, // (counter, val, newstring)
	{MSGC_GOTOIFCTRPC, 3}, // (counter, val, newstring)
	{MSGC_GOTOIFTRI, 2}, // (level, newstring)
	{MSGC_GOTOIFTRICOUNT, 2}, // (tricount, newstring)
	{MSGC_CTRUP, 2}, // (counter, val)
	{MSGC_CTRDN, 2}, // (counter, val)
	{MSGC_CTRSET, 2}, // (counter, val)
	{MSGC_CTRUPPC, 2}, // (counter, val)
	{MSGC_CTRDNPC, 2}, // (counter, val)
	{MSGC_CTRSETPC, 2}, // (counter, val)
	{MSGC_GIVEITEM, 1}, //  (itemid)
	{MSGC_TAKEITEM, 1}, //  (itemid)
	{MSGC_WARP, 6}, // (dmap, screen, x, y, effect, sound
	{MSGC_SETSCREEND, 4}, // (dmap, screen, reg, value)
	{MSGC_SFX, 1}, //  (sfx)
	{MSGC_MIDI, 1}, //  (midi)
	{MSGC_NAME, 0},
	{MSGC_GOTOIFCREEND, 5}, // (dmap, screen, reg, val, newstring)
	{MSGC_CHANGEPORTRAIT, 0}, // not implemented
	{MSGC_NEWLINE, 0},
	{MSGC_SHDCOLOR, 2}, // (cset,swatch)
	{MSGC_SHDTYPE, 1}, //  (type)
	{MSGC_DRAWTILE, 5}, // (tile, cset, wid, hei, flip)
	{MSGC_ENDSTRING, 0},
	{MSGC_WAIT_ADVANCE, 0},
	{MSGC_SETUPMENU, 5}, // (tile, cset, wid, hei, flip)
	{MSGC_MENUCHOICE, 5}, // (pos, upos, dpos, lpos, rpos)
	{MSGC_RUNMENU, 0},
	{MSGC_GOTOMENUCHOICE, 2}, // (pos, newstring)
	{MSGC_TRIGSECRETS, 1}, // (perm)
	{MSGC_SETSCREENSTATE, 2}, // (ind, state)
	{MSGC_SETSCREENSTATER, 4}, // (map, screen, ind, state)
	{MSGC_FONT, 1}, // (font)
	{MSGC_RUN_FRZ_GENSCR, 2}, // (script num, force_redraw)
	{MSGC_TRIG_CMB_COPYCAT, 1}, // (copycat id)
};

// Increments i and returns the control code argument pointed at.
static std::optional<word> parse_legacy_binary_argument(const std::string& str, size_t& i)
{
	if (i + 1 >= str.size()) return std::nullopt;

	byte val = str[++i] - 1;
	word ret = val;

	// If an argument is succeeded by 255, then it's a three-byte argument -
	// between 254 and 65535 (or whatever the maximum actually is)
	if (i + 2 < str.size() && uint8_t(str[i + 1]) == 255)
	{
		val = str[i + 2];
		word next = val;
		ret += 254 * next;
		i += 2;
	}

	return ret;
}

std::optional<int> get_scc_command_num_args(int code)
{
	auto it = legacy_scc_num_args.find(code);
	if (it == legacy_scc_num_args.end())
		return std::nullopt;
	return it->second;
}

// Parses a "legacy" encoding of a msg str mixed with plain ascii and binary-encoded string commands.
// A msg str is a mix of string literals and commands. The string literals are limited to the 32-126
// ascii values.
// Each string command is an encoded code of 1-30 or 127+, followed by a fixed number of
// arguments.
// The command code is written as +1 the actual value of the code.
// The number of arguments is determined by legacy_scc_num_args.
// Each argument is 1 or 3 bytes long. If the first byte is followed by a 255 byte, then it is a
// 3-byte encoded value. See parse_legacy_binary_argument.
value_and_warnings<ParsedMsgStr> parse_legacy_msg_str(const std::string& str)
{
	value_and_warnings<ParsedMsgStr> result;
	auto& [parsed_msg_str, warnings] = result;

	size_t prev = 0;
	size_t cur = 0;

	// The index after the last non-whitespace character.
	size_t last = 0;
	if (!str.empty())
	{
		for (int i = str.size() - 1; i >= 0; i--)
		{
			if (str[i] != ' ')
			{
				last = i + 1;
				break;
			}
		}
	}

	while (cur < last)
	{
		byte c = str[cur];
		if (c >= 32 && c <= 126)
		{
			cur++;
			continue;
		}

		auto expected_args_opt = get_scc_command_num_args(c - 1);
		if (!expected_args_opt)
		{
			warnings.push_back(fmt::format("Unknown command {}", (int)c - 1));
			cur++;
			continue;
		}

		if (prev != cur)
		{
			parsed_msg_str.literals.emplace_back(str.begin() + prev, str.begin() + cur);
			parsed_msg_str.segment_types.push_back(0);
		}
		
		auto& command = parsed_msg_str.commands.emplace_back();
		command.code = c - 1;
		parsed_msg_str.segment_types.push_back(1);

		int expected_args = *expected_args_opt;
		int found_num_args;
		for (found_num_args = 0; found_num_args < expected_args; found_num_args++)
		{
			if (auto val = parse_legacy_binary_argument(str, cur))
				command.args[found_num_args] = *val;
			else break;
		}
		command.num_args = found_num_args;

		if (found_num_args != expected_args)
			warnings.push_back(fmt::format("For command {}, expected {} args but got {}",
				(int)command.code, expected_args, found_num_args));

		cur++;
		prev = cur;
	}

	if (prev != last)
	{
		parsed_msg_str.literals.emplace_back(str.begin() + prev, str.begin() + last);
		parsed_msg_str.segment_types.push_back(0);
	}

	return result;
}

// Generates a string with human-readable string commands. Each command starts with `\`, is followed
// by the command code, then each successive argument separated by `\`. For example, this is a color
// command: `\26\1\3`.
// This is only used to present a human-readable/editable string in the editor. For saving back to
// the legacy encoding format, see parse_to_legacy_msg_str_encoding.
std::string ParsedMsgStr::serialize() const
{
	std::string result;

	size_t literal_index = 0;
	size_t command_index = 0;
	for (int i = 0; i < segment_types.size(); i++)
	{
		if (segment_types[i] == 0)
		{
			result += literals[literal_index++];
		}
		else
		{
			auto& command = commands[command_index++];
			result += fmt::format("\\{}", command.code);
			for (int j = 0; j < command.num_args; j++)
				result += fmt::format("\\{}", command.args[j]);
		}
	}

	return result;
}

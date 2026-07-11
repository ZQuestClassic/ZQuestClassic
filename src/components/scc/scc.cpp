#include "components/scc/scc.h"
#include "core/flags.h"
#include "base/check.h"
#include "base/util.h"
#include "base/containers.h"
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <map>
#include <optional>

struct StringCommandDefn
{
	const char* name;
	// The accepted argument count range. For fixed-arity commands min_args ==
	// max_args. Commands that accept optional trailing arguments use a range.
	int min_args;
	int max_args;

	StringCommandDefn(const char* name, int num_args)
		: name(name), min_args(num_args), max_args(num_args) {}
	StringCommandDefn(const char* name, int min_args, int max_args)
		: name(name), min_args(min_args), max_args(max_args) {}
};

static std::map<int, StringCommandDefn> scc_commands =
{
	// Formatting.
	{MSGC_COLOUR, {"TextColor", 2}}, // (cset,swatch)
	{MSGC_SHDCOLOR, {"ShadowColor", 2}}, // (cset,swatch)
	{MSGC_SHDTYPE, {"ShadowType", 1}}, //  (type)
	{MSGC_SPEED, {"Speed", 1}}, //  (speed)
	{MSGC_DRAWTILE, {"Tile", 5}}, // (tile, cset, wid, hei, flip)
	{MSGC_NEWLINE, {"Newline", 0}},
	{MSGC_FONT, {"Font", 1}}, // (font)
	{MSGC_NAME, {"Name", 0}},
	{MSGC_CHANGEPORTRAIT, {"Portrait", 6}}, // (tile, cset, x, y, twid, thei)

	// Menu
	{MSGC_SETUPMENU, {"SetupMenu", 5}}, // (tile, cset, wid, hei, flip)
	{MSGC_MENUCHOICE, {"MenuChoice", 5}}, // (pos, upos, dpos, lpos, rpos)
	{MSGC_RUNMENU, {"RunMenu", 0}},
	{MSGC_GOTOMENUCHOICE, {"GoIfMenuChoice", 2}}, // (pos, newstring)

	// Switch
	{MSGC_GOTOIFSCREEND, {"GoIfScreenD", 3}}, // (register, val, newstring)
	{MSGC_GOTOIFRAND, {"GoIfRand", 2}}, // (factor, newstring)
	{MSGC_GOTOIFITEM, {"GoIfItem", 2}}, // (itemid, newstring)
	{MSGC_GOTOIFCTR, {"GoIfCounter", 3}}, // (counter, val, newstring)
	{MSGC_GOTOIFCTRPC, {"GoIfCounter%", 3}}, // (counter, val, newstring)
	{MSGC_GOTOIFTRI, {"GoIfMcGuffin", 2}}, // (level, newstring)
	{MSGC_GOTOIFTRICOUNT, {"GoIfMcGuffinCount", 2}}, // (tricount, newstring)
	{MSGC_GOTOIFCREEND, {"GoIfAnyScreenD", 5}}, // (dmap, screen, reg, val, newstring)
	{MSGC_GOTOIFGLOBALSTATE, {"GoIfGlobalState", 3}}, // (state, value, newstring)
	{MSGC_GOTOIFSCREENSTATE, {"GoIfScreenState", 3}}, // (state, value, newstring)
	{MSGC_GOTOIFANYSCREENSTATE, {"GoIfAnyScreenState", 5}}, // (map, screen, state, value, newstring)
	{MSGC_GOTOIFSCREENEXSTATE, {"GoIfScreenExState", 3}}, // (state, value, newstring)
	{MSGC_GOTOIFANYSCREENEXSTATE, {"GoIfAnyScreenExState", 5}}, // (map, screen, state, value, newstring)
	{MSGC_GOTOIFLEVELSTATE, {"GoIfLevelState", 4}}, // (level, state, value, newstring)
	{MSGC_GOTOIFLEVELITEM, {"GoIfLevelItem", 4}}, // (level, item, value, newstring)
	{MSGC_GOTOIFSECRETS, {"GoIfSecrets", 1}}, // (newstring)
	{MSGC_GOTOIFBOTTLE, {"GoIfBottle", 3}}, // (quantity, fill type, newstring)

	// Control Mod
	{MSGC_CTRUP, {"CounterAdd", 2}}, // (counter, val)
	{MSGC_CTRDN, {"CounterSubtract", 2}}, // (counter, val)
	{MSGC_CTRSET, {"CounterSet", 2}}, // (counter, val)
	{MSGC_CTRUPPC, {"CounterAdd%", 2}}, // (counter, val)
	{MSGC_CTRDNPC, {"CounterSubtract%", 2}}, // (counter, val)
	{MSGC_CTRSETPC, {"CounterSet%", 2}}, // (counter, val)
	{MSGC_GIVEITEM, {"GiveItem", 1}}, //  (itemid)
	{MSGC_COLLECTITEM, {"CollectItem", 1}}, // (itemid)
	{MSGC_TAKEITEM, {"TakeItem", 1}}, //  (itemid)
	{MSGC_CHANGEBOTTLE, {"ChangeBottle", 3}}, // (quantity, old fill type, new fill type)

	// Misc
	{MSGC_WARP, {"Warp", 6}}, // (dmap, screen, x, y, effect, sound
	{MSGC_SETSCREEND, {"SetScreenD", 4}}, // (dmap, screen, reg, value)
	{MSGC_SETCURRENTSCREEND, {"SetCurrentScreenD", 2}}, // (reg, value)
	{MSGC_SFX, {"SFX", 1}}, //  (sfx)
	{MSGC_MIDI, {"MIDI", 1}}, //  (midi)
	{MSGC_MUSIC, {"Music", 1}}, //  (music)
	{MSGC_MUSIC_REFRESH, {"MusicRefresh", 1}}, //  (music)
	{MSGC_WAIT_ADVANCE, {"WaitAdvance", 0}},
	{MSGC_DELAY, {"Delay", 1}},
	{MSGC_FORCE_DELAY, {"ForceDelay", 1}},
	{MSGC_ENDSTRING, {"EndString", 0}},
	{MSGC_TRIGSECRETS, {"TriggerSecrets", 1}}, // (perm)
	{MSGC_TRIG_CMB_COPYCAT, {"TriggerComboCopycat", 1}}, // (copycat id)
	{MSGC_SETSCREENSTATE, {"SetScreenState", 2}}, // (state, value)
	{MSGC_SETSCREENSTATER, {"SetAnyScreenState", 4}}, // (map, screen, state, value)
	{MSGC_SETSCREENEXSTATE, {"SetScreenExState", 2}}, // (state, value)
	{MSGC_SETANYSCREENEXSTATE, {"SetAnyScreenExState", 4}}, // (map, screen, state, value)
	{MSGC_SETGLOBALSTATE, {"SetGlobalState", 2}}, // (state, value)
	{MSGC_SETLEVELSTATE, {"SetLevelState", 3}}, // (level, state, value)
	{MSGC_SETLEVELITEM, {"SetLevelItem", 3}}, // (level, item, value)
	{MSGC_RUN_FRZ_GENSCR, {"RunFrozenGenericScript", 2, 10}}, // (script num, force_redraw, [d0..d7])
	{MSGC_COUNTER, {"Counter", 1}}, // (ctr)
	{MSGC_MAXCOUNTER, {"MaxCounter", 1}}, // (ctr)
	{MSGC_KILLHERO, {"KillHero", 1}}, // (bypass_revive)
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

std::optional<const char*> get_scc_command_name(int code)
{
	auto it = scc_commands.find(code);
	if (it == scc_commands.end())
		return std::nullopt;
	return it->second.name;
}

// Returns the number of required arguments for a command. For commands that
// accept optional trailing arguments, this is the minimum. Callers that only
// need to know "is this a known command" can check has_value().
std::optional<int> get_scc_command_num_args(int code)
{
	auto it = scc_commands.find(code);
	if (it == scc_commands.end())
		return std::nullopt;
	return it->second.min_args;
}

// Returns the maximum number of arguments a command accepts (== the required
// count for fixed-arity commands).
std::optional<int> get_scc_command_max_args(int code)
{
	auto it = scc_commands.find(code);
	if (it == scc_commands.end())
		return std::nullopt;
	return it->second.max_args;
}

namespace flags
{
enum numparse_err_flags : uint8_t
{
	NPR_ERR_OVERFLOW       = 0x01,
	NPR_ERR_UNDERFLOW      = 0x02,
	NPR_ERR_TOO_MANY_DEC   = 0x04, // 1.00001
	NPR_ERR_TOO_FEW_DEC    = 0x08, // 1.
};
}

struct NumberParseResult
{
	zfix value;
	// The index after the last digit of the number.
	size_t next_index;
	bool has_decimal;
	numparse_err_flags errs;
};

/**
* @brief Parses an optional-signed decimal number from a string, with
* robust overflow/underflow checks.
*
* @param str The string to parse.
* @param start The index to start parsing from.
* @return An optional NumberParseResult. Returns std::nullopt if no digits
* are found after the optional sign (e.g., "\-", "\F").
*/
std::optional<NumberParseResult> parse_ascii_number(const std::string& str, size_t start)
{
	size_t i = start;
	if (i >= str.size())
		return std::nullopt; // Reached end of string

	bool is_negative = false;
	if (str[i] == '-')
	{
		is_negative = true;
		i++;
	}

	// Check if there are any digits after the (optional) sign
	if (i >= str.size() || !std::isdigit(static_cast<unsigned char>(str[i])))
		return std::nullopt; // No number found (e.g., "\-", "\F", "\")

	int ipart = 0, dpart = 0;
	int max_int = MAX_SCC_INT;
	int min_int = MIN_SCC_INT;
	numparse_err_flags errs{};
	bool has_decimal = false;

	while (i < str.size() && std::isdigit(static_cast<unsigned char>(str[i])))
	{
		int digit = (str[i] - '0');
		if (!errs)
		{
			if (is_negative)
			{
				// Check for underflow before multiplying
				if (ipart < min_int / 10 || (ipart == min_int / 10 && -digit < min_int % 10))
				{
					errs |= NPR_ERR_UNDERFLOW;
					ipart = min_int;
				}
				else
				{
					ipart = ipart * 10 - digit;
				}
			}
			else
			{
				// Check for overflow before multiplying
				if (ipart > max_int / 10 || (ipart == max_int / 10 && digit > max_int % 10))
				{
					errs |= NPR_ERR_OVERFLOW;
					ipart = max_int;
				}
				else
				{
					ipart = ipart * 10 + digit;
				}
			}
		}

		i++;
	}
	if (i < str.size() && str[i] == '.')
	{
		has_decimal = true;
		++i;
		int i2 = 1000;
		int max_dpart = 9999;
		if (ipart == MAX_SCC_INT)
			max_dpart = 3647;
		else if (ipart == MIN_SCC_INT)
			max_dpart = 3648;
		if (errs & (NPR_ERR_OVERFLOW|NPR_ERR_UNDERFLOW))
			dpart = max_dpart;
		if (i >= str.size() || !std::isdigit(static_cast<unsigned char>(str[i])))
		{
			errs |= NPR_ERR_TOO_FEW_DEC;
		}
		else
		{
			while (i < str.size() && i2 && std::isdigit(static_cast<unsigned char>(str[i])))
			{
				int digit = (str[i] - '0');
				
				if (!errs)
				{
					dpart += i2 * digit;
					if (dpart > max_dpart)
					{
						if (is_negative)
							errs |= NPR_ERR_UNDERFLOW;
						else
							errs |= NPR_ERR_OVERFLOW;
						// Unclamped, e.g. zfix(214748, 3648) overflows int32.
						dpart = max_dpart;
					}
				}
				
				++i;
				i2 /= 10;
			}
			if (i < str.size() && !i2 && std::isdigit(static_cast<unsigned char>(str[i])))
			{
				errs |= NPR_ERR_TOO_MANY_DEC;
				// step past all the excess digits
				while (i < str.size() && std::isdigit(static_cast<unsigned char>(str[i])))
					++i;
			}
		}
	}
	return {{zfix(ipart, is_negative ? -dpart : dpart), i, has_decimal, errs}};
}

static bool compareStringsCaseIns(const std::string& str1, const std::string& str2)
{
	if (str1.length() != str2.length())
		return false;

	for (int i = 0; i < str1.length(); ++i)
	{
		if (tolower(str1[i]) != tolower(str2[i]))
			return false;
	}

	return true;
}

static void consume_until_slash(const std::string& str, size_t& cur)
{
	while (cur < str.size() && str[cur] != '\\') cur++;
}

static const std::optional<int> lookup_string_command(const std::string& str, size_t& cur, std::vector<std::string>& warnings)
{
	size_t start = cur;
	consume_until_slash(str, cur);
	std::string name = std::string(str.substr(start, cur - start));

	for (auto& [code, defn] : scc_commands)
	{
		if (compareStringsCaseIns(defn.name, name))
			return code;
	}

	return std::nullopt;
}

static bool parse_ascii_scc_command(
	const std::string& str,
	size_t& cur,
	StringCommand& cmd,
	std::vector<std::string>& warnings)
{
	DCHECK(str[cur] == '\\');

	size_t start_index = cur;
	cur += 1; // Skip initiator '\'
	const size_t last = str.size();

	// Parse command code
	std::optional<int> code;
	// The source text of the command code, if it erroneously contained a decimal point.
	std::optional<std::string> decimal_command;

	if (isalpha(str[cur]))
	{
		if (auto r = lookup_string_command(str, cur, warnings))
			code = *r;
	}
	else if (auto num_res_opt = parse_ascii_number(str, cur))
	{
		auto num_res = *num_res_opt;

		// over/underflow are ignored, handled by 'unknown command'
		// decimal too few/many errors are ignored, as decimal existing at all
		//  is considered an error here:
		if (num_res.has_decimal)
			decimal_command = str.substr(cur, num_res.next_index - cur);
		cur = num_res.next_index;
		code = num_res.value.getTrunc();
	}

	if (!code)
		consume_until_slash(str, cur);

	cmd.code = code.value_or(0);
	auto defn_it = scc_commands.find(cmd.code);
	bool is_unknown_command = defn_it == scc_commands.end();
	int min_args = is_unknown_command ? 0 : defn_it->second.min_args;
	int max_args = is_unknown_command ? 0 : defn_it->second.max_args;
	int found_num_args = 0;
	
	bitstring can_decimal;
	switch (cmd.code)
	{
		case MSGC_GOTOIFSCREEND:
			can_decimal.set(1, true);
			break;
		case MSGC_SETSCREEND:
			can_decimal.set(3, true);
			break;
		case MSGC_SETCURRENTSCREEND:
			can_decimal.set(1, true);
			break;
		case MSGC_GOTOIFCREEND:
			can_decimal.set(3, true);
			break;
		case MSGC_RUN_FRZ_GENSCR:
			for (int idx = 2; idx < 10; ++idx)
				can_decimal.set(idx, true);
			break;
	}

	// Parse arguments.
	numparse_err_flags arg_errors{};
	bool found_non_numeric_argument = false;
	bool found_double_slash = false;
	std::set<int> invalid_decimal_args;
	while (cur < str.size())
	{
		if (cur >= last || str[cur] != '\\')
			break;
		if (str[cur + 1] == ' ' || cur + 1 == str.size())
			break;

		cur++; // Skip arg '\'

		// Terminator.
		if (str[cur] == ' ')
			break;
		if (str[cur] == '\\')
		{
			found_double_slash = true;
			cur--;
			break;
		}

		zfix value = 0;

		auto arg_res = parse_ascii_number(str, cur);
		if (!arg_res)
		{
			found_non_numeric_argument = true;
			consume_until_slash(str, cur);
		}
		else if (arg_res->next_index < str.size() && str[arg_res->next_index] == '.')
		{
			// A parsed number immediately followed by another '.' (e.g. "1.00.2") is one
			// malformed number, not a valid arg plus a bad terminator.
			found_non_numeric_argument = true;
			cur = arg_res->next_index;
			consume_until_slash(str, cur);
		}
		else
		{
			if (arg_res->has_decimal && !can_decimal.get(found_num_args))
			{
				invalid_decimal_args.insert(found_num_args);
				arg_res->value.doTrunc();
				// let the invalid decimal error swallow these errors
				arg_res->errs &= ~(NPR_ERR_TOO_FEW_DEC|NPR_ERR_TOO_MANY_DEC);
			}

			arg_errors |= arg_res->errs;
			value = arg_res->value;
			cur = arg_res->next_index;
		}

		if (found_num_args < MAX_SCC_ARG_COUNT)
			cmd.args[found_num_args] = value;
		found_num_args++;
	}

	cmd.num_args = std::min(found_num_args, MAX_SCC_ARG_COUNT);

	// Check terminator: must end in one of:
	//   `\ `
	//   `\<EOS>` (end of string)
	bool missing_slash = cur == last || str[cur] != '\\';
	if (!missing_slash) cur++; // Skip terminator '\'

	bool wrong_arg_count = found_num_args < min_args || found_num_args > max_args;
	bool is_valid_command = cmd.code != 0 && !missing_slash && !is_unknown_command && !wrong_arg_count && !arg_errors && !found_non_numeric_argument && !found_double_slash && invalid_decimal_args.empty() && !decimal_command;
	
	auto command_str = str.substr(start_index, cur - start_index);
	if (decimal_command)
		warnings.push_back(fmt::format("Expected integer command, found {} instead", *decimal_command));
	else if (is_unknown_command)
		warnings.push_back(fmt::format("Ignoring unknown command: {}", command_str));
	else if (missing_slash || found_double_slash)
		warnings.push_back(fmt::format("Expected slash (followed by either space or end of string) at end of command: {}", command_str));
	else
	{
		if (wrong_arg_count)
		{
			if (min_args == max_args)
				warnings.push_back(fmt::format("Expected {} args, but got {} for command: {}", min_args, found_num_args, command_str));
			else
				warnings.push_back(fmt::format("Expected between {} and {} args, but got {} for command: {}", min_args, max_args, found_num_args, command_str));
		}
		
		if (found_non_numeric_argument)
			warnings.push_back(fmt::format("Found non-numeric argument for command: {}", command_str));
		
		if (!invalid_decimal_args.empty())
		{
			if (invalid_decimal_args.size() == 1)
				warnings.push_back(fmt::format("Found argument with decimal places that does not allow decimal places: {} (argument #{})", command_str, *invalid_decimal_args.begin()));
			else
				warnings.push_back(fmt::format("Found arguments with decimal places that do not allow decimal places: {} (arguments #{})", command_str, fmt::join(invalid_decimal_args, ", #")));
		}
		if (arg_errors & NPR_ERR_OVERFLOW)
			warnings.push_back(fmt::format("Found argument that was too big for command: {} (max value is {})", command_str, MAX_SCC_ARG.str()));
		if (arg_errors & NPR_ERR_UNDERFLOW)
			warnings.push_back(fmt::format("Found argument that was too small for command: {} (min value is {})", command_str, MIN_SCC_ARG.str()));
		if (arg_errors & NPR_ERR_TOO_FEW_DEC)
			warnings.push_back(fmt::format("Found argument with hanging decimal place: {} (at least 1 digit after the decimal is required)", command_str));
		if (arg_errors & NPR_ERR_TOO_MANY_DEC)
			warnings.push_back(fmt::format("Found argument with too many decimal places for command: {} (max is 4 decimal places)", command_str));
	}

	// At end of string. This is a valid terminator.
	if (cur == last)
		return is_valid_command;

	// Followed by a space. This is a valid terminator.
	if (str[cur] == ' ')
	{
		cur++;
	}
	else
	{
		// Only produce this warning if everything else checks out.
		if (is_valid_command)
			warnings.push_back(fmt::format("Expected slash (followed by either space or end of string) at end of command: {}", str.substr(start_index, cur - start_index)));
		is_valid_command = false;
	}

	return is_valid_command;
}

// Parses an plain ascii-encoded msg str (with negative nums and escapes).
//
// - Unlike parse_legacy_binary_msg_str, this supports negative numbers and command text names (\Speed
//   is the same as \2)
// - Msg strs is encoded as ascii, mixing literals and commands
// - Commands are written as the command code followed by a fixed number of arguments: "\3\1\2\"
// - Commands must end with a backslash, then either a space or the end of the string. The space is
//   always ignored. This allows for an unambigous and simpler to read grammar
// - A literal backslash must be escaped: '\\' for '\'
//
// Example: "FIGHT \\ \10\100\-23\"
//
// - "FIGHT \ " is a literal (the \\ becomes \).
// - \10 is the start of a command code (10), with two args (100, -23).
value_and_warnings<ParsedMsgStr> parse_ascii_msg_str(const std::string& str)
{
	value_and_warnings<ParsedMsgStr> result;
	auto& [parsed_msg_str, warnings] = result;

	size_t cur = 0;
	const size_t last = str.size();
	
	// Tracks the starting index of the current literal segment.
	size_t literal_start_index = 0;
	bool is_parsing_invalid_segment = false;
	bool found_unescaped_braces = false;

	// Helper to push a literal segment as a string_view
	auto push_literal = [&](size_t literal_end_index) {
		if (literal_end_index > literal_start_index)
		{
			parsed_msg_str.literals.emplace_back(
				str.data() + literal_start_index,
				literal_end_index - literal_start_index
			);
			parsed_msg_str.segment_types.push_back(
				is_parsing_invalid_segment ? ParsedMsgStr::SegmentType::Invalid : ParsedMsgStr::SegmentType::Literal);
			is_parsing_invalid_segment = false;
		}

		// Mark that we are no longer in a literal segment.
		literal_start_index = std::string_view::npos; 
	};

	while (cur < last)
	{
		// If we're not in a literal segment, start one at the current index.
		if (literal_start_index == std::string_view::npos)
			literal_start_index = cur;

		char c = str[cur];

		if (c == '\n')
		{
			push_literal(cur++);

			auto& cmd = parsed_msg_str.commands.emplace_back();
			cmd.code = MSGC_NEWLINE;
			parsed_msg_str.segment_types.push_back(ParsedMsgStr::SegmentType::Command);

			continue;
		}

		// Will eventually support expression inside curly braces. For now, just warn.
		// https://discord.com/channels/876899628556091432/1277878877057978460/1360390301713170783
		if (!found_unescaped_braces && (c == '{' || c == '}'))
		{
			found_unescaped_braces = true;
			warnings.push_back("Error: { and } characters must be escaped: \\{ and \\}");
		}

		if (c == '{')
		{
			push_literal(cur);
			literal_start_index = cur;

			// Consume until next unescaped }
			cur++;
			while (cur < str.size())
			{
				if (str[cur] == '}' && str[cur - 1] != '\\')
				{
					cur++;
					break;
				}

				cur++;
			}

			is_parsing_invalid_segment = true;
			push_literal(cur++);
			continue;
		}

		if (c != '\\')
		{
			// Regular character, just advance
			cur++;
			continue;
		}

		// We found a '\'. Check for an escaped character.
		if (cur + 1 < last)
		{
			// The literals are stored as string_view, so we need to split escaped characters across
			// multiple literal segments.
			char next = str[cur + 1];
			if (next == '{' || next == '}')
			{
				push_literal(cur);
				// skip the escaping slash, and put the start of the next
				// literal at the escaped character.
				literal_start_index = cur + 1;
				cur += 2;
				continue;
			}
			else if (next == '\\')
			{
				push_literal(cur + 1); // Push "literal\"
				cur += 2; // Skip both the slashes.
				continue;
			}
		}

		if (cur + 1 == last)
		{
			push_literal(cur);

			warnings.push_back("Unexpected trailing slash (expected either a command or another slash)");
			is_parsing_invalid_segment = true;
			literal_start_index = cur;
			cur++;
			push_literal(cur);
			break;
		}

		size_t command_start_index = cur;
		StringCommand cmd;
		if (parse_ascii_scc_command(str, cur, cmd, warnings))
		{
			push_literal(command_start_index);
			cmd.start = command_start_index;
			cmd.length = cur - command_start_index;
			parsed_msg_str.commands.emplace_back(std::move(cmd));
			parsed_msg_str.segment_types.push_back(ParsedMsgStr::SegmentType::Command);
		}
		else
		{
			is_parsing_invalid_segment = true;
			push_literal(cur);
		}
	}

	// Add any trailing literal
	push_literal(last);

	return result;
}

// Parses a "legacy" encoding of a msg str mixed with plain ascii and binary-encoded string commands.
// A msg str is a mix of string literals and commands. The string literals are limited to the 32-126
// ascii values.
// Each string command is an encoded code of 1-30 or 127+, followed by a fixed number of
// arguments.
// The command code is written as +1 the actual value of the code.
// The number of arguments is determined by scc_commands.
// Each argument is 1 or 3 bytes long. If the first byte is followed by a 255 byte, then it is a
// 3-byte encoded value. See parse_legacy_binary_argument.
value_and_warnings<ParsedMsgStr> parse_legacy_binary_msg_str(const std::string& str)
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

		if (prev != cur)
		{
			parsed_msg_str.literals.emplace_back(str.begin() + prev, str.begin() + cur);
			parsed_msg_str.segment_types.push_back(ParsedMsgStr::SegmentType::Literal);
			prev = cur;
		}

		// The legacy binary format is not self-delimiting, so it can only encode
		// a fixed number of args. Commands that accept optional trailing args are
		// only ever authored in the new ascii format, so reading the required
		// (minimum) count here is correct for any command an old quest can contain.
		auto expected_args_opt = get_scc_command_num_args(c - 1);
		if (!expected_args_opt)
		{
			warnings.push_back(fmt::format("Unknown command {}", (int)c - 1));

			cur++;
			parsed_msg_str.literals.emplace_back(str.begin() + prev, str.begin() + cur);
			parsed_msg_str.segment_types.push_back(ParsedMsgStr::SegmentType::Invalid);
			prev = cur;
			continue;
		}

		auto& command = parsed_msg_str.commands.emplace_back();
		command.start = cur;
		command.code = c - 1;
		parsed_msg_str.segment_types.push_back(ParsedMsgStr::SegmentType::Command);

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
		command.length = cur - command.start;
	}

	// prev can be larger than last here, since the calculation for last unfortunately may also
	// include space (ascii 32) characters being used as the last arg of the last command.
	// An example of that is message 48 of The Slipstream:
	// .tmp/replay_uploads/50C00A5197148F3D1587A16509DCC89F/50C00A5197148F3D1587A16509DCC89F.qst
	if (prev < last)
	{
		parsed_msg_str.literals.emplace_back(str.begin() + prev, str.begin() + last);
		parsed_msg_str.segment_types.push_back(ParsedMsgStr::SegmentType::Literal);
	}

	return result;
}

// Generates a string that should roundtrip back into a ParsedMsgStr via parse_ascii_msg_str.
std::string ParsedMsgStr::serialize() const
{
	std::string result;

	size_t literal_index = 0;
	size_t command_index = 0;
	for (int i = 0; i < segment_types.size(); i++)
	{
		if (segment_types[i] == ParsedMsgStr::SegmentType::Command)
		{
			auto& command = commands[command_index++];
			result += fmt::format("\\{}", get_scc_command_name(command.code).value_or(""));
			for (int j = 0; j < command.num_args; j++)
				result += fmt::format("\\{}", command.args[j].str_trim());
			if (i == segment_types.size() - 1)
				result += "\\";
			else
				result += "\\ ";
		}
		else
		{
			std::string s = std::string(literals[literal_index++]);
			if (segment_types[i] == ParsedMsgStr::SegmentType::Literal)
			{
				util::replstr(s, R"(\)", R"(\\)");
				util::replstr(s, R"({)", R"(\{)");
				util::replstr(s, R"(})", R"(\})");
			}
			result += s;
		}
	}

	return result;
}

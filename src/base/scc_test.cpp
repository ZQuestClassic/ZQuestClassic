#include "base/scc.h"

#include <iomanip>
#include <iostream>

struct ExpectedCommand
{
	int code;
	int num_args;
	std::vector<int> args;
};

struct AsciiParserTestCase
{
	std::string description;
	std::string input;

	std::vector<ParsedMsgStr::SegmentType> expected_segment_types;
	std::vector<std::string_view> expected_literals;
	std::vector<ExpectedCommand> expected_commands;
	std::vector<std::string> expected_warnings;
};

std::vector<AsciiParserTestCase> get_ascii_parser_test_cases()
{
	using TC = AsciiParserTestCase;
	using Cmd = ExpectedCommand;

	auto Invalid = ParsedMsgStr::SegmentType::Invalid;
	auto Literal = ParsedMsgStr::SegmentType::Literal;
	auto Command = ParsedMsgStr::SegmentType::Command;

	return {
		TC{
			"Empty string",
			"",
			/*segments*/ {},
			/*literals*/ {},
			/*commands*/ {},
			/*warnings*/ {}
		},
		TC{
			"Literal only",
			"Hello, world! ^.^",
			/*segments*/ {Literal},
			/*literals*/ {"Hello, world! ^.^"},
			/*commands*/ {},
			/*warnings*/ {}
		},
		TC{
			"Literal only with whitespace",
			"    Hello,     world! ^.^     ",
			/*segments*/ {Literal},
			/*literals*/ {R"(    Hello,     world! ^.^     )"},
			/*commands*/ {},
			/*warnings*/ {}
		},
		TC{
			"Simple command with 6 args",
			R"(\18\1\2\3\4\5\6\)",
			/*segments*/ {Command},
			/*literals*/ {},
			/*commands*/ {Cmd{MSGC_WARP, 6, {1, 2, 3, 4, 5, 6}}},
			/*warnings*/ {}
		},
		TC{
			"Simple command with 6 args, but bad terminator",
			R"(\18\1\2\3\4\5\6)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(\18\1\2\3\4\5\6)"},
			/*commands*/ {},
			/*warnings*/ {R"(Expected slash (followed by either space or end of string) at end of command: \18\1\2\3\4\5\6)"}
		},
		TC{
			"Simple command using text name",
			R"(\Speed\1\)",
			/*segments*/ {Command},
			/*literals*/ {},
			/*commands*/ {Cmd{MSGC_SPEED, 1, {1}}},
			/*warnings*/ {}
		},
		TC{
			"Simple command using text name",
			R"(\GoIfCounter%\1\2\3\)",
			/*segments*/ {Command},
			/*literals*/ {},
			/*commands*/ {Cmd{MSGC_GOTOIFCTRPC, 3, {1, 2, 3}}},
			/*warnings*/ {}
		},
		TC{
			"Simple command using text name (case insensitive)",
			R"(\SPEED\1\)",
			/*segments*/ {Command},
			/*literals*/ {},
			/*commands*/ {Cmd{MSGC_SPEED, 1, {1}}},
			/*warnings*/ {}
		},
		TC{
			"Simple command using unknown text name",
			R"(\BadCommand\1\2\3\)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(\BadCommand\1\2\3\)"},
			/*commands*/ {},
			/*warnings*/ {R"(Ignoring unknown command: \BadCommand\1\2\3\)"}
		},
		TC{
			"Simple command using unknown text name (two in a row)",
			R"(\BadCommand\1\2\3\ \BadCommand\1\2\3\)",
			/*segments*/ {Invalid, Invalid},
			/*literals*/ {R"(\BadCommand\1\2\3\ )", R"(\BadCommand\1\2\3\)"},
			/*commands*/ {},
			/*warnings*/ {
				R"(Ignoring unknown command: \BadCommand\1\2\3\)",
				R"(Ignoring unknown command: \BadCommand\1\2\3\)",
			}
		},
		TC{
			"Literal newline turns into newline command",
			"Hello, world!\n^.^",
			/*segments*/ {Literal, Command, Literal},
			/*literals*/ {"Hello, world!", "^.^"},
			/*commands*/ {Cmd{MSGC_NEWLINE}},
			/*warnings*/ {}
		},
		TC{
			"Command newline turns into newline command",
			"Hello, world!\\Newline\\ ^.^",
			/*segments*/ {Literal, Command, Literal},
			/*literals*/ {"Hello, world!", "^.^"},
			/*commands*/ {Cmd{MSGC_NEWLINE}},
			/*warnings*/ {}
		},
		TC{
			"Mixed literal and command",
			R"(FIGHT \18\1\2\3\4\5\6\ BAG)",
			/*segments*/ {Literal, Command, Literal},
			/*literals*/ {"FIGHT ", "BAG"},
			/*commands*/ {Cmd{MSGC_WARP, 6, {1, 2, 3, 4, 5, 6}}},
			/*warnings*/ {}
		},
		TC{
			"Command followed by a space literal",
			R"(\10\1\2\  )", // Note the two spaces
			/*segments*/ {Command, Literal},
			/*literals*/ {" "}, // The second space
			/*commands*/ {Cmd{10, 2, {1, 2}}},
			/*warnings*/ {}
		},
		TC{
			"Command followed by escaped slash",
			R"(\10\1\2\ \\)",
			/*segments*/ {Command, Literal},
			/*literals*/ {R"(\)"},
			/*commands*/ {Cmd{10, 2, {1, 2}}},
			/*warnings*/ {}
		},
		TC{
			"Command followed by escaped slash and text",
			R"(\10\1\2\ \\More)",
			/*segments*/ {Command, Literal, Literal},
			/*literals*/ {R"(\)", "More"},
			/*commands*/ {Cmd{10, 2, {1, 2}}},
			/*warnings*/ {}
		},
		TC{
			"Escaped backslash in middle of literal",
			R"(Hello \\ world)",
			/*segments*/ {Literal, Literal},
			/*literals*/ {R"(Hello \)", R"( world)"},
			/*commands*/ {},
			/*warnings*/ {}
		},
		TC{
			"Escaped backslash at start, end, and multiple",
			R"(\\Hello\\\\world\\)",
			/*segments*/ {Literal, Literal, Literal, Literal},
			/*literals*/ {R"(\)", R"(Hello\)", R"(\)", R"(world\)"},
			/*commands*/ {},
			/*warnings*/ {}
		},
		TC{
			"Escaped backslash adjacent to commands",
			R"(Literal\\ \\\10\1\2\ \19\1\2\3\4\ \\More)",
			/*segments*/ {Literal, Literal, Command, Command, Literal, Literal},
			/*literals*/ {R"(Literal\)", R"( \)", R"(\)", R"(More)"},
			/*commands*/ {Cmd{MSGC_CTRUP, 2, {1, 2}}, Cmd{MSGC_SETSCREEND, 4, {1, 2, 3, 4}}},
			/*warnings*/ {}
		},
		TC{
			"Escaped backslash adjacent to invalid commands",
			R"(Literal\\ \\\10\1\2\ \19\1\2\3\4\\More)",
			/*segments*/ {Literal, Literal, Command, Invalid, Invalid},
			/*literals*/ {R"(Literal\)", R"( \)", R"(\19\1\2\3\4\)", R"(\More)"},
			/*commands*/ {Cmd{MSGC_CTRUP, 2, {1, 2}}},
			/*warnings*/ {
				R"(Expected slash (followed by either space or end of string) at end of command: \19\1\2\3\4\)",
				R"(Ignoring unknown command: \More)"
			}
		},
		TC{
			"Negative command code is ignored and produces warning",
			R"(Try me: \-2\10\20\)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(Try me: \-2\10\20\)"},
			/*commands*/ {},
			/*warnings*/ {R"(Ignoring unknown command: \-2\10\20\)"}
		},
		TC{
			"Negative arguments",
			R"(\18\-10\2\-30\4\-50\6\)",
			/*segments*/ {Command},
			/*literals*/ {},
			/*commands*/ {Cmd{MSGC_WARP, 6, {-10, 2, -30, 4, -50, 6}}},
			/*warnings*/ {}
		},
		TC{
			"Command with space",
			R"(\1\1\2\ )",
			/*segments*/ {Command},
			/*literals*/ {},
			/*commands*/ {Cmd{MSGC_COLOUR, 2, {1, 2}}},
			/*warnings*/ {}
		},
		TC{
			"Command at EOS",
			R"(\1\1\2\ )",
			/*segments*/ {Command},
			/*literals*/ {},
			/*commands*/ {Cmd{MSGC_COLOUR, 2, {1, 2}}},
			/*warnings*/ {}
		},
		TC{
			"Two commands",
			R"(\1\1\2\ \1\1\2\)",
			/*segments*/ {Command, Command},
			/*literals*/ {},
			/*commands*/ {Cmd{MSGC_COLOUR, 2, {1, 2}}, Cmd{MSGC_COLOUR, 2, {1, 2}}},
			/*warnings*/ {}
		},
		TC{
			"Command and literal",
			R"(\10\1\2\ More)",
			/*segments*/ {Command, Literal},
			/*literals*/ {"More"},
			/*commands*/ {Cmd{10, 2, {1, 2}}},
			/*warnings*/ {}
		},
		TC{
			"Literal and command",
			R"(Hello \10\1\2\)",
			/*segments*/ {Literal, Command},
			/*literals*/ {"Hello "},
			/*commands*/ {Cmd{10, 2, {1, 2}}},
			/*warnings*/ {}
		},
		TC{
			"Literal, command, literal",
			R"(Hello \10\1\2\ World)",
			/*segments*/ {Literal, Command, Literal},
			/*literals*/ {"Hello ", "World"},
			/*commands*/ {Cmd{10, 2, {1, 2}}},
			/*warnings*/ {}
		},
		TC{
			"Escaped slash literal (split-literal logic)",
			R"(Hello \\ \10\1\2\ World)",
			/*segments*/ {Literal, Literal, Command, Literal},
			/*literals*/ {"Hello \\", " ", "World"},
			/*commands*/ {Cmd{10, 2, {1, 2}}},
			/*warnings*/ {}
		},
		TC{
			"Error: Missing terminator slash",
			R"(\10\1\2More)",
			/*segments*/ {Invalid, Literal},
			/*literals*/ { R"(\10\1\2)", R"(More)"},
			/*commands*/ {},
			/*warnings*/ {R"(Expected slash (followed by either space or end of string) at end of command: \10\1\2)"}
		},
		TC{
			"Error: Invalid terminator (no space/EOS)",
			R"(\10\1\2\More)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(\10\1\2\More)",},
			/*commands*/ {},
			/*warnings*/ {R"(Expected slash (followed by either space or end of string) at end of command: \10\1\2\More)"}
		},
		TC{
			"Error: Missing arg",
			R"(\10\1\)", // Fails: MSGC_CTRUP expects 2 args, then terminator
			/*segments*/ {Invalid},
			/*literals*/ {R"(\10\1\)"},
			/*commands*/ {},
			/*warnings*/ {R"(Expected 2 args, but got 1 for command: \10\1\)"}
		},
		TC{
			"Error: Malformed command (bad code)",
			R"(\Hello)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(\Hello)"},
			/*commands*/ {},
			/*warnings*/ {R"(Ignoring unknown command: \Hello)"}
		},
		TC{
			"Negative args",
			R"(\10\-10\20\)",
			/*segments*/ {Command},
			/*literals*/ {},
			/*commands*/ {Cmd{10, 2, {-10, 20}}},
			/*warnings*/ {}
		},
		TC{
			"Error: bad terminator in second command followed by another bad command",
			R"(\10\1\2\ \20\1\2\3\4\\More)",
			/*segments*/ {Command, Invalid, Invalid},
			/*literals*/ {R"(\20\1\2\3\4\)", R"(\More)"},
			/*commands*/ {Cmd{10, 2, {1, 2}}},
			/*warnings*/ {
				R"(Expected slash (followed by either space or end of string) at end of command: \20\1\2\3\4\)",
				R"(Ignoring unknown command: \More)"
			}
		},
		TC{
			"Previous test case but valid and with an escaped slash",
			R"(\10\1\2\ \19\1\2\3\4\ \\More)",
			/*segments*/ {Command, Command, Literal, Literal},
			/*literals*/ {"\\", "More"},
			/*commands*/ {Cmd{10, 2, {1, 2}}, Cmd{19, 4, {1, 2, 3, 4}}},
			/*warnings*/ {}
		},
		TC{
			"Escaped slash at end of string",
			R"(Hello\\)",
			/*segments*/ {Literal},
			/*literals*/ {R"(Hello\)"},
			/*commands*/ {},
			/*warnings*/ {}
		},
		TC{
			"Command followed by escaped slash",
			R"(\22\ \\More)",
			/*segments*/ {Command, Literal, Literal},
			/*literals*/ {"\\", "More"},
			/*commands*/ {Cmd{MSGC_NAME, 0, {}}},
			/*warnings*/ {}
		},
		TC{
			"Error: Command with 0 args, invalid terminator",
			R"(\23\More)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(\23\More)"},
			/*commands*/ {},
			/*warnings*/ {R"(Expected slash (followed by either space or end of string) at end of command: \23\More)"}
		},
		TC{
			"Error: unknown command",
			R"(\99\1\)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(\99\1\)"},
			/*commands*/ {},
			/*warnings*/ {R"(Ignoring unknown command: \99\1\)"}
		},
		TC{
			"Error: unknown command",
			R"(\0Hello)",
			/*segments*/ {Invalid, Literal},
			/*literals*/ {R"(\0)", R"(Hello)"},
			/*commands*/ {},
			/*warnings*/ {R"(Ignoring unknown command: \0)"}
		},
		TC{
			"Error: unknown command",
			R"(\999Hello)",
			/*segments*/ {Invalid, Literal},
			/*literals*/ {R"(\999)", R"(Hello)"},
			/*commands*/ {},
			/*warnings*/ {R"(Ignoring unknown command: \999)"}
		},
		TC{
			"Error: incomplete arg",
			R"(\1\)", // MSGC_COLOUR expects 2 args
			/*segments*/ {Invalid},
			/*literals*/ {R"(\1\)"},
			/*commands*/ {},
			/*warnings*/ {R"(Expected 2 args, but got 0 for command: \1\)"}
		},
		TC{
			"Incomplete args (reaches end of string, no slash terminator)",
			R"(\3\1\2)", // MSGC_GOTOIFSCREEND expects 3 args
			/*segments*/ {Invalid},
			/*literals*/ {R"(\3\1\2)"},
			/*commands*/ {},
			/*warnings*/ {R"(Expected slash (followed by either space or end of string) at end of command: \3\1\2)"}
		},
		TC{
			"Malformed arg (non-digit)",
			R"(\1\1\FIGHT\)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(\1\1\FIGHT\)"},
			/*commands*/ {},
			/*warnings*/ {R"(Found non-numeric argument for command: \1\1\FIGHT\)"}
		},
		TC{
			"Malformed arg (just \\, then EOS)",
			R"(\128\1\)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(\128\1\)"},
			/*commands*/ {},
			/*warnings*/ {R"(Expected 5 args, but got 1 for command: \128\1\)"}
		},
		TC{
			"Trailing \\",
			R"(Hello\)",
			/*segments*/ {Literal, Invalid},
			/*literals*/ {R"(Hello)", R"(\)"},
			/*commands*/ {},
			/*warnings*/ {R"(Unexpected trailing slash (expected either a command or another slash))"}
		},
		TC{
			"Command code overflow",
			R"(\214749)", // MAX_SCC_ARG + 1
			/*segments*/ {Invalid},
			/*literals*/ {R"(\214749)"},
			/*commands*/ {},
			/*warnings*/ {R"(Ignoring unknown command: \214749)" }
		},
		TC{
			"Command code underflow",
			R"(\-214749)", // MIN_SCC_ARG - 1
			/*segments*/ {Invalid},
			/*literals*/ {R"(\-214749)"},
			/*commands*/ {},
			/*warnings*/ {R"(Ignoring unknown command: \-214749)" }
		},
		TC{
			"Argument overflow",
			R"(\129\214749\2\3\4\5\)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(\129\214749\2\3\4\5\)"},
			/*commands*/ {},
			/*warnings*/ {R"(Found argument that was too big for command: \129\214749\2\3\4\5\ (max value is 214748))"}
		},
		TC{
			"Argument underflow",
			R"(\129\-214749\2\3\4\5\)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(\129\-214749\2\3\4\5\)",},
			/*commands*/ {},
			/*warnings*/ {R"(Found argument that was too small for command: \129\-214749\2\3\4\5\ (min value is -214748))"}
		},
		TC{
			"Crazy string from tests/replays/misc/pkmn.qst",
			R"(\2\0\ \16\140\ \128\5\6\8\12\0\ \129\0\2\2\1\1\ FIGHT   \129\1\3\3\0\0\ BAG\25\ \129\2\0\0\3\3\ POKeMON \129\3\1\1\2\2\ RUN\130\ \131\0\4\ \131\1\0\ \131\2\0\ \131\3\0\)",
			/*segments*/ {Command, Command, Command, Command, Literal, Command, Literal, Command, Command, Literal, Command, Literal, Command, Command, Command, Command, Command},
			/*literals*/ {R"(FIGHT   )", R"(BAG)", R"(POKeMON )", R"(RUN)"},
			/*commands*/ {
				Cmd{2,   1, {0}},
				Cmd{16, 1, {140}},
				Cmd{128, 5, {5, 6, 8, 12, 0}},
				Cmd{129, 5, {0, 2, 2, 1, 1}},
				// "FIGHT   "
				Cmd{129, 5, {1, 3, 3, 0, 0}},
				// "BAG"
				Cmd{25,  0, {}},
				Cmd{129, 5, {2, 0, 0, 3, 3}},
				// "POKeMON "
				Cmd{129, 5, {3, 1, 1, 2, 2}},
				// "RUN"
				Cmd{130, 0, {}},
				Cmd{131, 2, {0, 4}},
				Cmd{131, 2, {1, 0}},
				Cmd{131, 2, {2, 0}},
				Cmd{131, 2, {3, 0}},
			},
			/*warnings*/ {}
		},
		TC{
			"Escaped { and }",
			R"(Hello \{world\})",
			/*segments*/ {Literal, Literal, Literal},
			/*literals*/ {"Hello ", "{world", "}"},
			/*commands*/ {},
			/*warnings*/ {}
		},
		// The plan is to extend SCCs considerably to support expressions inside curly braces.
		// https://discord.com/channels/876899628556091432/1277878877057978460/1360390301713170783
		TC{
			"Disallow unescaped { and }",
			R"(Hello {world})",
			/*segments*/ {Literal, Invalid},
			/*literals*/ {R"(Hello )", R"({world})"},
			/*commands*/ {},
			/*warnings*/ {R"(Error: { and } characters must be escaped: \{ and \})"}
		},
		TC{
			"Disallow unescaped { with no closing }",
			R"(Hello {world)",
			/*segments*/ {Literal, Invalid},
			/*literals*/ {R"(Hello )", R"({world)"},
			/*commands*/ {},
			/*warnings*/ {R"(Error: { and } characters must be escaped: \{ and \})"}
		},
		TC{
			"Error: { and } inside commands",
			R"(Hello \Color\{world}\)",
			/*segments*/ {Invalid},
			/*literals*/ {R"(Hello \Color\{world}\)"},
			/*commands*/ {},
			/*warnings*/ {R"(Ignoring unknown command: \Color\{world}\)"}
		},
	};
}

template <typename T>
void print_vec(const std::vector<T>& vec) {
	std::cerr << "{ ";
	for (size_t i = 0; i < vec.size(); ++i) {
		if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>) {
			std::cerr << "R\"(" << vec[i] << ")\"";
		} else {
			std::cerr << (int)vec[i];
		}
		if (i < vec.size() - 1) std::cerr << ", ";
	}
	std::cerr << " }";
}

void print_cmd(const StringCommand& cmd) {
	std::cerr << "{ code: " << (int)cmd.code 
			<< ", num_args: " << (int)cmd.num_args 
			<< ", args: [ ";
	for (int i = 0; i < cmd.num_args; ++i) {
		std::cerr << cmd.args[i] << (i == cmd.num_args - 1 ? "" : ", ");
	}
	std::cerr << " ] }";
}

void print_cmd(const ExpectedCommand& cmd) {
	std::cerr << "{ code: " << (int)cmd.code 
			<< ", num_args: " << (int)cmd.num_args 
			<< ", args: [ ";
	for (size_t i = 0; i < cmd.args.size(); ++i) {
		std::cerr << cmd.args[i] << (i == cmd.args.size() - 1 ? "" : ", ");
	}
	std::cerr << " ] }";
}

// --- ANSI Color Codes for legible output ---
#if defined(_WIN32)
// Windows doesn't support ANSI codes in cmd.exe by default
#define RESET ""
#define RED ""
#define GREEN ""
#define YELLOW ""
#define BLUE ""
#else
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#endif

static bool assert_expectations(const AsciiParserTestCase& tc, const value_and_warnings<ParsedMsgStr>& actual, std::string context = "")
{
	auto& [parsed_msg, warnings] = actual;
	bool passed = true;

	// Check segment types
	if (parsed_msg.segment_types != tc.expected_segment_types) {
		passed = false;
		std::cerr << RED << "  [FAIL] " << RESET << context << "Segment types mismatch.\n";
		std::cerr << "    Expected: "; print_vec(tc.expected_segment_types); std::cerr << "\n";
		std::cerr << "    Actual  : "; print_vec(parsed_msg.segment_types); std::cerr << "\n";
	}

	// Check literals
	if (parsed_msg.literals != tc.expected_literals) {
		passed = false;
		std::cerr << RED << "  [FAIL] " << RESET << context << "Literals mismatch.\n";
		std::cerr << "    Expected: "; print_vec(tc.expected_literals); std::cerr << "\n";
		std::cerr << "    Actual  : "; print_vec(parsed_msg.literals); std::cerr << "\n";
	}

	// Check commands
	if (parsed_msg.commands.size() != tc.expected_commands.size()) {
		passed = false;
		std::cerr << RED << "  [FAIL] " << RESET << context << "Command count mismatch.\n";
		std::cerr << "    Expected: " << tc.expected_commands.size() << "\n";
		std::cerr << "    Actual  : " << parsed_msg.commands.size() << "\n";
	} else {
		for (size_t i = 0; i < parsed_msg.commands.size(); ++i) {
			const auto& actual_cmd = parsed_msg.commands[i];
			const auto& expected_cmd = tc.expected_commands[i];
			bool cmd_ok = true;

			if (actual_cmd.code != expected_cmd.code) cmd_ok = false;
			if (actual_cmd.num_args != expected_cmd.num_args) cmd_ok = false;

			// Compare args
			if (cmd_ok) { // Only check args if basics are right
				if (static_cast<size_t>(actual_cmd.num_args) != expected_cmd.args.size()) {
					std::cerr << YELLOW << "  [NOTE] " << RESET << context << "Test case definition mismatch: "
							<< "expected num_args (" << expected_cmd.num_args
							<< ") != size of expected_args vector ("
							<< expected_cmd.args.size() << ")\n";
					cmd_ok = false;
				} else {
					for (int j = 0; j < actual_cmd.num_args; ++j) {
						if (actual_cmd.args[j] != expected_cmd.args[j]) {
							cmd_ok = false;
							break;
						}
					}
				}
			}

			if (!cmd_ok) {
				passed = false;
				std::cerr << RED << "  [FAIL] " << RESET << context << "Command " << i << " mismatch.\n";
				std::cerr << "    Expected: "; print_cmd(expected_cmd); std::cerr << "\n";
				std::cerr << "    Actual  : "; print_cmd(actual_cmd); std::cerr << "\n";
			}
		}
	}

	// Check warnings
	if (warnings != tc.expected_warnings) {
		passed = false;
		std::cerr << RED << "  [FAIL] " << RESET << context << "warnings mismatch.\n";
		std::cerr << "    Expected: "; print_vec(tc.expected_warnings); std::cerr << "\n";
		std::cerr << "    Actual  : "; print_vec(warnings); std::cerr << "\n";
	}

	return passed;
}

void test_scc()
{
	std::cerr << "========================================\n";
	std::cerr << "                scc_test.cpp\n";
	std::cerr << "========================================\n";

	auto all_test_cases = get_ascii_parser_test_cases();
	int tests_failed = 0;
	int test_num = 1;

	for (const auto& tc : all_test_cases)
	{
		bool test_passed = true;
		std::cerr << "--- Test " << std::setw(2) << test_num << ": "
				<< tc.description << " ---" << std::endl;

		auto result = parse_ascii_msg_str(tc.input);
		if (!assert_expectations(tc, result))
			test_passed = false;

		if (test_passed)
		{
			std::string serialized = result.first.serialize();
			auto result2 = parse_ascii_msg_str(serialized);
			if (!assert_expectations(tc, result2, "(roundtrip) "))
				test_passed = false;
		}

		// --- Test Summary ---
		if (test_passed) {
			std::cerr << GREEN << "  [PASS]" << RESET << "\n";
		} else {
			tests_failed++;
			std::cerr << "--- End of Test " << test_num << " ---\n";
		}
		std::cerr << std::endl;
		test_num++;
	}

	// --- Final Report ---
	std::cerr << "========================================\n";
	std::cerr << "                TEST SUMMARY\n";
	std::cerr << "========================================\n";
	if (tests_failed == 0) {
		std::cerr << GREEN << "All " << all_test_cases.size() << " tests passed!" << RESET << std::endl;
	} else {
		std::cerr << RED << tests_failed << " out of " << all_test_cases.size() << " tests failed." << RESET << std::endl;
	}
	std::cerr << "========================================\n";

	if (tests_failed)
		exit(1);
}

// TODO: make this not needed to compile...
bool DragAspect = false;
double aspect_ratio = 0;
int window_min_width = 0, window_min_height = 0;

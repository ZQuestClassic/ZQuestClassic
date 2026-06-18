// Unit tests for the ZASM optimizer. These reach into the optimizer's internals
// via zasm_optimize_internal.h. Run with `zplayer -test-zc`.

#include "test_runner/test_runner.h"
#include "zc/zasm_optimize.h"
#include "zc/zasm_optimize_internal.h"
#include "zc/zasm_utils.h"
#include "zc/ffscript.h"
#include "components/zasm/defines.h"
#include "components/zasm/serialize.h"
#include "base/util.h"
#include "base/zapp.h"
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#define C(i) (ctx.script->zasm[i])

// State for the EXPECT macro and the currently-running test.
static const char* expect_file;
static int expect_line;
static std::string name;
static bool current_test_failed;
static TestResults* current_results;

#define EXPECT(...) {\
	expect_file = __FILE__;\
	expect_line = __LINE__;\
	expect(__VA_ARGS__);\
}

static void print_string_delta(const std::string& expected, const std::string& got)
{
	std::vector<std::string> expected_lines, got_lines;
	util::split(expected, expected_lines, '\n');
	util::split(got, got_lines, '\n');
	if (expected_lines.size() <= 1 && got_lines.size() <= 1)
		return;

	fmt::println("= delta\n");
	for (int i = 0; i < std::max(expected_lines.size(), got_lines.size()); i++)
	{
		std::string expected_line = i < expected_lines.size() ? expected_lines[i] : "EOF";
		std::string got_line = i < got_lines.size() ? got_lines[i] : "EOF";
		if (expected_line == got_line)
		{
			fmt::println("  {}", expected_lines[i]);
		}
		else
		{
			fmt::println("- {}", expected_lines[i]);
			fmt::println("+ {}", got_lines[i]);
		}
	}
}

static void normalize_whitespace(std::string& text)
{
	util::trimstr(text);
	std::vector<std::string> lines;
	util::split(text, lines, '\n');
	for (auto& line : lines)
		line.erase(line.find_last_not_of("\t\n\v\f\r ") + 1);

    std::ostringstream os;
    for (int i = 0; i < lines.size() - 1; ++i) {
        os << lines.at(i) << "\n";
    }
    os << lines.back();
    text = os.str();
}

static std::string zasm_to_string_clean(const zasm_script* script)
{
	std::string str = zasm_to_string(script);
	normalize_whitespace(str);
	util::trimstr(str);
	return str + "\n";
}

template <typename T1, typename T2>
static void expect(std::string name, const T1& expected, const T2& got)
{
	if (expected != got)
	{
		current_test_failed = true;
		fmt::println("failure: {}\n{}:{}\n", name, expect_file, expect_line);
		fmt::println("expected {}, but got: {}\n", expected, got);
	}
}

static void expect(std::string name, const SimulationValue& expected, const SimulationValue& got)
{
	if (expected != got)
	{
		current_test_failed = true;
		fmt::println("failure: {}\n{}:{}\n", name, expect_file, expect_line);
		fmt::println("expected {}, but got: {}\n", expected.to_string(), got.to_string());
	}
}

static void expect(std::string name, zasm_script* script, std::vector<ffscript>&& s)
{
	bool success = script->size == s.size();
	for (int i = 0; i < s.size(); i++)
	{
		if (!success) break;
		if (script->zasm[i].command == NOP && s[i].command == NOP)
			continue;
		if (script->zasm[i] != s[i])
			success = false;
	}

	if (!success)
	{
		current_test_failed = true;
		auto expected_script = zasm_script{std::move(s)};
		std::string expected = zasm_to_string_clean(&expected_script);
		std::string got = zasm_to_string_clean(script);
		fmt::println("failure: {}\n{}:{}\n", name, expect_file, expect_line);
		fmt::println("= expected:\n\n{}", expected);
		fmt::println("= got:\n\n{}", got);
		print_string_delta(expected, got);
	}
}

// Simplified version of parsing ZASM.
static zasm_script zasm_from_string(std::string text)
{
	normalize_whitespace(text);

	std::vector<ffscript> instructions;
	std::vector<std::string> lines;
	util::split(text, lines, '\n');
	for (auto& line : lines)
	{
		util::trimstr(line);
		if (line.empty() || line.starts_with("Function"))
			continue;

		instructions.push_back(parse_zasm_op(line));
	}

	instructions.emplace_back(0xFFFF);
	return {std::move(instructions)};
}

// Used by test_optimize_zasm_unit.py
void zasm_optimize_run_for_file(std::string path)
{
	std::string text = util::read_text_file(path);

	std::vector<std::string> lines;
	util::split(text, lines, '\n');
	int last_comment_line = 0;
	for (auto& line : lines)
	{
		if (line.starts_with((";")))
		{
			last_comment_line++;
		}
		else break;
	}

	std::string zasm;
	for (int i = last_comment_line; i < lines.size(); i++)
		zasm += lines[i] + "\n";

	auto script = zasm_from_string(zasm);
	// Configures the first function as the "entry".
	script.script_datas.emplace_back(new script_data(ScriptType::None, 0));
	script.script_datas.back()->pc = 0;

	// Just in case there is a minor roundtrip difference, resave the input.
	{
		std::ofstream out(path, std::ios::binary);
		for (int i = 0; i < last_comment_line; i++)
			out << lines[i] + "\n";
		out << zasm_to_string_clean(&script);
		out.close();
	}

	fmt::println("\noutput:");
	zasm_optimize_verbose = true;
	zasm_init_meta_cache();
	zasm_optimize_script(&script);
	fmt::println("{}", zasm_to_string_clean(&script));
}

// Scope guard backing the TEST macro. Each matching TEST() counts as one test in
// the TestResults, and is marked failed if any EXPECT inside it failed.
struct TestScope
{
	bool should_run;
	bool entered = false;

	explicit TestScope(const std::string& test_name)
	{
		static auto filter = get_flag_string("-test-filter");
		should_run = !filter.has_value() || test_name.find(filter.value()) != std::string::npos;
		if (should_run)
		{
			name = test_name;
			current_test_failed = false;
			current_results->total++;
		}
	}

	bool keep()
	{
		if (!should_run)
			return false;
		if (entered)
		{
			if (current_test_failed)
				current_results->failed++;
			return false;
		}
		entered = true;
		return true;
	}
};

#define TEST(name_val) for (TestScope _scope(name_val); _scope.keep(); )

TestResults test_zasm_optimize([[maybe_unused]] bool verbose)
{
	TestResults tr{};
	current_results = &tr;
	zasm_script script;

	TEST("evaluate_binary_op")
	{
		// Basics
		// number op number -> number
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_EQ, num_one, num_one));
		EXPECT(name, num(10000),
			evaluate_binary_op(CMP_EQ | CMP_SETI, num_one, num_one));
		EXPECT(name, num(10000),
			evaluate_binary_op(CMP_EQ | CMP_SETI, num_one, num_one));
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_EQ, num_one, num(1337)));
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_LT, num_one, num(1337)));

		EXPECT(name, num_one,
			evaluate_binary_op(CMP_EQ, num_one, num_one));
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_NE, num_one, num_one));
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_LT, num_one, num_one));
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_LE, num_one, num_one));
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_LE, num_zero, num_one));

		// CMP_BOOL
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_EQ | CMP_BOOL, num_one, num_one));
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_EQ | CMP_BOOL, num(100), num_one));
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_EQ | CMP_BOOL, num_zero, num_one));
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_EQ | CMP_BOOL, num_zero, num_zero));

		// Comparing a number with a register results in an expression
		// register op number -> expression
		// number op register -> expression
		EXPECT(name, expr(reg(1), CMP_LT, num(1337)),
			evaluate_binary_op(CMP_LT, reg(1), num(1337)));
		EXPECT(name, expr(reg(1), CMP_GT, num(1337)),
			evaluate_binary_op(CMP_LT, num(1337), reg(1)));
		EXPECT(name, expr(reg(1), CMP_GE, num(1337)),
			evaluate_binary_op(CMP_LE, num(1337), reg(1)));

		// Comparing with "Never"
		// any Never op -> num_zero
		EXPECT(name, num_zero,
			evaluate_binary_op(0, num_one, num(1337)));
		EXPECT(name, num_zero,
			evaluate_binary_op(0, reg(1), num(1337)));
		EXPECT(name, num_zero,
			evaluate_binary_op(0, {ValueType::Expression}, {ValueType::Expression}));
		EXPECT(name, num_zero,
			evaluate_binary_op(0, {ValueType::Unknown}, {ValueType::Unknown}));

		// Comparing with "Always"
		// any Always op -> num_one
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_FLAGS, num_one, num(1337)));
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_FLAGS, reg(1), num(1337)));
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_FLAGS, {ValueType::Expression}, {ValueType::Expression}));
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_FLAGS, {ValueType::Unknown}, {ValueType::Unknown}));
		EXPECT(name, num(10000),
			evaluate_binary_op(CMP_FLAGS | CMP_SETI, {ValueType::Unknown}, {ValueType::Unknown}));

		// Comparison with equal operands.
		auto do_identity_test_cases = [&](const SimulationValue& expected_result, int cmp) {
			auto T = [&](const SimulationValue& x){
				EXPECT(name, expected_result,
					evaluate_binary_op(cmp, x, x));
			};
			T(num(0));
			T(num(1));
			T(num(10000));
			T(reg(1));
			T(expr(reg(1), CMP_EQ, num(2)));
			auto compound_expr = SimulationValue{ValueType::Expression, CMP_EQ};
			compound_expr.op1 = std::make_shared<SimulationValue>(expr(reg(2), CMP_EQ, num(1337)));
			compound_expr.op2 = std::make_shared<SimulationValue>(expr(reg(2), CMP_EQ, num(1337)));
			T(compound_expr);
		};
		do_identity_test_cases(num_one, CMP_FLAGS);
		do_identity_test_cases(num_one, CMP_EQ);
		do_identity_test_cases(num_one, CMP_GE);
		do_identity_test_cases(num_one, CMP_LE);
		do_identity_test_cases(num_zero, CMP_NE);
		do_identity_test_cases(num_zero, CMP_GT);
		do_identity_test_cases(num_zero, CMP_LT);
		do_identity_test_cases(num_zero, 0);

		EXPECT(name, {ValueType::Unknown},
			evaluate_binary_op(CMP_EQ, {ValueType::Unknown}, {ValueType::Unknown}));
		EXPECT(name, {ValueType::Unknown},
			evaluate_binary_op(CMP_NE, {ValueType::Unknown}, {ValueType::Unknown}));

		// Comparing independent expressions
		// expr_a ? expr_b -> expr_c
		auto compound_expr = SimulationValue{ValueType::Expression, CMP_EQ};
		compound_expr.op1 = std::make_shared<SimulationValue>(expr(reg(1), CMP_EQ, num(1337)));
		compound_expr.op2 = std::make_shared<SimulationValue>(expr(reg(2), CMP_EQ, num(1337)));
		EXPECT(name, compound_expr,
			evaluate_binary_op(CMP_EQ, expr(reg(1), CMP_EQ, num(1337)), expr(reg(2), CMP_EQ, num(1337))));

		// Boolean cast.
		// int CMP_NE 0 -> boolean
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_NE, num(1337), num(0)));

		// Returns boolean when possible.
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_EQ, num_zero, num(0)));
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_EQ, num_one, num(0)));
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_EQ, num(10000), num(0)));

		// Negation.
		// x CMP_EQ 0 -> !x
		EXPECT(name, expr(reg(2), CMP_LT, num(1337)),
			evaluate_binary_op(CMP_EQ, expr(reg(2), CMP_GE, num(1337)), num(0)));
		EXPECT(name, expr(reg(2), CMP_LT, num(1337)),
			evaluate_binary_op(CMP_EQ, expr(reg(2), CMP_GE, num(1337)), num_zero));
		EXPECT(name, expr(reg(2), CMP_LT, num(1337)),
			evaluate_binary_op(CMP_NE, expr(reg(2), CMP_GE, num(1337)), num_one));

		// Basic reductions.
		// (x == y) == True -> x == y
		EXPECT(name, expr(reg(2), CMP_EQ, reg(3)),
			evaluate_binary_op(CMP_EQ, expr(reg(2), CMP_EQ, reg(3)), num_one));
		EXPECT(name, expr(reg(2), CMP_EQ, reg(3)),
			evaluate_binary_op(CMP_EQ, num_one, expr(reg(2), CMP_EQ, reg(3))));
		// (x == y) == False -> x != y
		EXPECT(name, expr(reg(2), CMP_NE, reg(3)),
			evaluate_binary_op(CMP_EQ, expr(reg(2), CMP_EQ, reg(3)), num_zero));
		EXPECT(name, expr(reg(2), CMP_NE, reg(3)),
			evaluate_binary_op(CMP_EQ, num_zero, expr(reg(2), CMP_EQ, reg(3))));

		// Comparing w/o CMP_SETI on expressions w/ CMP_SETI removes CMP_SETI.
		EXPECT(name, expr(reg(2), CMP_EQ, reg(3)),
			evaluate_binary_op(CMP_EQ, expr(reg(2), CMP_EQ|CMP_SETI, reg(3)), num_one));

		// Weird stuff.
		// 0 <= (10 < D(2)) -> true
		EXPECT(name, num_one,
			evaluate_binary_op(CMP_LE, num(0), expr(num(10), CMP_LT, reg(2))));
		// 0 > (10 < D(2)) -> false
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_GT, num(0), expr(num(10), CMP_LT, reg(2))));
		// 0 >= (10 < D(2)) -> 10 < D(2)
		EXPECT(name, expr(num(10), CMP_LT, reg(2)),
			evaluate_binary_op(CMP_GE, num(0), expr(num(10), CMP_LT, reg(2))));
		// 0 < (10 < D(2)) -> 10 < D(2)
		EXPECT(name, expr(num(10), CMP_LT, reg(2)),
			evaluate_binary_op(CMP_LT, num(0), expr(num(10), CMP_LT, reg(2))));
		// 1 < (10 < D(2)) -> false
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_LT, num(1), expr(num(10), CMP_LT, reg(2))));
		// 10000 < (10 i< D(2)) -> 10 < D(2)
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_LT, num(10000), expr(num(10), CMP_LT | CMP_SETI, reg(2))));
		// 0 > (10 i< D(2)) -> 10 i< D(2)
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_GT, num(0), expr(num(10), CMP_LT, reg(2))));

		// Reduce comparison between mutually exclusive expressions.
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_EQ, expr(reg(2), CMP_GT, num(5)), expr(reg(2), CMP_LE, num(5))));
		EXPECT(name, num_zero,
			evaluate_binary_op(CMP_EQ, expr(reg(2), CMP_EQ, num(5)), expr(reg(2), CMP_NE, num(5))));
	}

	TEST("simulate")
	{
		std::vector<ffscript> s = {
			/*  0 */ {COMPAREV, D(2), 0},         // [Block 0 -> 1, 2]
			/*  1 */ {SETFALSE, D(2)},
			/*  2 */ {COMPAREV, D(3), 0},
			/*  3 */ {SETFALSE, D(3)},
			/*  4 */ {COMPARER, D(2), D(3)},
			/*  5 */ {SETFALSE, D(2)},
			/*  6 */ {COMPAREV, D(2), 0},
			/*  7 */ {GOTOTRUE, 9},

			/*  8 */ {TRACEV, 1},                 // [Block 1 -> 2]

			/*  9 */ {QUIT},                      // [Block 2 ->  ]
			/* 10 */ {0xFFFF},
		};
		script = zasm_script{0, "", std::move(s)};

		StructuredZasm structured_zasm = zasm_construct_structured(&script);
		OptContext ctx = create_context(structured_zasm, &script, structured_zasm.functions.at(0));
		SimulationState state{};
		state.pc = 0;
		state.final_pc = 7;

		simulate_and_advance(ctx, state);
		EXPECT(name, 1, state.pc);
		EXPECT(name, reg(2), state.operand_1);
		EXPECT(name, num(0), state.operand_2);

		simulate_and_advance(ctx, state);
		EXPECT(name, 2, state.pc);
		EXPECT(name, "Bool(D2)", state.d[2].to_string());

		simulate_and_advance(ctx, state);
		simulate_and_advance(ctx, state);
		EXPECT(name, 4, state.pc);
		EXPECT(name, "Bool(D3)", state.d[3].to_string());

		simulate_and_advance(ctx, state);
		simulate_and_advance(ctx, state);
		EXPECT(name, 6, state.pc);
		EXPECT(name, "(Bool(D2)) != (Bool(D3))", state.d[2].to_string());

		simulate_and_advance(ctx, state);
		simulate_and_advance(ctx, state);
		EXPECT(name, 7, state.pc);
		int cmp = command_to_cmp(C(state.pc).command, C(state.pc).arg1);
		EXPECT(name, "(Bool(D2)) == (Bool(D3))", evaluate_binary_op(cmp, state.operand_1, state.operand_2).to_string());
	}

	TEST("compile_conditional")
	{
		{
			std::vector<ffscript> r;
			compile_conditional(r, {GOTOCMP, 0, CMP_EQ}, reg(2), num_one);
			auto script = zasm_script(std::move(r));
			EXPECT(name, &script, {
				{COMPAREV, D(2), 1},
				{GOTOCMP, 0, CMP_EQ},
			});
		}

		{
			std::vector<ffscript> r;
			auto e = expr(reg(2), CMP_NE, num(0));
			compile_conditional(r, {GOTOCMP, 0, CMP_EQ},
				e, num(1));
			auto script = zasm_script(std::move(r));
			EXPECT(name, &script, {
				{COMPAREV, D(2), 0},
				{GOTOCMP, 0, CMP_NE},
			});
		}

		{
			std::vector<ffscript> r;
			auto e = expr(reg(2), CMP_GT, num(10));
			compile_conditional(r, {GOTOCMP, 0, CMP_NE},
				e, num(0));
			auto script = zasm_script(std::move(r));
			EXPECT(name, &script, {
				{COMPAREV, D(2), 10},
				{GOTOCMP, 0, CMP_GT},
			});
		}

		{
			std::vector<ffscript> r;
			auto e = expr(reg(2), CMP_NE, num(0));
			compile_conditional(r, {GOTOCMP, 0, CMP_NE},
				e, num(0));
			auto script = zasm_script(std::move(r));
			EXPECT(name, &script, {
				{COMPAREV, D(2), 0},
				{GOTOCMP, 0, CMP_NE},
			});
		}

		{
			std::vector<ffscript> r;
			compile_conditional(r, {GOTOCMP, 0, CMP_NE},
				boolean_cast(reg(2)), boolean_cast(reg(3)));
			auto script = zasm_script(std::move(r));
			EXPECT(name, &script, {
				{COMPARER, D(2), D(3)},
				{GOTOCMP, 0, CMP_NE|CMP_BOOL},
			});
		}
	}

	return tr;
}

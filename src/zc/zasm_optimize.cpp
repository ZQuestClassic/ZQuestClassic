// There are multiple tests that ensure the correctness of these optimizations:
//
// 1. python tests/test_optimize_zasm_unit.py
//
//    Optimizes small synthetic snippets of ZASM, each saved as a snapshot test.
//    See the files in tests/optimize_zasm and their expected values in tests/snapshots/optimize_zasm_unit
//    To see all at once: python tests/test_optimize_zasm_unit.py --print
//
// 2. python tests/test_optimize_zasm.py
//
//    Optimizes the scripts in all the quests in tests/replays, and creates a snapshot test for the summary
//    of the optimizer
//
// 3. zplayer -test-zc will run a few unit tests, located in this file.
//
// 4. python scripts/run_for_every_qst.py ./build/Debug/zplayer -extract-zasm %s -extract-zasm-optimize 2>&1 | code -
//
//    Run in debug mode (for asserts) on every quest in the database.
//
// 5. Finally, all the replay tests run with these optimizations applied, and since as of this writing no replays
//    were recorded w/ these optimizations, they grant a high confidence that these optimizations are sound.

#include "zc/zasm_optimize.h"
#include "base/general.h"
#include "base/util.h"
#include "base/zapp.h"
#include "base/zdefs.h"
#include "parser/parserDefs.h"
#include "zc/ffscript.h"
#include "zc/script_debug.h"
#include "zc/zasm_utils.h"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <memory>
#include <optional>
#include <stdbool.h>
#include <string>
#include <type_traits>
#include <utility>

static bool verbose = false;

bool zasm_optimize_enabled()
{
	static bool enabled = get_flag_bool("-optimize-zasm").value_or(false) || zc_get_config("zeldadx", "optimize_zasm", true);
	return enabled;
}

// Very useful tool for identifying a single bad optimization.
// Use with a tool like `find-first-fail`: https://gitlab.com/ole.tange/tangetools/-/blob/master/find-first-fail/find-first-fail
// 1. Place `if (bisect_tool_should_skip()) return;` somewhere before problematic code
// 2. Make a new script `tmp.sh` calling a failing replay:
//        python tests/run_replay_tests.py --filter stellar --frame 40000 --extra_args="-test-bisect $1"
// 3. Run the bisect script (may need to increase the end range up to 100000 or more):
//        bash ~/tools/find-first-fail.sh -s 0 -e 1000 -v -q bash tmp.sh
// 4. For the number given, set `-test-bisect` to that, and set a breakpoint
//    after the call to bisect_tool_should_skip. Whatever block being processed is the one to focus on.
static bool bisect_tool_should_skip()
{
	static int c = 0;
	static int x = get_flag_int("-test-bisect").value();
	// Skip the first x calls.
	bool skip = 0 <= c && c < x;
	c++;
	return skip;
}

enum class ValueType
{
	Uninitialized,
	Number,
	Register,
	Expression,
	Unknown,
};

struct SimulationValue
{
	bool operator==(const SimulationValue& other) const
	{
		if (type != other.type) return false;
		if (data != other.data) return false;
		if (type == ValueType::Expression)
		{
			if (!(*op1.get()).operator==(*other.op1.get())) return false;
			if (!(*op2.get()).operator==(*other.op2.get())) return false;
		}
		return true;
	}

	static SimulationValue num(int x)
	{
		return {ValueType::Number, x};
	}

	static SimulationValue reg(int x)
	{
		return {ValueType::Register, x};
	}

	static SimulationValue expr(SimulationValue x, int cmp, SimulationValue y)
	{
		auto op1 = std::make_shared<SimulationValue>(x);
		auto op2 = std::make_shared<SimulationValue>(y);
		return {ValueType::Expression, cmp, op1, op2};
	}

	bool is_register() const
	{
		return type == ValueType::Register;
	}

	bool is_number() const
	{
		return type == ValueType::Number;
	}

	bool is_expression() const
	{
		return type == ValueType::Expression;
	}

	bool is_equality_expression() const
	{
		return type == ValueType::Expression && (data & CMP_EQ);
	}

	SimulationValue negate() const
	{
		switch (type)
		{
			case ValueType::Number:
				return num(-data);
			case ValueType::Expression:
				return expr(*op1, INVERT_CMP(data), *op2);
			default:
				return {ValueType::Unknown};
		}
	}

	bool is_mutually_exclusive(const SimulationValue& other) const
	{
		ASSERT(type == ValueType::Expression);
		if (other.type != ValueType::Expression)
			return false;
		return *this == other.negate();
	}

	std::string to_string() const
	{
		if (type == ValueType::Expression)
		{
			if (is_bool_cast())
				return fmt::format("{}({})", data & CMP_SETI ? "IBool" : "Bool", op1->to_string());
			if (is_bool_cast_negated())
				return fmt::format("!{}({})", data & CMP_SETI ? "IBool" : "Bool", op1->to_string());

			std::string lhs = op1->type == ValueType::Expression ? fmt::format("({})", op1->to_string()) : op1->to_string();
			std::string rhs = op2->type == ValueType::Expression ? fmt::format("({})", op2->to_string()) : op2->to_string();

			if (data & CMP_BOOL)
				return fmt::format("Bool({}) {} Bool({})", lhs, CMP_STR(data | ~CMP_BOOL), rhs);
			return fmt::format("{} {} {}", lhs, CMP_STR(data), rhs);
		}
		if (type == ValueType::Register)
			return fmt::format("D{}", data);
		if (type == ValueType::Number)
			return fmt::format("{}", data);
		return "?";
	}

	bool is_bool_cast() const
	{
		if (type != ValueType::Expression || !(data & CMP_FLAGS))
			return false;
		if (!op2->is_number())
			return false;
		return op2->data == 0 && (data & CMP_FLAGS) == CMP_NE;
	}

	bool is_bool_cast_negated() const
	{
		if (type != ValueType::Expression || !(data & CMP_FLAGS))
			return false;
		if (!op2->is_number())
			return false;
		return op2->data == 0 && (data & CMP_FLAGS) == CMP_EQ;
	}

	void replace_value(const SimulationValue& match, const SimulationValue& replace_with)
	{
		if (*this == match)
			*this = replace_with;

		if (type == ValueType::Expression)
		{
			op1->replace_value(match, replace_with);
			op2->replace_value(match, replace_with);
		}
	}

	ValueType type;
	// [number] integer value
	// [register] register
	// [expression] cmp
	int data;
	// [expression]
	std::shared_ptr<SimulationValue> op1;
	std::shared_ptr<SimulationValue> op2;
};

#define num(x) SimulationValue::num(x)
#define reg(x) SimulationValue::reg(x)
#define expr(x, cmp, y) SimulationValue::expr(x, cmp, y)
#define boolean_cast(x) SimulationValue::expr(x, CMP_NE, num(0))
#define num_one num(1)
#define num_zero num(0)

template<class... Args>
bool one_of(const unsigned int var, const Args&... args)
{
  return ((var == args) || ...);
}

static const char *expect_file;
static int expect_line;
static bool tests_passed;
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

std::string zasm_to_string_clean(const script_data* script)
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
		tests_passed = false;
		fmt::println("failure: {}\n{}:{}\n", name, expect_file, expect_line);
		fmt::println("expected {}, but got: {}\n", expected, got);
	}
}

static void expect(std::string name, const SimulationValue& expected, const SimulationValue& got)
{
	if (expected != got)
	{
		tests_passed = false;
		fmt::println("failure: {}\n{}:{}\n", name, expect_file, expect_line);
		fmt::println("expected {}, but got: {}\n", expected.to_string(), got.to_string());
	}
}

static script_data script_from_vec(std::vector<ffscript>& s)
{
	script_data script(ScriptType::None, 0);
	s.push_back(0xFFFF);
	script.zasm = s.data();
	script.recalc_size();
	return script;
}

static std::string zasm_to_string(ffscript* s, size_t len)
{
	script_data script(ScriptType::None, 0);
	script.zasm = s;
	script.recalc_size();
	std::string result = zasm_to_string_clean(&script);
	script.zasm = nullptr;
	return result;
}

static void expect(std::string name, script_data* script, std::vector<ffscript> s)
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
		tests_passed = false;
		std::string expected = zasm_to_string(&s[0], s.size());
		std::string got = zasm_to_string(&script->zasm[0], script->size);
		fmt::println("failure: {}\n{}:{}\n", name, expect_file, expect_line);
		fmt::println("= expected:\n\n{}", expected);
		fmt::println("= got:\n\n{}", got);
		print_string_delta(expected, got);
	}
}

struct OptContext
{
	uint32_t saved;
	script_data* script;
	ZasmFunction fn;
	ZasmCFG cfg;
	bool cfg_stale;
	std::vector<pc_t> block_starts;
	std::set<pc_t> block_unreachable;
	StructuredZasm* structured_zasm;
	bool debug;
};

#define C(i) (ctx.script->zasm[i])
#define E(i) (ctx.cfg.block_edges.at(i))

static OptContext create_context_no_cfg(StructuredZasm& structured_zasm, script_data* script, const ZasmFunction& fn)
{
	OptContext ctx{};
	ctx.structured_zasm = &structured_zasm;
	ctx.script = script;
	ctx.fn = fn;
	ctx.debug = verbose;
	ctx.cfg_stale = true;
	return ctx;
}

static void add_context_cfg(OptContext& ctx)
{
	if (!ctx.cfg_stale)
		return;

	ctx.cfg = zasm_construct_cfg(ctx.script, {{ctx.fn.start_pc, ctx.fn.final_pc}});
	ctx.block_starts = std::vector<pc_t>(ctx.cfg.block_starts.begin(), ctx.cfg.block_starts.end());
	ctx.block_unreachable.clear();
	ctx.cfg_stale = false;
}

static OptContext create_context(StructuredZasm& structured_zasm, script_data* script, const ZasmFunction& fn)
{
	auto ctx = create_context_no_cfg(structured_zasm, script, fn);
	add_context_cfg(ctx);
	return ctx;
}

static void remove(OptContext& ctx, pc_t start, pc_t final)
{
	for (int i = start; i <= final; i++)
		C(i).command = NOP;
	ctx.saved += final - start + 1;
}

static void remove(OptContext& ctx, pc_t pc)
{
	C(pc).command = NOP;
	ctx.saved += 1;
}

static void optimize_by_block(OptContext& ctx, std::function<void(pc_t, pc_t, pc_t)> cb)
{
	for (pc_t i = 0; i < ctx.block_starts.size(); i++)
	{
		if (ctx.block_unreachable.contains(i))
			continue;

		pc_t start_pc = ctx.block_starts[i];
		pc_t final_pc = i == ctx.block_starts.size() - 1 ?
			ctx.fn.final_pc :
			ctx.block_starts[i + 1] - 1;
		cb(i, start_pc, final_pc);
	}
}

static void optimize_goto_next_instruction(OptContext& ctx)
{
	for (pc_t i = ctx.fn.start_pc; i < ctx.fn.final_pc; i++)
	{
		// If this is a GOTO to the next instruction, remove it.
		// This produces better blocks in the CFG construction because:
		// - usages of the CFG may assume blocks with a conditional branch MUST have two edges to distinct blocks
		// - it also is a minor optimization
		if (command_is_goto(C(i).command) && C(i).arg1 == i + 1)
		{
			remove(ctx, i);
			ctx.cfg_stale = true;
		}
	}
}

static void optimize_conseq_additive_impl(OptContext& ctx, word from, word to, bool write_at_end = false)
{
	optimize_by_block(ctx, [&](pc_t block_index, pc_t start_pc, pc_t final_pc){
		for (pc_t j = final_pc; j > start_pc; j--)
		{
			int command = C(j).command;
			int arg1 = C(j).arg1;
			if (command != from) continue;

			if ((from == PUSHV || from == PUSHR) && ctx.structured_zasm->function_calls.contains(j + 1))
			{
				// Don't break the thing that function calls are derived from in older ZASM.
				continue;
			}

			pc_t start = j;
			pc_t end = j;
			while (start >= start_pc)
			{
				int prev_command = C(start - 1).command;
				int prev_arg1 = C(start - 1).arg1;
				if (prev_command != from)
					break;
				// D5 is a special "null" register - it is never valid to read
				// from it, so we are free to remove writes.
				// ...except for the initial script call, which may have set initd[5].
				if (!(prev_arg1 == arg1 || (ctx.fn.id != 0 && prev_arg1 == D(5))))
					break;

				start--;
			}

			int count = end - start + 1;
			if (count > 1)
			{
				if (write_at_end)
				{
					C(end) = {to, arg1, count};
					remove(ctx, start, end - 1);
				}
				else
				{
					C(start) = {to, arg1, count};
					remove(ctx, start + 1, end);
				}

				j = start;
			}
		}
	});
}

static void optimize_conseq_additive(OptContext& ctx)
{
	add_context_cfg(ctx);
	optimize_conseq_additive_impl(ctx, PUSHR, PUSHARGSR);
	optimize_conseq_additive_impl(ctx, PUSHV, PUSHARGSV);
	optimize_conseq_additive_impl(ctx, PUSHVARGR, PUSHVARGSR);
	optimize_conseq_additive_impl(ctx, PUSHVARGV, PUSHVARGSV);
	optimize_conseq_additive_impl(ctx, POP, POPARGS, true);
}

// SETR, ADDV, LOADI -> LOADD
// Ex:
//   SETR            D6              D4
//   ADDV            D6              40000
//   LOADI           D2              D6
// ->
//   LOADD           D2              40000
static void optimize_loadi(OptContext& ctx)
{
	for (pc_t i = ctx.fn.start_pc + 2; i < ctx.fn.final_pc; i++)
	{
		int command = C(i).command;
		int arg1 = C(i).arg1;
		int arg2 = C(i).arg2;
		if (command != LOADI || arg2 != D(6)) continue;

		int addv_command = C(i - 1).command;
		int addv_arg1 = C(i - 1).arg1;
		int addv_arg2 = C(i - 1).arg2;

		ASSERT(addv_command == ADDV);
		ASSERT(addv_arg1 == D(6));

		int setr_command = C(i - 2).command;
		int setr_arg1 = C(i - 2).arg1;
		int setr_arg2 = C(i - 2).arg2;

		ASSERT(setr_command == SETR);
		ASSERT(setr_arg1 == D(6));
		ASSERT(setr_arg2 == D(4));

		C(i - 2) = {LOADD, arg1, addv_arg2};
		remove(ctx, i - 1, i);
	}
}

// SETV, PUSHR -> PUSHV
// Ex:
//   SETV            D2              5420000
//   PUSHR           D2
// ->
//   PUSHV           D2              5420000
static void optimize_setv_pushr(OptContext& ctx)
{
	add_context_cfg(ctx);
	optimize_by_block(ctx, [&](pc_t block_index, pc_t start_pc, pc_t final_pc){
		for (int j = start_pc; j < final_pc; j++)
		{
			if (C(j).command != SETV) continue;
			if (C(j + 1).command != PUSHR) continue;
			if (C(j).arg1 != C(j + 1).arg1) continue;

			// `zasm_construct_structured` is sensitive to a PUSH being just before
			// a function call, so unlike other places assign the NOP to the first
			// instruction.
			C(j + 1) = {PUSHV, C(j).arg2};
			remove(ctx, j);
		}
	});
}

static pc_t get_block_final(const OptContext& ctx, int block)
{
	return block == ctx.block_starts.size() - 1 ?
		ctx.fn.final_pc :
		ctx.block_starts.at(block + 1) - 1;
}

static std::pair<pc_t, pc_t> get_block_bounds(const OptContext& ctx, int block)
{
	return {ctx.block_starts.at(block), get_block_final(ctx, block)};
}

struct SimulationState
{
	pc_t block = 0;
	pc_t pc = 0;
	pc_t final_pc = 0;
	SimulationValue d[8];
	SimulationValue operand_1 = {ValueType::Uninitialized};
	int operand_1_backing_reg = -1;
	SimulationValue operand_2 = {ValueType::Uninitialized};
	int operand_2_backing_reg = -1;
	bool side_effects = false;
	bool bail = false;

	SimulationState()
	{
		for (int i = 0; i < 8; i++)
			d[i] = {ValueType::Register, D(i)};
	}

	void set_block(const OptContext& ctx, pc_t target_block)
	{
		block = target_block;
		std::tie(pc, final_pc) = get_block_bounds(ctx, target_block);
	}
};

static SimulationValue evaluate_binary_op(int cmp, SimulationValue a, SimulationValue b)
{
	ASSERT(cmp <= (CMP_FLAGS|CMP_BOOL));
	int flags = cmp & CMP_FLAGS;
	bool seti = cmp & CMP_SETI;
	bool boolcast = cmp & CMP_BOOL;

	if (cmp == 0)
		return num_zero;
	if (flags == CMP_FLAGS)
		return seti ? num(10000) : num_one;

	if (a.type == ValueType::Uninitialized || b.type == ValueType::Uninitialized)
		return {ValueType::Uninitialized};
	if (a.type == ValueType::Unknown || b.type == ValueType::Unknown)
		return {ValueType::Unknown};

	if (a.is_number() && b.is_number())
	{
		if (boolcast)
		{
			ASSERT(flags == CMP_EQ || flags == CMP_NE);
			bool v = !!a.data == !!b.data;
			if (flags == CMP_NE) v = !v;
			return num(v ? 1 : 0);
		}

		bool b_val = false;
		if (flags & CMP_EQ) b_val |= a.data == b.data;
		if (flags & CMP_LT) b_val |= a.data < b.data;
		if (flags & CMP_GT) b_val |= a.data > b.data;

		int v = b_val ? 1 : 0;
		if (seti) v *= 10000;
		return num(v);
	}

	bool should_swap = false;
	if (a.type != ValueType::Expression && b.type == ValueType::Expression)
		should_swap = true;
	if (a.is_number())
		should_swap = true;
	if (should_swap)
	{
		std::swap(a, b);

		// Flip the comparison.
		bool lt = flags & CMP_LT;
		bool gt = flags & CMP_GT;
		bool eq = flags & CMP_EQ;
		std::swap(lt, gt);
		cmp = 0;
		cmp |= (lt ? CMP_LT : 0);
		cmp |= (gt ? CMP_GT : 0);
		cmp |= (eq ? CMP_EQ : 0);
		cmp |= (seti ? CMP_SETI : 0);
		cmp |= (boolcast ? CMP_BOOL : 0);
		flags = cmp & CMP_FLAGS;
	}

	int true_as_int = (a.data & CMP_SETI) ? 10000 : 1;
	if (!seti)
	{
		if (a.type == ValueType::Expression) a.data = a.data & ~CMP_SETI;
		if (b.type == ValueType::Expression) b.data = b.data & ~CMP_SETI;
	}

	if (flags == CMP_EQ && a.type == ValueType::Expression && b == num_one)
		return a;
	if (flags == CMP_EQ && a.type == ValueType::Expression && b == num_zero)
	{
		if (!seti)
			a.data = a.data & ~CMP_SETI;
		return a.negate();
	}

	if (flags == CMP_EQ && ((a.is_number() && a.data == 0) || (b.is_number() && b.data == 0)))
	{
		const auto& not_int = a.is_number() ? b : a;
		if (not_int.type == ValueType::Expression)
			return not_int.negate();
	}

	if (flags == CMP_NE && ((a.is_number() && a.data) || (b.is_number() && b.data)))
	{
		const auto& not_int = a.is_number() ? b : a;
		if (not_int.type == ValueType::Expression)
			return not_int.negate();
	}

	if (flags == CMP_NE && ((a.is_number() && a.data == 0) || (b.is_number() && b.data == 0)))
	{
		const auto& not_int = a.is_number() ? b : a;
		if (not_int.type == ValueType::Expression)
			return not_int;
		return expr(not_int, CMP_NE, num(0));
	}

	if (a.type == ValueType::Register && b.is_number())
		return expr(a, cmp, b);

	if (a.type == b.type && ((flags & CMP_EQ) || flags == CMP_NE) && a == b)
	{
		if ((flags & CMP_EQ))
			return seti ? num(10000) : num_one;
		return num_zero;
	}

	if (!(flags & CMP_EQ) && a == b)
		return num_zero;

	if (a.type == ValueType::Register && b.type == ValueType::Register)
	{
		auto result = SimulationValue{ValueType::Expression, cmp};
		result.op1 = std::make_shared<SimulationValue>(a);
		result.op2 = std::make_shared<SimulationValue>(b);
		return result;
	}

	if (a.type == ValueType::Expression && b.type == ValueType::Expression)
	{
		if (flags == CMP_EQ && a.is_mutually_exclusive(b))
			return num_zero;

		auto result = SimulationValue{ValueType::Expression, cmp};
		result.op1 = std::make_shared<SimulationValue>(a);
		result.op2 = std::make_shared<SimulationValue>(b);
		return result;
	}

	if (a.type == ValueType::Expression && b.is_number())
	{
		int b_val = boolcast ? (b.data ? 1 : 0) : b.data;

		// Check the integer value of the expression against b.
		bool could_gt = true_as_int > b_val || 0 > b_val;
		bool could_lt = true_as_int < b_val || 0 < b_val;
		bool could_eq = true_as_int == b_val || 0 == b_val;

		if (flags == CMP_EQ && !could_eq)
			return num_zero;
		if (flags == CMP_GT && !could_gt)
			return num_zero;
		if (flags == CMP_LT && !could_lt)
			return num_zero;

		bool must_gt = 0 > b_val;
		bool must_lt = true_as_int < b_val;

		if ((flags & CMP_GT) && must_gt)
			return num_one;
		if ((flags & CMP_LT) && must_lt)
			return num_one;

		bool must_ge = 0 >= b_val;
		bool must_le = true_as_int <= b_val;

		if (flags == CMP_GE && must_ge)
			return num_one;
		if (flags == CMP_LE && must_le)
			return num_one;

		return a;
	}

	return {ValueType::Unknown};
}

static void infer_assign(OptContext& ctx, SimulationState& state, int reg, const SimulationValue& rhs)
{
	// If this is <reg == any>, just set the register to op2.
	if (rhs.type == ValueType::Expression && rhs.data == CMP_EQ)
	{
		bool just_one_register = (rhs.op1->type == ValueType::Register) ^ (rhs.op2->type == ValueType::Register);
		ASSERT(just_one_register);
		if (just_one_register)
		{
			if (rhs.op1->type == ValueType::Register && rhs.op1->data == reg)
			{
				state.d[reg] = *rhs.op2;
				return;
			}
			if (rhs.op2->type == ValueType::Register && rhs.op2->data == reg)
			{
				state.d[reg] = *rhs.op1;
				return;
			}
		}
	}

	state.d[reg] = rhs;
}

static void infer(SimulationValue& v, const SimulationValue& given)
{
	if (given.is_equality_expression() && *given.op1 == v)
		v = *given.op2;
	else if (given.is_mutually_exclusive(v))
		v = num_zero;
	else
		v.replace_value(given, num_one);
}

static void infer_values_given_branch(OptContext& ctx, SimulationState& state)
{
	int branch_command = C(state.pc).command;
	int branch_arg2 = C(state.pc).arg2;
	int branch_cmp = command_to_cmp(branch_command, branch_arg2);
	auto expression = evaluate_binary_op(branch_cmp, state.operand_1, state.operand_2);
	ASSERT(expression.is_expression() || expression.is_number());

	// If branching comparison is between a register and a value,
	// assign that register to what it must be to pass the condition.
	if (!(state.operand_1_backing_reg != -1 && state.operand_2_backing_reg != -1))
	{
		if (state.operand_1_backing_reg != -1)
			infer_assign(ctx, state, state.operand_1_backing_reg,
				expr(reg(state.operand_1_backing_reg), branch_cmp, state.operand_2));
		if (state.operand_2_backing_reg != -1)
			infer_assign(ctx, state, state.operand_2_backing_reg,
				expr(state.operand_1, branch_cmp, reg(state.operand_2_backing_reg)));
	}

	if (expression.is_number())
		return;

	for (int i = 0; i < 8; i++)
		infer(state.d[i], expression);
	infer(state.operand_1, expression);
	infer(state.operand_2, expression);
}

// Simulates execution of a single instruction, updating values for each register
// and the comparison result.
static void simulate(OptContext& ctx, SimulationState& state)
{
	int command = C(state.pc).command;
	int arg1 = C(state.pc).arg1;
	int arg2 = C(state.pc).arg2;
	int arg3 = C(state.pc).arg3;
	const auto& c = get_script_command(command);

	// Function calls invalidate all registers.
	bool is_function_call =
		command == CALLFUNC || (command_is_goto(command) && ctx.structured_zasm->function_calls.contains(state.pc));
	if (is_function_call)
	{
		for (int i = 0; i < 8; i++)
			state.d[i] = reg(i);
		return;
	}

	switch (command)
	{
		#define IS_GENERIC_REG(x) (x >= D(0) && x < D(8))
		case COMPARER:
			if (arg1 == SWITCHKEY || arg2 == SWITCHKEY)
			{
				state.bail = true;
				return;
			}
			// Lost Isle is an example of a quest with handwritten ZASM that uses
			// z-registers directly in compare statements, instead of via a generic
			// register.
			if (!IS_GENERIC_REG(arg1) || !IS_GENERIC_REG(arg2))
			{
				state.bail = true;
				return;
			}
			ASSERT(arg1 >= D(0) && arg1 < D(8));
			ASSERT(arg2 >= D(0) && arg2 < D(8));
			state.operand_1 = state.d[arg1];
			state.operand_2 = state.d[arg2];
			state.operand_1_backing_reg = arg1;
			state.operand_2_backing_reg = arg2;
			return;
		case COMPAREV:
			if (arg1 == SWITCHKEY)
			{
				state.bail = true;
				return;
			}
			if (!IS_GENERIC_REG(arg1))
			{
				state.bail = true;
				return;
			}
			state.operand_1 = state.d[arg1];
			state.operand_2 = {ValueType::Number, arg2};
			state.operand_1_backing_reg = arg1;
			state.operand_2_backing_reg = -1;
			return;
		case COMPAREV2:
			if (arg2 == SWITCHKEY)
			{
				state.bail = true;
				return;
			}
			if (!IS_GENERIC_REG(arg2))
			{
				state.bail = true;
				return;
			}
			state.operand_1 = state.d[arg2];
			state.operand_2 = {ValueType::Number, arg1};
			state.operand_1_backing_reg = arg2;
			state.operand_2_backing_reg = -1;
			return;
	}

	if (command_writes_comparison_result(command))
	{
		int cmp = command_to_cmp(command, arg2);
		auto e = evaluate_binary_op(cmp, state.operand_1, state.operand_2);
		state.d[arg1] = e;
		return;
	}

	if (c.args == 0)
	{
		state.side_effects = true;
		return;
	}

	switch (command)
	{
		case CASTBOOLF:
		{
			if (!IS_GENERIC_REG(arg1))
			{
				state.bail = true;
				return;
			}
			state.d[arg1] = evaluate_binary_op(CMP_NE, state.d[arg1], num(0));
			return;
		}

		case CASTBOOLI:
		{
			if (!IS_GENERIC_REG(arg1))
			{
				state.bail = true;
				return;
			}
			state.d[arg1] = evaluate_binary_op(CMP_NE | CMP_SETI, state.d[arg1], num(0));
			return;
		}

		case SETV:
		{
			if (!IS_GENERIC_REG(arg1))
			{
				state.bail = true;
				return;
			}
			state.d[arg1] = num(arg2);
			return;
		}

		case LOADD:
		case LOADI:
		{
			if (arg1 == D(2))
				state.bail = true;
			return;
		}

		case STRCMPR:
		case STRICMPR:
		{
			state.bail = true;
			return;
		}

		case POP:
		{
			if (!(arg1 >= D(0) && arg1 < D(8)))
			{
				state.side_effects = true;
				return;
			}

			state.d[arg1] = {ValueType::Unknown};
			state.side_effects = true;
			return;
		}

		case POPARGS:
		case PUSHARGSR:
		case PUSHR:
		case SETA1:
		case SETA2:
		case STACKWRITEATRV:
		case STACKWRITEATVV_IF:
		case STACKWRITEATVV:
		case STORED:
		case STOREDV:
		case STOREI:
		case TRACER:
		case TRACEV:
		{
			state.side_effects = true;
			return;
		}
	}

	if (c.writes_to_register(0))
	{
		if (!(arg1 >= D(0) && arg1 < D(8)))
		{
			state.side_effects = true;
			return;
		}
		state.d[arg1] = reg(arg1);
	}

	if (c.writes_to_register(1))
	{
		if (!(arg2 >= D(0) && arg2 < D(8)))
		{
			state.side_effects = true;
			return;
		}
		state.d[arg2] = reg(arg2);
	}

	if (c.writes_to_register(2))
	{
		if (!(arg3 >= D(0) && arg3 < D(8)))
		{
			state.side_effects = true;
			return;
		}
		state.d[arg3] = reg(arg3);
	}

	return;
}

static void simulate_and_advance(OptContext& ctx, SimulationState& state)
{
	simulate(ctx, state);
	if (state.pc != state.final_pc)
		state.pc += 1;
}

static void simulate_block(OptContext& ctx, SimulationState& state)
{
	while (true)
	{
		simulate(ctx, state);
		if (ctx.debug)
		{
			fmt::println("{}: {}", state.pc,
				script_debug_command_to_string(C(state.pc).command, C(state.pc).arg1, C(state.pc).arg2, C(state.pc).arg3));
			for (int i = 0; i < 8; i++)
			{
				if (!state.d[i].is_register())
					fmt::println("\tD{} = {}", i, state.d[i].to_string());
			}
			if (state.side_effects) fmt::println("\tside_effects");
			if (state.bail) fmt::println("\tbail");
		}
		if (state.pc == state.final_pc)
			break;

		state.pc += 1;
	}
}

// Take the branch at state.pc, and set simulation values accordingly.
static void simulate_infer_branch(OptContext& ctx, SimulationState& state)
{
	// Given we take the branch, infer what the other values must be.
	infer_values_given_branch(ctx, state);
	if (ctx.debug)
		fmt::println("inferred D2: {}", state.d[2].to_string());
	state.pc = C(state.pc).arg1;
	state.block = ctx.cfg.start_pc_to_block_id.at(state.pc);
	state.final_pc = get_block_final(ctx, state.block);
}

static bool simulate_block_advance(OptContext& ctx, SimulationState& state)
{
	if (!command_is_goto(C(state.pc).command))
	{
		if (E(state.block).size() == 0)
			return false;

		pc_t next_block = ctx.cfg.start_pc_to_block_id.at(state.pc + 1);
		ASSERT(E(state.block).size() == 1);
		ASSERT(E(state.block).at(0) == next_block);
		state.pc += 1;
		state.block = next_block;
		state.final_pc = get_block_final(ctx, state.block);
		return true;
	}

	if (C(state.pc).command == GOTO)
	{
		ASSERT(!ctx.structured_zasm->function_calls.contains(state.pc));
		state.pc = C(state.pc).arg1;
		pc_t next_block = ctx.cfg.start_pc_to_block_id.at(state.pc);
		state.block = next_block;
		state.final_pc = get_block_final(ctx, state.block);
		return true;
	}

	ASSERT(one_of(C(state.pc-1).command, COMPAREV));

	int command = C(state.pc).command;
	ASSERT(command_is_goto(command));
	int arg2 = C(state.pc).arg2;
	int cmp = command_to_cmp(command, arg2);
	auto e = evaluate_binary_op(cmp, state.operand_1, state.operand_2);

	// Can we determine which branch to take? If so, continue.
	if (!e.is_number())
		return false;

	if (e.data)
		state.pc = C(state.pc).arg1;
	else
		state.pc += 1;

	state.block = ctx.cfg.start_pc_to_block_id.at(state.pc);
	state.final_pc = get_block_final(ctx, state.block);
	return true;
}

static SimulationValue compile_value(const SimulationValue& value, std::vector<ffscript>& result)
{
	if (value.is_bool_cast())
	{
		result.emplace_back(CASTBOOLF, value.op1->data, 0);
		return *value.op1;
	}

	ASSERT(value.type != ValueType::Expression);
	return value;
}

// TODO: This is pretty jank. Feels too hardcoded.
static bool compile_conditional(SimulationValue& expression, std::vector<ffscript>& result)
{
	ASSERT(expression.type == ValueType::Expression);

	// Special case for comparison between registers that are first cast to boolean.
	if (expression.op1->is_bool_cast() && expression.op2->is_bool_cast())
	{
		if (expression.op1->op1->is_register() && expression.op2->op1->is_register())
		{
			result.emplace_back(COMPARER, expression.op1->op1->data, expression.op2->op1->data);
			expression.data |= CMP_BOOL;
			return true;
		}
	}

	auto lhs = compile_value(*expression.op1, result);
	auto rhs = compile_value(*expression.op2, result);

	int arg1 = lhs.data;
	int arg2 = rhs.data;
	if (lhs.is_register() && rhs.is_register())
		result.emplace_back(COMPARER, arg1, arg2);
	else if (lhs.is_register() && rhs.is_number())
		result.emplace_back(COMPAREV, arg1, arg2);
	else if (lhs.is_number() && rhs.is_register())
		result.emplace_back(COMPAREV, arg1, arg2);
	else
	{
		ASSERT(false);
		return false;
	}

	return true;
}

static std::vector<ffscript> compile_conditional(const ffscript& instr, const SimulationValue& op1, const SimulationValue& op2)
{
	int cmp = command_to_cmp(instr.command, instr.arg2);
	auto expression = evaluate_binary_op(cmp, op1, op2);

	std::vector<ffscript> result;
	if (!compile_conditional(expression, result))
		return result;

	if (command_is_goto(instr.command))
		result.emplace_back(GOTOCMP, instr.arg1, expression.data);
	else
		result.emplace_back(SETCMP, instr.arg1, expression.data);

	return result;
}

static void for_every_command_arg(const ffscript& instr, std::function<void(bool, bool, int)> fn)
{
	const auto& sc = get_script_command(instr.command);

	if (sc.is_register(0))
	{
		auto [read, write] = get_command_rw(instr.command, 0);
		fn(read, write, instr.arg1);
	}

	if (sc.is_register(1))
	{
		auto [read, write] = get_command_rw(instr.command, 1);
		fn(read, write, instr.arg2);
	}

	if (sc.is_register(2))
	{
		auto [read, write] = get_command_rw(instr.command, 2);
		fn(read, write, instr.arg3);
	}
}

// 1. If following a branch is guaranteed to jump to some other block given the initial
//    branch condition, rewrite the branch to jump directly to that end block. This can
//    only be done when there are no side effects. This removes all spurious branches.
// 2. Convert GOTOX to GOTOCMP
static void optimize_spurious_branches(OptContext& ctx)
{
	add_context_cfg(ctx);
	optimize_by_block(ctx, [&](pc_t block_index, pc_t start_pc, pc_t final_pc){
		// Only consider blocks with a conditional branch.
		// TODO: experiment with checking conditionless branches too.
		const auto& edges = ctx.cfg.block_edges.at(block_index);
		if (edges.size() != 2)
			return;

		int command = C(final_pc).command;
		if (!command_is_goto(command))
		{
			ASSERT(false);
			return;
		}

		if (command == GOTO && ctx.structured_zasm->function_calls.contains(final_pc))
		{
			ASSERT(false);
			return;
		}

		// If the first command is SETCMP, then some other block jumps to this
		// and reuses the comparison state. As that will be undefined, quit early.
		if (command_writes_comparison_result(C(start_pc).command))
			return;

		// COMPARER is left out because it doesn't seem to introduce any optimizations.
		if (!final_pc || !one_of(C(final_pc - 1).command, COMPAREV, COMPAREV2))
		{
			ASSERT(C(final_pc - 1).command == COMPARER);
			return;
		}

		if (ctx.debug)
			fmt::println("\n[spurious_branches] Block #{}\n", block_index);

		SimulationState state{};
		state.block = block_index;
		state.pc = start_pc;
		state.final_pc = final_pc;
		pc_t goto_pc = C(final_pc).arg1;

		// Simulate running the head block.
		simulate_block(ctx, state);

		if (state.bail)
			return;

		if (state.operand_1.type == ValueType::Unknown || state.operand_2.type == ValueType::Unknown)
			return;
		
		// Don't care about side effects in the head block.
		state.side_effects = false;
		simulate_infer_branch(ctx, state);

		while (true)
		{
			simulate_block(ctx, state);
			if (state.bail)
				return;
			if (state.side_effects)
				break;

			bool is_conditionless = E(state.block).size() == 1;
			if (!simulate_block_advance(ctx, state))
				break;

			// If just followed a branch, then save progress so far.
			// It's certain that jumping to this instead of the
			// original pc will not result in any observable difference.
			// Note: This only commits the progress so far when making decisions,
			//       ignoring fall-thrus or unconditional branches. Need better
			//       side effect tracking to handle that. For example case that
			//       could be improved, see "comparisons, random stuff (1)"
			if (!is_conditionless)
			{
				goto_pc = state.pc;
			}
		}

		// If this make the original target block unreachable, the final `unreachable_blocks` pass
		// will remove it.
		if (goto_pc != C(final_pc).arg1)
			ctx.cfg_stale = true;
		C(final_pc) = {GOTOCMP, (int)goto_pc, command_to_cmp(command, C(final_pc).arg2)};
		if (ctx.debug)
			fmt::println("rewrite {}: {}", final_pc, script_debug_command_to_string(C(final_pc)));
	});
}

static void optimize_reduce_comparisons(OptContext& ctx)
{
	add_context_cfg(ctx);
	optimize_by_block(ctx, [&](pc_t block_index, pc_t start_pc, pc_t final_pc){
		bool bail_comp_reduction = false;
		for (int j = start_pc; j < final_pc; j++)
		{
			int command = C(j).command;
			int final_command = C(final_pc).command;
			if (!one_of(command, COMPARER, COMPAREV, COMPAREV2, CASTBOOLF, CASTBOOLI)) continue;
			if (!command_is_goto(final_command)) continue;
			// if (!command_is_goto(final_command) && !one_of(final_command, GOTOR, RETURN, RETURNFUNC)) continue;
			if (final_command == GOTO) continue;

			// TODO support more than just when final command is a GOTO. Ex:
			/*
				11472: COMPARER        D3              D2           
				11473: SETLESS         D2                           
				11474: COMPAREV        D2              0            
				11475: SETTRUEI        D2                           
				11476: PUSHR           D2                           
			*/

			bool writes_comparison_result_to_d2 = false;
			pc_t k = j + 1;
			for (; k < final_pc; k++)
			{
				int command = C(k).command;
				if (!(command_uses_comparison_result(command) || one_of(command, NOP, COMPARER, COMPAREV, COMPAREV2, CASTBOOLF, CASTBOOLI)))
				{
					bail_comp_reduction = true;
					break;
				}

				for_every_command_arg(C(k), [&](bool read, bool write, int arg){
					if (arg == D(2) && write)
					{
						writes_comparison_result_to_d2 = true;
					}
				});
			}

			if (bail_comp_reduction)
				break;

			if (ctx.debug)
				fmt::println("\n[reduce_comparisons] Block #{}\n", block_index);

			std::vector<ffscript> expression_zasm;
			{
				SimulationState state{};
				state.block = block_index;
				state.pc = j;
				state.final_pc = final_pc;
				simulate_block(ctx, state);
				if (state.bail || state.side_effects)
					break;

				expression_zasm = compile_conditional(C(state.pc), state.operand_1, state.operand_2);
				if (expression_zasm.empty())
					break;

				// If not enough room to replace instructions in this block, bail.
				if (expression_zasm.size() > final_pc - start_pc + 1)
				{
					// Should never happen.
					ASSERT(false);
					break;
				}
			}

			pc_t target_pc = C(final_pc).arg1;
			bool target_block_uses_comparison_result = false;
			{
				auto [s, e] = get_block_bounds(ctx, ctx.cfg.start_pc_to_block_id.at(target_pc));
				for (pc_t i = s; i <= e; i++)
				{
					int command = C(i).command;
					if (command_writes_comparison_result(command))
					{
						target_block_uses_comparison_result = true;
						break;
					}
					if (one_of(command, COMPAREV, COMPAREV2, COMPARER))
						break;
				}
			}

			if (target_block_uses_comparison_result)
				return;

			// Determine if D2 is reused after the branch. If so, and the original code
			// sets the comparison result to D2, we need continue setting it (we are removing all but the GOTOCMP).
			// Note: big assumption here: that only the branch might use D2. Not checking if the fall through block
			//       might use D2.

			bool target_block_uses_d2 = false;
			if (writes_comparison_result_to_d2)
			{
				auto [s, e] = get_block_bounds(ctx, ctx.cfg.start_pc_to_block_id.at(target_pc));
				for (pc_t i = s; i <= ctx.fn.final_pc; i++)
				{
					int command = C(i).command;
					int arg1 = C(i).arg1;

					if (command == NOP)
						continue;

					// Functions return their value by setting D2.
					if (one_of(command, RETURN, RETURNFUNC, GOTOR))
					{
						target_block_uses_d2 = true;
						break;
					}

					// Function calls invalidate D2.
					if (command == CALLFUNC)
						break;
					if (command == GOTO && ctx.structured_zasm->start_pc_to_function.contains(arg1))
						break;

					bool writes_d2 = false;
					for_every_command_arg(C(i), [&](bool read, bool write, int arg){
						if (arg == D(2))
						{
							if (read && !writes_d2)
								target_block_uses_d2 = true;
							else if (write)
								writes_d2 = true;
						}
					});

					if (writes_d2 || target_block_uses_d2)
						break;
				}
			}

			if (target_block_uses_d2)
			{
				// TODO: wasm jit backend currently can only handle pairs of a COMPARE with a single SETX/GOTOX.
				if (is_web())
					break;

				int cmp = expression_zasm.back().arg2;
				for (pc_t k = final_pc - 1; k > j; k--)
				{
					// If the last SET for D2 has SETI, need to match that.
					if (command_writes_comparison_result(C(k).command) && C(k).arg1 == D(2))
					{
						if (command_to_cmp(C(k).command, C(k).arg2) & CMP_SETI)
							cmp |= CMP_SETI;
						break;
					}
				}
				if (C(final_pc - 1).command == COMPAREV && C(final_pc - 1).arg1 == D(2) && C(final_pc - 1).arg2 == 0)
					cmp = INVERT_CMP(cmp);
				expression_zasm.insert(expression_zasm.end() - 1, ffscript{SETCMP, D(2), cmp});
			}

			std::copy(expression_zasm.begin(), expression_zasm.end(), &C(j));
			remove(ctx, j + expression_zasm.size(), final_pc);
			ctx.cfg_stale = true;
			if (ctx.debug)
			{
				fmt::println("rewrite {}: {} -> {} commands", j, final_pc - j + 1, expression_zasm.size());
				for (int i = j; i <= final_pc; i++)
					fmt::println("{}: {}", i, script_debug_command_to_string(C(i)));
			}

			// TODO: Will need to be a loop when more than just final command being GOTO is handled.
			break;
		}
	});
}

static void optimize_unreachable_blocks(OptContext& ctx)
{
	add_context_cfg(ctx);

	std::set<pc_t> seen_ids = ctx.block_unreachable;
	std::vector<pc_t> pending_ids = {0};
	while (!pending_ids.empty())
	{
		pc_t id = pending_ids.back();
		pending_ids.pop_back();
		seen_ids.insert(id);

		for (auto calls_id : ctx.cfg.block_edges.at(id))
		{
			if (!seen_ids.contains(calls_id))
				pending_ids.push_back(calls_id);
		}
	}
	for (pc_t i = 0; i < ctx.cfg.block_starts.size(); i++)
	{
		if (!seen_ids.contains((i)))
		{
			auto [block_start, block_final] = get_block_bounds(ctx, i);
			remove(ctx, block_start, block_final);

			// Don't ever remove the end marker.
			if (block_final == ctx.script->size - 1)
			{
				C(block_final) = {0xFFFF};
				ctx.saved -= 1;
			}

			// Prevent needing to rebuild the CFG.
			ctx.block_unreachable.insert(i);
		}
	}
}

// Ideas for more opt passes:
// - Remove unused writes to a D register
static std::vector<std::pair<std::string, std::function<void(OptContext&)>>> passes = {
	{"goto_next_instruction", optimize_goto_next_instruction},
	{"unreachable_blocks", optimize_unreachable_blocks},
	{"conseq_additive", optimize_conseq_additive},
	{"loadi", optimize_loadi},
	{"setv_pushr", optimize_setv_pushr},
	{"spurious_branches", optimize_spurious_branches},
	{"reduce_comparisons", optimize_reduce_comparisons},
	{"unreachable_blocks_2", optimize_unreachable_blocks},
};

static void run_pass(OptimizeResults& results, int i, OptContext& ctx, std::pair<std::string, std::function<void(OptContext&)>> pass)
{
	auto start_time = std::chrono::steady_clock::now();

	auto [name, fn] = pass;
	ctx.saved = 0;
	fn(ctx);

	auto end_time = std::chrono::steady_clock::now();
	results.passes[i].instructions_saved += ctx.saved;
	results.passes[i].elapsed += std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
}

static void optimize_function(OptimizeResults& results, StructuredZasm& structured_zasm, script_data* script, const ZasmFunction& fn)
{
	OptContext ctx = create_context_no_cfg(structured_zasm, script, fn);
	for (int i = 0; i < passes.size(); i++)
	{
		run_pass(results, i, ctx, passes[i]);
	}
}

static OptimizeResults create_opt_results()
{
	OptimizeResults results{};
	for (auto [pass_name, _] : passes)
	{
		results.passes.push_back({pass_name, 0, 0});
	}
	return results;
}

OptimizeResults zasm_optimize(script_data* script)
{
	OptimizeResults results = create_opt_results();

	auto start_time = std::chrono::steady_clock::now();
	auto structured_zasm = zasm_construct_structured(script);

	for (const auto& fn : structured_zasm.functions)
	{
		optimize_function(results, structured_zasm, script, fn);
	}

	// TODO: remove NOPs and update GOTOs.
	auto end_time = std::chrono::steady_clock::now();
	results.elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

	for (const auto& pass : results.passes)
	{
		results.instructions_saved += pass.instructions_saved;
	}

	script->optimized = true;
	return results;
}

void zasm_optimize_and_log(script_data* script)
{
	auto r = zasm_optimize(script);
	double pct = 100.0 * r.instructions_saved / script->size;
	std::string str = fmt::format("[{}] optimized script. saved {} instr ({:.1f}%), took {} ms", zasm_script_unique_name(script), r.instructions_saved, pct, r.elapsed / 1000);
	al_trace("%s\n", str.c_str());
}

OptimizeResults zasm_optimize()
{
	int log_level = get_flag_int("-zasm-optimize-log").value_or(1);

	int size = 0;
	auto start_time = std::chrono::steady_clock::now();
	OptimizeResults results = create_opt_results();
	
	if (log_level >= 1)
		fmt::println("Optimizing scripts...");

	zasm_for_every_script([&](auto script){
		size += script->size;
		
		auto r = zasm_optimize(script);
		double pct = 100.0 * r.instructions_saved / script->size;
		if (log_level >= 2)
			fmt::println("\t[{}] saved {} instr ({:.1f}%), took {} ms", zasm_script_unique_name(script), r.instructions_saved, pct, r.elapsed / 1000);

		for (int i = 0; i < passes.size(); i++)
		{
			results.passes[i].instructions_saved += r.passes[i].instructions_saved;
			results.passes[i].elapsed += r.passes[i].elapsed;
		}
		results.instructions_saved += r.instructions_saved;
	});

	if (size == 0)
	{
		if (log_level >= 1)
			fmt::println("\nNo scripts found.");
		return results;
	}

	auto end_time = std::chrono::steady_clock::now();
	results.elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

	if (log_level >= 1)
	{
		fmt::println("\nFinished optimizing scripts:");

		for (const auto& pass : results.passes)
		{
			double pct = 100.0 * pass.instructions_saved / size;
			fmt::println("\t[{}] saved {} instr ({:.1f}%), took {} ms", pass.name, pass.instructions_saved, pct, pass.elapsed / 1000);
		}

		double pct = 100.0 * results.instructions_saved / size;
		fmt::println("\t[{}] saved {} instr ({:.1f}%), took {} ms\n", "total", results.instructions_saved, pct, results.elapsed / 1000);
	}

	return results;
}

static bool _TEST(std::string& name_var, std::string name)
{
	static auto filter = get_flag_string("-test-filter");
	if (filter.has_value() && name.find(filter.value()) == name.npos)
		return false;

	name_var = name;
	return true;
}

#define TEST(name_val) if (_TEST(name, name_val))

// TODO: Stole from ffasm.cpp
static std::optional<int> check_comparestr(char const* buf)
{
	int cmp = 0;
	if(buf[0] == 'B')
	{
		cmp |= CMP_BOOL;
		++buf;
	}
	if(buf[0] == 'I')
	{
		cmp |= CMP_SETI;
		++buf;
	}
	if(!strcmp(buf,"<"))
		cmp |= CMP_LT;
	else if(!strcmp(buf,">"))
		cmp |= CMP_GT;
	else if(!strcmp(buf,"<="))
		cmp |= CMP_LE;
	else if(!strcmp(buf,">="))
		cmp |= CMP_GE;
	else if(!strcmp(buf,"=="))
		cmp |= CMP_EQ;
	else if(!strcmp(buf,"!="))
		cmp |= CMP_NE;
	else if(!strcmp(buf,"Never"))
		;
	else if(!strcmp(buf,"Always"))
		cmp |= CMP_FLAGS;
	else return nullopt; //nonmatching
	return cmp;
}

// TODO: Stole from ffasm.cpp
static int32_t set_argument(char const* argbuf, int32_t& arg)
{
	extern script_variable variable_list[];
	int32_t i=0;
	char tempvar[80];
	
	while(variable_list[i].id>-1)
	{
		if(variable_list[i].maxcount>1)
		{
			for(int32_t j=0; j<variable_list[i].maxcount; ++j)
			{
#ifndef _MSC_VER
				if (__builtin_strlen(variable_list[i].name) > sizeof(((script_variable*)0)->name))
					__builtin_unreachable();
#endif

				if(strcmp(variable_list[i].name,"A")==0)
					sprintf(tempvar, "%s%d", variable_list[i].name, j+1);
				else sprintf(tempvar, "%s%d", variable_list[i].name, j);
				
				if(stricmp(argbuf,tempvar)==0)
				{
					int32_t temp = variable_list[i].id+(j*zc_max(1,variable_list[i].multiple));
					arg = temp;
					return 1;
				}
			}
		}
		else
		{
			if(stricmp(argbuf,variable_list[i].name)==0)
			{
				arg = variable_list[i].id;
				return 1;
			}
		}
		
		++i;
	}
	
	return 0;
}

// TODO: Stole from ffasm.cpp
static int zasm_arg_from_string(std::string text, int type)
{
	util::trimstr(text);

	switch (type)
	{
		case ARGTY_READ_REG:
		case ARGTY_WRITE_REG:
		case ARGTY_READWRITE_REG:
		{
			int arg = 0;
			set_argument(text.c_str(), arg);
			return arg;
		}
		break;

		case ARGTY_LITERAL:
		{
			return std::stoi(text);
		}

		case ARGTY_COMPARE_OP:
		{
			return check_comparestr(text.c_str()).value();
		}
	}

	return 0;
}

// Simplified version of parsing ZASM.
static script_data zasm_from_string(std::string text)
{
	normalize_whitespace(text);
	script_data script(ScriptType::None, 0);

	std::vector<ffscript> instructions;
	std::vector<std::string> lines;
	util::split(text, lines, '\n');
	for (auto& line : lines)
	{
		util::trimstr(line);
		if (line.starts_with("Function"))
			continue;

		auto tokens = util::split_args(line);
		if (tokens.size() == 0)
			continue;

		const auto& command_name = tokens[1];
		int command = get_script_command(command_name);
		const auto& sc = get_script_command(command);
		
		int arg1 = 0;
		int arg2 = 0;
		int arg3 = 0;
		if (sc.args >= 1)
			arg1 = zasm_arg_from_string(tokens[2], sc.arg_type[0]);
		if (sc.args >= 2)
			arg2 = zasm_arg_from_string(tokens[3], sc.arg_type[1]);
		if (sc.args >= 3)
			arg3 = zasm_arg_from_string(tokens[4], sc.arg_type[2]);

		instructions.emplace_back(command, arg1, arg2, arg3);
	}

	instructions.emplace_back(0xFFFF);
	ffscript* s = new ffscript[instructions.size()];
	std::copy(instructions.begin(), instructions.end(), s);
	script.zasm = s;
	script.recalc_size();

	return script;
}

// Used by test_optimize_zasm_test.py
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

	// Just in case there is a minor roundtrip difference, resave the input.
	{
		std::ofstream out(path, std::ios::binary);
		for (int i = 0; i < last_comment_line; i++)
			out << lines[i] + "\n";
		out << zasm_to_string_clean(&script);
		out.close();
	}

	fmt::println("\noutput:");
	verbose = true;
	zasm_optimize(&script);
	fmt::println("{}", zasm_to_string_clean(&script));
}

bool zasm_optimize_test()
{
	tests_passed = true;
	script_data script(ScriptType::None, 0);
	std::string name;

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
		ffscript s[] = {
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
		script.zasm = s;
		script.recalc_size();

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
			auto r = compile_conditional({GOTOCMP, 0, CMP_EQ},
				reg(2), num_one);
			auto script = script_from_vec(r);
			EXPECT(name, &script, {
				{COMPAREV, D(2), 1},
				{GOTOCMP, 0, CMP_EQ},
				{0xFFFF},
			});
			script.zasm = nullptr;
		}

		{
			auto e = expr(reg(2), CMP_NE, num(0));
			auto r = compile_conditional({GOTOCMP, 0, CMP_EQ},
				e, num(1));
			auto script = script_from_vec(r);
			EXPECT(name, &script, {
				{COMPAREV, D(2), 0},
				{GOTOCMP, 0, CMP_NE},
				{0xFFFF},
			});
			script.zasm = nullptr;
		}

		{
			auto e = expr(reg(2), CMP_GT, num(10));
			auto r = compile_conditional({GOTOCMP, 0, CMP_NE},
				e, num(0));
			auto script = script_from_vec(r);
			EXPECT(name, &script, {
				{COMPAREV, D(2), 10},
				{GOTOCMP, 0, CMP_GT},
				{0xFFFF},
			});
			script.zasm = nullptr;
		}

		{
			auto e = expr(reg(2), CMP_NE, num(0));
			auto r = compile_conditional({GOTOCMP, 0, CMP_NE},
				e, num(0));
			auto script = script_from_vec(r);
			EXPECT(name, &script, {
				{COMPAREV, D(2), 0},
				{GOTOCMP, 0, CMP_NE},
				{0xFFFF},
			});
			script.zasm = nullptr;
		}

		{
			auto r = compile_conditional({GOTOCMP, 0, CMP_NE},
				boolean_cast(reg(2)), boolean_cast(reg(3)));
			auto script = script_from_vec(r);
			EXPECT(name, &script, {
				{COMPARER, D(2), D(3)},
				{GOTOCMP, 0, CMP_NE|CMP_BOOL},
				{0xFFFF},
			});
			script.zasm = nullptr;
		}
	}

	script.zasm = nullptr;
	return tests_passed;
}

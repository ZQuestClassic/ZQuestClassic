// Internal declarations shared between the ZASM optimizer (zasm_optimize.cpp)
// and its unit tests (zasm_optimize_test.cpp). These are implementation details
// of the optimizer that are not part of its public API, but the unit tests need
// to construct and inspect them directly.
#pragma once

#include "zc/zasm_utils.h"
#include "components/zasm/serialize.h"
#include "parser/parserDefs.h"
#include <fmt/format.h>
#include <memory>
#include <set>
#include <string>
#include <vector>

// When enabled, the optimizer prints verbose tracing. Set by zasm_optimize_run_for_file.
extern bool zasm_optimize_verbose;

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
		return type == ValueType::Expression && (data == CMP_EQ);
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
			return zasm_var_to_string(data);
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

	ValueType type = ValueType::Uninitialized;
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

struct OptContext
{
	uint32_t saved;
	zasm_script* script;
	ZasmFunction fn;
	ZasmCFG cfg;
	bool cfg_stale;
	ZasmLiveness liveness_vars;
	std::set<pc_t> block_unreachable;
	StructuredZasm* structured_zasm;
	bool debug;
};

struct SimulationState
{
	pc_t block = 0;
	pc_t pc = 0;
	pc_t final_pc = 0;
	SimulationValue d[8];
	SimulationValue operand_1;
	int operand_1_backing_reg = -1;
	SimulationValue operand_2;
	int operand_2_backing_reg = -1;
	std::vector<SimulationValue> stack;
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
		std::tie(pc, final_pc) = ctx.cfg.get_block_bounds(target_block);
	}
};

SimulationValue evaluate_binary_op(int cmp, SimulationValue a, SimulationValue b);
OptContext create_context(StructuredZasm& structured_zasm, zasm_script* script, const ZasmFunction& fn);
void simulate_and_advance(OptContext& ctx, SimulationState& state);
bool compile_conditional(std::vector<ffscript>& result, const ffscript& instr, const SimulationValue& op1, const SimulationValue& op2);

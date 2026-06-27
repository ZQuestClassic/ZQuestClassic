#include "fmt/base.h"
#include "fmt/ranges.h"
#include "parser/Compiler.h"
#include "test_runner/assert.h"
#include "test_runner/test_runner.h"
#include "parser/config.h"
#include "zalleg/zsys.h"
#include "components/zasm/debug_data.h"
#include "components/zasm/eval.h"
#include <functional>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>

using namespace std::literals::string_literals;

void updateIncludePaths();

static std::unique_ptr<ZScript::ScriptsData> runCompiler(std::string script_path)
{
	bool has_qrs = false;
	ZScript::ScriptParser::initialize(has_qrs);
	updateIncludePaths();
	ZScript::CompileOption::OPT_NO_ERROR_HALT.setDefault(ZScript::OPTION_ON);

	bool metadata = true;
	bool docs = false;
	return ZScript::compile(script_path, metadata, docs);
}

static void TEST(std::string name, TestResults& tr, std::function<bool()> cb)
{
	try
	{
		if (!cb())
			tr.failed++;
	}
	catch (const std::exception& e)
	{
		fmt::println("[{}] error: {}", name, e.what());
		tr.failed++;
	}

	tr.total++;
}

static const DebugScope* resolveFileScope(const DebugData& debugData, std::string fname)
{
	const DebugScope* scope = debugData.resolveFileScope(fname);
	if (!scope)
		throw fmt::format("could not find file scope: {}", fname);

	return scope;
}

static const DebugScope* resolveScope(const DebugData& debugData, std::string identifier, const DebugScope* current_scope)
{
	const DebugScope* scope = debugData.resolveScope(identifier, current_scope);
	if (!scope)
		throw std::runtime_error(
			fmt::format("could not find scope: {} (scope: {})", identifier, debugData.getFullScopeName(current_scope)));
	return scope;
}

static const DebugSymbol* resolveSymbol(const DebugData& debugData, std::string identifier, const DebugScope* current_scope)
{
	const DebugSymbol* symbol = debugData.resolveSymbol(identifier, current_scope);
	if (!symbol)
		throw std::runtime_error(
			fmt::format("could not find symbol: {} (scope: {})", identifier, debugData.getFullScopeName(current_scope)));

	return symbol;
}

static const DebugType* resolveSymbolTypeUnwrap(const DebugData& debugData, std::string identifier, const DebugScope* current_scope)
{
	const DebugSymbol* symbol = debugData.resolveSymbol(identifier, current_scope);
	if (!symbol)
		throw std::runtime_error(
			fmt::format("could not find symbol: {} (scope: {})", identifier, debugData.getFullScopeName(current_scope)));

	auto* type = debugData.getType(symbol->type_id);
	while (type->tag == TYPE_CONST || type->tag == TYPE_ARRAY)
		type = debugData.getType(type->extra);
	return type;
}

static std::string resolveSymbolTypeName(const DebugData& debugData, std::string identifier, const DebugScope* current_scope)
{
	const DebugSymbol* symbol = debugData.resolveSymbol(identifier, current_scope);
	if (!symbol)
		throw std::runtime_error(
			fmt::format("could not find symbol: {} (scope: {})", identifier, debugData.getFullScopeName(current_scope)));

	auto* type = debugData.getType(symbol->type_id);
	return debugData.getTypeName(type);
}

static std::vector<const DebugSymbol*> getAllSymbolsWithin(const DebugData& debugData, const DebugScope* current_scope)
{
	std::vector<const DebugSymbol*> symbols;
	std::vector<const DebugScope*> scopes = {current_scope};
	std::set<const DebugScope*> seen;

	while (!scopes.empty())
	{
		auto* scope = scopes.back();
		scopes.pop_back();
		seen.insert(scope);

		for (auto* symbol : debugData.getChildSymbols(scope))
			symbols.push_back(symbol);

		auto children = debugData.getChildScopes(scope);
		for (auto* child : children)
			if (!seen.contains(child)) scopes.push_back(child);
	}

	return symbols;
}

static std::string symbolsToString(const DebugData& debugData, const std::vector<const DebugSymbol*>& symbols)
{
	std::vector<std::string> parts;

	for (auto* symbol : symbols)
		parts.push_back(debugData.getDebugSymbolName(symbol));

	return fmt::format("{}", fmt::join(parts, ", "));
}

TestResults test_parser([[maybe_unused]] bool verbose)
{
	TestResults tr{};

	int test_zc_arg = zapp_check_switch("-test-zc", {"test_dir"});
	CHECK(test_zc_arg > 0);
	std::string test_dir = zapp_get_arg_string(test_zc_arg + 1);

	auto results = runCompiler(test_dir + "/scripts/playground/auto/scopes.zs");
	if (!results->success)
	{
		tr.failed++;
		return tr;
	}

	TEST("debug data scopes", tr, [&]{
		// Roundtrip.
		auto encoded_debug_data = results->zasmCompilerResult.debugData.encode();
		results->zasmCompilerResult.debugData = DebugData::decode(encoded_debug_data).value();

		auto& debugData = results->zasmCompilerResult.debugData;
		if (debugData.source_files.empty())
			return false;
		if (debugData.scopes.empty())
			return false;

		printf("%s\n", debugData.internalToStringForDebugging().c_str());

		const DebugScope* root_scope = &debugData.scopes[0];
		if (root_scope->tag != TAG_ROOT)
			return false;

		std::string scopes_fname;
		for (const auto& fname : debugData.source_files)
		{
			if (fname.path.ends_with("tests/scripts/playground/auto/scopes.zs"))
			{
				scopes_fname = fname.path;
				break;
			}
		}

		const DebugScope* file_scope = resolveFileScope(debugData, scopes_fname);
		const DebugScope* a_namespace_scope = resolveScope(debugData, "A", file_scope);
		const DebugScope* a_b_namespace_scope = resolveScope(debugData, "A::B", file_scope);
		const DebugScope* d_namespace_scope = resolveScope(debugData, "D", file_scope);
		const DebugScope* a_fn_scope = resolveScope(debugData, "A::A_fn", file_scope);
		const DebugScope* a_b_fn_scope = resolveScope(debugData, "A::B::B_fn", file_scope);
		const DebugScope* a_cl_class_scope = resolveScope(debugData, "A::CL", file_scope);
		const DebugScope* a_trace_fn_scope = resolveScope(debugData, "A::trace", file_scope);
		const DebugScope* a_cl_ctor_fn_scope = resolveScope(debugData, "A::CL::CL", file_scope);
		const DebugScope* a_cl_trace_fn_scope = resolveScope(debugData, "A::CL::trace", file_scope);

		std::set<const DebugScope*> scopes = {file_scope, a_namespace_scope, a_b_namespace_scope, d_namespace_scope, a_fn_scope, a_b_fn_scope, a_cl_class_scope, a_trace_fn_scope, a_cl_ctor_fn_scope, a_cl_trace_fn_scope};
		if (scopes.size() != 10)
			return false;

		if (debugData.resolveScope("unused_fn", file_scope))
			throw "expected unused functions to not be in debug data"s;

		const DebugSymbol* global_var_never_shadowed = resolveSymbol(debugData, "GLOBAL_VAR_NEVER_SHADOWED", root_scope);
		if (&debugData.scopes[global_var_never_shadowed->scope_index] != file_scope)
			return false;

		// Global scopes and symbols are reachable everywhere.
		for (auto* scope : scopes)
		{
			resolveScope(debugData, "Trace", scope); // internal
			resolveScope(debugData, "Above", scope); // std
			resolveSymbol(debugData, "globalThing", scope); // tests/scripts/playground/auto/global_objects.zs

			const DebugSymbol* symbol = resolveSymbol(debugData, "GLOBAL_VAR_NEVER_SHADOWED", scope);
			if (symbol != global_var_never_shadowed)
				return false;
		}

		const DebugSymbol* global_var = resolveSymbol(debugData, "GLOBAL_VAR", root_scope);
		if (&debugData.scopes[global_var->scope_index] != file_scope)
			return false;

		if (resolveSymbol(debugData, "GLOBAL_VAR", file_scope) != global_var)
			return false;

		// Check variable shadowing.
		for (auto* scope : {a_namespace_scope, a_b_namespace_scope, a_fn_scope, a_b_fn_scope})
		{
			const DebugSymbol* symbol = resolveSymbol(debugData, "GLOBAL_VAR", scope);
			if (symbol == global_var)
				return false;
		}

		assertEqual(resolveSymbol(debugData, "GLOBAL_VAR", a_fn_scope)->storage, LOC_STACK);

		auto a_b_fn_child_scopes = debugData.getChildScopes(a_b_fn_scope);
		assertSize(a_b_fn_child_scopes, 1);

		if (resolveSymbol(debugData, "GLOBAL_VAR", a_b_fn_child_scopes[0])->storage != LOC_STACK)
			return false;

		// using namespace.
		const DebugScope* using_namespace_statement_fn_scope = resolveScope(debugData, "using_namespace_statement", file_scope);
		const DebugScope* using_namespace_statement_fn_child_scope = debugData.getChildScopes(using_namespace_statement_fn_scope)[0];
		resolveSymbol(debugData, "A_var", using_namespace_statement_fn_child_scope);
		resolveSymbol(debugData, "D_var", using_namespace_statement_fn_child_scope);
		resolveSymbol(debugData, "D_var", a_cl_class_scope);

		resolveScope(debugData, "loop_fn", file_scope);

		// Symbol and scope with same name can be resolved.
		// Audio (const Audio Audio) and Audio::AdjustMusicVolume (member function of Audio class)
		resolveSymbol(debugData, "Audio", file_scope);
		resolveScope(debugData, "Audio::AdjustMusicVolume", file_scope);

		auto loop_fn_symbols = getAllSymbolsWithin(debugData, resolveScope(debugData, "loop_fn", file_scope));
		assertEqual(symbolsToString(debugData, loop_fn_symbols), "int z @ Stack[0], int y @ Stack[0], int x @ Stack[0], int i @ Stack[0]"s);

		auto do_fn_symbols = getAllSymbolsWithin(debugData, resolveScope(debugData, "do_fn", file_scope));
		assertEqual(symbolsToString(debugData, do_fn_symbols), "int i @ Stack[1], int j @ Stack[0]"s);

		auto range_loop_fn_symbols = getAllSymbolsWithin(debugData, resolveScope(debugData, "range_loop_fn", file_scope));
		assertEqual(symbolsToString(debugData, range_loop_fn_symbols), "int[] arr @ Stack[4], int z @ Stack[3], int __LOOP_ITER @ Stack[3] <HIDDEN>, int[] __LOOP_ARR @ Stack[2] <HIDDEN>, int x @ Stack[1], int y @ Stack[0]"s);

		auto template_fns = debugData.resolveFunctions("template_fn", file_scope);
		assertSize(template_fns, 2);
		assertEqual(symbolsToString(debugData, getAllSymbolsWithin(debugData, template_fns[0])), "char32[] val1 @ Stack[2], int val2 @ Stack[1], char32[][][] double_arr @ Stack[0]"s);
		assertEqual(symbolsToString(debugData, getAllSymbolsWithin(debugData, template_fns[1])), "int val1 @ Stack[2], int val2 @ Stack[1], int[][] double_arr @ Stack[0]"s);

		// types.
		assertEqual(resolveSymbolTypeName(debugData, "CONST_GLOBAL_VAR", file_scope), "const int"s);
		assertEqual(resolveSymbolTypeName(debugData, "GLOBAL_BOOL", file_scope), "bool"s);
		assertEqual(resolveSymbolTypeName(debugData, "A::CL::this_ptr", file_scope), "CL"s);
		assertEqual(resolveSymbolTypeName(debugData, "BITDX_NORMAL", file_scope), "const BlitModeBitflags"s);
		assertEqual(resolveSymbolTypeUnwrap(debugData, "BITDX_NORMAL", file_scope)->tag, TYPE_BITFLAGS);

		// internal global variables.
		resolveSymbol(debugData, "NUM_COMBO_POS", file_scope);

		// aliased variables/functions are marked as hidden.
		assertTrue(!(resolveSymbol(debugData, "eweapon::Power", file_scope)->flags & SYM_FLAG_HIDDEN));
		assertTrue(resolveSymbol(debugData, "eweapon::Damage", file_scope)->flags & SYM_FLAG_HIDDEN);
		assertTrue(resolveSymbol(debugData, "itemdata::Defence", file_scope)->flags & SYM_FLAG_HIDDEN);
		assertTrue(resolveScope(debugData, "TraceFFC", file_scope)->flags & (SCOPE_FLAG_INTERNAL | SCOPE_FLAG_DEPRECATED));
		assertTrue(resolveScope(debugData, "EngineDegtoRad", file_scope)->flags & (SCOPE_FLAG_INTERNAL | SCOPE_FLAG_DEPRECATED));

		// alias function signatures.
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "DegToRad", file_scope)), "int DegToRad(int degrees)"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "DegtoRad", file_scope)), "int DegtoRad(int degrees)"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "EngineDegtoRad", file_scope)), "int EngineDegtoRad(int degrees)"s);

		// enums.
		const DebugScope* enum_scope = resolveScope(debugData, "Direction", file_scope);
		auto enum_symbols = debugData.getChildSymbols(enum_scope);
		assertSize(enum_symbols, 12);
		for (auto* scope : scopes)
		{
			// Named enum.
			resolveSymbol(debugData, "DIR_UP", scope);
			// Anonymous enum.
			resolveSymbol(debugData, "MIDI_NONE", scope);
			// ZScript doesn't actually support scoped access of enum members, but whatever.
			resolveSymbol(debugData, "Direction::DIR_UP", scope);
		}

		// base classes.
		assertEqual(resolveScope(debugData, "sprite", file_scope)->tag, TAG_CLASS);
		assertEqual(resolveScope(debugData, "sprite::OwnObject", file_scope)->tag, TAG_FUNCTION);
		assertEqual(resolveSymbol(debugData, "sprite::X", file_scope)->storage, LOC_REGISTER);
		assertEqual(resolveSymbol(debugData, "sprite::Misc", file_scope)->storage, LOC_REGISTER);
		assertEqual(resolveScope(debugData, "ffc", file_scope)->tag, TAG_CLASS);
		assertEqual(resolveScope(debugData, "ffc::OwnObject", file_scope)->tag, TAG_FUNCTION);
		assertEqual(resolveSymbol(debugData, "ffc::X", file_scope)->storage, LOC_REGISTER);
		assertEqual(resolveSymbol(debugData, "ffc::Misc", file_scope)->storage, LOC_REGISTER);

		// Function signatures.
		assertEqual(debugData.getFunctionSignature(a_b_fn_scope), "void A::B::B_fn()"s);
		assertEqual(debugData.getFunctionSignature(a_cl_trace_fn_scope), "void A::CL::trace()"s);
		assertEqual(debugData.getFunctionSignature(a_cl_ctor_fn_scope), "CL A::CL::CL()"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "has_constant", file_scope)), "void has_constant()"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "Trace", file_scope)), "void Trace(untyped val)"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "printf", file_scope)), "void printf(char32[] format, ...untyped[] values)"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "long_fn", file_scope)), "void long_fn(long a)"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "trace_all", file_scope)), "void trace_all(int a, ...int[] b)"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "trace_all_2", file_scope)), "void trace_all_2(...int[] a)"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "enum_fn", file_scope)), "Enum enum_fn(const Enum e)"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "ArrayCopy", file_scope)), "void ArrayCopy(T[] dest, T[] src)"s);
		assertEqual(debugData.getFunctionSignature(resolveScope(debugData, "Max", file_scope)), "T Max(...T[] values)"s);
		assertEqual(debugData.getFunctionSignature(template_fns[0]), "void template_fn(char32[] val1, int val2)"s);
		assertEqual(debugData.getFunctionSignature(template_fns[1]), "void template_fn(int val1, int val2)"s);
		// Note: parameters with default values are currently are not supported. Ideally they should
		// produce a new TAG_DEFAULT_PARAM scope at the callsite.
		assertEqual(debugData.getFunctionSignature(a_fn_scope), "void A::A_fn(int GLOBAL_VAR)"s);

		auto num_fns = debugData.resolveFunctions("num_fn", file_scope);
		assertSize(num_fns, 2);
		assertEqual(debugData.getFunctionSignature(num_fns[0]), "void num_fn(int a)"s);
		assertEqual(debugData.getFunctionSignature(num_fns[1]), "void num_fn(long a)"s);

		// Inline/internal functions.
		const DebugScope* max_fn_scope = resolveScope(debugData, "Max", file_scope);
		assertGreaterThan(max_fn_scope->start_pc, (pc_t)0);
		assertGreaterThan(max_fn_scope->end_pc, (pc_t)0);

		const DebugScope* websocket_ctor_scope = resolveScope(debugData, "websocket::websocket", file_scope);
		assertGreaterThan(websocket_ctor_scope->start_pc, (pc_t)0);
		assertGreaterThan(websocket_ctor_scope->end_pc, (pc_t)0);

		const DebugScope* audio_adjust_fn_scope = resolveScope(debugData, "Audio::AdjustMusicVolume", file_scope);
		assertGreaterThan(audio_adjust_fn_scope->start_pc, (pc_t)0);
		assertGreaterThan(audio_adjust_fn_scope->end_pc, (pc_t)0);

		// Separator rules: '.' for script/class members, '::' for namespaces/enums.
		// Script-scoped variables require '.':
		resolveSymbol(debugData, "scopes.SCRIPT_SCOPED_GLOBAL", file_scope);
		assertTrue(debugData.resolveSymbol("scopes::SCRIPT_SCOPED_GLOBAL", file_scope) == nullptr);
		// Namespace variables require '::' (not '.'):
		resolveSymbol(debugData, "A::cl", file_scope);
		assertTrue(debugData.resolveSymbol("A.cl", file_scope) == nullptr);
		// Static class methods accept both '.' and '::':
		auto static_class_fns = debugData.resolveFunctions("A::DataBag.StaticClassFunction", file_scope);
		assertSize(static_class_fns, 1);
		assertEqual(debugData.getFunctionSignature(static_class_fns[0]), "int A::DataBag::StaticClassFunction()"s);
		assertSize(debugData.resolveFunctions("A::DataBag::StaticClassFunction", file_scope), 1);

		return true;
	});

	struct MockVM : VMInterface
	{
		pc_t debug_add_int_start_pc;
		pc_t debug_add_int_int_start_pc;
		pc_t debug_add_int_long_start_pc;
		pc_t cl_ctor_start_pc;

		std::map<int32_t, int32_t> stack;
		std::map<int32_t, int32_t> globals;
		std::map<int32_t, int32_t> scriptvars;
		std::map<int32_t, int32_t> registers;
		std::map<int32_t, std::map<int32_t, int32_t>> heap; // ObjPtr -> { Offset -> Val }
		
		int32_t current_this_ptr = 0;
		int32_t next_heap_addr = 1000;

		int32_t readStack(int32_t offset) override
		{
			return stack[offset];
		}

		int32_t readGlobal(int32_t idx) override
		{
			return globals[idx];
		}

		int32_t readScript(int32_t idx) override
		{
			return scriptvars[idx];
		}

		int32_t readRegister(int32_t id) override
		{
			return registers[id];
		}

		std::optional<DebugValue> readObjectMember(DebugValue object, const DebugSymbol* sym) override
		{
			if (heap.find(object.raw_value) == heap.end()) throw std::runtime_error("Segfault: Invalid object pointer");
			return DebugValue{heap[object.raw_value][sym->offset], nullptr};
		}

		std::optional<std::vector<DebugValue>> readArray(DebugValue) override
		{
			return std::nullopt;
		}

		std::optional<DebugValue> readArrayElement(DebugValue, int) override
		{
			return std::nullopt;
		}

		std::optional<int32_t> readArraySize(DebugValue) override
		{
			return std::nullopt;
		}

		std::optional<std::string> readString(int32_t) override
		{
			return "";
		}

		void writeGlobal(int32_t, int32_t) override
		{
		}

		void writeScript(int32_t, int32_t) override
		{
		}

		void writeStack(int32_t, int32_t) override
		{
		}

		void writeRegister(int32_t, int32_t) override
		{
		}

		bool writeObjectMember(DebugValue, const DebugSymbol*, DebugValue) override
		{
			return false;
		}

		bool writeArrayElement(DebugValue, int32_t, DebugValue) override
		{
			return false;
		}

		void decreaseObjectReference(DebugValue, const DebugSymbol*) override
		{
		}

		void increaseObjectReference(DebugValue, const DebugSymbol*) override
		{
		}

		expected<int32_t, std::string> executeSandboxed(pc_t start_pc, [[maybe_unused]] int this_zasm_var, [[maybe_unused]] int this_raw_value, const std::vector<int32_t>& args) override
		{
			if (start_pc == debug_add_int_start_pc)
			{
				// DebugAdd(int a) -> returns a + 10
				return args[0] + (10 * FIXED_ONE);
			}

			if (start_pc == debug_add_int_int_start_pc)
			{
				// DebugAdd(int a, int b) -> returns a + b
				return args[0] + args[1];
			}

			if (start_pc == debug_add_int_long_start_pc)
			{
				// DebugAdd(int a, long b) -> returns a + b + 100L
				return args[0] + args[1] + 100;
			}

			if (start_pc == cl_ctor_start_pc)
			{
				return next_heap_addr++;
			}

			return 0;
		}

		DebugValue createArray(std::vector<int32_t>, const DebugType*) override
		{
			return {};
		}

		DebugValue createString(const std::string&) override
		{
			return {};
		}

		int32_t getThisPointer() override {
			return current_this_ptr;
		}

		void setGlobal(int idx, int val) { globals[idx] = val; }
		void setStack(int off, int val) { stack[off] = val; }
		void setObjectMember(int ptr, int off, int val) { heap[ptr][off] = val; }
	};

	TEST("expression evaluator logical", tr, [&]{
		auto& debugData = results->zasmCompilerResult.debugData;
		const DebugScope* root_scope = &debugData.scopes[0];
		MockVM vm{};

		auto eval = [&](const std::string& expr) -> DebugValue {
			ExpressionParser parser(expr);
			auto node = parser.parseExpression().value();
			ExpressionEvaluator eval(debugData, root_scope, vm);
			return eval.evaluate(node);
		};

		// Basic Logical AND (&&)
		{
			// Fixed Point (1.0 && 1.0 -> 1.0)
			assertEqual(eval("1 && 1").raw_value, FIXED_ONE);
			assertEqual(eval("1 && 0").raw_value, 0);
			assertEqual(eval("0 && 1").raw_value, 0);
			assertEqual(eval("0 && 0").raw_value, 0);

			DebugValue v = eval("1L && 1L");
			assertEqual(v.raw_value, FIXED_ONE);
			assertTrue(v.type->isBool(debugData));

			v = eval("1 && 1L");
			assertEqual(v.raw_value, FIXED_ONE);
			assertTrue(v.type->isBool(debugData));
		}

		// Basic Logical OR (||)
		{
			assertEqual(eval("1 || 0").raw_value, FIXED_ONE);
			assertEqual(eval("0 || 1").raw_value, FIXED_ONE);
			assertEqual(eval("0 || 0").raw_value, 0);
			assertEqual(eval("1 || 1").raw_value, FIXED_ONE);
		}

		// Precedence (|| < &&)
		{
			// 1 || 0 && 0
			assertEqual(eval("1 || 0 && 0").raw_value, FIXED_ONE);

			// 0 && 0 || 1
			assertEqual(eval("0 && 0 || 1").raw_value, FIXED_ONE);
		}

		// Comparisons
		{
			// Equality.
			assertEqual(eval("1 == 1").raw_value, FIXED_ONE); // 1.0 == 1.0
			assertEqual(eval("1 != 2").raw_value, FIXED_ONE);
			assertEqual(eval("1 == 2").raw_value, 0);

			// Relational.
			assertEqual(eval("1 < 2").raw_value, FIXED_ONE);
			assertEqual(eval("2 > 1").raw_value, FIXED_ONE);
			assertEqual(eval("1 >= 1").raw_value, FIXED_ONE);
			assertEqual(eval("1 <= 0").raw_value, 0);

			// Mixed Types (Fixed vs Long).
			// Comparison should NOT promote before comparing.
			DebugValue v = eval("1 == 1L");
			assertEqual(v.raw_value, 0);
			assertTrue(v.type->isBool(debugData));

			// 0.5 < 1L
			assertEqual(eval("(1/2) < 1L").raw_value, 0);

			// 0.5 > 1L
			assertEqual(eval("(1/2) > 1L").raw_value, FIXED_ONE);

			// 1 < 2 == 1 -> (1 < 2) == 1 -> 1 == 1 -> True
			assertEqual(eval("1 < 2 == 1").raw_value, FIXED_ONE);

			// 1 + 1 > 1 -> 2 > 1 -> True
			assertEqual(eval("1 + 1 > 1").raw_value, FIXED_ONE);

			// 5 == 4 + 1 -> 5 == 5 -> True
			assertEqual(eval("5 == 4 + 1").raw_value, FIXED_ONE);
		}

		// Precedence (Comparison vs Logical).
		{
			// Equality (==) has higher precedence than Logical AND (&&).
			assertEqual(eval("0 == 1 && 0").raw_value, 0);

			// .Equality (==) has higher precedence than Logical OR (||)
			assertEqual(eval("0 == 1 || 1").raw_value, FIXED_ONE);

			// Relational (<) has higher precedence than Logical AND (&&).
			assertEqual(eval("2 < 10 && 1").raw_value, FIXED_ONE);

			// Unary (!) has higher precedence than Relational (<).
			assertEqual(eval("!0 < 2").raw_value, FIXED_ONE);

			// Complex chaining.
			assertEqual(eval("1 == 1 || 1 == 0").raw_value, FIXED_ONE);
		}

		// Short-circuiting.
		{
			// Division by zero throws in evalBinaryOp.
			// 0 && (1/0) -> Left is false, Right (Error) is never evaluated.
			try {
				DebugValue res = eval("0 && (10 / 0)");
				assertEqual(res.raw_value, 0);
			} catch (const std::runtime_error& e) {
				tr.failed++;
				fmt::println("Short-circuit AND failed: Evaluated right side (DivByZero)");
			}

			// 1 || (1/0) -> Left is true, Right (Error) is never evaluated.
			try {
				DebugValue res = eval("1 || (10 / 0)");
				assertEqual(res.raw_value, FIXED_ONE);
			} catch (const std::runtime_error& e) {
				tr.failed++;
				fmt::println("Short-circuit OR failed: Evaluated right side (DivByZero)");
			}
		}

		// Complex Grouping
		{
			// (1 || 0) && 0 -> 1 && 0 -> 0
			assertEqual(eval("(1 || 0) && 0").raw_value, 0);

			// !1 || 1 -> 0 || 1 -> 1
			// ! binds tighter than ||
			assertEqual(eval("!1 || 1").raw_value, FIXED_ONE);
		}

		return true;
	});

	TEST("expression evaluator", tr, [&]{
		auto& debugData = results->zasmCompilerResult.debugData;
		const DebugScope* root_scope = &debugData.scopes[0];
		MockVM vm{};

		auto eval = [&](const std::string& expr, const DebugScope* scope = nullptr) -> DebugValue {
			if (!scope) scope = &debugData.scopes[0];

			ExpressionParser parser(expr);
			auto node = parser.parseExpression().value();
			ExpressionEvaluator eval(debugData, scope, vm);
			return eval.evaluate(node);
		};

		// Math.
		{
			// Fixed point literals (aka "int").
			auto val = eval("1 + 2");
			assertEqual(val.raw_value, 3 * FIXED_ONE);
			assertTrue(val.type->isFixed(debugData));

			// Long literals (1L = 1).
			val = eval("1L + 2L");
			assertEqual(val.raw_value, 3);
			assertTrue(val.type->isLong(debugData));

			// Mixed math (Fixed + Long = Fixed).
			val = eval("1 + 1L");
			assertEqual(val.raw_value, 10001);
			assertTrue(val.type->isFixed(debugData));

			// Mixed math (Long + Fixed = Fixed) - commutative case must also produce Fixed type.
			val = eval("1L + 1");
			assertEqual(val.raw_value, 10001);
			assertTrue(val.type->isFixed(debugData));

			// Division.
			val = eval("10 / 2");
			assertEqual(val.raw_value, 5 * FIXED_ONE);
			val = eval("10 / 2L");
			assertEqual(val.raw_value, 5 * FIXED_ONE * FIXED_ONE);

			// Multiplication.
			val = eval("10 * 2L");
			assertEqual(val.raw_value, ((10 * FIXED_ONE) * 2) / FIXED_ONE);

			// Long * Fixed must use fixed-point multiply, not plain integer multiply.
			val = eval("2L * 3");
			assertEqual(val.raw_value, (int32_t)(((int64_t)2 * (3 * FIXED_ONE)) / FIXED_ONE));
			assertTrue(val.type->isFixed(debugData));

			val = eval("2 * 3 * 4");
			assertEqual(val.raw_value, 24 * FIXED_ONE);

			val = eval("24 / 2 / 3");
			assertEqual(val.raw_value, 4 * FIXED_ONE);
		}

		// Variables.
		{
			const DebugSymbol* global_var = resolveSymbol(debugData, "GLOBAL_VAR", root_scope);
			vm.setGlobal(global_var->offset, 50 * FIXED_ONE);

			auto val = eval("GLOBAL_VAR");
			assertEqual(val.raw_value, 50 * FIXED_ONE);

			val = eval("GLOBAL_VAR + 10");
			assertEqual(val.raw_value, 60 * FIXED_ONE);
		}

		// Bitflags.
		{
			auto val = eval("BITDX_TRANS | BITDX_PIVOT");
			assertEqual(val.raw_value, 3 * FIXED_ONE);
			assertEqual(debugData.getTypeID(val.type), resolveSymbol(debugData, "BITDX_TRANS", root_scope)->type_id);

			val = eval("(BITDX_TRANS | BITDX_PIVOT) ^ BITDX_TRANS");
			assertEqual(val.raw_value, 2 * FIXED_ONE);
			assertEqual(debugData.getTypeID(val.type), resolveSymbol(debugData, "BITDX_TRANS", root_scope)->type_id);

			// Illegal op (should throw).
			assertThrows(eval("BITDX_TRANS + 1"));
			assertThrows(eval("BITDX_TRANS | UNBLOCK_NORM"));
		}

		// Objects.
		{
			int a_var_idx = resolveSymbol(debugData, "A::CL::A_var", root_scope)->offset;
			const DebugScope* cl_trace_function_scope = resolveScope(debugData, "A::CL::trace", root_scope);

			// Setup an object manually.
			int objPtr = 555;
			vm.current_this_ptr = objPtr;
			vm.setObjectMember(objPtr, a_var_idx, 100 * FIXED_ONE); // A_var = 100

			// Implicit 'this' access.
			auto val = eval("A_var", cl_trace_function_scope);
			assertEqual(val.raw_value, 100 * FIXED_ONE);

			// A_var is not defined in global scope.
			assertThrows(eval("A_var", root_scope));

			// Explicit 'obj->member' access.
			// We need a variable 'hero' that points to objPtr.
			// Let's hack a global variable for this test.
			vm.setGlobal(resolveSymbol(debugData, "A::cl", root_scope)->offset, objPtr);
			val = eval("A::cl->A_var", root_scope);
			assertEqual(val.raw_value, 100 * FIXED_ONE);

			const DebugScope* a_namespace_scope = resolveScope(debugData, "A", root_scope);
			val = eval("cl->A_var", a_namespace_scope);
			assertEqual(val.raw_value, 100 * FIXED_ONE);
			val = eval("A::cl->A_var", a_namespace_scope);
			assertEqual(val.raw_value, 100 * FIXED_ONE);

			// cl is not defined in global scope.
			assertThrows(eval("cl->A_var", root_scope));

			// new object.
			vm.cl_ctor_start_pc = debugData.resolveFunctions("A::CL::CL", root_scope).at(0)->start_pc;
			val = eval("new A::CL()");
			assertNotEqual(val.raw_value, 0);
			// Get type index of CL from the ctor.
			uint32_t cl_type_id = resolveScope(debugData, "A::CL::CL", root_scope)->type_id;
			assertEqual(debugData.getTypeID(val.type), cl_type_id);
		}

		// Functions.
		{
			auto debug_add_fns = debugData.resolveFunctions("DebugAdd", root_scope);
			assertEqual(debugData.getFunctionSignature(debug_add_fns[0]), "int DebugAdd(int a)"s);
			assertEqual(debugData.getFunctionSignature(debug_add_fns[1]), "int DebugAdd(int a, int b)"s);
			assertEqual(debugData.getFunctionSignature(debug_add_fns[2]), "int DebugAdd(int a, long b)"s);
			vm.debug_add_int_start_pc = debug_add_fns[0]->start_pc;
			vm.debug_add_int_int_start_pc = debug_add_fns[1]->start_pc;
			vm.debug_add_int_long_start_pc = debug_add_fns[2]->start_pc;

			auto val = eval("DebugAdd(1)"); // a + 10
			assertEqual(val.raw_value, 11 * FIXED_ONE);

			// There is no DebugAdd(long a), but type coercion kicks in.
			val = eval("DebugAdd(1L)"); // a + 10
			assertEqual(val.raw_value, 1 + 10 * FIXED_ONE);

			val = eval("DebugAdd(1, 2)");
			assertEqual(val.raw_value, 3 * FIXED_ONE);

			val = eval("DebugAdd(1, 2L)");  // a + b + 100L
			assertEqual(val.raw_value, 1 * FIXED_ONE + 2 + 100);

			val = eval("DebugAdd(1, 2) + 3");
			assertEqual(val.raw_value, 6 * FIXED_ONE);

			// Too many options, so should throw.
			assertThrows(eval("DebugAdd(0L, 0L)"));

			// A::cl is not const, but the param of DebugPrintCL is. Should get promoted.
			eval("DebugPrintCL(A::cl)");
			// untyped should be coerced to other types.
			eval("DebugPrintCL(NULL)");
			eval("long_fn(NULL)");
		}

		return true;
	});

	return tr;
}

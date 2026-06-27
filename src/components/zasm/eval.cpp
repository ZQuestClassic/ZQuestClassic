// This is only used for the debugger.

#include "components/zasm/eval.h"

#include "base/expected.h"
#include "components/zasm/debug_data.h"
#include <cctype>
#include <optional>
#include <stdexcept>
#include <fmt/ranges.h>
#include <string>

ExpressionParser::ExpressionParser(std::string s) : input(s)
{
}

char ExpressionParser::peek()
{
	return pos < input.size() ? input[pos] : 0;
}

char ExpressionParser::get()
{
	return pos < input.size() ? input[pos++] : 0;
}

void ExpressionParser::skipWhitespace()
{
	while (isspace(peek()))
		pos++;
}

bool ExpressionParser::match(char c)
{
	skipWhitespace();
	if (peek() == c)
	{
		pos++;
		return true;
	}
	return false;
}

expected<std::shared_ptr<ExprNode>, std::string> ExpressionParser::parseExpression()
{
	try {
		return parseAssignment();
	} catch (const std::exception& e) {
		return make_unexpected(e.what());
	}
}

std::shared_ptr<ExprNode> ExpressionParser::parseExpressionImpl()
{
	try {
		return parseAssignment();
	} catch (const std::exception& e) {
		return nullptr;
	}
}

std::shared_ptr<ExprNode> ExpressionParser::parseAssignment()
{
	auto left = parseLogicalOr();
	skipWhitespace();

	// Check for '=' (but not '==').
	if (peek() == '=' && (pos + 1 >= input.size() || input[pos + 1] != '='))
	{
		get(); // Consume '='
		auto right = parseAssignment();
		left = std::make_shared<AssignmentNode>(left, right);
	}

	return left;
}

std::shared_ptr<ExprNode> ExpressionParser::parseLogicalOr()
{
	auto left = parseLogicalAnd();
	skipWhitespace();
	while (peek() == '|' && pos + 1 < input.size() && input[pos + 1] == '|')
	{
		pos += 2; // Consume ||
		auto right = parseLogicalAnd();
		left = std::make_shared<BinaryOpNode>("||", left, right);
	}
	return left;
}

std::shared_ptr<ExprNode> ExpressionParser::parseLogicalAnd()
{
	auto left = parseBitOr();
	skipWhitespace();
	while (peek() == '&' && pos + 1 < input.size() && input[pos + 1] == '&')
	{
		pos += 2; // Consume &&
		auto right = parseBitOr();
		left = std::make_shared<BinaryOpNode>("&&", left, right);
	}
	return left;
}

std::shared_ptr<ExprNode> ExpressionParser::parseBitOr()
{
	auto left = parseBitXor();
	skipWhitespace();
	while (peek() == '|')
	{
		// If next char is |, it's a Logical OR, let the parent handle it
		if (pos + 1 < input.size() && input[pos + 1] == '|')
			break;

		std::string op(1, get());
		auto right = parseBitXor();
		left = std::make_shared<BinaryOpNode>(op, left, right);
	}
	return left;
}

std::shared_ptr<ExprNode> ExpressionParser::parseBitXor()
{
	auto left = parseBitAnd();
	skipWhitespace();
	while (peek() == '^')
	{
		std::string op(1, get());
		auto right = parseBitAnd();
		left = std::make_shared<BinaryOpNode>(op, left, right);
	}
	return left;
}

std::shared_ptr<ExprNode> ExpressionParser::parseBitAnd()
{
	auto left = parseEquality();
	skipWhitespace();
	while (peek() == '&')
	{
		// If next char is &, it's a Logical AND, let the parent handle it
		if (pos + 1 < input.size() && input[pos + 1] == '&')
			break;

		std::string op(1, get());
		auto right = parseEquality();
		left = std::make_shared<BinaryOpNode>(op, left, right);
	}
	return left;
}

std::shared_ptr<ExprNode> ExpressionParser::parseEquality()
{
	auto left = parseRelational();
	skipWhitespace();
	while (peek() == '=' || peek() == '!')
	{
		// Check for '==' or '!='
		if (peek() == '=' && pos + 1 < input.size() && input[pos + 1] == '=')
		{
			pos += 2;
			auto right = parseRelational();
			left = std::make_shared<BinaryOpNode>("==", left, right);
		}
		else if (peek() == '!' && pos + 1 < input.size() && input[pos + 1] == '=')
		{
			pos += 2;
			auto right = parseRelational();
			left = std::make_shared<BinaryOpNode>("!=", left, right);
		}
		else
		{
			break;
		}
	}
	return left;
}

std::shared_ptr<ExprNode> ExpressionParser::parseRelational()
{
	auto left = parseAdd();
	skipWhitespace();
	while (peek() == '<' || peek() == '>')
	{
		char first = get(); // Consume < or >
		std::string op(1, first);

		// Check for = (<= or >=)
		if (peek() == '=')
		{
			get();
			op += '=';
		}

		auto right = parseAdd();
		left = std::make_shared<BinaryOpNode>(op, left, right);
	}
	return left;
}

std::shared_ptr<ExprNode> ExpressionParser::parseAdd()
{
	auto left = parseFactor();
	skipWhitespace();
	while (peek() == '+' || peek() == '-')
	{
		std::string op(1, get());
		auto right = parseFactor();
		left = std::make_shared<BinaryOpNode>(op, left, right);
	}
	return left;
}

std::shared_ptr<ExprNode> ExpressionParser::parseFactor()
{
	auto left = parseUnary();
	skipWhitespace();
	while (peek() == '*' || peek() == '/')
	{
		std::string op(1, get());
		auto right = parseUnary();
		left = std::make_shared<BinaryOpNode>(op, left, right);
	}
	return left;
}

std::shared_ptr<ExprNode> ExpressionParser::parseUnary()
{
	skipWhitespace();
	char c = peek();
	
	if (c == '~' || c == '-' || c == '!') 
	{
		std::string op(1, get()); // Consume operator
		auto right = parseUnary(); // Recursive (allows ~~x)
		return std::make_shared<UnaryOpNode>(op, right);
	}
	
	return parsePrimary();
}

std::shared_ptr<ExprNode> ExpressionParser::parsePrimary()
{
	skipWhitespace();
	std::shared_ptr<ExprNode> left;
	char c = peek();

	if (match('"'))
	{
		std::string s;
		while (peek() != '"' && peek() != 0)
		{
			char nextChar = get();
			
			// Handle Escape Sequences
			if (nextChar == '\\')
			{
				if (peek() == 0) throw std::runtime_error("Unexpected end of string");
				
				char escaped = get();
				switch (escaped)
				{
					case 'n': s += '\n'; break;
					case 'r': s += '\r'; break;
					case 't': s += '\t'; break;
					case '\\': s += '\\'; break;
					case '"': s += '"'; break;
					case '0': s += '\0'; break;
					default: 
						// Unknown escape, just keep the character
						s += escaped; 
						break;
				}
			}
			else
			{
				s += nextChar;
			}
		}

		if (!match('"'))
			throw std::runtime_error("Unterminated string literal");

		left = std::make_shared<StringLiteralNode>(s);
	}
	else if (isdigit(c))
	{
		size_t start = pos;
		while (isdigit(peek()))
			pos++;

		bool is_long = false;
		if (peek() == 'L')
		{
			is_long = true;
			pos++;
		}

		std::string numStr = input.substr(start, pos - start - (is_long ? 1 : 0));
		int32_t val = std::stoi(numStr);

		DebugValue dv;
		if (is_long)
		{
			dv.raw_value = val;
			dv.type = &BasicTypes[TYPE_LONG];
		}
		else
		{
			dv.raw_value = val * FIXED_ONE;
			dv.type = &BasicTypes[TYPE_INT];
		}
		left = std::make_shared<LiteralNode>(dv);
	}
	else if (match('('))
	{
		left = parseExpressionImpl();
		if (!match(')'))
			throw std::runtime_error("Expected ')'");
	}
	else if (isalpha(c) || c == '_')
	{
		size_t start = pos;
		while (isalnum(peek()) || peek() == '_' || peek() == ':' || peek() == '.')
			pos++;
		std::string id = input.substr(start, pos - start);

		if (id == "new")
		{
			skipWhitespace();
			size_t c_start = pos;
			while (isalnum(peek()) || peek() == '_' || peek() == ':')
				pos++;
			std::string className = input.substr(c_start, pos - c_start);

			if (!match('('))
				throw std::runtime_error("Expected '(' after class name in new expression");

			std::vector<std::shared_ptr<ExprNode>> args;
			if (!match(')'))
			{
				do
				{
					args.push_back(parseExpressionImpl());
				} while (match(','));

				if (!match(')'))
					throw std::runtime_error("Expected ')' after arguments");
			}

			size_t last_component = className.find_last_of(":");
			if (last_component != std::string::npos)
			{
				className += "::" + className.substr(last_component + 1);
			}
			else
			{
				className += "::" + className;
			}

			left = std::make_shared<FuncCallNode>(className, args);
		}
		else if (id == "true")
		{
			DebugValue value{};
			value.raw_value = FIXED_ONE;
			value.type = &BasicTypes[TYPE_BOOL];
			left = std::make_shared<LiteralNode>(value);
		}
		else if (id == "false")
		{
			DebugValue value{};
			value.raw_value = 0;
			value.type = &BasicTypes[TYPE_BOOL];
			left = std::make_shared<LiteralNode>(value);
		}
		else
		{
			left = std::make_shared<VarNode>(id);
		}
	}
	else
	{
		throw std::runtime_error("Unexpected character");
	}

	// Postfix loop (handle -> and () and [])
	while (true)
	{
		skipWhitespace();

		// Function Call
		if (match('('))
		{
			// Determine function name
			std::string funcName;
			if (auto v = dynamic_cast<VarNode*>(left.get()))
				funcName = v->identifier;
			else if (auto m = dynamic_cast<MemberAccessNode*>(left.get()))
				funcName = m->member_name;
			else
				throw std::runtime_error("Expression is not callable");

			std::vector<std::shared_ptr<ExprNode>> args;
			if (!match(')'))
			{
				do
				{
					args.push_back(parseExpressionImpl());
				} while (match(','));
				if (!match(')'))
					throw std::runtime_error("Expected ')'");
			}

			auto call = std::make_shared<FuncCallNode>(funcName, args);

			// If previous node was MemberAccess, set Context
			if (left->type == E_MEMBER)
			{
				auto memNode = static_cast<MemberAccessNode*>(left.get());
				call->object_context = memNode->object;
			}

			left = call;
		}
		// Member Access
		else if (peek() == '-' && input[pos + 1] == '>')
		{
			pos += 2;
			skipWhitespace();

			size_t start = pos;
			while (isalnum(peek()) || peek() == '_')
				pos++;
			if (pos == start)
				throw std::runtime_error("Expected member name");

			std::string member = input.substr(start, pos - start);
			left = std::make_shared<MemberAccessNode>(left, member);
		}
		// Array indexing
		else if (match('[')) 
		{
			auto indexExpr = parseExpressionImpl();
			if (!match(']'))
				throw std::runtime_error("Expected ']' after array index");
			left = std::make_shared<IndexNode>(left, indexExpr);
		}
		else
		{
			break;
		}
	}

	return left;
}

ExpressionEvaluator::ExpressionEvaluator(const DebugData& dd, const DebugScope* scope, VMInterface& v)
	: debugData(dd), currentScope(scope), vm(v)
{
}

DebugValue ExpressionEvaluator::evaluate(std::shared_ptr<ExprNode> node)
{
	if (!node)
		return {0, &BasicTypes[TYPE_VOID], false};

	switch (node->type)
	{
		case E_LITERAL:
		{
			return static_cast<LiteralNode*>(node.get())->value;
		}

		case E_VAR:
		{
			auto varNode = static_cast<VarNode*>(node.get());
			const DebugSymbol* sym = debugData.resolveSymbol(varNode->identifier, currentScope);
			if (!sym)
				throw std::runtime_error("Unknown variable: " + varNode->identifier);
			return readSymbol(sym);
		}

		case E_MEMBER:
		{
			auto mem = static_cast<MemberAccessNode*>(node.get());
			DebugValue objVal = evaluate(mem->object);

			const DebugScope* cls = getClassScope(objVal.type);
			if (!cls)
				throw std::runtime_error("Accessing member of non-class type");

			const DebugSymbol* sym = debugData.resolveSymbol(mem->member_name, cls);
			if (!sym)
				throw std::runtime_error("Member not found: " + mem->member_name);

			auto result = vm.readObjectMember(objVal, sym);
			if (!result)
				throw std::runtime_error("Failed to read member: " + mem->member_name);

			return result.value();
		}

		case E_BINARY:
		{
			auto bin = static_cast<BinaryOpNode*>(node.get());

			// Handle logical ops with short-circuiting.
			if (bin->op == "&&" || bin->op == "||")
			{
				DebugValue l = evaluate(bin->left);
				bool l_true = (l.raw_value != 0);

				// Helper to determine '1' based on Fixed Point rules
				auto getTrueVal = [&](bool isFixed) { return FIXED_ONE; };

				if (bin->op == "&&")
				{
					if (!l_true)
						return {0, &BasicTypes[TYPE_BOOL], false}; // Short-circuit false.

					DebugValue r = evaluate(bin->right);
					return { (r.raw_value != 0) ? FIXED_ONE : 0, &BasicTypes[TYPE_BOOL], false };
				}
				else
				{
					if (l_true)
						return { getTrueVal(l.type->isFixed(debugData)), &BasicTypes[TYPE_BOOL], false }; // Short-circuit true.

					DebugValue r = evaluate(bin->right);
					return { (r.raw_value != 0) ? FIXED_ONE : 0, &BasicTypes[TYPE_BOOL], false };
				}
			}

			if (bin->op == "==" || bin->op == "!=" ||
				bin->op == "<" || bin->op == "<=" ||
				bin->op == ">" || bin->op == ">=")
			{
				DebugValue l = evaluate(bin->left);
				DebugValue r = evaluate(bin->right);

				int64_t l_val = l.raw_value;
				int64_t r_val = r.raw_value;

				bool res = false;
				if (bin->op == "==") res = (l_val == r_val);
				else if (bin->op == "!=") res = (l_val != r_val);
				else if (bin->op == "<")  res = (l_val < r_val);
				else if (bin->op == "<=") res = (l_val <= r_val);
				else if (bin->op == ">")  res = (l_val > r_val);
				else if (bin->op == ">=") res = (l_val >= r_val);

				return { res ? FIXED_ONE : 0, &BasicTypes[TYPE_BOOL], false };
			}

			DebugValue l = evaluate(bin->left);
			DebugValue r = evaluate(bin->right);
			return evalBinaryOp(bin->op, l, r);
		}

		case E_UNARY:
		{
			auto u = static_cast<UnaryOpNode*>(node.get());
			DebugValue val = evaluate(u->operand);

			if (u->op == "-")
			{
				val.raw_value = -val.raw_value;
			}
			else if (u->op == "~")
			{
				val.raw_value = ~val.raw_value;
			}
			else if (u->op == "!")
			{
				val.raw_value = (val.raw_value == 0) ? FIXED_ONE : 0;
			}
			return val;
		}

		case E_CALL:
		{
			auto call = static_cast<FuncCallNode*>(node.get());

			// Evaluate args.
			std::vector<DebugValue> argValues;
			std::vector<const DebugType*> argTypes;
			for (auto& arg : call->args)
			{
				DebugValue v = evaluate(arg);
				argValues.push_back(v);
				argTypes.push_back(v.type);
			}

			const DebugScope* searchScope = currentScope;
			int thisRawValue = 0;
			if (call->object_context)
			{
				DebugValue objVal = evaluate(call->object_context);
				searchScope = getClassScope(objVal.type);
				thisRawValue = objVal.raw_value;
			}

			const DebugScope* funcScope = resolveOverload(call->name, argTypes, searchScope);
			if (!funcScope)
				throw std::runtime_error("No matching function: " + call->name);

			auto symbols = debugData.getChildSymbols(funcScope);
			size_t numFixedParams = 0;
			bool hasVarargs = false;
			const DebugSymbol* varArgsSymbol = nullptr;

			for (auto& symbol : symbols)
			{
				if (symbol->flags & SYM_FLAG_HIDDEN)
					continue;

				if (symbol->flags & SYM_FLAG_VARARGS)
				{
					hasVarargs = true;
					varArgsSymbol = symbol;
					continue;
				}

				numFixedParams++;
			}

			// Determine how to set 'this' context (if any).
			int thisZasmVar = 0;
			bool passThisUsingStack = false;
			if (call->object_context && !symbols.empty())
			{
				const DebugSymbol* firstSymbol = symbols.front();
				if (firstSymbol->flags & SYM_FLAG_HIDDEN)
				{
					if (firstSymbol->storage == LOC_REGISTER)
						thisZasmVar = firstSymbol->offset;
					else
						passThisUsingStack = true;
				}
			}

			std::vector<int32_t> rawArgs;
			if (passThisUsingStack)
				rawArgs.push_back(thisRawValue);
			for (int i = 0; i < numFixedParams; i++)
				rawArgs.push_back(argValues[i].raw_value);

			if (hasVarargs)
			{
				std::vector<int32_t> rawVarArgs;
				for (size_t i = numFixedParams; i < argValues.size(); ++i)
					rawVarArgs.push_back(argValues[i].raw_value);

				auto arrayType = debugData.getType(varArgsSymbol->type_id);
				auto array = vm.createArray(rawVarArgs, arrayType);
				rawArgs.push_back(array.raw_value);
			}

			auto result = vm.executeSandboxed(funcScope->start_pc, thisZasmVar, thisRawValue, rawArgs);
			if (!result)
				throw std::runtime_error(result.error());

			DebugValue res{};
			res.type = debugData.getType(funcScope->type_id);
			res.raw_value = res.type->isVoid(debugData) ? 0 : result.value();
			return res;
		}

		case E_INDEX:
		{
			auto idxNode = static_cast<IndexNode*>(node.get());

			DebugValue arrVal = evaluate(idxNode->base);
			arrVal.type = arrVal.type->asNonConst(debugData);

			if (arrVal.type->tag != TYPE_ARRAY)
				throw std::runtime_error("Indexing applied to non-array type");

			DebugValue indexVal = evaluate(idxNode->index);

			// It doesn't matter what type the value is - ZScript always divides the index by 10000.
			int32_t index = indexVal.raw_value / FIXED_ONE;
			auto v = vm.readArrayElement(arrVal, index);
			if (!v)
				throw std::runtime_error("Invalid index for array");

			auto res = *v;
			res.is_lvalue = true;
			return res;
		}

		case E_STRING:
		{
			auto sNode = static_cast<StringLiteralNode*>(node.get());
			return vm.createString(sNode->value);
		}

		case E_ASSIGN:
		{
			auto assign = static_cast<AssignmentNode*>(node.get());
			DebugValue val = evaluate(assign->value);
			assignTo(assign->target, val);
			return val;
		}
	}

	return {0, 0, false};
}

DebugValue ExpressionEvaluator::readSymbol(const DebugSymbol* sym)
{
	if (sym->storage == LOC_CLASS)
	{
		int32_t thisPtr = vm.getThisPointer();
		if (auto v = vm.readObjectMember(DebugValue{thisPtr, nullptr}, sym))
			return v.value();
	}
	
	DebugValue v{};
	v.type = debugData.getType(sym->type_id);
	v.is_lvalue = true;

	if (sym->storage == CONSTANT)
		v.raw_value = sym->offset;
	else if (sym->storage == LOC_GLOBAL)
		v.raw_value = vm.readGlobal(sym->offset);
	else if (sym->storage == LOC_SCRIPT_INSTANCE)
		v.raw_value = vm.readScript(sym->offset);
	else if (sym->storage == LOC_STACK)
		v.raw_value = vm.readStack(sym->offset);
	else if (sym->storage == LOC_REGISTER)
		v.raw_value = vm.readRegister(sym->offset);

	return v;
}

void ExpressionEvaluator::assignTo(std::shared_ptr<ExprNode> target, DebugValue val)
{
	if (target->type == E_VAR)
	{
		auto varNode = static_cast<VarNode*>(target.get());
		const DebugSymbol* sym = debugData.resolveSymbol(varNode->identifier, currentScope);

		if (!sym) throw std::runtime_error("Cannot assign to unknown variable: " + varNode->identifier);

		if (sym->storage == CONSTANT)
			throw std::runtime_error("Cannot assign to constant: " + varNode->identifier);
		if (debugData.getType(sym->type_id)->tag == TYPE_CONST)
			throw std::runtime_error("Cannot assign to const variable: " + varNode->identifier);

		const DebugType* type = debugData.getType(sym->type_id);
		bool holds_reference = type->isArray(debugData) || type->isClass(debugData);
		if (holds_reference)
		{
			DebugValue previous_value = readSymbol(sym);
			vm.decreaseObjectReference(previous_value, sym);
		}
		
		if (sym->storage == LOC_GLOBAL)
			vm.writeGlobal(sym->offset, val.raw_value);
		else if (sym->storage == LOC_SCRIPT_INSTANCE)
			vm.writeScript(sym->offset, val.raw_value);
		else if (sym->storage == LOC_STACK)
			vm.writeStack(sym->offset, val.raw_value);
		else if (sym->storage == LOC_REGISTER)
			vm.writeRegister(sym->offset, val.raw_value);
		else if (sym->storage == LOC_CLASS)
		{
			int32_t thisPtr = vm.getThisPointer();
			DebugValue thisVal{thisPtr, nullptr}; // Type doesn't matter for the raw pointer
			if (!vm.writeObjectMember(thisVal, sym, val))
				throw std::runtime_error("Failed to write to member variable");
		}
		else
			throw std::runtime_error("Variable is not writable (Storage type " + std::to_string(sym->storage) + ")");

		if (holds_reference)
			vm.increaseObjectReference(val, sym);
	}
	else if (target->type == E_MEMBER)
	{
		auto memNode = static_cast<MemberAccessNode*>(target.get());

		DebugValue objVal = evaluate(memNode->object);

		const DebugScope* cls = getClassScope(objVal.type);
		if (!cls) throw std::runtime_error("Assignment to member of non-class type");

		const DebugSymbol* sym = debugData.resolveSymbol(memNode->member_name, cls);
		if (!sym) throw std::runtime_error("Member not found: " + memNode->member_name);

		if (!vm.writeObjectMember(objVal, sym, val))
			throw std::runtime_error("Failed to write to member: " + memNode->member_name);
	}
	else if (target->type == E_INDEX)
	{
		auto idxNode = static_cast<IndexNode*>(target.get());
		DebugValue arrVal = evaluate(idxNode->base);

		const DebugType* baseType = arrVal.type->asNonConst(debugData);
		if (baseType->tag != TYPE_ARRAY)
			throw std::runtime_error("Assignment to index of non-array type");

		DebugValue indexVal = evaluate(idxNode->index);
		if (!vm.writeArrayElement(arrVal, indexVal.raw_value / FIXED_ONE, val))
			throw std::runtime_error("Array assignment failed (Index out of bounds?)");
	}
	else
	{
		throw std::runtime_error("Expression is not an l-value (cannot be assigned to)");
	}
}

DebugValue ExpressionEvaluator::evalBinaryOp(const std::string& op, DebugValue l, DebugValue r)
{
	int32_t l_val = l.raw_value;
	int32_t r_val = r.raw_value;
	const DebugType* lType = l.type->asNonConst(debugData);
	const DebugType* rType = r.type->asNonConst(debugData);

	if (lType->isString(debugData) || rType->isString(debugData))
		throw std::runtime_error("Binary operations on strings not supported.");

	bool resultIsFixed = l.type->isFixed(debugData) || r.type->isFixed(debugData);

	if (lType->tag == TYPE_BITFLAGS)
	{
		if (rType->tag != TYPE_BITFLAGS || lType->extra != rType->extra)
			throw std::runtime_error("Bitflags operation requires matching types.");

		if (op == "|" || op == "&" || op == "^")
		{
			DebugValue res = l;

			auto scope = &debugData.scopes[lType->extra];
			bool fixed = scope->type_id == TYPE_INT;
			int l_val = fixed ? l.raw_value / FIXED_ONE : l.raw_value;
			int r_val = fixed ? r.raw_value / FIXED_ONE : r.raw_value;

			if (op == "|") res.raw_value = l_val | r_val;
			if (op == "&") res.raw_value = l_val & r_val;
			if (op == "^") res.raw_value = l_val ^ r_val;

			if (fixed) res.raw_value *= FIXED_ONE;

			return res;
		}
		throw std::runtime_error("Invalid operation '" + op + "' for Bitflags.");
	}

	DebugValue res{};
	res.type = resultIsFixed ? &BasicTypes[TYPE_INT] : &BasicTypes[TYPE_LONG];

	if (op == "+")
		res.raw_value = l_val + r_val;
	else if (op == "-")
		res.raw_value = l_val - r_val;
	else if (op == "*")
	{
		if (resultIsFixed)
		{
			int64_t bigRes = (int64_t)l_val * (int64_t)r_val;
			res.raw_value = (int32_t)(bigRes / FIXED_ONE);
		}
		else
			res.raw_value = l_val * r_val;
	}
	else if (op == "/")
	{
		if (r_val == 0)
			throw std::runtime_error("Divide by zero");
		if (resultIsFixed)
		{
			int64_t bigL = (int64_t)l_val * FIXED_ONE;
			res.raw_value = (int32_t)(bigL / r_val);
		}
		else
			res.raw_value = l_val / r_val;
	}
	return res;
}

const DebugScope* ExpressionEvaluator::getClassScope(const DebugType* type)
{
	type = type->asNonConst(debugData);
	if (type->tag == TYPE_CLASS)
		return &debugData.scopes[type->extra];
	return nullptr;
}

const DebugScope* ExpressionEvaluator::resolveOverload(const std::string& name, const std::vector<const DebugType*>& argTypes, const DebugScope* scope)
{
	auto candidates = debugData.resolveFunctions(name, scope);

	const DebugScope* bestMatch = nullptr;
	int bestScore = -1;
	bool isAmbiguous = false;
	bool bestIsVararg = false;

	for (const auto* func : candidates)
	{
		std::vector<const DebugType*> paramTypes;
		auto paramSymbols = debugData.getChildSymbols(func);
		bool isVararg = false;

		for (size_t i = 0; i < paramSymbols.size(); ++i)
		{
			auto param = paramSymbols[i];

			// If a function has a 'this' context, it is represented as as hidden variable in the
			// function scope. Ignore them for purposes of matching what function to call.
			if (param->flags & DebugSymbolFlags::SYM_FLAG_HIDDEN)
				continue;

			paramTypes.push_back(debugData.getType(param->type_id));

			// Check if this specific symbol is marked as varargs
			// (Usually this is the last parameter)
			if (param->flags & DebugSymbolFlags::SYM_FLAG_VARARGS)
				isVararg = true;
		}

		size_t numFixed = isVararg ? paramTypes.size() - 1 : paramTypes.size();

		// Must have at least enough args to cover the fixed parameters.
		if (argTypes.size() < numFixed)
			continue;

		// If not vararg, count must be exact.
		if (!isVararg && argTypes.size() != numFixed)
			continue;

		int score = 0;
		bool compatible = true;

		for (size_t i = 0; i < argTypes.size(); i++)
		{
			const DebugType* targetType;

			// Determine if we are matching a fixed param or the vararg tail.
			if (i < numFixed) {
				targetType = paramTypes[i];
			} else {
				// All remaining args match against the vararg element type.
				targetType = paramTypes.back()->asNonConst(debugData);

				// Should always be an array, but check to be sure.
				if (!targetType->isArray(debugData))
				{
					compatible = false;
					break;
				}

				targetType = debugData.getType(targetType->extra);
			}

			if (argTypes[i] == targetType) {
				score += 10;
			} else if (targetType->isUntyped(debugData)) {
				score += 5;
			} else if (debugData.canCoerceTypes(argTypes[i], targetType)) {
				score += 1;
			} else {
				compatible = false;
				break;
			}
		}

		if (compatible)
		{
			if (score > bestScore)
			{
				bestScore = score;
				bestMatch = func;
				bestIsVararg = isVararg;
				isAmbiguous = false;
			}
			else if (score == bestScore)
			{
				// Tie-Breaker: Prefer non-vararg functions over vararg functions
				// e.g. func(int) vs func(int, ...) called with func(1).
				if (bestIsVararg && !isVararg)
				{
					bestMatch = func;
					bestIsVararg = false;
					isAmbiguous = false;
				}
				else if (bestIsVararg == isVararg)
				{
					isAmbiguous = true;
				}
			}
		}
	}

	if (isAmbiguous)
		throw std::runtime_error("Ambiguous function call '" + name + "'. Multiple overloads match.");

	return bestMatch;
}

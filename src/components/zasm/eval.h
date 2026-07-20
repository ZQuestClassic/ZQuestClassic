#ifndef ZASM_EVAL_H_
#define ZASM_EVAL_H_

#include "base/expected.h"
#include "components/zasm/debug_data.h"
#include <optional>
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

struct DebugValue
{
	int32_t raw_value;    // The bits in memory
	const DebugType* type;
	bool is_lvalue;       // Can we assign to this?

	auto operator<=>(const DebugValue&) const = default;
};

// Math Constants
constexpr int32_t FIXED_ONE = 10000;

enum ExprType {
	E_LITERAL,
	E_VAR,
	E_BINARY,
	E_UNARY,
	E_CALL,
	E_MEMBER,
	E_INDEX,
	E_STRING,
	E_ASSIGN,
};

struct ExprNode {
	ExprType type;
	virtual ~ExprNode() = default;
};

struct LiteralNode : ExprNode {
	DebugValue value;
	LiteralNode(DebugValue v) : value(v) { type = E_LITERAL; }
};

struct VarNode : ExprNode {
	std::string identifier;
	VarNode(std::string id) : identifier(id) { type = E_VAR; }
};

struct BinaryOpNode : ExprNode {
	std::string op; // "+", "-", "*", "/"
	std::shared_ptr<ExprNode> left;
	std::shared_ptr<ExprNode> right;
	BinaryOpNode(std::string o, std::shared_ptr<ExprNode> l, std::shared_ptr<ExprNode> r) 
		: op(o), left(l), right(r) { type = E_BINARY; }
};

struct UnaryOpNode : ExprNode {
	std::string op;
	std::shared_ptr<ExprNode> operand;
	UnaryOpNode(std::string o, std::shared_ptr<ExprNode> r) 
		: op(o), operand(r) { type = E_UNARY; }
};

struct MemberAccessNode : ExprNode {
	std::shared_ptr<ExprNode> object;
	std::string member_name;
	MemberAccessNode(std::shared_ptr<ExprNode> obj, std::string name) 
		: object(obj), member_name(name) { type = E_MEMBER; }
};

struct FuncCallNode : ExprNode {
	std::string name;
	std::vector<std::shared_ptr<ExprNode>> args;
	std::shared_ptr<ExprNode> object_context; // Null if global function, set if obj->method()
	
	FuncCallNode(std::string n, std::vector<std::shared_ptr<ExprNode>> a) 
		: name(n), args(a), object_context(nullptr) { type = E_CALL; }
};

struct IndexNode : ExprNode {
	std::shared_ptr<ExprNode> base;
	std::shared_ptr<ExprNode> index;

	IndexNode(std::shared_ptr<ExprNode> b, std::shared_ptr<ExprNode> i)
		: base(b), index(i) { type = E_INDEX; }
};

struct StringLiteralNode : ExprNode {
    std::string value;
    StringLiteralNode(std::string v) : value(std::move(v)) { type = E_STRING; }
};

struct AssignmentNode : ExprNode {
	std::shared_ptr<ExprNode> target;
	std::shared_ptr<ExprNode> value;

	AssignmentNode(std::shared_ptr<ExprNode> t, std::shared_ptr<ExprNode> v)
		: target(t), value(v) { type = E_ASSIGN; }
};

class ExpressionParser
{
	std::string input;
	size_t pos = 0;

	char peek();
	char get();
	void skipWhitespace();
	bool match(char c);

	std::shared_ptr<ExprNode> parseAssignment();
	std::shared_ptr<ExprNode> parseLogicalOr();
	std::shared_ptr<ExprNode> parseLogicalAnd();
	std::shared_ptr<ExprNode> parseBitOr();
	std::shared_ptr<ExprNode> parseBitXor();
	std::shared_ptr<ExprNode> parseBitAnd();
	std::shared_ptr<ExprNode> parseEquality();
	std::shared_ptr<ExprNode> parseRelational();
	std::shared_ptr<ExprNode> parseAdd();
	std::shared_ptr<ExprNode> parseFactor();
	std::shared_ptr<ExprNode> parseUnary();
	std::shared_ptr<ExprNode> parsePrimary();
	std::shared_ptr<ExprNode> parseExpressionImpl();

public:
	ExpressionParser(std::string s);
	expected<std::shared_ptr<ExprNode>, std::string> parseExpression();
};

class VMInterface {
public:
    virtual ~VMInterface() = default;

    virtual int32_t readStack(int32_t offset) = 0;
    virtual int32_t readGlobal(int32_t global_index) = 0;
    virtual int32_t readScript(int32_t global_index) = 0;
    virtual int32_t readRegister(int32_t zasm_var) = 0;
    virtual std::optional<DebugValue> readObjectMember(DebugValue object, const DebugSymbol* sym) = 0;
	virtual std::optional<std::string> readString(int32_t string_ptr) = 0;
	virtual std::optional<std::vector<DebugValue>> readArray(DebugValue array) = 0;
	virtual std::optional<DebugValue> readArrayElement(DebugValue array, int index) = 0;
	virtual std::optional<int32_t> readArraySize(DebugValue array) = 0;
	virtual void writeGlobal(int32_t offset, int32_t value) = 0;
	virtual void writeScript(int32_t offset, int32_t value) = 0;
	virtual void writeStack(int32_t offset, int32_t value) = 0;
	virtual void writeRegister(int32_t offset, int32_t value) = 0;
	virtual bool writeObjectMember(DebugValue object, const DebugSymbol* sym, DebugValue value) = 0;
	virtual bool writeArrayElement(DebugValue array, int32_t index, DebugValue value) = 0;
	virtual void decreaseObjectReference(DebugValue value, const DebugSymbol* sym) = 0;
	virtual void increaseObjectReference(DebugValue value, const DebugSymbol* sym) = 0;
    virtual expected<int32_t, std::string> executeSandboxed(pc_t start_pc, int this_zasm_var, int this_raw_value, const std::vector<int32_t>& args) = 0;
	virtual DebugValue createArray(std::vector<int32_t> args, const DebugType* array_type) = 0;
	virtual DebugValue createString(const std::string& str) = 0;
    virtual int32_t getThisPointer() = 0;
};

class ExpressionEvaluator
{
	const DebugData& debugData;
	const DebugScope* currentScope;
	VMInterface& vm;

	DebugValue evalBinaryOp(const std::string& op, DebugValue l, DebugValue r);
	const DebugScope* getClassScope(const DebugType* type);
	const DebugScope* resolveOverload(const std::string& name, const std::vector<const DebugType*>& args, const DebugScope* scope);

public:
	ExpressionEvaluator(const DebugData& dd, const DebugScope* scope, VMInterface& v);
	DebugValue evaluate(std::shared_ptr<ExprNode> node);
	DebugValue readSymbol(const DebugSymbol* sym);
	void assignTo(std::shared_ptr<ExprNode> target, DebugValue val);
	std::string printValue(DebugValue value);
};

#endif

#ifndef ZSCRIPT_ASTVISITORS_H
#define ZSCRIPT_ASTVISITORS_H

#include "AST.h"
#include "CompileError.h"

enum
{
	puc_none, puc_vars, puc_funcs, puc_construct, puc_destruct, puc_max
};
namespace ZScript
{
	////////////////////////////////////////////////////////////////
	// Standard AST Visitor.
	class ASTVisitor
	{
	public:
		ASTVisitor() : parsing_user_class(puc_none), scope(nullptr) {}
		virtual ~ASTVisitor() = default;
		
		virtual void caseDefault(AST& host, void* param = NULL) {}
		// AST Subclasses
		virtual void caseFile(ASTFile& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseFloat(ASTFloat& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseString(ASTString& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseSetOption(ASTSetOption& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseUsing(ASTUsingDecl& host, void* param = NULL) {
			caseDefault(host, param);}
		// Statements
		virtual void caseBlock(ASTBlock& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtIf(ASTStmtIf& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtIfElse(ASTStmtIfElse& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtSwitch(ASTStmtSwitch& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseSwitchCases(ASTSwitchCases& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseRange(ASTRange& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtFor(ASTStmtFor& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtForEach(ASTStmtForEach& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtWhile(ASTStmtWhile& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtDo(ASTStmtDo& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtRepeat(ASTStmtRepeat& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtReturn(ASTStmtReturn& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtReturnVal(
				ASTStmtReturnVal& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtBreak(ASTStmtBreak& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtContinue(ASTStmtContinue& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStmtEmpty(ASTStmtEmpty& host, void* param = NULL) {
			caseDefault(host, param);}
		// Declarations
		virtual void caseScript(ASTScript& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseClass(ASTClass& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseNamespace(ASTNamespace& host, void* param = NULL){
			caseDefault(host, param);}
		virtual void caseImportDecl(ASTImportDecl& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseIncludePath(ASTIncludePath& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseImportCondDecl(ASTImportCondDecl& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseFuncDecl(ASTFuncDecl& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseDataDeclList(
				ASTDataDeclList& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseDataEnum(ASTDataEnum& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseDataDecl(ASTDataDecl& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseDataDeclExtraArray(
				ASTDataDeclExtraArray& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseDataTypeDef(ASTDataTypeDef& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseCustomDataTypeDef(ASTCustomDataTypeDef& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseScriptTypeDef(ASTScriptTypeDef& host,
		                               void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseAssert(ASTAssert& host, void* param = NULL) {
			caseDefault(host, param);}
		// Expressions
		virtual void caseExprConst(ASTExprConst& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseVarInitializer(ASTExprVarInitializer& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprAssign(ASTExprAssign& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprIdentifier(
				ASTExprIdentifier& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprArrow(ASTExprArrow& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprIndex(ASTExprIndex& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprCall(ASTExprCall& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprNegate(ASTExprNegate& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprDelete(ASTExprDelete& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprNot(ASTExprNot& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprBitNot(ASTExprBitNot& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprIncrement(
				ASTExprIncrement& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprPreIncrement(
				ASTExprPreIncrement& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprDecrement(
				ASTExprDecrement& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprPreDecrement(
				ASTExprPreDecrement& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprCast(ASTExprCast& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprAnd(ASTExprAnd& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprOr(ASTExprOr& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprGT(ASTExprGT& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprGE(ASTExprGE& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprLT(ASTExprLT& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprLE(ASTExprLE& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprEQ(ASTExprEQ& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprNE(ASTExprNE& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprAppxEQ(ASTExprAppxEQ& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprXOR(ASTExprXOR& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprPlus(ASTExprPlus& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprMinus(ASTExprMinus& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprTimes(ASTExprTimes& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprExpn(ASTExprExpn& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprDivide(ASTExprDivide& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprModulo(ASTExprModulo& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprBitAnd(ASTExprBitAnd& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprBitOr(ASTExprBitOr& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprBitXor(ASTExprBitXor& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprLShift(ASTExprLShift& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprRShift(ASTExprRShift& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseExprTernary(ASTTernaryExpr& host, void* param = NULL) {
			caseDefault(host, param);}
		// Literals
		virtual void caseNumberLiteral(
				ASTNumberLiteral& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseCharLiteral(
				ASTCharLiteral& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseBoolLiteral(
				ASTBoolLiteral& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseStringLiteral(
				ASTStringLiteral& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseArrayLiteral(
				ASTArrayLiteral& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseOptionValue(
				ASTOptionValue& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseIsIncluded(
				ASTIsIncluded& host, void* param = NULL) {
			caseDefault(host, param);}
		// Types
		virtual void caseScriptType(ASTScriptType& host, void* param = NULL) {
			caseDefault(host, param);}
		virtual void caseDataType(ASTDataType& host, void* param = NULL) {
			caseDefault(host, param);}
		
		int parsing_user_class;
	protected:
		//Current scope
		ZScript::Scope* scope;
	};

	////////////////////////////////////////////////////////////////
	// AST Visitor that recurses on elements and handles errors.

	class RecursiveVisitor : public ASTVisitor, public CompileErrorHandler
	{
	public:
		// Used as a parameter to signal that no val is needed.
		static void* const paramNone;
		// Used as a parameter to signal that the rval is needed.
		static void* const paramRead;
		// Used as a parameter to signal that the lval is needed.
		static void* const paramWrite;
		// Used as a parameter to signal that both lval and rval are needed.
		static void* const paramReadWrite;
		
		RecursiveVisitor() : breakNode(NULL), failure(false),
			failure_temp(false), failure_halt(false) {}
	
		// Mark as having failed.
		void fail() {failure = true;}
	
		// Used to signal that a compile error has occured.
		void handleError(CompileError const& error, std::string const* inf = nullptr) /*override*/;
		bool hasError() const /*override*/ {return failure;}

		void deprecWarn(AST* host, std::string const& s1, std::string const& s2);
	
		// Visits a single node. The only virtual visit function as all others
		// defer to this one.
		virtual void visit(AST& node, void* param = NULL);
		
		//internals
		void visitFunctionInternals(ZScript::Program& program);
		virtual void analyzeFunctionInternals(ZScript::Function& function) {}

		////////////////////////////////////////////////////////////////
		// Convenience Functions
	
		// Visits a single node if it exists.
		void visit(AST* node, void* param = NULL);
		// Visit a group of nodes.
		template <class Container>
		void visit(AST& host, Container const& nodes, void* param = NULL)
		{
			for (typename Container::const_iterator it = nodes.cbegin();
			     it != nodes.cend(); ++it)
			{
				if (breakRecursion(host, param)) return;
				visit(**it, param);
			}
		}
		
		template <class Container>
		void block_visit(AST& host, Container const& nodes, void* param = NULL)
		{
			for (typename Container::const_iterator it = nodes.cbegin();
			     it != nodes.cend(); ++it)
			{
				failure_temp = false;
				visit(**it, param);
				if(failure_halt) return;
			}
		}

		////////////////////////////////////////////////////////////////
		// Cases
	
		virtual void caseDefault(AST&, void*) {}
		virtual void caseFile(ASTFile& host, void* param = NULL);
		virtual void caseSetOption(ASTSetOption& host, void* param = NULL);
		// Statements
		virtual void caseBlock(ASTBlock& host, void* param = NULL);
		virtual void caseStmtIf(ASTStmtIf& host, void* param = NULL);
		virtual void caseStmtIfElse(ASTStmtIfElse& host, void* param = NULL);
		virtual void caseStmtSwitch(ASTStmtSwitch & host, void* param = NULL);
		virtual void caseSwitchCases(ASTSwitchCases & host, void* param = NULL);
		virtual void caseRange(ASTRange & host, void* param = NULL);
		virtual void caseStmtFor(ASTStmtFor& host, void* param = NULL);
		virtual void caseStmtForEach(ASTStmtForEach& host, void* param = NULL);
		virtual void caseStmtWhile(ASTStmtWhile& host, void* param = NULL);
		virtual void caseStmtDo(ASTStmtDo& host, void* param = NULL);
		virtual void caseStmtRepeat(ASTStmtRepeat& host, void* param = NULL);
		virtual void caseStmtReturnVal(
				ASTStmtReturnVal& host, void* param = NULL);
		// Declarations
		virtual void caseScript(ASTScript& host, void* param = NULL);
		virtual void caseClass(ASTClass& host, void* param = NULL);
		virtual void caseNamespace(ASTNamespace& host, void* param = NULL);
		virtual void caseImportDecl(ASTImportDecl& host, void* param = NULL);
		virtual void caseIncludePath(ASTIncludePath& host, void* param = NULL);
		virtual void caseImportCondDecl(ASTImportCondDecl& host, void* param = NULL);
		virtual void caseFuncDecl(ASTFuncDecl& host, void* param = NULL);
		virtual void caseDataDeclList(ASTDataDeclList& host, void* param = NULL);
		virtual void caseDataEnum(ASTDataEnum& host, void* param = NULL);
		virtual void caseDataDecl(ASTDataDecl& host, void* param = NULL);
		virtual void caseDataDeclExtraArray(
				ASTDataDeclExtraArray& host, void* param = NULL);
		virtual void caseDataTypeDef(ASTDataTypeDef&, void* param = NULL);
		virtual void caseCustomDataTypeDef(ASTCustomDataTypeDef&, void* param = NULL);
		virtual void caseAssert(ASTAssert& host, void* param = NULL);
		// Expressions
		virtual void caseExprConst(ASTExprConst& host, void* param = NULL);
		virtual void caseVarInitializer(ASTExprVarInitializer& host, void* param = NULL);
		virtual void caseExprAssign(ASTExprAssign& host, void* param = NULL);
		virtual void caseExprArrow(ASTExprArrow& host, void* param = NULL);
		virtual void caseExprIndex(ASTExprIndex& host, void* param = NULL);
		virtual void caseExprCall(ASTExprCall& host, void* param = NULL);
		virtual void caseExprNegate(ASTExprNegate& host, void* param = NULL);
		virtual void caseExprDelete(ASTExprDelete& host, void* param = NULL);
		virtual void caseExprNot(ASTExprNot& host, void* param = NULL);
		virtual void caseExprBitNot(ASTExprBitNot& host, void* param = NULL);
		virtual void caseExprIncrement(
				ASTExprIncrement& host, void* param = NULL);
		virtual void caseExprPreIncrement(
				ASTExprPreIncrement& host, void* param = NULL);
		virtual void caseExprDecrement(
				ASTExprDecrement& host, void* param = NULL);
		virtual void caseExprPreDecrement(
				ASTExprPreDecrement& host, void* param = NULL);
		virtual void caseExprCast(ASTExprCast& host, void* = NULL);
		virtual void caseExprAnd(ASTExprAnd& host, void* param = NULL);
		virtual void caseExprOr(ASTExprOr& host, void* param = NULL);
		virtual void caseExprGT(ASTExprGT& host, void* param = NULL);
		virtual void caseExprGE(ASTExprGE& host, void* param = NULL);
		virtual void caseExprLT(ASTExprLT& host, void* param = NULL);
		virtual void caseExprLE(ASTExprLE& host, void* param = NULL);
		virtual void caseExprEQ(ASTExprEQ& host, void* param = NULL);
		virtual void caseExprNE(ASTExprNE& host, void* param = NULL);
		virtual void caseExprAppxEQ(ASTExprAppxEQ& host, void* param = NULL);
		virtual void caseExprXOR(ASTExprXOR& host, void* param = NULL);
		virtual void caseExprPlus(ASTExprPlus& host, void* param = NULL);
		virtual void caseExprMinus(ASTExprMinus& host, void* param = NULL);
		virtual void caseExprTimes(ASTExprTimes& host, void* param = NULL);
		virtual void caseExprExpn(ASTExprExpn& host, void* param = NULL);
		virtual void caseExprDivide(ASTExprDivide& host, void* param = NULL);
		virtual void caseExprModulo(ASTExprModulo& host, void* param = NULL);
		virtual void caseExprBitAnd(ASTExprBitAnd& host, void* param = NULL);
		virtual void caseExprBitOr(ASTExprBitOr& host, void* param = NULL);
		virtual void caseExprBitXor(ASTExprBitXor& host, void* param = NULL);
		virtual void caseExprLShift(ASTExprLShift& host, void* param = NULL);
		virtual void caseExprRShift(ASTExprRShift& host, void* param = NULL);
		virtual void caseExprTernary(ASTTernaryExpr& host, void* param = NULL);
		// Literals
		virtual void caseNumberLiteral(
				ASTNumberLiteral& host, void* param = NULL);
		virtual void caseCharLiteral(
				ASTCharLiteral& host, void* param = NULL);
		virtual void caseArrayLiteral(ASTArrayLiteral& host, void* param = NULL);
		
		bool hasFailed() const {return failure;}
		bool hasTempFailed() const {return failure_temp;}
		
		// Signal a compile error if source can't be cast to target.
		void checkCast(ZScript::DataType const& sourceType,
		               ZScript::DataType const& targetType,
		               AST* node = NULL,
		               bool twoWay = false);
	protected:
		void deprecWarn(Function* func, AST* host, std::string const& s1, std::string const& s2);
		// Returns true if we have failed or for some other reason must break out
		// of recursion. Should be called with the current node and param between
		// each action that can fail.
		virtual bool breakRecursion(AST& host, void* param = NULL) const;
		virtual bool breakRecursion(void* param = NULL) const;

		// Current stack of visited nodes.
		std::vector<AST*> recursionStack;

		// Node which we are breaking recursion until we reach.
		AST* breakNode;
	
		// Set to true if any errors have occurred.
		bool failure;
		
		// Set to true if any errors have occurred. This is cleared when recursion reaches a block-level.
		bool failure_temp;
		
		// Set to true if a hard error occurs (Halting)
		bool failure_halt;
	};
}

#endif


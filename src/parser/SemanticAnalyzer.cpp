#include "SemanticAnalyzer.h"

#include "Scope.h"
#include "CompileError.h"

using namespace ZScript;

////////////////////////////////////////////////////////////////
// SemanticAnalyzer

SemanticAnalyzer::SemanticAnalyzer(Program& program)
	: deprecateGlobals(false), program(program)
{
	scope = &program.globalScope;
	caseProgram(*program.node);
}

void SemanticAnalyzer::analyzeFunctionInternals(Function& function)
{
	ASTFuncDecl* functionDecl = function.node;

	// Create function scope.
	function.internalScope = scope->makeChild();
	Scope& functionScope = *function.internalScope;

	// Grab the script.
	Script* script = NULL;
	if (scope->isScript()) script = &((ScriptScope*)scope)->script;

	// If this is the script's run method, add "this" to the scope.
	if (script && function.name == "run" && *function.returnType == ZVarType::ZVOID)
	{
		ZVarTypeId thisTypeId = ScriptParser::getThisType(script->getType());
		ZVarType const& thisType = *scope->getTable().getType(thisTypeId);
		function.thisVar = functionScope.addVariable(thisType, "this");
	}

	// Add the parameters to the scope.
	vector<ASTDataDecl*>& parameters = functionDecl->parameters;
	for (vector<ASTDataDecl*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
	{
		ASTDataDecl& parameter = **it;
		string const& name = parameter.name;
		ZVarType const& type = *parameter.resolveType(&functionScope);
		Variable* var = functionScope.addVariable(type, name, &parameter);
		if (var == NULL) handleError(CompileError::VarRedef, &parameter, name.c_str());
	}

	// Evaluate the function block under its scope.
	scope = &functionScope;
	AST::execute(functionDecl->block->statements, *this);
	scope = scope->getParent();
}

void SemanticAnalyzer::caseProgram(ASTProgram& host, void*)
{
	// Recurse on elements.
	RecursiveVisitor::caseProgram(host);

	vector<Function*> functions;

	// Analyze function internals.
	functions = program.getUserGlobalFunctions();
	for (vector<Function*>::iterator it = functions.begin(); it != functions.end(); ++it)
		analyzeFunctionInternals(**it);
	for (vector<Script*>::iterator it = program.scripts.begin();
		 it != program.scripts.end(); ++it)
	{
		Script& script = **it;
		scope = script.scope;
		functions = scope->getLocalFunctions();
		for (vector<Function*>::iterator it = functions.begin(); it != functions.end(); ++it)
			analyzeFunctionInternals(**it);
		scope = scope->getParent();
	}

}

// Statements

void SemanticAnalyzer::caseBlock(ASTBlock& host, void*)
{
	// Switch to block scope.
	scope = scope->makeChild();

	// Recurse.
	RecursiveVisitor::caseBlock(host);

	// Restore scope.
	scope = scope->getParent();
}

void SemanticAnalyzer::caseStmtFor(ASTStmtFor& host, void*)
{
	// Switch to block scope.
	scope = scope->makeChild();

	// Recurse.
	RecursiveVisitor::caseStmtFor(host);

	// Restore scope.
	scope = scope->getParent();
}

// Declarations

void SemanticAnalyzer::caseTypeDef(ASTTypeDef& host, void*)
{
	// Resolve the base type under current scope.
	ZVarType const& type = host.type->resolve(*scope);
	if (!type.isResolved())
	{
		handleError(CompileError::UnresolvedType, &host,
		            type.getName().c_str());
		return;
	}

	// Add type to the current scope under its new name.
	scope->addType(host.name, type, &host);
}

void SemanticAnalyzer::caseDataDeclList(ASTDataDeclList& host, void*)
{
	// Resolve the base type.
	ZVarType const& baseType = host.baseType->resolve(*scope);
	if (!baseType.isResolved())
	{
		handleError(CompileError::UnresolvedType, &host,
		            baseType.getName().c_str());
		return;
	}

	// Don't allow void type.
	if (baseType == ZVarType::ZVOID)
	{
		handleError(CompileError::VoidVar, &host);
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !baseType.canBeGlobal())
	{
		handleError(CompileError::RefVar, &host,
		            baseType.getName().c_str());
		return;
	}

	// Recurse on list contents.
	recurse(host, NULL, host.declarations());
}

void SemanticAnalyzer::caseDataDecl(ASTDataDecl& host, void*)
{
	// First do standard recursing.
	RecursiveVisitor::caseDataDecl(host);

	// Then resolve the type.
	ZVarType const& type = *host.resolveType(scope);
	if (!type.isResolved())
	{
		handleError(CompileError::UnresolvedType, &host,
		            type.getName().c_str());
		return;
	}

	// Don't allow void type.
	if (type == ZVarType::ZVOID)
	{
		handleError(CompileError::VoidVar, &host, host.name.c_str());
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !type.canBeGlobal())
	{
		handleError(CompileError::RefVar, &host,
		            (type.getName() + " " + host.name).c_str());
		return;
	}

	// Add the variable to scope.
	Variable* variable = scope->addVariable(type, host.name, &host);

	// If adding it failed, it means this scope already has a variable with
	// that name.
	if (variable == NULL)
	{
		handleError(CompileError::VarRedef, &host, host.name.c_str());
		return;
	}

	// Special message for deprecated global variables.
	if (scope->varDeclsDeprecated)
		handleError(CompileError::DeprecatedGlobal, &host, host.name.c_str());

	// Currently disabled syntaxes:
	if (type.getArrayDepth() > 1)
	{
		handleError(CompileError::UnimplementedFeature, &host,
		            "Nested Array Declarations");
	}
}

void SemanticAnalyzer::caseFuncDecl(ASTFuncDecl& host, void*)
{
	// Resolve the return type under current scope.
	ZVarType const& returnType = host.returnType->resolve(*scope);
	if (!returnType.isResolved())
	{
		handleError(CompileError::UnresolvedType, &host,
		            returnType.getName().c_str());
		return;
	}

	// Gather the paramater types.
	vector<ZVarType const*> paramTypes;
	vector<ASTDataDecl*> const& params = host.parameters;
	for (vector<ASTDataDecl*>::const_iterator it = params.begin();
		 it != params.end(); ++it)
	{
		ASTDataDecl& decl = **it;

		// Resolve the parameter type under current scope.
		ZVarType const& type = *decl.resolveType(scope);
		if (!type.isResolved())
		{
			handleError(CompileError::UnresolvedType, &decl,
			            type.getName().c_str());
			return;
		}

		// Don't allow void params.
		if (type == ZVarType::ZVOID)
		{
			handleError(CompileError::FunctionVoidParam, &decl,
			            decl.name.c_str());
			return;
		}

		paramTypes.push_back(&type);
	}

	// Add the function to the scope.
	Function* function = scope->addFunction(
			&returnType, host.name, paramTypes, &host);
	function->node = &host;

	// If adding it failed, it means this scope already has a function with
	// that name.
	if (function == NULL)
		handleError(CompileError::FunctionRedef, &host, host.name.c_str());
}

void SemanticAnalyzer::caseScript(ASTScript& host, void*)
{
	Script& script = *program.getScript(&host);
	string name = script.getName();

	// Recurse on script elements with its scope.
	scope = script.scope;
	RecursiveVisitor::caseScript(host);
	scope = scope->getParent();
	if (breakRecursion(host)) return;

	// Handle any script errors.
	vector<CompileError const*> errors = script.getErrors();
	for (vector<CompileError const*>::iterator it = errors.begin();
		 it != errors.end(); ++it)
	{
		handleError(**it, &host, name.c_str());
		if (breakRecursion(host)) return;
	}
}

// Expressions

void SemanticAnalyzer::caseExprConst(ASTExprConst& host, void*)
{
	ASTExpr* content = host.content;
	content->execute(*this);
}

void SemanticAnalyzer::caseExprAssign(ASTExprAssign& host, void*)
{
	host.left->markAsLVal();
	RecursiveVisitor::caseExprAssign(host);
}

void SemanticAnalyzer::caseExprCall(ASTExprCall& host, void*)
{
	ASTExpr* left = host.left;

	// If it's an arrow, recurse normally.
	if (left->isTypeArrow()) left->execute(*this);

	// If it's an identifier.
	if (left->isTypeIdentifier())
	{
		ASTExprIdentifier* identifier = (ASTExprIdentifier*)left;
		vector<int> possibleFunctionIds
			= scope->getFunctionIds(identifier->components);
		if (possibleFunctionIds.size() == 0)
		{
			handleError(CompileError::FuncUndeclared, &host,
			            identifier->asString().c_str());
			return;
		}

		scope->getTable().putPossibleNodeFuncIds(&host, possibleFunctionIds);
	}

	// NOTE Add in support for function objects around here.

	// Recurse on parameters.
	RecursiveVisitor::caseExprCall(host);
}

void SemanticAnalyzer::caseExprIdentifier(ASTExprIdentifier& host, void*)
{
	Variable* variable = scope->getVariable(host.components);
	if (!variable)
	{
		handleError(CompileError::VarUndeclared, &host,
		            host.asString().c_str());
		return;
	}

	host.binding = variable;
	scope->getTable().putNodeId(&host, variable->id);
}

// ExprArrow just recurses.

void SemanticAnalyzer::caseExprIndex(ASTExprIndex& host, void*)
{
	// If the left hand side is an arrow, then pass the index over, so if it's
	// a built-in command it has access to it.
	if (host.array->isTypeArrow())
	{
		ASTExprArrow* arrow = (ASTExprArrow*)host.array;
		arrow->index = host.index;
	}

	// Standard recursing.
	RecursiveVisitor::caseExprIndex(host);
}

// Literals

void SemanticAnalyzer::caseNumberLiteral(ASTNumberLiteral& host, void*)
{
    host.setVarType(ZVarType::FLOAT);
}

void SemanticAnalyzer::caseBoolLiteral(ASTBoolLiteral& host, void*)
{
    host.setVarType(ZVarType::BOOL);
}

void SemanticAnalyzer::caseStringLiteral(ASTStringLiteral& host, void*)
{
	// Assign type.
	SymbolTable& table = scope->getTable();
	ZVarType const* type = table.getCanonicalType(ZVarTypeArray(ZVarType::FLOAT));
	host.setVarType(type);

	// Add to scope as a managed literal.
	scope->addLiteral(host, type);
}

void SemanticAnalyzer::caseArrayLiteral(ASTArrayLiteral& host, void*)
{
	// Recurse on type, size, and elements.
	RecursiveVisitor::caseArrayLiteral(host);

	// If present, resolve the type.
	if (host.type)
	{
		ZVarType const& elementType = host.type->resolve(*scope);
		if (!elementType.isResolved())
		{
			handleError(CompileError::UnresolvedType, &host,
			            elementType.getName().c_str());
			return;
		}

		// Disallow void type.
		if (elementType == ZVarType::ZVOID)
		{
			handleError(CompileError::VoidArr, &host);
			return;
		}

		// Convert to array type.
		host.setVarType(scope->getTable().getCanonicalType(ZVarTypeArray(elementType)));
	}

	// Check that we have elements OR a type.
	if (host.getElements().size() == 0 && !host.type)
	{
		handleError(CompileError::EmptyArrayLiteral, &host);
	}

	// Add to scope as a managed literal.
	scope->addLiteral(host);
}


#include "SemanticAnalyzer.h"

#include "Scope.h"
#include "CompileError.h"

using namespace ZScript;

////////////////////////////////////////////////////////////////
// SemanticAnalyzer

SemanticAnalyzer::SemanticAnalyzer(Program& program)
	: failure(false), deprecateGlobals(false), program(program)
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
	vector<ASTDataDecl*>& parameters = functionDecl->getParameters();
	for (vector<ASTDataDecl*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
	{
		ASTDataDecl& parameter = **it;
		string const& name = parameter.name;
		ZVarType const& type = *parameter.resolveType(&functionScope);
		Variable* var = functionScope.addVariable(type, name, &parameter);
		if (var == NULL) compileError(parameter, &CompileError::VarRedef, name.c_str());
	}

	// Evaluate the function block under its scope.
	scope = &functionScope;
	list<ASTStmt*>& statements = functionDecl->block->getStatements();
	for (list<ASTStmt*>::iterator it = statements.begin(); it != statements.end(); ++it)
		(*it)->execute(*this);
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
	ZVarType const& type = host.getType()->resolve(*scope);
	if (!type.isResolved())
	{
		compileError(host, &CompileError::UnresolvedType, type.getName().c_str());
		return;
	}

	// Add type to the current scope under its new name.
	scope->addType(host.getName(), type, &host);
}

void SemanticAnalyzer::caseDataDeclList(ASTDataDeclList& host, void*)
{
	// Resolve the base type.
	ZVarType const& baseType = host.baseType->resolve(*scope);
	if (!baseType.isResolved())
	{
		compileError(host, &CompileError::UnresolvedType, baseType.getName().c_str());
		return;
	}

	// Don't allow void type.
	if (baseType == ZVarType::ZVOID)
	{
		compileError(host, &CompileError::VoidVar);
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !baseType.canBeGlobal())
	{
		compileError(host, &CompileError::RefVar, baseType.getName().c_str());
		return;
	}

	// Recurse on list contents.
	recurse(host, NULL, host.getDeclarations());
}

void SemanticAnalyzer::caseDataDecl(ASTDataDecl& host, void*)
{
	// First do standard recursing.
	RecursiveVisitor::caseDataDecl(host);

	// Then resolve the type.
	ZVarType const& type = *host.resolveType(scope);
	if (!type.isResolved())
	{
		compileError(host, &CompileError::UnresolvedType, type.getName().c_str());
		return;
	}

	// Don't allow void type.
	if (type == ZVarType::ZVOID)
	{
		compileError(host, &CompileError::VoidVar, host.name.c_str());
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !type.canBeGlobal())
	{
		compileError(host, &CompileError::RefVar,
					 (type.getName() + " " + host.name).c_str());
		return;
	}

	// Add the variable to scope.
	Variable* variable = scope->addVariable(type, host.name, &host);

	// If adding it failed, it means this scope already has a variable with
	// that name.
	if (variable == NULL)
	{
		compileError(host, &CompileError::VarRedef, host.name.c_str());
		return;
	}

	// Special message for deprecated global variables.
	if (scope->varDeclsDeprecated)
		compileError(host, &CompileError::DeprecatedGlobal, host.name.c_str());

	// Currently disabled syntaxes:
	if (type.getArrayDepth() > 1)
	{
		compileError(host, &CompileError::UnimplementedFeature,
					 "Nested Array Declarations.");
	}
}

void SemanticAnalyzer::caseFuncDecl(ASTFuncDecl& host, void*)
{
	// Resolve the return type under current scope.
	ZVarType const& returnType = host.returnType->resolve(*scope);
	if (!returnType.isResolved())
	{
		compileError(host, &CompileError::UnresolvedType,
					 returnType.getName().c_str());
		return;
	}

	// Gather the paramater types.
	vector<ZVarType const*> paramTypes;
	vector<ASTDataDecl*> const& params = host.getParameters();
	for (vector<ASTDataDecl*>::const_iterator it = params.begin();
		 it != params.end(); ++it)
	{
		ASTDataDecl& decl = **it;

		// Resolve the parameter type under current scope.
		ZVarType const& type = *decl.resolveType(scope);
		if (!type.isResolved())
		{
			compileError(decl, &CompileError::UnresolvedType,
						 type.getName().c_str());
			return;
		}

		// Don't allow void params.
		if (type == ZVarType::ZVOID)
		{
			compileError(decl, &CompileError::FunctionVoidParam,
						 decl.name.c_str());
			return;
		}

		paramTypes.push_back(&type);
	}

	// Add the function to the scope.
	Function* function = scope->addFunction(&returnType, host.name, paramTypes, &host);
	function->node = &host;

	// If adding it failed, it means this scope already has a function with
	// that name.
	if (function == NULL)
		compileError(host, &CompileError::FunctionRedef, host.name.c_str());
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
		compileError(host, *it, name.c_str());
		if (breakRecursion(host)) return;
	}
}

// Expressions

void SemanticAnalyzer::caseExprConst(ASTExprConst& host, void*)
{
	ASTExpr* content = host.getContent();
	content->execute(*this);

	if (content->hasDataValue())
		host.setDataValue(content->getDataValue());
}

void SemanticAnalyzer::caseExprAssign(ASTExprAssign& host, void*)
{
	host.getLVal()->markAsLVal();
	RecursiveVisitor::caseExprAssign(host);
}

void SemanticAnalyzer::caseExprCall(ASTExprCall& host, void*)
{
	ASTExpr* left = host.getLeft();

	// If it's an arrow, recurse normally.
	if (left->isTypeArrow()) left->execute(*this);

	// If it's an identifier.
	if (left->isTypeIdentifier())
	{
		ASTExprIdentifier* identifier = (ASTExprIdentifier*)left;
		vector<int> possibleFunctionIds = scope->getFunctionIds(identifier->getComponents());
		if (possibleFunctionIds.size() == 0)
		{
			compileError(host, &CompileError::FuncUndeclared,
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
	Variable* variable = scope->getVariable(host.getComponents());
	if (!variable)
	{
		compileError(host, &CompileError::VarUndeclared,
					 host.asString().c_str());
		return;
	}

	scope->getTable().putNodeId(&host, variable->id);
}

// ExprArrow just recurses.

void SemanticAnalyzer::caseExprIndex(ASTExprIndex& host, void*)
{
	// If the left hand side is an arrow, then pass the index over, so if it's
	// a built-in command it has access to it.
	if (host.getArray()->isTypeArrow())
	{
		ASTExprArrow* arrow = (ASTExprArrow*)host.getArray();
		arrow->setIndex(host.getIndex());
	}

	// Standard recursing.
	RecursiveVisitor::caseExprIndex(host);
}

// Literals

void SemanticAnalyzer::caseNumberLiteral(ASTNumberLiteral& host, void*)
{
    host.setVarType(ZVarType::FLOAT);
    pair<string,string> parts = host.getValue()->parseValue();
    pair<long, bool> val = ScriptParser::parseLong(parts);

    if (!val.second)
		compileError(
				host, &CompileError::ConstTrunc, host.getValue()->getValue());

    host.setDataValue(val.first);
}

void SemanticAnalyzer::caseBoolLiteral(ASTBoolLiteral& host, void*)
{
    host.setVarType(ZVarType::BOOL);
    host.setDataValue(host.getValue() ? 1L : 0L);
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
	if (host.getType())
	{
		ZVarType const& elementType = host.getType()->resolve(*scope);
		if (!elementType.isResolved())
		{
			compileError(host, &CompileError::UnresolvedType,
						 elementType.getName().c_str());
			return;
		}

		// Disallow void type.
		if (elementType == ZVarType::ZVOID)
		{
			compileError(host, &CompileError::VoidArr);
			return;
		}

		// Convert to array type.
		host.setVarType(scope->getTable().getCanonicalType(ZVarTypeArray(elementType)));
	}

	// Check that we have elements OR a type.
	if (host.getElements().size() == 0 && !host.getType())
	{
		compileError(host, &CompileError::EmptyArrayLiteral);
	}

	// Add to scope as a managed literal.
	scope->addLiteral(host);
}


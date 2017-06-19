#include "SemanticAnalyzer.h"

#include "Scope.h"
#include "ParseError.h"

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
		if (var == NULL)
		{
            printErrorMsg(&parameter, VARREDEF, name);
            failure = true;
            continue;
        }
	}

	// Evaluate the function block under its scope.
	scope = &functionScope;
	list<ASTStmt*>& statements = functionDecl->block->getStatements();
	for (list<ASTStmt*>::iterator it = statements.begin(); it != statements.end(); ++it)
		(*it)->execute(*this);
	scope = scope->getParent();
}

void SemanticAnalyzer::caseProgram(ASTProgram& host)
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

void SemanticAnalyzer::caseBlock(ASTBlock& host)
{
	// Switch to block scope.
	scope = scope->makeChild();

	// Recurse.
	RecursiveVisitor::caseBlock(host);

	// Restore scope.
	scope = scope->getParent();
}

void SemanticAnalyzer::caseStmtFor(ASTStmtFor& host)
{
	// Switch to block scope.
	scope = scope->makeChild();

	// Recurse.
	RecursiveVisitor::caseStmtFor(host);

	// Restore scope.
	scope = scope->getParent();
}

// Declarations

void SemanticAnalyzer::caseTypeDef(ASTTypeDef& host)
{
	// Resolve the base type under current scope.
	ZVarType const& type = host.getType()->resolve(*scope);
	if (!type.isResolved())
	{
		printErrorMsg(&host, UNRESOLVEDTYPE, type.getName());
		failure = true;
		return;
	}

	// Add type to the current scope under its new name.
	scope->addType(host.getName(), type, &host);
}

void SemanticAnalyzer::caseDataDeclList(ASTDataDeclList& host)
{
	// Resolve the base type.
	ZVarType const& baseType = host.baseType->resolve(*scope);
	if (!baseType.isResolved())
	{
		printErrorMsg(&host, UNRESOLVEDTYPE, baseType.getName());
		failure = true;
		return;
	}

	// Don't allow void type.
	if (baseType == ZVarType::ZVOID)
	{
		failure = true;
		printErrorMsg(&host, VOIDVAR);
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !baseType.canBeGlobal())
	{
		failure = true;
        printErrorMsg(&host, REFVAR, baseType.getName());
		return;
	}

	// Recurse on list contents.
	vector<ASTDataDecl*> const& decls = host.getDeclarations();
	for (vector<ASTDataDecl*>::const_iterator it = decls.begin();
		 it != decls.end(); ++it)
		caseDataDecl(**it);
}

void SemanticAnalyzer::caseDataDecl(ASTDataDecl& host)
{
	// First do standard recursing.
	RecursiveVisitor::caseDataDecl(host);

	// Then resolve the type.
	ZVarType const& type = *host.resolveType(scope);
	if (!type.isResolved())
	{
		printErrorMsg(&host, UNRESOLVEDTYPE, type.getName());
		failure = true;
		return;
	}

	// Don't allow void type.
	if (type == ZVarType::ZVOID)
	{
		failure = true;
		printErrorMsg(&host, VOIDVAR, host.name);
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !type.canBeGlobal())
	{
		failure = true;
        printErrorMsg(&host, REFVAR, type.getName() + " " + host.name);
		return;
	}

	// Add the variable to scope.
	Variable* variable = scope->addVariable(type, host.name, &host);

	// If adding it failed, it means this scope already has a variable with
	// that name.
	if (variable == NULL)
	{
		failure = true;
        printErrorMsg(&host, VARREDEF, host.name);
		return;
	}

	// Special message for deprecated global variables.
	if (scope->varDeclsDeprecated)
        printErrorMsg(&host, DEPRECATEDGLOBAL, host.name);
}

void SemanticAnalyzer::caseFuncDecl(ASTFuncDecl& host)
{
	// Resolve the return type under current scope.
	ZVarType const& returnType = host.returnType->resolve(*scope);
	if (!returnType.isResolved())
	{
		printErrorMsg(&host, UNRESOLVEDTYPE, returnType.getName());
		failure = true;
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
			printErrorMsg(&decl, UNRESOLVEDTYPE, type.getName());
			failure = true;
		}

		// Don't allow void params.
		if (type == ZVarType::ZVOID)
		{
			printErrorMsg(&decl, FUNCTIONVOIDPARAM, decl.name);
			failure = true;
		}

		paramTypes.push_back(&type);
	}

	// Add the function to the scope.
	Function* function = scope->addFunction(&returnType, host.name, paramTypes, &host);
	function->node = &host;

	// If adding it failed, it means this scope already has a function with
	// that name.
	if (function == NULL)
	{
		printErrorMsg(&host, FUNCTIONREDEF, host.name);
		failure = true;
	}
}

void SemanticAnalyzer::caseScript(ASTScript& host)
{
	Script& script = *program.getScript(&host);
	string name = script.getName();

	// Recurse on script elements with its scope.
	scope = script.scope;
	RecursiveVisitor::caseScript(host);
	scope = scope->getParent();
	if (failure) return;

	if (script.hasError())
	{
		script.printErrors();
		failure = true;
		return;
	}

}

// Expressions

void SemanticAnalyzer::caseExprAssign(ASTExprAssign& host)
{
	host.getLVal()->markAsLVal();
	RecursiveVisitor::caseExprAssign(host);
}

void SemanticAnalyzer::caseExprCall(ASTExprCall& host)
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
			printErrorMsg(&host, FUNCUNDECLARED, identifier->asString());
			failure = true;
			return;
		}

		scope->getTable().putPossibleNodeFuncIds(&host, possibleFunctionIds);
	}

	// NOTE Add in support for function objects around here.

	// Recurse on parameters.
	RecursiveVisitor::caseExprCall(host);
}

void SemanticAnalyzer::caseExprIdentifier(ASTExprIdentifier& host)
{
	Variable* variable = scope->getVariable(host.getComponents());
	if (!variable)
	{
		printErrorMsg(&host, VARUNDECLARED, host.asString());
		failure = true;
		return;
	}

	scope->getTable().putNodeId(&host, variable->id);
}

// ExprArrow just recurses.

void SemanticAnalyzer::caseExprIndex(ASTExprIndex& host)
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

void SemanticAnalyzer::caseStringLiteral(ASTStringLiteral& host)
{
	// Assign type.
	SymbolTable& table = scope->getTable();
	ZVarType const* type = table.getCanonicalType(ZVarTypeArray(ZVarType::FLOAT));
	host.setVarType(type);

	// Add to scope as a managed literal.
	scope->addLiteral(host, type);
}

void SemanticAnalyzer::caseArrayLiteral(ASTArrayLiteral& host)
{
	// Recurse on type, size, and elements.
	RecursiveVisitor::caseArrayLiteral(host);

	// If present, resolve the type.
	if (host.getType())
	{
		ZVarType const& elementType = host.getType()->resolve(*scope);
		if (!elementType.isResolved())
		{
			printErrorMsg(&host, UNRESOLVEDTYPE, elementType.getName());
			failure = true;
			return;
		}

		// Disallow void type.
		if (elementType == ZVarType::ZVOID)
		{
			printErrorMsg(&host, VOIDARR);
			failure = true;
			return;
		}

		// Convert to array type.
		host.setVarType(scope->getTable().getCanonicalType(ZVarTypeArray(elementType)));
	}

	// Check that we have elements OR a type.
	if (host.getElements().size() == 0 && !host.getType())
	{
		printErrorMsg(&host, EMPTYARRAYLITERAL);
		failure = true;
	}

	// Add to scope as a managed literal.
	scope->addLiteral(host);
}


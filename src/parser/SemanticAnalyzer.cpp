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
	list<ASTVarDecl*>& parameters = functionDecl->getParams();
	for (list<ASTVarDecl*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
	{
		ASTVarDecl& parameter = **it;
		string const& name = parameter.getName();
		ZVarType const& type = parameter.getType()->resolve(functionScope);
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
	list<ASTStmt*>& statements = functionDecl->getBlock()->getStatements();
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

void SemanticAnalyzer::caseVarDecl(ASTVarDecl& host)
{
	// Resolve the type under current scope.
	ZVarType const& type = host.getType()->resolve(*scope);
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
		printErrorMsg(&host, VOIDVAR, host.getName());
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !type.canBeGlobal())
	{
		failure = true;
        printErrorMsg(&host, REFVAR, type.getName() + " " + host.getName());
		return;
	}

	// Add variable to the scope.
	Variable* var = scope->addVariable(type, host.getName(), &host);

	// If adding it failed, it means this scope already has a variable with
	// that name.
	if (var == NULL)
	{
		failure = true;
        printErrorMsg(&host, VARREDEF, host.getName());
		return;
	}

	// Special message for deprecated global variables.
	if (scope->varDeclsDeprecated)
        printErrorMsg(&host, DEPRECATEDGLOBAL, host.getName());
}

void SemanticAnalyzer::caseVarDeclInitializer(ASTVarDeclInitializer& host)
{
	// Recurse on initializer.
	host.getInitializer()->execute(*this);

	// Do standard variable declaration.
	caseVarDecl(host);
}

void SemanticAnalyzer::caseArrayDecl(ASTArrayDecl& host)
{
	// Resolve the type under current scope.
	ZVarType const& elementType = host.getType()->resolve(*scope);
	if (!elementType.isResolved())
	{
		printErrorMsg(&host, UNRESOLVEDTYPE, elementType.getName());
		failure = true;
		return;
	}

	// Don't allow void type.
	if (elementType == ZVarType::ZVOID)
	{
		failure = true;
		printErrorMsg(&host, VOIDARR, host.getName());
		return;
	}

	// Convert to array type.
	ZVarTypeArray type(elementType);

	// Check for disallowed global types.
	if (scope->isGlobal() && !type.canBeGlobal())
	{
		failure = true;
        printErrorMsg(&host, REFARR, type.getName() + " " + host.getName());
		return;
	}

	// Add array to the scope.
	Variable* var = scope->addVariable(type, host.getName(), &host);

	// If adding it failed, it means this scope already has a variable with
	// that name.
	if (var == NULL)
	{
		failure = true;
        printErrorMsg(&host, ARRREDEF, host.getName());
		return;
	}

	// Special message for deprecated global variables.
	if (scope->isGlobal() && deprecateGlobals)
        printErrorMsg(&host, DEPRECATEDGLOBAL, host.getName());

	// Recurse on array's size.
	host.getSize()->execute(*this);

	// And on the initializer list.
	ASTArrayList* initializer = host.getList();
	if (initializer)
	{
		for (list<ASTExpr*>::iterator it = initializer->getList().begin(); it != initializer->getList().end(); ++it)
			(*it)->execute(*this);
	}
}

void SemanticAnalyzer::caseFuncDecl(ASTFuncDecl& host)
{
	// Resolve the return type under current scope.
	ZVarType const& returnType = host.getReturnType()->resolve(*scope);
	if (!returnType.isResolved())
	{
		printErrorMsg(&host, UNRESOLVEDTYPE, returnType.getName());
		failure = true;
	}

	// Gather the paramater types.
	vector<ZVarType const*> paramTypes;
	list<ASTVarDecl*>& params = host.getParams();
	for (list<ASTVarDecl*>::iterator it = params.begin(); it != params.end(); ++it)
	{
		// Resolve the paramater type under current scope.
		ZVarType const& type = (*it)->getType()->resolve(*scope);
		if (!type.isResolved())
		{
			printErrorMsg(&host, UNRESOLVEDTYPE, type.getName());
			failure = true;
		}

		// Don't allow void params.
		if (type == ZVarType::ZVOID)
		{
			printErrorMsg(*it, FUNCTIONVOIDPARAM, (*it)->getName());
			failure = true;
		}

		paramTypes.push_back(&type);
	}

	// Add the function to the scope.
	Function* function = scope->addFunction(&returnType, host.getName(), paramTypes, &host);
	function->node = &host;

	// If adding it failed, it means this scope already has a function with
	// that name.
	if (function == NULL)
	{
		printErrorMsg(&host, FUNCTIONREDEF, host.getName());
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
	// Assign varible id for anonymous "variable".
	int variableId = ScriptParser::getUniqueVarID();
	scope->getTable().putNodeId(&host, variableId);
}


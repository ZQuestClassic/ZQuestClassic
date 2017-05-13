#include "SemanticAnalyzer.h"

#include "Scope.h"
#include "ParseError.h"

////////////////////////////////////////////////////////////////
// SemanticAnalyzer

SemanticAnalyzer::SemanticAnalyzer() : failure(false), deprecateGlobals(false)
{
	results.symbols = &data;
}

void SemanticAnalyzer::analyzeFunctionInternals(ASTScript* script, ASTFuncDecl& function)
{
	// Create function scope.
	BasicScope functionScope(scope);

	// Grab the script type.
	ScriptType scriptType = SCRIPTTYPE_VOID;
	if (script) scriptType = script->getType()->getType();

	// If this is the script's run method, add "this" to the scope.
	ZVarType const& returnType = function.getReturnType()->resolve(*scope);
	if (function.getName() == "run" && returnType == ZVarType::ZVOID)
	{
		ZVarTypeId thisTypeId = ScriptParser::getThisType(scriptType);
		int variableId = functionScope.addVariable("this", thisTypeId);
		results.thisPtr[script] = variableId;
	}

	// Add the parameters to the scope.
	list<ASTVarDecl*>& parameters = function.getParams();
	for (list<ASTVarDecl*>::iterator it = parameters.begin(); it != parameters.end(); ++it)
	{
		ASTVarDecl& parameter = **it;
		string const& name = parameter.getName();
		ZVarType const& type = parameter.getType()->resolve(functionScope);
		int variableId = functionScope.addVariable(name, type, &parameter);
		if (variableId == -1)
		{
            printErrorMsg(&parameter, VARREDEF, name);
            failure = true;
            continue;
        }
	}

	// Evaluate the function block under its scope.
	scope = &functionScope;
	list<ASTStmt*>& statements = function.getBlock()->getStatements();
	for (list<ASTStmt*>::iterator it = statements.begin(); it != statements.end(); ++it)
		(*it)->execute(*this);
	scope = scope->getParent();
}

void SemanticAnalyzer::caseProgram(ASTProgram& host)
{
	// Create the global scope.
	scope = new GlobalScope(data);

	// Recurse on elements.
	RecursiveVisitor::caseProgram(host);

	// Analyze function internals.
	for (vector<ASTFuncDecl*>::iterator it = host.functions.begin(); it != host.functions.end(); ++it)
		analyzeFunctionInternals(**it);
	for (vector<ASTScript*>::iterator it = host.scripts.begin(); it != host.scripts.end(); ++it)
	{
		ASTScript& script = **it;
		list<ASTDecl*>& decls = script.getScriptBlock()->getDeclarations();
		for (list<ASTDecl*>::iterator it = decls.begin(); it != decls.end(); ++it)
		{
			// Only do function declarations.
			bool isFuncDecl;
			IsFuncDecl temp;
			(*it)->execute(temp, &isFuncDecl);
			if (!isFuncDecl) continue;
			// Analyze.
			scope = scope->getLocalChild(script.getName());
			analyzeFunctionInternals(&script, (ASTFuncDecl&)**it);
			scope = scope->getParent();
		}
	}

	// Save results.
	results.globalFuncs = host.functions;
	results.globalVars = host.variables;
	results.globalArrays = host.arrays;
	results.scripts = host.scripts;
}

// Statements

void SemanticAnalyzer::caseBlock(ASTBlock& host)
{
	// Switch to block scope.
	BasicScope blockScope(scope);
	scope = &blockScope;

	// Recurse.
	RecursiveVisitor::caseBlock(host);

	// Restore scope.
	scope = scope->getParent();
}

void SemanticAnalyzer::caseStmtFor(ASTStmtFor& host)
{
	// Switch to block scope.
	BasicScope blockScope(scope);
	scope = &blockScope;

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
	int variableId = scope->addVariable(host.getName(), type, &host);

	// If adding it failed, it means this scope already has a variable with
	// that name.
	if (variableId == -1)
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
	int variableId = scope->addVariable(host.getName(), type, &host);

	// If adding it failed, it means this scope already has a variable with
	// that name.
	if (variableId == -1)
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
	vector<ZVarType*> paramTypes;
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

		paramTypes.push_back((ZVarType*)&type);
	}

	// Add the function to the scope.
	int functionId = scope->addFunction(host.getName(), returnType, paramTypes, &host);

	// If adding it failed, it means this scope already has a function with
	// that name.
	if (functionId == -1)
	{
		printErrorMsg(&host, FUNCTIONREDEF, host.getName());
		failure = true;
	}
}

void SemanticAnalyzer::caseScript(ASTScript& host)
{
	string const& name = host.getName();

	// Check the script's type.
	ScriptType scriptType = host.getType()->getType();
	if (scriptType != SCRIPTTYPE_GLOBAL
		&& scriptType != SCRIPTTYPE_ITEM
		&& scriptType != SCRIPTTYPE_FFC)
	{
		printErrorMsg(&host, SCRIPTBADTYPE, name);
		failure = true;
		return;
	}

	// Create script's scope.
	Scope* scriptScope = scope->makeChild(name);
	if (!scriptScope)
	{
		printErrorMsg(&host, SCRIPTREDEF, name);
		failure = true;
		return;
	}
	scriptScope->varDeclsDeprecated = true;

	// Recurse on script elements with its scope.
	scope = scriptScope;
	RecursiveVisitor::caseScript(host);
	scope = scope->getParent();
	if (failure) return;

	// Get run function.
	vector<int> possibleRunIds = scriptScope->getLocalFunctionIds("run");
	if (possibleRunIds.size() > 1)
	{
		printErrorMsg(&host, TOOMANYRUN, name);
		failure = true;
		return;
	}
	else if (possibleRunIds.size() == 0)
	{
		printErrorMsg(&host, SCRIPTNORUN, name);
		failure = true;
		return;
	}
	int runId = possibleRunIds[0];
	if (data.getTypeId(ZVarType::ZVOID) != data.getFuncReturnTypeId(runId))
	{
		printErrorMsg(&host, SCRIPTRUNNOTVOID, name);
		failure = true;
		return;
	}

	// Save script info.
	results.runsymbols[&host] = runId;
	results.numParams[&host] = (int) data.getFuncParamTypeIds(runId).size();
	results.scriptTypes[&host] = scriptType;
}

// Expressions

void SemanticAnalyzer::caseExprAssign(ASTExprAssign& host)
{
	host.getLVal()->markAsLVal();
	RecursiveVisitor::caseExprAssign(host);
}

void SemanticAnalyzer::caseStringConstant(ASTStringConstant& host)
{
	// Assign varible id for anonymous "variable".
	int variableId = ScriptParser::getUniqueVarID();
	scope->getTable().putNodeId(&host, variableId);
}

void SemanticAnalyzer::caseFuncCall(ASTFuncCall& host)
{
	ASTExpr* left = host.getName();

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
	RecursiveVisitor::caseFuncCall(host);
}

void SemanticAnalyzer::caseExprIdentifier(ASTExprIdentifier& host)
{
	int variableId = scope->getVariableId(host.getComponents());
	if (variableId == -1)
	{
		printErrorMsg(&host, VARUNDECLARED, host.asString());
		failure = true;
		return;
	}

	scope->getTable().putNodeId(&host, variableId);
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


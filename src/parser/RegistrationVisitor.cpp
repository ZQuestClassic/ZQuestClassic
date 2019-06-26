/**
 * Registration handler, to ensure that anything global can be referenced in any order
 * Begun: 26th June, 2019
 * Author: Venrob
 */

#include "RegistrationVisitor.h"
#include <cassert>
#include "Scope.h"
#include "CompileError.h"

using std::string;
using std::vector;
using namespace ZScript;

////////////////////////////////////////////////////////////////
// RegistrationVisitor

RegistrationVisitor::RegistrationVisitor(Program& program)
	: program(program)
{
	scope = &program.getScope();
	caseRoot(program.getRoot());
	assert(dynamic_cast<RootScope*>(scope)); //Ensure no scope corruption
}

void RegistrationVisitor::visit(AST& node, void* param)
{
	if(node.isDisabled()) return; //Don't visit disabled nodes.
	if(registered(node)) return; //Don't double-register
	RecursiveVisitor::visit(node, param);
}

void RegistrationVisitor::caseDefault(AST& host, void* param)
{
	host.Register();
}

//Handle the root file specially!
void RegistrationVisitor::caseRoot(ASTFile& host, void* param)
{
	int recursionLimit = 50;
	while(--recursionLimit)
	{
		caseFile(host, param);
		if(registered(host)) return;
	}
	//Failed recursionLimit
	//VENROBTODO Compile Warning Here! SemanticAnalyzer should error, somewhere, as well.
}

void RegistrationVisitor::caseFile(ASTFile& host, void* param)
{
	if(host.scope)
		scope = host.scope;
	else
		scope = host.scope = scope->makeFileChild(host.asString());
	visit(host, host.options, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.use, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.dataTypes, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.scriptTypes, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.imports, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.variables, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.functions, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.namespaces, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.scripts, param);
	if(registered(host.options) && registered(host.use) && registered(host.dataTypes)
		&& registered(host.scriptTypes) && registered(host.imports) && registered(host.variables)
		&& registered(host.functions) && registered(host.namespaces) && registered(host.scripts))
	{
		host.Register();
	}
	scope = scope->getParent();
}

void RegistrationVisitor::caseSetOption(ASTSetOption& host, void* param)
{
	visit(host.expr.get(), param);
	if(!registered(*host.expr)) return; //Non-initialized constant
	
	// If the option name is "default", set the default option instead.
	if (host.name == "default")
	{
		CompileOptionSetting setting = host.getSetting(this, scope);
		if (!setting) return; // error
		scope->setDefaultOption(setting);
		return;
	}
	
	// Make sure the option is valid.
	if (!host.option.isValid())
	{
		handleError(CompileError::UnknownOption(&host, host.name));
		return;
	}

	// Set the option to the provided value.
	CompileOptionSetting setting = host.getSetting(this, scope);
	if (!setting) return; // error
	scope->setOption(host.option, setting);
	host.Register();
}

// Declarations
void RegistrationVisitor::caseScript(ASTScript& host, void* param)
{
	visit(host.type.get());
	if(!registered(*host.type)) return;
	
	Script& script;
	if(host.script)
		script = *host.script;
	else
		script = *(host.script = program.addScript(host, *scope, this));
	if (breakRecursion(host)) return;
	
	string name = script.getName();

	// Recurse on script elements with its scope.
	scope = &script.getScope();
	visit(host, host.options, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.use, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.types, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.variables, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.functions, param);
	scope = scope->getParent();
	if (breakRecursion(host)) return;
	//
	if(registered(host.options) && registered(host.use) && registered(host.types)
		&& registered(host.variables) && registered(host.functions))
	{
		host.Register();
	}
	else return;
	//
	if(script.getType() == ScriptType::untyped) return;
	// Check for a valid run function.
	vector<Function*> possibleRuns =
		//script.getScope().getLocalFunctions("run");
		script.getScope().getLocalFunctions(FFCore.scriptRunString);
	if (possibleRuns.size() == 0)
	{
		handleError(CompileError::ScriptNoRun(&host, name, FFCore.scriptRunString));
		if (breakRecursion(host)) return;
	}
	if (possibleRuns.size() > 1)
	{
		handleError(CompileError::TooManyRun(&host, name, FFCore.scriptRunString));
		if (breakRecursion(host)) return;
	}
	if (*possibleRuns[0]->returnType != DataType::ZVOID)
	{
		handleError(CompileError::ScriptRunNotVoid(&host, name, FFCore.scriptRunString));
		if (breakRecursion(host)) return;
	}
}

void RegistrationVisitor::caseNamespace(ASTNamespace& host, void* param)
{
	Namespace& namesp;
	if(host.namesp)
		namesp = *host.namesp;
	else
		namesp = *(host.namesp = program.addNamespace(host, *scope, this));
	if (breakRecursion(host)) return;

	// Recurse on script elements with its scope.
	// Namespaces' parent scope is RootScope*, not FileScope*. Store the FileScope* temporarily.
	Scope* temp = scope;
	scope = &namesp.getScope();
	visit(host, host.options, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.dataTypes, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.scriptTypes, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.use, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.variables, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.functions, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.namespaces, param);
	if (breakRecursion(host, param)) return;
	visit(host, host.scripts, param);
	scope = temp;
	if(registered(host.options) && registered(host.use) && registered(host.dataTypes)
		&& registered(host.scriptTypes) && registered(host.variables) && registered(host.functions)
		&& registered(host.namespaces) && registered(host.scripts))
	{
		host.Register();
	}
}

void RegistrationVisitor::caseImportDecl(ASTImportDecl& host, void* param)
{
	//Check if the import is valid, or to be stopped by header guard. -V
	if(getRoot(*scope)->checkImport(&host, *lookupOption(*scope, CompileOption::OPT_HEADER_GUARD) / 10000.0, this))
	{
		visit(host.getTree(), param);
		if(registered(host.getTree())) host.Register();
	}
	else
	{
		host.disable(); //Do not use this import; it is a duplicate, and duplicates have been disallowed! -V
	}
}

void RegistrationVisitor::caseUsing(ASTUsingDecl& host, void* param)
{
	//Handle adding scope
	ASTExprIdentifier* iden = host.getIdentifier();
	vector<string> components = iden->components;
	Scope* temp = host.always ? getRoot(*scope) : scope;
	int numMatches = temp->useNamespace(components, iden->delimiters);
	if(numMatches > 1)
		handleError(CompileError::TooManyUsing(&host, iden->asString()));
	else if(numMatches == -1)
		handleError(CompileError::DuplicateUsing(&host, iden->asString()));
	else if(numMatches==1)
		host.Register();
}

void RegistrationVisitor::caseDataTypeDef(ASTDataTypeDef& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseCustomDataTypeDef(ASTCustomDataTypeDef& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseScriptTypeDef(ASTScriptTypeDef& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseDataDeclList(ASTDataDeclList& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseDataEnum(ASTDataEnum& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseDataDeclExtraArray(ASTDataDeclExtraArray& host, void* param)
{
	//VENROBTODO do stuff here!
}

void RegistrationVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	//VENROBTODO do stuff here!
}

// Expressions -- Needed for constant evaluation
void RegistrationVisitor::caseExprConst(ASTExprConst& host, void* param)
{
	RecursiveVisitor::caseExprConst(host, param);
	if (host.getCompileTimeValue(this, scope)) host.Register();
}

void RegistrationVisitor::caseExprAssign(ASTExprAssign& host, void* param)
{
	visit(host.left.get(), paramWrite);
	if (breakRecursion(host)) return;
	visit(host.right.get(), paramRead);
	if (breakRecursion(host)) return;	
	if(!registered(*host.left, *host.right)) return;
	DataType const* ltype = host.left->getWriteType(scope, this);
	if (!ltype)
	{
		handleError(
			CompileError::NoWriteType(
				host.left.get(), host.left->asString()));
		return;
	}
	host.Register();
}

void RegistrationVisitor::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
	// Bind to named variable.
	host.binding = lookupDatum(*scope, host, this);
	if (!host.binding) return;

	// Can't write to a constant.
	if (param == paramWrite || param == paramReadWrite)
	{
		if (host.binding->type.isConstant())
		{
			handleError(CompileError::LValConst(&host, host.asString()));
			return;
		}
	}
	host.Register();
}

void RegistrationVisitor::caseExprArrow(ASTExprArrow& host, void* param)
{
	//VENROBTODO Error here. Arrows should not be found in global initializers.
}

void RegistrationVisitor::caseExprIndex(ASTExprIndex& host, void* param)
{
	visit(host.array.get());
	syncDisable(host, *host.array);
	if (breakRecursion(host)) return;
	visit(host.index.get());
	syncDisable(host, *host.index);
	if (breakRecursion(host)) return;
	if(registered(*host.array, *host.index)) host.Register();
}

void RegistrationVisitor::caseExprCall(ASTExprCall& host, void* param)
{
	//VENROBTODO Error here. Calls should not be found in global initializers.
}

void RegistrationVisitor::caseExprNegate(ASTExprNegate& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprNot(ASTExprNot& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprBitNot(ASTExprBitNot& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprIncrement(ASTExprIncrement& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprPreIncrement(ASTExprPreIncrement& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprDecrement(ASTExprDecrement& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprPreDecrement(ASTExprPreDecrement& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprCast(ASTExprCast& host, void* param)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprAnd(ASTExprAnd& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprOr(ASTExprOr& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprGT(ASTExprGT& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprGE(ASTExprGE& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprLT(ASTExprLT& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprLE(ASTExprLE& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprEQ(ASTExprEQ& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprNE(ASTExprNE& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprPlus(ASTExprPlus& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprMinus(ASTExprMinus& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprTimes(ASTExprTimes& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprDivide(ASTExprDivide& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprModulo(ASTExprModulo& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprBitAnd(ASTExprBitAnd& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprBitOr(ASTExprBitOr& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprBitXor(ASTExprBitXor& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprLShift(ASTExprLShift& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprRShift(ASTExprRShift& host, void* param)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprTernary(ASTTernaryExpr& host, void* param)
{
	visit(host.left.get());
	syncDisable(host, *host.left);
	if (breakRecursion(host)) return;
	visit(host.middle.get());
	syncDisable(host, *host.middle);
	if (breakRecursion(host)) return;
	visit(host.right.get());
	syncDisable(host, *host.right);
	if (breakRecursion(host)) return;
	if(registered(*host.left) && registered(*host.middle) && registered(*host.right)) host.Register();
}

//Types
void RegistrationVisitor::caseScriptType(ASTScriptType& host, void* param)
{
	ScriptType const& type = resolveScriptType(host, *scope);
	if(type.isValid()) host.Register();
}

void RegistrationVisitor::caseDataType(ASTDataType& host, void* param)
{
	DataType const& type = host.resolve(*scope, this);
	if(type.isResolved()) host.Register();
}

//Helper Functions
void RegistrationVisitor::analyzeUnaryExpr(ASTUnaryExpr& host)
{
	visit(host.operand.get());
	syncDisable(host, *host.operand);
	if (breakRecursion(host)) return;
	if(registered(*host.operand))host.Register();
}

void RegistrationVisitor::analyzeBinaryExpr(ASTBinaryExpr& host)
{
	visit(host.left.get());
	syncDisable(host, *host.left);
	if (breakRecursion(host)) return;
	visit(host.right.get());
	syncDisable(host, *host.right);
	if (breakRecursion(host)) return;
	if(registered(*host.left, *host.right)) host.Register();
}



#include "SemanticAnalyzer.h"

#include <cassert>
#include <sstream>
#include "Scope.h"
#include "Types.h"
#include "CompileError.h"

using std::string;
using std::vector;
using std::ostringstream;
using namespace ZScript;

struct tag {};
void* const SemanticAnalyzer::paramNone = new tag();
void* const SemanticAnalyzer::paramRead = new tag();
void* const SemanticAnalyzer::paramWrite = new tag();
void* const SemanticAnalyzer::paramReadWrite = new tag();

////////////////////////////////////////////////////////////////
// SemanticAnalyzer

SemanticAnalyzer::SemanticAnalyzer(Program& program)
	: deprecateGlobals(false), program(program), returnType(NULL)
{
	scope = &program.getScope();
	caseFile(program.getRoot());
	assert(dynamic_cast<RootScope*>(scope));

	// Analyze function internals.
	vector<Function*> functions = program.getUserGlobalFunctions();

	for (vector<Function*>::iterator it = functions.begin();
	     it != functions.end(); ++it)
		analyzeFunctionInternals(**it);
	
	for (vector<Script*>::iterator it = program.scripts.begin();
		 it != program.scripts.end(); ++it)
	{
		Script& script = **it;
		scope = &script.getScope();
		functions = scope->getLocalFunctions();
		for (vector<Function*>::iterator it = functions.begin();
		     it != functions.end(); ++it)
			analyzeFunctionInternals(**it);
		scope = scope->getParent();
	}
}

void SemanticAnalyzer::analyzeFunctionInternals(Function& function)
{
	ASTFuncDecl* functionDecl = function.node;

	Scope& functionScope = *function.internalScope;

	// Grab the script.
	Script* script = NULL;
	if (ScriptScope* ss = dynamic_cast<ScriptScope*>(scope))
		script = &ss->script;

	// Add the parameters to the scope.
	vector<ASTDataDecl*>& parameters = functionDecl->parameters.data();
	for (vector<ASTDataDecl*>::iterator it = parameters.begin();
	     it != parameters.end(); ++it)
	{
		ASTDataDecl& parameter = **it;
		string const& name = parameter.name;
		DataType const& type = *parameter.resolveType(&functionScope);
		Variable::create(functionScope, parameter, type, this);
	}

	// If this is the script's run method, add "this" to the scope.
	if (isRun(function))
	{
		DataTypeId thisTypeId = script->getType().getThisTypeId();
		DataType const& thisType = *scope->getTypeStore().getType(thisTypeId);
		function.thisVar =
			BuiltinVariable::create(functionScope, thisType, "this", this);
	}

	// Evaluate the function block under its scope and return type.
	DataType const* oldReturnType = returnType;
	returnType = function.returnType;
	scope = &functionScope;
	visit(functionDecl->block.get());
	scope = scope->getParent();
	returnType = oldReturnType;
}

void SemanticAnalyzer::caseFile(ASTFile& host, void*)
{
	scope = scope->makeFileChild(host.asString());
	RecursiveVisitor::caseFile(host);
	scope = scope->getParent();
}

void SemanticAnalyzer::caseSetOption(ASTSetOption& host, void*)
{
	// Recurse on elements.
	RecursiveVisitor::caseSetOption(host);
	if (breakRecursion(host)) return;

	// If the option name is "all", set the default option instead.
	if (host.name == "all")
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

void SemanticAnalyzer::caseStmtIf(ASTStmtIf& host, void*)
{
    RecursiveVisitor::caseStmtIf(host);
    if (breakRecursion(host)) return;

	checkCast(*host.condition->getReadType(), DataType::BOOL, &host);
}

void SemanticAnalyzer::caseStmtIfElse(ASTStmtIfElse& host, void*)
{
    RecursiveVisitor::caseStmtIfElse(host);
    if (breakRecursion(host)) return;

	checkCast(*host.condition->getReadType(), DataType::BOOL, &host);
}

void SemanticAnalyzer::caseStmtSwitch(ASTStmtSwitch& host, void*)
{
	RecursiveVisitor::caseStmtSwitch(host);
	if (breakRecursion(host)) return;

	checkCast(*host.key->getReadType(), DataType::FLOAT, &host);
}

void SemanticAnalyzer::caseStmtFor(ASTStmtFor& host, void*)
{
	// Visit children under new scope.
	scope = scope->makeChild();
	RecursiveVisitor::caseStmtFor(host);
	scope = scope->getParent();
    if (breakRecursion(host)) return;

	checkCast(*host.test->getReadType(), DataType::BOOL, &host);
}

void SemanticAnalyzer::caseStmtWhile(ASTStmtWhile& host, void*)
{
	RecursiveVisitor::caseStmtWhile(host);
	if (breakRecursion(host)) return;

	checkCast(*host.test->getReadType(), DataType::BOOL, &host);
}

void SemanticAnalyzer::caseStmtReturn(ASTStmtReturn& host, void*)
{
	if (*returnType != DataType::ZVOID)
		handleError(CompileError::FuncBadReturn(&host));
}

void SemanticAnalyzer::caseStmtReturnVal(ASTStmtReturnVal& host, void*)
{
    RecursiveVisitor::caseStmtReturnVal(host);
    if (breakRecursion(host)) return;

	checkCast(*host.value->getReadType(), *returnType, &host);
}

// Declarations

void SemanticAnalyzer::caseDataTypeDef(ASTDataTypeDef& host, void*)
{
	// Resolve the base type under current scope.
	DataType const& type = host.type->resolve(*scope);
	if (!type.isResolved())
	{
		handleError(CompileError::UnresolvedType(&host, type.getName()));
		return;
	}

	// Add type to the current scope under its new name.
	scope->addDataType(host.name, &type, &host);
}

void SemanticAnalyzer::caseScriptTypeDef(ASTScriptTypeDef& host, void*)
{
	// Resolve the base type under current scope.
	ScriptType type = resolveScriptType(*host.oldType, *scope);
	if (!type.isValid())
	{
		handleError(
			CompileError::UnresolvedScriptType(&host, host.oldType->name));
		return;
	}

	// Add type to the current scope under its new name.
	if (!scope->addScriptType(host.newName, type, &host))
	{
		ScriptType originalType = lookupScriptType(*scope, host.newName);
		if (originalType != type)
			handleError(
				CompileError::RedefScriptType(
					&host, host.newName, originalType.getName()));
		return;
	}
}

void SemanticAnalyzer::caseDataDeclList(ASTDataDeclList& host, void*)
{
	// Resolve the base type.
	DataType const& baseType = host.baseType->resolve(*scope);
	if (!baseType.isResolved())
	{
		handleError(CompileError::UnresolvedType(&host, baseType.getName()));
		return;
	}

	// Don't allow void type.
	if (baseType == DataType::ZVOID)
	{
		handleError(CompileError::VoidVar(&host, host.asString()));
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !baseType.canBeGlobal())
	{
		handleError(CompileError::RefVar(&host, baseType.getName()));
		return;
	}

	// Recurse on list contents.
	visit(host, host.getDeclarations());
}

void SemanticAnalyzer::caseDataDecl(ASTDataDecl& host, void*)
{
	// First do standard recursing.
	RecursiveVisitor::caseDataDecl(host);
	if (breakRecursion(host)) return;

	// Then resolve the type.
	DataType const& type = *host.resolveType(scope);
	if (!type.isResolved())
	{
		handleError(CompileError::UnresolvedType(&host, type.getName()));
		return;
	}

	// Don't allow void type.
	if (type == DataType::ZVOID)
	{
		handleError(CompileError::VoidVar(&host, host.name));
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !type.canBeGlobal())
	{
		handleError(CompileError::RefVar(
				            &host, type.getName() + " " + host.name));
		return;
	}

	// Currently disabled syntaxes:
	if (getArrayDepth(type) > 1)
	{
		handleError(CompileError::UnimplementedFeature(
				            &host, "Nested Array Declarations"));
		return;
	}

	// Is it a constant?
	bool isConstant = false;
	if (type == DataType::CONST_FLOAT)
	{
		// A constant without an initializer doesn't make sense.
		if (!host.getInitializer())
		{
			handleError(CompileError::ConstUninitialized(&host));
			return;
		}

		// Inline the constant if possible.
		isConstant = host.getInitializer()->getCompileTimeValue(this, scope);
	}

	if (isConstant)
	{
		if (scope->getLocalDatum(host.name))
		{
			handleError(CompileError::VarRedef(&host, host.name));
			return;
		}
		
		long value = *host.getInitializer()->getCompileTimeValue(this, scope);
		Constant::create(*scope, host, type, value, this);
	}
	
	else
	{
		if (scope->getLocalDatum(host.name))
		{
			handleError(CompileError::VarRedef(&host, host.name));
			return;
		}

		Variable::create(*scope, host, type, this);
	}

	// Check the initializer.
	if (host.getInitializer())
	{
		// Make sure we can cast the initializer to the type.
		DataType const& initType = *host.getInitializer()->getReadType();
		checkCast(initType, type, &host);
		if (breakRecursion(host)) return;

		// TODO check for array casting here.
	}	
}

void SemanticAnalyzer::caseDataDeclExtraArray(
		ASTDataDeclExtraArray& host, void*)
{
	// Type Check size expressions.
	RecursiveVisitor::caseDataDeclExtraArray(host);
	if (breakRecursion(host)) return;
	
	// Iterate over sizes.
	for (vector<ASTExpr*>::const_iterator it = host.dimensions.begin();
		 it != host.dimensions.end(); ++it)
	{
		ASTExpr& size = **it;

		// Make sure each size can cast to float.
		if (!size.getReadType()->canCastTo(DataType::FLOAT))
		{
			handleError(CompileError::NonIntegerArraySize(&host));
			return;
		}

		// Make sure that the size is constant.
		if (!size.getCompileTimeValue(this, scope))
		{
			handleError(CompileError::ExprNotConstant(&host));
			return;
		}
		
		if(optional<long> theSize = size.getCompileTimeValue(this, scope))
		{
			if(*theSize % 10000)
			{
				handleError(CompileError::ArrayDecimal(&host));
			}
			theSize = (*theSize / 10000);
			if(*theSize < 1 || *theSize > 214748)
			{
				handleError(CompileError::ArrayInvalidSize(&host));
				return;
			}
		}
	}
}

void SemanticAnalyzer::caseFuncDecl(ASTFuncDecl& host, void*)
{
	// Resolve the return type under current scope.
	DataType const& returnType = host.returnType->resolve(*scope);
	if (!returnType.isResolved())
	{
		handleError(
				CompileError::UnresolvedType(&host, returnType.getName()));
		return;
	}

	// Gather the parameter types.
	vector<DataType const*> paramTypes;
	vector<ASTDataDecl*> const& params = host.parameters.data();
	for (vector<ASTDataDecl*>::const_iterator it = params.begin();
		 it != params.end(); ++it)
	{
		ASTDataDecl& decl = **it;

		// Resolve the parameter type under current scope.
		DataType const& type = *decl.resolveType(scope);
		if (!type.isResolved())
		{
			handleError(CompileError::UnresolvedType(&decl, type.getName()));
			return;
		}

		// Don't allow void params.
		if (type == DataType::ZVOID)
		{
			handleError(CompileError::FunctionVoidParam(&decl, decl.name));
			return;
		}

		paramTypes.push_back(&type);
	}

	// Add the function to the scope.
	Function* function = scope->addFunction(
			&returnType, host.name, paramTypes, &host);

	// If adding it failed, it means this scope already has a function with
	// that name.
	if (function == NULL)
	{
		handleError(CompileError::FunctionRedef(&host, host.name));
		return;
	}

	function->node = &host;
}

void SemanticAnalyzer::caseScript(ASTScript& host, void*)
{
	Script& script = *program.addScript(host, *scope, this);
	if (breakRecursion(host)) return;
	
	string name = script.getName();

	// Recurse on script elements with its scope.
	scope = &script.getScope();
	RecursiveVisitor::caseScript(host);
	scope = scope->getParent();
	if (breakRecursion(host)) return;

	// Check for a valid run function.
	vector<Function*> possibleRuns =
		script.getScope().getLocalFunctions("run");
	if (possibleRuns.size() == 0)
	{
		handleError(CompileError::ScriptNoRun(&host, name));
		if (breakRecursion(host)) return;
	}
	if (possibleRuns.size() > 1)
	{
		handleError(CompileError::TooManyRun(&host, name));
		if (breakRecursion(host)) return;
	}
	if (*possibleRuns[0]->returnType != DataType::ZVOID)
	{
		handleError(CompileError::ScriptRunNotVoid(&host, name));
		if (breakRecursion(host)) return;
	}
}

// Expressions

void SemanticAnalyzer::caseExprConst(ASTExprConst& host, void*)
{
	RecursiveVisitor::caseExprConst(host);
	if (breakRecursion(host)) return;

	if (!host.getCompileTimeValue(this, scope))
	{
		handleError(CompileError::ExprNotConstant(&host));
		return;
	}
}

void SemanticAnalyzer::caseExprAssign(ASTExprAssign& host, void*)
{
	visit(host.left.get(), paramWrite);
	if (breakRecursion(host)) return;
	
	visit(host.right.get(), paramRead);
	if (breakRecursion(host)) return;	

    DataType const* rtype = host.right->getReadType();
	if (!rtype)
	{
		handleError(
			CompileError::NoReadType(
				host.right.get(), host.right->asString()));
		return;
	}
    
	DataType const* ltype = host.left->getWriteType();
	if (!ltype)
	{
		handleError(
			CompileError::NoWriteType(
				host.left.get(), host.left->asString()));
		return;
	}
	
	checkCast(*rtype, *ltype, &host);
	if (breakRecursion(host)) return;	

	if (*ltype == DataType::CONST_FLOAT)
		handleError(CompileError::LValConst(&host, host.left->asString()));
	if (breakRecursion(host)) return;	
}

void SemanticAnalyzer::caseExprIdentifier(
		ASTExprIdentifier& host, void* param)
{
	// Bind to named variable.
	host.binding = lookupDatum(*scope, host.components);
	if (!host.binding)
	{
		handleError(CompileError::VarUndeclared(&host, host.asString()));
		return;
	}

	// Can't write to a constant.
	if (param == paramWrite || param == paramReadWrite)
	{
		if (host.binding->type == DataType::CONST_FLOAT)
		{
			handleError(CompileError::LValConst(&host, host.asString()));
			return;
		}
	}
}

void SemanticAnalyzer::caseExprArrow(ASTExprArrow& host, void* param)
{
    // Recurse on left.
	visit(host.left.get());
    if (breakRecursion(host)) return;

	// Grab the left side's class.
    DataTypeClass const* leftType = dynamic_cast<DataTypeClass const*>(
		    &getNaiveType(*host.left->getReadType()));
    if (!leftType)
	{
		handleError(CompileError::ArrowNotPointer(&host));
        return;
	}
	host.leftClass = program.getTypeStore().getClass(leftType->getClassId());

	// Find read function.
	if (!param || param == paramRead || param == paramReadWrite)
	{
		host.readFunction = lookupGetter(*host.leftClass, host.right);
		if (!host.readFunction)
		{
			handleError(
					CompileError::ArrowNoVar(
							&host,
							host.right + (host.index ? "[]" : "")));
			return;
		}
		vector<DataType const*>& paramTypes = host.readFunction->paramTypes;
		if (paramTypes.size() != (host.index ? 2 : 1) || *paramTypes[0] != *leftType)
		{
			handleError(
					CompileError::ArrowNoVar(
							&host,
							host.right + (host.index ? "[]" : "")));
			return;
		}
	}

	// Find write function.
	if (param == paramWrite || param == paramReadWrite)
	{
		host.writeFunction = lookupSetter(*host.leftClass, host.right);
		if (!host.writeFunction)
		{
			handleError(
					CompileError::ArrowNoVar(
							&host,
							host.right + (host.index ? "[]" : "")));
			return;
		}
		vector<DataType const*>& paramTypes = host.writeFunction->paramTypes;
		if (paramTypes.size() != (host.index ? 3 : 2)
		    || *paramTypes[0] != *leftType)
		{
			handleError(
					CompileError::ArrowNoVar(
							&host,
							host.right + (host.index ? "[]" : "")));
			return;
		}
	}

	if (host.index)
	{
		visit(host.index.get());
        if (breakRecursion(host)) return;

        checkCast(*host.index->getReadType(), DataType::FLOAT,
                  host.index.get());
        if (breakRecursion(host)) return;
    }
}

void SemanticAnalyzer::caseExprIndex(ASTExprIndex& host, void* param)
{
	// Arrow handles its own indexing.
	if (host.array->isTypeArrow())
	{
		static_cast<ASTExprArrow&>(*host.array).index = host.index;
		visit(host.array.get(), param);
		return;
	}
	
	RecursiveVisitor::caseExprIndex(host);
	if (breakRecursion(host)) return;

	// The index must be a number.
    if (host.index)
    {
	    checkCast(*host.index->getReadType(), DataType::FLOAT,
	              host.index.get());
        if (breakRecursion(host)) return;
    }
}

void SemanticAnalyzer::caseExprCall(ASTExprCall& host, void*)
{
	// Cast left.
	ASTExprArrow* arrow = NULL;
	if (host.left->isTypeArrow())
		arrow = static_cast<ASTExprArrow*>(host.left.get());
	ASTExprIdentifier* identifier = NULL;
	if (host.left->isTypeIdentifier())
		identifier = static_cast<ASTExprIdentifier*>(host.left.get());
	
	// Don't visit left for identifier, since we don't want to bind to a
	// variable.
	if (!identifier)
	{
		visit(host.left.get(), paramNone);
		if (breakRecursion(host)) return;
	}

	visit(host, host.parameters);
	if (breakRecursion(host)) return;

	// Gather parameter types.
	vector<DataType const*> parameterTypes;
	if (arrow) parameterTypes.push_back(arrow->left->getReadType());
	for (vector<ASTExpr*>::const_iterator it = host.parameters.begin();
		 it != host.parameters.end(); ++it)
		parameterTypes.push_back((*it)->getReadType());

	// Grab functions with the proper name.
	vector<Function*> functions =
		identifier
		? lookupFunctions(*scope, identifier->components)
		: lookupFunctions(*arrow->leftClass, arrow->right);

	// Filter out invalid functions.
	for (vector<Function*>::iterator it = functions.begin();
		 it != functions.end();)
	{
		Function& function = **it;

		// Match against parameter count.
		if (function.paramTypes.size() != parameterTypes.size())
		{
			it = functions.erase(it);
			continue;
		}

		// Check parameter types.
		bool parametersMatch = true;
		for (int i = 0; i < parameterTypes.size(); ++i)
		{
			if (!parameterTypes[i]->canCastTo(*function.paramTypes[i]))
			{
				parametersMatch = false;
				break;
			}
		}
		if (!parametersMatch)
		{
			it = functions.erase(it);
			continue;
		}

		// Keep function.
		++it;
	}

	// Find function with least number of casts.
	vector<Function*> bestFunctions;
	int bestCastCount = parameterTypes.size() + 1;
	for (vector<Function*>::iterator it = functions.begin();
		 it != functions.end(); ++it)
	{
		// Count number of casts.
		Function& function = **it;
		int castCount = 0;
		for (int i = 0; i < parameterTypes.size(); ++i)
		{
			DataType const& from = getBaseType(*parameterTypes[i]);
			DataType const& to = getBaseType(*function.paramTypes[i]);
			if (from == to) continue;
			if (from == DataType::CONST_FLOAT && to == DataType::FLOAT)
				continue;
			++castCount;
		}

		// If this beats the record, clear results and keep it.
		if (castCount < bestCastCount)
		{
			bestFunctions.clear();
			bestFunctions.push_back(&function);
			bestCastCount = castCount;
		}

		// If this just matches the record, append it.
		else if (castCount == bestCastCount)
			bestFunctions.push_back(&function);
	}

	// We failed.
	if (bestFunctions.size() != 1)
	{
		FunctionSignature signature(host.left->asString(), parameterTypes);
		if (bestFunctions.size() == 0)
		{
			handleError(
					CompileError::NoFuncMatch(&host, signature.asString()));
		}
		else
		{
			// Build list of function signatures.
			ostringstream oss;
			for (vector<Function*>::const_iterator it = bestFunctions.begin();
			     it != bestFunctions.end(); ++it)
			{
				oss << "        "
				    << (*it)->getSignature().asString()
				    << "\n";
			}
			
			handleError(
					CompileError::TooFuncMatch(
							&host,
							signature.asString(),
							oss.str()));
		}
		return;
	}

	// Is this a call to a disabled tracing function?
	if (!*lookupOption(*scope, CompileOption::OPT_LOGGING)
	    && bestFunctions.front()->isTracing())
	{
		host.disable();
		return;
	}
		
	host.binding = bestFunctions.front();
}

void SemanticAnalyzer::caseExprNegate(ASTExprNegate& host, void*)
{
	analyzeUnaryExpr(host, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprNot(ASTExprNot& host, void*)
{
	analyzeUnaryExpr(host, DataType::BOOL);
}

void SemanticAnalyzer::caseExprBitNot(ASTExprBitNot& host, void*)
{
	analyzeUnaryExpr(host, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprIncrement(ASTExprIncrement& host, void*)
{
	analyzeIncrement(host);
}

void SemanticAnalyzer::caseExprPreIncrement(ASTExprPreIncrement& host, void*)
{
	analyzeIncrement(host);
}

void SemanticAnalyzer::caseExprDecrement(ASTExprDecrement& host, void*)
{
	analyzeIncrement(host);
}

void SemanticAnalyzer::caseExprPreDecrement(ASTExprPreDecrement& host, void*)
{
	analyzeIncrement(host);
}

void SemanticAnalyzer::caseExprAnd(ASTExprAnd& host, void*)
{
	analyzeBinaryExpr(host, DataType::BOOL, DataType::BOOL);
}

void SemanticAnalyzer::caseExprOr(ASTExprOr& host, void*)
{
	analyzeBinaryExpr(host, DataType::BOOL, DataType::BOOL);
}

void SemanticAnalyzer::caseExprGT(ASTExprGT& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprGE(ASTExprGE& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprLT(ASTExprLT& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprLE(ASTExprLE& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprEQ(ASTExprEQ& host, void*)
{
	RecursiveVisitor::caseExprEQ(host);
	if (breakRecursion(host)) return;

	checkCast(*host.right->getReadType(), *host.left->getReadType());
	if (breakRecursion(host)) return;
}

void SemanticAnalyzer::caseExprNE(ASTExprNE& host, void*)
{
	RecursiveVisitor::caseExprNE(host);
	if (breakRecursion(host)) return;

	checkCast(*host.right->getReadType(), *host.left->getReadType());
	if (breakRecursion(host)) return;
}

void SemanticAnalyzer::caseExprPlus(ASTExprPlus& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprMinus(ASTExprMinus& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprTimes(ASTExprTimes& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprDivide(ASTExprDivide& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprModulo(ASTExprModulo& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprBitAnd(ASTExprBitAnd& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprBitOr(ASTExprBitOr& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprBitXor(ASTExprBitXor& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprLShift(ASTExprLShift& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprRShift(ASTExprRShift& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
}

void SemanticAnalyzer::caseExprTernary(ASTTernaryExpr& host, void*)
{
	visit(host.left.get());
	if (breakRecursion(host)) return;
	checkCast(*host.left->getReadType(), DataType::BOOL, &host);
	if (breakRecursion(host)) return;
	
	visit(host.middle.get());
	if (breakRecursion(host)) return;
	visit(host.right.get());
	if (breakRecursion(host)) return;
	checkCast(*host.middle->getReadType(), *host.right->getReadType(), &host);
	if (breakRecursion(host)) return;
	checkCast(*host.right->getReadType(), *host.middle->getReadType(), &host);
	if (breakRecursion(host)) return;
}

// Literals

void SemanticAnalyzer::caseStringLiteral(ASTStringLiteral& host, void*)
{
	// Add to scope as a managed literal.
	Literal::create(*scope, host, *host.getReadType(), this);
}

void SemanticAnalyzer::caseArrayLiteral(ASTArrayLiteral& host, void*)
{
	RecursiveVisitor::caseArrayLiteral(host);
	if (breakRecursion(host)) return;

	// Check that we have some way to determine type.
	if (host.elements.size() == 0 && !host.type)
	{
		handleError(CompileError::EmptyArrayLiteral(&host));
		return;
	}

	// If present, type check the explicit size.
	if (host.size)
	{
		checkCast(*host.size->getReadType(), DataType::FLOAT,
		          host.size.get());
		if (breakRecursion(host)) return;
	}

	// Don't allow an explicit size if we're part of a declaration.
	if (host.size && host.declaration && hasSize(*host.declaration))
	{
		handleError(CompileError::ArrayLiteralResize(&host));
		return;
	}
	
	// If present, resolve the explicit type.
	if (host.type)
	{
		DataType const& elementType = host.type->resolve(*scope);
		if (!elementType.isResolved())
		{
			handleError(
					CompileError::UnresolvedType(
							&host, elementType.getName()));
			return;
		}

		// Disallow void type.
		if (elementType == DataType::ZVOID)
		{
			handleError(CompileError::VoidArr(&host, host.asString()));
			return;
		}

		// Convert to array type.
		host.setReadType(
				program.getTypeStore().getCanonicalType(
						DataTypeArray(elementType)));
	}

	// Otherwise, default to Untyped -V
	else
	{
		host.setReadType(
				program.getTypeStore().getCanonicalType(
						DataTypeArray(DataType::UNTYPED)));
	}

	// If initialized, check that each element can be cast to type.
	for (vector<ASTExpr*>::iterator it = host.elements.begin();
		 it != host.elements.end(); ++it)
	{
		ASTExpr& element = **it;
		checkCast(*element.getReadType(),
				  host.getReadType()->getElementType(), &host);
		if (breakRecursion(host)) return;
	}
	
	// Add to scope as a managed literal.
	Literal::create(*scope, host, *host.getReadType(), this);
}

void SemanticAnalyzer::caseOptionValue(ASTOptionValue& host, void*)
{
	if (optional<long> value = lookupOption(*scope, host.option))
		host.value = value;
	else
		handleError(CompileError::UnknownOption(&host, host.name));
}

void SemanticAnalyzer::checkCast(
		DataType const& sourceType, DataType const& targetType, AST* node)
{
	if (sourceType.canCastTo(targetType)) return;
	handleError(
		CompileError::IllegalCast(
			node, sourceType.getName(), targetType.getName()));
}

void SemanticAnalyzer::analyzeUnaryExpr(
		ASTUnaryExpr& host, DataType const& type)
{
	visit(host.operand.get());
	if (breakRecursion(host)) return;
	
	checkCast(*host.operand->getReadType(), type, &host);
	if (breakRecursion(host)) return;
}

void SemanticAnalyzer::analyzeIncrement(ASTUnaryExpr& host)
{
	visit(host.operand.get(), paramReadWrite);
    if (breakRecursion(host)) return;

	ASTExpr& operand = *host.operand;
    checkCast(*operand.getReadType(), DataType::FLOAT, &host);
    if (breakRecursion(host)) return;
}

void SemanticAnalyzer::analyzeBinaryExpr(
		ASTBinaryExpr& host, DataType const& leftType,
		DataType const& rightType)
{
	visit(host.left.get());
	if (breakRecursion(host)) return;
	checkCast(*host.left->getReadType(), leftType, &host);
	if (breakRecursion(host)) return;

	visit(host.right.get());
	if (breakRecursion(host)) return;
	checkCast(*host.right->getReadType(), rightType, &host);
	if (breakRecursion(host)) return;
}


#include "SemanticAnalyzer.h"

#include <cassert>
#include <sstream>
#include "Scope.h"
#include "Types.h"
#include "CompileError.h"

using std::string;
using std::vector;
using std::ostringstream;
using std::unique_ptr;
using namespace ZScript;

////////////////////////////////////////////////////////////////
// SemanticAnalyzer

SemanticAnalyzer::SemanticAnalyzer(Program& program)
	: program(program), returnType(NULL), deprecateGlobals(false)
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
	
	for (vector<Namespace*>::iterator it = program.namespaces.begin();
		 it != program.namespaces.end(); ++it)
	{
		Namespace& namesp = **it;
		scope = &namesp.getScope();
		functions = scope->getLocalFunctions();
		for (vector<Function*>::iterator it = functions.begin();
		     it != functions.end(); ++it)
			analyzeFunctionInternals(**it);
		scope = scope->getParent();
	}
	
	for (vector<UserClass*>::iterator it = program.classes.begin();
		 it != program.classes.end(); ++it)
	{
		UserClass& user_class = **it;
		ClassScope* cscope = &user_class.getScope();
		scope = cscope;
		
		DataType const* thisType = &cscope->user_class.getNode()->type->resolve(*cscope,this);
		DataType const* constType = thisType->getConstType();

		functions = cscope->getConstructors();
		parsing_user_class = puc_construct;
		for (vector<Function*>::iterator it = functions.begin();
		     it != functions.end(); ++it)
		{
			Function* func = *it;
			BuiltinVariable::create(*func->internalScope, *constType, "this", this);
			func->internalScope->stackDepth_--;
			analyzeFunctionInternals(*func);
		}
		
		functions = scope->getLocalFunctions();
		for (vector<Function*>::iterator it = functions.begin();
		     it != functions.end(); ++it)
		{
			Function* func = *it;
			if(func->getFlag(FUNCFLAG_STATIC))
				parsing_user_class = puc_none;
			else
			{
				parsing_user_class = puc_funcs;
				BuiltinVariable::create(*func->internalScope, *constType, "this", this);
				func->internalScope->stackDepth_--;
			}
			analyzeFunctionInternals(*func);
		}
		
		functions = cscope->getDestructor();
		parsing_user_class = puc_destruct;
		for (vector<Function*>::iterator it = functions.begin();
		     it != functions.end(); ++it)
		{
			Function* func = *it;
			BuiltinVariable::create(*func->internalScope, *constType, "this", this);
			func->internalScope->stackDepth_--;
			analyzeFunctionInternals(*func);
		}
		parsing_user_class = puc_none;
		
		scope = scope->getParent();
	}
}

void SemanticAnalyzer::analyzeFunctionInternals(Function& function)
{
	if(function.prototype) return; //Prototype functions have no internals to analyze!
	failure_temp = false;
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
		DataType const& type = parameter.resolveType(&functionScope, this);
		if (breakRecursion(parameter)) continue;
		Variable::create(functionScope, parameter, type, this);
	}
	if(breakRecursion()) return;
	

	// If this is the script's run method, add "this" to the scope.
	if (isRun(function))
	{
		DataTypeId thisTypeId = script->getType().getThisTypeId();
		switch(thisTypeId)
		{
			case ZTID_PLAYER:
				function.thisVar =
					BuiltinConstant::create(functionScope, DataType::PLAYER, "this", 0);
				break;
			case ZTID_SCREEN:
				function.thisVar =
					BuiltinConstant::create(functionScope, DataType::SCREEN, "this", 0);
				break;
			case ZTID_VOID:
				function.thisVar =
					BuiltinVariable::create(functionScope, DataType::ZVOID, "", 0);
				break;
			default:
				DataType const* thisType = scope->getTypeStore().getType(thisTypeId);
				DataType const* constType = thisType->getConstType();
				function.thisVar =
					BuiltinVariable::create(functionScope, constType != NULL ? *constType : *thisType, "this", this);
		}
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
	assert(host.scope); //Scope must be made during registration
	scope = host.scope;
	RecursiveVisitor::caseFile(host);
	scope = scope->getParent();
}

void SemanticAnalyzer::caseSetOption(ASTSetOption& host, void*)
{
	if(host.registered()) return; //Skip if already handled
	// Recurse on elements.
	RecursiveVisitor::caseSetOption(host);
	if (breakRecursion(host)) return;

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
}

void SemanticAnalyzer::caseUsing(ASTUsingDecl& host, void*)
{
	if(host.registered()) return; //Skip if already handled
	//Handle adding scope
	ASTExprIdentifier* iden = host.getIdentifier();
	Scope* temp = host.always ? getRoot(*scope) : scope;
	int32_t numMatches = temp->useNamespace(iden->components, iden->delimiters, iden->noUsing);
	if(numMatches > 1)
		handleError(CompileError::TooManyUsing(&host, iden->asString()));
	else if(!numMatches)
		handleError(CompileError::NoUsingMatch(&host, iden->asString()));
	else if(numMatches == -1)
		handleError(CompileError::DuplicateUsing(&host, iden->asString()));
}

// Statements

void SemanticAnalyzer::caseBlock(ASTBlock& host, void*)
{
	// Switch to block scope.
	if(!host.getScope())
	{
		host.setScope(scope->makeChild());
	}
	scope = host.getScope();

	// Recurse.
	RecursiveVisitor::caseBlock(host);

	// Restore scope.
	scope = scope->getParent();
}

void SemanticAnalyzer::caseStmtIf(ASTStmtIf& host, void*)
{
	// Switch to block scope.
	if(!host.getScope())
	{
		host.setScope(scope->makeChild());
	}
	Scope* oldscope = scope;
	scope = host.getScope();

	if(host.isDecl() && !host.condition.get())
		host.condition = new ASTExprIdentifier(host.declaration->name, host.location);
		
	// Recurse.
    RecursiveVisitor::caseStmtIf(host);
	
	// Restore scope.
	scope = oldscope;
    if (breakRecursion(host)) return;

	checkCast(*host.condition->getReadType(scope, this), DataType::UNTYPED, &host);
}

void SemanticAnalyzer::caseStmtIfElse(ASTStmtIfElse& host, void*)
{
    RecursiveVisitor::caseStmtIfElse(host);
}

void SemanticAnalyzer::caseStmtSwitch(ASTStmtSwitch& host, void* param)
{
	bool found_int = false, found_str = false;
	for (vector<ASTSwitchCases*>::iterator it = host.cases.begin(); it != host.cases.end(); ++it)
	{
		ASTSwitchCases* cases = *it;
		if(cases->str_cases.size())
			found_str = true;
		if(cases->cases.size())
			found_int = true;
	}
	if(found_int && found_str) //Error
	{
		handleError(CompileError::MixedSwitch(&host));
		return;
	}
	else if(found_str)
	{
		host.isString = true;
		for (vector<ASTSwitchCases*>::iterator it = host.cases.begin(); it != host.cases.end(); ++it)
		{
			visit(host, (*it)->str_cases, param);
		}
	}
	
	RecursiveVisitor::caseStmtSwitch(host);
	if (breakRecursion(host)) return;

	checkCast(*host.key->getReadType(scope, this), host.isString ? DataType::CHAR : DataType::FLOAT, &host);
}

void SemanticAnalyzer::caseRange(ASTRange& host, void*)
{
	RecursiveVisitor::caseRange(host);
	if(breakRecursion(host)) return;
	std::optional<int32_t> start = (*host.start).getCompileTimeValue(this, scope);
	std::optional<int32_t> end = (*host.end).getCompileTimeValue(this, scope);
	//`start` and `end` must exist, as they are ASTConstExpr. -V
	if(*start > *end)
	{
		handleError(CompileError::RangeInverted(&host, *start, *end));
	}
	else if(*start == *end)
	{
		handleError(CompileError::RangeEqual(&host, *start, *end));
	}
}

void SemanticAnalyzer::caseStmtFor(ASTStmtFor& host, void*)
{
	//Use sub-scope
	if(!host.getScope())
	{
		host.setScope(scope->makeChild());
	}
	scope = host.getScope();
	RecursiveVisitor::caseStmtFor(host);
	scope = scope->getParent();
    if (breakRecursion(host)) return;

	checkCast(*host.test->getReadType(scope, this), DataType::UNTYPED, &host);
}
void SemanticAnalyzer::caseStmtForEach(ASTStmtForEach& host, void* param)
{
	//Use sub-scope
	if(!host.getScope())
	{
		host.setScope(scope->makeChild());
	}
	scope = host.getScope();
	
	visit(host.arrExpr.get(), param);
    if (breakRecursion(host)) {scope = scope->getParent(); return;}
	
	//Get the type of the array expr
	DataType const& arrtype = *host.arrExpr->getReadType(scope, this);
	checkCast(arrtype, DataType::UNTYPED, &host);
    if (breakRecursion(host)) {scope = scope->getParent(); return;}
	//Get the base type of this type
	DataType const& ty = getNaiveType(arrtype, scope);
	
	//The array iter declaration
	ASTDataDecl* indxdecl = new ASTDataDecl(host.location);
	indxdecl->name = "__LOOP_ITER";
	indxdecl->baseType = new ASTDataType(DataType::FLOAT, host.location);
	host.indxdecl = indxdecl;
	//The array holder declaration
	ASTDataDecl* arrdecl = new ASTDataDecl(host.location);
	arrdecl->name = "__LOOP_ARR";
	arrdecl->setInitializer(host.arrExpr.clone());
	arrdecl->baseType = new ASTDataType(ty, host.location);
	host.arrdecl = arrdecl;
	//The data declaration
	ASTDataDecl* decl = new ASTDataDecl(host.location);
	decl->name = host.iden;
	decl->baseType = new ASTDataType(ty, host.location);
	host.decl = decl;
	
	visit(host.indxdecl.get(), param);
    if (breakRecursion(host)) {scope = scope->getParent(); return;}
	visit(host.arrdecl.get(), param);
    if (breakRecursion(host)) {scope = scope->getParent(); return;}
	visit(host.decl.get(), param);
    if (breakRecursion(host)) {scope = scope->getParent(); return;}
	
	visit(host.body.get(), param);
    if (breakRecursion(host)) {scope = scope->getParent(); return;}
	
	scope = scope->getParent();
    if (breakRecursion(host)) return;
	
	if(host.hasElse())
		visit(host.elseBlock.get(), param);
}

void SemanticAnalyzer::caseStmtWhile(ASTStmtWhile& host, void*)
{
	RecursiveVisitor::caseStmtWhile(host);
	if (breakRecursion(host)) return;

	checkCast(*host.test->getReadType(scope, this), DataType::UNTYPED, &host);
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
	
	if(host.value->isTempVal())
	{
		handleError(CompileError::BadTempVal(host.value.get()));
		if (breakRecursion(host)) return;
	}

	checkCast(*host.value->getReadType(scope, this), *returnType, &host);
}

void SemanticAnalyzer::caseStmtBreak(ASTStmtBreak& host, void*)
{
    RecursiveVisitor::caseStmtBreak(host);
    if (breakRecursion(host)) return;

	if(host.count.get())
	{
		if(std::optional<int32_t> v = host.count->getCompileTimeValue(this, scope))
		{
			int32_t val = *v;
			if(val < 0)
			{
				handleError(CompileError::BreakBadCount(&host, val));
				val = 10000;
			}
			host.breakCount = size_t(val/10000);
		}
	}
}

void SemanticAnalyzer::caseStmtContinue(ASTStmtContinue &host, void *)
{
    RecursiveVisitor::caseStmtContinue(host);
    if (breakRecursion(host)) return;

	if(host.count.get())
	{
		if(std::optional<int32_t> v = host.count->getCompileTimeValue(this, scope))
		{
			int32_t val = *v;
			if(val < 0)
			{
				handleError(CompileError::BreakBadCount(&host, val));
				val = 10000;
			}
			host.contCount = size_t(val/10000);
		}
	}
}

// Declarations

void SemanticAnalyzer::caseDataTypeDef(ASTDataTypeDef& host, void*)
{
	if(host.registered()) return; //Skip if already handled
	// Resolve the base type under current scope.
	DataType const& type = host.type->resolve(*scope, this);
    if (breakRecursion(*host.type.get())) return;
	if (!type.isResolved())
	{
		handleError(CompileError::UnresolvedType(&host, type.getName()));
		return;
	}

	// Add type to the current scope under its new name.
	if(!scope->addDataType(host.name, &type, &host))
	{
		unique_ptr<ASTExprIdentifier> temp(new ASTExprIdentifier(host.name, host.location));
		DataType const* originalType = lookupDataType(*scope, *temp, this, true);
		if (breakRecursion(host) || !originalType || (*originalType != type))
			handleError(
				CompileError::RedefDataType(
					&host, host.name));
		temp.reset();
		return;
	}
}

void SemanticAnalyzer::caseCustomDataTypeDef(ASTCustomDataTypeDef& host, void*)
{
	if(host.registered()) return; //Skip if already handled
	if(!host.type)
	{
		//Don't allow use of a name that already exists
		unique_ptr<ASTExprIdentifier> temp(new ASTExprIdentifier(host.name, host.location));
		if(DataType const* existingType = lookupDataType(*scope, *temp, this, true))
		{
			handleError(
				CompileError::RedefDataType(
					&host, host.name));
			temp.reset();
			return;
		}
		temp.reset();
		
		//Construct a new constant type
		DataTypeCustomConst* newConstType = new DataTypeCustomConst("const " + host.name);
		//Construct the base type
		DataTypeCustom* newBaseType = new DataTypeCustom(host.name, newConstType, nullptr, newConstType->getCustomId());
		
		//Set the type to the base type
		host.type.reset(new ASTDataType(newBaseType, host.location));
		//Set the enum type to the const type
		host.definition->baseType.reset(new ASTDataType(newConstType, host.location));
		
		DataType::addCustom(newBaseType);
		
		//This call should never fail, because of the error check above.
		scope->addDataType(host.name, newBaseType, &host);
		if (breakRecursion(*host.type.get())) return;
	}
	RecursiveVisitor::caseCustomDataTypeDef(host);
}

void SemanticAnalyzer::caseScriptTypeDef(ASTScriptTypeDef& host, void*)
{
	if(host.registered()) return; //Skip if already handled
	// Resolve the base type under current scope.
	ParserScriptType type = resolveScriptType(*host.oldType, *scope);
	if (!type.isValid())
	{
		handleError(
			CompileError::UnresolvedScriptType(&host, host.oldType->name));
		return;
	}

	// Add type to the current scope under its new name.
	if (!scope->addScriptType(host.newName, type, &host))
	{
		ParserScriptType originalType = lookupScriptType(*scope, host.newName);
		if (originalType != type)
			handleError(
				CompileError::RedefScriptType(
					&host, host.newName, originalType.getName()));
		return;
	}
}

void SemanticAnalyzer::caseDataDeclList(ASTDataDeclList& host, void*)
{
	if(!host.registered())  //Handle initial setup
	{
		// Resolve the base type.
		DataType const* baseType = &host.baseType->resolve(*scope, this);
		if (breakRecursion(*host.baseType.get())) return;
		if (!host.baseType->wasResolved())
		{
			handleError(CompileError::UnresolvedType(&host, baseType->getName()));
			return;
		}
		
		// Don't allow void type.
		if (baseType->isVoid())
		{
			handleError(CompileError::BadVarType(&host, host.asString(), baseType->getName()));
			return;
		}
		
		if (baseType->isAuto() && host.getDeclarations().size() > 1)
		{
			handleError(CompileError::GroupAuto(&host));
			return;
		}
		
		// Check for disallowed global types.
		if (scope->isGlobal() && !baseType->canBeGlobal())
		{
			handleError(CompileError::RefVar(&host, baseType->getName()));
			return;
		}
	}
	// Recurse on list contents.
	visit(host, host.getDeclarations());
}

void SemanticAnalyzer::caseDataEnum(ASTDataEnum& host, void* param)
{
	if(host.registered())
	{
		visit(host, host.getDeclarations());
		return;
	}
	// Resolve the base type.
	DataType const* baseType = &host.baseType->resolve(*scope, this);
	if (breakRecursion(*host.baseType.get())) return;
	if (!baseType->isResolved())
	{
		handleError(CompileError::UnresolvedType(&host, baseType->getName()));
		return;
	}

	// Don't allow void/auto types.
	if (baseType->isVoid() || baseType->isAuto())
	{
		handleError(CompileError::BadVarType(&host, host.asString(), baseType->getName()));
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !baseType->canBeGlobal())
	{
		handleError(CompileError::RefVar(&host, baseType->getName()));
		return;
	}

	//Handle initializer assignment
	int32_t ipart = -1, dpart = 0;
	std::vector<ASTDataDecl*> decls = host.getDeclarations();
	for(vector<ASTDataDecl*>::iterator it = decls.begin();
		it != decls.end(); ++it)
	{
		ASTDataDecl* declaration = *it;
		ASTExpr* init = declaration->getInitializer();
		if(!init) //auto-fill
		{
			ASTNumberLiteral* value = new ASTNumberLiteral(new ASTFloat(++ipart, dpart, host.location), host.location);
			declaration->setInitializer(value);
		}
		visit(declaration, param);
		if(breakRecursion(host, param)) return;
		if(init) //Set spot for next auto-fill, enforce const-ness
		{
			if(std::optional<int32_t> v = init->getCompileTimeValue(this, scope))
			{
				int32_t val = *v;
				ipart = val/10000;
				dpart = val%10000;
			}
			else
			{
				handleError(CompileError::ExprNotConstant(declaration));
				return;
			}
		}
		if(breakRecursion(host, param)) return;
	}
}

void SemanticAnalyzer::caseDataDecl(ASTDataDecl& host, void*)
{
	// First do standard recursing.
	RecursiveVisitor::caseDataDecl(host);
	if (breakRecursion(host)) return;

	// Then resolve the type.
	DataType const* type = host.resolve_ornull(scope, this);
	if (breakRecursion(host)) return;
	if(!host.registered())  //Handle initial setup
	{
		if (!type->isResolved())
		{
			handleError(CompileError::UnresolvedType(&host, type->getName()));
			return;
		}

		// Don't allow void type.
		if (type->isVoid())
		{
			handleError(CompileError::BadVarType(&host, host.name, type->getName()));
			return;
		}
		
		if (type->isAuto())
		{
			bool good = false;
			auto init = host.getInitializer();
			if(init)
			{
				auto readty = init->getReadType(scope, this);
				if(readty && readty->isResolved() && !readty->isVoid() && !readty->isAuto())
				{
					auto newty = type->isConstant() ? readty->getConstType() : readty->getMutType();
					host.replaceType(*newty);
					type = host.resolve_ornull(scope, this);
					good = true;
				}
			}
			if(!good)
			{
				handleError(CompileError::BadAutoType(&host));
				return;
			}
		}

		// Check for disallowed global types.
		if (scope->isGlobal() && !type->canBeGlobal())
		{
			handleError(CompileError::RefVar(
								&host, type->getName() + " " + host.name));
			return;
		}

		// Currently disabled syntaxes:
		if (getArrayDepth(*type) > 1)
		{
			handleError(CompileError::UnimplementedFeature(
								&host, "Nested Array Declarations"));
			return;
		}

		// Is it a constant?
		bool isConstant = false;
		if (type->isConstant())
		{
			// A constant without an initializer doesn't make sense.
			if (!host.getInitializer())
			{
				handleError(CompileError::ConstUninitialized(&host));
				return;
			}

			// Inline the constant if possible.
			isConstant = host.getInitializer()->getCompileTimeValue(this, scope).has_value();
			//The dataType is constant, but the initializer is not. This is not allowed in Global or Script scopes, as it causes crashes. -V
			if(!isConstant && (scope->isGlobal() || scope->isScript() || scope->isClass()))
			{
				handleError(CompileError::ConstNotConstant(&host, host.name));
				return;
			}
		}
		else if(parsing_user_class == puc_vars) //class variables
		{
			if(host.getInitializer())
			{
				handleError(CompileError::ClassNoInits(&host, host.name));
				return;
			}
		}

		if (isConstant)
		{
			if (scope->getLocalDatum(host.name))
			{
				handleError(CompileError::VarRedef(&host, host.name));
				return;
			}
			
			int32_t value = *host.getInitializer()->getCompileTimeValue(this, scope);
			Constant::create(*scope, host, *type, value, this);
		}
		else
		{
			if(parsing_user_class == puc_vars)
			{
				UserClassVar::create(*scope, host, *type, this);
			}
			else
			{
				if (scope->getLocalDatum(host.name))
				{
					handleError(CompileError::VarRedef(&host, host.name));
					return;
				}

				Variable::create(*scope, host, *type, this);
			}
		}
	}
	
	//Handle typechecking regardless of registration
	// Check the initializer.
	if (auto* initializer = host.getInitializer())
	{
		// Make sure we can cast the initializer to the type.
		DataType const& initType = *initializer->getReadType(scope, this);
		//If this is in an `enum`, then the write type is `CFLOAT`.
		ASTDataType temp=ASTDataType(DataType::CFLOAT, host.location);
		DataType const& enumType = temp.resolve(*scope, this);

		checkCast(initType, (host.list && host.list->isEnum()) ? enumType : *type, &host);
		if (breakRecursion(host)) return;
	}	
}

void SemanticAnalyzer::caseDataDeclExtraArray(
		ASTDataDeclExtraArray& host, void*)
{
	if(host.registered()) return; //Skip if already handled
	// Type Check size expressions.
	RecursiveVisitor::caseDataDeclExtraArray(host);
	if (breakRecursion(host)) return;
	
	// Iterate over sizes.
	for (vector<ASTExpr*>::const_iterator it = host.dimensions.begin();
		 it != host.dimensions.end(); ++it)
	{
		ASTExpr& size = **it;

		// Make sure each size can cast to float.
		if (!size.getReadType(scope, this)->canCastTo(DataType::FLOAT))
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
		
		if(std::optional<int32_t> theSize = size.getCompileTimeValue(this, scope))
		{
			if(*theSize % 10000)
			{
				handleError(CompileError::ArrayDecimal(&host));
			}
			theSize = (*theSize / 10000);
			if(*theSize < 0 || *theSize > 214748)
			{
				handleError(CompileError::ArrayInvalidSize(&host));
				return;
			}
		}
	}
}

void SemanticAnalyzer::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	if(host.registered()) return; //Skip if already handled
	
	Scope* oldScope = scope;
	
	if(host.parentScope)
		scope = host.parentScope;
	else if(host.iden->components.size() > 1)
	{
		ASTExprIdentifier const& id = *(host.iden);
		
		vector<string> scopeNames(id.components.begin(), --id.components.end());
		vector<string> scopeDelimiters(id.delimiters.begin(), id.delimiters.end());
		host.parentScope = lookupScope(*scope, scopeNames, scopeDelimiters, id.noUsing, host, this);
		if(!host.parentScope)
		{
			string scopeName = "";
			vector<string>::const_iterator del = scopeDelimiters.begin();
			for (vector<string>::const_iterator it = scopeNames.begin();
			   it != scopeNames.end();
			   ++it,++del)
			{
				scopeName = scopeName + *it + *del;
			}
			handleError(CompileError::NoScopeFound(&host, scopeName.c_str()));
			return;
		}
		scope = host.parentScope;
	}
	else host.parentScope = scope;
	
	if(host.getFlag(FUNCFLAG_INVALID))
	{
		handleError(CompileError::BadFuncModifiers(&host, host.invalidMsg));
		scope = oldScope;
		return;
	}
	/* This option is being disabled for now, as inlining of user functions is being disabled -V
	if(*lookupOption(*scope, CompileOption::OPT_FORCE_INLINE)
		&& !host.isRun())
	{
		host.setFlag(FUNCFLAG_INLINE);
	}*/
	// Resolve the return type under current scope.
	DataType const& returnType = host.returnType->resolve(*scope, this);
	if (breakRecursion(*host.returnType.get())) {scope = oldScope; return;}
	if (!returnType.isResolved())
	{
		handleError(
				CompileError::UnresolvedType(&host, returnType.getName()));
		scope = oldScope;
		return;
	}
	if(returnType.isAuto())
	{
		handleError(CompileError::BadReturnType(&host, returnType.getName()));
		scope = oldScope;
		return;
	}

	// Gather the parameter types.
	vector<DataType const*> paramTypes;
	vector<ASTDataDecl*> const& params = host.parameters.data();
	vector<string const*> paramNames;
	for (vector<ASTDataDecl*>::const_iterator it = params.begin();
		 it != params.end(); ++it)
	{
		ASTDataDecl& decl = **it;

		// Resolve the parameter type under current scope.
		DataType const& type = decl.resolveType(scope, this);
		if (breakRecursion(decl)) {scope = oldScope; return;}
		if (!type.isResolved())
		{
			handleError(CompileError::UnresolvedType(&decl, type.getName()));
			scope = oldScope;
			return;
		}

		// Don't allow void/auto params.
		if (type.isVoid() || type.isAuto())
		{
			handleError(CompileError::FunctionBadParamType(&decl, decl.name, type.getName()));
			scope = oldScope;
			return;
		}
		paramNames.push_back(new string(decl.name));
		paramTypes.push_back(&type);
	}
	if(host.prototype)
	{
		//Check the default return
		visit(host.defaultReturn.get());
		if(breakRecursion(host.defaultReturn.get())) {scope = oldScope; return;}
		
		DataType const& defValType = *host.defaultReturn->getReadType(scope, this);
		if(!defValType.isResolved())
		{
			handleError(CompileError::UnresolvedType(&host, defValType.getName()));
			scope = oldScope;
			return;
		}
		//Check type validity of default return
		if((*(host.defaultReturn->getCompileTimeValue(this, scope)) == 0) &&
			(defValType == DataType::CUNTYPED || defValType == DataType::UNTYPED))
		{
			//Default is null; don't check casting, as null needs to be valid even for things
			//that untyped does not normally cast to, such as VOID! -V
		}
		else checkCast(defValType, returnType, &host);
	}
	
	if(breakRecursion(host)) {scope = oldScope; return;}
	visit(host, host.optparams, param);
	if(breakRecursion(host)) {scope = oldScope; return;}
	
	auto parcnt = paramTypes.size() - host.optparams.size();
	for(auto it = host.optparams.begin(); it != host.optparams.end() && parcnt < paramTypes.size(); ++it, ++parcnt)
	{
		DataType const* getType = (*it)->getReadType(scope, this);
		if(getType)
			checkCast(*getType, *paramTypes[parcnt], &host);
		if(breakRecursion(host)) {scope = oldScope; return;}
		std::optional<int32_t> optVal = (*it)->getCompileTimeValue(this, scope);
		assert(optVal);
		host.optvals.push_back(*optVal);
	}
	if(breakRecursion(host)) {scope = oldScope; return;}

	// Add the function to the scope.
	Function* function = scope->addFunction(
			&returnType, host.name, paramTypes, paramNames, host.getFlags(), &host, this);
	host.func = function;

	scope = oldScope;
	if(breakRecursion(host)) return;
	// If adding it failed, it means this scope already has a function with
	// that name.
	if (function == NULL)
	{
		if(host.prototype) return; //Skip this error for prototype functions; error is handled inside 'addFunction()' above
		handleError(CompileError::FunctionRedef(&host, host.name));
		return;
	}

	function->node = &host;
}
#include "zc/ffscript.h"
extern FFScript FFCore;
void SemanticAnalyzer::caseScript(ASTScript& host, void* param)
{
	if(!host.script)
	{
		host.script = program.addScript(host, *scope, this);
		if (breakRecursion(host)) return;
	}
	Script& script = *host.script;
	string name = script.getName();
	scope = &script.getScope();
	RecursiveVisitor::caseScript(host, param);
	scope = scope->getParent();
	
	std::optional<int32_t> init_weight = script.getInitWeight();
	if(init_weight && script.getType() != ParserScriptType::global)
	{
		handleError(CompileError::BadAnnotation(&host, "InitScript", "Only scripts of type 'global' can be InitScripts!"));
		return;
	}
	
	if(script.getType() == ParserScriptType::untyped) return;
	
	// Check for a valid run function.
	// Always run this, to ensure it is correct after all registration phase.
	vector<Function*> possibleRuns =
		//script.getScope().getLocalFunctions("run");
		script.getScope().getLocalFunctions(FFCore.scriptRunString);
	if (possibleRuns.size() == 0)
	{
		handleError(CompileError::ScriptNoRun(&host, name, FFCore.scriptRunString));
		return;
	}
	if (possibleRuns.size() > 1)
	{
		handleError(CompileError::TooManyRun(&host, name, FFCore.scriptRunString));
		return;
	}
	if (*possibleRuns[0]->returnType != DataType::ZVOID)
	{
		handleError(CompileError::ScriptRunNotVoid(&host, name, FFCore.scriptRunString));
		return;
	}
	script.setRun(possibleRuns[0]);
}

void SemanticAnalyzer::caseClass(ASTClass& host, void* param)
{
	UserClass& user_class = host.user_class ? *host.user_class : *(host.user_class = program.addClass(host, *scope, this));
	if (breakRecursion(host)) return;
	string name = user_class.getName();
	
	if(!host.type)
	{
		//Don't allow use of a name that already exists
		unique_ptr<ASTExprIdentifier> temp(new ASTExprIdentifier(name, host.location));
		if(DataType const* existingType = lookupDataType(*scope, *temp, this, true))
		{
			handleError(
				CompileError::RedefDataType(
					&host, host.name));
			temp.reset();
			return;
		}
		temp.reset();
		
		//Construct a new constant type
		DataTypeCustomConst* newConstType = new DataTypeCustomConst("const " + host.name, &user_class);
		//Construct the base type
		DataTypeCustom* newBaseType = new DataTypeCustom(host.name, newConstType, &user_class, newConstType->getCustomId());
		
		//Set the type to the base type
		host.type.reset(new ASTDataType(newBaseType, host.location));
		
		DataType::addCustom(newBaseType);
		user_class.setType(newBaseType);
		
		//This call should never fail, because of the error check above.
		scope->addDataType(host.name, newBaseType, &host);
		if (breakRecursion(*host.type.get())) return;
		
		for (auto it = host.constructors.begin();
			 it != host.constructors.end(); ++it)
		{
			ASTFuncDecl* func = *it;
			func->returnType.reset(new ASTDataType(newBaseType, func->location));
		}
	}
	

	// Recurse on user_class elements with its scope.
	scope = &user_class.getScope();
	RecursiveVisitor::caseClass(host,param);
	scope = scope->getParent();
	if (breakRecursion(host)) return;
	//
	if(!host.constructors.size())
	{
		ASTFuncDecl* defcon = new ASTFuncDecl(host.location);
		ASTExprIdentifier* iden = new ASTExprIdentifier(name, host.location);
		defcon->iden = iden;
		defcon->name = name;
		defcon->prototype = true;
		defcon->defaultReturn = new ASTExprConst(new ASTExprCast(
				new ASTDataType(DataType::CUNTYPED, host.location),
				new ASTNumberLiteral(new ASTFloat(0, 0, host.location), host.location), host.location
			), host.location);
		defcon->returnType.reset(new ASTDataType(user_class.getType(), host.location));
		defcon->setFlag(FUNCFLAG_CONSTRUCTOR|FUNCFLAG_CLASSFUNC);
		host.constructors.push_back(defcon);
		//Re-visit the constructors now
		parsing_user_class = puc_construct;
		scope = &user_class.getScope();
		block_visit(host, host.constructors, param);
		scope = scope->getParent();
		parsing_user_class = puc_none;
	}
	else for (auto it = host.constructors.cbegin();
		 it != host.constructors.cend(); ++it)
	{
		ASTFuncDecl const* func = *it;
		if(func->name.compare(name)) //mismatch name
		{
			handleError(CompileError::NameMismatchC(&host, func->name.c_str(), name.c_str()));
			return;
		}
	}
	if(ASTFuncDecl const* func = host.destructor.get())
	{
		if(func->name.compare(name)) //mismatch name
		{
			handleError(CompileError::NameMismatchD(&host, func->name.c_str(), name.c_str()));
			return;
		}
		if(func->parameters.size()) //Destructor must have no params
		{
			handleError(CompileError::DestructorParam(&host, name.c_str()));
			return;
		}
	}
}

void SemanticAnalyzer::caseNamespace(ASTNamespace& host, void*)
{
	assert(host.namesp); //Scope must be made during registration
	Scope* temp = scope;
	scope = &host.namesp->getScope();
	RecursiveVisitor::caseNamespace(host);
	scope = temp;
}

void SemanticAnalyzer::caseImportDecl(ASTImportDecl& host, void*)
{
	//Check if the import is valid, or to be stopped by header guard. -V
	if(getRoot(*scope)->checkImport(&host, *lookupOption(*scope, CompileOption::OPT_HEADER_GUARD) / 10000.0, this))
	{
		RecursiveVisitor::caseImportDecl(host);
	}
}
void SemanticAnalyzer::caseIncludePath(ASTIncludePath& host, void*)
{}

void SemanticAnalyzer::caseImportCondDecl(ASTImportCondDecl& host, void* param)
{
	RecursiveVisitor::caseImportCondDecl(host, param);
}

extern vector<BasicCompileError> casserts;
extern bool delay_asserts;
void SemanticAnalyzer::caseAssert(ASTAssert& host, void* param)
{
	visit(host.expr.get(), param);
    if (breakRecursion(host)) return;
	int32_t val = *(host.expr->getCompileTimeValue(this, scope));
	if(val == 0)
	{
		ASTString* str = host.msg.get();
		CompileError cassert = str
			? CompileError::AssertFail(&host, str->getValue().c_str())
			: CompileError::AssertFail(&host, "");
		if(delay_asserts)
			casserts.emplace_back(cassert);
		else handleError(cassert);
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

void SemanticAnalyzer::caseVarInitializer(ASTExprVarInitializer& host, void*)
{
	RecursiveVisitor::caseExprConst(host);
	if (breakRecursion(host)) return;
	if(!(scope->isGlobal() || scope->isScript())) return; //Only require constant initializer if global var.
	if (!host.valueIsArray(scope, this) && !host.getCompileTimeValue(this, scope))
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

    DataType const* rtype = host.right->getReadType(scope, this);
	if (!rtype)
	{
		handleError(
			CompileError::NoReadType(
				host.right.get(), host.right->asString()));
		return;
	}
    
	DataType const* ltype = host.left->getWriteType(scope, this);
	if (!ltype)
	{
		handleError(
			CompileError::NoWriteType(
				host.left.get(), host.left->asString()));
		return;
	}
	
	if(host.right->isTempVal())
	{
		handleError(CompileError::BadTempVal(host.right.get()));
		if (breakRecursion(host)) return;
	}
	
	checkCast(*rtype, *ltype, &host);
	if (breakRecursion(host)) return;	

	if (ltype->isConstant())
		handleError(CompileError::LValConst(&host, host.left->asString()));
	if (breakRecursion(host)) return;	
}

void SemanticAnalyzer::caseExprIdentifier(
		ASTExprIdentifier& host, void* param)
{
	if(host.binding) return; //Skip if already handled
	// Bind to named variable.
	if(parsing_user_class > puc_vars)
		host.binding = lookupClassVars(*scope, host, this);
	if(!host.binding)
		host.binding = lookupDatum(*scope, host, this);
	if (!host.binding)
	{
		handleError(CompileError::VarUndeclared(&host, host.asString()));
		return;
	}

	// Can't write to a constant.
	if (param == paramWrite || param == paramReadWrite)
	{
		if (host.binding->type.isConstant())
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
	DataType const& base_ltype = *host.left->getReadType(scope, this);
	if(UserClass* user_class = base_ltype.getUsrClass())
	{
		ClassScope* cscope = &user_class->getScope();
		if(UserClassVar* dat = cscope->getClassVar(host.right))
		{
			host.rtype = &dat->type;
			if(!dat->type.isConstant())
				host.wtype = &dat->type;
			host.u_datum = dat;
		}
		else if(!host.iscall)
			handleError(CompileError::ArrowNoVar(
				&host,
				host.right,
				user_class->getName().c_str()));
		return;
	}
    DataTypeClass const* leftType =
		dynamic_cast<DataTypeClass const*>(&getNaiveType(base_ltype, scope));
    if (!leftType)
	{
		handleError(CompileError::ArrowNotPointer(&host));
        return;
	}
	host.leftClass = program.getTypeStore().getClass(leftType->getClassId());
	// zconsole_db("Arrow for type '%s->'", leftType->getName());
	Function* reader = lookupGetter(*host.leftClass, host.right);
	if(reader && reader->getFlag(FUNCFLAG_INTARRAY))
		host.arrayFunction = reader;
	
	// Find read function.
	if (!param || param == paramRead || param == paramReadWrite)
	{
		host.readFunction = reader;
		if (!host.readFunction)
		{
			handleError(
					CompileError::ArrowNoVar(
							&host,
							host.right + (host.index ? "[]" : ""),
							leftType->getName().c_str()));
			return;
		}
		
		vector<DataType const*>& paramTypes = host.readFunction->paramTypes;
		if (paramTypes.size() != ((host.index && !host.arrayFunction) ? 2 : 1) || *paramTypes[0] != *leftType)
		{
			handleError(
					CompileError::ArrowNoVar(
							&host,
							host.right + (host.index ? "[]" : ""),
							leftType->getName().c_str()));
			return;
		}
		
		if(host.arrayFunction)
			deprecWarn(host.arrayFunction, &host, "IntArray", leftType->getName() + "->" + host.right);
		else deprecWarn(host.readFunction, &host, "Variable", leftType->getName() + "->" + host.right);
	}

	// Find write function.
	if (param == paramWrite || param == paramReadWrite)
	{
		if(host.arrayFunction)
		{
			vector<DataType const*>& paramTypes = host.arrayFunction->paramTypes;
			if (paramTypes.size() != 1 || *paramTypes[0] != *leftType)
			{
				handleError(
						CompileError::ArrowNoVar(
								&host,
								host.right + (host.index ? "[]" : ""),
								leftType->getName().c_str()));
				return;
			}
		}
		else
		{
			host.writeFunction = lookupSetter(*host.leftClass, host.right);
			if (!host.writeFunction)
			{
				handleError(
						CompileError::ArrowNoVar(
								&host,
								host.right + (host.index ? "[]" : ""),
								leftType->getName().c_str()));
				return;
			}
			vector<DataType const*>& paramTypes = host.writeFunction->paramTypes;
			if (paramTypes.size() != (host.index ? 3 : 2)
				|| *paramTypes[0] != *leftType)
			{
				handleError(
						CompileError::ArrowNoVar(
								&host,
								host.right + (host.index ? "[]" : ""),
								leftType->getName().c_str()));
				return;
			}
		}
		
		if(host.arrayFunction)
			deprecWarn(host.arrayFunction, &host, "Constant", leftType->getName() + "->" + host.right);
		else deprecWarn(host.writeFunction, &host, "Variable", leftType->getName() + "->" + host.right);
	}

	if (host.index)
	{
		visit(host.index.get());
        if (breakRecursion(host)) return;

        checkCast(*host.index->getReadType(scope, this), DataType::FLOAT,
                  host.index.get());
        if (breakRecursion(host)) return;
    }
}

void SemanticAnalyzer::caseExprIndex(ASTExprIndex& host, void* param)
{
	bool did_array = false;
	// Arrow handles its own indexing.
	if (host.array->isTypeArrow())
	{
		ASTExprArrow* arrow = static_cast<ASTExprArrow*>(host.array.get());
		if (!arrow->arrayFunction && !arrow->isTypeArrowUsrClass())
		{
			arrow->index = host.index;
			visit(host.array.get(), param);
			did_array = true;
			if(arrow->arrayFunction)
				arrow->index = nullptr;
		}
	}
	if(!did_array)
		visit(host.array.get(), param);
	if (breakRecursion(host)) return;

	// The index must be a number.
	if (host.index)
	{
		visit(host.index.get(), paramRead); //only READ the index!
		checkCast(*host.index->getReadType(scope, this), DataType::FLOAT,
				  host.index.get());
		if (breakRecursion(host)) return;
	}
}

void SemanticAnalyzer::caseExprCall(ASTExprCall& host, void* param)
{
	// Cast left.
	ASTExprArrow* arrow = NULL;
	if (host.left->isTypeArrow())
	{
		arrow = static_cast<ASTExprArrow*>(host.left.get());
		arrow->iscall = true;
	}
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

	UserClass* user_class = nullptr;
	// Gather parameter types.
	vector<DataType const*> parameterTypes;
	if (arrow)
	{
		DataType const* arrtype = arrow->left->getReadType(scope, this);
		if((user_class = arrtype->getUsrClass()))
			;
		else parameterTypes.push_back(arrtype);
	}
	for (vector<ASTExpr*>::const_iterator it = host.parameters.begin();
		 it != host.parameters.end(); ++it)
		parameterTypes.push_back((*it)->getReadType(scope, this));

	// Grab functions with the proper name, and matching parameter types
	vector<Function*> functions;
	if(identifier)
	{
		if(host.isConstructor())
		{
			user_class = lookupClass(*scope, identifier->components, identifier->delimiters, identifier->noUsing);
			if(!user_class)
			{
				handleError(CompileError::NoClass(&host, identifier->asString()));
				return;
			}
			functions = lookupConstructors(*user_class, parameterTypes);
		}
		else
		{
			if(identifier->components.size() == 1 && parsing_user_class > puc_vars)
			{
				user_class = &scope->getClass()->user_class;
				if(parsing_user_class == puc_construct && identifier->components[0] == user_class->getName())
					functions = lookupConstructors(*user_class, parameterTypes);
				if(!functions.size())
					functions = lookupFunctions(*scope, identifier->components[0], parameterTypes, identifier->noUsing, true);
			}
			if(!functions.size())
				functions = lookupFunctions(*scope, identifier->components, identifier->delimiters, parameterTypes, identifier->noUsing);
		}
	}
	else if(user_class)
	{
		functions = lookupClassFuncs(*user_class, arrow->right, parameterTypes);
	}
	else functions = lookupFunctions(*arrow->leftClass, arrow->right, parameterTypes, true); //Never `using` arrow functions

	// Find function with least number of casts.
	vector<Function*> bestFunctions;
	int32_t bestCastCount = parameterTypes.size() + 1;
	for (vector<Function*>::iterator it = functions.begin();
		 it != functions.end(); ++it)
	{
		// Count number of casts.
		Function& function = **it;
		int32_t castCount = 0;
		size_t lowsize = zc_min(parameterTypes.size(), function.paramTypes.size());
		for(size_t i = 0; i < lowsize; ++i)
		{
			DataType const& from = getNaiveType(*parameterTypes[i], scope);
			DataType const& to = getNaiveType(*function.paramTypes[i], scope);
			if (from == to) continue;
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
	// We may have failed, but let's check optional parameters first...
	if(bestFunctions.size() > 1)
	{
		auto targSize = parameterTypes.size();
		int32_t bestDiff = -1;
		//Find the best (minimum) difference between the passed param count and function max param count
		for(auto it = bestFunctions.begin(); it != bestFunctions.end(); ++it)
		{
			int32_t maxSize = (*it)->paramTypes.size();
			int32_t diff = maxSize - targSize;
			if(bestDiff < 0 || diff < bestDiff) bestDiff = diff;
		}
		//Remove any functions that don't share the minimum difference.
		for(auto it = bestFunctions.begin(); it != bestFunctions.end();)
		{
			int32_t maxSize = (*it)->paramTypes.size();
			int32_t diff = maxSize - targSize;
			if(diff > bestDiff)
				it = bestFunctions.erase(it);
			else ++it;
		}
	}
	// We may have failed, though namespaces may resolve the issue. Check for namespace closeness.
	if(bestFunctions.size() > 1)
	{
		std::map<Function*, Scope*> bestNSs;
		std::map<Function*, Scope*> bestScripts;
		for (vector<Function*>::const_iterator it = bestFunctions.begin();
		     it != bestFunctions.end(); ++it)
		{
			Scope* ns = NULL;
			Scope* scr = NULL;
			for(Scope* current = (*it)->internalScope; current; current = current->getParent())
			{
				if(!scr && current->isScript())
				{
					scr = current;
				}
				if(current->isNamespace())
				{
					ns = current;
					break;
				}
			}
			bestNSs[*it] = ns;
			bestScripts[*it] = scr;
		}
		Function* bestFound = NULL;
		for(Scope* current = scope; current; current = current->getParent())
		{
			if(current->isScript())
			{
				for (vector<Function*>::const_iterator it = bestFunctions.begin();
				     it != bestFunctions.end(); ++it)
				{
					if(current == bestScripts[*it])
					{
						if(bestFound)
						{
							bestFound = NULL;
							current = NULL;
							break;
						}
						else bestFound = *it;
					}
				}
			}
			else if(current->isNamespace())
			{
				for (vector<Function*>::const_iterator it = bestFunctions.begin();
				     it != bestFunctions.end(); ++it)
				{
					if(current == bestNSs[*it])
					{
						if(bestFound)
						{
							bestFound = NULL;
							current = NULL;
							break;
						}
						else bestFound = *it;
					}
				}
			}
			if(!current) break;
		}
		if(bestFound) //Found a singular best; override the prior calculations, and salvage the call! -V
		{
			bestFunctions.clear();
			bestFunctions.push_back(bestFound);
		}
	}
	// We may have failed, but give higher priority to 'untyped' first...
	if(bestFunctions.size() > 1)
	{
		vector<Function*> newBestFunctions = bestFunctions;
		size_t maxsize = 0;
		for(auto it = newBestFunctions.begin(); it != newBestFunctions.end(); ++it)
		{
			if(maxsize < (*it)->paramTypes.size())
				maxsize = (*it)->paramTypes.size();
		}
		//Remove any strictly-less-specific functions
		for(size_t p = 0; p < maxsize; ++p)
		{
			int flag = 0;
			for(auto it = bestFunctions.begin(); flag != 3 && it != bestFunctions.end();++it)
			{
				auto& pty = (*it)->paramTypes;
				if (pty.size() <= p)
					continue;
				bool ut = pty.at(p)->isUntyped();
				if(ut) flag |= 1;
				else flag |= 2;
			}
			if(flag != 3) continue;
			for(auto it = newBestFunctions.begin(); it != newBestFunctions.end();)
			{
				auto& pty = (*it)->paramTypes;
				if (pty.size() <= p)
				{
					++it;
					continue;
				}
				bool ut = pty.at(p)->isUntyped();
				if (ut) //untyped, keep
				{
					++it;
					continue;
				}
				else if(parameterTypes.size() > p)
				{
					DataType const& from = getNaiveType(*parameterTypes[p], scope);
					DataType const& to = getNaiveType(*pty[p], scope);
					if(from == to) //Exact match, keep
					{
						++it;
						continue;
					}
				}
				//Not exact match, not untyped; junk it.
				it = newBestFunctions.erase(it);
				continue;
			}
			if(newBestFunctions.size() == 0)
				break; //Nothing left to loop on
		}
		if(newBestFunctions.size() > 0) //Don't overwrite if eliminated all
			bestFunctions = newBestFunctions;
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
				oss << "        ";
				string namespacenames = "";
				for(Scope* current = (*it)->internalScope; current; current = current->getParent())
				{
					if(!current->isNamespace()) continue;
					NamespaceScope* ns = static_cast<NamespaceScope*>(current);
					namespacenames = ns->namesp->getName() + "::" + namespacenames;
				}
				oss << namespacenames << (*it)->getSignature().asString() << "\n";
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
	if (*lookupOption(*scope, CompileOption::OPT_NO_LOGGING)
	    && bestFunctions.front()->isTracing())
	{
		host.disable();
		return;
	}
	
	host.binding = bestFunctions.front();
	deprecWarn(host.binding, &host, "Function", host.binding->getSignature().asString());
}

void SemanticAnalyzer::caseExprNegate(ASTExprNegate& host, void*)
{
	if(!host.done)
	{
		if(ASTNumberLiteral* lit = dynamic_cast<ASTNumberLiteral*>(host.operand.get()))
		{
			lit->negate();
			host.done = true;
		}
	}
	analyzeUnaryExpr(host, DataType::FLOAT);
}
void SemanticAnalyzer::caseExprDelete(ASTExprDelete& host, void*)
{
	visit(host.operand.get());
	if (breakRecursion(host)) return;
	
	DataType const* optype = host.operand->getReadType(scope, this);
	if(!optype->isUsrClass())
	{
		handleError(CompileError::BadDelete(&host));
	}
}

void SemanticAnalyzer::caseExprNot(ASTExprNot& host, void*)
{
	analyzeUnaryExpr(host, DataType::UNTYPED);
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

void SemanticAnalyzer::caseExprCast(ASTExprCast& host, void* param)
{
	RecursiveVisitor::caseExprCast(host);
	DataType const& castType = host.type->resolve(*scope, this);
	if(!castType.isResolved())
	{
		handleError(
				CompileError::UnresolvedType(
						&host, castType.getName()));
		return;
	}
	if(castType.isAuto())
	{
		handleError(CompileError::IllegalCast(&host, "anything", "auto"));
		return;
	}
}

void SemanticAnalyzer::caseExprAnd(ASTExprAnd& host, void*)
{
	analyzeBinaryExpr(host, DataType::UNTYPED, DataType::UNTYPED);
}

void SemanticAnalyzer::caseExprOr(ASTExprOr& host, void*)
{
	analyzeBinaryExpr(host, DataType::UNTYPED, DataType::UNTYPED);
}

void SemanticAnalyzer::caseExprXOR(ASTExprXOR& host, void*)
{
	analyzeBinaryExpr(host, DataType::UNTYPED, DataType::UNTYPED);
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

	checkCast(*host.right->getReadType(scope, this), *host.left->getReadType(scope, this), &host, true);
	if (breakRecursion(host)) return;
}

void SemanticAnalyzer::caseExprNE(ASTExprNE& host, void*)
{
	RecursiveVisitor::caseExprNE(host);
	if (breakRecursion(host)) return;

	checkCast(*host.right->getReadType(scope, this), *host.left->getReadType(scope, this), &host, true);
	if (breakRecursion(host)) return;
}

void SemanticAnalyzer::caseExprAppxEQ(ASTExprAppxEQ& host, void*)
{
	analyzeBinaryExpr(host, DataType::FLOAT, DataType::FLOAT);
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

void SemanticAnalyzer::caseExprExpn(ASTExprExpn& host, void*)
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
	checkCast(*host.left->getReadType(scope, this), DataType::UNTYPED, &host);
	if (breakRecursion(host)) return;
	
	visit(host.middle.get());
	if (breakRecursion(host)) return;
	visit(host.right.get());
	if (breakRecursion(host)) return;
	checkCast(*host.middle->getReadType(scope, this), *host.right->getReadType(scope, this), &host);
	if (breakRecursion(host)) return;
	checkCast(*host.right->getReadType(scope, this), *host.middle->getReadType(scope, this), &host);
	if (breakRecursion(host)) return;
}

// Literals

void SemanticAnalyzer::caseStringLiteral(ASTStringLiteral& host, void*)
{
	if(host.registered()) return; //Skip if already handled
	// Add to scope as a managed literal.
	Literal::create(*scope, host, *host.getReadType(scope, this), this);
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
		checkCast(*host.size->getReadType(scope, this), DataType::FLOAT,
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
		DataType const& elementType = host.type->resolve(*scope, this);
		if (breakRecursion(*host.type.get())) return;
		if (!elementType.isResolved())
		{
			handleError(
					CompileError::UnresolvedType(
							&host, elementType.getName()));
			return;
		}

		// Disallow void/auto type.
		if (elementType.isVoid() || elementType.isAuto())
		{
			handleError(CompileError::BadArrType(&host, host.asString(), elementType.getName()));
			return;
		}

		// Convert to array type.
		host.setReadType(
				program.getTypeStore().getCanonicalType(
						DataTypeArray(elementType)));
	}

	// Otherwise, default to Untyped -Em
	else
	{
		host.setReadType(
				program.getTypeStore().getCanonicalType(
						DataTypeArray(DataType::UNTYPED)));
	}

	// If initialized, check that each element can be cast to type.
	// and, if part of a declaration, check for temp values
	for (vector<ASTExpr*>::iterator it = host.elements.begin();
		 it != host.elements.end(); ++it)
	{
		ASTExpr& element = **it;
		if(host.declaration && element.isTempVal())
		{
			handleError(CompileError::BadTempVal(&element));
			if (breakRecursion(host)) return;
		}
		checkCast(*element.getReadType(scope, this),
				  host.getReadType(scope, this)->getElementType(), &host);
		if (breakRecursion(host)) return;
	}
	
	// Add to scope as a managed literal.
	Literal::create(*scope, host, *host.getReadType(scope, this), this);
}

void SemanticAnalyzer::caseOptionValue(ASTOptionValue& host, void*)
{
	/* handled in `ASTOptionValue->getCompileTimeValue()` now
	if (std::optional<int32_t> value = lookupOption(*scope, host.option))
		host.value = value;
	else
		handleError(CompileError::UnknownOption(&host, host.name));*/
}

void SemanticAnalyzer::caseIsIncluded(ASTIsIncluded& host, void*)
{}

void SemanticAnalyzer::analyzeUnaryExpr(
		ASTUnaryExpr& host, DataType const& type)
{
	visit(host.operand.get());
	if (breakRecursion(host)) return;
	
	checkCast(*host.operand->getReadType(scope, this), type, &host);
	if (breakRecursion(host)) return;
}

void SemanticAnalyzer::analyzeIncrement(ASTUnaryExpr& host)
{
	visit(host.operand.get(), paramReadWrite);
    if (breakRecursion(host)) return;

	ASTExpr& operand = *host.operand;
    checkCast(*operand.getReadType(scope, this), DataType::FLOAT, &host);
    if (breakRecursion(host)) return;
}

void SemanticAnalyzer::analyzeBinaryExpr(
		ASTBinaryExpr& host, DataType const& leftType,
		DataType const& rightType)
{
	visit(host.left.get());
	if (breakRecursion(host)) return;
	checkCast(*host.left->getReadType(scope, this), leftType, &host);
	if (breakRecursion(host)) return;

	visit(host.right.get());
	if (breakRecursion(host)) return;
	checkCast(*host.right->getReadType(scope, this), rightType, &host);
	if (breakRecursion(host)) return;
}


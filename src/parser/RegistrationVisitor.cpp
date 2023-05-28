/**
 * Registration handler, to ensure that anything global can be referenced in any order
 * Begun: 26th June, 2019
 * Author: Emily
 */

#include "parserDefs.h"
#include "RegistrationVisitor.h"
#include <cassert>
#include "Scope.h"
#include "CompileError.h"

#include "zc/ffscript.h"
extern FFScript FFCore;
using std::string;
using std::vector;
using namespace ZScript;
using std::unique_ptr;
////////////////////////////////////////////////////////////////
// RegistrationVisitor

RegistrationVisitor::RegistrationVisitor(Program& program)
	: program(program), hasChanged(false)
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

template <class Container>
void RegistrationVisitor::regvisit(AST& host, Container const& nodes, void* param)
{
	for (auto it = nodes.cbegin();
		 it != nodes.cend(); ++it)
	{
		if (breakRecursion(host, param)) return;
		visit(**it, param);
	}
}
template <class Container>
void RegistrationVisitor::block_regvisit(AST& host, Container const& nodes, void* param)
{
	for (auto it = nodes.cbegin();
		 it != nodes.cend(); ++it)
	{
		failure_temp = false;
		visit(**it, param);
		if(failure_halt) return;
	}
}

void RegistrationVisitor::caseDefault(AST& host, void* param)
{
	doRegister(host);
}

//Handle the root file specially!
void RegistrationVisitor::caseRoot(ASTFile& host, void* param)
{
	int32_t recursionLimit = REGISTRATION_REC_LIMIT;
	while(--recursionLimit)
	{
		caseFile(host, param);
		if(registered(host)) return;
		if(!hasChanged) return; //Nothing new was registered on this pass. Only errors remain.
		hasChanged = false;
	}
	//Failed recursionLimit
	handleError(CompileError::RegistrationRecursion(&host, REGISTRATION_REC_LIMIT));
}

void RegistrationVisitor::caseFile(ASTFile& host, void* param)
{
	if(host.scope)
		scope = host.scope;
	else
		scope = host.scope = scope->makeFileChild(host.asString());
	block_regvisit(host, host.options, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.use, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.dataTypes, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.scriptTypes, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.imports, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.condimports, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.variables, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.functions, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.namespaces, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.scripts, param);
	if (breakRecursion(host, param)) return;
	block_regvisit(host, host.classes, param);
	if(registered(host, host.options) && registered(host, host.use) && registered(host, host.dataTypes)
		&& registered(host, host.scriptTypes) && registered(host, host.imports) && registered(host, host.variables)
		&& registered(host, host.functions) && registered(host, host.namespaces) && registered(host, host.scripts)
		&& registered(host, host.condimports) && registered(host, host.classes))
	{
		doRegister(host);
	}
	scope = scope->getParent();
}

void RegistrationVisitor::caseSetOption(ASTSetOption& host, void* param)
{
	visit(host.expr.get(), param);
	if(!registered(host.expr.get())) return; //Non-initialized constant
	
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
		doRegister(host);
		return;
	}

	// Set the option to the provided value.
	CompileOptionSetting setting = host.getSetting(this, scope);
	if (!setting) return; // error
	scope->setOption(host.option, setting);
	doRegister(host);
}

// Declarations
void RegistrationVisitor::caseScript(ASTScript& host, void* param)
{
	visit(host.type.get());
	if(!registered(host.type.get())) return;
	
	Script& script = host.script ? *host.script : *(host.script = program.addScript(host, *scope, this));
	if (breakRecursion(host)) return;
	
	string name = script.getName();

	// Recurse on script elements with its scope.
	scope = &script.getScope();
	block_regvisit(host, host.options, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.use, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.types, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.variables, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.functions, param);
	scope = scope->getParent();
	if (breakRecursion(host)) return;
	//
	if(!(registered(host, host.options) && registered(host, host.use) && registered(host, host.types)
		&& registered(host, host.variables) && registered(host, host.functions)))
	{
		return;
	}
	
	if(script.getType() == ScriptType::untyped)
	{
		doRegister(host);
		return;
	}
	//
	// Check for a valid run function.
	vector<Function*> possibleRuns =
		//script.getScope().getLocalFunctions("run");
		script.getScope().getLocalFunctions(FFCore.scriptRunString);
	if (possibleRuns.size() == 0)
	{
		return; //Don't register
		//handleError(CompileError::ScriptNoRun(&host, name, FFCore.scriptRunString));
		//if (breakRecursion(host)) return;
	}
	doRegister(host);
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
	script.setRun(possibleRuns[0]);
}

void RegistrationVisitor::caseClass(ASTClass& host, void* param)
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
			doRegister(host);
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
	block_regvisit(host, host.options, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.use, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	block_regvisit(host, host.types, param);
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	parsing_user_class = puc_vars;
	block_regvisit(host, host.variables, param);
	parsing_user_class = puc_none;
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	parsing_user_class = puc_funcs;
	block_regvisit(host, host.functions, param);
	parsing_user_class = puc_none;
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	parsing_user_class = puc_construct;
	block_regvisit(host, host.constructors, param);
	parsing_user_class = puc_none;
	if (breakRecursion(host, param)) {scope = scope->getParent(); return;}
	parsing_user_class = puc_destruct;
	visit(host.destructor.get(), param);
	parsing_user_class = puc_none;
	scope = scope->getParent();
	if (breakRecursion(host)) return;
	//
	if(!(registered(host, host.options) && registered(host, host.use) && registered(host, host.types)
		&& registered(host, host.variables) && registered(host, host.functions)
		&& registered(host, host.constructors) && registered(host.destructor.get())))
	{
		return;
	}
	
	doRegister(host);
}

void RegistrationVisitor::caseNamespace(ASTNamespace& host, void* param)
{
	Namespace& namesp = host.namesp ? *host.namesp : (*(host.namesp = program.addNamespace(host, *scope, this)));
	if (breakRecursion(host)) return;

	// Recurse on script elements with its scope.
	// Namespaces' parent scope is RootScope*, not FileScope*. Store the FileScope* temporarily.
	Scope* temp = scope;
	scope = &namesp.getScope();
	block_regvisit(host, host.options, param);
	if (breakRecursion(host, param)) {scope = temp; return;}
	block_regvisit(host, host.dataTypes, param);
	if (breakRecursion(host, param)) {scope = temp; return;}
	block_regvisit(host, host.scriptTypes, param);
	if (breakRecursion(host, param)) {scope = temp; return;}
	block_regvisit(host, host.use, param);
	if (breakRecursion(host, param)) {scope = temp; return;}
	block_regvisit(host, host.variables, param);
	if (breakRecursion(host, param)) {scope = temp; return;}
	block_regvisit(host, host.functions, param);
	if (breakRecursion(host, param)) {scope = temp; return;}
	block_regvisit(host, host.namespaces, param);
	if (breakRecursion(host, param)) {scope = temp; return;}
	block_regvisit(host, host.scripts, param);
	if (breakRecursion(host, param)) {scope = temp; return;}
	block_regvisit(host, host.classes, param);
	scope = temp;
	if(registered(host, host.options) && registered(host, host.use) && registered(host, host.dataTypes)
		&& registered(host, host.scriptTypes) && registered(host, host.variables) && registered(host, host.functions)
		&& registered(host, host.namespaces) && registered(host, host.scripts))
	{
		doRegister(host);
	}
}

void RegistrationVisitor::caseImportDecl(ASTImportDecl& host, void* param)
{
	//Check if the import is valid, or to be stopped by header guard. -V
	if(getRoot(*scope)->checkImport(&host, *lookupOption(*scope, CompileOption::OPT_HEADER_GUARD) / 10000.0, this))
	{
		visit(host.getTree(), param);
		if(registered(host.getTree())) doRegister(host);
	}
}
void RegistrationVisitor::caseIncludePath(ASTIncludePath& host, void* param)
{}

void RegistrationVisitor::caseImportCondDecl(ASTImportCondDecl& host, void* param)
{
	visit(*host.cond, param);
	if(!registered(*host.cond)) return; //Not registered yet
	std::optional<int32_t> val = host.cond->getCompileTimeValue(this, scope);
	if(val && (*val != 0))
	{
		if(!host.preprocessed)
		{
			ScriptParser::preprocess_one(*host.import, ScriptParser::recursionLimit);
			host.preprocessed = true;
		}
		visit(*host.import, param);
		if(!registered(*host.import)) return;
	}
	doRegister(host);
}

void RegistrationVisitor::caseUsing(ASTUsingDecl& host, void* param)
{
	//Handle adding scope
	ASTExprIdentifier* iden = host.getIdentifier();
	Scope* temp = host.always ? getRoot(*scope) : scope;
	int32_t numMatches = temp->useNamespace(iden->components, iden->delimiters, iden->noUsing);
	if(numMatches == 0) //Quit before registering; check again later
		return;
	doRegister(host);
	if(numMatches > 1)
		handleError(CompileError::TooManyUsing(&host, iden->asString()));
	else if(numMatches == -1)
		handleError(CompileError::DuplicateUsing(&host, iden->asString()));
}

void RegistrationVisitor::caseDataTypeDef(ASTDataTypeDef& host, void* param)
{
	visit(host.type.get());
	if (breakRecursion(*host.type.get())) return;
	if(!registered(host.type.get())) return;
	doRegister(host);
	// Add type to the current scope under its new name.
	DataType const& type = host.type->resolve(*scope, this);
	if(!scope->addDataType(host.name, &type, &host))
	{
		unique_ptr<ASTExprIdentifier> temp(new ASTExprIdentifier(host.name, host.location));
		DataType const* originalType = lookupDataType(*scope, *temp, this, true);
		if (breakRecursion(host) || !originalType || (*originalType != type))
			handleError(
				CompileError::RedefDataType(
					&host, host.name));
		temp.reset();
	}
}

void RegistrationVisitor::caseCustomDataTypeDef(ASTCustomDataTypeDef& host, void* param)
{
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
			doRegister(host);
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
	visit(host.definition.get());
	if(registered(host.definition.get())) doRegister(host);
}

void RegistrationVisitor::caseScriptTypeDef(ASTScriptTypeDef& host, void* param)
{
	// Resolve the base type under current scope.
	ScriptType type = resolveScriptType(*host.oldType, *scope);
	if (!type.isValid()) return;

	doRegister(host);
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

void RegistrationVisitor::caseDataDeclList(ASTDataDeclList& host, void* param)
{
	// Resolve the base type.
	DataType const& baseType = host.baseType->resolve(*scope, this);
    if (breakRecursion(*host.baseType.get())) return;
	if (!host.baseType->wasResolved()) return;

	// Don't allow void type.
	if (baseType == DataType::ZVOID)
	{
		handleError(CompileError::VoidVar(&host, host.asString()));
		doRegister(host);
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !baseType.canBeGlobal())
	{
		handleError(CompileError::RefVar(&host, baseType.getName()));
		doRegister(host);
		return;
	}

	// Recurse on list contents.
	visit(host, host.getDeclarations());
	if (breakRecursion(host)) return;
	if(registered(host, host.getDeclarations())) doRegister(host);
}

void RegistrationVisitor::caseDataEnum(ASTDataEnum& host, void* param)
{
	// Resolve the base type.
	DataType const& baseType = host.baseType->resolve(*scope, this);
    if (breakRecursion(*host.baseType.get())) return;
	if (!baseType.isResolved()) return;

	// Don't allow void type.
	if (baseType == DataType::ZVOID)
	{
		handleError(CompileError::VoidVar(&host, host.asString()));
		doRegister(host);
		return;
	}

	// Check for disallowed global types.
	if (scope->isGlobal() && !baseType.canBeGlobal())
	{
		handleError(CompileError::RefVar(&host, baseType.getName()));
		doRegister(host);
		return;
	}

	//Handle initializer assignment
	int32_t ipart = -1, dpart = 0;
	std::vector<ASTDataDecl*> decls = host.getDeclarations();
	for(vector<ASTDataDecl*>::iterator it = decls.begin();
		it != decls.end(); ++it)
	{
		ASTDataDecl* declaration = *it;
		if(ASTExpr* init = declaration->getInitializer())
		{
			visit(init);
			if(!registered(init)) return;
			if(std::optional<int32_t> v = init->getCompileTimeValue(this, scope))
			{
				int32_t val = *v;
				ipart = val/10000;
				dpart = val%10000;
			}
			else return;
		}
		else
		{
			ASTNumberLiteral* value = new ASTNumberLiteral(new ASTFloat(++ipart, dpart, host.location), host.location);
			declaration->setInitializer(value);
		}
		visit(declaration, param);
		if(breakRecursion(host, param))
		{
			if(registered(declaration)) doRegister(host); //Decl errored, but registered; fatal error
			return;
		}
	}
	if(registered(host, host.getDeclarations())) doRegister(host);
}

void RegistrationVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	// First do standard recursing.
	RecursiveVisitor::caseDataDecl(host, paramRead);
	if (breakRecursion(host)) return;
	if(!(registered(host, host.extraArrays) && (!host.getInitializer() || registered(host.getInitializer())))) return;
	// Then resolve the type.
	DataType const& type = *host.resolveType(scope, this);
	if (breakRecursion(host)) return;
	if (!type.isResolved()) return;
	
	doRegister(host);

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
	if (type.isConstant())
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
		Constant::create(*scope, host, type, value, this);
	}
	else
	{
		if(parsing_user_class == puc_vars)
		{
			UserClassVar::create(*scope, host, type, this);
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
	}
}

void RegistrationVisitor::caseDataDeclExtraArray(ASTDataDeclExtraArray& host, void* param)
{
	// Type Check size expressions.
	RecursiveVisitor::caseDataDeclExtraArray(host);
	if (breakRecursion(host)) return;
	if(!registered(host, host.dimensions)) return;
	doRegister(host);
	
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
			if(*theSize < 1 || *theSize > 214748)
			{
				handleError(CompileError::ArrayInvalidSize(&host));
				return;
			}
		}
	}
}

void RegistrationVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
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
	if (!returnType.isResolved()) {scope = oldScope; return;}

	// Gather the parameter types.
	vector<DataType const*> paramTypes;
	vector<ASTDataDecl*> const& params = host.parameters.data();
	vector<string const*> paramNames;
	for (vector<ASTDataDecl*>::const_iterator it = params.begin();
		 it != params.end(); ++it)
	{
		ASTDataDecl& decl = **it;

		// Resolve the parameter type under current scope.
		DataType const& type = *decl.resolveType(scope, this);
		if (breakRecursion(decl)) {scope = oldScope; return;}
		if (!type.isResolved()) {scope = oldScope; return;}

		// Don't allow void params.
		if (type == DataType::ZVOID)
		{
			handleError(CompileError::FunctionVoidParam(&decl, decl.name));
			doRegister(host);
			scope = oldScope;
			return;
		}
		paramNames.push_back(new string(decl.name));
		paramTypes.push_back(&type);
	}
	if(host.prototype)
	{
		//Check the default return
		visit(host.defaultReturn.get(), param);
		if(breakRecursion(host.defaultReturn.get())) {scope = oldScope; return;}
		if(!(registered(host.defaultReturn.get()))) {scope = oldScope; return;}
		
		DataType const& defValType = *host.defaultReturn->getReadType(scope, this);
		if(!defValType.isResolved()) {scope = oldScope; return;}
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
		if(!getType) return;
		checkCast(*getType, *paramTypes[parcnt], &host);
		if(breakRecursion(host)) {scope = oldScope; return;}
		std::optional<int32_t> optVal = (*it)->getCompileTimeValue(this, scope);
		assert(optVal);
		host.optvals.push_back(*optVal);
	}
	if(breakRecursion(host)) {scope = oldScope; return;}
	
	doRegister(host);
	
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

// Expressions -- Needed for constant evaluation
void RegistrationVisitor::caseExprConst(ASTExprConst& host, void* param)
{
	RecursiveVisitor::caseExprConst(host, param);
	if (host.getCompileTimeValue(this, scope)) doRegister(host);
}

void RegistrationVisitor::caseVarInitializer(ASTExprVarInitializer& host, void* param)
{
	RecursiveVisitor::caseVarInitializer(host, param);
	if(registered(host.content.get()))
	{
		if(host.valueIsArray(scope, this)) doRegister(host);
		else
		{
			host.value = host.content->getCompileTimeValue(this, scope).value_or(0L); // sometimes has no value
			if(host.value) doRegister(host);
		}
	}
}

void RegistrationVisitor::caseExprAssign(ASTExprAssign& host, void* param)
{
	visit(host.left.get(), paramWrite);
	if (breakRecursion(host)) return;
	visit(host.right.get(), paramRead);
	if (breakRecursion(host)) return;	
	if(!(registered(host.left.get()) && registered(host.right.get()))) return;
	doRegister(host);
	DataType const* ltype = host.left->getWriteType(scope, this);
	if (!ltype)
	{
		handleError(
			CompileError::NoWriteType(
				host.left.get(), host.left->asString()));
		return;
	}
}

void RegistrationVisitor::caseExprIdentifier(ASTExprIdentifier& host, void* param)
{
	// Bind to named variable.
	host.binding = lookupDatum(*scope, host, this);
	if (!host.binding) return;
	doRegister(host);
	if(host.binding->type.isArray())
	{
		handleError(CompileError::NoArrayGlobalVar(&host));
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

void RegistrationVisitor::caseExprArrow(ASTExprArrow& host, void* param)
{
	//Doesn't get hit at registration time?
	doRegister(host);
}

void RegistrationVisitor::caseExprIndex(ASTExprIndex& host, void* param)
{
	visit(host.array.get());
	if (breakRecursion(host)) return;
	visit(host.index.get());
	if (breakRecursion(host)) return;
	if(registered(host.array.get()) && registered(host.index.get())) doRegister(host);
}

void RegistrationVisitor::caseExprCall(ASTExprCall& host, void* param)
{
	handleError(CompileError::GlobalVarFuncCall(&host));
}

void RegistrationVisitor::caseExprNegate(ASTExprNegate& host, void* param)
{
	if(!host.done)
	{
		if(ASTNumberLiteral* lit = dynamic_cast<ASTNumberLiteral*>(host.operand.get()))
		{
			if(lit->value.get())
			{
				lit->negate();
				host.done = true;
			}
		}
	}
	analyzeUnaryExpr(host);
}
void RegistrationVisitor::caseExprDelete(ASTExprDelete& host, void* param)
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

void RegistrationVisitor::caseExprAppxEQ(ASTExprAppxEQ& host, void* param)
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

void RegistrationVisitor::caseExprExpn(ASTExprExpn& host, void* param)
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
	if (breakRecursion(host)) return;
	visit(host.middle.get());
	if (breakRecursion(host)) return;
	visit(host.right.get());
	if (breakRecursion(host)) return;
	if(registered(host.left.get()) && registered(host.middle.get()) && registered(host.right.get())) doRegister(host);
}

//Types
void RegistrationVisitor::caseScriptType(ASTScriptType& host, void* param)
{
	ScriptType const& type = resolveScriptType(host, *scope);
	if(type.isValid()) doRegister(host);
}

void RegistrationVisitor::caseDataType(ASTDataType& host, void* param)
{
	DataType const& type = host.resolve(*scope, this);
	if(type.isResolved()) doRegister(host);
}

//Literals
void RegistrationVisitor::caseArrayLiteral(ASTArrayLiteral& host, void* param)
{
	RecursiveVisitor::caseArrayLiteral(host, param);
	if(registered(host.type.get()) && registered(host.size.get()) && registered(host, host.elements))
		doRegister(host);
}

void RegistrationVisitor::caseStringLiteral(ASTStringLiteral& host, void* param)
{
	// Add to scope as a managed literal.
	Literal::create(*scope, host, *host.getReadType(scope, this), this);
	doRegister(host);
}

//Helper Functions
void RegistrationVisitor::analyzeUnaryExpr(ASTUnaryExpr& host)
{
	visit(host.operand.get());
	if (breakRecursion(host)) return;
	if(registered(host.operand.get()))doRegister(host);
}

void RegistrationVisitor::analyzeBinaryExpr(ASTBinaryExpr& host)
{
	visit(host.left.get());
	if (breakRecursion(host)) return;
	visit(host.right.get());
	if (breakRecursion(host)) return;
	if((registered(host.left.get()) && registered(host.right.get()))) doRegister(host);
}

bool RegistrationVisitor::registered(AST& node) const
{
	return node.registered();
}

bool RegistrationVisitor::registered(AST* node) const
{
	if(node) return registered(*node);
	return true;
}

template <class Container>
bool RegistrationVisitor::registered(AST& host, Container const& nodes) const
{
	for(auto it = nodes.cbegin(); it != nodes.cend(); ++it)
	{
		if(!registered(*it)) return false;
	}
	return true;
}



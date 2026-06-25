/**
 * Registration handler, to ensure that anything global can be referenced in any order
 * Begun: 26th June, 2019
 * Author: Emily
 */

#include "parser/AST.h"
#include "parser/ASTVisitors.h"
#include "parser/ByteCode.h"
#include "parser/LibrarySymbols.h"
#include "parser/Types.h"
#include "parser/ZScript.h"
#include "parserDefs.h"
#include "RegistrationVisitor.h"
#include <bit>
#include <cassert>
#include <cstdint>
#include <sstream>
#include "Scope.h"
#include "CompileError.h"

#include "components/zasm/table.h"
#include "zc/ffscript.h"

using std::ostringstream;
using namespace ZScript;
using std::shared_ptr;
using std::unique_ptr;

int32_t StringToVar(std::string var);

////////////////////////////////////////////////////////////////
// RegistrationVisitor

RegistrationVisitor::RegistrationVisitor(Program& program)
	: RegBaseVisitor(program), hasChanged(false)
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
void RegistrationVisitor::block_regvisit_vec(Container const& nodes, void* param)
{
	for (auto it = nodes.cbegin();
		 it != nodes.cend(); ++it)
	{
		failure_temp = false;
		visit(**it, param);
		if(failure_halt) return;
	}
}

void RegistrationVisitor::caseDefault(AST& host, void*)
{
	doRegister(host);
}

//Handle the root file specially!
void RegistrationVisitor::caseRoot(ASTFile& host, void* param)
{
	// Visit every node until there nothing new is registered, or until we've tried for too long.
	//
	// Multiple passes are necessary to support recursive declarations like this:
	//
	//   const int A = B;
	//   const int B = 5;
	//   const int C = B;
	//
	// The first pass would error when visiting A, but in the next pass it would pass.
	// RegistrationVisitor::visit handles checking if a node should be looked at again.
	// Nodes that registered successfully call `doRegister()`.
	//
	// Errors that are potentially resolvable if a binding is defined later, will never error in this handler.
	// Instead, SemanticAnalyser will raise an error.
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
	ScopeReverter sr(&scope);
	if(host.scope)
		scope = host.scope;
	else
		scope = host.scope = scope->makeFileChild(host.asString());
	block_regvisit_vec(host.options, param);
	if (breakRecursion(host, param)) return;
	block_regvisit_vec(host.use, param);
	if (breakRecursion(host, param)) return;
	block_regvisit_vec(host.dataTypes, param);
	if (breakRecursion(host, param)) return;
	block_regvisit_vec(host.scriptTypes, param);
	if (breakRecursion(host, param)) return;
	block_regvisit_vec(host.imports, param);
	if (breakRecursion(host, param)) return;
	block_regvisit_vec(host.condimports, param);
	if (breakRecursion(host, param)) return;
	block_regvisit_vec(host.variables, param);
	if (breakRecursion(host, param)) return;
	block_regvisit_vec(host.functions, param);
	if (breakRecursion(host, param)) return;
	block_regvisit_vec(host.namespaces, param);
	if (breakRecursion(host, param)) return;
	block_regvisit_vec(host.scripts, param);
	if (breakRecursion(host, param)) return;
	block_regvisit_vec(host.classes, param);
	if(registered_vec(host.options) && registered_vec(host.use) && registered_vec(host.dataTypes)
		&& registered_vec(host.scriptTypes) && registered_vec(host.imports) && registered_vec(host.variables)
		&& registered_vec(host.functions) && registered_vec(host.namespaces) && registered_vec(host.scripts)
		&& registered_vec(host.condimports) && registered_vec(host.classes))
	{
		doRegister(host);
	}
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

	// Recurse on script elements with its scope.
	{
		ScopeReverter sr(&scope);
		scope = &script.getScope();

		block_regvisit_vec(host.options, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.use, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.types, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.variables, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.functions, param);
		if (breakRecursion(host)) return;
	}

	if(!(registered_vec(host.options) && registered_vec(host.use) && registered_vec(host.types)
		&& registered_vec(host.variables) && registered_vec(host.functions)))
	{
		return;
	}
	
	if(script.getType() == ParserScriptType::untyped)
	{
		doRegister(host);
		return;
	}
	//
	// Check for a valid run function.
	vector<Function*> possibleRuns = script.getScope().getLocalFunctions("run");
	if (possibleRuns.size() == 0)
	{
		return; //Don't register
	}
	doRegister(host);
	string name = script.getName();
	if (possibleRuns.size() > 1)
	{
		handleError(CompileError::TooManyRun(&host, name, "run"));
		return;
	}
	Function* runfunc = possibleRuns[0];
	AST* node = &host;
	if (runfunc->node)
		node = runfunc->node;
	if (*runfunc->returnType != DataType::ZVOID)
	{
		handleError(CompileError::ScriptRunNotVoid(node, name, "run"));
		return;
	}
	if (runfunc->getFlag(FUNCFLAG_STATIC))
	{
		handleError(CompileError::Error(node, "void run() functions cannot be static."));
		return;
	}
	script.setRun(possibleRuns[0]);
}

void RegistrationVisitor::caseClass(ASTClass& host, void* param)
{
	auto& parsed_comment = host.getParsedComment();
	UserClass* base_class = nullptr;
	if (auto parent_class_name = parsed_comment.get_tag("extends"))
	{
		ASTExprIdentifier ident{};
		ident.components.push_back(*parent_class_name);
		if (auto type = lookupDataType(*scope, ident, nullptr))
		{
			if (auto custom_type = dynamic_cast<const DataTypeCustom*>(type); custom_type)
				base_class = custom_type->getUsrClass();
		}
		else
		{
			handleError(CompileError::BadInternal(&host, fmt::format("Unknown class \"{}\" in @extends", *parent_class_name)));
		}
	}

	if (!host.user_class)
	{
		host.user_class = program.addClass(host, *scope, base_class, this);
		if (!host.user_class)
		{
			doRegister(host);
			return;
		}
	}

	UserClass& user_class = *host.user_class;

	if (auto zasm_ref = parsed_comment.get_tag("zasm_ref"))
	{
		user_class.internalRefVarString = *zasm_ref;
		user_class.internalRefVar = user_class.internalRefVarString.empty() ? NUL : StringToVar(user_class.internalRefVarString);
	}
	else
	{
		user_class.internalRefVar = NUL;
	}

	if (breakRecursion(host))
	{
		doRegister(host);
		return;
	}
	string name = user_class.getName();
	
	if(!host.type)
	{
		//Don't allow use of a name that already exists
		unique_ptr<ASTExprIdentifier> temp(new ASTExprIdentifier(name, host.location));
		if(lookupDataType(*scope, *temp, this, true))
		{
			handleError(
				CompileError::RedefDataType(&host, host.getName()));
			doRegister(host);
			return;
		}
		
		//Construct a new constant type
		DataTypeCustomConst* newConstType = new DataTypeCustomConst("const " + host.getName(), &user_class);
		newConstType->setSource(&host);
		//Construct the base type
		DataTypeCustom* newBaseType = new DataTypeCustom(host.getName(), newConstType, &user_class, newConstType->getCustomId());
		newBaseType->setSource(&host);
		
		//Set the type to the base type
		host.type.reset(new ASTDataType(newBaseType, host.location));
		
		DataType::addCustom(newBaseType);
		user_class.setType(newBaseType);
		
		//This call should never fail, because of the error check above.
		scope->addDataType(host.getName(), newBaseType, &host);
		if (breakRecursion(*host.type.get())) return;
		
		for (auto it = host.constructors.begin();
			 it != host.constructors.end(); ++it)
		{
			ASTFuncDecl* func = *it;
			func->returnType.reset(new ASTDataType(newBaseType, func->location));
		}
	}

	// Register class template parameters (ex: `class stack<T>`), so that
	// member function signatures can resolve them. The template types are bound
	// from the receiver's type at each call site (see lookupClassFuncs).
	if (!host.templates.empty() && user_class.template_types.empty())
	{
		if (user_class.internalRefVarString.empty())
		{
			handleError(CompileError::BadInternal(&host, "Class template parameters are only supported for internal classes"));
			return;
		}

		for (size_t q = 0; q < host.templates.size(); ++q)
		{
			string const& templ_name = host.templates[q]->getValue();
			user_class.template_types.emplace_back(DataTypeTemplate::create(templ_name));
			user_class.getScope().addDataType(templ_name, user_class.template_types.back().get(), nullptr);
		}
	}

	// Recurse on user_class elements with its scope.
	{
		ScopeReverter sr(&scope);
		scope = &user_class.getScope();

		block_regvisit_vec(host.options, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.use, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.types, param);
		if (breakRecursion(host, param)) return;
		parsing_user_class = puc_vars;
		block_regvisit_vec(host.variables, param);
		parsing_user_class = puc_none;
		if (breakRecursion(host, param)) return;
		parsing_user_class = puc_funcs;
		block_regvisit_vec(host.functions, param);
		parsing_user_class = puc_none;
		if (breakRecursion(host, param)) return;
		parsing_user_class = puc_construct;
		block_regvisit_vec(host.constructors, param);
		parsing_user_class = puc_none;
		if (breakRecursion(host, param)) return;

		parsing_user_class = puc_destruct;
		visit(host.destructor.get(), param);
		parsing_user_class = puc_none;
	}

	if (breakRecursion(host)) return;
	//
	if(!(registered_vec(host.options) && registered_vec(host.use) && registered_vec(host.types)
		&& registered_vec(host.variables) && registered_vec(host.functions)
		&& registered_vec(host.constructors) && registered(host.destructor.get())))
	{
		return;
	}

	ScopeReverter sr(&scope);
	scope = &user_class.getScope();

	for (auto node : host.variables)
	{
		if (node->internal)
			initInternalVar(node);
	}

	for (auto fn_decl : host.functions)
	{
		scope->initFunctionBinding(fn_decl->func, this);
		
		if (!fn_decl->isBinding())
			continue;

		if (user_class.internalRefVarString.empty())
			fn_decl->func->setIntFlag(IFUNCFLAG_SKIPPOINTER);
	}

	for (auto fn_decl : host.constructors)
	{
		scope->initFunctionBinding(fn_decl->func, this);
		
		if (!fn_decl->isBinding())
			continue;

		if (fn_decl->func->getFlag(FUNCFLAG_NIL))
		{
			scope->removeFunction(fn_decl->func);
			fn_decl->func = nullptr;
		}
	}

	doRegister(host);
}

void RegistrationVisitor::caseNamespace(ASTNamespace& host, void* param)
{
	Namespace* namesp_ = host.namesp ? host.namesp : ((host.namesp = program.addNamespace(host, *scope, this)));
	if (!namesp_) return;
	Namespace& namesp = *namesp_;
	if (breakRecursion(host)) return;

	// Options are set on an anonymous child scope of the namespace,
	// which is attached to any function made within this namespace.
	// See lookupOption.
	{
		ScopeReverter sr(&scope);
		lexical_options_scope = scope->makeChild();
		scope = lexical_options_scope;

		block_regvisit_vec(host.options, param);
		if (breakRecursion(host, param)) return;
	}

	// Recurse on script elements with its scope.
	{
		ScopeReverter sr(&scope);
		scope = &namesp.getScope();

		block_regvisit_vec(host.dataTypes, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.scriptTypes, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.use, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.variables, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.functions, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.namespaces, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.scripts, param);
		if (breakRecursion(host, param)) return;
		block_regvisit_vec(host.classes, param);
	}

	if(registered_vec(host.options) && registered_vec(host.use) && registered_vec(host.dataTypes)
		&& registered_vec(host.scriptTypes) && registered_vec(host.variables) && registered_vec(host.functions)
		&& registered_vec(host.namespaces) && registered_vec(host.scripts))
	{
		doRegister(host);
	}

	lexical_options_scope = nullptr;
}

void RegistrationVisitor::caseImportDecl(ASTImportDecl& host, void* param)
{
	if(getRoot(*scope)->checkImport(&host, this))
	{
		visit(host.getTree(), param);
		if(registered(host.getTree())) doRegister(host);
	}
}
void RegistrationVisitor::caseIncludePath(ASTIncludePath&, void*)
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
			ScriptParser::legacy_preprocess_one(*host.import, ScriptParser::recursionLimit);
			host.preprocessed = true;
		}
		visit(*host.import, param);
		if(!registered(*host.import)) return;
	}
	doRegister(host);
}

void RegistrationVisitor::caseUsing(ASTUsingDecl& host, void*)
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

void RegistrationVisitor::caseDataTypeDef(ASTDataTypeDef& host, void*)
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

void RegistrationVisitor::caseCustomDataTypeDef(ASTCustomDataTypeDef& host, void*)
{
	if(!host.type)
	{
		//Don't allow use of a name that already exists
		unique_ptr<ASTExprIdentifier> temp(new ASTExprIdentifier(host.name, host.location));
		if(lookupDataType(*scope, *temp, this, true))
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
		newConstType->setSource(&host);
		//Construct the base type
		DataTypeCustom* newBaseType = new DataTypeCustom(host.name, newConstType, nullptr, newConstType->getCustomId());
		newBaseType->setSource(&host);
		
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

void RegistrationVisitor::caseScriptTypeDef(ASTScriptTypeDef& host, void*)
{
	// Resolve the base type under current scope.
	ParserScriptType type = resolveScriptType(*host.oldType, *scope);
	if (!type.isValid()) return;

	doRegister(host);
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

void RegistrationVisitor::caseDataDeclList(ASTDataDeclList& host, void*)
{
	// Resolve the base type.
	DataType const* baseType = host.baseType->resolve_ornull(*scope, this);
    if (breakRecursion(*host.baseType.get())) return;
	if (!host.baseType->wasResolved() || !baseType) return;

	// Don't allow void type.
	if (baseType->isVoid())
	{
		handleError(CompileError::BadVarType(&host, host.asString(), baseType->getName()));
		doRegister(host);
		return;
	}
	
	if (baseType->isAuto() && host.getDeclarations().size() > 1)
	{
		handleError(CompileError::GroupAuto(&host));
		return;
	}
	
	if (host.was_exported && host.getDeclarations().size() > 1)
	{
		handleError(CompileError::Error(&host, "@Export() can't be used on multi-variable declarations!"));
		return;
	}
	
	if (!host.handled_staticness)
	{
		handle_staticness(&host, host.is_static, host.is_nonstatic, scope, false);
		host.handled_staticness = true;
	}

	if (breakRecursion(host)) return;
	// Recurse on list contents.
	visit_vec(host.getDeclarations());
	if (breakRecursion(host)) return;
	if(registered_vec(host.getDeclarations())) doRegister(host);
}

void RegistrationVisitor::caseDataEnum(ASTDataEnum& host, void* param)
{
	// Resolve the base type.
	DataType const* baseType = host.baseType->resolve_ornull(*scope, this);
    if (breakRecursion(*host.baseType.get())) return;
	if (!baseType) return;
	
	host.is_static = !scope->getFunctionScope();
	
	// Don't allow void/auto types.
	if (baseType->isVoid() || baseType->isAuto())
	{
		handleError(CompileError::BadVarType(&host, host.asString(), baseType->getName()));
		doRegister(host);
		return;
	}

	//Handle initializer assignment
	zfix value = 0;
	auto bitmode = host.getBitMode();
	switch(bitmode)
	{
		case ASTDataEnum::BIT_INT:
			value = 1;
			break;
		case ASTDataEnum::BIT_LONG:
			value = 0.0001_zf;
			break;
	}
	bool is_first = true;
	std::vector<ASTDataDecl*> decls = host.getDeclarations();
	for(vector<ASTDataDecl*>::iterator it = decls.begin();
		it != decls.end(); ++it)
	{
		ASTDataDecl* declaration = *it;
		if(ASTExpr* init = declaration->getInitializer())
		{
			scope->in_static_init = host.is_static;
			visit(init);
			std::optional<int32_t> v;
			if(registered(init))
				v = init->getCompileTimeValue(this, scope);
			scope->in_static_init = false;
			if (v)
				value = zslongToFix(*v);
			else return;
		}
		else
		{
			if(!is_first)
			{
				if(host.increment_val)
					value += *host.increment_val;
				else if(bitmode)
				{
					if (value == 0)
						value = bitmode == ASTDataEnum::BIT_INT ? 1_zf : 0.0001_zf;
					else
						value *= 2;
					uint32_t value_to_check = bitmode == ASTDataEnum::BIT_INT ? value.getInt() : value.getZLong();
					if (!std::has_single_bit(value_to_check))
					{
						handleError(CompileError::Error(declaration,
							fmt::format("Auto-assigned values for bitflags members must be a power-of-two, but got: {}\n{}",
							value,
							"Either change the previous member to be a power-of-two, or explicitly initialize this member.")));
						doRegister(host);
						return;
					}
				}
				else if(baseType->isLong())
					value += 0.0001_zf;
				else value += 1;
			}
			ASTNumberLiteral* lit = new ASTNumberLiteral(new ASTFloat(value.getTrunc(), value.getZLongDPart(), host.location), host.location);
			declaration->setInitializer(lit);
		}
		is_first = false;
		visit(declaration, param);
		if(breakRecursion(host, param))
		{
			if(registered(declaration)) doRegister(host); //Decl errored, but registered; fatal error
			return;
		}
	}
	if(registered_vec(host.getDeclarations())) doRegister(host);
}

void RegistrationVisitor::caseDataDecl(ASTDataDecl& host, void*)
{
	// First do standard recursing.
	RecursiveVisitor::caseDataDecl(host, paramRead);
	if (breakRecursion(host)) return;
	if(!(registered_vec(host.extraArrays) && (!host.getInitializer() || registered(host.getInitializer())))) return;
	// Then resolve the type.
	DataType const* type = host.resolve_ornull(scope, this);
	if (breakRecursion(host)) return;
	if (!type) return;
	auto* list = host.list;
	
	doRegister(host);
	
	handle_data_decl_registry(host);
}

void RegistrationVisitor::caseDataDeclExtraArray(ASTDataDeclExtraArray& host, void*)
{
	// Type Check size expressions.
	RecursiveVisitor::caseDataDeclExtraArray(host);
	if (breakRecursion(host)) return;
	if(!registered_vec(host.dimensions)) return;
	doRegister(host);
	
	// Iterate over sizes.
	for (vector<ASTExpr*>::const_iterator it = host.dimensions.begin();
		 it != host.dimensions.end(); ++it)
	{
		ASTExpr& size = **it;

		// Make sure each size can cast to float.
		if (!size.getReadType(scope, this)->canCastTo(DataType::FLOAT, scope))
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

void RegistrationVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	ScopeReverter sr(&scope);
	
	bool templated = !host.templates.empty();
	if(host.parentScope)
		scope = host.parentScope;
	else
	{
		if(host.identifier->components.size() > 1)
		{
			ASTExprIdentifier const& id = *(host.identifier);
			
			vector<string> scopeNames(id.components.begin(), id.components.end() - 1);
			vector<string> scopeDelimiters(id.delimiters.begin(), id.delimiters.end());
			host.parentScope = lookupScope(*scope, scopeNames, scopeDelimiters, id.noUsing, host, this);
			if(!host.parentScope)
			{
				return;
			}
			scope = host.parentScope;
		}
		else host.parentScope = scope;
		// Add an extra anonymous scope, used by templates
		host.parentScope = scope = scope->makeChild();
		scope->lexical_options_scope = lexical_options_scope;
	}
	Scope* extern_scope = scope;
	Scope* func_lives_in = scope->getParent();
	
	if(host.getFlag(FUNCFLAG_INVALID))
	{
		handleError(CompileError::BadFuncModifiers(&host, host.invalidMsg));
		return;
	}

	if (host.getFlag(FUNCFLAG_CONSTEXPR) && !host.isBinding())
	{
		host.invalidMsg += " `constexpr` is currently only allowed for internal function bindings.";
		handleError(CompileError::BadFuncModifiers(&host, host.invalidMsg));
		return;
	}

	/* This option is being disabled for now, as inlining of user functions is being disabled -V
	if(*lookupOption(*scope, CompileOption::OPT_FORCE_INLINE)
		&& !host.isRun())
	{
		host.setFlag(FUNCFLAG_INLINE);
	}*/
	
	if(templated && host.template_types.empty())
	{
		for(auto& ptr : host.templates)
		{
			string const& name = ptr->getValue();
			host.template_types.emplace_back(DataTypeTemplate::create(name));
			scope->addDataType(name, host.template_types.back().get(), nullptr);
		}
		host.param_template = host.parameters; //copy the pre-initialized params
	}

	// Member functions of a templated class implicitly take the class's template
	// types. They are bound from the receiver's type at each call site, rather
	// than inferred from arguments (see lookupClassFuncs).
	if (ClassScope* c_scope = scope->getClassScope())
	{
		UserClass& containing_class = c_scope->user_class;
		if (!containing_class.template_types.empty())
		{
			if (templated)
			{
				handleError(CompileError::BadInternal(&host, "Functions of a template class cannot have their own template parameters"));
				return;
			}
			if (host.template_types.empty())
				host.template_types = containing_class.template_types;
		}
	}
	
	// Resolve the return type under current scope.
	DataType const& returnType = host.returnType->resolve(*scope, this);
	if (breakRecursion(*host.returnType.get())) return;
	if (!returnType.isResolved()) return;
	if(returnType.isAuto())
	{
		handleError(CompileError::BadReturnType(&host, returnType.getName()));
		return;
	}

	// Gather the parameter types.
	vector<DataType const*> paramTypes;
	vector<ASTDataDecl*> const& params = host.parameters.data();
	vector<shared_ptr<const string>> paramNames;
	for (vector<ASTDataDecl*>::const_iterator it = params.begin();
		 it != params.end(); ++it)
	{
		ASTDataDecl& decl = **it;

		// Resolve the parameter type under current scope.
		DataType const* type = decl.resolve_ornull(scope, this);
		if (breakRecursion(decl)) return;
		if (!type) return;

		// Don't allow void/auto params.
		if (type->isVoid() || type->isAuto())
		{
			handleError(CompileError::FunctionBadParamType(&decl, decl.getName(), type->getName()));
			doRegister(host);
			return;
		}
		paramNames.emplace_back(new string(decl.getName()));
		paramTypes.push_back(type);
	}
	if(host.getFlag(FUNCFLAG_VARARGS) && !paramTypes.back()->isArray())
	{
		handleError(CompileError::BadVArgType(&host, paramTypes.back()->getName()));
		return;
	}
	if(host.prototype)
	{
		//Check the default return
		visit(host.defaultReturn.get(), param);
		if(breakRecursion(host.defaultReturn.get())) return;
		if(!(registered(host.defaultReturn.get()))) return;
		
		DataType const& defValType = *host.defaultReturn->getReadType(scope, this);
		if(!defValType.isResolved()) return;
		//Check type validity of default return
		if((*(host.defaultReturn->getCompileTimeValue(this, scope)) == 0) &&
			(defValType == DataType::CUNTYPED || defValType == DataType::UNTYPED))
		{
			//Default is null; don't check casting, as null needs to be valid even for things
			//that untyped does not normally cast to, such as VOID! -V
		}
		else checkCast(defValType, returnType, &host);
	}
	
	if(breakRecursion(host)) return;
	visit_vec(host.optparams, param);
	if(breakRecursion(host)) return;
	
	auto parcnt = paramTypes.size() - host.optparams.size();
	for(auto it = host.optparams.begin(); it != host.optparams.end() && parcnt < paramTypes.size(); ++it, ++parcnt)
	{
		DataType const* getType = (*it)->getReadType(scope, this);
		if(getType)
			checkCast(*getType, *paramTypes[parcnt], *it);
		if(breakRecursion(host)) return;
	}
	if(breakRecursion(host)) return;
	if (!host.handled_staticness)
	{
		bool is_static = host.getFlag(FUNCFLAG_STATIC);
		handle_staticness(&host, is_static, host.getFlag(FUNCFLAG_NONSTATIC), func_lives_in, host.isRun());
		host.setFlag(FUNCFLAG_STATIC, is_static);
		host.handled_staticness = true;
	}
	if(breakRecursion(host)) return;
	
	doRegister(host);
	
	// Add the function to the scope.
	Function* function = func_lives_in->addFunction(
			&returnType, host.getName(), paramTypes, paramNames, host.getFlags(), &host, this, extern_scope);
	host.func = function;
	
	if(breakRecursion(host)) return;
	// If adding it failed, it means this scope already has a function with
	// that name.
	if (function == NULL)
	{
		if(host.prototype) return; //Skip this error for prototype functions; error is handled inside 'addFunction()' above
		handleError(CompileError::FunctionRedef(&host, host.getName()));
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
			host.value = host.content->getCompileTimeValue(this, scope);
			if(host.value) doRegister(host);
		}
	}
}

void RegistrationVisitor::caseExprAssign(ASTExprAssign& host, void*)
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
	if (ltype->isConstant())
		handleError(CompileError::LValConst(&host, host.left->asString()));
}

void RegistrationVisitor::caseExprIdentifier(ASTExprIdentifier& host, void*)
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
}

void RegistrationVisitor::caseExprArrow(ASTExprArrow& host, void*)
{
	//Doesn't get hit at registration time?
	doRegister(host);
}

void RegistrationVisitor::caseExprIndex(ASTExprIndex& host, void*)
{
	visit(host.array.get());
	if (breakRecursion(host)) return;
	visit(host.index.get());
	if (breakRecursion(host)) return;
	if(registered(host.array.get()) && registered(host.index.get())) doRegister(host);
}

void RegistrationVisitor::caseExprCall(ASTExprCall& host, void*)
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
	
	bool r = true;
	// Don't visit left for identifier, since we don't want to bind to a
	// variable.
	if (!identifier)
	{
		visit(host.left.get(), paramNone);
		if (breakRecursion(host)) return;
		r = registered(host.left.get());
	}

	visit_vec(host.parameters);
	if (breakRecursion(host)) return;
	if(!(r && registered_vec(host.parameters)))
		return; //can't resolve yet

	UserClass* user_class = nullptr;
	DataType const* receiver_type = nullptr;
	// Gather parameter types.
	vector<DataType const*> parameterTypes;
	if (arrow)
	{
		DataType const* arrtype = arrow->left->getReadType(scope, this);
		if(!arrtype)
			return;
		if((user_class = arrtype->getUsrClass()))
			receiver_type = arrtype;
		else parameterTypes.push_back(arrtype);
	}
	for (vector<ASTExpr*>::const_iterator it = host.parameters.begin();
		 it != host.parameters.end(); ++it)
		parameterTypes.push_back((*it)->getReadType(scope, this));

	bool static_trimmed = false;
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
			// An explicit template argument, ex `new stack<int>()`.
			if (host.ctor_type_arg)
			{
				if (user_class->template_types.empty())
				{
					handleError(CompileError::BadInternal(&host, fmt::format("Class \"{}\" is not a template class", identifier->asString())));
					return;
				}
				DataType const& arg = host.ctor_type_arg->resolve(*scope, this);
				if (breakRecursion(*host.ctor_type_arg)) return;
				if (!arg.isResolved())
				{
					handleError(CompileError::UnresolvedType(&host, arg.getName()));
					return;
				}
				receiver_type = user_class->getTemplateInstance(arg.isConstant() ? arg.getMutType() : &arg);
			}
			functions = lookupConstructors(*user_class, parameterTypes, scope, receiver_type);
		}
		else
		{
			if(identifier->components.size() == 1 && parsing_user_class > puc_vars)
			{
				user_class = &scope->getClassScope()->user_class;
				if(parsing_user_class == puc_construct && identifier->components[0] == user_class->getName())
					functions = lookupConstructors(*user_class, parameterTypes, scope);
				if(!functions.size())
					functions = lookupFunctions(*scope, identifier->components[0], parameterTypes, identifier->noUsing, true, false, scope, &static_trimmed);
			}
			if(!functions.size())
				functions = lookupFunctions(*scope, identifier->components, identifier->delimiters, parameterTypes, identifier->noUsing, false, false, scope, &static_trimmed);
		}
	}
	else if(user_class)
	{
		functions = lookupClassFuncs(*user_class, arrow->right->getValue(), parameterTypes, scope, false, receiver_type);
	}
	else functions = lookupFunctions(arrow->leftClass->getScope(), arrow->right->getValue(), parameterTypes, true, false, false, scope, &static_trimmed); //Never `using` arrow functions

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
			for(Scope* current = (*it)->getInternalScope(); current; current = current->getParent())
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
			if (static_trimmed)
				handleError(CompileError::FuncNotStatic(&host, signature.asString()));
			else
				handleError(CompileError::NoFuncMatch(&host, signature.asString()));
		}
		else
		{
			// Sort to keep order same across platforms.
			std::sort(bestFunctions.begin(), bestFunctions.end(), [](Function* a, Function* b) {
				return a->id < b->id;
			});

			// Build list of function signatures.
			ostringstream oss;
			for (vector<Function*>::const_iterator it = bestFunctions.begin();
			     it != bestFunctions.end(); ++it)
			{
				oss << "        ";
				string namespacenames = "";
				for(Scope* current = (*it)->getInternalScope(); current; current = current->getParent())
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
	// `new stack<int>()` yields the instantiated type, not the base class type.
	if (host.isConstructor() && host.ctor_type_arg && receiver_type)
		host.binding->returnType = receiver_type;
	deprecWarn(host.binding, &host, "Function", host.binding->getUnaliasedSignature().asString());
	if(host.binding->getFlag(FUNCFLAG_READ_ONLY))
		handleError(CompileError::ReadOnly(&host, host.binding->getUnaliasedSignature().asString()));

	doRegister(host);
}

void RegistrationVisitor::caseExprNegate(ASTExprNegate& host, void*)
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
void RegistrationVisitor::caseExprDelete(ASTExprDelete& host, void*)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprNot(ASTExprNot& host, void*)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprBitNot(ASTExprBitNot& host, void*)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprIncrement(ASTExprIncrement& host, void*)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprDecrement(ASTExprDecrement& host, void*)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprCast(ASTExprCast& host, void*)
{
	analyzeUnaryExpr(host);
}

void RegistrationVisitor::caseExprAnd(ASTExprAnd& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprOr(ASTExprOr& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprGT(ASTExprGT& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprGE(ASTExprGE& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprLT(ASTExprLT& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprLE(ASTExprLE& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprEQ(ASTExprEQ& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprNE(ASTExprNE& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprAppxEQ(ASTExprAppxEQ& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprPlus(ASTExprPlus& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprMinus(ASTExprMinus& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprTimes(ASTExprTimes& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprExpn(ASTExprExpn& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprDivide(ASTExprDivide& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprModulo(ASTExprModulo& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprBitAnd(ASTExprBitAnd& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprBitOr(ASTExprBitOr& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprBitXor(ASTExprBitXor& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprLShift(ASTExprLShift& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprRShift(ASTExprRShift& host, void*)
{
	analyzeBinaryExpr(host);
}

void RegistrationVisitor::caseExprCoalesce(ASTExprCoalesce& host, void*)
{
	visit(host.left.get());
	if (breakRecursion(host)) return;
	visit(host.right.get());
	if (breakRecursion(host)) return;
	if(registered(host.left.get()) && registered(host.right.get())) doRegister(host);
}
void RegistrationVisitor::caseExprCoalesceAssign(ASTExprCoalesceAssign& host, void* param)
{
	caseExprCoalesce(host, param);
}

void RegistrationVisitor::caseExprTernary(ASTTernaryExpr& host, void*)
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
void RegistrationVisitor::caseScriptType(ASTScriptType& host, void*)
{
	ParserScriptType const& type = resolveScriptType(host, *scope);
	if(type.isValid()) doRegister(host);
}

void RegistrationVisitor::caseDataType(ASTDataType& host, void*)
{
	DataType const& type = host.resolve(*scope, this);
	if(type.isResolved()) doRegister(host);
}

//Literals
void RegistrationVisitor::caseArrayLiteral(ASTArrayLiteral& host, void* param)
{
	RecursiveVisitor::caseArrayLiteral(host, param);
	if(registered(host.type.get()) && registered(host.size.get()) && registered_vec(host.elements))
		doRegister(host);
}

void RegistrationVisitor::caseStringLiteral(ASTStringLiteral& host, void*)
{
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
bool RegistrationVisitor::registered_vec(Container const& nodes) const
{
	for(auto it = nodes.cbegin(); it != nodes.cend(); ++it)
	{
		if(!registered(*it)) return false;
	}
	return true;
}


void RegistrationVisitor::handle_staticness(AST* node, bool& is_static, bool is_nonstatic, Scope* target_scope, bool disallow)
{
	bool in_func = target_scope->getFunctionScope();
	bool in_script = !in_func && target_scope->getScriptScope();
	bool in_class = !in_func && target_scope->getClassScope();
	bool def_static = false;
	if (in_script)
	{
		switch (*lookupOption(*target_scope, CompileOption::OPT_DEFAULT_STATIC_SCRIPT_MEMBERS) / 10000)
		{
			case -1:
			case 1:
				def_static = !disallow;
				break;
		}
	}
	
	if (is_static && is_nonstatic)
		handleError(CompileError::Error(node, "Conflicting modifiers 'static' and 'nonstatic'"));
	
	if (is_nonstatic && (!in_script || disallow))
		handleError(CompileError::Error(node, "Unexpected modifier 'nonstatic'; not allowed here"));
	
	if (disallow)
	{
		if (is_static)
		{
			handleError(CompileError::Error(node, "Unexpected modifier 'static'; not allowed here"));
			is_static = false;
		}
	}
	else if (in_func)
	{
		if (is_static)
		{
			handleError(CompileError::Error(node, "Unexpected modifier 'static'; not allowed here"));
			is_static = false;
		}
	}
	else
	{
		if (!in_class && !in_script)
		{
			// Global-scope variables should not be marked static by the user
			// but SHOULD be marked static, so they get marked here.
			if (is_static)
			{
				handleError(CompileError::Error(node, "Unexpected modifier 'static'; not allowed here"));
				return;
			}
			else
				is_static = true;
		}
		else if (in_script && def_static)
		{
			is_static = !is_nonstatic;
		}
	}
}


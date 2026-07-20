#include "parser/AST.h"
#include "parser/ByteCode.h"
#include "parser/LibrarySymbols.h"
#include "parserDefs.h"
#include "RegBaseVisitor.h"
#include "Scope.h"
#include "CompileError.h"

#include "components/zasm/table.h"
#include "zc/ffscript.h"

using namespace ZScript;

RegBaseVisitor::RegBaseVisitor(Program& program)
	: RecursiveVisitor(program)
{}

void RegBaseVisitor::handle_data_decl_registry(ASTDataDecl& host)
{
	auto* list = host.list;
	DataType const* type = host.resolve_ornull(scope, this);
	assert(type);
	
	if (list && list->was_range_exported && !list->was_exported)
	{
		handleError(CompileError::ExportError(&host, "@ExportRange() requires @Export() to function!"));
		list->was_range_exported = false;
	}
	
	// Don't allow void type.
	if (type->isVoid())
	{
		handleError(CompileError::BadVarType(&host, host.getName(), type->getName()));
		return;
	}
	
	if (type->isAuto())
	{
		bool good = false;
		auto arr_depth = type->getArrayDepth();
		auto init = host.getInitializer();
		if(init)
		{
			auto readty = init->getReadType(scope, this);
			if(readty && readty->isResolved() && !readty->isVoid() && !readty->isAuto())
			{
				if(readty->getArrayDepth() < arr_depth)
				{
					handleError(CompileError::BadAutoType(&host, type->getName(), fmt::format("must have an initializer with type that is at least {}-depth array", arr_depth)));
					return;
				}
				if(type->isConstant())
					type = readty->getConstType();
				else if(!readty->isArray())
					type = readty->getMutType();
				else type = readty;
				host.setResolvedType(*type);
				good = true;
			}
		}
		if(!good)
		{
			handleError(CompileError::BadAutoType(&host, type->getName(), "must have an initializer with valid type to mimic."));
			return;
		}
	}
	

	bool is_static = host.is_static();
	bool is_classvar = parsing_user_class == puc_vars && !is_static;
	bool is_scriptvar = !is_static && scope->isScript();
	optional<int32_t> const_value;
	if (type->isConstant() && !host.getFlag(ASTDataDecl::FL_FORCE_VAR) && !(list && list->internal))
	{
		// A constant without an initializer doesn't make sense (unless it is internal).
		if (!host.getInitializer())
		{
			handleError(CompileError::ConstUninitialized(&host));
			return;
		}

		// Inline the constant if possible.
		scope->in_static_init = is_static;
		const_value = host.getInitializer()->getCompileTimeValue(this, scope);
		scope->in_static_init = false;
		
		//The dataType is constant, but the initializer is not. This is not allowed in static scopes, as it causes crashes. -V
		if (!const_value && is_static)
		{
			handleError(CompileError::ConstNotConstant(&host, host.getName()));
			return;
		}
		
		if (const_value && !(list && list->was_exported))
		{
			if (scope->getLocalDatum(host.getName()))
			{
				handleError(CompileError::VarRedef(&host, host.getName()));
				return;
			}
			
			Constant::create(*scope, host, *type, *const_value, this);
			return;
		}
	}
	
	if (list && list->was_exported)
	{
		if (is_static)
		{
			handleError(CompileError::ExportError(&host, "@Export() cannot be used on static variables!"));
			return;
		}
		if (!is_scriptvar)
		{
			handleError(CompileError::ExportError(&host, "@Export() can only be used on script-scope instance variables!"));
			return;
		}
		if (type->isArray())
		{
			handleError(CompileError::ExportError(&host, "@Export cannot be used on Arrays."));
			return;
		}
		
		bool isCompatible = false;
		if (auto const* simpleType = dynamic_cast<DataTypeSimple const*>(type))
		{
			auto id = simpleType->getId();
			if (id >= ZTID_PRIMITIVE_START && id <= ZTID_PRIMITIVE_END)
				isCompatible = true;
		}
		if (!isCompatible)
		{
			handleError(CompileError::ExportError(&host, fmt::format("@Export is incompatible with type '{}'", type->getName())));
			return;
		}
		
		if (type->isBool())
		{
			// Mark bool exports as checkboxes!
			list->export_data.min = 0_zf;
			list->export_data.max = 0.0001_zf;
			list->export_data.btn_type = nswapBOOL;
			
			if (list->was_range_exported)
			{
				handleError(CompileError::ExportError(&host, "@ExportRange() is incompatible with 'bool' variables!"));
				list->was_range_exported = false;
			}
		}
	}
	
	if (is_classvar)
	{
		if (host.getInitializer())
		{
			handleError(CompileError::ClassNoInits(&host, host.getName()));
			return;
		}
		
		UserClassVar::create(*scope, host, *type, this);
		for (auto alias : list->getParsedComment().get_multi_tag("alias"))
		{
			auto copy = host.clone();
			copy->identifier->setValue(alias);
			copy->list = list;
			copy->setFlag(ASTDataDecl::FL_HIDDEN, true);
			UserClassVar::create(*scope, *copy, *type, this);
		}
		for (auto alias : list->getParsedComment().get_multi_tag("deprecated_alias"))
		{
			auto copy = host.clone();
			copy->identifier->setValue(alias);
			copy->list = list;
			copy->setFlag(ASTDataDecl::FL_HIDDEN, true);
			UserClassVar::create(*scope, *copy, *type, this);
		}
		return;
	}
	
	if (is_scriptvar)
	{
		if (auto* init = host.getInitializer())
		{
			const_value = init->getCompileTimeValue(this, scope);
			if (!const_value)
			{
				handleError(CompileError::ExprNotConstant(init));
				return;
			}
		}
	}
	
	if (scope->getLocalDatum(host.getName()))
	{
		handleError(CompileError::VarRedef(&host, host.getName()));
		return;
	}
	
	if (list && list->internal)
	{
		auto ivar = InternalVariable::create(*scope, host, *type, this);
		initInternalVar(list);
		ivar->readfn = scope->getLocalGetter(host.getName());
		ivar->writefn = scope->getLocalSetter(host.getName());
		return;
	}
	
	auto* var = Variable::create(*scope, host, *type, this);
	if (is_scriptvar)
		scope->getScriptScope()->script.register_instance_var(var, const_value);
}


void RegBaseVisitor::initInternalVar(ASTDataDeclList* node)
{
	auto& parsed_comment = node->getParsedComment();

	int refvar = NUL;
	UserClass* user_class = nullptr;
	if (scope->isClass())
	{
		user_class = &scope->getClassScope()->user_class;
		refvar = user_class->internalRefVar;
	}

	for (auto decl : node->getDeclarations())
	{
		// Internal variables in classes must have a zasm_var.
		if (user_class && !parsed_comment.contains_tag("zasm_var"))
		{
			handleError(CompileError::BadInternal(decl, "Expected @zasm_var"));
			continue;
		}

		bool is_constant_zero = false;
		int fn_value;
		if (auto zasm_var = parsed_comment.get_tag("zasm_var"))
		{
			if (auto sv = get_script_variable(*zasm_var))
			{
				fn_value = *sv;
				if (auto v = dynamic_cast<InternalVariable*>(decl->manager))
					v->zasm_register = *sv;
				else if (auto v = dynamic_cast<UserClassVar*>(decl->manager))
					v->zasm_register = *sv;
			}
			else
			{
				handleError(CompileError::BadInternal(decl, fmt::format("Invalid ZASM register: {}", *zasm_var)));
				continue;
			}
		}
		else
		{
			is_constant_zero = true;
			fn_value = 0;
		}

		auto& ty = decl->manager->type;
		bool is_arr = ty.isArray();
		auto var_type = ty.baseType(*scope, nullptr);
		auto deprecated = parsed_comment.get_tag("deprecated");

		std::vector<std::string> names = {decl->getName()};
		for (auto alias : parsed_comment.get_multi_tag("alias"))
			names.push_back(alias);

		// Add getter(s).
		for (auto& name : names)
		{
			Function* fn;

			std::vector<const DataType*> params;
			if (user_class)
				params.push_back(user_class->getType());

			if (is_constant_zero)
			{
				fn = scope->addGetter(var_type, name, params, {}, 0);
				getConstant(refvar, fn, fn_value);
			}
			else if (is_arr)
			{
				fn = scope->addGetter(&ty, name, params, {}, 0);
				// `Screen` is special: normal usage doesn't use a ref variable explicity in the generated ZASM,
				// but when getting a reference to an array must save the current value of REFSCREEN. The
				// alternative is to use `@zasm_ref` on screendata but that's wasteful.
				getInternalArray(user_class->getName() == "screendata" ? REFSCREEN : refvar, fn, fn_value);

				auto params2 = params;
				params2.push_back(&DataType::FLOAT);
				Function* fn2 = scope->addFunction(var_type, name, params2, {});
				getIndexedVariable(refvar, fn2, fn_value);
				if (deprecated)
				{
					fn2->setFlag(FUNCFLAG_DEPRECATED);
					fn2->setInfo(*deprecated);
				}
			}
			else
			{
				fn = scope->addGetter(var_type, name, params, {}, 0);
				getVariable(refvar, fn, fn_value);
			}

			if (deprecated)
			{
				fn->setFlag(FUNCFLAG_DEPRECATED);
				fn->setInfo(*deprecated);
			}
		}

		// Add deprecated getter.
		if (auto deprecated_getter = parsed_comment.get_tag("deprecated_getter"))
		{
			if (is_arr)
			{
				handleError(CompileError::BadInternal(decl, "@deprecated_getter cannot be used on arrays"));
				continue;
			}

			std::string getter_name = *deprecated_getter;
			std::vector<const DataType*> params;
			if (refvar != NUL)
				params.push_back(user_class->getType());
			Function* fn = scope->addFunction(var_type, getter_name, params, {}, FUNCFLAG_DEPRECATED|FUNCFLAG_INTERNAL);
			fn->setExternalScope(scope->makeChild());
			fn->data_decl_source_node = decl;
			fn->setInfo(fmt::format("Use {} instead!", decl->getName()));

			getVariable(refvar, fn, fn_value);
		}

		if (is_constant_zero)
			continue;

		// Add setter(s).
		for (auto& name : names)
		{
			Function* fn;

			std::vector<const DataType*> params;
			if (user_class)
				params.push_back(user_class->getType());
			params.push_back(var_type);

			if (is_arr)
			{
				fn = scope->addSetter(&DataType::ZVOID, name, params, {}, 0);
				fn->setFlag(FUNCFLAG_READ_ONLY);

				std::vector<const DataType*> params2;
				if (user_class)
					params2.push_back(user_class->getType());
				params2.push_back(&DataType::FLOAT);
				params2.push_back(var_type);
				Function* fn2 = scope->addFunction(&DataType::ZVOID, name, params2, {});
				setIndexedVariable(refvar, fn2, fn_value);
				if (deprecated)
				{
					fn2->setFlag(FUNCFLAG_DEPRECATED);
					fn2->setInfo(*deprecated);
				}
				if (var_type->isUntyped())
					fn2->setFlag(FUNCFLAG_MAY_SET_OBJECT);
			}
			else if (var_type == &DataType::BOOL)
			{
				fn = scope->addSetter(&DataType::ZVOID, name, params, {}, 0);
				setBoolVariable(refvar, fn, fn_value);
			}
			else
			{
				fn = scope->addSetter(&DataType::ZVOID, name, params, {}, 0);
				setVariable(refvar, fn, fn_value);
			}

			if (deprecated)
			{
				fn->setFlag(FUNCFLAG_DEPRECATED);
				fn->setInfo(*deprecated);
			}

			if (node->readonly)
				fn->setFlag(FUNCFLAG_READ_ONLY);
		}
	}
}



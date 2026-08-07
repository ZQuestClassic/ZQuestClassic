#include "parser/AST.h"
#include "parser/ByteCode.h"
#include "parser/LibrarySymbols.h"
#include "parserDefs.h"
#include "RegBaseVisitor.h"
#include "Scope.h"
#include "CompileError.h"

#include "components/zasm/table.h"
#include "zc/ffscript.h"
#include <fmt/ranges.h>

using namespace ZScript;

RegBaseVisitor::RegBaseVisitor(Program& program)
	: RecursiveVisitor(program)
{}

void RegBaseVisitor::handle_data_decl_registry(ASTDataDecl& host)
{
	auto* list = host.list;
	DataType const* type = host.resolve_ornull(scope, this);
	assert(type);
	
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
	

	bool is_func_var = scope->getFunctionScope();
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
		scope->in_static_init = !is_func_var;
		const_value = host.getInitializer()->getCompileTimeValue(this, scope);
		scope->in_static_init = false;
		
		if (!is_func_var)
		{
			if (const_value)
				list->is_static = true; // force the list to be static
			else
			{
				// Non-function scopes require constant initializers for constants.
				handleError(CompileError::ConstNotConstant(&host, host.getName()));
				return;
			}
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
	
	bool is_static = host.is_static();
	bool is_classvar = parsing_user_class == puc_vars && !is_static;
	bool is_scriptvar = !is_static && scope->isScript();
	if (list && list->was_exported)
	{
		if (type->isConstant())
		{
			handleError(CompileError::ExportError(&host, "@Export() cannot be used on constants!"));
			return;
		}
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
		int32_t type_id = ZTID_VOID;
		if (auto const* simpleType = dynamic_cast<DataTypeSimple const*>(type))
		{
			auto id = simpleType->getId();
			if (id >= ZTID_PRIMITIVE_START && id <= ZTID_PRIMITIVE_END)
			{
				isCompatible = true;
				type_id = id;
			}
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
		else if (list->export_data.btn_type < 0)
		{
			// No explicit input field type - default from the variable's type.
			switch (type_id)
			{
				case ZTID_LONG:
					list->export_data.btn_type = nswapLDEC;
					break;
				case ZTID_RGBDATA:
					list->export_data.btn_type = nswapLHEX;
					break;
				default:
					list->export_data.btn_type = nswapDEC;
					break;
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

		vector<string> names = {decl->getName()};
		for (auto alias : parsed_comment.get_multi_tag("alias"))
			names.push_back(alias);

		// Add getter(s).
		for (auto& name : names)
		{
			Function* fn;

			vector<const DataType*> params;
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

			string getter_name = *deprecated_getter;
			// No parameter for the object: bindings never declare the left-hand
			// side of the arrow, it's pushed by the call site instead (and
			// getVariable skips even that when there's no ref variable).
			Function* fn = scope->addFunction(var_type, getter_name, {}, {}, FUNCFLAG_DEPRECATED|FUNCFLAG_INTERNAL);
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

			vector<const DataType*> params;
			if (user_class)
				params.push_back(user_class->getType());
			params.push_back(var_type);

			if (is_arr)
			{
				fn = scope->addSetter(&DataType::ZVOID, name, params, {}, 0);
				fn->setFlag(FUNCFLAG_READ_ONLY);

				vector<const DataType*> params2;
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

/////////////////////////////////////////
// Registration Helpers
/////////////////////////////////////////

void RegBaseVisitor::doRegister(AST& host)
{
	host.mark_registered();
	hasChanged = true;
}

bool RegBaseVisitor::registered(AST& node) const
{
	return node.registered();
}

bool RegBaseVisitor::registered(AST* node) const
{
	return !node || registered(*node);
}

/////////////////////////////////////////
// Visitor Cases
/////////////////////////////////////////

void RegBaseVisitor::caseDataEnum(ASTDataEnum& host, void* param)
{
	if (!parse_annotations_enum(host)) return;
	
	// Resolve the base type.
	DataType const* baseType = &host.baseType->resolve(*scope, this);
    if (breakRecursion(*host.baseType.get())) return;
	if (!baseType->isResolved())
	{
		if (!isRegistration())
			handleError(CompileError::UnresolvedType(&host, baseType->getName()));
		return;
	}
	
	// Don't allow void/auto types.
	if (baseType->isVoid() || baseType->isAuto())
	{
		handleError(CompileError::BadVarType(&host, host.asString(), baseType->getName()));
		doRegister(host);
		return;
	}
	
	host.is_static = !scope->getFunctionScope();
	

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
	vector<ASTDataDecl*> decls = host.getDeclarations();
	for(vector<ASTDataDecl*>::iterator it = decls.begin();
		it != decls.end(); ++it)
	{
		ASTDataDecl* declaration = *it;
		ASTExpr* init = declaration->getInitializer();
		if (!init)
		{
			if (!is_first)
			{
				if (host.increment_val)
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
		if (breakRecursion(host, param))
		{
			if (registered(declaration))
				doRegister(host); //Decl errored, but registered; fatal error
			return;
		}
		
		if (init)
		{
			if (isRegistration() && !registered(init))
				return;
			scope->in_static_init = host.is_static;
			optional<int32_t> v = init->getCompileTimeValue(this, scope);
			scope->in_static_init = false;
			if (v)
				value = zslongToFix(*v);
			else
			{
				if (!isRegistration())
					handleError(CompileError::ExprNotConstant(init));
				return;
			}
		}
	}
	if(registered_vec(host.getDeclarations()))
		doRegister(host);
}

/////////////////////////////////////////
// Annotation Helpers
/////////////////////////////////////////

map<string, ASTAnnotation*> RegBaseVisitor::validate_annotation_keys(ASTAnnotationList& annot_list, vector<string> const& expected, string const& object_type_name)
{
	map<string, ASTAnnotation*> ret {};
	auto& annots = annot_list.annots;
	for (auto it = annots.begin(); it != annots.end();)
	{
		string const& key = (*it)->key;
		if (!util::contains(expected, key))
		{
			handleError(CompileError::AnnotationError(*it, fmt::format("Annotation '@{}' not recognized for '{}'",
				key, object_type_name).c_str()));
		}
		else if (ret.contains(key))
		{
			handleError(CompileError::AnnotationError(*it, fmt::format("Duplicate Annotation '@{}'!", key).c_str()));
		}
		else
		{
			ret[key] = *it;
			++it;
			continue;
		}
			
		// Erase this errored annotation, and don't propagate the error state out
		// This allows processing more other errors at once
		failure_temp = false;
		it = annots.erase(it);
	}
	return ret;
}

bool RegBaseVisitor::parse_annot_param_as(ASTAnnotation& annot, size_t idx, AnnotParam_Parsed& output, AnnotParam_Parsed::Type ty)
{
	if (annot.params.size() <= idx)
	{
		if (ty != AnnotParam_Parsed::Type::NONE)
		{
			handleError(CompileError::AnnotationError(&annot, fmt::format("Expected at least {} parameters, only found {}!", idx+1, annot.params.size()).c_str()));
			return false;
		}
		return true;
	}
	AnnotParam_Raw& input = annot.params[idx];
	
	bool no_reg_errors = isRegistration();
	output = AnnotParam_Parsed();
	switch (ty)
	{
		case AnnotParam_Parsed::Type::NONE:
		{
			if (input.ty == AnnotParam_Raw::Type::NONE)
				return true;
			break;
		}
		case AnnotParam_Parsed::Type::STRING:
		case AnnotParam_Parsed::Type::STRING_UNESCAPED:
		{
			if (input.ty == AnnotParam_Raw::Type::STRING)
			{
				output.str = input.strval->getValue();
				if (ty == AnnotParam_Parsed::Type::STRING_UNESCAPED)
					output.str = util::disallow_escapes(util::escape_characters(output.str));
				output.ty = ty;
				return true;
			}
			break;
		}
		case AnnotParam_Parsed::Type::NUMBER:
		{
			if (input.ty == AnnotParam_Raw::Type::IDENTIFIER)
			{
				ASTExprIdentifier* ident = input.identval.get();
				// Use the identifier as a standard constant, try to resolve to a datum and get a constant value
				visit(ident);
				if (!ident->binding) // SemanticAnalyzer::caseExprIdentifier handles errors
					return false;
				
				if (auto val = ident->getCompileTimeValue(this, scope))
				{
					output.number = zslongToFix(*val);
					output.ty = ty;
					return true;
				}
				else
				{
					handleError(CompileError::ExprNotConstant(ident));
					return false;
				}
			}
			else if (input.ty == AnnotParam_Raw::Type::EXPR_CONST)
			{
				ASTExprConst* expr = input.exprval.get();
				visit(expr);
				if (auto val = expr->getCompileTimeValue(this, scope))
				{
					output.number = zslongToFix(*val);
					output.ty = ty;
					return true;
				}
				else return false;
			}
			break;
		}
		case AnnotParam_Parsed::Type::ENUM_NAME:
		{
			if (input.ty == AnnotParam_Raw::Type::IDENTIFIER)
			{
				ASTExprIdentifier* ident = input.identval.get();
				DataTypeUnresolved temp(ident->clone());
				DataType const* type = temp.resolve(*scope, no_reg_errors ? nullptr : this);
				if (breakRecursion())
					return false;
				if (type && type->isResolved())
				{
					if (type->isEnum())
					{
						output.enum_type = type;
						output.ty = ty;
						return true;
					}
					else
					{
						handleError(CompileError::AnnotationError(&annot, fmt::format("Annotation expected param {} to be enum type; found unexpected type '{}' instead!", idx, type->getName()).c_str()));
						return false;
					}
				}
				else
				{
					if (!no_reg_errors)
						handleError(CompileError::UnresolvedType(&annot, type ? type->getName() : "?????"));
					return false;
				}
			}
			break;
		}
		case AnnotParam_Parsed::Type::BTN_SWAP_TYPE:
		{
			if (input.ty == AnnotParam_Raw::Type::STRING)
			{
				auto str = input.strval->getValue();
				util::upperstr(str);
				if (str == "D")
					output.number = nswapDEC;
				else if (str == "H")
					output.number = nswapHEX;
				else if (str == "B")
					output.number = nswapBOOL;
				else if (str == "LD")
					output.number = nswapLDEC;
				else if (str == "LH")
					output.number = nswapLHEX;
				else if (str == "-1")
					output.number = -1;
				else
				{
					handleError(CompileError::AnnotationError(&annot,
						fmt::format("Annotation expected parameter {} to be \"D\", \"H\", \"B\","
							" \"LD\", or \"LH\", but it was {}!", idx, input.to_string(this, scope)).c_str()));
					return false;
				}
				output.str = str;
				output.ty = ty;
				return true;
			}
			break;
		}
	}
	
	// Invalid input type, error out
	string expected_ty_str = "??Unknown??";
	switch (ty)
	{
		case AnnotParam_Parsed::Type::NONE:
			expected_ty_str = "NONE";
			break;
		case AnnotParam_Parsed::Type::STRING:
		case AnnotParam_Parsed::Type::STRING_UNESCAPED:
			expected_ty_str = "String";
			break;
		case AnnotParam_Parsed::Type::BTN_SWAP_TYPE:
			expected_ty_str = "String";
			break;
		case AnnotParam_Parsed::Type::NUMBER:
			expected_ty_str = "Constant Number";
			break;
		case AnnotParam_Parsed::Type::ENUM_NAME:
			expected_ty_str = "Enum Type";
			break;
	}
	handleError(CompileError::AnnotationError(&annot,
		fmt::format("Annotation '@{}' parameter {}; expected {} but got '{}'",
			annot.key, idx, expected_ty_str, input.to_string(this, scope))));
	return false;
}

bool RegBaseVisitor::validate_annot_param_count(ASTAnnotation& annot, size_t min, optional<size_t> max)
{
	size_t mx = max.value_or(min);
	auto sz = annot.params.size();
	if (mx < min)
		zc_swap(mx, min);
	if (sz < min || sz > mx)
	{
		if (max && mx != min)
			handleError(CompileError::AnnotationError(&annot,
				fmt::format("Annotation '{}' found {} parameters; expected >= {} and <= {}",
					annot.key, sz, min, mx).c_str()));
		else
			handleError(CompileError::AnnotationError(&annot,
				fmt::format("Annotation '{}' found {} parameters; expected {}",
					annot.key, sz, min).c_str()));
		return false;
	}
	return true;
}

bool RegBaseVisitor::validate_annot_exclusions(string const& key, map<string, ASTAnnotation*> const& annots, vector<string> const& bad_keys)
{
	assert(annots.contains(key));
	vector<string> err_keys;
	for (auto& bad_key : bad_keys)
	{
		if (bad_key == key)
			continue;
		if (annots.contains(bad_key))
			err_keys.emplace_back(fmt::format("'@{}'", bad_key));
	}
	if (err_keys.empty())
		return true;
	handleError(CompileError::AnnotationError(annots.at(key),
		fmt::format("Annotation '@{}' is not compatible with {}", key, fmt::join(err_keys, ", "))));
	return false;
}

bool RegBaseVisitor::validate_annot_string_size(ASTAnnotation& annot, string& str, size_t max_length)
{
	if (str.size() > max_length)
	{
		handleError(CompileError::AnnotationError(&annot,
			fmt::format("Annotation String Value too long; found length {}, max was {}. String: \"{}\"", str.size(), max_length, str).c_str()));
		str = str.substr(0, max_length);
		return false;
	}
	return true;
}

/////////////////////////////////////////
// Annotation Handlers
/////////////////////////////////////////

/**
 * - Start a list, then within the list, start as many annotations as you like.
 * - End each annotation before starting another. End the list when done.
 * - Ending will return a bool. Starting will return `true` if the list was already fully processed previously.
 * - `ASTAnnotation& annot`, `const string key`, and `const size_t num_params` will be available in each annotation.
 * - `break;` from inside an annotation to fail parsing it.
 */
#define START_ANNOT_LIST(name, valid_keys) \
ASTAnnotationList& list = *node.name##_annotation; \
if (registered(list)) return true; \
bool _did_fail = false; \
bool _all_parsed = true; \
auto annots = validate_annotation_keys(list, valid_keys, #name);

#define END_ANNOT_LIST() \
if (_did_fail) failure_temp = true; \
if (_all_parsed) doRegister(list); \
else \
{ \
	if (!isRegistration() && !failure_temp) \
	{ \
		/* Catchall if an annotation somehow is never parsed but emits no error */ \
		vector<string> badKeys {}; \
		for (auto& [key, annot] : annots) \
		{ \
			if (!registered(annot)) \
				badKeys.emplace_back(fmt::format("'@{}'", key)); \
		} \
		DCHECK(!badKeys.empty()); \
		handleError(CompileError::AnnotationError(&list, fmt::format("Unable to resolve annotations {}", fmt::join(badKeys, ", ")).c_str())); \
	} \
	return false; \
} \
return !_did_fail;

#define START_ANNOT(name) \
{ \
	const string key = name; \
	auto it = annots.find(key); \
	if (it != annots.end()) \
	{ \
		auto& annot = *it->second; \
		if (!annot.isDisabled() && !registered(annot)) \
		{ \
			const size_t num_params = annot.params.size(); \
			bool _is_parsed = false; \
			do \
			{
#define END_ANNOT() \
				_is_parsed = true; \
			} while (false); \
			if (failure_temp) \
			{ \
				_did_fail = true; \
				failure_temp = false; \
				annot.disable(); \
			} \
			if (_is_parsed) \
				doRegister(annot); \
			else \
				_all_parsed = false; \
		} \
	} \
}

bool RegBaseVisitor::parse_annotations_enum(ASTDataEnum& node)
{
	if (breakRecursion()) return false;
	if (!node.enum_annotation) return true;
	
	static const vector<string> valid_keys = {
		"Increment",
		"Bitflags",
	};
	static const vector<string> exclusive_keys = {
		"Increment",
		"Bitflags",
	};
	
	START_ANNOT_LIST(enum, valid_keys)
	START_ANNOT("Increment")
	{
		if (!validate_annot_param_count(annot, 1))
			break;
		if (!validate_annot_exclusions(key, annots, exclusive_keys))
			break;
		
		AnnotParam_Parsed param;
		if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::NUMBER))
			break;
		
		node.increment_val = param.number;
	}
	END_ANNOT()
	START_ANNOT("Bitflags")
	{
		if (!validate_annot_param_count(annot, 1))
			break;
		if (!validate_annot_exclusions(key, annots, exclusive_keys))
			break;
		
		AnnotParam_Parsed param;
		if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING))
			break;
		
		if (param.str == "int")
			node.setBitMode(ASTDataEnum::BIT_INT);
		else if (param.str == "long")
			node.setBitMode(ASTDataEnum::BIT_LONG);
		else
		{
			handleError(CompileError::AnnotationError(&node,
				fmt::format("Annotation '@{}' expected \"int\" or \"long\", but got \"{}\"", key, param.str).c_str()));
			break;
		}
	}
	END_ANNOT()
	END_ANNOT_LIST()
}
bool RegBaseVisitor::parse_annotations_loop(ASTStmtRangeLoop& node)
{
	if (breakRecursion()) return false;
	if (!node.loop_annotation) return true;
	
	static const vector<string> valid_keys = {
		"AlwaysRunEndpoint",
	};
	
	START_ANNOT_LIST(loop, valid_keys)
	START_ANNOT("AlwaysRunEndpoint")
	{
		if (!validate_annot_param_count(annot, 1))
			break;
		
		AnnotParam_Parsed param;
		if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING))
			break;
		
		if (param.str == "off")
			node.overflow = ASTStmtRangeLoop::OVERFLOW_ALLOW;
		else if (param.str == "int")
			node.overflow = ASTStmtRangeLoop::OVERFLOW_INT;
		else if (param.str == "long" || param.str == "float")
			node.overflow = ASTStmtRangeLoop::OVERFLOW_LONG;
		else
		{
			handleError(CompileError::AnnotationError(&node,
				fmt::format("Annotation '@{}' expected \"off\", \"int\", \"float\", or \"long\", but got \"{}\"", key, param.str).c_str()));
			break;
		}
	}
	END_ANNOT()
	END_ANNOT_LIST()
}
bool RegBaseVisitor::parse_annotations_data(ASTDataDeclList& node)
{
	if (breakRecursion()) return false;
	if (!node.data_annotation) return true;
	
	static const vector<string> valid_keys = {
		"Export",
		"ExportRange",
	};
	
	START_ANNOT_LIST(data, valid_keys)
	START_ANNOT("Export")
	{
		if (node.getDeclarations().size() > 1)
		{
			handleError(CompileError::AnnotationError(&node, "@Export() can't be used on multi-variable declarations!"));
			break;
		}
		if (!validate_annot_param_count(annot, 1, 3))
			break;
		
		AnnotParam_Parsed p_name, p_helptext, p_btnty;
		if (!parse_annot_param_as(annot, 0, p_name, AnnotParam_Parsed::Type::STRING_UNESCAPED))
			break;
		
		if (num_params > 1 && !parse_annot_param_as(annot, 1, p_helptext, AnnotParam_Parsed::Type::STRING))
			break;
		
		if (num_params > 2 && !parse_annot_param_as(annot, 2, p_btnty, AnnotParam_Parsed::Type::BTN_SWAP_TYPE))
			break;
		
		validate_annot_string_size(annot, p_name.str, 255);
		if (p_helptext)
			validate_annot_string_size(annot, p_helptext.str, 65535);
		
		node.export_data.name = p_name.str;
		if (p_helptext)
			node.export_data.helptext = p_helptext.str;
		if (p_btnty)
			node.export_data.btn_type = p_btnty.number;
		node.was_exported = true;
	}
	END_ANNOT()
	START_ANNOT("ExportRange")
	{
		if (!annots.contains("Export"))
		{
			handleError(CompileError::AnnotationError(&node, "@ExportRange() requires @Export() to function!"));
			break;
		}
		if (!validate_annot_param_count(annot, 2))
			break;
		
		AnnotParam_Parsed p_min, p_max;
		if (!parse_annot_param_as(annot, 0, p_min, AnnotParam_Parsed::Type::NUMBER))
			break;
		if (!parse_annot_param_as(annot, 1, p_max, AnnotParam_Parsed::Type::NUMBER))
			break;
		
		if (p_min.number > p_max.number)
		{
			handleError(CompileError::AnnotationError(&node,
				fmt::format("Annotation '@{}' expected min <= max, but got {} > {}", key, p_min.number.str_trim(), p_max.number.str_trim()).c_str()));
			break;
		}
		
		node.export_data.min = p_min.number;
		node.export_data.max = p_max.number;
		node.was_range_exported = true;
	}
	END_ANNOT()
	END_ANNOT_LIST()
}
bool RegBaseVisitor::parse_annotations_script(ASTScript& node)
{
	if (breakRecursion()) return false;
	if (!node.script_annotation) return true;
	
	static vector<string> valid_keys = {
		"Author",
		"InitScript",
		"ScriptInfo",
		"ScriptSetup",
	};
	static bool initialized = false;
	if (!initialized)
	{
		for (int q = 0; q < 16; ++q)
		{
			valid_keys.emplace_back(fmt::format("Flag{}", q));
			valid_keys.emplace_back(fmt::format("FlagHelp{}", q));
		}
		for (int q = 0; q < 8; ++q)
		{
			valid_keys.emplace_back(fmt::format("ExportInitD{}", q));
			valid_keys.emplace_back(fmt::format("InitD{}", q));
			valid_keys.emplace_back(fmt::format("InitDType{}", q));
			valid_keys.emplace_back(fmt::format("InitDHelp{}", q));
		}
		for (int q = 0; q < NUM_ZMETA_ATTRIBUTES; ++q)
		{
			valid_keys.emplace_back(fmt::format("Attribute{}", q));
			valid_keys.emplace_back(fmt::format("AttributeHelp{}", q));
		}
		for (int q = 0; q < 8; ++q)
		{
			valid_keys.emplace_back(fmt::format("Attribyte{}", q));
			valid_keys.emplace_back(fmt::format("AttribyteHelp{}", q));
			valid_keys.emplace_back(fmt::format("Attrishort{}", q));
			valid_keys.emplace_back(fmt::format("AttrishortHelp{}", q));
		}
		initialized = true;
	}
	
	START_ANNOT_LIST(script, valid_keys)
	START_ANNOT("Author")
	{
		if (!validate_annot_param_count(annot, 1))
			break;
		
		AnnotParam_Parsed param;
		if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING_UNESCAPED))
			break;
		validate_annot_string_size(annot, param.str, 255);
		node.metadata.author = param.str;
	}
	END_ANNOT()
	START_ANNOT("InitScript")
	{
		if (!validate_annot_param_count(annot, 1))
			break;
		
		AnnotParam_Parsed param;
		if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::NUMBER))
			break;
		node.init_weight = param.number.getZLong();
	}
	END_ANNOT()
	START_ANNOT("ScriptInfo")
	{
		if (!validate_annot_param_count(annot, 1))
			break;
		
		AnnotParam_Parsed param;
		if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING))
			break;
		validate_annot_string_size(annot, param.str, 65535);
		node.metadata.script_info = param.str;
	}
	END_ANNOT()
	START_ANNOT("ScriptSetup")
	{
		if (!validate_annot_param_count(annot, 1))
			break;
		
		AnnotParam_Parsed param;
		if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING))
			break;
		validate_annot_string_size(annot, param.str, 65535);
		node.metadata.script_setup = param.str;
	}
	END_ANNOT()
	for (int q = 0; q < 16; ++q)
	{
		START_ANNOT(fmt::format("Flag{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING_UNESCAPED))
				break;
			validate_annot_string_size(annot, param.str, 255);
			node.metadata.usrflags[q] = param.str;
		}
		END_ANNOT()
		START_ANNOT(fmt::format("FlagHelp{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING))
				break;
			validate_annot_string_size(annot, param.str, 65535);
			node.metadata.usrflags_help[q] = param.str;
		}
		END_ANNOT()
	}
	for (int q = 0; q < 8; ++q)
	{
		string export_key = fmt::format("ExportInitD{}", q);
		START_ANNOT(export_key)
			if (!validate_annot_param_count(annot, 1, 3))
				break;
			if (!validate_annot_exclusions(key, annots, {
				fmt::format("InitD{}", q),
				fmt::format("InitDHelp{}", q),
				fmt::format("InitDType{}", q),
			}))
				break;
			
			AnnotParam_Parsed p_name, p_helptext, p_btnty;
			if (!parse_annot_param_as(annot, 0, p_name, AnnotParam_Parsed::Type::STRING_UNESCAPED))
				break;
			
			if (num_params > 1 && !parse_annot_param_as(annot, 1, p_helptext, AnnotParam_Parsed::Type::STRING))
				break;
			
			if (num_params > 2 && !parse_annot_param_as(annot, 2, p_btnty, AnnotParam_Parsed::Type::BTN_SWAP_TYPE))
				break;
			
			validate_annot_string_size(annot, p_name.str, 255);
			if (p_helptext)
				validate_annot_string_size(annot, p_helptext.str, 65535);
			
			node.metadata.initd_label[q] = p_name.str;
			if (p_helptext)
				node.metadata.initd_help[q] = p_helptext.str;
			if (p_btnty)
				node.metadata.initd_type[q] = p_btnty.number;
		END_ANNOT()
		START_ANNOT(fmt::format("InitD{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			if (!validate_annot_exclusions(key, annots, {export_key}))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING_UNESCAPED))
				break;
			validate_annot_string_size(annot, param.str, 255);
			node.metadata.initd_label[q] = param.str;
		}
		END_ANNOT()
		START_ANNOT(fmt::format("InitDHelp{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			if (!validate_annot_exclusions(key, annots, {export_key}))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING))
				break;
			validate_annot_string_size(annot, param.str, 65535);
			node.metadata.initd_help[q] = param.str;
		}
		END_ANNOT()
		START_ANNOT(fmt::format("InitDType{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			if (!validate_annot_exclusions(key, annots, {export_key}))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::BTN_SWAP_TYPE))
				break;
			node.metadata.initd_type[q] = param.number;
		}
		END_ANNOT()
	}
	for (int q = 0; q < NUM_ZMETA_ATTRIBUTES; ++q)
	{
		START_ANNOT(fmt::format("Attribute{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			string exclude_key = "";
			if (q >= 8 && q < 16)
				exclude_key = fmt::format("Attribyte{}", q-8);
			else if (q >= 16 && q < 24)
				exclude_key = fmt::format("Attrishort{}", q-16);
			if (!validate_annot_exclusions(key, annots, {exclude_key}))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING_UNESCAPED))
				break;
			validate_annot_string_size(annot, param.str, 255);
			node.metadata.attributes[q] = param.str;
		}
		END_ANNOT()
		START_ANNOT(fmt::format("AttributeHelp{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			string exclude_key = "";
			if (q >= 8 && q < 16)
				exclude_key = fmt::format("AttribyteHelp{}", q-8);
			else if (q >= 16 && q < 24)
				exclude_key = fmt::format("AttrishortHelp{}", q-16);
			if (!validate_annot_exclusions(key, annots, {exclude_key}))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING))
				break;
			validate_annot_string_size(annot, param.str, 65535);
			node.metadata.attributes_help[q] = param.str;
		}
		END_ANNOT()
	}
	for (int q = 0; q < 8; ++q)
	{
		START_ANNOT(fmt::format("Attribyte{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			string exclude_key = fmt::format("Attribute{}", q+8);
			if (!validate_annot_exclusions(key, annots, {exclude_key}))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING_UNESCAPED))
				break;
			validate_annot_string_size(annot, param.str, 255);
			node.metadata.attributes[q+8] = param.str;
		}
		END_ANNOT()
		START_ANNOT(fmt::format("AttribyteHelp{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			string exclude_key = fmt::format("AttributeHelp{}", q+8);
			if (!validate_annot_exclusions(key, annots, {exclude_key}))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING))
				break;
			validate_annot_string_size(annot, param.str, 65535);
			node.metadata.attributes_help[q+8] = param.str;
		}
		END_ANNOT()
		START_ANNOT(fmt::format("Attrishort{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			string exclude_key = fmt::format("Attribute{}", q+16);
			if (!validate_annot_exclusions(key, annots, {exclude_key}))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING_UNESCAPED))
				break;
			validate_annot_string_size(annot, param.str, 255);
			node.metadata.attributes[q+16] = param.str;
		}
		END_ANNOT()
		START_ANNOT(fmt::format("AttrishortHelp{}", q))
		{
			if (!validate_annot_param_count(annot, 1))
				break;
			string exclude_key = fmt::format("AttributeHelp{}", q+16);
			if (!validate_annot_exclusions(key, annots, {exclude_key}))
				break;
			
			AnnotParam_Parsed param;
			if (!parse_annot_param_as(annot, 0, param, AnnotParam_Parsed::Type::STRING))
				break;
			validate_annot_string_size(annot, param.str, 65535);
			node.metadata.attributes_help[q+16] = param.str;
		}
		END_ANNOT()
	}
	END_ANNOT_LIST()
}


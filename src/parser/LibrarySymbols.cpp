#include "symbols/SymbolDefs.h"
#include <fmt/format.h>
#include <sstream>

AccessorTable::AccessorTable(std::string const& name, byte tag, int32_t rettype,
	int32_t var, int32_t flags,
	vector<int32_t>const& params, vector<int32_t> const& optparams,
	byte extra_vargs, string const& info)
	: name(name), tag(tag), rettype(rettype), var(var),
	funcFlags(flags), params(params),
	optparams(optparams), extra_vargs(extra_vargs), info(info),
	alias_name(), alias_tag(0)
{}
AccessorTable::AccessorTable(std::string const& name, byte tag,
	string const& alias, byte alias_tag,
	int32_t flags, string const& info)
	: name(name), tag(tag),alias_name(alias), alias_tag(alias_tag),
	rettype(0), var(0),
	funcFlags(flags), info(info),
	params(),optparams(), extra_vargs()
{}

LibrarySymbols LibrarySymbols::nilsymbols = LibrarySymbols();
LibrarySymbols* LibrarySymbols::getTypeInstance(DataTypeId typeId)
{
    switch (typeId)
    {
		case ZTID_FFC: return &FFCSymbols::getInst();
		case ZTID_PLAYER: return &HeroSymbols::getInst();
		case ZTID_SCREEN: return &ScreenSymbols::getInst();
		case ZTID_REGION: return &RegionSymbols::getInst();
		case ZTID_VIEWPORT: return &ViewportSymbols::getInst();
		case ZTID_GAME: return &GameSymbols::getInst();
		case ZTID_ITEM: return &ItemSymbols::getInst();
		case ZTID_ITEMCLASS: return &ItemclassSymbols::getInst();
		case ZTID_NPC: return &NPCSymbols::getInst();
		case ZTID_LWPN: return &HeroWeaponSymbols::getInst();
		case ZTID_EWPN: return &EnemyWeaponSymbols::getInst();
		case ZTID_NPCDATA: return &NPCDataSymbols::getInst();
		case ZTID_DEBUG: return &DebugSymbols::getInst();
		case ZTID_AUDIO: return &AudioSymbols::getInst();
		case ZTID_COMBOS: return &CombosPtrSymbols::getInst();
		case ZTID_SPRITEDATA: return &SpriteDataSymbols::getInst();
		case ZTID_GRAPHICS: return &GraphicsSymbols::getInst();
		case ZTID_BITMAP: return &BitmapSymbols::getInst();
		case ZTID_TEXT: return &TextPtrSymbols::getInst();
		case ZTID_INPUT: return &InputSymbols::getInst();
		case ZTID_MAPDATA: return &MapDataSymbols::getInst();
		case ZTID_DMAPDATA: return &DMapDataSymbols::getInst();
		case ZTID_ZMESSAGE: return &MessageDataSymbols::getInst();
		case ZTID_SHOPDATA: return &ShopDataSymbols::getInst();
		case ZTID_DROPSET: return &DropsetSymbols::getInst();
		case ZTID_FILESYSTEM: return &FileSystemSymbols::getInst();
		case ZTID_FILE: return &FileSymbols::getInst();
		case ZTID_DIRECTORY: return &DirectorySymbols::getInst();
		case ZTID_STACK: return &StackSymbols::getInst();
		case ZTID_ZINFO: return &ZInfoSymbols::getInst();
		case ZTID_RNG: return &RNGSymbols::getInst();
		case ZTID_PALDATA: return &PalDataSymbols::getInst();
		case ZTID_BOTTLETYPE: return &BottleTypeSymbols::getInst();
		case ZTID_BOTTLESHOP: return &BottleShopSymbols::getInst();
		case ZTID_GENERICDATA: return &GenericDataSymbols::getInst();
		case ZTID_PORTAL: return &PortalSymbols::getInst();
		case ZTID_SAVPORTAL: return &SavedPortalSymbols::getInst();
		case ZTID_SUBSCREENDATA: return &SubscreenDataSymbols::getInst();
		case ZTID_SUBSCREENPAGE: return &SubscreenPageSymbols::getInst();
		case ZTID_SUBSCREENWIDGET: return &SubscreenWidgetSymbols::getInst();
		default: return &nilsymbols;
    }
}

void getConstant(int32_t refVar, Function* function, int32_t val)
{
	if(refVar != NUL)
	{
		throw compile_exception(fmt::format("Internal Constant {} has non-NUL refVar!", function->name));
	}
	function->setFlag(FUNCFLAG_INLINE);
	function->setIntFlag(IFUNCFLAG_SKIPPOINTER);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	addOpcode2(code, new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(val)));
	LABELBACK(label);
	function->giveCode(code);
	function->set_constexpr(CONSTEXPR_CBACK_HEADER(val)
		{
			return val;
		});
}

void getVariable(int32_t refVar, Function* function, int32_t var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop object pointer
	if(refVar == NUL)
	{
		function->setIntFlag(IFUNCFLAG_SKIPPOINTER);
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(var)));
		LABELBACK(label);
	}
	else
	{
		//Pop object pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(refVar)));
		LABELBACK(label);
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(var)));
	}
	RETURN();
	function->giveCode(code);
}

void getIndexedVariable(int32_t refVar, Function* function, int32_t var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop index
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
	LABELBACK(label);
	//pop object pointer
	POPREF();
	addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(var)));
	RETURN();
	function->giveCode(code);
}

void setVariable(int32_t refVar, Function* function, int32_t var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop off the value to set to
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
	LABELBACK(label);
	//pop object pointer
	POPREF();
	addOpcode2 (code, new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
	RETURN();
	function->giveCode(code);
}

void setBoolVariable(int32_t refVar, Function* function, int32_t var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop off the value to set to
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
	LABELBACK(label);
	//renormalize true to 1
	addOpcode2 (code, new OCastBoolI(new VarArgument(EXP1)));
	
	POPREF();
	addOpcode2 (code, new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
	RETURN();
	function->giveCode(code);
}

void setIndexedVariable(int32_t refVar, Function* function, int32_t var)
{
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop off index
	addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
	LABELBACK(label);
	//pop off the value to set to
	addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
	//pop object pointer
	POPREF();
	addOpcode2 (code, new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
	RETURN();
	function->giveCode(code);
}

void handleNil(int32_t refVar, Function* function)
{
	function->setFlag(FUNCFLAG_INLINE);
	if(refVar == NUL)
		function->setFlag(IFUNCFLAG_SKIPPOINTER);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	addOpcode2(code, new ONoOp());
	LABELBACK(label);
	function->giveCode(code);
}

void LibrarySymbols::addSymbolsToScope(Scope& scope)
{
	if(!table) return;
	TypeStore& typeStore = scope.getTypeStore();
	
	vector<string const*> blankParams;
	std::ostringstream errorstream;
	
	for (int32_t i = 0; table[i].name != ""; i++)
	{
		try
		{
			AccessorTable& entry = table[i];
			if(entry.alias_name)
			{
				Function* func = new Function();
				func->name = entry.name;
				func->setInfo(entry.info);
				func->setFlag(entry.funcFlags);
				func->setEntry(&entry);
				if(hasPrefixType)
					func->hasPrefixType = true;
				alias_functions[make_pair(entry.name,entry.tag)] = func;
				continue;
			}
			
			DataType const* returnType = typeStore.getType(entry.rettype);
			vector<DataType const*> paramTypes;
			for (auto& ptype : entry.params)
				paramTypes.push_back(typeStore.getType(ptype));
					
			std::string const& name = entry.name;
			std::string varName = name;
				
			// Strip out the array at the end.
			bool isArray = name.substr(name.size() - 2) == "[]";
			if (isArray)
				varName = name.substr(0, name.size() - 2);

			// Create function object.
			auto setorget = FUNCTION;
			Function* function = nullptr;
			if (name.substr(0, 5) == "const")
			{
				setorget = CONSTANT;
				varName = varName.substr(5); // Strip out "const"
				function = scope.addGetter(returnType, varName, paramTypes, blankParams, entry.funcFlags);
			}
			else if (entry.var > -1 && name.substr(0, 3) == "set")
			{
				setorget = SETTER;
				varName = varName.substr(3); // Strip out "set".
				function = scope.addSetter(returnType, varName, paramTypes, blankParams, entry.funcFlags);
			}
			else if (entry.var > -1 && name.substr(0, 3) == "get")
			{
				setorget = GETTER;
				varName = varName.substr(3); // Strip out "get".
				function = scope.addGetter(returnType, varName, paramTypes, blankParams, entry.funcFlags);
			}
			else
			{
				if(name.substr(0,4) == "_set")
				{
					setorget = SETTER;
					varName = varName.substr(4); // Strip out "_set".
				}
				else if(name.substr(0,4) == "_get")
				{
					setorget = GETTER;
					varName = varName.substr(4); // Strip out "_get".
				}
				function = scope.addFunction(returnType, varName, paramTypes, blankParams, entry.funcFlags);
			}
			
			if(!function)
				throw compile_exception(fmt::format("Failed to create internal function '{}', {}\n",name,entry.tag));
			
			function->setEntry(&entry);
			functions[make_pair(name,entry.tag)] = function;
			if(hasPrefixType)
				function->hasPrefixType = true; //Print the first type differently in error messages!
			
			function->opt_vals = entry.optparams;
			function->setInfo(entry.info);
			if(function->getFlag(FUNCFLAG_VARARGS))
			{
				function->extra_vargs = entry.extra_vargs;
				function->setFlag(FUNCFLAG_INLINE);
			}
			// Generate function code for getters/setters
			int32_t label = function->getLabel();
			if(function->getFlag(FUNCFLAG_NIL))
			{
				handleNil(refVar, function);
			}
			else switch(setorget)
			{
				case GETTER:
					if (isArray)
						getIndexedVariable(refVar, function, entry.var);
					else
						getVariable(refVar, function, entry.var);
					break;
				case SETTER:
					if (isArray)
						setIndexedVariable(refVar, function, entry.var);
					else if (entry.params.size() > 1 && entry.params[1] == ZTID_BOOL)
						setBoolVariable(refVar, function, entry.var);
					else
						setVariable(refVar, function, entry.var);
					break;
				case CONSTANT:
					getConstant(refVar, function, entry.var);
					break;
			}
		}
		catch(std::exception &e)
		{
			errorstream << e.what() << '\n';
		}
	}
	
	try
	{
		for(auto& p : alias_functions)
		{
			Function* func = p.second;
			auto entry = func->getEntry();
			Function* alias_func = getAlias(*entry->alias_name, entry->alias_tag);
			func->alias(alias_func);
			scope.addAlias(func);
		}
		generateCode();
	}
	catch (std::exception& e)
	{
		errorstream << e.what() << '\n';
	}

	functions.clear();
	
	std::string errors = errorstream.str();
	if(!errors.empty())
		throw compile_exception(errors);
}

Function* LibrarySymbols::getFunction(std::string const& name, byte tag) const
{
	std::pair<std::string, int32_t> p = make_pair(name, tag);
	Function* ret = find<Function*>(functions, p).value_or(nullptr);
	if(!ret)
		throw compile_exception(fmt::format("Unique internal function {} not found with tag {}!", name, tag));
	
	return ret;
}
Function* LibrarySymbols::getAlias(std::string const& name, byte tag) const
{
	std::pair<std::string, int32_t> p = make_pair(name, tag);
	Function* ret = find<Function*>(functions, p).value_or(nullptr);
	Function* ret2 = find<Function*>(alias_functions, p).value_or(nullptr);
	
	if(!XOR(ret, ret2))
		throw compile_exception(fmt::format("Unique internal function {} not found with tag {}!", name, tag));
	
	return ret ? ret : ret2;
}

LibrarySymbols::~LibrarySymbols(){}

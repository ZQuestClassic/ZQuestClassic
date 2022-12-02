#include "symbols/SymbolDefs.h"

LibrarySymbols* LibrarySymbols::getTypeInstance(DataTypeId typeId)
{
    switch (typeId)
    {
    case ZTID_FFC: return &FFCSymbols::getInst();
    case ZTID_PLAYER: return &HeroSymbols::getInst();
    case ZTID_SCREEN: return &ScreenSymbols::getInst();
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
    case ZTID_UNTYPED: return &UntypedSymbols::getInst();
    case ZTID_DROPSET: return &DropsetSymbols::getInst();
    case ZTID_PONDS: return &PondSymbols::getInst();
    case ZTID_WARPRING: return &WarpringSymbols::getInst();
    case ZTID_DOORSET: return &DoorsetSymbols::getInst();
    case ZTID_ZUICOLOURS: return &MiscColourSymbols::getInst();
    case ZTID_RGBDATAOLD: return &RGBSymbolsOld::getInst();
    case ZTID_PALETTEOLD: return &PaletteSymbolsOld::getInst();
    case ZTID_TUNES: return &TunesSymbols::getInst();
    case ZTID_PALCYCLE: return &PalCycleSymbols::getInst();
    case ZTID_GAMEDATA: return &GamedataSymbols::getInst();
    case ZTID_CHEATS: return &CheatsSymbols::getInst();
	case ZTID_FILESYSTEM: return &FileSystemSymbols::getInst();
	case ZTID_SUBSCREENDATA: return &SubscreenDataSymbols::getInst();
	case ZTID_FILE: return &FileSymbols::getInst();
	case ZTID_DIRECTORY: return &DirectorySymbols::getInst();
	case ZTID_STACK: return &StackSymbols::getInst();
	case ZTID_MODULE: return &ModuleSymbols::getInst();
	case ZTID_RNG: return &RNGSymbols::getInst();
	case ZTID_PALDATA: return &PalDataSymbols::getInst();
	case ZTID_BOTTLETYPE: return &BottleTypeSymbols::getInst();
	case ZTID_BOTTLESHOP: return &BottleShopSymbols::getInst();
	case ZTID_GENERICDATA: return &GenericDataSymbols::getInst();
    default: return NULL;
    }
}

void getVariable(int32_t refVar, Function* function, int32_t var)
{
	//Functions passed here take 1 opcode, +popref, +ret; therefore should be inlined -V
	function->setFlag(FUNCFLAG_INLINE);
	int32_t label = function->getLabel();
	vector<shared_ptr<Opcode>> code;
	//pop object pointer
	if(refVar == NUL)
	{
		function->internal_flags |= IFUNCFLAG_SKIPPOINTER;
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
	//Functions passed here take 2 opcodes, +popref, +ret; therefore should be inlined -V
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
	//Functions passed here take 2 opcodes, +popref, +ret; therefore should be inlined -V
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
	//Functions passed here take 3 opcodes, +popref, +ret; therefore should be inlined -V
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

void LibrarySymbols::addSymbolsToScope(Scope& scope)
{
	TypeStore& typeStore = scope.getTypeStore();
	
	vector<string const*> blankParams;
	
	if(table)
	for (int32_t i = 0; table[i].name != ""; i++)
	{
		AccessorTable& entry = table[i];
		
		DataType const* returnType = typeStore.getType(entry.rettype);
		vector<DataType const*> paramTypes;
		for (int32_t k = 0; entry.params[k] != -1 && k < 20; k++)
			paramTypes.push_back(typeStore.getType(entry.params[k]));
				
		std::string const& name = entry.name;
		std::string varName = name;
			
		// Strip out the array at the end.
		bool isArray = name.substr(name.size() - 2) == "[]";
		if (isArray)
			varName = name.substr(0, name.size() - 2);

		// Create function object.
		Function* function;
		if (entry.setorget == SETTER && name.substr(0, 3) == "set")
		{
			varName = varName.substr(3); // Strip out "set".
			function = scope.addSetter(returnType, varName, paramTypes, blankParams, entry.funcFlags);
		}
		else if (entry.setorget == GETTER && name.substr(0, 3) == "get")
		{
			varName = varName.substr(3); // Strip out "get".
			function = scope.addGetter(returnType, varName, paramTypes, blankParams, entry.funcFlags);
		}
		else
			function = scope.addFunction(returnType, varName, paramTypes, blankParams, entry.funcFlags);
		functions[make_pair(name,function->numParams())] = function;
		if(hasPrefixType)
			function->hasPrefixType = true; //Print the first type differently in error messages!
		// Generate function code for getters/setters
		int32_t label = function->getLabel();
		if (entry.setorget == GETTER)
		{
			if (isArray)
				getIndexedVariable(refVar, function, entry.var);
			else
				getVariable(refVar, function, entry.var);
		}
		if (entry.setorget == SETTER)
		{
			if (isArray)
				setIndexedVariable(refVar, function, entry.var);
			else if (entry.params[1] == ZTID_BOOL)
				setBoolVariable(refVar, function, entry.var);
			else
				setVariable(refVar, function, entry.var);
		}
			
	}
	
	if(table2)
	for (int32_t i = 0; table2[i].name != ""; i++)
	{
		AccessorTable2& entry = table2[i];
		
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
		Function* function;
		auto setorget = FUNCTION;
		bool internal = name[0] == '_';
		if(internal)
		{
			if(name.substr(0,4) == "_set")
			{
				setorget = SETTER;
				varName = varName.substr(4); // Strip out "_set".
				function = scope.addFunction(returnType, varName, paramTypes, blankParams, entry.funcFlags);
			}
			else if(name.substr(0,4) == "_get")
			{
				setorget = GETTER;
				varName = varName.substr(4); // Strip out "_get".
				function = scope.addFunction(returnType, varName, paramTypes, blankParams, entry.funcFlags);
			}
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
			function = scope.addFunction(returnType, varName, paramTypes, blankParams, entry.funcFlags);
		functions[make_pair(name,-1-entry.tag)] = function;
		if(hasPrefixType)
			function->hasPrefixType = true; //Print the first type differently in error messages!
		
		function->opt_vals = entry.optparams;
		if(function->getFlag(FUNCFLAG_VARARGS))
			function->setFlag(FUNCFLAG_INLINE);
		
		// Generate function code for getters/setters
		int32_t label = function->getLabel();
		if (setorget == GETTER)
		{
			if (isArray)
				getIndexedVariable(refVar, function, entry.var);
			else
				getVariable(refVar, function, entry.var);
		}
		else if (setorget == SETTER)
		{
			if (isArray)
				setIndexedVariable(refVar, function, entry.var);
			else if (entry.params.size() > 1 && entry.params[1] == ZTID_BOOL)
				setBoolVariable(refVar, function, entry.var);
			else
				setVariable(refVar, function, entry.var);
		}
	}
	
	
	generateCode();
	functions.clear();
}

Function* LibrarySymbols::getFunction(std::string const& name, int32_t numParams) const
{
	std::pair<std::string, int32_t> p = make_pair(name, numParams);
	Function* ret = find<Function*>(functions, p).value_or(nullptr);
	if(!ret)
	{
		char buf[256];
		sprintf(buf, "Unique internal function %s not found with %d parameters!", name.c_str(), numParams);
		throw std::runtime_error(buf);
	}
	return ret;
}

Function* LibrarySymbols::getFunction2(std::string const& name, int32_t tag) const
{
	std::pair<std::string, int32_t> p = make_pair(name, -1-tag);
	Function* ret = find<Function*>(functions, p).value_or(nullptr);
	if(!ret)
	{
		char buf[256];
		sprintf(buf, "Unique internal function %s not found with tag %d!", name.c_str(), tag);
		throw std::runtime_error(buf);
	}
	return ret;
}

LibrarySymbols::~LibrarySymbols()
{
    return;
}

//Empty table for an empty type? *shrug* Zoria's doing. -Em
UntypedSymbols UntypedSymbols::singleton = UntypedSymbols();
static AccessorTable NilTable[] =
{
//	  name,                     rettype,                    setorget,     var,              numindex,      funcFlags,                            numParams,   params
	{ "",                       -1,                         -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};
UntypedSymbols::UntypedSymbols()
{
    table = NilTable;
    refVar = REFNIL;
	//hasPrefixType = false;
}
void UntypedSymbols::generateCode()
{
}

//Unused Tables - Move to separate file if you want to implement them

PondSymbols PondSymbols::singleton = PondSymbols();

static AccessorTable PondsTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	 { "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,      1,             0,                                    1,           { ZTID_PONDS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    1,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

PondSymbols::PondSymbols()
{
    table = PondsTable;
    refVar = REFPONDS;
}

void PondSymbols::generateCode()
{
}

WarpringSymbols WarpringSymbols::singleton = WarpringSymbols();

static AccessorTable WarpringTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	 { "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,      1,             0,                                    1,           { ZTID_WARPRING, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

WarpringSymbols::WarpringSymbols()
{
    table = WarpringTable;
    refVar = REFWARPRINGS;
}

void WarpringSymbols::generateCode()
{
}

DoorsetSymbols DoorsetSymbols::singleton = DoorsetSymbols();

static AccessorTable DoorsetTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	 { "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,      1,             0,                                    1,           { ZTID_DOORSET, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

DoorsetSymbols::DoorsetSymbols()
{
    table = DoorsetTable;
    refVar = REFDOORS;
}

void DoorsetSymbols::generateCode()
{
}

MiscColourSymbols MiscColourSymbols::singleton = MiscColourSymbols();

static AccessorTable MiscColoursTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	 { "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,      1,             0,                                    1,           { ZTID_ZUICOLOURS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

MiscColourSymbols::MiscColourSymbols()
{
    table = MiscColoursTable;
    refVar = REFUICOLOURS;
}

void MiscColourSymbols::generateCode()
{
}

RGBSymbolsOld RGBSymbolsOld::singleton = RGBSymbolsOld();

static AccessorTable RGBTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

RGBSymbolsOld::RGBSymbolsOld()
{
    table = RGBTable;
    refVar = REFRGB;
}

void RGBSymbolsOld::generateCode()
{
	
}

PaletteSymbolsOld PaletteSymbolsOld::singleton = PaletteSymbolsOld();

static AccessorTable PaletteTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	 { "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,      1,             0,                                    1,           { ZTID_PALETTEOLD, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

PaletteSymbolsOld::PaletteSymbolsOld()
{
    table = PaletteTable;
    refVar = REFPALETTE;
}

void PaletteSymbolsOld::generateCode()
{
}

TunesSymbols TunesSymbols::singleton = TunesSymbols();

static AccessorTable TunesTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,             numindex,      funcFlags,                            numParams,       params
	 { "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,     1,             0,                                    1,           { ZTID_TUNES, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,              -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

TunesSymbols::TunesSymbols()
{
    table = TunesTable;
    refVar = REFTUNES;
}

void TunesSymbols::generateCode()
{
}

PalCycleSymbols PalCycleSymbols::singleton = PalCycleSymbols();

static AccessorTable PalCycleTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,               numindex,      funcFlags,                            numParams,   params
	 { "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,       1,             0,                                    1,           { ZTID_PALCYCLE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,                -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

PalCycleSymbols::PalCycleSymbols()
{
    table = PalCycleTable;
    refVar = REFPALCYCLE;
}

void PalCycleSymbols::generateCode()
{
}

GamedataSymbols GamedataSymbols::singleton = GamedataSymbols();

static AccessorTable GameDataTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	 { "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,      1,             0,                                    1,           { ZTID_GAMEDATA, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

GamedataSymbols::GamedataSymbols()
{
    table = GameDataTable;
    refVar = REFGAMEDATA;
}

void GamedataSymbols::generateCode()
{
}

CheatsSymbols CheatsSymbols::singleton = CheatsSymbols();

static AccessorTable CheatTable[] =
{
//	All of these return a function label error when used:
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	 { "getTest",                ZTID_FLOAT,         GETTER,       DEBUGREFFFC,      1,             0,                                    1,           { ZTID_CHEATS, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

CheatsSymbols::CheatsSymbols()
{
    table = CheatTable;
    refVar = REFCHEATS;
}

void CheatsSymbols::generateCode()
{
}


SubscreenDataSymbols SubscreenDataSymbols::singleton = SubscreenDataSymbols();

static AccessorTable SubscreenDataTable[] =
{
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
//	{ "DirExists",              ZTID_BOOL,          FUNCTION,     0,                1,             0,                                    2,           { ZTID_FILESYSTEM, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

SubscreenDataSymbols::SubscreenDataSymbols()
{
    table = SubscreenDataTable;
    refVar = REFSUBSCREEN;
}

void SubscreenDataSymbols::generateCode()
{
	//
}


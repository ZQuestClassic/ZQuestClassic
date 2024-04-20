#include "base/util.h"
#include "fmt/core.h"
#include "parser/ByteCode.h"
#include "parser/LibrarySymbols.h"
#include "parser/Types.h"
#include "parser/parserDefs.h"
#include "symbols/SymbolDefs.h"
#include <cstdio>
#include <cstdlib>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::ordered_json;

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
		case ZTID_WEBSOCKET: return &WebSocketSymbols::getInst();
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
	vector<shared_ptr<Opcode>> code;
	addOpcode2(code, new ONoOp(function->getLabel()));
	function->giveCode(code);
}

static std::vector<string> get_zasm_ops(Function* fn)
{
	std::vector<std::string> zasm;
	auto& code = fn->getCode();
	for (auto& op : code)
	{
		if (op->toString() == "RETURNFUNC")
			continue;

		std::string op_str = op->toString();
		util::replchar(op_str, ',', ' ');
		zasm.push_back(op_str);
	}
	return zasm;
}

static auto& get_symbols_json()
{
	static std::ifstream symbols_json_f("zscript_symbols.json");
	static json symbols_json = json::parse(symbols_json_f);
	return symbols_json;
}

static bool canConvertSymbolToBinding(const AccessorTable& entry)
{
	// Handled elsewhere.
	if (entry.alias_name)
		return false;

	// Just "get" is handled.
	if (entry.name.starts_with("set"))
		return false;
	if (entry.name.starts_with("_set") && (entry.funcFlags & FL_RDONLY))
		return false;

	return true;
}

static std::string get_type_binding_name(const DataType* type)
{
	if (!type) return "void";
	if (type->getName() == "T[]") return "T_ARR";
	return type->getName();
}

static std::map<std::string, CONSTEXPR_CBACK_TY> const_expr_global_impl = {
	{"Untype", CONSTEXPR_CBACK_HEADER()
		{
			return args[0];
		}
	},

	{"Pow", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0] && args[1])
			{
				if(!*args[0] && !*args[1])
					val = 1; //0^0
				else val = int(pow(*args[0]/10000.0,*args[1]/10000.0)*10000);
			}
			return val;
		}
	},

	{"LPow", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0] && args[1])
			{
				if(!*args[0] && !*args[1])
					val = 1; //0^0
				else val = int(pow(*args[0],*args[1]));
			}
			return val;
		}
	},

	{"InvPow", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0] && args[1])
			{
				double v1 = *args[0]/10000.0;
				if(!*args[1])
				{
					handler->handleError(CompileError::DivByZero(&node,"divide","InvPow(): "));
					val = 10000;
				}
				else
				{
					double v2 = 10000.0 / *args[1];
					if(!v1 && !v2)
						val = 1; //0^0
					else val = int(pow(v1,v2));
				}
			}
			return val;
		}
	},

	{"Factorial", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				auto v = *args[0] / 10000;
				if(v < 2)
					val = (v >= 0) ? 10000 : 0;
				else
				{
					int prod = 1;
					for(int q = v; v > 1; --v)
						prod *= q;
					val = prod*10000;
				}
			}
			return val;
		}
	},

	{"Abs", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				val = *args[0];
				if(*val < 0)
					val = -*val;
			}
			return val;
		}
	},

	{"Sqrt", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				double v = *args[0]/10000.0;
				if(v < 0)
				{
					handler->handleError(CompileError::NegSqrt(&node));
					val = -10000;
				}
				else
				{
					val = int(sqrt(v)*10000);
				}
			}
			return val;
		}
	},

	{"Floor", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = zslongToFix(*args[0]).doFloor().getZLong();
			return val;
		}
	},

	{"Truncate", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = zslongToFix(*args[0]).doTrunc().getZLong();
			return val;
		}
	},

	{"Round", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = zslongToFix(*args[0]).doRound().getZLong();
			return val;
		}
	},

	{"RoundAway", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = zslongToFix(*args[0]).doRoundAway().getZLong();
			return val;
		}
	},

	{"Ceiling", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = zslongToFix(*args[0]).doCeil().getZLong();
			return val;
		}
	},

	{"DegtoRad", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				double rangle = (*args[0] / 10000.0) * (PI / 180.0);
				rangle += rangle < 0?-0.00005:0.00005;
				val = int32_t(rangle * 10000.0);
			}
			return val;
		}
	},

	{"RadtoDeg", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				double rangle = (*args[0] / 10000.0) * (180.0 / PI);
				val = int32_t(rangle * 10000.0);
			}
			return val;
		}
	},

	{"WrapRadians", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				val = wrap_zslong_rad(*args[0]);
			}
			return val;
		}
	},

	{"WrapDegrees", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
			{
				val = wrap_zslong_deg(*args[0]);
			}
			return val;
		}
	},

	{"Max", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			for (const auto& arg : args)
			{
				if(!arg) //all args must be constant
					return nullopt;
				if(!val || *arg > *val)
					val = arg;
			}
			return val;
		}
	},

	{"Min", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			for (const auto& arg : args)
			{
				if(!arg) //all args must be constant
					return nullopt;
				if(!val || *arg < *val)
					val = arg;
			}
			return val;
		}
	},
};

static std::map<std::string, CONSTEXPR_CBACK_TY> const_expr_screen_impl = {
	{"LoadFFC", CONSTEXPR_CBACK_HEADER()
		{
			optional<int> val;
			if(args[0])
				val = *args[0] - 10000;
			return val;
		}
	},
};

bool setConstExprForBinding(Function* fn)
{
	std::map<std::string, CONSTEXPR_CBACK_TY> *impl_map = nullptr;

	if (fn->getInternalScope()->getParent()->isClass())
	{
		std::string class_name = fn->getInternalScope()->getParent()->getClass()->getName().value_or("");
		if (class_name == "screendata")
			impl_map = &const_expr_screen_impl;
		else
			return false;
	}
	else
	{
		impl_map = &const_expr_global_impl;
	}

	auto it = impl_map->find(fn->name);
	if (it != impl_map->end())
	{
		fn->set_constexpr(it->second);
		return true;
	}

	return false;
}

static std::vector<std::string> get_global_names_for_binding(const DataType* type)
{
	static std::vector<std::pair<const DataType*, const char *>> vars = {
		{&DataType::PLAYER, "Hero"},
		{&DataType::PLAYER, "Link"},
		{&DataType::PLAYER, "Player"},
		{&DataType::SCREEN, "Screen"},
		{&DataType::REGION, "Region"},
		{&DataType::GAME, "Game"},
		{&DataType::AUDIO, "Audio"},
		{&DataType::DEBUG, "Debug"},
		{&DataType::NPCDATA, "NPCData"},
		{&DataType::TEXT, "Text"},
		{&DataType::COMBOS, "ComboData"},
		{&DataType::SPRITEDATA, "SpriteData"},
		{&DataType::INPUT, "Input"},
		{&DataType::GRAPHICS, "Graphics"},
		{&DataType::MAPDATA, "MapData"},
		{&DataType::DMAPDATA, "DMapData"},
		{&DataType::ZMESSAGE, "MessageData"},
		{&DataType::SHOPDATA, "ShopData"},
		{&DataType::DROPSET, "DropData"},
		{&DataType::PONDS, "PondData"},
		{&DataType::WARPRING, "WarpRing"},
		{&DataType::DOORSET, "DoorSet"},
		{&DataType::ZUICOLOURS, "MiscColors"},
		{&DataType::TUNES, "MusicTrack"},
		{&DataType::PALCYCLE, "PalCycle"},
		{&DataType::GAMEDATA, "GameData"},
		{&DataType::CHEATS, "Cheats"},
		{&DataType::FILESYSTEM, "FileSystem"},
		{&DataType::ZINFO, "Module"},
		{&DataType::ZINFO, "ZInfo"},
		{&DataType::RNG, "RandGen"},
	};

	std::vector<std::string> names;
	if (!type) return names;
	for (auto [t, n] : vars)
	{
		if (t->getName() == type->getName())
			names.push_back(n);
	}
	return names;
}

static std::pair<std::string, std::vector<std::string>> find_symbol_docs(std::string indent, const AccessorTable& entry, std::string name, std::string class_name, int num_params, bool is_global, std::string symbol_name)
{
	std::string comment;
	std::vector<std::string> param_names;
	std::string class_name_lower = class_name;
	util::lowerstr(class_name_lower);
	std::string full_symbol_name = is_global ?
		symbol_name :
		fmt::format("{}->{}", class_name_lower == "eweapon" || class_name_lower == "lweapon" ? "weapon" : class_name_lower, symbol_name);
	for (auto& json : get_symbols_json())
	{
		if (json["name"] != full_symbol_name)
			continue;

		if (json["params"].is_array() && json["params"].size() != num_params)
			continue;

		comment = json["comment"];
		if (json["params"].is_array())
			param_names = json["params"];
		break;
	}

	if (!comment.empty())
	{
		// Wrap it.
		std::vector<std::string> lines;
		util::split(comment, lines, '\n');
		comment = "";

		for (int i = 0; i < lines.size(); i++)
		{
			size_t limit = 100;
			std::string wrapped = indent + "//";
			size_t current_line_len = 0;
			std::vector<std::string> words;
			util::split(lines[i], words, ' ');
			for (auto& word : words)
			{
				if (current_line_len + word.size() > limit - 3 - indent.size())
				{
					wrapped += fmt::format("\n{}//", indent);
					current_line_len = 2 + indent.size();
				}

				wrapped += ' ' + word;
				current_line_len += word.size() + 1;
			}

			comment += wrapped;
			if (i != lines.size() - 1)
				comment += "\n";
		}
	}

	return std::make_pair(comment, param_names);
}

void LibrarySymbols::processSymbol(Scope& scope, const AccessorTable& entry, std::string indent, bool is_global, std::vector<std::string>& methods, std::vector<std::string>& properties)
{
	if (!canConvertSymbolToBinding(entry))
		return;

	auto& typeStore = scope.getTypeStore();

	std::string symbol_name = entry.name;
	if (entry.name.starts_with("get"))
	{
		symbol_name = entry.name.substr(3);
	}
	else if (entry.name.starts_with("_get"))
	{
		symbol_name = entry.name.substr(4);
	}
	else if (entry.name.starts_with("_set"))
	{
		symbol_name = entry.name.substr(4);
	}
	else if (entry.name.starts_with("const"))
	{
		symbol_name = entry.name.substr(5);
	}
	if (entry.funcFlags & FL_ARR)
		symbol_name += "[]";

	auto fn = entry.alias_name ?
		getFunction(entry.alias_name.value(), entry.alias_tag) :
		getFunction(entry.name, entry.tag);
	auto entry_params = entry.params;
	if (!is_global && !entry_params.empty())
		entry_params.erase(entry_params.begin());

	auto [comment, param_names] = find_symbol_docs(indent, entry, name, this->name, entry_params.size(), is_global, symbol_name);

	auto rettype = typeStore.getType(entry.rettype);
	bool deprecated = entry.funcFlags & FL_DEPR;

	std::string decl;
	if (!comment.empty())
	{
		decl += fmt::format("{}\n", comment);
		decl += fmt::format("{}//\n", indent);
	}
	if (deprecated)
		decl += fmt::format("{}// @deprecated{}\n", indent, entry.info.empty() ? "" : " " + entry.info);

	for (int32_t j = 0; table[j].name != ""; j++)
	{
		if (table[j].alias_name == symbol_name)
		{
			decl += fmt::format("{}// @{} {}\n", indent, (table[j].funcFlags & FL_DEPR) ? "deprecated_alias" : "alias", table[j].name);
		}
	}

	if (entry.funcFlags & FL_EXIT)
	{
		decl += fmt::format("{}// @exit\n", indent);
	}

	// idk what this does tbh
	if (fn->getIntFlag(IFUNCFLAG_REASSIGNPTR))
	{
		decl += fmt::format("{}// @reassign_ptr\n", indent);
	}

	if ((entry.funcFlags & FL_VARG))
	{
		decl += fmt::format("{}// @vargs {}\n", indent, entry.extra_vargs);
	}

	std::vector<std::string> mods = {"internal"};

	if (entry.var != -1)
	{
		bool readonly = true;
		std::string setter_name = (entry.name.starts_with("_") ? "_set" : "set") + symbol_name;
		for (int32_t j = 0; table[j].name != ""; j++)
		{
			if (table[j].name == setter_name)
			{
				readonly = table[j].funcFlags & FL_RDONLY;
				break;
			}
		}

		if (entry.name.starts_with("_get"))
		{
			// These are old-style getters/setters using actual methods.
			if (fn->get_constexpr())
				mods.push_back("constexpr");
			std::string getter = symbol_name;
			auto zasm = get_zasm_ops(getFunction("_get" + symbol_name));
			decl += fmt::format("{}// @zasm\n{}//   {}\n", indent, indent, fmt::join(zasm, "\n" + indent + "//   "));
			decl += fmt::format("{}{} {} {}();\n", indent, fmt::join(mods, " "), get_type_binding_name(rettype), getter);
			methods.push_back(decl);
		}
		else if (entry.name.starts_with("_set"))
		{
			// These are old-style getters/setters using actual methods.
			if (fn->get_constexpr())
				mods.push_back("constexpr");

			std::string setter = symbol_name;
			auto zasm = get_zasm_ops(getFunction("_set" + symbol_name));
			decl += fmt::format("{}// @zasm\n{}//   {}\n", indent, indent, fmt::join(zasm, "\n" + indent + "//   "));
			decl += fmt::format("{}{} void {}({} value);\n", indent, fmt::join(mods, " "), setter, get_type_binding_name(typeStore.getType(entry_params[0])));
			methods.push_back(decl);
		}
		else
		{
			if (entry.funcFlags & FL_ARR)
			{
				decl += fmt::format("{}// @zasm_internal_array {}\n", indent, entry.var / 10000 - INTARR_OFFS);
			}
			else
			{
				decl += fmt::format("{}// @zasm_var {}\n", indent, VarToString(entry.var));
			}
			if (readonly)
				mods.push_back("const");
			decl += fmt::format("{}{} {} {};\n", indent, fmt::join(mods, " "), get_type_binding_name(rettype), symbol_name);
			properties.push_back(decl);
		}

		return;
	}

	if (fn->get_constexpr())
		mods.push_back("constexpr");

	std::vector<std::string> zasm = get_zasm_ops(fn);

	// Some symbols are just dead code (like MakeDirectional in eweapon, or SetWeaponSprite in npcdata). Ignore.
	if (zasm.empty())
		return;

	int param_count = 0;
	std::vector<std::string> params;
	for (int param : entry_params)
	{
		auto type = typeStore.getType(param);
		std::string name = param_names.empty() ?
			fmt::format("arg{}", param_count + 1) :
			param_names[param_count];
		params.push_back(fmt::format("{} {}", get_type_binding_name(type), name));
		param_count++;
	}
	// user-defined varargs do not functionally match internal varags right now ...
	// if (entry.funcFlags & FL_VARG)
	// {
	// 	auto vararg_type = &DataType::UNTYPED;
	// 	auto last_param_type = !entry_params.empty() ? typeStore.getType(entry_params.back()) : nullptr;
	// 	if (last_param_type->getName() == "T" || last_param_type->getName() == "T[]")
	// 		vararg_type = &DataType::TEMPLATE_T;
	// 	params.push_back(fmt::format("...{}[] {}", get_type_binding_name(vararg_type), "args"));
	// 	param_count++;
	// }
	for (int j = 0; j < entry.optparams.size(); j++)
	{
		int param_index = params.size() - entry.optparams.size() + j;
		std::string val;
		auto type = typeStore.getType(entry_params[param_index]);
		if (type->getName() == "bool")
		{
			val = entry.optparams[j] ? "true" : "false";
		}
		else if (type->isClass())
		{
			assert(entry.optparams[j] == 0);
			val = fmt::format("<{}>0", get_type_binding_name(type));
		}
		else
		{
			val = std::abs(entry.optparams[j]) >= 10000 || entry.optparams[j] == 0 ?
				std::to_string(entry.optparams[j] / 10000) :
				fmt::format("{}L", entry.optparams[j]);
		}
		params[param_index] += fmt::format(" = {}", val);
	}

	decl += fmt::format("{}// @zasm\n{}//   {}\n", indent, indent, fmt::join(zasm, "\n" + indent + "//   "));
	decl += fmt::format("{}{} {} {}({});\n", indent, fmt::join(mods, " "), get_type_binding_name(rettype), symbol_name, fmt::join(params, ", "));
	methods.push_back(decl);
}

static std::string get_ctor(std::string indent, std::string class_name, bool canHoldObject)
{
	if (class_name == "bitmap")
	{
		return fmt::format("{}{}", indent,
 R"(// @zasm
	//   POP D1
	//   POP D0
	//   SETR D2 CREATEBITMAP
	internal bitmap(int width = 256, int height = 256);)");
	}
	else if (class_name == "randgen")
	{
		return fmt::format("{}{}", indent,
 R"(// @zasm
	//   LOADRNG
	internal randgen();)");
	}
	else if (class_name == "stack")
	{
		return fmt::format("{}{}", indent,
 R"(// @zasm
	//   LOADSTACK
	internal stack();)");
	}
	else if (class_name == "websocket")
	{
		return fmt::format("{}{}", indent,
 R"(// @zasm
	//   POP D2
	//   WEBSOCKET_LOAD D2
	internal websocket(char32 url);)");
	}
	else if (class_name == "file")
	{
		return fmt::format("{}{}", indent,
 R"(// @zasm
	//   POP D3
	//   POP D2
	//   SETV REFFILE 0
	//   FILEOPENMODE D2 D3
	//   SETR D2 D3
	internal file(char32 path, char32 mode);)");
	}
	else if (class_name == "directory")
	{
		return fmt::format("{}{}", indent,
 R"(// @zasm
	//   POP D2
	//   LOADDIRECTORYR D2
	internal directory(char32 path);)");
	}
	else if (class_name == "paldata")
	{
		return fmt::format("{}{}", indent,
 R"(// @zasm
	//   CREATEPALDATA
	internal paldata();

	// @zasm
	//   POP D2
	//   CREATEPALDATACLR D2
	internal paldata(rgb color);)");
	}
	else if (!class_name.empty() && !canHoldObject)
	{
		std::string out;
		out += fmt::format("{}// @delete\n", indent);
		out += fmt::format("{}internal {}();", indent, class_name);
		return out;
	}
	return "";
}

void LibrarySymbols::writeSymbolsToBindings(Scope& scope)
{
	std::string output_path;
	std::string class_name = this->name;
	if (this == &GlobalSymbols::getInst())
	{
		output_path = "../../resources/include/bindings/global.zh";
	}
	else
	{
		std::string fname = class_name;
		util::lowerstr(fname);
		output_path = fmt::format("../../resources/include/bindings/{}.zh", fname);
	}

	bool is_global = class_name.empty();
	std::string indent = is_global ? "" : "\t";
	std::vector<std::string> properties;
	std::vector<std::string> methods;
	for (int32_t i = 0; table[i].name != ""; i++)
	{
		auto& entry = table[i];
		processSymbol(scope, entry, indent, is_global, methods, properties);
	}

	auto declarations = properties;
	for (auto& method : methods)
		declarations.push_back(method);

	FILE* f = fopen(output_path.c_str(), "w");
	if (class_name == "itemdata")
		fmt::println(f, "script typedef itemdata item;\n");
	if (class_name == "itemsprite")
		fmt::println(f, "typedef itemsprite item;\n");
	if (!is_global)
	{
		if (refVar != NUL)
			fmt::println(f, "// @zasm_ref {}", VarToString(refVar));
		fmt::println(f, "class {} {{", class_name);
	}

	if (!is_global)
	{
		std::string ctor = get_ctor(indent, class_name, type->canHoldObject());
		if (!ctor.empty())
			fmt::println(f, "{}\n", ctor);
	}

	fmt::print(f, "{}", fmt::join(declarations, "\n"));
	if (!is_global)
		fmt::println(f, "}}");
	// For bindings that are more like namespaces, like `Game->` - make a singleton.
	auto gnames = get_global_names_for_binding(type);
	for (int j = 0; j < gnames.size(); j++)
	{
		if (j == 0)
			fmt::println(f, "\ninternal const {} {};", class_name, gnames[j]);
		else
			fmt::println(f, "internal const {} {}; // @deprecated Use {}->", class_name, gnames[j], gnames[0]);
	}
	fclose(f);
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
			
			std::string const& name = entry.name;
			std::string varName = name;
			
			// Strip out the array at the end.
			bool isArray = name.substr(name.size() - 2) == "[]";
			if (isArray)
				varName = name.substr(0, name.size() - 2);
			
			if(entry.alias_name)
			{
				Function* func = new Function();
				func->setInfo(entry.info);
				func->setFlag(entry.funcFlags);
				func->setEntry(&entry);
				if(hasPrefixType)
					func->hasPrefixType = true;
				if (name.substr(0, 3) == "set")
				{
					assert(entry.alias_name->substr(0, 3) == "set");
					varName = varName.substr(3); // Strip out "set".
					func->name = varName;
				}
				else if (name.substr(0, 5) == "const")
				{
					assert(entry.alias_name->substr(0, 5) == "const");
					varName = varName.substr(5); // Strip out "const".
					func->name = varName;
				}
				else if (name.substr(0, 3) == "get")
				{
					assert(entry.alias_name->substr(0, 3) == "get");
					varName = varName.substr(3); // Strip out "get".
					func->name = varName;
				}
				else
				{
					func->name = varName;
				}
				alias_functions[make_pair(entry.name,entry.tag)] = func;
				continue;
			}
			
			DataType const* returnType = typeStore.getType(entry.rettype);
			vector<DataType const*> paramTypes;
			for (auto& ptype : entry.params)
				paramTypes.push_back(typeStore.getType(ptype));

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
			if(function->isNil())
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
			auto& entry = *func->getEntry();
			Function* alias_func = getAlias(*entry.alias_name, entry.alias_tag);
			func->alias(alias_func);
			if (entry.name.substr(0, 3) == "set")
				scope.addSetter(func);
			else if (entry.name.substr(0, 5) == "const")
				scope.addGetter(func);
			else if (entry.name.substr(0, 3) == "get")
				scope.addGetter(func);
			else scope.addAlias(func);
		}
		generateCode();
	}
	catch (std::exception& e)
	{
		errorstream << e.what() << '\n';
	}

	// TODO remove this after binding work is done
	if (std::getenv("MAKE_ZSCRIPT_BINDINGS") != nullptr)
		writeSymbolsToBindings(scope);

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

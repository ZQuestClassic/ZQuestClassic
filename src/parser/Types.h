#ifndef ZSCRIPT_TYPES_H
#define ZSCRIPT_TYPES_H

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <type_traits>
#include "CompilerUtils.h"
#include "parserDefs.h"

namespace ZScript
{
	////////////////////////////////////////////////////////////////
	// Forward Declarations
	class Function;
	class UserClass;
	class Scope;
	class ZClass;
	class DataType;
	class CompileErrorHandler;
	//AST
	class ASTExprIdentifier;

	typedef int32_t DataTypeId;

	////////////////////////////////////////////////////////////////
	// Stores and lookup types and classes.
	class TypeStore
	{
	public:
		TypeStore();
		~TypeStore();

		// Types
		DataType const* getType(DataTypeId typeId) const;
		std::optional<DataTypeId> getTypeId(DataType const& type) const;
		std::optional<DataTypeId> assignTypeId(DataType const& type);
		std::optional<DataTypeId> getOrAssignTypeId(DataType const& type);

		template <typename Type>
		Type const* getCanonicalType(Type const& type)
		{
			std::optional<DataTypeId> opt = getOrAssignTypeId(type);
			if (!opt)
				return nullptr;
			return static_cast<Type const*>(
					ownedTypes[*opt]);
		}
	
		// Classes
		std::vector<ZScript::ZClass*> getClasses() const {
			return ownedClasses;}
		ZScript::ZClass* getClass(int32_t classId) const;
		ZScript::ZClass* createClass(std::string const& name);

	private:
		// Comparator for pointers to types.
		struct TypeIdMapComparator {
			bool operator() (
					DataType const* const& lhs, DataType const* const& rhs)
					const;
		};

		std::vector<DataType const*> ownedTypes;
		std::map<DataType const*, DataTypeId, TypeIdMapComparator> typeIdMap;
		std::vector<ZClass*> ownedClasses;
	};

	std::vector<Function*> getClassFunctions(TypeStore const&);

	////////////////////////////////////////////////////////////////
	// Data Types

	enum DataTypeIdBuiltin
	{
		ZTID_START = 0,

		ZTID_PRIMITIVE_START = 0,
		ZTID_UNTYPED = 0,
		ZTID_VOID,
		ZTID_AUTO,
		ZTID_FLOAT,
		ZTID_CHAR,
		ZTID_BOOL,
		ZTID_LONG,
		ZTID_RGBDATA,
		ZTID_PRIMITIVE_END,

		ZTID_CLASS_START = ZTID_PRIMITIVE_END,
		ZTID_GAME = ZTID_CLASS_START,
		ZTID_PLAYER,
		ZTID_SCREEN,
		ZTID_REGION,
		ZTID_VIEWPORT,
		ZTID_FFC,
		ZTID_ITEM,
		ZTID_ITEMCLASS,
		ZTID_NPC,
		ZTID_LWPN,
		ZTID_EWPN,
		ZTID_NPCDATA,
		ZTID_DEBUG,
		ZTID_AUDIO,
		ZTID_COMBOS,
		ZTID_SPRITEDATA,
		ZTID_GRAPHICS,
		ZTID_BITMAP,
		ZTID_TEXT,
		ZTID_INPUT,
		ZTID_MAPDATA,
		ZTID_DMAPDATA,
		ZTID_ZMESSAGE,
		ZTID_SHOPDATA,
		ZTID_DROPSET,
		ZTID_PONDS,
		ZTID_WARPRING,
		ZTID_DOORSET,
		ZTID_ZUICOLOURS,
		ZTID_RGBDATAOLD, // ZTID_RGBDATA,
		ZTID_PALETTEOLD, // ZTID_PALETTE
		ZTID_TUNES,
		ZTID_PALCYCLE,
		ZTID_GAMEDATA,
		ZTID_CHEATS,
		ZTID_FILESYSTEM,
		ZTID_SUBSCREENDATA,
		ZTID_FILE,
		ZTID_ZINFO,
		ZTID_DIRECTORY,
		ZTID_RNG,
		ZTID_BOTTLETYPE,
		ZTID_BOTTLESHOP,
		ZTID_GENERICDATA,
		ZTID_STACK,
		ZTID_PALDATA,
		ZTID_PORTAL,
		ZTID_SAVPORTAL,
		ZTID_SUBSCREENPAGE,
		ZTID_SUBSCREENWIDGET,
		ZTID_CLASS_END,

		ZTID_END = ZTID_CLASS_END
	};

	static std::string getDataTypeName(DataTypeId id)
	{
		switch(id)
		{
			case ZTID_UNTYPED:
				return "UNTYPED";
			case ZTID_VOID:
				return "";
			case ZTID_FLOAT:
				return "INT";
			case ZTID_CHAR:
				return "CHAR32";
			case ZTID_LONG:
				return "LONG";
			case ZTID_BOOL:
				return "BOOL";
			case ZTID_RGBDATA:
				return "RGB";
			case ZTID_GAME:
				return "GAME";
			case ZTID_PLAYER:
				return "PLAYER";
			case ZTID_SCREEN:
				return "SCREEN";
			case ZTID_REGION:
				return "REGION";
			case ZTID_VIEWPORT:
				return "VIEWPORT";
			case ZTID_FFC:
				return "FFC";
			case ZTID_ITEM:
				return "ITEMSPRITE";
			case ZTID_ITEMCLASS:
				return "ITEMDATA";
			case ZTID_NPC:
				return "NPC";
			case ZTID_LWPN:
				return "LWEAPON";
			case ZTID_EWPN:
				return "EWEAPON";
			case ZTID_NPCDATA:
				return "NPCDATA";
			case ZTID_DEBUG:
				return "DEBUG";
			case ZTID_AUDIO:
				return "AUDIO";
			case ZTID_COMBOS:
				return "COMBOS";
			case ZTID_SPRITEDATA:
				return "SPRITEDATA";
			case ZTID_GRAPHICS:
				return "GRAPHICS";
			case ZTID_BITMAP:
				return "BITMAP";
			case ZTID_TEXT:
				return "TEXT";
			case ZTID_INPUT:
				return "INPUT";
			case ZTID_MAPDATA:
				return "MAPDATA";
			case ZTID_DMAPDATA:
				return "DMAPDATA";
			case ZTID_ZMESSAGE:
				return "ZMESSAGE";
			case ZTID_SHOPDATA:
				return "SHOPDATA";
			case ZTID_DROPSET:
				return "DROPSET";
			case ZTID_PONDS:
				return "PONDS";
			case ZTID_WARPRING:
				return "WARPRING";
			case ZTID_DOORSET:
				return "DOORSET";
			case ZTID_ZUICOLOURS:
				return "ZUICOLOURS";
			case ZTID_TUNES:
				return "TUNES";
			case ZTID_PALCYCLE:
				return "PALCYCLE";
			case ZTID_GAMEDATA:
				return "GAMEDATA";
			case ZTID_CHEATS:
				return "CHEATS";
			case ZTID_FILESYSTEM:
				return "FILESYSTEM";
			case ZTID_SUBSCREENDATA:
				return "SUBSCREENDATA";
			case ZTID_FILE:
				return "FILE";
			case ZTID_DIRECTORY:
				return "DIRECTORY";
			case ZTID_STACK:
				return "STACK";
			case ZTID_RNG:
				return "RNG";
			case ZTID_PALDATA:
				return "PALDATA";
			case ZTID_BOTTLETYPE:
				return "BOTTLETYPE";
			case ZTID_BOTTLESHOP:
				return "BOTTLESHOP";
			case ZTID_GENERICDATA:
				return "GENERICDATA";
			case ZTID_PORTAL:
				return "PORTAL";
			case ZTID_SAVPORTAL:
				return "SAVEDPORTAL";
			case ZTID_ZINFO:
				return "ZINFO";
			case ZTID_SUBSCREENPAGE:
				return "SUBSCREENPAGE";
			case ZTID_SUBSCREENWIDGET:
				return "SUBSCREENWIDGET";
			default:
				return "INT";
				/*char buf[16];
				_itoa(id,buf,10);
				std::string str(buf);
				return str;*/
		}
	}
	
	static DataTypeId getTypeId(std::string name)
	{
		if(int32_t v = atoi(name.c_str()))
			return v;
		else if(name == "UNTYPED")
			return ZTID_UNTYPED;
		else if(name == "")
			return ZTID_VOID;
		else if(name == "INT")
			return ZTID_FLOAT;
		else if(name == "CHAR32")
			return ZTID_CHAR;
		else if(name == "LONG")
			return ZTID_LONG;
		else if(name == "BOOL")
			return ZTID_BOOL;
		else if (name == "RGB")
			return ZTID_RGBDATA;
		else if(name == "GAME")
			return ZTID_GAME;
		else if(name == "PLAYER")
			return ZTID_PLAYER;
		else if(name == "SCREEN")
			return ZTID_SCREEN;
		else if(name == "REGION")
			return ZTID_REGION;
		else if(name == "VIEWPORT")
			return ZTID_VIEWPORT;
		else if(name == "FFC")
			return ZTID_FFC;
		else if(name == "ITEMSPRITE")
			return ZTID_ITEM;
		else if(name == "ITEMDATA")
			return ZTID_ITEMCLASS;
		else if(name == "NPC")
			return ZTID_NPC;
		else if(name == "LWEAPON")
			return ZTID_LWPN;
		else if(name == "EWEAPON")
			return ZTID_EWPN;
		else if(name == "NPCDATA")
			return ZTID_NPCDATA;
		else if(name == "DEBUG")
			return ZTID_DEBUG;
		else if(name == "AUDIO")
			return ZTID_AUDIO;
		else if(name == "COMBOS")
			return ZTID_COMBOS;
		else if(name == "SPRITEDATA")
			return ZTID_SPRITEDATA;
		else if(name == "GRAPHICS")
			return ZTID_GRAPHICS;
		else if(name == "BITMAP")
			return ZTID_BITMAP;
		else if(name == "TEXT")
			return ZTID_TEXT;
		else if(name == "INPUT")
			return ZTID_INPUT;
		else if(name == "MAPDATA")
			return ZTID_MAPDATA;
		else if(name == "DMAPDATA")
			return ZTID_DMAPDATA;
		else if(name == "ZMESSAGE")
			return ZTID_ZMESSAGE;
		else if(name == "SHOPDATA")
			return ZTID_SHOPDATA;
		else if(name == "DROPSET")
			return ZTID_DROPSET;
		else if(name == "PONDS")
			return ZTID_PONDS;
		else if(name == "WARPRING")
			return ZTID_WARPRING;
		else if(name == "DOORSET")
			return ZTID_DOORSET;
		else if(name == "ZUICOLOURS")
			return ZTID_ZUICOLOURS;
		// else if(name == "RGBDATA")
		// 	return ZTID_RGBDATA;
		// else if(name == "PALETTE")
		// 	return ZTID_PALETTE;
		else if(name == "TUNES")
			return ZTID_TUNES;
		else if(name == "PALCYCLE")
			return ZTID_PALCYCLE;
		else if(name == "GAMEDATA")
			return ZTID_GAMEDATA;
		else if(name == "CHEATS")
			return ZTID_CHEATS;
		else if(name == "FILESYSTEM")
			return ZTID_FILESYSTEM;
		else if(name == "SUBSCREENDATA")
			return ZTID_SUBSCREENDATA;
		else if(name == "FILE")
			return ZTID_FILE;
		else if(name == "DIRECTORY")
			return ZTID_DIRECTORY;
		else if(name == "STACK")
			return ZTID_STACK;
		else if(name == "MODULE")
			return ZTID_ZINFO;
		else if(name == "ZINFO")
			return ZTID_ZINFO;
		else if(name == "RNG")
			return ZTID_RNG;
		else if(name == "PALDATA")
			return ZTID_PALDATA;
		else if(name == "BOTTLETYPE")
			return ZTID_BOTTLETYPE;
		else if(name == "BOTTLESHOP")
			return ZTID_BOTTLESHOP;
		else if(name == "GENERICDATA")
			return ZTID_GENERICDATA;
		else if(name == "PORTAL")
			return ZTID_PORTAL;
		else if(name == "SAVEDPORTAL")
			return ZTID_SAVPORTAL;
		else if(name == "SUBSCREENPAGE")
			return ZTID_SUBSCREENPAGE;
		else if(name == "SUBSCREENWIDGET")
			return ZTID_SUBSCREENWIDGET;
		
		return ZTID_VOID;
	}
	
	class DataTypeSimple;
	class DataTypeSimpleConst;
	class DataTypeClass;
	class DataTypeClassConst;
	class DataTypeArray;
	class DataTypeCustom;
	class DataTypeCustomConst;

	class DataType
	{
	public:
		DataType(DataType* constType)
			: constType(constType ? constType->clone() : NULL)
		{}
		
		virtual ~DataType() {}
		// Call derived class's copy constructor.
		virtual DataType* clone() const = 0;

		// Resolution.
		virtual bool isResolved() const {return true;}
		virtual DataType* resolve(ZScript::Scope& scope, CompileErrorHandler* errorHandler) {return this;}
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const = 0;
		// Basics
		virtual std::string getName() const = 0;
		virtual bool canCastTo(DataType const& target) const = 0;
		virtual bool canBeGlobal() const {return true;}
		virtual DataType const* getConstType() const {return constType;}
		virtual DataType const* getMutType() const {return this;}

		// Derived class info.
		virtual bool isArray() const {return false;}
		virtual bool isClass() const {return false;}
		virtual bool isConstant() const {return false;}
		virtual bool isUntyped() const {return false;}
		virtual bool isVoid() const {return false;}
		virtual bool isAuto() const {return false;}
		virtual bool isCustom() const {return false;}
		virtual bool isUsrClass() const {return false;}
		virtual bool isLong() const {return false;}
		virtual UserClass* getUsrClass() const {return nullptr;}

		// Returns <0 if <rhs, 0, if ==rhs, and >0 if >rhs.
		int32_t compare(DataType const& rhs) const;
		
		//Static functions
		static DataType const* get(DataTypeId id);
		static DataTypeClass const* getClass(int32_t classId);
		static DataTypeCustom const* getCustom(int32_t customId) {
			return find<DataTypeCustom*>(customTypes, customId).value_or(std::add_pointer<DataTypeCustom>::type());
		};
		static void addCustom(DataTypeCustom* custom);
		static int32_t getUniqueCustomId() {return nextCustomId_++;}
		
	private:
		// Returns <0 if <rhs, 0, if ==rhs, and >0 if >rhs.
		// rhs is guaranteed to be the same class as the derived type.
		virtual int32_t selfCompare(DataType const& rhs) const = 0;

		//Static variables
		static int32_t nextCustomId_;
		static std::map<int32_t, DataTypeCustom*> customTypes;
		
		DataType* constType;
		// Standard Types.
	public:
		static DataTypeSimpleConst CAUTO;
		static DataTypeSimpleConst CUNTYPED;
		static DataTypeSimpleConst CFLOAT;
		static DataTypeSimpleConst CCHAR;
		static DataTypeSimpleConst CLONG;
		static DataTypeSimpleConst CBOOL;
		static DataTypeSimpleConst CRGBDATA;
		static DataTypeSimple UNTYPED;
		static DataTypeSimple ZAUTO;
		static DataTypeSimple ZVOID;
		static DataTypeSimple FLOAT;
		static DataTypeSimple CHAR;
		static DataTypeSimple LONG;
		static DataTypeSimple BOOL;
		static DataTypeSimple RGBDATA;
		static DataTypeArray STRING;
		//Classes: Global Pointer
		static DataTypeClassConst GAME;
		static DataTypeClassConst PLAYER;
		static DataTypeClassConst SCREEN;
		static DataTypeClassConst REGION;
		static DataTypeClassConst VIEWPORT;
		static DataTypeClassConst AUDIO;
		static DataTypeClassConst DEBUG;
		static DataTypeClassConst GRAPHICS;
		static DataTypeClassConst INPUT;
		static DataTypeClassConst TEXT;
		static DataTypeClassConst FILESYSTEM;
		static DataTypeClassConst ZINFO;
		//Class: Types
		static DataTypeClassConst CBITMAP;
		static DataTypeClassConst CCHEATS;
		static DataTypeClassConst CCOMBOS;
		static DataTypeClassConst CDOORSET;
		static DataTypeClassConst CDROPSET;
		static DataTypeClassConst CDMAPDATA;
		static DataTypeClassConst CEWPN;
		static DataTypeClassConst CFFC;
		static DataTypeClassConst CGAMEDATA;
		static DataTypeClassConst CITEM;
		static DataTypeClassConst CITEMCLASS;
		static DataTypeClassConst CLWPN;
		static DataTypeClassConst CMAPDATA;
		static DataTypeClassConst CZMESSAGE;
		static DataTypeClassConst CZUICOLOURS;
		static DataTypeClassConst CNPC;
		static DataTypeClassConst CNPCDATA;
		static DataTypeClassConst CPALCYCLE;
		static DataTypeClassConst CPALETTEOLD; //unused
		static DataTypeClassConst CPONDS;
		static DataTypeClassConst CRGBDATAOLD; //unused
		static DataTypeClassConst CSHOPDATA;
		static DataTypeClassConst CSPRITEDATA;
		static DataTypeClassConst CTUNES;
		static DataTypeClassConst CWARPRING;
		static DataTypeClassConst CSUBSCREENDATA;
		static DataTypeClassConst CFILE;
		static DataTypeClassConst CDIRECTORY;
		static DataTypeClassConst CSTACK;
		static DataTypeClassConst CRNG;
		static DataTypeClassConst CPALDATA;
		static DataTypeClassConst CBOTTLETYPE;
		static DataTypeClassConst CBOTTLESHOP;
		static DataTypeClassConst CGENERICDATA;
		static DataTypeClassConst CPORTAL;
		static DataTypeClassConst CSAVEDPORTAL;
		static DataTypeClassConst CSUBSCREENPAGE;
		static DataTypeClassConst CSUBSCREENWIDGET;
		//Class: Var Types
		static DataTypeClass BITMAP;
		static DataTypeClass CHEATS;
		static DataTypeClass COMBOS;
		static DataTypeClass DOORSET;
		static DataTypeClass DROPSET;
		static DataTypeClass DMAPDATA;
		static DataTypeClass EWPN;
		static DataTypeClass FFC;
		static DataTypeClass GAMEDATA;
		static DataTypeClass ITEM;
		static DataTypeClass ITEMCLASS;
		static DataTypeClass LWPN;
		static DataTypeClass MAPDATA;
		static DataTypeClass ZMESSAGE;
		static DataTypeClass ZUICOLOURS;
		static DataTypeClass NPC;
		static DataTypeClass NPCDATA;
		static DataTypeClass PALCYCLE;
		static DataTypeClass PALETTEOLD; //unused
		static DataTypeClass PONDS;
		static DataTypeClass RGBDATAOLD; //unused
		static DataTypeClass SHOPDATA;
		static DataTypeClass SPRITEDATA;
		static DataTypeClass TUNES;
		static DataTypeClass WARPRING;
		static DataTypeClass SUBSCREENDATA;
		static DataTypeClass FILE;
		static DataTypeClass DIRECTORY;
		static DataTypeClass STACK;
		static DataTypeClass RNG;
		static DataTypeClass PALDATA;
		static DataTypeClass BOTTLETYPE;
		static DataTypeClass BOTTLESHOP;
		static DataTypeClass GENERICDATA;
		static DataTypeClass PORTAL;
		static DataTypeClass SAVEDPORTAL;
		static DataTypeClass SUBSCREENPAGE;
		static DataTypeClass SUBSCREENWIDGET;
	};

	bool operator==(DataType const&, DataType const&);
	bool operator!=(DataType const&, DataType const&);
	bool operator<(DataType const&, DataType const&);
	bool operator<=(DataType const&, DataType const&);
	bool operator>(DataType const&, DataType const&);
	bool operator>=(DataType const&, DataType const&);

	// Get the data type stripped of consts and arrays.
	DataType const& getNaiveType(DataType const& type, Scope* scope);
	
	// Get the number of nested arrays at top level.
	int32_t getArrayDepth(DataType const&);

	class DataTypeUnresolved : public DataType
	{
	public:
		DataTypeUnresolved(ASTExprIdentifier* iden);
		~DataTypeUnresolved();
		DataTypeUnresolved* clone() const;
		
		virtual bool isResolved() const {return false;}
		virtual DataType* resolve(ZScript::Scope& scope, CompileErrorHandler* errorHandler);
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;
		
		virtual std::string getName() const;
		ASTExprIdentifier const* getIdentifier() const {return iden;}
		virtual bool canCastTo(DataType const& target) const {return false;}

	private:
		ASTExprIdentifier* iden;
		std::string name;

		int32_t selfCompare(DataType const& rhs) const;
	};

	class DataTypeSimple : public DataType
	{
	public:
		DataTypeSimple(int32_t simpleId, std::string const& name, DataType* constType);
		DataTypeSimple* clone() const {return new DataTypeSimple(*this);}

		virtual std::string getName() const {return name;}
		virtual bool canCastTo(DataType const& target) const;
		virtual bool canBeGlobal() const;
		virtual bool isConstant() const {return false;}
		virtual bool isUntyped() const {return simpleId == ZTID_UNTYPED;}
		virtual bool isVoid() const {return simpleId == ZTID_VOID;}
		virtual bool isAuto() const {return simpleId == ZTID_AUTO;}
		virtual bool isLong() const {return simpleId == ZTID_LONG;}

		int32_t getId() const {return simpleId;}
		
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;

	protected:
		int32_t simpleId;
		std::string name;

		int32_t selfCompare(DataType const& rhs) const;
	};
	
	class DataTypeSimpleConst : public DataTypeSimple
	{
	public:
		DataTypeSimpleConst(int32_t simpleId, std::string const& name);
		DataTypeSimpleConst* clone() const {return new DataTypeSimpleConst(*this);}
		
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;
		
		virtual bool isConstant() const {return true;}
		virtual DataType const* getConstType() const {return this;}
		virtual DataType const* getMutType() const {return get(getId());}
	};

	class DataTypeClass : public DataType
	{
	public:
		DataTypeClass(int32_t classId, DataType* constType);
		DataTypeClass(int32_t classId, std::string const& className, DataType* constType);
		DataTypeClass* clone() const {return new DataTypeClass(*this);}

		virtual DataType* resolve(ZScript::Scope& scope, CompileErrorHandler* errorHandler);
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;

		virtual std::string getName() const;
		virtual bool canCastTo(DataType const& target) const;
		virtual bool canBeGlobal() const {return true;}
		virtual bool isClass() const {return true;}
		virtual bool isConstant() const {return false;}

		std::string getClassName() const {return className;}
		int32_t getClassId() const {return classId;}
		
	protected:
		int32_t classId;
		std::string className;

		int32_t selfCompare(DataType const& other) const;
	};
	
	class DataTypeClassConst : public DataTypeClass
	{
	public:
		DataTypeClassConst(int32_t classId, std::string const& name);
		DataTypeClassConst* clone() const {return new DataTypeClassConst(*this);}
		
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;
		
		virtual bool isConstant() const {return true;}
		virtual DataType const* getConstType() const {return this;}
		virtual DataType const* getMutType() const {return getClass(getClassId());}
	};

	class DataTypeArray : public DataType
	{
	public:
		DataTypeArray(DataType const& elementType)
			: DataType(NULL), elementType(elementType) {}
		DataTypeArray* clone() const {return new DataTypeArray(*this);}

		virtual std::string getName() const {
			return elementType.getName() + "[]";}
		virtual bool canCastTo(DataType const& target) const;
		virtual bool canBeGlobal() const {return true;}
		virtual bool isArray() const {return true;}
		virtual bool isResolved() const {return elementType.isResolved();}
		virtual UserClass* getUsrClass() const {return elementType.getUsrClass();}

		DataType const& getElementType() const {return elementType;}
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;

	private:
		DataType const& elementType;

		int32_t selfCompare(DataType const& other) const;
	};
	
	class DataTypeCustom : public DataType
	{
	public:
		DataTypeCustom(std::string name, DataType* constType, UserClass* usrclass = nullptr, int32_t id = getUniqueCustomId())
			: DataType(constType), name(name), id(id), user_class(usrclass)
		{}
		DataTypeCustom* clone() const {return new DataTypeCustom(*this);}
		
		virtual bool isConstant() const {return false;}
		virtual bool isCustom() const {return true;}
		virtual bool isUsrClass() const {return user_class != nullptr;}
		virtual bool canBeGlobal() const {return true;}
		virtual UserClass* getUsrClass() const {return user_class;}
		virtual std::string getName() const {return name;}
		virtual bool canCastTo(DataType const& target) const;
		int32_t getCustomId() const {return id;}
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;
		
	protected:
		int32_t id;
		std::string name;
		UserClass* user_class;

		int32_t selfCompare(DataType const& other) const;
	};
	
	class DataTypeCustomConst : public DataTypeCustom
	{
	public:
		DataTypeCustomConst(std::string name, UserClass* user_class = nullptr)
			: DataTypeCustom(name, NULL, user_class)
		{}
		DataTypeCustomConst* clone() const {return new DataTypeCustomConst(*this);}
		
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;
		
		virtual bool isConstant() const {return true;}
		virtual DataType const* getConstType() const {return this;}
		virtual DataType const* getMutType() const {return getCustom(getCustomId());}
	};

	DataType const& getBaseType(DataType const&);

	////////////////////////////////////////////////////////////////
	// Script Types

	// Basically an enum.
	//the 'this' 'this->' stuff. -Z
	class ParserScriptType
	{
		friend bool operator==(ParserScriptType const& lhs, ParserScriptType const& rhs);
		friend bool operator!=(ParserScriptType const& lhs, ParserScriptType const& rhs);

	public:
		enum Id
		{
			idInvalid,
			idStart,
			idGlobal = idStart,
			idFfc,
			idItem,
			idNPC,
			idEWeapon,
			idLWeapon,
			idPlayer,
			idScreen,
			idDMap,
			idItemSprite,
			idUntyped,
			idComboData,
			idSubscreenData,
			idGenericScript,
			
			idEnd
		};
	
		ParserScriptType() : id_(idInvalid) {}
		
		std::string const& getName() const;
		ScriptType getTrueId() const
		{
			switch(id_)
			{
				case idInvalid:
					return ScriptType::None;
				case idGlobal:
					return ScriptType::Global;
				case idFfc:
					return ScriptType::FFC;
				case idItem:
					return ScriptType::Item;
				case idNPC:
					return ScriptType::NPC;
				case idEWeapon:
					return ScriptType::Ewpn;
				case idLWeapon:
					return ScriptType::Lwpn;
				case idPlayer:
					return ScriptType::Player;
				case idScreen:
					return ScriptType::Screen;
				case idDMap:
					return ScriptType::DMap;
				case idItemSprite:
					return ScriptType::ItemSprite;
				case idUntyped:
					return ScriptType::None;
				case idComboData:
					return ScriptType::Combo;
				case idSubscreenData:
					return ScriptType::EngineSubscreen;
				case idGenericScript:
					return ScriptType::Generic;
			}
			return ScriptType::None;
		}
		Id getId() const {return id_;}
		DataTypeId getThisTypeId() const;
		bool isValid() const {return id_ >= idStart && id_ < idEnd;}

		static ParserScriptType const invalid;
		static ParserScriptType const global;
		static ParserScriptType const ffc;
		static ParserScriptType const item;
		static ParserScriptType const npc;
		static ParserScriptType const eweapon;
		static ParserScriptType const lweapon;
		static ParserScriptType const player;
		static ParserScriptType const dmapdata;
		static ParserScriptType const screendata;
		static ParserScriptType const itemsprite;
		static ParserScriptType const untyped;
		static ParserScriptType const subscreendata;
		static ParserScriptType const combodata;
		static ParserScriptType const genericscr;

		ParserScriptType(Id id) : id_(id) {}

	private:
		
		Id id_;
	};

	// All invalid values are equal to each other.
	bool operator==(ParserScriptType const& lhs, ParserScriptType const& rhs);
	bool operator!=(ParserScriptType const& lhs, ParserScriptType const& rhs);
}

#endif

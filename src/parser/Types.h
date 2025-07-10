#ifndef ZSCRIPT_TYPES_H_
#define ZSCRIPT_TYPES_H_

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <type_traits>
#include "CompilerUtils.h"
#include "base/general.h"
#include "parser/Scope.h"
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

	private:
		// Comparator for pointers to types.
		struct TypeIdMapComparator {
			bool operator() (
					DataType const* const& lhs, DataType const* const& rhs)
					const;
		};

		std::vector<DataType const*> ownedTypes;
		std::map<DataType const*, DataTypeId, TypeIdMapComparator> typeIdMap;
	};

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

		ZTID_SCRIPT_TYPE_START = ZTID_PRIMITIVE_END,
		ZTID_COMBOS,
		ZTID_DMAPDATA,
		ZTID_EWPN,
		ZTID_FFC,
		ZTID_GENERICDATA,
		ZTID_ITEM,
		ZTID_ITEMCLASS,
		ZTID_LWPN,
		ZTID_NPC,
		ZTID_PLAYER,
		ZTID_SCREEN,
		ZTID_SUBSCREENDATA,
		ZTID_SCRIPT_TYPE_END,

		ZTID_END = ZTID_SCRIPT_TYPE_END
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
			case ZTID_PLAYER:
				return "PLAYER";
			case ZTID_SCREEN:
				return "SCREEN";
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
			case ZTID_COMBOS:
				return "COMBOS";
			case ZTID_DMAPDATA:
				return "DMAPDATA";
			case ZTID_SUBSCREENDATA:
				return "SUBSCREENDATA";
			default:
				return "INT";
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
		else if(name == "PLAYER")
			return ZTID_PLAYER;
		else if(name == "SCREEN")
			return ZTID_SCREEN;
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
		else if(name == "COMBOS")
			return ZTID_COMBOS;
		else if(name == "DMAPDATA")
			return ZTID_DMAPDATA;
		else if(name == "SUBSCREENDATA")
			return ZTID_SUBSCREENDATA;
		else if(name == "GENERICDATA")
			return ZTID_GENERICDATA;
		
		return ZTID_VOID;
	}
	
	class DataTypeSimple;
	class DataTypeSimpleConst;
	class DataTypeArray;
	class DataTypeCustom;
	class DataTypeCustomConst;
	
	// Get the number of nested arrays at top level.
	int32_t getArrayDepth(DataType const&);

	class DataType
	{
	public:
		DataType(DataType* constType)
			: constType(constType ? constType->clone() : NULL)
		{}
		
		virtual ~DataType() {}
		// Call derived class's copy constructor.
		virtual DataType* clone() const = 0;
		virtual int unique_type_id() const = 0;

		// Resolution.
		virtual bool isResolved() const {return true;}
		virtual DataType const* resolve(ZScript::Scope& scope, CompileErrorHandler* errorHandler) {return this;}
		virtual DataType const& getBaseType() const {return *this;}
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const = 0;
		// Basics
		virtual std::string getName() const = 0;
		virtual bool canCastTo(DataType const& target, bool allowDeprecatedArrayCast = true) const = 0;
		bool canCastTo(DataType const& target, const Scope& scope) const
		{
			bool legacy_arrays_opt = *lookupOption(scope, CompileOption::OPT_LEGACY_ARRAYS) != 0;
			return canCastTo(target, legacy_arrays_opt);
		};
		virtual DataType const& getShared(DataType const& target, Scope const* scope) const = 0;
		virtual bool canHoldObject() const {return getScriptObjectTypeId() != script_object_type::none;}
		bool isObject() const {return getScriptObjectTypeId() != script_object_type::none;}
		virtual script_object_type getScriptObjectTypeId() const {return script_object_type::none;}
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
		virtual bool isEnum() const {return false;}
		virtual bool isBitflagsEnum() const {return false;}
		virtual bool isUsrClass() const {return false;}
		virtual bool isLong() const {return false;}
		virtual bool isTemplate() const {return false;}
		virtual UserClass* getUsrClass() const {return nullptr;}
		
		int32_t getArrayDepth() const {return ZScript::
			getArrayDepth(*this);}

		// Returns <0 if <rhs, 0, if ==rhs, and >0 if >rhs.
		int32_t compare(DataType const& rhs) const;
		
		//Static functions
		static DataType const* get(DataTypeId id);
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
		static DataTypeSimpleConst CTEMPLATE_T;
		static DataTypeSimpleConst CAUTO;
		static DataTypeSimpleConst CUNTYPED;
		static DataTypeSimpleConst CFLOAT;
		static DataTypeSimpleConst CCHAR;
		static DataTypeSimpleConst CLONG;
		static DataTypeSimpleConst CBOOL;
		static DataTypeSimpleConst CRGBDATA;
		static DataTypeSimple TEMPLATE_T;
		static DataTypeSimple UNTYPED;
		static DataTypeSimple ZAUTO;
		static DataTypeSimple ZVOID;
		static DataTypeSimple FLOAT;
		static DataTypeSimple CHAR;
		static DataTypeSimple LONG;
		static DataTypeSimple BOOL;
		static DataTypeSimple RGBDATA;
		static const DataTypeArray* STRING;
	};

	bool operator==(DataType const&, DataType const&);
	bool operator!=(DataType const&, DataType const&);
	bool operator<(DataType const&, DataType const&);
	bool operator<=(DataType const&, DataType const&);
	bool operator>(DataType const&, DataType const&);
	bool operator>=(DataType const&, DataType const&);

	// Get the data type stripped of consts and arrays.
	DataType const& getNaiveType(DataType const& type, Scope* scope);

	class DataTypeUnresolved : public DataType
	{
	public:
		DataTypeUnresolved(ASTExprIdentifier* iden);
		~DataTypeUnresolved();
		DataTypeUnresolved* clone() const;
		int unique_type_id() const { return 1; }
		
		virtual bool isResolved() const {return false;}
		virtual DataType const* resolve(ZScript::Scope& scope, CompileErrorHandler* errorHandler);
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;
		
		virtual std::string getName() const;
		ASTExprIdentifier const* getIdentifier() const {return iden;}
		virtual bool canCastTo(DataType const& target, bool allowDeprecatedArrayCast = true) const {return false;}
		virtual DataType const& getShared(DataType const& target, Scope const* scope) const;

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
		int unique_type_id() const { return 2; }

		virtual std::string getName() const {return name;}
		virtual bool canCastTo(DataType const& target, bool allowDeprecatedArrayCast = true) const;
		virtual DataType const& getShared(DataType const& target, Scope const* scope) const;
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
		int unique_type_id() const { return 3; }
		
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;
		
		virtual bool isConstant() const {return true;}
		virtual DataType const* getConstType() const {return this;}
		virtual DataType const* getMutType() const {return get(getId());}
	};

	class DataTypeArray : public DataType
	{
	public:
		static DataTypeArray const* create(DataType const& elementType);
		static DataTypeArray const* create_depth(DataType const& elementType, uint depth);
		static DataTypeArray const* create_owning(DataType* elementType);
		explicit DataTypeArray(DataType const& elementType)
			: DataType(NULL), elementType(elementType), owned_type() {}
		DataTypeArray* clone() const {return new DataTypeArray(*this);}
		int unique_type_id() const { return 4; }

		virtual std::string getName() const {
			return elementType.getName() + "[]";}
		virtual bool canCastTo(DataType const& target, bool allowDeprecatedArrayCast = true) const;
		virtual DataType const& getShared(DataType const& target, Scope const* scope) const;
		virtual bool canHoldObject() const {return elementType.canHoldObject();}
		virtual script_object_type getScriptObjectTypeId() const {return script_object_type::array;}
		
		virtual bool isArray() const {return true;}
		virtual bool isTemplate() const {return elementType.isTemplate();}
		virtual bool isResolved() const {return elementType.isResolved();}
		virtual bool isAuto() const {return elementType.isAuto();}
		virtual bool isConstant() const {return elementType.isConstant();}
		virtual UserClass* getUsrClass() const {return elementType.getUsrClass();}

		DataType const& getElementType() const {return elementType;}
		virtual DataType const& getBaseType() const;
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;
		
		virtual DataType const* getConstType() const;
		virtual DataType const* getMutType() const;

		static std::vector<std::unique_ptr<DataTypeArray>> created_arr_types;

	private:
		DataType const& elementType;
		std::shared_ptr<DataType> owned_type;
		
		int32_t selfCompare(DataType const& other) const;
	};
	
	class DataTypeCustom : public DataType
	{
	public:
		DataTypeCustom(std::string name, DataType* constType, UserClass* usrclass = nullptr, int32_t id = getUniqueCustomId())
			: DataType(constType), id(id), name(name), user_class(usrclass), source(nullptr)
		{}
		DataTypeCustom* clone() const {return new DataTypeCustom(*this);}
		int unique_type_id() const { return 5; }
		
		virtual bool isConstant() const {return false;}
		virtual bool isCustom() const {return true;}
		virtual bool isEnum() const;
		virtual bool isBitflagsEnum() const;
		virtual bool isLong() const;
		virtual bool isUsrClass() const {return user_class != nullptr;}
		virtual bool canHoldObject() const;
		virtual script_object_type getScriptObjectTypeId() const {
			std::string name = getName();
			if (name == "bitmap") return script_object_type::bitmap;
			if (name == "directory") return script_object_type::dir;
			if (name == "file") return script_object_type::file;
			if (name == "paldata") return script_object_type::paldata;
			if (name == "randgen") return script_object_type::rng;
			if (name == "stack") return script_object_type::stack;
			if (name == "websocket") return script_object_type::websocket;
			if (canHoldObject())
				return script_object_type::object;
			return script_object_type::none;
		}
		virtual UserClass* getUsrClass() const {return user_class;}
		virtual std::string getName() const {return name;}
		virtual bool canCastTo(DataType const& target, bool allowDeprecatedArrayCast = true) const;
		virtual DataType const& getShared(DataType const& target, Scope const* scope) const;
		int32_t getCustomId() const {return id;}
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;

		void setSource(AST* source_) {source = source_;}
		const AST* getSource() const {return source;}
		
	protected:
		int32_t id;
		std::string name;
		UserClass* user_class;
		AST* source;

		int32_t selfCompare(DataType const& other) const;
	};
	
	class DataTypeCustomConst : public DataTypeCustom
	{
	public:
		DataTypeCustomConst(std::string name, UserClass* user_class = nullptr)
			: DataTypeCustom(name, NULL, user_class)
		{}
		DataTypeCustomConst* clone() const {return new DataTypeCustomConst(*this);}
		int unique_type_id() const { return 6; }
		
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const;
		
		virtual bool isConstant() const {return true;}
		virtual DataType const* getConstType() const {return this;}
		virtual DataType const* getMutType() const {return getCustom(getCustomId());}
	};
	
	class DataTypeTemplateConst;
	class DataTypeTemplate : public DataType
	{
	public:
		static DataTypeTemplate* create(std::string const& name);
		DataTypeTemplate* clone() const {return new DataTypeTemplate(*this);}
		int unique_type_id() const { return 7; }

		virtual std::string getName() const {return name;}
		uint32_t getId() const {return id;}
		virtual bool canCastTo(DataType const& target, bool allowDeprecatedArrayCast = true) const;
		virtual DataType const& getShared(DataType const& target, Scope const* scope) const;
		
		virtual bool isTemplate() const {return true;}
		
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const {return this;}
	protected:
		DataTypeTemplate(std::string const& name, uint32_t id, DataTypeTemplateConst*);
		std::string name;
		uint32_t id;

		int32_t selfCompare(DataType const& rhs) const;
	};
	
	class DataTypeTemplateConst : public DataTypeTemplate
	{
	public:
		DataTypeTemplateConst(std::string const& name, uint32_t id)
			: DataTypeTemplate(name, id, nullptr), mut_type(nullptr) {}
		DataTypeTemplateConst* clone() const {return new DataTypeTemplateConst(*this);};
		
		virtual DataType const* baseType(ZScript::Scope& scope, CompileErrorHandler* errorHandler) const {return this;}
		
		virtual bool isConstant() const {return true;}
		virtual DataType const* getConstType() const {return this;}
		virtual DataType const* getMutType() const {return mut_type;}
		
		DataType const* mut_type;
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
					return ScriptType::Hero;
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

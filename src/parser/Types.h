#ifndef ZSPARSER_TYPES_H
#define ZSPARSER_TYPES_H

#include <string>
#include <functional>
#include <iostream>
#include <vector>
#include <map>
#include "CompilerUtils.h"

using std::string;

// Forward Declarations
namespace ZScript
{
	class Function;
	class Scope;
	class ZClass;

	////////////////////////////////////////////////////////////////
	// Data Types

	typedef int DataTypeId;

	// I can't figure out a better way to do this in C++98.
	enum DataTypeClassId
	{
	ZVARTYPE_CLASSID_BASE,
	ZVARTYPE_CLASSID_SIMPLE,
	ZVARTYPE_CLASSID_UNRESOLVED,
	ZVARTYPE_CLASSID_CONST_FLOAT,
	ZVARTYPE_CLASSID_CLASS,
	ZVARTYPE_CLASSID_ARRAY
	};

	enum DataTypeIdBuiltin
	{
	ZVARTYPEID_START = 0,

	ZVARTYPEID_PRIMITIVE_START = 0,
    ZVARTYPEID_UNTYPED = 0,
    ZVARTYPEID_VOID,
    ZVARTYPEID_FLOAT,
    ZVARTYPEID_BOOL,
	ZVARTYPEID_PRIMITIVE_END,

	ZVARTYPEID_CONST_FLOAT = ZVARTYPEID_PRIMITIVE_END,

	ZVARTYPEID_CLASS_START,
    ZVARTYPEID_GAME = ZVARTYPEID_CLASS_START,
    ZVARTYPEID_LINK,
    ZVARTYPEID_SCREEN,
    ZVARTYPEID_FFC,
    ZVARTYPEID_ITEM,
    ZVARTYPEID_ITEMCLASS,
    ZVARTYPEID_NPC,
    ZVARTYPEID_LWPN,
    ZVARTYPEID_EWPN,
    ZVARTYPEID_NPCDATA,
    ZVARTYPEID_DEBUG,
    ZVARTYPEID_AUDIO,
    ZVARTYPEID_COMBOS,
    ZVARTYPEID_SPRITEDATA,
    ZVARTYPEID_GRAPHICS,
    ZVARTYPEID_TEXT,
    ZVARTYPEID_INPUT,
    ZVARTYPEID_MAPDATA,
    ZVARTYPEID_DMAPDATA,
    ZVARTYPEID_ZMESSAGE,
    ZVARTYPEID_SHOPDATA,
    ZVARTYPEID_DROPSET,
    ZVARTYPEID_PONDS,
    ZVARTYPEID_WARPRING,
    ZVARTYPEID_DOORSET,
    ZVARTYPEID_ZUICOLOURS,
    ZVARTYPEID_RGBDATA,
    ZVARTYPEID_PALETTE,
    ZVARTYPEID_TUNES,
    ZVARTYPEID_PALCYCLE,
    ZVARTYPEID_GAMEDATA,
	ZVARTYPEID_CHEATS,
	ZVARTYPEID_CLASS_END,

	ZVARTYPEID_END = ZVARTYPEID_CLASS_END
	};

	class DataTypeSimple;
	class DataTypeConstFloat;
	class DataTypeClass;
	class DataTypeArray;

	class DataType
	{
	public:
		virtual DataType* clone() const = 0;
	virtual string getName() const = 0;
		virtual DataType* resolve(ZScript::Scope& scope) {return this;}
	virtual bool isResolved() const {return true;}
	virtual bool canBeGlobal() const {return false;}
		virtual bool canCastTo(DataType const& target) const = 0;
	virtual int typeClassId() const {return ZVARTYPE_CLASSID_BASE;};

		int compare(DataType const& other) const;
		bool operator==(DataType const& other) const {return compare(other) == 0;}
		bool operator!=(DataType const& other) const {return compare(other) != 0;}
		bool operator<(DataType const& other) const {return compare(other) < 0;}

	// Comparator for pointers to types.
		struct PointerLess : public std::less<DataType*> {
			bool operator() (DataType* const& a, DataType* const& b) const
		{
			if (b == NULL) return false;
			if (a == NULL) return true;
			return *a < *b;
		}
		bool operator() (
					DataType const* const& a, DataType const* const& b) const
		{
			if (b == NULL) return false;
			if (a == NULL) return true;
			return *a < *b;
		}
	};

	// This comes up so often I'm adding in this shortcut.
	bool isArray() const {return typeClassId() == ZVARTYPE_CLASSID_ARRAY;}

	// Get the number of nested arrays at top level.
	int getArrayDepth() const;
	
	protected:
		virtual int selfCompare(DataType const& other) const = 0;

		// Standard Types.
	public:
		static DataTypeSimple const UNTYPED;
		static DataTypeSimple const ZVOID;
		static DataTypeSimple const FLOAT;
		static DataTypeSimple const BOOL;
		static DataTypeConstFloat const CONST_FLOAT;
		static DataTypeClass const FFC;
		static DataTypeClass const ITEM;
		static DataTypeClass const ITEMCLASS;
		static DataTypeClass const NPC;
		static DataTypeClass const LWPN;
		static DataTypeClass const EWPN;
		static DataTypeClass const GAME;
		static DataTypeClass const LINK;
		static DataTypeClass const SCREEN;
		static DataTypeClass const AUDIO;
		static DataTypeClass const DEBUG;
		static DataTypeClass const NPCDATA;
		static DataTypeClass const COMBOS;
		static DataTypeClass const SPRITEDATA;
		static DataTypeClass const GRAPHICS;
		static DataTypeClass const TEXT;
		static DataTypeClass const INPUT;
		static DataTypeClass const MAPDATA;
		static DataTypeClass const DMAPDATA;
		static DataTypeClass const ZMESSAGE;
		static DataTypeClass const SHOPDATA;
		static DataTypeClass const DROPSET;
		static DataTypeClass const PONDS;
		static DataTypeClass const WARPRING;
		static DataTypeClass const DOORSET;
		static DataTypeClass const ZUICOLOURS;
		static DataTypeClass const RGBDATA;
		static DataTypeClass const PALETTE;
		static DataTypeClass const TUNES;
		static DataTypeClass const PALCYCLE;
		static DataTypeClass const GAMEDATA;
		static DataTypeClass const CHEATS;
		static DataType const* get(DataTypeId id);
	};

	class DataTypeSimple : public DataType
	{
	public:
		DataTypeSimple(int simpleId, string const& name, string const& upName)
			: simpleId(simpleId), name(name), upName(upName) {}
		DataTypeSimple* clone() const {return new DataTypeSimple(*this);}
	string getName() const {return name;}
	string getUpName() const {return upName;}
	bool canBeGlobal() const;
		bool canCastTo(DataType const& target) const;
	int getId() const {return simpleId;}
	int typeClassId() const {return ZVARTYPE_CLASSID_SIMPLE;}
	protected:
		int selfCompare(DataType const& other) const;
	private:
	int simpleId;
	string name;
	string upName;
	};

	class DataTypeUnresolved : public DataType
	{
	public:
		DataTypeUnresolved(string const& name) : name(name) {}
		DataTypeUnresolved* clone() const {return new DataTypeUnresolved(*this);}
	string getName() const {return name;}
		DataType* resolve(ZScript::Scope& scope);
	bool isResolved() const {return false;}
		bool canCastTo(DataType const& target) const {return false;}
	int typeClassId() const {return ZVARTYPE_CLASSID_UNRESOLVED;}
	protected:
		int selfCompare(DataType const& other) const;
	private:
	string name;
	};

	// Temporary while only floats can be constant.
	class DataTypeConstFloat : public DataType
	{
	public:
		DataTypeConstFloat() {}
		DataType* clone() const {return new DataTypeConstFloat(*this);}
	string getName() const {return "const float";}
		DataType* resolve(ZScript::Scope& scope) {return this;}
	bool canBeGlobal() const {return true;}
		bool canCastTo(DataType const& target) const;
	int typeClassId() const {return ZVARTYPE_CLASSID_CONST_FLOAT;};
	protected:
		int selfCompare(DataType const& other) const {return 0;};
	};

	class DataTypeClass : public DataType
	{
	public:
		DataTypeClass(int classId) : classId(classId), className("") {}
		DataTypeClass(int classId, string const& className) : classId(classId), className(className) {}
		DataTypeClass* clone() const {return new DataTypeClass(*this);}
	string getName() const;
	string getClassName() const {return className;}
	int getClassId() const {return classId;}
		DataType* resolve(ZScript::Scope& scope);
	bool canBeGlobal() const {return true;}
		bool canCastTo(DataType const& target) const;
	int typeClassId() const {return ZVARTYPE_CLASSID_CLASS;}
	protected:
		int selfCompare(DataType const& other) const;
	private:
	int classId;
	string className;
	};

	class DataTypeArray : public DataType
	{
	public:
		DataTypeArray(DataType const& elementType) : elementType(elementType) {}
		DataTypeArray* clone() const {return new DataTypeArray(*this);}

	int typeClassId() const {return ZVARTYPE_CLASSID_ARRAY; }

	string getName() const {return elementType.getName() + "[]";}
		DataType* resolve(ZScript::Scope& scope) {return this;}

	bool canBeGlobal() const {return true;}
		bool canCastTo(DataType const& target) const;
		DataType const& getElementType() const {return elementType;}
		DataType const& getBaseType() const;
	protected:
		int selfCompare(DataType const& other) const;
	private:
		DataType const& elementType;
	};

	////////////////////////////////////////////////////////////////
	// Stores and lookup types and classes.
	class TypeStore
	{
	public:
		TypeStore();
		~TypeStore();

		// Types
		DataType const* getType(DataTypeId typeId) const;
		optional<DataTypeId> getTypeId(DataType const& type) const;
		optional<DataTypeId> assignTypeId(DataType const& type);
		optional<DataTypeId> getOrAssignTypeId(DataType const& type);

		template <typename Type>
		Type const* getCanonicalType(Type const& type)
		{
			return static_cast<Type const*>(
					ownedTypes[*getOrAssignTypeId(type)]);
		}
	
		// Classes
		std::vector<ZScript::ZClass*> getClasses() const {
			return ownedClasses;}
		ZScript::ZClass* getClass(int classId) const;
		ZScript::ZClass* createClass(string const& name);

	private:
		std::vector<DataType const*> ownedTypes;
		std::map<DataType const*, DataTypeId, DataType::PointerLess>
			typeIdMap;
		std::vector<ZClass*> ownedClasses;
	};

	std::vector<Function*> getClassFunctions(TypeStore const&);

	////////////////////////////////////////////////////////////////
	// Script Types

	// Basically an enum.
	class ScriptType
	{
	public:
		ScriptType()
			: id(ID_NULL), name("null"), thisTypeId(ZVARTYPEID_VOID)
		{}
		
		bool operator==(ScriptType const& other) const {
			return id == other.id;}
		string const& getName() const {return name;}
		DataTypeId getThisTypeId() const {return thisTypeId;}
		bool isNull() const {return id == ID_NULL;}

		static ScriptType const GLOBAL;
		static ScriptType const FFC;
		static ScriptType const ITEM;
		
	private:
		enum Id {ID_NULL, ID_GLOBAL, ID_FFC, ID_ITEM};
		
		ScriptType(Id id, string const& name, DataTypeId thisTypeId)
			: id(id), name(name), thisTypeId(thisTypeId)
		{}

		int id;
		string name;
		DataTypeId thisTypeId;
	};
}

#endif

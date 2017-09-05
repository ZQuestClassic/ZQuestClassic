#ifndef ZSPARSER_TYPES_H
#define ZSPARSER_TYPES_H

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "CompilerUtils.h"

using std::string;

namespace ZScript
{
	////////////////////////////////////////////////////////////////
	// Forward Declarations
	class Function;
	class Scope;
	class ZClass;
	class DataType;

	typedef int DataTypeId;

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
		ZVARTYPEID_START = 0,

		ZVARTYPEID_PRIMITIVE_START = 0,
		ZVARTYPEID_VOID = 0, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL,
		ZVARTYPEID_PRIMITIVE_END,

		ZVARTYPEID_CONST_FLOAT = ZVARTYPEID_PRIMITIVE_END,

		ZVARTYPEID_CLASS_START,
		ZVARTYPEID_GAME = ZVARTYPEID_CLASS_START, ZVARTYPEID_LINK, ZVARTYPEID_SCREEN,
		ZVARTYPEID_FFC, ZVARTYPEID_ITEM, ZVARTYPEID_ITEMCLASS, ZVARTYPEID_NPC, ZVARTYPEID_LWPN, ZVARTYPEID_EWPN,
		ZVARTYPEID_AUDIO, ZVARTYPEID_DEBUG, ZVARTYPEID_NPCDATA,
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

		// Returns <0 if <rhs, 0, if ==rhs, and >0 if >rhs.
		int compare(DataType const& rhs) const;

		// Derived class info.
		virtual bool isArray() const {return false;}
		virtual bool isClass() const {return false;}

		// Get the number of nested arrays at top level.
		int getArrayDepth() const;

	private:
		// Returns <0 if <rhs, 0, if ==rhs, and >0 if >rhs.
		// rhs is guaranteed to be the same class as the derived type.
		virtual int selfCompare(DataType const& rhs) const = 0;

		// Standard Types.
	public:
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
		static DataTypeClass const _LINK; //Rename this. "LINK" is still constant somewhere.
		static DataTypeClass const SCREEN;
		static DataTypeClass const AUDIO;
		static DataTypeClass const DEBUG;
		static DataTypeClass const NPCDATA;
		static DataType const* get(DataTypeId id);
	};

	bool operator==(DataType const&, DataType const&);
	bool operator!=(DataType const&, DataType const&);
	bool operator<(DataType const&, DataType const&);
	bool operator<=(DataType const&, DataType const&);
	bool operator>(DataType const&, DataType const&);
	bool operator>=(DataType const&, DataType const&);
	
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

	private:
		int simpleId;
		string name;
		string upName;

		int selfCompare(DataType const& rhs) const;
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

	private:
		string name;

		int selfCompare(DataType const& rhs) const;
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

	private:
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

		bool isClass() const {return true;}
		
	private:
		int classId;
		string className;

		int selfCompare(DataType const& other) const;
	};

	class DataTypeArray : public DataType
	{
	public:
		DataTypeArray(DataType const& elementType) : elementType(elementType) {}
		DataTypeArray* clone() const {return new DataTypeArray(*this);}

		string getName() const {return elementType.getName() + "[]";}
		DataType* resolve(ZScript::Scope& scope) {return this;}

		bool canBeGlobal() const {return true;}
		bool canCastTo(DataType const& target) const;
		DataType const& getElementType() const {return elementType;}
		DataType const& getBaseType() const;

		bool isArray() const {return true;}

	private:
		DataType const& elementType;

		int selfCompare(DataType const& other) const;
	};

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

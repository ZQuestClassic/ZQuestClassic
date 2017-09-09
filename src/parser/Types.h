#ifndef ZSPARSER_TYPES_H
#define ZSPARSER_TYPES_H

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "CompilerUtils.h"

namespace ZScript
{
	////////////////////////////////////////////////////////////////
	// Forward Declarations
	class Function;
	class Scope;
	class ZClass;

	class DataType;

	namespace detail
	{
		class DataTypeImpl;
		class DataTypeImplNull;
		class DataTypeImplUnresolved;
		class DataTypeImplSimple;
		class DataTypeImplConstFloat;
		class DataTypeImplClass;
		class DataTypeImplArray;
		struct DataTypeImplPtrLess;
	}

	////////////////////////////////////////////////////////////////
	// Stores and lookup types and classes.
	class TypeStore
	{
	public:
		
		TypeStore();
		~TypeStore();

		// Builtin type accessors
		DataType getVoid();
		DataType getBool();
		DataType getFloat();
		DataType getConstFloat();
		DataType getGame();
		DataType getDebug();
		DataType getScreen();
		DataType getAudio();
		DataType getLink();
		DataType getItemClass();
		DataType getItem();
		DataType getNpcClass();
		DataType getNpc();
		DataType getFfc();
		DataType getLWpn();
		DataType getEWpn();

		// Type creators
		DataType getUnresolved(std::string const& name);
		DataType getArrayOf(DataType const& elementType);
		
		// Classes
		std::vector<ZClass*> getClasses() const {return ownedClasses;}
		ZScript::ZClass* getClass(int classId) const;
		ZScript::ZClass* createClass(std::string const& name);

	private:
		struct DataTypeImplPtrLess
		{
			bool operator()(detail::DataTypeImpl const* const& lhs,
			                detail::DataTypeImpl const* const& rhs)
					const;
		};
		
		std::set<detail::DataTypeImpl const*, DataTypeImplPtrLess>
			ownedDataTypes;
		std::vector<ZClass*> ownedClasses;

		detail::DataTypeImplClass const* dataTypeGame;
		detail::DataTypeImplClass const* dataTypeDebug;
		detail::DataTypeImplClass const* dataTypeScreen;
		detail::DataTypeImplClass const* dataTypeAudio;
		detail::DataTypeImplClass const* dataTypeLink;
		detail::DataTypeImplClass const* dataTypeItemClass;
		detail::DataTypeImplClass const* dataTypeItem;
		detail::DataTypeImplClass const* dataTypeNpcClass;
		detail::DataTypeImplClass const* dataTypeNpc;
		detail::DataTypeImplClass const* dataTypeFfc;
		detail::DataTypeImplClass const* dataTypeLWpn;
		detail::DataTypeImplClass const* dataTypeEWpn;

		// Used to initialize the above class types.
		detail::DataTypeImplClass const* buildClass(
				std::string const& name, int id);
	};

	std::vector<Function*> getClassFunctions(TypeStore const&);

	////////////////////////////////////////////////////////////////
	// Data Types

	class DataType
	{
		friend class TypeStore;
		friend class detail::DataTypeImplUnresolved;
		
	public:
		DataType();

		TypeStore& getTypeStore() const {return *container;}
		
		// Type Resolution
		bool isResolved() const;
		void resolve(Scope&);

		// Basics
		std::string getName() const;
		bool canCastTo(DataType const& target) const;
		bool canBeGlobal() const;

		// Others
		bool isArray() const;
		optional<DataType> getElementType() const;
		bool isConst() const;
		ZClass* getClass() const;
		
		// Returns <0 if <rhs, 0, if ==rhs, and >0 if >rhs.
		int compare(DataType const& rhs) const;

	private:
		TypeStore* container;
		detail::DataTypeImpl const* impl;

		DataType(TypeStore* container, detail::DataTypeImpl const* impl)
			: container(container), impl(impl) {}
	};

	DataType arrayType(DataType const& elementType);
	
	bool operator==(DataType const&, DataType const&);
	bool operator!=(DataType const&, DataType const&);
	bool operator<(DataType const&, DataType const&);
	bool operator<=(DataType const&, DataType const&);
	bool operator>(DataType const&, DataType const&);
	bool operator>=(DataType const&, DataType const&);
	
	// Get the number of nested arrays at top level.
	int getArrayDepth(DataType const&);
	
	namespace detail
	{		
		// virtual base implementation.
		class DataTypeImpl
		{
		public:
			virtual ~DataTypeImpl() {}
			
			// Type Resolution
			virtual bool isResolved() const {return true;}
			virtual DataTypeImpl const* resolve(Scope&) const {
				return this;}

			// Basics
			virtual std::string getName() const = 0;
			virtual bool canCastTo(DataTypeImpl const& target) const = 0;
			virtual bool canBeGlobal() const {return true;}

			// Specific Subclass calls.
			virtual bool isConst() const {return false;}
			virtual DataTypeImpl const* unconst() const {return NULL;}
			virtual ZClass* getClass() const {return NULL;}
			virtual DataTypeImpl const* getElementType() const {
				return NULL;}
			
			// Returns <0 if <rhs, 0, if ==rhs, and >0 if >rhs.
			int compare(DataTypeImpl const& rhs) const;

		private:
			// Returns <0 if <rhs, 0, if ==rhs, and >0 if >rhs.
			// rhs is guaranteed to be the same class as the derived type.
			virtual int selfCompare(DataTypeImpl const& rhs) const = 0;
		};

		bool operator==(DataTypeImpl const&, DataTypeImpl const&);
		bool operator!=(DataTypeImpl const&, DataTypeImpl const&);
		bool operator<(DataTypeImpl const&, DataTypeImpl const&);
		bool operator<=(DataTypeImpl const&, DataTypeImpl const&);
		bool operator>(DataTypeImpl const&, DataTypeImpl const&);
		bool operator>=(DataTypeImpl const&, DataTypeImpl const&);
	
		// The null implementation.
		class DataTypeImplNull : public DataTypeImpl
		{
		public:
			static DataTypeImplNull const& singleton();
			
			virtual bool isResolved() const {return false;}
			virtual DataTypeImplNull const* resolve(Scope&) const {
				return this;}

			virtual std::string getName() const {return "NULL";}
			virtual bool canCastTo(DataTypeImpl const& target) const {
				return false;}
			virtual bool canBeGlobal() const {return false;}

		private:
			virtual int selfCompare(DataTypeImpl const& rhs) const {
				return 0;}
		};
		
		// An unresolved data type.
		class DataTypeImplUnresolved : public DataTypeImpl
		{
		public:
			DataTypeImplUnresolved(std::string const& name) : name(name) {}

			virtual bool isResolved() const {return false;}
			virtual DataTypeImpl const* resolve(Scope&) const;

			virtual std::string getName() const {return name;}
			virtual bool canCastTo(DataTypeImpl const& target) const {
				return false;}

		private:
			std::string name;

			virtual int selfCompare(DataTypeImpl const& rhs) const;
		};

		// A builtin basic data type.
		class DataTypeImplSimple : public DataTypeImpl
		{
		public:
			static DataTypeImplSimple const& getVoid();
			static DataTypeImplSimple const& getBool();
			static DataTypeImplSimple const& getFloat();
			
			virtual DataTypeImplSimple const* resolve(Scope&) const {
				return this;}
		
			virtual std::string getName() const {return name;}
			virtual bool canCastTo(DataTypeImpl const& target) const;

			int getId() const {return id;}

		private:
			enum Id {Id_Invalid, Id_Void, Id_Bool, Id_Float};
			
			Id id;
			std::string name;

			DataTypeImplSimple(Id id, std::string const& name);

			int selfCompare(DataTypeImpl const& rhs) const;
		};

		// For const float only.
		class DataTypeImplConstFloat : public DataTypeImpl
		{
		public:
			static DataTypeImplConstFloat const& singleton();
		
			virtual DataTypeImplConstFloat const* resolve(Scope&) const {
				return this;}

			virtual std::string getName() const {return "const float";}
			virtual bool canCastTo(DataTypeImpl const& target) const;
			virtual bool canBeGlobal() const {return true;}
			virtual bool isConst() const {return true;}
			virtual DataTypeImpl const* unconst() const {
				return &DataTypeImplSimple::getFloat();}

		private:
			DataTypeImplConstFloat() {}

			int selfCompare(DataTypeImpl const& rhs) const {return 0;};
		};

		// A data type for a specific class.
		class DataTypeImplClass : public DataTypeImpl
		{
			friend class ZScript::TypeStore;
			
		public:
			DataTypeImplClass(ZClass& klass) : klass(klass) {}

			virtual DataTypeImplClass const* resolve(Scope&) const {
				return this;}

			virtual std::string getName() const;
			virtual bool canCastTo(DataTypeImpl const& target) const;
			virtual bool canBeGlobal() const {return true;}
			virtual ZClass* getClass() const {return &klass;}
		
		private:
			ZClass& klass;

			int selfCompare(DataTypeImpl const& rhs) const;
		};

		class DataTypeImplArray : public DataTypeImpl
		{
			friend class TypeStore;
		public:
			DataTypeImplArray(DataTypeImpl const& elementType)
				: elementType(elementType) {}

			virtual DataTypeImplArray const* resolve(Scope&) const {
				return this;}

			virtual std::string getName() const {
				return elementType.getName() + "[]";}
			virtual bool canCastTo(DataTypeImpl const& target) const;
			virtual DataTypeImpl const* getElementType() const {
				return &elementType;}

		private:
			DataTypeImpl const& elementType;

			int selfCompare(DataTypeImpl const& rhs) const;
		};

	}

	////////////////////////////////////////////////////////////////
	// Script Types

	// Basically an enum.
	class ScriptType
	{
	public:
		static ScriptType const& getGlobal();
		static ScriptType const& getFfc();
		static ScriptType const& getItem();

		ScriptType();

		bool operator==(ScriptType const& other) const {
			return id == other.id;}
		std::string const& getName() const {return name;}
		optional<DataType> getThisType(TypeStore&) const;
		bool isNull() const {return id == Id_Null;}

	private:
		enum Id {Id_Null, Id_Global, Id_Ffc, Id_Item};

		Id id;
		std::string name;

		ScriptType(Id id, std::string const& name);
	};
}

#endif

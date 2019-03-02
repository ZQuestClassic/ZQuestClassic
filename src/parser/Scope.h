#ifndef ZSCRIPT_SCOPE_H
#define ZSCRIPT_SCOPE_H

#include <map>
#include <string>
#include <vector>
#include "CompilerUtils.h"
#include "CompileOption.h"

namespace ZScript
{
	////////////////////////////////////////////////////////////////
	// Forward Declarations
	
	// AST.h
	class AST;

	// CompileError.h
	class CompileErrorHandler;
	
	// Types.h
	class DataType;
	class TypeStore;
	class ScriptType;

	// ZScript.h
	class Script;
	class Datum;
	class Function;
	class FunctionSignature;

	// Local forward declarations
	class ZClass;
	class RootScope;
	class FileScope;
	class ScriptScope;
	class FunctionScope;

	////////////////////////////////////////////////////////////////
	
	class Scope : private NoCopy
	{
		// So Datum classes can only be generated in tandem with a scope.
		friend class Datum;
		
	public:
		Scope(TypeStore&);
		Scope(TypeStore&, std::string const& name);

		// Scope type.
		virtual bool isGlobal() const {return false;}
		virtual bool isScript() const {return false;}
		
		// Accessors
		TypeStore const& getTypeStore() const {return typeStore_;}
		TypeStore& getTypeStore() {return typeStore_;}
		optional<std::string> const& getName() const {return name_;}
		optional<std::string>& getName() {return name_;}

		// Inheritance
		virtual Scope* getParent() const = 0;
		virtual Scope* getChild(std::string const& name) const = 0;
		virtual std::vector<Scope*> getChildren() const = 0;
	
		// Lookup Local
		virtual DataType const* getLocalDataType(std::string const& name)
			const = 0;
		virtual optional<ScriptType> getLocalScriptType(
			std::string const& name) const = 0;
		virtual ZClass* getLocalClass(std::string const& name) const = 0;
		virtual Datum* getLocalDatum(std::string const& name) const = 0;
		virtual Function* getLocalGetter(std::string const& name) const = 0;
		virtual Function* getLocalSetter(std::string const& name) const = 0;
		virtual Function* getLocalFunction(
				FunctionSignature const& signature) const = 0;
		virtual std::vector<Function*> getLocalFunctions(
				std::string const& name) const = 0;
		virtual CompileOptionSetting getLocalOption(CompileOption option)
				const = 0;
	
		// Get All Local.
		virtual std::vector<Datum*> getLocalData() const = 0;
		virtual std::vector<Function*> getLocalFunctions() const = 0;
		virtual std::vector<Function*> getLocalGetters() const = 0;
		virtual std::vector<Function*> getLocalSetters() const = 0;
		virtual std::map<CompileOption, CompileOptionSetting>
				getLocalOptions() const = 0;

		// Add
		virtual Scope* makeChild() = 0;
		virtual Scope* makeChild(std::string const& name) = 0;
		virtual FileScope* makeFileChild(std::string const& filename) = 0;
		virtual ScriptScope* makeScriptChild(Script& script) = 0;
		virtual FunctionScope* makeFunctionChild(Function& function) = 0;
		virtual DataType const* addDataType(
				std::string const& name, DataType const* type, AST* node)
		= 0;
		virtual bool addScriptType(
			std::string const& name, ScriptType type, AST* node) = 0;
		//virtual ZClass* addClass(string const& name, AST* node) = 0;
		virtual Function* addGetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL)
		= 0;
		virtual Function* addSetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL)
		= 0;
		virtual Function* addFunction(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL)
		= 0;
		virtual void setDefaultOption(CompileOptionSetting value) = 0;
		virtual void setOption(
				CompileOption option, CompileOptionSetting value) = 0;

		////////////////
		// Stack

		// If this scope starts a new stack frame, return its total stack
		// size.
		virtual optional<int> getRootStackSize() const {return nullopt;}

		// Let this scope know that it needs to recalculate the stack size.
		virtual void invalidateStackSize();
		
		// Get the depth of the stack for this scope, not considering its
		// children.
		virtual int getLocalStackDepth() const {return 0;}

		// Get the stack offset for this local datum.
		virtual optional<int> getLocalStackOffset(Datum const&) const {
			return nullopt;}
		
	protected:
		TypeStore& typeStore_;
		optional<std::string> name_;

	private:
		// Add the datum to this scope, returning if successful. Called by
		// the Datum classes' ::create functions.
		virtual bool add(ZScript::Datum&, CompileErrorHandler*) = 0;
	};

	////////////////
	// Inheritance

	// Repeatedly get a child namespace with the names in order. Fail if any
	// name does not resolve.
	Scope* getDescendant(
			Scope const&, std::vector<std::string> const& names);

	// Find a scope with the given name in this scope.
	Scope* lookupScope(Scope const&, std::string const& name);

	// Find first scope with the given ancestry in this scope.
	Scope* lookupScope(Scope const&, std::vector<std::string> const& names);

	// Find all scopes with the given ancestry in this scope. Note than an
	// empty name list will the current scope and its ancestry.
	std::vector<Scope*> lookupScopes(
			Scope const&, std::vector<std::string> const& names);

	// Get the most distant parent.
	RootScope* getRoot(Scope const&);
	
	////////////////
	// Lookup

	// Attempt to resolve name to a type id under scope.
	DataType const* lookupDataType(Scope const&, std::string const& name);
	
	// Attempt to resolve name to a script type id under scope.
	ScriptType lookupScriptType(Scope const&, std::string const& name);
	
	// Attempt to resolve name to a class id under scope.
	ZClass* lookupClass(Scope const&, std::string const& name);

	// Attempt to resolve name to a variable under scope.
	Datum* lookupDatum(Scope const&, std::string const& name);
	Datum* lookupDatum(Scope const&, std::vector<std::string> const& name);
	
	// Attempt to resolve name to a getter under scope.
	Function* lookupGetter(Scope const&, std::string const& name);

	// Attempt to resolve name to a setter under scope.
	Function* lookupSetter(Scope const&, std::string const& name);

	// Attempt to resolve signature to a function under scope.
	Function* lookupFunction(Scope const&, FunctionSignature const&);
	
	// Attempt to resolve name to possible functions under scope.
	std::vector<Function*> lookupFunctions(
			Scope const&, std::string const& name);
	std::vector<Function*> lookupFunctions(
			Scope const&, std::vector<std::string> const& name);

	// Resolve an option value under the scope. Will only return empty if
	// the provided option is invalid. If the option is valid but not set,
	// returns the default value for it.
	optional<long> lookupOption(Scope const&, CompileOption);

	////////////////
	// Stack

	// Does this scope start a new stack frame?
	bool isStackRoot(Scope const&);

	// Get the stack offset for a datum, checking parents until we hit a
	// root.
	optional<int> lookupStackOffset(Scope const&, Datum const&);

	// Find the total size of the stack scope is in.
	optional<int> lookupStackSize(Scope const&);
	
	// Lookup the stack offset and then subtract it from the root stack
	// size.
	optional<int> lookupStackPosition(Scope const&, Datum const&);
	
	////////////////
	// Get all in branch

	// Recursively get all of something for a scope and its children.
	// usage: getInBranch<Function*>(scope, &Scope::getLocalFunctions)
	template <typename Element>
	std::vector<Element> getInBranch(
			Scope const& scope,
			std::vector<Element> (Scope::* call)() const)
	{
		std::vector<Element> results = (scope.*call)();
		std::vector<Scope*> children = scope.getChildren();
		for (std::vector<Scope*>::const_iterator it = children.begin();
		     it != children.end(); ++it)
		{
			std::vector<Element> subResults = getInBranch(**it, call);
			results.insert(results.end(),
			               subResults.begin(), subResults.end());
		}
		return results;
	}

	std::vector<Function*> getFunctionsInBranch(Scope const& scope);
	
	////////////////////////////////////////////////////////////////
	// BasicScope - Primary Scope implementation.
	
	class FunctionScope;
	class BasicScope : public Scope
	{
	public:
		BasicScope(Scope* parent);
		BasicScope(Scope* parent, std::string const& name);
		virtual ~BasicScope();

		// Inheritance
		virtual Scope* getParent() const {return parent_;}
		virtual Scope* getChild(std::string const& name) const;
		virtual std::vector<Scope*> getChildren() const;
	
		// Lookup Local
		DataType const* getLocalDataType(std::string const& name)
			const /*override*/;
		optional<ScriptType> getLocalScriptType(std::string const& name)
			const /*override*/;
		virtual ZClass* getLocalClass(std::string const& name) const;
		virtual Datum* getLocalDatum(std::string const& name) const;
		virtual Function* getLocalGetter(std::string const& name) const;
		virtual Function* getLocalSetter(std::string const& name) const;
		virtual Function* getLocalFunction(
				FunctionSignature const& signature) const;
		virtual std::vector<Function*> getLocalFunctions(
				std::string const& name) const;
		virtual CompileOptionSetting getLocalOption(CompileOption option) const;
		
		// Get All Local
		virtual std::vector<ZScript::Datum*> getLocalData() const;
		virtual std::vector<ZScript::Function*> getLocalFunctions() const;
		virtual std::vector<ZScript::Function*> getLocalGetters() const;
		virtual std::vector<ZScript::Function*> getLocalSetters() const;
		virtual std::map<CompileOption, CompileOptionSetting>
				getLocalOptions() const;

		// Add
		virtual Scope* makeChild();
		virtual Scope* makeChild(std::string const& name);
		virtual FileScope* makeFileChild(std::string const& filename);
		virtual ScriptScope* makeScriptChild(Script& script);
		virtual FunctionScope* makeFunctionChild(Function& function);
		virtual DataType const* addDataType(
				std::string const& name, DataType const* type,
				AST* node = NULL);
		bool addScriptType(
			std::string const& name, ScriptType type, AST* node)
			/*override*/;
		virtual Function* addGetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL);
		virtual Function* addSetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL);
		virtual Function* addFunction(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL);
		virtual void setDefaultOption(CompileOptionSetting value);
		virtual void setOption(
				CompileOption option, CompileOptionSetting value);
		
		// Stack
		virtual int getLocalStackDepth() const {return stackDepth_;}
		virtual optional<int> getLocalStackOffset(Datum const& datum) const;
		
	protected:
		Scope* parent_;
		std::map<std::string, Scope*> children_;
		std::vector<Scope*> anonymousChildren_;
		std::map<std::string, DataType const*> dataTypes_;
		std::map<std::string, ScriptType> scriptTypes_;
		std::map<std::string, ZClass*> classes_;
		std::vector<Datum*> anonymousData_;
		std::map<std::string, Datum*> namedData_;
		std::map<Datum*, int> stackOffsets_;
		int stackDepth_;
		std::map<std::string, Function*> getters_;
		std::map<std::string, Function*> setters_;
		std::map<std::string, std::vector<Function*> > functionsByName_;
		std::map<FunctionSignature, Function*> functionsBySignature_;
		std::map<CompileOption, CompileOptionSetting> options_;
		CompileOptionSetting defaultOption_;

		BasicScope(TypeStore&);
		BasicScope(TypeStore&, std::string const& name);

		virtual bool add(Datum&, CompileErrorHandler*);
		
	private:
		// Disabled since it's easy to call by accident instead of the Scope*
		// constructor.
		BasicScope(BasicScope const& base);
	};

	////////////////////////////////////////////////////////////////
	// FileScope

	class FileScope : public BasicScope
	{
	public:
		FileScope(Scope* parent, std::string const& filename);

		virtual bool isGlobal() const {return true;}
		
		// Override to also register in the root scope, and fail if already
		// present there as well.
		virtual Scope* makeChild(std::string const& name);
		virtual ScriptScope* makeScriptChild(Script& script);
		virtual DataType const* addDataType(
				std::string const& name, DataType const* type,
				AST* node = NULL);
		bool addScriptType(
			std::string const& name, ScriptType type, AST* node)
			/*override*/;
		virtual Function* addGetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL);
		virtual Function* addSetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL);
		virtual Function* addFunction(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes,
				AST* node = NULL);

	protected:
		virtual bool add(Datum&, CompileErrorHandler*);
		
	private:
		std::string filename_;

	};

	////////////////////////////////////////////////////////////////
	// RootScope - The highest level scope.

	// For the purpose of resolving data, functions, etc. the root scope has
	// counts as locally having the objects of all its FileScope
	// descendants. These lists are maintained locally in the desc*
	// variables.

	class RootScope : public BasicScope
	{
	public:
		RootScope(TypeStore&);
		
		virtual bool isGlobal() const {return true;}
		virtual optional<int> getRootStackSize() const;

		// Also check the descendant listings.
		// Single
		virtual Scope* getChild(std::string const& name) const;
		virtual DataType const* getLocalDataType(
				std::string const& name) const;
		optional<ScriptType> getLocalScriptType(std::string const& name)
			const /*override*/;
		virtual ZClass* getLocalClass(std::string const& name) const;
		virtual Datum* getLocalDatum(std::string const& name) const;
		virtual Function* getLocalGetter(std::string const& name) const;
		virtual Function* getLocalSetter(std::string const& name) const;
		virtual Function* getLocalFunction(
				FunctionSignature const& signature) const;
		virtual std::vector<Function*> getLocalFunctions(
				std::string const& name) const;
		// All
		virtual std::vector<Datum*> getLocalData() const;
		virtual std::vector<Function*> getLocalFunctions() const;
		virtual std::vector<Function*> getLocalGetters() const;
		virtual std::vector<Function*> getLocalSetters() const;

		// Register a descendant's thing.
		bool registerChild(std::string const& name, Scope* child);
		bool registerDataType(std::string const& name, DataType const* type);
		bool registerScriptType(std::string const& name, ScriptType type);
		bool registerClass(std::string const& name, ZClass* klass);
		bool registerDatum(std::string const& name, Datum* datum);
		bool registerGetter(std::string const& name, Function* getter);
		bool registerSetter(std::string const& name, Function* setter);
		bool registerFunction(Function* function);
		
	private:
		mutable optional<int> stackSize_;

		// Unowned pointers to descendant's stuff.
		std::map<std::string, Scope*> descChildren_;
		std::map<std::string, DataType const*> descDataTypes_;
		std::map<std::string, ScriptType> descScriptTypes_;
		std::map<std::string, ZClass*> descClasses_;
		std::map<std::string, Datum*> descData_;
		std::map<std::string, Function*> descGetters_;
		std::map<std::string, Function*> descSetters_;
		std::map<std::string, std::vector<Function*> > descFunctionsByName_;
		std::map<FunctionSignature, Function*> descFunctionsBySignature_;
	};
	
	////////////////////////////////////////////////////////////////
	
	class ScriptScope : public BasicScope
	{
	public:
		ScriptScope(Scope* parent, Script& script);
		virtual bool isScript() const {return true;}
		Script& script;
	};

	class FunctionScope : public BasicScope
	{
	public:
		FunctionScope(Scope* parent, Function& function);
		bool isFunction() const {return true;}
		Function& function;
		optional<int> getRootStackSize() const;
	private:
		mutable optional<int> stackSize;
	};

	enum ZClassIdBuiltin
	{
		ZCLASSID_START = 0,
		ZCLASSID_GAME = 0,
		ZCLASSID_LINK,
		ZCLASSID_SCREEN,
		ZCLASSID_FFC,
		ZCLASSID_ITEM,
		ZCLASSID_ITEMCLASS,
		ZCLASSID_NPC,
		ZCLASSID_LWPN,
		ZCLASSID_EWPN,
		ZCLASSID_NPCDATA,
		ZCLASSID_DEBUG,
		ZCLASSID_AUDIO,
		ZCLASSID_COMBOS,
		ZCLASSID_SPRITEDATA,
		ZCLASSID_GRAPHICS,
		ZCLASSID_BITMAP,
		ZCLASSID_TEXT,
		ZCLASSID_INPUT,
		ZCLASSID_MAPDATA,
		ZCLASSID_DMAPDATA,
		ZCLASSID_ZMESSAGE,
		ZCLASSID_SHOPDATA,
		ZCLASSID_DROPSET,
		ZCLASSID_PONDS,
		ZCLASSID_WARPRING,
		ZCLASSID_DOORSET,
		ZCLASSID_ZUICOLOURS,
		ZCLASSID_RGBDATA,
		ZCLASSID_PALETTE,
		ZCLASSID_TUNES,
		ZCLASSID_PALCYCLE,
		ZCLASSID_GAMEDATA,
		ZCLASSID_CHEATS,
		ZCLASSID_END
	};

	class ZClass : public BasicScope
	{
	public:
		ZClass(TypeStore&, std::string const& name, int id);
		std::string const name;
		int const id;
	};

};
#endif

#ifndef ZPARSER_SCOPE_H
#define ZPARSER_SCOPE_H

#include <assert.h>
#include "DataStructs.h"

class CompileErrorHandler;

namespace ZScript
{
	class Script;
	class Datum;
	class Function;

	class Scope
	{
		// So Datum classes can only be generated in tandem with a scope.
		friend class Datum;
		
	public:
		static Scope* makeGlobalScope(SymbolTable& table);

		Scope(SymbolTable& table);
		Scope(SymbolTable& table, string const& name);

		// Accessors
		SymbolTable const& getTable() const {return table;}
		SymbolTable& getTable() {return table;}
		optional<string> const& getName() const {return name;}
		optional<string>& getName() {return name;}
		
		// Scope Type
		virtual bool isGlobal() const {return false;}
		virtual bool isScript() const {return false;}
		virtual bool isFunction() const {return false;}

		// Inheritance
		virtual Scope* getParent() const = 0;
		virtual Scope* getChild(string const& name) const = 0;
		virtual vector<Scope*> getChildren() const = 0;
	
		// Lookup Local
		virtual ZVarType const* getLocalType(string const& name) const = 0;
		virtual ZClass* getLocalClass(string const& name) const = 0;
		virtual Datum* getLocalDatum(string const& name) const = 0;
		virtual Function* getLocalGetter(string const& name) const = 0;
		virtual Function* getLocalSetter(string const& name) const = 0;
		virtual Function* getLocalFunction(
				Function::Signature const& signature) const = 0;
		virtual vector<Function*> getLocalFunctions(
				string const& name) const = 0;
	
		// Get All Local.
		virtual vector<Datum*> getLocalData() const = 0;
		virtual vector<Function*> getLocalFunctions() const = 0;

		// Add
		virtual Scope* makeChild() = 0;
		virtual Scope* makeChild(string const& name) = 0;
		virtual FunctionScope* makeFunctionChild(Function& function) = 0;
		virtual ZVarType const* addType(
				string const& name, ZVarType const* type, AST* node) = 0;
		//virtual ZClass* addClass(string const& name, AST* node) = 0;
		virtual Function* addGetter(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL)
				= 0;
		virtual Function* addSetter(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL)
				= 0;
		virtual Function* addFunction(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL)
				= 0;

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
		
		bool varDeclsDeprecated;

	protected:
		SymbolTable& table;
		optional<string> name;

	private:
		// Add the datum to this scope, returning if successful. Called by
		// the Datum classes ::create functions.
		virtual bool add(ZScript::Datum&, CompileErrorHandler&) = 0;
	};

	////////////////
	// Inheritance
	
	// Repeatedly get a child namespace with the names in order. Fail if any
	// name does not resolve.
	Scope* getDescendant(Scope const&, vector<string> const& names);
	
	// Find a scope with the given name in this scope.
	Scope* lookupScope(Scope const&, string const& name);

	// Find first scope with the given ancestry in this scope.
	Scope* lookupScope(Scope const&, vector<string> const& names);

	// Find all scopes with the given ancestry in this scope. Note than an
	// empty name list will the current scope and its ancestry.
	vector<Scope*> lookupScopes(Scope const&, vector<string> const& names);
	
	////////////////
	// Lookup

	// Attempt to resolve name to a type id under scope.
	ZVarType const* lookupType(Scope const&, string const& name);
	
	// Attempt to resolve name to a class id under scope.
	ZClass* lookupClass(Scope const&, string const& name);

	// Attempt to resolve name to a variable under scope.
	Datum* lookupDatum(Scope const&, string const& name);
	Datum* lookupDatum(Scope const&, vector<string> const& name);
	
	// Attempt to resolve name to a getter under scope.
	Function* lookupGetter(Scope const&, string const& name);

	// Attempt to resolve name to a setter under scope.
	Function* lookupSetter(Scope const&, string const& name);

	// Attempt to resolve signature to a function under scope.
	Function* lookupFunction(Scope const&, Function::Signature const&);
	
	// Attempt to resolve name to possible functions under scope.
	vector<Function*> lookupFunctions(Scope const&, string const& name);
	vector<Function*> lookupFunctions(
			Scope const&, vector<string> const& name);

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
	vector<Element> getInBranch(
			Scope const& scope,
			vector<Element> (Scope::* call)() const)
	{
		vector<Element> results = (scope.*call)();
		vector<Scope*> children = scope.getChildren();
		for (vector<Scope*>::const_iterator it = children.begin();
		     it != children.end(); ++it)
		{
			vector<Element> subResults = getInBranch(**it, call);
			results.insert(results.end(),
			               subResults.begin(), subResults.end());
		}
		return results;
	}

	vector<Function*> getFunctionsInBranch(Scope const& scope);

	////////////////////////////////////////////////////////////////
	// BasicScope - Primary Scope implementation.

	class FunctionScope;
	class BasicScope : public Scope
	{
	public:
		BasicScope(Scope* parent);
		BasicScope(Scope* parent, string const& name);
		virtual ~BasicScope();

		// Inheritance
		Scope* getParent() const {return parent;}
		Scope* getChild(string const& name) const;
		vector<Scope*> getChildren() const;
	
		// Lookup Local
		ZVarType const* getLocalType(string const& name) const;
		ZClass* getLocalClass(string const& name) const;
		Datum* getLocalDatum(string const& name) const;
		Function* getLocalGetter(string const& name) const;
		Function* getLocalSetter(string const& name) const;
		Function* getLocalFunction(Function::Signature const& signature)
				const;
		vector<Function*> getLocalFunctions(string const& name) const;
	
		// Get All Local
		vector<ZScript::Datum*> getLocalData() const;
		vector<ZScript::Function*> getLocalFunctions() const;

		// Add
		Scope* makeChild();
		Scope* makeChild(string const& name);
		FunctionScope* makeFunctionChild(Function& function);
		ZVarType const* addType(
				string const& name, ZVarType const* type, AST* node = NULL);
		Function* addGetter(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL);
		Function* addSetter(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL);
		Function* addFunction(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL);

		// Stack
		int getLocalStackDepth() const {return stackDepth;}
		optional<int> getLocalStackOffset(Datum const& datum) const;
		
	protected:
		Scope* parent;
		map<string, Scope*> children;
		vector<Scope*> anonymousChildren;
		map<string, ZVarType const*> types;
		map<string, ZClass*> classes;
		vector<Datum*> anonymousData;
		map<string, Datum*> namedData;
		map<Datum*, int> stackOffsets;
		int stackDepth;
		map<string, Function*> getters;
		map<string, Function*> setters;
		map<string, vector<Function*> > functionsByName;
		map<Function::Signature, Function*> functionsBySignature;

		BasicScope(SymbolTable&);
		BasicScope(SymbolTable&, string const& name);
		
	private:
		// Disabled since it's easy to call by accident instead of the Scope*
		// constructor.
		BasicScope(BasicScope const& base);

		bool add(Datum&, CompileErrorHandler&);
	};

	class ScriptScope;
	class GlobalScope : public BasicScope
	{
	public:
		GlobalScope(SymbolTable& table);
		bool isGlobal() const {return true;}
		ScriptScope* makeScriptChild(Script& script);
		optional<int> getRootStackSize() const;
	private:
		mutable optional<int> stackSize;
	};

	class ScriptScope : public BasicScope
	{
	public:
		ScriptScope(GlobalScope* parent, Script& script);
		bool isScript() const {return true;}
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
		ZCLASSID_GAME = 0, ZCLASSID_LINK, ZCLASSID_SCREEN,
		ZCLASSID_FFC, ZCLASSID_ITEM, ZCLASSID_ITEMCLASS, ZCLASSID_NPC, ZCLASSID_LWPN, ZCLASSID_EWPN,
		ZCLASSID_AUDIO, ZCLASSID_DEBUG, ZCLASSID_NPCDATA,
		ZCLASSID_END
	};

	class ZClass : public BasicScope
	{
	public:
		ZClass(SymbolTable& table, string const& name, int id);
		string const name;
		int const id;
	};

};
#endif

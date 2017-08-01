#ifndef ZPARSER_SCOPE_H
#define ZPARSER_SCOPE_H

#include <assert.h>
#include "DataStructs.h"

namespace ZScript
{
	class Literal;
	class Variable;
	class Function;

	class Scope
	{
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

		// Inheritance
		virtual Scope* getParent() const = 0;
		virtual Scope* getChild(string const& name) const = 0;
		virtual vector<Scope*> getChildren() const = 0;
	
		// Lookup Local
		virtual ZVarType const* getLocalType(string const& name) const = 0;
		virtual ZClass* getLocalClass(string const& name) const = 0;
		virtual Variable* getLocalVariable(string const& name) const = 0;
		virtual Function* getLocalGetter(string const& name) const = 0;
		virtual Function* getLocalSetter(string const& name) const = 0;
		virtual Function* getLocalFunction(
				Function::Signature const& signature) const = 0;
		virtual vector<Function*> getLocalFunctions(
				string const& name) const = 0;
	
		// Get All Local.
		virtual vector<Literal*> getLocalLiterals() const = 0;
		virtual vector<Variable*> getLocalVariables() const = 0;
		virtual vector<Function*> getLocalFunctions() const = 0;

		// Add
		virtual Scope* makeChild() = 0;
		virtual Scope* makeChild(string const& name) = 0;
		virtual ZVarType const* addType(
				string const& name, ZVarType const* type, AST* node) = 0;
		//virtual ZClass* addClass(string const& name, AST* node) = 0;
		virtual ZScript::Literal* addLiteral(
				ASTLiteral& literal, ZVarType const* type = NULL) = 0;
		virtual ZScript::Variable* addVariable(
				ZVarType const& type, string const& name, AST* node = NULL)
				= 0;
		virtual ZScript::Function* addGetter(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL)
				= 0;
		virtual ZScript::Function* addSetter(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL)
				= 0;
		virtual ZScript::Function* addFunction(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL)
				= 0;

		bool varDeclsDeprecated;

	protected:
		SymbolTable& table;
		optional<string> name;
	
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
	Variable* lookupVariable(Scope const&, string const& name);
	Variable* lookupVariable(Scope const&, vector<string> const& name);
	
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
	
	class BasicScope : public Scope
	{
	public:
		BasicScope(Scope* parent);
		BasicScope(Scope* parent, string const& name);
		~BasicScope();

		// Inheritance
		Scope* getParent() const {return parent;}
		Scope* getChild(string const& name) const;
		vector<Scope*> getChildren() const;
	
		// Lookup Local
		ZVarType const* getLocalType(string const& name) const;
		ZClass* getLocalClass(string const& name) const;
		Variable* getLocalVariable(string const& name) const;
		Function* getLocalGetter(string const& name) const;
		Function* getLocalSetter(string const& name) const;
		Function* getLocalFunction(Function::Signature const& signature)
				const;
		vector<Function*> getLocalFunctions(string const& name) const;
	
		// Get All Local
		vector<ZScript::Literal*> getLocalLiterals() const;
		vector<ZScript::Variable*> getLocalVariables() const;
		vector<ZScript::Function*> getLocalFunctions() const;

		// Add
		Scope* makeChild();
		Scope* makeChild(string const& name);
		ZVarType const* addType(
				string const& name, ZVarType const* type, AST* node = NULL);
		Literal* addLiteral(ASTLiteral& literal, ZVarType const* type = NULL);
		Variable* addVariable(
				ZVarType const& type, string const& name, AST* node = NULL);
		Function* addGetter(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL);
		Function* addSetter(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL);
		Function* addFunction(
				ZVarType const* returnType, string const& name,
				vector<ZVarType const*> const& paramTypes, AST* node = NULL);
		
	protected:
		Scope* parent;
		map<string, Scope*> children;
		vector<Scope*> anonymousChildren;
		map<string, ZVarType const*> types;
		map<string, ZClass*> classes;
		vector<Literal*> literals;
		map<string, Variable*> variables;
		map<string, Function*> getters;
		map<string, Function*> setters;
		map<string, vector<Function*> > functionsByName;
		map<Function::Signature, Function*> functionsBySignature;

		BasicScope(SymbolTable& table);
		BasicScope(SymbolTable& table, string const& name);
		
		// Disabled since it's easy to call by accident instead of the Scope*
		// constructor.
		BasicScope(BasicScope const& base) : Scope(base.table) {assert(false);}
	};

	class ScriptScope;
	class GlobalScope : public BasicScope
	{
	public:
		GlobalScope(SymbolTable& table);
		bool isGlobal() const {return true;}
		ScriptScope* makeScriptChild(ZScript::Script& script);
	};

	class ScriptScope : public BasicScope
	{
	public:
		ScriptScope(GlobalScope* scope, ZScript::Script& script)
			: BasicScope(scope, script.getName()), script(script) {}
		bool isScript() const {return true;}
		ZScript::Script& script;
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

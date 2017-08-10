#ifndef ZSCRIPT_H
#define ZSCRIPT_H

#include <vector>
#include <map>
#include "AST.h"
#include "CompileError.h"
#include "CompilerUtils.h"
#include "Types.h"

using std::vector;
using std::map;

class SymbolTable;

namespace ZScript
{
	class Program;
	class Script;
	class Variable;
	class BuiltinVariable;
	class Function;
	class Scope;
	class GlobalScope;
	class ScriptScope;
	
	class Program
	{
	public:
		Program(ASTProgram* ast);
		~Program();
		ASTProgram* node;
		SymbolTable& table;
		GlobalScope& globalScope;

		vector<Script*> scripts;
		Script* getScript(string const& name) const;
		Script* getScript(ASTScript* node) const;

		// Gets the non-internal (user-defined) global scope functions.
		vector<Function*> getUserGlobalFunctions() const;

		// Gets all user-defined functions.
		vector<Function*> getUserFunctions() const;

		// Return a list of all errors in the script declaration.
		vector<CompileError const*> getErrors() const;
		// Does this script have a declaration error?
		bool hasError() const {return getErrors().size() > 0;}

	private:
		map<string, Script*> scriptsByName;
		map<ASTScript*, Script*> scriptsByNode;

		// Disabled.
		Program(Program const&);
		Program& operator=(Program const&);
	};

	class Script
	{
	public:
		Script(Program& program, ASTScript* script);

		ASTScript* node;
		ScriptScope* scope;

		string getName() const;
		ScriptType getType() const;
		Function* getRun() const;

		// Return a list of all errors in the script declaration.
		vector<CompileError const*> getErrors() const;
		// Does this script have a declaration error?
		bool hasError() const {return getErrors().size() > 0;}
	};

	// Something that can be resolved to a data value.
	class Datum
	{
	public:
		// The containing scope.
		Scope& scope;

		// The type of this data.
		ZVarType const& type;

		// Id for lookup tables.
		int const id;

		// Get the data's name.
		virtual optional<string> getName() const {return nullopt;}
		
		// Get the value at compile time.
		virtual optional<long> getCompileTimeValue() const {return nullopt;}

		// Get the declaring node.
		virtual AST* getNode() const {return NULL;}

		// Get the global register this uses.
		virtual optional<int> getGlobalId() const {return nullopt;}
		
	protected:
		Datum(Scope& scope, ZVarType const& type);

		// Call in static creation function to register with scope.
		bool tryAddToScope(CompileErrorHandler&);
	};

	bool isGlobal(Datum const& data);

	// A literal value that requires memory management.
	class Literal : public Datum
	{
	public:
		static Literal* create(
				Scope&, ASTLiteral&, ZVarType const&,
				CompileErrorHandler& = CompileErrorHandler::NONE);
		
		ASTLiteral* getNode() const {return &node;}

	private:
		Literal(Scope& scope, ASTLiteral& node, ZVarType const& type);

		ASTLiteral& node;
	};

	// A variable.
	class Variable : public Datum
	{
	public:
		static Variable* create(
				Scope&, ASTDataDecl&, ZVarType const&,
				CompileErrorHandler& = CompileErrorHandler::NONE);
		
		optional<string> getName() const {return node.name;}
		ASTDataDecl* getNode() const {return &node;}
		optional<int> getGlobalId() const {return globalId;}

	private:
		Variable(Scope& scope, ASTDataDecl& node, ZVarType const& type);

		ASTDataDecl& node;
		optional<int> globalId;
	};

	// A compiler generated variable.
	class BuiltinVariable : public Datum
	{
	public:
		static BuiltinVariable* create(
				Scope&, ZVarType const&, string const& name,
				CompileErrorHandler& = CompileErrorHandler::NONE);
		
		optional<string> getName() const {return name;}
		optional<int> getGlobalId() const {return globalId;}

	private:
		BuiltinVariable(Scope&, ZVarType const&, string const& name);

		string const name;
		optional<int> globalId;
	};

	// An inlined constant.
	class Constant : public Datum
	{
	public:
		static Constant* create(
				Scope&, ASTDataDecl&, ZVarType const&, long value,
				CompileErrorHandler& = CompileErrorHandler::NONE);
		
		optional<string> getName() const;

		optional<long> getCompileTimeValue() const {return value;}

		ASTDataDecl* getNode() const {return &node;}

	private:
		Constant(Scope&, ASTDataDecl&, ZVarType const&, long value);

		ASTDataDecl& node;
		long value;
	};
	
	// A builtin data value.
	class BuiltinConstant : public Datum
	{
	public:
		static BuiltinConstant* create(
				Scope&, ZVarType const&, string const& name, long value,
				CompileErrorHandler& = CompileErrorHandler::NONE);
		
		optional<string> getName() const {return name;}
		optional<long> getCompileTimeValue() const {return value;}

	private:
		BuiltinConstant(Scope&, ZVarType const&,
		                string const& name, long value);

		string name;
		long value;
	};
	
	class Function
	{
	public:
		// Comparable signature structure.
		class Signature
		{
		public:
			Signature(string const& name,
			          vector<ZVarType const*> const& parameterTypes);
			Signature(Function const& function);

			int compare(Signature const& other) const;
			bool operator==(Signature const& other) const;
			bool operator<(Signature const& other) const;
			string asString() const;
			operator string() const {return asString();}

			string name;
			vector<ZVarType const*> parameterTypes;
		};
		
		Function(ZVarType const* returnType, string const& name,
				 vector<ZVarType const*> paramTypes, int id);
		ZVarType const* returnType;
		string name;
		vector<ZVarType const*> paramTypes;
		int id;

		ASTFuncDecl* node;
		Scope* internalScope;
		BuiltinVariable* thisVar;
		
		Signature getSignature() const {return Signature(*this);}
		
		// If this is a script level function, return that script.
		Script* getScript() const;

		int getLabel() const;
		
	private:
		mutable optional<int> label;
	};
}

#endif

#ifndef ZSCRIPT_H
#define ZSCRIPT_H

#include <vector>
#include <map>
#include "AST.h"
#include "CompilerUtils.h"
#include "Types.h"

using std::vector;
using std::map;

class SymbolTable;
class GlobalScope;
class ScriptScope;

namespace ZScript
{
	class Program;
	class Script;
	class Variable;
	class Function;
	
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

		// Gets the global user-defined and (deprecated) script level variables.
		vector<Variable*> getUserGlobalVariables() const;

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

	class Literal
	{
	public:
		Literal(ASTLiteral* node, ZVarType const* type, int id);
		ASTLiteral* node;
		ZVarType const* type;
		int id;
	};

	class Variable
	{
	public:
		Variable(ASTDataDecl* node, ZVarType const* type, string const& name, int id);
		ASTDataDecl* node;
		ZVarType const* type;
		string name;
		int id;

		// Is this a global variable?
		bool global;

		// If this is a compile time constant, and its value.
		optional<long> compileTimeValue;
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

			int compare(Signature const& other) const;
			bool operator==(Signature const& other) const;
			bool operator<(Signature const& other) const;

			string name;
			vector<ZVarType const*> parameterTypes;
		};
		
		Function(ZVarType const* returnType, string const& name, vector<ZVarType const*> paramTypes, int id)
				: node(NULL), internalScope(NULL), thisVar(NULL),
				  returnType(returnType), name(name), paramTypes(paramTypes), id(id)
			{}
		ZVarType const* returnType;
		string name;
		vector<ZVarType const*> paramTypes;
		int id;

		ASTFuncDecl* node;
		Scope* internalScope;
		Variable* thisVar;

		// If this is a script level function, return that script.
		Script* getScript() const;
	};
}

#endif

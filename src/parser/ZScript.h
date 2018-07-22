#ifndef ZSCRIPT_H
#define ZSCRIPT_H

#include "DataStructs.h"
#include "Scope.h"

namespace ZScript
{
	class Program
	{
	public:
		Program(ASTProgram* ast);
		~Program();
		ASTProgram* node;
		SymbolTable table;
		GlobalScope globalScope;

		vector<Script*> scripts;
		Script* getScript(string const& name) const;
		Script* getScript(ASTScript* node) const;

		bool hasError() const;
		void printErrors() const;

	private:
		map<string, Script*> scriptsByName;
		map<ASTScript*, Script*> scriptsByNode;
	};

	struct Script
	{
		Script(Program& program, ASTScript* script);

		ASTScript* node;
		ScriptScope* scope;

		string getName() const;
		ScriptType getType() const;
		Function* getRun() const;

		bool hasError() const;
		void printErrors() const;
	};

	struct Variable
	{
		Variable(ZVarType const* type, string const& name, int id) : type(type), name(name), id(id) {}
		ZVarType const* type;
		string name;
		int id;
	};

	struct Function
	{
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
	};
}

#endif

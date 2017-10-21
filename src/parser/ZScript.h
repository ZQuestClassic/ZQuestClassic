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

namespace ZScript
{
	class TypeStore;
	class Program;
	class Script;
	class Variable;
	class BuiltinVariable;
	class Function;
	class Scope;
	class GlobalScope;
	class ScriptScope;
	class FunctionScope;

	////////////////////////////////////////////////////////////////
	// Program
	
	class Program
	{
	public:
		Program(ASTProgram&, TypeStore&, CompileErrorHandler&);
		~Program();

		ASTProgram& getNode() {return node;}
		TypeStore const& getTypeStore() const {return typeStore;}
		TypeStore& getTypeStore() {return typeStore;}
		GlobalScope& getScope() const {return *globalScope;}
		
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

		ASTProgram& node;
		TypeStore& typeStore;
		GlobalScope* globalScope;
		
		// Disabled.
		Program(Program const&);
		Program& operator=(Program const&);
	};

	// Gets all defined functions.
	vector<Function*> getFunctions(Program const&);

	////////////////////////////////////////////////////////////////
	// Script

	class UserScript;
	class BuiltinScript;
	
	class Script
	{
	public:
		virtual ~Script() {}
				
		virtual ScriptType getType() const = 0;
		virtual string const& getName() const = 0;
		virtual ASTScript* getNode() const = 0;
		virtual ScriptScope& getScope() = 0;
		virtual ScriptScope const& getScope() const = 0;

		vector<Opcode> code;
		
	protected:
		Script(Program& program);

	private:
		Program& program;
	};

	class UserScript : public Script
	{
		friend UserScript* createScript(
				Program&, ASTScript&, CompileErrorHandler&);

	public:
		ScriptType getType() const {return node.type->type;}
		string const& getName() const {return node.name;};
		ASTScript* getNode() const {return &node;};
		ScriptScope& getScope() {return *scope;}
		ScriptScope const& getScope() const {return *scope;}
		
	private:
		UserScript(Program&, ASTScript&);
		
		ASTScript& node;
		ScriptScope* scope;
	};

	class BuiltinScript : public Script
	{
		friend BuiltinScript* createScript(
				Program&, ScriptType, string const& name,
				CompileErrorHandler&);

	public:
		ScriptType getType() const {return type;}
		string const& getName() const {return name;};
		ASTScript* getNode() const {return NULL;};
		ScriptScope& getScope() {return *scope;}
		ScriptScope const& getScope() const {return *scope;}
		
	private:
		BuiltinScript(Program&, ScriptType, string const& name);
		
		ScriptType type;
		string name;
		ScriptScope* scope;
	};

	UserScript* createScript(Program&, ASTScript&, CompileErrorHandler&);
	BuiltinScript* createScript(
			Program&, ScriptType, string const& name, CompileErrorHandler&);
	
	Function* getRunFunction(Script const&);
	optional<int> getLabel(Script const&);

	////////////////////////////////////////////////////////////////
	// Datum
	
	// Something that can be resolved to a data value.
	class Datum
	{
	public:
		// The containing scope.
		Scope& scope;

		// The type of this data.
		DataType type;

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
		Datum(Scope& scope, DataType const& type);

		// Call in static creation function to register with scope.
		bool tryAddToScope(CompileErrorHandler&);
	};

	// Is this datum a global value?
	bool isGlobal(Datum const& data);

	// Return the stack offset of the value.
	optional<int> getStackOffset(Datum const&);

	// A literal value that requires memory management.
	class Literal : public Datum
	{
	public:
		static Literal* create(
				Scope&, ASTLiteral&, DataType const&,
				CompileErrorHandler& = CompileErrorHandler::NONE);
		
		ASTLiteral* getNode() const {return &node;}

	private:
		Literal(Scope& scope, ASTLiteral& node, DataType const& type);

		ASTLiteral& node;
	};

	// A variable.
	class Variable : public Datum
	{
	public:
		static Variable* create(
				Scope&, ASTDataDecl&, DataType const&,
				CompileErrorHandler& = CompileErrorHandler::NONE);
		
		optional<string> getName() const {return node.name;}
		ASTDataDecl* getNode() const {return &node;}
		optional<int> getGlobalId() const {return globalId;}

	private:
		Variable(Scope& scope, ASTDataDecl& node, DataType const& type);

		ASTDataDecl& node;
		optional<int> globalId;
	};

	// A compiler generated variable.
	class BuiltinVariable : public Datum
	{
	public:
		static BuiltinVariable* create(
				Scope&, DataType const&, string const& name,
				CompileErrorHandler& = CompileErrorHandler::NONE);
		
		optional<string> getName() const {return name;}
		optional<int> getGlobalId() const {return globalId;}

	private:
		BuiltinVariable(Scope&, DataType const&, string const& name);

		string const name;
		optional<int> globalId;
	};

	// An inlined constant.
	class Constant : public Datum
	{
	public:
		static Constant* create(
				Scope&, ASTDataDecl&, DataType const&, long value,
				CompileErrorHandler& = CompileErrorHandler::NONE);
		
		optional<string> getName() const;

		optional<long> getCompileTimeValue() const {return value;}

		ASTDataDecl* getNode() const {return &node;}

	private:
		Constant(Scope&, ASTDataDecl&, DataType const&, long value);

		ASTDataDecl& node;
		long value;
	};
	
	// A builtin data value.
	class BuiltinConstant : public Datum
	{
	public:
		static BuiltinConstant* create(
				Scope&, DataType const&, string const& name, long value,
				CompileErrorHandler& = CompileErrorHandler::NONE);
		
		optional<string> getName() const {return name;}
		optional<long> getCompileTimeValue() const {return value;}

	private:
		BuiltinConstant(Scope&, DataType const&,
		                string const& name, long value);

		string name;
		long value;
	};

	////////////////////////////////////////////////////////////////
	// Function
	
	class Function
	{
	public:
		// Comparable signature structure.
		class Signature
		{
		public:
			Signature(string const& name,
			          vector<DataType> const& parameterTypes);
			Signature(Function const& function);

			int compare(Signature const& other) const;
			bool operator==(Signature const& other) const;
			bool operator<(Signature const& other) const;
			string asString() const;
			operator string() const {return asString();}

			string name;
			vector<DataType> parameterTypes;
		};
		
		Function(DataType const& returnType, string const& name,
				 vector<DataType> const& paramTypes, int id);
		
		DataType returnType;
		string name;
		vector<DataType> paramTypes;
		int id;

		ASTFuncDecl* node;
		FunctionScope* internalScope;
		BuiltinVariable* thisVar;

		vector<Opcode> const& getCode() const {return code_;}
		void setCode(vector<Opcode> const& code);
		
		Signature getSignature() const {return Signature(*this);}
		
		// If this is a script level function, return that script.
		Script* getScript() const;

		int getLabel() const;
		
	private:
		mutable optional<int> label_;

		// Code implementing this function.
		vector<Opcode> code_;
	};

	// Is this function a "run" function?
	bool isRun(Function const&);

	// Get the size of the function stack.
	int getStackSize(Function const&);

	// Get the function's parameter count, including "this" if present.
	int getParameterCount(Function const&);
}

#endif

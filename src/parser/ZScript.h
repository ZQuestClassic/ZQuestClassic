#ifndef ZSCRIPT_ZSCRIPT_H
#define ZSCRIPT_ZSCRIPT_H

#include <vector>
#include <map>
#include "AST.h"
#include "CompilerUtils.h"
#include "Types.h"

#define BITS_SP	10
#define MAX_SCRIPT_REGISTERS	(1<<BITS_SP)

namespace ZScript
{
	class CompileErrorHandler;

	class TypeStore;
	class Program;
	class Script;
	class Namespace;
	class Variable;
	class BuiltinVariable;
	class Function;
	class Scope;
	class RootScope;
	class FileScope;
	class ScriptScope;
	class NamespaceScope;
	class FunctionScope;
	
	////////////////////////////////////////////////////////////////
	// Program
	
	class Program : private NoCopy
	{
	public:
		Program(ASTFile&, CompileErrorHandler*);
		~Program();

		ASTFile& getRoot() {return root_;}
		TypeStore const& getTypeStore() const {return typeStore_;}
		TypeStore& getTypeStore() {return typeStore_;}
		RootScope& getScope() const {return *rootScope_;}

		std::vector<Script*> scripts;
		std::vector<Namespace*> namespaces;
		Script* getScript(std::string const& name) const;
		Script* getScript(ASTScript* node) const;
		Script* addScript(ASTScript& node, Scope& parentScope, CompileErrorHandler* handler);
		Namespace* addNamespace(
			ASTNamespace& node, Scope& parentScope, CompileErrorHandler* handler);

		// Gets the non-internal (user-defined) global scope functions.
		std::vector<Function*> getUserGlobalFunctions() const;

		// Gets all user-defined functions.
		std::vector<Function*> getUserFunctions() const;
		// Gets all non-user-defined functions
		std::vector<Function*> getInternalFunctions() const;

		// Return a list of all errors in the script declaration.
		std::vector<CompileError const*> getErrors() const;
		// Does this script have a declaration error?
		bool hasError() const {return (getErrors().size());}

	private:
		std::map<std::string, Script*> scriptsByName_;
		std::map<ASTScript*, Script*> scriptsByNode_;

		TypeStore typeStore_;
		RootScope* rootScope_;
		ASTFile& root_;
	};

	// Gets all defined functions.
	std::vector<Function*> getFunctions(Program const&);

	////////////////////////////////////////////////////////////////
	// Script

	class UserScript;
	class BuiltinScript;
	
	class Script
	{
	public:
		virtual ~Script();

		virtual ScriptType getType() const = 0;
		virtual std::string const& getName() const = 0;
		virtual ASTScript* getNode() const = 0;
		virtual ScriptScope& getScope() = 0;
		virtual ScriptScope const& getScope() const = 0;

		std::vector<Opcode*> code;

	protected:
		Script(Program& program);

	private:
		Program& program;
	};

	class UserScript : public Script
	{
		friend UserScript* createScript(
				Program&, Scope&, ASTScript&, CompileErrorHandler*);

	public:
		ScriptType getType() const /*override*/;
		std::string const& getName() const /*override*/ {return node.name;};
		ASTScript* getNode() const /*override*/ {return &node;};
		ScriptScope& getScope() /*override*/ {return *scope;}
		ScriptScope const& getScope() const /*override*/ {return *scope;}

	private:
		UserScript(Program&, ASTScript&);

		ASTScript& node;
		ScriptScope* scope;
	};

	class BuiltinScript : public Script
	{
		friend BuiltinScript* createScript(
				Program&, Scope&, ScriptType, std::string const& name,
				CompileErrorHandler*);

	public:
		ScriptType getType() const /*override*/ {return type;}
		std::string const& getName() const /*override*/ {return name;};
		ASTScript* getNode() const /*override*/ {return NULL;};
		ScriptScope& getScope() /*override*/ {return *scope;}
		ScriptScope const& getScope() const /*override*/ {return *scope;}
		
	private:
		BuiltinScript(Program&, ScriptType, std::string const& name);
		
		ScriptType type;
		std::string name;
		ScriptScope* scope;
	};

	UserScript* createScript(
			Program&, Scope&, ASTScript&, CompileErrorHandler* = NULL);
	BuiltinScript* createScript(
			Program&, Scope&, ScriptType, std::string const& name,
			CompileErrorHandler* = NULL);
	
	Function* getRunFunction(Script const&);
	optional<int> getLabel(Script const&);

	
	////////////////////////////////////////////////////////////////
	// Namespace
	
	class Namespace
	{
		friend Namespace* createNamespace(Program& program, Scope& parentScope, ASTNamespace& node, CompileErrorHandler* errorHandler);
		
	public:
		Namespace(ASTNamespace& namesp);
		std::string const& getName() const {return name;}
		NamespaceScope& getScope() {return *scope;}
		NamespaceScope const& getScope() const {return *scope;}
		void setScope(NamespaceScope* newscope) {scope = newscope;}
		
	private:
		
		NamespaceScope* scope;
		std::string name;
	};
	
	Namespace* createNamespace(Program& program, Scope& parentScope, ASTNamespace& node, CompileErrorHandler* errorHandler = NULL);
	
	////////////////////////////////////////////////////////////////
	// Datum
	
	// Something that can be resolved to a data value.
	class Datum
	{
	public:
		// The containing scope.
		Scope& scope;

		// The type of this data.
		DataType const& type;

		// Id for lookup tables.
		int const id;

		// Get the data's name.
		virtual optional<std::string> getName() const {return nullopt;}
		
		// Get the value at compile time.
		virtual optional<long> getCompileTimeValue(bool getinitvalue = false) const {return nullopt;}

		// Get the declaring node.
		virtual AST* getNode() const {return NULL;}
		
		// Get the global register this uses.
		virtual optional<int> getGlobalId() const {return nullopt;}
		
		virtual bool isBuiltIn() const {return false;}
		
	protected:
		Datum(Scope& scope, DataType const& type);

		// Call in static creation function to register with scope.
		bool tryAddToScope(CompileErrorHandler* = NULL);
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
				CompileErrorHandler* = NULL);
		
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
				CompileErrorHandler* = NULL);

		optional<std::string> getName() const {return node.name;}
		ASTDataDecl* getNode() const {return &node;}
		optional<int> getGlobalId() const {return globalId;}
		optional<long> getCompileTimeValue(bool getinitvalue = false) const;
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
				Scope&, DataType const&, std::string const& name,
				CompileErrorHandler* = NULL);

		optional<std::string> getName() const {return name;}
		optional<int> getGlobalId() const {return globalId;}

		virtual bool isBuiltIn() const {return true;}
		
	private:
		BuiltinVariable(Scope&, DataType const&, std::string const& name);

		std::string const name;
		optional<int> globalId;
	};

	// An inlined constant.
	class Constant : public Datum
	{
	public:
		static Constant* create(
				Scope&, ASTDataDecl&, DataType const&, long value,
				CompileErrorHandler* = NULL);

		optional<std::string> getName() const;

		optional<long> getCompileTimeValue(bool getinitvalue = false) const {return value;}

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
				Scope&, DataType const&, std::string const& name, long value,
				CompileErrorHandler* = NULL);

		optional<std::string> getName() const {return name;}
		optional<long> getCompileTimeValue(bool getinitvalue = false) const {return value;}

		virtual bool isBuiltIn() const {return true;}
		
	private:
		BuiltinConstant(Scope&, DataType const&,
		                std::string const& name, long value);

		std::string name;
		long value;
	};

	////////////////////////////////////////////////////////////////
	// FunctionSignature

	// Comparable signature structure.
	class FunctionSignature
	{
	public:
		FunctionSignature(
				std::string const& name,
				std::vector<DataType const*> const& parameterTypes);
		FunctionSignature(Function const& function);

		int compare(FunctionSignature const& other) const;
		bool operator==(FunctionSignature const& other) const;
		bool operator<(FunctionSignature const& other) const;
		std::string asString() const;
		operator std::string() const {return asString();}

		std::string name;
		std::vector<DataType const*> parameterTypes;
	};
	
	
	////////////////////////////////////////////////////////////////
	// Function
	
	class Function
	{
	public:
		Function(DataType const* returnType, std::string const& name,
		         std::vector<DataType const*> paramTypes, int id, int flags = 0);
		~Function();
		
		DataType const* returnType;
		std::string name;
		std::vector<DataType const*> paramTypes;
		int id;

		ASTFuncDecl* node;
		FunctionScope* internalScope;
		Datum* thisVar;

		// Get the opcodes.
		std::vector<Opcode*> const& getCode() const {return ownedCode;}
		// Get and remove the code for this function.
		std::vector<Opcode*> takeCode();
		// Add code for this function, transferring ownership.
		// Clears the input vector.
		void giveCode(std::vector<Opcode*>& code);
		
		FunctionSignature getSignature() const {
			return FunctionSignature(*this);}
		
		// If this is a script level function, return that script.
		Script* getScript() const;

		int numParams() const {return paramTypes.size();}
		int getLabel() const;
		void setFlag(int flag, bool state = true)
		{
			if(node) state ? node->flags |= flag : node->flags &= ~flag;
			state ? flags |= flag : flags &= ~flag;
		}
		bool getFlag(int flag) const {return flags & flag;}
		
		bool isInternal() const {return !node;};
		
		// If this is a tracing function (disabled by `#option LOGGING false`)
		bool isTracing() const;
		
	private:
		mutable optional<int> label;
		int flags;

		// Code implementing this function.
		std::vector<Opcode*> ownedCode;
		friend class ASTFuncDecl;
	};

	// Is this function a "run" function?
	bool isRun(Function const&);

	// Get the size of the function stack.
	int getStackSize(Function const&);

	// Get the function's parameter count, including "this" if present.
	int getParameterCount(Function const&);
}

#endif

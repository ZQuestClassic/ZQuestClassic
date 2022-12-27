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
	class UserClass;
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
		std::vector<UserClass*> classes;
		std::vector<Namespace*> namespaces;
		Script* getScript(std::string const& name) const;
		Script* getScript(ASTScript* node) const;
		Script* addScript(ASTScript& node, Scope& parentScope, CompileErrorHandler* handler);
		UserClass* getClass(std::string const& name) const;
		UserClass* getClass(ASTClass* node) const;
		UserClass* addClass(ASTClass& node, Scope& parentScope, CompileErrorHandler* handler);
		Namespace* addNamespace(
			ASTNamespace& node, Scope& parentScope, CompileErrorHandler* handler);

		// Gets the non-internal (user-defined) global scope functions.
		std::vector<Function*> getUserGlobalFunctions() const;

		// Gets all user-defined functions.
		std::vector<Function*> getUserFunctions() const;
		// Gets all non-user-defined functions
		std::vector<Function*> getInternalFunctions() const;
		std::vector<Function*> getUserClassConstructors() const;
		std::vector<Function*> getUserClassDestructors() const;

		// Return a list of all errors in the script declaration.
		std::vector<CompileError const*> getErrors() const;
		// Does this script have a declaration error?
		bool hasError() const {return (getErrors().size()>0);}

	private:
		std::map<std::string, Script*> scriptsByName_;
		std::map<ASTScript*, Script*> scriptsByNode_;
		std::map<std::string, UserClass*> classesByName_;
		std::map<ASTClass*, UserClass*> classesByNode_;
		
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
		virtual std::string const& getAuthor() const = 0;
		virtual zasm_meta const& getMetadata() const = 0;
		virtual ASTScript* getNode() const = 0;
		virtual ScriptScope& getScope() = 0;
		virtual ScriptScope const& getScope() const = 0;
		
		void setRun(Function* func) {runFunc = func;}
		Function* getRun() const {return runFunc;}
		
		bool isPrototypeRun() const;
		
		std::vector<std::shared_ptr<Opcode>> code;

	protected:
		Script(Program& program);

	private:
		Function* runFunc;
		Program& program;
	};

	class UserScript : public Script
	{
		friend UserScript* createScript(
				Program&, Scope&, ASTScript&, CompileErrorHandler*);

	public:
		ScriptType getType() const /*override*/;
		std::string const& getName() const /*override*/ {return node.metadata.script_name;};
		std::string const& getAuthor() const /*override*/ {return node.metadata.author;};
		zasm_meta const& getMetadata() const /*override*/ {return node.metadata;};
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
		std::string const& getAuthor() const /*override*/ {return builtin_author;};
		zasm_meta const& getMetadata() const /*override*/
		{
			static zasm_meta builtin_meta;
			builtin_meta.autogen();
			return builtin_meta;
		};
		ASTScript* getNode() const /*override*/ {return NULL;};
		ScriptScope& getScope() /*override*/ {return *scope;}
		ScriptScope const& getScope() const /*override*/ {return *scope;}
		
	private:
		BuiltinScript(Program&, ScriptType, std::string const& name);
		static const std::string builtin_author;
		
		ScriptType type;
		std::string name;
		ScriptScope* scope;
	};

	UserScript* createScript(
			Program&, Scope&, ASTScript&, CompileErrorHandler* = NULL);
	BuiltinScript* createScript(
			Program&, Scope&, ScriptType, std::string const& name,
			CompileErrorHandler* = NULL);
	
	std::optional<int32_t> getLabel(Script const&);

	////////////////////////////////////////////////////////////////
	// UserClass

	class UserClass
	{
		friend UserClass* createClass(
			Program&, Scope&, ASTClass&, CompileErrorHandler*);
	public:
		~UserClass();

		std::string const& getName() const {return node.name;}
		ASTClass* getNode() const {return &node;}
		ClassScope& getScope() {return *scope;}
		ClassScope const& getScope() const {return *scope;}
		DataType* getType() {return classType;}
		void setType(DataType* t) {classType = t;}
		
		std::vector<int32_t> members;
	protected:
		UserClass(Program& program, ASTClass& user_class);
		DataType* classType;

	private:
		Program& program;
		ASTClass& node;
		ClassScope* scope;
	};

	UserClass* createClass(Program&, Scope&, ASTClass&, CompileErrorHandler* = NULL);

	
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
		int32_t const id;

		// Get the data's name.
		virtual std::optional<std::string> getName() const {return std::nullopt;}
		
		// Get the value at compile time.
		virtual std::optional<int32_t> getCompileTimeValue(bool getinitvalue = false) const {return std::nullopt;}

		// Get the declaring node.
		virtual AST* getNode() const {return NULL;}
		
		// Get the global register this uses.
		virtual std::optional<int32_t> getGlobalId() const {return std::nullopt;}
		
		virtual bool isBuiltIn() const {return false;}
		
	protected:
		Datum(Scope& scope, DataType const& type);

		// Call in static creation function to register with scope.
		bool tryAddToScope(CompileErrorHandler* = NULL);
	};

	// Is this datum a global value?
	bool isGlobal(Datum const& data);

	// Return the stack offset of the value.
	std::optional<int32_t> getStackOffset(Datum const&);

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

		std::optional<std::string> getName() const {return node.name;}
		ASTDataDecl* getNode() const {return &node;}
		std::optional<int32_t> getGlobalId() const {return globalId;}
		std::optional<int32_t> getCompileTimeValue(bool getinitvalue = false) const;
	private:
		Variable(Scope& scope, ASTDataDecl& node, DataType const& type);

		ASTDataDecl& node;
		std::optional<int32_t> globalId;
	};
	
	//A UserClass variable
	class UserClassVar : public Datum
	{
	public:
		static UserClassVar* create(
				Scope&, ASTDataDecl&, DataType const&,
				CompileErrorHandler* = NULL);

		std::optional<std::string> getName() const {return node.name;}
		ASTDataDecl* getNode() const {return &node;}
		UserClass* getClass() const {return &(scope.getClass()->user_class);}
		int32_t getIndex() const {return _index;}
		void setIndex(int32_t ind) {_index = ind;}
		int32_t getOrder() const {return _order_ind;}
		void setOrder(int32_t ind) {_order_ind = ind;}
		
		bool is_arr;
	private:
		UserClassVar(Scope& scope, ASTDataDecl& node, DataType const& type);
		
		int32_t _index;
		int32_t _order_ind;
		ASTDataDecl& node;
	};

	// A compiler generated variable.
	class BuiltinVariable : public Datum
	{
	public:
		static BuiltinVariable* create(
				Scope&, DataType const&, std::string const& name,
				CompileErrorHandler* = NULL);

		std::optional<std::string> getName() const {return name;}
		std::optional<int32_t> getGlobalId() const {return globalId;}

		virtual bool isBuiltIn() const {return true;}
		
	private:
		BuiltinVariable(Scope&, DataType const&, std::string const& name);

		std::string const name;
		std::optional<int32_t> globalId;
	};

	// An inlined constant.
	class Constant : public Datum
	{
	public:
		static Constant* create(
				Scope&, ASTDataDecl&, DataType const&, int32_t value,
				CompileErrorHandler* = NULL);

		std::optional<std::string> getName() const;

		std::optional<int32_t> getCompileTimeValue(bool getinitvalue = false) const {return value;}

		ASTDataDecl* getNode() const {return &node;}
	
	private:
		Constant(Scope&, ASTDataDecl&, DataType const&, int32_t value);

		ASTDataDecl& node;
		int32_t value;
	};

	// A builtin data value.
	class BuiltinConstant : public Datum
	{
	public:
		static BuiltinConstant* create(
				Scope&, DataType const&, std::string const& name, int32_t value,
				CompileErrorHandler* = NULL);

		std::optional<std::string> getName() const {return name;}
		std::optional<int32_t> getCompileTimeValue(bool getinitvalue = false) const {return value;}

		virtual bool isBuiltIn() const {return true;}
		
	private:
		BuiltinConstant(Scope&, DataType const&,
		                std::string const& name, int32_t value);

		std::string name;
		int32_t value;
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

		int32_t compare(FunctionSignature const& other) const;
		bool operator==(FunctionSignature const& other) const;
		bool operator<(FunctionSignature const& other) const;
		std::string asString() const;
		operator std::string() const {return asString();}

		std::string name;
		bool prefix;
		std::vector<DataType const*> parameterTypes;
	};
	
	
	////////////////////////////////////////////////////////////////
	// Function
	
	class Function
	{
	public:
		Function(DataType const* returnType, std::string const& name,
		         std::vector<DataType const*> paramTypes, std::vector<std::string const*> paramNames,
		         int32_t id, int32_t flags = 0, int32_t internal_flags = 0, bool prototype = false, ASTExprConst* defaultReturn = NULL);
		~Function();
		
		DataType const* returnType;
		std::string name, info;
		bool hasPrefixType;
		byte extra_vargs;
		std::vector<DataType const*> paramTypes;
		std::vector<std::string const*> paramNames;
		std::vector<int32_t> opt_vals;
		int32_t id;

		ASTFuncDecl* node;
		FunctionScope* internalScope;
		Datum* thisVar;

		// Get the opcodes.
		std::vector<std::shared_ptr<Opcode>> const& getCode() const {return ownedCode;}
		// Get and remove the code for this function.
		std::vector<std::shared_ptr<Opcode>> takeCode();
		// Add code for this function, transferring ownership.
		// Clears the input vector.
		void giveCode(std::vector<std::shared_ptr<Opcode>>& code);
		
		FunctionSignature getSignature() const {
			return FunctionSignature(*this);}
		
		// If this is a script level function, return that script.
		Script* getScript() const;
		UserClass* getClass() const;

		int32_t numParams() const {return paramTypes.size();}
		int32_t getLabel() const;
		int32_t getAltLabel() const;
		void setFlag(int32_t flag, bool state = true)
		{
			if(node) state ? node->flags |= flag : node->flags &= ~flag;
			state ? flags |= flag : flags &= ~flag;
		}
		bool getFlag(int32_t flag) const {return (flags & flag) != 0;}
		
		bool isInternal() const {return !node;};
		
		// If this is a tracing function (disabled by `#option LOGGING false`)
		bool isTracing() const;
		int32_t internal_flags;
		bool prototype;
		ASTExprConst* defaultReturn;
		
		bool shouldShowDepr(bool err) const;
		void ShownDepr(bool err);
		
	private:
		mutable std::optional<int32_t> label;
		mutable std::optional<int32_t> altlabel;
		int32_t flags;
		byte shown_depr;

		// Code implementing this function.
		std::vector<std::shared_ptr<Opcode>> ownedCode;
		friend class ASTFuncDecl;
	};

	// Is this function a "run" function?
	bool isRun(Function const&);

	// Get the size of the function stack.
	int32_t getStackSize(Function const&);

	// Get the function's parameter count, including "this" if present.
	int32_t getParameterCount(Function const&);
}

#endif

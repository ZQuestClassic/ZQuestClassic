#ifndef ZSCRIPT_SCOPE_H_
#define ZSCRIPT_SCOPE_H_

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
	class ASTNamespace;
	class ASTExprIdentifier;
	class ASTImportDecl;
	class ASTExprCall;
	class ASTBlock;
	class ASTFuncDecl;

	// CompileError.h
	class CompileErrorHandler;
	
	// Types.h
	class DataType;
	class TypeStore;
	class ParserScriptType;

	// ZScript.h
	class Script;
	class UserClass;
	class Datum;
	class UserClassVar;
	class Namespace;
	class Function;
	class FunctionSignature;

	// Local forward declarations
	class ZClass;
	class RootScope;
	class FileScope;
	class ScriptScope;
	class ClassScope;
	class FunctionScope;
	class NamespaceScope;

	////////////////////////////////////////////////////////////////
	
	static int32_t ScopeID = 0;

	class Scope : private NoCopy
	{
		// So Datum classes can only be generated in tandem with a scope.
		friend class Datum;
		
	public:
		Scope(TypeStore&);
		Scope(TypeStore&, std::string const& name);
		
		virtual ~Scope() {}
		// Scope type.
		virtual bool isGlobal() const {return false;}
		virtual bool isRoot() const {return false;}
		virtual bool isScript() const {return false;}
		virtual bool isClass() const {return false;}
		virtual bool isFunction() const {return false;}
		virtual bool isNamespace() const {return false;}
		virtual bool isFile() const {return false;}
		virtual bool isNamedEnum() const {return false;}
		
		// Accessors
		TypeStore const& getTypeStore() const {return typeStore_;}
		TypeStore& getTypeStore() {return typeStore_;}
		std::optional<std::string> const& getName() const {return name_;}
		std::optional<std::string>& getName() {return name_;}

		// Inheritance
		virtual Scope* getParent() const = 0;
		virtual Scope* getChild(std::string const& name) const = 0;
		virtual std::vector<Scope*> getChildren() const = 0;
		virtual FileScope* getFile() const = 0;
		virtual ScriptScope* getScript() = 0;
		virtual ClassScope* getClass() = 0;
		virtual int32_t useNamespace(std::string name, bool noUsing) = 0;
		virtual int32_t useNamespace(std::vector<std::string> names, std::vector<std::string> delimiters, bool noUsing) = 0;
	
		// Lookup Local
		virtual DataType const* getLocalDataType(std::string const& name)
			const = 0;
		virtual std::optional<ParserScriptType> getLocalScriptType(
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
		virtual std::vector<NamespaceScope*> getUsingNamespaces() const = 0;

		// Add
		virtual Scope* makeChild() = 0;
		virtual Scope* makeChild(std::string const& name) = 0;
		virtual FileScope* makeFileChild(std::string const& filename) = 0;
		virtual ScriptScope* makeScriptChild(Script& script) = 0;
		virtual ClassScope* makeClassChild(UserClass& user_class) = 0;
		virtual NamespaceScope* makeNamespaceChild(ASTNamespace& node) = 0;
		virtual FunctionScope* makeFunctionChild(Function& function) = 0;
		virtual DataType const* addDataType(
				std::string const& name, DataType const* type, AST* node)
		= 0;
		virtual bool addScriptType(
			std::string const& name, ParserScriptType type, AST* node) = 0;
		//virtual ZClass* addClass(string const& name, AST* node) = 0;
		virtual Function* addGetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes, std::vector<std::shared_ptr<const std::string>> const& paramNames,
				int32_t flags = 0, AST* node = NULL)
		= 0;
		virtual Function* addSetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes, std::vector<std::shared_ptr<const std::string>> const& paramNames,
				int32_t flags = 0, AST* node = NULL)
		= 0;
		virtual void addGetter(Function* func) = 0;
		virtual void addSetter(Function* func) = 0;
		virtual Function* addFunction(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes, std::vector<std::shared_ptr<const std::string>> const& paramNames,
				int32_t flags = 0, ASTFuncDecl* node = NULL, CompileErrorHandler* handler = NULL, Scope* subscope = NULL)
		= 0;
		virtual bool addAlias(Function* funcptr, CompileErrorHandler* handler = NULL) = 0;
		virtual void removeFunction(Function* func) = 0;
		virtual void setDefaultOption(CompileOptionSetting value) = 0;
		virtual void setOption(
				CompileOption option, CompileOptionSetting value) = 0;

		////////////////
		// Stack

		// If this scope starts a new stack frame, return its total stack
		// size.
		virtual std::optional<int32_t> getRootStackSize() const {return std::nullopt;}

		// Let this scope know that it needs to recalculate the stack size.
		virtual void invalidateStackSize();
		
		// Get the depth of the stack for this scope, not considering its
		// children.
		virtual int32_t getLocalStackDepth() const {return 0;}

		// Get the stack offset for this local datum.
		virtual std::optional<int32_t> getLocalStackOffset(Datum const&) const {
			return std::nullopt;}
			
		//
		bool operator==(Scope* other) {return id == other->getId();}
		
		virtual bool remove(ZScript::Datum&) = 0;

		void initFunctionBinding(Function* fn, CompileErrorHandler* handler);
	
	Scope* lexical_options_scope;

	protected:
		TypeStore& typeStore_;
		std::optional<std::string> name_;
		std::vector<NamespaceScope*> usingNamespaces;
		int32_t getId() const {return id;}

	private:
		// Add/Remove the datum to this scope, returning if successful. Called by
		// the Datum classes' ::create functions.
		virtual bool add(ZScript::Datum&, CompileErrorHandler*) = 0;
		int32_t id;
	};

	////////////////
	// Inheritance

	// Repeatedly get a child namespace with the names in order. Fail if any
	// name does not resolve.
	Scope* getDescendant(
			Scope const&, std::vector<std::string> const& names, std::vector<std::string> const& delimiters);

	// Find a scope with the given name in this scope.
	Scope* lookupScope(Scope const&, std::string const& name, bool noUsing, AST& host, CompileErrorHandler* errorHandler);

	// Find first scope with the given ancestry in this scope.
	Scope* lookupScope(Scope const&, std::vector<std::string> const& names, std::vector<std::string> const& delimiters, bool noUsing, AST& host, CompileErrorHandler* errorHandler);

	// Find all scopes with the given ancestry in this scope. Note than an
	// empty name list will the current scope and its ancestry.
	std::vector<Scope*> lookupScopes(
			Scope const&, std::vector<std::string> const& names, std::vector<std::string> const& delimiters, bool noUsing);
	// This version will ONLY get scopes from usingNamespaces, not normally available scopes
	std::vector<Scope*> lookupUsingScopes(
			Scope const&, std::vector<std::string> const& names, std::vector<std::string> const& delimiters);

	// Get the most distant parent.
	RootScope* getRoot(Scope const&);
	
	////////////////
	// Lookup

	// Attempt to resolve name to a type id under scope.
	DataType const* lookupDataType(Scope const& scope, std::string const& name, ASTExprIdentifier& host, CompileErrorHandler* errorHandler, bool isTypedefCheck = false, bool forceSkipUsing = false);
	DataType const* lookupDataType(Scope const& scope, ASTExprIdentifier& host, CompileErrorHandler* errorHandler, bool isTypedefCheck = false);
	
	// Attempt to resolve name to a script type id under scope.
	ParserScriptType lookupScriptType(Scope const&, std::string const& name);
	
	// Attempt to resolve name to a class id under scope.
	ZClass* lookupClass(Scope const&, std::string const& name);

	// Attempt to resolve name to a variable under scope.
	Datum* lookupDatum(Scope &, std::string const& name, ASTExprIdentifier& host, CompileErrorHandler* errorHandler, bool forceSkipUsing = false);
	Datum* lookupDatum(Scope &, ASTExprIdentifier& host, CompileErrorHandler* errorHandler);
	UserClassVar* lookupClassVars(Scope& scope, ASTExprIdentifier& host, CompileErrorHandler* errorHandler);
	
	// Attempt to resolve name to a getter under scope.
	Function* lookupGetter(Scope const&, std::string const& name);

	// Attempt to resolve name to a setter under scope.
	Function* lookupSetter(Scope const&, std::string const& name);

	// Attempt to resolve signature to a function under scope.
	//Function* lookupFunction(Scope const&, FunctionSignature const&); //Disabled, as nothing uses this. -V
	
	// Attempt to resolve name to possible functions under scope.
	std::vector<Function*> lookupFunctions(
			Scope&, std::string const& name, std::vector<DataType const*> const& parameterTypes, bool noUsing, bool isClass = false, bool skipParamCheck = false, Scope const* caller_scope = nullptr);
	std::vector<Function*> lookupFunctions(
			Scope&, std::vector<std::string> const& name, std::vector<std::string> const& delimiters, std::vector<DataType const*> const& parameterTypes, bool noUsing, bool isClass = false, bool skipParamCheck = false, Scope const* caller_scope = nullptr);
	
	UserClass* lookupClass(Scope& scope, std::string const& name, bool noUsing);
	UserClass* lookupClass(Scope& scope, std::vector<std::string> const& names,
		std::vector<std::string> const& delimiters, bool noUsing);
	std::vector<Function*> lookupConstructors(UserClass const& user_class, std::vector<DataType const*> const& parameterTypes, Scope const* scope);
	std::vector<Function*> lookupClassFuncs(UserClass const& user_class,
		std::string const& name, std::vector<DataType const*> const& parameterTypes, Scope const* scope, bool ignoreParams = false);
	inline void trimBadFunctions(std::vector<Function*>& functions, std::vector<DataType const*> const& parameterTypes, Scope const* scope, bool trimClasses = true);

	// Resolve an option value under the scope. Will only return empty if
	// the provided option is invalid. If the option is valid but not set,
	// returns the default value for it.
	std::optional<int32_t> lookupOption(Scope const&, CompileOption);
	std::optional<int32_t> lookupOption(Scope const*, CompileOption);
	
	std::vector<NamespaceScope*> lookupUsingNamespaces(Scope const& scope);

	////////////////
	// Stack

	// Does this scope start a new stack frame?
	bool isStackRoot(Scope const&);

	// Get the stack offset for a datum, checking parents until we hit a
	// root.
	std::optional<int32_t> lookupStackOffset(Scope const&, Datum const&);
	// Remove a datum from existence
	bool eraseDatum(Scope const& scope, Datum& datum);

	// Find the total size of the stack scope is in.
	std::optional<int32_t> lookupStackSize(Scope const&);
	
	// Lookup the stack offset and then subtract it from the root stack
	// size.
	std::optional<int32_t> lookupStackPosition(Scope const&, Datum const&);
	
	////////////////
	// Get all in branch

	// Recursively get all of something for a scope and its children.
	// usage: getInBranch<Function*>(scope, &Scope::getLocalFunctions)
	template <typename Element>
	std::vector<Element> getInBranch(
			Scope const& scope,
			std::vector<Element> (Scope::* call)() const,
			bool skipFile = false)
	{
		std::vector<Element> results;
		if(!(skipFile && scope.isFile())) results = (scope.*call)();
		std::vector<Scope*> children = scope.getChildren();
		for (std::vector<Scope*>::const_iterator it = children.begin();
		     it != children.end(); ++it)
		{
			std::vector<Element> subResults = getInBranch(**it, call, skipFile);
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
		BasicScope(Scope* parent, FileScope* parentFile);
		BasicScope(Scope* parent, FileScope* parentFile, std::string const& name);
		virtual ~BasicScope();
		
		// Inheritance
		virtual Scope* getParent() const {return parent_;}
		virtual Scope* getChild(std::string const& name) const;
		virtual std::vector<Scope*> getChildren() const;
		virtual FileScope* getFile() const {return parentFile_;}
		virtual ScriptScope* getScript();
		virtual ClassScope* getClass();
		virtual int32_t useNamespace(std::string name, bool noUsing);
		virtual int32_t useNamespace(std::vector<std::string> names, std::vector<std::string> delimiters, bool noUsing);
	
		// Lookup Local
		DataType const* getLocalDataType(std::string const& name)
			const /*override*/;
		std::optional<ParserScriptType> getLocalScriptType(std::string const& name)
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
		virtual std::vector<NamespaceScope*> getUsingNamespaces() const {return usingNamespaces;};

		// Add
		virtual Scope* makeChild();
		virtual Scope* makeChild(std::string const& name);
		virtual FileScope* makeFileChild(std::string const& filename);
		virtual ScriptScope* makeScriptChild(Script& script);
		virtual ClassScope* makeClassChild(UserClass& user_class);
		virtual NamespaceScope* makeNamespaceChild(ASTNamespace& node);
		virtual FunctionScope* makeFunctionChild(Function& function);
		virtual DataType const* addDataType(
				std::string const& name, DataType const* type,
				AST* node = NULL);
		bool addScriptType(
			std::string const& name, ParserScriptType type, AST* node)
			/*override*/;
		virtual Function* addGetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes, std::vector<std::shared_ptr<const std::string>> const& paramNames,
				int32_t flags = 0, AST* node = NULL);
		virtual Function* addSetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes, std::vector<std::shared_ptr<const std::string>> const& paramNames,
				int32_t flags = 0, AST* node = NULL);
		virtual void addGetter(Function* func);
		virtual void addSetter(Function* func);
		virtual Function* addFunction(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes, std::vector<std::shared_ptr<const std::string>> const& paramNames,
				int32_t flags = 0, ASTFuncDecl* node = NULL, CompileErrorHandler* handler = NULL, Scope* subscope = NULL);
		virtual bool addAlias(Function* funcptr, CompileErrorHandler* handler = NULL);
		virtual void removeFunction(Function* func);
		virtual void setDefaultOption(CompileOptionSetting value);
		virtual void setOption(
				CompileOption option, CompileOptionSetting value);
		
		// Stack
		virtual int32_t getLocalStackDepth() const {return stackDepth_;}
		virtual std::optional<int32_t> getLocalStackOffset(Datum const& datum) const;

		int32_t stackDepth_;
		virtual bool remove(Datum&);
		Scope* parent_;
	protected:
		FileScope* parentFile_;
		std::map<std::string, Scope*> children_;
		std::vector<Scope*> anonymousChildren_;
		std::map<std::string, DataType const*> dataTypes_;
		std::map<std::string, ParserScriptType> scriptTypes_;
		std::map<std::string, ZClass*> classes_;
		std::vector<Datum*> anonymousData_;
		std::map<std::string, Datum*> namedData_;
		std::map<Datum*, int32_t> stackOffsets_;
		std::map<std::string, Function*> getters_;
		std::map<std::string, Function*> setters_;
		std::map<std::string, std::vector<Function*> > functionsByName_;
		std::map<FunctionSignature, Function*> functionsBySignature_;
		std::map<CompileOption, CompileOptionSetting> options_;
		CompileOptionSetting defaultOption_;

		BasicScope(TypeStore&);
		BasicScope(TypeStore&, std::string const& name);

		virtual bool can_add(Datum&, CompileErrorHandler* errorHandler = nullptr);
		virtual bool add(Datum&, CompileErrorHandler*);
		void decr_stack_recursive(int32_t offset);
		
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
		virtual bool isFile() const {return true;}
		
		void setFile() {parentFile_ = this;}
		
		// Override to also register in the root scope, and fail if already
		// present there as well.
		virtual Scope* makeChild(std::string const& name);
		virtual ScriptScope* makeScriptChild(Script& script);
		virtual ClassScope* makeClassChild(UserClass& user_class);
		virtual NamespaceScope* makeNamespaceChild(ASTNamespace& node);
		virtual DataType const* addDataType(
				std::string const& name, DataType const* type,
				AST* node = NULL);
		bool addScriptType(
			std::string const& name, ParserScriptType type, AST* node)
			/*override*/;
		virtual Function* addGetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes, std::vector<std::shared_ptr<const std::string>> const& paramNames,
				int32_t flags = 0, AST* node = NULL);
		virtual Function* addSetter(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes, std::vector<std::shared_ptr<const std::string>> const& paramNames,
				int32_t flags = 0, AST* node = NULL);
		virtual Function* addFunction(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes, std::vector<std::shared_ptr<const std::string>> const& paramNames,
				int32_t flags = 0, ASTFuncDecl* node = NULL, CompileErrorHandler* handler = NULL, Scope* subscope = NULL);
		virtual bool addAlias(Function* funcptr, CompileErrorHandler* handler = NULL);
		virtual void removeFunction(Function* func);
		void removeLocalFunction(Function* function);
		
	protected:
		virtual bool can_add(Datum&, CompileErrorHandler* errorHandler = nullptr);
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
		virtual bool isRoot() const {return true;}
		virtual std::optional<int32_t> getRootStackSize() const;

		// Also check the descendant listings.
		// Single
		virtual Scope* getChild(std::string const& name) const;
		virtual DataType const* getLocalDataType(
				std::string const& name) const;
		std::optional<ParserScriptType> getLocalScriptType(std::string const& name)
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
		bool registerScriptType(std::string const& name, ParserScriptType type);
		bool registerClass(std::string const& name, ZClass* klass);
		bool registerDatum(std::string const& name, Datum* datum);
		bool registerGetter(std::string const& name, Function* getter);
		bool registerSetter(std::string const& name, Function* setter);
		bool registerFunction(Function* function);
		virtual void removeFunction(Function* func);
		std::optional<Function*> getDescFuncBySig(FunctionSignature& sig);
		
		bool checkImport(ASTImportDecl* node, CompileErrorHandler* errorHandler);
		bool isImported(std::string const& path);
	private:
		mutable std::optional<int32_t> stackSize_;

		// Unowned pointers to descendant's stuff.
		std::map<std::string, Scope*> descChildren_;
		std::map<std::string, DataType const*> descDataTypes_;
		std::map<std::string, ParserScriptType> descScriptTypes_;
		std::map<std::string, ZClass*> descClasses_;
		std::map<std::string, Datum*> descData_;
		std::map<std::string, Function*> descGetters_;
		std::map<std::string, Function*> descSetters_;
		std::map<std::string, std::vector<Function*> > descFunctionsByName_;
		std::map<FunctionSignature, Function*> descFunctionsBySignature_;
		std::map<std::string, ASTImportDecl*> importsByName_;
	};
	
	////////////////////////////////////////////////////////////////
	
	class ScriptScope : public BasicScope
	{
	public:
		ScriptScope(Scope* parent, FileScope* parentFile, Script& script);
		virtual bool isScript() const {return true;}
		Script& script;
	};
	class ClassScope : public BasicScope
	{
	public:
		ClassScope(Scope* parent, FileScope* parentFile, UserClass& user_class);
		virtual bool isClass() const {return true;}
		UserClass& user_class;
		
		std::vector<Function*> getMemberFuncs() const;
		std::vector<Function*> getConstructors() const;
		std::vector<Function*> getDestructor() const;
		bool add(Datum& datum, CompileErrorHandler* errorHandler);
		void removeFunction(Function* function);
		void parse_ucv();
		UserClassVar* getClassVar(std::string const& name);
		const std::map<std::string, UserClassVar*>& getClassData();
		virtual Function* addFunction(
				DataType const* returnType, std::string const& name,
				std::vector<DataType const*> const& paramTypes, std::vector<std::shared_ptr<const std::string>> const& paramNames,
				int32_t flags = 0, ASTFuncDecl* node = NULL, CompileErrorHandler* handler = NULL, Scope* subscope = NULL);
	private:
		std::map<FunctionSignature, Function*> constructorsBySignature_;
		Function* destructor_;
		std::map<std::string, UserClassVar*> classData_;
	};

	class FunctionScope : public BasicScope
	{
	public:
		FunctionScope(Scope* parent, FileScope* parentFile, Function& function);
		bool isFunction() const {return true;}
		Function& function;
		std::optional<int32_t> getRootStackSize() const;
	private:
		mutable std::optional<int32_t> stackSize;
	};
	
	class NamespaceScope : public BasicScope
	{
	public:
		NamespaceScope(Scope* parent, FileScope* parentFile, Namespace* namesp);
		~NamespaceScope();
		virtual bool isGlobal() const {return true;}
		virtual bool isNamespace() const {return true;};
		Namespace* namesp;
		Scope* current_lexical_scope;
	};

	class InlineScope : public BasicScope
	{
	public:
		InlineScope(Scope* parent, FileScope* parentFile, ASTExprCall* node, ASTBlock* block);
		ASTExprCall* node;
		ASTBlock* block;
	};
	
	// enum ZClassIdBuiltin
	// {
	// 	ZCLID_START = 0,
	// 	ZCLID_GAME = 0,
	// 	// ZCLID_PLAYER,
	// 	// ZCLID_SCREEN,
	// 	// ZCLID_REGION,
	// 	// ZCLID_FFC,
	// 	// ZCLID_ITEM,
	// 	// ZCLID_ITEMCLASS,
	// 	// ZCLID_NPC,
	// 	// ZCLID_LWPN,
	// 	// ZCLID_EWPN,
	// 	// ZCLID_NPCDATA,
	// 	// ZCLID_DEBUG,
	// 	// ZCLID_AUDIO,
	// 	// ZCLID_COMBOS,
	// 	ZCLID_SPRITEDATA,
	// 	ZCLID_GRAPHICS,
	// 	ZCLID_BITMAP,
	// 	ZCLID_TEXT,
	// 	ZCLID_INPUT,
	// 	ZCLID_MAPDATA,
	// 	ZCLID_DMAPDATA,
	// 	ZCLID_ZMESSAGE,
	// 	ZCLID_SHOPDATA,
	// 	ZCLID_DROPSET,
	// 	// ZCLID_PONDS,
	// 	ZCLID_WARPRING,
	// 	ZCLID_DOORSET,
	// 	ZCLID_ZUICOLOURS,
	// 	ZCLID_RGBDATA,
	// 	ZCLID_END
	// };

	class ZClass : public BasicScope
	{
	public:
		ZClass(TypeStore&, std::string const& name, int32_t id);
		std::string const name;
		int32_t const id;
	};

};
#endif

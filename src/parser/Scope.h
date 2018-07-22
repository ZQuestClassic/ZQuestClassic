#ifndef ZPARSER_SCOPE_H
#define ZPARSER_SCOPE_H

#include <assert.h>
#include "DataStructs.h"

class Scope
{
public:
	struct Variable
	{
		Variable(ZVarType const* type, string const& name, int id) : type(type), name(name), id(id) {}
		ZVarType const* type;
		string name;
		int id;
	};

	static Scope* makeGlobalScope(SymbolTable& table);

	Scope(SymbolTable& table);

	// SymbolTable
	SymbolTable const& getTable() const {return table;}
	SymbolTable& getTable() {return table;}

	// Others
	virtual bool isGlobal() const {return false;}

	////////////////
	// Virtual Methods

	// Inheritance
	virtual Scope* getParent() const = 0;
	virtual Scope* makeChild(string const& name) = 0;
	virtual Scope* getLocalChild(string const& name) const = 0;
	// Types
	virtual int getLocalTypeId(string const& name) const = 0;
	virtual int addType(string const& name, ZVarTypeId typeId, AST* node) = 0;
	// Classes
	virtual int getLocalClassId(string const& name) const = 0;
	virtual int addClass(string const& name, AST* node) = 0;
	// Variables
	virtual Variable* getLocalVariable(string const& name) const = 0;
	virtual Variable* addVariable(ZVarType const& type, string const& name, AST* node = NULL) = 0;
	// Properties
	virtual int getLocalGetterId(int varId) const = 0;
	virtual int getLocalSetterId(int varId) const = 0;
	virtual int addGetter(int varId, vector<ZVarTypeId> const& paramTypeIds, AST* node) = 0;
	virtual int addSetter(int varId, vector<ZVarTypeId> const& paramTypeIds, AST* node) = 0;
	// Functions
	virtual void getLocalFunctionIds(vector<int>& ids, string const& name) const = 0;
	virtual int getLocalFunctionId(FunctionSignature const& signature) const = 0;
	virtual int addFunction(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node) = 0;

	////////////////
	// Convenience Methods

	// Inheritance
	Scope& getOrMakeLocalChild(string const& name);
	Scope* getNamespace(string const& name) const;
	Scope* getLocalChild(vector<string> const& names) const;
	Scope* getChild(vector<string>const& names)const;
	// Types
	int getTypeId(string const& name) const;
	ZVarType* getLocalType(string const& name) const;
	ZVarType* getType(string const& name) const;
	int addType(string const& name, ZVarType const& type, AST* node);
	int addType(string const& name, ZVarTypeId typeId);
	int addType(string const& name, ZVarType const& type);
	// Classes
	int getClassId(string const& name) const;
	ZClass* getLocalClass(string const& name) const;
	ZClass* getClass(string const& name) const;
	int addClass(string const& name);
	// Variables
	int getLocalVariableId(string const& name) const;
	Variable* getVariable(string const& name) const;
	int getVariableId(string const& name) const;
	Variable* getVariable(vector<string> const& name) const;
	int getVariableId(vector<string> const& name) const;
	Variable* addVariable(ZVarTypeId typeId, string const& name, AST* node = NULL);
	// Properties
	int getGetterId(int varId) const;
	int getGetterId(string const& name) const;
	int getSetterId(int varId) const;
	int getSetterId(string const& name) const;
	int addGetter(int varId, vector<ZVarTypeId> const& paramTypeIds);
	int addSetter(int varId, vector<ZVarTypeId> const& paramTypeIds);
	// Functions
	vector<int> getLocalFunctionIds(string const& name) const;
	void getFunctionIds(vector<int>& ids, string const& name) const;
	vector<int> getFunctionIds(string const& name) const;
	vector<int> getFunctionIds(vector<string> const& names)const;
	int getFunctionId(FunctionSignature const& signature) const;
	int addFunction(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes, AST* node);
	int addFunction(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds);
	int addFunction(string const& name, ZVarType const& returnType, vector<ZVarType*> const& paramTypes);

	bool varDeclsDeprecated;
protected:
	SymbolTable& table;
};

class BasicScope : public Scope
{
public:
	BasicScope(SymbolTable& table);
	BasicScope(Scope* parent);
	~BasicScope();
	// Children.
	Scope* getParent() const;
	Scope* makeChild(string const& name);
	Scope* getLocalChild(string const& name) const;
	// Types
	int getLocalTypeId(string const& name) const;
	using Scope::addType;
	int addType(string const& name, ZVarTypeId typeId, AST* node);
	// Classes
	int getLocalClassId(string const& name) const;
	int addClass(string const& name, AST* node);
	// Variables
	Variable* getLocalVariable(string const& name) const;
	using Scope::addVariable;
	Variable* addVariable(ZVarType const& type, string const& name, AST* node = NULL);
	// Properties
	int getLocalGetterId(int varId) const;
	int getLocalSetterId(int varId) const;
	using Scope::addGetter;
	int addGetter(int varId, vector<ZVarTypeId> const& paramTypeIds, AST* node);
	using Scope::addSetter;
	int addSetter(int varId, vector<ZVarTypeId> const& paramTypeIds, AST* node);
	// Functions
	using Scope::getLocalFunctionIds;
	void getLocalFunctionIds(vector<int>& ids, string const& name) const;
	int getLocalFunctionId(FunctionSignature const& signature) const;
	using Scope::addFunction;
	int addFunction(string const& name, ZVarTypeId returnTypeId, vector<ZVarTypeId> const& paramTypeIds, AST* node);
private:
    Scope* parent;
    map<string, Scope*> children;
	map<string, int> types;
	map<string, int> classes;
	map<string, Variable*> variables;
	map<int, int> getters;
	map<int, int> setters;
	map<string, vector<int> > functionsByName;
	map<FunctionSignature, int> functionsBySignature;

	// Disabled since it's easy to call by accident instead of the Scope*
	// constructor.
	BasicScope(BasicScope const& base) : Scope(base.table) {assert(false);}
};

class GlobalScope : public BasicScope
{
public:
	GlobalScope(SymbolTable& table);
	bool isGlobal() const {return true;}
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
	ZClass(SymbolTable& table, string const& name, int id);
	string const name;
	int const id;
};

#endif

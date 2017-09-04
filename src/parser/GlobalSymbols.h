#ifndef GLOBALSYMBOLS_H
#define GLOBALSYMBOLS_H

#include "DataStructs.h"
#include <string>
#include <map>
#include <vector>

using std::string;
using std::map;
using std::vector;

class SymbolTable;
namespace ZScript
{
	class Function;
	class Scope;
}

static const int SETTER = 0;
static const int GETTER = 1;
static const int FUNCTION = 2;

struct AccessorTable
{
    string name;
    int rettype;
    int setorget;
    int var;
    int numindex;
    int params[20];
};

class LibrarySymbols
{
public:
	static LibrarySymbols* getTypeInstance(DataTypeId typeId);

	virtual void addSymbolsToScope(ZScript::Scope& scope);
    virtual ~LibrarySymbols();

protected:
    AccessorTable *table;
	LibrarySymbols() : refVar(0) {}
    int refVar;

	ZScript::Function* getFunction(string const& name) const;

private:
    map<string, ZScript::Function*> functions;
	
	// Generates the code for functions which can't be auto generated.
	virtual void generateCode() = 0;
};

class GlobalSymbols : public LibrarySymbols
{
public:
    static GlobalSymbols &getInst()
    {
        return singleton;
    }

private:
    static GlobalSymbols singleton;
    GlobalSymbols();
	void generateCode();
};

class FFCSymbols : public LibrarySymbols
{
public:
    static FFCSymbols &getInst()
    {
        return singleton;
    }
private:
    static FFCSymbols singleton;
    FFCSymbols();
	void generateCode();
};

class LinkSymbols : public LibrarySymbols
{
public:
    static LinkSymbols &getInst()
    {
        return singleton;
    }
private:
    static LinkSymbols singleton;
    LinkSymbols();
	void generateCode();
};

class ScreenSymbols : public LibrarySymbols
{
public:
    static ScreenSymbols &getInst()
    {
        return singleton;
    }
private:
    static ScreenSymbols singleton;
    ScreenSymbols();
	void generateCode();
};

class ItemSymbols : public LibrarySymbols
{
public:
    static ItemSymbols &getInst()
    {
        return singleton;
    }
private:
    static ItemSymbols singleton;
    ItemSymbols();
	void generateCode();
};

class ItemclassSymbols : public LibrarySymbols
{
public:
    static ItemclassSymbols &getInst()
    {
        return singleton;
    }
private:
    static ItemclassSymbols singleton;
    ItemclassSymbols();
	void generateCode();
};

class GameSymbols : public LibrarySymbols
{
public:
    static GameSymbols &getInst()
    {
        return singleton;
    }
private:
    static GameSymbols singleton;
    GameSymbols();
	void generateCode();
};

class NPCSymbols : public LibrarySymbols
{
public:
    static NPCSymbols &getInst()
    {
        return singleton;
    }
private:
    static NPCSymbols singleton;
    NPCSymbols();
	void generateCode();
};

class LinkWeaponSymbols : public LibrarySymbols
{
public:
    static LinkWeaponSymbols &getInst()
    {
        return singleton;
    }
private:
    static LinkWeaponSymbols singleton;
    LinkWeaponSymbols();
	void generateCode();
};

class EnemyWeaponSymbols : public LibrarySymbols
{
public:
    static EnemyWeaponSymbols &getInst()
    {
        return singleton;
    }
private:
    static EnemyWeaponSymbols singleton;
    EnemyWeaponSymbols();
	void generateCode();
};


class AudioSymbols : public LibrarySymbols
{
public:
    static AudioSymbols &getInst()
    {
        return singleton;
    }
private:
    static AudioSymbols singleton;
    AudioSymbols();
	void generateCode();
};

class DebugSymbols : public LibrarySymbols
{
public:
    static DebugSymbols &getInst()
    {
        return singleton;
    }
private:
    static DebugSymbols singleton;
    DebugSymbols();
	void generateCode();
};

class NPCDataSymbols : public LibrarySymbols
{
public:
    static NPCDataSymbols &getInst()
    {
        return singleton;
    }
private:
    static NPCDataSymbols singleton;
    NPCDataSymbols();
	void generateCode();
};

#endif


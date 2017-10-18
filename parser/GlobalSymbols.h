#ifndef GLOBALSYMBOLS_H
#define GLOBALSYMBOLS_H

#include "DataStructs.h"
#include <string>
#include <map>
#include <vector>

using std::string;
using std::map;
using std::vector;

class Scope;
class SymbolTable;

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
    virtual void addSymbolsToScope(Scope *scope, SymbolTable *t);
    virtual map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
    virtual pair<int, vector<int> > matchFunction(string name, SymbolTable *t);
    virtual ~LibrarySymbols();
protected:
    AccessorTable *table;
    LibrarySymbols() {}
    int firstid;
    int refVar;
    map<string, int> memberids;
    virtual vector<Opcode *> getVariable(LinkTable &lt, int id, int var);
    virtual vector<Opcode *> setVariable(LinkTable &lt, int id, int var);
    virtual vector<Opcode *> setBoolVariable(LinkTable &lt, int id, int var);
    virtual vector<Opcode *> getIndexedVariable(LinkTable &lt, int id, int var);
    virtual vector<Opcode *> setIndexedVariable(LinkTable &lt, int id, int var);
    
};

class GlobalSymbols : public LibrarySymbols
{
public:
    static GlobalSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static GlobalSymbols singleton;
    GlobalSymbols();
};

class FFCSymbols : public LibrarySymbols
{
public:
    static FFCSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static FFCSymbols singleton;
    FFCSymbols();
};

class LinkSymbols : public LibrarySymbols
{
public:
    static LinkSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static LinkSymbols singleton;
    LinkSymbols();
};

class ScreenSymbols : public LibrarySymbols
{
public:
    static ScreenSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static ScreenSymbols singleton;
    ScreenSymbols();
};

class ItemSymbols : public LibrarySymbols
{
public:
    static ItemSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static ItemSymbols singleton;
    ItemSymbols();
};

class ItemclassSymbols : public LibrarySymbols
{
public:
    static ItemclassSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static ItemclassSymbols singleton;
    ItemclassSymbols();
};

class GameSymbols : public LibrarySymbols
{
public:
    static GameSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
    static GameSymbols singleton;
    GameSymbols();
};

class NPCSymbols : public LibrarySymbols
{
public:
    static NPCSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static NPCSymbols singleton;
    NPCSymbols();
};

class LinkWeaponSymbols : public LibrarySymbols
{
public:
    static LinkWeaponSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static LinkWeaponSymbols singleton;
    LinkWeaponSymbols();
};

class EnemyWeaponSymbols : public LibrarySymbols
{
public:
    static EnemyWeaponSymbols &getInst()
    {
        return singleton;
    }
    map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
protected:
private:
    static EnemyWeaponSymbols singleton;
    EnemyWeaponSymbols();
};


#endif


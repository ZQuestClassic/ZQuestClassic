#ifndef GLOBALSYMBOLS_H
#define GLOBALSYMBOLS_H

class Scope;
class SymbolTable;

const static int SETTER = 0;
const static int GETTER = 1;
const static int FUNCTION = 2;

struct AccessorTable
{
	string name;
	int rettype;
	int setorget;
	int var;
	int numindex;
	int params[5];
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
	static GlobalSymbols &getInst() {return singleton;}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
	static GlobalSymbols singleton;
	GlobalSymbols();
};

class FFCSymbols : public LibrarySymbols
{
public:
	static FFCSymbols &getInst() {return singleton;}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
	static FFCSymbols singleton;
	FFCSymbols();
};

class LinkSymbols : public LibrarySymbols
{
public:
	static LinkSymbols &getInst() {return singleton;}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
	static LinkSymbols singleton;
	LinkSymbols();
};

class ScreenSymbols : public LibrarySymbols
{
public:
	static ScreenSymbols &getInst() {return singleton;}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
	static ScreenSymbols singleton;
	ScreenSymbols();
};

class ItemSymbols : public LibrarySymbols
{
public:
	static ItemSymbols &getInst() {return singleton;}
protected:
private:
	static ItemSymbols singleton;
	ItemSymbols();
};

class ItemclassSymbols : public LibrarySymbols
{
public:
	static ItemclassSymbols &getInst() {return singleton;}
protected:
private:
	static ItemclassSymbols singleton;
	ItemclassSymbols();
};

class GameSymbols : public LibrarySymbols
{
public:
	static GameSymbols &getInst() {return singleton;}
	map<int, vector<Opcode *> > addSymbolsCode(LinkTable &lt);
private:
	static GameSymbols singleton;
	GameSymbols();
};


#endif

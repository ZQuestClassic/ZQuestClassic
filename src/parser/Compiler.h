#ifndef COMPILER_H
#define COMPILER_H

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include <map>
#include <vector>
#include <string>
#include <stdio.h>

using std::string;
using std::vector;
using std::pair;

class ArgumentVisitor;

class Opcode
{
public:
    Opcode() : label(-1) {}
    virtual ~Opcode() {}
    virtual string toString()=0;
    int getLabel()
    {
        return label;
    }
    void setLabel(int l)
    {
        label=l;
    }
    string printLine(bool showlabel = false)
    {
        char buf[100];
        
        if(label == -1)
            return " " + toString() + "\n";
            
        sprintf(buf, "l%d:", label);
        return (showlabel ? string(buf) : " ")+ toString() + "\n";
    }
    Opcode * makeClone()
    {
        Opcode *dup = clone();
        dup->setLabel(label);
        return dup;
    }
    virtual void execute(ArgumentVisitor &host, void *param)
    {
        void *temp;
        temp=&host;
        param=param; /*these are here to bypass compiler warnings about unused arguments*/
    }
protected:
    virtual Opcode *clone()=0;
private:
    int label;
};

enum ScriptType {SCRIPTTYPE_VOID, SCRIPTTYPE_GLOBAL, SCRIPTTYPE_FFC, SCRIPTTYPE_ITEM};

struct ScriptsData
{
    std::map<string, vector<Opcode *> > theScripts;
    std::map<string, ScriptType> scriptTypes;
};

ScriptsData *compile(const char *filename);

typedef int ZVarTypeId;

enum ZVarTypeIdSimple
{
    ZVARTYPEID_VOID, ZVARTYPEID_FLOAT, ZVARTYPEID_BOOL,
    ZVARTYPEID_FFC, ZVARTYPEID_ITEM, ZVARTYPEID_ITEMCLASS, ZVARTYPEID_NPC, ZVARTYPEID_LWPN, ZVARTYPEID_EWPN,
    ZVARTYPEID_GAME, ZVARTYPEID_LINK, ZVARTYPEID_SCREEN,
	ZVARTYPEID_END
};

class ZVarTypeSimple;

class ZVarType
{
public:
	virtual ZVarType* clone() const = 0;
	virtual string getName() const = 0;
	virtual int compare(ZVarType const& other) const
	{
		int c = classCompareId() - other.classCompareId();
		if (c) return c;
		return selfCompare(other);
	}
	bool operator==(ZVarType const& other) const {return compare(other) == 0;}
	bool operator<(ZVarType const& other) const {return compare(other) < 0;}

	struct PointerLess : public std::less<ZVarType*> {
		bool operator() (ZVarType* const& a, ZVarType* const& b) const {return *a < *b;}
	};
protected:
	virtual int classCompareId() const = 0;
	virtual int selfCompare(ZVarType const& other) const = 0;

// Standard Types.
public:
	static ZVarTypeSimple const VOID;
	static ZVarTypeSimple const FLOAT;
	static ZVarTypeSimple const BOOL;
	static ZVarTypeSimple const FFC;
	static ZVarTypeSimple const ITEM;
	static ZVarTypeSimple const ITEMCLASS;
	static ZVarTypeSimple const NPC;
	static ZVarTypeSimple const LWPN;
	static ZVarTypeSimple const EWPN;
	static ZVarTypeSimple const GAME;
	static ZVarTypeSimple const LINK;
	static ZVarTypeSimple const SCREEN;
	static ZVarType const* get(ZVarTypeId id);
};

class ZVarTypeSimple : public ZVarType
{
public:
	ZVarTypeSimple(ZVarTypeIdSimple simpleId, string const& name) : simpleId(simpleId), name(name) {}
	ZVarTypeSimple* clone() const {return new ZVarTypeSimple(*this);}
	string getName() const {return name;}
	ZVarTypeIdSimple getId() const {return simpleId;}
protected:
	int classCompareId() const {return 0;}
	int selfCompare(ZVarType const& other) const {return simpleId - ((ZVarTypeSimple const&)(other)).simpleId;}
private:
	ZVarTypeIdSimple simpleId;
	string name;
};

struct SymbolData;
struct FunctionData;
struct IntermediateData;

class AST;

class ScriptParser
{
public:
    static int getUniqueVarID()
    {
        return vid++;
    }
    static int getUniqueFuncID()
    {
        return fid++;
    }
    static int getUniqueLabelID()
    {
        return lid++;
    }
    static int getUniqueGlobalID()
    {
        return gid++;
    }
    static bool preprocess(AST *theAST, int reclevel, std::map<string,long> *constants);
    static SymbolData *buildSymbolTable(AST *theAST, std::map<string, long> *constants);
    static FunctionData *typeCheck(SymbolData *sdata);
    static IntermediateData *generateOCode(FunctionData *fdata);
    static ScriptsData *assemble(IntermediateData *id);
    static void resetState()
    {
        vid=0;
        fid=0;
        gid=1;
        lid=0;
    }
    static pair<long,bool> parseLong(pair<string,string> parts);
	static int getThisType(ScriptType type)
	{
		switch (type)
		{
		case SCRIPTTYPE_FFC:
			return ZVARTYPEID_FFC;
		case SCRIPTTYPE_ITEM:
			return ZVARTYPEID_ITEMCLASS;
		case SCRIPTTYPE_GLOBAL:
		case SCRIPTTYPE_VOID:
			return ZVARTYPEID_VOID;
		}
	}
    static string printType(ZVarTypeId type)
    {
        switch(type)
        {
        case ZVARTYPEID_FLOAT:
            return "float";
            
        case ZVARTYPEID_BOOL:
            return "bool";
            
        case ZVARTYPEID_VOID:
            return "void";
            
        case ZVARTYPEID_FFC:
            return "ffc";
            
        case ZVARTYPEID_LINK:
            return "link";
            
        case ZVARTYPEID_SCREEN:
            return "screen";
            
        case ZVARTYPEID_ITEM:
            return "item";
            
        case ZVARTYPEID_ITEMCLASS:
            return "itemdata";
            
        case ZVARTYPEID_GAME:
            return "game";
            
        case ZVARTYPEID_NPC:
            return "npc";
            
        case ZVARTYPEID_LWPN:
            return "lweapon";
            
        case ZVARTYPEID_EWPN:
            return "eweapon";
            
        default:
            return "wtf";
        }
    }
private:
    static string trimQuotes(string quoteds);
    static vector<Opcode *> assembleOne(vector<Opcode *> script, std::map<int, vector<Opcode *> > &otherfuncs, int numparams);
    static int vid;
    static int fid;
    static int gid;
    static int lid;
};


#endif


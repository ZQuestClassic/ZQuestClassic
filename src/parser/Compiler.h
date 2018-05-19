#ifndef COMPILER_H //2.53 Updated to 16th Jan, 2017
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
    ZVARTYPEID_VOID,
    ZVARTYPEID_FLOAT,
    ZVARTYPEID_BOOL,
    ZVARTYPEID_FFC,
    ZVARTYPEID_ITEM,
    ZVARTYPEID_ITEMCLASS,
    ZVARTYPEID_NPC,
    ZVARTYPEID_LWPN,
    ZVARTYPEID_EWPN,
    ZVARTYPEID_GAME,
    ZVARTYPEID_LINK,
    ZVARTYPEID_SCREEN,
    ZVARTYPEID_NPCDATA,
    ZVARTYPEID_DEBUG,
    ZVARTYPEID_AUDIO,
    ZVARTYPEID_COMBOS,
    ZVARTYPEID_SPRITEDATA,
    ZVARTYPEID_GRAPHICS,
    ZVARTYPEID_TEXT,
    ZVARTYPEID_INPUT,
    ZVARTYPEID_MAPDATA,
    ZVARTYPEID_DMAPDATA,
    ZVARTYPEID_ZMESSAGE,
    ZVARTYPEID_SHOPDATA,
    ZVARTYPEID_UNTYPED,
    ZVARTYPEID_DROPSET,
    ZVARTYPEID_PONDS,
    ZVARTYPEID_WARPRING,
    ZVARTYPEID_DOORSET,
    ZVARTYPEID_ZUICOLOURS,
    ZVARTYPEID_RGBDATA,
    ZVARTYPEID_PALETTE,
    ZVARTYPEID_TUNES,
    ZVARTYPEID_PALCYCLE,
    ZVARTYPEID_GAMEDATA,
	ZVARTYPEID_CHEATS,
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
	static ZVarTypeSimple const NPCDATA;
	static ZVarTypeSimple const DEBUG;
	static ZVarTypeSimple const AUDIO;
	static ZVarTypeSimple const COMBOS;
	static ZVarTypeSimple const SPRITEDATA;
	static ZVarTypeSimple const GRAPHICS;
	static ZVarTypeSimple const TEXT;
	static ZVarTypeSimple const INPUT;
	static ZVarTypeSimple const MAPDATA;
	static ZVarTypeSimple const DMAPDATA;
	static ZVarTypeSimple const ZMESSAGE;
	static ZVarTypeSimple const SHOPDATA;
	static ZVarTypeSimple const UNTYPED;
	static ZVarTypeSimple const DROPSET;
	static ZVarTypeSimple const PONDS;
	static ZVarTypeSimple const WARPRING;
	static ZVarTypeSimple const DOORSET;
	static ZVarTypeSimple const ZUICOLOURS;
	static ZVarTypeSimple const RGBDATA;
	static ZVarTypeSimple const PALETTE;
	static ZVarTypeSimple const TUNES;
	static ZVarTypeSimple const PALCYCLE;
	static ZVarTypeSimple const GAMEDATA;
	static ZVarTypeSimple const CHEATS;
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
        switch (type)
        {
        case ZVARTYPEID_FLOAT: return "float";
        case ZVARTYPEID_BOOL: return "bool";
        case ZVARTYPEID_VOID: return "void";
        case ZVARTYPEID_FFC: return "ffc";
        case ZVARTYPEID_LINK: return "link";
        case ZVARTYPEID_SCREEN: return "screen";
        case ZVARTYPEID_ITEM: return "item";
        case ZVARTYPEID_ITEMCLASS: return "itemdata";
        case ZVARTYPEID_GAME: return "game";
        case ZVARTYPEID_NPC: return "npc";
        case ZVARTYPEID_LWPN: return "lweapon";
        case ZVARTYPEID_EWPN: return "eweapon";
        case ZVARTYPEID_NPCDATA: return "NPCData";
        case ZVARTYPEID_DEBUG: return "Debug";
        case ZVARTYPEID_AUDIO: return "Audio";
        case ZVARTYPEID_COMBOS: return "ComboData";
        case ZVARTYPEID_SPRITEDATA: return "SpriteData";
        case ZVARTYPEID_GRAPHICS: return "Graphics";
        case ZVARTYPEID_TEXT: return "Text->";
        case ZVARTYPEID_INPUT: return "Input->";
        case ZVARTYPEID_MAPDATA: return "MapData->";
        case ZVARTYPEID_DMAPDATA: return "DMapData->";
        case ZVARTYPEID_ZMESSAGE: return "MessageData->";
        case ZVARTYPEID_SHOPDATA: return "ShopData->";
        case ZVARTYPEID_UNTYPED: return "Untyped->";
        case ZVARTYPEID_DROPSET: return "dropdata->";
        case ZVARTYPEID_PONDS: return "ponddata->";
        case ZVARTYPEID_WARPRING: return "warpring->";
        case ZVARTYPEID_DOORSET: return "doorset->";
        case ZVARTYPEID_ZUICOLOURS: return "misccolors->";
        case ZVARTYPEID_RGBDATA: return "rgbdata->";
        case ZVARTYPEID_PALETTE: return "palette->";
        case ZVARTYPEID_TUNES: return "musictrack->";
        case ZVARTYPEID_PALCYCLE: return "palcycle->";
        case ZVARTYPEID_GAMEDATA: return "gamedata->";
        case ZVARTYPEID_CHEATS: return "cheats->";
        default: return "wtf";
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


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

typedef int ZVarTypeId;


struct ScriptsData
{
    std::map<string, vector<Opcode *> > theScripts;
    std::map<string, ScriptType> scriptTypes;
};

ScriptsData *compile(const char *filename);


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
    static const int TYPE_FLOAT = 0;
    static const int TYPE_BOOL = 1;
    static const int TYPE_VOID = 2;
    static const int TYPE_FFC = 3;
    static const int TYPE_LINK = 4;
    static const int TYPE_SCREEN = 5;
    static const int TYPE_GLOBAL = 6;
    static const int TYPE_ITEM = 7;
    static const int TYPE_ITEMCLASS = 8;
    static const int TYPE_GAME = 9;
    static const int TYPE_NPC = 10;
    static const int TYPE_LWPN = 11;
    static const int TYPE_EWPN = 12;
    
    //New Types
    static const int TYPE_NPCDATA = 13;
    static const int TYPE_DEBUG = 14;
    static const int TYPE_AUDIO = 15;
    static const int TYPE_COMBOS = 16;
    static const int TYPE_SPRITEDATA = 17;
    static const int TYPE_GRAPHICS = 18;
    static const int TYPE_TEXT = 19;
    static const int TYPE_INPUT = 20;
    static const int TYPE_MAPDATA = 21;
    static const int TYPE_DMAPDATA = 22;
    static const int TYPE_ZMESSAGE = 23;
    static const int TYPE_SHOPDATA = 24;
    static const int TYPE_UNTYPED = 25;
    
    static const int TYPE_DROPSET = 26;
    static const int TYPE_PONDS = 27;
    static const int TYPE_WARPRING = 28;
    static const int TYPE_DOORSET = 29;
    static const int TYPE_ZUICOLOURS = 30;
    static const int TYPE_RGBDATA = 31;
    static const int TYPE_PALETTE = 32;
    static const int TYPE_TUNES = 33;
    static const int TYPE_PALCYCLE = 34;
    static const int TYPE_GAMEDATA = 35;
    static const int TYPE_CHEATS = 36;
    
    
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
						return TYPE_FFC;
				case SCRIPTTYPE_ITEM:
						return TYPE_ITEMCLASS;
				case SCRIPTTYPE_GLOBAL:
				case SCRIPTTYPE_VOID:
						return TYPE_VOID;
				}
		}
    static string printType(ZVarTypeId type)
    {
        switch (type)
        {
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL: return "bool";
        case TYPE_VOID: return "void";
        case TYPE_FFC: return "ffc";
        case TYPE_LINK: return "link";
        case TYPE_SCREEN: return "screen";
        case TYPE_GLOBAL: return "global";
        case TYPE_ITEM: return "item";
        case TYPE_ITEMCLASS: return "itemdata";
        case TYPE_GAME: return "game";
        case TYPE_NPC: return "npc";
        case TYPE_LWPN: return "lweapon";
        case TYPE_EWPN: return "eweapon";
        case TYPE_NPCDATA: return "NPCData";
        case TYPE_DEBUG: return "Debug";
        case TYPE_AUDIO: return "Audio";
        case TYPE_COMBOS: return "ComboData";
        case TYPE_SPRITEDATA: return "SpriteData";
        case TYPE_GRAPHICS: return "Graphics";
        case TYPE_TEXT: return "Text->";
        case TYPE_INPUT: return "Input->";
        case TYPE_MAPDATA: return "MapData->";
        case TYPE_DMAPDATA: return "DMapData->";
        case TYPE_ZMESSAGE: return "MessageData->";
        case TYPE_SHOPDATA: return "ShopData->";
        case TYPE_UNTYPED: return "Untyped->";
        case TYPE_DROPSET: return "dropdata->";
        case TYPE_PONDS: return "ponddata->";
        case TYPE_WARPRING: return "warpring->";
        case TYPE_DOORSET: return "doorset->";
        case TYPE_ZUICOLOURS: return "misccolors->";
        case TYPE_RGBDATA: return "rgbdata->";
        case TYPE_PALETTE: return "palette->";
        case TYPE_TUNES: return "musictrack->";
        case TYPE_PALCYCLE: return "palcycle->";
        case TYPE_GAMEDATA: return "gamedata->";
        case TYPE_CHEATS: return "cheats->";
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


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
	int getLabel() {return label;}
	void setLabel(int l) {label=l;}
	string printLine(bool showlabel = false)
	{
		char buf[100];
		if(label == -1)
			return " " + toString() + "\n";
		sprintf(buf, "l%d:", label);
		return (showlabel ? string(buf) : " ")+ toString() + "\n";
	}
	Opcode * makeClone() {
		Opcode *dup = clone();
		dup->setLabel(label);
		return dup;
	}
	virtual void execute(ArgumentVisitor &host, void *param) { void *temp; temp=&host; param=param; /*these are here to bypass compiler warnings about unused arguments*/ }
protected:
	virtual Opcode *clone()=0;
private:
	int label;
};

struct ScriptsData
{
	std::map<string, vector<Opcode *> > theScripts;
	std::map<string, int> scriptTypes;
};

ScriptsData *compile(const char *filename);


struct SymbolData;
struct FunctionData;
struct IntermediateData;

class AST;

class ScriptParser
{
public:
	static int getUniqueVarID() {return vid++;}
	static int getUniqueFuncID() {return fid++;}
	static int getUniqueLabelID() {return lid++;}
	static int getUniqueGlobalID() {return gid++;}
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
	static bool preprocess(AST *theAST, int reclevel, std::map<string,long> *constants);
	static SymbolData *buildSymbolTable(AST *theAST, std::map<string, long> *constants);
	static FunctionData *typeCheck(SymbolData *sdata);
	static IntermediateData *generateOCode(FunctionData *fdata);
	static ScriptsData *assemble(IntermediateData *id);
	static void resetState() {vid=0;fid=0;gid=1;lid=0;}
	static pair<long,bool> parseLong(pair<string,string> parts);
	static string printType(int type) {
		switch(type)
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
 

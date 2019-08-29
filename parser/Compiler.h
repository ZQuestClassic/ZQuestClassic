#ifndef COMPILER_H
#define COMPILER_H

#ifndef __GTHREAD_HIDE_WIN32API                             
#define __GTHREAD_HIDE_WIN32API 
#endif                            //prevent indirectly including windows.h

#include <map>
#include <vector>
#include <string>

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
	virtual void execute(ArgumentVisitor &host, void *param) {}
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

ScriptsData *compile(char *filename);


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
	const static int TYPE_FLOAT = 0;
	const static int TYPE_BOOL = 1;
	const static int TYPE_VOID = 2;
	const static int TYPE_FFC = 3;
	const static int TYPE_LINK = 4;
	const static int TYPE_SCREEN = 5;
	const static int TYPE_GLOBAL = 6;
	const static int TYPE_ITEM = 7;
	const static int TYPE_ITEMCLASS = 8;
	const static int TYPE_GAME = 9;
	static bool preprocess(AST *theAST, int reclevel);
	static SymbolData *buildSymbolTable(AST *theAST);
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
		case TYPE_ITEMCLASS: return "itemclass";
		case TYPE_GAME: return "game";
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

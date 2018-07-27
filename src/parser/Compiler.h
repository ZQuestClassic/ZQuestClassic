#ifndef COMPILER_H //2.53 Updated to 16th Jan, 2017
#define COMPILER_H

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include <map>
#include <vector>
#include <string>
#include <stdio.h>

#include "CompilerUtils.h"
#include "Types.h"

using std::string;
using std::vector;
using std::pair;

class ArgumentVisitor;

namespace ZScript
{
	class Program;
}

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
    virtual void execute(ArgumentVisitor&, void*) {}
protected:
    virtual Opcode *clone()=0;
private:
    int label;
};

class ScriptsData
{
public:
	ScriptsData(ZScript::Program&);
    std::map<string, vector<Opcode *> > theScripts;
	std::map<string, ZScript::ScriptType> scriptTypes;
};

ScriptsData *compile(const char *filename);

namespace ZScript
{
	class Program;
}

struct SymbolData;
struct FunctionData;
struct IntermediateData;

class ASTProgram;

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
    static bool preprocess(ASTProgram* theAST, int reclevel);
    static SymbolData* buildSymbolTable(ASTProgram* theAST);
    static IntermediateData* generateOCode(FunctionData& fdata);
    static void assemble(IntermediateData* id);
    static void resetState()
    {
        vid=0;
        fid=0;
        gid=1;
        lid=0;
    }
    static pair<long,bool> parseLong(pair<string,string> parts);
private:
    static string prepareFilename(string const& filename);
    static vector<Opcode *> assembleOne(vector<Opcode *> script, std::map<int, vector<Opcode *> > &otherfuncs, int numparams);
    static int vid;
    static int fid;
    static int gid;
    static int lid;
};


#endif


#ifndef COMPILER_H
#define COMPILER_H

#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <stdio.h>

#include "CompilerUtils.h"
#include "Opcode.h"
#include "Types.h"

using std::string;
using std::vector;
using std::pair;

class ArgumentVisitor;

namespace ZScript
{
	class Program;
}

class ScriptsData
{
public:
	ScriptsData(ZScript::Program&);
	std::map<string, vector<ZScript::Opcode> > theScripts;
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
	static vector<ZScript::Opcode> assembleOne(
			ZScript::Program& program,
			vector<ZScript::Opcode> const& script, int numparams);
    static int vid;
    static int fid;
    static int gid;
    static int lid;
};


#endif


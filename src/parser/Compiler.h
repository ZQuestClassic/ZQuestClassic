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

	std::string printLine(bool showlabel = false)
    {
	    std::ostringstream line;
	    if (label != -1 && showlabel)
		    line << "l" << label << ": ";
	    else
		    line << " ";
	    line << toString();
	    if (!comment.empty())
		    line << " ; " << comment;
	    line << "\n";
	    return line.str();
    }
	
	Opcode * makeClone()
    {
        Opcode *dup = clone();
        dup->setLabel(label);
        dup->setComment(comment);
        return dup;
    }
	virtual void execute(ArgumentVisitor&, void*) {}

	std::string getComment() const {return comment;}
	void setComment(std::string const& c) {comment = c;}
	void appendComment(std::string const& c)
	{
		comment += " ";
		comment += c;
	}
	                                   
protected:
    virtual Opcode *clone()=0;
private:
    int label;
	std::string comment;
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
    static vector<Opcode *> assembleOne(ZScript::Program& program, vector<Opcode*> script, int numparams);
    static int vid;
    static int fid;
    static int gid;
    static int lid;
};


#endif


//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first//2.53 Updated to 16th Jan, 2017

#include "GlobalSymbols.h"
#include "ByteCode.h"
#include "../zsyssimple.h"
#include <assert.h>
#define MAXDMAPS         512                                 //this and
#define MAXLEVELS        512	

const int radsperdeg = 572958;


//sanity underflow
#define typeVOID ScriptParser::TYPE_VOID
#define S ScriptParser::TYPE_SCREEN
#define F ScriptParser::TYPE_FLOAT

#define ARGS_4(t, arg1, arg2, arg3, arg4) \
	{ t, arg1, arg2, arg3, arg4, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
#define ARGS_6(t, arg1, arg2, arg3, arg4, arg5, arg6) \
	{ t, arg1, arg2, arg3, arg4, arg5, arg6, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
#define ARGS_8(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
	{ t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }

#define POP_ARGS(num_args, t) \
	for(int _i(0); _i < num_args; ++_i) \
		code.push_back(new OPopRegister(new VarArgument(t)))



void LibrarySymbols::addSymbolsToScope(Scope *scope, SymbolTable *t)
{
    //waste an ID, OH WELL
    memberids.clear();
    firstid = ScriptParser::getUniqueFuncID()+1;
    int id = firstid;
    
    for(int i=0; table[i].name != ""; i++,id++)
    {
        vector<int> param;
        
        for(int k=0; table[i].params[k] != -1 && k<20; k++)
            param.push_back(table[i].params[k]);
            
        string name = table[i].name;
        scope->getFuncSymbols().addFunction(name, table[i].rettype,param);
        t->putFunc(id, table[i].rettype);
        t->putFuncDecl(id, param);
        memberids[name]=id;
    }
}

pair<int, vector<int> > LibrarySymbols::matchFunction(string name, SymbolTable *t)
{
    pair<int,vector<int> > rval;
    int id = -1;
    map<string, int>::iterator it = memberids.find(name);
    
    if(it != memberids.end())
        id = it->second;
        
    if(id == -1)
    {
        rval.first = -1;
        return rval;
    }
    
    //else we're good
    rval.first = id;
    rval.second = t->getFuncParams(id);
    return rval;
}

map<int, vector<Opcode *> > LibrarySymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval;
    
    for(int i=0; table[i].name != ""; i++)
    {
        int var = table[i].var;
        string name = table[i].name;
        bool isIndexed = table[i].numindex > 1;
        int id = memberids[name];
        int label = lt.functionToLabel(id);
        
        switch(table[i].setorget)
        {
        case GETTER:
            if(isIndexed)
                rval[label] = getIndexedVariable(lt, id, var);
            else
                rval[label] = getVariable(lt, id, var);
                
            break;
            
        case SETTER:
        {
            if(isIndexed)
            {
                rval[label] = setIndexedVariable(lt, id, var);
            }
            else
            {
                if(table[i].params[1] == ScriptParser::TYPE_BOOL)
                {
                    rval[label] = setBoolVariable(lt, id, var);
                }
                else
                {
                    rval[label] = setVariable(lt, id, var);
                }
            }
            
            break;
        }
        }
    }
    
    return rval;
}

vector<Opcode *> LibrarySymbols::getVariable(LinkTable &lt, int id, int var)
{
    int label  = lt.functionToLabel(id);
    vector<Opcode *> code;
    //pop object pointer
    Opcode *first = new OPopRegister(new VarArgument(EXP2));
    first->setLabel(label);
    code.push_back(first);
    //load object pointer into ref register
    if(refVar!=NUL) code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
    code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(var)));
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    return code;
}

vector<Opcode *> LibrarySymbols::getIndexedVariable(LinkTable &lt, int id, int var)
{
    int label  = lt.functionToLabel(id);
    vector<Opcode *> code;
    //pop index
    Opcode *first = new OPopRegister(new VarArgument(INDEX));
    first->setLabel(label);
    code.push_back(first);
    //pop object pointer
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    //load object pointer into ref register
    if(refVar!=NUL) code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
    code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(var)));
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    return code;
}

vector<Opcode *> LibrarySymbols::setVariable(LinkTable &lt, int id, int var)
{
    int label  = lt.functionToLabel(id);
    vector<Opcode *> code;
    //pop off the value to set to
    Opcode *first = new OPopRegister(new VarArgument(EXP1));
    first->setLabel(label);
    code.push_back(first);
    //pop object pointer
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    //load object pointer into ref register
    if(refVar!=NUL) code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
    code.push_back(new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    return code;
}

vector<Opcode *> LibrarySymbols::setBoolVariable(LinkTable &lt, int id, int var)
{
    int label  = lt.functionToLabel(id);
    vector<Opcode *> code;
    //pop off the value to set to
    Opcode *first = new OPopRegister(new VarArgument(EXP1));
    first->setLabel(label);
    code.push_back(first);
    //renormalize true to 1
    int donerenorm = ScriptParser::getUniqueLabelID();
    code.push_back(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    code.push_back(new OGotoTrueImmediate(new LabelArgument(donerenorm)));
    code.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
    
    //pop object pointer
    Opcode *next = new OPopRegister(new VarArgument(EXP2));
    next->setLabel(donerenorm);
    code.push_back(next);
    //load object pointer into ref register
    if(refVar!=NUL) code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
    code.push_back(new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    return code;
}

vector<Opcode *> LibrarySymbols::setIndexedVariable(LinkTable &lt, int id, int var)
{
    int label  = lt.functionToLabel(id);
    vector<Opcode *> code;
    //pop off index
    Opcode *first = new OPopRegister(new VarArgument(INDEX));
    first->setLabel(label);
    code.push_back(first);
    //pop off the value to set to
    code.push_back(new OPopRegister(new VarArgument(EXP1)));
    //pop object pointer
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    //load object pointer into ref register
    if(refVar!=NUL) code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
    code.push_back(new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    return code;
}

LibrarySymbols::~LibrarySymbols()
{
    return;
}

GlobalSymbols GlobalSymbols::singleton;

static AccessorTable GlobalTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "Rand",                   ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Quit",                   ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Waitframe",              ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Waitdraw",               ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Trace",                  ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TraceB",                 ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_BOOL,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TraceS",                 ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TraceNL",                ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ClearTrace",             ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TraceToBase",            ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,	    ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     -1,    				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,   					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "Sin",                    ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Cos",                    ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Tan",                    ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ArcTan",                 ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ArcSin",                 ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ArcCos",                 ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "RadianSin",              ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "RadianCos",              ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "RadianTan",              ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Max",                    ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Min",                    ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Pow",                    ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "InvPow",                 ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Factorial",              ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Abs",                    ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Log10",                  ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Ln",                     ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Sqrt",                   ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
//  { "CalculateSpline",        ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
//  { "CollisionRect",          ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
//  { "CollisionBox",           ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CopyTile",               ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SwapTile",               ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ClearTile",              ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "OverlayTile",               ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScriptRAM",           ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScriptRAM",           ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetGlobalRAM",           ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetGlobalRAM",           ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetColorBuffer",         ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,	    ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,   					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "SetDepthBuffer",         ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,	    ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,   					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "GetColorBuffer",         ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,	    ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,   					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "GetDepthBuffer",         ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,	    ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,   					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "SizeOfArray",            ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "IsValidArray",            ScriptParser::TYPE_BOOL,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

GlobalSymbols::GlobalSymbols()
{
    table = GlobalTable;
    refVar = NUL;
}

map<int, vector<Opcode *> > GlobalSymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval;
    int id;
    //int Rand(int maxval)
    {
        id = memberids["Rand"];
        int label  = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop maxval
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new ORandRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void Quit()
    {
        id = memberids["Quit"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OQuit());
        rval[label]=code;
    }
    //void Waitframe()
    {
        id = memberids["Waitframe"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OWaitframe();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Waitdraw()
    {
        id = memberids["Waitdraw"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OWaitdraw();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Trace(int val)
    {
        id = memberids["Trace"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OTraceRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void TraceB(bool val)
    {
        id = memberids["TraceB"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OTrace2Register(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void TraceS(bool val)
    {
        id = memberids["TraceS"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OTrace6Register(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void TraceNL()
    {
        id = memberids["TraceNL"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OTrace3();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void ClearTrace()
    {
        id = memberids["ClearTrace"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OTrace4();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void TraceToBase(float, float, float)
    {
        id = memberids["TraceToBase"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OTrace5Register();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Sin(int val)
    {
        id = memberids["Sin"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OSinRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int RadianSin(int val)
    {
        id = memberids["RadianSin"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OMultImmediate(new VarArgument(EXP2), new LiteralArgument(radsperdeg)));
        code.push_back(new OSinRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int ArcSin(int val)
    {
        id = memberids["ArcSin"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArcSinRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Cos(int val)
    {
        id = memberids["Cos"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OCosRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int RadianCos(int val)
    {
        id = memberids["RadianCos"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OMultImmediate(new VarArgument(EXP2), new LiteralArgument(radsperdeg)));
        code.push_back(new OCosRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int ArcCos(int val)
    {
        id = memberids["ArcCos"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArcCosRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Tan(int val)
    {
        id = memberids["Tan"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OTanRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int ArcTan(int X, int Y)
    {
        id = memberids["ArcTan"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OATanRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int RadianTan(int val)
    {
        id = memberids["RadianTan"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OMultImmediate(new VarArgument(EXP2), new LiteralArgument(radsperdeg)));
        code.push_back(new OTanRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Max(int first, int second)
    {
        id = memberids["Max"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OMaxRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Min(int first, int second)
    {
        id = memberids["Min"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OMinRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Pow(int first, int second)
    {
        id = memberids["Pow"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPowRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int InvPow(int first, int second)
    {
        id = memberids["InvPow"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OInvPowRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Factorial(int val)
    {
        id = memberids["Factorial"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OFactorial(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Abs(int val)
    {
        id = memberids["Abs"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OAbsRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Log10(int val)
    {
        id = memberids["Log10"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OLog10Register(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Ln(int val)
    {
        id = memberids["Ln"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OLogERegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int Sqrt(int val)
    {
        id = memberids["Sqrt"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OSqrtRegister(new VarArgument(EXP1), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    
    //int CopyTile(int source, int dest)
    {
        id = memberids["CopyTile"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OCopyTileRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
     //int OverlayTile(int first, int second)
    {
        id = memberids["OverlayTile"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OOverlayTileRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int SwapTile(int first, int second)
    {
        id = memberids["SwapTile"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OSwapTileRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void ClearTile(int tile)
    {
        id = memberids["ClearTile"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OClearTileRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void GetGlobalRAM(int)
    {
        int id2 = memberids["GetGlobalRAM"];
        int label = lt.functionToLabel(id2);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(GLOBALRAMD)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetGlobalRAM(int, int)
    {
        int id2 = memberids["SetGlobalRAM"];
        int label = lt.functionToLabel(id2);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OSetRegister(new VarArgument(GLOBALRAMD), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void GetScriptRAM(int)
    {
        int id2 = memberids["GetScriptRAM"];
        int label = lt.functionToLabel(id2);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        //code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SCRIPTRAMD)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScriptRAM(int, int)
    {
        int id2 = memberids["SetScriptRAM"];
        int label = lt.functionToLabel(id2);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OSetRegister(new VarArgument(SCRIPTRAMD), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetColorBuffer(int amount, int offset, int stride, int *ptr)
    {
        id = memberids["SetColorBuffer"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OSetColorBufferRegister();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void SetDepthBuffer(int amount, int offset, int stride, int *ptr)
    {
        id = memberids["SetDepthBuffer"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OSetDepthBufferRegister();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void GetColorBuffer(int amount, int offset, int stride, int *ptr)
    {
        id = memberids["GetColorBuffer"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OGetColorBufferRegister();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void GetDepthBuffer(int amount, int offset, int stride, int *ptr)
    {
        id = memberids["GetDepthBuffer"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OGetDepthBufferRegister();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int SizeOfArray(int val)
    {
        id = memberids["SizeOfArray"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArraySize(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //bool IsValidArray(int val)
    {
        id = memberids["IsValidArray"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
	first->setLabel(label);
	code.push_back(first);
	code.push_back(new OIsValidArray(new VarArgument(EXP1)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    return rval;
}

FFCSymbols FFCSymbols::singleton = FFCSymbols();

static AccessorTable FFCTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getData",                ScriptParser::TYPE_FLOAT,         GETTER,       DATA,                 1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setData",                ScriptParser::TYPE_VOID,          SETTER,       DATA,                 1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getScript",              ScriptParser::TYPE_FLOAT,         GETTER,       FFSCRIPT,             1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setScript",              ScriptParser::TYPE_VOID,          SETTER,       FFSCRIPT,             1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCSet",                ScriptParser::TYPE_FLOAT,         GETTER,       FCSET,                1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCSet",                ScriptParser::TYPE_VOID,          SETTER,       FCSET,                1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDelay",               ScriptParser::TYPE_FLOAT,         GETTER,       DELAY,                1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDelay",               ScriptParser::TYPE_VOID,          SETTER,       DELAY,                1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getX",                   ScriptParser::TYPE_FLOAT,         GETTER,       FX,                   1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ScriptParser::TYPE_VOID,          SETTER,       FX,                   1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ScriptParser::TYPE_FLOAT,         GETTER,       FY,                   1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ScriptParser::TYPE_VOID,          SETTER,       FY,                   1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getVx",                  ScriptParser::TYPE_FLOAT,         GETTER,       XD,                   1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setVx",                  ScriptParser::TYPE_VOID,          SETTER,       XD,                   1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getVy",                  ScriptParser::TYPE_FLOAT,         GETTER,       YD,                   1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setVy",                  ScriptParser::TYPE_VOID,          SETTER,       YD,                   1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAx",                  ScriptParser::TYPE_FLOAT,         GETTER,       XD2,                  1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAx",                  ScriptParser::TYPE_VOID,          SETTER,       XD2,                  1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAy",                  ScriptParser::TYPE_FLOAT,         GETTER,       YD2,                  1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAy",                  ScriptParser::TYPE_VOID,          SETTER,       YD2,                  1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //{ "WasTriggered",           ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlags[]",             ScriptParser::TYPE_BOOL,          GETTER,       FFFLAGSD,             11,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlags[]",             ScriptParser::TYPE_VOID,          SETTER,       FFFLAGSD,             11,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_BOOL,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ScriptParser::TYPE_FLOAT,         GETTER,       FFTWIDTH,             1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ScriptParser::TYPE_VOID,          SETTER,       FFTWIDTH,             1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       FFTHEIGHT,            1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ScriptParser::TYPE_VOID,          SETTER,       FFTHEIGHT,            1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getEffectWidth",         ScriptParser::TYPE_FLOAT,         GETTER,       FFCWIDTH,             1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEffectWidth",         ScriptParser::TYPE_VOID,          SETTER,       FFCWIDTH,             1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getEffectHeight",        ScriptParser::TYPE_FLOAT,         GETTER,       FFCHEIGHT,            1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEffectHeight",        ScriptParser::TYPE_VOID,          SETTER,       FFCHEIGHT,            1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLink",                ScriptParser::TYPE_FLOAT,         GETTER,       FFLINK,               1,      {  ScriptParser::TYPE_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLink",                ScriptParser::TYPE_VOID,          SETTER,       FFLINK,               1,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ScriptParser::TYPE_FLOAT,         GETTER,       FFMISCD,              16,     {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ScriptParser::TYPE_VOID,          SETTER,       FFMISCD,              16,     {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInitD[]",             ScriptParser::TYPE_FLOAT,         GETTER,       FFINITDD,             8,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInitD[]",             ScriptParser::TYPE_VOID,          SETTER,       FFINITDD,             8,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //{ "getD[]",                ScriptParser::TYPE_FLOAT,         GETTER,       FFDD,                 8,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //{ "setD[]",	               ScriptParser::TYPE_VOID,          SETTER,       FFDD,                 8,      {  ScriptParser::TYPE_FFC,           ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

FFCSymbols::FFCSymbols()
{
    table = FFCTable;
    refVar = REFFFC;
}

map<int, vector<Opcode *> > FFCSymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
    //bool WasTriggered(ffc)
    /*{
    	int id = memberids["WasTriggered"];
    	int label  = lt.functionToLabel(id);
    	vector<Opcode *> code;
    	//pop ffc
    	Opcode *first = new OPopRegister(new VarArgument(EXP2));
    	first->setLabel(label);
    	code.push_back(first);
    	//if ffc = -1, it is "this"
    	int thislabel = ScriptParser::getUniqueLabelID();
    	code.push_back(new OCompareImmediate(new VarArgument(EXP2), new LiteralArgument(-1)));
    	code.push_back(new OGotoTrueImmediate(new LabelArgument(thislabel)));
    	//if not this
    	//NOT POSSIBLE YET
    	//QUIT
    	code.push_back(new OQuit());
    	//if "this"
    	code.push_back(new OCheckTrig());
    	int truelabel = ScriptParser::getUniqueLabelID();
    	code.push_back(new OGotoTrueImmediate(new LabelArgument(truelabel)));
    	code.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
    	code.push_back(new OPopRegister(new VarArgument(EXP2)));
    	code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    	Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1));
    	next->setLabel(truelabel);
    	code.push_back(next);
    	code.push_back(new OPopRegister(new VarArgument(EXP2)));
    	code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    	rval[label]=code;
    }*/
    return rval;
}

LinkSymbols LinkSymbols::singleton = LinkSymbols();

static AccessorTable LinkSTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getEaten",                   ScriptParser::TYPE_FLOAT,         GETTER,       LINKEATEN,                1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEaten",                   ScriptParser::TYPE_VOID,          SETTER,       LINKEATEN,                1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getX",                   ScriptParser::TYPE_FLOAT,         GETTER,       LINKX,                1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ScriptParser::TYPE_VOID,          SETTER,       LINKX,                1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ScriptParser::TYPE_FLOAT,         GETTER,       LINKY,                1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ScriptParser::TYPE_VOID,          SETTER,       LINKY,                1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getZ",                   ScriptParser::TYPE_FLOAT,         GETTER,       LINKZ,                1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setZ",                   ScriptParser::TYPE_VOID,          SETTER,       LINKZ,                1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJump",                ScriptParser::TYPE_FLOAT,         GETTER,       LINKJUMP,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setJump",                ScriptParser::TYPE_VOID,          SETTER,       LINKJUMP,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDir",                 ScriptParser::TYPE_FLOAT,         GETTER,       LINKDIR,              1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDir",                 ScriptParser::TYPE_VOID,          SETTER,       LINKDIR,              1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitDir",              ScriptParser::TYPE_FLOAT,         GETTER,       LINKHITDIR,           1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitDir",              ScriptParser::TYPE_VOID,          SETTER,       LINKHITDIR,           1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getSwordJinx",           ScriptParser::TYPE_FLOAT,         GETTER,       LINKSWORDJINX,        1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setSwordJinx",           ScriptParser::TYPE_VOID,          SETTER,       LINKSWORDJINX,        1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getItemJinx",            ScriptParser::TYPE_FLOAT,         GETTER,       LINKITEMJINX,         1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setItemJinx",            ScriptParser::TYPE_VOID,          SETTER,       LINKITEMJINX,         1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHP",                  ScriptParser::TYPE_FLOAT,         GETTER,       LINKHP,               1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHP",                  ScriptParser::TYPE_VOID,          SETTER,       LINKHP,               1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMP",                  ScriptParser::TYPE_FLOAT,         GETTER,       LINKMP,               1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMP",                  ScriptParser::TYPE_VOID,          SETTER,       LINKMP,               1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMaxHP",               ScriptParser::TYPE_FLOAT,         GETTER,       LINKMAXHP,            1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMaxHP",               ScriptParser::TYPE_VOID,          SETTER,       LINKMAXHP,            1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMaxMP",               ScriptParser::TYPE_FLOAT,         GETTER,       LINKMAXMP,            1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMaxMP",               ScriptParser::TYPE_VOID,          SETTER,       LINKMAXMP,            1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAction",              ScriptParser::TYPE_FLOAT,         GETTER,       LINKACTION,           1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAction",              ScriptParser::TYPE_VOID,          SETTER,       LINKACTION,           1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHeldItem",            ScriptParser::TYPE_FLOAT,         GETTER,       LINKHELD,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHeldItem",            ScriptParser::TYPE_VOID,          SETTER,       LINKHELD,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Warp",                   ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PitWarp",                ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputStart",          ScriptParser::TYPE_BOOL,          GETTER,       INPUTSTART,           1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputStart",          ScriptParser::TYPE_VOID,          SETTER,       INPUTSTART,           1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputMap",            ScriptParser::TYPE_BOOL,          GETTER,       INPUTMAP,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputMap",            ScriptParser::TYPE_VOID,          SETTER,       INPUTMAP,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputUp",             ScriptParser::TYPE_BOOL,          GETTER,       INPUTUP,              1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputUp",             ScriptParser::TYPE_VOID,          SETTER,       INPUTUP,              1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputDown",           ScriptParser::TYPE_BOOL,          GETTER,       INPUTDOWN,            1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputDown",           ScriptParser::TYPE_VOID,          SETTER,       INPUTDOWN,            1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputLeft",           ScriptParser::TYPE_BOOL,          GETTER,       INPUTLEFT,            1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputLeft",           ScriptParser::TYPE_VOID,          SETTER,       INPUTLEFT,            1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputRight",          ScriptParser::TYPE_BOOL,          GETTER,       INPUTRIGHT,           1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputRight",          ScriptParser::TYPE_VOID,          SETTER,       INPUTRIGHT,           1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputA",              ScriptParser::TYPE_BOOL,          GETTER,       INPUTA,               1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputA",              ScriptParser::TYPE_VOID,          SETTER,       INPUTA,               1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputB",              ScriptParser::TYPE_BOOL,          GETTER,       INPUTB,               1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputB",              ScriptParser::TYPE_VOID,          SETTER,       INPUTB,               1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputL",              ScriptParser::TYPE_BOOL,          GETTER,       INPUTL,               1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputL",              ScriptParser::TYPE_VOID,          SETTER,       INPUTL,               1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputR",              ScriptParser::TYPE_BOOL,          GETTER,       INPUTR,               1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputR",              ScriptParser::TYPE_VOID,          SETTER,       INPUTR,               1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputEx1",            ScriptParser::TYPE_BOOL,          GETTER,       INPUTEX1,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputEx1",            ScriptParser::TYPE_VOID,          SETTER,       INPUTEX1,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputEx2",            ScriptParser::TYPE_BOOL,          GETTER,       INPUTEX2,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputEx2",            ScriptParser::TYPE_VOID,          SETTER,       INPUTEX2,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputEx3",            ScriptParser::TYPE_BOOL,          GETTER,       INPUTEX3,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputEx3",            ScriptParser::TYPE_VOID,          SETTER,       INPUTEX3,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputEx4",            ScriptParser::TYPE_BOOL,          GETTER,       INPUTEX4,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputEx4",            ScriptParser::TYPE_VOID,          SETTER,       INPUTEX4,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressStart",          ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSSTART,      1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressStart",          ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSSTART,      1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressUp",             ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSUP,         1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressUp",             ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSUP,         1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressDown",           ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSDOWN,       1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressDown",           ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSDOWN,       1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressLeft",           ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSLEFT,       1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressLeft",           ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSLEFT,       1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressRight",          ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSRIGHT,      1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressRight",          ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSRIGHT,      1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressA",              ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSA,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressA",              ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSA,          1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressB",              ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSB,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressB",              ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSB,          1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressL",              ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSL,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressL",              ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSL,          1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressR",              ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSR,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressR",              ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSR,          1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressEx1",            ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSEX1,        1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressEx1",            ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSEX1,        1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressEx2",            ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSEX2,        1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressEx2",            ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSEX2,        1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressEx3",            ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSEX3,        1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressEx3",            ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSEX3,        1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressEx4",            ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSEX4,        1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressEx4",            ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSEX4,        1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputMouseX",         ScriptParser::TYPE_FLOAT,         GETTER,       INPUTMOUSEX,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputMouseX",         ScriptParser::TYPE_VOID,          SETTER,       INPUTMOUSEX,          1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputMouseY",         ScriptParser::TYPE_FLOAT,         GETTER,       INPUTMOUSEY,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputMouseY",         ScriptParser::TYPE_VOID,          SETTER,       INPUTMOUSEY,          1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputMouseZ",         ScriptParser::TYPE_FLOAT,         GETTER,       INPUTMOUSEZ,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputMouseZ",         ScriptParser::TYPE_VOID,          SETTER,       INPUTMOUSEZ,          1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputMouseB",         ScriptParser::TYPE_FLOAT,         GETTER,       INPUTMOUSEB,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputMouseB",         ScriptParser::TYPE_VOID,          SETTER,       INPUTMOUSEB,          1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getItem[]",              ScriptParser::TYPE_BOOL,          GETTER,       LINKITEMD,            256,    {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setItem[]",              ScriptParser::TYPE_VOID,          SETTER,       LINKITEMD,            256,    {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_BOOL,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitWidth",            ScriptParser::TYPE_FLOAT,         GETTER,       LINKHXSZ,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitWidth",            ScriptParser::TYPE_VOID,          SETTER,       LINKHXSZ,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitHeight",           ScriptParser::TYPE_FLOAT,         GETTER,       LINKHYSZ,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitHeight",           ScriptParser::TYPE_VOID,          SETTER,       LINKHYSZ,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitZHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       LINKHZSZ,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitZHeight",          ScriptParser::TYPE_VOID,          SETTER,       LINKHZSZ,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ScriptParser::TYPE_FLOAT,         GETTER,       LINKTXSZ,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ScriptParser::TYPE_VOID,          SETTER,       LINKTXSZ,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       LINKTYSZ,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ScriptParser::TYPE_VOID,          SETTER,       LINKTYSZ,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawXOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       LINKXOFS,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawXOffset",         ScriptParser::TYPE_VOID,          SETTER,       LINKXOFS,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawYOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       LINKYOFS,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawYOffset",         ScriptParser::TYPE_VOID,          SETTER,       LINKYOFS,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawZOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       LINKZOFS,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawZOffset",         ScriptParser::TYPE_VOID,          SETTER,       LINKZOFS,             1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitXOffset",          ScriptParser::TYPE_FLOAT,         GETTER,       LINKHXOFS,            1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitXOffset",          ScriptParser::TYPE_VOID,          SETTER,       LINKHXOFS,            1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitYOffset",          ScriptParser::TYPE_FLOAT,         GETTER,       LINKHYOFS,            1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitYOffset",          ScriptParser::TYPE_VOID,          SETTER,       LINKHYOFS,            1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrunk",               ScriptParser::TYPE_FLOAT,         GETTER,       LINKDRUNK,            1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrunk",               ScriptParser::TYPE_VOID,          SETTER,       LINKDRUNK,            1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getEquipment",           ScriptParser::TYPE_FLOAT,         GETTER,       LINKEQUIP,            1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEquipment",           ScriptParser::TYPE_VOID,          SETTER,       LINKEQUIP,            1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputAxisUp",         ScriptParser::TYPE_BOOL,          GETTER,       INPUTAXISUP,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputAxisUp",         ScriptParser::TYPE_VOID,          SETTER,       INPUTAXISUP,          1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputAxisDown",       ScriptParser::TYPE_BOOL,          GETTER,       INPUTAXISDOWN,        1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputAxisDown",       ScriptParser::TYPE_VOID,          SETTER,       INPUTAXISDOWN,        1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputAxisLeft",       ScriptParser::TYPE_BOOL,          GETTER,       INPUTAXISLEFT,        1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputAxisLeft",       ScriptParser::TYPE_VOID,          SETTER,       INPUTAXISLEFT,        1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputAxisRight",      ScriptParser::TYPE_BOOL,          GETTER,       INPUTAXISRIGHT,       1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputAxisRight",      ScriptParser::TYPE_VOID,          SETTER,       INPUTAXISRIGHT,       1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressAxisUp",         ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSAXISUP,     1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressAxisUp",         ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSAXISUP,     1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressAxisDown",       ScriptParser::TYPE_BOOL,   	    GETTER,       INPUTPRESSAXISDOWN,   1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressAxisDown",       ScriptParser::TYPE_VOID,   	    SETTER,       INPUTPRESSAXISDOWN,   1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressAxisLeft",	      ScriptParser::TYPE_BOOL,   	    GETTER,       INPUTPRESSAXISLEFT,   1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressAxisLeft",       ScriptParser::TYPE_VOID,   	    SETTER,       INPUTPRESSAXISLEFT,   1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressAxisRight",      ScriptParser::TYPE_BOOL,   	    GETTER,       INPUTPRESSAXISRIGHT,  1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressAxisRight",      ScriptParser::TYPE_VOID,   	    SETTER,       INPUTPRESSAXISRIGHT,  1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInvisible",           ScriptParser::TYPE_BOOL,         GETTER,       LINKINVIS,            1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInvisible",           ScriptParser::TYPE_VOID,          SETTER,       LINKINVIS,            1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection",       ScriptParser::TYPE_BOOL,         GETTER,       LINKINVINC,           1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection",       ScriptParser::TYPE_VOID,          SETTER,       LINKINVINC,           1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ScriptParser::TYPE_FLOAT,         GETTER,       LINKMISCD,            16,     {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ScriptParser::TYPE_VOID,          SETTER,       LINKMISCD,            16,     {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLadderX",             ScriptParser::TYPE_FLOAT,         GETTER,       LINKLADDERX,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLadderY",             ScriptParser::TYPE_FLOAT,         GETTER,       LINKLADDERY,          1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",                ScriptParser::TYPE_FLOAT,         GETTER,       LINKTILE,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTile",                ScriptParser::TYPE_VOID,          SETTER,       LINKTILE,             1,      {  ScriptParser::TYPE_LINK,         ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlip",                ScriptParser::TYPE_FLOAT,         GETTER,       LINKFLIP,             1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlip",                ScriptParser::TYPE_VOID,          SETTER,       LINKFLIP,             1,      {  ScriptParser::TYPE_LINK,         ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressMap",            ScriptParser::TYPE_BOOL,          GETTER,       INPUTPRESSMAP,        1,      {  ScriptParser::TYPE_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressMap",            ScriptParser::TYPE_VOID,          SETTER,       INPUTPRESSMAP,        1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SelectAWeapon",          ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SelectBWeapon",          ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_LINK,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
};

LinkSymbols::LinkSymbols()
{
    table = LinkSTable;
    refVar = NUL;
}

map<int, vector<Opcode *> > LinkSymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
    //Warp(link, int, int)
    {
        int id = memberids["Warp"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop ffc, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        //ffc must be this (link is not a user-accessible type)
        code.push_back(new OWarp(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //PitWarp(link, int, int)
    {
        int id = memberids["PitWarp"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop ffc, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        //ffc must be this (link is not a user-accessible type)
        code.push_back(new OPitWarp(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //SelectAWeapon(link, int)
    {
        int id = memberids["SelectAWeapon"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSelectAWeaponRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //SelectBWeapon(link, int)
    {
        int id = memberids["SelectBWeapon"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSelectBWeaponRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    return rval;
}

ScreenSymbols ScreenSymbols::singleton = ScreenSymbols();

static AccessorTable ScreenTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getD[]",                 ScriptParser::TYPE_FLOAT,         GETTER,       SDD,                  8,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setD[]",                 ScriptParser::TYPE_VOID,          SETTER,       SDD,                  8,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboD[]",            ScriptParser::TYPE_FLOAT,         GETTER,       COMBODD,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboD[]",            ScriptParser::TYPE_VOID,          SETTER,       COMBODD,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboC[]",            ScriptParser::TYPE_FLOAT,         GETTER,       COMBOCD,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboC[]",            ScriptParser::TYPE_VOID,          SETTER,       COMBOCD,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboF[]",            ScriptParser::TYPE_FLOAT,         GETTER,       COMBOFD,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboF[]",            ScriptParser::TYPE_VOID,          SETTER,       COMBOFD,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboI[]",            ScriptParser::TYPE_FLOAT,         GETTER,       COMBOID,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboI[]",            ScriptParser::TYPE_VOID,          SETTER,       COMBOID,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboT[]",            ScriptParser::TYPE_FLOAT,         GETTER,       COMBOTD,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboT[]",            ScriptParser::TYPE_VOID,          SETTER,       COMBOTD,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboS[]",            ScriptParser::TYPE_FLOAT,         GETTER,       COMBOSD,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboS[]",            ScriptParser::TYPE_VOID,          SETTER,       COMBOSD,              176,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDoor[]",              ScriptParser::TYPE_FLOAT,         GETTER,       SCRDOORD,               4,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDoor[]",              ScriptParser::TYPE_VOID,          SETTER,       SCRDOORD,               4,    {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getState[]",             ScriptParser::TYPE_BOOL,          GETTER,       SCREENSTATED,          32,    {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setState[]",             ScriptParser::TYPE_VOID,          SETTER,       SCREENSTATED,          32,    {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_BOOL,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLit",                 ScriptParser::TYPE_BOOL,          GETTER,       LIT,                    1,    {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLit",                 ScriptParser::TYPE_VOID,          SETTER,       LIT,                    1,    {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWavy",                ScriptParser::TYPE_FLOAT,         GETTER,       WAVY,                   1,    {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setWavy",                ScriptParser::TYPE_VOID,          SETTER,       WAVY,                   1,    {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getQuake",               ScriptParser::TYPE_FLOAT,         GETTER,       QUAKE,                  1,    {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setQuake",               ScriptParser::TYPE_VOID,          SETTER,       QUAKE,                1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "NumItems",               ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCOUNT,            1,      {  ScriptParser::TYPE_SCREEN,       -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetRenderTarget",        ScriptParser::TYPE_FLOAT,         GETTER,       GETRENDERTARGET,            1,      {  ScriptParser::TYPE_SCREEN,       -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadItem",               ScriptParser::TYPE_ITEM,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CreateItem",             ScriptParser::TYPE_ITEM,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadFFC",                ScriptParser::TYPE_FFC,           FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "NumNPCs",                ScriptParser::TYPE_FLOAT,         GETTER,       NPCCOUNT,             1,      {  ScriptParser::TYPE_SCREEN,       -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadNPC",                ScriptParser::TYPE_NPC,           FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CreateNPC",              ScriptParser::TYPE_NPC,           FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ClearSprites",           ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Rectangle",              ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_BOOL,      ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                      } },
    { "Circle",                 ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_BOOL,      ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "Arc",                    ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_BOOL,      ScriptParser::TYPE_BOOL,      ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "Ellipse",                ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_BOOL,      ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "Line",                   ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "Spline",                 ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "PutPixel",               ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawCharacter",          ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawInteger",            ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawTile",               ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,      ScriptParser::TYPE_BOOL,    ScriptParser::TYPE_FLOAT,                           -1,                           -1,                           -1,                           -1,                           } },
    { "DrawCombo",              ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,      ScriptParser::TYPE_BOOL,    ScriptParser::TYPE_FLOAT,                           -1,                           -1,                           -1,                           } },
    { "Quad",                   ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,  ScriptParser::TYPE_FLOAT,                         -1,                           -1,                           -1,                           -1,                           } },
    { "Triangle",               ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,                      -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "Quad3D",                 ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "Triangle3D",             ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "FastTile",               ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "FastCombo",              ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawString",             ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawLayer",     typeVOID, FUNCTION, 0, 1, ARGS_8(S,F,F,F,F,F,F,F,F) },
    { "DrawScreen",    typeVOID, FUNCTION, 0, 1, ARGS_6(S,F,F,F,F,F,F) },
    { "DrawBitmap",             ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,       ScriptParser::TYPE_FLOAT,   ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_BOOL,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "SetRenderTarget",        ScriptParser::TYPE_VOID,		    FUNCTION,	  0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,		  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1							 } },
    { "Message",                ScriptParser::TYPE_VOID,		    FUNCTION,	   0,                   1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,		  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1							 } },
    { "NumLWeapons",            ScriptParser::TYPE_FLOAT,         GETTER,       LWPNCOUNT,            1,      {  ScriptParser::TYPE_SCREEN,       -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadLWeapon",            ScriptParser::TYPE_LWPN,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CreateLWeapon",          ScriptParser::TYPE_LWPN,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "NumEWeapons",            ScriptParser::TYPE_FLOAT,         GETTER,       EWPNCOUNT,            1,      {  ScriptParser::TYPE_SCREEN,       -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadEWeapon",            ScriptParser::TYPE_EWPN,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CreateEWeapon",          ScriptParser::TYPE_EWPN,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "isSolid",                ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        ScriptParser::TYPE_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetSideWarp",            ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,     -1,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "SetTileWarp",            ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,		 ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,     -1,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "LayerScreen",            ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LayerMap",               ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlags[]",             ScriptParser::TYPE_FLOAT,         GETTER,       SCREENFLAGSD,        10,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlags[]",             ScriptParser::TYPE_VOID,          SETTER,       SCREENFLAGSD,        10,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getEFlags[]",            ScriptParser::TYPE_FLOAT,         GETTER,       SCREENEFLAGSD,        3,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEFlags[]",            ScriptParser::TYPE_VOID,          SETTER,       SCREENEFLAGSD,        3,      {  ScriptParser::TYPE_SCREEN,        ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TriggerSecrets",         ScriptParser::TYPE_VOID,          FUNCTION,     0,       		        1,      {  ScriptParser::TYPE_SCREEN,        -1,    					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getRoomType",            ScriptParser::TYPE_FLOAT,         GETTER,       ROOMTYPE,             1,      {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setRoomType",            ScriptParser::TYPE_VOID,          SETTER,       ROOMTYPE,           1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getRoomData",            ScriptParser::TYPE_FLOAT,         GETTER,       ROOMDATA,             1,      {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setRoomData",            ScriptParser::TYPE_VOID,          SETTER,       ROOMDATA,             1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMovingBlockX",        ScriptParser::TYPE_FLOAT,         GETTER,       PUSHBLOCKX,           1,      {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMovingBlockX",        ScriptParser::TYPE_VOID,          SETTER,       PUSHBLOCKX,           1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMovingBlockY",        ScriptParser::TYPE_FLOAT,         GETTER,       PUSHBLOCKY,           1,      {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMovingBlockY",        ScriptParser::TYPE_VOID,          SETTER,       PUSHBLOCKY,           1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMovingBlockCombo",    ScriptParser::TYPE_FLOAT,         GETTER,       PUSHBLOCKCOMBO,       1,      {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMovingBlockCombo",    ScriptParser::TYPE_VOID,          SETTER,       PUSHBLOCKCOMBO,       1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMovingBlockCSet",     ScriptParser::TYPE_FLOAT,         GETTER,       PUSHBLOCKCSET,        1,      {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMovingBlockCSet",     ScriptParser::TYPE_VOID,          SETTER,       PUSHBLOCKCSET,        1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getUnderCombo",          ScriptParser::TYPE_FLOAT,         GETTER,       UNDERCOMBO,           1,      {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setUnderCombo",          ScriptParser::TYPE_VOID,          SETTER,       UNDERCOMBO,           1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getUnderCSet",           ScriptParser::TYPE_FLOAT,         GETTER,       UNDERCSET,            1,      {  ScriptParser::TYPE_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setUnderCSet",           ScriptParser::TYPE_VOID,          SETTER,       UNDERCSET,            1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetSideWarpDMap",        ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetSideWarpScreen",      ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetSideWarpType",        ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetTileWarpDMap",        ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetTileWarpScreen",      ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetTileWarpType",        ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_SCREEN,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                                -1,                              -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};


ScreenSymbols::ScreenSymbols()
{
    table = ScreenTable;
    refVar = NUL;
}

map<int, vector<Opcode *> > ScreenSymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
    //item LoadItem(screen, int)
    {
        int id = memberids["LoadItem"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        //convert from 1-index to 0-index
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OLoadItemRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFITEM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //item CreateItem(screen, int)
    {
        int id = memberids["CreateItem"];
        
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OCreateItemRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFITEM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //ffc LoadFFC(screen, int)
    {
        int id = memberids["LoadFFC"];
        
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        //code.push_back(new OSetRegister(new VarArgument(REFFFC), new VarArgument(EXP1)));
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //npc LoadNPC(screen, int)
    {
        int id = memberids["LoadNPC"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        //convert from 1-index to 0-index
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OLoadNPCRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //npc CreateNPC(screen, int)
    {
        int id = memberids["CreateNPC"];
        
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OCreateNPCRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //npc LoadLWeapon(screen, int)
    {
        int id = memberids["LoadLWeapon"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        //convert from 1-index to 0-index
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OLoadLWpnRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFLWPN)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //npc CreateLWeapon(screen, int)
    {
        int id = memberids["CreateLWeapon"];
        
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OCreateLWpnRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFLWPN)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //ewpn LoadEWeapon(screen, int)
    {
        int id = memberids["LoadEWeapon"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        //convert from 1-index to 0-index
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OLoadEWpnRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFEWPN)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //ewpn CreateEWeapon(screen, int)
    {
        int id = memberids["CreateEWeapon"];
        
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OCreateEWpnRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFEWPN)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void ClearSprites(screen, int)
    {
        int id = memberids["ClearSprites"];
        
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OClearSpritesRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFNPC)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void Rectangle(screen, float, float, float, float, float, float, float, float, float, float, bool, float)
    {
        int id = memberids["Rectangle"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new ORectangleRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(12, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        
        rval[label]=code;
    }
    //void Circle(screen, float, float, float, float, float, float, float, float, float, bool, float)
    {
        int id = memberids["Circle"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OCircleRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(11, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Arc(screen, float, float, float, float, float, float, float, float, float, float, float, bool, bool, float)
    {
        int id = memberids["Arc"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OArcRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(14, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Ellipse(screen, float, float, float, float, float, bool, float, float, float)
    {
        int id = memberids["Ellipse"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OEllipseRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(12, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Line(screen, float, float, float, float, float, float, float, float, float, float, float)
    {
        int id = memberids["Line"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OLineRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(11, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Spline(screen, float, float, float, float, float, float, float, float, float, float, float)
    {
        int id = memberids["Spline"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OSplineRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(11, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void PutPixel(screen, float, float, float, float, float, float, float, float)
    {
        int id = memberids["PutPixel"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPutPixelRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(8, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void DrawCharacter(screen, float, float, float, float, float, float, float, float, float, float)
    {
        int id = memberids["DrawCharacter"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new ODrawCharRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(10, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void DrawInteger(screen, float, float, float, float, float, float, float, float, float, float, float)
    {
        int id = memberids["DrawInteger"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new ODrawIntRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(11, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void DrawTile(screen, float, float, float, float, float, bool, float, float, float)
    {
        int id = memberids["DrawTile"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new ODrawTileRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(15, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void DrawCombo(screen, float, float, float, float, float, bool, float, float, float)
    {
        int id = memberids["DrawCombo"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new ODrawComboRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(16, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Quad(screen, float, float, float, float, float, float, float, float, float)
    {
        int id = memberids["Quad"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OQuadRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(15, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Triangle(screen, float, float, float, float, float, float, float, float, float)
    {
        int id = memberids["Triangle"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OTriangleRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(13, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //void Quad3D(screen, float, float, float, float, float, float, float, float, float)
    {
        int id = memberids["Quad3D"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OQuad3DRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(8, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Triangle3D(screen, float, float, float, float, float, float, float, float, float)
    {
        int id = memberids["Triangle3D"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OTriangle3DRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(8, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //void FastTile(screen, float, float, float, float, float)
    {
        int id = memberids["FastTile"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OFastTileRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(6, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void FastCombo(screen, float, float, float, float, float)
    {
        int id = memberids["FastCombo"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OFastComboRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(6, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void DrawString(screen, float, float, float, float, float, float, float, int *string)
    {
        int id = memberids["DrawString"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new ODrawStringRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(9, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void DrawLayer(screen, float, float, float, float, float, float, float, float)
    {
        int id = memberids["DrawLayer"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new ODrawLayerRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(8, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void DrawScreen(screen, float, float, float, float, float, float)
    {
        int id = memberids["DrawScreen"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new ODrawScreenRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(6, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void DrawBitmap(screen, float, float, float, float, float, float, float, float, float, bool)
    {
        int id = memberids["DrawBitmap"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new ODrawBitmapRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(12, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void SetRenderTarget(bitmap)
    {
        int id = memberids["SetRenderTarget"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OSetRenderTargetRegister();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Message(screen, float)
    {
        int id = memberids["Message"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OMessageRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //bool isSolid(screen, int, int)
    {
        int id = memberids["isSolid"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OIsSolid(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetSideWarp(screen, float, float, float, float)
    {
        int id = memberids["SetSideWarp"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OSetSideWarpRegister();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void SetTileWarp(screen, float, float, float, float)
    {
        int id = memberids["SetTileWarp"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OSetTileWarpRegister();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //float LayerScreen(screen, float)
    {
        int id = memberids["LayerScreen"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OLayerScreenRegister(new VarArgument(EXP1),new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //float LayerMap(screen, float)
    {
        int id = memberids["LayerMap"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OLayerMapRegister(new VarArgument(EXP1),new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void TriggerSecrets(screen)
    {
        int id = memberids["TriggerSecrets"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OTriggerSecrets());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int GetSideWarpDMap(screen, int)
    {
        int id = memberids["GetSideWarpDMap"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetSideWarpDMap(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int GetSideWarpScreen(screen, int)
    {
        int id = memberids["GetSideWarpScreen"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetSideWarpScreen(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int GetSideWarpType(screen, int)
    {
        int id = memberids["GetSideWarpType"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetSideWarpType(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int GetTileWarpDMap(screen, int)
    {
        int id = memberids["GetTileWarpDMap"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetTileWarpDMap(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int GetTileWarpScreen(screen, int)
    {
        int id = memberids["GetTileWarpScreen"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetTileWarpScreen(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int GetTileWarpType(screen, int)
    {
        int id = memberids["GetTileWarpType"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetTileWarpType(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    return rval;
}

ItemSymbols ItemSymbols::singleton = ItemSymbols();

static AccessorTable itemTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getX",                   ScriptParser::TYPE_FLOAT,         GETTER,       ITEMX,                1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ScriptParser::TYPE_VOID,          SETTER,       ITEMX,                1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ScriptParser::TYPE_FLOAT,         GETTER,       ITEMY,                1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ScriptParser::TYPE_VOID,          SETTER,       ITEMY,                1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getZ",                   ScriptParser::TYPE_FLOAT,         GETTER,       ITEMZ,                1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setZ",                   ScriptParser::TYPE_VOID,          SETTER,       ITEMZ,                1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJump",                ScriptParser::TYPE_FLOAT,         GETTER,       ITEMJUMP,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setJump",                ScriptParser::TYPE_VOID,          SETTER,       ITEMJUMP,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawStyle",           ScriptParser::TYPE_FLOAT,         GETTER,       ITEMDRAWTYPE,         1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawStyle",           ScriptParser::TYPE_VOID,          SETTER,       ITEMDRAWTYPE,         1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getID",                  ScriptParser::TYPE_FLOAT,         GETTER,       ITEMID,               1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setID",                  ScriptParser::TYPE_VOID,          SETTER,       ITEMID,               1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",                ScriptParser::TYPE_FLOAT,         GETTER,       ITEMTILE,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTile",                ScriptParser::TYPE_VOID,          SETTER,       ITEMTILE,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalTile",        ScriptParser::TYPE_FLOAT,         GETTER,       ITEMOTILE,            1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "setOriginalTile",        ScriptParser::TYPE_VOID,          SETTER,       ITEMOTILE,            1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "getCSet",                ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCSET,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCSet",                ScriptParser::TYPE_VOID,          SETTER,       ITEMCSET,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlashCSet",           ScriptParser::TYPE_FLOAT,         GETTER,       ITEMFLASHCSET,        1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlashCSet",           ScriptParser::TYPE_VOID,          SETTER,       ITEMFLASHCSET,        1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getNumFrames",           ScriptParser::TYPE_FLOAT,         GETTER,       ITEMFRAMES,           1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setNumFrames",           ScriptParser::TYPE_VOID,          SETTER,       ITEMFRAMES,           1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFrame",               ScriptParser::TYPE_FLOAT,         GETTER,       ITEMFRAME,            1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFrame",               ScriptParser::TYPE_VOID,          SETTER,       ITEMFRAME,            1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getASpeed",              ScriptParser::TYPE_FLOAT,         GETTER,       ITEMASPEED,           1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setASpeed",              ScriptParser::TYPE_VOID,          SETTER,       ITEMASPEED,           1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDelay",               ScriptParser::TYPE_FLOAT,         GETTER,       ITEMDELAY,            1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDelay",               ScriptParser::TYPE_VOID,          SETTER,       ITEMDELAY,            1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlash",               ScriptParser::TYPE_BOOL,          GETTER,       ITEMFLASH,            1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlash",               ScriptParser::TYPE_VOID,          SETTER,       ITEMFLASH,            1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlip",                ScriptParser::TYPE_FLOAT,         GETTER,       ITEMFLIP,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlip",                ScriptParser::TYPE_VOID,          SETTER,       ITEMFLIP,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getExtend",              ScriptParser::TYPE_FLOAT,         GETTER,       ITEMEXTEND,           1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setExtend",              ScriptParser::TYPE_VOID,          SETTER,       ITEMEXTEND,           1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitWidth",            ScriptParser::TYPE_FLOAT,         GETTER,       ITEMHXSZ,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitWidth",            ScriptParser::TYPE_VOID,          SETTER,       ITEMHXSZ,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitHeight",           ScriptParser::TYPE_FLOAT,         GETTER,       ITEMHYSZ,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitHeight",           ScriptParser::TYPE_VOID,          SETTER,       ITEMHYSZ,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitZHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       ITEMHZSZ,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitZHeight",          ScriptParser::TYPE_VOID,          SETTER,       ITEMHZSZ,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ScriptParser::TYPE_FLOAT,         GETTER,       ITEMTXSZ,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ScriptParser::TYPE_VOID,          SETTER,       ITEMTXSZ,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       ITEMTYSZ,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ScriptParser::TYPE_VOID,          SETTER,       ITEMTYSZ,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawXOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       ITEMXOFS,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawXOffset",         ScriptParser::TYPE_VOID,          SETTER,       ITEMXOFS,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawYOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       ITEMYOFS,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawYOffset",         ScriptParser::TYPE_VOID,          SETTER,       ITEMYOFS,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawZOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       ITEMZOFS,             1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawZOffset",         ScriptParser::TYPE_VOID,          SETTER,       ITEMZOFS,             1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitXOffset",          ScriptParser::TYPE_FLOAT,         GETTER,       ITEMHXOFS,            1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitXOffset",          ScriptParser::TYPE_VOID,          SETTER,       ITEMHXOFS,            1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitYOffset",          ScriptParser::TYPE_FLOAT,         GETTER,       ITEMHYOFS,            1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitYOffset",          ScriptParser::TYPE_VOID,          SETTER,       ITEMHYOFS,            1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPickup",              ScriptParser::TYPE_FLOAT,         GETTER,       ITEMPICKUP,           1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPickup",              ScriptParser::TYPE_VOID,          SETTER,       ITEMPICKUP,           1,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "isValid",                ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ScriptParser::TYPE_FLOAT,         GETTER,       ITEMMISCD,            16,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ScriptParser::TYPE_VOID,          SETTER,       ITEMMISCD,            16,      {  ScriptParser::TYPE_ITEM,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

ItemSymbols::ItemSymbols()
{
    table = itemTable;
    refVar = REFITEM;
}

map<int, vector<Opcode *> > ItemSymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
    //bool isValid(item)
    {
        int id = memberids["isValid"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new OIsValidItem(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    return rval;
}

ItemclassSymbols ItemclassSymbols::singleton = ItemclassSymbols();

static AccessorTable itemclassTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getFamily",              ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCLASSFAMILY,      1,      {  ScriptParser::TYPE_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFamily",              ScriptParser::TYPE_VOID,          SETTER,       ITEMCLASSFAMILY,      1,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLevel",               ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCLASSFAMTYPE,     1,      {  ScriptParser::TYPE_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLevel",               ScriptParser::TYPE_VOID,          SETTER,       ITEMCLASSFAMTYPE,     1,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAmount",              ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCLASSAMOUNT,      1,      {  ScriptParser::TYPE_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAmount",              ScriptParser::TYPE_VOID,          SETTER,       ITEMCLASSAMOUNT,      1,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMax",                 ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCLASSMAX,         1,      {  ScriptParser::TYPE_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMax",                 ScriptParser::TYPE_VOID,          SETTER,       ITEMCLASSMAX,         1,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMaxIncrement",        ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCLASSSETMAX,      1,      {  ScriptParser::TYPE_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMaxIncrement",        ScriptParser::TYPE_VOID,          SETTER,       ITEMCLASSSETMAX,      1,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getKeep",                ScriptParser::TYPE_BOOL,          GETTER,       ITEMCLASSSETGAME,     1,      {  ScriptParser::TYPE_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setKeep",                ScriptParser::TYPE_VOID,          SETTER,       ITEMCLASSSETGAME,     1,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCounter",             ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCLASSCOUNTER,     1,      {  ScriptParser::TYPE_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCounter",             ScriptParser::TYPE_VOID,          SETTER,       ITEMCLASSCOUNTER,     1,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getUseSound",            ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCLASSUSESOUND,    1,      {  ScriptParser::TYPE_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setUseSound",            ScriptParser::TYPE_VOID,          SETTER,       ITEMCLASSUSESOUND,    1,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPower",               ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCLASSPOWER,       1,      {  ScriptParser::TYPE_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPower",               ScriptParser::TYPE_VOID,          SETTER,       ITEMCLASSPOWER,       1,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInitD[]",             ScriptParser::TYPE_FLOAT,         GETTER,       ITEMCLASSINITDD,      2,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInitD[]",             ScriptParser::TYPE_VOID,          SETTER,       ITEMCLASSINITDD,      2,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetName",                ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_ITEMCLASS,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

ItemclassSymbols::ItemclassSymbols()
{
    table = itemclassTable;
    refVar = REFITEMCLASS;
}
map<int, vector<Opcode *> > ItemclassSymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
    //void GetName(itemclass, int)
    {
        int id = memberids["GetName"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetItemName(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    return rval;
}

GameSymbols GameSymbols::singleton = GameSymbols();

static AccessorTable gameTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "GetCurScreen",           ScriptParser::TYPE_FLOAT,         GETTER,       CURSCR,               1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetCurDMapScreen",       ScriptParser::TYPE_FLOAT,         GETTER,       CURDSCR,              1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetCurMap",              ScriptParser::TYPE_FLOAT,         GETTER,       CURMAP,               1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetCurDMap",             ScriptParser::TYPE_FLOAT,         GETTER,       CURDMAP,              1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetCurLevel",            ScriptParser::TYPE_FLOAT,         GETTER,       CURLEVEL,             1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getNumDeaths",           ScriptParser::TYPE_FLOAT,         GETTER,       GAMEDEATHS,           1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setNumDeaths",           ScriptParser::TYPE_VOID,          SETTER,       GAMEDEATHS,           1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCheat",               ScriptParser::TYPE_FLOAT,         GETTER,       GAMECHEAT,            1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCheat",               ScriptParser::TYPE_VOID,          SETTER,       GAMECHEAT,            1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTime",                ScriptParser::TYPE_FLOAT,         GETTER,       GAMETIME,             1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTime",                ScriptParser::TYPE_VOID,          SETTER,       GAMETIME,             1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHasPlayed",           ScriptParser::TYPE_BOOL,          GETTER,       GAMEHASPLAYED,        1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHasPlayed",           ScriptParser::TYPE_VOID,          SETTER,       GAMEHASPLAYED,        1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTimeValid",           ScriptParser::TYPE_BOOL,          GETTER,       GAMETIMEVALID,        1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTimeValid",           ScriptParser::TYPE_VOID,          SETTER,       GAMETIMEVALID,        1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getGuyCount[]",          ScriptParser::TYPE_FLOAT,         GETTER,       GAMEGUYCOUNT,         2,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setGuyCount[]",          ScriptParser::TYPE_VOID,          SETTER,       GAMEGUYCOUNT,         2,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getContinueScreen",      ScriptParser::TYPE_FLOAT,         GETTER,       GAMECONTSCR,          1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setContinueScreen",      ScriptParser::TYPE_VOID,          SETTER,       GAMECONTSCR,          1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getContinueDMap",        ScriptParser::TYPE_FLOAT,         GETTER,       GAMECONTDMAP,         1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setContinueDMap",        ScriptParser::TYPE_VOID,          SETTER,       GAMECONTDMAP,         1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCounter[]",           ScriptParser::TYPE_FLOAT,         GETTER,       GAMECOUNTERD,        32,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCounter[]",           ScriptParser::TYPE_VOID,          SETTER,       GAMECOUNTERD,        32,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMCounter[]",          ScriptParser::TYPE_FLOAT,         GETTER,       GAMEMCOUNTERD,       32,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMCounter[]",          ScriptParser::TYPE_VOID,          SETTER,       GAMEMCOUNTERD,       32,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDCounter[]",          ScriptParser::TYPE_FLOAT,         GETTER,       GAMEDCOUNTERD,       32,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDCounter[]",          ScriptParser::TYPE_VOID,          SETTER,       GAMEDCOUNTERD,       32,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getGeneric[]",           ScriptParser::TYPE_FLOAT,         GETTER,       GAMEGENERICD,       256,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setGeneric[]",           ScriptParser::TYPE_VOID,          SETTER,       GAMEGENERICD,       256,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLItems[]",            ScriptParser::TYPE_FLOAT,         GETTER,       GAMELITEMSD,        MAXLEVELS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLItems[]",            ScriptParser::TYPE_VOID,          SETTER,       GAMELITEMSD,        MAXLEVELS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLKeys[]",             ScriptParser::TYPE_FLOAT,         GETTER,       GAMELKEYSD,         MAXLEVELS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLKeys[]",             ScriptParser::TYPE_VOID,          SETTER,       GAMELKEYSD,         MAXLEVELS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenState",         ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenState",         ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,	  ScriptParser::TYPE_BOOL,		  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenD",             ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenD",             ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapScreenD",         ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetDMapScreenD",         ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadItemData",           ScriptParser::TYPE_ITEMCLASS,     FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PlaySound",              ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PlayMIDI",               ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PlayEnhancedMusic",      ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapMusicFilename",   ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapMusicTrack",      ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetDMapEnhancedMusic",   ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboData",           ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboData",           ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboCSet",           ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboCSet",           ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboFlag",           ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboFlag",           ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboType",           ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboType",           ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboInherentFlag",   ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboInherentFlag",   ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboSolid",          ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboSolid",          ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,     ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetMIDI",                ScriptParser::TYPE_FLOAT,         GETTER,       GETMIDI,              1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenFlags",         ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenEFlags",        ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapFlags[]",         ScriptParser::TYPE_FLOAT,         GETTER,       DMAPFLAGSD,         MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapFlags[]",         ScriptParser::TYPE_VOID,          SETTER,       DMAPFLAGSD,         MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapLevel[]",         ScriptParser::TYPE_FLOAT,         GETTER,       DMAPLEVELD,         MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapLevel[]",         ScriptParser::TYPE_VOID,          SETTER,       DMAPLEVELD,         MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapCompass[]",       ScriptParser::TYPE_FLOAT,         GETTER,       DMAPCOMPASSD,       MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapCompass[]",       ScriptParser::TYPE_VOID,          SETTER,       DMAPCOMPASSD,       MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapContinue[]",      ScriptParser::TYPE_FLOAT,         GETTER,       DMAPCONTINUED,      MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapContinue[]",      ScriptParser::TYPE_VOID,          SETTER,       DMAPCONTINUED,      MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapMIDI[]",          ScriptParser::TYPE_FLOAT,         GETTER,       DMAPMIDID,          MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapMIDI[]",          ScriptParser::TYPE_VOID,          SETTER,       DMAPMIDID,          MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Save",                   ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "End",                    ScriptParser::TYPE_VOID,          FUNCTION,     0,       	   	        1,      {  ScriptParser::TYPE_GAME,         -1,    					          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ComboTile",              ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetSaveName",            ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetSaveName",            ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetMessage",             ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapName",            ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapTitle",           ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapIntro",           ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStandalone",          ScriptParser::TYPE_BOOL,          GETTER,       GAMESTANDALONE,       1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setStandalone",          ScriptParser::TYPE_VOID,          SETTER,       GAMESTANDALONE,       1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ShowSaveScreen",         ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ShowSaveQuitScreen",     ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLastEntranceScreen",  ScriptParser::TYPE_FLOAT,         GETTER,       GAMEENTRSCR,          1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLastEntranceScreen",  ScriptParser::TYPE_VOID,          SETTER,       GAMEENTRSCR,          1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLastEntranceDMap",    ScriptParser::TYPE_FLOAT,         GETTER,       GAMEENTRDMAP,         1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLastEntranceDMap",    ScriptParser::TYPE_VOID,          SETTER,       GAMEENTRDMAP,         1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getClickToFreezeEnabled",ScriptParser::TYPE_BOOL,          GETTER,       GAMECLICKFREEZE,      1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setClickToFreezeEnabled",ScriptParser::TYPE_VOID,          SETTER,       GAMECLICKFREEZE,      1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapOffset[]",        ScriptParser::TYPE_FLOAT,         GETTER,       DMAPOFFSET,         MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapOffset[]",        ScriptParser::TYPE_VOID,          SETTER,       DMAPOFFSET,         MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapMap[]",           ScriptParser::TYPE_FLOAT,         GETTER,       DMAPMAP,            MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapMap[]",           ScriptParser::TYPE_VOID,          SETTER,       DMAPMAP,            MAXDMAPS,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetFFCScript",           ScriptParser::TYPE_FLOAT,         FUNCTION,     0,                    1,      {  ScriptParser::TYPE_GAME,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getVersion",	ScriptParser::TYPE_FLOAT,          GETTER,       ZELDAVERSION,      1,      {  ScriptParser::TYPE_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   
    { "",                       -1,                               -1,           -1,                  -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

GameSymbols::GameSymbols()
{
    table = gameTable;
    refVar = NUL;
}

map<int, vector<Opcode *> > GameSymbols::addSymbolsCode(LinkTable &lt)
{
    map<int,vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
    //itemclass LoadItemData(game, int)
    {
        int id = memberids["LoadItemData"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OLoadItemDataRegister(new VarArgument(EXP1)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFITEMCLASS)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //bool GetScreenState(game, int,int,int)
    {
        int id = memberids["GetScreenState"];
        int label = lt.functionToLabel(id);
        int done = ScriptParser::getUniqueLabelID();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OMultImmediate(new VarArgument(EXP1), new LiteralArgument(1360000)));
        code.push_back(new OAddRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SCREENSTATEDD)));
        code.push_back(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
        code.push_back(new OGotoTrueImmediate(new LabelArgument(done)));
        code.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OGotoImmediate(new LabelArgument(done)));
        Opcode *next = new OPopRegister(new VarArgument(EXP2));
        next->setLabel(done);
        code.push_back(next);
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenState(game, int,int,int,bool)
    {
        int id = memberids["SetScreenState"];
        int label = lt.functionToLabel(id);
        int done = ScriptParser::getUniqueLabelID();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
        code.push_back(new OMultImmediate(new VarArgument(EXP1), new LiteralArgument(1360000)));
        code.push_back(new OAddRegister(new VarArgument(INDEX), new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OCompareImmediate(new VarArgument(SFTEMP), new LiteralArgument(0)));
        code.push_back(new OGotoTrueImmediate(new LabelArgument(done)));
        code.push_back(new OSetImmediate(new VarArgument(SFTEMP), new LiteralArgument(10000)));
        Opcode *next = new OSetRegister(new VarArgument(SCREENSTATEDD), new VarArgument(SFTEMP));
        next->setLabel(done);
        code.push_back(next);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenD(game, int,int)
    {
        int id = memberids["GetScreenD"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SDDD)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenD(game, int,int,int)
    {
        int id = memberids["SetScreenD"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SDDD), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetDMapScreenD(game, int,int,int)
    {
        int id = memberids["GetDMapScreenD"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SDDDD)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetDMapScreenD(game, int,int,int,int)
    {
        int id = memberids["SetDMapScreenD"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SDDDD), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void PlaySound(game, int)
    {
        int id = memberids["PlaySound"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OPlaySoundRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void PlayMIDI(game, int)
    {
        int id = memberids["PlayMIDI"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OPlayMIDIRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void PlayEnhancedMusic(game, int, int)
    {
        int id = memberids["PlayEnhancedMusic"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OPlayEnhancedMusic(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void GetDMapMusicFilename(game, int, int)
    {
        int id = memberids["GetDMapMusicFilename"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetDMapMusicFilename(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetDMapMusicTrack(game, int)
    {
        int id = memberids["GetDMapMusicTrack"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetDMapMusicTrack(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void SetDMapEnhancedMusic(game, int,int,int)
    {
        int id = memberids["SetDMapEnhancedMusic"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OSetDMapEnhancedMusic();
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int GetComboData(int,int,int)
    {
        int id = memberids["GetComboData"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBODDM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetComboData(int,int,int,int)
    {
        int id = memberids["SetComboData"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(COMBODDM), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetComboCSet(int,int,int)
    {
        int id = memberids["GetComboCSet"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBOCDM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetComboCSet(int,int,int,int)
    {
        int id = memberids["SetComboCSet"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(COMBOCDM), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetComboFlag(int,int,int)
    {
        int id = memberids["GetComboFlag"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBOFDM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetComboFlag(int,int,int,int)
    {
        int id = memberids["SetComboFlag"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(COMBOFDM), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetComboType(int,int,int)
    {
        int id = memberids["GetComboType"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBOTDM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetComboType(int,int,int,int)
    {
        int id = memberids["SetComboType"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(COMBOTDM), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetComboInherentFlag(int,int,int)
    {
        int id = memberids["GetComboInherentFlag"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBOIDM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetComboInherentFlag(int,int,int,int)
    {
        int id = memberids["SetComboInherentFlag"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(COMBOIDM), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetComboCollision(int,int,int)
    {
        int id = memberids["GetComboSolid"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(COMBOSDM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetComboCollision(int,int,int,int)
    {
        int id = memberids["SetComboSolid"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(COMBOSDM), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenFlags(game,int,int,int)
    {
        int id = memberids["GetScreenFlags"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenFlags(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenEFlags(game,int,int,int)
    {
        int id = memberids["GetScreenEFlags"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenEFlags(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void Save(game)
    {
        int id = memberids["Save"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OSave());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void End(game)
    {
        int id = memberids["End"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OEnd());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int ComboTile(game,int)
    {
        int id = memberids["ComboTile"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OComboTile(new VarArgument(EXP1),new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void GetSaveName(game, int)
    {
        int id = memberids["GetSaveName"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetSaveName(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void GetSaveName(game, int)
    {
        int id = memberids["SetSaveName"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetSaveName(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //GetMessage(game, int, int)
    {
        int id = memberids["GetMessage"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        code.push_back(new OGetMessage(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //GetDMapName(game, int, int)
    {
        int id = memberids["GetDMapName"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        code.push_back(new OGetDMapName(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //GetDMapTitle(game, int, int)
    {
        int id = memberids["GetDMapTitle"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        code.push_back(new OGetDMapTitle(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //GetDMapIntro(game, int, int)
    {
        int id = memberids["GetDMapIntro"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        code.push_back(new OGetDMapIntro(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //bool ShowSaveScreen(game)
    {
        int id = memberids["ShowSaveScreen"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OShowSaveScreen(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //void ShowSaveQuitScreen(game)
    {
        int id = memberids["ShowSaveQuitScreen"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OShowSaveQuitScreen());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //int GetFFCScript(game, int)
    {
        int id = memberids["GetFFCScript"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetFFCScript(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    return rval;
}

NPCSymbols NPCSymbols::singleton = NPCSymbols();

static AccessorTable npcTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getX",                   ScriptParser::TYPE_FLOAT,         GETTER,       NPCX,                 1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ScriptParser::TYPE_VOID,          SETTER,       NPCX,                 1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ScriptParser::TYPE_FLOAT,         GETTER,       NPCY,                 1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ScriptParser::TYPE_VOID,          SETTER,       NPCY,                 1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getZ",                   ScriptParser::TYPE_FLOAT,         GETTER,       NPCZ,                 1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setZ",                   ScriptParser::TYPE_VOID,          SETTER,       NPCZ,                 1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJump",                ScriptParser::TYPE_FLOAT,         GETTER,       NPCJUMP,              1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setJump",                ScriptParser::TYPE_VOID,          SETTER,       NPCJUMP,              1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDir",                 ScriptParser::TYPE_FLOAT,         GETTER,       NPCDIR,               1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDir",                 ScriptParser::TYPE_VOID,          SETTER,       NPCDIR,               1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getRate",                ScriptParser::TYPE_FLOAT,         GETTER,       NPCRATE,              1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setRate",                ScriptParser::TYPE_VOID,          SETTER,       NPCRATE,              1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHoming",              ScriptParser::TYPE_FLOAT,         GETTER,       NPCHOMING,            1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHoming",              ScriptParser::TYPE_VOID,          SETTER,       NPCHOMING,            1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStep",				  ScriptParser::TYPE_FLOAT,	        GETTER,       NPCSTEP,              1,      {  ScriptParser::TYPE_NPC,			-1,								  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                  } },
    { "setStep",				  ScriptParser::TYPE_VOID,          SETTER,       NPCSTEP,              1,      {  ScriptParser::TYPE_NPC,			 ScriptParser::TYPE_FLOAT,		  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                       } },
    { "getASpeed",              ScriptParser::TYPE_FLOAT,         GETTER,       NPCFRAMERATE,         1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setASpeed",              ScriptParser::TYPE_VOID,          SETTER,       NPCFRAMERATE,         1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHaltrate",            ScriptParser::TYPE_FLOAT,         GETTER,       NPCHALTRATE,          1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHaltrate",            ScriptParser::TYPE_VOID,          SETTER,       NPCHALTRATE,          1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawStyle",           ScriptParser::TYPE_FLOAT,         GETTER,       NPCDRAWTYPE,          1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawStyle",           ScriptParser::TYPE_VOID,          SETTER,       NPCDRAWTYPE,          1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHP",                  ScriptParser::TYPE_FLOAT,         GETTER,       NPCHP,                1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHP",                  ScriptParser::TYPE_VOID,          SETTER,       NPCHP,                1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getID",                  ScriptParser::TYPE_FLOAT,         GETTER,       NPCID,                1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setID",                  ScriptParser::TYPE_VOID,          SETTER,       NPCID,                1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getType",                ScriptParser::TYPE_FLOAT,         GETTER,       NPCTYPE,              1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setType",                ScriptParser::TYPE_VOID,          SETTER,       NPCTYPE,              1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDamage",              ScriptParser::TYPE_FLOAT,         GETTER,       NPCDP,                1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDamage",              ScriptParser::TYPE_VOID,          SETTER,       NPCDP,                1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWeaponDamage",        ScriptParser::TYPE_FLOAT,         GETTER,       NPCWDP,               1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setWeaponDamage",        ScriptParser::TYPE_VOID,          SETTER,       NPCWDP,               1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",                ScriptParser::TYPE_FLOAT,         GETTER,       NPCTILE,              1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTile",                ScriptParser::TYPE_VOID,          SETTER,       NPCTILE,              1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalTile",        ScriptParser::TYPE_FLOAT,         GETTER,       NPCOTILE,             1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setOriginalTile",        ScriptParser::TYPE_VOID,          SETTER,       NPCOTILE,             1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWeapon",              ScriptParser::TYPE_FLOAT,         GETTER,       NPCWEAPON,            1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setWeapon",              ScriptParser::TYPE_VOID,          SETTER,       NPCWEAPON,            1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getItemSet",             ScriptParser::TYPE_FLOAT,         GETTER,       NPCITEMSET,           1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setItemSet",             ScriptParser::TYPE_VOID,          SETTER,       NPCITEMSET,           1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCSet",                ScriptParser::TYPE_FLOAT,         GETTER,       NPCCSET,              1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCSet",                ScriptParser::TYPE_VOID,          SETTER,       NPCCSET,              1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getBossPal",             ScriptParser::TYPE_FLOAT,         GETTER,       NPCBOSSPAL,           1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setBossPal",             ScriptParser::TYPE_VOID,          SETTER,       NPCBOSSPAL,           1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getSFX",                 ScriptParser::TYPE_FLOAT,         GETTER,       NPCBGSFX,             1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setSFX",                 ScriptParser::TYPE_VOID,          SETTER,       NPCBGSFX,             1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getExtend",              ScriptParser::TYPE_FLOAT,         GETTER,       NPCEXTEND,            1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setExtend",              ScriptParser::TYPE_VOID,          SETTER,       NPCEXTEND,            1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitWidth",            ScriptParser::TYPE_FLOAT,         GETTER,       NPCHXSZ,              1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitWidth",            ScriptParser::TYPE_VOID,          SETTER,       NPCHXSZ,              1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitHeight",           ScriptParser::TYPE_FLOAT,         GETTER,       NPCHYSZ,              1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitHeight",           ScriptParser::TYPE_VOID,          SETTER,       NPCHYSZ,              1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitZHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       NPCHZSZ,              1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitZHeight",          ScriptParser::TYPE_VOID,          SETTER,       NPCHZSZ,              1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ScriptParser::TYPE_FLOAT,         GETTER,       NPCTXSZ,              1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ScriptParser::TYPE_VOID,          SETTER,       NPCTXSZ,              1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       NPCTYSZ,              1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ScriptParser::TYPE_VOID,          SETTER,       NPCTYSZ,              1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawXOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       NPCXOFS,              1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawXOffset",         ScriptParser::TYPE_VOID,          SETTER,       NPCXOFS,              1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawYOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       NPCYOFS,              1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawYOffset",         ScriptParser::TYPE_VOID,          SETTER,       NPCYOFS,              1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawZOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       NPCZOFS,              1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawZOffset",         ScriptParser::TYPE_VOID,          SETTER,       NPCZOFS,              1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitXOffset",          ScriptParser::TYPE_FLOAT,         GETTER,       NPCHXOFS,             1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitXOffset",          ScriptParser::TYPE_VOID,          SETTER,       NPCHXOFS,             1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitYOffset",          ScriptParser::TYPE_FLOAT,         GETTER,       NPCHYOFS,             1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitYOffset",          ScriptParser::TYPE_VOID,          SETTER,       NPCHYOFS,             1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "isValid",				  ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ScriptParser::TYPE_FLOAT,         GETTER,       NPCMISCD,            16,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ScriptParser::TYPE_VOID,          SETTER,       NPCMISCD,            16,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAttributes[]",        ScriptParser::TYPE_FLOAT,         GETTER,       NPCDD,               12,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAttributes[]",        ScriptParser::TYPE_VOID,          SETTER,       NPCDD,               12,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMiscFlags",           ScriptParser::TYPE_FLOAT,         GETTER,       NPCMFLAGS,            1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMiscFlags",           ScriptParser::TYPE_VOID,          SETTER,       NPCMFLAGS,            1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection", 	  ScriptParser::TYPE_BOOL,         GETTER,       NPCCOLLDET,           1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection",       ScriptParser::TYPE_VOID,          SETTER,       NPCCOLLDET,           1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStun",                ScriptParser::TYPE_FLOAT,         GETTER,       NPCSTUN,              1,      {  ScriptParser::TYPE_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setStun",                ScriptParser::TYPE_VOID,          SETTER,       NPCSTUN,              1,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDefense[]",           ScriptParser::TYPE_FLOAT,         GETTER,       NPCDEFENSED,         16,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDefense[]",           ScriptParser::TYPE_VOID,          SETTER,       NPCDEFENSED,         16,      {  ScriptParser::TYPE_NPC,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetName",                ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_NPC,     ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHunger",              ScriptParser::TYPE_FLOAT,         GETTER,       NPCHUNGER,            1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHunger",              ScriptParser::TYPE_VOID,          SETTER,       NPCHUNGER,            1,      {  ScriptParser::TYPE_NPC,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "BreakShield",			  ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

NPCSymbols::NPCSymbols()
{
    table = npcTable;
    refVar = REFNPC;
}

map<int, vector<Opcode *> > NPCSymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
    //bool isValid(npc)
    {
        int id = memberids["isValid"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new OIsValidNPC(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void GetName(npc, int)
    {
        int id = memberids["GetName"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetNPCName(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void BreakShield(npc)
    {
        int id = memberids["BreakShield"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Break shield
        code.push_back(new OBreakShield(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    return rval;
}

LinkWeaponSymbols LinkWeaponSymbols::singleton = LinkWeaponSymbols();

static AccessorTable lwpnTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getX",                   ScriptParser::TYPE_FLOAT,         GETTER,       LWPNX,                1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ScriptParser::TYPE_VOID,          SETTER,       LWPNX,                1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ScriptParser::TYPE_FLOAT,         GETTER,       LWPNY,                1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ScriptParser::TYPE_VOID,          SETTER,       LWPNY,                1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getZ",                   ScriptParser::TYPE_FLOAT,         GETTER,       LWPNZ,                1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setZ",                   ScriptParser::TYPE_VOID,          SETTER,       LWPNZ,                1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJump",                ScriptParser::TYPE_FLOAT,         GETTER,       LWPNJUMP,             1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setJump",                ScriptParser::TYPE_VOID,          SETTER,       LWPNJUMP,             1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDir",                 ScriptParser::TYPE_FLOAT,         GETTER,       LWPNDIR,              1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDir",                 ScriptParser::TYPE_VOID,          SETTER,       LWPNDIR,              1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAngle",               ScriptParser::TYPE_FLOAT,         GETTER,       LWPNANGLE,            1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAngle",               ScriptParser::TYPE_VOID,          SETTER,       LWPNANGLE,            1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStep",			      ScriptParser::TYPE_FLOAT,	        GETTER,	      LWPNSTEP,             1,	    {  ScriptParser::TYPE_LWPN,			-1,								  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setStep",			      ScriptParser::TYPE_VOID,		    SETTER,	      LWPNSTEP,             1,	    {  ScriptParser::TYPE_LWPN,			 ScriptParser::TYPE_FLOAT,		  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getNumFrames",           ScriptParser::TYPE_FLOAT,         GETTER,       LWPNFRAMES,           1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setNumFrames",           ScriptParser::TYPE_VOID,          SETTER,       LWPNFRAMES,           1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFrame",               ScriptParser::TYPE_FLOAT,         GETTER,       LWPNFRAME,            1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFrame",               ScriptParser::TYPE_VOID,          SETTER,       LWPNFRAME,            1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawStyle",           ScriptParser::TYPE_FLOAT,         GETTER,       LWPNDRAWTYPE,         1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawStyle",           ScriptParser::TYPE_VOID,          SETTER,       LWPNDRAWTYPE,         1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDamage",              ScriptParser::TYPE_FLOAT,         GETTER,       LWPNPOWER,            1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDamage",              ScriptParser::TYPE_VOID,          SETTER,       LWPNPOWER,            1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getID",                  ScriptParser::TYPE_FLOAT,         GETTER,       LWPNID,               1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setID",                  ScriptParser::TYPE_VOID,          SETTER,       LWPNID,               1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAngular",             ScriptParser::TYPE_BOOL,          GETTER,       LWPNANGULAR,          1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAngular",             ScriptParser::TYPE_VOID,          SETTER,       LWPNANGULAR,          1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getBehind",              ScriptParser::TYPE_BOOL,          GETTER,       LWPNBEHIND,           1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setBehind",              ScriptParser::TYPE_VOID,          SETTER,       LWPNBEHIND,           1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getASpeed",              ScriptParser::TYPE_FLOAT,         GETTER,       LWPNASPEED,           1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setASpeed",              ScriptParser::TYPE_VOID,          SETTER,       LWPNASPEED,           1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",                ScriptParser::TYPE_FLOAT,         GETTER,       LWPNTILE,             1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTile",                ScriptParser::TYPE_VOID,          SETTER,       LWPNTILE,             1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlashCSet",           ScriptParser::TYPE_FLOAT,         GETTER,       LWPNFLASHCSET,        1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlashCSet",           ScriptParser::TYPE_VOID,          SETTER,       LWPNFLASHCSET,        1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDeadState",           ScriptParser::TYPE_FLOAT,         GETTER,       LWPNDEAD,             1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDeadState",           ScriptParser::TYPE_VOID,          SETTER,       LWPNDEAD,             1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCSet",                ScriptParser::TYPE_FLOAT,         GETTER,       LWPNCSET,             1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCSet",                ScriptParser::TYPE_VOID,          SETTER,       LWPNCSET,             1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlash",               ScriptParser::TYPE_BOOL,          GETTER,       LWPNFLASH,            1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlash",               ScriptParser::TYPE_VOID,          SETTER,       LWPNFLASH,            1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlip",                ScriptParser::TYPE_FLOAT,         GETTER,       LWPNFLIP,             1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlip",                ScriptParser::TYPE_VOID,          SETTER,       LWPNFLIP,             1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalTile",        ScriptParser::TYPE_FLOAT,         GETTER,       LWPNOTILE,            1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setOriginalTile",        ScriptParser::TYPE_VOID,          SETTER,       LWPNOTILE,            1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalCSet",        ScriptParser::TYPE_FLOAT,         GETTER,       LWPNOCSET,            1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setOriginalCSet",		  ScriptParser::TYPE_VOID,          SETTER,       LWPNOCSET,            1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getExtend",              ScriptParser::TYPE_FLOAT,         GETTER,       LWPNEXTEND,           1,      {  ScriptParser::TYPE_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setExtend",              ScriptParser::TYPE_VOID,          SETTER,       LWPNEXTEND,           1,      {  ScriptParser::TYPE_LWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitWidth",            ScriptParser::TYPE_FLOAT,         GETTER,       LWPNHXSZ,             1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitWidth",            ScriptParser::TYPE_VOID,          SETTER,       LWPNHXSZ,             1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitHeight",           ScriptParser::TYPE_FLOAT,         GETTER,       LWPNHYSZ,             1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitHeight",           ScriptParser::TYPE_VOID,          SETTER,       LWPNHYSZ,             1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitZHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       LWPNHZSZ,             1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitZHeight",          ScriptParser::TYPE_VOID,          SETTER,       LWPNHZSZ,             1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ScriptParser::TYPE_FLOAT,         GETTER,       LWPNTXSZ,             1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ScriptParser::TYPE_VOID,          SETTER,       LWPNTXSZ,             1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       LWPNTYSZ,             1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ScriptParser::TYPE_VOID,          SETTER,       LWPNTYSZ,             1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawXOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       LWPNXOFS,             1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawXOffset",         ScriptParser::TYPE_VOID,          SETTER,       LWPNXOFS,             1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawYOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       LWPNYOFS,             1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawYOffset",         ScriptParser::TYPE_VOID,          SETTER,       LWPNYOFS,             1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawZOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       LWPNZOFS,             1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawZOffset",         ScriptParser::TYPE_VOID,          SETTER,       LWPNZOFS,             1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitXOffset",          ScriptParser::TYPE_FLOAT,         GETTER,       LWPNHXOFS,            1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitXOffset",          ScriptParser::TYPE_VOID,          SETTER,       LWPNHXOFS,            1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitYOffset",          ScriptParser::TYPE_FLOAT,         GETTER,       LWPNHYOFS,            1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitYOffset",          ScriptParser::TYPE_VOID,          SETTER,       LWPNHYOFS,            1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "isValid",			      ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_LWPN,          -1,                              -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "UseSprite",              ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ScriptParser::TYPE_FLOAT,         GETTER,       LWPNMISCD,           16,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ScriptParser::TYPE_VOID,          SETTER,       LWPNMISCD,           16,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection", 	  ScriptParser::TYPE_BOOL,         GETTER,       LWPNCOLLDET,          1,      {  ScriptParser::TYPE_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection", 	  ScriptParser::TYPE_VOID,          SETTER,       LWPNCOLLDET,          1,      {  ScriptParser::TYPE_LWPN,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                                -1,                              -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

LinkWeaponSymbols::LinkWeaponSymbols()
{
    table = lwpnTable;
    refVar = REFLWPN;
}

map<int, vector<Opcode *> > LinkWeaponSymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
    int id=-1;
    //bool isValid(lweapon)
    {
        id = memberids["isValid"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new OIsValidLWpn(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void UseSprite(lweapon, int val)
    {
        id = memberids["UseSprite"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the val
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop off the pointer
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
        code.push_back(new OUseSpriteLWpn(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    return rval;
}

EnemyWeaponSymbols EnemyWeaponSymbols::singleton = EnemyWeaponSymbols();

static AccessorTable ewpnTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getX",                   ScriptParser::TYPE_FLOAT,         GETTER,       EWPNX,                1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ScriptParser::TYPE_VOID,          SETTER,       EWPNX,                1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ScriptParser::TYPE_FLOAT,         GETTER,       EWPNY,                1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ScriptParser::TYPE_VOID,          SETTER,       EWPNY,                1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getZ",                   ScriptParser::TYPE_FLOAT,         GETTER,       EWPNZ,                1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setZ",                   ScriptParser::TYPE_VOID,          SETTER,       EWPNZ,                1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJump",                ScriptParser::TYPE_FLOAT,         GETTER,       EWPNJUMP,             1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setJump",                ScriptParser::TYPE_VOID,          SETTER,       EWPNJUMP,             1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDir",                 ScriptParser::TYPE_FLOAT,         GETTER,       EWPNDIR,              1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDir",                 ScriptParser::TYPE_VOID,          SETTER,       EWPNDIR,              1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAngle",               ScriptParser::TYPE_FLOAT,         GETTER,       EWPNANGLE,            1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAngle",               ScriptParser::TYPE_VOID,          SETTER,       EWPNANGLE,            1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStep",				  ScriptParser::TYPE_FLOAT,         GETTER,	      EWPNSTEP,             1,      {  ScriptParser::TYPE_EWPN,			-1,								  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setStep",				  ScriptParser::TYPE_VOID,		    SETTER,	      EWPNSTEP,             1,      {  ScriptParser::TYPE_EWPN,			 ScriptParser::TYPE_FLOAT,		  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getNumFrames",           ScriptParser::TYPE_FLOAT,         GETTER,       EWPNFRAMES,           1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setNumFrames",           ScriptParser::TYPE_VOID,          SETTER,       EWPNFRAMES,           1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFrame",               ScriptParser::TYPE_FLOAT,         GETTER,       EWPNFRAME,            1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFrame",               ScriptParser::TYPE_VOID,          SETTER,       EWPNFRAME,            1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawStyle",           ScriptParser::TYPE_FLOAT,         GETTER,       EWPNDRAWTYPE,         1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawStyle",           ScriptParser::TYPE_VOID,          SETTER,       EWPNDRAWTYPE,         1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDamage",              ScriptParser::TYPE_FLOAT,         GETTER,       EWPNPOWER,            1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDamage",              ScriptParser::TYPE_VOID,          SETTER,       EWPNPOWER,            1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getID",                  ScriptParser::TYPE_FLOAT,         GETTER,       EWPNID,               1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setID",                  ScriptParser::TYPE_VOID,          SETTER,       EWPNID,               1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAngular",             ScriptParser::TYPE_BOOL,          GETTER,       EWPNANGULAR,          1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAngular",             ScriptParser::TYPE_VOID,          SETTER,       EWPNANGULAR,          1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getBehind",              ScriptParser::TYPE_BOOL,          GETTER,       EWPNBEHIND,           1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setBehind",              ScriptParser::TYPE_VOID,          SETTER,       EWPNBEHIND,           1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getASpeed",              ScriptParser::TYPE_FLOAT,         GETTER,       EWPNASPEED,           1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setASpeed",              ScriptParser::TYPE_VOID,          SETTER,       EWPNASPEED,           1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",                ScriptParser::TYPE_FLOAT,         GETTER,       EWPNTILE,             1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTile",                ScriptParser::TYPE_VOID,          SETTER,       EWPNTILE,             1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlashCSet",           ScriptParser::TYPE_FLOAT,         GETTER,       EWPNFLASHCSET,        1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlashCSet",           ScriptParser::TYPE_VOID,          SETTER,       EWPNFLASHCSET,        1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDeadState",           ScriptParser::TYPE_FLOAT,         GETTER,       EWPNDEAD,             1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDeadState",           ScriptParser::TYPE_VOID,          SETTER,       EWPNDEAD,             1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCSet",                ScriptParser::TYPE_FLOAT,         GETTER,       EWPNCSET,             1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCSet",                ScriptParser::TYPE_VOID,          SETTER,       EWPNCSET,             1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlash",               ScriptParser::TYPE_BOOL,          GETTER,       EWPNFLASH,            1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlash",               ScriptParser::TYPE_VOID,          SETTER,       EWPNFLASH,            1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlip",                ScriptParser::TYPE_FLOAT,         GETTER,       EWPNFLIP,             1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlip",                ScriptParser::TYPE_VOID,          SETTER,       EWPNFLIP,             1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalTile",        ScriptParser::TYPE_FLOAT,         GETTER,       EWPNOTILE,            1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setOriginalTile",        ScriptParser::TYPE_VOID,          SETTER,       EWPNOTILE,            1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalCSet",        ScriptParser::TYPE_FLOAT,         GETTER,       EWPNOCSET,            1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setOriginalCSet",	      ScriptParser::TYPE_VOID,          SETTER,       EWPNOCSET,            1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getExtend",              ScriptParser::TYPE_FLOAT,         GETTER,       EWPNEXTEND,           1,      {  ScriptParser::TYPE_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setExtend",              ScriptParser::TYPE_VOID,          SETTER,       EWPNEXTEND,           1,      {  ScriptParser::TYPE_EWPN,           ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitWidth",            ScriptParser::TYPE_FLOAT,         GETTER,       EWPNHXSZ,             1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitWidth",            ScriptParser::TYPE_VOID,          SETTER,       EWPNHXSZ,             1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitHeight",           ScriptParser::TYPE_FLOAT,         GETTER,       EWPNHYSZ,             1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitHeight",           ScriptParser::TYPE_VOID,          SETTER,       EWPNHYSZ,             1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitZHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       EWPNHZSZ,             1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitZHeight",          ScriptParser::TYPE_VOID,          SETTER,       EWPNHZSZ,             1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ScriptParser::TYPE_FLOAT,         GETTER,       EWPNTXSZ,             1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ScriptParser::TYPE_VOID,          SETTER,       EWPNTXSZ,             1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ScriptParser::TYPE_FLOAT,         GETTER,       EWPNTYSZ,             1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ScriptParser::TYPE_VOID,          SETTER,       EWPNTYSZ,             1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawXOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       EWPNXOFS,             1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawXOffset",         ScriptParser::TYPE_VOID,          SETTER,       EWPNXOFS,             1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawYOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       EWPNYOFS,             1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawYOffset",         ScriptParser::TYPE_VOID,          SETTER,       EWPNYOFS,             1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawZOffset",         ScriptParser::TYPE_FLOAT,         GETTER,       EWPNZOFS,             1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawZOffset",         ScriptParser::TYPE_VOID,          SETTER,       EWPNZOFS,             1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitXOffset",          ScriptParser::TYPE_FLOAT,         GETTER,       EWPNHXOFS,            1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitXOffset",          ScriptParser::TYPE_VOID,          SETTER,       EWPNHXOFS,            1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitYOffset",          ScriptParser::TYPE_FLOAT,         GETTER,       EWPNHYOFS,            1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitYOffset",          ScriptParser::TYPE_VOID,          SETTER,       EWPNHYOFS,            1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "isValid",				  ScriptParser::TYPE_BOOL,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_EWPN,          -1,                              -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "UseSprite",              ScriptParser::TYPE_VOID,          FUNCTION,     0,                    1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ScriptParser::TYPE_FLOAT,         GETTER,       EWPNMISCD,           16,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ScriptParser::TYPE_VOID,          SETTER,       EWPNMISCD,           16,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_FLOAT,         ScriptParser::TYPE_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection", 	  ScriptParser::TYPE_FLOAT,         GETTER,       EWPNCOLLDET,          1,      {  ScriptParser::TYPE_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection", 	  ScriptParser::TYPE_VOID,          SETTER,       EWPNCOLLDET,          1,      {  ScriptParser::TYPE_EWPN,          ScriptParser::TYPE_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                                -1,                              -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

EnemyWeaponSymbols::EnemyWeaponSymbols()
{
    table = ewpnTable;
    refVar = REFEWPN;
}

map<int, vector<Opcode *> > EnemyWeaponSymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
    int id=-1;
    //bool isValid(eweapon)
    {
        id = memberids["isValid"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the pointer
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //Check validity
        code.push_back(new OIsValidEWpn(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void UseSprite(eweapon, int val)
    {
        id = memberids["UseSprite"];
        int label = lt.functionToLabel(id);
        vector<Opcode *> code;
        //pop off the val
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop off the pointer
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
        code.push_back(new OUseSpriteEWpn(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    return rval;
}


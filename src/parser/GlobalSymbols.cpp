
#include "../precompiled.h" //always first

#include "GlobalSymbols.h"
#include "ByteCode.h"
#include "Scope.h"
#include "../zsyssimple.h"
#include "ZScript.h"
#include <assert.h>
#include "../zdefs.h" //Needed for using defined values. -Z

using namespace ZScript;

const int radsperdeg = 572958;


//sanity underflow
#define typeVOID ZVARTYPEID_VOID
#define S ZVARTYPEID_SCREEN
#define F ZVARTYPEID_FLOAT

#define ARGS_4(t, arg1, arg2, arg3, arg4) \
	{ t, arg1, arg2, arg3, arg4, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
#define ARGS_6(t, arg1, arg2, arg3, arg4, arg5, arg6) \
	{ t, arg1, arg2, arg3, arg4, arg5, arg6, -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
#define ARGS_8(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
	{ t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
#define ARGS_12(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12) { t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,arg9,arg10,arg11,arg12,-1,-1,-1,-1,-1,-1,-1 }
#define ARGS_13(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13) { t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,arg9,arg10,arg11,arg12,arg13,-1,-1,-1,-1,-1,-1 }
#define ARGS_15(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15) { t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,arg9,arg10,arg11,arg12,arg13,arg14,arg15,-1,-1,-1,-1 }

#define ARGS_16(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16) { t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,arg9,arg10,arg11,arg12,arg13,arg14,arg15,arg16,-1,-1,-1 }

#define ARGS_17(t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17) { t, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8,arg9,arg10,arg11,arg12,arg13,arg14,arg15,arg16,arg17,-1,-1 }

#define POP_ARGS(num_args, t) \
	for(int _i(0); _i < num_args; ++_i) \
		code.push_back(new OPopRegister(new VarArgument(t)))

LibrarySymbols* LibrarySymbols::getTypeInstance(ZVarTypeId typeId)
{
    switch (typeId)
    {
    case ZVARTYPEID_FFC: return &FFCSymbols::getInst();
    case ZVARTYPEID_LINK: return &LinkSymbols::getInst();
    case ZVARTYPEID_SCREEN: return &ScreenSymbols::getInst();
    case ZVARTYPEID_GAME: return &GameSymbols::getInst();
    case ZVARTYPEID_ITEM: return &ItemSymbols::getInst();
    case ZVARTYPEID_ITEMCLASS: return &ItemclassSymbols::getInst();
    case ZVARTYPEID_NPC: return &NPCSymbols::getInst();
    case ZVARTYPEID_LWPN: return &LinkWeaponSymbols::getInst();
    case ZVARTYPEID_EWPN: return &EnemyWeaponSymbols::getInst();
	case ZVARTYPEID_AUDIO: return &AudioSymbols::getInst();
	case ZVARTYPEID_DEBUG: return &DebugSymbols::getInst();
	case ZVARTYPEID_NPCDATA: return &NPCDataSymbols::getInst();
    default: return NULL;
    }
}

void LibrarySymbols::addSymbolsToScope(Scope& scope)
{
	SymbolTable& symbolTable = scope.getTable();

    functions.clear();
	getters.clear();
	setters.clear();

    for (int i = 0; table[i].name != ""; i++)
    {
		AccessorTable& entry = table[i];
		ZVarType const* returnType = symbolTable.getType(entry.rettype);
		vector<ZVarType const*> paramTypes;
        for (int k = 0; entry.params[k] != -1 && k < 20; k++)
			paramTypes.push_back(symbolTable.getType(entry.params[k]));

        string name = entry.name;
		string varName = name;

		// Strip out the array at the end.
		bool isArray = name.substr(name.size() - 2) == "[]";
		if (isArray)
			varName = name.substr(0, name.size() - 2);


		if (entry.setorget == SETTER && name.substr(0, 3) == "set")
		{
			varName = varName.substr(3); // Strip out "set".
			Function* function = scope.addSetter(returnType, varName, paramTypes);
			assert(function);
			setters[name] = function;
		}
		else if (entry.setorget == GETTER && name.substr(0, 3) == "get")
		{
			varName = varName.substr(3); // Strip out "get".
			Function* function = scope.addGetter(returnType, varName, paramTypes);
			assert(function);
			getters[name] = function;
		}
		else
		{
			Function* function = scope.addFunction(returnType, varName, paramTypes);
			assert(function != NULL);
			functions[name] = function;
		}
    }
}

Function* LibrarySymbols::getFunction(string const& name) const
{
	map<string, Function*>::const_iterator it;
	it = functions.find(name);
	if (it != functions.end()) {return it->second;}
	it = getters.find(name);
	if (it != getters.end()) {return it->second;}
	it = setters.find(name);
	if (it != setters.end()) {return it->second;}
	return 0;
}

vector<Opcode*> getVariable(int refVar, Function* function, int var)
{
	int label = function->getLabel();
    vector<Opcode *> code;
    //pop object pointer
    Opcode *first = new OPopRegister(new VarArgument(EXP2));
    first->setLabel(label);
    code.push_back(first);

    // Load object pointer into ref register.
    if (refVar != NUL)
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
    code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(var)));
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    return code;
}

vector<Opcode*> getIndexedVariable(int refVar, Function* function, int var)
{
	int label = function->getLabel();
    vector<Opcode *> code;
    //pop index
    Opcode *first = new OPopRegister(new VarArgument(INDEX));
    first->setLabel(label);
    code.push_back(first);
    //pop object pointer
    code.push_back(new OPopRegister(new VarArgument(EXP2)));

    // Load object pointer into ref register.
    if (refVar != NUL)
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
    code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(var)));
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    return code;
}

vector<Opcode*> setVariable(int refVar, Function* function, int var)
{
	int label = function->getLabel();
    vector<Opcode *> code;
    //pop off the value to set to
    Opcode *first = new OPopRegister(new VarArgument(EXP1));
    first->setLabel(label);
    code.push_back(first);
    //pop object pointer
    code.push_back(new OPopRegister(new VarArgument(EXP2)));

    // Load object pointer into ref register.
    if (refVar != NUL)
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
    code.push_back(new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    return code;
}

vector<Opcode*> setBoolVariable(int refVar, Function* function, int var)
{
	int label = function->getLabel();
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

    // Load object pointer into ref register.
    if (refVar != NUL)
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
    code.push_back(new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    return code;
}

vector<Opcode*> setIndexedVariable(int refVar, Function* function, int var)
{
	int label = function->getLabel();
    vector<Opcode *> code;
    //pop off index
    Opcode *first = new OPopRegister(new VarArgument(INDEX));
    first->setLabel(label);
    code.push_back(first);
    //pop off the value to set to
    code.push_back(new OPopRegister(new VarArgument(EXP1)));
    //pop object pointer
    code.push_back(new OPopRegister(new VarArgument(EXP2)));

    // Load object pointer into ref register.
    if (refVar != NUL)
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
    code.push_back(new OSetRegister(new VarArgument(var), new VarArgument(EXP1)));
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.push_back(new OGotoRegister(new VarArgument(EXP2)));
    return code;
}

map<int, vector<Opcode*> > LibrarySymbols::generateCode()
{
    map<int, vector<Opcode*> > rval;
    
    for (int i = 0; table[i].name != ""; ++i)
    {
        int var = table[i].var;
        string name = table[i].name;
        bool isIndexed = table[i].numindex > 1;
        Function* function = getFunction(name);
        int label = function->getLabel();
        
        switch(table[i].setorget)
        {
        case GETTER:
            if(isIndexed)
	            rval[label] = getIndexedVariable(refVar, function, var);
            else
                rval[label] = getVariable(refVar, function, var);
                
            break;
            
        case SETTER:
        {
            if(isIndexed)
            {
                rval[label] = setIndexedVariable(refVar, function, var);
            }
            else
            {
                if(table[i].params[1] == ZVARTYPEID_BOOL)
                {
                    rval[label] = setBoolVariable(refVar, function, var);
                }
                else
                {
                    rval[label] = setVariable(refVar, function, var);
                }
            }
            
            break;
        }
        }
    }
    
    return rval;
}

LibrarySymbols::~LibrarySymbols()
{
    return;
}

GlobalSymbols GlobalSymbols::singleton;

static AccessorTable GlobalTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "Rand",                   ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Quit",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Waitframe",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Waitdraw",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Trace",                  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TraceB",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_BOOL,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TraceS",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TraceNL",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ClearTrace",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TraceToBase",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,	    ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     -1,    				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,   					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "Sin",                    ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Cos",                    ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Tan",                    ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ArcTan",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ArcSin",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ArcCos",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "RadianSin",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "RadianCos",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "RadianTan",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Max",                    ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Min",                    ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Pow",                    ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "InvPow",                 ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Factorial",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Abs",                    ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Log10",                  ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Ln",                     ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Sqrt",                   ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
//  { "CalculateSpline",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
//  { "CollisionRect",          ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
//  { "CollisionBox",           ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CopyTile",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SwapTile",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ClearTile",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScriptRAM",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScriptRAM",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetGlobalRAM",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetGlobalRAM",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetColorBuffer",         ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,	    ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,   					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "SetDepthBuffer",         ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,	    ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,   					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "GetColorBuffer",         ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,	    ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,   					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "GetDepthBuffer",         ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,	    ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,     				   -1,   					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "SizeOfArray",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SizeOfArrayBool",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_BOOL,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SizeOfArrayFFC",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_FFC,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SizeOfArrayItem",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_ITEM,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SizeOfArrayLWeapon",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_LWPN,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SizeOfArrayEWeapon",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_EWPN,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SizeOfArrayNPC",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_NPC,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SizeOfArrayItemdata",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_ITEMCLASS,        -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "OverlayTile",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

GlobalSymbols::GlobalSymbols()
{
    table = GlobalTable;
    refVar = NUL;
}

map<int, vector<Opcode*> > GlobalSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval;
    int id;
    //int Rand(int maxval)
    {
	    Function* function = functions["Rand"];
        int label = function->getLabel();
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
	    Function* function = functions["Quit"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OQuit());
        rval[label]=code;
    }
    //void Waitframe()
    {
	    Function* function = functions["Waitframe"];
        int label = function->getLabel();
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
	    Function* function = functions["Waitdraw"];
        int label = function->getLabel();
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
	    Function* function = functions["Trace"];
        int label = function->getLabel();
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
	    Function* function = functions["TraceB"];
        int label = function->getLabel();
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
	    Function* function = functions["TraceS"];
        int label = function->getLabel();
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
	    Function* function = functions["TraceNL"];
        int label = function->getLabel();
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
	    Function* function = functions["ClearTrace"];
        int label = function->getLabel();
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
	    Function* function = functions["TraceToBase"];
        int label = function->getLabel();
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
	    Function* function = functions["Sin"];
        int label = function->getLabel();
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
	    Function* function = functions["RadianSin"];
        int label = function->getLabel();
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
	    Function* function = functions["ArcSin"];
        int label = function->getLabel();
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
	    Function* function = functions["Cos"];
        int label = function->getLabel();
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
	    Function* function = functions["RadianCos"];
        int label = function->getLabel();
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
	    Function* function = functions["ArcCos"];
        int label = function->getLabel();
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
	    Function* function = functions["Tan"];
        int label = function->getLabel();
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
	    Function* function = functions["ArcTan"];
        int label = function->getLabel();
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
	    Function* function = functions["RadianTan"];
        int label = function->getLabel();
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
	    Function* function = functions["Max"];
        int label = function->getLabel();
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
	    Function* function = functions["Min"];
        int label = function->getLabel();
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
	    Function* function = functions["Pow"];
        int label = function->getLabel();
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
	    Function* function = functions["InvPow"];
        int label = function->getLabel();
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
	    Function* function = functions["Factorial"];
        int label = function->getLabel();
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
	    Function* function = functions["Abs"];
        int label = function->getLabel();
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
	    Function* function = functions["Log10"];
        int label = function->getLabel();
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
	    Function* function = functions["Ln"];
        int label = function->getLabel();
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
	    Function* function = functions["Sqrt"];
        int label = function->getLabel();
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
	    Function* function = functions["CopyTile"];
        int label = function->getLabel();
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
    //int SwapTile(int first, int second)
    {
	    Function* function = functions["SwapTile"];
        int label = function->getLabel();
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
    
    //int SwapTile(int first, int second)
    {
	    Function* function = functions["OverlayTile"];
        int label = function->getLabel();
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
    
    //void ClearTile(int tile)
    {
	    Function* function = functions["ClearTile"];
        int label = function->getLabel();
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
	    Function* function = functions["GetGlobalRAM"];
        int label = function->getLabel();
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
	    Function* function = functions["SetGlobalRAM"];
        int label = function->getLabel();
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
	    Function* function = functions["GetScriptRAM"];
        int label = function->getLabel();
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
	    Function* function = functions["SetScriptRAM"];
        int label = function->getLabel();
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
	    Function* function = functions["SetColorBuffer"];
        int label = function->getLabel();
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
	    Function* function = functions["SetDepthBuffer"];
        int label = function->getLabel();
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
	    Function* function = functions["GetColorBuffer"];
        int label = function->getLabel();
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
	    Function* function = functions["GetDepthBuffer"];
        int label = function->getLabel();
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
	    Function* function = functions["SizeOfArray"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArraySize(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int SizeOfArrayFFC(ffc *ptr)
    {
	    Function* function = functions["SizeOfArrayFFC"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArraySizeF(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
     //int SizeOfArrayNPC(npc *ptr)
    {
	    Function* function = functions["SizeOfArrayNPC"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArraySizeN(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    } 
    
    //int SizeOfArrayBool(bool *ptr)
    {
	    Function* function = functions["SizeOfArrayBool"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArraySizeB(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    } //int SizeOfArrayItem(item *ptr)
    {
	    Function* function = functions["SizeOfArrayItem"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArraySizeI(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    } //int SizeOfArrayItemdata(itemdata *ptr)
    {
	    Function* function = functions["SizeOfArrayItemdata"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArraySizeID(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    } //int SizeOfArrayLWeapon(lweapon *ptr)
    {
	    Function* function = functions["SizeOfArrayLWeapon"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArraySizeL(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    } //int SizeOfArrayEWeapon(eweapon *ptr)
    {
	    Function* function = functions["SizeOfArrayEWeapon"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OArraySizeE(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    return rval;
}

FFCSymbols FFCSymbols::singleton = FFCSymbols();

static AccessorTable FFCTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getData",                ZVARTYPEID_FLOAT,         GETTER,       DATA,                 1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setData",                ZVARTYPEID_VOID,          SETTER,       DATA,                 1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getScript",              ZVARTYPEID_FLOAT,         GETTER,       FFSCRIPT,             1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setScript",              ZVARTYPEID_VOID,          SETTER,       FFSCRIPT,             1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       FCSET,                1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCSet",                ZVARTYPEID_VOID,          SETTER,       FCSET,                1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDelay",               ZVARTYPEID_FLOAT,         GETTER,       DELAY,                1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDelay",               ZVARTYPEID_VOID,          SETTER,       DELAY,                1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getX",                   ZVARTYPEID_FLOAT,         GETTER,       FX,                   1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ZVARTYPEID_VOID,          SETTER,       FX,                   1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ZVARTYPEID_FLOAT,         GETTER,       FY,                   1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ZVARTYPEID_VOID,          SETTER,       FY,                   1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getVx",                  ZVARTYPEID_FLOAT,         GETTER,       XD,                   1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setVx",                  ZVARTYPEID_VOID,          SETTER,       XD,                   1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getVy",                  ZVARTYPEID_FLOAT,         GETTER,       YD,                   1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setVy",                  ZVARTYPEID_VOID,          SETTER,       YD,                   1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAx",                  ZVARTYPEID_FLOAT,         GETTER,       XD2,                  1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAx",                  ZVARTYPEID_VOID,          SETTER,       XD2,                  1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAy",                  ZVARTYPEID_FLOAT,         GETTER,       YD2,                  1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAy",                  ZVARTYPEID_VOID,          SETTER,       YD2,                  1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //{ "WasTriggered",           ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlags[]",             ZVARTYPEID_BOOL,          GETTER,       FFFLAGSD,             2,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlags[]",             ZVARTYPEID_VOID,          SETTER,       FFFLAGSD,             2,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,         ZVARTYPEID_BOOL,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       FFTWIDTH,             1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       FFTWIDTH,             1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       FFTHEIGHT,            1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       FFTHEIGHT,            1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getEffectWidth",         ZVARTYPEID_FLOAT,         GETTER,       FFCWIDTH,             1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEffectWidth",         ZVARTYPEID_VOID,          SETTER,       FFCWIDTH,             1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getEffectHeight",        ZVARTYPEID_FLOAT,         GETTER,       FFCHEIGHT,            1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEffectHeight",        ZVARTYPEID_VOID,          SETTER,       FFCHEIGHT,            1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLink",                ZVARTYPEID_FLOAT,         GETTER,       FFLINK,               1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLink",                ZVARTYPEID_VOID,          SETTER,       FFLINK,               1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       FFMISCD,              16,     {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       FFMISCD,              16,     {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInitD[]",             ZVARTYPEID_FLOAT,         GETTER,       FFINITDD,             8,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInitD[]",             ZVARTYPEID_VOID,          SETTER,       FFINITDD,             8,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //{ "getD[]",                ZVARTYPEID_FLOAT,         GETTER,       FFDD,                 8,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //{ "setD[]",	               ZVARTYPEID_VOID,          SETTER,       FFDD,                 8,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //get or set ffc id with this->ID. useful for determining which ffc is active. 
    { "getID",                ZVARTYPEID_FLOAT,         GETTER,       FFCID,               1,      {  ZVARTYPEID_FFC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setID",                   ZVARTYPEID_VOID,          SETTER,       FFCID,                   1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   
    { "ChangeFFCScript",                   ZVARTYPEID_VOID,          SETTER,       FFCID,                   1,      {  ZVARTYPEID_FFC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
       { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

FFCSymbols::FFCSymbols()
{
    table = FFCTable;
    refVar = REFFFC;
}

map<int, vector<Opcode *> > FFCSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::generateCode();
	
	
//void ChangeFFCScript(ffc, int)
    {
	    Function* function = functions["ChangeFFCScript"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OChangeFFCScriptRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //bool WasTriggered(ffc)
    /*{
      Function* function = functions["WasTriggered"];
    	int label = function->getLabel();
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
    { "getX",                   ZVARTYPEID_FLOAT,         GETTER,       LINKX,                1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ZVARTYPEID_VOID,          SETTER,       LINKX,                1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ZVARTYPEID_FLOAT,         GETTER,       LINKY,                1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ZVARTYPEID_VOID,          SETTER,       LINKY,                1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getZ",                   ZVARTYPEID_FLOAT,         GETTER,       LINKZ,                1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setZ",                   ZVARTYPEID_VOID,          SETTER,       LINKZ,                1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJump",                ZVARTYPEID_FLOAT,         GETTER,       LINKJUMP,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setJump",                ZVARTYPEID_VOID,          SETTER,       LINKJUMP,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDir",                 ZVARTYPEID_FLOAT,         GETTER,       LINKDIR,              1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDir",                 ZVARTYPEID_VOID,          SETTER,       LINKDIR,              1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitDir",              ZVARTYPEID_FLOAT,         GETTER,       LINKHITDIR,           1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitDir",              ZVARTYPEID_VOID,          SETTER,       LINKHITDIR,           1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getSwordJinx",           ZVARTYPEID_FLOAT,         GETTER,       LINKSWORDJINX,        1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setSwordJinx",           ZVARTYPEID_VOID,          SETTER,       LINKSWORDJINX,        1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getItemJinx",            ZVARTYPEID_FLOAT,         GETTER,       LINKITEMJINX,         1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setItemJinx",            ZVARTYPEID_VOID,          SETTER,       LINKITEMJINX,         1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHP",                  ZVARTYPEID_FLOAT,         GETTER,       LINKHP,               1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHP",                  ZVARTYPEID_VOID,          SETTER,       LINKHP,               1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMP",                  ZVARTYPEID_FLOAT,         GETTER,       LINKMP,               1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMP",                  ZVARTYPEID_VOID,          SETTER,       LINKMP,               1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMaxHP",               ZVARTYPEID_FLOAT,         GETTER,       LINKMAXHP,            1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMaxHP",               ZVARTYPEID_VOID,          SETTER,       LINKMAXHP,            1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMaxMP",               ZVARTYPEID_FLOAT,         GETTER,       LINKMAXMP,            1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMaxMP",               ZVARTYPEID_VOID,          SETTER,       LINKMAXMP,            1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAction",              ZVARTYPEID_FLOAT,         GETTER,       LINKACTION,           1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAction",              ZVARTYPEID_VOID,          SETTER,       LINKACTION,           1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHeldItem",            ZVARTYPEID_FLOAT,         GETTER,       LINKHELD,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHeldItem",            ZVARTYPEID_VOID,          SETTER,       LINKHELD,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Warp",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PitWarp",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputStart",          ZVARTYPEID_BOOL,          GETTER,       INPUTSTART,           1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputStart",          ZVARTYPEID_VOID,          SETTER,       INPUTSTART,           1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputMap",            ZVARTYPEID_BOOL,          GETTER,       INPUTMAP,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputMap",            ZVARTYPEID_VOID,          SETTER,       INPUTMAP,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputUp",             ZVARTYPEID_BOOL,          GETTER,       INPUTUP,              1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputUp",             ZVARTYPEID_VOID,          SETTER,       INPUTUP,              1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputDown",           ZVARTYPEID_BOOL,          GETTER,       INPUTDOWN,            1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputDown",           ZVARTYPEID_VOID,          SETTER,       INPUTDOWN,            1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputLeft",           ZVARTYPEID_BOOL,          GETTER,       INPUTLEFT,            1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputLeft",           ZVARTYPEID_VOID,          SETTER,       INPUTLEFT,            1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputRight",          ZVARTYPEID_BOOL,          GETTER,       INPUTRIGHT,           1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputRight",          ZVARTYPEID_VOID,          SETTER,       INPUTRIGHT,           1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputA",              ZVARTYPEID_BOOL,          GETTER,       INPUTA,               1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputA",              ZVARTYPEID_VOID,          SETTER,       INPUTA,               1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputB",              ZVARTYPEID_BOOL,          GETTER,       INPUTB,               1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputB",              ZVARTYPEID_VOID,          SETTER,       INPUTB,               1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputL",              ZVARTYPEID_BOOL,          GETTER,       INPUTL,               1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputL",              ZVARTYPEID_VOID,          SETTER,       INPUTL,               1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputR",              ZVARTYPEID_BOOL,          GETTER,       INPUTR,               1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputR",              ZVARTYPEID_VOID,          SETTER,       INPUTR,               1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputEx1",            ZVARTYPEID_BOOL,          GETTER,       INPUTEX1,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputEx1",            ZVARTYPEID_VOID,          SETTER,       INPUTEX1,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputEx2",            ZVARTYPEID_BOOL,          GETTER,       INPUTEX2,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputEx2",            ZVARTYPEID_VOID,          SETTER,       INPUTEX2,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputEx3",            ZVARTYPEID_BOOL,          GETTER,       INPUTEX3,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputEx3",            ZVARTYPEID_VOID,          SETTER,       INPUTEX3,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputEx4",            ZVARTYPEID_BOOL,          GETTER,       INPUTEX4,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputEx4",            ZVARTYPEID_VOID,          SETTER,       INPUTEX4,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressStart",          ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSSTART,      1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressStart",          ZVARTYPEID_VOID,          SETTER,       INPUTPRESSSTART,      1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressUp",             ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSUP,         1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressUp",             ZVARTYPEID_VOID,          SETTER,       INPUTPRESSUP,         1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressDown",           ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSDOWN,       1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressDown",           ZVARTYPEID_VOID,          SETTER,       INPUTPRESSDOWN,       1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressLeft",           ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSLEFT,       1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressLeft",           ZVARTYPEID_VOID,          SETTER,       INPUTPRESSLEFT,       1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressRight",          ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSRIGHT,      1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressRight",          ZVARTYPEID_VOID,          SETTER,       INPUTPRESSRIGHT,      1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressA",              ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSA,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressA",              ZVARTYPEID_VOID,          SETTER,       INPUTPRESSA,          1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressB",              ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSB,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressB",              ZVARTYPEID_VOID,          SETTER,       INPUTPRESSB,          1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressL",              ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSL,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressL",              ZVARTYPEID_VOID,          SETTER,       INPUTPRESSL,          1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressR",              ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSR,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressR",              ZVARTYPEID_VOID,          SETTER,       INPUTPRESSR,          1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressEx1",            ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSEX1,        1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressEx1",            ZVARTYPEID_VOID,          SETTER,       INPUTPRESSEX1,        1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressEx2",            ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSEX2,        1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressEx2",            ZVARTYPEID_VOID,          SETTER,       INPUTPRESSEX2,        1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressEx3",            ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSEX3,        1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressEx3",            ZVARTYPEID_VOID,          SETTER,       INPUTPRESSEX3,        1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressEx4",            ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSEX4,        1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressEx4",            ZVARTYPEID_VOID,          SETTER,       INPUTPRESSEX4,        1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputMouseX",         ZVARTYPEID_FLOAT,         GETTER,       INPUTMOUSEX,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputMouseX",         ZVARTYPEID_VOID,          SETTER,       INPUTMOUSEX,          1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputMouseY",         ZVARTYPEID_FLOAT,         GETTER,       INPUTMOUSEY,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputMouseY",         ZVARTYPEID_VOID,          SETTER,       INPUTMOUSEY,          1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputMouseZ",         ZVARTYPEID_FLOAT,         GETTER,       INPUTMOUSEZ,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputMouseZ",         ZVARTYPEID_VOID,          SETTER,       INPUTMOUSEZ,          1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputMouseB",         ZVARTYPEID_FLOAT,         GETTER,       INPUTMOUSEB,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputMouseB",         ZVARTYPEID_VOID,          SETTER,       INPUTMOUSEB,          1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getItem[]",              ZVARTYPEID_BOOL,          GETTER,       LINKITEMD,            256,    {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setItem[]",              ZVARTYPEID_VOID,          SETTER,       LINKITEMD,            256,    {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         ZVARTYPEID_BOOL,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitWidth",            ZVARTYPEID_FLOAT,         GETTER,       LINKHXSZ,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitWidth",            ZVARTYPEID_VOID,          SETTER,       LINKHXSZ,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitHeight",           ZVARTYPEID_FLOAT,         GETTER,       LINKHYSZ,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitHeight",           ZVARTYPEID_VOID,          SETTER,       LINKHYSZ,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitZHeight",          ZVARTYPEID_FLOAT,         GETTER,       LINKHZSZ,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitZHeight",          ZVARTYPEID_VOID,          SETTER,       LINKHZSZ,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       LINKTXSZ,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       LINKTXSZ,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       LINKTYSZ,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       LINKTYSZ,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawXOffset",         ZVARTYPEID_FLOAT,         GETTER,       LINKXOFS,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawXOffset",         ZVARTYPEID_VOID,          SETTER,       LINKXOFS,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawYOffset",         ZVARTYPEID_FLOAT,         GETTER,       LINKYOFS,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawYOffset",         ZVARTYPEID_VOID,          SETTER,       LINKYOFS,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawZOffset",         ZVARTYPEID_FLOAT,         GETTER,       LINKZOFS,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawZOffset",         ZVARTYPEID_VOID,          SETTER,       LINKZOFS,             1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitXOffset",          ZVARTYPEID_FLOAT,         GETTER,       LINKHXOFS,            1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitXOffset",          ZVARTYPEID_VOID,          SETTER,       LINKHXOFS,            1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitYOffset",          ZVARTYPEID_FLOAT,         GETTER,       LINKHYOFS,            1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitYOffset",          ZVARTYPEID_VOID,          SETTER,       LINKHYOFS,            1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrunk",               ZVARTYPEID_FLOAT,         GETTER,       LINKDRUNK,            1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrunk",               ZVARTYPEID_VOID,          SETTER,       LINKDRUNK,            1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getEquipment",           ZVARTYPEID_FLOAT,         GETTER,       LINKEQUIP,            1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEquipment",           ZVARTYPEID_VOID,          SETTER,       LINKEQUIP,            1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputAxisUp",         ZVARTYPEID_BOOL,          GETTER,       INPUTAXISUP,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputAxisUp",         ZVARTYPEID_VOID,          SETTER,       INPUTAXISUP,          1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputAxisDown",       ZVARTYPEID_BOOL,          GETTER,       INPUTAXISDOWN,        1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputAxisDown",       ZVARTYPEID_VOID,          SETTER,       INPUTAXISDOWN,        1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputAxisLeft",       ZVARTYPEID_BOOL,          GETTER,       INPUTAXISLEFT,        1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputAxisLeft",       ZVARTYPEID_VOID,          SETTER,       INPUTAXISLEFT,        1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInputAxisRight",      ZVARTYPEID_BOOL,          GETTER,       INPUTAXISRIGHT,       1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInputAxisRight",      ZVARTYPEID_VOID,          SETTER,       INPUTAXISRIGHT,       1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressAxisUp",         ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSAXISUP,     1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressAxisUp",         ZVARTYPEID_VOID,          SETTER,       INPUTPRESSAXISUP,     1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressAxisDown",       ZVARTYPEID_BOOL,   	    GETTER,       INPUTPRESSAXISDOWN,   1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressAxisDown",       ZVARTYPEID_VOID,   	    SETTER,       INPUTPRESSAXISDOWN,   1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressAxisLeft",	      ZVARTYPEID_BOOL,   	    GETTER,       INPUTPRESSAXISLEFT,   1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressAxisLeft",       ZVARTYPEID_VOID,   	    SETTER,       INPUTPRESSAXISLEFT,   1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressAxisRight",      ZVARTYPEID_BOOL,   	    GETTER,       INPUTPRESSAXISRIGHT,  1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressAxisRight",      ZVARTYPEID_VOID,   	    SETTER,       INPUTPRESSAXISRIGHT,  1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInvisible",           ZVARTYPEID_FLOAT,         GETTER,       LINKINVIS,            1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInvisible",           ZVARTYPEID_VOID,          SETTER,       LINKINVIS,            1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection",       ZVARTYPEID_FLOAT,         GETTER,       LINKINVINC,           1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection",       ZVARTYPEID_VOID,          SETTER,       LINKINVINC,           1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       LINKMISCD,            32,     {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       LINKMISCD,            32,     {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLadderX",             ZVARTYPEID_FLOAT,         GETTER,       LINKLADDERX,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLadderY",             ZVARTYPEID_FLOAT,         GETTER,       LINKLADDERY,          1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",                ZVARTYPEID_FLOAT,         GETTER,       LINKTILE,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTile",                ZVARTYPEID_VOID,          SETTER,       LINKTILE,             1,      {  ZVARTYPEID_LINK,         ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlip",                ZVARTYPEID_FLOAT,         GETTER,       LINKFLIP,             1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlip",                ZVARTYPEID_VOID,          SETTER,       LINKFLIP,             1,      {  ZVARTYPEID_LINK,         ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPressMap",            ZVARTYPEID_BOOL,          GETTER,       INPUTPRESSMAP,        1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPressMap",            ZVARTYPEID_VOID,          SETTER,       INPUTPRESSMAP,        1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SelectAWeapon",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SelectBWeapon",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
     { "setInvFrames",               ZVARTYPEID_VOID,          SETTER,       LINKINVFRAME,             1,      {  ZVARTYPEID_LINK,         ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInvFrames",            ZVARTYPEID_FLOAT,          GETTER,       LINKINVFRAME,         1,      {  ZVARTYPEID_LINK,          -1,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInvFlicker",                ZVARTYPEID_VOID,          SETTER,       LINKCANFLICKER,             1,      {  ZVARTYPEID_LINK,         ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInvFlicker",            ZVARTYPEID_BOOL,          GETTER,       LINKCANFLICKER,         1,      {  ZVARTYPEID_LINK,          -1,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHurtSound",               ZVARTYPEID_VOID,          SETTER,       LINKHURTSFX,             1,      {  ZVARTYPEID_LINK,         ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHurtSound",            ZVARTYPEID_FLOAT,          GETTER,       LINKHURTSFX,         1,      {  ZVARTYPEID_LINK,          -1,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
  
// what are these supposed to do? Is Get/SetItemA sufficient?
/*    { "getUsingItem",            ZVARTYPEID_FLOAT,         GETTER,       LINKUSINGITEM,         1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setUsingItem",            ZVARTYPEID_VOID,          SETTER,       LINKUSINGITEM,         1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //Not sure what the 'attack' var is used for at present, but that is what etItemA returned, so I'm renaming these to ->Attack
    { "getAttack",            ZVARTYPEID_FLOAT,         GETTER,       LINKUSINGITEMA,         1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAttack",            ZVARTYPEID_VOID,          SETTER,       LINKUSINGITEMA,         1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
*/    
     { "SetItemA",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         -1,     -1,    -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetItemB",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,     -1,    -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   { "SetItemSlot",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   
//    { "setItemB",            ZVARTYPEID_VOID,          SETTER,       LINKITEMB,         1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
//    { "setItemA",                ZVARTYPEID_VOID,          SETTER,       LINKITEMA,             1,      {  ZVARTYPEID_LINK,         ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
//    { "getItemB",            ZVARTYPEID_FLOAT,          GETTER,       LINKITEMB,         1,      {  ZVARTYPEID_LINK,          -1,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
//    { "getItemA",                ZVARTYPEID_FLOAT,          GETTER,       LINKITEMA,             1,      {  ZVARTYPEID_LINK,         -1,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getEaten",            ZVARTYPEID_FLOAT,         GETTER,       LINKEATEN,         1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEaten",            ZVARTYPEID_VOID,          SETTER,       LINKEATEN,         1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDiagonal",           ZVARTYPEID_BOOL,          GETTER,       LINKDIAG,        1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDiagonal",           ZVARTYPEID_VOID,          SETTER,       LINKDIAG,        1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getBigHitbox",           ZVARTYPEID_BOOL,          GETTER,       LINKBIGHITBOX,        1,      {  ZVARTYPEID_LINK,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setBigHitbox",           ZVARTYPEID_VOID,          SETTER,       LINKBIGHITBOX,        1,      {  ZVARTYPEID_LINK,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
};

LinkSymbols::LinkSymbols()
{
    table = LinkSTable;
    refVar = NUL;
}

map<int, vector<Opcode *> > LinkSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::generateCode();
    //Warp(link, int, int)
    {
	    Function* function = functions["Warp"];
        int label = function->getLabel();
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
    
       //void SetItemSlot(link, int item, int slot, int force)
    {
	    Function* function = functions["SetItemSlot"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETITEMSLOT), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //void SetItemA(link, int)
    {
	    Function* function = functions["SetItemA"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OSetRegister(new VarArgument(GAMESETA), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetItemB(link, int)
    {
	    Function* function = functions["SetItemB"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP2));
        first->setLabel(label);
        code.push_back(first);
        //code.push_back(new OPopRegister(new VarArgument(INDEX)));
        code.push_back(new OSetRegister(new VarArgument(GAMESETB), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //PitWarp(link, int, int)
    {
	    Function* function = functions["PitWarp"];
        int label = function->getLabel();
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
	    Function* function = functions["SelectAWeapon"];
        int label = function->getLabel();
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
	    Function* function = functions["SelectBWeapon"];
        int label = function->getLabel();
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
    { "getD[]",                 ZVARTYPEID_FLOAT,         GETTER,       SDD,                  8,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setD[]",                 ZVARTYPEID_VOID,          SETTER,       SDD,                  8,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboD[]",            ZVARTYPEID_FLOAT,         GETTER,       COMBODD,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboD[]",            ZVARTYPEID_VOID,          SETTER,       COMBODD,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboC[]",            ZVARTYPEID_FLOAT,         GETTER,       COMBOCD,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboC[]",            ZVARTYPEID_VOID,          SETTER,       COMBOCD,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboF[]",            ZVARTYPEID_FLOAT,         GETTER,       COMBOFD,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboF[]",            ZVARTYPEID_VOID,          SETTER,       COMBOFD,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboI[]",            ZVARTYPEID_FLOAT,         GETTER,       COMBOID,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboI[]",            ZVARTYPEID_VOID,          SETTER,       COMBOID,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboT[]",            ZVARTYPEID_FLOAT,         GETTER,       COMBOTD,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboT[]",            ZVARTYPEID_VOID,          SETTER,       COMBOTD,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getComboS[]",            ZVARTYPEID_FLOAT,         GETTER,       COMBOSD,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setComboS[]",            ZVARTYPEID_VOID,          SETTER,       COMBOSD,              176,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDoor[]",              ZVARTYPEID_FLOAT,         GETTER,       SCRDOORD,               4,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDoor[]",              ZVARTYPEID_VOID,          SETTER,       SCRDOORD,               4,    {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getState[]",             ZVARTYPEID_BOOL,          GETTER,       SCREENSTATED,          32,    {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setState[]",             ZVARTYPEID_VOID,          SETTER,       SCREENSTATED,          32,    {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         ZVARTYPEID_BOOL,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLit",                 ZVARTYPEID_BOOL,          GETTER,       LIT,                    1,    {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLit",                 ZVARTYPEID_VOID,          SETTER,       LIT,                    1,    {  ZVARTYPEID_SCREEN,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWavy",                ZVARTYPEID_FLOAT,         GETTER,       WAVY,                   1,    {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setWavy",                ZVARTYPEID_VOID,          SETTER,       WAVY,                   1,    {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getQuake",               ZVARTYPEID_FLOAT,         GETTER,       QUAKE,                  1,    {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setQuake",               ZVARTYPEID_VOID,          SETTER,       QUAKE,                1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "NumItems",               ZVARTYPEID_FLOAT,         GETTER,       ITEMCOUNT,            1,      {  ZVARTYPEID_SCREEN,       -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadItem",               ZVARTYPEID_ITEM,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CreateItem",             ZVARTYPEID_ITEM,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadFFC",                ZVARTYPEID_FFC,           FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "NumNPCs",                ZVARTYPEID_FLOAT,         GETTER,       NPCCOUNT,             1,      {  ZVARTYPEID_SCREEN,       -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadNPC",                ZVARTYPEID_NPC,           FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CreateNPC",              ZVARTYPEID_NPC,           FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ClearSprites",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Rectangle",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                      } },
    { "Circle",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "Arc",                    ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "Ellipse",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_BOOL,      ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "Line",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "Spline",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "PutPixel",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawCharacter",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawInteger",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawTile",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,      ZVARTYPEID_BOOL,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           } },
    { "DrawCombo",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,      ZVARTYPEID_BOOL,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           } },
    { "Quad",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,  ZVARTYPEID_FLOAT,                         -1,                           -1,                           -1,                           -1,                           } },
    { "Triangle",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,                      -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           } },
    { "Quad3D",                 ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "Triangle3D",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "FastTile",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "FastCombo",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawString",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawLayer",     typeVOID, FUNCTION, 0, 1, ARGS_8(S,F,F,F,F,F,F,F,F) },
    { "DrawScreen",    typeVOID, FUNCTION, 0, 1, ARGS_6(S,F,F,F,F,F,F) },
    { "DrawBitmap",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,       ZVARTYPEID_FLOAT,   ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,    ZVARTYPEID_BOOL,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "DrawBitmapEx",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,        { ZVARTYPEID_SCREEN, ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_FLOAT,ZVARTYPEID_BOOL, -1,                           -1,                           -1,							  } },
    { "SetRenderTarget",        ZVARTYPEID_VOID,		    FUNCTION,	  0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,		  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1							 } },
    { "Message",                ZVARTYPEID_VOID,		    FUNCTION,	   0,                   1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,		  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1,							  -1,							-1							 } },
    { "NumLWeapons",            ZVARTYPEID_FLOAT,         GETTER,       LWPNCOUNT,            1,      {  ZVARTYPEID_SCREEN,       -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadLWeapon",            ZVARTYPEID_LWPN,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CreateLWeapon",          ZVARTYPEID_LWPN,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "NumEWeapons",            ZVARTYPEID_FLOAT,         GETTER,       EWPNCOUNT,            1,      {  ZVARTYPEID_SCREEN,       -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadEWeapon",            ZVARTYPEID_EWPN,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CreateEWeapon",          ZVARTYPEID_EWPN,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "isSolid",                ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetSideWarp",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,     -1,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "SetTileWarp",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,     -1,     -1,                           -1,                          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,							  } },
    { "LayerScreen",            ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LayerMap",               ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlags[]",             ZVARTYPEID_FLOAT,         GETTER,       SCREENFLAGSD,        10,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlags[]",             ZVARTYPEID_VOID,          SETTER,       SCREENFLAGSD,        10,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getEFlags[]",            ZVARTYPEID_FLOAT,         GETTER,       SCREENEFLAGSD,        3,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEFlags[]",            ZVARTYPEID_VOID,          SETTER,       SCREENEFLAGSD,        3,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TriggerSecrets",         ZVARTYPEID_VOID,          FUNCTION,     0,       		        1,      {  ZVARTYPEID_SCREEN,        -1,    					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getRoomType",            ZVARTYPEID_FLOAT,         GETTER,       ROOMTYPE,             1,      {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //{ "setRoomType",            ZVARTYPEID_VOID,          SETTER,       SCREENTYPE,           1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getRoomData",            ZVARTYPEID_FLOAT,         GETTER,       ROOMDATA,             1,      {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setRoomData",            ZVARTYPEID_VOID,          SETTER,       ROOMDATA,             1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMovingBlockX",        ZVARTYPEID_FLOAT,         GETTER,       PUSHBLOCKX,           1,      {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMovingBlockX",        ZVARTYPEID_VOID,          SETTER,       PUSHBLOCKX,           1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMovingBlockY",        ZVARTYPEID_FLOAT,         GETTER,       PUSHBLOCKY,           1,      {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMovingBlockY",        ZVARTYPEID_VOID,          SETTER,       PUSHBLOCKY,           1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMovingBlockCombo",    ZVARTYPEID_FLOAT,         GETTER,       PUSHBLOCKCOMBO,       1,      {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMovingBlockCombo",    ZVARTYPEID_VOID,          SETTER,       PUSHBLOCKCOMBO,       1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMovingBlockCSet",     ZVARTYPEID_FLOAT,         GETTER,       PUSHBLOCKCSET,        1,      {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMovingBlockCSet",     ZVARTYPEID_VOID,          SETTER,       PUSHBLOCKCSET,        1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getUnderCombo",          ZVARTYPEID_FLOAT,         GETTER,       UNDERCOMBO,           1,      {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setUnderCombo",          ZVARTYPEID_VOID,          SETTER,       UNDERCOMBO,           1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getUnderCSet",           ZVARTYPEID_FLOAT,         GETTER,       UNDERCSET,            1,      {  ZVARTYPEID_SCREEN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setUnderCSet",           ZVARTYPEID_VOID,          SETTER,       UNDERCSET,            1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetSideWarpDMap",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetSideWarpScreen",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetSideWarpType",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetTileWarpDMap",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetTileWarpScreen",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetTileWarpType",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ZapIn",        		ZVARTYPEID_VOID,          FUNCTION,     0,       			1,      {  ZVARTYPEID_SCREEN,        -1,    					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ZapOut",         	ZVARTYPEID_VOID,          FUNCTION,     0,       		        1,      {  ZVARTYPEID_SCREEN,        -1,    					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "WavyIn",         	ZVARTYPEID_VOID,          FUNCTION,     0,       		        1,      {  ZVARTYPEID_SCREEN,        -1,    					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "WavyOut",         	ZVARTYPEID_VOID,          FUNCTION,     0,       		        1,      {  ZVARTYPEID_SCREEN,        -1,    					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "OpeningWipe",         	ZVARTYPEID_VOID,          FUNCTION,     0,       		        1,      {  ZVARTYPEID_SCREEN,        -1,    					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "CreateLWeaponDx",              ZVARTYPEID_LWPN,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,        ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Polygon",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,		 ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,  ZVARTYPEID_FLOAT,                         -1,                           -1,                           -1,                           -1,                           } },
    { "TriggerSecret",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_SCREEN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
 
    
    
    
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                                -1,                              -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};


ScreenSymbols::ScreenSymbols()
{
    table = ScreenTable;
    refVar = NUL;
}

map<int, vector<Opcode *> > ScreenSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::generateCode();
    //item LoadItem(screen, int)
    {
	    Function* function = functions["LoadItem"];
        int label = function->getLabel();
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
	    Function* function = functions["CreateItem"];
        
        int label = function->getLabel();
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
	    Function* function = functions["LoadFFC"];
        
        int label = function->getLabel();
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
	    Function* function = functions["LoadNPC"];
        int label = function->getLabel();
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
	    Function* function = functions["CreateNPC"];
        
        int label = function->getLabel();
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
	    Function* function = functions["LoadLWeapon"];
        int label = function->getLabel();
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
	    Function* function = functions["CreateLWeapon"];
        
        int label = function->getLabel();
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
    
    //lweapon CreateLWeaponDX(screen, int type, int itemid)
    {
	    Function* function = functions["CreateLWeaponDx"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(CREATELWPNDX)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
     
    //ewpn LoadEWeapon(screen, int)
    {
	    Function* function = functions["LoadEWeapon"];
        int label = function->getLabel();
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
	    Function* function = functions["CreateEWeapon"];
        
        int label = function->getLabel();
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
	    Function* function = functions["ClearSprites"];
        
        int label = function->getLabel();
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
	    Function* function = functions["Rectangle"];
        int label = function->getLabel();
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
	    Function* function = functions["Circle"];
        int label = function->getLabel();
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
	    Function* function = functions["Arc"];
        int label = function->getLabel();
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
	    Function* function = functions["Ellipse"];
        int label = function->getLabel();
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
	    Function* function = functions["Line"];
        int label = function->getLabel();
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
	    Function* function = functions["Spline"];
        int label = function->getLabel();
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
	    Function* function = functions["PutPixel"];
        int label = function->getLabel();
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
	    Function* function = functions["DrawCharacter"];
        int label = function->getLabel();
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
	    Function* function = functions["DrawInteger"];
        int label = function->getLabel();
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
	    Function* function = functions["DrawTile"];
        int label = function->getLabel();
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
	    Function* function = functions["DrawCombo"];
        int label = function->getLabel();
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
	    Function* function = functions["Quad"];
        int label = function->getLabel();
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
    //void Quad(screen, float, float, float, float, float, float, float, float, float)
    {
	    Function* function = functions["Polygon"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPolygonRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(6, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void Triangle(screen, float, float, float, float, float, float, float, float, float)
    {
	    Function* function = functions["Triangle"];
        int label = function->getLabel();
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
	    Function* function = functions["Quad3D"];
        int label = function->getLabel();
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
	    Function* function = functions["Triangle3D"];
        int label = function->getLabel();
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
	    Function* function = functions["FastTile"];
        int label = function->getLabel();
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
	    Function* function = functions["FastCombo"];
        int label = function->getLabel();
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
	    Function* function = functions["DrawString"];
        int label = function->getLabel();
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
	    Function* function = functions["DrawLayer"];
        int label = function->getLabel();
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
	    Function* function = functions["DrawScreen"];
        int label = function->getLabel();
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
	    Function* function = functions["DrawBitmap"];
        int label = function->getLabel();
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
    
    //void DrawBitmapEx(screen, float, float, float, float, float, float, float, float, float, float, bool)
    {
	    Function* function = functions["DrawBitmapEx"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new ODrawBitmapExRegister();
        first->setLabel(label);
        code.push_back(first);
        POP_ARGS(16, EXP2);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //void SetRenderTarget(bitmap)
    {
	    Function* function = functions["SetRenderTarget"];
        int label = function->getLabel();
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
	    Function* function = functions["Message"];
        int label = function->getLabel();
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
	    Function* function = functions["isSolid"];
        int label = function->getLabel();
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
	    Function* function = functions["SetSideWarp"];
        int label = function->getLabel();
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
	    Function* function = functions["SetTileWarp"];
        int label = function->getLabel();
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
	    Function* function = functions["LayerScreen"];
        int label = function->getLabel();
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
	    Function* function = functions["LayerMap"];
        int label = function->getLabel();
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
	    Function* function = functions["TriggerSecrets"];
        int label = function->getLabel();
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
    //void ZapIn(screen)
    {
	    Function* function = functions["ZapIn"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OZapIn());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
      

	//void ZapOut(screen)
    {
	    Function* function = functions["ZapOut"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OZapOut());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
     //void OpeningWipe(screen)
    {
	    Function* function = functions["OpeningWipe"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OOpenWipe());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }

	//void WavyIn(screen)
    {
	    Function* function = functions["WavyIn"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OWavyIn());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
            
	//void WavyOut(screen)
    {
	    Function* function = functions["WavyOut"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OWavyOut());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //int GetSideWarpDMap(screen, int)
    {
	    Function* function = functions["GetSideWarpDMap"];
        int label = function->getLabel();
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
	    Function* function = functions["GetSideWarpScreen"];
        int label = function->getLabel();
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
	    Function* function = functions["GetSideWarpType"];
        int label = function->getLabel();
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
	    Function* function = functions["GetTileWarpDMap"];
        int label = function->getLabel();
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
	    Function* function = functions["GetTileWarpScreen"];
        int label = function->getLabel();
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
	    Function* function = functions["GetTileWarpType"];
        int label = function->getLabel();
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
    
    
//void TriggerSecret(screen, int)
    {
	    Function* function = functions["TriggerSecret"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OTriggerSecretRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    return rval;
}

ItemSymbols ItemSymbols::singleton = ItemSymbols();

static AccessorTable itemTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getX",                   ZVARTYPEID_FLOAT,         GETTER,       ITEMX,                1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ZVARTYPEID_VOID,          SETTER,       ITEMX,                1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ZVARTYPEID_FLOAT,         GETTER,       ITEMY,                1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ZVARTYPEID_VOID,          SETTER,       ITEMY,                1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getZ",                   ZVARTYPEID_FLOAT,         GETTER,       ITEMZ,                1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setZ",                   ZVARTYPEID_VOID,          SETTER,       ITEMZ,                1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJump",                ZVARTYPEID_FLOAT,         GETTER,       ITEMJUMP,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setJump",                ZVARTYPEID_VOID,          SETTER,       ITEMJUMP,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawStyle",           ZVARTYPEID_FLOAT,         GETTER,       ITEMDRAWTYPE,         1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawStyle",           ZVARTYPEID_VOID,          SETTER,       ITEMDRAWTYPE,         1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getID",                  ZVARTYPEID_FLOAT,         GETTER,       ITEMID,               1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setID",                  ZVARTYPEID_VOID,          SETTER,       ITEMID,               1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",                ZVARTYPEID_FLOAT,         GETTER,       ITEMTILE,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTile",                ZVARTYPEID_VOID,          SETTER,       ITEMTILE,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalTile",        ZVARTYPEID_FLOAT,         GETTER,       ITEMOTILE,            1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "setOriginalTile",        ZVARTYPEID_VOID,          SETTER,       ITEMOTILE,            1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                          } },
    { "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       ITEMCSET,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCSet",                ZVARTYPEID_VOID,          SETTER,       ITEMCSET,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlashCSet",           ZVARTYPEID_FLOAT,         GETTER,       ITEMFLASHCSET,        1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlashCSet",           ZVARTYPEID_VOID,          SETTER,       ITEMFLASHCSET,        1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getNumFrames",           ZVARTYPEID_FLOAT,         GETTER,       ITEMFRAMES,           1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setNumFrames",           ZVARTYPEID_VOID,          SETTER,       ITEMFRAMES,           1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFrame",               ZVARTYPEID_FLOAT,         GETTER,       ITEMFRAME,            1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFrame",               ZVARTYPEID_VOID,          SETTER,       ITEMFRAME,            1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getASpeed",              ZVARTYPEID_FLOAT,         GETTER,       ITEMASPEED,           1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setASpeed",              ZVARTYPEID_VOID,          SETTER,       ITEMASPEED,           1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDelay",               ZVARTYPEID_FLOAT,         GETTER,       ITEMDELAY,            1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDelay",               ZVARTYPEID_VOID,          SETTER,       ITEMDELAY,            1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlash",               ZVARTYPEID_BOOL,          GETTER,       ITEMFLASH,            1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlash",               ZVARTYPEID_VOID,          SETTER,       ITEMFLASH,            1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlip",                ZVARTYPEID_FLOAT,         GETTER,       ITEMFLIP,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlip",                ZVARTYPEID_VOID,          SETTER,       ITEMFLIP,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getExtend",              ZVARTYPEID_FLOAT,         GETTER,       ITEMEXTEND,           1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setExtend",              ZVARTYPEID_VOID,          SETTER,       ITEMEXTEND,           1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitWidth",            ZVARTYPEID_FLOAT,         GETTER,       ITEMHXSZ,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitWidth",            ZVARTYPEID_VOID,          SETTER,       ITEMHXSZ,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitHeight",           ZVARTYPEID_FLOAT,         GETTER,       ITEMHYSZ,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitHeight",           ZVARTYPEID_VOID,          SETTER,       ITEMHYSZ,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitZHeight",          ZVARTYPEID_FLOAT,         GETTER,       ITEMHZSZ,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitZHeight",          ZVARTYPEID_VOID,          SETTER,       ITEMHZSZ,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       ITEMTXSZ,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       ITEMTXSZ,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       ITEMTYSZ,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       ITEMTYSZ,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawXOffset",         ZVARTYPEID_FLOAT,         GETTER,       ITEMXOFS,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawXOffset",         ZVARTYPEID_VOID,          SETTER,       ITEMXOFS,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawYOffset",         ZVARTYPEID_FLOAT,         GETTER,       ITEMYOFS,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawYOffset",         ZVARTYPEID_VOID,          SETTER,       ITEMYOFS,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawZOffset",         ZVARTYPEID_FLOAT,         GETTER,       ITEMZOFS,             1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawZOffset",         ZVARTYPEID_VOID,          SETTER,       ITEMZOFS,             1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitXOffset",          ZVARTYPEID_FLOAT,         GETTER,       ITEMHXOFS,            1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitXOffset",          ZVARTYPEID_VOID,          SETTER,       ITEMHXOFS,            1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitYOffset",          ZVARTYPEID_FLOAT,         GETTER,       ITEMHYOFS,            1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitYOffset",          ZVARTYPEID_VOID,          SETTER,       ITEMHYOFS,            1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPickup",              ZVARTYPEID_FLOAT,         GETTER,       ITEMPICKUP,           1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPickup",              ZVARTYPEID_VOID,          SETTER,       ITEMPICKUP,           1,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "isValid",                ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_ITEM,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       ITEMMISCD,            32,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       ITEMMISCD,            32,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

ItemSymbols::ItemSymbols()
{
    table = itemTable;
    refVar = REFITEM;
}

map<int, vector<Opcode *> > ItemSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::generateCode();
    //bool isValid(item)
    {
	    Function* function = functions["isValid"];
        int label = function->getLabel();
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
    { "getFamily",              ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSFAMILY,      1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFamily",              ZVARTYPEID_VOID,          SETTER,       ITEMCLASSFAMILY,      1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLevel",               ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSFAMTYPE,     1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLevel",               ZVARTYPEID_VOID,          SETTER,       ITEMCLASSFAMTYPE,     1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAmount",              ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSAMOUNT,      1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAmount",              ZVARTYPEID_VOID,          SETTER,       ITEMCLASSAMOUNT,      1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMax",                 ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSMAX,         1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMax",                 ZVARTYPEID_VOID,          SETTER,       ITEMCLASSMAX,         1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMaxIncrement",        ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSSETMAX,      1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMaxIncrement",        ZVARTYPEID_VOID,          SETTER,       ITEMCLASSSETMAX,      1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getKeep",                ZVARTYPEID_BOOL,          GETTER,       ITEMCLASSSETGAME,     1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setKeep",                ZVARTYPEID_VOID,          SETTER,       ITEMCLASSSETGAME,     1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCounter",             ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSCOUNTER,     1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCounter",             ZVARTYPEID_VOID,          SETTER,       ITEMCLASSCOUNTER,     1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getUseSound",            ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSUSESOUND,    1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setUseSound",            ZVARTYPEID_VOID,          SETTER,       ITEMCLASSUSESOUND,    1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getID",               ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSID,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   
    { "getPower",               ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSPOWER,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setPower",               ZVARTYPEID_VOID,          SETTER,       ITEMCLASSPOWER,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInitD[]",             ZVARTYPEID_FLOAT,         GETTER,       ITEMCLASSINITDD,      2,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInitD[]",             ZVARTYPEID_VOID,          SETTER,       ITEMCLASSINITDD,      2,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetName",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getModifier",               ZVARTYPEID_FLOAT,         GETTER,       IDATALTM,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setModifier",               ZVARTYPEID_VOID,          SETTER,       IDATALTM,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getScript",               ZVARTYPEID_FLOAT,         GETTER,       IDATASCRIPT,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setScript",               ZVARTYPEID_VOID,          SETTER,       IDATASCRIPT,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getPScript",               ZVARTYPEID_FLOAT,         GETTER,       IDATAPSCRIPT,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setPScript",               ZVARTYPEID_VOID,          SETTER,       IDATAPSCRIPT,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMagicCost",               ZVARTYPEID_FLOAT,         GETTER,       IDATAMAGCOST,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setMagicCost",               ZVARTYPEID_VOID,          SETTER,       IDATAMAGCOST,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMinHearts",               ZVARTYPEID_FLOAT,         GETTER,       IDATAMINHEARTS,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setMinHearts",               ZVARTYPEID_VOID,          SETTER,       IDATAMINHEARTS,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",               ZVARTYPEID_FLOAT,         GETTER,       IDATATILE,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setTile",               ZVARTYPEID_VOID,          SETTER,       IDATATILE,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlash",               ZVARTYPEID_FLOAT,         GETTER,       IDATAMISC,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setFlash",               ZVARTYPEID_VOID,          SETTER,       IDATAMISC,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCSet",               ZVARTYPEID_FLOAT,         GETTER,       IDATACSET,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setCSet",               ZVARTYPEID_VOID,          SETTER,       IDATACSET,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
 //   { "getFrame",               ZVARTYPEID_FLOAT,         GETTER,       IDATAFRAME,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
  //  { "setFrame",               ZVARTYPEID_VOID,          SETTER,       IDATAFRAME,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAFrames",               ZVARTYPEID_FLOAT,         GETTER,       IDATAFRAMES,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setAFrames",               ZVARTYPEID_VOID,          SETTER,       IDATAFRAMES,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getASpeed",               ZVARTYPEID_FLOAT,         GETTER,       IDATAASPEED,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setASpeed",               ZVARTYPEID_VOID,          SETTER,       IDATAASPEED,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDelay",               ZVARTYPEID_FLOAT,         GETTER,       IDATADELAY,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setDelay",               ZVARTYPEID_VOID,          SETTER,       IDATADELAY,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCombine",                ZVARTYPEID_BOOL,          GETTER,       IDATACOMBINE,     1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCombine",                ZVARTYPEID_VOID,          SETTER,       IDATACOMBINE,     1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDowngrade",                ZVARTYPEID_BOOL,          GETTER,       IDATADOWNGRADE,     1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDowngrade",                ZVARTYPEID_VOID,          SETTER,       IDATADOWNGRADE,     1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getKeepOld",                ZVARTYPEID_BOOL,          GETTER,       IDATAKEEPOLD,     1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setKeepOld",                ZVARTYPEID_VOID,          SETTER,       IDATAKEEPOLD,     1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getRupeeCost",                ZVARTYPEID_BOOL,          GETTER,       IDATARUPEECOST,     1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setRupeeCost",                ZVARTYPEID_VOID,          SETTER,       IDATARUPEECOST,     1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getEdible",                ZVARTYPEID_BOOL,          GETTER,       IDATAEDIBLE,     1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setEdible",                ZVARTYPEID_VOID,          SETTER,       IDATAEDIBLE,     1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getUnused",                ZVARTYPEID_BOOL,          GETTER,       IDATAFLAGUNUSED,     1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setUnused",                ZVARTYPEID_VOID,          SETTER,       IDATAFLAGUNUSED,     1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getGainLower",                ZVARTYPEID_BOOL,          GETTER,       IDATAGAINLOWER,     1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setGainLower",                ZVARTYPEID_VOID,          SETTER,       IDATAGAINLOWER,     1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAttributes[]",              ZVARTYPEID_FLOAT,         GETTER,       IDATAATTRIB,              15,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAttributes[]",              ZVARTYPEID_VOID,          SETTER,       IDATAATTRIB,              15,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       IDATAATTRIB,              10,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       IDATAATTRIB,              10,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlags[]",              ZVARTYPEID_BOOL,         GETTER,       IDATAFLAGS,              5,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlags[]",              ZVARTYPEID_VOID,          SETTER,       IDATAFLAGS,              5,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,         ZVARTYPEID_BOOL,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   
    { "getSprites[]",              ZVARTYPEID_FLOAT,         GETTER,       IDATASPRITE,              10,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setSprites[]",              ZVARTYPEID_VOID,          SETTER,       IDATASPRITE,              10,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   
    { "getWeapon",               ZVARTYPEID_FLOAT,         GETTER,       IDATAUSEWPN,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setWeapon",               ZVARTYPEID_VOID,          SETTER,       IDATAUSEWPN,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDefense",               ZVARTYPEID_FLOAT,         GETTER,       IDATAUSEDEF,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setDefense",               ZVARTYPEID_VOID,          SETTER,       IDATAUSEDEF,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getRange",               ZVARTYPEID_FLOAT,         GETTER,       IDATAWRANGE,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setRange",               ZVARTYPEID_VOID,          SETTER,       IDATAWRANGE,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getDuration",               ZVARTYPEID_FLOAT,         GETTER,       IDATADURATION,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setDuration",               ZVARTYPEID_VOID,          SETTER,       IDATADURATION,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },

    
    
    { "getMovement[]",              ZVARTYPEID_FLOAT,         GETTER,       IDATAUSEMVT,             ITEM_MOVEMENT_PATTERNS,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMovement[]",              ZVARTYPEID_VOID,          SETTER,       IDATAUSEMVT,              ITEM_MOVEMENT_PATTERNS,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getWeaponD[]",              ZVARTYPEID_FLOAT,         GETTER,       IDATAWPNINITD,             8,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setWeaponD[]",              ZVARTYPEID_VOID,          SETTER,       IDATAWPNINITD,              8,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWeaponMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       IDATAMISCD,             32,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setWeaponMisc[]",              ZVARTYPEID_VOID,          SETTER,       IDATAMISCD,              32,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getDuplicates",               ZVARTYPEID_FLOAT,         GETTER,       IDATADUPLICATES,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setDuplicates",               ZVARTYPEID_VOID,          SETTER,       IDATADUPLICATES,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },

    { "getDrawLayer",               ZVARTYPEID_FLOAT,         GETTER,       IDATADRAWLAYER,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setDrawLayer",               ZVARTYPEID_VOID,          SETTER,       IDATADRAWLAYER,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },

    { "getCollectFlags",               ZVARTYPEID_FLOAT,         GETTER,       IDATACOLLECTFLAGS,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setCollectFlags",               ZVARTYPEID_VOID,          SETTER,       IDATACOLLECTFLAGS,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },

    { "getWeaponScript",               ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPONSCRIPT,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setWeaponScript",               ZVARTYPEID_VOID,          SETTER,       IDATAWEAPONSCRIPT,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },

    { "getWeaponHitXOffset",               ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPHXOFS,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setWeaponHitXOffset",               ZVARTYPEID_VOID,          SETTER,       IDATAWEAPHXOFS,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWeaponHitYOffset",               ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPHYOFS,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setWeaponHitYOffset",               ZVARTYPEID_VOID,          SETTER,       IDATAWEAPHYOFS,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },

    { "getWeaponHitWidth",               ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPHXSZ,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setWeaponHitWidth",               ZVARTYPEID_VOID,          SETTER,       IDATAWEAPHXSZ,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },

    { "getWeaponHitHeight",               ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPHYSZ,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setWeaponHitHeight",               ZVARTYPEID_VOID,          SETTER,       IDATAWEAPHYSZ,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },

    { "getWeaponHitZHeight",               ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPHZSZ,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setWeaponHitZHeight",               ZVARTYPEID_VOID,          SETTER,       IDATAWEAPHZSZ,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },

    { "getWeaponDrawXOffset",               ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPXOFS,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setWeaponDrawXOffset",               ZVARTYPEID_VOID,          SETTER,       IDATAWEAPXOFS,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWeaponDrawYOffset",               ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPYOFS,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setWeaponDrawYOffset",               ZVARTYPEID_VOID,          SETTER,       IDATAWEAPYOFS,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWeaponDrawZOffset",               ZVARTYPEID_FLOAT,         GETTER,       IDATAWEAPZOFS,       1,      {  ZVARTYPEID_ITEMCLASS,    -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },      
    { "setWeaponDrawZOffset",               ZVARTYPEID_VOID,          SETTER,       IDATAWEAPZOFS,       1,      {  ZVARTYPEID_ITEMCLASS,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },

    
    
    
    
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

ItemclassSymbols::ItemclassSymbols()
{
    table = itemclassTable;
    refVar = REFITEMCLASS;
}
map<int, vector<Opcode *> > ItemclassSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::generateCode();
    //void GetName(itemclass, int)
    {
	    Function* function = functions["GetName"];
        int label = function->getLabel();
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
    { "GetCurScreen",           ZVARTYPEID_FLOAT,         GETTER,       CURSCR,               1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetCurDMapScreen",       ZVARTYPEID_FLOAT,         GETTER,       CURDSCR,              1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetCurMap",              ZVARTYPEID_FLOAT,         GETTER,       CURMAP,               1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetCurDMap",             ZVARTYPEID_FLOAT,         GETTER,       CURDMAP,              1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetCurLevel",            ZVARTYPEID_FLOAT,         GETTER,       CURLEVEL,             1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getNumDeaths",           ZVARTYPEID_FLOAT,         GETTER,       GAMEDEATHS,           1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setNumDeaths",           ZVARTYPEID_VOID,          SETTER,       GAMEDEATHS,           1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCheat",               ZVARTYPEID_FLOAT,         GETTER,       GAMECHEAT,            1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCheat",               ZVARTYPEID_VOID,          SETTER,       GAMECHEAT,            1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTime",                ZVARTYPEID_FLOAT,         GETTER,       GAMETIME,             1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTime",                ZVARTYPEID_VOID,          SETTER,       GAMETIME,             1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHasPlayed",           ZVARTYPEID_BOOL,          GETTER,       GAMEHASPLAYED,        1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHasPlayed",           ZVARTYPEID_VOID,          SETTER,       GAMEHASPLAYED,        1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTimeValid",           ZVARTYPEID_BOOL,          GETTER,       GAMETIMEVALID,        1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTimeValid",           ZVARTYPEID_VOID,          SETTER,       GAMETIMEVALID,        1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getGuyCount[]",          ZVARTYPEID_FLOAT,         GETTER,       GAMEGUYCOUNT,         2,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setGuyCount[]",          ZVARTYPEID_VOID,          SETTER,       GAMEGUYCOUNT,         2,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getContinueScreen",      ZVARTYPEID_FLOAT,         GETTER,       GAMECONTSCR,          1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setContinueScreen",      ZVARTYPEID_VOID,          SETTER,       GAMECONTSCR,          1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getContinueDMap",        ZVARTYPEID_FLOAT,         GETTER,       GAMECONTDMAP,         1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setContinueDMap",        ZVARTYPEID_VOID,          SETTER,       GAMECONTDMAP,         1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCounter[]",           ZVARTYPEID_FLOAT,         GETTER,       GAMECOUNTERD,        32,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCounter[]",           ZVARTYPEID_VOID,          SETTER,       GAMECOUNTERD,        32,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMCounter[]",          ZVARTYPEID_FLOAT,         GETTER,       GAMEMCOUNTERD,       32,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMCounter[]",          ZVARTYPEID_VOID,          SETTER,       GAMEMCOUNTERD,       32,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDCounter[]",          ZVARTYPEID_FLOAT,         GETTER,       GAMEDCOUNTERD,       32,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDCounter[]",          ZVARTYPEID_VOID,          SETTER,       GAMEDCOUNTERD,       32,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getGeneric[]",           ZVARTYPEID_FLOAT,         GETTER,       GAMEGENERICD,       256,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setGeneric[]",           ZVARTYPEID_VOID,          SETTER,       GAMEGENERICD,       256,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLItems[]",            ZVARTYPEID_FLOAT,         GETTER,       GAMELITEMSD,        256,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLItems[]",            ZVARTYPEID_VOID,          SETTER,       GAMELITEMSD,        256,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLKeys[]",             ZVARTYPEID_FLOAT,         GETTER,       GAMELKEYSD,         256,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLKeys[]",             ZVARTYPEID_VOID,          SETTER,       GAMELKEYSD,         256,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenState",         ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenState",         ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,	  ZVARTYPEID_BOOL,		  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenD",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenD",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapScreenD",         ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetDMapScreenD",         ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "LoadItemData",           ZVARTYPEID_ITEMCLASS,     FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PlaySound",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PlayMIDI",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PlayEnhancedMusic",      ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapMusicFilename",   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapMusicTrack",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetDMapEnhancedMusic",   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboData",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboData",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboCSet",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboCSet",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboFlag",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboFlag",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboType",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboType",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboInherentFlag",   ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboInherentFlag",   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetComboSolid",          ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetComboSolid",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetMIDI",                ZVARTYPEID_FLOAT,         GETTER,       GETMIDI,              1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenFlags",         ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenEFlags",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapFlags[]",         ZVARTYPEID_FLOAT,         GETTER,       DMAPFLAGSD,         512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapFlags[]",         ZVARTYPEID_VOID,          SETTER,       DMAPFLAGSD,         512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapLevel[]",         ZVARTYPEID_FLOAT,         GETTER,       DMAPLEVELD,         512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapLevel[]",         ZVARTYPEID_VOID,          SETTER,       DMAPLEVELD,         512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapCompass[]",       ZVARTYPEID_FLOAT,         GETTER,       DMAPCOMPASSD,       512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapCompass[]",       ZVARTYPEID_VOID,          SETTER,       DMAPCOMPASSD,       512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapContinue[]",      ZVARTYPEID_FLOAT,         GETTER,       DMAPCONTINUED,      512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapContinue[]",      ZVARTYPEID_VOID,          SETTER,       DMAPCONTINUED,      512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapMIDI[]",          ZVARTYPEID_FLOAT,         GETTER,       DMAPMIDID,          512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapMIDI[]",          ZVARTYPEID_VOID,          SETTER,       DMAPMIDID,          512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "Save",                   ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "End",                    ZVARTYPEID_VOID,          FUNCTION,     0,       	   	        1,      {  ZVARTYPEID_GAME,         -1,    					          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ComboTile",              ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetSaveName",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetSaveName",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetMessage",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapName",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapTitle",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetDMapIntro",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStandalone",          ZVARTYPEID_BOOL,          GETTER,       GAMESTANDALONE,       1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setStandalone",          ZVARTYPEID_VOID,          SETTER,       GAMESTANDALONE,       1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ShowSaveScreen",         ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ShowSaveQuitScreen",     ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLastEntranceScreen",  ZVARTYPEID_FLOAT,         GETTER,       GAMEENTRSCR,          1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLastEntranceScreen",  ZVARTYPEID_VOID,          SETTER,       GAMEENTRSCR,          1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getLastEntranceDMap",    ZVARTYPEID_FLOAT,         GETTER,       GAMEENTRDMAP,         1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setLastEntranceDMap",    ZVARTYPEID_VOID,          SETTER,       GAMEENTRDMAP,         1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getClickToFreezeEnabled",ZVARTYPEID_BOOL,          GETTER,       GAMECLICKFREEZE,      1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setClickToFreezeEnabled",ZVARTYPEID_VOID,          SETTER,       GAMECLICKFREEZE,      1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapOffset[]",        ZVARTYPEID_FLOAT,         GETTER,       DMAPOFFSET,         512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapOffset[]",        ZVARTYPEID_VOID,          SETTER,       DMAPOFFSET,         512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDMapMap[]",           ZVARTYPEID_FLOAT,         GETTER,       DMAPMAP,            512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapMap[]",           ZVARTYPEID_VOID,          SETTER,       DMAPMAP,            512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetFFCScript",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //Get an item script ID, similar to GetFFCScript()
    { "GetItemScript",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //Monochrome mode
    { "GreyscaleOn",                    ZVARTYPEID_VOID,          FUNCTION,     0,       	   	        1,      {  ZVARTYPEID_GAME,         -1,    					          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GreyscaleOff",                    ZVARTYPEID_VOID,          FUNCTION,     0,       	   	        1,      {  ZVARTYPEID_GAME,         -1,    					          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //Joypad and Keyboard. -Z
    { "getButtonPress[]",             ZVARTYPEID_BOOL,         GETTER,       BUTTONPRESS,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setButtonPress[]",             ZVARTYPEID_VOID,          SETTER,       BUTTONPRESS,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_BOOL,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getButtonInput[]",             ZVARTYPEID_BOOL,         GETTER,       BUTTONINPUT,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setButtonInput[]",             ZVARTYPEID_VOID,          SETTER,       BUTTONINPUT,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_BOOL,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getButtonHeld[]",             ZVARTYPEID_BOOL,         GETTER,       BUTTONHELD,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setButtonHeld[]",             ZVARTYPEID_VOID,          SETTER,       BUTTONHELD,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_BOOL,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getReadKey[]",             ZVARTYPEID_BOOL,         GETTER,       READKEY,         127,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getKeyPress[]",             ZVARTYPEID_BOOL,         GETTER,       KEYPRESS,         127,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setKeyPress[]",             ZVARTYPEID_VOID,          SETTER,       KEYPRESS,         127,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_BOOL,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDisableItem[]",             ZVARTYPEID_BOOL,         GETTER,       DISABLEDITEM,        256,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDisableItem[]",             ZVARTYPEID_VOID,          SETTER,       DISABLEDITEM,         256,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_BOOL,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJoypadPress[]",             ZVARTYPEID_FLOAT,         GETTER,       JOYPADPRESS,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //Read or change the palette (level palette) for any given DMap
    { "getDMapPalette[]",         ZVARTYPEID_FLOAT,         GETTER,       DMAPLEVELPAL,         512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapPalette[]",         ZVARTYPEID_VOID,          SETTER,       DMAPLEVELPAL,         512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //Get the map count 
    { "MapCount",              ZVARTYPEID_FLOAT,         GETTER,       GAMEMAXMAPS,               1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    //Versioning
    { "getVersion",	ZVARTYPEID_FLOAT,          GETTER,       ZELDAVERSION,      1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getBuild",	ZVARTYPEID_FLOAT,          GETTER,       ZELDABUILD,      1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getBeta",	ZVARTYPEID_FLOAT,          GETTER,       ZELDABETA,      1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDisableActiveSubscreen",ZVARTYPEID_BOOL,          GETTER,       NOACTIVESUBSC,      1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDisableActiveSubscreen",ZVARTYPEID_VOID,          SETTER,       NOACTIVESUBSC,      1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetMessage",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetDMapName",            ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetDMapTitle",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetDMapIntro",           ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenEnemy",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenEnemy",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenDoor",        ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,     -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenDoor",          ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "SetScreenWidth",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenWidth",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenHeight",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenHeight",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenViewX",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenViewX",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenViewY",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenViewY",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenGuy",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenGuy",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenString",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenString",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenRoomType",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenRoomType",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenEntryX",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenEntryX",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "SetScreenEntryY",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenEntryY",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenItem",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenItem",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenUndercombo",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenUndercombo",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenUnderCSet",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenUnderCSet",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenCatchall",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    -1,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenCatchall",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenLayerOpacity",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenLayerOpacity",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenSecretCombo",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenSecretCombo",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenSecretCSet",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenSecretCSet",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenSecretFlag",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenSecretFlag",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenLayerMap",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenLayerMap",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenLayerScreen",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenLayerScreen",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenPath",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenPath",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenWarpReturnX",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenWarpReturnX",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "SetScreenWarpReturnY",             ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,     ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,						                -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetScreenWarpReturnY",             ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    ZVARTYPEID_FLOAT,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHighestStringID",	ZVARTYPEID_FLOAT,          GETTER,       GAMENUMMESSAGES,      1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getNumMessages",	ZVARTYPEID_FLOAT,          GETTER,       GAMENUMMESSAGES,      1,      {  ZVARTYPEID_GAME,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
  
   
    
    { "",                       -1,                               -1,           -1,                  -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

GameSymbols::GameSymbols()
{
    table = gameTable;
    refVar = NUL;
}

map<int, vector<Opcode *> > GameSymbols::generateCode()
{
    map<int,vector<Opcode *> > rval = LibrarySymbols::generateCode();
    //itemclass LoadItemData(game, int)
    {
	    Function* function = functions["LoadItemData"];
        int label = function->getLabel();
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
	    Function* function = functions["GetScreenState"];
        int label = function->getLabel();
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
	    Function* function = functions["SetScreenState"];
        int label = function->getLabel();
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
	    Function* function = functions["GetScreenD"];
        int label = function->getLabel();
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
	    Function* function = functions["SetScreenD"];
        int label = function->getLabel();
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
	    Function* function = functions["GetDMapScreenD"];
        int label = function->getLabel();
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
	    Function* function = functions["SetDMapScreenD"];
        int label = function->getLabel();
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
	    Function* function = functions["PlaySound"];
        int label = function->getLabel();
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
	    Function* function = functions["PlayMIDI"];
        int label = function->getLabel();
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
	    Function* function = functions["PlayEnhancedMusic"];
        int label = function->getLabel();
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
	    Function* function = functions["GetDMapMusicFilename"];
        int label = function->getLabel();
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
	    Function* function = functions["GetDMapMusicTrack"];
        int label = function->getLabel();
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
	    Function* function = functions["SetDMapEnhancedMusic"];
        int label = function->getLabel();
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
	    Function* function = functions["GetComboData"];
        int label = function->getLabel();
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
	    Function* function = functions["SetComboData"];
        int label = function->getLabel();
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
	    Function* function = functions["GetComboCSet"];
        int label = function->getLabel();
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
	    Function* function = functions["SetComboCSet"];
        int label = function->getLabel();
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
	    Function* function = functions["GetComboFlag"];
        int label = function->getLabel();
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
	    Function* function = functions["SetComboFlag"];
        int label = function->getLabel();
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
	    Function* function = functions["GetComboType"];
        int label = function->getLabel();
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
	    Function* function = functions["SetComboType"];
        int label = function->getLabel();
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
	    Function* function = functions["GetComboInherentFlag"];
        int label = function->getLabel();
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
	    Function* function = functions["SetComboInherentFlag"];
        int label = function->getLabel();
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
	    Function* function = functions["GetComboSolid"];
        int label = function->getLabel();
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
	    Function* function = functions["SetComboSolid"];
        int label = function->getLabel();
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
	    Function* function = functions["GetScreenFlags"];
        int label = function->getLabel();
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
	    Function* function = functions["GetScreenEFlags"];
        int label = function->getLabel();
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
	    Function* function = functions["Save"];
        int label = function->getLabel();
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
	    Function* function = functions["End"];
        int label = function->getLabel();
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
	    Function* function = functions["ComboTile"];
        int label = function->getLabel();
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
	    Function* function = functions["GetSaveName"];
        int label = function->getLabel();
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
	    Function* function = functions["SetSaveName"];
        int label = function->getLabel();
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
	    Function* function = functions["GetMessage"];
        int label = function->getLabel();
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
	    Function* function = functions["GetDMapName"];
        int label = function->getLabel();
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
	    Function* function = functions["GetDMapTitle"];
        int label = function->getLabel();
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
	    Function* function = functions["GetDMapIntro"];
        int label = function->getLabel();
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
    
    
    
    //void GreyscaleOn(game)
    {
	    Function* function = functions["GreyscaleOn"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OGreyscaleOn());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
            
	//void GreyscaleOff(game)
    {
	    Function* function = functions["GreyscaleOff"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OGreyscaleOff());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    
   // SetMessage(game, int, int)
    {
	    Function* function = functions["SetMessage"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        code.push_back(new OSetMessage(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //SetDMapName(game, int, int)
    {
	    Function* function = functions["SetDMapName"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        code.push_back(new OSetDMapName(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //SetDMapTitle(game, int, int)
    {
	    Function* function = functions["SetDMapTitle"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        code.push_back(new OSetDMapTitle(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //SetDMapIntro(game, int, int)
    {
	    Function* function = functions["SetDMapIntro"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(SFTEMP)));
        code.push_back(new OSetDMapIntro(new VarArgument(EXP2), new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //bool ShowSaveScreen(game)
    {
	    Function* function = functions["ShowSaveScreen"];
        int label = function->getLabel();
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
	    Function* function = functions["ShowSaveQuitScreen"];
        int label = function->getLabel();
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
	    Function* function = functions["GetFFCScript"];
        int label = function->getLabel();
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
    
    
     //int GetItemScript(game, int)
    {
	    Function* function = functions["GetItemScript"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetItemScript(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
     //int GetScreenEnemy(game,int,int,int)
    {
	    Function* function = functions["GetScreenEnemy"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenEnemy(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
     //int GetScreenDoor(game,int,int,int)
    {
	    Function* function = functions["GetScreenDoor"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenDoor(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenEnemy(int,int,int,int)
    {
	    Function* function = functions["SetScreenEnemy"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENENEMY), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenDoor(int,int,int,int)
    {
	    Function* function = functions["SetScreenDoor"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENDOOR), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //void SetScreenWidth(game, int,int,int)
    {
	    Function* function = functions["SetScreenWidth"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENWIDTH), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenWidth(game, int,int)
    {
	    Function* function = functions["GetScreenWidth"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENWIDTH)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //void SetScreenHeight(game, int,int,int)
    {
	    Function* function = functions["SetScreenHeight"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENHEIGHT), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenHeight(game, int,int)
    {
	    Function* function = functions["GetScreenHeight"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENHEIGHT)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenViewX(game, int,int,int)
    {
	    Function* function = functions["SetScreenViewX"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENVIEWX), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenViewX(game, int,int)
    {
	    Function* function = functions["GetScreenViewX"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENVIEWX)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
     //void SetScreenViewY(game, int,int,int)
    {
	    Function* function = functions["SetScreenViewY"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENVIEWY), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenViewY(game, int,int)
    {
	    Function* function = functions["GetScreenViewY"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENVIEWY)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenGuy(game, int,int,int)
    {
	    Function* function = functions["SetScreenGuy"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENGUY), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenGuy(game, int,int)
    {
	    Function* function = functions["GetScreenGuy"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENGUY)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenString(game, int,int,int)
    {
	    Function* function = functions["SetScreenString"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENSTRING), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenString(game, int,int)
    {
	    Function* function = functions["GetScreenString"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENSTRING)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenRoomType(game, int,int,int)
    {
	    Function* function = functions["SetScreenRoomType"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENROOM), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenRoomType(game, int,int)
    {
	    Function* function = functions["GetScreenRoomType"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENROOM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenEntryX(game, int,int,int)
    {
	    Function* function = functions["SetScreenEntryX"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENENTX), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenEntryX(game, int,int)
    {
	    Function* function = functions["GetScreenEntryX"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENENTX)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenEntryY(game, int,int,int)
    {
	    Function* function = functions["SetScreenEntryY"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENENTY), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenEntryY(game, int,int)
    {
	    Function* function = functions["GetScreenEntryY"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENENTY)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
     //void SetScreenItem(game, int,int,int)
    {
	    Function* function = functions["SetScreenItem"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENITEM), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenItem(game, int,int)
    {
	    Function* function = functions["GetScreenItem"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENITEM)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
     //void SetScreenUndercombo(game, int,int,int)
    {
	    Function* function = functions["SetScreenUndercombo"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENUNDCMB), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenUndercombo(game, int,int)
    {
	    Function* function = functions["GetScreenUndercombo"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENUNDCMB)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenUnderCSet(game, int,int,int)
    {
	    Function* function = functions["SetScreenUnderCSet"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENUNDCST), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenUnderCSet(game, int,int)
    {
	    Function* function = functions["GetScreenUnderCSet"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENUNDCST)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenCatchall(game, int,int,int)
    {
	    Function* function = functions["SetScreenCatchall"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(SETSCREENCATCH), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenCatchall(game, int,int)
    {
	    Function* function = functions["GetScreenCatchall"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX2));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(SETSCREENCATCH)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //void SetScreenLayerOpacity(int,int,int,int)
    {
	    Function* function = functions["SetScreenLayerOpacity"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENLAYOP), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenLayerOpacity(game,int,int,int)
    {
	    Function* function = functions["GetScreenLayerOpacity"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenLayerOpacity(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }

     //void SetScreenSecretCombo(int,int,int,int)
    {
	    Function* function = functions["SetScreenSecretCombo"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENSECCMB), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenSecretCombo(game,int,int,int)
    {
	    Function* function = functions["GetScreenSecretCombo"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenSecretCombo(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }

     //void SetScreenSecretCSet(int,int,int,int)
    {
	    Function* function = functions["SetScreenSecretCSet"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENSECCST), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenSecretCSet(game,int,int,int)
    {
	    Function* function = functions["GetScreenSecretCSet"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenSecretCSet(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenSecretFlag(int,int,int,int)
    {
	    Function* function = functions["SetScreenSecretFlag"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENSECFLG), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenSecretFlag(game,int,int,int)
    {
	    Function* function = functions["GetScreenSecretFlag"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenSecretFlag(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }

     //void SetScreenLayerMap(int,int,int,int)
    {
	    Function* function = functions["SetScreenLayerMap"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENLAYMAP), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenLayerMap(game,int,int,int)
    {
	    Function* function = functions["GetScreenLayerMap"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenLayerMap(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }

    
    //void SetScreenLayerScreen(int,int,int,int)
    {
	    Function* function = functions["SetScreenLayerScreen"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENLAYSCR), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenLayerScreen(game,int,int,int)
    {
	    Function* function = functions["GetScreenLayerScreen"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenLayerScreen(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }

    //void SetScreenPath(int,int,int,int)
    {
	    Function* function = functions["SetScreenPath"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENPATH), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenPath(game,int,int,int)
    {
	    Function* function = functions["GetScreenPath"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenPath(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenWarpReturnX(int,int,int,int)
    {
	    Function* function = functions["SetScreenWarpReturnX"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENWARPRX), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenWarpReturnX(game,int,int,int)
    {
	    Function* function = functions["GetScreenWarpReturnX"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenWarpReturnX(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void SetScreenWarpReturnY(int,int,int,int)
    {
	    Function* function = functions["SetScreenWarpReturnY"];
        int label = function->getLabel();
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
        code.push_back(new OSetRegister(new VarArgument(SETSCREENWARPRY), new VarArgument(EXP2)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //int GetScreenWarpReturnY(game,int,int,int)
    {
	    Function* function = functions["GetScreenWarpReturnY"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(INDEX));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(EXP1)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetScreenWarpReturnY(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    return rval;
}

NPCSymbols NPCSymbols::singleton = NPCSymbols();

static AccessorTable npcTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "getX",                   ZVARTYPEID_FLOAT,         GETTER,       NPCX,                 1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ZVARTYPEID_VOID,          SETTER,       NPCX,                 1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ZVARTYPEID_FLOAT,         GETTER,       NPCY,                 1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ZVARTYPEID_VOID,          SETTER,       NPCY,                 1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getZ",                   ZVARTYPEID_FLOAT,         GETTER,       NPCZ,                 1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setZ",                   ZVARTYPEID_VOID,          SETTER,       NPCZ,                 1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJump",                ZVARTYPEID_FLOAT,         GETTER,       NPCJUMP,              1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setJump",                ZVARTYPEID_VOID,          SETTER,       NPCJUMP,              1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDir",                 ZVARTYPEID_FLOAT,         GETTER,       NPCDIR,               1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDir",                 ZVARTYPEID_VOID,          SETTER,       NPCDIR,               1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getRate",                ZVARTYPEID_FLOAT,         GETTER,       NPCRATE,              1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setRate",                ZVARTYPEID_VOID,          SETTER,       NPCRATE,              1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHoming",              ZVARTYPEID_FLOAT,         GETTER,       NPCHOMING,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHoming",              ZVARTYPEID_VOID,          SETTER,       NPCHOMING,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStep",				  ZVARTYPEID_FLOAT,	        GETTER,       NPCSTEP,              1,      {  ZVARTYPEID_NPC,			-1,								  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                  } },
    { "setStep",				  ZVARTYPEID_VOID,          SETTER,       NPCSTEP,              1,      {  ZVARTYPEID_NPC,			 ZVARTYPEID_FLOAT,		  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                       } },
    { "getASpeed",              ZVARTYPEID_FLOAT,         GETTER,       NPCFRAMERATE,         1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setASpeed",              ZVARTYPEID_VOID,          SETTER,       NPCFRAMERATE,         1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHaltrate",            ZVARTYPEID_FLOAT,         GETTER,       NPCHALTRATE,          1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHaltrate",            ZVARTYPEID_VOID,          SETTER,       NPCHALTRATE,          1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawStyle",           ZVARTYPEID_FLOAT,         GETTER,       NPCDRAWTYPE,          1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawStyle",           ZVARTYPEID_VOID,          SETTER,       NPCDRAWTYPE,          1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHP",                  ZVARTYPEID_FLOAT,         GETTER,       NPCHP,                1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHP",                  ZVARTYPEID_VOID,          SETTER,       NPCHP,                1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getID",                  ZVARTYPEID_FLOAT,         GETTER,       NPCID,                1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setID",                  ZVARTYPEID_VOID,          SETTER,       NPCID,                1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getType",                ZVARTYPEID_FLOAT,         GETTER,       NPCTYPE,              1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setType",                ZVARTYPEID_VOID,          SETTER,       NPCTYPE,              1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDamage",              ZVARTYPEID_FLOAT,         GETTER,       NPCDP,                1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDamage",              ZVARTYPEID_VOID,          SETTER,       NPCDP,                1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWeaponDamage",        ZVARTYPEID_FLOAT,         GETTER,       NPCWDP,               1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setWeaponDamage",        ZVARTYPEID_VOID,          SETTER,       NPCWDP,               1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",                ZVARTYPEID_FLOAT,         GETTER,       NPCTILE,              1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTile",                ZVARTYPEID_VOID,          SETTER,       NPCTILE,              1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalTile",        ZVARTYPEID_FLOAT,         GETTER,       NPCOTILE,             1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setOriginalTile",        ZVARTYPEID_VOID,          SETTER,       NPCOTILE,             1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWeapon",              ZVARTYPEID_FLOAT,         GETTER,       NPCWEAPON,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setWeapon",              ZVARTYPEID_VOID,          SETTER,       NPCWEAPON,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getItemSet",             ZVARTYPEID_FLOAT,         GETTER,       NPCITEMSET,           1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setItemSet",             ZVARTYPEID_VOID,          SETTER,       NPCITEMSET,           1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       NPCCSET,              1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCSet",                ZVARTYPEID_VOID,          SETTER,       NPCCSET,              1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getBossPal",             ZVARTYPEID_FLOAT,         GETTER,       NPCBOSSPAL,           1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setBossPal",             ZVARTYPEID_VOID,          SETTER,       NPCBOSSPAL,           1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getSFX",                 ZVARTYPEID_FLOAT,         GETTER,       NPCBGSFX,             1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setSFX",                 ZVARTYPEID_VOID,          SETTER,       NPCBGSFX,             1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getExtend",              ZVARTYPEID_FLOAT,         GETTER,       NPCEXTEND,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setExtend",              ZVARTYPEID_VOID,          SETTER,       NPCEXTEND,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitWidth",            ZVARTYPEID_FLOAT,         GETTER,       NPCHXSZ,              1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitWidth",            ZVARTYPEID_VOID,          SETTER,       NPCHXSZ,              1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitHeight",           ZVARTYPEID_FLOAT,         GETTER,       NPCHYSZ,              1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitHeight",           ZVARTYPEID_VOID,          SETTER,       NPCHYSZ,              1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitZHeight",          ZVARTYPEID_FLOAT,         GETTER,       NPCHZSZ,              1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitZHeight",          ZVARTYPEID_VOID,          SETTER,       NPCHZSZ,              1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       NPCTXSZ,              1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       NPCTXSZ,              1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       NPCTYSZ,              1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       NPCTYSZ,              1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawXOffset",         ZVARTYPEID_FLOAT,         GETTER,       NPCXOFS,              1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawXOffset",         ZVARTYPEID_VOID,          SETTER,       NPCXOFS,              1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawYOffset",         ZVARTYPEID_FLOAT,         GETTER,       NPCYOFS,              1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawYOffset",         ZVARTYPEID_VOID,          SETTER,       NPCYOFS,              1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawZOffset",         ZVARTYPEID_FLOAT,         GETTER,       NPCZOFS,              1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawZOffset",         ZVARTYPEID_VOID,          SETTER,       NPCZOFS,              1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitXOffset",          ZVARTYPEID_FLOAT,         GETTER,       NPCHXOFS,             1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitXOffset",          ZVARTYPEID_VOID,          SETTER,       NPCHXOFS,             1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitYOffset",          ZVARTYPEID_FLOAT,         GETTER,       NPCHYOFS,             1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitYOffset",          ZVARTYPEID_VOID,          SETTER,       NPCHYOFS,             1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "isValid",		ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       NPCMISCD,            32,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       NPCMISCD,            32,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAttributes[]",        ZVARTYPEID_FLOAT,         GETTER,       NPCDD,               15,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAttributes[]",        ZVARTYPEID_VOID,          SETTER,       NPCDD,               15,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMiscFlags",           ZVARTYPEID_FLOAT,         GETTER,       NPCMFLAGS,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMiscFlags",           ZVARTYPEID_VOID,          SETTER,       NPCMFLAGS,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection", 	  ZVARTYPEID_FLOAT,         GETTER,       NPCCOLLDET,           1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection",       ZVARTYPEID_VOID,          SETTER,       NPCCOLLDET,           1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStun",                ZVARTYPEID_FLOAT,         GETTER,       NPCSTUN,              1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setStun",                ZVARTYPEID_VOID,          SETTER,       NPCSTUN,              1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDefense[]",           ZVARTYPEID_FLOAT,         GETTER,       NPCDEFENSED,         40,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDefense[]",           ZVARTYPEID_VOID,          SETTER,       NPCDEFENSED,         40,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetName",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_NPC,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHunger",              ZVARTYPEID_FLOAT,         GETTER,       NPCHUNGER,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHunger",              ZVARTYPEID_VOID,          SETTER,       NPCHUNGER,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "BreakShield",		ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getWeaponSprite",          ZVARTYPEID_FLOAT,         GETTER,       NPCWEAPSPRITE,             1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setWeaponSprite",          ZVARTYPEID_VOID,          SETTER,       NPCWEAPSPRITE,             1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	//2.future srit compatibility. -Z
    { "getScriptDefense[]",           ZVARTYPEID_FLOAT,         GETTER,       NPCSCRDEFENSED,         10,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setScriptDefense[]",           ZVARTYPEID_VOID,          SETTER,       NPCSCRDEFENSED,         10,      { ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   
    
    { "getInvFrames",             ZVARTYPEID_FLOAT,         GETTER,       NPCINVINC,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInvFrames",              ZVARTYPEID_VOID,          SETTER,       NPCINVINC,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getInvincible",              ZVARTYPEID_FLOAT,         GETTER,       NPCSUPERMAN,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setInvincible",              ZVARTYPEID_VOID,          SETTER,       NPCSUPERMAN,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getRingleader",              ZVARTYPEID_BOOL,         GETTER,       NPCRINGLEAD,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setRingleader",              ZVARTYPEID_VOID,          SETTER,       NPCRINGLEAD,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHasItem",              ZVARTYPEID_BOOL,         GETTER,       NPCHASITEM,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHasItem",              ZVARTYPEID_VOID,          SETTER,       NPCHASITEM,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    
    
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

NPCSymbols::NPCSymbols()
{
    table = npcTable;
    refVar = REFNPC;
}

map<int, vector<Opcode *> > NPCSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::generateCode();
    //bool isValid(npc)
    {
	    Function* function = functions["isValid"];
        int label = function->getLabel();
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
	    Function* function = functions["GetName"];
        int label = function->getLabel();
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
	    Function* function = functions["BreakShield"];
        int label = function->getLabel();
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
    { "getX",                   ZVARTYPEID_FLOAT,         GETTER,       LWPNX,                1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ZVARTYPEID_VOID,          SETTER,       LWPNX,                1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ZVARTYPEID_FLOAT,         GETTER,       LWPNY,                1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ZVARTYPEID_VOID,          SETTER,       LWPNY,                1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getZ",                   ZVARTYPEID_FLOAT,         GETTER,       LWPNZ,                1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setZ",                   ZVARTYPEID_VOID,          SETTER,       LWPNZ,                1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJump",                ZVARTYPEID_FLOAT,         GETTER,       LWPNJUMP,             1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setJump",                ZVARTYPEID_VOID,          SETTER,       LWPNJUMP,             1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDir",                 ZVARTYPEID_FLOAT,         GETTER,       LWPNDIR,              1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDir",                 ZVARTYPEID_VOID,          SETTER,       LWPNDIR,              1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAngle",               ZVARTYPEID_FLOAT,         GETTER,       LWPNANGLE,            1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAngle",               ZVARTYPEID_VOID,          SETTER,       LWPNANGLE,            1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStep",			      ZVARTYPEID_FLOAT,	        GETTER,	      LWPNSTEP,             1,	    {  ZVARTYPEID_LWPN,			-1,								  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setStep",			      ZVARTYPEID_VOID,		    SETTER,	      LWPNSTEP,             1,	    {  ZVARTYPEID_LWPN,			 ZVARTYPEID_FLOAT,		  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getNumFrames",           ZVARTYPEID_FLOAT,         GETTER,       LWPNFRAMES,           1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setNumFrames",           ZVARTYPEID_VOID,          SETTER,       LWPNFRAMES,           1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFrame",               ZVARTYPEID_FLOAT,         GETTER,       LWPNFRAME,            1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFrame",               ZVARTYPEID_VOID,          SETTER,       LWPNFRAME,            1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawStyle",           ZVARTYPEID_FLOAT,         GETTER,       LWPNDRAWTYPE,         1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawStyle",           ZVARTYPEID_VOID,          SETTER,       LWPNDRAWTYPE,         1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDamage",              ZVARTYPEID_FLOAT,         GETTER,       LWPNPOWER,            1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDamage",              ZVARTYPEID_VOID,          SETTER,       LWPNPOWER,            1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getID",                  ZVARTYPEID_FLOAT,         GETTER,       LWPNID,               1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setID",                  ZVARTYPEID_VOID,          SETTER,       LWPNID,               1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAngular",             ZVARTYPEID_BOOL,          GETTER,       LWPNANGULAR,          1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAngular",             ZVARTYPEID_VOID,          SETTER,       LWPNANGULAR,          1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getBehind",              ZVARTYPEID_BOOL,          GETTER,       LWPNBEHIND,           1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setBehind",              ZVARTYPEID_VOID,          SETTER,       LWPNBEHIND,           1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getASpeed",              ZVARTYPEID_FLOAT,         GETTER,       LWPNASPEED,           1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setASpeed",              ZVARTYPEID_VOID,          SETTER,       LWPNASPEED,           1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",                ZVARTYPEID_FLOAT,         GETTER,       LWPNTILE,             1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTile",                ZVARTYPEID_VOID,          SETTER,       LWPNTILE,             1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlashCSet",           ZVARTYPEID_FLOAT,         GETTER,       LWPNFLASHCSET,        1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlashCSet",           ZVARTYPEID_VOID,          SETTER,       LWPNFLASHCSET,        1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDeadState",           ZVARTYPEID_FLOAT,         GETTER,       LWPNDEAD,             1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDeadState",           ZVARTYPEID_VOID,          SETTER,       LWPNDEAD,             1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       LWPNCSET,             1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCSet",                ZVARTYPEID_VOID,          SETTER,       LWPNCSET,             1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlash",               ZVARTYPEID_BOOL,          GETTER,       LWPNFLASH,            1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlash",               ZVARTYPEID_VOID,          SETTER,       LWPNFLASH,            1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlip",                ZVARTYPEID_FLOAT,         GETTER,       LWPNFLIP,             1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlip",                ZVARTYPEID_VOID,          SETTER,       LWPNFLIP,             1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalTile",        ZVARTYPEID_FLOAT,         GETTER,       LWPNOTILE,            1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setOriginalTile",        ZVARTYPEID_VOID,          SETTER,       LWPNOTILE,            1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalCSet",        ZVARTYPEID_FLOAT,         GETTER,       LWPNOCSET,            1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setOriginalCSet",		  ZVARTYPEID_VOID,          SETTER,       LWPNOCSET,            1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getExtend",              ZVARTYPEID_FLOAT,         GETTER,       LWPNEXTEND,           1,      {  ZVARTYPEID_LWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setExtend",              ZVARTYPEID_VOID,          SETTER,       LWPNEXTEND,           1,      {  ZVARTYPEID_LWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitWidth",            ZVARTYPEID_FLOAT,         GETTER,       LWPNHXSZ,             1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitWidth",            ZVARTYPEID_VOID,          SETTER,       LWPNHXSZ,             1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitHeight",           ZVARTYPEID_FLOAT,         GETTER,       LWPNHYSZ,             1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitHeight",           ZVARTYPEID_VOID,          SETTER,       LWPNHYSZ,             1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitZHeight",          ZVARTYPEID_FLOAT,         GETTER,       LWPNHZSZ,             1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitZHeight",          ZVARTYPEID_VOID,          SETTER,       LWPNHZSZ,             1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       LWPNTXSZ,             1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       LWPNTXSZ,             1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       LWPNTYSZ,             1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       LWPNTYSZ,             1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawXOffset",         ZVARTYPEID_FLOAT,         GETTER,       LWPNXOFS,             1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawXOffset",         ZVARTYPEID_VOID,          SETTER,       LWPNXOFS,             1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawYOffset",         ZVARTYPEID_FLOAT,         GETTER,       LWPNYOFS,             1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawYOffset",         ZVARTYPEID_VOID,          SETTER,       LWPNYOFS,             1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawZOffset",         ZVARTYPEID_FLOAT,         GETTER,       LWPNZOFS,             1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawZOffset",         ZVARTYPEID_VOID,          SETTER,       LWPNZOFS,             1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitXOffset",          ZVARTYPEID_FLOAT,         GETTER,       LWPNHXOFS,            1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitXOffset",          ZVARTYPEID_VOID,          SETTER,       LWPNHXOFS,            1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitYOffset",          ZVARTYPEID_FLOAT,         GETTER,       LWPNHYOFS,            1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitYOffset",          ZVARTYPEID_VOID,          SETTER,       LWPNHYOFS,            1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "isValid",			      ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_LWPN,          -1,                              -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "UseSprite",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       LWPNMISCD,           32,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       LWPNMISCD,           32,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection", 	  ZVARTYPEID_FLOAT,         GETTER,       LWPNCOLLDET,          1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection", 	  ZVARTYPEID_VOID,          SETTER,       LWPNCOLLDET,          1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getRange",          ZVARTYPEID_FLOAT,         GETTER,       LWPNRANGE,            1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setRange",          ZVARTYPEID_VOID,          SETTER,       LWPNRANGE,            1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                                -1,                              -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};



LinkWeaponSymbols::LinkWeaponSymbols()
{
    table = lwpnTable;
    refVar = REFLWPN;
}

map<int, vector<Opcode *> > LinkWeaponSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::generateCode();
    int id=-1;
    //bool isValid(lweapon)
    {
	    Function* function = functions["isValid"];
        int label = function->getLabel();
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
	    Function* function = functions["UseSprite"];
        int label = function->getLabel();
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
    { "getX",                   ZVARTYPEID_FLOAT,         GETTER,       EWPNX,                1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setX",                   ZVARTYPEID_VOID,          SETTER,       EWPNX,                1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getY",                   ZVARTYPEID_FLOAT,         GETTER,       EWPNY,                1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setY",                   ZVARTYPEID_VOID,          SETTER,       EWPNY,                1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getZ",                   ZVARTYPEID_FLOAT,         GETTER,       EWPNZ,                1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setZ",                   ZVARTYPEID_VOID,          SETTER,       EWPNZ,                1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getJump",                ZVARTYPEID_FLOAT,         GETTER,       EWPNJUMP,             1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setJump",                ZVARTYPEID_VOID,          SETTER,       EWPNJUMP,             1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDir",                 ZVARTYPEID_FLOAT,         GETTER,       EWPNDIR,              1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDir",                 ZVARTYPEID_VOID,          SETTER,       EWPNDIR,              1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAngle",               ZVARTYPEID_FLOAT,         GETTER,       EWPNANGLE,            1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAngle",               ZVARTYPEID_VOID,          SETTER,       EWPNANGLE,            1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStep",				  ZVARTYPEID_FLOAT,         GETTER,	      EWPNSTEP,             1,      {  ZVARTYPEID_EWPN,			-1,								  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setStep",				  ZVARTYPEID_VOID,		    SETTER,	      EWPNSTEP,             1,      {  ZVARTYPEID_EWPN,			 ZVARTYPEID_FLOAT,		  -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getNumFrames",           ZVARTYPEID_FLOAT,         GETTER,       EWPNFRAMES,           1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setNumFrames",           ZVARTYPEID_VOID,          SETTER,       EWPNFRAMES,           1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFrame",               ZVARTYPEID_FLOAT,         GETTER,       EWPNFRAME,            1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFrame",               ZVARTYPEID_VOID,          SETTER,       EWPNFRAME,            1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawStyle",           ZVARTYPEID_FLOAT,         GETTER,       EWPNDRAWTYPE,         1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawStyle",           ZVARTYPEID_VOID,          SETTER,       EWPNDRAWTYPE,         1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDamage",              ZVARTYPEID_FLOAT,         GETTER,       EWPNPOWER,            1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDamage",              ZVARTYPEID_VOID,          SETTER,       EWPNPOWER,            1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getID",                  ZVARTYPEID_FLOAT,         GETTER,       EWPNID,               1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setID",                  ZVARTYPEID_VOID,          SETTER,       EWPNID,               1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAngular",             ZVARTYPEID_BOOL,          GETTER,       EWPNANGULAR,          1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAngular",             ZVARTYPEID_VOID,          SETTER,       EWPNANGULAR,          1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getBehind",              ZVARTYPEID_BOOL,          GETTER,       EWPNBEHIND,           1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setBehind",              ZVARTYPEID_VOID,          SETTER,       EWPNBEHIND,           1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getASpeed",              ZVARTYPEID_FLOAT,         GETTER,       EWPNASPEED,           1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setASpeed",              ZVARTYPEID_VOID,          SETTER,       EWPNASPEED,           1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTile",                ZVARTYPEID_FLOAT,         GETTER,       EWPNTILE,             1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTile",                ZVARTYPEID_VOID,          SETTER,       EWPNTILE,             1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlashCSet",           ZVARTYPEID_FLOAT,         GETTER,       EWPNFLASHCSET,        1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlashCSet",           ZVARTYPEID_VOID,          SETTER,       EWPNFLASHCSET,        1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDeadState",           ZVARTYPEID_FLOAT,         GETTER,       EWPNDEAD,             1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDeadState",           ZVARTYPEID_VOID,          SETTER,       EWPNDEAD,             1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       EWPNCSET,             1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCSet",                ZVARTYPEID_VOID,          SETTER,       EWPNCSET,             1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlash",               ZVARTYPEID_BOOL,          GETTER,       EWPNFLASH,            1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlash",               ZVARTYPEID_VOID,          SETTER,       EWPNFLASH,            1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getFlip",                ZVARTYPEID_FLOAT,         GETTER,       EWPNFLIP,             1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setFlip",                ZVARTYPEID_VOID,          SETTER,       EWPNFLIP,             1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalTile",        ZVARTYPEID_FLOAT,         GETTER,       EWPNOTILE,            1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setOriginalTile",        ZVARTYPEID_VOID,          SETTER,       EWPNOTILE,            1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getOriginalCSet",        ZVARTYPEID_FLOAT,         GETTER,       EWPNOCSET,            1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setOriginalCSet",	      ZVARTYPEID_VOID,          SETTER,       EWPNOCSET,            1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getExtend",              ZVARTYPEID_FLOAT,         GETTER,       EWPNEXTEND,           1,      {  ZVARTYPEID_EWPN,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setExtend",              ZVARTYPEID_VOID,          SETTER,       EWPNEXTEND,           1,      {  ZVARTYPEID_EWPN,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitWidth",            ZVARTYPEID_FLOAT,         GETTER,       EWPNHXSZ,             1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitWidth",            ZVARTYPEID_VOID,          SETTER,       EWPNHXSZ,             1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitHeight",           ZVARTYPEID_FLOAT,         GETTER,       EWPNHYSZ,             1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitHeight",           ZVARTYPEID_VOID,          SETTER,       EWPNHYSZ,             1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitZHeight",          ZVARTYPEID_FLOAT,         GETTER,       EWPNHZSZ,             1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitZHeight",          ZVARTYPEID_VOID,          SETTER,       EWPNHZSZ,             1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileWidth",           ZVARTYPEID_FLOAT,         GETTER,       EWPNTXSZ,             1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileWidth",           ZVARTYPEID_VOID,          SETTER,       EWPNTXSZ,             1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getTileHeight",          ZVARTYPEID_FLOAT,         GETTER,       EWPNTYSZ,             1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setTileHeight",          ZVARTYPEID_VOID,          SETTER,       EWPNTYSZ,             1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawXOffset",         ZVARTYPEID_FLOAT,         GETTER,       EWPNXOFS,             1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawXOffset",         ZVARTYPEID_VOID,          SETTER,       EWPNXOFS,             1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawYOffset",         ZVARTYPEID_FLOAT,         GETTER,       EWPNYOFS,             1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawYOffset",         ZVARTYPEID_VOID,          SETTER,       EWPNYOFS,             1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDrawZOffset",         ZVARTYPEID_FLOAT,         GETTER,       EWPNZOFS,             1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDrawZOffset",         ZVARTYPEID_VOID,          SETTER,       EWPNZOFS,             1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitXOffset",          ZVARTYPEID_FLOAT,         GETTER,       EWPNHXOFS,            1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitXOffset",          ZVARTYPEID_VOID,          SETTER,       EWPNHXOFS,            1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHitYOffset",          ZVARTYPEID_FLOAT,         GETTER,       EWPNHYOFS,            1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHitYOffset",          ZVARTYPEID_VOID,          SETTER,       EWPNHYOFS,            1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "isValid",				  ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_EWPN,          -1,                              -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "UseSprite",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       EWPNMISCD,           32,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       EWPNMISCD,           32,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection", 	  ZVARTYPEID_FLOAT,         GETTER,       EWPNCOLLDET,          1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection", 	  ZVARTYPEID_VOID,          SETTER,       EWPNCOLLDET,          1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                                -1,                              -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

EnemyWeaponSymbols::EnemyWeaponSymbols()
{
    table = ewpnTable;
    refVar = REFEWPN;
}

map<int, vector<Opcode *> > EnemyWeaponSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval = LibrarySymbols::generateCode();
    int id=-1;
    //bool isValid(eweapon)
    {
	    Function* function = functions["isValid"];
        int label = function->getLabel();
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
	    Function* function = functions["UseSprite"];
        int label = function->getLabel();
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




AudioSymbols AudioSymbols::singleton = AudioSymbols();

static AccessorTable AudioTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
    { "PlaySound",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_AUDIO,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "EndSound",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      { ZVARTYPEID_AUDIO,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PauseSound",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_AUDIO,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ResumeSound",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_AUDIO,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ContinueSound",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_AUDIO,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "AdjustSound",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_AUDIO,          ZVARTYPEID_FLOAT,        ZVARTYPEID_FLOAT,                           ZVARTYPEID_BOOL,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PauseMusic",         ZVARTYPEID_VOID,          FUNCTION,     0,       		        1,      {  ZVARTYPEID_AUDIO,        -1,    					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "ResumeMusic",         ZVARTYPEID_VOID,          FUNCTION,     0,       		        1,      {  ZVARTYPEID_AUDIO,        -1,    					   -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   
    { "PlayMIDI",               ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_AUDIO,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "PlayEnhancedMusic",      ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_AUDIO,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

AudioSymbols::AudioSymbols()
{
    table = AudioTable;
    refVar = NUL;
}

map<int, vector<Opcode *> > AudioSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval;
    int id=-1;
	    
	
    
    //void AdjustSound(game, int,int,bool)
    {
	    Function* function = functions["AdjustSound"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the params
        Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPopRegister(new VarArgument(INDEX2)));
        code.push_back(new OPopRegister(new VarArgument(INDEX)));
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetRegister(new VarArgument(ADJUSTSFX), new VarArgument(SFTEMP)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    //void PlaySound(game, int)
    {
	    Function* function = functions["PlaySound"];
        int label = function->getLabel();
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
    
    //void EndSound(game, int)
    {
	    Function* function = functions["EndSound"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OEndSoundRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //void PauseSound(game, int)
    {
	    Function* function = functions["PauseSound"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OPauseSoundRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //void ContinueSound(game, int)
    {
	    Function* function = functions["ContinueSound"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OContinueSFX(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //void ResumeSound(game, int)
    {
	    Function* function = functions["ResumeSound"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OResumeSoundRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    //void PauseMusic(game)
    {
	    Function* function = functions["PauseMusic"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OPauseMusic());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //void ResumeMusic(game)
    {
	    Function* function = functions["ResumeMusic"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop pointer, and ignore it
        Opcode *first = new OPopRegister(new VarArgument(NUL));
        first->setLabel(label);
        code.push_back(first);
        code.push_back(new OResumeMusic());
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //void PlayMIDI(game, int)
    {
	    Function* function = functions["PlayMIDI"];
        int label = function->getLabel();
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
	    Function* function = functions["PlayEnhancedMusic"];
        int label = function->getLabel();
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
    return rval;
}


DebugSymbols DebugSymbols::singleton = DebugSymbols();

static AccessorTable DebugTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
	//All of these return a function label error when used:
	{ "getRefFFC",               ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFFFC,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "getRefItem",               ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFITEM,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "getRefItemdata",               ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFITEMDATA,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "getRefNPC",               ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFNPC,            1,      { ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "getRefLWeapon",               ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFLWEAPON,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "getRefEWeapon",               ZVARTYPEID_FLOAT,         GETTER,       DEBUGREFEWEAPON,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "getSP",               ZVARTYPEID_FLOAT,         GETTER,       DEBUGSP,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "setRefFFC",               ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFFFC,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "setRefItem",               ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFITEM,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "setRefItemdata",               ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFITEMDATA,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "setRefNPC",               ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFNPC,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "setRefLWeapon",               ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFLWEAPON,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "setRefEWeapon",               ZVARTYPEID_FLOAT,         SETTER,       DEBUGREFEWEAPON,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "setSP",               ZVARTYPEID_FLOAT,         SETTER,       DEBUGSP,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "getGDR[]",              ZVARTYPEID_FLOAT,         GETTER,       DEBUGGDR,              256,     {  ZVARTYPEID_DEBUG,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "setGDR[]",              ZVARTYPEID_VOID,          SETTER,       DEBUGGDR,              256,     {  ZVARTYPEID_DEBUG,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
	//These all work, but may not be permitted by the rest of the dev team. 
	//At least they are now in a pointer class that denotes that they can be dangerous. 
	{ "GetBoolPointer",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_BOOL,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "SetBoolPointer",      ZVARTYPEID_BOOL,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "GetNPCPointer",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_NPC,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "SetNPCPointer",      ZVARTYPEID_NPC,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "GetLWeaponPointer",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_LWPN,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "SetLWeaponPointer",      ZVARTYPEID_LWPN,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "GetEWeaponPointer",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_EWPN,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "SetEWeaponPointer",      ZVARTYPEID_EWPN,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "GetFFCPointer",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FFC,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "SetFFCPointer",      ZVARTYPEID_FFC,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "GetItemPointer",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_ITEM,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "SetItemPointer",      ZVARTYPEID_ITEM,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "GetItemdataPointer",      ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_ITEMCLASS,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	{ "SetItemdataPointer",      ZVARTYPEID_ITEMCLASS,         FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
	
	//Changing the subscreen and screen offsets seems to do nothing. 
	 { "getSubscreenOffset",               ZVARTYPEID_FLOAT,         GETTER,       PASSSUBOFS,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setSubscreenOffset",               ZVARTYPEID_VOID,          SETTER,       PASSSUBOFS,            1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getSubscreenHeight",               ZVARTYPEID_FLOAT,         GETTER,       GAMESUBSCHEIGHT,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setSubscreenHeight",               ZVARTYPEID_VOID,          SETTER,       GAMESUBSCHEIGHT,            1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getPlayfieldOffset",               ZVARTYPEID_FLOAT,         GETTER,       GAMEPLAYFIELDOFS,            1,      {  ZVARTYPEID_DEBUG,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setPlayfieldOffset",               ZVARTYPEID_VOID,          SETTER,       GAMEPLAYFIELDOFS,            1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "TriggerSecret",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
 { "ChangeFFCScript",              ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_DEBUG,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
 
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

DebugSymbols::DebugSymbols()
{
    table = DebugTable;
    refVar = NUL;
}

map<int, vector<Opcode *> > DebugSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval;
    int id=-1;
	
	
     //int GetPointer(itemclass, itemclass)
    {
	    Function* function = functions["GetItemdataPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetItemDataPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //int SetPointer(itemclass, float)
    {
	    Function* function = functions["SetItemdataPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetItemDataPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int GetPointer(item, item)
    {
	    Function* function = functions["GetItemPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetItemPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //int SetPointer(item, float)
    {
	    Function* function = functions["SetItemPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetItemPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }    
    //int GetPointer(ffc, ffc)
    {
	    Function* function = functions["GetFFCPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetFFCPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //int SetPointer(ffc, float)
    {
	    Function* function = functions["SetFFCPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetFFCPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
          //int GetPointer(eweapon, eweapon)
    {
	    Function* function = functions["GetEWeaponPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetEWeaponPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //int SetPointer(eweapon, float)
    {
	    Function* function = functions["SetEWeaponPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetEWeaponPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
       //int GetPointer(lweapon, lweapon)
    {
	    Function* function = functions["GetLWeaponPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetLWeaponPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //int SetPointer(lweapon, float)
    {
	    Function* function = functions["SetLWeaponPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetLWeaponPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
	 //int GetPointer(npc, ffc)
    {
	    Function* function = functions["GetNPCPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetNPCPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //int SetPointer(npc, float)
    {
	    Function* function = functions["SetNPCPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetNPCPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    //int GetPointer(game, bool)
    {
	    Function* function = functions["GetBoolPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OGetBoolPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    //int SetPointer(game, float)
    {
	    Function* function = functions["SetBoolPointer"];
        int label = function->getLabel();
        vector<Opcode *> code;
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OSetBoolPointer(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label]=code;
    }
    
    
//void TriggerSecret(game, int)
    {
	    Function* function = functions["TriggerSecret"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OTriggerSecretRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    
    
//void ChangeFFCScript(game, int)
    {
	    Function* function = functions["ChangeFFCScript"];
        int label = function->getLabel();
        vector<Opcode *> code;
        //pop off the param
        Opcode *first = new OPopRegister(new VarArgument(EXP1));
        first->setLabel(label);
        code.push_back(first);
        //pop pointer, and ignore it
        code.push_back(new OPopRegister(new VarArgument(NUL)));
        code.push_back(new OChangeFFCScriptRegister(new VarArgument(EXP1)));
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OGotoRegister(new VarArgument(EXP2)));
        rval[label] = code;
    }
    return rval;
}


NPCDataSymbols NPCDataSymbols::singleton = NPCDataSymbols();

static AccessorTable NPCDataTable[] =
{
    //name,                     rettype,                        setorget,     var,              numindex,      params
   { "getCSet",                ZVARTYPEID_FLOAT,         GETTER,       FCSET,                1,      {  ZVARTYPEID_NPCDATA,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   /*
	Use functions that accept two args (ID, value) per field for setters
	use functions that accept one arg (ID) per field as getters
	Plus One master function to set all attriutes using an array?
*/   
   
   //SetName
    { "",                      -1,                               -1,           -1,                   -1,      { -1,                               -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } }
};

NPCDataSymbols::NPCDataSymbols()
{
    table = NPCDataTable;
    refVar = NUL;
}

map<int, vector<Opcode *> > NPCDataSymbols::generateCode()
{
    map<int, vector<Opcode *> > rval;
    int id=-1;

    /*
	//bool isValid(eweapon)
    {
    Function* function = functions["isValid"];
        int label = function->getLabel();
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
    */
    
    return rval;
}

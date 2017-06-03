
#include "../precompiled.h" //always first

#include "GlobalSymbols.h"
#include "ByteCode.h"
#include "Scope.h"
#include "../zsyssimple.h"
#include <assert.h>

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
        string name = entry.name;
        vector<int> paramTypeIds;
        for (int k = 0; entry.params[k] != -1 && k < 20; k++)
            paramTypeIds.push_back(entry.params[k]);

		string varName = name;

		// Strip out the array at the end.
		bool isArray = name.substr(name.size() - 2) == "[]";
		if (isArray)
			varName = name.substr(0, name.size() - 2);

		if (entry.setorget == SETTER && name.substr(0, 3) == "set")
		{
			varName = varName.substr(3); // Strip out "set".
			int varId = scope.getLocalVariableId(varName);
			if (varId == -1)
				varId = scope.addVariable(varName, paramTypeIds[1]);
			int functionId = scope.addSetter(varId, paramTypeIds);
			assert(functionId != -1);
			setters[name] = functionId;
		}
		else if (entry.setorget == GETTER && name.substr(0, 3) == "get")
		{
			varName = varName.substr(3); // Strip out "get".
			int varId = scope.getLocalVariableId(varName);
			if (varId == -1)
				varId = scope.addVariable(varName, entry.rettype);
			int functionId = scope.addGetter(varId, paramTypeIds);
			assert(functionId != -1);
			getters[name] = functionId;
		}
		else
		{
			int functionId = scope.addFunction(varName, entry.rettype, paramTypeIds);
			assert(functionId != -1);
			functions[name] = functionId;
		}
    }
}

int LibrarySymbols::matchFunction(string const& name)
{
    map<string, int>::iterator it = functions.find(name);
    if (it != functions.end())
		return it->second;
	return -1;
}

int LibrarySymbols::matchGetter(string const& name)
{
    map<string, int>::iterator it = getters.find(name);
    if (it != getters.end())
		return it->second;
	return -1;
}

int LibrarySymbols::matchSetter(string const& name)
{
    map<string, int>::iterator it = setters.find(name);
    if (it != setters.end())
		return it->second;
	return -1;
}

int LibrarySymbols::getFunctionId(string const& name) const
{
	map<string, int>::const_iterator it;
	it = functions.find(name);
	if (it != functions.end()) {return it->second;}
	it = getters.find(name);
	if (it != getters.end()) {return it->second;}
	it = setters.find(name);
	if (it != setters.end()) {return it->second;}
	return 0;
}

map<int, vector<Opcode *> > LibrarySymbols::addSymbolsCode(LinkTable &lt)
{
    map<int, vector<Opcode *> > rval;
    
    for(int i=0; table[i].name != ""; i++)
    {
        int var = table[i].var;
        string name = table[i].name;
        bool isIndexed = table[i].numindex > 1;
        int id = getFunctionId(name);
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
                if(table[i].params[1] == ZVARTYPEID_BOOL)
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
    if(refVar!=NUL)
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
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
    if(refVar!=NUL)
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
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
    if(refVar!=NUL)
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
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
    if(refVar!=NUL)
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
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
    if(refVar!=NUL)
        code.push_back(new OSetRegister(new VarArgument(refVar), new VarArgument(EXP2)));
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
        id = functions["Rand"];
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
        id = functions["Quit"];
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
        id = functions["Waitframe"];
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
        id = functions["Waitdraw"];
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
        id = functions["Trace"];
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
        id = functions["TraceB"];
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
        id = functions["TraceS"];
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
        id = functions["TraceNL"];
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
        id = functions["ClearTrace"];
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
        id = functions["TraceToBase"];
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
        id = functions["Sin"];
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
        id = functions["RadianSin"];
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
        id = functions["ArcSin"];
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
        id = functions["Cos"];
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
        id = functions["RadianCos"];
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
        id = functions["ArcCos"];
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
        id = functions["Tan"];
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
        id = functions["ArcTan"];
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
        id = functions["RadianTan"];
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
        id = functions["Max"];
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
        id = functions["Min"];
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
        id = functions["Pow"];
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
        id = functions["InvPow"];
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
        id = functions["Factorial"];
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
        id = functions["Abs"];
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
        id = functions["Log10"];
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
        id = functions["Ln"];
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
        id = functions["Sqrt"];
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
        id = functions["CopyTile"];
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
    //int SwapTile(int first, int second)
    {
        id = functions["SwapTile"];
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
        id = functions["ClearTile"];
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
        int id2 = functions["GetGlobalRAM"];
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
        int id2 = functions["SetGlobalRAM"];
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
        int id2 = functions["GetScriptRAM"];
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
        int id2 = functions["SetScriptRAM"];
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
        id = functions["SetColorBuffer"];
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
        id = functions["SetDepthBuffer"];
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
        id = functions["GetColorBuffer"];
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
        id = functions["GetDepthBuffer"];
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
        id = functions["SizeOfArray"];
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
    	int id = functions["WasTriggered"];
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
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       LINKMISCD,            16,     {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       LINKMISCD,            16,     {  ZVARTYPEID_LINK,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
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
        int id = functions["Warp"];
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
        int id = functions["PitWarp"];
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
        int id = functions["SelectAWeapon"];
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
        int id = functions["SelectBWeapon"];
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
        int id = functions["LoadItem"];
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
        int id = functions["CreateItem"];
        
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
        int id = functions["LoadFFC"];
        
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
        int id = functions["LoadNPC"];
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
        int id = functions["CreateNPC"];
        
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
        int id = functions["LoadLWeapon"];
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
        int id = functions["CreateLWeapon"];
        
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
        int id = functions["LoadEWeapon"];
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
        int id = functions["CreateEWeapon"];
        
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
        int id = functions["ClearSprites"];
        
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
        int id = functions["Rectangle"];
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
        int id = functions["Circle"];
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
        int id = functions["Arc"];
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
        int id = functions["Ellipse"];
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
        int id = functions["Line"];
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
        int id = functions["Spline"];
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
        int id = functions["PutPixel"];
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
        int id = functions["DrawCharacter"];
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
        int id = functions["DrawInteger"];
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
        int id = functions["DrawTile"];
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
        int id = functions["DrawCombo"];
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
        int id = functions["Quad"];
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
        int id = functions["Triangle"];
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
        int id = functions["Quad3D"];
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
        int id = functions["Triangle3D"];
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
        int id = functions["FastTile"];
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
        int id = functions["FastCombo"];
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
        int id = functions["DrawString"];
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
        int id = functions["DrawLayer"];
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
        int id = functions["DrawScreen"];
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
        int id = functions["DrawBitmap"];
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
        int id = functions["SetRenderTarget"];
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
        int id = functions["Message"];
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
        int id = functions["isSolid"];
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
        int id = functions["SetSideWarp"];
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
        int id = functions["SetTileWarp"];
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
        int id = functions["LayerScreen"];
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
        int id = functions["LayerMap"];
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
        int id = functions["TriggerSecrets"];
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
    //void ZapIn(screen)
    {
        int id = functions["ZapIn"];
        int label = lt.functionToLabel(id);
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
        int id = functions["ZapOut"];
        int label = lt.functionToLabel(id);
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
        int id = functions["OpeningWipe"];
        int label = lt.functionToLabel(id);
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
        int id = functions["WavyIn"];
        int label = lt.functionToLabel(id);
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
        int id = functions["WavyOut"];
        int label = lt.functionToLabel(id);
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
        int id = functions["GetSideWarpDMap"];
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
        int id = functions["GetSideWarpScreen"];
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
        int id = functions["GetSideWarpType"];
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
        int id = functions["GetTileWarpDMap"];
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
        int id = functions["GetTileWarpScreen"];
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
        int id = functions["GetTileWarpType"];
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
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       ITEMMISCD,            16,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       ITEMMISCD,            16,      {  ZVARTYPEID_ITEM,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
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
        int id = functions["isValid"];
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
    { "setSprites[]",              ZVARTYPEID_VOID,          SETTER,       IDATASPRITE,              160,     {  ZVARTYPEID_ITEMCLASS,           ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   
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
        int id = functions["GetName"];
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
    { "GetItemScript",           ZVARTYPEID_FLOAT,         FUNCTION,     0,                    1,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GreyscaleOn",                    ZVARTYPEID_VOID,          FUNCTION,     0,       	   	        1,      {  ZVARTYPEID_GAME,         -1,    					          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GreyscaleOff",                    ZVARTYPEID_VOID,          FUNCTION,     0,       	   	        1,      {  ZVARTYPEID_GAME,         -1,    					          -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getButtonPress[]",             ZVARTYPEID_FLOAT,         GETTER,       BUTTONPRESS,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setButtonPress[]",             ZVARTYPEID_VOID,          SETTER,       BUTTONPRESS,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getButtonInput[]",             ZVARTYPEID_FLOAT,         GETTER,       BUTTONINPUT,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setButtonInput[]",             ZVARTYPEID_VOID,          SETTER,       BUTTONINPUT,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getButtonHeld[]",             ZVARTYPEID_FLOAT,         GETTER,       BUTTONHELD,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setButtonHeld[]",             ZVARTYPEID_VOID,          SETTER,       BUTTONHELD,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getReadKey[]",             ZVARTYPEID_FLOAT,         GETTER,       READKEY,         127,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
   
    { "getKeyPress[]",             ZVARTYPEID_FLOAT,         GETTER,       KEYPRESS,         127,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setKeyPress[]",             ZVARTYPEID_VOID,          SETTER,       KEYPRESS,         127,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getDisableItem[]",             ZVARTYPEID_FLOAT,         GETTER,       DISABLEDITEM,        256,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDisableItem[]",             ZVARTYPEID_VOID,          SETTER,       DISABLEDITEM,         256,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
    { "getJoypadPress[]",             ZVARTYPEID_FLOAT,         GETTER,       JOYPADPRESS,         18,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
  
    { "getDMapPalette[]",         ZVARTYPEID_FLOAT,         GETTER,       DMAPLEVELPAL,         512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDMapPalette[]",         ZVARTYPEID_VOID,          SETTER,       DMAPLEVELPAL,         512,      {  ZVARTYPEID_GAME,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    
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
        int id = functions["LoadItemData"];
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
        int id = functions["GetScreenState"];
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
        int id = functions["SetScreenState"];
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
        int id = functions["GetScreenD"];
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
        int id = functions["SetScreenD"];
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
        int id = functions["GetDMapScreenD"];
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
        int id = functions["SetDMapScreenD"];
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
        int id = functions["PlaySound"];
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
        int id = functions["PlayMIDI"];
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
        int id = functions["PlayEnhancedMusic"];
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
        int id = functions["GetDMapMusicFilename"];
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
        int id = functions["GetDMapMusicTrack"];
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
        int id = functions["SetDMapEnhancedMusic"];
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
        int id = functions["GetComboData"];
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
        int id = functions["SetComboData"];
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
        int id = functions["GetComboCSet"];
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
        int id = functions["SetComboCSet"];
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
        int id = functions["GetComboFlag"];
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
        int id = functions["SetComboFlag"];
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
        int id = functions["GetComboType"];
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
        int id = functions["SetComboType"];
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
        int id = functions["GetComboInherentFlag"];
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
        int id = functions["SetComboInherentFlag"];
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
        int id = functions["GetComboSolid"];
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
        int id = functions["SetComboSolid"];
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
        int id = functions["GetScreenFlags"];
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
        int id = functions["GetScreenEFlags"];
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
        int id = functions["Save"];
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
        int id = functions["End"];
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
        int id = functions["ComboTile"];
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
        int id = functions["GetSaveName"];
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
        int id = functions["SetSaveName"];
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
        int id = functions["GetMessage"];
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
        int id = functions["GetDMapName"];
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
        int id = functions["GetDMapTitle"];
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
        int id = functions["GetDMapIntro"];
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
    
    
    
    //void GreyscaleOn(game)
    {
        int id = functions["GreyscaleOn"];
        int label = lt.functionToLabel(id);
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
        int id = functions["GreyscaleOff"];
        int label = lt.functionToLabel(id);
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
    
    
    //bool ShowSaveScreen(game)
    {
        int id = functions["ShowSaveScreen"];
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
        int id = functions["ShowSaveQuitScreen"];
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
        int id = functions["GetFFCScript"];
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
    
    
     //int GetItemScript(game, int)
    {
        int id = functions["GetItemScript"];
        int label = lt.functionToLabel(id);
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
    { "isValid",				  ZVARTYPEID_BOOL,          FUNCTION,     0,                    1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       NPCMISCD,            16,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       NPCMISCD,            16,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getAttributes[]",        ZVARTYPEID_FLOAT,         GETTER,       NPCDD,               10,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setAttributes[]",        ZVARTYPEID_VOID,          SETTER,       NPCDD,               10,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getMiscFlags",           ZVARTYPEID_FLOAT,         GETTER,       NPCMFLAGS,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMiscFlags",           ZVARTYPEID_VOID,          SETTER,       NPCMFLAGS,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection", 	  ZVARTYPEID_FLOAT,         GETTER,       NPCCOLLDET,           1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection",       ZVARTYPEID_VOID,          SETTER,       NPCCOLLDET,           1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getStun",                ZVARTYPEID_FLOAT,         GETTER,       NPCSTUN,              1,      {  ZVARTYPEID_NPC,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setStun",                ZVARTYPEID_VOID,          SETTER,       NPCSTUN,              1,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getDefense[]",           ZVARTYPEID_FLOAT,         GETTER,       NPCDEFENSED,         16,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setDefense[]",           ZVARTYPEID_VOID,          SETTER,       NPCDEFENSED,         16,      {  ZVARTYPEID_NPC,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "GetName",                ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_NPC,     ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getHunger",              ZVARTYPEID_FLOAT,         GETTER,       NPCHUNGER,            1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setHunger",              ZVARTYPEID_VOID,          SETTER,       NPCHUNGER,            1,      {  ZVARTYPEID_NPC,           ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "BreakShield",			  ZVARTYPEID_VOID,          FUNCTION,     0,                    1,      {  ZVARTYPEID_NPC,          -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
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
        int id = functions["isValid"];
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
        int id = functions["GetName"];
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
        int id = functions["BreakShield"];
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
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       LWPNMISCD,           16,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       LWPNMISCD,           16,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection", 	  ZVARTYPEID_FLOAT,         GETTER,       LWPNCOLLDET,          1,      {  ZVARTYPEID_LWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection", 	  ZVARTYPEID_VOID,          SETTER,       LWPNCOLLDET,          1,      {  ZVARTYPEID_LWPN,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
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
        id = functions["isValid"];
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
        id = functions["UseSprite"];
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
    { "getMisc[]",              ZVARTYPEID_FLOAT,         GETTER,       EWPNMISCD,           16,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,        -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setMisc[]",              ZVARTYPEID_VOID,          SETTER,       EWPNMISCD,           16,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_FLOAT,         ZVARTYPEID_FLOAT,    -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "getCollDetection", 	  ZVARTYPEID_FLOAT,         GETTER,       EWPNCOLLDET,          1,      {  ZVARTYPEID_EWPN,         -1,                               -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
    { "setCollDetection", 	  ZVARTYPEID_VOID,          SETTER,       EWPNCOLLDET,          1,      {  ZVARTYPEID_EWPN,          ZVARTYPEID_BOOL,         -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1,                           -1                           } },
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
        id = functions["isValid"];
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
        id = functions["UseSprite"];
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


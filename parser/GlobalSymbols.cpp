#include "GlobalSymbols.h"
#include "ByteCode.h"
#include "../zsyssimple.h"
#include <assert.h>

const int radsperdeg = 572958;

void LibrarySymbols::addSymbolsToScope(Scope *scope, SymbolTable *t)
{
	//waste an ID, OH WELL
	memberids.clear();
	firstid = ScriptParser::getUniqueFuncID()+1;
	int id = firstid;
	for(int i=0; table[i].name != ""; i++,id++)
	{
		vector<int> param;
		for(int k=0; table[i].params[k] != -1; k++)
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

static AccessorTable GlobalTable[] = {
	{"Rand",		ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT, -1} },
	{"Quit",		ScriptParser::TYPE_VOID,	FUNCTION,	0,	1,	{-1} },
	{"Waitframe",	ScriptParser::TYPE_VOID,	FUNCTION,	0,	1,	{-1} },
	{"Trace",		ScriptParser::TYPE_VOID,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT,-1} },
	{"Sin",			ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT,-1} },
	{"Cos",			ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT,-1} },
	{"Tan",			ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT,-1} },
	{"RadianSin",	ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT,-1} },
	{"RadianCos",	ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT,-1} },
	{"RadianTan",	ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT,-1} },
	{"Max",			ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"Min",			ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"Pow",			ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"InvPow",		ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"Factorial",	ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT, -1} },
	{"Abs",			ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT, -1} },
	{"Sqrt",		ScriptParser::TYPE_FLOAT,	FUNCTION,	0,	1,	{ScriptParser::TYPE_FLOAT, -1} },
  { "",                     -1,                              -1,            -1,                -1,   { -1,                         -1,                          -1,                          -1,                          -1 } }
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
	return rval;
}

FFCSymbols FFCSymbols::singleton = FFCSymbols();

static AccessorTable FFCTable[] = {
	{"getData",		ScriptParser::TYPE_FLOAT,	GETTER,		DATA,	1,	{ScriptParser::TYPE_FFC, -1} },
	{"setData",		ScriptParser::TYPE_VOID,	SETTER,		DATA,	1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getCSet",		ScriptParser::TYPE_FLOAT,	GETTER,		FCSET,	1,	{ScriptParser::TYPE_FFC, -1} },
	{"setCSet",		ScriptParser::TYPE_VOID,	SETTER,		FCSET,	1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getDelay",	ScriptParser::TYPE_FLOAT,	GETTER,		DELAY,	1,	{ScriptParser::TYPE_FFC, -1} },
	{"setDelay",	ScriptParser::TYPE_VOID,	SETTER,		DELAY,	1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getX",		ScriptParser::TYPE_FLOAT,	GETTER,		FX,		1,	{ScriptParser::TYPE_FFC, -1} },
	{"setX",		ScriptParser::TYPE_VOID,	SETTER,		FX,		1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getY",		ScriptParser::TYPE_FLOAT,	GETTER,		FY,		1,	{ScriptParser::TYPE_FFC, -1} },
	{"setY",		ScriptParser::TYPE_VOID,	SETTER,		FY,		1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getVx",		ScriptParser::TYPE_FLOAT,	GETTER,		XD,		1,	{ScriptParser::TYPE_FFC, -1} },
	{"setVx",		ScriptParser::TYPE_VOID,	SETTER,		XD,		1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getVy",		ScriptParser::TYPE_FLOAT,	GETTER,		YD,		1,	{ScriptParser::TYPE_FFC, -1} },
	{"setVy",		ScriptParser::TYPE_VOID,	SETTER,		YD,		1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getAx",		ScriptParser::TYPE_FLOAT,	GETTER,		XD2,	1,	{ScriptParser::TYPE_FFC, -1} },
	{"setAx",		ScriptParser::TYPE_VOID,	SETTER,		XD2,	1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getAy",		ScriptParser::TYPE_FLOAT,	GETTER,		YD2,	1,	{ScriptParser::TYPE_FFC, -1} },
	{"setAy",		ScriptParser::TYPE_VOID,	SETTER,		YD2,	1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"WasTriggered",ScriptParser::TYPE_BOOL,	FUNCTION,	0,		1,	{ScriptParser::TYPE_FFC, -1} },
	{"getFlags[]",	ScriptParser::TYPE_BOOL,	GETTER,		FFFLAGSD,2,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"setFlags[]",	ScriptParser::TYPE_VOID,	SETTER,		FFFLAGSD,2,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_BOOL, -1} },
	{"getTileWidth",ScriptParser::TYPE_FLOAT,	GETTER,		FFTWIDTH,1,	{ScriptParser::TYPE_FFC, -1} },
	{"setTileWidth",ScriptParser::TYPE_VOID,	SETTER,		FFTWIDTH,1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getTileHeight",ScriptParser::TYPE_FLOAT,	GETTER,		FFTHEIGHT,1,{ScriptParser::TYPE_FFC, -1} },
	{"setTileHeight",ScriptParser::TYPE_VOID,	SETTER,		FFTHEIGHT,1,{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getEffectWidth",ScriptParser::TYPE_FLOAT,	GETTER,		FFCWIDTH,1, {ScriptParser::TYPE_FFC, -1} },
	{"setEffectWidth",ScriptParser::TYPE_VOID,	SETTER,		FFCWIDTH,1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getEffectHeight",ScriptParser::TYPE_FLOAT,GETTER,		FFCHEIGHT,1,{ScriptParser::TYPE_FFC, -1} },
	{"setEffectHeight",ScriptParser::TYPE_VOID, SETTER,		FFCHEIGHT,1,{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
	{"getLink",		ScriptParser::TYPE_FLOAT,	GETTER,		FFLINK,	1,	{ScriptParser::TYPE_FFC, -1} },
	{"setLink",		ScriptParser::TYPE_VOID,	SETTER,		FFLINK,	1,	{ScriptParser::TYPE_FFC, ScriptParser::TYPE_FLOAT, -1} },
  { "",                     -1,                              -1,            -1,                -1,   { -1,                         -1,                          -1,                          -1,                          -1 } }
};

FFCSymbols::FFCSymbols()
{
	table = FFCTable;
	refVar = REFFFC;
}

map<int, vector<Opcode *> > FFCSymbols::addSymbolsCode(LinkTable &lt)
{
	map<int, vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
	//bool WasTriggered(fc)
	{
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
	}
	return rval;
}

LinkSymbols LinkSymbols::singleton = LinkSymbols();

static AccessorTable LinkSTable[] = {
	{"getX",		ScriptParser::TYPE_FLOAT,	GETTER,		LINKX,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setX",		ScriptParser::TYPE_VOID,	SETTER,		LINKX,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_FLOAT, -1} },
	{"getY",		ScriptParser::TYPE_FLOAT,	GETTER,		LINKY,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setY",		ScriptParser::TYPE_VOID,	SETTER,		LINKY,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_FLOAT, -1} },
	{"getDir",		ScriptParser::TYPE_FLOAT,	GETTER,		LINKDIR,1,	{ScriptParser::TYPE_LINK, -1} },
	{"setDir",		ScriptParser::TYPE_VOID,	SETTER,		LINKDIR,1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_FLOAT, -1} },
	{"getHP",		ScriptParser::TYPE_FLOAT,	GETTER,		LINKHP,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setHP",		ScriptParser::TYPE_VOID,	SETTER,		LINKHP,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_FLOAT, -1} },
	{"getMP",		ScriptParser::TYPE_FLOAT,	GETTER,		LINKMP,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setMP",		ScriptParser::TYPE_VOID,	SETTER,		LINKMP,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_FLOAT, -1} },
	{"getMaxHP",	ScriptParser::TYPE_FLOAT,	GETTER,		LINKMAXHP,1,{ScriptParser::TYPE_LINK, -1} },
	{"setMaxHP",	ScriptParser::TYPE_VOID,	SETTER,		LINKMAXHP,1,{ScriptParser::TYPE_LINK, ScriptParser::TYPE_FLOAT, -1} },
	{"getMaxMP",	ScriptParser::TYPE_FLOAT,	GETTER,		LINKMAXMP,1,{ScriptParser::TYPE_LINK, -1} },
	{"setMaxMP",	ScriptParser::TYPE_VOID,	SETTER,		LINKMAXMP,1,{ScriptParser::TYPE_LINK, ScriptParser::TYPE_FLOAT, -1} },
	{"getAction",	ScriptParser::TYPE_FLOAT,	GETTER,		LINKACTION,1,{ScriptParser::TYPE_LINK, -1} },
	{"setAction",	ScriptParser::TYPE_VOID,	SETTER,		LINKACTION,1,{ScriptParser::TYPE_LINK, ScriptParser::TYPE_FLOAT, -1} },
	{"Warp",		ScriptParser::TYPE_VOID,	FUNCTION,	0,		1,	{ScriptParser::TYPE_LINK,ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"PitWarp",		ScriptParser::TYPE_VOID,	FUNCTION,	0,		1,	{ScriptParser::TYPE_LINK,ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"getInputStart",ScriptParser::TYPE_BOOL,GETTER,	INPUTSTART,1,{ScriptParser::TYPE_LINK, -1} },
	{"setInputStart",ScriptParser::TYPE_VOID,SETTER,		INPUTSTART,1,{ScriptParser::TYPE_LINK, ScriptParser::TYPE_BOOL, -1} },
	{"getInputUp",	ScriptParser::TYPE_BOOL,	GETTER,		INPUTUP,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setInputUp", ScriptParser::TYPE_VOID,	SETTER,		INPUTUP,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_BOOL, -1} },
	{"getInputDown",ScriptParser::TYPE_BOOL,	GETTER,		INPUTDOWN,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setInputDown",ScriptParser::TYPE_VOID,	SETTER,		INPUTDOWN,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_BOOL, -1} },
	{"getInputLeft",ScriptParser::TYPE_BOOL,	GETTER,		INPUTLEFT,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setInputLeft", ScriptParser::TYPE_VOID,	SETTER,		INPUTLEFT,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_BOOL, -1} },
	{"getInputRight",ScriptParser::TYPE_BOOL,	GETTER,		INPUTRIGHT,1,	{ScriptParser::TYPE_LINK, -1} },
	{"setInputRight", ScriptParser::TYPE_VOID,	SETTER,		INPUTRIGHT,1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_BOOL, -1} },
	{"getInputA",	ScriptParser::TYPE_BOOL,	GETTER,		INPUTA,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setInputA",	ScriptParser::TYPE_VOID,	SETTER,		INPUTA,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_BOOL, -1} },
	{"getInputB",	ScriptParser::TYPE_BOOL,	GETTER,		INPUTB,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setInputB",	ScriptParser::TYPE_VOID,	SETTER,		INPUTB,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_BOOL, -1} },
	{"getInputL",	ScriptParser::TYPE_BOOL,	GETTER,		INPUTL,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setInputL",	ScriptParser::TYPE_VOID,	SETTER,		INPUTL,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_BOOL, -1} },
	{"getInputR",	ScriptParser::TYPE_BOOL,	GETTER,		INPUTR,	1,	{ScriptParser::TYPE_LINK, -1} },
	{"setInputR",	ScriptParser::TYPE_VOID,	SETTER,		INPUTR,	1,	{ScriptParser::TYPE_LINK, ScriptParser::TYPE_BOOL, -1} },
	{"getItem[]",	ScriptParser::TYPE_BOOL,	GETTER,		LINKITEMD,256,{ScriptParser::TYPE_LINK, ScriptParser::TYPE_FLOAT, -1} },
	{"setItem[]",	ScriptParser::TYPE_VOID,	SETTER,		LINKITEMD,256,{ScriptParser::TYPE_LINK, ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_BOOL, -1} },
  { "",                     -1,                              -1,            -1,                -1,   { -1,                         -1,                          -1,                          -1,                          -1 } }
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
	return rval;
}

ScreenSymbols ScreenSymbols::singleton = ScreenSymbols();

static AccessorTable ScreenTable[] = {
	{"getD[]",		ScriptParser::TYPE_FLOAT,	GETTER,		SDD,		8,	{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, -1} },
	{"setD[]",		ScriptParser::TYPE_VOID,	SETTER,		SDD,		8,	{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"getComboD[]",	ScriptParser::TYPE_FLOAT,	GETTER,		COMBODD,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, -1} },
	{"setComboD[]",	ScriptParser::TYPE_VOID,	SETTER,		COMBODD,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"getComboC[]",	ScriptParser::TYPE_FLOAT,	GETTER,		COMBOCD,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, -1} },
	{"setComboC[]",	ScriptParser::TYPE_VOID,	SETTER,		COMBOCD,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"getComboF[]",	ScriptParser::TYPE_FLOAT,	GETTER,		COMBOFD,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, -1} },
	{"setComboF[]",	ScriptParser::TYPE_VOID,	SETTER,		COMBOFD,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"getComboI[]", ScriptParser::TYPE_FLOAT,	GETTER,		COMBOID,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, -1} },
	{"setComboI[]",	ScriptParser::TYPE_VOID,	SETTER,		COMBOID,	176,{ScriptParser::TYPE_SCREEN,	ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"getComboT[]",	ScriptParser::TYPE_FLOAT,	GETTER,		COMBOTD,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, -1} },
	{"setComboT[]",	ScriptParser::TYPE_VOID,	SETTER,		COMBOTD,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"getComboS[]",	ScriptParser::TYPE_FLOAT,	GETTER,		COMBOSD,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, -1} },
	{"setComboS[]",	ScriptParser::TYPE_VOID,	SETTER,		COMBOSD,	176,{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, ScriptParser::TYPE_FLOAT, -1} },
	{"NumItems",	ScriptParser::TYPE_FLOAT,	GETTER,		ITEMCOUNT,	1,	{ScriptParser::TYPE_SCREEN, -1} },
	{"LoadItem",	ScriptParser::TYPE_ITEM,	FUNCTION,	0,			1,	{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, -1} },
	{"CreateItem",	ScriptParser::TYPE_ITEM,	FUNCTION,	0,			1,	{ScriptParser::TYPE_SCREEN,	ScriptParser::TYPE_FLOAT, -1} },
	{"LoadFFC",		ScriptParser::TYPE_FFC,		FUNCTION,	0,			1,	{ScriptParser::TYPE_SCREEN,	ScriptParser::TYPE_FLOAT, -1} },
	{"NumNPCs",		ScriptParser::TYPE_FLOAT,	GETTER,		NPCCOUNT,	1,	{ScriptParser::TYPE_SCREEN,	-1} },
	{"LoadNPC",		ScriptParser::TYPE_NPC,		FUNCTION,	0,			1,	{ScriptParser::TYPE_SCREEN,	ScriptParser::TYPE_FLOAT, -1} },
	{"CreateNPC",	ScriptParser::TYPE_NPC,		FUNCTION,	0,			1,	{ScriptParser::TYPE_SCREEN, ScriptParser::TYPE_FLOAT, -1} },
  { "",                     -1,                              -1,            -1,                -1,   { -1,                         -1,                          -1,                          -1,                          -1 } }
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
	return rval;
}

ItemSymbols ItemSymbols::singleton = ItemSymbols();

static AccessorTable itemTable[] = {
	{"getX",		ScriptParser::TYPE_FLOAT,	GETTER,		ITEMX,		1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setX",		ScriptParser::TYPE_VOID,	SETTER,		ITEMX,		1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getY",		ScriptParser::TYPE_FLOAT,	GETTER,		ITEMY,		1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setY",		ScriptParser::TYPE_VOID,	SETTER,		ITEMY,		1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getDrawType",	ScriptParser::TYPE_FLOAT,	GETTER,		ITEMDRAWTYPE,1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setDrawType",	ScriptParser::TYPE_VOID,	SETTER,		ITEMDRAWTYPE,1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getClass",	ScriptParser::TYPE_ITEMCLASS,GETTER,	ITEMID,		1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setClass",	ScriptParser::TYPE_VOID,	SETTER,		ITEMID,		1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_ITEMCLASS, -1} },
	{"getTile",		ScriptParser::TYPE_FLOAT,	GETTER,		ITEMTILE,	1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setTile",		ScriptParser::TYPE_VOID,	SETTER,		ITEMTILE,	1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getCSet",		ScriptParser::TYPE_FLOAT,	GETTER,		ITEMCSET,	1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setCSet",		ScriptParser::TYPE_VOID,	SETTER,		ITEMCSET,	1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getFlashCSet",ScriptParser::TYPE_FLOAT,	GETTER,		ITEMFLASHCSET,1,{ScriptParser::TYPE_ITEM, -1} },
	{"setFlashCSet",ScriptParser::TYPE_VOID,	SETTER,		ITEMFLASHCSET,1,{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getNumFrames",ScriptParser::TYPE_FLOAT,	GETTER,		ITEMFRAMES,	1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setNumFrames",ScriptParser::TYPE_VOID,	SETTER,		ITEMFRAMES,	1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getFrame",	ScriptParser::TYPE_FLOAT,	GETTER,		ITEMFRAME,	1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setFrame",	ScriptParser::TYPE_VOID,	SETTER,		ITEMFRAME,	1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getASpeed",	ScriptParser::TYPE_FLOAT,	GETTER,		ITEMASPEED,	1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setASpeed",	ScriptParser::TYPE_VOID,	SETTER,		ITEMASPEED,	1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getDelay",	ScriptParser::TYPE_FLOAT,	GETTER,		ITEMDELAY,	1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setDelay",	ScriptParser::TYPE_VOID,	SETTER,		ITEMDELAY,	1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getFlash",	ScriptParser::TYPE_BOOL,	GETTER,		ITEMFLASH,	1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setFlash",	ScriptParser::TYPE_VOID,	SETTER,		ITEMFLASH,	1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_BOOL, -1} },
	{"getFlip",		ScriptParser::TYPE_FLOAT,	GETTER,		ITEMFLIP,	1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setFlip",		ScriptParser::TYPE_VOID,	SETTER,		ITEMFLIP,	1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
	{"getExtend",	ScriptParser::TYPE_FLOAT,	GETTER,		ITEMEXTEND,	1,	{ScriptParser::TYPE_ITEM, -1} },
	{"setExtend",	ScriptParser::TYPE_VOID,	SETTER,		ITEMEXTEND,	1,	{ScriptParser::TYPE_ITEM, ScriptParser::TYPE_FLOAT, -1} },
  { "",                     -1,                              -1,            -1,                -1,   { -1,                         -1,                          -1,                          -1,                          -1 } }
};

ItemSymbols::ItemSymbols() {
	table = itemTable;
	refVar = REFITEM;
}

ItemclassSymbols ItemclassSymbols::singleton = ItemclassSymbols();

static AccessorTable itemclassTable[] = {
	{"getFamily",	ScriptParser::TYPE_FLOAT,	GETTER,		ITEMCLASSFAMILY,	1,	{ScriptParser::TYPE_ITEMCLASS, -1, -1, -1} },
	{"setFamily",	ScriptParser::TYPE_VOID,	SETTER,		ITEMCLASSFAMILY,	1,	{ScriptParser::TYPE_ITEMCLASS, ScriptParser::TYPE_FLOAT, -1, -1} },
	{"getLevel",	ScriptParser::TYPE_FLOAT,	GETTER,		ITEMCLASSFAMTYPE,	1,	{ScriptParser::TYPE_ITEMCLASS, -1, -1, -1} },
	{"setLevel",	ScriptParser::TYPE_VOID,	SETTER,		ITEMCLASSFAMTYPE,	1,	{ScriptParser::TYPE_ITEMCLASS, ScriptParser::TYPE_FLOAT, -1, -1} },
	{"getAmount",	ScriptParser::TYPE_FLOAT,	GETTER,		ITEMCLASSAMOUNT,	1,	{ScriptParser::TYPE_ITEMCLASS, -1, -1, -1} },
	{"setAmount",	ScriptParser::TYPE_VOID,	SETTER,		ITEMCLASSAMOUNT,	1,	{ScriptParser::TYPE_ITEMCLASS, ScriptParser::TYPE_FLOAT, -1, -1} },
	{"getMax",		ScriptParser::TYPE_FLOAT,	GETTER,		ITEMCLASSMAX,		1,	{ScriptParser::TYPE_ITEMCLASS, -1, -1, -1} },
	{"setMax",		ScriptParser::TYPE_VOID,	SETTER,		ITEMCLASSMAX,		1,	{ScriptParser::TYPE_ITEMCLASS, ScriptParser::TYPE_FLOAT, -1, -1} },
	{"getMaxIncrement",	ScriptParser::TYPE_FLOAT,GETTER,	ITEMCLASSSETMAX,	1,	{ScriptParser::TYPE_ITEMCLASS, -1, -1, -1} },
	{"setMaxIncrement",	ScriptParser::TYPE_VOID,SETTER,		ITEMCLASSSETMAX,	1,	{ScriptParser::TYPE_ITEMCLASS, ScriptParser::TYPE_FLOAT, -1, -1} },
	{"getKeep",		ScriptParser::TYPE_BOOL,	GETTER,		ITEMCLASSSETGAME,	1,	{ScriptParser::TYPE_ITEMCLASS, -1, -1, -1} },
	{"setKeep",		ScriptParser::TYPE_VOID,	SETTER,		ITEMCLASSSETGAME,	1,	{ScriptParser::TYPE_ITEMCLASS, ScriptParser::TYPE_BOOL, -1, -1} },
	{"getCounter",	ScriptParser::TYPE_FLOAT,	GETTER,		ITEMCLASSCOUNTER,	1,	{ScriptParser::TYPE_ITEMCLASS, -1, -1, -1} },
	{"setCounter",	ScriptParser::TYPE_VOID,	SETTER,		ITEMCLASSCOUNTER,	1,	{ScriptParser::TYPE_ITEMCLASS, ScriptParser::TYPE_FLOAT, -1, -1} },
  { "",                     -1,                              -1,            -1,                -1,   { -1,                         -1,                          -1,                          -1,                          -1 } }
};

ItemclassSymbols::ItemclassSymbols() {
	table = itemclassTable;
	refVar = REFITEMCLASS;
}

GameSymbols GameSymbols::singleton = GameSymbols();

static AccessorTable gameTable[] = {
  { "GetCurScreen",         ScriptParser::TYPE_FLOAT,        GETTER,        CURSCR,            1,    { ScriptParser::TYPE_GAME,    -1,                          -1,                          -1,                          -1 } },   
  { "GetCurMap",            ScriptParser::TYPE_FLOAT,        GETTER,        CURMAP,            1,    { ScriptParser::TYPE_GAME,    -1,                          -1,                          -1,                          -1 } },   
  { "GetCurDMap",           ScriptParser::TYPE_FLOAT,        GETTER,        CURDMAP,           1,    { ScriptParser::TYPE_GAME,    -1,                          -1,                          -1,                          -1 } },   
  { "getNumDeaths",         ScriptParser::TYPE_FLOAT,        GETTER,        GAMEDEATHS,        1,    { ScriptParser::TYPE_GAME,    -1,                          -1,                          -1,                          -1 } },   
  { "setNumDeaths",         ScriptParser::TYPE_VOID,         SETTER,        GAMEDEATHS,        1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "getCheat",             ScriptParser::TYPE_FLOAT,        GETTER,        GAMECHEAT,         1,    { ScriptParser::TYPE_GAME,    -1,                          -1,                          -1,                          -1 } },   
  { "setCheat",             ScriptParser::TYPE_VOID,         SETTER,        GAMECHEAT,         1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "getTime",              ScriptParser::TYPE_FLOAT,        GETTER,        GAMETIME,          1,    { ScriptParser::TYPE_GAME,    -1,                          -1,                          -1,                          -1 } },   
  { "setTime",              ScriptParser::TYPE_VOID,         SETTER,        GAMETIME,          1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "getHasPlayed",         ScriptParser::TYPE_BOOL,         GETTER,        GAMEHASPLAYED,     1,    { ScriptParser::TYPE_GAME,    -1,                          -1,                          -1,                          -1 } },   
  { "setHasPlayed",         ScriptParser::TYPE_VOID,         SETTER,        GAMEHASPLAYED,     1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_BOOL,     -1,                          -1,                          -1 } },   
  { "getTimeValid",         ScriptParser::TYPE_BOOL,         GETTER,        GAMETIMEVALID,     1,    { ScriptParser::TYPE_GAME,    -1,                          -1,                          -1,                          -1 } },   
  { "setTimeValid",         ScriptParser::TYPE_VOID,         SETTER,        GAMETIMEVALID,     1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_BOOL,     -1,                          -1,                          -1 } },   
  { "getGuyCount[]",        ScriptParser::TYPE_FLOAT,        GETTER,        GAMEGUYCOUNT,      2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "setGuyCount[]",        ScriptParser::TYPE_VOID,         SETTER,        GAMEGUYCOUNT,      2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    -1,                          -1 } },   
  { "getContinueScreen",    ScriptParser::TYPE_FLOAT,        GETTER,        GAMECONTSCR,       1,    { ScriptParser::TYPE_GAME,    -1,                          -1,                          -1,                          -1 } },   
  { "setContinueScreen",    ScriptParser::TYPE_VOID,         SETTER,        GAMECONTSCR,       1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "getContinueDMap",      ScriptParser::TYPE_FLOAT,        GETTER,        GAMECONTDMAP,      1,    { ScriptParser::TYPE_GAME,    -1,                          -1,                          -1,                          -1 } },   
  { "setContinueDMap",      ScriptParser::TYPE_VOID,         SETTER,        GAMECONTDMAP,      1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "getCounter[]",         ScriptParser::TYPE_FLOAT,        GETTER,        GAMECOUNTERD,      2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "setCounter[]",         ScriptParser::TYPE_VOID,         SETTER,        GAMECOUNTERD,      2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    -1,                          -1 } },   
  { "getMCounter[]",        ScriptParser::TYPE_FLOAT,        GETTER,        GAMEMCOUNTERD,     2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "setMCounter[]",        ScriptParser::TYPE_VOID,         SETTER,        GAMEMCOUNTERD,     2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    -1,                          -1 } },   
  { "getDCounter[]",        ScriptParser::TYPE_FLOAT,        GETTER,        GAMEDCOUNTERD,     2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "setDCounter[]",        ScriptParser::TYPE_VOID,         SETTER,        GAMEDCOUNTERD,     2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    -1,                          -1 } },   
  { "getGeneric[]",         ScriptParser::TYPE_FLOAT,        GETTER,        GAMEGENERICD,      2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "setGeneric[]",         ScriptParser::TYPE_VOID,         SETTER,        GAMEGENERICD,      2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    -1,                          -1 } },   
  { "getLItems[]",          ScriptParser::TYPE_FLOAT,        GETTER,        GAMELITEMSD,       2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "setLItems[]",          ScriptParser::TYPE_VOID,         SETTER,        GAMELITEMSD,       2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    -1,                          -1 } },   
  { "getLKeys[]",           ScriptParser::TYPE_FLOAT,        GETTER,        GAMELKEYSD,        2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "setLKeys[]",           ScriptParser::TYPE_VOID,         SETTER,        GAMELKEYSD,        2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    -1,                          -1 } },   
  { "getCurMapFlag[]",      ScriptParser::TYPE_BOOL,	     GETTER,        GAMEMAPFLAGD,      2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "setCurMapFlag[]",      ScriptParser::TYPE_VOID,         SETTER,        GAMEMAPFLAGD,      2,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_BOOL,	 -1,                          -1 } },   
  { "GetMapFlag",           ScriptParser::TYPE_BOOL,         FUNCTION,      0,                 1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    -1,                          -1 } },   
  { "SetMapFlag",           ScriptParser::TYPE_VOID,         FUNCTION,      0,                 1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_BOOL,    -1 } },   
  { "GetScreenD",           ScriptParser::TYPE_FLOAT,        FUNCTION,      0,                 1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    -1,                          -1 } },   
  { "SetScreenD",           ScriptParser::TYPE_VOID,         FUNCTION,      0,                 1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    ScriptParser::TYPE_FLOAT,    -1 } },   
  { "LoadItemClass",        ScriptParser::TYPE_ITEMCLASS,    FUNCTION,      0,                 1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "PlaySound",            ScriptParser::TYPE_VOID,         FUNCTION,      0,                 1,    { ScriptParser::TYPE_GAME,    ScriptParser::TYPE_FLOAT,    -1,                          -1,                          -1 } },   
  { "",                     -1,                              -1,            -1,                -1,   { -1,                         -1,                          -1,                          -1,                          -1 } }
};

GameSymbols::GameSymbols() {
	table = gameTable;
	refVar = NUL;
}

map<int, vector<Opcode *> > GameSymbols::addSymbolsCode(LinkTable &lt)
{
	map<int,vector<Opcode *> > rval = LibrarySymbols::addSymbolsCode(lt);
	//itemclass LoadItemClass(game, int)
	{
		int id = memberids["LoadItemClass"];
		int label = lt.functionToLabel(id);
		vector<Opcode *> code;
		//pop off the param
		Opcode *first = new OPopRegister(new VarArgument(EXP1));
		first->setLabel(label);
		code.push_back(first);
		//pop pointer, and ignore it
		code.push_back(new OPopRegister(new VarArgument(NUL)));
		code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(REFITEMCLASS)));
		code.push_back(new OPopRegister(new VarArgument(EXP2)));
		code.push_back(new OGotoRegister(new VarArgument(EXP2)));
		rval[label] = code;
	}
	//bool GetMapFlag(game, int,int)
	{
		int id = memberids["GetMapFlag"];
		int label = lt.functionToLabel(id);
		int done = ScriptParser::getUniqueLabelID();
		vector<Opcode *> code;
		//pop off the params
		Opcode *first = new OPopRegister(new VarArgument(INDEX2));
		first->setLabel(label);
		code.push_back(first);
		code.push_back(new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		code.push_back(new OPopRegister(new VarArgument(NUL)));
		code.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(GAMEMAPFLAGDD)));
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
	//void SetMapFlag(game, int,int,bool)
	{
		int id = memberids["SetMapFlag"];
		int label = lt.functionToLabel(id);
		int done = ScriptParser::getUniqueLabelID();
		vector<Opcode *> code;
		//pop off the params
		Opcode *first = new OPopRegister(new VarArgument(SFTEMP));
		first->setLabel(label);
		code.push_back(first);
		code.push_back(new OPopRegister(new VarArgument(INDEX2)));
		code.push_back(new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		code.push_back(new OPopRegister(new VarArgument(NUL)));
		code.push_back(new OCompareImmediate(new VarArgument(SFTEMP), new LiteralArgument(0)));
		code.push_back(new OGotoTrueImmediate(new LabelArgument(done)));
		code.push_back(new OSetImmediate(new VarArgument(SFTEMP), new LiteralArgument(10000)));
		Opcode *next = new OSetRegister(new VarArgument(GAMEMAPFLAGDD), new VarArgument(SFTEMP));
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
	//void PlaySOund(game, int)
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
	return rval;
}

NPCSymbols NPCSymbols::singleton = NPCSymbols();

static AccessorTable npcTable[] = {
	{ "getX",				  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCX,				 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setX",				  ScriptParser::TYPE_VOID,		   SETTER,		  NPCX,				 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getY",				  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCY,			     1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setY",				  ScriptParser::TYPE_VOID,		   SETTER,		  NPCY,				 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getDir",				  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCDIR,		     1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setDir",				  ScriptParser::TYPE_VOID,		   SETTER,		  NPCDIR,			 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getRate",			  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCRATE,			 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setRate",			  ScriptParser::TYPE_VOID,		   SETTER,		  NPCRATE,			 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getASpeed",			  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCFRAMERATE,		 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setFSpeed",			  ScriptParser::TYPE_VOID,		   SETTER,		  NPCFRAMERATE,		 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getHaltrate",		  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCHALTRATE,		 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setHaltrate",		  ScriptParser::TYPE_VOID,		   SETTER,		  NPCHALTRATE,		 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getDrawStyle",		  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCDRAWTYPE,		 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setDrawStyle",		  ScriptParser::TYPE_VOID,		   SETTER,		  NPCDRAWTYPE,		 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getHP",				  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCHP,			 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setHP",				  ScriptParser::TYPE_VOID,		   SETTER,		  NPCHP,			 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getDamage",			  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCDP,			 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setDamage",			  ScriptParser::TYPE_VOID,		   SETTER,		  NPCDP,			 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getWeaponDamage",	  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCWDP,			 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setWeaponDamage",	  ScriptParser::TYPE_VOID,		   SETTER,		  NPCWDP,			 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getTile",			  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCTILE,			 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setTile",			  ScriptParser::TYPE_VOID,		   SETTER,		  NPCTILE,			 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getWeapon",			  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCWEAPON,		 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setWeapon",			  ScriptParser::TYPE_VOID,		   SETTER,		  NPCWEAPON,		 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getItemSet",			  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCITEMSET,		 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setItemSet",			  ScriptParser::TYPE_VOID,		   SETTER,		  NPCITEMSET,		 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getCSet",			  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCCSET,			 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setCSet",			  ScriptParser::TYPE_VOID,		   SETTER,		  NPCCSET,			 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getBossPal",			  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCBOSSPAL,		 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setBossPal",			  ScriptParser::TYPE_VOID,		   SETTER,		  NPCBOSSPAL,		 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getSFX",				  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCBGSFX,			 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setSFX",				  ScriptParser::TYPE_VOID,		   SETTER,		  NPCBGSFX,			 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
	{ "getExtend",			  ScriptParser::TYPE_FLOAT,		   GETTER,		  NPCEXTEND,		 1,	   { ScriptParser::TYPE_NPC,	 -1,						  -1,						   -1,							-1 } },
	{ "setExtend",			  ScriptParser::TYPE_VOID,		   SETTER,		  NPCEXTEND,		 1,	   { ScriptParser::TYPE_NPC,	 ScriptParser::TYPE_FLOAT,	  -1,						   -1,							-1 } },
    { "",                     -1,                              -1,            -1,                -1,   { -1,                         -1,                          -1,                          -1,                          -1 } }
};

NPCSymbols::NPCSymbols() {
	table = npcTable;
	refVar = REFNPC;
}

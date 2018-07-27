//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "../zsyssimple.h"
#include "ByteCode.h"
#include "CompileError.h"
#include "GlobalSymbols.h"
#include "y.tab.hpp"
#include <iostream>
#include <assert.h>
#include <string>
#include <cstdlib>

#include "ASTVisitors.h"
#include "DataStructs.h"
#include "Scope.h"
#include "SemanticAnalyzer.h"
#include "BuildVisitors.h"
#include "ZScript.h"
using namespace std;
using namespace ZScript;
//#define PARSER_DEBUG

ASTProgram* resAST;

ScriptsData* compile(const char *filename);

#ifdef PARSER_DEBUG
int main(int argc, char *argv[])
{
    if (argc < 2) return -1;
    compile(argv[1]);
}
#endif

ScriptsData* compile(const char *filename)
{
    ScriptParser::resetState();

    box_out("Pass 1: Parsing");
    box_eol();
    
    if (go(filename) != 0 || !resAST)
    {
		CompileError::CantOpenSource.print(NULL);
        return NULL;
    }
    
    ASTProgram* theAST = resAST;
    
    box_out("Pass 2: Preprocessing");
    box_eol();
    
    if (!ScriptParser::preprocess(theAST, RECURSIONLIMIT))
    {
        delete theAST;
        return NULL;
    }
    
    box_out("Pass 3: Analyzing Code");
    box_eol();
    
    SimpleCompileErrorHandler handler;
    Program program(*theAST, handler);
    if (handler.hasError())
    {
	    delete theAST;
	    return NULL;
    }

	SemanticAnalyzer semanticAnalyzer(program);
    if (semanticAnalyzer.hasFailed())
    {
        delete theAST;
        return NULL;
    }
    
    FunctionData fd(program);
    
    if (fd.globalVariables.size() > 256)
	{
		CompileError::TooManyGlobal.print(NULL);
		delete theAST;
		return NULL;
	}
    
    box_out("Pass 4: Generating object code");
    box_eol();
    
    IntermediateData* id = ScriptParser::generateOCode(fd);
    
    if (id == NULL)
	{
		delete theAST;
		return NULL;
	}
    
    box_out("Pass 5: Assembling");
    box_eol();

    ScriptParser::assemble(id);
    delete id;

    ScriptsData* result = new ScriptsData(program);
    
    box_out("Success!");
    box_eol();
    
	delete theAST;

    return result;
}

int ScriptParser::vid = 0;
int ScriptParser::fid = 0;
int ScriptParser::gid = 1;
int ScriptParser::lid = 0;

string ScriptParser::prepareFilename(string const& filename)
{
    string retval = filename.substr(1, filename.size() - 2); // strip quotes.

	for (int i = 0; retval[i]; ++i)
        {
#ifdef _ALLEGRO_WINDOWS
		if (retval[i] == '/') retval[i] = '\\';
#else
		if (retval[i] == '\\') retval[i] = '/';
#endif
        }
    return retval;
}
        
bool ScriptParser::preprocess(ASTProgram* theAST, int reclimit)
{
    if (reclimit == 0)
            {
		CompileError::ImportRecursion.print(NULL);
            return false;
        }
        
    // Repeat parsing process for each of import files
	vector<ASTImportDecl*>& imports = theAST->imports;
    for (vector<ASTImportDecl*>::iterator it = imports.begin();
		 it != imports.end(); it = imports.erase(it))
        {
        string fn = prepareFilename((*it)->filename);

        if (go(fn.c_str()) != 0 || !resAST)
            {
			CompileError::CantOpenImport.print(*it, fn);
            return false;
            }
            
        ASTProgram* recAST = resAST;
        if (!preprocess(recAST, reclimit - 1))
        {
            delete recAST;
            return false;
        }
        
        // Put the imported code into theAST.
        theAST->merge(*recAST);
    
		delete *it;
    }
    
    return true;
}

IntermediateData* ScriptParser::generateOCode(FunctionData& fdata)
{
	Program& program = fdata.program;
	SymbolTable* symbols = &program.getTable();
	vector<Datum*>& globalVariables = fdata.globalVariables;

    // Z_message("yes");
    bool failure = false;
    
    //we now have labels for the functions and ids for the global variables.
    //we can now generate the code to intialize the globals
    IntermediateData *rval = new IntermediateData(fdata);
    
    // Generate global function code.
    overwritePairs(rval->funcs, GlobalSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, FFCSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, ItemSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, ItemclassSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, LinkSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, ScreenSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, GameSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, NPCSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, LinkWeaponSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, EnemyWeaponSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, AudioSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, DebugSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, NPCDataSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, TextPtrSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, GfxPtrSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, SpriteDataSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, CombosPtrSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, AudioSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, DebugSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, NPCDataSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, InputSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, MapDataSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, DMapDataSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, MessageDataSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, ShopDataSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, UntypedSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, DropsetSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, PondSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, WarpringSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, DoorsetSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, MiscColourSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, RGBSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, PaletteSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, TunesSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, PalCycleSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, GamedataSymbols::getInst().generateCode());
    overwritePairs(rval->funcs, CheatsSymbols::getInst().generateCode());
    
    // Push 0s for init stack space.
    rval->globalsInit.push_back(
		    new OSetImmediate(new VarArgument(EXP1),
		                      new LiteralArgument(0)));
    int globalStackSize = *program.getScope().getRootStackSize();
    for (int i = 0; i < globalStackSize; ++i)
	    rval->globalsInit.push_back(
			    new OPushRegister(new VarArgument(EXP1)));
    
    // Generate variable init code.
    for (vector<Datum*>::iterator it = globalVariables.begin();
		 it != globalVariables.end(); ++it)
    {
		Datum& variable = **it;
		AST& node = *variable.getNode();
        
        OpcodeContext oc;
        oc.symbols = symbols;
        
        BuildOpcodes bo;
        node.execute(bo, &oc);
        if (bo.hasFailed()) failure = true;
        appendElements(rval->globalsInit, oc.initCode);
        appendElements(rval->globalsInit, bo.getResult());
    }
    
    // Pop off everything.
    for (int i = 0; i < globalStackSize; ++i)
	    rval->globalsInit.push_back(
			    new OPopRegister(new VarArgument(EXP2)));
        
    //globals have been initialized, now we repeat for the functions
	vector<Function*> funs = program.getUserFunctions();
	for (vector<Function*>::iterator it = funs.begin();
	     it != funs.end(); ++it)
    {
		Function& function = **it;
		ASTFuncDecl& node = *function.node;
		int nodeId = symbols->getNodeId(&node);

		bool isRun = ZScript::isRun(function);
        string scriptname;
		Script* functionScript = function.getScript();
		if (functionScript)
			scriptname = functionScript->getName();
        
        vector<Opcode *> funccode;
        
		int stackSize = getStackSize(function);
        
        // Start of the function.
        Opcode* first = new OSetImmediate(new VarArgument(EXP1),
                                          new LiteralArgument(0));
        first->setLabel(function.getLabel());
        funccode.push_back(first);

        // Push 0s for the local variables.
        for (int i = stackSize - getParameterCount(function); i > 0; --i)
            funccode.push_back(new OPushRegister(new VarArgument(EXP1)));
        
        // Push on the this, if a script
        if (isRun)
        {
	        ScriptType type = program.getScript(scriptname)->getType();
	        if (type == ScriptType::FFC)
                funccode.push_back(
		                new OSetRegister(new VarArgument(EXP2),
		                                 new VarArgument(REFFFC)));
	        else if (type == ScriptType::ITEM)
                funccode.push_back(
		                new OSetRegister(new VarArgument(EXP2),
		                                 new VarArgument(REFITEMCLASS)));
            
            funccode.push_back(new OPushRegister(new VarArgument(EXP2)));
        }
        
        // Set up the stack frame register
        funccode.push_back(new OSetRegister(new VarArgument(SFRAME),
                                            new VarArgument(SP)));
        OpcodeContext oc;
        oc.symbols = symbols;
        BuildOpcodes bo;
        node.execute(bo, &oc);
        
        if (bo.hasFailed()) failure = true;
            
        appendElements(funccode, bo.getResult());
        
        // Add appendix code.
        Opcode* next = new OSetImmediate(new VarArgument(EXP2),
                                         new LiteralArgument(0));
        next->setLabel(bo.getReturnLabelID());
        funccode.push_back(next);
        
        // Pop off everything.
        for (int i = 0; i < stackSize; ++i)
        {
            funccode.push_back(new OPopRegister(new VarArgument(EXP2)));
        }
        
        //if it's a main script, quit.
		if (isRun)
		{
			// Note: the stack still contains the "this" pointer
			// But since the script is about to terminate, we don't
			// care about popping it off.
			funccode.push_back(new OQuit());
		}
        else
        {
			// Not a script's run method, so no "this" pointer to
			// pop off. The top of the stack is now the function
			// return address (pushed on by the caller).
            //pop off the return address
            funccode.push_back(new OPopRegister(new VarArgument(EXP2)));
            //and return
            funccode.push_back(new OGotoRegister(new VarArgument(EXP2)));
        }
        
        rval->funcs[function.getLabel()] = funccode;
    }
    
    if (failure)
    {
        delete rval;
        return NULL;
    }
    
    //Z_message("yes");
    return rval;
}

void ScriptParser::assemble(IntermediateData *id)
{
	Program& program = id->program;
	
	map<Script*, vector<Opcode*> > scriptCode;
    map<int, vector<Opcode *> > funcs = id->funcs;
    vector<Opcode*> ginit = id->globalsInit;
    
    // Do the global inits
    
    // If there's a global script called "Init", append it to ~Init:
	Script* userInit = program.getScript("Init");
	if (userInit && userInit->getType() == ScriptType::GLOBAL)
    {
	    int label = *getLabel(*userInit);
        ginit.push_back(new OGotoImmediate(new LabelArgument(label)));
    }
    
	Script* init = program.getScript("~Init");
	init->code = assembleOne(ginit, funcs, 0);
    
    for (vector<Script*>::const_iterator it = program.scripts.begin();
         it != program.scripts.end(); ++it)
        {
	    Script& script = **it;
	    if (script.getName() == "~Init") continue;
	    vector<Opcode*> code = funcs[*getLabel(script)];
        int numparams = getRunFunction(script)->paramTypes.size();
        script.code = assembleOne(code, funcs, numparams);
    }
}

vector<Opcode *> ScriptParser::assembleOne(vector<Opcode *> script, map<int, vector<Opcode *> > &otherfuncs, int numparams)
{
    vector<Opcode *> rval;
    //first, push on the params to the run
    int i;
    
    for(i=0; i<numparams && i<9; i++)
    {
        rval.push_back(new OPushRegister(new VarArgument(i)));
    }
    
    for(; i<numparams; i++)
    {
        rval.push_back(new OPushRegister(new VarArgument(EXP1)));
    }
    
    //next, find all labels jumped to by the script code
    map<int,bool> labels;
    
    for(vector<Opcode *>::iterator it = script.begin(); it != script.end(); it++)
    {
        GetLabels temp;
        (*it)->execute(temp, &labels);
    }
    
    //do some fixed-point bullshit
    size_t oldnumlabels = 0;
    
    while(oldnumlabels != labels.size())
    {
        oldnumlabels = labels.size();
        
        for(map<int,bool>::iterator lit = labels.begin(); lit != labels.end(); lit++)
        {
            map<int, vector<Opcode *> >::iterator it = otherfuncs.find(lit->first);
            
            if(it == otherfuncs.end())
                continue;
                
            for(vector<Opcode *>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
            {
                GetLabels temp;
                (*it2)->execute(temp, &labels);
            }
        }
    }
    
    //make the rval
    for(vector<Opcode *>::iterator it = script.begin(); it != script.end(); it++)
    {
        rval.push_back((*it)->makeClone());
    }
    
    for(map<int,bool>::iterator it = labels.begin(); it != labels.end(); it++)
    {
        map<int, vector<Opcode *> >::iterator it2 = otherfuncs.find(it->first);
        
        if(it2 != otherfuncs.end())
        {
            for(vector<Opcode *>::iterator it3 = (*it2).second.begin(); it3 != (*it2).second.end(); it3++)
            {
                rval.push_back((*it3)->makeClone());
            }
        }
    }
    
    //set the label line numbers
    map<int, int> linenos;
    int lineno=1;
    
    for(vector<Opcode *>::iterator it = rval.begin(); it != rval.end(); it++)
    {
        if((*it)->getLabel() != -1)
        {
            linenos[(*it)->getLabel()]=lineno;
        }
        
        lineno++;
    }
    
    //now fill in those labels
    for(vector<Opcode *>::iterator it = rval.begin(); it != rval.end(); it++)
    {
        SetLabels temp;
        (*it)->execute(temp, &linenos);
    }
    
    return rval;
}

pair<long,bool> ScriptParser::parseLong(pair<string, string> parts)
{
    // Not sure if this should really check for negative numbers;
    // in most contexts, that's checked beforehand. parts only
    // includes the minus if this is a constant. - Saf
    bool negative=false;
    pair<long, bool> rval;
    rval.second=true;
    
    if(parts.first.data()[0]=='-')
    {
        negative=true;
        parts.first = parts.first.substr(1);
    }
    
    if(parts.second.size() > 4)
    {
        rval.second = false;
        parts.second = parts.second.substr(0,4);
    }
    
    if(parts.first.size() > 6)
    {
        rval.second = false;
        parts.first = parts.first.substr(0,6);
    }
    
    int firstpart = atoi(parts.first.c_str());
    
    if(firstpart > 214747)
    {
        firstpart = 214747;
        rval.second = false;
    }
    
    long intval = ((long)(firstpart))*10000;
    //add fractional part; tricky!
    int fpart = 0;
    
    while(parts.second.length() < 4)
        parts.second += "0";
        
    for(unsigned int i = 0; i < 4; i++)
    {
        fpart *= 10;
        fpart += parts.second[i] - '0';
    }
    
    /*for(unsigned int i=0; i<4; i++)
    {
    	fpart*=10;
    	char tmp[2];
    	tmp[0] = parts.second.at(i);
    	tmp[1] = 0;
    	fpart += atoi(tmp);
    }*/
    rval.first = intval + fpart;
    if(negative)
        rval.first = -rval.first;
    return rval;
}

ScriptsData::ScriptsData(Program& program)
{
	for (vector<Script*>::const_iterator it = program.scripts.begin();
	     it != program.scripts.end(); ++it)
	{
		Script& script = **it;
		string const& name = script.getName();
		theScripts[name] = script.code;
		script.code = vector<Opcode*>();
		scriptTypes[name] = script.getType();
	}
}

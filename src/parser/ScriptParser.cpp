#include "../precompiled.h" //always first

#include "../zsyssimple.h"
#include "CompileError.h"
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

using std::string;
using std::vector;
using std::set;
using std::map;

using namespace ZScript;
using namespace ZAsm;
//#define PARSER_DEBUG

ASTProgram* resAST;
TypeStore* resTS;

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
    TypeStore typeStore;
    resTS = &typeStore;

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
    Program program(*theAST, typeStore,handler);
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
	TypeStore& typeStore = program.getTypeStore();
	vector<Datum*>& globalVariables = fdata.globalVariables;

    // Z_message("yes");
    bool failure = false;

    //we now have labels for the functions and ids for the global variables.
    //we can now generate the code to intialize the globals
    IntermediateData *rval = new IntermediateData(fdata);

    // Push 0s for init stack space.
    rval->globalsInit.push_back(opSETV(varExp1(), 0));
    int globalStackSize = *program.getScope().getRootStackSize();
    for (int i = 0; i < globalStackSize; ++i)
	    rval->globalsInit.push_back(opPUSHR(varExp1()));
    
    // Generate variable init code.
    for (vector<Datum*>::iterator it = globalVariables.begin();
		 it != globalVariables.end(); ++it)
    {
		Datum& variable = **it;
		AST& node = *variable.getNode();

        OpcodeContext oc;
        oc.typeStore = &typeStore;

        BuildOpcodes bo(typeStore);
        node.execute(bo, &oc);
        if (bo.hasFailed()) failure = true;

        int start = rval->globalsInit.size();
        appendElements(rval->globalsInit, oc.initCode);
        appendElements(rval->globalsInit, bo.getResult());
        rval->globalsInit[start].withComment(node.asString());
    }

    // Pop off everything.
    for (int i = 0; i < globalStackSize; ++i)
	    rval->globalsInit.push_back(opPOP(varExp2()));
        
    //globals have been initialized, now we repeat for the functions
	vector<Function*> funs = program.getUserFunctions();
	for (vector<Function*>::iterator it = funs.begin();
	     it != funs.end(); ++it)
    {
		Function& function = **it;
		ASTFuncDecl& node = *function.node;

		bool isRun = ZScript::isRun(function);
		string scriptname;
		Script* functionScript = function.getScript();
		if (functionScript)
			scriptname = functionScript->getName();

        vector<Opcode> funccode;

		int stackSize = getStackSize(function);

        // Start of the function.
        funccode.push_back(
		        opSETV(varExp1(), 0).withLabel(function.getLabel()));

        // Push 0s for the local variables.
        for (int i = stackSize - getParameterCount(function); i > 0; --i)
            funccode.push_back(opPUSHR(varExp1()));
        
        // Push on the this, if a script
        if (isRun)
        {
	        ScriptType type = program.getScript(scriptname)->getType();
	        if (type == ScriptType::getFfc())
		        funccode.push_back(opSETR(varExp2(), varREFFFC()));
	        else if (type == ScriptType::getItem())
		        funccode.push_back(opSETR(varExp2(), varREFITEMCLASS()));
            
            funccode.push_back(opPUSHR(varExp2()));
        }
        
        // Set up the stack frame register
        funccode.push_back(opSETR(varSFrame(), varSP()));
        OpcodeContext oc;
        oc.typeStore = &typeStore;
        BuildOpcodes bo(typeStore);
        node.execute(bo, &oc);

        if (bo.hasFailed()) failure = true;

        appendElements(funccode, bo.getResult());
        
        // Add appendix code.
        funccode.push_back(
		        opSETV(varExp2(), 0).withLabel(bo.getReturnLabelID()));
        
        // Pop off everything.
        for (int i = 0; i < stackSize; ++i)
            funccode.push_back(opPOP(varExp2()));
        
        //if it's a main script, quit.
		if (isRun)
		{
			// Note: the stack still contains the "this" pointer
			// But since the script is about to terminate, we don't
			// care about popping it off.
			funccode.push_back(opQUIT());
		}
        else
        {
			// Not a script's run method, so no "this" pointer to
			// pop off. The top of the stack is now the function
			// return address (pushed on by the caller).
            //pop off the return address
            funccode.push_back(opPOP(varExp2()));
            //and return
            funccode.push_back(opGOTOR(varExp2()));
        }
        
		function.setCode(funccode);
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

    vector<Opcode> ginit = id->globalsInit;

    // Do the global inits
    
    // If there's a global script called "Init", append it to ~Init:
	Script* userInit = program.getScript("Init");
	if (userInit && userInit->getType() == ScriptType::getGlobal())
    {
	    int label = *getLabel(*userInit);
        ginit.push_back(opGOTO(label));
    }

	Script* init = program.getScript("~Init");
	init->code = assembleOne(program, ginit, 0);
    
    for (vector<Script*>::const_iterator it = program.scripts.begin();
         it != program.scripts.end(); ++it)
    {
	    Script& script = **it;
	    if (script.getName() == "~Init") continue;
	    Function& run = *getRunFunction(script);
        int numparams = getRunFunction(script)->paramTypes.size();
        script.code = assembleOne(program, run.getCode(), numparams);
    }
}

vector<Opcode> ScriptParser::assembleOne(
		Program& program, vector<Opcode> const& runCode, int numparams)
{
    vector<Opcode> rval;

    // Push on the params to the run.
    int i;
    for (i = 0; i < numparams && i < 9; ++i)
	    rval.push_back(opPUSHR(getVariable(VariableId(i))));
    for (; i < numparams; ++i)
        rval.push_back(opPUSHR(varExp1()));

    // Generate a map of labels to functions.
    vector<Function*> allFunctions = getFunctions(program);
    map<int, Function*> functionsByLabel;
    for (vector<Function*>::iterator it = allFunctions.begin();
         it != allFunctions.end(); ++it)
    {
	    Function& function = **it;
	    functionsByLabel[function.getLabel()] = &function;
    }

    // Grab all labels directly jumped to.
    set<int> usedLabels;
    for (vector<Opcode>::const_iterator it = runCode.begin();
         it != runCode.end(); ++it)
    {
	    vector<Label*> labels = getArgLabels(*it);
	    for (vector<Label*>::const_iterator it = labels.begin();
	         it != labels.end(); ++it)
		    usedLabels.insert(*(*it)->getId());
    }
    set<int> unprocessedLabels(usedLabels);

    // Grab labels used by each function until we run out of functions.
    while (!unprocessedLabels.empty())
    {
	    int label = *unprocessedLabels.begin();
	    Function* function =
		    find<Function*>(functionsByLabel, label).value_or(NULL);
	    if (function)
	    {
		    vector<Opcode> const& functionCode = function->getCode();
		    for (vector<Opcode>::const_iterator it = functionCode.begin();
		         it != functionCode.end(); ++it)
		    {
			    vector<Label*> labels = getArgLabels(*it);
			    for (vector<Label*>::const_iterator it = labels.begin();
			         it != labels.end(); ++it)
			    {
				    int id = *(*it)->getId();
				    if (find<int>(usedLabels, id)) continue;
				    usedLabels.insert(id);
				    unprocessedLabels.insert(id);
			    }
		    }
	    }

	    unprocessedLabels.erase(label);
    }
    
    // Make the rval
    appendElements(rval, runCode);
    
    for (set<int>::iterator it = usedLabels.begin();
         it != usedLabels.end(); ++it)
    {
	    int label = *it;
	    Function* function =
		    find<Function*>(functionsByLabel, label).value_or(NULL);
	    if (!function) continue;

	    appendElements(rval, function->getCode());
    }
    
    // Map label ids to line numbers.
    map<int, int> linenos;
    int lineno = 1;
    for (vector<Opcode>::iterator it = rval.begin();
         it != rval.end(); ++it)
    {
	    if (optional<int> id = it->getLabel().getId())
            linenos[*id] = lineno;
        ++lineno;
    }
    
    // Now fill in those line numbers
    for (vector<Opcode>::iterator it = rval.begin();
         it != rval.end(); ++it)
    {
	    Opcode& opcode = *it;
	    vector<Label*> labels = getArgLabels(opcode);
	    for (vector<Label*>::iterator it = labels.begin();
	         it != labels.end(); ++it)
	    {
		    Label& label = **it;
		    int id = *label.getId();
		    optional<int> lineno = find<int>(linenos, id);
		    if (!lineno)
		    {
			    char temp[200];
			    sprintf(temp, "Internal error: couldn't find function label %d", id);
			    box_out(temp);
			    box_eol();
		    }
		    label.setLine(lineno.value_or(0));
	    }
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
		script.code = vector<Opcode>();
		scriptTypes[name] = script.getType();
	}
}

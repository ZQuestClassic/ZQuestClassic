//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "../zsyssimple.h"
#include "ByteCode.h"
#include "CompileError.h"
#include "CompileOption.h"
#include "GlobalSymbols.h"
#include "y.tab.hpp"
#include <iostream>
#include <assert.h>
#include <cstdlib>
#include <string>
#include <memory>
#include <boost/move/unique_ptr.hpp>

#include "ASTVisitors.h"
#include "DataStructs.h"
#include "Scope.h"
#include "SemanticAnalyzer.h"
#include "BuildVisitors.h"
#include "RegistrationVisitor.h"
#include "ZScript.h"
using boost::movelib::unique_ptr;
using namespace ZScript;

extern char ZQincludePaths[MAX_INCLUDE_PATHS][512];
//#define PARSER_DEBUG

ScriptsData* compile(string const& filename);

#if PARSER_DEBUG < 0
int main(int argc, char *argv[])
{
	if (argc < 2) return -1;
	compile(string(argv[1]));
}
#endif

void ScriptParser::initialize()
{
	vid = 0;
	fid = 0;
	gid = 1;
	lid = 0;
	CompileError::initialize();
	CompileOption::initialize();
}

ScriptsData* ZScript::compile(string const& filename)
{
    ScriptParser::initialize();

	box_out("Pass 1: Parsing");
	box_eol();

	unique_ptr<ASTFile> root(parseFile(filename));
	if (!root.get())
	{
		box_out_err(CompileError::CantOpenSource(NULL));
		return NULL;
	}
    
	box_out("Pass 2: Preprocessing");
	box_eol();
    
	if (!ScriptParser::preprocess(root.get(), ScriptParser::recursionLimit))
		return NULL;
    
	SimpleCompileErrorHandler handler;
	Program program(*root, &handler);
	if (handler.hasError())
		return NULL;
	
	box_out("Pass 3: Registration");
	box_eol();

	RegistrationVisitor regVisitor(program);
	if(regVisitor.hasFailed()) return NULL;
	
	box_out("Pass 4: Analyzing Code");
	box_eol();
	
	SemanticAnalyzer semanticAnalyzer(program);
	if (semanticAnalyzer.hasFailed() || regVisitor.hasFailed())
		return NULL;
    
	FunctionData fd(program);
	if (fd.globalVariables.size() > MAX_SCRIPT_REGISTERS)
	{
		box_out_err(CompileError::TooManyGlobal(NULL));
		return NULL;
	}
    
	box_out("Pass 5: Generating object code");
	box_eol();
    
	unique_ptr<IntermediateData> id(ScriptParser::generateOCode(fd));
	if (!id.get())
		return NULL;
    
	box_out("Pass 6: Assembling");
	box_eol();

	ScriptParser::assemble(id.get());

	ScriptsData* result = new ScriptsData(program);
    
	box_out("Success!");
	box_eol();

	return result;
}

int ScriptParser::vid = 0;
int ScriptParser::fid = 0;
int ScriptParser::gid = 1;
int ScriptParser::lid = 0;

string ScriptParser::prepareFilename(string const& filename)
{
	string retval = filename;

	regulate_path(retval);
	return retval;
}

bool ScriptParser::preprocess_one(ASTImportDecl& importDecl, int reclimit)
{
	// Parse the imported file.
	string* fname = NULL;
	string includePath;
	string importname = prepareFilename(importDecl.getFilename());
	if(!importDecl.isInclude()) //Check root dir first for imports
	{
		FILE* f = fopen(importname.c_str(), "r");
		if(f)
		{
			fclose(f);
			fname = &importname;
		}
	}
	if(!fname)
	{
		// Scan include paths
		int importfound = importname.find_first_not_of("/\\");
		if(importfound != string::npos) //If the import is not just `/`'s and `\`'s...
		{
			if(importfound != 0)
				importname = importname.substr(importfound); //Remove leading `/` and `\`
			//Convert the include string to a proper import path
			for ( int q = 0; q < MAX_INCLUDE_PATHS && !fname; ++q ) //Loop through all include paths, or until valid file is found
			{
				if( ZQincludePaths[q][0] != '\0' )
				{
					includePath = &*ZQincludePaths[q];
					//Add a `/` to the end of the include path, if it is missing
					int lastnot = includePath.find_last_not_of("/\\");
					int last = includePath.find_last_of("/\\");
					if(lastnot != string::npos)
					{
						if(last == string::npos || last < lastnot)
							includePath += "/";
					}
					includePath = prepareFilename(includePath + importname);
					FILE* f = fopen(includePath.c_str(), "r");
					if(!f) continue;
					fclose(f);
					fname = &includePath;
				}
			}
		}
	}
	//
	string filename = fname ? *fname : prepareFilename(importname); //Check root dir last, if nothing has been found yet.
	FILE* f = fopen(filename.c_str(), "r");
	if(f)
	{
		fclose(f);
	}
	else
	{
		box_out_err(CompileError::CantOpenImport(&importDecl, filename));
		return false;
	}
	unique_ptr<ASTFile> imported(parseFile(filename));
	if (!imported.get())
	{
		box_out_err(CompileError::CantParseImport(&importDecl, filename));
		return false;
	}

	// Save the AST in the import declaration.
	importDecl.giveTree(imported.release());
	
	// Recurse on imports.
	if (!preprocess(importDecl.getTree(), reclimit - 1))
		return false;
	
	return true;
}

bool ScriptParser::preprocess(ASTFile* root, int reclimit)
{
	assert(root);
	
	if (reclimit == 0)
	{
		box_out_err(CompileError::ImportRecursion(NULL, recursionLimit));
		return false;
	}
        
	// Repeat parsing process for each of import files
	for (vector<ASTImportDecl*>::iterator it = root->imports.begin();
	     it != root->imports.end(); ++it)
	{
		if(!preprocess_one(**it, reclimit)) return false;
	}
    
	return true;
}

IntermediateData* ScriptParser::generateOCode(FunctionData& fdata)
{
	Program& program = fdata.program;
	Scope* scope = &program.getScope();
	TypeStore* typeStore = &program.getTypeStore();
	vector<Datum*>& globalVariables = fdata.globalVariables;

	// Z_message("yes");
	bool failure = false;
    
	//we now have labels for the functions and ids for the global variables.
	//we can now generate the code to intialize the globals
	IntermediateData *rval = new IntermediateData(fdata);
    
	// Push 0s for init stack space.
	/* Why? The stack should already be init'd to all 0, anyway?
	rval->globalsInit.push_back(
			new OSetImmediate(new VarArgument(EXP1),
			                  new LiteralArgument(0)));
	int globalStackSize = *program.getScope().getRootStackSize();
	for (int i = 0; i < globalStackSize; ++i)
		rval->globalsInit.push_back(
				new OPushRegister(new VarArgument(EXP1)));*/
    
	// Generate variable init code.
	for (vector<Datum*>::iterator it = globalVariables.begin();
	     it != globalVariables.end(); ++it)
	{
		Datum& variable = **it;
		AST& node = *variable.getNode();
        
		OpcodeContext oc(typeStore);
        
		BuildOpcodes bo(scope);
		node.execute(bo, &oc);
		if (bo.hasError()) failure = true;
		appendElements(rval->globalsInit, oc.initCode);
		appendElements(rval->globalsInit, bo.getResult());
	}
    
	// Pop off everything.
	/* See above; why push this in the first place?
	for (int i = 0; i < globalStackSize; ++i)
		rval->globalsInit.push_back(
				new OPopRegister(new VarArgument(EXP2)));*/
        
	//globals have been initialized, now we repeat for the functions
	vector<Function*> funs = program.getUserFunctions();
	for (vector<Function*>::iterator it = funs.begin();
	     it != funs.end(); ++it)
	{
		Function& function = **it;
		if(function.getFlag(FUNCFLAG_INLINE)) continue; //Skip inline func decls, they are handled at call location -V
		ASTFuncDecl& node = *function.node;

		bool isRun = ZScript::isRun(function);
		string scriptname;
		Script* functionScript = function.getScript();
		if (functionScript)
		{
			scriptname = functionScript->getName();
		}
		scope = function.internalScope;
        
		vector<Opcode *> funccode;
        
		int stackSize = getStackSize(function);
        
		// Start of the function.
		Opcode* first = new OSetImmediate(new VarArgument(EXP1),
		                                  new LiteralArgument(0));
		first->setLabel(function.getLabel());
		funccode.push_back(first);

		// Push on the this, if a script
		if (isRun)
		{
			ScriptType type = program.getScript(scriptname)->getType();
			
			if (type == ScriptType::ffc )
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							 new VarArgument(REFFFC)));
				
				
			}
			else if (type == ScriptType::item )
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							 new VarArgument(REFITEMCLASS)));
				
			}
			else if (type == ScriptType::npc )
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							 new VarArgument(REFNPC)));
				
			}
			else if (type == ScriptType::lweapon )
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							 new VarArgument(REFLWPN)));
			}
			else if (type == ScriptType::eweapon )
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							 new VarArgument(REFEWPN)));
				
			}
			else if (type == ScriptType::dmapdata )
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							 new VarArgument(REFDMAPDATA)));
			
			}
			else if (type == ScriptType::itemsprite)
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							new VarArgument(REFITEM)));
			}
			else if (type == ScriptType::subscreendata)
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							new VarArgument(REFSUBSCREEN)));
			}
			else if (type == ScriptType::combodata)
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							new VarArgument(REFCOMBODATA)));
			}
			/* Do we want these here--ever? -Z
			else if (type == ScriptType::link )
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							 new VarArgument(link?)));
				
			}
			else if (type == ScriptType::screen )
			{
				funccode.push_back(
					new OSetRegister(new VarArgument(EXP2),
							 new VarArgument(tempscr?)));
				
			}
				*/
				
		
			/*
			if (type == ScriptType::ffc)
				funccode.push_back(
						new OSetRegister(new VarArgument(EXP2),
						                 new VarArgument(REFFFC)));
			else if (type == ScriptType::item)
				funccode.push_back(
						new OSetRegister(new VarArgument(EXP2),
						                 new VarArgument(REFITEMCLASS)));
			*/
			funccode.push_back(new OPushRegister(new VarArgument(EXP2)));
		}
        
		// Push 0s for the local variables.
		for (int i = stackSize - getParameterCount(function); i > 0; --i)
			funccode.push_back(new OPushRegister(new VarArgument(EXP1)));
        
		// Set up the stack frame register
		funccode.push_back(new OSetRegister(new VarArgument(SFRAME),
		                                    new VarArgument(SP)));
		OpcodeContext oc(typeStore);
		BuildOpcodes bo(scope);
		node.execute(bo, &oc);
        
		if (bo.hasError()) failure = true;
            
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
			//and return
			funccode.push_back(new OReturn());
		}
        
		function.giveCode(funccode);
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
	vector<Opcode*> ginit = id->globalsInit;
    
	// Do the global inits
    
	// If there's a global script called "Init", append it to ~Init:
	Script* userInit = program.getScript("Init");
	if (userInit && userInit->getType() == ScriptType::global)
	{
		int label = *getLabel(*userInit);
		ginit.push_back(new OGotoImmediate(new LabelArgument(label)));
	}
    
	Script* init = program.getScript("~Init");
	init->code = assembleOne(program, ginit, 0);
    
	for (vector<Script*>::const_iterator it = program.scripts.begin();
	     it != program.scripts.end(); ++it)
	{
		Script& script = **it;
		if (script.getName() == "~Init") continue;
		if(script.getType() == ScriptType::untyped) continue;
		Function& run = *script.getRun();
		int numparams = script.getRun()->paramTypes.size();
		script.code = assembleOne(program, run.getCode(), numparams);
	}
}

vector<Opcode*> ScriptParser::assembleOne(
		Program& program, vector<Opcode*> runCode, int numparams)
{
	vector<Opcode *> rval;
    
	// Push on the params to the run.
	int i;
	for (i = 0; i < numparams && i < 9; ++i)
		rval.push_back(new OPushRegister(new VarArgument(i)));
	for (; i < numparams; ++i)
		rval.push_back(new OPushRegister(new VarArgument(EXP1)));
    
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
	std::set<int> usedLabels;
	for (vector<Opcode*>::iterator it = runCode.begin();
	     it != runCode.end(); ++it)
	{
		GetLabels temp(usedLabels);
		(*it)->execute(temp, NULL);
	}
	std::set<int> unprocessedLabels(usedLabels);
    
	// Grab labels used by each function until we run out of functions.
	while (!unprocessedLabels.empty())
	{
		int label = *unprocessedLabels.begin();
		Function* function =
			find<Function*>(functionsByLabel, label).value_or(NULL);
		if (function)
		{
			vector<Opcode*> const& functionCode = function->getCode();
			for (vector<Opcode*>::const_iterator it = functionCode.begin();
			     it != functionCode.end(); ++it)
			{
				GetLabels temp(usedLabels);
				(*it)->execute(temp, NULL);
				insertElements(unprocessedLabels, temp.newLabels);
			}
		}

		unprocessedLabels.erase(label);
	}
    
	// Make the rval
	for (vector<Opcode*>::iterator it = runCode.begin();
	     it != runCode.end(); ++it)
		rval.push_back((*it)->makeClone());
    
	for (std::set<int>::iterator it = usedLabels.begin();
	     it != usedLabels.end(); ++it)
	{
		int label = *it;
		Function* function =
			find<Function*>(functionsByLabel, label).value_or(NULL);
		if (!function) continue;
        
		vector<Opcode*> functionCode = function->getCode();
		for (vector<Opcode*>::iterator it = functionCode.begin();
		     it != functionCode.end(); ++it)
			rval.push_back((*it)->makeClone());
	}
    
	// Set the label line numbers.
	map<int, int> linenos;
	int lineno = 1;
    
	for (vector<Opcode*>::iterator it = rval.begin();
	     it != rval.end(); ++it)
	{
		if ((*it)->getLabel() != -1)
			linenos[(*it)->getLabel()] = lineno;
		lineno++;
	}
    
	// Now fill in those labels
	for (vector<Opcode*>::iterator it = rval.begin();
	     it != rval.end(); ++it)
	{
		SetLabels temp;
		(*it)->execute(temp, &linenos);
	}
    
	return rval;
}

std::pair<long,bool> ScriptParser::parseLong(std::pair<string, string> parts, Scope* scope)
{
	// Not sure if this should really check for negative numbers;
	// in most contexts, that's checked beforehand. parts only
	// includes the minus if this is a constant. - Saf
	bool negative=false;
	std::pair<long, bool> rval;
	rval.second=true;
	bool intOneLarger = *lookupOption(*scope, CompileOption::OPT_TRUE_INT_SIZE) != 0;
    
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
	if(intOneLarger) //MAX_INT should be 214748, but if that is the value, there should be no float component. -V
	{
		if(firstpart > 214748)
		{
			firstpart = 214748;
			rval.second = false;
		}
	}
	else if(firstpart > 214747)
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
	  
	if(intOneLarger && firstpart == 214748 && (negative ? fpart > 3648 : fpart > 3647))
	{
		fpart = negative ? 3648 : 3647;
		rval.second = false;
	}
	
	
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
		zasm_meta& meta = theScripts[name].first;
		theScripts[name].second = script.code;
		meta.autogen();
		meta.script_type = script.getType().getTrueId();
		string const& author = script.getAuthor().substr(0,32);
		strcpy(meta.script_name, name.substr(0,32).c_str());
		strcpy(meta.author, author.c_str());
		al_trace(meta.script_name);
		al_trace(meta.author);
		al_trace(name.c_str());
		al_trace(author.c_str());
		if(Function* run = script.getRun())
		{
			int ind = 0;
			for(vector<string const*>::const_iterator it = run->paramNames.begin();
				it != run->paramNames.end(); ++it)
			{
				char* dest = meta.run_idens[ind++];
				strcpy(dest, (**it).c_str());	
			}
			ind = 0;
			for(vector<DataType const*>::const_iterator it = run->paramTypes.begin();
				it != run->paramTypes.end(); ++it)
			{
				optional<DataTypeId> id = program.getTypeStore().getTypeId(**it);
				meta.run_types[ind++] = id ? *id : ZVARTYPEID_VOID;
			}
		}
		
		script.code = vector<Opcode*>();
		scriptTypes[name] = script.getType();
	}
}

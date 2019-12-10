//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "ParseError.h"
#include "y.tab.hpp"
#include "TypeChecker.h"
#include "GlobalSymbols.h"
#include "ByteCode.h"
#include "../zsyssimple.h"
#include <iostream>
#include <assert.h>
#include <string>
#include <cstdlib>

#include "DataStructs.h"
#include "SymbolVisitors.h"
#include "UtilVisitors.h"
#include "AST.h"
#include "BuildVisitors.h"
using namespace std;
//#define PARSER_DEBUG

AST *resAST;

ScriptsData * compile(const char *filename);

#ifdef PARSER_DEBUG
int main(int argc, char *argv[])
{
    if(argc < 2)
        return -1;
        
    compile(argv[1]);
}
#endif

ScriptsData * compile(const char *filename)
{
    ScriptParser::resetState();
    headerguards.clear();

#ifndef SCRIPTPARSER_COMPILE
    box_out("Pass 1: Parsing");
    box_eol();
#endif
    
    int resultGo = go(filename);	
    if( resultGo == -99 )  
	{
		printErrorMsg(NULL,WARNHEADER);
		
		return NULL;
		//header guard issues need a unique return and to skip
		//this process
	}
    else if(resultGo != 0 || !resAST)
    {
        printErrorMsg(NULL, CANTOPENSOURCE);
        return NULL;
    }
    
    AST *theAST = resAST;
    
#ifndef SCRIPTPARSER_COMPILE
    box_out("Pass 2: Preprocessing");
    box_eol();
    headerguardsPPC.clear();
#endif
    map<string, long> *consts = new map<string,long>();
    
    if(!ScriptParser::preprocess(theAST, RECURSIONLIMIT,consts))
    {
        delete theAST;
        delete consts;
        return NULL;
    }
    
#ifndef SCRIPTPARSER_COMPILE
    box_out("Pass 3: Building symbol tables");
    box_eol();
#endif
    SymbolData *d = ScriptParser::buildSymbolTable(theAST,consts);
    
    if(d==NULL)
    {
        //delete theAST;
        delete consts;
        return NULL;
    }
    
    //d->symbols->printDiagnostics();
#ifndef SCRIPTPARSER_COMPILE
    box_out("Pass 4: Type-checking/Completing function symbol tables/Constant folding");
    box_eol();
#endif
    FunctionData *fd = ScriptParser::typeCheck(d);
    
    if(fd == NULL)
    {
        //delete theAST;
        delete consts;
        /*if(d->symbols) delete d->symbols;
        for(vector<ASTFuncDecl *>::iterator it2 = d->globalFuncs.begin(); it2 != d->globalFuncs.end(); it2++)
        {
        	delete *it2;
        }
        for(vector<ASTScript *>::iterator it2 = d->scripts.begin(); it2 != d->scripts.end(); it2++)
        {
        	delete *it2;
        }
        for(vector<ASTVarDecl *>::iterator it2 = d->globalVars.begin(); it2 != d->globalVars.end(); it2++)
        {
        	delete *it2;
        }
        for(vector<ASTArrayDecl *>::iterator it2 = d->globalArrays.begin(); it2 != d->globalArrays.end(); it2++)\
        {
        	delete *it2;
        }
        delete d;*/
        return NULL;
    }
    
#ifndef SCRIPTPARSER_COMPILE
    box_out("Pass 5: Generating object code");
    box_eol();
#endif
    
    IntermediateData *id = ScriptParser::generateOCode(fd);
    
    if(id == NULL)
    {
        //delete theAST;
        delete consts;
        /*if(d->symbols) delete d->symbols;
        for(vector<ASTFuncDecl *>::iterator it2 = d->globalFuncs.begin(); it2 != d->globalFuncs.end(); it2++)
        {
        	delete *it2;
        }
        for(vector<ASTScript *>::iterator it2 = d->scripts.begin(); it2 != d->scripts.end(); it2++)
        {
        	delete *it2;
        }
        for(vector<ASTVarDecl *>::iterator it2 = d->globalVars.begin(); it2 != d->globalVars.end(); it2++)
        {
        	delete *it2;
        }
        for(vector<ASTArrayDecl *>::iterator it2 = d->globalArrays.begin(); it2 != d->globalArrays.end(); it2++)\
        {
        	delete *it2;
        }
        delete d;*/
        /*if(fd->symbols) delete fd->symbols;
        for(vector<ASTFuncDecl *>::iterator it2 = fd->functions.begin(); it2 != fd->functions.end(); it2++)
        {
        	delete *it2;
        }
        for(vector<ASTVarDecl *>::iterator it2 = fd->newGlobalVars.begin(); it2 != fd->newGlobalVars.end(); it2++)
        {
        	delete *it2;
        }
        for(vector<ASTVarDecl *>::iterator it2 = fd->globalVars.begin(); it2 != fd->globalVars.end(); it2++)
        {
        	delete *it2;
        }
        for(vector<ASTArrayDecl *>::iterator it2 = fd->newGlobalArrays.begin(); it2 != fd->newGlobalArrays.end(); it2++)
        {
        	delete *it2;
        }
        for(vector<ASTArrayDecl *>::iterator it2 = fd->globalArrays.begin(); it2 != fd->globalArrays.end(); it2++)\
        {
        	delete *it2;
        }
        delete fd;*/
        return NULL;
    }
    
#ifndef SCRIPTPARSER_COMPILE
    box_out("Pass 6: Assembling");
    box_eol();
#endif
    ScriptsData *final = ScriptParser::assemble(id);
    box_out("Success!");
    box_eol();
    
    //delete theAST;
    delete consts;
    /*if(d->symbols) delete d->symbols;
    for(vector<ASTFuncDecl *>::iterator it2 = d->globalFuncs.begin(); it2 != d->globalFuncs.end(); it2++)
    {
    	delete *it2;
    }
    for(vector<ASTScript *>::iterator it2 = d->scripts.begin(); it2 != d->scripts.end(); it2++)
    {
    	delete *it2;
    }
    for(vector<ASTVarDecl *>::iterator it2 = d->globalVars.begin(); it2 != d->globalVars.end(); it2++)
    {
    	delete *it2;
    }
    for(vector<ASTArrayDecl *>::iterator it2 = d->globalArrays.begin(); it2 != d->globalArrays.end(); it2++)\
    {
    	delete *it2;
    }
    delete d;
    if(fd->symbols) delete fd->symbols;
    for(vector<ASTFuncDecl *>::iterator it2 = fd->functions.begin(); it2 != fd->functions.end(); it2++)
    {
    	delete *it2;
    }
    for(vector<ASTVarDecl *>::iterator it2 = fd->newGlobalVars.begin(); it2 != fd->newGlobalVars.end(); it2++)
    {
    	delete *it2;
    }
    for(vector<ASTVarDecl *>::iterator it2 = fd->globalVars.begin(); it2 != fd->globalVars.end(); it2++)
    {
    	delete *it2;
    }
    for(vector<ASTArrayDecl *>::iterator it2 = fd->newGlobalArrays.begin(); it2 != fd->newGlobalArrays.end(); it2++)
    {
    	delete *it2;
    }
    for(vector<ASTArrayDecl *>::iterator it2 = fd->globalArrays.begin(); it2 != fd->globalArrays.end(); it2++)\
    {
    	delete *it2;
    }
    delete fd;*/
    /*for(map<int, vector<Opcode *> >::iterator it = id->funcs.begin(); it != id->funcs.end(); it++)
    {
    	for(vector<Opcode *>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
    	{
    		delete *it2;
    	}
    }
    for(vector<Opcode *>::iterator it = id->globalsInit.begin(); it != id->globalsInit.end(); it++)
    {
    	delete *it;
    }
    for(vector<Opcode *>::iterator it = id->globalasInit.begin(); it != id->globalasInit.end(); it++)
    {
    	delete *it;
    }
    delete id;*/
    
    return final;
}

int ScriptParser::vid = 0;
int ScriptParser::fid = 0;
int ScriptParser::gid = 1;
int ScriptParser::lid = 0;

// The following is NOT AT ALL compliant with the C++ standard
// but apparently required by the MingW gcc...
#ifndef _MSC_VER
const int ScriptParser::TYPE_FLOAT;
const int ScriptParser::TYPE_BOOL;
const int ScriptParser::TYPE_VOID;
const int ScriptParser::TYPE_LINK;
const int ScriptParser::TYPE_FFC;
const int ScriptParser::TYPE_ITEM;
const int ScriptParser::TYPE_ITEMCLASS;
const int ScriptParser::TYPE_SCREEN;
const int ScriptParser::TYPE_GLOBAL;
const int ScriptParser::TYPE_GAME;
const int ScriptParser::TYPE_NPC;
const int ScriptParser::TYPE_LWPN;
const int ScriptParser::TYPE_EWPN;
#endif

string ScriptParser::trimQuotes(string quoteds)
{
    string rval = quoteds.substr(1,quoteds.size()-2);
    return rval;
}

bool ScriptParser::preprocess(AST *theAST, int reclimit, map<string,long> *constants)
{
    headerguards.clear();
    if(reclimit == 0)
    {
        printErrorMsg(NULL, IMPORTRECURSION);
        return false;
    }
    
    //Repeat parsing process for each of import files
    GetImports gi;
    theAST->execute(gi, NULL);
    vector<ASTImportDecl *> imports = gi.getResult();
    
    for(vector<ASTImportDecl *>::iterator it = imports.begin(); it != imports.end(); it++)
    {
	    //the filename
	    //Plan:
	    /*
		Add an iterator for number of imports
		If iterator < 1:
			Store it in a vector of strings
			Incr iterator
		else
		scan vector for a match
		if it matches, issue a warning in box_out
			AND skip it
			else incr iterator and try to import the file

		if importing, scan include paths
		this needs to be done in go()?
	    
		No
		before callong go(fn)
		std::str tmpfn = "@@" +fn;
		//if registerednames has no substring matching tempfn
		    registerednames += tempfn;
		//else, warn and skip
		
		We need a string that is global to the parser
		As the parser INVOKES, clear it, and start tacking on delim+filename
		string.find("substr")
			if that returns string::npos, then we add it
			if it returns anything else, then we warn and skip
			    
				if(string::npos == foundImports.find(name))
				{
				    foundImports += "@@" + name;
				    //GO
				}
				else
				{
				    //WARN
				}
	    
	    */
        string fn = trimQuotes((*it)->getFilename());
        //this handles correcting the path for the host OS
        for(int i=0; fn[i]; i++)
        {
#ifdef _ALLEGRO_WINDOWS
        
            if(fn[i]=='/')
                fn[i]='\\';
                
#else
                
            if(fn[i]=='\\')
                fn[i]='/';
                
#endif
        }
        
	
        int PPCresult = goPPC(fn.c_str());
	if (PPCresult == -99 )
	{
		printErrorMsg(*it,WARNHEADER, fn);
		
	}
        else if( PPCresult != 0 || !resAST ) 
		
        {
	    
            printErrorMsg(*it,CANTOPENIMPORT, fn);
            
            for(vector<ASTImportDecl *>::iterator it2 = imports.begin(); it2 != imports.end(); it2++)
            {
                delete *it2;
            }
            
            return false;
        }
        
        AST *recAST = resAST;
        
        if(!preprocess(recAST, reclimit-1,constants))
        {
            for(vector<ASTImportDecl *>::iterator it2 = imports.begin(); it2 != imports.end(); it2++)
            {
                delete *it2;
            }
            
            delete recAST;
            return false;
        }
        
        //Put the imported code into theAST
        MergeASTs temp;
        theAST->execute(temp, recAST);
    }
    
    for(vector<ASTImportDecl *>::iterator it2 = imports.begin(); it2 != imports.end(); it2++)
    {
        delete *it2;
    }
    
    //check that there are no more stupidly placed imports in the file
    CheckForExtraneousImports c;
    theAST->execute(c, NULL);
    
    if(!c.isOK())
        return false;
        
    //get the constants
    GetConsts gc;
    theAST->execute(gc,NULL);
    vector<ASTConstDecl *> consts = gc.getResult();
    bool failure = false;
    
    for(vector<ASTConstDecl *>::iterator it = consts.begin(); it != consts.end(); it++)
    {
        map<string, long>::iterator find = constants->find((*it)->getName());
        
        if(find != constants->end())
        {
            printErrorMsg(*it, CONSTREDEF, (*it)->getName());
            failure=true;
        }
        else
        {
            pair<string,string> parts = (*it)->getValue()->parseValue();
            pair<long,bool> val = ScriptParser::parseLong(parts);
            
            if(!val.second)
            {
                printErrorMsg(*it, CONSTTRUNC, (*it)->getValue()->getValue());
            }
            
            (*constants)[(*it)->getName()] = val.first;
        }
        
        delete *it;
    }
    
    if(failure)
    {
        return false;
    }
    
    return true;
}

SymbolData *ScriptParser::buildSymbolTable(AST *theAST, map<string, long> *constants)
{
    SymbolData *rval = new SymbolData();
    SymbolTable *t = new SymbolTable(constants);
    Scope *globalScope = new Scope(NULL);
    bool failure = false;
    
    //ADD LIBRARY FUNCTIONS TO THE GLOBAL SCOPE HERE
    GlobalSymbols::getInst().addSymbolsToScope(globalScope, t);
    FFCSymbols::getInst().addSymbolsToScope(globalScope,t);
    ItemSymbols::getInst().addSymbolsToScope(globalScope,t);
    ItemclassSymbols::getInst().addSymbolsToScope(globalScope,t);
    LinkSymbols::getInst().addSymbolsToScope(globalScope,t);
    ScreenSymbols::getInst().addSymbolsToScope(globalScope,t);
    GameSymbols::getInst().addSymbolsToScope(globalScope,t);
    NPCSymbols::getInst().addSymbolsToScope(globalScope,t);
    LinkWeaponSymbols::getInst().addSymbolsToScope(globalScope,t);
    EnemyWeaponSymbols::getInst().addSymbolsToScope(globalScope,t);
    
    //strip the global functions from the AST
    GetGlobalFuncs gc;
    theAST->execute(gc, NULL);
    vector<ASTFuncDecl *> fds = gc.getResult();
    
    //add these functions to the global scope
    for(vector<ASTFuncDecl *>::iterator it = fds.begin(); it != fds.end(); it++)
    {
        vector<int> params;
        
        for(list<ASTVarDecl *>::iterator it2 = (*it)->getParams().begin();
                it2 != (*it)->getParams().end(); it2++)
        {
            int type;
            ExtractType temp;
            (*it2)->getType()->execute(temp, &type);
            
            if(type == ScriptParser::TYPE_VOID)
            {
                printErrorMsg(*it2, FUNCTIONVOIDPARAM, (*it2)->getName());
                failure=true;
            }
            
            params.push_back(type);
        }
        
        int rettype;
        ExtractType temp;
        (*it)->getReturnType()->execute(temp, &rettype);
        int id = globalScope->getFuncSymbols().addFunction((*it)->getName(), rettype, params);
        
        if(id == -1)
        {
            printErrorMsg(*it, FUNCTIONREDEF, (*it)->getName());
            failure=true;
        }
        
        if(failure)
        {
            for(vector<ASTFuncDecl *>::iterator it2 = fds.begin(); it2 != fds.end(); it2++)
            {
                delete *it2;
            }
            
            delete globalScope;
            delete t;
            delete rval;
            delete theAST;
            return NULL;
        }
        
        t->putAST(*it, id);
        t->putFunc(id, rettype);
        t->putFuncDecl(id, params);
        
    }
    
    rval->globalFuncs = fds;
    
    
    
    //add global pointers
    int vid2;
    
    //add a Link global variable
    vid2 = globalScope->getVarSymbols().addVariable("Link", ScriptParser::TYPE_LINK);
    t->putVar(vid2, ScriptParser::TYPE_LINK);
    t->addGlobalPointer(vid2);
    //add a Screen global variable
    vid2 = globalScope->getVarSymbols().addVariable("Screen", ScriptParser::TYPE_SCREEN);
    t->putVar(vid2, ScriptParser::TYPE_SCREEN);
    t->addGlobalPointer(vid2);
    //add a Game global variable
    vid2 = globalScope->getVarSymbols().addVariable("Game", ScriptParser::TYPE_GAME);
    t->putVar(vid2, ScriptParser::TYPE_GAME);
    t->addGlobalPointer(vid2);
    
    //strip the global variables from the AST
    GetGlobalVars gv;
    theAST->execute(gv, NULL);
    vector<ASTVarDecl *> gvs = gv.getResult();
    vector<ASTArrayDecl *> gvas = gv.getResultA();
    
    //add the variables to the global scope
    for(vector<ASTVarDecl *>::iterator it = gvs.begin(); it != gvs.end(); it++)
    {
        BuildScriptSymbols bss;
        pair<Scope * ,SymbolTable *> param(globalScope, t);
        (*it)->execute(bss, &param);
        
        if(!bss.isOK())
            failure = true;
    }
    
    for(vector<ASTArrayDecl *>::iterator it = gvas.begin(); it != gvas.end(); it++)
    {
        BuildScriptSymbols bss;
        pair<Scope * , SymbolTable *> param(globalScope, t);
        (*it)->execute(bss, &param);
        
        if(!bss.isOK())
            failure = true;
    }
    
    vector<ASTScript *> scripts;
    
    if(!failure)
    {
        //put script variables and functions in their script subscopes
        GetScripts gs;
        theAST->execute(gs, NULL);
        scripts = gs.getResult();
        
        for(vector<ASTScript *>::iterator it = scripts.begin(); it != scripts.end(); it++)
        {
            int scripttype;
            ExtractType et;
            (*it)->getType()->execute(et, &scripttype);
            
            if(!(scripttype == ScriptParser::TYPE_FFC || scripttype == ScriptParser::TYPE_ITEMCLASS
                    || scripttype == ScriptParser::TYPE_GLOBAL))
            {
                printErrorMsg(*it, SCRIPTBADTYPE, (*it)->getName());
                failure = true;
                continue;
            }
            
            Scope *subscope = new Scope(globalScope);
            
            if(!globalScope->addNamedChild((*it)->getName(), subscope))
            {
                printErrorMsg(*it, SCRIPTREDEF, (*it)->getName());
                failure = true;
                delete subscope;
                continue;
            }
            
            pair<Scope *, SymbolTable *> param(subscope, t);
            BuildScriptSymbols bss;
            bss.enableDeprecationWarnings();
            (*it)->execute(bss, &param);
            
            if(!bss.isOK())
                failure=true;
            else
            {
                //find the start symbol
                vector<int> possibleruns = subscope->getFuncsInScope((*it)->getName(), "run");
                int runid = -1;
                
                if(possibleruns.size() > 1)
                {
                    printErrorMsg(*it, TOOMANYRUN, (*it)->getName());
                    failure = true;
                }
                else if(possibleruns.size() == 1)
                    runid = possibleruns[0];
                    
                if(!failure)
                {
                    if(runid == -1)
                    {
                        printErrorMsg(*it, SCRIPTNORUN, (*it)->getName());
                        failure = true;
                    }
                    else
                    {
                        int type = t->getFuncType(runid);
                        
                        if(type != ScriptParser::TYPE_VOID)
                        {
                            printErrorMsg(*it, SCRIPTRUNNOTVOID, (*it)->getName());
                            failure = true;
                        }
                        else
                        {
                            rval->runsymbols[*it] = runid;
                            rval->numParams[*it] = (int)t->getFuncParams(runid).size();
                            rval->scriptTypes[*it] = scripttype;
                        }
                    }
                }
            }
        }
    }
    
    //all non-local variables have been added to the table, so process all function
    //declarations and add their local variables. As well, look up the symbol id of
    //all variables and functions accessed and called within that function
    
    //starting with global functions
    if(!failure)
    {
        for(vector<ASTFuncDecl *>::iterator it = fds.begin(); it != fds.end(); it++)
        {
            Scope *subscope = new Scope(globalScope);
            BFSParam param = {subscope, t,ScriptParser::TYPE_VOID};
            BuildFunctionSymbols bfs;
            (*it)->execute(bfs, &param);
            
            if(!bfs.isOK())
                failure = true;
                
            delete subscope;
        }
    }
    
    //now do script function
    if(!failure)
    {
        for(vector<ASTScript *>::iterator it = scripts.begin(); it != scripts.end(); it++)
        {
            Scope *subscope = globalScope->getNamedChild((*it)->getName());
            Scope *newscope = new Scope(subscope);
            BFSParam param = {newscope, t,rval->scriptTypes[*it]};
            list<ASTDecl *> decls = (*it)->getScriptBlock()->getDeclarations();
            
            for(list<ASTDecl *>::iterator it2 = decls.begin(); it2 != decls.end(); it2++)
            {
                bool isfuncdecl;
                IsFuncDecl temp;
                (*it2)->execute(temp, &isfuncdecl);
                
                if(isfuncdecl)
                {
                    BuildFunctionSymbols bfs;
                    (*it2)->execute(bfs, &param);
                    
                    if(!bfs.isOK())
                        failure = true;
                        
                    if(bfs.getThisVID() != -1)
                        rval->thisPtr[*it]=bfs.getThisVID();
                }
                
            }
            
            delete newscope;
        }
    }
    
    if(failure)
    {
        for(vector<ASTFuncDecl *>::iterator it2 = fds.begin(); it2 != fds.end(); it2++)
        {
            delete *it2;
        }
        
        for(vector<ASTScript *>::iterator it2 = scripts.begin(); it2 != scripts.end(); it2++)
        {
            delete *it2;
        }
        
        for(vector<ASTVarDecl *>::iterator it2 = gvs.begin(); it2 != gvs.end(); it2++)
            delete *it2;
            
        for(vector<ASTArrayDecl *>::iterator it2 = gvas.begin(); it2 != gvas.end(); it2++)
            delete *it2;
            
        delete globalScope;
        delete t;
        delete rval;
        delete theAST;
        return NULL;
    }
    
    delete globalScope;
    delete theAST;
    rval->symbols = t;
    rval->scripts = scripts;
    rval->globalFuncs = fds;
    rval->globalVars = gvs;
    rval->globalArrays = gvas;
    
    return rval;
}

FunctionData *ScriptParser::typeCheck(SymbolData *sdata)
{
    //build the functiondata
    FunctionData *fd = new FunctionData;
    fd->symbols = sdata->symbols;
    fd->newGlobalVars = sdata->globalVars;
    fd->newGlobalArrays = sdata->globalArrays;
    vector<ASTScript *> scripts = sdata->scripts;
    vector<ASTFuncDecl *> funcs = sdata->globalFuncs;
    map<ASTScript *, int> runsymbols = sdata->runsymbols;
    map<ASTScript *, int> numparams = sdata->numParams;
    map<ASTScript *, int> scripttypes = sdata->scriptTypes;
    map<ASTScript *, int> thisptr = sdata->thisPtr;
    delete sdata;
    bool failure = false;
    map<int, bool> usednums;
    
    //strip var and func decls from the scripts
    for(vector<ASTScript *>::iterator it = scripts.begin(); it != scripts.end(); it++)
    {
        fd->scriptRunSymbols[(*it)->getName()] = runsymbols[*it];
        fd->numParams[(*it)->getName()] = numparams[*it];
        fd->scriptTypes[(*it)->getName()] = scripttypes[*it];
        fd->thisPtr[(*it)->getName()] = thisptr[*it];
        //strip vars and funcs
        list<ASTDecl *> &stuff = (*it)->getScriptBlock()->getDeclarations();
        
        for(list<ASTDecl *>::iterator it2 = stuff.begin(); it2 != stuff.end();)
        {
            bool isFunc = false;
            IsFuncDecl temp;
            (*it2)->execute(temp, &isFunc);
            
            if(isFunc)
            {
                fd->functions.push_back((ASTFuncDecl *)*it2);
            }
            
            bool IsArray = false;
            IsArrayDecl temp2;
            (*it2)->execute(temp2, &IsArray);
            
            if(IsArray)
            {
                fd->globalArrays.push_back((ASTArrayDecl *)*it2);
            }
            
            if(!isFunc && !IsArray)
            {
                fd->globalVars.push_back((ASTVarDecl *)*it2);
            }
            
            it2 = stuff.erase(it2);
        }
    }
    
    for(vector<ASTScript *>::iterator it = scripts.begin(); it != scripts.end(); it++)
    {
        delete *it;
    }
    
    for(vector<ASTFuncDecl *>::iterator it = funcs.begin(); it != funcs.end(); it++)
    {
        fd->functions.push_back(*it);
    }
    
    if(failure)
    {
        //delete stuff
        for(vector<ASTVarDecl *>::iterator it = fd->globalVars.begin(); it != fd->globalVars.end(); it++)
        {
            delete *it;
        }
        
        for(vector<ASTArrayDecl *>::iterator it = fd->globalArrays.begin(); it != fd->globalArrays.end(); it++)
        {
            delete *it;
        }
        
        for(vector<ASTFuncDecl *>::iterator it = fd->functions.begin(); it != fd->functions.end(); it++)
        {
            delete *it;
        }
        
        for(vector<ASTVarDecl *>::iterator it = fd->newGlobalVars.begin(); it != fd->newGlobalVars.end(); it++)
            delete *it;
            
        for(vector<ASTArrayDecl *>::iterator it = fd->newGlobalArrays.begin(); it != fd->newGlobalArrays.end(); it++)
            delete *it;
            
        delete fd->symbols;
        delete fd;
        return NULL;
    }
    
    //fd is now loaded with all the info
    //so run type-checker visitor
    for(vector<ASTVarDecl *>::iterator it = fd->globalVars.begin(); it != fd->globalVars.end(); it++)
    {
        pair<SymbolTable *, int> param = pair<SymbolTable *, int>(fd->symbols, -1);
        TypeCheck tc;
        (*it)->execute(tc, &param);
        
        if(!tc.isOK())
            failure = true;
    }
    
    for(vector<ASTArrayDecl *>::iterator it = fd->globalArrays.begin(); it != fd->globalArrays.end(); it++)
    {
        pair<SymbolTable *, int> param = pair<SymbolTable *, int>(fd->symbols, -1);
        TypeCheck tc;
        (*it)->execute(tc, &param);
        
        if(!tc.isOK())
            failure = true;
    }
    
    for(vector<ASTVarDecl *>::iterator it = fd->newGlobalVars.begin(); it != fd->newGlobalVars.end(); it++)
    {
        pair<SymbolTable *, int> param = pair<SymbolTable *, int>(fd->symbols, -1);
        TypeCheck tc;
        (*it)->execute(tc, &param);
        
        if(!tc.isOK())
            failure = true;
    }
    
    for(vector<ASTArrayDecl *>::iterator it = fd->newGlobalArrays.begin(); it != fd->newGlobalArrays.end(); it++)
    {
        pair<SymbolTable *, int> param = pair<SymbolTable *, int>(fd->symbols, -1);
        TypeCheck tc;
        (*it)->execute(tc, &param);
        
        if(!tc.isOK())
            failure = true;
    }
    
    for(vector<ASTFuncDecl *>::iterator it = fd->functions.begin(); it != fd->functions.end(); it++)
    {
        int rettype = fd->symbols->getFuncType(*it);
        pair<SymbolTable *, int> param = pair<SymbolTable *, int>(fd->symbols, rettype);
        TypeCheck tc;
        (*it)->execute(tc, &param);
        
        if(!tc.isOK())
            failure = true;
    }
    
    if(fd->globalVars.size() + fd->newGlobalVars.size() > 256)
    {
        printErrorMsg(NULL, TOOMANYGLOBAL);
        failure = true;
    }
    
    if(failure)
    {
        //delete stuff
        for(vector<ASTVarDecl *>::iterator it = fd->globalVars.begin(); it != fd->globalVars.end(); it++)
        {
            delete *it;
        }
        
        for(vector<ASTArrayDecl *>::iterator it = fd->globalArrays.begin(); it != fd->globalArrays.end(); it++)
        {
            delete *it;
        }
        
        for(vector<ASTFuncDecl *>::iterator it = fd->functions.begin(); it != fd->functions.end(); it++)
        {
            delete *it;
        }
        
        for(vector<ASTVarDecl *>::iterator it = fd->newGlobalVars.begin(); it != fd->newGlobalVars.end(); it++)
            delete *it;
            
        for(vector<ASTArrayDecl *>::iterator it = fd->newGlobalArrays.begin(); it != fd->newGlobalArrays.end(); it++)
            delete *it;
            
        delete fd->symbols;
        delete fd;
        return NULL;
    }
    
    return fd;
}

IntermediateData *ScriptParser::generateOCode(FunctionData *fdata)
{
    //Z_message("yes");
    bool failure = false;
    vector<ASTFuncDecl *> funcs = fdata->functions;
    vector<ASTVarDecl *> globals = fdata->globalVars;
    vector<ASTArrayDecl *> globalas = fdata->globalArrays;
    
    //we have no need of newglobals at this point anymore
    for(vector<ASTVarDecl *>::iterator it = fdata->newGlobalVars.begin(); it != fdata->newGlobalVars.end(); it++)
        globals.push_back(*it);
        
    for(vector<ASTArrayDecl *>::iterator it = fdata->newGlobalArrays.begin(); it != fdata->newGlobalArrays.end(); it++)
        globalas.push_back(*it);
        
    map<string, int> runsymbols = fdata->scriptRunSymbols;
    SymbolTable *symbols = fdata->symbols;
    map<string, int> numparams = fdata->numParams;
    map<string, int> scripttypes = fdata->scriptTypes;
    map<string, int> thisptr = fdata->thisPtr;
    delete fdata;
    LinkTable lt;
    
    for(vector<ASTVarDecl *>::iterator it = globals.begin(); it != globals.end(); it++)
    {
        int vid2 = symbols->getID(*it);
        lt.addGlobalVar(vid2);
    }
    
    for(vector<ASTArrayDecl *>::iterator it = globalas.begin(); it != globalas.end(); it++)
    {
        int vid2 = symbols->getID(*it);
        lt.addGlobalVar(vid2);
    }
    
    //Z_message("yes");
    //and add the this pointers
    for(vector<int>::iterator it = symbols->getGlobalPointers().begin(); it != symbols->getGlobalPointers().end(); it++)
    {
        lt.addGlobalPointer(*it);
    }
    
    for(vector<ASTFuncDecl *>::iterator it = funcs.begin(); it != funcs.end(); it++)
    {
        int fid2 = symbols->getID(*it);
        lt.functionToLabel(fid2);
    }
    
    //Z_message("yes");
    
    //we now have labels for the functions and ids for the global variables.
    //we can now generate the code to intialize the globals
    IntermediateData *rval = new IntermediateData();
    
    //Link against the global symbols, and add their labels
    map<int, vector<Opcode *> > globalcode = GlobalSymbols::getInst().addSymbolsCode(lt);
    
    for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
    {
        rval->funcs[it->first] = it->second;
    }
    
    globalcode = FFCSymbols::getInst().addSymbolsCode(lt);
    
    for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
    {
        rval->funcs[it->first] = it->second;
    }
    
    globalcode = ItemSymbols::getInst().addSymbolsCode(lt);
    
    for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
    {
        rval->funcs[it->first] = it->second;
    }
    
    globalcode = ItemclassSymbols::getInst().addSymbolsCode(lt);
    
    for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
    {
        rval->funcs[it->first] = it->second;
    }
    
    globalcode = LinkSymbols::getInst().addSymbolsCode(lt);
    
    for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
    {
        rval->funcs[it->first] = it->second;
    }
    
    globalcode = ScreenSymbols::getInst().addSymbolsCode(lt);
    
    for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
    {
        rval->funcs[it->first] = it->second;
    }
    
    globalcode = GameSymbols::getInst().addSymbolsCode(lt);
    
    for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
    {
        rval->funcs[it->first] = it->second;
    }
    
    globalcode = NPCSymbols::getInst().addSymbolsCode(lt);
    
    for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
    {
        rval->funcs[it->first] = it->second;
    }
    
    globalcode = LinkWeaponSymbols::getInst().addSymbolsCode(lt);
    
    for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
    {
        rval->funcs[it->first] = it->second;
    }
    
    globalcode = EnemyWeaponSymbols::getInst().addSymbolsCode(lt);
    
    for(map<int, vector<Opcode *> >::iterator it = globalcode.begin(); it != globalcode.end(); it++)
    {
        rval->funcs[it->first] = it->second;
    }
    
    //Z_message("yes");
    
    for(vector<ASTVarDecl *>::iterator it = globals.begin(); it != globals.end(); it++)
    {
        OpcodeContext oc;
        oc.linktable = &lt;
        oc.symbols = symbols;
        oc.stackframe = NULL;
        BuildOpcodes bo;
        (*it)->execute(bo, &oc);
        
        if(!bo.isOK())
        {
            failure = true;
        }
        
        vector<Opcode *> code = bo.getResult();
        
        for(vector<Opcode *>::iterator it2 = code.begin(); it2!= code.end(); it2++)
        {
            rval->globalsInit.push_back(*it2);
        }
        
        delete *it; //say so long to our lovely data structure the AST
    }
    
    //Z_message("yes");
    for(vector<ASTArrayDecl *>::iterator it = globalas.begin(); it != globalas.end(); it++)
    {
        OpcodeContext oc;
        oc.linktable = &lt;
        oc.symbols = symbols;
        oc.stackframe = NULL;
        BuildOpcodes bo;
        (*it)->execute(bo, &oc);
        
        if(!bo.isOK())
        {
            failure = true;
        }
        
        vector<Opcode *> code = bo.getResult();
        
        for(vector<Opcode *>::iterator it2 = code.begin(); it2!= code.end(); it2++)
        {
            rval->globalasInit.push_back(*it2);
        }
        
        delete *it; //say so long to our lovely data structure the AST
    }
    
    //Z_message("yes");
    
    //globals have been initialized, now we repeat for the functions
    for(vector<ASTFuncDecl *>::iterator it = funcs.begin(); it != funcs.end(); it++)
    {
        bool isarun = false;
        string scriptname;
        
        for(map<string,int>::iterator it2 = runsymbols.begin(); it2 != runsymbols.end(); it2++)
        {
            if(it2->second == symbols->getID(*it))
            {
                isarun=true;
                scriptname = it2->first;
                break;
            }
        }
        
        vector<Opcode *> funccode;
		// generate a mapping from local variables to stack offests
		StackFrame sf;

		int offset = 0;
		//if this is a run, there is the this pointer
		if (isarun)
		{
			sf.addToFrame(thisptr[scriptname], offset);
			offset += 10000;
		}

        //assign the local, non-parameters to slots on the stack
        
        AssignStackSymbols assign(&sf, symbols, offset);
        (*it)->getBlock()->execute(assign, NULL);
        
		offset = assign.getHighWaterOffset();
        
        //finally, assign the parameters, in reverse order
		for (list<ASTVarDecl *>::reverse_iterator paramit = (*it)->getParams().rbegin(); paramit != (*it)->getParams().rend(); ++paramit)
		{
			int vid = symbols->getID(*paramit);
			sf.addToFrame(vid, offset);
			offset += 10000;
		}

		int totvars = offset / 10000;

        //start of the function
        Opcode *first = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
        first->setLabel(lt.functionToLabel(symbols->getID(*it)));
        funccode.push_back(first);
        //push on the 0s
        int numtoallocate = totvars-(unsigned int)symbols->getFuncParams(symbols->getID(*it)).size();
		
	//Skip the runline: Fix for run() params being eaten. 
	if (isarun) numtoallocate--;

        for(int i = 0; i < numtoallocate; i++)
        {
            funccode.push_back(new OPushRegister(new VarArgument(EXP1)));
        }
        
        //push on the this, if a script
        if(isarun)
        {
            switch(scripttypes[scriptname])
            {
            case ScriptParser::TYPE_FFC:
                funccode.push_back(new OSetRegister(new VarArgument(EXP2), new VarArgument(REFFFC)));
                break;
                
            case ScriptParser::TYPE_ITEMCLASS:
                funccode.push_back(new OSetRegister(new VarArgument(EXP2), new VarArgument(REFITEMCLASS)));
                break;
                
            case ScriptParser::TYPE_GLOBAL:
                //don't care, we don't have a valid this pointer
                break;
            }
            
            funccode.push_back(new OPushRegister(new VarArgument(EXP2)));
        }
        
        //set up the stack frame register
        funccode.push_back(new OSetRegister(new VarArgument(SFRAME), new VarArgument(SP)));
        OpcodeContext oc;
        oc.linktable = &lt;
        oc.symbols = symbols;
        oc.stackframe = &sf;
        BuildOpcodes bo;
        (*it)->execute(bo, &oc);
        
        if(!bo.isOK())
            failure = true;
            
        vector<Opcode *> code = bo.getResult();
        
        for(vector<Opcode *>::iterator it2 = code.begin(); it2 != code.end(); it2++)
        {
            funccode.push_back(*it2);
        }
        
        //add appendix code
        //nop label
        Opcode *next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
        next->setLabel(bo.getReturnLabelID());
        funccode.push_back(next);
        
        //pop off everything
        for(int i=0; i< totvars; i++)
        {
            funccode.push_back(new OPopRegister(new VarArgument(EXP2)));
        }
        
        //if it's a main script, quit.
		if (isarun)
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
        
        rval->funcs[lt.functionToLabel(symbols->getID(*it))]=funccode;
        delete *it;
    }
    
    //Z_message("yes");
    
    //update the run symbols
    for(map<string, int>::iterator it = runsymbols.begin(); it != runsymbols.end(); it++)
    {
        int labelid = lt.functionToLabel(it->second);
        rval->scriptRunLabels[it->first] = labelid;
        rval->numParams[it->first] = numparams[it->first];
        rval->scriptTypes[it->first] = scripttypes[it->first];
    }
    
    delete symbols; //and so long to our beloved ;) symbol table
    //Z_message("yes");
    
    if(failure)
    {
        //delete all kinds of crap if there was a problem :-/
        for(map<int, vector<Opcode *> >::iterator it = rval->funcs.begin(); it != rval->funcs.end(); it++)
        {
            for(vector<Opcode *>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
            {
                delete *it2;
            }
        }
        
        for(vector<Opcode *>::iterator it = rval->globalsInit.begin(); it != rval->globalsInit.end(); it++)
        {
            delete *it;
        }
        
        for(vector<Opcode *>::iterator it = rval->globalasInit.begin(); it != rval->globalasInit.end(); it++)
        {
            delete *it;
        }
        
        delete rval;
        return NULL;
    }
    
    //Z_message("yes");
    return rval;
}

ScriptsData *ScriptParser::assemble(IntermediateData *id)
{
    //finally, finish off this bitch
    ScriptsData *rval = new ScriptsData;
    map<int, vector<Opcode *> > funcs = id->funcs;
    vector<Opcode *> ginit = id->globalsInit;
    {
        vector<Opcode *> temp = id->globalasInit;
        
        //push global array allocations onto the global variable allocations
        for(vector<Opcode *>::iterator i = temp.begin(); i != temp.end(); i++)
            ginit.push_back(*i);
    }
    map<string, int> scripts = id->scriptRunLabels;
    map<string, int> numparams = id->numParams;
    map<string, int> scripttypes = id->scriptTypes;
    delete id;
    
    //do the global inits
    //if there's a global script called "Init", append it to ~Init:
    map<string, int>::iterator it = scripts.find("Init");
    
    if(it != scripts.end() && scripttypes["Init"] == ScriptParser::TYPE_GLOBAL)
    {
        //append
        //get label
        int label = funcs[scripts["Init"]][0]->getLabel();
        ginit.push_back(new OGotoImmediate(new LabelArgument(label)));
    }
    
    rval->theScripts["~Init"] = assembleOne(ginit, funcs, 0);
    rval->scriptTypes["~Init"] = ScriptParser::TYPE_GLOBAL;
    
    for(map<string, int>::iterator it2 = scripts.begin(); it2 != scripts.end(); it2++)
    {
        vector<Opcode *> code = funcs[it2->second];
        rval->theScripts[it2->first] = assembleOne(code, funcs, numparams[it2->first]);
        rval->scriptTypes[it2->first] = scripttypes[it2->first];
    }
    
    for(vector<Opcode *>::iterator it2 = ginit.begin(); it2 != ginit.end(); it2++)
    {
        delete *it2;
    }
    
    for(map<int, vector<Opcode *> >::iterator it2 = funcs.begin(); it2 != funcs.end(); it2++)
    {
        for(vector<Opcode *>::iterator it3 = it2->second.begin(); it3 != it2->second.end(); it3++)
        {
            delete *it3;
        }
    }
    
    return rval;
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


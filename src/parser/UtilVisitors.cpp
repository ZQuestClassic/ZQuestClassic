//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "UtilVisitors.h"
#include <assert.h>
#include "../zsyssimple.h"
#include "ParseError.h"

////////////////////////////////////////////////////////////////////////////////
void GetImports::caseDefault(void *param)
{
    if(param != NULL)
        *(bool *)param = false;
}

void GetImports::caseImportDecl(ASTImportDecl &, void *param)
{
    if(param != NULL)
        *(bool *)param = true;
}

void GetImports::caseDeclList(ASTDeclList &host, void *)
{
    list<ASTDecl *> &l = host.getDeclarations();
    
    for(list<ASTDecl *>::iterator it = l.begin(); it != l.end();)
    {
        bool isimport;
        (*it)->execute(*this, &isimport);
        
        if(isimport)
        {
            result.push_back((ASTImportDecl *)*it);
            it=l.erase(it);
        }
        else
            it++;
    }
}

void GetImports::caseProgram(ASTProgram &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    host.getDeclarations()->execute(*this,NULL);
}

////////////////////////////////////////////////////////////////////////////////
void GetConsts::caseDefault(void *param)
{
    if(param != NULL)
        *(bool *)param = false;
}

void GetConsts::caseConstDecl(ASTConstDecl &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    void *temp;
    temp=&host;
    
    if(param != NULL)
        *(bool *)param = true;
}

void GetConsts::caseDeclList(ASTDeclList &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    list<ASTDecl *> &l = host.getDeclarations();
    
    for(list<ASTDecl *>::iterator it = l.begin(); it != l.end();)
    {
        bool isconst;
        (*it)->execute(*this, &isconst);
        
        if(isconst)
        {
            result.push_back((ASTConstDecl *)*it);
            it=l.erase(it);
        }
        else
            it++;
    }
}

void GetConsts::caseProgram(ASTProgram &host, void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    host.getDeclarations()->execute(*this,NULL);
}

void GetGlobalVars::caseDefault(void *param)
{
    if(param != NULL)
        *(int *)param = 0;
}

void GetGlobalVars::caseVarDecl(ASTVarDecl &, void *param)
{
    if(param != NULL)
        *(int *)param = 1;
}

void GetGlobalVars::caseVarDeclInitializer(ASTVarDeclInitializer &, void *param)
{
    if(param != NULL)
        *(int *)param = 1;
}

void GetGlobalVars::caseArrayDecl(ASTArrayDecl &, void *param)
{
    if(param != NULL)
        *(int *)param = 2;
}

void GetGlobalVars::caseProgram(ASTProgram &host, void *)
{
    host.getDeclarations()->execute(*this, NULL);
}

void GetGlobalVars::caseDeclList(ASTDeclList &host, void *)
{
    list<ASTDecl *> &l = host.getDeclarations();
    
    for(list<ASTDecl *>::iterator it = l.begin(); it != l.end();)
    {
        int dectype;
        (*it)->execute(*this, &dectype);
        
        if(dectype==1)
        {
            result.push_back((ASTVarDecl *)(*it));
            it=l.erase(it);
        }
        else if(dectype==2)
        {
            resultA.push_back((ASTArrayDecl *)(*it));
            it=l.erase(it);
        }
        else
            it++;
    }
}

void GetGlobalFuncs::caseDefault(void *param)
{
    if(param != NULL)
        *(bool *)param = false;
}

void GetGlobalFuncs::caseFuncDecl(ASTFuncDecl &, void *param)
{
    if(param != NULL)
        *(bool *)param = true;
}

void GetGlobalFuncs::caseDeclList(ASTDeclList &host, void *)
{
    list<ASTDecl *> &l = host.getDeclarations();
    
    for(list<ASTDecl *>::iterator it = l.begin(); it != l.end();)
    {
        bool isfuncdecl;
        (*it)->execute(*this, &isfuncdecl);
        
        if(isfuncdecl)
        {
            result.push_back((ASTFuncDecl *)*it);
            it=l.erase(it);
        }
        else
            it++;
    }
}

void GetGlobalFuncs::caseProgram(ASTProgram &host, void *)
{
    host.getDeclarations()->execute(*this,NULL);
}

void GetScripts::caseDefault(void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    //there should be nothing left in here now
    assert(false);
}

void GetScripts::caseProgram(ASTProgram &host, void *param)
{
    host.getDeclarations()->execute(*this,param);
}

void GetScripts::caseDeclList(ASTDeclList &host, void *param)
{
    list<ASTDecl *> &l = host.getDeclarations();
    
    for(list<ASTDecl *>::iterator it = l.begin(); it != l.end();)
    {
        (*it)->execute(*this, param);
        result.push_back((ASTScript *)*it);
        it=l.erase(it);
    }
}

void GetScripts::caseScript(ASTScript &host, void *param)
{
    void *temp;
    temp=&host;
    param=param; /*these are here to bypass compiler warnings about unused arguments*/
}

void MergeASTs::caseDefault(void *param)
{
    //these are here to bypass compiler warnings about unused arguments
    param=param;
    
    box_out("Something BAD BROKEN in the parser code!");
    box_eol();
    assert(false);
}

void MergeASTs::caseProgram(ASTProgram &host, void *param)
{
    assert(param);
    ASTProgram *other = (ASTProgram *)param;
    list<ASTDecl *> &decls = other->getDeclarations()->getDeclarations();
    
    for(list<ASTDecl *>::iterator it = decls.begin(); it != decls.end();)
    {
        host.getDeclarations()->addDeclaration((*it));
        it = decls.erase(it);
    }
    
    delete other;
}

void CheckForExtraneousImports::caseDefault(void *)
{

}

void CheckForExtraneousImports::caseImportDecl(ASTImportDecl &host, void *)
{
    ok = false;
    printErrorMsg(&host, IMPORTBADSCOPE);
}

void ExtractType::caseDefault(void *)
{
    //unreachable
    assert(false);
}

void ExtractType::caseTypeBool(ASTTypeBool &, void *param)
{
    *(int *)param = ScriptParser::TYPE_BOOL;
}

void ExtractType::caseTypeFloat(ASTTypeFloat &, void *param)
{
    *(int *)param = ScriptParser::TYPE_FLOAT;
}

void ExtractType::caseTypeVoid(ASTTypeVoid &, void *param)
{
    *(int *)param = ScriptParser::TYPE_VOID;
}

void ExtractType::caseTypeFFC(ASTTypeFFC &, void *param)
{
    *(int *)param = ScriptParser::TYPE_FFC;
}

void ExtractType::caseTypeGlobal(ASTTypeGlobal &, void *param)
{
    *(int *)param = ScriptParser::TYPE_GLOBAL;
}

void ExtractType::caseTypeItem(ASTTypeItem &, void *param)
{
    *(int *)param = ScriptParser::TYPE_ITEM;
}

void ExtractType::caseTypeItemclass(ASTTypeItemclass &, void *param)
{
    *(int *)param = ScriptParser::TYPE_ITEMCLASS;
}

void ExtractType::caseTypeNPC(ASTTypeNPC &, void *param)
{
    *(int *)param = ScriptParser::TYPE_NPC;
}

void ExtractType::caseTypeLWpn(ASTTypeLWpn &, void *param)
{
    *(int *)param = ScriptParser::TYPE_LWPN;
}

void ExtractType::caseTypeEWpn(ASTTypeEWpn &, void *param)
{
    *(int *)param = ScriptParser::TYPE_EWPN;
}
void ExtractType::caseTypeNPCData(ASTTypeNPCData &, void *param)
{
    *(int *)param = ScriptParser::TYPE_NPCDATA;
}
void ExtractType::caseTypeMapData(ASTTypeMapData &, void *param)
{
    *(int *)param = ScriptParser::TYPE_MAPDATA;
}
void ExtractType::caseTypeDebug(ASTTypeDebug &, void *param)
{
    *(int *)param = ScriptParser::TYPE_DEBUG;
}
void ExtractType::caseTypeAudio(ASTTypeAudio &, void *param)
{
    *(int *)param = ScriptParser::TYPE_AUDIO;
}
void ExtractType::caseTypeComboData(ASTTypeComboData &, void *param)
{
    *(int *)param = ScriptParser::TYPE_COMBOS;
}
void ExtractType::caseTypeSpriteData(ASTTypeSpriteData &, void *param)
{
    *(int *)param = ScriptParser::TYPE_SPRITEDATA;
}
void ExtractType::caseTypeGraphics(ASTTypeGraphics &, void *param)
{
    *(int *)param = ScriptParser::TYPE_GRAPHICS;
}
void ExtractType::caseTypeText(ASTTypeText &, void *param)
{
    *(int *)param = ScriptParser::TYPE_TEXT;
}
void ExtractType::caseTypeInput(ASTTypeInput &, void *param)
{
    *(int *)param = ScriptParser::TYPE_INPUT;
}


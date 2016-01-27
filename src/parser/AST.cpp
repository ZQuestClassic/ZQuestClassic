
#include "../precompiled.h" //always first
#include "AST.h"


ASTProgram::~ASTProgram()
{
    delete decls;
}

ASTDeclList::~ASTDeclList()
{
    list<ASTDecl *>::iterator it;
    
    for(it = decls.begin(); it != decls.end(); it++)
    {
        delete *it;
    }
    
    decls.clear();
}

void ASTDeclList::addDeclaration(ASTDecl *newdecl)
{
    decls.push_front(newdecl);
}

ASTFuncDecl::~ASTFuncDecl()
{
    delete rettype;
    delete block;
    list<ASTVarDecl *>::iterator it;
    
    for(it = params.begin(); it != params.end(); it++)
    {
        delete *it;
    }
    
    params.clear();
}

ASTArrayDecl::~ASTArrayDecl()
{
    delete type;
    delete list;
    delete size;
}

void ASTArrayList::addParam(ASTExpr *expr)
{
    exprs.push_back(expr);
}

ASTArrayList::~ASTArrayList()
{
    list<ASTExpr *>::iterator it;
    
    for(it = exprs.begin(); it != exprs.end(); it++)
    {
        delete *it;
    }
    
    exprs.clear();
}

void ASTFuncDecl::addParam(ASTVarDecl *param)
{
    params.push_front(param);
}

ASTVarDecl::~ASTVarDecl()
{
    delete type;
}

ASTVarDeclInitializer::~ASTVarDeclInitializer()
{
    delete initial;
}


ASTFuncCall::~ASTFuncCall()
{
    list<ASTExpr *>::iterator it;
    
    for(it = params.begin(); it != params.end(); it++)
    {
        delete *it;
    }
    
    params.clear();
    delete name;
}

void ASTBlock::addStatement(ASTStmt *stmt)
{
    statements.push_back(stmt);
}

ASTBlock::~ASTBlock()
{
    list<ASTStmt *>::iterator it;
    
    for(it=statements.begin(); it != statements.end(); it++)
    {
        delete *it;
    }
    
    statements.clear();
}

ASTScript::~ASTScript()
{
    delete sblock;
    delete type;
}

pair<string, string> ASTFloat::parseValue()
{
    string f = getValue();
    string intpart;
    string fpart;
    
    switch(getType())
    {
    case TYPE_DECIMAL:
    {
        bool founddot = false;
        
        for(unsigned int i=0; i<f.size(); i++)
        {
            if(f.at(i) == '.')
            {
                intpart = f.substr(0, i);
                fpart = f.substr(i+1,f.size()-i-1);
                founddot = true;
                break;
            }
        }
        
        if(!founddot)
        {
            intpart = f;
            fpart = "";
        }
        
        if(negative) intpart = "-" + intpart;
        
        break;
    }
    
    case TYPE_HEX:
    {
        //trim off the "0x"
        f = f.substr(2,f.size()-2);
        //parse the hex
        long val2=0;
        
        for(unsigned int i=0; i<f.size(); i++)
        {
            char d = f.at(i);
            val2*=16;
            
            if('0' <= d && d <= '9')
                val2+=(d-'0');
            else if('A' <= d && d <= 'F')
                val2+=(10+d-'A');
            else
                val2+=(10+d-'a');
        }
        
        if(negative && val2 > 0) val2 *= -1;
        
        char temp[60];
        sprintf(temp, "%ld", val2);
        intpart = temp;
        fpart = "";
        break;
    }
    
    case TYPE_BINARY:
    {
        //trim off the 'b'
        f = f.substr(0,f.size()-1);
        long val2=0;
        
        for(unsigned int i=0; i<f.size(); i++)
        {
            char b = f.at(i);
            val2<<=1;
            val2+=b-'0';
        }
        
        if(negative && val2 > 0) val2 *= -1;
        
        char temp[60];
        sprintf(temp, "%ld", val2);
        intpart = temp;
        fpart = "";
        break;
    }
    }
    
    return pair<string,string>(intpart, fpart);
}


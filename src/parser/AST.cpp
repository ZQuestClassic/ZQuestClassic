
#include "../precompiled.h" //always first
#include "AST.h"

////////////////////////////////////////////////////////////////

// ASTProgram

ASTProgram::~ASTProgram() {delete decls;}

ASTProgram* ASTProgram::clone() const
{
	return new ASTProgram(
			decls != NULL ? decls->clone() : NULL,
			getLocation());
}

// ASTFloat

ASTFloat::ASTFloat(char *Value, int Type, LocationData Loc)
		: AST(Loc), type(Type), negative(false), val((string)Value)
{}

ASTFloat::ASTFloat(const char *Value, int Type, LocationData Loc)
		: AST(Loc), type(Type), negative(false), val((string)Value)
{}

ASTFloat::ASTFloat(string Value, int Type, LocationData Loc)
		: AST(Loc), type(Type), negative(false), val(Value)
{}

ASTFloat::ASTFloat(long Value, int Type, LocationData Loc)
		: AST(Loc), type(Type), negative(false)
{
	char tmp[15];
	sprintf(tmp, "%ld", Value);
	val = string(tmp);
}

ASTFloat* ASTFloat::clone() const
{
	ASTFloat* c = new ASTFloat(val, type, getLocation());
	c->set_negative(negative);
	return c;
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
        // Trim off the "0x".
        f = f.substr(2,f.size()-2);
        // Parse the hex.
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

// ASTString

ASTString::ASTString(const char *strval, LocationData Loc)
		: AST(Loc), str((string)strval)
{}

ASTString::ASTString(string Str, LocationData Loc)
		: AST(Loc), str(Str)
{}

ASTString* ASTString::clone() const
{
	return new ASTString(str, getLocation());
}

////////////////////////////////////////////////////////////////
// Statements

// ASTStmt

// ASTBlock

ASTBlock::~ASTBlock()
{
    list<ASTStmt *>::iterator it;

    for (it = statements.begin(); it != statements.end(); it++)
    {
        delete *it;
    }

    statements.clear();
}

ASTBlock* ASTBlock::clone() const
{
	ASTBlock* c = new ASTBlock(getLocation());
	for (list<ASTStmt*>::const_iterator it = statements.begin(); it != statements.end(); ++it)
		c->statements.push_back((*it)->clone());
	for (list<long>::const_iterator it = arrayRefs.begin(); it != arrayRefs.end(); ++it)
		c->getArrayRefs()->push_back(*it);
	return c;
}

void ASTBlock::addStatement(ASTStmt *stmt)
{
    statements.push_back(stmt);
}

// ASTStmtAssign

ASTStmtAssign::~ASTStmtAssign()
{
	delete lval;
	delete rval;
}

ASTStmtAssign* ASTStmtAssign::clone() const
{
	return new ASTStmtAssign(
			lval != NULL ? lval->clone() : NULL,
			rval != NULL ? rval->clone() : NULL,
			getLocation());
}

// ASTStmtIf

ASTStmtIf::~ASTStmtIf()
{
	delete cond;
	delete stmt;
}

ASTStmtIf* ASTStmtIf::clone() const
{
	return new ASTStmtIf(
			cond != NULL ? cond->clone() : NULL,
			stmt != NULL ? stmt->clone() : NULL,
			getLocation());
}

// ASTStmtIfElse

ASTStmtIfElse::ASTStmtIfElse(ASTExpr *Cond, ASTStmt *Ifstmt, ASTStmt *Elsestmt, LocationData Loc)
		: ASTStmtIf(Cond, Ifstmt, Loc), elsestmt(Elsestmt)
{}

ASTStmtIfElse::~ASTStmtIfElse()
{
	delete elsestmt;
}

ASTStmtIfElse* ASTStmtIfElse::clone() const
{
	return new ASTStmtIfElse(
			getCondition() != NULL ? getCondition()->clone() : NULL,
			getStmt() != NULL ? getStmt()->clone() : NULL,
			elsestmt != NULL ? elsestmt->clone() : NULL,
			getLocation());
}

// ASTStmtSwitch

ASTStmtSwitch::~ASTStmtSwitch()
{
	delete key;
	for (vector<ASTSwitchCases*>::iterator it = cases.begin(); it != cases.end(); ++it)
		delete *it;
}

ASTStmtSwitch* ASTStmtSwitch::clone() const
{
	ASTStmtSwitch* c = new ASTStmtSwitch(getLocation());
	for (vector<ASTSwitchCases*>::const_iterator it = cases.begin(); it != cases.end(); ++it)
		c->addCases((*it)->clone());
	return c;
}

// ASTSwitchCases

ASTSwitchCases::~ASTSwitchCases()
{
	for (vector<ASTExprConst*>::iterator it = cases.begin(); it != cases.end(); ++it)
		delete *it;
	delete block;
}

ASTSwitchCases* ASTSwitchCases::clone() const
{
	ASTSwitchCases* c = new ASTSwitchCases(getLocation());
	for (vector<ASTExprConst*>::const_iterator it = cases.begin(); it != cases.end(); ++it)
		c->addCase((*it)->clone());
	if (isDefault)
		c->addDefaultCase();
	c->setBlock(block->clone());
	return c;
}

// ASTStmtFor

ASTStmtFor::ASTStmtFor(ASTStmt *Prec, ASTExpr *Term, ASTStmt *Incr, ASTStmt *Stmt, LocationData Loc)
		: ASTStmt(Loc), prec(Prec), term(Term), incr(Incr), stmt(Stmt)
{}

ASTStmtFor::~ASTStmtFor()
{
	delete prec;
	delete term;
	delete incr;
	delete stmt;
}

ASTStmtFor* ASTStmtFor::clone() const
{
	return new ASTStmtFor(
			prec != NULL ? prec->clone() : NULL,
			term != NULL ? term->clone() : NULL,
			incr != NULL ? incr->clone() : NULL,
			stmt != NULL ? stmt->clone() : NULL,
			getLocation());
}

// ASTStmtWhile

ASTStmtWhile::ASTStmtWhile(ASTExpr *Cond, ASTStmt *Stmt, LocationData Loc)
		: ASTStmt(Loc), cond(Cond), stmt(Stmt)
{}

ASTStmtWhile::~ASTStmtWhile()
{
	delete cond;
	delete stmt;
}

ASTStmtWhile* ASTStmtWhile::clone() const
{
	return new ASTStmtWhile(
			cond != NULL ? cond->clone() : NULL,
			stmt != NULL ? stmt->clone() : NULL,
			getLocation());
}

// ASTStmtDo

ASTStmtDo::ASTStmtDo(ASTExpr *Cond, ASTStmt *Stmt, LocationData Loc)
		: ASTStmt(Loc), cond(Cond), stmt(Stmt)
{}

ASTStmtDo::~ASTStmtDo()
{
	delete cond;
	delete stmt;
}

ASTStmtDo* ASTStmtDo::clone() const
{
	return new ASTStmtDo(
			cond != NULL ? cond->clone() : NULL,
			stmt != NULL ? stmt->clone() : NULL,
			getLocation());
}

// ASTStmtReturn

ASTStmtReturn* ASTStmtReturn::clone() const
{
	return new ASTStmtReturn(getLocation());
}

// ASTStmtReturnVal

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

ASTFuncId::~ASTFuncId()
{
	list<ASTType *>::iterator it;
	for (it = params.begin(); it != params.end(); it++)
	{
		delete *it;
	}

	params.clear();
}

void ASTFuncId::addParam(ASTType *param)
{
	params.push_front(param);
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

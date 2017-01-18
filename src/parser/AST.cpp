#include "../precompiled.h" //always first //2.53 Updated to 16th Jan, 2017
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

ASTStmtReturnVal::~ASTStmtReturnVal()
{
	delete retval;
}

ASTStmtReturnVal* ASTStmtReturnVal::clone() const
{
	return new ASTStmtReturnVal(
			retval != NULL ? retval->clone() : NULL,
			getLocation());
}

// ASTStmtBreak

ASTStmtBreak* ASTStmtBreak::clone() const
{
	return new ASTStmtBreak(getLocation());
}

// ASTStmtContinue

ASTStmtContinue* ASTStmtContinue::clone() const
{
	return new ASTStmtContinue(getLocation());
}

// ASTStmtEmpty

ASTStmtEmpty* ASTStmtEmpty::clone() const
{
	return new ASTStmtEmpty(getLocation());
}

////////////////////////////////////////////////////////////////
// Declarations

// ASTDecl

// ASTScript

ASTScript::ASTScript(ASTScriptType *Type, string Name, ASTDeclList *Sblock, LocationData Loc)
		: ASTDecl(Loc), type(Type), name(Name), sblock(Sblock)
{}

ASTScript::~ASTScript()
{
    delete sblock;
    delete type;
}

ASTScript* ASTScript::clone() const
{
	return new ASTScript(
			type != NULL ? type->clone() : NULL,
			name,
			sblock != NULL ? sblock->clone() : NULL,
			getLocation());
}

// ASTDeclList

ASTDeclList::~ASTDeclList()
{
    list<ASTDecl *>::iterator it;

    for(it = decls.begin(); it != decls.end(); it++)
    {
        delete *it;
}

    decls.clear();
}

ASTDeclList* ASTDeclList::clone() const
{
	ASTDeclList* c = new ASTDeclList(getLocation());
	for (list<ASTDecl*>::const_iterator it = decls.begin(); it != decls.end(); ++it)
		c->decls.push_back((*it)->clone());
	return c;
}

void ASTDeclList::addDeclaration(ASTDecl *newdecl)
{
    decls.push_front(newdecl);
}

// ASTImportDecl

ASTImportDecl* ASTImportDecl::clone() const
{
	return new ASTImportDecl(filename, getLocation());
}

// ASTConstDecl

ASTConstDecl* ASTConstDecl::clone() const
{
	return new ASTConstDecl(name, val->clone(), getLocation());
}

// ASTFuncDecl

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

ASTFuncDecl* ASTFuncDecl::clone() const
{
	ASTFuncDecl* c = new ASTFuncDecl(getLocation());
	c->setName(name);
	for (list<ASTVarDecl*>::const_iterator it = params.begin(); it != params.end(); ++it)
		c->params.push_back((*it)->clone());
	c->setReturnType(rettype->clone());
	c->setBlock(block->clone());
	return c;
}

void ASTFuncDecl::addParam(ASTVarDecl *param)
{
    params.push_front(param);
}

// ASTArrayDecl

ASTArrayDecl::ASTArrayDecl(ASTType *Type, string Name, ASTExpr *Size, ASTArrayList *List, LocationData Loc)
		: ASTDecl(Loc), name(Name), list(List), size(Size), type(Type)
{}

ASTArrayDecl::~ASTArrayDecl()
{
    delete type;
    delete list;
    delete size;
}

ASTArrayDecl* ASTArrayDecl::clone() const
{
	ASTArrayList* c_list = NULL;
	if (list != NULL) c_list = list->clone();
	return new ASTArrayDecl(type->clone(), name, size->clone(), c_list, getLocation());
}

// ASTArrayList

ASTArrayList::~ASTArrayList()
{
    list<ASTExpr *>::iterator it;

    for(it = exprs.begin(); it != exprs.end(); it++)
    {
        delete *it;
    }

    exprs.clear();
}

ASTArrayList* ASTArrayList::clone() const
{
	ASTArrayList* c = new ASTArrayList(getLocation());
	if (listIsString) c->makeString();
	for (list<ASTExpr*>::const_iterator it = exprs.begin(); it != exprs.end(); ++it)
		c->exprs.push_back((*it)->clone());
	return c;
}

void ASTArrayList::addParam(ASTExpr *expr)
{
    exprs.push_back(expr);
}

// ASTVarDecl

ASTVarDecl::~ASTVarDecl()
{
    delete type;
}

ASTVarDecl* ASTVarDecl::clone() const
{
	return new ASTVarDecl(type->clone(), name, getLocation());
}

// ASTVarDeclInitializer

ASTVarDeclInitializer::ASTVarDeclInitializer(ASTType *Type, string Name, ASTExpr *Initial, LocationData Loc)
		: ASTVarDecl(Type,Name,Loc), initial(Initial)
{}

ASTVarDeclInitializer::~ASTVarDeclInitializer()
{
    delete initial;
}

ASTVarDeclInitializer* ASTVarDeclInitializer::clone() const
{
	return new ASTVarDeclInitializer(getType()->clone(), getName(), initial->clone(), getLocation());
}

////////////////////////////////////////////////////////////////
// Expressions

// ASTExpr

void ASTExpr::setIntValue(long val)
{
	hasval = true;
	intval=val;
}

// ASTExprConst

// ASTNumConstant

ASTNumConstant* ASTNumConstant::clone() const
{
	ASTNumConstant* c = new ASTNumConstant(val->clone(), getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	return c;
}

// ASTBoolConstant

ASTBoolConstant* ASTBoolConstant::clone() const
{
	ASTBoolConstant* c = new ASTBoolConstant(value, getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	return c;
}

// ASTExprDot

ASTExprDot* ASTExprDot::clone() const
{
	ASTExprDot* c = new ASTExprDot(nspace, name, getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	if (isConstant()) {c->markConstant();}
	return c;
}

// ASTExprArrow

ASTExprArrow::~ASTExprArrow()
{
	delete lval;
	delete index;
}

ASTExprArrow* ASTExprArrow::clone() const
{
	ASTExpr* c_lval = lval != NULL ? lval->clone() : NULL;
	ASTExprArrow* c = new ASTExprArrow(c_lval, rval, getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	c->setIndex(index != NULL ? index->clone() : NULL);
	return c;
}

// ASTExprArray

ASTExprArray* ASTExprArray::clone() const
{
	ASTExprArray* c = new ASTExprArray(nspace, name, getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	c->setIndex(index != NULL ? index->clone() : NULL);
	return c;
}

// ASTFuncCall

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

ASTFuncCall* ASTFuncCall::clone() const
{
	ASTFuncCall* c = new ASTFuncCall(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	c->setName(name != NULL ? name->clone() : NULL);
	for (list<ASTExpr*>::const_iterator it = params.begin(); it != params.end(); ++it)
		c->params.push_back((*it)->clone());
	return c;
}

// ASTUnaryExpr

// ASTExprNegate

ASTExprNegate* ASTExprNegate::clone() const
{
	ASTExprNegate* c = new ASTExprNegate(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprNot

ASTExprNot* ASTExprNot::clone() const
{
	ASTExprNot* c = new ASTExprNot(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprBitNot

ASTExprBitNot* ASTExprBitNot::clone() const
{
	ASTExprBitNot* c = new ASTExprBitNot(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprIncrement

ASTExprIncrement* ASTExprIncrement::clone() const
{
	ASTExprIncrement* c = new ASTExprIncrement(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprPreIncrement

ASTExprPreIncrement* ASTExprPreIncrement::clone() const
{
	ASTExprPreIncrement* c = new ASTExprPreIncrement(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprDecrement

ASTExprDecrement* ASTExprDecrement::clone() const
{
	ASTExprDecrement* c = new ASTExprDecrement(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprPreDecrement

ASTExprPreDecrement* ASTExprPreDecrement::clone() const
{
	ASTExprPreDecrement* c = new ASTExprPreDecrement(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTBinaryExpr

ASTBinaryExpr::~ASTBinaryExpr()
{
	delete first;
	delete second;
}

// ASTLogExpr

// ASTExprAnd

ASTExprAnd* ASTExprAnd::clone() const
{
	ASTExprAnd* c = new ASTExprAnd(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprOr

ASTExprOr* ASTExprOr::clone() const
{
	ASTExprOr* c = new ASTExprOr(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTRelExpr

// ASTExprGT

ASTExprGT* ASTExprGT::clone() const
{
	ASTExprGT* c = new ASTExprGT(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprGE

ASTExprGE* ASTExprGE::clone() const
{
	ASTExprGE* c = new ASTExprGE(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}
    
// ASTExprLT

ASTExprLT* ASTExprLT::clone() const
    {
	ASTExprLT* c = new ASTExprLT(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
    }
    
// ASTExprLE

ASTExprLE* ASTExprLE::clone() const
{
	ASTExprLE* c = new ASTExprLE(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprEQ

ASTExprEQ* ASTExprEQ::clone() const
{
	ASTExprEQ* c = new ASTExprEQ(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprNE

ASTExprNE* ASTExprNE::clone() const
{
	ASTExprNE* c = new ASTExprNE(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTAddExpr

// ASTExprPlus

ASTExprPlus* ASTExprPlus::clone() const
{
	ASTExprPlus* c = new ASTExprPlus(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprMinus

ASTExprMinus* ASTExprMinus::clone() const
{
	ASTExprMinus* c = new ASTExprMinus(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTMultExpr

// ASTExprTimes

ASTExprTimes* ASTExprTimes::clone() const
{
	ASTExprTimes* c = new ASTExprTimes(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprDivide

ASTExprDivide* ASTExprDivide::clone() const
{
	ASTExprDivide* c = new ASTExprDivide(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprModulo
    
ASTExprModulo* ASTExprModulo::clone() const
    {
	ASTExprModulo* c = new ASTExprModulo(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
    }
    
// ASTBitExpr

// ASTExprBitAnd

ASTExprBitAnd* ASTExprBitAnd::clone() const
{
	ASTExprBitAnd* c = new ASTExprBitAnd(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprBitOr

ASTExprBitOr* ASTExprBitOr::clone() const
{
	ASTExprBitOr* c = new ASTExprBitOr(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprBitXor

ASTExprBitXor* ASTExprBitXor::clone() const
{
	ASTExprBitXor* c = new ASTExprBitXor(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTShiftExpr

// ASTExprLShift

ASTExprLShift* ASTExprLShift::clone() const
{
	ASTExprLShift* c = new ASTExprLShift(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

// ASTExprRShift

ASTExprRShift* ASTExprRShift::clone() const
{
	ASTExprRShift* c = new ASTExprRShift(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getFirstOperand();
	c->setFirstOperand(operand != NULL ? operand->clone() : NULL);
	operand = getSecondOperand();
	c->setSecondOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

////////////////////////////////////////////////////////////////
// Types

// ASTScriptType

// ASTType

// ASTTypeVoid

ASTTypeVoid* ASTTypeVoid::clone() const
{
	return new ASTTypeVoid(getLocation());
}

// ASTTypeGlobal

ASTTypeGlobal* ASTTypeGlobal::clone() const
{
	return new ASTTypeGlobal(getLocation());
}

// ASTTypeFloat

ASTTypeFloat* ASTTypeFloat::clone() const
{
	return new ASTTypeFloat(getLocation());
}

// ASTTypeBool
        
ASTTypeBool* ASTTypeBool::clone() const
            {
	return new ASTTypeBool(getLocation());
        }
        
// ASTTypeFFC

ASTTypeFFC* ASTTypeFFC::clone() const
        {
	return new ASTTypeFFC(getLocation());
        }
        
// ASTTypeItem
        
ASTTypeItem* ASTTypeItem::clone() const
{
	return new ASTTypeItem(getLocation());
    }
    
// ASTTypeItemclass
        
ASTTypeItemclass* ASTTypeItemclass::clone() const
        {
	return new ASTTypeItemclass(getLocation());
        }
        
// ASTTypeNPC
        
ASTTypeNPC* ASTTypeNPC::clone() const
{
	return new ASTTypeNPC(getLocation());
    }
    
// ASTTypeLWpn
        
ASTTypeLWpn* ASTTypeLWpn::clone() const
        {
	return new ASTTypeLWpn(getLocation());
        }
        
// ASTTypeEWpn
    
ASTTypeEWpn* ASTTypeEWpn::clone() const
{
	return new ASTTypeEWpn(getLocation());
}

// ASTTypeNPCData

ASTTypeNPCData* ASTTypeNPCData::clone() const
{
	return new ASTTypeNPCData(getLocation());
}

// ASTTypeMapData

ASTTypeMapData* ASTTypeMapData::clone() const
{
	return new ASTTypeMapData(getLocation());
}

// ASTTypeDebug

ASTTypeDebug* ASTTypeDebug::clone() const
{
	return new ASTTypeDebug(getLocation());
}

// ASTTypeAudio

ASTTypeAudio* ASTTypeAudio::clone() const
{
	return new ASTTypeAudio(getLocation());
}

// ASTTypeComboData

ASTTypeComboData* ASTTypeComboData::clone() const
{
	return new ASTTypeComboData(getLocation());
}

// ASTTypeSpriteData

ASTTypeSpriteData* ASTTypeSpriteData::clone() const
{
	return new ASTTypeSpriteData(getLocation());
}

// ASTTypeGraphics

ASTTypeGraphics* ASTTypeGraphics::clone() const
{
	return new ASTTypeGraphics(getLocation());
}

// ASTTypeText

ASTTypeText* ASTTypeText::clone() const
{
	return new ASTTypeText(getLocation());
}

// ASTTypeInput

ASTTypeInput* ASTTypeInput::clone() const
{
	return new ASTTypeInput(getLocation());
}



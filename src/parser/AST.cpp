#include "../precompiled.h" //always first //2.53 Updated to 16th Jan, 2017
#include "AST.h"

////////////////////////////////////////////////////////////////

// ASTProgram

ASTProgram::ASTProgram(LocationData const& location) : AST(location) {}

ASTProgram::ASTProgram(ASTProgram const& base) : AST(base.getLocation())
{
	for (vector<ASTImportDecl*>::const_iterator it = base.imports.begin(); it != base.imports.end(); ++it)
		imports.push_back((*it)->clone());
	for (vector<ASTVarDecl*>::const_iterator it = base.variables.begin(); it != base.variables.end(); ++it)
		variables.push_back((*it)->clone());
	for (vector<ASTArrayDecl*>::const_iterator it = base.arrays.begin(); it != base.arrays.end(); ++it)
		arrays.push_back((*it)->clone());
	for (vector<ASTFuncDecl*>::const_iterator it = base.functions.begin(); it != base.functions.end(); ++it)
		functions.push_back((*it)->clone());
	for (vector<ASTTypeDef*>::const_iterator it = base.types.begin(); it != base.types.end(); ++it)
		types.push_back((*it)->clone());
	for (vector<ASTScript*>::const_iterator it = base.scripts.begin(); it != base.scripts.end(); ++it)
		scripts.push_back((*it)->clone());
}

ASTProgram::~ASTProgram()
{
	for (vector<ASTImportDecl*>::const_iterator it = imports.begin(); it != imports.end(); ++it)
		delete *it;
	for (vector<ASTVarDecl*>::const_iterator it = variables.begin(); it != variables.end(); ++it)
		delete *it;
	for (vector<ASTArrayDecl*>::const_iterator it = arrays.begin(); it != arrays.end(); ++it)
		delete *it;
	for (vector<ASTFuncDecl*>::const_iterator it = functions.begin(); it != functions.end(); ++it)
		delete *it;
	for (vector<ASTTypeDef*>::const_iterator it = types.begin(); it != types.end(); ++it)
		delete *it;
	for (vector<ASTScript*>::const_iterator it = scripts.begin(); it != scripts.end(); ++it)
		delete *it;
}

void ASTProgram::addDeclaration(ASTDecl* declaration)
{
	switch (declaration->declarationClassId())
	{
	case ASTDECL_CLASSID_SCRIPT:
		scripts.push_back((ASTScript*)declaration);
		break;
	case ASTDECL_CLASSID_IMPORT:
		imports.push_back((ASTImportDecl*)declaration);
		break;
	case ASTDECL_CLASSID_FUNCTION:
		functions.push_back((ASTFuncDecl*)declaration);
		break;
	case ASTDECL_CLASSID_ARRAY:
		arrays.push_back((ASTArrayDecl*)declaration);
		break;
	case ASTDECL_CLASSID_VARIABLE:
		variables.push_back((ASTVarDecl*)declaration);
		break;
	case ASTDECL_CLASSID_TYPE:
		types.push_back((ASTTypeDef*)declaration);
		break;
	}
}

ASTProgram& ASTProgram::merge(ASTProgram& other)
{
	for (vector<ASTImportDecl*>::const_iterator it = other.imports.begin(); it != other.imports.end(); ++it)
		imports.push_back(*it);
	other.imports.clear();
	for (vector<ASTVarDecl*>::const_iterator it = other.variables.begin(); it != other.variables.end(); ++it)
		variables.push_back(*it);
	other.variables.clear();
	for (vector<ASTArrayDecl*>::const_iterator it = other.arrays.begin(); it != other.arrays.end(); ++it)
		arrays.push_back(*it);
	other.arrays.clear();
	for (vector<ASTFuncDecl*>::const_iterator it = other.functions.begin(); it != other.functions.end(); ++it)
		functions.push_back(*it);
	other.functions.clear();
	for (vector<ASTTypeDef*>::const_iterator it = other.types.begin(); it != other.types.end(); ++it)
		types.push_back(*it);
	other.types.clear();
	for (vector<ASTScript*>::const_iterator it = other.scripts.begin(); it != other.scripts.end(); ++it)
		scripts.push_back(*it);
	other.scripts.clear();
	return *this;
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

ASTArrayDecl::ASTArrayDecl(ASTVarType *Type, string Name, ASTExpr *Size, ASTArrayList *List, LocationData Loc)
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

void ASTArrayList::addString(string const & str)
{
	LocationData & loc = getLocation();
	for (unsigned int i = 1; i < str.length() - 1; ++i)
		this->addParam(new ASTNumConstant(new ASTFloat(long(str[i]), 0, loc), loc));
	this->addParam(new ASTNumConstant(new ASTFloat(0L, 0, loc), loc));
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

ASTVarDeclInitializer::ASTVarDeclInitializer(ASTVarType *Type, string Name, ASTExpr *Initial, LocationData Loc)
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

// ASTTypeDef

ASTTypeDef::~ASTTypeDef()
{
	delete type;
}

ASTTypeDef* ASTTypeDef::clone() const
{
	return new ASTTypeDef(type->clone(), name, getLocation());
}

////////////////////////////////////////////////////////////////
// Expressions

// ASTExpr

void ASTExpr::setIntValue(long val)
{
	hasval = true;
	intval = val;
}

void ASTExpr::setIntValue(bool val)
{
	hasval = true;
	intval = val ? 1 : 0;
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

// ASTStringConstant

ASTStringConstant::ASTStringConstant(ASTString const & raw)
		: ASTExpr(raw.getLocation()), str(raw.getValue().substr(1, raw.getValue().size() - 2))
{}

ASTStringConstant* ASTStringConstant::clone() const
{
	ASTStringConstant* c = new ASTStringConstant(str, getLocation());
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

// ASTVarType

ZVarType const& ASTVarType::resolve(Scope& scope)
{
	ZVarType* resolved = type->resolve(scope);
	if (type != resolved)
	{
		delete type;
		type = resolved;
	}
	return *type;
}

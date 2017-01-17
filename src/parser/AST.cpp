#include "../precompiled.h" //always first
#include "AST.h"


ASTProgram::~ASTProgram()
{
    delete decls;
}

ASTProgram* ASTProgram::clone() const
{
	return new ASTProgram(
			decls != NULL ? decls->clone() : NULL,
			getLocation());
}

ASTString* ASTString::clone() const
{
	return new ASTString(str, getLocation());
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

ASTImportDecl* ASTImportDecl::clone() const
{
	return new ASTImportDecl(filename, getLocation());
}

ASTConstDecl* ASTConstDecl::clone() const
{
	return new ASTConstDecl(name, val->clone(), getLocation());
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
	return new ASTArrayDecl(type->clone(), name, size->clone(), reg, c_list, getLocation());
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

ASTTypeFloat* ASTTypeFloat::clone() const
{
	return new ASTTypeFloat(getLocation());
}

ASTTypeBool* ASTTypeBool::clone() const
{
	return new ASTTypeBool(getLocation());
}

ASTTypeVoid* ASTTypeVoid::clone() const
{
	return new ASTTypeVoid(getLocation());
}

ASTTypeFFC* ASTTypeFFC::clone() const
{
	return new ASTTypeFFC(getLocation());
}

ASTTypeGlobal* ASTTypeGlobal::clone() const
{
	return new ASTTypeGlobal(getLocation());
}

ASTTypeItem* ASTTypeItem::clone() const
{
	return new ASTTypeItem(getLocation());
}

ASTTypeItemclass* ASTTypeItemclass::clone() const
{
	return new ASTTypeItemclass(getLocation());
}

ASTTypeNPC* ASTTypeNPC::clone() const
{
	return new ASTTypeNPC(getLocation());
}

ASTTypeLWpn* ASTTypeLWpn::clone() const
{
	return new ASTTypeLWpn(getLocation());
}

ASTTypeEWpn* ASTTypeEWpn::clone() const
{
	return new ASTTypeEWpn(getLocation());
}

ASTVarDecl::~ASTVarDecl()
{
    delete type;
}

ASTVarDecl* ASTVarDecl::clone() const
{
	return new ASTVarDecl(type->clone(), name, getLocation());
}

ASTVarDeclInitializer::~ASTVarDeclInitializer()
{
    delete initial;
}

ASTVarDeclInitializer* ASTVarDeclInitializer::clone() const
{
	return new ASTVarDeclInitializer(getType()->clone(), getName(), initial->clone(), getLocation());
}

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

ASTExprNegate* ASTExprNegate::clone() const
{
	ASTExprNegate* c = new ASTExprNegate(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

ASTExprNot* ASTExprNot::clone() const
{
	ASTExprNot* c = new ASTExprNot(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

ASTExprBitNot* ASTExprBitNot::clone() const
{
	ASTExprBitNot* c = new ASTExprBitNot(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

ASTExprIncrement* ASTExprIncrement::clone() const
{
	ASTExprIncrement* c = new ASTExprIncrement(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

ASTExprPreIncrement* ASTExprPreIncrement::clone() const
{
	ASTExprPreIncrement* c = new ASTExprPreIncrement(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

ASTExprDecrement* ASTExprDecrement::clone() const
{
	ASTExprDecrement* c = new ASTExprDecrement(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

ASTExprPreDecrement* ASTExprPreDecrement::clone() const
{
	ASTExprPreDecrement* c = new ASTExprPreDecrement(getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	ASTExpr* operand = getOperand();
	c->setOperand(operand != NULL ? operand->clone() : NULL);
	return c;
}

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

ASTNumConstant* ASTNumConstant::clone() const
{
	ASTNumConstant* c = new ASTNumConstant(val->clone(), getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	return c;
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

ASTBoolConstant* ASTBoolConstant::clone() const
{
	ASTBoolConstant* c = new ASTBoolConstant(value, getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	return c;
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

ASTBlock::~ASTBlock()
{
    list<ASTStmt *>::iterator it;
    
    for(it=statements.begin(); it != statements.end(); it++)
    {
        delete *it;
    }
    
    statements.clear();
}

ASTStmtAssign* ASTStmtAssign::clone() const
{
	return new ASTStmtAssign(
			lval != NULL ? lval->clone() : NULL,
			rval != NULL ? rval->clone() : NULL,
			getLocation());
}

ASTExprDot* ASTExprDot::clone() const
{
	ASTExprDot* c = new ASTExprDot(nspace, name, getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	return c;
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

ASTExprArray* ASTExprArray::clone() const
{
	ASTExprArray* c = new ASTExprArray(nspace, name, getLocation());
	if (hasIntValue()) c->setIntValue(getIntValue());
	c->setType(getType());
	c->setIndex(index != NULL ? index->clone() : NULL);
	return c;
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

ASTStmtIf* ASTStmtIf::clone() const
{
	return new ASTStmtIf(
			cond != NULL ? cond->clone() : NULL,
			stmt != NULL ? stmt->clone() : NULL,
			getLocation());
}

ASTStmtIfElse* ASTStmtIfElse::clone() const
{
	return new ASTStmtIfElse(
			getCondition() != NULL ? getCondition()->clone() : NULL,
			getStmt() != NULL ? getStmt()->clone() : NULL,
			elsestmt != NULL ? elsestmt->clone() : NULL,
			getLocation());
}

ASTStmtReturn* ASTStmtReturn::clone() const
{
	return new ASTStmtReturn(getLocation());
}

ASTStmtReturnVal* ASTStmtReturnVal::clone() const
{
	return new ASTStmtReturnVal(
			retval != NULL ? retval->clone() : NULL,
			getLocation());
}

ASTStmtBreak* ASTStmtBreak::clone() const
{
	return new ASTStmtBreak(getLocation());
}

ASTStmtContinue* ASTStmtContinue::clone() const
{
	return new ASTStmtContinue(getLocation());
}

ASTStmtEmpty* ASTStmtEmpty::clone() const
{
	return new ASTStmtEmpty(getLocation());
}

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

ASTStmtWhile* ASTStmtWhile::clone() const
{
	return new ASTStmtWhile(
			cond != NULL ? cond->clone() : NULL,
			stmt != NULL ? stmt->clone() : NULL,
			getLocation());
}

ASTStmtDo* ASTStmtDo::clone() const
{
	return new ASTStmtDo(
			cond != NULL ? cond->clone() : NULL,
			stmt != NULL ? stmt->clone() : NULL,
			getLocation());
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


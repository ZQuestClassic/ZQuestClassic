//2.53 Updated to 16th Jan, 2017
#include "../precompiled.h" //always first

#include "UtilVisitors.h"
#include <assert.h>
#include "../zsyssimple.h"
#include "ParseError.h"

////////////////////////////////////////////////////////////////
// RecursiveVisitor

void RecursiveVisitor::caseProgram(ASTProgram& host, void* param)
{
	for (vector<ASTImportDecl*>::const_iterator it = host.imports.begin();
		 it != host.imports.end(); ++it)
		(*it)->execute(*this, param);
	for (vector<ASTTypeDef*>::const_iterator it = host.types.begin();
		 it != host.types.end(); ++it)
		(*it)->execute(*this, param);
	for (vector<ASTDataDeclList*>::const_iterator it = host.variables.begin();
		 it != host.variables.end(); ++it)
		(*it)->execute(*this, param);
	for (vector<ASTFuncDecl*>::const_iterator it = host.functions.begin();
		 it != host.functions.end(); ++it)
		(*it)->execute(*this, param);
	for (vector<ASTScript*>::const_iterator it = host.scripts.begin();
		 it != host.scripts.end(); ++it)
		(*it)->execute(*this, param);
}

void RecursiveVisitor::caseProgram(ASTProgram &host)
{
	for (vector<ASTImportDecl*>::const_iterator it = host.imports.begin();
		 it != host.imports.end(); ++it)
		(*it)->execute(*this);
	for (vector<ASTTypeDef*>::const_iterator it = host.types.begin();
		 it != host.types.end(); ++it)
		(*it)->execute(*this);
	for (vector<ASTDataDeclList*>::const_iterator it = host.variables.begin();
		 it != host.variables.end(); ++it)
		(*it)->execute(*this);
	for (vector<ASTFuncDecl*>::const_iterator it = host.functions.begin();
		 it != host.functions.end(); ++it)
		(*it)->execute(*this);
	for (vector<ASTScript*>::const_iterator it = host.scripts.begin();
		 it != host.scripts.end(); ++it)
		(*it)->execute(*this);
}

// Statements

void RecursiveVisitor::caseBlock(ASTBlock &host, void *param)
{
	list<ASTStmt *> l = host.getStatements();

	for(list<ASTStmt *>::iterator it = l.begin(); it != l.end(); it++)
		(*it)->execute(*this,param);
}

void RecursiveVisitor::caseBlock(ASTBlock &host)
{
	list<ASTStmt *> l = host.getStatements();

	for(list<ASTStmt *>::iterator it = l.begin(); it != l.end(); it++)
		(*it)->execute(*this);
}

void RecursiveVisitor::caseStmtIf(ASTStmtIf &host, void *param)
{
	host.getCondition()->execute(*this, param);
	host.getStmt()->execute(*this, param);
}

void RecursiveVisitor::caseStmtIf(ASTStmtIf &host)
{
	host.getCondition()->execute(*this);
	host.getStmt()->execute(*this);
}

void RecursiveVisitor::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
	host.getCondition()->execute(*this, param);
	host.getStmt()->execute(*this, param);
	host.getElseStmt()->execute(*this, param);
}

void RecursiveVisitor::caseStmtIfElse(ASTStmtIfElse &host)
{
	host.getCondition()->execute(*this);
	host.getStmt()->execute(*this);
	host.getElseStmt()->execute(*this);
}

void RecursiveVisitor::caseStmtSwitch(ASTStmtSwitch & host, void* param)
{
	host.getKey()->execute(*this, param);

	vector<ASTSwitchCases*> & cases = host.getCases();
	for (vector<ASTSwitchCases*>::iterator it = cases.begin(); it != cases.end(); ++it)
		(*it)->execute(*this, param);
}

void RecursiveVisitor::caseStmtSwitch(ASTStmtSwitch & host)
{
	host.getKey()->execute(*this);

	vector<ASTSwitchCases*> & cases = host.getCases();
	for (vector<ASTSwitchCases*>::iterator it = cases.begin(); it != cases.end(); ++it)
		(*it)->execute(*this);
}

void RecursiveVisitor::caseSwitchCases(ASTSwitchCases & host, void* param)
{
	vector<ASTExprConst*> & cases = host.getCases();
	for (vector<ASTExprConst*>::iterator it = cases.begin();
		 it != cases.end();
		 ++it)
	{
		(*it)->execute(*this, param);
	}

	host.getBlock()->execute(*this, param);
}

void RecursiveVisitor::caseSwitchCases(ASTSwitchCases & host)
{
	vector<ASTExprConst*> & cases = host.getCases();
	for (vector<ASTExprConst*>::iterator it = cases.begin();
		 it != cases.end();
		 ++it)
	{
		(*it)->execute(*this);
	}

	host.getBlock()->execute(*this);
}

void RecursiveVisitor::caseStmtFor(ASTStmtFor &host, void *param)
{
	host.getPrecondition()->execute(*this, param);
	host.getTerminationCondition()->execute(*this, param);
	host.getIncrement()->execute(*this, param);
	host.getStmt()->execute(*this, param);
}

void RecursiveVisitor::caseStmtFor(ASTStmtFor &host)
{
	host.getPrecondition()->execute(*this);
	host.getIncrement()->execute(*this);
	host.getTerminationCondition()->execute(*this);
	host.getStmt()->execute(*this);
}

void RecursiveVisitor::caseStmtWhile(ASTStmtWhile &host, void *param)
{
	host.getCond()->execute(*this, param);
	host.getStmt()->execute(*this, param);
}

void RecursiveVisitor::caseStmtWhile(ASTStmtWhile &host)
{
	host.getCond()->execute(*this);
	host.getStmt()->execute(*this);
}

void RecursiveVisitor::caseStmtDo(ASTStmtDo &host, void *param)
{
	host.getStmt()->execute(*this, param);
	host.getCond()->execute(*this, param);
}

void RecursiveVisitor::caseStmtDo(ASTStmtDo &host)
{
	host.getStmt()->execute(*this);
	host.getCond()->execute(*this);
}

void RecursiveVisitor::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
	host.getReturnValue()->execute(*this, param);
}

void RecursiveVisitor::caseStmtReturnVal(ASTStmtReturnVal &host)
{
	host.getReturnValue()->execute(*this);
}

// Declarations

void RecursiveVisitor::caseScript(ASTScript &host, void *param)
{
	host.getType()->execute(*this, param);
	for (vector<ASTTypeDef*>::const_iterator it = host.types.begin();
		 it != host.types.end(); ++it)
		(*it)->execute(*this, param);
	for (vector<ASTDataDeclList*>::const_iterator it = host.variables.begin();
		 it != host.variables.end(); ++it)
		(*it)->execute(*this, param);
	for (vector<ASTFuncDecl*>::const_iterator it = host.functions.begin();
		 it != host.functions.end(); ++it)
		(*it)->execute(*this, param);
}

void RecursiveVisitor::caseScript(ASTScript &host)
{
	host.getType()->execute(*this);
	for (vector<ASTTypeDef*>::const_iterator it = host.types.begin();
		 it != host.types.end(); ++it)
		(*it)->execute(*this);
	for (vector<ASTDataDeclList*>::const_iterator it = host.variables.begin();
		 it != host.variables.end(); ++it)
		(*it)->execute(*this);
	for (vector<ASTFuncDecl*>::const_iterator it = host.functions.begin();
		 it != host.functions.end(); ++it)
		(*it)->execute(*this);
}

void RecursiveVisitor::caseFuncDecl(ASTFuncDecl& host, void* param)
{
	if (host.returnType) host.returnType->execute(*this, param);

	for (vector<ASTDataDecl*>::iterator it = host.getParameters().begin();
		 it != host.getParameters().end(); it++)
		(*it)->execute(*this, param);

	if (host.block) host.block->execute(*this, param);
}

void RecursiveVisitor::caseFuncDecl(ASTFuncDecl &host)
{
	if (host.returnType) host.returnType->execute(*this);

	for (vector<ASTDataDecl*>::iterator it = host.getParameters().begin();
		 it != host.getParameters().end(); it++)
		(*it)->execute(*this);

	if (host.block) host.block->execute(*this);
}

void RecursiveVisitor::caseDataDeclList(ASTDataDeclList& host, void* param)
{
	if (host.baseType) host.baseType->execute(*this, param);
	for (vector<ASTDataDecl*>::const_iterator it = host.getDeclarations().begin();
	     it != host.getDeclarations().end(); ++it)
		(*it)->execute(*this, param);
}

void RecursiveVisitor::caseDataDeclList(ASTDataDeclList& host)
{
	if (host.baseType) host.baseType->execute(*this);
	for (vector<ASTDataDecl*>::const_iterator it = host.getDeclarations().begin();
		 it != host.getDeclarations().end(); ++it)
			(*it)->execute(*this);
}

void RecursiveVisitor::caseDataDecl(ASTDataDecl& host, void* param)
{
	if (host.baseType) host.baseType->execute(*this, param);
	for (vector<ASTDataDeclExtraArray*>::iterator it = host.extraArrays.begin();
		 it != host.extraArrays.end(); ++it)
		(*it)->execute(*this, param);
	if (host.getInitializer()) host.getInitializer()->execute(*this, param);
}

void RecursiveVisitor::caseDataDecl(ASTDataDecl& host)
{
	if (host.baseType) host.baseType->execute(*this);
	for (vector<ASTDataDeclExtraArray*>::iterator it = host.extraArrays.begin();
		 it != host.extraArrays.end(); ++it)
		(*it)->execute(*this);
	if (host.getInitializer()) host.getInitializer()->execute(*this);
}

void RecursiveVisitor::caseDataDeclExtraArray(ASTDataDeclExtraArray& host, void* param)
{
	for (vector<ASTExpr*>::iterator it = host.dimensions.begin();
		 it != host.dimensions.end(); ++it)
		(*it)->execute(*this, param);
}

void RecursiveVisitor::caseDataDeclExtraArray(ASTDataDeclExtraArray& host)
{
	for (vector<ASTExpr*>::iterator it = host.dimensions.begin();
		 it != host.dimensions.end(); ++it)
		(*it)->execute(*this);
}

void RecursiveVisitor::caseTypeDef(ASTTypeDef& host, void* param)
{
	host.getType()->execute(*this, param);
}

void RecursiveVisitor::caseTypeDef(ASTTypeDef& host)
{
	host.getType()->execute(*this);
}

// Expressions

void RecursiveVisitor::caseExprConst(ASTExprConst &host, void *param)
{
	host.getContent()->execute(*this, param);
}

void RecursiveVisitor::caseExprConst(ASTExprConst &host)
{
	host.getContent()->execute(*this);
}

void RecursiveVisitor::caseExprAssign(ASTExprAssign &host, void *param)
{
	host.getLVal()->execute(*this, param);
	host.getRVal()->execute(*this, param);
}

void RecursiveVisitor::caseExprAssign(ASTExprAssign &host)
{
	host.getLVal()->execute(*this);
	host.getRVal()->execute(*this);
}

void RecursiveVisitor::caseExprArrow(ASTExprArrow &host, void*param)
{
	host.getLeft()->execute(*this, param);

	if(host.getIndex())
		host.getIndex()->execute(*this, param);
}

void RecursiveVisitor::caseExprArrow(ASTExprArrow &host)
{
	host.getLeft()->execute(*this);

	if(host.getIndex())
		host.getIndex()->execute(*this);
}

void RecursiveVisitor::caseExprIndex(ASTExprIndex &host, void *param)
{
	if (host.getArray()) host.getArray()->execute(*this, param);
	if (host.getIndex()) host.getIndex()->execute(*this, param);
}

void RecursiveVisitor::caseExprIndex(ASTExprIndex &host)
{
	if (host.getArray()) host.getArray()->execute(*this);
	if (host.getIndex()) host.getIndex()->execute(*this);
}

void RecursiveVisitor::caseExprCall(ASTExprCall& host, void *param)
{
	//host.getLeft()->execute(*this, param);

	list<ASTExpr *> l = host.getParams();
	for (list<ASTExpr *>::iterator it = l.begin(); it != l.end(); it++)
	{
		(*it)->execute(*this, param);
	}
}

void RecursiveVisitor::caseExprCall(ASTExprCall &host)
{
	//host.getLeft()->execute(*this);

	list<ASTExpr *> l = host.getParams();
	for (list<ASTExpr *>::iterator it = l.begin(); it != l.end(); it++)
	{
		(*it)->execute(*this);
	}
}

void RecursiveVisitor::caseExprNegate(ASTExprNegate &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprNegate(ASTExprNegate &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprNot(ASTExprNot &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprNot(ASTExprNot &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprBitNot(ASTExprBitNot &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprBitNot(ASTExprBitNot &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprIncrement(ASTExprIncrement &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprIncrement(ASTExprIncrement &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprPreIncrement(ASTExprPreIncrement &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprPreIncrement(ASTExprPreIncrement &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprDecrement(ASTExprDecrement &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprDecrement(ASTExprDecrement &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprPreDecrement(ASTExprPreDecrement &host, void *param)
{
	host.getOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprPreDecrement(ASTExprPreDecrement &host)
{
	host.getOperand()->execute(*this);
}

void RecursiveVisitor::caseExprAnd(ASTExprAnd &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprAnd(ASTExprAnd &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprOr(ASTExprOr &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprOr(ASTExprOr &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprGT(ASTExprGT &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprGT(ASTExprGT &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprGE(ASTExprGE &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprGE(ASTExprGE &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprLT(ASTExprLT &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprLT(ASTExprLT &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprLE(ASTExprLE &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprLE(ASTExprLE &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprEQ(ASTExprEQ &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprEQ(ASTExprEQ &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprNE(ASTExprNE &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprNE(ASTExprNE &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprPlus(ASTExprPlus &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprPlus(ASTExprPlus &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprMinus(ASTExprMinus &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprMinus(ASTExprMinus &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprTimes(ASTExprTimes &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprTimes(ASTExprTimes &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprDivide(ASTExprDivide &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprDivide(ASTExprDivide &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprModulo(ASTExprModulo &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprModulo(ASTExprModulo &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprBitAnd(ASTExprBitAnd &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprBitAnd(ASTExprBitAnd &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprBitOr(ASTExprBitOr &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprBitOr(ASTExprBitOr &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprBitXor(ASTExprBitXor &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprBitXor(ASTExprBitXor &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprLShift(ASTExprLShift &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprLShift(ASTExprLShift &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

void RecursiveVisitor::caseExprRShift(ASTExprRShift &host, void *param)
{
	host.getFirstOperand()->execute(*this, param);
	host.getSecondOperand()->execute(*this, param);
}

void RecursiveVisitor::caseExprRShift(ASTExprRShift &host)
{
	host.getFirstOperand()->execute(*this);
	host.getSecondOperand()->execute(*this);
}

// Literals

void RecursiveVisitor::caseNumberLiteral(ASTNumberLiteral& host, void* param)
{
	host.getValue()->execute(*this, param);
}

void RecursiveVisitor::caseNumberLiteral(ASTNumberLiteral& host)
{
	host.getValue()->execute(*this);
}

void RecursiveVisitor::caseArrayLiteral(ASTArrayLiteral& host, void* param)
{
	ASTVarType* type = host.getType();
	if (type) type->execute(*this, param);
	ASTExpr* size = host.getSize();
	if (size) size->execute(*this, param);
	vector<ASTExpr*> elements = host.getElements();
	for (vector<ASTExpr*>::iterator it = elements.begin();
		 it != elements.end(); ++it)
		(*it)->execute(*this, param);
}

void RecursiveVisitor::caseArrayLiteral(ASTArrayLiteral& host)
{
	ASTVarType* type = host.getType();
	if (type) type->execute(*this);
	ASTExpr* size = host.getSize();
	if (size) size->execute(*this);
	vector<ASTExpr*> elements = host.getElements();
	for (vector<ASTExpr*>::iterator it = elements.begin();
		 it != elements.end(); ++it)
		(*it)->execute(*this);
}

////////////////////////////////////////////////////////////////////////////////
// CheckForExtraneousImports

void CheckForExtraneousImports::caseDefault(void *) {}

void CheckForExtraneousImports::caseImportDecl(ASTImportDecl &host, void *)
{
    ok = false;
    printErrorMsg(&host, IMPORTBADSCOPE);
}


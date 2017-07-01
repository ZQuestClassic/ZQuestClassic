#ifndef BUILDVISITORS_H
#define BUILDVISITORS_H

#include "AST.h"
#include "UtilVisitors.h"
#include "ByteCode.h"
#include <stack>
#include <algorithm>

class BuildOpcodes : public RecursiveVisitor
{
public:
    virtual void caseDefault(void *param);
    virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
    virtual void caseVarDecl(ASTVarDecl &host, void *param);
    virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);
    virtual void caseArrayDecl(ASTArrayDecl &host, void *param);
    virtual void caseBlock(ASTBlock &host, void *param);
    virtual void caseExprAnd(ASTExprAnd &host, void *param);
    virtual void caseExprOr(ASTExprOr &host, void *param);
    virtual void caseExprGT(ASTExprGT &host, void *param);
    virtual void caseExprGE(ASTExprGE &host, void *param);
    virtual void caseExprLT(ASTExprLT &host, void *param);
    virtual void caseExprLE(ASTExprLE &host, void *param);
    virtual void caseExprEQ(ASTExprEQ &host, void *param);
    virtual void caseExprNE(ASTExprNE &host, void *param);
    virtual void caseExprPlus(ASTExprPlus &host, void *param);
    virtual void caseExprMinus(ASTExprMinus &host, void *param);
    virtual void caseExprTimes(ASTExprTimes &host, void *param);
    virtual void caseExprDivide(ASTExprDivide &host, void *param);
    virtual void caseExprNot(ASTExprNot &host, void *param);
    virtual void caseExprNegate(ASTExprNegate &host, void *param);
    virtual void caseFuncCall(ASTFuncCall &host, void *param);
    virtual void caseStmtAssign(ASTStmtAssign &host, void *param);
    virtual void caseExprDot(ASTExprDot &host, void *param);
    virtual void caseExprArrow(ASTExprArrow &host, void *param);
    virtual void caseExprArray(ASTExprArray &host, void *param);
    virtual void caseStmtFor(ASTStmtFor &host, void *param);
    virtual void caseStmtIf(ASTStmtIf &host, void *param);
    virtual void caseStmtIfElse(ASTStmtIfElse &host, void *param);
    virtual void caseStmtReturn(ASTStmtReturn &host, void *param);
    virtual void caseStmtReturnVal(ASTStmtReturnVal &host, void *param);
    virtual void caseStmtEmpty(ASTStmtEmpty &host, void *param);
    virtual void caseNumConstant(ASTNumConstant &host, void *param);
    virtual void caseBoolConstant(ASTBoolConstant &host, void *param);
    virtual void caseStmtWhile(ASTStmtWhile &host, void *param);
    virtual void caseStmtDo(ASTStmtDo &host, void *param);
    virtual void caseExprBitOr(ASTExprBitOr &host, void *param);
    virtual void caseExprBitXor(ASTExprBitXor &host, void *param);
    virtual void caseExprBitAnd(ASTExprBitAnd &host, void *param);
    virtual void caseExprLShift(ASTExprLShift &host, void *param);
    virtual void caseExprRShift(ASTExprRShift &host, void *param);
    virtual void caseExprModulo(ASTExprModulo &host, void *param);
    virtual void caseExprBitNot(ASTExprBitNot &host, void *param);
    virtual void caseExprIncrement(ASTExprIncrement &host, void *param);
    virtual void caseExprPreIncrement(ASTExprPreIncrement &host, void *param);
    virtual void caseExprDecrement(ASTExprDecrement &host, void *param);
    virtual void caseExprPreDecrement(ASTExprPreDecrement &host, void *param);
    virtual void caseStmtBreak(ASTStmtBreak &host, void *param);
    virtual void caseStmtContinue(ASTStmtContinue &host, void *param);
    vector<Opcode *> getResult()
    {
        return result;
    }
    int getReturnLabelID()
    {
        return returnlabelid;
    }
    list<long> *getArrayRefs()
    {
        return &arrayRefs;
    }
    BuildOpcodes() : continuelabelid(-1), breaklabelid(-1), failure(false) {}
    bool isOK()
    {
        return !failure;
    }
    void castFromBool(vector<Opcode *> &result, int reg);
private:
    vector<Opcode *> result;
    int returnlabelid;
    int continuelabelid;
    int breaklabelid;
    list<long> *breakRef;
    list<long> arrayRefs;
    bool failure;
};

class AssignStackSymbols : public RecursiveVisitor
{
public:
	AssignStackSymbols(StackFrame *sf, SymbolTable *st, int baseoffset) : sf(sf), st(st), curoffset(baseoffset), highWaterOffset(baseoffset)
	{
	}

    virtual void caseDefault(void *) { }
    virtual void caseVarDecl(ASTVarDecl &host, void *)
    {

        int vid = st->getID(&host);
		sf->addToFrame(vid, curoffset);
		curoffset += 10000;
		highWaterOffset = std::max(highWaterOffset, curoffset);
    }
    virtual void caseArrayDecl(ASTArrayDecl &host, void *)
    {        
        int vid = st->getID(&host);
		sf->addToFrame(vid, curoffset);
		curoffset += 10000;
		highWaterOffset = std::max(highWaterOffset, curoffset);
    }
    virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
    {
        caseVarDecl(host, param);
    }

	virtual void caseBlock(ASTBlock &host, void *param)
	{
		prevframes.push(curoffset);
		list<ASTStmt *> l = host.getStatements();

		for (list<ASTStmt *>::iterator it = l.begin(); it != l.end(); it++)
			(*it)->execute(*this, param);

		curoffset = prevframes.top();
		prevframes.pop();
	}

	virtual void caseStmtFor(ASTStmtFor &host, void *param)
	{
		prevframes.push(curoffset);

		host.getPrecondition()->execute(*this, param);
		host.getIncrement()->execute(*this, param);
		host.getTerminationCondition()->execute(*this, param);
		host.getStmt()->execute(*this, param);

		curoffset = prevframes.top();
		prevframes.pop();
	}

	int getHighWaterOffset() { return highWaterOffset; }

private:
	StackFrame *sf;
	SymbolTable *st;
	int curoffset;
	int highWaterOffset;
	std::stack<int> prevframes;
};

class LValBOHelper : public ASTVisitor
{
public:
    virtual void caseDefault(void *param);
    virtual void caseExprDot(ASTExprDot &host, void *param);
    virtual void caseExprArrow(ASTExprArrow &host, void *param);
    virtual void caseExprArray(ASTExprArray &host, void *param);
    virtual void caseVarDecl(ASTVarDecl &host, void *param);
    vector<Opcode *> getResult()
    {
        return result;
    }
private:
    vector<Opcode *> result;
};

class GetLabels : public ArgumentVisitor
{
public:
    void caseLabel(LabelArgument &host, void *param)
    {
        map<int,bool> *labels = (map<int,bool> *)param;
        (*labels)[host.getID()] = true;
    }
};

class SetLabels : public ArgumentVisitor
{
public:
    void caseLabel(LabelArgument &host, void *param)
    {
        map<int, int> *labels = (map<int, int> *)param;
        int lineno = (*labels)[host.getID()];
        
        if(lineno==0)
        {
            char temp[200];
            sprintf(temp,"Internal error: couldn't find function label %d", host.getID());
            box_out(temp);
            box_eol();
        }
        
        host.setLineNo(lineno);
    }
};

#endif


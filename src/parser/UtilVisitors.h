#ifndef UTILVISITORS_H //2.53 Updated to 16th Jan, 2017
#define UTILVISITORS_H

#include "AST.h"

class RecursiveVisitor : public ASTVisitor
{
public:
    virtual void caseDefault(void*) {}
    virtual void caseProgram(ASTProgram &host, void *param);
	// Statements
    virtual void caseBlock(ASTBlock &host, void *param);
    virtual void caseStmtAssign(ASTStmtAssign &host, void *param);
    virtual void caseStmtIf(ASTStmtIf &host, void *param);
    virtual void caseStmtIfElse(ASTStmtIfElse &host, void *param);
    virtual void caseStmtFor(ASTStmtFor &host, void *param);
    virtual void caseStmtWhile(ASTStmtWhile &host, void *param);
    virtual void caseStmtDo(ASTStmtDo &host, void *param);
    virtual void caseStmtReturnVal(ASTStmtReturnVal &host, void *param);
	// Declarations
    virtual void caseScript(ASTScript &host, void *param);
    virtual void caseDeclList(ASTDeclList &host, void *param);
    virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
    virtual void caseArrayDecl(ASTArrayDecl &host, void *param);
    virtual void caseVarDecl(ASTVarDecl &host, void *param);
    virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);
	// Expressions
    virtual void caseNumConstant(ASTNumConstant &host, void *param);
    virtual void caseExprArrow(ASTExprArrow &host, void *param);
    virtual void caseExprArray(ASTExprArray &host, void *param);
    virtual void caseFuncCall(ASTFuncCall &host, void *param);
    virtual void caseExprNegate(ASTExprNegate &host, void *param);
    virtual void caseExprNot(ASTExprNot &host, void *param);
    virtual void caseExprBitNot(ASTExprBitNot &host, void *param);
    virtual void caseExprIncrement(ASTExprIncrement &host, void *param);
    virtual void caseExprPreIncrement(ASTExprPreIncrement &host, void *param);
    virtual void caseExprDecrement(ASTExprDecrement &host, void *param);
    virtual void caseExprPreDecrement(ASTExprPreDecrement &host, void *param);
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
    virtual void caseExprModulo(ASTExprModulo &host, void *param);
    virtual void caseExprBitAnd(ASTExprBitAnd &host, void *param);
    virtual void caseExprBitOr(ASTExprBitOr &host, void *param);
    virtual void caseExprBitXor(ASTExprBitXor &host, void *param);
    virtual void caseExprLShift(ASTExprLShift &host, void *param);
    virtual void caseExprRShift(ASTExprRShift &host, void *param);
};

// Removes all import declarations from a program
// Call with param = NULL
//NOT responsible for freeing said imports
class GetImports : public ASTVisitor
{
public:
    GetImports() : result() {}

    void caseDefault(void *param);
    void caseProgram(ASTProgram &host, void *param);
    void caseDeclList(ASTDeclList &host, void *param);
    void caseImportDecl(ASTImportDecl &host, void *param);

    vector<ASTImportDecl *> const &getResult() const {return result;}
    vector<ASTImportDecl *> &getResult() {return result;}
private:
    vector<ASTImportDecl *> result;
};

class GetConsts : public ASTVisitor
{
public:
    GetConsts() : result() {}

    void caseDefault(void *param);
    void caseProgram(ASTProgram &host, void *param);
    void caseDeclList(ASTDeclList &host, void *param);
    void caseConstDecl(ASTConstDecl &host, void *param);

    vector<ASTConstDecl *> const &getResult() const {return result;}
    vector<ASTConstDecl *> &getResult() {return result;}
private:
    vector<ASTConstDecl *> result;
};

class GetGlobalVars : public ASTVisitor
{
public:
    GetGlobalVars() : result() {}

    void caseDefault(void *param);
    void caseProgram(ASTProgram &host, void *param);
    void caseDeclList(ASTDeclList &host, void *param);
    void caseArrayDecl(ASTArrayDecl &host, void *param);
    void caseVarDecl(ASTVarDecl &host, void *param);
    void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);

    vector<ASTVarDecl *> getResult() {return result;}
    vector<ASTArrayDecl *> getResultA() {return resultA;}
private:
    vector<ASTVarDecl *> result;
    vector<ASTArrayDecl *> resultA;
};

class GetGlobalFuncs : public ASTVisitor
{
public:
    GetGlobalFuncs() : result() {}
    virtual void caseDefault(void *param);
    virtual void caseProgram(ASTProgram &host, void *param);
    virtual void caseDeclList(ASTDeclList &host, void *param);
    virtual void caseFuncDecl(ASTFuncDecl &host, void *param);

    vector<ASTFuncDecl *> getResult() {return result;}
private:
    vector<ASTFuncDecl *> result;
};

class GetScripts : public ASTVisitor
{
public:
    GetScripts() : result() {}
    void caseDefault(void *param);
    void caseProgram(ASTProgram &host, void *param);
    void caseDeclList(ASTDeclList &host, void *param);
    void caseScript(ASTScript &host, void *param);

    vector<ASTScript *> getResult() {return result;}
private:
    vector<ASTScript *> result;
};

// Merges one AST into another. Pass the AST that will be destroyed in the
// process as the param.  The param AST gets completely consumed and deleted.
class MergeASTs : public ASTVisitor
{
public:
    void caseDefault(void *param);
    void caseProgram(ASTProgram &host, void *param);
};

class CheckForExtraneousImports : public RecursiveVisitor
{
public:
    CheckForExtraneousImports() : ok(true) {}

    void caseDefault(void *param);
    void caseImportDecl(ASTImportDecl &host, void *param);

	bool isOK() const {return ok;}
private:
    bool ok;
};

class ExtractType : public ASTVisitor
{
public:
    void caseDefault(void *param);
    void caseTypeVoid(ASTTypeVoid &host, void *param);
    void caseTypeGlobal(ASTTypeGlobal &host, void *param);
    void caseTypeFloat(ASTTypeFloat &host, void *param);
    void caseTypeBool(ASTTypeBool &host, void *param);
    void caseTypeFFC(ASTTypeFFC &host, void *param);
    void caseTypeItem(ASTTypeItem &host, void *param);
    void caseTypeItemclass(ASTTypeItemclass &host, void *param);
    void caseTypeNPC(ASTTypeNPC &host, void *param);
    void caseTypeLWpn(ASTTypeLWpn &host, void *param);
    void caseTypeEWpn(ASTTypeEWpn &host, void *param);
    void caseTypeMapData(ASTTypeMapData &host, void *param);
    void caseTypeNPCData(ASTTypeNPCData &host, void *param);
    void caseTypeDebug(ASTTypeDebug &host, void *param);
    void caseTypeAudio(ASTTypeAudio &host, void *param);
    void caseTypeComboData(ASTTypeComboData &host, void *param);
    void caseTypeSpriteData(ASTTypeSpriteData &host, void *param);
    void caseTypeGraphics(ASTTypeGraphics &host, void *param);
    void caseTypeText(ASTTypeText &host, void *param);
    void caseTypeInput(ASTTypeInput &host, void *param);
};

class IsDotExpr : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool *)param = false;}
    void caseExprDot(ASTExprDot &, void *param) {*(bool *)param = true;}
};

class IsArrayDecl : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool *)param = false;}
    void caseArrayDecl(ASTArrayDecl &, void *param) {*(bool *)param = true;}
};

class IsBlock : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool *)param = false;}
    void caseBlock(ASTBlock &, void *param) {*(bool *)param = true;}
};

class IsFuncDecl : public ASTVisitor
{
public:
    void caseDefault(void *param) {*(bool *)param = false;}
    virtual void caseFuncDecl(ASTFuncDecl &, void *param) {*(bool *)param = true;}
};

#endif


#ifndef UTILVISITORS_H //2.53 Updated to 16th Jan, 2017
#define UTILVISITORS_H

#include "AST.h"

class RecursiveVisitor : public ASTVisitor
{
public:
    virtual void caseDefault(void*) {}
    virtual void caseProgram(ASTProgram &host, void *param);
    virtual void caseProgram(ASTProgram &host);
	// Statements
    virtual void caseBlock(ASTBlock &host, void *param);
    virtual void caseBlock(ASTBlock &host);
    virtual void caseStmtAssign(ASTStmtAssign &host, void *param);
    virtual void caseStmtAssign(ASTStmtAssign &host);
    virtual void caseStmtIf(ASTStmtIf &host, void *param);
    virtual void caseStmtIf(ASTStmtIf &host);
    virtual void caseStmtIfElse(ASTStmtIfElse &host, void *param);
    virtual void caseStmtIfElse(ASTStmtIfElse &host);
	virtual void caseStmtSwitch(ASTStmtSwitch & host, void* param);
	virtual void caseStmtSwitch(ASTStmtSwitch & host);
	virtual void caseSwitchCases(ASTSwitchCases & host, void* param);
	virtual void caseSwitchCases(ASTSwitchCases & host);
    virtual void caseStmtFor(ASTStmtFor &host, void* param);
    virtual void caseStmtFor(ASTStmtFor &host);
    virtual void caseStmtWhile(ASTStmtWhile &host, void *param);
    virtual void caseStmtWhile(ASTStmtWhile &host);
    virtual void caseStmtDo(ASTStmtDo &host, void *param);
    virtual void caseStmtDo(ASTStmtDo &host);
    virtual void caseStmtReturnVal(ASTStmtReturnVal &host, void *param);
    virtual void caseStmtReturnVal(ASTStmtReturnVal &host);
	// Declarations
    virtual void caseScript(ASTScript &host, void *param);
    virtual void caseScript(ASTScript &host);
    virtual void caseDeclList(ASTDeclList &host, void *param);
    virtual void caseDeclList(ASTDeclList &host);
    virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
    virtual void caseFuncDecl(ASTFuncDecl &host);
    virtual void caseArrayDecl(ASTArrayDecl &host, void *param);
    virtual void caseArrayDecl(ASTArrayDecl &host);
    virtual void caseVarDecl(ASTVarDecl &host, void *param);
    virtual void caseVarDecl(ASTVarDecl &host);
    virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);
    virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host);
	// Expressions
	virtual void caseExprConst(ASTExprConst &host, void *param);
	virtual void caseExprConst(ASTExprConst &host);
    virtual void caseNumConstant(ASTNumConstant &host, void *param);
    virtual void caseNumConstant(ASTNumConstant &host);
    virtual void caseExprArrow(ASTExprArrow &host, void *param);
    virtual void caseExprArrow(ASTExprArrow &host);
    virtual void caseExprArray(ASTExprArray &host, void *param);
    virtual void caseExprArray(ASTExprArray &host);
    virtual void caseFuncCall(ASTFuncCall &host, void *param);
    virtual void caseFuncCall(ASTFuncCall &host);
    virtual void caseExprNegate(ASTExprNegate &host, void *param);
    virtual void caseExprNegate(ASTExprNegate &host);
    virtual void caseExprNot(ASTExprNot &host, void *param);
    virtual void caseExprNot(ASTExprNot &host);
    virtual void caseExprBitNot(ASTExprBitNot &host, void *param);
    virtual void caseExprBitNot(ASTExprBitNot &host);
    virtual void caseExprIncrement(ASTExprIncrement &host, void *param);
    virtual void caseExprIncrement(ASTExprIncrement &host);
    virtual void caseExprPreIncrement(ASTExprPreIncrement &host, void *param);
    virtual void caseExprPreIncrement(ASTExprPreIncrement &host);
    virtual void caseExprDecrement(ASTExprDecrement &host, void *param);
    virtual void caseExprDecrement(ASTExprDecrement &host);
    virtual void caseExprPreDecrement(ASTExprPreDecrement &host, void *param);
    virtual void caseExprPreDecrement(ASTExprPreDecrement &host);
    virtual void caseExprAnd(ASTExprAnd &host, void *param);
    virtual void caseExprAnd(ASTExprAnd &host);
    virtual void caseExprOr(ASTExprOr &host, void *param);
    virtual void caseExprOr(ASTExprOr &host);
    virtual void caseExprGT(ASTExprGT &host, void *param);
    virtual void caseExprGT(ASTExprGT &host);
    virtual void caseExprGE(ASTExprGE &host, void *param);
    virtual void caseExprGE(ASTExprGE &host);
    virtual void caseExprLT(ASTExprLT &host, void *param);
    virtual void caseExprLT(ASTExprLT &host);
    virtual void caseExprLE(ASTExprLE &host, void *param);
    virtual void caseExprLE(ASTExprLE &host);
    virtual void caseExprEQ(ASTExprEQ &host, void *param);
    virtual void caseExprEQ(ASTExprEQ &host);
    virtual void caseExprNE(ASTExprNE &host, void *param);
    virtual void caseExprNE(ASTExprNE &host);
    virtual void caseExprPlus(ASTExprPlus &host, void *param);
    virtual void caseExprPlus(ASTExprPlus &host);
    virtual void caseExprMinus(ASTExprMinus &host, void *param);
    virtual void caseExprMinus(ASTExprMinus &host);
    virtual void caseExprTimes(ASTExprTimes &host, void *param);
    virtual void caseExprTimes(ASTExprTimes &host);
    virtual void caseExprDivide(ASTExprDivide &host, void *param);
    virtual void caseExprDivide(ASTExprDivide &host);
    virtual void caseExprModulo(ASTExprModulo &host, void *param);
    virtual void caseExprModulo(ASTExprModulo &host);
    virtual void caseExprBitAnd(ASTExprBitAnd &host, void *param);
    virtual void caseExprBitAnd(ASTExprBitAnd &host);
    virtual void caseExprBitOr(ASTExprBitOr &host, void *param);
    virtual void caseExprBitOr(ASTExprBitOr &host);
    virtual void caseExprBitXor(ASTExprBitXor &host, void *param);
    virtual void caseExprBitXor(ASTExprBitXor &host);
    virtual void caseExprLShift(ASTExprLShift &host, void *param);
    virtual void caseExprLShift(ASTExprLShift &host);
    virtual void caseExprRShift(ASTExprRShift &host, void *param);
    virtual void caseExprRShift(ASTExprRShift &host);
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
    void caseTypeDMap(ASTTypeDMap &host, void *param);
    void caseTypeZMsg(ASTTypeZMsg &host, void *param);
    void caseTypeShopData(ASTTypeShopData &host, void *param);
    void caseTypeUntyped(ASTTypeUntyped &host, void *param);
    
    void caseTypeDropsets(ASTTypeDropsets &host, void *param);
    void caseTypePonds(ASTTypePonds &host, void *param);
    void caseTypeWarprings(ASTTypeWarprings &host, void *param);
    void caseTypeDoorsets(ASTTypeDoorsets &host, void *param);
    void caseTypeZColours(ASTTypeZColours &host, void *param);
    void caseTypeRGB(ASTTypeRGB &host, void *param);
    void caseTypePalette(ASTTypePalette &host, void *param);
    void caseTypeTunes(ASTTypeTunes &host, void *param);
    void caseTypePalCycle(ASTTypePalCycle &host, void *param);
    void caseTypeGamedata(ASTTypeGamedata &host, void *param);
    void caseTypeCheats(ASTTypeCheats &host, void *param);
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


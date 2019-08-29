#ifndef SCRIPTPARSER_H
#define SCRIPTPARSER_H

class AST;
// for flex and bison
#define YYSTYPE AST*

#include "y.tab.hpp"
#include "Compiler.h"
#include <list>
#include <vector>
#include <map>
#include <string>

using namespace std;

extern string curfilename;
int go(const char *f);

class ASTScript;
class ASTImportDecl;
class ASTConstDecl;
class ASTFuncDecl;
class ASTFloat;
class ASTString;
class ASTProgram;
class ASTDeclList;
class ASTDecl;
class ASTVarDecl;
class ASTType;
class ASTBlock;
class ASTTypeFloat;
class ASTTypeBool;
class ASTTypeVoid;
class ASTTypeFFC;
class ASTTypeItem;
class ASTTypeItemclass;
class ASTTypeGlobal;
class ASTVarDeclInitializer;
class ASTExpr;
class ASTExprAnd;
class ASTExprOr;
class ASTExprGT;
class ASTExprGE;
class ASTExprLT;
class ASTExprLE;
class ASTExprEQ;
class ASTExprNE;
class ASTExprPlus;
class ASTExprMinus;
class ASTExprTimes;
class ASTExprDivide;
class ASTExprNot;
class ASTExprNegate;
class ASTExprArrow;
class ASTNumConstant;
class ASTFuncCall;
class ASTBoolConstant;
class ASTBlock;
class ASTStmt;
class ASTStmtAssign;
class ASTExprDot;
class ASTStmtFor;
class ASTStmtIf;
class ASTStmtIfElse;
class ASTStmtReturn;
class ASTStmtReturnVal;
class ASTStmtEmpty;
class ASTScript;
class ASTStmtWhile;
class ASTExprBitOr;
class ASTExprBitAnd;
class ASTExprBitXor;
class ASTExprLShift;
class ASTExprRShift;
class ASTExprModulo;
class ASTExprBitNot;
class ASTExprIncrement;
class ASTExprDecrement;
class ASTStmtBreak;
class ASTStmtContinue;

class ASTVisitor
{
public:
	virtual void caseDefault(void *param)=0;
	virtual void caseProgram(ASTProgram &host, void *param) {caseDefault(param);}
	virtual void caseFloat(ASTFloat &host, void *param) {caseDefault(param);}
	virtual void caseString(ASTString &host, void *param) {caseDefault(param);}
	virtual void caseDeclList(ASTDeclList &host, void *param) {caseDefault(param);}
	virtual void caseImportDecl(ASTImportDecl &host, void *param) {caseDefault(param);}
	virtual void caseConstDecl(ASTConstDecl &host, void *param) {caseDefault(param);}
	virtual void caseFuncDecl(ASTFuncDecl &host, void *param) {caseDefault(param);}
	virtual void caseTypeFloat(ASTTypeFloat &host, void *param) {caseDefault(param);}
	virtual void caseTypeBool(ASTTypeBool &host, void *param) {caseDefault(param);}
	virtual void caseTypeVoid(ASTTypeVoid &host, void *param) {caseDefault(param);}
	virtual void caseTypeFFC(ASTTypeFFC &host, void *param) {caseDefault(param);}
	virtual void caseTypeItem(ASTTypeItem &host, void *param) {caseDefault(param);}
	virtual void caseTypeItemclass(ASTTypeItemclass &host, void *param) {caseDefault(param);}
	virtual void caseTypeGlobal(ASTTypeGlobal &host, void *param) {caseDefault(param);}
	virtual void caseVarDecl(ASTVarDecl &host, void *param) {caseDefault(param);}
	virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param) {caseDefault(param);}
	virtual void caseExprAnd(ASTExprAnd &host, void *param) {caseDefault(param);}
	virtual void caseExprOr(ASTExprOr &host, void *param) {caseDefault(param);}
	virtual void caseExprGT(ASTExprGT &host, void *param) {caseDefault(param);}
	virtual void caseExprGE(ASTExprGE &host, void *param) {caseDefault(param);}
	virtual void caseExprLT(ASTExprLT &host, void *param) {caseDefault(param);}
	virtual void caseExprLE(ASTExprLE &host, void *param) {caseDefault(param);}
	virtual void caseExprEQ(ASTExprEQ &host, void *param) {caseDefault(param);}
	virtual void caseExprNE(ASTExprNE &host, void *param) {caseDefault(param);}
	virtual void caseExprPlus(ASTExprPlus &host, void *param) {caseDefault(param);}
	virtual void caseExprMinus(ASTExprMinus &host, void *param) {caseDefault(param);}
	virtual void caseExprTimes(ASTExprTimes &host, void *param) {caseDefault(param);}
	virtual void caseExprDivide(ASTExprDivide &host, void *param) {caseDefault(param);}
	virtual void caseExprNot(ASTExprNot &host, void *param) {caseDefault(param);}
	virtual void caseExprArrow(ASTExprArrow &host, void *param) {caseDefault(param);}
	virtual void caseExprNegate(ASTExprNegate &host, void *param) {caseDefault(param);}
	virtual void caseNumConstant(ASTNumConstant &host, void *param) {caseDefault(param);}
	virtual void caseFuncCall(ASTFuncCall &host, void *param) {caseDefault(param);}
	virtual void caseBoolConstant(ASTBoolConstant &host, void *param) {caseDefault(param);}
	virtual void caseBlock(ASTBlock &host, void *param) {caseDefault(param);}
	virtual void caseStmtAssign(ASTStmtAssign &host, void *param) {caseDefault(param);}
	virtual void caseExprDot(ASTExprDot &host, void *param) {caseDefault(param);}
	virtual void caseStmtFor(ASTStmtFor &host, void *param) {caseDefault(param);}
	virtual void caseStmtIf(ASTStmtIf &host, void *param) {caseDefault(param);}
	virtual void caseStmtIfElse(ASTStmtIfElse &host, void *param) {caseDefault(param);}
	virtual void caseStmtReturn(ASTStmtReturn &host, void *param) {caseDefault(param);}
	virtual void caseStmtReturnVal(ASTStmtReturnVal &host, void *param) {caseDefault(param);}
	virtual void caseStmtEmpty(ASTStmtEmpty &host, void *param) {caseDefault(param);}
	virtual void caseScript(ASTScript &host, void *param) {caseDefault(param);}
	virtual void caseStmtWhile(ASTStmtWhile &host, void *param) {caseDefault(param);}
	virtual void caseExprBitOr(ASTExprBitOr &host, void *param) {caseDefault(param);}
	virtual void caseExprBitXor(ASTExprBitXor &host, void *param) {caseDefault(param);}
	virtual void caseExprBitAnd(ASTExprBitAnd &host, void *param) {caseDefault(param);}
	virtual void caseExprLShift(ASTExprLShift &host, void *param) {caseDefault(param);}
	virtual void caseExprRShift(ASTExprRShift &host, void *param) {caseDefault(param);}
	virtual void caseExprBitNot(ASTExprBitNot &host, void *param) {caseDefault(param);}
	virtual void caseExprIncrement(ASTExprIncrement &host, void *param) {caseDefault(param);}
	virtual void caseExprDecrement(ASTExprDecrement &host, void *param) {caseDefault(param);}
	virtual void caseExprModulo(ASTExprModulo &host, void *param) {caseDefault(param);}
	virtual void caseStmtBreak(ASTStmtBreak &host, void *param) {caseDefault(param);}
	virtual void caseStmtContinue(ASTStmtContinue &host, void *param) {caseDefault(param);}
	virtual ~ASTVisitor() {}
};

//////////////////////////////////////////////////////////////////////////////
class LocationData
{
public:
	LocationData(YYLTYPE loc){
		first_line = loc.first_line;
		last_line = loc.last_line;
		first_column = loc.first_column;
		last_column = loc.last_column;
		fname = curfilename;
	}
	int first_line;
	int last_line;
	int first_column;
	int last_column;
	string fname;
};
//////////////////////////////////////////////////////////////////////////////
class AST
{
public:
	AST(LocationData loc) : loc(loc) {}
	virtual void execute(ASTVisitor &visitor, void *param)=0;
	virtual ~AST() {}
	LocationData &getLocation() {return loc;}
private:
	LocationData loc;
	//NOT IMPLEMENTED - do not use
	AST(AST &other);
	AST &operator=(AST &other);
};

class ASTStmt : public AST
{
public:
	ASTStmt(LocationData loc) : AST(loc) {}
};

class ASTProgram : public AST
{
public:
	ASTProgram(ASTDeclList *decls, LocationData loc) : AST(loc), decls(decls) {};

	ASTDeclList *getDeclarations() {return decls;}

	~ASTProgram();

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseProgram(*this,param);
	}
private:
	ASTDeclList *decls;
};

class ASTFloat : public AST
{
public:
	ASTFloat(const char * value, int type, LocationData loc) : AST(loc), type(type) {
		val = string(value);
	}
	string getValue() {return val;}
	pair<string,string> parseValue();
	int getType() {return type;}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseFloat(*this, param);
	}
	const static int TYPE_DECIMAL=0;
	const static int TYPE_BINARY=1;
	const static int TYPE_HEX=2;
private:
	int type;
	string val;
};

class ASTString : public AST
{
public:
	ASTString(const char *strval, LocationData loc) : AST(loc)
	{
		str = string(strval);
	}
	string getValue() {return str;}

	void execute(ASTVisitor &visitor, void *param)
	{
		return visitor.caseString(*this,param);
	}
private:
	string str;
};

class ASTDeclList : public AST
{
public:
	ASTDeclList(LocationData loc) : AST(loc), decls() {}
	~ASTDeclList();

	void addDeclaration(ASTDecl *newdecl);
	
	list<ASTDecl *> &getDeclarations() {return decls;}

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseDeclList(*this, param);
	}
private:
	list<ASTDecl *> decls;
};

class ASTDecl : public ASTStmt
{
public:
	ASTDecl(LocationData loc) : ASTStmt(loc) {}
};

class ASTImportDecl : public ASTDecl
{
public:
	ASTImportDecl(string file, LocationData loc) : ASTDecl(loc), filename(file) {}

	string getFilename() {return filename;}

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseImportDecl(*this,param);
	}
private:
	string filename;
};

class ASTConstDecl : public ASTDecl
{
public:
	ASTConstDecl(string name, ASTFloat *val, LocationData loc) : ASTDecl(loc), name(name), val(val) {}
	string getName() {return name;}
	ASTFloat *getValue() {return val;}
	~ASTConstDecl() {delete val;}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseConstDecl(*this,param);
	}
private:
	string name;
	ASTFloat *val;
};

class ASTFuncDecl : public ASTDecl
{
public:
	ASTFuncDecl(LocationData loc) : ASTDecl(loc), params() {
		
	}

	void setName(string n) {name=n;}
	void setReturnType(ASTType *type) {rettype=type;}
	void setBlock(ASTBlock *b) {block=b;}
	
	~ASTFuncDecl();
	void addParam(ASTVarDecl *param);
	list<ASTVarDecl *> &getParams() {return params;}
	ASTType *getReturnType() {return rettype;}
	ASTBlock *getBlock() {return block;}
	string getName() {return name;}

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseFuncDecl(*this,param);
	}
private:
	string name;
	list<ASTVarDecl *> params;
	ASTType *rettype;
	ASTBlock *block;
};

class ASTType : public AST
{
public:
	ASTType(LocationData loc) : AST(loc) {}
	virtual void execute(ASTVisitor &visitor, void *param)=0;
};

class ASTTypeFloat : public ASTType
{
public:
	ASTTypeFloat(LocationData loc) : ASTType(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseTypeFloat(*this,param);
	}
};

class ASTTypeBool : public ASTType
{
public:
	ASTTypeBool(LocationData loc) : ASTType(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseTypeBool(*this,param);
	}
};

class ASTTypeVoid : public ASTType
{
public:
	ASTTypeVoid(LocationData loc) : ASTType(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseTypeVoid(*this,param);
	}
};

class ASTTypeFFC : public ASTType
{
public:
	ASTTypeFFC(LocationData loc) : ASTType(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseTypeFFC(*this,param);
	}
};

class ASTTypeGlobal : public ASTType
{
public:
	ASTTypeGlobal(LocationData loc) : ASTType(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseTypeGlobal(*this,param);
	}
};

class ASTTypeItem : public ASTType
{
public:
	ASTTypeItem(LocationData loc) : ASTType(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseTypeItem(*this,param);
	}
};

class ASTTypeItemclass : public ASTType
{
public:
	ASTTypeItemclass(LocationData loc) : ASTType(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseTypeItemclass(*this,param);
	}
};

class ASTVarDecl : public ASTDecl
{
public:
	ASTVarDecl(ASTType *type, string name, LocationData loc) : ASTDecl(loc), type(type), name(name) {}
	ASTType *getType() {return type;}
	string getName() {return name;}
	virtual ~ASTVarDecl();
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseVarDecl(*this, param);
	}
private:
	ASTType *type;
	string name;
};

class ASTVarDeclInitializer : public ASTVarDecl
{
public:
	ASTVarDeclInitializer(ASTType *type, string name, ASTExpr *initial, LocationData loc) : 
	 ASTVarDecl(type,name,loc), initial(initial) {}
	~ASTVarDeclInitializer();
	ASTExpr *getInitializer() {return initial;}

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseVarDeclInitializer(*this,param);
	}
private:
	ASTExpr *initial;
};

class ASTExpr : public ASTStmt
{
public:
	ASTExpr(LocationData loc) : ASTStmt(loc), hasval(false), intval(0), type(-1) {}
	virtual ~ASTExpr() {}
	long getIntValue() {return intval;}
	bool hasIntValue() {return hasval;}
	int getType() {return type;}
	void setIntValue(long val) {hasval = true; intval=val;}
	void setType(int t) {type=t;}
private:
	bool hasval;
	long intval;
	int type;
};

class ASTUnaryExpr : public ASTExpr
{
public:
	ASTUnaryExpr(LocationData loc) : ASTExpr(loc) {}
	virtual ~ASTUnaryExpr() {delete operand;}

	void setOperand(ASTExpr *e) {operand=e;}
	ASTExpr *getOperand() {return operand;}
private:
	ASTExpr *operand;
};

class ASTBinaryExpr : public ASTExpr
{
public:
	ASTBinaryExpr(LocationData loc) : ASTExpr(loc) {}
	virtual ~ASTBinaryExpr() {delete first; delete second;}
	void setFirstOperand(ASTExpr *e) {first=e;}
	void setSecondOperand(ASTExpr *e) {second=e;}
	ASTExpr *getFirstOperand() {return first;}
	ASTExpr *getSecondOperand() {return second;}
private:
	ASTExpr *first;
	ASTExpr *second;
};

class ASTLogExpr : public ASTBinaryExpr
{
public:
	ASTLogExpr(LocationData loc) : ASTBinaryExpr(loc) {}
};

class ASTBitExpr : public ASTBinaryExpr
{
public:
	ASTBitExpr(LocationData loc) : ASTBinaryExpr(loc) {}
};

class ASTShiftExpr : public ASTBinaryExpr
{
public:
	ASTShiftExpr(LocationData loc) : ASTBinaryExpr(loc) {}
};

class ASTExprAnd : public ASTLogExpr
{
public:
	ASTExprAnd(LocationData loc) : ASTLogExpr(loc) {}

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprAnd(*this,param);
	}
};

class ASTExprOr : public ASTLogExpr
{
public:
	ASTExprOr(LocationData loc) : ASTLogExpr(loc) {}

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprOr(*this,param);
	}
};

class ASTRelExpr : public ASTBinaryExpr
{
public:
	ASTRelExpr(LocationData loc) : ASTBinaryExpr(loc) {}
};

class ASTExprGT : public ASTRelExpr
{
public:
	ASTExprGT(LocationData loc) : ASTRelExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprGT(*this, param);
	}
};

class ASTExprGE : public ASTRelExpr
{
public:
	ASTExprGE(LocationData loc) : ASTRelExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprGE(*this, param);
	}
};

class ASTExprLT : public ASTRelExpr
{
public:
	ASTExprLT(LocationData loc) : ASTRelExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprLT(*this, param);
	}
};

class ASTExprLE : public ASTRelExpr
{
public:
	ASTExprLE(LocationData loc) : ASTRelExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprLE(*this, param);
	}
};

class ASTExprEQ : public ASTRelExpr
{
public:
	ASTExprEQ(LocationData loc) : ASTRelExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprEQ(*this, param);
	}
};

class ASTExprNE : public ASTRelExpr
{
public:
	ASTExprNE(LocationData loc) : ASTRelExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprNE(*this, param);
	}
};

class ASTAddExpr : public ASTBinaryExpr
{
public:
	ASTAddExpr(LocationData loc) : ASTBinaryExpr(loc) {}
};

class ASTExprPlus : public ASTAddExpr
{
public:
	ASTExprPlus(LocationData loc) : ASTAddExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprPlus(*this,param);
	}
};

class ASTExprMinus : public ASTAddExpr
{
public:
	ASTExprMinus(LocationData loc) : ASTAddExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprMinus(*this, param);
	}
};

class ASTMultExpr : public ASTBinaryExpr
{
public:
	ASTMultExpr(LocationData loc) : ASTBinaryExpr(loc) {}
};

class ASTExprTimes : public ASTMultExpr
{
public:
	ASTExprTimes(LocationData loc) : ASTMultExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprTimes(*this, param);
	}
};

class ASTExprDivide : public ASTMultExpr
{
public:
	ASTExprDivide(LocationData loc) : ASTMultExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprDivide(*this,param);
	}
};

class ASTExprBitOr : public ASTBitExpr
{
public:
	ASTExprBitOr(LocationData loc) : ASTBitExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprBitOr(*this,param);
	}
};

class ASTExprBitXor : public ASTBitExpr
{
public:
	ASTExprBitXor(LocationData loc) : ASTBitExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprBitXor(*this,param);
	}
};

class ASTExprBitAnd : public ASTBitExpr
{
public:
	ASTExprBitAnd(LocationData loc) : ASTBitExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprBitAnd(*this,param);
	}
};

class ASTExprLShift : public ASTShiftExpr
{
public:
	ASTExprLShift(LocationData loc) : ASTShiftExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprLShift(*this,param);
	}
};

class ASTExprRShift : public ASTShiftExpr
{
public:
	ASTExprRShift(LocationData loc) : ASTShiftExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprRShift(*this,param);
	}
};

class ASTExprNegate : public ASTUnaryExpr
{
public:
	ASTExprNegate(LocationData loc) : ASTUnaryExpr(loc) {}

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprNegate(*this,param);
	}
};

class ASTExprNot : public ASTUnaryExpr
{
public:
	ASTExprNot(LocationData loc) : ASTUnaryExpr(loc) {}

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprNot(*this,param);
	}
};

class ASTExprBitNot : public ASTUnaryExpr
{
public:
	ASTExprBitNot(LocationData loc) : ASTUnaryExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprBitNot(*this,param);
	}
};

class ASTExprIncrement : public ASTUnaryExpr
{
public:
	ASTExprIncrement(LocationData loc) : ASTUnaryExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprIncrement(*this,param);
	}
};

class ASTExprDecrement : public ASTUnaryExpr
{
public:
	ASTExprDecrement(LocationData loc) : ASTUnaryExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprDecrement(*this,param);
	}
};

class ASTExprModulo : public ASTMultExpr
{
public:
	ASTExprModulo(LocationData loc) : ASTMultExpr(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprModulo(*this,param);
	}
};

class ASTNumConstant : public ASTExpr
{
public:
	ASTNumConstant(ASTFloat *value, LocationData loc) : ASTExpr(loc), val(value) {}

	ASTFloat *getValue() {return val;}

	~ASTNumConstant() {delete val;}

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseNumConstant(*this,param);
	}
private:
	ASTFloat *val;
};

class ASTFuncCall : public ASTExpr
{
public:
	ASTFuncCall(LocationData loc) : ASTExpr(loc), params() {}

	list<ASTExpr *> &getParams() {return params;}

	void setName(ASTExpr *n) {name=n;}
	ASTExpr * getName() {return name;}
	void addParam(ASTExpr *param) {params.push_front(param);}

	~ASTFuncCall();

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseFuncCall(*this,param);
	}
private:
	ASTExpr *name;
	list<ASTExpr *> params;
};

class ASTBoolConstant : public ASTExpr
{
public:
	ASTBoolConstant(bool value, LocationData loc) : ASTExpr(loc), value(value) {}
	bool getValue() {return value;}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseBoolConstant(*this, param);
	}
private:
	bool value;
};

class ASTBlock : public ASTStmt
{
public:
	ASTBlock(LocationData loc) : ASTStmt(loc), statements() {}
	~ASTBlock();

	list<ASTStmt *> &getStatements() {return statements;}
	void addStatement(ASTStmt *stmt);

	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseBlock(*this,param);
	}
private:
	list<ASTStmt *> statements;
};

class ASTStmtAssign : public ASTStmt
{
public:
	ASTStmtAssign(ASTStmt *lval, ASTExpr *rval, LocationData loc) : ASTStmt(loc), lval(lval), rval(rval) {}
	ASTStmt *getLVal() {return lval;}
	ASTExpr *getRVal() {return rval;}
	~ASTStmtAssign() {delete lval; delete rval;}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseStmtAssign(*this,param);
	}
private:
	ASTStmt *lval;
	ASTExpr *rval;
};

class ASTExprDot : public ASTExpr
{
public:
	ASTExprDot(string nspace, string name, LocationData loc) : ASTExpr(loc), name(name), nspace(nspace) {}
	string getName() {return name;}
	string getNamespace() {return nspace;}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprDot(*this,param);
	}
private:
	string name;
	string nspace;
};

class ASTExprArrow : public ASTExpr
{
public:
	ASTExprArrow(ASTExpr *lval, string rval, LocationData loc) : ASTExpr(loc), lval(lval), rval(rval), index(NULL) {}
	string getName() {return rval;}
	ASTExpr *getLVal() {return lval;}
	~ASTExprArrow() {delete lval; if(index) delete index;}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseExprArrow(*this,param);
	}
	ASTExpr *getIndex() {return index;}
	void setIndex(ASTExpr *e) {index = e;}
private:
	ASTExpr *lval;
	string rval;
	ASTExpr *index;
};

class ASTStmtFor : public ASTStmt
{
public:
	ASTStmtFor(ASTStmt *prec, ASTExpr *term, ASTStmt *incr, ASTStmt *stmt, LocationData loc) :
	  ASTStmt(loc), prec(prec), term(term), incr(incr), stmt(stmt) {}
	~ASTStmtFor() {delete prec; delete term; delete incr; delete stmt;}
	ASTStmt *getPrecondition() {return prec;}
	ASTExpr *getTerminationCondition() {return term;}
	ASTStmt *getIncrement() {return incr;}
	ASTStmt *getStmt() {return stmt;}
	void execute(ASTVisitor &visitor, void *param)
	{
		return visitor.caseStmtFor(*this,param);
	}
private:
	ASTStmt *prec;
	ASTExpr *term;
	ASTStmt *incr;
	ASTStmt *stmt;
};

class ASTStmtIf : public ASTStmt
{
public:
	ASTStmtIf(ASTExpr *cond, ASTStmt *stmt, LocationData loc) : ASTStmt(loc), cond(cond), stmt(stmt) {}
	ASTExpr *getCondition() {return cond;}
	ASTStmt *getStmt() {return stmt;}
	~ASTStmtIf() {delete cond; delete stmt;}
	void execute(ASTVisitor &visitor, void *param)
	{
		return visitor.caseStmtIf(*this,param);
	}
private:
	ASTExpr *cond;
	ASTStmt *stmt;
};

class ASTStmtIfElse : public ASTStmtIf
{
public:
	ASTStmtIfElse(ASTExpr *cond, ASTStmt *ifstmt, ASTStmt *elsestmt, LocationData loc) :
	  ASTStmtIf(cond,ifstmt, loc), elsestmt(elsestmt) {}
	~ASTStmtIfElse() {delete elsestmt;}
	ASTStmt *getElseStmt() {return elsestmt;}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseStmtIfElse(*this, param);
	}
private:
	ASTStmt *elsestmt;
};

class ASTStmtReturn : public ASTStmt
{
public:
	ASTStmtReturn(LocationData loc) : ASTStmt(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseStmtReturn(*this, param);
	}
};

class ASTStmtReturnVal : public ASTStmt
{
public:
	ASTStmtReturnVal(ASTExpr *retval, LocationData loc) : ASTStmt(loc), retval(retval) {}
	ASTExpr *getReturnValue() {return retval;}
	~ASTStmtReturnVal() {delete retval;}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseStmtReturnVal(*this,param);
	}
private:
	ASTExpr *retval;
};

class ASTStmtBreak : public ASTStmt
{
public:
	ASTStmtBreak(LocationData loc) : ASTStmt(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseStmtBreak(*this,param);
	}
};

class ASTStmtContinue : public ASTStmt
{
public:
	ASTStmtContinue(LocationData loc) : ASTStmt(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseStmtContinue(*this, param);
	}
};

class ASTStmtEmpty : public ASTStmt
{
public:
	ASTStmtEmpty(LocationData loc) : ASTStmt(loc) {}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseStmtEmpty(*this, param);
	}
};

class ASTScript : public ASTDecl
{
public:
	ASTScript(ASTType *type, string name, ASTDeclList *sblock, LocationData loc) : ASTDecl(loc), type(type), name(name), sblock(sblock) {}
	~ASTScript();
	ASTDeclList *getScriptBlock() {return sblock;}
	ASTType *getType() {return type;}
	string getName() {return name;}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseScript(*this,param);
	}
private:
	ASTType *type;
	string name;
	ASTDeclList *sblock;
};

class ASTStmtWhile : public ASTStmt
{
public:
	ASTStmtWhile(ASTExpr *cond, ASTStmt *stmt, LocationData loc) : ASTStmt(loc), cond(cond), stmt(stmt) {}
	~ASTStmtWhile() {delete cond; delete stmt;}
	ASTExpr *getCond() {return cond;}
	ASTStmt *getStmt() {return stmt;}
	void execute(ASTVisitor &visitor, void *param)
	{
		visitor.caseStmtWhile(*this,param);
	}
private:
	ASTExpr *cond;
	ASTStmt *stmt;
};

//////////////////////////////////////////////////////////////////////////////
class Clone : public ASTVisitor
{
public:
	virtual void caseDefault(void *param);
	virtual void caseProgram(ASTProgram &host, void *param);
	virtual void caseFloat(ASTFloat &host, void *param);
	virtual void caseString(ASTString &host, void *param);
	virtual void caseDeclList(ASTDeclList &host, void *param);
	virtual void caseImportDecl(ASTImportDecl &host, void *param);
	virtual void caseConstDecl(ASTConstDecl &host, void *param);
	virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
	virtual void caseTypeFloat(ASTTypeFloat &host, void *param);
	virtual void caseTypeBool(ASTTypeBool &host, void *param);
	virtual void caseTypeVoid(ASTTypeVoid &host, void *param);
	virtual void caseTypeFFC(ASTTypeFFC &host, void *param);
	virtual void caseTypeItem(ASTTypeItem &host, void *param);
	virtual void caseTypeItemclass(ASTTypeItemclass &host, void *param);
	virtual void caseTypeGlobal(ASTTypeGlobal &host, void *param);
	virtual void caseVarDecl(ASTVarDecl &host, void *param);
	virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);
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
	virtual void caseNumConstant(ASTNumConstant &host, void *param);
	virtual void caseFuncCall(ASTFuncCall &host, void *param);
	virtual void caseBoolConstant(ASTBoolConstant &host, void *param);
	virtual void caseBlock(ASTBlock &host, void *param);
	virtual void caseStmtAssign(ASTStmtAssign &host, void *param);
	virtual void caseExprDot(ASTExprDot &host, void *param);
	virtual void caseExprArrow(ASTExprArrow &host, void *param);
	virtual void caseStmtFor(ASTStmtFor &host, void *param);
	virtual void caseStmtIf(ASTStmtIf &host, void *param);
	virtual void caseStmtIfElse(ASTStmtIfElse &host, void *param);
	virtual void caseStmtReturn(ASTStmtReturn &host, void *param);
	virtual void caseStmtReturnVal(ASTStmtReturnVal &host, void *param);
	virtual void caseStmtEmpty(ASTStmtEmpty &host, void *param);
	virtual void caseScript(ASTScript &host, void *param);
	virtual void caseStmtWhile(ASTStmtWhile &host, void *param);
	virtual void caseExprBitOr(ASTExprBitOr &host, void *param);
	virtual void caseExprBitXor(ASTExprBitXor &host, void *param);
	virtual void caseExprBitAnd(ASTExprBitAnd &host, void *param);
	virtual void caseExprLShift(ASTExprLShift &host, void *param);
	virtual void caseExprRShift(ASTExprRShift &host, void *param);
	virtual void caseExprModulo(ASTExprModulo &host, void *param);
	virtual void caseExprBitNot(ASTExprBitNot &host, void *param);
	virtual void caseExprIncrement(ASTExprIncrement &host, void *param);
	virtual void caseExprDecrement(ASTExprDecrement &host, void *param);
	virtual void caseStmtBreak(ASTStmtBreak &host, void *param);
	virtual void caseStmtContinue(ASTStmtContinue &host, void *param);

	AST *getResult() {return result;}
private:
	AST *result;
};

class RecursiveVisitor : public ASTVisitor
{
public:
	virtual void caseDefault(void *param) {}
	virtual void caseProgram(ASTProgram &host, void *param){
		host.getDeclarations()->execute(*this,param);
	}
	virtual void caseDeclList(ASTDeclList &host, void *param)
	{
		list<ASTDecl *> l = host.getDeclarations();
		for(list<ASTDecl *>::iterator it = l.begin(); it != l.end(); it++)
		{
			(*it)->execute(*this,param);
		}
	}
	virtual void caseFuncDecl(ASTFuncDecl &host, void *param)
	{
		host.getReturnType()->execute(*this,param);
		list<ASTVarDecl *> l = host.getParams();
		for(list<ASTVarDecl *>::iterator it = l.begin(); it != l.end(); it++)
		{
			(*it)->execute(*this,param);
		}
		host.getBlock()->execute(*this,param);
	}
	virtual void caseVarDecl(ASTVarDecl &host, void *param)
	{
		host.getType()->execute(*this,param);
	}
	virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
	{
		host.getType()->execute(*this,param);
		host.getInitializer()->execute(*this,param);
	}
	virtual void caseExprAnd(ASTExprAnd &host, void *param)
	{
		host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprOr(ASTExprOr &host, void *param)
	{
				host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprGT(ASTExprGT &host, void *param)
	{		
		host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprGE(ASTExprGE &host, void *param)
	{
				host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprLT(ASTExprLT &host, void *param)
	{
				host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprLE(ASTExprLE &host, void *param)
	{
				host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprEQ(ASTExprEQ &host, void *param)
	{
				host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprNE(ASTExprNE &host, void *param)
	{
				host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprPlus(ASTExprPlus &host, void *param)
	{
				host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprMinus(ASTExprMinus &host, void *param)
	{
				host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprTimes(ASTExprTimes &host, void *param)
	{
				host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprDivide(ASTExprDivide &host, void *param)
	{
				host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprBitOr(ASTExprBitOr &host, void *param)
	{
		host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprBitXor(ASTExprBitXor &host, void *param)
	{
		host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprBitAnd(ASTExprBitAnd &host, void *param)
	{
		host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprLShift(ASTExprLShift &host, void *param)
	{
		host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprRShift(ASTExprRShift &host, void *param)
	{
		host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprModulo(ASTExprModulo &host, void *param)
	{
		host.getFirstOperand()->execute(*this,param);
		host.getSecondOperand()->execute(*this,param);
	}
	virtual void caseExprNot(ASTExprNot &host, void *param)
	{
		host.getOperand()->execute(*this,param);
	}
	virtual void caseExprBitNot(ASTExprBitNot &host, void *param)
	{
		host.getOperand()->execute(*this,param);
	}
	virtual void caseExprIncrement(ASTExprIncrement &host, void *param)
	{
		host.getOperand()->execute(*this,param);
	}
	virtual void caseExprDecrement(ASTExprDecrement &host, void *param)
	{
		host.getOperand()->execute(*this,param);
	}
	virtual void caseExprNegate(ASTExprNegate &host, void *param)
	{
		host.getOperand()->execute(*this,param);
	}
	virtual void caseExprArrow(ASTExprArrow &host, void*param)
	{
		host.getLVal()->execute(*this,param);
		if(host.getIndex())
			host.getIndex()->execute(*this,param);
	}
	virtual void caseNumConstant(ASTNumConstant &host, void *param)
	{
		host.getValue()->execute(*this,param);
	}
	virtual void caseFuncCall(ASTFuncCall &host, void *param)
	{
		list<ASTExpr *> l = host.getParams();
		for(list<ASTExpr *>::iterator it = l.begin(); it != l.end(); it++)
		{
			(*it)->execute(*this,param);
		}
	}
	virtual void caseBlock(ASTBlock &host, void *param)
	{
		list<ASTStmt *> l = host.getStatements();
		for(list<ASTStmt *>::iterator it = l.begin(); it != l.end(); it++)
			(*it)->execute(*this,param);
	}
	virtual void caseStmtAssign(ASTStmtAssign &host, void *param)
	{
		host.getLVal()->execute(*this,param);
		host.getRVal()->execute(*this,param);
	}
	virtual void caseStmtFor(ASTStmtFor &host, void *param)
	{
		host.getPrecondition()->execute(*this,param);
		host.getIncrement()->execute(*this,param);
		host.getTerminationCondition()->execute(*this,param);
		host.getStmt()->execute(*this,param);
	}
	virtual void caseStmtIf(ASTStmtIf &host, void *param)
	{
		host.getCondition()->execute(*this,param);
		host.getStmt()->execute(*this,param);
	}
	virtual void caseStmtIfElse(ASTStmtIfElse &host, void *param)
	{
		host.getCondition()->execute(*this,param);
		host.getStmt()->execute(*this,param);
		host.getElseStmt()->execute(*this,param);
	}
	virtual void caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
	{
		host.getReturnValue()->execute(*this,param);
	}
	virtual void caseScript(ASTScript &host, void *param)
	{
		host.getType()->execute(*this,param);
		host.getScriptBlock()->execute(*this,param);
	}
	virtual void caseStmtWhile(ASTStmtWhile &host, void *param)
	{
		host.getCond()->execute(*this,param);
		host.getStmt()->execute(*this,param);
	}
};

//removes all import declarations from a program
//call with param=NULL
//NOT responsible for freeing said imports
class GetImports : public ASTVisitor
{
public:
	GetImports() : result() {}
	virtual void caseDefault(void *param);
	virtual void caseDeclList(ASTDeclList &host, void *param);
	virtual void caseProgram(ASTProgram &host, void *param);
	virtual void caseImportDecl(ASTImportDecl &host, void *param);
	vector<ASTImportDecl *> &getResult() {return result;}
private:
	vector<ASTImportDecl *> result;
};

class GetConsts : public ASTVisitor
{
public:
	GetConsts() : result() {}
	virtual void caseDefault(void *param);
	virtual void caseDeclList(ASTDeclList &host, void *param);
	virtual void caseProgram(ASTProgram &host, void *param);
	virtual void caseConstDecl(ASTConstDecl &host, void *param);
	vector<ASTConstDecl *> &getResult() {return result;}
private:
	vector<ASTConstDecl *> result;
};

class GetGlobalFuncs : public ASTVisitor
{
public:
	GetGlobalFuncs() : result() {}
	virtual void caseDefault(void *param);
	virtual void caseDeclList(ASTDeclList &host, void *param);
	virtual void caseProgram(ASTProgram &host, void *param);
	virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
	vector<ASTFuncDecl *> getResult() {return result;}
private:
	vector<ASTFuncDecl *> result;
};

class GetScripts : public ASTVisitor
{
public:
	GetScripts() : result() {}
	virtual void caseDefault(void *param);
	virtual void caseScript(ASTScript &host, void *param);
	virtual void caseProgram(ASTProgram &host, void *param);
	virtual void caseDeclList(ASTDeclList &host, void *param);
	vector<ASTScript *> getResult() {return result;}
private:
	vector<ASTScript *> result;
};

//Merges one AST into another. Pass the AST that will be destroyed in the process
//as the param.
//The param AST gets completely consumed and deleted
class MergeASTs : public ASTVisitor
{
public:
	virtual void caseDefault(void *param);
	virtual void caseProgram(ASTProgram &host, void *param);
};

class CheckForExtraneousImports : public RecursiveVisitor
{
public:
	CheckForExtraneousImports() : ok(true) {}
	bool isOK() {return ok;}
	virtual void caseDefault(void *param);
	virtual void caseImportDecl(ASTImportDecl &host, void *param);
private:
	bool ok;
};

class ExtractType : public ASTVisitor
{
public:
	virtual void caseDefault(void *param);
	virtual void caseTypeVoid(ASTTypeVoid &host, void *param);
	virtual void caseTypeFloat(ASTTypeFloat &host, void *param);
	virtual void caseTypeBool(ASTTypeBool &host, void *param);
	virtual void caseTypeFFC(ASTTypeFFC &host, void *param);
	virtual void caseTypeItem(ASTTypeItem &host, void *param);
	virtual void caseTypeItemclass(ASTTypeItemclass &host, void *param);
	virtual void caseTypeGlobal(ASTTypeGlobal &host, void *param);
};

//builds the global symbols (functions and variables) for a script.
//param should be a pair<Scope, SymbolTable> pointer.
class BuildScriptSymbols : public ASTVisitor
{
public:
	BuildScriptSymbols() : failure(false) {}
	virtual void caseDefault(void *param);
	virtual void caseScript(ASTScript &host,void *param);
	virtual void caseVarDecl(ASTVarDecl &host, void *param);
	virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
	{
		caseVarDecl(host, param);
	}
	virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
	bool isOK() {return !failure;}
private:
	bool failure;
};

class BuildFunctionSymbols : public RecursiveVisitor
{
public:
	BuildFunctionSymbols() : failure(false) {}
	virtual void caseDefault(void *param) {}
	virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
	virtual void caseVarDecl(ASTVarDecl &host, void *param);
	virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
	{
		host.getInitializer()->execute(*this,param);
		caseVarDecl(host, param);
	}
	virtual void caseBlock(ASTBlock &host, void *param);
	virtual void caseStmtFor(ASTStmtFor &host, void *param);
	virtual void caseFuncCall(ASTFuncCall &host, void *param);
	virtual void caseExprDot(ASTExprDot &host, void *param);
	virtual void caseExprArrow(ASTExprArrow &host, void *param);
	bool isOK() {return !failure;}
private:
	bool failure;
};

class IsDotExpr : public ASTVisitor
{
public:
	virtual void caseDefault(void *param)
	{
		*(bool *)param = false;
	}
	virtual void caseExprDot(ASTExprDot &host, void *param)
	{
		*(bool *)param = true;
	}
};

class IsFuncDecl : public ASTVisitor
{
public:
	virtual void caseDefault(void *param) {*(bool *)param = false;}
	virtual void caseFuncDecl(ASTFuncDecl &host, void *param) {*(bool *)param = true;}
};
//////////////////////////////////////////////////////////////////////////////
class VariableSymbols
{
public:
	VariableSymbols() : symbols() {}
	int addVariable(string name, int type);
	bool containsVariable(string name);
	int getID(string name);
private:
	map<string, pair<int, int> > symbols;
};

class FunctionSymbols
{
public:
	FunctionSymbols() : symbols(), ambiguous() {}
	int addFunction(string name, int rettype, vector<int> paramtype);
	bool containsFunction(string name, vector<int> &params);
	int getID(string name, vector<int> &params);
	vector<int> getFuncIDs(string name);
private:
	map<pair<string, vector<int> >, pair<int,int> > symbols;
	map<string, vector<int> > ambiguous;
};

class SymbolTable
{
public:
	SymbolTable() : varTypes(), funcTypes(), astToID(), funcParams() {}
	int getVarType(int varID);
	int getFuncType(int funcID) {return funcTypes[funcID];}
	void putVar(int ID, int type) {varTypes[ID]=type;}
	void putFunc(int ID, int type);
	void putFuncDecl(int ID, vector<int> params) {funcParams[ID]=params;}
	void putAST(AST *obj, int ID);
	void putAmbiguousFunc(AST *func, vector<int> possibleIDs) {astToAmbiguousFuncIDs[func]=possibleIDs;}
	int getVarType(AST *obj);
	int getFuncType(AST *obj);
	vector<int> getFuncParams(int funcID) {return funcParams[funcID];}
	vector<int> getAmbiguousFuncs(AST *func) {return astToAmbiguousFuncIDs[func];}
	int getID(AST *obj) {return astToID[obj];}
	void printDiagnostics();
	vector<int> &getGlobalPointers(void) {return globalPointers;}
	void addGlobalPointer(int vid) {globalPointers.push_back(vid);}
private:
	map<int, int> varTypes;
	map<int, int> funcTypes;
	map<AST *, int> astToID;
	map<AST *, vector<int> > astToAmbiguousFuncIDs;
	map<int, vector<int> > funcParams;
	vector<int> globalPointers;
};

class Scope
{
public:
	Scope(Scope *parent) : namedChildren(), parent(parent), vars(), funcs() {}
	~Scope();
	VariableSymbols &getVarSymbols() {return vars;}
	FunctionSymbols &getFuncSymbols() {return funcs;}
	bool addNamedChild(string name, Scope *child);
	int getVarInScope(string nspace, string name);
	vector<int> getFuncsInScope(string nspace, string name);
	Scope *getNamedChild(string name);
private:
	map<string, Scope *> namedChildren;
	Scope *parent;
	VariableSymbols vars;
	FunctionSymbols funcs;
};
struct SymbolData
{
	SymbolTable *symbols;
	vector<ASTFuncDecl *> globalFuncs;
	vector<ASTScript *> scripts;
	map<ASTScript *, int> runsymbols;
	map<ASTScript *, int> numParams;
	map<ASTScript *, int> scriptTypes;
	map<ASTScript *, int> thisPtr;
};

struct FunctionData
{
	SymbolTable *symbols;
	vector<ASTFuncDecl *> functions;
	vector<ASTVarDecl *> globalVars;
	map<string, int> scriptRunSymbols;
	map<string, int> numParams;
	map<string, int> scriptTypes;
	map<string, int> thisPtr;
};

struct IntermediateData
{
	map<int, vector<Opcode *> > funcs;
	vector<Opcode *> globalsInit;
	map<string, int> scriptRunLabels;
	map<string, int> numParams;
	map<string, int> scriptTypes;
	map<string, int> thisPtr;
};

class LinkTable
{
public:
	int functionToLabel(int fid);
	int getGlobalID(int vid);
	int addGlobalVar(int vid);
	void addGlobalPointer(int vid) {globalIDs[vid]=0;}
private:
	map<int, int> funcLabels;
	map<int, int> globalIDs;
};

class StackFrame
{
public:
	void addToFrame(int vid, int offset) {stackoffset[vid] = offset;}
	int getOffset(int vid);
private:
	map<int, int> stackoffset;
};

struct OpcodeContext
{
	StackFrame *stackframe;
	LinkTable *linktable;
	SymbolTable *symbols;
};



//////////////////////////////////////////////////////////////////////////////

#define RECURSIONLIMIT 30

#endif

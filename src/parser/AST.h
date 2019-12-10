#ifndef AST_H //2.53 Updated to 16th Jan, 2017
#define AST_H

class AST;
// for flex and bison
#define YYSTYPE AST*

#ifdef _MSC_VER
#if (_MSC_VER <= 1600)
#define snprintf _snprintf
#endif
#endif

#include "y.tab.hpp"
#include "Compiler.h"
#include <list>
#include <vector>
#include <map>
#include <string>

using std::string;
using std::pair;
using std::map;
using std::vector;
using std::list;

#define RECURSIONLIMIT 30

extern string curfilename;
extern string headerguards;
int go(const char *f);

class ASTScript;
class ASTImportDecl;
class ASTConstDecl;
class ASTFuncDecl;
class ASTArrayDecl;
class ASTArrayList;
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
class ASTTypeNPC;
class ASTTypeLWpn;
class ASTTypeEWpn;
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
class ASTExprArray;
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
class ASTStmtDo;
class ASTExprBitOr;
class ASTExprBitAnd;
class ASTExprBitXor;
class ASTExprLShift;
class ASTExprRShift;
class ASTExprModulo;
class ASTExprBitNot;
class ASTExprIncrement;
class ASTExprPreIncrement;
class ASTExprDecrement;
class ASTExprPreDecrement;
class ASTStmtBreak;
class ASTStmtContinue;

class ASTVisitor
{
public:
    virtual void caseDefault(void *param)=0;
    virtual void caseProgram(ASTProgram &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseFloat(ASTFloat &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseString(ASTString &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseDeclList(ASTDeclList &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseImportDecl(ASTImportDecl &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseConstDecl(ASTConstDecl &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseFuncDecl(ASTFuncDecl &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseArrayDecl(ASTArrayDecl &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseTypeFloat(ASTTypeFloat &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseTypeBool(ASTTypeBool &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseTypeVoid(ASTTypeVoid &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseTypeFFC(ASTTypeFFC &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseTypeItem(ASTTypeItem &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseTypeItemclass(ASTTypeItemclass &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseTypeNPC(ASTTypeNPC &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseTypeLWpn(ASTTypeLWpn &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseTypeEWpn(ASTTypeEWpn &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseTypeGlobal(ASTTypeGlobal &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseVarDecl(ASTVarDecl &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseVarDeclInitializer(ASTVarDeclInitializer &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprAnd(ASTExprAnd &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprOr(ASTExprOr &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprGT(ASTExprGT &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprGE(ASTExprGE &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprLT(ASTExprLT &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprLE(ASTExprLE &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprEQ(ASTExprEQ &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprNE(ASTExprNE &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprPlus(ASTExprPlus &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprMinus(ASTExprMinus &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprTimes(ASTExprTimes &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprDivide(ASTExprDivide &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprNot(ASTExprNot &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprArrow(ASTExprArrow &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprArray(ASTExprArray &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprNegate(ASTExprNegate &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseNumConstant(ASTNumConstant &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseFuncCall(ASTFuncCall &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseBoolConstant(ASTBoolConstant &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseBlock(ASTBlock &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtAssign(ASTStmtAssign &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprDot(ASTExprDot &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtFor(ASTStmtFor &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtIf(ASTStmtIf &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtIfElse(ASTStmtIfElse &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtReturn(ASTStmtReturn &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtReturnVal(ASTStmtReturnVal &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtEmpty(ASTStmtEmpty &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseScript(ASTScript &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtWhile(ASTStmtWhile &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtDo(ASTStmtDo &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprBitOr(ASTExprBitOr &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprBitXor(ASTExprBitXor &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprBitAnd(ASTExprBitAnd &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprLShift(ASTExprLShift &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprRShift(ASTExprRShift &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprBitNot(ASTExprBitNot &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprIncrement(ASTExprIncrement &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprPreIncrement(ASTExprPreIncrement &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprDecrement(ASTExprDecrement &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprPreDecrement(ASTExprPreDecrement &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseExprModulo(ASTExprModulo &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtBreak(ASTStmtBreak &, void *param)
    {
        caseDefault(param);
    }
    virtual void caseStmtContinue(ASTStmtContinue &, void *param)
    {
        caseDefault(param);
    }
    virtual ~ASTVisitor() {}
};

//////////////////////////////////////////////////////////////////////////////
class LocationData
{
public:
    LocationData(YYLTYPE loc)
    {
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
    AST(LocationData Loc) : loc(Loc) {}
    virtual void execute(ASTVisitor &visitor, void *param)=0;
    virtual ~AST() {}
    LocationData &getLocation()
    {
        return loc;
    }
private:
    LocationData loc;
    //NOT IMPLEMENTED - do not use
    AST(AST &other);
    AST &operator=(AST &other);
};

class ASTStmt : public AST
{
public:
    ASTStmt(LocationData Loc) : AST(Loc) {}
    virtual ~ASTStmt() {}
};

class ASTProgram : public AST
{
public:
    ASTProgram(ASTDeclList *Decls, LocationData Loc) : AST(Loc), decls(Decls) {};
    
    ASTDeclList *getDeclarations()
    {
        return decls;
    }
    
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
    ASTFloat(char *Value, int Type, LocationData Loc) : AST(Loc), type(Type), negative(false)
    {
        val = string(Value);
    }
    ASTFloat(const char *Value, int Type, LocationData Loc) : AST(Loc), type(Type), negative(false)
    {
        val = string(Value);
    }
    ASTFloat(long Value, int Type, LocationData Loc) : AST(Loc), type(Type), negative(false)
    {
        char tmp[15];
        sprintf(tmp, "%ld", Value);
        val = string(tmp);
    }
    string getValue()
    {
        return val;
    }
    pair<string,string> parseValue();
    int getType()
    {
        return type;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseFloat(*this, param);
    }
    void set_negative(bool neg)
    {
        negative = neg;
    }
    static const int TYPE_DECIMAL=0;
    static const int TYPE_BINARY=1;
    static const int TYPE_HEX=2;
private:
    int type;
    string val;
    bool negative;
};

class ASTString : public AST
{
public:
    ASTString(const char *strval, LocationData Loc) : AST(Loc)
    {
        str = string(strval);
    }
    string getValue()
    {
        return str;
    }
    
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
    ASTDeclList(LocationData Loc) : AST(Loc), decls() {}
    ~ASTDeclList();
    
    void addDeclaration(ASTDecl *newdecl);
    
    list<ASTDecl *> &getDeclarations()
    {
        return decls;
    }
    
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
    ASTDecl(LocationData Loc) : ASTStmt(Loc) {}
    virtual ~ASTDecl() {}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTDecl(ASTDecl &);
    ASTDecl &operator=(ASTDecl &);
};

class ASTImportDecl : public ASTDecl
{
public:
    ASTImportDecl(string file, LocationData Loc) : ASTDecl(Loc), filename(file) {}
    
    string getFilename()
    {
        return filename;
    }
    
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseImportDecl(*this,param);
    }
private:
    string filename;
    //NOT IMPLEMENTED; DO NOT USE
    ASTImportDecl(ASTImportDecl &);
    ASTImportDecl &operator=(ASTImportDecl &);
};

class ASTConstDecl : public ASTDecl
{
public:
    ASTConstDecl(string Name, ASTFloat *Val, LocationData Loc) : ASTDecl(Loc), name(Name), val(Val) {}
    string getName()
    {
        return name;
    }
    ASTFloat *getValue()
    {
        return val;
    }
    ~ASTConstDecl()
    {
        delete val;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseConstDecl(*this,param);
    }
private:
    string name;
    ASTFloat *val;
    //NOT IMPLEMENTED; DO NOT USE
    ASTConstDecl(ASTConstDecl &);
    ASTConstDecl &operator=(ASTConstDecl &);
};

class ASTFuncDecl : public ASTDecl
{
public:
    ASTFuncDecl(LocationData Loc) : ASTDecl(Loc), params()
    {
    
    }
    
    void setName(string n)
    {
        name=n;
    }
    void setReturnType(ASTType *type)
    {
        rettype=type;
    }
    void setBlock(ASTBlock *b)
    {
        block=b;
    }
    
    ~ASTFuncDecl();
    void addParam(ASTVarDecl *param);
    list<ASTVarDecl *> &getParams()
    {
        return params;
    }
    ASTType *getReturnType()
    {
        return rettype;
    }
    ASTBlock *getBlock()
    {
        return block;
    }
    string getName()
    {
        return name;
    }
    
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

class ASTArrayDecl : public ASTDecl
{
public:
    ASTArrayDecl(ASTType *Type, string Name, AST *Size, bool isReg, ASTArrayList *List, LocationData Loc) :
        ASTDecl(Loc), name(Name), list(List), size(Size), type(Type), reg(isReg) { }
    ~ASTArrayDecl();
    ASTType *getType()
    {
        return type;
    }
    string getName()
    {
        return name;
    }
    //If reg, size is an ASTExpr. If not, it's an ASTFloat
    AST *getSize()
    {
        return size;
    }
    bool isRegister()
    {
        return reg;
    }
    ASTArrayList *getList()
    {
        return list;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseArrayDecl(*this,param);
    }
private:
    string name;
    ASTArrayList *list;
    AST *size;
    ASTType *type;
    
    bool reg;
    
    //NOT IMPLEMENTED; DO NOT USE
    ASTArrayDecl(ASTArrayDecl &);
    ASTArrayDecl &operator=(ASTArrayDecl &);
};

class ASTArrayList : public AST
{
public:
    ASTArrayList(LocationData Loc) : AST(Loc), listIsString(false) {}
    ~ASTArrayList();
    list<ASTExpr *> &getList()
    {
        return exprs;
    }
    void addParam(ASTExpr *expr);
    bool isString()
    {
        return listIsString;
    }
    void makeString()
    {
        listIsString = true;
    }
    //Just to allow us to instantiate the object
    void execute(ASTVisitor &, void *) { }
private:
    list<ASTExpr *> exprs;
    bool listIsString;
    
    //NOT IMPLEMENTED
    ASTArrayList(ASTArrayList &);
    ASTArrayList &operator=(ASTArrayList &);
};

class ASTType : public AST
{
public:
    ASTType(LocationData Loc) : AST(Loc) {}
    virtual void execute(ASTVisitor &visitor, void *param)=0;
};

class ASTTypeFloat : public ASTType
{
public:
    ASTTypeFloat(LocationData Loc) : ASTType(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseTypeFloat(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTTypeFloat(ASTTypeFloat &);
    ASTTypeFloat &operator=(ASTTypeFloat &);
};

class ASTTypeBool : public ASTType
{
public:
    ASTTypeBool(LocationData Loc) : ASTType(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseTypeBool(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTTypeBool(ASTTypeBool &);
    ASTTypeBool &operator =(ASTTypeBool &);
};

class ASTTypeVoid : public ASTType
{
public:
    ASTTypeVoid(LocationData Loc) : ASTType(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseTypeVoid(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTTypeVoid(ASTTypeVoid &);
    ASTTypeVoid &operator=(ASTTypeVoid &);
};

class ASTTypeFFC : public ASTType
{
public:
    ASTTypeFFC(LocationData Loc) : ASTType(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseTypeFFC(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTTypeFFC(ASTTypeFFC &);
    ASTTypeFFC &operator=(ASTTypeFFC &);
};

class ASTTypeGlobal : public ASTType
{
public:
    ASTTypeGlobal(LocationData Loc) : ASTType(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseTypeGlobal(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTTypeGlobal(ASTTypeGlobal &);
    ASTTypeGlobal &operator=(ASTTypeGlobal &);
};

class ASTTypeItem : public ASTType
{
public:
    ASTTypeItem(LocationData Loc) : ASTType(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseTypeItem(*this,param);
    }
private:
    ASTTypeItem(ASTTypeItem &);
    ASTTypeItem &operator=(ASTTypeItem &);
};

class ASTTypeItemclass : public ASTType
{
public:
    ASTTypeItemclass(LocationData Loc) : ASTType(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseTypeItemclass(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTTypeItemclass(ASTTypeItemclass &);
    ASTTypeItemclass &operator=(ASTTypeItemclass &);
};

class ASTTypeNPC : public ASTType
{
public:
    ASTTypeNPC(LocationData Loc) : ASTType(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseTypeNPC(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTTypeNPC(ASTTypeNPC &);
    ASTTypeNPC &operator=(ASTTypeNPC &);
};

class ASTTypeLWpn : public ASTType
{
public:
    ASTTypeLWpn(LocationData Loc) : ASTType(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseTypeLWpn(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTTypeLWpn(ASTTypeLWpn &);
    ASTTypeLWpn &operator=(ASTTypeLWpn &);
};

class ASTTypeEWpn : public ASTType
{
public:
    ASTTypeEWpn(LocationData Loc) : ASTType(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseTypeEWpn(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTTypeEWpn(ASTTypeEWpn &);
    ASTTypeEWpn &operator=(ASTTypeEWpn &);
};

class ASTVarDecl : public ASTDecl
{
public:
    ASTVarDecl(ASTType *Type, string Name, LocationData Loc) : ASTDecl(Loc), type(Type), name(Name) {}
    ASTType *getType()
    {
        return type;
    }
    string getName()
    {
        return name;
    }
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
    ASTVarDeclInitializer(ASTType *Type, string Name, ASTExpr *Initial, LocationData Loc) :
        ASTVarDecl(Type,Name,Loc), initial(Initial) {}
    ~ASTVarDeclInitializer();
    ASTExpr *getInitializer()
    {
        return initial;
    }
    
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseVarDeclInitializer(*this,param);
    }
private:
    ASTExpr *initial;
    //NOT IMPLEMENTED; DO NOT USE
    ASTVarDeclInitializer(ASTVarDeclInitializer &);
    ASTVarDeclInitializer &operator=(ASTVarDeclInitializer &);
};

class ASTExpr : public ASTStmt
{
public:
    ASTExpr(LocationData Loc) : ASTStmt(Loc), hasval(false), intval(0), type(-1) {}
    virtual ~ASTExpr() {}
    long getIntValue()
    {
        return intval;
    }
    bool hasIntValue()
    {
        return hasval;
    }
    int getType()
    {
        return type;
    }
    void setIntValue(long val)
    {
        hasval = true;
        intval=val;
    }
    void setType(int t)
    {
        type=t;
    }
private:
    bool hasval;
    long intval;
    int type;
    //NOT IMPLEMENTED; DO NOT USE
    ASTExpr(ASTExpr &);
    ASTExpr &operator=(ASTExpr &);
};

class ASTUnaryExpr : public ASTExpr
{
public:
    ASTUnaryExpr(LocationData Loc) : ASTExpr(Loc) {}
    virtual ~ASTUnaryExpr()
    {
        delete operand;
    }
    
    void setOperand(ASTExpr *e)
    {
        operand=e;
    }
    ASTExpr *getOperand()
    {
        return operand;
    }
private:
    ASTExpr *operand;
    //NOT IMPLEMENTED; DO NOT USE
    ASTUnaryExpr(ASTUnaryExpr &);
    ASTUnaryExpr&operator=(ASTUnaryExpr&);
};

class ASTBinaryExpr : public ASTExpr
{
public:
    ASTBinaryExpr(LocationData Loc) : ASTExpr(Loc) {}
    virtual ~ASTBinaryExpr()
    {
        delete first;
        delete second;
    }
    void setFirstOperand(ASTExpr *e)
    {
        first=e;
    }
    void setSecondOperand(ASTExpr *e)
    {
        second=e;
    }
    ASTExpr *getFirstOperand()
    {
        return first;
    }
    ASTExpr *getSecondOperand()
    {
        return second;
    }
private:
    ASTExpr *first;
    ASTExpr *second;
};

class ASTLogExpr : public ASTBinaryExpr
{
public:
    ASTLogExpr(LocationData Loc) : ASTBinaryExpr(Loc) {}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTLogExpr(ASTLogExpr &);
    ASTLogExpr &operator=(ASTLogExpr &);
};

class ASTBitExpr : public ASTBinaryExpr
{
public:
    ASTBitExpr(LocationData Loc) : ASTBinaryExpr(Loc) {}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTBitExpr &operator=(ASTBitExpr &);
    ASTBitExpr(ASTBitExpr &);
};

class ASTShiftExpr : public ASTBinaryExpr
{
public:
    ASTShiftExpr(LocationData Loc) : ASTBinaryExpr(Loc) {}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTShiftExpr(ASTShiftExpr &);
    ASTShiftExpr &operator=(ASTShiftExpr &);
};

class ASTExprAnd : public ASTLogExpr
{
public:
    ASTExprAnd(LocationData Loc) : ASTLogExpr(Loc) {}
    
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprAnd(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTExprAnd(ASTExprAnd &);
    ASTExprAnd &operator=(ASTExprAnd &);
};

class ASTExprOr : public ASTLogExpr
{
public:
    ASTExprOr(LocationData Loc) : ASTLogExpr(Loc) {}
    
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprOr(*this,param);
    }
};

class ASTRelExpr : public ASTBinaryExpr
{
public:
    ASTRelExpr(LocationData Loc) : ASTBinaryExpr(Loc) {}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTRelExpr(ASTRelExpr &);
    ASTRelExpr &operator=(ASTRelExpr&);
};

class ASTExprGT : public ASTRelExpr
{
public:
    ASTExprGT(LocationData Loc) : ASTRelExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprGT(*this, param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTExprGT(ASTExprGT &);
    ASTExprGT &operator=(ASTExprGT &);
};

class ASTExprGE : public ASTRelExpr
{
public:
    ASTExprGE(LocationData Loc) : ASTRelExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprGE(*this, param);
    }
};

class ASTExprLT : public ASTRelExpr
{
public:
    ASTExprLT(LocationData Loc) : ASTRelExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprLT(*this, param);
    }
};

class ASTExprLE : public ASTRelExpr
{
public:
    ASTExprLE(LocationData Loc) : ASTRelExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprLE(*this, param);
    }
};

class ASTExprEQ : public ASTRelExpr
{
public:
    ASTExprEQ(LocationData Loc) : ASTRelExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprEQ(*this, param);
    }
};

class ASTExprNE : public ASTRelExpr
{
public:
    ASTExprNE(LocationData Loc) : ASTRelExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprNE(*this, param);
    }
};

class ASTAddExpr : public ASTBinaryExpr
{
public:
    ASTAddExpr(LocationData Loc) : ASTBinaryExpr(Loc) {}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTAddExpr(ASTAddExpr &);
    ASTAddExpr &operator=(ASTAddExpr &);
};

class ASTExprPlus : public ASTAddExpr
{
public:
    ASTExprPlus(LocationData Loc) : ASTAddExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprPlus(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTExprPlus(ASTExprPlus &);
    ASTExprPlus &operator=(ASTExprPlus &);
};

class ASTExprMinus : public ASTAddExpr
{
public:
    ASTExprMinus(LocationData Loc) : ASTAddExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprMinus(*this, param);
    }
};

class ASTMultExpr : public ASTBinaryExpr
{
public:
    ASTMultExpr(LocationData Loc) : ASTBinaryExpr(Loc) {}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTMultExpr(ASTMultExpr &);
    ASTMultExpr &operator=(ASTMultExpr &);
};

class ASTExprTimes : public ASTMultExpr
{
public:
    ASTExprTimes(LocationData Loc) : ASTMultExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprTimes(*this, param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTExprTimes(ASTExprTimes &);
    ASTExprTimes &operator=(ASTExprTimes &);
};

class ASTExprDivide : public ASTMultExpr
{
public:
    ASTExprDivide(LocationData Loc) : ASTMultExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprDivide(*this,param);
    }
};

class ASTExprBitOr : public ASTBitExpr
{
public:
    ASTExprBitOr(LocationData Loc) : ASTBitExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprBitOr(*this,param);
    }
};

class ASTExprBitXor : public ASTBitExpr
{
public:
    ASTExprBitXor(LocationData Loc) : ASTBitExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprBitXor(*this,param);
    }
};

class ASTExprBitAnd : public ASTBitExpr
{
public:
    ASTExprBitAnd(LocationData Loc) : ASTBitExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprBitAnd(*this,param);
    }
};

class ASTExprLShift : public ASTShiftExpr
{
public:
    ASTExprLShift(LocationData Loc) : ASTShiftExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprLShift(*this,param);
    }
};

class ASTExprRShift : public ASTShiftExpr
{
public:
    ASTExprRShift(LocationData Loc) : ASTShiftExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprRShift(*this,param);
    }
};

class ASTExprNegate : public ASTUnaryExpr
{
public:
    ASTExprNegate(LocationData Loc) : ASTUnaryExpr(Loc) {}
    
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprNegate(*this,param);
    }
};

class ASTExprNot : public ASTUnaryExpr
{
public:
    ASTExprNot(LocationData Loc) : ASTUnaryExpr(Loc) {}
    
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprNot(*this,param);
    }
};

class ASTExprBitNot : public ASTUnaryExpr
{
public:
    ASTExprBitNot(LocationData Loc) : ASTUnaryExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprBitNot(*this,param);
    }
};

class ASTExprIncrement : public ASTUnaryExpr
{
public:
    ASTExprIncrement(LocationData Loc) : ASTUnaryExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprIncrement(*this,param);
    }
};

class ASTExprPreIncrement : public ASTUnaryExpr
{
public:
    ASTExprPreIncrement(LocationData Loc) : ASTUnaryExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprPreIncrement(*this,param);
    }
};

class ASTExprDecrement : public ASTUnaryExpr
{
public:
    ASTExprDecrement(LocationData Loc) : ASTUnaryExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprDecrement(*this,param);
    }
};

class ASTExprPreDecrement : public ASTUnaryExpr
{
public:
    ASTExprPreDecrement(LocationData Loc) : ASTUnaryExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprPreDecrement(*this,param);
    }
};

class ASTExprModulo : public ASTMultExpr
{
public:
    ASTExprModulo(LocationData Loc) : ASTMultExpr(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprModulo(*this,param);
    }
};

class ASTNumConstant : public ASTExpr
{
public:
    ASTNumConstant(ASTFloat *value, LocationData Loc) : ASTExpr(Loc), val(value) {}
    
    ASTFloat *getValue()
    {
        return val;
    }
    
    ~ASTNumConstant()
    {
        delete val;
    }
    
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
    ASTFuncCall(LocationData Loc) : ASTExpr(Loc), params() {}
    
    list<ASTExpr *> &getParams()
    {
        return params;
    }
    
    void setName(ASTExpr *n)
    {
        name=n;
    }
    ASTExpr * getName()
    {
        return name;
    }
    void addParam(ASTExpr *param)
    {
        params.push_front(param);
    }
    
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
    ASTBoolConstant(bool Value, LocationData Loc) : ASTExpr(Loc), value(Value) {}
    bool getValue()
    {
        return value;
    }
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
    ASTBlock(LocationData Loc) : ASTStmt(Loc), statements() {}
    ~ASTBlock();
    
    list<ASTStmt *> &getStatements()
    {
        return statements;
    }
    list<long> *getArrayRefs()
    {
        return &arrayRefs;
    }
    void addStatement(ASTStmt *Stmt);
    
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseBlock(*this,param);
    }
private:
    list<ASTStmt *> statements;
    list<long> arrayRefs;
    //NOT IMPLEMENTED; DO NOT USE
    ASTBlock(ASTBlock &);
    ASTBlock &operator=(ASTBlock &);
};

class ASTStmtAssign : public ASTStmt
{
public:
    ASTStmtAssign(ASTStmt *Lval, ASTExpr *Rval, LocationData Loc) : ASTStmt(Loc), lval(Lval), rval(Rval) {}
    ASTStmt *getLVal()
    {
        return lval;
    }
    ASTExpr *getRVal()
    {
        return rval;
    }
    ~ASTStmtAssign()
    {
        delete lval;
        delete rval;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseStmtAssign(*this,param);
    }
private:
    ASTStmt *lval;
    ASTExpr *rval;
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtAssign(ASTStmtAssign &);
    ASTStmtAssign &operator=(ASTStmtAssign &);
};

class ASTExprDot : public ASTExpr
{
public:
    ASTExprDot(string Nspace, string Name, LocationData Loc) : ASTExpr(Loc), name(Name), nspace(Nspace) {}
    string getName()
    {
        return name;
    }
    string getNamespace()
    {
        return nspace;
    }
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
    ASTExprArrow(ASTExpr *Lval, string Rval, LocationData Loc) : ASTExpr(Loc), lval(Lval), rval(Rval), index(NULL) {}
    string getName()
    {
        return rval;
    }
    ASTExpr *getLVal()
    {
        return lval;
    }
    ~ASTExprArrow()
    {
        delete lval;
        
        if(index) delete index;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprArrow(*this,param);
    }
    ASTExpr *getIndex()
    {
        return index;
    }
    void setIndex(ASTExpr *e)
    {
        index = e;
    }
private:
    ASTExpr *lval;
    string rval;
    ASTExpr *index;
};

class ASTExprArray : public ASTExpr
{
public:
    ASTExprArray(string Nspace, string Name, LocationData Loc) : ASTExpr(Loc), name(Name), index(NULL), nspace(Nspace) {}
    string getName()
    {
        return name;
    }
    string getNamespace()
    {
        return nspace;
    }
    ~ASTExprArray()
    {
        if(index) delete index;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseExprArray(*this,param);
    }
    ASTExpr *getIndex()
    {
        return index;
    }
    void setIndex(ASTExpr *e)
    {
        index = e;
    }
private:
    string name;
    ASTExpr *index;
    string nspace;
};

class ASTStmtFor : public ASTStmt
{
public:
    ASTStmtFor(ASTStmt *Prec, ASTExpr *Term, ASTStmt *Incr, ASTStmt *Stmt, LocationData Loc) :
        ASTStmt(Loc), prec(Prec), term(Term), incr(Incr), stmt(Stmt) {}
    ~ASTStmtFor()
    {
        delete prec;
        delete term;
        delete incr;
        delete stmt;
    }
    ASTStmt *getPrecondition()
    {
        return prec;
    }
    ASTExpr *getTerminationCondition()
    {
        return term;
    }
    ASTStmt *getIncrement()
    {
        return incr;
    }
    ASTStmt *getStmt()
    {
        return stmt;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        return visitor.caseStmtFor(*this,param);
    }
private:
    ASTStmt *prec;
    ASTExpr *term;
    ASTStmt *incr;
    ASTStmt *stmt;
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtFor(ASTStmtFor &);
    ASTStmtFor &operator=(ASTStmtFor &);
};

class ASTStmtIf : public ASTStmt
{
public:
    ASTStmtIf(ASTExpr *Cond, ASTStmt *Stmt, LocationData Loc) : ASTStmt(Loc), cond(Cond), stmt(Stmt) {}
    ASTExpr *getCondition()
    {
        return cond;
    }
    ASTStmt *getStmt()
    {
        return stmt;
    }
    virtual ~ASTStmtIf()
    {
        delete cond;
        delete stmt;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        return visitor.caseStmtIf(*this,param);
    }
private:
    ASTExpr *cond;
    ASTStmt *stmt;
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtIf(ASTStmtIf &);
    ASTStmtIf &operator=(ASTStmtIf &);
};

class ASTStmtIfElse : public ASTStmtIf
{
public:
    ASTStmtIfElse(ASTExpr *Cond, ASTStmt *Ifstmt, ASTStmt *Elsestmt, LocationData Loc) :
        ASTStmtIf(Cond,Ifstmt,Loc), elsestmt(Elsestmt) {}
    ~ASTStmtIfElse()
    {
        delete elsestmt;
    }
    ASTStmt *getElseStmt()
    {
        return elsestmt;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseStmtIfElse(*this, param);
    }
private:
    ASTStmt *elsestmt;
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtIfElse(ASTStmtIfElse &);
    ASTStmtIfElse &operator=(ASTStmtIfElse &);
};

class ASTStmtReturn : public ASTStmt
{
public:
    ASTStmtReturn(LocationData Loc) : ASTStmt(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseStmtReturn(*this, param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtReturn(ASTStmtReturn &);
    ASTStmtReturn &operator=(ASTStmtReturn &);
};

class ASTStmtReturnVal : public ASTStmt
{
public:
    ASTStmtReturnVal(ASTExpr *Retval, LocationData Loc) : ASTStmt(Loc), retval(Retval) {}
    ASTExpr *getReturnValue()
    {
        return retval;
    }
    ~ASTStmtReturnVal()
    {
        delete retval;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseStmtReturnVal(*this,param);
    }
private:
    ASTExpr *retval;
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtReturnVal(ASTStmtReturnVal &);
    ASTStmtReturnVal &operator=(ASTStmtReturnVal &);
};

class ASTStmtBreak : public ASTStmt
{
public:
    ASTStmtBreak(LocationData Loc) : ASTStmt(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseStmtBreak(*this,param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtBreak(ASTStmtBreak &);
    ASTStmtBreak &operator=(ASTStmtBreak &);
};

class ASTStmtContinue : public ASTStmt
{
public:
    ASTStmtContinue(LocationData Loc) : ASTStmt(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseStmtContinue(*this, param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtContinue(ASTStmtContinue &);
    ASTStmtContinue &operator=(ASTStmtContinue &);
};

class ASTStmtEmpty : public ASTStmt
{
public:
    ASTStmtEmpty(LocationData Loc) : ASTStmt(Loc) {}
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseStmtEmpty(*this, param);
    }
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtEmpty(ASTStmtEmpty &);
    ASTStmtEmpty &operator=(ASTStmtEmpty&);
};

class ASTScript : public ASTDecl
{
public:
    ASTScript(ASTType *Type, string Name, ASTDeclList *Sblock, LocationData Loc) : ASTDecl(Loc), type(Type), name(Name), sblock(Sblock) {}
    ~ASTScript();
    ASTDeclList *getScriptBlock()
    {
        return sblock;
    }
    ASTType *getType()
    {
        return type;
    }
    string getName()
    {
        return name;
    }
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
    ASTStmtWhile(ASTExpr *Cond, ASTStmt *Stmt, LocationData Loc) : ASTStmt(Loc), cond(Cond), stmt(Stmt) {}
    ~ASTStmtWhile()
    {
        delete cond;
        delete stmt;
    }
    ASTExpr *getCond()
    {
        return cond;
    }
    ASTStmt *getStmt()
    {
        return stmt;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseStmtWhile(*this,param);
    }
private:
    ASTExpr *cond;
    ASTStmt *stmt;
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtWhile(ASTStmtWhile &);
    ASTStmtWhile &operator=(ASTStmtWhile &);
};

class ASTStmtDo : public ASTStmt
{
public:
    ASTStmtDo(ASTExpr *Cond, ASTStmt *Stmt, LocationData Loc) : ASTStmt(Loc), cond(Cond), stmt(Stmt) {}
    ~ASTStmtDo()
    {
        delete cond;
        delete stmt;
    }
    ASTExpr *getCond()
    {
        return cond;
    }
    ASTStmt *getStmt()
    {
        return stmt;
    }
    void execute(ASTVisitor &visitor, void *param)
    {
        visitor.caseStmtDo(*this,param);
    }
private:
    ASTExpr *cond;
    ASTStmt *stmt;
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtDo(ASTStmtDo &);
    ASTStmtDo &operator=(ASTStmtDo &);
};
#endif


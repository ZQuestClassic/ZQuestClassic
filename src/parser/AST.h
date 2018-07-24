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
int go(const char *f);

// AST Subclasses.
class AST; // virtual
class ASTProgram;
class ASTFloat;
class ASTString;
// Statements
class ASTStmt; // virtual
class ASTBlock;
class ASTStmtIf;
class ASTStmtIfElse;
class ASTStmtSwitch;
class ASTSwitchCases;
class ASTStmtFor;
class ASTStmtWhile;
class ASTStmtDo;
class ASTStmtReturn;
class ASTStmtReturnVal;
class ASTStmtBreak;
class ASTStmtContinue;
class ASTStmtEmpty;
class ASTCompileError;
// Declarations
class ASTDecl; // virtual
class ASTScript;
class ASTImportDecl;
class ASTFuncDecl;
class ASTDataDeclList;
class ASTDataDecl;
class ASTDataDeclExtraArray;
class ASTTypeDef;
// Expressions
class ASTExpr; // virtual
class ASTExprConst;
class ASTExprAssign;
class ASTExprIdentifier;
class ASTExprArrow;
class ASTExprIndex;
class ASTExprCall;
class ASTUnaryExpr; // virtual
class ASTExprNegate;
class ASTExprNot;
class ASTExprBitNot;
class ASTExprIncrement;
class ASTExprPreIncrement;
class ASTExprDecrement;
class ASTExprPreDecrement;
class ASTBinaryExpr; // virtual
class ASTLogExpr; // virtual
class ASTExprAnd;
class ASTExprOr;
class ASTRelExpr; // virtual
class ASTExprGT;
class ASTExprGE;
class ASTExprLT;
class ASTExprLE;
class ASTExprEQ;
class ASTExprNE;
class ASTAddExpr; // virtual
class ASTExprPlus;
class ASTExprMinus;
class ASTMultExpr; // virtual
class ASTExprTimes;
class ASTExprDivide;
class ASTExprModulo;
class ASTBitExpr; // virtual
class ASTExprBitAnd;
class ASTExprBitOr;
class ASTExprBitXor;
class ASTShiftExpr; // virtual
class ASTExprLShift;
class ASTExprRShift;
// Literals
class ASTLiteral; // virtual
class ASTNumberLiteral;
class ASTBoolLiteral;
class ASTStringLiteral;
class ASTArrayLiteral;
// Types
class ASTScriptType;
class ASTVarType;

class ASTVisitor
{
public:
    virtual ~ASTVisitor() {}
    virtual void caseDefault(void *param) = 0;
	// AST Subclasses
    virtual void caseProgram(ASTProgram&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseFloat(ASTFloat&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseString(ASTString&, void* param = NULL) {
		caseDefault(param);}
	// Statements
    virtual void caseBlock(ASTBlock&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtIf(ASTStmtIf&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtIfElse(ASTStmtIfElse&, void* param = NULL) {
		caseDefault(param);}
	virtual void caseStmtSwitch(ASTStmtSwitch&, void* param = NULL) {
		caseDefault(param);}
	virtual void caseSwitchCases(ASTSwitchCases&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtFor(ASTStmtFor&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtWhile(ASTStmtWhile&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtDo(ASTStmtDo&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtReturn(ASTStmtReturn&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtReturnVal(ASTStmtReturnVal&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtBreak(ASTStmtBreak&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtContinue(ASTStmtContinue&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStmtEmpty(ASTStmtEmpty&, void* param = NULL) {
		caseDefault(param);}
	virtual void caseCompileError(ASTCompileError& node, void* param = NULL) {
		caseDefault(param);}
	// Declarations
    virtual void caseScript(ASTScript&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseImportDecl(ASTImportDecl&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseFuncDecl(ASTFuncDecl&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseDataDeclList(ASTDataDeclList&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseDataDecl(ASTDataDecl&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseDataDeclExtraArray(
			ASTDataDeclExtraArray&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseTypeDef(ASTTypeDef&, void* param = NULL) {
		caseDefault(param);}
	// Expressions
    virtual void caseExprConst(ASTExprConst&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprAssign(ASTExprAssign&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprIdentifier(ASTExprIdentifier&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprArrow(ASTExprArrow&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprIndex(ASTExprIndex&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprCall(ASTExprCall&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprNegate(ASTExprNegate&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprNot(ASTExprNot&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprBitNot(ASTExprBitNot&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprIncrement(ASTExprIncrement&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprPreIncrement(
			ASTExprPreIncrement&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprDecrement(ASTExprDecrement&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprPreDecrement(ASTExprPreDecrement&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprAnd(ASTExprAnd&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprOr(ASTExprOr&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprGT(ASTExprGT&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprGE(ASTExprGE&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprLT(ASTExprLT&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprLE(ASTExprLE&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprEQ(ASTExprEQ&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprNE(ASTExprNE&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprPlus(ASTExprPlus&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprMinus(ASTExprMinus&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprTimes(ASTExprTimes&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprDivide(ASTExprDivide&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprModulo(ASTExprModulo&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprBitAnd(ASTExprBitAnd&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprBitOr(ASTExprBitOr&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprBitXor(ASTExprBitXor&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprLShift(ASTExprLShift&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseExprRShift(ASTExprRShift&, void* param = NULL) {
		caseDefault(param);}
	// Literals
    virtual void caseNumberLiteral(ASTNumberLiteral&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseBoolLiteral(ASTBoolLiteral&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseStringLiteral(ASTStringLiteral&, void* param = NULL) {
		caseDefault(param);}
	virtual void caseArrayLiteral(ASTArrayLiteral& node, void* param = NULL) {
		caseDefault(param);}
	// Types
	virtual void caseScriptType(ASTScriptType&, void* param = NULL) {
		caseDefault(param);}
    virtual void caseVarType(ASTVarType&, void* param = NULL) {
		caseDefault(param);}
};

//////////////////////////////////////////////////////////////////////////////
class LocationData
{
public:
	LocationData()
			: first_line(-1), last_line(-1),
			  first_column(-1), last_column(-1),
			  fname(curfilename)
	{}

    LocationData(YYLTYPE loc)
			: first_line(loc.first_line), last_line(loc.last_line),
			  first_column(loc.first_column), last_column(loc.last_column),
			  fname(curfilename)
	{}

    int first_line;
    int last_line;
    int first_column;
    int last_column;
    string fname;

	static LocationData const NONE;
};

////////////////////////////////////////////////////////////////

class AST
{
public:
    AST(LocationData const& location) : loc(location) {}
	AST(AST const& base) : loc(base.loc) {}
	AST& operator=(AST const& rhs);

	virtual AST* clone() const = 0;
    LocationData const &getLocation() const {return loc;}
    LocationData &getLocation() {return loc;}
	void updateLocation(LocationData const& location) {loc = location;}
	virtual void execute(ASTVisitor &visitor, void* param = NULL) = 0;

	virtual string asString() const {return "unknown";}

	// Subclass Predicates (replacing typeof and such)
	virtual bool isTypeArrow() const {return false;}
	virtual bool isTypeIndex() const {return false;}
	virtual bool isTypeIdentifier() const {return false;}
	virtual bool isTypeVarDecl() const {return false;}
	virtual bool isTypeArrayDecl() const {return false;}
	virtual bool isStringLiteral() const {return false;}
	virtual bool isArrayLiteral() const {return false;}
private:
    LocationData loc;
};

class ASTProgram : public AST
{
public:
    ASTProgram(LocationData const& location);
	ASTProgram(ASTProgram const& base);
    ~ASTProgram();
	ASTProgram* clone() const {return new ASTProgram(*this);}
    
    void execute(ASTVisitor &visitor, void *param) {visitor.caseProgram(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseProgram(*this);}

	void addDeclaration(ASTDecl* declaration);
	ASTProgram& merge(ASTProgram& other);

	// Public since we'll be clearing them and such.
	vector<ASTImportDecl*> imports;
	vector<ASTDataDeclList*> variables;
	vector<ASTFuncDecl*> functions;
	vector<ASTTypeDef*> types;
	vector<ASTScript*> scripts;
};

class ASTFloat : public AST
{
public:
    ASTFloat(char *Value, int Type, LocationData Loc);
    ASTFloat(const char *Value, int Type, LocationData Loc);
    ASTFloat(string Value, int Type, LocationData Loc);
    ASTFloat(long Value, int Type, LocationData Loc);
	ASTFloat* clone() const;
    string getValue() const {return val;}
    pair<string,string> parseValue();
    int getType() const {return type;}
    void execute(ASTVisitor &visitor, void *param) {visitor.caseFloat(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseFloat(*this);}
    void set_negative(bool neg) {negative = neg;}
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
	ASTString(const char *strval, LocationData Loc);
    ASTString(string Str, LocationData Loc);
	ASTString* clone() const;
    string getValue() const {return str;}
    void execute(ASTVisitor &visitor, void *param) {visitor.caseString(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseString(*this);}
private:
    string str;
};

////////////////////////////////////////////////////////////////
// Statements

class ASTStmt : public AST
{
public:
    ASTStmt(LocationData const& location) : AST(location) {}
	ASTStmt(ASTStmt const& base) : AST(base) {}
	ASTStmt& operator=(ASTStmt const& rhs);
	virtual ASTStmt* clone() const = 0;
};
    
class ASTBlock : public ASTStmt
{
public:
    ASTBlock(LocationData Loc) : ASTStmt(Loc), statements() {}
    ~ASTBlock();
	ASTBlock* clone() const;
    
    list<ASTStmt *> const &getStatements() const {return statements;}
    list<ASTStmt *> &getStatements() {return statements;}
    void addStatement(ASTStmt *Stmt);
    void execute(ASTVisitor &visitor, void *param) {visitor.caseBlock(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseBlock(*this);}
private:
    list<ASTStmt *> statements;
};
    
class ASTStmtIf : public ASTStmt
    {
public:
    ASTStmtIf(ASTExpr *Cond, ASTStmt *Stmt, LocationData Loc) : ASTStmt(Loc), cond(Cond), stmt(Stmt) {}
    virtual ~ASTStmtIf();
	ASTStmtIf* clone() const;

    ASTExpr *getCondition() const {return cond;}
    ASTStmt *getStmt() const {return stmt;}
    void execute(ASTVisitor &visitor, void *param) {return visitor.caseStmtIf(*this, param);}
    void execute(ASTVisitor &visitor) {return visitor.caseStmtIf(*this);}
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
    ASTStmtIfElse(ASTExpr *Cond, ASTStmt *Ifstmt, ASTStmt *Elsestmt, LocationData Loc);
    ~ASTStmtIfElse();
	ASTStmtIfElse* clone() const;

    ASTStmt *getElseStmt() const {return elsestmt;}
    void execute(ASTVisitor &visitor, void *param) {visitor.caseStmtIfElse(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseStmtIfElse(*this);}
private:
    ASTStmt *elsestmt;
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtIfElse(ASTStmtIfElse &);
    ASTStmtIfElse &operator=(ASTStmtIfElse &);
};

class ASTStmtSwitch : public ASTStmt
{
public:
	ASTStmtSwitch(LocationData Loc) : ASTStmt(Loc), key(NULL), cases() {}
	~ASTStmtSwitch();
	ASTStmtSwitch* clone() const;

	void execute(ASTVisitor& visitor, void* param) {visitor.caseStmtSwitch(*this, param);}
	void execute(ASTVisitor& visitor) {visitor.caseStmtSwitch(*this);}
	void setKey(ASTExpr* k) {key = k;}
	ASTExpr* getKey() const {return key;}
	void addCases(ASTSwitchCases* c) {cases.push_back(c);}
	vector<ASTSwitchCases*> & getCases() {return cases;}
	vector<ASTSwitchCases*> const & getCases() const {return cases;}
private:
	ASTExpr* key;
	vector<ASTSwitchCases*> cases;
};

class ASTSwitchCases : public AST
{
public:
	ASTSwitchCases(LocationData Loc) : AST(Loc), isDefault(false), cases() {}
	~ASTSwitchCases();
	ASTSwitchCases* clone() const;

	void execute(ASTVisitor& visitor, void* param) {visitor.caseSwitchCases(*this, param);}
	void execute(ASTVisitor& visitor) {visitor.caseSwitchCases(*this);}
	void addCase(ASTExprConst* expr) {cases.push_back(expr);}
	vector<ASTExprConst*> & getCases() {return cases;}
	vector<ASTExprConst*> const & getCases() const {return cases;}
	void addDefaultCase() {isDefault = true;}
	bool isDefaultCase() const {return isDefault;}
	void setBlock(ASTBlock* b) {block = b;}
	ASTBlock* getBlock() const {return block;}
private:
	bool isDefault;
	vector<ASTExprConst*> cases;
	ASTBlock* block;
};


class ASTStmtFor : public ASTStmt
{
public:
    ASTStmtFor(ASTStmt *Prec, ASTExpr *Term, ASTStmt *Incr, ASTStmt *Stmt, LocationData Loc);
    ~ASTStmtFor();
	ASTStmtFor* clone() const;

    ASTStmt *getPrecondition() const {return prec;}
    ASTExpr *getTerminationCondition() const {return term;}
    ASTStmt *getIncrement() const {return incr;}
    ASTStmt *getStmt() const {return stmt;}
    void execute(ASTVisitor &visitor, void *param) {return visitor.caseStmtFor(*this, param);}
    void execute(ASTVisitor &visitor) {return visitor.caseStmtFor(*this);}
private:
    ASTStmt *prec;
    ASTExpr *term;
    ASTStmt *incr;
    ASTStmt *stmt;
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtFor(ASTStmtFor &);
    ASTStmtFor &operator=(ASTStmtFor &);
};

class ASTStmtWhile : public ASTStmt
{
public:
    ASTStmtWhile(ASTExpr *Cond, ASTStmt *Stmt, LocationData Loc);
    ~ASTStmtWhile();
	ASTStmtWhile* clone() const;

    ASTExpr *getCond() const {return cond;}
    ASTStmt *getStmt() const {return stmt;}
    void execute(ASTVisitor &visitor, void *param) {visitor.caseStmtWhile(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseStmtWhile(*this);}
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
    ASTStmtDo(ASTExpr *Cond, ASTStmt *Stmt, LocationData Loc);
    ~ASTStmtDo();
	ASTStmtDo* clone() const;
	
    ASTExpr *getCond() const {return cond;}
    ASTStmt *getStmt() const {return stmt;}
    void execute(ASTVisitor &visitor, void *param) {visitor.caseStmtDo(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseStmtDo(*this);}
private:
    ASTExpr *cond;
    ASTStmt *stmt;
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtDo(ASTStmtDo &);
    ASTStmtDo &operator=(ASTStmtDo &);
};

class ASTStmtReturn : public ASTStmt
{
public:
    ASTStmtReturn(LocationData Loc) : ASTStmt(Loc) {}
	ASTStmtReturn* clone() const;

    void execute(ASTVisitor &visitor, void *param) {visitor.caseStmtReturn(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseStmtReturn(*this);}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtReturn(ASTStmtReturn &);
    ASTStmtReturn &operator=(ASTStmtReturn &);
};

class ASTStmtReturnVal : public ASTStmt
{
public:
    ASTStmtReturnVal(ASTExpr *Retval, LocationData Loc) : ASTStmt(Loc), retval(Retval) {}
    ~ASTStmtReturnVal();
	ASTStmtReturnVal* clone() const;

    ASTExpr *getReturnValue() const {return retval;}
    void execute(ASTVisitor &visitor, void *param) {visitor.caseStmtReturnVal(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseStmtReturnVal(*this);}
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
	ASTStmtBreak* clone() const;

    void execute(ASTVisitor &visitor, void *param) {visitor.caseStmtBreak(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseStmtBreak(*this);}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtBreak(ASTStmtBreak &);
    ASTStmtBreak &operator=(ASTStmtBreak &);
};

class ASTStmtContinue : public ASTStmt
{
public:
    ASTStmtContinue(LocationData Loc) : ASTStmt(Loc) {}
	ASTStmtContinue* clone() const;

    void execute(ASTVisitor &visitor, void *param) {visitor.caseStmtContinue(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseStmtContinue(*this);}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtContinue(ASTStmtContinue &);
    ASTStmtContinue &operator=(ASTStmtContinue &);
};

class ASTStmtEmpty : public ASTStmt
{
public:
    ASTStmtEmpty(LocationData Loc) : ASTStmt(Loc) {}
	ASTStmtEmpty* clone() const;

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtEmpty(*this, param);}
private:
    //NOT IMPLEMENTED; DO NOT USE
    ASTStmtEmpty(ASTStmtEmpty &);
    ASTStmtEmpty &operator=(ASTStmtEmpty&);
};

class CompileError;
class ASTCompileError : public ASTStmt
{
public:
	ASTCompileError(ASTExpr* errorId, ASTStmt* statement,
					LocationData const& location = LocationData::NONE);
	ASTCompileError(ASTCompileError const& base);
	~ASTCompileError();
	ASTCompileError& operator=(ASTCompileError const& rhs);
	ASTCompileError* clone() const {return new ASTCompileError(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseCompileError(*this, param);}
	
	// The expression for the error id. If NULL, no id is specified.
	ASTExpr* errorId;

	// The statement to execute while catching the compile error.
	ASTStmt* statement;

	// If the specified error has been triggered.
	bool errorTriggered;
	
	// Get the error id as an integer. -1 for invalid.
	int getErrorId() const;

	// Does this node handle the given error?
	bool canHandle(CompileError const& error) const;
};

////////////////////////////////////////////////////////////////
// Declarations

enum ASTDeclClassId
{
	ASTDECL_CLASSID_NONE,
	ASTDECL_CLASSID_SCRIPT,
	ASTDECL_CLASSID_IMPORT,
	ASTDECL_CLASSID_CONSTANT,
	ASTDECL_CLASSID_FUNCTION,
	ASTDECL_CLASSID_DATALIST,
	ASTDECL_CLASSID_DATA,
	ASTDECL_CLASSID_TYPE
};

class ASTDecl : public ASTStmt
{
public:
    ASTDecl(LocationData const& location) : ASTStmt(location) {}
	ASTDecl(ASTDecl const& base) : ASTStmt(base) {}
    virtual ~ASTDecl() {}
	virtual ASTDecl& operator=(ASTDecl const& rhs);
	virtual ASTDecl* clone() const = 0;

	virtual ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_NONE;}
};

class ASTScript : public ASTDecl
{
public:
    ASTScript(LocationData const& location);
	ASTScript(ASTScript const& base);
	~ASTScript();
	ASTScript& operator=(ASTScript const& rhs);
	ASTScript* clone() const {return new ASTScript(*this);}

    void execute(ASTVisitor &visitor, void *param) {visitor.caseScript(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseScript(*this);}

	ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_SCRIPT;}
    
	// Adds a declaration to the proper vector.
	void addDeclaration(ASTDecl& declaration);

    ASTScriptType* getType() const {return type;}
	void setType(ASTScriptType* node) {type = node;}

    string const& getName() const {return name;}
    string getName() {return name;}
	void setName(string const& n) {name = n;}
    
	vector<ASTDataDeclList*> variables;
	vector<ASTFuncDecl*> functions;
	vector<ASTTypeDef*> types;
private:
    ASTScriptType* type;
    string name;
};

class ASTImportDecl : public ASTDecl
    {
public:
    ASTImportDecl(string file, LocationData Loc) : ASTDecl(Loc), filename(file) {}
	ASTImportDecl* clone() const;
    
    string getFilename() const {return filename;}
    void execute(ASTVisitor &visitor, void *param) {visitor.caseImportDecl(*this,param);}
    void execute(ASTVisitor &visitor) {visitor.caseImportDecl(*this);}
	ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_IMPORT;}
private:
    string filename;
    //NOT IMPLEMENTED; DO NOT USE
    ASTImportDecl(ASTImportDecl &);
    ASTImportDecl &operator=(ASTImportDecl &);
};

class ASTFuncDecl : public ASTDecl
{
public:
    ASTFuncDecl(LocationData const& location = LocationData::NONE);
	ASTFuncDecl(ASTFuncDecl const& base);
	ASTFuncDecl& operator=(ASTFuncDecl const& rhs);
	ASTFuncDecl* clone() const {return new ASTFuncDecl(*this);}
    ~ASTFuncDecl();
    
    void execute(ASTVisitor &visitor, void *param) {visitor.caseFuncDecl(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseFuncDecl(*this);}
	ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_FUNCTION;}

	vector<ASTDataDecl*>& getParameters() {return parameters;}
	void addParameter(ASTDataDecl* parameter);

	ASTVarType* returnType;
    string name;
	ASTBlock* block;

private:
	vector<ASTDataDecl*> parameters;
};

namespace ZScript
{
	class Variable;
}

// A line of variable/constant declarations:
// int a, b, c[];
class ASTDataDeclList : public ASTDecl
{
public:
	ASTDataDeclList(LocationData const& location = LocationData::NONE);
	ASTDataDeclList(ASTDataDeclList const& base);
	ASTDataDeclList& operator=(ASTDataDeclList const& rhs);
	~ASTDataDeclList();
	ASTDataDeclList* clone() const {return new ASTDataDeclList(*this);}

	void execute(ASTVisitor& visitor, void* param) {visitor.caseDataDeclList(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseDataDeclList(*this);}
	ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_DATALIST;}

	// The base type at the start of the line shared by all the declarations.
	ASTVarType* baseType;

	vector<ASTDataDecl*> const& getDeclarations() const {return declarations;}
	void addDeclaration(ASTDataDecl* declaration);

private:
	// The list of individual data declarations.
	vector<ASTDataDecl*> declarations;
};

// Declares a single variable or constant. May or may not be inside an
// ASTDataDeclList.
class ASTDataDecl : public ASTDecl
{
public:
	ASTDataDecl(LocationData const& location = LocationData::NONE);
	ASTDataDecl(ASTDataDecl const& base);
	~ASTDataDecl();
	ASTDataDecl& operator=(ASTDataDecl const& rhs);
	ASTDataDecl* clone() const {return new ASTDataDecl(*this);}

	void execute(ASTVisitor& visitor, void* param) {visitor.caseDataDecl(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseDataDecl(*this);}
	ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_DATA;}

	// The list containing this declaration. Should be set by that list when
	// this is added.
	ASTDataDeclList* list;

	// Reference back to the variable manager for this node. Should be set by
	// that Variable when it is created.
	ZScript::Variable* manager;

	// This type of this data (minus the extra arrays). This should only be
	// set if this declaration is not part of a list, as the list's base type
	// should be used instead in that case.
	ASTVarType* baseType;

	// The symbol this declaration is binding.
    string name;

	ASTExpr* getInitializer() const {return initializer;}
	void setInitializer(ASTExpr* initializer);

	// Extra array type for this specific declaration. The final type is the
	// list's base type combined with these.
	vector<ASTDataDeclExtraArray*> extraArrays;

	// Resolves the type, using either the list's or this node's own base type
	// as appropriate.
	ZVarType const* resolveType(Scope* scope) const;

private:
	// The initialization expression. Optional.
	ASTExpr* initializer;
};

// The extra array parameters appended to a data declaration name.
class ASTDataDeclExtraArray : public AST
{
public:
	ASTDataDeclExtraArray(LocationData const& location = LocationData::NONE);
	ASTDataDeclExtraArray(ASTDataDeclExtraArray const& base);
	ASTDataDeclExtraArray& operator=(ASTDataDeclExtraArray const& rhs);
	ASTDataDeclExtraArray* clone() const {return new ASTDataDeclExtraArray(*this);}
	~ASTDataDeclExtraArray();

	void execute(ASTVisitor& visitor, void* param) {visitor.caseDataDeclExtraArray(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseDataDeclExtraArray(*this);}

	// The vector of array dimensions. Empty means unspecified.
	vector<ASTExpr*> dimensions;

	// If this declares an a sized array.
	bool hasSize() const {return dimensions.size();}

	// If all expressions in the dimension array are constant.
	bool isConstant() const;
    
	// If this is constant, get the total size. Returns -1 if not constant or
	// not specified.
	int getTotalSize() const;
};

class ASTTypeDef : public ASTDecl
{
public:
	ASTTypeDef(ASTVarType *type, string const& name, LocationData location) : ASTDecl(location), type(type), name(name) {}
	~ASTTypeDef();
	ASTTypeDef* clone() const;

	ASTVarType* getType() const {return type;}
	string getName() const {return name;}

    void execute(ASTVisitor &visitor, void *param) {visitor.caseTypeDef(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseTypeDef(*this);}
	ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_TYPE;}
private:
	ASTVarType* type;
	string name;
};

////////////////////////////////////////////////////////////////
// Expressions

class ASTExpr : public ASTStmt
{
public:
    ASTExpr(LocationData const& location) : ASTStmt(location), hasValue(false), value(0L), varType(NULL), lval(false) {}
	ASTExpr(ASTExpr const& base);
	ASTExpr& operator=(ASTExpr const& rhs);
	virtual ASTExpr* clone() const = 0;

	virtual bool isConstant() const = 0;

	void markAsLVal() {lval = true;}
	bool isLVal() {return lval;}

	bool hasDataValue() const {return hasValue;}
	long getDataValue() const {return value;}
	void setDataValue(long v) {value = v; hasValue = true;}

	ZVarType const* getVarType() const {return varType;}
	void setVarType(ZVarType const& type) {varType = &type;}
	void setVarType(ZVarType& type) {varType = (ZVarType const*)&type;}
	void setVarType(ZVarType const* type) {varType = type;}
	void setVarType(ZVarType* type) {varType = (ZVarType const*)type;}

private:
	bool hasValue;
	long value;
	ZVarType const* varType;
	bool lval;
};

// Wrap around an expression to type it as constant.
class ASTExprConst : public ASTExpr
{
public:
    ASTExprConst(ASTExpr* content) : ASTExpr(content->getLocation()), content(content) {}
	ASTExprConst(ASTExpr* content, LocationData const& location) : ASTExpr(location), content(content) {}
	ASTExprConst(ASTExprConst const& base);
	ASTExprConst& operator=(ASTExprConst const& rhs);
	~ASTExprConst() {delete content;}
	ASTExprConst* clone() const {return new ASTExprConst(*this);}

	ASTExpr* getContent() const {return content;}
	bool isConstant() const {return true;}
    void execute(ASTVisitor &visitor, void *param) {visitor.caseExprConst(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseExprConst(*this);}
private:
	ASTExpr* content;
};

class ASTExprAssign : public ASTExpr
{
public:
	ASTExprAssign(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTExpr(location), lval(left), rval(right) {}
	~ASTExprAssign();
	ASTExprAssign* clone() const;

	ASTExpr* getLVal() const {return lval;}
	ASTExpr* getRVal() const {return rval;}
	bool isConstant() const {return rval && rval->isConstant();}
	void execute(ASTVisitor& visitor, void* param) {visitor.caseExprAssign(*this, param);}
	void execute(ASTVisitor& visitor) {visitor.caseExprAssign(*this);}
private:
	ASTExpr* lval;
	ASTExpr* rval;
};

class ASTExprIdentifier : public ASTExpr
{
public:
    ASTExprIdentifier(string const& name, LocationData const& location);
	ASTExprIdentifier(ASTExprIdentifier const& base);
	ASTExprIdentifier& operator=(ASTExprIdentifier const& base);
	ASTExprIdentifier* clone() const {return new ASTExprIdentifier(*this);}

	vector<string>const& getComponents() const {return components;}
	void appendComponent(string const& component) {components.push_back(component);}
	bool isConstant() const {return isConstant_;}
	void markConstant() {isConstant_ = true;}
	void execute(ASTVisitor &visitor, void *param) {visitor.caseExprIdentifier(*this, param);}
	void execute(ASTVisitor &visitor) {visitor.caseExprIdentifier(*this);}

	string asString() const;
	bool isTypeIdentifier() const {return true;}
private:
	vector<string> components;
	bool isConstant_;
};

class ASTExprArrow : public ASTExpr
{
public:
    ASTExprArrow(ASTExpr* left, string const& right, LocationData const& location);
	ASTExprArrow(ASTExprArrow const& base);
	ASTExprArrow& operator=(ASTExprArrow const& rhs);
    ~ASTExprArrow();
	ASTExprArrow* clone() const {return new ASTExprArrow(*this);}

    ASTExpr* getLeft() const {return left;}
    string getRight() const {return right;}
    ASTExpr* getIndex() const {return index;}
    void setIndex(ASTExpr* e) {index = e;}
	bool isConstant() const {return false;}

    void execute(ASTVisitor &visitor, void *param) {visitor.caseExprArrow(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseExprArrow(*this);}

	string asString() const;

	bool isTypeArrow() const {return true;}
private:
    ASTExpr* left;
    string right;
    ASTExpr* index;
};

class ASTExprIndex : public ASTExpr
{
public:
    ASTExprIndex(ASTExpr* array, ASTExpr* index, LocationData const& location);
	ASTExprIndex(ASTExprIndex const& base);
	ASTExprIndex& operator=(ASTExprIndex const& rhs);
    ~ASTExprIndex() {delete array; delete index;}
	ASTExprIndex* clone() const {return new ASTExprIndex(*this);}
	bool isTypeIndex() const {return true;}
    
	ASTExpr* getArray()const {return array;}
    ASTExpr* getIndex() const {return index;}
	void setArray(ASTExpr* e) {array = e;}
    void setIndex(ASTExpr* e) {index = e;}
	bool isConstant() const;

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprIndex(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprIndex(*this);}
private:
	ASTExpr* array;
    ASTExpr* index;
};

class ASTExprCall : public ASTExpr
{
public:
	ASTExprCall(LocationData const& location);
	ASTExprCall(ASTExprCall const& base);
	ASTExprCall& operator=(ASTExprCall const& rhs);
	~ASTExprCall();
	ASTExprCall* clone() const {return new ASTExprCall(*this);}

	void execute(ASTVisitor& visitor, void* param) {visitor.caseExprCall(*this, param);}
	void execute(ASTVisitor& visitor) {visitor.caseExprCall(*this);}

	bool isConstant() const {return false;}

	ASTExpr* getLeft() const {return left;}
	void setLeft(ASTExpr* expr) {left = expr;}
    list<ASTExpr*> const &getParams() const {return params;}
    list<ASTExpr*> &getParams() {return params;}
	void addParam(ASTExpr* param) {params.push_back(param);}

private:
    ASTExpr* left;
    list<ASTExpr*> params;
};

class ASTUnaryExpr : public ASTExpr
{
public:
    ASTUnaryExpr(LocationData const& location) : ASTExpr(location) {}
	ASTUnaryExpr(ASTUnaryExpr const& base);
	ASTUnaryExpr& operator=(ASTUnaryExpr const& rhs);
    virtual ~ASTUnaryExpr() {delete operand;}
	virtual ASTUnaryExpr* clone() const = 0;

    ASTExpr* getOperand() const {return operand;}
    void setOperand(ASTExpr* e) {operand = e;}
	virtual bool isConstant() const {return operand->isConstant();}
private:
    ASTExpr* operand;
};

class ASTExprNegate : public ASTUnaryExpr
{
public:
    ASTExprNegate(LocationData const& location) : ASTUnaryExpr(location) {}
	ASTExprNegate* clone() const {return new ASTExprNegate(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprNegate(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprNegate(*this);}
};

class ASTExprNot : public ASTUnaryExpr
{
public:
    ASTExprNot(LocationData const& location) : ASTUnaryExpr(location) {}
	ASTExprNot* clone() const {return new ASTExprNot(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprNot(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprNot(*this);}
};

class ASTExprBitNot : public ASTUnaryExpr
{
public:
    ASTExprBitNot(LocationData const& location) : ASTUnaryExpr(location) {}
	ASTExprBitNot* clone() const {return new ASTExprBitNot(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprBitNot(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprBitNot(*this);}
};

class ASTExprIncrement : public ASTUnaryExpr
{
public:
    ASTExprIncrement(LocationData const& location) : ASTUnaryExpr(location) {}
	ASTExprIncrement* clone() const {return new ASTExprIncrement(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprIncrement(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprIncrement(*this);}
	bool isConstant() const {return false;}
};

class ASTExprPreIncrement : public ASTUnaryExpr
{
public:
    ASTExprPreIncrement(LocationData const& location) : ASTUnaryExpr(location) {}
	ASTExprPreIncrement* clone() const {return new ASTExprPreIncrement(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprPreIncrement(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprPreIncrement(*this);}
	bool isConstant() const {return false;}
};

class ASTExprDecrement : public ASTUnaryExpr
{
public:
    ASTExprDecrement(LocationData const& location) : ASTUnaryExpr(location) {}
	ASTExprDecrement* clone() const {return new ASTExprDecrement(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprDecrement(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprDecrement(*this);}
	bool isConstant() const {return false;}
};

class ASTExprPreDecrement : public ASTUnaryExpr
{
public:
    ASTExprPreDecrement(LocationData const& location) : ASTUnaryExpr(location) {}
	ASTExprPreDecrement* clone() const {return new ASTExprPreDecrement(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprPreDecrement(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprPreDecrement(*this);}
	bool isConstant() const {return false;}
};

class ASTBinaryExpr : public ASTExpr
{
public:
    ASTBinaryExpr(LocationData const& location) : ASTExpr(location) {}
	ASTBinaryExpr(ASTExpr* first, ASTExpr* second, LocationData const& location)
			: ASTExpr(location), first(first), second(second) {}
	ASTBinaryExpr(ASTBinaryExpr const& base);
	ASTBinaryExpr& operator=(ASTBinaryExpr const& rhs);
    virtual ~ASTBinaryExpr();
	virtual ASTBinaryExpr* clone() const = 0;

    ASTExpr* getFirstOperand() const {return first;}
    void setFirstOperand(ASTExpr* e) {first = e;}
    ASTExpr* getSecondOperand() const {return second;}
    void setSecondOperand(ASTExpr* e) {second = e;}
	bool isConstant() const {return first->isConstant() && second->isConstant();}
private:
    ASTExpr* first;
    ASTExpr* second;
};

class ASTLogExpr : public ASTBinaryExpr
{
public:
    ASTLogExpr(LocationData const& location) : ASTBinaryExpr(location) {}
	ASTLogExpr(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTBinaryExpr(left, right, location) {}
	virtual ASTLogExpr* clone() const = 0;
};

class ASTExprAnd : public ASTLogExpr
{
public:
    ASTExprAnd(LocationData const& location) : ASTLogExpr(location) {}
	ASTExprAnd(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTLogExpr(left, right, location) {}
	ASTExprAnd* clone() const {return new ASTExprAnd(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprAnd(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprAnd(*this);}
};

class ASTExprOr : public ASTLogExpr
{
public:
    ASTExprOr(LocationData const& location) : ASTLogExpr(location) {}
	ASTExprOr(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTLogExpr(left, right, location) {}
	ASTExprOr* clone() const {return new ASTExprOr(*this);}
    
    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprOr(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprOr(*this);}
};

class ASTRelExpr : public ASTBinaryExpr
{
public:
    ASTRelExpr(LocationData const& location) : ASTBinaryExpr(location) {}
	ASTRelExpr(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTBinaryExpr(left, right, location) {}
	virtual ASTRelExpr* clone() const = 0;
};

class ASTExprGT : public ASTRelExpr
{
public:
    ASTExprGT(LocationData const& location) : ASTRelExpr(location) {}
	ASTExprGT(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTRelExpr(left, right, location) {}
	ASTExprGT* clone() const {return new ASTExprGT(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprGT(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprGT(*this);}
};

class ASTExprGE : public ASTRelExpr
{
public:
    ASTExprGE(LocationData const& location) : ASTRelExpr(location) {}
	ASTExprGE* clone() const {return new ASTExprGE(*this);}
	ASTExprGE(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTRelExpr(left, right, location) {}

    void execute(ASTVisitor &visitor, void *param) {visitor.caseExprGE(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseExprGE(*this);}
};

class ASTExprLT : public ASTRelExpr
{
public:
    ASTExprLT(LocationData const& location) : ASTRelExpr(location) {}
	ASTExprLT(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTRelExpr(left, right, location) {}
	ASTExprLT* clone() const {return new ASTExprLT(*this);}

	void execute(ASTVisitor& visitor, void* param) {visitor.caseExprLT(*this, param);}
	void execute(ASTVisitor& visitor) {visitor.caseExprLT(*this);}
};

class ASTExprLE : public ASTRelExpr
{
public:
    ASTExprLE(LocationData const& location) : ASTRelExpr(location) {}
	ASTExprLE(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTRelExpr(left, right, location) {}
	ASTExprLE* clone() const {return new ASTExprLE(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprLE(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprLE(*this);}
};

class ASTExprEQ : public ASTRelExpr
{
public:
    ASTExprEQ(LocationData const& location) : ASTRelExpr(location) {}
	ASTExprEQ(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTRelExpr(left, right, location) {}
	ASTExprEQ* clone() const {return new ASTExprEQ(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprEQ(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprEQ(*this);}
};

class ASTExprNE : public ASTRelExpr
{
public:
    ASTExprNE(LocationData const& location) : ASTRelExpr(location) {}
	ASTExprNE(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTRelExpr(left, right, location) {}
	ASTExprNE* clone() const {return new ASTExprNE(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprNE(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprNE(*this);}
};

class ASTAddExpr : public ASTBinaryExpr
{
public:
    ASTAddExpr(LocationData const& location) : ASTBinaryExpr(location) {}
	ASTAddExpr(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTBinaryExpr(left, right, location) {}
	virtual ASTAddExpr* clone() const = 0;
};

class ASTExprPlus : public ASTAddExpr
{
public:
    ASTExprPlus(LocationData const& location) : ASTAddExpr(location) {}
	ASTExprPlus(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTAddExpr(left, right, location) {}
	ASTExprPlus* clone() const {return new ASTExprPlus(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprPlus(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprPlus(*this);}
};

class ASTExprMinus : public ASTAddExpr
{
public:
    ASTExprMinus(LocationData const& location) : ASTAddExpr(location) {}
	ASTExprMinus(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTAddExpr(left, right, location) {}
	ASTExprMinus* clone() const {return new ASTExprMinus(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprMinus(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprMinus(*this);}
};

class ASTMultExpr : public ASTBinaryExpr
{
public:
    ASTMultExpr(LocationData const& location) : ASTBinaryExpr(location) {}
	ASTMultExpr(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTBinaryExpr(left, right, location) {}
	virtual ASTMultExpr* clone() const = 0;
};

class ASTExprTimes : public ASTMultExpr
{
public:
    ASTExprTimes(LocationData const& location) : ASTMultExpr(location) {}
	ASTExprTimes(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTMultExpr(left, right, location) {}
	ASTExprTimes* clone() const {return new ASTExprTimes(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprTimes(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprTimes(*this);}
};

class ASTExprDivide : public ASTMultExpr
{
public:
    ASTExprDivide(LocationData const& location) : ASTMultExpr(location) {}
	ASTExprDivide(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTMultExpr(left, right, location) {}
	ASTExprDivide* clone() const {return new ASTExprDivide(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprDivide(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprDivide(*this);}
};

class ASTExprModulo : public ASTMultExpr
{
public:
    ASTExprModulo(LocationData const& location) : ASTMultExpr(location) {}
	ASTExprModulo(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTMultExpr(left, right, location) {}
	ASTExprModulo* clone() const {return new ASTExprModulo(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprModulo(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprModulo(*this);}
};

class ASTBitExpr : public ASTBinaryExpr
{
public:
    ASTBitExpr(LocationData const& location) : ASTBinaryExpr(location) {}
	ASTBitExpr(ASTExpr* left, ASTExpr* right, LocationData const& location) :
			ASTBinaryExpr(left, right, location) {}
	virtual ASTBitExpr* clone() const = 0;
};

class ASTExprBitAnd : public ASTBitExpr
{
public:
    ASTExprBitAnd(LocationData const& location) : ASTBitExpr(location) {}
	ASTExprBitAnd(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTBitExpr(left, right, location) {}
	ASTExprBitAnd* clone() const {return new ASTExprBitAnd(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprBitAnd(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprBitAnd(*this);}
};

class ASTExprBitOr : public ASTBitExpr
{
public:
    ASTExprBitOr(LocationData const& location) : ASTBitExpr(location) {}
	ASTExprBitOr(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTBitExpr(left, right, location) {}
	ASTExprBitOr* clone() const {return new ASTExprBitOr(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprBitOr(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprBitOr(*this);}
};

class ASTExprBitXor : public ASTBitExpr
{
public:
    ASTExprBitXor(LocationData const& location) : ASTBitExpr(location) {}
	ASTExprBitXor(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTBitExpr(left, right, location) {}
	ASTExprBitXor* clone() const {return new ASTExprBitXor(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprBitXor(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprBitXor(*this);}
};

class ASTShiftExpr : public ASTBinaryExpr
{
public:
    ASTShiftExpr(LocationData const& location) : ASTBinaryExpr(location) {}
	ASTShiftExpr(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTBinaryExpr(left, right, location) {}
	virtual ASTShiftExpr* clone() const = 0;
};

class ASTExprLShift : public ASTShiftExpr
{
public:
    ASTExprLShift(LocationData const& location) : ASTShiftExpr(location) {}
	ASTExprLShift(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTShiftExpr(left, right, location) {}
	ASTExprLShift* clone() const {return new ASTExprLShift(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprLShift(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprLShift(*this);}
};

class ASTExprRShift : public ASTShiftExpr
{
public:
    ASTExprRShift(LocationData const& location) : ASTShiftExpr(location) {}
	ASTExprRShift(ASTExpr* left, ASTExpr* right, LocationData const& location)
			: ASTShiftExpr(left, right, location) {}
	ASTExprRShift* clone() const {return new ASTExprRShift(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseExprRShift(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseExprRShift(*this);}
};

// Literals

namespace ZScript
{
	class Literal;
}

class ASTLiteral : public ASTExpr
{
public:
	ASTLiteral(LocationData const& location) : ASTExpr(location), manager(NULL) {}
	ASTLiteral(ASTLiteral const& base) : ASTExpr(base), manager(base.manager) {}
	ASTLiteral& operator=(ASTLiteral const& rhs);

	ZScript::Literal* manager;
};

class ASTNumberLiteral : public ASTLiteral
{
public:
    ASTNumberLiteral(ASTFloat* value, LocationData const& location) : ASTLiteral(location), val(value) {}
	ASTNumberLiteral(ASTNumberLiteral const& base);
	ASTNumberLiteral& operator=(ASTNumberLiteral const& rhs);
    ~ASTNumberLiteral() {delete val;}
	ASTNumberLiteral* clone() const {return new ASTNumberLiteral(*this);}

    void execute(ASTVisitor& visitor, void* param) {visitor.caseNumberLiteral(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseNumberLiteral(*this);}

    ASTFloat* getValue() const {return val;}
	bool isConstant() const {return true;}
private:
    ASTFloat* val;
};

class ASTBoolLiteral : public ASTLiteral
{
public:
    ASTBoolLiteral(bool value, LocationData const& location) : ASTLiteral(location), value(value) {}
	ASTBoolLiteral(ASTBoolLiteral const& base);
	ASTBoolLiteral& operator=(ASTBoolLiteral const& base);
	ASTBoolLiteral* clone() const {return new ASTBoolLiteral(*this);}

    void execute(ASTVisitor &visitor, void *param) {visitor.caseBoolLiteral(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseBoolLiteral(*this);}

    bool getValue() const {return value;}
	bool isConstant() const {return true;}
private:
    bool value;
};

class ASTStringLiteral : public ASTLiteral
{
public:
	ASTStringLiteral(char const* str, LocationData const& location);
	ASTStringLiteral(string const& str, LocationData const& location);
	ASTStringLiteral(ASTString const& raw);
	ASTStringLiteral(ASTStringLiteral const& base);
	ASTStringLiteral& operator=(ASTStringLiteral const& rhs);
	ASTStringLiteral* clone() const {return new ASTStringLiteral(*this);}

	void execute (ASTVisitor& visitor, void* param) {visitor.caseStringLiteral(*this, param);}
	void execute (ASTVisitor& visitor) {visitor.caseStringLiteral(*this);}
	bool isStringLiteral() const {return true;}

	bool isConstant() const {return true;}

	string getValue() const {return data;}

	// The data declaration that this literal may be part of. If NULL that
	// means this is not part of a data declaration. This should be managed by
	// that declaration and not modified by this object at all.
	ASTDataDecl* declaration;
private:
	string data;
};

class ASTArrayLiteral : public ASTLiteral
{
public:
	ASTArrayLiteral(LocationData const& location);
	ASTArrayLiteral(ASTArrayLiteral const& base);
	ASTArrayLiteral& operator=(ASTArrayLiteral const& rhs);
	ASTArrayLiteral* clone() const {return new ASTArrayLiteral(*this);}
	~ASTArrayLiteral();

	void execute (ASTVisitor& visitor, void* param) {visitor.caseArrayLiteral(*this, param);}
	void execute (ASTVisitor& visitor) {visitor.caseArrayLiteral(*this);}
	bool isArrayLiteral() const {return true;}

	bool isConstant() const {return true;}

	ASTVarType* getType() const {return type;}
	void setType(ASTVarType* node) {type = node;}
	ASTExpr* getSize() const {return size;}
	void setSize(ASTExpr* node) {size = node;}
	vector<ASTExpr*> getElements() const {return elements;}
	void appendElement(ASTExpr* element) {elements.push_back(element);}

	// The data declaration that this literal may be part of. If NULL that
	// means this is not part of a data declaration. This should be managed by
	// that declaration and not modified by this object at all.
	ASTDataDecl* declaration;

private:
	ASTVarType* type;
	ASTExpr* size;
	vector<ASTExpr*> elements;
};

// Types

class ASTScriptType : public AST
{
public:
	ASTScriptType(ScriptType Type, LocationData Loc) : AST(Loc), type(Type) {}
	ASTScriptType* clone() const {return new ASTScriptType(type, getLocation());}
	ScriptType getType() const {return type;}
    void execute(ASTVisitor &visitor, void *param) {visitor.caseScriptType(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseScriptType(*this);}
private:
	ScriptType type;
};

class ASTVarType : public AST
{
public:
    ASTVarType(ZVarType const& type, LocationData Loc) : AST(Loc), type(type.clone()) {}
    ASTVarType(ZVarType* type, LocationData Loc) : AST(Loc), type(type) {}
	ASTVarType(ASTVarType const& base) : AST(base.getLocation()), type(base.type->clone()) {}
	~ASTVarType() {delete type;}
	ASTVarType* clone() const {return new ASTVarType(*this);}
	ASTVarType& operator=(ASTVarType const& base)
	{
		getLocation() = base.getLocation();
		type = base.type->clone();
	}
    void execute(ASTVisitor &visitor, void* param) {visitor.caseVarType(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseVarType(*this);}
	ZVarType const& resolve(Scope& scope);
private:
	ZVarType* type;
};

#endif


#ifndef AST_H
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

class CompileErrorHandler;

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
class ASTStmtCompileError;
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
	virtual void caseStmtCompileError(ASTStmtCompileError& node, void* param = NULL) {
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
	AST(LocationData const& location = LocationData::NONE);
	AST(AST const& base);
	// Calls subclass's copy constructor on self.
	virtual AST* clone() const = 0;

	virtual void execute(ASTVisitor& visitor, void* param = NULL) = 0;
	virtual string asString() const {return "unknown";}

	// Filename and linenumber.
    LocationData location;

	// Subclass Predicates (replacing typeof and such).
	virtual bool isTypeArrow() const {return false;}
	virtual bool isTypeIndex() const {return false;}
	virtual bool isTypeIdentifier() const {return false;}
	virtual bool isTypeVarDecl() const {return false;}
	virtual bool isTypeArrayDecl() const {return false;}
	virtual bool isStringLiteral() const {return false;}
	virtual bool isArrayLiteral() const {return false;}

	// Clone a single node pointer.
	template <class Node>
	static Node* clone(Node* node) {return node ? node->clone() : NULL;}
	// Clone a vector of AST nodes.
	template <class Node>
	static vector<Node*> clone(vector<Node*> const& nodes)
	{
		vector<Node*> clones;
		for (typename vector<Node*>::const_iterator it = nodes.begin();
			 it != nodes.end(); ++it)
			clones.push_back((*it)->clone());
		return clones;
	}	
	// Clone a list of AST nodes.
	template <class Node> static list<Node*> clone(list<Node*> const& nodes)
	{
		list<Node*> clones;
		for (typename list<Node*>::const_iterator it = nodes.begin();
			 it != nodes.end(); ++it)
			clones.push_back((*it)->clone());
		return clones;
	}
	// Execute a single node pointer.
	template <class Node>
	static void execute(Node* node, ASTVisitor& visitor, void* param = NULL)
	{
		if (node) node->execute(visitor, param);
	}
	// Execute a vector of AST nodes.
	template <class Node>
	static void execute(
			vector<Node*> const& nodes, ASTVisitor& visitor,
			void* param = NULL)
	{
		for (typename vector<Node*>::const_iterator it = nodes.begin();
			 it != nodes.end(); ++it)
			(*it)->execute(visitor, param);
	}
	// Execute a list of AST nodes.
	template <class Node>
	static void execute(
			list<Node*> const& nodes, ASTVisitor& visitor,
			void* param = NULL)
	{
		for (typename list<Node*>::const_iterator it = nodes.begin();
			 it != nodes.end(); ++it)
			(*it)->execute(visitor, param);
	}

protected:
	// For leaf classes to call in their own assignment operators.
	AST& operator=(AST const& rhs);
};


////////////////////////////////////////////////////////////////

class ASTProgram : public AST
{
public:
    ASTProgram(LocationData const& location = LocationData::NONE);
	ASTProgram(ASTProgram const& base);
	~ASTProgram();
	ASTProgram& operator=(ASTProgram const& rhs);
	ASTProgram* clone() const {return new ASTProgram(*this);}

    void execute(ASTVisitor &visitor, void *param) {visitor.caseProgram(*this, param);}
    void execute(ASTVisitor &visitor) {visitor.caseProgram(*this);}

	// Add a declaration to the proper list based on its type.
	void addDeclaration(ASTDecl* declaration);
	// Steal all the contents of other.
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
	enum Type {TYPE_DECIMAL, TYPE_BINARY, TYPE_HEX};
	
    ASTFloat(char* value, Type type,
			 LocationData const& location = LocationData::NONE);
    ASTFloat(char const* value, Type type,
			 LocationData const& location = LocationData::NONE);
    ASTFloat(string const& value, Type type,
			 LocationData const& location = LocationData::NONE);
    ASTFloat(long value, Type type,
			 LocationData const& location = LocationData::NONE);
	ASTFloat(ASTFloat const& base);
	ASTFloat& operator=(ASTFloat const& rhs);
	ASTFloat* clone() const {return new ASTFloat(*this);}
	
    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseFloat(*this, param);}
    	
    pair<string,string> parseValue();

	Type type;
    string value;
    bool negative;
};

class ASTString : public AST
{
public:
	ASTString(const char* str,
			  LocationData const& location = LocationData::NONE);
    ASTString(string const& str,
			  LocationData const& location = LocationData::NONE);
	ASTString(ASTString const& base);
	ASTString& operator=(ASTString const& rhs);
	ASTString* clone() const {return new ASTString(*this);}
	
    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseString(*this, param);}

    string getValue() const {return str;}
private:
    string str;
};

////////////////////////////////////////////////////////////////
// Statements

class ASTStmt : public AST
{
public:
    ASTStmt(LocationData const& location = LocationData::NONE);
	ASTStmt(ASTStmt const& base) : AST(base) {}
	virtual ASTStmt* clone() const = 0;
protected:
	ASTStmt& operator=(ASTStmt const& rhs);
};

class ASTBlock : public ASTStmt
{
public:
    ASTBlock(LocationData const& location = LocationData::NONE);
	ASTBlock(ASTBlock const& base);
    ~ASTBlock();
	ASTBlock& operator=(ASTBlock const& rhs);
	ASTBlock* clone() const {return new ASTBlock(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseBlock(*this, param);}

	// List of statements this block contains.
    vector<ASTStmt*> statements;
};

class ASTStmtIf : public ASTStmt
{
public:
	ASTStmtIf(ASTExpr* condition, ASTStmt* thenStatement,
			  LocationData const& location = LocationData::NONE);
	ASTStmtIf(ASTStmtIf const& base);
    virtual ~ASTStmtIf();
	ASTStmtIf& operator=(ASTStmtIf const& rhs);
	ASTStmtIf* clone() const {return new ASTStmtIf(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		return visitor.caseStmtIf(*this, param);}

	ASTExpr* condition;
	ASTStmt* thenStatement;
};

class ASTStmtIfElse : public ASTStmtIf
{
public:
	ASTStmtIfElse(
		ASTExpr* condition, ASTStmt* thenStatement, ASTStmt* elseStatement,
		LocationData const& location = LocationData::NONE);
	ASTStmtIfElse(ASTStmtIfElse const& base);
    ~ASTStmtIfElse();
	ASTStmtIfElse& operator=(ASTStmtIfElse const& rhs);
	ASTStmtIfElse* clone() const {return new ASTStmtIfElse(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtIfElse(*this, param);}

    ASTStmt* elseStatement;
};

// A switch statement.
class ASTStmtSwitch : public ASTStmt
{
public:
	ASTStmtSwitch(LocationData const& location = LocationData::NONE);
	ASTStmtSwitch(ASTStmtSwitch const& base);
	~ASTStmtSwitch();
	ASTStmtSwitch& operator=(ASTStmtSwitch const& rhs);
	ASTStmtSwitch* clone() const {return new ASTStmtSwitch(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtSwitch(*this, param);}

	// The key expression used to switch.
	ASTExpr* key;
	// A vector of case groupings.
	vector<ASTSwitchCases*> cases;
private:
};

// A grouping of switch statement labels, and the code for the group.
class ASTSwitchCases : public AST
{
public:
	ASTSwitchCases(LocationData const& location = LocationData::NONE);
	ASTSwitchCases(ASTSwitchCases const& base);
	~ASTSwitchCases();
	ASTSwitchCases& operator=(ASTSwitchCases const& rhs);
	ASTSwitchCases* clone() const {return new ASTSwitchCases(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseSwitchCases(*this, param);}

	// The list of case labels.
	vector<ASTExprConst*> cases;
	// If the default case is included in this grouping.
	bool isDefault;
	// The block to run.
	ASTBlock* block;
};


class ASTStmtFor : public ASTStmt
{
public:
    ASTStmtFor(ASTStmt* setup = NULL,
			   ASTExpr* test = NULL,
			   ASTStmt* increment = NULL,
			   ASTStmt* body = NULL,
			   LocationData const& location = LocationData::NONE);
	ASTStmtFor(ASTStmtFor const& base);
    ~ASTStmtFor();
	ASTStmtFor& operator=(ASTStmtFor const& rhs);
	ASTStmtFor* clone() const {return new ASTStmtFor(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		return visitor.caseStmtFor(*this, param);}

    ASTStmt* setup;
    ASTExpr* test;
    ASTStmt* increment;
    ASTStmt* body;
};

class ASTStmtWhile : public ASTStmt
{
public:
    ASTStmtWhile(ASTExpr* test = NULL,
				 ASTStmt* body = NULL,
				 LocationData const& location = LocationData::NONE);
	ASTStmtWhile(ASTStmtWhile const& base);
    ~ASTStmtWhile();
	ASTStmtWhile& operator=(ASTStmtWhile const& rhs);
	ASTStmtWhile* clone() const {return new ASTStmtWhile(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtWhile(*this, param);}

    ASTExpr* test;
    ASTStmt* body;
};

class ASTStmtDo : public ASTStmt
{
public:
    ASTStmtDo(ASTExpr* test = NULL,
			  ASTStmt* body = NULL,
			  LocationData const& location = LocationData::NONE);
	ASTStmtDo(ASTStmtDo const& base);
    ~ASTStmtDo();
	ASTStmtDo& operator=(ASTStmtDo const& rhs);
	ASTStmtDo* clone() const {return new ASTStmtDo(*this);}
	
    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtDo(*this, param);}

    ASTExpr* test;
    ASTStmt* body;
};

class ASTStmtReturn : public ASTStmt
{
public:
    ASTStmtReturn(LocationData const& location = LocationData::NONE);
	ASTStmtReturn(ASTStmtReturn const& base);
	ASTStmtReturn& operator=(ASTStmtReturn const& rhs);
	ASTStmtReturn* clone() const {return new ASTStmtReturn(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtReturn(*this, param);}
};

class ASTStmtReturnVal : public ASTStmtReturn
{
public:
    ASTStmtReturnVal(ASTExpr* value = NULL,
					 LocationData const& location = LocationData::NONE);
	ASTStmtReturnVal(ASTStmtReturnVal const& base);
    ~ASTStmtReturnVal();
	ASTStmtReturnVal& operator=(ASTStmtReturnVal const& rhs);
	ASTStmtReturnVal* clone() const {return new ASTStmtReturnVal(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtReturnVal(*this, param);}

    ASTExpr* value;
};

class ASTStmtBreak : public ASTStmt
{
public:
    ASTStmtBreak(LocationData const& location = LocationData::NONE);
	ASTStmtBreak(ASTStmtBreak const& base);
	ASTStmtBreak& operator=(ASTStmtBreak const& rhs);
	ASTStmtBreak* clone() const {return new ASTStmtBreak(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtBreak(*this, param);}
};

class ASTStmtContinue : public ASTStmt
{
public:
    ASTStmtContinue(LocationData const& location = LocationData::NONE);
	ASTStmtContinue(ASTStmtContinue const& base);
	ASTStmtContinue& operator=(ASTStmtContinue const& rhs);
	ASTStmtContinue* clone() const {return new ASTStmtContinue(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtContinue(*this, param);}
};

class ASTStmtEmpty : public ASTStmt
{
public:
    ASTStmtEmpty(LocationData const& location = LocationData::NONE);
	ASTStmtEmpty(ASTStmtEmpty const& base);
	ASTStmtEmpty& operator=(ASTStmtEmpty const& rhs);
	ASTStmtEmpty* clone() const {return new ASTStmtEmpty(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtEmpty(*this, param);}
};

class CompileError;
class ASTStmtCompileError : public ASTStmt
{
public:
	ASTStmtCompileError(ASTExpr* errorId = NULL,
					ASTStmt* statement = NULL,
					LocationData const& location = LocationData::NONE);
	ASTStmtCompileError(ASTStmtCompileError const& base);
	~ASTStmtCompileError();
	ASTStmtCompileError& operator=(ASTStmtCompileError const& rhs);
	ASTStmtCompileError* clone() const {return new ASTStmtCompileError(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStmtCompileError(*this, param);}
	
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

// Different declaration subclasses.
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

// virtual
class ASTDecl : public ASTStmt
{
public:
    ASTDecl(LocationData const& location = LocationData::NONE);
	ASTDecl(ASTDecl const& base) : ASTStmt(base) {}
	virtual ASTDecl* clone() const = 0;

	// Return the subclass id.
	virtual ASTDeclClassId declarationClassId() const = 0;

protected:
	virtual ASTDecl& operator=(ASTDecl const& rhs);
};

// Declares a script.
class ASTScript : public ASTDecl
{
public:
    ASTScript(LocationData const& location = LocationData::NONE);
	ASTScript(ASTScript const& base);
	~ASTScript();
	ASTScript& operator=(ASTScript const& rhs);
	ASTScript* clone() const {return new ASTScript(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseScript(*this, param);}

	ASTDeclClassId declarationClassId() const {
		return ASTDECL_CLASSID_SCRIPT;}

	// Adds a declaration to the proper vector.
	void addDeclaration(ASTDecl& declaration);

    ASTScriptType* type;
    string name;
	vector<ASTDataDeclList*> variables;
	vector<ASTFuncDecl*> functions;
	vector<ASTTypeDef*> types;
};

class ASTImportDecl : public ASTDecl
{
public:
    ASTImportDecl(string const& filename,
				  LocationData const& location = LocationData::NONE);
	ASTImportDecl(ASTImportDecl const& base);
	ASTImportDecl& operator=(ASTImportDecl const& rhs);
	ASTImportDecl* clone() const {return new ASTImportDecl(*this);}
    
    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseImportDecl(*this,param);}

	ASTDeclClassId declarationClassId() const {
		return ASTDECL_CLASSID_IMPORT;}

    string filename;
};

class ASTFuncDecl : public ASTDecl
{
public:
    ASTFuncDecl(LocationData const& location = LocationData::NONE);
	ASTFuncDecl(ASTFuncDecl const& base);
    ~ASTFuncDecl();
	ASTFuncDecl& operator=(ASTFuncDecl const& rhs);
	ASTFuncDecl* clone() const {return new ASTFuncDecl(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseFuncDecl(*this, param);}

	ASTDeclClassId declarationClassId() const {
		return ASTDECL_CLASSID_FUNCTION;}

	ASTVarType* returnType;
	vector<ASTDataDecl*> parameters;
	string name;
	ASTBlock* block;
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
	~ASTDataDeclList();
	ASTDataDeclList& operator=(ASTDataDeclList const& rhs);
	ASTDataDeclList* clone() const {return new ASTDataDeclList(*this);}

	void execute(ASTVisitor& visitor, void* param) {visitor.caseDataDeclList(*this, param);}
    void execute(ASTVisitor& visitor) {visitor.caseDataDeclList(*this);}
	ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_DATALIST;}

	// The base type at the start of the line shared by all the declarations.
	ASTVarType* baseType;

	vector<ASTDataDecl*> const& declarations() const {return mDeclarations;}
	void addDeclaration(ASTDataDecl* declaration);

private:
	// The list of individual data declarations.
	vector<ASTDataDecl*> mDeclarations;
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

	ASTExpr* initializer() const {return mInitializer;}
	ASTExpr* initializer(ASTExpr* initializer);

	// Extra array type for this specific declaration. The final type is the
	// list's base type combined with these.
	vector<ASTDataDeclExtraArray*> extraArrays;

	// Resolves the type, using either the list's or this node's own base type
	// as appropriate.
	ZVarType const* resolveType(Scope* scope) const;

private:
	// The initialization expression. Optional.
	ASTExpr* mInitializer;
};

// The extra array parameters appended to a data declaration name.
class ASTDataDeclExtraArray : public AST
{
public:
	ASTDataDeclExtraArray(LocationData const& location = LocationData::NONE);
	ASTDataDeclExtraArray(ASTDataDeclExtraArray const& base);
	~ASTDataDeclExtraArray();
	ASTDataDeclExtraArray& operator=(ASTDataDeclExtraArray const& rhs);
	ASTDataDeclExtraArray* clone() const {return new ASTDataDeclExtraArray(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseDataDeclExtraArray(*this, param);}

	// The vector of array dimensions. Empty means unspecified.
	vector<ASTExpr*> dimensions;

	// If this declares an a sized array.
	bool hasSize() const {return dimensions.size() > 0;}

	// Get the total size of this array at compile time.
	optional<int> getCompileTimeSize(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTTypeDef : public ASTDecl
{
public:
	ASTTypeDef(ASTVarType* type = NULL,
			   string const& name = "",
			   LocationData const& location = LocationData::NONE);
	ASTTypeDef(ASTTypeDef const& base);
	~ASTTypeDef();
	ASTTypeDef& operator=(ASTTypeDef const& rhs);
	ASTTypeDef* clone() const {return new ASTTypeDef(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseTypeDef(*this, param);}

	ASTDeclClassId declarationClassId() const {return ASTDECL_CLASSID_TYPE;}

	ASTVarType* type;
	string name;
};

////////////////////////////////////////////////////////////////
// Expressions

class ASTExpr : public ASTStmt
{
public:
    ASTExpr(LocationData const& location = LocationData::NONE);
	ASTExpr(ASTExpr const& base);
	virtual ASTExpr* clone() const = 0;

	virtual bool isConstant() const = 0;

	void markAsLVal() {lval = true;}
	bool isLVal() {return lval;}

	// Return this expression's value if it has already been resolved at
	// compile time.
	virtual optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const
	{return nullopt;}
	
	ZVarType const* getVarType() const {return varType;}
	void setVarType(ZVarType const& type) {varType = &type;}
	void setVarType(ZVarType& type) {varType = (ZVarType const*)&type;}
	void setVarType(ZVarType const* type) {varType = type;}
	void setVarType(ZVarType* type) {varType = (ZVarType const*)type;}

private:
	ZVarType const* varType;
	bool lval;

protected:
	ASTExpr& operator=(ASTExpr const& rhs);
};

// Wrap around an expression to type it as constant.
class ASTExprConst : public ASTExpr
{
public:
	ASTExprConst(ASTExpr* content = NULL,
				 LocationData const& location = LocationData::NONE);
	ASTExprConst(ASTExprConst const& base);
	~ASTExprConst() {delete content;}
	ASTExprConst& operator=(ASTExprConst const& rhs);
	ASTExprConst* clone() const {return new ASTExprConst(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprConst(*this, param);}

	bool isConstant() const {return true;}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
	
	ASTExpr* content;
};

class ASTExprAssign : public ASTExpr
{
public:
	ASTExprAssign(ASTExpr* left = NULL,
				  ASTExpr* right = NULL,
				  LocationData const& location = LocationData::NONE);
	ASTExprAssign(ASTExprAssign const& base);
	~ASTExprAssign();
	ASTExprAssign& operator=(ASTExprAssign const& rhs);
	ASTExprAssign* clone() const {return new ASTExprAssign(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprAssign(*this, param);}

	bool isConstant() const {return right && right->isConstant();}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
	
	ASTExpr* left;
	ASTExpr* right;
};

class ASTExprIdentifier : public ASTExpr
{
public:
    ASTExprIdentifier(string const& name = "",
					  LocationData const& location = LocationData::NONE);
	ASTExprIdentifier(ASTExprIdentifier const& base);
	ASTExprIdentifier& operator=(ASTExprIdentifier const& base);
	ASTExprIdentifier* clone() const {return new ASTExprIdentifier(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprIdentifier(*this, param);}
	string asString() const;
	bool isTypeIdentifier() const {return true;}

	bool isConstant() const {return mIsConstant;}
	void markConstant() {mIsConstant = true;}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
	
	// The identifier components separated by '.'.
	vector<string> components;

	// What this identifier refers to.
	ZScript::Variable* binding;
private:
	bool mIsConstant;
};

class ASTExprArrow : public ASTExpr
{
public:
	ASTExprArrow(ASTExpr* left = NULL,
				 string const& right = "",
				 LocationData const& location = LocationData::NONE);
	ASTExprArrow(ASTExprArrow const& base);
	ASTExprArrow& operator=(ASTExprArrow const& rhs);
	~ASTExprArrow();
	ASTExprArrow* clone() const {return new ASTExprArrow(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprArrow(*this, param);}
	string asString() const;
	bool isTypeArrow() const {return true;}

	bool isConstant() const {return false;}

	ASTExpr* left;
	string right;
	ASTExpr* index;
};

class ASTExprIndex : public ASTExpr
{
public:
	ASTExprIndex(ASTExpr* array = NULL,
				 ASTExpr* index = NULL,
				 LocationData const& location = LocationData::NONE);
	ASTExprIndex(ASTExprIndex const& base);
	~ASTExprIndex() {delete array; delete index;}
	ASTExprIndex& operator=(ASTExprIndex const& rhs);
	ASTExprIndex* clone() const {return new ASTExprIndex(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprIndex(*this, param);}
	bool isTypeIndex() const {return true;}

	bool isConstant() const;

	ASTExpr* array;
	ASTExpr* index;
};

class ASTExprCall : public ASTExpr
{
public:
	ASTExprCall(LocationData const& location = LocationData::NONE);
	ASTExprCall(ASTExprCall const& base);
	~ASTExprCall();
	ASTExprCall& operator=(ASTExprCall const& rhs);
	ASTExprCall* clone() const {return new ASTExprCall(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprCall(*this, param);}

	bool isConstant() const {return false;}

    ASTExpr* left;
    vector<ASTExpr*> parameters;
};

// virtual
class ASTUnaryExpr : public ASTExpr
{
public:
    ASTUnaryExpr(LocationData const& location = LocationData::NONE);
	ASTUnaryExpr(ASTUnaryExpr const& base);
    virtual ~ASTUnaryExpr() {delete operand;}
	virtual ASTUnaryExpr* clone() const = 0;

	virtual bool isConstant() const {return operand->isConstant();}

    ASTExpr* operand;

protected:
	ASTUnaryExpr& operator=(ASTUnaryExpr const& rhs);
};

class ASTExprNegate : public ASTUnaryExpr
{
public:
    ASTExprNegate(LocationData const& location = LocationData::NONE);
	ASTExprNegate(ASTExprNegate const& base);
	ASTExprNegate& operator=(ASTExprNegate const& rhs);
	ASTExprNegate* clone() const {return new ASTExprNegate(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprNegate(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprNot : public ASTUnaryExpr
{
public:
    ASTExprNot(LocationData const& location = LocationData::NONE);
	ASTExprNot(ASTExprNot const& base);
	ASTExprNot& operator=(ASTExprNot const& rhs);
	ASTExprNot* clone() const {return new ASTExprNot(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprNot(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprBitNot : public ASTUnaryExpr
{
public:
    ASTExprBitNot(LocationData const& location = LocationData::NONE);
	ASTExprBitNot(ASTExprBitNot const& base);
	ASTExprBitNot& operator=(ASTExprBitNot const& rhs);
	ASTExprBitNot* clone() const {return new ASTExprBitNot(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprBitNot(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprIncrement : public ASTUnaryExpr
{
public:
    ASTExprIncrement(LocationData const& location = LocationData::NONE);
	ASTExprIncrement(ASTExprIncrement const& base);
	ASTExprIncrement& operator=(ASTExprIncrement const& rhs);
	ASTExprIncrement* clone() const {return new ASTExprIncrement(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprIncrement(*this, param);}

	bool isConstant() const {return false;}
};

class ASTExprPreIncrement : public ASTUnaryExpr
{
public:
    ASTExprPreIncrement(LocationData const& location = LocationData::NONE);
	ASTExprPreIncrement(ASTExprPreIncrement const& base);
	ASTExprPreIncrement& operator=(ASTExprPreIncrement const& rhs);
	ASTExprPreIncrement* clone() const {return new ASTExprPreIncrement(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprPreIncrement(*this, param);}

	bool isConstant() const {return false;}
};

class ASTExprDecrement : public ASTUnaryExpr
{
public:
    ASTExprDecrement(LocationData const& location = LocationData::NONE);
	ASTExprDecrement(ASTExprDecrement const& base);
	ASTExprDecrement& operator=(ASTExprDecrement const& rhs);
	ASTExprDecrement* clone() const {return new ASTExprDecrement(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprDecrement(*this, param);}

	bool isConstant() const {return false;}
};

class ASTExprPreDecrement : public ASTUnaryExpr
{
public:
    ASTExprPreDecrement(LocationData const& location = LocationData::NONE);
	ASTExprPreDecrement(ASTExprPreDecrement const& base);
	ASTExprPreDecrement& operator=(ASTExprPreDecrement const& rhs);
	ASTExprPreDecrement* clone() const {
		return new ASTExprPreDecrement(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprPreDecrement(*this, param);}

	bool isConstant() const {return false;}
};

// virtual
class ASTBinaryExpr : public ASTExpr
{
public:
	ASTBinaryExpr(ASTExpr* left = NULL,
				  ASTExpr* right = NULL,
				  LocationData const& location = LocationData::NONE);
	ASTBinaryExpr(ASTBinaryExpr const& base);
	virtual ~ASTBinaryExpr();
	virtual ASTBinaryExpr* clone() const = 0;

	bool isConstant() const;

	ASTExpr* left;
	ASTExpr* right;

protected:
	ASTBinaryExpr& operator=(ASTBinaryExpr const& rhs);
};

// virtual
class ASTLogExpr : public ASTBinaryExpr
{
public:
	ASTLogExpr(ASTExpr* left = NULL,
			   ASTExpr* right = NULL,
			   LocationData const& location = LocationData::NONE);
	ASTLogExpr(ASTLogExpr const& base);
	virtual ASTLogExpr* clone() const = 0;

protected:
	ASTLogExpr& operator=(ASTLogExpr const& rhs);
};

class ASTExprAnd : public ASTLogExpr
{
public:
	ASTExprAnd(ASTExpr* left = NULL,
			   ASTExpr* right = NULL,
			   LocationData const& location = LocationData::NONE);
	ASTExprAnd(ASTExprAnd const& base);
	ASTExprAnd& operator=(ASTExprAnd const& rhs);
	ASTExprAnd* clone() const {return new ASTExprAnd(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprAnd(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprOr : public ASTLogExpr
{
public:
	ASTExprOr(ASTExpr* left = NULL,
			  ASTExpr* right = NULL,
			  LocationData const& location = LocationData::NONE);
	ASTExprOr(ASTExprOr const& base);
	ASTExprOr& operator=(ASTExprOr const& rhs);
	ASTExprOr* clone() const {return new ASTExprOr(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprOr(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

// virtual
class ASTRelExpr : public ASTBinaryExpr
{
public:
	ASTRelExpr(ASTExpr* left = NULL,
			   ASTExpr* right = NULL,
			   LocationData const& location = LocationData::NONE);
	ASTRelExpr(ASTRelExpr const& base);
	virtual ASTRelExpr* clone() const = 0;

protected:
	ASTRelExpr& operator=(ASTRelExpr const& rhs);
};

class ASTExprGT : public ASTRelExpr
{
public:
	ASTExprGT(ASTExpr* left = NULL,
			  ASTExpr* right = NULL,
			  LocationData const& location = LocationData::NONE);
	ASTExprGT(ASTExprGT const& base);
	ASTExprGT& operator=(ASTExprGT const& rhs);
	ASTExprGT* clone() const {return new ASTExprGT(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprGT(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprGE : public ASTRelExpr
{
public:
	ASTExprGE(ASTExpr* left = NULL,
			  ASTExpr* right = NULL,
			  LocationData const& location = LocationData::NONE);
	ASTExprGE(ASTExprGE const& base);
	ASTExprGE& operator=(ASTExprGE const& rhs);
	ASTExprGE* clone() const {return new ASTExprGE(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprGE(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprLT : public ASTRelExpr
{
public:
	ASTExprLT(ASTExpr* left = NULL,
			  ASTExpr* right = NULL,
			  LocationData const& location = LocationData::NONE);
	ASTExprLT(ASTExprLT const& base);
	ASTExprLT& operator=(ASTExprLT const& rhs);
	ASTExprLT* clone() const {return new ASTExprLT(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprLT(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprLE : public ASTRelExpr
{
public:
	ASTExprLE(ASTExpr* left = NULL,
			  ASTExpr* right = NULL,
			  LocationData const& location = LocationData::NONE);
	ASTExprLE(ASTExprLE const& base);
	ASTExprLE& operator=(ASTExprLE const& rhs);
	ASTExprLE* clone() const {return new ASTExprLE(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprLE(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprEQ : public ASTRelExpr
{
public:
	ASTExprEQ(ASTExpr* left = NULL,
			  ASTExpr* right = NULL,
			  LocationData const& location = LocationData::NONE);
	ASTExprEQ(ASTExprEQ const& base);
	ASTExprEQ& operator=(ASTExprEQ const& rhs);
	ASTExprEQ* clone() const {return new ASTExprEQ(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprEQ(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprNE : public ASTRelExpr
{
public:
	ASTExprNE(ASTExpr* left = NULL,
			  ASTExpr* right = NULL,
			  LocationData const& location = LocationData::NONE);
	ASTExprNE(ASTExprNE const& base);
	ASTExprNE& operator=(ASTExprNE const& rhs);
	ASTExprNE* clone() const {return new ASTExprNE(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprNE(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

// virtual
class ASTAddExpr : public ASTBinaryExpr
{
public:
	ASTAddExpr(ASTExpr* left = NULL,
			   ASTExpr* right = NULL,
			   LocationData const& location = LocationData::NONE);
	ASTAddExpr(ASTAddExpr const& base);
	virtual ASTAddExpr* clone() const = 0;

protected:
	ASTAddExpr& operator=(ASTAddExpr const& rhs);
};

class ASTExprPlus : public ASTAddExpr
{
public:
	ASTExprPlus(ASTExpr* left = NULL,
				ASTExpr* right = NULL,
				LocationData const& location = LocationData::NONE);
	ASTExprPlus(ASTExprPlus const& base);
	ASTExprPlus& operator=(ASTExprPlus const& rhs);
	ASTExprPlus* clone() const {return new ASTExprPlus(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprPlus(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprMinus : public ASTAddExpr
{
public:
	ASTExprMinus(ASTExpr* left = NULL,
				 ASTExpr* right = NULL,
				 LocationData const& location = LocationData::NONE);
	ASTExprMinus(ASTExprMinus const& base);
	ASTExprMinus& operator=(ASTExprMinus const& rhs);
	ASTExprMinus* clone() const {return new ASTExprMinus(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprMinus(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

// virtual
class ASTMultExpr : public ASTBinaryExpr
{
public:
	ASTMultExpr(ASTExpr* left = NULL,
				ASTExpr* right = NULL,
				LocationData const& location = LocationData::NONE);
	ASTMultExpr(ASTMultExpr const& base);
	virtual ASTMultExpr* clone() const = 0;

protected:
	ASTMultExpr& operator=(ASTMultExpr const& rhs);
};

class ASTExprTimes : public ASTMultExpr
{
public:
	ASTExprTimes(ASTExpr* left = NULL,
				 ASTExpr* right = NULL,
				 LocationData const& location = LocationData::NONE);
	ASTExprTimes(ASTExprTimes const& base);
	ASTExprTimes& operator=(ASTExprTimes const& rhs);
	ASTExprTimes* clone() const {return new ASTExprTimes(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprTimes(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprDivide : public ASTMultExpr
{
public:
	ASTExprDivide(ASTExpr* left = NULL,
				  ASTExpr* right = NULL,
				  LocationData const& location = LocationData::NONE);
	ASTExprDivide(ASTExprDivide const& base);
	ASTExprDivide& operator=(ASTExprDivide const& rhs);
	ASTExprDivide* clone() const {return new ASTExprDivide(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprDivide(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprModulo : public ASTMultExpr
{
public:
	ASTExprModulo(ASTExpr* left = NULL,
				  ASTExpr* right = NULL,
				  LocationData const& location = LocationData::NONE);
	ASTExprModulo(ASTExprModulo const& base);
	ASTExprModulo& operator=(ASTExprModulo const& rhs);
	ASTExprModulo* clone() const {return new ASTExprModulo(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprModulo(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

// virtual
class ASTBitExpr : public ASTBinaryExpr
{
public:
	ASTBitExpr(ASTExpr* left = NULL, ASTExpr* right = NULL,
			LocationData const& location = LocationData::NONE);
	ASTBitExpr(ASTBitExpr const& base);
	virtual ASTBitExpr* clone() const = 0;

protected:
	ASTBitExpr& operator=(ASTBitExpr const& rhs);
};

class ASTExprBitAnd : public ASTBitExpr
{
public:
	ASTExprBitAnd(ASTExpr* left = NULL, ASTExpr* right = NULL,
			LocationData const& location = LocationData::NONE);
	ASTExprBitAnd(ASTExprBitAnd const& base);
	ASTExprBitAnd& operator=(ASTExprBitAnd const& rhs);
	ASTExprBitAnd* clone() const {return new ASTExprBitAnd(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprBitAnd(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprBitOr : public ASTBitExpr
{
public:
	ASTExprBitOr(ASTExpr* left = NULL, ASTExpr* right = NULL,
			LocationData const& location = LocationData::NONE);
	ASTExprBitOr(ASTExprBitOr const& base);
	ASTExprBitOr& operator=(ASTExprBitOr const& rhs);
	ASTExprBitOr* clone() const {return new ASTExprBitOr(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprBitOr(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprBitXor : public ASTBitExpr
{
public:
	ASTExprBitXor(ASTExpr* left = NULL, ASTExpr* right = NULL,
			LocationData const& location = LocationData::NONE);
	ASTExprBitXor(ASTExprBitXor const& base);
	ASTExprBitXor& operator=(ASTExprBitXor const& rhs);
	ASTExprBitXor* clone() const {return new ASTExprBitXor(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprBitXor(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

// virtual
class ASTShiftExpr : public ASTBinaryExpr
{
public:
	ASTShiftExpr(
			ASTExpr* left = NULL, ASTExpr* right = NULL,
			LocationData const& location = LocationData::NONE);
	ASTShiftExpr(ASTShiftExpr const& base);
	virtual ASTShiftExpr* clone() const = 0;

protected:
	ASTShiftExpr& operator=(ASTShiftExpr const& rhs);
};

class ASTExprLShift : public ASTShiftExpr
{
public:
	ASTExprLShift(ASTExpr* left = NULL, ASTExpr* right = NULL,
			LocationData const& location = LocationData::NONE);
	ASTExprLShift(ASTExprLShift const& base);
	ASTExprLShift& operator=(ASTExprLShift const& rhs);
	ASTExprLShift* clone() const {return new ASTExprLShift(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprLShift(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

class ASTExprRShift : public ASTShiftExpr
{
public:
	ASTExprRShift(ASTExpr* left = NULL, ASTExpr* right = NULL,
			LocationData const& location = LocationData::NONE);
	ASTExprRShift(ASTExprRShift const& base);
	ASTExprRShift& operator=(ASTExprRShift const& rhs);
	ASTExprRShift* clone() const {return new ASTExprRShift(*this);}

	void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseExprRShift(*this, param);}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
};

// Literals

namespace ZScript
{
	class Literal;
}

// virtual
class ASTLiteral : public ASTExpr
{
public:
	ASTLiteral(LocationData const& location = LocationData::NONE);
	ASTLiteral(ASTLiteral const& base) : ASTExpr(base), manager(NULL) {}
	virtual ASTLiteral* clone() const = 0;
	
	ZScript::Literal* manager;

protected:
	ASTLiteral& operator=(ASTLiteral const& rhs);
};

class ASTNumberLiteral : public ASTLiteral
{
public:
    ASTNumberLiteral(
			ASTFloat* value = NULL,
			LocationData const& location = LocationData::NONE);
	ASTNumberLiteral(ASTNumberLiteral const& base);
    ~ASTNumberLiteral() {delete value;}
	ASTNumberLiteral& operator=(ASTNumberLiteral const& rhs);
	ASTNumberLiteral* clone() const {return new ASTNumberLiteral(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseNumberLiteral(*this, param);}

	bool isConstant() const {return true;}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const;
	
    ASTFloat* value;
};

class ASTBoolLiteral : public ASTLiteral
{
public:
    ASTBoolLiteral(
			bool value = false,
			LocationData const& location = LocationData::NONE);
	ASTBoolLiteral(ASTBoolLiteral const& base);
	ASTBoolLiteral& operator=(ASTBoolLiteral const& base);
	ASTBoolLiteral* clone() const {return new ASTBoolLiteral(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseBoolLiteral(*this, param);}

	bool isConstant() const {return true;}

	optional<long> getCompileTimeValue(
			CompileErrorHandler* errorHandler = NULL)
			const
	{return value ? 10000L : 0L;}
	
    bool value;
};

class ASTStringLiteral : public ASTLiteral
{
public:
	ASTStringLiteral(
			char const* str = "",
			LocationData const& location = LocationData::NONE);
	ASTStringLiteral(
			string const& str,
			LocationData const& location = LocationData::NONE);
	ASTStringLiteral(ASTString const& raw);
	ASTStringLiteral(ASTStringLiteral const& base);
	ASTStringLiteral& operator=(ASTStringLiteral const& rhs);
	ASTStringLiteral* clone() const {return new ASTStringLiteral(*this);}

	void execute (ASTVisitor& visitor, void* param = NULL) {
		visitor.caseStringLiteral(*this, param);}
	bool isStringLiteral() const {return true;}

	bool isConstant() const {return true;}

	// The data declaration that this literal may be part of. If NULL that
	// means this is not part of a data declaration. This should be managed by
	// that declaration and not modified by this object at all.
	ASTDataDecl* declaration;

	string value;
};

class ASTArrayLiteral : public ASTLiteral
{
public:
	ASTArrayLiteral(LocationData const& location = LocationData::NONE);
	ASTArrayLiteral(ASTArrayLiteral const& base);
	~ASTArrayLiteral();
	ASTArrayLiteral& operator=(ASTArrayLiteral const& rhs);
	ASTArrayLiteral* clone() const {return new ASTArrayLiteral(*this);}

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

	// Optional type specification.
	ASTVarType* type;
	// Optional size specification.
	ASTExpr* size;
	// The array elements.
	vector<ASTExpr*> elements;
};

// Types

class ASTScriptType : public AST
{
public:
	ASTScriptType(
			ScriptType type = SCRIPTTYPE_VOID,
			LocationData const& location = LocationData::NONE);
	ASTScriptType(ASTScriptType const& base);
	ASTScriptType& operator=(ASTScriptType const& rhs);
	ASTScriptType* clone() const {return new ASTScriptType(*this);}

    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseScriptType(*this, param);}

	ScriptType type;
};

class ASTVarType : public AST
{
public:
	// Takes ownership of type.
    ASTVarType(
			ZVarType* type = NULL,
			LocationData const& location = LocationData::NONE);
	// Clones type.
    ASTVarType(
			ZVarType const& type,
			LocationData const& location = LocationData::NONE);
	ASTVarType(ASTVarType const& base);
	~ASTVarType() {delete type;}
	ASTVarType& operator=(ASTVarType const& rhs);
	ASTVarType* clone() const {return new ASTVarType(*this);}
	
    void execute(ASTVisitor& visitor, void* param = NULL) {
		visitor.caseVarType(*this, param);}

	ZVarType const& resolve(Scope& scope);

	// Owned by this object.
	ZVarType* type;
};

#endif


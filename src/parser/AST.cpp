#include "../precompiled.h" //always first
#include "AST.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "Scope.h"

#include <assert.h>

////////////////////////////////////////////////////////////////

// AST

AST& AST::operator=(AST const& rhs)
{
	loc = rhs.loc;
	return *this;
}

// ASTProgram

ASTProgram::ASTProgram(LocationData const& location) : AST(location) {}

ASTProgram::ASTProgram(ASTProgram const& base) : AST(base.getLocation())
{
	for (vector<ASTImportDecl*>::const_iterator it = base.imports.begin();
		 it != base.imports.end(); ++it)
		imports.push_back((*it)->clone());
	for (vector<ASTDataDeclList*>::const_iterator it = base.variables.begin();
		 it != base.variables.end(); ++it)
		variables.push_back((*it)->clone());
	for (vector<ASTFuncDecl*>::const_iterator it = base.functions.begin();
		 it != base.functions.end(); ++it)
		functions.push_back((*it)->clone());
	for (vector<ASTTypeDef*>::const_iterator it = base.types.begin();
		 it != base.types.end(); ++it)
		types.push_back((*it)->clone());
	for (vector<ASTScript*>::const_iterator it = base.scripts.begin();
		 it != base.scripts.end(); ++it)
		scripts.push_back((*it)->clone());
}

ASTProgram::~ASTProgram()
{
	for (vector<ASTImportDecl*>::const_iterator it = imports.begin();
		 it != imports.end(); ++it)
		delete *it;
	for (vector<ASTDataDeclList*>::const_iterator it = variables.begin();
		 it != variables.end(); ++it)
		delete *it;
	for (vector<ASTFuncDecl*>::const_iterator it = functions.begin();
		 it != functions.end(); ++it)
		delete *it;
	for (vector<ASTTypeDef*>::const_iterator it = types.begin();
		 it != types.end(); ++it)
		delete *it;
	for (vector<ASTScript*>::const_iterator it = scripts.begin();
		 it != scripts.end(); ++it)
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
	case ASTDECL_CLASSID_DATALIST:
		variables.push_back((ASTDataDeclList*)declaration);
		break;
	case ASTDECL_CLASSID_TYPE:
		types.push_back((ASTTypeDef*)declaration);
		break;
	}
}

ASTProgram& ASTProgram::merge(ASTProgram& other)
{
	for (vector<ASTImportDecl*>::const_iterator it = other.imports.begin();
		 it != other.imports.end(); ++it)
		imports.push_back(*it);
	other.imports.clear();
	for (vector<ASTDataDeclList*>::const_iterator it = other.variables.begin();
		 it != other.variables.end(); ++it)
		variables.push_back(*it);
	other.variables.clear();
	for (vector<ASTFuncDecl*>::const_iterator it = other.functions.begin();
		 it != other.functions.end(); ++it)
		functions.push_back(*it);
	other.functions.clear();
	for (vector<ASTTypeDef*>::const_iterator it = other.types.begin();
		 it != other.types.end(); ++it)
		types.push_back(*it);
	other.types.clear();
	for (vector<ASTScript*>::const_iterator it = other.scripts.begin();
		 it != other.scripts.end(); ++it)
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

ASTStmt& ASTStmt::operator=(ASTStmt const& rhs)
{
	AST::operator=(rhs);
	return *this;
}

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

// ASTCompileError

ASTCompileError::ASTCompileError(
		ASTExpr* errorId, ASTStmt* statement,
		LocationData const& location)
	: ASTStmt(location), errorId(errorId), statement(statement),
	  errorTriggered(false)
{}

ASTCompileError::ASTCompileError(ASTCompileError const& base)
	: ASTStmt(base),
	  errorId(base.errorId ? base.errorId->clone() : NULL),
	  statement(base.statement ? base.statement->clone() : NULL),
	  errorTriggered(base.errorTriggered)
	  
{}

ASTCompileError::~ASTCompileError()
{
	delete errorId;
	delete statement;
}

ASTCompileError& ASTCompileError::operator=(ASTCompileError const& rhs)
{
	ASTStmt::operator=(rhs);
	// Delete.
	delete errorId;
	delete statement;
	// Copy.
	errorId = rhs.errorId ? rhs.errorId->clone() : NULL;
	statement = rhs.statement ? rhs.statement->clone() : NULL;
	errorTriggered = rhs.errorTriggered;
	// Return.
	return *this;
}

int ASTCompileError::getErrorId() const
{
	if (!errorId) return -1;
	if (!errorId->hasDataValue()) return -1;
	return errorId->getDataValue() / 10000;
}

bool ASTCompileError::canHandle(CompileError const& error) const
{
	return error.id == getErrorId();
}

////////////////////////////////////////////////////////////////
// Declarations

// ASTDecl

ASTDecl& ASTDecl::operator=(ASTDecl const& rhs)
{
	ASTStmt::operator=(rhs);
	return *this;
}

// ASTScript

ASTScript::ASTScript(LocationData const& location)
	: ASTDecl(location), type(NULL), name("") {}

ASTScript::ASTScript(ASTScript const& base)
	: ASTDecl(base.getLocation()),
	  type(base.type ? base.type->clone() : NULL),
	  name(base.name)
{
	for (vector<ASTDataDeclList*>::const_iterator it = base.variables.begin();
		 it != base.variables.end(); ++it)
		variables.push_back((*it)->clone());
	for (vector<ASTFuncDecl*>::const_iterator it = base.functions.begin();
		 it != base.functions.end(); ++it)
		functions.push_back((*it)->clone());
	for (vector<ASTTypeDef*>::const_iterator it = base.types.begin();
		 it != base.types.end(); ++it)
		types.push_back((*it)->clone());
}

ASTScript::~ASTScript()
{
	delete type;
	for (vector<ASTDataDeclList*>::const_iterator it = variables.begin();
		 it != variables.end(); ++it)
		delete *it;
	for (vector<ASTFuncDecl*>::const_iterator it = functions.begin();
		 it != functions.end(); ++it)
		delete *it;
	for (vector<ASTTypeDef*>::const_iterator it = types.begin();
		 it != types.end(); ++it)
		delete *it;
}

ASTScript& ASTScript::operator=(ASTScript const& rhs)
{
	ASTDecl::operator=(rhs);
	// Delete.
	delete type;
	for (vector<ASTDataDeclList*>::const_iterator it = variables.begin();
		 it != variables.end(); ++it)
		delete *it;
	for (vector<ASTFuncDecl*>::const_iterator it = functions.begin();
		 it != functions.end(); ++it)
		delete *it;
	for (vector<ASTTypeDef*>::const_iterator it = types.begin();
		 it != types.end(); ++it)
		delete *it;
	// Copy.
	for (vector<ASTDataDeclList*>::const_iterator it = rhs.variables.begin();
		 it != rhs.variables.end(); ++it)
		variables.push_back((*it)->clone());
	for (vector<ASTFuncDecl*>::const_iterator it = rhs.functions.begin();
		 it != rhs.functions.end(); ++it)
		functions.push_back((*it)->clone());
	for (vector<ASTTypeDef*>::const_iterator it = rhs.types.begin();
		 it != rhs.types.end(); ++it)
		types.push_back((*it)->clone());
	type = rhs.type ? rhs.type->clone() : NULL;
	name = rhs.name;
	// Return.
	return *this;
}

void ASTScript::addDeclaration(ASTDecl& declaration)
{
	switch (declaration.declarationClassId())
	{
	case ASTDECL_CLASSID_FUNCTION:
		functions.push_back(&(ASTFuncDecl&)declaration);
		break;
	case ASTDECL_CLASSID_DATALIST:
		variables.push_back(&(ASTDataDeclList&)declaration);
		break;
	case ASTDECL_CLASSID_TYPE:
		types.push_back(&(ASTTypeDef&)declaration);
		break;
	}
}

// ASTImportDecl

ASTImportDecl* ASTImportDecl::clone() const
{
	return new ASTImportDecl(filename, getLocation());
}

// ASTFuncDecl

ASTFuncDecl::ASTFuncDecl(LocationData const& location)
	: ASTDecl(location), returnType(NULL), block(NULL)
{}

ASTFuncDecl::ASTFuncDecl(ASTFuncDecl const& base)
	: ASTDecl(base),
	  returnType(base.returnType ? base.returnType->clone() : NULL),
	  name(base.name),
	  block(base.block ? base.block->clone() : NULL)
{
	for (vector<ASTDataDecl*>::const_iterator it = base.parameters.begin();
		 it != base.parameters.end(); ++it)
		parameters.push_back((*it)->clone());
}

ASTFuncDecl& ASTFuncDecl::operator=(ASTFuncDecl const& rhs)
{
	ASTDecl::operator=(rhs);
	// Delete.
	delete returnType;
	delete block;
	for (vector<ASTDataDecl*>::iterator it = parameters.begin();
		 it != parameters.end(); ++it)
		delete *it;
	// Copy.
	returnType = rhs.returnType ? rhs.returnType->clone() : NULL;
	name = rhs.name;
	block = rhs.block ? rhs.block->clone() : NULL;
	for (vector<ASTDataDecl*>::const_iterator it = rhs.parameters.begin();
		 it != rhs.parameters.end(); ++it)
		parameters.push_back((*it)->clone());
	// Return.
	return *this;
}

ASTFuncDecl::~ASTFuncDecl()
{
	delete returnType;
	delete block;
	for (vector<ASTDataDecl*>::iterator it = parameters.begin();
		 it != parameters.end(); ++it)
		delete *it;
}

void ASTFuncDecl::addParameter(ASTDataDecl* parameter)
{
	parameters.push_back(parameter);
}

// ASTDataDeclList

ASTDataDeclList::ASTDataDeclList(LocationData const& location)
	: ASTDecl(location), baseType(NULL)
{}

ASTDataDeclList::ASTDataDeclList(ASTDataDeclList const& base)
	: ASTDecl(base), baseType(base.baseType)
{
	for (vector<ASTDataDecl*>::const_iterator it = base.declarations.begin();
		 it != base.declarations.end(); ++it)
		declarations.push_back((*it)->clone());
}

ASTDataDeclList& ASTDataDeclList::operator=(ASTDataDeclList const& rhs)
{
	ASTDecl::operator=(rhs);
	// Delete.
	for (vector<ASTDataDecl*>::iterator it = declarations.begin();
		 it != declarations.end(); ++it)
		delete *it;
	// Copy.
    baseType = rhs.baseType;
	for (vector<ASTDataDecl*>::const_iterator it = rhs.declarations.begin();
		 it != rhs.declarations.end(); ++it)
		declarations.push_back((*it)->clone());
	// Return.
	return *this;
}

ASTDataDeclList::~ASTDataDeclList()
{
	for (vector<ASTDataDecl*>::iterator it = declarations.begin();
		 it != declarations.end(); ++it)
		delete *it;
}

void ASTDataDeclList::addDeclaration(ASTDataDecl* declaration)
{
	// Declarations in a list should not have their own type.
	assert(!declaration->baseType);

	declaration->list = this;
	declarations.push_back(declaration);
}

// ASTDataDecl

ASTDataDecl::ASTDataDecl(LocationData const& location)
	: ASTDecl(location), list(NULL), manager(NULL),
	  baseType(NULL), initializer(NULL)
{}

ASTDataDecl::ASTDataDecl(ASTDataDecl const& base)
	: ASTDecl(base),
	  baseType(base.baseType ? base.baseType->clone() : NULL),
	  name(base.name),
	  initializer(base.initializer ? base.initializer->clone() : NULL),
	  // list and manager indicate being this object being "owned" by them, so
	  // it doesn't make sense for a copy to keep those values.
	  list(NULL), manager(NULL)
{
	for (vector<ASTDataDeclExtraArray*>::const_iterator it = base.extraArrays.begin();
		 it != base.extraArrays.end(); ++it)
		extraArrays.push_back((*it)->clone());
}


ASTDataDecl::~ASTDataDecl()
{
	delete baseType;
	for (vector<ASTDataDeclExtraArray*>::const_iterator it = extraArrays.begin();
		 it != extraArrays.end(); ++it)
		delete *it;
	delete initializer;
}
ASTDataDecl& ASTDataDecl::operator=(ASTDataDecl const& rhs)
{
	ASTDecl::operator=(rhs);
	// Delete.
	delete baseType;
	for (vector<ASTDataDeclExtraArray*>::const_iterator it = extraArrays.begin();
		 it != extraArrays.end(); ++it)
		delete *it;
	delete initializer;
	// Copy.
	list = NULL;
	manager = NULL;
	baseType = rhs.baseType ? rhs.baseType->clone() : NULL;
	name = rhs.name;
	for (vector<ASTDataDeclExtraArray*>::const_iterator it = rhs.extraArrays.begin();
		 it != rhs.extraArrays.end(); ++it)
		extraArrays.push_back((*it)->clone());
	initializer = rhs.initializer ? rhs.initializer->clone() : NULL;
	// Return.
	return *this;
}

void ASTDataDecl::setInitializer(ASTExpr* initializer)
{
	this->initializer = initializer;

	// Give a string or array literal a reference back to this object so it
	// can grab size information.
	if (extraArrays.size() > 0)
	{
		if (initializer->isArrayLiteral())
		{
			ASTArrayLiteral& arrayLiteral = *(ASTArrayLiteral*)initializer;
			arrayLiteral.declaration = this;
		}
		if (initializer->isStringLiteral())
		{
			ASTStringLiteral& stringLiteral = *(ASTStringLiteral*)initializer;
			stringLiteral.declaration = this;
		}
	}
}

ZVarType const* ASTDataDecl::resolveType(Scope* scope) const
{
	SymbolTable& table = scope->getTable();

	// First resolve the base type.
	ASTVarType* baseTypeNode = list ? list->baseType : baseType;
	ZVarType const* type = &baseTypeNode->resolve(*scope);

	// If we have any arrays, tack them onto the base type.
	for (vector<ASTDataDeclExtraArray*>::const_iterator it = extraArrays.begin();
		 it != extraArrays.end(); ++it)
	{
		ZVarTypeArray arrayType(*type);
		type = table.getCanonicalType(arrayType);
	}

	return type;
}

// ASTDataDeclExtraArray

ASTDataDeclExtraArray::ASTDataDeclExtraArray(LocationData const& location)
	: AST(location)
{}

ASTDataDeclExtraArray::ASTDataDeclExtraArray(ASTDataDeclExtraArray const& base)
	: AST(base)
{
	for (vector<ASTExpr*>::const_iterator it = base.dimensions.begin();
		 it != base.dimensions.end(); ++it)
		dimensions.push_back((*it)->clone());
}

ASTDataDeclExtraArray& ASTDataDeclExtraArray::operator=(ASTDataDeclExtraArray const& rhs)
{
	AST::operator=(rhs);
	// Delete.
	for (vector<ASTExpr*>::iterator it = dimensions.begin();
		 it != dimensions.end(); ++it)
		delete *it;
	// Copy.
	for (vector<ASTExpr*>::const_iterator it = rhs.dimensions.begin();
		 it != rhs.dimensions.end(); ++it)
		dimensions.push_back((*it)->clone());
	// Return.
	return *this;
}

ASTDataDeclExtraArray::~ASTDataDeclExtraArray()
{
	for (vector<ASTExpr*>::iterator it = dimensions.begin();
		 it != dimensions.end(); ++it)
		delete *it;
}

bool ASTDataDeclExtraArray::isConstant() const
{
	for (vector<ASTExpr*>::const_iterator it = dimensions.begin();
		 it != dimensions.end(); ++it)
		if (!(*it)->hasDataValue()) return false;
	return true;
}

int ASTDataDeclExtraArray::getTotalSize() const
{
	if (dimensions.size() == 0) return -1;
	long size = 1;
	for (vector<ASTExpr*>::const_iterator it = dimensions.begin();
		 it != dimensions.end(); ++it)
	{
		ASTExpr& expr = **it;
		if (!expr.hasDataValue()) return -1;
		size *= expr.getDataValue() / 10000L;
	}
	return size;
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

ASTExpr::ASTExpr(ASTExpr const& base)
	: ASTStmt(base), hasValue(base.hasValue), value(base.value), lval(false), varType(base.varType)
{}

ASTExpr& ASTExpr::operator=(ASTExpr const& rhs)
{
	ASTStmt::operator=(rhs);
	lval = rhs.lval;
	hasValue = rhs.hasValue;
	value = rhs.value;
	varType = rhs.varType;
	return *this;
}



// ASTExprConst

ASTExprConst::ASTExprConst(ASTExprConst const& base) : ASTExpr(base)
{
	content = base.content ? base.content->clone() : NULL;
}

ASTExprConst& ASTExprConst::operator=(ASTExprConst const& rhs)
{
	ASTExpr::operator=(rhs);
	content = rhs.content->clone();
	return *this;
}

// ASTExprAssign

ASTExprAssign::~ASTExprAssign()
{
	delete lval;
	delete rval;
}

ASTExprAssign* ASTExprAssign::clone() const
{
	return new ASTExprAssign(
			lval != NULL ? lval->clone() : NULL,
			rval != NULL ? rval->clone() : NULL,
			getLocation());
}

// ASTExprIdentifier

ASTExprIdentifier::ASTExprIdentifier(string const& name, LocationData const& location)
	: ASTExpr(location), isConstant_(false)
{
	components.push_back(name);
}

ASTExprIdentifier::ASTExprIdentifier(ASTExprIdentifier const& base)
	: ASTExpr(base), isConstant_(base.isConstant_), components(base.components)
{}

ASTExprIdentifier& ASTExprIdentifier::operator=(ASTExprIdentifier const& base)
{
	ASTExpr::operator=(base);
	components = base.components;
	isConstant_ = base.isConstant_;
	return *this;
}

string ASTExprIdentifier::asString() const
{
	string s = components.front();
	for (vector<string>::const_iterator it = components.begin() + 1;
	   it != components.end();
	   ++it)
	{
		s = s + "." + *it;
	}

	return s;
}

// ASTExprArrow

ASTExprArrow::ASTExprArrow(ASTExpr* left, string const& right, LocationData const& location)
	: ASTExpr(location), left(left), right(right), index(NULL)
{}

ASTExprArrow::ASTExprArrow(ASTExprArrow const& base)
	: ASTExpr(base), left(base.left->clone()), right(base.right),
	  index(base.index ? base.index->clone() : NULL)
{}

ASTExprArrow& ASTExprArrow::operator=(ASTExprArrow const& rhs)
{
	ASTExpr::operator=(rhs);
	delete left;
	left = rhs.left->clone();
	right = rhs.right;
	delete index;
	index = rhs.index ? rhs.index->clone() : NULL;
	return *this;
}

ASTExprArrow::~ASTExprArrow()
{
	delete left;
	delete index;
}

string ASTExprArrow::asString() const
{
	string s = left->asString() + "->" + right;
	if (index != NULL) s += "[" + index->asString() + "]";
	return s;
}

// ASTExprIndex

ASTExprIndex::ASTExprIndex(ASTExpr* array, ASTExpr* index, LocationData const& location)
	: ASTExpr(location), array(array), index(index)
{}

ASTExprIndex::ASTExprIndex(ASTExprIndex const& base)
	: ASTExpr(base.getLocation()),
	  array(base.array ? base.array->clone() : NULL),
	  index(base.index ? base.index->clone() : NULL)
{}

ASTExprIndex& ASTExprIndex::operator=(ASTExprIndex const& rhs)
{
	ASTExpr::operator=(rhs);
	array = rhs.array ? rhs.array->clone() : NULL;
	index = rhs.index ? rhs.index->clone() : NULL;
	return *this;
}

bool ASTExprIndex::isConstant() const
{
	if (array == NULL || index == NULL) return false;
	return array->isConstant() && index->isConstant();
}


// ASTExprCall

ASTExprCall::ASTExprCall(LocationData const& location)
	: ASTExpr(location), left(NULL)
{}

ASTExprCall::ASTExprCall(ASTExprCall const& base)
	: ASTExpr(base), left(base.left)
{
	for (list<ASTExpr*>::const_iterator it = base.params.begin(); it != base.params.end(); ++it)
		params.push_back((*it)->clone());
}

ASTExprCall& ASTExprCall::operator=(ASTExprCall const& rhs)
{
	ASTExpr::operator=(rhs);
	delete left;
	left = rhs.left;
	for (list<ASTExpr*>::iterator it = params.begin(); it != params.end(); ++it)
		delete *it;
	for (list<ASTExpr*>::const_iterator it = rhs.params.begin(); it != rhs.params.end(); ++it)
		params.push_back((*it)->clone());
	return *this;
}

ASTExprCall::~ASTExprCall()
{
    delete left;
	for (list<ASTExpr*>::iterator it = params.begin(); it != params.end(); ++it)
		delete *it;
}

// ASTUnaryExpr

ASTUnaryExpr::ASTUnaryExpr(ASTUnaryExpr const& base)
	: ASTExpr(base), operand(base.operand ? base.operand->clone() : NULL)
{}

ASTUnaryExpr& ASTUnaryExpr::operator=(ASTUnaryExpr const& rhs)
{
	ASTExpr::operator=(rhs);
	delete operand;
	operand = rhs.operand ? rhs.operand->clone() : NULL;
	return *this;
}

// ASTExprNegate

// ASTExprNot

// ASTExprBitNot

// ASTExprIncrement

// ASTExprPreIncrement

// ASTExprDecrement

// ASTExprPreDecrement

// ASTBinaryExpr

ASTBinaryExpr::ASTBinaryExpr(ASTBinaryExpr const& base)
	: ASTExpr(base),
	  first(base.first ? base.first->clone() : NULL),
	  second(base.second ? base.second->clone() : NULL)
{}

ASTBinaryExpr& ASTBinaryExpr::operator=(ASTBinaryExpr const& rhs)
{
	ASTExpr::operator=(rhs);
	delete first;
	first = rhs.first ? rhs.first->clone() : NULL;
	delete second;
	second = rhs.second ? rhs.second->clone() : NULL;
	return *this;
}

ASTBinaryExpr::~ASTBinaryExpr()
{
	delete first;
	delete second;
}

// ASTLogExpr

// ASTExprAnd

// ASTExprOr

// ASTRelExpr

// ASTExprGT

// ASTExprGE

// ASTExprLT

// ASTExprLE

// ASTExprEQ

// ASTExprNE

// ASTAddExpr

// ASTExprPlus

// ASTExprMinus

// ASTMultExpr

// ASTExprTimes

// ASTExprDivide

// ASTExprModulo

// ASTBitExpr

// ASTExprBitAnd

// ASTExprBitOr

// ASTExprBitXor

// ASTShiftExpr

// ASTExprLShift

// ASTExprRShift

////////////////////////////////////////////////////////////////
// Literals

// ASTLiteral

ASTLiteral& ASTLiteral::operator=(ASTLiteral const& rhs)
{
	ASTExpr::operator=(rhs);
	manager = rhs.manager;
	return *this;
}

// ASTNumberLiteral

ASTNumberLiteral::ASTNumberLiteral(ASTNumberLiteral const& base) : ASTLiteral(base)
{
	val = base.val ? base.val->clone() : NULL;
}

ASTNumberLiteral& ASTNumberLiteral::operator=(ASTNumberLiteral const& rhs)
{
	ASTLiteral::operator=(rhs);
	delete val;
	val = rhs.val ? rhs.val->clone() : NULL;
	return *this;
}

// ASTBoolLiteral

ASTBoolLiteral::ASTBoolLiteral(ASTBoolLiteral const& base)
	: ASTLiteral(base), value(base.value)
{}

ASTBoolLiteral& ASTBoolLiteral::operator=(ASTBoolLiteral const& rhs)
{
	ASTLiteral::operator=(rhs);
	value = rhs.value;
	return *this;
}

// ASTStringLiteral

ASTStringLiteral::ASTStringLiteral(char const* str, LocationData const& location)
	: ASTLiteral(location), data(str), declaration(NULL)
{}

ASTStringLiteral::ASTStringLiteral(string const& str, LocationData const& location)
	: ASTLiteral(location), data(str), declaration(NULL)
{}

ASTStringLiteral::ASTStringLiteral(ASTString const& raw)
	: ASTLiteral(raw.getLocation()),
	  data(raw.getValue().substr(1, raw.getValue().size() - 2)),
	  declaration(NULL)
{}

ASTStringLiteral::ASTStringLiteral(ASTStringLiteral const& base)
	: ASTLiteral(base), data(base.data),
	  // declaration field is managed by the declaration itself, so it stays
	  // NULL regardless.
	  declaration(NULL)
{}

ASTStringLiteral& ASTStringLiteral::operator=(ASTStringLiteral const& rhs)
{
	ASTLiteral::operator=(rhs);
	data = rhs.data;
	declaration = NULL;
	return *this;
}

// ASTArrayLiteral

ASTArrayLiteral::ASTArrayLiteral(LocationData const& location)
	: ASTLiteral(location), type(NULL), size(NULL), declaration(NULL)
{}

ASTArrayLiteral::ASTArrayLiteral(ASTArrayLiteral const& base)
	: ASTLiteral(base),
	  type(base.type ? base.type->clone() : NULL),
	  size(base.size ? base.size->clone() : NULL),
	  // declaration field is managed by the declaration itself, so it stays
	  // NULL regardless.
	  declaration(NULL)
{
	for (vector<ASTExpr*>::const_iterator it = base.elements.begin();
		 it != base.elements.end(); ++it)
		elements.push_back((*it)->clone());
}

ASTArrayLiteral& ASTArrayLiteral::operator=(ASTArrayLiteral const& rhs)
{
	ASTLiteral::operator=(rhs);
	// Delete.
	delete type;
	delete size;
	for (vector<ASTExpr*>::iterator it = elements.begin();
		 it != elements.end(); ++it)
		delete *it;
	// Copy.
	type = rhs.type ? rhs.type->clone() : NULL;
	size = rhs.size ? rhs.size->clone() : NULL;
	declaration = NULL;
	for (vector<ASTExpr*>::const_iterator it = rhs.elements.begin();
		 it != rhs.elements.end(); ++it)
		elements.push_back((*it)->clone());
	// Return.
	return *this;
}

ASTArrayLiteral::~ASTArrayLiteral()
{
	delete type;
	delete size;
	for (vector<ASTExpr*>::iterator it = elements.begin();
		 it != elements.end(); ++it)
		delete *it;
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

#include "../precompiled.h" //always first //2.53 Updated to 16th Jan, 2017
#include "AST.h"
#include "CompileError.h"
#include "CompilerUtils.h"
#include "DataStructs.h"
#include "Scope.h"

#include <assert.h>

////////////////////////////////////////////////////////////////

// AST

AST::AST(LocationData const& location) : location(location) {}

AST::AST(AST const& base) : location(base.location) {}

AST& AST::operator=(AST const& rhs)
{
	location = rhs.location;
	return *this;
}

// ASTProgram

ASTProgram::ASTProgram(LocationData const& location) : AST(location) {}

ASTProgram::ASTProgram(ASTProgram const& base)
	: AST(base),
	  imports(AST::clone(base.imports)),
	  variables(AST::clone(base.variables)),
	  functions(AST::clone(base.functions)),
	  types(AST::clone(base.types)),
	  scripts(AST::clone(base.scripts))
{}

ASTProgram::~ASTProgram()
{
	deleteElements(imports);
	deleteElements(variables);
	deleteElements(functions);
	deleteElements(types);
	deleteElements(scripts);
}

ASTProgram& ASTProgram::operator=(ASTProgram const& rhs)
{
	AST::operator=(rhs);

	deleteElements(imports);
	deleteElements(variables);
	deleteElements(functions);
	deleteElements(types);
	deleteElements(scripts);

	imports = AST::clone(rhs.imports);
	variables = AST::clone(rhs.variables);
	functions = AST::clone(rhs.functions);
	types = AST::clone(rhs.types);
	scripts = AST::clone(rhs.scripts);

	return *this;
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
	imports.insert(imports.end(), other.imports.begin(), other.imports.end());
	other.imports.clear();
	variables.insert(variables.end(), other.variables.begin(),
					 other.variables.end());
	other.variables.clear();
	functions.insert(functions.end(), other.functions.begin(),
					 other.functions.end());
	other.functions.clear();
	types.insert(types.end(), other.types.begin(), other.types.end());
	other.types.clear();
	scripts.insert(scripts.end(), other.scripts.begin(), other.scripts.end());
	other.scripts.clear();
	return *this;
}

// ASTFloat

ASTFloat::ASTFloat(char* value, Type type, LocationData const& location)
	: AST(location), type(type), negative(false), value((string)value)
{}
    
ASTFloat::ASTFloat(char const* value, Type type, LocationData const& location)
	: AST(location), type(type), negative(false), value((string)value)
{}
    
ASTFloat::ASTFloat(
		string const& value, Type type, LocationData const& location)
	: AST(location), type(type), negative(false), value(value)
{}

ASTFloat::ASTFloat(long value, Type type, LocationData const& location)
	: AST(location), type(type), negative(false)
{
	char tmp[15];
	sprintf(tmp, "%ld", value);
	ASTFloat::value = string(tmp);
}

ASTFloat::ASTFloat(ASTFloat const& base)
	: AST(base), type(base.type), value(base.value), negative(base.negative)
{}

ASTFloat& ASTFloat::operator=(ASTFloat const& rhs)
{
	AST::operator=(rhs);

	type = rhs.type;
	value = rhs.value;
	negative = rhs.negative;

	return *this;
}

pair<string, string> ASTFloat::parseValue()
{
    string f = value;
    string intpart;
    string fpart;

    switch(type)
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

ASTString::ASTString(const char* str, LocationData const& location)
	: AST(location), str((string)str)
{}

ASTString::ASTString(string const& str, LocationData const& location)
	: AST(location), str(str)
{}

ASTString::ASTString(ASTString const& base)
	: AST(base), str(base.str)
{}

ASTString& ASTString::operator=(ASTString const& rhs)
{
	AST::operator=(rhs);

	str = rhs.str;

	return *this;
}

////////////////////////////////////////////////////////////////
// Statements

// ASTStmt

ASTStmt::ASTStmt(LocationData const& location) : AST(location) {}

ASTStmt& ASTStmt::operator=(ASTStmt const& rhs)
{
	AST::operator=(rhs);

	return *this;
}

// ASTBlock

ASTBlock::ASTBlock(LocationData const& location) : ASTStmt(location) {}
    
ASTBlock::ASTBlock(ASTBlock const& base)
	: ASTStmt(base), statements(AST::clone(base.statements))
{}

ASTBlock::~ASTBlock()
{
	deleteElements(statements);
}

ASTBlock& ASTBlock::operator=(ASTBlock const& rhs)
{
	ASTStmt::operator=(rhs);

	deleteElements(statements);

	statements = AST::clone(rhs.statements);

	return *this;
}

// ASTStmtIf

ASTStmtIf::ASTStmtIf(ASTExpr* condition,
					 ASTStmt* thenStatement,
					 LocationData const& location)
	: ASTStmt(location), condition(condition), thenStatement(thenStatement)
{}

ASTStmtIf::ASTStmtIf(ASTStmtIf const& base)
	: ASTStmt(base),
	  condition(AST::clone(condition)),
	  thenStatement(AST::clone(thenStatement))
{}

ASTStmtIf::~ASTStmtIf()
{
	delete condition;
	delete thenStatement;
}

ASTStmtIf& ASTStmtIf::operator=(ASTStmtIf const& rhs)
{
	ASTStmt::operator=(rhs);

	delete condition;
	delete thenStatement;

	condition = AST::clone(rhs.condition);
	thenStatement = AST::clone(rhs.thenStatement);

	return *this;
}

// ASTStmtIfElse
    
ASTStmtIfElse::ASTStmtIfElse(
		ASTExpr* condition, ASTStmt* thenStatement,
		ASTStmt* elseStatement, LocationData const& location)
	: ASTStmtIf(condition, thenStatement, location),
	  elseStatement(elseStatement)
{}

ASTStmtIfElse::ASTStmtIfElse(ASTStmtIfElse const& base)
	: ASTStmtIf(base), elseStatement(AST::clone(base.elseStatement))
{}

ASTStmtIfElse::~ASTStmtIfElse()
{
	delete elseStatement;
}

ASTStmtIfElse& ASTStmtIfElse::operator=(ASTStmtIfElse const& rhs)
{
	ASTStmtIf::operator=(rhs);

	delete elseStatement;

	elseStatement = AST::clone(rhs.elseStatement);

	return *this;
}

// ASTStmtSwitch

ASTStmtSwitch::ASTStmtSwitch(LocationData const& location)
	: ASTStmt(location), key(NULL)
{}

ASTStmtSwitch::ASTStmtSwitch(ASTStmtSwitch const& base)
	: ASTStmt(base), key(AST::clone(base.key)), cases(AST::clone(base.cases))
{}

ASTStmtSwitch::~ASTStmtSwitch()
{
	delete key;
	deleteElements(cases);
}

ASTStmtSwitch& ASTStmtSwitch::operator=(ASTStmtSwitch const& rhs)
{
	ASTStmt::operator=(rhs);

	delete key;
	deleteElements(cases);

	key = AST::clone(rhs.key);
	cases = AST::clone(rhs.cases);

	return *this;
}

// ASTSwitchCases

ASTSwitchCases::ASTSwitchCases(LocationData const& location)
	: AST(location), isDefault(false), block(NULL)
{}

ASTSwitchCases::ASTSwitchCases(ASTSwitchCases const& base)
	: AST(base),
	  cases(AST::clone(base.cases)),
	  isDefault(base.isDefault),
	  block(AST::clone(base.block))
{}

ASTSwitchCases::~ASTSwitchCases()
{
	deleteElements(cases);
	delete block;
}

ASTSwitchCases& ASTSwitchCases::operator=(ASTSwitchCases const& rhs)
{
	AST::operator=(rhs);

	deleteElements(cases);
	delete block;

	cases = AST::clone(rhs.cases);
	isDefault = rhs.isDefault;
	block = AST::clone(rhs.block);

	return *this;
}

// ASTStmtFor

ASTStmtFor::ASTStmtFor(
		ASTStmt* setup, ASTExpr* test, ASTStmt* increment, ASTStmt* body,
		LocationData const& location)
	: ASTStmt(location), setup(setup), test(test), increment(increment),
	  body(body)
{}

ASTStmtFor::ASTStmtFor(ASTStmtFor const& base)
	: ASTStmt(base),
	  setup(AST::clone(base.setup)),
	  test(AST::clone(base.test)),
	  increment(AST::clone(base.increment)),
	  body(AST::clone(base.body))
{}

ASTStmtFor::~ASTStmtFor()
{
	delete setup;
	delete test;
	delete increment;
	delete body;
}

ASTStmtFor& ASTStmtFor::operator=(ASTStmtFor const& rhs)
{
	ASTStmt::operator=(rhs);

	delete setup;
	delete test;
	delete increment;
	delete body;

	setup = AST::clone(rhs.setup);
	test = AST::clone(rhs.test);
	increment = AST::clone(rhs.increment);
	body = AST::clone(rhs.body);

	return *this;
}

// ASTStmtWhile

ASTStmtWhile::ASTStmtWhile(
		ASTExpr* test, ASTStmt* body, LocationData const& location)
	: ASTStmt(location), test(test), body(body)
{}

ASTStmtWhile::ASTStmtWhile(ASTStmtWhile const& base)
	: ASTStmt(base),
	  test(AST::clone(base.test)),
	  body(AST::clone(base.body))
{}

ASTStmtWhile::~ASTStmtWhile()
{
	delete test;
	delete body;
}

ASTStmtWhile& ASTStmtWhile::operator=(ASTStmtWhile const& rhs)
{
	ASTStmt::operator=(rhs);

	delete test;
	delete body;

	test = AST::clone(rhs.test);
	body = AST::clone(rhs.body);

	return *this;
}

// ASTStmtDo

ASTStmtDo::ASTStmtDo(
		ASTExpr* test, ASTStmt* body, LocationData const& location)
	: ASTStmt(location), test(test), body(body)
{}

ASTStmtDo::ASTStmtDo(ASTStmtDo const& base)
	: ASTStmt(base),
	  test(AST::clone(base.test)),
	  body(AST::clone(base.body))
{}

ASTStmtDo::~ASTStmtDo()
{
	delete test;
	delete body;
}

ASTStmtDo& ASTStmtDo::operator=(ASTStmtDo const& rhs)
{
	ASTStmt::operator=(rhs);

	delete test;
	delete body;

	test = AST::clone(rhs.test);
	body = AST::clone(rhs.body);

	return *this;
}

// ASTStmtReturn

ASTStmtReturn::ASTStmtReturn(LocationData const& location)
	: ASTStmt(location)
{}

ASTStmtReturn::ASTStmtReturn(ASTStmtReturn const& base)
	: ASTStmt(base)
{}

ASTStmtReturn& ASTStmtReturn::operator=(ASTStmtReturn const& rhs)
{
	ASTStmt::operator=(rhs);

	return *this;
}

// ASTStmtReturnVal

ASTStmtReturnVal::ASTStmtReturnVal(
		ASTExpr* value, LocationData const& location)
	: ASTStmtReturn(location), value(value)
{}

ASTStmtReturnVal::ASTStmtReturnVal(ASTStmtReturnVal const& base)
	: ASTStmtReturn(base), value(AST::clone(base.value))
{}

ASTStmtReturnVal::~ASTStmtReturnVal()
{
	delete value;
}

ASTStmtReturnVal& ASTStmtReturnVal::operator=(ASTStmtReturnVal const& rhs)
{
	ASTStmtReturn::operator=(rhs);

	delete value;

	value = AST::clone(rhs.value);

	return *this;
}

// ASTStmtBreak

ASTStmtBreak::ASTStmtBreak(LocationData const& location)
	: ASTStmt(location)
{}

ASTStmtBreak::ASTStmtBreak(ASTStmtBreak const& base)
	: ASTStmt(base)
{}

ASTStmtBreak& ASTStmtBreak::operator=(ASTStmtBreak const& rhs)
{
	ASTStmt::operator=(rhs);

	return *this;
}

// ASTStmtContinue

ASTStmtContinue::ASTStmtContinue(LocationData const& location)
	: ASTStmt(location)
{}

ASTStmtContinue::ASTStmtContinue(ASTStmtContinue const& base)
	: ASTStmt(base)
{}

ASTStmtContinue& ASTStmtContinue::operator=(ASTStmtContinue const& rhs)
{
	ASTStmt::operator=(rhs);

	return *this;
}

// ASTStmtEmpty

ASTStmtEmpty::ASTStmtEmpty(LocationData const& location)
	: ASTStmt(location)
{}

ASTStmtEmpty::ASTStmtEmpty(ASTStmtEmpty const& base)
	: ASTStmt(base)
{}

ASTStmtEmpty& ASTStmtEmpty::operator=(ASTStmtEmpty const& rhs)
{
	ASTStmt::operator=(rhs);

	return *this;
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
	  errorId(AST::clone(base.errorId)),
	  statement(AST::clone(base.statement)),
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

	delete errorId;
	delete statement;

	errorId = AST::clone(rhs.errorId);
	statement = AST::clone(rhs.statement);
	errorTriggered = rhs.errorTriggered;

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

ASTDecl::ASTDecl(LocationData const& location)
	: ASTStmt(location)
{}

ASTDecl& ASTDecl::operator=(ASTDecl const& rhs)
{
	ASTStmt::operator=(rhs);
	return *this;
}

// ASTScript

ASTScript::ASTScript(LocationData const& location)
	: ASTDecl(location), type(NULL), name("") {}

ASTScript::ASTScript(ASTScript const& base)
	: ASTDecl(base),
	  type(AST::clone(base.type)),
	  name(base.name),
	  variables(AST::clone(base.variables)),
	  functions(AST::clone(base.functions)),
	  types(AST::clone(base.types))
{}

ASTScript::~ASTScript()
{
	delete type;
	deleteElements(variables);
	deleteElements(functions);
	deleteElements(types);
}

ASTScript& ASTScript::operator=(ASTScript const& rhs)
{
	ASTDecl::operator=(rhs);

	delete type;
	deleteElements(variables);
	deleteElements(functions);
	deleteElements(types);

	type = AST::clone(rhs.type);
	name = rhs.name;
	variables = AST::clone(rhs.variables);
	functions = AST::clone(rhs.functions);
	types = AST::clone(rhs.types);

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

ASTImportDecl::ASTImportDecl(
		string const& filename, LocationData const& location)
	: ASTDecl(location), filename(filename)
{}

ASTImportDecl::ASTImportDecl(ASTImportDecl const& base)
	: ASTDecl(base), filename(base.filename)
{}

ASTImportDecl& ASTImportDecl::operator=(ASTImportDecl const& rhs)
{
	ASTDecl::operator=(rhs);

	filename = rhs.filename;

	return *this;
}

// ASTFuncDecl

ASTFuncDecl::ASTFuncDecl(LocationData const& location)
	: ASTDecl(location), returnType(NULL), block(NULL)
{}

ASTFuncDecl::ASTFuncDecl(ASTFuncDecl const& base)
	: ASTDecl(base),
	  returnType(AST::clone(base.returnType)),
	  parameters(AST::clone(base.parameters)),
	  name(base.name),
	  block(AST::clone(base.block))
{}

ASTFuncDecl::~ASTFuncDecl()
{
	delete returnType;
	deleteElements(parameters);
	delete block;
}

ASTFuncDecl& ASTFuncDecl::operator=(ASTFuncDecl const& rhs)
{
	ASTDecl::operator=(rhs);

	delete returnType;
	deleteElements(parameters);
	delete block;

	returnType = AST::clone(rhs.returnType);
	parameters = AST::clone(rhs.parameters);
	name = rhs.name;
	block = AST::clone(rhs.block);

	return *this;
}

// ASTDataDeclList

ASTDataDeclList::ASTDataDeclList(LocationData const& location)
	: ASTDecl(location), baseType(NULL)
{}

ASTDataDeclList::ASTDataDeclList(ASTDataDeclList const& base)
	: ASTDecl(base),
	  baseType(AST::clone(base.baseType)),
	  mDeclarations(AST::clone(base.mDeclarations))
{}

ASTDataDeclList::~ASTDataDeclList()
{
	deleteElements(mDeclarations);
}

ASTDataDeclList& ASTDataDeclList::operator=(ASTDataDeclList const& rhs)
{
	ASTDecl::operator=(rhs);

	delete baseType;
	deleteElements(mDeclarations);

    baseType = AST::clone(rhs.baseType);
	mDeclarations = AST::clone(rhs.mDeclarations);
	
	return *this;
}

void ASTDataDeclList::addDeclaration(ASTDataDecl* declaration)
{
	// Declarations in a list should not have their own type.
	assert(!declaration->baseType);

	declaration->list = this;
	mDeclarations.push_back(declaration);
}

// ASTDataDecl

ASTDataDecl::ASTDataDecl(LocationData const& location)
	: ASTDecl(location), list(NULL), manager(NULL),
	  baseType(NULL), mInitializer(NULL)
{}

ASTDataDecl::ASTDataDecl(ASTDataDecl const& base)
	: ASTDecl(base),
	  baseType(AST::clone(base.baseType)),
	  name(base.name),
	  mInitializer(AST::clone(base.mInitializer)),
	  extraArrays(AST::clone(base.extraArrays)),
	  // list and manager indicate being this object being "owned" by them, so
	  // it doesn't make sense for a copy to keep those values.
	  list(NULL), manager(NULL)
{}

ASTDataDecl::~ASTDataDecl()
{
	delete baseType;
	deleteElements(extraArrays);
	delete mInitializer;
}

ASTDataDecl& ASTDataDecl::operator=(ASTDataDecl const& rhs)
{
	ASTDecl::operator=(rhs);

	delete baseType;
	deleteElements(extraArrays);
	delete mInitializer;

	list = NULL;
	manager = NULL;
	baseType = AST::clone(rhs.baseType);
	name = rhs.name;
	extraArrays = AST::clone(rhs.extraArrays);
	mInitializer = AST::clone(rhs.mInitializer);

	return *this;
}

ASTExpr* ASTDataDecl::initializer(ASTExpr* initializer)
{
	mInitializer = initializer;

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

	return initializer;
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

ASTDataDeclExtraArray::ASTDataDeclExtraArray(
		ASTDataDeclExtraArray const& base)
	: AST(base), dimensions(AST::clone(base.dimensions))
{}

ASTDataDeclExtraArray::~ASTDataDeclExtraArray()
{
	deleteElements(dimensions);
}

ASTDataDeclExtraArray& ASTDataDeclExtraArray::operator=(
		ASTDataDeclExtraArray const& rhs)
{
	AST::operator=(rhs);

	deleteElements(dimensions);

	dimensions = AST::clone(rhs.dimensions);

	return *this;
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

ASTTypeDef::ASTTypeDef(
		ASTVarType* type, string const& name, LocationData const& location)
	 : ASTDecl(location), type(type), name(name)
{}

ASTTypeDef::ASTTypeDef(ASTTypeDef const& base)
	: ASTDecl(base), type(AST::clone(base.type)), name(base.name)
{}

ASTTypeDef::~ASTTypeDef()
{
	delete type;
}

ASTTypeDef& ASTTypeDef::operator=(ASTTypeDef const& rhs)
{
	ASTDecl::operator=(rhs);

	delete type;

	type = AST::clone(rhs.type);

	return *this;
}

////////////////////////////////////////////////////////////////
// Expressions

// ASTExpr

ASTExpr::ASTExpr(LocationData const& location)
	: ASTStmt(location),
	  hasValue(false),
	  value(0L),
	  varType(NULL),
	  lval(false)
{}

ASTExpr::ASTExpr(ASTExpr const& base)
	: ASTStmt(base),
	  hasValue(base.hasValue),
	  value(base.value),
	  varType(base.varType),
	  lval(false)
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

ASTExprConst::ASTExprConst(ASTExpr* content, LocationData const& location)
	: ASTExpr(location), content(content)
{}

ASTExprConst::ASTExprConst(ASTExprConst const& base)
	: ASTExpr(base), content(AST::clone(base.content))
{}

ASTExprConst& ASTExprConst::operator=(ASTExprConst const& rhs)
{
	ASTExpr::operator=(rhs);
	
	content = AST::clone(rhs.content);
	
	return *this;
}

// ASTExprAssign

ASTExprAssign::ASTExprAssign(ASTExpr* left, ASTExpr* right,
							 LocationData const& location)
	: ASTExpr(location), left(left), right(right) {}

ASTExprAssign::ASTExprAssign(ASTExprAssign const& base)
	: ASTExpr(base), left(AST::clone(base.left)), right(AST::clone(base.right))
{}

ASTExprAssign::~ASTExprAssign()
{
	delete left;
	delete right;
}

ASTExprAssign& ASTExprAssign::operator=(ASTExprAssign const& rhs)
{
	ASTExpr::operator=(rhs);

	delete left;
	delete right;

	left = AST::clone(rhs.left);
	right = AST::clone(rhs.right);

	return *this;
}

// ASTExprIdentifier

ASTExprIdentifier::ASTExprIdentifier(string const& name,
									 LocationData const& location)
	: ASTExpr(location), mIsConstant(false)
{
	if (name != "") components.push_back(name);
}

ASTExprIdentifier::ASTExprIdentifier(ASTExprIdentifier const& base)
	: ASTExpr(base), mIsConstant(base.mIsConstant),
	  components(base.components)
{}

ASTExprIdentifier& ASTExprIdentifier::operator=(ASTExprIdentifier const& base)
{
	ASTExpr::operator=(base);
	
	components = base.components;
	mIsConstant = base.mIsConstant;
	
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

ASTExprArrow::ASTExprArrow(ASTExpr* left, string const& right,
						   LocationData const& location)
	: ASTExpr(location), left(left), right(right), index(NULL)
{}

ASTExprArrow::ASTExprArrow(ASTExprArrow const& base)
	: ASTExpr(base),
	  left(AST::clone(base.left)),
	  right(base.right),
	  index(AST::clone(base.index))
{}

ASTExprArrow& ASTExprArrow::operator=(ASTExprArrow const& rhs)
{
	ASTExpr::operator=(rhs);
	
	delete left;
	delete index;

	left = AST::clone(rhs.left);
	right = rhs.right;
	index = AST::clone(rhs.index);

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

ASTExprIndex::ASTExprIndex(ASTExpr* array, ASTExpr* index,
						   LocationData const& location)
	: ASTExpr(location), array(array), index(index)
{}

ASTExprIndex::ASTExprIndex(ASTExprIndex const& base)
	: ASTExpr(base),
	  array(AST::clone(base.array)),
	  index(AST::clone(base.index))
{}

ASTExprIndex& ASTExprIndex::operator=(ASTExprIndex const& rhs)
{
	ASTExpr::operator=(rhs);

	delete array;
	delete index;
	
	array = AST::clone(rhs.array);
	index = AST::clone(rhs.index);
	
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
	: ASTExpr(base),
	  left(AST::clone(base.left)),
	  parameters(AST::clone(base.parameters))
{}

ASTExprCall::~ASTExprCall()
{
    delete left;
	deleteElements(parameters);
}

ASTExprCall& ASTExprCall::operator=(ASTExprCall const& rhs)
{
	ASTExpr::operator=(rhs);

    delete left;
	deleteElements(parameters);

	left = AST::clone(rhs.left);
	parameters = AST::clone(rhs.parameters);

	return *this;
}

// ASTUnaryExpr

ASTUnaryExpr::ASTUnaryExpr(LocationData const& location)
	: ASTExpr(location)
{}

ASTUnaryExpr::ASTUnaryExpr(ASTUnaryExpr const& base)
	: ASTExpr(base), operand(AST::clone(base.operand))
{}

ASTUnaryExpr& ASTUnaryExpr::operator=(ASTUnaryExpr const& rhs)
{
	ASTExpr::operator=(rhs);
	
	delete operand;

	operand = AST::clone(rhs.operand);

	return *this;
}

// ASTExprNegate

ASTExprNegate::ASTExprNegate(LocationData const& location)
	: ASTUnaryExpr(location)
{}

ASTExprNegate::ASTExprNegate(ASTExprNegate const& base)
	: ASTUnaryExpr(base)
{}

ASTExprNegate& ASTExprNegate::operator=(ASTExprNegate const& rhs)
{
	ASTUnaryExpr::operator=(rhs);

	return *this;
}

// ASTExprNot

ASTExprNot::ASTExprNot(LocationData const& location)
	: ASTUnaryExpr(location)
{}

ASTExprNot::ASTExprNot(ASTExprNot const& base)
	: ASTUnaryExpr(base)
{}

ASTExprNot& ASTExprNot::operator=(ASTExprNot const& rhs)
{
	ASTUnaryExpr::operator=(rhs);

	return *this;
}

// ASTExprBitNot

ASTExprBitNot::ASTExprBitNot(LocationData const& location)
	: ASTUnaryExpr(location)
{}

ASTExprBitNot::ASTExprBitNot(ASTExprBitNot const& base)
	: ASTUnaryExpr(base)
{}

ASTExprBitNot& ASTExprBitNot::operator=(ASTExprBitNot const& rhs)
{
	ASTUnaryExpr::operator=(rhs);

	return *this;
}

// ASTExprIncrement

ASTExprIncrement::ASTExprIncrement(LocationData const& location)
	: ASTUnaryExpr(location)
{}

ASTExprIncrement::ASTExprIncrement(ASTExprIncrement const& base)
	: ASTUnaryExpr(base)
{}

ASTExprIncrement& ASTExprIncrement::operator=(ASTExprIncrement const& rhs)
{
	ASTUnaryExpr::operator=(rhs);

	return *this;
}

// ASTExprPreIncrement

ASTExprPreIncrement::ASTExprPreIncrement(LocationData const& location)
	: ASTUnaryExpr(location)
{}

ASTExprPreIncrement::ASTExprPreIncrement(ASTExprPreIncrement const& base)
	: ASTUnaryExpr(base)
{}

ASTExprPreIncrement& ASTExprPreIncrement::operator=(
		ASTExprPreIncrement const& rhs)
{
	ASTUnaryExpr::operator=(rhs);

	return *this;
}

// ASTExprDecrement

ASTExprDecrement::ASTExprDecrement(LocationData const& location)
	: ASTUnaryExpr(location)
{}

ASTExprDecrement::ASTExprDecrement(ASTExprDecrement const& base)
	: ASTUnaryExpr(base)
{}

ASTExprDecrement& ASTExprDecrement::operator=(ASTExprDecrement const& rhs)
{
	ASTUnaryExpr::operator=(rhs);

	return *this;
}

// ASTExprPreDecrement

ASTExprPreDecrement::ASTExprPreDecrement(LocationData const& location)
	: ASTUnaryExpr(location)
{}

ASTExprPreDecrement::ASTExprPreDecrement(ASTExprPreDecrement const& base)
	: ASTUnaryExpr(base)
{}

ASTExprPreDecrement& ASTExprPreDecrement::operator=(
		ASTExprPreDecrement const& rhs)
{
	ASTUnaryExpr::operator=(rhs);

	return *this;
}

// ASTBinaryExpr

ASTBinaryExpr::ASTBinaryExpr(ASTExpr* left, ASTExpr* right,
							 LocationData const& location)
	: ASTExpr(location), left(left), right(right)
{}

ASTBinaryExpr::ASTBinaryExpr(ASTBinaryExpr const& base)
	: ASTExpr(base), left(AST::clone(base.left)),
	  right(AST::clone(base.right))
{}

ASTBinaryExpr::~ASTBinaryExpr()
{
	delete left;
	delete right;
}

ASTBinaryExpr& ASTBinaryExpr::operator=(ASTBinaryExpr const& rhs)
{
	ASTExpr::operator=(rhs);
	
	delete left;
	delete right;

	left = AST::clone(rhs.left);
	right = AST::clone(rhs.right);
	
	return *this;
}

bool ASTBinaryExpr::isConstant() const
{
	if (left && !left->isConstant()) return false;
	if (right && !right->isConstant()) return false;
	return true;
}

// ASTLogExpr

ASTLogExpr::ASTLogExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

ASTLogExpr::ASTLogExpr(ASTLogExpr const& base) : ASTBinaryExpr(base) {}

ASTLogExpr& ASTLogExpr::operator=(ASTLogExpr const& rhs)
{
	ASTBinaryExpr::operator=(rhs);

	return *this;
}

// ASTExprAnd

ASTExprAnd::ASTExprAnd(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTLogExpr(left, right, location)
{}

ASTExprAnd::ASTExprAnd(ASTExprAnd const& base) : ASTLogExpr(base) {}

ASTExprAnd& ASTExprAnd::operator=(ASTExprAnd const& rhs)
{
	ASTLogExpr::operator=(rhs);

	return *this;
}

// ASTExprOr

ASTExprOr::ASTExprOr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTLogExpr(left, right, location)
{}

ASTExprOr::ASTExprOr(ASTExprOr const& base) : ASTLogExpr(base) {}

ASTExprOr& ASTExprOr::operator=(ASTExprOr const& rhs)
{
	ASTLogExpr::operator=(rhs);

	return *this;
}

// ASTRelExpr

ASTRelExpr::ASTRelExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

ASTRelExpr::ASTRelExpr(ASTRelExpr const& base) : ASTBinaryExpr(base) {}

ASTRelExpr& ASTRelExpr::operator=(ASTRelExpr const& rhs)
{
	ASTBinaryExpr::operator=(rhs);

	return *this;
}

// ASTExprGT

ASTExprGT::ASTExprGT(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

ASTExprGT::ASTExprGT(ASTExprGT const& base) : ASTRelExpr(base) {}

ASTExprGT& ASTExprGT::operator=(ASTExprGT const& rhs)
{
	ASTRelExpr::operator=(rhs);

	return *this;
}

// ASTExprGE

ASTExprGE::ASTExprGE(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

ASTExprGE::ASTExprGE(ASTExprGE const& base) : ASTRelExpr(base) {}

ASTExprGE& ASTExprGE::operator=(ASTExprGE const& rhs)
{
	ASTRelExpr::operator=(rhs);

	return *this;
}

// ASTExprLT

ASTExprLT::ASTExprLT(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

ASTExprLT::ASTExprLT(ASTExprLT const& base) : ASTRelExpr(base) {}

ASTExprLT& ASTExprLT::operator=(ASTExprLT const& rhs)
{
	ASTRelExpr::operator=(rhs);

	return *this;
}

// ASTExprLE

ASTExprLE::ASTExprLE(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

ASTExprLE::ASTExprLE(ASTExprLE const& base) : ASTRelExpr(base) {}

ASTExprLE& ASTExprLE::operator=(ASTExprLE const& rhs)
{
	ASTRelExpr::operator=(rhs);

	return *this;
}

// ASTExprEQ

ASTExprEQ::ASTExprEQ(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

ASTExprEQ::ASTExprEQ(ASTExprEQ const& base) : ASTRelExpr(base) {}

ASTExprEQ& ASTExprEQ::operator=(ASTExprEQ const& rhs)
{
	ASTRelExpr::operator=(rhs);

	return *this;
}

// ASTExprNE

ASTExprNE::ASTExprNE(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

ASTExprNE::ASTExprNE(ASTExprNE const& base) : ASTRelExpr(base) {}

ASTExprNE& ASTExprNE::operator=(ASTExprNE const& rhs)
{
	ASTRelExpr::operator=(rhs);

	return *this;
}

// ASTAddExpr

ASTAddExpr::ASTAddExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

ASTAddExpr::ASTAddExpr(ASTAddExpr const& base) : ASTBinaryExpr(base) {}

ASTAddExpr& ASTAddExpr::operator=(ASTAddExpr const& rhs)
{
	ASTBinaryExpr::operator=(rhs);

	return *this;
}

// ASTExprPlus

ASTExprPlus::ASTExprPlus(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTAddExpr(left, right, location)
{}

ASTExprPlus::ASTExprPlus(ASTExprPlus const& base) : ASTAddExpr(base) {}

ASTExprPlus& ASTExprPlus::operator=(ASTExprPlus const& rhs)
{
	ASTAddExpr::operator=(rhs);

	return *this;
}

// ASTExprMinus

ASTExprMinus::ASTExprMinus(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTAddExpr(left, right, location)
{}

ASTExprMinus::ASTExprMinus(ASTExprMinus const& base) : ASTAddExpr(base) {}

ASTExprMinus& ASTExprMinus::operator=(ASTExprMinus const& rhs)
{
	ASTAddExpr::operator=(rhs);

	return *this;
}

// ASTMultExpr

ASTMultExpr::ASTMultExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

ASTMultExpr::ASTMultExpr(ASTMultExpr const& base) : ASTBinaryExpr(base) {}

ASTMultExpr& ASTMultExpr::operator=(ASTMultExpr const& rhs)
{
	ASTBinaryExpr::operator=(rhs);

	return *this;
}

// ASTExprTimes

ASTExprTimes::ASTExprTimes(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTMultExpr(left, right, location)
{}

ASTExprTimes::ASTExprTimes(ASTExprTimes const& base) : ASTMultExpr(base) {}

ASTExprTimes& ASTExprTimes::operator=(ASTExprTimes const& rhs)
{
	ASTMultExpr::operator=(rhs);

	return *this;
}

// ASTExprDivide

ASTExprDivide::ASTExprDivide(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTMultExpr(left, right, location)
{}

ASTExprDivide::ASTExprDivide(ASTExprDivide const& base) : ASTMultExpr(base) {}

ASTExprDivide& ASTExprDivide::operator=(ASTExprDivide const& rhs)
{
	ASTMultExpr::operator=(rhs);

	return *this;
}

// ASTExprModulo
    
ASTExprModulo::ASTExprModulo(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTMultExpr(left, right, location)
{}

ASTExprModulo::ASTExprModulo(ASTExprModulo const& base) : ASTMultExpr(base) {}

ASTExprModulo& ASTExprModulo::operator=(ASTExprModulo const& rhs)
{
	ASTMultExpr::operator=(rhs);

	return *this;
}

// ASTBitExpr

ASTBitExpr::ASTBitExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

ASTBitExpr::ASTBitExpr(ASTBitExpr const& base) : ASTBinaryExpr(base) {}

ASTBitExpr& ASTBitExpr::operator=(ASTBitExpr const& rhs)
{
	ASTBinaryExpr::operator=(rhs);

	return *this;
}

// ASTExprBitAnd

ASTExprBitAnd::ASTExprBitAnd(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBitExpr(left, right, location)
{}

ASTExprBitAnd::ASTExprBitAnd(ASTExprBitAnd const& base) : ASTBitExpr(base) {}

ASTExprBitAnd& ASTExprBitAnd::operator=(ASTExprBitAnd const& rhs)
{
	ASTBitExpr::operator=(rhs);

	return *this;
}

// ASTExprBitOr

ASTExprBitOr::ASTExprBitOr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBitExpr(left, right, location)
{}

ASTExprBitOr::ASTExprBitOr(ASTExprBitOr const& base) : ASTBitExpr(base) {}

ASTExprBitOr& ASTExprBitOr::operator=(ASTExprBitOr const& rhs)
{
	ASTBitExpr::operator=(rhs);

	return *this;
}

// ASTExprBitXor

ASTExprBitXor::ASTExprBitXor(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBitExpr(left, right, location)
{}

ASTExprBitXor::ASTExprBitXor(ASTExprBitXor const& base) : ASTBitExpr(base) {}

ASTExprBitXor& ASTExprBitXor::operator=(ASTExprBitXor const& rhs)
{
	ASTBitExpr::operator=(rhs);

	return *this;
}

// ASTShiftExpr

ASTShiftExpr::ASTShiftExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

ASTShiftExpr::ASTShiftExpr(ASTShiftExpr const& base) : ASTBinaryExpr(base) {}

ASTShiftExpr& ASTShiftExpr::operator=(ASTShiftExpr const& rhs)
{
	ASTBinaryExpr::operator=(rhs);

	return *this;
}

// ASTExprLShift

ASTExprLShift::ASTExprLShift(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTShiftExpr(left, right, location)
{}

ASTExprLShift::ASTExprLShift(ASTExprLShift const& base) : ASTShiftExpr(base) {}

ASTExprLShift& ASTExprLShift::operator=(ASTExprLShift const& rhs)
{
	ASTShiftExpr::operator=(rhs);

	return *this;
}

// ASTExprRShift

ASTExprRShift::ASTExprRShift(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTShiftExpr(left, right, location)
{}

ASTExprRShift::ASTExprRShift(ASTExprRShift const& base) : ASTShiftExpr(base) {}

ASTExprRShift& ASTExprRShift::operator=(ASTExprRShift const& rhs)
{
	ASTShiftExpr::operator=(rhs);

	return *this;
}

////////////////////////////////////////////////////////////////
// Literals

// ASTLiteral

ASTLiteral::ASTLiteral(LocationData const& location)
	: ASTExpr(location), manager(NULL)
{}

ASTLiteral& ASTLiteral::operator=(ASTLiteral const& rhs)
{
	ASTExpr::operator=(rhs);
	manager = rhs.manager;
	return *this;
}

// ASTNumberLiteral

ASTNumberLiteral::ASTNumberLiteral(
		ASTFloat* value, LocationData const& location)
	: ASTLiteral(location), value(value)
{}

ASTNumberLiteral::ASTNumberLiteral(ASTNumberLiteral const& base)
	: ASTLiteral(base), value(AST::clone(base.value))
{}

ASTNumberLiteral& ASTNumberLiteral::operator=(ASTNumberLiteral const& rhs)
{
	ASTLiteral::operator=(rhs);

	delete value;

	value = AST::clone(rhs.value);

	return *this;
}

// ASTBoolLiteral

ASTBoolLiteral::ASTBoolLiteral(bool value, LocationData const& location)
	: ASTLiteral(location), value(value)
{}

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
	: ASTLiteral(location), value(str), declaration(NULL)
{}

ASTStringLiteral::ASTStringLiteral(
		string const& str, LocationData const& location)
	: ASTLiteral(location), value(str), declaration(NULL)
{}

ASTStringLiteral::ASTStringLiteral(ASTString const& raw)
	: ASTLiteral(raw.location),
	  value(raw.getValue().substr(1, raw.getValue().size() - 2)),
	  declaration(NULL)
{}

ASTStringLiteral::ASTStringLiteral(ASTStringLiteral const& base)
	: ASTLiteral(base), value(base.value),
	  // declaration field is managed by the declaration itself, so it stays
	  // NULL regardless.
	  declaration(NULL)
{}

ASTStringLiteral& ASTStringLiteral::operator=(ASTStringLiteral const& rhs)
{
	ASTLiteral::operator=(rhs);
	value = rhs.value;
	declaration = NULL;
	return *this;
}

// ASTArrayLiteral

ASTArrayLiteral::ASTArrayLiteral(LocationData const& location)
	: ASTLiteral(location), type(NULL), size(NULL), declaration(NULL)
{}

ASTArrayLiteral::ASTArrayLiteral(ASTArrayLiteral const& base)
	: ASTLiteral(base),
	  type(AST::clone(base.type)),
	  size(AST::clone(base.size)),
	  elements(AST::clone(base.elements)),
	  // declaration field is managed by the declaration itself, so it stays
	  // NULL regardless.
	  declaration(NULL)
{}

ASTArrayLiteral::~ASTArrayLiteral()
{
	delete type;
	delete size;
	deleteElements(elements);
}

ASTArrayLiteral& ASTArrayLiteral::operator=(ASTArrayLiteral const& rhs)
{
	ASTLiteral::operator=(rhs);

	delete type;
	delete size;
	deleteElements(elements);

	type = AST::clone(rhs.type);
	size = AST::clone(rhs.size);
	elements = AST::clone(rhs.elements);
	declaration = NULL;

	return *this;
}

////////////////////////////////////////////////////////////////
// Types

// ASTScriptType

ASTScriptType::ASTScriptType(ScriptType type, LocationData const& location)
	: AST(location), type(type)
{}

ASTScriptType::ASTScriptType(ASTScriptType const& base)
	: AST(base), type(base.type)
{}

ASTScriptType& ASTScriptType::operator=(ASTScriptType const& rhs)
{
	AST::operator=(rhs);

	type = rhs.type;

	return *this;
}

// ASTVarType

ASTVarType::ASTVarType(ZVarType* type, LocationData const& location)
	: AST(location), type(type)
{}

ASTVarType::ASTVarType(ZVarType const& type, LocationData const& location)
	: AST(location), type(type.clone())
{}

ASTVarType::ASTVarType(ASTVarType const& base)
	: AST(base), type(AST::clone(base.type))
{}

ASTVarType& ASTVarType::operator=(ASTVarType const& rhs)
{
	AST::operator=(rhs);

	type = AST::clone(rhs.type);

	return *this;
}

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

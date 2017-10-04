#include "../precompiled.h" //always first
#include "ASTVisitors.h"
#include "CompilerUtils.h"
#include "DataStructs.h"
#include "Scope.h"

#include <assert.h>
#include <sstream>

using namespace ZScript;

////////////////////////////////////////////////////////////////

// AST

AST::AST(LocationData const& location)
	: location(location), disabled(false)
{}

AST::AST(AST const& rhs) :
	location(rhs.location), disabled(rhs.disabled)
{}

AST::~AST()
{
	deleteElements(compileErrorCatches);
}

AST& AST::operator=(AST const& rhs)
{
	deleteElements(compileErrorCatches);
	
	location = rhs.location;
	compileErrorCatches = AST::clone(compileErrorCatches);
	disabled = rhs.disabled;
	
	return *this;
}

string getLineString(AST const& node)
{
	return LimitString(256)
		<< "[" << node.location.fname.substr(0, 8)
		<< "..:" << node.location.first_line << "] "
		<< node.asString();
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

void ASTProgram::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseProgram(*this, param);
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

void ASTFloat::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseFloat(*this, param);
}

string ASTFloat::asString() const
{
	return value;
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

void ASTString::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseString(*this, param);
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

void ASTBlock::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseBlock(*this, param);
}

string ASTBlock::asString() const
{
	LimitString s;
	s << "{ ";
	for (vector<ASTStmt*>::const_iterator it = statements.begin();
	     it != statements.end(); ++it)
		s << (*it)->asString() << "; ";
	s << "}";
	return s;
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

void ASTStmtIf::execute(ASTVisitor& visitor, void* param)
{
	return visitor.caseStmtIf(*this, param);
}

string ASTStmtIf::asString() const
{
	return LimitString()
		<< "if (" << condition->asString() << ") "
		<< thenStatement->asString();
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

void ASTStmtIfElse::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtIfElse(*this, param);
}

std::string ASTStmtIfElse::asString() const
{
	return LimitString()
		<< "if (" << condition->asString() << ") "
		<< thenStatement->asString()
		<< " else " << elseStatement->asString();
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

void ASTStmtSwitch::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtSwitch(*this, param);
}

std::string ASTStmtSwitch::asString() const
{
	LimitString s;
	s << "switch (" << key->asString() << ") {";

	for (vector<ASTSwitchCases*>::const_iterator it = cases.begin();
	     it != cases.end(); ++it)
		s << (*it)->asString();

	return s << "}";
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

void ASTSwitchCases::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseSwitchCases(*this, param);
}

std::string ASTSwitchCases::asString() const
{
	LimitString s;

	for (vector<ASTExprConst*>::const_iterator it = cases.begin();
	     it != cases.end(); ++it)
		s << "case " << (*it)->asString() << ": ";

	if (isDefault) s << "default: ";
	s << block->asString();

	return s;
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

void ASTStmtFor::execute(ASTVisitor& visitor, void* param)
{
	return visitor.caseStmtFor(*this, param);
}

std::string ASTStmtFor::asString() const
{
	return LimitString()
		<< "for ("
		<< setup->asString() << "; "
		<< test->asString() << "; "
		<< increment->asString() << ") "
		<< body->asString();
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

void ASTStmtWhile::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtWhile(*this, param);
}

std::string ASTStmtWhile::asString() const
{
	return LimitString()
		<< "while (" << test->asString() << ") "
		<< body->asString();
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

void ASTStmtDo::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtDo(*this, param);
}

std::string ASTStmtDo::asString() const
{
	return LimitString()
		<< "do " << body->asString()
		<< " while (" << test->asString() << ");";
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

void ASTStmtReturn::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtReturn(*this, param);
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

void ASTStmtReturnVal::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtReturnVal(*this, param);
}

string ASTStmtReturnVal::asString() const
{
	return LimitString() << "return " << value->asString() << ";";
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

void ASTStmtBreak::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtBreak(*this, param);
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

void ASTStmtContinue::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtContinue(*this, param);
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

void ASTStmtEmpty::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtEmpty(*this, param);
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

void ASTScript::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseScript(*this, param);
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

void ASTImportDecl::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseImportDecl(*this,param);
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

void ASTFuncDecl::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseFuncDecl(*this, param);
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

void ASTDataDeclList::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseDataDeclList(*this, param);
}

string ASTDataDeclList::asString() const
{
	LimitString s;
	s << (*baseType)->getName() << " ";

	bool first = true;
	for (vector<ASTDataDecl*>::const_iterator it = mDeclarations.begin();
	     it != mDeclarations.end(); ++it)
	{
		if (first) first = false; else s << ", ";
		s << (*it)->asString();
	}

	s << ";";
	return s;
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

void ASTDataDecl::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseDataDecl(*this, param);
}

string ASTDataDecl::asString() const
{
	LimitString s;
	if (baseType) s << (*baseType)->getName() << " ";
	s << name;

	for (vector<ASTDataDeclExtraArray*>::const_iterator it =
		     extraArrays.begin();
	     it != extraArrays.end(); ++it)
		s << "[]";

	if (mInitializer) s << " = " << mInitializer->asString();
	if (!list) s << ";";
	return s;
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

DataType ASTDataDecl::resolveType(ZScript::Scope* scope) const
{
	// First resolve the base type.
	ASTVarType* baseTypeNode = list ? list->baseType : baseType;
	baseTypeNode->resolve(*scope);
	DataType type = **baseTypeNode;

	// If we have any arrays, tack them onto the base type.
	for (vector<ASTDataDeclExtraArray*>::const_iterator it =
		     extraArrays.begin();
		 it != extraArrays.end(); ++it)
		type = arrayType(type);

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

void ASTDataDeclExtraArray::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseDataDeclExtraArray(*this, param);
}

optional<int> ASTDataDeclExtraArray::getCompileTimeSize(
		CompileErrorHandler* errorHandler)
		const
{
	if (dimensions.size() == 0) return nullopt;
	int size = 1;
	for (vector<ASTExpr*>::const_iterator it = dimensions.begin();
		 it != dimensions.end(); ++it)
	{
		ASTExpr& expr = **it;
		if (optional<long> value = expr.getCompileTimeValue(errorHandler))
			size *= *value / 10000L;
		else
			return nullopt;
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

void ASTTypeDef::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseTypeDef(*this, param);
}

////////////////////////////////////////////////////////////////
// Expressions

// ASTExpr

ASTExpr::ASTExpr(LocationData const& location)
	: ASTStmt(location)
{}

ASTExpr::ASTExpr(ASTExpr const& base)
	: ASTStmt(base), varType(base.varType)
{}

ASTExpr& ASTExpr::operator=(ASTExpr const& rhs)
{
	ASTStmt::operator=(rhs);
	
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

void ASTExprConst::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprConst(*this, param);
}

optional<long> ASTExprConst::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	return content ? content->getCompileTimeValue(errorHandler) : nullopt;
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

void ASTExprAssign::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprAssign(*this, param);
}

string ASTExprAssign::asString() const
{
	return LimitString() << left->asString() << " = " << right->asString();
}

optional<long> ASTExprAssign::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	return right ? right->getCompileTimeValue(errorHandler) : nullopt;
}

// ASTExprIdentifier

ASTExprIdentifier::ASTExprIdentifier(string const& name,
									 LocationData const& location)
	: ASTExpr(location), binding(NULL), mIsConstant(false)
{
	if (name != "") components.push_back(name);
}

ASTExprIdentifier::ASTExprIdentifier(ASTExprIdentifier const& base)
	: ASTExpr(base), binding(NULL), mIsConstant(base.mIsConstant),
	  components(base.components)
{}

ASTExprIdentifier& ASTExprIdentifier::operator=(ASTExprIdentifier const& base)
{
	ASTExpr::operator=(base);
	
	components = base.components;
	mIsConstant = base.mIsConstant;
	binding = NULL;
	
	return *this;
}

void ASTExprIdentifier::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprIdentifier(*this, param);
}

string ASTExprIdentifier::asString() const
{
	LimitString s;
	s << components.front();
	for (vector<string>::const_iterator it = components.begin() + 1;
	     it != components.end(); ++it)
		s << "." << *it;
	return s;
}

optional<long> ASTExprIdentifier::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	return binding ? binding->getCompileTimeValue() : nullopt;
}

optional<DataType> ASTExprIdentifier::getReadType(TypeStore&) const
{
	if (binding) return binding->type;
	return nullopt;
}

optional<DataType> ASTExprIdentifier::getWriteType(TypeStore&) const
{
	if (binding) return binding->type;
	return nullopt;
}

// ASTExprArrow

ASTExprArrow::ASTExprArrow(ASTExpr* left, string const& right,
						   LocationData const& location)
	: ASTExpr(location), left(left), right(right), index(NULL),
	  readFunction(NULL), writeFunction(NULL), leftClass(NULL)
{}

ASTExprArrow::ASTExprArrow(ASTExprArrow const& base)
	: ASTExpr(base),
	  left(AST::clone(base.left)),
	  right(base.right),
	  index(AST::clone(base.index)),
	  readFunction(NULL), writeFunction(NULL), leftClass(NULL)
{}

ASTExprArrow::~ASTExprArrow()
{
	delete left;
}

ASTExprArrow& ASTExprArrow::operator=(ASTExprArrow const& rhs)
{
	ASTExpr::operator=(rhs);
	
	delete left;
	delete index;

	left = AST::clone(rhs.left);
	right = rhs.right;
	index = AST::clone(rhs.index);
	readFunction = NULL;
	writeFunction = NULL;
	leftClass = NULL;

	return *this;
}

void ASTExprArrow::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprArrow(*this, param);
}

string ASTExprArrow::asString() const
{
	return LimitString() << left->asString() << "->" << right;
}

optional<DataType> ASTExprArrow::getReadType(TypeStore&) const
{
	if (readFunction) return readFunction->returnType;
	return nullopt;
}

optional<DataType> ASTExprArrow::getWriteType(TypeStore&) const
{
	if (writeFunction) return writeFunction->paramTypes.back();
	return nullopt;
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

void ASTExprIndex::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprIndex(*this, param);
}

string ASTExprIndex::asString() const
{
	return LimitString()
		<< array->asString()
		<< "[" << index->asString() << "]";
}

bool ASTExprIndex::isConstant() const
{
	if (array == NULL || index == NULL) return false;
	return array->isConstant() && index->isConstant();
}

optional<DataType> ASTExprIndex::getReadType(TypeStore& ts) const
{
	if (array->isTypeArrow()) return array->getReadType(ts);
	return ASTExpr::getReadType(ts);
}

optional<DataType> ASTExprIndex::getWriteType(TypeStore& ts) const
{
	if (array->isTypeArrow()) return array->getWriteType(ts);
	return ASTExpr::getWriteType(ts);
}
	
// ASTExprCall

ASTExprCall::ASTExprCall(LocationData const& location)
	: ASTExpr(location), left(NULL), binding(NULL)
{}

ASTExprCall::ASTExprCall(ASTExprCall const& base)
	: ASTExpr(base),
	  left(AST::clone(base.left)),
	  parameters(AST::clone(base.parameters)),
	  binding(NULL)
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
	binding = NULL;

	return *this;
}

void ASTExprCall::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprCall(*this, param);
}

string ASTExprCall::asString() const
{
	LimitString s;
	s << left->asString();

	vector<ASTExpr*>::const_iterator it = parameters.begin();
	if (it != parameters.end())
	{
		s << "(" << (*it)->asString();
		for (++it; it != parameters.end(); ++it)
			s << ", " << (*it)->asString();
		s << ")";
	}

	return s;
}

optional<DataType> ASTExprCall::getReadType(TypeStore&) const
{
	if (binding) return binding->returnType;
	return nullopt;
}

optional<DataType> ASTExprCall::getWriteType(TypeStore&) const
{
	return nullopt;
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

void ASTExprNegate::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprNegate(*this, param);
}

string ASTExprNegate::asString() const
{
	return LimitString() << "-" << operand->asString();
}

optional<long> ASTExprNegate::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!operand) return nullopt;
	if (optional<long> value = operand->getCompileTimeValue())
		return -*value;
	return nullopt;
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

void ASTExprNot::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprNot(*this, param);
}

string ASTExprNot::asString() const
{
	return LimitString() << "!" << operand->asString();
}

optional<long> ASTExprNot::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!operand) return nullopt;
	if (optional<long> value = operand->getCompileTimeValue())
		return *value ? 0L : 10000L;
	return nullopt;
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

void ASTExprBitNot::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprBitNot(*this, param);
}

string ASTExprBitNot::asString() const
{
	return LimitString() << "~" << operand->asString();
}

optional<long> ASTExprBitNot::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!operand) return nullopt;
	if (optional<long> value = operand->getCompileTimeValue())
		return ~(*value / 10000L) * 10000L;
	return nullopt;
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

void ASTExprIncrement::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprIncrement(*this, param);
}

string ASTExprIncrement::asString() const
{
	return LimitString() << operand->asString() << "++";
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

void ASTExprPreIncrement::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprPreIncrement(*this, param);
}

string ASTExprPreIncrement::asString() const
{
	return LimitString() << "++" << operand->asString();
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

void ASTExprDecrement::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprDecrement(*this, param);
}

string ASTExprDecrement::asString() const
{
	return LimitString() << operand->asString() << "--";
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

void ASTExprPreDecrement::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprPreDecrement(*this, param);
}

string ASTExprPreDecrement::asString() const
{
	return LimitString() << "--" << operand->asString();
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

void ASTExprAnd::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprAnd(*this, param);
}

string ASTExprAnd::asString() const
{
	return LimitString()
		<< left->asString() << " && " << right->asString();
}

optional<long> ASTExprAnd::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;
	return (*leftValue && *rightValue) ? 10000L : 0L;
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

void ASTExprOr::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprOr(*this, param);
}

string ASTExprOr::asString() const
{
	return LimitString()
		<< left->asString() << " || " << right->asString();
}

optional<long> ASTExprOr::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;
	return (*leftValue || *rightValue) ? 10000L : 0L;
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

void ASTExprGT::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprGT(*this, param);
}

string ASTExprGT::asString() const
{
	return LimitString()
		<< left->asString() << " > " << right->asString();
}

optional<long> ASTExprGT::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;
	return (*leftValue > *rightValue) ? 10000L : 0L;
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

void ASTExprGE::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprGE(*this, param);
}

string ASTExprGE::asString() const
{
	return LimitString()
		<< left->asString() << " >= " << right->asString();
}

optional<long> ASTExprGE::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;
	return (*leftValue >= *rightValue) ? 10000L : 0L;
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

void ASTExprLT::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprLT(*this, param);
}

string ASTExprLT::asString() const
{
	return LimitString()
		<< left->asString() << " < " << right->asString();
}

optional<long> ASTExprLT::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;
	return (*leftValue < *rightValue) ? 10000L : 0L;
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

void ASTExprLE::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprLE(*this, param);
}

string ASTExprLE::asString() const
{
	return LimitString()
		<< left->asString() << " <= " << right->asString();
}

optional<long> ASTExprLE::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;
	return (*leftValue <= *rightValue) ? 10000L : 0L;
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

void ASTExprEQ::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprEQ(*this, param);
}

string ASTExprEQ::asString() const
{
	return LimitString()
		<< left->asString() << " == " << right->asString();
}

optional<long> ASTExprEQ::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;
	return (*leftValue == *rightValue) ? 10000L : 0L;
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

void ASTExprNE::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprNE(*this, param);
}

string ASTExprNE::asString() const
{
	return LimitString()
		<< left->asString() << " != " << right->asString();
}

optional<long> ASTExprNE::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;
	return (*leftValue != *rightValue) ? 10000L : 0L;
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

void ASTExprPlus::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprPlus(*this, param);
}

string ASTExprPlus::asString() const
{
	return LimitString()
		<< left->asString() << " + " << right->asString();
}

optional<long> ASTExprPlus::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;
	return *leftValue + *rightValue;
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

void ASTExprMinus::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprMinus(*this, param);
}

string ASTExprMinus::asString() const
{
	return LimitString()
		<< left->asString() << " - " << right->asString();
}

optional<long> ASTExprMinus::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;
	return *leftValue - *rightValue;
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

void ASTExprTimes::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprTimes(*this, param);
}

string ASTExprTimes::asString() const
{
	return LimitString()
		<< left->asString() << " * " << right->asString();
}

optional<long> ASTExprTimes::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;

	return long(*leftValue * (*rightValue / 10000.0));
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

void ASTExprDivide::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprDivide(*this, param);
}

string ASTExprDivide::asString() const
{
	return LimitString()
		<< left->asString() << " / " << right->asString();
}

optional<long> ASTExprDivide::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;

	if (*rightValue == 0)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::DivByZero, this);
		return nullopt;
	}
	return *leftValue / *rightValue * 10000L;
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

void ASTExprModulo::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprModulo(*this, param);
}

string ASTExprModulo::asString() const
{
	return LimitString()
		<< left->asString() << " % " << right->asString();
}

optional<long> ASTExprModulo::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;

	if (*rightValue == 0)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::DivByZero, this);
		return nullopt;
	}
	return *leftValue % *rightValue;
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

void ASTExprBitAnd::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprBitAnd(*this, param);
}

string ASTExprBitAnd::asString() const
{
	return LimitString()
		<< left->asString() << " & " << right->asString();
}

optional<long> ASTExprBitAnd::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;

	return ((*leftValue / 10000L) & (*rightValue / 10000L)) * 10000L;
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

void ASTExprBitOr::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprBitOr(*this, param);
}

string ASTExprBitOr::asString() const
{
	return LimitString()
		<< left->asString() << " | " << right->asString();
}

optional<long> ASTExprBitOr::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;

	return ((*leftValue / 10000L) | (*rightValue / 10000L)) * 10000L;
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

void ASTExprBitXor::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprBitXor(*this, param);
}

string ASTExprBitXor::asString() const
{
	return LimitString()
		<< left->asString() << " ^ " << right->asString();
}

optional<long> ASTExprBitXor::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;

	return ((*leftValue / 10000L) ^ (*rightValue / 10000L)) * 10000L;
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

void ASTExprLShift::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprLShift(*this, param);
}

string ASTExprLShift::asString() const
{
	return LimitString()
		<< left->asString() << " << " << right->asString();
}

optional<long> ASTExprLShift::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;

	if (*rightValue % 10000L)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::ShiftNotInt, this);
		rightValue = (*rightValue / 10000L) * 10000L;
	}
	
	return ((*leftValue / 10000L) << (*rightValue / 10000L)) * 10000L;
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

void ASTExprRShift::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprRShift(*this, param);
}

string ASTExprRShift::asString() const
{
	return LimitString()
		<< left->asString() << " >> " << right->asString();
}

optional<long> ASTExprRShift::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler);
	if (!rightValue) return nullopt;

	if (*rightValue % 10000L)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::ShiftNotInt, this);
		rightValue = (*rightValue / 10000L) * 10000L;
	}
	
	return ((*leftValue / 10000L) >> (*rightValue / 10000L)) * 10000L;
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

void ASTNumberLiteral::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseNumberLiteral(*this, param);
}

string ASTNumberLiteral::asString() const
{
	return value->asString();
}

optional<long> ASTNumberLiteral::getCompileTimeValue(
		CompileErrorHandler* errorHandler)
		const
{
	if (!value) return nullopt;
    pair<long, bool> val = ScriptParser::parseLong(value->parseValue());

    if (!val.second && errorHandler)
	    errorHandler->handleError(
				CompileError::ConstTrunc, this, value->value);

	return val.first;
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

void ASTBoolLiteral::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseBoolLiteral(*this, param);
}

string ASTBoolLiteral::asString() const
{
	return value ? "true" : "false";
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

void ASTStringLiteral::execute (ASTVisitor& visitor, void* param)
{
	visitor.caseStringLiteral(*this, param);
}

string ASTStringLiteral::asString() const
{
	return (LimitString() << "\"" << value).str() + "\"";
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
	readType = DataType();

	return *this;
}

void ASTArrayLiteral::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseArrayLiteral(*this, param);
}

string ASTArrayLiteral::asString() const
{
	LimitString s;
	if (type)
	{
		s << "(" << (*type)->getName();
		if (size) s << "[" << size->asString() << "]";
		s << ")";
	}

	s << "{";
	vector<ASTExpr*>::const_iterator it = elements.begin();
	if (it != elements.end())
	{
		s << (*it)->asString();
		for (++it; it != elements.end(); ++it)
			s << ", " << (*it)->asString();
	}
	s << "}";
	
	return s;
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

void ASTScriptType::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseScriptType(*this, param);
}

// ASTVarType

ASTVarType::ASTVarType(DataType const& type, LocationData const& location)
	: AST(location), type(type)
{}

ASTVarType::ASTVarType(ASTVarType const& rhs)
	: AST(rhs), type(rhs.type)
{}

ASTVarType& ASTVarType::operator=(ASTVarType const& rhs)
{
	AST::operator=(rhs);
	type = rhs.type;
	return *this;
}

void ASTVarType::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseVarType(*this, param);
}

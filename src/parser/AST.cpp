#include "../precompiled.h" // Always first.

#include "ASTVisitors.h"
#include "CompilerUtils.h"
#include "DataStructs.h"
#include "Scope.h"

#include <assert.h>
#include <sstream>

using std::pair;
using std::string;
using std::ostringstream;
using std::vector;
using namespace ZScript;

////////////////////////////////////////////////////////////////
// LocationData

string LocationData::asString() const
{
	ostringstream out;
	if (fname != "tmp" && fname != "")
		out << fname << " ";
	if (first_line == last_line)
	{
		out << "Line " << first_line << " @ ";
		if (first_column == last_column)
			out << "Column " << first_column;
		else
			out << "Columns " << first_column
				<< "-" << last_column;
	}
	else
	{
		out << "Line " << first_line << " @ Column " << first_column
		    << " - "
		    << "Line " << last_line << " @ Column " << last_column;
	}
	return out.str();
}

////////////////////////////////////////////////////////////////

// AST

AST::AST(LocationData const& location)
	: location(location), disabled(false)
{}

// ASTFile

ASTFile::ASTFile(LocationData const& location) : AST(location) {}

void ASTFile::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseFile(*this, param);
}

string ASTFile::asString() const
{
	return location.fname;
}

void ASTFile::addDeclaration(ASTDecl* declaration)
{
	switch (declaration->getDeclarationType())
	{
	case ASTDecl::TYPE_SCRIPT:
		scripts.push_back(static_cast<ASTScript*>(declaration));
		break;
	case ASTDecl::TYPE_IMPORT:
		imports.push_back(static_cast<ASTImportDecl*>(declaration));
		break;
	case ASTDecl::TYPE_FUNCTION:
		functions.push_back(static_cast<ASTFuncDecl*>(declaration));
		break;
	case ASTDecl::TYPE_DATALIST:
		variables.push_back(static_cast<ASTDataDeclList*>(declaration));
		break;
	case ASTDecl::TYPE_DATATYPE:
		dataTypes.push_back(static_cast<ASTDataTypeDef*>(declaration));
		break;
	case ASTDecl::TYPE_SCRIPTTYPE:
		scriptTypes.push_back(static_cast<ASTScriptTypeDef*>(declaration));
		break;
	}
}

bool ASTFile::hasDeclarations() const
{
	return !imports.empty()
		|| !variables.empty()
		|| !functions.empty()
		|| !dataTypes.empty()
		|| !scriptTypes.empty()
		|| !scripts.empty();
}

// ASTFloat

ASTFloat::ASTFloat(char* value, Type type, LocationData const& location)
	: AST(location), type(type), negative(false),
	  value(static_cast<string>(value))
{}
    
ASTFloat::ASTFloat(char const* value, Type type, LocationData const& location)
	: AST(location), type(type), negative(false),
	  value(static_cast<string>(value))
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

void ASTFloat::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseFloat(*this, param);
}

pair<string, string> ASTFloat::parseValue() const
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
	: AST(location), str(static_cast<string>(str))
{}

ASTString::ASTString(string const& str, LocationData const& location)
	: AST(location), str(str)
{}

void ASTString::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseString(*this, param);
}

// ASTSetOption

ASTSetOption::ASTSetOption(
		string const& name, ASTExprConst* expr,
		LocationData const& location)
	: AST(location), name(name),
	  option(CompileOption::get(name).value_or(CompileOption::Invalid)),
	  expr(expr)
{}

ASTSetOption::ASTSetOption(
		string const& name, CompileOptionSetting setting,
		LocationData const& location)
	: AST(location), name(name),
	  option(CompileOption::get(name).value_or(CompileOption::Invalid)),
	  expr(NULL), setting(setting)
{}

void ASTSetOption::execute(ASTVisitor& visitor, void* param)
{
	return visitor.caseSetOption(*this, param);
}

string ASTSetOption::asString() const
{
	return "#option " + name + " "
		+ (expr.get() ? expr->asString() : setting.asString());
}

CompileOptionSetting ASTSetOption::getSetting(
		CompileErrorHandler* handler, Scope* scope) const
{
	if (expr.get())
	{
		if (optional<long> value = expr->getCompileTimeValue(handler, scope))
			return CompileOptionSetting(*value);
		handler->handleError(CompileError::ExprNotConstant(this));
		return CompileOptionSetting::Invalid;
	}

	return setting;
}

////////////////////////////////////////////////////////////////
// Statements

// ASTStmt

ASTStmt::ASTStmt(LocationData const& location)
	: AST(location), disabled_(false)
{}

// ASTBlock

ASTBlock::ASTBlock(LocationData const& location) : ASTStmt(location) {}
    
void ASTBlock::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseBlock(*this, param);
}

// ASTStmtIf

ASTStmtIf::ASTStmtIf(ASTExpr* condition,
					 ASTStmt* thenStatement,
					 LocationData const& location)
	: ASTStmt(location), condition(condition), thenStatement(thenStatement)
{}

void ASTStmtIf::execute(ASTVisitor& visitor, void* param)
{
	return visitor.caseStmtIf(*this, param);
}

// ASTStmtIfElse
    
ASTStmtIfElse::ASTStmtIfElse(
		ASTExpr* condition, ASTStmt* thenStatement,
		ASTStmt* elseStatement, LocationData const& location)
	: ASTStmtIf(condition, thenStatement, location),
	  elseStatement(elseStatement)
{}

void ASTStmtIfElse::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtIfElse(*this, param);
}

// ASTStmtSwitch

ASTStmtSwitch::ASTStmtSwitch(LocationData const& location)
	: ASTStmt(location), key(NULL)
{}

void ASTStmtSwitch::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtSwitch(*this, param);
}

// ASTSwitchCases

ASTSwitchCases::ASTSwitchCases(LocationData const& location)
	: AST(location), isDefault(false), block(NULL)
{}

void ASTSwitchCases::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseSwitchCases(*this, param);
}

// ASTStmtFor

ASTStmtFor::ASTStmtFor(
		ASTStmt* setup, ASTExpr* test, ASTStmt* increment, ASTStmt* body,
		LocationData const& location)
	: ASTStmt(location), setup(setup), test(test), increment(increment),
	  body(body)
{}

void ASTStmtFor::execute(ASTVisitor& visitor, void* param)
{
	return visitor.caseStmtFor(*this, param);
}

// ASTStmtWhile

ASTStmtWhile::ASTStmtWhile(
		ASTExpr* test, ASTStmt* body, LocationData const& location)
	: ASTStmt(location), test(test), body(body)
{}

void ASTStmtWhile::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtWhile(*this, param);
}

// ASTStmtDo

ASTStmtDo::ASTStmtDo(
		ASTExpr* test, ASTStmt* body, LocationData const& location)
	: ASTStmt(location), test(test), body(body)
{}

void ASTStmtDo::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtDo(*this, param);
}

// ASTStmtReturn

ASTStmtReturn::ASTStmtReturn(LocationData const& location)
	: ASTStmt(location)
{}

void ASTStmtReturn::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtReturn(*this, param);
}

// ASTStmtReturnVal

ASTStmtReturnVal::ASTStmtReturnVal(
		ASTExpr* value, LocationData const& location)
	: ASTStmtReturn(location), value(value)
{}

void ASTStmtReturnVal::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtReturnVal(*this, param);
}

// ASTStmtBreak

ASTStmtBreak::ASTStmtBreak(LocationData const& location)
	: ASTStmt(location)
{}

void ASTStmtBreak::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtBreak(*this, param);
}

// ASTStmtContinue

ASTStmtContinue::ASTStmtContinue(LocationData const& location)
	: ASTStmt(location)
{}

void ASTStmtContinue::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtContinue(*this, param);
}

// ASTStmtEmpty

ASTStmtEmpty::ASTStmtEmpty(LocationData const& location)
	: ASTStmt(location)
{}

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

// ASTScript

ASTScript::ASTScript(LocationData const& location)
	: ASTDecl(location), type(NULL), name("") {}

void ASTScript::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseScript(*this, param);
}

void ASTScript::addDeclaration(ASTDecl& declaration)
{
	switch (declaration.getDeclarationType())
    {
	case ASTDecl::TYPE_FUNCTION:
		functions.push_back(static_cast<ASTFuncDecl*>(&declaration));
		break;
	case ASTDecl::TYPE_DATALIST:
		variables.push_back(static_cast<ASTDataDeclList*>(&declaration));
		break;
	case ASTDecl::TYPE_DATATYPE:
		types.push_back(static_cast<ASTDataTypeDef*>(&declaration));
		break;
	}
}

// ASTImportDecl

ASTImportDecl::ASTImportDecl(
		string const& filename, LocationData const& location)
	: ASTDecl(location), filename_(filename)
{}

void ASTImportDecl::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseImportDecl(*this,param);
}

// ASTFuncDecl

ASTFuncDecl::ASTFuncDecl(LocationData const& location)
	: ASTDecl(location), returnType(NULL), block(NULL)
{}

void ASTFuncDecl::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseFuncDecl(*this, param);
}

// ASTDataDeclList

ASTDataDeclList::ASTDataDeclList(LocationData const& location)
	: ASTDecl(location), baseType(NULL)
{}

ASTDataDeclList::ASTDataDeclList(ASTDataDeclList const& other)
	: ASTDecl(other),
	  baseType(other.baseType)
{
	for (vector<ASTDataDecl*>::const_iterator it =
		     other.declarations_.begin();
	     it != other.declarations_.end(); ++it)
		addDeclaration(*it);
}

ASTDataDeclList& ASTDataDeclList::operator=(ASTDataDeclList const& rhs)
{
	ASTDecl::operator=(rhs);

    baseType = rhs.baseType;
    declarations_.clear();
	for (vector<ASTDataDecl*>::const_iterator it = rhs.declarations_.begin();
	     it != rhs.declarations_.end(); ++it)
		addDeclaration(*it);
	
	return *this;
}

void ASTDataDeclList::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseDataDeclList(*this, param);
}

void ASTDataDeclList::addDeclaration(ASTDataDecl* declaration)
{
	// Declarations in a list should not have their own type.
	assert(!declaration->baseType);

	declaration->list = this;
	declarations_.push_back(declaration);
}

// ASTDataEnum

ASTDataEnum::ASTDataEnum(LocationData const& location)
	: ASTDataDeclList(location), nextVal(0)
{
	baseType = new ASTDataType(DataType::CFLOAT, location);
}

void ASTDataEnum::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseDataEnum(*this, param);
}

void ASTDataEnum::addDeclaration(ASTDataDecl* declaration)
{
	if(ASTExpr* init = declaration->getInitializer())
	{
		if(init->getCompileTimeValue())
		{
			nextVal = *init->getCompileTimeValue() / 10000;
		}
	}
	else
	{
		ASTNumberLiteral* value = new ASTNumberLiteral(new ASTFloat(nextVal, ASTFloat::TYPE_DECIMAL, location), location);
		declaration->setInitializer(value);
	}
	++nextVal;
	ASTDataDeclList::addDeclaration(declaration);
}

// ASTDataDecl

ASTDataDecl::ASTDataDecl(LocationData const& location)
	: ASTDecl(location), list(NULL), manager(NULL),
	  baseType(NULL), initializer_(NULL)
{}

ASTDataDecl::ASTDataDecl(ASTDataDecl const& other)
	: ASTDecl(other),
	  list(NULL), manager(NULL),
	  baseType(other.baseType),
	  name(other.name),
	  extraArrays(other.extraArrays)
{
	setInitializer(other.initializer_.clone());
}

ASTDataDecl& ASTDataDecl::operator=(ASTDataDecl const& rhs)
{
	ASTDecl::operator=(rhs);

	list = NULL;
	manager = NULL;
	baseType = rhs.baseType;
	name = rhs.name;
	extraArrays = rhs.extraArrays;
	setInitializer(rhs.initializer_.clone());

	return *this;
}

void ASTDataDecl::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseDataDecl(*this, param);
}

void ASTDataDecl::setInitializer(ASTExpr* initializer)
{
	initializer_ = initializer;

	// Give a string or array literal a reference back to this object so it
	// can grab size information.
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

DataType const* ASTDataDecl::resolveType(ZScript::Scope* scope)
{
	TypeStore& typeStore = scope->getTypeStore();

	// First resolve the base type.
	ASTDataType* baseTypeNode = list ? list->baseType.get() : baseType.get();
	DataType const* type = &baseTypeNode->resolve(*scope);

	// If we have any arrays, tack them onto the base type.
	for (vector<ASTDataDeclExtraArray*>::const_iterator it = extraArrays.begin();
		 it != extraArrays.end(); ++it)
	{
		DataTypeArray arrayType(*type);
		type = typeStore.getCanonicalType(arrayType);
	}

	return type;
}

bool ZScript::hasSize(ASTDataDecl const& decl)
{
	for (vector<ASTDataDeclExtraArray*>::const_iterator it =
		     decl.extraArrays.begin();
	     it != decl.extraArrays.end(); ++it)
		if ((*it)->hasSize()) return true;
	return false;
}

// ASTDataDeclExtraArray

ASTDataDeclExtraArray::ASTDataDeclExtraArray(LocationData const& location)
	: AST(location)
{}

void ASTDataDeclExtraArray::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseDataDeclExtraArray(*this, param);
}

optional<int> ASTDataDeclExtraArray::getCompileTimeSize(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (dimensions.size() == 0) return nullopt;
	int size = 1;
	for (vector<ASTExpr*>::const_iterator it = dimensions.begin();
		 it != dimensions.end(); ++it)
	{
		ASTExpr& expr = **it;
		if (optional<long> value = expr.getCompileTimeValue(errorHandler, scope))
			size *= *value / 10000L;
		else
			return nullopt;
	}
	return size;
}

// ASTDataTypeDef

ASTDataTypeDef::ASTDataTypeDef(
		ASTDataType* type, string const& name, LocationData const& location)
	 : ASTDecl(location), type(type), name(name)
{}

void ASTDataTypeDef::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseDataTypeDef(*this, param);
}

// ASTScriptTypeDef

ASTScriptTypeDef::ASTScriptTypeDef(ASTScriptType* oldType,
                                   std::string const& newName,
                                   LocationData const& location)
		: ASTDecl(location), oldType(oldType), newName(newName)
{}

void ASTScriptTypeDef::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseScriptTypeDef(*this, param);
}

////////////////////////////////////////////////////////////////
// Expressions

// ASTExpr

ASTExpr::ASTExpr(LocationData const& location)
	: ASTStmt(location)
{}

// ASTExprConst

ASTExprConst::ASTExprConst(ASTExpr* content, LocationData const& location)
	: ASTExpr(location), content(content)
{}

void ASTExprConst::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprConst(*this, param);
}

optional<long> ASTExprConst::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	return content ? content->getCompileTimeValue(errorHandler, scope) : nullopt;
}

// ASTExprAssign

ASTExprAssign::ASTExprAssign(ASTExpr* left, ASTExpr* right,
							 LocationData const& location)
	: ASTExpr(location), left(left), right(right) {}

void ASTExprAssign::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprAssign(*this, param);
}

optional<long> ASTExprAssign::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	return right ? right->getCompileTimeValue(errorHandler, scope) : nullopt;
}

// ASTExprIdentifier

ASTExprIdentifier::ASTExprIdentifier(string const& name,
									 LocationData const& location)
	: ASTExpr(location), binding(NULL), constant_(false)
{
	if (name != "") components.push_back(name);
}

void ASTExprIdentifier::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprIdentifier(*this, param);
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

optional<long> ASTExprIdentifier::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	return binding ? binding->getCompileTimeValue() : nullopt;
}

DataType const* ASTExprIdentifier::getReadType() const
{
	return binding ? &binding->type : NULL;
}

DataType const* ASTExprIdentifier::getWriteType() const
{
	return binding ? &binding->type : NULL;
}

// ASTExprArrow

ASTExprArrow::ASTExprArrow(ASTExpr* left, string const& right,
						   LocationData const& location)
	: ASTExpr(location), left(left), right(right), index(NULL),
	  readFunction(NULL), writeFunction(NULL), leftClass(NULL)
{}

void ASTExprArrow::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprArrow(*this, param);
}

string ASTExprArrow::asString() const
{
	string s = left->asString() + "->" + right;
	if (index != NULL) s += "[" + index->asString() + "]";
	return s;
}

DataType const* ASTExprArrow::getReadType() const
{
	return readFunction ? readFunction->returnType : NULL;
}

DataType const* ASTExprArrow::getWriteType() const
{
	return writeFunction ? writeFunction->paramTypes.back() : NULL;
}

// ASTExprIndex

ASTExprIndex::ASTExprIndex(ASTExpr* array, ASTExpr* index,
						   LocationData const& location)
	: ASTExpr(location), array(array), index(index)
{}

void ASTExprIndex::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprIndex(*this, param);
}

bool ASTExprIndex::isConstant() const
{
	if (array == NULL || index == NULL) return false;
	return array->isConstant() && index->isConstant();
}

DataType const* ASTExprIndex::getReadType() const
{
	DataType const* type = array->getReadType();
	if (type && type->isArray() && !array->isTypeArrow())
	{
		DataTypeArray const* atype = static_cast<DataTypeArray const*>(type);
		type = &atype->getElementType();
	}
	return type;
}

DataType const* ASTExprIndex::getWriteType() const
{
	DataType const* type = array->getWriteType();
	if (type && type->isArray() && !array->isTypeArrow())
	{
		DataTypeArray const* atype = static_cast<DataTypeArray const*>(type);
		type = &atype->getElementType();
	}
	return type;
}
	
// ASTExprCall

ASTExprCall::ASTExprCall(LocationData const& location)
	: ASTExpr(location), binding(NULL)
{}

void ASTExprCall::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprCall(*this, param);
}

DataType const* ASTExprCall::getReadType() const
{
	return binding ? binding->returnType : NULL;
}

DataType const* ASTExprCall::getWriteType() const
{
	return NULL;
}

// ASTUnaryExpr

ASTUnaryExpr::ASTUnaryExpr(LocationData const& location)
	: ASTExpr(location)
{}

// ASTExprNegate

ASTExprNegate::ASTExprNegate(LocationData const& location)
	: ASTUnaryExpr(location)
{}

void ASTExprNegate::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprNegate(*this, param);
}

optional<long> ASTExprNegate::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!operand) return nullopt;
	if (optional<long> value = operand->getCompileTimeValue(errorHandler, scope))
		return -*value;
	return nullopt;
}

// ASTExprNot

ASTExprNot::ASTExprNot(LocationData const& location)
	: ASTUnaryExpr(location)
{}

void ASTExprNot::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprNot(*this, param);
}

optional<long> ASTExprNot::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!operand) return nullopt;
	if (optional<long> value = operand->getCompileTimeValue(errorHandler, scope))
		return *value ? 0L : 10000L;
	return nullopt;
}

// ASTExprBitNot

ASTExprBitNot::ASTExprBitNot(LocationData const& location)
	: ASTUnaryExpr(location)
{}

void ASTExprBitNot::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprBitNot(*this, param);
}

optional<long> ASTExprBitNot::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!operand) return nullopt;
	if (optional<long> value = operand->getCompileTimeValue(errorHandler, scope))
		return ~(*value / 10000L) * 10000L;
	return nullopt;
}

// ASTExprIncrement

ASTExprIncrement::ASTExprIncrement(LocationData const& location)
	: ASTUnaryExpr(location)
{}

void ASTExprIncrement::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprIncrement(*this, param);
}

// ASTExprPreIncrement

ASTExprPreIncrement::ASTExprPreIncrement(LocationData const& location)
	: ASTUnaryExpr(location)
{}

void ASTExprPreIncrement::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprPreIncrement(*this, param);
}

// ASTExprDecrement

ASTExprDecrement::ASTExprDecrement(LocationData const& location)
	: ASTUnaryExpr(location)
{}

void ASTExprDecrement::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprDecrement(*this, param);
}

// ASTExprPreDecrement

ASTExprPreDecrement::ASTExprPreDecrement(LocationData const& location)
	: ASTUnaryExpr(location)
{}

void ASTExprPreDecrement::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprPreDecrement(*this, param);
}

// ASTBinaryExpr

ASTBinaryExpr::ASTBinaryExpr(ASTExpr* left, ASTExpr* right,
							 LocationData const& location)
	: ASTExpr(location), left(left), right(right)
{}

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

// ASTExprAnd

ASTExprAnd::ASTExprAnd(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTLogExpr(left, right, location)
{}

void ASTExprAnd::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprAnd(*this, param);
}

optional<long> ASTExprAnd::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;
	return (*leftValue && *rightValue) ? 10000L : 0L;
}

// ASTExprOr

ASTExprOr::ASTExprOr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTLogExpr(left, right, location)
{}

void ASTExprOr::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprOr(*this, param);
}

optional<long> ASTExprOr::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;
	return (*leftValue || *rightValue) ? 10000L : 0L;
}

// ASTRelExpr

ASTRelExpr::ASTRelExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

// ASTExprGT

ASTExprGT::ASTExprGT(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

void ASTExprGT::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprGT(*this, param);
}

optional<long> ASTExprGT::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;
	return (*leftValue > *rightValue) ? 10000L : 0L;
}

// ASTExprGE

ASTExprGE::ASTExprGE(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

void ASTExprGE::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprGE(*this, param);
}

optional<long> ASTExprGE::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;
	return (*leftValue >= *rightValue) ? 10000L : 0L;
}

// ASTExprLT

ASTExprLT::ASTExprLT(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

void ASTExprLT::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprLT(*this, param);
}

optional<long> ASTExprLT::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;
	return (*leftValue < *rightValue) ? 10000L : 0L;
}

// ASTExprLE

ASTExprLE::ASTExprLE(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

void ASTExprLE::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprLE(*this, param);
}

optional<long> ASTExprLE::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;
	return (*leftValue <= *rightValue) ? 10000L : 0L;
}

// ASTExprEQ

ASTExprEQ::ASTExprEQ(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

void ASTExprEQ::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprEQ(*this, param);
}

optional<long> ASTExprEQ::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;
	return (*leftValue == *rightValue) ? 10000L : 0L;
}

// ASTExprNE

ASTExprNE::ASTExprNE(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

void ASTExprNE::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprNE(*this, param);
}

optional<long> ASTExprNE::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;
	return (*leftValue != *rightValue) ? 10000L : 0L;
}

// ASTAddExpr

ASTAddExpr::ASTAddExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

// ASTExprPlus

ASTExprPlus::ASTExprPlus(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTAddExpr(left, right, location)
{}

void ASTExprPlus::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprPlus(*this, param);
}

optional<long> ASTExprPlus::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;
	return *leftValue + *rightValue;
}

// ASTExprMinus

ASTExprMinus::ASTExprMinus(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTAddExpr(left, right, location)
{}

void ASTExprMinus::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprMinus(*this, param);
}

optional<long> ASTExprMinus::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;
	return *leftValue - *rightValue;
}

// ASTMultExpr

ASTMultExpr::ASTMultExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

// ASTExprTimes

ASTExprTimes::ASTExprTimes(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTMultExpr(left, right, location)
{}

void ASTExprTimes::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprTimes(*this, param);
}

optional<long> ASTExprTimes::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;

	return long(*leftValue * (*rightValue / 10000.0));
}

// ASTExprDivide

ASTExprDivide::ASTExprDivide(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTMultExpr(left, right, location)
{}

void ASTExprDivide::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprDivide(*this, param);
}

optional<long> ASTExprDivide::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;

	if (*rightValue == 0)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::DivByZero(this));
		return nullopt;
	}
	
	if(*lookupOption(*scope, CompileOption::OPT_TRUNCATE_DIVISION_BY_LITERAL_BUG)
		&& left->isLiteral() && right->isLiteral())
	{
		return *leftValue / *rightValue * 10000L;
	}
	
	return static_cast<long>((*leftValue * 1.0) / (*rightValue * 1.0) * (10000L));
}

// ASTExprModulo
    
ASTExprModulo::ASTExprModulo(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTMultExpr(left, right, location)
{}

void ASTExprModulo::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprModulo(*this, param);
}

optional<long> ASTExprModulo::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;

	if (*rightValue == 0)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::DivByZero(this));
		return nullopt;
	}
	return *leftValue % *rightValue;
}

// ASTBitExpr

ASTBitExpr::ASTBitExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

// ASTExprBitAnd

ASTExprBitAnd::ASTExprBitAnd(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBitExpr(left, right, location)
{}

void ASTExprBitAnd::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprBitAnd(*this, param);
}

optional<long> ASTExprBitAnd::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;

	return ((*leftValue / 10000L) & (*rightValue / 10000L)) * 10000L;
}

// ASTExprBitOr

ASTExprBitOr::ASTExprBitOr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBitExpr(left, right, location)
{}

void ASTExprBitOr::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprBitOr(*this, param);
}

optional<long> ASTExprBitOr::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;

	return ((*leftValue / 10000L) | (*rightValue / 10000L)) * 10000L;
}

// ASTExprBitXor

ASTExprBitXor::ASTExprBitXor(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBitExpr(left, right, location)
{}

void ASTExprBitXor::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprBitXor(*this, param);
}

optional<long> ASTExprBitXor::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;

	return ((*leftValue / 10000L) ^ (*rightValue / 10000L)) * 10000L;
}

// ASTShiftExpr

ASTShiftExpr::ASTShiftExpr(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTBinaryExpr(left, right, location)
{}

// ASTExprLShift

ASTExprLShift::ASTExprLShift(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTShiftExpr(left, right, location)
{}

void ASTExprLShift::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprLShift(*this, param);
}

optional<long> ASTExprLShift::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;

	if (*rightValue % 10000L)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::ShiftNotInt(this));
		rightValue = (*rightValue / 10000L) * 10000L;
	}
	
	return ((*leftValue / 10000L) << (*rightValue / 10000L)) * 10000L;
}

// ASTExprRShift

ASTExprRShift::ASTExprRShift(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTShiftExpr(left, right, location)
{}

void ASTExprRShift::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprRShift(*this, param);
}

optional<long> ASTExprRShift::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return nullopt;

	if (*rightValue % 10000L)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::ShiftNotInt(this));
		rightValue = (*rightValue / 10000L) * 10000L;
	}
	
	return ((*leftValue / 10000L) >> (*rightValue / 10000L)) * 10000L;
}

// ASTTernaryExpr

ASTTernaryExpr::ASTTernaryExpr(ASTExpr* left, ASTExpr* middle, ASTExpr* right,
							 LocationData const& location)
	: ASTExpr(location), left(left), middle(middle), right(right)
{}

bool ASTTernaryExpr::isConstant() const
{
	if (left && !left->isConstant()) return false;
	if (middle && !middle->isConstant()) return false;
	if (right && !right->isConstant()) return false;
	return true;
}

void ASTTernaryExpr::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprTernary(*this, param);
}

optional<long> ASTTernaryExpr::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (!left || !middle || !right) return nullopt;
	optional<long> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return nullopt;
	optional<long> middleValue = middle->getCompileTimeValue(errorHandler, scope);
	optional<long> rightValue = right->getCompileTimeValue(errorHandler, scope);
	
	if(*leftValue)
	{
		if(!middleValue) return nullopt;
		return *middleValue;
	}
	else
	{
		if (!rightValue) return nullopt;
		return *rightValue;
	}
}

////////////////////////////////////////////////////////////////
// Literals

// ASTLiteral

ASTLiteral::ASTLiteral(LocationData const& location)
	: ASTExpr(location), manager(NULL)
{}

// ASTNumberLiteral

ASTNumberLiteral::ASTNumberLiteral(
		ASTFloat* value, LocationData const& location)
	: ASTLiteral(location), value(value)
{}

void ASTNumberLiteral::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseNumberLiteral(*this, param);
}

optional<long> ASTNumberLiteral::getCompileTimeValue(
	CompileErrorHandler* errorHandler, Scope* scope) const
{
	if (!value) return nullopt;
    pair<long, bool> val = ScriptParser::parseLong(value->parseValue());

    if (!val.second && errorHandler)
	    errorHandler->handleError(
			    CompileError::ConstTrunc(this, value->value.c_str()));

	return val.first;
}

// ASTBoolLiteral

ASTBoolLiteral::ASTBoolLiteral(bool value, LocationData const& location)
	: ASTLiteral(location), value(value)
{}

void ASTBoolLiteral::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseBoolLiteral(*this, param);
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
	  value(raw.getValue()),
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

DataTypeArray const* ASTStringLiteral::getReadType() const
{
	return &DataType::STRING;
}


// ASTArrayLiteral

ASTArrayLiteral::ASTArrayLiteral(LocationData const& location)
	: ASTLiteral(location), type(NULL), size(NULL), declaration(NULL),
	  readType_(NULL)
{}

ASTArrayLiteral::ASTArrayLiteral(ASTArrayLiteral const& other)
	: ASTLiteral(other),
	  declaration(NULL),
	  type(other.type),
	  size(other.size),
	  elements(other.elements),
	  readType_(NULL)
{}

ASTArrayLiteral& ASTArrayLiteral::operator=(ASTArrayLiteral const& rhs)
{
	ASTLiteral::operator=(rhs);

	type = rhs.type;
	size = rhs.size;
	elements = rhs.elements;
	declaration = NULL;
	readType_ = NULL;

	return *this;
}

void ASTArrayLiteral::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseArrayLiteral(*this, param);
}

// ASTOptionValue

ASTOptionValue::ASTOptionValue(
		string const& name, LocationData const& location)
	: ASTLiteral(location),
	  name(name),
	  option(CompileOption::get(name).value_or(CompileOption::Invalid)),
	  value(nullopt)
{}

void ASTOptionValue::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseOptionValue(*this, param);
}

std::string ASTOptionValue::asString() const
{
	return "OPTION_VALUE(" + *option.getName() + ")";
}

////////////////////////////////////////////////////////////////
// Types

// ASTScriptType

ASTScriptType::ASTScriptType(ScriptType type, LocationData const& location)
		: AST(location), type(type)
{}

ASTScriptType::ASTScriptType(string const& name, LocationData const& location)
		: AST(location), name(name)
{}

void ASTScriptType::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseScriptType(*this, param);
}

ScriptType ZScript::resolveScriptType(ASTScriptType const& node,
                                      Scope const& scope)
{
	if (node.type.isValid()) return node.type;
	return lookupScriptType(scope, node.name);
}

// ASTDataType

ASTDataType::ASTDataType(DataType* type, LocationData const& location)
	: AST(location), type(type)
{}

ASTDataType::ASTDataType(DataType const& type, LocationData const& location)
	: AST(location), type(type.clone())
{}

void ASTDataType::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseDataType(*this, param);
}

DataType const& ASTDataType::resolve(ZScript::Scope& scope)
{
	DataType* resolved = type->resolve(scope);
	if (resolved && type != resolved)
		type.reset(resolved);
	return *type;
}


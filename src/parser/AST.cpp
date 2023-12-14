#include "ASTVisitors.h"
#include "CompilerUtils.h"
#include "DataStructs.h"
#include "Scope.h"

#include <assert.h>
#include <sstream>

#include "zc/ffscript.h"
extern FFScript FFCore;

using std::pair;
using std::string;
using std::ostringstream;
using std::vector;
using namespace ZScript;
using namespace util;

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
	: location(location), errorDisabled(false), disabled_(false), isRegistered(false), isReachable(false)
{}

// ASTFile

ASTFile::ASTFile(LocationData const& location) : AST(location), scope(NULL) {}

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
	case ASTDecl::TYPE_CLASS:
		classes.push_back(static_cast<ASTClass*>(declaration));
		break;
	case ASTDecl::TYPE_IMPORT:
		imports.push_back(static_cast<ASTImportDecl*>(declaration));
		break;
	case ASTDecl::TYPE_IMPORT_COND:
		condimports.push_back(static_cast<ASTImportCondDecl*>(declaration));
		break;
	case ASTDecl::TYPE_INCLUDE_PATH:
		inclpaths.push_back(static_cast<ASTIncludePath*>(declaration));
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
	case ASTDecl::TYPE_NAMESPACE:
		namespaces.push_back(static_cast<ASTNamespace*>(declaration));
		break;
	case ASTDecl::TYPE_USING:
		use.push_back(static_cast<ASTUsingDecl*>(declaration));
		break;
	case ASTDecl::TYPE_ASSERT:
		asserts.push_back(static_cast<ASTAssert*>(declaration));
		break;
	}
}

bool ASTFile::hasDeclarations() const
{
	return !imports.empty()
		|| !condimports.empty()
		|| !variables.empty()
		|| !functions.empty()
		|| !dataTypes.empty()
		|| !scriptTypes.empty()
		|| !scripts.empty()
		|| !namespaces.empty()
		|| !use.empty()
		|| !asserts.empty();
}

// ASTFloat

ASTFloat::ASTFloat(char* val, Type type, LocationData const& location)
	: AST(location), type(type), value(static_cast<string>(val)),
	negative(false)
{
	initNeg();
}
    
ASTFloat::ASTFloat(char const* val, Type type, LocationData const& location)
	: AST(location), type(type), value(static_cast<string>(val)),
	negative(false)
{
	initNeg();
}
    
ASTFloat::ASTFloat(
		string const& val, Type type, LocationData const& location)
	: AST(location), type(type), value(val), negative(false)
{
	initNeg();
}

ASTFloat::ASTFloat(int32_t val, Type type, LocationData const& location)
	: AST(location), type(type), negative(false)
{
	char tmp[15];
	sprintf(tmp, "%d", val);
	value = string(tmp);
	initNeg();
}

ASTFloat::ASTFloat(int32_t ipart, int32_t dpart, LocationData const& location)
	: AST(location), type(TYPE_DECIMAL), negative(false)
{
	char tmp[13];
	if(dpart)
		sprintf(tmp,"%d.%04d",ipart,dpart);
	else
		sprintf(tmp,"%d",ipart);
	value = string(tmp);
	initNeg();
}

void ASTFloat::initNeg()
{
	if(value[0] == '-')
	{
		negative = !negative;
		value = value.substr(1);
	}
}

void ASTFloat::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseFloat(*this, param);
}

pair<string, string> ASTFloat::parseValue(Scope* scope) const
{
	string f = value;
	string intpart;
	string fpart;
	bool is_long = false;
	bool alt = false;
	bool neg = negative;
	switch(type)
	{
		case TYPE_L_DECIMAL: case TYPE_L_BINARY: case TYPE_L_HEX: case TYPE_L_OCTAL:
		case TYPE_L_BINARY_2: case TYPE_L_OCTAL_2:
			is_long = true;
			break;
	}
	switch(type)
	{
		case TYPE_BINARY_2: case TYPE_L_BINARY_2:
		case TYPE_OCTAL_2: case TYPE_L_OCTAL_2:
			alt = true;
			break;
	}

	switch(type)
	{
		case TYPE_DECIMAL:
		{
			bool founddot = false;

			for(size_t i=0; i<f.size(); i++)
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

			if(neg) intpart = "-" + intpart;

			break;
		}
		
		case TYPE_L_DECIMAL:
		{
			// Trim off the "L".
			f = f.substr(0,f.size()-1);
			if(f.at(0) == '-')
			{
				neg = !neg;
				f = f.substr(1, f.size()-1);
			}
			//Add leading zero's up to the decimal place
			while(f.size() < 4)
				f = "0" + f;
			if(f.size() == 4)
			{
				intpart = "0";
				fpart = f;
			}
			else
			{
				intpart = f.substr(0, f.size()-4);
				fpart = f.substr(f.size()-4, 4);
			}
			if(neg) intpart = "-" + intpart;
			break;
		}
		
		case TYPE_L_HEX:
		case TYPE_HEX:
		{
			if(is_long)
			{
				// Trim off the "0x" and "L".
				f = f.substr(2,f.size()-3);
			}
			else
			{
				// Trim off the "0x".
				f = f.substr(2,f.size()-2);
			}
			// Parse the hex.
			int32_t val2=0;
		
			for(size_t i=0; i<f.size(); i++)
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
		
			if(neg && val2 > 0) val2 *= -1;

			char temp[60];
			char temp2[60];
			if(is_long)
			{
				sprintf(temp, "%ld", abs(val2 / 10000L));
				sprintf(temp2, "%04ld", abs(val2 % 10000L));
				intpart = temp;
				if(val2 < 0) intpart = "-" + intpart;
				fpart = temp2;
			}
			else
			{
				sprintf(temp, "%d", val2);
				intpart = temp;
				fpart = "";
			}
			break;
		}

		case TYPE_OCTAL:
		case TYPE_L_OCTAL:
		case TYPE_OCTAL_2:
		case TYPE_L_OCTAL_2:
		{
			if(alt)
			{
				//Trim '0o' prefix
				f = f.substr(2,f.size()-2);
				if(is_long) //...and 'L' suffix
					f = f.substr(0,f.size()-1);
			}
			else if(is_long)
			{
				// Trim off the "oL".
				f = f.substr(0,f.size()-2);
			}
			else
			{
				// Trim off the "o".
				f = f.substr(0,f.size()-1);
			}
			// Parse the octal.
			int32_t val2=0;
		
			for(size_t i=0; i<f.size(); i++)
			{
				char d = f.at(i);
				val2*=8;

				val2+=(d-'0');
			}
		
			if(neg && val2 > 0) val2 *= -1;

			char temp[60];
			char temp2[60];
			if(is_long)
			{
				sprintf(temp, "%ld", abs(val2 / 10000L));
				sprintf(temp2, "%04ld", abs(val2 % 10000L));
				intpart = temp;
				if(val2 < 0) intpart = "-" + intpart;
				fpart = temp2;
			}
			else
			{
				sprintf(temp, "%d", val2);
				intpart = temp;
				fpart = "";
			}
			break;
		}

		case TYPE_BINARY:
		case TYPE_L_BINARY:
		case TYPE_BINARY_2:
		case TYPE_L_BINARY_2:
		{
			if(alt)
			{
				//Trim '0b' prefix
				f = f.substr(2,f.size()-2);
				if(is_long) //...and 'L' suffix
					f = f.substr(0,f.size()-1);
			}
			else if(is_long)
			{
				//trim 'Lb' / 'bL' suffix
				f = f.substr(0,f.size()-2);
			}
			else
			{
				//trim 'b' suffix
				f = f.substr(0,f.size()-1);
			}
			int32_t val2=0;
			if(is_long || (*lookupOption(scope, CompileOption::OPT_BINARY_32BIT) != 0))
			{
				if(f.size() > 32)
				{
					f = f.substr(f.size() - 32, 32);
				}
				if(f.size() == 32)
				{
					/*if(f.find_first_of('0') == string::npos)
					{
						val2 = -1;
						goto parselong_skipbinary;
					}*/
				}
			}
			else if(f.size() > 18)
			{
				f = f.substr(f.size() - 18, 18);
			}
			
			for(size_t i=0; i<f.size(); i++)
			{
				val2<<=1;
				if(f.at(i) == '1') val2 |= 1;
			}

			if(neg && val2 > 0) val2 *= -1;

			if(is_long || (*lookupOption(scope, CompileOption::OPT_BINARY_32BIT) != 0))
			{
				char temp[60];
				sprintf(temp, "%d", abs(val2/10000));
				intpart = temp;
				if(val2 < 0) intpart = "-" + intpart;
				sprintf(temp, "%04d", abs(val2%10000));
				fpart = temp;
			}
			else
			{
				char temp[60];
				sprintf(temp, "%d", val2);
				intpart = temp;
				fpart = "";
			}
			break;
		}
	}

	return pair<string,string>(intpart, fpart);
}
int32_t ASTFloat::getValue(Scope* scope)
{
	string f = value;
	int32_t outval = 0;
	bool is_long = false;
	bool alt = false;
	bool neg = negative;
	switch(type)
	{
		case TYPE_L_DECIMAL: case TYPE_L_BINARY: case TYPE_L_HEX: case TYPE_L_OCTAL:
		case TYPE_L_BINARY_2: case TYPE_L_OCTAL_2:
			is_long = true;
			break;
	}
	switch(type)
	{
		case TYPE_BINARY_2: case TYPE_L_BINARY_2:
		case TYPE_OCTAL_2: case TYPE_L_OCTAL_2:
			alt = true;
			break;
	}

	switch(type)
	{
		case TYPE_DECIMAL:
		{
			bool founddot = false;

			for(size_t i=0; i<f.size(); i++)
			{
				if(f.at(i) == '.')
				{
					outval = std::stoi(f.substr(0,i))*10000 + std::stoi(f.substr(i+1,zc_min(4,f.size()-i-1)));
					founddot = true;
					break;
				}
			}

			if(!founddot)
			{
				outval = std::stoi(f)*10000;
			}

			if(neg) outval = -outval;
			break;
		}
		
		case TYPE_L_DECIMAL:
		{
			// Trim off the "L".
			f = f.substr(0,f.size()-1);
			outval = std::stoi(f);
			if(neg) outval = -outval;
			break;
		}
		
		case TYPE_L_HEX:
		case TYPE_HEX:
		{
			if(is_long)
			{
				// Trim off the "0x" and "L".
				f = f.substr(2,f.size()-3);
			}
			else
			{
				// Trim off the "0x".
				f = f.substr(2,f.size()-2);
			}
			// Parse the hex.
			for(size_t i=0; i<f.size(); i++)
			{
				char d = f.at(i);
				outval*=16;

				if('0' <= d && d <= '9')
					outval+=(d-'0');
				else if('A' <= d && d <= 'F')
					outval+=(10+d-'A');
				else
					outval+=(10+d-'a');
			}
		
			if(neg && outval > 0) outval *= -1;
			break;
		}

		case TYPE_OCTAL:
		case TYPE_L_OCTAL:
		case TYPE_OCTAL_2:
		case TYPE_L_OCTAL_2:
		{
			if(alt)
			{
				//Trim '0o' prefix
				f = f.substr(2,f.size()-2);
				if(is_long) //...and 'L' suffix
					f = f.substr(0,f.size()-1);
			}
			else if(is_long)
			{
				// Trim off the "oL".
				f = f.substr(0,f.size()-2);
			}
			else
			{
				// Trim off the "o".
				f = f.substr(0,f.size()-1);
			}
			// Parse the octal.
			for(size_t i=0; i<f.size(); i++)
			{
				char d = f.at(i);
				outval*=8;

				outval+=(d-'0');
			}
		
			if(neg && outval > 0) outval *= -1;
			break;
		}

		case TYPE_BINARY:
		case TYPE_L_BINARY:
		case TYPE_BINARY_2:
		case TYPE_L_BINARY_2:
		{
			if(alt)
			{
				//Trim '0b' prefix
				f = f.substr(2,f.size()-2);
				if(is_long) //...and 'L' suffix
					f = f.substr(0,f.size()-1);
			}
			else if(is_long)
			{
				//trim 'Lb' / 'bL' suffix
				f = f.substr(0,f.size()-2);
			}
			else
			{
				//trim 'b' suffix
				f = f.substr(0,f.size()-1);
			}
			if(is_long || (*lookupOption(scope, CompileOption::OPT_BINARY_32BIT) != 0))
			{
				if(f.size() > 32)
				{
					f = f.substr(f.size() - 32, 32);
				}
				if(f.size() == 32)
				{
					/*if(f.find_first_of('0') == string::npos)
					{
						val2 = -1;
						goto parselong_skipbinary;
					}*/
				}
			}
			else if(f.size() > 18)
			{
				f = f.substr(f.size() - 18, 18);
			}
			
			for(size_t i=0; i<f.size(); i++)
			{
				outval<<=1;
				if(f.at(i) == '1') outval |= 1;
			}

			if(neg && outval > 0) outval *= -1;
			break;
		}
	}

	return outval;
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

// ASTAnnotation

ASTAnnotation::ASTAnnotation(ASTString* key, ASTString* strval, LocationData const& location)
	: AST(location), key(key), strval(strval)
{}
ASTAnnotation::ASTAnnotation(ASTString* key, ASTFloat* intval, LocationData const& location)
	: AST(location), key(key), intval(intval)
{}

void ASTAnnotation::execute(ASTVisitor& visitor, void* param)
{
	
}

// ASTAnnotationList

ASTAnnotationList::ASTAnnotationList(LocationData const& location)
	: AST(location)
{}

void ASTAnnotationList::execute(ASTVisitor& visitor, void* param)
{
	
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
		CompileErrorHandler* handler, Scope* scope)
{
	if (expr.get())
	{
		if (std::optional<int32_t> value = expr->getCompileTimeValue(handler, scope))
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
	: AST(location)
{}

// ASTBlock

ASTBlock::ASTBlock(LocationData const& location) : ASTStmt(location), scope(NULL) {}

void ASTBlock::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseBlock(*this, param);
}

// ASTStmtIf

ASTStmtIf::ASTStmtIf(ASTExpr* condition,
					 ASTBlock* thenStatement,
					 LocationData const& location)
	: ASTStmt(location), condition(condition), declaration(NULL), thenStatement(thenStatement), inverted(false), _isDecl(false),
		scope(NULL)
{}
ASTStmtIf::ASTStmtIf(ASTDataDecl* declaration,
					 ASTBlock* thenStatement,
					 LocationData const& location)
	: ASTStmt(location), condition(NULL), declaration(declaration), thenStatement(thenStatement), inverted(false), _isDecl(true),
		scope(NULL)
{}

void ASTStmtIf::execute(ASTVisitor& visitor, void* param)
{
	return visitor.caseStmtIf(*this, param);
}

// ASTStmtIfElse
    
ASTStmtIfElse::ASTStmtIfElse(
		ASTExpr* condition, ASTBlock* thenStatement,
		ASTBlock* elseStatement, LocationData const& location)
	: ASTStmtIf(condition, thenStatement, location),
	  elseStatement(elseStatement)
{}
ASTStmtIfElse::ASTStmtIfElse(
		ASTDataDecl* declaration, ASTBlock* thenStatement,
		ASTBlock* elseStatement, LocationData const& location)
	: ASTStmtIf(declaration, thenStatement, location),
	  elseStatement(elseStatement)
{}

void ASTStmtIfElse::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtIfElse(*this, param);
}

// ASTStmtSwitch

ASTStmtSwitch::ASTStmtSwitch(LocationData const& location)
	: ASTStmt(location), key(NULL), isString(false)
{}

void ASTStmtSwitch::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtSwitch(*this, param);
}

optional<vector<ASTSwitchCases*>> ASTStmtSwitch::getCompileTimeCases(
	CompileErrorHandler* errorHandler, Scope* scope)
{
	if(isString) return nullopt;
	if(!key) return nullopt;
	auto keyval = key->getCompileTimeValue(errorHandler, scope);
	if(!keyval) return nullopt;
	vector<ASTSwitchCases*> ret;
	if(cases.size() == 1 && cases.back()->isDefault)
	{
		ret.push_back(cases.back());
		return ret;
	}
	ASTSwitchCases* defcase = nullptr;
	ASTSwitchCases* foundcase = nullptr;
	for (auto it = cases.begin(); it != cases.end(); ++it)
	{
		ASTSwitchCases* cases = *it;
		bool found = false;
		// Run the tests for these cases.
		for (auto it = cases->cases.begin(); !found && it != cases->cases.end(); ++it)
		{
			// Test this individual case.
			if(auto val = (*it)->getCompileTimeValue(errorHandler, scope))
			{
				if(*val == *keyval)
					found = true;
			}
		}
		for (auto it = cases->ranges.begin(); !found && it != cases->ranges.end(); ++it)
		{
			ASTRange& range = **it;
			//Test each full range
			auto low_val = (*range.start).getCompileTimeValue(errorHandler, scope);
			auto high_val = (*range.end).getCompileTimeValue(errorHandler, scope);
			
			if(low_val && high_val && (*low_val <= *keyval && *high_val >= *keyval))
			{
				found = true;
			}
		}

		// If this set includes the default case, mark it.
		if (cases->isDefault)
			defcase = cases;
		if(found)
		{
			foundcase = cases;
			break;
		}
	}
	if(!foundcase) foundcase = defcase;
	if(!foundcase)
		return ret; //empty vector
	
	//Grab the foundcase, and every case after it (that it might fallthrough into)
	bool hit_found = false;
	for (auto it = cases.begin(); it != cases.end(); ++it)
	{
		ASTSwitchCases* cases = *it;
		if(!hit_found)
		{
			if(foundcase == cases)
				hit_found = true;
			else continue;
		}
		ret.push_back(cases);
	}
	return ret;
}

// ASTSwitchCases

ASTSwitchCases::ASTSwitchCases(LocationData const& location)
	: AST(location), isDefault(false), block(NULL)
{}

void ASTSwitchCases::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseSwitchCases(*this, param);
}

// ASTRange

ASTRange::ASTRange(ASTExprConst* start, ASTExprConst* end, LocationData const& location)
	: AST(location), start(start), end(end)
{}

void ASTRange::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseRange(*this, param);
}

// ASTStmtFor

ASTStmtFor::ASTStmtFor(
		ASTStmt* setup, ASTExpr* test, ASTStmt* increment, ASTStmt* body,
		ASTStmt* elseBlock, LocationData const& location)
	: ASTStmt(location), setup(setup), test(test), increment(increment),
	  body(body), elseBlock(elseBlock), scope(nullptr),
	  ends_loop(true), ends_else(true)
{}

void ASTStmtFor::execute(ASTVisitor& visitor, void* param)
{
	return visitor.caseStmtFor(*this, param);
}

// ASTStmtForEach

ASTStmtForEach::ASTStmtForEach(
	std::string const& identifier, ASTExpr* expr, ASTStmt* body,
	ASTStmt* elseBlock, LocationData const& location)
	: ASTStmt(location), iden(identifier), indxdecl(nullptr), arrdecl(nullptr),
		decl(nullptr), arrExpr(expr), body(body),
		elseBlock(elseBlock), scope(nullptr),
	  ends_loop(true), ends_else(true)
{}

void ASTStmtForEach::execute(ASTVisitor& visitor, void* param)
{
	return visitor.caseStmtForEach(*this, param);
}

// ASTStmtWhile

ASTStmtWhile::ASTStmtWhile(ASTExpr* test, ASTStmt* body,
	ASTStmt* elseBlock, LocationData const& location)
	: ASTStmt(location), test(test), body(body),
		elseBlock(elseBlock), inverted(false),
	  ends_loop(true), ends_else(true)
{}

void ASTStmtWhile::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtWhile(*this, param);
}

// ASTStmtDo

ASTStmtDo::ASTStmtDo(ASTExpr* test, ASTStmt* body,
	ASTStmt* elseBlock, LocationData const& location)
	: ASTStmt(location), test(test), body(body),
		elseBlock(elseBlock), inverted(false),
	  ends_loop(true), ends_else(true)
{}

void ASTStmtDo::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtDo(*this, param);
}

// ASTStmtRepeat

ASTStmtRepeat::ASTStmtRepeat(
		ASTExprConst* iter, ASTStmt* body, LocationData const& location)
	: ASTStmt(location), iter(iter), body(body)
{}

void ASTStmtRepeat::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtRepeat(*this, param);
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

ASTStmtBreak::ASTStmtBreak(ASTNumberLiteral* val, LocationData const& location)
	: ASTStmt(location), breakCount(1), count(val)
{}

void ASTStmtBreak::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseStmtBreak(*this, param);
}

// ASTStmtContinue

ASTStmtContinue::ASTStmtContinue(ASTNumberLiteral* val, LocationData const& location)
	: ASTStmt(location), contCount(1), count(val)
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
	: ASTDecl(location), type(NULL), script(NULL)
{
	metadata.autogen();
}

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
	case ASTDecl::TYPE_USING:
		use.push_back(static_cast<ASTUsingDecl*>(&declaration));
		break;
	case ASTDecl::TYPE_ASSERT:
		asserts.push_back(static_cast<ASTAssert*>(&declaration));
		break;
	}
}

// ASTClass

ASTClass::ASTClass(LocationData const& location)
	: ASTDecl(location), name(""), user_class(NULL)
{}

void ASTClass::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseClass(*this, param);
}

void ASTClass::addDeclaration(ASTDecl& declaration)
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
	case ASTDecl::TYPE_USING:
		use.push_back(static_cast<ASTUsingDecl*>(&declaration));
		break;
	case ASTDecl::TYPE_ASSERT:
		asserts.push_back(static_cast<ASTAssert*>(&declaration));
		break;
	}
}

// ASTNamespace

ASTNamespace::ASTNamespace(LocationData const& location, std::string name)
	: ASTDecl(location), name(name), namesp(NULL)
{}

void ASTNamespace::addDeclaration(ASTDecl& declaration)
{
	switch (declaration.getDeclarationType())
	{
	case ASTDecl::TYPE_SCRIPT:
		scripts.push_back(static_cast<ASTScript*>(&declaration));
		break;
	case ASTDecl::TYPE_CLASS:
		classes.push_back(static_cast<ASTClass*>(&declaration));
		break;
	case ASTDecl::TYPE_FUNCTION:
		functions.push_back(static_cast<ASTFuncDecl*>(&declaration));
		break;
	case ASTDecl::TYPE_DATALIST:
		variables.push_back(static_cast<ASTDataDeclList*>(&declaration));
		break;
	case ASTDecl::TYPE_DATATYPE:
		dataTypes.push_back(static_cast<ASTDataTypeDef*>(&declaration));
		break;
	case ASTDecl::TYPE_SCRIPTTYPE:
		scriptTypes.push_back(static_cast<ASTScriptTypeDef*>(&declaration));
		break;
	case ASTDecl::TYPE_NAMESPACE:
		namespaces.push_back(static_cast<ASTNamespace*>(&declaration));
		break;
	case ASTDecl::TYPE_USING:
		use.push_back(static_cast<ASTUsingDecl*>(&declaration));
		break;
	case ASTDecl::TYPE_ASSERT:
		asserts.push_back(static_cast<ASTAssert*>(&declaration));
		break;
	}
}

void ASTNamespace::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseNamespace(*this, param);
}

// ASTImportDecl

ASTImportDecl::ASTImportDecl(
		string const& filename, LocationData const& location, bool isInclude)
	: ASTDecl(location), filename_(filename), checked(false), validated(false),
	include_(isInclude)
{}

void ASTImportDecl::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseImportDecl(*this,param);
}
// ASTIncludePath

ASTIncludePath::ASTIncludePath(
		string const& filename, LocationData const& location)
	: ASTDecl(location), path(filename)
{}

void ASTIncludePath::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseIncludePath(*this,param);
}

// ASTImportCondDecl

ASTImportCondDecl::ASTImportCondDecl(
		ASTExprConst* cond, ASTImportDecl* import, LocationData const& location)
	: ASTDecl(location), cond(cond), import(import), preprocessed(false)
{}

void ASTImportCondDecl::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseImportCondDecl(*this, param);
}

// ASTFuncDecl

ASTFuncDecl::ASTFuncDecl(LocationData const& location)
	: ASTDecl(location), returnType(NULL), iden(NULL), block(NULL), invalidMsg(""), func(NULL), parentScope(NULL), prototype(false),
	  defaultReturn(NULL), flags(0)
{}

void ASTFuncDecl::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseFuncDecl(*this, param);
}

void ASTFuncDecl::setFlag(int32_t flag, bool state)
{
	if(flag&FUNCFLAG_INLINE)
		if(state)
		{
			setFlag(FUNCFLAG_INVALID);
			invalidMsg += " Only internal functions may be inline at this time.";
			return;
		}
	
	if(func) state ? func->flags |= flag : func->flags &= ~flag;
	state ? flags |= flag : flags &= ~flag;
}

bool ASTFuncDecl::isRun() const
{
	return name == FFCore.scriptRunString;
}

// ASTDataDeclList

ASTDataDeclList::ASTDataDeclList(LocationData const& location)
	: ASTDecl(location), baseType(NULL)
{}

ASTDataDeclList::ASTDataDeclList(ASTDataDeclList const& other)
	: ASTDecl(other),
	  baseType(other.baseType)
{
	for (auto it = other.declarations_.cbegin();
	     it != other.declarations_.cend(); ++it)
	{
		ASTDataDecl* decl = (*it)->clone();
		if(decl->baseType)
			decl->baseType.release();
		addDeclaration(decl);
	}
}

ASTDataDeclList& ASTDataDeclList::operator=(ASTDataDeclList const& rhs)
{
	ASTDecl::operator=(rhs);

    baseType = rhs.baseType;
    declarations_.clear();
	for (auto it = rhs.declarations_.cbegin();
	     it != rhs.declarations_.cend(); ++it)
	{
		ASTDataDecl* decl = (*it)->clone();
		if(decl->baseType)
			decl->baseType.release();
		addDeclaration(decl);
	}
	
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

ASTDataEnum::ASTDataEnum(ASTDataEnum const& other)
	: ASTDataDeclList(other)
{}

void ASTDataEnum::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseDataEnum(*this, param);
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
	if(other.initializer_)
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
	if(ASTExprVarInitializer* init = dynamic_cast<ASTExprVarInitializer*>(initializer))
	{
		initializer_ = init;
	}
	else
	{
		initializer_ = new ASTExprVarInitializer(initializer, initializer->location);
	}

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

DataType const& ASTDataDecl::resolveType(ZScript::Scope* scope, CompileErrorHandler* errorHandler)
{
	TypeStore& typeStore = scope->getTypeStore();

	// First resolve the base type.
	ASTDataType* baseTypeNode = list ? list->baseType.get() : baseType.get();
	DataType const* type = &baseTypeNode->resolve(*scope, errorHandler);
	if(baseTypeNode->errorDisabled)
	{
		errorDisabled = true;
		return *type;
	}
	if (!type->isResolved())
		return *type;
	
	// If we have any arrays, tack them onto the base type.
	for (vector<ASTDataDeclExtraArray*>::const_iterator it = extraArrays.begin();
		 it != extraArrays.end(); ++it)
	{
		DataTypeArray arrayType(*type);
		type = typeStore.getCanonicalType(arrayType);
	}

	return *type;
}

DataType const* ASTDataDecl::resolve_ornull(ZScript::Scope* scope, CompileErrorHandler* errorHandler)
{
	DataType const& ty = resolveType(scope, errorHandler);
	return ty.isResolved() ? &ty : nullptr;
}

void ASTDataDecl::replaceType(DataType const& newty)
{
	ASTDataType* baseTypeNode = list ? list->baseType.get() : baseType.get();
	baseTypeNode->replace(newty);
}

bool ZScript::hasSize(ASTDataDecl const& decl)
{
	for (auto it = decl.extraArrays.cbegin(); it != decl.extraArrays.cend(); ++it)
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

std::optional<int32_t> ASTDataDeclExtraArray::getCompileTimeSize(
		CompileErrorHandler* errorHandler, Scope* scope)
		const
{
	if (dimensions.size() == 0) return std::nullopt;
	int32_t size = 1;
	for (auto it = dimensions.cbegin(); it != dimensions.cend(); ++it)
	{
		ASTExpr& expr = **it;
		if (std::optional<int32_t> value = expr.getCompileTimeValue(errorHandler, scope))
			size *= *value / 10000L;
		else
			return std::nullopt;
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

//ASTCustomDataTypeDef

ASTCustomDataTypeDef::ASTCustomDataTypeDef(
		ASTDataType* type, std::string const& name, ASTDataEnum* defn,
			LocationData const& location)
		: ASTDataTypeDef(type, name, location), definition(defn)
{}

void ASTCustomDataTypeDef::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseCustomDataTypeDef(*this, param);
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

// ASTUsingDecl

ASTUsingDecl::ASTUsingDecl(ASTExprIdentifier* iden, LocationData const& location, bool always)
	: ASTDecl(location), always(always), identifier(iden)
{}

void ASTUsingDecl::execute(ASTVisitor& visitor, void* param)
{
	return visitor.caseUsing(*this, param);
}

// ASTAssert

ASTAssert::ASTAssert(ASTExprConst* expr, ASTString* msg, LocationData const& location)
	: ASTDecl(location), expr(expr), msg(msg)
{}

void ASTAssert::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseAssert(*this, param);
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

std::optional<int32_t> ASTExprConst::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	return content ? content->getCompileTimeValue(errorHandler, scope) : std::nullopt;
}

// ASTExprVarInitializer

ASTExprVarInitializer::ASTExprVarInitializer(ASTExpr* content, LocationData const& location)
	: ASTExprConst(content, location), value(std::nullopt)
{}

void ASTExprVarInitializer::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseVarInitializer(*this, param);
}

std::optional<int32_t> ASTExprVarInitializer::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if(scope->isGlobal() || scope->isScript())
		return value;
	else
		return value ? value : content->getCompileTimeValue(errorHandler, scope);
}

bool ASTExprVarInitializer::valueIsArray(Scope* scope, CompileErrorHandler* errorHandler)
{
	DataType const* type = getReadType(scope, errorHandler);
	return type && type->isArray();
}

// ASTExprAssign

ASTExprAssign::ASTExprAssign(ASTExpr* left, ASTExpr* right,
							 LocationData const& location)
	: ASTExpr(location), left(left), right(right) {}

void ASTExprAssign::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprAssign(*this, param);
}

std::optional<int32_t> ASTExprAssign::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	return right ? right->getCompileTimeValue(errorHandler, scope) : std::nullopt;
}

// ASTExprIdentifier

ASTExprIdentifier::ASTExprIdentifier(string const& name,
									 LocationData const& location)
	: ASTExpr(location), binding(NULL), noUsing(false), constant_(false)
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
	vector<string>::const_iterator del = delimiters.begin();
	for (vector<string>::const_iterator it = components.begin() + 1;
	   it != components.end();
	   ++it)
	{
		s = s + *del + *it;
		++del;
	}

	return s;
}

std::optional<int32_t> ASTExprIdentifier::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	return binding ? binding->getCompileTimeValue(scope->isGlobal() || scope->isScript()) : std::nullopt;
}

DataType const* ASTExprIdentifier::getReadType(Scope* scope, CompileErrorHandler* errorHandler)
{
	return binding ? &binding->type : NULL;
}

DataType const* ASTExprIdentifier::getWriteType(Scope* scope, CompileErrorHandler* errorHandler)
{
	return binding ? &binding->type : NULL;
}

// ASTExprArrow

ASTExprArrow::ASTExprArrow(ASTExpr* left, string const& right,
						   LocationData const& location)
	: ASTExpr(location), left(left), right(right), index(NULL),
	  leftClass(NULL), arrayFunction(NULL), readFunction(NULL), writeFunction(NULL),
	  rtype(NULL), wtype(NULL), u_datum(NULL), iscall(false)
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

DataType const* ASTExprArrow::getReadType(Scope* scope, CompileErrorHandler* errorHandler)
{
	if(rtype) return rtype;
	if(arrayFunction) return arrayFunction->returnType;
	return readFunction ? readFunction->returnType : NULL;
}

DataType const* ASTExprArrow::getWriteType(Scope* scope, CompileErrorHandler* errorHandler)
{
	if(wtype) return wtype;
	if(arrayFunction) return arrayFunction->returnType;
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

DataType const* ASTExprIndex::getReadType(Scope* scope, CompileErrorHandler* errorHandler)
{
	DataType const* type = array->getReadType(scope, errorHandler);
	if (type && type->isArray() && !array->isTypeArrow())
	{
		DataTypeArray const* atype = static_cast<DataTypeArray const*>(type);
		type = &atype->getElementType();
	}
	return type;
}

DataType const* ASTExprIndex::getWriteType(Scope* scope, CompileErrorHandler* errorHandler)
{
	DataType const* type = array->getWriteType(scope, errorHandler);
	if (type && type->isArray() && !array->isTypeArrow())
	{
		DataTypeArray const* atype = static_cast<DataTypeArray const*>(type);
		type = &atype->getElementType();
	}
	return type;
}
	
// ASTExprCall

ASTExprCall::ASTExprCall(LocationData const& location)
	: ASTExpr(location), _constructor(false), binding(NULL)
{}

void ASTExprCall::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprCall(*this, param);
}

optional<int32_t> ASTExprCall::getCompileTimeValue(CompileErrorHandler* errorHandler, Scope* scope)
{
	if(!binding)
		return nullopt;
	auto constfunc = binding->get_constexpr();
	if(!constfunc)
		return nullopt;
	vector<optional<int32_t>> param_vals;
	for(auto expr : parameters)
		param_vals.push_back(expr->getCompileTimeValue(errorHandler, scope));
	return constfunc(param_vals);
}
DataType const* ASTExprCall::getReadType(Scope* scope, CompileErrorHandler* errorHandler)
{
	return binding ? binding->returnType : NULL;
}

DataType const* ASTExprCall::getWriteType(Scope* scope, CompileErrorHandler* errorHandler)
{
	return NULL;
}

// ASTUnaryExpr

ASTUnaryExpr::ASTUnaryExpr(LocationData const& location)
	: ASTExpr(location)
{}

// ASTExprDelete

ASTExprDelete::ASTExprDelete(LocationData const& location)
	: ASTUnaryExpr(location)
{}

void ASTExprDelete::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprDelete(*this, param);
}

// ASTExprNegate

ASTExprNegate::ASTExprNegate(LocationData const& location)
	: ASTUnaryExpr(location), done(false)
{}

void ASTExprNegate::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprNegate(*this, param);
}

std::optional<int32_t> ASTExprNegate::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!operand) return std::nullopt;
	if (std::optional<int32_t> value = operand->getCompileTimeValue(errorHandler, scope))
		return done ? *value : -*value;
	return std::nullopt;
}

// ASTExprNot

ASTExprNot::ASTExprNot(LocationData const& location)
	: ASTUnaryExpr(location), inverted(false)
{}

void ASTExprNot::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprNot(*this, param);
}

std::optional<int32_t> ASTExprNot::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!operand) return std::nullopt;
	if (std::optional<int32_t> value = operand->getCompileTimeValue(errorHandler, scope))
		return *value ? 0L : (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L);
	return std::nullopt;
}

// ASTExprBitNot

ASTExprBitNot::ASTExprBitNot(LocationData const& location)
	: ASTUnaryExpr(location)
{}

void ASTExprBitNot::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprBitNot(*this, param);
}

std::optional<int32_t> ASTExprBitNot::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!operand) return std::nullopt;
	if (std::optional<int32_t> value = operand->getCompileTimeValue(errorHandler, scope))
	{
		if(*lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
		   || operand->isLong(scope, errorHandler))
			return ~*value;
		return ~(*value / 10000L) * 10000L;
	}
	return std::nullopt;
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

// ASTExprCast

ASTExprCast::ASTExprCast(ASTDataType* type, ASTExpr* expr, LocationData const& location)
	: ASTUnaryExpr(location), type(type)
{
	operand = expr;
}

void ASTExprCast::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprCast(*this, param);
}

std::optional<int32_t> ASTExprCast::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!operand) return std::nullopt;
	return operand->getCompileTimeValue(errorHandler, scope);
}

DataType const* ASTExprCast::getReadType(Scope* scope, CompileErrorHandler* errorHandler)
{
	return type->resolve_ornull(*scope, errorHandler);
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

std::optional<int32_t> ASTExprAnd::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	bool short_circuit = *lookupOption(*scope, CompileOption::OPT_SHORT_CIRCUIT) != 0;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	if(short_circuit && !*leftValue) return 0L; //Cut it short if we already know the result, and the option is on.
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	return (*leftValue && *rightValue) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0L;
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

std::optional<int32_t> ASTExprOr::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	bool short_circuit = *lookupOption(*scope, CompileOption::OPT_SHORT_CIRCUIT) != 0;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	if(short_circuit && *leftValue) return 10000L; //Cut it int16_t if we already know the result, and the option is on.
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	return (*leftValue || *rightValue) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0L;
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

std::optional<int32_t> ASTExprGT::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	return (*leftValue > *rightValue) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0L;
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

std::optional<int32_t> ASTExprGE::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	return (*leftValue >= *rightValue) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0L;
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

std::optional<int32_t> ASTExprLT::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	return (*leftValue < *rightValue) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0L;
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

std::optional<int32_t> ASTExprLE::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	return (*leftValue <= *rightValue) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0L;
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

std::optional<int32_t> ASTExprEQ::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	return (*leftValue == *rightValue) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0L;
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

std::optional<int32_t> ASTExprNE::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	return (*leftValue != *rightValue) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0L;
}

// ASTExprAppxEQ

ASTExprAppxEQ::ASTExprAppxEQ(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

void ASTExprAppxEQ::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprAppxEQ(*this, param);
}

std::optional<int32_t> ASTExprAppxEQ::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	return (abs(*leftValue - *rightValue) <= (*lookupOption(*scope, CompileOption::OPT_APPROX_EQUAL_MARGIN))) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0L;
}

// ASTExprXOR

ASTExprXOR::ASTExprXOR(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTRelExpr(left, right, location)
{}

void ASTExprXOR::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprXOR(*this, param);
}

std::optional<int32_t> ASTExprXOR::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	return ((!*leftValue) != (!*rightValue)) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0L;
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

std::optional<int32_t> ASTExprPlus::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
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

std::optional<int32_t> ASTExprMinus::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
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

std::optional<int32_t> ASTExprTimes::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if(rightValue && (*rightValue == 0)) return 0;
	if(leftValue && (*leftValue == 0)) return 0;
	if (!leftValue) return std::nullopt;
	if (!rightValue) return std::nullopt;

	return int32_t(*leftValue * (*rightValue / 10000.0));
}

// ASTExprExpn

ASTExprExpn::ASTExprExpn(
		ASTExpr* left, ASTExpr* right, LocationData const& location)
	: ASTMultExpr(left, right, location)
{}

void ASTExprExpn::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseExprExpn(*this, param);
}

std::optional<int32_t> ASTExprExpn::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	bool is_long = left->isLong(scope, errorHandler) || right->isLong(scope, errorHandler);
	
	if(rightValue && (*rightValue == 0)) return 10000; //x^0 == 1
	if(leftValue) //1^x == 1
	{
		if(is_long)
		{
			if(*leftValue == 1) return 1;
		}
		else if(*leftValue == 10000) return 10000;
	}
	
	if (!leftValue) return std::nullopt;
	if (!rightValue) return std::nullopt;
	if(*leftValue == 0) return 0;
	if(is_long)
		return int32_t(pow(*leftValue,*rightValue));
	else
	{
		double temp_l = *leftValue / 10000.0;
		double temp_r = *rightValue / 10000.0;
		return int32_t(pow(temp_l, temp_r) * 10000.0);
	}
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

std::optional<int32_t> ASTExprDivide::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;

	if (*rightValue == 0)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::DivByZero(this, "divide"));
		return (*leftValue >= 0) ? 2147483647 : -2147483647; //error value
	}
	
	if(*lookupOption(*scope, CompileOption::OPT_TRUNCATE_DIVISION_BY_LITERAL_BUG)
		&& left->isLiteral() && right->isLiteral())
	{
		return *leftValue / *rightValue * 10000L;
	}
	
	return static_cast<int32_t>((*leftValue * 1.0) / (*rightValue * 1.0) * (10000L));
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

std::optional<int32_t> ASTExprModulo::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;

	if (*rightValue == 0)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::DivByZero(this,"modulo"));
		return std::nullopt;
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

std::optional<int32_t> ASTExprBitAnd::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	if(*lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
	   || left->isLong(scope, errorHandler)
	   || right->isLong(scope, errorHandler))
		return *leftValue & *rightValue;
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

std::optional<int32_t> ASTExprBitOr::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;

	if(*lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
	   || left->isLong(scope, errorHandler)
	   || right->isLong(scope, errorHandler))
		return *leftValue | *rightValue;
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

std::optional<int32_t> ASTExprBitXor::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;

	if(*lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
	   || left->isLong(scope, errorHandler)
	   || right->isLong(scope, errorHandler))
		return *leftValue ^ *rightValue;
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

std::optional<int32_t> ASTExprLShift::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	
	if (*rightValue % 10000L)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::ShiftNotInt(this));
		rightValue = (*rightValue / 10000L) * 10000L;
	}

	if(*lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
	   || left->isLong(scope, errorHandler)
	   || right->isLong(scope, errorHandler))
		return *leftValue << (*rightValue / 10000L);
	
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

std::optional<int32_t> ASTExprRShift::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	if (!rightValue) return std::nullopt;
	
	if (*rightValue % 10000L)
	{
		if (errorHandler)
			errorHandler->handleError(CompileError::ShiftNotInt(this));
		rightValue = (*rightValue / 10000L) * 10000L;
	}

	if(*lookupOption(*scope, CompileOption::OPT_BINARY_32BIT)
	   || left->isLong(scope, errorHandler)
	   || right->isLong(scope, errorHandler))
		return *leftValue >> (*rightValue / 10000L);
	
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

std::optional<int32_t> ASTTernaryExpr::getCompileTimeValue(
		CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!left || !middle || !right) return std::nullopt;
	std::optional<int32_t> leftValue = left->getCompileTimeValue(errorHandler, scope);
	if (!leftValue) return std::nullopt;
	std::optional<int32_t> middleValue = middle->getCompileTimeValue(errorHandler, scope);
	std::optional<int32_t> rightValue = right->getCompileTimeValue(errorHandler, scope);
	
	if(*leftValue)
	{
		if(!middleValue) return std::nullopt;
		return *middleValue;
	}
	else
	{
		if (!rightValue) return std::nullopt;
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

std::optional<int32_t> ASTNumberLiteral::getCompileTimeValue(
	CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!value) return std::nullopt;
    pair<int32_t, bool> val = ScriptParser::parseLong(value->parseValue(scope), scope);
	
    if (!val.second && errorHandler)
	    errorHandler->handleError(
			    CompileError::ConstTrunc(this, value->value.c_str()));

	return val.first;
}

void ASTNumberLiteral::negate()
{
	if(value) value.get()->negative = !(value.get()->negative);
}

// ASTLongNumberLiteral

ASTLongNumberLiteral::ASTLongNumberLiteral(
		ASTFloat* value, LocationData const& location)
	: ASTNumberLiteral(value, location)
{}


// ASTCharLiteral

ASTCharLiteral::ASTCharLiteral(
		ASTFloat* value, LocationData const& location)
	: ASTLiteral(location), value(value)
{}

void ASTCharLiteral::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseCharLiteral(*this, param);
}

std::optional<int32_t> ASTCharLiteral::getCompileTimeValue(
	CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!value) return std::nullopt;
    pair<int32_t, bool> val = ScriptParser::parseLong(value->parseValue(scope), scope);

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
	: ASTLiteral(location), declaration(NULL), value(str)
{}

ASTStringLiteral::ASTStringLiteral(
		string const& str, LocationData const& location)
	: ASTLiteral(location), declaration(NULL), value(str)
{}

ASTStringLiteral::ASTStringLiteral(ASTString const& raw)
	: ASTLiteral(raw.location),
	  declaration(NULL),
	  value(raw.getValue())
{}

ASTStringLiteral::ASTStringLiteral(ASTStringLiteral const& base)
	: ASTLiteral(base), declaration(NULL),
	  // declaration field is managed by the declaration itself, so it stays
	  // NULL regardless.
	  value(base.value)
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

DataTypeArray const* ASTStringLiteral::getReadType(Scope* scope, CompileErrorHandler* errorHandler)
{
	return &DataType::STRING;
}


// ASTArrayLiteral

ASTArrayLiteral::ASTArrayLiteral(LocationData const& location)
	: ASTLiteral(location), declaration(NULL), type(NULL), size(NULL),
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
	  value(std::nullopt)
{}

void ASTOptionValue::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseOptionValue(*this, param);
}

std::optional<int32_t> ASTOptionValue::getCompileTimeValue(
	CompileErrorHandler* errorHandler, Scope* scope)
{
	if (!scope) return std::nullopt;
	if (std::optional<int32_t> value = lookupOption(*scope, option))
		return value;
	errorHandler->handleError(CompileError::UnknownOption(this, name));
	return std::nullopt;
}

std::string ASTOptionValue::asString() const
{
	return "OPTION_VALUE(" + *option.getName() + ")";
}

// ASTIsIncluded

ASTIsIncluded::ASTIsIncluded(
		string const& str, LocationData const& location)
	: ASTLiteral(location)
{
	name = cropPath(str);
	lowerstr(name);
}

void ASTIsIncluded::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseIsIncluded(*this, param);
}

std::optional<int32_t> ASTIsIncluded::getCompileTimeValue(
	CompileErrorHandler* errorHandler, Scope* scope)
{
	RootScope* root = getRoot(*scope);
	return root->isImported(name) ? (*lookupOption(*scope, CompileOption::OPT_BOOL_TRUE_RETURN_DECIMAL) ? 1L : 10000L) : 0;
}

////////////////////////////////////////////////////////////////
// Types

// ASTScriptType

ASTScriptType::ASTScriptType(ParserScriptType type, LocationData const& location)
		: AST(location), type(type)
{}

ASTScriptType::ASTScriptType(string const& name, LocationData const& location)
		: AST(location), name(name)
{}

void ASTScriptType::execute(ASTVisitor& visitor, void* param)
{
	visitor.caseScriptType(*this, param);
}

ParserScriptType ZScript::resolveScriptType(ASTScriptType const& node,
                                      Scope const& scope)
{
	if (node.type.isValid()) return node.type;
	return lookupScriptType(scope, node.name);
}

// ASTDataType

ASTDataType::ASTDataType(DataType* type, LocationData const& location)
	: AST(location), type(type->clone()), constant_(0), becomeArray(false),
	wasResolved_(false)
{}

ASTDataType::ASTDataType(DataType const& type, LocationData const& location)
	: AST(location), type(type.clone()), constant_(0), becomeArray(false),
	wasResolved_(false)
{}

void ASTDataType::execute(ASTVisitor& visitor, void* param)
{	
	visitor.caseDataType(*this, param);
}

DataType const& ASTDataType::resolve(ZScript::Scope& scope, CompileErrorHandler* errorHandler)
{
	if(!wasResolved_)
	{
		DataType const* resolved = type->resolve(scope, errorHandler);
		if(resolved && constant_)
		{
			string name = resolved->getName();
			if(constant_>1 || resolved->isConstant())
			{
				errorHandler->handleError(CompileError::ConstAlreadyConstant(this, name));
				return DataType::ZVOID;
			}
			resolved = resolved->getConstType();
		}
		if(resolved)
		{
			DataType* result = nullptr;
			if(becomeArray)
			{
				auto* basety = resolved->baseType(scope, errorHandler);
				result = new DataTypeArray(*basety);
			}
			else result = resolved->clone();
			type.reset(result);
			wasResolved_ = result && result->isResolved();
		}
	}
	return *type;
}
DataType const* ASTDataType::resolve_ornull(ZScript::Scope& scope, CompileErrorHandler* errorHandler)
{
	DataType const& ty = resolve(scope, errorHandler);
	return ty.isResolved() ? &ty : nullptr;
}
void ASTDataType::replace(DataType const& newty)
{
	wasResolved_ = false;
	constant_ = false;
	becomeArray = false;
	type.reset(newty.clone());
}


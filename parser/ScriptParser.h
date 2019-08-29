#ifndef SCRIPTPARSER_H
#define SCRIPTPARSER_H

class AST;
// for flex and bison
#define YYSTYPE AST*

#include "y.tab.hpp"

#include <vector>

using namespace std;

class ASTScript;
class ASTImport;

//////////////////////////////////////////////////////////////////////////////
class AST
{
};

class ASTProgram : public AST
{
public:
private:
	vector<ASTScript *> scripts;
	vector<ASTImport *> imports;
};

class ASTFloat : public AST
{
public:
	ASTFloat(float val) : val(val) {}
	float getValue() {return val;}
private:
	float val;
};

class ASTString : public AST
{
public:
	ASTString(const char *strval)
	{
		str = string(strval);
	}
	string getValue() {return str;}
private:
	string str;
};

#endif

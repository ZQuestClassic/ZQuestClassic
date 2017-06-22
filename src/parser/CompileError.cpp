#include "../precompiled.h" //always first

#include "CompileError.h"
#include "../zsyssimple.h"
#include <assert.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

// Weird macros to insert constants into strings at preprocessing stage.
#define STRING1(s) #s
#define STRING(s) STRING1(s)

CompileError::CompileError(int id, char code, bool warning, string const& format)
	: id(id), code(code), warning(warning), format(format)
{}

void CompileError::print(AST* offender, ...) const
{
	va_list args;
	va_start(args, offender);
	vprint(offender, args);
	va_end(args);
}

void CompileError::vprint(AST* offender, va_list args) const
{
    ostringstream oss;

	// Output location data.
    if (offender)
    {
        LocationData const& ld = offender->getLocation();
        oss << ld.fname << ", line " << ld.first_line << ": ";
    }

	// Error or warning?
	if (warning) oss << "Warning";
	else oss << "Error";

	// Error Code and Id
	char idstr[8];
	sprintf(idstr, "%03d", id);
	oss << " " << code << idstr << ": ";

	// Message.
	char msg[1024];
	vsprintf(msg, format.c_str(), args);
	oss << msg;
	
#ifndef SCRIPTPARSER_COMPILE
    box_out(oss.str().c_str());
    box_eol();
#endif
}
	
////////////////////////////////////////////////////////////////
// Error Definitions

CompileError const CompileError::CantOpenSource(
		0, 'P', false, "Can't open or parse input file!");
CompileError const CompileError::CantOpenImport(
		1, 'P', false, "Failure to parse imported file %s.");
CompileError const CompileError::ImportRecursion(
		2, 'P', false,
		"Recursion limit of " STRING(RECURSIONLIMIT)
		" hit while preprocessing. Perhaps you have circular imports?");
// DEPRECATED
CompileError const CompileError::ImportBadScope(
		3, 'P', false,
		"You may only place import statements at file scope.");
CompileError const CompileError::FunctionRedef(
		4, 'S', false,
		"Function %s was already declared with that type signature.");
CompileError const CompileError::FunctionVoidParam(
		5, 'S', false, "Function parameter %s cannot have void type.");
CompileError const CompileError::ScriptRedef(
		6, 'S', false, "Duplicate script with name %s already exists.");
CompileError const CompileError::VoidVar(
		7, 'S', false, "Variable %s can't have type void.");
CompileError const CompileError::VarRedef(
		8, 'S', false,
		"There is already a variable with name %s in this scope.");
CompileError const CompileError::VarUndeclared(
		9, 'S', false, "Variable %s is undeclared.");
CompileError const CompileError::FuncUndeclared(
		10, 'S', false, "Function %s is undeclared.");
CompileError const CompileError::ScriptNoRun(
		11, 'S', false, "Script %s must implement void run().");
CompileError const CompileError::ScriptRunNotVoid(
		12, 'S', false, "Script %s's run() must have return type void.");
// DEPRECATED
CompileError const CompileError::ScriptNumNotInt(
		13, 'T', false, "Script %s has id that's not an integer.");
// DEPRECATED
CompileError const CompileError::ScriptNumTooBig(
		14, 'T', false, "Script %s's id must be between 0 and 255.");
// DEPRECATED
CompileError const CompileError::ScriptNumRedef(
		15, 'T', false, "Script %s's id is already in use."); 
CompileError const CompileError::ImplictCast(
		16, 'T', true, "Cast from %s.");
CompileError const CompileError::IllegalCast(
		17, 'T', false, "Cannot cast from %s.");
// DEPRECATED
CompileError const CompileError::VoidExpr(
		18, 'T', false, "Operand is void.");
CompileError const CompileError::DivByZero(
		19, 'T', false, "Constant division by zero.");
CompileError const CompileError::ConstTrunc(
		20, 'T', true, "Truncation of constant %s.");
CompileError const CompileError::NoFuncMatch(
		21, 'T', false, "Could not match type signature %s.");
CompileError const CompileError::TooFuncMatch(
		22, 'T', false, "Two or more functions match type signature %s.");
CompileError const CompileError::FuncBadReturn(
		23, 'T', false, "This function must return a value.");
CompileError const CompileError::TooManyGlobal(
		24, 'L', false, "Too many global variables.");
CompileError const CompileError::ShiftNotInt(
		25, 'T', true,
		"Constant bitshift by noninteger amount;"
		" truncating to nearest integer.");
CompileError const CompileError::RefVar(
		26, 'S', false, "Variable type cannot be global [%s].");
CompileError const CompileError::ArrowNotPointer(
		27, 'T', false,
		"Left of the arrow (->) operator must be a pointer type (ffc, etc).");
CompileError const CompileError::ArrowNoFunc(
		28, 'T', false, "That pointer type does not have a function %s.");
CompileError const CompileError::ArrowNoVar(
		29, 'T', false, "That pointer type does not have a variable %s.");
CompileError const CompileError::TooManyRun(
		30, 'S', false, "Script %s may have only one run method.");
// DEPRECATED
CompileError const CompileError::IndexNotInt(
		31, 'T', false, "The index of %s must be an integer.");
CompileError const CompileError::ScriptBadType(
		32, 'S', false, "Script %s is of illegal type.");
CompileError const CompileError::BreakBad(
		33, 'G', false,
		"Break must lie inside of an enclosing for or while loop.");
CompileError const CompileError::ContinueBad(
		34, 'G', false,
		"Continue must lie inside of an enclosing for or while loop.");
CompileError const CompileError::ConstRedef(
		35, 'P', false, "There is already a constant with name %s defined.");
CompileError const CompileError::LValConst(
		36, 'T', false, "Cannot change the value of constant variable %s.");
CompileError const CompileError::BadGlobalInit(
		37, 'T', false,
		"Global variables can only be initialized to constants or"
		" globals declared in the same script.");
CompileError const CompileError::DeprecatedGlobal(
		38, 'S', true,
		"Script-scope global variable declaration syntax is deprecated;"
		" put declarations at file scope instead.");
CompileError const CompileError::VoidArr(
		39, 'S', false, "Array %s can't have type void.");
CompileError const CompileError::RefArr(
		40, 'S', false, "Array type cannot be global [%s].");
CompileError const CompileError::ArrRedef(
		41, 'S', false,
		"There is already an array with name%s defined in this scope.");
CompileError const CompileError::ArrayTooSmall(
		42, 'A', false, "Array is too small.");
CompileError const CompileError::ArrayListTooLarge(
		43, 'A', false, "Array initializer larger than specified dimensions.");
CompileError const CompileError::ArrayListStringTooLarge(
		44, 'A', false,
		"String array initializer larger than specified dimensions,"
		" space must be allocated for NULL terminator.");
CompileError const CompileError::NonIntegerArraySize(
		45, 'T', false, "Arrays can only be initialized to numerical values.");
CompileError const CompileError::ExprNotConstant(
		46, 'T', false, "Expression not constant.");
CompileError const CompileError::UnresolvedType(
		47, 'T', false, "Type '%s' is unknown.");
CompileError const CompileError::ConstUninitialized(
		48, 'T', false, "Const types must be initialized.");
CompileError const CompileError::ConstAssign(
		49, 'T', false, "You cannot assign to a constant value.");
CompileError const CompileError::EmptyArrayLiteral(
		50, 'S', false, "Array Literals must not be empty.");
CompileError const CompileError::DimensionMismatch(
		51, 'B', false, "Array Size Mismatch.");
CompileError const CompileError::ArrayLiteralResize(
		52, 'S', false, "Re-specifying array size.");
CompileError const CompileError::MissingCompileError(
		53, 'C', true, "Expected error %d did not occur.");



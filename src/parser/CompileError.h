#ifndef COMPILE_ERROR_H
#define COMPILE_ERROR_H

#include "AST.h"
#include <cstdarg>
#include <string>

using std::string;

class CompileError
{
public:
	CompileError(int id, char code, bool warning, string const& format);

	// An internal id number for the error.
	int id;

	// A single letter code classifying the error type.
	char code;

	// True if this is only a warning.
	bool warning;

	// The format string for printing out the error message.
	string format;

	// Print out the error message. Optional format arguments.
	void print(AST* offender, ...) const;
	// Print by directly passing varargs list.
	void vprint(AST* offender, std::va_list args) const;

	// Comparison on id.
	bool operator==(CompileError const& rhs) const {return id == rhs.id;}

	// Defined errors.
	static CompileError const CantOpenSource;
	static CompileError const CantOpenImport;
	static CompileError const ImportRecursion;
	static CompileError const ImportBadScope; // DEPRECATED
	static CompileError const FunctionRedef;
	static CompileError const FunctionVoidParam;
	static CompileError const ScriptRedef;
	static CompileError const VoidVar;
	static CompileError const VarRedef;
	static CompileError const VarUndeclared;
	static CompileError const FuncUndeclared;
	static CompileError const ScriptNoRun;
	static CompileError const ScriptRunNotVoid;
	static CompileError const ScriptNumNotInt; // DEPRECATED
	static CompileError const ScriptNumTooBig; // DEPRECATED
	static CompileError const ScriptNumRedef; // DEPRECATED
	static CompileError const ImplictCast;
	static CompileError const IllegalCast;
	static CompileError const VoidExpr; // DEPRECATED
	static CompileError const DivByZero;
	static CompileError const ConstTrunc;
	static CompileError const NoFuncMatch;
	static CompileError const TooFuncMatch;
	static CompileError const FuncBadReturn;
	static CompileError const TooManyGlobal;
	static CompileError const ShiftNotInt;
	static CompileError const RefVar;
	static CompileError const ArrowNotPointer;
	static CompileError const ArrowNoFunc;
	static CompileError const ArrowNoVar;
	static CompileError const TooManyRun;
	static CompileError const IndexNotInt; // DEPRECATED
	static CompileError const ScriptBadType;
	static CompileError const BreakBad;
	static CompileError const ContinueBad;
	static CompileError const ConstRedef;
	static CompileError const LValConst;
	static CompileError const BadGlobalInit;
	static CompileError const DeprecatedGlobal;
	static CompileError const VoidArr;
	static CompileError const RefArr;
	static CompileError const ArrRedef;
	static CompileError const ArrayTooSmall;
	static CompileError const ArrayListTooLarge;
	static CompileError const ArrayListStringTooLarge;
	static CompileError const NonIntegerArraySize;
	static CompileError const ExprNotConstant;
	static CompileError const UnresolvedType;
	static CompileError const ConstUninitialized;
	static CompileError const ConstAssign;
	static CompileError const EmptyArrayLiteral;
	static CompileError const DimensionMismatch;
	static CompileError const ArrayLiteralResize;
	static CompileError const MissingCompileError;
};

#endif

#ifndef SCRIPTING_ZASM_VARIABLES_H
#define SCRIPTING_ZASM_VARIABLES_H
#include "ZAsmVariableTable.h"

#include <string>

namespace ZAsm
{
	enum VariableId
	{
		// Create variable id enum, eg. VarId_DATA
#		define X(START, COUNT, NAME) VarId_##NAME = START,
		ZASM_VARIABLE_TABLE
#		undef X
		VariableCount
	};

	////////////////////////////////////////////////////////////////
	// Variable Definitions
	
	class VariableDef
	{
	public:
		VariableDef(VariableId firstId, int count, std::string const& name);

		VariableId getFirstId() const {return firstId_;}
		VariableId getLastId() const {
			return VariableId(firstId_ + count_ - 1);}
		int getCount() const {return count_;}
		std::string const& getName() const {return name_;}
		
	private:
		VariableId firstId_;
		int count_;
		std::string name_;
	};

	// Lookup variable definition.
	VariableDef const* getVariableDef(VariableId id);
	VariableDef const* getVariableDef(std::string const& name);
	
	// Declare VariableDef constants. Example:
	// static VariableDef const VarDef_DATA(0x000A, 1, "DATA")
#	define X(START, COUNT, NAME) \
	VariableDef const VarDef_##NAME(VariableId(START), COUNT, #NAME);
	ZASM_VARIABLE_TABLE
#	undef X

	////////////////////////////////////////////////////////////////
	// Variables

	class Variable
	{
	public:
		static Variable Null;
		
		Variable(VariableDef const* definition, int index = 0);

		bool isNull() const {return definition_ == NULL;}
		VariableId getId() const;
		std::string toString() const;
		
	private:
		VariableDef const* definition_;
		int index_;

		Variable() : definition_(NULL), index_(-1) {}
	};

	Variable getVariable(VariableId id);
	Variable getVariable(std::string const& name);

	// TODO Rewrite to grab from the table instead.
	Variable varD(int index);
#	define VAR_D(INDEX) (INDEX)
	Variable varA(int index);
#	define VAR_A(INDEX) (INDEX + 8)
	Variable varGD(int index);
#	define VAR_GD(INDEX) (INDEX + 0x2F5)

}

#undef ZASM_VARIABLE_TABLE
#endif

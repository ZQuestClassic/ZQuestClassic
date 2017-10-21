#ifndef SCRIPTING_ZASM_VARIABLES_H
#define SCRIPTING_ZASM_VARIABLES_H
#include "ZAsmVariableTable.h"

#include <string>

namespace ZAsm
{
	enum VariableId
	{
		// Create variable id enum, eg. VarId_DATA
#		define VARIABLE(START, COUNT, NAME) VarId_##NAME = START,
		ZASM_VARIABLE_TABLE
#		undef VARIABLE
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
#	define VARIABLE(START, COUNT, NAME) \
	VariableDef const VarDef_##NAME(VariableId(START), COUNT, #NAME);
	ZASM_VARIABLE_TABLE
#	undef VARIABLE

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

	inline bool operator==(Variable const& lhs, Variable const& rhs) {
		return lhs.getId() == rhs.getId();}
	inline bool operator!=(Variable const& lhs, Variable const& rhs) {
		return !operator==(lhs, rhs);}
	inline bool operator< (Variable const& lhs, Variable const& rhs) {
		return lhs.getId() < rhs.getId();}
	inline bool operator> (Variable const& lhs, Variable const& rhs) {
		return operator<(rhs, lhs);}
	inline bool operator<=(Variable const& lhs, Variable const& rhs) {
		return !operator>(lhs, rhs);}
	inline bool operator>=(Variable const& lhs, Variable const& rhs) {
		return !operator<(lhs, rhs);}
	
	// TODO Rewrite to grab from the table instead.
#	define VAR_D(INDEX) (INDEX)
#	define VAR_A(INDEX) (INDEX + 8)
#	define VAR_GD(INDEX) (INDEX + 0x2F5)

	// Declare Variable constructors. Example:
	// Variable varD(int index = 0);
#	define VARIABLE(START, COUNT, NAME)\
	Variable var##NAME(int index = 0);
	ZASM_VARIABLE_TABLE
#	undef VARIABLE
}

#undef ZASM_VARIABLE_TABLE
#endif

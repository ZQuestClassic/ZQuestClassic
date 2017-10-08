#include "../precompiled.h"
#include "LibraryHelper.h"

#include <cassert>
#include "CompilerUtils.h"
#include "Scope.h"
#include "Types.h"

//using namespace std;
using std::string;

using namespace ZScript;

LibraryHelper::LibraryHelper(
		Scope& scope, ZAsm::Variable const& objectVariable,
		optional<DataType> objectType)
	: scope_(scope), objectVariable_(objectVariable),
	  objectType_(objectType)
{}

LibraryHelper::call_tag const LibraryHelper::asVariable;
LibraryHelper::call_tag const LibraryHelper::asFunction;

typedef VectorBuilder<DataType> P;

Function& LibraryHelper::addFunction(
		DataType const& returnType,
		string const& name,
		vector<DataType> parameterTypes)
{
	if (objectType_)
		parameterTypes.insert(parameterTypes.begin(), *objectType_);
	return *scope_.addFunction(returnType, name, parameterTypes);
}

void LibraryHelper::addGetter(
		ZAsm::Variable const& variable,
		DataType const& type, string const& name,
		call_tag const& call)
{
	////////////////
	// Setup types.
	vector<DataType> paramTypes;
	if (objectType_) paramTypes.push_back(*objectType_);

	////////////////
	// Generate function code.
    vector<Opcode> code;
    // Pop argument (object pointer).
    code.push_back(opPOP(varExp2()));
    // Set used object register to pointer.
    if (objectVariable_ != varNull())
	    code.push_back(opSETR(objectVariable_, varExp2()));
    // Store variable value into EXP1.
    code.push_back(opSETR(varExp1(), variable));
    // Return from function.
    appendReturn(code);

    ////////////////
	// Create the function.
    Function* function;
    if (&call == &asVariable)
	    function = scope_.addGetter(type, name, paramTypes);
    else if (&call == &asFunction)
	    function = scope_.addFunction(type, name, paramTypes);
	code.front().withLabel(function->getLabel());
    function->setCode(code);
}

void LibraryHelper::addGetter(
		ZAsm::Variable const& variable,
		DataType const& type, string const& name,
		int arraySize)
{
	////////////////
	// Setup types.
	TypeStore& typeStore = scope_.getTypeStore();
	vector<DataType> paramTypes;
	if (objectType_) paramTypes.push_back(*objectType_);
	paramTypes.push_back(typeStore.getFloat());

	////////////////
	// Generate function code.
    vector<Opcode> code;
    // Pop argument (index).
    code.push_back(opPOP(varIndex1()));
    // Pop argument (object pointer).
    code.push_back(opPOP(varExp2()));
    // Set used object register to pointer.
    if (objectVariable_ != varNull())
        code.push_back(opSETR(objectVariable_, varExp2()));
    // Store variable value into EXP1.
    code.push_back(opSETR(varExp1(), variable));
    // Return from function.
    appendReturn(code);

    ////////////////
	// Create the function.
	Function& function = *scope_.addGetter(type, name, paramTypes);
	code.front().withLabel(function.getLabel());
    function.setCode(code);
}

void LibraryHelper::addSetter(
		ZAsm::Variable const& variable,
		DataType const& type, string const& name)
{
	////////////////
	// Setup types.
	TypeStore& typeStore = scope_.getTypeStore();
	vector<DataType> paramTypes;
	if (objectType_) paramTypes.push_back(*objectType_);
	paramTypes.push_back(type);

	////////////////
	// Generate function code.
    vector<Opcode> code;
    // Pop argument (new value).
    code.push_back(opPOP(varExp1()));
    // If the argument is a bool, normalize true to 1.
    optional<int> skipLabel;
    if (type == typeStore.getBool())
    {
	    skipLabel = ScriptParser::getUniqueLabelID();
	    code.push_back(opCOMPAREV(varExp1(), 0));
	    code.push_back(opGOTOTRUE(*skipLabel));
	    code.push_back(opSETV(varExp1(), 10000));
    }
    // Pop argument (object pointer).
    code.push_back(opPOP(varExp2()));
    if (skipLabel) code.back().withLabel(*skipLabel);
    // Set used object register to pointer.
    if (objectVariable_ != varNull())
        code.push_back(opSETR(objectVariable_, varExp2()));
    // Store variable value into EXP1.
    code.push_back(opSETR(variable, varExp1()));
    // Return from function.
    appendReturn(code);

    ////////////////
	// Create the function.
	Function& function = *scope_.addSetter(type, name, paramTypes);
	code.front().withLabel(function.getLabel());
    function.setCode(code);
}

void LibraryHelper::addSetter(
		ZAsm::Variable const& variable,
		DataType const& type, string const& name,
		int arraySize)
{
	////////////////
	// Setup types.
	TypeStore& typeStore = scope_.getTypeStore();
	vector<DataType> paramTypes;
	if (objectType_) paramTypes.push_back(*objectType_);
	paramTypes.push_back(typeStore.getFloat());
	paramTypes.push_back(type);

	////////////////
	// Generate function code.
    vector<Opcode> code;
    // Pop argument (index).
    code.push_back(opPOP(varIndex1()));
    // Pop argument (new value).
    code.push_back(opPOP(varExp1()));
    // Pop argument (object pointer).
    code.push_back(opPOP(varExp2()));
    // Set used object register to pointer.
    if (objectVariable_ != varNull())
        code.push_back(opSETR(objectVariable_, varExp2()));
    // Store variable value into EXP1.
    code.push_back(opSETR(variable, varExp1()));
    // Return from function.
    appendReturn(code);

    ////////////////
	// Create the function.
	Function& function = *scope_.addSetter(type, name, paramTypes);
	code.front().withLabel(function.getLabel());
    function.setCode(code);
}

void ZScript::appendReturn(vector<Opcode>& code, optional<int> label)
{
	code.push_back(opPOP(varExp2()));
	if (label) code.back().withLabel(*label);
	code.push_back(opGOTOR(varExp2()));
}

void ZScript::addPair(
		LibraryHelper& lh, ZAsm::Variable const& variable,
		DataType const& type, string const& name)
{
	lh.addGetter(variable, type, name);
	lh.addSetter(variable, type, name);
}

void ZScript::addPair(
		LibraryHelper& lh, ZAsm::Variable const& variable,
		DataType const& type, string const& name, int arraySize)
{
	lh.addGetter(variable, type, name, arraySize);
	lh.addSetter(variable, type, name, arraySize);
}

void ZScript::defineFunction(
		LibraryHelper& lh,
		DataType const& returnType,
		string const& name,
		vector<DataType> const& parameterTypes,
		vector<ZAsm::Variable> const& parameterVariables,
		vector<Opcode> const& opcodes)
{
	Function& function = lh.addFunction(returnType, name, parameterTypes);
	int parameterCount = function.paramTypes.size();
	assert(parameterVariables.size() <= parameterCount);
	
	vector<Opcode> code;
	// Pop the arguments into the supplied registers.
	for (vector<ZAsm::Variable>::const_reverse_iterator it =
		     parameterVariables.rbegin();
	     it != parameterVariables.rend(); ++it)
		code.push_back(opPOP(*it));
	// Call the passed opcode.
	appendElements(code, opcodes);
	// Pop the remaining arguments.
	for (int i = parameterVariables.size(); i < parameterCount; ++i)
		code.push_back(opPOP(varNull()));
	// Return from the function.
	appendReturn(code);

	code.front().withLabel(function.getLabel());
	function.setCode(code);
}

void ZScript::defineFunction(
		LibraryHelper& lh,
		DataType const& returnType,
		std::string const& name,
		std::vector<DataType> const& parameterTypes,
		std::vector<ZAsm::Variable> const& parameterVariables,
		Opcode const& opcode)
{
	defineFunction(
			lh, returnType, name, parameterTypes, parameterVariables,
			VectorBuilder<Opcode>() << opcode);
}

void ZScript::defineFunction(
		LibraryHelper& lh,
		DataType const& returnType,
		std::string const& name,
		std::vector<DataType> const& parameterTypes,
		std::vector<Opcode> const& opcodes)
{
	defineFunction(
			lh, returnType, name, parameterTypes, vector<ZAsm::Variable>(),
			opcodes);
}

void ZScript::defineFunction(
		LibraryHelper& lh,
		DataType const& returnType,
		std::string const& name,
		std::vector<DataType> const& parameterTypes,
		Opcode const& opcode)
{
	defineFunction(
			lh, returnType, name, parameterTypes, vector<ZAsm::Variable>(),
			VectorBuilder<Opcode>() << opcode);
}

void ZScript::defineFunction(
		LibraryHelper& lh, DataType const& returnType,
		std::string const& name, Opcode const& opcode)
{
	defineFunction(
			lh, returnType,
			name, vector<DataType>(), vector<ZAsm::Variable>(),
			VectorBuilder<Opcode>() << opcode);
}

void ZScript::defineFunction(
		LibraryHelper& lh, std::string const& name, Opcode const& opcode)
{
	defineFunction(
			lh, lh.getScope().getTypeStore().getVoid(),
			name, vector<DataType>(), vector<ZAsm::Variable>(),
			VectorBuilder<Opcode>() << opcode);
}


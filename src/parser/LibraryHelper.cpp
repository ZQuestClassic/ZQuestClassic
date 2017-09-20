#include "../precompiled.h"
#include "LibraryHelper.h"

#include <cassert>
#include "ByteCode.h"
#include "CompilerUtils.h"
#include "Scope.h"
#include "Types.h"

//using namespace std;
using std::string;

using namespace ZScript;

LibraryHelper::LibraryHelper(
		Scope& scope, int objectRegister, optional<DataType> objectType)
	: scope(scope), objectRegister(objectRegister),
	  objectType(objectType)
{}

LibraryHelper::call_tag const LibraryHelper::asVariable;
LibraryHelper::call_tag const LibraryHelper::asFunction;

typedef VectorBuilder<DataType> P;

Function& LibraryHelper::addFunction(
		DataType const& returnType,
		string const& name,
		vector<DataType> parameterTypes)
{
	if (objectType)
		parameterTypes.insert(parameterTypes.begin(), *objectType);
	return *scope.addFunction(returnType, name, parameterTypes);
}

void LibraryHelper::addGetter(
		int variableRegister, DataType const& type, string const& name,
		call_tag const& call)
{
	////////////////
	// Setup types.
	vector<DataType> paramTypes;
	if (objectType) paramTypes.push_back(*objectType);

	////////////////
	// Generate function code.
    vector<Opcode*> code;
    // Pop argument (object pointer).
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    // Set used object register to pointer.
    if (objectRegister != NUL)
        code.push_back(new OSetRegister(new VarArgument(objectRegister),
                                        new VarArgument(EXP2)));
    // Store variable value into EXP1.
    code.push_back(new OSetRegister(new VarArgument(EXP1),
                                    new VarArgument(variableRegister)));
    // Return from function.
    appendReturn(code);

    ////////////////
	// Create the function.
    Function* function;
    if (&call == &asVariable)
	    function = scope.addGetter(type, name, paramTypes);
    else if (&call == &asFunction)
	    function = scope.addFunction(type, name, paramTypes);
	code.front()->setLabel(function->getLabel());
    function->giveCode(code);
}

void LibraryHelper::addGetter(
		int variableRegister, DataType const& type, string const& name,
		int arraySize)
{
	////////////////
	// Setup types.
	TypeStore& typeStore = scope.getTypeStore();
	vector<DataType> paramTypes;
	if (objectType) paramTypes.push_back(*objectType);
	paramTypes.push_back(typeStore.getFloat());

	////////////////
	// Generate function code.
    vector<Opcode*> code;
    // Pop argument (index).
    code.push_back(new OPopRegister(new VarArgument(INDEX)));
    // Pop argument (object pointer).
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    // Set used object register to pointer.
    if (objectRegister != NUL)
        code.push_back(new OSetRegister(new VarArgument(objectRegister),
                                        new VarArgument(EXP2)));
    // Store variable value into EXP1.
    code.push_back(new OSetRegister(new VarArgument(EXP1),
                                    new VarArgument(variableRegister)));
    // Return from function.
    appendReturn(code);

    ////////////////
	// Create the function.
	Function& function = *scope.addGetter(type, name, paramTypes);
	code.front()->setLabel(function.getLabel());
    function.giveCode(code);
}

void LibraryHelper::addSetter(
		int variableRegister, DataType const& type, string const& name)
{
	////////////////
	// Setup types.
	TypeStore& typeStore = scope.getTypeStore();
	vector<DataType> paramTypes;
	if (objectType) paramTypes.push_back(*objectType);
	paramTypes.push_back(type);

	////////////////
	// Generate function code.
    vector<Opcode*> code;
    // Pop argument (new value).
    code.push_back(new OPopRegister(new VarArgument(EXP1)));
    // If the argument is a bool, normalize true to 1.
    int skipLabel = -1;
    if (type == typeStore.getBool())
    {
	    skipLabel = ScriptParser::getUniqueLabelID();
	    code.push_back(new OCompareImmediate(new VarArgument(EXP1),
	                                         new LiteralArgument(0)));
	    code.push_back(new OGotoTrueImmediate(new LabelArgument(skipLabel)));
	    code.push_back(new OSetImmediate(new VarArgument(EXP1),
	                                     new LiteralArgument(10000)));
    }
    // Pop argument (object pointer).
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    code.back()->setLabel(skipLabel);
    // Set used object register to pointer.
    if (objectRegister != NUL)
        code.push_back(new OSetRegister(new VarArgument(objectRegister),
                                        new VarArgument(EXP2)));
    // Store variable value into EXP1.
    code.push_back(new OSetRegister(new VarArgument(variableRegister),
                                    new VarArgument(EXP1)));
    // Return from function.
    appendReturn(code);

    ////////////////
	// Create the function.
	Function& function = *scope.addSetter(type, name, paramTypes);
	code.front()->setLabel(function.getLabel());
    function.giveCode(code);
}

void LibraryHelper::addSetter(
		int variableRegister, DataType const& type, string const& name,
		int arraySize)
{
	////////////////
	// Setup types.
	TypeStore& typeStore = scope.getTypeStore();
	vector<DataType> paramTypes;
	if (objectType) paramTypes.push_back(*objectType);
	paramTypes.push_back(typeStore.getFloat());
	paramTypes.push_back(type);

	////////////////
	// Generate function code.
    vector<Opcode*> code;
    // Pop argument (index).
    code.push_back(new OPopRegister(new VarArgument(INDEX)));
    // Pop argument (new value).
    code.push_back(new OPopRegister(new VarArgument(EXP1)));
    // Pop argument (object pointer).
    code.push_back(new OPopRegister(new VarArgument(EXP2)));
    // Set used object register to pointer.
    if (objectRegister != NUL)
        code.push_back(new OSetRegister(new VarArgument(objectRegister),
                                        new VarArgument(EXP2)));
    // Store variable value into EXP1.
    code.push_back(new OSetRegister(new VarArgument(variableRegister),
                                    new VarArgument(EXP1)));
    // Return from function.
    appendReturn(code);

    ////////////////
	// Create the function.
	Function& function = *scope.addSetter(type, name, paramTypes);
	code.front()->setLabel(function.getLabel());
    function.giveCode(code);
}

void ZScript::appendReturn(
		vector<Opcode*>& code, optional<int> label)
{
	code.push_back(new OPopRegister(new VarArgument(EXP2)));
	if (label) code.front()->setLabel(*label);
	code.push_back(new OGotoRegister(new VarArgument(EXP2)));
}

void ZScript::addPair(
		LibraryHelper& lh, int variableRegister, DataType const& type,
		string const& name)
{
	lh.addGetter(variableRegister, type, name);
	lh.addSetter(variableRegister, type, name);
}

void ZScript::addPair(
		LibraryHelper& lh, int variableRegister, DataType const& type,
		string const& name, int arraySize)
{
	lh.addGetter(variableRegister, type, name, arraySize);
	lh.addSetter(variableRegister, type, name, arraySize);
}

void ZScript::defineFunction(
		LibraryHelper& lh,
		DataType const& returnType,
		string const& name,
		vector<DataType> const& parameterTypes,
		vector<int> const& parameterRegisters,
		vector<Opcode*> const& ownedOpcodes)
{
	Function& function = lh.addFunction(returnType, name, parameterTypes);
	int parameterCount = function.paramTypes.size();
	assert(parameterRegisters.size() <= parameterCount);
	
	vector<Opcode*> code;
	// Pop the arguments into the supplied registers.
	for (vector<int>::const_reverse_iterator it =
		     parameterRegisters.rbegin();
	     it != parameterRegisters.rend(); ++it)
		code.push_back(new OPopRegister(new VarArgument(*it)));
	// Call the passed opcode.
	appendElements(code, ownedOpcodes);
	// Pop the remaining arguments.
	for (int i = parameterRegisters.size(); i < parameterCount; ++i)
		code.push_back(new OPopRegister(new VarArgument(NUL)));
	// Return from the function.
	appendReturn(code);

	code.front()->setLabel(function.getLabel());
	function.giveCode(code);
}

void ZScript::defineFunction(
		LibraryHelper& lh,
		DataType const& returnType,
		std::string const& name,
		std::vector<DataType> const& parameterTypes,
		std::vector<int> const& parameterRegisters,
		Opcode* ownedOpcode)
{
	defineFunction(
			lh, returnType, name, parameterTypes, parameterRegisters,
			VectorBuilder<Opcode*>() << ownedOpcode);
}

void ZScript::defineFunction(
		LibraryHelper& lh,
		DataType const& returnType,
		std::string const& name,
		std::vector<DataType> const& parameterTypes,
		std::vector<Opcode*> const& ownedOpcodes)
{
	defineFunction(
			lh, returnType, name, parameterTypes, vector<int>(),
			ownedOpcodes);
}

void ZScript::defineFunction(
		LibraryHelper& lh,
		DataType const& returnType,
		std::string const& name,
		std::vector<DataType> const& parameterTypes,
		Opcode* ownedOpcode)
{
	defineFunction(
			lh, returnType, name, parameterTypes, vector<int>(),
			VectorBuilder<Opcode*>() << ownedOpcode);
}

void ZScript::defineFunction(
		LibraryHelper& lh, DataType const& returnType,
		std::string const& name, Opcode* ownedOpcode)
{
	defineFunction(
			lh, returnType,
			name, vector<DataType>(), vector<int>(),
			VectorBuilder<Opcode*>() << ownedOpcode);
}

void ZScript::defineFunction(
		LibraryHelper& lh, std::string const& name, Opcode* ownedOpcode)
{
	defineFunction(
			lh, lh.getScope().getTypeStore().getVoid(),
			name, vector<DataType>(), vector<int>(),
			VectorBuilder<Opcode*>() << ownedOpcode);
}


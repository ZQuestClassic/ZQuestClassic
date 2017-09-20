#ifndef ZSCRIPT_LIBRARIES_HELPERS_H
#define ZSCRIPT_LIBRARIES_HELPERS_H

#include <string>
#include <vector>
#include "ByteCode.h"
#include "Types.h"

// Helper functions for implementing ZScript::Library::export(Scope&) in the
// separate library classes.

namespace ZScript
{
	class DataType;
	class Function;
	class Scope;
	
	// Used by Library subclasses to help setup scopes.
	class LibraryHelper
	{
	public:
		LibraryHelper(Scope&, int objectRegister,
		              optional<DataType> objectType = nullopt);
			
		// When passed this indicates whether the function shall be treated
		// as a variable (defined with a getter/setter), or as an actual
		// function call.
		struct call_tag {};
		static call_tag const asVariable;
		static call_tag const asFunction;
		
		// Make a function in the given scope with the specified return
		// type, name, and parameter types. Returns the created function.
		Function& addFunction(DataType const& returnType,
		                      std::string const& name,
		                      std::vector<DataType> parameterTypes);

		// Create a getter for a variable.
		void addGetter(
				int variableRegister, DataType const& type,
				std::string const& name,
				call_tag const& call = asVariable);

		// Create a getter for an array variable.
		void addGetter(
				int variableRegister, DataType const& type,
				std::string const& name, int arraySize);

		// Create a setter for a variable.
		void addSetter(
				int variableRegister, DataType const& type,
				std::string const& name);
		
		// Create a setter for an array variable.
		void addSetter(
				int variableRegister, DataType const& type,
				std::string const& name, int arraySize);

		// Accessors
		Scope const& getScope() const {return scope;}
		Scope& getScope() {return scope;}
		optional<DataType> getObjectType() const {return objectType;}
		int getObjectRegister() const {return objectRegister;}
		
	private:
		Scope& scope;
		int objectRegister;
		optional<DataType> objectType;
	};

	// Appends a the 2 opcode return sequence to the given code list.
	void appendReturn(vector<Opcode*>& code, optional<int> label = nullopt);
	
	// Call addGetter and addSetter with same arguments.
	void addPair(LibraryHelper& lh,
	             int variableRegister, DataType const& type,
	             std::string const& name);

	// Call addGetter and addSetter with same arguments.
	void addPair(LibraryHelper& lh,
	             int variableRegister, DataType const& type,
	             std::string const& name, int arraySize);

	// Defines a function in the specified library. The function does the
	// following:
	// * Pop arguments into the parameterRegisters, starting from the end.
	// * Run ownedOpcodes.
	// * Pop the remaining arguments off the stack and return.
	void defineFunction(
			LibraryHelper& lh,
			DataType const& returnType,
			std::string const& name,
			std::vector<DataType> const& parameterTypes,
			std::vector<int> const& parameterRegisters,
			std::vector<Opcode*> const& ownedOpcodes);

	// Single opcode variant.
	void defineFunction(
			LibraryHelper& lh,
			DataType const& returnType,
			std::string const& name,
			std::vector<DataType> const& parameterTypes,
			std::vector<int> const& parameterRegisters,
			Opcode* ownedOpcode);

	// No register variant.
	void defineFunction(
			LibraryHelper& lh,
			DataType const& returnType,
			std::string const& name,
			std::vector<DataType> const& parameterTypes,
			std::vector<Opcode*> const& ownedOpcodes);

	// No register, single opcode variant.
	void defineFunction(
			LibraryHelper& lh,
			DataType const& returnType,
			std::string const& name,
			std::vector<DataType> const& parameterTypes,
			Opcode* ownedOpcode);

	// No argument variant.
	void defineFunction(
			LibraryHelper& lh,
			DataType const& returnType,
			std::string const& name,
			Opcode* ownedOpcode);
	
	// Void, no argument variant.
	void defineFunction(
			LibraryHelper& lh,
			std::string const& name,
			Opcode* ownedOpcode);
	
}

#endif

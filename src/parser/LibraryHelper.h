#ifndef ZSCRIPT_LIBRARIES_HELPERS_H
#define ZSCRIPT_LIBRARIES_HELPERS_H

#include <string>
#include <vector>
#include "../scripting/ZAsmVariables.h"
#include "Opcode.h"
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
		LibraryHelper(Scope&, ZAsm::Variable const& objectVariable,
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
				ZAsm::Variable const& variable,
				DataType const& type, std::string const& name,
				call_tag const& call = asVariable);

		// Create a getter for an array variable.
		void addGetter(
				ZAsm::Variable const& variable,
				DataType const& type, std::string const& name,
				int arraySize);

		// Create a setter for a variable.
		void addSetter(
				ZAsm::Variable const& variable,
				DataType const& type, std::string const& name);
		
		// Create a setter for an array variable.
		void addSetter(
				ZAsm::Variable const& variable,
				DataType const& type, std::string const& name,
				int arraySize);

		// Accessors
		Scope const& getScope() const {return scope_;}
		Scope& getScope() {return scope_;}
		optional<DataType> getObjectType() const {return objectType_;}
		ZAsm::Variable const& getObjectVariable() const {
			return objectVariable_;}
		
	private:
		Scope& scope_;
		ZAsm::Variable objectVariable_;
		optional<DataType> objectType_;
	};

	// Appends a the 2 opcode return sequence to the given code list.
	void appendReturn(std::vector<ZScript::Opcode>& code,
	                  optional<int> label = nullopt);
	
	// Call addGetter and addSetter with same arguments.
	void addPair(LibraryHelper& lh,
	             ZAsm::Variable const& variable, DataType const& type,
	             std::string const& name);

	// Call addGetter and addSetter with same arguments.
	void addPair(LibraryHelper& lh,
	             ZAsm::Variable const& variable, DataType const& type,
	             std::string const& name, int arraySize);

	// Defines a function in the specified library. The function does the
	// following:
	// * Pop arguments into the registers, starting from the end.
	// * Run provided opcodes.
	// * Pop the remaining arguments off the stack and return.
	void defineFunction(
			LibraryHelper& lh,
			DataType const& returnType,
			std::string const& name,
			std::vector<DataType> const& parameterTypes,
			std::vector<ZAsm::Variable> const& parameterVariables,
			std::vector<Opcode> const& opcodes);

	// Single opcode variant.
	void defineFunction(
			LibraryHelper& lh,
			DataType const& returnType,
			std::string const& name,
			std::vector<DataType> const& parameterTypes,
			std::vector<ZAsm::Variable> const& parameterVariables,
			Opcode const& opcode);

	// No register variant.
	void defineFunction(
			LibraryHelper& lh,
			DataType const& returnType,
			std::string const& name,
			std::vector<DataType> const& parameterTypes,
			std::vector<Opcode> const& opcodes);

	// No register, single opcode variant.
	void defineFunction(
			LibraryHelper& lh,
			DataType const& returnType,
			std::string const& name,
			std::vector<DataType> const& parameterTypes,
			Opcode const& opcode);

	// No argument variant.
	void defineFunction(
			LibraryHelper& lh,
			DataType const& returnType,
			std::string const& name,
			Opcode const& opcode);
	
	// Void, no argument variant.
	void defineFunction(
			LibraryHelper& lh,
			std::string const& name,
			Opcode const& opcode);
	
}

#endif

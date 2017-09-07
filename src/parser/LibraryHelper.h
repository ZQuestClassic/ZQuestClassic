#ifndef ZSCRIPT_LIBRARIES_HELPERS_H
#define ZSCRIPT_LIBRARIES_HELPERS_H

#include <string>
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
		LibraryHelper(
				Scope&, int objectRegister, DataType const* objectType);
			
		// When passed this indicates whether the function shall be treated
		// as a variable (defined with a getter/setter), or as an actual
		// function call.
		struct call_tag {};
		static call_tag const asVariable;
		static call_tag const asFunction;
		
		// Make a function in the given scope with the specified return
		// type, name, and parameter types. Append a NULL to terminate the
		// varargs parameter list. Returns the created function.
		Function& addFunction(
				DataType const* returnType,
				std::string name, ...
				/* DataType const*... parameterTypes, */
				/* NULL */);

		// Create a getter for a variable.
		void addGetter(
				int variableRegister, DataType const* type,
				std::string const& name,
				call_tag const& call = asVariable);

		// Create a getter for an array variable.
		void addGetter(
				int variableRegister, DataType const* type,
				std::string const& name, int arraySize);

		// Create a setter for a variable.
		void addSetter(
				int variableRegister, DataType const* type,
				std::string const& name);
		
		// Create a setter for an array variable.
		void addSetter(
				int variableRegister, DataType const* type,
				std::string const& name, int arraySize);

	private:
		Scope& scope;
		int objectRegister;
		DataType const* objectType;
	};

	// Call addGetter and addSetter with same arguments.
	void addPair(LibraryHelper& lh,
	             int variableRegister, DataType const* type,
	             std::string const& name);

	// Call addGetter and addSetter with same arguments.
	void addPair(LibraryHelper& lh,
	             int variableRegister, DataType const* type,
	             std::string const& name, int arraySize);
}

#endif

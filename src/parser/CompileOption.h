#ifndef ZSCRIPT_COMPILE_OPTION_H
#define ZSCRIPT_COMPILE_OPTION_H

#include <string>
#include "CompilerUtils.h"

namespace ZScript
{
	// This class has value semantics. Basically, just treat it like an
	// enum.
	class CompileOption
	{
	public:
		static void initialize();

		// Declare static instance for each option.
#		define X(NAME, DEFAULT) \
		static CompileOption NAME;
#		include "CompileOption.xtable"
#		undef X

		bool operator==(CompileOption const& rhs) const {
			return id_ == rhs.id_;}
		
		// Is this a valid option id?
		bool isValid() const;

		// Get the name of the option.
		std::string* getName() const;
	
		// Get the default option value.
		optional<int> getDefault() const;
	
	private:
		int id_;
	};

};

#endif

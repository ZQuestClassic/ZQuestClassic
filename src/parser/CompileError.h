#ifndef COMPILE_ERROR_H
#define COMPILE_ERROR_H

#include <string>
#include "CompilerUtils.h"

#include <cstdarg>
#include <cstdio>

extern std::string formatStr(std::string const* format, ...);

namespace ZScript
{
	// Forward Declaration (AST.h)
	class AST;
	
	class CompileError
	{
	public:
		// Inner compile error implementation.
		class Impl;

		// Enum of compile error ids.
		enum Id
		{
			// Generate an id for each compile error code.
#			define X(NAME, ...) id##NAME,
#			include "CompileError.xtable"
#			undef X
			// Total number of compile error codes.
			idCount
		};

		// Call at least once before using CompileError instances.
		static void initialize();

		////////////////
		// Generate factory functions to construct CompileError instances
		// for each type of error.
		// Example:
		// static CompileError FunctionRedef(AST const*, std::string const&);

		// Define argument types. Non-void has preceding comma to fit in the
		// argument list properly.
#		define EXPAND(X) X
#		define TYPE_VOID /* ignore void types */
#		define TYPE_void /* ignore void types */
#		define TYPE_INT ,int32_t
#		define TYPE_STR ,std::string const&
		// Split on USED field.
#		define X(NAME, CODE, USED, ...) EXPAND(X_##USED(NAME, CODE, __VA_ARGS__))
#		define X_D(...) /* don't make function for deprecated error code */
#		define X_A(NAME, CODE, STRICT, ARG1, ARG2, FORMAT) \
		static CompileError NAME(AST const* TYPE_##ARG1 TYPE_##ARG2);
#		include "CompileError.xtable"
#		undef EXPAND
#		undef TYPE_VOID
#		undef TYPE_void
#		undef TYPE_INT
#		undef TYPE_STR
#		undef X
#		undef X_D
#		undef X_A
		////////////////

		CompileError(); // Default constructor - creates invalid instance.
		CompileError(CompileError const&);
		~CompileError();
		CompileError& operator=(CompileError const&);

		// Get id if a valid instance.
		std::optional<Id> getId() const;
		// Get if strict (an error), or not (a warning).
		bool isStrict() const;
		std::string toString() const;

	private:
		CompileError(Impl*);
		
		Impl* pimpl_;
	};

	void log_error(CompileError const&);
	void logDebugMessage(const char* msg);

	class CompileErrorHandler
	{
	public:
		virtual void handleError(CompileError const&, std::string const* inf = nullptr) = 0;
		virtual bool hasError() const = 0;
	};

	// Prints out the error on receiving it, and tracks the number of errors
	// and warnings.
	class SimpleCompileErrorHandler : public CompileErrorHandler
	{
	public:
		SimpleCompileErrorHandler() : errorCount_(0), warningCount_(0) {}

		void handleError(CompileError const&, std::string const* inf = nullptr);
	
		bool hasError() const {return errorCount_ > 0;}
		int32_t getErrorCount() const {return errorCount_;}
		int32_t getWarningCount() const {return warningCount_;}
	
	private:
		int32_t errorCount_;
		int32_t warningCount_;
	};
}
	
#endif

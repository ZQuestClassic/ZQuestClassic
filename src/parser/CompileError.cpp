#include "CompileError.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include "zsyssimple.h"
#include "AST.h"
#include "CompilerUtils.h"
#include "parserDefs.h"
XTableHelper XH;
#include <cstdarg>
#include <cstdio>


using namespace ZScript;
using std::string;
using std::ostringstream;
using std::setfill;
using std::setw;

////////////////////////////////////////////////////////////////
// CompileError::Impl interface

class CompileError::Impl
{
public:
	virtual ~Impl() {}
	// Create a copy.
	virtual CompileError::Impl* clone() const = 0;
	// Get this error's id.
	virtual CompileError::Id getId() const = 0;
	// Get the formatted message for this error.
	virtual string getMessage() const = 0;
	// Get the AST node that's the source of the error.
	virtual AST const* getSource() const = 0;
};

////////////////////////////////////////////////////////////////
// Table holding compile error code data.
namespace // file local
{
	// Compile Error entries
	struct Entry
	{
		// Blank entry.
		Entry() : used(false), strict(true) {}
		// Full entry.
		Entry(char const* name, char const* code,
		      bool strict, char const* format)
			: name(name), code(*code), used(true),
			  strict(strict), format(format)
		{}
		// Deprecated entry.
		Entry(char const* name, char const* code)
			: name(name), code(*code), used(false),
			  strict(false), format("")
		{}
		
		string name;
		char code;
		bool used;
		bool strict;
		string format;
	};

	Entry entries[CompileError::idCount];
}

////////////////////////////////////////////////////////////////
// CompileError::initialize()

// Fills the entries table with the xtable data.
void CompileError::initialize()
{
	static bool initialized = false;
	if (!initialized)
	{
#		define EXPAND(X) X
#		define STRICT_W false
#		define STRICT_E true
		// Split on if it's deprecated or not.
#		define X(NAME, CODE, USED, ...) EXPAND(X_##USED(NAME, CODE, __VA_ARGS__))
#		define X_D(NAME, CODE, ...) \
		entries[id##NAME] = Entry(#NAME, #CODE);
#		define X_A(NAME, CODE, STRICT, ARG1, ARG2, FORMAT) \
		entries[id##NAME] = Entry(#NAME, #CODE, STRICT_##STRICT, FORMAT);
#		include "CompileError.xtable"
#		undef EXPAND
#		undef STRICT_W
#		undef STRICT_E
#		undef X
#		undef X_D
#		undef X_A
		
		initialized = true;
	}
}

////////////////////////////////////////////////////////////////
// CompileError::Impl implementations
namespace // file local
{
	////////////////////////////////////////////////////////////////
	// Convert int32_t to int32_t, and string to char const*
	// for the format function.

	// formatArgOut<Type>::type = Type
	template <typename Arg>
	struct formatArgOut {typedef Arg type;};
	// formatArgOut<string>::type = char const*
	template <>
	struct formatArgOut<string> {typedef char const* type;};

	// formatArg returns argument by default.
	template <typename Arg>
	typename formatArgOut<Arg>::type formatArg(Arg const& arg) {return arg;}

	// If a string, formatArg returns the c string underneath.
	template <>
	char const* formatArg<string>(string const& arg)
	{
		return arg.c_str();
	}
	////////////////////////////////////////////////////////////////
	
	// Two argument specialization (default).
	template <typename A = void, typename B = void>
	class CEImpl : public CompileError::Impl
	{
	public:
		CEImpl(CompileError::Id id, AST const* source,
		       A const& arg1, B const& arg2)
			: id_(id), source_(source), arg1_(arg1), arg2_(arg2)
		{}
		CEImpl* clone() const /*override*/ {return new CEImpl(*this);}
		
		CompileError::Id getId() const /*override*/ {return id_;}
		string getMessage() const /*override*/ {
			return XH.formatStr(&entries[id_].format,
			                 formatArg<A>(arg1_),
			                 formatArg<B>(arg2_));}
		AST const* getSource() const /*override*/ {return source_;}

	private:
		CompileError::Id id_;
		AST const* source_;
		A arg1_;
		B arg2_;
	};

	// One argument specialization.
	template <typename A>
	class CEImpl<A, void> : public CompileError::Impl
	{
	public:
		CEImpl(CompileError::Id id, AST const* source, A const& arg)
			: id_(id), source_(source), arg_(arg)
		{}
		CEImpl* clone() const /*override*/ {return new CEImpl(*this);}
		
		CompileError::Id getId() const /*override*/ {return id_;}
		string getMessage() const /*override*/ {
			return XH.formatStr(&entries[id_].format, formatArg<A>(arg_));}
		AST const* getSource() const /*override*/ {return source_;}

	private:
		CompileError::Id id_;
		AST const* source_;
		A arg_;
	};

	// No argument specialization.
	template <>
	class CEImpl<void, void> : public CompileError::Impl
	{
	public:
		CEImpl(CompileError::Id id, AST const* source)
			: id_(id), source_(source)
		{}
		CEImpl* clone() const /*override*/ {return new CEImpl(*this);}
		
		CompileError::Id getId() const /*override*/ {return id_;}
		string getMessage() const /*override*/ {return entries[id_].format;}
		AST const* getSource() const /*override*/ {return source_;}

	private:
		CompileError::Id id_;
		AST const* source_;
	};

}

////////////////////////////////////////////////////////////////
// CompileError factory functions

#define EXPAND(X) X
// Define argument types. Non-void has preceding comma to fit in the
// argument list properly.
#define ARG_VOID(ARGNAME) /* ignore void types */
#define ARG_INT(ARGNAME) ,int32_t ARGNAME
#define ARG_STR(ARGNAME) ,string const& ARGNAME
// CEImpl template types
#define TYPE_VOID void
#define TYPE_INT int32_t
#define TYPE_STR string
// CEImpl constructor arguments
#define ARGC_VOID(ARGNAME) /* ignore void types */
#define ARGC_INT(ARGNAME) ,ARGNAME
#define ARGC_STR(ARGNAME) ,ARGNAME
// Split on USED field.
#define X(NAME, CODE, USED, ...) EXPAND(X_##USED(NAME, CODE, __VA_ARGS__))
#define X_D(...) /* don't make function for deprecated error code */
#define X_A(NAME, CODE, STRICT, ARG1, ARG2, FORMAT) \
CompileError CompileError::NAME( \
		AST const* source ARG_##ARG1(arg1) ARG_##ARG2(arg2)) \
{ \
	return new CEImpl<TYPE_##ARG1, TYPE_##ARG2>( \
			id##NAME, source ARGC_##ARG1(arg1) ARGC_##ARG2(arg2)); \
}
#include "CompileError.xtable"
#undef EXPAND
#undef ARG_VOID
#undef ARG_INT
#undef ARG_STR
#undef TYPE_VOID
#undef TYPE_INT
#undef TYPE_STR
#undef ARGC_VOID
#undef ARGC_INT
#undef ARGC_STR
#undef X
#undef X_D
#undef X_A

////////////////////////////////////////////////////////////////
// CompileError

CompileError::CompileError() : pimpl_(NULL) {}

CompileError::CompileError(CompileError const& other)
	: pimpl_(other.pimpl_ ? other.pimpl_->clone() : NULL)
{}

CompileError::~CompileError()
{
	delete pimpl_;
}

CompileError& CompileError::operator=(CompileError const& rhs)
{
	delete pimpl_;
	pimpl_ = rhs.pimpl_ ? rhs.pimpl_->clone() : NULL;
	return *this;
}

std::optional<CompileError::Id> CompileError::getId() const
{
	if (pimpl_) return pimpl_->getId();
	return std::nullopt;
}

bool CompileError::isStrict() const
{
	if (!pimpl_) return true;
	return entries[pimpl_->getId()].strict;
}

string CompileError::toString() const
{
	if (!pimpl_) return "unknown error";
	Id id = pimpl_->getId();
	Entry& entry = entries[id];
	
	ostringstream oss;

	// Output location data.
	if (AST const* source = pimpl_->getSource())
	{
		if (auto loc = source->getIdentifierLocation())
			oss << loc->asString();
		else
	    	oss << source->location.asString();
	}

	// Error or warning?
	oss << " - " << (isStrict() ? "Error" : "Warning");

	// Error Code and Id
	oss << " " << entry.code
	    << setw(3) << setfill('0') << id << setw(0)
	    << ": ";

	// Message.
	oss << pimpl_->getMessage();

	return oss.str();
}

BasicCompileError::BasicCompileError(CompileError const& err)
{
	errmsg = err.toString();
	id = *(err.getId());
	strict = err.isStrict();
}
void BasicCompileError::print() const
{
	std::string s = errmsg;
	char const* ptr = s.c_str();
	while(ptr[0]==' '||ptr[0]=='\r'||ptr[0]=='\n') ++ptr;
	if(strict)
		zconsole_error("%s",ptr);
	else
		zconsole_warn("%s",ptr);
}
extern bool zscript_error_out;
extern uint32_t zscript_failcode;
void BasicCompileError::handle() const
{
	print();
	if(!zscript_failcode && strict)
		zscript_failcode = id;
	zscript_error_out = true;
}

CompileError::CompileError(CompileError::Impl* pimpl) : pimpl_(pimpl) {}

void ZScript::log_error(CompileError const& error)
{
	zconsole_error("%s", error.toString().c_str());
}

void ZScript::logDebugMessage(const char* msg)
{
	zconsole_info("Debug: %s", msg);
}

////////////////////////////////////////////////////////////////
// CompileErrorHandler

void SimpleCompileErrorHandler::handleError(CompileError const& error, std::string const* inf)
{
	if (error.isStrict())
	{
		++errorCount_;
		if(inf && inf->size())
			zconsole_error("%s\nINFO: %s",error.toString().c_str(),inf->c_str());
		else zconsole_error("%s",error.toString().c_str());
	}
	else
	{
		++warningCount_;
		if(inf && inf->size())
			zconsole_warn("%s\nINFO: %s",error.toString().c_str(),inf->c_str());
		else zconsole_warn("%s",error.toString().c_str());
	}
}

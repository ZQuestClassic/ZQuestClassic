#ifndef ZSCRIPT_OPCODE_H
#define ZSCRIPT_OPCODE_H

#include <string>
#include <vector>
#include "../scripting/ZAsmCommands.h"
#include "../scripting/ZAsmVariables.h"
#include "CompilerUtils.h"

#include "../scripting/ZAsmCommandTable.h"

namespace ZScript
{
	////////////////////////////////////////////////////////////////
	// Label
	class Label
	{
	public:
		Label() {}
		Label(int id) : id_(id) {}

		optional<int> getId() const {return id_;}
		optional<int> getLine() const {return line_;}
		void setLine(int line) {line_ = line;}
		
	private:
		optional<int> id_;
		optional<int> line_;
	};
	
	
	////////////////////////////////////////////////////////////////
	// Opcode Arguments

	// Pimpl idiom
	namespace detail
	{
		class OpcodeArgumentImpl
		{
		public:
			virtual ~OpcodeArgumentImpl() {}
			virtual OpcodeArgumentImpl* clone() const = 0;
			virtual std::string toString() const = 0;
			virtual Label* getLabel() {return NULL;}
		};
	}
	
	class OpcodeArgument : public SafeBool<OpcodeArgument>
	{
	public:
		static OpcodeArgument literal(long value);
		static OpcodeArgument variable(ZAsm::Variable const& variable);
		static OpcodeArgument label(int label);
		static OpcodeArgument label(Label const& label);
		
		OpcodeArgument() : pimpl_(NULL) {}
		
		OpcodeArgument(OpcodeArgument const& rhs)
			: pimpl_(rhs.pimpl_ ? rhs.pimpl_->clone() : NULL) {}
		~OpcodeArgument() {delete pimpl_;}
		OpcodeArgument& operator=(OpcodeArgument const& rhs) {
			delete pimpl_;
			pimpl_ = rhs.pimpl_ ? rhs.pimpl_->clone() : NULL;
			return *this;}

		std::string toString() const {
			return pimpl_ ? pimpl_->toString() : "<NULL>";}
		Label* getLabel() const {
			return pimpl_ ? pimpl_->getLabel() : NULL;}
		bool safe_bool() const {return pimpl_;}

	private:
		detail::OpcodeArgumentImpl* pimpl_;

		OpcodeArgument(detail::OpcodeArgumentImpl* pimpl) : pimpl_(pimpl) {}
	};

	////////////////////////////////////////////////////////////////
	// Standard register arguments.
	
	ZAsm::Variable& varIndex1();
	ZAsm::Variable& varIndex2();
	ZAsm::Variable& varExp1();
	ZAsm::Variable& varExp2();
	ZAsm::Variable& varSFrame();
	ZAsm::Variable& varNull();
	ZAsm::Variable& varSFTemp();

	////////////////////////////////////////////////////////////////
	// Opcodes
	
	class Opcode
	{
	public:
		Opcode();
		Opcode(ZAsm::CommandDef* command);
		Opcode(ZAsm::CommandDef* command,
		       OpcodeArgument const& firstArg);
		Opcode(ZAsm::CommandDef* command,
		       OpcodeArgument const& firstArg,
		       OpcodeArgument const& secondArg);
		
		ZAsm::CommandDef* getDef() const {return command_;}
		OpcodeArgument const& getFirstArg() const {return firstArg_;}
		OpcodeArgument& getFirstArg() {return firstArg_;}
		OpcodeArgument const& getSecondArg() const {return secondArg_;}
		OpcodeArgument& getSecondArg() {return secondArg_;}

		Label& getLabel() {return label_;}
		// Overwrites label and returns self.
		Opcode& withLabel(Label const& label);
		
		std::string getComment() const {return comment_;}
		// Appends comment and returns self.
		Opcode& withComment(std::string const& comment);

		std::string toString() const;
		std::string toLine(bool showLabel = false) const;

	private:
		ZAsm::CommandDef* command_;
		OpcodeArgument firstArg_;
		OpcodeArgument secondArg_;
		Label label_;
		std::string comment_;
	};

	std::vector<Label*> getArgLabels(Opcode const&);
		
	// Declares opcode functions. Example:
	// Opcode opSETV(ZAsm::Variable const& arg0, long arg1) {
	//     return Opcode(&Cmd_SETV,
	//                   OpcodeArgument::variable(arg0),
	//                   OpcodeArgument::literal(arg1));}
	typedef OpcodeArgument Arg;
	typedef ZAsm::Variable Var;
#	define ArgNone_ArgNone(NAME)\
	Opcode op##NAME();
#	define ArgLabel_ArgNone(NAME)\
	Opcode op##NAME(int arg0);
#	define ArgLabel_ArgRegister(NAME)\
	Opcode op##NAME(int arg0, Var const& arg1);
#	define ArgRegister_ArgNone(NAME)\
	Opcode op##NAME(Var const& arg0);
#	define ArgRegister_ArgRegister(NAME)\
	Opcode op##NAME(Var const& arg0, Var const& arg1);
#	define ArgRegister_ArgValue(NAME)\
	Opcode op##NAME(Var const& arg0, long arg1);
#	define ArgValue_ArgNone(NAME)\
	Opcode op##NAME(long arg0);
#	define ArgValue_ArgRegister(NAME)\
	Opcode op##NAME(long arg0, Var const& arg1);
#	define ArgValue_ArgValue(NAME)\
	Opcode op##NAME(long arg0, long arg1);
#	define COMMAND(ARG0, ARG1, ID, NAME) ARG0##_##ARG1 (NAME)
	ZASM_COMMAND_TABLE
#	undef ArgNone_ArgNone
#	undef ArgLabel_ArgNone
#	undef ArgLabel_ArgRegister
#	undef ArgRegister_ArgNone
#	undef ArgRegister_ArgRegister
#	undef ArgRegister_ArgValue
#	undef ArgValue_ArgNone
#	undef ArgValue_ArgRegister
#	undef ArgValue_ArgValue
#	undef COMMAND

	// Use a label argument for SETV.
	Opcode opSETV(ZAsm::Variable const& arg0, Label const& arg1);
}

#undef ZASM_COMMAND_TABLE
#endif

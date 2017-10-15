#include "../precompiled.h"
#include "Opcode.h"

#include <cmath>
#include <iomanip>
#include <sstream>

#include "../scripting/ZAsmCommandTable.h"

using namespace std;
using namespace ZScript;

////////////////////////////////////////////////////////////////
// Opcode Argument Implementations

class LiteralArg : public detail::OpcodeArgumentImpl
{
public:
	LiteralArg(long value) : value_(value) {}
	virtual LiteralArg* clone() const {return new LiteralArg(*this);}

	virtual string toString() const
	{
		ostringstream out;
		if (value_ < 0) out << "-";

		out << (abs(value_) / 10000);
		if (value_ % 10000 == 0) return out.str();

		out << "." << setfill('0') << setw(4) << (abs(value_) % 10000);
		return out.str();
	}

private:
	long value_;
};

class VarArg : public detail::OpcodeArgumentImpl
{
public:
	VarArg(ZAsm::Variable const& variable) : variable_(variable) {}
	virtual VarArg* clone() const {return new VarArg(*this);}

	virtual string toString() const {return variable_.toString();}
	
private:
	ZAsm::Variable variable_;
};

class LabelArg : public detail::OpcodeArgumentImpl
{
public:
	LabelArg(Label const& label) : label_(label) {}
	virtual LabelArg* clone() const {return new LabelArg(*this);}

	virtual string toString() const
	{
		ostringstream out;
		if (optional<int> line = label_.getLine())
			out << *line;
		else
			out << "l" << *label_.getId();
		return out.str();
	}
	virtual Label* getLabel() {return &label_;}
	
private:
	Label label_;
};

////////////////////////////////////////////////////////////////
// Opcode Argument

OpcodeArgument OpcodeArgument::literal(long value)
{
	return OpcodeArgument(new LiteralArg(value));
}

OpcodeArgument OpcodeArgument::variable(ZAsm::Variable const& variable)
{
	return OpcodeArgument(new VarArg(variable));
}

OpcodeArgument OpcodeArgument::label(int label)
{
	return OpcodeArgument(new LabelArg(Label(label)));
}

OpcodeArgument OpcodeArgument::label(Label const& label)
{
	return OpcodeArgument(new LabelArg(label));
}

////////////////////////////////////////////////////////////////
// Standard register arguments.

ZAsm::Variable& ZScript::varIndex1()
{
	static ZAsm::Variable instance = ZAsm::varD(0);
	return instance;
}

ZAsm::Variable& ZScript::varIndex2()
{
	static ZAsm::Variable instance = ZAsm::varD(1);
	return instance;
}

ZAsm::Variable& ZScript::varExp1()
{
	static ZAsm::Variable instance = ZAsm::varD(2);
	return instance;
}

ZAsm::Variable& ZScript::varExp2()
{
	static ZAsm::Variable instance = ZAsm::varD(3);
	return instance;
}

ZAsm::Variable& ZScript::varSFrame()
{
	static ZAsm::Variable instance = ZAsm::varD(4);
	return instance;
}

ZAsm::Variable& ZScript::varNull()
{
	static ZAsm::Variable instance = ZAsm::varD(5);
	return instance;
}

ZAsm::Variable& ZScript::varSFTemp()
{
	static ZAsm::Variable instance = ZAsm::varD(6);
	return instance;
}

////////////////////////////////////////////////////////////////
// Opcode

Opcode::Opcode() : command_(NULL) {}

Opcode::Opcode(ZAsm::CommandDef* command)
	: command_(command)
{}

Opcode::Opcode(ZAsm::CommandDef* command,
		       OpcodeArgument const& firstArg)
	: command_(command), firstArg_(firstArg)
{}

Opcode::Opcode(ZAsm::CommandDef* command,
               OpcodeArgument const& firstArg,
		       OpcodeArgument const& secondArg)
	: command_(command), firstArg_(firstArg), secondArg_(secondArg)
{}

Opcode& Opcode::withLabel(Label const& label)
{
	label_ = label;
	return *this;
}

Opcode& Opcode::withComment(std::string const& comment)
{
	if (comment_.size() != 0) comment_ += " ";
	comment_ += comment;
	return *this;
}

string Opcode::toString() const
{
	if (!command_) {return "<NULL>";}

	ostringstream out;
	out << command_->getName();
	if (firstArg_) out << " " << firstArg_.toString();
	if (secondArg_) out << "," << secondArg_.toString();
	return out.str();
}

string Opcode::toLine(bool showLabel) const
{
	ostringstream out;
	if (!showLabel)
		out << " ";
	else if (optional<int> id = label_.getId())
		out << "l" << *id << ": ";
	else
		out << " ";
	
	out << toString();

	if (!comment_.empty())
		out << " ; " << comment_;
	out << "\n";

	return out.str();
}

vector<Label*> ZScript::getArgLabels(Opcode const& opcode)
{
	vector<Label*> out;
	if (Label* label = opcode.getFirstArg().getLabel())
		out.push_back(label);
	if (Label* label = opcode.getSecondArg().getLabel())
		out.push_back(label);
	return out;
}

// Declares opcode functions. Example:
// Opcode ZScript::opSETV(ZAsm::Variable const& arg0, long arg1) {
//     return Opcode(&Cmd_SETV,
//                   OpcodeArgument::variable(arg0),
//                   OpcodeArgument::literal(arg1));}
typedef OpcodeArgument Arg;
typedef ZAsm::Variable Var;
#define ArgNone_ArgNone(NAME)\
	Opcode ZScript::op##NAME() {return Opcode(&ZAsm::Cmd_##NAME);}
#define ArgLabel_ArgNone(NAME)\
	Opcode ZScript::op##NAME(int arg0) { \
		return Opcode(&ZAsm::Cmd_##NAME, Arg::label(arg0));}
#define ArgLabel_ArgRegister(NAME)\
	Opcode ZScript::op##NAME(int arg0, Var const& arg1) { \
		return Opcode(&ZAsm::Cmd_##NAME, Arg::label(arg0), \
		              Arg::variable(arg1));}
#define ArgRegister_ArgNone(NAME)\
	Opcode ZScript::op##NAME(Var const& arg0) { \
		return Opcode(&ZAsm::Cmd_##NAME, Arg::variable(arg0));}
#define ArgRegister_ArgRegister(NAME)\
	Opcode ZScript::op##NAME(Var const& arg0, Var const& arg1) { \
		return Opcode(&ZAsm::Cmd_##NAME, Arg::variable(arg0), \
		              Arg::variable(arg1));}
#define ArgRegister_ArgValue(NAME)\
	Opcode ZScript::op##NAME(Var const& arg0, long arg1) { \
		return Opcode(&ZAsm::Cmd_##NAME, Arg::variable(arg0), \
		              Arg::literal(arg1));}
#define ArgValue_ArgNone(NAME)\
	Opcode ZScript::op##NAME(long arg0) { \
		return Opcode(&ZAsm::Cmd_##NAME, Arg::literal(arg0));}
#define ArgValue_ArgRegister(NAME)\
	Opcode ZScript::op##NAME(long arg0, Var const& arg1) { \
		return Opcode(&ZAsm::Cmd_##NAME, Arg::literal(arg0), \
		              Arg::variable(arg1));}
#define ArgValue_ArgValue(NAME)\
	Opcode ZScript::op##NAME(long arg0, long arg1) { \
		return Opcode(&ZAsm::Cmd_##NAME, Arg::literal(arg0), \
		              Arg::literal(arg1));}
#define COMMAND(ARG0, ARG1, ID, NAME) ARG0##_##ARG1 (NAME)
ZASM_COMMAND_TABLE
#undef ArgNone_ArgNone
#undef ArgLabel_ArgNone
#undef ArgLabel_ArgRegister
#undef ArgRegister_ArgNone
#undef ArgRegister_ArgRegister
#undef ArgRegister_ArgValue
#undef ArgValue_ArgNone
#undef ArgValue_ArgRegister
#undef ArgValue_ArgValue
#undef COMMAND

Opcode ZScript::opSETV(ZAsm::Variable const& arg0, Label const& arg1)
{
	return Opcode(&ZAsm::Cmd_SETV, Arg::variable(arg0), Arg::label(arg1));
}


#undef ZASM_COMMAND_TABLE

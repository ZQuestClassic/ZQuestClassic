#ifndef OPCODE_H_
#define OPCODE_H_

#include "Compiler.h"
#include "zasm/defines.h"
#include "zasm/serialize.h"

#include <memory>
#include <vector>
#include <string>

int32_t StringToVar(std::string var);

namespace ZScript
{
	class LiteralArgument;
	class CompareArgument;
	class VarArgument;
	class LiteralVarArgument;
	class LabelArgument;
	class GlobalArgument;
	class StringArgument;
	class VectorArgument;

	class ArgumentVisitor
	{
	public:
		virtual void caseLiteral(LiteralArgument&, void *){}
		virtual void caseCompare(CompareArgument&, void *){}
		virtual void caseString(StringArgument&, void *){}
		virtual void caseVector(VectorArgument&, void *){}
		virtual void caseVar(VarArgument&, void *){}
		virtual void caseLiteralVar(LiteralVarArgument&, void *){}
		virtual void caseLabel(LabelArgument&, void *){}
		virtual void caseGlobal(GlobalArgument&, void *){}
		void execute(std::vector<std::shared_ptr<Opcode>>& vec, void* param)
		{
			for (auto it = vec.begin(); it != vec.end(); ++it)
			{
				(*it)->execute(*this, param);
			}
		}
		void execute(std::vector<std::shared_ptr<Opcode>> const& vec, void* param)
		{
			for (auto it = vec.cbegin(); it != vec.cend(); ++it)
			{
				(*it)->execute(*this, param);
			}
		}
		virtual ~ArgumentVisitor() {}
	};

	class Argument
	{
	public:
		virtual std::string toString() const = 0;
		virtual void execute(ArgumentVisitor &host, void *param)=0;
		virtual Argument* clone() const = 0;
		virtual ~Argument() {}
		bool operator==(Argument const& other) const
		{
			return toString() == other.toString();
		}
	};

	class LiteralArgument : public Argument
	{
	public:
		LiteralArgument(int32_t Value) : value(Value) {}
		LiteralArgument(ScriptType Value) : value((int)Value) {}
		std::string toString() const;
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseLiteral(*this, param);
		}
		LiteralArgument* clone() const
		{
			return new LiteralArgument(value);
		}
		bool operator==(int val) const
		{
			return val == value;
		}
		bool operator==(zfix const& val) const
		{
			return val.getZLong() == value;
		}
		int32_t value;
	};
	inline bool operator==(int val, LiteralArgument const& arg)
	{
		return arg == val;
	}
	inline bool operator==(zfix const& val, LiteralArgument const& arg)
	{
		return arg == val;
	}
	
	class CompareArgument : public Argument
	{
	public:
		CompareArgument(int32_t Value) : value(Value) {}
		std::string toString() const;
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseCompare(*this, param);
		}
		CompareArgument* clone() const
		{
			return new CompareArgument(value);
		}
		int32_t value;
	};
	
	class StringArgument : public Argument
	{
	public:
		StringArgument(std::string const& Value) : value(Value) {}
		std::string toString() const;
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseString(*this, param);
		}
		StringArgument* clone() const
		{
			return new StringArgument(value);
		}
		bool operator==(string const& str) const
		{
			return str == value;
		}
	private:
		std::string value;
	};
	inline bool operator==(string const& val, StringArgument const& arg)
	{
		return arg == val;
	}
	
	class VectorArgument : public Argument
	{
	public:
		VectorArgument(std::vector<int32_t> const& Value) : value(Value) {}
		std::string toString() const;
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseVector(*this, param);
		}
		VectorArgument* clone() const
		{
			return new VectorArgument(value);
		}
		bool operator==(vector<int32_t> const& vec) const
		{
			return vec == value;
		}
	private:
		std::vector<int32_t> value;
	};
	inline bool operator==(vector<int32_t> const& val, VectorArgument const& arg)
	{
		return arg == val;
	}

	std::string VarToString(int32_t ID);

	class VarArgument : public Argument
	{
	public:
		VarArgument(int32_t id) : ID(id) {}
		std::string toString() const;
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseVar(*this,param);
		}
		VarArgument* clone() const
		{
			return new VarArgument(ID);
		}
		int32_t ID;
	};

	class LiteralVarArgument : public Argument
	{
	public:
		LiteralVarArgument(int32_t id) : ID(id) {}
		std::string toString() const;
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseLiteralVar(*this,param);
		}
		LiteralVarArgument* clone() const
		{
			return new LiteralVarArgument(ID);
		}
		int32_t ID;
	};

	class GlobalArgument : public VarArgument
	{
	public:
		GlobalArgument(int32_t id) : VarArgument(id) {}
		std::string toString() const;
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseGlobal(*this,param);
		}
		GlobalArgument* clone() const
		{
			return new GlobalArgument(ID);
		}
	};

	class LabelArgument : public Argument
	{
	public:
		LabelArgument(int32_t id, bool altstr = false) : ID(id), altstr(altstr), haslineno(false) {}
		std::string toString() const;
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseLabel(*this,param);
		}
		LabelArgument* clone() const
		{
			return new LabelArgument(ID, altstr);
		}
		void setID(int32_t newid)
		{
			ID = newid;
		}
		int32_t getID()
		{
			return ID;
		}
		void setLineNo(int32_t l)
		{
			haslineno=true;
			lineno=l;
		}
		bool operator==(int lbl) const
		{
			return lbl == ID;
		}
	private:
		int32_t ID;
		int32_t lineno;
		bool haslineno;
		bool altstr;
	};
	inline bool operator==(int val, LabelArgument const& arg)
	{
		return arg == val;
	}

	template <ASM_DEFINE Command>
	class Opcode0 : public Opcode
	{
	public:
		Opcode0() {}
		~Opcode0() {}
		void execute(ArgumentVisitor &host, void *param)
		{
		}
		std::string toString() const
		{
			return zasm_op_to_string(Command);
		}
	protected:
		Opcode* clone() const
		{
			return new Opcode0<Command>();
		}
	};

	template <ASM_DEFINE Command, typename Arg>
	class Opcode1 : public Opcode
	{
	public:
		Opcode1(Arg *A) : a(A) {}
		~Opcode1()
		{
			delete a;
		}
		Arg* getArgument()
		{
			return a;
		}
		Arg const* getArgument() const
		{
			return a;
		}
		Arg* takeArgument()
		{
			auto tmp = a;
			a = nullptr;
			return tmp;
		}
		void execute(ArgumentVisitor &host, void *param)
		{
			a->execute(host, param);
		}
		std::string toString() const
		{
			return fmt::format("{} {}", zasm_op_to_string(Command), a->toString());
		}
	protected:
		Arg *a;

		Opcode* clone() const
		{
			return new Opcode1<Command, Arg>(a->clone());
		}
	};

	template <ASM_DEFINE Command, typename Arg1, typename Arg2>
	class Opcode2 : public Opcode
	{
	public:
		Opcode2(Arg1 *A, Arg2 *B) : a(A), b(B) {}
		~Opcode2()
		{
			delete a;
			delete b;
		}
		Arg1* getFirstArgument()
		{
			return a;
		}
		Arg1 const* getFirstArgument() const
		{
			return a;
		}
		Arg2* getSecondArgument()
		{
			return b;
		}
		Arg2 const* getSecondArgument() const
		{
			return b;
		}
		Arg1* takeFirstArgument()
		{
			auto tmp = a;
			a = nullptr;
			return tmp;
		}
		Arg2* takeSecondArgument()
		{
			auto tmp = b;
			b = nullptr;
			return tmp;
		}
		void execute(ArgumentVisitor &host, void *param)
		{
			a->execute(host, param);
			b->execute(host, param);
		}
		std::string toString() const
		{
			return fmt::format("{} {} {}", zasm_op_to_string(Command), a->toString(), b->toString());
		}
	protected:
		Arg1 *a;
		Arg2 *b;

		Opcode* clone() const
		{
			return new Opcode2<Command, Arg1, Arg2>(a->clone(), b->clone());
		}
	};

	template <ASM_DEFINE Command, typename Arg1, typename Arg2, typename Arg3>
	class Opcode3 : public Opcode
	{
	public:
		Opcode3(Arg1 *A, Arg2 *B, Arg3 *C) : a(A), b(B), c(C) {}
		~Opcode3()
		{
			delete a;
			delete b;
			delete c;
		}
		Arg1* getFirstArgument()
		{
			return a;
		}
		Arg1 const* getFirstArgument() const
		{
			return a;
		}
		Arg2* getSecondArgument()
		{
			return b;
		}
		Arg2 const* getSecondArgument() const
		{
			return b;
		}
		Arg3* getThirdArgument()
		{
			return c;
		}
		Arg3 const* getThirdArgument() const
		{
			return c;
		}
		Arg1* takeFirstArgument()
		{
			auto tmp = a;
			a = nullptr;
			return tmp;
		}
		Arg2* takeSecondArgument()
		{
			auto tmp = b;
			b = nullptr;
			return tmp;
		}
		Arg3* takeThirdArgument()
		{
			auto tmp = c;
			c = nullptr;
			return tmp;
		}
		void execute(ArgumentVisitor &host, void *param)
		{
			a->execute(host, param);
			b->execute(host, param);
			c->execute(host, param);
		}
		std::string toString() const
		{
			return fmt::format("{} {} {} {}", zasm_op_to_string(Command), a->toString(), b->toString(), c->toString());
		}
	protected:
		Arg1 *a;
		Arg2 *b;
		Arg3 *c;

		Opcode* clone() const
		{
			return new Opcode3<Command, Arg1, Arg2, Arg3>(a->clone(), b->clone(), c->clone());
		}
	};

	class RawOpcode : public Opcode
	{
	public:
		RawOpcode(std::string str) : str(str) {}
		void execute(ArgumentVisitor &host, void *param)
		{
		}
		std::string toString() const
		{
			return str;
		}
		Opcode* clone() const
		{
			return new RawOpcode(str);
		}
		std::string str;
	};

}

#endif

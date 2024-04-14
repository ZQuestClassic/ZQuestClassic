#ifndef BYTECODE_H
#define BYTECODE_H

#include "ASTVisitors.h"
#include "DataStructs.h"
#include "Compiler.h"
#include "zsyssimple.h"
#include "zasm/defines.h"

#include <string>

/*
 SP - stack pointer
 D4 - stack frame pointer
 D6 - stack frame offset accumulator
 D2 - expression accumulator #1
 D3 - expression accumulator #2
 D0 - array index accumulator
 D1 - secondary array index accumulator
 D5 - pure SETR sink
 */
#define INDEX                   rINDEX
#define INDEX2                  rINDEX2
#define EXP1                    rEXP1
#define EXP2                    rEXP2
#define SFRAME                  rSFRAME
#define NUL                     rNUL
#define SFTEMP                  rSFTEMP
#define WHAT_NO_7               rWHAT_NO_7

//{ INTERNAL ARRAYS

#define INTARR_OFFS 65536
#define INTARR_SCREEN_NPC       (65536+0)
#define INTARR_SCREEN_ITEMSPR   (65536+1)
#define INTARR_SCREEN_LWPN      (65536+2)
#define INTARR_SCREEN_EWPN      (65536+3)
#define INTARR_SCREEN_FFC       (65536+4)
#define INTARR_SCREEN_PORTALS   (65536+5)
#define INTARR_SAVPRTL          (65536+6)

//} END INTERNAL ARRAYS

namespace ZScript
{
	class LiteralArgument;
	class CompareArgument;
	class VarArgument;
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
		Argument* clone() const
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
		Argument* clone() const
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
		Argument* clone() const
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
		Argument* clone() const
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
		Argument* clone() const
		{
			return new VarArgument(ID);
		}
		int32_t ID;
	};

	class GlobalArgument : public Argument
	{
	public:
		GlobalArgument(int32_t id) : ID(id) {}
		std::string toString() const;
		void execute(ArgumentVisitor &host, void *param)
		{
			host.caseGlobal(*this,param);
		}
		Argument* clone() const
		{
			return new GlobalArgument(ID);
		}
	private:
		int32_t ID;
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
		Argument* clone() const
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

	class UnaryOpcode : public Opcode
	{
	public:
		UnaryOpcode(Argument *A) : a(A) {}
		~UnaryOpcode()
		{
			delete a;
		}
		Argument* getArgument()
		{
			return a;
		}
		Argument const* getArgument() const
		{
			return a;
		}
		Argument* takeArgument()
		{
			auto tmp = a;
			a = nullptr;
			return tmp;
		}
		void execute(ArgumentVisitor &host, void *param)
		{
			a->execute(host, param);
		}
	protected:
		Argument *a;
	};

	class BinaryOpcode : public Opcode
	{
	public:
		BinaryOpcode(Argument *A, Argument *B) : a(A), b(B) {}
		~BinaryOpcode()
		{
			delete a;
			delete b;
		}
		Argument* getFirstArgument()
		{
			return a;
		}
		Argument const* getFirstArgument() const
		{
			return a;
		}
		Argument* getSecondArgument()
		{
			return b;
		}
		Argument const* getSecondArgument() const
		{
			return b;
		}
		Argument* takeFirstArgument()
		{
			auto tmp = a;
			a = nullptr;
			return tmp;
		}
		Argument* takeSecondArgument()
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
	protected:
		Argument *a;
		Argument *b;
	};
	
	class TernaryOpcode : public Opcode
	{
	public:
		TernaryOpcode(Argument *A, Argument *B, Argument *C) : a(A), b(B), c(C) {}
		~TernaryOpcode()
		{
			delete a;
			delete b;
			delete c;
		}
		Argument* getFirstArgument()
		{
			return a;
		}
		Argument const* getFirstArgument() const
		{
			return a;
		}
		Argument* getSecondArgument()
		{
			return b;
		}
		Argument const* getSecondArgument() const
		{
			return b;
		}
		Argument* getThirdArgument()
		{
			return c;
		}
		Argument const* getThirdArgument() const
		{
			return c;
		}
		Argument* takeFirstArgument()
		{
			auto tmp = a;
			a = nullptr;
			return tmp;
		}
		Argument* takeSecondArgument()
		{
			auto tmp = b;
			b = nullptr;
			return tmp;
		}
		Argument* takeThirdArgument()
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
	protected:
		Argument *a;
		Argument *b;
		Argument *c;
	};

	class OSetTrue : public UnaryOpcode
	{
	public:
		OSetTrue(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetTrue(a->clone());
		}
	};

	class OSetTrueI : public UnaryOpcode
	{
	public:
		OSetTrueI(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetTrueI(a->clone());
		}
	};

	class OSetFalse : public UnaryOpcode
	{
	public:
		OSetFalse(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetFalse(a->clone());
		}
	};

	class OSetFalseI : public UnaryOpcode
	{
	public:
		OSetFalseI(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetFalseI(a->clone());
		}
	};

	class OSetMore : public UnaryOpcode
	{
	public:
		OSetMore(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetMore(a->clone());
		}
	};

	class OSetMoreI : public UnaryOpcode
	{
	public:
		OSetMoreI(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetMoreI(a->clone());
		}
	};

	class OSetLess : public UnaryOpcode
	{
	public:
		OSetLess(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetLess(a->clone());
		}
	};

	class OSetLessI : public UnaryOpcode
	{
	public:
		OSetLessI(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetLessI(a->clone());
		}
	};

	class OSetImmediate : public BinaryOpcode
	{
	public:
		OSetImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetImmediate(a->clone(),b->clone());
		}
	};

	class OSetRegister : public BinaryOpcode
	{
	public:
		OSetRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetRegister(a->clone(),b->clone());
		}
	};

	class OSetObject : public BinaryOpcode
	{
	public:
		OSetObject(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetObject(a->clone(),b->clone());
		}
	};

	class OReadPODArrayR : public BinaryOpcode
	{
	public:
		OReadPODArrayR(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReadPODArrayR(a->clone(),b->clone());
		}
	};

	class OReadPODArrayI : public BinaryOpcode
	{
	public:
		OReadPODArrayI(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReadPODArrayI(a->clone(),b->clone());
		}
	};

	class OWritePODArrayRR : public BinaryOpcode
	{
	public:
		OWritePODArrayRR(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWritePODArrayRR(a->clone(),b->clone());
		}
	};

	class OWritePODArrayRI : public BinaryOpcode
	{
	public:
		OWritePODArrayRI(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWritePODArrayRI(a->clone(),b->clone());
		}
	};

	class OWritePODArrayIR : public BinaryOpcode
	{
	public:
		OWritePODArrayIR(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWritePODArrayIR(a->clone(),b->clone());
		}
	};

	class OWritePODArrayII : public BinaryOpcode
	{
	public:
		OWritePODArrayII(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWritePODArrayII(a->clone(),b->clone());
		}
	};
	class OWritePODString : public BinaryOpcode
	{
	public:
		OWritePODString(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWritePODString(a->clone(),b->clone());
		}
	};
	class OWritePODArray : public BinaryOpcode
	{
	public:
		OWritePODArray(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWritePODArray(a->clone(),b->clone());
		}
	};
	class OConstructClass : public BinaryOpcode
	{
	public:
		OConstructClass(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OConstructClass(a->clone(),b->clone());
		}
	};

	class OMarkTypeClass : public UnaryOpcode
	{
	public:
		OMarkTypeClass(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMarkTypeClass(a->clone());
		}
	};

	class OReadObject : public BinaryOpcode
	{
	public:
		OReadObject(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReadObject(a->clone(),b->clone());
		}
	};
	class OWriteObject : public BinaryOpcode
	{
	public:
		OWriteObject(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWriteObject(a->clone(),b->clone());
		}
	};
	class OFreeObject : public UnaryOpcode
	{
	public:
		OFreeObject(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFreeObject(a->clone());
		}
	};
	class OOwnObject : public UnaryOpcode
	{
	public:
		OOwnObject(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OOwnObject(a->clone());
		}
	};
	class ODestructor : public UnaryOpcode
	{
	public:
		ODestructor(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODestructor(a->clone());
		}
	};
	class OGlobalObject : public UnaryOpcode
	{
	public:
		OGlobalObject(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGlobalObject(a->clone());
		}
	};
	class OObjOwnBitmap : public BinaryOpcode
	{
	public:
		OObjOwnBitmap(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OObjOwnBitmap(a->clone(),b->clone());
		}
	};
	class OObjOwnPaldata : public BinaryOpcode
	{
	public:
		OObjOwnPaldata(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OObjOwnPaldata(a->clone(),b->clone());
		}
	};
	class OObjOwnFile : public BinaryOpcode
	{
	public:
		OObjOwnFile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OObjOwnFile(a->clone(),b->clone());
		}
	};
	class OObjOwnDir : public BinaryOpcode
	{
	public:
		OObjOwnDir(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OObjOwnDir(a->clone(),b->clone());
		}
	};
	class OObjOwnStack : public BinaryOpcode
	{
	public:
		OObjOwnStack(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OObjOwnStack(a->clone(),b->clone());
		}
	};
	class OObjOwnRNG : public BinaryOpcode
	{
	public:
		OObjOwnRNG(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OObjOwnRNG(a->clone(),b->clone());
		}
	};
	class OObjOwnClass : public BinaryOpcode
	{
	public:
		OObjOwnClass(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OObjOwnClass(a->clone(),b->clone());
		}
	};
	class OObjOwnArray : public BinaryOpcode
	{
	public:
		OObjOwnArray(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OObjOwnArray(a->clone(),b->clone());
		}
	};
	class OQuitNoDealloc : public Opcode
	{
	public:
		OQuitNoDealloc() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OQuitNoDealloc();
		}
	};
	class OSetCustomCursor : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetCustomCursor();
		}
	};
	class ONPCCanPlace : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCCanPlace();
		}
	};
	class ONPCIsFlickerFrame : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCIsFlickerFrame();
		}
	};
	class OItemGetDispName : public UnaryOpcode
	{
	public:
		OItemGetDispName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OItemGetDispName(a->clone());
		}
	};
	class OItemSetDispName : public UnaryOpcode
	{
	public:
		OItemSetDispName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OItemSetDispName(a->clone());
		}
	};
	class OItemGetShownName : public UnaryOpcode
	{
	public:
		OItemGetShownName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OItemGetShownName(a->clone());
		}
	};
	class OHeroMoveXY : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroMoveXY();
		}
	};
	class OHeroCanMoveXY : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroCanMoveXY();
		}
	};
	class OHeroLiftRelease : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroLiftRelease();
		}
	};
	class OHeroLiftGrab : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroLiftGrab();
		}
	};
	class OHeroIsFlickerFrame : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroIsFlickerFrame();
		}
	};
	class OLoadPortalRegister : public UnaryOpcode
	{
	public:
		OLoadPortalRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadPortalRegister(a->clone());
		}
	};
	class OCreatePortal : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCreatePortal();
		}
	};
	class OLoadSavPortalRegister : public UnaryOpcode
	{
	public:
		OLoadSavPortalRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadSavPortalRegister(a->clone());
		}
	};
	class OCreateSavPortal : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCreateSavPortal();
		}
	};
	class OPortalRemove : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPortalRemove();
		}
	};
	class OSavedPortalRemove : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSavedPortalRemove();
		}
	};
	class OSavedPortalGenerate : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSavedPortalGenerate();
		}
	};
	class OUseSpritePortal : public UnaryOpcode
	{
	public:
		OUseSpritePortal(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OUseSpritePortal(a->clone());
		}
	};
	class OHeroMoveAtAngle : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroMoveAtAngle();
		}
	};
	class OHeroCanMoveAtAngle : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroCanMoveAtAngle();
		}
	};
	class OHeroMove : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroMove();
		}
	};
	class OHeroCanMove : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroCanMove();
		}
	};
	class ODrawLightCircle : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawLightCircle();
		}
	};
	class ODrawLightSquare : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawLightSquare();
		}
	};
	class ODrawLightCone : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawLightCone();
		}
	};
	class OPeek : public UnaryOpcode
	{
	public:
		OPeek(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPeek(a->clone());
		}
	};
	class OPeekAtImmediate : public BinaryOpcode
	{
	public:
		OPeekAtImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPeekAtImmediate(a->clone(),b->clone());
		}
	};
	class OMakeVargArray : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMakeVargArray();
		}
	};
	class OPrintfArr : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPrintfArr();
		}
	};
	class OSPrintfArr : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSPrintfArr();
		}
	};
	class OCurrentItemID : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCurrentItemID();
		}
	};
	class OArrayPush : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArrayPush();
		}
	};
	class OArrayPop : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArrayPop();
		}
	};
	class OLoadSubscreenDataRV : public BinaryOpcode
	{
	public:
		OLoadSubscreenDataRV(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadSubscreenDataRV(a->clone(),b->clone());
		}
	};
	class OSwapSubscrV : public UnaryOpcode
	{
	public:
		OSwapSubscrV(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSwapSubscrV(a->clone());
		}
	};
	class OGetSubscreenName : public UnaryOpcode
	{
	public:
		OGetSubscreenName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetSubscreenName(a->clone());
		}
	};
	class OSetSubscreenName : public UnaryOpcode
	{
	public:
		OSetSubscreenName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetSubscreenName(a->clone());
		}
	};


	class OAddImmediate : public BinaryOpcode
	{
	public:
		OAddImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAddImmediate(a->clone(),b->clone());
		}
	};

	class OAddRegister : public BinaryOpcode
	{
	public:
		OAddRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAddRegister(a->clone(),b->clone());
		}
	};

	class OSubImmediate : public BinaryOpcode
	{
	public:
		OSubImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubImmediate(a->clone(),b->clone());
		}
	};

	class OSubImmediate2 : public BinaryOpcode
	{
	public:
		OSubImmediate2(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubImmediate2(a->clone(),b->clone());
		}
	};

	class OSubRegister : public BinaryOpcode
	{
	public:
		OSubRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubRegister(a->clone(),b->clone());
		}
	};

	class OMultImmediate : public BinaryOpcode
	{
	public:
		OMultImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMultImmediate(a->clone(),b->clone());
		}
	};

	class OMultRegister : public BinaryOpcode
	{
	public:
		OMultRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMultRegister(a->clone(),b->clone());
		}
	};

	class ODivImmediate : public BinaryOpcode
	{
	public:
		ODivImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODivImmediate(a->clone(),b->clone());
		}
	};

	class ODivImmediate2 : public BinaryOpcode
	{
	public:
		ODivImmediate2(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODivImmediate2(a->clone(),b->clone());
		}
	};

	class ODivRegister : public BinaryOpcode
	{
	public:
		ODivRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODivRegister(a->clone(),b->clone());
		}
	};

	class OCompareImmediate : public BinaryOpcode
	{
	public:
		OCompareImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCompareImmediate(a->clone(),b->clone());
		}
	};

	class OCompareImmediate2 : public BinaryOpcode
	{
	public:
		OCompareImmediate2(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCompareImmediate2(a->clone(),b->clone());
		}
	};

	class OCompareRegister : public BinaryOpcode
	{
	public:
		OCompareRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCompareRegister(a->clone(),b->clone());
		}
	};

	class OInternalStringCompare : public BinaryOpcode
	{
	public:
		OInternalStringCompare(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OInternalStringCompare(a->clone(),b->clone());
		}
	};

	class OInternalInsensitiveStringCompare : public BinaryOpcode
	{
	public:
		OInternalInsensitiveStringCompare(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OInternalInsensitiveStringCompare(a->clone(),b->clone());
		}
	};

	class OWaitframe : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWaitframe();
		}
	};
	
	class OWaitframes : public UnaryOpcode
	{
	public:
		OWaitframes(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWaitframes(a->clone());
		}
	};

	class OWaitdraw : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWaitdraw();
		}
	};
	
	class OWaitTo : public BinaryOpcode
	{
	public:
		OWaitTo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWaitTo(a->clone(),b->clone());
		}
	};
	
	class OWaitEvent : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWaitEvent();
		}
	};
	
	class ONoOp : public Opcode
	{
	public:
		ONoOp() = default;
		ONoOp(int lbl) : ONoOp()
		{
			setLabel(lbl);
		}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONoOp();
		}
	};
	
	class OCastBoolI : public UnaryOpcode
	{
	public:
		OCastBoolI(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCastBoolI(a->clone());
		}
	};
	
	class OCastBoolF : public UnaryOpcode
	{
	public:
		OCastBoolF(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCastBoolF(a->clone());
		}
	};

	class OGotoImmediate : public UnaryOpcode
	{
	public:
		OGotoImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGotoImmediate(a->clone());
		}
	};

	class OGotoTrueImmediate: public UnaryOpcode
	{
	public:
		OGotoTrueImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGotoTrueImmediate(a->clone());
		}
	};

	class OGotoFalseImmediate: public UnaryOpcode
	{
	public:
		OGotoFalseImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGotoFalseImmediate(a->clone());
		}
	};

	class OGotoMoreImmediate : public UnaryOpcode
	{
	public:
		OGotoMoreImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGotoMoreImmediate(a->clone());
		}
	};

	class OGotoLessImmediate : public UnaryOpcode
	{
	public:
		OGotoLessImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGotoLessImmediate(a->clone());
		}
	};

	class OPushRegister : public UnaryOpcode
	{
	public:
		OPushRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPushRegister(a->clone());
		}
	};

	class OPushImmediate : public UnaryOpcode
	{
	public:
		OPushImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPushImmediate(a->clone());
		}
	};

	class OPopRegister : public UnaryOpcode
	{
	public:
		OPopRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPopRegister(a->clone());
		}
	};

	class OPopArgsRegister : public BinaryOpcode
	{
	public:
		OPopArgsRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPopArgsRegister(a->clone(),b->clone());
		}
	};

	class OPushArgsRegister : public BinaryOpcode
	{
	public:
		OPushArgsRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPushArgsRegister(a->clone(),b->clone());
		}
	};

	class OPushArgsImmediate : public BinaryOpcode
	{
	public:
		OPushArgsImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPushArgsImmediate(a->clone(),b->clone());
		}
	};
	
	class OPushVargV : public UnaryOpcode
	{
	public:
		OPushVargV(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPushVargV(a->clone());
		}
	};
	
	class OPushVargR : public UnaryOpcode
	{
	public:
		OPushVargR(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPushVargR(a->clone());
		}
	};
	
	class OPushVargsV : public BinaryOpcode
	{
	public:
		OPushVargsV(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPushVargsV(a->clone(),b->clone());
		}
	};
	
	class OPushVargsR : public BinaryOpcode
	{
	public:
		OPushVargsR(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPushVargsR(a->clone(),b->clone());
		}
	};

	class OLoadIndirect : public BinaryOpcode
	{
	public:
		OLoadIndirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadIndirect(a->clone(),b->clone());
		}
	};

	class OStoreIndirect : public BinaryOpcode
	{
	public:
		OStoreIndirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStoreIndirect(a->clone(),b->clone());
		}
	};

	class OLoadDirect : public BinaryOpcode
	{
	public:
		OLoadDirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadDirect(a->clone(),b->clone());
		}
	};
	class OStoreDirect : public BinaryOpcode
	{
	public:
		OStoreDirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStoreDirect(a->clone(),b->clone());
		}
	};
	class OStoreDirectV : public BinaryOpcode
	{
	public:
		OStoreDirectV(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStoreDirectV(a->clone(),b->clone());
		}
	};

	class OLoad : public BinaryOpcode
	{
	public:
		OLoad(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoad(a->clone(),b->clone());
		}
	};
	class OStore : public BinaryOpcode
	{
	public:
		OStore(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStore(a->clone(),b->clone());
		}
	};
	class OStoreV : public BinaryOpcode
	{
	public:
		OStoreV(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStoreV(a->clone(),b->clone());
		}
	};
	class OStoreObject : public BinaryOpcode
	{
	public:
		OStoreObject(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStoreObject(a->clone(),b->clone());
		}
	};

	class OQuit : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OQuit();
		}
	};

	class OGotoRegister : public UnaryOpcode
	{
	public:
		OGotoRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGotoRegister(a->clone());
		}
	};

	class OTraceRegister : public UnaryOpcode
	{
	public:
		OTraceRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTraceRegister(a->clone());
		}
	};
	class OTraceImmediate : public UnaryOpcode
	{
	public:
		OTraceImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTraceImmediate(a->clone());
		}
	};

	class OTraceLRegister : public UnaryOpcode
	{
	public:
		OTraceLRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTraceLRegister(a->clone());
		}
	};

	class OTrace2Register : public UnaryOpcode
	{
	public:
		OTrace2Register(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTrace2Register(a->clone());
		}
	};

	class OTrace3 : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTrace3();
		}
	};

	class OTrace4 : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTrace4();
		}
	};

	class OTrace5Register : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTrace5Register();
		}
	};

	class OTrace6Register : public UnaryOpcode
	{
	public:
		OTrace6Register(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTrace6Register(a->clone());
		}
	};
	
	class OPrintfImmediate : public UnaryOpcode
	{
	public:
		OPrintfImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPrintfImmediate(a->clone());
		}
	};

	class OSPrintfImmediate : public UnaryOpcode
	{
	public:
		OSPrintfImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSPrintfImmediate(a->clone());
		}
	};

	class OPrintfVargs : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPrintfVargs();
		}
	};

	class OSPrintfVargs : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSPrintfVargs();
		}
	};

	class OBreakpoint : public UnaryOpcode
	{
	public:
		OBreakpoint(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBreakpoint(a->clone());
		}
	};

	class OAndImmediate : public BinaryOpcode
	{
	public:
		OAndImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAndImmediate(a->clone(),b->clone());
		}
	};

	class OAndRegister : public BinaryOpcode
	{
	public:
		OAndRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAndRegister(a->clone(),b->clone());
		}
	};

	class OOrImmediate : public BinaryOpcode
	{
	public:
		OOrImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OOrImmediate(a->clone(),b->clone());
		}
	};

	class OOrRegister : public BinaryOpcode
	{
	public:
		OOrRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OOrRegister(a->clone(),b->clone());
		}
	};

	class OXorImmediate : public BinaryOpcode
	{
	public:
		OXorImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OXorImmediate(a->clone(), b->clone());
		}
	};

	class OXorRegister : public BinaryOpcode
	{
	public:
		OXorRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OXorRegister(a->clone(), b->clone());
		}
	};

	class ONot : public UnaryOpcode
	{
	public:
		ONot(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONot(a->clone());
		}
	};

	class OLShiftImmediate : public BinaryOpcode
	{
	public:
		OLShiftImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLShiftImmediate(a->clone(), b->clone());
		}
	};

	class OLShiftRegister : public BinaryOpcode
	{
	public:
		OLShiftRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLShiftRegister(a->clone(), b->clone());
		}
	};

	class ORShiftImmediate : public BinaryOpcode
	{
	public:
		ORShiftImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORShiftImmediate(a->clone(), b->clone());
		}
	};

	class ORShiftRegister : public BinaryOpcode
	{
	public:
		ORShiftRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORShiftRegister(a->clone(), b->clone());
		}
	};
	
	class O32BitAndImmediate : public BinaryOpcode
	{
	public:
		O32BitAndImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitAndImmediate(a->clone(),b->clone());
		}
	};

	class O32BitAndRegister : public BinaryOpcode
	{
	public:
		O32BitAndRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitAndRegister(a->clone(),b->clone());
		}
	};

	class O32BitOrImmediate : public BinaryOpcode
	{
	public:
		O32BitOrImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitOrImmediate(a->clone(),b->clone());
		}
	};

	class O32BitOrRegister : public BinaryOpcode
	{
	public:
		O32BitOrRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitOrRegister(a->clone(),b->clone());
		}
	};

	class O32BitXorImmediate : public BinaryOpcode
	{
	public:
		O32BitXorImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitXorImmediate(a->clone(), b->clone());
		}
	};

	class O32BitXorRegister : public BinaryOpcode
	{
	public:
		O32BitXorRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitXorRegister(a->clone(), b->clone());
		}
	};

	class O32BitNot : public UnaryOpcode
	{
	public:
		O32BitNot(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitNot(a->clone());
		}
	};

	class O32BitLShiftImmediate : public BinaryOpcode
	{
	public:
		O32BitLShiftImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitLShiftImmediate(a->clone(), b->clone());
		}
	};

	class O32BitLShiftRegister : public BinaryOpcode
	{
	public:
		O32BitLShiftRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitLShiftRegister(a->clone(), b->clone());
		}
	};

	class O32BitRShiftImmediate : public BinaryOpcode
	{
	public:
		O32BitRShiftImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitRShiftImmediate(a->clone(), b->clone());
		}
	};

	class O32BitRShiftRegister : public BinaryOpcode
	{
	public:
		O32BitRShiftRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new O32BitRShiftRegister(a->clone(), b->clone());
		}
	};

	class OModuloImmediate : public BinaryOpcode
	{
	public:
		OModuloImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OModuloImmediate(a->clone(), b->clone());
		}
	};

	class OModuloImmediate2 : public BinaryOpcode
	{
	public:
		OModuloImmediate2(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OModuloImmediate2(a->clone(), b->clone());
		}
	};

	class OModuloRegister : public BinaryOpcode
	{
	public:
		OModuloRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OModuloRegister(a->clone(), b->clone());
		}
	};

	class OSinRegister : public BinaryOpcode
	{
	public:
		OSinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSinRegister(a->clone(), b->clone());
		}
	};

	class OArcSinRegister : public BinaryOpcode
	{
	public:
		OArcSinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArcSinRegister(a->clone(), b->clone());
		}
	};

	class OCosRegister : public BinaryOpcode
	{
	public:
		OCosRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCosRegister(a->clone(), b->clone());
		}
	};

	class OArcCosRegister : public BinaryOpcode
	{
	public:
		OArcCosRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArcCosRegister(a->clone(), b->clone());
		}
	};

	class OTanRegister : public BinaryOpcode
	{
	public:
		OTanRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTanRegister(a->clone(), b->clone());
		}
	};
	
	class OEngineDegtoRad : public BinaryOpcode
	{
	public:
		OEngineDegtoRad(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OEngineDegtoRad(a->clone(), b->clone());
		}
	};
	
	class OEngineRadtoDeg : public BinaryOpcode
	{
	public:
		OEngineRadtoDeg(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OEngineRadtoDeg(a->clone(), b->clone());
		}
	};
	
	class Ostrlen : public BinaryOpcode
	{
	public:
		Ostrlen(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Ostrlen(a->clone(), b->clone());
		}
	};

	class OATanRegister : public UnaryOpcode
	{
	public:
		OATanRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OATanRegister(a->clone());
		}
	};

	class OMaxRegister : public BinaryOpcode
	{
	public:
		OMaxRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMaxRegister(a->clone(), b->clone());
		}
	};

	class OMinRegister : public BinaryOpcode
	{
	public:
		OMinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMinRegister(a->clone(), b->clone());
		}
	};
	
	class OMaxNew: public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMaxNew();
		}
	};
	
	class OMinNew: public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMinNew();
		}
	};
	
	class OChoose: public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OChoose();
		}
	};

	class OPowRegister : public BinaryOpcode
	{
	public:
		OPowRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPowRegister(a->clone(), b->clone());
		}
	};
	class OPowImmediate : public BinaryOpcode
	{
	public:
		OPowImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPowImmediate(a->clone(), b->clone());
		}
	};
	class OPowImmediate2 : public BinaryOpcode
	{
	public:
		OPowImmediate2(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPowImmediate2(a->clone(), b->clone());
		}
	};

	class OLPowRegister : public BinaryOpcode
	{
	public:
		OLPowRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLPowRegister(a->clone(), b->clone());
		}
	};
	class OLPowImmediate : public BinaryOpcode
	{
	public:
		OLPowImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLPowImmediate(a->clone(), b->clone());
		}
	};
	class OLPowImmediate2 : public BinaryOpcode
	{
	public:
		OLPowImmediate2(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLPowImmediate2(a->clone(), b->clone());
		}
	};

	class OInvPowRegister : public BinaryOpcode
	{
	public:
		OInvPowRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OInvPowRegister(a->clone(), b->clone());
		}
	};

	class OFactorial : public UnaryOpcode
	{
	public:
		OFactorial(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFactorial(a->clone());
		}
	};

	class OAbsRegister : public UnaryOpcode
	{
	public:
		OAbsRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAbsRegister(a->clone());
		}
	};

	class OLog10Register : public UnaryOpcode
	{
	public:
		OLog10Register(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLog10Register(a->clone());
		}
	};

	class OLogERegister : public UnaryOpcode
	{
	public:
		OLogERegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLogERegister(a->clone());
		}
	};

	class OArraySize : public UnaryOpcode
	{
	public:
		OArraySize(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArraySize(a->clone());
		}
	};


	class OArraySizeF : public UnaryOpcode
	{
	public:
		OArraySizeF(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArraySizeF(a->clone());
		}
	};
	class OArraySizeN : public UnaryOpcode
	{
	public:
		OArraySizeN(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArraySizeN(a->clone());
		}
	};
	class OArraySizeE : public UnaryOpcode
	{
	public:
		OArraySizeE(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArraySizeE(a->clone());
		}
	};
	class OArraySizeL : public UnaryOpcode
	{
	public:
		OArraySizeL(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArraySizeL(a->clone());
		}
	};
	class OArraySizeB : public UnaryOpcode
	{
	public:
		OArraySizeB(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArraySizeB(a->clone());
		}
	};
	class OArraySizeI : public UnaryOpcode
	{
	public:
		OArraySizeI(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArraySizeI(a->clone());
		}
	};
	class OArraySizeID : public UnaryOpcode
	{
	public:
		OArraySizeID(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArraySizeID(a->clone());
		}
	};

	class OCheckTrig : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCheckTrig();
		}
	};

	class ORandRegister : public BinaryOpcode
	{
	public:
		ORandRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORandRegister(a->clone(), b->clone());
		}
	};
	
	class OSRandRegister : public UnaryOpcode
	{
	public:
		OSRandRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSRandRegister(a->clone());
		}
	};
	
	class OSRandImmediate : public UnaryOpcode
	{
	public:
		OSRandImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSRandImmediate(a->clone());
		}
	};
	
	class OSRandRand : public UnaryOpcode
	{
	public:
		OSRandRand(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSRandRand(a->clone());
		}
	};
	
	class ORNGRand1 : public Opcode
	{
	public:
		ORNGRand1() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORNGRand1();
		}
	};
	
	class ORNGRand2 : public UnaryOpcode
	{
	public:
		ORNGRand2(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORNGRand2(a->clone());
		}
	};

	class ORNGRand3 : public BinaryOpcode
	{
	public:
		ORNGRand3(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORNGRand3(a->clone(), b->clone());
		}
	};
	
	class ORNGLRand1 : public Opcode
	{
	public:
		ORNGLRand1() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORNGLRand1();
		}
	};
	
	class ORNGLRand2 : public UnaryOpcode
	{
	public:
		ORNGLRand2(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORNGLRand2(a->clone());
		}
	};

	class ORNGLRand3 : public BinaryOpcode
	{
	public:
		ORNGLRand3(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORNGLRand3(a->clone(), b->clone());
		}
	};
	
	class ORNGSeed : public UnaryOpcode
	{
	public:
		ORNGSeed(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORNGSeed(a->clone());
		}
	};
	
	class ORNGRSeed : public Opcode
	{
	public:
		ORNGRSeed() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORNGRSeed();
		}
	};
	
	class ORNGFree : public Opcode
	{
	public:
		ORNGFree() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORNGFree();
		}
	};
	
	class OSqrtRegister : public BinaryOpcode
	{
	public:
		OSqrtRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSqrtRegister(a->clone(),b->clone());
		}
	};

	class OWarp : public BinaryOpcode
	{
	public:
		OWarp(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWarp(a->clone(), b->clone());
		}
	};

	class OPitWarp : public BinaryOpcode
	{
	public:
		OPitWarp(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPitWarp(a->clone(), b->clone());
		}
	};

	class OCreateItemRegister : public UnaryOpcode
	{
	public:
		OCreateItemRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCreateItemRegister(a->clone());
		}
	};

	class OCreateNPCRegister : public UnaryOpcode
	{
	public:
		OCreateNPCRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCreateNPCRegister(a->clone());
		}
	};

	class OCreateLWpnRegister : public UnaryOpcode
	{
	public:
		OCreateLWpnRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCreateLWpnRegister(a->clone());
		}
	};

	class OCreateEWpnRegister : public UnaryOpcode
	{
	public:
		OCreateEWpnRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCreateEWpnRegister(a->clone());
		}
	};

	class OLoadItemRegister : public UnaryOpcode
	{
	public:
		OLoadItemRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadItemRegister(a->clone());
		}
	};

	class OLoadItemDataRegister : public UnaryOpcode
	{
	public:
		OLoadItemDataRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadItemDataRegister(a->clone());
		}
	};

	class OLoadShopDataRegister : public UnaryOpcode
	{
	public:
		OLoadShopDataRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadShopDataRegister(a->clone());
		}
	};


	class OLoadInfoShopDataRegister : public UnaryOpcode
	{
	public:
		OLoadInfoShopDataRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadInfoShopDataRegister(a->clone());
		}
	};

	class OLoadNPCDataRegister : public UnaryOpcode
	{
	public:
		OLoadNPCDataRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadNPCDataRegister(a->clone());
		}
	};


	class OLoadMessageDataRegister : public UnaryOpcode
	{
	public:
		OLoadMessageDataRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadMessageDataRegister(a->clone());
		}
	};


	class OLoadDMapDataRegister : public UnaryOpcode
	{
	public:
		OLoadDMapDataRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadDMapDataRegister(a->clone());
		}
	};

	class OLoadStack : public Opcode
	{
	public:
		OLoadStack() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadStack();
		}
	};

	class OLoadDirectoryRegister : public UnaryOpcode
	{
	public:
		OLoadDirectoryRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadDirectoryRegister(a->clone());
		}
	};

	class OLoadRNG : public Opcode
	{
	public:
		OLoadRNG() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadRNG();
		}
	};

	class OCreatePalData : public Opcode
	{
	public:
		OCreatePalData() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCreatePalData();
		}
	};
	
	class OCreatePalDataClr : public UnaryOpcode
	{
	public:
		OCreatePalDataClr(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCreatePalDataClr(a->clone());
		}
	};

	class OCreateRGBHex : public UnaryOpcode
	{
	public:
		OCreateRGBHex(Argument* A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCreateRGBHex(a->clone());
		}
	};

	class OCreateRGB : public Opcode
	{
	public:
		OCreateRGB() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCreateRGB();
		}
	};

	class OConvertFromRGB : public Opcode
	{
	public:
		OConvertFromRGB() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OConvertFromRGB();
		}
	};

	class OConvertToRGB : public Opcode
	{
	public:
		OConvertToRGB() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OConvertToRGB();
		}
	};

	class OGetTilePixel : public Opcode
	{
	public:
		OGetTilePixel() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetTilePixel();
		}
	};

	class OSetTilePixel : public Opcode
	{
	public:
		OSetTilePixel() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetTilePixel();
		}
	};
	
	class OMixColorArray : public Opcode
	{
	public:
		OMixColorArray() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMixColorArray();
		}
	};
	
	class OLoadLevelPalette : public UnaryOpcode
	{
	public:
		OLoadLevelPalette(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadLevelPalette(a->clone());
		}
	};

	class OLoadSpritePalette : public UnaryOpcode
	{
	public:
		OLoadSpritePalette(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadSpritePalette(a->clone());
		}
	};

	class OLoadMainPalette : public Opcode
	{
	public:
		OLoadMainPalette() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadMainPalette();
		}
	};

	class OLoadCyclePalette : public UnaryOpcode
	{
	public:
		OLoadCyclePalette(Argument* A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadCyclePalette(a->clone());
		}
	};

	class OLoadBitmapPalette : public UnaryOpcode
	{
	public:
		OLoadBitmapPalette(Argument* A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadBitmapPalette(a->clone());
		}
	};
	
	class OWriteLevelPalette : public UnaryOpcode
	{
	public:
		OWriteLevelPalette(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWriteLevelPalette(a->clone());
		}
	};
	
	class OWriteLevelCSet : public BinaryOpcode
	{
	public:
		OWriteLevelCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWriteLevelCSet(a->clone(), b->clone());
		}
	};
	
	class OWriteSpritePalette : public UnaryOpcode
	{
	public:
		OWriteSpritePalette(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWriteSpritePalette(a->clone());
		}
	};
	
	class OWriteSpriteCSet : public BinaryOpcode
	{
	public:
		OWriteSpriteCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWriteSpriteCSet(a->clone(), b->clone());
		}
	};
	
	class OWriteMainPalette : public Opcode
	{
	public:
		OWriteMainPalette() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWriteMainPalette();
		}
	};
	
	class OWriteMainCSet : public UnaryOpcode
	{
	public:
		OWriteMainCSet(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWriteMainCSet(a->clone());
		}
	};

	class OWriteCyclePalette : public UnaryOpcode
	{
	public:
		OWriteCyclePalette(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWriteCyclePalette(a->clone());
		}
	};

	class OWriteCycleCSet : public BinaryOpcode
	{
	public:
		OWriteCycleCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWriteCycleCSet(a->clone(), b->clone());
		}
	};
	
	class OPalDataColorValid : public UnaryOpcode
	{
	public:
		OPalDataColorValid(Argument* A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPalDataColorValid(a->clone());
		}
	};
	
	class OPalDataClearColor : public UnaryOpcode
	{
	public:
		OPalDataClearColor(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPalDataClearColor(a->clone());
		}
	};

	class OPalDataClearCSet : public UnaryOpcode
	{
	public:
		OPalDataClearCSet(Argument* A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPalDataClearCSet(a->clone());
		}
	};

	class OPalDataMix : public Opcode
	{
	public:
		OPalDataMix() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPalDataMix();
		}
	};
	
	class OPalDataMixCSet : public Opcode
	{
	public:
		OPalDataMixCSet() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPalDataMixCSet();
		}
	};
	
	class OPalDataCopy : public UnaryOpcode
	{
	public:
		OPalDataCopy(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPalDataCopy(a->clone());
		}
	};

	class OPalDataCopyCSet : public Opcode
	{
	public:
		OPalDataCopyCSet() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPalDataCopyCSet();
		}
	};
	
	class OLoadDropsetRegister : public UnaryOpcode
	{
	public:
		OLoadDropsetRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadDropsetRegister(a->clone());
		}
	};

	class OGetBottleShopName : public UnaryOpcode
	{
	public:
		OGetBottleShopName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetBottleShopName(a->clone());
		}
	};

	class OSetBottleShopName : public UnaryOpcode
	{
	public:
		OSetBottleShopName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetBottleShopName(a->clone());
		}
	};

	class OGetBottleName : public UnaryOpcode
	{
	public:
		OGetBottleName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetBottleName(a->clone());
		}
	};

	class OSetBottleName : public UnaryOpcode
	{
	public:
		OSetBottleName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetBottleName(a->clone());
		}
	};

	class OLoadBottleTypeRegister : public UnaryOpcode
	{
	public:
		OLoadBottleTypeRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadBottleTypeRegister(a->clone());
		}
	};

	class OLoadBShopRegister : public UnaryOpcode
	{
	public:
		OLoadBShopRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadBShopRegister(a->clone());
		}
	};

	class OLoadGenericDataR : public UnaryOpcode
	{
	public:
		OLoadGenericDataR(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadGenericDataR(a->clone());
		}
	};

	class ODMapDataGetNameRegister : public UnaryOpcode
	{
	public:
		ODMapDataGetNameRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODMapDataGetNameRegister(a->clone());
		}
	};

	class ODMapDataSetNameRegister : public UnaryOpcode
	{
	public:
		ODMapDataSetNameRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODMapDataSetNameRegister(a->clone());
		}
	};

	class ODMapDataGetTitleRegister : public UnaryOpcode
	{
	public:
		ODMapDataGetTitleRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODMapDataGetTitleRegister(a->clone());
		}
	};

	class ODMapDataSetTitleRegister : public UnaryOpcode
	{
	public:
		ODMapDataSetTitleRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODMapDataSetTitleRegister(a->clone());
		}
	};

	class ODMapDataGetIntroRegister : public UnaryOpcode
	{
	public:
		ODMapDataGetIntroRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODMapDataGetIntroRegister(a->clone());
		}
	};

	class ODMapDataSetIntroRegister : public UnaryOpcode
	{
	public:
		ODMapDataSetIntroRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODMapDataSetIntroRegister(a->clone());
		}
	};

	class ODMapDataGetMusicRegister : public UnaryOpcode
	{
	public:
		ODMapDataGetMusicRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODMapDataGetMusicRegister(a->clone());
		}
	};

	class ODMapDataSetMusicRegister : public UnaryOpcode
	{
	public:
		ODMapDataSetMusicRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODMapDataSetMusicRegister(a->clone());
		}
	};


	class OMessageDataSetStringRegister : public UnaryOpcode
	{
	public:
		OMessageDataSetStringRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMessageDataSetStringRegister(a->clone());
		}
	};


	class OMessageDataGetStringRegister : public UnaryOpcode
	{
	public:
		OMessageDataGetStringRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMessageDataGetStringRegister(a->clone());
		}
	};

	class OLoadComboDataRegister : public UnaryOpcode
	{
	public:
		OLoadComboDataRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadComboDataRegister(a->clone());
		}
	};

	class OLoadMapDataRegister : public BinaryOpcode
	{
	public:
		OLoadMapDataRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadMapDataRegister(a->clone(), b->clone());
		}
	};


	class OLoadSpriteDataRegister : public UnaryOpcode
	{
	public:
		OLoadSpriteDataRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadSpriteDataRegister(a->clone());
		}
	};


	class OLoadScreenDataRegister : public UnaryOpcode
	{
	public:
		OLoadScreenDataRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadScreenDataRegister(a->clone());
		}
	};


	class OLoadBitmapDataRegister : public UnaryOpcode
	{
	public:
		OLoadBitmapDataRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadBitmapDataRegister(a->clone());
		}
	};

	class OLoadNPCRegister : public UnaryOpcode
	{
	public:
		OLoadNPCRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadNPCRegister(a->clone());
		}
	};

	class OLoadLWpnRegister : public UnaryOpcode
	{
	public:
		OLoadLWpnRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadLWpnRegister(a->clone());
		}
	};

	class OLoadEWpnRegister : public UnaryOpcode
	{
	public:
		OLoadEWpnRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadEWpnRegister(a->clone());
		}
	};

	class OPlaySoundRegister : public UnaryOpcode
	{
	public:
		OPlaySoundRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPlaySoundRegister(a->clone());
		}
	};

	// Audio->


	class OAdjustVolumeRegister : public UnaryOpcode
	{
	public:
		OAdjustVolumeRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAdjustVolumeRegister(a->clone());
		}
	};


	class OAdjustSFXVolumeRegister : public UnaryOpcode
	{
	public:
		OAdjustSFXVolumeRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAdjustSFXVolumeRegister(a->clone());
		}
	};

	class OAdjustSound : public Opcode
	{
	public:
		OAdjustSound() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAdjustSound();
		}
	};

	class OPlaySoundEX : public Opcode
	{
	public:
		OPlaySoundEX() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPlaySoundEX();
		}
	};

	class OGetSoundCompletion : public UnaryOpcode
	{
	public:
		OGetSoundCompletion(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetSoundCompletion(a->clone());
		}
	};

	class OEndSoundRegister : public UnaryOpcode
	{
	public:
		OEndSoundRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OEndSoundRegister(a->clone());
		}
	};


	class OPauseSoundRegister : public UnaryOpcode
	{
	public:
		OPauseSoundRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPauseSoundRegister(a->clone());
		}
	};


	class OResumeSoundRegister : public UnaryOpcode
	{
	public:
		OResumeSoundRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OResumeSoundRegister(a->clone());
		}
	};


	class OPauseSFX : public UnaryOpcode
	{
	public:
		OPauseSFX(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPauseSFX(a->clone());
		}
	};

	class OResumeSFX : public UnaryOpcode
	{
	public:
		OResumeSFX(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OResumeSFX(a->clone());
		}
	};

	class OContinueSFX : public UnaryOpcode
	{
	public:
		OContinueSFX(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OContinueSFX(a->clone());
		}
	};


	class OPauseMusic : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPauseMusic();
		}
	};

	class OResumeMusic : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OResumeMusic();
		}
	};


	//END Audio

	class OPlayMIDIRegister : public UnaryOpcode
	{
	public:
		OPlayMIDIRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPlayMIDIRegister(a->clone());
		}
	};

	class OPlayEnhancedMusic : public BinaryOpcode
	{
	public:
		OPlayEnhancedMusic(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPlayEnhancedMusic(a->clone(), b->clone());
		}
	};
	
	class OPlayEnhancedMusicEx : public BinaryOpcode
	{
	public:
		OPlayEnhancedMusicEx(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPlayEnhancedMusicEx(a->clone(), b->clone());
		}
	};
	
	class OGetEnhancedMusicPos : public UnaryOpcode
	{
	public:
		OGetEnhancedMusicPos(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetEnhancedMusicPos(a->clone());
		}
	};
	
	class OSetEnhancedMusicPos : public UnaryOpcode
	{
	public:
		OSetEnhancedMusicPos(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetEnhancedMusicPos(a->clone());
		}
	};
	
	class OSetEnhancedMusicSpeed : public UnaryOpcode
	{
	public:
		OSetEnhancedMusicSpeed(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetEnhancedMusicSpeed(a->clone());
		}
	};

	class OGetEnhancedMusicLength : public UnaryOpcode
	{
	public:
		OGetEnhancedMusicLength(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetEnhancedMusicLength(a->clone());
		}
	};

	class OSetEnhancedMusicLoop : public BinaryOpcode
	{
	public:
		OSetEnhancedMusicLoop(Argument* A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetEnhancedMusicLoop(a->clone(), b->clone());
		}
	};

	class OCrossfadeEnhancedMusic : public Opcode
	{
	public:
		OCrossfadeEnhancedMusic() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCrossfadeEnhancedMusic();
		}
	};

	class OGetDMapMusicFilename : public BinaryOpcode
	{
	public:
		OGetDMapMusicFilename(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetDMapMusicFilename(a->clone(), b->clone());
		}
	};
	
	class OGetNPCDataInitDLabel : public BinaryOpcode
	{
	public:
		OGetNPCDataInitDLabel(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetNPCDataInitDLabel(a->clone(), b->clone());
		}
	};

	class OGetDMapMusicTrack : public UnaryOpcode
	{
	public:
		OGetDMapMusicTrack(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetDMapMusicTrack(a->clone());
		}
	};

	class OSetDMapEnhancedMusic : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetDMapEnhancedMusic();
		}
	};

	class OGetSaveName : public UnaryOpcode
	{
	public:
		OGetSaveName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetSaveName(a->clone());
		}
	};

	class OGetDMapName : public BinaryOpcode
	{
	public:
		OGetDMapName(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetDMapName(a->clone(), b->clone());
		}
	};

	class OSetDMapName : public BinaryOpcode
	{
	public:
		OSetDMapName(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetDMapName(a->clone(), b->clone());
		}
	};

	class OSetDMapIntro : public BinaryOpcode
	{
	public:
		OSetDMapIntro(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetDMapIntro(a->clone(), b->clone());
		}
	};

	class OSetDMapTitle : public BinaryOpcode
	{
	public:
		OSetDMapTitle(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetDMapTitle(a->clone(), b->clone());
		}
	};


	class OSetMessage : public BinaryOpcode
	{
	public:
		OSetMessage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetMessage(a->clone(), b->clone());
		}
	};


	class OGetDMapIntro : public BinaryOpcode
	{
	public:
		OGetDMapIntro(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetDMapIntro(a->clone(), b->clone());
		}
	};

	class OGetDMapTitle : public BinaryOpcode
	{
	public:
		OGetDMapTitle(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetDMapTitle(a->clone(), b->clone());
		}
	};

	class OSetSaveName : public UnaryOpcode
	{
	public:
		OSetSaveName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetSaveName(a->clone());
		}
	};

	class OGetItemName : public UnaryOpcode
	{
	public:
		OGetItemName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetItemName(a->clone());
		}
	};

	class OGetNPCName : public UnaryOpcode
	{
	public:
		OGetNPCName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetNPCName(a->clone());
		}
	};

	class OGetMessage : public BinaryOpcode
	{
	public:
		OGetMessage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetMessage(a->clone(), b->clone());
		}
	};

	class OClearSpritesRegister : public UnaryOpcode
	{
	public:
		OClearSpritesRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OClearSpritesRegister(a->clone());
		}
	};

	class OMessageRegister : public UnaryOpcode
	{
	public:
		OMessageRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMessageRegister(a->clone());
		}
	};

	class OIsSolid : public UnaryOpcode
	{
	public:
		OIsSolid(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsSolid(a->clone());
		}
	};

	class OIsSolidMapdata : public UnaryOpcode
	{
	public:
		OIsSolidMapdata(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsSolidMapdata(a->clone());
		}
	};

	class OIsSolidMapdataLayer : public UnaryOpcode
	{
	public:
		OIsSolidMapdataLayer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsSolidMapdataLayer(a->clone());
		}
	};

	class OIsSolidLayer : public UnaryOpcode
	{
	public:
		OIsSolidLayer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsSolidLayer(a->clone());
		}
	};

	class OLoadTmpScr : public UnaryOpcode
	{
	public:
		OLoadTmpScr(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadTmpScr(a->clone());
		}
	};

	class OLoadScrollScr : public UnaryOpcode
	{
	public:
		OLoadScrollScr(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadScrollScr(a->clone());
		}
	};

	class OSetSideWarpRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetSideWarpRegister();
		}
	};

	class OSetTileWarpRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetTileWarpRegister();
		}
	};

	class OGetSideWarpDMap : public UnaryOpcode
	{
	public:
		OGetSideWarpDMap(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetSideWarpDMap(a->clone());
		}
	};

	class OGetSideWarpScreen : public UnaryOpcode
	{
	public:
		OGetSideWarpScreen(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetSideWarpScreen(a->clone());
		}
	};

	class OGetSideWarpType : public UnaryOpcode
	{
	public:
		OGetSideWarpType(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetSideWarpType(a->clone());
		}
	};

	class OGetTileWarpDMap : public UnaryOpcode
	{
	public:
		OGetTileWarpDMap(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetTileWarpDMap(a->clone());
		}
	};

	class OGetTileWarpScreen : public UnaryOpcode
	{
	public:
		OGetTileWarpScreen(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetTileWarpScreen(a->clone());
		}
	};

	class OGetTileWarpType : public UnaryOpcode
	{
	public:
		OGetTileWarpType(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetTileWarpType(a->clone());
		}
	};

	class OLayerScreenRegister : public BinaryOpcode
	{
	public:
		OLayerScreenRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLayerScreenRegister(a->clone(), b->clone());
		}
	};

	class OLayerMapRegister : public BinaryOpcode
	{
	public:
		OLayerMapRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLayerMapRegister(a->clone(), b->clone());
		}
	};

	class OTriggerSecrets : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTriggerSecrets();
		}
	};

	class OTriggerSecretsFor : public UnaryOpcode
	{
	public:
		OTriggerSecretsFor(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode *clone() const
		{
			return new OTriggerSecretsFor(a->clone());
		}
	};

	class OIsValidArray : public UnaryOpcode
	{
	public:
		OIsValidArray(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsValidArray(a->clone());
		}
	};

	class OIsValidItem : public UnaryOpcode
	{
	public:
		OIsValidItem(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsValidItem(a->clone());
		}
	};

	class OIsValidNPC : public UnaryOpcode
	{
	public:
		OIsValidNPC(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsValidNPC(a->clone());
		}
	};

	class OIsValidLWpn : public UnaryOpcode
	{
	public:
		OIsValidLWpn(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsValidLWpn(a->clone());
		}
	};

	class OIsValidEWpn : public UnaryOpcode
	{
	public:
		OIsValidEWpn(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsValidEWpn(a->clone());
		}
	};
	
	class OMakeAngularLwpn : public UnaryOpcode
	{
	public:
		OMakeAngularLwpn(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMakeAngularLwpn(a->clone());
		}
	};
	
	class OMakeAngularEwpn : public UnaryOpcode
	{
	public:
		OMakeAngularEwpn(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMakeAngularEwpn(a->clone());
		}
	};
	
	class OMakeDirectionalLwpn : public UnaryOpcode
	{
	public:
		OMakeDirectionalLwpn(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMakeDirectionalLwpn(a->clone());
		}
	};
	
	class OMakeDirectionalEwpn : public UnaryOpcode
	{
	public:
		OMakeDirectionalEwpn(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMakeDirectionalEwpn(a->clone());
		}
	};

	class OUseSpriteLWpn : public UnaryOpcode
	{
	public:
		OUseSpriteLWpn(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OUseSpriteLWpn(a->clone());
		}
	};

	class OUseSpriteEWpn : public UnaryOpcode
	{
	public:
		OUseSpriteEWpn(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OUseSpriteEWpn(a->clone());
		}
	};

	class ORectangleRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORectangleRegister();
		}
	};

	class OFrameRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFrameRegister();
		}
	};

	class OCircleRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCircleRegister();
		}
	};

	class OArcRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OArcRegister();
		}
	};

	class OEllipseRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OEllipseRegister();
		}
	};

	class OLineRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLineRegister();
		}
	};

	class OSplineRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSplineRegister();
		}
	};

	class OPutPixelRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPutPixelRegister();
		}
	};
	
	class OPutPixelArrayRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPutPixelArrayRegister();
		}
	};
	
	class OPutTileArrayRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPutTileArrayRegister();
		}
	};
	
	class OPutLinesArrayRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPutLinesArrayRegister();
		}
	};
	
	class OFastComboArrayRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFastComboArrayRegister();
		}
	};

	class ODrawCharRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawCharRegister();
		}
	};

	class ODrawIntRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawIntRegister();
		}
	};

	class ODrawTileRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawTileRegister();
		}
	};

	class ODrawTileCloakedRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawTileCloakedRegister();
		}
	};

	class ODrawComboRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawComboRegister();
		}
	};

	class ODrawComboCloakedRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawComboCloakedRegister();
		}
	};

	class OQuadRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OQuadRegister();
		}
	};

	class OTriangleRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTriangleRegister();
		}
	};

	class OQuad3DRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OQuad3DRegister();
		}
	};

	class OTriangle3DRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTriangle3DRegister();
		}
	};

	class OFastTileRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFastTileRegister();
		}
	};

	class OFastComboRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFastComboRegister();
		}
	};

	class ODrawStringRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawStringRegister();
		}
	};

	class ODrawString2Register : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawString2Register();
		}
	};

	class ODrawLayerRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawLayerRegister();
		}
	};

	class ODrawScreenRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawScreenRegister();
		}
	};

	class ODrawBitmapRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawBitmapRegister();
		}
	};


	class ODrawBitmapExRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODrawBitmapExRegister();
		}
	};

	class OSetRenderTargetRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetRenderTargetRegister();
		}
	};

	class OSetDepthBufferRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetDepthBufferRegister();
		}
	};

	class OGetDepthBufferRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetDepthBufferRegister();
		}
	};

	class OSetColorBufferRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetColorBufferRegister();
		}
	};

	class OGetColorBufferRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetColorBufferRegister();
		}
	};

	class OCopyTileRegister : public BinaryOpcode
	{
	public:
		OCopyTileRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCopyTileRegister(a->clone(),b->clone());
		}
	};
	
	class Ostrcpy : public BinaryOpcode
	{
	public:
		Ostrcpy(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Ostrcpy(a->clone(),b->clone());
		}
	};
	
	class oARRAYCOPY : public BinaryOpcode
	{
	public:
		oARRAYCOPY(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new oARRAYCOPY(a->clone(),b->clone());
		}
	};


	class OOverlayTileRegister : public BinaryOpcode
	{
	public:
		OOverlayTileRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OOverlayTileRegister(a->clone(),b->clone());
		}
	};

	class OSwapTileRegister : public BinaryOpcode
	{
	public:
		OSwapTileRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSwapTileRegister(a->clone(),b->clone());
		}
	};

	class OClearTileRegister : public UnaryOpcode
	{
	public:
		OClearTileRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OClearTileRegister(a->clone());
		}
	};

	class OAllocateMemRegister : public BinaryOpcode
	{
	public:
		OAllocateMemRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAllocateMemRegister(a->clone(),b->clone());
		}
	};

	class OAllocateMemImmediate : public TernaryOpcode
	{
	public:
		OAllocateMemImmediate(Argument *A, Argument *B, Argument* C = new LiteralArgument(0)) : TernaryOpcode(A,B,C) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAllocateMemImmediate(a->clone(),b->clone(),c->clone());
		}
	};

	class OAllocateGlobalMemImmediate : public TernaryOpcode
	{
	public:
		OAllocateGlobalMemImmediate(Argument *A, Argument* B, Argument* C = new LiteralArgument(0)) : TernaryOpcode(A,B,C) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAllocateGlobalMemImmediate(a->clone(),b->clone(),c->clone());
		}
	};

	class OAllocateGlobalMemRegister : public BinaryOpcode
	{
	public:
		OAllocateGlobalMemRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAllocateGlobalMemRegister(a->clone(),b->clone());
		}
	};

	class ODeallocateMemRegister : public UnaryOpcode
	{
	public:
		ODeallocateMemRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODeallocateMemRegister(a->clone());
		}
	};

	class ODeallocateMemImmediate : public UnaryOpcode
	{
	public:
		ODeallocateMemImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODeallocateMemImmediate(a->clone());
		}
	};

	class OResizeArrayRegister : public BinaryOpcode
	{
	public:
		OResizeArrayRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OResizeArrayRegister(a->clone(), b->clone());
		}
	};
	class OOwnArrayRegister : public UnaryOpcode
	{
	public:
		OOwnArrayRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OOwnArrayRegister(a->clone());
		}
	};
	class ODestroyArrayRegister : public UnaryOpcode
	{
	public:
		ODestroyArrayRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODestroyArrayRegister(a->clone());
		}
	};

	class OSave : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSave();
		}
	};

	class OGetScreenFlags : public UnaryOpcode
	{
	public:
		OGetScreenFlags(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenFlags(a->clone());
		}
	};

	class OGetScreenEFlags : public UnaryOpcode
	{
	public:
		OGetScreenEFlags(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenEFlags(a->clone());
		}
	};

	class OEnd : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OEnd();
		}
	};

	class OGameReload : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGameReload();
		}
	};
	
	class OGameContinue : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGameContinue();
		}
	};
	
	class OGameSaveQuit : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGameSaveQuit();
		}
	};
	
	class OGameSaveContinue : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGameSaveContinue();
		}
	};

	class OShowF6Screen : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OShowF6Screen();
		}
	};

	class OComboTile : public BinaryOpcode
	{
	public:
		OComboTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OComboTile(a->clone(), b->clone());
		}
	};

	class OBreakShield : public UnaryOpcode
	{
	public:
		OBreakShield(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBreakShield(a->clone());
		}
	};

	class OShowSaveScreen : public UnaryOpcode
	{
	public:
		OShowSaveScreen(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OShowSaveScreen(a->clone());
		}
	};

	class OShowSaveQuitScreen : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OShowSaveQuitScreen();
		}
	};

	class OSelectAWeaponRegister : public UnaryOpcode
	{
	public:
		OSelectAWeaponRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSelectAWeaponRegister(a->clone());
		}
	};

	class OSelectBWeaponRegister : public UnaryOpcode
	{
	public:
		OSelectBWeaponRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSelectBWeaponRegister(a->clone());
		}
	};

	class OSelectXWeaponRegister : public UnaryOpcode
	{
	public:
		OSelectXWeaponRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSelectXWeaponRegister(a->clone());
		}
	};

	class OSelectYWeaponRegister : public UnaryOpcode
	{
	public:
		OSelectYWeaponRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSelectYWeaponRegister(a->clone());
		}
	};

	class OGetFFCScript : public UnaryOpcode
	{
	public:
		OGetFFCScript(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetFFCScript(a->clone());
		}
	};
	
	class OGetComboScript : public UnaryOpcode
	{
	public:
		OGetComboScript(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetComboScript(a->clone());
		}
	};

	class OGetItemScript : public UnaryOpcode
	{
	public:
		OGetItemScript(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetItemScript(a->clone());
		}
	};



	//2,54

	class OZapIn : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OZapIn();
		}
	};

	class OZapOut : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OZapOut();
		}
	};


	class OGreyscaleOn : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGreyscaleOn();
		}
	};

	class OGreyscaleOff : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGreyscaleOff();
		}
	};


	//These need to be unary opcodes that accept bool linkvisible. 
	class OWavyIn : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWavyIn();
		}
	};


	class OWavyOut : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWavyOut();
		}
	};

	class OOpenWipe : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OOpenWipe();
		}
	};

	class OCloseWipe : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCloseWipe();
		}
	};

	class OOpenWipeShape : public UnaryOpcode
	{
	public:
		OOpenWipeShape(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OOpenWipeShape(a->clone());
		}
	};
	
	class OCloseWipeShape : public UnaryOpcode
	{
	public:
		OCloseWipeShape(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCloseWipeShape(a->clone());
		}
	};

	class OGetFFCPointer : public UnaryOpcode
	{
	public:
		OGetFFCPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetFFCPointer(a->clone());
		}
	};

	class OSetFFCPointer : public UnaryOpcode
	{
	public:
		OSetFFCPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetFFCPointer(a->clone());
		}
	};


	class OGetNPCPointer : public UnaryOpcode
	{
	public:
		OGetNPCPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetNPCPointer(a->clone());
		}
	};


	class OSetNPCPointer : public UnaryOpcode
	{
	public:
		OSetNPCPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetNPCPointer(a->clone());
		}
	};


	class OGetLWeaponPointer : public UnaryOpcode
	{
	public:
		OGetLWeaponPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetLWeaponPointer(a->clone());
		}
	};


	class OSetLWeaponPointer : public UnaryOpcode
	{
	public:
		OSetLWeaponPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetLWeaponPointer(a->clone());
		}
	};


	class OGetEWeaponPointer : public UnaryOpcode
	{
	public:
		OGetEWeaponPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetEWeaponPointer(a->clone());
		}
	};


	class OSetEWeaponPointer : public UnaryOpcode
	{
	public:
		OSetEWeaponPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetEWeaponPointer(a->clone());
		}
	};


	class OGetItemPointer : public UnaryOpcode
	{
	public:
		OGetItemPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetItemPointer(a->clone());
		}
	};


	class OSetItemPointer : public UnaryOpcode
	{
	public:
		OSetItemPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetItemPointer(a->clone());
		}
	};


	class OGetItemDataPointer : public UnaryOpcode
	{
	public:
		OGetItemDataPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetItemDataPointer(a->clone());
		}
	};


	class OSetItemDataPointer : public UnaryOpcode
	{
	public:
		OSetItemDataPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetItemDataPointer(a->clone());
		}
	};


	class OGetBoolPointer : public UnaryOpcode
	{
	public:
		OGetBoolPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetBoolPointer(a->clone());
		}
	};


	class OSetBoolPointer : public UnaryOpcode
	{
	public:
		OSetBoolPointer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetBoolPointer(a->clone());
		}
	};


	class OGetScreenDoor : public UnaryOpcode
	{
	public:
		OGetScreenDoor(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenDoor(a->clone());
		}
	};


	class OGetScreenEnemy : public UnaryOpcode
	{
	public:
		OGetScreenEnemy(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenEnemy(a->clone());
		}
	};

	class OGetScreenLayerOpacity : public UnaryOpcode
	{
	public:
		OGetScreenLayerOpacity(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenLayerOpacity(a->clone());
		}
	};
	class OGetScreenSecretCombo : public UnaryOpcode
	{
	public:
		OGetScreenSecretCombo(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenSecretCombo(a->clone());
		}
	};
	class OGetScreenSecretCSet : public UnaryOpcode
	{
	public:
		OGetScreenSecretCSet(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenSecretCSet(a->clone());
		}
	};
	class OGetScreenSecretFlag : public UnaryOpcode
	{
	public:
		OGetScreenSecretFlag(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenSecretFlag(a->clone());
		}
	};
	class OGetScreenLayerMap : public UnaryOpcode
	{
	public:
		OGetScreenLayerMap(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenLayerMap(a->clone());
		}
	};
	class OGetScreenLayerScreen : public UnaryOpcode
	{
	public:
		OGetScreenLayerScreen(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenLayerScreen(a->clone());
		}
	};
	class OGetScreenPath : public UnaryOpcode
	{
	public:
		OGetScreenPath(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenPath(a->clone());
		}
	};
	class OGetScreenWarpReturnX : public UnaryOpcode
	{
	public:
		OGetScreenWarpReturnX(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenWarpReturnX(a->clone());
		}
	};
	class OGetScreenWarpReturnY : public UnaryOpcode
	{
	public:
		OGetScreenWarpReturnY(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetScreenWarpReturnY(a->clone());
		}
	};

	class OTriggerSecretRegister : public UnaryOpcode
	{
	public:
		OTriggerSecretRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTriggerSecretRegister(a->clone());
		}
	};

	class OPolygonRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPolygonRegister();
		}
	};

	class OBMPPolygonRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPPolygonRegister();
		}
	};
	
	class ONDataBaseTile : public BinaryOpcode
	{
	public:
		ONDataBaseTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataBaseTile(a->clone(), b->clone());
		}
	};

	class ONDataEHeight : public BinaryOpcode
	{
	public:
		ONDataEHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataEHeight(a->clone(), b->clone());
		}
	};

	//one input, no return
	class ONDataFlags : public BinaryOpcode
	{
	public:
		ONDataFlags(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataFlags(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataFlags2 : public BinaryOpcode
	{
	public:
		ONDataFlags2(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataFlags2(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataWidth : public BinaryOpcode
	{
	public:
		ONDataWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHeight : public BinaryOpcode
	{
	public:
		ONDataHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataTile : public BinaryOpcode
	{
	public:
		ONDataTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataTile(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSWidth : public BinaryOpcode
	{
	public:
		ONDataSWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSHeight : public BinaryOpcode
	{
	public:
		ONDataSHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataETile : public BinaryOpcode
	{
	public:
		ONDataETile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataETile(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataEWidth : public BinaryOpcode
	{
	public:
		ONDataEWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataEWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHP : public BinaryOpcode
	{
	public:
		ONDataHP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHP(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataFamily : public BinaryOpcode
	{
	public:
		ONDataFamily(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataFamily(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataCSet : public BinaryOpcode
	{
	public:
		ONDataCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataCSet(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataAnim : public BinaryOpcode
	{
	public:
		ONDataAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataAnim(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataEAnim : public BinaryOpcode
	{
	public:
		ONDataEAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataEAnim(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataFramerate : public BinaryOpcode
	{
	public:
		ONDataFramerate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataFramerate(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataEFramerate : public BinaryOpcode
	{
	public:
		ONDataEFramerate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataEFramerate(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataTouchDamage : public BinaryOpcode
	{
	public:
		ONDataTouchDamage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataTouchDamage(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataWeaponDamage : public BinaryOpcode
	{
	public:
		ONDataWeaponDamage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataWeaponDamage(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataWeapon : public BinaryOpcode
	{
	public:
		ONDataWeapon(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataWeapon(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataRandom : public BinaryOpcode
	{
	public:
		ONDataRandom(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataRandom(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHalt : public BinaryOpcode
	{
	public:
		ONDataHalt(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHalt(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataStep : public BinaryOpcode
	{
	public:
		ONDataStep(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataStep(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHoming : public BinaryOpcode
	{
	public:
		ONDataHoming(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHoming(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHunger : public BinaryOpcode
	{
	public:
		ONDataHunger(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHunger(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataropset : public BinaryOpcode
	{
	public:
		ONDataropset(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataropset(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataBGSound : public BinaryOpcode
	{
	public:
		ONDataBGSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataBGSound(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHitSound : public BinaryOpcode
	{
	public:
		ONDataHitSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHitSound(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataDeathSound : public BinaryOpcode
	{
	public:
		ONDataDeathSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataDeathSound(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataXofs : public BinaryOpcode
	{
	public:
		ONDataXofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataXofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataYofs : public BinaryOpcode
	{
	public:
		ONDataYofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataYofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataZofs : public BinaryOpcode
	{
	public:
		ONDataZofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataZofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHitXOfs : public BinaryOpcode
	{
	public:
		ONDataHitXOfs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHitXOfs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHYOfs : public BinaryOpcode
	{
	public:
		ONDataHYOfs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHYOfs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHitWidth : public BinaryOpcode
	{
	public:
		ONDataHitWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHitWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHitHeight : public BinaryOpcode
	{
	public:
		ONDataHitHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHitHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataHitZ : public BinaryOpcode
	{
	public:
		ONDataHitZ(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataHitZ(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataTileWidth : public BinaryOpcode
	{
	public:
		ONDataTileWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataTileWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataTileHeight : public BinaryOpcode
	{
	public:
		ONDataTileHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataTileHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataWeapSprite : public BinaryOpcode
	{
	public:
		ONDataWeapSprite(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataWeapSprite(a->clone(), b->clone());
		}
	};

	//two inputs, two returns

	class ONDataScriptDef : public UnaryOpcode
	{
	public:
		ONDataScriptDef(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataScriptDef(a->clone());
		}
	};
	//two inputs, two returns

	class ONDataDefense : public UnaryOpcode
	{
	public:
		ONDataDefense(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataDefense(a->clone());
		}
	};
	//two inputs, two returns

	class ONDataSizeFlag : public UnaryOpcode
	{
	public:
		ONDataSizeFlag(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSizeFlag(a->clone());
		}
	};
	//two inputs, two returns

	class ONDatattributes : public UnaryOpcode
	{
	public:
		ONDatattributes(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDatattributes(a->clone());
		}
	};

	class ONDataSetBaseTile : public BinaryOpcode
	{
	public:
		ONDataSetBaseTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetBaseTile(a->clone(), b->clone());
		}
	};
	class ONDataSetEHeight : public BinaryOpcode
	{
	public:
		ONDataSetEHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetEHeight(a->clone(), b->clone());
		}
	};

	class ONDataSetFlags : public BinaryOpcode
	{
	public:
		ONDataSetFlags(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetFlags(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetFlags2 : public BinaryOpcode
	{
	public:
		ONDataSetFlags2(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetFlags2(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetWidth : public BinaryOpcode
	{
	public:
		ONDataSetWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHeight : public BinaryOpcode
	{
	public:
		ONDataSetHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetTile : public BinaryOpcode
	{
	public:
		ONDataSetTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetTile(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetSWidth : public BinaryOpcode
	{
	public:
		ONDataSetSWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetSWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetSHeight : public BinaryOpcode
	{
	public:
		ONDataSetSHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetSHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetETile : public BinaryOpcode
	{
	public:
		ONDataSetETile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetETile(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetEWidth : public BinaryOpcode
	{
	public:
		ONDataSetEWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetEWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHP : public BinaryOpcode
	{
	public:
		ONDataSetHP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHP(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetFamily : public BinaryOpcode
	{
	public:
		ONDataSetFamily(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetFamily(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetCSet : public BinaryOpcode
	{
	public:
		ONDataSetCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetCSet(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetAnim : public BinaryOpcode
	{
	public:
		ONDataSetAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetAnim(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetEAnim : public BinaryOpcode
	{
	public:
		ONDataSetEAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetEAnim(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetFramerate : public BinaryOpcode
	{
	public:
		ONDataSetFramerate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetFramerate(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetEFramerate : public BinaryOpcode
	{
	public:
		ONDataSetEFramerate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetEFramerate(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetTouchDamage : public BinaryOpcode
	{
	public:
		ONDataSetTouchDamage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetTouchDamage(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetWeaponDamage : public BinaryOpcode
	{
	public:
		ONDataSetWeaponDamage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetWeaponDamage(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetWeapon : public BinaryOpcode
	{
	public:
		ONDataSetWeapon(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetWeapon(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetRandom : public BinaryOpcode
	{
	public:
		ONDataSetRandom(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetRandom(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHalt : public BinaryOpcode
	{
	public:
		ONDataSetHalt(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHalt(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetStep : public BinaryOpcode
	{
	public:
		ONDataSetStep(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetStep(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHoming : public BinaryOpcode
	{
	public:
		ONDataSetHoming(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHoming(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHunger : public BinaryOpcode
	{
	public:
		ONDataSetHunger(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHunger(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetropset : public BinaryOpcode
	{
	public:
		ONDataSetropset(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetropset(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHitSound : public BinaryOpcode
	{
	public:
		ONDataSetHitSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHitSound(a->clone(), b->clone());
		}
	};

	//one input, no return
	class ONDataSetBGSound : public BinaryOpcode
	{
	public:
		ONDataSetBGSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetBGSound(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetDeathSound : public BinaryOpcode
	{
	public:
		ONDataSetDeathSound(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetDeathSound(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetXofs : public BinaryOpcode
	{
	public:
		ONDataSetXofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetXofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetYofs : public BinaryOpcode
	{
	public:
		ONDataSetYofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetYofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetZofs : public BinaryOpcode
	{
	public:
		ONDataSetZofs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetZofs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHitXOfs : public BinaryOpcode
	{
	public:
		ONDataSetHitXOfs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHitXOfs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHYOfs : public BinaryOpcode
	{
	public:
		ONDataSetHYOfs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHYOfs(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHitWidth : public BinaryOpcode
	{
	public:
		ONDataSetHitWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHitWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHitHeight : public BinaryOpcode
	{
	public:
		ONDataSetHitHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHitHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetHitZ : public BinaryOpcode
	{
	public:
		ONDataSetHitZ(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetHitZ(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetTileWidth : public BinaryOpcode
	{
	public:
		ONDataSetTileWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetTileWidth(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetTileHeight : public BinaryOpcode
	{
	public:
		ONDataSetTileHeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetTileHeight(a->clone(), b->clone());
		}
	};
	//one input, no return
	class ONDataSetWeapSprite : public BinaryOpcode
	{
	public:
		ONDataSetWeapSprite(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONDataSetWeapSprite(a->clone(), b->clone());
		}
	};

	//ComboData

	class OCDataBlockEnemy : public BinaryOpcode
	{
	public:
		OCDataBlockEnemy(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataBlockEnemy(a->clone(), b->clone());
		}
	};
	class OCDataBlockHole : public BinaryOpcode
	{
	public:
		OCDataBlockHole(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataBlockHole(a->clone(), b->clone());
		}
	};
	class OCDataBlockTrig : public BinaryOpcode
	{
	public:
		OCDataBlockTrig(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataBlockTrig(a->clone(), b->clone());
		}
	};
	class OCDataConveyX : public BinaryOpcode
	{
	public:
		OCDataConveyX(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataConveyX(a->clone(), b->clone());
		}
	};
	class OCDataConveyY : public BinaryOpcode
	{
	public:
		OCDataConveyY(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataConveyY(a->clone(), b->clone());
		}
	};
	class OCDataCreateNPC : public BinaryOpcode
	{
	public:
		OCDataCreateNPC(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataCreateNPC(a->clone(), b->clone());
		}
	};
	class OCDataCreateEnemW : public BinaryOpcode
	{
	public:
		OCDataCreateEnemW(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataCreateEnemW(a->clone(), b->clone());
		}
	};
	class OCDataCreateEnemC : public BinaryOpcode
	{
	public:
		OCDataCreateEnemC(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataCreateEnemC(a->clone(), b->clone());
		}
	};
	class OCDataDirch : public BinaryOpcode
	{
	public:
		OCDataDirch(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataDirch(a->clone(), b->clone());
		}
	};
	class OCDataDistTiles : public BinaryOpcode
	{
	public:
		OCDataDistTiles(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataDistTiles(a->clone(), b->clone());
		}
	};
	class OCDataDiveItem : public BinaryOpcode
	{
	public:
		OCDataDiveItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataDiveItem(a->clone(), b->clone());
		}
	};
	class OCDataAttrib : public BinaryOpcode
	{
	public:
		OCDataAttrib(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataAttrib(a->clone(), b->clone());
		}
	};
	class OCDataDecoTile : public BinaryOpcode
	{
	public:
		OCDataDecoTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataDecoTile(a->clone(), b->clone());
		}
	};
	class OCDataDock : public BinaryOpcode
	{
	public:
		OCDataDock(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataDock(a->clone(), b->clone());
		}
	};
	class OCDataFairy : public BinaryOpcode
	{
	public:
		OCDataFairy(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataFairy(a->clone(), b->clone());
		}
	};
	class OCDataDecoType : public BinaryOpcode
	{
	public:
		OCDataDecoType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataDecoType(a->clone(), b->clone());
		}
	};
	class OCDataHookshotGrab : public BinaryOpcode
	{
	public:
		OCDataHookshotGrab(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataHookshotGrab(a->clone(), b->clone());
		}
	};
	class OCDataLockBlock : public BinaryOpcode
	{
	public:
		OCDataLockBlock(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataLockBlock(a->clone(), b->clone());
		}
	};
	class OCDataLockBlockChange : public BinaryOpcode
	{
	public:
		OCDataLockBlockChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataLockBlockChange(a->clone(), b->clone());
		}
	};
	class OCDataMagicMirror : public BinaryOpcode
	{
	public:
		OCDataMagicMirror(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataMagicMirror(a->clone(), b->clone());
		}
	};
	class OCDataModHP : public BinaryOpcode
	{
	public:
		OCDataModHP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataModHP(a->clone(), b->clone());
		}
	};
	class OCDataModHPDelay : public BinaryOpcode
	{
	public:
		OCDataModHPDelay(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataModHPDelay(a->clone(), b->clone());
		}
	};
	class OCDataModHpType : public BinaryOpcode
	{
	public:
		OCDataModHpType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataModHpType(a->clone(), b->clone());
		}
	};
	class OCDataModMP : public BinaryOpcode
	{
	public:
		OCDataModMP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataModMP(a->clone(), b->clone());
		}
	};
	class OCDataMpdMPDelay : public BinaryOpcode
	{
	public:
		OCDataMpdMPDelay(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataMpdMPDelay(a->clone(), b->clone());
		}
	};
	class OCDataModMPType : public BinaryOpcode
	{
	public:
		OCDataModMPType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataModMPType(a->clone(), b->clone());
		}
	};
	class OCDataNoPush : public BinaryOpcode
	{
	public:
		OCDataNoPush(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataNoPush(a->clone(), b->clone());
		}
	};
	class OCDataOverhead : public BinaryOpcode
	{
	public:
		OCDataOverhead(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataOverhead(a->clone(), b->clone());
		}
	};
	class OCDataEnemyLoc : public BinaryOpcode
	{
	public:
		OCDataEnemyLoc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataEnemyLoc(a->clone(), b->clone());
		}
	};
	class OCDataPushDir : public BinaryOpcode
	{
	public:
		OCDataPushDir(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataPushDir(a->clone(), b->clone());
		}
	};
	class OCDataPushWeight : public BinaryOpcode
	{
	public:
		OCDataPushWeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataPushWeight(a->clone(), b->clone());
		}
	};
	class OCDataPushWait : public BinaryOpcode
	{
	public:
		OCDataPushWait(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataPushWait(a->clone(), b->clone());
		}
	};
	class OCDataPushed : public BinaryOpcode
	{
	public:
		OCDataPushed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataPushed(a->clone(), b->clone());
		}
	};
	class OCDataRaft : public BinaryOpcode
	{
	public:
		OCDataRaft(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataRaft(a->clone(), b->clone());
		}
	};
	class OCDataResetRoom : public BinaryOpcode
	{
	public:
		OCDataResetRoom(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataResetRoom(a->clone(), b->clone());
		}
	};
	class OCDataSavePoint : public BinaryOpcode
	{
	public:
		OCDataSavePoint(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataSavePoint(a->clone(), b->clone());
		}
	};
	class OCDataFreeezeScreen : public BinaryOpcode
	{
	public:
		OCDataFreeezeScreen(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataFreeezeScreen(a->clone(), b->clone());
		}
	};
	class OCDataSecretCombo : public BinaryOpcode
	{
	public:
		OCDataSecretCombo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataSecretCombo(a->clone(), b->clone());
		}
	};
	class OCDataSingular : public BinaryOpcode
	{
	public:
		OCDataSingular(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataSingular(a->clone(), b->clone());
		}
	};
	class OCDataSlowMove : public BinaryOpcode
	{
	public:
		OCDataSlowMove(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataSlowMove(a->clone(), b->clone());
		}
	};
	class OCDataStatue : public BinaryOpcode
	{
	public:
		OCDataStatue(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataStatue(a->clone(), b->clone());
		}
	};
	class OCDataStepType : public BinaryOpcode
	{
	public:
		OCDataStepType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataStepType(a->clone(), b->clone());
		}
	};
	class OCDataSteoChange : public BinaryOpcode
	{
	public:
		OCDataSteoChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataSteoChange(a->clone(), b->clone());
		}
	};
	class OCDataStrikeRem : public BinaryOpcode
	{
	public:
		OCDataStrikeRem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataStrikeRem(a->clone(), b->clone());
		}
	};
	class OCDataStrikeRemType : public BinaryOpcode
	{
	public:
		OCDataStrikeRemType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataStrikeRemType(a->clone(), b->clone());
		}
	};
	class OCDataStrikeChange : public BinaryOpcode
	{
	public:
		OCDataStrikeChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataStrikeChange(a->clone(), b->clone());
		}
	};
	class OCDataStrikeChangeItem : public BinaryOpcode
	{
	public:
		OCDataStrikeChangeItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataStrikeChangeItem(a->clone(), b->clone());
		}
	};
	class OCDataTouchItem : public BinaryOpcode
	{
	public:
		OCDataTouchItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataTouchItem(a->clone(), b->clone());
		}
	};
	class OCDataTouchStairs : public BinaryOpcode
	{
	public:
		OCDataTouchStairs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataTouchStairs(a->clone(), b->clone());
		}
	};
	class OCDataTriggerType : public BinaryOpcode
	{
	public:
		OCDataTriggerType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataTriggerType(a->clone(), b->clone());
		}
	};
	class OCDataTriggerSens : public BinaryOpcode
	{
	public:
		OCDataTriggerSens(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataTriggerSens(a->clone(), b->clone());
		}
	};
	class OCDataWarpType : public BinaryOpcode
	{
	public:
		OCDataWarpType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataWarpType(a->clone(), b->clone());
		}
	};
	class OCDataWarpSens : public BinaryOpcode
	{
	public:
		OCDataWarpSens(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataWarpSens(a->clone(), b->clone());
		}
	};
	class OCDataWarpDirect : public BinaryOpcode
	{
	public:
		OCDataWarpDirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataWarpDirect(a->clone(), b->clone());
		}
	};
	class OCDataWarpLoc : public BinaryOpcode
	{
	public:
		OCDataWarpLoc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataWarpLoc(a->clone(), b->clone());
		}
	};
	class OCDataWater : public BinaryOpcode
	{
	public:
		OCDataWater(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataWater(a->clone(), b->clone());
		}
	};

	class OCDataWinGame : public BinaryOpcode
	{
	public:
		OCDataWinGame(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataWinGame(a->clone(), b->clone());
		}
	};

	class OCDataWhistle : public BinaryOpcode
	{
	public:
		OCDataWhistle(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataWhistle(a->clone(), b->clone());
		}
	};
	class OCDataWeapBlockLevel : public BinaryOpcode
	{
	public:
		OCDataWeapBlockLevel(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataWeapBlockLevel(a->clone(), b->clone());
		}
	};
	class OCDataTile : public BinaryOpcode
	{
	public:
		OCDataTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataTile(a->clone(), b->clone());
		}
	};
	class OCDataFlip : public BinaryOpcode
	{
	public:
		OCDataFlip(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataFlip(a->clone(), b->clone());
		}
	};
	class OCDataWalkability : public BinaryOpcode
	{
	public:
		OCDataWalkability(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataWalkability(a->clone(), b->clone());
		}
	};
	class OCDataType : public BinaryOpcode
	{
	public:
		OCDataType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataType(a->clone(), b->clone());
		}
	};
	class OCDataCSets : public BinaryOpcode
	{
	public:
		OCDataCSets(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataCSets(a->clone(), b->clone());
		}
	};
	class OCDataFoo : public BinaryOpcode
	{
	public:
		OCDataFoo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataFoo(a->clone(), b->clone());
		}
	};
	class OCDataFrames : public BinaryOpcode
	{
	public:
		OCDataFrames(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataFrames(a->clone(), b->clone());
		}
	};
	class OCDataSpeed : public BinaryOpcode
	{
	public:
		OCDataSpeed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataSpeed(a->clone(), b->clone());
		}
	};
	class OCDataNext : public BinaryOpcode
	{
	public:
		OCDataNext(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataNext(a->clone(), b->clone());
		}
	};
	class OCDataNextCSet : public BinaryOpcode
	{
	public:
		OCDataNextCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataNextCSet(a->clone(), b->clone());
		}
	};
	class OCDataFlag : public BinaryOpcode
	{
	public:
		OCDataFlag(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataFlag(a->clone(), b->clone());
		}
	};
	class OCDataSkipAnim : public BinaryOpcode
	{
	public:
		OCDataSkipAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataSkipAnim(a->clone(), b->clone());
		}
	};
	class OCDataTimer : public BinaryOpcode
	{
	public:
		OCDataTimer(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataTimer(a->clone(), b->clone());
		}
	};
	class OCDataAnimY : public BinaryOpcode
	{
	public:
		OCDataAnimY(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataAnimY(a->clone(), b->clone());
		}
	};
	class OCDataAnimFlags : public BinaryOpcode
	{
	public:
		OCDataAnimFlags(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataAnimFlags(a->clone(), b->clone());
		}
	};
	class OCDataBlockWeapon : public UnaryOpcode
	{
	public:
		OCDataBlockWeapon(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataBlockWeapon(a->clone());
		}
	};
	class OCDataExpansion : public UnaryOpcode
	{
	public:
		OCDataExpansion(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataExpansion(a->clone());
		}
	};
	class OCDataStrikeWeapon : public UnaryOpcode
	{
	public:
		OCDataStrikeWeapon(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataStrikeWeapon(a->clone());
		}
	};


	class OCSetDataBlockEnemy : public BinaryOpcode
	{
	public:
		OCSetDataBlockEnemy(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataBlockEnemy(a->clone(), b->clone());
		}
	};
	class OCSetDataBlockHole : public BinaryOpcode
	{
	public:
		OCSetDataBlockHole(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataBlockHole(a->clone(), b->clone());
		}
	};
	class OCSetDataBlockTrig : public BinaryOpcode
	{
	public:
		OCSetDataBlockTrig(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataBlockTrig(a->clone(), b->clone());
		}
	};
	class OCSetDataConveyX : public BinaryOpcode
	{
	public:
		OCSetDataConveyX(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataConveyX(a->clone(), b->clone());
		}
	};
	class OCSetDataConveyY : public BinaryOpcode
	{
	public:
		OCSetDataConveyY(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataConveyY(a->clone(), b->clone());
		}
	};
	class OCSetDataCreateNPC : public BinaryOpcode
	{
	public:
		OCSetDataCreateNPC(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataCreateNPC(a->clone(), b->clone());
		}
	};
	class OCSetDataCreateEnemW : public BinaryOpcode
	{
	public:
		OCSetDataCreateEnemW(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataCreateEnemW(a->clone(), b->clone());
		}
	};
	class OCSetDataCreateEnemC : public BinaryOpcode
	{
	public:
		OCSetDataCreateEnemC(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataCreateEnemC(a->clone(), b->clone());
		}
	};
	class OCSetDataDirch : public BinaryOpcode
	{
	public:
		OCSetDataDirch(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataDirch(a->clone(), b->clone());
		}
	};
	class OCSetDataDistTiles : public BinaryOpcode
	{
	public:
		OCSetDataDistTiles(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataDistTiles(a->clone(), b->clone());
		}
	};
	class OCSetDataDiveItem : public BinaryOpcode
	{
	public:
		OCSetDataDiveItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataDiveItem(a->clone(), b->clone());
		}
	};
	class OCSetDataAttrib : public BinaryOpcode
	{
	public:
		OCSetDataAttrib(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataAttrib(a->clone(), b->clone());
		}
	};
	class OCSetDataDecoTile : public BinaryOpcode
	{
	public:
		OCSetDataDecoTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataDecoTile(a->clone(), b->clone());
		}
	};
	class OCSetDataDock : public BinaryOpcode
	{
	public:
		OCSetDataDock(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataDock(a->clone(), b->clone());
		}
	};
	class OCSetDataFairy : public BinaryOpcode
	{
	public:
		OCSetDataFairy(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataFairy(a->clone(), b->clone());
		}
	};
	class OCSetDataDecoType : public BinaryOpcode
	{
	public:
		OCSetDataDecoType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataDecoType(a->clone(), b->clone());
		}
	};
	class OCSetDataHookshotGrab : public BinaryOpcode
	{
	public:
		OCSetDataHookshotGrab(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataHookshotGrab(a->clone(), b->clone());
		}
	};
	class OCSetDataLockBlock : public BinaryOpcode
	{
	public:
		OCSetDataLockBlock(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataLockBlock(a->clone(), b->clone());
		}
	};
	class OCSetDataLockBlockChange : public BinaryOpcode
	{
	public:
		OCSetDataLockBlockChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataLockBlockChange(a->clone(), b->clone());
		}
	};
	class OCSetDataMagicMirror : public BinaryOpcode
	{
	public:
		OCSetDataMagicMirror(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataMagicMirror(a->clone(), b->clone());
		}
	};
	class OCSetDataModHP : public BinaryOpcode
	{
	public:
		OCSetDataModHP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataModHP(a->clone(), b->clone());
		}
	};
	class OCSetDataModHPDelay : public BinaryOpcode
	{
	public:
		OCSetDataModHPDelay(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataModHPDelay(a->clone(), b->clone());
		}
	};
	class OCSetDataModHpType : public BinaryOpcode
	{
	public:
		OCSetDataModHpType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataModHpType(a->clone(), b->clone());
		}
	};
	class OCSetDataModMP : public BinaryOpcode
	{
	public:
		OCSetDataModMP(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataModMP(a->clone(), b->clone());
		}
	};
	class OCSetDataMpdMPDelay : public BinaryOpcode
	{
	public:
		OCSetDataMpdMPDelay(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataMpdMPDelay(a->clone(), b->clone());
		}
	};
	class OCSetDataModMPType : public BinaryOpcode
	{
	public:
		OCSetDataModMPType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataModMPType(a->clone(), b->clone());
		}
	};
	class OCSetDataNoPush : public BinaryOpcode
	{
	public:
		OCSetDataNoPush(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataNoPush(a->clone(), b->clone());
		}
	};
	class OCSetDataOverhead : public BinaryOpcode
	{
	public:
		OCSetDataOverhead(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataOverhead(a->clone(), b->clone());
		}
	};
	class OCSetDataEnemyLoc : public BinaryOpcode
	{
	public:
		OCSetDataEnemyLoc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataEnemyLoc(a->clone(), b->clone());
		}
	};
	class OCSetDataPushDir : public BinaryOpcode
	{
	public:
		OCSetDataPushDir(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataPushDir(a->clone(), b->clone());
		}
	};
	class OCSetDataPushWeight : public BinaryOpcode
	{
	public:
		OCSetDataPushWeight(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataPushWeight(a->clone(), b->clone());
		}
	};
	class OCSetDataPushWait : public BinaryOpcode
	{
	public:
		OCSetDataPushWait(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataPushWait(a->clone(), b->clone());
		}
	};
	class OCSetDataPushed : public BinaryOpcode
	{
	public:
		OCSetDataPushed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataPushed(a->clone(), b->clone());
		}
	};
	class OCSetDataRaft : public BinaryOpcode
	{
	public:
		OCSetDataRaft(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataRaft(a->clone(), b->clone());
		}
	};
	class OCSetDataResetRoom : public BinaryOpcode
	{
	public:
		OCSetDataResetRoom(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataResetRoom(a->clone(), b->clone());
		}
	};
	class OCSetDataSavePoint : public BinaryOpcode
	{
	public:
		OCSetDataSavePoint(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataSavePoint(a->clone(), b->clone());
		}
	};
	class OCSetDataFreeezeScreen : public BinaryOpcode
	{
	public:
		OCSetDataFreeezeScreen(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataFreeezeScreen(a->clone(), b->clone());
		}
	};
	class OCSetDataSecretCombo : public BinaryOpcode
	{
	public:
		OCSetDataSecretCombo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataSecretCombo(a->clone(), b->clone());
		}
	};
	class OCSetDataSingular : public BinaryOpcode
	{
	public:
		OCSetDataSingular(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataSingular(a->clone(), b->clone());
		}
	};
	class OCSetDataSlowMove : public BinaryOpcode
	{
	public:
		OCSetDataSlowMove(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataSlowMove(a->clone(), b->clone());
		}
	};
	class OCSetDataStatue : public BinaryOpcode
	{
	public:
		OCSetDataStatue(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataStatue(a->clone(), b->clone());
		}
	};
	class OCSetDataStepType : public BinaryOpcode
	{
	public:
		OCSetDataStepType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataStepType(a->clone(), b->clone());
		}
	};
	class OCSetDataSteoChange : public BinaryOpcode
	{
	public:
		OCSetDataSteoChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataSteoChange(a->clone(), b->clone());
		}
	};
	class OCSetDataStrikeRem : public BinaryOpcode
	{
	public:
		OCSetDataStrikeRem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataStrikeRem(a->clone(), b->clone());
		}
	};
	class OCSetDataStrikeRemType : public BinaryOpcode
	{
	public:
		OCSetDataStrikeRemType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataStrikeRemType(a->clone(), b->clone());
		}
	};
	class OCSetDataStrikeChange : public BinaryOpcode
	{
	public:
		OCSetDataStrikeChange(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataStrikeChange(a->clone(), b->clone());
		}
	};
	class OCSetDataStrikeChangeItem : public BinaryOpcode
	{
	public:
		OCSetDataStrikeChangeItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataStrikeChangeItem(a->clone(), b->clone());
		}
	};
	class OCSetDataTouchItem : public BinaryOpcode
	{
	public:
		OCSetDataTouchItem(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataTouchItem(a->clone(), b->clone());
		}
	};
	class OCSetDataTouchStairs : public BinaryOpcode
	{
	public:
		OCSetDataTouchStairs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataTouchStairs(a->clone(), b->clone());
		}
	};
	class OCSetDataTriggerType : public BinaryOpcode
	{
	public:
		OCSetDataTriggerType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataTriggerType(a->clone(), b->clone());
		}
	};
	class OCSetDataTriggerSens : public BinaryOpcode
	{
	public:
		OCSetDataTriggerSens(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataTriggerSens(a->clone(), b->clone());
		}
	};
	class OCSetDataWarpType : public BinaryOpcode
	{
	public:
		OCSetDataWarpType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataWarpType(a->clone(), b->clone());
		}
	};
	class OCSetDataWarpSens : public BinaryOpcode
	{
	public:
		OCSetDataWarpSens(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataWarpSens(a->clone(), b->clone());
		}
	};
	class OCSetDataWarpDirect : public BinaryOpcode
	{
	public:
		OCSetDataWarpDirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataWarpDirect(a->clone(), b->clone());
		}
	};
	class OCSetDataWarpLoc : public BinaryOpcode
	{
	public:
		OCSetDataWarpLoc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataWarpLoc(a->clone(), b->clone());
		}
	};
	class OCSetDataWater : public BinaryOpcode
	{
	public:
		OCSetDataWater(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataWater(a->clone(), b->clone());
		}
	};
	class OCSetDataWhistle : public BinaryOpcode
	{
	public:
		OCSetDataWhistle(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataWhistle(a->clone(), b->clone());
		}
	};
	class OCSetDataWeapBlockLevel : public BinaryOpcode
	{
	public:
		OCSetDataWeapBlockLevel(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataWeapBlockLevel(a->clone(), b->clone());
		}
	};
	class OCSetDataTile : public BinaryOpcode
	{
	public:
		OCSetDataTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataTile(a->clone(), b->clone());
		}
	};
	class OCSetDataFlip : public BinaryOpcode
	{
	public:
		OCSetDataFlip(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataFlip(a->clone(), b->clone());
		}
	};
	class OCSetDataWalkability : public BinaryOpcode
	{
	public:
		OCSetDataWalkability(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataWalkability(a->clone(), b->clone());
		}
	};
	class OCSetDataType : public BinaryOpcode
	{
	public:
		OCSetDataType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataType(a->clone(), b->clone());
		}
	};
	class OCSetDataCSets : public BinaryOpcode
	{
	public:
		OCSetDataCSets(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataCSets(a->clone(), b->clone());
		}
	};
	class OCSetDataFoo : public BinaryOpcode
	{
	public:
		OCSetDataFoo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataFoo(a->clone(), b->clone());
		}
	};
	class OCSetDataFrames : public BinaryOpcode
	{
	public:
		OCSetDataFrames(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataFrames(a->clone(), b->clone());
		}
	};
	class OCSetDataSpeed : public BinaryOpcode
	{
	public:
		OCSetDataSpeed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataSpeed(a->clone(), b->clone());
		}
	};
	class OCSetDataNext : public BinaryOpcode
	{
	public:
		OCSetDataNext(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataNext(a->clone(), b->clone());
		}
	};
	class OCSetDataNextCSet : public BinaryOpcode
	{
	public:
		OCSetDataNextCSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataNextCSet(a->clone(), b->clone());
		}
	};
	class OCSetDataFlag : public BinaryOpcode
	{
	public:
		OCSetDataFlag(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataFlag(a->clone(), b->clone());
		}
	};
	class OCSetDataSkipAnim : public BinaryOpcode
	{
	public:
		OCSetDataSkipAnim(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataSkipAnim(a->clone(), b->clone());
		}
	};

	class OCDataLadderPass : public BinaryOpcode
	{
	public:
		OCDataLadderPass(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCDataLadderPass(a->clone(), b->clone());
		}
	};

	class OCSetDataLadderPass : public BinaryOpcode
	{
	public:
		OCSetDataLadderPass(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataLadderPass(a->clone(), b->clone());
		}
	};
	class OCSetDataTimer : public BinaryOpcode
	{
	public:
		OCSetDataTimer(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataTimer(a->clone(), b->clone());
		}
	};
	class OCSetDataAnimY : public BinaryOpcode
	{
	public:
		OCSetDataAnimY(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataAnimY(a->clone(), b->clone());
		}
	};
	class OCSetDataAnimFlags : public BinaryOpcode
	{
	public:
		OCSetDataAnimFlags(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataAnimFlags(a->clone(), b->clone());
		}
	};
	class OCSetDataBlockWeapon : public UnaryOpcode
	{
	public:
		OCSetDataBlockWeapon(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataBlockWeapon(a->clone());
		}
	};
	class OCSetDataExpansion : public UnaryOpcode
	{
	public:
		OCSetDataExpansion(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataExpansion(a->clone());
		}
	};
	class OCSetDataStrikeWeapon : public UnaryOpcode
	{
	public:
		OCSetDataStrikeWeapon(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataStrikeWeapon(a->clone());
		}
	};

	class OCSetDataWinGame : public BinaryOpcode
	{
	public:
		OCSetDataWinGame(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCSetDataWinGame(a->clone(), b->clone());
		}
	};

	//SpriteData
	class OSDataTile : public BinaryOpcode
	{
	public:
		OSDataTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSDataTile(a->clone(), b->clone());
		}
	};

	class OSDataMisc : public BinaryOpcode
	{
	public:
		OSDataMisc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSDataMisc(a->clone(), b->clone());
		}
	};

	class OSDataCSets : public BinaryOpcode
	{
	public:
		OSDataCSets(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSDataCSets(a->clone(), b->clone());
		}
	};

	class OSDataFrames : public BinaryOpcode
	{
	public:
		OSDataFrames(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSDataFrames(a->clone(), b->clone());
		}
	};

	class OSDataSpeed : public BinaryOpcode
	{
	public:
		OSDataSpeed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSDataSpeed(a->clone(), b->clone());
		}
	};
	class OSDataType : public BinaryOpcode
	{
	public:
		OSDataType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSDataType(a->clone(), b->clone());
		}
	};

	class OSSetDataTile : public BinaryOpcode
	{
	public:
		OSSetDataTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSSetDataTile(a->clone(), b->clone());
		}
	};

	class OSSetDataMisc : public BinaryOpcode
	{
	public:
		OSSetDataMisc(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSSetDataMisc(a->clone(), b->clone());
		}
	};

	class OSSetDataFrames : public BinaryOpcode
	{
	public:
		OSSetDataFrames(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSSetDataFrames(a->clone(), b->clone());
		}
	};

	class OSSetDataSpeed : public BinaryOpcode
	{
	public:
		OSSetDataSpeed(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSSetDataSpeed(a->clone(), b->clone());
		}
	};
	class OSSetDataType : public BinaryOpcode
	{
	public:
		OSSetDataType(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSSetDataType(a->clone(), b->clone());
		}
	};

	//Continue Screen


	class OSSetContinueScreen : public BinaryOpcode
	{
	public:
		OSSetContinueScreen(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSSetContinueScreen(a->clone(), b->clone());
		}
	};
	class OSSetContinueString : public BinaryOpcode
	{
	public:
		OSSetContinueString(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSSetContinueString	(a->clone(), b->clone());
		}
	};

	//Visual effects with one bool input


	class OWavyR : public UnaryOpcode
	{
	public:
		OWavyR(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWavyR(a->clone());
		}
	};

	class OZapR : public UnaryOpcode
	{
	public:
		OZapR(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OZapR(a->clone());
		}
	};

	class OGreyscaleR : public UnaryOpcode
	{
	public:
		OGreyscaleR(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGreyscaleR(a->clone());
		}
	};
	
	class OMonochromeR : public UnaryOpcode
	{
	public:
		OMonochromeR(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMonochromeR(a->clone());
		}
	};
	
	class OClearTint : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OClearTint();
		}
	};
	
	class OTintR : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTintR();
		}
	};
	
	class OMonoHueR : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMonoHueR();
		}
	};
	
	//bitmap functions
	
	class OBMPDrawBitmapExRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawBitmapExRegister();
		}
	};
	class OBMPBlitTO : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPBlitTO();
		}
	};
	
	class OBMPGetPixel : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPGetPixel();
		}
	};
	class OBMPMode7 : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPMode7();
		}
	};
	class OBMPQuadRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPQuadRegister();
		}
	};
	class OBMPTriangleRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPTriangleRegister();
		}
	};
	class OBMPQuad3DRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPQuad3DRegister();
		}
	};
	class OBMPTriangle3DRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPTriangle3DRegister();
		}
	};
	class OBMPDrawLayerRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawLayerRegister();
		}
	};
	class OBMPDrawScreenRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenRegister();
		}
	};
	class OBMPDrawStringRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawStringRegister();
		}
	};
	class OBMPDrawString2Register : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawString2Register();
		}
	};
	class OBMPFastComboRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPFastComboRegister();
		}
	};
	class OBMPFastTileRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPFastTileRegister();
		}
	};
	class OBMPDrawComboRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawComboRegister();
		}
	};
	class OBMPDrawComboCloakedRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawComboCloakedRegister();
		}
	};
	class OBMPDrawTileRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawTileRegister();
		}
	};
	class OBMPDrawTileCloakedRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawTileCloakedRegister();
		}
	};
	class OBMPDrawIntRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawIntRegister();
		}
	};

	class OBMPDrawCharRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawCharRegister();
		}
	};
	class OBMPPutPixelRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPPutPixelRegister();
		}
	};
	class OBMPSplineRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPSplineRegister();
		}
	};
	class OBMPLineRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPLineRegister();
		}
	};
	class OBMPEllipseRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPEllipseRegister();
		}
	};
	class OBMPArcRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPArcRegister();
		}
	};
	class OBMPCircleRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPCircleRegister();
		}
	};
	class OBMPRectangleRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPRectangleRegister();
		}
	};
	class OBMPFrameRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPFrameRegister();
		}
	};
	
	class OHeroWarpExRegister : public UnaryOpcode
	{
	public:
		OHeroWarpExRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroWarpExRegister(a->clone());
		}
	};
	
	class OHeroExplodeRegister : public UnaryOpcode
	{
	public:
		OHeroExplodeRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OHeroExplodeRegister(a->clone());
		}
	};
	
	class OSwitchNPC : public UnaryOpcode
	{
	public:
		OSwitchNPC(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSwitchNPC(a->clone());
		}
	};
	class OSwitchItem : public UnaryOpcode
	{
	public:
		OSwitchItem(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSwitchItem(a->clone());
		}
	};
	class OSwitchLW : public UnaryOpcode
	{
	public:
		OSwitchLW(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSwitchLW(a->clone());
		}
	};
	class OSwitchEW : public UnaryOpcode
	{
	public:
		OSwitchEW(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSwitchEW(a->clone());
		}
	};
	class OSwitchCombo : public BinaryOpcode
	{
	public:
		OSwitchCombo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSwitchCombo(a->clone(),b->clone());
		}
	};
	class OKillPlayer : public UnaryOpcode
	{
	public:
		OKillPlayer(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OKillPlayer(a->clone());
		}
	};
	class OScreenDoSpawn : public Opcode
	{
	public:
		OScreenDoSpawn() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OScreenDoSpawn();
		}
	};
	class OScreenTriggerCombo : public BinaryOpcode
	{
	public:
		OScreenTriggerCombo(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OScreenTriggerCombo(a->clone(),b->clone());
		}
	};
	
	class ONPCMovePaused : public Opcode
	{
	public:
		ONPCMovePaused() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCMovePaused();
		}
	};
	class ONPCMove : public Opcode
	{
	public:
		ONPCMove() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCMove();
		}
	};
	class ONPCMoveAngle : public Opcode
	{
	public:
		ONPCMoveAngle() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCMoveAngle();
		}
	};
	class ONPCMoveXY : public Opcode
	{
	public:
		ONPCMoveXY() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCMoveXY();
		}
	};
	class ONPCCanMoveDir : public Opcode
	{
	public:
		ONPCCanMoveDir() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCCanMoveDir();
		}
	};
	class ONPCCanMoveAngle : public Opcode
	{
	public:
		ONPCCanMoveAngle() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCCanMoveAngle();
		}
	};
	class ONPCCanMoveXY : public Opcode
	{
	public:
		ONPCCanMoveXY() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCCanMoveXY();
		}
	};
	
	class OGetSystemRTCRegister : public UnaryOpcode
	{
	public:
		OGetSystemRTCRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetSystemRTCRegister(a->clone());
		}
	};
	

	class ONPCExplodeRegister : public UnaryOpcode
	{
	public:
		ONPCExplodeRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCExplodeRegister(a->clone());
		}
	};
	
	class OLWeaponExplodeRegister : public UnaryOpcode
	{
	public:
		OLWeaponExplodeRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLWeaponExplodeRegister(a->clone());
		}
	};

	class OEWeaponExplodeRegister : public UnaryOpcode
	{
	public:
		OEWeaponExplodeRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OEWeaponExplodeRegister(a->clone());
		}
	};

	class OItemExplodeRegister : public UnaryOpcode
	{
	public:
		OItemExplodeRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OItemExplodeRegister(a->clone());
		}
	};
	
	class ORunItemScript : public UnaryOpcode
	{
	public:
		ORunItemScript(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORunItemScript(a->clone());
		}
	};
	/*
	class ORunItemScript : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORunItemScript();
		}
	};
	*/
	
	//new npc-> functions for npc scripts
	class ONPCDead : public UnaryOpcode
	{
	public:
		ONPCDead(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCDead(a->clone());
		}
	};
	class ONPCCanSlide : public UnaryOpcode
	{
	public:
		ONPCCanSlide(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCCanSlide(a->clone());
		}
	};
	class ONPCSlide : public UnaryOpcode
	{
	public:
		ONPCSlide(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCSlide(a->clone());
		}
	};
	class ONPCRemove : public UnaryOpcode
	{
	public:
		ONPCRemove(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCRemove(a->clone());
		}
	};
	
	class OLWpnRemove : public Opcode
	{
	public:
		OLWpnRemove() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLWpnRemove();
		}
	};
	class OEWpnRemove : public Opcode
	{
	public:
		OEWpnRemove() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OEWpnRemove();
		}
	};
	class OItemRemove : public Opcode
	{
	public:
		OItemRemove() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OItemRemove();
		}
	};
	class ONPCStopSFX : public UnaryOpcode
	{
	public:
		ONPCStopSFX(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCStopSFX(a->clone());
		}
	};
	class ONPCAttack : public UnaryOpcode
	{
	public:
		ONPCAttack(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCAttack(a->clone());
		}
	};
	class ONPCNewDir : public UnaryOpcode
	{
	public:
		ONPCNewDir(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCNewDir(a->clone());
		}
	};
	class ONPCConstWalk : public UnaryOpcode
	{
	public:
		ONPCConstWalk(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCConstWalk(a->clone());
		}
	};
	class ONPCConstWalk8 : public UnaryOpcode
	{
	public:
		ONPCConstWalk8(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCConstWalk8(a->clone());
		}
	};
	class ONPCVarWalk : public UnaryOpcode
	{
	public:
		ONPCVarWalk(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCVarWalk(a->clone());
		}
	};
	class ONPCVarWalk8 : public UnaryOpcode
	{
	public:
		ONPCVarWalk8(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCVarWalk8(a->clone());
		}
	};
	class ONPCHaltWalk : public UnaryOpcode
	{
	public:
		ONPCHaltWalk(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCHaltWalk(a->clone());
		}
	};
	class ONPCHaltWalk8 : public UnaryOpcode
	{
	public:
		ONPCHaltWalk8(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCHaltWalk8(a->clone());
		}
	};
	class ONPCFloatWalk : public UnaryOpcode
	{
	public:
		ONPCFloatWalk(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCFloatWalk(a->clone());
		}
	};
	class ONPCBreatheFire : public UnaryOpcode
	{
	public:
		ONPCBreatheFire(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCBreatheFire(a->clone());
		}
	};
	class ONPCNewDir8 : public UnaryOpcode
	{
	public:
		ONPCNewDir8(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCNewDir8(a->clone());
		}
	};
	class ONPCHeroInRange : public UnaryOpcode
	{
	public:
		ONPCHeroInRange(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCHeroInRange(a->clone());
		}
	};
	class ONPCAdd : public UnaryOpcode
	{
	public:
		ONPCAdd(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCAdd(a->clone());
		}
	};
	class ONPCCanMove : public UnaryOpcode
	{
	public:
		ONPCCanMove(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCCanMove(a->clone());
		}
	};
	class ONPCHitWith : public UnaryOpcode
	{
	public:
		ONPCHitWith(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCHitWith(a->clone());
		}
	};
	class ONPCKnockback : public BinaryOpcode
	{
	public:
		ONPCKnockback(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ONPCKnockback(a->clone(),b->clone());
		}
	};
	class OGetNPCDataName : public UnaryOpcode
	{
	public:
		OGetNPCDataName(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetNPCDataName(a->clone());
		}
	};
	
	class OIsValidBitmap : public UnaryOpcode
	{
	public:
		OIsValidBitmap(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsValidBitmap(a->clone());
		}
	};
	
	class OIsAllocatedBitmap : public UnaryOpcode
	{
	public:
		OIsAllocatedBitmap(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OIsAllocatedBitmap(a->clone());
		}
	};
	
	class OAllocateBitmap : public UnaryOpcode
	{
	public:
		OAllocateBitmap(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAllocateBitmap(a->clone());
		}
	};
	
	class OReadBitmap : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReadBitmap();
		}
	};
	
	class OClearBitmap : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OClearBitmap();
		}
	};
	
	class OBitmapClearToColor : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapClearToColor();
		}
	};
	
	class ORegenerateBitmap : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORegenerateBitmap();
		}
	};
	
	class OWriteBitmap : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWriteBitmap();
		}
	};
	
	class OBitmapFree : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapFree();
		}
	};
	
	class OBitmapOwn : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapOwn();
		}
	};
	class OFileOwn : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileOwn();
		}
	};
	class ODirectoryOwn : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODirectoryOwn();
		}
	};
	class ORNGOwn : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORNGOwn();
		}
	};
	
	class OBitmapWriteTile : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapWriteTile();
		}
	};
	
	class OBitmapDither : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapDither();
		}
	};
	
	class OBitmapReplColor : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapReplColor();
		}
	};
	
	class OBitmapShiftColor : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapShiftColor();
		}
	};
	
	class OBitmapMaskDraw : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapMaskDraw();
		}
	};
	
	class OBitmapMaskDraw2 : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapMaskDraw2();
		}
	};
	
	class OBitmapMaskDraw3 : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapMaskDraw3();
		}
	};
	
	class OBitmapMaskBlit : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapMaskBlit();
		}
	};
	
	class OBitmapMaskBlit2 : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapMaskBlit2();
		}
	};
	
	class OBitmapMaskBlit3 : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBitmapMaskBlit3();
		}
	};
	
	class OBMPDrawScreenSolidRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenSolidRegister();
		}
	};
	class OBMPDrawScreenSolid2Register : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenSolid2Register();
		}
	};
	class OBMPDrawScreenComboFRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenComboFRegister();
		}
	};
	class OBMPDrawScreenComboIRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenComboIRegister();
		}
	};
	class OBMPDrawScreenComboTRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenComboTRegister();
		}
	};

	class OBMPDrawScreenSolidityRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenSolidityRegister();
		}
	};
	class OBMPDrawScreenSolidMaskRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenSolidMaskRegister();
		}
	};
	class OBMPDrawScreenCTypeRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenCTypeRegister();
		}
	};
	class OBMPDrawScreenCFlagRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenCFlagRegister();
		}
	};
	class OBMPDrawScreenCIFlagRegister : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OBMPDrawScreenCIFlagRegister();
		}
	};
	
	//Text ptr opcodes
	class OFontHeight : public UnaryOpcode
	{
	public:
		OFontHeight(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFontHeight(a->clone());
		}
	};
	
	class OStringWidth : public BinaryOpcode
	{
	public:
		OStringWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStringWidth(a->clone(),b->clone());
		}
	};
	
	class OCharWidth : public BinaryOpcode
	{
	public:
		OCharWidth(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCharWidth(a->clone(),b->clone());
		}
	};
	
	class OMessageWidth : public UnaryOpcode
	{
	public:
		OMessageWidth(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMessageWidth(a->clone());
		}
	};
	
	class OMessageHeight : public UnaryOpcode
	{
	public:
		OMessageHeight(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMessageHeight(a->clone());
		}
	};
	
	//
	class OStrCmp : public UnaryOpcode
	{
	public:
		OStrCmp(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStrCmp(a->clone());
		}
	};
	class OStrNCmp : public UnaryOpcode
	{
	public:
		OStrNCmp(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStrNCmp(a->clone());
		}
	};
	class OStrICmp : public UnaryOpcode
	{
	public:
		OStrICmp(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStrICmp(a->clone());
		}
	};
	class OStrNICmp : public UnaryOpcode
	{
	public:
		OStrNICmp(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStrNICmp(a->clone());
		}
	};
	
	class Oxlen : public BinaryOpcode
	{
	public:
		Oxlen(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oxlen(a->clone(), b->clone());
		}
	};
	
	class Oxtoi : public BinaryOpcode
	{
	public:
		Oxtoi(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oxtoi(a->clone(), b->clone());
		}
	};
	
	class Oilen : public BinaryOpcode
	{
	public:
		Oilen(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oilen(a->clone(), b->clone());
		}
	};
	
	
	class Oatoi : public BinaryOpcode
	{
	public:
		Oatoi(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oatoi(a->clone(), b->clone());
		}
	};
	
	class Oatol : public BinaryOpcode
	{
	public:
		Oatol(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oatol(a->clone(), b->clone());
		}
	};
	
	class Ostrcspn : public UnaryOpcode
	{
	public:
		Ostrcspn(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Ostrcspn(a->clone());
		}
	};
	class Ostrstr : public UnaryOpcode
	{
	public:
		Ostrstr(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Ostrstr(a->clone());
		}
	};
	
	
	class Oitoa : public BinaryOpcode
	{
	public:
		Oitoa(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oitoa(a->clone(),b->clone());
		}
	};
	
	class Oxtoa : public BinaryOpcode
	{
	public:
		Oxtoa(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oxtoa(a->clone(),b->clone());
		}
	};
	
	class Oitoacat : public BinaryOpcode
	{
	public:
		Oitoacat(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oitoacat(a->clone(),b->clone());
		}
	};
	
	class OSaveGameStructs : public BinaryOpcode
	{
	public:
		OSaveGameStructs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSaveGameStructs(a->clone(),b->clone());
		}
	};
	class OReadGameStructs : public BinaryOpcode
	{
	public:
		OReadGameStructs(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReadGameStructs(a->clone(),b->clone());
		}
	};
	class Ostrcat : public UnaryOpcode
	{
	public:
		Ostrcat(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Ostrcat(a->clone());
		}
	};
	class Ostrspn : public UnaryOpcode
	{
	public:
		Ostrspn(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Ostrspn(a->clone());
		}
	};
	class Ostrchr : public UnaryOpcode
	{
	public:
		Ostrchr(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Ostrchr(a->clone());
		}
	};
	class Ostrrchr : public UnaryOpcode
	{
	public:
		Ostrrchr(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Ostrrchr(a->clone());
		}
	};
	class Oxlen2 : public UnaryOpcode
	{
	public:
		Oxlen2(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oxlen2(a->clone());
		}
	};
	class Oxtoi2 : public UnaryOpcode
	{
	public:
		Oxtoi2(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oxtoi2(a->clone());
		}
	};
	class Oilen2 : public UnaryOpcode
	{
	public:
		Oilen2(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oilen2(a->clone());
		}
	};
	class Oatoi2 : public UnaryOpcode
	{
	public:
		Oatoi2(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oatoi2(a->clone());
		}
	};
	class Oremchr2 : public UnaryOpcode
	{
	public:
		Oremchr2(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oremchr2(a->clone());
		}
	};
	
	
	class Ouppertolower : public BinaryOpcode
	{
	public:
		Ouppertolower(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Ouppertolower(a->clone(), b->clone());
		}
	};
	
	class Olowertoupper : public BinaryOpcode
	{
	public:
		Olowertoupper(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Olowertoupper(a->clone(), b->clone());
		}
	};
	
	class Oconvertcase : public BinaryOpcode
	{
	public:
		Oconvertcase(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new Oconvertcase(a->clone(), b->clone());
		}
	};
	
	
	//Game->Getthingbystring
	class OGETNPCSCRIPT : public UnaryOpcode
	{
	public:
		OGETNPCSCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETNPCSCRIPT(a->clone());
		}
	};
	class OGETLWEAPONSCRIPT : public UnaryOpcode
	{
	public:
		OGETLWEAPONSCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETLWEAPONSCRIPT(a->clone());
		}
	};
	class OGETEWEAPONSCRIPT : public UnaryOpcode
	{
	public:
		OGETEWEAPONSCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETEWEAPONSCRIPT(a->clone());
		}
	};
	class OGETGENERICSCRIPT : public UnaryOpcode
	{
	public:
		OGETGENERICSCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETGENERICSCRIPT(a->clone());
		}
	};
	class OGETHEROSCRIPT : public UnaryOpcode
	{
	public:
		OGETHEROSCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETHEROSCRIPT(a->clone());
		}
	};
	class OGETGLOBALSCRIPT : public UnaryOpcode
	{
	public:
		OGETGLOBALSCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETGLOBALSCRIPT(a->clone());
		}
	};
	class OGETDMAPSCRIPT : public UnaryOpcode
	{
	public:
		OGETDMAPSCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETDMAPSCRIPT(a->clone());
		}
	};
	class OGETSCREENSCRIPT : public UnaryOpcode
	{
	public:
		OGETSCREENSCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETSCREENSCRIPT(a->clone());
		}
	};
	class OGETSPRITESCRIPT : public UnaryOpcode
	{
	public:
		OGETSPRITESCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETSPRITESCRIPT(a->clone());
		}
	};
	class OGETUNTYPEDSCRIPT : public UnaryOpcode
	{
	public:
		OGETUNTYPEDSCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETUNTYPEDSCRIPT(a->clone());
		}
	};
	class OGETSUBSCREENSCRIPT : public UnaryOpcode
	{
	public:
		OGETSUBSCREENSCRIPT(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETSUBSCREENSCRIPT(a->clone());
		}
	};
	class OGETNPCBYNAME : public UnaryOpcode
	{
	public:
		OGETNPCBYNAME(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETNPCBYNAME(a->clone());
		}
	};
	class OGETITEMBYNAME : public UnaryOpcode
	{
	public:
		OGETITEMBYNAME(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETITEMBYNAME(a->clone());
		}
	};
	class OGETCOMBOBYNAME : public UnaryOpcode
	{
	public:
		OGETCOMBOBYNAME(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETCOMBOBYNAME(a->clone());
		}
	};
	class OGETDMAPBYNAME : public UnaryOpcode
	{
	public:
		OGETDMAPBYNAME(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGETDMAPBYNAME(a->clone());
		}
	};
	
	class OLoadNPCBySUIDRegister : public UnaryOpcode
	{
	public:
		OLoadNPCBySUIDRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadNPCBySUIDRegister(a->clone());
		}
	};
	
	class OLoadLWeaponBySUIDRegister : public UnaryOpcode
	{
	public:
		OLoadLWeaponBySUIDRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadLWeaponBySUIDRegister(a->clone());
		}
	};
	
	class OLoadEWeaponBySUIDRegister : public UnaryOpcode
	{
	public:
		OLoadEWeaponBySUIDRegister(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadEWeaponBySUIDRegister(a->clone());
		}
	};
	
	class OByte : public UnaryOpcode
	{
	public:
		OByte(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OByte(a->clone());
		}
	};
	
	class OCeiling : public UnaryOpcode
	{
	public:
		OCeiling(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCeiling(a->clone());
		}
	};
	
	class OFloor : public UnaryOpcode
	{
	public:
		OFloor(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFloor(a->clone());
		}
	};
	
	class OTruncate : public UnaryOpcode
	{
	public:
		OTruncate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OTruncate(a->clone());
		}
	};
	
	class ORound : public UnaryOpcode
	{
	public:
		ORound(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORound(a->clone());
		}
	};
	
	class ORoundAway : public UnaryOpcode
	{
	public:
		ORoundAway(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORoundAway(a->clone());
		}
	};
	
	class OToInteger : public UnaryOpcode
	{
	public:
		OToInteger(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OToInteger(a->clone());
		}
	};
	
	class OWord : public UnaryOpcode
	{
	public:
		OWord(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWord(a->clone());
		}
	};
	
	class OShort : public UnaryOpcode
	{
	public:
		OShort(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OShort(a->clone());
		}
	};
	
	
	class OSByte : public UnaryOpcode
	{
	public:
		OSByte(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSByte(a->clone());
		}
	};
	
	class OReturn : public Opcode
	{
	public:
		std::string toString() const;
		OReturn* clone() const
		{
			return new OReturn();
		}
	};
	
	class OGraphicsGetpixel : public UnaryOpcode
	{
	public:
		OGraphicsGetpixel(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGraphicsGetpixel(a->clone());
		}
	};
	
	class OGraphicsCountColor : public UnaryOpcode
	{
	public:
		OGraphicsCountColor(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGraphicsCountColor(a->clone());
		}
	};
	
	class ODirExists : public UnaryOpcode
	{
	public:
		ODirExists(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODirExists(a->clone());
		}
	};
	
	class OFileExists : public UnaryOpcode
	{
	public:
		OFileExists(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileExists(a->clone());
		}
	};
	
	class OFileSystemRemove : public UnaryOpcode
	{
	public:
		OFileSystemRemove(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileSystemRemove(a->clone());
		}
	};
	
	class OFileClose : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileClose();
		}
	};
	
	class OFileFree : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileFree();
		}
	};
	
	class OFileIsAllocated : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileIsAllocated();
		}
	};
	
	class OFileIsValid : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileIsValid();
		}
	};
	
	class OAllocateFile : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OAllocateFile();
		}
	};
	
	class OFileFlush : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileFlush();
		}
	};
	
	class OFileGetChar : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileGetChar();
		}
	};
	
	class OFileRewind : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileRewind();
		}
	};
	
	class OFileClearError : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileClearError();
		}
	};
	
	class OFileOpen : public UnaryOpcode
	{
	public:
		OFileOpen(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileOpen(a->clone());
		}
	};
	
	class OFileCreate : public UnaryOpcode
	{
	public:
		OFileCreate(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileCreate(a->clone());
		}
	};
	
	class OFileReadString : public UnaryOpcode
	{
	public:
		OFileReadString(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileReadString(a->clone());
		}
	};
	
	class OFileWriteString : public UnaryOpcode
	{
	public:
		OFileWriteString(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileWriteString(a->clone());
		}
	};
	
	class OFilePutChar : public UnaryOpcode
	{
	public:
		OFilePutChar(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFilePutChar(a->clone());
		}
	};
	
	class OFileUngetChar : public UnaryOpcode
	{
	public:
		OFileUngetChar(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileUngetChar(a->clone());
		}
	};
	
	class OFileGetError : public UnaryOpcode
	{
	public:
		OFileGetError(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileGetError(a->clone());
		}
	};
	
	class OFileRemove : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileRemove();
		}
	};
	
	class OFileReadChars : public BinaryOpcode
	{
	public:
		OFileReadChars(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileReadChars(a->clone(), b->clone());
		}
	};
	
	class OFileReadBytes : public BinaryOpcode
	{
	public:
		OFileReadBytes(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileReadBytes(a->clone(), b->clone());
		}
	};
	
	class OFileReadInts : public BinaryOpcode
	{
	public:
		OFileReadInts(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileReadInts(a->clone(), b->clone());
		}
	};
	
	class OFileWriteChars : public BinaryOpcode
	{
	public:
		OFileWriteChars(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileWriteChars(a->clone(), b->clone());
		}
	};
	
	class OFileWriteBytes : public BinaryOpcode
	{
	public:
		OFileWriteBytes(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileWriteBytes(a->clone(), b->clone());
		}
	};
	
	class OFileWriteInts : public BinaryOpcode
	{
	public:
		OFileWriteInts(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileWriteInts(a->clone(), b->clone());
		}
	};
	
	class OFileSeek : public BinaryOpcode
	{
	public:
		OFileSeek(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileSeek(a->clone(), b->clone());
		}
	};
	
	class OFileOpenMode : public BinaryOpcode
	{
	public:
		OFileOpenMode(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OFileOpenMode(a->clone(), b->clone());
		}
	};
	
	class ODirectoryGet : public BinaryOpcode
	{
	public:
		ODirectoryGet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODirectoryGet(a->clone(), b->clone());
		}
	};
	
	class ODirectoryReload : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODirectoryReload();
		}
	};
	
	class ODirectoryFree : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new ODirectoryFree();
		}
	};
	
	class OStackFree : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackFree();
		}
	};
	class OStackOwn : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackOwn();
		}
	};
	class OStackClear : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackClear();
		}
	};
	
	class OStackPopBack : public UnaryOpcode
	{
	public:
		OStackPopBack(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackPopBack(a->clone());
		}
	};
	class OStackPopFront : public UnaryOpcode
	{
	public:
		OStackPopFront(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackPopFront(a->clone());
		}
	};
	class OStackPeekBack : public UnaryOpcode
	{
	public:
		OStackPeekBack(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackPeekBack(a->clone());
		}
	};
	class OStackPeekFront : public UnaryOpcode
	{
	public:
		OStackPeekFront(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackPeekFront(a->clone());
		}
	};
	class OStackPushBack : public UnaryOpcode
	{
	public:
		OStackPushBack(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackPushBack(a->clone());
		}
	};
	class OStackPushFront : public UnaryOpcode
	{
	public:
		OStackPushFront(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackPushFront(a->clone());
		}
	};
	class OStackGet : public UnaryOpcode
	{
	public:
		OStackGet(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackGet(a->clone());
		}
	};
	class OStackSet : public BinaryOpcode
	{
	public:
		OStackSet(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackSet(a->clone(), b->clone());
		}
	};
	
	class OModuleGetIC : public BinaryOpcode
	{
	public:
		OModuleGetIC(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OModuleGetIC(a->clone(), b->clone());
		}
	};
	class OGetScreenForComboPos : public UnaryOpcode
	{
	public:
		OGetScreenForComboPos(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode *clone() const
		{
			return new OGetScreenForComboPos(a->clone());
		}
	};
	class ORunGenericFrozenScript : public UnaryOpcode
	{
	public:
		ORunGenericFrozenScript(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORunGenericFrozenScript(a->clone());
		}
	};
	class OPalDataFree : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPalDataFree();
		}
	};
	class OPalDataOwn : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OPalDataOwn();
		}
	};

	class OReservedZ3_01 : public BinaryOpcode
	{
	public:
		OReservedZ3_01(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReservedZ3_01(a->clone(),b->clone());
		}
	};

	class OReservedZ3_02 : public BinaryOpcode
	{
	public:
		OReservedZ3_02(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReservedZ3_02(a->clone(),b->clone());
		}
	};

	class OReservedZ3_03 : public BinaryOpcode
	{
	public:
		OReservedZ3_03(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReservedZ3_03(a->clone(),b->clone());
		}
	};

	class OReservedZ3_04 : public BinaryOpcode
	{
	public:
		OReservedZ3_04(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReservedZ3_04(a->clone(),b->clone());
		}
	};

	class OReservedZ3_05 : public BinaryOpcode
	{
	public:
		OReservedZ3_05(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReservedZ3_05(a->clone(),b->clone());
		}
	};

	class OReservedZ3_06 : public BinaryOpcode
	{
	public:
		OReservedZ3_06(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReservedZ3_06(a->clone(),b->clone());
		}
	};

	class OReservedZ3_07 : public BinaryOpcode
	{
	public:
		OReservedZ3_07(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReservedZ3_07(a->clone(),b->clone());
		}
	};

	class OReservedZ3_08 : public BinaryOpcode
	{
	public:
		OReservedZ3_08(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReservedZ3_08(a->clone(),b->clone());
		}
	};

	class OReservedZ3_09 : public BinaryOpcode
	{
	public:
		OReservedZ3_09(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReservedZ3_09(a->clone(),b->clone());
		}
	};

	class OReservedZ3_10 : public BinaryOpcode
	{
	public:
		OReservedZ3_10(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReservedZ3_10(a->clone(),b->clone());
		}
	};



	class OSubscrSwapPages : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubscrSwapPages();
		}
	};

	class OSubscrPgFindWidget : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubscrPgFindWidget();
		}
	};

	class OSubscrPgMvCursor : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubscrPgMvCursor();
		}
	};

	class OSubscrPgSwapWidgets : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubscrPgSwapWidgets();
		}
	};

	class OSubscrPgNewWidget : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubscrPgNewWidget();
		}
	};

	class OSubscrPgDelete : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubscrPgDelete();
		}
	};

	class OGetSubWidgSelTxtOverride : public UnaryOpcode
	{
	public:
		OGetSubWidgSelTxtOverride(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetSubWidgSelTxtOverride(a->clone());
		}
	};

	class OSetSubWidgSelTxtOverride : public UnaryOpcode
	{
	public:
		OSetSubWidgSelTxtOverride(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetSubWidgSelTxtOverride(a->clone());
		}
	};

	class OSubWidgTy_GetText : public UnaryOpcode
	{
	public:
		OSubWidgTy_GetText(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubWidgTy_GetText(a->clone());
		}
	};

	class OSubWidgTy_SetText : public UnaryOpcode
	{
	public:
		OSubWidgTy_SetText(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubWidgTy_SetText(a->clone());
		}
	};



	class OSubscrPgFindWidgetLbl : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSubscrPgFindWidgetLbl();
		}
	};



	class OGetSubWidgLabel : public UnaryOpcode
	{
	public:
		OGetSubWidgLabel(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGetSubWidgLabel(a->clone());
		}
	};

	class OSetSubWidgLabel : public UnaryOpcode
	{
	public:
		OSetSubWidgLabel(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetSubWidgLabel(a->clone());
		}
	};



	class OWrapRadians : public UnaryOpcode
	{
	public:
		OWrapRadians(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWrapRadians(a->clone());
		}
	};

	class OWrapDegrees : public UnaryOpcode
	{
	public:
		OWrapDegrees(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWrapDegrees(a->clone());
		}
	};



	class OCallFunc : public UnaryOpcode
	{
	public:
		OCallFunc(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OCallFunc(a->clone());
		}
	};

	class OReturnFunc : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OReturnFunc();
		}
	};



	class OSetCompare : public BinaryOpcode
	{
	public:
		OSetCompare(Argument *A, Argument* B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OSetCompare(a->clone(),b->clone());
		}
	};

	class OGotoCompare : public BinaryOpcode
	{
	public:
		OGotoCompare(Argument *A, Argument* B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGotoCompare(a->clone(),b->clone());
		}
	};



	class OStackWriteAtRV : public BinaryOpcode
	{
	public:
		OStackWriteAtRV(Argument *A, Argument* B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackWriteAtRV(a->clone(),b->clone());
		}
	};

	class OStackWriteAtVV : public BinaryOpcode
	{
	public:
		OStackWriteAtVV(Argument *A, Argument* B) : BinaryOpcode(A,B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackWriteAtVV(a->clone(),b->clone());
		}
	};



	class OStackWriteAtVV_If : public TernaryOpcode
	{
	public:
		OStackWriteAtVV_If(Argument *A, Argument* B, Argument* C) : TernaryOpcode(A,B,C) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OStackWriteAtVV_If(a->clone(),b->clone(),c->clone());
		}
	};

	class OLoadWebSocket : public UnaryOpcode
	{
	public:
		OLoadWebSocket(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OLoadWebSocket(a->clone());
		}
	};

	class OWebSocketFree : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWebSocketFree();
		}
	};

	class OWebSocketOwn : public Opcode
	{
	public:
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWebSocketOwn();
		}
	};

	class OWebSocketGetError : public UnaryOpcode
	{
	public:
		OWebSocketGetError(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWebSocketGetError(a->clone());
		}
	};

	class OWebSocketSend : public BinaryOpcode
	{
	public:
		OWebSocketSend(Argument *A, Argument *B) : BinaryOpcode(A, B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWebSocketSend(a->clone(), b->clone());
		}
	};

	class OWebSocketReceive : public UnaryOpcode
	{
	public:
		OWebSocketReceive(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OWebSocketReceive(a->clone());
		}
	};

	class OGC : public Opcode
	{
	public:
		OGC() : Opcode() {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OGC();
		}
	};

	class ORefInc : public UnaryOpcode
	{
	public:
		ORefInc(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORefInc(a->clone());
		}
	};

	class ORefDec : public UnaryOpcode
	{
	public:
		ORefDec(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORefDec(a->clone());
		}
	};

	class ORefAutorelease : public UnaryOpcode
	{
	public:
		ORefAutorelease(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORefAutorelease(a->clone());
		}
	};

	class ORefRemove : public UnaryOpcode
	{
	public:
		ORefRemove(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORefRemove(a->clone());
		}
	};

	class ORefCount : public UnaryOpcode
	{
	public:
		ORefCount(Argument *A) : UnaryOpcode(A) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new ORefCount(a->clone());
		}
	};

	class OMarkTypeStack : public BinaryOpcode
	{
	public:
		OMarkTypeStack(Argument *A, Argument *B) : BinaryOpcode(A, B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMarkTypeStack(a->clone(), b->clone());
		}
	};

	class OMarkTypeRegister : public BinaryOpcode
	{
	public:
		OMarkTypeRegister(Argument *A, Argument *B) : BinaryOpcode(A, B) {}
		std::string toString() const;
		Opcode* clone() const
		{
			return new OMarkTypeRegister(a->clone(), b->clone());
		}
	};
}

#endif

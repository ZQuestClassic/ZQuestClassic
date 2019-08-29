#ifndef BYTECODE_H
#define BYTECODE_H

//#include "ScriptParser.h"
#include "AST.h"
#include "UtilVisitors.h"
#include "DataStructs.h"
#include "Compiler.h"
#include "../zsyssimple.h"

#include <string>

using namespace std;

/*
 I will reserve the registers in the following scheme:
 SP - stack pointer
 D4 - stack frame pointer
 D6 - stack frame offset accumulator
 D2 - expression accumulator #1
 D3 - expression accumulator #2
 D0 - array index accumulator
 D1 - secondary array index accumulator
 D5 - pure SETR sink
 */
#define FY 15
#define FX 14
#define DELAY 13
#define FCSET 12
#define DATA 11
#define SP 10
#define NUL 5
#define SFRAME 4
#define INDEX 0
#define SFTEMP 6
#define INDEX2 1
#define EXP1 2
#define EXP2 3
#define XD 16
#define YD 17
#define XD2 18
#define YD2 19
#define LINKX 20
#define LINKY 21
#define LINKDIR 22
#define LINKHP 23
#define LINKMP 24
#define LINKMAXHP 25
#define LINKMAXMP 26
#define LINKACTION 27
#define INPUTSTART 28
#define INPUTUP 29
#define INPUTDOWN 30
#define INPUTLEFT 31
#define INPUTRIGHT 32
#define INPUTA 33
#define INPUTB 34
#define INPUTL 35
#define INPUTR 36
#define SDD 37 // 8 of these
#define COMBODD 38 //176 of these
#define COMBOCD 39 //176 of these
#define COMBOFD 40 //176 of these
#define REFFFC 41
#define REFITEM 42
#define ITEMCOUNT 43
#define ITEMX	44
#define ITEMY 45
#define ITEMDRAWTYPE 46
#define ITEMID 47
#define ITEMTILE 48
#define ITEMCSET 49
#define ITEMFLASHCSET 50
#define ITEMFRAMES 51
#define ITEMFRAME 52
#define ITEMASPEED 53
#define ITEMDELAY 54
#define ITEMFLASH 55
#define ITEMFLIP 56
#define ITEMEXTEND 57
#define ITEMCLASSFAMILY 58
#define ITEMCLASSFAMTYPE 59
#define ITEMCLASSAMOUNT 60
#define ITEMCLASSMAX 61
#define ITEMCLASSSETMAX 62
#define ITEMCLASSSETGAME 63
#define ITEMCLASSCOUNTER 64
#define REFITEMCLASS 65
//#define GETA 66
//#define GETB 67
//#define GETL 68
//#define GETR 69
//#define GETUP 70
//#define GETDOWN 71
//#define GETLEFT 72
//#define GETRIGHT 73
//#define GETSTART 74
#define COMBOID 75
#define COMBOTD 76
#define COMBOSD 77
#define CURSCR 78
#define CURMAP 79
#define CURDMAP 80
#define GAMEDEATHS 81
#define GAMECHEAT 82
#define GAMETIME 83
#define GAMEHASPLAYED 84
#define GAMETIMEVALID 85
#define GAMEGUYCOUNT 86
#define GAMECONTSCR 87
#define GAMECONTDMAP 88
#define GAMECOUNTERD 89
#define GAMEMCOUNTERD 90
#define GAMEDCOUNTERD 91
#define GAMEGENERICD 92
#define GAMEITEMSD 93
#define GAMELITEMSD 94
#define GAMELKEYSD 95
#define GAMEMAPFLAGD 96
#define GAMEMAPFLAGDD 97
#define SDDD 98
#define FFFLAGSD 99
#define FFTWIDTH 100
#define FFTHEIGHT 101
#define FFCWIDTH 102
#define FFCHEIGHT 103
#define FFLINK 104
#define LINKITEMD 105

class LiteralArgument;
class VarArgument;
class LabelArgument;
class GlobalArgument;

class ArgumentVisitor
{
public:
	virtual void caseLiteral(LiteralArgument &host, void *param) {void *temp; temp=&host; param=param; /*these are here to bypass compiler warnings about unused arguments*/}
	virtual void caseVar(VarArgument &host, void *param) {void *temp; temp=&host; param=param; /*these are here to bypass compiler warnings about unused arguments*/}
	virtual void caseLabel(LabelArgument &host, void *param) {void *temp; temp=&host; param=param; /*these are here to bypass compiler warnings about unused arguments*/}
	virtual void caseGlobal(GlobalArgument &host, void *param) {void *temp; temp=&host; param=param; /*these are here to bypass compiler warnings about unused arguments*/}
	virtual ~ArgumentVisitor() {}
};

class Argument
{
public:
	virtual string toString()=0;
	virtual void execute(ArgumentVisitor &host, void *param)=0;
	virtual Argument *clone()=0;
	virtual ~Argument() {}
};

class LiteralArgument : public Argument
{
public:
	LiteralArgument(long Value) : value(Value) {}
	string toString();
	void execute(ArgumentVisitor &host, void *param)
	{
		host.caseLiteral(*this, param);
	}
	Argument *clone() {return new LiteralArgument(value);}
private:
	long value;
};

class VarArgument : public Argument
{
public:
	VarArgument(int id) : ID(id) {}
	string toString();
	void execute(ArgumentVisitor &host, void *param)
	{
		host.caseVar(*this,param);
	}
	Argument *clone() {return new VarArgument(ID);}
private:
	int ID;
};

class GlobalArgument : public Argument
{
public:
	GlobalArgument(int id) : ID(id) {}
	string toString();
	void execute(ArgumentVisitor &host, void *param)
	{
		host.caseGlobal(*this,param);
	}
	Argument *clone() {return new GlobalArgument(ID);}
private:
	int ID;
};

class LabelArgument : public Argument
{
public:
	LabelArgument(int id) : ID(id), haslineno(false) {}
	string toString();
	void execute(ArgumentVisitor &host, void *param)
	{
		host.caseLabel(*this,param);
	}
	Argument *clone() {return new LabelArgument(ID);}
	int getID() {return ID;}
	void setLineNo(int l) {haslineno=true; lineno=l;}
private:
	int ID;
	int lineno;
	bool haslineno;
};

class UnaryOpcode : public Opcode
{
public:
	UnaryOpcode(Argument *A) : a(A) {}
	~UnaryOpcode() {delete a;}
	Argument *getArgument() {return a;}
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
	~BinaryOpcode() {delete a; delete b;}
	Argument *getFirstArgument() {return a;}
	Argument *getSecondArgument() {return b;}
	void execute(ArgumentVisitor &host, void *param)
	{
		a->execute(host, param);
		b->execute(host, param);
	}
protected:
	Argument *a;
	Argument *b;
};

class OSetImmediate : public BinaryOpcode
{
public:
	OSetImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OSetImmediate(a->clone(),b->clone());}
};

class OSetRegister : public BinaryOpcode
{
public:
	OSetRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OSetRegister(a->clone(),b->clone());}
};

class OAddImmediate : public BinaryOpcode
{
public:
	OAddImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OAddImmediate(a->clone(),b->clone());}
};

class OAddRegister : public BinaryOpcode
{
public:
	OAddRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OAddRegister(a->clone(),b->clone());}
};

class OSubImmediate : public BinaryOpcode
{
public:
	OSubImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OSubImmediate(a->clone(),b->clone());}
};

class OSubRegister : public BinaryOpcode
{
public:
	OSubRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OSubRegister(a->clone(),b->clone());}
};

class OMultImmediate : public BinaryOpcode
{
public:
	OMultImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OMultImmediate(a->clone(),b->clone());}
};

class OMultRegister : public BinaryOpcode
{
public:
	OMultRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OMultRegister(a->clone(),b->clone());}
};

class ODivImmediate : public BinaryOpcode
{
public:
	ODivImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new ODivImmediate(a->clone(),b->clone());}
};

class ODivRegister : public BinaryOpcode
{
public:
	ODivRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new ODivRegister(a->clone(),b->clone());}
};

class OCompareImmediate : public BinaryOpcode
{
public:
	OCompareImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OCompareImmediate(a->clone(),b->clone());}
};

class OCompareRegister : public BinaryOpcode
{
public:
	OCompareRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OCompareRegister(a->clone(),b->clone());}
};

class OWaitframe : public Opcode
{
public:
	string toString();
	Opcode *clone() {return new OWaitframe();}
};

class OGotoImmediate : public UnaryOpcode
{
public:
	OGotoImmediate(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OGotoImmediate(a->clone());}
};

class OGotoTrueImmediate: public UnaryOpcode
{
public:
	OGotoTrueImmediate(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OGotoTrueImmediate(a->clone());}
};

class OGotoFalseImmediate: public UnaryOpcode
{
public:
	OGotoFalseImmediate(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OGotoFalseImmediate(a->clone());}
};

class OGotoMoreImmediate : public UnaryOpcode
{
public:
	OGotoMoreImmediate(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OGotoMoreImmediate(a->clone());}
};

class OGotoLessImmediate : public UnaryOpcode
{
public:
	OGotoLessImmediate(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OGotoLessImmediate(a->clone());}
};

class OPushRegister : public UnaryOpcode
{
public:
	OPushRegister(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OPushRegister(a->clone());}
};

class OPopRegister : public UnaryOpcode
{
public:
	OPopRegister(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OPopRegister(a->clone());}
};

class OLoadIndirect : public BinaryOpcode
{
public:
	OLoadIndirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OLoadIndirect(a->clone(),b->clone());}
};

class OStoreIndirect : public BinaryOpcode
{
public:
	OStoreIndirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OStoreIndirect(a->clone(),b->clone());}
};

class OQuit : public Opcode
{
public:
	string toString();
	Opcode *clone() {return new OQuit();}
};

class OGotoRegister : public UnaryOpcode
{
public:
	OGotoRegister(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OGotoRegister(a->clone());}
};

class OTraceRegister : public UnaryOpcode
{
public:
	OTraceRegister(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OTraceRegister(a->clone());}
};

class OAndImmediate : public BinaryOpcode
{
public:
	OAndImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OAndImmediate(a->clone(),b->clone());}
};

class OAndRegister : public BinaryOpcode
{
public:
	OAndRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OAndRegister(a->clone(),b->clone());}
};

class OOrImmediate : public BinaryOpcode
{
public:
    OOrImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OOrImmediate(a->clone(),b->clone());}
};

class OOrRegister : public BinaryOpcode
{
public:
	OOrRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OOrRegister(a->clone(),b->clone());}
};

class OXorImmediate : public BinaryOpcode
{
public:
	OXorImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OXorImmediate(a->clone(), b->clone());}
};

class OXorRegister : public BinaryOpcode
{
public:
	OXorRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OXorRegister(a->clone(), b->clone());}
};

class ONot : public UnaryOpcode
{
public:
	ONot(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new ONot(a->clone());}
};

class OLShiftImmediate : public BinaryOpcode
{
public:
	OLShiftImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OLShiftImmediate(a->clone(), b->clone());}
};

class OLShiftRegister : public BinaryOpcode
{
public:
	OLShiftRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OLShiftRegister(a->clone(), b->clone());}
};

class ORShiftImmediate : public BinaryOpcode
{
public:
	ORShiftImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new ORShiftImmediate(a->clone(), b->clone());}
};

class ORShiftRegister : public BinaryOpcode
{
public:
	ORShiftRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new ORShiftRegister(a->clone(), b->clone());}
};

class OModuloImmediate : public BinaryOpcode
{
public:
	OModuloImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OModuloImmediate(a->clone(), b->clone());}
};

class OModuloRegister : public BinaryOpcode
{
public:
	OModuloRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OModuloRegister(a->clone(), b->clone());}
};

class OSinRegister : public BinaryOpcode
{
public:
	OSinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OSinRegister(a->clone(), b->clone());}
};

class OCosRegister : public BinaryOpcode
{
public:
	OCosRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OCosRegister(a->clone(), b->clone());}
};

class OTanRegister : public BinaryOpcode
{
public:
	OTanRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OTanRegister(a->clone(), b->clone());}
};

class OMaxRegister : public BinaryOpcode
{
public:
	OMaxRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OMaxRegister(a->clone(), b->clone());}
};

class OMinRegister : public BinaryOpcode
{
public:
	OMinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OMinRegister(a->clone(), b->clone());}
};

class OPowRegister : public BinaryOpcode
{
public:
	OPowRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OPowRegister(a->clone(), b->clone());}
};

class OInvPowRegister : public BinaryOpcode
{
public:
	OInvPowRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OInvPowRegister(a->clone(), b->clone());}
};

class OFactorial : public UnaryOpcode
{
public:
	OFactorial(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OFactorial(a->clone());}
};

class OAbsRegister : public UnaryOpcode
{
public:
	OAbsRegister(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OAbsRegister(a->clone());}
};

class OCheckTrig : public Opcode
{
public:
	string toString();
	Opcode *clone() {return new OCheckTrig();}
};

class ORandRegister : public BinaryOpcode
{
public:
	ORandRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new ORandRegister(a->clone(), b->clone());}
};

class OSqrtRegister : public BinaryOpcode
{
public:
	OSqrtRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OSqrtRegister(a->clone(),b->clone());}
};

class OWarp : public BinaryOpcode
{
public:
	OWarp(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OWarp(a->clone(), b->clone());}
};

class OPitWarp : public BinaryOpcode
{
public:
	OPitWarp(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
	string toString();
	Opcode *clone() {return new OPitWarp(a->clone(), b->clone());}
};

class OCreateItemRegister : public UnaryOpcode
{
public:
	OCreateItemRegister(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OCreateItemRegister(a->clone());}
};

class OPlaySoundRegister : public UnaryOpcode
{
public:
	OPlaySoundRegister(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone() {return new OPlaySoundRegister(a->clone());}
};

#endif

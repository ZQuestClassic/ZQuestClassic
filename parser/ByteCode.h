#ifndef BYTECODE_H
#define BYTECODE_H

#include "ScriptParser.h"
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
#define INPUTSENABLED 28
#define FORCEDUP 29
#define FORCEDDOWN 30
#define FORCEDLEFT 31
#define FORCEDRIGHT 32
#define FORCEDA 33
#define FORCEDB 34
#define FORCEDL 35
#define FORCEDR 36
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
#define GETA 66
#define GETB 67
#define GETL 68
#define GETR 69
#define GETUP 70
#define GETDOWN 71
#define GETLEFT 72
#define GETRIGHT 73
#define GETSTART 74
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

class LiteralArgument;
class VarArgument;
class LabelArgument;
class GlobalArgument;

class ArgumentVisitor
{
public:
	virtual void caseLiteral(LiteralArgument &host, void *param) {}
	virtual void caseVar(VarArgument &host, void *param) {}
	virtual void caseLabel(LabelArgument &host, void *param) {}
	virtual void caseGlobal(GlobalArgument &host, void *param) {}
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
	LiteralArgument(int value) : value(value) {}
	string toString();
	void execute(ArgumentVisitor &host, void *param)
	{
		host.caseLiteral(*this, param);
	}
	Argument *clone() {return new LiteralArgument(value);}
private:
	int value;
};

class VarArgument : public Argument
{
public:
	VarArgument(int ID) : ID(ID) {}
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
	GlobalArgument(int ID) : ID(ID) {}
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
	LabelArgument(int ID) : ID(ID), haslineno(false) {}
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
	UnaryOpcode(Argument *a) : a(a) {}
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
	BinaryOpcode(Argument *a, Argument *b) : a(a), b(b) {}
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
	OSetImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OSetImmediate(a->clone(),b->clone());}
};

class OSetRegister : public BinaryOpcode
{
public:
	OSetRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OSetRegister(a->clone(),b->clone());}
};

class OAddImmediate : public BinaryOpcode
{
public:
	OAddImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OAddImmediate(a->clone(),b->clone());}
};

class OAddRegister : public BinaryOpcode
{
public:
	OAddRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OAddRegister(a->clone(),b->clone());}
};

class OSubImmediate : public BinaryOpcode
{
public:
	OSubImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OSubImmediate(a->clone(),b->clone());}
};

class OSubRegister : public BinaryOpcode
{
public:
	OSubRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OSubRegister(a->clone(),b->clone());}
};

class OMultImmediate : public BinaryOpcode
{
public:
	OMultImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OMultImmediate(a->clone(),b->clone());}
};

class OMultRegister : public BinaryOpcode
{
public:
	OMultRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OMultRegister(a->clone(),b->clone());}
};

class ODivImmediate : public BinaryOpcode
{
public:
	ODivImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new ODivImmediate(a->clone(),b->clone());}
};

class ODivRegister : public BinaryOpcode
{
public:
	ODivRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new ODivRegister(a->clone(),b->clone());}
};

class OCompareImmediate : public BinaryOpcode
{
public:
	OCompareImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OCompareImmediate(a->clone(),b->clone());}
};

class OCompareRegister : public BinaryOpcode
{
public:
	OCompareRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
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
	OGotoImmediate(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OGotoImmediate(a->clone());}
};

class OGotoTrueImmediate: public UnaryOpcode
{
public:
	OGotoTrueImmediate(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OGotoTrueImmediate(a->clone());}
};

class OGotoFalseImmediate: public UnaryOpcode
{
public:
	OGotoFalseImmediate(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OGotoFalseImmediate(a->clone());}
};

class OGotoMoreImmediate : public UnaryOpcode
{
public:
	OGotoMoreImmediate(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OGotoMoreImmediate(a->clone());}
};

class OGotoLessImmediate : public UnaryOpcode
{
public:
	OGotoLessImmediate(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OGotoLessImmediate(a->clone());}
};

class OPushRegister : public UnaryOpcode
{
public:
	OPushRegister(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OPushRegister(a->clone());}
};

class OPopRegister : public UnaryOpcode
{
public:
	OPopRegister(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OPopRegister(a->clone());}
};

class OLoadIndirect : public BinaryOpcode
{
public:
	OLoadIndirect(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OLoadIndirect(a->clone(),b->clone());}
};

class OStoreIndirect : public BinaryOpcode
{
public:
	OStoreIndirect(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
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
	OGotoRegister(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OGotoRegister(a->clone());}
};

class OTraceRegister : public UnaryOpcode
{
public:
	OTraceRegister(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OTraceRegister(a->clone());}
};

class OAndImmediate : public BinaryOpcode
{
public:
	OAndImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OAndImmediate(a->clone(),b->clone());}
};

class OAndRegister : public BinaryOpcode
{
public:
	OAndRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OAndRegister(a->clone(),b->clone());}
};

class OOrImmediate : public BinaryOpcode
{
public:
    OOrImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OOrImmediate(a->clone(),b->clone());}
};

class OOrRegister : public BinaryOpcode
{
public:
	OOrRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OOrRegister(a->clone(),b->clone());}
};

class OXorImmediate : public BinaryOpcode
{
public:
	OXorImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OXorImmediate(a->clone(), b->clone());}
};

class OXorRegister : public BinaryOpcode
{
public:
	OXorRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OXorRegister(a->clone(), b->clone());}
};

class ONot : public UnaryOpcode
{
public:
	ONot(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new ONot(a->clone());}
};

class OLShiftImmediate : public BinaryOpcode
{
public:
	OLShiftImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OLShiftImmediate(a->clone(), b->clone());}
};

class OLShiftRegister : public BinaryOpcode
{
public:
	OLShiftRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OLShiftRegister(a->clone(), b->clone());}
};

class ORShiftImmediate : public BinaryOpcode
{
public:
	ORShiftImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new ORShiftImmediate(a->clone(), b->clone());}
};

class ORShiftRegister : public BinaryOpcode
{
public:
	ORShiftRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new ORShiftRegister(a->clone(), b->clone());}
};

class OModuloImmediate : public BinaryOpcode
{
public:
	OModuloImmediate(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OModuloImmediate(a->clone(), b->clone());}
};

class OModuloRegister : public BinaryOpcode
{
public:
	OModuloRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OModuloRegister(a->clone(), b->clone());}
};

class OSinRegister : public BinaryOpcode
{
public:
	OSinRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OSinRegister(a->clone(), b->clone());}
};

class OCosRegister : public BinaryOpcode
{
public:
	OCosRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OCosRegister(a->clone(), b->clone());}
};

class OTanRegister : public BinaryOpcode
{
public:
	OTanRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OTanRegister(a->clone(), b->clone());}
};

class OMaxRegister : public BinaryOpcode
{
public:
	OMaxRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OMaxRegister(a->clone(), b->clone());}
};

class OMinRegister : public BinaryOpcode
{
public:
	OMinRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OMinRegister(a->clone(), b->clone());}
};

class OPowRegister : public BinaryOpcode
{
public:
	OPowRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OPowRegister(a->clone(), b->clone());}
};

class OInvPowRegister : public BinaryOpcode
{
public:
	OInvPowRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OInvPowRegister(a->clone(), b->clone());}
};

class OFactorial : public UnaryOpcode
{
public:
	OFactorial(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OFactorial(a->clone());}
};

class OAbsRegister : public UnaryOpcode
{
public:
	OAbsRegister(Argument *a) : UnaryOpcode(a) {}
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
	ORandRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new ORandRegister(a->clone(), b->clone());}
};

class OSqrtRegister : public BinaryOpcode
{
public:
	OSqrtRegister(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OSqrtRegister(a->clone(),b->clone());}
};

class OWarp : public BinaryOpcode
{
public:
	OWarp(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OWarp(a->clone(), b->clone());}
};

class OPitWarp : public BinaryOpcode
{
public:
	OPitWarp(Argument *a, Argument *b) : BinaryOpcode(a,b) {}
	string toString();
	Opcode *clone() {return new OPitWarp(a->clone(), b->clone());}
};

class OCreateItemRegister : public UnaryOpcode
{
public:
	OCreateItemRegister(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OCreateItemRegister(a->clone());}
};

class OPlaySoundRegister : public UnaryOpcode
{
public:
	OPlaySoundRegister(Argument *a) : UnaryOpcode(a) {}
	string toString();
	Opcode *clone() {return new OPlaySoundRegister(a->clone());}
};

///////////////////////////////////////////////////////////////////////////////////
class BuildOpcodes : public RecursiveVisitor
{
public:
	virtual void caseDefault(void *param);
	virtual void caseFuncDecl(ASTFuncDecl &host, void *param);
	virtual void caseVarDecl(ASTVarDecl &host, void *param);
	virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param);
	virtual void caseExprAnd(ASTExprAnd &host, void *param);
	virtual void caseExprOr(ASTExprOr &host, void *param);
	virtual void caseExprGT(ASTExprGT &host, void *param);
	virtual void caseExprGE(ASTExprGE &host, void *param);
	virtual void caseExprLT(ASTExprLT &host, void *param);
	virtual void caseExprLE(ASTExprLE &host, void *param);
	virtual void caseExprEQ(ASTExprEQ &host, void *param);
	virtual void caseExprNE(ASTExprNE &host, void *param);
	virtual void caseExprPlus(ASTExprPlus &host, void *param);
	virtual void caseExprMinus(ASTExprMinus &host, void *param);
	virtual void caseExprTimes(ASTExprTimes &host, void *param);
	virtual void caseExprDivide(ASTExprDivide &host, void *param);
	virtual void caseExprNot(ASTExprNot &host, void *param);
	virtual void caseExprNegate(ASTExprNegate &host, void *param);
	virtual void caseFuncCall(ASTFuncCall &host, void *param);
	virtual void caseStmtAssign(ASTStmtAssign &host, void *param);
	virtual void caseExprDot(ASTExprDot &host, void *param);
	virtual void caseExprArrow(ASTExprArrow &host, void *param);
	virtual void caseStmtFor(ASTStmtFor &host, void *param);
	virtual void caseStmtIf(ASTStmtIf &host, void *param);
	virtual void caseStmtIfElse(ASTStmtIfElse &host, void *param);
	virtual void caseStmtReturn(ASTStmtReturn &host, void *param);
	virtual void caseStmtReturnVal(ASTStmtReturnVal &host, void *param);
	virtual void caseStmtEmpty(ASTStmtEmpty &host, void *param);
	virtual void caseNumConstant(ASTNumConstant &host, void *param);
	virtual void caseBoolConstant(ASTBoolConstant &host, void *param);
	virtual void caseStmtWhile(ASTStmtWhile &host, void *param);
	virtual void caseExprBitOr(ASTExprBitOr &host, void *param);
	virtual void caseExprBitXor(ASTExprBitXor &host, void *param);
	virtual void caseExprBitAnd(ASTExprBitAnd &host, void *param);
	virtual void caseExprLShift(ASTExprLShift &host, void *param);
	virtual void caseExprRShift(ASTExprRShift &host, void *param);
	virtual void caseExprModulo(ASTExprModulo &host, void *param);
	virtual void caseExprBitNot(ASTExprBitNot &host, void *param);
	virtual void caseExprIncrement(ASTExprIncrement &host, void *param);
	virtual void caseExprDecrement(ASTExprDecrement &host, void *param);
	vector<Opcode *> getResult() {return result;}
	int getReturnLabelID() {return returnlabelid;}
private:
	vector<Opcode *> result;
	int returnlabelid;
};

class CountStackSymbols : public RecursiveVisitor
{
public:
	virtual void caseDefault(void *param) {}
	virtual void caseVarDecl(ASTVarDecl &host, void *param)
	{
		pair<vector<int> *, SymbolTable *> *p = (pair<vector<int> *, SymbolTable *> *)param;
		int vid = p->second->getID(&host);
		p->first->push_back(vid);
	}
	virtual void caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
	{
		caseVarDecl(host, param);
	}
};

class LValBOHelper : public ASTVisitor
{
public:
	virtual void caseDefault(void *param);
	virtual void caseExprDot(ASTExprDot &host, void *param);
	virtual void caseExprArrow(ASTExprArrow &host, void *param);
	virtual void caseVarDecl(ASTVarDecl &host, void *param);
	vector<Opcode *> getResult() {return result;}
private:
	vector<Opcode *> result;
};

class GetLabels : public ArgumentVisitor
{
public:
	void caseLabel(LabelArgument &host, void *param)
	{
		map<int,bool> *labels = (map<int,bool> *)param;
		(*labels)[host.getID()] = true;
	}
};

class SetLabels : public ArgumentVisitor
{
public:
	void caseLabel(LabelArgument &host, void *param)
	{
		map<int, int> *labels = (map<int, int> *)param;
		int lineno = (*labels)[host.getID()];
		if(lineno==0)
		{
			char temp[200];
			sprintf(temp,"Internal error: couldn't find function label %d", host.getID());
			box_out(temp);
			box_eol();
		}
		host.setLineNo(lineno);
	}
};

#endif

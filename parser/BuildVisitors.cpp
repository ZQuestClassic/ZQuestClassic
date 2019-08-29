#include "BuildVisitors.h"
#include <assert.h>
#include "ParseError.h"

void BuildOpcodes::caseDefault(void *param)
{
  //these are here to bypass compiler warnings about unused arguments
  param=param;

	//unreachable
	assert(false);
}

void BuildOpcodes::caseFuncDecl(ASTFuncDecl &host, void *param)
{
	returnlabelid = ScriptParser::getUniqueLabelID();
	host.getBlock()->execute(*this,param);
}

void BuildOpcodes::caseVarDecl(ASTVarDecl &host, void *param)
{
	//initialize to 0
	OpcodeContext *c = (OpcodeContext *)param;
	int globalid = c->linktable->getGlobalID(c->symbols->getID(&host));
	if(globalid != -1)
	{
		//it's a global var
		//just set it to 0
		result.push_back(new OSetImmediate(new GlobalArgument(globalid), new LiteralArgument(0)));
		return;
	}
	//it's a local var
	//set it to 0 on the stack
	int offset = c->stackframe->getOffset(c->symbols->getID(&host));
	result.push_back(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
	result.push_back(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}

void BuildOpcodes::caseVarDeclInitializer(ASTVarDeclInitializer &host, void *param)
{
	OpcodeContext *c = (OpcodeContext *)param;
	//compute the value of the initializer
	host.getInitializer()->execute(*this,param);
	//value of expr now stored int EXP1
	int globalid = c->linktable->getGlobalID(c->symbols->getID(&host));
	if(globalid != -1)
	{
		//it's a global var
		result.push_back(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
		return;
	}
	//it's a local var
	//set it on the stack
	int offset = c->stackframe->getOffset(c->symbols->getID(&host));
	result.push_back(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
	result.push_back(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
	result.push_back(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}
	
void BuildOpcodes::caseExprAnd(ASTExprAnd &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	int falselabel = ScriptParser::getUniqueLabelID();
	int endlabel = ScriptParser::getUniqueLabelID();
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoTrueImmediate(new LabelArgument(falselabel)));
	result.push_back(new OCompareImmediate(new VarArgument(EXP2), new LiteralArgument(0)));
	result.push_back(new OGotoTrueImmediate(new LabelArgument(falselabel)));
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1)));
	result.push_back(new OGotoImmediate(new LabelArgument(endlabel)));
	Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
	next->setLabel(falselabel);
	result.push_back(next);
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(endlabel);
	result.push_back(next);
}

void BuildOpcodes::caseExprOr(ASTExprOr &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	int truelabel = ScriptParser::getUniqueLabelID();
	int endlabel = ScriptParser::getUniqueLabelID();
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoFalseImmediate(new LabelArgument(truelabel)));
	result.push_back(new OCompareImmediate(new VarArgument(EXP2), new LiteralArgument(0)));
	result.push_back(new OGotoFalseImmediate(new LabelArgument(truelabel)));
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoImmediate(new LabelArgument(endlabel)));
	Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1));
	next->setLabel(truelabel);
	result.push_back(next);
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(endlabel);
	result.push_back(next);
}

void BuildOpcodes::caseExprGT(ASTExprGT &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	int checkeqlabel = ScriptParser::getUniqueLabelID();
	int truelabel = ScriptParser::getUniqueLabelID();
	int donelabel = ScriptParser::getUniqueLabelID();
	result.push_back(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	result.push_back(new OGotoMoreImmediate(new LabelArgument(checkeqlabel)));
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoImmediate(new LabelArgument(donelabel)));
	Opcode *next = new OGotoFalseImmediate(new LabelArgument(truelabel));
	next->setLabel(checkeqlabel);
	result.push_back(next);
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoImmediate(new LabelArgument(donelabel)));
	next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1));
	next->setLabel(truelabel);
	result.push_back(next);
	//noop just for label purposes
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(donelabel);
	result.push_back(next);
}
	
void BuildOpcodes::caseExprGE(ASTExprGE &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	int truelabel = ScriptParser::getUniqueLabelID();
	int donelabel = ScriptParser::getUniqueLabelID();
	result.push_back(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	result.push_back(new OGotoMoreImmediate(new LabelArgument(truelabel)));
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoImmediate(new LabelArgument(donelabel)));
	Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1));
	next->setLabel(truelabel);
	result.push_back(next);
	//noop just for label purposes
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(donelabel);
	result.push_back(next);
}

void BuildOpcodes::caseExprLT(ASTExprLT &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	int truelabel = ScriptParser::getUniqueLabelID();
	int checkeqlabel = ScriptParser::getUniqueLabelID();
	int donelabel = ScriptParser::getUniqueLabelID();
	result.push_back(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	result.push_back(new OGotoLessImmediate(new LabelArgument(checkeqlabel)));
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoImmediate(new LabelArgument(donelabel)));
	Opcode *next = new OGotoFalseImmediate(new LabelArgument(truelabel));
	next->setLabel(checkeqlabel);
	result.push_back(next);
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoImmediate(new LabelArgument(donelabel)));
	next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1));
	next->setLabel(truelabel);
	result.push_back(next);
	//noop just for label purposes
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(donelabel);
	result.push_back(next);
}

void BuildOpcodes::caseExprLE(ASTExprLE &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	int truelabel = ScriptParser::getUniqueLabelID();
	int donelabel = ScriptParser::getUniqueLabelID();
	result.push_back(new OCompareRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	result.push_back(new OGotoLessImmediate(new LabelArgument(truelabel)));
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoImmediate(new LabelArgument(donelabel)));
	Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1));
	next->setLabel(truelabel);
	result.push_back(next);
	//noop just for label purposes
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(donelabel);
	result.push_back(next);
}

void BuildOpcodes::caseExprEQ(ASTExprEQ &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	int truelabel = ScriptParser::getUniqueLabelID();
	int donelabel = ScriptParser::getUniqueLabelID();
	result.push_back(new OCompareRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	result.push_back(new OGotoTrueImmediate(new LabelArgument(truelabel)));
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoImmediate(new LabelArgument(donelabel)));
	Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1));
	next->setLabel(truelabel);
	result.push_back(next);
	//noop just for label purposes
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(donelabel);
	result.push_back(next);
}

void BuildOpcodes::caseExprNE(ASTExprNE &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	int truelabel = ScriptParser::getUniqueLabelID();
	int donelabel = ScriptParser::getUniqueLabelID();
	result.push_back(new OCompareRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
	result.push_back(new OGotoFalseImmediate(new LabelArgument(truelabel)));
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoImmediate(new LabelArgument(donelabel)));
	Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1));
	next->setLabel(truelabel);
	result.push_back(next);
	//noop just for label purposes
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(donelabel);
	result.push_back(next);
}

void BuildOpcodes::caseExprPlus(ASTExprPlus &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new OAddRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprMinus(ASTExprMinus &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new OSubRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	result.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprTimes(ASTExprTimes &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new OMultRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprDivide(ASTExprDivide &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new ODivRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	result.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprModulo(ASTExprModulo &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new OModuloRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	result.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprBitOr(ASTExprBitOr &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new OOrRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprBitXor(ASTExprBitXor &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new OXorRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprBitAnd(ASTExprBitAnd &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new OAndRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprLShift(ASTExprLShift &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new OLShiftRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	result.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprRShift(ASTExprRShift &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	//compute both sides
	host.getFirstOperand()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	host.getSecondOperand()->execute(*this,param);
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	result.push_back(new ORShiftRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	result.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprNot(ASTExprNot &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	host.getOperand()->execute(*this,param);
	result.push_back(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	int truelabel = ScriptParser::getUniqueLabelID();
	int donelabel = ScriptParser::getUniqueLabelID();
	result.push_back(new OGotoTrueImmediate(new LabelArgument(truelabel)));
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoImmediate(new LabelArgument(donelabel)));
	Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(1));
	next->setLabel(truelabel);
	result.push_back(next);
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(donelabel);
	result.push_back(next);
}

void BuildOpcodes::caseExprNegate(ASTExprNegate &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	host.getOperand()->execute(*this,param);
	result.push_back(new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0)));
	result.push_back(new OSubRegister(new VarArgument(EXP2), new VarArgument(EXP1)));
	result.push_back(new OSetRegister(new VarArgument(EXP1), new VarArgument(EXP2)));
}

void BuildOpcodes::caseExprBitNot(ASTExprBitNot &host, void *param)
{
	if(host.hasIntValue())
	{
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
		return;
	}
	host.getOperand()->execute(*this,param);
	result.push_back(new ONot(new VarArgument(EXP1)));
}

void BuildOpcodes::caseExprIncrement(ASTExprIncrement &host, void *param)
{
	//annoying
	OpcodeContext *c = (OpcodeContext *)param;
	//load value of the variable into EXP1
	//except if it is an arrow expr, in which case the gettor function is stored
	//in this AST*
	bool isdotexpr;
	IsDotExpr temp;
	host.getOperand()->execute(temp, &isdotexpr);
	if(isdotexpr)
	{
		host.getOperand()->execute(*this,param);
	}
	else
	{
		int oldid = c->symbols->getID(host.getOperand());
		c->symbols->putAST(host.getOperand(), c->symbols->getID(&host));
		host.getOperand()->execute(*this,param);
		c->symbols->putAST(host.getOperand(), oldid);
	}
	
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	
	//increment EXP1
	result.push_back(new OAddImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
	//store it
	LValBOHelper helper;
	host.getOperand()->execute(helper, param);
	vector<Opcode *> subcode = helper.getResult();
	for(vector<Opcode *>::iterator it = subcode.begin(); it != subcode.end(); it++)
	{
		result.push_back(*it);
	}
	//pop EXP1
	result.push_back(new OPopRegister(new VarArgument(EXP1)));
}
void BuildOpcodes::caseExprDecrement(ASTExprDecrement &host, void *param)
{
	//annoying
	OpcodeContext *c = (OpcodeContext *)param;
	//load value of the variable into EXP1
	//except if it is an arrow expr, in which case the gettor function is stored
	//in this AST*
	bool isdotexpr;
	IsDotExpr temp;
	host.getOperand()->execute(temp, &isdotexpr);
	if(isdotexpr)
	{
		host.getOperand()->execute(*this,param);
	}
	else
	{
		int oldid = c->symbols->getID(host.getOperand());
		c->symbols->putAST(host.getOperand(), c->symbols->getID(&host));
		host.getOperand()->execute(*this,param);
		c->symbols->putAST(host.getOperand(), oldid);
	}
	
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	
	//decrement EXP1
	result.push_back(new OSubImmediate(new VarArgument(EXP1), new LiteralArgument(10000)));
	//store it
	LValBOHelper helper;
	host.getOperand()->execute(helper, param);
	vector<Opcode *> subcode = helper.getResult();
	for(vector<Opcode *>::iterator it = subcode.begin(); it != subcode.end(); it++)
	{
		result.push_back(*it);
	}
	//pop EXP1
	result.push_back(new OPopRegister(new VarArgument(EXP1)));
}
void BuildOpcodes::caseFuncCall(ASTFuncCall &host, void *param)
{
	OpcodeContext *c = (OpcodeContext *)param;
	int funclabel = c->linktable->functionToLabel(c->symbols->getID(&host));
	//push the stack frame pointer
	result.push_back(new OPushRegister(new VarArgument(SFRAME)));
	//push the return address
	int returnaddr = ScriptParser::getUniqueLabelID();
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LabelArgument(returnaddr)));
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	//if the function is a pointer function (->func()) we need to push the left-hand-side
	bool isdotexpr;
	IsDotExpr temp;
	host.getName()->execute(temp, &isdotexpr);
	if(!isdotexpr)
	{
		//load the value of the left-hand of the arrow into EXP1
		((ASTExprArrow *)host.getName())->getLVal()->execute(*this,param);
		//host.getName()->execute(*this,param);
		//push it onto the stack
		result.push_back(new OPushRegister(new VarArgument(EXP1)));	
	}
	//push the parameters, in forward order
	for(list<ASTExpr *>::iterator it = host.getParams().begin(); it != host.getParams().end(); it++)
	{
		(*it)->execute(*this,param);
		result.push_back(new OPushRegister(new VarArgument(EXP1)));
	}
	//goto
	result.push_back(new OGotoImmediate(new LabelArgument(funclabel)));
	//pop the stack frame pointer
	Opcode *next = new OPopRegister(new VarArgument(SFRAME));
	next->setLabel(returnaddr);
	result.push_back(next);
}
	
void BuildOpcodes::caseStmtAssign(ASTStmtAssign &host, void *param)
{
	//load the rval into EXP1
	host.getRVal()->execute(*this,param);
	//and store it
	LValBOHelper helper;
	host.getLVal()->execute(helper, param);
	vector<Opcode *> subcode = helper.getResult();
	for(vector<Opcode *>::iterator it = subcode.begin(); it != subcode.end(); it++)
	{
		result.push_back(*it);
	}
}

void BuildOpcodes::caseExprDot(ASTExprDot &host, void *param)
{
	OpcodeContext *c = (OpcodeContext *)param;
	int vid = c->symbols->getID(&host);
	if(vid == -1)
	{
		//constant, just load its value
		result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(c->symbols->getConstantVal(host.getName()))));
		return;
	}
	int globalid = c->linktable->getGlobalID(vid);
	if(globalid != -1)
	{
		//global variable, so just get its value
		result.push_back(new OSetRegister(new VarArgument(EXP1), new GlobalArgument(globalid)));
		return;
	}
	//local variable, get its value from the stack
	int offset = c->stackframe->getOffset(vid);
	result.push_back(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
	result.push_back(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
	result.push_back(new OLoadIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}

void BuildOpcodes::caseExprArrow(ASTExprArrow &host, void *param)
{
	OpcodeContext *c = (OpcodeContext *)param;
	bool isIndexed = host.getIndex() != NULL;
	//this is like actually a function call
	//to the appropriate gettor method
	//so, set that up:
	//push the stack frame
	result.push_back(new OPushRegister(new VarArgument(SFRAME)));
	int returnlabel = ScriptParser::getUniqueLabelID();
	//push the return address
	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LabelArgument(returnlabel)));
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	//get the rhs of the arrow
	host.getLVal()->execute(*this,param);
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	//if indexed, push the index
	if(isIndexed)
	{
		host.getIndex()->execute(*this,param);
		result.push_back(new OPushRegister(new VarArgument(EXP1)));
	}
	//call the function
	int label = c->linktable->functionToLabel(c->symbols->getID(&host));
	result.push_back(new OGotoImmediate(new LabelArgument(label)));
	//pop the stack frame
	Opcode *next = new OPopRegister(new VarArgument(SFRAME));
	next->setLabel(returnlabel);
	result.push_back(next);
}

void BuildOpcodes::caseStmtFor(ASTStmtFor &host, void *param)
{
	//run the precondition
	host.getPrecondition()->execute(*this,param);
	int loopstart = ScriptParser::getUniqueLabelID();
	int loopend = ScriptParser::getUniqueLabelID();
	int loopincr = ScriptParser::getUniqueLabelID();
	//nop
	Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
	next->setLabel(loopstart);
	result.push_back(next);
	//test the termination condition
	host.getTerminationCondition()->execute(*this,param);
	result.push_back(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoTrueImmediate(new LabelArgument(loopend)));
	//run the loop body
	//save the old break and continue values
	int oldbreak = breaklabelid;
	int oldcontinue = continuelabelid;
	breaklabelid = loopend;
	continuelabelid = loopincr;
	host.getStmt()->execute(*this,param);
	breaklabelid = oldbreak;
	continuelabelid = oldcontinue;
	//run the increment
	//nop
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(loopincr);
	result.push_back(next);
	host.getIncrement()->execute(*this,param);
	result.push_back(new OGotoImmediate(new LabelArgument(loopstart)));
	//nop
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(loopend);
	result.push_back(next);
}
	
void BuildOpcodes::caseStmtIf(ASTStmtIf &host, void *param)
{
	//run the test
	host.getCondition()->execute(*this,param);
	int endif = ScriptParser::getUniqueLabelID();
	result.push_back(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoTrueImmediate(new LabelArgument(endif)));
	//run the block
	host.getStmt()->execute(*this,param);
	//nop
	Opcode *next = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
	next->setLabel(endif);
	result.push_back(next);
}

void BuildOpcodes::caseStmtWhile(ASTStmtWhile &host, void *param)
{
	int startlabel = ScriptParser::getUniqueLabelID();
	int endlabel = ScriptParser::getUniqueLabelID();
	//run the test
	//nop to label start
	Opcode *start = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
	start->setLabel(startlabel);
	result.push_back(start);
	host.getCond()->execute(*this,param);
	result.push_back(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoTrueImmediate(new LabelArgument(endlabel)));
	int oldbreak = breaklabelid;
	int oldcontinue = continuelabelid;
	breaklabelid = endlabel;
	continuelabelid = startlabel;
	host.getStmt()->execute(*this,param);
	breaklabelid = oldbreak;
	continuelabelid = oldcontinue;
	result.push_back(new OGotoImmediate(new LabelArgument(startlabel)));
	//nop to end while
	Opcode *end = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
	end->setLabel(endlabel);
	result.push_back(end);
}

void BuildOpcodes::caseStmtDo(ASTStmtDo &host, void *param)
{
	int startlabel = ScriptParser::getUniqueLabelID();
	int endlabel = ScriptParser::getUniqueLabelID();
	int continuelabel = ScriptParser::getUniqueLabelID();
	//nop to label start
	Opcode *start = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
	start->setLabel(startlabel);
	result.push_back(start);
	
	int oldbreak = breaklabelid;
	int oldcontinue = continuelabelid;
	breaklabelid = endlabel;
	continuelabelid = continuelabel;
	host.getStmt()->execute(*this,param);
	breaklabelid = oldbreak;
	continuelabelid = oldcontinue;
	
	start = new OSetImmediate(new VarArgument(NUL), new LiteralArgument(0));
	start->setLabel(continuelabel);
	result.push_back(start);
	host.getCond()->execute(*this,param);
	result.push_back(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoTrueImmediate(new LabelArgument(endlabel)));
	result.push_back(new OGotoImmediate(new LabelArgument(startlabel)));
	//nop to end dowhile
	Opcode *end = new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(0));
	end->setLabel(endlabel);
	result.push_back(end);
}
	
void BuildOpcodes::caseStmtIfElse(ASTStmtIfElse &host, void *param)
{
	//run the test
	host.getCondition()->execute(*this,param);
	int elseif = ScriptParser::getUniqueLabelID();
	int endif = ScriptParser::getUniqueLabelID();
	result.push_back(new OCompareImmediate(new VarArgument(EXP1), new LiteralArgument(0)));
	result.push_back(new OGotoTrueImmediate(new LabelArgument(elseif)));
	//run if blocl
	host.getStmt()->execute(*this,param);
	result.push_back(new OGotoImmediate(new LabelArgument(endif)));
	Opcode *next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(elseif);
	result.push_back(next);
	host.getElseStmt()->execute(*this,param);
	next = new OSetImmediate(new VarArgument(EXP2), new LiteralArgument(0));
	next->setLabel(endif);
	result.push_back(next);
}
	
void BuildOpcodes::caseStmtReturn(ASTStmtReturn &host, void *param)
{
  //these are here to bypass compiler warnings about unused arguments
  void *temp;
  temp=&host;
  param=param;

	result.push_back(new OGotoImmediate(new LabelArgument(returnlabelid)));
}

void BuildOpcodes::caseStmtReturnVal(ASTStmtReturnVal &host, void *param)
{
	host.getReturnValue()->execute(*this,param);
	result.push_back(new OGotoImmediate(new LabelArgument(returnlabelid)));
}

void BuildOpcodes::caseStmtEmpty(ASTStmtEmpty &host, void *param)
{
  //these are here to bypass compiler warnings about unused arguments
  void *temp;
  temp=&host;
  param=param;

	//empty
}
	
void BuildOpcodes::caseNumConstant(ASTNumConstant &host, void *param)
{
  //these are here to bypass compiler warnings about unused arguments
  param=param;

	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
}
	
void BuildOpcodes::caseBoolConstant(ASTBoolConstant &host, void *param)
{
  //these are here to bypass compiler warnings about unused arguments
  param=param;

	result.push_back(new OSetImmediate(new VarArgument(EXP1), new LiteralArgument(host.getIntValue())));
}

void BuildOpcodes::caseStmtBreak(ASTStmtBreak &host, void *param)
{
  //these are here to bypass compiler warnings about unused arguments
  param=param;

	if(breaklabelid == -1)
	{
		printErrorMsg(&host, BREAKBAD);
		failure = true;
		return;
	}
	result.push_back(new OGotoImmediate(new LabelArgument(breaklabelid)));
}

void BuildOpcodes::caseStmtContinue(ASTStmtContinue &host, void *param)
{
  //these are here to bypass compiler warnings about unused arguments
  param=param;

	if(continuelabelid == -1)
	{
		printErrorMsg(&host, CONTINUEBAD);
		failure = true;
		return;
	}
	result.push_back(new OGotoImmediate(new LabelArgument(continuelabelid)));
}
/////////////////////////////////////////////////////////////////////////////////
void LValBOHelper::caseDefault(void *param)
{
  //these are here to bypass compiler warnings about unused arguments
  param=param;

	assert(false);
}

void LValBOHelper::caseExprDot(ASTExprDot &host, void *param)
{
	OpcodeContext *c = (OpcodeContext *)param;
	int vid = c->symbols->getID(&host);
	int globalid = c->linktable->getGlobalID(vid);
	if(globalid != -1)
	{
		//global variable
		result.push_back(new OSetRegister(new GlobalArgument(globalid), new VarArgument(EXP1)));
		return;
	}
	//set the stack
	int offset = c->stackframe->getOffset(vid);
	
	result.push_back(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
	result.push_back(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
	result.push_back(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}

void LValBOHelper::caseExprArrow(ASTExprArrow &host, void *param)
{
	OpcodeContext *c = (OpcodeContext *)param;
	int isIndexed = (host.getIndex() != NULL);
	//this is actually implemented as a settor function call
	//so do that
	//push the stack frame
	result.push_back(new OPushRegister(new VarArgument(SFRAME)));
	int returnlabel = ScriptParser::getUniqueLabelID();
	//push the return address
	result.push_back(new OSetImmediate(new VarArgument(EXP2), new LabelArgument(returnlabel)));
	result.push_back(new OPushRegister(new VarArgument(EXP2)));
	//push the lhs of the arrow
	//but first save the value of EXP1
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	vector<Opcode *> toadd;
	BuildOpcodes oc;
	host.getLVal()->execute(oc, param);
	toadd = oc.getResult();
	for(vector<Opcode *>::iterator it = toadd.begin(); it != toadd.end(); it++)
	{
		result.push_back(*it);
	}
	//pop the old value of EXP1
	result.push_back(new OPopRegister(new VarArgument(EXP2)));
	//and push the lhs
	result.push_back(new OPushRegister(new VarArgument(EXP1)));
	//and push the old value of EXP1
	result.push_back(new OPushRegister(new VarArgument(EXP2)));
	//and push the index, if indexed
	if(isIndexed)
	{
		BuildOpcodes oc2;
		host.getIndex()->execute(oc2, param);
		toadd = oc2.getResult();
		for(vector<Opcode *>::iterator it = toadd.begin(); it != toadd.end(); it++)
		{
			result.push_back(*it);
		}
		result.push_back(new OPushRegister(new VarArgument(EXP1)));
	}
	//finally, goto!
	int label = c->linktable->functionToLabel(c->symbols->getID(&host));
	result.push_back(new OGotoImmediate(new LabelArgument(label)));
	//pop the stack frame
	Opcode *next = new OPopRegister(new VarArgument(SFRAME));
	next->setLabel(returnlabel);
	result.push_back(next);
}

void LValBOHelper::caseVarDecl(ASTVarDecl &host, void *param)
{
	//cannot be a global variable, so just stuff it in the stack
	OpcodeContext *c = (OpcodeContext *)param;
	int vid = c->symbols->getID(&host);
	int offset = c->stackframe->getOffset(vid);
	result.push_back(new OSetRegister(new VarArgument(SFTEMP), new VarArgument(SFRAME)));
	result.push_back(new OAddImmediate(new VarArgument(SFTEMP), new LiteralArgument(offset)));
	result.push_back(new OStoreIndirect(new VarArgument(EXP1), new VarArgument(SFTEMP)));
}

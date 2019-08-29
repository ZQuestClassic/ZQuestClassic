#include "ByteCode.h"
#include "DataStructs.h"
#include "ParseError.h"
#include "../zsyssimple.h"
#include <assert.h>
#include <iostream>

string LiteralArgument::toString()
{
	char temp[40];
	string sign = value < 0 ? "-" : "";
	sprintf(temp,"%ld", abs(value)/10000);
	string first = string(temp);
	if(value % 10000 == 0)
		return sign + first;
	sprintf(temp,"%ld", abs(value)%10000);
	string second = string(temp);
	while(second.length() < 4)
		second = "0" + second;
	return sign + first + "." + second;
}

string VarArgument::toString()
{
	char temp[40];
	switch(ID)
	{
	case SP:
		return "SP";
	case DATA:
		return "DATA";
	case FCSET:
		return "CSET";
	case DELAY:
		return "DELAY";
	case FX:
		return "X";
	case FY:
		return "Y";
	case XD:
		return "XD";
	case YD:
		return "YD";
	case XD2:
		return "XD2";
	case YD2:
		return "YD2";
	case LINKX:
		return "LINKX";
	case LINKY:
		return "LINKY";
	case LINKDIR:
		return "LINKDIR";
	case LINKHP:
		return "LINKHP";
	case LINKMP:
		return "LINKMP";
	case LINKMAXHP:
		return "LINKMAXHP";
	case LINKMAXMP:
		return "LINKMAXMP";
	case LINKACTION:
		return "LINKACTION";
	case INPUTSTART:
		return "INPUTSTART";
	case INPUTUP:
		return "INPUTUP";
	case INPUTDOWN:
		return "INPUTDOWN";
	case INPUTLEFT:
		return "INPUTLEFT";
	case INPUTRIGHT:
		return "INPUTRIGHT";
	case INPUTA:
		return "INPUTA";
	case INPUTB:
		return "INPUTB";
	case INPUTL:
		return "INPUTL";
	case INPUTR:
		return "INPUTR";
	case SDD:
		return "SDD";
	case COMBODD:
		return "COMBODD";
	case COMBOCD:
		return "COMBOCD";
	case COMBOFD:
		return "COMBOFD";
	case REFFFC:
		return "REFFFC";
	case REFITEM:
		return "REFITEM";
	case ITEMCOUNT:
		return "ITEMCOUNT";
	case ITEMX:
		return "ITEMX";
	case ITEMY:
		return "ITEMY";
	case ITEMDRAWTYPE:
		return "ITEMDRAWTYPE";
	case ITEMID:
		return "ITEMID";
	case ITEMTILE:
		return "ITEMTILE";
	case ITEMCSET:
		return "ITEMCSET";
	case ITEMFLASHCSET:
		return "ITEMFLASHCSET";
	case ITEMFRAMES:
		return "ITEMFRAMES";
	case ITEMFRAME:
		return "ITEMFRAME";
	case ITEMASPEED:
		return "ITEMASPEED";
	case ITEMDELAY:
		return "ITEMDELAY";
	case ITEMFLASH:
		return "ITEMFLASH";
	case ITEMFLIP:
		return "ITEMFLIP";
	case ITEMEXTEND:
		return "ITEMEXTEND";
	case ITEMCLASSFAMILY:
		return "ICLASSFAMILY";
	case ITEMCLASSFAMTYPE:
		return "ICLASSFAMTYPE";
	case ITEMCLASSAMOUNT:
		return "ICLASSAMOUNT";
	case ITEMCLASSMAX:
		return "ICLASSMAX";
	case ITEMCLASSSETMAX:
		return "ICLASSSETMAX";
	case ITEMCLASSSETGAME:
		return "ICLASSSETGAME";
	case ITEMCLASSCOUNTER:
		return "ICLASSCOUNTER";
	case REFITEMCLASS:
		return "REFITEMCLASS";
	case COMBOID:
		return "COMBOID";
	case COMBOTD:
		return "COMBOTD";
	case COMBOSD:
		return "COMBOSD";
	case CURSCR:
		return "CURSCR";
	case CURMAP:
		return "CURMAP";
	case CURDMAP:
		return "CURDMAP";
	case GAMEDEATHS:
		return "GAMEDEATHS";
	case GAMECHEAT:
		return "GAMECHEAT";
	case GAMETIME:
		return "GAMETIME";
	case GAMEHASPLAYED:
		return "GAMEHASPLAYED";
	case GAMETIMEVALID:
		return "GAMETIMEVALID";
	case GAMEGUYCOUNT:
		return "GAMEGUYCOUNT";
	case GAMECONTSCR:
		return "GAMECONTSCR";
	case GAMECONTDMAP:
		return "GAMECONTDMAP";
	case GAMECOUNTERD:
		return "GAMECOUNTERD";
	case GAMEMCOUNTERD:
		return "GAMEMCOUNTERD";
	case GAMEDCOUNTERD:
		return "GAMEDCOUNTERD";
	case GAMEGENERICD:
		return "GAMEGENERICD";
	case GAMEITEMSD:
		return "GAMEITEMSD";
	case GAMELITEMSD:
		return "GAMELITEMSD";
	case GAMELKEYSD:
		return "GAMELKEYSD";
	case GAMEMAPFLAGD:
		return "GAMEMAPFLAGD";
	case GAMEMAPFLAGDD:
		return "GAMEMAPFLAGDD";
	case SDDD:
		return "SDDD";
	case FFFLAGSD:
		return "FFFLAGSD";
	case FFTWIDTH:
		return "FFTWIDTH";
	case FFTHEIGHT:
		return "FFTHEIGHT";
	case FFCWIDTH:
		return "FFCWIDTH";
	case FFCHEIGHT:
		return "FFCHEIGHT";
	case FFLINK:
		return "FFLINK";
	case LINKITEMD:
		return "LINKITEMD";
	case REFNPC:
		return "REFNPC";
	case NPCCOUNT:
		return "NPCCOUNT";
	case NPCX:
		return "NPCX";
	case NPCY:
		return "NPCY";
	case NPCDIR:
		return "NPCDIR";
	case NPCRATE:
		return "NPCRATE";
	case NPCFRAMERATE:
		return "NPCFRAMERATE";
	case NPCHALTRATE:
		return "NPCHALTRATE";
	case NPCDRAWTYPE:
		return "NPCDRAWTYPE";
	case NPCHP:
		return "NPCHP";
	case NPCDP:
		return "NPCDP";
	case NPCWDP:
		return "NPCWDP";
	case NPCTILE:
		return "NPCTILE";
	case NPCWEAPON:
		return "NPCWEAPON";
	case NPCITEMSET:
		return "NPCITEMSET";
	case NPCCSET:
		return "NPCCSET";
	case NPCBOSSPAL:
		return "NPCBOSSPAL";
	case NPCBGSFX:
		return "NPCBGSFX";
	case NPCEXTEND:
		return "NPCEXTEND";
	default:
		{
			sprintf(temp, "d%d", ID);
			return string(temp);
		}
	}
}

string GlobalArgument::toString()
{
	char temp[40];
	sprintf(temp, "gd%d", ID);
	return string(temp);
}

string LabelArgument::toString()
{
	if(!haslineno)
	{
		char temp[40];
		sprintf(temp, "l%d", ID);
		return string(temp);
	}
	else
	{
		char temp[40];
		sprintf(temp, "%d", lineno);
		return string(temp);
	}
}

string OSetImmediate::toString()
{
	return "SETV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSetRegister::toString()
{
	return "SETR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAddImmediate::toString()
{
	return "ADDV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAddRegister::toString()
{
	return "ADDR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSubImmediate::toString()
{
	return "SUBV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSubRegister::toString()
{
	return "SUBR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMultImmediate::toString()
{
	return "MULTV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OMultRegister::toString()
{
	return "MULTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODivImmediate::toString()
{
	return "DIVV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODivRegister::toString()
{
	return "DIVR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCompareImmediate::toString()
{
	return "COMPAREV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCompareRegister::toString()
{
	return "COMPARER " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWaitframe::toString(){
	return "WAITFRAME";
}

string OGotoImmediate::toString()
{
	return "GOTO " + getArgument()->toString();
}

string OGotoTrueImmediate::toString()
{
	return "GOTOTRUE " + getArgument()->toString();
}

string OGotoFalseImmediate::toString()
{
	return "GOTOFALSE " + getArgument()->toString();
}

string OGotoMoreImmediate::toString()
{
	return "GOTOMORE " + getArgument()->toString();
}

string OGotoLessImmediate::toString()
{
	return "GOTOLESS " + getArgument()->toString();
}

string OPushRegister::toString()
{
	return "PUSHR " + getArgument()->toString();
}

string OPopRegister::toString()
{
	return "POP " + getArgument()->toString();
}

string OLoadIndirect::toString()
{
	return "LOADI " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OStoreIndirect::toString()
{
	return "STOREI " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OQuit::toString()
{
	return "QUIT";
}

string OGotoRegister::toString()
{
	return "GOTOR " + getArgument()->toString();
}

string OAndImmediate::toString()
{
	return "ANDV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAndRegister::toString()
{
	return "ANDR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OOrImmediate::toString()
{
	return "ORV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OOrRegister::toString()
{
	return "ORR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OXorImmediate::toString()
{
	return "XORV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OXorRegister::toString()
{
	return "XORR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSinRegister::toString()
{
	return "SINR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCosRegister::toString()
{
	return "COSR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OTanRegister::toString()
{
	return "TANR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMinRegister::toString()
{
	return "MINR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMaxRegister::toString()
{
	return "MAXR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPowRegister::toString()
{
	return "POWERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OInvPowRegister::toString()
{
	return "IPOWERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OFactorial::toString()
{
	return "FACTORIAL " + getArgument()->toString();
}

string OAbsRegister::toString()
{
	return "ABS " + getArgument()->toString();
}

string OLShiftImmediate::toString()
{
	return "LSHIFTV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OLShiftRegister::toString()
{
	return "LSHIFTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ORShiftImmediate::toString()
{
	return "RSHIFTV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ORShiftRegister::toString()
{
	return "RSHIFTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OModuloImmediate::toString()
{
	return "MODV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OModuloRegister::toString()
{
	return "MODR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ONot::toString()
{
	return "NOT " + getArgument()->toString();
}

string OTraceRegister::toString()
{
	return "TRACER " + getArgument()->toString();
}

string ORandRegister::toString()
{
	return "RNDR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCheckTrig::toString()
{
	return "CHECKTRIG";
}

string OWarp::toString()
{
	return "WARPR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPitWarp::toString()
{
	return "PITWARPR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSqrtRegister::toString()
{
	return "SQROOTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCreateItemRegister::toString()
{
	return "CREATEITEMR " + getArgument()->toString();
}

string OCreateNPCRegister::toString()
{
	return "CREATENPCR " + getArgument()->toString();
}

string OPlaySoundRegister::toString()
{
	return "PLAYSOUNDR " + getArgument()->toString();
}
//////////////////////////////////////////////////////////////////////////////////////

int LinkTable::functionToLabel(int fid)
{
	map<int,int>::iterator it = funcLabels.find(fid);
	if(it != funcLabels.end())
		return (*it).second;
	int newid = ScriptParser::getUniqueLabelID();
	funcLabels[fid]=newid;
	return newid;
}

int LinkTable::getGlobalID(int vid)
{
	map<int, int>::iterator it = globalIDs.find(vid);
	if(it == globalIDs.end())
		return -1;
	return it->second;
}

int LinkTable::addGlobalVar(int vid)
{
	int newid = ScriptParser::getUniqueGlobalID();
	globalIDs[vid]=newid;
	return newid;
}

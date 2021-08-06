#include "ffdebug.h"

extern long sarg1;
extern long sarg2;

namespace ffdebug
{
	char varToStringTemp[40];
	const char * const varToString(int ID)
	{
		switch(ID)
		{
			case SP:
				return "SP";
				
			case DATA:
				return "DATA";
				
			case FFSCRIPT:
				return "FFSCRIPT";
				
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
				
			case LINKZ:
				return "LINKZ";
				
			case LINKJUMP:
				return "LINKJUMP";
				
			case LINKDIR:
				return "LINKDIR";
				
			case LINKSWORDJINX:
				return "LINKSWORDJINX";
				
			case LINKITEMJINX:
				return "LINKITEMJINX";
				
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
				
			case LINKHELD:
				return "LINKHELD";
				
			case LINKINVIS:
				return "LINKINVIS";
				
			case LINKINVINC:
				return "LINKINVINC";
				
			case LINKMISCD:
				return "LINKMISCD";
				
			case INPUTSTART:
				return "INPUTSTART";
				
			case INPUTMAP:
				return "INPUTMAP";
				
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
				
			case INPUTEX1:
				return "INPUTEX1";
				
			case INPUTEX2:
				return "INPUTEX2";
				
			case INPUTEX3:
				return "INPUTEX3";
				
			case INPUTEX4:
				return "INPUTEX4";
				
			case INPUTAXISUP:
				return "INPUTAXISUP";
				
			case INPUTAXISDOWN:
				return "INPUTAXISDOWN";
				
			case INPUTAXISLEFT:
				return "INPUTAXISLEFT";
				
			case INPUTAXISRIGHT:
				return "INPUTAXISRIGHT";
				
			case INPUTPRESSSTART:
				return "INPUTPRESSSTART";
				
			case INPUTPRESSUP:
				return "INPUTPRESSUP";
				
			case INPUTPRESSDOWN:
				return "INPUTPRESSDOWN";
				
			case INPUTPRESSLEFT:
				return "INPUTPRESSLEFT";
				
			case INPUTPRESSRIGHT:
				return "INPUTPRESSRIGHT";
				
			case INPUTPRESSA:
				return "INPUTPRESSA";
				
			case INPUTPRESSB:
				return "INPUTPRESSB";
				
			case INPUTPRESSL:
				return "INPUTPRESSL";
				
			case INPUTPRESSR:
				return "INPUTPRESSR";
				
			case INPUTPRESSEX1:
				return "INPUTPRESSEX1";
				
			case INPUTPRESSEX2:
				return "INPUTPRESSEX2";
				
			case INPUTPRESSEX3:
				return "INPUTPRESSEX3";
				
			case INPUTPRESSEX4:
				return "INPUTPRESSEX4";
				
			case INPUTPRESSAXISUP:
				return "PRESSAXISUP";
				
			case INPUTPRESSAXISDOWN:
				return "PRESSAXISDOWN";
				
			case INPUTPRESSAXISLEFT:
				return "PRESSAXISLEFT";
				
			case INPUTPRESSAXISRIGHT:
				return "PRESSAXISRIGHT";
				
			case INPUTMOUSEX:
				return "INPUTMOUSEX";
				
			case INPUTMOUSEY:
				return "INPUTMOUSEY";
				
			case INPUTMOUSEZ:
				return "INPUTMOUSEZ";
				
			case INPUTMOUSEB:
				return "INPUTMOUSEB";
				
			case SDD:
				return "SDD";
				
			case SCREENFLAGSD:
				return "SCREENFLAGSD";
				
			case SCREENEFLAGSD:
				return "SCREENEFLAGSD";
				
			case COMBODD:
				return "COMBODD";
				
			case COMBOCD:
				return "COMBOCD";
				
			case COMBOFD:
				return "COMBOFD";
				
			case COMBODDM:
				return "COMBODDM";
				
			case COMBOCDM:
				return "COMBOCDM";
				
			case COMBOFDM:
				return "COMBOFDM";
				
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
				
			case ITEMZ:
				return "ITEMZ";
				
			case ITEMJUMP:
				return "ITEMJUMP";
				
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
				
			case ITEMPICKUP:
				return "ITEMPICKUP";
				
			case ITEMMISCD:
				return "ITEMMISCD";
				
			case IDATAFAMILY:
				return "IDATAFAMILY";
				
			case IDATALEVEL:
				return "IDATALEVEL";
				
			case IDATAAMOUNT:
				return "IDATAAMOUNT";
				
			case IDATAMAX:
				return "IDATAMAX";
				
			case IDATASETMAX:
				return "IDATAKEEP";
				
			case IDATACOUNTER:
				return "IDATACOUNTER";
				
			case IDATAUSESOUND:
				return "IDATAUSESOUND";
				
			case IDATAPOWER:
				return "IDATAPOWER";
				
			case IDATAINITDD:
				return "IDATAINITDD";
				
			case REFITEMCLASS:
				return "REFITEMCLASS";
				
			case COMBOID:
				return "COMBOID";
				
			case COMBOTD:
				return "COMBOTD";
				
			case COMBOSD:
				return "COMBOSD";
			case COMBOED:
				return "COMBOED";
				
			case COMBOIDM:
				return "COMBOIDM";
				
			case COMBOTDM:
				return "COMBOTDM";
				
			case COMBOSDM:
				return "COMBOSDM";
				
			case GETMIDI:
				return "GETMIDI";
				
			case CURSCR:
				return "CURSCR";
				
			case CURDSCR:
				return "CURDSCR";
				
			case CURMAP:
				return "CURMAP";
				
			case CURDMAP:
				return "CURDMAP";
				
			case CURLEVEL:
				return "CURLEVEL";
				
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
				
			case SCREENSTATED:
				return "SCREENSTATED";
				
			case SCREENSTATEDD:
				return "SCREENSTATEDD";
				
			case DMAPFLAGSD:
				return "DMAPFLAGSD";
				
			case DMAPLEVELD:
				return "DMAPLEVELD";
				
			case DMAPCOMPASSD:
				return "DMAPCOMPASSD";
				
			case DMAPCONTINUED:
				return "DMAPCONTINUED";
				
			case DMAPMIDID:
				return "DMAPMIDID";
				
			case DMAPOFFSET:
				return "DMAPOFFSET";
				
			case DMAPMAP:
				return "DMAPMAP";
				
			case SDDD:
				return "SDDD";
				
			case SDDDD:
				return "SDDDD";
				
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
				
			case FFMISCD:
				return "FFMISCD";
				
			case FFINITDD:
				return "FFINITDD";
				
			/*case FFDD:
				return "FFDD";*/
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
				
			case NPCZ:
				return "NPCZ";
				
			case NPCJUMP:
				return "NPCJUMP";
				
			case NPCDIR:
				return "NPCDIR";
				
			case NPCRATE:
				return "NPCRATE";
				
			case NPCHOMING:
				return "NPCHOMING";
				
			case NPCFRAMERATE:
				return "NPCFRAMERATE";
				
			case NPCHALTRATE:
				return "NPCHALTRATE";
				
			case NPCDRAWTYPE:
				return "NPCDRAWTYPE";
				
			case NPCHP:
				return "NPCHP";
				
			case NPCID:
				return "NPCID";
				
			case NPCTYPE:
				return "NPCTYPE";
				
			case NPCDP:
				return "NPCDP";
				
			case NPCWDP:
				return "NPCWDP";
				
			case NPCTILE:
				return "NPCTILE";
				
			case NPCOTILE:
				return "NPCOTILE";
				
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
				
			case NPCSTEP:
				return "NPCSTEP";
				
			case NPCDEFENSED:
				return "NPCDEFENSED";
				
			case NPCMISCD:
				return "NPCMISCD";
				
			case NPCDD:
				return "NPCDD";
				
			case NPCMFLAGS:
				return "NPCMFLAGS";
				
			case NPCCOLLDET:
				return "NPCCOLLDET";
				
			case NPCSTUN:
				return "NPCSTUN";
				
			case NPCHUNGER:
				return "NPCHUNGER";
				
			case SCRDOORD:
				return "SCRDOORD";
				
			case LIT:
				return "LIT";
				
			case WAVY:
				return "WAVY";
				
			case QUAKE:
				return "QUAKE";
				
			case ITEMOTILE:
				return "ITEMOTILE";
				
			case REFLWPN:
				return "REFLWPN";
				
			case LWPNCOUNT:
				return "LWPNCOUNT";
				
			case LWPNX:
				return "LWPNX";
				
			case LWPNY:
				return "LWPNY";
				
			case LWPNZ:
				return "LWPNZ";
				
			case LWPNJUMP:
				return "LWPNJUMP";
				
			case LWPNDIR:
				return "LWPNDIR";
				
			case LWPNANGLE:
				return "LWPNANGLE";
				
			case LWPNSTEP:
				return "LWPNSTEP";
				
			case LWPNFRAMES:
				return "LWPNFRAMES";
				
			case LWPNFRAME:
				return "LWPNFRAME";
				
			case LWPNDRAWTYPE:
				return "LWPNDRAWTYPE";
				
			case LWPNPOWER:
				return "LWPNPOWER";
				
			case LWPNID:
				return "LWPNID";
				
			case LWPNANGULAR:
				return "LWPNANGULAR";
				
			case LWPNBEHIND:
				return "LWPNBEHIND";
				
			case LWPNASPEED:
				return "LWPNASPEED";
				
			case LWPNTILE:
				return "LWPNTILE";
				
			case LWPNFLASHCSET:
				return "LWPNFLASHCSET";
				
			case LWPNDEAD:
				return "LWPNDEAD";
				
			case LWPNCSET:
				return "LWPNCSET";
				
			case LWPNFLASH:
				return "LWPNFLASH";
				
			case LWPNFLIP:
				return "LWPNFLIP";
				
			case LWPNOTILE:
				return "LWPNOTILE";
				
			case LWPNOCSET:
				return "LWPNOCSET";
				
			case LWPNEXTEND:
				return "LWPNEXTEND";
				
			case LWPNCOLLDET:
				return "LWPNCOLLDET";
				
			case REFEWPN:
				return "REFEWPN";
				
			case EWPNCOUNT:
				return "EWPNCOUNT";
				
			case EWPNX:
				return "EWPNX";
				
			case EWPNY:
				return "EWPNY";
				
			case EWPNZ:
				return "EWPNZ";
				
			case EWPNJUMP:
				return "EWPNJUMP";
				
			case EWPNDIR:
				return "EWPNDIR";
				
			case EWPNANGLE:
				return "EWPNANGLE";
				
			case EWPNSTEP:
				return "EWPNSTEP";
				
			case EWPNFRAMES:
				return "EWPNFRAMES";
				
			case EWPNFRAME:
				return "EWPNFRAME";
				
			case EWPNDRAWTYPE:
				return "EWPNDRAWTYPE";
				
			case EWPNPOWER:
				return "EWPNPOWER";
				
			case EWPNID:
				return "EWPNID";
				
			case EWPNANGULAR:
				return "EWPNANGULAR";
				
			case EWPNBEHIND:
				return "EWPNBEHIND";
				
			case EWPNASPEED:
				return "EWPNASPEED";
				
			case EWPNTILE:
				return "EWPNTILE";
				
			case EWPNFLASHCSET:
				return "EWPNFLASHCSET";
				
			case EWPNDEAD:
				return "EWPNDEAD";
				
			case EWPNCSET:
				return "EWPNCSET";
				
			case EWPNFLASH:
				return "EWPNFLASH";
				
			case EWPNFLIP:
				return "EWPNFLIP";
				
			case EWPNOTILE:
				return "EWPNOTILE";
				
			case EWPNOCSET:
				return "EWPNOCSET";
				
			case EWPNEXTEND:
				return "EWPNEXTEND";
				
			case EWPNCOLLDET:
				return "EWPNCOLLDET";
				
			case SCRIPTRAM:
				return "SCRIPTRAM";
				
			case GLOBALRAM:
				return "GLOBALRAM";
				
			case SCRIPTRAMD:
				return "SCRIPTRAMD";
				
			case GLOBALRAMD:
				return "GLOBALRAMD";
				
			case LWPNHXOFS:
				return "LWPNHXOFS";
				
			case LWPNHYOFS:
				return "LWPNHYOFS";
				
			case LWPNXOFS:
				return "LWPNXOFS";
				
			case LWPNYOFS:
				return "LWPNYOFS";
				
			case LWPNZOFS:
				return "LWPNZOFS";
				
			case LWPNHXSZ:
				return "LWPNHXSZ";
				
			case LWPNHYSZ:
				return "LWPNHYSZ";
				
			case LWPNHZSZ:
				return "LWPNHZSZ";
				
			case LWPNTXSZ:
				return "LWPNTXSZ";
				
			case LWPNTYSZ:
				return "LWPNTYSZ";
				
			case LWPNMISCD:
				return "LWPNMISCD";
				
			case EWPNHXOFS:
				return "EWPNHXOFS";
				
			case EWPNHYOFS:
				return "EWPNHYOFS";
				
			case EWPNXOFS:
				return "EWPNXOFS";
				
			case EWPNYOFS:
				return "EWPNYOFS";
				
			case EWPNZOFS:
				return "EWPNZOFS";
				
			case EWPNHXSZ:
				return "EWPNHXSZ";
				
			case EWPNHYSZ:
				return "EWPNHYSZ";
				
			case EWPNHZSZ:
				return "EWPNHZSZ";
				
			case EWPNTXSZ:
				return "EWPNTXSZ";
				
			case EWPNTYSZ:
				return "EWPNTYSZ";
				
			case EWPNMISCD:
				return "EWPNMISCD";
				
			case NPCHXOFS:
				return "NPCHXOFS";
				
			case NPCHYOFS:
				return "NPCHYOFS";
				
			case NPCXOFS:
				return "NPCXOFS";
				
			case NPCYOFS:
				return "NPCYOFS";
				
			case NPCZOFS:
				return "NPCZOFS";
				
			case NPCHXSZ:
				return "NPCHXSZ";
				
			case NPCHYSZ:
				return "NPCHYSZ";
				
			case NPCHZSZ:
				return "NPCHZSZ";
				
			case NPCTXSZ:
				return "NPCTXSZ";
				
			case NPCTYSZ:
				return "NPCTYSZ";
				
			case ITEMHXOFS:
				return "ITEMHXOFS";
				
			case ITEMHYOFS:
				return "ITEMHYOFS";
				
			case ITEMXOFS:
				return "ITEMXOFS";
				
			case ITEMYOFS:
				return "ITEMYOFS";
				
			case ITEMZOFS:
				return "ITEMZOFS";
				
			case ITEMHXSZ:
				return "ITEMHXSZ";
				
			case ITEMHYSZ:
				return "ITEMHYSZ";
				
			case ITEMHZSZ:
				return "ITEMHZSZ";
				
			case ITEMTXSZ:
				return "ITEMTXSZ";
				
			case ITEMTYSZ:
				return "ITEMTYSZ";
				
			case LINKHXOFS:
				return "LINKHXOFS";
				
			case LINKHYOFS:
				return "LINKHYOFS";
				
			case LINKXOFS:
				return "LINKXOFS";
				
			case LINKYOFS:
				return "LINKYOFS";
				
			case LINKZOFS:
				return "LINKZOFS";
				
			case LINKHXSZ:
				return "LINKHXSZ";
				
			case LINKHYSZ:
				return "LINKHYSZ";
				
			case LINKHZSZ:
				return "LINKHZSZ";
				
			case LINKTXSZ:
				return "LINKTXSZ";
				
			case LINKTYSZ:
				return "LINKTYSZ";
				
			case LINKDRUNK:
				return "LINKDRUNK";
				
			case LINKEQUIP:
				return "LINKEQUIP";
				
			case LINKLADDERX:
				return "LINKLADDERX";
				
			case LINKLADDERY:
				return "LINKLADDERY";
				
			case ROOMTYPE:
				return "ROOMTYPE";
				
			case ROOMDATA:
				return "ROOMDATA";
				
			case LINKTILE:
				return "LINKTILE";
				
			case LINKFLIP:
				return "LINKFLIP";
				
			case INPUTPRESSMAP:
				return "INPUTPRESSMAP";
				
			case GAMESTANDALONE:
				return "GAMESTANDALONE";
				
			case GAMEENTRSCR:
				return "GAMEENTRSCREEN";
				
			case GAMEENTRDMAP:
				return "GAMEENTRDMAP";
				
			case GAMECLICKFREEZE:
				return "GAMECLICKFREEZE";
				
			case PUSHBLOCKX:
				return "PUSHBLOCKX";
				
			case PUSHBLOCKY:
				return "PUSHBLOCKY";
				
			case PUSHBLOCKCOMBO:
				return "PUSHBLOCKCOMBO";
				
			case PUSHBLOCKCSET:
				return "PUSHBLOCKCSET";
				
			case UNDERCOMBO:
				return "UNDERCOMBO";
				
			case UNDERCSET:
				return "UNDERCSET";
				
			default:
			{
				sprintf(varToStringTemp, "d%d", ID);
				return varToStringTemp;
			}
		}
	}

	script_command command_list[NUMCOMMANDS+1]=
	{
		//name                args arg1 arg2 more
		{ "SETV",                2,   0,   1,   0},
		{ "SETR",                2,   0,   0,   0},
		{ "ADDR",                2,   0,   0,   0},
		{ "ADDV",                2,   0,   1,   0},
		{ "SUBR",                2,   0,   0,   0},
		{ "SUBV",                2,   0,   1,   0},
		{ "MULTR",               2,   0,   0,   0},
		{ "MULTV",               2,   0,   1,   0},
		{ "DIVR",                2,   0,   0,   0},
		{ "DIVV",                2,   0,   1,   0},
		{ "WAITFRAME",           0,   0,   0,   0},
		{ "GOTO",                1,   1,   0,   0},
		{ "CHECKTRIG",           0,   0,   0,   0},
		{ "WARP",                2,   1,   1,   0},
		{ "COMPARER",            2,   0,   0,   0},
		{ "COMPAREV",            2,   0,   1,   0},
		{ "GOTOTRUE",            2,   0,   0,   0},
		{ "GOTOFALSE",           2,   0,   0,   0},
		{ "GOTOLESS",            2,   0,   0,   0},
		{ "GOTOMORE",            2,   0,   0,   0},
		{ "LOAD1",               2,   0,   0,   0},
		{ "LOAD2",               2,   0,   0,   0},
		{ "SETA1",               2,   0,   0,   0},
		{ "SETA2",               2,   0,   0,   0},
		{ "QUIT",                0,   0,   0,   0},
		{ "SINR",                2,   0,   0,   0},
		{ "SINV",                2,   0,   1,   0},
		{ "COSR",                2,   0,   0,   0},
		{ "COSV",                2,   0,   1,   0},
		{ "TANR",                2,   0,   0,   0},
		{ "TANV",                2,   0,   1,   0},
		{ "MODR",                2,   0,   0,   0},
		{ "MODV",                2,   0,   1,   0},
		{ "ABS",                 1,   0,   0,   0},
		{ "MINR",                2,   0,   0,   0},
		{ "MINV",                2,   0,   1,   0},
		{ "MAXR",                2,   0,   0,   0},
		{ "MAXV",                2,   0,   1,   0},
		{ "RNDR",                2,   0,   0,   0},
		{ "RNDV",                2,   0,   1,   0},
		{ "FACTORIAL",           1,   0,   0,   0},
		{ "POWERR",              2,   0,   0,   0},
		{ "POWERV",              2,   0,   1,   0},
		{ "IPOWERR",             2,   0,   0,   0},
		{ "IPOWERV",             2,   0,   1,   0},
		{ "ANDR",                2,   0,   0,   0},
		{ "ANDV",                2,   0,   1,   0},
		{ "ORR",                 2,   0,   0,   0},
		{ "ORV",                 2,   0,   1,   0},
		{ "XORR",                2,   0,   0,   0},
		{ "XORV",                2,   0,   1,   0},
		{ "NANDR",               2,   0,   0,   0},
		{ "NANDV",               2,   0,   1,   0},
		{ "NORR",                2,   0,   0,   0},
		{ "NORV",                2,   0,   1,   0},
		{ "XNORR",               2,   0,   0,   0},
		{ "XNORV",               2,   0,   1,   0},
		{ "NOT",                 1,   0,   0,   0},
		{ "LSHIFTR",             2,   0,   0,   0},
		{ "LSHIFTV",             2,   0,   1,   0},
		{ "RSHIFTR",             2,   0,   0,   0},
		{ "RSHIFTV",             2,   0,   1,   0},
		{ "TRACER",              1,   0,   0,   0},
		{ "TRACEV",              1,   1,   0,   0},
		{ "TRACE3",              0,   0,   0,   0},
		{ "LOOP",                2,   1,   0,   0},
		{ "PUSHR",               1,   0,   0,   0},
		{ "PUSHV",               1,   1,   0,   0},
		{ "POP",                 1,   0,   0,   0},
		{ "ENQUEUER",            2,   0,   0,   0},
		{ "ENQUEUEV",            2,   0,   1,   0},
		{ "DEQUEUE",             1,   0,   0,   0},
		{ "PLAYSOUNDR",          1,   0,   0,   0},
		{ "PLAYSOUNDV",          1,   1,   0,   0},
		{ "LOADLWEAPONR",        1,   0,   0,   0},
		{ "LOADLWEAPONV",        1,   1,   0,   0},
		{ "LOADITEMR",           1,   0,   0,   0},
		{ "LOADITEMV",           1,   1,   0,   0},
		{ "LOADNPCR",            1,   0,   0,   0},
		{ "LOADNPCV",            1,   1,   0,   0},
		{ "CREATELWEAPONR",      1,   0,   0,   0},
		{ "CREATELWEAPONV",      1,   1,   0,   0},
		{ "CREATEITEMR",         1,   0,   0,   0},
		{ "CREATEITEMV",         1,   1,   0,   0},
		{ "CREATENPCR",          1,   0,   0,   0},
		{ "CREATENPCV",          1,   1,   0,   0},
		{ "LOADI",               2,   0,   0,   0},
		{ "STOREI",              2,   0,   0,   0},
		{ "GOTOR",               1,   0,   0,   0},
		{ "SQROOTV",             2,   0,   1,   0},
		{ "SQROOTR",             2,   0,   0,   0},
		{ "CREATEEWEAPONR",      1,   0,   0,   0},
		{ "CREATEEWEAPONV",      1,   1,   0,   0},
		{ "PITWARP",             2,   1,   1,   0},
		{ "WARPR",               2,   0,   0,   0},
		{ "PITWARPR",            2,   0,   0,   0},
		{ "CLEARSPRITESR",       1,   0,   0,   0},
		{ "CLEARSPRITESV",       1,   1,   0,   0},
		{ "RECT",                0,   0,   0,   0},
		{ "CIRCLE",              0,   0,   0,   0},
		{ "ARC",                 0,   0,   0,   0},
		{ "ELLIPSE",             0,   0,   0,   0},
		{ "LINE",                0,   0,   0,   0},
		{ "PUTPIXEL",            0,   0,   0,   0},
		{ "DRAWTILE",            0,   0,   0,   0},
		{ "DRAWCOMBO",           0,   0,   0,   0},
		{ "ELLIPSE2",            0,   0,   0,   0},
		{ "SPLINE",              0,   0,   0,   0},
		{ "FLOODFILL",           0,   0,   0,   0},
		{ "COMPOUNDR",           1,   0,   0,   0},
		{ "COMPOUNDV",           1,   1,   0,   0},
		{ "MSGSTRR",             1,   0,   0,   0},
		{ "MSGSTRV",             1,   1,   0,   0},
		{ "ISVALIDITEM",         1,   0,   0,   0},
		{ "ISVALIDNPC",          1,   0,   0,   0},
		{ "PLAYMIDIR",           1,   0,   0,   0},
		{ "PLAYMIDIV",           1,   1,   0,   0},
		{ "COPYTILEVV",          2,   1,   1,   0},
		{ "COPYTILEVR",          2,   1,   0,   0},
		{ "COPYTILERV",          2,   0,   1,   0},
		{ "COPYTILERR",          2,   0,   0,   0},
		{ "SWAPTILEVV",          2,   1,   1,   0},
		{ "SWAPTILEVR",          2,   1,   0,   0},
		{ "SWAPTILERV",          2,   0,   1,   0},
		{ "SWAPTILERR",          2,   0,   0,   0},
		{ "CLEARTILEV",          1,   1,   0,   0},
		{ "CLEARTILER",          1,   0,   0,   0},
		{ "OVERLAYTILEVV",       2,   1,   1,   0},
		{ "OVERLAYTILEVR",       2,   1,   0,   0},
		{ "OVERLAYTILERV",       2,   0,   1,   0},
		{ "OVERLAYTILERR",       2,   0,   0,   0},
		{ "FLIPROTTILEVV",       2,   1,   1,   0},
		{ "FLIPROTTILEVR",       2,   1,   0,   0},
		{ "FLIPROTTILERV",       2,   0,   1,   0},
		{ "FLIPROTTILERR",       2,   0,   0,   0},
		{ "GETTILEPIXELV",       1,   1,   0,   0},
		{ "GETTILEPIXELR",       1,   0,   0,   0},
		{ "SETTILEPIXELV",       1,   1,   0,   0},
		{ "SETTILEPIXELR",       1,   0,   0,   0},
		{ "SHIFTTILEVV",         2,   1,   1,   0},
		{ "SHIFTTILEVR",         2,   1,   0,   0},
		{ "SHIFTTILERV",         2,   0,   1,   0},
		{ "SHIFTTILERR",         2,   0,   0,   0},
		{ "ISVALIDLWPN",         1,   0,   0,   0},
		{ "ISVALIDEWPN",         1,   0,   0,   0},
		{ "LOADEWEAPONR",        1,   0,   0,   0},
		{ "LOADEWEAPONV",        1,   1,   0,   0},
		{ "ALLOCATEMEMR",        2,   0,   0,   0},
		{ "ALLOCATEMEMV",        2,   0,   1,   0},
		{ "ALLOCATEGMEM",        2,   0,   1,   0},
		{ "DEALLOCATEMEMR",      1,   0,   0,   0},
		{ "DEALLOCATEMEMV",      1,   1,   0,   0},
		{ "WAITDRAW",			   0,   0,   0,   0},
		{ "ARCTANR",		       1,   0,   0,   0},
		{ "LWPNUSESPRITER",      1,   0,   0,   0},
		{ "LWPNUSESPRITEV",      1,   1,   0,   0},
		{ "EWPNUSESPRITER",      1,   0,   0,   0},
		{ "EWPNUSESPRITEV",      1,   1,   0,   0},
		{ "LOADITEMDATAR",       1,   0,   0,   0},
		{ "LOADITEMDATAV",       1,   1,   0,   0},
		{ "BITNOT",              1,   0,   0,   0},
		{ "LOG10",               1,   0,   0,   0},
		{ "LOGE",                1,   0,   0,   0},
		{ "ISSOLID",             1,   0,   0,   0},
		{ "LAYERSCREEN",         2,   0,   0,   0},
		{ "LAYERMAP",            2,   0,   0,   0},
		{ "TRACE2R",             1,   0,   0,   0},
		{ "TRACE2V",             1,   1,   0,   0},
		{ "TRACE4",              0,   0,   0,   0},
		{ "TRACE5",              0,   0,   0,   0},
		{ "SECRETS",			   0,   0,   0,   0},
		{ "DRAWCHAR",            0,   0,   0,   0},
		{ "GETSCREENFLAGS",      1,   0,   0,   0},
		{ "QUAD",                0,   0,   0,   0},
		{ "TRIANGLE",            0,   0,   0,   0},
		{ "ARCSINR",             2,   0,   0,   0},
		{ "ARCSINV",             2,   1,   0,   0},
		{ "ARCCOSR",             2,   0,   0,   0},
		{ "ARCCOSV",             2,   1,   0,   0},
		{ "GAMEEND",             0,   0,   0,   0},
		{ "DRAWINT",             0,   0,   0,   0},
		{ "SETTRUE",             1,   0,   0,   0},
		{ "SETFALSE",            1,   0,   0,   0},
		{ "SETMORE",             1,   0,   0,   0},
		{ "SETLESS",             1,   0,   0,   0},
		{ "FASTTILE",            0,   0,   0,   0},
		{ "FASTCOMBO",           0,   0,   0,   0},
		{ "DRAWSTRING",          0,   0,   0,   0},
		{ "SETSIDEWARP",         0,   0,   0,   0},
		{ "SAVE",                0,   0,   0,   0},
		{ "TRACE6",              0,   0,   0,   0},
		{ "DEPRECATED",	       1,   0,   0,   0},
		{ "QUAD3D",              0,   0,   0,   0},
		{ "TRIANGLE3D",          0,   0,   0,   0},
		{ "SETCOLORB",           0,   0,   0,   0},
		{ "SETDEPTHB",           0,   0,   0,   0},
		{ "GETCOLORB",           0,   0,   0,   0},
		{ "GETDEPTHB",           0,   0,   0,   0},
		{ "COMBOTILE",           2,   0,   0,   0},
		{ "SETTILEWARP",         0,   0,   0,   0},
		{ "GETSCREENEFLAGS",     1,   0,   0,   0},
		{ "GETSAVENAMER",        1,   0,   0,   0},
		{ "ARRAYSIZE",           1,   0,   0,   0},
		{ "ITEMNAMER",           1,   0,   0,   0},
		{ "SETSAVENAMER",        1,   0,   0,   0},
		{ "PLAYENHMUSIC",        2,   0,   0,   0},
		{ "GETMUSICFILE",        2,   0,   0,   0},
		{ "GETMUSICTRACK",       1,   0,   0,   0},
		{ "SETDMAPENHMUSIC",     0,   0,   0,   0},
		{ "BREAKSHIELD",         1,   0,   0,   0},
		{ "SAVESCREEN",          1,   0,   0,   0},
		{ "SAVEQUITSCREEN",      0,   0,   0,   0},
		{ "SELECTAWPNR",         1,   0,   0,   0},
		{ "SELECTAWPNV",         1,   1,   0,   0},
		{ "SELECTBWPNR",         1,   0,   0,   0},
		{ "SELECTBWPNV",         1,   1,   0,   0},
		{ "GETSIDEWARPDMAP",     1,   0,   0,   0},
		{ "GETSIDEWARPSCR",      1,   0,   0,   0},
		{ "GETSIDEWARPTYPE",     1,   0,   0,   0},
		{ "GETTILEWARPDMAP",     1,   0,   0,   0},
		{ "GETTILEWARPSCR",      1,   0,   0,   0},
		{ "GETTILEWARPTYPE",     1,   0,   0,   0},
		{ "",                    0,   0,   0,   0}
	};


	void print_disassembly(const word scommand)
	{
		script_command s_c = command_list[scommand];
		
		if(s_c.args == 2)
		{
			al_trace("%14s: ", s_c.name);
			
			if(s_c.arg1_type == 0)
				al_trace("%10s (val = %9d), ", varToString(sarg1), get_register(sarg1));
			else
				al_trace("%10s (val = %9d), ", "immediate", sarg1);
				
			if(s_c.arg2_type == 0)
				al_trace("%10s (val = %9d)\n", varToString(sarg2), get_register(sarg2));
			else
				al_trace("%10s (val = %9d)\n", "immediate", sarg2);
		}
		else if(s_c.args == 1)
		{
			al_trace("%14s: ", s_c.name);
			
			if(s_c.arg1_type == 0)
				al_trace("%10s (val = %9d)\n", varToString(sarg1), get_register(sarg1));
			else
				al_trace("%10s (val = %9d)\n", "immediate", sarg1);
		}
		else
			al_trace("%14s\n", s_c.name);
			
	}
}


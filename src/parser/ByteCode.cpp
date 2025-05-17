//2.53 Updated to 16th Jan, 2017
#include "ByteCode.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "zsyssimple.h"
#include <assert.h>
#include <iostream>
#include <cstdlib>

using namespace ZScript;
using std::ostringstream;
using std::string;

string la_toString(int32_t value)
{
    char temp[128];
    string sign = value < 0 ? "-" : "";
    sprintf(temp,"%d", abs(value/10000));
    string first = string(temp);
    
    if(value % 10000 == 0)
        return sign + first;
        
    sprintf(temp,"%d", abs(value%10000));
    string second = string(temp);
    
    while(second.length() < 4)
        second = "0" + second;
        
    return sign + first + "." + second;
}

string LiteralArgument::toString() const
{
	return la_toString(value);
}

string ZScript::VarToString(int32_t ID)
{
	char temp[128];
	
	switch(ID)
	{
		case SP:
			return "SP";
		case PC:
			return "PC";
		case SWITCHKEY:
			return "SWITCHKEY";
			
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
		case FFCID:
		return "FFCID";
			
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
			
		case LINKHITDIR:
			return "LINKHITDIR";
			
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
			
		case LINKTILE:
			return "LINKTILE";
			
		case LINKFLIP:
			return "LINKFLIP";
			
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
			
		case INPUTPRESSMAP:
			return "INPUTPRESSMAP";
			
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
		
		case BUTTONPRESS: return "BUTTONPRESS";
		case BUTTONINPUT: return "BUTTONINPUT";
		case BUTTONHELD: return "BUTTONHELD";
		case RAWKEY: return "RAWKEY";
		case READKEY: return "READKEY";
		case DISABLEKEY: return "DISABLEKEY";
		case DISABLEBUTTON: return "DISABLEBUTTON";
		case JOYPADPRESS: return "JOYPADPRESS";
		
		 case LINKINVFRAME:
		return "LINKINVFRAME";
		case LINKCANFLICKER:
		return "LINKCANFLICKER";
		case LINKHURTSFX:
		return "LINKHURTSFX";
		
		case LINKEATEN: return "LINKEATEN";
		case LINKGRABBED: return "LINKGRABBED";
		case HEROBUNNY: return "HEROBUNNY";
		
		case LINKITEMB: return "LINKITEMB";

		case LINKITEMA: return "LINKITEMA";
		case LINKITEMX: return "LINKITEMX";
		case LINKITEMY: return "LINKITEMY";
		case SETITEMSLOT: return "SETITEMSLOT";
		case GAMESETB: return "GAMESETB";
		case GAMESETA: return "GAMESETA";
		case LINKUSINGITEM: return "LINKUSINGITEM";
		case LINKUSINGITEMA: return "LINKUSINGITEMA";
		case LINKUSINGITEMB: return "LINKUSINGITEMB";
		
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
		
		case VIEWPORT_X:
			return "VIEWPORT_X";
		case VIEWPORT_Y:
			return "VIEWPORT_Y";
		case VIEWPORT_WIDTH:
			return "VIEWPORT_WIDTH";
		case VIEWPORT_HEIGHT:
			return "VIEWPORT_HEIGHT";
		
		case REGION_WIDTH:
			return "REGION_WIDTH";
		case REGION_HEIGHT:
			return "REGION_HEIGHT";
		case REGION_SCREEN_WIDTH:
			return "REGION_SCREEN_WIDTH";
		case REGION_SCREEN_HEIGHT:
			return "REGION_SCREEN_HEIGHT";
		case REGION_UNUSED:
			return "REGION_UNUSED";
		case REGION_NUM_COMBOS:
			return "REGION_NUM_COMBOS";
		
		case HERO_SCREEN:
			return "HERO_SCREEN";
		case SCREEN_INDEX:
			return "SCREEN_INDEX";
		case SCREEN_DRAW_ORIGIN:
			return "SCREEN_DRAW_ORIGIN";
		case SCREEN_FLAG:
			return "SCREEN_FLAG";
		case MAPDATA_FLAG:
			return "MAPDATA_FLAG";
		
		case REFMAPDATA: return "REFMAPDATA";
		case REFSCREENDATA: return "REFSCREENDATA";
		case REFCOMBODATA: return "REFCOMBODATA";
		case REFSPRITEDATA: return "REFSPRITEDATA";
		case REFBITMAP: return "REFBITMAP";
		case REFNPCCLASS: return "REFNPCCLASS";
		
		 case REFDMAPDATA: return "REFDMAPDATA";
		 case REFSHOPDATA: return "REFSHOPDATA";
		 case REFMSGDATA: return "REFMSGDATA";
		 case REFNIL: return "REFNIL";
		
		 case REFDROPS: return "REFDROPS";
		 case REFPONDS: return "REFPONDS";
		 case REFWARPRINGS: return "REFWARPRINGS";
		 case REFDOORS: return "REFDOORS";
		 case REFUICOLOURS: return "REFUICOLOURS";
		 case REFRGB: return "REFRGB";
		 case REFPALETTE: return "REFPALETTE";
		 case REFTUNES: return "REFTUNES";
		 case REFPALCYCLE: return "REFPALCYCLE";
		 case REFGAMEDATA: return "REFGAMEDATA";
		 case REFCHEATS: return "REFCHEATS";
			
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
			
		//This DOES NOT DO what ZScript.txt claims that it does, and needs to be fixed. -Z
		case ITEMFRAME:
			return "ITEMFRAME";
			
		case ITEMASPEED:
			return "ITEMASPEED";
		
		case ITEMACLK:
			return "ITEMACLK";
			
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
			
		case ITEMNOSOUND:
			return "ITEMNOSOUND";
			
		case ITEMNOHOLDSOUND:
			return "ITEMNOHOLDSOUND";
			
		case ITEMMISCD:
			return "ITEMMISCD";
			
		case ITEMCLASSFAMILY:
			return "IDATAFAMILY";
			
		case ITEMCLASSFAMTYPE:
			return "IDATALEVEL";
			
		case ITEMCLASSAMOUNT:
			return "IDATAAMOUNT";
			
		case ITEMCLASSMAX:
			return "IDATAMAX";
			
		case ITEMCLASSSETMAX:
			return "IDATASETMAX";
			
		case ITEMCLASSSETGAME:
			return "IDATAKEEP";
			
		case ITEMCLASSCOUNTER:
			return "IDATACOUNTER";
			
		case ITEMCLASSUSESOUND:
			return "IDATAUSESOUND";
		case IDATAUSESOUND2:
			return "IDATAUSESOUND2";
			
		case ITEMCLASSPOWER:
			return "IDATAPOWER";
			
		case ITEMCLASSINITDD:
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
		case GAMEMAXMAPS: return "GAMEMAXMAPS";
			
		case CURLEVEL:
			return "CURLEVEL";
			
		case GAMEDEATHS:
			return "GAMEDEATHS";
			
		case GAMECHEAT:
			return "GAMECHEAT";
		case GAMEMAXCHEAT:
			return "GAMEMAXCHEAT";
		case SHOWNMSG:
			return "SHOWNMSG";
		
		case ZELDAVERSION:
		return "ZELDAVERSION";
		case ZELDABUILD:
		return "ZELDABUILD";
		case ZELDABETA:
		return "ZELDABETA";
		
			
		case GAMETIME:
			return "GAMETIME";
			
		case GAMEHASPLAYED:
			return "GAMEHASPLAYED";
			
		case GAMESTANDALONE:
			return "GAMESTANDALONE";
			
		case GAMETIMEVALID:
			return "GAMETIMEVALID";
			
		case GAMEGUYCOUNT:
			return "GAMEGUYCOUNT";
			
		case GAMECONTSCR:
			return "GAMECONTSCR";
			
		case GAMECONTDMAP:
			return "GAMECONTDMAP";
			
		case GAMEENTRSCR:
			return "GAMEENTRSCR";
			
		case GAMEENTRDMAP:
			return "GAMEENTRDMAP";
			
		case GAMECOUNTERD:
			return "GAMECOUNTERD";
			
		case GAMEMCOUNTERD:
			return "GAMEMCOUNTERD";
			
		case GAMEDCOUNTERD:
			return "GAMEDCOUNTERD";
			
		case GAMEGENERICD:
			return "GAMEGENERICD";
		
		case GAMEMISC:
			return "GAMEMISC";
			
		case GAMEITEMSD:
			return "GAMEITEMSD";
			
		case GAMELITEMSD:
			return "GAMELITEMSD";
			
		case GAMELKEYSD:
			return "GAMELKEYSD";
		case GAMELSWITCH:
			return "GAMELSWITCH";
		case GAMEGSWITCH:
			return "GAMEGSWITCH";
		case GAMEBOTTLEST:
			return "GAMEBOTTLEST";
			
		case REFBOTTLETYPE:
			return "REFBOTTLETYPE";
		case REFBOTTLESHOP:
			return "REFBOTTLESHOP";
		case BOTTLECOUNTER:
			return "BOTTLECOUNTER";
		case BOTTLEAMOUNT:
			return "BOTTLEAMOUNT";
		case BOTTLEPERCENT:
			return "BOTTLEPERCENT";
		case BOTTLEFLAGS:
			return "BOTTLEFLAGS";
		case BOTTLENEXT:
			return "BOTTLENEXT";
		case BSHOPFILL:
			return "BSHOPFILL";
		case BSHOPCOMBO:
			return "BSHOPCOMBO";
		case BSHOPCSET:
			return "BSHOPCSET";
		case BSHOPPRICE:
			return "BSHOPPRICE";
		case BSHOPSTR:
			return "BSHOPSTR";
		case COMBODUSRFLAGARR:
			return "COMBODUSRFLAGARR";
		case COMBODGENFLAGARR:
			return "COMBODGENFLAGARR";
			
		case HERORESPAWNX:
			return "HERORESPAWNX";
		case HERORESPAWNY:
			return "HERORESPAWNY";
		case HERORESPAWNDMAP:
			return "HERORESPAWNDMAP";
		case HERORESPAWNSCR:
			return "HERORESPAWNSCR";
		case HEROSWITCHTIMER:
			return "HEROSWITCHTIMER";
		case HEROSWITCHMAXTIMER:
			return "HEROSWITCHMAXTIMER";
		case HEROTOTALDYOFFS:
			return "HEROTOTALDYOFFS";
		case NPCTOTALDYOFFS:
			return "NPCTOTALDYOFFS";
		case LWPNTOTALDYOFFS:
			return "LWPNTOTALDYOFFS";
		case EWPNTOTALDYOFFS:
			return "EWPNTOTALDYOFFS";
		case NPCSWHOOKED:
			return "NPCSWHOOKED";
		case LWSWHOOKED:
			return "LWSWHOOKED";
		case EWSWHOOKED:
			return "EWSWHOOKED";
		case ITMSWHOOKED:
			return "ITMSWHOOKED";
		case DEBUGTESTING:
			return "DEBUGTESTING";
		case GAMEMISCSPR:
			return "GAMEMISCSPR";
		case GAMEMISCSFX:
			return "GAMEMISCSFX";
		case GAMEEVENTDATA:
			return "GAMEEVENTDATA";
			
		case SCREENSTATED:
			return "SCREENSTATED";
			
		case SCREENSTATEDD:
			return "SCREENSTATEDD";
			
		case DMAPFLAGSD:
			return "DMAPFLAGSD";
			
		case DMAPLEVELD:
			return "DMAPLEVELD";
		
		case DMAPLEVELPAL:
		return "DMAPLEVELPAL";
			
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
			
		case GAMECLICKFREEZE:
			return "GAMECLICKFREEZE";
			
		case NOACTIVESUBSC: return "NOACTIVESUBSC"; //Disable active subscreen.
		
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
		
		case DISABLEDITEM: return "DISABLEDITEM";
			
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
		case NPCSCRDEFENSED: return "NPCSCRDEFENSED";
			
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
		
		
		case NPCINVINC:
			return "NPCINVINC";
		case NPCSUPERMAN:
			return "NPCSUPERMAN";
		case NPCHASITEM:
			return "NPCHASITEM";
		case NPCRINGLEAD:
			return "NPCRINGLEAD";
		case NPCSHIELD:
			return "NPCSHIELD";
		
		case NPCFROZEN:
			return "NPCFROZEN";   
		case NPCFROZENTILE:
			return "NPCFROZENTILE";
		case NPCFROZENCSET:
			return "NPCFROZENCSET"; 
		case NPCBEHAVIOUR:
			return "NPCBEHAVIOUR"; 
			
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
		
		case LWPNDEGANGLE:
			return "LWPNDEGANGLE";
			
		case LWPNVX:
			return "LWPNVX";
			
		case LWPNVY:
			return "LWPNVY";
			
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
			
		case LWPNAUTOROTATE:
			return "LWPNAUTOROTATE";
			
		case LWPNFLAGS:
			return "LWPNFLAGS";
		case EWPNFLAGS:
			return "EWPNFLAGS";
			
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
		
		case LWPNPARENT:
			return "LWPNPARENT";
		case LWPNLEVEL:
			return "LWPNLEVEL";
		
		case EWPNLEVEL:
			return "EWPNLEVEL";
		case EWPNPARENT:
			return "EWPNPARENT";
			
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
			
		case EWPNDEGANGLE:
			return "EWPNDEGANGLE";
			
		case EWPNVX:
			return "EWPNVX";
			
		case EWPNVY:
			return "EWPNVY";
			
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
			
		case EWPNAUTOROTATE:
			return "EWPNAUTOROTATE";
			
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
		
		case LWPNSHADOWXOFS:
			return "LWPNSHADOWXOFS";
			
		case LWPNSHADOWYOFS:
			return "LWPNSHADOWYOFS";
			
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
			
		case EWPNSHADOWXOFS:
			return "EWPNSHADOWXOFS";
			
		case EWPNSHADOWYOFS:
			return "EWPNSHADOWYOFS";
			
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
			
		case NPCSHADOWXOFS:
			return "NPCSHADOWXOFS";
			
		case NPCSHADOWYOFS:
			return "NPCSHADOWYOFS";
			
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
			
		case ITEMSHADOWXOFS:
			return "ITEMSHADOWXOFS";
			
		case ITEMSHADOWYOFS:
			return "ITEMSHADOWYOFS";
			
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
			
		case HEROSHADOWXOFS:
			return "HEROSHADOWXOFS";
			
		case HEROSHADOWYOFS:
			return "HEROSHADOWYOFS";
			
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
			
		case PUSHBLOCKX:
			return "PUSHBLOCKX";
			
		case PUSHBLOCKY:
			return "PUSHBLOCKY";
			
		case PUSHBLOCKLAYER:
			return "PUSHBLOCKLAYER";
			
		case PUSHBLOCKCOMBO:
			return "PUSHBLOCKCOMBO";
			
		case PUSHBLOCKCSET:
			return "PUSHBLOCKCSET";
			
		case UNDERCOMBO:
			return "UNDERCOMBO";
			
		case UNDERCSET:
			return "UNDERCSET";
		
		//2.6
		case CREATELWPNDX: return "CREATELWPNDX";
		
		//2.54 -Z
	/* 2.54 Implemented
		*/
		
		
	//itemclass / itemdata

		//case IDATAFRAME: return "IDATAFRAME";
		case IDATACOSTCOUNTER: return "IDATACOSTCOUNTER";
		case IDATACOSTCOUNTER2: return "IDATACOSTCOUNTER2";
		case ITEMCLASSID: return "IDATAID";
		case IDATALTM: return "IDATALTM";
		case IDATAPSCRIPT: return "IDATAPSCRIPT";
		case IDATASCRIPT: return "IDATASCRIPT";
		case IDATAMAGCOST: return "IDATAMAGCOST";
		case IDATACOST2: return "IDATACOST2";
		case IDATAMINHEARTS: return "IDATAMINHEARTS";
		case IDATATILE: return "IDATATILE";
		case IDATAMISC: return "IDATAMISC";    
		case IDATACSET: return "IDATACSET";
		case IDATAFLASHCSET: return "IDATAFLASHCSET";
		case IDATAFRAMES: return "IDATAFRAMES";
		case IDATAASPEED: return "IDATAASPEED";
		case IDATADELAY: return "IDATADELAY"; 
		case IDATACOMBINE: return "IDATACOMBINE";
		case IDATADOWNGRADE: return "IDATADOWNGRADE";
		case IDATAKEEPOLD: return "IDATAKEEPOLD";
		case IDATARUPEECOST: return "IDATARUPEECOST";
		case IDATAEDIBLE: return "IDATAEDIBLE";
		case IDATAFLAGUNUSED: return "IDATAFLAGUNUSED";
		case IDATAGAINLOWER: return "IDATAGAINLOWER";
		case IDATAPSTRING: return "IDATAPSTRING";
		case IDATAPFLAGS: return "IDATAPFLAGS";
		
		//idata arrays
		case IDATAATTRIB: return "IDATAATTRIB";
		case IDATAFLAGS: return "IDATAFLAGS";
		case IDATASPRITE: return "IDATASPRITE";
			
		case IDATAUSEWPN: return "IDATAUSEWPN";
		case IDATAUSEDEF: return "IDATAUSEDEF";
		case IDATAWRANGE: return "IDATAWRANGE";
		case IDATAMAGICTIMER: return "IDATAMAGICTIMER";
		case IDATAMAGICTIMER2: return "IDATAMAGICTIMER2";
		case IDATAUSEMVT: return "IDATAUSEMVT";
		case IDATADURATION: return "IDATADURATION";
		case IDATADUPLICATES: return "IDATADUPLICATES";
		case IDATADRAWLAYER: return "IDATADRAWLAYER";
		case IDATACOLLECTFLAGS: return "IDATACOLLECTFLAGS";
		case IDATAWEAPONSCRIPT: return "IDATAWEAPONSCRIPT";
		case IDATAMISCD: return "IDATAMISCD";
		case IDATAWEAPHXOFS: return "IDATAWEAPHXOFS";
		case IDATAWEAPHYOFS: return "IDATAWEAPHYOFS";
		case IDATAWEAPHYSZ: return "IDATAWEAPHYSZ";
		case IDATAWEAPHXSZ: return "IDATAWEAPHXSZ";
		case IDATAWEAPHZSZ: return "IDATAWEAPHZSZ";
		case IDATAWEAPXOFS: return "IDATAWEAPXOFS";
		case IDATAWEAPYOFS: return "IDATAWEAPYOFS";
		case IDATAWEAPZOFS: return "IDATAWEAPZOFS";
		case IDATAWPNINITD: return "IDATAWPNINITD";
		
		//Item Sizing
		case IDATAOVERRIDEFLWEAP: return "IDATAOVERRIDEFLWEAP";
		case IDATATILEHWEAP: return "IDATATILEHWEAP";
		case IDATATILEWWEAP: return "IDATATILEWWEAP";
		case IDATAHZSZWEAP: return "IDATAHZSZWEAP";
		case IDATAHYSZWEAP: return "IDATAHYSZWEAP";
		case IDATAHXSZWEAP: return "IDATAHXSZWEAP";
		case IDATADYOFSWEAP: return "IDATADYOFSWEAP";
		case IDATADXOFSWEAP: return "IDATADXOFSWEAP";
		case IDATAHYOFSWEAP: return "IDATAHYOFSWEAP";
		case IDATAHXOFSWEAP: return "IDATAHXOFSWEAP";
		case IDATAOVERRIDEFL: return "IDATAOVERRIDEFL";
		case IDATAPICKUP: return "IDATAPICKUP";
		case IDATATILEH: return "IDATATILEH";
		case IDATATILEW: return "IDATATILEW";
		case IDATAHZSZ: return "IDATAHZSZ";
		case IDATAHYSZ: return "IDATAHYSZ";
		case IDATAHXSZ: return "IDATAHXSZ";
		case IDATADYOFS: return "IDATADYOFS";
		case IDATADXOFS: return "IDATADXOFS";
		case IDATAHYOFS: return "IDATAHYOFS";
		case IDATAHXOFS: return "IDATAHXOFS";
		
		case NPCWEAPSPRITE: return "NPCWEAPSPRITE";
		
		//Debug->
		
		case DEBUGREFFFC: return "DEBUGREFFFC";
		case DEBUGREFITEM: return "DEBUGREFITEM";
		case DEBUGREFNPC: return "DEBUGREFNPC";
		case DEBUGREFITEMDATA: return "DEBUGREFITEMDATA";
		case DEBUGREFLWEAPON: return "DEBUGREFLWEAPON";
		case DEBUGREFEWEAPON: return "DEBUGREFEWEAPON";
		case DEBUGSP: return "DEBUGSP";
		case DEBUGGDR: return "DEBUGGDR";
		
		case LWPNRANGE: return "LWPNRANGE";
		
		case SETSCREENDOOR: return "SETSCREENDOOR";
		case SETSCREENENEMY: return "SETSCREENENEMY";
		case SCREENWIDTH: return "SCREENWIDTH";
		case SCREENHEIGHT: return "SCREENHEIGHT";
		case SCREENVIEWX: return "SCREENVIEWX";
		case SCREENVIEWY: return "SCREENVIEWY";
		case SCREENGUY: return "SCREENGUY";
		case SCREENSTRING: return "SCREENSTRING";
		case SCREENROOM: return "SCREENROOM";
		case SCREENENTX: return "SCREENENTX";
		case SCREENENTY: return "SCREENENTY";
		case SCREENITEM: return "SCREENITEM";
		case SCREENUNDCMB: return "SCREENUNDCMB";
		case SCREENUNDCST: return "SCREENUNDCST";
		case SCREENCATCH: return "SCREENCATCH";
		case SETSCREENLAYOP: return "SETSCREENLAYOP";
		case SETSCREENSECCMB: return "SETSCREENSECCMB";
		case SETSCREENSECCST: return "SETSCREENSECCST";
		case SETSCREENSECFLG: return "SETSCREENSECFLG";
		case SETSCREENLAYMAP: return "SETSCREENLAYMAP";
		case SETSCREENLAYSCR: return "SETSCREENLAYSCR";
		case SETSCREENPATH: return "SETSCREENPATH";
		case SETSCREENWARPRX: return "SETSCREENWARPRX";
		case SETSCREENWARPRY: return "SETSCREENWARPRY";
		
		case GAMENUMMESSAGES: return "GAMENUMMESSAGES";
		case GAMESUBSCHEIGHT: return "GAMESUBSCHEIGHT";
		case GAMEPLAYFIELDOFS: return "GAMEPLAYFIELDOFS";
		case PASSSUBOFS: return "PASSSUBOFS";
		case COMBODATAID: return "COMBODATAID";
		
		case LINKBIGHITBOX: return "LINKBIGHITBOX";
		case LINKDIAG: return "LINKDIAG";
		
		//NPCData
		
		//three inputs no return (function-only)
		case SETNPCDATASCRIPTDEF: return "SETNPCDATASCRIPTDEF";
		case SETNPCDATADEFENSE: return "SETNPCDATADEFENSE";
		case SETNPCDATASIZEFLAG: return "SETNPCDATASIZEFLAG";
		case SETNPCDATAATTRIBUTE: return "SETNPCDATAATTRIBUTE";
		
		//ComboData
		
		//three inputs no return (function-only)
		case SCDBLOCKWEAPON: return "SCDBLOCKWEAPON";
		case SCDEXPANSION: return "SCDEXPANSION";
		case SCDSTRIKEWEAPONS: return "SCDSTRIKEWEAPONS";
		
		//Game Over Screen
		case SETGAMEOVERELEMENT: return "SETGAMEOVERELEMENT";
		case SETGAMEOVERSTRING: return "SETGAMEOVERSTRING";
		
		//Input->Mouse[]
		case MOUSEARR: return "MOUSEARR";
		
		
		//DataTypes
		
		//spritedata sd->
		case SPRITEDATATILE: return "SPRITEDATATILE";
		case SPRITEDATAMISC: return "SPRITEDATAMISC";
		case SPRITEDATAFLAGS: return "SPRITEDATAFLAGS";
		case SPRITEDATACSETS: return "SPRITEDATACSETS";
		case SPRITEDATAFLCSET: return "SPRITEDATAFLCSET";
		case SPRITEDATAFRAMES: return "SPRITEDATAFRAMES";
		case SPRITEDATASPEED: return "SPRITEDATASPEED";
		case SPRITEDATATYPE: return "SPRITEDATATYPE";
		case SPRITEDATAID: return "SPRITEDATAID";
		
		//npcdata nd->
		case NPCDATATILE: return "NPCDATATILE";
		case NPCDATAHEIGHT: return "NPCDATAHEIGHT";
		case NPCDATAFLAGS: return "NPCDATAFLAGS";
		case NPCDATAFLAGS2: return "NPCDATAFLAGS2";
		case NPCDATAHITSFX: return "NPCDATAHITSFX";
		case NPCDATAWIDTH: return "NPCDATAWIDTH";
		case NPCDATASTILE: return "NPCDATASTILE";
		case NPCDATASWIDTH: return "NPCDATASWIDTH";
		case NPCDATASHEIGHT: return "NPCDATASHEIGHT";
		case NPCDATAETILE: return "NPCDATAETILE";
		case NPCDATAEWIDTH: return "NPCDATAEWIDTH";
		case NPCDATAEHEIGHT: return "NPCDATAEHEIGHT";
		case NPCDATAHP: return "NPCDATAHP";
		case NPCDATAFAMILY: return "NPCDATAFAMILY";
		case NPCDATACSET: return "NPCDATACSET";
		case NPCDATAANIM: return "NPCDATAANIM";
		case NPCDATAEANIM: return "NPCDATAEANIM";
		case NPCDATAFRAMERATE: return "NPCDATAFRAMERATE";
		case NPCDATAEFRAMERATE: return "NPCDATAEFRAMERATE";
		case NPCDATATOUCHDAMAGE: return "NPCDATATOUCHDAMAGE";
		case NPCDATAWEAPONDAMAGE: return "NPCDATAWEAPONDAMAGE";
		case NPCDATAWEAPON: return "NPCDATAWEAPON";
		case NPCDATARANDOM: return "NPCDATARANDOM";
		case NPCDATAHALT: return "NPCDATAHALT";
		case NPCDATASTEP: return "NPCDATASTEP";
		case NPCDATAHOMING: return "NPCDATAHOMING";
		case NPCDATAHUNGER: return "NPCDATAHUNGER";
		case NPCDATADROPSET: return "NPCDATADROPSET";
		case NPCDATABGSFX: return "NPCDATABGSFX";
		case NPCDATADEATHSFX: return "NPCDATADEATHSFX";
		case NPCDATAXOFS: return "NPCDATAXOFS";
		case NPCDATAYOFS: return "NPCDATAYOFS";
		case NPCDATAZOFS: return "NPCDATAZOFS";
		case NPCDATAHXOFS: return "NPCDATAHXOFS";
		case NPCDATAHYOFS: return "NPCDATAHYOFS";
		case NPCDATAHITWIDTH: return "NPCDATAHITWIDTH";
		case NPCDATAHITHEIGHT: return "NPCDATAHITHEIGHT";
		case NPCDATAHITZ: return "NPCDATAHITZ";
		case NPCDATATILEWIDTH: return "NPCDATATILEWIDTH";
		case NPCDATATILEHEIGHT: return "NPCDATATILEHEIGHT";
		case NPCDATAWPNSPRITE: return "NPCDATAWPNSPRITE";
		case NPCDATADEFENSE: return "NPCDATADEFENSE";
		case NPCDATASIZEFLAG: return "NPCDATASIZEFLAG";
		case NPCDATAATTRIBUTE: return "NPCDATAATTRIBUTE";
		case NPCDATASHIELD: return "NPCDATASHIELD";
		case NPCDATAFROZENTILE: return "NPCDATAFROZENTILE";
		case NPCDATAFROZENCSET: return "NPCDATAFROZENCSET";
		case NPCDATABEHAVIOUR: return "NPCDATABEHAVIOUR";
			
		case MAPDATAVALID: return "MAPDATAVALID";
		case MAPDATAGUY: return "MAPDATAGUY";
		case MAPDATASTRING: return "MAPDATASTRING";
		case MAPDATAROOM: return "MAPDATAROOM";
		case MAPDATAITEM: return "MAPDATAITEM";
		case MAPDATAHASITEM: return "MAPDATAHASITEM";
		case MAPDATATILEWARPTYPE: return "MAPDATATILEWARPTYPE";
		case MAPDATATILEWARPOVFLAGS: return "MAPDATATILEWARPOVFLAGS";
		case MAPDATADOORCOMBOSET: return "MAPDATADOORCOMBOSET";
		case MAPDATAWARPRETX: return "MAPDATAWARPRETX";
		case MAPDATAWARPRETY: return "MAPDATAWARPRETY";
		case MAPDATAWARPRETURNC: return "MAPDATAWARPRETURNC";
		case MAPDATASTAIRX: return "MAPDATASTAIRX";
		case MAPDATASTAIRY: return "MAPDATASTAIRY";
		case MAPDATACOLOUR: return "MAPDATACOLOUR";
		case MAPDATAENEMYFLAGS: return "MAPDATAENEMYFLAGS";
		case MAPDATADOOR: return "MAPDATADOOR";
		case MAPDATATILEWARPDMAP: return "MAPDATATILEWARPDMAP";
		case MAPDATATILEWARPSCREEN: return "MAPDATATILEWARPSCREEN";
		case MAPDATAEXITDIR: return "MAPDATAEXITDIR";
		case MAPDATAENEMY: return "MAPDATAENEMY";
		case MAPDATAPATTERN: return "MAPDATAPATTERN";
		case MAPDATASIDEWARPTYPE: return "MAPDATASIDEWARPTYPE";
		case MAPDATASIDEWARPOVFLAGS: return "MAPDATASIDEWARPOVFLAGS";
		case MAPDATAWARPARRIVALX: return "MAPDATAWARPARRIVALX";
		case MAPDATAWARPARRIVALY: return "MAPDATAWARPARRIVALY";
		case MAPDATAPATH: return "MAPDATAPATH";
		case MAPDATASIDEWARPSC: return "MAPDATASIDEWARPSC";
		case MAPDATASIDEWARPDMAP: return "MAPDATASIDEWARPDMAP";
		case MAPDATASIDEWARPINDEX: return "MAPDATASIDEWARPINDEX";
		case MAPDATAUNDERCOMBO: return "MAPDATAUNDERCOMBO";
		case MAPDATAUNDERCSET: return "MAPDATAUNDERCSET";
		case MAPDATACATCHALL: return "MAPDATACATCHALL";
		case MAPDATAFLAGS: return "MAPDATAFLAGS";
		case MAPDATACSENSITIVE: return "MAPDATACSENSITIVE";
		case MAPDATANORESET: return "MAPDATANORESET";
		case MAPDATANOCARRY: return "MAPDATANOCARRY";
		case MAPDATALAYERMAP: return "MAPDATALAYERMAP";
		case MAPDATALAYERSCREEN: return "MAPDATALAYERSCREEN";
		case MAPDATALAYEROPACITY: return "MAPDATALAYEROPACITY";
		case MAPDATATIMEDWARPTICS: return "MAPDATATIMEDWARPTICS";
		case MAPDATANEXTMAP: return "MAPDATANEXTMAP";
		case MAPDATANEXTSCREEN: return "MAPDATANEXTSCREEN";
		case MAPDATASECRETCOMBO: return "MAPDATASECRETCOMBO";
		case MAPDATASECRETCSET: return "MAPDATASECRETCSET";
		case MAPDATASECRETFLAG: return "MAPDATASECRETFLAG";
		case MAPDATAVIEWX: return "MAPDATAVIEWX";
		case MAPDATAVIEWY: return "MAPDATAVIEWY";
		case MAPDATASCREENWIDTH: return "MAPDATASCREENWIDTH";
		case MAPDATASCREENHEIGHT: return "MAPDATASCREENHEIGHT";
		case MAPDATAENTRYX: return "MAPDATAENTRYX";
		case MAPDATAENTRYY: return "MAPDATAENTRYY";
		case MAPDATANUMFF: return "MAPDATANUMFF";
		case MAPDATAFFDATA: return "MAPDATAFFDATA";
		case MAPDATAFFCSET: return "MAPDATAFFCSET";
		case MAPDATAFFDELAY: return "MAPDATAFFDELAY";
		case MAPDATAFFX: return "MAPDATAFFX";
		case MAPDATAFFY: return "MAPDATAFFY";
		case MAPDATAFFXDELTA: return "MAPDATAFFXDELTA";
		case MAPDATAFFYDELTA: return "MAPDATAFFYDELTA";
		case MAPDATAFFXDELTA2: return "MAPDATAFFXDELTA2";
		case MAPDATAFFYDELTA2: return "MAPDATAFFYDELTA2";
		case MAPDATAFFFLAGS: return "MAPDATAFFFLAGS";
		case MAPDATAFFWIDTH: return "MAPDATAFFWIDTH";
		case MAPDATAFFHEIGHT: return "MAPDATAFFHEIGHT";
		case MAPDATAFFLINK: return "MAPDATAFFLINK";
		case MAPDATAFFSCRIPT: return "MAPDATAFFSCRIPT";
		case MAPDATAINTID: return "MAPDATAINTID"; //needs to be a function; [32][10]
		case MAPDATAINITA: return "MAPDATAINITA"; //needs to be a function [32][2]
		case MAPDATAFFINITIALISED: return "MAPDATAFFINITIALISED";
		case MAPDATASCRIPTENTRY: return "MAPDATASCRIPTENTRY";
		case MAPDATASCRIPTOCCUPANCY: return "MAPDATASCRIPTOCCUPANCY";
		case MAPDATASCRIPTEXIT: return "MAPDATASCRIPTEXIT";
		case MAPDATAOCEANSFX: return "MAPDATAOCEANSFX";
		case MAPDATABOSSSFX: return "MAPDATABOSSSFX";
		case MAPDATASECRETSFX: return "MAPDATASECRETSFX";
		case MAPDATAHOLDUPSFX: return "MAPDATAHOLDUPSFX";
		case MAPDATASCREENMIDI: return "MAPDATASCREENMIDI";
		case MAPDATALENSLAYER: return "MAPDATALENSLAYER";
		
		case MAPDATACOMBODD: return "MAPDATACOMBODD";
		case MAPDATACOMBOCD: return "MAPDATACOMBOCD";
		case MAPDATACOMBOFD: return "MAPDATACOMBOFD";
		case MAPDATACOMBOTD: return "MAPDATACOMBOTD";
		case MAPDATACOMBOID: return "MAPDATACOMBOID";
		case MAPDATACOMBOSD: return "MAPDATACOMBOSD";
		case MAPDATACOMBOED: return "MAPDATACOMBOED";
		
		
		
		case MAPDATAMISCD: return "MAPDATAMISCD";
		case MAPDATASCREENSTATED: return "MAPDATASCREENSTATED";
		case MAPDATASCREENFLAGSD: return "MAPDATASCREENFLAGSD";
		case MAPDATASCREENEFLAGSD: return "MAPDATASCREENEFLAGSD";
		
			
		case SCREENDATAVALID: return "SCREENDATAVALID";
		case SCREENDATAGUY: return "SCREENDATAGUY";
		case SCREENDATASTRING: return "SCREENDATASTRING";
		case SCREENDATAROOM: return "SCREENDATAROOM";
		case SCREENDATAITEM: return "SCREENDATAITEM";
		case SCREENDATAHASITEM: return "SCREENDATAHASITEM";
		case SCREENDATATILEWARPTYPE: return "SCREENDATATILEWARPTYPE";
		case SCREENDATATILEWARPOVFLAGS: return "SCREENDATATILEWARPOVFLAGS";
		case SCREENDATADOORCOMBOSET: return "SCREENDATADOORCOMBOSET";
		case SCREENDATAWARPRETX: return "SCREENDATAWARPRETX";
		case SCREENDATAWARPRETY: return "SCREENDATAWARPRETY";
		case SCREENDATAWARPRETURNC: return "SCREENDATAWARPRETURNC";
		case SCREENDATASTAIRX: return "SCREENDATASTAIRX";
		case SCREENDATASTAIRY: return "SCREENDATASTAIRY";
		case SCREENDATACOLOUR: return "SCREENDATACOLOUR";
		case SCREENDATAENEMYFLAGS: return "SCREENDATAENEMYFLAGS";
		case SCREENDATADOOR: return "SCREENDATADOOR";
		case SCREENDATATILEWARPDMAP: return "SCREENDATATILEWARPDMAP";
		case SCREENDATATILEWARPSCREEN: return "SCREENDATATILEWARPSCREEN";
		case SCREENDATAEXITDIR: return "SCREENDATAEXITDIR";
		case SCREENDATAENEMY: return "SCREENDATAENEMY";
		case SCREENDATAPATTERN: return "SCREENDATAPATTERN";
		case SCREENDATASIDEWARPTYPE: return "SCREENDATASIDEWARPTYPE";
		case SCREENDATASIDEWARPOVFLAGS: return "SCREENDATASIDEWARPOVFLAGS";
		case SCREENDATAWARPARRIVALX: return "SCREENDATAWARPARRIVALX";
		case SCREENDATAWARPARRIVALY: return "SCREENDATAWARPARRIVALY";
		case SCREENDATAPATH: return "SCREENDATAPATH";
		case SCREENDATASIDEWARPSC: return "SCREENDATASIDEWARPSC";
		case SCREENDATASIDEWARPDMAP: return "SCREENDATASIDEWARPDMAP";
		case SCREENDATASIDEWARPINDEX: return "SCREENDATASIDEWARPINDEX";
		case SCREENDATAUNDERCOMBO: return "SCREENDATAUNDERCOMBO";
		case SCREENDATAUNDERCSET: return "SCREENDATAUNDERCSET";
		case SCREENDATACATCHALL: return "SCREENDATACATCHALL";
		case SCREENDATAFLAGS: return "SCREENDATAFLAGS";
		case SCREENDATACSENSITIVE: return "SCREENDATACSENSITIVE";
		case SCREENDATANORESET: return "SCREENDATANORESET";
		case SCREENDATANOCARRY: return "SCREENDATANOCARRY";
		case SCREENDATALAYERMAP: return "SCREENDATALAYERMAP";
		case SCREENDATALAYERSCREEN: return "SCREENDATALAYERSCREEN";
		case SCREENDATALAYEROPACITY: return "SCREENDATALAYEROPACITY";
		case SCREENDATALAYERINVIS: return "SCREENDATALAYERINVIS";
		case SCREENDATASCRIPTDRAWS: return "SCREENDATASCRIPTDRAWS";
		case SCREENDATATIMEDWARPTICS: return "SCREENDATATIMEDWARPTICS";
		case SCREENDATANEXTMAP: return "SCREENDATANEXTMAP";
		case SCREENDATANEXTSCREEN: return "SCREENDATANEXTSCREEN";
		case SCREENDATASECRETCOMBO: return "SCREENDATASECRETCOMBO";
		case SCREENDATASECRETCSET: return "SCREENDATASECRETCSET";
		case SCREENDATASECRETFLAG: return "SCREENDATASECRETFLAG";
		case SCREENDATAVIEWX: return "SCREENDATAVIEWX";
		case SCREENDATAVIEWY: return "SCREENDATAVIEWY";
		case SCREENDATASCREENWIDTH: return "SCREENDATASCREENWIDTH";
		case SCREENDATASCREENHEIGHT: return "SCREENDATASCREENHEIGHT";
		case SCREENDATAENTRYX: return "SCREENDATAENTRYX";
		case SCREENDATAENTRYY: return "SCREENDATAENTRYY";
		case SCREENDATANUMFF: return "SCREENDATANUMFF";
		case SCREENDATAFFDATA: return "SCREENDATAFFDATA";
		case SCREENDATAFFCSET: return "SCREENDATAFFCSET";
		case SCREENDATAFFDELAY: return "SCREENDATAFFDELAY";
		case SCREENDATAFFX: return "SCREENDATAFFX";
		case SCREENDATAFFY: return "SCREENDATAFFY";
		case SCREENDATAFFXDELTA: return "SCREENDATAFFXDELTA";
		case SCREENDATAFFYDELTA: return "SCREENDATAFFYDELTA";
		case SCREENDATAFFXDELTA2: return "SCREENDATAFFXDELTA2";
		case SCREENDATAFFYDELTA2: return "SCREENDATAFFYDELTA2";
		case SCREENDATAFFFLAGS: return "SCREENDATAFFFLAGS";
		case SCREENDATAFFWIDTH: return "SCREENDATAFFWIDTH";
		case SCREENDATAFFHEIGHT: return "SCREENDATAFFHEIGHT";
		case SCREENDATAFFLINK: return "SCREENDATAFFLINK";
		case SCREENDATAFFSCRIPT: return "SCREENDATAFFSCRIPT";
		case SCREENDATAINTID: return "SCREENDATAINTID"; //needs to be a function; [32][10]
		case SCREENDATAINITA: return "SCREENDATAINITA"; //needs to be a function [32][2]
		case SCREENDATAFFINITIALISED: return "SCREENDATAFFINITIALISED";
		case SCREENDATASCRIPTENTRY: return "SCREENDATASCRIPTENTRY";
		case SCREENDATASCRIPTOCCUPANCY: return "SCREENDATASCRIPTOCCUPANCY";
		case SCREENDATASCRIPTEXIT: return "SCREENDATASCRIPTEXIT";
		case SCREENDATAOCEANSFX: return "SCREENDATAOCEANSFX";
		case SCREENDATABOSSSFX: return "SCREENDATABOSSSFX";
		case SCREENDATASECRETSFX: return "SCREENDATASECRETSFX";
		case SCREENDATAHOLDUPSFX: return "SCREENDATAHOLDUPSFX";
		case SCREENDATASCREENMIDI: return "SCREENDATASCREENMIDI";
		case SCREENDATALENSLAYER: return "SCREENDATALENSLAYER";
		
		case LINKSCRIPTTILE: return "LINKSCRIPTTILE";
		case LINKSCRIPFLIP: return "LINKSCRIPFLIP";
		case MAPDATAITEMX: return "MAPDATAITEMX";
		case MAPDATAITEMY: return "MAPDATAITEMY";
		case SCREENDATAITEMX: return "SCREENDATAITEMX";
		case SCREENDATAITEMY: return "SCREENDATAITEMY";
		
		case MAPDATAFFEFFECTWIDTH: return "MAPDATAFFEFFECTWIDTH";
		case MAPDATAFFEFFECTHEIGHT: return "MAPDATAFFEFFECTHEIGHT";
		case SCREENDATAFFEFFECTWIDTH: return "SCREENDATAFFEFFECTWIDTH";
		case SCREENDATAFFEFFECTHEIGHT: return "SCREENDATAFFEFFECTHEIGHT";
		case LOADMAPDATA: return "LOADMAPDATA";
		case CREATEBITMAP: return "CREATEBITMAP";
		case SHOPDATANAME: return "SHOPDATANAME";
		case SHOPDATAITEM: return "SHOPDATAITEM";
		case SHOPDATAHASITEM: return "SHOPDATAHASITEM";
		case SHOPDATAPRICE: return "SHOPDATAPRICE";
		case SHOPDATASTRING: return "SHOPDATASTRING";
		
		case AUDIOVOLUME: return "AUDIOVOLUME";
		case AUDIOPAN: return "AUDIOPAN";
		
		case MESSAGEDATANEXT: return "MESSAGEDATANEXT";
		case MESSAGEDATATILE: return "MESSAGEDATATILE";
		case MESSAGEDATACSET: return "MESSAGEDATACSET";
		case MESSAGEDATATRANS: return "MESSAGEDATATRANS";
		case MESSAGEDATAFONT: return "MESSAGEDATAFONT";
		case MESSAGEDATAX: return "MESSAGEDATAX";
		case MESSAGEDATAY: return "MESSAGEDATAY";
		case MESSAGEDATAW: return "MESSAGEDATAW";
		case MESSAGEDATAH: return "MESSAGEDATAH";
		case MESSAGEDATASFX: return "MESSAGEDATASFX";
		case MESSAGEDATALISTPOS: return "MESSAGEDATALISTPOS";
		case MESSAGEDATAVSPACE: return "MESSAGEDATAVSPACE";
		case MESSAGEDATAHSPACE: return "MESSAGEDATAHSPACE";
		case MESSAGEDATAFLAGS: return "MESSAGEDATAFLAGS";
		case MESSAGEDATAMARGINS: return "MESSAGEDATAMARGINS";
		case MESSAGEDATAPORTTILE: return "MESSAGEDATAPORTTILE";
		case MESSAGEDATAPORTCSET: return "MESSAGEDATAPORTCSET";
		case MESSAGEDATAPORTX: return "MESSAGEDATAPORTX";
		case MESSAGEDATAPORTY: return "MESSAGEDATAPORTY";
		case MESSAGEDATAPORTWID: return "MESSAGEDATAPORTWID";
		case MESSAGEDATAPORTHEI: return "MESSAGEDATAPORTHEI";
		case MESSAGEDATAFLAGSARR: return "MESSAGEDATAFLAGSARR";
		case MESSAGEDATATEXTWID: return "MESSAGEDATATEXTWID";
		case MESSAGEDATATEXTHEI: return "MESSAGEDATATEXTHEI";
		case MESSAGEDATATEXTLEN: return "MESSAGEDATATEXTLEN";
		
		case DMAPDATAID: return "DMAPDATAID";
		case DMAPDATAMAP: return "DMAPDATAMAP";
		case DMAPDATALEVEL: return "DMAPDATALEVEL";
		case DMAPDATAOFFSET: return "DMAPDATAOFFSET";
		case DMAPDATACOMPASS: return "DMAPDATACOMPASS";
		case DMAPDATAPALETTE: return "DMAPDATAPALETTE";
		case DMAPDATAMIDI: return "DMAPDATAMIDI";
		case DMAPDATACONTINUE: return "DMAPDATACONTINUE";
		case DMAPDATATYPE: return "DMAPDATATYPE";
		case DMAPDATAGRID: return "DMAPDATAGRID";
		case DMAPDATAMINIMAPTILE: return "DMAPDATAMINIMAPTILE";
		case DMAPDATAMINIMAPCSET: return "DMAPDATAMINIMAPCSET";
		case DMAPDATALARGEMAPTILE: return "DMAPDATALARGEMAPTILE";
		case DMAPDATALARGEMAPCSET: return "DMAPDATALARGEMAPCSET";
		case DMAPDATAMUISCTRACK: return "DMAPDATAMUISCTRACK";
		case DMAPDATASUBSCRA: return "DMAPDATASUBSCRA";
		case DMAPDATASUBSCRP: return "DMAPDATASUBSCRP";
		case DMAPDATADISABLEDITEMS: return "DMAPDATADISABLEDITEMS";
		case DMAPDATAFLAGS: return "DMAPDATAFLAGS";
		case DMAPDATAMIRRDMAP: return "DMAPDATAMIRRDMAP";
		
		case ITEMPSTRING: return "ITEMPSTRING";
		case ITEMPSTRINGFLAGS: return "ITEMPSTRINGFLAGS";
		case ITEMOVERRIDEFLAGS: return "ITEMOVERRIDEFLAGS";
		case IDATAGRADUAL: return "IDATAGRADUAL";
		case IDATASPRSCRIPT: return "IDATASPRSCRIPT";
		case IDATAPSOUND: return "IDATAPSOUND";
		case IDATACONSTSCRIPT: return "IDATACONSTSCRIPT";
		case IDATASSWIMDISABLED: return "IDATASSWIMDISABLED";
		case IDATABUNNYABLE: return "IDATABUNNYABLE";
		case IDATAJINXIMMUNE: return "IDATAJINXIMMUNE";
		case IDATAJINXSWAP: return "IDATAJINXSWAP";
		
		
		case LINKPUSH: return "LINKPUSH";
		case LINKSTUN: return "LINKSTUN";
		case TYPINGMODE: return "TYPINGMODE";
	//	case DMAPDATAGRAVITY: return "DMAPDATAGRAVITY";
	//	case DMAPDATAJUMPLAYER: return "DMAPDATAJUMPLAYER";


		//newcombo struct
		case COMBODTILE: return "COMBODTILE";
		case COMBODFLIP: return "COMBODFLIP";
		case COMBODWALK: return "COMBODWALK";
		case COMBODEFFECT: return "COMBODEFFECT";
		case COMBODTYPE: return "COMBODTYPE";
		case COMBODCSET: return "COMBODCSET";
		case COMBODFOO: return "COMBODFOO";
		case COMBODFRAMES: return "COMBODFRAMES";
		case COMBODNEXTD: return "COMBODNEXTD";
		case COMBODNEXTC: return "COMBODNEXTC";
		case COMBODFLAG: return "COMBODFLAG";
		case COMBODSKIPANIM: return "COMBODSKIPANIM";
		case COMBODNEXTTIMER: return "COMBODNEXTTIMER";
		case COMBODAKIMANIMY: return "COMBODAKIMANIMY";
		case COMBODANIMFLAGS: return "COMBODANIMFLAGS";
		case COMBODEXPANSION: return "COMBODEXPANSION";
		case COMBODATTRIBUTES: return "COMBODATTRIBUTES";
		case COMBODATTRIBYTES: return "COMBODATTRIBYTES";
		case COMBODATTRISHORTS: return "COMBODATTRISHORTS";
		case COMBODUSRFLAGS: return "COMBODUSRFLAGS";
		case COMBODTRIGGERFLAGS: return "COMBODTRIGGERFLAGS";
		case COMBODTRIGGERFLAGS2: return "COMBODTRIGGERFLAGS2";
		case COMBODTRIGGERBUTTON: return "COMBODTRIGGERBUTTON";
		case COMBODTRIGGERLEVEL: return "COMBODTRIGGERLEVEL";

		//comboclass struct
		case COMBODNAME: return "COMBODNAME";
		case COMBODBLOCKNPC: return "COMBODBLOCKNPC";
		case COMBODBLOCKHOLE: return "COMBODBLOCKHOLE";
		case COMBODBLOCKTRIG: return "COMBODBLOCKTRIG";
		case COMBODBLOCKWEAPON: return "COMBODBLOCKWEAPON";
		case COMBODCONVXSPEED: return "COMBODCONVXSPEED";
		case COMBODCONVYSPEED: return "COMBODCONVYSPEED";
		case COMBODSPAWNNPC: return "COMBODSPAWNNPC";
		case COMBODSPAWNNPCWHEN: return "COMBODSPAWNNPCWHEN";
		case COMBODSPAWNNPCCHANGE: return "COMBODSPAWNNPCCHANGE";
		case COMBODDIRCHANGETYPE: return "COMBODDIRCHANGETYPE";
		case COMBODDISTANCECHANGETILES: return "COMBODDISTANCECHANGETILES";
		case COMBODDIVEITEM: return "COMBODDIVEITEM";
		case COMBODDOCK: return "COMBODDOCK";
		case COMBODFAIRY: return "COMBODFAIRY";
		case COMBODFFATTRCHANGE: return "COMBODFFATTRCHANGE";
		case COMBODFOORDECOTILE: return "COMBODFOORDECOTILE";
		case COMBODFOORDECOTYPE: return "COMBODFOORDECOTYPE";
		case COMBODHOOKSHOTPOINT: return "COMBODHOOKSHOTPOINT";
		case COMBODLADDERPASS: return "COMBODLADDERPASS";
		case COMBODLOCKBLOCK: return "COMBODLOCKBLOCK";
		case COMBODLOCKBLOCKCHANGE: return "COMBODLOCKBLOCKCHANGE";
		case COMBODMAGICMIRROR: return "COMBODMAGICMIRROR";
		case COMBODMODHPAMOUNT: return "COMBODMODHPAMOUNT";
		case COMBODMODHPDELAY: return "COMBODMODHPDELAY";
		case COMBODMODHPTYPE: return "COMBODMODHPTYPE";
		case COMBODNMODMPAMOUNT: return "COMBODNMODMPAMOUNT";
		case COMBODMODMPDELAY: return "COMBODMODMPDELAY";
		case COMBODMODMPTYPE: return "COMBODMODMPTYPE";
		case COMBODNOPUSHBLOCK: return "COMBODNOPUSHBLOCK";
		case COMBODOVERHEAD: return "COMBODOVERHEAD";
		case COMBODPLACENPC: return "COMBODPLACENPC";
		case COMBODPUSHDIR: return "COMBODPUSHDIR";
		case COMBODPUSHWAIT: return "COMBODPUSHWAIT";
		case COMBODPUSHHEAVY: return "COMBODPUSHHEAVY";
		case COMBODPUSHED: return "COMBODPUSHED";
		case COMBODRAFT: return "COMBODRAFT";
		case COMBODRESETROOM: return "COMBODRESETROOM";
		case COMBODSAVEPOINTTYPE: return "COMBODSAVEPOINTTYPE";
		case COMBODSCREENFREEZETYPE: return "COMBODSCREENFREEZETYPE";
		case COMBODSECRETCOMBO: return "COMBODSECRETCOMBO";
		case COMBODSINGULAR: return "COMBODSINGULAR";
		case COMBODSLOWWALK: return "COMBODSLOWWALK";
		case COMBODSTATUETYPE: return "COMBODSTATUETYPE";
		case COMBODSTEPTYPE: return "COMBODSTEPTYPE";
		case COMBODSTEPCHANGEINTO: return "COMBODSTEPCHANGEINTO";
		case COMBODSTRIKEWEAPONS: return "COMBODSTRIKEWEAPONS";
		case COMBODSTRIKEREMNANTS: return "COMBODSTRIKEREMNANTS";
		case COMBODSTRIKEREMNANTSTYPE: return "COMBODSTRIKEREMNANTSTYPE";
		case COMBODSTRIKECHANGE: return "COMBODSTRIKECHANGE";
		case COMBODSTRIKEITEM: return "COMBODSTRIKEITEM";
		case COMBODTOUCHITEM: return "COMBODTOUCHITEM";
		case COMBODTOUCHSTAIRS: return "COMBODTOUCHSTAIRS";
		case COMBODTRIGGERTYPE: return "COMBODTRIGGERTYPE";
		case COMBODTRIGGERSENS: return "COMBODTRIGGERSENS";
		case COMBODWARPTYPE: return "COMBODWARPTYPE";
		case COMBODWARPSENS: return "COMBODWARPSENS";
		case COMBODWARPDIRECT: return "COMBODWARPDIRECT";
		case COMBODWARPLOCATION: return "COMBODWARPLOCATION";
		case COMBODWATER: return "COMBODWATER";
		case COMBODWHISTLE: return "COMBODWHISTLE";
		case COMBODWINGAME: return "COMBODWINGAME";
		case COMBODBLOCKWPNLEVEL: return "COMBODBLOCKWPNLEVEL";
		case LINKHITBY: return "LINKHITBY";
		case LINKDEFENCE: return "LINKDEFENCE";
		case NPCHITBY: return "NPCHITBY";
		case NPCISCORE: return "NPCISCORE";
		case NPCSCRIPTUID: return "NPCSCRIPTUID";
		case LWEAPONSCRIPTUID: return "LWEAPONSCRIPTUID";
		case EWEAPONSCRIPTUID: return "EWEAPONSCRIPTUID";
		case ITEMSCRIPTUID: return "ITEMSCRIPTUID";
		case DMAPDATASIDEVIEW: return "DMAPDATASIDEVIEW";
		case DMAPDATAASUBSCRIPT: return "DMAPDATAASUBSCRIPT";
		case DMAPDATAPSUBSCRIPT: return "DMAPDATAPSUBSCRIPT";
		case DMAPDATASUBINITD: return "DMAPDATASUBINITD";
		case DMAPDATAMAPSCRIPT: return "DMAPDATAMAPSCRIPT";
		case DMAPDATAMAPINITD: return "DMAPDATAMAPINITD";
		case DMAPDATACHARTED: return "DMAPDATACHARTED";
		
		
		case DONULL: return "DONULL";
		case DEBUGD: return "DEBUGD";
		case GETPIXEL: return "GETPIXEL";
		case DOUNTYPE: return "DOUNTYPE";
		case LINKTILEMOD: return "LINKTILEMOD";
		case NPCINITD: return "NPCINITD";
		
		case NPCCOLLISION: return "NPCCOLLISION";
		case NPCLINEDUP: return "NPCLINEDUP";
		case NPCDATAINITD: return "NPCDATAINITD";
		case NPCDATASCRIPT: return "NPCDATASCRIPT";
		case NPCMATCHINITDLABEL: return "NPCMATCHINITDLABEL";
		//lweapon scripts
		case LWPNSCRIPT: return "LWPNSCRIPT";
		case LWPNINITD: return "LWPNINITD";
		case ITEMFAMILY: return "ITEMFAMILY";
		case ITEMLEVEL: return "ITEMLEVEL";
		
		case EWPNSCRIPT: return "EWPNSCRIPT";
		case EWPNINITD: return "EWPNINITD";
		case NPCSCRIPT: return "NPCSCRIPT";
		case DMAPSCRIPT: return "DMAPSCRIPT";
		case DMAPINITD: return "DMAPINITD";
		case SCREENSCRIPT: return "SCREENSCRIPT";
		case SCREENSECRETSTRIGGERED: return "SCREENSECRETSTRIGGERED";
		case SCREENINITD: return "SCREENINITD";
		case LINKINITD: return "LINKINITD";
		case NPCDATAWEAPONINITD: return "NPCDATAWEAPONINITD";
		case NPCDATAWEAPONSCRIPT: return "NPCDATAWEAPONSCRIPT";
		
		case NPCSCRIPTTILE: return "NPCSCRIPTTILE";
		case NPCSCRIPTFLIP: return "NPCSCRIPTFLIP";
		case LWPNSCRIPTTILE: return "LWPNSCRIPTTILE";
		case LWPNSCRIPTFLIP: return "LWPNSCRIPTFLIP";
		case EWPNSCRIPTTILE: return "EWPNSCRIPTTILE";
		case EWPNSCRIPTFLIP: return "EWPNSCRIPTFLIP";
		
		case LINKENGINEANIMATE: return "LINKENGINEANIMATE";
		case NPCENGINEANIMATE: return "NPCENGINEANIMATE";
		case LWPNENGINEANIMATE: return "LWPNENGINEANIMATE";
		case EWPNENGINEANIMATE: return "EWPNENGINEANIMATE";
		
		case SKIPCREDITS: return "SKIPCREDITS";
		case SKIPF6: return "SKIPF6";
		case LWPNUSEWEAPON: return "LWPNUSEWEAPON";
		case LWPNUSEDEFENCE: return "LWPNUSEDEFENCE";

		case LWPNROTATION: return "LWPNROTATION";
		case EWPNROTATION: return "EWPNROTATION";
		case NPCROTATION: return "NPCROTATION";
		case ITEMROTATION: return "ITEMROTATION";
		case LINKROTATION: return "LINKROTATION";
		
		case LWPNSCALE: return "LWPNSCALE";
		case EWPNSCALE: return "EWPNSCALE";
		case NPCSCALE: return "NPCSCALE";
		case ITEMSCALE: return "ITEMSCALE";
		case LINKSCALE: return "LINKSCALE";
		case ITEMSPRITESCRIPT: return "ITEMSPRITESCRIPT";
		case FFRULE: return "FFRULE";
		case NUMDRAWS: return "NUMDRAWS";
		case MAXDRAWS: return "MAXDRAWS";
		case BITMAPWIDTH: return "BITMAPWIDTH";
		case BITMAPHEIGHT: return "BITMAPHEIGHT";
		case ALLOCATEBITMAPR: return "ALLOCATEBITMAPR";
		case KEYMODIFIERS: return "KEYMODIFIERS";
		case SIMULATEKEYPRESS: return "SIMULATEKEYPRESS";
		case KEYBINDINGS: return "KEYBINDINGS";
		
		case MAPDATASCRIPT: return "MAPDATASCRIPT";
		
		case MAPDATAINITDARRAY: return "MAPDATAINITDARRAY";
		
		case LWPNGRAVITY: return "LWPNGRAVITY";
		case EWPNGRAVITY: return "EWPNGRAVITY";
		case NPCGRAVITY: return "NPCGRAVITY";
		case ITEMGRAVITY: return "ITEMGRAVITY";
		case LINKGRAVITY: return "LINKGRAVITY";
		case MAPDATASIDEWARPID: return "MAPDATASIDEWARPID";
		case SCREENSIDEWARPID: return "SCREENSIDEWARPID";
		
		case MAPDATALAYERINVIS: return "MAPDATALAYERINVIS";
		case MAPDATASCRIPTDRAWS: return "MAPDATASCRIPTDRAWS";
		
		case ITEMSCRIPTTILE: return "ITEMSCRIPTTILE";
		case ITEMSCRIPTFLIP: return "ITEMSCRIPTFLIP";
		case MAPDATAMAP: return "MAPDATAMAP";
		case MAPDATASCREEN: return "MAPDATASCREEN";
		case IDATAVALIDATE: return "IDATAVALIDATE";
		case IDATAVALIDATE2: return "IDATAVALIDATE2";
		case GAMESUSPEND: return "GAMESUSPEND";
		case LINKOTILE: return "LINKOTILE";
		case LINKOFLIP: return "LINKOFLIP";
		case ITEMSPRITEINITD: return "ITEMSPRITEINITD";
		case ZSCRIPTVERSION: return "ZSCRIPTVERSION";
		case REFFILE: return "REFFILE";
		case REFDIRECTORY: return "REFDIRECTORY";
		case REFSTACK: return "REFSTACK";
		case REFSUBSCREEN: return "REFSUBSCREEN";
		case REFRNG: return "REFRNG";
		
		case CLASS_THISKEY: return "CLASS_THISKEY";
		case CLASS_THISKEY2: return "CLASS_THISKEY2";
		case ZELDABETATYPE: return "ZELDABETATYPE";
		case HEROCOYOTETIME: return "HEROCOYOTETIME";
		case FFCLASTCHANGERX: return "FFCLASTCHANGERX";
		case FFCLASTCHANGERY: return "FFCLASTCHANGERY";
		case LWPNTIMEOUT: return "LWPNTIMEOUT";
		case EWPNTIMEOUT: return "EWPNTIMEOUT";
		case COMBODTRIGGERLSTATE: return "COMBODTRIGGERLSTATE";
		case COMBODTRIGGERGSTATE: return "COMBODTRIGGERGSTATE";
		case COMBODTRIGGERGTIMER: return "COMBODTRIGGERGTIMER";
		case GAMEMOUSECURSOR: return "GAMEMOUSECURSOR";
		case COMBODTRIGGERGENSCRIPT: return "COMBODTRIGGERGENSCRIPT";
		case COMBODTRIGGERGROUP: return "COMBODTRIGGERGROUP";
		case COMBODTRIGGERGROUPVAL: return "COMBODTRIGGERGROUPVAL";
		case HEROLIFTEDWPN: return "HEROLIFTEDWPN";
		case HEROLIFTTIMER: return "HEROLIFTTIMER";
		case HEROLIFTMAXTIMER: return "HEROLIFTMAXTIMER";
		case HEROLIFTHEIGHT: return "HEROLIFTHEIGHT";
		case HEROHAMMERSTATE: return "HEROHAMMERSTATE";
		case HEROLIFTFLAGS: return "HEROLIFTFLAGS";
		case COMBODLIFTWEAPONITEM: return "COMBODLIFTWEAPONITEM";
		case LWPNDEATHITEM: return "LWPNDEATHITEM";
		case LWPNDEATHDROPSET: return "LWPNDEATHDROPSET";
		case LWPNDEATHIPICKUP: return "LWPNDEATHIPICKUP";
		case LWPNDEATHSPRITE: return "LWPNDEATHSPRITE";
		case LWPNDEATHSFX: return "LWPNDEATHSFX";
		case EWPNDEATHITEM: return "EWPNDEATHITEM";
		case EWPNDEATHDROPSET: return "EWPNDEATHDROPSET";
		case EWPNDEATHIPICKUP: return "EWPNDEATHIPICKUP";
		case EWPNDEATHSPRITE: return "EWPNDEATHSPRITE";
		case EWPNDEATHSFX: return "EWPNDEATHSFX";
		case LWPNLIFTLEVEL: return "LWPNLIFTLEVEL";
		case LWPNLIFTTIME: return "LWPNLIFTTIME";
		case LWPNLIFTHEIGHT: return "LWPNLIFTHEIGHT";
		case EWPNLIFTLEVEL: return "EWPNLIFTLEVEL";
		case EWPNLIFTTIME: return "EWPNLIFTTIME";
		case EWPNLIFTHEIGHT: return "EWPNLIFTHEIGHT";
		case HEROSHIELDJINX: return "HEROSHIELDJINX";
		case MAPDATALENSSHOWS: return "MAPDATALENSSHOWS";
		case MAPDATALENSHIDES: return "MAPDATALENSHIDES";
		case SCREENLENSSHOWS: return "SCREENLENSSHOWS";
		case SCREENLENSHIDES: return "SCREENLENSHIDES";
		case GAMETRIGGROUPS: return "GAMETRIGGROUPS";
		case GAMEOVERRIDEITEMS: return "GAMEOVERRIDEITEMS";
		case DMAPDATASUBSCRO: return "DMAPDATASUBSCRO";
		case REFSUBSCREENPAGE: return "REFSUBSCREENPAGE";
		case REFSUBSCREENWIDG: return "REFSUBSCREENWIDG";
		case SUBDATACURPG: return "SUBDATACURPG";
		case SUBDATANUMPG: return "SUBDATANUMPG";
		case SUBDATAPAGES: return "SUBDATAPAGES";
		case SUBDATATYPE: return "SUBDATATYPE";
		case SUBDATAFLAGS: return "SUBDATAFLAGS";
		case SUBDATACURSORPOS: return "SUBDATACURSORPOS";
		case SUBDATASCRIPT: return "SUBDATASCRIPT";
		case SUBDATAINITD: return "SUBDATAINITD";
		case SUBDATABTNLEFT: return "SUBDATABTNLEFT";
		case SUBDATABTNRIGHT: return "SUBDATABTNRIGHT";
		case SUBDATATRANSLEFTTY: return "SUBDATATRANSLEFTTY";
		case SUBDATATRANSLEFTSFX: return "SUBDATATRANSLEFTSFX";
		case SUBDATATRANSLEFTFLAGS: return "SUBDATATRANSLEFTFLAGS";
		case SUBDATATRANSLEFTARGS: return "SUBDATATRANSLEFTARGS";
		case PORTALX: return "PORTALX";
		case PORTALY: return "PORTALY";
		case PORTALDMAP: return "PORTALDMAP";
		case PORTALSCREEN: return "PORTALSCREEN";
		case PORTALACLK: return "PORTALACLK";
		case PORTALAFRM: return "PORTALAFRM";
		case PORTALOTILE: return "PORTALOTILE";
		case PORTALASPD: return "PORTALASPD";
		case PORTALFRAMES: return "PORTALFRAMES";
		case PORTALSAVED: return "PORTALSAVED";
		case PORTALCLOSEDIS: return "PORTALCLOSEDIS";
		case REFPORTAL: return "REFPORTAL";
		case REFSAVPORTAL: return "REFSAVPORTAL";
		case PORTALWARPSFX: return "PORTALWARPSFX";
		case PORTALWARPVFX: return "PORTALWARPVFX";
		case SAVEDPORTALX: return "SAVEDPORTALX";
		case SAVEDPORTALY: return "SAVEDPORTALY";
		case SAVEDPORTALSRCDMAP: return "SAVEDPORTALSRCDMAP";
		case SAVEDPORTALDESTDMAP: return "SAVEDPORTALDESTDMAP";
		case SAVEDPORTALSRCSCREEN: return "SAVEDPORTALSRCSCREEN";
		case SAVEDPORTALDSTSCREEN: return "SAVEDPORTALDSTSCREEN";
		case SAVEDPORTALWARPSFX: return "SAVEDPORTALWARPSFX";
		case SAVEDPORTALWARPVFX: return "SAVEDPORTALWARPVFX";
		case SAVEDPORTALSPRITE: return "SAVEDPORTALSPRITE";
		case SAVEDPORTALPORTAL: return "SAVEDPORTALPORTAL";
		case PORTALCOUNT: return "PORTALCOUNT";
		case SAVEDPORTALCOUNT: return "SAVEDPORTALCOUNT";
		
		case LINKCLIMBING: return "LINKCLIMBING";
		case NPCIMMORTAL: return "NPCIMMORTAL";
		case NPCNOSLIDE: return "NPCNOSLIDE";
		case NPCKNOCKBACKSPEED: return "NPCKNOCKBACKSPEED";
		case NPCNOSCRIPTKB: return "NPCNOSCRIPTKB";
		case GETRENDERTARGET: return "GETRENDERTARGET";
		case HERONOSTEPFORWARD: return "HERONOSTEPFORWARD";
		
		case SCREENDATATWARPRETSQR: return "SCREENDATATWARPRETSQR";
		case SCREENDATASWARPRETSQR: return "SCREENDATASWARPRETSQR";
		case MAPDATATWARPRETSQR: return "MAPDATATWARPRETSQR";
		case MAPDATASWARPRETSQR: return "MAPDATASWARPRETSQR";
		case NPCSUBMERGED: return "NPCSUBMERGED";
		case EWPNPARENTUID: return "EWPNPARENTUID";
		case GAMEGRAVITY: return "GAMEGRAVITY";
		case GAMESCROLLING: return "GAMESCROLLING";
		case COMBODASPEED: return "COMBODASPEED";
		case DROPSETITEMS: return "DROPSETITEMS";
		case DROPSETCHANCES: return "DROPSETCHANCES";
		case DROPSETNULLCHANCE: return "DROPSETNULLCHANCE";
		case DROPSETCHOOSE: return "DROPSETCHOOSE";
		case NPCPARENTUID: return "NPCPARENTUID";
		case KEYPRESS: return "KEYPRESS";
		case KEYINPUT: return "KEYINPUT";
		
		case SPRITEMAXNPC: return "SPRITEMAXNPC";
		case SPRITEMAXLWPN: return "SPRITEMAXLWPN";
		case SPRITEMAXEWPN: return "SPRITEMAXEWPN";
		case SPRITEMAXITEM: return "SPRITEMAXITEM";
		case SPRITEMAXPARTICLE: return "SPRITEMAXPARTICLE";
		case SPRITEMAXDECO: return "SPRITEMAXDECO";
		case HEROHEALTHBEEP: return "HEROHEALTHBEEP";
		case NPCRANDOM: return "NPCRANDOM";
		case COMBOXR: return "COMBOXR";
		case COMBOYR: return "COMBOYR";
		case COMBOPOSR: return "COMBOPOSR";
		case COMBOLAYERR: return "COMBOLAYERR";
		case COMBODATASCRIPT: return "COMBODATASCRIPT";
		case COMBODATAINITD: return "COMBODATAINITD";
		case HEROSCRIPTCSET: return "HEROSCRIPTCSET";
		case SHOPDATATYPE: return "SHOPDATATYPE";
		case HEROSTEPS: return "HEROSTEPS";
		case HEROSTEPRATE: return "HEROSTEPRATE";
		case COMBODOTILE: return "COMBODOTILE";
		case COMBODFRAME: return "COMBODFRAME";
		case COMBODACLK: return "COMBODACLK";
		
		case FILEPOS: return "FILEPOS";
		case FILEEOF: return "FILEEOF";
		case FILEERR: return "FILEERR";
		
		case INCQST: return "INCQST";
		case HEROJUMPCOUNT: return "HEROJUMPCOUNT";
		
		case HEROPULLDIR: return "HEROPULLDIR";
		case HEROPULLCLK: return "HEROPULLCLK";
		case HEROFALLCLK: return "HEROFALLCLK";
		case HEROFALLCMB: return "HEROFALLCMB";
		case HEROMOVEFLAGS: return "HEROMOVEFLAGS";
		case ITEMFALLCLK: return "ITEMFALLCLK";
		case ITEMFALLCMB: return "ITEMFALLCMB";
		case ITEMMOVEFLAGS: return "ITEMMOVEFLAGS";
		case LWPNFALLCLK: return "LWPNFALLCLK";
		case LWPNFALLCMB: return "LWPNFALLCMB";
		case LWPNMOVEFLAGS: return "LWPNMOVEFLAGS";
		case EWPNFALLCLK: return "EWPNFALLCLK";
		case EWPNFALLCMB: return "EWPNFALLCMB";
		case EWPNMOVEFLAGS: return "EWPNMOVEFLAGS";
		case NPCFALLCLK: return "NPCFALLCLK";
		case NPCFALLCMB: return "NPCFALLCMB";
		case NPCMOVEFLAGS: return "NPCMOVEFLAGS";
		case ISBLANKTILE: return "ISBLANKTILE";
		case LWPNSPECIAL: return "LWPNSPECIAL";
		case MODULEGETINT: return "MODULEGETINT";
		case MODULEGETSTR: return "MODULEGETSTR";
		case NPCORIGINALHP: return "NPCORIGINALHP";
		
		case CLOCKCLK: return "CLOCKCLK";
		case CLOCKACTIVE: return "CLOCKACTIVE";
		case NPCHITDIR: return "NPCHITDIR";
		case DMAPDATAFLAGARR: return "DMAPDATAFLAGARR";
		case LINKCSET: return "LINKCSET";
		case NPCSLIDECLK: return "NPCSLIDECLK";
		case NPCFADING: return "NPCFADING";
		case DISTANCE: return "DISTANCE";
		case DISTANCESCALE: return "DISTANCESCALE";
		case STDARR: return "STDARR";
		case GHOSTARR: return "GHOSTARR";
		case TANGOARR: return "TANGOARR";
		case NPCHALTCLK: return "NPCHALTCLK";
		case NPCMOVESTATUS: return "NPCMOVESTATUS";
		case NPCFRAME: return "NPCFRAME";
		
		case DIRECTORYSIZE: return "DIRECTORYSIZE";
		case STACKSIZE: return "STACKSIZE";
		case STACKFULL: return "STACKFULL";
		case LONGDISTANCE: return "LONGDISTANCE ";
		case LONGDISTANCESCALE: return "LONGDISTANCESCALE";
		case ACTIVESSSPEED: return "ACTIVESSSPEED";
		case HEROISWARPING: return "HEROISWARPING";
		
		case ITEMGLOWRAD: return "ITEMGLOWRAD";
		case NPCGLOWRAD: return "NPCGLOWRAD";
		case LWPNGLOWRAD: return "LWPNGLOWRAD";
		case EWPNGLOWRAD: return "EWPNGLOWRAD";
		
		case ITEMGLOWSHP: return "ITEMGLOWSHP";
		case NPCGLOWSHP: return "NPCGLOWSHP";
		case LWPNGLOWSHP: return "LWPNGLOWSHP";
		case EWPNGLOWSHP: return "EWPNGLOWSHP";
		
		case ITEMDIR: return "ITEMDIR";
		case ITEMFORCEGRAB: return "ITEMFORCEGRAB";
		case COMBODTRIGGERITEM: return "COMBODTRIGGERITEM";
		case COMBODTRIGGERTIMER: return "COMBODTRIGGERTIMER";
		case COMBODTRIGGERSFX: return "COMBODTRIGGERSFX";
		case COMBODTRIGGERCHANGECMB: return "COMBODTRIGGERCHANGECMB";
		case COMBODTRIGGERPROX: return "COMBODTRIGGERPROX";
		case COMBODTRIGGERLIGHTBEAM: return "COMBODTRIGGERLIGHTBEAM";
		case COMBODTRIGGERCTR: return "COMBODTRIGGERCTR";
		case COMBODTRIGGERCTRAMNT: return "COMBODTRIGGERCTRAMNT";
		
		case COMBODTRIGGERCOOLDOWN: return "COMBODTRIGGERCOOLDOWN";
		case COMBODTRIGGERCOPYCAT: return "COMBODTRIGGERCOPYCAT";
		case COMBODTRIGITEMPICKUP: return "COMBODTRIGITEMPICKUP";
		case COMBODTRIGEXSTATE: return "COMBODTRIGEXSTATE";
		case COMBODTRIGSPAWNENEMY: return "COMBODTRIGSPAWNENEMY";
		case COMBODTRIGSPAWNITEM: return "COMBODTRIGSPAWNITEM";
		case COMBODTRIGCSETCHANGE: return "COMBODTRIGCSETCHANGE";
		case COMBODLIFTGFXCOMBO: return "COMBODLIFTGFXCOMBO";
		case COMBODLIFTGFXCCSET: return "COMBODLIFTGFXCCSET";
		case COMBODLIFTUNDERCMB: return "COMBODLIFTUNDERCMB";
		case COMBODLIFTUNDERCS: return "COMBODLIFTUNDERCS";
		case COMBODLIFTDAMAGE: return "COMBODLIFTDAMAGE";
		case COMBODLIFTLEVEL: return "COMBODLIFTLEVEL";
		case COMBODLIFTITEM: return "COMBODLIFTITEM";
		case COMBODLIFTFLAGS: return "COMBODLIFTFLAGS";
		case COMBODLIFTGFXTYPE: return "COMBODLIFTGFXTYPE";
		case COMBODLIFTGFXSPRITE: return "COMBODLIFTGFXSPRITE";
		case COMBODLIFTSFX: return "COMBODLIFTSFX";
		case COMBODLIFTBREAKSPRITE: return "COMBODLIFTBREAKSPRITE";
		case COMBODLIFTBREAKSFX: return "COMBODLIFTBREAKSFX";
		case COMBODLIFTHEIGHT: return "COMBODLIFTHEIGHT";
		case COMBODLIFTTIME: return "COMBODLIFTTIME";
		
		case SCREENEXSTATED: return "SCREENEXSTATED";
		case MAPDATAEXSTATED: return "MAPDATAEXSTATED";
		case HEROSTANDING: return "HEROSTANDING";
		
		case ITEMENGINEANIMATE: return "ITEMENGINEANIMATE";
		case LWPNUNBL: return "LWPNUNBL";
		case EWPNUNBL: return "EWPNUNBL";
		
		case NPCSHADOWSPR: return "NPCSHADOWSPR";
		case LWPNSHADOWSPR: return "LWPNSHADOWSPR";
		case EWPNSHADOWSPR: return "EWPNSHADOWSPR";
		case ITEMSHADOWSPR: return "ITEMSHADOWSPR";
		case NPCSPAWNSPR: return "NPCSPAWNSPR";
		case NPCDEATHSPR: return "NPCDEATHSPR";
		case NPCDSHADOWSPR: return "NPCDSHADOWSPR";
		case NPCDSPAWNSPR: return "NPCDSPAWNSPR";
		case NPCDDEATHSPR: return "NPCDDEATHSPR";
		case ITEMDROPPEDBY: return "ITEMDROPPEDBY";
		
		case REFGENERICDATA: return "REFGENERICDATA";
		case GENDATARUNNING: return "GENDATARUNNING";
		case GENDATASIZE: return "GENDATASIZE";
		case GENDATAEXITSTATE: return "GENDATAEXITSTATE";
		case GENDATADATA: return "GENDATADATA";
		case GENDATAINITD: return "GENDATAINITD";
		case GENDATARELOADSTATE: return "GENDATARELOADSTATE";
		case GENDATAEVENTSTATE: return "GENDATAEVENTSTATE";
		
		case COMBODCSET2FLAGS: return "COMBODCSET2FLAGS";
		case HEROIMMORTAL: return "HEROIMMORTAL";
		case NPCCANFLICKER: return "NPCCANFLICKER";
		case NPCDROWNCLK: return "NPCDROWNCLK";
		case NPCDROWNCMB: return "NPCDROWNCMB";
		case ITEMDROWNCLK: return "ITEMDROWNCLK";
		case ITEMDROWNCMB: return "ITEMDROWNCMB";
		case LWPNDROWNCLK: return "LWPNDROWNCLK";
		case LWPNDROWNCMB: return "LWPNDROWNCMB";
		case EWPNDROWNCLK: return "EWPNDROWNCLK";
		case EWPNDROWNCMB: return "EWPNDROWNCMB";
		case HERODROWNCLK: return "HERODROWNCLK";
		case HERODROWNCMB: return "HERODROWNCMB";
		case NPCFAKEZ: return "NPCFAKEZ";
		case ITEMFAKEZ: return "ITEMFAKEZ";
		case LWPNFAKEZ: return "LWPNFAKEZ";
		case EWPNFAKEZ: return "EWPNFAKEZ";
		case HEROFAKEZ: return "HEROFAKEZ";
		case NPCFAKEJUMP: return "NPCFAKEJUMP";
		case ITEMFAKEJUMP: return "ITEMFAKEJUMP";
		case LWPNFAKEJUMP: return "LWPNFAKEJUMP";
		case EWPNFAKEJUMP: return "EWPNFAKEJUMP";
		case HEROFAKEJUMP: return "HEROFAKEJUMP";
		
		case REFPALDATA: return "REFPALDATA";
		case PALDATACOLOR: return "PALDATACOLOR";
		case PALDATAR: return "PALDATAR";
		case PALDATAG: return "PALDATAG";
		case PALDATAB: return "PALDATAB";
		case DMAPDATALOOPSTART: return "DMAPDATALOOPSTART";
		case DMAPDATALOOPEND: return "DMAPDATALOOPEND";
		case DMAPDATAXFADEIN: return "DMAPDATAXFADEIN";
		case DMAPDATAXFADEOUT: return "DMAPDATAXFADEOUT";
		case MUSICUPDATECOND: return "MUSICUPDATECOND";
		case MUSICUPDATEFLAGS: return "MUSICUPDATEFLAGS";
		case DMAPDATAINTROSTRINGID: return "DMAPDATAINTROSTRINGID";
		case IS8BITTILE: return "IS8BITTILE";
		case NPCFLICKERCOLOR: return "NPCFLICKERCOLOR";
		case HEROFLICKERCOLOR: return "HEROFLICKERCOLOR";
		case NPCFLASHINGCSET: return "NPCFLASHINGCSET";
		case HEROFLASHINGCSET: return "HEROFLASHINGCSET";
		case NPCFLICKERTRANSP: return "NPCFLICKERTRANSP";
		case HEROFLICKERTRANSP: return "HEROFLICKERTRANSP";
		case RESRVD_VAR_MOOSH15: return "RESRVD_VAR_MOOSH15";
		case RESRVD_VAR_MOOSH16: return "RESRVD_VAR_MOOSH16";
		case RESRVD_VAR_MOOSH17: return "RESRVD_VAR_MOOSH17";
		case RESRVD_VAR_MOOSH18: return "RESRVD_VAR_MOOSH18";
		case RESRVD_VAR_MOOSH19: return "RESRVD_VAR_MOOSH19";
		case RESRVD_VAR_MOOSH20: return "RESRVD_VAR_MOOSH20";
		case RESRVD_VAR_MOOSH21: return "RESRVD_VAR_MOOSH21";
		case RESRVD_VAR_MOOSH22: return "RESRVD_VAR_MOOSH22";
		case RESRVD_VAR_MOOSH23: return "RESRVD_VAR_MOOSH23";
		case RESRVD_VAR_MOOSH24: return "RESRVD_VAR_MOOSH24";
		case RESRVD_VAR_MOOSH25: return "RESRVD_VAR_MOOSH25";
		case RESRVD_VAR_MOOSH26: return "RESRVD_VAR_MOOSH26";
		case RESRVD_VAR_MOOSH27: return "RESRVD_VAR_MOOSH27";
		case RESRVD_VAR_MOOSH28: return "RESRVD_VAR_MOOSH28";
		case RESRVD_VAR_MOOSH29: return "RESRVD_VAR_MOOSH29";
		case RESRVD_VAR_MOOSH30: return "RESRVD_VAR_MOOSH30";
		case REGION_ID: return "REGION_ID";
		case REGION_ORIGIN_SCREEN: return "REGION_ORIGIN_SCREEN";

		case RESRVD_VAR_Z3_01: return "RESRVD_VAR_Z3_01";
		case RESRVD_VAR_Z3_02: return "RESRVD_VAR_Z3_02";
		case RESRVD_VAR_Z3_03: return "RESRVD_VAR_Z3_03";
		case RESRVD_VAR_Z3_04: return "RESRVD_VAR_Z3_04";
		case RESRVD_VAR_Z3_05: return "RESRVD_VAR_Z3_05";
		case RESRVD_VAR_Z3_06: return "RESRVD_VAR_Z3_06";
		case RESRVD_VAR_Z3_07: return "RESRVD_VAR_Z3_07";
		case RESRVD_VAR_Z3_12: return "RESRVD_VAR_Z3_12";
		
		case SUBDATATRANSRIGHTTY: return "SUBDATATRANSRIGHTTY";
		case SUBDATATRANSRIGHTSFX: return "SUBDATATRANSRIGHTSFX";
		case SUBDATATRANSRIGHTFLAGS: return "SUBDATATRANSRIGHTFLAGS";
		case SUBDATATRANSRIGHTARGS: return "SUBDATATRANSRIGHTARGS";
		case SUBDATASELECTORDSTX: return "SUBDATASELECTORDSTX";
		case SUBDATASELECTORDSTY: return "SUBDATASELECTORDSTY";
		case SUBDATASELECTORDSTW: return "SUBDATASELECTORDSTW";
		case SUBDATASELECTORDSTH: return "SUBDATASELECTORDSTH";
		case SUBDATASELECTORWID: return "SUBDATASELECTORWID";
		case SUBDATASELECTORHEI: return "SUBDATASELECTORHEI";
		case SUBDATASELECTORTILE: return "SUBDATASELECTORTILE";
		case SUBDATASELECTORCSET: return "SUBDATASELECTORCSET";
		case SUBDATASELECTORFRM: return "SUBDATASELECTORFRM";
		case SUBDATASELECTORASPD: return "SUBDATASELECTORASPD";
		case SUBDATASELECTORDELAY: return "SUBDATASELECTORDELAY";
		case SUBDATATRANSCLK: return "SUBDATATRANSCLK";
		case SUBDATATRANSTY: return "SUBDATATRANSTY";
		case SUBDATATRANSFLAGS: return "SUBDATATRANSFLAGS";
		case SUBDATATRANSARGS: return "SUBDATATRANSARGS";
		case SUBDATATRANSFROMPG: return "SUBDATATRANSFROMPG";
		case SUBDATATRANSTOPG: return "SUBDATATRANSTOPG";
		case SUBDATASELECTORFLASHCSET: return "SUBDATASELECTORFLASHCSET";
		case GAMEASUBOPEN: return "GAMEASUBOPEN";
		case GAMENUMASUB: return "GAMENUMASUB";
		case GAMENUMPSUB: return "GAMENUMPSUB";
		case GAMENUMOSUB: return "GAMENUMOSUB";
		case SUBPGINDEX: return "SUBPGINDEX";
		case SUBPGNUMWIDG: return "SUBPGNUMWIDG";
		case SUBPGWIDGETS: return "SUBPGWIDGETS";
		case SUBPGSUBDATA: return "SUBPGSUBDATA";
		case SUBPGCURSORPOS: return "SUBPGCURSORPOS";
		case SUBWIDGTYPE: return "SUBWIDGTYPE";
		case SUBWIDGINDEX: return "SUBWIDGINDEX";
		case SUBWIDGPAGE: return "SUBWIDGPAGE";
		case SUBWIDGPOS: return "SUBWIDGPOS";
		case SUBWIDGPOSES: return "SUBWIDGPOSES";
		case SUBWIDGPOSFLAG: return "SUBWIDGPOSFLAG";
		case SUBWIDGX: return "SUBWIDGX";
		case SUBWIDGY: return "SUBWIDGY";
		case SUBWIDGW: return "SUBWIDGW";
		case SUBWIDGH: return "SUBWIDGH";
		case SUBWIDGGENFLAG: return "SUBWIDGGENFLAG";
		case SUBWIDGFLAG: return "SUBWIDGFLAG";
		case SUBWIDGSELECTORDSTX: return "SUBWIDGSELECTORDSTX";
		case SUBWIDGSELECTORDSTY: return "SUBWIDGSELECTORDSTY";
		case SUBWIDGSELECTORDSTW: return "SUBWIDGSELECTORDSTW";
		case SUBWIDGSELECTORDSTH: return "SUBWIDGSELECTORDSTH";
		case SUBWIDGSELECTORWID: return "SUBWIDGSELECTORWID";
		case SUBWIDGSELECTORHEI: return "SUBWIDGSELECTORHEI";
		case SUBWIDGSELECTORTILE: return "SUBWIDGSELECTORTILE";
		
		case SUBWIDGSELECTORCSET: return "SUBWIDGSELECTORCSET";
		case SUBWIDGSELECTORFLASHCSET: return "SUBWIDGSELECTORFLASHCSET";
		case SUBWIDGSELECTORFRM: return "SUBWIDGSELECTORFRM";
		case SUBWIDGSELECTORASPD: return "SUBWIDGSELECTORASPD";
		case SUBWIDGSELECTORDELAY: return "SUBWIDGSELECTORDELAY";
		case SUBWIDGPRESSSCRIPT: return "SUBWIDGPRESSSCRIPT";
		case SUBWIDGPRESSINITD: return "SUBWIDGPRESSINITD";
		case SUBWIDGBTNPRESS: return "SUBWIDGBTNPRESS";
		case SUBWIDGBTNPG: return "SUBWIDGBTNPG";
		case SUBWIDGPGMODE: return "SUBWIDGPGMODE";
		case SUBWIDGPGTARG: return "SUBWIDGPGTARG";
		case SUBWIDGTRANSPGTY: return "SUBWIDGTRANSPGTY";
		case SUBWIDGTRANSPGSFX: return "SUBWIDGTRANSPGSFX";
		case SUBWIDGTRANSPGFLAGS: return "SUBWIDGTRANSPGFLAGS";
		case SUBWIDGTRANSPGARGS: return "SUBWIDGTRANSPGARGS";
		
		case SUBWIDGTY_CSET: return "SUBWIDGTY_CSET";
		case SUBWIDGTY_TILE: return "SUBWIDGTY_TILE";
		
		case SUBWIDGTY_FONT: return "SUBWIDGTY_FONT";
		case SUBWIDGTY_ALIGN: return "SUBWIDGTY_ALIGN";
		case SUBWIDGTY_SHADOWTY: return "SUBWIDGTY_SHADOWTY";
		case SUBWIDGTY_COLOR_TXT: return "SUBWIDGTY_COLOR_TXT";
		case SUBWIDGTY_COLOR_SHD: return "SUBWIDGTY_COLOR_SHD";
		case SUBWIDGTY_COLOR_BG: return "SUBWIDGTY_COLOR_BG";
		case SUBWIDGTY_COLOR_TXT2: return "SUBWIDGTY_COLOR_TXT2";
		case SUBWIDGTY_COLOR_SHD2: return "SUBWIDGTY_COLOR_SHD2";
		case SUBWIDGTY_COLOR_BG2: return "SUBWIDGTY_COLOR_BG2";
		
		case SUBWIDGTY_COLOR_OLINE: return "SUBWIDGTY_COLOR_OLINE";
		case SUBWIDGTY_COLOR_FILL: return "SUBWIDGTY_COLOR_FILL";
		
		case SUBWIDGTY_BUTTON: return "SUBWIDGTY_BUTTON";
		case SUBWIDGTY_COUNTERS: return "SUBWIDGTY_COUNTERS";
		case SUBWIDGTY_MINDIG: return "SUBWIDGTY_MINDIG";
		case SUBWIDGTY_MAXDIG: return "SUBWIDGTY_MAXDIG";
		case SUBWIDGTY_INFITM: return "SUBWIDGTY_INFITM";
		case SUBWIDGTY_INFCHAR: return "SUBWIDGTY_INFCHAR";
		case SUBWIDGTY_COSTIND: return "SUBWIDGTY_COSTIND";
		
		case SUBWIDGTY_COLOR_PLAYER: return "SUBWIDGTY_COLOR_PLAYER";
		case SUBWIDGTY_COLOR_CMPBLNK: return "SUBWIDGTY_COLOR_CMPBLNK";
		case SUBWIDGTY_COLOR_CMPOFF: return "SUBWIDGTY_COLOR_CMPOFF";
		case SUBWIDGTY_COLOR_ROOM: return "SUBWIDGTY_COLOR_ROOM";
		case SUBWIDGTY_ITEMCLASS: return "SUBWIDGTY_ITEMCLASS";
		case SUBWIDGTY_ITEMID: return "SUBWIDGTY_ITEMID";
		case SUBWIDGTY_FRAMETILE: return "SUBWIDGTY_FRAMETILE";
		case SUBWIDGTY_FRAMECSET: return "SUBWIDGTY_FRAMECSET";
		case SUBWIDGTY_PIECETILE: return "SUBWIDGTY_PIECETILE";
		case SUBWIDGTY_PIECECSET: return "SUBWIDGTY_PIECECSET";
		case SUBWIDGTY_FLIP: return "SUBWIDGTY_FLIP";
		case SUBWIDGTY_NUMBER: return "SUBWIDGTY_NUMBER";
		case SUBWIDGTY_CORNER: return "SUBWIDGTY_CORNER";
		
		case SUBWIDGTY_FRAMES: return "SUBWIDGTY_FRAMES";
		case SUBWIDGTY_SPEED: return "SUBWIDGTY_SPEED";
		case SUBWIDGTY_DELAY: return "SUBWIDGTY_DELAY";
		case SUBWIDGTY_CONTAINER: return "SUBWIDGTY_CONTAINER";
		case SUBWIDGTY_GAUGE_WID: return "SUBWIDGTY_GAUGE_WID";
		case SUBWIDGTY_GAUGE_HEI: return "SUBWIDGTY_GAUGE_HEI";
		case SUBWIDGTY_UNITS: return "SUBWIDGTY_UNITS";
		case SUBWIDGTY_HSPACE: return "SUBWIDGTY_HSPACE";
		case SUBWIDGTY_VSPACE: return "SUBWIDGTY_VSPACE";
		case SUBWIDGTY_GRIDX: return "SUBWIDGTY_GRIDX";
		case SUBWIDGTY_GRIDY: return "SUBWIDGTY_GRIDY";
		case SUBWIDGTY_ANIMVAL: return "SUBWIDGTY_ANIMVAL";
		case SUBWIDGTY_SHOWDRAIN: return "SUBWIDGTY_SHOWDRAIN";
		case SUBWIDGTY_PERCONTAINER: return "SUBWIDGTY_PERCONTAINER";
		case SUBWIDGTY_TABSIZE: return "SUBWIDGTY_TABSIZE";
		
		case GAMEASUBYOFF: return "GAMEASUBYOFF";
		
		case SUBWIDGDISPITM: return "SUBWIDGDISPITM";
		case SUBWIDGEQPITM: return "SUBWIDGEQPITM";
		
		case SUBWIDG_DISPX: return "SUBWIDG_DISPX";
		case SUBWIDG_DISPY: return "SUBWIDG_DISPY";
		case SUBWIDG_DISPW: return "SUBWIDG_DISPW";
		case SUBWIDG_DISPH: return "SUBWIDG_DISPH";
		
		case SCREENSCRDATASIZE: return "SCREENSCRDATASIZE";
		case SCREENSCRDATA: return "SCREENSCRDATA";
		case MAPDATASCRDATASIZE: return "MAPDATASCRDATASIZE";
		case MAPDATASCRDATA: return "MAPDATASCRDATA";
		
		case HEROSHOVEOFFSET: return "HEROSHOVEOFFSET";
		
		case SCREENDATAGUYCOUNT: return "SCREENDATAGUYCOUNT";
		case MAPDATAGUYCOUNT: return "MAPDATAGUYCOUNT";
		
		case IDATAUSEBURNSPR: return "IDATAUSEBURNSPR";
		case IDATABURNINGSPR: return "IDATABURNINGSPR";
		case LWPNSPRITES: return "LWPNSPRITES";
		case EWPNSPRITES: return "EWPNSPRITES";
		
		case SCREENDATAEXDOOR: return "SCREENDATAEXDOOR";
		case MAPDATAEXDOOR: return "MAPDATAEXDOOR";
		case COMBODTRIGEXDOORDIR: return "COMBODTRIGEXDOORDIR";
		case COMBODTRIGEXDOORIND: return "COMBODTRIGEXDOORIND";
		
		case IDATABURNINGLIGHTRAD: return "IDATABURNINGLIGHTRAD";
		
		case LWPNBURNLIGHTRADIUS: return "LWPNBURNLIGHTRADIUS";
		case EWPNBURNLIGHTRADIUS: return "EWPNBURNLIGHTRADIUS";
		
		case IDATAATTRIB_L: return "IDATAATTRIB_L";
		
		default:
		{
			sprintf(temp, "d%d", ID);
			return string(temp);
		}
	}
}

string VarArgument::toString() const
{
	return VarToString(ID);
}

string GlobalArgument::toString() const
{
    char temp[40];
    sprintf(temp, "gd%d", ID);
    return string(temp);
}

string LabelArgument::toString() const
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

string LabelArgument::toStringSetV() const
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
        sprintf(temp, "%.4f", lineno * 0.0001f);
        return string(temp);
    }
}

string StringArgument::toString() const
{
	return util::escape_string(value);
}

string VectorArgument::toString() const
{
	return util::stringify_vector(value, true);
}

string OSetTrue::toString() const
{
    return "SETTRUE " + getArgument()->toString();
}

string OSetTrueI::toString() const
{
    return "SETTRUEI " + getArgument()->toString();
}

string OSetFalse::toString() const
{
    return "SETFALSE " + getArgument()->toString();
}

string OSetFalseI::toString() const
{
    return "SETFALSEI " + getArgument()->toString();
}

string OSetMore::toString() const
{
    return "SETMORE " + getArgument()->toString();
}

string OSetMoreI::toString() const
{
    return "SETMOREI " + getArgument()->toString();
}

string OSetLess::toString() const
{
    return "SETLESS " + getArgument()->toString();
}

string OSetLessI::toString() const
{
    return "SETLESSI " + getArgument()->toString();
}

string OSetImmediate::toString() const
{
	ostringstream oss;
	oss << "SETV " << getFirstArgument()->toString() << ",";
	Argument const* second = getSecondArgument();
	if (LabelArgument const* label = dynamic_cast<LabelArgument const*>(second))
		oss << label->toStringSetV();
	else
		oss << second->toString();
	return oss.str();
}

string OSetRegister::toString() const
{
    return "SETR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OReadPODArrayR::toString() const
{
	return "READPODARRAYR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OReadPODArrayI::toString() const
{
	return "READPODARRAYV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWritePODArrayRR::toString() const
{
	return "WRITEPODARRAYRR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWritePODArrayRI::toString() const
{
	return "WRITEPODARRAYRV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWritePODArrayIR::toString() const
{
	return "WRITEPODARRAYVR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWritePODArrayII::toString() const
{
	return "WRITEPODARRAYVV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OWritePODString::toString() const
{
	return "WRITEPODSTRING " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OWritePODArray::toString() const
{
	return "WRITEPODARRAY " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OConstructClass::toString() const
{
	return "ZCLASS_CONSTRUCT " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReadObject::toString() const
{
	return "ZCLASS_READ " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OWriteObject::toString() const
{
	return "ZCLASS_WRITE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OFreeObject::toString() const
{
	return "ZCLASS_FREE " + getArgument()->toString();
}
string OOwnObject::toString() const
{
	return "ZCLASS_OWN " + getArgument()->toString();
}
string ODestructor::toString() const
{
	return "STARTDESTRUCTOR " + getArgument()->toString();
}
string OGlobalObject::toString() const
{
	return "ZCLASS_GLOBALIZE " + getArgument()->toString();
}
string OObjOwnBitmap::toString() const
{
	return "OBJ_OWN_BITMAP " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnPaldata::toString() const
{
	return "OBJ_OWN_PALDATA " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnFile::toString() const
{
	return "OBJ_OWN_FILE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnDir::toString() const
{
	return "OBJ_OWN_DIR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnStack::toString() const
{
	return "OBJ_OWN_STACK " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnRNG::toString() const
{
	return "OBJ_OWN_RNG " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnClass::toString() const
{
	return "OBJ_OWN_CLASS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnArray::toString() const
{
	return "OBJ_OWN_ARRAY " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OQuitNoDealloc::toString() const
{
	return "QUIT_NO_DEALLOC";
}
string OSetCustomCursor::toString() const
{
	return "GAMESETCUSTOMCURSOR";
}
string OItemGetDispName::toString() const
{
	return "ITEMGETDISPLAYNAME " + getArgument()->toString();
}
string OItemSetDispName::toString() const
{
	return "ITEMSETDISPLAYNAME " + getArgument()->toString();
}
string OItemGetShownName::toString() const
{
	return "ITEMGETSHOWNNAME " + getArgument()->toString();
}
string OHeroMoveXY::toString() const
{
	return "HEROMOVEXY";
}
string OHeroCanMoveXY::toString() const
{
	return "HEROCANMOVEXY";
}
string OHeroLiftRelease::toString() const
{
	return "HEROLIFTRELEASE";
}
string OHeroLiftGrab::toString() const
{
	return "HEROLIFTGRAB";
}
string OHeroIsFlickerFrame::toString() const
{
	return "HEROISFLICKERFRAME";
}
string OLoadPortalRegister::toString() const
{
	return "LOADPORTAL " + getArgument()->toString();
}
string OCreatePortal::toString() const
{
	return "CREATEPORTAL";
}
string OLoadSavPortalRegister::toString() const
{
	return "LOADSAVPORTAL " + getArgument()->toString();
}
string OCreateSavPortal::toString() const
{
	return "CREATESAVPORTAL";
}
string OPortalRemove::toString() const
{
	return "PORTALREMOVE";
}
string OSavedPortalRemove::toString() const
{
	return "SAVEDPORTALREMOVE";
}
string OSavedPortalGenerate::toString() const
{
	return "SAVEDPORTALGENERATE";
}
string OUseSpritePortal::toString() const
{
	return "PORTALUSESPRITE " + getArgument()->toString();
}
string OHeroMoveAtAngle::toString() const
{
	return "HEROMOVEATANGLE";
}
string OHeroCanMoveAtAngle::toString() const
{
	return "HEROCANMOVEATANGLE";
}
string OHeroMove::toString() const
{
	return "HEROMOVE";
}
string OHeroCanMove::toString() const
{
	return "HEROCANMOVE";
}
string ODrawLightCircle::toString() const
{
	return "DRAWLIGHT_CIRCLE";
}
string ODrawLightSquare::toString() const
{
	return "DRAWLIGHT_SQUARE";
}
string ODrawLightCone::toString() const
{
	return "DRAWLIGHT_CONE";
}
string OPeek::toString() const
{
	return "PEEK " + getArgument()->toString();
}
string OPeekAtImmediate::toString() const
{
	return "PEEKATV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();;
}
string OMakeVargArray::toString() const
{
	return "MAKEVARGARRAY";
}
string OPrintfArr::toString() const
{
	return "PRINTFA";
}
string OSPrintfArr::toString() const
{
	return "SPRINTFA";
}
string OCurrentItemID::toString() const
{
	return "CURRENTITEMID";
}
string OArrayPush::toString() const
{
	return "ARRAYPUSH";
}
string OArrayPop::toString() const
{
	return "ARRAYPOP";
}
string OLoadSubscreenDataRV::toString() const
{
	return "LOADSUBDATARV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OSwapSubscrV::toString() const
{
	return "SWAPSUBSCREENV " + getArgument()->toString();
}
string OGetSubscreenName::toString() const
{
	return "SUBDATA_GET_NAME " + getArgument()->toString();
}
string OSetSubscreenName::toString() const
{
	return "SUBDATA_SET_NAME " + getArgument()->toString();
}


string OAddImmediate::toString() const
{
    return "ADDV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAddRegister::toString() const
{
    return "ADDR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSubImmediate::toString() const
{
    return "SUBV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSubImmediate2::toString() const
{
    return "SUBV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSubRegister::toString() const
{
    return "SUBR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMultImmediate::toString() const
{
    return "MULTV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OMultRegister::toString() const
{
    return "MULTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODivImmediate::toString() const
{
    return "DIVV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODivImmediate2::toString() const
{
    return "DIVV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODivRegister::toString() const
{
    return "DIVR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCompareImmediate::toString() const
{
    return "COMPAREV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCompareImmediate2::toString() const
{
    return "COMPAREV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCompareRegister::toString() const
{
    return "COMPARER " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OInternalStringCompare::toString() const
{
	return "STRCMPR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OInternalInsensitiveStringCompare::toString() const
{
	return "STRICMPR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWaitframe::toString() const
{
    return "WAITFRAME";
}

string OWaitframes::toString() const
{
    return "WAITFRAMESR " + getArgument()->toString();
}

string OWaitdraw::toString() const
{
    return "WAITDRAW";
}

string OWaitTo::toString() const
{
    return "WAITTO " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWaitEvent::toString() const
{
    return "WAITEVENT";
}

string ONoOp::toString() const
{
	return "NOP";
}

string OCastBoolI::toString() const
{
	return "CASTBOOLI " + getArgument()->toString();
}

string OCastBoolF::toString() const
{
	return "CASTBOOLF " + getArgument()->toString();
}

//I would like to add a Jump instruction tot he parser, which would be 'GOTOLABEL' -Z
string OGotoImmediate::toString() const
{
    return "GOTO " + getArgument()->toString();
}

string OGotoTrueImmediate::toString() const
{
    return "GOTOTRUE " + getArgument()->toString();
}

string OGotoFalseImmediate::toString() const
{
    return "GOTOFALSE " + getArgument()->toString();
}

string OGotoMoreImmediate::toString() const
{
    return "GOTOMORE " + getArgument()->toString();
}

string OGotoLessImmediate::toString() const
{
    return "GOTOLESS " + getArgument()->toString();
}

string OPushRegister::toString() const
{
    return "PUSHR " + getArgument()->toString();
}

string OPushImmediate::toString() const
{
	ostringstream oss;
	oss << "PUSHV ";
	Argument const* arg = getArgument();
	if (LabelArgument const* label = dynamic_cast<LabelArgument const*>(arg))
		oss << label->toStringSetV();
	else
		oss << arg->toString();
	return oss.str();
    return "PUSHV " + getArgument()->toString();
}

string OPopRegister::toString() const
{
    return "POP " + getArgument()->toString();
}

string OPopArgsRegister::toString() const
{
    return "POPARGS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPushVargV::toString() const
{
    return "PUSHVARGV " + getArgument()->toString();
}

string OPushVargR::toString() const
{
    return "PUSHVARGR " + getArgument()->toString();
}

string OLoadIndirect::toString() const
{
    return "LOADI " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OStoreIndirect::toString() const
{
    return "STOREI " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OLoadDirect::toString() const
{
    return "LOADD " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OStoreDirect::toString() const
{
    return "STORED " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OQuit::toString() const
{
    return "QUIT";
}

string OGotoRegister::toString() const
{
    return "GOTOR " + getArgument()->toString();
}

string OAndImmediate::toString() const
{
    return "ANDV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAndRegister::toString() const
{
    return "ANDR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OOrImmediate::toString() const
{
    return "ORV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OOrRegister::toString() const
{
    return "ORR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OXorImmediate::toString() const
{
    return "XORV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OXorRegister::toString() const
{
    return "XORR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitAndImmediate::toString() const
{
    return "ANDV32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitAndRegister::toString() const
{
    return "ANDR32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitOrImmediate::toString() const
{
    return "ORV32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitOrRegister::toString() const
{
    return "ORR32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitXorImmediate::toString() const
{
    return "XORV32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitXorRegister::toString() const
{
    return "XORR32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSinRegister::toString() const
{
    return "SINR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCosRegister::toString() const
{
    return "COSR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OTanRegister::toString() const
{
    return "TANR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OEngineDegtoRad::toString() const
{
    return "DEGTORAD " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OEngineRadtoDeg::toString() const
{
    return "RADTODEG " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Ostrlen::toString() const
{
    return "STRINGLENGTH " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OATanRegister::toString() const
{
    return "ARCTANR " + getArgument()->toString();
}

string OArcCosRegister::toString() const
{
    return "ARCCOSR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OArcSinRegister::toString() const
{
    return "ARCSINR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMinRegister::toString() const
{
    return "MINR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMaxRegister::toString() const
{
    return "MAXR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OMaxNew::toString() const
{
    return "MAXVARG";
}
string OMinNew::toString() const
{
    return "MINVARG";
}
string OChoose::toString() const
{
    return "CHOOSEVARG";
}

string OPowRegister::toString() const
{
    return "POWERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OPowImmediate::toString() const
{
    return "POWERV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OPowImmediate2::toString() const
{
    return "POWERV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OLPowRegister::toString() const
{
    return "LPOWERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OLPowImmediate::toString() const
{
    return "LPOWERV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OLPowImmediate2::toString() const
{
    return "LPOWERV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OInvPowRegister::toString() const
{
    return "IPOWERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OFactorial::toString() const
{
    return "FACTORIAL " + getArgument()->toString();
}

string OAbsRegister::toString() const
{
    return "ABS " + getArgument()->toString();
}

string OLog10Register::toString() const
{
    return "LOG10 " + getArgument()->toString();
}

string OLogERegister::toString() const
{
    return "LOGE " + getArgument()->toString();
}

string OArraySize::toString() const
{
    return "ARRAYSIZE " + getArgument()->toString();
}


string OArraySizeF::toString() const
{
    return "ARRAYSIZEF " + getArgument()->toString();
}
string OArraySizeN::toString() const
{
    return "ARRAYSIZEN " + getArgument()->toString();
}
string OArraySizeE::toString() const
{
    return "ARRAYSIZEE " + getArgument()->toString();
}
string OArraySizeL::toString() const
{
    return "ARRAYSIZEL " + getArgument()->toString();
}
string OArraySizeB::toString() const
{
    return "ARRAYSIZEB " + getArgument()->toString();
}
string OArraySizeI::toString() const
{
    return "ARRAYSIZEI " + getArgument()->toString();
}
string OArraySizeID::toString() const
{
    return "ARRAYSIZEID " + getArgument()->toString();
}


string OCalcSplineRegister::toString() const
{
    return "CALCSPLINER" + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSetColorRegister::toString() const
{
    return "SETCOLORR";
}

string OSetDepthRegister::toString() const
{
    return "SETDEPTHR";
}

string OCollisionRectRegister::toString() const
{
    return "COLLISIONRECTR" + getArgument()->toString();
}

string OCollisionBoxRegister::toString() const
{
    return "COLLISIONBOXR" + getArgument()->toString();
}

string OLShiftImmediate::toString() const
{
    return "LSHIFTV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OLShiftRegister::toString() const
{
    return "LSHIFTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ORShiftImmediate::toString() const
{
    return "RSHIFTV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ORShiftRegister::toString() const
{
    return "RSHIFTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitLShiftImmediate::toString() const
{
    return "LSHIFTV32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitLShiftRegister::toString() const
{
    return "LSHIFTR32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitRShiftImmediate::toString() const
{
    return "RSHIFTV32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitRShiftRegister::toString() const
{
    return "RSHIFTR32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OModuloImmediate::toString() const
{
    return "MODV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OModuloImmediate2::toString() const
{
    return "MODV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OModuloRegister::toString() const
{
    return "MODR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ONot::toString() const
{
    return "BITNOT " + getArgument()->toString();
}

string O32BitNot::toString() const
{
    return "BITNOT32 " + getArgument()->toString();
}

string OTraceRegister::toString() const
{
    return "TRACER " + getArgument()->toString();
}
string OTraceImmediate::toString() const
{
    return "TRACEV " + getArgument()->toString();
}

string OTraceLRegister::toString() const
{
    return "TRACELR " + getArgument()->toString();
}

string OTrace2Register::toString() const
{
    return "TRACE2R " + getArgument()->toString();
}

string OTrace3::toString() const
{
    return "TRACE3";
}

string OTrace4::toString() const
{
    return "TRACE4";
}

string OTrace5Register::toString() const
{
    return "TRACE5";
}

string OTrace6Register::toString() const
{
    return "TRACE6 " + getArgument()->toString();
}

string OPrintfImmediate::toString() const
{
	return "PRINTFV " + getArgument()->toString();
}

string OSPrintfImmediate::toString() const
{
	return "SPRINTFV " + getArgument()->toString();
}

string OPrintfVargs::toString() const
{
	return "PRINTFVARG";
}

string OSPrintfVargs::toString() const
{
	return "SPRINTFVARG";
}

string OBreakpoint::toString() const
{
    return "BREAKPOINT " + getArgument()->toString();
}


string ORandRegister::toString() const
{
    return "RNDR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSRandRegister::toString() const
{
    return "SRNDR " + getArgument()->toString();
}

string OSRandImmediate::toString() const
{
    return "SRNDV " + getArgument()->toString();
}

string OSRandRand::toString() const
{
    return "SRNDRND " + getArgument()->toString();
}

string ORNGRand1::toString() const
{
    return "RNGRAND1";
}

string ORNGRand2::toString() const
{
    return "RNGRAND2 " + getArgument()->toString();
}

string ORNGRand3::toString() const
{
    return "RNGRAND3 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ORNGLRand1::toString() const
{
    return "RNGLRAND1";
}

string ORNGLRand2::toString() const
{
    return "RNGLRAND2 " + getArgument()->toString();
}

string ORNGLRand3::toString() const
{
    return "RNGLRAND3 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ORNGSeed::toString() const
{
    return "RNGSEED " + getArgument()->toString();
}

string ORNGRSeed::toString() const
{
    return "RNGRSEED";
}

string ORNGFree::toString() const
{
    return "RNGFREE";
}

string OCheckTrig::toString() const
{
    return "CHECKTRIG";
}

string OWarp::toString() const
{
    return "WARPR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPitWarp::toString() const
{
    return "PITWARPR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSqrtRegister::toString() const
{
    return "SQROOTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCreateItemRegister::toString() const
{
    return "CREATEITEMR " + getArgument()->toString();
}

string OCreateNPCRegister::toString() const
{
    return "CREATENPCR " + getArgument()->toString();
}

string OCreateLWpnRegister::toString() const
{
    return "CREATELWEAPONR " + getArgument()->toString();
}

string OCreateEWpnRegister::toString() const
{
    return "CREATEEWEAPONR " + getArgument()->toString();
}

string OLoadItemRegister::toString() const
{
    return "LOADITEMR " + getArgument()->toString();
}

string OLoadItemDataRegister::toString() const
{
    return "LOADITEMDATAR " + getArgument()->toString();
}

//New Types

string OLoadShopDataRegister::toString() const
{
    return "LOADSHOPR " + getArgument()->toString();
}
string OLoadInfoShopDataRegister::toString() const
{
    return "LOADINFOSHOPR " + getArgument()->toString();
}
string OLoadMessageDataRegister::toString() const
{
    return "LOADMESSAGEDATAR " + getArgument()->toString();
}

string OLoadDMapDataRegister::toString() const
{
    return "LOADDMAPDATAR " + getArgument()->toString();
}

string OLoadStack::toString() const
{
    return "LOADSTACK";
}

string OLoadDirectoryRegister::toString() const
{
    return "LOADDIRECTORYR " + getArgument()->toString();
}

string OLoadRNG::toString() const
{
    return "LOADRNG";
}

string OCreatePalData::toString() const
{
    return "CREATEPALDATA";
}

string OCreatePalDataClr::toString() const
{
	return "CREATEPALDATACLR " + getArgument()->toString();
}

string OMixColorArray::toString() const
{
	return "MIXCLR";
}
string OCreateRGBHex::toString() const
{
	return "CREATERGBHEX " + getArgument()->toString();
}
string OCreateRGB::toString() const
{
	return "CREATERGB";
}
string OConvertFromRGB::toString() const
{
	return "CONVERTFROMRGB";
}
string OConvertToRGB::toString() const
{
	return "CONVERTTORGB";
}
string OGetTilePixel::toString() const
{
	return "GETTILEPIXEL";
}
string OSetTilePixel::toString() const
{
	return "SETTILEPIXEL";
}
string OLoadLevelPalette::toString() const
{
    return "PALDATALOADLEVEL " + getArgument()->toString();
}

string OLoadSpritePalette::toString() const
{
    return "PALDATALOADSPRITE " + getArgument()->toString();
}

string OLoadMainPalette::toString() const
{
    return "PALDATALOADMAIN";
}

string OLoadCyclePalette::toString() const
{
	return "PALDATALOADCYCLE " + getArgument()->toString();
}

string OLoadBitmapPalette::toString() const
{
	return "PALDATALOADBITMAP " + getArgument()->toString();
}

string OWriteLevelPalette::toString() const
{
    return "PALDATAWRITELEVEL " + getArgument()->toString();
}

string OWriteLevelCSet::toString() const
{
    return "PALDATAWRITELEVELCS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWriteSpritePalette::toString() const
{
    return "PALDATAWRITESPRITE " + getArgument()->toString();
}

string OWriteSpriteCSet::toString() const
{
    return "PALDATAWRITESPRITECS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWriteMainPalette::toString() const
{
    return "PALDATAWRITEMAIN";
}

string OWriteMainCSet::toString() const
{
    return "PALDATAWRITEMAINCS " + getArgument()->toString();
}

string OWriteCyclePalette::toString() const
{
	return "PALDATAWRITECYCLE " + getArgument()->toString();
}

string OWriteCycleCSet::toString() const
{
	return "PALDATAWRITECYCLECS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPalDataColorValid::toString() const
{
	return "PALDATAVALIDCLR " + getArgument()->toString();
}

string OPalDataClearColor::toString() const
{
    return "PALDATACLEARCLR " + getArgument()->toString();
}

string OPalDataClearCSet::toString() const
{
	return "PALDATACLEARCSET " + getArgument()->toString();
}

string OPalDataMix::toString() const
{
    return "PALDATAMIX";
}

string OPalDataMixCSet::toString() const
{
    return "PALDATAMIXCS";
}

string OPalDataCopy::toString() const
{
	return "PALDATACOPY " + getArgument()->toString();
}

string OPalDataCopyCSet::toString() const
{
	return "PALDATACOPYCSET";
}

string OPalDataFree::toString() const
{
	return "PALDATAFREE";
}

string OPalDataOwn::toString() const
{
	return "PALDATAOWN";
}

string OLoadDropsetRegister	::toString() const
{
    return "LOADDROPSETR " + getArgument()->toString();
}

string OGetBottleShopName::toString() const
{
    return "BSHOPNAMEGET " + getArgument()->toString();
}

string OSetBottleShopName::toString() const
{
    return "BSHOPNAMESET " + getArgument()->toString();
}

string OGetBottleName::toString() const
{
    return "BOTTLENAMEGET " + getArgument()->toString();
}

string OSetBottleName::toString() const
{
    return "BOTTLENAMESET " + getArgument()->toString();
}

string OLoadBottleTypeRegister::toString() const
{
    return "LOADBOTTLETYPE " + getArgument()->toString();
}

string OLoadBShopRegister::toString() const
{
    return "LOADBSHOPDATA " + getArgument()->toString();
}
string OLoadGenericDataR::toString() const
{
    return "LOADGENERICDATA " + getArgument()->toString();
}

string ODMapDataGetNameRegister::toString() const
{
    return "DMAPDATAGETNAMER " + getArgument()->toString();
}

string ODMapDataSetNameRegister::toString() const
{
    return "DMAPDATASETNAMER " + getArgument()->toString();
}

string ODMapDataGetTitleRegister::toString() const
{
    return "DMAPDATAGETTITLER " + getArgument()->toString();
}

string ODMapDataSetTitleRegister::toString() const
{
    return "DMAPDATASETTITLER " + getArgument()->toString();
}

string ODMapDataGetIntroRegister::toString() const
{
    return "DMAPDATAGETINTROR " + getArgument()->toString();
}

string ODMapDataSetIntroRegister::toString() const
{
    return "DMAPDATANSETITROR " + getArgument()->toString();
}

string ODMapDataGetMusicRegister::toString() const
{
    return "DMAPDATAGETMUSICR " + getArgument()->toString();
}

string ODMapDataSetMusicRegister::toString() const
{
    return "DMAPDATASETMUSICR " + getArgument()->toString();
}

string OMessageDataSetStringRegister::toString() const
{
    return "MESSAGEDATASETSTRINGR " + getArgument()->toString();
}
string OMessageDataGetStringRegister::toString() const
{
    return "MESSAGEDATAGETSTRINGR " + getArgument()->toString();
}
string OLoadNPCDataRegister::toString() const
{
    return "LOADNPCDATAR " + getArgument()->toString();
}
string OLoadComboDataRegister::toString() const
{
    return "LOADCOMBODATAR " + getArgument()->toString();
}
string OLoadMapDataRegister::toString() const
{
    return "LOADMAPDATAR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OLoadSpriteDataRegister::toString() const
{
    return "LOADSPRITEDATAR " + getArgument()->toString();
}
string OLoadScreenDataRegister::toString() const
{
    return "LOADSCREENDATAR " + getArgument()->toString();
}
string OLoadBitmapDataRegister::toString() const
{
    return "LOADBITMAPDATAR " + getArgument()->toString();
}

string OLoadNPCRegister::toString() const
{
    return "LOADNPCR " + getArgument()->toString();
}

string OLoadFFC::toString() const
{
    return "LOAD_FFC " + getArgument()->toString();
}

string OLoadFFC2::toString() const
{
    return "LOAD_FFC_2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OLoadLWpnRegister::toString() const
{
    return "LOADLWEAPONR " + getArgument()->toString();
}

string OLoadEWpnRegister::toString() const
{
    return "LOADEWEAPONR " + getArgument()->toString();
}
string OAdjustVolumeRegister::toString() const
{
    return "ADJUSTVOLUMER " + getArgument()->toString();
}
string OAdjustSFXVolumeRegister::toString() const
{
    return "ADJUSTSFXVOLUMER " + getArgument()->toString();
}
string OAdjustSound::toString() const
{
	return "ADJUSTSFX";
}
string OPlaySoundEX::toString() const
{
	return "PLAYSOUNDEX";
}
string OGetSoundCompletion::toString() const
{
	return "GETSFXCOMPLETION " + getArgument()->toString();
}
string OPlaySoundRegister::toString() const
{
    return "PLAYSOUNDR " + getArgument()->toString();
}

string OPlayMIDIRegister::toString() const
{
    return "PLAYMIDIR " + getArgument()->toString();
}

string OPlayEnhancedMusic::toString() const
{
    return "PLAYENHMUSIC " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPlayEnhancedMusicEx::toString() const
{
    return "PLAYENHMUSICEX " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetEnhancedMusicPos::toString() const
{
    return "GETENHMUSICPOS " + getArgument()->toString();
}

string OSetEnhancedMusicPos::toString() const
{
    return "SETENHMUSICPOS " + getArgument()->toString();
}

string OSetEnhancedMusicSpeed::toString() const
{
    return "SETENHMUSICSPEED " + getArgument()->toString();
}

string OGetEnhancedMusicLength::toString() const
{
	return "GETENHMUSICLEN " + getArgument()->toString();
}

string OSetEnhancedMusicLoop::toString() const
{
	return "SETENHMUSICLOOP " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCrossfadeEnhancedMusic::toString() const
{
	return "ENHCROSSFADE";
}

string OGetDMapMusicFilename::toString() const
{
    return "GETMUSICFILE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetNPCDataInitDLabel::toString() const
{
    return "NPCGETINITDLABEL " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetDMapMusicTrack::toString() const
{
    return "GETMUSICTRACK " + getArgument()->toString();
}

// Audio->
string OEndSoundRegister::toString() const
{
    return "ENDSOUNDR " + getArgument()->toString();
}


string OContinueSFX::toString() const
{
    return "CONTINUESFX " + getArgument()->toString();
}

string OPauseSoundRegister::toString() const
{
    return "PAUSESOUNDR " + getArgument()->toString();
}

string OPauseMusic::toString() const
{
    return "PAUSEMUSIC";
}
string OResumeMusic::toString() const
{
    return "RESUMEMUSIC";
}

string OResumeSoundRegister::toString() const
{
    return "RESUMESOUNDR " + getArgument()->toString();
}

//END Audio

string OSetDMapEnhancedMusic::toString() const
{
    return "SETDMAPENHMUSIC";
}

string OGetSaveName::toString() const
{
    return "GETSAVENAME " + getArgument()->toString();
}

string OSetSaveName::toString() const
{
    return "SETSAVENAME " + getArgument()->toString();
}

string OGetDMapName::toString() const
{
    return "GETDMAPNAME " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetDMapTitle::toString() const
{
    return "GETDMAPTITLE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetDMapIntro::toString() const
{
    return "GETDMAPINTRO " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OSetDMapName::toString() const
{
    return "SETDMAPNAME " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSetDMapTitle::toString() const
{
    return "SETDMAPTITLE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSetDMapIntro::toString() const
{
    return "SETDMAPINTRO " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetItemName::toString() const
{
    return "ITEMNAME " + getArgument()->toString();
}

string OGetNPCName::toString() const
{
    return "NPCNAME " + getArgument()->toString();
}

string OGetMessage::toString() const
{
    return "GETMESSAGE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OSetMessage::toString() const
{
    return "SETMESSAGE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OClearSpritesRegister::toString() const
{
    return "CLEARSPRITESR " + getArgument()->toString();
}

string ORectangleRegister::toString() const
{
    return "RECT";
}

string OFrameRegister::toString() const
{
    return "FRAMER";
}

string OCircleRegister::toString() const
{
    return "CIRCLE";
}

string OArcRegister::toString() const
{
    return "ARC";
}

string OEllipseRegister::toString() const
{
    return "ELLIPSE";
}

string OLineRegister::toString() const
{
    return "LINE";
}

string OSplineRegister::toString() const
{
    return "SPLINE";
}

string OPutPixelRegister::toString() const
{
    return "PUTPIXEL";
}

string OPutPixelArrayRegister::toString() const
{
    return "PIXELARRAYR";
}

string OPutTileArrayRegister::toString() const
{
    return "TILEARRAYR";
}

string OPutLinesArrayRegister::toString() const
{
    return "LINESARRAY";
}

string ODrawCharRegister::toString() const
{
    return "DRAWCHAR";
}

string ODrawIntRegister::toString() const
{
    return "DRAWINT";
}

string ODrawTileRegister::toString() const
{
    return "DRAWTILE";
}

string ODrawTileCloakedRegister::toString() const
{
    return "DRAWTILECLOAKEDR";
}

string ODrawComboRegister::toString() const
{
    return "DRAWCOMBO";
}

string ODrawComboCloakedRegister::toString() const
{
    return "DRAWCOMBOCLOAKEDR";
}

string OFastTileRegister::toString() const
{
    return "FASTTILE";
}

string OFastComboRegister::toString() const
{
    return "FASTCOMBO";
}

string OFastComboArrayRegister::toString() const
{
    return "COMBOARRAYR";
}

string ODrawStringRegister::toString() const
{
    return "DRAWSTRING";
}

string ODrawString2Register::toString() const
{
    return "DRAWSTRINGR2";
}

string ODrawBitmapRegister::toString() const
{
    return "DRAWBITMAP";
}

string ODrawBitmapExRegister::toString() const
{
    return "BITMAPEXR";
}

string OSetRenderTargetRegister::toString() const
{
    return "SETRENDERTARGET";
}

string OSetDepthBufferRegister::toString() const
{
    return "SETDEPTHB";
}

string OGetDepthBufferRegister::toString() const
{
    return "GETDEPTHB";
}

string OSetColorBufferRegister::toString() const
{
    return "SETCOLORB";
}

string OGetColorBufferRegister::toString() const
{
    return "GETCOLORB";
}

string OQuadRegister::toString() const
{
    return "QUAD";
}

string OTriangleRegister::toString() const
{
    return "TRIANGLE";
}


string OQuad3DRegister::toString() const
{
    return "QUAD3D";
}

string OTriangle3DRegister::toString() const
{
    return "TRIANGLE3D";
}

string ODrawLayerRegister::toString() const
{
    return "DRAWLAYER";
}

string ODrawScreenRegister::toString() const
{
    return "DRAWSCREEN";
}

string OMessageRegister::toString() const
{
    return "MSGSTRR " + getArgument()->toString();
}

string OIsSolid::toString() const
{
    return "ISSOLID " + getArgument()->toString();
}

string OIsSolidMapdata::toString() const
{
    return "MAPDATAISSOLID " + getArgument()->toString();
}

string OIsSolidMapdataLayer::toString() const
{
    return "MAPDATAISSOLIDLYR " + getArgument()->toString();
}

string OIsSolidLayer::toString() const
{
    return "ISSOLIDLAYER " + getArgument()->toString();
}

string OLoadTmpScr::toString() const
{
    return "LOADTMPSCR " + getArgument()->toString();
}

string OLoadTmpScr2::toString() const
{
    return "LOADTMPSCR2 " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OLoadTmpScrComboPos::toString() const
{
    return "REGION_LOAD_TMPSCR_FOR_LAYER_COMBO_POS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OLoadScrollScr::toString() const
{
    return "LOADSCROLLSCR " + getArgument()->toString();
}

string OLoadScrollScr2::toString() const
{
    return "LOADSCROLLSCR2 " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OSetSideWarpRegister::toString() const
{
    return "SETSIDEWARP";
}

string OGetSideWarpDMap::toString() const
{
    return "GETSIDEWARPDMAP " + getArgument()->toString();
}

string OGetSideWarpScreen::toString() const
{
    return "GETSIDEWARPSCR " + getArgument()->toString();
}

string OGetSideWarpType::toString() const
{
    return "GETSIDEWARPTYPE " + getArgument()->toString();
}

string OGetTileWarpDMap::toString() const
{
    return "GETTILEWARPDMAP " + getArgument()->toString();
}

string OGetTileWarpScreen::toString() const
{
    return "GETTILEWARPSCR " + getArgument()->toString();
}

string OGetTileWarpType::toString() const
{
    return "GETTILEWARPTYPE " + getArgument()->toString();
}

string OSetTileWarpRegister::toString() const
{
    return "SETTILEWARP";
}

string OLayerScreenRegister::toString() const
{
    return "LAYERSCREEN " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OLayerMapRegister::toString() const
{
    return "LAYERMAP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OTriggerSecrets::toString() const
{
    return "SECRETS";
}

string OTriggerSecretsFor::toString() const
{
    return "REGION_TRIGGER_SECRETS " + getArgument()->toString();
}

string OIsValidArray::toString() const
{
    return "ISVALIDARRAY " + getArgument()->toString();
}

string OIsValidItem::toString() const
{
    return "ISVALIDITEM " + getArgument()->toString();
}

string OIsValidNPC::toString() const
{
    return "ISVALIDNPC " + getArgument()->toString();
}

string OCopyTileRegister::toString() const
{
    return "COPYTILERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Ostrcpy::toString() const
{
    return "STRINGCOPY " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OOverlayTileRegister::toString() const
{
    return "OVERLAYTILERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSwapTileRegister::toString() const
{
    return "SWAPTILERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OClearTileRegister::toString() const
{
    return "CLEARTILER " + getArgument()->toString();
}

string OIsValidLWpn::toString() const
{
    return "ISVALIDLWPN " + getArgument()->toString();
}

string OIsValidEWpn::toString() const
{
    return "ISVALIDEWPN " + getArgument()->toString();
}

string OMakeAngularLwpn::toString() const
{
    return "LWPNMAKEANGULAR " + getArgument()->toString();
}

string OMakeAngularEwpn::toString() const
{
    return "EWPNMAKEANGULAR " + getArgument()->toString();
}

string OMakeDirectionalLwpn::toString() const
{
    return "LWPNMAKEDIRECTIONAL " + getArgument()->toString();
}

string OMakeDirectionalEwpn::toString() const
{
    return "EWPNMAKEDIRECTIONAL " + getArgument()->toString();
}

string OUseSpriteLWpn::toString() const
{
    return "LWPNUSESPRITER " + getArgument()->toString();
}

string OUseSpriteEWpn::toString() const
{
    return "EWPNUSESPRITER " + getArgument()->toString();
}

string OAllocateMemRegister::toString() const
{
    return "ALLOCATEMEMR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAllocateMemImmediate::toString() const
{
    return "ALLOCATEMEMV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAllocateGlobalMemImmediate::toString() const
{
    return "ALLOCATEGMEMV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAllocateGlobalMemRegister::toString() const
{
    return "ALLOCATEGMEMR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODeallocateMemRegister::toString() const
{
    return "DEALLOCATEMEMR " + getArgument()->toString();
}

string ODeallocateMemImmediate::toString() const
{
    return "DEALLOCATEMEMV " + getArgument()->toString();
}

string OResizeArrayRegister::toString() const
{
    return "RESIZEARRAYR " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OOwnArrayRegister::toString() const
{
    return "OWNARRAYR " + getArgument()->toString();
}
string ODestroyArrayRegister::toString() const
{
    return "DESTROYARRAYR " + getArgument()->toString();
}

string OSave::toString() const
{
    return "SAVE";
}

string OGetScreenFlags::toString() const
{
    return "GETSCREENFLAGS " + getArgument()->toString();
}

string OGetScreenEFlags::toString() const
{
    return "GETSCREENEFLAGS " + getArgument()->toString();
}

string OEnd::toString() const
{
    return "GAMEEND";
}

string OGameReload::toString() const
{
    return "GAMERELOAD";
}

string OGameContinue::toString() const
{
    return "GAMECONTINUE";
}

string OGameSaveQuit::toString() const
{
    return "GAMESAVEQUIT";
}

string OGameSaveContinue::toString() const
{
    return "GAMESAVECONTINUE";
}

string OShowF6Screen::toString() const
{
    return "SHOWF6SCREEN";
}

string OComboTile::toString() const
{
    return "COMBOTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OBreakShield::toString() const
{
    return "BREAKSHIELD " + getArgument()->toString();
}

string OShowSaveScreen::toString() const
{
    return "SAVESCREEN " + getArgument()->toString();
}

string OShowSaveQuitScreen::toString() const
{
    return "SAVEQUITSCREEN";
}

string OSelectAWeaponRegister::toString() const
{
    return "SELECTAWPNR " + getArgument()->toString();
}

string OSelectBWeaponRegister::toString() const
{
    return "SELECTBWPNR " + getArgument()->toString();
}

string OSelectXWeaponRegister::toString() const
{
    return "SELECTXWPNR " + getArgument()->toString();
}

string OSelectYWeaponRegister::toString() const
{
    return "SELECTYWPNR " + getArgument()->toString();
}

string OGetFFCScript::toString() const
{
    return "GETFFCSCRIPT " + getArgument()->toString();
}

string OGetComboScript::toString() const
{
    return "GETCOMBOSCRIPT " + getArgument()->toString();
}

//2.54

string OGreyscaleOn::toString() const
{
    return "GREYSCALEON";
}

string OGreyscaleOff::toString() const
{
    return "GREYSCALEOFF";
}

string OZapIn::toString() const
{
    return "ZAPIN";
}

string OZapOut::toString() const
{
    return "ZAPOUT";
}

//These need to be unary opcodes that accept bool linkvisible. 
string OWavyIn::toString() const
{
    return "WAVYIN";
}

string OWavyOut::toString() const
{
    return "WAVYOUT";
}

string OOpenWipe::toString() const
{
    return "OPENWIPE";
}

string OCloseWipe::toString() const
{
    return "CLOSEWIPE";
}

string OOpenWipeShape::toString() const
{
    return "OPENWIPESHAPE " + getArgument()->toString();
}

string OCloseWipeShape::toString() const
{
    return "CLOSEWIPESHAPE " + getArgument()->toString();
}

//Game->GetItemScript(int32_t ptr[])
string OGetItemScript::toString() const
{
    return "GETITEMSCRIPT " + getArgument()->toString();
}


string OGetLWeaponPointer::toString() const
{
    return "LWPNARRPTR " + getArgument()->toString();
}

string OSetLWeaponPointer::toString() const
{
    return "LWPNARRPTR2 " + getArgument()->toString();
}

string OGetEWeaponPointer::toString() const
{
    return "EWPNARRPTR " + getArgument()->toString();
}

string OSetEWeaponPointer::toString() const
{
    return "EWPNARRPTR2 " + getArgument()->toString();
}

string OGetItemPointer::toString() const
{
    return "IDATAARRPTR " + getArgument()->toString();
}

string OSetItemPointer::toString() const
{
    return "ITEMARRPTR2 " + getArgument()->toString();
}

string OGetItemDataPointer::toString() const
{
    return "IDATAARRPTR " + getArgument()->toString();
}

string OSetItemDataPointer::toString() const
{
    return "IDATAARRPTR2 " + getArgument()->toString();
}

string OGetFFCPointer::toString() const
{
    return "FFCARRPTR " + getArgument()->toString();
}

string OSetFFCPointer::toString() const
{
    return "FFCARRPTR2 " + getArgument()->toString();
}

string OGetBoolPointer::toString() const
{
    return "BOOLARRPTR " + getArgument()->toString();
}

string OSetBoolPointer::toString() const
{
    return "BOOLARRPTR2 " + getArgument()->toString();
}

string OGetNPCPointer::toString() const
{
    return "NPCARRPTR " + getArgument()->toString();
}

string OSetNPCPointer::toString() const
{
    return "NPCARRPTR2 " + getArgument()->toString();
}


string OGetScreenDoor::toString() const
{
    return "GETSCREENDOOR " + getArgument()->toString();
}

string OGetScreenEnemy::toString() const
{
    return "GETSCREENENEMY " + getArgument()->toString();
}


string OGetScreenLayerOpacity::toString() const
{
    return "GETSCREENLAYOP " + getArgument()->toString();
}

string OGetScreenSecretCombo::toString() const
{
    return "GETSCREENSECCMB " + getArgument()->toString();
}

string OGetScreenSecretCSet::toString() const
{
    return "GETSCREENSECCST " + getArgument()->toString();
}

string OGetScreenSecretFlag::toString() const
{
    return "GETSCREENSECFLG " + getArgument()->toString();
}

string OGetScreenLayerMap::toString() const
{
    return "GETSCREENLAYMAP " + getArgument()->toString();
}

string OGetScreenLayerScreen::toString() const
{
    return "GETSCREENLAYSCR " + getArgument()->toString();
}

string OGetScreenPath::toString() const
{
    return "GETSCREENPATH " + getArgument()->toString();
}

string OGetScreenWarpReturnX::toString() const
{
    return "GETSCREENWARPRX " + getArgument()->toString();
}

string OGetScreenWarpReturnY::toString() const
{
    return "GETSCREENWARPRY " + getArgument()->toString();
}

string OTriggerSecretRegister::toString() const
{
    return "TRIGGERSECRETR " + getArgument()->toString();
}

string OBMPPolygonRegister::toString() const
{
    return "BMPPOLYGONR";
}

string OPolygonRegister::toString() const
{
    return "POLYGONR";
}


//NPCData

string ONDataBaseTile::toString() const
{
    return "GETNPCDATATILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataEHeight::toString() const
{
    return "GETNPCDATAEHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string ONDataFlags::toString() const
{
    return "GETNPCDATAFLAGS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataFlags2::toString() const
{
    return "GETNPCDATAFLAGS2 " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataWidth::toString() const
{
    return "GETNPCDATAWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHeight::toString() const
{
    return "GETNPCDATAHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataTile::toString() const
{
    return "GETNPCDATASTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSWidth::toString() const
{
    return "GETNPCDATASWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSHeight::toString() const
{
    return "GETNPCDATASHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataETile::toString() const
{
    return "GETNPCDATAETILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataEWidth::toString() const
{
    return "GETNPCDATAEWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHP::toString() const
{
    return "GETNPCDATAHP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataFamily::toString() const
{
    return "GETNPCDATAFAMILY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataCSet::toString() const
{
    return "GETNPCDATACSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataAnim::toString() const
{
    return "GETNPCDATAANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataEAnim::toString() const
{
    return "GETNPCDATAEANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataFramerate::toString() const
{
    return "GETNPCDATAFRAMERATE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataEFramerate::toString() const
{
    return "GETNPCDATAEFRAMERATE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataTouchDamage::toString() const
{
    return "GETNPCDATATOUCHDMG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataWeaponDamage::toString() const
{
    return "GETNPCDATAWPNDAMAGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataWeapon::toString() const
{
    return "GETNPCDATAWEAPON " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataRandom::toString() const
{
    return "GETNPCDATARANDOM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHalt::toString() const
{
    return "GETNPCDATAHALT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataStep::toString() const
{
    return "GETNPCDATASTEP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHoming::toString() const
{
    return "GETNPCDATAHOMING " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHunger::toString() const
{
    return "GETNPCDATAHUNGER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataropset::toString() const
{
    return "GETNPCDATADROPSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataBGSound::toString() const
{
    return "GETNPCDATABGSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHitSound::toString() const
{
    return "GETNPCDATAHITSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataDeathSound::toString() const
{
    return "GETNPCDATADEATHSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataXofs::toString() const
{
    return "GETNPCDATAXOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataYofs::toString() const
{
    return "GETNPCDATAYOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataZofs::toString() const
{
    return "GETNPCDATAZOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHitXOfs::toString() const
{
    return "GETNPCDATAHXOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHYOfs::toString() const
{
    return "GETNPCDATAHYOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHitWidth::toString() const
{
    return "GETNPCDATAHITWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHitHeight::toString() const
{
    return "GETNPCDATAHITHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHitZ::toString() const
{
    return "GETNPCDATAHITZ " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataTileWidth::toString() const
{
    return "GETNPCDATATILEWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataTileHeight::toString() const
{
    return "GETNPCDATATILEHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataWeapSprite::toString() const
{
    return "GETNPCDATAWPNSPRITE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

//two inputs, one return
string ONDataScriptDef::toString() const
{
    return "GETNPCDATASCRIPTDEF " + getArgument()->toString();
}
//two inputs, one return
string ONDataDefense::toString() const
{
    return "GETNPCDATADEFENSE " + getArgument()->toString();
}
//two inputs, one return
string ONDataSizeFlag::toString() const
{
    return "GETNPCDATASIZEFLAG " + getArgument()->toString();
}
//two inputs, one return
string ONDatattributes::toString() const
{
    return "GETNPCDATAATTRIBUTE " + getArgument()->toString();
}

string ONDataSetBaseTile::toString() const
{
    return "SETNPCDATATILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetEHeight::toString() const
{
    return "SETNPCDATAEHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string ONDataSetFlags::toString() const
{
    return "SETNPCDATAFLAGS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetFlags2::toString() const
{
    return "SETNPCDATAFLAGS2 " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetWidth::toString() const
{
    return "SETNPCDATAWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHeight::toString() const
{
    return "SETNPCDATAHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetTile::toString() const
{
    return "SETNPCDATASTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetSWidth::toString() const
{
    return "SETNPCDATASWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetSHeight::toString() const
{
    return "SETNPCDATASHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetETile::toString() const
{
    return "SETNPCDATAETILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetEWidth::toString() const
{
    return "SETNPCDATAEWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHP::toString() const
{
    return "SETNPCDATAHP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetFamily::toString() const
{
    return "SETNPCDATAFAMILY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetCSet::toString() const
{
    return "SETNPCDATACSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetAnim::toString() const
{
    return "SETNPCDATAANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetEAnim::toString() const
{
    return "SETNPCDATAEANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetFramerate::toString() const
{
    return "SETNPCDATAFRAMERATE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetEFramerate::toString() const
{
    return "SETNPCDATAEFRAMERATE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetTouchDamage::toString() const
{
    return "SETNPCDATATOUCHDMG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetWeaponDamage::toString() const
{
    return "SETNPCDATAWPNDAMAGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetWeapon::toString() const
{
    return "SETNPCDATAWEAPON " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetRandom::toString() const
{
    return "SETNPCDATARANDOM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHalt::toString() const
{
    return "SETNPCDATAHALT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetStep::toString() const
{
    return "SETNPCDATASTEP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHoming::toString() const
{
    return "SETNPCDATAHOMING " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHunger::toString() const
{
    return "SETNPCDATAHUNGER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetropset::toString() const
{
    return "SETNPCDATADROPSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetBGSound::toString() const
{
    return "SETNPCDATABGSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHitSound::toString() const
{
    return "SETNPCDATAHITSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetDeathSound::toString() const
{
    return "SETNPCDATADEATHSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetXofs::toString() const
{
    return "SETNPCDATAXOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetYofs::toString() const
{
    return "SETNPCDATAYOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetZofs::toString() const
{
    return "SETNPCDATAZOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHitXOfs::toString() const
{
    return "SETNPCDATAHXOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHYOfs::toString() const
{
    return "SETNPCDATAHYOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHitWidth::toString() const
{
    return "SETNPCDATAHITWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHitHeight::toString() const
{
    return "SETNPCDATAHITHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHitZ::toString() const
{
    return "SETNPCDATAHITZ " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetTileWidth::toString() const
{
    return "SETNPCDATATILEWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetTileHeight::toString() const
{
    return "SETNPCDATATILEHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetWeapSprite::toString() const
{
    return "SETNPCDATAWPNSPRITE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

//ComboData

string OCDataBlockEnemy::toString() const
{
    return "GCDBLOCKENEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataBlockHole::toString() const
{
    return "GCDBLOCKHOLE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataBlockTrig::toString() const
{
    return "GCDBLOCKTRIG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataConveyX::toString() const
{
    return "GCDCONVEYSPDX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataConveyY::toString() const
{
    return "GCDCONVEYSPDY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataCreateNPC::toString() const
{
    return "GCDCREATEENEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataCreateEnemW::toString() const
{
    return "GCDCREATEENEMWH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataCreateEnemC::toString() const
{
    return "GCDCREATEENEMCH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataDirch::toString() const
{
    return "GCDDIRCHTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataDistTiles::toString() const
{
    return "GCDDISTCHTILES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataDiveItem::toString() const
{
    return "GCDDIVEITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataDock::toString() const
{
    return "GCDDOCK " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFairy::toString() const
{
    return "GCDFAIRY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataAttrib::toString() const
{
    return "GCDFFCOMBOATTRIB " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataDecoTile::toString() const
{
    return "GCDFOOTDECOTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OCDataLadderPass::toString() const
{
    return "GCDLADDERPASS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataLadderPass::toString() const
{
    return "SCDLADDERPASS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}


string OCDataDecoType::toString() const
{
    return "GCDFOOTDECOTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataHookshotGrab::toString() const
{
    return "GCDHOOKSHOTGRAB " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataLockBlock::toString() const
{
    return "GCDLOCKBLOCKTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataLockBlockChange::toString() const
{
    return "GCDLOCKBLOCKCHANGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataMagicMirror::toString() const
{
    return "GCDMAGICMIRRORTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataModHP::toString() const
{
    return "GCDMODIFYHPAMOUNT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataModHPDelay::toString() const
{
    return "GCDMODIFYHPDELAY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataModHpType::toString() const
{
    return "GCDMODIFYHPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataModMP::toString() const
{
    return "GCDMODIFYMPAMOUNT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataMpdMPDelay::toString() const
{
    return "GCDMODIFYMPDELAY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataModMPType::toString() const
{
    return "GCDMODIFYMPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataNoPush::toString() const
{
    return "GCDNOPUSHBLOCKS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataOverhead::toString() const
{
    return "GCDOVERHEAD " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataEnemyLoc::toString() const
{
    return "GCDPLACEENEMY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataPushDir::toString() const
{
    return "GCDPUSHDIR " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataPushWeight::toString() const
{
    return "GCDPUSHWEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataPushWait::toString() const
{
    return "GCDPUSHWAIT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataPushed::toString() const
{
    return "GCDPUSHED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataRaft::toString() const
{
    return "GCDRAFT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataResetRoom::toString() const
{
    return "GCDRESETROOM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSavePoint::toString() const
{
    return "GCDSAVEPOINT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFreeezeScreen::toString() const
{
    return "GCDSCREENFREEZE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSecretCombo::toString() const
{
    return "GCDSECRETCOMBO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSingular::toString() const
{
    return "GCDSINGULAR " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSlowMove::toString() const
{
    return "GCDSLOWMOVE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStatue::toString() const
{
    return "GCDSTATUE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStepType::toString() const
{
    return "GCDSTEPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSteoChange::toString() const
{
    return "GCDSTEPCHANGETO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStrikeRem::toString() const
{
    return "GCDSTRIKEREMNANTS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStrikeRemType::toString() const
{
    return "GCDSTRIKEREMNANTSTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStrikeChange::toString() const
{
    return "GCDSTRIKECHANGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStrikeChangeItem::toString() const
{
    return "GCDSTRIKECHANGEITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTouchItem::toString() const
{
    return "GCDTOUCHITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTouchStairs::toString() const
{
    return "GCDTOUCHSTAIRS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTriggerType::toString() const
{
    return "GCDTRIGGERTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTriggerSens::toString() const
{
    return "GCDTRIGGERSENS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWarpType::toString() const
{
    return "GCDWARPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWarpSens::toString() const
{
    return "GCDWARPSENS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWarpDirect::toString() const
{
    return "GCDWARPDIRECT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWarpLoc::toString() const
{
    return "GCDWARPLOCATION " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWater::toString() const
{
    return "GCDWATER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OCDataWinGame::toString() const
{
    return "GCDWINGAME " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWhistle::toString() const
{
    return "GCDWHISTLE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWeapBlockLevel::toString() const
{
    return "GCDBLOCKWEAPLVL " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTile::toString() const
{
    return "GCDTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFlip::toString() const
{
    return "GCDFLIP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWalkability::toString() const
{
    return "GCDWALK " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataType::toString() const
{
    return "GCDTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataCSets::toString() const
{
    return "GCDCSETS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFoo::toString() const
{
    return "GCDFOO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFrames::toString() const
{
    return "GCDFRAMES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSpeed::toString() const
{
    return "GCDSPEED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataNext::toString() const
{
    return "GCDNEXTCOMBO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataNextCSet::toString() const
{
    return "GCDNEXTCSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFlag::toString() const
{
    return "GCDFLAG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSkipAnim::toString() const
{
    return "GCDSKIPANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTimer::toString() const
{
    return "GCDNEXTTIMER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataAnimY::toString() const
{
    return "GCDSKIPANIMY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataAnimFlags::toString() const
{
    return "GCDANIMFLAGS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OCDataBlockWeapon::toString() const
{
    return "GCDBLOCKWEAPON " + getArgument()->toString();
}
string OCDataExpansion::toString() const
{
    return "GCDEXPANSION " + getArgument()->toString();
}
string OCDataStrikeWeapon::toString() const
{
    return "GCDSTRIKEWEAPONS " + getArgument()->toString();
}

string OCSetDataBlockEnemy::toString() const
{
    return "SCDBLOCKENEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataBlockHole::toString() const
{
    return "SCDBLOCKHOLE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataBlockTrig::toString() const
{
    return "SCDBLOCKTRIG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataConveyX::toString() const
{
    return "SCDCONVEYSPDX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataConveyY::toString() const
{
    return "SCDCONVEYSPDY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataCreateNPC::toString() const
{
    return "SCDCREATEENEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataCreateEnemW::toString() const
{
    return "SCDCREATEENEMWH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataCreateEnemC::toString() const
{
    return "SCDCREATEENEMCH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDirch::toString() const
{
    return "SCDDIRCHTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDistTiles::toString() const
{
    return "SCDDISTCHTILES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDiveItem::toString() const
{
    return "SCDDIVEITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDock::toString() const
{
    return "SCDDOCK " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFairy::toString() const
{
    return "SCDFAIRY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataAttrib::toString() const
{
    return "SCDFFCOMBOATTRIB " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDecoTile::toString() const
{
    return "SCDFOOTDECOTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDecoType::toString() const
{
    return "SCDFOOTDECOTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataHookshotGrab::toString() const
{
    return "SCDHOOKSHOTGRAB " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataLockBlock::toString() const
{
    return "SCDLOCKBLOCKTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataLockBlockChange::toString() const
{
    return "SCDLOCKBLOCKCHANGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataMagicMirror::toString() const
{
    return "SCDMAGICMIRRORTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataModHP::toString() const
{
    return "SCDMODIFYHPAMOUNT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataModHPDelay::toString() const
{
    return "SCDMODIFYHPDELAY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataModHpType::toString() const
{
    return "SCDMODIFYHPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataModMP::toString() const
{
    return "SCDMODIFYMPAMOUNT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataMpdMPDelay::toString() const
{
    return "SCDMODIFYMPDELAY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataModMPType::toString() const
{
    return "SCDMODIFYMPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataNoPush::toString() const
{
    return "SCDNOPUSHBLOCKS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataOverhead::toString() const
{
    return "SCDOVERHEAD " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataEnemyLoc::toString() const
{
    return "SCDPLACEENEMY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataPushDir::toString() const
{
    return "SCDPUSHDIR " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataPushWeight::toString() const
{
    return "SCDPUSHWEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataPushWait::toString() const
{
    return "SCDPUSHWAIT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataPushed::toString() const
{
    return "SCDPUSHED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataRaft::toString() const
{
    return "SCDRAFT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataResetRoom::toString() const
{
    return "SCDRESETROOM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSavePoint::toString() const
{
    return "SCDSAVEPOINT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFreeezeScreen::toString() const
{
    return "SCDSCREENFREEZE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSecretCombo::toString() const
{
    return "SCDSECRETCOMBO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSingular::toString() const
{
    return "SCDSINGULAR " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSlowMove::toString() const
{
    return "SCDSLOWMOVE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStatue::toString() const
{
    return "SCDSTATUE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStepType::toString() const
{
    return "SCDSTEPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSteoChange::toString() const
{
    return "SCDSTEPCHANGETO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStrikeRem::toString() const
{
    return "SCDSTRIKEREMNANTS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStrikeRemType::toString() const
{
    return "SCDSTRIKEREMNANTSTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStrikeChange::toString() const
{
    return "SCDSTRIKECHANGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStrikeChangeItem::toString() const
{
    return "SCDSTRIKECHANGEITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTouchItem::toString() const
{
    return "SCDTOUCHITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTouchStairs::toString() const
{
    return "SCDTOUCHSTAIRS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTriggerType::toString() const
{
    return "SCDTRIGGERTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTriggerSens::toString() const
{
    return "SCDTRIGGERSENS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWarpType::toString() const
{
    return "SCDWARPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWarpSens::toString() const
{
    return "SCDWARPSENS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWarpDirect::toString() const
{
    return "SCDWARPDIRECT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWarpLoc::toString() const
{
    return "SCDWARPLOCATION " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWater::toString() const
{
    return "SCDWATER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWhistle::toString() const
{
    return "SCDWHISTLE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWeapBlockLevel::toString() const
{
    return "SCDBLOCKWEAPLVL " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTile::toString() const
{
    return "SCDTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFlip::toString() const
{
    return "SCDFLIP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWalkability::toString() const
{
    return "SCDWALK " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataType::toString() const
{
    return "SCDTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataCSets::toString() const
{
    return "SCDCSETS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFoo::toString() const
{
    return "SCDFOO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFrames::toString() const
{
    return "SCDFRAMES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSpeed::toString() const
{
    return "SCDSPEED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataNext::toString() const
{
    return "SCDNEXTCOMBO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataNextCSet::toString() const
{
    return "SCDNEXTCSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFlag::toString() const
{
    return "SCDFLAG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSkipAnim::toString() const
{
    return "SCDSKIPANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTimer::toString() const
{
    return "SCDNEXTTIMER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataAnimY::toString() const
{
    return "SCDSKIPANIMY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataAnimFlags::toString() const
{
    return "SCDANIMFLAGS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OCSetDataWinGame::toString() const
{
    return "SCDWINGAME " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

//SpriteData
string OSDataTile::toString() const
{
    return "GETSPRITEDATATILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSDataMisc::toString() const
{
    return "GETSPRITEDATAMISC " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSDataCSets::toString() const
{
    return "GETSPRITEDATACGETS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSDataFrames::toString() const
{
    return "GETSPRITEDATAFRAMES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSDataSpeed::toString() const
{
    return "GETSPRITEDATASPEED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSDataType::toString() const
{
    return "GETSPRITEDATATYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OSSetDataTile::toString() const
{
    return "SETSPRITEDATATILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetDataMisc::toString() const
{
    return "SETSPRITEDATAMISC " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetDataCSets::toString() const
{
    return "SETSPRITEDATACGETS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetDataFrames::toString() const
{
    return "SETSPRITEDATAFRAMES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetDataSpeed::toString() const
{
    return "SETSPRITEDATASPEED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetDataType::toString() const
{
    return "SETSPRITEDATATYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

//Continue Screen Settings
string OSSetContinueScreen::toString() const
{
    return "SETCONTINUESCREEN " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetContinueString::toString() const
{
    return "SETCONTINUESTRING " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

//Visual effects with one bool arg.

string OWavyR::toString() const
{
    return "FXWAVYR " + getArgument()->toString();
}

string OZapR::toString() const
{
    return "FXZAPR " + getArgument()->toString();
}

string OGreyscaleR::toString() const
{
    return "GREYSCALER " + getArgument()->toString();
}

string OMonochromeR::toString() const
{
    return "MONOCHROMER " + getArgument()->toString();
}

string OClearTint::toString() const
{
    return "CLEARTINT";
}

string OTintR::toString() const
{
    return "TINT";
}

string OMonoHueR::toString() const
{
    return "MONOHUE";
}

//Bitmap commands

string OBMPRectangleRegister::toString() const
{
    return "BMPRECTR";
}

string OBMPFrameRegister::toString() const
{
    return "BMPFRAMER";
}

string OBMPCircleRegister::toString() const
{
    return "BMPCIRCLER";
}

string OBMPArcRegister::toString() const
{
    return "BMPARCR";
}

string OBMPEllipseRegister::toString() const
{
    return "BMPELLIPSER";
}

string OBMPLineRegister::toString() const
{
    return "BMPLINER";
}

string OBMPSplineRegister::toString() const
{
    return "BMPSPLINER";
}

string OBMPPutPixelRegister::toString() const
{
    return "BMPPUTPIXELR";
}

string OBMPDrawCharRegister::toString() const
{
    return "BMPDRAWCHARR";
}

string OBMPDrawIntRegister::toString() const
{
    return "BMPDRAWINTR";
}

string OBMPDrawTileRegister::toString() const
{
    return "BMPDRAWTILER";
}

string OBMPDrawTileCloakedRegister::toString() const
{
    return "BMPDRAWTILECLOAKEDR";
}

string OBMPDrawComboRegister::toString() const
{
    return "BMPDRAWCOMBOR";
}

string OBMPDrawComboCloakedRegister::toString() const
{
    return "BMPDRAWCOMBOCLOAKEDR";
}

string OBMPFastTileRegister::toString() const
{
    return "BMPFASTTILER";
}

string OBMPFastComboRegister::toString() const
{
    return "BMPFASTCOMBOR";
}

string OBMPDrawStringRegister::toString() const
{
    return "BMPDRAWSTRINGR";
}

string OBMPDrawString2Register::toString() const
{
    return "BMPDRAWSTRINGR2";
}

string OBMPDrawBitmapExRegister::toString() const
{
    return "BMPBLIT";
}
string OBMPBlitTO::toString() const
{
    return "BMPBLITTO";
}

string OBMPGetPixel::toString() const
{
    return "BITMAPGETPIXEL";
}
string OBMPMode7::toString() const
{
    return "BMPMODE7";
}

string OBMPQuadRegister::toString() const
{
    return "BMPQUADR";
}

string OBMPTriangleRegister::toString() const
{
    return "BMPTRIANGLER";
}


string OBMPQuad3DRegister::toString() const
{
    return "BMPQUAD3DR";
}

string OBMPTriangle3DRegister::toString() const
{
    return "BMPTRIANGLE3DR";
}

string OBMPDrawLayerRegister::toString() const
{
    return "BMPDRAWLAYERR";
}

string OBMPDrawScreenRegister::toString() const
{
    return "BMPDRAWSCREENR";
}

string OHeroWarpExRegister::toString() const
{
    return "LINKWARPEXR " + getArgument()->toString();
}

string OHeroExplodeRegister::toString() const
{
    return "LINKEXPLODER " + getArgument()->toString();
}

string OSwitchNPC::toString() const
{
	return "SWITCHNPC " + getArgument()->toString();
}
string OSwitchItem::toString() const
{
	return "SWITCHITM " + getArgument()->toString();
}
string OSwitchLW::toString() const
{
	return "SWITCHLW " + getArgument()->toString();
}
string OSwitchEW::toString() const
{
	return "SWITCHEW " + getArgument()->toString();
}
string OSwitchCombo::toString() const
{
	return "SWITCHCMB " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OKillPlayer::toString() const
{
	return "KILLPLAYER " + getArgument()->toString();
}

string OScreenDoSpawn::toString() const
{
	return "SCREENDOSPAWN";
}

string OScreenTriggerCombo::toString() const
{
	return "SCRTRIGGERCOMBO " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ONPCMovePaused::toString() const
{
	return "NPCMOVEPAUSED";
}
string ONPCMove::toString() const
{
	return "NPCMOVE";
}
string ONPCMoveAngle::toString() const
{
	return "NPCMOVEANGLE";
}
string ONPCMoveXY::toString() const
{
	return "NPCMOVEXY";
}
string ONPCCanMoveDir::toString() const
{
	return "NPCCANMOVEDIR";
}
string ONPCCanMoveAngle::toString() const
{
	return "NPCCANMOVEANGLE";
}
string ONPCCanMoveXY::toString() const
{
	return "NPCCANMOVEXY";
}
string ONPCCanPlace::toString() const
{
	return "NPCCANPLACE";
}
string ONPCIsFlickerFrame::toString() const
{
	return "NPCISFLICKERFRAME";
}


string OGetSystemRTCRegister::toString() const
{
    return "GETRTCTIMER " + getArgument()->toString();
}


string ONPCExplodeRegister::toString() const
{
    return "NPCEXPLODER " + getArgument()->toString();
}
string OLWeaponExplodeRegister::toString() const
{
    return "LWEAPONEXPLODER " + getArgument()->toString();
}
string OEWeaponExplodeRegister::toString() const
{
    return "EWEAPONEXPLODER " + getArgument()->toString();
}
string OItemExplodeRegister::toString() const
{
    return "ITEMEXPLODER " + getArgument()->toString();
}

string ORunItemScript::toString() const
{
    return "RUNITEMSCRIPT " + getArgument()->toString();
}

//new npc-> functions for npc scripts
string ONPCDead::toString() const
{
    return "NPCDEAD " + getArgument()->toString();
}
string ONPCCanSlide::toString() const
{
    return "NPCCANSLIDE " + getArgument()->toString();
}
string ONPCSlide::toString() const
{
    return "NPCSLIDE " + getArgument()->toString();
}
string ONPCBreatheFire::toString() const
{
    return "NPCFIREBREATH " + getArgument()->toString();
}
string ONPCNewDir8::toString() const
{
    return "NPCNEWDIR8 " + getArgument()->toString();
}
string ONPCRemove::toString() const
{
    return "NPCKICKBUCKET";
}
string OLWpnRemove::toString() const
{
    return "LWPNDEL";
}
string OEWpnRemove::toString() const
{
    return "EWPNDEL";
}
string OItemRemove::toString() const
{
    return "ITEMDEL";
}
string ONPCStopSFX::toString() const
{
    return "NPCSTOPBGSFX";
}
string ONPCAttack::toString() const
{
    return "NPCATTACK";
}
string ONPCNewDir::toString() const
{
    return "NPCNEWDIR " + getArgument()->toString();
}
string ONPCConstWalk::toString() const
{
    return "NPCCONSTWALK " + getArgument()->toString();
}
string ONPCConstWalk8::toString() const
{
    return "NPCCONSTWALK8 " + getArgument()->toString();
}
string ONPCVarWalk::toString() const
{
    return "NPCVARWALK " + getArgument()->toString();
}
string ONPCVarWalk8::toString() const
{
    return "NPCVARWALK8 " + getArgument()->toString();
}
string ONPCHaltWalk::toString() const
{
    return "NPCHALTWALK " + getArgument()->toString();
}
string ONPCHaltWalk8::toString() const
{
    return "NPCHALTWALK8 " + getArgument()->toString();
}
string ONPCFloatWalk::toString() const
{
    return "NPCFLOATWALK " + getArgument()->toString();
}
string ONPCHeroInRange::toString() const
{
    return "NPCLINKINRANGE " + getArgument()->toString();
}
string ONPCAdd::toString() const
{
    return "NPCADD " + getArgument()->toString();
}
string ONPCCanMove::toString() const
{
    return "NPCCANMOVE " + getArgument()->toString();
}
string ONPCHitWith::toString() const
{
    return "NPCHITWITH " + getArgument()->toString();
}
string ONPCKnockback::toString() const
{
    return "NPCKNOCKBACK " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetNPCDataName::toString() const
{
    return "NPCDATAGETNAME " + getArgument()->toString();
}

string OAllocateBitmap::toString() const
{
    return "ALLOCATEBITMAP " + getArgument()->toString();
}

string OClearBitmap::toString() const
{
    return "CLEARBITMAP";
}

string OBitmapClearToColor::toString() const
{
    return "BITMAPCLEARTOCOLOR";
}

string ORegenerateBitmap::toString() const
{
    return "REGENERATEBITMAP";
}

string OReadBitmap::toString() const
{
    return "READBITMAP";
}
string OWriteBitmap::toString() const
{
    return "WRITEBITMAP";
}
string OBitmapFree::toString() const
{
    return "BITMAPFREE";
}
string OBitmapOwn::toString() const
{
    return "BITMAPOWN";
}
string OFileOwn::toString() const
{
    return "FILEOWN";
}
string ODirectoryOwn::toString() const
{
    return "DIRECTORYOWN";
}
string ORNGOwn::toString() const
{
    return "RNGOWN";
}
string OBitmapWriteTile::toString() const
{
    return "BMPWRITETILE";
}
string OBitmapDither::toString() const
{
    return "BMPDITHER";
}
string OBitmapReplColor::toString() const
{
    return "BMPREPLCOLOR";
}
string OBitmapShiftColor::toString() const
{
    return "BMPSHIFTCOLOR";
}
string OBitmapMaskDraw::toString() const
{
    return "BMPMASKDRAW";
}
string OBitmapMaskDraw2::toString() const
{
    return "BMPMASKDRAW2";
}
string OBitmapMaskDraw3::toString() const
{
    return "BMPMASKDRAW3";
}
string OBitmapMaskBlit::toString() const
{
    return "BMPMASKBLIT";
}
string OBitmapMaskBlit2::toString() const
{
    return "BMPMASKBLIT2";
}
string OBitmapMaskBlit3::toString() const
{
    return "BMPMASKBLIT3";
}

string OIsValidBitmap::toString() const
{
    return "ISVALIDBITMAP " + getArgument()->toString();
}

string OIsAllocatedBitmap::toString() const
{
    return "ISALLOCATEDBITMAP " + getArgument()->toString();
}


string OBMPDrawScreenSolidRegister::toString() const
{
    return "BMPDRAWSCREENSOLIDR";
}

string OBMPDrawScreenSolid2Register::toString() const
{
    return "BMPDRAWSCREENSOLID2R";
}
string OBMPDrawScreenComboFRegister::toString() const
{
    return "BMPDRAWSCREENCOMBOFR";
}
string OBMPDrawScreenComboIRegister::toString() const
{
    return "BMPDRAWSCREENCOMBOIR";
}
string OBMPDrawScreenComboTRegister::toString() const
{
    return "BMPDRAWSCREENCOMBOTR";
}
string OGraphicsGetpixel::toString() const
{
    return "GRAPHICSGETPIXEL " + getArgument()->toString();
}
string OGraphicsCountColor::toString() const
{
    return "GRAPHICSCOUNTCOLOR " + getArgument()->toString();
}

string OBMPDrawScreenSolidityRegister::toString() const
{
    return "BMPDRAWLAYERSOLIDITYR";
}
string OBMPDrawScreenSolidMaskRegister::toString() const
{
    return "BMPDRAWLAYERSOLIDR";
}
string OBMPDrawScreenCTypeRegister::toString() const
{
    return "BMPDRAWLAYERCTYPER";
}
string OBMPDrawScreenCFlagRegister::toString() const
{
    return "BMPDRAWLAYERCFLAGR";
}
string OBMPDrawScreenCIFlagRegister::toString() const
{
    return "BMPDRAWLAYERCIFLAGR";
}

//Text ptr opcodes
string OFontHeight::toString() const
{
	return "FONTHEIGHTR " + getArgument()->toString();
}

string OStringWidth::toString() const
{
	return "STRINGWIDTHR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCharWidth::toString() const
{
	return "CHARWIDTHR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMessageWidth::toString() const
{
	return "MESSAGEWIDTHR " + getArgument()->toString();
}

string OMessageHeight::toString() const
{
	return "MESSAGEHEIGHTR " + getArgument()->toString();
}

//

string OStrCmp::toString() const
{
    return "STRINGCOMPARE " + getArgument()->toString();
}

string OStrNCmp::toString() const
{
    return "STRINGNCOMPARE " + getArgument()->toString();
}

string OStrICmp::toString() const
{
    return "STRINGICOMPARE " + getArgument()->toString();
}

string OStrNICmp::toString() const
{
    return "STRINGNICOMPARE " + getArgument()->toString();
}

//based on Ostrcpy
string oARRAYCOPY::toString() const
{
    return "ARRAYCOPY " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

/*to do 
  //1 INPUT, NO RETURN 
     { "REMCHR",                2,   0,   0,   0},
     { "STRINGUPPERLOWER",                2,   0,   0,   0},
     { "STRINGLOWERUPPER",                2,   0,   0,   0},
     { "STRINGCONVERTCASE",                2,   0,   0,   0},
     */

//1 inp, 1 ret, baseds on STRINGLENGTH / Ostrlen

string Oxlen::toString() const
{
    return "XLEN " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string Oxtoi::toString() const
{
    return "XTOI " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string Oilen::toString() const
{
    return "ILEN " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string Oatoi::toString() const
{
    return "ATOI " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

//2 inp, 1 ret, based on STRINGCOMPARE / OStrCmp

string Ostrcspn::toString() const
{
    return "STRCSPN " + getArgument()->toString();
}

string Ostrstr::toString() const
{
    return "STRSTR " + getArgument()->toString();
}

string Oitoa::toString() const
{
    return "ITOA " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Oxtoa::toString() const
{
    return "XTOA " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Oitoacat::toString() const
{
    return "ITOACAT " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSaveGameStructs::toString() const
{
    return "SAVEGAMESTRUCTS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OReadGameStructs::toString() const
{
    return "READGAMESTRUCTS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Ostrcat::toString() const
{
    return "STRCAT " + getArgument()->toString();
}

string Ostrspn::toString() const
{
    return "STRSPN " + getArgument()->toString();
}
string Ostrchr::toString() const
{
    return "STRCHR " + getArgument()->toString();
}

string Ostrrchr::toString() const
{
    return "STRRCHR " + getArgument()->toString();
}
string Oxlen2::toString() const
{
    return "XLEN2 " + getArgument()->toString();
}

string Oxtoi2::toString() const
{
    return "XTOI2 " + getArgument()->toString();
}
string Oilen2::toString() const
{
    return "ILEN2 " + getArgument()->toString();
}
string Oatoi2::toString() const
{
    return "ATOI2 " + getArgument()->toString();
}
string Oremchr2::toString() const
{
    return "REMCHR2 " + getArgument()->toString();
}


/*to do
  //3 INPUT 1 RET 
    { "XTOA3",		       1,   0,   0,   0},
    { "STRCATF",		       1,   0,   0,   0},
    { "ITOA3",		       1,   0,   0,   0},
    { "STRSTR3",		       1,   0,   0,   0},
    { "REMNCHR3",		       1,   0,   0,   0},
    { "STRCAT3",		       1,   0,   0,   0},
    { "STRNCAT3",		       1,   0,   0,   0},
    { "STRCHR3",		       1,   0,   0,   0},
    { "STRRCHR3",		       1,   0,   0,   0},
    { "STRSPN3",		       1,   0,   0,   0},
    { "STRCSPN3",		       1,   0,   0,   0},
    
*/








string Ouppertolower::toString() const
{
    return "UPPERTOLOWER " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string Olowertoupper::toString() const
{
    return "LOWERTOUPPER " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string Oconvertcase::toString() const
{
    return "CONVERTCASE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

//Game->GetByString functions
//similar to Oconvertcase

string OGETNPCSCRIPT::toString() const
{
    return "GETNPCSCRIPT " + getArgument()->toString();
}
string OGETLWEAPONSCRIPT::toString() const
{
    return "GETLWEAPONSCRIPT " + getArgument()->toString();
}
string OGETEWEAPONSCRIPT::toString() const
{
    return "GETEWEAPONSCRIPT " + getArgument()->toString();
}
string OGETGENERICSCRIPT::toString() const
{
    return "GETGENERICSCRIPT " + getArgument()->toString();
}
string OGETHEROSCRIPT::toString() const
{
    return "GETHEROSCRIPT " + getArgument()->toString();
}
string OGETGLOBALSCRIPT::toString() const
{
    return "GETGLOBALSCRIPT " + getArgument()->toString();
}
string OGETDMAPSCRIPT::toString() const
{
    return "GETDMAPSCRIPT " + getArgument()->toString();
}
string OGETSCREENSCRIPT::toString() const
{
    return "GETSCREENSCRIPT " + getArgument()->toString();
}
string OGETSPRITESCRIPT::toString() const
{
    return "GETSPRITESCRIPT " + getArgument()->toString();
}
string OGETUNTYPEDSCRIPT::toString() const
{
    return "GETUNTYPEDSCRIPT " + getArgument()->toString();
}
string OGETSUBSCREENSCRIPT::toString() const
{
    return "GETSUBSCREENSCRIPT " + getArgument()->toString();
}
string OGETNPCBYNAME::toString() const
{
    return "GETNPCBYNAME " + getArgument()->toString();
}
string OGETITEMBYNAME::toString() const
{
    return "GETITEMBYNAME " + getArgument()->toString();
}
string OGETCOMBOBYNAME::toString() const
{
    return "GETCOMBOBYNAME " + getArgument()->toString();
}
string OGETDMAPBYNAME::toString() const
{
    return "GETDMAPBYNAME " + getArgument()->toString();
}

string OLoadNPCBySUIDRegister::toString() const
{
    return "LOADNPCBYSUID " + getArgument()->toString();
}
string OLoadLWeaponBySUIDRegister::toString() const
{
    return "LOADLWEAPONBYSUID " + getArgument()->toString();
}
string OLoadEWeaponBySUIDRegister::toString() const
{
    return "LOADWEAPONCBYSUID " + getArgument()->toString();
}

string OByte::toString() const
{
    return "TOBYTE " + getArgument()->toString();
}

string OToInteger::toString() const
{
    return "TOINTEGER " + getArgument()->toString();
}

string OFloor::toString() const
{
    return "FLOOR " + getArgument()->toString();
}

string OCeiling::toString() const
{
    return "CEILING " + getArgument()->toString();
}

string OSByte::toString() const
{
    return "TOSIGNEDBYTE " + getArgument()->toString();
}
string OWord::toString() const
{
    return "TOWORD " + getArgument()->toString();
}
string OShort::toString() const
{
    return "TOSHORT " + getArgument()->toString();
}

string OReturn::toString() const
{
	return "RETURN";
}

string ODirExists::toString() const
{
	return "DIREXISTS " + getArgument()->toString();
}

string OFileExists::toString() const
{
	return "FILEEXISTS " + getArgument()->toString();
}

string OFileSystemRemove::toString() const
{
	return "FILESYSREMOVE " + getArgument()->toString();
}

string OFileClose::toString() const
{
	return "FILECLOSE";
};

string OFileFree::toString() const
{
	return "FILEFREE";
};

string OFileIsAllocated::toString() const
{
	return "FILEISALLOCATED";
};

string OFileIsValid::toString() const
{
	return "FILEISVALID";
};

string OAllocateFile::toString() const
{
	return "FILEALLOCATE";
};

string OFileFlush::toString() const
{
	return "FILEFLUSH";
};

string OFileGetChar::toString() const
{
	return "FILEGETCHAR";
};

string OFileRewind::toString() const
{
	return "FILEREWIND";
};

string OFileClearError::toString() const
{
	return "FILECLEARERR";
};
//
string OFileOpen::toString() const
{
	return "FILEOPEN " + getArgument()->toString();
};

string OFileCreate::toString() const
{
	return "FILECREATE " + getArgument()->toString();
};

string OFileReadString::toString() const
{
	return "FILEREADSTR " + getArgument()->toString();
};

string OFileWriteString::toString() const
{
	return "FILEWRITESTR " + getArgument()->toString();
};

string OFilePutChar::toString() const
{
	return "FILEPUTCHAR " + getArgument()->toString();
};

string OFileUngetChar::toString() const
{
	return "FILEUNGETCHAR " + getArgument()->toString();
};

string OFileGetError::toString() const
{
	return "FILEGETERROR " + getArgument()->toString();
};
//
string OFileReadChars::toString() const
{
	return "FILEREADCHARS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileReadBytes::toString() const
{
	return "FILEREADBYTES " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OFileReadInts::toString() const
{
	return "FILEREADINTS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileWriteChars::toString() const
{
	return "FILEWRITECHARS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileWriteBytes::toString() const
{
	return "FILEWRITEBYTES " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OFileWriteInts::toString() const
{
	return "FILEWRITEINTS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileSeek::toString() const
{
	return "FILESEEK " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileOpenMode::toString() const
{
	return "FILEOPENMODE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileRemove::toString() const
{
	return "FILEREMOVE";
};

string ODirectoryGet::toString() const
{
	return "DIRECTORYGET " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODirectoryReload::toString() const
{
	return "DIRECTORYRELOAD";
}

string ODirectoryFree::toString() const
{
	return "DIRECTORYFREE";
}

string OStackFree::toString() const
{
	return "STACKFREE";
}
string OStackOwn::toString() const
{
	return "STACKOWN";
}
string OStackClear::toString() const
{
	return "STACKCLEAR";
}

string OStackPopBack::toString() const
{
	return "STACKPOPBACK " + getArgument()->toString();
}
string OStackPopFront::toString() const
{
	return "STACKPOPFRONT " + getArgument()->toString();
}
string OStackPeekBack::toString() const
{
	return "STACKPEEKBACK " + getArgument()->toString();
}
string OStackPeekFront::toString() const
{
	return "STACKPEEKFRONT " + getArgument()->toString();
}
string OStackPushBack::toString() const
{
	return "STACKPUSHBACK " + getArgument()->toString();
}
string OStackPushFront::toString() const
{
	return "STACKPUSHFRONT " + getArgument()->toString();
}
string OStackGet::toString() const
{
	return "STACKGET " + getArgument()->toString();
}
string OStackSet::toString() const
{
    return "STACKSET " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OModuleGetIC::toString() const
{
    return "MODULEGETIC " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

//////////////////////////////////////////////////////////////////////////////////////

string ORunGenericFrozenScript::toString() const
{
	return "RUNGENFRZSCR " + getArgument()->toString();
};

string OReservedZ3_01::toString() const
{
	return "RESRVD_OP_Z3_01 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_02::toString() const
{
	return "RESRVD_OP_Z3_02 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_03::toString() const
{
	return "RESRVD_OP_Z3_03 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_04::toString() const
{
	return "RESRVD_OP_Z3_04 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_05::toString() const
{
	return "RESRVD_OP_Z3_05 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_06::toString() const
{
	return "RESRVD_OP_Z3_06 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_07::toString() const
{
	return "RESRVD_OP_Z3_07 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_08::toString() const
{
	return "RESRVD_OP_Z3_08 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_09::toString() const
{
	return "RESRVD_OP_Z3_09 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_10::toString() const
{
	return "RESRVD_OP_Z3_10 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OSubscrSwapPages::toString() const
{
	return "SUBDATA_SWAP_PAGES";
}
string OSubscrPgFindWidget::toString() const
{
	return "SUBPAGE_FIND_WIDGET";
}
string OSubscrPgMvCursor::toString() const
{
	return "SUBPAGE_MOVE_SEL";
}
string OSubscrPgSwapWidgets::toString() const
{
	return "SUBPAGE_SWAP_WIDG";
}
string OSubscrPgNewWidget::toString() const
{
	return "SUBPAGE_NEW_WIDG";
}
string OSubscrPgDelete::toString() const
{
	return "SUBPAGE_DELETE";
}
string OGetSubWidgSelTxtOverride::toString() const
{
	return "SUBWIDG_GET_SELTEXT_OVERRIDE " + getArgument()->toString();
}
string OSetSubWidgSelTxtOverride::toString() const
{
	return "SUBWIDG_SET_SELTEXT_OVERRIDE " + getArgument()->toString();
}
string OSubWidgTy_GetText::toString() const
{
	return "SUBWIDG_TY_GETTEXT " + getArgument()->toString();
}
string OSubWidgTy_SetText::toString() const
{
	return "SUBWIDG_TY_SETTEXT " + getArgument()->toString();
}


string OSubscrPgFindWidgetLbl::toString() const
{
	return "SUBPAGE_FIND_WIDGET_BY_LABEL";
}


string OGetSubWidgLabel::toString() const
{
	return "SUBWIDG_GET_LABEL " + getArgument()->toString();
}
string OSetSubWidgLabel::toString() const
{
	return "SUBWIDG_SET_LABEL " + getArgument()->toString();
}


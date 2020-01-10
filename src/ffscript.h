#ifndef _FFSCRIPT_H_
#define _FFSCRIPT_H_
#include "zdefs.h"
#include <utility>
#include <string>
#include <list>

extern long ffmisc[32][16];
extern refInfo ffcScriptData[32];

long get_register(const long arg);
int run_script(const byte type, const word script, const byte i = -1); //Global scripts don't need 'i'
int ffscript_engine(const bool preload);

void clear_ffc_stack(const byte i);
void clear_global_stack();
void deallocateArray(const long ptrval);
void clearScriptHelperData();

enum { ccNone, ccWhite, ccWhiteIntense, ccRed, ccGreen, ccBlue, ccRedIntense, ccGreenIntense, ccBlueIntense,
	ccRedGreen, ccRedBlue, ccGreenBlue, ccRedGreenIntense, ccRedBlueIntense, ccGreenBlueIntense };



class FFScript
{
	
    
	public:
		void ZScriptConsole(bool open);
		void ZScriptConsolePrint(int colourformat, const char * const format,...);

	private:
};

struct script_command
{
    char name[32];
    byte args;
    byte arg1_type; //0=reg, 1=val;
    byte arg2_type; //0=reg, 1=val;
    bool more_stuff;
};

struct script_variable
{
    char name[32];
    long id;
    word maxcount;
    byte multiple;
};

// Defines for script flags
#define TRUEFLAG          0x0001
#define MOREFLAG          0x0002
#define TRIGGERED         0x0004

//What are these for exactly?
//#define fflong(x,y,z)       (((x[(y)][(z)])<<24)+((x[(y)][(z)+1])<<16)+((x[(y)][(z)+2])<<8)+(x[(y)][(z)+3]))
//#define ffword(x,y,z)       (((x[(y)][(z)])<<8)+(x[(y)][(z)+1]))

// Defines for the ASM operations
enum ASM_DEFINE
{
	SETV,                 //0x0000
	SETR,                 //0x0001
	ADDR,                 //0x0002
	ADDV,                 //0x0003
	SUBR,                 //0x0004
	SUBV,                 //0x0005
	MULTR,                //0x0006
	MULTV,                //0x0007
	DIVR,                 //0x0008
	DIVV,                 //0x0009
	WAITFRAME,            //0x000A
	GOTO,                 //0x000B
	CHECKTRIG,            //0x000C //NOT IMPLEMENTED
	WARP,                 //0x000D
	COMPARER,             //0x000E
	COMPAREV,             //0x000F
	GOTOTRUE,             //0x0010
	GOTOFALSE,            //0x0011
	GOTOLESS,             //0x0012
	GOTOMORE,             //0x0013
	LOAD1,                //0x0014
	LOAD2,                //0x0015
	SETA1,                //0x0016
	SETA2,                //0x0017
	QUIT,                 //0x0018
	SINR,                 //0x0019
	SINV,                 //0x001A
	COSR,                 //0x001B
	COSV,                 //0x001C
	TANR,                 //0x001D
	TANV,                 //0x001E
	MODR,                 //0x001F
	MODV,                 //0x0020
	ABSR,                 //0x0021
	MINR,                 //0x0022
	MINV,                 //0x0023
	MAXR,                 //0x0024
	MAXV,                 //0x0025
	RNDR,                 //0x0026
	RNDV,                 //0x0027
	FACTORIAL,            //0x0028
	POWERR,               //0x0029
	POWERV,               //0x002A
	IPOWERR,              //0x002B
	IPOWERV,              //0x002C
	ANDR,                 //0x002D
	ANDV,                 //0x002E
	ORR,                  //0x002F
	ORV,                  //0x0030
	XORR,                 //0x0031
	XORV,                 //0x0032
	NANDR,                //0x0033
	NANDV,                //0x0034
	NORR,                 //0x0035
	NORV,                 //0x0036
	XNORR,                //0x0037
	XNORV,                //0x0038
	NOT,                  //0x0039
	LSHIFTR,              //0x003A
	LSHIFTV,              //0x003B
	RSHIFTR,              //0x003C
	RSHIFTV,              //0x003D
	TRACER,               //0x003E
	TRACEV,               //0x003F
	TRACE3,               //0x0040
	LOOP,                 //0x0041
	PUSHR,                //0x0042
	PUSHV,                //0x0043
	POP,                  //0x0044
	ENQUEUER,             //0x0045 //NOT IMPLEMENTED
	ENQUEUEV,             //0x0046 //NOT IMPLEMENTED
	DEQUEUE,              //0x0047 //NOT IMPLEMENTED
	PLAYSOUNDR,           //0x0048
	PLAYSOUNDV,           //0x0049
	LOADLWEAPONR,          //0x004A
	LOADLWEAPONV,          //0x004B
	LOADITEMR,             //0x004C
	LOADITEMV,             //0x004D
	LOADNPCR,              //0x004E
	LOADNPCV,              //0x004F
	CREATELWEAPONR,        //0x0050
	CREATELWEAPONV,        //0x0051
	CREATEITEMR,           //0x0052
	CREATEITEMV,           //0x0053
	CREATENPCR,            //0x0054
	CREATENPCV,            //0x0055
	LOADI,                 //0x0056
	STOREI,                //0x0057
	GOTOR,                 //0x0058
	SQROOTV,               //0x0059
	SQROOTR,               //0x005A
	CREATEEWEAPONR,        //0x005B
	CREATEEWEAPONV,        //0x005C
	PITWARP,               //0x005D
	WARPR,                 //0x005E
	PITWARPR,              //0x005F
	CLEARSPRITESR,         //0x0060
	CLEARSPRITESV,         //0x0061
	RECTR,                 //0x0062
	CIRCLER,               //0x0063
	ARCR,                  //0x0064
	ELLIPSER,              //0x0065
	LINER,                 //0x0066
	PUTPIXELR,             //0x0067
	DRAWTILER,             //0x0068
	DRAWCOMBOR,            //0x0069
	ELLIPSE2,              //0x006A
	SPLINER,               //0x006B
	FLOODFILL,             //0x006C
	COMPOUNDR,             //0x006D
	COMPOUNDV,             //0x006E
	MSGSTRR,               //0x006F
	MSGSTRV,               //0x0070
	ISVALIDITEM,           //0x0071
	ISVALIDNPC,            //0x0072
	PLAYMIDIR,             //0x0073
	PLAYMIDIV,             //0x0074
	COPYTILEVV,            //0x0075
	COPYTILEVR,            //0x0076
	COPYTILERV,            //0x0077
	COPYTILERR,            //0x0078
	SWAPTILEVV,            //0x0079
	SWAPTILEVR,            //0x007A
	SWAPTILERV,            //0x007B
	SWAPTILERR,            //0x007C
	CLEARTILEV,            //0x007D
	CLEARTILER,            //0x007E
	OVERLAYTILEVV,         //0x007F
	OVERLAYTILEVR,         //0x0080
	OVERLAYTILERV,         //0x0081
	OVERLAYTILERR,         //0x0082
	FLIPROTTILEVV,         //0x0083
	FLIPROTTILEVR,         //0x0084
	FLIPROTTILERV,         //0x0085
	FLIPROTTILERR,         //0x0086
	GETTILEPIXELV,         //0x0087
	GETTILEPIXELR,         //0x0088
	SETTILEPIXELV,         //0x0089
	SETTILEPIXELR,         //0x008A
	SHIFTTILEVV,           //0x008B
	SHIFTTILEVR,           //0x008C
	SHIFTTILERV,           //0x008D
	SHIFTTILERR,           //0x008E
	ISVALIDLWPN,           //0x008F
	ISVALIDEWPN,           //0x0090
	LOADEWEAPONR,          //0x0091
	LOADEWEAPONV,          //0x0092
	ALLOCATEMEMR,          //0x0093
	ALLOCATEMEMV,          //0x0094
	ALLOCATEGMEMV,         //0x0095
	DEALLOCATEMEMR,		 //0x0096
	DEALLOCATEMEMV,		 //0x0097 //Pointless, can't deallocate a value
	WAITDRAW,				 //0x0098
	ARCTANR,				 //0x0099
	LWPNUSESPRITER,		 //0x09A
	LWPNUSESPRITEV,		 //0x09B
	EWPNUSESPRITER,		 //0x09C
	EWPNUSESPRITEV,		 //0x09D
	LOADITEMDATAR,		 //0x09E
	LOADITEMDATAV,		 //0x09F
	BITNOT,				 //0x00A0
	LOG10,				 //0x00A1
	LOGE,                  //0x00A2
	ISSOLID,               //0x00A3
	LAYERSCREEN,           //0x00A4
	LAYERMAP,              //0x00A5
	TRACE2R,		    //0x00A6
	TRACE2V,		    //0x00A7
	TRACE4,			    //0x00A8
	TRACE5,			    //0x00A9
	SECRETS,		    //0x00AA
	DRAWCHARR,		    //0x00AB
	GETSCREENFLAGS,	    //0x00AC
	QUADR,              //0X00AD
	TRIANGLER,          //0X00AE
	ARCSINR,              //0x00AF
	ARCSINV,              //0x00B0
	ARCCOSR,              //0x00B1
	ARCCOSV,              //0x00B2
	GAMEEND,              //0x00B3
	DRAWINTR,             //0x00B4
	SETTRUE,              //0x00B5
	SETFALSE,             //0x00B6
	SETMORE,              //0x00B7
	SETLESS,              //0x00B8
	FASTTILER,            //0x00B9
	FASTCOMBOR,           //0x00BA
	DRAWSTRINGR,          //0x00BB
	SETSIDEWARP,			//0x00BC
	SAVE,				    //0x00BD
	TRACE6,				//0x00BE
	WHATNO0x00BF, //PTROFF,//0x00BF
	QUAD3DR,              //0x00C0
	TRIANGLE3DR,          //0x00C1
	SETCOLORB,        	//0x00C2 for 2.6
	SETDEPTHB,        	//0x00C3 '
	GETCOLORB,        	//0x00C4 '
	GETDEPTHB,        	//0x00C5 '
	COMBOTILE,			//0x00C6
	SETTILEWARP,			//0x00C7
	GETSCREENEFLAGS,		//0x00C8
	GETSAVENAME,          //0x00C9
	ARRAYSIZE,            //0x00CA
	ITEMNAME,             //0x00CB
	SETSAVENAME,          //0x00CC
	NPCNAME,         		//0x00CD
	GETMESSAGE,			//0x00CE
	GETDMAPNAME,			//0x00CF
	GETDMAPTITLE,			//0x00D0
	GETDMAPINTRO,			//0x00D1
	ALLOCATEGMEMR,        //0x00D2
	BITMAPR,         		//0x00D3
	SETRENDERTARGET,      //0x00D4
	PLAYENHMUSIC,         //0x00D5
	GETMUSICFILE,         //0x00D6
	GETMUSICTRACK,        //0x00D7
	SETDMAPENHMUSIC,      //0x00D8
	DRAWLAYERR,           //0x00D9
	DRAWSCREENR,          //0x00DA
	BREAKSHIELD,          //0x00DB
	SAVESCREEN,           //0x00DC
	SAVEQUITSCREEN,       //0x00DD
	SELECTAWPNR,          //0x00DE
	SELECTAWPNV,          //0x00DF
	SELECTBWPNR,          //0x00E0
	SELECTBWPNV,          //0x00E1
	GETSIDEWARPDMAP,      //0x00E2
	GETSIDEWARPSCR,       //0x00E3
	GETSIDEWARPTYPE,      //0x00E4
	GETTILEWARPDMAP,      //0x00E5
	GETTILEWARPSCR,       //0x00E6
	GETTILEWARPTYPE,      //0x00E7
	GETFFCSCRIPT,         //0x00E8
	BITMAPEXR,	//0x00E9
	__RESERVED_FOR_QUAD2R, //0x00EA
	WAVYIN, //0x00EB
	WAVYOUT, //0x00EC
	ZAPIN, //0x00ED
	ZAPOUT, //0x00EF
	OPENWIPE, //0x00F0
	FREE0x00F1, //0x00F1 was SETLINKTILE
	FREE0x00F2, //0x00F2 was SETLINKEXTEND
	FREE0x00F3, //0x00F3 was GETLINKEXTEND
	SETMESSAGE,			//0x00F4
	SETDMAPNAME,			//0x00F5
	SETDMAPTITLE,			//0x00F5
	SETDMAPINTRO,			//0x00F7
	GREYSCALEON,			//0x00F8
	GREYSCALEOFF,			//0x00F9
	ENDSOUNDR,           //0x00FA
	ENDSOUNDV,           //0x00FB
	PAUSESOUNDR, 	//0x00FC
	PAUSESOUNDV,	//0x00FD
	RESUMESOUNDR,	//0x00FE
	RESUMESOUNDV,	//0x00FF
	PAUSEMUSIC,		//0x0100
	RESUMEMUSIC,	//0x0101
	LWPNARRPTR,	//0x0102
	EWPNARRPTR,	//0x0103
	ITEMARRPTR,	//0x0104
	IDATAARRPTR,	//0x0105
	FFCARRPTR,	//0x0106
	BOOLARRPTR,	//0x0107
	NPCARRPTR,	//0x0108
	LWPNARRPTR2,	//0x0109
	EWPNARRPTR2,	//0x0110
	ITEMARRPTR2,	//0x0111
	IDATAARRPTR2,	//0x0112
	FFCARRPTR2,	//0x0113
	BOOLARRPTR2,	//0x0114
	NPCARRPTR2,	//0x0115
	ARRAYSIZEB,            //0x0116
	ARRAYSIZEF,            //0x0117 
	ARRAYSIZEN,            //0x0118
	ARRAYSIZEL,            //0x0119
	ARRAYSIZEE,            //0x011A
	ARRAYSIZEI,            //0x011B
	ARRAYSIZEID,            //0x011C
	POLYGONR,		//0x011D
	__RESERVED_FOR_POLYGON3DR,		//0x011E
	__RESERVED_FOR_SETRENDERSOURCE,	//0x011F
	LINESARRAY,	//0x0120
	PIXELARRAYR,	//0x0121
	TILEARRAYR,		//0x0122
	COMBOARRAYR,	//0x0123
	RES0000,		//0x0124
	RES0001,		//0x0125
	RES0002,		//0x0126
	RES0003,		//0x0127
	RES0004,		//0x0128
	RES0005,		//0x0129
	RES0006,		//0x012A
	RES0007,		//0x012B
	RES0008,		//0x012C
	RES0009,		//0x012D
	RES000A,		//0x012E
	RES000B,		//0x012F
	RES000C,		//0x0130
	RES000D,		//0x0131
	RES000E,		//0x0132
	RES000F,		//0x0133
	__RESERVED_FOR_CREATELWPN2VV,            //0x0134
	__RESERVED_FOR_CREATELWPN2VR,            //0x0135
	__RESERVED_FOR_CREATELWPN2RV,            //0x0136
	__RESERVED_FOR_CREATELWPN2RR,            //0x0137
	GETSCREENDOOR, //0x0138
	GETSCREENENEMY, //0x0139
	PAUSESFX,
	RESUMESFX,
	CONTINUESFX,
	ADJUSTSFX,
	//__RESERVED_FOR_GETSCREENFLAG, //0x013A
	GETITEMSCRIPT,
	GETSCREENLAYOP,
	GETSCREENSECCMB,
	GETSCREENSECCST,
	GETSCREENSECFLG,
	GETSCREENLAYMAP,
	GETSCREENLAYSCR,
	GETSCREENPATH,
	GETSCREENWARPRX,
	GETSCREENWARPRY,
	TRIGGERSECRETR,
	TRIGGERSECRETV,
	CHANGEFFSCRIPTR,
	CHANGEFFSCRIPTV,
	
	//NPCData
	GETNPCDATAFLAGS,
	GETNPCDATAFLAGS2,
	GETNPCDATAWIDTH,
	GETNPCDATAHEIGHT,
	GETNPCDATASTILE,
	GETNPCDATASWIDTH,
	GETNPCDATASHEIGHT,
	GETNPCDATAETILE,
	GETNPCDATAEWIDTH,
	GETNPCDATAHP,
	GETNPCDATAFAMILY,
	GETNPCDATACSET,
	GETNPCDATAANIM,
	GETNPCDATAEANIM,
	GETNPCDATAFRAMERATE,
	GETNPCDATAEFRAMERATE,
	GETNPCDATATOUCHDMG,
	GETNPCDATAWPNDAMAGE,
	GETNPCDATAWEAPON,
	GETNPCDATARANDOM,
	GETNPCDATAHALT,
	GETNPCDATASTEP,
	GETNPCDATAHOMING,
	GETNPCDATAHUNGER,
	GETNPCDATADROPSET,
	GETNPCDATABGSFX,
	GETNPCDATADEATHSFX,
	GETNPCDATAXOFS,
	GETNPCDATAYOFS,
	GETNPCDATAZOFS,
	GETNPCDATAHXOFS,
	GETNPCDATAHYOFS,
	GETNPCDATAHITWIDTH,
	GETNPCDATAHITHEIGHT,
	GETNPCDATAHITZ,
	GETNPCDATATILEWIDTH,
	GETNPCDATATILEHEIGHT,
	GETNPCDATAWPNSPRITE,
	//TWO INPUTS, ONE RETURN
	GETNPCDATASCRIPTDEF,
	GETNPCDATADEFENSE,
	GETNPCDATASIZEFLAG,
	GETNPCDATAATTRIBUTE,

	//TWO INPUTS, ONE RETURN
	SETNPCDATAFLAGS,
	SETNPCDATAFLAGS2,
	SETNPCDATAWIDTH,
	SETNPCDATAHEIGHT,
	SETNPCDATASTILE,
	SETNPCDATASWIDTH,
	SETNPCDATASHEIGHT,
	SETNPCDATAETILE,
	SETNPCDATAEWIDTH,
	SETNPCDATAHP,
	SETNPCDATAFAMILY,
	SETNPCDATACSET,
	SETNPCDATAANIM,
	SETNPCDATAEANIM,
	SETNPCDATAFRAMERATE,
	SETNPCDATAEFRAMERATE,
	SETNPCDATATOUCHDMG,
	SETNPCDATAWPNDAMAGE,
	SETNPCDATAWEAPON,
	SETNPCDATARANDOM,
	SETNPCDATAHALT,
	SETNPCDATASTEP,
	SETNPCDATAHOMING,
	SETNPCDATAHUNGER,
	SETNPCDATADROPSET,
	SETNPCDATABGSFX,
	SETNPCDATADEATHSFX,
	SETNPCDATAXOFS,
	SETNPCDATAYOFS,
	SETNPCDATAZOFS,
	SETNPCDATAHXOFS,
	SETNPCDATAHYOFS,
	SETNPCDATAHITWIDTH,
	SETNPCDATAHITHEIGHT,
	SETNPCDATAHITZ,
	SETNPCDATATILEWIDTH,
	SETNPCDATATILEHEIGHT,
	SETNPCDATAWPNSPRITE,
	SETNPCDATAHITSFX,
	GETNPCDATAHITSFX,
	//ComboData
	GCDBLOCKENEM,
	GCDBLOCKHOLE,
	GCDBLOCKTRIG,
	GCDCONVEYSPDX,
	GCDCONVEYSPDY,
	GCDCREATEENEM,
	GCDCREATEENEMWH,
	GCDCREATEENEMCH,
	GCDDIRCHTYPE,
	GCDDISTCHTILES,
	GCDDIVEITEM,
	GCDDOCK,
	GCDFAIRY,
	GCDFFCOMBOATTRIB,
	GCDFOOTDECOTILE,
	GCDFOOTDECOTYPE,
	GCDHOOKSHOTGRAB,
	GCDLADDERPASS,
	GCDLOCKBLOCKTYPE,
	GCDLOCKBLOCKCHANGE,
	GCDMAGICMIRRORTYPE,
	GCDMODIFYHPAMOUNT,
	GCDMODIFYHPDELAY,
	GCDMODIFYHPTYPE,
	GCDMODIFYMPAMOUNT,
	GCDMODIFYMPDELAY,
	GCDMODIFYMPTYPE,
	GCDNOPUSHBLOCKS,
	GCDOVERHEAD,
	GCDPLACEENEMY,
	GCDPUSHDIR,
	GCDPUSHWEIGHT,
	GCDPUSHWAIT,
	GCDPUSHED,
	GCDRAFT,
	GCDRESETROOM,
	GCDSAVEPOINT,
	GCDSCREENFREEZE,
	GCDSECRETCOMBO,
	GCDSINGULAR,
	GCDSLOWMOVE,
	GCDSTATUE,
	GCDSTEPTYPE,
	GCDSTEPCHANGETO,
	GCDSTRIKEREMNANTS,
	GCDSTRIKEREMNANTSTYPE,
	GCDSTRIKECHANGE,
	GCDSTRIKECHANGEITEM,
	GCDTOUCHITEM,
	GCDTOUCHSTAIRS,
	GCDTRIGGERTYPE,
	GCDTRIGGERSENS,
	GCDWARPTYPE,
	GCDWARPSENS,
	GCDWARPDIRECT,
	GCDWARPLOCATION,
	GCDWATER,
	GCDWHISTLE,
	GCDWINGAME,
	GCDBLOCKWEAPLVL,
	GCDTILE,
	GCDFLIP,
	GCDWALK,
	GCDTYPE,
	GCDCSETS,
	GCDFOO,
	GCDFRAMES,
	GCDSPEED,
	GCDNEXTCOMBO,
	GCDNEXTCSET,
	GCDFLAG,
	GCDSKIPANIM,
	GCDNEXTTIMER,
	GCDSKIPANIMY,
	GCDANIMFLAGS,
	GCDBLOCKWEAPON,
	GCDEXPANSION,
	GCDSTRIKEWEAPONS,
	SCDBLOCKENEM,
	SCDBLOCKHOLE,
	SCDBLOCKTRIG,
	SCDCONVEYSPDX,
	SCDCONVEYSPDY,
	SCDCREATEENEM,
	SCDCREATEENEMWH,
	SCDCREATEENEMCH,
	SCDDIRCHTYPE,
	SCDDISTCHTILES,
	SCDDIVEITEM,
	SCDDOCK,
	SCDFAIRY,
	SCDFFCOMBOATTRIB,
	SCDFOOTDECOTILE,
	SCDFOOTDECOTYPE,
	SCDHOOKSHOTGRAB,
	SCDLADDERPASS,
	SCDLOCKBLOCKTYPE,
	SCDLOCKBLOCKCHANGE,
	SCDMAGICMIRRORTYPE,
	SCDMODIFYHPAMOUNT,
	SCDMODIFYHPDELAY,
	SCDMODIFYHPTYPE,
	SCDMODIFYMPAMOUNT,
	SCDMODIFYMPDELAY,
	SCDMODIFYMPTYPE,
	SCDNOPUSHBLOCKS,
	SCDOVERHEAD,
	SCDPLACEENEMY,
	SCDPUSHDIR,
	SCDPUSHWEIGHT,
	SCDPUSHWAIT,
	SCDPUSHED,
	SCDRAFT,
	SCDRESETROOM,
	SCDSAVEPOINT,
	SCDSCREENFREEZE,
	SCDSECRETCOMBO,
	SCDSINGULAR,
	SCDSLOWMOVE,
	SCDSTATUE,
	SCDSTEPTYPE,
	SCDSTEPCHANGETO,
	SCDSTRIKEREMNANTS,
	SCDSTRIKEREMNANTSTYPE,
	SCDSTRIKECHANGE,
	SCDSTRIKECHANGEITEM,
	SCDTOUCHITEM,
	SCDTOUCHSTAIRS,
	SCDTRIGGERTYPE,
	SCDTRIGGERSENS,
	SCDWARPTYPE,
	SCDWARPSENS,
	SCDWARPDIRECT,
	SCDWARPLOCATION,
	SCDWATER,
	SCDWHISTLE,
	SCDWINGAME,
	SCDBLOCKWEAPLVL,
	SCDTILE,
	SCDFLIP,
	SCDWALK,
	SCDTYPE,
	SCDCSETS,
	SCDFOO,
	SCDFRAMES,
	SCDSPEED,
	SCDNEXTCOMBO,
	SCDNEXTCSET,
	SCDFLAG,
	SCDSKIPANIM,
	SCDNEXTTIMER,
	SCDSKIPANIMY,
	SCDANIMFLAGS,
	GETNPCDATATILE,
	GETNPCDATAEHEIGHT,
	SETNPCDATATILE,
	SETNPCDATAEHEIGHT,
	//SpriteData
	GETSPRITEDATASTRING,
	GETSPRITEDATATILE,
	GETSPRITEDATAMISC,
	GETSPRITEDATACGETS,
	GETSPRITEDATAFRAMES,
	GETSPRITEDATASPEED,
	GETSPRITEDATATYPE,
	SETSPRITEDATASTRING,
	SETSPRITEDATATILE,
	SETSPRITEDATAMISC,
	SETSPRITEDATACSETS,
	SETSPRITEDATAFRAMES,
	SETSPRITEDATASPEED,
	SETSPRITEDATATYPE,
	SETCONTINUESCREEN,
	SETCONTINUESTRING,
	LOADNPCDATAR,
	LOADNPCDATAV,
	LOADCOMBODATAR,
	LOADCOMBODATAV,
	LOADMAPDATAR,
	LOADMAPDATAV,
	LOADSPRITEDATAR,
	LOADSPRITEDATAV,
	LOADSCREENDATAR,
	LOADSCREENDATAV,
	LOADBITMAPDATAR,
	LOADBITMAPDATAV,
	LOADSHOPR,
	LOADSHOPV,
	LOADINFOSHOPR,
	LOADINFOSHOPV,
	
	LOADMESSAGEDATAR, //COMMAND
	LOADMESSAGEDATAV,//COMMAND

	MESSAGEDATASETSTRINGR, //command
	MESSAGEDATASETSTRINGV, //command
	MESSAGEDATAGETSTRINGR, //command
	MESSAGEDATAGETSTRINGV, //command
	
	LOADDMAPDATAR,
	LOADDMAPDATAV,
	DMAPDATAGETNAMER,
	DMAPDATAGETNAMEV,
	DMAPDATASETNAMER,
	DMAPDATASETNAMEV,
	DMAPDATAGETTITLER,
	DMAPDATAGETTITLEV,
	DMAPDATASETTITLER,
	DMAPDATASETTITLEV,
	DMAPDATAGETINTROR,
	DMAPDATAGETINTROV,
	DMAPDATANSETITROR,
	DMAPDATASETINTROV,
	DMAPDATAGETMUSICR,
	DMAPDATAGETMUSICV,
	DMAPDATASETMUSICR,
	DMAPDATASETMUSICV,
	
	ADJUSTSFXVOLUMER,		
	ADJUSTSFXVOLUMEV,		
	ADJUSTVOLUMER,		
	ADJUSTVOLUMEV,	
	
	FXWAVYR,
	FXWAVYV,
	FXZAPR,
	FXZAPV,
	GREYSCALER,
	GREYSCALEV,

	RETURN,
	MONOCHROMER,
	MONOCHROMEV,
	CLEARTINT,
	TINT,
	MONOHUE,
	
	BMPRECTR,
	BMPCIRCLER,
	BMPARCR,
	BMPELLIPSER,
	BMPLINER,
	BMPSPLINER,
	BMPPUTPIXELR,
	BMPDRAWTILER,
	BMPDRAWCOMBOR,
	BMPFASTTILER,
	BMPFASTCOMBOR,
	BMPDRAWCHARR,
	BMPDRAWINTR,
	BMPDRAWSTRINGR,
	BMPQUADR,
	BMPQUAD3DR,
	BMPTRIANGLER,
	BMPTRIANGLE3DR,
	BMPPOLYGONR,
	BMPDRAWLAYERR,
	BMPDRAWSCREENR,
	BMPBLIT,
	LINKWARPEXR,
	LINKWARPEXV,
	LINKEXPLODER,
	LINKEXPLODEV,
	NPCEXPLODER,
	NPCEXPLODEV,
	ITEMEXPLODER,
	ITEMEXPLODEV,
	LWEAPONEXPLODER,
	LWEAPONEXPLODEV,
	EWEAPONEXPLODER,
	EWEAPONEXPLODEV,
	RUNITEMSCRIPT,
	GETRTCTIMER,
	GETRTCTIMEV,
	
	//new npc functions for npc scripts
	NPCDEAD,
	NPCKICKBUCKET,
	NPCSTOPBGSFX,
	NPCCANMOVE,
	NPCNEWDIR8,
	NPCNEWDIR,
	NPCCONSTWALK,
	NPCCONSTWALK8,
	NPCVARWALK,
	NPCVARWALK8,
	NPCHALTWALK,
	NPCHALTWALK8,
	NPCFLOATWALK,
	// moved to a var: NPCLINEDUP,
	NPCLINKINRANGE,
	NPCATTACK,
	NPCPLACEONAXIS,
	NPCADD,
	NPCFIREBREATH,
	NPCCANSLIDE,
	NPCSLIDE,
	NPCHITWITH,
	// moved to a var: NPCCOLLISION 
	NPCGETINITDLABEL,
	GAMECONTINUE,
	MAPDATAISSOLID,
	SHOWF6SCREEN,
	NPCDATAGETNAME,
	PLAYENHMUSICEX,
	GETENHMUSICPOS,
	SETENHMUSICPOS,
	SETENHMUSICSPEED,
	ISVALIDBITMAP,
	READBITMAP,
	WRITEBITMAP,
	ALLOCATEBITMAP,
	CLEARBITMAP,
	REGENERATEBITMAP,
	BMPBLITTO,
	BMPDRAWSCREENSOLIDR,
	BMPDRAWSCREENCOMBOFR,
	BMPDRAWSCREENCOMBOIR,
	BMPDRAWSCREENCOMBOTR,
	BMPDRAWSCREENSOLID2R,
	GRAPHICSGETPIXEL,
	
	BMPDRAWLAYERSOLIDR,
	BMPDRAWLAYERCFLAGR,
	BMPDRAWLAYERCTYPER,
	BMPDRAWLAYERCIFLAGR,
	BMPDRAWLAYERSOLIDITYR,
	BMPMODE7,
	BITMAPGETPIXEL,
	NOP,
	STRINGCOMPARE,
	STRINGNCOMPARE,
	STRINGLENGTH,
	STRINGCOPY,
	CASTBOOLI,
	CASTBOOLF,
	SETTRUEI,
	SETFALSEI,
	SETMOREI,
	SETLESSI,
		//2 INPUT 0 RETURN 
	ARRAYCOPY,
	ARRAYNCOPY,
	    //1 INPUT, NO RETURN 
	REMCHR,
	STRINGUPPERLOWER,
	STRINGLOWERUPPER,
	STRINGCONVERTCASE,
	    //1 input, 1 ret
	XLEN,
	XTOI,
	ILEN,
	ATOI,
	    //2 INPUT, 1 RET
	STRCSPN,
	STRSTR,
	XTOA,
	ITOA,
	STRCAT,
	STRSPN,
	STRCHR,
	STRRCHR,
	    //2 INP, 1 RET OVERLOADS
	XLEN2,
	XTOI2,
	ILEN2,
	ATOI2,
	REMCHR2,    
	    //3 INPUT 1 RET 
	XTOA3,
	STRCATF,
	ITOA3,
	STRSTR3,
	REMNCHR3,
	STRCAT3,
	STRNCAT3,
	STRCHR3,
	STRRCHR3,
	STRSPN3,
	STRCSPN3,
	UPPERTOLOWER,
	LOWERTOUPPER,
	CONVERTCASE,
	//Game->Get
	GETNPCSCRIPT,
	GETLWEAPONSCRIPT,
	GETEWEAPONSCRIPT,
	GETHEROSCRIPT,
	GETGLOBALSCRIPT,
	GETDMAPSCRIPT,
	GETSCREENSCRIPT,
	GETSPRITESCRIPT,
	GETUNTYPEDSCRIPT,
	GETSUBSCREENSCRIPT,
	GETNPCBYNAME,
	GETITEMBYNAME,
	GETCOMBOBYNAME,
	GETDMAPBYNAME,
	
	SRNDR,
	SRNDV,
	SRNDRND,
	SAVEGAMESTRUCTS,
	READGAMESTRUCTS,
	
	ANDR32,
	ANDV32,
	ORR32,
	ORV32,
	XORR32,
	XORV32,
	BITNOT32,
	LSHIFTR32,
	LSHIFTV32,
	RSHIFTR32,
	RSHIFTV32,
	ISALLOCATEDBITMAP,

	FONTHEIGHTR,
	STRINGWIDTHR,
	CHARWIDTHR,
	MESSAGEWIDTHR,
	MESSAGEHEIGHTR,
	ISVALIDARRAY,
	DIREXISTS,
	GAMESAVEQUIT,
	GAMESAVECONTINUE,
	DRAWTILECLOAKEDR,
	BMPDRAWTILECLOAKEDR,
	DRAWCOMBOCLOAKEDR,
	BMPDRAWCOMBOCLOAKEDR,
	NPCKNOCKBACK,
	CLOSEWIPE,
	OPENWIPESHAPE,
	CLOSEWIPESHAPE,
	FILEEXISTS,
	BITMAPCLEARTOCOLOR,
	LOADNPCBYSUID,
	LOADLWEAPONBYSUID,
	LOADWEAPONCBYSUID,
	LOADDROPSETR,
    LOADTMPSCR,
    LOADSCROLLSCR,
    MAPDATAISSOLIDLYR,
    ISSOLIDLAYER,
	BREAKPOINT,
	TOBYTE,
	TOWORD,
	TOSHORT,
	TOSIGNEDBYTE,
	TOINTEGER,
	FLOOR,
	CEILING,


	NUMCOMMANDS           //0x016E
};


//ZASM registers
//When adding indexed variables the index will be loaded into ri.d[0], don't add a register for each one!
#define D(n)               ((0x0000)+(n)) //8
#define A(n)               ((0x0008)+(n)) //2
#define DATA                 0x000A
#define FCSET                0x000B
#define DELAY                0x000C
#define FX                   0x000D
#define FY                   0x000E
#define XD                   0x000F
#define YD                   0x0010
#define XD2                  0x0011
#define YD2                  0x0012
#define FLAG                 0x0013
#define WIDTH                0x0014
#define HEIGHT               0x0015
#define LINK                 0x0016
//#define COMBOD(n)          ((0x0017)+((n)*3)) ~loads spare here...
//#define COMBOC(n)          ((0x0018)+((n)*3))
//#define COMBOF(n)          ((0x0019)+((n)*3))

//n=0-175, so if n=175, then ((0x0019)+((n)*3))=0x00226
#define INPUTSTART           0x0227
#define INPUTUP              0x0228
#define INPUTDOWN            0x0229
#define INPUTLEFT            0x022A
#define INPUTRIGHT           0x022B
#define INPUTA               0x022C
#define INPUTB               0x022D
#define INPUTL               0x022E
#define INPUTR               0x022F

#define LINKX                0x0230
#define LINKY                0x0231
#define LINKDIR              0x0232
#define LINKHP               0x0233
#define LINKMP               0x0234
#define LINKMAXHP            0x0235
#define LINKMAXMP            0x0236
#define LINKACTION           0x0237
#define LINKITEMD            0x0238
#define LINKZ                0x0239
#define LINKJUMP             0x023A
#define LINKSWORDJINX        0x023B
#define LINKITEMJINX         0x023C
#define LINKDRUNK            0x023D
#define LINKHELD             0x023E
#define INPUTMAP             0x023F
#define LINKEQUIP            0x0240
#define LINKINVIS            0x0241
#define LINKINVINC           0x0242
#define LINKLADDERX          0x0243
#define LINKLADDERY          0x0244
#define LINKHITDIR           0x0245
#define LINKTILE             0x0246
#define LINKFLIP             0x0247
#define INPUTPRESSMAP        0x0248
//0x0249-0x0258 are reserved for future Link variables
#define UNUSED25             0x0249
#define UNUSED26             0x024A
#define UNUSED27             0x024B
#define UNUSED28             0x024C
#define UNUSED29             0x024D
#define UNUSED30             0x024E
#define UNUSED31             0x024F
#define UNUSED32             0x0250
#define UNUSED33             0x0251
#define UNUSED34             0x0252
#define UNUSED35             0x0253
#define UNUSED36             0x0254
#define UNUSED37             0x0255
#define UNUSED38             0x0256
#define UNUSED39             0x0257
#define UNUSED40             0x0258

#define LWPNX                0x0259
#define LWPNY                0x025A
#define LWPNDIR              0x025B
#define LWPNSTEP             0x025C
#define LWPNANGULAR          0x025D
#define LWPNANGLE            0x025E
#define LWPNDRAWTYPE         0x025F
#define LWPNPOWER            0x0260
#define LWPNDEAD             0x0261
#define LWPNID               0x0262
#define LWPNTILE             0x0263
#define LWPNCSET             0x0264
#define LWPNFLASHCSET        0x0265
#define LWPNFRAMES           0x0266
#define LWPNFRAME            0x0267
#define LWPNASPEED           0x0268
#define LWPNFLASH            0x0269
#define LWPNFLIP             0x026A
#define LWPNCOUNT            0x026B
#define LWPNEXTEND           0x026C
#define LWPNOTILE            0x026D
#define LWPNOCSET            0x026E
#define LWPNZ                0x026F
#define LWPNJUMP             0x0270
#define LWPNCOLLDET          0x0271
//   0x0272-   0x028B are reserved for future weapon variables
#define UNUSED47             0x0272
#define UNUSED48             0x0273
#define UNUSED49             0x0274
#define GETRENDERTARGET             0x0275
#define UNUSED51             0x0276
#define UNUSED52             0x0277
#define UNUSED53             0x0278
#define UNUSED54             0x0279
#define UNUSED55             0x027A
#define UNUSED56             0x027B
#define UNUSED57             0x027C
#define UNUSED58             0x027D
#define UNUSED59             0x027E
#define UNUSED60             0x027F
#define UNUSED61             0x0280
#define UNUSED62             0x0281
#define UNUSED63             0x0282
#define UNUSED64             0x0283
#define UNUSED65             0x0284
#define UNUSED66             0x0285
#define UNUSED67             0x0286
#define UNUSED68             0x0287
#define UNUSED69             0x0288
#define UNUSED70             0x0289
#define UNUSED71             0x028A
#define UNUSED72             0x028B

#define ITEMX                0x028C
#define ITEMY                0x028D
#define ITEMDRAWTYPE         0x028E
#define ITEMID               0x028F
#define ITEMTILE             0x0290
#define ITEMCSET             0x0291
#define ITEMFLASHCSET        0x0292
#define ITEMFRAMES           0x0293
#define ITEMFRAME            0x0294
#define ITEMASPEED           0x0295
#define ITEMDELAY            0x0296
#define ITEMFLASH            0x0297
#define ITEMFLIP             0x0298
#define ITEMCOUNT            0x0299
#define IDATAFAMILY          0x029A
#define IDATALEVEL           0x029B
#define IDATAKEEP            0x029C
#define IDATAAMOUNT          0x029D
#define IDATASETMAX          0x029E
#define IDATAMAX             0x029F
#define IDATACOUNTER         0x02A0
#define ITEMEXTEND           0x02A1
#define ITEMZ                0x02A2
#define ITEMJUMP             0x02A3
#define ITEMOTILE            0x02A4
#define IDATAUSESOUND        0x02A5
#define IDATAPOWER           0x02A6

//0x02A7-0x02B9 are reserved for future item variables
#define UNUSED86             0x02A7
#define UNUSED87             0x02A8
#define UNUSED88             0x02A9
#define UNUSED89             0x02AA
#define UNUSED90             0x02AB
#define UNUSED91             0x02AC
#define UNUSED92             0x02AD
#define UNUSED93             0x02AE
#define UNUSED94             0x02AF
#define UNUSED95             0x02B0
#define UNUSED96             0x02B1
#define UNUSED97             0x02B2
#define UNUSED98             0x02B3
#define UNUSED99             0x02B4
#define UNUSED100            0x02B5
#define UNUSED101            0x02B6
#define UNUSED102            0x02B7
#define UNUSED103            0x02B8

#define ZELDAVERSION            0x02B9

#define NPCX                 0x02BA
#define NPCY                 0x02BB
#define NPCDIR               0x02BC
#define NPCRATE              0x02BD
#define NPCFRAMERATE         0x02BE
#define NPCHALTRATE          0x02BF
#define NPCDRAWTYPE          0x02C0
#define NPCHP                0x02C1
#define NPCID                0x02C2
#define NPCDP                0x02C3
#define NPCWDP               0x02C4
#define NPCOTILE             0x02C5
#define NPCENEMY             0x02C6
#define NPCWEAPON            0x02C7
#define NPCITEMSET           0x02C8
#define NPCCSET              0x02C9
#define NPCBOSSPAL           0x02CA
#define NPCBGSFX             0x02CB
#define NPCCOUNT             0x02CC
#define NPCEXTEND            0x02CD
#define NPCZ                 0x02CE
#define NPCJUMP              0x02CF
#define NPCSTEP              0x02D0
#define NPCTILE              0x02D1
#define NPCHOMING            0x02D2
#define NPCDD	             0x02D3
//#define NPCD(n)	           ((0x101D)+(n)) //10 spare
#define NPCTYPE              0x02DE
#define NPCMFLAGS            0x02DF
#define NPCCOLLDET           0x02E0
#define NPCSTUN              0x02E1
#define NPCHUNGER            0x02E2
//0x02E3-0x02EB are reserved for future NPC variables
#define UNUSED126            0x02E2
#define UNUSED127            0x02E3
#define UNUSED128            0x02E4
#define UNUSED129            0x02E5
#define UNUSED130            0x02E6
#define UNUSED131            0x02E7
#define UNUSED132            0x02E8
#define UNUSED133            0x02E9
#define UNUSED134            0x02EA
#define UNUSED135            0x02EB
//I'm stealing this one for the stack pointer -DD
#define SP                   0x02EC

//#define SD(n)              ((0x02ED)+(n)) //Spare 8
#define GD(n)              ((0x02F5)+(n)) //256

// Defines for accessing gamedata
//#define GAMECOUNTER(n)     ((0x03F5)+((n)*3)) //All of these spare
//#define GAMEMCOUNTER(n)    ((0x03F6)+((n)*3))
//#define GAMEDCOUNTER(n)    ((0x03F7)+((n)*3))
//#define GAMEGENERIC(n)     ((0x0455)+(n))
//#define GAMEITEMS(n)       ((0x0555)+(n))
//#define GAMELITEMS(n)      ((0x0655)+(n))
//#define GAMELKEYS(n)       ((0x0755)+(n))
#define GAMEDEATHS           0x0855
#define GAMECHEAT            0x0856
#define GAMETIME             0x0857
#define GAMEHASPLAYED        0x0858
#define GAMETIMEVALID        0x0859
#define GAMEGUYCOUNTD        0x085A
#define GAMEGUYCOUNT         0x085B
#define GAMECONTSCR          0x085C
#define GAMECONTDMAP         0x085D
#define GAMECOUNTERD         0x085E
#define GAMEMCOUNTERD        0x085F
#define GAMEDCOUNTERD        0x0860
#define GAMEGENERICD         0x0861
#define GAMEITEMSD           0x0862
#define GAMELITEMSD          0x0863
#define GAMELKEYSD           0x0864
#define ROOMTYPE             0x0865
#define ROOMDATA             0x0866
//#define SCREENSTATE(n)     ((0x0867)+(n)) //spare 30
#define SCREENSTATED         0x0885

#define CURSCR               0x0886
#define CURMAP               0x0887
#define CURDMAP              0x0888
#define COMBODD	             0x0889
#define COMBOCD              0x088A
#define COMBOFD              0x088B
#define COMBOTD              0x088C
#define COMBOID              0x088D

//Defines for all the extra crap that's been added since
#define REFITEMCLASS         0x088E
#define REFITEM              0x088F
#define REFFFC               0x0890
#define REFLWPN              0x0891
#define REFNPC               0x0892
#define REFLWPNCLASS         0x0893
#define REFNPCCLASS          0x0894
//#define REFSUBSCREEN         0x0895 //Not implemented
#define REFEWPN              0x0896
#define REFEWPNCLASS         0x0897

#define EWPNX                0x0898
#define EWPNY                0x0899
#define EWPNDIR              0x089A
#define EWPNSTEP             0x089B
#define EWPNANGULAR          0x089C
#define EWPNANGLE            0x089D
#define EWPNDRAWTYPE         0x089E
#define EWPNPOWER            0x089F
#define EWPNDEAD             0x08A0
#define EWPNID               0x08A1
#define EWPNTILE             0x08A2
#define EWPNCSET             0x08A3
#define EWPNFLASHCSET        0x08A4
#define EWPNFRAMES           0x08A5
#define EWPNFRAME            0x08A6
#define EWPNASPEED           0x08A7
#define EWPNFLASH            0x08A8
#define EWPNFLIP             0x08A9
#define EWPNCOUNT            0x08AA
#define EWPNEXTEND           0x08AB
#define EWPNOTILE            0x08AC
#define EWPNOCSET            0x08AD
#define EWPNZ                0x08AE
#define EWPNJUMP             0x08AF

//#define SCRDOOR(n)         ((0x08B0)+(n)) //spare one
#define SCRDOORD             0x08B4

#define CURDSCR              0x08B5
//#define GETSTART             0x08B6 //?

#define COMBOSD	             0x08B7
#define SCREENSTATEDD        0x08B8
#define SDD                  0x08B9
#define SDDD                 0x08BA
#define GDD                  0x08BB

#define FFFLAGSD             0x08BC
#define FFCWIDTH             0x08BD
#define FFCHEIGHT            0x08BE
#define FFTWIDTH             0x08BF
#define FFTHEIGHT            0x08C0
#define FFLINK               0x08C1

//#define GS(n)              ((0x08C2)+(n)) //this never did anything anyway

#define WAVY                 0x09C2
#define QUAKE                0x09C3
#define INPUTMOUSEX          0x09C4
#define INPUTMOUSEY          0x09C5
#define INPUTMOUSEZ          0x09C6
#define INPUTMOUSEB          0x09C7

#define COMBODDM	         0x09C8
#define COMBOCDM             0x09C9
#define COMBOFDM             0x09CA
#define COMBOTDM             0x09CB
#define COMBOIDM             0x09CC
#define COMBOSDM             0x09CD

#define SCRIPTRAM            0x09CE
#define GLOBALRAM            0x09CF
#define SCRIPTRAMD           0x09D0
#define GLOBALRAMD           0x09D1
//#define SAVERAM              0x09D2 //Spare one here

#define LWPNHXOFS			 0x09D3
#define LWPNHYOFS			 0x09D4
#define LWPNXOFS			 0x09D5
#define LWPNYOFS			 0x09D6
#define LWPNZOFS			 0x09D7
#define LWPNHXSZ			 0x09D8
#define LWPNHYSZ			 0x09D9
#define LWPNHZSZ			 0x09DA

#define EWPNHXOFS			 0x09DB
#define EWPNHYOFS			 0x09DC
#define EWPNXOFS			 0x09DD
#define EWPNYOFS			 0x09DE
#define EWPNZOFS			 0x09DF
#define EWPNHXSZ			 0x09E0
#define EWPNHYSZ			 0x09E1
#define EWPNHZSZ			 0x09E2

#define NPCHXOFS			 0x09E3
#define NPCHYOFS			 0x09E4
#define NPCXOFS				 0x09E5
#define NPCYOFS				 0x09E6
#define NPCZOFS				 0x09E7
#define NPCHXSZ			     0x09E8
#define NPCHYSZ			     0x09E9
#define NPCHZSZ			     0x09EA

#define ITEMHXOFS			 0x09EB
#define ITEMHYOFS			 0x09EC
#define ITEMXOFS			 0x09ED
#define ITEMYOFS			 0x09EE
#define ITEMZOFS			 0x09EF
#define ITEMHXSZ			 0x09F0
#define ITEMHYSZ			 0x09F1
#define ITEMHZSZ			 0x09F2

#define LWPNTXSZ			 0x09F3
#define LWPNTYSZ			 0x09F4

#define EWPNTXSZ			 0x09F5
#define EWPNTYSZ			 0x09F6

#define NPCTXSZ				 0x09F7
#define NPCTYSZ				 0x09F8

#define ITEMTXSZ			 0x09F9
#define ITEMTYSZ			 0x09FA

#define LINKHXOFS			 0x09FB
#define LINKHYOFS			 0x09FC
#define LINKXOFS			 0x09FD
#define LINKYOFS			 0x09FE
#define LINKZOFS			 0x09FF
#define LINKHXSZ			 0x1000
#define LINKHYSZ			 0x1001
#define LINKHZSZ			 0x1002
#define LINKTXSZ			 0x1003
#define LINKTYSZ			 0x1004

#define LWPNBEHIND			 0x1005
#define EWPNBEHIND			 0x1006

#define SDDDD				 0x1007
#define CURLEVEL			 0x1008
#define ITEMPICKUP			 0x1009
#define LIT                  0x100A
#define INPUTEX1             0x100B
#define INPUTEX2             0x100C
#define INPUTEX3             0x100D
#define INPUTEX4             0x100E
#define INPUTPRESSSTART      0x100F
#define INPUTPRESSUP         0x1010
#define INPUTPRESSDOWN       0x1011
#define INPUTPRESSLEFT       0x1012
#define INPUTPRESSRIGHT      0x1013
#define INPUTPRESSA          0x1014
#define INPUTPRESSB          0x1015
#define INPUTPRESSL          0x1016
#define INPUTPRESSR          0x1017
#define INPUTPRESSEX1        0x1018
#define INPUTPRESSEX2        0x1019
#define INPUTPRESSEX3        0x101A
#define INPUTPRESSEX4        0x101B

#define LWPNMISCD            0x101C
//#define LWPNMISC(n)        ((0x101D)+(n)) //16 spare
#define EWPNMISCD            0x102D
//#define EWPNMISC(n)        ((0x102E)+(n)) //16 spare
#define NPCMISCD             0x103E
//#define NPCMISC(n)         ((0x103F)+(n)) //16 spare
#define ITEMMISCD            0x104F
//#define ITEMMISC(n)        ((0x1050)+(n)) //16 spare
#define FFMISCD              0x1060
//#define FFMISC(n)          ((0x1061)+(n)) //16 spare

#define GETMIDI              0x1071

#define INPUTAXISUP	 	     0x1072
#define INPUTAXISDOWN	     0x1073
#define INPUTAXISLEFT	     0x1074
#define INPUTAXISRIGHT	     0x1075
#define INPUTPRESSAXISUP	 0x1076
#define INPUTPRESSAXISDOWN   0x1077
#define INPUTPRESSAXISLEFT   0x1078
#define INPUTPRESSAXISRIGHT  0x1079

#define FFSCRIPT		     0x107A
#define SCREENFLAGSD         0x107B
//#define SCREENFLAGS(n)     ((0x107C)+(n)) //10 spare here
#define SCREENEFLAGSD        0x1086
//#define SCREENEFLAGS(n)    ((0x1087)+(n)) //3 spare here
#define FFINITDD		     0x108B
//#define FFINITD(n)		   ((0x108C)+(n)) //8 spare
#define LINKMISCD            0x1094
//#define LINKMISC(n)        ((0x1095)+(n)) //16 spare
#define DMAPFLAGSD		     0x10A5
#define EWPNCOLLDET          0x10A6

#define NPCDEFENSED	         0x10A7
#define DMAPLEVELD		     0x10A8
#define DMAPCOMPASSD         0x10A9
#define DMAPCONTINUED        0x10AA
#define DMAPMIDID            0x10AB
#define IDATAINITDD      	 0x10AC //8

#define GAMESTANDALONE       0x10AD

#define GAMEENTRSCR          0x10AE
#define GAMEENTRDMAP         0x10AF

#define GAMECLICKFREEZE      0x10B0

#define PUSHBLOCKX           0x10B1
#define PUSHBLOCKY           0x10B2
#define PUSHBLOCKCOMBO       0x10B3
#define PUSHBLOCKCSET        0x10B4
#define UNDERCOMBO           0x10B5
#define UNDERCSET            0x10B6
#define DMAPOFFSET           0x10B7
#define DMAPMAP              0x10B8

#define NUMVARIABLES         0x10B9

// Script types

#define SCRIPT_GLOBAL          0
#define SCRIPT_FFC             1
#define SCRIPT_SCREEN          2
#define SCRIPT_LINK            3
#define SCRIPT_ITEM            4
#define SCRIPT_LWPN            5
#define SCRIPT_NPC             6
#define SCRIPT_SUBSCREEN       7
#define SCRIPT_EWPN            8

struct quad3Dstruct
{
    int index;
    float pos[12], uv[8];
    int size[2], color[4];
};

struct triangle3Dstruct
{
    int index;
    float pos[9], uv[6];
    int size[2], color[3];
};

#endif

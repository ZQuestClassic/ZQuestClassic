#include "zasm_table.h"
#include "base/zdefs.h"
#include "zc/ffscript.h"
#include <optional>
#include <utility>

#define NUM      ARGTY::LITERAL
#define CMP      ARGTY::COMPARE_OP
#define REG      ARGTY::UNUSED_REG
#define REG_R    ARGTY::READ_REG
#define REG_W    ARGTY::WRITE_REG
#define REG_RW   ARGTY::READWRITE_REG

#define CMPUSED  ARGFL_COMPARE_USED
#define CMPSET   ARGFL_COMPARE_SET
#define UNIMPL   ARGFL_UNIMPL
script_command command_list[NUMCOMMANDS+1]=
{
	//name args arg1 arg2 more
	{ "SETV", 2, { REG_W, NUM }, 0, 0 },
	{ "SETR", 2, { REG_W, REG_R }, 0, 0 },
	{ "ADDR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "ADDV", 2, { REG_RW, NUM }, 0, 0 },
	{ "SUBR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SUBV", 2, { REG_RW, NUM }, 0, 0 },
	{ "MULTR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "MULTV", 2, { REG_RW, NUM }, 0, 0 },
	{ "DIVR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "DIVV", 2, { REG_RW, NUM }, 0, 0 },
	{ "WAITFRAME", 0, {}, 0, 0 },
	{ "GOTO", 1, { NUM }, 0, 0 },
	{ "CHECKTRIG", 0, {}, 0, 0 },
	{ "WARP", 2, { NUM, NUM }, 0, 0 },
	{ "COMPARER", 2, { REG_R, REG_R }, 0, CMPSET },
	{ "COMPAREV", 2, { REG_R, NUM }, 0, CMPSET },
	{ "GOTOTRUE", 1, { NUM }, 0, CMPUSED },
	{ "GOTOFALSE", 1, { NUM }, 0, CMPUSED },
	{ "GOTOLESS", 1, { NUM }, 0, CMPUSED },
	{ "GOTOMORE", 1, { NUM }, 0, CMPUSED },
	{ "LOAD1", 2, { REG_W, REG_R }, 0, 0 },
	{ "LOAD2", 2, { REG_W, REG_R }, 0, 0 },
	{ "SETA1", 2, { REG_R, REG_R }, 0, 0 },
	{ "SETA2", 2, { REG_R, REG_R }, 0, 0 },
	{ "QUIT", 0, {}, 0, 0 },
	{ "SINR", 2, { REG_W, REG_R }, 0, 0 },
	{ "SINV", 2, { REG_W, NUM }, 0, 0 },
	{ "COSR", 2, { REG_W, REG_R }, 0, 0 },
	{ "COSV", 2, { REG_W, NUM }, 0, 0 },
	{ "TANR", 2, { REG_W, REG_R }, 0, 0 },
	{ "TANV", 2, { REG_W, NUM }, 0, 0 },
	{ "MODR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "MODV", 2, { REG_RW, NUM }, 0, 0 },
	{ "ABS", 1, { REG_RW }, 0, 0 },
	{ "MINR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "MINV", 2, { REG_RW, NUM }, 0, 0 },
	{ "MAXR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "MAXV", 2, { REG_RW, NUM }, 0, 0 },
	{ "RNDR", 2, { REG_W, REG_R }, 0, 0 },
	{ "RNDV", 2, { REG_W, NUM }, 0, 0 },
	{ "FACTORIAL", 1, { REG_RW }, 0, 0 },
	{ "POWERR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "POWERV", 2, { REG_RW, NUM }, 0, 0 },
	{ "IPOWERR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "IPOWERV", 2, { REG_RW, NUM }, 0, 0 },
	{ "ANDR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "ANDV", 2, { REG_RW, NUM }, 0, 0 },
	{ "ORR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "ORV", 2, { REG_RW, NUM }, 0, 0 },
	{ "XORR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "XORV", 2, { REG_RW, NUM }, 0, 0 },
	{ "NANDR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "NANDV", 2, { REG_RW, NUM }, 0, 0 },
	{ "NORR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "NORV", 2, { REG_RW, NUM }, 0, 0 },
	{ "XNORR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "XNORV", 2, { REG_RW, NUM }, 0, 0 },
	{ "NOT", 1, { REG_RW }, 0, 0 },
	{ "LSHIFTR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "LSHIFTV", 2, { REG_RW, NUM }, 0, 0 },
	{ "RSHIFTR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "RSHIFTV", 2, { REG_RW, NUM }, 0, 0 },
	{ "TRACER", 1, { REG_R }, 0, 0 },
	{ "TRACEV", 1, { NUM }, 0, 0 },
	{ "TRACE3", 0, {}, 0, 0 },
	{ "LOOP", 2, { NUM, REG_R }, 0, 0 }, //!TODO ERROR Writes to sarg1 as a register
	{ "PUSHR", 1, { REG_R }, 0, 0 },
	{ "PUSHV", 1, { NUM }, 0, 0 },
	{ "POP", 1, { REG_W }, 0, 0 },
	{ "ENQUEUER", 2, { REG, REG }, 0, UNIMPL }, // Unimplemented
	{ "ENQUEUEV", 2, { REG, NUM }, 0, UNIMPL }, // Unimplemented
	{ "DEQUEUE", 1, { REG }, 0, UNIMPL }, // Unimplemented
	{ "PLAYSOUNDR", 1, { REG_R }, 0, 0 },
	{ "PLAYSOUNDV", 1, { NUM }, 0, 0 },
	{ "LOADLWEAPONR", 1, { REG_R }, 0, 0 },
	{ "LOADLWEAPONV", 1, { NUM }, 0, 0 },
	{ "LOADITEMR", 1, { REG_R }, 0, 0 },
	{ "LOADITEMV", 1, { NUM }, 0, 0 },
	{ "LOADNPCR", 1, { REG_R }, 0, 0 },
	{ "LOADNPCV", 1, { NUM }, 0, 0 },
	{ "CREATELWEAPONR", 1, { REG_R }, 0, 0 },
	{ "CREATELWEAPONV", 1, { NUM }, 0, 0 },
	{ "CREATEITEMR", 1, { REG_R }, 0, 0 },
	{ "CREATEITEMV", 1, { NUM }, 0, 0 },
	{ "CREATENPCR", 1, { REG_R }, 0, 0 },
	{ "CREATENPCV", 1, { NUM }, 0, 0 },
	{ "LOADI", 2, { REG_W, REG_R }, 0, 0 },
	{ "STOREI", 2, { REG_R, REG_R }, 0, 0 },
	{ "GOTOR", 1, { REG_R }, 0, 0 },
	{ "SQROOTV", 2, { REG_W, NUM }, 0, 0 },
	{ "SQROOTR", 2, { REG_W, REG_R }, 0, 0 },
	{ "CREATEEWEAPONR", 1, { REG_R }, 0, 0 },
	{ "CREATEEWEAPONV", 1, { NUM }, 0, 0 },
	{ "PITWARP", 2, { NUM, NUM }, 0, 0 },
	{ "WARPR", 2, { REG_R, REG_R }, 0, 0 },
	{ "PITWARPR", 2, { REG_R, REG_R }, 0, 0 },
	{ "CLEARSPRITESR", 1, { REG_R }, 0, 0 },
	{ "CLEARSPRITESV", 1, { NUM }, 0, 0 },
	{ "RECT", 0, {}, 0, 0 },
	{ "CIRCLE", 0, {}, 0, 0 },
	{ "ARC", 0, {}, 0, 0 },
	{ "ELLIPSE", 0, {}, 0, 0 },
	{ "LINE", 0, {}, 0, 0 },
	{ "PUTPIXEL", 0, {}, 0, 0 },
	{ "DRAWTILE", 0, {}, 0, 0 },
	{ "DRAWCOMBO", 0, {}, 0, 0 },
	{ "ELLIPSE2", 0, {}, 0, 0 },
	{ "SPLINE", 0, {}, 0, 0 },
	{ "FLOODFILL", 0, {}, 0, 0 },
	{ "COMPOUNDR", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "COMPOUNDV", 1, { NUM }, 0, UNIMPL }, // Unimplemented - no case
	{ "MSGSTRR", 1, { REG_R }, 0, 0 },
	{ "MSGSTRV", 1, { NUM }, 0, 0 },
	{ "ISVALIDITEM", 1, { REG_RW }, 0, 0 },
	{ "ISVALIDNPC", 1, { REG_RW }, 0, 0 },
	{ "PLAYMIDIR", 1, { REG_R }, 0, 0 },
	{ "PLAYMIDIV", 1, { NUM }, 0, 0 },
	{ "COPYTILEVV", 2, { NUM, NUM }, 0, 0 },
	{ "COPYTILEVR", 2, { NUM, REG_R }, 0, 0 },
	{ "COPYTILERV", 2, { REG_R, NUM }, 0, 0 },
	{ "COPYTILERR", 2, { REG_R, REG_R }, 0, 0 },
	{ "SWAPTILEVV", 2, { NUM, NUM }, 0, 0 },
	{ "SWAPTILEVR", 2, { NUM, REG_R }, 0, 0 },
	{ "SWAPTILERV", 2, { REG_R, NUM }, 0, 0 },
	{ "SWAPTILERR", 2, { REG_R, REG_R }, 0, 0 },
	{ "CLEARTILEV", 1, { NUM }, 0, 0 },
	{ "CLEARTILER", 1, { REG_R }, 0, 0 },
	{ "OVERLAYTILEVV", 2, { NUM, NUM }, 0, 0 },
	{ "OVERLAYTILEVR", 2, { NUM, REG_R }, 0, 0 },
	{ "OVERLAYTILERV", 2, { REG_R, NUM }, 0, 0 },
	{ "OVERLAYTILERR", 2, { REG_R, REG_R }, 0, 0 },
	{ "FLIPROTTILEVV", 2, { NUM, NUM }, 0, 0 },
	{ "FLIPROTTILEVR", 2, { NUM, REG_R }, 0, 0 },
	{ "FLIPROTTILERV", 2, { REG_R, NUM }, 0, 0 },
	{ "FLIPROTTILERR", 2, { REG_R, REG_R }, 0, 0 },
	{ "GETTILEPIXEL", 0, {}, 0, 0 },
	{ "NPCISFLICKERFRAME", 0, {}, 0, 0 },
	{ "SETTILEPIXEL", 0, {}, 0, 0 },
	{ "HEROISFLICKERFRAME", 0, {}, 0, 0 },
	{ "SHIFTTILEVV", 2, { NUM, NUM }, 0, 0 },
	{ "SHIFTTILEVR", 2, { NUM, REG_R }, 0, 0 },
	{ "SHIFTTILERV", 2, { REG_R, NUM }, 0, 0 },
	{ "SHIFTTILERR", 2, { REG_R, REG_R }, 0, 0 },
	{ "ISVALIDLWPN", 1, { REG_RW }, 0, 0 },
	{ "ISVALIDEWPN", 1, { REG_RW }, 0, 0 },
	{ "LOADEWEAPONR", 1, { REG_R }, 0, 0 },
	{ "LOADEWEAPONV", 1, { NUM }, 0, 0 },
	{ "ALLOCATEMEMR", 2, { REG_W, REG_R }, 0, 0 },
	{ "ALLOCATEMEMV", 3, { REG_W, NUM, NUM }, 0, 0 },
	{ "ALLOCATEGMEMV", 3, { REG_W, NUM, NUM }, 0, 0 },
	{ "DEALLOCATEMEMR", 1, { REG_R }, 0, 0 },
	{ "DEALLOCATEMEMV", 1, { NUM }, 0, 0 },
	{ "WAITDRAW", 0, {}, 0, 0 },
	{ "ARCTANR", 1, { REG_W }, 0, 0 },
	{ "LWPNUSESPRITER", 1, { REG_R }, 0, 0 },
	{ "LWPNUSESPRITEV", 1, { NUM }, 0, 0 },
	{ "EWPNUSESPRITER", 1, { REG_R }, 0, 0 },
	{ "EWPNUSESPRITEV", 1, { NUM }, 0, 0 },
	{ "LOADITEMDATAR", 1, { REG_R }, 0, 0 },
	{ "LOADITEMDATAV", 1, { NUM }, 0, 0 },
	{ "BITNOT", 1, { REG_RW }, 0, 0 },
	{ "LOG10", 1, { REG_RW }, 0, 0 },
	{ "LOGE", 1, { REG_RW }, 0, 0 },
	{ "ISSOLID", 1, { REG_W }, 0, 0 },
	{ "LAYERSCREEN", 2, { REG_W, REG_R }, 0, 0 },
	{ "LAYERMAP", 2, { REG_W, REG_R }, 0, 0 },
	{ "TRACE2R", 1, { REG_R }, 0, 0 },
	{ "TRACE2V", 1, { NUM }, 0, 0 },
	{ "TRACE4", 0, {}, 0, 0 },
	{ "TRACE5", 0, {}, 0, 0 },
	{ "SECRETS", 0, {}, 0, 0 },
	{ "DRAWCHAR", 0, {}, 0, 0 },
	{ "GETSCREENFLAGS", 1, { REG_W }, 0, 0 },
	{ "QUAD", 0, {}, 0, 0 },
	{ "TRIANGLE", 0, {}, 0, 0 },
	{ "ARCSINR", 2, { REG_W, REG_R }, 0, 0 },
	{ "ARCSINV", 2, { NUM, REG }, 0, UNIMPL }, //!TODO ERROR Writes to sarg1 as a register, reads from sarg2 as a literal
	{ "ARCCOSR", 2, { REG_W, REG_R }, 0, 0 },
	{ "ARCCOSV", 2, { NUM, REG }, 0, UNIMPL }, //!TODO ERROR Writes to sarg1 as a register, reads from sarg2 as a literal
	{ "GAMEEND", 0, {}, 0, 0 },
	{ "DRAWINT", 0, {}, 0, 0 },
	{ "SETTRUE", 1, { REG_W }, 0, CMPUSED },
	{ "SETFALSE", 1, { REG_W }, 0, CMPUSED },
	{ "SETMORE", 1, { REG_W }, 0, CMPUSED },
	{ "SETLESS", 1, { REG_W }, 0, CMPUSED },
	{ "FASTTILE", 0, {}, 0, 0 },
	{ "FASTCOMBO", 0, {}, 0, 0 },
	{ "DRAWSTRING", 0, {}, 0, 0 },
	{ "SETSIDEWARP", 0, {}, 0, 0 },
	{ "SAVE", 0, {}, 0, 0 },
	{ "TRACE6", 0, {}, 0, 0 },
	{ "DEPRECATED", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "QUAD3D", 0, {}, 0, 0 },
	{ "TRIANGLE3D", 0, {}, 0, 0 },
	{ "SETCOLORB", 0, {}, 0, 0 },
	{ "SETDEPTHB", 0, {}, 0, 0 },
	{ "GETCOLORB", 0, {}, 0, 0 },
	{ "GETDEPTHB", 0, {}, 0, 0 },
	{ "COMBOTILE", 2, { REG_W, REG_R }, 0, 0 },
	{ "SETTILEWARP", 0, {}, 0, 0 },
	{ "GETSCREENEFLAGS", 1, { REG_W }, 0, 0 },
	{ "GETSAVENAME", 1, { REG_R }, 0, 0 },
	{ "ARRAYSIZE", 1, { REG_R }, 0, 0 },
	{ "ITEMNAME", 1, { REG_R }, 0, 0 },
	{ "SETSAVENAME", 1, { REG_R }, 0, 0 },
	{ "NPCNAME", 1, { REG_R }, 0, 0 },
	{ "GETMESSAGE", 2, { REG_R, REG_R }, 0, 0 },
	{ "GETDMAPNAME", 2, { REG_R, REG_R }, 0, 0 },
	{ "GETDMAPTITLE", 2, { REG_R, REG_R }, 0, 0 },
	{ "GETDMAPINTRO", 2, { REG_R, REG_R }, 0, 0 },
	{ "ALLOCATEGMEMR", 2, { REG_W, REG_R }, 0, 0 },
	{ "DRAWBITMAP", 0, {}, 0, 0 },
	{ "SETRENDERTARGET", 0, {}, 0, 0 },
	{ "PLAYENHMUSIC", 2, { REG_R, REG_RW }, 0, 0 },
	{ "GETMUSICFILE", 2, { REG_R, REG_R }, 0, 0 },
	{ "GETMUSICTRACK", 1, { REG_RW }, 0, 0 },
	{ "SETDMAPENHMUSIC", 0, {}, 0, 0 },
	{ "DRAWLAYER", 0, {}, 0, 0 },
	{ "DRAWSCREEN", 0, {}, 0, 0 },
	{ "BREAKSHIELD", 1, { REG_R }, 0, 0 },
	{ "SAVESCREEN", 1, { REG_W }, 0, 0 },
	{ "SAVEQUITSCREEN", 0, {}, 0, 0 },
	{ "SELECTAWPNR", 1, { REG_R }, 0, 0 },
	{ "SELECTAWPNV", 1, { NUM }, 0, 0 },
	{ "SELECTBWPNR", 1, { REG_R }, 0, 0 },
	{ "SELECTBWPNV", 1, { NUM }, 0, 0 },
	{ "GETSIDEWARPDMAP", 1, { REG_RW }, 0, 0 },
	{ "GETSIDEWARPSCR", 1, { REG_RW }, 0, 0 },
	{ "GETSIDEWARPTYPE", 1, { REG_RW }, 0, 0 },
	{ "GETTILEWARPDMAP", 1, { REG_RW }, 0, 0 },
	{ "GETTILEWARPSCR", 1, { REG_RW }, 0, 0 },
	{ "GETTILEWARPTYPE", 1, { REG_RW }, 0, 0 },
	{ "GETFFCSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "BITMAPEXR", 0, {}, 0, 0 },
	{ "__RESERVED_FOR_QUAD2R", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "WAVYIN", 0, {}, 0, 0 },
	{ "WAVYOUT", 0, {}, 0, 0 },
	{ "ZAPIN", 0, {}, 0, 0 },
	{ "ZAPOUT", 0, {}, 0, 0 },
	{ "OPENWIPE", 0, {}, 0, 0 },
	{ "FREE0x00F1", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "FREE0x00F2", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "FREE0x00F3", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "SETMESSAGE", 2, { REG_R, REG_R }, 0, 0 },
	{ "SETDMAPNAME", 2, { REG_R, REG_R }, 0, 0 },
	{ "SETDMAPTITLE", 2, { REG_R, REG_R }, 0, 0 },
	{ "SETDMAPINTRO", 2, { REG_R, REG_R }, 0, 0 },
	{ "GREYSCALEON", 0, {}, 0, 0 },
	{ "GREYSCALEOFF", 0, {}, 0, 0 },
	{ "ENDSOUNDR", 1, { REG_R }, 0, 0 },
	{ "ENDSOUNDV", 1, { NUM }, 0, 0 },
	{ "PAUSESOUNDR", 1, { REG_R }, 0, 0 },
	{ "PAUSESOUNDV", 1, { NUM }, 0, 0 },
	{ "RESUMESOUNDR", 1, { REG_R }, 0, 0 },
	{ "RESUMESOUNDV", 1, { NUM }, 0, 0 },
	{ "PAUSEMUSIC", 0, {}, 0, 0 },
	{ "RESUMEMUSIC", 0, {}, 0, 0 },
	{ "LWPNARRPTR", 1, { REG_RW }, 0, 0 },
	{ "EWPNARRPTR", 1, { REG_RW }, 0, 0 },
	{ "EWPNARRPTR", 1, { REG_RW }, 0, 0 },
	{ "IDATAARRPTR", 1, { REG_RW }, 0, 0 },
	{ "FFCARRPTR", 1, { REG_RW }, 0, 0 },
	{ "BOOLARRPTR", 1, { REG_RW }, 0, 0 },
	{ "BOOLARRPTR", 1, { REG_RW }, 0, 0 },
	{ "LWPNARRPTR2", 1, { REG_RW }, 0, 0 },
	{ "EWPNARRPTR2", 1, { REG_RW }, 0, 0 },
	{ "ITEMARRPTR2", 1, { REG_RW }, 0, 0 },
	{ "IDATAARRPTR2", 1, { REG_RW }, 0, 0 },
	{ "FFCARRPTR2", 1, { REG_RW }, 0, 0 },
	{ "BOOLARRPTR2", 1, { REG_RW }, 0, 0 },
	{ "NPCARRPTR2", 1, { REG_RW }, 0, 0 },
	{ "ARRAYSIZEB", 1, { REG_R }, 0, 0 },
	{ "ARRAYSIZEF", 1, { REG_R }, 0, 0 },
	{ "ARRAYSIZEN", 1, { REG_R }, 0, 0 },
	{ "ARRAYSIZEL", 1, { REG_R }, 0, 0 },
	{ "ARRAYSIZEE", 1, { REG_R }, 0, 0 },
	{ "ARRAYSIZEI", 1, { REG_R }, 0, 0 },
	{ "ARRAYSIZEID", 1, { REG_R }, 0, 0 },
	{ "POLYGONR", 0, {}, 0, 0 },
	{ "__RESERVED_FOR_POLYGON3DR", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "__RESERVED_FOR_SETRENDERSOURCE", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "LINESARRAY", 0, {}, 0, 0 },
	{ "PIXELARRAYR", 0, {}, 0, 0 },
	{ "TILEARRAYR", 0, {}, 0, 0 },
	{ "COMBOARRAYR", 0, {}, 0, 0 },
	{ "RES0000", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES0001", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES0002", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES0003", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES0004", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES0005", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES0006", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES0007", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES0008", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES0009", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES000A", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES000B", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES000C", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES000D", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES000E", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "RES000F", 0, {}, 0, UNIMPL }, // Unimplemented - no case
	{ "__RESERVED_FOR_CREATELWPN2VV", 2, { NUM, NUM }, 0, UNIMPL }, // Unimplemented - no case
	{ "__RESERVED_FOR_CREATELWPN2VR", 2, { NUM, REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "__RESERVED_FOR_CREATELWPN2RV", 2, { REG, NUM }, 0, UNIMPL }, // Unimplemented - no case
	{ "__RESERVED_FOR_CREATELWPN2RR", 2, { REG, REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "GETSCREENDOOR", 1, { REG_W }, 0, 0 },
	{ "GETSCREENENEMY", 1, { REG_W }, 0, 0 },
	{ "PAUSESFX", 1, { REG }, 0, 0 },
	{ "RESUMESFX", 1, { REG }, 0, 0 },
	{ "CONTINUESFX", 1, { REG }, 0, 0 },
	{ "ADJUSTSFX", 0, {}, 0, 0 },
	{ "GETITEMSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "GETSCREENLAYOP", 1, { REG_W }, 0, 0 },
	{ "GETSCREENSECCMB", 1, { REG_W }, 0, 0 },
	{ "GETSCREENSECCST", 1, { REG_W }, 0, 0 },
	{ "GETSCREENSECFLG", 1, { REG_W }, 0, 0 },
	{ "GETSCREENLAYMAP", 1, { REG_W }, 0, 0 },
	{ "GETSCREENLAYSCR", 1, { REG_W }, 0, 0 },
	{ "GETSCREENPATH", 1, { REG_W }, 0, 0 },
	{ "GETSCREENWARPRX", 1, { REG_W }, 0, 0 },
	{ "GETSCREENWARPRY", 1, { REG_W }, 0, 0 },
	{ "TRIGGERSECRETR", 1, { REG_R }, 0, 0 },
	{ "TRIGGERSECRETV", 1, { NUM }, 0, 0 },
	{ "NIL_UNUSED_1", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "NIL_UNUSED_2", 1, { NUM }, 0, UNIMPL }, // Unimplemented - no case
	//NPCData
	//one input, one return
	{ "GETNPCDATAFLAGS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAFLAGS2", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAWIDTH", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAHEIGHT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATASTILE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATASWIDTH", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATASHEIGHT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAETILE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAEWIDTH", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAHP", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAFAMILY", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATACSET", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAANIM", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAEANIM", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAFRAMERATE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAEFRAMERATE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATATOUCHDMG", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAWPNDAMAGE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAWEAPON", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATARANDOM", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAHALT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATASTEP", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAHOMING", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAHUNGER", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATADROPSET", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATABGSFX", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATADEATHSFX", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAXOFS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAYOFS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAZOFS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAHXOFS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAHYOFS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAHITWIDTH", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAHITHEIGHT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAHITZ", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATATILEWIDTH", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATATILEHEIGHT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETNPCDATAWPNSPRITE", 2, { REG_W, REG_R }, 0, 0 },
	//two inputs one return
	{ "GETNPCDATASCRIPTDEF", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "GETNPCDATADEFENSE", 1, { REG_W }, 0, 0 },
	{ "GETNPCDATASIZEFLAG", 1, { REG_W }, 0, 0 },
	{ "GETNPCDATAATTRIBUTE", 1, { REG_W }, 0, 0 },
	//two inputs no return
	{ "SETNPCDATAFLAGS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAFLAGS2", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAWIDTH", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHEIGHT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATASTILE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATASWIDTH", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATASHEIGHT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAETILE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAEWIDTH", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHP", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAFAMILY", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATACSET", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAANIM", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAEANIM", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAFRAMERATE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAEFRAMERATE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATATOUCHDMG", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAWPNDAMAGE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAWEAPON", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATARANDOM", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHALT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATASTEP", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHOMING", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHUNGER", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATADROPSET", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATABGSFX", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATADEATHSFX", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAXOFS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAYOFS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAZOFS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHXOFS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHYOFS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHITWIDTH", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHITHEIGHT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHITZ", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATATILEWIDTH", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATATILEHEIGHT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAWPNSPRITE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAHITSFX", 2, { REG_RW, REG_R }, 0, 0 },
	{ "GETNPCDATAHITSFX", 2, { REG_RW, REG_R }, 0, 0 },
	//Combodata, one input no return
	{ "GCDBLOCKENEM", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDBLOCKHOLE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDBLOCKTRIG", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDCONVEYSPDX", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDCONVEYSPDY", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDCREATEENEM", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDCREATEENEMWH", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDCREATEENEMCH", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDDIRCHTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDDISTCHTILES", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDDIVEITEM", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDDOCK", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDFAIRY", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDFFCOMBOATTRIB", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDFOOTDECOTILE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDFOOTDECOTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDHOOKSHOTGRAB", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDLADDERPASS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDLOCKBLOCKTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDLOCKBLOCKCHANGE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDMAGICMIRRORTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDMODIFYHPAMOUNT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDMODIFYHPDELAY", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDMODIFYHPTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDMODIFYMPAMOUNT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDMODIFYMPDELAY", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDMODIFYMPTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDNOPUSHBLOCKS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDOVERHEAD", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDPLACEENEMY", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDPUSHDIR", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDPUSHWEIGHT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDPUSHWAIT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDPUSHED", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDRAFT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDRESETROOM", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSAVEPOINT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSCREENFREEZE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSECRETCOMBO", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSINGULAR", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSLOWMOVE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSTATUE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSTEPTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSTEPCHANGETO", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSTRIKEREMNANTS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSTRIKEREMNANTSTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSTRIKECHANGE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSTRIKECHANGEITEM", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDTOUCHITEM", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDTOUCHSTAIRS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDTRIGGERTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDTRIGGERSENS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDWARPTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDWARPSENS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDWARPDIRECT", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDWARPLOCATION", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDWATER", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDWHISTLE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDWINGAME", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDBLOCKWEAPLVL", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDTILE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDFLIP", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDWALK", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDTYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDCSETS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDFOO", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDFRAMES", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSPEED", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDNEXTCOMBO", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDNEXTCSET", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDFLAG", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSKIPANIM", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDNEXTTIMER", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDSKIPANIMY", 2, { REG_W, REG_R }, 0, 0 },
	{ "GCDANIMFLAGS", 2, { REG_W, REG_R }, 0, 0 },
	//combodata two input, one return
	{ "GCDBLOCKWEAPON", 1, { REG_W }, 0, 0 },
	{ "GCDEXPANSION", 1, { REG_W }, 0, 0 },
	{ "GCDSTRIKEWEAPONS", 1, { REG_W }, 0, 0 },
	//combodata two input, one return
	{ "SCDBLOCKENEM", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDBLOCKHOLE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDBLOCKTRIG", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDCONVEYSPDX", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDCONVEYSPDY", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDCREATEENEM", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDCREATEENEMWH", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDCREATEENEMCH", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDDIRCHTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDDISTCHTILES", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDDIVEITEM", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDDOCK", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDFAIRY", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDFFCOMBOATTRIB", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDFOOTDECOTILE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDFOOTDECOTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDHOOKSHOTGRAB", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDLADDERPASS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDLOCKBLOCKTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDLOCKBLOCKCHANGE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDMAGICMIRRORTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDMODIFYHPAMOUNT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDMODIFYHPDELAY", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDMODIFYHPTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDMODIFYMPAMOUNT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDMODIFYMPDELAY", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDMODIFYMPTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDNOPUSHBLOCKS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDOVERHEAD", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDPLACEENEMY", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDPUSHDIR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDPUSHWEIGHT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDPUSHWAIT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDPUSHED", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDRAFT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDRESETROOM", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSAVEPOINT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSCREENFREEZE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSECRETCOMBO", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSINGULAR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSLOWMOVE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSTATUE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSTEPTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSTEPCHANGETO", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSTRIKEREMNANTS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSTRIKEREMNANTSTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSTRIKECHANGE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSTRIKECHANGEITEM", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDTOUCHITEM", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDTOUCHSTAIRS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDTRIGGERTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDTRIGGERSENS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDWARPTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDWARPSENS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDWARPDIRECT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDWARPLOCATION", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDWATER", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDWHISTLE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDWINGAME", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDBLOCKWEAPLVL", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDTILE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDFLIP", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDWALK", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDTYPE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDCSETS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDFOO", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDFRAMES", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSPEED", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDNEXTCOMBO", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDNEXTCSET", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDFLAG", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSKIPANIM", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDNEXTTIMER", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDSKIPANIMY", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SCDANIMFLAGS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "GETNPCDATATILE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "GETNPCDATAEHEIGHT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATATILE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETNPCDATAEHEIGHT", 2, { REG_RW, REG_R }, 0, 0 },
	{ "GETSPRITEDATASTRING", 2, { REG, REG }, 0, UNIMPL }, // Unimplemented - no case
	//SpriteData
	{ "GETSPRITEDATATILE", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETSPRITEDATAMISC", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETSPRITEDATACGETS", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETSPRITEDATAFRAMES", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETSPRITEDATASPEED", 2, { REG_W, REG_R }, 0, 0 },
	{ "GETSPRITEDATATYPE", 2, { REG_W, REG_R }, 0, 0 },
	{ "SETSPRITEDATASTRING", 2, { REG, REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "SETSPRITEDATATILE", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETSPRITEDATAMISC", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETSPRITEDATACSETS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETSPRITEDATAFRAMES", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETSPRITEDATASPEED", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SETSPRITEDATATYPE", 2, { REG_RW, REG_R }, 0, 0 },
	//Game->SetContinueScreenSetting
	{ "SETCONTINUESCREEN", 2, { REG_R, REG_R }, 0, 0 },
	//Game->SetContinueScreenString
	{ "SETCONTINUESTRING", 2, { REG_RW, REG_R }, 0, 0 },
	
	{ "LOADNPCDATAR", 1, { REG_R }, 0, 0 },
	{ "LOADNPCDATAV", 1, { NUM }, 0, 0 },
	
	{ "LOADCOMBODATAR", 1, { REG_R }, 0, 0 },
	{ "LOADCOMBODATAV", 1, { NUM }, 0, 0 },
	
	{ "LOADMAPDATAR", 1, { REG_R }, 0, 0 },
	{ "LOADMAPDATAV", 1, { NUM }, 0, 0 },
	
	{ "LOADSPRITEDATAR", 1, { REG_R }, 0, 0 },
	{ "LOADSPRITEDATAV", 1, { NUM }, 0, 0 },
 
	{ "LOADSCREENDATAR", 1, { REG_R }, 0, 0 },
	{ "LOADSCREENDATAV", 1, { NUM }, 0, 0 },

	{ "LOADBITMAPDATAR", 1, { REG_R }, 0, 0 },
	{ "LOADBITMAPDATAV", 1, { NUM }, 0, 0 },
	
	{ "LOADSHOPR", 1, { REG_R }, 0, 0 },
	{ "LOADSHOPV", 1, { NUM }, 0, 0 },

	{ "LOADINFOSHOPR", 1, { REG_R }, 0, 0 },
	{ "LOADINFOSHOPV", 1, { NUM }, 0, 0 },
	
	{ "LOADMESSAGEDATAR", 1, { REG_R }, 0, 0 },
	{ "LOADMESSAGEDATAV", 1, { NUM }, 0, 0 },
	{ "MESSAGEDATASETSTRINGR", 1, { REG_R }, 0, 0 },
	{ "MESSAGEDATASETSTRINGV", 1, { NUM }, 0, 0 },
	{ "MESSAGEDATAGETSTRINGR", 1, { REG_R }, 0, 0 },
	{ "MESSAGEDATAGETSTRINGV", 1, { NUM }, 0, 0 },
	
	{ "LOADDMAPDATAR", 1, { REG_R }, 0, 0 },
	{ "LOADDMAPDATAV", 1, { NUM }, 0, 0 },
	{ "DMAPDATAGETNAMER", 1, { REG_R }, 0, 0 },
	{ "DMAPDATAGETNAMEV", 1, { NUM }, 0, 0 },
	{ "DMAPDATASETNAMER", 1, { REG_R }, 0, 0 },
	{ "DMAPDATASETNAMEV", 1, { NUM }, 0, 0 },
	{ "DMAPDATAGETTITLER", 1, { REG_R }, 0, 0 },
	{ "DMAPDATAGETTITLEV", 1, { NUM }, 0, 0 },
	{ "DMAPDATASETTITLER", 1, { REG_R }, 0, 0 },
	{ "DMAPDATASETTITLEV", 1, { NUM }, 0, 0 },
	
	{ "DMAPDATAGETINTROR", 1, { REG_R }, 0, 0 },
	{ "DMAPDATAGETINTROV", 1, { NUM }, 0, 0 },
	{ "DMAPDATANSETITROR", 1, { REG_R }, 0, 0 },
	{ "DMAPDATASETINTROV", 1, { NUM }, 0, 0 },
	{ "DMAPDATAGETMUSICR", 1, { REG_R }, 0, 0 },
	{ "DMAPDATAGETMUSICV", 1, { NUM }, 0, 0 },
	{ "DMAPDATASETMUSICR", 1, { REG_R }, 0, 0 },
	{ "DMAPDATASETMUSICV", 1, { NUM }, 0, 0 },
	
	{ "ADJUSTSFXVOLUMER", 1, { REG_R }, 0, 0 },
	{ "ADJUSTSFXVOLUMEV", 1, { NUM }, 0, 0 },
	
	{ "ADJUSTVOLUMER", 1, { REG_R }, 0, 0 },
	{ "ADJUSTVOLUMEV", 1, { NUM }, 0, 0 },
	
	{ "FXWAVYR", 1, { REG_R }, 0, 0 },
	{ "FXWAVYV", 1, { NUM }, 0, 0 },
	
	{ "FXZAPR", 1, { REG_R }, 0, 0 },
	{ "FXZAPV", 1, { NUM }, 0, 0 },
	
	{ "GREYSCALER", 1, { REG_R }, 0, 0 },
	{ "GREYSCALEV", 1, { NUM }, 0, 0 },
	{ "RETURN", 0, {}, 0, 0 },
	{ "MONOCHROMER", 1, { REG_R }, 0, 0 },
	{ "MONOCHROMEV", 1, { NUM }, 0, 0 },
	{ "CLEARTINT", 0, {}, 0, 0 },
	{ "TINT", 0, {}, 0, 0 },
	{ "MONOHUE", 0, {}, 0, 0 },
	
	{ "BMPRECTR", 0, {}, 0, 0 },
	{ "BMPCIRCLER", 0, {}, 0, 0 },
	{ "BMPARCR", 0, {}, 0, 0 },
	{ "BMPELLIPSER", 0, {}, 0, 0 },
	{ "BMPLINER", 0, {}, 0, 0 },
	{ "BMPSPLINER", 0, {}, 0, 0 },
	{ "BMPPUTPIXELR", 0, {}, 0, 0 },
	{ "BMPDRAWTILER", 0, {}, 0, 0 },
	{ "BMPDRAWCOMBOR", 0, {}, 0, 0 },
	{ "BMPFASTTILER", 0, {}, 0, 0 },
	{ "BMPFASTCOMBOR", 0, {}, 0, 0 },
	{ "BMPDRAWCHARR", 0, {}, 0, 0 },
	{ "BMPDRAWINTR", 0, {}, 0, 0 },
	{ "BMPDRAWSTRINGR", 0, {}, 0, 0 },
	{ "BMPQUADR", 0, {}, 0, 0 },
	{ "BMPQUAD3DR", 0, {}, 0, 0 },
	{ "BMPTRIANGLER", 0, {}, 0, 0 },
	{ "BMPTRIANGLE3DR", 0, {}, 0, 0 },
	{ "BMPPOLYGONR", 0, {}, 0, 0 },
	{ "BMPDRAWLAYERR", 0, {}, 0, 0 },
	{ "BMPDRAWSCREENR", 0, {}, 0, 0 },
	{ "BMPBLIT", 0, {}, 0, 0 },
	
	{ "LINKWARPEXR", 1, { REG_R }, 0, 0 },
	{ "LINKWARPEXV", 1, { NUM }, 0, 0 },
	{ "LINKEXPLODER", 1, { REG_R }, 0, 0 },
	{ "LINKEXPLODEV", 1, { NUM }, 0, 0 },
	{ "NPCEXPLODER", 1, { REG_R }, 0, 0 },
	{ "NPCEXPLODEV", 1, { NUM }, 0, 0 },
	
	{ "ITEMEXPLODER", 1, { REG_R }, 0, 0 },
	{ "ITEMEXPLODEV", 1, { NUM }, 0, 0 },
	{ "LWEAPONEXPLODER", 1, { REG_R }, 0, 0 },
	{ "LWEAPONEXPLODEV", 1, { NUM }, 0, 0 },
	{ "EWEAPONEXPLODER", 1, { REG_R }, 0, 0 },
	{ "EWEAPONEXPLODEV", 1, { NUM }, 0, 0 },
	{ "RUNITEMSCRIPT", 1, { REG_R }, 0, 0 },
	{ "GETRTCTIMER", 1, { REG_RW }, 0, 0 },
	{ "GETRTCTIMEV", 1, { NUM }, 0, UNIMPL }, //!TODO ERROR Reads from and writes to sarg1 as a register
	
	//new npc functions for npc scripts
	{ "NPCDEAD", 1, { REG_W }, 0, 0 },
	{ "NPCKICKBUCKET", 0, {}, 0, 0 },
	{ "NPCSTOPBGSFX", 0, {}, 0, 0 },
	{ "NPCCANMOVE", 1, { REG_RW }, 0, 0 },
	{ "NPCNEWDIR8", 0, {}, 0, 0 },
	{ "NPCNEWDIR", 0, {}, 0, 0 },
	{ "NPCCONSTWALK", 0, {}, 0, 0 },
	{ "NPCCONSTWALK8", 0, {}, 0, 0 },
	{ "NPCVARWALK", 0, {}, 0, 0 },
	{ "NPCVARWALK8", 0, {}, 0, 0 },
	{ "NPCHALTWALK", 0, {}, 0, 0 },
	{ "NPCHALTWALK8", 0, {}, 0, 0 },
	{ "NPCFLOATWALK", 0, {}, 0, 0 },
	// moved to a var: { "NPCLINEDUP", 0, {}, 0, 0 },
	{ "NPCLINKINRANGE", 1, { REG_RW }, 0, 0 }, //!TODO ERROR Writes the unused sarg2 as a register
	{ "NPCATTACK", 0, {}, 0, 0 },
	{ "NPCPLACEONAXIS", 0, {}, 0, 0 },
	{ "NPCADD", 1, { REG_R }, 0, 0 },
	{ "NPCFIREBREATH", 0, {}, 0, 0 },
	{ "NPCCANSLIDE", 1, { REG_W }, 0, 0 },
	{ "NPCSLIDE", 1, { REG_W }, 0, 0 },
	{ "NPCHITWITH", 1, { REG_RW }, 0, 0 },
	{ "NPCGETINITDLABEL", 0, {}, 0, 0 },
	// moved to a var: { "NPCCOLLISION", 0, {}, 0, 0 }, //how to implement this?
	{ "GAMECONTINUE", 0, {}, 0, 0 },
	{ "MAPDATAISSOLID", 1, { REG_W }, 0, 0 },
	{ "SHOWF6SCREEN", 0, {}, 0, 0 },
	{ "NPCDATAGETNAME", 1, { REG_R }, 0, 0 },
	{ "PLAYENHMUSICEX", 2, { REG_R, REG_RW }, 0, 0 },
	{ "GETENHMUSICPOS", 1, { REG_W }, 0, 0 },
	{ "SETENHMUSICPOS", 1, { REG_R }, 0, 0 },
	{ "SETENHMUSICSPEED", 1, { REG_R }, 0, 0 },
	{ "ISVALIDBITMAP", 1, { REG_RW }, 0, 0 },
	{ "READBITMAP", 0, {}, 0, 0 },
	{ "WRITEBITMAP", 0, {}, 0, 0 },
	{ "ALLOCATEBITMAP", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "CLEARBITMAP", 0, {}, 0, 0 },
	{ "REGENERATEBITMAP", 0, {}, 0, 0 },
	{ "BMPBLITTO", 0, {}, 0, 0 },
	
	{ "BMPDRAWSCREENSOLIDR", 0, {}, 0, 0 },
	{ "BMPDRAWSCREENCOMBOFR", 0, {}, 0, 0 },
	{ "BMPDRAWSCREENCOMBOIR", 0, {}, 0, 0 },
	{ "BMPDRAWSCREENCOMBOTR", 0, {}, 0, 0 },
	{ "BMPDRAWSCREENSOLID2R", 0, {}, 0, 0 },
	{ "GRAPHICSGETPIXEL", 1, { REG_W }, 0, 0 },
	
	{ "BMPDRAWLAYERSOLIDR", 0, {}, 0, 0 },
	{ "BMPDRAWLAYERCFLAGR", 0, {}, 0, 0 },
	{ "BMPDRAWLAYERCTYPER", 0, {}, 0, 0 },
	{ "BMPDRAWLAYERCIFLAGR", 0, {}, 0, 0 },
	{ "BMPDRAWLAYERSOLIDITYR", 0, {}, 0, 0 },
	{ "BMPMODE7", 0, {}, 0, 0 },
	{ "BITMAPGETPIXEL", 0, {}, 0, 0 },
	{ "NOP", 0, {}, 0, 0 },
	{ "STRINGCOMPARE", 1, { REG_W }, 0, 0 },
	{ "STRINGNCOMPARE", 1, { REG_W }, 0, 0 },
	{ "STRINGLENGTH", 2, { REG_W, REG_R }, 0, 0 },
	{ "STRINGCOPY", 2, { REG_R, REG_R }, 0, 0 },
	{ "CASTBOOLI", 1, { REG_RW }, 0, 0 },
	{ "CASTBOOLF", 1, { REG_RW }, 0, 0 },
	{ "SETTRUEI", 1, { REG_W }, 0, CMPUSED },
	{ "SETFALSEI", 1, { REG_W }, 0, CMPUSED },
	{ "SETMOREI", 1, { REG_W }, 0, CMPUSED },
	{ "SETLESSI", 1, { REG_W }, 0, CMPUSED },
	
	{ "ARRAYCOPY", 2, { REG_R, REG_R }, 0, 0 },
	{ "ARRAYNCOPY", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	
	//1 INPUT, NO RETURN
	{ "REMCHR", 2, { REG, REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRINGUPPERLOWER", 2, { REG, REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRINGLOWERUPPER", 2, { REG, REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRINGCONVERTCASE", 2, { REG, REG }, 0, UNIMPL }, // Unimplemented - no case
	
	//1 input, 1 ret
	{ "XLEN", 2, { REG, REG_R }, 0, UNIMPL }, // Unimplemented
	{ "XTOI", 2, { REG_W, REG_R }, 0, 0 },
	{ "ILEN", 2, { REG_W, REG_R }, 0, 0 },
	{ "ATOI", 2, { REG_W, REG_R }, 0, 0 },
 
	//2 INPUT, 1 RET, based on strcmp
	{ "STRCSPN", 1, { REG_W }, 0, 0 },
	{ "STRSTR", 1, { REG_W }, 0, 0 },
	{ "XTOA", 2, { REG_RW, REG_R }, 0, 0 },
	{ "ITOA", 2, { REG_RW, REG_R }, 0, 0 },
	{ "STRCAT", 1, { REG_W }, 0, 0 },
	{ "STRSPN", 1, { REG_W }, 0, 0 },
	{ "STRCHR", 1, { REG_W }, 0, 0 },
	{ "STRRCHR", 1, { REG_W }, 0, 0 },
	//2 INP, 1 RET OVERLOADS
	{ "XLEN2", 1, { REG }, 0, UNIMPL }, // Unimplemented
	{ "XTOI2", 1, { REG_W }, 0, 0 },
	{ "ILEN2", 1, { REG }, 0, UNIMPL }, // Unimplemented
	{ "ATOI2", 1, { REG }, 0, UNIMPL }, // Unimplemented
	{ "REMCHR2", 1, { REG }, 0, UNIMPL }, // Unimplemented
	
	//3 INPUT 1 RET
	{ "XTOA3", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRCATF", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "ITOA3", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRSTR3", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "REMNCHR3", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRCAT3", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRNCAT3", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRCHR3", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRRCHR3", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRSPN3", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	{ "STRCSPN3", 1, { REG }, 0, UNIMPL }, // Unimplemented - no case
	
	{ "UPPERTOLOWER", 2, { REG_RW, REG }, 0, 0 },
	{ "LOWERTOUPPER", 2, { REG_RW, REG }, 0, 0 },
	{ "CONVERTCASE", 2, { REG_RW, REG }, 0, 0 },
	//Game->Get
	{ "GETNPCSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "GETLWEAPONSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "GETEWEAPONSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "GETHEROSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "GETGLOBALSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "GETDMAPSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "GETSCREENSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "GETSPRITESCRIPT", 1, { REG_RW }, 0, 0 },
	{ "GETUNTYPEDSCRIPT", 1, { REG_W }, 0, 0 },
	{ "GETSUBSCREENSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "GETNPCBYNAME", 1, { REG_RW }, 0, 0 },
	{ "GETITEMBYNAME", 1, { REG_RW }, 0, 0 },
	{ "GETCOMBOBYNAME", 1, { REG_RW }, 0, 0 },
	{ "GETDMAPBYNAME", 1, { REG_RW }, 0, 0 },
	
	{ "SRNDR", 1, { REG_R }, 0, 0 },
	{ "SRNDV", 1, { NUM }, 0, 0 },
	{ "SRNDRND", 1, { REG_W }, 0, 0 },
	{ "SAVEGAMESTRUCTS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "READGAMESTRUCTS", 2, { REG_RW, REG_R }, 0, 0 },
	{ "ANDR32", 2, { REG_RW, REG_R }, 0, 0 },
	{ "ANDV32", 2, { REG_RW, NUM }, 0, 0 },
	{ "ORR32", 2, { REG_RW, REG_R }, 0, 0 },
	{ "ORV32", 2, { REG_RW, NUM }, 0, 0 },
	{ "XORR32", 2, { REG_RW, REG_R }, 0, 0 },
	{ "XORV32", 2, { REG_RW, NUM }, 0, 0 },
	{ "BITNOT32", 1, { REG_RW }, 0, 0 },
	{ "LSHIFTR32", 2, { REG_RW, REG_R }, 0, 0 },
	{ "LSHIFTV32", 2, { REG_RW, NUM }, 0, 0 },
	{ "RSHIFTR32", 2, { REG_RW, REG_R }, 0, 0 },
	{ "RSHIFTV32", 2, { REG_RW, NUM }, 0, 0 },
	{ "ISALLOCATEDBITMAP", 1, { REG_RW }, 0, 0 },
	{ "FONTHEIGHTR", 1, { REG_R }, 0, 0 },
	{ "STRINGWIDTHR", 2, { REG_R, REG_R }, 0, 0 },
	{ "CHARWIDTHR", 2, { REG_R, REG_R }, 0, 0 },
	{ "MESSAGEWIDTHR", 1, { REG_R }, 0, 0 },
	{ "MESSAGEHEIGHTR", 1, { REG_R }, 0, 0 },
	{ "ISVALIDARRAY", 1, { REG_RW }, 0, 0 },
	{ "DIREXISTS", 1, { REG_RW }, 0, 0 },
	{ "GAMESAVEQUIT", 0, {}, 0, 0 },
	{ "GAMESAVECONTINUE", 0, {}, 0, 0 },
	{ "DRAWTILECLOAKEDR", 0, {}, 0, 0 },
	{ "BMPDRAWTILECLOAKEDR", 0, {}, 0, 0 },
	{ "DRAWCOMBOCLOAKEDR", 0, {}, 0, 0 },
	{ "BMPDRAWCOMBOCLOAKEDR", 0, {}, 0, 0 },
	{ "NPCKNOCKBACK", 2, { REG_RW, REG_R }, 0, 0 },
	{ "CLOSEWIPE", 0, {}, 0, 0 },
	{ "OPENWIPESHAPE", 1, { REG_R }, 0, 0 },
	{ "CLOSEWIPESHAPE", 1, { REG_R }, 0, 0 },
	{ "FILEEXISTS", 1, { REG_RW }, 0, 0 },
	{ "BITMAPCLEARTOCOLOR", 0, {}, 0, 0 },
	{ "LOADNPCBYSUID", 1, { REG_R }, 0, 0 },
	{ "LOADLWEAPONBYSUID", 1, { REG_R }, 0, 0 },
	{ "LOADWEAPONCBYSUID", 1, { REG_R }, 0, 0 },
	{ "LOADDROPSETR", 1, { REG_R }, 0, 0 },
	{ "LOADTMPSCR", 1, { REG_RW }, 0, 0 },
	{ "LOADSCROLLSCR", 1, { REG_RW }, 0, 0 },
	{ "MAPDATAISSOLIDLYR", 1, { REG_W }, 0, 0 },
	{ "ISSOLIDLAYER", 1, { REG_W }, 0, 0 },
	{ "BREAKPOINT", 1, { REG_R }, 0, 0 },
	{ "TOBYTE", 1, { REG_RW }, 0, 0 },
	{ "TOWORD", 1, { REG_RW }, 0, 0 },
	{ "TOSHORT", 1, { REG_RW }, 0, 0 },
	{ "TOSIGNEDBYTE", 1, { REG_RW }, 0, 0 },
	{ "TOINTEGER", 1, { REG_RW }, 0, 0 },
	{ "FLOOR", 1, { REG_RW }, 0, 0 },
	{ "CEILING", 1, { REG_RW }, 0, 0 },
	
	{ "FILECLOSE", 0, {}, 0, 0 },
	{ "FILEFREE", 0, {}, 0, 0 },
	{ "FILEISALLOCATED", 0, {}, 0, 0 },
	{ "FILEISVALID", 0, {}, 0, 0 },
	{ "FILEALLOCATE", 0, {}, 0, 0 },
	{ "FILEFLUSH", 0, {}, 0, 0 },
	{ "FILEGETCHAR", 0, {}, 0, 0 },
	{ "FILEREWIND", 0, {}, 0, 0 },
	{ "FILECLEARERR", 0, {}, 0, 0 },
	
	{ "FILEOPEN", 1, { REG_R }, 0, 0 },
	{ "FILECREATE", 1, { REG_R }, 0, 0 },
	{ "FILEREADSTR", 1, { REG_R }, 0, 0 },
	{ "FILEWRITESTR", 1, { REG_R }, 0, 0 },
	{ "FILEPUTCHAR", 1, { REG_R }, 0, 0 },
	{ "FILEUNGETCHAR", 1, { REG_R }, 0, 0 },
	
	{ "FILEREADCHARS", 2, { REG_R, REG_R }, 0, 0 },
	{ "FILEREADINTS", 2, { REG_R, REG_R }, 0, 0 },
	{ "FILEWRITECHARS", 2, { REG_R, REG_R }, 0, 0 },
	{ "FILEWRITEINTS", 2, { REG_R, REG_R }, 0, 0 },
	{ "FILESEEK", 2, { REG_R, REG_R }, 0, 0 },
	{ "FILEOPENMODE", 2, { REG_R, REG_R }, 0, 0 },
	{ "FILEGETERROR", 1, { REG_R }, 0, 0 },
	
	{ "BITMAPFREE", 0, {}, 0, 0 },
	
	{ "POPARGS", 2, { REG_W, NUM }, 0, 0 },
	{ "GAMERELOAD", 0, {}, 0, 0 },
	
	{ "READPODARRAYR", 2, { REG_W, REG_R }, 0, 0 },
	{ "READPODARRAYV", 2, { REG_W, NUM }, 0, 0 },
	{ "WRITEPODARRAYRR", 2, { REG_R, REG_R }, 0, 0 },
	{ "WRITEPODARRAYRV", 2, { REG_R, NUM }, 0, 0 },
	{ "WRITEPODARRAYVR", 2, { NUM, REG_R }, 0, 0 },
	{ "WRITEPODARRAYVV", 2, { NUM, NUM }, 0, 0 },
	
	{ "PRINTFV", 1, { NUM }, 0, 0 },
	{ "SPRINTFV", 1, { NUM }, 0, 0 },
	
	{ "STRCMPR", 2, { REG_R, REG_R }, 0, CMPSET },
	{ "STRICMPR", 2, { REG_R, REG_R }, 0, CMPSET },
	{ "STRINGICOMPARE", 1, { REG_W }, 0, 0 },
	{ "STRINGNICOMPARE", 1, { REG_W }, 0, 0 },
	
	{ "FILEREMOVE", 0, {}, 0, 0 },
	{ "FILESYSREMOVE", 1, { REG_RW }, 0, 0 },
	
	{ "DRAWSTRINGR2", 0, {}, 0, 0 },
	{ "BMPDRAWSTRINGR2", 0, {}, 0, 0 },
	
	{ "MODULEGETIC", 2, { REG_R, REG_R }, 0, 0 },
	{ "ITOACAT", 2, { REG_RW, REG_R }, 0, 0 },
	
	{ "FRAMER", 0, {}, 0, 0 },
	{ "BMPFRAMER", 0, {}, 0, 0 },
	
	{ "LOADDIRECTORYR", 1, { REG_RW }, 0, 0 },
	{ "DIRECTORYGET", 2, { REG_RW, REG_R }, 0, 0 },
	{ "DIRECTORYRELOAD", 0, {}, 0, 0 },
	{ "DIRECTORYFREE", 0, {}, 0, 0 },
	{ "FILEWRITEBYTES", 2, { REG_R, REG_R }, 0, 0 },
	{ "GETCOMBOSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "FILEREADBYTES", 2, { REG_R, REG_R }, 0, 0 },
	
	{ "LOADRNG", 0, {}, 0, 0 },
	{ "RNGRAND1", 0, {}, 0, 0 },
	{ "RNGRAND2", 1, { REG_RW }, 0, 0 },
	{ "RNGRAND3", 2, { REG_RW, REG_R }, 0, 0 },
	{ "RNGLRAND1", 0, {}, 0, 0 },
	{ "RNGLRAND2", 1, { REG_R }, 0, 0 },
	{ "RNGLRAND3", 2, { REG_R, REG_R }, 0, 0 },
	{ "RNGSEED", 1, { REG_R }, 0, 0 },
	{ "RNGRSEED", 0, {}, 0, 0 },
	{ "RNGFREE", 0, {}, 0, 0 },
	{ "LWPNDEL", 0, {}, 0, 0 },
	{ "EWPNDEL", 0, {}, 0, 0 },
	{ "ITEMDEL", 0, {}, 0, 0 },
	{ "BMPWRITETILE", 0, {}, 0, 0 },
	{ "BMPDITHER", 0, {}, 0, 0 },
	{ "BMPREPLCOLOR", 0, {}, 0, 0 },
	{ "BMPSHIFTCOLOR", 0, {}, 0, 0 },
	{ "BMPMASKDRAW", 0, {}, 0, 0 },
	{ "RESIZEARRAYR", 2, { REG_R, REG_R }, 0, 0 },
	{ "BSHOPNAMEGET", 1, { REG_R }, 0, 0 },
	{ "BSHOPNAMESET", 1, { REG_R }, 0, 0 },
	{ "BOTTLENAMEGET", 1, { REG_R }, 0, 0 },
	{ "BOTTLENAMESET", 1, { REG_R }, 0, 0 },
	{ "LOADBOTTLETYPE", 1, { REG_R }, 0, 0 },
	{ "LOADBSHOPDATA", 1, { REG_R }, 0, 0 },
	{ "SWITCHNPC", 1, { REG_RW }, 0, 0 },
	{ "SWITCHCMB", 2, { REG_RW, REG_R }, 0, 0 },
	{ "SWITCHITM", 1, { REG_RW }, 0, 0 },
	{ "SWITCHLW", 1, { REG_RW }, 0, 0 },
	{ "SWITCHEW", 1, { REG_RW }, 0, 0 },
	{ "SCREENDOSPAWN", 0, {}, 0, 0 },
	{ "NPCMOVEPAUSED", 0, {}, 0, 0 },
	{ "NPCMOVE", 0, {}, 0, 0 },
	{ "NPCMOVEANGLE", 0, {}, 0, 0 },
	{ "NPCMOVEXY", 0, {}, 0, 0 },
	{ "NPCCANMOVEDIR", 0, {}, 0, 0 },
	{ "NPCCANMOVEANGLE", 0, {}, 0, 0 },
	{ "NPCCANMOVEXY", 0, {}, 0, 0 },
	{ "SELECTXWPNR", 1, { REG_R }, 0, 0 },
	{ "SELECTYWPNR", 1, { REG_R }, 0, 0 },
	{ "BITMAPOWN", 0, {}, 0, 0 },
	{ "FILEOWN", 0, {}, 0, 0 },
	{ "DIRECTORYOWN", 0, {}, 0, 0 },
	{ "RNGOWN", 0, {}, 0, 0 },
	{ "LOADGENERICDATA", 1, { REG_R }, 0, 0 },
	{ "RUNGENFRZSCR", 1, { REG_W }, 0, 0 },
	{ "WAITTO", 2, { REG_R, REG_R }, 0, 0 },
	{ "GETGENERICSCRIPT", 1, { REG_RW }, 0, 0 },
	{ "KILLPLAYER", 1, { REG_R }, 0, 0 },
	{ "DEGTORAD", 2, { REG_W, REG_R }, 0, 0 },
	{ "RADTODEG", 2, { REG_W, REG_R }, 0, 0 },
	{ "LWPNMAKEANGULAR", 1, { REG }, 0, 0 },
	{ "EWPNMAKEANGULAR", 1, { REG }, 0, 0 },
	{ "LWPNMAKEDIRECTIONAL", 1, { REG }, 0, 0 },
	{ "EWPNMAKEDIRECTIONAL", 1, { REG }, 0, 0 },
	{ "BMPMASKDRAW2", 0, {}, 0, 0 },
	{ "BMPMASKDRAW3", 0, {}, 0, 0 },
	{ "BMPMASKBLIT", 0, {}, 0, 0 },
	{ "BMPMASKBLIT2", 0, {}, 0, 0 },
	{ "BMPMASKBLIT3", 0, {}, 0, 0 },
	{ "SUBV2", 2, { NUM, REG_RW }, 0, 0 },
	{ "DIVV2", 2, { NUM, REG_RW }, 0, 0 },
	{ "COMPAREV2", 2, { NUM, REG_R }, 0, CMPSET },
	{ "MODV2", 2, { NUM, REG_RW }, 0, 0 },
	{ "STACKFREE", 0, {}, 0, 0 },
	{ "STACKOWN", 0, {}, 0, 0 },
	{ "STACKGET", 1, { REG_RW }, 0, 0 },
	{ "STACKSET", 2, { REG_R, REG_R }, 0, 0 },
	{ "STACKPOPBACK", 1, { REG_W }, 0, 0 },
	{ "STACKPOPFRONT", 1, { REG_W }, 0, 0 },
	{ "STACKPEEKBACK", 1, { REG_W }, 0, 0 },
	{ "STACKPEEKFRONT", 1, { REG_W }, 0, 0 },
	{ "STACKPUSHBACK", 1, { REG_R }, 0, 0 },
	{ "STACKPUSHFRONT", 1, { REG_R }, 0, 0 },
	{ "LOADSTACK", 0, {}, 0, 0 },
	{ "STACKCLEAR", 0, {}, 0, 0 },
	{ "POWERV2", 2, { NUM, REG_RW }, 0, 0 },
	{ "LPOWERR", 2, { REG_RW, REG_R }, 0, 0 },
	{ "LPOWERV", 2, { REG_RW, NUM }, 0, 0 },
	{ "LPOWERV2", 2, { NUM, REG_RW }, 0, 0 },
	{ "SCRTRIGGERCOMBO", 2, { REG_RW, REG_R }, 0, 0 },
	{ "WAITEVENT", 0, {}, 0, 0 },
	{ "OWNARRAYR", 1, { REG_R }, 0, 0 },
	{ "DESTROYARRAYR", 1, { REG_R }, 0, 0 },
	{ "GRAPHICSCOUNTCOLOR", 1, { REG_W }, 0, 0 },
	{ "WRITEPODSTRING", 1, { REG_R }, 1, 0 },
	{ "WRITEPODARRAY", 1, { REG_R }, 2, 0 },
	{ "ZCLASS_CONSTRUCT", 1, { REG_W }, 2, 0 },
	{ "ZCLASS_READ", 2, { REG_R, NUM }, 0, 0 },
	{ "ZCLASS_WRITE", 2, { REG_R, NUM }, 0, 0 },
	{ "ZCLASS_FREE", 1, { REG_R }, 0, 0 },
	{ "ZCLASS_OWN", 1, { REG_R }, 0, 0 },
	{ "STARTDESTRUCTOR", 0, {}, 1, 0 },
	{ "ZCLASS_GLOBALIZE", 1, { REG_R }, 0, 0 },
	{ "LOADD", 2, { REG_W, NUM }, 0, 0 },
	{ "STORED", 2, { REG_R, NUM }, 0, 0 },
	{ "OBJ_OWN_BITMAP", 2, { REG_R, NUM }, 0, 0 },
	{ "OBJ_OWN_PALDATA", 2, { REG_R, NUM }, 0, 0 },
	{ "OBJ_OWN_FILE", 2, { REG_R, NUM }, 0, 0 },
	{ "OBJ_OWN_DIR", 2, { REG_R, NUM }, 0, 0 },
	{ "OBJ_OWN_STACK", 2, { REG_R, NUM }, 0, 0 },
	{ "OBJ_OWN_RNG", 2, { REG_R, NUM }, 0, 0 },
	{ "OBJ_OWN_CLASS", 2, { REG_R, NUM }, 0, 0 },
	{ "OBJ_OWN_ARRAY", 2, { REG_R, NUM }, 0, 0 },
	{ "QUIT_NO_DEALLOC", 0, {}, 0, 0 },
	{ "GAMESETCUSTOMCURSOR", 0, {}, 0, 0 },
	{ "NPCCANPLACE", 0, {}, 0, 0 },
	{ "ITEMGETDISPLAYNAME", 1, { REG_R }, 0, 0 },
	{ "ITEMSETDISPLAYNAME", 1, { REG_R }, 0, 0 },
	{ "ITEMGETSHOWNNAME", 1, { REG_R }, 0, 0 },
	{ "HEROMOVEXY", 0, {}, 0, 0 },
	{ "HEROCANMOVEXY", 0, {}, 0, 0 },
	{ "HEROLIFTRELEASE", 0, {}, 0, 0 },
	{ "HEROLIFTGRAB", 0, {}, 0, 0 },
	{ "LOADPORTAL", 0, {}, 0, 0 },
	{ "CREATEPORTAL", 0, {}, 0, 0 },
	{ "LOADSAVPORTAL", 0, {}, 0, 0 },
	{ "CREATESAVPORTAL", 0, {}, 0, 0 },

	{ "CREATEPALDATA", 0, {}, 0, 0 },
	{ "CREATEPALDATACLR", 1, { REG_R }, 0, 0 },
	{ "MIXCLR", 0, {}, 0, 0 },
	{ "CREATERGBHEX", 1, { REG_R }, 0, 0 },
	{ "CREATERGB", 0, {}, 0, 0 },
	{ "PALDATALOADLEVEL", 1, { REG_R }, 0, 0 },
	{ "PALDATALOADSPRITE", 1, { REG_R }, 0, 0 },
	{ "PALDATALOADMAIN", 0, {}, 0, 0 },
	{ "PALDATALOADCYCLE", 1, { REG_R }, 0, 0 },
	{ "PALDATALOADBITMAP", 1, { REG_R }, 0, 0 },
	{ "PALDATAWRITELEVEL", 1, { REG_R }, 0, 0 },
	{ "PALDATAWRITELEVELCS", 2, { REG_R, REG_R }, 0, 0 },
	{ "PALDATAWRITESPRITE", 1, { REG_R }, 0, 0 },
	{ "PALDATAWRITESPRITECS", 2, { REG_R, REG_R }, 0, 0 },
	{ "PALDATAWRITEMAIN", 0, {}, 0, 0 },
	{ "PALDATAWRITEMAINCS", 1, { REG_R }, 0, 0 },
	{ "PALDATAWRITECYCLE", 1, { REG_R }, 0, 0 },
	{ "PALDATAWRITECYCLECS", 2, { REG_R, REG_R }, 0, 0 },
	{ "PALDATAVALIDCLR", 1, { REG_RW }, 0, 0 },
	{ "PALDATACLEARCLR", 1, { REG_RW }, 0, 0 },
	{ "PALDATACLEARCSET", 1, { REG_R }, 0, 0 },
	{ "PALDATAMIX", 0, {}, 0, 0 },
	{ "PALDATAMIXCS", 0, {}, 0, 0 },
	{ "PALDATACOPY", 1, { REG_R }, 0, 0 },
	{ "PALDATACOPYCSET", 0, {}, 0, 0 },
	{ "PALDATAFREE", 0, {}, 0, 0 },
	{ "PALDATAOWN", 0, {}, 0, 0 },
	{ "MAXVARG", 0, {}, 0, 0 },
	{ "MINVARG", 0, {}, 0, 0 },
	{ "CHOOSEVARG", 0, {}, 0, 0 },
	{ "PUSHVARGV", 1, { NUM }, 0, 0 },
	{ "PUSHVARGR", 1, { REG_R }, 0, 0 },
	{ "PRINTFVARG", 0, {}, 0, 0 },
	{ "SPRINTFVARG", 0, {}, 0, 0 },
	{ "TRACELR", 1, { REG_R }, 0, 0 },
	{ "WAITFRAMESR", 1, { REG_R }, 0, 0 },
	{ "GETSCREENFORCOMBOPOS", 1, { REG_R }, 0, 0},
	{ "SECRETSFORR", 1, { REG_R }, 0, 0 },
	{ "RESRVD_OP_Z3_03", 0, {}, 0, 0 },
	{ "RESRVD_OP_Z3_04", 0, {}, 0, 0 },
	{ "RESRVD_OP_Z3_05", 0, {}, 0, 0 },
	{ "RESRVD_OP_Z3_06", 0, {}, 0, 0 },
	{ "RESRVD_OP_Z3_07", 0, {}, 0, 0 },
	{ "RESRVD_OP_Z3_08", 0, {}, 0, 0 },
	{ "RESRVD_OP_Z3_09", 0, {}, 0, 0 },
	{ "RESRVD_OP_Z3_10", 0, {}, 0, 0 },
	{ "PORTALREMOVE", 0, {}, 0, 0 },
	{ "SAVEDPORTALREMOVE", 0, {}, 0, 0 },
	{ "SAVEDPORTALGENERATE", 0, {}, 0, 0 },
	{ "PORTALUSESPRITE", 1, { REG_R }, 0, 0 },
	{ "HEROMOVEATANGLE", 0, {}, 0, 0 },
	{ "HEROCANMOVEATANGLE", 0, {}, 0, 0 },
	{ "HEROMOVE", 0, {}, 0, 0 },
	{ "HEROCANMOVE", 0, {}, 0, 0 },
	{ "DRAWLIGHT_CIRCLE", 0, {}, 0, 0 },
	{ "DRAWLIGHT_SQUARE", 0, {}, 0, 0 },
	{ "DRAWLIGHT_CONE", 0, {}, 0, 0 },
	{ "PEEK", 1, { REG_W }, 0, 0 },
	{ "PEEKATV", 2, { REG_W, NUM }, 0, 0 },
	{ "MAKEVARGARRAY", 0, {}, 0, 0 },
	{ "PRINTFA", 0, {}, 0, 0 },
	{ "SPRINTFA", 0, {}, 0, 0 },
	{ "CURRENTITEMID", 0, {}, 0, 0 },
	{ "ARRAYPUSH", 0, {}, 0, 0 },
	{ "ARRAYPOP", 0, {}, 0, 0 },
	{ "LOADSUBDATARV", 2, { REG_R, NUM }, 0, 0 },
	{ "SWAPSUBSCREENV", 1, { NUM }, 0, 0 },
	{ "SUBDATA_GET_NAME", 1, { REG_R }, 0, 0 },
	{ "SUBDATA_SET_NAME", 1, { REG_R }, 0, 0 },
	{ "CONVERTFROMRGB", 0, {}, 0, 0 },
	{ "CONVERTTORGB", 0, {}, 0, 0 },
	{ "GETENHMUSICLEN", 1, { REG_W }, 0, 0 },
	{ "SETENHMUSICLOOP", 2, { REG_R, REG_R }, 0, 0 },
	{ "PLAYSOUNDEX", 0, {}, 0, 0 },
	{ "GETSFXCOMPLETION", 1, { REG_RW }, 0, 0 },
	{ "ENHCROSSFADE", 0, {}, 0, 0 },
	{ "RESRVD_OP_MOOSH_08", 0, {}, 0, 0 },
	{ "RESRVD_OP_MOOSH_09", 0, {}, 0, 0 },
	{ "RESRVD_OP_MOOSH_10", 0, {}, 0, 0 },
	
	{ "SUBDATA_SWAP_PAGES", 0, {}, 0, 0 },
	{ "SUBPAGE_FIND_WIDGET", 0, {}, 0, 0 },
	{ "SUBPAGE_MOVE_SEL", 0, {}, 0, 0 },
	{ "SUBPAGE_SWAP_WIDG", 0, {}, 0, 0 },
	{ "SUBPAGE_NEW_WIDG", 0, {}, 0, 0 },
	{ "SUBPAGE_DELETE", 0, {}, 0, 0 },
	{ "SUBWIDG_GET_SELTEXT_OVERRIDE", 1, { REG_R }, 0, 0 },
	{ "SUBWIDG_SET_SELTEXT_OVERRIDE", 1, { REG_R }, 0, 0 },
	{ "SUBWIDG_TY_GETTEXT", 1, { REG_R }, 0, 0 },
	{ "SUBWIDG_TY_SETTEXT", 1, { REG_R }, 0, 0 },

	{ "SUBPAGE_FIND_WIDGET_BY_LABEL", 0, {}, 0, 0 },

	{ "SUBWIDG_GET_LABEL", 1, { REG_R }, 0, 0 },
	{ "SUBWIDG_SET_LABEL", 1, { REG_R }, 0, 0 },
	{ "PUSHARGSR", 2, { REG_R, NUM }, 0, 0 },
	{ "PUSHARGSV", 2, { NUM, NUM }, 0, 0 },
	{ "PUSHVARGSR", 2, { REG_R, NUM }, 0, 0 },
	{ "PUSHVARGSV", 2, { NUM, NUM }, 0, 0 },

	{ "WRAPRADIANS", 0, {}, 0, 0 },
	{ "WRAPDEGREES", 0, {}, 0, 0 },

	{ "CALLFUNC", 1, { NUM }, 0, 0 },
	{ "RETURNFUNC", 0, {}, 0, 0 },

	{ "SETCMP", 2, { REG_W, CMP }, 0, CMPUSED },
	{ "GOTOCMP", 2, { NUM, CMP }, 0, CMPUSED },
	{ "STACKWRITEATRV", 2, { REG_R, NUM }, 0, 0 },
	{ "STACKWRITEATVV", 2, { NUM, NUM }, 0, 0 },

	{ "TRUNCATE", 1, { REG_RW }, 0, 0 },
	{ "ROUND", 1, { REG_RW }, 0, 0 },
	{ "ROUNDAWAY", 1, { REG_RW }, 0, 0 },
	{ "STOREDV", 2, { NUM, NUM }, 0, 0 },

	{ "STACKWRITEATVV_IF", 3, { NUM, NUM, CMP }, 0, CMPUSED },

	{ "LOAD", 2, { REG_W, NUM }, 0, 0 },
	{ "STORE", 2, { REG_R, NUM }, 0, 0 },
	{ "STOREV", 2, { NUM, NUM }, 0, 0 },

	{ "WEBSOCKET_SEND", 2, { REG_R, REG_R }, 0, 0 },
	{ "WEBSOCKET_LOAD", 1, { REG_R }, 0, 0 },
	{ "WEBSOCKET_FREE", 0, { }, 0, 0 },
	{ "WEBSOCKET_OWN", 0, { }, 0, 0 },
	{ "WEBSOCKET_ERROR", 1, { REG_W }, 0, 0 },
	{ "WEBSOCKET_RECEIVE", 1, { REG_W }, 0, 0 },
	
	{ "ATOL", 2, { REG_W, REG_R }, 0, 0 },

	{ "REF_INC", 1, { NUM }, 0, 0 },
	{ "REF_DEC", 1, { NUM }, 0, 0 },
	{ "REF_AUTORELEASE", 1, { REG_R }, 0, 0 },
	{ "REF_REMOVE", 1, { NUM }, 0, 0 },
	{ "REF_COUNT", 1, { REG_RW }, 0, 0 },
	{ "MARK_TYPE_STACK", 2, { NUM, NUM }, 0, 0 },
	{ "MARK_TYPE_REG", 2, { REG_W, NUM }, 0, 0 },
	{ "ZCLASS_MARK_TYPE", 0, {}, 2, 0 },
	{ "STORE_OBJECT", 2, { REG_R, NUM }, 0, 0 },
	{ "GC", 0, { }, 0, 0 },
	{ "SET_OBJECT", 2, { REG_W, REG_R }, 0, 0 },

	{ "", 0, {}, 0, 0 }
};

script_variable variable_list[]=
{
	//name id maxcount multiple
	{ "D", D(0), 8, 0 },
	{ "A", A(0), 2, 0 },
	{ "DATA", DATA, 0, 0 },
	{ "CSET", FCSET, 0, 0 },
	{ "DELAY", DELAY, 0, 0 },
	{ "X", FX, 0, 0 },
	{ "Y", FY, 0, 0 },
	{ "XD", XD, 0, 0 },
	{ "YD", YD, 0, 0 },
	{ "XD2", XD2, 0, 0 },
	{ "YD2", YD2, 0, 0 },
	{ "FLAG", FLAG, 0, 0 },
	{ "WIDTH", WIDTH, 0, 0 },
	{ "HEIGHT", HEIGHT, 0, 0 },
	{ "LINK", LINK, 0, 0 },
	{ "FFFLAGSD", FFFLAGSD, 0, 0 },
	{ "FFCWIDTH", FFCWIDTH, 0, 0 },
	{ "FFCHEIGHT", FFCHEIGHT, 0, 0 },
	{ "FFTWIDTH", FFTWIDTH, 0, 0 },
	{ "FFTHEIGHT", FFTHEIGHT, 0, 0 },
	{ "FFLINK", FFLINK, 0, 0 },
	// { "COMBOD", COMBOD(0), 176, 3 },
	// { "COMBOC", COMBOC(0), 176, 3 },
	// { "COMBOF", COMBOF(0), 176, 3 },
	{ "INPUTSTART", INPUTSTART, 0, 0 },
	{ "INPUTUP", INPUTUP, 0, 0 },
	{ "INPUTDOWN", INPUTDOWN, 0, 0 },
	{ "INPUTLEFT", INPUTLEFT, 0, 0 },
	{ "INPUTRIGHT", INPUTRIGHT, 0, 0 },
	{ "INPUTA", INPUTA, 0, 0 },
	{ "INPUTB", INPUTB, 0, 0 },
	{ "INPUTL", INPUTL, 0, 0 },
	{ "INPUTR", INPUTR, 0, 0 },
	{ "INPUTMOUSEX", INPUTMOUSEX, 0, 0 },
	{ "INPUTMOUSEY", INPUTMOUSEY, 0, 0 },
	{ "LINKX", LINKX, 0, 0 },
	{ "LINKY", LINKY, 0, 0 },
	{ "LINKZ", LINKZ, 0, 0 },
	{ "LINKJUMP", LINKJUMP, 0, 0 },
	{ "LINKDIR", LINKDIR, 0, 0 },
	{ "LINKHITDIR", LINKHITDIR, 0, 0 },
	{ "LINKHP", LINKHP, 0, 0 },
	{ "LINKMP", LINKMP, 0, 0 },
	{ "LINKMAXHP", LINKMAXHP, 0, 0 },
	{ "LINKMAXMP", LINKMAXMP, 0, 0 },
	{ "LINKACTION", LINKACTION, 0, 0 },
	{ "LINKHELD", LINKHELD, 0, 0 },
	{ "LINKITEMD", LINKITEMD, 0, 0 },
	{ "LINKSWORDJINX", LINKSWORDJINX, 0, 0 },
	{ "LINKITEMJINX", LINKITEMJINX, 0, 0 },
	{ "LINKDRUNK", LINKDRUNK, 0, 0 },
	{ "ITEMX", ITEMX, 0, 0 },
	{ "ITEMY", ITEMY, 0, 0 },
	{ "ITEMZ", ITEMZ, 0, 0 },
	{ "ITEMJUMP", ITEMJUMP, 0, 0 },
	{ "ITEMDRAWTYPE", ITEMDRAWTYPE, 0, 0 },
	{ "ITEMID", ITEMID, 0, 0 },
	{ "ITEMTILE", ITEMTILE, 0, 0 },
	{ "ITEMOTILE", ITEMOTILE, 0, 0 },
	{ "ITEMCSET", ITEMCSET, 0, 0 },
	{ "ITEMFLASHCSET", ITEMFLASHCSET, 0, 0 },
	{ "ITEMFRAMES", ITEMFRAMES, 0, 0 },
	{ "ITEMFRAME", ITEMFRAME, 0, 0 },
	{ "ITEMASPEED", ITEMASPEED, 0, 0 },
	{ "ITEMDELAY", ITEMDELAY, 0, 0 },
	{ "ITEMFLASH", ITEMFLASH, 0, 0 },
	{ "ITEMFLIP", ITEMFLIP, 0, 0 },
	{ "ITEMCOUNT", ITEMCOUNT, 0, 0 },
	{ "IDATAFAMILY", IDATAFAMILY, 0, 0 },
	{ "IDATALEVEL", IDATALEVEL, 0, 0 },
	{ "IDATAKEEP", IDATAKEEP, 0, 0 },
	{ "IDATAAMOUNT", IDATAAMOUNT, 0, 0 },
	{ "IDATASETMAX", IDATASETMAX, 0, 0 },
	{ "IDATAMAX", IDATAMAX, 0, 0 },
	{ "IDATACOUNTER", IDATACOUNTER, 0, 0 },
	{ "ITEMEXTEND", ITEMEXTEND, 0, 0 },
	{ "NPCX", NPCX, 0, 0 },
	{ "NPCY", NPCY, 0, 0 },
	{ "NPCZ", NPCZ, 0, 0 },
	{ "NPCJUMP", NPCJUMP, 0, 0 },
	{ "NPCDIR", NPCDIR, 0, 0 },
	{ "NPCRATE", NPCRATE, 0, 0 },
	{ "NPCSTEP", NPCSTEP, 0, 0 },
	{ "NPCFRAMERATE", NPCFRAMERATE, 0, 0 },
	{ "NPCHALTRATE", NPCHALTRATE, 0, 0 },
	{ "NPCDRAWTYPE", NPCDRAWTYPE, 0, 0 },
	{ "NPCHP", NPCHP, 0, 0 },
	{ "NPCID", NPCID, 0, 0 },
	{ "NPCDP", NPCDP, 0, 0 },
	{ "NPCWDP", NPCWDP, 0, 0 },
	{ "NPCOTILE", NPCOTILE, 0, 0 },
	{ "NPCENEMY", NPCENEMY, 0, 0 },
	{ "NPCWEAPON", NPCWEAPON, 0, 0 },
	{ "NPCITEMSET", NPCITEMSET, 0, 0 },
	{ "NPCCSET", NPCCSET, 0, 0 },
	{ "NPCBOSSPAL", NPCBOSSPAL, 0, 0 },
	{ "NPCBGSFX", NPCBGSFX, 0, 0 },
	{ "NPCCOUNT", NPCCOUNT, 0, 0 },
	{ "GD", GD(0), 1024, 0 },
	{ "SDD", SDD, 0, 0 },
	{ "GDD", GDD, 0, 0 },
	{ "SDDD", SDDD, 0, 0 },
	{ "SCRDOORD", SCRDOORD, 0, 0 },
	{ "GAMEDEATHS", GAMEDEATHS, 0, 0 },
	{ "GAMECHEAT", GAMECHEAT, 0, 0 },
	{ "GAMETIME", GAMETIME, 0, 0 },
	{ "GAMEHASPLAYED", GAMEHASPLAYED, 0, 0 },
	{ "GAMETIMEVALID", GAMETIMEVALID, 0, 0 },
	{ "GAMEGUYCOUNT", GAMEGUYCOUNT, 0, 0 },
	{ "GAMECONTSCR", GAMECONTSCR, 0, 0 },
	{ "GAMECONTDMAP", GAMECONTDMAP, 0, 0 },
	{ "GAMECOUNTERD", GAMECOUNTERD, 0, 0 },
	{ "GAMEMCOUNTERD", GAMEMCOUNTERD, 0, 0 },
	{ "GAMEDCOUNTERD", GAMEDCOUNTERD, 0, 0 },
	{ "GAMEGENERICD", GAMEGENERICD, 0, 0 },
	{ "GAMEMISC", GAMEMISC, 0, 0 },
	{ "GAMEITEMSD", GAMEITEMSD, 0, 0 },
	{ "GAMELITEMSD", GAMELITEMSD, 0, 0 },
	{ "GAMELKEYSD", GAMELKEYSD, 0, 0 },
	{ "SCREENSTATED", SCREENSTATED, 0, 0 },
	{ "SCREENSTATEDD", SCREENSTATEDD, 0, 0 },
	{ "GAMEGUYCOUNTD", GAMEGUYCOUNTD, 0, 0 },
	{ "CURMAP", CURMAP, 0, 0 },
	{ "CURSCR", CURSCR, 0, 0 },
	{ "CURDSCR", CURDSCR, 0, 0 },
	{ "CURDMAP", CURDMAP, 0, 0 },
	{ "COMBODD", COMBODD, 0, 0 },
	{ "COMBOCD", COMBOCD, 0, 0 },
	{ "COMBOFD", COMBOFD, 0, 0 },
	{ "COMBOTD", COMBOTD, 0, 0 },
	{ "COMBOID", COMBOID, 0, 0 },
	{ "COMBOSD", COMBOSD, 0, 0 },
	{ "REFITEMCLASS", REFITEMCLASS, 0, 0 },
	{ "REFITEM", REFITEM, 0, 0 },
	{ "REFFFC", REFFFC, 0, 0 },
	{ "REFLWPN", REFLWPN, 0, 0 },
	{ "REFEWPN", REFEWPN, 0, 0 },
	{ "REFLWPNCLASS", REFLWPNCLASS, 0, 0 },
	{ "REFEWPNCLASS", REFEWPNCLASS, 0, 0 },
	{ "REFNPC", REFNPC, 0, 0 },
	{ "REFNPCCLASS", REFNPCCLASS, 0, 0 },
	{ "LWPNX", LWPNX, 0, 0 },
	{ "LWPNY", LWPNY, 0, 0 },
	{ "LWPNZ", LWPNZ, 0, 0 },
	{ "LWPNJUMP", LWPNJUMP, 0, 0 },
	{ "LWPNDIR", LWPNDIR, 0, 0 },
	{ "LWPNSTEP", LWPNSTEP, 0, 0 },
	{ "LWPNANGULAR", LWPNANGULAR, 0, 0 },
	{ "LWPNANGLE", LWPNANGLE, 0, 0 },
	{ "LWPNDRAWTYPE", LWPNDRAWTYPE, 0, 0 },
	{ "LWPNPOWER", LWPNPOWER, 0, 0 },
	{ "LWPNDEAD", LWPNDEAD, 0, 0 },
	{ "LWPNID", LWPNID, 0, 0 },
	{ "LWPNTILE", LWPNTILE, 0, 0 },
	{ "LWPNCSET", LWPNCSET, 0, 0 },
	{ "LWPNFLASHCSET", LWPNFLASHCSET, 0, 0 },
	{ "LWPNFRAMES", LWPNFRAMES, 0, 0 },
	{ "LWPNFRAME", LWPNFRAME, 0, 0 },
	{ "LWPNASPEED", LWPNASPEED, 0, 0 },
	{ "LWPNFLASH", LWPNFLASH, 0, 0 },
	{ "LWPNFLIP", LWPNFLIP, 0, 0 },
	{ "LWPNCOUNT", LWPNCOUNT, 0, 0 },
	{ "LWPNEXTEND", LWPNEXTEND, 0, 0 },
	{ "LWPNOTILE", LWPNOTILE, 0, 0 },
	{ "LWPNOCSET", LWPNOCSET, 0, 0 },
	{ "EWPNX", EWPNX, 0, 0 },
	{ "EWPNY", EWPNY, 0, 0 },
	{ "EWPNZ", EWPNZ, 0, 0 },
	{ "EWPNJUMP", EWPNJUMP, 0, 0 },
	{ "EWPNDIR", EWPNDIR, 0, 0 },
	{ "EWPNSTEP", EWPNSTEP, 0, 0 },
	{ "EWPNANGULAR", EWPNANGULAR, 0, 0 },
	{ "EWPNANGLE", EWPNANGLE, 0, 0 },
	{ "EWPNDRAWTYPE", EWPNDRAWTYPE, 0, 0 },
	{ "EWPNPOWER", EWPNPOWER, 0, 0 },
	{ "EWPNDEAD", EWPNDEAD, 0, 0 },
	{ "EWPNID", EWPNID, 0, 0 },
	{ "EWPNTILE", EWPNTILE, 0, 0 },
	{ "EWPNCSET", EWPNCSET, 0, 0 },
	{ "EWPNFLASHCSET", EWPNFLASHCSET, 0, 0 },
	{ "EWPNFRAMES", EWPNFRAMES, 0, 0 },
	{ "EWPNFRAME", EWPNFRAME, 0, 0 },
	{ "EWPNASPEED", EWPNASPEED, 0, 0 },
	{ "EWPNFLASH", EWPNFLASH, 0, 0 },
	{ "EWPNFLIP", EWPNFLIP, 0, 0 },
	{ "EWPNCOUNT", EWPNCOUNT, 0, 0 },
	{ "EWPNEXTEND", EWPNEXTEND, 0, 0 },
	{ "EWPNOTILE", EWPNOTILE, 0, 0 },
	{ "EWPNOCSET", EWPNOCSET, 0, 0 },
	{ "NPCEXTEND", NPCEXTEND, 0, 0 },
	{ "SP", SP, 0, 0 },
	{ "SP2", SP2, 0, 0 },
	{ "WAVY", WAVY, 0, 0 },
	{ "QUAKE", QUAKE, 0, 0 },
	{ "IDATAUSESOUND", IDATAUSESOUND, 0, 0 },
	{ "INPUTMOUSEZ", INPUTMOUSEZ, 0, 0 },
	{ "INPUTMOUSEB", INPUTMOUSEB, 0, 0 },
	{ "COMBODDM", COMBODDM, 0, 0 },
	{ "COMBOCDM", COMBOCDM, 0, 0 },
	{ "COMBOFDM", COMBOFDM, 0, 0 },
	{ "COMBOTDM", COMBOTDM, 0, 0 },
	{ "COMBOIDM", COMBOIDM, 0, 0 },
	{ "COMBOSDM", COMBOSDM, 0, 0 },
	{ "SCRIPTRAM", SCRIPTRAM, 0, 0 },
	{ "GLOBALRAM", GLOBALRAM, 0, 0 },
	{ "SCRIPTRAMD", SCRIPTRAMD, 0, 0 },
	{ "GLOBALRAMD", GLOBALRAMD, 0, 0 },
	{ "LWPNHXOFS", LWPNHXOFS, 0, 0 },
	{ "LWPNHYOFS", LWPNHYOFS, 0, 0 },
	{ "LWPNXOFS", LWPNXOFS, 0, 0 },
	{ "LWPNYOFS", LWPNYOFS, 0, 0 },
	{ "LWPNZOFS", LWPNZOFS, 0, 0 },
	{ "LWPNHXSZ", LWPNHXSZ, 0, 0 },
	{ "LWPNHYSZ", LWPNHYSZ, 0, 0 },
	{ "LWPNHZSZ", LWPNHZSZ, 0, 0 },
	{ "EWPNHXOFS", EWPNHXOFS, 0, 0 },
	{ "EWPNHYOFS", EWPNHYOFS, 0, 0 },
	{ "EWPNXOFS", EWPNXOFS, 0, 0 },
	{ "EWPNYOFS", EWPNYOFS, 0, 0 },
	{ "EWPNZOFS", EWPNZOFS, 0, 0 },
	{ "EWPNHXSZ", EWPNHXSZ, 0, 0 },
	{ "EWPNHYSZ", EWPNHYSZ, 0, 0 },
	{ "EWPNHZSZ", EWPNHZSZ, 0, 0 },
	{ "NPCHXOFS", NPCHXOFS, 0, 0 },
	{ "NPCHYOFS", NPCHYOFS, 0, 0 },
	{ "NPCXOFS", NPCXOFS, 0, 0 },
	{ "NPCYOFS", NPCYOFS, 0, 0 },
	{ "NPCZOFS", NPCZOFS, 0, 0 },
	{ "NPCHXSZ", NPCHXSZ, 0, 0 },
	{ "NPCHYSZ", NPCHYSZ, 0, 0 },
	{ "NPCHZSZ", NPCHZSZ, 0, 0 },
	{ "ITEMHXOFS", ITEMHXOFS, 0, 0 },
	{ "ITEMHYOFS", ITEMHYOFS, 0, 0 },
	{ "ITEMXOFS", ITEMXOFS, 0, 0 },
	{ "ITEMYOFS", ITEMYOFS, 0, 0 },
	{ "ITEMZOFS", ITEMZOFS, 0, 0 },
	{ "ITEMHXSZ", ITEMHXSZ, 0, 0 },
	{ "ITEMHYSZ", ITEMHYSZ, 0, 0 },
	{ "ITEMHZSZ", ITEMHZSZ, 0, 0 },
	{ "LWPNTXSZ", LWPNTXSZ, 0, 0 },
	{ "LWPNTYSZ", LWPNTYSZ, 0, 0 },
	{ "EWPNTXSZ", EWPNTXSZ, 0, 0 },
	{ "EWPNTYSZ", EWPNTYSZ, 0, 0 },
	{ "NPCTXSZ", NPCTXSZ, 0, 0 },
	{ "NPCTYSZ", NPCTYSZ, 0, 0 },
	{ "ITEMTXSZ", ITEMTXSZ, 0, 0 },
	{ "ITEMTYSZ", ITEMTYSZ, 0, 0 },
	{ "LINKHXOFS", LINKHXOFS, 0, 0 },
	{ "LINKHYOFS", LINKHYOFS, 0, 0 },
	{ "LINKXOFS", LINKXOFS, 0, 0 },
	{ "LINKYOFS", LINKYOFS, 0, 0 },
	{ "LINKZOFS", LINKZOFS, 0, 0 },
	{ "LINKHXSZ", LINKHXSZ, 0, 0 },
	{ "LINKHYSZ", LINKHYSZ, 0, 0 },
	{ "LINKHZSZ", LINKHZSZ, 0, 0 },
	{ "LINKTXSZ", LINKTXSZ, 0, 0 },
	{ "LINKTYSZ", LINKTYSZ, 0, 0 },
	{ "NPCTILE", NPCTILE, 0, 0 },
	{ "LWPNBEHIND", LWPNBEHIND, 0, 0 },
	{ "EWPNBEHIND", EWPNBEHIND, 0, 0 },
	{ "SDDDD", SDDDD, 0, 0 },
	{ "CURLEVEL", CURLEVEL, 0, 0 },
	{ "ITEMPICKUP", ITEMPICKUP, 0, 0 },
	{ "INPUTMAP", INPUTMAP, 0, 0 },
	{ "NUM", (int)NUM, 0, 0 },
	{ "INPUTEX1", INPUTEX1, 0, 0 },
	{ "INPUTEX2", INPUTEX2, 0, 0 },
	{ "INPUTEX3", INPUTEX3, 0, 0 },
	{ "INPUTEX4", INPUTEX4, 0, 0 },
	{ "INPUTPRESSSTART", INPUTPRESSSTART, 0, 0 },
	{ "INPUTPRESSUP", INPUTPRESSUP, 0, 0 },
	{ "INPUTPRESSDOWN", INPUTPRESSDOWN, 0, 0 },
	{ "INPUTPRESSLEFT", INPUTPRESSLEFT, 0, 0 },
	{ "INPUTPRESSRIGHT", INPUTPRESSRIGHT, 0, 0 },
	{ "INPUTPRESSA", INPUTPRESSA, 0, 0 },
	{ "INPUTPRESSB", INPUTPRESSB, 0, 0 },
	{ "INPUTPRESSL", INPUTPRESSL, 0, 0 },
	{ "INPUTPRESSR", INPUTPRESSR, 0, 0 },
	{ "INPUTPRESSEX1", INPUTPRESSEX1, 0, 0 },
	{ "INPUTPRESSEX2", INPUTPRESSEX2, 0, 0 },
	{ "INPUTPRESSEX3", INPUTPRESSEX3, 0, 0 },
	{ "INPUTPRESSEX4", INPUTPRESSEX4, 0, 0 },
	{ "LWPNMISCD", LWPNMISCD, 0, 0 },
	{ "EWPNMISCD", EWPNMISCD, 0, 0 },
	{ "NPCMISCD", NPCMISCD, 0, 0 },
	{ "ITEMMISCD", ITEMMISCD, 0, 0 },
	{ "FFMISCD", FFMISCD, 0, 0 },
	{ "GETMIDI", GETMIDI, 0, 0 },
	{ "NPCHOMING", NPCHOMING, 0, 0 },
	{ "NPCDD", NPCDD, 0, 0 },
	{ "LINKEQUIP", LINKEQUIP, 0, 0 },
	{ "INPUTAXISUP", INPUTAXISUP, 0, 0 },
	{ "INPUTAXISDOWN", INPUTAXISDOWN, 0, 0 },
	{ "INPUTAXISLEFT", INPUTAXISLEFT, 0, 0 },
	{ "INPUTAXISRIGHT", INPUTAXISRIGHT, 0, 0 },
	{ "PRESSAXISUP", INPUTPRESSAXISUP, 0, 0 },
	{ "PRESSAXISDOWN", INPUTPRESSAXISDOWN, 0, 0 },
	{ "PRESSAXISLEFT", INPUTPRESSAXISLEFT, 0, 0 },
	{ "PRESSAXISRIGHT", INPUTPRESSAXISRIGHT, 0, 0 },
	{ "NPCTYPE", NPCTYPE, 0, 0 },
	{ "FFSCRIPT", FFSCRIPT, 0, 0 },
	{ "SCREENFLAGSD", SCREENFLAGSD, 0, 0 },
	{ "LINKINVIS", LINKINVIS, 0, 0 },
	{ "LINKINVINC", LINKINVINC, 0, 0 },
	{ "SCREENEFLAGSD", SCREENEFLAGSD, 0, 0 },
	{ "NPCMFLAGS", NPCMFLAGS, 0, 0 },
	{ "FFINITDD", FFINITDD, 0, 0 },
	{ "LINKMISCD", LINKMISCD, 0, 0 },
	{ "DMAPFLAGSD", DMAPFLAGSD, 0, 0 },
	{ "LWPNCOLLDET", LWPNCOLLDET, 0, 0 },
	{ "EWPNCOLLDET", EWPNCOLLDET, 0, 0 },
	{ "NPCCOLLDET", NPCCOLLDET, 0, 0 },
	{ "LINKLADDERX", LINKLADDERX, 0, 0 },
	{ "LINKLADDERY", LINKLADDERY, 0, 0 },
	{ "NPCSTUN", NPCSTUN, 0, 0 },
	{ "NPCDEFENSED", NPCDEFENSED, 0, 0 },
	{ "IDATAPOWER", IDATAPOWER, 0, 0 },
	{ "DMAPLEVELD", DMAPLEVELD, 0, 0 },
	{ "DMAPCOMPASSD", DMAPCOMPASSD, 0, 0 },
	{ "DMAPCONTINUED", DMAPCONTINUED, 0, 0 },
	{ "DMAPMIDID", DMAPMIDID, 0, 0 },
	{ "IDATAINITDD", IDATAINITDD, 0, 0 },
	{ "ROOMTYPE", ROOMTYPE, 0, 0 },
	{ "ROOMDATA", ROOMDATA, 0, 0 },
	{ "LINKTILE", LINKTILE, 0, 0 },
	{ "LINKFLIP", LINKFLIP, 0, 0 },
	{ "INPUTPRESSMAP", INPUTPRESSMAP, 0, 0 },
	{ "NPCHUNGER", NPCHUNGER, 0, 0 },
	{ "GAMESTANDALONE", GAMESTANDALONE, 0, 0 },
	{ "GAMEENTRSCR", GAMEENTRSCR, 0, 0 },
	{ "GAMEENTRDMAP", GAMEENTRDMAP, 0, 0 },
	{ "GAMECLICKFREEZE", GAMECLICKFREEZE, 0, 0 },
	{ "PUSHBLOCKX", PUSHBLOCKX, 0, 0 },
	{ "PUSHBLOCKY", PUSHBLOCKY, 0, 0 },
	{ "PUSHBLOCKCOMBO", PUSHBLOCKCOMBO, 0, 0 },
	{ "PUSHBLOCKCSET", PUSHBLOCKCSET, 0, 0 },
	{ "UNDERCOMBO", UNDERCOMBO, 0, 0 },
	{ "UNDERCSET", UNDERCSET, 0, 0 },
	{ "DMAPOFFSET", DMAPOFFSET, 0, 0 },
	{ "DMAPMAP", DMAPMAP, 0, 0 },
	{ "__RESERVED_FOR_GAMETHROTTLE", __RESERVED_FOR_GAMETHROTTLE, 0, 0 },
	{ "REFMAPDATA", REFMAPDATA, 0, 0 },
	{ "REFSCREENDATA", REFSCREENDATA, 0, 0 },
	{ "REFCOMBODATA", REFCOMBODATA, 0, 0 },
	{ "REFSPRITEDATA", REFSPRITEDATA, 0, 0 },
	{ "REFBITMAP", REFBITMAP, 0, 0 },
	{ "REFDMAPDATA", REFDMAPDATA, 0, 0 },
	{ "REFSHOPDATA", REFSHOPDATA, 0, 0 },
	{ "REFMSGDATA", REFMSGDATA, 0, 0 },
	{ "REFUNTYPED", REFUNTYPED, 0, 0 },
	{ "REFDROPS", REFDROPS, 0, 0 },
	{ "REFPONDS", REFPONDS, 0, 0 },
	{ "REFWARPRINGS", REFWARPRINGS, 0, 0 },
	{ "REFDOORS", REFDOORS, 0, 0 },
	{ "REFUICOLOURS", REFUICOLOURS, 0, 0 },
	{ "REFRGB", REFRGB, 0, 0 },
	{ "REFPALETTE", REFPALETTE, 0, 0 },
	{ "REFTUNES", REFTUNES, 0, 0 },
	{ "REFPALCYCLE", REFPALCYCLE, 0, 0 },
	{ "REFGAMEDATA", REFGAMEDATA, 0, 0 },
	{ "REFCHEATS", REFCHEATS, 0, 0 },
	{ "IDATAMAGICTIMER", IDATAMAGICTIMER, 0, 0 },
	{ "IDATALTM", IDATALTM, 0, 0 },
	{ "IDATASCRIPT", IDATASCRIPT, 0, 0 },
	{ "IDATAPSCRIPT", IDATAPSCRIPT, 0, 0 },
	{ "IDATAMAGCOST", IDATAMAGCOST, 0, 0 },
	{ "IDATAMINHEARTS", IDATAMINHEARTS, 0, 0 },
	{ "IDATATILE", IDATATILE, 0, 0 },
	{ "IDATAMISC", IDATAMISC, 0, 0 },
	{ "IDATACSET", IDATACSET, 0, 0 },
	{ "IDATAFRAMES", IDATAFRAMES, 0, 0 },
	{ "IDATAASPEED", IDATAASPEED, 0, 0 },
	{ "IDATADELAY", IDATADELAY, 0, 0 },
	{ "IDATACOMBINE", IDATACOMBINE, 0, 0 },
	{ "IDATADOWNGRADE", IDATADOWNGRADE, 0, 0 },
	{ "IDATAPSTRING", IDATAPSTRING, 0, 0 },
	{ "IDATAPFLAGS", IDATAPFLAGS, 0, 0 },
	{ "IDATAKEEPOLD", IDATAKEEPOLD, 0, 0 },
	{ "IDATARUPEECOST", IDATARUPEECOST, 0, 0 },
	{ "IDATAEDIBLE", IDATAEDIBLE, 0, 0 },
	{ "IDATAFLAGUNUSED", IDATAFLAGUNUSED, 0, 0 },
	{ "IDATAGAINLOWER", IDATAGAINLOWER, 0, 0 },
	{ "RESVD0024", RESVD024, 0, 0 },
	{ "RESVD0025", RESVD025, 0, 0 },
	{ "RESVD0026", RESVD026, 0, 0 },
	{ "IDATAID", IDATAID, 0, 0 },
	{ "__RESERVED_FOR_LINKEXTEND", __RESERVED_FOR_LINKEXTEND, 0, 0 },
	{ "NPCSCRDEFENSED", NPCSCRDEFENSED, 0, 0 },
	{ "__RESERVED_FOR_SETLINKTILE", __RESERVED_FOR_SETLINKTILE, 0, 0 },
	{ "__RESERVED_FOR_SETLINKEXTEND", __RESERVED_FOR_SETLINKEXTEND, 0, 0 },
	{ "__RESERVED_FOR_SIDEWARPSFX", __RESERVED_FOR_SIDEWARPSFX, 0, 0 },
	{ "__RESERVED_FOR_PITWARPSFX", __RESERVED_FOR_PITWARPSFX, 0, 0 },
	{ "__RESERVED_FOR_SIDEWARPVISUAL", __RESERVED_FOR_SIDEWARPVISUAL, 0, 0 },
	{ "__RESERVED_FOR_PITWARPVISUAL", __RESERVED_FOR_PITWARPVISUAL, 0, 0 },
	{ "GAMESETA", GAMESETA, 0, 0 },
	{ "GAMESETB", GAMESETB, 0, 0 },
	{ "SETITEMSLOT", SETITEMSLOT, 0, 0 },
	{ "LINKITEMB", LINKITEMB, 0, 0 },
	{ "LINKITEMA", LINKITEMA, 0, 0 },
	{ "__RESERVED_FOR_LINKWALKTILE", __RESERVED_FOR_LINKWALKTILE, 0, 0 }, //Walk sprite
	{ "__RESERVED_FOR_LINKFLOATTILE", __RESERVED_FOR_LINKFLOATTILE, 0, 0 }, //float sprite
	{ "__RESERVED_FOR_LINKSWIMTILE", __RESERVED_FOR_LINKSWIMTILE, 0, 0 }, //swim sprite
	{ "__RESERVED_FOR_LINKDIVETILE", __RESERVED_FOR_LINKDIVETILE, 0, 0 }, //dive sprite
	{ "__RESERVED_FOR_LINKSLASHTILE", __RESERVED_FOR_LINKSLASHTILE, 0, 0 }, //slash sprite
	{ "__RESERVED_FOR_LINKJUMPTILE", __RESERVED_FOR_LINKJUMPTILE, 0, 0 }, //jump sprite
	{ "__RESERVED_FOR_LINKCHARGETILE", __RESERVED_FOR_LINKCHARGETILE, 0, 0 }, //charge sprite
	{ "__RESERVED_FOR_LINKSTABTILE", __RESERVED_FOR_LINKSTABTILE, 0, 0 }, //stab sprite
	{ "__RESERVED_FOR_LINKCASTTILE", __RESERVED_FOR_LINKCASTTILE, 0, 0 }, //casting sprite
	{ "__RESERVED_FOR_LINKHOLD1LTILE", __RESERVED_FOR_LINKHOLD1LTILE, 0, 0 }, //hold1land sprite
	{ "__RESERVED_FOR_LINKHOLD2LTILE", __RESERVED_FOR_LINKHOLD2LTILE, 0, 0 }, //hold2land sprite
	{ "__RESERVED_FOR_LINKHOLD1WTILE", __RESERVED_FOR_LINKHOLD1WTILE, 0, 0 }, //hold1water sprite
	{ "__RESERVED_FOR_LINKHOLD2WTILE", __RESERVED_FOR_LINKHOLD2WTILE, 0, 0 }, //hold2water sprite
	{ "__RESERVED_FOR_LINKPOUNDTILE", __RESERVED_FOR_LINKPOUNDTILE, 0, 0 }, //hammer pound sprite
	{ "__RESERVED_FOR_LINKSWIMSPD", __RESERVED_FOR_LINKSWIMSPD, 0, 0 },
	{ "__RESERVED_FOR_LINKWALKANMSPD", __RESERVED_FOR_LINKWALKANMSPD, 0, 0 },
	{ "__RESERVED_FOR_LINKANIMTYPE", __RESERVED_FOR_LINKANIMTYPE, 0, 0 },
	{ "LINKINVFRAME", LINKINVFRAME, 0, 0 },
	{ "LINKCANFLICKER", LINKCANFLICKER, 0, 0 },
	{ "LINKHURTSFX", LINKHURTSFX, 0, 0 },
	{ "NOACTIVESUBSC", NOACTIVESUBSC, 0, 0 },
	{ "LWPNRANGE", LWPNRANGE, 0, 0 },
	{ "ZELDAVERSION", ZELDAVERSION, 0, 0 },
	{ "ZELDABUILD", ZELDABUILD, 0, 0 },
	{ "ZELDABETA", ZELDABETA, 0, 0 },
	{ "NPCINVINC", NPCINVINC, 0, 0 },
	{ "NPCSUPERMAN", NPCSUPERMAN, 0, 0 },
	{ "NPCHASITEM", NPCHASITEM, 0, 0 },
	{ "NPCRINGLEAD", NPCRINGLEAD, 0, 0 },
	{ "IDATAFRAME", IDATAFRAME, 0, 0 },
	{ "ITEMACLK", ITEMACLK, 0, 0 },
	{ "FFCID", FFCID, 0, 0 },
	{ "IDATAATTRIB", IDATAATTRIB, 0, 0 },
	{ "IDATASPRITE", IDATASPRITE, 0, 0 },
	{ "IDATAFLAGS", IDATAFLAGS, 0, 0 },
	{ "DMAPLEVELPAL", DMAPLEVELPAL, 0, 0 },
	{ "__RESERVED_FOR_ITEMPTR", __RESERVED_FOR_ITEMPTR, 0, 0 },
	{ "__RESERVED_FOR_NPCPTR", __RESERVED_FOR_NPCPTR, 0, 0 },
	{ "__RESERVED_FOR_LWPNPTR", __RESERVED_FOR_LWPNPTR, 0, 0 },
	{ "__RESERVED_FOR_EWPNPTR", __RESERVED_FOR_EWPNPTR, 0, 0 },
	{ "SETSCREENDOOR", SETSCREENDOOR, 0, 0 },
	{ "SETSCREENENEMY", SETSCREENENEMY, 0, 0 },
	{ "GAMEMAXMAPS", GAMEMAXMAPS, 0, 0 },
	{ "CREATELWPNDX", CREATELWPNDX, 0, 0 },
	{ "__RESERVED_FOR_SCREENFLAG", __RESERVED_FOR_SCREENFLAG, 0, 0 },
	{ "BUTTONPRESS", BUTTONPRESS, 0, 0 },
	{ "BUTTONINPUT", BUTTONINPUT, 0, 0 },
	{ "BUTTONHELD", BUTTONHELD, 0, 0 },
	{ "RAWKEY", RAWKEY, 0, 0 },
	{ "READKEY", READKEY, 0, 0 },
	{ "JOYPADPRESS", JOYPADPRESS, 0, 0 },
	{ "DISABLEDITEM", DISABLEDITEM, 0, 0 },
	{ "LINKDIAG", LINKDIAG, 0, 0 },
	{ "LINKBIGHITBOX", LINKBIGHITBOX, 0, 0 },
	{ "LINKEATEN", LINKEATEN, 0, 0 },
	{ "__RESERVED_FOR_LINKRETSQUARE", __RESERVED_FOR_LINKRETSQUARE, 0, 0 },
	{ "__RESERVED_FOR_LINKWARPSOUND", __RESERVED_FOR_LINKWARPSOUND, 0, 0 },
	{ "__RESERVED_FOR_PLAYPITWARPSFX", __RESERVED_FOR_PLAYPITWARPSFX, 0, 0 },
	{ "__RESERVED_FOR_WARPEFFECT", __RESERVED_FOR_WARPEFFECT, 0, 0 },
	{ "__RESERVED_FOR_PLAYWARPSOUND", __RESERVED_FOR_PLAYWARPSOUND, 0, 0 },
	{ "LINKUSINGITEM", LINKUSINGITEM, 0, 0 },
	{ "LINKUSINGITEMA", LINKUSINGITEMA, 0, 0 },
	{ "LINKUSINGITEMB", LINKUSINGITEMB, 0, 0 },
// { "DMAPLEVELPAL", DMAPLEVELPAL, 0, 0 },
// { "LINKZHEIGHT", LINKZHEIGHT, 0, 0 },
// { "ITEMINDEX", ITEMINDEX, 0, 0 },
// { "LWPNINDEX", LWPNINDEX, 0, 0 },
// { "EWPNINDEX", EWPNINDEX, 0, 0 },
// { "NPCINDEX", NPCINDEX, 0, 0 },
// TABLE END
	{ "IDATAUSEWPN", IDATAUSEWPN, 0, 0 }, //UseWeapon
	{ "IDATAUSEDEF", IDATAUSEDEF, 0, 0 }, //UseDefense
	{ "IDATAWRANGE", IDATAWRANGE, 0, 0 }, //Range
	{ "IDATAUSEMVT", IDATAUSEMVT, 0, 0 }, //Movement[]
	{ "IDATADURATION", IDATADURATION, 0, 0 }, //Duration
	
	{ "IDATADUPLICATES", IDATADUPLICATES, 0, 0 }, //Duplicates
	{ "IDATADRAWLAYER", IDATADRAWLAYER, 0, 0 }, //DrawLayer
	{ "IDATACOLLECTFLAGS", IDATACOLLECTFLAGS, 0, 0 }, //CollectFlags
	{ "IDATAWEAPONSCRIPT", IDATAWEAPONSCRIPT, 0, 0 }, //WeaponScript
	{ "IDATAMISCD", IDATAMISCD, 0, 0 }, //WeaponMisc[32]
	{ "IDATAWEAPHXOFS", IDATAWEAPHXOFS, 0, 0 }, //WeaponHitXOffset
	{ "IDATAWEAPHYOFS", IDATAWEAPHYOFS, 0, 0 }, //WeaponHitYOffset
	{ "IDATAWEAPHXSZ", IDATAWEAPHYSZ, 0, 0 }, //WeaponHitWidth
	{ "IDATAWEAPHYSZ", IDATAWEAPHYSZ, 0, 0 }, //WeaponHitHeight
	{ "IDATAWEAPHZSZ", IDATAWEAPHZSZ, 0, 0 }, //WeaponHitZHeight
	{ "IDATAWEAPXOFS", IDATAWEAPXOFS, 0, 0 }, //WeaponDrawXOffset
	{ "IDATAWEAPYOFS", IDATAWEAPYOFS, 0, 0 }, //WeaponDrawYOffset
	{ "IDATAWEAPZOFS", IDATAWEAPZOFS, 0, 0 }, //WeaponDrawZOffset
	{ "IDATAWPNINITD", IDATAWPNINITD, 0, 0 }, //WeaponD[8]
	
	{ "NPCWEAPSPRITE", NPCWEAPSPRITE, 0, 0 }, //WeaponSprite
	
	{ "DEBUGREFFFC", DEBUGREFFFC, 0, 0 }, //REFFFC
	{ "DEBUGREFITEM", DEBUGREFITEM, 0, 0 }, //REFITEM
	{ "DEBUGREFNPC", DEBUGREFNPC, 0, 0 }, //REFNPC
	{ "DEBUGREFITEMDATA", DEBUGREFITEMDATA, 0, 0 }, //REFITEMCLASS
	{ "DEBUGREFLWEAPON", DEBUGREFLWEAPON, 0, 0 }, //REFLWPN
	{ "DEBUGREFEWEAPON", DEBUGREFEWEAPON, 0, 0 }, //REFEWPN
	{ "DEBUGSP", DEBUGSP, 0, 0 }, //SP
	{ "DEBUGGDR", DEBUGGDR, 0, 0 }, //GDR[256]
	{ "SCREENWIDTH", SCREENWIDTH, 0, 0 },
	{ "SCREENHEIGHT", SCREENHEIGHT, 0, 0 },
	{ "SCREENVIEWX", SCREENVIEWX, 0, 0 },
	{ "SCREENVIEWY", SCREENVIEWY, 0, 0 },
	{ "SCREENGUY", SCREENGUY, 0, 0 },
	{ "SCREENSTRING", SCREENSTRING, 0, 0 },
	{ "SCREENROOM", SCREENROOM, 0, 0 },
	{ "SCREENENTX", SCREENENTX, 0, 0 },
	{ "SCREENENTY", SCREENENTY, 0, 0 },
	{ "SCREENITEM", SCREENITEM, 0, 0 },
	{ "SCREENUNDCMB", SCREENUNDCMB, 0, 0 },
	{ "SCREENUNDCST", SCREENUNDCST, 0, 0 },
	{ "SCREENCATCH", SCREENCATCH, 0, 0 },
	{ "SETSCREENLAYOP", SETSCREENLAYOP, 0, 0 },
	{ "SETSCREENSECCMB", SETSCREENSECCMB, 0, 0 },
	{ "SETSCREENSECCST", SETSCREENSECCST, 0, 0 },
	{ "SETSCREENSECFLG", SETSCREENSECFLG, 0, 0 },
	{ "SETSCREENLAYMAP", SETSCREENLAYMAP, 0, 0 },
	{ "SETSCREENLAYSCR", SETSCREENLAYSCR, 0, 0 },
	{ "SETSCREENPATH", SETSCREENPATH, 0, 0 },
	{ "SETSCREENWARPRX", SETSCREENWARPRX, 0, 0 },
	{ "SETSCREENWARPRY", SETSCREENWARPRY, 0, 0 },
	{"GAMENUMMESSAGES", GAMENUMMESSAGES, 0, 0 },
	{"GAMESUBSCHEIGHT", GAMESUBSCHEIGHT, 0, 0 },
	{"GAMEPLAYFIELDOFS", GAMEPLAYFIELDOFS, 0, 0 },
	{"PASSSUBOFS", PASSSUBOFS, 0, 0 },
	
	{"COMBODATAID", COMBODATAID, 0, 0 },
	{"REFFILE", REFFILE, 0, 0 },
	{"REFSUBSCREEN", REFSUBSCREEN, 0, 0 },
	// {"MAPDATAID", MAPDATAID, 0, 0 },
	// {"MAPDATAID", MAPDATAID, 0, 0 },
	

	//NPCData
	{"SETNPCDATASCRIPTDEF", SETNPCDATASCRIPTDEF, 0, 0 },
	{"SETNPCDATADEFENSE", SETNPCDATADEFENSE, 0, 0 },
	{"SETNPCDATASIZEFLAG", SETNPCDATASIZEFLAG, 0, 0 },
	{"SETNPCDATAATTRIBUTE", SETNPCDATAATTRIBUTE, 0, 0 },
	
	{"SCDBLOCKWEAPON", SCDBLOCKWEAPON, 0, 0 },
	{"SCDSTRIKEWEAPONS", SCDSTRIKEWEAPONS, 0, 0 },
	{"SCDEXPANSION", SCDEXPANSION, 0, 0 },
	{"SETGAMEOVERELEMENT", SETGAMEOVERELEMENT, 0, 0 },
	{"SETGAMEOVERSTRING", SETGAMEOVERSTRING, 0, 0 },
	{"MOUSEARR", MOUSEARR, 0, 0 },
	
	{"IDATAOVERRIDEFLWEAP", IDATAOVERRIDEFLWEAP, 0, 0 },
	{"IDATATILEHWEAP", IDATATILEHWEAP, 0, 0 },
	{"IDATATILEWWEAP", IDATATILEWWEAP, 0, 0 },
	{"IDATAHZSZWEAP", IDATAHZSZWEAP, 0, 0 },
	{"IDATAHYSZWEAP", IDATAHYSZWEAP, 0, 0 },
	{"IDATAHXSZWEAP", IDATAHXSZWEAP, 0, 0 },
	{"IDATADYOFSWEAP", IDATADYOFSWEAP, 0, 0 },
	{"IDATADXOFSWEAP", IDATADXOFSWEAP, 0, 0 },
	{"IDATAHYOFSWEAP", IDATAHYOFSWEAP, 0, 0 },
	{"IDATAHXOFSWEAP", IDATAHXOFSWEAP, 0, 0 },
	{"IDATAOVERRIDEFL", IDATAOVERRIDEFL, 0, 0 },
	{"IDATAPICKUP", IDATAPICKUP, 0, 0 },
	{"IDATATILEH", IDATATILEH, 0, 0 },
	{"IDATATILEW", IDATATILEW, 0, 0 },
	{"IDATAHZSZ", IDATAHZSZ, 0, 0 },
	{"IDATAHYSZ", IDATAHYSZ, 0, 0 },
	{"IDATAHXSZ", IDATAHXSZ, 0, 0 },
	{"IDATADYOFS", IDATADYOFS, 0, 0 },
	{"IDATADXOFS", IDATADXOFS, 0, 0 },
	{"IDATAHYOFS", IDATAHYOFS, 0, 0 },
	{"IDATAHXOFS", IDATAHXOFS, 0, 0 },
	//spritedata sd->
	{"SPRITEDATATILE", SPRITEDATATILE, 0, 0 },
	{"SPRITEDATAMISC", SPRITEDATAMISC, 0, 0 },
	{"SPRITEDATACSETS", SPRITEDATACSETS, 0, 0 },
	{"SPRITEDATAFRAMES", SPRITEDATAFRAMES, 0, 0 },
	{"SPRITEDATASPEED", SPRITEDATASPEED, 0, 0 },
	{"SPRITEDATATYPE", SPRITEDATATYPE, 0, 0 },
	
	//npcdata nd->
	{"NPCDATATILE", NPCDATATILE, 0, 0 },
	{"NPCDATAHEIGHT", NPCDATAHEIGHT, 0, 0 },
	{"NPCDATAFLAGS", NPCDATAFLAGS, 0, 0 },
	{"NPCDATAFLAGS2", NPCDATAFLAGS2, 0, 0 },
	{"NPCDATAWIDTH", NPCDATAWIDTH, 0, 0 },
	{"NPCDATAHITSFX", NPCDATAHITSFX, 0, 0 },
	{"NPCDATASTILE", NPCDATASTILE, 0, 0 },
	{"NPCDATASWIDTH", NPCDATASWIDTH, 0, 0 },
	{"NPCDATASHEIGHT", NPCDATASHEIGHT, 0, 0 },
	{"NPCDATAETILE", NPCDATAETILE, 0, 0 },
	{"NPCDATAEWIDTH", NPCDATAEWIDTH, 0, 0 },
	{"NPCDATAEHEIGHT", NPCDATAEHEIGHT, 0, 0 },
	{"NPCDATAHP", NPCDATAHP, 0, 0 },
	{"NPCDATAFAMILY", NPCDATAFAMILY, 0, 0 },
	{"NPCDATACSET", NPCDATACSET, 0, 0 },
	{"NPCDATAANIM", NPCDATAANIM, 0, 0 },
	{"NPCDATAEANIM", NPCDATAEANIM, 0, 0 },
	{"NPCDATAFRAMERATE", NPCDATAFRAMERATE, 0, 0 },
	{"NPCDATAEFRAMERATE", NPCDATAEFRAMERATE, 0, 0 },
	{"NPCDATATOUCHDAMAGE", NPCDATATOUCHDAMAGE, 0, 0 },
	{"NPCDATAWEAPONDAMAGE", NPCDATAWEAPONDAMAGE, 0, 0 },
	{"NPCDATAWEAPON", NPCDATAWEAPON, 0, 0 },
	{"NPCDATARANDOM", NPCDATARANDOM, 0, 0 },
	{"NPCDATAHALT", NPCDATAHALT, 0, 0 },
	{"NPCDATASTEP", NPCDATASTEP, 0, 0 },
	{"NPCDATAHOMING", NPCDATAHOMING, 0, 0 },
	{"NPCDATAHUNGER", NPCDATAHUNGER, 0, 0 },
	{"NPCDATADROPSET", NPCDATADROPSET, 0, 0 },
	{"NPCDATABGSFX", NPCDATABGSFX, 0, 0 },
	{"NPCDATADEATHSFX", NPCDATADEATHSFX, 0, 0 },
	{"NPCDATAXOFS", NPCDATAXOFS, 0, 0 },
	{"NPCDATAYOFS", NPCDATAYOFS, 0, 0 },
	{"NPCDATAZOFS", NPCDATAZOFS, 0, 0 },
	{"NPCDATAHXOFS", NPCDATAHXOFS, 0, 0 },
	{"NPCDATAHYOFS", NPCDATAHYOFS, 0, 0 },
	{"NPCDATAHITWIDTH", NPCDATAHITWIDTH, 0, 0 },
	{"NPCDATAHITHEIGHT", NPCDATAHITHEIGHT, 0, 0 },
	{"NPCDATAHITZ", NPCDATAHITZ, 0, 0 },
	{"NPCDATATILEWIDTH", NPCDATATILEWIDTH, 0, 0 },
	{"NPCDATATILEHEIGHT", NPCDATATILEHEIGHT, 0, 0 },
	{"NPCDATAWPNSPRITE", NPCDATAWPNSPRITE, 0, 0 },
	{"NPCDATADEFENSE", NPCDATADEFENSE, 0, 0 },
	{"NPCDATASIZEFLAG", NPCDATASIZEFLAG, 0, 0 },
	{"NPCDATAATTRIBUTE", NPCDATAATTRIBUTE, 0, 0 },
	
	{"NPCDATAFROZENTILE", NPCDATAFROZENTILE, 0, 0 },
	{"NPCDATAFROZENCSET", NPCDATAFROZENCSET, 0, 0 },
	
	//mapdata md->
	{"MAPDATAVALID", MAPDATAVALID, 0, 0 },
	{"MAPDATAGUY", MAPDATAGUY, 0, 0 },
	{"MAPDATASTRING", MAPDATASTRING, 0, 0 },
	{"MAPDATAROOM", MAPDATAROOM, 0, 0 },
	{"MAPDATAITEM", MAPDATAITEM, 0, 0 },
	{"MAPDATAHASITEM", MAPDATAHASITEM, 0, 0 },
	{"MAPDATATILEWARPTYPE", MAPDATATILEWARPTYPE, 0, 0 },
	{"MAPDATATILEWARPOVFLAGS", MAPDATATILEWARPOVFLAGS, 0, 0 },
	{"MAPDATADOORCOMBOSET", MAPDATADOORCOMBOSET, 0, 0 },
	{"MAPDATAWARPRETX", MAPDATAWARPRETX, 0, 0 },
	{"MAPDATAWARPRETY", MAPDATAWARPRETY, 0, 0 },
	{"MAPDATAWARPRETURNC", MAPDATAWARPRETURNC, 0, 0 },
	{"MAPDATASTAIRX", MAPDATASTAIRX, 0, 0 },
	{"MAPDATASTAIRY", MAPDATASTAIRY, 0, 0 },
	{"MAPDATACOLOUR", MAPDATACOLOUR, 0, 0 },
	{"MAPDATAENEMYFLAGS", MAPDATAENEMYFLAGS, 0, 0 },
	{"MAPDATADOOR", MAPDATADOOR, 0, 0 },
	{"MAPDATATILEWARPDMAP", MAPDATATILEWARPDMAP, 0, 0 },
	{"MAPDATATILEWARPSCREEN", MAPDATATILEWARPSCREEN, 0, 0 },
	{"MAPDATAEXITDIR", MAPDATAEXITDIR, 0, 0 },
	{"MAPDATAENEMY", MAPDATAENEMY, 0, 0 },
	{"MAPDATAPATTERN", MAPDATAPATTERN, 0, 0 },
	{"MAPDATASIDEWARPTYPE", MAPDATASIDEWARPTYPE, 0, 0 },
	{"MAPDATASIDEWARPOVFLAGS", MAPDATASIDEWARPOVFLAGS, 0, 0 },
	{"MAPDATAWARPARRIVALX", MAPDATAWARPARRIVALX, 0, 0 },
	{"MAPDATAWARPARRIVALY", MAPDATAWARPARRIVALY, 0, 0 },
	{"MAPDATAPATH", MAPDATAPATH, 0, 0 },
	{"MAPDATASIDEWARPSC", MAPDATASIDEWARPSC, 0, 0 },
	{"MAPDATASIDEWARPDMAP", MAPDATASIDEWARPDMAP, 0, 0 },
	{"MAPDATASIDEWARPINDEX", MAPDATASIDEWARPINDEX, 0, 0 },
	{"MAPDATAUNDERCOMBO", MAPDATAUNDERCOMBO, 0, 0 },
	{"MAPDATAUNDERCSET", MAPDATAUNDERCSET, 0, 0 },
	{"MAPDATACATCHALL", MAPDATACATCHALL, 0, 0 },
	{"MAPDATAFLAGS", MAPDATAFLAGS, 0, 0 },
	{"MAPDATACSENSITIVE", MAPDATACSENSITIVE, 0, 0 },
	{"MAPDATANORESET", MAPDATANORESET, 0, 0 },
	{"MAPDATANOCARRY", MAPDATANOCARRY, 0, 0 },
	{"MAPDATALAYERMAP", MAPDATALAYERMAP, 0, 0 },
	{"MAPDATALAYERSCREEN", MAPDATALAYERSCREEN, 0, 0 },
	{"MAPDATALAYEROPACITY", MAPDATALAYEROPACITY, 0, 0 },
	{"MAPDATATIMEDWARPTICS", MAPDATATIMEDWARPTICS, 0, 0 },
	{"MAPDATANEXTMAP", MAPDATANEXTMAP, 0, 0 },
	{"MAPDATANEXTSCREEN", MAPDATANEXTSCREEN, 0, 0 },
	{"MAPDATASECRETCOMBO", MAPDATASECRETCOMBO, 0, 0 },
	{"MAPDATASECRETCSET", MAPDATASECRETCSET, 0, 0 },
	{"MAPDATASECRETFLAG", MAPDATASECRETFLAG, 0, 0 },
	{"MAPDATAVIEWX", MAPDATAVIEWX, 0, 0 },
	{"MAPDATAVIEWY", MAPDATAVIEWY, 0, 0 },
	{"MAPDATASCREENWIDTH", MAPDATASCREENWIDTH, 0, 0 },
	{"MAPDATASCREENHEIGHT", MAPDATASCREENHEIGHT, 0, 0 },
	{"MAPDATAENTRYX", MAPDATAENTRYX, 0, 0 },
	{"MAPDATAENTRYY", MAPDATAENTRYY, 0, 0 },
	{"MAPDATANUMFF", MAPDATANUMFF, 0, 0 },
	{"MAPDATAFFDATA", MAPDATAFFDATA, 0, 0 },
	{"MAPDATAFFCSET", MAPDATAFFCSET, 0, 0 },
	{"MAPDATAFFDELAY", MAPDATAFFDELAY, 0, 0 },
	{"MAPDATAFFX", MAPDATAFFX, 0, 0 },
	{"MAPDATAFFY", MAPDATAFFY, 0, 0 },
	{"MAPDATAFFXDELTA", MAPDATAFFXDELTA, 0, 0 },
	{"MAPDATAFFYDELTA", MAPDATAFFYDELTA, 0, 0 },
	{"MAPDATAFFXDELTA2", MAPDATAFFXDELTA2, 0, 0 },
	{"MAPDATAFFYDELTA2", MAPDATAFFYDELTA2, 0, 0 },
	{"MAPDATAFFFLAGS", MAPDATAFFFLAGS, 0, 0 },
	{"MAPDATAFFWIDTH", MAPDATAFFWIDTH, 0, 0 },
	{"MAPDATAFFHEIGHT", MAPDATAFFHEIGHT, 0, 0 },
	{"MAPDATAFFLINK", MAPDATAFFLINK, 0, 0 },
	{"MAPDATAFFSCRIPT", MAPDATAFFSCRIPT, 0, 0 },
	{"MAPDATAINTID", MAPDATAINTID, 0, 0 }, //Needs to be a function [32][10]
	{"MAPDATAINITA", MAPDATAINITA, 0, 0 }, //needs to be a function, [32][2]
	{"MAPDATAFFINITIALISED", MAPDATAFFINITIALISED, 0, 0 },
	{"MAPDATASCRIPTENTRY", MAPDATASCRIPTENTRY, 0, 0 },
	{"MAPDATASCRIPTOCCUPANCY", MAPDATASCRIPTOCCUPANCY, 0, 0 },
	{"MAPDATASCRIPTEXIT", MAPDATASCRIPTEXIT, 0, 0 },
	{"MAPDATAOCEANSFX", MAPDATAOCEANSFX, 0, 0 },
	{"MAPDATABOSSSFX", MAPDATABOSSSFX, 0, 0 },
	{"MAPDATASECRETSFX", MAPDATASECRETSFX, 0, 0 },
	{"MAPDATAHOLDUPSFX", MAPDATAHOLDUPSFX, 0, 0 },
	{"MAPDATASCREENMIDI", MAPDATASCREENMIDI, 0, 0 },
	{"MAPDATALENSLAYER", MAPDATALENSLAYER, 0, 0 },
	{"MAPDATAMISCD", MAPDATAMISCD, 0, 0},
	
	{"MAPDATASCREENSTATED", MAPDATASCREENSTATED, 0, 0},
	{"MAPDATASCREENFLAGSD", MAPDATASCREENFLAGSD, 0, 0},
	{"MAPDATASCREENEFLAGSD", MAPDATASCREENEFLAGSD, 0, 0},
	
	
	
	{"MAPDATACOMBODD", MAPDATACOMBODD, 0, 0},
	{"MAPDATACOMBOCD", MAPDATACOMBOCD, 0, 0},
	{"MAPDATACOMBOFD", MAPDATACOMBOFD, 0, 0},
	{"MAPDATACOMBOTD", MAPDATACOMBOTD, 0, 0},
	{"MAPDATACOMBOID", MAPDATACOMBOID, 0, 0},
	{"MAPDATACOMBOSD", MAPDATACOMBOSD, 0, 0},
	
	{"SCREENDATAVALID", SCREENDATAVALID, 0, 0 },
	{"SCREENDATAGUY", SCREENDATAGUY, 0, 0 },
	{"SCREENDATASTRING", SCREENDATASTRING, 0, 0 },
	{"SCREENDATAROOM", SCREENDATAROOM, 0, 0 },
	{"SCREENDATAITEM", SCREENDATAITEM, 0, 0 },
	{"SCREENDATAHASITEM", SCREENDATAHASITEM, 0, 0 },
	{"SCREENDATATILEWARPTYPE", SCREENDATATILEWARPTYPE, 0, 0 },
	{"SCREENDATATILEWARPOVFLAGS", SCREENDATATILEWARPOVFLAGS, 0, 0 },
	{"SCREENDATADOORCOMBOSET", SCREENDATADOORCOMBOSET, 0, 0 },
	{"SCREENDATAWARPRETX", SCREENDATAWARPRETX, 0, 0 },
	{"SCREENDATAWARPRETY", SCREENDATAWARPRETY, 0, 0 },
	{"SCREENDATAWARPRETURNC", SCREENDATAWARPRETURNC, 0, 0 },
	{"SCREENDATASTAIRX", SCREENDATASTAIRX, 0, 0 },
	{"SCREENDATASTAIRY", SCREENDATASTAIRY, 0, 0 },
	{"SCREENDATACOLOUR", SCREENDATACOLOUR, 0, 0 },
	{"SCREENDATAENEMYFLAGS", SCREENDATAENEMYFLAGS, 0, 0 },
	{"SCREENDATADOOR", SCREENDATADOOR, 0, 0 },
	{"SCREENDATATILEWARPDMAP", SCREENDATATILEWARPDMAP, 0, 0 },
	{"SCREENDATATILEWARPSCREEN", SCREENDATATILEWARPSCREEN, 0, 0 },
	{"SCREENDATAEXITDIR", SCREENDATAEXITDIR, 0, 0 },
	{"SCREENDATAENEMY", SCREENDATAENEMY, 0, 0 },
	{"SCREENDATAPATTERN", SCREENDATAPATTERN, 0, 0 },
	{"SCREENDATASIDEWARPTYPE", SCREENDATASIDEWARPTYPE, 0, 0 },
	{"SCREENDATASIDEWARPOVFLAGS", SCREENDATASIDEWARPOVFLAGS, 0, 0 },
	{"SCREENDATAWARPARRIVALX", SCREENDATAWARPARRIVALX, 0, 0 },
	{"SCREENDATAWARPARRIVALY", SCREENDATAWARPARRIVALY, 0, 0 },
	{"SCREENDATAPATH", SCREENDATAPATH, 0, 0 },
	{"SCREENDATASIDEWARPSC", SCREENDATASIDEWARPSC, 0, 0 },
	{"SCREENDATASIDEWARPDMAP", SCREENDATASIDEWARPDMAP, 0, 0 },
	{"SCREENDATASIDEWARPINDEX", SCREENDATASIDEWARPINDEX, 0, 0 },
	{"SCREENDATAUNDERCOMBO", SCREENDATAUNDERCOMBO, 0, 0 },
	{"SCREENDATAUNDERCSET", SCREENDATAUNDERCSET, 0, 0 },
	{"SCREENDATACATCHALL", SCREENDATACATCHALL, 0, 0 },
	{"SCREENDATAFLAGS", SCREENDATAFLAGS, 0, 0 },
	{"SCREENDATACSENSITIVE", SCREENDATACSENSITIVE, 0, 0 },
	{"SCREENDATANORESET", SCREENDATANORESET, 0, 0 },
	{"SCREENDATANOCARRY", SCREENDATANOCARRY, 0, 0 },
	{"SCREENDATALAYERMAP", SCREENDATALAYERMAP, 0, 0 },
	{"SCREENDATALAYERSCREEN", SCREENDATALAYERSCREEN, 0, 0 },
	{"SCREENDATALAYEROPACITY", SCREENDATALAYEROPACITY, 0, 0 },
	{"SCREENDATATIMEDWARPTICS", SCREENDATATIMEDWARPTICS, 0, 0 },
	{"SCREENDATANEXTMAP", SCREENDATANEXTMAP, 0, 0 },
	{"SCREENDATANEXTSCREEN", SCREENDATANEXTSCREEN, 0, 0 },
	{"SCREENDATASECRETCOMBO", SCREENDATASECRETCOMBO, 0, 0 },
	{"SCREENDATASECRETCSET", SCREENDATASECRETCSET, 0, 0 },
	{"SCREENDATASECRETFLAG", SCREENDATASECRETFLAG, 0, 0 },
	{"SCREENDATAVIEWX", SCREENDATAVIEWX, 0, 0 },
	{"SCREENDATAVIEWY", SCREENDATAVIEWY, 0, 0 },
	{"SCREENDATASCREENWIDTH", SCREENDATASCREENWIDTH, 0, 0 },
	{"SCREENDATASCREENHEIGHT", SCREENDATASCREENHEIGHT, 0, 0 },
	{"SCREENDATAENTRYX", SCREENDATAENTRYX, 0, 0 },
	{"SCREENDATAENTRYY", SCREENDATAENTRYY, 0, 0 },
	{"SCREENDATANUMFF", SCREENDATANUMFF, 0, 0 },
	{"SCREENDATAFFDATA", SCREENDATAFFDATA, 0, 0 },
	{"SCREENDATAFFCSET", SCREENDATAFFCSET, 0, 0 },
	{"SCREENDATAFFDELAY", SCREENDATAFFDELAY, 0, 0 },
	{"SCREENDATAFFX", SCREENDATAFFX, 0, 0 },
	{"SCREENDATAFFY", SCREENDATAFFY, 0, 0 },
	{"SCREENDATAFFXDELTA", SCREENDATAFFXDELTA, 0, 0 },
	{"SCREENDATAFFYDELTA", SCREENDATAFFYDELTA, 0, 0 },
	{"SCREENDATAFFXDELTA2", SCREENDATAFFXDELTA2, 0, 0 },
	{"SCREENDATAFFYDELTA2", SCREENDATAFFYDELTA2, 0, 0 },
	{"SCREENDATAFFFLAGS", SCREENDATAFFFLAGS, 0, 0 },
	{"SCREENDATAFFWIDTH", SCREENDATAFFWIDTH, 0, 0 },
	{"SCREENDATAFFHEIGHT", SCREENDATAFFHEIGHT, 0, 0 },
	{"SCREENDATAFFLINK", SCREENDATAFFLINK, 0, 0 },
	{"SCREENDATAFFSCRIPT", SCREENDATAFFSCRIPT, 0, 0 },
	{"SCREENDATAINTID", SCREENDATAINTID, 0, 0 }, //Needs to be a function [32][10]
	{"SCREENDATAINITA", SCREENDATAINITA, 0, 0 }, //needs to be a function, [32][2]
	{"SCREENDATAFFINITIALISED", SCREENDATAFFINITIALISED, 0, 0 },
	{"SCREENDATASCRIPTENTRY", SCREENDATASCRIPTENTRY, 0, 0 },
	{"SCREENDATASCRIPTOCCUPANCY", SCREENDATASCRIPTOCCUPANCY, 0, 0 },
	{"SCREENDATASCRIPTEXIT", SCREENDATASCRIPTEXIT, 0, 0 },
	{"SCREENDATAOCEANSFX", SCREENDATAOCEANSFX, 0, 0 },
	{"SCREENDATABOSSSFX", SCREENDATABOSSSFX, 0, 0 },
	{"SCREENDATASECRETSFX", SCREENDATASECRETSFX, 0, 0 },
	{"SCREENDATAHOLDUPSFX", SCREENDATAHOLDUPSFX, 0, 0 },
	{"SCREENDATASCREENMIDI", SCREENDATASCREENMIDI, 0, 0 },
	{"SCREENDATALENSLAYER", SCREENDATALENSLAYER, 0, 0 },
	
	{"LINKSCRIPTTILE", LINKSCRIPTTILE, 0, 0 },
	{"LINKSCRIPFLIP", LINKSCRIPFLIP, 0, 0 },
	{"MAPDATAITEMX", MAPDATAITEMX, 0, 0 },
	{"MAPDATAITEMY", MAPDATAITEMY, 0, 0 },
	{"SCREENDATAITEMX", SCREENDATAITEMX, 0, 0 },
	{"SCREENDATAITEMY", SCREENDATAITEMY, 0, 0 },
	
	{"MAPDATAFFEFFECTWIDTH", MAPDATAFFEFFECTWIDTH, 0, 0 },
	{"MAPDATAFFEFFECTHEIGHT", MAPDATAFFEFFECTHEIGHT, 0, 0 },
	{"SCREENDATAFFEFFECTWIDTH", SCREENDATAFFEFFECTWIDTH, 0, 0 },
	{"SCREENDATAFFEFFECTHEIGHT", SCREENDATAFFEFFECTHEIGHT, 0, 0 },
	
	{"LOADMAPDATA", LOADMAPDATA, 0, 0 },
	{"LWPNPARENT", LWPNPARENT, 0, 0 },
	{"LWPNLEVEL", LWPNLEVEL, 0, 0 },
	{"EWPNLEVEL", EWPNLEVEL, 0, 0 },
	{"EWPNPARENT", EWPNPARENT, 0, 0 },
	{"NPCPARENTUID", NPCPARENTUID, 0, 0 },
	
	
	{"SHOPDATANAME", SHOPDATANAME, 0, 0 },
	{"SHOPDATAITEM", SHOPDATAITEM, 0, 0 },
	{"SHOPDATAHASITEM", SHOPDATAHASITEM, 0, 0 },
	{"SHOPDATAPRICE", SHOPDATAPRICE, 0, 0 },
	{"SHOPDATASTRING", SHOPDATASTRING, 0, 0 },
	
	{"NPCDATASHIELD", NPCDATASHIELD, 0, 0 },
	{"NPCSHIELD", NPCSHIELD, 0, 0 },
	{"AUDIOVOLUME", AUDIOVOLUME, 0, 0 },
	{"AUDIOPAN", AUDIOPAN, 0, 0 },
	
	{"MESSAGEDATANEXT", MESSAGEDATANEXT, 0, 0 },
	{"MESSAGEDATATILE", MESSAGEDATATILE, 0, 0 },
	{"MESSAGEDATACSET", MESSAGEDATACSET, 0, 0 },
	{"MESSAGEDATATRANS", MESSAGEDATATRANS, 0, 0 },
	{"MESSAGEDATAFONT", MESSAGEDATAFONT, 0, 0 },
	{"MESSAGEDATAX", MESSAGEDATAX, 0, 0 },
	{"MESSAGEDATAY", MESSAGEDATAY, 0, 0 },
	{"MESSAGEDATAW", MESSAGEDATAW, 0, 0 },
	{"MESSAGEDATAH", MESSAGEDATAH, 0, 0 },
	{"MESSAGEDATASFX", MESSAGEDATASFX, 0, 0 },
	{"MESSAGEDATALISTPOS", MESSAGEDATALISTPOS, 0, 0 },
	{"MESSAGEDATAVSPACE", MESSAGEDATAVSPACE, 0, 0 },
	{"MESSAGEDATAHSPACE", MESSAGEDATAHSPACE, 0, 0 },
	{"MESSAGEDATAFLAGS", MESSAGEDATAFLAGS, 0, 0 },
	
	{"DMAPDATAMAP", DMAPDATAMAP, 0, 0 },
	{"DMAPDATALEVEL", DMAPDATALEVEL, 0, 0 },
	{"DMAPDATAOFFSET", DMAPDATAOFFSET, 0, 0 },
	{"DMAPDATACOMPASS", DMAPDATACOMPASS, 0, 0 },
	{"DMAPDATAPALETTE", DMAPDATAPALETTE, 0, 0 },
	{"DMAPDATAMIDI", DMAPDATAMIDI, 0, 0 },
	{"DMAPDATACONTINUE", DMAPDATACONTINUE, 0, 0 },
	{"DMAPDATATYPE", DMAPDATATYPE, 0, 0 },
	{"DMAPDATAGRID", DMAPDATAGRID, 0, 0 },
	{"DMAPDATAMINIMAPTILE", DMAPDATAMINIMAPTILE, 0, 0 },
	{"DMAPDATAMINIMAPCSET", DMAPDATAMINIMAPCSET, 0, 0 },
	{"DMAPDATALARGEMAPTILE", DMAPDATALARGEMAPTILE, 0, 0 },
	{"DMAPDATALARGEMAPCSET", DMAPDATALARGEMAPCSET, 0, 0 },
	{"DMAPDATAMUISCTRACK", DMAPDATAMUISCTRACK, 0, 0 },
	{"DMAPDATASUBSCRA", DMAPDATASUBSCRA, 0, 0 },
	{"DMAPDATASUBSCRP", DMAPDATASUBSCRP, 0, 0 },
	{"DMAPDATADISABLEDITEMS", DMAPDATADISABLEDITEMS, 0, 0 },
	{"DMAPDATAFLAGS", DMAPDATAFLAGS, 0, 0 },
	{"DMAPDATAID", DMAPDATAID, 0, 0 },
	
	{"NPCFROZEN", NPCFROZEN, 0, 0 },
	{"NPCFROZENTILE", NPCFROZENTILE, 0, 0 },
	{"NPCFROZENCSET", NPCFROZENCSET, 0, 0 },
	
	{"ITEMPSTRING", ITEMPSTRING, 0, 0 },
	{"ITEMPSTRINGFLAGS", ITEMPSTRINGFLAGS, 0, 0 },
	{"ITEMOVERRIDEFLAGS", ITEMOVERRIDEFLAGS, 0, 0 },
	{"LINKPUSH", LINKPUSH, 0, 0 },
	{"LINKSTUN", LINKSTUN, 0, 0 },
	{"IDATACOSTCOUNTER", IDATACOSTCOUNTER, 0, 0 },
	{"TYPINGMODE", TYPINGMODE, 0, 0 },
// {"DMAPDATAGRAVITY", DMAPDATAGRAVITY, 0, 0 },
// {"DMAPDATAJUMPLAYER", DMAPDATAJUMPLAYER, 0, 0 },
	//end ffscript vars
	//END VARS END OF BYTECODE
	
	//newcombo
	{"COMBODTILE", COMBODTILE, 0, 0 },
	{"COMBODFLIP", COMBODFLIP, 0, 0 },
	{"COMBODWALK", COMBODWALK, 0, 0 },
	{"COMBODTYPE", COMBODTYPE, 0, 0 },
	{"COMBODCSET", COMBODCSET, 0, 0 },
	{"COMBODFOO", COMBODFOO, 0, 0 },
	{"COMBODFRAMES", COMBODFRAMES, 0, 0 },
	{"COMBODNEXTD", COMBODNEXTD, 0, 0 },
	{"COMBODNEXTC", COMBODNEXTC, 0, 0 },
	{"COMBODFLAG", COMBODFLAG, 0, 0 },
	{"COMBODSKIPANIM", COMBODSKIPANIM, 0, 0 },
	{"COMBODNEXTTIMER", COMBODNEXTTIMER, 0, 0 },
	{"COMBODAKIMANIMY", COMBODAKIMANIMY, 0, 0 },
	{"COMBODANIMFLAGS", COMBODANIMFLAGS, 0, 0 },
	{"COMBODEXPANSION", COMBODEXPANSION, 0, 0 },
	{"COMBODATTRIBUTES", COMBODATTRIBUTES, 0, 0 },
	{"COMBODATTRIBYTES", COMBODATTRIBYTES, 0, 0 },
	{"COMBODUSRFLAGS", COMBODUSRFLAGS, 0, 0 },
	{"COMBODTRIGGERFLAGS", COMBODTRIGGERFLAGS, 0, 0 },
	{"COMBODTRIGGERLEVEL", COMBODTRIGGERLEVEL, 0, 0 },

	//comboclass
	{"COMBODNAME", COMBODNAME, 0, 0 },
	{"COMBODBLOCKNPC", COMBODBLOCKNPC, 0, 0 },
	{"COMBODBLOCKHOLE", COMBODBLOCKHOLE, 0, 0 },
	{"COMBODBLOCKTRIG", COMBODBLOCKTRIG, 0, 0 },
	{"COMBODBLOCKWEAPON", COMBODBLOCKWEAPON, 0, 0 },
	{"COMBODCONVXSPEED", COMBODCONVXSPEED, 0, 0 },
	{"COMBODCONVYSPEED", COMBODCONVYSPEED, 0, 0 },
	{"COMBODSPAWNNPC", COMBODSPAWNNPC, 0, 0 },
	{"COMBODSPAWNNPCWHEN", COMBODSPAWNNPCWHEN, 0, 0 },
	{"COMBODSPAWNNPCCHANGE", COMBODSPAWNNPCCHANGE, 0, 0 },
	{"COMBODDIRCHANGETYPE", COMBODDIRCHANGETYPE, 0, 0 },
	{"COMBODDISTANCECHANGETILES", COMBODDISTANCECHANGETILES, 0, 0 },
	{"COMBODDIVEITEM", COMBODDIVEITEM, 0, 0 },
	{"COMBODDOCK", COMBODDOCK, 0, 0 },
	{"COMBODFAIRY", COMBODFAIRY, 0, 0 },
	{"COMBODFFATTRCHANGE", COMBODFFATTRCHANGE, 0, 0 },
	{"COMBODFOORDECOTILE", COMBODFOORDECOTILE, 0, 0 },
	{"COMBODFOORDECOTYPE", COMBODFOORDECOTYPE, 0, 0 },
	{"COMBODHOOKSHOTPOINT", COMBODHOOKSHOTPOINT, 0, 0 },
	{"COMBODLADDERPASS", COMBODLADDERPASS, 0, 0 },
	{"COMBODLOCKBLOCK", COMBODLOCKBLOCK, 0, 0 },
	{"COMBODLOCKBLOCKCHANGE", COMBODLOCKBLOCKCHANGE, 0, 0 },
	{"COMBODMAGICMIRROR", COMBODMAGICMIRROR, 0, 0 },
	{"COMBODMODHPAMOUNT", COMBODMODHPAMOUNT, 0, 0 },
	{"COMBODMODHPDELAY", COMBODMODHPDELAY, 0, 0 },
	{"COMBODMODHPTYPE", COMBODMODHPTYPE, 0, 0 },
	{"COMBODNMODMPAMOUNT", COMBODNMODMPAMOUNT, 0, 0 },
	{"COMBODMODMPDELAY", COMBODMODMPDELAY, 0, 0 },
	{"COMBODMODMPTYPE", COMBODMODMPTYPE, 0, 0 },
	{"COMBODNOPUSHBLOCK", COMBODNOPUSHBLOCK, 0, 0 },
	{"COMBODOVERHEAD", COMBODOVERHEAD, 0, 0 },
	{"COMBODPLACENPC", COMBODPLACENPC, 0, 0 },
	{"COMBODPUSHDIR", COMBODPUSHDIR, 0, 0 },
	{"COMBODPUSHWAIT", COMBODPUSHWAIT, 0, 0 },
	{"COMBODPUSHHEAVY", COMBODPUSHHEAVY, 0, 0 },
	{"COMBODPUSHED", COMBODPUSHED, 0, 0 },
	{"COMBODRAFT", COMBODRAFT, 0, 0 },
	{"COMBODRESETROOM", COMBODRESETROOM, 0, 0 },
	{"COMBODSAVEPOINTTYPE", COMBODSAVEPOINTTYPE, 0, 0 },
	{"COMBODSCREENFREEZETYPE", COMBODSCREENFREEZETYPE, 0, 0 },
	{"COMBODSECRETCOMBO", COMBODSECRETCOMBO, 0, 0 },
	{"COMBODSINGULAR", COMBODSINGULAR, 0, 0 },
	{"COMBODSLOWWALK", COMBODSLOWWALK, 0, 0 },
	{"COMBODSTATUETYPEs", COMBODSTATUETYPE, 0, 0 },
	{"COMBODSTEPTYPE", COMBODSTEPTYPE, 0, 0 },
	{"COMBODSTEPCHANGEINTO", COMBODSTEPCHANGEINTO, 0, 0 },
	{"COMBODSTRIKEWEAPONS", COMBODSTRIKEWEAPONS, 0, 0 },
	{"COMBODSTRIKEREMNANTS", COMBODSTRIKEREMNANTS, 0, 0 },
	{"COMBODSTRIKEREMNANTSTYPE", COMBODSTRIKEREMNANTSTYPE, 0, 0 },
	{"COMBODSTRIKECHANGE", COMBODSTRIKECHANGE, 0, 0 },
	{"COMBODSTRIKEITEM", COMBODSTRIKEITEM, 0, 0 },
	{"COMBODTOUCHITEM", COMBODTOUCHITEM, 0, 0 },
	{"COMBODTOUCHSTAIRS", COMBODTOUCHSTAIRS, 0, 0 },
	{"COMBODTRIGGERTYPE", COMBODTRIGGERTYPE, 0, 0 },
	{"COMBODTRIGGERSENS", COMBODTRIGGERSENS, 0, 0 },
	{"COMBODWARPTYPE", COMBODWARPTYPE, 0, 0 },
	{"COMBODWARPSENS", COMBODWARPSENS, 0, 0 },
	{"COMBODWARPDIRECT", COMBODWARPDIRECT, 0, 0 },
	{"COMBODWARPLOCATION", COMBODWARPLOCATION, 0, 0 },
	{"COMBODWATER", COMBODWATER, 0, 0 },
	{"COMBODWHISTLE", COMBODWHISTLE, 0, 0 },
	{"COMBODWINGAME", COMBODWINGAME, 0, 0 },
	{"COMBODBLOCKWPNLEVEL", COMBODBLOCKWPNLEVEL, 0, 0 },

	{"TYPINGMODE", TYPINGMODE, 0, 0 },
	{"TYPINGMODE", TYPINGMODE, 0, 0 },
	{"LINKHITBY", LINKHITBY, 0, 0 },
	{"LINKDEFENCE", LINKDEFENCE, 0, 0 },
	{"NPCHITBY", NPCHITBY, 0, 0 },
	{"NPCISCORE", NPCISCORE, 0, 0 },
	{"NPCSCRIPTUID", NPCSCRIPTUID, 0, 0 },
	{"LWEAPONSCRIPTUID", LWEAPONSCRIPTUID, 0, 0 },
	{"EWEAPONSCRIPTUID", EWEAPONSCRIPTUID, 0, 0 },
	{"ITEMSCRIPTUID", ITEMSCRIPTUID, 0, 0 },
	
	{"DMAPDATASIDEVIEW", DMAPDATASIDEVIEW, 0, 0 },
	
	{"DONULL", DONULL, 0, 0 },
	{"DEBUGD", DEBUGD, 0, 0 },
	{"GETPIXEL", GETPIXEL, 0, 0 },
	{"DOUNTYPE", DOUNTYPE, 0, 0 },
	{"NPCBEHAVIOUR", NPCBEHAVIOUR, 0, 0 },
	{"NPCDATABEHAVIOUR", NPCDATABEHAVIOUR, 0, 0 },
	{"CREATEBITMAP", CREATEBITMAP, 0, 0 },
	{"LINKTILEMOD", LINKTILEMOD, 0, 0 },
	{"NPCINITD", NPCINITD, 0, 0 },
	{"NPCCOLLISION", NPCCOLLISION, 0, 0 },
	{"NPCLINEDUP", NPCLINEDUP, 0, 0 },
	
	{"NPCDATAINITD", NPCLINEDUP, 0, 0 },
	{"NPCDATASCRIPT", NPCDATASCRIPT, 0, 0 },
	{"NPCMATCHINITDLABEL", NPCMATCHINITDLABEL, 0, 0 },
	//lweapon scripts
	{"LWPNSCRIPT", LWPNSCRIPT, 0, 0 },
	{"LWPNINITD", LWPNINITD, 0, 0 },
	{"ITEMFAMILY", ITEMFAMILY, 0, 0 },
	{"ITEMLEVEL", ITEMLEVEL, 0, 0 },
	
	{"EWPNSCRIPT", EWPNSCRIPT, 0, 0 },
	{"EWPNINITD", EWPNINITD, 0, 0 },
	
	{"NPCSCRIPT", NPCSCRIPT, 0, 0 },
	
	{"DMAPSCRIPT", DMAPSCRIPT, 0, 0 },
	{"DMAPINITD", DMAPINITD, 0, 0 },
	
	{"SCREENSCRIPT", SCREENSCRIPT, 0, 0 },
	{"SCREENINITD", SCREENINITD, 0, 0 },
	
	{"LINKINITD", LINKINITD, 0, 0 },
	{"NPCDATAWEAPONINITD", NPCDATAWEAPONINITD, 0, 0 },
	{"NPCDATAWEAPONSCRIPT", NPCDATAWEAPONSCRIPT, 0, 0 },
	
	{"NPCSCRIPTTILE", NPCSCRIPTTILE, 0, 0 },
	{"NPCSCRIPTFLIP", NPCSCRIPTFLIP, 0, 0 },
	{"LWPNSCRIPTTILE", LWPNSCRIPTTILE, 0, 0 },
	{"LWPNSCRIPTFLIP", LWPNSCRIPTFLIP, 0, 0 },
	{"EWPNSCRIPTTILE", EWPNSCRIPTTILE, 0, 0 },
	{"EWPNSCRIPTFLIP", EWPNSCRIPTFLIP, 0, 0 },
	
	{"LINKENGINEANIMATE", LINKENGINEANIMATE, 0, 0 },
	{"NPCENGINEANIMATE", NPCENGINEANIMATE, 0, 0 },
	{"LWPNENGINEANIMATE", LWPNENGINEANIMATE, 0, 0 },
	{"EWPNENGINEANIMATE", EWPNENGINEANIMATE, 0, 0 },
	{"SKIPCREDITS", SKIPCREDITS, 0, 0 },
	{"SKIPF6", SKIPF6, 0, 0 },
	{"LWPNUSEWEAPON", LWPNUSEWEAPON, 0, 0 },
	{"LWPNUSEDEFENCE", LWPNUSEDEFENCE, 0, 0 },
	{"LWPNROTATION", LWPNROTATION, 0, 0 },
	{"EWPNROTATION", EWPNROTATION, 0, 0 },
	{"NPCROTATION", NPCROTATION, 0, 0 },
	{"ITEMROTATION", ITEMROTATION, 0, 0 },
	{"LINKROTATION", LINKROTATION, 0, 0 },
	{"LWPNSCALE", LWPNSCALE, 0, 0 },
	{"EWPNSCALE", EWPNSCALE, 0, 0 },
	{"NPCSCALE", NPCSCALE, 0, 0 },
	{"ITEMSCALE", ITEMSCALE, 0, 0 },
	{"LINKSCALE", LINKSCALE, 0, 0 },
	{"ITEMSPRITESCRIPT", ITEMSPRITESCRIPT, 0, 0 },
	{"FFRULE", FFRULE, 0, 0 },
	{"NUMDRAWS", NUMDRAWS, 0, 0 },
	{"MAXDRAWS", MAXDRAWS, 0, 0 },
	{"BITMAPWIDTH", BITMAPWIDTH, 0, 0 },
	{"BITMAPHEIGHT", BITMAPHEIGHT, 0, 0 },
	{"ALLOCATEBITMAPR", ALLOCATEBITMAPR, 0, 0 },
	{"KEYMODIFIERS", KEYMODIFIERS, 0, 0 },
	{"SIMULATEKEYPRESS", SIMULATEKEYPRESS, 0, 0 },
	{"KEYBINDINGS", KEYBINDINGS, 0, 0 },
	
	{"SCREENSCRIPT", SCREENSCRIPT, 0, 0 },
	{"SCREENINITD", SCREENINITD, 0, 0 },
	{"MAPDATASCRIPT", MAPDATASCRIPT, 0, 0 },
	{"MAPDATAINITDARRAY", MAPDATAINITDARRAY, 0, 0 },
	{"LWPNGRAVITY", LWPNGRAVITY, 0, 0 },
	{"EWPNGRAVITY", EWPNGRAVITY, 0, 0 },
	{"NPCGRAVITY", NPCGRAVITY, 0, 0 },
	{"ITEMGRAVITY", ITEMGRAVITY, 0, 0 },
	
	{"MAPDATASIDEWARPID", MAPDATASIDEWARPID, 0, 0 },
	{"SCREENSIDEWARPID", SCREENSIDEWARPID, 0, 0 },
	{"LINKGRAVITY", LINKGRAVITY, 0, 0 },
	{"SCREENDATALAYERINVIS", SCREENDATALAYERINVIS, 0, 0 },
	{"SCREENDATASCRIPTDRAWS", SCREENDATASCRIPTDRAWS, 0, 0 },
	{"MAPDATALAYERINVIS", MAPDATALAYERINVIS, 0, 0 },
	{"MAPDATASCRIPTDRAWS", MAPDATASCRIPTDRAWS, 0, 0 },
	
	{"ITEMSCRIPTTILE", ITEMSCRIPTTILE, 0, 0 },
	{"ITEMSCRIPTFLIP", ITEMSCRIPTFLIP, 0, 0 },
	{"MAPDATAMAP", MAPDATAMAP, 0, 0 },
	{"MAPDATASCREEN", MAPDATASCREEN, 0, 0 },
	{"IDATAVALIDATE", IDATAVALIDATE, 0, 0 },
	{ "DISABLEKEY", DISABLEKEY, 0, 0 },
	{ "DISABLEBUTTON", DISABLEBUTTON, 0, 0 },
	{ "GAMESUSPEND", GAMESUSPEND, 0, 0 },
	{ "LINKOTILE", LINKOTILE, 0, 0 },
	{ "LINKOFLIP", LINKOFLIP, 0, 0 },
	{ "ITEMSPRITEINITD", ITEMSPRITEINITD, 0, 0 },
	
	{ "ZSCRIPTVERSION", ZSCRIPTVERSION, 0, 0 },
	{ "REFFILE", REFFILE, 0, 0 },
	{ "LINKCLIMBING", LINKCLIMBING, 0, 0 },
	{ "NPCIMMORTAL", NPCIMMORTAL, 0, 0 },
	{ "NPCNOSLIDE", NPCNOSLIDE, 0, 0 },
	{ "NPCKNOCKBACKSPEED", NPCKNOCKBACKSPEED, 0, 0 },
	{ "NPCNOSCRIPTKB", NPCNOSCRIPTKB, 0, 0 },
	{ "GETRENDERTARGET", GETRENDERTARGET, 0, 0 },
	{ "HERONOSTEPFORWARD", HERONOSTEPFORWARD, 0, 0 },
	{ "SCREENDATATWARPRETSQR", SCREENDATATWARPRETSQR, 0, 0 },
	{ "SCREENDATASWARPRETSQR", SCREENDATASWARPRETSQR, 0, 0 },
	{ "MAPDATATWARPRETSQR", MAPDATATWARPRETSQR, 0, 0 },
	{ "MAPDATASWARPRETSQR", MAPDATASWARPRETSQR, 0, 0 },
	{ "NPCSUBMERGED", NPCSUBMERGED, 0, 0 },
	{ "EWPNPARENTUID", EWPNPARENTUID, 0, 0 },
	{ "GAMEGRAVITY", GAMEGRAVITY, 0, 0 },
	{ "COMBODASPEED", COMBODASPEED, 0, 0 },
	{ "DROPSETITEMS", DROPSETITEMS, 0, 0 },
	{ "DROPSETCHANCES", DROPSETCHANCES, 0, 0 },
	{ "DROPSETNULLCHANCE", DROPSETNULLCHANCE, 0, 0 },
	{ "DROPSETCHOOSE", DROPSETCHOOSE, 0, 0 },
	{ "KEYPRESS", KEYPRESS, 0, 0 },
	{ "KEYINPUT", KEYINPUT, 0, 0 },
	{ "SPRITEMAXNPC", SPRITEMAXNPC, 0, 0 },
	{ "SPRITEMAXLWPN", SPRITEMAXLWPN, 0, 0 },
	{ "SPRITEMAXEWPN", SPRITEMAXEWPN, 0, 0 },
	{ "SPRITEMAXITEM", SPRITEMAXITEM, 0, 0 },
	{ "SPRITEMAXPARTICLE", SPRITEMAXPARTICLE, 0, 0 },
	{ "SPRITEMAXDECO", SPRITEMAXDECO, 0, 0 },
	{ "HEROHEALTHBEEP", HEROHEALTHBEEP, 0, 0 },
	{ "NPCRANDOM", NPCRANDOM, 0, 0 },
	{ "COMBOXR", COMBOXR, 0, 0 },
	{ "COMBOYR", COMBOYR, 0, 0 },
	{ "COMBOPOSR", COMBOPOSR, 0, 0 },
	{ "COMBODATASCRIPT", COMBODATASCRIPT, 0, 0 },
	{ "COMBODATAINITD", COMBODATAINITD, 0, 0 },
	{ "HEROSCRIPTCSET", HEROSCRIPTCSET, 0, 0 },
	{ "SHOPDATATYPE", SHOPDATATYPE, 0, 0 },
	{ "HEROSTEPS", HEROSTEPS, 0, 0 },
	{ "HEROSTEPRATE", HEROSTEPRATE, 0, 0 },
	{ "COMBODOTILE", COMBODOTILE, 0, 0 },
	{ "COMBODFRAME", COMBODFRAME, 0, 0 },
	{ "COMBODACLK", COMBODACLK, 0, 0 },
	{ "PC", PC, 0, 0 },
	{ "GAMESCROLLING", GAMESCROLLING, 0, 0 },
	{ "MESSAGEDATAMARGINS", MESSAGEDATAMARGINS, 0, 0 },
	{ "MESSAGEDATAPORTTILE", MESSAGEDATAPORTTILE, 0, 0 },
	{ "MESSAGEDATAPORTCSET", MESSAGEDATAPORTCSET, 0, 0 },
	{ "MESSAGEDATAPORTX", MESSAGEDATAPORTX, 0, 0 },
	{ "MESSAGEDATAPORTY", MESSAGEDATAPORTY, 0, 0 },
	{ "MESSAGEDATAPORTWID", MESSAGEDATAPORTWID, 0, 0 },
	{ "MESSAGEDATAPORTHEI", MESSAGEDATAPORTHEI, 0, 0 },
	{ "MESSAGEDATAFLAGSARR", MESSAGEDATAFLAGSARR, 0, 0 },
	{ "FILEPOS", FILEPOS, 0, 0 },
	{ "FILEEOF", FILEEOF, 0, 0 },
	{ "FILEERR", FILEERR, 0, 0 },
	{ "MESSAGEDATATEXTWID", MESSAGEDATATEXTWID, 0, 0 },
	{ "MESSAGEDATATEXTHEI", MESSAGEDATATEXTHEI, 0, 0 },
	{ "SWITCHKEY", SWITCHKEY, 0, 0 },
	{ "INCQST", INCQST, 0, 0 },
	{ "HEROJUMPCOUNT", HEROJUMPCOUNT, 0, 0 },
	{ "HEROPULLDIR", HEROPULLDIR, 0, 0 },
	{ "HEROPULLCLK", HEROPULLCLK, 0, 0 },
	{ "HEROFALLCLK", HEROFALLCLK, 0, 0 },
	{ "HEROFALLCMB", HEROFALLCMB, 0, 0 },
	{ "HEROMOVEFLAGS", HEROMOVEFLAGS, 0, 0 },
	{ "ITEMFALLCLK", ITEMFALLCLK, 0, 0 },
	{ "ITEMFALLCMB", ITEMFALLCMB, 0, 0 },
	{ "ITEMMOVEFLAGS", ITEMMOVEFLAGS, 0, 0 },
	{ "LWPNFALLCLK", LWPNFALLCLK, 0, 0 },
	{ "LWPNFALLCMB", LWPNFALLCMB, 0, 0 },
	{ "LWPNMOVEFLAGS", LWPNMOVEFLAGS, 0, 0 },
	{ "EWPNFALLCLK", EWPNFALLCLK, 0, 0 },
	{ "EWPNFALLCMB", EWPNFALLCMB, 0, 0 },
	{ "EWPNMOVEFLAGS", EWPNMOVEFLAGS, 0, 0 },
	{ "NPCFALLCLK", NPCFALLCLK, 0, 0 },
	{ "NPCFALLCMB", NPCFALLCMB, 0, 0 },
	{ "NPCMOVEFLAGS", NPCMOVEFLAGS, 0, 0 },
	{ "ISBLANKTILE", ISBLANKTILE, 0, 0 },
	{ "LWPNSPECIAL", LWPNSPECIAL, 0, 0 },
	{ "DMAPDATAASUBSCRIPT", DMAPDATAASUBSCRIPT, 0, 0 },
	{ "DMAPDATAPSUBSCRIPT", DMAPDATAPSUBSCRIPT, 0, 0 },
	{ "DMAPDATASUBINITD", DMAPDATASUBINITD, 0, 0 },
	{ "MODULEGETINT", MODULEGETINT, 0, 0 },
	{ "MODULEGETSTR", MODULEGETSTR, 0, 0 },
	{ "NPCORIGINALHP", NPCORIGINALHP, 0, 0 },
	{ "DMAPDATAMAPSCRIPT", DMAPDATAMAPSCRIPT, 0, 0 },
	{ "DMAPDATAMAPINITD", DMAPDATAMAPINITD, 0, 0 },
	{ "CLOCKCLK", CLOCKCLK, 0, 0 },
	{ "CLOCKACTIVE", CLOCKACTIVE, 0, 0 },
	{ "NPCHITDIR", NPCHITDIR, 0, 0 },
	{ "DMAPDATAFLAGARR", DMAPDATAFLAGARR, 0, 0 },
	{ "LINKCSET", LINKCSET, 0, 0 },
	{ "NPCSLIDECLK", NPCSLIDECLK, 0, 0 },
	{ "NPCFADING", NPCFADING, 0, 0 },
	{ "DISTANCE", DISTANCE, 0, 0 },
	{ "STDARR", STDARR, 0, 0 },
	{ "GHOSTARR", GHOSTARR, 0, 0 },
	{ "TANGOARR", TANGOARR, 0, 0 },
	{ "NPCHALTCLK", NPCHALTCLK, 0, 0 },
	{ "NPCMOVESTATUS", NPCMOVESTATUS, 0, 0 },
	{ "DISTANCESCALE", DISTANCESCALE, 0, 0 },
	{ "DMAPDATACHARTED", DMAPDATACHARTED, 0, 0 },
	{ "REFDIRECTORY", REFDIRECTORY, 0, 0 },
	{ "DIRECTORYSIZE", DIRECTORYSIZE, 0, 0 },
	{ "LONGDISTANCE", LONGDISTANCE, 0, 0 },
	{ "LONGDISTANCESCALE", LONGDISTANCESCALE, 0, 0 },
	{ "COMBOED", COMBOED, 0, 0 },
	{ "MAPDATACOMBOED", MAPDATACOMBOED, 0, 0 },
	{ "COMBODEFFECT", COMBODEFFECT, 0, 0 },
	{ "SCREENSECRETSTRIGGERED", SCREENSECRETSTRIGGERED, 0, 0 },
	{ "ITEMDIR", ITEMDIR, 0, 0 },
	
	{ "NPCFRAME", NPCFRAME, 0, 0 },
	{ "LINKITEMX", LINKITEMX, 0, 0 },
	{ "LINKITEMY", LINKITEMY, 0, 0 },
	{ "ACTIVESSSPEED", ACTIVESSSPEED, 0, 0 },
	{ "HEROISWARPING", HEROISWARPING, 0, 0 },
	
	{ "ITEMGLOWRAD", ITEMGLOWRAD, 0, 0 },
	{ "NPCGLOWRAD", NPCGLOWRAD, 0, 0 },
	{ "LWPNGLOWRAD", LWPNGLOWRAD, 0, 0 },
	{ "EWPNGLOWRAD", EWPNGLOWRAD, 0, 0 },
	{ "ITEMGLOWSHP", ITEMGLOWSHP, 0, 0 },
	{ "NPCGLOWSHP", NPCGLOWSHP, 0, 0 },
	{ "LWPNGLOWSHP", LWPNGLOWSHP, 0, 0 },
	{ "EWPNGLOWSHP", EWPNGLOWSHP, 0, 0 },
	{ "ITEMENGINEANIMATE", ITEMENGINEANIMATE, 0, 0 },
	{ "REFRNG", REFRNG, 0, 0 },
	{ "LWPNUNBL", LWPNUNBL, 0, 0 },
	{ "EWPNUNBL", EWPNUNBL, 0, 0 },
	{ "NPCSHADOWSPR", NPCSHADOWSPR, 0, 0 },
	{ "LWPNSHADOWSPR", LWPNSHADOWSPR, 0, 0 },
	{ "EWPNSHADOWSPR", EWPNSHADOWSPR, 0, 0 },
	{ "ITEMSHADOWSPR", ITEMSHADOWSPR, 0, 0 },
	{ "NPCSPAWNSPR", NPCSPAWNSPR, 0, 0 },
	{ "NPCDEATHSPR", NPCDEATHSPR, 0, 0 },
	{ "NPCDSHADOWSPR", NPCDSHADOWSPR, 0, 0 },
	{ "NPCDSPAWNSPR", NPCDSPAWNSPR, 0, 0 },
	{ "NPCDDEATHSPR", NPCDDEATHSPR, 0, 0 },
	
	{ "COMBOLAYERR", COMBOLAYERR, 0, 0 },
	{ "COMBODATTRISHORTS", COMBODATTRISHORTS, 0, 0 },
	
	{ "PUSHBLOCKLAYER", PUSHBLOCKLAYER, 0, 0 },
	{ "LINKGRABBED", LINKGRABBED, 0, 0 },
	{ "HEROBUNNY", HEROBUNNY, 0, 0 },
	
	{ "GAMELSWITCH", GAMELSWITCH, 0, 0 },
	{ "GAMEBOTTLEST", GAMEBOTTLEST, 0, 0 },
	
	{ "REFBOTTLETYPE", REFBOTTLETYPE, 0, 0 },
	{ "REFBOTTLESHOP", REFBOTTLESHOP, 0, 0 },
	{ "BOTTLECOUNTER", BOTTLECOUNTER, 0, 0 },
	{ "BOTTLEAMOUNT", BOTTLEAMOUNT, 0, 0 },
	{ "BOTTLEPERCENT", BOTTLEPERCENT, 0, 0 },
	{ "BOTTLEFLAGS", BOTTLEFLAGS, 0, 0 },
	{ "BOTTLENEXT", BOTTLENEXT, 0, 0 },
	{ "BSHOPFILL", BSHOPFILL, 0, 0 },
	{ "BSHOPCOMBO", BSHOPCOMBO, 0, 0 },
	{ "BSHOPCSET", BSHOPCSET, 0, 0 },
	{ "BSHOPPRICE", BSHOPPRICE, 0, 0 },
	{ "BSHOPSTR", BSHOPSTR, 0, 0 },
	{ "COMBODUSRFLAGARR", COMBODUSRFLAGARR, 0, 0 },
	{ "COMBODGENFLAGARR", COMBODGENFLAGARR, 0, 0 },
	{ "HERORESPAWNX", HERORESPAWNX, 0, 0 },
	{ "HERORESPAWNY", HERORESPAWNY, 0, 0 },
	{ "HERORESPAWNDMAP", HERORESPAWNDMAP, 0, 0 },
	{ "HERORESPAWNSCR", HERORESPAWNSCR, 0, 0 },
	{ "IDATAUSESOUND2", IDATAUSESOUND2, 0, 0 },
	{ "HEROSWITCHTIMER", HEROSWITCHTIMER, 0, 0 },
	{ "HEROSWITCHMAXTIMER", HEROSWITCHMAXTIMER, 0, 0 },
	{ "NPCSWHOOKED", NPCSWHOOKED, 0, 0 },
	{ "GAMEMISCSPR", GAMEMISCSPR, 0, 0 },
	{ "GAMEMISCSFX", GAMEMISCSFX, 0, 0 },
	{ "HEROTOTALDYOFFS", HEROTOTALDYOFFS, 0, 0 },
	{ "NPCTOTALDYOFFS", NPCTOTALDYOFFS, 0, 0 },
	{ "LWPNTOTALDYOFFS", LWPNTOTALDYOFFS, 0, 0 },
	{ "EWPNTOTALDYOFFS", EWPNTOTALDYOFFS, 0, 0 },
	{ "LWSWHOOKED", LWSWHOOKED, 0, 0 },
	{ "EWSWHOOKED", EWSWHOOKED, 0, 0 },
	{ "ITMSWHOOKED", ITMSWHOOKED, 0, 0 },
	{ "DEBUGTESTING", DEBUGTESTING, 0, 0 },
	{ "GAMEMAXCHEAT", GAMEMAXCHEAT, 0, 0 },
	{ "SHOWNMSG", SHOWNMSG, 0, 0 },
	{"COMBODTRIGGERFLAGS2", COMBODTRIGGERFLAGS2, 0, 0 },
	{"COMBODTRIGGERBUTTON", COMBODTRIGGERBUTTON, 0, 0 },
	{ "REFGENERICDATA", REFGENERICDATA, 0, 0 },
	{ "GENDATARUNNING", GENDATARUNNING, 0, 0 },
	{ "GENDATASIZE", GENDATASIZE, 0, 0 },
	{ "GENDATAEXITSTATE", GENDATAEXITSTATE, 0, 0 },
	{ "GENDATADATA", GENDATADATA, 0, 0 },
	{ "GENDATAINITD", GENDATAINITD, 0, 0 },
	{ "GENDATARELOADSTATE", GENDATARELOADSTATE, 0, 0 },
	{ "COMBODCSET2FLAGS", COMBODCSET2FLAGS, 0, 0 },
	{ "HEROIMMORTAL", HEROIMMORTAL, 0, 0 },
	{ "NPCCANFLICKER", NPCCANFLICKER, 0, 0 },
	{ "NPCDROWNCLK", NPCDROWNCLK, 0, 0 },
	{ "NPCDROWNCMB", NPCDROWNCMB, 0, 0 },
	{ "ITEMDROWNCLK", ITEMDROWNCLK, 0, 0 },
	{ "ITEMDROWNCMB", ITEMDROWNCMB, 0, 0 },
	{ "LWPNDROWNCLK", LWPNDROWNCLK, 0, 0 },
	{ "LWPNDROWNCMB", LWPNDROWNCMB, 0, 0 },
	{ "EWPNDROWNCLK", EWPNDROWNCLK, 0, 0 },
	{ "EWPNDROWNCMB", EWPNDROWNCMB, 0, 0 },
	{ "HERODROWNCLK", HERODROWNCLK, 0, 0 },
	{ "HERODROWNCMB", HERODROWNCMB, 0, 0 },
	{ "NPCFAKEZ", NPCFAKEZ, 0, 0 },
	{ "ITEMFAKEZ", ITEMFAKEZ, 0, 0 },
	{ "LWPNFAKEZ", LWPNFAKEZ, 0, 0 },
	{ "EWPNFAKEZ", EWPNFAKEZ, 0, 0 },
	{ "HEROFAKEZ", HEROFAKEZ, 0, 0 },
	{ "NPCFAKEJUMP", NPCFAKEJUMP, 0, 0 },
	{ "ITEMFAKEJUMP", ITEMFAKEJUMP, 0, 0 },
	{ "LWPNFAKEJUMP", LWPNFAKEJUMP, 0, 0 },
	{ "EWPNFAKEJUMP", EWPNFAKEJUMP, 0, 0 },
	{ "HEROFAKEJUMP", HEROFAKEJUMP, 0, 0 },
	{ "HEROSHADOWXOFS", HEROSHADOWXOFS, 0, 0 },
	{ "HEROSHADOWYOFS", HEROSHADOWYOFS, 0, 0 },
	{ "NPCSHADOWXOFS", NPCSHADOWXOFS, 0, 0 },
	{ "NPCSHADOWYOFS", NPCSHADOWYOFS, 0, 0 },
	{ "ITEMSHADOWXOFS", ITEMSHADOWXOFS, 0, 0 },
	{ "ITEMSHADOWYOFS", ITEMSHADOWYOFS, 0, 0 },
	{ "LWPNSHADOWXOFS", LWPNSHADOWXOFS, 0, 0 },
	{ "LWPNSHADOWYOFS", LWPNSHADOWYOFS, 0, 0 },
	{ "EWPNSHADOWXOFS", EWPNSHADOWXOFS, 0, 0 },
	{ "EWPNSHADOWYOFS", EWPNSHADOWYOFS, 0, 0 },
	{ "LWPNDEGANGLE", LWPNDEGANGLE, 0, 0 },
	{ "EWPNDEGANGLE", EWPNDEGANGLE, 0, 0 },
	{ "LWPNVX", LWPNVX, 0, 0 },
	{ "LWPNVY", LWPNVY, 0, 0 },
	{ "EWPNVX", EWPNVX, 0, 0 },
	{ "EWPNVY", EWPNVY, 0, 0 },
	{ "LWPNAUTOROTATE", LWPNAUTOROTATE, 0, 0 },
	{ "EWPNAUTOROTATE", EWPNAUTOROTATE, 0, 0 },
	{ "IDATACOSTCOUNTER2", IDATACOSTCOUNTER2, 0, 0 },
	{ "IDATAMAGICTIMER2", IDATAMAGICTIMER2, 0, 0 },
	{ "IDATACOST2", IDATACOST2, 0, 0 },
	{ "IDATAVALIDATE2", IDATAVALIDATE2, 0, 0 },
	{ "MESSAGEDATATEXTLEN", MESSAGEDATATEXTLEN, 0, 0 },
	{ "LWPNFLAGS", LWPNFLAGS, 0, 0 },
	{ "EWPNFLAGS", EWPNFLAGS, 0, 0 },
	{ "REFSTACK", REFSTACK, 0, 0 },
	{ "STACKSIZE", STACKSIZE, 0, 0 },
	{ "STACKFULL", STACKFULL, 0, 0 },
	{ "ITEMFORCEGRAB", ITEMFORCEGRAB, 0, 0 },
	{ "COMBODTRIGGERITEM", COMBODTRIGGERITEM, 0, 0 },
	{ "COMBODTRIGGERTIMER", COMBODTRIGGERTIMER, 0, 0 },
	{ "COMBODTRIGGERSFX", COMBODTRIGGERSFX, 0, 0 },
	{ "COMBODTRIGGERCHANGECMB", COMBODTRIGGERCHANGECMB, 0, 0 },
	{ "SCREENEXSTATED", SCREENEXSTATED, 0, 0 },
	{ "MAPDATAEXSTATED", MAPDATAEXSTATED, 0, 0 },
	{ "HEROSTANDING", HEROSTANDING, 0, 0 },
	{ "COMBODTRIGGERPROX", COMBODTRIGGERPROX, 0, 0 },
	{ "COMBODTRIGGERLIGHTBEAM", COMBODTRIGGERLIGHTBEAM, 0, 0 },
	{ "COMBODTRIGGERCTR", COMBODTRIGGERCTR, 0, 0 },
	{ "COMBODTRIGGERCTRAMNT", COMBODTRIGGERCTRAMNT, 0, 0 },
	{ "GENDATAEVENTSTATE", GENDATAEVENTSTATE, 0, 0 },
	{ "GAMEEVENTDATA", GAMEEVENTDATA, 0, 0 },
	{ "ITEMDROPPEDBY", ITEMDROPPEDBY, 0, 0 },
	{ "GAMEGSWITCH", GAMEGSWITCH, 0, 0 },
	
	{ "COMBODTRIGGERCOOLDOWN", COMBODTRIGGERCOOLDOWN, 0, 0 },
	{ "COMBODTRIGGERCOPYCAT", COMBODTRIGGERCOPYCAT, 0, 0 },
	{ "COMBODTRIGITEMPICKUP", COMBODTRIGITEMPICKUP, 0, 0 },
	{ "COMBODTRIGEXSTATE", COMBODTRIGEXSTATE, 0, 0 },
	{ "COMBODTRIGSPAWNENEMY", COMBODTRIGSPAWNENEMY, 0, 0 },
	{ "COMBODTRIGSPAWNITEM", COMBODTRIGSPAWNITEM, 0, 0 },
	{ "COMBODTRIGCSETCHANGE", COMBODTRIGCSETCHANGE, 0, 0 },
	{ "COMBODLIFTGFXCOMBO", COMBODLIFTGFXCOMBO, 0, 0 },
	{ "COMBODLIFTGFXCCSET", COMBODLIFTGFXCCSET, 0, 0 },
	{ "COMBODLIFTUNDERCMB", COMBODLIFTUNDERCMB, 0, 0 },
	{ "COMBODLIFTUNDERCS", COMBODLIFTUNDERCS, 0, 0 },
	{ "COMBODLIFTDAMAGE", COMBODLIFTDAMAGE, 0, 0 },
	{ "COMBODLIFTLEVEL", COMBODLIFTLEVEL, 0, 0 },
	{ "COMBODLIFTITEM", COMBODLIFTITEM, 0, 0 },
	{ "COMBODLIFTFLAGS", COMBODLIFTFLAGS, 0, 0 },
	{ "COMBODLIFTGFXTYPE", COMBODLIFTGFXTYPE, 0, 0 },
	{ "COMBODLIFTGFXSPRITE", COMBODLIFTGFXSPRITE, 0, 0 },
	{ "COMBODLIFTSFX", COMBODLIFTSFX, 0, 0 },
	{ "COMBODLIFTBREAKSPRITE", COMBODLIFTBREAKSPRITE, 0, 0 },
	{ "COMBODLIFTBREAKSFX", COMBODLIFTBREAKSFX, 0, 0 },
	{ "COMBODLIFTHEIGHT", COMBODLIFTHEIGHT, 0, 0 },
	{ "COMBODLIFTTIME", COMBODLIFTTIME, 0, 0 },
	{ "CLASS_THISKEY", CLASS_THISKEY, 0, 0 },
	{ "ZELDABETATYPE", ZELDABETATYPE, 0, 0 },
	{ "HEROCOYOTETIME", HEROCOYOTETIME, 0, 0 },
	{ "FFCLASTCHANGERX", FFCLASTCHANGERX, 0, 0 },
	{ "FFCLASTCHANGERY", FFCLASTCHANGERY, 0, 0 },
	{ "LWPNTIMEOUT", LWPNTIMEOUT, 0, 0 },
	{ "EWPNTIMEOUT", EWPNTIMEOUT, 0, 0 },
	{ "COMBODTRIGGERLSTATE", COMBODTRIGGERLSTATE, 0, 0 },
	{ "COMBODTRIGGERGSTATE", COMBODTRIGGERGSTATE, 0, 0 },
	{ "COMBODTRIGGERGTIMER", COMBODTRIGGERGTIMER, 0, 0 },
	{ "GAMEMOUSECURSOR", GAMEMOUSECURSOR, 0, 0 },
	{ "COMBODTRIGGERGENSCRIPT", COMBODTRIGGERGENSCRIPT, 0, 0 },
	{ "COMBODTRIGGERGROUP", COMBODTRIGGERGROUP, 0, 0 },
	{ "COMBODTRIGGERGROUPVAL", COMBODTRIGGERGROUPVAL, 0, 0 },
	{ "HEROLIFTEDWPN", HEROLIFTEDWPN, 0, 0 },
	{ "HEROLIFTTIMER", HEROLIFTTIMER, 0, 0 },
	{ "HEROLIFTMAXTIMER", HEROLIFTMAXTIMER, 0, 0 },
	{ "HEROLIFTHEIGHT", HEROLIFTHEIGHT, 0, 0 },
	{ "HEROHAMMERSTATE", HEROHAMMERSTATE, 0, 0 },
	{ "HEROLIFTFLAGS", HEROLIFTFLAGS, 0, 0 },
	{ "COMBODLIFTWEAPONITEM", COMBODLIFTWEAPONITEM, 0, 0 },
	{ "LWPNDEATHITEM", LWPNDEATHITEM, 0, 0 },
	{ "LWPNDEATHDROPSET", LWPNDEATHDROPSET, 0, 0 },
	{ "LWPNDEATHIPICKUP", LWPNDEATHIPICKUP, 0, 0 },
	{ "LWPNDEATHSPRITE", LWPNDEATHSPRITE, 0, 0 },
	{ "LWPNDEATHSFX", LWPNDEATHSFX, 0, 0 },
	{ "EWPNDEATHITEM", EWPNDEATHITEM, 0, 0 },
	{ "EWPNDEATHDROPSET", EWPNDEATHDROPSET, 0, 0 },
	{ "EWPNDEATHIPICKUP", EWPNDEATHIPICKUP, 0, 0 },
	{ "EWPNDEATHSPRITE", EWPNDEATHSPRITE, 0, 0 },
	{ "EWPNDEATHSFX", EWPNDEATHSFX, 0, 0 },
	
	{ "REFPALDATA", REFPALDATA, 0, 0 },
	{ "PALDATACOLOR", PALDATACOLOR, 0, 0 },
	{ "PALDATAR", PALDATAR, 0, 0 },
	{ "PALDATAG", PALDATAG, 0, 0 },
	{ "PALDATAB", PALDATAB, 0, 0 },

	{ "DMAPDATALOOPSTART", DMAPDATALOOPSTART, 0, 0 },
	{ "DMAPDATALOOPEND", DMAPDATALOOPEND, 0, 0 },
	{ "DMAPDATAXFADEIN", DMAPDATAXFADEIN, 0, 0 },
	{ "DMAPDATAXFADEOUT", DMAPDATAXFADEOUT, 0, 0 },
	{ "MUSICUPDATECOND", MUSICUPDATECOND, 0, 0 },
	{ "MUSICUPDATEFLAGS", MUSICUPDATEFLAGS, 0, 0 },
	{ "DMAPDATAINTROSTRINGID", DMAPDATAINTROSTRINGID, 0, 0 },
	{ "IS8BITTILE", IS8BITTILE, 0, 0 },
	{ "NPCFLICKERCOLOR", NPCFLICKERCOLOR, 0, 0 },
	{ "HEROFLICKERCOLOR", HEROFLICKERCOLOR, 0, 0 },
	{ "NPCFLASHINGCSET", NPCFLASHINGCSET, 0, 0 },
	{ "HEROFLASHINGCSET", HEROFLASHINGCSET, 0, 0 },
	{ "NPCFLICKERTRANSP", NPCFLICKERTRANSP, 0, 0 },
	{ "HEROFLICKERTRANSP", HEROFLICKERTRANSP, 0, 0 },
	{ "RESRVD_VAR_MOOSH15", RESRVD_VAR_MOOSH15, 0, 0 },
	{ "RESRVD_VAR_MOOSH16", RESRVD_VAR_MOOSH16, 0, 0 },
	{ "RESRVD_VAR_MOOSH17", RESRVD_VAR_MOOSH17, 0, 0 },
	{ "RESRVD_VAR_MOOSH18", RESRVD_VAR_MOOSH18, 0, 0 },
	{ "RESRVD_VAR_MOOSH19", RESRVD_VAR_MOOSH19, 0, 0 },
	{ "RESRVD_VAR_MOOSH20", RESRVD_VAR_MOOSH20, 0, 0 },
	{ "RESRVD_VAR_MOOSH21", RESRVD_VAR_MOOSH21, 0, 0 },
	{ "RESRVD_VAR_MOOSH22", RESRVD_VAR_MOOSH22, 0, 0 },
	{ "RESRVD_VAR_MOOSH23", RESRVD_VAR_MOOSH23, 0, 0 },
	{ "RESRVD_VAR_MOOSH24", RESRVD_VAR_MOOSH24, 0, 0 },
	{ "RESRVD_VAR_MOOSH25", RESRVD_VAR_MOOSH25, 0, 0 },
	{ "RESRVD_VAR_MOOSH26", RESRVD_VAR_MOOSH26, 0, 0 },
	{ "RESRVD_VAR_MOOSH27", RESRVD_VAR_MOOSH27, 0, 0 },
	{ "RESRVD_VAR_MOOSH28", RESRVD_VAR_MOOSH28, 0, 0 },
	{ "RESRVD_VAR_MOOSH29", RESRVD_VAR_MOOSH29, 0, 0 },
	{ "RESRVD_VAR_MOOSH30", RESRVD_VAR_MOOSH30, 0, 0 },
	{ "DMAPDATAMIRRDMAP", DMAPDATAMIRRDMAP, 0, 0 },
	{ "IDATAGRADUAL", IDATAGRADUAL, 0, 0 },
	{ "IDATASPRSCRIPT", IDATASPRSCRIPT, 0, 0 },
	{ "IDATAPSOUND", IDATAPSOUND, 0, 0 },
	{ "IDATACONSTSCRIPT", IDATACONSTSCRIPT, 0, 0 },
	{ "IDATASSWIMDISABLED", IDATASSWIMDISABLED, 0, 0 },
	{ "IDATABUNNYABLE", IDATABUNNYABLE, 0, 0 },
	{ "IDATAJINXIMMUNE", IDATAJINXIMMUNE, 0, 0 },
	{ "IDATAJINXSWAP", IDATAJINXSWAP, 0, 0 },
	{ "SPRITEDATAFLCSET", SPRITEDATAFLCSET, 0, 0 },
	{ "SPRITEDATAFLAGS", SPRITEDATAFLAGS, 0, 0 },
	{ "SPRITEDATAID", SPRITEDATAID, 0, 0 },
	{ "CLASS_THISKEY2", CLASS_THISKEY2, 0, 0 },
	{ "VIEWPORT_MODE", VIEWPORT_MODE, 0, 0},
	{ "VIEWPORT_X", VIEWPORT_X, 0, 0},
	{ "VIEWPORT_Y", VIEWPORT_Y, 0, 0},
	{ "VIEWPORT_WIDTH", VIEWPORT_WIDTH, 0, 0},
	{ "VIEWPORT_HEIGHT", VIEWPORT_HEIGHT, 0, 0},
	{ "Z3_RSRVD_1", Z3_RSRVD_1, 0, 0},
	{ "Z3_RSRVD_2", Z3_RSRVD_2, 0, 0},
	{ "REGION_WIDTH", REGION_WIDTH, 0, 0},
	{ "REGION_HEIGHT", REGION_HEIGHT, 0, 0},
	{ "REGION_SCREEN_WIDTH", REGION_SCREEN_WIDTH, 0, 0},
	{ "REGION_SCREEN_HEIGHT", REGION_SCREEN_HEIGHT, 0, 0},
	{ "FF_SCREEN_INDEX", FF_SCREEN_INDEX, 0, 0},
	{ "REGION_UNUSED", REGION_UNUSED, 0, 0},
	{ "REGION_NUM_COMBOS", REGION_NUM_COMBOS, 0, 0},
	{ "REGION_ID", REGION_ID, 0, 0},
	{ "REGION_ORIGIN_SCREEN", REGION_ORIGIN_SCREEN, 0, 0},
	{ "LWPNLIFTLEVEL", LWPNLIFTLEVEL, 0, 0},
	{ "LWPNLIFTTIME", LWPNLIFTTIME, 0, 0},
	{ "LWPNLIFTHEIGHT", LWPNLIFTHEIGHT, 0, 0},
	{ "EWPNLIFTLEVEL", EWPNLIFTLEVEL, 0, 0},
	{ "EWPNLIFTTIME", EWPNLIFTTIME, 0, 0},
	{ "EWPNLIFTHEIGHT", EWPNLIFTHEIGHT, 0, 0},
	{ "HEROSHIELDJINX", HEROSHIELDJINX, 0, 0},
	{ "MAPDATALENSSHOWS", MAPDATALENSSHOWS, 0, 0},
	{ "MAPDATALENSHIDES", MAPDATALENSHIDES, 0, 0},
	{ "SCREENLENSSHOWS", SCREENLENSSHOWS, 0, 0},
	{ "SCREENLENSHIDES", SCREENLENSHIDES, 0, 0},
	{ "GAMETRIGGROUPS", GAMETRIGGROUPS, 0, 0},
	{ "GAMEOVERRIDEITEMS", GAMEOVERRIDEITEMS, 0, 0},
	{ "DMAPDATASUBSCRO", DMAPDATASUBSCRO, 0, 0},
	{ "REFSUBSCREENPAGE", REFSUBSCREENPAGE, 0, 0},
	{ "REFSUBSCREENWIDG", REFSUBSCREENWIDG, 0, 0},
	{ "SUBDATACURPG", SUBDATACURPG, 0, 0},
	{ "SUBDATANUMPG", SUBDATANUMPG, 0, 0},
	{ "SUBDATAPAGES", SUBDATAPAGES, 0, 0},
	{ "SUBDATATYPE", SUBDATATYPE, 0, 0},
	{ "SUBDATAFLAGS", SUBDATAFLAGS, 0, 0},
	{ "SUBDATACURSORPOS", SUBDATACURSORPOS, 0, 0},
	{ "SUBDATASCRIPT", SUBDATASCRIPT, 0, 0},
	{ "SUBDATAINITD", SUBDATAINITD, 0, 0},
	{ "SUBDATABTNLEFT", SUBDATABTNLEFT, 0, 0},
	{ "SUBDATABTNRIGHT", SUBDATABTNRIGHT, 0, 0},
	{ "SUBDATATRANSLEFTTY", SUBDATATRANSLEFTTY, 0, 0},
	{ "SUBDATATRANSLEFTSFX", SUBDATATRANSLEFTSFX, 0, 0},
	{ "SUBDATATRANSLEFTFLAGS", SUBDATATRANSLEFTFLAGS, 0, 0},
	{ "SUBDATATRANSLEFTARGS", SUBDATATRANSLEFTARGS, 0, 0},
	{ "PORTALX", PORTALX, 0, 0},
	{ "PORTALY", PORTALY, 0, 0},
	{ "PORTALDMAP", PORTALDMAP, 0, 0},
	{ "PORTALSCREEN", PORTALSCREEN, 0, 0},
	{ "PORTALACLK", PORTALACLK, 0, 0},
	{ "PORTALAFRM", PORTALAFRM, 0, 0},
	{ "PORTALOTILE", PORTALOTILE, 0, 0},
	{ "PORTALASPD", PORTALASPD, 0, 0},
	{ "PORTALFRAMES", PORTALFRAMES, 0, 0},
	{ "PORTALSAVED", PORTALSAVED, 0, 0},
	{ "PORTALCLOSEDIS", PORTALCLOSEDIS, 0, 0},
	{ "REFPORTAL", REFPORTAL, 0, 0},
	{ "REFSAVPORTAL", REFSAVPORTAL, 0, 0},
	{ "PORTALWARPSFX", PORTALWARPSFX, 0, 0},
	{ "PORTALWARPVFX", PORTALWARPVFX, 0, 0},
	{ "SAVEDPORTALX", SAVEDPORTALX, 0, 0},
	{ "SAVEDPORTALY", SAVEDPORTALY, 0, 0},
	{ "SAVEDPORTALSRCDMAP", SAVEDPORTALSRCDMAP, 0, 0},
	{ "SAVEDPORTALDESTDMAP", SAVEDPORTALDESTDMAP, 0, 0},
	{ "SAVEDPORTALSRCSCREEN", SAVEDPORTALSRCSCREEN, 0, 0},
	{ "SAVEDPORTALWARPSFX", SAVEDPORTALWARPSFX, 0, 0},
	{ "SAVEDPORTALWARPVFX", SAVEDPORTALWARPVFX, 0, 0},
	{ "SAVEDPORTALSPRITE", SAVEDPORTALSPRITE, 0, 0},
	{ "SAVEDPORTALPORTAL", SAVEDPORTALPORTAL, 0, 0},
	{ "PORTALCOUNT", PORTALCOUNT, 0, 0},
	{ "SAVEDPORTALCOUNT", SAVEDPORTALCOUNT, 0, 0},
	{ "SAVEDPORTALDSTSCREEN", SAVEDPORTALDSTSCREEN, 0, 0},

	{ "SUBDATATRANSRIGHTTY", SUBDATATRANSRIGHTTY, 0, 0 },
	{ "SUBDATATRANSRIGHTSFX", SUBDATATRANSRIGHTSFX, 0, 0 },
	{ "SUBDATATRANSRIGHTFLAGS", SUBDATATRANSRIGHTFLAGS, 0, 0 },
	{ "SUBDATATRANSRIGHTARGS", SUBDATATRANSRIGHTARGS, 0, 0 },
	{ "SUBDATASELECTORDSTX", SUBDATASELECTORDSTX, 0, 0 },
	{ "SUBDATASELECTORDSTY", SUBDATASELECTORDSTY, 0, 0 },
	{ "SUBDATASELECTORDSTW", SUBDATASELECTORDSTW, 0, 0 },
	{ "SUBDATASELECTORDSTH", SUBDATASELECTORDSTH, 0, 0 },
	{ "SUBDATASELECTORWID", SUBDATASELECTORWID, 0, 0 },
	{ "SUBDATASELECTORHEI", SUBDATASELECTORHEI, 0, 0 },
	{ "SUBDATASELECTORTILE", SUBDATASELECTORTILE, 0, 0 },
	{ "SUBDATASELECTORCSET", SUBDATASELECTORCSET, 0, 0 },
	{ "SUBDATASELECTORFRM", SUBDATASELECTORFRM, 0, 0 },
	{ "SUBDATASELECTORASPD", SUBDATASELECTORASPD, 0, 0 },
	{ "SUBDATASELECTORDELAY", SUBDATASELECTORDELAY, 0, 0 },
	{ "SUBDATATRANSCLK", SUBDATATRANSCLK, 0, 0 },
	{ "SUBDATATRANSTY", SUBDATATRANSTY, 0, 0 },
	{ "SUBDATATRANSFLAGS", SUBDATATRANSFLAGS, 0, 0 },
	{ "SUBDATATRANSARGS", SUBDATATRANSARGS, 0, 0 },
	{ "SUBDATATRANSFROMPG", SUBDATATRANSFROMPG, 0, 0 },
	{ "SUBDATATRANSTOPG", SUBDATATRANSTOPG, 0, 0 },
	{ "SUBDATASELECTORFLASHCSET", SUBDATASELECTORFLASHCSET, 0, 0 },
	{ "GAMEASUBOPEN", GAMEASUBOPEN, 0, 0 },
	{ "GAMENUMASUB", GAMENUMASUB, 0, 0 },
	{ "GAMENUMPSUB", GAMENUMPSUB, 0, 0 },
	{ "GAMENUMOSUB", GAMENUMOSUB, 0, 0 },
	{ "SUBPGINDEX", SUBPGINDEX, 0, 0 },
	{ "SUBPGNUMWIDG", SUBPGNUMWIDG, 0, 0 },
	{ "SUBPGWIDGETS", SUBPGWIDGETS, 0, 0 },
	{ "SUBPGSUBDATA", SUBPGSUBDATA, 0, 0 },
	{ "SUBPGCURSORPOS", SUBPGCURSORPOS, 0, 0 },
	{ "SUBWIDGTYPE", SUBWIDGTYPE, 0, 0 },
	{ "SUBWIDGINDEX", SUBWIDGINDEX, 0, 0 },
	{ "SUBWIDGPAGE", SUBWIDGPAGE, 0, 0 },
	{ "SUBWIDGPOS", SUBWIDGPOS, 0, 0 },
	{ "SUBWIDGPOSES", SUBWIDGPOSES, 0, 0 },
	{ "SUBWIDGPOSFLAG", SUBWIDGPOSFLAG, 0, 0 },
	{ "SUBWIDGX", SUBWIDGX, 0, 0 },
	{ "SUBWIDGY", SUBWIDGY, 0, 0 },
	{ "SUBWIDGW", SUBWIDGW, 0, 0 },
	{ "SUBWIDGH", SUBWIDGH, 0, 0 },
	{ "SUBWIDGGENFLAG", SUBWIDGGENFLAG, 0, 0 },
	{ "SUBWIDGFLAG", SUBWIDGFLAG, 0, 0 },
	{ "SUBWIDGSELECTORDSTX", SUBWIDGSELECTORDSTX, 0, 0 },
	{ "SUBWIDGSELECTORDSTY", SUBWIDGSELECTORDSTY, 0, 0 },
	{ "SUBWIDGSELECTORDSTW", SUBWIDGSELECTORDSTW, 0, 0 },
	{ "SUBWIDGSELECTORDSTH", SUBWIDGSELECTORDSTH, 0, 0 },
	{ "SUBWIDGSELECTORWID", SUBWIDGSELECTORWID, 0, 0 },
	{ "SUBWIDGSELECTORHEI", SUBWIDGSELECTORHEI, 0, 0 },
	{ "SUBWIDGSELECTORTILE", SUBWIDGSELECTORTILE, 0, 0 },

	{ "SUBWIDGSELECTORCSET", SUBWIDGSELECTORCSET, 0, 0 },
	{ "SUBWIDGSELECTORFLASHCSET", SUBWIDGSELECTORFLASHCSET, 0, 0 },
	{ "SUBWIDGSELECTORFRM", SUBWIDGSELECTORFRM, 0, 0 },
	{ "SUBWIDGSELECTORASPD", SUBWIDGSELECTORASPD, 0, 0 },
	{ "SUBWIDGSELECTORDELAY", SUBWIDGSELECTORDELAY, 0, 0 },
	{ "SUBWIDGPRESSSCRIPT", SUBWIDGPRESSSCRIPT, 0, 0 },
	{ "SUBWIDGPRESSINITD", SUBWIDGPRESSINITD, 0, 0 },
	{ "SUBWIDGBTNPRESS", SUBWIDGBTNPRESS, 0, 0 },
	{ "SUBWIDGBTNPG", SUBWIDGBTNPG, 0, 0 },
	{ "SUBWIDGPGMODE", SUBWIDGPGMODE, 0, 0 },
	{ "SUBWIDGPGTARG", SUBWIDGPGTARG, 0, 0 },
	{ "SUBWIDGTRANSPGTY", SUBWIDGTRANSPGTY, 0, 0 },
	{ "SUBWIDGTRANSPGSFX", SUBWIDGTRANSPGSFX, 0, 0 },
	{ "SUBWIDGTRANSPGFLAGS", SUBWIDGTRANSPGFLAGS, 0, 0 },
	{ "SUBWIDGTRANSPGARGS", SUBWIDGTRANSPGARGS, 0, 0 },

	{ "SUBWIDGTY_CSET", SUBWIDGTY_CSET, 0, 0 },
	{ "SUBWIDGTY_TILE", SUBWIDGTY_TILE, 0, 0 },

	{ "SUBWIDGTY_FONT", SUBWIDGTY_FONT, 0, 0 },
	{ "SUBWIDGTY_ALIGN", SUBWIDGTY_ALIGN, 0, 0 },
	{ "SUBWIDGTY_SHADOWTY", SUBWIDGTY_SHADOWTY, 0, 0 },
	{ "SUBWIDGTY_COLOR_TXT", SUBWIDGTY_COLOR_TXT, 0, 0 },
	{ "SUBWIDGTY_COLOR_SHD", SUBWIDGTY_COLOR_SHD, 0, 0 },
	{ "SUBWIDGTY_COLOR_BG", SUBWIDGTY_COLOR_BG, 0, 0 },

	{ "SUBWIDGTY_COLOR_OLINE", SUBWIDGTY_COLOR_OLINE, 0, 0 },
	{ "SUBWIDGTY_COLOR_FILL", SUBWIDGTY_COLOR_FILL, 0, 0 },

	{ "SUBWIDGTY_BUTTON", SUBWIDGTY_BUTTON, 0, 0 },
	{ "SUBWIDGTY_COUNTERS", SUBWIDGTY_COUNTERS, 0, 0 },
	{ "SUBWIDGTY_MINDIG", SUBWIDGTY_MINDIG, 0, 0 },
	{ "SUBWIDGTY_MAXDIG", SUBWIDGTY_MAXDIG, 0, 0 },
	{ "SUBWIDGTY_INFITM", SUBWIDGTY_INFITM, 0, 0 },
	{ "SUBWIDGTY_INFCHAR", SUBWIDGTY_INFCHAR, 0, 0 },
	{ "SUBWIDGTY_COSTIND", SUBWIDGTY_COSTIND, 0, 0 },

	{ "SUBWIDGTY_COLOR_PLAYER", SUBWIDGTY_COLOR_PLAYER, 0, 0 },
	{ "SUBWIDGTY_COLOR_CMPBLNK", SUBWIDGTY_COLOR_CMPBLNK, 0, 0 },
	{ "SUBWIDGTY_COLOR_CMPOFF", SUBWIDGTY_COLOR_CMPOFF, 0, 0 },
	{ "SUBWIDGTY_COLOR_ROOM", SUBWIDGTY_COLOR_ROOM, 0, 0 },
	{ "SUBWIDGTY_ITEMCLASS", SUBWIDGTY_ITEMCLASS, 0, 0 },
	{ "SUBWIDGTY_ITEMID", SUBWIDGTY_ITEMID, 0, 0 },
	{ "SUBWIDGTY_FRAMETILE", SUBWIDGTY_FRAMETILE, 0, 0 },
	{ "SUBWIDGTY_FRAMECSET", SUBWIDGTY_FRAMECSET, 0, 0 },
	{ "SUBWIDGTY_PIECETILE", SUBWIDGTY_PIECETILE, 0, 0 },
	{ "SUBWIDGTY_PIECECSET", SUBWIDGTY_PIECECSET, 0, 0 },
	{ "SUBWIDGTY_FLIP", SUBWIDGTY_FLIP, 0, 0 },
	{ "SUBWIDGTY_NUMBER", SUBWIDGTY_NUMBER, 0, 0 },
	{ "SUBWIDGTY_CORNER", SUBWIDGTY_CORNER, 0, 0 },

	{ "SUBWIDGTY_FRAMES", SUBWIDGTY_FRAMES, 0, 0 },
	{ "SUBWIDGTY_SPEED", SUBWIDGTY_SPEED, 0, 0 },
	{ "SUBWIDGTY_DELAY", SUBWIDGTY_DELAY, 0, 0 },
	{ "SUBWIDGTY_CONTAINER", SUBWIDGTY_CONTAINER, 0, 0 },
	{ "SUBWIDGTY_GAUGE_WID", SUBWIDGTY_GAUGE_WID, 0, 0 },
	{ "SUBWIDGTY_GAUGE_HEI", SUBWIDGTY_GAUGE_HEI, 0, 0 },
	{ "SUBWIDGTY_UNITS", SUBWIDGTY_UNITS, 0, 0 },
	{ "SUBWIDGTY_HSPACE", SUBWIDGTY_HSPACE, 0, 0 },
	{ "SUBWIDGTY_VSPACE", SUBWIDGTY_VSPACE, 0, 0 },
	{ "SUBWIDGTY_GRIDX", SUBWIDGTY_GRIDX, 0, 0 },
	{ "SUBWIDGTY_GRIDY", SUBWIDGTY_GRIDY, 0, 0 },
	{ "SUBWIDGTY_ANIMVAL", SUBWIDGTY_ANIMVAL, 0, 0 },
	{ "SUBWIDGTY_SHOWDRAIN", SUBWIDGTY_SHOWDRAIN, 0, 0 },
	{ "SUBWIDGTY_PERCONTAINER", SUBWIDGTY_PERCONTAINER, 0, 0 },
	{ "SUBWIDGTY_TABSIZE", SUBWIDGTY_TABSIZE, 0, 0 },

	{ "GAMEASUBYOFF", GAMEASUBYOFF, 0, 0 },

	{ "SUBWIDGDISPITM", SUBWIDGDISPITM, 0, 0 },
	{ "SUBWIDGEQPITM", SUBWIDGEQPITM, 0, 0 },

	{ "SUBWIDG_DISPX", SUBWIDG_DISPX, 0, 0 },
	{ "SUBWIDG_DISPY", SUBWIDG_DISPY, 0, 0 },
	{ "SUBWIDG_DISPW", SUBWIDG_DISPW, 0, 0 },
	{ "SUBWIDG_DISPH", SUBWIDG_DISPH, 0, 0 },

	{ "SCREENSCRDATASIZE", SCREENSCRDATASIZE, 0, 0 },
	{ "SCREENSCRDATA", SCREENSCRDATA, 0, 0 },
	{ "MAPDATASCRDATASIZE", MAPDATASCRDATASIZE, 0, 0 },
	{ "MAPDATASCRDATA", MAPDATASCRDATA, 0, 0 },

	{ "HEROSHOVEOFFSET", HEROSHOVEOFFSET, 0, 0 },

	{ "SCREENDATAGUYCOUNT", SCREENDATAGUYCOUNT, 0, 0 },
	{ "MAPDATAGUYCOUNT", MAPDATAGUYCOUNT, 0, 0 },
	{ "ITEMNOSOUND", ITEMNOSOUND, 0, 0 },
	{ "ITEMNOHOLDSOUND", ITEMNOHOLDSOUND, 0, 0 },

	{ "IDATAUSEBURNSPR", IDATAUSEBURNSPR, 0, 0 },
	{ "IDATABURNINGSPR", IDATABURNINGSPR, 0, 0 },
	{ "LWPNSPRITES", LWPNSPRITES, 0, 0 },
	{ "EWPNSPRITES", EWPNSPRITES, 0, 0 },

	{ "SCREENDATAEXDOOR", SCREENDATAEXDOOR, 0, 0 },
	{ "MAPDATAEXDOOR", MAPDATAEXDOOR, 0, 0 },
	{ "COMBODTRIGEXDOORDIR", COMBODTRIGEXDOORDIR, 0, 0 },
	{ "COMBODTRIGEXDOORIND", COMBODTRIGEXDOORIND, 0, 0 },

	{ "IDATABURNINGLIGHTRAD", IDATABURNINGLIGHTRAD, 0, 0 },

	{ "LWPNBURNLIGHTRADIUS", LWPNBURNLIGHTRADIUS, 0, 0 },
	{ "EWPNBURNLIGHTRADIUS", EWPNBURNLIGHTRADIUS, 0, 0 },

	{ "IDATAATTRIB_L", IDATAATTRIB_L, 0, 0 },

	{ "HEROSLIDING", HEROSLIDING, 0, 0 },
	{ "HEROICECMB", HEROICECMB, 0, 0 },
	{ "HEROSCRICECMB", HEROSCRICECMB, 0, 0 },
	{ "HEROICEVX", HEROICEVX, 0, 0 },
	{ "HEROICEVY", HEROICEVY, 0, 0 },
	{ "HEROICEENTRYFRAMES", HEROICEENTRYFRAMES, 0, 0 },
	{ "HEROICEENTRYMAXFRAMES", HEROICEENTRYMAXFRAMES, 0, 0 },

	{ "REFWEBSOCKET", REFWEBSOCKET, 0, 0 },
	{ "WEBSOCKET_STATE", WEBSOCKET_STATE, 0, 0 },
	{ "WEBSOCKET_URL", WEBSOCKET_URL, 0, 0 },
	{ "WEBSOCKET_HAS_MESSAGE", WEBSOCKET_HAS_MESSAGE, 0, 0 },
	{ "WEBSOCKET_MESSAGE_TYPE", WEBSOCKET_MESSAGE_TYPE, 0, 0 },

	{ " ", -1, 0, 0 }
};

std::initializer_list<CommandDependency> get_command_implicit_dependencies(int command)
{
	typedef std::initializer_list<CommandDependency> T;

	switch (command)
	{
		case LOAD:
		case LOADD:
		case STORE:
		case STOREV:
		case STORED:
		case STOREDV:
		{
			static T r = {{rSFRAME, REG_R}};
			return r;
		}

		case ATOI2:
		case ILEN2:
		case READPODARRAYR:
		case READPODARRAYV:
		case REMCHR2:
		case WRITEPODARRAYRR:
		case WRITEPODARRAYRV:
		case WRITEPODARRAYVR:
		case WRITEPODARRAYVV:
		case XLEN2:
		case XTOI2:
		{
			static T r = {{rINDEX, REG_R}};
			return r;
		}

		case ZCLASS_CONSTRUCT:
		case ZCLASS_WRITE:
		{
			static T r = {{rEXP1, REG_R}};
			return r;
		}
		
		case READBITMAP:
		{
			static T r = {{rEXP2, REG_R}};
			return r;
		}

		case ARRAYPOP:
		case ARRAYPUSH:
		case CHARWIDTHR:
		case CHOOSEVARG:
		case CREATEPORTAL:
		case CREATESAVPORTAL:
		case CURRENTITEMID:
		case FILECREATE:
		case FILEFLUSH:
		case FILEGETCHAR:
		case FILEISALLOCATED:
		case FILEISVALID:
		case FILEOPEN:
		case FILEPUTCHAR:
		case FILEREADSTR:
		case FILEREMOVE:
		case FILESEEK:
		case FILEUNGETCHAR:
		case FILEWRITESTR:
		case FONTHEIGHTR:
		case HEROCANMOVE:
		case HEROCANMOVEATANGLE:
		case HEROCANMOVEXY:
		case HEROISFLICKERFRAME:
		case HEROLIFTRELEASE:
		case HEROMOVE:
		case HEROMOVEATANGLE:
		case HEROMOVEXY:
		case LOADPORTAL:
		case LOADSAVPORTAL:
		case MAKEVARGARRAY:
		case MAXVARG:
		case MESSAGEHEIGHTR:
		case MESSAGEWIDTHR:
		case MINVARG:
		case NPCCANPLACE:
		case NPCISFLICKERFRAME:
		case NPCMOVEPAUSED:
		case RNGLRAND1:
		case RNGLRAND2:
		case RNGLRAND3:
		case RNGRAND1:
		case RNGRSEED:
		case SAVEDPORTALGENERATE:
		case SCREENDOSPAWN:
		case SPRINTFA:
		case SPRINTFVARG:
		case STRINGWIDTHR:
		case SUBPAGE_FIND_WIDGET_BY_LABEL:
		case SUBPAGE_FIND_WIDGET:
		case SUBPAGE_MOVE_SEL:
		case SUBPAGE_NEW_WIDG:
		case WEBSOCKET_LOAD:
		case WRAPDEGREES:
		case WRAPRADIANS:
		case ZCLASS_FREE:
		case ZCLASS_READ:
		{
			static T r = {{rEXP1, REG_W}};
			return r;
		}

		case REGENERATEBITMAP:
		{
			static T r = {{rEXP2, REG_RW}};
			return r;
		}

		case FILEREADBYTES:
		case FILEREADCHARS:
		case FILEREADINTS:
		case FILEWRITEBYTES:
		case FILEWRITECHARS:
		case FILEWRITEINTS:
		{
			static T r = {{rINDEX, REG_R}, {rEXP1, REG_W}};
			return r;
		}

		case FILEALLOCATE:
		case NPCADD:
		{
			static T r = {{rEXP1, REG_W}, {rEXP2, REG_W}};
			return r;
		}

		case NPCCANMOVEANGLE:
		case NPCCANMOVEDIR:
		case NPCCANMOVEXY:
		case NPCMOVE:
		case NPCMOVEANGLE:
		case NPCMOVEXY:
		{
			static T r = {{rINDEX, REG_R}, {rEXP1, REG_RW}, {rEXP2, REG_R}};
			return r;
		}

		case ARCTANR:
		case ISSOLID:
		case MAPDATAISSOLID:
		case STRCAT:
		case STRCHR:
		case STRCSPN:
		case STRINGCOMPARE:
		case STRINGICOMPARE:
		case STRRCHR:
		case STRSPN:
		case STRSTR:
		{
			static T r = {{rINDEX, REG_R}, {rINDEX2, REG_R}};
			return r;
		}
		
		case STRINGNCOMPARE:
		case STRINGNICOMPARE:
		{
			static T r = {{rINDEX, REG_R}, {rEXP1, REG_R}, {rEXP2, REG_R}};
			return r;
		}

		case MAPDATAISSOLIDLYR:
		case ISSOLIDLAYER:
		{
			static T r = {{rINDEX, REG_R}, {rINDEX2, REG_R}, {rEXP1, REG_R}};
			return r;
		}
		
		case POP:
		case POPARGS:
		case PUSHARGSR:
		case PUSHARGSV:
		case PUSHR:
		case PUSHV:
		{
			static T r = {{SP, REG_RW}, {SP2, REG_RW}};
			return r;
		}
	}

	return {};
}

std::initializer_list<int> get_register_dependencies(int reg)
{
	switch (reg)
	{
		case AUDIOVOLUME:
		case BOTTLEAMOUNT:
		case BOTTLECOUNTER:
		case BOTTLEFLAGS:
		case BOTTLEPERCENT:
		case BSHOPCOMBO:
		case BSHOPCSET:
		case BSHOPFILL:
		case BSHOPPRICE:
		case BSHOPSTR:
		case BUTTONHELD:
		case BUTTONINPUT:
		case BUTTONPRESS:
		case COMBOCD:
		case COMBODATAINITD:
		case COMBODATTRIBUTES:
		case COMBODATTRIBYTES:
		case COMBODATTRISHORTS:
		case COMBODBLOCKWEAPON:
		case COMBODD:
		case COMBODEXPANSION:
		case COMBODGENFLAGARR:
		case COMBODSTRIKEWEAPONS:
		case COMBODTRIGGERBUTTON:
		case COMBODTRIGGERFLAGS:
		case COMBODTRIGGERFLAGS2:
		case COMBOED:
		case COMBOFD:
		case COMBOID:
		case COMBOSD:
		case COMBOTD:
		case DEBUGD:
		case DEBUGGDR:
		case DISABLEBUTTON:
		case DISABLEDITEM:
		case DISABLEKEY:
		case DMAPCOMPASSD:
		case DMAPCONTINUED:
		case DMAPDATACHARTED:
		case DMAPDATADISABLEDITEMS:
		case DMAPDATAFLAGARR:
		case DMAPDATAGRID:
		case DMAPDATALARGEMAPCSET:
		case DMAPDATALARGEMAPTILE:
		case DMAPDATAMAPINITD:
		case DMAPDATAMINIMAPCSET:
		case DMAPDATAMINIMAPTILE:
		case DMAPDATASUBINITD:
		case DMAPFLAGSD:
		case DMAPINITD:
		case DMAPLEVELD:
		case DMAPLEVELPAL:
		case DMAPMAP:
		case DMAPMIDID:
		case DMAPOFFSET:
		case DROPSETCHANCES:
		case DROPSETITEMS:
		case EWPNBURNLIGHTRADIUS:
		case EWPNFLAGS:
		case EWPNINITD:
		case EWPNMISCD:
		case EWPNMOVEFLAGS:
		case EWPNSPRITES:
		case FFFLAGSD:
		case FFINITDD:
		case FFMISCD:
		case FFRULE:
		case GAMEBOTTLEST:
		case GAMECOUNTERD:
		case GAMEDCOUNTERD:
		case GAMEEVENTDATA:
		case GAMEGENERICD:
		case GAMEGRAVITY:
		case GAMEGSWITCH:
		case GAMEGUYCOUNT:
		case GAMEGUYCOUNTD:
		case GAMEITEMSD:
		case GAMELITEMSD:
		case GAMELKEYSD:
		case GAMELSWITCH:
		case GAMEMCOUNTERD:
		case GAMEMISC:
		case GAMEMISCSFX:
		case GAMEMISCSPR:
		case GAMEOVERRIDEITEMS:
		case GAMESCROLLING:
		case GAMESUSPEND:
		case GAMETRIGGROUPS:
		case GDD:
		case GENDATADATA:
		case GENDATAEVENTSTATE:
		case GENDATAEXITSTATE:
		case GENDATAINITD:
		case GENDATARELOADSTATE:
		case GHOSTARR:
		case GLOBALRAMD:
		case HEROLIFTFLAGS:
		case HEROMOVEFLAGS:
		case HEROSTEPS:
		case IDATAATTRIB_L:
		case IDATAATTRIB:
		case IDATABURNINGLIGHTRAD:
		case IDATABURNINGSPR:
		case IDATAFLAGS:
		case IDATAINITDD:
		case IDATAMISCD:
		case IDATASPRITE:
		case IDATAUSEMVT:
		case IDATAWPNINITD:
		case IS8BITTILE:
		case ISBLANKTILE:
		case ITEMMISCD:
		case ITEMMOVEFLAGS:
		case ITEMSPRITEINITD:
		case JOYPADPRESS:
		case KEYBINDINGS:
		case KEYINPUT:
		case KEYPRESS:
		case LINKDEFENCE:
		case LINKHITBY:
		case LINKITEMD:
		case LINKMISCD:
		case LWPNBURNLIGHTRADIUS:
		case LWPNFLAGS:
		case LWPNINITD:
		case LWPNMISCD:
		case LWPNMOVEFLAGS:
		case LWPNSPRITES:
		case MAPDATACOMBOCD:
		case MAPDATACOMBODD:
		case MAPDATACOMBOED:
		case MAPDATACOMBOFD:
		case MAPDATACOMBOID:
		case MAPDATACOMBOSD:
		case MAPDATACOMBOTD:
		case MAPDATADOOR:
		case MAPDATAENEMY:
		case MAPDATAEXSTATED:
		case MAPDATAFFCSET:
		case MAPDATAFFDATA:
		case MAPDATAFFDELAY:
		case MAPDATAFFEFFECTHEIGHT:
		case MAPDATAFFEFFECTWIDTH:
		case MAPDATAFFFLAGS:
		case MAPDATAFFHEIGHT:
		case MAPDATAFFINITIALISED:
		case MAPDATAFFLINK:
		case MAPDATAFFSCRIPT:
		case MAPDATAFFWIDTH:
		case MAPDATAFFX:
		case MAPDATAFFXDELTA:
		case MAPDATAFFXDELTA2:
		case MAPDATAFFY:
		case MAPDATAFFYDELTA:
		case MAPDATAFFYDELTA2:
		case MAPDATAFLAGS:
		case MAPDATAINITD:
		case MAPDATAINITDARRAY:
		case MAPDATALAYERINVIS:
		case MAPDATALAYERMAP:
		case MAPDATALAYEROPACITY:
		case MAPDATALAYERSCREEN:
		case MAPDATALENSHIDES:
		case MAPDATALENSSHOWS:
		case MAPDATAMISCD:
		case MAPDATANUMFF:
		case MAPDATAPATH:
		case MAPDATASCRDATA:
		case MAPDATASCREENEFLAGSD:
		case MAPDATASCREENFLAGSD:
		case MAPDATASCREENSTATED:
		case MAPDATASCRIPTDRAWS:
		case MAPDATASECRETCOMBO:
		case MAPDATASECRETCSET:
		case MAPDATASECRETFLAG:
		case MAPDATASIDEWARPDMAP:
		case MAPDATASIDEWARPID:
		case MAPDATASIDEWARPOVFLAGS:
		case MAPDATASIDEWARPSC:
		case MAPDATASIDEWARPTYPE:
		case MAPDATASWARPRETSQR:
		case MAPDATATILEWARPDMAP:
		case MAPDATATILEWARPOVFLAGS:
		case MAPDATATILEWARPSCREEN:
		case MAPDATATILEWARPTYPE:
		case MAPDATATWARPRETSQR:
		case MAPDATAWARPRETX:
		case MAPDATAWARPRETY:
		case MESSAGEDATAFLAGSARR:
		case MESSAGEDATAMARGINS:
		case MOUSEARR:
		case MUSICUPDATEFLAGS:
		case NPCBEHAVIOUR:
		case NPCDATAATTRIBUTE:
		case NPCDATABEHAVIOUR:
		case NPCDATADEFENSE:
		case NPCDATAINITD:
		case NPCDATASHIELD:
		case NPCDATAWEAPONINITD:
		case NPCDD:
		case NPCDEFENSED:
		case NPCHITBY:
		case NPCINITD:
		case NPCMISCD:
		case NPCMOVEFLAGS:
		case NPCSCRDEFENSED:
		case NPCSHIELD:
		case PALDATAB:
		case PALDATACOLOR:
		case PALDATAG:
		case PALDATAR:
		case RAWKEY:
		case READKEY:
		case SCRDOORD:
		case SCREENDATADOOR:
		case SCREENDATAENEMY:
		case SCREENDATAFFINITIALISED:
		case SCREENDATAFLAGS:
		case SCREENDATALAYERINVIS:
		case SCREENDATALAYERMAP:
		case SCREENDATALAYEROPACITY:
		case SCREENDATALAYERSCREEN:
		case SCREENDATANUMFF:
		case SCREENDATAPATH:
		case SCREENDATASCRIPTDRAWS:
		case SCREENDATASECRETCOMBO:
		case SCREENDATASECRETCSET:
		case SCREENDATASECRETFLAG:
		case SCREENDATASIDEWARPDMAP:
		case SCREENDATASIDEWARPOVFLAGS:
		case SCREENDATASIDEWARPSC:
		case SCREENDATASIDEWARPTYPE:
		case SCREENDATASWARPRETSQR:
		case SCREENDATATILEWARPDMAP:
		case SCREENDATATILEWARPOVFLAGS:
		case SCREENDATATILEWARPSCREEN:
		case SCREENDATATILEWARPTYPE:
		case SCREENDATATWARPRETSQR:
		case SCREENDATAWARPRETX:
		case SCREENDATAWARPRETY:
		case SCREENEFLAGSD:
		case SCREENEXSTATED:
		case SCREENFLAGSD:
		case SCREENINITD:
		case SCREENLENSHIDES:
		case SCREENLENSSHOWS:
		case SCREENSCRDATA:
		case SCREENSIDEWARPID:
		case SCREENSTATED:
		case SCRIPTRAMD:
		case SDD:
		case SHOPDATAHASITEM:
		case SHOPDATAITEM:
		case SHOPDATAPRICE:
		case SHOPDATASTRING:
		case SPRITEDATAFLAGS:
		case STDARR:
		case SUBDATABTNLEFT:
		case SUBDATABTNRIGHT:
		case SUBDATAFLAGS:
		case SUBDATAINITD:
		case SUBDATAPAGES:
		case SUBDATASELECTORASPD:
		case SUBDATASELECTORCSET:
		case SUBDATASELECTORDELAY:
		case SUBDATASELECTORFLASHCSET:
		case SUBDATASELECTORFRM:
		case SUBDATASELECTORHEI:
		case SUBDATASELECTORTILE:
		case SUBDATASELECTORWID:
		case SUBDATATRANSARGS:
		case SUBDATATRANSFLAGS:
		case SUBDATATRANSLEFTARGS:
		case SUBDATATRANSLEFTFLAGS:
		case SUBDATATRANSRIGHTARGS:
		case SUBDATATRANSRIGHTFLAGS:
		case SUBPGWIDGETS:
		case SUBWIDGBTNPG:
		case SUBWIDGBTNPRESS:
		case SUBWIDGFLAG:
		case SUBWIDGGENFLAG:
		case SUBWIDGPOSES:
		case SUBWIDGPOSFLAG:
		case SUBWIDGPRESSINITD:
		case SUBWIDGSELECTORASPD:
		case SUBWIDGSELECTORCSET:
		case SUBWIDGSELECTORDELAY:
		case SUBWIDGSELECTORFLASHCSET:
		case SUBWIDGSELECTORFRM:
		case SUBWIDGSELECTORHEI:
		case SUBWIDGSELECTORTILE:
		case SUBWIDGSELECTORWID:
		case SUBWIDGTRANSPGARGS:
		case SUBWIDGTRANSPGFLAGS:
		case SUBWIDGTY_CORNER:
		case SUBWIDGTY_COUNTERS:
		case SUBWIDGTY_CSET:
		case SUBWIDGTY_TILE:
		case TANGOARR:
		{
			static auto r1 = {rINDEX};
			return r1;
		}

		case CREATEBITMAP:
		case CREATELWPNDX:
		case GLOBALRAM:
		case LINKOTILE:
		case LOADMAPDATA:
		case MAPDATAINITA:
		case MAPDATAINTID:
		case MODULEGETINT:
		case NPCCOLLISION:
		case NPCLINEDUP:
		case SCREENCATCH:
		case SCREENENTX:
		case SCREENENTY:
		case SCREENGUY:
		case SCREENITEM:
		case SCREENROOM:
		case SCREENSTATEDD:
		case SCREENSTRING:
		case SCREENUNDCMB:
		case SCREENUNDCST:
		case SCRIPTRAM:
		case SDDD:
		{
			static auto r = {rINDEX, rINDEX2};
			return r;
		}

		case COMBOCDM:
		case COMBODDM:
		case COMBOFDM:
		case COMBOIDM:
		case COMBOSDM:
		case COMBOTDM:
		case SDDDD:
		{
			static auto r = {rINDEX, rINDEX2, rEXP1};
			return r;
		}

		case DISTANCE:
		case LONGDISTANCE:
		{
			static auto r = {rINDEX, rINDEX2, rEXP1, rSFTEMP};
			return r;
		}

		case DISTANCESCALE:
		case LONGDISTANCESCALE:
		{
			static auto r = {rINDEX, rINDEX2, rEXP1, rSFTEMP, rWHAT_NO_7};
			return r;
		}
	}

	return {};
}

std::optional<int> get_register_ref_dependency(int reg)
{
	switch (reg)
	{
		case DATA:
		case DELAY:
		case FCSET:
		case FFCHEIGHT:
		case FFCID:
		case FFCWIDTH:
		case FFFLAGSD:
		case FFLINK:
		case FFMISCD:
		case FFSCRIPT:
		case FFTHEIGHT:
		case FFTWIDTH:
		case FX:
		case FY:
		case XD:
		case XD2:
		case YD:
		case YD2:
			return REFFFC;

		case COMBODACLK:
		case COMBODAKIMANIMY:
		case COMBODANIMFLAGS:
		case COMBODASPEED:
		case COMBODATAINITD:
		case COMBODATASCRIPT:
		case COMBODATTRIBUTES:
		case COMBODATTRIBYTES:
		case COMBODATTRISHORTS:
		case COMBODBLOCKHOLE:
		case COMBODBLOCKNPC:
		case COMBODBLOCKTRIG:
		case COMBODBLOCKWEAPON:
		case COMBODBLOCKWPNLEVEL:
		case COMBODCONVXSPEED:
		case COMBODCONVYSPEED:
		case COMBODCSET:
		case COMBODCSET2FLAGS:
		case COMBODDIRCHANGETYPE:
		case COMBODDISTANCECHANGETILES:
		case COMBODDIVEITEM:
		case COMBODDOCK:
		case COMBODEFFECT:
		case COMBODEXPANSION:
		case COMBODFAIRY:
		case COMBODFFATTRCHANGE:
		case COMBODFLAG:
		case COMBODFLIP:
		case COMBODFOO:
		case COMBODFOORDECOTILE:
		case COMBODFOORDECOTYPE:
		case COMBODFRAME:
		case COMBODFRAMES:
		case COMBODGENFLAGARR:
		case COMBODHOOKSHOTPOINT:
		case COMBODLADDERPASS:
		case COMBODLIFTBREAKSFX:
		case COMBODLIFTBREAKSPRITE:
		case COMBODLIFTDAMAGE:
		case COMBODLIFTFLAGS:
		case COMBODLIFTGFXCCSET:
		case COMBODLIFTGFXCOMBO:
		case COMBODLIFTGFXSPRITE:
		case COMBODLIFTGFXTYPE:
		case COMBODLIFTHEIGHT:
		case COMBODLIFTITEM:
		case COMBODLIFTLEVEL:
		case COMBODLIFTSFX:
		case COMBODLIFTTIME:
		case COMBODLIFTUNDERCMB:
		case COMBODLIFTUNDERCS:
		case COMBODLIFTWEAPONITEM:
		case COMBODLOCKBLOCK:
		case COMBODLOCKBLOCKCHANGE:
		case COMBODMAGICMIRROR:
		case COMBODMODHPAMOUNT:
		case COMBODMODHPDELAY:
		case COMBODMODHPTYPE:
		case COMBODMODMPDELAY:
		case COMBODMODMPTYPE:
		case COMBODNEXTC:
		case COMBODNEXTD:
		case COMBODNEXTTIMER:
		case COMBODNMODMPAMOUNT:
		case COMBODNOPUSHBLOCK:
		case COMBODOTILE:
		case COMBODOVERHEAD:
		case COMBODPLACENPC:
		case COMBODPUSHDIR:
		case COMBODPUSHED:
		case COMBODPUSHHEAVY:
		case COMBODPUSHWAIT:
		case COMBODRAFT:
		case COMBODRESETROOM:
		case COMBODSAVEPOINTTYPE:
		case COMBODSCREENFREEZETYPE:
		case COMBODSECRETCOMBO:
		case COMBODSINGULAR:
		case COMBODSKIPANIM:
		case COMBODSLOWWALK:
		case COMBODSPAWNNPC:
		case COMBODSPAWNNPCCHANGE:
		case COMBODSPAWNNPCWHEN:
		case COMBODSTATUETYPE:
		case COMBODSTEPCHANGEINTO:
		case COMBODSTEPTYPE:
		case COMBODSTRIKECHANGE:
		case COMBODSTRIKEITEM:
		case COMBODSTRIKEREMNANTS:
		case COMBODSTRIKEREMNANTSTYPE:
		case COMBODSTRIKEWEAPONS:
		case COMBODTILE:
		case COMBODTOUCHITEM:
		case COMBODTOUCHSTAIRS:
		case COMBODTRIGCSETCHANGE:
		case COMBODTRIGEXDOORDIR:
		case COMBODTRIGEXDOORIND:
		case COMBODTRIGEXSTATE:
		case COMBODTRIGGERBUTTON:
		case COMBODTRIGGERCHANGECMB:
		case COMBODTRIGGERCOOLDOWN:
		case COMBODTRIGGERCOPYCAT:
		case COMBODTRIGGERCTR:
		case COMBODTRIGGERCTRAMNT:
		case COMBODTRIGGERFLAGS:
		case COMBODTRIGGERFLAGS2:
		case COMBODTRIGGERGENSCRIPT:
		case COMBODTRIGGERGROUP:
		case COMBODTRIGGERGROUPVAL:
		case COMBODTRIGGERGSTATE:
		case COMBODTRIGGERGTIMER:
		case COMBODTRIGGERITEM:
		case COMBODTRIGGERLEVEL:
		case COMBODTRIGGERLIGHTBEAM:
		case COMBODTRIGGERLSTATE:
		case COMBODTRIGGERPROX:
		case COMBODTRIGGERSENS:
		case COMBODTRIGGERSFX:
		case COMBODTRIGGERTIMER:
		case COMBODTRIGGERTYPE:
		case COMBODTRIGITEMPICKUP:
		case COMBODTRIGSPAWNENEMY:
		case COMBODTRIGSPAWNITEM:
		case COMBODTYPE:
		case COMBODUSRFLAGARR:
		case COMBODUSRFLAGS:
		case COMBODWALK:
		case COMBODWARPDIRECT:
		case COMBODWARPLOCATION:
		case COMBODWARPSENS:
		case COMBODWARPTYPE:
		case COMBODWATER:
		case COMBODWHISTLE:
		case COMBODWINGAME:
			return REFCOMBODATA;

		case GETRENDERTARGET:
		case ITEMACLK:
		case ITEMASPEED:
		case ITEMCOUNT:
		case ITEMCSET:
		case ITEMDELAY:
		case ITEMDIR:
		case ITEMDRAWTYPE:
		case ITEMDROPPEDBY:
		case ITEMDROWNCLK:
		case ITEMDROWNCMB:
		case ITEMENGINEANIMATE:
		case ITEMEXTEND:
		case ITEMFAKEJUMP:
		case ITEMFAKEZ:
		case ITEMFALLCLK:
		case ITEMFALLCMB:
		case ITEMFAMILY:
		case ITEMFLASH:
		case ITEMFLASHCSET:
		case ITEMFLIP:
		case ITEMFORCEGRAB:
		case ITEMFRAME:
		case ITEMFRAMES:
		case ITEMGLOWRAD:
		case ITEMGLOWSHP:
		case ITEMGRAVITY:
		case ITEMHXOFS:
		case ITEMHXSZ:
		case ITEMHYOFS:
		case ITEMHYSZ:
		case ITEMHZSZ:
		case ITEMID:
		case ITEMJUMP:
		case ITEMLEVEL:
		case ITEMMISCD:
		case ITEMMOVEFLAGS:
		case ITEMNOHOLDSOUND:
		case ITEMNOSOUND:
		case ITEMOTILE:
		case ITEMOVERRIDEFLAGS:
		case ITEMPICKUP:
		case ITEMPSTRING:
		case ITEMPSTRINGFLAGS:
		case ITEMROTATION:
		case ITEMSCALE:
		case ITEMSCRIPTFLIP:
		case ITEMSCRIPTTILE:
		case ITEMSCRIPTUID:
		case ITEMSHADOWSPR:
		case ITEMSHADOWXOFS:
		case ITEMSHADOWYOFS:
		case ITEMSPRITEINITD:
		case ITEMSPRITESCRIPT:
		case ITEMTILE:
		case ITEMTXSZ:
		case ITEMTYSZ:
		case ITEMX:
		case ITEMXOFS:
		case ITEMY:
		case ITEMYOFS:
		case ITEMZ:
		case ITEMZOFS:
		case ITMSWHOOKED:
			return REFITEM;

		case DEBUGREFNPC:
		case NPCBEHAVIOUR:
		case NPCBGSFX:
		case NPCBOSSPAL:
		case NPCCANFLICKER:
		case NPCCOLLDET:
		case NPCCSET:
		case NPCDD:
		case NPCDEATHSPR:
		case NPCDEFENSED:
		case NPCDIR:
		case NPCDP:
		case NPCDRAWTYPE:
		case NPCDROWNCLK:
		case NPCDROWNCMB:
		case NPCENGINEANIMATE:
		case NPCEXTEND:
		case NPCFADING:
		case NPCFAKEJUMP:
		case NPCFAKEZ:
		case NPCFALLCLK:
		case NPCFALLCMB:
		case NPCFLICKERCOLOR:
		case NPCFLICKERTRANSP:
		case NPCFRAME:
		case NPCFRAMERATE:
		case NPCFROZEN:
		case NPCFROZENCSET:
		case NPCFROZENTILE:
		case NPCGLOWRAD:
		case NPCGLOWSHP:
		case NPCGRAVITY:
		case NPCHALTCLK:
		case NPCHALTRATE:
		case NPCHASITEM:
		case NPCHITBY:
		case NPCHITDIR:
		case NPCHOMING:
		case NPCHP:
		case NPCHUNGER:
		case NPCHXOFS:
		case NPCHXSZ:
		case NPCHYOFS:
		case NPCHYSZ:
		case NPCHZSZ:
		case NPCIMMORTAL:
		case NPCINITD:
		case NPCINVINC:
		case NPCISCORE:
		case NPCITEMSET:
		case NPCJUMP:
		case NPCKNOCKBACKSPEED:
		case NPCMISCD:
		case NPCMOVEFLAGS:
		case NPCMOVESTATUS:
		case NPCNOSCRIPTKB:
		case NPCNOSLIDE:
		case NPCORIGINALHP:
		case NPCOTILE:
		case NPCPARENTUID:
		case NPCRANDOM:
		case NPCRATE:
		case NPCRINGLEAD:
		case NPCROTATION:
		case NPCSCALE:
		case NPCSCRDEFENSED:
		case NPCSCRIPT:
		case NPCSCRIPTFLIP:
		case NPCSCRIPTTILE:
		case NPCSHADOWSPR:
		case NPCSHADOWXOFS:
		case NPCSHADOWYOFS:
		case NPCSHIELD:
		case NPCSLIDECLK:
		case NPCSPAWNSPR:
		case NPCSTEP:
		case NPCSTUN:
		case NPCSUPERMAN:
		case NPCTILE:
		case NPCTXSZ:
		case NPCTYPE:
		case NPCTYSZ:
		case NPCWDP:
		case NPCWEAPON:
		case NPCWEAPSPRITE:
		case NPCX:
		case NPCXOFS:
		case NPCY:
		case NPCYOFS:
		case NPCZ:
		case NPCZOFS:
			return REFNPC;
	}

	// TODO: see has_implemented_register_invalidations

	return std::nullopt;
}

bool has_register_dependency(int reg)
{
	return get_register_dependencies(reg).size() || get_register_ref_dependency(reg).has_value();
}

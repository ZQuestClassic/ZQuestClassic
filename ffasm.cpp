#ifndef __GTHREAD_HIDE_WIN32API
#define __GTHREAD_HIDE_WIN32API 1
#endif                            //prevent indirectly including windows.h

#include "precompiled.h" //always first

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "zc_malloc.h"
#include "ffasm.h"
#include "zquest.h"
#include "zsys.h"
#ifdef ALLEGRO_MACOSX
#define strnicmp strncasecmp
#endif

#ifdef ALLEGRO_MACOSX
#define strnicmp strncasecmp
#endif

#ifdef ALLEGRO_LINUX
#define strnicmp strncasecmp
#endif

#ifdef _MSC_VER
#define stricmp _stricmp
#define strnicmp _strnicmp
#endif

extern char *datapath, *temppath;

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
    { "ALLOCATEGMEMV",       2,   0,   1,   0},
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
    { "GETSAVENAME",         1,   0,   0,   0},
    { "ARRAYSIZE",           1,   0,   0,   0},
    { "ITEMNAME",            1,   0,   0,   0},
    { "SETSAVENAME",         1,   0,   0,   0},
    { "NPCNAME",             1,   0,   0,   0},
    { "GETMESSAGE",          2,   0,   0,   0},
    { "GETDMAPNAME",         2,   0,   0,   0},
    { "GETDMAPTITLE",        2,   0,   0,   0},
    { "GETDMAPINTRO",        2,   0,   0,   0},
    { "ALLOCATEGMEMR",       2,   0,   0,   0},
    { "DRAWBITMAP",          0,   0,   0,   0},
    { "SETRENDERTARGET",     0,   0,   0,   0},
    { "PLAYENHMUSIC",        2,   0,   0,   0},
    { "GETMUSICFILE",        2,   0,   0,   0},
    { "GETMUSICTRACK",       1,   0,   0,   0},
    { "SETDMAPENHMUSIC",     0,   0,   0,   0},
    { "DRAWLAYER",           0,   0,   0,   0},
    { "DRAWSCREEN",          0,   0,   0,   0},
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
    { "GETFFCSCRIPT",        1,   0,   0,   0},
    { "",                    0,   0,   0,   0}
};


script_variable variable_list[]=
{
    //name                id                maxcount       multiple
    { "D",                 D(0),                 8,             0 },
    { "A",                 A(0),                 2,             0 },
    { "DATA",              DATA,                 0,             0 },
    { "CSET",              FCSET,                0,             0 },
    { "DELAY",             DELAY,                0,             0 },
    { "X",                 FX,                   0,             0 },
    { "Y",                 FY,                   0,             0 },
    { "XD",                XD,                   0,             0 },
    { "YD",                YD,                   0,             0 },
    { "XD2",               XD2,                  0,             0 },
    { "YD2",               YD2,                  0,             0 },
    { "FLAG",              FLAG,                 0,             0 },
    { "WIDTH",             WIDTH,                0,             0 },
    { "HEIGHT",            HEIGHT,               0,             0 },
    { "LINK",              LINK,                 0,             0 },
    { "FFFLAGSD",          FFFLAGSD,             0,             0 },
    { "FFCWIDTH",          FFCWIDTH,             0,             0 },
    { "FFCHEIGHT",         FFCHEIGHT,            0,             0 },
    { "FFTWIDTH",          FFTWIDTH,             0,             0 },
    { "FFTHEIGHT",         FFTHEIGHT,            0,             0 },
    { "FFLINK",            FFLINK,               0,             0 },
    //{ "COMBOD",            COMBOD(0),          176,             3 },
    //{ "COMBOC",            COMBOC(0),          176,             3 },
    //{ "COMBOF",            COMBOF(0),          176,             3 },
    { "INPUTSTART",        INPUTSTART,           0,             0 },
    { "INPUTUP",           INPUTUP,              0,             0 },
    { "INPUTDOWN",         INPUTDOWN,            0,             0 },
    { "INPUTLEFT",         INPUTLEFT,            0,             0 },
    { "INPUTRIGHT",        INPUTRIGHT,           0,             0 },
    { "INPUTA",            INPUTA,               0,             0 },
    { "INPUTB",            INPUTB,               0,             0 },
    { "INPUTL",            INPUTL,               0,             0 },
    { "INPUTR",            INPUTR,               0,             0 },
    { "INPUTMOUSEX",       INPUTMOUSEX,          0,             0 },
    { "INPUTMOUSEY",       INPUTMOUSEY,          0,             0 },
    { "LINKX",             LINKX,                0,             0 },
    { "LINKY",             LINKY,                0,             0 },
    { "LINKZ",             LINKZ,                0,             0 },
    { "LINKJUMP",          LINKJUMP,             0,             0 },
    { "LINKDIR",           LINKDIR,              0,             0 },
    { "LINKHITDIR",        LINKHITDIR,           0,             0 },
    { "LINKHP",            LINKHP,               0,             0 },
    { "LINKMP",            LINKMP,               0,             0 },
    { "LINKMAXHP",         LINKMAXHP,            0,             0 },
    { "LINKMAXMP",         LINKMAXMP,            0,             0 },
    { "LINKACTION",        LINKACTION,           0,             0 },
    { "LINKHELD",          LINKHELD,             0,             0 },
    { "LINKITEMD",         LINKITEMD,            0,             0 },
    { "LINKSWORDJINX",     LINKSWORDJINX,        0,             0 },
    { "LINKITEMJINX",      LINKITEMJINX,         0,             0 },
    { "LINKDRUNK",         LINKDRUNK,            0,             0 },
    { "ITEMX",             ITEMX,                0,             0 },
    { "ITEMY",             ITEMY,                0,             0 },
    { "ITEMZ",             ITEMZ,                0,             0 },
    { "ITEMJUMP",          ITEMJUMP,             0,             0 },
    { "ITEMDRAWTYPE",      ITEMDRAWTYPE,         0,             0 },
    { "ITEMID",            ITEMID,               0,             0 },
    { "ITEMTILE",          ITEMTILE,             0,             0 },
    { "ITEMOTILE",         ITEMOTILE,            0,             0 },
    { "ITEMCSET",          ITEMCSET,             0,             0 },
    { "ITEMFLASHCSET",     ITEMFLASHCSET,        0,             0 },
    { "ITEMFRAMES",        ITEMFRAMES,           0,             0 },
    { "ITEMFRAME",         ITEMFRAME,            0,             0 },
    { "ITEMASPEED",        ITEMASPEED,           0,             0 },
    { "ITEMDELAY",         ITEMDELAY,            0,             0 },
    { "ITEMFLASH",         ITEMFLASH,            0,             0 },
    { "ITEMFLIP",          ITEMFLIP,             0,             0 },
    { "ITEMCOUNT",         ITEMCOUNT,            0,             0 },
    { "IDATAFAMILY",       IDATAFAMILY,          0,             0 },
    { "IDATALEVEL",        IDATALEVEL,           0,             0 },
    { "IDATAKEEP",         IDATAKEEP,            0,             0 },
    { "IDATAAMOUNT",       IDATAAMOUNT,          0,             0 },
    { "IDATASETMAX",       IDATASETMAX,          0,             0 },
    { "IDATAMAX",          IDATAMAX,             0,             0 },
    { "IDATACOUNTER",      IDATACOUNTER,         0,             0 },
    { "ITEMEXTEND",        ITEMEXTEND,           0,             0 },
    { "NPCX",              NPCX,                 0,             0 },
    { "NPCY",              NPCY,                 0,             0 },
    { "NPCZ",              NPCZ,                 0,             0 },
    { "NPCJUMP",           NPCJUMP,              0,             0 },
    { "NPCDIR",            NPCDIR,               0,             0 },
    { "NPCRATE",           NPCRATE,              0,             0 },
    { "NPCSTEP",           NPCSTEP,              0,             0 },
    { "NPCFRAMERATE",      NPCFRAMERATE,         0,             0 },
    { "NPCHALTRATE",       NPCHALTRATE,          0,             0 },
    { "NPCDRAWTYPE",       NPCDRAWTYPE,          0,             0 },
    { "NPCHP",             NPCHP,                0,             0 },
    { "NPCID",             NPCID,                0,             0 },
    { "NPCDP",             NPCDP,                0,             0 },
    { "NPCWDP",            NPCWDP,               0,             0 },
    { "NPCOTILE",          NPCOTILE,             0,             0 },
    { "NPCENEMY",          NPCENEMY,             0,             0 },
    { "NPCWEAPON",         NPCWEAPON,            0,             0 },
    { "NPCITEMSET",        NPCITEMSET,           0,             0 },
    { "NPCCSET",           NPCCSET,              0,             0 },
    { "NPCBOSSPAL",        NPCBOSSPAL,           0,             0 },
    { "NPCBGSFX",          NPCBGSFX,             0,             0 },
    { "NPCCOUNT",          NPCCOUNT,             0,             0 },
    { "GD",                GD(0),              256,             0 },
    { "SDD",               SDD,                  0,             0 },
    { "GDD",               GDD,                  0,             0 },
    { "SDDD",              SDDD,                 0,             0 },
    { "SCRDOORD",          SCRDOORD,             0,             0 },
    { "GAMEDEATHS",        GAMEDEATHS,           0,             0 },
    { "GAMECHEAT",         GAMECHEAT,            0,             0 },
    { "GAMETIME",          GAMETIME,             0,             0 },
    { "GAMEHASPLAYED",     GAMEHASPLAYED,        0,             0 },
    { "GAMETIMEVALID",     GAMETIMEVALID,        0,             0 },
    { "GAMEGUYCOUNT",      GAMEGUYCOUNT,         0,             0 },
    { "GAMECONTSCR",       GAMECONTSCR,          0,             0 },
    { "GAMECONTDMAP",      GAMECONTDMAP,         0,             0 },
    { "GAMECOUNTERD",      GAMECOUNTERD,         0,             0 },
    { "GAMEMCOUNTERD",     GAMEMCOUNTERD,        0,             0 },
    { "GAMEDCOUNTERD",     GAMEDCOUNTERD,        0,             0 },
    { "GAMEGENERICD",      GAMEGENERICD,         0,             0 },
    { "GAMEITEMSD",        GAMEITEMSD,           0,             0 },
    { "GAMELITEMSD",       GAMELITEMSD,          0,             0 },
    { "GAMELKEYSD",        GAMELKEYSD,           0,             0 },
    { "SCREENSTATED",      SCREENSTATED,         0,             0 },
    { "SCREENSTATEDD",     SCREENSTATEDD,        0,             0 },
    { "GAMEGUYCOUNTD",     GAMEGUYCOUNTD,        0,             0 },
    { "CURMAP",            CURMAP,               0,             0 },
    { "CURSCR",            CURSCR,               0,             0 },
    { "CURDSCR",           CURDSCR,              0,             0 },
    { "CURDMAP",           CURDMAP,              0,             0 },
    { "COMBODD",           COMBODD,              0,             0 },
    { "COMBOCD",           COMBOCD,              0,             0 },
    { "COMBOFD",           COMBOFD,              0,             0 },
    { "COMBOTD",           COMBOTD,              0,             0 },
    { "COMBOID",           COMBOID,              0,             0 },
    { "COMBOSD",           COMBOSD,              0,             0 },
    { "REFITEMCLASS",      REFITEMCLASS,         0,             0 },
    { "REFITEM",           REFITEM,              0,             0 },
    { "REFFFC",            REFFFC,               0,             0 },
    { "REFLWPN",           REFLWPN,              0,             0 },
    { "REFEWPN",           REFEWPN,              0,             0 },
    { "REFLWPNCLASS",      REFLWPNCLASS,         0,             0 },
    { "REFEWPNCLASS",      REFEWPNCLASS,         0,             0 },
    { "REFNPC",            REFNPC,               0,             0 },
    { "REFNPCCLASS",       REFNPCCLASS,          0,             0 },
    { "LWPNX",             LWPNX,                0,             0 },
    { "LWPNY",             LWPNY,                0,             0 },
    { "LWPNZ",             LWPNZ,                0,             0 },
    { "LWPNJUMP",          LWPNJUMP,             0,             0 },
    { "LWPNDIR",           LWPNDIR,              0,             0 },
    { "LWPNSTEP",          LWPNSTEP,             0,             0 },
    { "LWPNANGULAR",       LWPNANGULAR,          0,             0 },
    { "LWPNANGLE",         LWPNANGLE,            0,             0 },
    { "LWPNDRAWTYPE",      LWPNDRAWTYPE,         0,             0 },
    { "LWPNPOWER",         LWPNPOWER,            0,             0 },
    { "LWPNDEAD",          LWPNDEAD,             0,             0 },
    { "LWPNID",            LWPNID,               0,             0 },
    { "LWPNTILE",          LWPNTILE,             0,             0 },
    { "LWPNCSET",          LWPNCSET,             0,             0 },
    { "LWPNFLASHCSET",     LWPNFLASHCSET,        0,             0 },
    { "LWPNFRAMES",        LWPNFRAMES,           0,             0 },
    { "LWPNFRAME",         LWPNFRAME,            0,             0 },
    { "LWPNASPEED",        LWPNASPEED,           0,             0 },
    { "LWPNFLASH",         LWPNFLASH,            0,             0 },
    { "LWPNFLIP",          LWPNFLIP,             0,             0 },
    { "LWPNCOUNT",         LWPNCOUNT,            0,             0 },
    { "LWPNEXTEND",        LWPNEXTEND,           0,             0 },
    { "LWPNOTILE",         LWPNOTILE,            0,             0 },
    { "LWPNOCSET",         LWPNOCSET,            0,             0 },
    { "EWPNX",             EWPNX,                0,             0 },
    { "EWPNY",             EWPNY,                0,             0 },
    { "EWPNZ",             EWPNZ,                0,             0 },
    { "EWPNJUMP",          EWPNJUMP,             0,             0 },
    { "EWPNDIR",           EWPNDIR,              0,             0 },
    { "EWPNSTEP",          EWPNSTEP,             0,             0 },
    { "EWPNANGULAR",       EWPNANGULAR,          0,             0 },
    { "EWPNANGLE",         EWPNANGLE,            0,             0 },
    { "EWPNDRAWTYPE",      EWPNDRAWTYPE,         0,             0 },
    { "EWPNPOWER",         EWPNPOWER,            0,             0 },
    { "EWPNDEAD",          EWPNDEAD,             0,             0 },
    { "EWPNID",            EWPNID,               0,             0 },
    { "EWPNTILE",          EWPNTILE,             0,             0 },
    { "EWPNCSET",          EWPNCSET,             0,             0 },
    { "EWPNFLASHCSET",     EWPNFLASHCSET,        0,             0 },
    { "EWPNFRAMES",        EWPNFRAMES,           0,             0 },
    { "EWPNFRAME",         EWPNFRAME,            0,             0 },
    { "EWPNASPEED",        EWPNASPEED,           0,             0 },
    { "EWPNFLASH",         EWPNFLASH,            0,             0 },
    { "EWPNFLIP",          EWPNFLIP,             0,             0 },
    { "EWPNCOUNT",         EWPNCOUNT,            0,             0 },
    { "EWPNEXTEND",        EWPNEXTEND,           0,             0 },
    { "EWPNOTILE",         EWPNOTILE,            0,             0 },
    { "EWPNOCSET",         EWPNOCSET,            0,             0 },
    { "NPCEXTEND",         NPCEXTEND,            0,             0 },
    { "SP",                SP,                   0,             0 },
    { "SP",                SP,                   0,             0 },
    { "WAVY",              WAVY,                 0,             0 },
    { "QUAKE",             QUAKE,                0,             0 },
    { "IDATAUSESOUND",     IDATAUSESOUND,        0,             0 },
    { "INPUTMOUSEZ",       INPUTMOUSEZ,          0,             0 },
    { "INPUTMOUSEB",       INPUTMOUSEB,          0,             0 },
    { "COMBODDM",          COMBODDM,             0,             0 },
    { "COMBOCDM",           COMBOCDM,            0,             0 },
    { "COMBOFDM",           COMBOFDM,            0,             0 },
    { "COMBOTDM",           COMBOTDM,            0,             0 },
    { "COMBOIDM",           COMBOIDM,            0,             0 },
    { "COMBOSDM",           COMBOSDM,            0,             0 },
    { "SCRIPTRAM",          SCRIPTRAM,           0,             0 },
    { "GLOBALRAM",          GLOBALRAM,           0,             0 },
    { "SCRIPTRAMD",         SCRIPTRAMD,          0,             0 },
    { "GLOBALRAMD",         GLOBALRAMD,          0,             0 },
    { "LWPNHXOFS",          LWPNHXOFS,           0,             0 },
    { "LWPNHYOFS",          LWPNHYOFS,           0,             0 },
    { "LWPNXOFS",           LWPNXOFS,            0,             0 },
    { "LWPNYOFS",           LWPNYOFS,            0,             0 },
    { "LWPNZOFS",           LWPNZOFS,            0,             0 },
    { "LWPNHXSZ",           LWPNHXSZ,            0,             0 },
    { "LWPNHYSZ",           LWPNHYSZ,            0,             0 },
    { "LWPNHZSZ",           LWPNHZSZ,            0,             0 },
    { "EWPNHXOFS",          EWPNHXOFS,           0,             0 },
    { "EWPNHYOFS",          EWPNHYOFS,           0,             0 },
    { "EWPNXOFS",           EWPNXOFS,            0,             0 },
    { "EWPNYOFS",           EWPNYOFS,            0,             0 },
    { "EWPNZOFS",           EWPNZOFS,            0,             0 },
    { "EWPNHXSZ",           EWPNHXSZ,            0,             0 },
    { "EWPNHYSZ",           EWPNHYSZ,            0,             0 },
    { "EWPNHZSZ",           EWPNHZSZ,            0,             0 },
    { "NPCHXOFS",           NPCHXOFS,            0,             0 },
    { "NPCHYOFS",           NPCHYOFS,            0,             0 },
    { "NPCXOFS",            NPCXOFS,             0,             0 },
    { "NPCYOFS",            NPCYOFS,             0,             0 },
    { "NPCZOFS",            NPCZOFS,             0,             0 },
    { "NPCHXSZ",            NPCHXSZ,             0,             0 },
    { "NPCHYSZ",            NPCHYSZ,             0,             0 },
    { "NPCHZSZ",            NPCHZSZ,             0,             0 },
    { "ITEMHXOFS",          ITEMHXOFS,           0,             0 },
    { "ITEMHYOFS",          ITEMHYOFS,           0,             0 },
    { "ITEMXOFS",           ITEMXOFS,            0,             0 },
    { "ITEMYOFS",           ITEMYOFS,            0,             0 },
    { "ITEMZOFS",           ITEMZOFS,            0,             0 },
    { "ITEMHXSZ",           ITEMHXSZ,            0,             0 },
    { "ITEMHYSZ",           ITEMHYSZ,            0,             0 },
    { "ITEMHZSZ",           ITEMHZSZ,            0,             0 },
    { "LWPNTXSZ",           LWPNTXSZ,            0,             0 },
    { "LWPNTYSZ",           LWPNTYSZ,            0,             0 },
    { "EWPNTXSZ",           EWPNTXSZ,            0,             0 },
    { "EWPNTYSZ",           EWPNTYSZ,            0,             0 },
    { "NPCTXSZ",            NPCTXSZ,             0,             0 },
    { "NPCTYSZ",            NPCTYSZ,             0,             0 },
    { "ITEMTXSZ",           ITEMTXSZ,            0,             0 },
    { "ITEMTYSZ",           ITEMTYSZ,            0,             0 },
    { "LINKHXOFS",          LINKHXOFS,           0,             0 },
    { "LINKHYOFS",          LINKHYOFS,           0,             0 },
    { "LINKXOFS",           LINKXOFS,            0,             0 },
    { "LINKYOFS",           LINKYOFS,            0,             0 },
    { "LINKZOFS",           LINKZOFS,            0,             0 },
    { "LINKHXSZ",           LINKHXSZ,            0,             0 },
    { "LINKHYSZ",           LINKHYSZ,            0,             0 },
    { "LINKHZSZ",           LINKHZSZ,            0,             0 },
    { "LINKTXSZ",           LINKTXSZ,            0,             0 },
    { "LINKTYSZ",           LINKTYSZ,            0,             0 },
    { "NPCTILE",            NPCTILE,             0,             0 },
    { "LWPNBEHIND",         LWPNBEHIND,          0,             0 },
    { "EWPNBEHIND",         EWPNBEHIND,          0,             0 },
    { "SDDDD",              SDDDD,               0,             0 },
    { "CURLEVEL",           CURLEVEL,            0,             0 },
    { "ITEMPICKUP",         ITEMPICKUP,          0,             0 },
    { "INPUTMAP",           INPUTMAP,            0,             0 },
    { "LIT",                LIT,                 0,             0 },
    { "INPUTEX1",           INPUTEX1,            0,             0 },
    { "INPUTEX2",           INPUTEX2,            0,             0 },
    { "INPUTEX3",           INPUTEX3,            0,             0 },
    { "INPUTEX4",           INPUTEX4,            0,             0 },
    { "INPUTPRESSSTART",    INPUTPRESSSTART,     0,             0 },
    { "INPUTPRESSUP",       INPUTPRESSUP,        0,             0 },
    { "INPUTPRESSDOWN",     INPUTPRESSDOWN,      0,             0 },
    { "INPUTPRESSLEFT",     INPUTPRESSLEFT,      0,             0 },
    { "INPUTPRESSRIGHT",    INPUTPRESSRIGHT,     0,             0 },
    { "INPUTPRESSA",        INPUTPRESSA,         0,             0 },
    { "INPUTPRESSB",        INPUTPRESSB,         0,             0 },
    { "INPUTPRESSL",        INPUTPRESSL,         0,             0 },
    { "INPUTPRESSR",        INPUTPRESSR,         0,             0 },
    { "INPUTPRESSEX1",      INPUTPRESSEX1,       0,             0 },
    { "INPUTPRESSEX2",      INPUTPRESSEX2,       0,             0 },
    { "INPUTPRESSEX3",      INPUTPRESSEX3,       0,             0 },
    { "INPUTPRESSEX4",      INPUTPRESSEX4,       0,             0 },
    { "LWPNMISCD",          LWPNMISCD,           0,             0 },
    { "EWPNMISCD",          EWPNMISCD,           0,             0 },
    { "NPCMISCD",           NPCMISCD,            0,             0 },
    { "ITEMMISCD",          ITEMMISCD,           0,             0 },
    { "FFMISCD",            FFMISCD,             0,             0 },
    { "GETMIDI",            GETMIDI,             0,             0 },
    { "NPCHOMING",          NPCHOMING,           0,             0 },
    { "NPCDD",			  NPCDD,			   0,             0 },
    { "LINKEQUIP",		  LINKEQUIP,		   0,             0 },
    { "INPUTAXISUP",        INPUTAXISUP,         0,             0 },
    { "INPUTAXISDOWN",      INPUTAXISDOWN,       0,             0 },
    { "INPUTAXISLEFT",      INPUTAXISLEFT,       0,             0 },
    { "INPUTAXISRIGHT",     INPUTAXISRIGHT,      0,             0 },
    { "PRESSAXISUP",        INPUTPRESSAXISUP,    0,             0 },
    { "PRESSAXISDOWN",      INPUTPRESSAXISDOWN,  0,             0 },
    { "PRESSAXISLEFT",      INPUTPRESSAXISLEFT,  0,             0 },
    { "PRESSAXISRIGHT",     INPUTPRESSAXISRIGHT, 0,             0 },
    { "NPCTYPE",			  NPCTYPE,             0,             0 },
    { "FFSCRIPT",			  FFSCRIPT,            0,             0 },
    { "SCREENFLAGSD",       SCREENFLAGSD,        0,             0 },
    { "LINKINVIS",          LINKINVIS,           0,             0 },
    { "LINKINVINC",         LINKINVINC,          0,             0 },
    { "SCREENEFLAGSD",      SCREENEFLAGSD,       0,             0 },
    { "NPCMFLAGS",          NPCMFLAGS,           0,             0 },
    { "FFINITDD",           FFINITDD,            0,             0 },
    { "LINKMISCD",          LINKMISCD,           0,             0 },
    { "DMAPFLAGSD",         DMAPFLAGSD,          0,             0 },
    { "LWPNCOLLDET",        LWPNCOLLDET,         0,             0 },
    { "EWPNCOLLDET",        EWPNCOLLDET,         0,             0 },
    { "NPCCOLLDET",         NPCCOLLDET,          0,             0 },
    { "LINKLADDERX",        LINKLADDERX,         0,             0 },
    { "LINKLADDERY",        LINKLADDERY,         0,             0 },
    { "NPCSTUN",            NPCSTUN,             0,             0 },
    { "NPCDEFENSED",        NPCDEFENSED,         0,             0 },
    { "IDATAPOWER",         IDATAPOWER,          0,             0 },
    { "DMAPLEVELD",         DMAPLEVELD,          0,             0 },
    { "DMAPCOMPASSD",       DMAPCOMPASSD,        0,             0 },
    { "DMAPCONTINUED",      DMAPCONTINUED,       0,             0 },
    { "DMAPMIDID",          DMAPMIDID,           0,             0 },
    { "IDATAINITDD",        IDATAINITDD,         0,             0 },
    { "ROOMTYPE",           ROOMTYPE,            0,             0 },
    { "ROOMDATA",           ROOMDATA,            0,             0 },
    { "LINKTILE",           LINKTILE,            0,             0 },
    { "LINKFLIP",           LINKFLIP,            0,             0 },
    { "INPUTPRESSMAP",      INPUTPRESSMAP,       0,             0 },
    { "NPCHUNGER",          NPCHUNGER,           0,             0 },
    { "GAMESTANDALONE",     GAMESTANDALONE,      0,             0 },
    { "GAMEENTRSCR",        GAMEENTRSCR,         0,             0 },
    { "GAMEENTRDMAP",       GAMEENTRDMAP,        0,             0 },
    { "GAMECLICKFREEZE",    GAMECLICKFREEZE,     0,             0 },
    { "PUSHBLOCKX",         PUSHBLOCKX,          0,             0 },
    { "PUSHBLOCKY",         PUSHBLOCKY,          0,             0 },
    { "PUSHBLOCKCOMBO",     PUSHBLOCKCOMBO,      0,             0 },
    { "PUSHBLOCKCSET",      PUSHBLOCKCSET,       0,             0 },
    { "UNDERCOMBO",         UNDERCOMBO,          0,             0 },
    { "UNDERCSET",          UNDERCSET,           0,             0 },
    { "DMAPOFFSET",         DMAPOFFSET,          0,             0 },
    { "DMAPMAP",            DMAPMAP,             0,             0 },
    { " ",                       -1,             0,             0 }
};

long ffparse(char *string)
{
    //return int(atof(string)*10000);
    
    //this function below isn't working too well yet
    //clean_numeric_string(string);
    double negcheck = atof(string);
    
    //if no decimal point, ascii to int conversion
    char *ptr=strchr(string, '.');
    
    if(!ptr)
    {
        return atoi(string)*10000;
    }
    
    long ret=0;
    char *tempstring1;
    tempstring1=(char *)zc_malloc(strlen(string)+5);
    sprintf(tempstring1, string);
    
    for(int i=0; i<4; ++i)
    {
        tempstring1[strlen(string)+i]='0';
    }
    
    ptr=strchr(tempstring1, '.');
    *ptr=0;
    ret=atoi(tempstring1)*10000;
    
    ++ptr;
    char *ptr2=ptr;
    ptr2+=4;
    *ptr2=0;
    
    if(negcheck<0)
        ret-=atoi(ptr);
    else ret+=atoi(ptr);
    
    zc_free(tempstring1);
    return ret;
}

bool ffcheck(char *arg)
{

    for(int i=0; i<0x100; i++)
    {
        if(arg[i]!='\0')
        {
            if(i==0)
            {
                if(arg[i]!='-' && arg[i]!='.' && !(arg[i] >= '0' && arg[i] <='9'))
                    return false;
            }
            else
            {
                if(arg[i]!='.' && !(arg[i] >= '0' && arg[i] <='9'))
                    return false;
            }
        }
        else
        {
            i=0x100;
        }
    }
    
    return true;
}

char labels[65536][80];
int lines[65536];
int numlines;

int parse_script(ffscript **script)
{
    if(!getname("Import Script (.txt)","txt",NULL,datapath,false))
        return D_CLOSE;
        
    return parse_script_file(script,temppath, true);
}

int parse_script_file(ffscript **script, const char *path, bool report_success)
{
    saved=false;
    FILE *fscript = fopen(path,"rb");
    char *buffer = new char[0x400];
    char *combuf = new char[0x100];
    char *arg1buf = new char[0x100];
    char *arg2buf = new char[0x100];
    bool stop=false;
    bool success=true;
    numlines = 0;
    int num_commands;
    
    for(int i=0;; i++)
    {
        buffer[0]=0;
        
        if(stop)
        {
            num_commands=i+1;
            break;
        }
        
        for(int j=0; j<0x400; j++)
        {
            char temp;
            temp = getc(fscript);
            
            if(feof(fscript))
            {
                stop=true;
                buffer[j]='\0';
                j=0x400;
                ungetc(temp,fscript);
            }
            else
            {
                ungetc(temp,fscript);
                buffer[j] = getc(fscript);
                
                if(buffer[j] == ';' || buffer[j] == '\n' || buffer[j] == 13)
                {
                    if(buffer[j] == '\n')
                    {
                        buffer[j] = '\0';
                        j=0x400;
                    }
                    else
                    {
                        while(getc(fscript)!='\n')
                        {
                            if(feof(fscript))
                            {
                                stop=true;
                                break;
                            }
                        }
                        
                        buffer[j] = '\0';
                        j=0x400;
                    }
                }
            }
        }
        
        int k=0;
        
        while(buffer[k] == ' ' || buffer[k] == '\t') k++;
        
        if(buffer[k] == '\0')
        {
            i--;
            continue;
        }
        
        k=0;
        
        if(buffer[k] != ' ' && buffer[k] !='\t' && buffer[k] != '\0')
        {
            while(buffer[k] != ' ' && buffer[k] !='\t' && buffer[k] != '\0')
            {
                labels[numlines][k] = buffer[k];
                k++;
            }
            
            labels[numlines][k] = '\0';
            lines[numlines] = i;
            numlines++;
        }
    }
    
    fseek(fscript, 0, SEEK_SET);
    stop = false;
    
    if((*script)!=NULL) delete [](*script);
    
    (*script) = new ffscript[num_commands];
    
    for(int i=0; i<num_commands; i++)
    {
        if(stop)
        {
            (*script)[i].command = 0xFFFF;
            break;
        }
        else
        {
            /*
                  sprintf(buffer, "");
                  sprintf(combuf, "");
                  sprintf(arg1buf, "");
                  sprintf(arg2buf, "");
            */
            buffer[0]=0;
            combuf[0]=0;
            arg1buf[0]=0;
            arg2buf[0]=0;
            
            for(int j=0; j<0x400; j++)
            {
                char temp;
                temp = getc(fscript);
                
                if(feof(fscript))
                {
                    stop=true;
                    buffer[j]='\0';
                    j=0x400;
                    ungetc(temp,fscript);
                }
                else
                {
                    ungetc(temp,fscript);
                    buffer[j] = getc(fscript);
                    
                    if(buffer[j] == ';' || buffer[j] == '\n' || buffer[j] == 13)
                    {
                        if(buffer[j] == '\n')
                        {
                            buffer[j] = '\0';
                            j=0x400;
                        }
                        else
                        {
                            while(getc(fscript)!='\n')
                            {
                                if(feof(fscript))
                                {
                                    stop=true;
                                    break;
                                }
                            }
                            
                            buffer[j] = '\0';
                            j=0x400;
                        }
                    }
                }
            }
            
            int k=0, l=0;
            
            while(buffer[k] == ' ' || buffer[k] == '\t') k++;
            
            if(buffer[k] == '\0')
            {
                i--;
                continue;
            }
            
            k=0;
            
            while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0') k++;
            
            while((buffer[k] == ' ' || buffer[k] == '\t') && buffer[k] != '\0')  k++;
            
            while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
            {
                combuf[l] = buffer[k];
                k++;
                l++;
            }
            
            combuf[l] = '\0';
            l=0;
            
            while((buffer[k] == ' ' || buffer[k] == '\t') && buffer[k] != '\0') k++;
            
            while(buffer[k] != ',' && buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
            {
                arg1buf[l] = buffer[k];
                k++;
                l++;
            }
            
            arg1buf[l] = '\0';
            l=0;
            
            while((buffer[k] == ' ' || buffer[k] == '\t' || buffer[k] == ',') && buffer[k] != '\0') k++;
            
            while(buffer[k] != ' ' && buffer[k] != '\t' && buffer[k] != '\0')
            {
                arg2buf[l] = buffer[k];
                k++;
                l++;
            }
            
            arg2buf[l] = '\0';
            int parse_err;
            
            if(!(parse_script_section(combuf, arg1buf, arg2buf, script, i, parse_err)))
            {
                char buf[80],buf2[80],buf3[80],name[13];
                const char* errstrbuf[] =
                {
                    "invalid instruction!",
                    "parameter 1 invalid!",
                    "parameter 2 invalid!"
                };
                extract_name(temppath,name,FILENAME8_3);
                sprintf(buf,"Unable to parse instruction %d from script %s",i+1,name);
                sprintf(buf2,"The error was: %s",errstrbuf[parse_err]);
                sprintf(buf3,"The command was (%s) (%s,%s)",combuf,arg1buf,arg2buf);
                jwin_alert("Error",buf,buf2,buf3,"O&K",NULL,'k',0,lfont);
                stop=true;
                success=false;
                (*script)[0].command = 0xFFFF;
            }
        }
    }
    
    if(report_success && success) //(!stop) // stop is never true here
    {
        char buf[80],name[13];
        extract_name(temppath,name,FILENAME8_3);
        sprintf(buf,"Script %s has been parsed",name);
        jwin_alert("Success",buf,NULL,NULL,"O&K",NULL,'k',0,lfont);
    }
    
    delete [] buffer;
    delete [] combuf;
    delete [] arg1buf;
    delete [] arg2buf;
    fclose(fscript);
    return success?D_O_K:D_CLOSE;
}

int set_argument(char *argbuf, ffscript **script, int com, int argument)
{
    long *arg;
    
    if(argument)
    {
        arg = &((*script)[com].arg2);
    }
    else
    {
        arg = &((*script)[com].arg1);
    }
    
    int i=0;
    char tempvar[20];
    
    while(variable_list[i].id>-1)
    {
        if(variable_list[i].maxcount>1)
        {
            for(int j=0; j<variable_list[i].maxcount; ++j)
            {
                if(strcmp(variable_list[i].name,"A")==0)
                    sprintf(tempvar, "%s%d", variable_list[i].name, j+1);
                else sprintf(tempvar, "%s%d", variable_list[i].name, j);
                
                if(stricmp(argbuf,tempvar)==0)
                {
                    long temp = variable_list[i].id+(j*zc_max(1,variable_list[i].multiple));
                    *arg = temp;
                    return 1;
                }
            }
        }
        else
        {
            if(stricmp(argbuf,variable_list[i].name)==0)
            {
                *arg = variable_list[i].id;
                return 1;
            }
        }
        
        ++i;
    }
    
    return 0;
}

#define ERR_INSTRUCTION 0
#define ERR_PARAM1 1
#define ERR_PARAM2 2

int parse_script_section(char *combuf, char *arg1buf, char *arg2buf, ffscript **script, int com, int &retcode)
{
    (*script)[com].arg1 = 0;
    (*script)[com].arg2 = 0;
    bool found_command=false;
    
    for(int i=0; i<NUMCOMMANDS&&!found_command; ++i)
    {
        if(strcmp(combuf,command_list[i].name)==0)
        {
            found_command=true;
            (*script)[com].command = i;
            
            if(((strnicmp(combuf,"GOTO",4)==0)||(strnicmp(combuf,"LOOP",4)==0)) && stricmp(combuf, "GOTOR"))
            {
                bool nomatch = true;
                
                for(int j=0; j<numlines; j++)
                {
                    if(stricmp(arg1buf,labels[j])==0)
                    {
                        (*script)[com].arg1 = lines[j];
                        nomatch = false;
                        j=numlines;
                    }
                }
                
                if(nomatch)
                {
                    (*script)[com].arg1 = atoi(arg1buf)-1;
                }
                
                if(strnicmp(combuf,"LOOP",4)==0)
                {
                    if(command_list[i].arg2_type==1)  //this should NEVER happen with a loop, as arg2 needs to be a variable
                    {
                        if(!ffcheck(arg2buf))
                        {
                            retcode=ERR_PARAM2;
                            return 0;
                        }
                        
                        (*script)[com].arg2 = ffparse(arg2buf);
                    }
                    else
                    {
                        if(!set_argument(arg2buf, script, com, 1))
                        {
                            retcode=ERR_PARAM2;
                            return 0;
                        }
                    }
                }
            }
            else
            {
                if(command_list[i].args>0)
                {
                    if(command_list[i].arg1_type==1)
                    {
                        if(!ffcheck(arg1buf))
                        {
                            retcode=ERR_PARAM1;
                            return 0;
                        }
                        
                        (*script)[com].arg1 = ffparse(arg1buf);
                    }
                    else
                    {
                        if(!set_argument(arg1buf, script, com, 0))
                        {
                            retcode=ERR_PARAM1;
                            return 0;
                        }
                    }
                    
                    if(command_list[i].args>1)
                    {
                        if(command_list[i].arg2_type==1)
                        {
                            if(!ffcheck(arg2buf))
                            {
                                retcode=ERR_PARAM2;
                                return 0;
                            }
                            
                            (*script)[com].arg2 = ffparse(arg2buf);
                        }
                        else
                        {
                            if(!set_argument(arg2buf, script, com, 1))
                            {
                                retcode=ERR_PARAM2;
                                return 0;
                            }
                        }
                    }
                }
            }
        }
    }
    
    if(found_command)
    {
        return 1;
    }
    
    retcode=ERR_INSTRUCTION;
    return 0;
}


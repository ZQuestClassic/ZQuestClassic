#ifndef _FFSCRIPT_H_
#define _FFSCRIPT_H_
#include "zdefs.h"

// Defines for script flags
#define TRUEFLAG          0x0001
#define MOREFLAG          0x0002
#define TRIGGERED         0x0004

#define fflong(x,y,z)       (((x[(y)][(z)])<<24)+((x[(y)][(z)+1])<<16)+((x[(y)][(z)+2])<<8)+(x[(y)][(z)+3]))
#define ffword(x,y,z)       (((x[(y)][(z)])<<8)+(x[(y)][(z)+1]))

// Defines for the ASM operations
enum
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
  CHECKTRIG,            //0x000C
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
  TRACENL,              //0x0040
  LOOP,                 //0x0041
  PUSHR,                //0x0042
  PUSHV,                //0x0043
  POP,                  //0x0044
  ENQUEUER,             //0x0045
  ENQUEUEV,             //0x0046
  DEQUEUE,              //0x0047
  PLAYSOUNDR,           //0x0048
  PLAYSOUNDV,           //0x0049
  LOADWEAPONR,          //0x004A
  LOADWEAPONV,          //0x004B
  LOADITEMR,            //0x004C //DEPRECATED
  LOADITEMV,            //0x004D //DEPRECATED
  LOADNPCR,             //0x004E
  LOADNPCV,             //0x004F
  CREATELWEAPONR,       //0x0050
  CREATELWEAPONV,       //0x0051
  CREATEITEMR,          //0x0052
  CREATEITEMV,          //0x0053
  CREATENPCR,           //0x0054
  CREATENPCV,           //0x0055
  LOADI,                //0x0056
  STOREI,               //0x0057
  GOTOR,                //0x0058
  SQROOTV,              //0x0059
  SQROOTR,              //0x005A
  CREATEEWEAPONR,       //0x005B
  CREATEEWEAPONV,       //0x005C
  PITWARP,              //0x005D
  WARPR,                //0x005E
  PITWARPR,             //0x005F
  CLEARSPRITESR,        //0x0060
  CLEARSPRITESV,        //0x0061
  RECTR,                //0x0062
  CIRCLER,              //0x0063
  ARCR,                 //0x0064
  ELLIPSER,             //0x0065
  LINER,                //0x0066
  PUTPIXELR,            //0x0067
  DRAWTILER,            //0x0068
  DRAWCOMBOR,           //0x0069
  ELLIPSE2,             //0x006A
  SPLINE,               //0x006B
  FLOODFILL,            //0x006C
  COMPOUNDR,            //0x006D
  COMPOUNDV,            //0x006E
  NUMCOMMANDS           //0x006F
};

#define D(n)            ((0x0000)+(n))
#define A(n)            ((0x0008)+(n))
#define DATA              0x000A
#define FCSET             0x000B
#define DELAY             0x000C
#define FX                0x000D
#define FY                0x000E
#define XD                0x000F
#define YD                0x0010
#define XD2               0x0011
#define YD2               0x0012
#define FLAG              0x0013
#define WIDTH             0x0014
#define HEIGHT            0x0015
#define LINK              0x0016
#define COMBOD(n)       ((0x0017)+((n)*3))
#define COMBOC(n)       ((0x0018)+((n)*3))
#define COMBOF(n)       ((0x0019)+((n)*3))

//0x00227-0X022F aren't used?
//n=0-175, so if n=175, then ((0x0019)+((n)*3))=0x00226
#define INPUTSTART        0x0227
#define INPUTUP           0x0228
#define INPUTDOWN         0x0229
#define INPUTLEFT         0x022A
#define INPUTRIGHT        0x022B
#define INPUTA            0x022C
#define INPUTB            0x022D
#define INPUTL            0x022E
#define INPUTR            0x022F

#define LINKX             0x0230
#define LINKY             0x0231
#define LINKDIR           0x0232
#define LINKHP            0x0233
#define LINKMP            0x0234
#define LINKMAXHP         0x0235
#define LINKMAXMP         0x0236
#define LINKACTION        0x0237
#define LINKITEMD         0x0238
#define LINKZ             0x0239
#define LINKJUMP          0x023A
#define LINKSWORDJINX     0x023B
#define LINKITEMJINX      0x023C
#define LINKCHARGED       0x023D
//0x023E-0x0258 are reserved for future Link variables
#define UNUSED14          0x023E
#define UNUSED15          0x023F
#define UNUSED16          0x0240
#define UNUSED17          0x0241
#define UNUSED18          0x0242
#define UNUSED19          0x0243
#define UNUSED20          0x0244
#define UNUSED21          0x0245
#define UNUSED22          0x0246
#define UNUSED23          0x0247
#define UNUSED24          0x0248
#define UNUSED25          0x0249
#define UNUSED26          0x024A
#define UNUSED27          0x024B
#define UNUSED28          0x024C
#define UNUSED29          0x024D
#define UNUSED30          0x024E
#define UNUSED31          0x024F
#define UNUSED32          0x0250
#define UNUSED33          0x0251
#define UNUSED34          0x0252
#define UNUSED35          0x0253
#define UNUSED36          0x0254
#define UNUSED37          0x0255
#define UNUSED38          0x0256
#define UNUSED39          0x0257
#define UNUSED40          0x0258

#define LWPNX             0x0259
#define LWPNY             0x025A
#define LWPNDIR           0x025B
#define LWPNSTEP          0x025C
#define LWPNANGULAR       0x025D
#define LWPNANGLE         0x025E
#define LWPNDRAWTYPE      0x025F
#define LWPNPOWER         0x0260
#define LWPNDEAD          0x0261
#define LWPNID            0x0262
#define LWPNTILE          0x0263
#define LWPNCSET          0x0264
#define LWPNFLASHCSET     0x0265
#define LWPNFRAMES        0x0266
#define LWPNFRAME         0x0267
#define LWPNASPEED        0x0268
#define LWPNFLASH         0x0269
#define LWPNFLIP          0x026A
#define LWPNCOUNT         0x026B
#define LWPNEXTEND        0x026C
#define LWPNOTILE         0x026D
#define LWPNOCSET         0x026E
#define LWPNZ             0x026F
#define LWPNJUMP           0x0270
//0x0271-0x028B are reserved for future weapon variables
#define UNUSED46          0x0271
#define UNUSED47          0x0272
#define UNUSED48          0x0273
#define UNUSED49          0x0274
#define UNUSED50          0x0275
#define UNUSED51          0x0276
#define UNUSED52          0x0277
#define UNUSED53          0x0278
#define UNUSED54          0x0279
#define UNUSED55          0x027A
#define UNUSED56          0x027B
#define UNUSED57          0x027C
#define UNUSED58          0x027D
#define UNUSED59          0x027E
#define UNUSED60          0x027F
#define UNUSED61          0x0280
#define UNUSED62          0x0281
#define UNUSED63          0x0282
#define UNUSED64          0x0283
#define UNUSED65          0x0284
#define UNUSED66          0x0285
#define UNUSED67          0x0286
#define UNUSED68          0x0287
#define UNUSED69          0x0288
#define UNUSED70          0x0289
#define UNUSED71          0x028A
#define UNUSED72          0x028B

#define ITEMX             0x028C
#define ITEMY             0x028D
#define ITEMDRAWTYPE      0x028E
#define ITEMID            0x028F
#define ITEMTILE          0x0290
#define ITEMCSET          0x0291
#define ITEMFLASHCSET     0x0292
#define ITEMFRAMES        0x0293
#define ITEMFRAME         0x0294
#define ITEMASPEED        0x0295
#define ITEMDELAY         0x0296
#define ITEMFLASH         0x0297
#define ITEMFLIP          0x0298
#define ITEMCOUNT         0x0299
#define ICLASSFAMILY      0x029A
#define ICLASSFAMTYPE     0x029B
#define ICLASSSETGAME     0x029C
#define ICLASSAMOUNT      0x029D
#define ICLASSSETMAX      0x029E
#define ICLASSMAX         0x029F
#define ICLASSCOUNTER     0x02A0
#define ITEMEXTEND        0x02A1
#define ITEMZ             0x02A2
#define ITEMJUMP          0x02A3

//0x02A4-0x02B9 are reserved for future item variables
#define UNUSED83          0x02A4
#define UNUSED84          0x02A5
#define UNUSED85          0x02A6
#define UNUSED86          0x02A7
#define UNUSED87          0x02A8
#define UNUSED88          0x02A9
#define UNUSED89          0x02AA
#define UNUSED90          0x02AB
#define UNUSED91          0x02AC
#define UNUSED92          0x02AD
#define UNUSED93          0x02AE
#define UNUSED94          0x02AF
#define UNUSED95          0x02B0
#define UNUSED96          0x02B1
#define UNUSED97          0x02B2
#define UNUSED98          0x02B3
#define UNUSED99          0x02B4
#define UNUSED100         0x02B5
#define UNUSED101         0x02B6
#define UNUSED102         0x02B7
#define UNUSED103         0x02B8
#define UNUSED104         0x02B9

#define NPCX              0x02BA
#define NPCY              0x02BB
#define NPCDIR            0x02BC
#define NPCRATE           0x02BD
#define NPCFRAMERATE      0x02BE
#define NPCHALTRATE       0x02BF
#define NPCDRAWTYPE       0x02C0
#define NPCHP             0x02C1
#define NPCID             0x02C2
#define NPCDP             0x02C3
#define NPCWDP            0x02C4
#define NPCTILE           0x02C5
#define NPCENEMY          0x02C6
#define NPCWEAPON         0x02C7
#define NPCITEMSET        0x02C8
#define NPCCSET           0x02C9
#define NPCBOSSPAL        0x02CA
#define NPCBGSFX          0x02CB
#define NPCCOUNT          0x02CC
#define NPCEXTEND         0x02CD
#define NPCZ              0x02CE
#define NPCJUMP           0x02CF
//0x02D0-0x02EB are reserved for future NPC variables
#define UNUSED108         0x02D0
#define UNUSED109         0x02D1
#define UNUSED110         0x02D2
#define UNUSED111         0x02D3
#define UNUSED112         0x02D4
#define UNUSED113         0x02D5
#define UNUSED114         0x02D6
#define UNUSED115         0x02D7
#define UNUSED116         0x02D8
#define UNUSED117         0x02D9
#define UNUSED118         0x02DA
#define UNUSED119         0x02DB
#define UNUSED120         0x02DC
#define UNUSED121         0x02DD
#define UNUSED122         0x02DE
#define UNUSED123         0x02DF
#define UNUSED124         0x02E0
#define UNUSED125         0x02E1
#define UNUSED126         0x02E2
#define UNUSED127         0x02E3
#define UNUSED128         0x02E4
#define UNUSED129         0x02E5
#define UNUSED130         0x02E6
#define UNUSED131         0x02E7
#define UNUSED132         0x02E8
#define UNUSED133         0x02E9
#define UNUSED134         0x02EA
#define UNUSED135         0x02EB
//I'm stealing this one for the stack pointer -DD
#define SP                0x02EC

#define SD(n)           ((0x02ED)+(n))
#define GD(n)           ((0x02F5)+(n))

// Defines for accessing gamedata
#define GAMECOUNTER(n)  ((0x03F5)+((n)*3))
#define GAMEMCOUNTER(n) ((0x03F6)+((n)*3))
#define GAMEDCOUNTER(n) ((0x03F7)+((n)*3))
#define GAMEGENERIC(n)	((0x0455)+(n))
#define GAMEITEMS(n)    ((0x0555)+(n))
#define GAMELITEMS(n)   ((0x0655)+(n))
#define GAMELKEYS(n)    ((0x0755)+(n))
#define GAMEDEATHS        0x0855
#define GAMECHEAT         0x0856
#define GAMETIME          0x0857
#define GAMEHASPLAYED     0x0858
#define GAMETIMEVALID     0x0859
#define GAMEGUYCOUNTD     0x085A
#define GAMEGUYCOUNT      0x085B
#define GAMECONTSCR       0x085C
#define GAMECONTDMAP      0x085D
#define GAMECOUNTERD      0x085E
#define GAMEMCOUNTERD     0x085F
#define GAMEDCOUNTERD     0x0860
#define GAMEGENERICD      0x0861
#define GAMEITEMSD        0x0862
#define GAMELITEMSD       0x0863
#define GAMELKEYSD        0x0864
#define GAMEMAPFLAG(n)  ((0x0865)+(n))
#define GAMEMAPFLAGD      0x0885

#define CURSCR            0x0886
#define CURMAP            0x0887
#define CURDMAP           0x0888
#define COMBODD	          0x0889
#define COMBOCD           0x088A
#define COMBOFD           0x088B
#define COMBOTD           0x088C
#define COMBOID           0x088D

#define REFITEMCLASS      0x088E
#define REFITEM           0x088F
#define REFFFC            0x0890
#define REFLWPN           0x0891
#define REFNPC            0x0892
#define REFLWPNCLASS      0x0893
#define REFNPCCLASS       0x0894
#define REFSUBSCREEN      0x0895
#define REFEWPN           0x0896
#define REFEWPNCLASS      0x0897

#define EWPNX             0x0898
#define EWPNY             0x0899
#define EWPNDIR           0x089A
#define EWPNSTEP          0x089B
#define EWPNANGULAR       0x089C
#define EWPNANGLE         0x089D
#define EWPNDRAWTYPE      0x089E
#define EWPNPOWER         0x089F
#define EWPNDEAD          0x08A0
#define EWPNID            0x08A1
#define EWPNTILE          0x08A2
#define EWPNCSET          0x08A3
#define EWPNFLASHCSET     0x08A4
#define EWPNFRAMES        0x08A5
#define EWPNFRAME         0x08A6
#define EWPNASPEED        0x08A7
#define EWPNFLASH         0x08A8
#define EWPNFLIP          0x08A9
#define EWPNCOUNT         0x08AA
#define EWPNEXTEND        0x08AB
#define EWPNOTILE         0x08AC
#define EWPNOCSET         0x08AD
#define EWPNZ             0x08AE
#define EWPNJUMP          0x08AF

//#define GETA              0x08AE
//#define GETB              0x08AF
//#define GETL              0x08B0
//#define GETR              0x08B1
//#define GETLEFT           0x08B2
//#define GETRIGHT          0x08B3
//#define GETUP             0x08B4
//#define GETDOWN           0x08B5
//#define GETSTART          0x08B6

#define COMBOSD	             0x08B7
#define GAMEMAPFLAGDD        0x08B8
#define SDD                  0x08B9
#define SDDD                 0x08BA
#define GDD                  0x08BB

#define FFFLAGSD             0x08BC
#define FFCWIDTH             0x08BD
#define FFCHEIGHT            0x08BE
#define FFTWIDTH             0x08BF
#define FFTHEIGHT            0x08C0
#define FFLINK               0x08C1

#define GS(n)              ((0x08C2)+(n))
#define NUMVARIABLES         0x09C1


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


typedef struct script_command
{
  char name[16];
  byte args;
  byte arg1_type; //0=reg, 1=val;
  byte arg2_type; //0=reg, 1=val;
  bool more_stuff;
} script_command;

typedef struct script_variable
{
  char name[16];
  long id;
  word maxcount;
  byte multiple;
} script_variable;

void do_set(int script, word *pc, byte i, bool v);
void do_add(int script, word *pc, byte i, bool v);
void do_sub(int script, word *pc, byte i, bool v);
void do_mult(int script, word *pc, byte i, bool v);
void do_div(int script, word *pc, byte i, bool v);
void do_comp(int script, word *pc, byte i, bool v);
void do_load(int script, word *pc, byte i, int a);
void do_seta(int script, word *pc, byte i, int a);
void do_trig(int script, word *pc, byte i, bool v, int type);
int run_script(int script, byte i, int stype);
int ffscript_engine(bool preload);
void write_stack(int script, byte i,int sp,long value);
int read_stack(int script, byte i,int sp);
#endif

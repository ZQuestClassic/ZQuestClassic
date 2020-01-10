#ifndef BYTECODE_H //2.53 Updated to 16th Jan, 2017
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
#define INDEX                   0
#define INDEX2                  1
#define EXP1                    2
#define EXP2                    3
#define SFRAME                  4
#define NUL                     5
#define SFTEMP                  6
#define WHAT_NO_7               7 // What, no 7?
#define WHAT_NO_8               8 // What, no 8?
#define ZELDAVERSION               9 // What, no 9?
#define SP                     10
#define DATA                   11
#define FCSET                  12
#define DELAY                  13
#define FX                     14
#define FY                     15
#define XD                     16
#define YD                     17
#define XD2                    18
#define YD2                    19
#define LINKX                  20
#define LINKY                  21
#define LINKDIR                22
#define LINKHP                 23
#define LINKMP                 24
#define LINKMAXHP              25
#define LINKMAXMP              26
#define LINKACTION             27
#define INPUTSTART             28
#define INPUTUP                29
#define INPUTDOWN              30
#define INPUTLEFT              31
#define INPUTRIGHT             32
#define INPUTA                 33
#define INPUTB                 34
#define INPUTL                 35
#define INPUTR                 36
#define SDD                    37 //  8 of these
#define COMBODD                38 // 176 of these
#define COMBOCD                39 // 176 of these
#define COMBOFD                40 // 176 of these
#define REFFFC                 41
#define REFITEM                42
#define ITEMCOUNT              43
#define ITEMX                  44
#define ITEMY                  45
#define ITEMDRAWTYPE           46
#define ITEMID                 47
#define ITEMTILE               48
#define ITEMCSET               49
#define ITEMFLASHCSET          50
#define ITEMFRAMES             51
#define ITEMFRAME              52
#define ITEMASPEED             53
#define ITEMDELAY              54
#define ITEMFLASH              55
#define ITEMFLIP               56
#define ITEMEXTEND             57
#define ITEMCLASSFAMILY        58
#define ITEMCLASSFAMTYPE       59
#define ITEMCLASSAMOUNT        60
#define ITEMCLASSMAX           61
#define ITEMCLASSSETMAX        62
#define ITEMCLASSSETGAME       63
#define ITEMCLASSCOUNTER       64
#define REFITEMCLASS           65
#define LINKZ                  66
#define LINKJUMP               67
#define ITEMZ                  68
#define ITEMJUMP               69
#define NPCZ                   70
#define NPCJUMP                71
#define WHAT_NO_72             72 // What, no 72?
#define LINKSWORDJINX          73
#define LINKITEMJINX           74
#define COMBOID                75
#define COMBOTD                76
#define COMBOSD                77
#define CURSCR                 78
#define CURMAP                 79
#define CURDMAP                80
#define GAMEDEATHS             81
#define GAMECHEAT              82
#define GAMETIME               83
#define GAMEHASPLAYED          84
#define GAMETIMEVALID          85
#define GAMEGUYCOUNT           86
#define GAMECONTSCR            87
#define GAMECONTDMAP           88
#define GAMECOUNTERD           89
#define GAMEMCOUNTERD          90
#define GAMEDCOUNTERD          91
#define GAMEGENERICD           92
#define GAMEITEMSD             93
#define GAMELITEMSD            94
#define GAMELKEYSD             95
#define SCREENSTATED           96
#define SCREENSTATEDD          97
#define SDDD                   98
#define FFFLAGSD               99
#define FFTWIDTH              100
#define FFTHEIGHT             101
#define FFCWIDTH              102
#define FFCHEIGHT             103
#define FFLINK                104
#define LINKITEMD             105
#define REFNPC                106
#define NPCCOUNT              107
#define NPCX                  108
#define NPCY                  109
#define NPCDIR                110
#define NPCRATE               111
#define NPCFRAMERATE          112
#define NPCHALTRATE           113
#define NPCDRAWTYPE           114
#define NPCHP                 115
#define NPCDP                 116
#define NPCWDP                117
#define NPCOTILE              118
#define NPCWEAPON             119
#define NPCITEMSET            120
#define NPCCSET               121
#define NPCBOSSPAL            122
#define NPCBGSFX              123
#define NPCEXTEND             124
#define SCRDOORD              125
#define CURDSCR               126
#define LINKHELD              127
#define NPCSTEP               128
#define ITEMOTILE             129
#define INPUTMOUSEX           130
#define INPUTMOUSEY           131
#define QUAKE                 132
#define WAVY                  133
#define NPCID                 134
#define ITEMCLASSUSESOUND     135
#define INPUTMOUSEZ           136
#define INPUTMOUSEB           137
#define REFLWPN               138
#define LWPNCOUNT             139
#define LWPNX                 140
#define LWPNY                 141
#define LWPNDIR               142
#define LWPNSTEP              143
#define LWPNANGULAR           144
#define LWPNANGLE             145
#define LWPNDRAWTYPE          146
#define LWPNPOWER             147
#define LWPNDEAD              148
#define LWPNID                149
#define LWPNTILE              150
#define LWPNCSET              151
#define LWPNFLASHCSET         152
#define LWPNFRAMES            153
#define LWPNFRAME             154
#define LWPNASPEED            155
#define LWPNFLASH             156
#define LWPNFLIP              157
#define LWPNEXTEND            158
#define LWPNOTILE             159
#define LWPNOCSET             160
#define LWPNZ                 161
#define LWPNJUMP              162
#define REFEWPN               163
#define EWPNX                 164
#define EWPNY                 165
#define EWPNDIR               166
#define EWPNSTEP              167
#define EWPNANGULAR           168
#define EWPNANGLE             169
#define EWPNDRAWTYPE          170
#define EWPNPOWER             171
#define EWPNDEAD              172
#define EWPNID                173
#define EWPNTILE              174
#define EWPNCSET              175
#define EWPNFLASHCSET         176
#define EWPNFRAMES            177
#define EWPNFRAME             178
#define EWPNASPEED            179
#define EWPNFLASH             180
#define EWPNFLIP              181
#define EWPNCOUNT             182
#define EWPNEXTEND            183
#define EWPNOTILE             184
#define EWPNOCSET             185
#define EWPNZ                 186
#define EWPNJUMP              187
#define COMBODDM              188 // 176 of these
#define COMBOCDM              189 // 176 of these
#define COMBOFDM              190 // 176 of these
#define COMBOIDM              191 // 176 of these
#define COMBOTDM              192 // 176 of these
#define COMBOSDM              193 // 176 of these
#define SCRIPTRAM			  194
#define GLOBALRAM			  195
#define SCRIPTRAMD			  196
#define GLOBALRAMD			  197
#define WHAT_NO_198		      198 //What, no 198?
#define LWPNHXOFS			  199
#define LWPNHYOFS			  200
#define LWPNXOFS			  201
#define LWPNYOFS			  202
#define LWPNZOFS			  203
#define LWPNHXSZ			  204
#define LWPNHYSZ			  205
#define LWPNHZSZ			  206
#define EWPNHXOFS			  207
#define EWPNHYOFS			  208
#define EWPNXOFS			  209
#define EWPNYOFS			  210
#define EWPNZOFS			  211
#define EWPNHXSZ			  212
#define EWPNHYSZ			  213
#define EWPNHZSZ			  214
#define NPCHXOFS			  215
#define NPCHYOFS			  216
#define NPCXOFS				  217
#define NPCYOFS				  218
#define NPCZOFS				  219
#define NPCHXSZ			      220
#define NPCHYSZ			      221
#define NPCHZSZ			      222
#define ITEMHXOFS			  223
#define ITEMHYOFS			  224
#define ITEMXOFS			  225
#define ITEMYOFS			  226
#define ITEMZOFS			  227
#define ITEMHXSZ			  228
#define ITEMHYSZ			  229
#define ITEMHZSZ			  230
#define LWPNTXSZ			  231
#define LWPNTYSZ			  232
#define EWPNTXSZ			  233
#define EWPNTYSZ			  234
#define NPCTXSZ				  235
#define NPCTYSZ				  236
#define ITEMTXSZ			  237
#define ITEMTYSZ			  238
#define LINKHXOFS			  239
#define LINKHYOFS			  240
#define LINKXOFS			  241
#define LINKYOFS			  242
#define LINKZOFS			  243
#define LINKHXSZ			  244
#define LINKHYSZ			  245
#define LINKHZSZ			  246
#define LINKTXSZ			  247
#define LINKTYSZ			  248
#define LINKDRUNK			  249
#define NPCTILE		 	 	  250
#define LWPNBEHIND            251
#define EWPNBEHIND            252
#define SDDDD                 253
#define CURLEVEL              254
#define ITEMPICKUP	      255
#define INPUTMAP              256
#define LIT                   257
#define INPUTEX1             258
#define INPUTEX2             259
#define INPUTEX3             260
#define INPUTEX4             261
#define INPUTPRESSSTART      262
#define INPUTPRESSUP         263
#define INPUTPRESSDOWN       264
#define INPUTPRESSLEFT       265
#define INPUTPRESSRIGHT      266
#define INPUTPRESSA          267
#define INPUTPRESSB          268
#define INPUTPRESSL          269
#define INPUTPRESSR          270
#define INPUTPRESSEX1        271
#define INPUTPRESSEX2        272
#define INPUTPRESSEX3        273
#define INPUTPRESSEX4        274
#define LWPNMISCD            275 //16 of these
#define EWPNMISCD            276 //16 of these
#define NPCMISCD             277 //16 of these
#define ITEMMISCD            278 //16 of these
#define FFMISCD              279 //16 of these
#define GETMIDI              280
#define NPCHOMING            281
#define NPCDD                282 //10 of these
#define LINKEQUIP            283

#define INPUTAXISUP			284
#define INPUTAXISDOWN		285
#define INPUTAXISLEFT		286
#define INPUTAXISRIGHT		287
#define INPUTPRESSAXISUP    288
#define INPUTPRESSAXISDOWN  289
#define INPUTPRESSAXISLEFT  290
#define INPUTPRESSAXISRIGHT 291

#define NPCTYPE			    292
#define FFSCRIPT			293
#define SCREENFLAGSD		294 //10 of these
#define LINKINVIS			295
#define LINKINVINC			296
#define SCREENEFLAGSD		297 //3 of these
#define NPCMFLAGS			298
#define FFINITDD			299 //8 of these
#define LINKMISCD           300 //16 of these
#define DMAPFLAGSD			301 //2 of these
#define LWPNCOLLDET			302
#define EWPNCOLLDET			303
#define NPCCOLLDET			304
#define LINKLADDERX		    305
#define LINKLADDERY		    306
#define NPCSTUN			    307
#define NPCDEFENSED		    308
#define ITEMCLASSPOWER      309
#define DMAPLEVELD			310 //512 of these
#define DMAPCOMPASSD		311 //512 of these
#define DMAPCONTINUED		312 //512 of these
#define DMAPMIDID			313 //512 of these
#define ITEMCLASSINITDD		314 //8 of these
#define LINKHITDIR			315
#define ROOMTYPE			316
#define ROOMDATA			317
#define LINKTILE			318
#define LINKFLIP			319
#define INPUTPRESSMAP		320
#define NPCHUNGER			321
#define GAMESTANDALONE		322
#define GAMEENTRSCR			323
#define GAMEENTRDMAP		324
#define GAMECLICKFREEZE		325
#define PUSHBLOCKX			326
#define PUSHBLOCKY			327
#define PUSHBLOCKCOMBO		328
#define PUSHBLOCKCSET		329
#define UNDERCOMBO			330
#define UNDERCSET			331
#define DMAPOFFSET			332 //512 of these
#define DMAPMAP				333 //512 of these
//#define FFDD			    309 //8 of these
#define GETRENDERTARGET		1083 //From 2.55, ported to 2.53R3/2.53.1

class LiteralArgument;
class VarArgument;
class LabelArgument;
class GlobalArgument;

class ArgumentVisitor
{
public:
    virtual void caseLiteral(LiteralArgument &host, void *param)
    {
        void *temp;
        temp=&host;
        param=param; /*these are here to bypass compiler warnings about unused arguments*/
    }
    virtual void caseVar(VarArgument &host, void *param)
    {
        void *temp;
        temp=&host;
        param=param; /*these are here to bypass compiler warnings about unused arguments*/
    }
    virtual void caseLabel(LabelArgument &host, void *param)
    {
        void *temp;
        temp=&host;
        param=param; /*these are here to bypass compiler warnings about unused arguments*/
    }
    virtual void caseGlobal(GlobalArgument &host, void *param)
    {
        void *temp;
        temp=&host;
        param=param; /*these are here to bypass compiler warnings about unused arguments*/
    }
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
    Argument *clone()
    {
        return new LiteralArgument(value);
    }
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
    Argument *clone()
    {
        return new VarArgument(ID);
    }
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
    Argument *clone()
    {
        return new GlobalArgument(ID);
    }
private:
    int ID;
};

class LabelArgument : public Argument
{
public:
    LabelArgument(int id) : ID(id), haslineno(false) {}
    string toString();
    string toStringSetV();
    void execute(ArgumentVisitor &host, void *param)
    {
        host.caseLabel(*this,param);
    }
    Argument *clone()
    {
        return new LabelArgument(ID);
    }
    int getID()
    {
        return ID;
    }
    void setLineNo(int l)
    {
        haslineno=true;
        lineno=l;
    }
private:
    int ID;
    int lineno;
    bool haslineno;
};

class UnaryOpcode : public Opcode
{
public:
    UnaryOpcode(Argument *A) : a(A) {}
    ~UnaryOpcode()
    {
        delete a;
    }
    Argument *getArgument()
    {
        return a;
    }
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
    ~BinaryOpcode()
    {
        delete a;
        delete b;
    }
    Argument *getFirstArgument()
    {
        return a;
    }
    Argument *getSecondArgument()
    {
        return b;
    }
    void execute(ArgumentVisitor &host, void *param)
    {
        a->execute(host, param);
        b->execute(host, param);
    }
protected:
    Argument *a;
    Argument *b;
};

class OSetTrue : public UnaryOpcode
{
public:
    OSetTrue(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OSetTrue(a->clone());
    }
};

class OSetFalse : public UnaryOpcode
{
public:
    OSetFalse(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OSetFalse(a->clone());
    }
};

class OSetMore : public UnaryOpcode
{
public:
    OSetMore(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OSetMore(a->clone());
    }
};

class OSetLess : public UnaryOpcode
{
public:
    OSetLess(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OSetLess(a->clone());
    }
};

class OSetImmediate : public BinaryOpcode
{
public:
    OSetImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OSetImmediate(a->clone(),b->clone());
    }
};

class OSetRegister : public BinaryOpcode
{
public:
    OSetRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OSetRegister(a->clone(),b->clone());
    }
};

class OAddImmediate : public BinaryOpcode
{
public:
    OAddImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OAddImmediate(a->clone(),b->clone());
    }
};

class OAddRegister : public BinaryOpcode
{
public:
    OAddRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OAddRegister(a->clone(),b->clone());
    }
};

class OSubImmediate : public BinaryOpcode
{
public:
    OSubImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OSubImmediate(a->clone(),b->clone());
    }
};

class OSubRegister : public BinaryOpcode
{
public:
    OSubRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OSubRegister(a->clone(),b->clone());
    }
};

class OMultImmediate : public BinaryOpcode
{
public:
    OMultImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OMultImmediate(a->clone(),b->clone());
    }
};

class OMultRegister : public BinaryOpcode
{
public:
    OMultRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OMultRegister(a->clone(),b->clone());
    }
};

class ODivImmediate : public BinaryOpcode
{
public:
    ODivImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new ODivImmediate(a->clone(),b->clone());
    }
};

class ODivRegister : public BinaryOpcode
{
public:
    ODivRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new ODivRegister(a->clone(),b->clone());
    }
};

class OCompareImmediate : public BinaryOpcode
{
public:
    OCompareImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OCompareImmediate(a->clone(),b->clone());
    }
};

class OCompareRegister : public BinaryOpcode
{
public:
    OCompareRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OCompareRegister(a->clone(),b->clone());
    }
};

class OWaitframe : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OWaitframe();
    }
};

class OWaitdraw : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OWaitdraw();
    }
};

class OGotoImmediate : public UnaryOpcode
{
public:
    OGotoImmediate(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGotoImmediate(a->clone());
    }
};

class OGotoTrueImmediate: public UnaryOpcode
{
public:
    OGotoTrueImmediate(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGotoTrueImmediate(a->clone());
    }
};

class OGotoFalseImmediate: public UnaryOpcode
{
public:
    OGotoFalseImmediate(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGotoFalseImmediate(a->clone());
    }
};

class OGotoMoreImmediate : public UnaryOpcode
{
public:
    OGotoMoreImmediate(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGotoMoreImmediate(a->clone());
    }
};

class OGotoLessImmediate : public UnaryOpcode
{
public:
    OGotoLessImmediate(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGotoLessImmediate(a->clone());
    }
};

class OPushRegister : public UnaryOpcode
{
public:
    OPushRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OPushRegister(a->clone());
    }
};

class OPushImmediate : public UnaryOpcode
	{
	public:
		OPushImmediate(Argument *A) : UnaryOpcode(A) {}
		std::string toString();
		Opcode *clone()
		{
			return new OPushImmediate(a->clone());
		}
	};

class OPopRegister : public UnaryOpcode
{
public:
    OPopRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OPopRegister(a->clone());
    }
};

class OLoadIndirect : public BinaryOpcode
{
public:
    OLoadIndirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OLoadIndirect(a->clone(),b->clone());
    }
};

class OStoreIndirect : public BinaryOpcode
{
public:
    OStoreIndirect(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OStoreIndirect(a->clone(),b->clone());
    }
};

class OQuit : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OQuit();
    }
};

class OGotoRegister : public UnaryOpcode
{
public:
    OGotoRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGotoRegister(a->clone());
    }
};

class OTraceRegister : public UnaryOpcode
{
public:
    OTraceRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OTraceRegister(a->clone());
    }
};

class OTrace2Register : public UnaryOpcode
{
public:
    OTrace2Register(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OTrace2Register(a->clone());
    }
};

class OTrace3 : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OTrace3();
    }
};

class OTrace4 : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OTrace4();
    }
};

class OTrace5Register : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OTrace5Register();
    }
};

class OTrace6Register : public UnaryOpcode
{
public:
    OTrace6Register(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OTrace6Register(a->clone());
    }
};

class OAndImmediate : public BinaryOpcode
{
public:
    OAndImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OAndImmediate(a->clone(),b->clone());
    }
};

class OAndRegister : public BinaryOpcode
{
public:
    OAndRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OAndRegister(a->clone(),b->clone());
    }
};

class OOrImmediate : public BinaryOpcode
{
public:
    OOrImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OOrImmediate(a->clone(),b->clone());
    }
};

class OOrRegister : public BinaryOpcode
{
public:
    OOrRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OOrRegister(a->clone(),b->clone());
    }
};

class OXorImmediate : public BinaryOpcode
{
public:
    OXorImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OXorImmediate(a->clone(), b->clone());
    }
};

class OXorRegister : public BinaryOpcode
{
public:
    OXorRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OXorRegister(a->clone(), b->clone());
    }
};

class ONot : public UnaryOpcode
{
public:
    ONot(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new ONot(a->clone());
    }
};

class OLShiftImmediate : public BinaryOpcode
{
public:
    OLShiftImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OLShiftImmediate(a->clone(), b->clone());
    }
};

class OLShiftRegister : public BinaryOpcode
{
public:
    OLShiftRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OLShiftRegister(a->clone(), b->clone());
    }
};

class ORShiftImmediate : public BinaryOpcode
{
public:
    ORShiftImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new ORShiftImmediate(a->clone(), b->clone());
    }
};

class ORShiftRegister : public BinaryOpcode
{
public:
    ORShiftRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new ORShiftRegister(a->clone(), b->clone());
    }
};

class OModuloImmediate : public BinaryOpcode
{
public:
    OModuloImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OModuloImmediate(a->clone(), b->clone());
    }
};

class OModuloRegister : public BinaryOpcode
{
public:
    OModuloRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OModuloRegister(a->clone(), b->clone());
    }
};

class OSinRegister : public BinaryOpcode
{
public:
    OSinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OSinRegister(a->clone(), b->clone());
    }
};

class OArcSinRegister : public BinaryOpcode
{
public:
    OArcSinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OArcSinRegister(a->clone(), b->clone());
    }
};

class OCosRegister : public BinaryOpcode
{
public:
    OCosRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OCosRegister(a->clone(), b->clone());
    }
};

class OArcCosRegister : public BinaryOpcode
{
public:
    OArcCosRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OArcCosRegister(a->clone(), b->clone());
    }
};

class OTanRegister : public BinaryOpcode
{
public:
    OTanRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OTanRegister(a->clone(), b->clone());
    }
};

class OATanRegister : public UnaryOpcode
{
public:
    OATanRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OATanRegister(a->clone());
    }
};

class OMaxRegister : public BinaryOpcode
{
public:
    OMaxRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OMaxRegister(a->clone(), b->clone());
    }
};

class OMinRegister : public BinaryOpcode
{
public:
    OMinRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OMinRegister(a->clone(), b->clone());
    }
};

class OPowRegister : public BinaryOpcode
{
public:
    OPowRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OPowRegister(a->clone(), b->clone());
    }
};

class OInvPowRegister : public BinaryOpcode
{
public:
    OInvPowRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OInvPowRegister(a->clone(), b->clone());
    }
};

class OFactorial : public UnaryOpcode
{
public:
    OFactorial(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OFactorial(a->clone());
    }
};

class OAbsRegister : public UnaryOpcode
{
public:
    OAbsRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OAbsRegister(a->clone());
    }
};

class OLog10Register : public UnaryOpcode
{
public:
    OLog10Register(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OLog10Register(a->clone());
    }
};

class OLogERegister : public UnaryOpcode
{
public:
    OLogERegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OLogERegister(a->clone());
    }
};

class OArraySize : public UnaryOpcode
{
public:
    OArraySize(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OArraySize(a->clone());
    }
};

class OCheckTrig : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OCheckTrig();
    }
};

class ORandRegister : public BinaryOpcode
{
public:
    ORandRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new ORandRegister(a->clone(), b->clone());
    }
};

class OSqrtRegister : public BinaryOpcode
{
public:
    OSqrtRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OSqrtRegister(a->clone(),b->clone());
    }
};

class OCalcSplineRegister : public BinaryOpcode
{
public:
    OCalcSplineRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OCalcSplineRegister(a->clone(),b->clone());
    }
};

class OSetColorRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OSetColorRegister();
    }
};

class OSetDepthRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OSetDepthRegister();
    }
};

class OCollisionRectRegister : public UnaryOpcode
{
public:
    OCollisionRectRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OCollisionRectRegister(a->clone());
    }
};

class OCollisionBoxRegister : public UnaryOpcode
{
public:
    OCollisionBoxRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OCollisionBoxRegister(a->clone());
    }
};

class OWarp : public BinaryOpcode
{
public:
    OWarp(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OWarp(a->clone(), b->clone());
    }
};

class OPitWarp : public BinaryOpcode
{
public:
    OPitWarp(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OPitWarp(a->clone(), b->clone());
    }
};

class OCreateItemRegister : public UnaryOpcode
{
public:
    OCreateItemRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OCreateItemRegister(a->clone());
    }
};

class OCreateNPCRegister : public UnaryOpcode
{
public:
    OCreateNPCRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OCreateNPCRegister(a->clone());
    }
};

class OCreateLWpnRegister : public UnaryOpcode
{
public:
    OCreateLWpnRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OCreateLWpnRegister(a->clone());
    }
};

class OCreateEWpnRegister : public UnaryOpcode
{
public:
    OCreateEWpnRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OCreateEWpnRegister(a->clone());
    }
};

class OLoadItemRegister : public UnaryOpcode
{
public:
    OLoadItemRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OLoadItemRegister(a->clone());
    }
};

class OLoadItemDataRegister : public UnaryOpcode
{
public:
    OLoadItemDataRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OLoadItemDataRegister(a->clone());
    }
};

class OLoadNPCRegister : public UnaryOpcode
{
public:
    OLoadNPCRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OLoadNPCRegister(a->clone());
    }
};

class OLoadLWpnRegister : public UnaryOpcode
{
public:
    OLoadLWpnRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OLoadLWpnRegister(a->clone());
    }
};

class OLoadEWpnRegister : public UnaryOpcode
{
public:
    OLoadEWpnRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OLoadEWpnRegister(a->clone());
    }
};

class OPlaySoundRegister : public UnaryOpcode
{
public:
    OPlaySoundRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OPlaySoundRegister(a->clone());
    }
};

class OPlayMIDIRegister : public UnaryOpcode
{
public:
    OPlayMIDIRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OPlayMIDIRegister(a->clone());
    }
};

class OPlayEnhancedMusic : public BinaryOpcode
{
public:
    OPlayEnhancedMusic(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OPlayEnhancedMusic(a->clone(), b->clone());
    }
};

class OGetDMapMusicFilename : public BinaryOpcode
{
public:
    OGetDMapMusicFilename(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OGetDMapMusicFilename(a->clone(), b->clone());
    }
};

class OGetDMapMusicTrack : public UnaryOpcode
{
public:
    OGetDMapMusicTrack(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetDMapMusicTrack(a->clone());
    }
};

class OSetDMapEnhancedMusic : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OSetDMapEnhancedMusic();
    }
};

class OGetSaveName : public UnaryOpcode
{
public:
    OGetSaveName(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetSaveName(a->clone());
    }
};

class OGetDMapName : public BinaryOpcode
{
public:
    OGetDMapName(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OGetDMapName(a->clone(), b->clone());
    }
};

class OGetDMapIntro : public BinaryOpcode
{
public:
    OGetDMapIntro(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OGetDMapIntro(a->clone(), b->clone());
    }
};

class OGetDMapTitle : public BinaryOpcode
{
public:
    OGetDMapTitle(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OGetDMapTitle(a->clone(), b->clone());
    }
};

class OSetSaveName : public UnaryOpcode
{
public:
    OSetSaveName(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OSetSaveName(a->clone());
    }
};

class OGetItemName : public UnaryOpcode
{
public:
    OGetItemName(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetItemName(a->clone());
    }
};

class OGetNPCName : public UnaryOpcode
{
public:
    OGetNPCName(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetNPCName(a->clone());
    }
};

class OGetMessage : public BinaryOpcode
{
public:
    OGetMessage(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OGetMessage(a->clone(), b->clone());
    }
};

class OClearSpritesRegister : public UnaryOpcode
{
public:
    OClearSpritesRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OClearSpritesRegister(a->clone());
    }
};

class OMessageRegister : public UnaryOpcode
{
public:
    OMessageRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OMessageRegister(a->clone());
    }
};

class OIsSolid : public UnaryOpcode
{
public:
    OIsSolid(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OIsSolid(a->clone());
    }
};

class OSetSideWarpRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OSetSideWarpRegister();
    }
};

class OSetTileWarpRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OSetTileWarpRegister();
    }
};

class OGetSideWarpDMap : public UnaryOpcode
{
public:
    OGetSideWarpDMap(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetSideWarpDMap(a->clone());
    }
};

class OGetSideWarpScreen : public UnaryOpcode
{
public:
    OGetSideWarpScreen(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetSideWarpScreen(a->clone());
    }
};

class OGetSideWarpType : public UnaryOpcode
{
public:
    OGetSideWarpType(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetSideWarpType(a->clone());
    }
};

class OGetTileWarpDMap : public UnaryOpcode
{
public:
    OGetTileWarpDMap(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetTileWarpDMap(a->clone());
    }
};

class OGetTileWarpScreen : public UnaryOpcode
{
public:
    OGetTileWarpScreen(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetTileWarpScreen(a->clone());
    }
};

class OGetTileWarpType : public UnaryOpcode
{
public:
    OGetTileWarpType(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetTileWarpType(a->clone());
    }
};

class OLayerScreenRegister : public BinaryOpcode
{
public:
    OLayerScreenRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OLayerScreenRegister(a->clone(), b->clone());
    }
};

class OLayerMapRegister : public BinaryOpcode
{
public:
    OLayerMapRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OLayerMapRegister(a->clone(), b->clone());
    }
};

class OTriggerSecrets : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OTriggerSecrets();
    }
};

class OIsValidItem : public UnaryOpcode
{
public:
    OIsValidItem(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OIsValidItem(a->clone());
    }
};

class OIsValidNPC : public UnaryOpcode
{
public:
    OIsValidNPC(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OIsValidNPC(a->clone());
    }
};

class OIsValidLWpn : public UnaryOpcode
{
public:
    OIsValidLWpn(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OIsValidLWpn(a->clone());
    }
};

class OIsValidEWpn : public UnaryOpcode
{
public:
    OIsValidEWpn(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OIsValidEWpn(a->clone());
    }
};

class OUseSpriteLWpn : public UnaryOpcode
{
public:
    OUseSpriteLWpn(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OUseSpriteLWpn(a->clone());
    }
};

class OUseSpriteEWpn : public UnaryOpcode
{
public:
    OUseSpriteEWpn(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OUseSpriteEWpn(a->clone());
    }
};

class ORectangleRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new ORectangleRegister();
    }
};

class OCircleRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OCircleRegister();
    }
};

class OArcRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OArcRegister();
    }
};

class OEllipseRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OEllipseRegister();
    }
};

class OLineRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OLineRegister();
    }
};

class OSplineRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OSplineRegister();
    }
};

class OPutPixelRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OPutPixelRegister();
    }
};

class ODrawCharRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new ODrawCharRegister();
    }
};

class ODrawIntRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new ODrawIntRegister();
    }
};

class ODrawTileRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new ODrawTileRegister();
    }
};

class ODrawComboRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new ODrawComboRegister();
    }
};

class OQuadRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OQuadRegister();
    }
};

class OTriangleRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OTriangleRegister();
    }
};

class OQuad3DRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OQuad3DRegister();
    }
};

class OTriangle3DRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OTriangle3DRegister();
    }
};

class OFastTileRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OFastTileRegister();
    }
};

class OFastComboRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OFastComboRegister();
    }
};

class ODrawStringRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new ODrawStringRegister();
    }
};

class ODrawLayerRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new ODrawLayerRegister();
    }
};

class ODrawScreenRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new ODrawScreenRegister();
    }
};

class ODrawBitmapRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new ODrawBitmapRegister();
    }
};

class OSetRenderTargetRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OSetRenderTargetRegister();
    }
};

class OSetDepthBufferRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OSetDepthBufferRegister();
    }
};

class OGetDepthBufferRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OGetDepthBufferRegister();
    }
};

class OSetColorBufferRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OSetColorBufferRegister();
    }
};

class OGetColorBufferRegister : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OGetColorBufferRegister();
    }
};

class OCopyTileRegister : public BinaryOpcode
{
public:
    OCopyTileRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OCopyTileRegister(a->clone(),b->clone());
    }
};

class OOverlayTileRegister : public BinaryOpcode
{
public:
    OOverlayTileRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OOverlayTileRegister(a->clone(),b->clone());
    }
};

class OSwapTileRegister : public BinaryOpcode
{
public:
    OSwapTileRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OSwapTileRegister(a->clone(),b->clone());
    }
};

class OClearTileRegister : public UnaryOpcode
{
public:
    OClearTileRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OClearTileRegister(a->clone());
    }
};

class OAllocateMemRegister : public BinaryOpcode
{
public:
    OAllocateMemRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OAllocateMemRegister(a->clone(),b->clone());
    }
};

class OAllocateMemImmediate : public BinaryOpcode
{
public:
    OAllocateMemImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OAllocateMemImmediate(a->clone(),b->clone());
    }
};

class OAllocateGlobalMemImmediate : public BinaryOpcode
{
public:
    OAllocateGlobalMemImmediate(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OAllocateGlobalMemImmediate(a->clone(),b->clone());
    }
};

class OAllocateGlobalMemRegister : public BinaryOpcode
{
public:
    OAllocateGlobalMemRegister(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OAllocateGlobalMemRegister(a->clone(),b->clone());
    }
};

class ODeallocateMemRegister : public UnaryOpcode
{
public:
    ODeallocateMemRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new ODeallocateMemRegister(a->clone());
    }
};

class ODeallocateMemImmediate : public UnaryOpcode
{
public:
    ODeallocateMemImmediate(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new ODeallocateMemImmediate(a->clone());
    }
};

class OSave : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OSave();
    }
};

class OGetScreenFlags : public UnaryOpcode
{
public:
    OGetScreenFlags(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetScreenFlags(a->clone());
    }
};

class OGetScreenEFlags : public UnaryOpcode
{
public:
    OGetScreenEFlags(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetScreenEFlags(a->clone());
    }
};

class OEnd : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OEnd();
    }
};

class OComboTile : public BinaryOpcode
{
public:
    OComboTile(Argument *A, Argument *B) : BinaryOpcode(A,B) {}
    string toString();
    Opcode *clone()
    {
        return new OComboTile(a->clone(), b->clone());
    }
};

class OBreakShield : public UnaryOpcode
{
public:
    OBreakShield(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OBreakShield(a->clone());
    }
};

class OShowSaveScreen : public UnaryOpcode
{
public:
    OShowSaveScreen(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OShowSaveScreen(a->clone());
    }
};

class OShowSaveQuitScreen : public Opcode
{
public:
    string toString();
    Opcode *clone()
    {
        return new OShowSaveQuitScreen();
    }
};

class OSelectAWeaponRegister : public UnaryOpcode
{
public:
    OSelectAWeaponRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OSelectAWeaponRegister(a->clone());
    }
};

class OSelectBWeaponRegister : public UnaryOpcode
{
public:
    OSelectBWeaponRegister(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OSelectBWeaponRegister(a->clone());
    }
};

class OGetFFCScript : public UnaryOpcode
{
public:
    OGetFFCScript(Argument *A) : UnaryOpcode(A) {}
    string toString();
    Opcode *clone()
    {
        return new OGetFFCScript(a->clone());
    }
};

class OIsValidArray : public UnaryOpcode
{
public:
	OIsValidArray(Argument *A) : UnaryOpcode(A) {}
	string toString();
	Opcode *clone()
	{
		return new OIsValidArray(a->clone());
	}
};

#endif


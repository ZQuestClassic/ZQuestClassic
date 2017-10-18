#include "aszc.h"
#include <ffc.h>
#include <guys.h>
#include <link.h>
#include <maps.h>
#include <sound.h>
#include <sprite.h>
#include <zc_alleg.h>
#include <zc_sys.h>
#include <zdefs.h>
#include <zelda.h>
#include "scriptData.h"
#include "util.h"
#include "scriptarray/scriptarray.h"
#include "scriptstdstring/scriptstdstring.h"
#include "scriptbuilder/scriptbuilder.h"
#include "scriptmath/scriptmath.h"
#include <angelscript.h>
#include <cstdio>
#include <cstdlib>

extern LinkClass Link;

// Global variables from link.cpp
extern int directWpn;

static sprite* currentSprite;
static enemy* currentEnemy;
static item* currentItem;
static weapon* currentWeapon;
static sprite* getRealSprite();
static enemy* getRealEnemy();
static item* getRealItem();
static weapon* getRealWeapon();
static LinkClass* getRealLink();
static void assignSpriteScript(sprite* spr, const char* scriptName);

static asIScriptEngine* asEngine=0;

static void messageCallback(const asSMessageInfo* msg, void* param)
{
    const char* type;
    switch(msg->type)
    {
    case asMSGTYPE_WARNING:
        type="Warning";
        break;
        
    case asMSGTYPE_INFORMATION:
        type="Info";
        break;
        
    case asMSGTYPE_ERROR:
    default:
        type="Error";
        break;
    }

	al_trace("%s: %s (%d, %d): %s\n", type, msg->section, msg->row, msg->col, msg->message);
}

static void fixCons(fix* addr)
{
    new(addr) fix(0);
}

static void fixConsCopy(fix* addr, const fix& other)
{
    new(addr) fix(other);
}

static void fixConsInt(fix* addr, int value)
{
    new(addr) fix(value);
}

static void fixConsFloat(fix* addr, float value)
{
    new(addr) fix(value);
}

static int fixComp(fix f1, fix f2)
{
    if(f1<f2)
        return -1;
    if(f1>f2)
        return 1;
    return 0;
}

static int fixV(fix& f)
{
    return f.v;
}

static void fixVAND(fix& f, unsigned int andVal)
{
    f.v&=andVal;
}

static fix fixSin(fix& f)
{
    // You'd think casting to double would be okay, but nope.
    return sin(f);
}

static void registerFix()
{
    asEngine->RegisterObjectType("fix", sizeof(fix), asOBJ_VALUE|asOBJ_POD|asOBJ_APP_CLASS_CAK|asOBJ_APP_CLASS_ALLINTS);
    
    // Constructors
    asEngine->RegisterObjectBehaviour("fix", asBEHAVE_CONSTRUCT, "void fix()", asFUNCTION(fixCons), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectBehaviour("fix", asBEHAVE_CONSTRUCT, "void fix(const fix &in)", asFUNCTION(fixConsCopy), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectBehaviour("fix", asBEHAVE_CONSTRUCT, "void fix(int)", asFUNCTION(fixConsInt), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectBehaviour("fix", asBEHAVE_CONSTRUCT, "void fix(float)", asFUNCTION(fixConsFloat), asCALL_CDECL_OBJFIRST);
    
    // Arithmetic
    asEngine->RegisterObjectMethod("fix", "fix opNeg() const", asMETHODPR(fix, operator-, () const, fix), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("fix", "fix opAdd(fix) const", asFUNCTIONPR(operator+, (fix, fix), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opAdd(int) const", asFUNCTIONPR(operator+, (fix, int), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opAdd(float) const", asFUNCTIONPR(operator+, (fix, float), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opSub(fix) const", asFUNCTIONPR(operator-, (fix, fix), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opSub(int) const", asFUNCTIONPR(operator-, (fix, int), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opSub(float) const", asFUNCTIONPR(operator-, (fix, float), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opMul(fix) const", asFUNCTIONPR(operator*, (fix, fix), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opMul(int) const", asFUNCTIONPR(operator*, (fix, int), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opMul(float) const", asFUNCTIONPR(operator*, (fix, float), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opDiv(fix) const", asFUNCTIONPR(operator/, (fix, fix), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opDiv(int) const", asFUNCTIONPR(operator/, (fix, int), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opDiv(float) const", asFUNCTIONPR(operator/, (fix, float), fix), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "fix opPostInc()", asMETHODPR(fix, operator++, (), fix&), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("fix", "fix opPostDec()", asMETHODPR(fix, operator--, (), fix&), asCALL_THISCALL);
    
    // Assignment
    asEngine->RegisterObjectMethod("fix", "fix& opAssign(fix)", asMETHODPR(fix, operator=, (const fix&), fix&), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("fix", "fix& opAssign(int)", asMETHODPR(fix, operator=, (const int), fix&), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("fix", "fix& opAssign(float)", asMETHODPR(fix, operator=, (const float), fix&), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("fix", "fix& opAddAssign(const fix &in)", asMETHODPR(fix, operator+=, (fix), fix&), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("fix", "fix& opSubAssign(const fix &in)", asMETHODPR(fix, operator-=, (fix), fix&), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("fix", "fix& opMulAssign(const fix &in)", asMETHODPR(fix, operator*=, (fix), fix&), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("fix", "fix& opDivAssign(const fix &in)", asMETHODPR(fix, operator/=, (fix), fix&), asCALL_THISCALL);
    
    // Allegro's comparison operators return int, not bool. Not sure if
    // this might be a problem on platforms where they're not the same size.
    asEngine->RegisterObjectMethod("fix", "bool opEquals(const fix) const", asFUNCTIONPR(operator==, (fix, fix), int), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "bool opEquals(const int) const", asFUNCTIONPR(operator==, (fix, int), int), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "bool opEquals(const float) const", asFUNCTIONPR(operator==, (fix, float), int), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "int opCmp(const fix) const", asFUNCTION(fixComp), asCALL_CDECL_OBJFIRST);
    
    // Casts
    asEngine->RegisterObjectMethod("fix", "int opImplConv() const", asMETHODPR(fix, operator int, () const, int), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("fix", "float opImplConv() const", asMETHODPR(fix, operator float, () const, float), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("fix", "double opImplConv() const", asMETHODPR(fix, operator double, () const, double), asCALL_THISCALL);
    
    // x.v
    asEngine->RegisterObjectMethod("fix", "int getV()", asFUNCTION(fixV), asCALL_CDECL_OBJFIRST);
    asEngine->RegisterObjectMethod("fix", "void vAND(uint)", asFUNCTION(fixVAND), asCALL_CDECL_OBJFIRST);
    
    // And more
    asEngine->RegisterObjectMethod("fix", "fix sin()", asFUNCTION(fixSin), asCALL_CDECL_OBJFIRST);
}

// All the classes are registered together first because their methods
// need to take each other as arguments.
static void registerTypes()
{
    asEngine->RegisterObjectType("__RealSprite", sizeof(sprite), asOBJ_REF|asOBJ_NOCOUNT);
    asEngine->RegisterGlobalFunction("__RealSprite@ __getRealSprite()", asFUNCTION(getRealSprite), asCALL_CDECL);
    asEngine->RegisterObjectType("__RealLink", sizeof(LinkClass), asOBJ_REF|asOBJ_NOCOUNT);
    asEngine->RegisterGlobalFunction("__RealLink@ __getRealLink()", asFUNCTION(getRealLink), asCALL_CDECL);
    asEngine->RegisterObjectType("__RealEnemy", sizeof(ASEnemy), asOBJ_REF|asOBJ_NOCOUNT);
    asEngine->RegisterGlobalFunction("__RealEnemy@ __getRealEnemy()", asFUNCTION(getRealEnemy), asCALL_CDECL);
    asEngine->RegisterObjectType("__RealItem", sizeof(item), asOBJ_REF|asOBJ_NOCOUNT);
    asEngine->RegisterGlobalFunction("__RealItem@ __getRealItem()", asFUNCTION(getRealItem), asCALL_CDECL);
    asEngine->RegisterObjectType("__RealWeapon", sizeof(weapon), asOBJ_REF|asOBJ_NOCOUNT);
    asEngine->RegisterGlobalFunction("__RealWeapon@ __getRealWeapon()", asFUNCTION(getRealWeapon), asCALL_CDECL);
    asEngine->RegisterObjectType("mapscr", sizeof(mapscr), asOBJ_REF|asOBJ_NOCOUNT);
    asEngine->RegisterObjectType("FFC", sizeof(FFC), asOBJ_REF|asOBJ_NOCOUNT);
    asEngine->RegisterInterface("spriteIF"); // Dummy interface for getters
}
void TraceI(int i) { al_trace("%d\n", i); }
void registerGlobalFunctions() // And properties... Whatever.
{
    asEngine->RegisterGlobalFunction("void TraceI(int)", asFUNCTION(TraceI), asCALL_CDECL);
    asEngine->RegisterGlobalProperty("const int frame", &frame);
    asEngine->RegisterGlobalProperty("const int db", &db);
    asEngine->RegisterGlobalProperty("const bool BSZ", &BSZ);
    asEngine->RegisterGlobalProperty("const int fadeclk", &fadeclk);
    asEngine->RegisterGlobalProperty("bool watch", &watch);
    asEngine->RegisterGlobalProperty("const int whistleclk", &whistleclk);
    asEngine->RegisterGlobalProperty("int wallm_load_clk", &wallm_load_clk);
    asEngine->RegisterGlobalProperty("int hasitem", &hasitem);
    asEngine->RegisterGlobalProperty("const int playing_field_offset", &playing_field_offset);
    asEngine->RegisterGlobalProperty("const int loadside", &loadside);
    asEngine->RegisterGlobalProperty("int guycarryingitem", &guycarryingitem);
    asEngine->RegisterGlobalProperty("int16 lensclk", &lensclk);
    asEngine->RegisterGlobalProperty("int currmap", &currmap);
    asEngine->RegisterGlobalProperty("int currscr", &currscr);
    asEngine->RegisterGlobalProperty("int dlevel", &dlevel);
    asEngine->RegisterGlobalProperty("int directWpn", &directWpn);
    asEngine->RegisterGlobalProperty("bool pull_link", &pull_link);
    asEngine->RegisterGlobalProperty("bool toogam", &toogam);
    
    asEngine->RegisterGlobalFunction("int rand()", asFUNCTION(rand), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("mapscr@ getTmpscr(int)", asFUNCTION(asGetTmpscr), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("mapscr@ getTmpscr2(int)", asFUNCTION(asGetTmpscr2), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("mapscr@ getTmpscr3(int)", asFUNCTION(asGetTmpscr3), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("int hit_enemy(int, int, int, int, int, int, int)", asFUNCTION(hit_enemy), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int getFFCAt(int, int)", asFUNCTION(getFFCAt), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void loadpalset(int, int)", asFUNCTION(loadpalset), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void music_stop()", asFUNCTION(music_stop), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void playLevelMusic()", asFUNCTION(playLevelMusic), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void setupscreen()", asFUNCTION(setupscreen), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool screenFlagSet(int)", asFUNCTION(asScreenFlagIsSet), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int getComboType(int)", asFUNCTION(asGetComboType), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int getSpriteTile(int)", asFUNCTION(getSpriteTile), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool questRuleEnabled(int)", asFUNCTION(asQuestRuleEnabled), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void sfx(int, int=128)", asFUNCTION(asSFX), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void stop_sfx(int)", asFUNCTION(asStopSFX), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool iswater(int)", asFUNCTION(asIsWater), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool _walkflag(int, int, int)", asFUNCTION(_walkflag), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int MAPCOMBO(int x, int y)", asFUNCTION(MAPCOMBO), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int MAPFLAG(int x, int y)", asFUNCTION(MAPFLAG), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int MAPCOMBOFLAG(int x, int y)", asFUNCTION(MAPCOMBOFLAG), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int MAPFFCOMBOFLAG(int x, int y)", asFUNCTION(MAPFFCOMBOFLAG), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int COMBOTYPE(int x, int y)", asFUNCTION(COMBOTYPE), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int FFCOMBOTYPE(int x, int y)", asFUNCTION(FFCOMBOTYPE), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool findentrance(int, int, int, bool)", asFUNCTION(findentrance), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool m_walkflag(int, int, int, int=-1000, int=-1000)", asFUNCTION(m_walkflag), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool isdungeon(int=-1, int=-1)", asFUNCTION(isdungeon), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int select_dropitem(int, int, int)", asFUNCTION(select_dropitem), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int addenemy(int, int, int, int, int)", asFUNCTIONPR(addenemy, (int, int, int, int, int), int), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void kill_em_all()", asFUNCTION(kill_em_all), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void setGuyGridAt(int, int, int)", asFUNCTION(asSetGuyGridAt), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool getmapflag(int=32)", asFUNCTION(getmapflag), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void setmapflag(int=32)", asFUNCTIONPR(setmapflag, (int), void), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void change_counter(int, int16)", asFUNCTION(asChangeCounter), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void change_dcounter(int, int16)", asFUNCTION(asChangeDCounter), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int get_generic(int)", asFUNCTION(asGetGeneric), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int getGravity()", asFUNCTION(asGetGravity), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int getTerminalVelocity()", asFUNCTION(asGetTerminalVelocity), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int decGuyCount(int)", asFUNCTION(asDecGuyCount), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("int getItemProp(int, int)", asFUNCTION(asGetItemProp), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int current_item(int)", asFUNCTIONPR(current_item, (int), int), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int current_item_id(int, bool=true)", asFUNCTION(current_item_id), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int current_item_power(int)", asFUNCTION(current_item_power), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int set_item(int, bool)", asFUNCTION(asSetItem), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void setLevelItem(int, int)", asFUNCTION(asSetLevelItem), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int getitem(int, bool=false)", asFUNCTION(getitem), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("int getWeaponProp(int, int)", asFUNCTION(asGetWeaponProp), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("int getComboProp(int, int)", asFUNCTION(asGetComboProp), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int getComboClassProp(int, int)", asFUNCTION(asGetComboClassProp), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("spriteIF@ __getGuy(int)", asFUNCTION(asGetGuyObject), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int numGuys()", asFUNCTION(asNumGuys), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int getGuyProp(int, int)", asFUNCTION(asGetGuyProp), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void swapGuys(int, int)", asFUNCTION(asSwapGuys), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("spriteIF@ __getItem(int)", asFUNCTION(asGetItemObject), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int numItems()", asFUNCTION(asNumItems), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void deleteItem(int)", asFUNCTION(asDeleteItem), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void placeitem(int, int, int, int, int=0)", asFUNCTION(asPlaceItem), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("spriteIF@ __getLwpn(int)", asFUNCTION(asGetLwpnObject), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int numLwpns()", asFUNCTION(asNumLwpns), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("spriteIF@ __getEwpn(int)", asFUNCTION(asGetEwpnObject), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("int numEwpns()", asFUNCTION(asNumEwpns), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void deleteEwpn(int)", asFUNCTION(asDeleteEwpn), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("void addDecoration(int, int, int)", asFUNCTION(asAddDecoration), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("void moveChainLinks(int, int)", asFUNCTION(asMoveChainLinks), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("void runZScript(uint8, uint16, uint8=255)", asFUNCTION(ZScriptVersion::RunScript), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("bool Up()", asFUNCTION(asInputUp), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool Down()", asFUNCTION(asInputDown), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool Left()", asFUNCTION(asInputLeft), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool Right()", asFUNCTION(asInputRight), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool cAbtn()", asFUNCTION(asCAbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool cBbtn()", asFUNCTION(asCBbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool cSbtn()", asFUNCTION(asCSbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool cMbtn()", asFUNCTION(asCMbtn), asCALL_CDECL); // Menu again...
    asEngine->RegisterGlobalFunction("bool cLbtn()", asFUNCTION(asCLbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool cRbtn()", asFUNCTION(asCRbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool cPbtn()", asFUNCTION(asCPbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool cEx1btn()", asFUNCTION(asCEx1btn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool cEx2btn()", asFUNCTION(asCEx2btn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool cEx3btn()", asFUNCTION(asCEx3btn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool cEx4btn()", asFUNCTION(asCEx4btn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool AxisUp()", asFUNCTION(asInputAxisUp), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool AxisDown()", asFUNCTION(asInputAxisDown), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool AxisLeft()", asFUNCTION(asInputAxisLeft), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool AxisRight()", asFUNCTION(asInputAxisRight), asCALL_CDECL);
    
    asEngine->RegisterGlobalFunction("bool rUp()", asFUNCTION(asRUp), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rDown()", asFUNCTION(asRDown), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rLeft()", asFUNCTION(asRLeft), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rRight()", asFUNCTION(asRRight), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rAbtn()", asFUNCTION(asRAbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rBbtn()", asFUNCTION(asRBbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rSbtn()", asFUNCTION(asRSbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rMbtn()", asFUNCTION(asRMbtn), asCALL_CDECL); // Menu button. Hmm...
    asEngine->RegisterGlobalFunction("bool rLbtn()", asFUNCTION(asRLbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rRbtn()", asFUNCTION(asRRbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rPbtn()", asFUNCTION(asRPbtn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rEx1btn()", asFUNCTION(asREx1btn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rEx2btn()", asFUNCTION(asREx2btn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rEx3btn()", asFUNCTION(asREx3btn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rEx4btn()", asFUNCTION(asREx4btn), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rAxisUp()", asFUNCTION(asRAxisUp), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rAxisDown()", asFUNCTION(asRAxisDown), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rAxisLeft()", asFUNCTION(asRAxisLeft), asCALL_CDECL);
    asEngine->RegisterGlobalFunction("bool rAxisRight()", asFUNCTION(asRAxisRight), asCALL_CDECL);
}

void registerMapscr()
{
    asEngine->RegisterObjectProperty("mapscr", "uint16 undercombo", asOFFSET(mapscr, undercombo));
    asEngine->RegisterObjectProperty("mapscr", "uint8 undercset", asOFFSET(mapscr, undercset));
    asEngine->RegisterObjectProperty("mapscr", "uint16 catchall", asOFFSET(mapscr, catchall));
    asEngine->RegisterObjectProperty("mapscr", "uint8 secretsfx", asOFFSET(mapscr, secretsfx));
    
    asEngine->RegisterObjectMethod("mapscr", "uint16 get_data(int)", asMETHOD(mapscr, getData), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "void set_data(int, uint16)", asMETHOD(mapscr, setData), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "uint8 get_cset(int)", asMETHOD(mapscr, getCSet), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "void set_cset(int, uint8)", asMETHOD(mapscr, setCSet), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "uint8 get_sflag(int)", asMETHOD(mapscr, getSFlag), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "void set_sflag(int, uint8)", asMETHOD(mapscr, setSFlag), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "uint16 get_secretcombo(int)", asMETHOD(mapscr, getSecretCombo), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "void set_secretcombo(int, uint16)", asMETHOD(mapscr, setSecretCombo), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "uint8 get_secretcset(int)", asMETHOD(mapscr, getSecretCSet), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "void set_secretcset(int, uint8)", asMETHOD(mapscr, setSecretCSet), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "uint8 get_secretflag(int)", asMETHOD(mapscr, getSecretFlag), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "void set_secretflag(int, uint8)", asMETHOD(mapscr, setSecretFlag), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("mapscr", "FFC@ get_ffcs(int)", asMETHOD(mapscr, getFFC), asCALL_THISCALL);
}

void registerFFC()
{
    asEngine->RegisterObjectMethod("FFC", "void modCombo(int16)", asMETHOD(FFC, modCombo), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("FFC", "void setCombo(uint16)", asMETHOD(FFC, setCombo), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("FFC", "void setCSet(uint16)", asMETHOD(FFC, setCSet), asCALL_THISCALL);
}

void registerSprite()
{
    // Most variables and functions aren't in here yet.
    // These should be moved  entirely into the script classes when no uses in C++ remain.
    asEngine->RegisterObjectProperty("__RealSprite", "int uid", asOFFSET(sprite, uid));
    asEngine->RegisterObjectProperty("__RealSprite", "fix x", asOFFSET(sprite, x));
    asEngine->RegisterObjectProperty("__RealSprite", "fix y", asOFFSET(sprite, y));
    asEngine->RegisterObjectProperty("__RealSprite", "fix z", asOFFSET(sprite, z));
    asEngine->RegisterObjectProperty("__RealSprite", "fix fall", asOFFSET(sprite, fall));
    asEngine->RegisterObjectProperty("__RealSprite", "int tile", asOFFSET(sprite, tile));
    asEngine->RegisterObjectProperty("__RealSprite", "int shadowtile", asOFFSET(sprite, shadowtile));
    asEngine->RegisterObjectProperty("__RealSprite", "int cs", asOFFSET(sprite, cs));
    asEngine->RegisterObjectProperty("__RealSprite", "int flip", asOFFSET(sprite, flip));
    asEngine->RegisterObjectProperty("__RealSprite", "int c_clk", asOFFSET(sprite, c_clk));
    asEngine->RegisterObjectProperty("__RealSprite", "int clk", asOFFSET(sprite, clk));
    asEngine->RegisterObjectProperty("__RealSprite", "int misc", asOFFSET(sprite, misc));
    asEngine->RegisterObjectProperty("__RealSprite", "fix xofs", asOFFSET(sprite, xofs));
    asEngine->RegisterObjectProperty("__RealSprite", "fix yofs", asOFFSET(sprite, yofs));
    asEngine->RegisterObjectProperty("__RealSprite", "fix zofs", asOFFSET(sprite, zofs));
    asEngine->RegisterObjectProperty("__RealSprite", "int hxofs", asOFFSET(sprite, hxofs));
    asEngine->RegisterObjectProperty("__RealSprite", "int hyofs", asOFFSET(sprite, hyofs));
    asEngine->RegisterObjectProperty("__RealSprite", "int hxsz", asOFFSET(sprite, hxsz));
    asEngine->RegisterObjectProperty("__RealSprite", "int hysz", asOFFSET(sprite, hysz));
    asEngine->RegisterObjectProperty("__RealSprite", "int hzsz", asOFFSET(sprite, hzsz));
    asEngine->RegisterObjectProperty("__RealSprite", "int txsz", asOFFSET(sprite, txsz));
    asEngine->RegisterObjectProperty("__RealSprite", "int tysz", asOFFSET(sprite, tysz));
    asEngine->RegisterObjectProperty("__RealSprite", "int id", asOFFSET(sprite, id));
    asEngine->RegisterObjectProperty("__RealSprite", "int dir", asOFFSET(sprite, dir));
    asEngine->RegisterObjectProperty("__RealSprite", "bool angular", asOFFSET(sprite, angular));
    asEngine->RegisterObjectProperty("__RealSprite", "bool canfreeze", asOFFSET(sprite, canfreeze));
    asEngine->RegisterObjectProperty("__RealSprite", "double angle", asOFFSET(sprite, angle));
    asEngine->RegisterObjectProperty("__RealSprite", "int lasthit", asOFFSET(sprite, lasthit));
    asEngine->RegisterObjectProperty("__RealSprite", "int lasthitclk", asOFFSET(sprite, lasthitclk));
    asEngine->RegisterObjectProperty("__RealSprite", "int drawstyle", asOFFSET(sprite, drawstyle));
    asEngine->RegisterObjectProperty("__RealSprite", "int extend", asOFFSET(sprite, extend));
    asEngine->RegisterObjectProperty("__RealSprite", "uint8 scriptcoldet", asOFFSET(sprite, scriptcoldet));
    
    asEngine->RegisterObjectMethod("__RealSprite", "void draw()", asMETHOD(sprite, scriptDraw), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealSprite", "void drawcloaked()", asMETHOD(sprite, scriptDrawCloaked), asCALL_THISCALL);
}

void registerLink()
{
    asEngine->RegisterObjectProperty("__RealLink", "bool tapping", asOFFSET(LinkClass, tapping));
    asEngine->RegisterObjectProperty("__RealLink", "bool stomping", asOFFSET(LinkClass, stomping));
    asEngine->RegisterObjectProperty("__RealLink", "int ladderx", asOFFSET(LinkClass, ladderx));
    asEngine->RegisterObjectProperty("__RealLink", "int laddery", asOFFSET(LinkClass, laddery));
    asEngine->RegisterObjectProperty("__RealLink", "int charging", asOFFSET(LinkClass, charging));
    asEngine->RegisterObjectProperty("__RealLink", "int spins", asOFFSET(LinkClass, spins));
    asEngine->RegisterObjectProperty("__RealLink", "int drunkclk", asOFFSET(LinkClass, drunkclk));
    asEngine->RegisterObjectProperty("__RealLink", "int hoverclk", asOFFSET(LinkClass, hoverclk));
    asEngine->RegisterObjectProperty("__RealLink", "int hclk", asOFFSET(LinkClass, hclk));
    asEngine->RegisterObjectProperty("__RealLink", "int attackclk", asOFFSET(LinkClass, attackclk));
    asEngine->RegisterObjectProperty("__RealLink", "int attack", asOFFSET(LinkClass, attack));
    asEngine->RegisterObjectProperty("__RealLink", "int attackid", asOFFSET(LinkClass, attackid));
    asEngine->RegisterObjectProperty("__RealLink", "int drownclk", asOFFSET(LinkClass, drownclk));
    asEngine->RegisterObjectProperty("__RealLink", "int slashxofs", asOFFSET(LinkClass, slashxofs));
    asEngine->RegisterObjectProperty("__RealLink", "int slashyofs", asOFFSET(LinkClass, slashyofs));
    asEngine->RegisterObjectProperty("__RealLink", "uint8 hopclk", asOFFSET(LinkClass, hopclk));
    asEngine->RegisterObjectProperty("__RealLink", "uint8 diveclk", asOFFSET(LinkClass, diveclk));
    asEngine->RegisterObjectProperty("__RealLink", "uint8 inlikelike", asOFFSET(LinkClass, inlikelike));
    asEngine->RegisterObjectProperty("__RealLink", "int shiftdir", asOFFSET(LinkClass, shiftdir));
    asEngine->RegisterObjectProperty("__RealLink", "int sdir", asOFFSET(LinkClass, sdir));
    asEngine->RegisterObjectProperty("__RealLink", "int hopdir", asOFFSET(LinkClass, hopdir));
    asEngine->RegisterObjectProperty("__RealLink", "int holddir", asOFFSET(LinkClass, holddir));
    asEngine->RegisterObjectProperty("__RealLink", "int landswim", asOFFSET(LinkClass, landswim));
    asEngine->RegisterObjectProperty("__RealLink", "bool ilswim", asOFFSET(LinkClass, ilswim));
    asEngine->RegisterObjectProperty("__RealLink", "bool walkable", asOFFSET(LinkClass, walkable));
    asEngine->RegisterObjectProperty("__RealLink", "int action", asOFFSET(LinkClass, action));
    asEngine->RegisterObjectProperty("__RealLink", "fix climb_cover_x", asOFFSET(LinkClass, climb_cover_x));
    asEngine->RegisterObjectProperty("__RealLink", "fix climb_cover_y", asOFFSET(LinkClass, climb_cover_y));
    asEngine->RegisterObjectProperty("__RealLink", "fix falling_oldy", asOFFSET(LinkClass, falling_oldy));
    asEngine->RegisterObjectProperty("__RealLink", "bool diagonalMovement", asOFFSET(LinkClass, diagonalMovement));
    
    asEngine->RegisterObjectMethod("__RealLink", "int getAction()", asMETHOD(LinkClass, getAction), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealLink", "void hitlink(int)", asMETHOD(LinkClass, hitlink), asCALL_THISCALL);
}

void registerEnemy()
{
    // Only a fraction of the variables and functions
    asEngine->RegisterObjectProperty("__RealEnemy", "int o_tile", asOFFSET(ASEnemy, o_tile));
    asEngine->RegisterObjectProperty("__RealEnemy", "int frate", asOFFSET(ASEnemy, frate));
    asEngine->RegisterObjectProperty("__RealEnemy", "int hp", asOFFSET(ASEnemy, hp));
    asEngine->RegisterObjectProperty("__RealEnemy", "int hclk", asOFFSET(ASEnemy, hclk));
    asEngine->RegisterObjectProperty("__RealEnemy", "int stunclk", asOFFSET(ASEnemy, stunclk));
    asEngine->RegisterObjectProperty("__RealEnemy", "int fading", asOFFSET(ASEnemy, fading));
    asEngine->RegisterObjectProperty("__RealEnemy", "int superman", asOFFSET(ASEnemy, superman));
    asEngine->RegisterObjectProperty("__RealEnemy", "bool mainguy", asOFFSET(ASEnemy, mainguy));
    asEngine->RegisterObjectProperty("__RealEnemy", "bool did_armos", asOFFSET(ASEnemy, did_armos));
    asEngine->RegisterObjectProperty("__RealEnemy", "uint8 item_set", asOFFSET(ASEnemy, item_set));
    asEngine->RegisterObjectProperty("__RealEnemy", "uint8 grumble", asOFFSET(ASEnemy, grumble));
    asEngine->RegisterObjectProperty("__RealEnemy", "bool itemguy", asOFFSET(ASEnemy, itemguy));
    asEngine->RegisterObjectProperty("__RealEnemy", "bool count_enemy", asOFFSET(ASEnemy, count_enemy));
    asEngine->RegisterObjectProperty("__RealEnemy", "fix step", asOFFSET(ASEnemy, step));
    asEngine->RegisterObjectProperty("__RealEnemy", "fix floor_y", asOFFSET(ASEnemy, floor_y));
    asEngine->RegisterObjectProperty("__RealEnemy", "bool dying", asOFFSET(ASEnemy, dying));
    asEngine->RegisterObjectProperty("__RealEnemy", "bool ceiling", asOFFSET(ASEnemy, ceiling));
    asEngine->RegisterObjectProperty("__RealEnemy", "bool leader", asOFFSET(ASEnemy, leader));
    asEngine->RegisterObjectProperty("__RealEnemy", "bool scored", asOFFSET(ASEnemy, scored));
    asEngine->RegisterObjectProperty("__RealEnemy", "bool script_spawned", asOFFSET(ASEnemy, script_spawned));
    asEngine->RegisterObjectProperty("__RealEnemy", "uint flags", asOFFSET(ASEnemy, flags));
    asEngine->RegisterObjectProperty("__RealEnemy", "uint flags2", asOFFSET(ASEnemy, flags2));
    asEngine->RegisterObjectProperty("__RealEnemy", "int16 family", asOFFSET(ASEnemy, family));
    asEngine->RegisterObjectProperty("__RealEnemy", "int16 dcset", asOFFSET(ASEnemy, dcset));
    asEngine->RegisterObjectProperty("__RealEnemy", "int16 anim", asOFFSET(ASEnemy, anim));
    asEngine->RegisterObjectProperty("__RealEnemy", "int16 dp", asOFFSET(ASEnemy, dp));
    asEngine->RegisterObjectProperty("__RealEnemy", "int16 wpn", asOFFSET(ASEnemy, wpn));
    asEngine->RegisterObjectProperty("__RealEnemy", "int16 rate", asOFFSET(ASEnemy, rate));
    asEngine->RegisterObjectProperty("__RealEnemy", "int16 hrate", asOFFSET(ASEnemy, hrate));
    asEngine->RegisterObjectProperty("__RealEnemy", "int16 homing", asOFFSET(ASEnemy, homing));
    asEngine->RegisterObjectProperty("__RealEnemy", "fix dstep", asOFFSET(ASEnemy, dstep));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dmisc1", asOFFSET(ASEnemy, dmisc1));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dmisc2", asOFFSET(ASEnemy, dmisc2));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dmisc3", asOFFSET(ASEnemy, dmisc3));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dmisc4", asOFFSET(ASEnemy, dmisc4));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dmisc5", asOFFSET(ASEnemy, dmisc5));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dmisc6", asOFFSET(ASEnemy, dmisc6));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dmisc7", asOFFSET(ASEnemy, dmisc7));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dmisc8", asOFFSET(ASEnemy, dmisc8));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dmisc9", asOFFSET(ASEnemy, dmisc9));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dmisc10", asOFFSET(ASEnemy, dmisc10));
    asEngine->RegisterObjectProperty("__RealEnemy", "int16 bgsfx", asOFFSET(ASEnemy, bgsfx));
    asEngine->RegisterObjectProperty("__RealEnemy", "int dummy_int1", asOFFSET(ASEnemy, dummy_int1));
    asEngine->RegisterObjectProperty("__RealEnemy", "int clk2", asOFFSET(ASEnemy, clk2));
    asEngine->RegisterObjectProperty("__RealEnemy", "int sclk", asOFFSET(ASEnemy, sclk));
    asEngine->RegisterObjectProperty("__RealEnemy", "uint16 s_tile", asOFFSET(ASEnemy, s_tile));
    asEngine->RegisterObjectProperty("__RealEnemy", "int scriptFlags", asOFFSET(ASEnemy, scriptFlags));
    asEngine->RegisterObjectProperty("__RealEnemy", "bool haslink", asOFFSET(ASEnemy, haslink));
    
    asEngine->RegisterObjectMethod("__RealEnemy", "void leave_item()", asMETHOD(ASEnemy, leave_item), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "bool canmove(int, fix, int, int, int, int, int)", asMETHODPR(ASEnemy, canmove, (int, fix, int, int, int, int, int), bool), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void fireWeapon()", asMETHOD(ASEnemy, fireWeapon), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "bool isFiring() const", asMETHOD(ASEnemy, isFiring), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void updateFiring()", asMETHOD(ASEnemy, updateFiring), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void setBreathTimer(int)", asMETHOD(ASEnemy, setBreathTimer), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void setAttackOwner(int)", asMETHOD(ASEnemy, setAttackOwner), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void activateDeathAttack()", asMETHOD(ASEnemy, activateDeathAttack), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void draw()", asMETHOD(ASEnemy, scriptDraw), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void drawblock()", asMETHOD(ASEnemy, scriptDrawBlock), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void drawshadow()", asMETHOD(ASEnemy, scriptDrawShadow), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void masked_draw(int, int, int, int)", asMETHOD(ASEnemy, scriptMaskedDraw), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void overtilecloaked16(int, int, int, int)", asMETHOD(ASEnemy, overTileCloaked16), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void overtile16(int, int, int, int, int)", asMETHOD(ASEnemy, overTile16), asCALL_THISCALL);
    
    asEngine->RegisterObjectMethod("__RealEnemy", "void stopBGSFX()", asMETHOD(ASEnemy, stopBGSFX), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void playDeathSFX()", asMETHOD(ASEnemy, playDeathSFX), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "void playHitSFX(int)", asMETHOD(ASEnemy, playHitSFX), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "int defenditemclass(int, int)", asMETHOD(ASEnemy, scriptDefendItemClass), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "int getDefendedItemPower()", asMETHOD(ASEnemy, getDefendedItemPower), asCALL_THISCALL);
    asEngine->RegisterObjectMethod("__RealEnemy", "int takehit(__RealWeapon@)", asMETHOD(ASEnemy, defaultTakeHit), asCALL_THISCALL);
}

void registerItem()
{
    asEngine->RegisterObjectProperty("__RealItem", "int pickup", asOFFSET(item, pickup));
    asEngine->RegisterObjectProperty("__RealItem", "int clk2", asOFFSET(item, clk2));
}

void registerWeapon()
{
    asEngine->RegisterObjectProperty("__RealWeapon", "int power", asOFFSET(weapon, power));
    asEngine->RegisterObjectProperty("__RealWeapon", "int type", asOFFSET(weapon, type));
    asEngine->RegisterObjectProperty("__RealWeapon", "int parentid", asOFFSET(weapon, parentid));
    asEngine->RegisterObjectProperty("__RealWeapon", "int parentitem", asOFFSET(weapon, parentitem));
    asEngine->RegisterObjectProperty("__RealWeapon", "uint16 aframe", asOFFSET(weapon, aframe));
    asEngine->RegisterObjectProperty("__RealWeapon", "int o_tile", asOFFSET(weapon, o_tile));
    asEngine->RegisterObjectProperty("__RealWeapon", "int o_cset", asOFFSET(weapon, o_cset));
    asEngine->RegisterObjectProperty("__RealWeapon", "bool aimedBrang", asOFFSET(weapon, aimedBrang));
    
    asEngine->RegisterObjectMethod("__RealWeapon", "void adjustDraggedItem(int)", asMETHOD(weapon, adjustDraggedItem), asCALL_THISCALL);
}

void initializeAngelScript()
{
    asEngine=asCreateScriptEngine();
    asEngine->SetMessageCallback(asFUNCTION(messageCallback), 0, asCALL_CDECL);
    RegisterScriptArray(asEngine, true);
    RegisterStdString(asEngine);
    RegisterScriptMath(asEngine);
    registerFix();
    registerTypes();
    registerGlobalFunctions();
    registerMapscr();
    registerFFC();
    registerSprite();
    registerLink();
    registerEnemy();
    registerItem();
    registerWeapon();
    
    // These should be moved into a .dat file eventually; there's no reason
    // to let users edit them. They might even be distributed as bytecode.
    CScriptBuilder builder;
    builder.StartNewModule(asEngine, "everything");
    
    builder.AddSectionFromFile("angelscript/common.as");
    
    builder.AddSectionFromFile("angelscript/sprite.as");
    builder.AddSectionFromFile("angelscript/link.as");
    builder.AddSectionFromFile("angelscript/enemy.as");
    builder.AddSectionFromFile("angelscript/weapon.as");
    
    builder.AddSectionFromFile("angelscript/aquamentus.as");
    builder.AddSectionFromFile("angelscript/fairy.as");
    builder.AddSectionFromFile("angelscript/fallingRock.as");
    builder.AddSectionFromFile("angelscript/fire.as");
    builder.AddSectionFromFile("angelscript/digdogger.as");
    builder.AddSectionFromFile("angelscript/dodongo.as");
    builder.AddSectionFromFile("angelscript/ganon.as");
    builder.AddSectionFromFile("angelscript/ghini.as");
    builder.AddSectionFromFile("angelscript/gleeok.as");
    builder.AddSectionFromFile("angelscript/gohma.as");
    builder.AddSectionFromFile("angelscript/guy.as");
    builder.AddSectionFromFile("angelscript/item.as");
    builder.AddSectionFromFile("angelscript/keese.as");
    builder.AddSectionFromFile("angelscript/lanmola.as");
    builder.AddSectionFromFile("angelscript/leever.as");
    builder.AddSectionFromFile("angelscript/manhandla.as");
    builder.AddSectionFromFile("angelscript/moldorm.as");
    builder.AddSectionFromFile("angelscript/npc.as");
    builder.AddSectionFromFile("angelscript/other.as");
    builder.AddSectionFromFile("angelscript/patra.as");
    builder.AddSectionFromFile("angelscript/patraBS.as");
    builder.AddSectionFromFile("angelscript/peahat.as");
    builder.AddSectionFromFile("angelscript/projectileShooter.as");
    builder.AddSectionFromFile("angelscript/spinningTile.as");
    builder.AddSectionFromFile("angelscript/tektite.as");
    builder.AddSectionFromFile("angelscript/trapConstant.as");
    builder.AddSectionFromFile("angelscript/trapLOS.as");
    builder.AddSectionFromFile("angelscript/trigger.as");
    builder.AddSectionFromFile("angelscript/walkingEnemy.as");
    builder.AddSectionFromFile("angelscript/wallMaster.as");
    builder.AddSectionFromFile("angelscript/wizzrobe.as");
    builder.AddSectionFromFile("angelscript/zora.as");
    builder.BuildModule();
    
    // Create global Link
    currentSprite=&Link;
    assignSpriteScript(&Link, "LinkClass");
    Link.scriptInit();
    /*
    asIScriptContext* context=asEngine->CreateContext();
    asIScriptModule *module=asEngine->GetModuleByIndex(0);
    asIScriptFunction* func=module->GetFunctionByDecl("void init()");
    context->Prepare(func);
    context->Execute();
    context->Release();
    */
}

void shutDownAngelScript()
{
    asEngine->ShutDownAndRelease();
}

// This is kind of ugly, but it'll do...
static sprite* getRealSprite()
{
    return currentSprite;
}

static enemy* getRealEnemy()
{
    return currentEnemy;
}

static item* getRealItem()
{
    return currentItem;
}

static LinkClass* getRealLink()
{
    return &Link;
}

static weapon* getRealWeapon()
{
    return currentWeapon;
}

static void assignSpriteScript(sprite* spr, const char* scriptName)
{
    currentSprite=spr;
    
    asIScriptModule *module=asEngine->GetModuleByIndex(0);
    asIObjectType* objType=module->GetObjectTypeByName(scriptName);
    asIScriptObject* scriptObj=(asIScriptObject*)asEngine->CreateScriptObject(objType);
    spr->setScriptData(new EntityScriptData(scriptObj, asEngine->CreateContext()));
}

void assignEnemyScript(enemy* en, const char* scriptName)
{
    currentEnemy=en;
    assignSpriteScript(en, scriptName);
}

void assignItemScript(item* it, const char* scriptName)
{
    currentItem=it;
    assignSpriteScript(it, scriptName);
}

void assignWeaponScript(weapon* w, const char* scriptName)
{
    currentWeapon=w;
    assignSpriteScript(w, scriptName);
}

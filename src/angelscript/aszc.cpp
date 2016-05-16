#include "config.h"
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
	int r = 0;
    r = asEngine->RegisterObjectType("fix", sizeof(fix), asOBJ_VALUE|asOBJ_POD|asOBJ_APP_CLASS_CAK|asOBJ_APP_CLASS_ALLINTS); Assert(r >= 0);
    
    // Constructors
    r = asEngine->RegisterObjectBehaviour("fix", asBEHAVE_CONSTRUCT, "void fix()", asFUNCTION(fixCons), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectBehaviour("fix", asBEHAVE_CONSTRUCT, "void fix(const fix &in)", asFUNCTION(fixConsCopy), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectBehaviour("fix", asBEHAVE_CONSTRUCT, "void fix(int)", asFUNCTION(fixConsInt), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectBehaviour("fix", asBEHAVE_CONSTRUCT, "void fix(float)", asFUNCTION(fixConsFloat), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    
    // Arithmetic
    r = asEngine->RegisterObjectMethod("fix", "fix opNeg() const", asMETHODPR(fix, operator-, () const, fix), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opAdd(fix) const", asFUNCTIONPR(operator+, (fix, fix), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opAdd(int) const", asFUNCTIONPR(operator+, (fix, int), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opAdd(float) const", asFUNCTIONPR(operator+, (fix, float), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opSub(fix) const", asFUNCTIONPR(operator-, (fix, fix), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opSub(int) const", asFUNCTIONPR(operator-, (fix, int), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opSub(float) const", asFUNCTIONPR(operator-, (fix, float), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opMul(fix) const", asFUNCTIONPR(operator*, (fix, fix), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opMul(int) const", asFUNCTIONPR(operator*, (fix, int), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opMul(float) const", asFUNCTIONPR(operator*, (fix, float), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opDiv(fix) const", asFUNCTIONPR(operator/, (fix, fix), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opDiv(int) const", asFUNCTIONPR(operator/, (fix, int), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opDiv(float) const", asFUNCTIONPR(operator/, (fix, float), fix), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opPostInc()", asMETHODPR(fix, operator++, (), fix&), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix opPostDec()", asMETHODPR(fix, operator--, (), fix&), asCALL_THISCALL); Assert(r >= 0);
    
    // Assignment
    r = asEngine->RegisterObjectMethod("fix", "fix& opAssign(const fix &in)", asMETHODPR(fix, operator=, (const fix&), fix&), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix& opAssign(int)", asMETHODPR(fix, operator=, (const int), fix&), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix& opAssign(float)", asMETHODPR(fix, operator=, (const float), fix&), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix& opAddAssign(const fix &in)", asMETHODPR(fix, operator+=, (fix), fix&), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix& opSubAssign(const fix &in)", asMETHODPR(fix, operator-=, (fix), fix&), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix& opMulAssign(const fix &in)", asMETHODPR(fix, operator*=, (fix), fix&), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "fix& opDivAssign(const fix &in)", asMETHODPR(fix, operator/=, (fix), fix&), asCALL_THISCALL); Assert(r >= 0);
    
    // Allegro's comparison operators return int, not bool. Not sure if
    // this might be a problem on platforms where they're not the same size.
    r = asEngine->RegisterObjectMethod("fix", "int opEquals(const fix) const", asFUNCTIONPR(operator==, (fix, fix), int), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "int opEquals(const int) const", asFUNCTIONPR(operator==, (fix, int), int), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "int opEquals(const float) const", asFUNCTIONPR(operator==, (fix, float), int), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "int opCmp(const fix) const", asFUNCTION(fixComp), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    
    // Casts
    r = asEngine->RegisterObjectMethod("fix", "int opImplConv() const", asMETHODPR(fix, operator int, () const, int), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "float opImplConv() const", asMETHODPR(fix, operator float, () const, float), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "double opImplConv() const", asMETHODPR(fix, operator double, () const, double), asCALL_THISCALL); Assert(r >= 0);
    
    // x.v
    r = asEngine->RegisterObjectMethod("fix", "int getV()", asFUNCTION(fixV), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("fix", "void vAND(uint)", asFUNCTION(fixVAND), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
    
    // And more
    r = asEngine->RegisterObjectMethod("fix", "fix sin()", asFUNCTION(fixSin), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
}

// All the classes are registered together first because their methods
// need to take each other as arguments.
static void registerTypes()
{
	int r = 0;
    r = asEngine->RegisterObjectType("__RealSprite", sizeof(sprite), asOBJ_REF|asOBJ_NOCOUNT); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("__RealSprite@ __getRealSprite()", asFUNCTION(getRealSprite), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterObjectType("__RealLink", sizeof(LinkClass), asOBJ_REF|asOBJ_NOCOUNT); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("__RealLink@ __getRealLink()", asFUNCTION(getRealLink), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterObjectType("__RealEnemy", sizeof(enemy), asOBJ_REF|asOBJ_NOCOUNT); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("__RealEnemy@ __getRealEnemy()", asFUNCTION(getRealEnemy), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterObjectType("__RealItem", sizeof(item), asOBJ_REF|asOBJ_NOCOUNT); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("__RealItem@ __getRealItem()", asFUNCTION(getRealItem), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterObjectType("__RealWeapon", sizeof(weapon), asOBJ_REF|asOBJ_NOCOUNT); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("__RealWeapon@ __getRealWeapon()", asFUNCTION(getRealWeapon), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterObjectType("mapscr", sizeof(mapscr), asOBJ_REF|asOBJ_NOCOUNT); Assert(r >= 0);
    r = asEngine->RegisterObjectType("FFC", sizeof(FFC), asOBJ_REF|asOBJ_NOCOUNT); Assert(r >= 0);
    r = asEngine->RegisterInterface("spriteIF"); Assert(r >= 0); // Dummy interface for getters
}
void TraceI(int i) { al_trace("%d\n", i); }
void registerGlobalFunctions() // And properties... Whatever.
{
	int r = 0;
    r = asEngine->RegisterGlobalFunction("void TraceI(int)", asFUNCTION(TraceI), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("const int frame", &frame); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("const int db", &db); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("const bool BSZ", &BSZ); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("const int fadeclk", &fadeclk); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("bool watch", &watch); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("const int whistleclk", &whistleclk); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("int wallm_load_clk", &wallm_load_clk); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("int hasitem", &hasitem); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("const int playing_field_offset", &playing_field_offset); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("const int loadside", &loadside); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("int guycarryingitem", &guycarryingitem); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("int16 lensclk", &lensclk); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("int currmap", &currmap); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("int currscr", &currscr); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("int dlevel", &dlevel); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("int directWpn", &directWpn); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("bool pull_link", &pull_link); Assert(r >= 0);
    r = asEngine->RegisterGlobalProperty("bool toogam", &toogam); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("int rand()", asFUNCTION(rand), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("mapscr@ getTmpscr(int)", asFUNCTION(asGetTmpscr), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("mapscr@ getTmpscr2(int)", asFUNCTION(asGetTmpscr2), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("mapscr@ getTmpscr3(int)", asFUNCTION(asGetTmpscr3), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("int hit_enemy(int, int, int, int, int, int, int)", asFUNCTION(hit_enemy), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int getFFCAt(int, int)", asFUNCTION(getFFCAt), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void loadpalset(int, int)", asFUNCTION(loadpalset), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void music_stop()", asFUNCTION(music_stop), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void playLevelMusic()", asFUNCTION(playLevelMusic), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void setupscreen()", asFUNCTION(setupscreen), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool screenFlagSet(int)", asFUNCTION(asScreenFlagIsSet), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int getComboType(int)", asFUNCTION(asGetComboType), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int getSpriteTile(int)", asFUNCTION(getSpriteTile), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool questRuleEnabled(int)", asFUNCTION(asQuestRuleEnabled), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void sfx(int, int=128)", asFUNCTION(asSFX), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void stop_sfx(int)", asFUNCTION(asStopSFX), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool iswater(int)", asFUNCTION(asIsWater), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool _walkflag(int, int, int)", asFUNCTION(_walkflag), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int MAPCOMBO(int x, int y)", asFUNCTION(MAPCOMBO), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int MAPFLAG(int x, int y)", asFUNCTION(MAPFLAG), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int MAPCOMBOFLAG(int x, int y)", asFUNCTION(MAPCOMBOFLAG), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int MAPFFCOMBOFLAG(int x, int y)", asFUNCTION(MAPFFCOMBOFLAG), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int COMBOTYPE(int x, int y)", asFUNCTION(COMBOTYPE), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int FFCOMBOTYPE(int x, int y)", asFUNCTION(FFCOMBOTYPE), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool findentrance(int, int, int, bool)", asFUNCTION(findentrance), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool m_walkflag(int, int, int, int=-1000, int=-1000)", asFUNCTION(m_walkflag), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool isdungeon(int=-1, int=-1)", asFUNCTION(isdungeon), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int select_dropitem(int, int, int)", asFUNCTION(select_dropitem), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int addenemy(int, int, int, int, int)", asFUNCTIONPR(addenemy, (int, int, int, int, int), int), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void kill_em_all()", asFUNCTION(kill_em_all), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void setGuyGridAt(int, int, int)", asFUNCTION(asSetGuyGridAt), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool getmapflag(int=32)", asFUNCTION(getmapflag), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void setmapflag(int=32)", asFUNCTIONPR(setmapflag, (int), void), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void change_counter(int, int16)", asFUNCTION(asChangeCounter), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void change_dcounter(int, int16)", asFUNCTION(asChangeDCounter), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int get_generic(int)", asFUNCTION(asGetGeneric), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int getGravity()", asFUNCTION(asGetGravity), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int getTerminalVelocity()", asFUNCTION(asGetTerminalVelocity), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int decGuyCount(int)", asFUNCTION(asDecGuyCount), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("int getItemProp(int, int)", asFUNCTION(asGetItemProp), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int current_item(int)", asFUNCTIONPR(current_item, (int), int), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int current_item_id(int, bool=true)", asFUNCTION(current_item_id), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int current_item_power(int)", asFUNCTION(current_item_power), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int set_item(int, bool)", asFUNCTION(asSetItem), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void setLevelItem(int, int)", asFUNCTION(asSetLevelItem), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int getitem(int, bool=false)", asFUNCTION(getitem), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("int getWeaponProp(int, int)", asFUNCTION(asGetWeaponProp), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("int getComboProp(int, int)", asFUNCTION(asGetComboProp), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int getComboClassProp(int, int)", asFUNCTION(asGetComboClassProp), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("spriteIF@ __getGuy(int)", asFUNCTION(asGetGuyObject), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int numGuys()", asFUNCTION(asNumGuys), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int getGuyProp(int, int)", asFUNCTION(asGetGuyProp), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void swapGuys(int, int)", asFUNCTION(asSwapGuys), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("spriteIF@ __getItem(int)", asFUNCTION(asGetItemObject), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int numItems()", asFUNCTION(asNumItems), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void deleteItem(int)", asFUNCTION(asDeleteItem), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void placeitem(int, int, int, int, int=0)", asFUNCTION(asPlaceItem), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("spriteIF@ __getLwpn(int)", asFUNCTION(asGetLwpnObject), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int numLwpns()", asFUNCTION(asNumLwpns), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("spriteIF@ __getEwpn(int)", asFUNCTION(asGetEwpnObject), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("int numEwpns()", asFUNCTION(asNumEwpns), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void deleteEwpn(int)", asFUNCTION(asDeleteEwpn), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("void addDecoration(int, int, int)", asFUNCTION(asAddDecoration), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("void moveChainLinks(int, int)", asFUNCTION(asMoveChainLinks), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("void runZScript(uint8, uint16, uint8=255)", asFUNCTION(ZScriptVersion::RunScript), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("bool Up()", asFUNCTION(asInputUp), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool Down()", asFUNCTION(asInputDown), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool Left()", asFUNCTION(asInputLeft), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool Right()", asFUNCTION(asInputRight), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool cAbtn()", asFUNCTION(asCAbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool cBbtn()", asFUNCTION(asCBbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool cSbtn()", asFUNCTION(asCSbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool cMbtn()", asFUNCTION(asCMbtn), asCALL_CDECL); Assert(r >= 0); // Menu again...
    r = asEngine->RegisterGlobalFunction("bool cLbtn()", asFUNCTION(asCLbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool cRbtn()", asFUNCTION(asCRbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool cPbtn()", asFUNCTION(asCPbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool cEx1btn()", asFUNCTION(asCEx1btn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool cEx2btn()", asFUNCTION(asCEx2btn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool cEx3btn()", asFUNCTION(asCEx3btn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool cEx4btn()", asFUNCTION(asCEx4btn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool AxisUp()", asFUNCTION(asInputAxisUp), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool AxisDown()", asFUNCTION(asInputAxisDown), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool AxisLeft()", asFUNCTION(asInputAxisLeft), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool AxisRight()", asFUNCTION(asInputAxisRight), asCALL_CDECL); Assert(r >= 0);
    
    r = asEngine->RegisterGlobalFunction("bool rUp()", asFUNCTION(asRUp), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rDown()", asFUNCTION(asRDown), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rLeft()", asFUNCTION(asRLeft), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rRight()", asFUNCTION(asRRight), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rAbtn()", asFUNCTION(asRAbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rBbtn()", asFUNCTION(asRBbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rSbtn()", asFUNCTION(asRSbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rMbtn()", asFUNCTION(asRMbtn), asCALL_CDECL); Assert(r >= 0); // Menu button. Hmm...
    r = asEngine->RegisterGlobalFunction("bool rLbtn()", asFUNCTION(asRLbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rRbtn()", asFUNCTION(asRRbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rPbtn()", asFUNCTION(asRPbtn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rEx1btn()", asFUNCTION(asREx1btn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rEx2btn()", asFUNCTION(asREx2btn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rEx3btn()", asFUNCTION(asREx3btn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rEx4btn()", asFUNCTION(asREx4btn), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rAxisUp()", asFUNCTION(asRAxisUp), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rAxisDown()", asFUNCTION(asRAxisDown), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rAxisLeft()", asFUNCTION(asRAxisLeft), asCALL_CDECL); Assert(r >= 0);
    r = asEngine->RegisterGlobalFunction("bool rAxisRight()", asFUNCTION(asRAxisRight), asCALL_CDECL); Assert(r >= 0);
}

void registerMapscr()
{
	int r = 0;
    r = asEngine->RegisterObjectProperty("mapscr", "uint16 undercombo", asOFFSET(mapscr, undercombo)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("mapscr", "uint8 undercset", asOFFSET(mapscr, undercset)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("mapscr", "uint16 catchall", asOFFSET(mapscr, catchall)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("mapscr", "uint8 secretsfx", asOFFSET(mapscr, secretsfx)); Assert(r >= 0);
    
    r = asEngine->RegisterObjectMethod("mapscr", "uint16 get_data(int)", asMETHOD(mapscr, getData), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "void set_data(int, uint16)", asMETHOD(mapscr, setData), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "uint8 get_cset(int)", asMETHOD(mapscr, getCSet), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "void set_cset(int, uint8)", asMETHOD(mapscr, setCSet), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "uint8 get_sflag(int)", asMETHOD(mapscr, getSFlag), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "void set_sflag(int, uint8)", asMETHOD(mapscr, setSFlag), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "uint16 get_secretcombo(int)", asMETHOD(mapscr, getSecretCombo), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "void set_secretcombo(int, uint16)", asMETHOD(mapscr, setSecretCombo), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "uint8 get_secretcset(int)", asMETHOD(mapscr, getSecretCSet), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "void set_secretcset(int, uint8)", asMETHOD(mapscr, setSecretCSet), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "uint8 get_secretflag(int)", asMETHOD(mapscr, getSecretFlag), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "void set_secretflag(int, uint8)", asMETHOD(mapscr, setSecretFlag), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("mapscr", "FFC@ get_ffcs(int)", asMETHOD(mapscr, getFFC), asCALL_THISCALL); Assert(r >= 0);
}

void registerFFC()
{
	int r = 0;
    r = asEngine->RegisterObjectMethod("FFC", "void modCombo(int16)", asMETHOD(FFC, modCombo), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("FFC", "void setCombo(uint16)", asMETHOD(FFC, setCombo), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("FFC", "void setCSet(uint16)", asMETHOD(FFC, setCSet), asCALL_THISCALL); Assert(r >= 0);
}

void registerSprite()
{
    // Most variables and functions aren't in here yet.
    // These should be moved  entirely into the script classes when no uses in C++ remain.
	int r = 0;
    r = asEngine->RegisterObjectProperty("__RealSprite", "int uid", asOFFSET(sprite, uid)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "fix x", asOFFSET(sprite, x)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "fix y", asOFFSET(sprite, y)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "fix z", asOFFSET(sprite, z)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "fix fall", asOFFSET(sprite, fall)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int tile", asOFFSET(sprite, tile)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int shadowtile", asOFFSET(sprite, shadowtile)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int cs", asOFFSET(sprite, cs)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int flip", asOFFSET(sprite, flip)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int c_clk", asOFFSET(sprite, c_clk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int clk", asOFFSET(sprite, clk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int misc", asOFFSET(sprite, misc)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "fix xofs", asOFFSET(sprite, xofs)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "fix yofs", asOFFSET(sprite, yofs)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "fix zofs", asOFFSET(sprite, zofs)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int hxofs", asOFFSET(sprite, hxofs)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int hyofs", asOFFSET(sprite, hyofs)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int hxsz", asOFFSET(sprite, hxsz)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int hysz", asOFFSET(sprite, hysz)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int hzsz", asOFFSET(sprite, hzsz)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int txsz", asOFFSET(sprite, txsz)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int tysz", asOFFSET(sprite, tysz)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int id", asOFFSET(sprite, id)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int dir", asOFFSET(sprite, dir)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "bool angular", asOFFSET(sprite, angular)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "bool canfreeze", asOFFSET(sprite, canfreeze)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "double angle", asOFFSET(sprite, angle)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int lasthit", asOFFSET(sprite, lasthit)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int lasthitclk", asOFFSET(sprite, lasthitclk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int drawstyle", asOFFSET(sprite, drawstyle)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "int extend", asOFFSET(sprite, extend)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealSprite", "uint8 scriptcoldet", asOFFSET(sprite, scriptcoldet)); Assert(r >= 0);
    
    //r = asEngine->RegisterObjectMethod("__RealSprite", "void draw()", asMETHOD(sprite, scriptDraw), asCALL_THISCALL); Assert(r >= 0);
    //r = asEngine->RegisterObjectMethod("__RealSprite", "void drawcloaked()", asMETHOD(sprite, scriptDrawCloaked), asCALL_THISCALL); Assert(r >= 0);
}

void registerLink()
{
	int r = 0;
    r = asEngine->RegisterObjectProperty("__RealLink", "bool tapping", asOFFSET(LinkClass, tapping)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "bool stomping", asOFFSET(LinkClass, stomping)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int ladderx", asOFFSET(LinkClass, ladderx)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int laddery", asOFFSET(LinkClass, laddery)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int charging", asOFFSET(LinkClass, charging)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int spins", asOFFSET(LinkClass, spins)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int drunkclk", asOFFSET(LinkClass, drunkclk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int hoverclk", asOFFSET(LinkClass, hoverclk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int hclk", asOFFSET(LinkClass, hclk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int attackclk", asOFFSET(LinkClass, attackclk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int attack", asOFFSET(LinkClass, attack)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int attackid", asOFFSET(LinkClass, attackid)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int drownclk", asOFFSET(LinkClass, drownclk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int slashxofs", asOFFSET(LinkClass, slashxofs)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int slashyofs", asOFFSET(LinkClass, slashyofs)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "uint8 hopclk", asOFFSET(LinkClass, hopclk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "uint8 diveclk", asOFFSET(LinkClass, diveclk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "uint8 inlikelike", asOFFSET(LinkClass, inlikelike)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int shiftdir", asOFFSET(LinkClass, shiftdir)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int sdir", asOFFSET(LinkClass, sdir)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int hopdir", asOFFSET(LinkClass, hopdir)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int holddir", asOFFSET(LinkClass, holddir)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int landswim", asOFFSET(LinkClass, landswim)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "bool ilswim", asOFFSET(LinkClass, ilswim)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "bool walkable", asOFFSET(LinkClass, walkable)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "int action", asOFFSET(LinkClass, action)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "fix climb_cover_x", asOFFSET(LinkClass, climb_cover_x)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "fix climb_cover_y", asOFFSET(LinkClass, climb_cover_y)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "fix falling_oldy", asOFFSET(LinkClass, falling_oldy)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealLink", "bool diagonalMovement", asOFFSET(LinkClass, diagonalMovement)); Assert(r >= 0);
    
    r = asEngine->RegisterObjectMethod("__RealLink", "int getAction()", asMETHOD(LinkClass, getAction), asCALL_THISCALL); Assert(r >= 0);
    r = asEngine->RegisterObjectMethod("__RealLink", "void hitlink(int)", asMETHOD(LinkClass, hitlink), asCALL_THISCALL); Assert(r >= 0);
}

void registerEnemy()
{
}

void registerItem()
{
	int r = 0;
    r = asEngine->RegisterObjectProperty("__RealItem", "int pickup", asOFFSET(item, pickup)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealItem", "int clk2", asOFFSET(item, clk2)); Assert(r >= 0);
}

void registerWeapon()
{
	int r = 0;
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int power", asOFFSET(weapon, power)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int type", asOFFSET(weapon, type)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int dead", asOFFSET(weapon, dead)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int clk2", asOFFSET(weapon, clk2)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int misc2", asOFFSET(weapon, misc2)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int ignorecombo", asOFFSET(weapon, ignorecombo)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "bool isLit", asOFFSET(weapon, isLit)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int parentid", asOFFSET(weapon, parentid)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int parentitem", asOFFSET(weapon, parentitem)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int dragging", asOFFSET(weapon, dragging)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "fix step", asOFFSET(weapon, step)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "bool bounce", asOFFSET(weapon, bounce)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "bool ignoreLink", asOFFSET(weapon, ignoreLink)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "uint16 flash", asOFFSET(weapon, flash)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "uint16 wid", asOFFSET(weapon, wid)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "uint16 aframe", asOFFSET(weapon, aframe)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "uint16 csclk", asOFFSET(weapon, csclk)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int o_tile", asOFFSET(weapon, o_tile)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int o_cset", asOFFSET(weapon, o_cset)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int o_speed", asOFFSET(weapon, o_speed)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int o_type", asOFFSET(weapon, o_type)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int frames", asOFFSET(weapon, frames)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int o_flip", asOFFSET(weapon, o_flip)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int temp1", asOFFSET(weapon, temp1)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "bool behind", asOFFSET(weapon, behind)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "bool aimedBrang", asOFFSET(weapon, aimedBrang)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int minX", asOFFSET(weapon, minX)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int maxX", asOFFSET(weapon, maxX)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int minY", asOFFSET(weapon, minY)); Assert(r >= 0);
    r = asEngine->RegisterObjectProperty("__RealWeapon", "int maxY", asOFFSET(weapon, maxY)); Assert(r >= 0);
    
    r = asEngine->RegisterObjectMethod("__RealWeapon", "void adjustDraggedItem(int)", asMETHOD(weapon, adjustDraggedItem), asCALL_THISCALL); Assert(r >= 0);
}

void initializeAngelScript()
{
    // Nope.
    return;
    
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
    builder.AddSectionFromFile("angelscript/walkflagInfo.as");
    builder.AddSectionFromFile("angelscript/walkingEnemy.as");
    builder.AddSectionFromFile("angelscript/wallMaster.as");
    builder.AddSectionFromFile("angelscript/wizzrobe.as");
    builder.AddSectionFromFile("angelscript/zora.as");

    int buildResult = builder.BuildModule(); //if this fails zc will crash.
	Assert(buildResult == 0);
	if(buildResult != 0)
	{
		exit(1);
	}
    
    // Create global Link
    currentSprite=&Link;
    assignSpriteScript(&Link, "LinkClass");
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
    //asEngine->ShutDownAndRelease();
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
    //spr->setScriptData(new EntityScriptData(scriptObj, asEngine->CreateContext()));
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

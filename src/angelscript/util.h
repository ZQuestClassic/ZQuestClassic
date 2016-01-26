#ifndef _ZC_ANGELSCRIPT_UTIL_H_
#define _ZC_ANGELSCRIPT_UTIL_H_

#include <types.h>
class asIScriptObject;
class enemy;
struct mapscr;

// Various functions for scripts.

void asSFX(int sound, int x);
void asStopSFX(int sound);
bool asQuestRuleEnabled(int rule);
int getSpriteTile(int spr);
bool asIsWater(int pos);
void asSetGuyGridAt(int x, int y, int value);
void asChangeCounter(int id, short value);
void asChangeDCounter(int id, short value);
int asGetGeneric(int id);
void asSetLevelItem(int level, int item);
int asGetGravity();
int asGetTerminalVelocity();
void asDecGuyCount(int screen);

// mapscr access
mapscr* asGetTmpscr(int idx);
mapscr* asGetTmpscr2(int idx);
mapscr* asGetTmpscr3(int idx);

// Item stuff
int asGetItemProp(int id, int prop);
void asSetItem(int id, bool value);

// Weapon stuff
int asGetWeaponProp(int id, int prop);

// Combo stuff
int asGetComboProp(int id, int prop);
int asGetComboClassProp(int type, int prop);

// Sprite list access
asIScriptObject* asGetGuyObject(int index);
int asNumGuys();
int asGetGuyProp(int id, int prop);
void asSwapGuys(int index1, int index2);

asIScriptObject* asGetItemObject(int index);
int asNumItems();
void asDeleteItem(int index);

asIScriptObject* asGetLwpnObject(int index);
int asNumLwpns();

asIScriptObject* asGetEwpnObject(int index);
int asNumEwpns();
void asDeleteEwpn(int index);

void asPlaceItem(int x, int y, int id, int pickupFlags, int clk);

void asAddDecoration(int type, int x, int y);

void asMoveChainLinks(int dx, int dy);

// Input
bool asInputUp();
bool asInputDown();
bool asInputLeft();
bool asInputRight();
bool asCAbtn();
bool asCBbtn();
bool asCSbtn();
bool asCMbtn();
bool asCLbtn();
bool asCRbtn();
bool asCPbtn();
bool asCEx1btn();
bool asCEx2btn();
bool asCEx3btn();
bool asCEx4btn();
bool asInputAxisUp();
bool asInputAxisDown();
bool asInputAxisLeft();
bool asInputAxisRight();

bool asRUp();
bool asRDown();
bool asRLeft();
bool asRRight();
bool asRAbtn();
bool asRBbtn();
bool asRSbtn();
bool asRMbtn();
bool asRLbtn();
bool asRRbtn();
bool asRPbtn();
bool asREx1btn();
bool asREx2btn();
bool asREx3btn();
bool asREx4btn();
bool asRAxisUp();
bool asRAxisDown();
bool asRAxisLeft();
bool asRAxisRight();

#endif


#pragma once


class ScriptedEnemy : public enemy
{
public:


};


namespace ScriptEnemy
{
}

//TODO: This is not even close to done.
//many of these need to be replaced.

void ScriptRegistrar::RegisterScriptEnemy(asIScriptEngine* engine)
{
	int r;
	//using namespace ScriptEnemy;


	engine->RegisterObjectType("zcEnemy", sizeof(ScriptedEnemy), asOBJ_REF | asOBJ_NOCOUNT);
	engine->RegisterGlobalFunction("zcEnemy@ GetZCEnemy()", asFUNCTION(GetRealEnemy), asCALL_CDECL);

	// Sprite
	engine->RegisterObjectProperty("zcSprite", "int uid", asOFFSET(sprite, uid));
	engine->RegisterObjectProperty("zcSprite", "int x", asOFFSET(sprite, x));
	engine->RegisterObjectProperty("zcSprite", "int y", asOFFSET(sprite, y));
	engine->RegisterObjectProperty("zcSprite", "int z", asOFFSET(sprite, z));
	engine->RegisterObjectProperty("zcSprite", "int fall", asOFFSET(sprite, fall));
	engine->RegisterObjectProperty("zcSprite", "int tile", asOFFSET(sprite, tile));
	engine->RegisterObjectProperty("zcSprite", "int shadowtile", asOFFSET(sprite, shadowtile));
	engine->RegisterObjectProperty("zcSprite", "int cs", asOFFSET(sprite, cs));
	engine->RegisterObjectProperty("zcSprite", "int flip", asOFFSET(sprite, flip));
	engine->RegisterObjectProperty("zcSprite", "int c_clk", asOFFSET(sprite, c_clk));
	engine->RegisterObjectProperty("zcSprite", "int clk", asOFFSET(sprite, clk));
	engine->RegisterObjectProperty("zcSprite", "int misc", asOFFSET(sprite, misc));
	engine->RegisterObjectProperty("zcSprite", "int xofs", asOFFSET(sprite, xofs));
	engine->RegisterObjectProperty("zcSprite", "int yofs", asOFFSET(sprite, yofs));
	engine->RegisterObjectProperty("zcSprite", "int zofs", asOFFSET(sprite, zofs));
	engine->RegisterObjectProperty("zcSprite", "int hxofs", asOFFSET(sprite, hxofs));
	engine->RegisterObjectProperty("zcSprite", "int hyofs", asOFFSET(sprite, hyofs));
	engine->RegisterObjectProperty("zcSprite", "int hxsz", asOFFSET(sprite, hxsz));
	engine->RegisterObjectProperty("zcSprite", "int hysz", asOFFSET(sprite, hysz));
	engine->RegisterObjectProperty("zcSprite", "int hzsz", asOFFSET(sprite, hzsz));
	engine->RegisterObjectProperty("zcSprite", "int txsz", asOFFSET(sprite, txsz));
	engine->RegisterObjectProperty("zcSprite", "int tysz", asOFFSET(sprite, tysz));
	engine->RegisterObjectProperty("zcSprite", "int id", asOFFSET(sprite, id));
	engine->RegisterObjectProperty("zcSprite", "int dir", asOFFSET(sprite, dir));
	engine->RegisterObjectProperty("zcSprite", "bool angular", asOFFSET(sprite, angular));
	engine->RegisterObjectProperty("zcSprite", "bool canfreeze", asOFFSET(sprite, canfreeze));
	engine->RegisterObjectProperty("zcSprite", "double angle", asOFFSET(sprite, angle));
	engine->RegisterObjectProperty("zcSprite", "int lasthit", asOFFSET(sprite, lasthit));
	engine->RegisterObjectProperty("zcSprite", "int lasthitclk", asOFFSET(sprite, lasthitclk));
	engine->RegisterObjectProperty("zcSprite", "int drawstyle", asOFFSET(sprite, drawstyle));
	engine->RegisterObjectProperty("zcSprite", "int extend", asOFFSET(sprite, extend));
	engine->RegisterObjectProperty("zcSprite", "uint8 scriptcoldet", asOFFSET(sprite, scriptcoldet));

	// Enemy
	engine->RegisterObjectProperty("zcEnemy", "int o_tile", asOFFSET(ASEnemy, o_tile));
	engine->RegisterObjectProperty("zcEnemy", "int frate", asOFFSET(ASEnemy, frate));
	engine->RegisterObjectProperty("zcEnemy", "int hp", asOFFSET(ASEnemy, hp));
	engine->RegisterObjectProperty("zcEnemy", "int hclk", asOFFSET(ASEnemy, hclk));
	engine->RegisterObjectProperty("zcEnemy", "int stunclk", asOFFSET(ASEnemy, stunclk));
	engine->RegisterObjectProperty("zcEnemy", "int fading", asOFFSET(ASEnemy, fading));
	engine->RegisterObjectProperty("zcEnemy", "int superman", asOFFSET(ASEnemy, superman));
	engine->RegisterObjectProperty("zcEnemy", "bool mainguy", asOFFSET(ASEnemy, mainguy));
	engine->RegisterObjectProperty("zcEnemy", "bool did_armos", asOFFSET(ASEnemy, did_armos));
	engine->RegisterObjectProperty("zcEnemy", "uint8 item_set", asOFFSET(ASEnemy, item_set));
	engine->RegisterObjectProperty("zcEnemy", "uint8 grumble", asOFFSET(ASEnemy, grumble));
	engine->RegisterObjectProperty("zcEnemy", "bool itemguy", asOFFSET(ASEnemy, itemguy));
	engine->RegisterObjectProperty("zcEnemy", "bool count_enemy", asOFFSET(ASEnemy, count_enemy));
	engine->RegisterObjectProperty("zcEnemy", "int step", asOFFSET(ASEnemy, step));
	engine->RegisterObjectProperty("zcEnemy", "int floor_y", asOFFSET(ASEnemy, floor_y));
	engine->RegisterObjectProperty("zcEnemy", "bool dying", asOFFSET(ASEnemy, dying));
	engine->RegisterObjectProperty("zcEnemy", "bool ceiling", asOFFSET(ASEnemy, ceiling));
	engine->RegisterObjectProperty("zcEnemy", "bool leader", asOFFSET(ASEnemy, leader));
	engine->RegisterObjectProperty("zcEnemy", "bool scored", asOFFSET(ASEnemy, scored));
	engine->RegisterObjectProperty("zcEnemy", "bool script_spawned", asOFFSET(ASEnemy, script_spawned));
	engine->RegisterObjectProperty("zcEnemy", "uint flags", asOFFSET(ASEnemy, flags));
	engine->RegisterObjectProperty("zcEnemy", "uint flags2", asOFFSET(ASEnemy, flags2));
	engine->RegisterObjectProperty("zcEnemy", "int16 family", asOFFSET(ASEnemy, family));
	engine->RegisterObjectProperty("zcEnemy", "int16 dcset", asOFFSET(ASEnemy, dcset));
	engine->RegisterObjectProperty("zcEnemy", "int16 anim", asOFFSET(ASEnemy, anim));
	engine->RegisterObjectProperty("zcEnemy", "int16 dp", asOFFSET(ASEnemy, dp));
	engine->RegisterObjectProperty("zcEnemy", "int16 wpn", asOFFSET(ASEnemy, wpn));
	engine->RegisterObjectProperty("zcEnemy", "int16 rate", asOFFSET(ASEnemy, rate));
	engine->RegisterObjectProperty("zcEnemy", "int16 hrate", asOFFSET(ASEnemy, hrate));
	engine->RegisterObjectProperty("zcEnemy", "int16 homing", asOFFSET(ASEnemy, homing));
	engine->RegisterObjectProperty("zcEnemy", "int dstep", asOFFSET(ASEnemy, dstep));
	engine->RegisterObjectProperty("zcEnemy", "int dmisc1", asOFFSET(ASEnemy, dmisc1));
	engine->RegisterObjectProperty("zcEnemy", "int dmisc2", asOFFSET(ASEnemy, dmisc2));
	engine->RegisterObjectProperty("zcEnemy", "int dmisc3", asOFFSET(ASEnemy, dmisc3));
	engine->RegisterObjectProperty("zcEnemy", "int dmisc4", asOFFSET(ASEnemy, dmisc4));
	engine->RegisterObjectProperty("zcEnemy", "int dmisc5", asOFFSET(ASEnemy, dmisc5));
	engine->RegisterObjectProperty("zcEnemy", "int dmisc6", asOFFSET(ASEnemy, dmisc6));
	engine->RegisterObjectProperty("zcEnemy", "int dmisc7", asOFFSET(ASEnemy, dmisc7));
	engine->RegisterObjectProperty("zcEnemy", "int dmisc8", asOFFSET(ASEnemy, dmisc8));
	engine->RegisterObjectProperty("zcEnemy", "int dmisc9", asOFFSET(ASEnemy, dmisc9));
	engine->RegisterObjectProperty("zcEnemy", "int dmisc10", asOFFSET(ASEnemy, dmisc10));
	engine->RegisterObjectProperty("zcEnemy", "int16 bgsfx", asOFFSET(ASEnemy, bgsfx));
	engine->RegisterObjectProperty("zcEnemy", "int dummy_int1", asOFFSET(ASEnemy, dummy_int1));
	engine->RegisterObjectProperty("zcEnemy", "int clk2", asOFFSET(ASEnemy, clk2));
	engine->RegisterObjectProperty("zcEnemy", "int sclk", asOFFSET(ASEnemy, sclk));
	engine->RegisterObjectProperty("zcEnemy", "uint16 s_tile", asOFFSET(ASEnemy, s_tile));
	engine->RegisterObjectProperty("zcEnemy", "int scriptFlags", asOFFSET(ASEnemy, scriptFlags));
	engine->RegisterObjectProperty("zcEnemy", "bool haslink", asOFFSET(ASEnemy, haslink));

	engine->RegisterObjectMethod("zcEnemy", "void leave_item()", asMETHOD(ASEnemy, leave_item), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "bool canmove(int, int, int, int, int, int, int)", asMETHODPR(ASEnemy, canmove, (int, int, int, int, int, int, int), bool), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void fireWeapon()", asMETHOD(ASEnemy, fireWeapon), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "bool isFiring() const", asMETHOD(ASEnemy, isFiring), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void updateFiring()", asMETHOD(ASEnemy, updateFiring), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void setBreathTimer(int)", asMETHOD(ASEnemy, setBreathTimer), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void setAttackOwner(int)", asMETHOD(ASEnemy, setAttackOwner), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void activateDeathAttack()", asMETHOD(ASEnemy, activateDeathAttack), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void draw()", asMETHOD(ASEnemy, scriptDraw), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void drawblock()", asMETHOD(ASEnemy, scriptDrawBlock), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void drawshadow()", asMETHOD(ASEnemy, scriptDrawShadow), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void masked_draw(int, int, int, int)", asMETHOD(ASEnemy, scriptMaskedDraw), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void overtilecloaked16(int, int, int, int)", asMETHOD(ASEnemy, overTileCloaked16), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void overtile16(int, int, int, int, int)", asMETHOD(ASEnemy, overTile16), asCALL_THISCALL);

	engine->RegisterObjectMethod("zcEnemy", "void stopBGSFX()", asMETHOD(ASEnemy, stopBGSFX), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void playDeathSFX()", asMETHOD(ASEnemy, playDeathSFX), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "void playHitSFX(int)", asMETHOD(ASEnemy, playHitSFX), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "int defenditemclass(int, int)", asMETHOD(ASEnemy, scriptDefendItemClass), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "int getDefendedItemPower()", asMETHOD(ASEnemy, getDefendedItemPower), asCALL_THISCALL);
	engine->RegisterObjectMethod("zcEnemy", "int takehit(__RealWeapon@)", asMETHOD(ASEnemy, defaultTakeHit), asCALL_THISCALL);


}


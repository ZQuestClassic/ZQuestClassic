
#pragma once


struct sNPC
{

	//////////////////////////////////////////////////////////////////////////
	/// Setters
	//////////////////////////////////////////////////////////////////////////

	void SetID(int32 value)
	{
		// todo
	}

	void SetType(int32 value)
	{
		// todo
	}

	void SetX(int32 value)
	{
		// todo
	}

	void SetY(int32 value)
	{
		// todo
	}

	void SetZ(int32 value)
	{
		// todo
	}

	void SetJump(int32 value)
	{
		// todo
	}

	void SetDir(int32 value)
	{
		// todo
	}

	void SetRate(int32 value)
	{
		// todo
	}

	void SetHaltrate(int32 value)
	{
		// todo
	}

	void SetHoming(int32 value)
	{
		// todo
	}

	void SetHunger(int32 value)
	{
		// todo
	}

	void SetStep(int32 value)
	{
		// todo
	}

	void SetCollDetection(bool value)
	{
		// todo
	}

	void SetASpeed(int32 value)
	{
		// todo
	}

	void SetDrawStyle(int32 value)
	{
		// todo
	}

	void SetHP(int32 value)
	{
		// todo
	}

	void SetDamage(int32 value)
	{
		// todo
	}

	void SetWeaponDamage(int32 value)
	{
		// todo
	}

	void SetStun(int32 value)
	{
		// todo
	}

	void SetOriginalTile(int32 value)
	{
		// todo
	}

	void SetTile(int32 value)
	{
		// todo
	}

	void SetWeapon(int32 value)
	{
		// todo
	}

	void SetItemSet(int32 value)
	{
		// todo
	}

	void SetCSet(int32 value)
	{
		// todo
	}

	void SetBossPal(int32 value)
	{
		// todo
	}

	void SetSFX(int32 value)
	{
		// todo
	}

	void SetExtend(int32 value)
	{
		// todo
	}

	void SetTileWidth(int32 value)
	{
		// todo
	}

	void SetTileHeight(int32 value)
	{
		// todo
	}

	void SetHitWidth(int32 value)
	{
		// todo
	}

	void SetHitHeight(int32 value)
	{
		// todo
	}

	void SetHitZHeight(int32 value)
	{
		// todo
	}

	void SetHitXOffset(int32 value)
	{
		// todo
	}

	void SetHitYOffset(int32 value)
	{
		// todo
	}

	void SetDrawXOffset(int32 value)
	{
		// todo
	}

	void SetDrawYOffset(int32 value)
	{
		// todo
	}

	void SetDrawZOffset(int32 value)
	{
		// todo
	}

	void SetDefense(int32 i, int32 value)
	{
		// todo
	}

	void SetAttributes(int32 i, int32 value)
	{
		// todo
	}

	void SetMiscFlags(int32 value)
	{
		// todo
	}

	void SetMisc(int32 i, int32 value)
	{
		// todo
	}

	void SetBreakShield()(void value)
	{
		// todo
	}


	//////////////////////////////////////////////////////////////////////////
	/// Getters
	//////////////////////////////////////////////////////////////////////////

	bool IsValid()
	{
		return isValid();
	}

	ScriptString GetName()
	{
		return GetName(int32 buffer;
	}

	int32 GetID()
	{
		return ID;
	}

	int32 GetType()
	{
		return Type;
	}

	int32 GetX()
	{
		return X;
	}

	int32 GetY()
	{
		return Y;
	}

	int32 GetZ()
	{
		return Z;
	}

	int32 GetJump()
	{
		return Jump;
	}

	int32 GetDir()
	{
		return Dir;
	}

	int32 GetRate()
	{
		return Rate;
	}

	int32 GetHaltrate()
	{
		return Haltrate;
	}

	int32 GetHoming()
	{
		return Homing;
	}

	int32 GetHunger()
	{
		return Hunger;
	}

	int32 GetStep()
	{
		return Step;
	}

	bool GetCollDetection()
	{
		return CollDetection;
	}

	int32 GetASpeed()
	{
		return ASpeed;
	}

	int32 GetDrawStyle()
	{
		return DrawStyle;
	}

	int32 GetHP()
	{
		return HP;
	}

	int32 GetDamage()
	{
		return Damage;
	}

	int32 GetWeaponDamage()
	{
		return WeaponDamage;
	}

	int32 GetStun()
	{
		return Stun;
	}

	int32 GetOriginalTile()
	{
		return OriginalTile;
	}

	int32 GetTile()
	{
		return Tile;
	}

	int32 GetWeapon()
	{
		return Weapon;
	}

	int32 GetItemSet()
	{
		return ItemSet;
	}

	int32 GetCSet()
	{
		return CSet;
	}

	int32 GetBossPal()
	{
		return BossPal;
	}

	int32 GetSFX()
	{
		return SFX;
	}

	int32 GetExtend()
	{
		return Extend;
	}

	int32 GetTileWidth()
	{
		return TileWidth;
	}

	int32 GetTileHeight()
	{
		return TileHeight;
	}

	int32 GetHitWidth()
	{
		return HitWidth;
	}

	int32 GetHitHeight()
	{
		return HitHeight;
	}

	int32 GetHitZHeight()
	{
		return HitZHeight;
	}

	int32 GetHitXOffset()
	{
		return HitXOffset;
	}

	int32 GetHitYOffset()
	{
		return HitYOffset;
	}

	int32 GetDrawXOffset()
	{
		return DrawXOffset;
	}

	int32 GetDrawYOffset()
	{
		return DrawYOffset;
	}

	int32 GetDrawZOffset()
	{
		return DrawZOffset;
	}

	int32 GetDefense(int32 i)
	{
		return Defense;
	}

	int32 GetAttributes(int32 i)
	{
		return Attributes;
	}

	int32 GetMiscFlags()
	{
		return MiscFlags;
	}

	int32 GetMisc(int32 i)
	{
		return Misc;
	}

}




void ScriptRegistrar::RegisterLegacyFFC(asIScriptEngine* engine)
{
	int r;

	r = engine->RegisterObjectType("NPC", sizeof(int32), asOBJ_APP_PRIMITIVE); Assert(r >= 0);

	// Setters
	r = engine->RegisterObjectMethod("NPC", "bool get_isValid()", asMETHOD(sNPC, GetisValid()), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "void get_GetName(int32 buffer(int)", asMETHOD(sNPC, GetGetName(int32 buffer), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_ID()", asMETHOD(sNPC, GetID), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Type()", asMETHOD(sNPC, GetType), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_X()", asMETHOD(sNPC, GetX), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Y()", asMETHOD(sNPC, GetY), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Z()", asMETHOD(sNPC, GetZ), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Jump()", asMETHOD(sNPC, GetJump), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Dir()", asMETHOD(sNPC, GetDir), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Rate()", asMETHOD(sNPC, GetRate), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Haltrate()", asMETHOD(sNPC, GetHaltrate), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Homing()", asMETHOD(sNPC, GetHoming), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Hunger()", asMETHOD(sNPC, GetHunger), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Step()", asMETHOD(sNPC, GetStep), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "bool get_CollDetection()", asMETHOD(sNPC, GetCollDetection), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_ASpeed()", asMETHOD(sNPC, GetASpeed), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_DrawStyle()", asMETHOD(sNPC, GetDrawStyle), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HP()", asMETHOD(sNPC, GetHP), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Damage()", asMETHOD(sNPC, GetDamage), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_WeaponDamage()", asMETHOD(sNPC, GetWeaponDamage), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Stun()", asMETHOD(sNPC, GetStun), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_OriginalTile()", asMETHOD(sNPC, GetOriginalTile), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Tile()", asMETHOD(sNPC, GetTile), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Weapon()", asMETHOD(sNPC, GetWeapon), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_ItemSet()", asMETHOD(sNPC, GetItemSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_CSet()", asMETHOD(sNPC, GetCSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_BossPal()", asMETHOD(sNPC, GetBossPal), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_SFX()", asMETHOD(sNPC, GetSFX), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Extend()", asMETHOD(sNPC, GetExtend), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_TileWidth()", asMETHOD(sNPC, GetTileWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_TileHeight()", asMETHOD(sNPC, GetTileHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HitWidth()", asMETHOD(sNPC, GetHitWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HitHeight()", asMETHOD(sNPC, GetHitHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HitZHeight()", asMETHOD(sNPC, GetHitZHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HitXOffset()", asMETHOD(sNPC, GetHitXOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HitYOffset()", asMETHOD(sNPC, GetHitYOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_DrawXOffset()", asMETHOD(sNPC, GetDrawXOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_DrawYOffset()", asMETHOD(sNPC, GetDrawYOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_DrawZOffset()", asMETHOD(sNPC, GetDrawZOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Defense(int)", asMETHOD(sNPC, GetDefense), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Attributes(int)", asMETHOD(sNPC, GetAttributes), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_MiscFlags()", asMETHOD(sNPC, GetMiscFlags), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Misc(int)", asMETHOD(sNPC, GetMisc), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "void get_BreakShield()", asMETHOD(sNPC, GetBreakShield()), asCALL_THISCALL); Assert(r >= 0);

	// Getters
	r = engine->RegisterObjectMethod("NPC", "bool get_isValid()", asMETHOD(sNPC, GetisValid()), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "void get_GetName(int32 buffer(int)", asMETHOD(sNPC, GetGetName(int32 buffer), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_ID()", asMETHOD(sNPC, GetID), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Type()", asMETHOD(sNPC, GetType), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_X()", asMETHOD(sNPC, GetX), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Y()", asMETHOD(sNPC, GetY), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Z()", asMETHOD(sNPC, GetZ), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Jump()", asMETHOD(sNPC, GetJump), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Dir()", asMETHOD(sNPC, GetDir), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Rate()", asMETHOD(sNPC, GetRate), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Haltrate()", asMETHOD(sNPC, GetHaltrate), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Homing()", asMETHOD(sNPC, GetHoming), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Hunger()", asMETHOD(sNPC, GetHunger), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Step()", asMETHOD(sNPC, GetStep), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "bool get_CollDetection()", asMETHOD(sNPC, GetCollDetection), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_ASpeed()", asMETHOD(sNPC, GetASpeed), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_DrawStyle()", asMETHOD(sNPC, GetDrawStyle), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HP()", asMETHOD(sNPC, GetHP), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Damage()", asMETHOD(sNPC, GetDamage), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_WeaponDamage()", asMETHOD(sNPC, GetWeaponDamage), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Stun()", asMETHOD(sNPC, GetStun), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_OriginalTile()", asMETHOD(sNPC, GetOriginalTile), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Tile()", asMETHOD(sNPC, GetTile), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Weapon()", asMETHOD(sNPC, GetWeapon), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_ItemSet()", asMETHOD(sNPC, GetItemSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_CSet()", asMETHOD(sNPC, GetCSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_BossPal()", asMETHOD(sNPC, GetBossPal), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_SFX()", asMETHOD(sNPC, GetSFX), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Extend()", asMETHOD(sNPC, GetExtend), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_TileWidth()", asMETHOD(sNPC, GetTileWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_TileHeight()", asMETHOD(sNPC, GetTileHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HitWidth()", asMETHOD(sNPC, GetHitWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HitHeight()", asMETHOD(sNPC, GetHitHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HitZHeight()", asMETHOD(sNPC, GetHitZHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HitXOffset()", asMETHOD(sNPC, GetHitXOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_HitYOffset()", asMETHOD(sNPC, GetHitYOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_DrawXOffset()", asMETHOD(sNPC, GetDrawXOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_DrawYOffset()", asMETHOD(sNPC, GetDrawYOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_DrawZOffset()", asMETHOD(sNPC, GetDrawZOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Defense(int)", asMETHOD(sNPC, GetDefense), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Attributes(int)", asMETHOD(sNPC, GetAttributes), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_MiscFlags()", asMETHOD(sNPC, GetMiscFlags), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "int32 get_Misc(int)", asMETHOD(sNPC, GetMisc), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("NPC", "void get_BreakShield()", asMETHOD(sNPC, GetBreakShield()), asCALL_THISCALL); Assert(r >= 0);


}


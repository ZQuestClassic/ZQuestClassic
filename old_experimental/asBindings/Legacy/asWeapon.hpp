
#pragma once


struct sWeapon
{
	int32 uid;

	//////////////////////////////////////////////////////////////////////////
	/// Setters
	//////////////////////////////////////////////////////////////////////////

	void SetUseSprite(int32 id, void value)
	{
		// todo
	}

	void SetBehind(bool value)
	{
		// todo
	}

	void SetID(int32 value)
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

	void SetDrawStyle(int32 value)
	{
		// todo
	}

	void SetDir(int32 value)
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

	void SetOriginalCSet(int32 value)
	{
		// todo
	}

	void SetCSet(int32 value)
	{
		// todo
	}

	void SetFlashCSet(int32 value)
	{
		// todo
	}

	void SetNumFrames(int32 value)
	{
		// todo
	}

	void SetFrame(int32 value)
	{
		// todo
	}

	void SetASpeed(int32 value)
	{
		// todo
	}

	void SetDamage(int32 value)
	{
		// todo
	}

	void SetStep(int32 value)
	{
		// todo
	}

	void SetAngle(int32 value)
	{
		// todo
	}

	void SetAngular(bool value)
	{
		// todo
	}

	void SetCollDetection(bool value)
	{
		// todo
	}

	void SetDeadState(int32 value)
	{
		// todo
	}

	void SetFlash(bool value)
	{
		// todo
	}

	void SetFlip(int32 value)
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

	void SetMisc(int32 i, int32 value)
	{
		// todo
	}


	//////////////////////////////////////////////////////////////////////////
	/// Getters
	//////////////////////////////////////////////////////////////////////////

	bool GetisValid()()
	{
		return isValid();
	}

	void GetUseSprite(int32 id)()
	{
		return UseSprite(int32 id);
	}

	bool GetBehind()
	{
		return Behind;
	}

	int32 GetID()
	{
		return ID;
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

	int32 GetDrawStyle()
	{
		return DrawStyle;
	}

	int32 GetDir()
	{
		return Dir;
	}

	int32 GetOriginalTile()
	{
		return OriginalTile;
	}

	int32 GetTile()
	{
		return Tile;
	}

	int32 GetOriginalCSet()
	{
		return OriginalCSet;
	}

	int32 GetCSet()
	{
		return CSet;
	}

	int32 GetFlashCSet()
	{
		return FlashCSet;
	}

	int32 GetNumFrames()
	{
		return NumFrames;
	}

	int32 GetFrame()
	{
		return Frame;
	}

	int32 GetASpeed()
	{
		return ASpeed;
	}

	int32 GetDamage()
	{
		return Damage;
	}

	int32 GetStep()
	{
		return Step;
	}

	int32 GetAngle()
	{
		return Angle;
	}

	bool GetAngular()
	{
		return Angular;
	}

	bool GetCollDetection()
	{
		return CollDetection;
	}

	int32 GetDeadState()
	{
		return DeadState;
	}

	bool GetFlash()
	{
		return Flash;
	}

	int32 GetFlip()
	{
		return Flip;
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

	int32 GetMisc(int32 i)
	{
		return Misc;
	}

};



void ScriptRegistrar::RegisterLegacyWeapon(asIScriptEngine* engine)
{
	int r;
	using namespace LinkBindings;

	r = engine->RegisterObjectType("Weapon", sizeof(int32), asOBJ_APP_PRIMITIVE); Assert(r >= 0);

	// Setters
	r = engine->RegisterObjectMethod("Weapon", "void set_UseSprite(int32 id)()", asMETHOD(sWeapon, SetUseSprite(int32 id)), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Behind()", asMETHOD(sWeapon, SetBehind), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_ID()", asMETHOD(sWeapon, SetID), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_X()", asMETHOD(sWeapon, SetX), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Y()", asMETHOD(sWeapon, SetY), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Z()", asMETHOD(sWeapon, SetZ), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Jump()", asMETHOD(sWeapon, SetJump), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_DrawStyle()", asMETHOD(sWeapon, SetDrawStyle), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Dir()", asMETHOD(sWeapon, SetDir), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_OriginalTile()", asMETHOD(sWeapon, SetOriginalTile), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Tile()", asMETHOD(sWeapon, SetTile), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_OriginalCSet()", asMETHOD(sWeapon, SetOriginalCSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_CSet()", asMETHOD(sWeapon, SetCSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_FlashCSet()", asMETHOD(sWeapon, SetFlashCSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_NumFrames()", asMETHOD(sWeapon, SetNumFrames), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Frame()", asMETHOD(sWeapon, SetFrame), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_ASpeed()", asMETHOD(sWeapon, SetASpeed), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Damage()", asMETHOD(sWeapon, SetDamage), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Step()", asMETHOD(sWeapon, SetStep), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Angle()", asMETHOD(sWeapon, SetAngle), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Angular()", asMETHOD(sWeapon, SetAngular), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_CollDetection()", asMETHOD(sWeapon, SetCollDetection), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_DeadState()", asMETHOD(sWeapon, SetDeadState), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Flash()", asMETHOD(sWeapon, SetFlash), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Flip()", asMETHOD(sWeapon, SetFlip), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Extend()", asMETHOD(sWeapon, SetExtend), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_TileWidth()", asMETHOD(sWeapon, SetTileWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_TileHeight()", asMETHOD(sWeapon, SetTileHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_HitWidth()", asMETHOD(sWeapon, SetHitWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_HitHeight()", asMETHOD(sWeapon, SetHitHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_HitZHeight()", asMETHOD(sWeapon, SetHitZHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_HitXOffset()", asMETHOD(sWeapon, SetHitXOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_HitYOffset()", asMETHOD(sWeapon, SetHitYOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_DrawXOffset()", asMETHOD(sWeapon, SetDrawXOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_DrawYOffset()", asMETHOD(sWeapon, SetDrawYOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_DrawZOffset()", asMETHOD(sWeapon, SetDrawZOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void set_Misc(int)", asMETHOD(sWeapon, SetMisc), asCALL_THISCALL); Assert(r >= 0);


	// Getters
	r = engine->RegisterObjectMethod("Weapon", "bool get_isValid()()", asMETHOD(sWeapon, GetisValid()), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "void get_UseSprite(int32 id)()", asMETHOD(sWeapon, GetUseSprite(int32 id)), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "bool get_Behind()", asMETHOD(sWeapon, GetBehind), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_ID()", asMETHOD(sWeapon, GetID), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_X()", asMETHOD(sWeapon, GetX), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Y()", asMETHOD(sWeapon, GetY), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Z()", asMETHOD(sWeapon, GetZ), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Jump()", asMETHOD(sWeapon, GetJump), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_DrawStyle()", asMETHOD(sWeapon, GetDrawStyle), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Dir()", asMETHOD(sWeapon, GetDir), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_OriginalTile()", asMETHOD(sWeapon, GetOriginalTile), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Tile()", asMETHOD(sWeapon, GetTile), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_OriginalCSet()", asMETHOD(sWeapon, GetOriginalCSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_CSet()", asMETHOD(sWeapon, GetCSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_FlashCSet()", asMETHOD(sWeapon, GetFlashCSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_NumFrames()", asMETHOD(sWeapon, GetNumFrames), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Frame()", asMETHOD(sWeapon, GetFrame), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_ASpeed()", asMETHOD(sWeapon, GetASpeed), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Damage()", asMETHOD(sWeapon, GetDamage), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Step()", asMETHOD(sWeapon, GetStep), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Angle()", asMETHOD(sWeapon, GetAngle), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "bool get_Angular()", asMETHOD(sWeapon, GetAngular), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "bool get_CollDetection()", asMETHOD(sWeapon, GetCollDetection), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_DeadState()", asMETHOD(sWeapon, GetDeadState), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "bool get_Flash()", asMETHOD(sWeapon, GetFlash), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Flip()", asMETHOD(sWeapon, GetFlip), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Extend()", asMETHOD(sWeapon, GetExtend), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_TileWidth()", asMETHOD(sWeapon, GetTileWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_TileHeight()", asMETHOD(sWeapon, GetTileHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_HitWidth()", asMETHOD(sWeapon, GetHitWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_HitHeight()", asMETHOD(sWeapon, GetHitHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_HitZHeight()", asMETHOD(sWeapon, GetHitZHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_HitXOffset()", asMETHOD(sWeapon, GetHitXOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_HitYOffset()", asMETHOD(sWeapon, GetHitYOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_DrawXOffset()", asMETHOD(sWeapon, GetDrawXOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_DrawYOffset()", asMETHOD(sWeapon, GetDrawYOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_DrawZOffset()", asMETHOD(sWeapon, GetDrawZOffset), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("Weapon", "int32 get_Misc(int)", asMETHOD(sWeapon, GetMisc), asCALL_THISCALL); Assert(r >= 0);

}



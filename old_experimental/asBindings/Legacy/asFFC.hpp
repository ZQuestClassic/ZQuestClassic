
#pragma once


struct sFFC
{

	//////////////////////////////////////////////////////////////////////////
	/// Setters
	//////////////////////////////////////////////////////////////////////////

	void SetData(int32 value)
	{
		// todo
	}

	void SetScript(int32 value)
	{
		// todo
	}

	void SetCSet(int32 value)
	{
		// todo
	}

	void SetDelay(int32 value)
	{
		// todo
	}

	void SetX(float value)
	{
		// todo
	}

	void SetY(float value)
	{
		// todo
	}

	void SetVx(float value)
	{
		// todo
	}

	void SetVy(float value)
	{
		// todo
	}

	void SetAx(float value)
	{
		// todo
	}

	void SetAy(float value)
	{
		// todo
	}

	void SetFlags(int32 i, bool value)
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

	void SetEffectWidth(int32 value)
	{
		// todo
	}

	void SetEffectHeight(int32 value)
	{
		// todo
	}

	void SetLink(int32 value)
	{
		// todo
	}

	void SetInitD(int32 i, float value)
	{
		// todo
	}

	void SetMisc(int32 i, float value)
	{
		// todo
	}


	//////////////////////////////////////////////////////////////////////////
	/// Getters
	//////////////////////////////////////////////////////////////////////////

	int32 GetData()
	{
		return Data;
	}

	int32 GetScript()
	{
		return Script;
	}

	int32 GetCSet()
	{
		return CSet;
	}

	int32 GetDelay()
	{
		return Delay;
	}

	float GetX()
	{
		return X;
	}

	float GetY()
	{
		return Y;
	}

	float GetVx()
	{
		return Vx;
	}

	float GetVy()
	{
		return Vy;
	}

	float GetAx()
	{
		return Ax;
	}

	float GetAy()
	{
		return Ay;
	}

	bool GetFlags(int32 i)
	{
		return Flags;
	}

	int32 GetTileWidth()
	{
		return TileWidth;
	}

	int32 GetTileHeight()
	{
		return TileHeight;
	}

	int32 GetEffectWidth()
	{
		return EffectWidth;
	}

	int32 GetEffectHeight()
	{
		return EffectHeight;
	}

	int32 GetLink()
	{
		return Link;
	}

	float GetInitD(int32 i)
	{
		return InitD;
	}

	float GetMisc(int32 i)
	{
		return Misc;
	}

};


void ScriptRegistrar::RegisterLegacyFFC(asIScriptEngine* engine)
{
	int r;

	r = engine->RegisterObjectType("FFC", sizeof(int32), asOBJ_APP_PRIMITIVE); Assert(r >= 0);

	// Setters
	r = engine->RegisterObjectMethod("sFFC", "void set_Data()", asMETHOD(sFFC, SetData), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_Script()", asMETHOD(sFFC, SetScript), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_CSet()", asMETHOD(sFFC, SetCSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_Delay()", asMETHOD(sFFC, SetDelay), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_X()", asMETHOD(sFFC, SetX), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_Y()", asMETHOD(sFFC, SetY), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_Vx()", asMETHOD(sFFC, SetVx), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_Vy()", asMETHOD(sFFC, SetVy), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_Ax()", asMETHOD(sFFC, SetAx), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_Ay()", asMETHOD(sFFC, SetAy), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_Flags(int)", asMETHOD(sFFC, SetFlags), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_TileWidth()", asMETHOD(sFFC, SetTileWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_TileHeight()", asMETHOD(sFFC, SetTileHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_EffectWidth()", asMETHOD(sFFC, SetEffectWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_EffectHeight()", asMETHOD(sFFC, SetEffectHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_Link()", asMETHOD(sFFC, SetLink), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_InitD(int)", asMETHOD(sFFC, SetInitD), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "void set_Misc(int)", asMETHOD(sFFC, SetMisc), asCALL_THISCALL); Assert(r >= 0);

	// Getters
	r = engine->RegisterObjectMethod("sFFC", "int32 get_Data()", asMETHOD(sFFC, GetData), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "int32 get_Script()", asMETHOD(sFFC, GetScript), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "int32 get_CSet()", asMETHOD(sFFC, GetCSet), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "int32 get_Delay()", asMETHOD(sFFC, GetDelay), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "float get_X()", asMETHOD(sFFC, GetX), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "float get_Y()", asMETHOD(sFFC, GetY), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "float get_Vx()", asMETHOD(sFFC, GetVx), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "float get_Vy()", asMETHOD(sFFC, GetVy), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "float get_Ax()", asMETHOD(sFFC, GetAx), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "float get_Ay()", asMETHOD(sFFC, GetAy), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "bool get_Flags(int)", asMETHOD(sFFC, GetFlags), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "int32 get_TileWidth()", asMETHOD(sFFC, GetTileWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "int32 get_TileHeight()", asMETHOD(sFFC, GetTileHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "int32 get_EffectWidth()", asMETHOD(sFFC, GetEffectWidth), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "int32 get_EffectHeight()", asMETHOD(sFFC, GetEffectHeight), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "int32 get_Link()", asMETHOD(sFFC, GetLink), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "float get_InitD(int)", asMETHOD(sFFC, GetInitD), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("sFFC", "float get_Misc(int)", asMETHOD(sFFC, GetMisc), asCALL_THISCALL); Assert(r >= 0);


}




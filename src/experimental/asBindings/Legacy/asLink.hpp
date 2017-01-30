
#pragma once



namespace LinkBindings
{
	LinkClass Link;//remove me;

	int32 GetX()
	{
		return (int32)Link.getX();
	}

	void SetX(int32 value)
	{
		return Link.setX(value);
	}

	int32 GetY()
	{
		return (int32)Link.getY();
	}

	void SetY(int32 value)
	{
		return Link.setY(value);
	}

	int32 GetZ()
	{
		return (int32)Link.getZ();
	}

	void SetZ(int32 value)
	{
		return Link.setZ(value);
	}

	bool GetInvisible()
	{
		return Link.getDontDraw();
	}

	bool CollDetection()
	{
		return Link.scriptcoldet;
	}

	int32 GetJump()
	{
		return int32(-Link.getFall()) * 100;
	}

	int32 SwordJinx()
	{
		return Link.getSwordClk();
	}

	int32 ItemJinx()
	{
		return Link.getItemClk();
	}

	int32 Drunk()
	{
		return Link.DrunkClock();
	}

	int32 GetDir()
	{
		return Link.dir;
	}

	void SetDir(int32 value)
	{
		return Link.setDir(value);
	}

	int32 HitDir()
	{
		//todo
	}

	int32 GetHP()
	{
		return game->get_life();
	}

	void SetHP(int32 value)
	{
		game->set_life(value);
	}

	int32 GetMP()
	{
		return game->get_magic();
	}

	void SetMP(int32 value)
	{
		game->set_magic(value);
	}

	int32 GetMaxHP()
	{
		return game->get_maxlife();
	}

	void SetMaxHP(int32 value)
	{
		game->set_maxlife(value);
	}

	int32 GetMaxMP()
	{
		return game->get_maxmagic();
	}

	void SetMaxMP(int32 value)
	{
		game->set_maxmagic(value);
	}


	int32 GetAction()
	{
		return Link.getAction();
	}

	void SetAction(int32 value)
	{
		Link.setAction(value);
	}

	int32 GetHeldItem()
	{
		return Link.getHeldItem();
	}

	void SetAction(int32 value)
	{
		Link.setHeldItem(value);
	}

	int32 GetLadderX()
	{
		return Link.getLadderX();
	}

	void SetLadderX(int32 value)
	{
		Link.setL(value);
	}

	int32 GetLadderY()
	{
		return Link.getLadderY();
	}

	void SetLadderY(int32 value)
	{
		Link.setL(value);
	}

	bool GetItem(int32 i)
	{
		return game->item[Clamp(i, 0, MAXITEMS-1)];
	}

	void SetItem(int32 i, bool value)
	{
		game->item[Clamp(i, 0, MAXITEMS-1)] = value;
	}

	int32 GetEquipment()
	{
		return ((Awpn&0xFF)|((Bwpn&0xFF)<<8));
	}

	void SetEquipment(int32 value)
	{
		Link.SetEquipment(value);
	}

	int32 GetTile()
	{
		return Link.tile;
	}

	void SetTile(int32 value)
	{
		Link.tile = value;
	}

	int32 GetFlip()
	{
		return Link.flip;
	}

	void SetFlip(int32 value)
	{
		Link.flip = value;
	}

	int32 GetHitZHeight()
	{
		return Link.GetHitZHeight();
	}

	void SetHitZHeight(int32 value)
	{
		Link.SetHitZHeight(value);
	}

	int32 GetHitXOffset()
	{
		return Link.hxofs;
	}

	void SetHitXOffset(int32 value)
	{
		Link.hxofs = value;
	}

	int32 GetHitYOffset()
	{
		return Link.hyofs;
	}

	void SetHitYOffset(int32 value)
	{
		Link.hyofs = value;
	}

	int32 DrawXOffset()
	{
		//todo
	}

	int32 DrawYOffset()
	{
		//todo
	}

	int32 DrawZOffset()
	{
		//todo
	}

	int32 GetMisc(int32 i)
	{
		return Link.miscellaneous[i & 15];
	}

	void SetMisc(int32 i, int32 value)
	{
		Link.miscellaneous[i & 15] = value;
	}

	void Warp(int32 DMap, int32 screen)
	{
		Link.dowarp(DMap, screen);
	}

	void PitWarp(int32 DMap, int32 screen)
	{
		Link.dowarp(DMap, screen);
	}

	void SelectAWeapon(int32 dir)
	{
		//todo
	}

	void SelectBWeapon(int32 dir)
	{
		//todo
	}

}



void ScriptRegistrar::RegisterLegacyLink(asIScriptEngine* engine)
{
	int r;
	using namespace LinkBindings;

	engine->SetDefaultNamespace("Link");

	// Getters
	r = engine->RegisterGlobalFunction("int get_X()", asFUNCTION(GetX), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Y()", asFUNCTION(GetY), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Z()", asFUNCTION(GetZ), asCALL_CDECL); Assert(r >= 0);

	// Setters
	r = engine->RegisterGlobalFunction("void set_X()", asFUNCTION(SetX), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Y()", asFUNCTION(SetY), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Z()", asFUNCTION(SetZ), asCALL_CDECL); Assert(r >= 0);


	engine->SetDefaultNamespace("");
}






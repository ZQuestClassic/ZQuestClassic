
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

	void SetInvisible(bool value)
	{
		return Link.setDontDraw(value);
	}

	bool GetCollDetection()
	{
		return Link.scriptcoldet;
	}

	void SetCollDetection(bool value)
	{
		Link.scriptcoldet = value;
	}

	int32 GetJump()
	{
		return int32(-Link.getFall()) * 100;
	}

	void SetJump(int32 value)
	{
		Link.setFall(fix((-value * (100.0)));
	}

	int32 GetSwordJinx()
	{
		return Link.getSwordClk();
	}

	void SetSwordJinx(int32 value)
	{
		return Link.setSwordClk(value);
	}

	int32 GetItemJinx()
	{
		return Link.getItemClk();
	}

	void SetItemJinx(int32 value)
	{
		return Link.setItemClk(value);
	}

	int32 GetDrunk()
	{
		return Link.DrunkClock();
	}

	void SetDrunk(int32 value)
	{
		return Link.setDrunkClock(value);
	}

	int32 GetDir()
	{
		return Link.dir;
	}

	void SetDir(int32 value)
	{
		//Link.setDir() calls reset_hookshot(), which removes the sword sprite.. O_o
		if(Link.getAction() == attacking)
			Link.dir = value;

		else Link.setDir(value);
	}

	int32 GetHitDir()
	{
		return Link.getHitDir();
	}

	void SetHitDir(int32 value)
	{
		Link.setHitDir(value);
	}

	int32 GetHP()
	{
		return game->get_life();
	}

	void SetHP(int32 value)
	{
		game->set_life(Clamp(value, 0, game->get_maxlife()));
	}

	int32 GetMP()
	{
		return game->get_magic();
	}

	void SetMP(int32 value)
	{
		game->set_magic(Clamp(value, 0, game->get_maxmagic()));
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

	void SetHeldItem(int32 value)
	{
		Link.setHeldItem(Clamp(value, 0, MAXITEMS - 1));
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
		return Link.getHitZHeight();
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

	// Input
	bool GetInputState(int32 i)
	{
		return control_state[Min(i, 17)];
	};

	void SetInputState(int32 i, bool value)
	{
		control_state[Min(i, 17)] = value;
	};

	bool GetInputPress(int32 i)
	{
		return button_press[Min(i, 17)];
	};

	void SetInputPress(int32 i, bool value)
	{
		button_press[Min(i, 17)] = value;
	};

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
	r = engine->RegisterGlobalFunction("int get_Invisible()", asFUNCTION(GetInvisible), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_CollDetection()", asFUNCTION(GetCollDetection), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Jump()", asFUNCTION(GetJump), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_SwordJinx()", asFUNCTION(GetSwordJinx), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_ItemJinx()", asFUNCTION(GetItemJinx), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Drunk()", asFUNCTION(GetDrunk), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Dir()", asFUNCTION(GetDir), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_HitDir()", asFUNCTION(GetHitDir), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_HP()", asFUNCTION(GetHP), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_MP()", asFUNCTION(GetMP), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_MaxHP()", asFUNCTION(GetMaxHP), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_MaxMP()", asFUNCTION(GetMaxMP), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Action()", asFUNCTION(GetAction), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_HeldItem()", asFUNCTION(GetHeldItem), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_LadderX()", asFUNCTION(GetLadderX), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_LadderY()", asFUNCTION(GetLadderY), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("bool get_Item(int)", asFUNCTION(GetItem), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Equipment()", asFUNCTION(GetEquipment), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Tile()", asFUNCTION(GetTile), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Flip()", asFUNCTION(GetFlip), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_HitZHeight()", asFUNCTION(GetHitZHeight), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_HitXOffset()", asFUNCTION(GetHitXOffset), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_HitYOffset()", asFUNCTION(GetHitYOffset), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_DrawXOffset()", asFUNCTION(GetDrawXOffset), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_DrawYOffset()", asFUNCTION(GetDrawYOffset), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_DrawZOffset()", asFUNCTION(GetDrawZOffset), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Misc(int)", asFUNCTION(GetMisc), asCALL_CDECL); Assert(r >= 0);


	// Setters
	r = engine->RegisterGlobalFunction("void set_X(int)", asFUNCTION(SetX), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Y(int)", asFUNCTION(SetY), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Z(int)", asFUNCTION(SetZ), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Invisible(int)", asFUNCTION(SetInvisible), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_CollDetection(int)", asFUNCTION(SetCollDetection), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Jump(int)", asFUNCTION(SetJump), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_SwordJinx(int)", asFUNCTION(SetSwordJinx), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_ItemJinx(int)", asFUNCTION(SetItemJinx), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Drunk(int)", asFUNCTION(SetDrunk), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Dir(int)", asFUNCTION(SetDir), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_HitDir(int)", asFUNCTION(SetHitDir), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_HP(int)", asFUNCTION(SetHP), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_MP(int)", asFUNCTION(SetMP), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_MaxHP(int)", asFUNCTION(SetMaxHP), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_MaxMP(int)", asFUNCTION(SetMaxMP), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Action(int)", asFUNCTION(SetAction), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_HeldItem(int)", asFUNCTION(SetHeldItem), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_LadderX(int)", asFUNCTION(SetLadderX), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_LadderY(int)", asFUNCTION(SetLadderY), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Item(int, bool)", asFUNCTION(SetItem), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Equipment(int)", asFUNCTION(SetEquipment), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Tile(int)", asFUNCTION(SetTile), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Flip(int)", asFUNCTION(SetFlip), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_HitZHeight(int)", asFUNCTION(SetHitZHeight), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_HitXOffset(int)", asFUNCTION(SetHitXOffset), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_HitYOffset(int)", asFUNCTION(SetHitYOffset), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_DrawXOffset(int)", asFUNCTION(SetDrawXOffset), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_DrawYOffset(int)", asFUNCTION(SetDrawYOffset), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_DrawZOffset(int)", asFUNCTION(SetDrawZOffset), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Misc(int, int)", asFUNCTION(SetMisc), asCALL_CDECL); Assert(r >= 0);

	engine->SetDefaultNamespace("");
}






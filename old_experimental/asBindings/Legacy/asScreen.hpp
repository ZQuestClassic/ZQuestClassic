
#pragma once



namespace ScreenBindings
{
	//////////////////////////////////////////////////////////////////////////
	/// Setters
	//////////////////////////////////////////////////////////////////////////

	void SetD(int32 i, int value)
	{
		// todo
	}

	void SetFlags(int32 i, int32 value)
	{
		// todo
	}

	void SetEFlags(int32 i, int32 value)
	{
		// todo
	}

	void SetComboD(int32 i, int32 value)
	{
		// todo
	}

	void SetComboC(int32 i, int32 value)
	{
		// todo
	}

	void SetComboF(int32 i, int32 value)
	{
		// todo
	}

	void SetComboI(int32 i, int32 value)
	{
		// todo
	}

	void SetComboT(int32 i, int32 value)
	{
		// todo
	}

	void SetComboS(int32 i, int32 value)
	{
		// todo
	}

	void SetMovingBlockX(int32 value)
	{
		// todo
	}

	void SetMovingBlockY(int32 value)
	{
		// todo
	}

	void SetMovingBlockCombo(int32 value)
	{
		// todo
	}

	void SetMovingBlockCSet(int32 value)
	{
		// todo
	}

	void SetUnderCombo(int32 value)
	{
		// todo
	}

	void SetUnderCSet(int32 value)
	{
		// todo
	}

	void SetState(int32 i, bool value)
	{
		// todo
	}

	void SetDoor(int32 i, int32 value)
	{
		// todo
	}

	void SetRoomType(int32 value)
	{
		// todo
	}

	void SetRoomData(int32 value)
	{
		// todo
	}

	void SetTriggerSecrets()(void value)
	{
		// todo
	}

	void SetLit(bool value)
	{
		// todo
	}

	void SetWavy(int32 value)
	{
		// todo
	}

	void SetQuake(int32 value)
	{
		// todo
	}

	void SetSideWarp(int32 warp, int32 screen, int32 dmap, int32 type)(void value)
	{
		// todo
	}

	void SetTileWarp(int32 warp, int32 screen, int32 dmap, int32 type)(void value)
	{
		// todo
	}

	void SetGetSideWarpDMap(int32 warp)(void value)
	{
		// todo
	}

	void SetGetSideWarpScreen(int32 warp)(void value)
	{
		// todo
	}

	void SetGetSideWarpType(int32 warp)(void value)
	{
		// todo
	}

	void SetGetTileWarpDMap(int32 warp)(void value)
	{
		// todo
	}

	void SetGetTileWarpScreen(int32 warp)(void value)
	{
		// todo
	}

	void SetGetTileWarpType(int32 warp)(void value)
	{
		// todo
	}

	void SetLayerMap(int32 n)(int32 value)
	{
		// todo
	}

	void SetLayerScreen(int32 n)(int32 value)
	{
		// todo
	}

	void SetNumItems()(int32 value)
	{
		// todo
	}

	void SetLoadItem(int32 num)(item value)
	{
		// todo
	}

	void SetCreateItem(int32 id)(item value)
	{
		// todo
	}

	void SetLoadFFC(int32 num)(ffc value)
	{
		// todo
	}

	void SetNumNPCs()(int32 value)
	{
		// todo
	}

	void SetLoadNPC(int32 num)(npc value)
	{
		// todo
	}

	void SetCreateNPC(int32 id)(npc value)
	{
		// todo
	}

	void SetNumLWeapons()(int32 value)
	{
		// todo
	}

	void SetLoadLWeapon(int32 num)(lweapon value)
	{
		// todo
	}

	void SetCreateLWeapon(int32 type)(lweapon value)
	{
		// todo
	}

	void SetNumEWeapons()(int32 value)
	{
		// todo
	}

	void SetLoadEWeapon(int32 num)(eweapon value)
	{
		// todo
	}

	void SetCreateEWeapon(int32 type)(eweapon value)
	{
		// todo
	}

	void SetisSolid(int32 x, int32 y)(bool value)
	{
		// todo
	}

	void SetClearSprites(int32 spritelist)(void value)
	{
		// todo
	}

	void SetMessage(const ScriptString& value)
	{
		// todo
	}


	//////////////////////////////////////////////////////////////////////////
	/// Getters
	//////////////////////////////////////////////////////////////////////////

	int GetD(int32 i)
	{
		return D;
	}

	int32 GetFlags(int32 i)
	{
		return Flags;
	}

	int32 GetEFlags(int32 i)
	{
		return EFlags;
	}

	int32 GetComboD(int32 i)
	{
		return ComboD;
	}

	int32 GetComboC(int32 i)
	{
		return ComboC;
	}

	int32 GetComboF(int32 i)
	{
		return ComboF;
	}

	int32 GetComboI(int32 i)
	{
		return ComboI;
	}

	int32 GetComboT(int32 i)
	{
		return ComboT;
	}

	int32 GetComboS(int32 i)
	{
		return ComboS;
	}

	int32 GetMovingBlockX()
	{
		return MovingBlockX;
	}

	int32 GetMovingBlockY()
	{
		return MovingBlockY;
	}

	int32 GetMovingBlockCombo()
	{
		return MovingBlockCombo;
	}

	int32 GetMovingBlockCSet()
	{
		return MovingBlockCSet;
	}

	int32 GetUnderCombo()
	{
		return UnderCombo;
	}

	int32 GetUnderCSet()
	{
		return UnderCSet;
	}

	bool GetState(int32 i)
	{
		return State;
	}

	int32 GetDoor(int32 i)
	{
		return Door;
	}

	int32 GetRoomType()
	{
		return RoomType;
	}

	int32 GetRoomData()
	{
		return RoomData;
	}

	void GetTriggerSecrets()
	{
		return TriggerSecrets();
	}

	bool GetLit()
	{
		return Lit;
	}

	int32 GetWavy()
	{
		return Wavy;
	}

	int32 GetQuake()
	{
		return Quake;
	}

	void GetSetSideWarp(int32 warp, int32 screen, int32 dmap, int32 type)
	{
		return SetSideWarp(int32 warp, int32 screen, int32 dmap, int32 type);
	}

	void GetSetTileWarp(int32 warp, int32 screen, int32 dmap, int32 type)
	{
		return SetTileWarp(int32 warp, int32 screen, int32 dmap, int32 type);
	}

	voidGetSideWarpDMap(int32 warp)
	{
		return GetSideWarpDMap(int32 warp);
	}

	voidGetSideWarpScreen(int32 warp)
	{
		return GetSideWarpScreen(int32 warp);
	}

	voidGetSideWarpType(int32 warp)
	{
		return GetSideWarpType(int32 warp);
	}

	voidGetTileWarpDMap(int32 warp)
	{
		return GetTileWarpDMap(int32 warp);
	}

	voidGetTileWarpScreen(int32 warp)
	{
		return GetTileWarpScreen(int32 warp);
	}

	voidGetTileWarpType(int32 warp)
	{
		return GetTileWarpType(int32 warp);
	}

	int32 GetLayerMap(int32 n)
	{
		return LayerMap(int32 n);
	}

	int32 GetLayerScreen(int32 n)
	{
		return LayerScreen(int32 n);
	}

	int32 GetNumItems()
	{
		return NumItems();
	}

	item GetLoadItem(int32 num)
	{
		return LoadItem(int32 num);
	}

	item GetCreateItem(int32 id)
	{
		return CreateItem(int32 id);
	}

	ffc GetLoadFFC(int32 num)
	{
		return LoadFFC(int32 num);
	}

	int32 GetNumNPCs()
	{
		return NumNPCs();
	}

	npc GetLoadNPC(int32 num)
	{
		return LoadNPC(int32 num);
	}

	npc GetCreateNPC(int32 id)
	{
		return CreateNPC(int32 id);
	}

	int32 GetNumLWeapons()
	{
		return NumLWeapons();
	}

	lweapon GetLoadLWeapon(int32 num)
	{
		return LoadLWeapon(int32 num);
	}

	lweapon GetCreateLWeapon(int32 type)
	{
		return CreateLWeapon(int32 type);
	}

	int32 GetNumEWeapons()
	{
		return NumEWeapons();
	}

	eweapon GetLoadEWeapon(int32 num)
	{
		return LoadEWeapon(int32 num);
	}

	eweapon GetCreateEWeapon(int32 type)
	{
		return CreateEWeapon(int32 type);
	}

	bool GetisSolid(int32 x, int32 y)
	{
		return isSolid(int32 x, int32 y);
	}

	void GetClearSprites(int32 spritelist)
	{
		return ClearSprites(int32 spritelist);
	}
}



void ScriptRegistrar::RegisterLegacyScreen(asIScriptEngine* engine)
{
	int r;
	using namespace ScreenBindings;

	engine->SetDefaultNamespace("Screen");

	// Getters
	r = engine->RegisterGlobalFunction("int get_D(int)", asFUNCTION(GetD), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Flags(int)", asFUNCTION(GetFlags), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_EFlags(int)", asFUNCTION(GetEFlags), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_ComboD(int)", asFUNCTION(GetComboD), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_ComboC(int)", asFUNCTION(GetComboC), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_ComboF(int)", asFUNCTION(GetComboF), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_ComboI(int)", asFUNCTION(GetComboI), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_ComboT(int)", asFUNCTION(GetComboT), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_ComboS(int)", asFUNCTION(GetComboS), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_MovingBlockX()", asFUNCTION(GetMovingBlockX), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_MovingBlockY()", asFUNCTION(GetMovingBlockY), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_MovingBlockCombo()", asFUNCTION(GetMovingBlockCombo), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_MovingBlockCSet()", asFUNCTION(GetMovingBlockCSet), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_UnderCombo()", asFUNCTION(GetUnderCombo), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_UnderCSet()", asFUNCTION(GetUnderCSet), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("bool get_State(int)", asFUNCTION(GetState), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Door(int)", asFUNCTION(GetDoor), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_RoomType()", asFUNCTION(GetRoomType), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_RoomData()", asFUNCTION(GetRoomData), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_TriggerSecrets()", asFUNCTION(GetTriggerSecrets()), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("bool get_Lit()", asFUNCTION(GetLit), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Wavy()", asFUNCTION(GetWavy), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_Quake()", asFUNCTION(GetQuake), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_SetSideWarp(int warp, int screen, int dmap, int type)", asFUNCTION(GetSetSideWarp(int warp, int screen, int dmap, int type)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_SetTileWarp(int warp, int screen, int dmap, int type)", asFUNCTION(GetSetTileWarp(int warp, int screen, int dmap, int type)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_GetSideWarpDMap(int warp)", asFUNCTION(GetGetSideWarpDMap), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_GetSideWarpScreen(int warp)", asFUNCTION(GetGetSideWarpScreen), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_GetSideWarpType(int warp)", asFUNCTION(GetGetSideWarpType), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_GetTileWarpDMap(int warp)", asFUNCTION(GetGetTileWarpDMap), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_GetTileWarpScreen(int warp)", asFUNCTION(GetGetTileWarpScreen), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_GetTileWarpType(int warp)", asFUNCTION(GetGetTileWarpType), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_LayerMap(int n)", asFUNCTION(GetLayerMap), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_LayerScreen(int n)", asFUNCTION(GetLayerScreen), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_NumItems()", asFUNCTION(GetNumItems()), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("item get_LoadItem(int num)", asFUNCTION(GetLoadItem), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("item get_CreateItem(int id)", asFUNCTION(GetCreateItem(int id)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("ffc get_LoadFFC(int num)", asFUNCTION(GetLoadFFC), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_NumNPCs()", asFUNCTION(GetNumNPCs()), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("npc get_LoadNPC(int num)", asFUNCTION(GetLoadNPC), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("npc get_CreateNPC(int id)", asFUNCTION(GetCreateNPC(int id)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_NumLWeapons()", asFUNCTION(GetNumLWeapons()), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("lweapon get_LoadLWeapon(int num)", asFUNCTION(GetLoadLWeapon), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("lweapon get_CreateLWeapon(int type)", asFUNCTION(GetCreateLWeapon(int type)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("int get_NumEWeapons()", asFUNCTION(GetNumEWeapons()), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("eweapon get_LoadEWeapon(int num)", asFUNCTION(GetLoadEWeapon), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("eweapon get_CreateEWeapon(int type)", asFUNCTION(GetCreateEWeapon(int type)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("bool get_isSolid(int x, int y)", asFUNCTION(GetisSolid(int x, int y)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_ClearSprites(int spritelist)", asFUNCTION(GetClearSprites(int spritelist)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void get_Message(int string)", asFUNCTION(GetMessage(int string)), asCALL_CDECL); Assert(r >= 0);

	// Setters
	r = engine->RegisterGlobalFunction("void set_D(int)", asFUNCTION(SetD), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Flags(int)", asFUNCTION(SetFlags), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_EFlags(int)", asFUNCTION(SetEFlags), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_ComboD(int)", asFUNCTION(SetComboD), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_ComboC(int)", asFUNCTION(SetComboC), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_ComboF(int)", asFUNCTION(SetComboF), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_ComboI(int)", asFUNCTION(SetComboI), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_ComboT(int)", asFUNCTION(SetComboT), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_ComboS(int)", asFUNCTION(SetComboS), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_MovingBlockX()", asFUNCTION(SetMovingBlockX), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_MovingBlockY()", asFUNCTION(SetMovingBlockY), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_MovingBlockCombo()", asFUNCTION(SetMovingBlockCombo), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_MovingBlockCSet()", asFUNCTION(SetMovingBlockCSet), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_UnderCombo()", asFUNCTION(SetUnderCombo), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_UnderCSet()", asFUNCTION(SetUnderCSet), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_State(int)", asFUNCTION(SetState), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Door(int)", asFUNCTION(SetDoor), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_RoomType()", asFUNCTION(SetRoomType), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_RoomData()", asFUNCTION(SetRoomData), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_TriggerSecrets()", asFUNCTION(SetTriggerSecrets()), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Lit()", asFUNCTION(SetLit), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Wavy()", asFUNCTION(SetWavy), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_Quake()", asFUNCTION(SetQuake), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_SetSideWarp(int warp, int screen, int dmap, int type)", asFUNCTION(SetSetSideWarp), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_SetTileWarp(int warp, int screen, int dmap, int type)", asFUNCTION(SetSetTileWarp), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_GetSideWarpDMap(int warp)", asFUNCTION(SetGetSideWarpDMap), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_GetSideWarpScreen(int warp)", asFUNCTION(SetGetSideWarpScreen), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_GetSideWarpType(int warp)", asFUNCTION(SetGetSideWarpType), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_GetTileWarpDMap(int warp)", asFUNCTION(SetGetTileWarpDMap), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_GetTileWarpScreen(int warp)", asFUNCTION(SetGetTileWarpScreen), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_GetTileWarpType(int warp)", asFUNCTION(SetGetTileWarpType), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_LayerMap(int n)", asFUNCTION(SetLayerMap), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_LayerScreen(int n)", asFUNCTION(SetLayerScreen), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_NumItems()", asFUNCTION(SetNumItems()), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_LoadItem(int num)", asFUNCTION(SetLoadItem), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_CreateItem(int)", asFUNCTION(SetCreateItem(int id)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_LoadFFC(int)", asFUNCTION(SetLoadFFC), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_NumNPCs()", asFUNCTION(SetNumNPCs()), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_LoadNPC(int)", asFUNCTION(SetLoadNPC), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_CreateNPC(int)", asFUNCTION(SetCreateNPC(int id)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_NumLWeapons()", asFUNCTION(SetNumLWeapons()), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_LoadLWeapon(int)", asFUNCTION(SetLoadLWeapon), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_CreateLWeapon(int)", asFUNCTION(SetCreateLWeapon(int type)), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_NumEWeapons()", asFUNCTION(SetNumEWeapons()), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_LoadEWeapon(int)", asFUNCTION(SetLoadEWeapon), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_CreateEWeapon(int)", asFUNCTION(SetCreateEWeapon), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_isSolid(int, int)", asFUNCTION(SetisSolid), asCALL_CDECL); Assert(r >= 0);
	r = engine->RegisterGlobalFunction("void set_ClearSprites(int)", asFUNCTION(SetClearSprites), asCALL_CDECL); Assert(r >= 0);


	engine->SetDefaultNamespace("");
}



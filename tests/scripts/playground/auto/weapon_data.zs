#include "std.zh"
#include "auto/test_runner.zs"

generic script weapon_data
{
	void run()
	{
		Test::Init();

		// A script-owned weapondata round-trips through every get/set register.
		weapondata wd = new weapondata();
		Test::Assert(wd->Valid, "new weapondata is valid");

		wd->HitWidth = 12;
		wd->HitHeight = 13;
		wd->HitZHeight = 14;
		wd->Timeout = 99;
		wd->PierceCount = 4;
		wd->LiftLevel = 3;
		wd->Step = 250; // fixed-point, exercises the *100 scaling
		wd->ViewportDespawnRange = 80;

		Test::AssertEqual(wd->HitWidth, 12, "HitWidth");
		Test::AssertEqual(wd->HitHeight, 13, "HitHeight");
		Test::AssertEqual(wd->HitZHeight, 14, "HitZHeight");
		Test::AssertEqual(wd->Timeout, 99, "Timeout");
		Test::AssertEqual(wd->PierceCount, 4, "PierceCount");
		Test::AssertEqual(wd->LiftLevel, 3, "LiftLevel");
		Test::AssertEqual(wd->Step, 250, "Step");
		Test::AssertEqual(wd->ViewportDespawnRange, 80, "ViewportDespawnRange");

		// Array members: an InitD value and the bitflag arrays.
		wd->InitD[2] = 4242;
		Test::AssertEqual(wd->InitD[2], 4242, "InitD[2]");
		wd->Flags[WFLAG_UPDATE_BURNSPR] = true;
		wd->Flags[WFLAG_BREAKS_ON_SOLID] = true;
		Test::Assert(wd->Flags[WFLAG_UPDATE_BURNSPR], "Flags[WFLAG_UPDATE_BURNSPR]");
		Test::Assert(wd->Flags[WFLAG_BREAKS_ON_SOLID], "Flags[WFLAG_BREAKS_ON_SOLID]");
		Test::Assert(!wd->Flags[WFLAG_PICKUP_ITEMS], "Flags[WFLAG_PICKUP_ITEMS] is unset");

		// CopyTo produces an independent copy.
		weapondata wd2 = new weapondata();
		wd->CopyTo(wd2);
		Test::AssertEqual(wd2->HitWidth, 12, "CopyTo HitWidth");
		Test::AssertEqual(wd2->InitD[2], 4242, "CopyTo InitD[2]");
		Test::Assert(wd2->Flags[WFLAG_UPDATE_BURNSPR], "CopyTo Flags");
		wd->HitWidth = 1;
		Test::AssertEqual(wd2->HitWidth, 12, "CopyTo is independent of source");

		// ApplyTo writes the settings onto a live weapon. wd2 has the burn-sprite
		// flag set, so this also exercises load_weap_data's sprite-loading path.
		lweapon weap = Screen->CreateLWeapon(LW_ARROW);
		wd2->ApplyTo(weap);
		Test::AssertEqual(weap->LiftLevel, 3, "ApplyTo LiftLevel");
		weap->Remove();

		// itemdata's weapondata is backed by the item buffer: writes persist to the
		// source and the handle stays valid across reloads of that source.
		itemdata arrow = Game->LoadItemData(I_ARROW1);
		const weapondata idata_wd = arrow->WeaponData;
		Test::Assert(idata_wd->Valid, "itemdata WeaponData valid");
		idata_wd->Timeout = 55;
		Test::AssertEqual(Game->LoadItemData(I_ARROW1)->WeaponData->Timeout, 55,
			"itemdata WeaponData write persists to the source");

		// combodata exposes both the misc and lift weapon data.
		combodata cd = Game->LoadComboData(1);
		Test::Assert(cd->MiscWeaponData->Valid, "combodata MiscWeaponData valid");
		Test::Assert(cd->LiftWeaponData->Valid, "combodata LiftWeaponData valid");

		// Object identity: two reads of the same source return the same handle (the
		// source is deduped), so a write through one is visible through the other.
		// A freshly-created weapondata is always a distinct object.
		itemdata arrow2 = Game->LoadItemData(I_ARROW1);
		weapondata a = arrow2->WeaponData;
		weapondata b = arrow2->WeaponData;
		Test::AssertEqual(<int>a, <int>b, "same source yields the same handle");
		a->PierceCount = 7;
		Test::AssertEqual(b->PierceCount, 7, "write through one handle is seen by the other");
		weapondata c = new weapondata();
		weapondata d = new weapondata();
		Test::Assert(<int>c != <int>d, "each new weapondata is a distinct object");

		// An npc's weapondata re-resolves the enemy each access: it is valid while
		// the enemy lives and becomes invalid once the enemy is gone, rather than
		// dangling. (The handle is held in a variable across the enemy's removal.)
		npc e = Screen->CreateNPC(NPC_GEL);
		const weapondata npc_wd = e->WeaponData;
		Test::Assert(npc_wd->Valid, "npc WeaponData valid while the enemy lives");
		e->Remove();
		Waitframe();
		Test::Assert(!npc_wd->Valid, "npc WeaponData invalid after the enemy is removed");
		// Reading a field (rather than Valid) through an invalidated handle is a safe
		// no-op returning 0. (The read logs a script error; the JIT now syncs ri->pc
		// before engine register reads, so its error trace matches the interpreter's.)
		Test::AssertEqual(npc_wd->Timeout, 0, "field read on invalid handle returns 0");

		// The same invalidation happens when the source enemy is cleared by a screen
		// change, which is the scenario scripts hit in practice: grab the handle,
		// leave the screen, and the handle no longer resolves.
		npc e2 = Screen->CreateNPC(NPC_GEL);
		const weapondata screen_wd = e2->WeaponData;
		Test::Assert(screen_wd->Valid, "npc WeaponData valid before screen change");
		int map = Game->LoadDMapData(Test::TestingDmap)->Map;
		Game->LoadMapData(map, 1)->Valid = 1;
		Player->Warp(Test::TestingDmap, 1);
		Waitframe();
		Test::Assert(!screen_wd->Valid, "npc WeaponData invalid after screen change");

		Test::End();
	}
}

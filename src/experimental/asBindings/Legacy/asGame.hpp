
#pragma once


namespace GameBindings
{
	int32 GetCurScreen()
	{
		//todo
	}

	int32 GetCurDMapScreen()
	{
		//todo
	}

	int32 GetCurLevel()
	{
		//todo
	}

	int32 GetCurMap()
	{
		//todo
	}

	int32 GetCurDMap()
	{
		//todo
	}

	int32 DMapFlags[]
	{
		//todo
	}

	int32 DMapLevel[]
	{
		//todo
	}

	int32 DMapCompass[]
	{
		//todo
	}

	int32 DMapContinue[]
	{
		//todo
	}

	int32 DMapMIDI[]
	{
		//todo
	}

	void GetDMapName(int32 DMap, int32 buffer[])
	{
		//todo
	}

	void GetDMapTitle(int32 DMap, int32 buffer[])
	{
		//todo
	}

	void GetDMapIntro(int32 DMap, int32 buffer[])
	{
		//todo
	}

	int32 DMapOffset[]
	{
		//todo
	}

	int32 DMapMap[]
	{
		//todo
	}

	int32 NumDeaths
	{
		//todo
	}

	int32 Cheat
	{
		//todo
	}

	int32 Time
	{
		//todo
	}

	bool TimeValid
	{
		//todo
	}

	bool HasPlayed
	{
		//todo
	}

	bool Standalone
	{
		//todo
	}

	int32 GuyCount[]
	{
		//todo
	}

	int32 ContinueDMap
	{
		//todo
	}

	int32 ContinueScreen
	{
		//todo
	}

	int32 LastEntranceDMap
	{
		//todo
	}

	int32 LastEntranceScreen
	{
		//todo
	}

	int32 Counter[]
	{
		//todo
	}

	int32 MCounter[]
	{
		//todo
	}

	int32 DCounter[]
	{
		//todo
	}

	int32 Generic[]
	{
		//todo
	}

	int32 LItems[]
	{
		//todo
	}

	int32 LKeys[]
	{
		//todo
	}

	int32 GetScreenFlags(int32 map, int32 screen, int32 flagset)
	{
		//todo
	}

	int32 GetScreenEFlags(int32 map, int32 screen, int32 flagset)
	{
		//todo
	}

	bool GetScreenState(int32 map, int32 screen, int32 flag)
	{
		//todo
	}

	void SetScreenState(int32 map, int32 screen, int32 flag, bool value)
	{
		//todo
	}

	float_or_int32 GetScreenD(int32 screen, int32 reg)
	{
		//todo
	}

	void SetScreenD(int32 screen, int32 reg, float_or_int32 value)
	{
		//todo
	}

	float_or_int32 GetDMapScreenD(int32 dmap, int32 screen, int32 reg)
	{
		//todo
	}

	void SetDMapScreenD(int32 dmap, int32 screen, int32 reg, float_or_int32 value)
	{
		//todo
	}

	itemdata LoadItemData(int32 item)
	{
		//todo
	}

	void PlaySound(int32 soundid)
	{
		//todo
	}

	void PlayMIDI(int32 MIDIid)
	{
		//todo
	}

	int32 GetMIDI()
	{
		//todo
	}

	bool PlayEnhancedMusic(int32 filename[], int32 track)
	{
		//todo
	}

	void GetDMapMusicFilename(int32 dmap, int32 buf[])
	{
		//todo
	}

	int32 GetDMapMusicTrack(int32 dmap)
	{
		//todo
	}

	void SetDMapEnhancedMusic(int32 dmap, int32 filename[], int32 track)
	{
		//todo
	}

	int32 GetComboData(int32 map, int32 screen, int32 position)
	{
		//todo
	}

	void SetComboData(int32 map, int32 screen, int32 position, int32 value)
	{
		//todo
	}

	int32 GetComboCSet(int32 map, int32 screen, int32 position)
	{
		//todo
	}

	void SetComboCSet(int32 map, int32 screen, int32 position, int32 value)
	{
		//todo
	}

	int32 GetComboFlag(int32 map, int32 screen, int32 position)
	{
		//todo
	}

	void SetComboFlag(int32 map, int32 screen, int32 position, int32 value)
	{
		//todo
	}

	int32 GetComboType(int32 map, int32 screen, int32 position)
	{
		//todo
	}

	void SetComboType(int32 map, int32 screen, int32 position, int32 value)
	{
		//todo
	}

	int32 GetComboInherentFlag(int32 map, int32 screen, int32 position)
	{
		//todo
	}

	void SetComboInherentFlag(int32 map, int32 screen, int32 position, int32 value)
	{
		//todo
	}

	int32 GetComboSolid(int32 map, int32 screen, int32 position)
	{
		//todo
	}

	void SetComboSolid(int32 map, int32 screen, int32 position, int32 value)
	{
		//todo
	}

	int32 ComboTile(int32 combo)
	{
		//todo
	}

	void GetSaveName(int32 buffer[])
	{
		//todo
	}

	void SetSaveName(int32 name[])
	{
		//todo
	}

	void End()
	{
		//todo
	}

	void Save()
	{
		//todo
	}

	bool ShowSaveScreen()
	{
		//todo
	}

	void ShowSaveQuitScreen()
	{
		//todo
	}

	void GetMessage(int32 string, int32 buffer[])
	{
		//todo
	}

	int32 GetFFCScript(int32 name[])
	{
		//todo
	}

	bool ClickToFreezeEnabled
	{
		//todo
	}

}

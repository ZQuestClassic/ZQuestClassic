
#define float_or_int32 float


namespace GlobalBindings
{
	int32 Rand(int32 maxvalue);
	void Quit();
	void Waitframe();
	void Waitdraw(); //?

	// DEPRICATED ***
	void Trace(int32 val);
	void TraceB(bool state);
	void TraceS(int32 s[]);
	void TraceToBase(int32 val, int32 base, int32 mindigits);
	void TraceNL();
	void ClearTrace();
	// DEPRICATED ***

	// DEPRICATED ***
	float Sin(float deg);
	float Cos(float deg);
	float Tan(float deg);
	// DEPRICATED ***

	float RadianSin(float rad);
	float RadianCos(float rad);
	float RadianTan(float rad);

	float ArcTan(int32 x, int32 y);
	float ArcSin(float x);
	float ArcCos(float x);
	float Max(float a, float b);
	float Min(float a, float b);
	int32 Pow(int32 base, int32 exp);
	int32 InvPow(int32 base, int32 exp);
	float Log10(float val);
	float Ln(float val);
	int32 Factorial(int32 val);
	float Abs(float val);
	float Sqrt(float val);

	// DEPRICATED ***
	void CopyTile(int32 srctile, int32 desttile);
	void SwapTile(int32 firsttile, int32 secondtile);
	void ClearTile(int32 tileref);
	int32 SizeOfArray(int32 array[]);
	// DEPRICATED ***
}


namespace FFCBindings
{
	int32 Data;
	int32 Script; // DEPRICATED ***
	int32 CSet;
	int32 Delay;
	float X;
	float Y;
	float Vx;
	float Vy;
	float Ax;
	float Ay;
	bool Flags[];
	int32 TileWidth;
	int32 TileHeight;
	int32 EffectWidth;
	int32 EffectHeight;
	int32 Link;
	float InitD[];
	float Misc[];
};


namespace LinkBindings
{
	int32 X;
	int32 Y;
	int32 Z;
	bool Invisible;
	bool CollDetection;
	int32 Jump;
	int32 SwordJinx;
	int32 ItemJinx;
	int32 Drunk;
	int32 Dir;
	int32 HitDir;
	int32 HP;
	int32 MP;
	int32 MaxHP;
	int32 MaxMP;
	int32 Action;
	int32 HeldItem;
	int32 LadderX;

	// DEPRICATED ***
	bool InputStart;
	bool InputMap;
	bool InputUp;
	bool InputDown;
	bool InputLeft;
	bool InputRight;
	bool InputA;
	bool InputB;
	bool InputL;
	bool InputR;
	bool InputEx1;
	bool InputEx2;
	bool InputEx3;
	bool InputEx4;
	bool InputAxisUp;
	bool InputAxisDown;
	bool InputAxisLeft;
	bool InputAxisRight;
	// DEPRICATED ***

	// DEPRICATED ***
	bool PressStart;
	bool PressMap;
	bool PressUp;
	bool PressDown;
	bool PressLeft;
	bool PressRight;
	bool PressA;
	bool PressB;
	bool PressL;
	bool PressR;
	bool PressEx1;
	bool PressEx2;
	bool PressEx3;
	bool PressEx4;
	bool PressAxisUp;
	bool PressAxisDown;
	bool PressAxisLeft;
	bool PressAxisRight;
	// DEPRICATED ***

	// DEPRICATED ***
	int32 InputMouseX;
	int32 InputMouseY;
	int32 InputMouseB;
	int32 InputMouseZ;
	// DEPRICATED ***

	bool Item[];
	int32 Equipment;
	int32 Tile;
	int32 Flip;
	int32 HitZHeight;
	int32 HitXOffset;
	int32 HitYOffset;
	int32 DrawXOffset;
	int32 DrawYOffset;
	int32 DrawZOffset;
	float_or_int32 Misc[];
	void Warp(int32 DMap, int32 screen);
	void PitWarp(int32 DMap, int32 screen);
	SelectAWeapon(int32 dir);
	SelectBWeapon(int32 dir);
};


namespace Screen
{
	float_or_int32 D[];
	int32 Flags[];
	int32 EFlags[];
	int32 ComboD[];
	int32 ComboC[];
	int32 ComboF[];
	int32 ComboI[];
	int32 ComboT[];
	int32 ComboS[];
	int32 MovingBlockX;
	int32 MovingBlockY;
	int32 MovingBlockCombo;
	int32 MovingBlockCSet;
	int32 UnderCombo;
	int32 UnderCSet;
	bool State[];
	int32 Door[];
	int32 RoomType;
	int32 RoomData;
	void TriggerSecrets();
	bool Lit;
	int32 Wavy;
	int32 Quake;
	void SetSideWarp(int32 warp, int32 screen, int32 dmap, int32 type);
	void SetTileWarp(int32 warp, int32 screen, int32 dmap, int32 type);
	void GetSideWarpDMap(int32 warp);
	void GetSideWarpScreen(int32 warp);
	void GetSideWarpType(int32 warp);
	void GetTileWarpDMap(int32 warp);
	void GetTileWarpScreen(int32 warp);
	void GetTileWarpType(int32 warp);
	int32 LayerMap(int32 n);
	int32 LayerScreen(int32 n);
	int32 NumItems();
	item LoadItem(int32 num);
	item CreateItem(int32 id);
	ffc LoadFFC(int32 num); // DEPRICATED ***
	int32 NumNPCs();
	npc LoadNPC(int32 num);
	npc CreateNPC(int32 id);
	int32 NumLWeapons();
	lweapon LoadLWeapon(int32 num);
	lweapon CreateLWeapon(int32 type);
	int32 NumEWeapons();
	eweapon LoadEWeapon(int32 num);
	eweapon CreateEWeapon(int32 type);
	bool isSolid(int32 x, int32 y);
	void ClearSprites(int32 spritelist);

	 // DEPRICATED ***
	void Rectangle(int32 layer, int32 x, int32 y, int32 x2, int32 y2, int32 color, float_or_int32 scale, int32 rx, int32 ry, int32 rangle, bool fill, int32 opacity);
	void Circle(int32 layer, int32 x, int32 y, int32 radius, int32 color, float_or_int32 scale, int32 rx, int32 ry, int32 rangle, bool fill, int32 opacity);
	void Arc(int32 layer, int32 x, int32 y, int32 radius, int32 startangle, int32 endangle, int32 color, float_or_int32 scale, int32 rx, int32 ry, int32 rangle, bool closed, bool fill, int32 opacity);
	void Ellipse(int32 layer, int32 x, int32 y, int32 xradius, int32 yradius, int32 color, float_or_int32 scale, int32 rx, int32 ry, int32 rangle, bool fill, int32 opacity);
	void Spline(int32 layer, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3,int32 x4, int32 y4, int32 color, int32 opacity);
	void Line(int32 layer, int32 x, int32 y, int32 x2, int32 y2, int32 color, float_or_int32 scale, int32 rx, int32 ry, int32 rangle, int32 opacity);
	void PutPixel(int32 layer, int32 x, int32 y, int32 color, int32 rx, int32 ry, int32 rangle, int32 opacity);
	void DrawTile(int32 layer, int32 x, int32 y, int32 tile, int32 blockw, int32 blockh, int32 cset, int32 xscale, int32 yscale, int32 rx, int32 ry, int32 rangle, int32 flip, bool transparency, int32 opacity);
	void FastTile( int32 layer, int32 x, int32 y, int32 tile, int32 cset, int32 opacity );
	void DrawCombo(int32 layer, int32 x, int32 y, int32 combo, int32 w, int32 h, int32 cset, int32 xscale, int32 yscale, int32 rx, int32 ry, int32 rangle, int32 frame, int32 flip, bool transparency, int32 opacity);
	void FastCombo( int32 layer, int32 x, int32 y, int32 combo, int32 cset, int32 opacity );
	 // DEPRICATED ***

	void Message(int32 string);

	 // DEPRICATED ***
	void DrawCharacter(int32 layer, int32 x, int32 y, int32 font,int32 color, int32 background_color,int32 width, int32 height, int32 glyph, int32 opacity );
	void DrawInteger(int32 layer, int32 x, int32 y, int32 font,int32 color, int32 background_color,int32 width, int32 height, int32 number, int32 number_decimal_places, int32 opacity);
	void DrawString( int32 layer, int32 x, int32 y, int32 font,int32 color, int32 background_color,int32 format, int32 ptr[], int32 opacity );
	void Quad( int32 layer, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, int32 x4, int32 y4,int32 w, int32 h, int32 cset, int32 flip, int32 texture, int32 render_mode);
	void Triangle( int32 layer, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3,int32 w, int32 h, int32 cset, int32 flip, int32 texture, int32 render_mode);
	void Quad3D( int32 layer, int32 pos[], int32 uv[], int32 cset[], int32 size[], int32 flip, int32 texture, int32 render_mode );
	void Triangle3D( int32 layer, int32 pos[], int32 uv[], int32 csets[], int32 size[], int32 flip, int32 texture, int32 render_mode );
	void SetRenderTarget( int32 bitmap_id );
	void DrawBitmap( int32 layer, int32 bitmap_id, int32 source_x, int32 source_y, int32 source_w, int32 source_h, int32 dest_x, int32 dest_y, int32 dest_w, int32 dest_h, float_or_int32 rotation, bool mask);
	void DrawLayer(int32 layer, int32 source_map, int32 source_screen, int32 source_layer, int32 x, int32 y, float_or_int32 rotation, int32 opacity);
	void DrawScreen(int32 layer, int32 map, int32 source_screen, int32 x, int32 y, float_or_int32 rotation);
	 // DEPRICATED ***
}; 


namespace ItemBindings
{
	bool isValid();
	int32 X;
	int32 Y;
	int32 Jump;
	int32 DrawStyle;
	int32 ID;
	int32 OriginalTile;
	int32 Tile;
	int32 CSet;
	int32 FlashCSet;
	int32 NumFrames;
	int32 Frame;
	int32 ASpeed;
	int32 Delay;
	bool Flash;
	int32 Flip;
	int32 Pickup;
	int32 Extend;
	int32 TileWidth;
	int32 TileHeight;
	int32 HitWidth;
	int32 HitHeight;
	int32 HitZHeight;
	int32 HitXOffset;
	int32 HitYOffset;
	int32 DrawXOffset;
	int32 DrawYOffset;
	int32 DrawZOffset;
	float_or_int32 Misc[];
};

namespace WeaponBindings
{
	bool isValid();
	void UseSprite(int32 id);
	bool Behind;
	int32 ID;
	int32 X;
	int32 Y;
	int32 Z;
	int32 Jump;
	int32 DrawStyle;
	int32 Dir;
	int32 OriginalTile;
	int32 Tile;
	int32 OriginalCSet;
	int32 CSet;
	int32 FlashCSet;
	int32 NumFrames;
	int32 Frame;
	int32 ASpeed;
	int32 Damage;
	int32 Step;
	int32 Angle;
	bool Angular;
	bool CollDetection;
	int32 DeadState;
	bool Flash;
	int32 Flip;
	int32 Extend;
	int32 TileWidth;
	int32 TileHeight;
	int32 HitWidth;
	int32 HitHeight;
	int32 HitZHeight;
	int32 HitXOffset;
	int32 HitYOffset;
	int32 DrawXOffset;
	int32 DrawYOffset;
	int32 DrawZOffset;
	float_or_int32 Misc[];
}; 

namespace ItemDataBindings
{
	float_or_int32 InitD[];
	void GetName(int32 buffer[]);
	int32 Family;
	int32 Level;
	int32 Power;
	int32 Amount;
	int32 MaxIncrement;
	int32 Max;
	bool Keep;
	int32 Counter;
	int32 UseSound;
}; 


namespace GameBindings
{
	int32 GetCurScreen();
	int32 GetCurDMapScreen();
	int32 GetCurLevel();
	int32 GetCurMap();
	int32 GetCurDMap();
	int32 DMapFlags[];
	int32 DMapLevel[];
	int32 DMapCompass[];
	int32 DMapContinue[];
	int32 DMapMIDI[];
	void GetDMapName(int32 DMap, int32 buffer[]);
	void GetDMapTitle(int32 DMap, int32 buffer[]);
	void GetDMapIntro(int32 DMap, int32 buffer[]);
	int32 DMapOffset[];
	int32 DMapMap[];
	int32 NumDeaths;
	int32 Cheat;
	int32 Time;
	bool TimeValid;
	bool HasPlayed;
	bool Standalone;
	int32 GuyCount[];
	int32 ContinueDMap;
	int32 ContinueScreen;
	int32 LastEntranceDMap;
	int32 LastEntranceScreen;
	int32 Counter[];
	int32 MCounter[];
	int32 DCounter[];
	int32 Generic[];
	int32 LItems[];
	int32 LKeys[];
	int32 GetScreenFlags(int32 map, int32 screen, int32 flagset);
	int32 GetScreenEFlags(int32 map, int32 screen, int32 flagset);
	bool GetScreenState(int32 map, int32 screen, int32 flag);
	void SetScreenState(int32 map, int32 screen, int32 flag, bool value);
	float_or_int32 GetScreenD(int32 screen, int32 reg);
	void SetScreenD(int32 screen, int32 reg, float_or_int32 value);
	float_or_int32 GetDMapScreenD(int32 dmap, int32 screen, int32 reg);
	void SetDMapScreenD(int32 dmap, int32 screen, int32 reg, float_or_int32 value);
	itemdata LoadItemData(int32 item);
	void PlaySound(int32 soundid);
	void PlayMIDI(int32 MIDIid);
	int32 GetMIDI();
	bool PlayEnhancedMusic(int32 filename[], int32 track);
	void GetDMapMusicFilename(int32 dmap, int32 buf[]);
	int32 GetDMapMusicTrack(int32 dmap);
	void SetDMapEnhancedMusic(int32 dmap, int32 filename[], int32 track);
	int32 GetComboData(int32 map, int32 screen, int32 position);
	void SetComboData(int32 map, int32 screen, int32 position, int32 value);
	int32 GetComboCSet(int32 map, int32 screen, int32 position);
	void SetComboCSet(int32 map, int32 screen, int32 position, int32 value);
	int32 GetComboFlag(int32 map, int32 screen, int32 position);
	void SetComboFlag(int32 map, int32 screen, int32 position, int32 value);
	int32 GetComboType(int32 map, int32 screen, int32 position);
	void SetComboType(int32 map, int32 screen, int32 position, int32 value);
	int32 GetComboInherentFlag(int32 map, int32 screen, int32 position);
	void SetComboInherentFlag(int32 map, int32 screen, int32 position, int32 value);
	int32 GetComboSolid(int32 map, int32 screen, int32 position);
	void SetComboSolid(int32 map, int32 screen, int32 position, int32 value);
	int32 ComboTile(int32 combo);
	void GetSaveName(int32 buffer[]);
	void SetSaveName(int32 name[]);
	void End();
	void Save();
	bool ShowSaveScreen();
	void ShowSaveQuitScreen();
	void GetMessage(int32 string, int32 buffer[]);
	int32 GetFFCScript(int32 name[]);
	bool ClickToFreezeEnabled;
}; 


namespace NPCBindings
{
	bool isValid();
	void GetName(int32 buffer[]);
	int32 ID;
	int32 Type;
	int32 X;
	int32 Y;
	int32 Z;
	int32 Jump;
	int32 Dir;
	int32 Rate;
	int32 Haltrate;
	int32 Homing;
	int32 Hunger;
	int32 Step;
	bool CollDetection;
	int32 ASpeed;
	int32 DrawStyle;
	int32 HP;
	int32 Damage;
	int32 WeaponDamage;
	int32 Stun;
	int32 OriginalTile;
	int32 Tile;
	int32 Weapon;
	int32 ItemSet;
	int32 CSet;
	int32 BossPal;
	int32 SFX;
	int32 Extend;
	int32 TileWidth;
	int32 TileHeight;
	int32 HitWidth;
	int32 HitHeight;
	int32 HitZHeight;
	int32 HitXOffset;
	int32 HitYOffset;
	int32 DrawXOffset;
	int32 DrawYOffset;
	int32 DrawZOffset;
	int32 Defense[];
	int32 Attributes[];
	int32 MiscFlags;
	float_or_int32 Misc[];
	void BreakShield();
}; 

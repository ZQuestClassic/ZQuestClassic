


namespace GlobalBindings
{

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
	void Trace(int32 val);
	void TraceB(bool state);
	void TraceS(int32 s[]);
	void TraceToBase(int32 val, int32 base, int32 mindigits);
	void TraceNL();
	void ClearTrace();
	// DEPRICATED ***


	// DEPRICATED ***
	float RadianSin(float rad);
	float RadianCos(float rad);
	float RadianTan(float rad);
	// DEPRICATED ***

	// DEPRICATED ***
	void CopyTile(int32 srctile, int32 desttile);
	void SwapTile(int32 firsttile, int32 secondtile);
	void ClearTile(int32 tileref);
	int32 SizeOfArray(int32 array[]);
	// DEPRICATED ***

}





namespace LinkBindings
{
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
}


namespace ScreenBindings
{

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






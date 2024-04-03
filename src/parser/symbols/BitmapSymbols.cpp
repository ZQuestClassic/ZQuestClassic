#include "SymbolDefs.h"

BitmapSymbols BitmapSymbols::singleton = BitmapSymbols();

static AccessorTable BitmapTable[] =
{
//	  name,                    tag,            rettype,  var,          funcFlags,  params,optparams
	{ "getWidth",                0,         ZTID_FLOAT,   BITMAPWIDTH,         0,  { ZTID_BITMAP },{} },
	{ "setWidth",                0,          ZTID_VOID,   BITMAPWIDTH, FL_RDONLY,  { ZTID_BITMAP, ZTID_FLOAT },{} },
	{ "getHeight",               0,         ZTID_FLOAT,   BITMAPHEIGHT,        0,  { ZTID_BITMAP },{} },
	{ "setHeight",               0,          ZTID_VOID,   BITMAPHEIGHT,FL_RDONLY,  { ZTID_BITMAP, ZTID_FLOAT },{} },
	
	{ "GetPixel",                0,         ZTID_FLOAT,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "CountColor",              0,         ZTID_FLOAT,   -1,                  0,  { ZTID_BITMAP, ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ -10000 } },
	{ "Rectangle",               0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{ 10000, 0, 0, 0, 10000, 1280000 } },
	{ "Circle",                  0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{ 10000, 0, 0, 0, 10000, 1280000 } },
	{ "Arc",                     0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_BOOL, ZTID_FLOAT },{ 10000, 0, 0, 0, 10000, 10000, 1280000 } },
	{ "Ellipse",                 0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{ 10000, 0, 0, 0, 10000, 1280000 } },
	{ "Line",                    0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 10000, 0, 0, 0, 1280000 } },
	{ "Spline",                  0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 10000 } },
	{ "PutPixel",                0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0, 0, 0, 1280000 } },
	{ "DrawCharacter",           0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_CHAR, ZTID_FLOAT },{ 1280000 } },
	{ "DrawInteger",             0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0, 1280000 } },
	{ "DrawTile",                0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{ -10000, -10000, 0, 0, 0, 0, 10000, 1280000 } },
	{ "DrawTileCloaked",         0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "DrawCombo",               0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{ -10000, -10000, 0, 0, 0, 0, 10000, 1280000 } },
	{ "DrawComboCloaked",        0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "Quad",                    0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BITMAP },{ 0 } },
	{ "Triangle",                0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BITMAP },{} },
	{ "Quad3D",                  0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BITMAP },{} },
	{ "Triangle3D",              0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BITMAP },{} },
	{ "FastTile",                0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 1280000 } },
	{ "FastCombo",               0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 1280000 } },
	{ "DrawString",              0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_CHAR, ZTID_FLOAT },{ 1280000 } },
	{ "DrawString",              1,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawLayer",               0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0, 1280000 } },
	{ "DrawScreen",              0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "Blit",                    0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_UNTYPED, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{ 0, 0, 0, 0, 0, 10000 } },
	{ "BlitTo",                  0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_UNTYPED, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{ 0, 0, 0, 0, 0, 10000 } },
	{ "DrawPlane",               0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_UNTYPED, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{ 10000, 10000, 10000 } },
	{ "isValid",                 0,          ZTID_BOOL,   -1,                  0,  { ZTID_BITMAP },{} },
	{ "isAllocated",             0,          ZTID_BOOL,   -1,                  0,  { ZTID_BITMAP },{} },
	{ "Write",                   0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_CHAR, ZTID_BOOL },{ 0 } },
	{ "Read",                    0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_CHAR },{} },
	{ "Create",                  0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0, 2560000, 2560000 } },
	{ "Polygon",                 0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 1280000 } },
	{ "ClearToColor",            0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "Free",                    0,          ZTID_VOID,   -1,            FL_DEPR,  { ZTID_BITMAP },{},0,"Free() no longer does anything as of ZC 3.0. Objects are now freed automatically." },
	{ "Own",                     0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP },{} },
	{ "DrawFrame",               0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_FLOAT },{ 10000, 1280000 } },
	{ "WriteTile",               0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL, ZTID_BOOL },{ 10000, 0 } },
	
	{ "Dither",                  0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "ReplaceColors",           0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "ShiftColors",             0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "MaskedDraw",              0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_BITMAP, ZTID_FLOAT },{} },
	{ "MaskedDraw",              1,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "MaskedDraw",              2,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "MaskedBlit",              0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_BITMAP, ZTID_BITMAP, ZTID_BOOL },{} },
	{ "MaskedBlit",              1,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_BITMAP, ZTID_BITMAP, ZTID_BOOL, ZTID_FLOAT },{} },
	{ "MaskedBlit",              2,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_BITMAP, ZTID_BITMAP, ZTID_BOOL, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "Clear",                   0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT }, { 0 } },
	
	{ "DrawScreenSolid",         0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "DrawScreenSolidity",      0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "DrawScreenComboTypes",    0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "DrawScreenComboFlags",    0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	{ "DrawScreenComboIFlags",   0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{ 0 } },
	
	//Undocumented? Why?
	{ "DrawLayerSolid",          0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawLayerSolidity",       0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawLayerComboTypes",     0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawLayerComboFlags",     0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "DrawLayerComboIFlags",    0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	//Intentionally Undocumented
	{ "RevBlit",                 0,          ZTID_VOID,   -1,                  0,  { ZTID_BITMAP, ZTID_FLOAT, ZTID_BITMAP, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_FLOAT, ZTID_BOOL },{ 0, 0, 0, 0, 0, 10000 } },
	
	{ "",                        0,          ZTID_VOID,   -1,                  0,  {},{} }
};

BitmapSymbols::BitmapSymbols()
{
    table = BitmapTable;
	refVar = REFBITMAP;
}

void BitmapSymbols::generateCode()
{
	//void GetPixel(bitmap, x, y)
	{
		Function* function = getFunction("GetPixel");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		//pop pointer to EXP1
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		addOpcode2 (code, new OGraphicsGetpixel(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void CountColor(bitmap, bitmap, int, int, int, int)
	{
		Function* function = getFunction("CountColor");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OGraphicsCountColor(new VarArgument(EXP1)));
		LABELBACK(label);
		POP_ARGS(6, REFBITMAP);
		RETURN();
		function->giveCode(code);
	}
	/*
	//int32_t Create(bitmap, int32_t map,int32_t scr)
	{
		Function* function = getFunction("Create");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		//pop pointer, and ignore it
		POPREF();
		addOpcode2 (code, new OSetRegister(new VarArgument(EXP1), new VarArgument(CREATEBITMAP)));
		RETURN();
		function->giveCode(code);
	}
	*/
	 //void Rectangle(bitmap, float, float, float, float, float, float, float, float, float, float, bool, float)
	{
		Function* function = getFunction("Rectangle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPRectangleRegister());
		LABELBACK(label);
		POP_ARGS(12, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		RETURN();
        
		function->giveCode(code);
	}
	//void DrawFrame(bitmap, float, float, float, float, float, float, float, bool, float)
	{
		Function* function = getFunction("DrawFrame");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPFrameRegister());
		LABELBACK(label);
		POP_ARGS(9, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		RETURN();
        
		function->giveCode(code);
	}
	//void Read(bitmap, layer, "filename")
	{
		Function* function = getFunction("Read");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OReadBitmap());
		REASSIGN_PTR(EXP2);
		LABELBACK(label);
		POP_ARGS(2, NUL);
		//pop pointer, and ignore it
		POPREF();
		RETURN();
		function->giveCode(code);

	}
	//void Clear(bitmap, layer)
	{
		Function* function = getFunction("Clear");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OClearBitmap());
		LABELBACK(label);
		POP_ARGS(1, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		
		RETURN();
		function->giveCode(code);

	}
	//void Create(bitmap, layer, int32_t h, int32_t w)
	{
		Function* function = getFunction("Create");
		
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new ORegenerateBitmap());
		REASSIGN_PTR(EXP2);
		LABELBACK(label);
		POP_ARGS(3, NUL);
		//pop pointer, and ignore it
		POPREF();
		
		RETURN();
		function->giveCode(code);

	}
	//void Write(bitmap, layer, "filename")
	{
		Function* function = getFunction("Write");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OWriteBitmap());
		LABELBACK(label);
		POP_ARGS(3, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		RETURN();
        
		function->giveCode(code);
	}
	//void Circle(bitmap, float, float, float, float, float, float, float, float, float, bool, float)
	{
		Function* function = getFunction("Circle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPCircleRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Arc(bitmap, float, float, float, float, float, float, float, float, float, float, float, bool, bool, float)
	{
		Function* function = getFunction("Arc");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPArcRegister());
		LABELBACK(label);
		POP_ARGS(14, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Ellipse(bitmap, float, float, float, float, float, bool, float, float, float)
	{
		Function* function = getFunction("Ellipse");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPEllipseRegister());
		LABELBACK(label);
		POP_ARGS(12, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Line(bitmap, float, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("Line");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPLineRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Spline(bitmap, float, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("Spline");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPSplineRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void PutPixel(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("PutPixel");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPPutPixelRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawCharacter(bitmap, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawCharacter");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawCharRegister());
		LABELBACK(label);
		POP_ARGS(10, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawInteger(bitmap, float, float, float, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawInteger");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawIntRegister());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawTile(bitmap, float, float, float, float, float, bool, float, float, float)
	{
		Function* function = getFunction("DrawTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawTileRegister());
		LABELBACK(label);
		POP_ARGS(15, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawTileCloaked(bitmap, ...args)
	{
		Function* function = getFunction("DrawTileCloaked");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawTileCloakedRegister());
		LABELBACK(label);
		POP_ARGS(7, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawCombo(bitmap, float, float, float, float, float, bool, float, float, float)
	{
		Function* function = getFunction("DrawCombo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawComboRegister());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawComboCloaked(bitmap, ...args)
	{
		Function* function = getFunction("DrawComboCloaked");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawComboCloakedRegister());
		LABELBACK(label);
		POP_ARGS(7, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Quad(bitmap, float, float, float, float, float, float, float, float, float, bitmap)
	{
		Function* function = getFunction("Quad");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPQuadRegister());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Polygon(bitmap, float, float, float, float, float)
	
	{
		Function* function = getFunction("Polygon");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPPolygonRegister());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
    
	//void Triangle(bitmap, float, float, float, float, float, float, float, float, float, bitmap)
	{
		Function* function = getFunction("Triangle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPTriangleRegister());
		LABELBACK(label);
		POP_ARGS(14, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
    
	//void Quad3D(bitmap, float, float, float, float, float, float, float, float, float, bitmap)
	{
		Function* function = getFunction("Quad3D");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPQuad3DRegister());
		LABELBACK(label);
		POP_ARGS(9, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void Triangle3D(bitmap, float, float, float, float, float, float, float, float, float, bitmap)
	{
		Function* function = getFunction("Triangle3D");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPTriangle3DRegister());
		LABELBACK(label);
		POP_ARGS(9, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
    
	//void FastTile(bitmap, float, float, float, float, float)
	{
		Function* function = getFunction("FastTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPFastTileRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void FastCombo(bitmap, float, float, float, float, float)
	{
		Function* function = getFunction("FastCombo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPFastComboRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawString(bitmap, float, float, float, float, float, float, float, int32_t *string)
	{
		Function* function = getFunction("DrawString");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawStringRegister());
		LABELBACK(label);
		POP_ARGS(9, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawString(bitmap, float, float, float, float, float, float, float, int32_t *string)
	{
		Function* function = getFunction("DrawString", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawString2Register());
		LABELBACK(label);
		POP_ARGS(11, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayer(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayer");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawLayerRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayerComboIFlags(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayerComboIFlags");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenCIFlagRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayerComboFlags(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayerComboFlags");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenCFlagRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayerSolid(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayerSolid");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenSolidMaskRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawLayerComboTypes(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayerComboTypes");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenCTypeRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void DrawLayerSolidity(bitmap, float, float, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawLayerSolidity");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenSolidityRegister());
		LABELBACK(label);
		POP_ARGS(8, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawScreen(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreen");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void DrawScreenSolidity(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreenSolidity");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenSolidRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void DrawScreenSolid(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreenSolid");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenSolid2Register());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawScreenComboTypes(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreenComboTypes");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenComboTRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawScreenComboFlags(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreenComboFlags");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenComboFRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawScreenComboFlags(bitmap, float, float, float, float, float, float)
	{
		Function* function = getFunction("DrawScreenComboIFlags");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawScreenComboIRegister());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
    
	//void DrawBitmapEx(bitmap, float, float, float, float, float, float, float, float, float, float, bool)
	{
		Function* function = getFunction("Blit");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPDrawBitmapExRegister());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		POPREF();
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawPlane(bitmap, float, float, float, float, float, float, float, float, float, float, bool)
	{
		Function* function = getFunction("DrawPlane");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPMode7());
		LABELBACK(label);
		POP_ARGS(13, NUL);
		//pop pointer, and ignore it
		POPREF();
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawBitmapEx(bitmap, float, float, float, float, float, float, float, float, float, float, bool)
	{
		Function* function = getFunction("BlitTo");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPBlitTO());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		POPREF();
        
		RETURN();
		function->giveCode(code);
	}
	//void DrawBitmapEx(bitmap, float, float, float, float, float, float, float, float, float, float, bool)
	{
		Function* function = getFunction("RevBlit");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBMPBlitTO());
		LABELBACK(label);
		POP_ARGS(16, NUL);
		//pop pointer, and ignore it
		POPREF();
        
		RETURN();
		function->giveCode(code);
	}
	//bool isValid(bitmap)
	{
		Function* function = getFunction("isValid");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsValidBitmap(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool isAllocated(bitmap)
	{
		Function* function = getFunction("isAllocated");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the pointer
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//Check validity
		addOpcode2 (code, new OIsAllocatedBitmap(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void ClearToColor(bitmap, layer, color)
	{
		Function* function = getFunction("ClearToColor");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapClearToColor());
		LABELBACK(label);
		POP_ARGS(2, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		
		RETURN();
		function->giveCode(code);

	}
	//void Free(bitmap)
	{
		Function* function = getFunction("Free");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OBitmapFree());
		RETURN();
		function->giveCode(code);
	}
	//void Own(bitmap)
	{
		Function* function = getFunction("Own");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OBitmapOwn());
		RETURN();
		function->giveCode(code);
	}
	
	//void WriteTile(bitmap, int32_t, int32_t, int32_t, int32_t, bool, bool)
	{
		Function* function = getFunction("WriteTile");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapWriteTile());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void Dither(bitmap, int32_t, int32_t, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("Dither");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapDither());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void ReplaceColors(bitmap, int32_t, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("ReplaceColors");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapReplColor());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void ShiftColors(bitmap, int32_t, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("ShiftColors");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapShiftColor());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	
	//void MaskedDraw(bitmap, int32_t, bitmap, int32_t)
	{
		Function* function = getFunction("MaskedDraw");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskDraw());
		LABELBACK(label);
		POP_ARGS(3, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void MaskedDraw(bitmap, int32_t, bitmap, int32_t, int32_t)
	{
		Function* function = getFunction("MaskedDraw", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskDraw2());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void MaskedDraw(bitmap, int32_t, bitmap, int32_t, int32_t, int32_t)
	{
		Function* function = getFunction("MaskedDraw", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskDraw3());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void MaskedBlit(bitmap, int32_t, bitmap, bitmap)
	{
		Function* function = getFunction("MaskedBlit");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskBlit());
		LABELBACK(label);
		POP_ARGS(4, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void MaskedBlit(bitmap, int32_t, bitmap, bitmap, int32_t)
	{
		Function* function = getFunction("MaskedBlit", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskBlit2());
		LABELBACK(label);
		POP_ARGS(5, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
	//void MaskedBlit(bitmap, int32_t, bitmap, bitmap, int32_t, int32_t)
	{
		Function* function = getFunction("MaskedBlit", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OBitmapMaskBlit3());
		LABELBACK(label);
		POP_ARGS(6, NUL);
		//pop pointer, and ignore it
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
        
		RETURN();
		function->giveCode(code);
	}
}


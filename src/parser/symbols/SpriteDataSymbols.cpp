#include "SymbolDefs.h"

SpriteDataSymbols SpriteDataSymbols::singleton = SpriteDataSymbols();

static AccessorTable SpriteDataTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getTile",                    0,         ZTID_FLOAT,   SPRITEDATATILE,            0,  { ZTID_SPRITEDATA },{} },
	{ "setTile",                    0,          ZTID_VOID,   SPRITEDATATILE,            0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getMisc",                    0,         ZTID_FLOAT,   SPRITEDATAMISC,            0,  { ZTID_SPRITEDATA },{} },
	{ "setMisc",                    0,          ZTID_VOID,   SPRITEDATAMISC,            0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   SPRITEDATACSETS,           0,  { ZTID_SPRITEDATA },{} },
	{ "setCSet",                    0,          ZTID_VOID,   SPRITEDATACSETS,           0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getFrames",                  0,         ZTID_FLOAT,   SPRITEDATAFRAMES,          0,  { ZTID_SPRITEDATA },{} },
	{ "setFrames",                  0,          ZTID_VOID,   SPRITEDATAFRAMES,          0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getSpeed",                   0,         ZTID_FLOAT,   SPRITEDATASPEED,           0,  { ZTID_SPRITEDATA },{} },
	{ "setSpeed",                   0,          ZTID_VOID,   SPRITEDATASPEED,           0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getType",                    0,         ZTID_FLOAT,   SPRITEDATATYPE,            0,  { ZTID_SPRITEDATA },{} },
	{ "setType",                    0,          ZTID_VOID,   SPRITEDATATYPE,            0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

SpriteDataSymbols::SpriteDataSymbols()
{
	table = SpriteDataTable;
	refVar = REFSPRITEDATA;
}

void SpriteDataSymbols::generateCode()
{
}


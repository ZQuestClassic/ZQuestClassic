#include "SymbolDefs.h"

SpriteDataSymbols SpriteDataSymbols::singleton = SpriteDataSymbols();

static AccessorTable SpriteDataTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getID",                      0,         ZTID_FLOAT,   SPRITEDATAID,              0,  { ZTID_SPRITEDATA },{} },
	{ "setID",                      0,          ZTID_VOID,   SPRITEDATAID,      FL_RDONLY,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getTile",                    0,         ZTID_FLOAT,   SPRITEDATATILE,            0,  { ZTID_SPRITEDATA },{} },
	{ "setTile",                    0,          ZTID_VOID,   SPRITEDATATILE,            0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getCSet",                    0,         ZTID_FLOAT,   SPRITEDATACSETS,           0,  { ZTID_SPRITEDATA },{} },
	{ "setCSet",                    0,          ZTID_VOID,   SPRITEDATACSETS,           0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getFlashCSet",               0,         ZTID_FLOAT,   SPRITEDATAFLCSET,          0,  { ZTID_SPRITEDATA },{} },
	{ "setFlashCSet",               0,          ZTID_VOID,   SPRITEDATAFLCSET,          0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getFrames",                  0,         ZTID_FLOAT,   SPRITEDATAFRAMES,          0,  { ZTID_SPRITEDATA },{} },
	{ "setFrames",                  0,          ZTID_VOID,   SPRITEDATAFRAMES,          0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getSpeed",                   0,         ZTID_FLOAT,   SPRITEDATASPEED,           0,  { ZTID_SPRITEDATA },{} },
	{ "setSpeed",                   0,          ZTID_VOID,   SPRITEDATASPEED,           0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getType",                    0,         ZTID_FLOAT,   SPRITEDATATYPE,            0,  { ZTID_SPRITEDATA },{} },
	{ "setType",                    0,          ZTID_VOID,   SPRITEDATATYPE,            0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "getFlags[]",                 0,          ZTID_BOOL,   SPRITEDATAFLAGS,           0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	{ "setFlags[]",                 0,          ZTID_VOID,   SPRITEDATAFLAGS,           0,  { ZTID_SPRITEDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	
	//Intentionally undocumented
	{ "getMisc",                    0,         ZTID_FLOAT,   SPRITEDATAMISC,            0,  { ZTID_SPRITEDATA },{} },
	{ "setMisc",                    0,          ZTID_VOID,   SPRITEDATAMISC,            0,  { ZTID_SPRITEDATA, ZTID_FLOAT },{} },
	
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


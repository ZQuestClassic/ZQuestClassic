#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../ByteCode.h"
#include "../CompilerUtils.h"
#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;

Screen const& Screen::singleton()
{
	static Screen const instance;
	return instance;
}

void Screen::addTo(Scope& scope) const
{
	TypeStore& typeStore = scope.getTypeStore();
	DataType tVoid = typeStore.getVoid();
	DataType tBool = typeStore.getBool();
	DataType tFloat = typeStore.getFloat();
	DataType tGame = typeStore.getGame();
	DataType tDebug = typeStore.getDebug();
	DataType tScreen = typeStore.getScreen();
	DataType tAudio = typeStore.getAudio();
	DataType tLink = typeStore.getLink();
	DataType tItemClass = typeStore.getItemClass();
	DataType tItem = typeStore.getItem();
	DataType tNpcClass = typeStore.getNpcClass();
	DataType tNpc = typeStore.getNpc();
	DataType tFfc = typeStore.getFfc();
	DataType tLWpn = typeStore.getLWpn();
	DataType tEWpn = typeStore.getEWpn();

	typedef VectorBuilder<DataType> P;
	typedef VectorBuilder<int> R;
	typedef VectorBuilder<Opcode*> O;
	
	LibraryHelper lh(scope, NUL, tScreen);
	LibraryHelper::call_tag const& asFunction = LibraryHelper::asFunction;

	addPair(lh, SDD, tFloat, "D", 8);
	addPair(lh, COMBODD, tFloat, "ComboD", 176);
	addPair(lh, COMBOCD, tFloat, "ComboC", 176);
	addPair(lh, COMBOFD, tFloat, "ComboF", 176);
	addPair(lh, COMBOID, tFloat, "ComboI", 176);
	addPair(lh, COMBOTD, tFloat, "ComboT", 176);
	addPair(lh, COMBOSD, tFloat, "ComboS", 176);
	addPair(lh, SCRDOORD, tFloat, "Door", 4);
	addPair(lh, SCREENSTATED, tBool, "State", 32);
	addPair(lh, LIT, tBool, "Lit");
	addPair(lh, WAVY, tFloat, "Wavy");
	addPair(lh, QUAKE, tFloat, "Quake");
	lh.addGetter(ITEMCOUNT, tFloat, "NumItems", asFunction);
	lh.addGetter(NPCCOUNT, tFloat, "NumNPCs", asFunction);
	lh.addGetter(LWPNCOUNT, tFloat, "NumLWeapons", asFunction);
	lh.addGetter(EWPNCOUNT, tFloat, "NumEWeapons", asFunction);
	addPair(lh, SCREENFLAGSD, tFloat, "Flags", 10);
	addPair(lh, SCREENEFLAGSD, tFloat, "EFlags", 3);
	lh.addGetter(ROOMTYPE, tFloat, "RoomType"); // make pair?
	addPair(lh, ROOMDATA, tFloat, "RoomData");
	addPair(lh, PUSHBLOCKX, tFloat, "MovingBlockX");
	addPair(lh, PUSHBLOCKY, tFloat, "MovingBlockY");
	addPair(lh, PUSHBLOCKCOMBO, tFloat, "MovingBlockCombo");
	addPair(lh, PUSHBLOCKCSET, tFloat, "MovingBlockCSet");
	addPair(lh, UNDERCOMBO, tFloat, "UnderCombo");
	addPair(lh, UNDERCSET, tFloat, "UnderCSet");

    // item Screen->LoadItem(float index)
	defineFunction(
			lh, tItem, "LoadItem",
			P() << tFloat, R() << EXP1,
			O() << new OSubImmediate(new VarArgument(EXP1),
			                         new LiteralArgument(10000))
			    << new OLoadItemRegister(new VarArgument(EXP1))
			    << new OSetRegister(new VarArgument(EXP1),
			                        new VarArgument(REFITEM)));
    
    // item Screen->CreateItem(float id)
	defineFunction(
			lh, tItem, "CreateItem",
			P() << tFloat, R() << EXP1,
			O() << new OCreateItemRegister(new VarArgument(EXP1))
			    << new OSetRegister(new VarArgument(EXP1),
			                        new VarArgument(REFITEM)));

    // ffc Screen->LoadFFC(float index)
	defineFunction(
			lh, tFfc, "LoadFFC",
			P() << tFloat, R() << EXP1,
			new OSubImmediate(new VarArgument(EXP1),
			                  new LiteralArgument(10000)));

    // npc Screen->LoadNPC(float index)
	defineFunction(
			lh, tNpc, "LoadNPC",
			P() << tFloat, R() << EXP1,
			O() << new OSubImmediate(new VarArgument(EXP1),
			                         new LiteralArgument(10000))
			    << new OLoadNPCRegister(new VarArgument(EXP1))
			    << new OSetRegister(new VarArgument(EXP1),
			                        new VarArgument(REFNPC)));

    // npc Screen->CreateNPC(float id)
	defineFunction(
			lh, tNpc, "CreateNPC",
			P() << tFloat, R() << EXP1,
			O() << new OCreateNPCRegister(new VarArgument(EXP1))
			    << new OSetRegister(new VarArgument(EXP1),
			                        new VarArgument(REFNPC)));

    // lweapon Screen->LoadLWeapon(float index)
	defineFunction(
			lh, tLWpn, "LoadLWeapon",
			P() << tFloat, R() << EXP1,
			O() << new OSubImmediate(new VarArgument(EXP1),
			                         new LiteralArgument(10000))
			    << new OLoadLWpnRegister(new VarArgument(EXP1))
			    << new OSetRegister(new VarArgument(EXP1),
			                        new VarArgument(REFLWPN)));

    // lweapon Screen->CreateLWeapon(float id)
	defineFunction(
			lh, tLWpn, "CreateLWeapon",
			P() << tFloat, R() << EXP1,
			O() << new OCreateLWpnRegister(new VarArgument(EXP1))
			    << new OSetRegister(new VarArgument(EXP1),
			                        new VarArgument(REFLWPN)));
    
    // lweapon Screen->CreateLWeaponDX(float type, float id)
    defineFunction(
		    lh, tLWpn, "CreateLWeaponDx",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(CREATELWPNDX)));
     
    // eweapon Screen->LoadEWeapon(float index)
	defineFunction(
			lh, tEWpn, "LoadEWeapon",
			P() << tFloat, R() << EXP1,
			O() << new OSubImmediate(new VarArgument(EXP1),
			                         new LiteralArgument(10000))
			    << new OLoadEWpnRegister(new VarArgument(EXP1))
			    << new OSetRegister(new VarArgument(EXP1),
			                        new VarArgument(REFEWPN)));

    // eweapon Screen->CreateEWeapon(float id)
	defineFunction(
			lh, tEWpn, "CreateEWeapon",
			P() << tFloat, R() << EXP1,
			O() << new OCreateEWpnRegister(new VarArgument(EXP1))
			    << new OSetRegister(new VarArgument(EXP1),
			                        new VarArgument(REFEWPN)));

    // void Screen->ClearSprites(float spriteListId)
    defineFunction(
		    lh, tVoid, "ClearSprites",
		    P() << tFloat, R() << EXP1,
		    new OClearSpritesRegister(new VarArgument(EXP1)));

    // void Screen->Rectangle(...)
    defineFunction(
		    lh, tVoid, "Rectangle",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tBool  << tFloat,
		    new ORectangleRegister());

    // void Screen->Circle(...)
    defineFunction(
		    lh, tVoid, "Circle",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tBool  << tFloat,
		    new OCircleRegister());

    // void Screen->Arc(...)
    defineFunction(
		    lh, tVoid, "Arc",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tBool
		        << tBool  << tFloat,
		    new OArcRegister());

    // void Screen->Ellipse(...)
    defineFunction(
		    lh, tVoid, "Ellipse",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tBool << tFloat,
		    new OEllipseRegister());

    // void Screen->Line(...)
    defineFunction(
		    lh, tVoid, "Line",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat,
		    new OLineRegister());
	    
    // void Screen->Spline(...)
    defineFunction(
		    lh, tVoid, "Spline",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat,
		    new OSplineRegister());

    // void Screen->PutPixel(...)
    defineFunction(
		    lh, tVoid, "PutPixel",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat,
		    new OPutPixelRegister());

    // void Screen->DrawCharacter(...)
    defineFunction(
		    lh, tVoid, "DrawCharacter",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat,
		    new ODrawCharRegister());
    
    // void Screen->DrawInteger(...)
    defineFunction(
		    lh, tVoid, "DrawInteger",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat,
		    new ODrawIntRegister());

    // void Screen->DrawTile(...)
    defineFunction(
		    lh, tVoid, "DrawTile",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tBool << tFloat,
		    new ODrawTileRegister());
    
    // void Screen->DrawCombo(...)
    defineFunction(
		    lh, tVoid, "DrawCombo",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tBool << tFloat,
		    new ODrawComboRegister());
    
    // void Screen->Quad(...)
    defineFunction(
		    lh, tVoid, "Quad",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat,
		    new OQuadRegister());

    // void Screen->Polygon(...)
    defineFunction(
		    lh, tVoid, "Polygon",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat,
		    new OPolygonRegister());

    // void Screen->Triangle(...)
    defineFunction(
		    lh, tVoid, "Triangle",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat,
		    new OTriangleRegister());
    
    // void Screen->Quad3D(...)
    defineFunction(
		    lh, tVoid, "Quad3D",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat,
		    new OQuad3DRegister());
    
    // void Screen->Triangle3D(...)
    defineFunction(
		    lh, tVoid, "Triangle3D",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat,
		    new OTriangle3DRegister());
    
    // void Screen->FastTile(...)
    defineFunction(
		    lh, tVoid, "FastTile",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat,
		    new OFastTileRegister());
    
    // void Screen->FastCombo(...)
    defineFunction(
		    lh, tVoid, "FastCombo",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat,
		    new OFastComboRegister());

    // void Screen->DrawString(...)
    defineFunction(
		    lh, tVoid, "DrawString",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
		        << tFloat,
		    new ODrawStringRegister());
    
    // void Screen->DrawLayer(...)
    defineFunction(
		    lh, tVoid, "DrawLayer",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat,
		    new ODrawLayerRegister());
    
    // void Screen->DrawScreen(...)
    defineFunction(
		    lh, tVoid, "DrawScreen",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat,
		    new ODrawScreenRegister());

    // void Screen->DrawBitmap(...)
    defineFunction(
		    lh, tVoid, "DrawBitmap",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tBool,
		    new ODrawBitmapRegister());
    
    // void Screen->DrawBitmapEx(...)
    defineFunction(
		    lh, tVoid, "DrawBitmapEx",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tBool,
		    new ODrawBitmapExRegister());
    
    // void Screen->SetRenderTarget(float bitmap)
    defineFunction(
		    lh, tVoid, "SetRenderTarget", P() << tFloat,
		    new OSetRenderTargetRegister());
    
    // void Screen->Message(float id)
    defineFunction(
		    lh, tVoid, "Message",
		    P() << tFloat, R() << EXP2,
		    new OMessageRegister(new VarArgument(EXP2)));

    // bool Screen->isSolid(float x, float y)
    defineFunction(
		    lh, tBool, "isSolid",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OIsSolid(new VarArgument(EXP1)));

    // void Screen->SetSideWarp(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetSideWarp",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    new OSetSideWarpRegister());

    // void Screen->SetTileWarp(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetTileWarp",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    new OSetTileWarpRegister());
    
    // float Screen->LayerScreen(float layer)
    defineFunction(
		    lh, tFloat, "LayerScreen",
		    P() << tFloat, R() << EXP2,
		    new OLayerScreenRegister(new VarArgument(EXP1),
		                             new VarArgument(EXP2)));
    
    // float Screen->LayerMap(float layer)
    defineFunction(
		    lh, tFloat, "LayerMap",
		    P() << tFloat, R() << EXP2,
		    new OLayerMapRegister(new VarArgument(EXP1),
		                          new VarArgument(EXP2)));

    // void Screen->TriggerSecrets()
    defineFunction(lh, "TriggerSecrets", new OTriggerSecrets());
    defineFunction(lh, "ZapIn",          new OZapIn());
    defineFunction(lh, "ZapOut",         new OZapOut());
    defineFunction(lh, "OpeningWipe",    new OOpenWipe());
    defineFunction(lh, "WavyIn",         new OWavyIn());
    defineFunction(lh, "WavyOut",        new OWavyOut());
    
    // float Screen->GetSideWarpDMap(float warp)
    defineFunction(
		    lh, tFloat, "GetSideWarpDMap",
		    P() << tFloat, R() << EXP1,
		    new OGetSideWarpDMap(new VarArgument(EXP1)));

    // float Screen->GetSideWarpScreen(screen this, float warp)
    defineFunction(
		    lh, tFloat, "GetSideWarpScreen",
		    P() << tFloat, R() << EXP1,
		    new OGetSideWarpScreen(new VarArgument(EXP1)));

    // float Screen->GetSideWarpType(float warp)
    defineFunction(
		    lh, tFloat, "GetSideWarpType",
		    P() << tFloat, R() << EXP1,
		    new OGetSideWarpType(new VarArgument(EXP1)));

    // float Screen->GetTileWarpDMap(float warp)
    defineFunction(
		    lh, tFloat, "GetTileWarpDMap",
		    P() << tFloat, R() << EXP1,
		    new OGetTileWarpDMap(new VarArgument(EXP1)));

    // float Screen->GetTileWarpScreen(float warp)
    defineFunction(
		    lh, tFloat, "GetTileWarpScreen",
		    P() << tFloat, R() << EXP1,
		    new OGetTileWarpScreen(new VarArgument(EXP1)));

    // float Screen->GetTileWarpType(float warp)
    defineFunction(
		    lh, tFloat, "GetTileWarpType",
		    P() << tFloat, R() << EXP1,
		    new OGetTileWarpType(new VarArgument(EXP1)));

    // void Screen->TriggerSecret(float secret)
    defineFunction(
		    lh, tVoid, "TriggerSecret",
		    P() << tFloat, R() << EXP1,
		    new OTriggerSecretRegister(new VarArgument(EXP1)));
}

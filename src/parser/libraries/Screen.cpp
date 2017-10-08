#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

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
	typedef VectorBuilder<ZAsm::Variable> R;
	typedef VectorBuilder<Opcode> O;
	
	LibraryHelper lh(scope, varNull(), tScreen);
	LibraryHelper::call_tag const& asFunction = LibraryHelper::asFunction;

	addPair(lh, varSDD(), tFloat, "D", 8);
	addPair(lh, varCOMBODD(), tFloat, "ComboD", 176);
	addPair(lh, varCOMBOCD(), tFloat, "ComboC", 176);
	addPair(lh, varCOMBOFD(), tFloat, "ComboF", 176);
	addPair(lh, varCOMBOID(), tFloat, "ComboI", 176);
	addPair(lh, varCOMBOTD(), tFloat, "ComboT", 176);
	addPair(lh, varCOMBOSD(), tFloat, "ComboS", 176);
	addPair(lh, varSCRDOORD(), tFloat, "Door", 4);
	addPair(lh, varSCREENSTATED(), tBool, "State", 32);
	addPair(lh, varLIT(), tBool, "Lit");
	addPair(lh, varWAVY(), tFloat, "Wavy");
	addPair(lh, varQUAKE(), tFloat, "Quake");
	lh.addGetter(varITEMCOUNT(), tFloat, "NumItems", asFunction);
	lh.addGetter(varNPCCOUNT(), tFloat, "NumNPCs", asFunction);
	lh.addGetter(varLWPNCOUNT(), tFloat, "NumLWeapons", asFunction);
	lh.addGetter(varEWPNCOUNT(), tFloat, "NumEWeapons", asFunction);
	addPair(lh, varSCREENFLAGSD(), tFloat, "Flags", 10);
	addPair(lh, varSCREENEFLAGSD(), tFloat, "EFlags", 3);
	lh.addGetter(varROOMTYPE(), tFloat, "RoomType"); // make pair?
	addPair(lh, varROOMDATA(), tFloat, "RoomData");
	addPair(lh, varPUSHBLOCKX(), tFloat, "MovingBlockX");
	addPair(lh, varPUSHBLOCKY(), tFloat, "MovingBlockY");
	addPair(lh, varPUSHBLOCKCOMBO(), tFloat, "MovingBlockCombo");
	addPair(lh, varPUSHBLOCKCSET(), tFloat, "MovingBlockCSet");
	addPair(lh, varUNDERCOMBO(), tFloat, "UnderCombo");
	addPair(lh, varUNDERCSET(), tFloat, "UnderCSet");

    // item Screen->LoadItem(float index)
	defineFunction(
			lh, tItem, "LoadItem",
			P() << tFloat, R() << varExp1(),
			O() << opSUBV(varExp1(), 10000)
			    << opLOADITEMR(varExp1())
			    << opSETR(varExp1(), varREFITEM()));
    
    // item Screen->CreateItem(float id)
	defineFunction(
			lh, tItem, "CreateItem",
			P() << tFloat, R() << varExp1(),
			O() << opCREATEITEMR(varExp1())
			    << opSETR(varExp1(), varREFITEM()));

    // ffc Screen->LoadFFC(float index)
	defineFunction(
			lh, tFfc, "LoadFFC",
			P() << tFloat, R() << varExp1(),
			opSUBV(varExp1(), 10000));

    // npc Screen->LoadNPC(float index)
	defineFunction(
			lh, tNpc, "LoadNPC",
			P() << tFloat, R() << varExp1(),
			O() << opSUBV(varExp1(), 10000)
			    << opLOADNPCR(varExp1())
			    << opSETR(varExp1(), varREFNPC()));

    // npc Screen->CreateNPC(float id)
	defineFunction(
			lh, tNpc, "CreateNPC",
			P() << tFloat, R() << varExp1(),
			O() << opCREATENPCR(varExp1())
			    << opSETR(varExp1(), varREFNPC()));

    // lweapon Screen->LoadLWeapon(float index)
	defineFunction(
			lh, tLWpn, "LoadLWeapon",
			P() << tFloat, R() << varExp1(),
			O() << opSUBV(varExp1(), 10000)
			    << opLOADLWEAPONR(varExp1())
			    << opSETR(varExp1(), varREFLWPN()));

    // lweapon Screen->CreateLWeapon(float id)
	defineFunction(
			lh, tLWpn, "CreateLWeapon",
			P() << tFloat, R() << varExp1(),
			O() << opCREATELWEAPONR(varExp1())
			    << opSETR(varExp1(), varREFLWPN()));
    
    // lweapon Screen->CreateLWeaponDX(float type, float id)
    defineFunction(
		    lh, tLWpn, "CreateLWeaponDx",
		    P() << tFloat   << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opSETR(varExp1(), varCREATELWPNDX()));
     
    // eweapon Screen->LoadEWeapon(float index)
	defineFunction(
			lh, tEWpn, "LoadEWeapon",
			P() << tFloat, R() << varExp1(),
			O() << opSUBV(varExp1(), 10000)
			    << opLOADEWEAPONR(varExp1())
			    << opSETR(varExp1(), varREFEWPN()));

    // eweapon Screen->CreateEWeapon(float id)
	defineFunction(
			lh, tEWpn, "CreateEWeapon",
			P() << tFloat, R() << varExp1(),
			O() << opCREATEEWEAPONR(varExp1())
			    << opSETR(varExp1(), varREFEWPN()));

    // void Screen->ClearSprites(float spriteListId)
    defineFunction(
		    lh, tVoid, "ClearSprites",
		    P() << tFloat, R() << varExp1(),
		    opCLEARSPRITESR(varExp1()));

    // void Screen->Rectangle(...)
    defineFunction(
		    lh, tVoid, "Rectangle",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tBool  << tFloat,
		    opRECTR());

    // void Screen->Circle(...)
    defineFunction(
		    lh, tVoid, "Circle",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tBool  << tFloat,
		    opCIRCLER());

    // void Screen->Arc(...)
    defineFunction(
		    lh, tVoid, "Arc",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tBool
		        << tBool  << tFloat,
		    opARCR());

    // void Screen->Ellipse(...)
    defineFunction(
		    lh, tVoid, "Ellipse",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tBool << tFloat,
		    opELLIPSER());

    // void Screen->Line(...)
    defineFunction(
		    lh, tVoid, "Line",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat,
		    opLINER());
	    
    // void Screen->Spline(...)
    defineFunction(
		    lh, tVoid, "Spline",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat,
		    opSPLINER());

    // void Screen->PutPixel(...)
    defineFunction(
		    lh, tVoid, "PutPixel",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat,
		    opPUTPIXELR());

    // void Screen->DrawCharacter(...)
    defineFunction(
		    lh, tVoid, "DrawCharacter",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat,
		    opDRAWCHARR());
    
    // void Screen->DrawInteger(...)
    defineFunction(
		    lh, tVoid, "DrawInteger",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat,
		    opDRAWINTR());

    // void Screen->DrawTile(...)
    defineFunction(
		    lh, tVoid, "DrawTile",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tBool << tFloat,
		    opDRAWTILER());
    
    // void Screen->DrawCombo(...)
    defineFunction(
		    lh, tVoid, "DrawCombo",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tBool << tFloat,
		    opDRAWCOMBOR());
    
    // void Screen->Quad(...)
    defineFunction(
		    lh, tVoid, "Quad",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat,
		    opQUADR());

    // void Screen->Polygon(...)
    defineFunction(
		    lh, tVoid, "Polygon",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat,
		    opPOLYGONR());

    // void Screen->Triangle(...)
    defineFunction(
		    lh, tVoid, "Triangle",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat,
		    opTRIANGLER());
    
    // void Screen->Quad3D(...)
    defineFunction(
		    lh, tVoid, "Quad3D",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat,
		    opQUAD3DR());
    
    // void Screen->Triangle3D(...)
    defineFunction(
		    lh, tVoid, "Triangle3D",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat,
		    opTRIANGLE3DR());
    
    // void Screen->FastTile(...)
    defineFunction(
		    lh, tVoid, "FastTile",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat,
		    opFASTTILER());
    
    // void Screen->FastCombo(...)
    defineFunction(
		    lh, tVoid, "FastCombo",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat,
		    opFASTCOMBOR());

    // void Screen->DrawString(...)
    defineFunction(
		    lh, tVoid, "DrawString",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
		        << tFloat,
		    opDRAWSTRINGR());
    
    // void Screen->DrawLayer(...)
    defineFunction(
		    lh, tVoid, "DrawLayer",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat,
		    opDRAWLAYERR());
    
    // void Screen->DrawScreen(...)
    defineFunction(
		    lh, tVoid, "DrawScreen",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat,
		    opDRAWSCREENR());

    // void Screen->DrawBitmap(...)
    defineFunction(
		    lh, tVoid, "DrawBitmap",
		    P() << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tFloat
			    << tFloat << tFloat << tFloat << tBool,
		    opBITMAPR());
    
    // void Screen->DrawBitmapEx(...)
    defineFunction(
		    lh, tVoid, "DrawBitmapEx",
		    P() << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tFloat
		        << tFloat << tFloat << tFloat << tBool,
		    opBITMAPEXR());
    
    // void Screen->SetRenderTarget(float bitmap)
    defineFunction(
		    lh, tVoid, "SetRenderTarget", P() << tFloat,
		    opSETRENDERTARGET());
    
    // void Screen->Message(float id)
    defineFunction(
		    lh, tVoid, "Message",
		    P() << tFloat, R() << varExp2(),
		    opMSGSTRR(varExp2()));

    // bool Screen->isSolid(float x, float y)
    defineFunction(
		    lh, tBool, "isSolid",
		    P() << tFloat << tFloat,
		    R() << varIndex1() << varIndex2(),
		    opISSOLID(varExp1()));

    // void Screen->SetSideWarp(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetSideWarp",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    opSETSIDEWARP());

    // void Screen->SetTileWarp(float, float, float, float)
    defineFunction(
		    lh, tVoid, "SetTileWarp",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    opSETTILEWARP());
    
    // float Screen->LayerScreen(float layer)
    defineFunction(
		    lh, tFloat, "LayerScreen",
		    P() << tFloat, R() << varExp2(),
		    opLAYERSCREEN(varExp1(), varExp2()));
    
    // float Screen->LayerMap(float layer)
    defineFunction(
		    lh, tFloat, "LayerMap",
		    P() << tFloat, R() << varExp2(),
		    opLAYERMAP(varExp1(), varExp2()));

    // void Screen->TriggerSecrets()
    defineFunction(lh, "TriggerSecrets", opSECRETS());
    defineFunction(lh, "ZapIn",          opZAPIN());
    defineFunction(lh, "ZapOut",         opZAPOUT());
    defineFunction(lh, "OpeningWipe",    opOPENWIPE());
    defineFunction(lh, "WavyIn",         opWAVYIN());
    defineFunction(lh, "WavyOut",        opWAVYOUT());
    
    // float Screen->GetSideWarpDMap(float warp)
    defineFunction(
		    lh, tFloat, "GetSideWarpDMap",
		    P() << tFloat, R() << varExp1(),
		    opGETSIDEWARPDMAP(varExp1()));

    // float Screen->GetSideWarpScreen(screen this, float warp)
    defineFunction(
		    lh, tFloat, "GetSideWarpScreen",
		    P() << tFloat, R() << varExp1(),
		    opGETSIDEWARPSCR(varExp1()));

    // float Screen->GetSideWarpType(float warp)
    defineFunction(
		    lh, tFloat, "GetSideWarpType",
		    P() << tFloat, R() << varExp1(),
		    opGETSIDEWARPTYPE(varExp1()));

    // float Screen->GetTileWarpDMap(float warp)
    defineFunction(
		    lh, tFloat, "GetTileWarpDMap",
		    P() << tFloat, R() << varExp1(),
		    opGETTILEWARPDMAP(varExp1()));

    // float Screen->GetTileWarpScreen(float warp)
    defineFunction(
		    lh, tFloat, "GetTileWarpScreen",
		    P() << tFloat, R() << varExp1(),
		    opGETTILEWARPSCR(varExp1()));

    // float Screen->GetTileWarpType(float warp)
    defineFunction(
		    lh, tFloat, "GetTileWarpType",
		    P() << tFloat, R() << varExp1(),
		    opGETTILEWARPTYPE(varExp1()));

    // void Screen->TriggerSecret(float secret)
    defineFunction(
		    lh, tVoid, "TriggerSecret",
		    P() << tFloat, R() << varExp1(),
		    opTRIGGERSECRETR(varExp1()));
}

#include "../../precompiled.h"
#include "../Library.h"
#include "../LibraryHelper.h"

#include "../Scope.h"

using namespace ZScript;
using namespace ZScript::Libraries;
using namespace ZAsm;

Libraries::Global const& Libraries::Global::singleton()
{
	static Libraries::Global const instance;
	return instance;
}

const int radsperdeg = 572958;

void Global::addTo(Scope& scope) const
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

	LibraryHelper lh(scope, varNull());
	
    // int Rand(int maxval)
	defineFunction(
			lh, tFloat, "Rand",
			P() << tFloat, R() << varExp2(),
			opRNDR(varExp1(), varExp2()));
    
    // void Quit()
    {
	    Function& function = lh.addFunction(tVoid, "Quit", P());
        vector<Opcode> code;
        code.push_back(opPOP(varExp2()));
        code.push_back(opQUIT());
        function.setCode(code);
    }
    
    // void Waitframe()
    defineFunction(lh, "Waitframe", opWAITFRAME());
    
    // void Waitdraw()
    defineFunction(lh, "Waitdraw", opWAITDRAW());

    // void Trace(int val)
    defineFunction(
		    lh, tVoid, "Trace",
		    P() << tFloat, R() << varExp2(),
		    opTRACER(varExp2()));

    // void TraceB(bool val)
    defineFunction(
		    lh, tVoid, "TraceB",
		    P() << tBool, R() << varExp2(),
		    opTRACE2R(varExp2()));

    // void TraceS(string val)
    defineFunction(
		    lh, tVoid, "TraceS",
		    P() << tFloat, R() << varExp2(),
		    opTRACE6(varExp2()));

    // void TraceNL()
    defineFunction(lh, "TraceNL", opTRACE3());

    // void ClearTrace()
    defineFunction(lh, "ClearTrace", opTRACE4());

    // void TraceToBase(float, float, float)
    defineFunction(
		    lh, tVoid, "TraceToBase",
		    P() << tFloat << tFloat << tFloat,
		    opTRACE5());

    // float Sin(float val)
    defineFunction(
		    lh, tFloat, "Sin",
		    P() << tFloat, R() << varExp2(),
		    opSINR(varExp1(), varExp2()));

    // float RadianSin(float val)
    defineFunction(
		    lh, tFloat, "RadianSin",
		    P() << tFloat, R() << varExp2(),
		    O() << opMULTV(varExp2(), radsperdeg)
		        << opSINR(varExp1(), varExp2()));

    // float ArcSin(float val)
    defineFunction(
		    lh, tFloat, "ArcSin",
		    P() << tFloat, R() << varExp2(),
		    opARCSINR(varExp1(), varExp2()));

    // float Cos(float val)
    defineFunction(
		    lh, tFloat, "Cos",
		    P() << tFloat, R() << varExp2(),
		    opCOSR(varExp1(), varExp2()));

    // float RadianCos(float val)
    defineFunction(
		    lh, tFloat, "RadianCos",
		    P() << tFloat, R() << varExp2(),
		    O() << opMULTV(varExp2(), radsperdeg)
		        << opCOSR(varExp1(), varExp2()));

    // float ArcCos(float val)
    defineFunction(
		    lh, tFloat, "ArcCos",
		    P() << tFloat, R() << varExp2(),
		    opARCCOSR(varExp1(), varExp2()));

    // float Tan(float val)
    defineFunction(
		    lh, tFloat, "Tan",
		    P() << tFloat, R() << varExp2(),
		    opTANR(varExp1(), varExp2()));

    // float ArcTan(float X, float Y)
    defineFunction(
		    lh, tFloat, "ArcTan",
		    P() << tFloat   << tFloat,
		    R() << varIndex1()<< varIndex2(),
		    opARCTANR(varExp1()));

    // float RadianTan(float val)
    defineFunction(
		    lh, tFloat, "RadianTan",
		    P() << tFloat, R() << varExp2(),
		    O() << opMULTV(varExp2(), radsperdeg)
		        << opTANR(varExp1(), varExp2()));

    // float Max(float first, float second)
    defineFunction(
		    lh, tFloat, "Max",
		    P() << tFloat  << tFloat,
		    R() << varExp1() << varExp2(),
		    opMAXR(varExp1(), varExp2()));
    
    // float Min(float first, float second)
    defineFunction(
		    lh, tFloat, "Min",
		    P() << tFloat  << tFloat,
		    R() << varExp1() << varExp2(),
		    opMINR(varExp1(), varExp2()));

    // float Pow(float first, float second)
    defineFunction(
		    lh, tFloat, "Pow",
		    P() << tFloat  << tFloat,
		    R() << varExp1() << varExp2(),
		    opPOWERR(varExp1(), varExp2()));

    // float InvPow(float first, float second)
    defineFunction(
		    lh, tFloat, "InvPow",
		    P() << tFloat  << tFloat,
		    R() << varExp1() << varExp2(),
		    opIPOWERR(varExp1(), varExp2()));

    // float Factorial(float val)
    defineFunction(
		    lh, tFloat, "Factorial",
		    P() << tFloat, R() << varExp1(),
		    opFACTORIAL(varExp1()));

    // float Abs(float val)
    defineFunction(
		    lh, tFloat, "Abs",
		    P() << tFloat, R() << varExp1(),
		    opABS(varExp1()));

    // float Log10(float val)
    defineFunction(
		    lh, tFloat, "Log10",
		    P() << tFloat, R() << varExp1(),
		    opLOG10(varExp1()));

    // float Ln(float val)
    defineFunction(
		    lh, tFloat, "Ln",
		    P() << tFloat, R() << varExp1(),
		    opLOGE(varExp1()));

    // float Sqrt(float val)
    defineFunction(
		    lh, tFloat, "Sqrt",
		    P() << tFloat, R() << varExp1(),
		    opSQROOTR(varExp1(), varExp1()));
    
    // void CopyTile(float source, float dest)
    defineFunction(
		    lh, tVoid, "CopyTile",
		    P() << tFloat  << tFloat,
		    R() << varExp1() << varExp2(),
		    opCOPYTILERR(varExp1(), varExp2()));

    // void SwapTile(float first, float second)
    defineFunction(
		    lh, tVoid, "SwapTile",
		    P() << tFloat  << tFloat,
		    R() << varExp1() << varExp2(),
		    opSWAPTILERR(varExp1(), varExp2()));
    
    // void OverlayTile(float first, float second)
    defineFunction(
		    lh, tVoid, "OverlayTile",
		    P() << tFloat  << tFloat,
		    R() << varExp1() << varExp2(),
		    opOVERLAYTILERR(varExp1(), varExp2()));
    
    // void ClearTile(float tile)
    defineFunction(
		    lh, tVoid, "ClearTile",
		    P() << tFloat, R() << varExp2(),
		    opCLEARTILER(varExp2()));

    // float GetGlobalRAM(float)
    defineFunction(
		    lh, tFloat, "GetGlobalRAM",
		    P() << tFloat, R() << varIndex1(),
		    opSETR(varExp1(), varGLOBALRAMD()));

    // void SetGlobalRAM(float, float)
    defineFunction(
		    lh, tVoid, "SetGlobalRAM",
		    P() << tFloat << tFloat,
		    R() <<  varIndex1()<<   varExp2(),
		    opSETR(varGLOBALRAMD(), varExp2()));

    // float GetScriptRAM(float)
    defineFunction(
		    lh, tFloat, "GetScriptRAM",
		    P() << tFloat, R() << varIndex1(),
		    opSETR(varExp1(), varSCRIPTRAMD()));

    // void SetScriptRAM(float, float)
    defineFunction(
		    lh, tVoid, "SetScriptRAM",
		    P() << tFloat << tFloat,
		    R() <<  varIndex1() <<   varExp2(),
		    opSETR(varSCRIPTRAMD(), varExp2()));

    // void SetColorBuffer(
    //     float amount, float offset, float stride, float* ptr)
    defineFunction(
		    lh, tVoid, "SetColorBuffer",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    opSETCOLORB());

    // void SetDepthBuffer(
    //     float amount, float offset, float stride, float* ptr)
    defineFunction(
		    lh, tVoid, "SetDepthBuffer",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    opSETDEPTHB());

    // void GetColorBuffer(
    //     float amount, float offset, float stride, float* ptr)
    defineFunction(
		    lh, tVoid, "GetColorBuffer",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    opGETCOLORB());

    // void GetDepthBuffer(
    //     float amount, float offset, float stride, float* ptr)
    defineFunction(
		    lh, tVoid, "GetDepthBuffer",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    opGETDEPTHB());

    // float SizeOfArray(float val)
    defineFunction(
		    lh, tFloat, "SizeOfArray",
		    P() << tFloat, R() << varExp1(),
		    opARRAYSIZE(varExp1()));

    // float SizeOfArrayFFC(ffc* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayFFC",
		    P() << tFfc, R() << varExp1(),
		    opARRAYSIZEF(varExp1()));
    
    // float SizeOfArrayNPC(npc* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayNPC",
		    P() << tNpc, R() << varExp1(),
		    opARRAYSIZEN(varExp1()));
    
    // float SizeOfArrayBool(bool* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayBool",
		    P() << tBool, R() << varExp1(),
		    opARRAYSIZEB(varExp1()));

    // float SizeOfArrayItem(item* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayItem",
		    P() << tItem, R() << varExp1(),
		    opARRAYSIZEI(varExp1()));

    // float SizeOfArrayItemdata(itemdata* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayItemdata",
		    P() << tItemClass, R() << varExp1(),
		    opARRAYSIZEID(varExp1()));

    // float SizeOfArrayLWeapon(lweapon* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayLWeapon",
		    P() << tLWpn, R() << varExp1(),
		    opARRAYSIZEL(varExp1()));

    // int SizeOfArrayEWeapon(eweapon* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayEWeapon",
		    P() << tEWpn, R() << varExp1(),
		    opARRAYSIZEE(varExp1()));
}



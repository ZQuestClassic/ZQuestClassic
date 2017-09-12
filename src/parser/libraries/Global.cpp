#include "../../precompiled.h"
#include "../Library.h"

#include "../ByteCode.h"
#include "../CompilerUtils.h"
#include "../Scope.h"
#include "../LibraryHelper.h"

using namespace ZScript;
using namespace ZScript::Libraries;

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
	typedef VectorBuilder<int> R;
	typedef VectorBuilder<Opcode*> O;

	LibraryHelper lh(scope, NUL);
	
    // int Rand(int maxval)
	defineFunction(
			lh, tFloat, "Rand", P() << tFloat, R() << EXP2,
			new ORandRegister(new VarArgument(EXP1),
			                  new VarArgument(EXP2)));
    
    // void Quit()
    {
	    Function& function = lh.addFunction(tVoid, "Quit", P());
        vector<Opcode*> code;
        code.push_back(new OPopRegister(new VarArgument(EXP2)));
        code.push_back(new OQuit());
        code.front()->setLabel(function.getLabel());
        function.giveCode(code);
    }
    
    // void Waitframe()
    defineFunction(lh, "Waitframe", new OWaitframe());
    
    // void Waitdraw()
    defineFunction(lh, "Waitdraw", new OWaitdraw());

    // void Trace(int val)
    defineFunction(
		    lh, tVoid, "Trace",
		    P() << tFloat, R() << EXP2,
		    new OTraceRegister(new VarArgument(EXP2)));

    // void TraceB(bool val)
    defineFunction(
		    lh, tVoid, "TraceB", P() << tBool, R() << EXP2,
		    new OTrace2Register(new VarArgument(EXP2)));

    // void TraceS(bool val)
    defineFunction(
		    lh, tVoid, "TraceS", P() << tFloat, R() << INDEX,
		    new OTrace6Register(new VarArgument(EXP2)));

    // void TraceNL()
    defineFunction(lh, "TraceNL", new OTrace3());

    // void ClearTrace()
    defineFunction(lh, "ClearTrace", new OTrace4());

    // void TraceToBase(float, float, float)
    defineFunction(
		    lh, tVoid, "TraceToBase",
		    P() << tFloat << tFloat << tFloat,
		    new OTrace5Register());

    // float Sin(float val)
    defineFunction(
		    lh, tFloat, "Sin", P() << tFloat, R() << EXP2,
		    new OSinRegister(new VarArgument(EXP1),
		                     new VarArgument(EXP2)));

    // float RadianSin(float val)
    defineFunction(
		    lh, tFloat, "RadianSin",
		    P() << tFloat, R() << EXP2,
		    O() << new OMultImmediate(new VarArgument(EXP2),
		                              new LiteralArgument(radsperdeg))
		        << new OSinRegister(new VarArgument(EXP1),
		                            new VarArgument(EXP2)));

    // float ArcSin(float val)
    defineFunction(
		    lh, tFloat, "ArcSin",
		    P() << tFloat, R() << EXP2,
		    new OArcSinRegister(new VarArgument(EXP1),
		                        new VarArgument(EXP2)));

    // float Cos(float val)
    defineFunction(
		    lh, tFloat, "Cos",
		    P() << tFloat, R() << EXP2,
		    new OCosRegister(new VarArgument(EXP1),
		                     new VarArgument(EXP2)));

    // float RadianCos(float val)
    defineFunction(
		    lh, tFloat, "RadianCos",
		    P() << tFloat, R() << EXP2,
		    O() << new OMultImmediate(new VarArgument(EXP2),
		                              new LiteralArgument(radsperdeg))
		        << new OCosRegister(new VarArgument(EXP1),
		                            new VarArgument(EXP2)));

    // float ArcCos(float val)
    defineFunction(
		    lh, tFloat, "ArcCos",
		    P() << tFloat, R() << EXP2,
		    new OArcCosRegister(new VarArgument(EXP1),
		                        new VarArgument(EXP2)));

    // float Tan(float val)
    defineFunction(
		    lh, tFloat, "Tan",
		    P() << tFloat, R() << EXP2,
		    new OTanRegister(new VarArgument(EXP1),
		                     new VarArgument(EXP2)));

    // float ArcTan(float X, float Y)
    defineFunction(
		    lh, tFloat, "ArcTan",
		    P() << tFloat << tFloat,
		    R() <<  INDEX << INDEX2,
		    new OATanRegister(new VarArgument(EXP1)));

    // float RadianTan(float val)
    defineFunction(
		    lh, tFloat, "RadianTan",
		    P() << tFloat, R() << EXP2,
		    O() << new OMultImmediate(new VarArgument(EXP2),
		                              new LiteralArgument(radsperdeg))
		        << new OTanRegister(new VarArgument(EXP1),
		                            new VarArgument(EXP2)));

    // float Max(float first, float second)
    defineFunction(
		    lh, tFloat, "Max",
		    P() << tFloat << tFloat,
		    R() <<   EXP1 <<   EXP2,
		    new OMaxRegister(new VarArgument(EXP1),
		                     new VarArgument(EXP2)));
    
    // float Min(float first, float second)
    defineFunction(
		    lh, tFloat, "Min",
		    P() << tFloat << tFloat,
		    R() <<   EXP1 <<   EXP2,
		    new OMinRegister(new VarArgument(EXP1),
		                     new VarArgument(EXP2)));

    // float Pow(float first, float second)
    defineFunction(
		    lh, tFloat, "Pow",
		    P() << tFloat << tFloat,
		    R() <<   EXP1 <<   EXP2,
		    new OPowRegister(new VarArgument(EXP1),
		                     new VarArgument(EXP2)));

    // float InvPow(float first, float second)
    defineFunction(
		    lh, tFloat, "InvPow",
		    P() << tFloat << tFloat,
		    R() <<   EXP1 <<   EXP2,
		    new OInvPowRegister(new VarArgument(EXP1),
		                        new VarArgument(EXP2)));

    // float Factorial(float val)
    defineFunction(
		    lh, tFloat, "Factorial",
		    P() << tFloat, R() << EXP1,
		    new OFactorial(new VarArgument(EXP1)));

    // float Abs(float val)
    defineFunction(
		    lh, tFloat, "Abs",
		    P() << tFloat, R() << EXP1,
		    new OAbsRegister(new VarArgument(EXP1)));

    // float Log10(float val)
    defineFunction(
		    lh, tFloat, "Log10",
		    P() << tFloat, R() << EXP1,
		    new OLog10Register(new VarArgument(EXP1)));

    // float Ln(float val)
    defineFunction(
		    lh, tFloat, "Ln",
		    P() << tFloat, R() << EXP1,
		    new OLogERegister(new VarArgument(EXP1)));

    // float Sqrt(float val)
    defineFunction(
		    lh, tFloat, "Sqrt",
		    P() << tFloat, R() << EXP1,
		    new OSqrtRegister(new VarArgument(EXP1),
		                      new VarArgument(EXP1)));
    
    // void CopyTile(float source, float dest)
    defineFunction(
		    lh, tVoid, "CopyTile",
		    P() << tFloat << tFloat,
		    R() <<   EXP1 <<   EXP2,
		    new OCopyTileRegister(new VarArgument(EXP1),
		                          new VarArgument(EXP2)));

    // void SwapTile(float first, float second)
    defineFunction(
		    lh, tVoid, "SwapTile",
		    P() << tFloat << tFloat,
		    R() <<   EXP1 <<   EXP2,
		    new OSwapTileRegister(new VarArgument(EXP1),
		                          new VarArgument(EXP2)));
    
    // void OverlayTile(float first, float second)
    defineFunction(
		    lh, tVoid, "OverlayTile",
		    P() << tFloat << tFloat,
		    R() <<   EXP1 <<   EXP2,
		    new OOverlayTileRegister(new VarArgument(EXP1),
		                             new VarArgument(EXP2)));
    
    // void ClearTile(float tile)
    defineFunction(
		    lh, tVoid, "ClearTile",
		    P() << tFloat, R() << EXP2,
		    new OClearTileRegister(new VarArgument(EXP2)));

    // float GetGlobalRAM(float)
    defineFunction(
		    lh, tFloat, "GetGlobalRAM",
		    P() << tFloat, R() << INDEX,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(GLOBALRAMD)));

    // void SetGlobalRAM(float, float)
    defineFunction(
		    lh, tVoid, "SetGlobalRAM",
		    P() << tFloat << tFloat,
		    R() <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(GLOBALRAMD),
		                     new VarArgument(EXP2)));

    // float GetScriptRAM(float)
    defineFunction(
		    lh, tFloat, "GetScriptRAM",
		    P() << tFloat, R() << INDEX,
		    new OSetRegister(new VarArgument(EXP1),
		                     new VarArgument(SCRIPTRAMD)));

    // void SetScriptRAM(float, float)
    defineFunction(
		    lh, tVoid, "SetScriptRAM",
		    P() << tFloat << tFloat,
		    R() <<  INDEX <<   EXP2,
		    new OSetRegister(new VarArgument(SCRIPTRAMD),
		                     new VarArgument(EXP2)));

    // void SetColorBuffer(
    //     float amount, float offset, float stride, float* ptr)
    defineFunction(
		    lh, tVoid, "SetColorBuffer",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    new OSetColorBufferRegister());

    // void SetDepthBuffer(
    //     float amount, float offset, float stride, float* ptr)
    defineFunction(
		    lh, tVoid, "SetDepthBuffer",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    new OSetDepthBufferRegister());

    // void GetColorBuffer(
    //     float amount, float offset, float stride, float* ptr)
    defineFunction(
		    lh, tVoid, "GetColorBuffer",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    new OGetColorBufferRegister());

    // void GetDepthBuffer(
    //     float amount, float offset, float stride, float* ptr)
    defineFunction(
		    lh, tVoid, "GetDepthBuffer",
		    P() << tFloat << tFloat << tFloat << tFloat,
		    new OGetDepthBufferRegister());

    // float SizeOfArray(float val)
    defineFunction(
		    lh, tFloat, "SizeOfArray",
		    P() << tFloat, R() << EXP1,
		    new OArraySize(new VarArgument(EXP1)));

    // float SizeOfArrayFFC(ffc* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayFFC",
		    P() << tFfc, R() << EXP1,
		    new OArraySizeF(new VarArgument(EXP1)));
    
    // float SizeOfArrayNPC(npc* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayNPC",
		    P() << tNpc, R() << EXP1,
		    new OArraySizeN(new VarArgument(EXP1)));
    
    // float SizeOfArrayBool(bool* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayBool",
		    P() << tBool, R() << EXP1,
		    new OArraySizeB(new VarArgument(EXP1)));

    // float SizeOfArrayItem(item* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayItem",
		    P() << tItem, R() << EXP1,
		    new OArraySizeI(new VarArgument(EXP1)));

    // float SizeOfArrayItemdata(itemdata* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayItemdata",
		    P() << tItemClass, R() << EXP1,
		    new OArraySizeID(new VarArgument(EXP1)));

    // float SizeOfArrayLWeapon(lweapon* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayLWeapon",
		    P() << tLWpn, R() << EXP1,
		    new OArraySizeL(new VarArgument(EXP1)));

    // int SizeOfArrayEWeapon(eweapon* ptr)
    defineFunction(
		    lh, tFloat, "SizeOfArrayEWeapon",
		    P() << tEWpn, R() << EXP1,
		    new OArraySizeE(new VarArgument(EXP1)));
}



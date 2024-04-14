#include "ByteCode.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "zasm/serialize.h"
#include "zsyssimple.h"
#include <assert.h>
#include <iostream>
#include <cstdlib>

using namespace ZScript;
using std::ostringstream;
using std::string;

string la_toString(int32_t value)
{
    char temp[128];
    string sign = value < 0 ? "-" : "";
    sprintf(temp,"%d", abs(value/10000));
    string first = string(temp);
    
    if(value % 10000 == 0)
        return sign + first;
        
    sprintf(temp,"%d", abs(value%10000));
    string second = string(temp);
    
    while(second.length() < 4)
        second = "0" + second;
        
    return sign + first + "." + second;
}

string LiteralArgument::toString() const
{
	return la_toString(value);
}

string CompareArgument::toString() const
{
	return CMP_STR(value);
}

string ZScript::VarToString(int32_t ID)
{
	return zasm_var_to_string(ID);
}

string VarArgument::toString() const
{
	return VarToString(ID);
}

string GlobalArgument::toString() const
{
    char temp[40];
    sprintf(temp, "GD%d", ID);
    return string(temp);
}

string LabelArgument::toString() const
{
	char buf[40];
	if(!haslineno)
		sprintf(buf, "l%d", ID);
	else if(altstr)
		sprintf(buf, "%d.%04d", lineno / 10000, abs(lineno % 10000));
	else
		sprintf(buf, "%d", lineno);
	return string(buf);
}

string StringArgument::toString() const
{
	return util::escape_string(value);
}

string VectorArgument::toString() const
{
	return util::stringify_vector(value, true);
}

string OSetTrue::toString() const
{
    return "SETTRUE " + getArgument()->toString();
}

string OSetTrueI::toString() const
{
    return "SETTRUEI " + getArgument()->toString();
}

string OSetFalse::toString() const
{
    return "SETFALSE " + getArgument()->toString();
}

string OSetFalseI::toString() const
{
    return "SETFALSEI " + getArgument()->toString();
}

string OSetMore::toString() const
{
    return "SETMORE " + getArgument()->toString();
}

string OSetMoreI::toString() const
{
    return "SETMOREI " + getArgument()->toString();
}

string OSetLess::toString() const
{
    return "SETLESS " + getArgument()->toString();
}

string OSetLessI::toString() const
{
    return "SETLESSI " + getArgument()->toString();
}

string OSetImmediate::toString() const
{
	return "SETV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSetRegister::toString() const
{
    return "SETR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSetObject::toString() const
{
    return "SET_OBJECT " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OReadPODArrayR::toString() const
{
	return "READPODARRAYR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OReadPODArrayI::toString() const
{
	return "READPODARRAYV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWritePODArrayRR::toString() const
{
	return "WRITEPODARRAYRR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWritePODArrayRI::toString() const
{
	return "WRITEPODARRAYRV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWritePODArrayIR::toString() const
{
	return "WRITEPODARRAYVR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWritePODArrayII::toString() const
{
	return "WRITEPODARRAYVV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OWritePODString::toString() const
{
	return "WRITEPODSTRING " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OWritePODArray::toString() const
{
	return "WRITEPODARRAY " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OConstructClass::toString() const
{
	return "ZCLASS_CONSTRUCT " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OMarkTypeClass::toString() const
{
	return "ZCLASS_MARK_TYPE " + getArgument()->toString();
}
string OReadObject::toString() const
{
	return "ZCLASS_READ " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OWriteObject::toString() const
{
	return "ZCLASS_WRITE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OFreeObject::toString() const
{
	return "ZCLASS_FREE " + getArgument()->toString();
}
string OOwnObject::toString() const
{
	return "ZCLASS_OWN " + getArgument()->toString();
}
string ODestructor::toString() const
{
	return "STARTDESTRUCTOR " + getArgument()->toString();
}
string OGlobalObject::toString() const
{
	return "ZCLASS_GLOBALIZE " + getArgument()->toString();
}
string OObjOwnBitmap::toString() const
{
	return "OBJ_OWN_BITMAP " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnPaldata::toString() const
{
	return "OBJ_OWN_PALDATA " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnFile::toString() const
{
	return "OBJ_OWN_FILE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnDir::toString() const
{
	return "OBJ_OWN_DIR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnStack::toString() const
{
	return "OBJ_OWN_STACK " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnRNG::toString() const
{
	return "OBJ_OWN_RNG " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnClass::toString() const
{
	return "OBJ_OWN_CLASS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OObjOwnArray::toString() const
{
	return "OBJ_OWN_ARRAY " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OQuitNoDealloc::toString() const
{
	return "QUIT_NO_DEALLOC";
}
string OSetCustomCursor::toString() const
{
	return "GAMESETCUSTOMCURSOR";
}
string OItemGetDispName::toString() const
{
	return "ITEMGETDISPLAYNAME " + getArgument()->toString();
}
string OItemSetDispName::toString() const
{
	return "ITEMSETDISPLAYNAME " + getArgument()->toString();
}
string OItemGetShownName::toString() const
{
	return "ITEMGETSHOWNNAME " + getArgument()->toString();
}
string OHeroMoveXY::toString() const
{
	return "HEROMOVEXY";
}
string OHeroCanMoveXY::toString() const
{
	return "HEROCANMOVEXY";
}
string OHeroLiftRelease::toString() const
{
	return "HEROLIFTRELEASE";
}
string OHeroLiftGrab::toString() const
{
	return "HEROLIFTGRAB";
}
string OHeroIsFlickerFrame::toString() const
{
	return "HEROISFLICKERFRAME";
}
string OLoadPortalRegister::toString() const
{
	return "LOADPORTAL " + getArgument()->toString();
}
string OCreatePortal::toString() const
{
	return "CREATEPORTAL";
}
string OLoadSavPortalRegister::toString() const
{
	return "LOADSAVPORTAL " + getArgument()->toString();
}
string OCreateSavPortal::toString() const
{
	return "CREATESAVPORTAL";
}
string OPortalRemove::toString() const
{
	return "PORTALREMOVE";
}
string OSavedPortalRemove::toString() const
{
	return "SAVEDPORTALREMOVE";
}
string OSavedPortalGenerate::toString() const
{
	return "SAVEDPORTALGENERATE";
}
string OUseSpritePortal::toString() const
{
	return "PORTALUSESPRITE " + getArgument()->toString();
}
string OHeroMoveAtAngle::toString() const
{
	return "HEROMOVEATANGLE";
}
string OHeroCanMoveAtAngle::toString() const
{
	return "HEROCANMOVEATANGLE";
}
string OHeroMove::toString() const
{
	return "HEROMOVE";
}
string OHeroCanMove::toString() const
{
	return "HEROCANMOVE";
}
string ODrawLightCircle::toString() const
{
	return "DRAWLIGHT_CIRCLE";
}
string ODrawLightSquare::toString() const
{
	return "DRAWLIGHT_SQUARE";
}
string ODrawLightCone::toString() const
{
	return "DRAWLIGHT_CONE";
}
string OPeek::toString() const
{
	return "PEEK " + getArgument()->toString();
}
string OPeekAtImmediate::toString() const
{
	return "PEEKATV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();;
}
string OMakeVargArray::toString() const
{
	return "MAKEVARGARRAY";
}
string OPrintfArr::toString() const
{
	return "PRINTFA";
}
string OSPrintfArr::toString() const
{
	return "SPRINTFA";
}
string OCurrentItemID::toString() const
{
	return "CURRENTITEMID";
}
string OArrayPush::toString() const
{
	return "ARRAYPUSH";
}
string OArrayPop::toString() const
{
	return "ARRAYPOP";
}
string OLoadSubscreenDataRV::toString() const
{
	return "LOADSUBDATARV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OSwapSubscrV::toString() const
{
	return "SWAPSUBSCREENV " + getArgument()->toString();
}
string OGetSubscreenName::toString() const
{
	return "SUBDATA_GET_NAME " + getArgument()->toString();
}
string OSetSubscreenName::toString() const
{
	return "SUBDATA_SET_NAME " + getArgument()->toString();
}


string OAddImmediate::toString() const
{
    return "ADDV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAddRegister::toString() const
{
    return "ADDR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSubImmediate::toString() const
{
    return "SUBV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSubImmediate2::toString() const
{
    return "SUBV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSubRegister::toString() const
{
    return "SUBR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMultImmediate::toString() const
{
    return "MULTV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OMultRegister::toString() const
{
    return "MULTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODivImmediate::toString() const
{
    return "DIVV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODivImmediate2::toString() const
{
    return "DIVV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODivRegister::toString() const
{
    return "DIVR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCompareImmediate::toString() const
{
    return "COMPAREV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCompareImmediate2::toString() const
{
    return "COMPAREV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCompareRegister::toString() const
{
    return "COMPARER " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OInternalStringCompare::toString() const
{
	return "STRCMPR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OInternalInsensitiveStringCompare::toString() const
{
	return "STRICMPR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWaitframe::toString() const
{
    return "WAITFRAME";
}

string OWaitframes::toString() const
{
    return "WAITFRAMESR " + getArgument()->toString();
}

string OWaitdraw::toString() const
{
    return "WAITDRAW";
}

string OWaitTo::toString() const
{
    return "WAITTO " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWaitEvent::toString() const
{
    return "WAITEVENT";
}

string ONoOp::toString() const
{
	return "NOP";
}

string OCastBoolI::toString() const
{
	return "CASTBOOLI " + getArgument()->toString();
}

string OCastBoolF::toString() const
{
	return "CASTBOOLF " + getArgument()->toString();
}

//I would like to add a Jump instruction tot he parser, which would be 'GOTOLABEL' -Z
string OGotoImmediate::toString() const
{
    return "GOTO " + getArgument()->toString();
}

string OGotoTrueImmediate::toString() const
{
    return "GOTOTRUE " + getArgument()->toString();
}

string OGotoFalseImmediate::toString() const
{
    return "GOTOFALSE " + getArgument()->toString();
}

string OGotoMoreImmediate::toString() const
{
    return "GOTOMORE " + getArgument()->toString();
}

string OGotoLessImmediate::toString() const
{
    return "GOTOLESS " + getArgument()->toString();
}

string OPushRegister::toString() const
{
    return "PUSHR " + getArgument()->toString();
}

string OPushImmediate::toString() const
{
	return "PUSHV " + getArgument()->toString();
}

string OPopRegister::toString() const
{
    return "POP " + getArgument()->toString();
}

string OPopArgsRegister::toString() const
{
    return "POPARGS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPushArgsRegister::toString() const
{
    return "PUSHARGSR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPushArgsImmediate::toString() const
{
    return "PUSHARGSV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPushVargV::toString() const
{
    return "PUSHVARGV " + getArgument()->toString();
}

string OPushVargR::toString() const
{
    return "PUSHVARGR " + getArgument()->toString();
}

string OPushVargsV::toString() const
{
    return "PUSHVARGSV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPushVargsR::toString() const
{
    return "PUSHVARGSR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OLoadIndirect::toString() const
{
    return "LOADI " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OStoreIndirect::toString() const
{
    return "STOREI " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OLoadDirect::toString() const
{
    return "LOADD " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OStoreDirect::toString() const
{
    return "STORED " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OStoreDirectV::toString() const
{
    return "STOREDV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OLoad::toString() const
{
    return "LOAD " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OStore::toString() const
{
    return "STORE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OStoreV::toString() const
{
    return "STOREV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OStoreObject::toString() const
{
    return "STORE_OBJECT " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OQuit::toString() const
{
    return "QUIT";
}

string OGotoRegister::toString() const
{
    return "GOTOR " + getArgument()->toString();
}

string OAndImmediate::toString() const
{
    return "ANDV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAndRegister::toString() const
{
    return "ANDR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OOrImmediate::toString() const
{
    return "ORV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OOrRegister::toString() const
{
    return "ORR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OXorImmediate::toString() const
{
    return "XORV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OXorRegister::toString() const
{
    return "XORR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitAndImmediate::toString() const
{
    return "ANDV32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitAndRegister::toString() const
{
    return "ANDR32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitOrImmediate::toString() const
{
    return "ORV32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitOrRegister::toString() const
{
    return "ORR32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitXorImmediate::toString() const
{
    return "XORV32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitXorRegister::toString() const
{
    return "XORR32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSinRegister::toString() const
{
    return "SINR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCosRegister::toString() const
{
    return "COSR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OTanRegister::toString() const
{
    return "TANR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OEngineDegtoRad::toString() const
{
    return "DEGTORAD " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OEngineRadtoDeg::toString() const
{
    return "RADTODEG " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Ostrlen::toString() const
{
    return "STRINGLENGTH " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OATanRegister::toString() const
{
    return "ARCTANR " + getArgument()->toString();
}

string OArcCosRegister::toString() const
{
    return "ARCCOSR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OArcSinRegister::toString() const
{
    return "ARCSINR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMinRegister::toString() const
{
    return "MINR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMaxRegister::toString() const
{
    return "MAXR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OMaxNew::toString() const
{
    return "MAXVARG";
}
string OMinNew::toString() const
{
    return "MINVARG";
}
string OChoose::toString() const
{
    return "CHOOSEVARG";
}

string OPowRegister::toString() const
{
    return "POWERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OPowImmediate::toString() const
{
    return "POWERV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OPowImmediate2::toString() const
{
    return "POWERV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OLPowRegister::toString() const
{
    return "LPOWERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OLPowImmediate::toString() const
{
    return "LPOWERV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OLPowImmediate2::toString() const
{
    return "LPOWERV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OInvPowRegister::toString() const
{
    return "IPOWERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OFactorial::toString() const
{
    return "FACTORIAL " + getArgument()->toString();
}

string OAbsRegister::toString() const
{
    return "ABS " + getArgument()->toString();
}

string OLog10Register::toString() const
{
    return "LOG10 " + getArgument()->toString();
}

string OLogERegister::toString() const
{
    return "LOGE " + getArgument()->toString();
}

string OArraySize::toString() const
{
    return "ARRAYSIZE " + getArgument()->toString();
}


string OArraySizeF::toString() const
{
    return "ARRAYSIZEF " + getArgument()->toString();
}
string OArraySizeN::toString() const
{
    return "ARRAYSIZEN " + getArgument()->toString();
}
string OArraySizeE::toString() const
{
    return "ARRAYSIZEE " + getArgument()->toString();
}
string OArraySizeL::toString() const
{
    return "ARRAYSIZEL " + getArgument()->toString();
}
string OArraySizeB::toString() const
{
    return "ARRAYSIZEB " + getArgument()->toString();
}
string OArraySizeI::toString() const
{
    return "ARRAYSIZEI " + getArgument()->toString();
}
string OArraySizeID::toString() const
{
    return "ARRAYSIZEID " + getArgument()->toString();
}

string OLShiftImmediate::toString() const
{
    return "LSHIFTV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OLShiftRegister::toString() const
{
    return "LSHIFTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ORShiftImmediate::toString() const
{
    return "RSHIFTV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ORShiftRegister::toString() const
{
    return "RSHIFTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitLShiftImmediate::toString() const
{
    return "LSHIFTV32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitLShiftRegister::toString() const
{
    return "LSHIFTR32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitRShiftImmediate::toString() const
{
    return "RSHIFTV32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string O32BitRShiftRegister::toString() const
{
    return "RSHIFTR32 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OModuloImmediate::toString() const
{
    return "MODV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OModuloImmediate2::toString() const
{
    return "MODV2 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OModuloRegister::toString() const
{
    return "MODR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ONot::toString() const
{
    return "BITNOT " + getArgument()->toString();
}

string O32BitNot::toString() const
{
    return "BITNOT32 " + getArgument()->toString();
}

string OTraceRegister::toString() const
{
    return "TRACER " + getArgument()->toString();
}
string OTraceImmediate::toString() const
{
    return "TRACEV " + getArgument()->toString();
}

string OTraceLRegister::toString() const
{
    return "TRACELR " + getArgument()->toString();
}

string OTrace2Register::toString() const
{
    return "TRACE2R " + getArgument()->toString();
}

string OTrace3::toString() const
{
    return "TRACE3";
}

string OTrace4::toString() const
{
    return "TRACE4";
}

string OTrace5Register::toString() const
{
    return "TRACE5";
}

string OTrace6Register::toString() const
{
    return "TRACE6 " + getArgument()->toString();
}

string OPrintfImmediate::toString() const
{
	return "PRINTFV " + getArgument()->toString();
}

string OSPrintfImmediate::toString() const
{
	return "SPRINTFV " + getArgument()->toString();
}

string OPrintfVargs::toString() const
{
	return "PRINTFVARG";
}

string OSPrintfVargs::toString() const
{
	return "SPRINTFVARG";
}

string OBreakpoint::toString() const
{
    return "BREAKPOINT " + getArgument()->toString();
}


string ORandRegister::toString() const
{
    return "RNDR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSRandRegister::toString() const
{
    return "SRNDR " + getArgument()->toString();
}

string OSRandImmediate::toString() const
{
    return "SRNDV " + getArgument()->toString();
}

string OSRandRand::toString() const
{
    return "SRNDRND " + getArgument()->toString();
}

string ORNGRand1::toString() const
{
    return "RNGRAND1";
}

string ORNGRand2::toString() const
{
    return "RNGRAND2 " + getArgument()->toString();
}

string ORNGRand3::toString() const
{
    return "RNGRAND3 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ORNGLRand1::toString() const
{
    return "RNGLRAND1";
}

string ORNGLRand2::toString() const
{
    return "RNGLRAND2 " + getArgument()->toString();
}

string ORNGLRand3::toString() const
{
    return "RNGLRAND3 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ORNGSeed::toString() const
{
    return "RNGSEED " + getArgument()->toString();
}

string ORNGRSeed::toString() const
{
    return "RNGRSEED";
}

string ORNGFree::toString() const
{
    return "RNGFREE";
}

string OCheckTrig::toString() const
{
    return "CHECKTRIG";
}

string OWarp::toString() const
{
    return "WARPR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPitWarp::toString() const
{
    return "PITWARPR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSqrtRegister::toString() const
{
    return "SQROOTR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCreateItemRegister::toString() const
{
    return "CREATEITEMR " + getArgument()->toString();
}

string OCreateNPCRegister::toString() const
{
    return "CREATENPCR " + getArgument()->toString();
}

string OCreateLWpnRegister::toString() const
{
    return "CREATELWEAPONR " + getArgument()->toString();
}

string OCreateEWpnRegister::toString() const
{
    return "CREATEEWEAPONR " + getArgument()->toString();
}

string OLoadItemRegister::toString() const
{
    return "LOADITEMR " + getArgument()->toString();
}

string OLoadItemDataRegister::toString() const
{
    return "LOADITEMDATAR " + getArgument()->toString();
}

//New Types

string OLoadShopDataRegister::toString() const
{
    return "LOADSHOPR " + getArgument()->toString();
}
string OLoadInfoShopDataRegister::toString() const
{
    return "LOADINFOSHOPR " + getArgument()->toString();
}
string OLoadMessageDataRegister::toString() const
{
    return "LOADMESSAGEDATAR " + getArgument()->toString();
}

string OLoadDMapDataRegister::toString() const
{
    return "LOADDMAPDATAR " + getArgument()->toString();
}

string OLoadStack::toString() const
{
    return "LOADSTACK";
}

string OLoadDirectoryRegister::toString() const
{
    return "LOADDIRECTORYR " + getArgument()->toString();
}

string OLoadRNG::toString() const
{
    return "LOADRNG";
}

string OCreatePalData::toString() const
{
    return "CREATEPALDATA";
}

string OCreatePalDataClr::toString() const
{
	return "CREATEPALDATACLR " + getArgument()->toString();
}

string OMixColorArray::toString() const
{
	return "MIXCLR";
}
string OCreateRGBHex::toString() const
{
	return "CREATERGBHEX " + getArgument()->toString();
}
string OCreateRGB::toString() const
{
	return "CREATERGB";
}
string OConvertFromRGB::toString() const
{
	return "CONVERTFROMRGB";
}
string OConvertToRGB::toString() const
{
	return "CONVERTTORGB";
}
string OGetTilePixel::toString() const
{
	return "GETTILEPIXEL";
}
string OSetTilePixel::toString() const
{
	return "SETTILEPIXEL";
}
string OLoadLevelPalette::toString() const
{
    return "PALDATALOADLEVEL " + getArgument()->toString();
}

string OLoadSpritePalette::toString() const
{
    return "PALDATALOADSPRITE " + getArgument()->toString();
}

string OLoadMainPalette::toString() const
{
    return "PALDATALOADMAIN";
}

string OLoadCyclePalette::toString() const
{
	return "PALDATALOADCYCLE " + getArgument()->toString();
}

string OLoadBitmapPalette::toString() const
{
	return "PALDATALOADBITMAP " + getArgument()->toString();
}

string OWriteLevelPalette::toString() const
{
    return "PALDATAWRITELEVEL " + getArgument()->toString();
}

string OWriteLevelCSet::toString() const
{
    return "PALDATAWRITELEVELCS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWriteSpritePalette::toString() const
{
    return "PALDATAWRITESPRITE " + getArgument()->toString();
}

string OWriteSpriteCSet::toString() const
{
    return "PALDATAWRITESPRITECS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OWriteMainPalette::toString() const
{
    return "PALDATAWRITEMAIN";
}

string OWriteMainCSet::toString() const
{
    return "PALDATAWRITEMAINCS " + getArgument()->toString();
}

string OWriteCyclePalette::toString() const
{
	return "PALDATAWRITECYCLE " + getArgument()->toString();
}

string OWriteCycleCSet::toString() const
{
	return "PALDATAWRITECYCLECS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPalDataColorValid::toString() const
{
	return "PALDATAVALIDCLR " + getArgument()->toString();
}

string OPalDataClearColor::toString() const
{
    return "PALDATACLEARCLR " + getArgument()->toString();
}

string OPalDataClearCSet::toString() const
{
	return "PALDATACLEARCSET " + getArgument()->toString();
}

string OPalDataMix::toString() const
{
    return "PALDATAMIX";
}

string OPalDataMixCSet::toString() const
{
    return "PALDATAMIXCS";
}

string OPalDataCopy::toString() const
{
	return "PALDATACOPY " + getArgument()->toString();
}

string OPalDataCopyCSet::toString() const
{
	return "PALDATACOPYCSET";
}

string OPalDataFree::toString() const
{
	return "PALDATAFREE";
}

string OPalDataOwn::toString() const
{
	return "PALDATAOWN";
}

string OLoadDropsetRegister	::toString() const
{
    return "LOADDROPSETR " + getArgument()->toString();
}

string OGetBottleShopName::toString() const
{
    return "BSHOPNAMEGET " + getArgument()->toString();
}

string OSetBottleShopName::toString() const
{
    return "BSHOPNAMESET " + getArgument()->toString();
}

string OGetBottleName::toString() const
{
    return "BOTTLENAMEGET " + getArgument()->toString();
}

string OSetBottleName::toString() const
{
    return "BOTTLENAMESET " + getArgument()->toString();
}

string OLoadBottleTypeRegister::toString() const
{
    return "LOADBOTTLETYPE " + getArgument()->toString();
}

string OLoadBShopRegister::toString() const
{
    return "LOADBSHOPDATA " + getArgument()->toString();
}
string OLoadGenericDataR::toString() const
{
    return "LOADGENERICDATA " + getArgument()->toString();
}

string ODMapDataGetNameRegister::toString() const
{
    return "DMAPDATAGETNAMER " + getArgument()->toString();
}

string ODMapDataSetNameRegister::toString() const
{
    return "DMAPDATASETNAMER " + getArgument()->toString();
}

string ODMapDataGetTitleRegister::toString() const
{
    return "DMAPDATAGETTITLER " + getArgument()->toString();
}

string ODMapDataSetTitleRegister::toString() const
{
    return "DMAPDATASETTITLER " + getArgument()->toString();
}

string ODMapDataGetIntroRegister::toString() const
{
    return "DMAPDATAGETINTROR " + getArgument()->toString();
}

string ODMapDataSetIntroRegister::toString() const
{
    return "DMAPDATANSETITROR " + getArgument()->toString();
}

string ODMapDataGetMusicRegister::toString() const
{
    return "DMAPDATAGETMUSICR " + getArgument()->toString();
}

string ODMapDataSetMusicRegister::toString() const
{
    return "DMAPDATASETMUSICR " + getArgument()->toString();
}

string OMessageDataSetStringRegister::toString() const
{
    return "MESSAGEDATASETSTRINGR " + getArgument()->toString();
}
string OMessageDataGetStringRegister::toString() const
{
    return "MESSAGEDATAGETSTRINGR " + getArgument()->toString();
}
string OLoadNPCDataRegister::toString() const
{
    return "LOADNPCDATAR " + getArgument()->toString();
}
string OLoadComboDataRegister::toString() const
{
    return "LOADCOMBODATAR " + getArgument()->toString();
}
string OLoadMapDataRegister::toString() const
{
    return "LOADMAPDATAR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OLoadSpriteDataRegister::toString() const
{
    return "LOADSPRITEDATAR " + getArgument()->toString();
}
string OLoadScreenDataRegister::toString() const
{
    return "LOADSCREENDATAR " + getArgument()->toString();
}
string OLoadBitmapDataRegister::toString() const
{
    return "LOADBITMAPDATAR " + getArgument()->toString();
}

string OLoadNPCRegister::toString() const
{
    return "LOADNPCR " + getArgument()->toString();
}


string OLoadLWpnRegister::toString() const
{
    return "LOADLWEAPONR " + getArgument()->toString();
}

string OLoadEWpnRegister::toString() const
{
    return "LOADEWEAPONR " + getArgument()->toString();
}
string OAdjustVolumeRegister::toString() const
{
    return "ADJUSTVOLUMER " + getArgument()->toString();
}
string OAdjustSFXVolumeRegister::toString() const
{
    return "ADJUSTSFXVOLUMER " + getArgument()->toString();
}
string OAdjustSound::toString() const
{
	return "ADJUSTSFX";
}
string OPlaySoundEX::toString() const
{
	return "PLAYSOUNDEX";
}
string OGetSoundCompletion::toString() const
{
	return "GETSFXCOMPLETION " + getArgument()->toString();
}
string OPlaySoundRegister::toString() const
{
    return "PLAYSOUNDR " + getArgument()->toString();
}

string OPlayMIDIRegister::toString() const
{
    return "PLAYMIDIR " + getArgument()->toString();
}

string OPlayEnhancedMusic::toString() const
{
    return "PLAYENHMUSIC " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OPlayEnhancedMusicEx::toString() const
{
    return "PLAYENHMUSICEX " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetEnhancedMusicPos::toString() const
{
    return "GETENHMUSICPOS " + getArgument()->toString();
}

string OSetEnhancedMusicPos::toString() const
{
    return "SETENHMUSICPOS " + getArgument()->toString();
}

string OSetEnhancedMusicSpeed::toString() const
{
    return "SETENHMUSICSPEED " + getArgument()->toString();
}

string OGetEnhancedMusicLength::toString() const
{
	return "GETENHMUSICLEN " + getArgument()->toString();
}

string OSetEnhancedMusicLoop::toString() const
{
	return "SETENHMUSICLOOP " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCrossfadeEnhancedMusic::toString() const
{
	return "ENHCROSSFADE";
}

string OGetDMapMusicFilename::toString() const
{
    return "GETMUSICFILE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetNPCDataInitDLabel::toString() const
{
    return "NPCGETINITDLABEL " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetDMapMusicTrack::toString() const
{
    return "GETMUSICTRACK " + getArgument()->toString();
}

// Audio->
string OEndSoundRegister::toString() const
{
    return "ENDSOUNDR " + getArgument()->toString();
}


string OContinueSFX::toString() const
{
    return "CONTINUESFX " + getArgument()->toString();
}

string OPauseSoundRegister::toString() const
{
    return "PAUSESOUNDR " + getArgument()->toString();
}

string OPauseMusic::toString() const
{
    return "PAUSEMUSIC";
}
string OResumeMusic::toString() const
{
    return "RESUMEMUSIC";
}

string OResumeSoundRegister::toString() const
{
    return "RESUMESOUNDR " + getArgument()->toString();
}

//END Audio

string OSetDMapEnhancedMusic::toString() const
{
    return "SETDMAPENHMUSIC";
}

string OGetSaveName::toString() const
{
    return "GETSAVENAME " + getArgument()->toString();
}

string OSetSaveName::toString() const
{
    return "SETSAVENAME " + getArgument()->toString();
}

string OGetDMapName::toString() const
{
    return "GETDMAPNAME " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetDMapTitle::toString() const
{
    return "GETDMAPTITLE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetDMapIntro::toString() const
{
    return "GETDMAPINTRO " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OSetDMapName::toString() const
{
    return "SETDMAPNAME " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSetDMapTitle::toString() const
{
    return "SETDMAPTITLE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSetDMapIntro::toString() const
{
    return "SETDMAPINTRO " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetItemName::toString() const
{
    return "ITEMNAME " + getArgument()->toString();
}

string OGetNPCName::toString() const
{
    return "NPCNAME " + getArgument()->toString();
}

string OGetMessage::toString() const
{
    return "GETMESSAGE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OSetMessage::toString() const
{
    return "SETMESSAGE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OClearSpritesRegister::toString() const
{
    return "CLEARSPRITESR " + getArgument()->toString();
}

string ORectangleRegister::toString() const
{
    return "RECT";
}

string OFrameRegister::toString() const
{
    return "FRAMER";
}

string OCircleRegister::toString() const
{
    return "CIRCLE";
}

string OArcRegister::toString() const
{
    return "ARC";
}

string OEllipseRegister::toString() const
{
    return "ELLIPSE";
}

string OLineRegister::toString() const
{
    return "LINE";
}

string OSplineRegister::toString() const
{
    return "SPLINE";
}

string OPutPixelRegister::toString() const
{
    return "PUTPIXEL";
}

string OPutPixelArrayRegister::toString() const
{
    return "PIXELARRAYR";
}

string OPutTileArrayRegister::toString() const
{
    return "TILEARRAYR";
}

string OPutLinesArrayRegister::toString() const
{
    return "LINESARRAY";
}

string ODrawCharRegister::toString() const
{
    return "DRAWCHAR";
}

string ODrawIntRegister::toString() const
{
    return "DRAWINT";
}

string ODrawTileRegister::toString() const
{
    return "DRAWTILE";
}

string ODrawTileCloakedRegister::toString() const
{
    return "DRAWTILECLOAKEDR";
}

string ODrawComboRegister::toString() const
{
    return "DRAWCOMBO";
}

string ODrawComboCloakedRegister::toString() const
{
    return "DRAWCOMBOCLOAKEDR";
}

string OFastTileRegister::toString() const
{
    return "FASTTILE";
}

string OFastComboRegister::toString() const
{
    return "FASTCOMBO";
}

string OFastComboArrayRegister::toString() const
{
    return "COMBOARRAYR";
}

string ODrawStringRegister::toString() const
{
    return "DRAWSTRING";
}

string ODrawString2Register::toString() const
{
    return "DRAWSTRINGR2";
}

string ODrawBitmapRegister::toString() const
{
    return "DRAWBITMAP";
}

string ODrawBitmapExRegister::toString() const
{
    return "BITMAPEXR";
}

string OSetRenderTargetRegister::toString() const
{
    return "SETRENDERTARGET";
}

string OSetDepthBufferRegister::toString() const
{
    return "SETDEPTHB";
}

string OGetDepthBufferRegister::toString() const
{
    return "GETDEPTHB";
}

string OSetColorBufferRegister::toString() const
{
    return "SETCOLORB";
}

string OGetColorBufferRegister::toString() const
{
    return "GETCOLORB";
}

string OQuadRegister::toString() const
{
    return "QUAD";
}

string OTriangleRegister::toString() const
{
    return "TRIANGLE";
}


string OQuad3DRegister::toString() const
{
    return "QUAD3D";
}

string OTriangle3DRegister::toString() const
{
    return "TRIANGLE3D";
}

string ODrawLayerRegister::toString() const
{
    return "DRAWLAYER";
}

string ODrawScreenRegister::toString() const
{
    return "DRAWSCREEN";
}

string OMessageRegister::toString() const
{
    return "MSGSTRR " + getArgument()->toString();
}

string OIsSolid::toString() const
{
    return "ISSOLID " + getArgument()->toString();
}

string OIsSolidMapdata::toString() const
{
    return "MAPDATAISSOLID " + getArgument()->toString();
}

string OIsSolidMapdataLayer::toString() const
{
    return "MAPDATAISSOLIDLYR " + getArgument()->toString();
}

string OIsSolidLayer::toString() const
{
    return "ISSOLIDLAYER " + getArgument()->toString();
}

string OLoadTmpScr::toString() const
{
    return "LOADTMPSCR " + getArgument()->toString();
}

string OLoadScrollScr::toString() const
{
    return "LOADSCROLLSCR " + getArgument()->toString();
}

string OSetSideWarpRegister::toString() const
{
    return "SETSIDEWARP";
}

string OGetSideWarpDMap::toString() const
{
    return "GETSIDEWARPDMAP " + getArgument()->toString();
}

string OGetSideWarpScreen::toString() const
{
    return "GETSIDEWARPSCR " + getArgument()->toString();
}

string OGetSideWarpType::toString() const
{
    return "GETSIDEWARPTYPE " + getArgument()->toString();
}

string OGetTileWarpDMap::toString() const
{
    return "GETTILEWARPDMAP " + getArgument()->toString();
}

string OGetTileWarpScreen::toString() const
{
    return "GETTILEWARPSCR " + getArgument()->toString();
}

string OGetTileWarpType::toString() const
{
    return "GETTILEWARPTYPE " + getArgument()->toString();
}

string OSetTileWarpRegister::toString() const
{
    return "SETTILEWARP";
}

string OLayerScreenRegister::toString() const
{
    return "LAYERSCREEN " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OLayerMapRegister::toString() const
{
    return "LAYERMAP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OTriggerSecrets::toString() const
{
    return "SECRETS";
}

string OTriggerSecretsFor::toString() const
{
    return "REGION_TRIGGER_SECRETS " + getArgument()->toString();
}

string OIsValidArray::toString() const
{
    return "ISVALIDARRAY " + getArgument()->toString();
}

string OIsValidItem::toString() const
{
    return "ISVALIDITEM " + getArgument()->toString();
}

string OIsValidNPC::toString() const
{
    return "ISVALIDNPC " + getArgument()->toString();
}

string OCopyTileRegister::toString() const
{
    return "COPYTILERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Ostrcpy::toString() const
{
    return "STRINGCOPY " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OOverlayTileRegister::toString() const
{
    return "OVERLAYTILERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSwapTileRegister::toString() const
{
    return "SWAPTILERR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OClearTileRegister::toString() const
{
    return "CLEARTILER " + getArgument()->toString();
}

string OIsValidLWpn::toString() const
{
    return "ISVALIDLWPN " + getArgument()->toString();
}

string OIsValidEWpn::toString() const
{
    return "ISVALIDEWPN " + getArgument()->toString();
}

string OMakeAngularLwpn::toString() const
{
    return "LWPNMAKEANGULAR " + getArgument()->toString();
}

string OMakeAngularEwpn::toString() const
{
    return "EWPNMAKEANGULAR " + getArgument()->toString();
}

string OMakeDirectionalLwpn::toString() const
{
    return "LWPNMAKEDIRECTIONAL " + getArgument()->toString();
}

string OMakeDirectionalEwpn::toString() const
{
    return "EWPNMAKEDIRECTIONAL " + getArgument()->toString();
}

string OUseSpriteLWpn::toString() const
{
    return "LWPNUSESPRITER " + getArgument()->toString();
}

string OUseSpriteEWpn::toString() const
{
    return "EWPNUSESPRITER " + getArgument()->toString();
}

string OAllocateMemRegister::toString() const
{
    return "ALLOCATEMEMR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OAllocateMemImmediate::toString() const
{
    return "ALLOCATEMEMV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString() + "," + getThirdArgument()->toString();
}

string OAllocateGlobalMemImmediate::toString() const
{
    return "ALLOCATEGMEMV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString() + "," + getThirdArgument()->toString();
}

string OAllocateGlobalMemRegister::toString() const
{
    return "ALLOCATEGMEMR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODeallocateMemRegister::toString() const
{
    return "DEALLOCATEMEMR " + getArgument()->toString();
}

string ODeallocateMemImmediate::toString() const
{
    return "DEALLOCATEMEMV " + getArgument()->toString();
}

string OResizeArrayRegister::toString() const
{
    return "RESIZEARRAYR " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OOwnArrayRegister::toString() const
{
    return "OWNARRAYR " + getArgument()->toString();
}
string ODestroyArrayRegister::toString() const
{
    return "DESTROYARRAYR " + getArgument()->toString();
}

string OSave::toString() const
{
    return "SAVE";
}

string OGetScreenFlags::toString() const
{
    return "GETSCREENFLAGS " + getArgument()->toString();
}

string OGetScreenEFlags::toString() const
{
    return "GETSCREENEFLAGS " + getArgument()->toString();
}

string OEnd::toString() const
{
    return "GAMEEND";
}

string OGameReload::toString() const
{
    return "GAMERELOAD";
}

string OGameContinue::toString() const
{
    return "GAMECONTINUE";
}

string OGameSaveQuit::toString() const
{
    return "GAMESAVEQUIT";
}

string OGameSaveContinue::toString() const
{
    return "GAMESAVECONTINUE";
}

string OShowF6Screen::toString() const
{
    return "SHOWF6SCREEN";
}

string OComboTile::toString() const
{
    return "COMBOTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OBreakShield::toString() const
{
    return "BREAKSHIELD " + getArgument()->toString();
}

string OShowSaveScreen::toString() const
{
    return "SAVESCREEN " + getArgument()->toString();
}

string OShowSaveQuitScreen::toString() const
{
    return "SAVEQUITSCREEN";
}

string OSelectAWeaponRegister::toString() const
{
    return "SELECTAWPNR " + getArgument()->toString();
}

string OSelectBWeaponRegister::toString() const
{
    return "SELECTBWPNR " + getArgument()->toString();
}

string OSelectXWeaponRegister::toString() const
{
    return "SELECTXWPNR " + getArgument()->toString();
}

string OSelectYWeaponRegister::toString() const
{
    return "SELECTYWPNR " + getArgument()->toString();
}

string OGetFFCScript::toString() const
{
    return "GETFFCSCRIPT " + getArgument()->toString();
}

string OGetComboScript::toString() const
{
    return "GETCOMBOSCRIPT " + getArgument()->toString();
}

//2.54

string OGreyscaleOn::toString() const
{
    return "GREYSCALEON";
}

string OGreyscaleOff::toString() const
{
    return "GREYSCALEOFF";
}

string OZapIn::toString() const
{
    return "ZAPIN";
}

string OZapOut::toString() const
{
    return "ZAPOUT";
}

//These need to be unary opcodes that accept bool linkvisible. 
string OWavyIn::toString() const
{
    return "WAVYIN";
}

string OWavyOut::toString() const
{
    return "WAVYOUT";
}

string OOpenWipe::toString() const
{
    return "OPENWIPE";
}

string OCloseWipe::toString() const
{
    return "CLOSEWIPE";
}

string OOpenWipeShape::toString() const
{
    return "OPENWIPESHAPE " + getArgument()->toString();
}

string OCloseWipeShape::toString() const
{
    return "CLOSEWIPESHAPE " + getArgument()->toString();
}

//Game->GetItemScript(int32_t ptr[])
string OGetItemScript::toString() const
{
    return "GETITEMSCRIPT " + getArgument()->toString();
}


string OGetLWeaponPointer::toString() const
{
    return "LWPNARRPTR " + getArgument()->toString();
}

string OSetLWeaponPointer::toString() const
{
    return "LWPNARRPTR2 " + getArgument()->toString();
}

string OGetEWeaponPointer::toString() const
{
    return "EWPNARRPTR " + getArgument()->toString();
}

string OSetEWeaponPointer::toString() const
{
    return "EWPNARRPTR2 " + getArgument()->toString();
}

string OGetItemPointer::toString() const
{
    return "IDATAARRPTR " + getArgument()->toString();
}

string OSetItemPointer::toString() const
{
    return "ITEMARRPTR2 " + getArgument()->toString();
}

string OGetItemDataPointer::toString() const
{
    return "IDATAARRPTR " + getArgument()->toString();
}

string OSetItemDataPointer::toString() const
{
    return "IDATAARRPTR2 " + getArgument()->toString();
}

string OGetFFCPointer::toString() const
{
    return "FFCARRPTR " + getArgument()->toString();
}

string OSetFFCPointer::toString() const
{
    return "FFCARRPTR2 " + getArgument()->toString();
}

string OGetBoolPointer::toString() const
{
    return "BOOLARRPTR " + getArgument()->toString();
}

string OSetBoolPointer::toString() const
{
    return "BOOLARRPTR2 " + getArgument()->toString();
}

string OGetNPCPointer::toString() const
{
    return "NPCARRPTR " + getArgument()->toString();
}

string OSetNPCPointer::toString() const
{
    return "NPCARRPTR2 " + getArgument()->toString();
}


string OGetScreenDoor::toString() const
{
    return "GETSCREENDOOR " + getArgument()->toString();
}

string OGetScreenEnemy::toString() const
{
    return "GETSCREENENEMY " + getArgument()->toString();
}


string OGetScreenLayerOpacity::toString() const
{
    return "GETSCREENLAYOP " + getArgument()->toString();
}

string OGetScreenSecretCombo::toString() const
{
    return "GETSCREENSECCMB " + getArgument()->toString();
}

string OGetScreenSecretCSet::toString() const
{
    return "GETSCREENSECCST " + getArgument()->toString();
}

string OGetScreenSecretFlag::toString() const
{
    return "GETSCREENSECFLG " + getArgument()->toString();
}

string OGetScreenLayerMap::toString() const
{
    return "GETSCREENLAYMAP " + getArgument()->toString();
}

string OGetScreenLayerScreen::toString() const
{
    return "GETSCREENLAYSCR " + getArgument()->toString();
}

string OGetScreenPath::toString() const
{
    return "GETSCREENPATH " + getArgument()->toString();
}

string OGetScreenWarpReturnX::toString() const
{
    return "GETSCREENWARPRX " + getArgument()->toString();
}

string OGetScreenWarpReturnY::toString() const
{
    return "GETSCREENWARPRY " + getArgument()->toString();
}

string OTriggerSecretRegister::toString() const
{
    return "TRIGGERSECRETR " + getArgument()->toString();
}

string OBMPPolygonRegister::toString() const
{
    return "BMPPOLYGONR";
}

string OPolygonRegister::toString() const
{
    return "POLYGONR";
}


//NPCData

string ONDataBaseTile::toString() const
{
    return "GETNPCDATATILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataEHeight::toString() const
{
    return "GETNPCDATAEHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string ONDataFlags::toString() const
{
    return "GETNPCDATAFLAGS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataFlags2::toString() const
{
    return "GETNPCDATAFLAGS2 " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataWidth::toString() const
{
    return "GETNPCDATAWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHeight::toString() const
{
    return "GETNPCDATAHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataTile::toString() const
{
    return "GETNPCDATASTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSWidth::toString() const
{
    return "GETNPCDATASWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSHeight::toString() const
{
    return "GETNPCDATASHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataETile::toString() const
{
    return "GETNPCDATAETILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataEWidth::toString() const
{
    return "GETNPCDATAEWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHP::toString() const
{
    return "GETNPCDATAHP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataFamily::toString() const
{
    return "GETNPCDATAFAMILY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataCSet::toString() const
{
    return "GETNPCDATACSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataAnim::toString() const
{
    return "GETNPCDATAANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataEAnim::toString() const
{
    return "GETNPCDATAEANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataFramerate::toString() const
{
    return "GETNPCDATAFRAMERATE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataEFramerate::toString() const
{
    return "GETNPCDATAEFRAMERATE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataTouchDamage::toString() const
{
    return "GETNPCDATATOUCHDMG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataWeaponDamage::toString() const
{
    return "GETNPCDATAWPNDAMAGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataWeapon::toString() const
{
    return "GETNPCDATAWEAPON " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataRandom::toString() const
{
    return "GETNPCDATARANDOM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHalt::toString() const
{
    return "GETNPCDATAHALT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataStep::toString() const
{
    return "GETNPCDATASTEP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHoming::toString() const
{
    return "GETNPCDATAHOMING " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHunger::toString() const
{
    return "GETNPCDATAHUNGER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataropset::toString() const
{
    return "GETNPCDATADROPSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataBGSound::toString() const
{
    return "GETNPCDATABGSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHitSound::toString() const
{
    return "GETNPCDATAHITSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataDeathSound::toString() const
{
    return "GETNPCDATADEATHSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataXofs::toString() const
{
    return "GETNPCDATAXOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataYofs::toString() const
{
    return "GETNPCDATAYOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataZofs::toString() const
{
    return "GETNPCDATAZOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHitXOfs::toString() const
{
    return "GETNPCDATAHXOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHYOfs::toString() const
{
    return "GETNPCDATAHYOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHitWidth::toString() const
{
    return "GETNPCDATAHITWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHitHeight::toString() const
{
    return "GETNPCDATAHITHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataHitZ::toString() const
{
    return "GETNPCDATAHITZ " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataTileWidth::toString() const
{
    return "GETNPCDATATILEWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataTileHeight::toString() const
{
    return "GETNPCDATATILEHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataWeapSprite::toString() const
{
    return "GETNPCDATAWPNSPRITE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

//two inputs, one return
string ONDataScriptDef::toString() const
{
    return "GETNPCDATASCRIPTDEF " + getArgument()->toString();
}
//two inputs, one return
string ONDataDefense::toString() const
{
    return "GETNPCDATADEFENSE " + getArgument()->toString();
}
//two inputs, one return
string ONDataSizeFlag::toString() const
{
    return "GETNPCDATASIZEFLAG " + getArgument()->toString();
}
//two inputs, one return
string ONDatattributes::toString() const
{
    return "GETNPCDATAATTRIBUTE " + getArgument()->toString();
}

string ONDataSetBaseTile::toString() const
{
    return "SETNPCDATATILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetEHeight::toString() const
{
    return "SETNPCDATAEHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string ONDataSetFlags::toString() const
{
    return "SETNPCDATAFLAGS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetFlags2::toString() const
{
    return "SETNPCDATAFLAGS2 " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetWidth::toString() const
{
    return "SETNPCDATAWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHeight::toString() const
{
    return "SETNPCDATAHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetTile::toString() const
{
    return "SETNPCDATASTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetSWidth::toString() const
{
    return "SETNPCDATASWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetSHeight::toString() const
{
    return "SETNPCDATASHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetETile::toString() const
{
    return "SETNPCDATAETILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetEWidth::toString() const
{
    return "SETNPCDATAEWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHP::toString() const
{
    return "SETNPCDATAHP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetFamily::toString() const
{
    return "SETNPCDATAFAMILY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetCSet::toString() const
{
    return "SETNPCDATACSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetAnim::toString() const
{
    return "SETNPCDATAANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetEAnim::toString() const
{
    return "SETNPCDATAEANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetFramerate::toString() const
{
    return "SETNPCDATAFRAMERATE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetEFramerate::toString() const
{
    return "SETNPCDATAEFRAMERATE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetTouchDamage::toString() const
{
    return "SETNPCDATATOUCHDMG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetWeaponDamage::toString() const
{
    return "SETNPCDATAWPNDAMAGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetWeapon::toString() const
{
    return "SETNPCDATAWEAPON " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetRandom::toString() const
{
    return "SETNPCDATARANDOM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHalt::toString() const
{
    return "SETNPCDATAHALT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetStep::toString() const
{
    return "SETNPCDATASTEP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHoming::toString() const
{
    return "SETNPCDATAHOMING " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHunger::toString() const
{
    return "SETNPCDATAHUNGER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetropset::toString() const
{
    return "SETNPCDATADROPSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetBGSound::toString() const
{
    return "SETNPCDATABGSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHitSound::toString() const
{
    return "SETNPCDATAHITSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetDeathSound::toString() const
{
    return "SETNPCDATADEATHSFX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetXofs::toString() const
{
    return "SETNPCDATAXOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetYofs::toString() const
{
    return "SETNPCDATAYOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetZofs::toString() const
{
    return "SETNPCDATAZOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHitXOfs::toString() const
{
    return "SETNPCDATAHXOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHYOfs::toString() const
{
    return "SETNPCDATAHYOFS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHitWidth::toString() const
{
    return "SETNPCDATAHITWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHitHeight::toString() const
{
    return "SETNPCDATAHITHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetHitZ::toString() const
{
    return "SETNPCDATAHITZ " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetTileWidth::toString() const
{
    return "SETNPCDATATILEWIDTH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetTileHeight::toString() const
{
    return "SETNPCDATATILEHEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string ONDataSetWeapSprite::toString() const
{
    return "SETNPCDATAWPNSPRITE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

//ComboData

string OCDataBlockEnemy::toString() const
{
    return "GCDBLOCKENEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataBlockHole::toString() const
{
    return "GCDBLOCKHOLE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataBlockTrig::toString() const
{
    return "GCDBLOCKTRIG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataConveyX::toString() const
{
    return "GCDCONVEYSPDX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataConveyY::toString() const
{
    return "GCDCONVEYSPDY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataCreateNPC::toString() const
{
    return "GCDCREATEENEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataCreateEnemW::toString() const
{
    return "GCDCREATEENEMWH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataCreateEnemC::toString() const
{
    return "GCDCREATEENEMCH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataDirch::toString() const
{
    return "GCDDIRCHTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataDistTiles::toString() const
{
    return "GCDDISTCHTILES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataDiveItem::toString() const
{
    return "GCDDIVEITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataDock::toString() const
{
    return "GCDDOCK " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFairy::toString() const
{
    return "GCDFAIRY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataAttrib::toString() const
{
    return "GCDFFCOMBOATTRIB " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataDecoTile::toString() const
{
    return "GCDFOOTDECOTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OCDataLadderPass::toString() const
{
    return "GCDLADDERPASS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataLadderPass::toString() const
{
    return "SCDLADDERPASS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}


string OCDataDecoType::toString() const
{
    return "GCDFOOTDECOTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataHookshotGrab::toString() const
{
    return "GCDHOOKSHOTGRAB " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataLockBlock::toString() const
{
    return "GCDLOCKBLOCKTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataLockBlockChange::toString() const
{
    return "GCDLOCKBLOCKCHANGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataMagicMirror::toString() const
{
    return "GCDMAGICMIRRORTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataModHP::toString() const
{
    return "GCDMODIFYHPAMOUNT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataModHPDelay::toString() const
{
    return "GCDMODIFYHPDELAY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataModHpType::toString() const
{
    return "GCDMODIFYHPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataModMP::toString() const
{
    return "GCDMODIFYMPAMOUNT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataMpdMPDelay::toString() const
{
    return "GCDMODIFYMPDELAY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataModMPType::toString() const
{
    return "GCDMODIFYMPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataNoPush::toString() const
{
    return "GCDNOPUSHBLOCKS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataOverhead::toString() const
{
    return "GCDOVERHEAD " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataEnemyLoc::toString() const
{
    return "GCDPLACEENEMY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataPushDir::toString() const
{
    return "GCDPUSHDIR " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataPushWeight::toString() const
{
    return "GCDPUSHWEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataPushWait::toString() const
{
    return "GCDPUSHWAIT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataPushed::toString() const
{
    return "GCDPUSHED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataRaft::toString() const
{
    return "GCDRAFT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataResetRoom::toString() const
{
    return "GCDRESETROOM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSavePoint::toString() const
{
    return "GCDSAVEPOINT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFreeezeScreen::toString() const
{
    return "GCDSCREENFREEZE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSecretCombo::toString() const
{
    return "GCDSECRETCOMBO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSingular::toString() const
{
    return "GCDSINGULAR " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSlowMove::toString() const
{
    return "GCDSLOWMOVE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStatue::toString() const
{
    return "GCDSTATUE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStepType::toString() const
{
    return "GCDSTEPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSteoChange::toString() const
{
    return "GCDSTEPCHANGETO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStrikeRem::toString() const
{
    return "GCDSTRIKEREMNANTS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStrikeRemType::toString() const
{
    return "GCDSTRIKEREMNANTSTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStrikeChange::toString() const
{
    return "GCDSTRIKECHANGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataStrikeChangeItem::toString() const
{
    return "GCDSTRIKECHANGEITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTouchItem::toString() const
{
    return "GCDTOUCHITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTouchStairs::toString() const
{
    return "GCDTOUCHSTAIRS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTriggerType::toString() const
{
    return "GCDTRIGGERTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTriggerSens::toString() const
{
    return "GCDTRIGGERSENS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWarpType::toString() const
{
    return "GCDWARPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWarpSens::toString() const
{
    return "GCDWARPSENS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWarpDirect::toString() const
{
    return "GCDWARPDIRECT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWarpLoc::toString() const
{
    return "GCDWARPLOCATION " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWater::toString() const
{
    return "GCDWATER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OCDataWinGame::toString() const
{
    return "GCDWINGAME " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWhistle::toString() const
{
    return "GCDWHISTLE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWeapBlockLevel::toString() const
{
    return "GCDBLOCKWEAPLVL " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTile::toString() const
{
    return "GCDTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFlip::toString() const
{
    return "GCDFLIP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataWalkability::toString() const
{
    return "GCDWALK " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataType::toString() const
{
    return "GCDTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataCSets::toString() const
{
    return "GCDCSETS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFoo::toString() const
{
    return "GCDFOO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFrames::toString() const
{
    return "GCDFRAMES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSpeed::toString() const
{
    return "GCDSPEED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataNext::toString() const
{
    return "GCDNEXTCOMBO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataNextCSet::toString() const
{
    return "GCDNEXTCSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataFlag::toString() const
{
    return "GCDFLAG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataSkipAnim::toString() const
{
    return "GCDSKIPANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataTimer::toString() const
{
    return "GCDNEXTTIMER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataAnimY::toString() const
{
    return "GCDSKIPANIMY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCDataAnimFlags::toString() const
{
    return "GCDANIMFLAGS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OCDataBlockWeapon::toString() const
{
    return "GCDBLOCKWEAPON " + getArgument()->toString();
}
string OCDataExpansion::toString() const
{
    return "GCDEXPANSION " + getArgument()->toString();
}
string OCDataStrikeWeapon::toString() const
{
    return "GCDSTRIKEWEAPONS " + getArgument()->toString();
}

string OCSetDataBlockEnemy::toString() const
{
    return "SCDBLOCKENEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataBlockHole::toString() const
{
    return "SCDBLOCKHOLE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataBlockTrig::toString() const
{
    return "SCDBLOCKTRIG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataConveyX::toString() const
{
    return "SCDCONVEYSPDX " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataConveyY::toString() const
{
    return "SCDCONVEYSPDY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataCreateNPC::toString() const
{
    return "SCDCREATEENEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataCreateEnemW::toString() const
{
    return "SCDCREATEENEMWH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataCreateEnemC::toString() const
{
    return "SCDCREATEENEMCH " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDirch::toString() const
{
    return "SCDDIRCHTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDistTiles::toString() const
{
    return "SCDDISTCHTILES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDiveItem::toString() const
{
    return "SCDDIVEITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDock::toString() const
{
    return "SCDDOCK " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFairy::toString() const
{
    return "SCDFAIRY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataAttrib::toString() const
{
    return "SCDFFCOMBOATTRIB " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDecoTile::toString() const
{
    return "SCDFOOTDECOTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataDecoType::toString() const
{
    return "SCDFOOTDECOTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataHookshotGrab::toString() const
{
    return "SCDHOOKSHOTGRAB " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataLockBlock::toString() const
{
    return "SCDLOCKBLOCKTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataLockBlockChange::toString() const
{
    return "SCDLOCKBLOCKCHANGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataMagicMirror::toString() const
{
    return "SCDMAGICMIRRORTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataModHP::toString() const
{
    return "SCDMODIFYHPAMOUNT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataModHPDelay::toString() const
{
    return "SCDMODIFYHPDELAY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataModHpType::toString() const
{
    return "SCDMODIFYHPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataModMP::toString() const
{
    return "SCDMODIFYMPAMOUNT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataMpdMPDelay::toString() const
{
    return "SCDMODIFYMPDELAY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataModMPType::toString() const
{
    return "SCDMODIFYMPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataNoPush::toString() const
{
    return "SCDNOPUSHBLOCKS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataOverhead::toString() const
{
    return "SCDOVERHEAD " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataEnemyLoc::toString() const
{
    return "SCDPLACEENEMY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataPushDir::toString() const
{
    return "SCDPUSHDIR " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataPushWeight::toString() const
{
    return "SCDPUSHWEIGHT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataPushWait::toString() const
{
    return "SCDPUSHWAIT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataPushed::toString() const
{
    return "SCDPUSHED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataRaft::toString() const
{
    return "SCDRAFT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataResetRoom::toString() const
{
    return "SCDRESETROOM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSavePoint::toString() const
{
    return "SCDSAVEPOINT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFreeezeScreen::toString() const
{
    return "SCDSCREENFREEZE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSecretCombo::toString() const
{
    return "SCDSECRETCOMBO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSingular::toString() const
{
    return "SCDSINGULAR " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSlowMove::toString() const
{
    return "SCDSLOWMOVE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStatue::toString() const
{
    return "SCDSTATUE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStepType::toString() const
{
    return "SCDSTEPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSteoChange::toString() const
{
    return "SCDSTEPCHANGETO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStrikeRem::toString() const
{
    return "SCDSTRIKEREMNANTS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStrikeRemType::toString() const
{
    return "SCDSTRIKEREMNANTSTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStrikeChange::toString() const
{
    return "SCDSTRIKECHANGE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataStrikeChangeItem::toString() const
{
    return "SCDSTRIKECHANGEITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTouchItem::toString() const
{
    return "SCDTOUCHITEM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTouchStairs::toString() const
{
    return "SCDTOUCHSTAIRS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTriggerType::toString() const
{
    return "SCDTRIGGERTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTriggerSens::toString() const
{
    return "SCDTRIGGERSENS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWarpType::toString() const
{
    return "SCDWARPTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWarpSens::toString() const
{
    return "SCDWARPSENS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWarpDirect::toString() const
{
    return "SCDWARPDIRECT " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWarpLoc::toString() const
{
    return "SCDWARPLOCATION " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWater::toString() const
{
    return "SCDWATER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWhistle::toString() const
{
    return "SCDWHISTLE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWeapBlockLevel::toString() const
{
    return "SCDBLOCKWEAPLVL " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTile::toString() const
{
    return "SCDTILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFlip::toString() const
{
    return "SCDFLIP " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataWalkability::toString() const
{
    return "SCDWALK " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataType::toString() const
{
    return "SCDTYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataCSets::toString() const
{
    return "SCDCSETS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFoo::toString() const
{
    return "SCDFOO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFrames::toString() const
{
    return "SCDFRAMES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSpeed::toString() const
{
    return "SCDSPEED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataNext::toString() const
{
    return "SCDNEXTCOMBO " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataNextCSet::toString() const
{
    return "SCDNEXTCSET " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataFlag::toString() const
{
    return "SCDFLAG " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataSkipAnim::toString() const
{
    return "SCDSKIPANIM " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataTimer::toString() const
{
    return "SCDNEXTTIMER " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataAnimY::toString() const
{
    return "SCDSKIPANIMY " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OCSetDataAnimFlags::toString() const
{
    return "SCDANIMFLAGS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OCSetDataWinGame::toString() const
{
    return "SCDWINGAME " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

//SpriteData
string OSDataTile::toString() const
{
    return "GETSPRITEDATATILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSDataMisc::toString() const
{
    return "GETSPRITEDATAMISC " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSDataCSets::toString() const
{
    return "GETSPRITEDATACGETS " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSDataFrames::toString() const
{
    return "GETSPRITEDATAFRAMES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSDataSpeed::toString() const
{
    return "GETSPRITEDATASPEED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSDataType::toString() const
{
    return "GETSPRITEDATATYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

string OSSetDataTile::toString() const
{
    return "SETSPRITEDATATILE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetDataMisc::toString() const
{
    return "SETSPRITEDATAMISC " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetDataFrames::toString() const
{
    return "SETSPRITEDATAFRAMES " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetDataSpeed::toString() const
{
    return "SETSPRITEDATASPEED " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetDataType::toString() const
{
    return "SETSPRITEDATATYPE " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

//Continue Screen Settings
string OSSetContinueScreen::toString() const
{
    return "SETCONTINUESCREEN " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}
string OSSetContinueString::toString() const
{
    return "SETCONTINUESTRING " + getFirstArgument()->toString() + "," +  getSecondArgument()->toString();
}

//Visual effects with one bool arg.

string OWavyR::toString() const
{
    return "FXWAVYR " + getArgument()->toString();
}

string OZapR::toString() const
{
    return "FXZAPR " + getArgument()->toString();
}

string OGreyscaleR::toString() const
{
    return "GREYSCALER " + getArgument()->toString();
}

string OMonochromeR::toString() const
{
    return "MONOCHROMER " + getArgument()->toString();
}

string OClearTint::toString() const
{
    return "CLEARTINT";
}

string OTintR::toString() const
{
    return "TINT";
}

string OMonoHueR::toString() const
{
    return "MONOHUE";
}

//Bitmap commands

string OBMPRectangleRegister::toString() const
{
    return "BMPRECTR";
}

string OBMPFrameRegister::toString() const
{
    return "BMPFRAMER";
}

string OBMPCircleRegister::toString() const
{
    return "BMPCIRCLER";
}

string OBMPArcRegister::toString() const
{
    return "BMPARCR";
}

string OBMPEllipseRegister::toString() const
{
    return "BMPELLIPSER";
}

string OBMPLineRegister::toString() const
{
    return "BMPLINER";
}

string OBMPSplineRegister::toString() const
{
    return "BMPSPLINER";
}

string OBMPPutPixelRegister::toString() const
{
    return "BMPPUTPIXELR";
}

string OBMPDrawCharRegister::toString() const
{
    return "BMPDRAWCHARR";
}

string OBMPDrawIntRegister::toString() const
{
    return "BMPDRAWINTR";
}

string OBMPDrawTileRegister::toString() const
{
    return "BMPDRAWTILER";
}

string OBMPDrawTileCloakedRegister::toString() const
{
    return "BMPDRAWTILECLOAKEDR";
}

string OBMPDrawComboRegister::toString() const
{
    return "BMPDRAWCOMBOR";
}

string OBMPDrawComboCloakedRegister::toString() const
{
    return "BMPDRAWCOMBOCLOAKEDR";
}

string OBMPFastTileRegister::toString() const
{
    return "BMPFASTTILER";
}

string OBMPFastComboRegister::toString() const
{
    return "BMPFASTCOMBOR";
}

string OBMPDrawStringRegister::toString() const
{
    return "BMPDRAWSTRINGR";
}

string OBMPDrawString2Register::toString() const
{
    return "BMPDRAWSTRINGR2";
}

string OBMPDrawBitmapExRegister::toString() const
{
    return "BMPBLIT";
}
string OBMPBlitTO::toString() const
{
    return "BMPBLITTO";
}

string OBMPGetPixel::toString() const
{
    return "BITMAPGETPIXEL";
}
string OBMPMode7::toString() const
{
    return "BMPMODE7";
}

string OBMPQuadRegister::toString() const
{
    return "BMPQUADR";
}

string OBMPTriangleRegister::toString() const
{
    return "BMPTRIANGLER";
}


string OBMPQuad3DRegister::toString() const
{
    return "BMPQUAD3DR";
}

string OBMPTriangle3DRegister::toString() const
{
    return "BMPTRIANGLE3DR";
}

string OBMPDrawLayerRegister::toString() const
{
    return "BMPDRAWLAYERR";
}

string OBMPDrawScreenRegister::toString() const
{
    return "BMPDRAWSCREENR";
}

string OHeroWarpExRegister::toString() const
{
    return "LINKWARPEXR " + getArgument()->toString();
}

string OHeroExplodeRegister::toString() const
{
    return "LINKEXPLODER " + getArgument()->toString();
}

string OSwitchNPC::toString() const
{
	return "SWITCHNPC " + getArgument()->toString();
}
string OSwitchItem::toString() const
{
	return "SWITCHITM " + getArgument()->toString();
}
string OSwitchLW::toString() const
{
	return "SWITCHLW " + getArgument()->toString();
}
string OSwitchEW::toString() const
{
	return "SWITCHEW " + getArgument()->toString();
}
string OSwitchCombo::toString() const
{
	return "SWITCHCMB " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OKillPlayer::toString() const
{
	return "KILLPLAYER " + getArgument()->toString();
}

string OScreenDoSpawn::toString() const
{
	return "SCREENDOSPAWN";
}

string OScreenTriggerCombo::toString() const
{
	return "SCRTRIGGERCOMBO " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ONPCMovePaused::toString() const
{
	return "NPCMOVEPAUSED";
}
string ONPCMove::toString() const
{
	return "NPCMOVE";
}
string ONPCMoveAngle::toString() const
{
	return "NPCMOVEANGLE";
}
string ONPCMoveXY::toString() const
{
	return "NPCMOVEXY";
}
string ONPCCanMoveDir::toString() const
{
	return "NPCCANMOVEDIR";
}
string ONPCCanMoveAngle::toString() const
{
	return "NPCCANMOVEANGLE";
}
string ONPCCanMoveXY::toString() const
{
	return "NPCCANMOVEXY";
}
string ONPCCanPlace::toString() const
{
	return "NPCCANPLACE";
}
string ONPCIsFlickerFrame::toString() const
{
	return "NPCISFLICKERFRAME";
}


string OGetSystemRTCRegister::toString() const
{
    return "GETRTCTIMER " + getArgument()->toString();
}


string ONPCExplodeRegister::toString() const
{
    return "NPCEXPLODER " + getArgument()->toString();
}
string OLWeaponExplodeRegister::toString() const
{
    return "LWEAPONEXPLODER " + getArgument()->toString();
}
string OEWeaponExplodeRegister::toString() const
{
    return "EWEAPONEXPLODER " + getArgument()->toString();
}
string OItemExplodeRegister::toString() const
{
    return "ITEMEXPLODER " + getArgument()->toString();
}

string ORunItemScript::toString() const
{
    return "RUNITEMSCRIPT " + getArgument()->toString();
}

//new npc-> functions for npc scripts
string ONPCDead::toString() const
{
    return "NPCDEAD " + getArgument()->toString();
}
string ONPCCanSlide::toString() const
{
    return "NPCCANSLIDE " + getArgument()->toString();
}
string ONPCSlide::toString() const
{
    return "NPCSLIDE " + getArgument()->toString();
}
string ONPCBreatheFire::toString() const
{
    return "NPCFIREBREATH " + getArgument()->toString();
}
string ONPCNewDir8::toString() const
{
    return "NPCNEWDIR8 " + getArgument()->toString();
}
string ONPCRemove::toString() const
{
    return "NPCKICKBUCKET " + getArgument()->toString();
}
string OLWpnRemove::toString() const
{
    return "LWPNDEL";
}
string OEWpnRemove::toString() const
{
    return "EWPNDEL";
}
string OItemRemove::toString() const
{
    return "ITEMDEL";
}
string ONPCStopSFX::toString() const
{
    return "NPCSTOPBGSFX " + getArgument()->toString();
}
string ONPCAttack::toString() const
{
    return "NPCATTACK " + getArgument()->toString();
}
string ONPCNewDir::toString() const
{
    return "NPCNEWDIR " + getArgument()->toString();
}
string ONPCConstWalk::toString() const
{
    return "NPCCONSTWALK " + getArgument()->toString();
}
string ONPCConstWalk8::toString() const
{
    return "NPCCONSTWALK8 " + getArgument()->toString();
}
string ONPCVarWalk::toString() const
{
    return "NPCVARWALK " + getArgument()->toString();
}
string ONPCVarWalk8::toString() const
{
    return "NPCVARWALK8 " + getArgument()->toString();
}
string ONPCHaltWalk::toString() const
{
    return "NPCHALTWALK " + getArgument()->toString();
}
string ONPCHaltWalk8::toString() const
{
    return "NPCHALTWALK8 " + getArgument()->toString();
}
string ONPCFloatWalk::toString() const
{
    return "NPCFLOATWALK " + getArgument()->toString();
}
string ONPCHeroInRange::toString() const
{
    return "NPCLINKINRANGE " + getArgument()->toString();
}
string ONPCAdd::toString() const
{
    return "NPCADD " + getArgument()->toString();
}
string ONPCCanMove::toString() const
{
    return "NPCCANMOVE " + getArgument()->toString();
}
string ONPCHitWith::toString() const
{
    return "NPCHITWITH " + getArgument()->toString();
}
string ONPCKnockback::toString() const
{
    return "NPCKNOCKBACK " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetNPCDataName::toString() const
{
    return "NPCDATAGETNAME " + getArgument()->toString();
}

string OAllocateBitmap::toString() const
{
    return "ALLOCATEBITMAP " + getArgument()->toString();
}

string OClearBitmap::toString() const
{
    return "CLEARBITMAP";
}

string OBitmapClearToColor::toString() const
{
    return "BITMAPCLEARTOCOLOR";
}

string ORegenerateBitmap::toString() const
{
    return "REGENERATEBITMAP";
}

string OReadBitmap::toString() const
{
    return "READBITMAP";
}
string OWriteBitmap::toString() const
{
    return "WRITEBITMAP";
}
string OBitmapFree::toString() const
{
    return "BITMAPFREE";
}
string OBitmapOwn::toString() const
{
    return "BITMAPOWN";
}
string OFileOwn::toString() const
{
    return "FILEOWN";
}
string ODirectoryOwn::toString() const
{
    return "DIRECTORYOWN";
}
string ORNGOwn::toString() const
{
    return "RNGOWN";
}
string OBitmapWriteTile::toString() const
{
    return "BMPWRITETILE";
}
string OBitmapDither::toString() const
{
    return "BMPDITHER";
}
string OBitmapReplColor::toString() const
{
    return "BMPREPLCOLOR";
}
string OBitmapShiftColor::toString() const
{
    return "BMPSHIFTCOLOR";
}
string OBitmapMaskDraw::toString() const
{
    return "BMPMASKDRAW";
}
string OBitmapMaskDraw2::toString() const
{
    return "BMPMASKDRAW2";
}
string OBitmapMaskDraw3::toString() const
{
    return "BMPMASKDRAW3";
}
string OBitmapMaskBlit::toString() const
{
    return "BMPMASKBLIT";
}
string OBitmapMaskBlit2::toString() const
{
    return "BMPMASKBLIT2";
}
string OBitmapMaskBlit3::toString() const
{
    return "BMPMASKBLIT3";
}

string OIsValidBitmap::toString() const
{
    return "ISVALIDBITMAP " + getArgument()->toString();
}

string OIsAllocatedBitmap::toString() const
{
    return "ISALLOCATEDBITMAP " + getArgument()->toString();
}


string OBMPDrawScreenSolidRegister::toString() const
{
    return "BMPDRAWSCREENSOLIDR";
}

string OBMPDrawScreenSolid2Register::toString() const
{
    return "BMPDRAWSCREENSOLID2R";
}
string OBMPDrawScreenComboFRegister::toString() const
{
    return "BMPDRAWSCREENCOMBOFR";
}
string OBMPDrawScreenComboIRegister::toString() const
{
    return "BMPDRAWSCREENCOMBOIR";
}
string OBMPDrawScreenComboTRegister::toString() const
{
    return "BMPDRAWSCREENCOMBOTR";
}
string OGraphicsGetpixel::toString() const
{
    return "GRAPHICSGETPIXEL " + getArgument()->toString();
}
string OGraphicsCountColor::toString() const
{
    return "GRAPHICSCOUNTCOLOR " + getArgument()->toString();
}

string OBMPDrawScreenSolidityRegister::toString() const
{
    return "BMPDRAWLAYERSOLIDITYR";
}
string OBMPDrawScreenSolidMaskRegister::toString() const
{
    return "BMPDRAWLAYERSOLIDR";
}
string OBMPDrawScreenCTypeRegister::toString() const
{
    return "BMPDRAWLAYERCTYPER";
}
string OBMPDrawScreenCFlagRegister::toString() const
{
    return "BMPDRAWLAYERCFLAGR";
}
string OBMPDrawScreenCIFlagRegister::toString() const
{
    return "BMPDRAWLAYERCIFLAGR";
}

//Text ptr opcodes
string OFontHeight::toString() const
{
	return "FONTHEIGHTR " + getArgument()->toString();
}

string OStringWidth::toString() const
{
	return "STRINGWIDTHR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OCharWidth::toString() const
{
	return "CHARWIDTHR " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMessageWidth::toString() const
{
	return "MESSAGEWIDTHR " + getArgument()->toString();
}

string OMessageHeight::toString() const
{
	return "MESSAGEHEIGHTR " + getArgument()->toString();
}

//

string OStrCmp::toString() const
{
    return "STRINGCOMPARE " + getArgument()->toString();
}

string OStrNCmp::toString() const
{
    return "STRINGNCOMPARE " + getArgument()->toString();
}

string OStrICmp::toString() const
{
    return "STRINGICOMPARE " + getArgument()->toString();
}

string OStrNICmp::toString() const
{
    return "STRINGNICOMPARE " + getArgument()->toString();
}

//based on Ostrcpy
string oARRAYCOPY::toString() const
{
    return "ARRAYCOPY " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

/*to do 
  //1 INPUT, NO RETURN 
     { "REMCHR",                2,   0,   0,   0},
     { "STRINGUPPERLOWER",                2,   0,   0,   0},
     { "STRINGLOWERUPPER",                2,   0,   0,   0},
     { "STRINGCONVERTCASE",                2,   0,   0,   0},
     */

//1 inp, 1 ret, baseds on STRINGLENGTH / Ostrlen

string Oxlen::toString() const
{
    return "XLEN " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string Oxtoi::toString() const
{
    return "XTOI " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string Oilen::toString() const
{
    return "ILEN " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string Oatoi::toString() const
{
    return "ATOI " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Oatol::toString() const
{
	return "ATOL " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

//2 inp, 1 ret, based on STRINGCOMPARE / OStrCmp

string Ostrcspn::toString() const
{
    return "STRCSPN " + getArgument()->toString();
}

string Ostrstr::toString() const
{
    return "STRSTR " + getArgument()->toString();
}

string Oitoa::toString() const
{
    return "ITOA " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Oxtoa::toString() const
{
    return "XTOA " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Oitoacat::toString() const
{
    return "ITOACAT " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OSaveGameStructs::toString() const
{
    return "SAVEGAMESTRUCTS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OReadGameStructs::toString() const
{
    return "READGAMESTRUCTS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string Ostrcat::toString() const
{
    return "STRCAT " + getArgument()->toString();
}

string Ostrspn::toString() const
{
    return "STRSPN " + getArgument()->toString();
}
string Ostrchr::toString() const
{
    return "STRCHR " + getArgument()->toString();
}

string Ostrrchr::toString() const
{
    return "STRRCHR " + getArgument()->toString();
}
string Oxlen2::toString() const
{
    return "XLEN2 " + getArgument()->toString();
}

string Oxtoi2::toString() const
{
    return "XTOI2 " + getArgument()->toString();
}
string Oilen2::toString() const
{
    return "ILEN2 " + getArgument()->toString();
}
string Oatoi2::toString() const
{
    return "ATOI2 " + getArgument()->toString();
}
string Oremchr2::toString() const
{
    return "REMCHR2 " + getArgument()->toString();
}


/*to do
  //3 INPUT 1 RET 
    { "XTOA3",		       1,   0,   0,   0},
    { "STRCATF",		       1,   0,   0,   0},
    { "ITOA3",		       1,   0,   0,   0},
    { "STRSTR3",		       1,   0,   0,   0},
    { "REMNCHR3",		       1,   0,   0,   0},
    { "STRCAT3",		       1,   0,   0,   0},
    { "STRNCAT3",		       1,   0,   0,   0},
    { "STRCHR3",		       1,   0,   0,   0},
    { "STRRCHR3",		       1,   0,   0,   0},
    { "STRSPN3",		       1,   0,   0,   0},
    { "STRCSPN3",		       1,   0,   0,   0},
    
*/








string Ouppertolower::toString() const
{
    return "UPPERTOLOWER " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string Olowertoupper::toString() const
{
    return "LOWERTOUPPER " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string Oconvertcase::toString() const
{
    return "CONVERTCASE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

//Game->GetByString functions
//similar to Oconvertcase

string OGETNPCSCRIPT::toString() const
{
    return "GETNPCSCRIPT " + getArgument()->toString();
}
string OGETLWEAPONSCRIPT::toString() const
{
    return "GETLWEAPONSCRIPT " + getArgument()->toString();
}
string OGETEWEAPONSCRIPT::toString() const
{
    return "GETEWEAPONSCRIPT " + getArgument()->toString();
}
string OGETGENERICSCRIPT::toString() const
{
    return "GETGENERICSCRIPT " + getArgument()->toString();
}
string OGETHEROSCRIPT::toString() const
{
    return "GETHEROSCRIPT " + getArgument()->toString();
}
string OGETGLOBALSCRIPT::toString() const
{
    return "GETGLOBALSCRIPT " + getArgument()->toString();
}
string OGETDMAPSCRIPT::toString() const
{
    return "GETDMAPSCRIPT " + getArgument()->toString();
}
string OGETSCREENSCRIPT::toString() const
{
    return "GETSCREENSCRIPT " + getArgument()->toString();
}
string OGETSPRITESCRIPT::toString() const
{
    return "GETSPRITESCRIPT " + getArgument()->toString();
}
string OGETUNTYPEDSCRIPT::toString() const
{
    return "GETUNTYPEDSCRIPT " + getArgument()->toString();
}
string OGETSUBSCREENSCRIPT::toString() const
{
    return "GETSUBSCREENSCRIPT " + getArgument()->toString();
}
string OGETNPCBYNAME::toString() const
{
    return "GETNPCBYNAME " + getArgument()->toString();
}
string OGETITEMBYNAME::toString() const
{
    return "GETITEMBYNAME " + getArgument()->toString();
}
string OGETCOMBOBYNAME::toString() const
{
    return "GETCOMBOBYNAME " + getArgument()->toString();
}
string OGETDMAPBYNAME::toString() const
{
    return "GETDMAPBYNAME " + getArgument()->toString();
}

string OLoadNPCBySUIDRegister::toString() const
{
    return "LOADNPCBYSUID " + getArgument()->toString();
}
string OLoadLWeaponBySUIDRegister::toString() const
{
    return "LOADLWEAPONBYSUID " + getArgument()->toString();
}
string OLoadEWeaponBySUIDRegister::toString() const
{
    return "LOADWEAPONCBYSUID " + getArgument()->toString();
}

string OByte::toString() const
{
    return "TOBYTE " + getArgument()->toString();
}

string OToInteger::toString() const
{
    return "TOINTEGER " + getArgument()->toString();
}

string OFloor::toString() const
{
    return "FLOOR " + getArgument()->toString();
}

string OTruncate::toString() const
{
    return "TRUNCATE " + getArgument()->toString();
}

string ORound::toString() const
{
    return "ROUND " + getArgument()->toString();
}

string ORoundAway::toString() const
{
    return "ROUNDAWAY " + getArgument()->toString();
}

string OCeiling::toString() const
{
    return "CEILING " + getArgument()->toString();
}

string OSByte::toString() const
{
    return "TOSIGNEDBYTE " + getArgument()->toString();
}
string OWord::toString() const
{
    return "TOWORD " + getArgument()->toString();
}
string OShort::toString() const
{
    return "TOSHORT " + getArgument()->toString();
}

string OReturn::toString() const
{
	return "RETURN";
}

string ODirExists::toString() const
{
	return "DIREXISTS " + getArgument()->toString();
}

string OFileExists::toString() const
{
	return "FILEEXISTS " + getArgument()->toString();
}

string OFileSystemRemove::toString() const
{
	return "FILESYSREMOVE " + getArgument()->toString();
}

string OFileClose::toString() const
{
	return "FILECLOSE";
};

string OFileFree::toString() const
{
	return "FILEFREE";
};

string OFileIsAllocated::toString() const
{
	return "FILEISALLOCATED";
};

string OFileIsValid::toString() const
{
	return "FILEISVALID";
};

string OAllocateFile::toString() const
{
	return "FILEALLOCATE";
};

string OFileFlush::toString() const
{
	return "FILEFLUSH";
};

string OFileGetChar::toString() const
{
	return "FILEGETCHAR";
};

string OFileRewind::toString() const
{
	return "FILEREWIND";
};

string OFileClearError::toString() const
{
	return "FILECLEARERR";
};
//
string OFileOpen::toString() const
{
	return "FILEOPEN " + getArgument()->toString();
};

string OFileCreate::toString() const
{
	return "FILECREATE " + getArgument()->toString();
};

string OFileReadString::toString() const
{
	return "FILEREADSTR " + getArgument()->toString();
};

string OFileWriteString::toString() const
{
	return "FILEWRITESTR " + getArgument()->toString();
};

string OFilePutChar::toString() const
{
	return "FILEPUTCHAR " + getArgument()->toString();
};

string OFileUngetChar::toString() const
{
	return "FILEUNGETCHAR " + getArgument()->toString();
};

string OFileGetError::toString() const
{
	return "FILEGETERROR " + getArgument()->toString();
};
//
string OFileReadChars::toString() const
{
	return "FILEREADCHARS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileReadBytes::toString() const
{
	return "FILEREADBYTES " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OFileReadInts::toString() const
{
	return "FILEREADINTS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileWriteChars::toString() const
{
	return "FILEWRITECHARS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileWriteBytes::toString() const
{
	return "FILEWRITEBYTES " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OFileWriteInts::toString() const
{
	return "FILEWRITEINTS " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileSeek::toString() const
{
	return "FILESEEK " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileOpenMode::toString() const
{
	return "FILEOPENMODE " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OFileRemove::toString() const
{
	return "FILEREMOVE";
};

string ODirectoryGet::toString() const
{
	return "DIRECTORYGET " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string ODirectoryReload::toString() const
{
	return "DIRECTORYRELOAD";
}

string ODirectoryFree::toString() const
{
	return "DIRECTORYFREE";
}

string OStackFree::toString() const
{
	return "STACKFREE";
}
string OStackOwn::toString() const
{
	return "STACKOWN";
}
string OStackClear::toString() const
{
	return "STACKCLEAR";
}

string OStackPopBack::toString() const
{
	return "STACKPOPBACK " + getArgument()->toString();
}
string OStackPopFront::toString() const
{
	return "STACKPOPFRONT " + getArgument()->toString();
}
string OStackPeekBack::toString() const
{
	return "STACKPEEKBACK " + getArgument()->toString();
}
string OStackPeekFront::toString() const
{
	return "STACKPEEKFRONT " + getArgument()->toString();
}
string OStackPushBack::toString() const
{
	return "STACKPUSHBACK " + getArgument()->toString();
}
string OStackPushFront::toString() const
{
	return "STACKPUSHFRONT " + getArgument()->toString();
}
string OStackGet::toString() const
{
	return "STACKGET " + getArgument()->toString();
}
string OStackSet::toString() const
{
    return "STACKSET " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OModuleGetIC::toString() const
{
    return "MODULEGETIC " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGetScreenForComboPos::toString() const
{
    return "REGION_SCREEN_FOR_COMBO_POS " + getArgument()->toString();
}


//////////////////////////////////////////////////////////////////////////////////////

string ORunGenericFrozenScript::toString() const
{
	return "RUNGENFRZSCR " + getArgument()->toString();
};

string OReservedZ3_01::toString() const
{
	return "RESRVD_OP_Z3_01 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_02::toString() const
{
	return "RESRVD_OP_Z3_02 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_03::toString() const
{
	return "RESRVD_OP_Z3_03 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_04::toString() const
{
	return "RESRVD_OP_Z3_04 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_05::toString() const
{
	return "RESRVD_OP_Z3_05 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_06::toString() const
{
	return "RESRVD_OP_Z3_06 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_07::toString() const
{
	return "RESRVD_OP_Z3_07 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_08::toString() const
{
	return "RESRVD_OP_Z3_08 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_09::toString() const
{
	return "RESRVD_OP_Z3_09 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OReservedZ3_10::toString() const
{
	return "RESRVD_OP_Z3_10 " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OSubscrSwapPages::toString() const
{
	return "SUBDATA_SWAP_PAGES";
}
string OSubscrPgFindWidget::toString() const
{
	return "SUBPAGE_FIND_WIDGET";
}
string OSubscrPgMvCursor::toString() const
{
	return "SUBPAGE_MOVE_SEL";
}
string OSubscrPgSwapWidgets::toString() const
{
	return "SUBPAGE_SWAP_WIDG";
}
string OSubscrPgNewWidget::toString() const
{
	return "SUBPAGE_NEW_WIDG";
}
string OSubscrPgDelete::toString() const
{
	return "SUBPAGE_DELETE";
}
string OGetSubWidgSelTxtOverride::toString() const
{
	return "SUBWIDG_GET_SELTEXT_OVERRIDE " + getArgument()->toString();
}
string OSetSubWidgSelTxtOverride::toString() const
{
	return "SUBWIDG_SET_SELTEXT_OVERRIDE " + getArgument()->toString();
}
string OSubWidgTy_GetText::toString() const
{
	return "SUBWIDG_TY_GETTEXT " + getArgument()->toString();
}
string OSubWidgTy_SetText::toString() const
{
	return "SUBWIDG_TY_SETTEXT " + getArgument()->toString();
}


string OSubscrPgFindWidgetLbl::toString() const
{
	return "SUBPAGE_FIND_WIDGET_BY_LABEL";
}


string OGetSubWidgLabel::toString() const
{
	return "SUBWIDG_GET_LABEL " + getArgument()->toString();
}
string OSetSubWidgLabel::toString() const
{
	return "SUBWIDG_SET_LABEL " + getArgument()->toString();
}


string OWrapRadians::toString() const
{
	return "WRAPRADIANS " + getArgument()->toString();
}
string OWrapDegrees::toString() const
{
	return "WRAPDEGREES " + getArgument()->toString();
}


string OCallFunc::toString() const
{
	return "CALLFUNC " + getArgument()->toString();
}


string OReturnFunc::toString() const
{
	return "RETURNFUNC";
}


string OSetCompare::toString() const
{
	return "SETCMP " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OGotoCompare::toString() const
{
	return "GOTOCMP " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OStackWriteAtRV::toString() const
{
	return "STACKWRITEATRV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}
string OStackWriteAtVV::toString() const
{
	return "STACKWRITEATVV " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


string OStackWriteAtVV_If::toString() const
{
	return "STACKWRITEATVV_IF " + getFirstArgument()->toString() + "," + getSecondArgument()->toString() + "," + getThirdArgument()->toString();
}

string OLoadWebSocket::toString() const
{
	return "WEBSOCKET_LOAD " + getArgument()->toString();
};

string OWebSocketSend::toString() const
{
	return "WEBSOCKET_SEND " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
};

string OWebSocketFree::toString() const
{
	return "WEBSOCKET_FREE";
}

string OWebSocketOwn::toString() const
{
	return "WEBSOCKET_OWN";
}

string OWebSocketGetError::toString() const
{
	return "WEBSOCKET_ERROR " + getArgument()->toString();
}

string OWebSocketReceive::toString() const
{
	return "WEBSOCKET_RECEIVE " + getArgument()->toString();
}

string OGC::toString() const
{
	return "GC";
}

string ORefInc::toString() const
{
	return "REF_INC " + getArgument()->toString();
}

string ORefDec::toString() const
{
	return "REF_DEC " + getArgument()->toString();
}

string ORefAutorelease::toString() const
{
	return "REF_AUTORELEASE " + getArgument()->toString();
}

string ORefRemove::toString() const
{
	return "REF_REMOVE " + getArgument()->toString();
}

string ORefCount::toString() const
{
	return "REF_COUNT " + getArgument()->toString();
}

string OMarkTypeStack::toString() const
{
	return "MARK_TYPE_STACK " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}

string OMarkTypeRegister::toString() const
{
	return "MARK_TYPE_REG " + getFirstArgument()->toString() + "," + getSecondArgument()->toString();
}


//2.53 Updated to 16th Jan, 2017
#include "ByteCode.h"
#include "CompileError.h"
#include "DataStructs.h"
#include "zsyssimple.h"
#include <assert.h>
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

extern script_variable variable_list[];
string ZScript::VarToString(int32_t ID)
{
	for(int q = 0; variable_list[q].id > -1; ++q)
	{
		if(variable_list[q].id == ID)
			return variable_list[q].name;
	}
	return fmt::format("d{}", ID);
}

string VarArgument::toString() const
{
	return VarToString(ID);
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

int32_t OSetTrue::command() const
{
	return SETTRUE;
}

int32_t OSetTrueI::command() const
{
	return SETTRUEI;
}

int32_t OSetFalse::command() const
{
	return SETFALSE;
}

int32_t OSetFalseI::command() const
{
	return SETFALSEI;
}

int32_t OSetMore::command() const
{
	return SETMORE;
}

int32_t OSetMoreI::command() const
{
	return SETMOREI;
}

int32_t OSetLess::command() const
{
	return SETLESS;
}

int32_t OSetLessI::command() const
{
	return SETLESSI;
}

int32_t OSetImmediate::command() const
{
	return SETV;
}

int32_t OSetRegister::command() const
{
	return SETR;
}

int32_t OReadPODArrayR::command() const
{
	return READPODARRAYR;
}

int32_t OReadPODArrayI::command() const
{
	return READPODARRAYV;
}

int32_t OWritePODArrayRR::command() const
{
	return WRITEPODARRAYRR;
}

int32_t OWritePODArrayRI::command() const
{
	return WRITEPODARRAYRV;
}

int32_t OWritePODArrayIR::command() const
{
	return WRITEPODARRAYVR;
}

int32_t OWritePODArrayII::command() const
{
	return WRITEPODARRAYVV;
}
int32_t OWritePODString::command() const
{
	return WRITEPODSTRING;
}
int32_t OWritePODArray::command() const
{
	return WRITEPODARRAY;
}
int32_t OConstructClass::command() const
{
	return ZCLASS_CONSTRUCT;
}
int32_t OReadObject::command() const
{
	return ZCLASS_READ;
}
int32_t OWriteObject::command() const
{
	return ZCLASS_WRITE;
}
int32_t OFreeObject::command() const
{
	return ZCLASS_FREE;
}
int32_t OOwnObject::command() const
{
	return ZCLASS_OWN;
}
int32_t ODestructor::command() const
{
	return STARTDESTRUCTOR;
}
int32_t OGlobalObject::command() const
{
	return ZCLASS_GLOBALIZE;
}
int32_t OObjOwnBitmap::command() const
{
	return OBJ_OWN_BITMAP;
}
int32_t OObjOwnPaldata::command() const
{
	return OBJ_OWN_PALDATA;
}
int32_t OObjOwnFile::command() const
{
	return OBJ_OWN_FILE;
}
int32_t OObjOwnDir::command() const
{
	return OBJ_OWN_DIR;
}
int32_t OObjOwnStack::command() const
{
	return OBJ_OWN_STACK;
}
int32_t OObjOwnRNG::command() const
{
	return OBJ_OWN_RNG;
}
int32_t OObjOwnClass::command() const
{
	return OBJ_OWN_CLASS;
}
int32_t OObjOwnArray::command() const
{
	return OBJ_OWN_ARRAY;
}
int32_t OQuitNoDealloc::command() const
{
	return QUIT_NO_DEALLOC;
}
int32_t OSetCustomCursor::command() const
{
	return GAMESETCUSTOMCURSOR;
}
int32_t OItemGetDispName::command() const
{
	return ITEMGETDISPLAYNAME;
}
int32_t OItemSetDispName::command() const
{
	return ITEMSETDISPLAYNAME;
}
int32_t OItemGetShownName::command() const
{
	return ITEMGETSHOWNNAME;
}
int32_t OHeroMoveXY::command() const
{
	return HEROMOVEXY;
}
int32_t OHeroCanMoveXY::command() const
{
	return HEROCANMOVEXY;
}
int32_t OHeroLiftRelease::command() const
{
	return HEROLIFTRELEASE;
}
int32_t OHeroLiftGrab::command() const
{
	return HEROLIFTGRAB;
}
int32_t OHeroIsFlickerFrame::command() const
{
	return HEROISFLICKERFRAME;
}
int32_t OLoadPortalRegister::command() const
{
	return LOADPORTAL;
}
int32_t OCreatePortal::command() const
{
	return CREATEPORTAL;
}
int32_t OLoadSavPortalRegister::command() const
{
	return LOADSAVPORTAL;
}
int32_t OCreateSavPortal::command() const
{
	return CREATESAVPORTAL;
}
int32_t OPortalRemove::command() const
{
	return PORTALREMOVE;
}
int32_t OSavedPortalRemove::command() const
{
	return SAVEDPORTALREMOVE;
}
int32_t OSavedPortalGenerate::command() const
{
	return SAVEDPORTALGENERATE;
}
int32_t OUseSpritePortal::command() const
{
	return PORTALUSESPRITE;
}
int32_t OHeroMoveAtAngle::command() const
{
	return HEROMOVEATANGLE;
}
int32_t OHeroCanMoveAtAngle::command() const
{
	return HEROCANMOVEATANGLE;
}
int32_t OHeroMove::command() const
{
	return HEROMOVE;
}
int32_t OHeroCanMove::command() const
{
	return HEROCANMOVE;
}
int32_t ODrawLightCircle::command() const
{
	return DRAWLIGHT_CIRCLE;
}
int32_t ODrawLightSquare::command() const
{
	return DRAWLIGHT_SQUARE;
}
int32_t ODrawLightCone::command() const
{
	return DRAWLIGHT_CONE;
}
int32_t OPeek::command() const
{
	return PEEK;
}
int32_t OPeekAtImmediate::command() const
{
	return PEEKATV;
}
int32_t OMakeVargArray::command() const
{
	return MAKEVARGARRAY;
}
int32_t OPrintfArr::command() const
{
	return PRINTFA;
}
int32_t OSPrintfArr::command() const
{
	return SPRINTFA;
}
int32_t OCurrentItemID::command() const
{
	return CURRENTITEMID;
}
int32_t OArrayPush::command() const
{
	return ARRAYPUSH;
}
int32_t OArrayPop::command() const
{
	return ARRAYPOP;
}
int32_t OLoadSubscreenDataRV::command() const
{
	return LOADSUBDATARV;
}
int32_t OSwapSubscrV::command() const
{
	return SWAPSUBSCREENV;
}
int32_t OGetSubscreenName::command() const
{
	return SUBDATA_GET_NAME;
}
int32_t OSetSubscreenName::command() const
{
	return SUBDATA_SET_NAME;
}


int32_t OAddImmediate::command() const
{
	return ADDV;
}

int32_t OAddRegister::command() const
{
	return ADDR;
}

int32_t OSubImmediate::command() const
{
	return SUBV;
}

int32_t OSubImmediate2::command() const
{
	return SUBV2;
}

int32_t OSubRegister::command() const
{
	return SUBR;
}

int32_t OMultImmediate::command() const
{
	return MULTV;
}


int32_t OMultRegister::command() const
{
	return MULTR;
}

int32_t ODivImmediate::command() const
{
	return DIVV;
}

int32_t ODivImmediate2::command() const
{
	return DIVV2;
}

int32_t ODivRegister::command() const
{
	return DIVR;
}

int32_t OCompareImmediate::command() const
{
	return COMPAREV;
}

int32_t OCompareImmediate2::command() const
{
	return COMPAREV2;
}

int32_t OCompareRegister::command() const
{
	return COMPARER;
}

int32_t OInternalStringCompare::command() const
{
	return STRCMPR;
}

int32_t OInternalInsensitiveStringCompare::command() const
{
	return STRICMPR;
}

int32_t OWaitframe::command() const
{
	return WAITFRAME;
}

int32_t OWaitframes::command() const
{
	return WAITFRAMESR;
}

int32_t OWaitdraw::command() const
{
	return WAITDRAW;
}

int32_t OWaitTo::command() const
{
	return WAITTO;
}

int32_t OWaitEvent::command() const
{
	return WAITEVENT;
}

int32_t ONoOp::command() const
{
	return NOP;
}

int32_t OCastBoolI::command() const
{
	return CASTBOOLI;
}

int32_t OCastBoolF::command() const
{
	return CASTBOOLF;
}

//I would like to add a Jump instruction tot he parser, which would be 'GOTOLABEL' -Z
int32_t OGotoImmediate::command() const
{
	return GOTO;
}

int32_t OGotoTrueImmediate::command() const
{
	return GOTOTRUE;
}

int32_t OGotoFalseImmediate::command() const
{
	return GOTOFALSE;
}

int32_t OGotoMoreImmediate::command() const
{
	return GOTOMORE;
}

int32_t OGotoLessImmediate::command() const
{
	return GOTOLESS;
}

int32_t OPushRegister::command() const
{
	return PUSHR;
}

int32_t OPushImmediate::command() const
{
	return PUSHV;
}

int32_t OPopRegister::command() const
{
	return POP;
}

int32_t OPopArgsRegister::command() const
{
	return POPARGS;
}

int32_t OPushArgsRegister::command() const
{
	return PUSHARGSR;
}

int32_t OPushArgsImmediate::command() const
{
	return PUSHARGSV;
}

int32_t OPushVargV::command() const
{
	return PUSHVARGV;
}

int32_t OPushVargR::command() const
{
	return PUSHVARGR;
}

int32_t OPushVargsV::command() const
{
	return PUSHVARGSV;
}

int32_t OPushVargsR::command() const
{
	return PUSHVARGSR;
}

int32_t OLoadIndirect::command() const
{
	return LOADI;
}

int32_t OStoreIndirect::command() const
{
	return STOREI;
}

int32_t OLoadDirect::command() const
{
	return LOADD;
}
int32_t OStoreDirect::command() const
{
	return STORED;
}
int32_t OStoreDirectV::command() const
{
	return STOREDV;
}

int32_t OQuit::command() const
{
	return QUIT;
}

int32_t OGotoRegister::command() const
{
	return GOTOR;
}

int32_t OAndImmediate::command() const
{
	return ANDV;
}

int32_t OAndRegister::command() const
{
	return ANDR;
}

int32_t OOrImmediate::command() const
{
	return ORV;
}

int32_t OOrRegister::command() const
{
	return ORR;
}

int32_t OXorImmediate::command() const
{
	return XORV;
}

int32_t OXorRegister::command() const
{
	return XORR;
}

int32_t O32BitAndImmediate::command() const
{
	return ANDV32;
}

int32_t O32BitAndRegister::command() const
{
	return ANDR32;
}

int32_t O32BitOrImmediate::command() const
{
	return ORV32;
}

int32_t O32BitOrRegister::command() const
{
	return ORR32;
}

int32_t O32BitXorImmediate::command() const
{
	return XORV32;
}

int32_t O32BitXorRegister::command() const
{
	return XORR32;
}

int32_t OSinRegister::command() const
{
	return SINR;
}

int32_t OCosRegister::command() const
{
	return COSR;
}

int32_t OTanRegister::command() const
{
	return TANR;
}

int32_t OEngineDegtoRad::command() const
{
	return DEGTORAD;
}

int32_t OEngineRadtoDeg::command() const
{
	return RADTODEG;
}

int32_t Ostrlen::command() const
{
	return STRINGLENGTH;
}

int32_t OATanRegister::command() const
{
	return ARCTANR;
}

int32_t OArcCosRegister::command() const
{
	return ARCCOSR;
}

int32_t OArcSinRegister::command() const
{
	return ARCSINR;
}

int32_t OMinRegister::command() const
{
	return MINR;
}

int32_t OMaxRegister::command() const
{
	return MAXR;
}
int32_t OMaxNew::command() const
{
	return MAXVARG;
}
int32_t OMinNew::command() const
{
	return MINVARG;
}
int32_t OChoose::command() const
{
	return CHOOSEVARG;
}

int32_t OPowRegister::command() const
{
	return POWERR;
}
int32_t OPowImmediate::command() const
{
	return POWERV;
}
int32_t OPowImmediate2::command() const
{
	return POWERV2;
}
int32_t OLPowRegister::command() const
{
	return LPOWERR;
}
int32_t OLPowImmediate::command() const
{
	return LPOWERV;
}
int32_t OLPowImmediate2::command() const
{
	return LPOWERV2;
}

int32_t OInvPowRegister::command() const
{
	return IPOWERR;
}

int32_t OFactorial::command() const
{
	return FACTORIAL;
}

int32_t OAbsRegister::command() const
{
	return ABS;
}

int32_t OLog10Register::command() const
{
	return LOG10;
}

int32_t OLogERegister::command() const
{
	return LOGE;
}

int32_t OArraySize::command() const
{
	return ARRAYSIZE;
}


int32_t OArraySizeF::command() const
{
	return ARRAYSIZEF;
}
int32_t OArraySizeN::command() const
{
	return ARRAYSIZEN;
}
int32_t OArraySizeE::command() const
{
	return ARRAYSIZEE;
}
int32_t OArraySizeL::command() const
{
	return ARRAYSIZEL;
}
int32_t OArraySizeB::command() const
{
	return ARRAYSIZEB;
}
int32_t OArraySizeI::command() const
{
	return ARRAYSIZEI;
}
int32_t OArraySizeID::command() const
{
	return ARRAYSIZEID;
}

int32_t OLShiftImmediate::command() const
{
	return LSHIFTV;
}

int32_t OLShiftRegister::command() const
{
	return LSHIFTR;
}

int32_t ORShiftImmediate::command() const
{
	return RSHIFTV;
}

int32_t ORShiftRegister::command() const
{
	return RSHIFTR;
}

int32_t O32BitLShiftImmediate::command() const
{
	return LSHIFTV32;
}

int32_t O32BitLShiftRegister::command() const
{
	return LSHIFTR32;
}

int32_t O32BitRShiftImmediate::command() const
{
	return RSHIFTV32;
}

int32_t O32BitRShiftRegister::command() const
{
	return RSHIFTR32;
}

int32_t OModuloImmediate::command() const
{
	return MODV;
}

int32_t OModuloImmediate2::command() const
{
	return MODV2;
}

int32_t OModuloRegister::command() const
{
	return MODR;
}

int32_t ONot::command() const
{
	return BITNOT;
}

int32_t O32BitNot::command() const
{
	return BITNOT32;
}

int32_t OTraceRegister::command() const
{
	return TRACER;
}
int32_t OTraceImmediate::command() const
{
	return TRACEV;
}

int32_t OTraceLRegister::command() const
{
	return TRACELR;
}

int32_t OTrace2Register::command() const
{
	return TRACE2R;
}

int32_t OTrace3::command() const
{
	return TRACE3;
}

int32_t OTrace4::command() const
{
	return TRACE4;
}

int32_t OTrace5Register::command() const
{
	return TRACE5;
}

int32_t OTrace6Register::command() const
{
	return TRACE6;
}

int32_t OPrintfImmediate::command() const
{
	return PRINTFV;
}

int32_t OSPrintfImmediate::command() const
{
	return SPRINTFV;
}

int32_t OPrintfVargs::command() const
{
	return PRINTFVARG;
}

int32_t OSPrintfVargs::command() const
{
	return SPRINTFVARG;
}

int32_t OBreakpoint::command() const
{
	return BREAKPOINT;
}


int32_t ORandRegister::command() const
{
	return RNDR;
}

int32_t OSRandRegister::command() const
{
	return SRNDR;
}

int32_t OSRandImmediate::command() const
{
	return SRNDV;
}

int32_t OSRandRand::command() const
{
	return SRNDRND;
}

int32_t ORNGRand1::command() const
{
	return RNGRAND1;
}

int32_t ORNGRand2::command() const
{
	return RNGRAND2;
}

int32_t ORNGRand3::command() const
{
	return RNGRAND3;
}

int32_t ORNGLRand1::command() const
{
	return RNGLRAND1;
}

int32_t ORNGLRand2::command() const
{
	return RNGLRAND2;
}

int32_t ORNGLRand3::command() const
{
	return RNGLRAND3;
}

int32_t ORNGSeed::command() const
{
	return RNGSEED;
}

int32_t ORNGRSeed::command() const
{
	return RNGRSEED;
}

int32_t ORNGFree::command() const
{
	return RNGFREE;
}

int32_t OCheckTrig::command() const
{
	return CHECKTRIG;
}

int32_t OWarp::command() const
{
	return WARPR;
}

int32_t OPitWarp::command() const
{
	return PITWARPR;
}

int32_t OSqrtRegister::command() const
{
	return SQROOTR;
}

int32_t OCreateItemRegister::command() const
{
	return CREATEITEMR;
}

int32_t OCreateNPCRegister::command() const
{
	return CREATENPCR;
}

int32_t OCreateLWpnRegister::command() const
{
	return CREATELWEAPONR;
}

int32_t OCreateEWpnRegister::command() const
{
	return CREATEEWEAPONR;
}

int32_t OLoadItemRegister::command() const
{
	return LOADITEMR;
}

int32_t OLoadItemDataRegister::command() const
{
	return LOADITEMDATAR;
}

//New Types

int32_t OLoadShopDataRegister::command() const
{
	return LOADSHOPR;
}
int32_t OLoadInfoShopDataRegister::command() const
{
	return LOADINFOSHOPR;
}
int32_t OLoadMessageDataRegister::command() const
{
	return LOADMESSAGEDATAR;
}

int32_t OLoadDMapDataRegister::command() const
{
	return LOADDMAPDATAR;
}

int32_t OLoadStack::command() const
{
	return LOADSTACK;
}

int32_t OLoadDirectoryRegister::command() const
{
	return LOADDIRECTORYR;
}

int32_t OLoadRNG::command() const
{
	return LOADRNG;
}

int32_t OCreatePalData::command() const
{
	return CREATEPALDATA;
}

int32_t OCreatePalDataClr::command() const
{
	return CREATEPALDATACLR;
}

int32_t OMixColorArray::command() const
{
	return MIXCLR;
}
int32_t OCreateRGBHex::command() const
{
	return CREATERGBHEX;
}
int32_t OCreateRGB::command() const
{
	return CREATERGB;
}
int32_t OConvertFromRGB::command() const
{
	return CONVERTFROMRGB;
}
int32_t OConvertToRGB::command() const
{
	return CONVERTTORGB;
}
int32_t OGetTilePixel::command() const
{
	return GETTILEPIXEL;
}
int32_t OSetTilePixel::command() const
{
	return SETTILEPIXEL;
}
int32_t OLoadLevelPalette::command() const
{
	return PALDATALOADLEVEL;
}

int32_t OLoadSpritePalette::command() const
{
	return PALDATALOADSPRITE;
}

int32_t OLoadMainPalette::command() const
{
	return PALDATALOADMAIN;
}

int32_t OLoadCyclePalette::command() const
{
	return PALDATALOADCYCLE;
}

int32_t OLoadBitmapPalette::command() const
{
	return PALDATALOADBITMAP;
}

int32_t OWriteLevelPalette::command() const
{
	return PALDATAWRITELEVEL;
}

int32_t OWriteLevelCSet::command() const
{
	return PALDATAWRITELEVELCS;
}

int32_t OWriteSpritePalette::command() const
{
	return PALDATAWRITESPRITE;
}

int32_t OWriteSpriteCSet::command() const
{
	return PALDATAWRITESPRITECS;
}

int32_t OWriteMainPalette::command() const
{
	return PALDATAWRITEMAIN;
}

int32_t OWriteMainCSet::command() const
{
	return PALDATAWRITEMAINCS;
}

int32_t OWriteCyclePalette::command() const
{
	return PALDATAWRITECYCLE;
}

int32_t OWriteCycleCSet::command() const
{
	return PALDATAWRITECYCLECS;
}

int32_t OPalDataColorValid::command() const
{
	return PALDATAVALIDCLR;
}

int32_t OPalDataClearColor::command() const
{
	return PALDATACLEARCLR;
}

int32_t OPalDataClearCSet::command() const
{
	return PALDATACLEARCSET;
}

int32_t OPalDataMix::command() const
{
	return PALDATAMIX;
}

int32_t OPalDataMixCSet::command() const
{
	return PALDATAMIXCS;
}

int32_t OPalDataCopy::command() const
{
	return PALDATACOPY;
}

int32_t OPalDataCopyCSet::command() const
{
	return PALDATACOPYCSET;
}

int32_t OPalDataFree::command() const
{
	return PALDATAFREE;
}

int32_t OPalDataOwn::command() const
{
	return PALDATAOWN;
}

int32_t OLoadDropsetRegister	::command() const
{
	return LOADDROPSETR;
}

int32_t OGetBottleShopName::command() const
{
	return BSHOPNAMEGET;
}

int32_t OSetBottleShopName::command() const
{
	return BSHOPNAMESET;
}

int32_t OGetBottleName::command() const
{
	return BOTTLENAMEGET;
}

int32_t OSetBottleName::command() const
{
	return BOTTLENAMESET;
}

int32_t OLoadBottleTypeRegister::command() const
{
	return LOADBOTTLETYPE;
}

int32_t OLoadBShopRegister::command() const
{
	return LOADBSHOPDATA;
}
int32_t OLoadGenericDataR::command() const
{
	return LOADGENERICDATA;
}

int32_t ODMapDataGetNameRegister::command() const
{
	return DMAPDATAGETNAMER;
}

int32_t ODMapDataSetNameRegister::command() const
{
	return DMAPDATASETNAMER;
}

int32_t ODMapDataGetTitleRegister::command() const
{
	return DMAPDATAGETTITLER;
}

int32_t ODMapDataSetTitleRegister::command() const
{
	return DMAPDATASETTITLER;
}

int32_t ODMapDataGetIntroRegister::command() const
{
	return DMAPDATAGETINTROR;
}

int32_t ODMapDataSetIntroRegister::command() const
{
	return DMAPDATANSETITROR;
}

int32_t ODMapDataGetMusicRegister::command() const
{
	return DMAPDATAGETMUSICR;
}

int32_t ODMapDataSetMusicRegister::command() const
{
	return DMAPDATASETMUSICR;
}

int32_t OMessageDataSetStringRegister::command() const
{
	return MESSAGEDATASETSTRINGR;
}
int32_t OMessageDataGetStringRegister::command() const
{
	return MESSAGEDATAGETSTRINGR;
}
int32_t OLoadNPCDataRegister::command() const
{
	return LOADNPCDATAR;
}
int32_t OLoadComboDataRegister::command() const
{
	return LOADCOMBODATAR;
}
int32_t OLoadMapDataRegister::command() const
{
	return LOADMAPDATAR;
}
int32_t OLoadSpriteDataRegister::command() const
{
	return LOADSPRITEDATAR;
}
int32_t OLoadScreenDataRegister::command() const
{
	return LOADSCREENDATAR;
}
int32_t OLoadBitmapDataRegister::command() const
{
	return LOADBITMAPDATAR;
}

int32_t OLoadNPCRegister::command() const
{
	return LOADNPCR;
}


int32_t OLoadLWpnRegister::command() const
{
	return LOADLWEAPONR;
}

int32_t OLoadEWpnRegister::command() const
{
	return LOADEWEAPONR;
}
int32_t OAdjustVolumeRegister::command() const
{
	return ADJUSTVOLUMER;
}
int32_t OAdjustSFXVolumeRegister::command() const
{
	return ADJUSTSFXVOLUMER;
}
int32_t OAdjustSound::command() const
{
	return ADJUSTSFX;
}
int32_t OPlaySoundEX::command() const
{
	return PLAYSOUNDEX;
}
int32_t OGetSoundCompletion::command() const
{
	return GETSFXCOMPLETION;
}
int32_t OPlaySoundRegister::command() const
{
	return PLAYSOUNDR;
}

int32_t OPlayMIDIRegister::command() const
{
	return PLAYMIDIR;
}

int32_t OPlayEnhancedMusic::command() const
{
	return PLAYENHMUSIC;
}

int32_t OPlayEnhancedMusicEx::command() const
{
	return PLAYENHMUSICEX;
}

int32_t OGetEnhancedMusicPos::command() const
{
	return GETENHMUSICPOS;
}

int32_t OSetEnhancedMusicPos::command() const
{
	return SETENHMUSICPOS;
}

int32_t OSetEnhancedMusicSpeed::command() const
{
	return SETENHMUSICSPEED;
}

int32_t OGetEnhancedMusicLength::command() const
{
	return GETENHMUSICLEN;
}

int32_t OSetEnhancedMusicLoop::command() const
{
	return SETENHMUSICLOOP;
}

int32_t OCrossfadeEnhancedMusic::command() const
{
	return ENHCROSSFADE;
}

int32_t OGetDMapMusicFilename::command() const
{
	return GETMUSICFILE;
}

int32_t OGetNPCDataInitDLabel::command() const
{
	return NPCGETINITDLABEL;
}

int32_t OGetDMapMusicTrack::command() const
{
	return GETMUSICTRACK;
}

// Audio->
int32_t OEndSoundRegister::command() const
{
	return ENDSOUNDR;
}


int32_t OContinueSFX::command() const
{
	return CONTINUESFX;
}

int32_t OPauseSoundRegister::command() const
{
	return PAUSESOUNDR;
}

int32_t OPauseMusic::command() const
{
	return PAUSEMUSIC;
}
int32_t OResumeMusic::command() const
{
	return RESUMEMUSIC;
}

int32_t OResumeSoundRegister::command() const
{
	return RESUMESOUNDR;
}

//END Audio

int32_t OSetDMapEnhancedMusic::command() const
{
	return SETDMAPENHMUSIC;
}

int32_t OGetSaveName::command() const
{
	return GETSAVENAME;
}

int32_t OSetSaveName::command() const
{
	return SETSAVENAME;
}

int32_t OGetDMapName::command() const
{
	return GETDMAPNAME;
}

int32_t OGetDMapTitle::command() const
{
	return GETDMAPTITLE;
}

int32_t OGetDMapIntro::command() const
{
	return GETDMAPINTRO;
}


int32_t OSetDMapName::command() const
{
	return SETDMAPNAME;
}

int32_t OSetDMapTitle::command() const
{
	return SETDMAPTITLE;
}

int32_t OSetDMapIntro::command() const
{
	return SETDMAPINTRO;
}

int32_t OGetItemName::command() const
{
	return ITEMNAME;
}

int32_t OGetNPCName::command() const
{
	return NPCNAME;
}

int32_t OGetMessage::command() const
{
	return GETMESSAGE;
}


int32_t OSetMessage::command() const
{
	return SETMESSAGE;
}

int32_t OClearSpritesRegister::command() const
{
	return CLEARSPRITESR;
}

int32_t ORectangleRegister::command() const
{
	return RECTR;
}

int32_t OFrameRegister::command() const
{
	return FRAMER;
}

int32_t OCircleRegister::command() const
{
	return CIRCLER;
}

int32_t OArcRegister::command() const
{
	return ARCR;
}

int32_t OEllipseRegister::command() const
{
	return ELLIPSER;
}

int32_t OLineRegister::command() const
{
	return LINER;
}

int32_t OSplineRegister::command() const
{
	return SPLINER;
}

int32_t OPutPixelRegister::command() const
{
	return PUTPIXELR;
}

int32_t OPutPixelArrayRegister::command() const
{
	return PIXELARRAYR;
}

int32_t OPutTileArrayRegister::command() const
{
	return TILEARRAYR;
}

int32_t OPutLinesArrayRegister::command() const
{
	return LINESARRAY;
}

int32_t ODrawCharRegister::command() const
{
	return DRAWCHARR;
}

int32_t ODrawIntRegister::command() const
{
	return DRAWINTR;
}

int32_t ODrawTileRegister::command() const
{
	return DRAWTILER;
}

int32_t ODrawTileCloakedRegister::command() const
{
	return DRAWTILECLOAKEDR;
}

int32_t ODrawComboRegister::command() const
{
	return DRAWCOMBOR;
}

int32_t ODrawComboCloakedRegister::command() const
{
	return DRAWCOMBOCLOAKEDR;
}

int32_t OFastTileRegister::command() const
{
	return FASTTILER;
}

int32_t OFastComboRegister::command() const
{
	return FASTCOMBOR;
}

int32_t OFastComboArrayRegister::command() const
{
	return COMBOARRAYR;
}

int32_t ODrawStringRegister::command() const
{
	return DRAWSTRINGR;
}

int32_t ODrawString2Register::command() const
{
	return DRAWSTRINGR2;
}

int32_t ODrawBitmapRegister::command() const
{
	return BITMAPR;
}

int32_t ODrawBitmapExRegister::command() const
{
	return BITMAPEXR;
}

int32_t OSetRenderTargetRegister::command() const
{
	return SETRENDERTARGET;
}

int32_t OSetDepthBufferRegister::command() const
{
	return SETDEPTHB;
}

int32_t OGetDepthBufferRegister::command() const
{
	return GETDEPTHB;
}

int32_t OSetColorBufferRegister::command() const
{
	return SETCOLORB;
}

int32_t OGetColorBufferRegister::command() const
{
	return GETCOLORB;
}

int32_t OQuadRegister::command() const
{
	return QUADR;
}

int32_t OTriangleRegister::command() const
{
	return TRIANGLER;
}


int32_t OQuad3DRegister::command() const
{
	return QUAD3DR;
}

int32_t OTriangle3DRegister::command() const
{
	return TRIANGLE3DR;
}

int32_t ODrawLayerRegister::command() const
{
	return DRAWLAYERR;
}

int32_t ODrawScreenRegister::command() const
{
	return DRAWSCREENR;
}

int32_t OMessageRegister::command() const
{
	return MSGSTRR;
}

int32_t OIsSolid::command() const
{
	return ISSOLID;
}

int32_t OIsSolidMapdata::command() const
{
	return MAPDATAISSOLID;
}

int32_t OIsSolidMapdataLayer::command() const
{
	return MAPDATAISSOLIDLYR;
}

int32_t OIsSolidLayer::command() const
{
	return ISSOLIDLAYER;
}

int32_t OLoadTmpScr::command() const
{
	return LOADTMPSCR;
}

int32_t OLoadScrollScr::command() const
{
	return LOADSCROLLSCR;
}

int32_t OSetSideWarpRegister::command() const
{
	return SETSIDEWARP;
}

int32_t OGetSideWarpDMap::command() const
{
	return GETSIDEWARPDMAP;
}

int32_t OGetSideWarpScreen::command() const
{
	return GETSIDEWARPSCR;
}

int32_t OGetSideWarpType::command() const
{
	return GETSIDEWARPTYPE;
}

int32_t OGetTileWarpDMap::command() const
{
	return GETTILEWARPDMAP;
}

int32_t OGetTileWarpScreen::command() const
{
	return GETTILEWARPSCR;
}

int32_t OGetTileWarpType::command() const
{
	return GETTILEWARPTYPE;
}

int32_t OSetTileWarpRegister::command() const
{
	return SETTILEWARP;
}

int32_t OLayerScreenRegister::command() const
{
	return LAYERSCREEN;
}

int32_t OLayerMapRegister::command() const
{
	return LAYERMAP;
}

int32_t OTriggerSecrets::command() const
{
	return SECRETS;
}

int32_t OIsValidArray::command() const
{
	return ISVALIDARRAY;
}

int32_t OIsValidItem::command() const
{
	return ISVALIDITEM;
}

int32_t OIsValidNPC::command() const
{
	return ISVALIDNPC;
}

int32_t OCopyTileRegister::command() const
{
	return COPYTILERR;
}

int32_t Ostrcpy::command() const
{
	return STRINGCOPY;
}

int32_t OOverlayTileRegister::command() const
{
	return OVERLAYTILERR;
}

int32_t OSwapTileRegister::command() const
{
	return SWAPTILERR;
}

int32_t OClearTileRegister::command() const
{
	return CLEARTILER;
}

int32_t OIsValidLWpn::command() const
{
	return ISVALIDLWPN;
}

int32_t OIsValidEWpn::command() const
{
	return ISVALIDEWPN;
}

int32_t OMakeAngularLwpn::command() const
{
	return LWPNMAKEANGULAR;
}

int32_t OMakeAngularEwpn::command() const
{
	return EWPNMAKEANGULAR;
}

int32_t OMakeDirectionalLwpn::command() const
{
	return LWPNMAKEDIRECTIONAL;
}

int32_t OMakeDirectionalEwpn::command() const
{
	return EWPNMAKEDIRECTIONAL;
}

int32_t OUseSpriteLWpn::command() const
{
	return LWPNUSESPRITER;
}

int32_t OUseSpriteEWpn::command() const
{
	return EWPNUSESPRITER;
}

int32_t OAllocateMemRegister::command() const
{
	return ALLOCATEMEMR;
}

int32_t OAllocateMemImmediate::command() const
{
	return ALLOCATEMEMV;
}

int32_t OAllocateGlobalMemImmediate::command() const
{
	return ALLOCATEGMEMV;
}

int32_t OAllocateGlobalMemRegister::command() const
{
	return ALLOCATEGMEMR;
}

int32_t ODeallocateMemRegister::command() const
{
	return DEALLOCATEMEMR;
}

int32_t ODeallocateMemImmediate::command() const
{
	return DEALLOCATEMEMV;
}

int32_t OResizeArrayRegister::command() const
{
	return RESIZEARRAYR;
}
int32_t OOwnArrayRegister::command() const
{
	return OWNARRAYR;
}
int32_t ODestroyArrayRegister::command() const
{
	return DESTROYARRAYR;
}

int32_t OSave::command() const
{
	return SAVE;
}

int32_t OGetScreenFlags::command() const
{
	return GETSCREENFLAGS;
}

int32_t OGetScreenEFlags::command() const
{
	return GETSCREENEFLAGS;
}

int32_t OEnd::command() const
{
	return GAMEEND;
}

int32_t OGameReload::command() const
{
	return GAMERELOAD;
}

int32_t OGameContinue::command() const
{
	return GAMECONTINUE;
}

int32_t OGameSaveQuit::command() const
{
	return GAMESAVEQUIT;
}

int32_t OGameSaveContinue::command() const
{
	return GAMESAVECONTINUE;
}

int32_t OShowF6Screen::command() const
{
	return SHOWF6SCREEN;
}

int32_t OComboTile::command() const
{
	return COMBOTILE;
}

int32_t OBreakShield::command() const
{
	return BREAKSHIELD;
}

int32_t OShowSaveScreen::command() const
{
	return SAVESCREEN;
}

int32_t OShowSaveQuitScreen::command() const
{
	return SAVEQUITSCREEN;
}

int32_t OSelectAWeaponRegister::command() const
{
	return SELECTAWPNR;
}

int32_t OSelectBWeaponRegister::command() const
{
	return SELECTBWPNR;
}

int32_t OSelectXWeaponRegister::command() const
{
	return SELECTXWPNR;
}

int32_t OSelectYWeaponRegister::command() const
{
	return SELECTYWPNR;
}

int32_t OGetFFCScript::command() const
{
	return GETFFCSCRIPT;
}

int32_t OGetComboScript::command() const
{
	return GETCOMBOSCRIPT;
}

//2.54

int32_t OGreyscaleOn::command() const
{
	return GREYSCALEON;
}

int32_t OGreyscaleOff::command() const
{
	return GREYSCALEOFF;
}

int32_t OZapIn::command() const
{
	return ZAPIN;
}

int32_t OZapOut::command() const
{
	return ZAPOUT;
}

//These need to be unary opcodes that accept bool linkvisible. 
int32_t OWavyIn::command() const
{
	return WAVYIN;
}

int32_t OWavyOut::command() const
{
	return WAVYOUT;
}

int32_t OOpenWipe::command() const
{
	return OPENWIPE;
}

int32_t OCloseWipe::command() const
{
	return CLOSEWIPE;
}

int32_t OOpenWipeShape::command() const
{
	return OPENWIPESHAPE;
}

int32_t OCloseWipeShape::command() const
{
	return CLOSEWIPESHAPE;
}

//Game->GetItemScript(int32_t ptr[])
int32_t OGetItemScript::command() const
{
	return GETITEMSCRIPT;
}


int32_t OGetLWeaponPointer::command() const
{
	return LWPNARRPTR;
}

int32_t OSetLWeaponPointer::command() const
{
	return LWPNARRPTR2;
}

int32_t OGetEWeaponPointer::command() const
{
	return EWPNARRPTR;
}

int32_t OSetEWeaponPointer::command() const
{
	return EWPNARRPTR2;
}

int32_t OGetItemPointer::command() const
{
	return ITEMARRPTR;
}

int32_t OSetItemPointer::command() const
{
	return ITEMARRPTR2;
}

int32_t OGetItemDataPointer::command() const
{
	return IDATAARRPTR;
}

int32_t OSetItemDataPointer::command() const
{
	return IDATAARRPTR2;
}

int32_t OGetFFCPointer::command() const
{
	return FFCARRPTR;
}

int32_t OSetFFCPointer::command() const
{
	return FFCARRPTR2;
}

int32_t OGetBoolPointer::command() const
{
	return BOOLARRPTR;
}

int32_t OSetBoolPointer::command() const
{
	return BOOLARRPTR2;
}

int32_t OGetNPCPointer::command() const
{
	return NPCARRPTR;
}

int32_t OSetNPCPointer::command() const
{
	return NPCARRPTR2;
}


int32_t OGetScreenDoor::command() const
{
	return GETSCREENDOOR;
}

int32_t OGetScreenEnemy::command() const
{
	return GETSCREENENEMY;
}


int32_t OGetScreenLayerOpacity::command() const
{
	return GETSCREENLAYOP;
}

int32_t OGetScreenSecretCombo::command() const
{
	return GETSCREENSECCMB;
}

int32_t OGetScreenSecretCSet::command() const
{
	return GETSCREENSECCST;
}

int32_t OGetScreenSecretFlag::command() const
{
	return GETSCREENSECFLG;
}

int32_t OGetScreenLayerMap::command() const
{
	return GETSCREENLAYMAP;
}

int32_t OGetScreenLayerScreen::command() const
{
	return GETSCREENLAYSCR;
}

int32_t OGetScreenPath::command() const
{
	return GETSCREENPATH;
}

int32_t OGetScreenWarpReturnX::command() const
{
	return GETSCREENWARPRX;
}

int32_t OGetScreenWarpReturnY::command() const
{
	return GETSCREENWARPRY;
}

int32_t OTriggerSecretRegister::command() const
{
	return TRIGGERSECRETR;
}

int32_t OBMPPolygonRegister::command() const
{
	return BMPPOLYGONR;
}

int32_t OPolygonRegister::command() const
{
	return POLYGONR;
}


//NPCData

int32_t ONDataBaseTile::command() const
{
	return GETNPCDATATILE;
}
int32_t ONDataEHeight::command() const
{
	return GETNPCDATAEHEIGHT;
}

int32_t ONDataFlags::command() const
{
	return GETNPCDATAFLAGS;
}
int32_t ONDataFlags2::command() const
{
	return GETNPCDATAFLAGS2;
}
int32_t ONDataWidth::command() const
{
	return GETNPCDATAWIDTH;
}
int32_t ONDataHeight::command() const
{
	return GETNPCDATAHEIGHT;
}
int32_t ONDataTile::command() const
{
	return GETNPCDATASTILE;
}
int32_t ONDataSWidth::command() const
{
	return GETNPCDATASWIDTH;
}
int32_t ONDataSHeight::command() const
{
	return GETNPCDATASHEIGHT;
}
int32_t ONDataETile::command() const
{
	return GETNPCDATAETILE;
}
int32_t ONDataEWidth::command() const
{
	return GETNPCDATAEWIDTH;
}
int32_t ONDataHP::command() const
{
	return GETNPCDATAHP;
}
int32_t ONDataFamily::command() const
{
	return GETNPCDATAFAMILY;
}
int32_t ONDataCSet::command() const
{
	return GETNPCDATACSET;
}
int32_t ONDataAnim::command() const
{
	return GETNPCDATAANIM;
}
int32_t ONDataEAnim::command() const
{
	return GETNPCDATAEANIM;
}
int32_t ONDataFramerate::command() const
{
	return GETNPCDATAFRAMERATE;
}
int32_t ONDataEFramerate::command() const
{
	return GETNPCDATAEFRAMERATE;
}
int32_t ONDataTouchDamage::command() const
{
	return GETNPCDATATOUCHDMG;
}
int32_t ONDataWeaponDamage::command() const
{
	return GETNPCDATAWPNDAMAGE;
}
int32_t ONDataWeapon::command() const
{
	return GETNPCDATAWEAPON;
}
int32_t ONDataRandom::command() const
{
	return GETNPCDATARANDOM;
}
int32_t ONDataHalt::command() const
{
	return GETNPCDATAHALT;
}
int32_t ONDataStep::command() const
{
	return GETNPCDATASTEP;
}
int32_t ONDataHoming::command() const
{
	return GETNPCDATAHOMING;
}
int32_t ONDataHunger::command() const
{
	return GETNPCDATAHUNGER;
}
int32_t ONDataropset::command() const
{
	return GETNPCDATADROPSET;
}
int32_t ONDataBGSound::command() const
{
	return GETNPCDATABGSFX;
}
int32_t ONDataHitSound::command() const
{
	return GETNPCDATAHITSFX;
}
int32_t ONDataDeathSound::command() const
{
	return GETNPCDATADEATHSFX;
}
int32_t ONDataXofs::command() const
{
	return GETNPCDATAXOFS;
}
int32_t ONDataYofs::command() const
{
	return GETNPCDATAYOFS;
}
int32_t ONDataZofs::command() const
{
	return GETNPCDATAZOFS;
}
int32_t ONDataHitXOfs::command() const
{
	return GETNPCDATAHXOFS;
}
int32_t ONDataHYOfs::command() const
{
	return GETNPCDATAHYOFS;
}
int32_t ONDataHitWidth::command() const
{
	return GETNPCDATAHITWIDTH;
}
int32_t ONDataHitHeight::command() const
{
	return GETNPCDATAHITHEIGHT;
}
int32_t ONDataHitZ::command() const
{
	return GETNPCDATAHITZ;
}
int32_t ONDataTileWidth::command() const
{
	return GETNPCDATATILEWIDTH;
}
int32_t ONDataTileHeight::command() const
{
	return GETNPCDATATILEHEIGHT;
}
int32_t ONDataWeapSprite::command() const
{
	return GETNPCDATAWPNSPRITE;
}

//two inputs, one return
int32_t ONDataScriptDef::command() const
{
	return GETNPCDATASCRIPTDEF;
}
//two inputs, one return
int32_t ONDataDefense::command() const
{
	return GETNPCDATADEFENSE;
}
//two inputs, one return
int32_t ONDataSizeFlag::command() const
{
	return GETNPCDATASIZEFLAG;
}
//two inputs, one return
int32_t ONDatattributes::command() const
{
	return GETNPCDATAATTRIBUTE;
}

int32_t ONDataSetBaseTile::command() const
{
	return SETNPCDATATILE;
}
int32_t ONDataSetEHeight::command() const
{
	return SETNPCDATAEHEIGHT;
}

int32_t ONDataSetFlags::command() const
{
	return SETNPCDATAFLAGS;
}
int32_t ONDataSetFlags2::command() const
{
	return SETNPCDATAFLAGS2;
}
int32_t ONDataSetWidth::command() const
{
	return SETNPCDATAWIDTH;
}
int32_t ONDataSetHeight::command() const
{
	return SETNPCDATAHEIGHT;
}
int32_t ONDataSetTile::command() const
{
	return SETNPCDATASTILE;
}
int32_t ONDataSetSWidth::command() const
{
	return SETNPCDATASWIDTH;
}
int32_t ONDataSetSHeight::command() const
{
	return SETNPCDATASHEIGHT;
}
int32_t ONDataSetETile::command() const
{
	return SETNPCDATAETILE;
}
int32_t ONDataSetEWidth::command() const
{
	return SETNPCDATAEWIDTH;
}
int32_t ONDataSetHP::command() const
{
	return SETNPCDATAHP;
}
int32_t ONDataSetFamily::command() const
{
	return SETNPCDATAFAMILY;
}
int32_t ONDataSetCSet::command() const
{
	return SETNPCDATACSET;
}
int32_t ONDataSetAnim::command() const
{
	return SETNPCDATAANIM;
}
int32_t ONDataSetEAnim::command() const
{
	return SETNPCDATAEANIM;
}
int32_t ONDataSetFramerate::command() const
{
	return SETNPCDATAFRAMERATE;
}
int32_t ONDataSetEFramerate::command() const
{
	return SETNPCDATAEFRAMERATE;
}
int32_t ONDataSetTouchDamage::command() const
{
	return SETNPCDATATOUCHDMG;
}
int32_t ONDataSetWeaponDamage::command() const
{
	return SETNPCDATAWPNDAMAGE;
}
int32_t ONDataSetWeapon::command() const
{
	return SETNPCDATAWEAPON;
}
int32_t ONDataSetRandom::command() const
{
	return SETNPCDATARANDOM;
}
int32_t ONDataSetHalt::command() const
{
	return SETNPCDATAHALT;
}
int32_t ONDataSetStep::command() const
{
	return SETNPCDATASTEP;
}
int32_t ONDataSetHoming::command() const
{
	return SETNPCDATAHOMING;
}
int32_t ONDataSetHunger::command() const
{
	return SETNPCDATAHUNGER;
}
int32_t ONDataSetropset::command() const
{
	return SETNPCDATADROPSET;
}
int32_t ONDataSetBGSound::command() const
{
	return SETNPCDATABGSFX;
}
int32_t ONDataSetHitSound::command() const
{
	return SETNPCDATAHITSFX;
}
int32_t ONDataSetDeathSound::command() const
{
	return SETNPCDATADEATHSFX;
}
int32_t ONDataSetXofs::command() const
{
	return SETNPCDATAXOFS;
}
int32_t ONDataSetYofs::command() const
{
	return SETNPCDATAYOFS;
}
int32_t ONDataSetZofs::command() const
{
	return SETNPCDATAZOFS;
}
int32_t ONDataSetHitXOfs::command() const
{
	return SETNPCDATAHXOFS;
}
int32_t ONDataSetHYOfs::command() const
{
	return SETNPCDATAHYOFS;
}
int32_t ONDataSetHitWidth::command() const
{
	return SETNPCDATAHITWIDTH;
}
int32_t ONDataSetHitHeight::command() const
{
	return SETNPCDATAHITHEIGHT;
}
int32_t ONDataSetHitZ::command() const
{
	return SETNPCDATAHITZ;
}
int32_t ONDataSetTileWidth::command() const
{
	return SETNPCDATATILEWIDTH;
}
int32_t ONDataSetTileHeight::command() const
{
	return SETNPCDATATILEHEIGHT;
}
int32_t ONDataSetWeapSprite::command() const
{
	return SETNPCDATAWPNSPRITE;
}

//ComboData

int32_t OCDataBlockEnemy::command() const
{
	return GCDBLOCKENEM;
}
int32_t OCDataBlockHole::command() const
{
	return GCDBLOCKHOLE;
}
int32_t OCDataBlockTrig::command() const
{
	return GCDBLOCKTRIG;
}
int32_t OCDataConveyX::command() const
{
	return GCDCONVEYSPDX;
}
int32_t OCDataConveyY::command() const
{
	return GCDCONVEYSPDY;
}
int32_t OCDataCreateNPC::command() const
{
	return GCDCREATEENEM;
}
int32_t OCDataCreateEnemW::command() const
{
	return GCDCREATEENEMWH;
}
int32_t OCDataCreateEnemC::command() const
{
	return GCDCREATEENEMCH;
}
int32_t OCDataDirch::command() const
{
	return GCDDIRCHTYPE;
}
int32_t OCDataDistTiles::command() const
{
	return GCDDISTCHTILES;
}
int32_t OCDataDiveItem::command() const
{
	return GCDDIVEITEM;
}
int32_t OCDataDock::command() const
{
	return GCDDOCK;
}
int32_t OCDataFairy::command() const
{
	return GCDFAIRY;
}
int32_t OCDataAttrib::command() const
{
	return GCDFFCOMBOATTRIB;
}
int32_t OCDataDecoTile::command() const
{
	return GCDFOOTDECOTILE;
}

int32_t OCDataLadderPass::command() const
{
	return GCDLADDERPASS;
}
int32_t OCSetDataLadderPass::command() const
{
	return SCDLADDERPASS;
}


int32_t OCDataDecoType::command() const
{
	return GCDFOOTDECOTYPE;
}
int32_t OCDataHookshotGrab::command() const
{
	return GCDHOOKSHOTGRAB;
}
int32_t OCDataLockBlock::command() const
{
	return GCDLOCKBLOCKTYPE;
}
int32_t OCDataLockBlockChange::command() const
{
	return GCDLOCKBLOCKCHANGE;
}
int32_t OCDataMagicMirror::command() const
{
	return GCDMAGICMIRRORTYPE;
}
int32_t OCDataModHP::command() const
{
	return GCDMODIFYHPAMOUNT;
}
int32_t OCDataModHPDelay::command() const
{
	return GCDMODIFYHPDELAY;
}
int32_t OCDataModHpType::command() const
{
	return GCDMODIFYHPTYPE;
}
int32_t OCDataModMP::command() const
{
	return GCDMODIFYMPAMOUNT;
}
int32_t OCDataMpdMPDelay::command() const
{
	return GCDMODIFYMPDELAY;
}
int32_t OCDataModMPType::command() const
{
	return GCDMODIFYMPTYPE;
}
int32_t OCDataNoPush::command() const
{
	return GCDNOPUSHBLOCKS;
}
int32_t OCDataOverhead::command() const
{
	return GCDOVERHEAD;
}
int32_t OCDataEnemyLoc::command() const
{
	return GCDPLACEENEMY;
}
int32_t OCDataPushDir::command() const
{
	return GCDPUSHDIR;
}
int32_t OCDataPushWeight::command() const
{
	return GCDPUSHWEIGHT;
}
int32_t OCDataPushWait::command() const
{
	return GCDPUSHWAIT;
}
int32_t OCDataPushed::command() const
{
	return GCDPUSHED;
}
int32_t OCDataRaft::command() const
{
	return GCDRAFT;
}
int32_t OCDataResetRoom::command() const
{
	return GCDRESETROOM;
}
int32_t OCDataSavePoint::command() const
{
	return GCDSAVEPOINT;
}
int32_t OCDataFreeezeScreen::command() const
{
	return GCDSCREENFREEZE;
}
int32_t OCDataSecretCombo::command() const
{
	return GCDSECRETCOMBO;
}
int32_t OCDataSingular::command() const
{
	return GCDSINGULAR;
}
int32_t OCDataSlowMove::command() const
{
	return GCDSLOWMOVE;
}
int32_t OCDataStatue::command() const
{
	return GCDSTATUE;
}
int32_t OCDataStepType::command() const
{
	return GCDSTEPTYPE;
}
int32_t OCDataSteoChange::command() const
{
	return GCDSTEPCHANGETO;
}
int32_t OCDataStrikeRem::command() const
{
	return GCDSTRIKEREMNANTS;
}
int32_t OCDataStrikeRemType::command() const
{
	return GCDSTRIKEREMNANTSTYPE;
}
int32_t OCDataStrikeChange::command() const
{
	return GCDSTRIKECHANGE;
}
int32_t OCDataStrikeChangeItem::command() const
{
	return GCDSTRIKECHANGEITEM;
}
int32_t OCDataTouchItem::command() const
{
	return GCDTOUCHITEM;
}
int32_t OCDataTouchStairs::command() const
{
	return GCDTOUCHSTAIRS;
}
int32_t OCDataTriggerType::command() const
{
	return GCDTRIGGERTYPE;
}
int32_t OCDataTriggerSens::command() const
{
	return GCDTRIGGERSENS;
}
int32_t OCDataWarpType::command() const
{
	return GCDWARPTYPE;
}
int32_t OCDataWarpSens::command() const
{
	return GCDWARPSENS;
}
int32_t OCDataWarpDirect::command() const
{
	return GCDWARPDIRECT;
}
int32_t OCDataWarpLoc::command() const
{
	return GCDWARPLOCATION;
}
int32_t OCDataWater::command() const
{
	return GCDWATER;
}

int32_t OCDataWinGame::command() const
{
	return GCDWINGAME;
}
int32_t OCDataWhistle::command() const
{
	return GCDWHISTLE;
}
int32_t OCDataWeapBlockLevel::command() const
{
	return GCDBLOCKWEAPLVL;
}
int32_t OCDataTile::command() const
{
	return GCDTILE;
}
int32_t OCDataFlip::command() const
{
	return GCDFLIP;
}
int32_t OCDataWalkability::command() const
{
	return GCDWALK;
}
int32_t OCDataType::command() const
{
	return GCDTYPE;
}
int32_t OCDataCSets::command() const
{
	return GCDCSETS;
}
int32_t OCDataFoo::command() const
{
	return GCDFOO;
}
int32_t OCDataFrames::command() const
{
	return GCDFRAMES;
}
int32_t OCDataSpeed::command() const
{
	return GCDSPEED;
}
int32_t OCDataNext::command() const
{
	return GCDNEXTCOMBO;
}
int32_t OCDataNextCSet::command() const
{
	return GCDNEXTCSET;
}
int32_t OCDataFlag::command() const
{
	return GCDFLAG;
}
int32_t OCDataSkipAnim::command() const
{
	return GCDSKIPANIM;
}
int32_t OCDataTimer::command() const
{
	return GCDNEXTTIMER;
}
int32_t OCDataAnimY::command() const
{
	return GCDSKIPANIMY;
}
int32_t OCDataAnimFlags::command() const
{
	return GCDANIMFLAGS;
}

int32_t OCDataBlockWeapon::command() const
{
	return GCDBLOCKWEAPON;
}
int32_t OCDataExpansion::command() const
{
	return GCDEXPANSION;
}
int32_t OCDataStrikeWeapon::command() const
{
	return GCDSTRIKEWEAPONS;
}

int32_t OCSetDataBlockEnemy::command() const
{
	return SCDBLOCKENEM;
}
int32_t OCSetDataBlockHole::command() const
{
	return SCDBLOCKHOLE;
}
int32_t OCSetDataBlockTrig::command() const
{
	return SCDBLOCKTRIG;
}
int32_t OCSetDataConveyX::command() const
{
	return SCDCONVEYSPDX;
}
int32_t OCSetDataConveyY::command() const
{
	return SCDCONVEYSPDY;
}
int32_t OCSetDataCreateNPC::command() const
{
	return SCDCREATEENEM;
}
int32_t OCSetDataCreateEnemW::command() const
{
	return SCDCREATEENEMWH;
}
int32_t OCSetDataCreateEnemC::command() const
{
	return SCDCREATEENEMCH;
}
int32_t OCSetDataDirch::command() const
{
	return SCDDIRCHTYPE;
}
int32_t OCSetDataDistTiles::command() const
{
	return SCDDISTCHTILES;
}
int32_t OCSetDataDiveItem::command() const
{
	return SCDDIVEITEM;
}
int32_t OCSetDataDock::command() const
{
	return SCDDOCK;
}
int32_t OCSetDataFairy::command() const
{
	return SCDFAIRY;
}
int32_t OCSetDataAttrib::command() const
{
	return SCDFFCOMBOATTRIB;
}
int32_t OCSetDataDecoTile::command() const
{
	return SCDFOOTDECOTILE;
}
int32_t OCSetDataDecoType::command() const
{
	return SCDFOOTDECOTYPE;
}
int32_t OCSetDataHookshotGrab::command() const
{
	return SCDHOOKSHOTGRAB;
}
int32_t OCSetDataLockBlock::command() const
{
	return SCDLOCKBLOCKTYPE;
}
int32_t OCSetDataLockBlockChange::command() const
{
	return SCDLOCKBLOCKCHANGE;
}
int32_t OCSetDataMagicMirror::command() const
{
	return SCDMAGICMIRRORTYPE;
}
int32_t OCSetDataModHP::command() const
{
	return SCDMODIFYHPAMOUNT;
}
int32_t OCSetDataModHPDelay::command() const
{
	return SCDMODIFYHPDELAY;
}
int32_t OCSetDataModHpType::command() const
{
	return SCDMODIFYHPTYPE;
}
int32_t OCSetDataModMP::command() const
{
	return SCDMODIFYMPAMOUNT;
}
int32_t OCSetDataMpdMPDelay::command() const
{
	return SCDMODIFYMPDELAY;
}
int32_t OCSetDataModMPType::command() const
{
	return SCDMODIFYMPTYPE;
}
int32_t OCSetDataNoPush::command() const
{
	return SCDNOPUSHBLOCKS;
}
int32_t OCSetDataOverhead::command() const
{
	return SCDOVERHEAD;
}
int32_t OCSetDataEnemyLoc::command() const
{
	return SCDPLACEENEMY;
}
int32_t OCSetDataPushDir::command() const
{
	return SCDPUSHDIR;
}
int32_t OCSetDataPushWeight::command() const
{
	return SCDPUSHWEIGHT;
}
int32_t OCSetDataPushWait::command() const
{
	return SCDPUSHWAIT;
}
int32_t OCSetDataPushed::command() const
{
	return SCDPUSHED;
}
int32_t OCSetDataRaft::command() const
{
	return SCDRAFT;
}
int32_t OCSetDataResetRoom::command() const
{
	return SCDRESETROOM;
}
int32_t OCSetDataSavePoint::command() const
{
	return SCDSAVEPOINT;
}
int32_t OCSetDataFreeezeScreen::command() const
{
	return SCDSCREENFREEZE;
}
int32_t OCSetDataSecretCombo::command() const
{
	return SCDSECRETCOMBO;
}
int32_t OCSetDataSingular::command() const
{
	return SCDSINGULAR;
}
int32_t OCSetDataSlowMove::command() const
{
	return SCDSLOWMOVE;
}
int32_t OCSetDataStatue::command() const
{
	return SCDSTATUE;
}
int32_t OCSetDataStepType::command() const
{
	return SCDSTEPTYPE;
}
int32_t OCSetDataSteoChange::command() const
{
	return SCDSTEPCHANGETO;
}
int32_t OCSetDataStrikeRem::command() const
{
	return SCDSTRIKEREMNANTS;
}
int32_t OCSetDataStrikeRemType::command() const
{
	return SCDSTRIKEREMNANTSTYPE;
}
int32_t OCSetDataStrikeChange::command() const
{
	return SCDSTRIKECHANGE;
}
int32_t OCSetDataStrikeChangeItem::command() const
{
	return SCDSTRIKECHANGEITEM;
}
int32_t OCSetDataTouchItem::command() const
{
	return SCDTOUCHITEM;
}
int32_t OCSetDataTouchStairs::command() const
{
	return SCDTOUCHSTAIRS;
}
int32_t OCSetDataTriggerType::command() const
{
	return SCDTRIGGERTYPE;
}
int32_t OCSetDataTriggerSens::command() const
{
	return SCDTRIGGERSENS;
}
int32_t OCSetDataWarpType::command() const
{
	return SCDWARPTYPE;
}
int32_t OCSetDataWarpSens::command() const
{
	return SCDWARPSENS;
}
int32_t OCSetDataWarpDirect::command() const
{
	return SCDWARPDIRECT;
}
int32_t OCSetDataWarpLoc::command() const
{
	return SCDWARPLOCATION;
}
int32_t OCSetDataWater::command() const
{
	return SCDWATER;
}
int32_t OCSetDataWhistle::command() const
{
	return SCDWHISTLE;
}
int32_t OCSetDataWeapBlockLevel::command() const
{
	return SCDBLOCKWEAPLVL;
}
int32_t OCSetDataTile::command() const
{
	return SCDTILE;
}
int32_t OCSetDataFlip::command() const
{
	return SCDFLIP;
}
int32_t OCSetDataWalkability::command() const
{
	return SCDWALK;
}
int32_t OCSetDataType::command() const
{
	return SCDTYPE;
}
int32_t OCSetDataCSets::command() const
{
	return SCDCSETS;
}
int32_t OCSetDataFoo::command() const
{
	return SCDFOO;
}
int32_t OCSetDataFrames::command() const
{
	return SCDFRAMES;
}
int32_t OCSetDataSpeed::command() const
{
	return SCDSPEED;
}
int32_t OCSetDataNext::command() const
{
	return SCDNEXTCOMBO;
}
int32_t OCSetDataNextCSet::command() const
{
	return SCDNEXTCSET;
}
int32_t OCSetDataFlag::command() const
{
	return SCDFLAG;
}
int32_t OCSetDataSkipAnim::command() const
{
	return SCDSKIPANIM;
}
int32_t OCSetDataTimer::command() const
{
	return SCDNEXTTIMER;
}
int32_t OCSetDataAnimY::command() const
{
	return SCDSKIPANIMY;
}
int32_t OCSetDataAnimFlags::command() const
{
	return SCDANIMFLAGS;
}

int32_t OCSetDataWinGame::command() const
{
	return SCDWINGAME;
}

//SpriteData
int32_t OSDataTile::command() const
{
	return GETSPRITEDATATILE;
}
int32_t OSDataMisc::command() const
{
	return GETSPRITEDATAMISC;
}
int32_t OSDataCSets::command() const
{
	return GETSPRITEDATACGETS;
}
int32_t OSDataFrames::command() const
{
	return GETSPRITEDATAFRAMES;
}
int32_t OSDataSpeed::command() const
{
	return GETSPRITEDATASPEED;
}
int32_t OSDataType::command() const
{
	return GETSPRITEDATATYPE;
}

int32_t OSSetDataTile::command() const
{
	return SETSPRITEDATATILE;
}
int32_t OSSetDataMisc::command() const
{
	return SETSPRITEDATAMISC;
}
int32_t OSSetDataCSets::command() const
{
	return SETSPRITEDATACSETS;
}
int32_t OSSetDataFrames::command() const
{
	return SETSPRITEDATAFRAMES;
}
int32_t OSSetDataSpeed::command() const
{
	return SETSPRITEDATASPEED;
}
int32_t OSSetDataType::command() const
{
	return SETSPRITEDATATYPE;
}

//Continue Screen Settings
int32_t OSSetContinueScreen::command() const
{
	return SETCONTINUESCREEN;
}
int32_t OSSetContinueString::command() const
{
	return SETCONTINUESTRING;
}

//Visual effects with one bool arg.

int32_t OWavyR::command() const
{
	return FXWAVYR;
}

int32_t OZapR::command() const
{
	return FXZAPR;
}

int32_t OGreyscaleR::command() const
{
	return GREYSCALER;
}

int32_t OMonochromeR::command() const
{
	return MONOCHROMER;
}

int32_t OClearTint::command() const
{
	return CLEARTINT;
}

int32_t OTintR::command() const
{
	return TINT;
}

int32_t OMonoHueR::command() const
{
	return MONOHUE;
}

//Bitmap commands

int32_t OBMPRectangleRegister::command() const
{
	return BMPRECTR;
}

int32_t OBMPFrameRegister::command() const
{
	return BMPFRAMER;
}

int32_t OBMPCircleRegister::command() const
{
	return BMPCIRCLER;
}

int32_t OBMPArcRegister::command() const
{
	return BMPARCR;
}

int32_t OBMPEllipseRegister::command() const
{
	return BMPELLIPSER;
}

int32_t OBMPLineRegister::command() const
{
	return BMPLINER;
}

int32_t OBMPSplineRegister::command() const
{
	return BMPSPLINER;
}

int32_t OBMPPutPixelRegister::command() const
{
	return BMPPUTPIXELR;
}

int32_t OBMPDrawCharRegister::command() const
{
	return BMPDRAWCHARR;
}

int32_t OBMPDrawIntRegister::command() const
{
	return BMPDRAWINTR;
}

int32_t OBMPDrawTileRegister::command() const
{
	return BMPDRAWTILER;
}

int32_t OBMPDrawTileCloakedRegister::command() const
{
	return BMPDRAWTILECLOAKEDR;
}

int32_t OBMPDrawComboRegister::command() const
{
	return BMPDRAWCOMBOR;
}

int32_t OBMPDrawComboCloakedRegister::command() const
{
	return BMPDRAWCOMBOCLOAKEDR;
}

int32_t OBMPFastTileRegister::command() const
{
	return BMPFASTTILER;
}

int32_t OBMPFastComboRegister::command() const
{
	return BMPFASTCOMBOR;
}

int32_t OBMPDrawStringRegister::command() const
{
	return BMPDRAWSTRINGR;
}

int32_t OBMPDrawString2Register::command() const
{
	return BMPDRAWSTRINGR2;
}

int32_t OBMPDrawBitmapExRegister::command() const
{
	return BMPBLIT;
}
int32_t OBMPBlitTO::command() const
{
	return BMPBLITTO;
}

int32_t OBMPGetPixel::command() const
{
	return BITMAPGETPIXEL;
}
int32_t OBMPMode7::command() const
{
	return BMPMODE7;
}

int32_t OBMPQuadRegister::command() const
{
	return BMPQUADR;
}

int32_t OBMPTriangleRegister::command() const
{
	return BMPTRIANGLER;
}


int32_t OBMPQuad3DRegister::command() const
{
	return BMPQUAD3DR;
}

int32_t OBMPTriangle3DRegister::command() const
{
	return BMPTRIANGLE3DR;
}

int32_t OBMPDrawLayerRegister::command() const
{
	return BMPDRAWLAYERR;
}

int32_t OBMPDrawScreenRegister::command() const
{
	return BMPDRAWSCREENR;
}

int32_t OHeroWarpExRegister::command() const
{
	return LINKWARPEXR;
}

int32_t OHeroExplodeRegister::command() const
{
	return LINKEXPLODER;
}

int32_t OSwitchNPC::command() const
{
	return SWITCHNPC;
}
int32_t OSwitchItem::command() const
{
	return SWITCHITM;
}
int32_t OSwitchLW::command() const
{
	return SWITCHLW;
}
int32_t OSwitchEW::command() const
{
	return SWITCHEW;
}
int32_t OSwitchCombo::command() const
{
	return SWITCHCMB;
}
int32_t OKillPlayer::command() const
{
	return KILLPLAYER;
}

int32_t OScreenDoSpawn::command() const
{
	return SCREENDOSPAWN;
}

int32_t OScreenTriggerCombo::command() const
{
	return SCRTRIGGERCOMBO;
}

int32_t ONPCMovePaused::command() const
{
	return NPCMOVEPAUSED;
}
int32_t ONPCMove::command() const
{
	return NPCMOVE;
}
int32_t ONPCMoveAngle::command() const
{
	return NPCMOVEANGLE;
}
int32_t ONPCMoveXY::command() const
{
	return NPCMOVEXY;
}
int32_t ONPCCanMoveDir::command() const
{
	return NPCCANMOVEDIR;
}
int32_t ONPCCanMoveAngle::command() const
{
	return NPCCANMOVEANGLE;
}
int32_t ONPCCanMoveXY::command() const
{
	return NPCCANMOVEXY;
}
int32_t ONPCCanPlace::command() const
{
	return NPCCANPLACE;
}
int32_t ONPCIsFlickerFrame::command() const
{
	return NPCISFLICKERFRAME;
}


int32_t OGetSystemRTCRegister::command() const
{
	return GETRTCTIMER;
}


int32_t ONPCExplodeRegister::command() const
{
	return NPCEXPLODER;
}
int32_t OLWeaponExplodeRegister::command() const
{
	return LWEAPONEXPLODER;
}
int32_t OEWeaponExplodeRegister::command() const
{
	return EWEAPONEXPLODER;
}
int32_t OItemExplodeRegister::command() const
{
	return ITEMEXPLODER;
}

int32_t ORunItemScript::command() const
{
	return RUNITEMSCRIPT;
}

//new npc-> functions for npc scripts
int32_t ONPCDead::command() const
{
	return NPCDEAD;
}
int32_t ONPCCanSlide::command() const
{
	return NPCCANSLIDE;
}
int32_t ONPCSlide::command() const
{
	return NPCSLIDE;
}
int32_t ONPCBreatheFire::command() const
{
	return NPCFIREBREATH;
}
int32_t ONPCNewDir8::command() const
{
	return NPCNEWDIR8;
}
int32_t ONPCRemove::command() const
{
	return NPCKICKBUCKET;
}
int32_t OLWpnRemove::command() const
{
	return LWPNDEL;
}
int32_t OEWpnRemove::command() const
{
	return EWPNDEL;
}
int32_t OItemRemove::command() const
{
	return ITEMDEL;
}
int32_t ONPCStopSFX::command() const
{
	return NPCSTOPBGSFX;
}
int32_t ONPCAttack::command() const
{
	return NPCATTACK;
}
int32_t ONPCNewDir::command() const
{
	return NPCNEWDIR;
}
int32_t ONPCConstWalk::command() const
{
	return NPCCONSTWALK;
}
int32_t ONPCConstWalk8::command() const
{
	return NPCCONSTWALK8;
}
int32_t ONPCVarWalk::command() const
{
	return NPCVARWALK;
}
int32_t ONPCVarWalk8::command() const
{
	return NPCVARWALK8;
}
int32_t ONPCHaltWalk::command() const
{
	return NPCHALTWALK;
}
int32_t ONPCHaltWalk8::command() const
{
	return NPCHALTWALK8;
}
int32_t ONPCFloatWalk::command() const
{
	return NPCFLOATWALK;
}
int32_t ONPCHeroInRange::command() const
{
	return NPCLINKINRANGE;
}
int32_t ONPCAdd::command() const
{
	return NPCADD;
}
int32_t ONPCCanMove::command() const
{
	return NPCCANMOVE;
}
int32_t ONPCHitWith::command() const
{
	return NPCHITWITH;
}
int32_t ONPCKnockback::command() const
{
	return NPCKNOCKBACK;
}

int32_t OGetNPCDataName::command() const
{
	return NPCDATAGETNAME;
}

int32_t OAllocateBitmap::command() const
{
	return ALLOCATEBITMAP;
}

int32_t OClearBitmap::command() const
{
	return CLEARBITMAP;
}

int32_t OBitmapClearToColor::command() const
{
	return BITMAPCLEARTOCOLOR;
}

int32_t ORegenerateBitmap::command() const
{
	return REGENERATEBITMAP;
}

int32_t OReadBitmap::command() const
{
	return READBITMAP;
}
int32_t OWriteBitmap::command() const
{
	return WRITEBITMAP;
}
int32_t OBitmapFree::command() const
{
	return BITMAPFREE;
}
int32_t OBitmapOwn::command() const
{
	return BITMAPOWN;
}
int32_t OFileOwn::command() const
{
	return FILEOWN;
}
int32_t ODirectoryOwn::command() const
{
	return DIRECTORYOWN;
}
int32_t ORNGOwn::command() const
{
	return RNGOWN;
}
int32_t OBitmapWriteTile::command() const
{
	return BMPWRITETILE;
}
int32_t OBitmapDither::command() const
{
	return BMPDITHER;
}
int32_t OBitmapReplColor::command() const
{
	return BMPREPLCOLOR;
}
int32_t OBitmapShiftColor::command() const
{
	return BMPSHIFTCOLOR;
}
int32_t OBitmapMaskDraw::command() const
{
	return BMPMASKDRAW;
}
int32_t OBitmapMaskDraw2::command() const
{
	return BMPMASKDRAW2;
}
int32_t OBitmapMaskDraw3::command() const
{
	return BMPMASKDRAW3;
}
int32_t OBitmapMaskBlit::command() const
{
	return BMPMASKBLIT;
}
int32_t OBitmapMaskBlit2::command() const
{
	return BMPMASKBLIT2;
}
int32_t OBitmapMaskBlit3::command() const
{
	return BMPMASKBLIT3;
}

int32_t OIsValidBitmap::command() const
{
	return ISVALIDBITMAP;
}

int32_t OIsAllocatedBitmap::command() const
{
	return ISALLOCATEDBITMAP;
}


int32_t OBMPDrawScreenSolidRegister::command() const
{
	return BMPDRAWSCREENSOLIDR;
}

int32_t OBMPDrawScreenSolid2Register::command() const
{
	return BMPDRAWSCREENSOLID2R;
}
int32_t OBMPDrawScreenComboFRegister::command() const
{
	return BMPDRAWSCREENCOMBOFR;
}
int32_t OBMPDrawScreenComboIRegister::command() const
{
	return BMPDRAWSCREENCOMBOIR;
}
int32_t OBMPDrawScreenComboTRegister::command() const
{
	return BMPDRAWSCREENCOMBOTR;
}
int32_t OGraphicsGetpixel::command() const
{
	return GRAPHICSGETPIXEL;
}
int32_t OGraphicsCountColor::command() const
{
	return GRAPHICSCOUNTCOLOR;
}

int32_t OBMPDrawScreenSolidityRegister::command() const
{
	return BMPDRAWLAYERSOLIDITYR;
}
int32_t OBMPDrawScreenSolidMaskRegister::command() const
{
	return BMPDRAWLAYERSOLIDR;
}
int32_t OBMPDrawScreenCTypeRegister::command() const
{
	return BMPDRAWLAYERCTYPER;
}
int32_t OBMPDrawScreenCFlagRegister::command() const
{
	return BMPDRAWLAYERCFLAGR;
}
int32_t OBMPDrawScreenCIFlagRegister::command() const
{
	return BMPDRAWLAYERCIFLAGR;
}

//Text ptr opcodes
int32_t OFontHeight::command() const
{
	return FONTHEIGHTR;
}

int32_t OStringWidth::command() const
{
	return STRINGWIDTHR;
}

int32_t OCharWidth::command() const
{
	return CHARWIDTHR;
}

int32_t OMessageWidth::command() const
{
	return MESSAGEWIDTHR;
}

int32_t OMessageHeight::command() const
{
	return MESSAGEHEIGHTR;
}

//

int32_t OStrCmp::command() const
{
	return STRINGCOMPARE;
}

int32_t OStrNCmp::command() const
{
	return STRINGNCOMPARE;
}

int32_t OStrICmp::command() const
{
	return STRINGICOMPARE;
}

int32_t OStrNICmp::command() const
{
	return STRINGNICOMPARE;
}

//based on Ostrcpy
int32_t oARRAYCOPY::command() const
{
	return ARRAYCOPY;
}

/*to do 
  //1 INPUT, NO RETURN 
     { "REMCHR",                2,   0,   0,   0},
     { "STRINGUPPERLOWER",                2,   0,   0,   0},
     { "STRINGLOWERUPPER",                2,   0,   0,   0},
     { "STRINGCONVERTCASE",                2,   0,   0,   0},
     */

//1 inp, 1 ret, baseds on STRINGLENGTH / Ostrlen

int32_t Oxlen::command() const
{
	return XLEN;
}
int32_t Oxtoi::command() const
{
	return XTOI;
}
int32_t Oilen::command() const
{
	return ILEN;
}
int32_t Oatoi::command() const
{
	return ATOI;
}

//2 inp, 1 ret, based on STRINGCOMPARE / OStrCmp

int32_t Ostrcspn::command() const
{
	return STRCSPN;
}

int32_t Ostrstr::command() const
{
	return STRSTR;
}

int32_t Oitoa::command() const
{
	return ITOA;
}

int32_t Oxtoa::command() const
{
	return XTOA;
}

int32_t Oitoacat::command() const
{
	return ITOACAT;
}

int32_t OSaveGameStructs::command() const
{
	return SAVEGAMESTRUCTS;
}

int32_t OReadGameStructs::command() const
{
	return READGAMESTRUCTS;
}

int32_t Ostrcat::command() const
{
	return STRCAT;
}

int32_t Ostrspn::command() const
{
	return STRSPN;
}
int32_t Ostrchr::command() const
{
	return STRCHR;
}

int32_t Ostrrchr::command() const
{
	return STRRCHR;
}
int32_t Oxlen2::command() const
{
	return XLEN2;
}

int32_t Oxtoi2::command() const
{
	return XTOI2;
}
int32_t Oilen2::command() const
{
	return ILEN2;
}
int32_t Oatoi2::command() const
{
	return ATOI2;
}
int32_t Oremchr2::command() const
{
	return REMCHR2;
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








int32_t Ouppertolower::command() const
{
	return UPPERTOLOWER;
}
int32_t Olowertoupper::command() const
{
	return LOWERTOUPPER;
}
int32_t Oconvertcase::command() const
{
	return CONVERTCASE;
}

//Game->GetByString functions
//similar to Oconvertcase

int32_t OGETNPCSCRIPT::command() const
{
	return GETNPCSCRIPT;
}
int32_t OGETLWEAPONSCRIPT::command() const
{
	return GETLWEAPONSCRIPT;
}
int32_t OGETEWEAPONSCRIPT::command() const
{
	return GETEWEAPONSCRIPT;
}
int32_t OGETGENERICSCRIPT::command() const
{
	return GETGENERICSCRIPT;
}
int32_t OGETHEROSCRIPT::command() const
{
	return GETHEROSCRIPT;
}
int32_t OGETGLOBALSCRIPT::command() const
{
	return GETGLOBALSCRIPT;
}
int32_t OGETDMAPSCRIPT::command() const
{
	return GETDMAPSCRIPT;
}
int32_t OGETSCREENSCRIPT::command() const
{
	return GETSCREENSCRIPT;
}
int32_t OGETSPRITESCRIPT::command() const
{
	return GETSPRITESCRIPT;
}
int32_t OGETUNTYPEDSCRIPT::command() const
{
	return GETUNTYPEDSCRIPT;
}
int32_t OGETSUBSCREENSCRIPT::command() const
{
	return GETSUBSCREENSCRIPT;
}
int32_t OGETNPCBYNAME::command() const
{
	return GETNPCBYNAME;
}
int32_t OGETITEMBYNAME::command() const
{
	return GETITEMBYNAME;
}
int32_t OGETCOMBOBYNAME::command() const
{
	return GETCOMBOBYNAME;
}
int32_t OGETDMAPBYNAME::command() const
{
	return GETDMAPBYNAME;
}

int32_t OLoadNPCBySUIDRegister::command() const
{
	return LOADNPCBYSUID;
}
int32_t OLoadLWeaponBySUIDRegister::command() const
{
	return LOADLWEAPONBYSUID;
}
int32_t OLoadEWeaponBySUIDRegister::command() const
{
	return LOADWEAPONCBYSUID;
}

int32_t OByte::command() const
{
	return TOBYTE;
}

int32_t OToInteger::command() const
{
	return TOINTEGER;
}

int32_t OFloor::command() const
{
	return FLOOR;
}

int32_t OTruncate::command() const
{
	return TRUNCATE;
}

int32_t ORound::command() const
{
	return ROUND;
}

int32_t ORoundAway::command() const
{
	return ROUNDAWAY;
}

int32_t OCeiling::command() const
{
	return CEILING;
}

int32_t OSByte::command() const
{
	return TOSIGNEDBYTE;
}
int32_t OWord::command() const
{
	return TOWORD;
}
int32_t OShort::command() const
{
	return TOSHORT;
}

int32_t OReturn::command() const
{
	return RETURN;
}

int32_t ODirExists::command() const
{
	return DIREXISTS;
}

int32_t OFileExists::command() const
{
	return FILEEXISTS;
}

int32_t OFileSystemRemove::command() const
{
	return FILESYSREMOVE;
}

int32_t OFileClose::command() const
{
	return FILECLOSE;
};

int32_t OFileFree::command() const
{
	return FILEFREE;
};

int32_t OFileIsAllocated::command() const
{
	return FILEISALLOCATED;
};

int32_t OFileIsValid::command() const
{
	return FILEISVALID;
};

int32_t OAllocateFile::command() const
{
	return FILEALLOCATE;
};

int32_t OFileFlush::command() const
{
	return FILEFLUSH;
};

int32_t OFileGetChar::command() const
{
	return FILEGETCHAR;
};

int32_t OFileRewind::command() const
{
	return FILEREWIND;
};

int32_t OFileClearError::command() const
{
	return FILECLEARERR;
};
//
int32_t OFileOpen::command() const
{
	return FILEOPEN;
};

int32_t OFileCreate::command() const
{
	return FILECREATE;
};

int32_t OFileReadString::command() const
{
	return FILEREADSTR;
};

int32_t OFileWriteString::command() const
{
	return FILEWRITESTR;
};

int32_t OFilePutChar::command() const
{
	return FILEPUTCHAR;
};

int32_t OFileUngetChar::command() const
{
	return FILEUNGETCHAR;
};

int32_t OFileGetError::command() const
{
	return FILEGETERROR;
};
//
int32_t OFileReadChars::command() const
{
	return FILEREADCHARS;
};

int32_t OFileReadBytes::command() const
{
	return FILEREADBYTES;
}

int32_t OFileReadInts::command() const
{
	return FILEREADINTS;
};

int32_t OFileWriteChars::command() const
{
	return FILEWRITECHARS;
};

int32_t OFileWriteBytes::command() const
{
	return FILEWRITEBYTES;
}

int32_t OFileWriteInts::command() const
{
	return FILEWRITEINTS;
};

int32_t OFileSeek::command() const
{
	return FILESEEK;
};

int32_t OFileOpenMode::command() const
{
	return FILEOPENMODE;
};

int32_t OFileRemove::command() const
{
	return FILEREMOVE;
};

int32_t ODirectoryGet::command() const
{
	return DIRECTORYGET;
}

int32_t ODirectoryReload::command() const
{
	return DIRECTORYRELOAD;
}

int32_t ODirectoryFree::command() const
{
	return DIRECTORYFREE;
}

int32_t OStackFree::command() const
{
	return STACKFREE;
}
int32_t OStackOwn::command() const
{
	return STACKOWN;
}
int32_t OStackClear::command() const
{
	return STACKCLEAR;
}

int32_t OStackPopBack::command() const
{
	return STACKPOPBACK;
}
int32_t OStackPopFront::command() const
{
	return STACKPOPFRONT;
}
int32_t OStackPeekBack::command() const
{
	return STACKPEEKBACK;
}
int32_t OStackPeekFront::command() const
{
	return STACKPEEKFRONT;
}
int32_t OStackPushBack::command() const
{
	return STACKPUSHBACK;
}
int32_t OStackPushFront::command() const
{
	return STACKPUSHFRONT;
}
int32_t OStackGet::command() const
{
	return STACKGET;
}
int32_t OStackSet::command() const
{
	return STACKSET;
}

int32_t OModuleGetIC::command() const
{
	return MODULEGETIC;
}


//////////////////////////////////////////////////////////////////////////////////////

int32_t ORunGenericFrozenScript::command() const
{
	return RUNGENFRZSCR;
};

int32_t OReservedZ3_01::command() const
{
	return RESRVD_OP_Z3_01;
}
int32_t OReservedZ3_02::command() const
{
	return RESRVD_OP_Z3_02;
}
int32_t OReservedZ3_03::command() const
{
	return RESRVD_OP_Z3_03;
}
int32_t OReservedZ3_04::command() const
{
	return RESRVD_OP_Z3_04;
}
int32_t OReservedZ3_05::command() const
{
	return RESRVD_OP_Z3_05;
}
int32_t OReservedZ3_06::command() const
{
	return RESRVD_OP_Z3_06;
}
int32_t OReservedZ3_07::command() const
{
	return RESRVD_OP_Z3_07;
}
int32_t OReservedZ3_08::command() const
{
	return RESRVD_OP_Z3_08;
}
int32_t OReservedZ3_09::command() const
{
	return RESRVD_OP_Z3_09;
}
int32_t OReservedZ3_10::command() const
{
	return RESRVD_OP_Z3_10;
}


int32_t OSubscrSwapPages::command() const
{
	return SUBDATA_SWAP_PAGES;
}
int32_t OSubscrPgFindWidget::command() const
{
	return SUBPAGE_FIND_WIDGET;
}
int32_t OSubscrPgMvCursor::command() const
{
	return SUBPAGE_MOVE_SEL;
}
int32_t OSubscrPgSwapWidgets::command() const
{
	return SUBPAGE_SWAP_WIDG;
}
int32_t OSubscrPgNewWidget::command() const
{
	return SUBPAGE_NEW_WIDG;
}
int32_t OSubscrPgDelete::command() const
{
	return SUBPAGE_DELETE;
}
int32_t OGetSubWidgSelTxtOverride::command() const
{
	return SUBWIDG_GET_SELTEXT_OVERRIDE;
}
int32_t OSetSubWidgSelTxtOverride::command() const
{
	return SUBWIDG_SET_SELTEXT_OVERRIDE;
}
int32_t OSubWidgTy_GetText::command() const
{
	return SUBWIDG_TY_GETTEXT;
}
int32_t OSubWidgTy_SetText::command() const
{
	return SUBWIDG_TY_SETTEXT;
}


int32_t OSubscrPgFindWidgetLbl::command() const
{
	return SUBPAGE_FIND_WIDGET_BY_LABEL;
}


int32_t OGetSubWidgLabel::command() const
{
	return SUBWIDG_GET_LABEL;
}
int32_t OSetSubWidgLabel::command() const
{
	return SUBWIDG_SET_LABEL;
}


int32_t OWrapRadians::command() const
{
	return WRAPRADIANS;
}
int32_t OWrapDegrees::command() const
{
	return WRAPDEGREES;
}


int32_t OCallFunc::command() const
{
	return CALLFUNC;
}


int32_t OReturnFunc::command() const
{
	return RETURNFUNC;
}


int32_t OSetCompare::command() const
{
	return SETCMP;
}

int32_t OGotoCompare::command() const
{
	return GOTOCMP;
}


int32_t OStackWriteAtRV::command() const
{
	return STACKWRITEATRV;
}
int32_t OStackWriteAtVV::command() const
{
	return STACKWRITEATVV;
}


int32_t OStackWriteAtVV_If::command() const
{
	return STACKWRITEATVV_IF;
}


int32_t OSetGVarR::command() const
{
	return SETGVARR;
}
int32_t OSetGVarV::command() const
{
	return SETGVARV;
}
int32_t OGetGVar::command() const
{
	return GETGVAR;
}


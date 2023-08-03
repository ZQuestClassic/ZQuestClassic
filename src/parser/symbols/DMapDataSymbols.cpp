#include "SymbolDefs.h"

DMapDataSymbols DMapDataSymbols::singleton = DMapDataSymbols();

static AccessorTable DMapDataTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "GetName",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_DMAPDATA, ZTID_CHAR },{} },
	{ "SetName",                    0,          ZTID_VOID,   -1,                        0,  { ZTID_DMAPDATA, ZTID_CHAR },{} },
	{ "GetTitle",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_DMAPDATA, ZTID_CHAR },{} },
	{ "SetTitle",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_DMAPDATA, ZTID_CHAR },{} },
	{ "GetIntro",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_DMAPDATA, ZTID_CHAR },{} },
	{ "SetIntro",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_DMAPDATA, ZTID_CHAR },{} },
	{ "GetMusic",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_DMAPDATA, ZTID_CHAR },{} },
	{ "SetMusic",                   0,          ZTID_VOID,   -1,                        0,  { ZTID_DMAPDATA, ZTID_CHAR },{} },
	//
	{ "getID",                      0,         ZTID_FLOAT,   DMAPDATAID,                0,  { ZTID_DMAPDATA },{} },
	{ "getMap",                     0,         ZTID_FLOAT,   DMAPDATAMAP,               0,  { ZTID_DMAPDATA },{} },
	{ "setMap",                     0,          ZTID_VOID,   DMAPDATAMAP,               0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getLevel",                   0,         ZTID_FLOAT,   DMAPDATALEVEL,             0,  { ZTID_DMAPDATA },{} },
	{ "setLevel",                   0,          ZTID_VOID,   DMAPDATALEVEL,             0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getOffset",                  0,         ZTID_FLOAT,   DMAPDATAOFFSET,            0,  { ZTID_DMAPDATA },{} },
	{ "setOffset",                  0,          ZTID_VOID,   DMAPDATAOFFSET,            0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getCompass",                 0,         ZTID_FLOAT,   DMAPDATACOMPASS,           0,  { ZTID_DMAPDATA },{} },
	{ "setCompass",                 0,          ZTID_VOID,   DMAPDATACOMPASS,           0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getPalette",                 0,         ZTID_FLOAT,   DMAPDATAPALETTE,           0,  { ZTID_DMAPDATA },{} },
	{ "setPalette",                 0,          ZTID_VOID,   DMAPDATAPALETTE,           0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getMIDI",                    0,         ZTID_FLOAT,   DMAPDATAMIDI,              0,  { ZTID_DMAPDATA },{} },
	{ "setMIDI",                    0,          ZTID_VOID,   DMAPDATAMIDI,              0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getContinue",                0,         ZTID_FLOAT,   DMAPDATACONTINUE,          0,  { ZTID_DMAPDATA },{} },
	{ "setContinue",                0,          ZTID_VOID,   DMAPDATACONTINUE,          0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getType",                    0,         ZTID_FLOAT,   DMAPDATATYPE,              0,  { ZTID_DMAPDATA },{} },
	{ "setType",                    0,          ZTID_VOID,   DMAPDATATYPE,              0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getScript",                  0,         ZTID_FLOAT,   DMAPSCRIPT,                0,  { ZTID_DMAPDATA },{} },
	{ "setScript",                  0,          ZTID_VOID,   DMAPSCRIPT,                0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getMusicTrack",              0,         ZTID_FLOAT,   DMAPDATAMUISCTRACK,        0,  { ZTID_DMAPDATA },{} },
	{ "setMusicTrack",              0,          ZTID_VOID,   DMAPDATAMUISCTRACK,        0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getActiveSubscreen",         0,         ZTID_FLOAT,   DMAPDATASUBSCRA,           0,  { ZTID_DMAPDATA },{} },
	{ "setActiveSubscreen",         0,          ZTID_VOID,   DMAPDATASUBSCRA,           0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getPassiveSubscreen",        0,         ZTID_FLOAT,   DMAPDATASUBSCRP,           0,  { ZTID_DMAPDATA },{} },
	{ "setPassiveSubscreen",        0,          ZTID_VOID,   DMAPDATASUBSCRP,           0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getGrid[]",                  0,         ZTID_FLOAT,   DMAPDATAGRID,              0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setGrid[]",                  0,          ZTID_VOID,   DMAPDATAGRID,              0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getMiniMapTile[]",           0,         ZTID_FLOAT,   DMAPDATAMINIMAPTILE,       0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setMiniMapTile[]",           0,          ZTID_VOID,   DMAPDATAMINIMAPTILE,       0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getMiniMapCSet[]",           0,         ZTID_FLOAT,   DMAPDATAMINIMAPCSET,       0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setMiniMapCSet[]",           0,          ZTID_VOID,   DMAPDATAMINIMAPCSET,       0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getInitD[]",                 0,         ZTID_FLOAT,   DMAPINITD,                 0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setInitD[]",                 0,          ZTID_VOID,   DMAPINITD,                 0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getMapTile[]",               0,         ZTID_FLOAT,   DMAPDATALARGEMAPTILE,      0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setMapTile[]",               0,          ZTID_VOID,   DMAPDATALARGEMAPTILE,      0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getMapCSet[]",               0,         ZTID_FLOAT,   DMAPDATALARGEMAPCSET,      0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setMapCSet[]",               0,          ZTID_VOID,   DMAPDATALARGEMAPCSET,      0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	{ "getDisabledItems[]",         0,          ZTID_BOOL,   DMAPDATADISABLEDITEMS,     0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setDisabledItems[]",         0,          ZTID_VOID,   DMAPDATADISABLEDITEMS,     0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_BOOL },{} },
	{ "getFlagset[]",               0,         ZTID_FLOAT,   DMAPDATAFLAGARR,           0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setFlagset[]",               0,          ZTID_VOID,   DMAPDATAFLAGARR,           0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getSideview",                0,          ZTID_BOOL,   DMAPDATASIDEVIEW,          0,  { ZTID_DMAPDATA },{} },
	{ "setSideview",                0,          ZTID_VOID,   DMAPDATASIDEVIEW,          0,  { ZTID_DMAPDATA, ZTID_BOOL },{} },
	
	{ "getASubScript",              0,         ZTID_FLOAT,   DMAPDATAASUBSCRIPT,        0,  { ZTID_DMAPDATA },{} },
	{ "setASubScript",              0,          ZTID_VOID,   DMAPDATAASUBSCRIPT,        0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getPSubScript",              0,         ZTID_FLOAT,   DMAPDATAPSUBSCRIPT,        0,  { ZTID_DMAPDATA },{} },
	{ "setPSubScript",              0,          ZTID_VOID,   DMAPDATAPSUBSCRIPT,        0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getSubInitD[]",              0,       ZTID_UNTYPED,   DMAPDATASUBINITD,          0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setSubInitD[]",              0,          ZTID_VOID,   DMAPDATASUBINITD,          0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getMapScript",               0,         ZTID_FLOAT,   DMAPDATAMAPSCRIPT,         0,  { ZTID_DMAPDATA },{} },
	{ "setMapScript",               0,          ZTID_VOID,   DMAPDATAMAPSCRIPT,         0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "getMapInitD[]",              0,       ZTID_UNTYPED,   DMAPDATAMAPINITD,          0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setMapInitD[]",              0,          ZTID_VOID,   DMAPDATAMAPINITD,          0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_UNTYPED },{} },
	{ "getCharted[]",               0,         ZTID_FLOAT,   DMAPDATACHARTED,           0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	{ "setCharted[]",               0,          ZTID_VOID,   DMAPDATACHARTED,           0,  { ZTID_DMAPDATA, ZTID_FLOAT, ZTID_FLOAT },{} },
	
	{ "getMirrorDMap",              0,         ZTID_FLOAT,   DMAPDATAMIRRDMAP,          0,  { ZTID_DMAPDATA },{} },
	{ "setMirrorDMap",              0,          ZTID_VOID,   DMAPDATAMIRRDMAP,          0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	
	//Intentionally undocumented
	{ "getFlags",                   0,         ZTID_FLOAT,   DMAPDATAFLAGS,             0,  { ZTID_DMAPDATA },{} },
	{ "setFlags",                   0,          ZTID_VOID,   DMAPDATAFLAGS,             0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },
	
	{ "getMusicLoopStart",               0,         ZTID_FLOAT,   DMAPDATALOOPSTART,         0,  { ZTID_DMAPDATA },{} },
	{ "setMusicLoopStart",               0,          ZTID_VOID,   DMAPDATALOOPSTART,         0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },

	{ "getMusicLoopEnd",                 0,         ZTID_FLOAT,   DMAPDATALOOPEND,         0,  { ZTID_DMAPDATA },{} },
	{ "setMusicLoopEnd",                 0,          ZTID_VOID,   DMAPDATALOOPEND,         0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },

	{ "getMusicCrossfadeIn",                 0,         ZTID_FLOAT,   DMAPDATAXFADEIN,         0,  { ZTID_DMAPDATA },{} },
	{ "setMusicCrossfadeIn",                 0,          ZTID_VOID,   DMAPDATAXFADEIN,         0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },

	{ "getMusicCrossfadeOut",                 0,         ZTID_FLOAT,   DMAPDATAXFADEOUT,         0,  { ZTID_DMAPDATA },{} },
	{ "setMusicCrossfadeOut",                 0,          ZTID_VOID,   DMAPDATAXFADEOUT,         0,  { ZTID_DMAPDATA, ZTID_FLOAT },{} },

	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

DMapDataSymbols::DMapDataSymbols()
{
	table = DMapDataTable;
	refVar = REFDMAPDATA;
}

void DMapDataSymbols::generateCode()
{
	//void GetName(dmapdata, char)
	{
		Function* function = getFunction("GetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ODMapDataGetNameRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetName(bottledata, char)
	{
		Function* function = getFunction("SetName");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ODMapDataSetNameRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void GetTitle(dmapdata, char)
	{
		Function* function = getFunction("GetTitle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ODMapDataGetTitleRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetTitle(bottledata, char)
	{
		Function* function = getFunction("SetTitle");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ODMapDataSetTitleRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void GetIntro(dmapdata, char)
	{
		Function* function = getFunction("GetIntro");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ODMapDataGetIntroRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetIntro(bottledata, char)
	{
		Function* function = getFunction("SetIntro");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ODMapDataSetIntroRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void GetMusic(dmapdata, char)
	{
		Function* function = getFunction("GetMusic");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ODMapDataGetMusicRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//void SetMusic(bottledata, char)
	{
		Function* function = getFunction("SetMusic");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the param
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ODMapDataSetMusicRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	
}


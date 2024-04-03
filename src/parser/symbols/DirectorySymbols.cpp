#include "SymbolDefs.h"

DirectorySymbols DirectorySymbols::singleton = DirectorySymbols();

static AccessorTable DirectoryTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "getSize",                    0,         ZTID_FLOAT,   DIRECTORYSIZE,             0,  { ZTID_DIRECTORY },{} },
	{ "setSize",                    0,          ZTID_VOID,   DIRECTORYSIZE,     FL_RDONLY,  { ZTID_DIRECTORY, ZTID_FLOAT },{} },
	{ "GetFilename",                0,          ZTID_BOOL,   -1,                        0,  { ZTID_DIRECTORY, ZTID_FLOAT, ZTID_CHAR },{} },
	{ "Reload",                     0,          ZTID_VOID,   -1,                        0,  { ZTID_DIRECTORY },{} },
	{ "Free",                       0,          ZTID_VOID,   -1,                  FL_DEPR,  { ZTID_DIRECTORY },{},0,"Free() no longer does anything as of ZC 3.0. Objects are now freed automatically." },
	{ "Own",                        0,          ZTID_VOID,   -1,                        0,  { ZTID_DIRECTORY },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

DirectorySymbols::DirectorySymbols()
{
	table = DirectoryTable;
	refVar = REFDIRECTORY;
}

void DirectorySymbols::generateCode()
{
	//bool GetFilename(directory, int32_t, char32)
	{
		Function* function = getFunction("GetFilename");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ODirectoryGet(new VarArgument(EXP1), new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void Reload(directory)
	{
		Function* function = getFunction("Reload");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ODirectoryReload());
		RETURN();
		function->giveCode(code);
	}
	//void Free(directory)
	{
		Function* function = getFunction("Free");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ODirectoryFree());
		RETURN();
		function->giveCode(code);
	}
	//void Own(directory)
	{
		Function* function = getFunction("Own");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new ODirectoryOwn());
		RETURN();
		function->giveCode(code);
	}
}


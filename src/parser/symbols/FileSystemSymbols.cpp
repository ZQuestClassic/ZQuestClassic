#include "SymbolDefs.h"

FileSystemSymbols FileSystemSymbols::singleton = FileSystemSymbols();

static AccessorTable FileSystemTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "DirExists",                  0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILESYSTEM, ZTID_CHAR },{} },
	{ "FileExists",                 0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILESYSTEM, ZTID_CHAR },{} },
	{ "Remove",                     0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILESYSTEM, ZTID_CHAR },{} },
	{ "LoadDirectory",              0,     ZTID_DIRECTORY,   -1,                   FL_INL,  { ZTID_FILESYSTEM, ZTID_CHAR },{} },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
};

FileSystemSymbols::FileSystemSymbols()
{
	table = FileSystemTable;
	refVar = NUL;
}

void FileSystemSymbols::generateCode()
{
	//bool DirExists(FileSystem, char32*)
	{
		Function* function = getFunction("DirExists");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new ODirExists(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool FileExists(FileSystem, char32*)
	{
		Function* function = getFunction("FileExists");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileExists(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool Remove(FileSystem, char32*)
	{
		Function* function = getFunction("Remove");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileSystemRemove(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//directory LoadDirectory(FileSystem, char32*)
	{
		Function* function = getFunction("LoadDirectory");
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OLoadDirectoryRegister(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
}


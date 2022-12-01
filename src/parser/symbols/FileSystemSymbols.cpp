#include "SymbolDefs.h"

FileSystemSymbols FileSystemSymbols::singleton = FileSystemSymbols();

static AccessorTable FileSystemTable[] =
{
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	{ "DirExists",              ZVARTYPEID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_FILESYSTEM, ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "FileExists",             ZVARTYPEID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_FILESYSTEM, ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Remove",                 ZVARTYPEID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_FILESYSTEM, ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "LoadDirectory",          ZVARTYPEID_DIRECTORY,     FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZVARTYPEID_FILESYSTEM, ZVARTYPEID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
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
	    Function* function = getFunction("DirExists", 2);
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
	    Function* function = getFunction("FileExists", 2);
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
	    Function* function = getFunction("Remove", 2);
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
	    Function* function = getFunction("LoadDirectory", 2);
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


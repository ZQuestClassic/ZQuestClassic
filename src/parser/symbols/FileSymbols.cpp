#include "SymbolDefs.h"

FileSymbols FileSymbols::singleton = FileSymbols();

static AccessorTable FileTable[] =
{
//	  name,                     rettype,                  setorget,     var,              numindex,      funcFlags,                            numParams,   params
	{ "Open",                   ZTID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZTID_FILE, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Create",                 ZTID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZTID_FILE, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "OpenMode",               ZTID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      3,           { ZTID_FILE, ZTID_CHAR, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Close",                  ZTID_VOID,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Free",                   ZTID_VOID,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Own",                    ZTID_VOID,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isAllocated",            ZTID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "isValid",                ZTID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Allocate",               ZTID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Flush",                  ZTID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ReadChars",              ZTID_FLOAT,         FUNCTION,     0,                1,             0,                                    4,           { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ReadInts",               ZTID_FLOAT,         FUNCTION,     0,                1,             0,                                    4,           { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WriteChars",             ZTID_FLOAT,         FUNCTION,     0,                1,             0,                                    4,           { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WriteInts",              ZTID_FLOAT,         FUNCTION,     0,                1,             0,                                    4,           { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ReadString",             ZTID_FLOAT,         FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZTID_FILE, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WriteString",            ZTID_FLOAT,         FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZTID_FILE, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetChar",                ZTID_CHAR,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "PutChar",                ZTID_CHAR,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZTID_FILE, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "UngetChar",              ZTID_CHAR,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZTID_FILE, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Seek",                   ZTID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      3,           { ZTID_FILE, ZTID_LONG, ZTID_BOOL, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Rewind",                 ZTID_VOID,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ClearError",             ZTID_VOID,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getPos",                 ZTID_LONG,          GETTER,       FILEPOS,          1,             0,                                    1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getEOF",                 ZTID_FLOAT,         GETTER,       FILEEOF,          1,             0,                                    1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "getError",               ZTID_FLOAT,         GETTER,       FILEERR,          1,             0,                                    1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "GetError",               ZTID_VOID,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      2,           { ZTID_FILE, ZTID_CHAR, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "Remove",                 ZTID_BOOL,          FUNCTION,     0,                1,             FUNCFLAG_INLINE,                      1,           { ZTID_FILE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "WriteBytes",             ZTID_FLOAT,         FUNCTION,     0,                1,             0,                                    4,           { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	{ "ReadBytes",              ZTID_FLOAT,         FUNCTION,     0,                1,             0,                                    4,           { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } },
	
	{ "",                       -1,                       -1,           -1,               -1,            0,                                    0,           { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } }
};

FileSymbols::FileSymbols()
{
	table = FileTable;
	refVar = REFFILE;
}

void FileSymbols::generateCode()
{
	//bool Open(file, char32*)
	{
		Function* function = getFunction("Open", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileOpen(new VarArgument(EXP1)));
		REASSIGN_PTR(EXP2);
		RETURN();
		function->giveCode(code);
	}
	//bool Create(file, char32*)
	{
		Function* function = getFunction("Create", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop off the params
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileCreate(new VarArgument(EXP1)));
		REASSIGN_PTR(EXP2);
		RETURN();
		function->giveCode(code);
	}
	//void Close(file)
	{
		Function* function = getFunction("Close", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OFileClose());
		RETURN();
		function->giveCode(code);
	}
	//void Free(file)
	{
		Function* function = getFunction("Free", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OFileFree());
		RETURN();
		function->giveCode(code);
	}
	//void Own(file)
	{
		Function* function = getFunction("Own", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OFileOwn());
		RETURN();
		function->giveCode(code);
	}
	//bool isAllocated(file)
	{
		Function* function = getFunction("isAllocated", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OFileIsAllocated());
		RETURN();
		function->giveCode(code);
	}
	//bool isValid(file)
	{
		Function* function = getFunction("isValid", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OFileIsValid());
		RETURN();
		function->giveCode(code);
	}
	//bool Allocate(file)
	{
		Function* function = getFunction("Allocate", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OAllocateFile());
		REASSIGN_PTR(EXP2);
		RETURN();
		function->giveCode(code);
	}
	//bool Flush(file)
	{
		Function* function = getFunction("Flush", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OFileFlush());
		RETURN();
		function->giveCode(code);
	}
	//int32_t ReadChars(file, char32*, int32_t, int32_t)
	{
		Function* function = getFunction("ReadChars", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileReadChars(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t ReadString(file, char32*)
	{
		Function* function = getFunction("ReadString", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileReadString(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t ReadInts(file, char32*, int32_t, int32_t)
	{
		Function* function = getFunction("ReadInts", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileReadInts(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t WriteChars(file, char32*, int32_t, int32_t)
	{
		Function* function = getFunction("WriteChars", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileWriteChars(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t WriteString(file, char32*)
	{
		Function* function = getFunction("WriteString", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileWriteString(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t WriteInts(file, char32*, int32_t, int32_t)
	{
		Function* function = getFunction("WriteInts", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileWriteInts(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//char32 GetChar(file)
	{
		Function* function = getFunction("GetChar", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OFileGetChar());
		RETURN();
		function->giveCode(code);
	}
	//char32 PutChar(file, char32 c)
	{
		Function* function = getFunction("PutChar", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFilePutChar(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//char32 UngetChar(file, char32 c)
	{
		Function* function = getFunction("UngetChar", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileUngetChar(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool Seek(file, int_full, bool)
	{
		Function* function = getFunction("Seek", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileSeek(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//void Rewind(file)
	{
		Function* function = getFunction("Rewind", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OFileRewind());
		RETURN();
		function->giveCode(code);
	}
	//void ClearError(file)
	{
		Function* function = getFunction("ClearError", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OFileClearError());
		RETURN();
		function->giveCode(code);
	}
	//int32_t OpenMode(file, char32*, char32*)
	{
		Function* function = getFunction("OpenMode", 3);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileOpenMode(new VarArgument(EXP1),new VarArgument(EXP2)));
		REASSIGN_PTR(EXP2);
		RETURN();
		function->giveCode(code);
	}
	//int32_t GetError(file, char32*)
	{
		Function* function = getFunction("GetError", 2);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		LABELBACK(label);
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileGetError(new VarArgument(EXP1)));
		RETURN();
		function->giveCode(code);
	}
	//bool Remove(file)
	{
		Function* function = getFunction("Remove", 1);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		//pop pointer
		ASSERT_NON_NUL();
		POPREF();
		LABELBACK(label);
		addOpcode2 (code, new OFileRemove());
		RETURN();
		function->giveCode(code);
	}
	//int32_t WriteBytes(file, char32*, int32_t, int32_t)
	{
		Function* function = getFunction("WriteBytes", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileWriteBytes(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
	//int32_t ReadBytes(file, char32*, int32_t, int32_t)
	{
		Function* function = getFunction("ReadBytes", 4);
		int32_t label = function->getLabel();
		vector<shared_ptr<Opcode>> code;
		addOpcode2 (code, new OPopRegister(new VarArgument(INDEX)));
		LABELBACK(label);
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP2)));
		addOpcode2 (code, new OPopRegister(new VarArgument(EXP1)));
		//pop pointer
		POPREF();
		addOpcode2 (code, new OFileReadBytes(new VarArgument(EXP1),new VarArgument(EXP2)));
		RETURN();
		function->giveCode(code);
	}
}


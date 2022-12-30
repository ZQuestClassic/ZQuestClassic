#include "SymbolDefs.h"

FileSymbols FileSymbols::singleton = FileSymbols();

static AccessorTable FileTable[] =
{
	//name,                       tag,            rettype,   var,               funcFlags,  params,optparams
	{ "Open",                       0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILE, ZTID_CHAR },{} },
	{ "Create",                     0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILE, ZTID_CHAR },{} },
	{ "OpenMode",                   0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILE, ZTID_CHAR, ZTID_CHAR },{} },
	{ "Close",                      0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "Free",                       0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "Own",                        0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "isAllocated",                0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "isValid",                    0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "Allocate",                   0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "Flush",                      0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "ReadChars",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT },{ -10000, 0 } },
	{ "ReadInts",                   0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT },{ -10000, 0 } },
	{ "WriteChars",                 0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT },{ -10000, 0 } },
	{ "WriteInts",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT },{ -10000, 0 } },
	{ "ReadString",                 0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_FILE, ZTID_CHAR },{} },
	{ "WriteString",                0,         ZTID_FLOAT,   -1,                   FL_INL,  { ZTID_FILE, ZTID_CHAR },{} },
	{ "GetChar",                    0,          ZTID_CHAR,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "PutChar",                    0,          ZTID_CHAR,   -1,                   FL_INL,  { ZTID_FILE, ZTID_CHAR },{} },
	{ "UngetChar",                  0,          ZTID_CHAR,   -1,                   FL_INL,  { ZTID_FILE, ZTID_CHAR },{} },
	{ "Seek",                       0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILE, ZTID_LONG, ZTID_BOOL },{ 0 } },
	{ "Rewind",                     0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "ClearError",                 0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "getPos",                     0,          ZTID_LONG,   FILEPOS,                   0,  { ZTID_FILE },{} },
	{ "getEOF",                     0,         ZTID_FLOAT,   FILEEOF,                   0,  { ZTID_FILE },{} },
	{ "getError",                   0,         ZTID_FLOAT,   FILEERR,                   0,  { ZTID_FILE },{} },
	{ "GetError",                   0,          ZTID_VOID,   -1,                   FL_INL,  { ZTID_FILE, ZTID_CHAR },{} },
	{ "Remove",                     0,          ZTID_BOOL,   -1,                   FL_INL,  { ZTID_FILE },{} },
	{ "WriteBytes",                 0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT },{ -10000, 0 } },
	{ "ReadBytes",                  0,         ZTID_FLOAT,   -1,                        0,  { ZTID_FILE, ZTID_CHAR, ZTID_FLOAT, ZTID_FLOAT },{ -10000, 0 } },
	
	{ "",                           0,          ZTID_VOID,   -1,                        0,  {},{} }
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
		Function* function = getFunction("Open");
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
		Function* function = getFunction("Create");
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
		Function* function = getFunction("Close");
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
		Function* function = getFunction("Free");
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
		Function* function = getFunction("Own");
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
		Function* function = getFunction("isAllocated");
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
		Function* function = getFunction("isValid");
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
		Function* function = getFunction("Allocate");
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
		Function* function = getFunction("Flush");
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
		Function* function = getFunction("ReadChars");
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
		Function* function = getFunction("ReadString");
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
		Function* function = getFunction("ReadInts");
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
		Function* function = getFunction("WriteChars");
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
		Function* function = getFunction("WriteString");
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
		Function* function = getFunction("WriteInts");
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
		Function* function = getFunction("GetChar");
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
		Function* function = getFunction("PutChar");
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
		Function* function = getFunction("UngetChar");
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
		Function* function = getFunction("Seek");
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
		Function* function = getFunction("Rewind");
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
		Function* function = getFunction("ClearError");
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
		Function* function = getFunction("OpenMode");
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
		Function* function = getFunction("GetError");
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
		Function* function = getFunction("Remove");
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
		Function* function = getFunction("WriteBytes");
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
		Function* function = getFunction("ReadBytes");
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


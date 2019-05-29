
#pragma once

#include "ScriptCommon.h"
#include "Utility.h"
#include "Array.h"

#include <string>
#include <string.h>


struct CppPreprocessorInputArgs
{
	const char** args; // Can be NULL.
	int32 argCount;
	const char* workingDirectory; // Can be NULL ... todo: include paths.. don't use this...
	const char* filename;
	const char* outputProcessedFilename; // Can be NULL.
	const char* outputErrorSringFilename; // Can be NULL.
};


struct CppPreprocessorOutput
{
	int32 returnStatus;
	GLOBAL_PTR char* code; // owned by process; do not free.
	GLOBAL_PTR const char* errorString; // owned by process; do not free.
	u32 codeLength;
	u32 errorStringLength;
	SMinuteSecondsInfo elapsedTime;
};


class CppPreprocessor //this may just be a procedure.
{
public:
	void Init();
	void Shutdown(); //free resources used.

	CppPreprocessorOutput ProcessFile(const CppPreprocessorInputArgs& input);

private:
	static int __cdecl FPutChar(int c, MCPP_OUTDEST od);
	static int __cdecl FPutString(const char* str, MCPP_OUTDEST od);
	static int __cdecl FPrintF(MCPP_OUTDEST od, const char * format, ...);
};


struct ScriptFileData
{
	std::string fullPathAndFilename;
	std::string filename;
	std::string code;
	int32 line;

	ScriptFileData()
		: filename(), code(), line(1)
	{
		// 8kb file default sounds like a reasonable average script size.
		code.reserve(1024 * 8);
	}
};


class Postprocessor
{
public:

	// Only valid for code that has been run through the preprocessor, otherwise
	// this is just an overly complicated way of returning the original code.
	Array<ScriptFileData*> ProcessPreprocessedMemoryFile(const char* scriptFilename, char* code, u32 codeLength);

protected:

	void ProcessLineDirective(char* code, u32 codeLength, u32& index, std::string& filename, int32& outLineNumber);
	void RemoveLine(char* code, u32 codeLength, u32& index, bool removeLineEnding);
	void RemovePath(std::string& filepath);

	bool GetNextToken(const char* str, u32 stringLength, u32& position, std::string& token);
	bool GetNextLine(const char* code, u32 codeLength, u32& index, int32& line, std::string& currentLine);

	std::string m_noAllocToken;
};



enum eBuildFlags
{
	kBuildGeneratePreprocessedFile = 1,
	kBuildOutputScriptSectionsToFiles = 2
};

Array<ScriptFileData*> GenerateScriptSectionsFromFile(const char* filename, u32 buildFlags);


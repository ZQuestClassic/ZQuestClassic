
#include "Utility.h"
#include "Preprocessor.h"


//////////////////////////////////////////////////////////////////////////
/// CppPreprocessor
//////////////////////////////////////////////////////////////////////////

void CppPreprocessor::Init()
{
	//free memory from buffers
	mcpp_use_mem_buffers(1);
}


void CppPreprocessor::Shutdown()
{
	//free memory from buffers...again... yes I know it's weird.
	mcpp_use_mem_buffers(1);
}


CppPreprocessorOutput CppPreprocessor::ProcessFile(const CppPreprocessorInputArgs& input)
{
	const uint32 kMaxStaticArgs = 32;

	CppPreprocessorOutput output = {};
	SimpleTimer simpleTimer = {};
	Array<const char*> args;

	args.Allocate(input.argCount + kMaxStaticArgs);
	{
		if(input.workingDirectory)
			args.Add(input.workingDirectory);

		args.Add("-W17");
		args.Add("-+");
		args.Add("-k");
		args.Add("-D__DEFINED__");

		if(input.args && input.argCount)
			args.InsertRange(args.Size(), input.args, input.argCount);

		args.Add(input.filename);
	}

	simpleTimer.Start();
	//mcpp_use_mem_buffers(1);

	output.returnStatus			= mcpp_lib_main(args.count, (char**)args.data);
	output.code					= mcpp_get_mem_buffer(MCPP_OUT);
	output.errorString			= mcpp_get_mem_buffer(MCPP_ERR);
	output.elapsedTime			= simpleTimer.GetElapsedTime();
	output.codeLength			= Strlen(output.code);
	output.errorStringLength	= Strlen(output.errorString);

	// Output processed file. (useful for debugging)
	if(input.outputProcessedFilename && output.code)
	{
		SaveStringToFile(input.outputProcessedFilename, output.code, output.codeLength);
	}

	// Output error string. (useful for debugging)
	if(input.outputErrorSringFilename)
	{
		if(output.errorString)
		{
			SaveStringToFile
				(input.outputErrorSringFilename, output.errorString, output.errorStringLength);
		}
		else
		{
			// No Error. Simply write the non-error confirmation and timing info.
			char sprintfBuffer[256] = {};
			int n = sprintf(sprintfBuffer, "Preprocessor finished with status: %d. Elapsed time: %d minutes, %.2f seconds.\n",
				output.returnStatus,
				output.elapsedTime.minutes,
				output.elapsedTime.seconds
				);
			///char c = '\n';
			SaveMemoryToFile(input.outputErrorSringFilename, sprintfBuffer, n);
		}
	}

	printf("Preprocessor finished with status: %d. Elapsed time: %d minutes, %.2f seconds.\n",
		output.returnStatus,
		output.elapsedTime.minutes,
		output.elapsedTime.seconds
	);

	// cleanup
	args.Free();

	return output;
}


int CppPreprocessor::FPutChar(int c, MCPP_OUTDEST od)
{
	return 0;
}


int CppPreprocessor::FPutString(const char* str, MCPP_OUTDEST od)
{
	return 0;
}


int CppPreprocessor::FPrintF(MCPP_OUTDEST od, const char * format, ...)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
/// Postprocessor
//////////////////////////////////////////////////////////////////////////


bool Postprocessor::GetNextToken(const char* str, u32 stringLength, u32& position, std::string& token)
{
	Assert(position <= stringLength);

	u32 tokenStart;
	u32 tokenEnd;

	tokenStart = FindFirstNonWhitespaceOrLE(str, stringLength, position);

	if(tokenStart < stringLength)
	{
		if(str[tokenStart] == '"' ) //string literal can contain delimiters
		{
			tokenEnd = tokenStart + 1;

			while(tokenEnd < stringLength && str[tokenEnd] != '"') //todo: what about '\"'... ?
				++tokenEnd;

			++tokenEnd;
		}
		else
		{
			tokenEnd = FindFirstWhitespaceOrLE(str, stringLength, tokenStart);
		}

		if(tokenEnd > stringLength)
			tokenEnd = stringLength;
	}
	else
	{
		position = stringLength;
		return false;
	}

	token.assign(str + tokenStart,str + tokenEnd);
	position = tokenEnd;

	return true;
}


bool Postprocessor::GetNextLine(const char* code, u32 codeLength, u32& index, int32& line, std::string& currentLine)
{
	u32 startPosition = index;

	while(index < codeLength)
	{
		if(code[index++] == '\n')
		{
			++line;
			currentLine.assign(&code[startPosition], index - startPosition);
			return true;
		}
	}

	return false; //eof
}


void Postprocessor::RemovePath(std::string& filepath)
{
	for(int32 i(int32(filepath.size() - 1)); i >= 0 ; --i)
	{
		const char c = filepath[i];

		if(c == '"')
		{
			filepath.erase(i, 1);
		}
		else if(c == '/')
		{
			filepath.erase(0, i + 1);
			break;
		}
		else if(c == '\\')
		{
			filepath.erase(0, i + 1);
			break;
		}
	}
}


void Postprocessor::RemoveLine(char* code, u32 codeLength, u32& index, bool removeLineEnding)
{
	// Todo: We can't actually remove the line without an expensive copy here so we just use whiteout.
	// In the future if a secondary destination buffer is used this can be corrected.

	while(index < codeLength && code[index] != '\n')
	{
		code[index] = ' ';
		++index;
	}

	if(removeLineEnding && index < codeLength && code[index] == '\n')
		code[index++] = ' ';
}


void Postprocessor::ProcessLineDirective(char* code, u32 codeLength, u32& index, std::string& filename, int32& outLineNumber)
{
	filename.clear();

	// get the line number
	GetNextToken(code, codeLength, index, m_noAllocToken);

	outLineNumber = atoi(m_noAllocToken.c_str());

	// get the filename
	GetNextToken(code, codeLength, index, filename);
	RemovePath(filename);
}


Array<ScriptFileData*> Postprocessor::ProcessPreprocessedMemoryFile
					(const char* scriptFilename, char* code, u32 codeLength)
{
	Array<ScriptFileData*> scriptSections;

	ScriptFileData* currentScript = new ScriptFileData();
	currentScript->filename = scriptFilename;
	RemovePath(currentScript->filename);

	scriptSections.Allocate(256);
	scriptSections.Add(currentScript);

	bool eof = false;
	int32 line = 1;
	u32 currentCodeIndex = 0;

	std::string token;
	std::string filename;
	std::string currentLineString;

	// Preprocessor 2nd pass: Clean up the preprocessed file and put together our script modules
	// so that line numbers match output error or debugging information.
	// (This is not technically required, so it may be possible to give
	//  the user the option via ZC runtime editing to disable this)

	while(!eof)
	{
		// process the directive, if any
		if(currentCodeIndex < codeLength && code[currentCodeIndex] == '#')
		{
			u32 tempPosition = currentCodeIndex + 1;
			GetNextToken(code, codeLength, tempPosition, token);

			// get info and append to scripts
			if(token == "line")
			{
				int32 outLineNumber = 0;

				ProcessLineDirective(code, codeLength, tempPosition, filename, outLineNumber);
				RemoveLine(code, codeLength, currentCodeIndex, (outLineNumber == 1));

				// Get current script section.
				u32 i = 0;
				for(; i < scriptSections.Size(); ++i)
				{
					if(scriptSections[i]->filename == filename)
					{
						currentScript = scriptSections[i];
						break;
					}
				}

				// Add new script section.
				if(i == scriptSections.Size())
				{
					currentScript = new ScriptFileData(); //todo
					currentScript->filename = filename;

					scriptSections.Add(currentScript);
				}

				int32 lineNumberDifference = outLineNumber - 1 - currentScript->line;
				if(lineNumberDifference > 0)
				{
					currentScript->code.append(lineNumberDifference, '\n');
					currentScript->line += lineNumberDifference;
				}
			}
			else if(token == "include")
			{
				//delete the current line
				RemoveLine(code, codeLength, currentCodeIndex, false);
			}
			else
			{
				printf("Unknown token on line %i!", line);
			}
		}

		// we can be assured any '#' info has already been resolved
		else if(GetNextLine(code, codeLength, currentCodeIndex, line, currentLineString))
		{
			currentScript->code += currentLineString;
			currentScript->line++;
		}
		else eof = true;

	}

	for(u32 i(0); i < scriptSections.Size(); ++ i)
	{
		u32 codeSectionLength = scriptSections[i]->code.length();
		if(codeSectionLength && scriptSections[i]->code[codeSectionLength - 1] != '\n')
		{
			scriptSections[i]->code += '\n';
		}
	}

	return scriptSections;
}


// simply returns the scripts for now.
// it's easy enough to return any additional information that may be required later, however.
Array<ScriptFileData*> GenerateScriptSectionsFromFile(const char* filename, u32 buildFlags)
{
	Array<ScriptFileData*> scripts;

	CppPreprocessor cppPreprocessor = {};
	CppPreprocessorInputArgs input = {};
	{
		input.filename = filename;
		input.outputErrorSringFilename = "error.log";

		if(buildFlags & kBuildGeneratePreprocessedFile)
			input.outputProcessedFilename = "output.txt";
	}

	cppPreprocessor.Init();

	// Output resources are destroyed once we:
	// a) Call Init(),
	// b) Process another file,
	// c) Call Shutdown().
	CppPreprocessorOutput output = cppPreprocessor.ProcessFile(input);

	if(output.returnStatus == 0)
	{
		Postprocessor postprocessor = Postprocessor();

		// There *shouldn't* be any errors here.
		// But it may be possible to return zero actual scripts.
		scripts = postprocessor.ProcessPreprocessedMemoryFile(input.filename, output.code, output.codeLength);

		if(!scripts.Empty())
		{
			if(buildFlags & kBuildOutputScriptSectionsToFiles)
			{
				_mkdir("OUTPUT/"); //error check?

				std::string outSSFilename;
				outSSFilename.reserve(64);

				for(u32 i(0); i < scripts.Size(); ++i)
				{
					ScriptFileData& script = *scripts[i];

					outSSFilename = "OUTPUT/";
					outSSFilename += script.filename;
					outSSFilename += ".txt";

					SaveStringToFile(outSSFilename.c_str(), script.code.c_str(), script.code.length());
				}
			}
		}

		//for(u32 i(0); i < scripts.Size(); ++i)
		//	delete scripts[i];

		//scripts.Free();
	}
	else // There were preprocessor errors
	{
		//cannot compile
	}

	cppPreprocessor.Shutdown();

	return scripts;
}


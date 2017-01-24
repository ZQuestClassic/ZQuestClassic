
#include "../Config.h"

#ifdef _MSC_VER
#pragma warning ( push )
#pragma warning ( disable : 151 241 138 ) //stupid dinkumware can't fucking write libraries worth shit.
#pragma warning ( disable : 4275 4996 ) //clib "may be unsafe" warnings.
#endif


#include "../Array.cpp"
#include "../Utility.cpp"
#include "../Preprocessor.cpp"
#include "../ScriptEngine.cpp"

#include <direct.h>

#ifdef _MSC_VER
#pragma warning ( pop )
#endif


int main()
{

	CppPreprocessor cppPreprocessor = {};
	CppPreprocessorInputArgs input = {};
	{
		input.filename = "../TestScript.cpp";
		input.outputErrorSringFilename = "error.log";
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

		PODArray<ScriptFileData*> scripts = 
			postprocessor.ProcessPreprocessedMemoryFile(input.filename, output.code, output.codeLength);

		_mkdir("OUTPUT/");
		{
			for(u32 i(0); i < scripts.Size(); ++i)
			{
				ScriptFileData script = *scripts[i];

				std::string fn = "OUTPUT/";
				fn += script.filename;
				fn += ".txt";

				SaveMemoryToFile(fn.c_str(), script.code.c_str(), script.code.length());
			}
		}

		for(u32 i(0); i < scripts.Size(); ++i)
			delete scripts[i];

		scripts.Free();

		return 0;
	}

	cppPreprocessor.Shutdown();


	return 1;
}



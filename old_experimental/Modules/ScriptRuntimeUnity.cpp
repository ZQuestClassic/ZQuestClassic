
#include "../ScriptCommon.h"

#ifdef _MSC_VER
#pragma warning ( push )
#pragma warning ( disable : 151 241 138 ) //stupid dinkumware can't fucking write libraries worth shit.
#pragma warning ( disable : 4275 4996 ) //clib "may be unsafe" warnings.
#endif

#include <direct.h>

//////////////////////////////////////////////////////////////////////////
#define ScriptString std::string

//////////////////////////////////////////////////////////////////////////

#include "../ScriptEngine.cpp"
#include "../ScriptContextPool.cpp"
#include "../ScriptRegistrar.cpp"

#ifdef _MSC_VER
#pragma warning ( pop )
#endif


int main()
{
	//lex();
	//ConvertShit("../asBindings/LegacyBindings.cpp");
	//return 0;

	InitScriptEngine();

	const char* filename = "../Scripts/TestObjects.h"; //"../Scripts/TestSyntax.h"; //"../Scripts/Test1.cs";
	bool r = LoadAndBuildScriptFile(filename);

	//////////////////////////////////////////////////////////////////////////
	//Todo: All the script messin' goes here.
	//////////////////////////////////////////////////////////////////////////

	if(r)
	{

		ScriptClassThunk thunk = {};
		foo(thunk);

		SimpleTimer timer;

		const int NUM_SCRIPTS = 1;
		Script script[NUM_SCRIPTS] = {};

		for(u32 i(0); i != NUM_SCRIPTS; ++i)
		{
			if(!script[i].Init(&thunk))
				Printf("-----------Init failed!------------");
		}

		int frames=0;
		int nd=0;
		timer.Start();

		CallGlobalCallbackFunction(0);
		CallGlobalCallbackFunction(1);

		while(nd < NUM_SCRIPTS)
		{
			for(u32 i(0); i != NUM_SCRIPTS; ++i)
			{
				if(script[i].OnUpdate() == asEXECUTION_EXCEPTION)
				{
					script[i].OnDestroy();
					nd++; //destroy object?
				}
			}

			frames++;
		}

		SMinuteSecondsInfo msi = timer.GetElapsedTime();
		Printf("Elapsed time: %i minutes, %.2f seconds.\n",
			msi.minutes,
			msi.seconds
			);

		Printf("frames: %i, %.5f.\n",
			frames,
			((msi.seconds + (float(msi.minutes * 60))) * 1000.0f) / float(frames)
			);

		for(u32 i(0); i != NUM_SCRIPTS; ++i)
		{
			script[i].Destroy();
		}


	} //r
	//////////////////////////////////////////////////////////////////////////
	ShutdownScriptEngine();

	return r ? 0 : 1;
}




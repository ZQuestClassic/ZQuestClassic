
#include "Preprocessor.h"
#include "ScriptEngine.h"
#include "Utility.h"
#include "Array.h"

extern void RegisterEverything(); //////////////////////////////////////////////////////////////////////////


void InitScriptEngine()
{
	Assert(!asScriptEngine);
	Assert(!asDefaultScriptModule);

	asScriptEngine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	asDefaultScriptModule = asScriptEngine->GetModule(NULL, asGM_CREATE_IF_NOT_EXISTS);

	asScriptEngine->SetMessageCallback(asFUNCTION(ScriptCompilerMessageCallback), 0, asCALL_CDECL);
	SetDefaultScriptEngineProperties(asScriptEngine);

	RegisterEverything(); //////////////////////////////////////////////////////////////////////////

	// todo: Optionally initialize contexts for 32 ffcs + 1 global script as per 2.50.
	scriptContextPool.Init(0);
}


void ShutdownScriptEngine()
{
	if(asScriptEngine)
	{
		ShutdownGlobalCallbackTable();
		scriptContextPool.Shutdown();

		//asDefaultScriptModule->Discard();
		asScriptEngine->ShutDownAndRelease();

		asScriptEngine = NULL;
		asDefaultScriptModule = NULL;
	}

}


//////////////////////////////////////////////////////////////////////////



#define MAX_SCRIPT_GLOBAL_CALLBACKS 64 //this value can be any size.

asIScriptFunction* globalScriptCallbackCache[MAX_SCRIPT_GLOBAL_CALLBACKS] = {};
//const char* globalScriptCallbackDeclarations[MAX_SCRIPT_GLOBAL_CALLBACKS] = {};

// todo: Allow the quest to say whether this should be called, or not.
// eg; scripts will dynamically set-up callbacks based on unknown factors.
void SetupDefaultGlobalCallbackTable(const char** funcDeclarations, u32 count)
{
	Assert(asScriptEngine);
	Assert(count <= MAX_SCRIPT_GLOBAL_CALLBACKS);

	for(u32 i(0); i != count; ++i)
	{
		globalScriptCallbackCache[i] = 
			asDefaultScriptModule->GetFunctionByDecl(funcDeclarations[i]);
	}
}


void ShutdownGlobalCallbackTable()
{
	// Must be called before shutting down the engine.
	Assert(asScriptEngine);

	for(u32 i(0); i < MAX_SCRIPT_GLOBAL_CALLBACKS; ++i)
	{
		if(globalScriptCallbackCache[i])
		{
			globalScriptCallbackCache[i]->Release();
			globalScriptCallbackCache[i] = NULL;
		}
	}
}

//
//void CallGlobalScriptFunction(asIScriptFunction* scriptFunction)
//{
//	asIScriptContext* tempContext = AquireReusableContext();
//	CallScriptFunction(tempContext, scriptFunction);
//}


void CallGlobalCallbackFunction(u32 id)
{
	asIScriptFunction* callbackFunction = globalScriptCallbackCache[id];
	if(callbackFunction)
	{
		asIScriptContext* scriptContext = AquireReusableContext();

		scriptContext->Prepare(callbackFunction);
		scriptContext->Execute(); // don't care about the result. exceptions are logged automatically.

		ReleaseReusableContext(scriptContext);
	}
}


bool CallGlobalCallbackFunctionWithArgs(u32 id, ScriptVariant* args, u32 argCount, ScriptVariant* returnValue)
{
	asIScriptFunction* callbackFunction = globalScriptCallbackCache[id];
	if(callbackFunction)
	{
		asIScriptContext* scriptContext = AquireReusableContext();
		Assert(scriptContext);

		int result = scriptContext->Prepare(callbackFunction);
		if(result == 0)
		{
			for(u32 i(0); i < argCount; ++i)
			{
				ScriptVariant& arg = args[i];
				int setResult = -1;

				if(arg.type == ScriptVariant::INT)
					setResult = scriptContext->SetArgWord(i, arg.u.i);

				if(arg.type == ScriptVariant::FLOAT)
					setResult = scriptContext->SetArgFloat(i, arg.u.f);

				if(arg.type == ScriptVariant::OBJECT)
					setResult = scriptContext->SetArgObject(i, arg.u.obj);

				if(setResult != 0)
				{
					const char* fs = callbackFunction->GetDeclaration();
					if(!fs)
						fs = "unknown";

					ErrorLog("Error: CallGlobalCallbackFunction incorrect argument %d. Function signature is \"%s\"\n", i, fs);

					ReleaseReusableContext(scriptContext);
					return false;
				}
			}

			result = scriptContext->Execute();
			if(result == asEXECUTION_FINISHED)
			{
				if(returnValue)
				{
					if(returnValue->type == ScriptVariant::INT)
						returnValue->u.i = scriptContext->GetReturnWord();

					if(returnValue->type == ScriptVariant::FLOAT)
						returnValue->u.f = scriptContext->GetReturnFloat();

					if(returnValue->type == ScriptVariant::OBJECT)
						returnValue->u.obj = scriptContext->GetReturnObject();
				}
			}
			else
			{
				ReleaseReusableContext(scriptContext);
				return false;
			}

			ReleaseReusableContext(scriptContext);
			return true;
		}
	}

	return false;
}



//////////////////////////////////////////////////////////////////////////




ScriptDelegateCallback callBackList[8] = {}; //arbitrary

void SetCallback(int32 callbackType, asIScriptFunction* callbackFunction)
{
	ScriptDelegateCallback& callback = callBackList[callbackType];

	if(callback.scriptFunction)
		callback.scriptFunction->Release();

	if(callback.object)
		asScriptEngine->ReleaseScriptObject(callback.object, callback.objectTypeInfo);

	if(callbackFunction->GetFuncType() == asFUNC_DELEGATE)
	{
		callback.scriptFunction = callbackFunction->GetDelegateFunction();
		callback.object = callbackFunction->GetDelegateObject();
		callback.objectTypeInfo = callbackFunction->GetDelegateObjectType();

		// Hold on to the object and method
		asScriptEngine->AddRefScriptObject(callback.object, callback.objectTypeInfo);
		callback.scriptFunction->AddRef();

		// Release the delegate, since it won't be used anymore
		callbackFunction->Release();
	}
	else
	{
		callback.scriptFunction = callbackFunction->GetDelegateFunction();
		callback.object = NULL;
		callback.objectTypeInfo = NULL;
	}
}


// Executes a script function of type "void f()" with error checking, exception logging, etc.
// The script context must be fully bound and prepared before it can be executed.
int ExecuteScriptContext(asIScriptContext* scriptContext)
{
	Assert(scriptContext);

	int result = scriptContext->Execute();
	switch(result)
	{
	case asEXECUTION_FINISHED:
		{
			//return params?
			break;
		}
	case asEXECUTION_SUSPENDED:
	case asEXECUTION_ABORTED:
		{
			break;
		}
	case asEXECUTION_EXCEPTION:
		{
			LogScriptException(scriptContext);
			break;
		}
	case asEXECUTION_PREPARED:
	case asEXECUTION_UNINITIALIZED:
	case asEXECUTION_ACTIVE:
	case asEXECUTION_ERROR:
		{
			break;
		}
	}

	return result;
}


int CallScriptFunction(asIScriptContext* scriptContext, asIScriptFunction* scriptFunction)
{
	Assert(scriptFunction);

	scriptContext->Prepare(scriptFunction);
	return ExecuteScriptContext(scriptContext);
}


int CallScriptFunctionByName(asIScriptContext* scriptContext, const char* functionDeclaration)
{
	asIScriptFunction* scriptFunction = asDefaultScriptModule->GetFunctionByDecl(functionDeclaration);
	if(scriptFunction)
		return CallScriptFunction(scriptContext, scriptFunction);

	return -1;
}


asIScriptObject* CreateScriptObject(asIScriptContext* scriptContext, asIScriptFunction* factoryFunction)
{
	scriptContext->Prepare(factoryFunction);
	scriptContext->Execute();

	asIScriptObject* scriptObject = *(asIScriptObject**)scriptContext->GetAddressOfReturnValue();
	if(scriptObject)
		scriptObject->AddRef();

	return scriptObject;
}


asIScriptObject* CreateScriptObjectByName(asIScriptContext* scriptContext, const char* className, const char* factoryName)
{
	asITypeInfo* typeInfo = asDefaultScriptModule->GetTypeInfoByDecl(className);
	asIScriptFunction* factoryFunction = typeInfo->GetFactoryByDecl(factoryName);

	return CreateScriptObject(scriptContext, factoryFunction);
}


void SetDefaultScriptEngineProperties(asIScriptEngine* engine)
{
	// asEP_COMPILER_WARNINGS
	//
	// Set how warnings should be treated: 0 - dismiss, 1 - emit, 2 - treat as error.
	//
	engine->SetEngineProperty(asEP_COMPILER_WARNINGS, 1);

	// asEP_AUTO_GARBAGE_COLLECT
	//
	// TODO: This needs serious testing later on to determine the performance implications.
	//
	engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, 0);

	// asEP_BUILD_WITHOUT_LINE_CUES
	//
	// Needed for debugging. Inserts suspend instructions after each statement.
	// (We don't have a debugger though :( ...)
	engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, 1);

	// asEP_MAX_STACK_SIZE
	//
	// Maximum stack size in bytes for script contexts. Default: 0 (no limit).
	// TODO: (This is for testing right now.)
	//
	engine->SetEngineProperty(asEP_MAX_STACK_SIZE, 1024 * 1024);

	// asEP_ALLOW_IMPLICIT_HANDLE_TYPES
	//
	// May be a language improvement in terms of simpler syntax, which is nice,
	// however it is not supported completely as of angelscript version "2.31.2."
	//
	engine->SetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES, 1);

	// asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE
	//
	// Disallow value assignment for reference types to avoid ambiguity. Default: false. 
	// *NOTE*
	// - angelscript references are like value-types and reference-types similar to C#.
	// - This is the 'gotcha' of the script language. Instead of just declaring a reference of a type
	// - and then 'pointing' to an object, it will perform a deep copy instead if the scripter is
	// - not careful. Probably the leading cause of silent bugs.
	//
	// (This needs to be fully evaluated.)
	//
	engine->SetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE, 1);

	engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 0); // Great optimization, but needs testing. Also unsafe.
	engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, 1);
	engine->SetEngineProperty(asEP_REQUIRE_ENUM_SCOPE, 1);
	engine->SetEngineProperty(asEP_ALLOW_MULTILINE_STRINGS, 1);
	engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, 1);

	//engine->SetEngineProperty(asEP_EXPAND_DEF_ARRAY_TO_TMPL, 1);
	//engine->SetEngineProperty(asEP_ALTER_SYNTAX_NAMED_ARGS, 1); // no idea about this.
}

  
void LogScriptException(asIScriptContext* scriptContext)
{
	if(scriptContext && scriptContext->GetState() == asEXECUTION_EXCEPTION)
	{
		asIScriptEngine* engine = scriptContext->GetEngine();
		asIScriptFunction* scriptFunction = scriptContext->GetExceptionFunction();

		ScriptLog("[Script Exception: \"%s\"]\n", scriptContext->GetExceptionString());
		ScriptLog("\tModule:  %s,\n", scriptFunction->GetModuleName() );
		ScriptLog("\tFuncion: %s,\n", scriptFunction->GetDeclaration() );
		ScriptLog("\tSection: %s,\n", scriptFunction->GetScriptSectionName() );
		ScriptLog("\tLine:    %d,\n", scriptContext->GetExceptionLineNumber() );
		//ScriptLog("\tDesc:    %s,\n", scriptContext->GetExceptionString() );

		asUINT callstackSize = scriptContext->GetCallstackSize();
		if(callstackSize > 0)
		{
			ScriptLog("\t[Call Stack]:\n");

			for(asUINT n = 1; n < callstackSize; ++n)
			{
				scriptFunction = scriptContext->GetFunction(n);

				if(scriptFunction)
				{
					if(scriptFunction->GetFuncType() == asFUNC_SCRIPT)
					{
						ScriptLog("\t%s, Line: %d, %s\n",
							scriptFunction->GetScriptSectionName() ? scriptFunction->GetScriptSectionName() : "",
							scriptContext->GetLineNumber(n),
							scriptFunction->GetDeclaration()
							);
					}
					else
					{
						// nested call
					}
				}
				else
				{
					// nested call
				}
			}
		}
	}
}


void ScriptCompilerMessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERROR";

	if(msg->type == asMSGTYPE_WARNING) 
	{
		type = "WARN ";
	}
	else if(msg->type == asMSGTYPE_INFORMATION) 
	{
		type = "INFO ";
	}

	CompileLog("%s [line: %d, %d] : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}



bool LoadAndBuildScriptFile(const char* filename,
							const char* moduleName,
							bool discardModule,
							bool buildModule
							)
{
	SimpleTimer totalCompileTime;

	asIScriptEngine* engine = asScriptEngine;

	if(buildModule)
	{
		// speedup since we own the buffers.
		engine->SetEngineProperty(asEP_COPY_SCRIPT_SECTIONS, 0);
	}
	else
	{
		// forced, else we have a memory corruption.
		engine->SetEngineProperty(asEP_COPY_SCRIPT_SECTIONS, 1);
	}

	totalCompileTime.Start();

	// Step 1: Load and generate AS compilable scripts.
	Array<ScriptFileData*> scripts = GenerateScriptSectionsFromFile(filename, 1 | 2);

	// TODO ///
	//////////////////////////////////////////////////////////////////////////
	// Optional: Discard current module and recompile.
	//if(discardModule)
	//	engine->DiscardModule(moduleName);

	//asIScriptModule* module = engine->GetModule(moduleName, asGM_CREATE_IF_NOT_EXISTS);
	asIScriptModule* module = asDefaultScriptModule;

	bool compileSuccess = true;

	// Step 2: Load and scripts into AS.
	if(!scripts.Empty())
	{
		for(u32 i(0); i < scripts.Size(); ++i)
		{
			ScriptFileData& script = *scripts[i];

			int result = module->AddScriptSection(script.filename.c_str(), script.code.c_str(), script.code.size(), 0);
			if(result < 0) 
			{
				compileSuccess = false;
				ScriptLog("Failed to compile file: %s.\n", script.filename.c_str());
			}
		}

		// Step 3: Build the damn things already!
		if(buildModule)
		{
			if(compileSuccess) // Unlikely but theoretical possibility this is false.
			{
				int buildResult = module->Build();
				if(buildResult < 0)
					compileSuccess = false;
			}
		}
	}

	// Cleanup
	for(u32 i(0); i < scripts.Size(); ++i)
		delete scripts[i];

	scripts.Free();

	SMinuteSecondsInfo msi = totalCompileTime.GetElapsedTime();

	CompileLog("Build finished with status: %i. Elapsed time: %i minutes, %.2f seconds.\n",
		compileSuccess ? 0 : 1,
		msi.minutes,
		msi.seconds
	);

	// Leftover error handling here.
	if(compileSuccess)
	{
		CompileLog("Build Succeeded.\n");
	}
	else
	{
		CompileLog("Build Failed.\n");
	}

	return compileSuccess;
}










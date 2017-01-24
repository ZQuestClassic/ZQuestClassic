
#include "ScriptEngine.h"



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


void CallScriptFunction(asIScriptContext* scriptContext, asIScriptFunction* scriptFunction)
{
	scriptContext->Prepare(scriptFunction);

	// todo: args?

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
}


void CallScriptFunctionByName(asIScriptContext* scriptContext, const char* functionDeclaration)
{
	asIScriptFunction* scriptFunction = asDefaultScriptModule->GetFunctionByDecl(functionDeclaration);
	CallScriptFunction(scriptContext, scriptFunction);
}


asIScriptObject* CreateScriptObject(asIScriptContext* scriptContext, asIScriptFunction* factoryFunction)
{
	scriptContext->Prepare(factoryFunction);
	scriptContext->Execute();

	asIScriptObject* scriptObject = *(asIScriptObject**)scriptContext->GetAddressOfReturnValue();
	scriptObject->AddRef();

	return scriptObject;
}


asIScriptObject* CreateScriptObjectByName(asIScriptContext* scriptContext, const char* className, const char* factoryName)
{
	asITypeInfo* typeInfo = asDefaultScriptModule->GetTypeInfoByDecl(className);
	asIScriptFunction* factoryFunction = typeInfo->GetFactoryByDecl(factoryName);

	return CreateScriptObject(scriptContext, factoryFunction);
}


void Waitframes(int32 frames)
{
	asIScriptContext *scriptContext = asGetActiveContext();
	if(scriptContext)
	{
		Script* activeScript = (Script*)scriptContext->GetUserData();
		if(activeScript)
		{
			activeScript->Suspend(frames);

			// Suspend the script object.
			// The script class will be notified and resume execution on a later frame.
			scriptContext->Suspend(); 
		}
		else
		{
			ScriptLog("Waitframes(%i) error dummy-head!", frames);
			//script-writer messed up.
		}
	}
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
	engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, 1);

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


	engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, 0); // Great optimization, but needs testing
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

		ScriptLog("[Script Exception]");
		ScriptLog("\tModule:  %s,", scriptFunction->GetModuleName() );
		ScriptLog("\tFuncion: %s,", scriptFunction->GetDeclaration() );
		ScriptLog("\tSection: %s,", scriptFunction->GetScriptSectionName() );
		ScriptLog("\tLine:    %d,", scriptContext->GetExceptionLineNumber() );
		ScriptLog("\tDesc:    %s,", scriptContext->GetExceptionString() );

		ScriptLog("\t[Call Stack]");

		for(asUINT n = 1; n < scriptContext->GetCallstackSize(); ++n)
		{
			scriptFunction = scriptContext->GetFunction(n);

			if(scriptFunction)
			{
				if(scriptFunction->GetFuncType() == asFUNC_SCRIPT)
				{
					ScriptLog("\t%s, Line: %d, %s",
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


void ScriptCompilerMessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERROR";

	if(msg->type == asMSGTYPE_WARNING) 
	{
		type = "WARN";
	}
	else if(msg->type == asMSGTYPE_INFORMATION) 
	{
		type = "INFO";
	}

	ScriptLog("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}














#pragma once

#include "Config.h"
#include "include/angelscript.h"


GLOBAL_PTR asIScriptEngine* asScriptEngine;
GLOBAL_PTR asIScriptModule* asDefaultScriptModule;


struct ScriptObjectInfo
{
	asIScriptFunction* factoryFunction;
	asIScriptFunction* updateFunction;
};

struct ScriptDelegateCallback
{
	asIScriptFunction* scriptFunction;
	void* object;
	asITypeInfo* objectTypeInfo;
};


void CallScriptFunction(asIScriptContext* scriptContext, asIScriptFunction* scriptFunction);
void CallScriptFunctionByName(asIScriptContext* scriptContext, const char* functionDeclaration);

asIScriptObject* CreateScriptObject(asIScriptContext* scriptContext, asIScriptFunction* factoryFunction);
asIScriptObject* CreateScriptObjectByName(asIScriptContext* scriptContext, const char* className, const char* factoryName);

void Waitframes(int32 frames);


void InitScriptEngine();
void ShutdownScriptEngine();

void SetDefaultScriptEngineProperties(asIScriptEngine* engine);
void LogScriptException(asIScriptContext* scriptContext);
void ScriptCompilerMessageCallback(const asSMessageInfo* msg, void* param);

bool LoadAndBuildScriptFile(const char* filename, const char* moduleName = NULL, bool discardModule = true, bool buildModule = true);





//wip
struct Script
{
	int32 waitFrames;
	int32 scriptStatus;
	void* object;
	asIScriptObject* scriptObject;
	asIScriptFunction* updateFunction;
	asIScriptContext* scriptContext;
	asITypeInfo* objectType;


	int32 Update()
	{
		asScriptEngine->GetObjectTypeByIndex(777); //todo
		updateFunction = asDefaultScriptModule->GetFunctionByDecl("void run()");

		if(scriptStatus == asEXECUTION_SUSPENDED)
		{
			if(--waitFrames > 0)
				return asEXECUTION_SUSPENDED;
		}
		else
		{
			scriptContext->Prepare(updateFunction);
		}

		scriptContext->SetObject(object);
		scriptStatus = scriptContext->Execute();

	}

	// Suspend execution of this object. eg; script->Waitframes(x);
	void Suspend(int32 frames)
	{
		waitFrames = uint32(frames);
	}

	void Destroy();

	asIScriptObject* GetScriptObject() const { return m_ScriptObject; }


protected:
	asIScriptObject* m_ScriptObject;

};




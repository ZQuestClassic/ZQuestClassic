
#pragma once

#include "ScriptCommon.h"
#include "ScriptContextPool.h"


GLOBAL_PTR asIScriptEngine* asScriptEngine;
GLOBAL_PTR asIScriptModule* asDefaultScriptModule;


struct ScriptDelegateCallback
{
	asIScriptFunction* scriptFunction;
	void* object;
	asITypeInfo* objectTypeInfo;
};


struct ScriptVariant
{
	enum Type
	{
		NONE,
		INT,
		FLOAT,
		OBJECT
	};

	Type type;

	union
	{
		int32 i;
		float f;
		void* obj;
	} u;

	void SetInt(int32 value) { type = INT; u.i = value; }
	void SetFloat(float value) { type = FLOAT; u.f = value; }
	void SetObject(void* value) { type = OBJECT; u.obj = value; }

	int32 GetInt() const { Assert(type == INT); return u.i; }
	float GetFloat() const { Assert(type == FLOAT); return u.f; }
	void* GetObject() const { Assert(type == OBJECT); return u.obj; }

};


//
//struct ScriptObjectInfo
//{
//	asIScriptFunction* factoryFunction;
//	asIScriptFunction* updateFunction;
//};

void CallGlobalCallbackFunction(u32 id);
bool CallGlobalCallbackFunctionWithArgs(u32 id, ScriptVariant* args, u32 argCount, ScriptVariant* returnValue = NULL);

//void CallGlobalScriptCallbackFunction(asIScriptFunction* scriptFunction);
int ExecuteScriptContext(asIScriptContext* scriptContext);
int CallScriptFunction(asIScriptContext* scriptContext, asIScriptFunction* scriptFunction);
int CallScriptFunctionByName(asIScriptContext* scriptContext, const char* functionDeclaration);

asIScriptObject* CreateScriptObject(asIScriptContext* scriptContext, asIScriptFunction* factoryFunction);
asIScriptObject* CreateScriptObjectByName(asIScriptContext* scriptContext, const char* className, const char* factoryName);


void InitScriptEngine();
void ShutdownScriptEngine();

void ShutdownGlobalCallbackTable(); //////////////////////////////////////////////////////////////////////////

void SetDefaultScriptEngineProperties(asIScriptEngine* engine);
void LogScriptException(asIScriptContext* scriptContext);
void ScriptCompilerMessageCallback(const asSMessageInfo* msg, void* param);

bool LoadAndBuildScriptFile(const char* filename, const char* moduleName = NULL, bool discardModule = true, bool buildModule = true);



struct ScriptObjectDelcInfo
{
	const char* decl;
	const char* factoryDecl;
};

struct ScriptObjectDeclAtomTable
{
	ScriptObjectDelcInfo entries[1024];

};

// Cached function pointers.
// (This is as fast as we can get script interop, however all thunks have to be reset whenever a (re)compile happens.)
struct ScriptClassThunk
{
	asIScriptFunction* factoryFunction; // Constructor. Same as "OnCreate()"
	asIScriptFunction* logicFunction;
	asIScriptFunction* drawFunction;
	asIScriptFunction* collideFunction;
	asIScriptFunction* destroyFunction;
	asITypeInfo* objectTypeInfo;
};


void foo(ScriptClassThunk& thunk)
{
	asITypeInfo* objectTypeInfo = asDefaultScriptModule->GetTypeInfoByDecl("GlobalScript1");

	if(objectTypeInfo)
	{
		thunk.factoryFunction = objectTypeInfo->GetFactoryByDecl("GlobalScript1 @GlobalScript1()");
		thunk.logicFunction = objectTypeInfo->GetMethodByDecl("void OnPigeonRaid()");
		thunk.drawFunction = objectTypeInfo->GetMethodByDecl("void OnDraw()");
		thunk.collideFunction = objectTypeInfo->GetMethodByDecl("void OnHit()");
		thunk.destroyFunction = objectTypeInfo->GetMethodByDecl("void OnDestroy()");
		thunk.objectTypeInfo = objectTypeInfo;
	}
}

//wip
struct Script
{
	int32 wait;
	int32 scriptStatus;
	asIScriptObject* scriptObject;
	asIScriptContext* scriptContext; // this script owns the context.
	ScriptClassThunk* pThunk; //invalidated on recompiles.

	/// Init returns false if no script object could be created.
	bool Init(ScriptClassThunk* thunkPtr)
	{
		Assert(thunkPtr);

		pThunk = thunkPtr;
		scriptContext = scriptContextPool.AquireContext();
		if(scriptContext)
		{
			scriptContext->SetUserData(this);
			scriptObject = CreateScriptObject(scriptContext, pThunk->factoryFunction);
			if(scriptObject)
			{
				// Set up everything else here so that update can be called without additional logic or state.
	//			scriptContext->SetObject(scriptObject); //?

				return true;
			}
		}

		//destroy ???
		return false;
	}

	void Destroy()
	{
		if(scriptObject)
		{
			scriptObject->Release(); // Release the object ref so it can be garbage collected.
			scriptObject = NULL;
		}

		if(scriptContext)
		{
			scriptContextPool.ReleaseContext(scriptContext);
			scriptContext->SetUserData(NULL);
		}
	}

	void OnDestroy()
	{
		if(scriptObject && pThunk->destroyFunction)
		{
			scriptContext->Prepare(pThunk->destroyFunction);
			scriptContext->SetObject(scriptObject);

			ExecuteScriptContext(scriptContext);
		}
	}

	int32 OnUpdate()
	{
		if(scriptObject)
		{
			if(scriptStatus == asEXECUTION_SUSPENDED) //flag instead?
			{
				if(--wait > 0)
					return asEXECUTION_SUSPENDED;

				//scriptContext->Execute();
			}
			else
			{
				scriptContext->Prepare(pThunk->logicFunction);
				scriptContext->SetObject(scriptObject); //?
			}

			//scriptContext->SetObject(scriptObject); //?
			scriptStatus = ExecuteScriptContext(scriptContext); //scriptContext->Execute();
		}

		return scriptStatus;
	}

	// Suspend execution of this object. eg; script->Waitframes(x);
	void Suspend(int32 frames)
	{
		scriptStatus = asEXECUTION_SUSPENDED;
		wait = u32(frames); // -1 = infinite (828 days and 13 hours is accurate enough.)
	}

	//asIScriptObject* GetScriptObject() const { return m_ScriptObject; }


protected:

};




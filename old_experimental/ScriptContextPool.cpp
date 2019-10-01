
#include "ScriptContextPool.h"

ScriptContextPool scriptContextPool;


asIScriptContext* ScriptContextPool::AquireContext()
{
	asIScriptContext* scriptContext = NULL;

	if(contextCount)
	{
		// Optimized to try and reuse the last (x) contexts used in FIFO order.
		// Returns the last used context to possibly avoid cold cache misses.
		scriptContext = contexts[--contextCount];
	}
	else
	{
		Assert(contextCount < MAX_SCRIPT_CONTEXTS); //todo

		totalAllocatedContextCount++;
		scriptContext = asScriptEngine->CreateContext();
	}

	return scriptContext;
}


void ScriptContextPool::ReleaseContext(asIScriptContext* scriptContext)
{
	Assert(scriptContext);
	Assert(contextCount < MAX_SCRIPT_CONTEXTS);

	scriptContext->Unprepare();
	contexts[contextCount++] = scriptContext;
}


void ScriptContextPool::ReleaseAllUnusedContexts()
{
	for(u32 i(0); i != contextCount; ++i)
	{
		contexts[i]->Release();
		contexts[i] = NULL;
	}

	totalAllocatedContextCount -= contextCount;
	contextCount = 0;
}


void ScriptContextPool::Init(u32 initialCapacity)
{
	Assert(asScriptEngine);
	Assert(!asGlobalReusableContext);

	asGlobalReusableContext = asScriptEngine->CreateContext();
	Assert(asGlobalReusableContext);

	// Set this once so that every global callback can use it without extra handling.
	asGlobalReusableContext->SetExceptionCallback(asFUNCTION(LogScriptException), NULL, asCALL_CDECL);

	for(u32 i(0); i < initialCapacity; ++i)
	{
		contexts[i] = asScriptEngine->CreateContext();
		Assert(contexts[i]);
	}

	contextCount = initialCapacity;
	totalAllocatedContextCount = initialCapacity;
}


void ScriptContextPool::Shutdown()
{
	asGlobalReusableContext->Release();
	asGlobalReusableContext = NULL;

	ReleaseAllUnusedContexts();

	// This will trigger if any contexts were leaked or were not returned properly.
	Assert(totalAllocatedContextCount == 0);
}


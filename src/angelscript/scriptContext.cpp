
#include "scriptContext.h"


asIScriptContext* ScriptContextPool::AquireContext()
{
	asIScriptContext* ptr = 0;

	if(contextCount)
	{
		// Return the last used context to possibly avoid cold cache misses.
		ptr = contexts[--contextCount];
	}
	else
	{
		//todo: context cap?

		totalAllocatedContextCount++;
		ptr = pEngine->CreateContext();
	}

	return ptr;
}


void ScriptContextPool::ReleaseContext(asIScriptContext* ptr)
{
	Assert(ptr);
	Assert(contextCount < MAX_SCRIPT_CONTEXTS);

	ptr->Unprepare();
	contexts[contextCount++] = ptr;
}


void ScriptContextPool::ReleaseAllUnusedContexts()
{
	for(uint32 i(0); i != contextCount; ++i)
	{
		contexts[i]->Release();
	}

	totalAllocatedContextCount -= contextCount;
}


void ScriptContextPool::Init(asIScriptEngine* engine, uint32 initialCapacity)
{
	Assert(engine);

	pEngine = engine;
	reusableContext = engine->CreateContext();

	for(uint32 i(0); i < initialCapacity; ++i)
	{
		contexts[i] = engine->CreateContext();
		contextCount = initialCapacity;
		totalAllocatedContextCount = initialCapacity;
	}
}


void ScriptContextPool::Destroy()
{
	reusableContext->Release();
	ReleaseAllUnusedContexts();

	// This will trigger if any contexts were leaked or were not returned properly.
	Assert(totalAllocatedContextCount == 0);
}


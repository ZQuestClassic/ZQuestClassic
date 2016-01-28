
#pragma once

#include <angelscript.h>
#include "config.h"


#define MAX_SCRIPT_CONTEXTS 512 // should be bigger later on.

// asScriptContext pool that caches script contexts for performance.
//
struct ScriptContextPool
{
	uint32 totalAllocatedContextCount;
	uint32 contextCount;
	asIScriptContext* contexts[MAX_SCRIPT_CONTEXTS];

	asIScriptContext* reusableContext;
	asIScriptEngine* pEngine;

	//   Context that is always available for general use as long as the following conditions are met:
	// - The script function that is called cannot call into the engine recursively through callbacks.
	// - The engine cannot store this context outside of function scope. (Nested callbacks would require a stack of reusable contexts)
	asIScriptContext* GetReusableContext()
	{
		Assert(reusableContext);
		return reusableContext;
	}

	/// Gets a free context from the pool.
	asIScriptContext* AquireContext();

	/// Returns a context to the pool. (This will unprepare the context automatically)
	void ReleaseContext(asIScriptContext* ptr);

	// todo: Not so much the ideal way of pruning the pool...
	void ReleaseAllUnusedContexts();

	void Init(asIScriptEngine* engine, uint32 initialCapacity);
	void Destroy();

};

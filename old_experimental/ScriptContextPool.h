
#pragma once

#include "Config.h"


#define MAX_SCRIPT_CONTEXTS 512 * 4 // should be bigger later on.

/// @asGlobalReusableContext
///   Context that is always available for general use as long as the following conditions are met:
/// - The script function that is called cannot call into the engine recursively through callbacks.
/// - The engine cannot store this context outside of function scope. (Nested callbacks would require a stack of reusable contexts)
GLOBAL_PTR asIScriptContext* asGlobalReusableContext;


asIScriptContext* AquireReusableContext()
{
	return asGlobalReusableContext;
}

void ReleaseReusableContext(asIScriptContext* ptr)
{
	Assert(ptr && ptr == asGlobalReusableContext);
}



class ScriptContextPool
{
public:
	/// Gets a free context from the pool.
	asIScriptContext* AquireContext();

	/// Returns a context to the pool. (This will unprepare the context automatically)
	void ReleaseContext(asIScriptContext* ptr);

	// todo: Not so much the ideal way of pruning the pool...
	void ReleaseAllUnusedContexts();

	void Init(u32 initialCapacity);
	void Shutdown();

protected:
	//u32 totalAllocatedContextCountOfProcess;
	u32 totalAllocatedContextCount;
	u32 contextCount;
	asIScriptContext* contexts[MAX_SCRIPT_CONTEXTS];
};

extern ScriptContextPool scriptContextPool;



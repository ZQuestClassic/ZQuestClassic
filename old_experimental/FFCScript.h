
#pragma once



void PrepareContextWithArgList(asIScriptContext* scriptContext, asIScriptFunction* f, int* args, u32 argCount)
{
	Assert(scriptContext);
	Assert(f);

	u32 paramCount = Min(argCount, f->GetParamCount());
	for(u32 i(0); i != paramCount; ++i)
	{
		scriptContext->SetArgWord(i, args[i]);
	}

	scriptContext->Prepare(f);
}


// For 2.5 item scripts.
void CallSingleFrameScript(const char* name, int* args, u32 argCount)
{
	asIScriptContext* scriptContext = AquireReusableContext();
	asIScriptFunction* f = asDefaultScriptModule->GetFunctionByName(name);

	if(scriptContext && f)
	{
		PrepareContextWithArgList(scriptContext, f, args, argCount);
		ExecuteScriptContext(scriptContext);
	}

	ReleaseReusableContext(scriptContext);
}


void CallItemPickupScript(const char* name, int* args, u32 argCount)
{
	CallSingleFrameScript(name, args, argCount); 
}


void CallItemOnUsageScript(const char* name, int* args, u32 argCount)
{
	CallSingleFrameScript(name, args, argCount); 
}


struct asFFCScript
{
	asIScriptContext* scriptContext;
	asIScriptFunction* runFunction;
	bool isRunning;

	// TODO: need ffc script name!
	void Init(int* args, u32 argCount)
	{
		scriptContext = scriptContextPool.AquireContext();

		if(scriptContext)
		{
			asITypeInfo* typeInfo = NULL; //fixme: get name here...
			runFunction = typeInfo->GetMethodByName("run");

			if(runFunction)
			{
				PrepareContextWithArgList(scriptContext, runFunction, args, argCount);
				isRunning = true;
			}
		}
	}

	void Run()
	{
		if(isRunning)
		{
			// todo: Waitframes.
			int result = scriptContext->Execute();

			if(result == asEXECUTION_SUSPENDED)
			{
				// Success! Restart the ffc script the next frame.
			}
			else
			{
				isRunning = false;
				LogScriptException(scriptContext);
			}
		}
	}
};




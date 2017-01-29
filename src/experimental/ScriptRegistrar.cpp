

struct ScriptRegistrar
{
	void RegisterScriptHandle(asIScriptEngine* engine);
	void RegisterMathFunctions(asIScriptEngine* engine);
	void RegisterVector2i(asIScriptEngine* engine);
	void RegisterVector2f(asIScriptEngine* engine);
	void RegisterRect(asIScriptEngine* engine);
	void RegisterMathDependencies(asIScriptEngine* engine); // Cross-dependencies between math types goes last.
	void RegisterGlobalFunctions(asIScriptEngine* engine); // These will need arrays and strings registered.
}
static ScriptRegistrar;


#include "asBindings/asHandle.hpp"
#include "asBindings/asMath.hpp"
#include "asBindings/asLink.hpp"
#include "asBindings/AddOn/scriptstdstring.cpp"
#include "asBindings/AddOn/scriptarray.cpp"



void RegisterEverything()
{
	Assert(asScriptEngine);
	ScriptRegistrar.RegisterScriptHandle(asScriptEngine);
	ScriptRegistrar.RegisterMathFunctions(asScriptEngine);
	ScriptRegistrar.RegisterVector2i(asScriptEngine);
	ScriptRegistrar.RegisterVector2f(asScriptEngine);
	ScriptRegistrar.RegisterRect(asScriptEngine);

	RegisterStdString(asScriptEngine);
	RegisterScriptArray_Native(asScriptEngine);

	// Dependants
	ScriptRegistrar.RegisterMathDependencies(asScriptEngine);
	ScriptRegistrar.RegisterGlobalFunctions(asScriptEngine);

}


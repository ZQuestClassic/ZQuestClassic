

struct ScriptRegistrar
{
	void RegisterMathFunctions(asIScriptEngine* engine);
	void RegisterVector2i(asIScriptEngine* engine);
	void RegisterVector2f(asIScriptEngine* engine);
	void RegisterRect(asIScriptEngine* engine);
	void RegisterMathDependencies(asIScriptEngine* engine); // Cross-dependencies between math types goes last.
	void RegisterGlobalFunctions(asIScriptEngine* engine);

}
static ScriptRegistrar;


#include "asBindings/asMath.hpp"
#include "asBindings/asLink.hpp"
#include "asBindings/AddOn/scriptstdstring.cpp"
#include "asBindings/AddOn/scriptarray.cpp"



void RegisterEverything()
{
	Assert(asScriptEngine);
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


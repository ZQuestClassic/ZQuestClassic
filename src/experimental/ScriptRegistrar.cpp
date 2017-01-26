

struct ScriptRegistrar
{
	void RegisterMathFunctions(asIScriptEngine* engine);
	//void RegisterRegisterVector2i(asIScriptEngine* engine);
	//void RegisterRegisterVector2f(asIScriptEngine* engine);
	//void RegisterMathDependencies(asIScriptEngine* engine); // Cross-dependencies between math types goes last.

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
	//ScriptRegistrar.RegisterRegisterVector2i(asScriptEngine);
	//ScriptRegistrar.RegisterRegisterVector2f(asScriptEngine);
	//ScriptRegistrar.RegisterMathDependencies(asScriptEngine);

	RegisterStdString(asScriptEngine);
	RegisterScriptArray_Native(asScriptEngine);

}


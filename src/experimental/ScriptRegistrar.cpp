

struct ScriptRegistrar
{
	void RegisterMathFunctions(asIScriptEngine* engine);


}
static ScriptRegistrar;


#include "asBindings/asMath.hpp"
#include "asBindings/asLink.hpp"



void RegisterEverything()
{
	Assert(asScriptEngine);
	ScriptRegistrar.RegisterMathFunctions(asScriptEngine);
}


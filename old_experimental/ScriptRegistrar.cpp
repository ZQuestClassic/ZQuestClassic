

struct ScriptRegistrar
{
	template <class T>
	void RegisterArraySpecialization(asIScriptEngine* engine, const char* decl, const char* type, const char* paramDecl);
	void RegisterArrayTemplateSpecializations(asIScriptEngine* engine);

	void RegisterScriptHandle(asIScriptEngine* engine);
	void RegisterMathFunctions(asIScriptEngine* engine);
	void RegisterVector2i(asIScriptEngine* engine);
	void RegisterVector2f(asIScriptEngine* engine);
	void RegisterRect(asIScriptEngine* engine);
	void RegisterMathDependencies(asIScriptEngine* engine); // Cross-dependencies between math types goes last.
	void RegisterGlobalFunctions(asIScriptEngine* engine); // These will need arrays and strings registered.

	void RegisterLegacyLink(asIScriptEngine* engine);

	//why not?
	void RegisterEverything();

}
static ScriptRegistrar;


#include "asBindings/asHandle.hpp"
#include "asBindings/asMath.hpp"
#include "asBindings/asLink.hpp"
#include "asBindings/AddOn/scriptstdstring.cpp"
#include "asBindings/AddOn/scriptarray.cpp"
#include "asBindings/asArrayTypes.hpp"



void ScriptRegistrar::RegisterEverything()
{
	Assert(asScriptEngine);

	RegisterScriptHandle(asScriptEngine);
	RegisterMathFunctions(asScriptEngine);
	RegisterVector2i(asScriptEngine);
	RegisterVector2f(asScriptEngine);
	RegisterRect(asScriptEngine);

	//todo:
	RegisterStdString(asScriptEngine);
	RegisterScriptArray_Native(asScriptEngine);

	RegisterArrayTemplateSpecializations(asScriptEngine);

	//todo:
	// Dependants
	RegisterMathDependencies(asScriptEngine);
	RegisterGlobalFunctions(asScriptEngine);

}

void RegisterEverything()
{
	ScriptRegistrar.RegisterEverything();
}

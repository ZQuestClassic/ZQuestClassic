

struct zcLink {};
zcLink zclink;

void RegisterLinkClass(asIScriptEngine* engine)
{
	//It makes sense to have Link a namespace in some ways..since you can't instantiate it..
	//Link.X += 2; ..Is there a performance difference between them?
	engine->SetDefaultNamespace("Link");

	//engine->RegisterObjectType("LinkClass", sizeof(zcLink), asOBJ_VALUE | asOBJ_POD | asOBJ_ABSTRACT);
	//engine->RegisterGlobalProperty("LinkClass Link", &zclink);
}





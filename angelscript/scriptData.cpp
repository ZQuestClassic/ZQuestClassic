#include "scriptData.h"

EntityScriptData::EntityScriptData(asIScriptObject* o, asIScriptContext* c):
    object(o),
    context(c)
{
}

EntityScriptData::~EntityScriptData()
{
    context->Release();
    object->Release();
}

asIScriptFunction* EntityScriptData::getFunction(const char* decl)
{
    return object->GetObjectType()->GetMethodByDecl(decl);
}

void EntityScriptData::runFunction(asIScriptFunction* func)
{
    context->Prepare(func);
    context->SetObject(object);
    context->Execute();
}

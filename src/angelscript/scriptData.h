#ifndef _ZC_ANGELSCRIPT_SCRIPTDATA_H_
#define _ZC_ANGELSCRIPT_SCRIPTDATA_H_

#include <angelscript.h>

// The script data and context.
// This class was created in anticipation of greater complexity.
// It doesn't do much at the moment.
class EntityScriptData
{
public:
    EntityScriptData(asIScriptObject* object, asIScriptContext* context);
    ~EntityScriptData();
    
    asIScriptFunction* getFunction(const char* decl);
    
    void runFunction(asIScriptFunction* func);
    
    // I'm not entirely certain this is safe in case of type mismatches
    // (e.g. a function takes a byte and is given a word), but it seems
    // to work just fine.
    template<typename T>
    void runFunction(asIScriptFunction* func, T t)
    {
        context->Prepare(func);
        context->SetObject(object);
        setArg(0, t);
        context->Execute();
    }
    
    template<typename T, typename U>
    void runFunction(asIScriptFunction* func, T t, U u)
    {
        context->Prepare(func);
        context->SetObject(object);
        setArg(0, t);
        setArg(1, u);
        context->Execute();
    }
    
    template<typename T>
    inline T getLastResult() const
    {
        return *reinterpret_cast<T*>(context->GetAddressOfReturnValue());
    }
    
    inline asIScriptObject* getObject()
    {
        return object;
    }
    
private:
    asIScriptObject* object;
    asIScriptContext* context;
    
    inline void setArg(int arg, int val)
    {
        context->SetArgDWord(arg, val);
    }
    
    inline void setArg(int arg, asIScriptObject* val)
    {
        context->SetArgObject(arg, val);
    }
};

#endif

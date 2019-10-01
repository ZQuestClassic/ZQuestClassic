
#pragma once


class CScriptHandle 
{
public:

	// Constructors
	CScriptHandle()
	{
		m_ref  = 0;
		m_type = 0;
	}

	CScriptHandle(const CScriptHandle &other)
	{
		m_ref  = other.m_ref;
		m_type = other.m_type;

		AddRefHandle();
	}

	CScriptHandle(void *ref, asITypeInfo *type)
	{
		m_ref  = ref;
		m_type = type;

		AddRefHandle();
	}

	CScriptHandle(void* ref, int typeId)
	{
		m_ref  = 0;
		m_type = 0;

		Assign(ref, typeId);
	}

	~CScriptHandle()
	{
		ReleaseHandle();
	}

	CScriptHandle& operator =(const CScriptHandle &other)
	{
		Set(other.m_ref, other.m_type);
		return *this;
	}

	void Set(void *ref, asITypeInfo *type)
	{
		if(m_ref == ref) return;

		ReleaseHandle();

		m_ref  = ref;
		m_type = type;

		AddRefHandle();
	}

	bool operator ==(const CScriptHandle &otherHandle) const
	{
		if(m_ref == otherHandle.m_ref && m_type == otherHandle.m_type)
			return true;

		// TODO: If type is not the same, we should attempt to do a dynamic cast,
		//       which may change the pointer for application registered classes

		return false;
	}

	bool CScriptHandle::operator!=(const CScriptHandle &o) const
	{
		return !(*this == o);
	}

	bool Equals(void *ref, int typeId) const
	{
		// Null handles are received as reference to a null handle
		if(typeId == 0)
			ref = 0;

		// Dereference handles to get the object
		if(typeId & asTYPEID_OBJHANDLE)
		{
			// Compare the actual reference
			ref = *(void**)ref;
			typeId &= ~asTYPEID_OBJHANDLE;
		}

		// TODO: If typeId is not the same, we should attempt to do a dynamic cast, 
		//       which may change the pointer for application registered classes

		if(ref == m_ref)
			return true;

		return false;
	}


	void* GetRef() const
	{
		return m_ref;
	}

	asITypeInfo* GetType() const
	{
		return m_type;
	}

	int CScriptHandle::GetTypeId() const
	{
		if(m_type == 0)
			return 0;

		return m_type->GetTypeId() | asTYPEID_OBJHANDLE;
	}

	// Dynamic cast to desired handle type
	// AngelScript: used as '@obj = cast<obj>(ref);'
	void Cast(void **outRef, int typeId)
	{
		// If we hold a null handle, then just return null
		if(m_type == 0)
		{
			*outRef = 0;
			return;
		}

		// It is expected that the outRef is always a handle
		Assert(typeId & asTYPEID_OBJHANDLE);

		// Compare the type id of the actual object
		typeId &= ~asTYPEID_OBJHANDLE;
		asITypeInfo *type = asScriptEngine->GetTypeInfoById(typeId);

		*outRef = 0;

		// RefCastObject will increment the refCount of the returned object if successful
		asScriptEngine->RefCastObject(m_ref, m_type, type, outRef);
	}

	//void Construct(CScriptHandle *self, void *ref, int typeId);

	void ReleaseHandle()
	{
		if(m_ref && m_type)
		{
			asScriptEngine->ReleaseScriptObject(m_ref, m_type);
			//engine->Release();

			m_ref  = 0;
			m_type = 0;
		}
	}

	void AddRefHandle()
	{
		if(m_ref && m_type)
		{
			asScriptEngine->AddRefScriptObject(m_ref, m_type);

			// Hold on to the engine so it isn't destroyed while
			// a reference to a script object is still held
			//engine->AddRef();
		}
	}

	CScriptHandle& Assign(void *ref, int typeId)
	{
		// When receiving a null handle we just clear our memory
		if(typeId == 0)
		{
			Set(0, 0);
			return *this;
		}

		// Dereference received handles to get the object
		if(typeId & asTYPEID_OBJHANDLE)
		{
			// Store the actual reference
			ref = *(void**)ref;
			typeId &= ~asTYPEID_OBJHANDLE;
		}

		// Get the object type
		asITypeInfo *type = asScriptEngine->GetTypeInfoById(typeId);

		// If the argument is another CScriptHandle, we should copy the content instead
		if(type && strcmp(type->GetName(), "ref") == 0)
		{
			CScriptHandle *r = (CScriptHandle*)ref;
			ref  = r->m_ref;
			type = r->m_type;
		}

		Set(ref, type);

		return *this;
	}

protected:
	void        *m_ref;
	asITypeInfo *m_type;
};




namespace ScriptBindings
{
	void ConstructHandle(CScriptHandle *self) { new(self) CScriptHandle(); }
	void ConstructHandle(CScriptHandle *self, const CScriptHandle &o) { new(self) CScriptHandle(o); }
	void ConstructHandle(CScriptHandle *self, void *ref, int typeId) { new(self) CScriptHandle(ref, typeId); }
	void DestructHandle(CScriptHandle *self) { self->~CScriptHandle(); }
}

void ScriptRegistrar::RegisterScriptHandle(asIScriptEngine *engine)
{
	int r;
	using namespace ScriptBindings;

	r = engine->RegisterObjectType("ref", sizeof(CScriptHandle), asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_APP_CLASS_CDAK); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(ConstructHandle, (CScriptHandle *), void), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f(const ref &in)", asFUNCTIONPR(ConstructHandle, (CScriptHandle *, const CScriptHandle &), void), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f(const ?&in)", asFUNCTIONPR(ConstructHandle, (CScriptHandle *, void *, int), void), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_DESTRUCT, "void f()", asFUNCTIONPR(DestructHandle, (CScriptHandle *), void), asCALL_CDECL_OBJFIRST); Assert(r >= 0);
	r = engine->RegisterObjectMethod("ref", "void opCast(?&out)", asMETHODPR(CScriptHandle, Cast, (void **, int), void), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("ref", "ref &opHndlAssign(const ref &in)", asMETHOD(CScriptHandle, operator=), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("ref", "ref &opHndlAssign(const ?&in)", asMETHOD(CScriptHandle, Assign), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("ref", "bool opEquals(const ref &in) const", asMETHODPR(CScriptHandle, operator==, (const CScriptHandle &) const, bool), asCALL_THISCALL); Assert(r >= 0);
	r = engine->RegisterObjectMethod("ref", "bool opEquals(const ?&in) const", asMETHODPR(CScriptHandle, Equals, (void*, int) const, bool), asCALL_THISCALL); Assert(r >= 0);
}





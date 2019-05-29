
#include "asScriptTemplate.h"


AS_NAMESPACE_BEGIN




ScriptTemplateClass::ScriptTemplateClass( asIObjectType *objectType )
	{
		_refCount		= 1;
		_gcFlag			= false;
		_objType		= objectType;
		_typeId			= _objType->GetTypeId();
		_subtypeId		= _objType->GetSubTypeId();
		_isHandle		= _subtypeId & asTYPEID_OBJHANDLE ? true : false;
		_isMaskObject	= _subtypeId & asTYPEID_MASK_OBJECT ? true : false;	

		_objType->AddRef();

		//save the primitive type for fast lookup
		if( _subtypeId == asTYPEID_BOOL || _subtypeId == asTYPEID_INT8 || _subtypeId == asTYPEID_UINT8 ) _valueType = 1;
		else if( _subtypeId == asTYPEID_INT16 || _subtypeId == asTYPEID_UINT16 ) _valueType = 2;
		else if( _subtypeId == asTYPEID_INT32 || _subtypeId == asTYPEID_UINT32 || _subtypeId == asTYPEID_FLOAT ) _valueType = 3;
		else if( _subtypeId == asTYPEID_INT64 || _subtypeId == asTYPEID_UINT64 || _subtypeId == asTYPEID_DOUBLE ) _valueType = 4;
		else _valueType = 0;


		if( _isMaskObject )
		{
			_needsStorage = _isHandle;
			_elementSize = sizeof(asPWORD);
		}
		else
		{
			_needsStorage = true;
			_elementSize = _objType->GetEngine()->GetSizeOfPrimitiveType( _subtypeId );
		}


		// Notify the GC of the successful creation
		if( _objType->GetFlags() & asOBJ_GC )
			_objType->GetEngine()->NotifyGarbageCollectorOfNewObject( this, _typeId );
	}


ScriptTemplateClass::~ScriptTemplateClass()
	{
		if( _objType )
			_objType->Release();
	}


ScriptTemplateClass &ScriptTemplateClass::operator = ( const ScriptTemplateClass &other )
	{
		// Only perform the copy if the array types are the same
		if( &other != this && other.GetArrayObjectType() == GetArrayObjectType() )
		{

		}

		return *this;
	}

/*
void *ScriptTemplateClass::AquirePtr()
	{
		if( _needsStorage )
			return (void*)new ubyte[ _elementSize ];

		return 0;
	}


void ScriptTemplateClass::ReleasePtr( void *_Ptr )
	{
		if( _needsStorage && _Ptr != 0 )
			delete [] (ubyte*)_Ptr;
	}
*/


void* ScriptTemplateClass::GetReturnObjectPtr( storage_type *_Ptr )
{
	return (void*)_Ptr;
}


void ScriptTemplateClass::SetValue( storage_type *_Ptr, void *value )
{
	if( (_subtypeId & ~0x03FFFFFF) && !(_isHandle) )
	{
		_objType->GetEngine()->CopyScriptObject( _Ptr->ptr, value, _subtypeId );
	}

	else if( _isHandle )
	{
		*(void**)_Ptr = *(void**)value;
		_objType->GetEngine()->AddRefScriptObject(*(void**)value, _subtypeId);
	}

	else
	{
		switch( _valueType )
		{
			case 0:
				throw("Unknown type");
				break;
			case 1: 
				*(char*)_Ptr = *(char*)value;
				break;
			case 2: 
				*(short*)_Ptr = *(short*)value;
				break;
			case 3: 
				*(int*)_Ptr = *(int*)value;
				break;
			case 4: // 64 bit depricated
				throw("Unsupported exception");
				*(double*)_Ptr = *(double*)value;
				break;
		}
	}

}


void ScriptTemplateClass::ConstructObject( storage_type *_Ptr )
	{
		if( _isHandle ) //_needsStorage ?
			//*_Ptr = memset( *_Ptr, 0, _elementSize );
			_Ptr->ptr = 0;


		else if( _isMaskObject )
		{
			_Ptr->ptr = _objType->GetEngine()->CreateScriptObject(_subtypeId);
		}
	}


void ScriptTemplateClass::DestructObject( storage_type *_Ptr )
	{
		if( _isHandle )
			//_objType->GetEngine()->ReleaseScriptObject( *(void**)_Ptr->ptr, _subtypeId );
			_objType->GetEngine()->ReleaseScriptObject( _Ptr->ptr, _subtypeId );

		else if( _isMaskObject )
		{
			_objType->GetEngine()->ReleaseScriptObject( _Ptr->ptr, _subtypeId );
		}
	}


asIObjectType *ScriptTemplateClass::GetArrayObjectType() const
	{
		return _objType;
	}


int ScriptTemplateClass::GetArrayTypeId() const
	{
		return _objType->GetTypeId();
	}


int ScriptTemplateClass::GetElementTypeId() const
	{
		return _objType->GetSubTypeId();
	}


void ScriptTemplateClass::AddRef() const
	{
		// Clear the GC flag then increase the counter
		_gcFlag = false;
		_refCount++;
	}


void ScriptTemplateClass::Release() const
	{
		// Now do the actual releasing (clearing the flag set by GC)
		_gcFlag = false;
		if( --_refCount == 0 )
		{
			delete this;
		}
	}


int ScriptTemplateClass::GetRefCount()
	{
		return _refCount;
	}


void ScriptTemplateClass::SetFlag()
	{
		_gcFlag = true;
	}


bool ScriptTemplateClass::GetFlag()
	{
		return _gcFlag;
	}









AS_NAMESPACE_END



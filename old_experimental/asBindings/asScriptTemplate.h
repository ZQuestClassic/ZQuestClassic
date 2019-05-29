
#pragma once

#include <angelscript.h>


AS_NAMESPACE_BEGIN





class ScriptTemplateClass
{
public:
	struct InternalStorageValueType32
	{
		InternalStorageValueType32( int val = 0 ) : value(val) {}
		union
		{
			int		value;
			float	real;
			void*	ptr;
		};
	};

	typedef InternalStorageValueType32 storage_type;


	ScriptTemplateClass( asIObjectType *objectType );
	virtual ~ScriptTemplateClass();

	ScriptTemplateClass &operator = ( const ScriptTemplateClass& );

	void*			AquirePtr();
	void			ReleasePtr( void *_Ptr );

	void*			GetReturnObjectPtr( storage_type *_Ptr );
	void			ConstructObject( storage_type *_Ptr );
	void			DestructObject( storage_type *_Ptr );
	void			SetValue( storage_type *_Ptr, void *value );

	// GC methods
	int				GetRefCount();
	void			SetFlag();
	bool			GetFlag();
	virtual void	EnumReferences( asIScriptEngine *engine ) = 0;
	virtual void	ReleaseAllHandles( asIScriptEngine *engine ) = 0;

	void			AddRef() const;
	void			Release() const;


	asIObjectType*	GetArrayObjectType() const;
	int				GetArrayTypeId() const;
	int				GetElementTypeId() const;

protected:
	mutable int		_refCount;
	mutable bool	_gcFlag;
	bool			_isHandle;
	bool			_isMaskObject;
	bool			_needsStorage;
	int				_valueType;
	int				_typeId;
	int				_subtypeId;
	int				_elementSize;
	asIObjectType *	_objType;

};






AS_NAMESPACE_END


#endif


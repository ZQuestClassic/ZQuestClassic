#if defined (_MSC_VER) && defined (USING_ARRAY)
#define ZCArray Array
#define ZCArrayIterator ArrayIterator
#endif

#ifndef __zc_array_h_
#define __zc_array_h_

#include "base/zdefs.h"
#include <sstream>
#include <functional>
//#define _DEBUGZCARRAY


template <typename T> class ZCArrayIterator;
template <typename T> class ZCArray;

template <class T>
class ZCArrayIterator
{
public:
    friend class ZCArray<T>;
    typedef uint32_t size_type;
    typedef const T& const_reference;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef T* pointer;
    
    ZCArrayIterator() : _ptr(0) {}
    ZCArrayIterator(T *_Tptr) : _ptr(_Tptr) {}
    ZCArrayIterator(const ZCArrayIterator& _It) : _ptr(_It._ptr) {}
    ~ZCArrayIterator() {}
    
    bool operator == (const ZCArrayIterator& v) const
    {
        return (_ptr == v._ptr);
    }
    bool operator != (const ZCArrayIterator& v) const
    {
        return (_ptr != v._ptr);
    }
    bool operator < (const ZCArrayIterator& v) const
    {
        return (_ptr <  v._ptr);
    }
    bool operator > (const ZCArrayIterator& v) const
    {
        return (_ptr >  v._ptr);
    }
    bool operator <= (const ZCArrayIterator& v) const
    {
        return (_ptr <= v._ptr);
    }
    bool operator >= (const ZCArrayIterator& v) const
    {
        return (_ptr >= v._ptr);
    }
    
    pointer		     operator ->()
    {
        return &(*_ptr);
    }
    const_pointer    operator ->() const
    {
        return &(*_ptr);
    }
    reference		 operator  *()
    {
        return (*_ptr);
    }
    const_reference  operator  *() const
    {
        return (*_ptr);
    }
    reference		 operator [](size_type _Index)
    {
        return(*(_ptr + _Index));
    }
    const_reference  operator [](size_type _Index) const
    {
        return(*(_ptr + _Index));
    }
    
    ZCArrayIterator &operator ++ ()
    {
        ++_ptr;
        return *this;
    }
    ZCArrayIterator &operator -- ()
    {
        --_ptr;
        return *this;
    }
    ZCArrayIterator  operator ++ (int32_t)
    {
        pointer _Tmp = _ptr;
        _ptr++;
        return ZCArrayIterator(_Tmp);
    }
    ZCArrayIterator  operator -- (int32_t)
    {
        pointer _Tmp = _ptr;
        _ptr--;
        return ZCArrayIterator(_Tmp);
    }
    
    ZCArrayIterator &operator += (int32_t _Offset)
    {
        _ptr += _Offset;
        return *this;
    }
    ZCArrayIterator &operator -= (int32_t _Offset)
    {
        _ptr -= _Offset;
        return *this;
    }
    ZCArrayIterator  operator + (int32_t _Offset) const
    {
        return ZCArrayIterator(_ptr + _Offset);
    }
    ZCArrayIterator  operator - (int32_t _Offset) const
    {
        return ZCArrayIterator(_ptr - _Offset);
    }
    
protected:
    pointer _ptr;
};

template <typename T>
class ZCArray
{
public:
    friend class ZCArrayIterator<T>;
    typedef const ZCArrayIterator<T> const_iterator;
    typedef ZCArrayIterator<T> iterator;
    typedef uint32_t size_type;
    typedef const T& const_reference;
    typedef const T* const_pointer;
    typedef const T const_type;
    typedef T& reference;
    typedef T* pointer;
    typedef T type;
    
    ZCArray() : _ptr(NULL), _size(0), _valid(false), _object_type(false)
    {
        for(int32_t i = 0; i < 4; i++)
            _dim[i] = 0;
    }
    
    ZCArray(size_type _Size) : _ptr(NULL), _valid(false), _object_type(false)
    {
        _SetDimensions(0, 0, _Size);
        _Alloc(_size);
    }
    
    ZCArray(size_type _Y, size_type _X) : _ptr(NULL), _valid(false), _object_type(false)
    {
        _SetDimensions(0, _Y, _X);
        _Alloc(_size);
    }
    
    ZCArray(size_type _Z, size_type _Y, size_type _X) : _ptr(NULL), _valid(false), _object_type(false)
    {
        _SetDimensions(_Z, _Y, _X);
        _Alloc(_size);
    }
    
    ZCArray(const ZCArray &_Array) : _ptr(NULL), _size(0), _valid(false)
    {
        for(int32_t i = 0; i < 4; i++) _dim[i] = 0;
        
        Copy(_Array);
    }
    
    ~ZCArray()
    {
        _Delete();
    }
    
    const ZCArray &operator = (const ZCArray &_Array)
    {
        if(this != &_Array)
            Copy(_Array);
        return *this;
    }
    
    bool operator == (const ZCArray &_Array) const
    {
        if(_size != _Array._size)
            return false;
		if(_valid != _Array._valid)
			return false;
		if(_object_type != _Array._object_type)
			return false;
            
        for(size_type i(0); i < _size; i++)
            if(*(_ptr + i) != *(_Array._ptr + i))
                return false;
                
        return true;
    }
    
    bool operator != (const ZCArray &right) const
    {
        return !(*this == right);
    }
    
    reference operator()(size_type _X)
    {
        return _ptr[ _X ];
    }
    reference operator()(size_type _Y, size_type _X)
    {
        return _ptr[ _X + _Y * _dim[0] ];
    }
    reference operator()(size_type _Z, size_type _Y, size_type _X)
    {
        return _ptr[ _X + _Y * _dim[0] + _Z * _dim[3] ];
    }
    
    const_reference operator()(size_type _X) const
    {
        return _ptr[ _X ];
    }
    const_reference operator()(size_type _Y, size_type _X) const
    {
        return _ptr[ _X + _Y * _dim[0] ];
    }
    const_reference operator()(size_type _Z, size_type _Y, size_type _X) const
    {
        return _ptr[ _X + _Y * _dim[0] + _Z * _dim[3] ];
    }
    
    
    pointer		 operator ->()
    {
        return &(*_ptr);
    }
    const_pointer	 operator ->() const
    {
        return &(*_ptr);
    }
    type			 operator * ()
    {
        return *_ptr;
    }
    const_type 	 operator * () const
    {
        return *_ptr;
    }
    reference		 operator [](const size_type& i)
    {
        return _ptr[i];
    }
    const_reference operator [](const size_type& i) const
    {
        return _ptr[i];
    }
	
	std::vector<T> getVec() const
	{
		std::vector<T> vec;
		for(auto q = 0; q < _size; ++q)
			vec.push_back(_ptr[q]);
		return vec;
	}
    
	std::string asString(std::function<char const*(int32_t)> formatter, const size_type& limit) const
	{
		std::ostringstream oss;
		oss << "{ ";
		size_type s = _size;
		bool overflow = limit < s;
		if(overflow)
			s = limit;
		
		for(auto q = 0; q < s; ++q)
		{
			oss << formatter(_ptr[q]);
            if (q + 1 < s)
                oss << ", ";
		}
        if (overflow)
            oss << ", ...";
		oss << " }";
		return oss.str();
	}
	
    reference At(size_type _X)
    {
        if(!_Bounds(_X))
            return _ptr[ 0 ]; //Must be a better way of returning an invalied element in a template than this...
            
        return _ptr[ _X ];
    }
    reference At(const size_type& _Y, const size_type& _X)
    {
        if(!_Bounds(_X,_Y))
            return _ptr[ 0 ];
            
        return _ptr[ Offset(_Y, _X) ];
    }
    reference At(const size_type& _Z, const size_type& _Y, const size_type& _X)
    {
        if(!_BoundsExp(_X,_Y,_Z))
            return _ptr[ 0 ];
            
        return _ptr[ Offset(_Z, _Y, _X) ];
    }
    
    reference StrictAt(size_type _X)
    {
        _BoundsExp(_X);
        return _ptr[ _X ];
    }
    reference StrictAt(size_type _Y, size_type _X)
    {
        _BoundsExp(_X,_Y);
        return _ptr[ Offset(_Y, _X) ];
    }
    reference StrictAt(size_type _Z, size_type _Y, size_type _X)
    {
        _BoundsExp(_X,_Y,_Z);
        return _ptr[ Offset(_Z, _Y, _X) ];
    }
    
    reference Front()
    {
        return *_ptr;
    }
    reference Back()
    {
        return _ptr[ _size - 1 ];
    }
    const_reference Front() const
    {
        return *_ptr;
    }
    const_reference Back() const
    {
        return _ptr[ _size - 1 ];
    }
    
    size_type Offset(const size_type& _Z, const size_type& _Y, const size_type& _X) const
    {
        return (_X + _Y * _dim[0] + _Z * _dim[3]);
    }
    size_type Offset(const size_type& _Y, const size_type& _X) const
    {
        return (_X + _Y * _dim[0]);
    }
    
    iterator Begin()
    {
        return iterator(_ptr);
    }
    iterator End()
    {
        return iterator(_ptr + _size);
    }
    const_iterator Begin()	const
    {
        return const_iterator(_ptr);
    }
    const_iterator End()	const
    {
        return const_iterator(_ptr + _size);
    }
    
    size_type Size() const
    {
        return _size;
    }
    bool Empty() const
    {
        return (_size == 0);
    }
	bool Valid() const
	{
		return _valid;
	}
	void setValid(bool v)
	{
		_valid = v;
	}
	void setObjectType(bool v)
	{
		_object_type = v;
	}
	bool ObjectType() const
	{
		return _object_type;
	}
    
    size_type Offset(const size_type _Z, const size_type _Y, const size_type _X) const
    {
        return (_X + _Y * _dim[0] + _Z * _dim[3]);
    }
    size_type Offset(const size_type _Y, const size_type _X) const
    {
        return (_X + _Y * _dim[0]);
    }
    
    void Assign(const size_type _Begin, const size_type _End, const type& _Val = type())
    {
        for(size_type i(_Begin); i < _End; i++)
            _ptr[ i ] = _Val;
    }
    
    void Resize(const size_type _Size)
    {
        Resize(0, 0, _Size);
    }
    void Resize(const size_type _Y, const size_type _X)
    {
        Resize(0, _Y, _X);
    }
    void Resize(const size_type _Z, const size_type _Y, const size_type _X)
    {
		if(_SameDimensions(_Z,_Y,_X)) return;
        const size_type _OldSize = _size;
        const size_type _NewSize = _GetSize(_Z, _Y, _X);
        
        _SetDimensions(_Z, _Y, _X);
        
        if(_NewSize == 0)
        {
            _Delete();
            _size = _NewSize;
        }
        else if(_OldSize != _NewSize)
            _ReAssign(_OldSize, _NewSize);
    }
    
	void Push(type val, int indx = -1)
	{
		++_dim[0];
		_ReAssign(_size, _size+1, indx, val);
	}
	
	type Pop(int indx = -1)
	{
		type ret = _ptr[(indx<0||indx>=_size) ? _size-1 : indx];
		--_dim[0];
		_ReAssign(_size, _size-1, indx);
		return ret;
	}
	
	
    void Copy(const ZCArray &_Array)
    {
        if(_Array.Empty())
        {
            Clear();
			setValid(_Array.Valid());
            return;
        }
		setValid(_Array.Valid());
        
#ifdef _DEBUGZCARRAY
        
        // _size should always be 0 if _ptr is null...so this shouldn't ever happen
        if((!_ptr && _size > 0) || (!_Array._ptr && _Array._size > 0))
        {
            al_trace("ZCArray copy op error!!!: -lhs p: %p -size: %u -rhs p: %p -size: %u\n",
                     (void*)_ptr, _size, (void*)_Array._ptr, _Array._size);
            _size = _Array.Size() + 1;
        }
        
#endif
        
        if(_size != _Array.Size())
            _Alloc(_Array.Size());
            
        for(int32_t i = 0; i < 4; i++)
            _dim[ i ] = _Array._dim[ i ];
            
        for(size_type i(0); i < _size; i++)
            _ptr[ i ] = _Array._ptr[ i ];

		setObjectType(_Array.ObjectType());
    }
    
    void GetDimensions(size_type _4dim[]) const
    {
        _4dim[0] = _dim[0];
        _4dim[1] = _dim[1];
        _4dim[2] = _dim[2];
        _4dim[3] = _dim[3];
    }
    
    void Clear()
    {
        Resize(0);
		_valid = false;
		_object_type = false;
    }
    
    
protected:

    void _Alloc(size_type size)
    {
    
#ifdef _DEBUGZCARRAY
        al_trace("Memory to allocate: %i\n", size);
#endif
        
        if(_ptr)
            _Delete();
            
        if(size == 0)
        {
            al_trace("Tried to allocate zero sized array\n");
#ifdef _DEBUGZCARRAY
            throw("Cannot allocate a zero sized array.");
#else
            size = 1;
#endif
        }
        
        _ptr = new type[ size ];
        _size = size;
    }
    
    void _ReAssign(const size_type _OldSize, const size_type _NewSize, int indx = -1, type fillval = 0)
    {
		if(_NewSize == 0)
		{
			_Delete();
			return;
		}
		
        pointer _oldPtr = _ptr;
        _ptr = new type[ _NewSize ]{};
		if(indx < 0 || indx > _size) indx = _size;
        
        const size_type _copyRange = (_OldSize < _NewSize ? _OldSize : _NewSize);
		const size_type size_diff = zc_max(_OldSize,_NewSize)-zc_min(_OldSize,_NewSize);
        
		if(indx == _size)
		{
			for(size_type i(0); i < _copyRange; i++)
				_ptr[ i ] = _oldPtr[ i ];
			if(_OldSize < _NewSize)
				Assign(_OldSize, _NewSize, fillval);
        }
		else if(_OldSize < _NewSize)
		{
			size_type offs = 0;
			for(size_type i(0); i < _copyRange; i++)
			{
				if(i==indx)
				{
					offs = size_diff;
					Assign(i, i+size_diff, fillval);
				}
				_ptr[ i+offs ] = _oldPtr[ i ];
			}
		}
		else
		{
			size_type offs = 0;
			for(size_type i(0); i < _copyRange; i++)
			{
				if(i==indx)
					offs = size_diff;
				_ptr[ i ] = _oldPtr[ i+offs ];
			}
		}
            
        _Delete(_oldPtr);
        _size = _NewSize;
    }
    
    void _Delete()
    {
        if(_ptr)
            delete [] _ptr;
            
        _ptr = NULL;
        
        _size = 0;
    }
    
    void _Delete(pointer _Ptr)
    {
        if(_Ptr)
            delete [] _Ptr;
            
        _Ptr = NULL;
    }
    
    void _SetDimensions(size_type _Z, size_type _Y, size_type _X)
    {
        _dim[0] = _X;
        _dim[1] = _Y;
        _dim[2] = _Z;
        _dim[3] = (_X * _Y);
        
        _size = _GetSize(_X, _Y, _Z);
    }
	
	bool _SameDimensions(size_type _X)
	{
		return _dim[0] == _X && _dim[1] == 0 && _dim[2] == 0;
	}
	bool _SameDimensions(size_type _Y, size_type _X)
	{
		return _dim[0] == _X && _dim[1] == _Y && _dim[2] == 0;
	}
	bool _SameDimensions(size_type _Z, size_type _Y, size_type _X)
	{
		return _dim[0] == _X && _dim[1] == _Y && _dim[2] == _Z;
	}
    
    size_type _GetSize(size_type _Z, size_type _Y, size_type _X) const
    {
        if(_Z > 0)
            return (_X * _Y * _Z);
        else if(_Y > 0)
            return (_X * _Y);
            
        return (_X);
    }
    
    void _BoundsExp(size_type _X)
    {
        if(_X >= _size)
        {
            al_trace("Array indices out of range.\n");
            throw("Array indices out of range.");
        }
    }
    
    void _BoundsExp(size_type _X, size_type _Y) const
    {
        if(Offset(_Y, _X) >= _size)
        {
            al_trace("Array indices out of range.\n");
            throw("Array indices out of range.");
        }
    }
    
    void _BoundsExp(size_type _X, size_type _Y, size_type _Z) const
    {
        if(Offset(_Z, _Y, _X) >= _size)
        {
            al_trace("Array indices out of range.\n");
            throw("Array indices out of range.");
        }
    }
    
    bool _Bounds(size_type _X)
    {
        if(_X >= _size)
        {
            al_trace("Array index (%i) out of range (%i).\n", _X, _size);
            return false;
        }
        
        return true;
    }
    
    bool _Bounds(size_type _X, size_type _Y) const
    {
        if(Offset(_Y, _X) >= _size)
        {
            al_trace("Array indices out of range.\n");
            return false;
        }
        
        return true;
    }
    
    bool _Bounds(size_type _X, size_type _Y, size_type _Z) const
    {
        if(Offset(_Z, _Y, _X) >= _size)
        {
            al_trace("Array indices out of range.\n");
            return false;
        }
        
        return true;
    }
    
private:
    pointer _ptr;
    size_type _size;
    size_type _dim[ 4 ];
	bool _valid;
	bool _object_type;
    
};

typedef ZCArray<int32_t> ZScriptArray;

#endif


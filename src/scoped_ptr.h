//////////////////////////////////////
// Fast Container Library
// MIT License
//////////////////////////////////////
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once
#include <cstddef> // for size_t

template <class T>
struct scoped_ptr_deleter
{
	typedef T value_type;
	void operator()( const T* p ) const { delete p; }
};


template <class T>
struct scoped_array_ptr_deleter
{
	typedef T value_type;
	void operator()( const T* p ) const { delete [] p; }
};



// scoped_ptr

template < class T, class Deleter = scoped_ptr_deleter<T> > 
class scoped_ptr
{
public:
	typedef T					element_type;
	typedef scoped_ptr<T>		this_type;
	typedef Deleter				deleter_type;

	explicit scoped_ptr( T* p = 0 )
		: _ptr(p)
		{
		}

	~scoped_ptr()
		{
			delete_ptr();
		}

	operator bool() const { return _ptr != 0; }
	bool operator!() const { return _ptr == 0; }

	T& operator*() const { return *_ptr; }
	T* operator->() const { return _ptr; }
	T* get() const { return _ptr; }

	void reset( T* p = 0 )
	{
		if( p != _ptr )
		{
			delete_ptr();
			_ptr = p;
		}
	}
 
	void swap( scoped_ptr& x )
	{
		T* tmp = _ptr;
		_ptr = x._ptr;
		x._ptr = tmp;
	}

	void delete_ptr() const
	{
		Deleter deleter;
		deleter(_ptr);
	}

protected:
	scoped_ptr( const scoped_ptr& );
	scoped_ptr& operator=( const scoped_ptr& );
	scoped_ptr& operator=( T* p );

	T* _ptr;
};



template <class T, class D> inline
	void swap( scoped_ptr<T, D>& a, scoped_ptr<T, D>& b )
	{
		a.swap(b);
	}

template <class T, class D>
	bool operator <(const scoped_ptr<T, D>& a, const scoped_ptr<T, D>& b)
	{
		return (a.get() < b.get());
	}




// scoped_array_ptr

template < class T, class Deleter = scoped_array_ptr_deleter<T> > 
class scoped_array_ptr
{
public:
	typedef T						element_type;
	typedef scoped_array_ptr<T>		this_type;
	typedef Deleter					deleter_type;

	explicit scoped_array_ptr( T* p = 0 )
		: _ptr(p)
		{
		}

	~scoped_array_ptr()
		{
			delete_ptr();
		}

	operator bool() const { return _ptr != 0; }
	bool operator!() const { return _ptr == 0; }

	T& operator*() const { return *_ptr; }
	T* operator->() const { return _ptr; }
	T* get() const { return _ptr; }

	T& operator[]( size_t i ) const { return _ptr[i]; }

	void reset( T* p = 0 )
	{
		if( p != _ptr )
		{
			delete_ptr();
			_ptr = p;
		}
	}
 
	void swap( scoped_array_ptr& x )
	{
		T* tmp = _ptr;
		_ptr = x._ptr;
		x._ptr = tmp;
	}

	void delete_ptr() const
	{
		Deleter deleter;
		deleter(_ptr);
	}

protected:
	scoped_array_ptr( const scoped_array_ptr& );
	scoped_array_ptr& operator=( const scoped_array_ptr& );
	scoped_array_ptr& operator=( T* p );

	T* _ptr;
};



template <class T, class D> inline
	void swap( scoped_array_ptr<T, D>& a, scoped_array_ptr<T, D>& b )
	{
		a.swap(b);
	}

template <class T, class D>
	bool operator <( const scoped_array_ptr<T, D>& a, const scoped_array_ptr<T, D>& b )
	{
		return (a.get() < b.get());
	}



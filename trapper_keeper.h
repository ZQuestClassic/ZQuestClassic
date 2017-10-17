
/* For stupid memory management bullshit */
#ifndef __trapperkeeper_h_
#define __trapperkeeper_h_


#ifndef TK_NO_CRTDBG
#ifdef _MSC_VER


#pragma once

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


	#ifndef TK_NO_NEW_REMAP
		//taken from msdn
		#ifdef _DEBUG
			#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
			#define new DEBUG_NEW
		#endif
	#endif


	#ifdef _DEBUG
		inline static void InitCrtDebug( int flags = 0 )
			{
				if( flags == 0  )
				flags = 
					_CRTDBG_ALLOC_MEM_DF 
					| 
					_CRTDBG_LEAK_CHECK_DF 
					//|
					//_CRTDBG_REPORT_FLAG
					;


				_CrtSetDbgFlag(flags);

			}
	#else // NOT _DEBUG
		/*inline static void InitCrtDebug( int flags = 0 )
			{
				flags = flags; //do nothing
			}*/
	#endif

#else // NOT _MSC_VER
	/*static void InitCrtDebug( int flags = 0 )
		{
			flags = flags; //do nothing
		}*/
#endif //_MSC_VER
#endif //TK_HEADER_NO_DEBUG_REMAP


//#include <list>
#include <vector>
#include <utility>



namespace zc
{

/*
template <class T>
	inline T* alloc( size_t size ) not used
		{
			T* base = new T[ size ];
			trapper_keeper.insorb( (TrapperKeeper::trap_type*)base, size );
			return base;
		}

template <class T>
	inline T* free( T* ptr ) not used
		{
			trapper_keeper.poop( (TrapperKeeper::trap_type*)ptr );
			delete [] ptr;
			return NULL;
		}
*/


/* Like auto_ptr for c++ or c arrays. Non-Transferrable */
template <class T>
class auto_ptr
{
public:
	typedef T* pointer;
	typedef T& ref;

	enum __AllocationType {
		_isMalloc,
		_isNew,
		_isNotArray
	};

	auto_ptr( pointer p, int _AllocType = _isNew ) 
		: _ptr(p), _alv(_AllocType) 
	{}
	~auto_ptr()
		{
			if(_alv == _isMalloc) 
				this->free_memory();
			else this->delete_memory();
		}

	pointer set_ptr( pointer p ) { return (_ptr = p); }
	pointer get_ptr() const { return _ptr; }

	pointer free_memory()
		{
			if(_ptr) 
				free(_ptr);
			return (_ptr = NULL);
		}

	pointer delete_memory()
		{
			if(_ptr)
			{
				if( _alv != _isNew )
					delete _ptr;
				else 
					delete [] _ptr;
			}
			return (_ptr = NULL);
		}

protected:
	pointer _ptr;
	int _alv;

private:
	auto_ptr( const auto_ptr& );
	auto_ptr &operator=( const auto_ptr& );

};


template <class T>
class smart_ptr
{
private:
	T* _ptr;

public:

	smart_ptr()								: _ptr(NULL)			{}
	smart_ptr( T* _NewPtr )					: _ptr(_NewPtr)			{}
	smart_ptr( const smart_ptr& _NewPtr )	: _ptr(_NewPtr._ptr)	{}
	~smart_ptr()
	{
		_Delete();
	}

	T* operator () ( void ) const { return _ptr; }

	/* So we can hijack a new operator assignment*/
/*	T* operator = ( T* _NewPtr )
	{
		_Delete();
		return (_ptr = _NewPtr);
	}
*/
	smart_ptr &operator = ( const smart_ptr& _NewPtr )
	{
		_Delete();
		_ptr = _NewPtr._ptr;
		smart_ptr &_Ref = const_cast<smart_ptr&>(_NewPtr);
		_Ref._Orphan();
		return *this;
	}

	T* release()
	{
		T *tmp = _ptr;
		_Orphan();
		return tmp;
	}

	T* get() const
	{
		return _ptr;
	}

	T* set( T* _NewPtr )
	{
		_Delete();
		return (_ptr = _NewPtr);
	}

	//T& operator *() const	{ return *_ptr;	}
	//T* operator->() const	{ return _ptr;	}

	T &operator []		( const int i )			{ return _ptr[ i ]; }
	const T operator [] ( const int i ) const	{ return _ptr[ i ]; }

	bool operator == ( const T* _Rhs ) const { return (_ptr == _Rhs); }
	bool operator != ( const T* _Rhs ) const { return (_ptr != _Rhs); }
	bool operator >= ( const T* _Rhs ) const { return (_ptr >= _Rhs); }
	bool operator <= ( const T* _Rhs ) const { return (_ptr <= _Rhs); }
	bool operator >  ( const T* _Rhs ) const { return (_ptr >  _Rhs); }
	bool operator <  ( const T* _Rhs ) const { return (_ptr <  _Rhs); }

private:

	smart_ptr &_Delete()
	{
		if(_ptr)
		{
			delete [] _ptr;
			_Orphan();
		}
		return *this;
	}

	void _Orphan()
	{
		_ptr = NULL;
	}

};



} //namespace zc


template <class T>
class TheButcher //There is entirely way too much rape going on.
{
private:
	std::vector<T*> _rape;
public:

	TheButcher()  { _rape.reserve( 512 ); }
	~TheButcher() { angry_mob(); }

	T *rape( unsigned int length )
		{
			_rape.push_back( new T[ length ] );
			return _rape.back();
		}

	void angry_mob()
		{
		    // I need 'typename' here to compile. -L
			typename std::vector<T*>::iterator it;
			for( it = _rape.begin(); it != _rape.end(); ++it )
			{
				delete [] (*it);
			}
			_rape.clear();
		}

};


static TheButcher<char> the_char_butcher;



#endif

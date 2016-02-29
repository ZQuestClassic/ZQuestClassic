#ifndef _zc_garbagecollector_h_
#define _zc_garbagecollector_h_

#include <vector>


class GarbageCollector
{
private:
    class __gcAny
    {
    private:
        class __gcBase
        {
        public:
            virtual ~__gcBase() {}
        };
        
        template<class T>
        class __gcType : public __gcBase
        {
        public:
            __gcType(const T& type, bool a = false) : _p(type), _array(a) {}
            ~__gcType()
            {
                if(_p)
                {
                    if(_array)
                        delete [] _p;
                    else
                        delete _p;
                        
                    _p = 0;
                }
            }
            
        private:
            T _p;
            bool _array;
        };
        
    public:
        template<class T>
        __gcAny(const T& ptr, bool _array = false) : _p(new __gcType<T>(ptr, _array)) {}
        __gcAny(const __gcAny& a) : _p(a._p) {}
        ~__gcAny()
        {
            if(_p)
                delete _p, _p = 0;
        }
        
    private:
        __gcAny &operator = (const __gcAny& a);
        __gcBase* _p;
    };
    
    std::vector<__gcAny*> _bin;
    
public:
    GarbageCollector() {}
    ~GarbageCollector()
    {
        destroy_garbage();
    }
    
    void destroy_garbage()
    {
        // lifo deletion
        for(std::vector<__gcAny*> ::iterator it = _bin.end(); it != _bin.begin();)
            delete(*--it);
    }
    
    template<class _Ptr>
    _Ptr operator()(const _Ptr& _collect)
    {
        return collect(_collect, false);
    }
    
    template<class _Ptr>
    _Ptr operator [](const _Ptr& _collect)
    {
        return collect(_collect, true);
    }
    
    template<class _Ptr>
    _Ptr collect(const _Ptr& _collect, bool _isArray)
    {
        _Ptr ptr = _collect;
        _bin.push_back(new __gcAny(_collect, _isArray));
        return ptr;
    }
    
};




#endif


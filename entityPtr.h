#ifndef _ZC_ENTITYPTR_H_
#define _ZC_ENTITYPTR_H_

#include "debug.h"

/** Used to track the number of EntityPtrs to a given sprite and
 *  whether the sprite has been deleted.
 */
class EntityRef
{
public:
    EntityRef(): refCount(0), deleted(false) {}
    inline void inc() { refCount++; }
    inline void dec() { ZCASSERT(refCount>=1); if(--refCount==0 && deleted) delete this; }
    inline void onEntityDelete() { if(refCount<=0) delete this; else deleted=true; }
    
    int refCount;
    bool deleted;
};

/** A smart pointer class for sprites. When cast to bool, it evaluates to false
 *  if no sprite is set or if the sprite has been deleted.
 */
template<typename T>
class EntityPtr
{
public:
    EntityPtr():
        spr(0),
        ref(0)
    {
    }
    
    EntityPtr(T* s):
        spr(s),
        ref(s->ref)
    {
        ref->inc();
    }
    
    EntityPtr(EntityPtr& other):
        spr(other.spr),
        ref(other.ref)
    {
        ref->inc();
    }
    
    ~EntityPtr()
    {
        if(ref) ref->dec();
    }
    
    inline EntityPtr& operator=(const EntityPtr& other)
    {
        reset(other.spr);
        return *this;
    }
    
    void clear()
    {
        if(ref) ref->dec();
        spr=0;
        ref=0;
    }
    
    void reset(T* newT)
    {
        if(spr==newT)
            return;
        if(ref) ref->dec();
        spr=newT;
        ref=newT->ref;
        if(ref) ref->inc();
    }
    
    void del()
    {
        // Should this set ref->deleted? Currently, that's handled by ~sprite(),
        // so pointers don't consider it deleted until the next frame.
        if(spr && !ref->deleted)
            spr->markForDeletion();
        clear();
    }
    
    inline operator bool() const { return spr && !ref->deleted; }
    inline T& operator*() { return *spr; }
    inline T* operator->() { return spr; }
    inline T* get() { return spr; }
    inline const T& operator*() const { return *spr; }
    inline const T* operator->() const { return spr; }
    inline const T* get() const { return spr; }
    
private:
    T* spr;
    EntityRef* ref;
};

#endif

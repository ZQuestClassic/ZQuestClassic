#ifndef _zscript_rendertarget_h_
#define _zscript_rendertarget_h_

#include<allegro.h>


#ifdef _MSC_VER
#ifndef _FORCE_INLINE
#define _FORCE_INLINE __forceinline
#endif
#else
#ifndef _FORCE_INLINE
#define _FORCE_INLINE inline
#endif
#endif



class ZScriptDrawingRenderTarget
{
public:
    static const int MaxBuffers = 7;
    
    //These aren't allocated unless requested by the user,
    //so we can handle sizes up to 512x512 with script drawing.
    static const int BitmapWidth = 512;
    static const int BitmapHeight = 512;
    
protected:
    BITMAP* _bitmap[ MaxBuffers ];
    int _current_target;
    
public:
    ZScriptDrawingRenderTarget() : _current_target(-1)
    {
        for(int i(0); i < MaxBuffers; ++i)
        {
            _bitmap[i] = 0;
        }
    }
    
    ~ZScriptDrawingRenderTarget()
    {
        for(int i(0); i < MaxBuffers; ++i)
        {
            if(_bitmap[i])
                destroy_bitmap(_bitmap[i]);
        }
    }
    
    _FORCE_INLINE void SetCurrentRenderTarget(int target)
    {
        _current_target = target;
    }
    
    _FORCE_INLINE int GetCurrentRenderTarget()
    {
        return _current_target;
    }
    
    _FORCE_INLINE BITMAP* GetTargetBitmap(int target)
    {
        if(target < 0 || target >= MaxBuffers)
            return 0;
            
        if(!_bitmap[target])
            _bitmap[target] = create_bitmap_ex(8, BitmapWidth, BitmapHeight);
            
        return _bitmap[target];
    }
    
    BITMAP* GetBitmapPtr(int target)
    {
        if(target < 0 || target >= MaxBuffers)
            return 0;
            
        return _bitmap[target];
    }
    
private:
    ZScriptDrawingRenderTarget(const ZScriptDrawingRenderTarget&);
    ZScriptDrawingRenderTarget &operator =(const ZScriptDrawingRenderTarget&);
};

extern ZScriptDrawingRenderTarget* zscriptDrawingRenderTarget;



//void do_primitives(BITMAP *targetBitmap, int type, mapscr *, int xoffset, int yoffset);


#endif


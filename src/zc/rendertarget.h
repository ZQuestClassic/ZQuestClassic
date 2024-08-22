#ifndef ZSCRIPT_RENDERTARGET_H_
#define ZSCRIPT_RENDERTARGET_H_

#include "base/compiler.h"

#include <allegro.h>

class ZScriptDrawingRenderTarget
{
public:
    static const int32_t MaxBuffers = 7;
    
    //These aren't allocated unless requested by the user,
    //so we can handle sizes up to 512x512 with script drawing.
    static const int32_t BitmapWidth = 512;
    static const int32_t BitmapHeight = 512;
    
protected:
    BITMAP* _bitmap[ MaxBuffers ];
    int32_t _current_target;
    
public:
    ZScriptDrawingRenderTarget() : _current_target(-1)
    {
        for(int32_t i(0); i < MaxBuffers; ++i)
        {
            _bitmap[i] = 0;
        }
    }
    
    ~ZScriptDrawingRenderTarget()
    {
        for(int32_t i(0); i < MaxBuffers; ++i)
        {
            if(_bitmap[i])
                destroy_bitmap(_bitmap[i]);
        }
    }
    
    ZC_FORCE_INLINE void SetCurrentRenderTarget(int32_t target)
    {
        _current_target = target;
    }
    
    ZC_FORCE_INLINE int32_t GetCurrentRenderTarget()
    {
        return _current_target;
    }
    
    ZC_FORCE_INLINE BITMAP* GetTargetBitmap(int32_t target)
    {
        if(target < 0 || target >= MaxBuffers)
            return 0;
            
        if(!_bitmap[target])
            _bitmap[target] = create_bitmap_ex(8, BitmapWidth, BitmapHeight);
            
        return _bitmap[target];
    }
    
    BITMAP* GetBitmapPtr(int32_t target)
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


#endif


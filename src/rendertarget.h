#ifndef _zscript_rendertarget_h_
#define _zscript_rendertarget_h_

#include<allegro.h>
#include "util.h"


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

	struct SourceParamInfo
	{
		int i;
		int x, y, w, h;
	};

	static const int MaxBuffers = 7;
    
    //These aren't allocated unless requested by the user,
    //so we can handle sizes up to 512x512 with script drawing.
    static const int BitmapWidth = 512;
    static const int BitmapHeight = 512;
    
protected:
    BITMAP* _bitmap[ MaxBuffers ];
    int _current_target;
	SourceParamInfo _sourceInfo;
   
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

	_FORCE_INLINE void CreateBitmap(int index, int w, int h, int depth = 8)
	{
		if((unsigned)index < MaxBuffers)
		{
			int minBitmapSize = 1;
			int maxBitmapSize = 2048;
			w = clamp(w, minBitmapSize, maxBitmapSize);
			h = clamp(h, minBitmapSize, maxBitmapSize);

			if(_bitmap[index])
			{
				if(_bitmap[index]->w == w && _bitmap[index]->h == h)
					return;

				destroy_bitmap(_bitmap[index]);
				_bitmap[index] = NULL;
			}

			_bitmap[index] = create_bitmap_ex(depth, w, h);
		}
	}
  
    _FORCE_INLINE void SetCurrentRenderSource(int source, int x, int y, int w, int h)
    {
		SourceParamInfo si = { source, x, y, w, h };
        _sourceInfo = si;
    }
    
    _FORCE_INLINE int GetCurrentRenderSource()
    {
        return _sourceInfo.i;
    }

	_FORCE_INLINE SourceParamInfo& GetCurrentRenderSourceInfo()
	{
		return _sourceInfo;
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


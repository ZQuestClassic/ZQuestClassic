#ifndef _ZC_GUIBITMAPRENDERER_H_
#define _ZC_GUIBITMAPRENDERER_H_

#include <types.h>
struct BITMAP;

class GUIBitmapRenderer
{
public:
    GUIBitmapRenderer(int scale);
    ~GUIBitmapRenderer();
    void initialize(int width, int height);
    inline BITMAP* getBitmap() { return bmp; }
    void* getData();
    void getActualSize(int& width, int& height) const;
    inline int getScale() const { return scale; }
    
    void clear();
    void drawRect(int x, int y, int width, int height, int color);
    void fillRect(int x, int y, int width, int height, int color);
    void drawLine(int x1, int y1, int x2, int y2, int color);
    void putTile(int tile, int cset, Orientation orientation, int x, int y);
    
private:
    const int scale;
    BITMAP* bmp;
};

#endif

#ifndef _ZC_GUI_ALLEGRO_RENDERER_H_
#define _ZC_GUI_ALLEGRO_RENDERER_H_

#include <stack>
#include <string>
struct BITMAP;
struct FONT;

namespace GUI
{

struct GUIColor;

class AllegroGUIRenderer
{
public:
    enum textAlignment { ta_left, ta_center };
    enum frameType { ft_convex, ft_concave };
    enum partialRoundedRect { prr_top, prr_bottom, prr_topRight, prr_bottomRight };
    
    AllegroGUIRenderer();
    ~AllegroGUIRenderer();
    void pushClipRect(int x, int y, int width, int height);
    void popClipRect();
    
    void drawLine(int x1, int y1, int x2, int y2, const GUIColor& color);
    void drawText(const std::string& text, int x, int y, FONT* font,
      const GUIColor& color, textAlignment alignment=ta_left);
    void drawRect(int x, int y, int width, int height, const GUIColor& color);
    void fillRect(int x, int y, int width, int height, const GUIColor& color);
    void fillCircle(int x, int y, int radius, const GUIColor& color);
    void drawRoundedRect(int x, int y, int width, int height, int radius, const GUIColor& color);
    void fillRoundedRect(int x, int y, int width, int height, int radius, const GUIColor& color);
    void drawPartialRoundedRect(int x, int y, int width, int height, int radius,
      partialRoundedRect part, const GUIColor& color);
    void fillPartialRoundedRect(int x, int y, int width, int height, int radius,
      partialRoundedRect part, const GUIColor& color);
    void fillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, const GUIColor& color);
    void drawFrame(int x, int y, int width, int height, const GUIColor& color, frameType type, int depth);
    void blitBitmap(BITMAP* bmp, int x, int y);
    
    inline void setOrigin(int x, int y) { originX=x; originY=y; }
    
private:
    struct ClipRect { int x1, y1, x2, y2; };
    
    BITMAP* target;
    int originX, originY;
    std::stack<ClipRect> clipRectStack;
    
    inline void adjustPoint(int& x, int& y) const { x+=originX; y+=originY; }
};

}

#endif

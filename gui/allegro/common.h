#ifndef _ZC_GUI_ALLEGRO_COMMON_H_
#define _ZC_GUI_ALLEGRO_COMMON_H_

#include <jwin.h>
#include <allegro.h>
#include <string>

struct MouseInput;

extern BITMAP* hw_screen;
extern volatile int myvsync;
extern int zqwin_scale;
extern int scheme[jcMAX];

int gui_textout_ln(BITMAP *bmp, unsigned char *s, int x, int y, int color, int bg, int pos);
bool is_zquest();

namespace GUI
{

struct GUIColor
{
    int red, green, blue;
    inline operator int() const { return makecol(red, green, blue); }
    GUIColor adjust(int ar, int ag, int ab) const;
    GUIColor adjust(float ar, float ag, float ab) const;
};

struct GUIPalette
{
    GUIColor outline;
    
    GUIColor winBorder;
    GUIColor winTitleText;
    GUIColor winBody;
    GUIColor winBodyText;
    
    GUIColor editBG;
    GUIColor editText;
    GUIColor editHL;
    GUIColor editInactiveHL;
    GUIColor editTextHL;
        
    GUIColor button;
    GUIColor buttonHL;
    GUIColor buttonText;
    GUIColor buttonTextHL;
    
    GUIColor scrollbarFill;
    GUIColor scrollbarSlider;
};

extern GUIPalette pal;

#define MSG_SCROLL MSG_USER
#define D_WANTWHEEL D_USER

// Is this useful?  Not sure.
inline bool dialogUnderMouse(DIALOG* d)
{
    return mouse_in_rect(d->x, d->y, d->w, d->h);
}

// Things that have maximum sizes call this from setSizeAndPos()
// to adjust inputs.
inline void adjustSizeAndPos(int& pos, int& size, int maxSize)
{
    pos+=(size-maxSize)/2;
    size=maxSize;
}

/// Forwards MSG_WHEEL to DIALOGs that handle it.
void forwardMouseWheel(DIALOG* d, int c);

/// Translates Allegro dialog messages into MouseInput.
void translateMouseInput(int msg, int c, int bmpX, int bmpY, int scale,
  MouseInput& out);

/// Dialog proc that does nothing.
int dummyProc(int msg, DIALOG* d, int c);

/// Finds the character preceded by '&' in a string.
inline char getKey(const std::string& text)
{
    int amp=text.find('&');
    return (amp>=0) ? text[amp+1] : '\0';
}

}

#endif

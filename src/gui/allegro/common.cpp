#include "common.h"
#include "scrollbar.h"
#include "../key.h"
#include "../mouse.h"
#include <zsys.h>

namespace GUI
{

namespace
{

struct KeyMapEntry
{
    int alleg;
    zcKey zc;
};

const KeyMapEntry keyMap[]={
    { KEY_ESC, key_ESCAPE },
    { KEY_ENTER, key_ENTER },
    { KEY_SPACE, key_SPACE },
    { KEY_EQUALS, key_PLUS }, { KEY_PLUS_PAD, key_PLUS },
    { KEY_MINUS, key_MINUS }, { KEY_MINUS_PAD, key_MINUS },
    { KEY_UP, key_UP },
    { KEY_DOWN, key_DOWN },
    { KEY_LEFT, key_LEFT },
    { KEY_RIGHT, key_RIGHT },
    { KEY_PGUP, key_PAGEUP },
    { KEY_PGDN, key_PAGEDOWN },
    { KEY_HOME, key_HOME },
    { KEY_END, key_END },
    { KEY_A, key_A },
    { KEY_B, key_B },
    { KEY_C, key_C },
    { KEY_D, key_D },
    { KEY_E, key_E },
    { KEY_F, key_F },
    { KEY_G, key_G },
    { KEY_H, key_H },
    { KEY_I, key_I },
    { KEY_J, key_J },
    { KEY_K, key_K },
    { KEY_L, key_L },
    { KEY_M, key_M },
    { KEY_N, key_N },
    { KEY_O, key_O },
    { KEY_P, key_P },
    { KEY_Q, key_Q },
    { KEY_R, key_R },
    { KEY_S, key_S },
    { KEY_T, key_T },
    { KEY_U, key_U },
    { KEY_V, key_V },
    { KEY_W, key_W },
    { KEY_X, key_X },
    { KEY_Y, key_Y },
    { KEY_Z, key_Z },
    { KEY_1, key_1 }, { KEY_1_PAD, key_1 },
    { KEY_2, key_2 }, { KEY_2_PAD, key_2 },
    { KEY_3, key_3 }, { KEY_3_PAD, key_3 },
    { KEY_4, key_4 }, { KEY_4_PAD, key_4 },
    { KEY_5, key_5 }, { KEY_5_PAD, key_5 },
    { KEY_6, key_6 }, { KEY_6_PAD, key_6 },
    { KEY_7, key_7 }, { KEY_7_PAD, key_7 },
    { KEY_8, key_8 }, { KEY_8_PAD, key_8 },
    { KEY_9, key_9 }, { KEY_9_PAD, key_9 },
    { KEY_0, key_0 }, { KEY_0_PAD, key_0 },
    { KEY_F1, key_F1 },
    { KEY_F2, key_F2 },
    { KEY_F3, key_F3 },
    { KEY_F4, key_F4 },
    { KEY_F5, key_F5 },
    { KEY_F6, key_F6 },
    { KEY_F7, key_F7 },
    { KEY_F8, key_F8 },
    { KEY_F9, key_F9 },
    { KEY_F10, key_F10 },
    { KEY_F11, key_F11 },
    { KEY_F12, key_F12 },
    { 0, key_ESCAPE }
};

} // namespace

GUIColor GUIColor::adjust(int ar, int ag, int ab) const
{
    GUIColor gc;
    gc.red=vbound(red+ar, 0, 255);
    gc.green=vbound(green+ag, 0, 255);
    gc.blue=vbound(blue+ab, 0, 255);
    return gc;
}

GUIColor GUIColor::adjust(float ar, float ag, float ab) const
{
    GUIColor gc;
    gc.red=vbound(static_cast<int>(ar*red), 0, 255);
    gc.green=vbound(static_cast<int>(ag*green), 0, 255);
    gc.blue=vbound(static_cast<int>(ab*blue), 0, 255);
    return gc;
}

GUIPalette pal=
{
    { 0, 16, 32 },      // outline
    
    { 0, 64, 128 },     // winBorder
    { 255, 255, 255 },  // winTitleText
    { 224, 224, 224 },  // winBody
    { 0, 0, 0},         // winBodyText
    
    { 255, 255, 255 },  // editBG
    { 0, 0, 0 },        // editText
    { 0, 64, 128 },     // editHL
    { 64, 64, 64 },     // editInactiveHL
    { 255, 255, 255 },  // editTextHL
    
    { 0, 64, 128 },     // button
    { 128, 192, 255 },  // buttonHL
    { 255, 255, 255 },  // buttonText
    { 255, 255, 255 },  // buttonTextHL
    
    { 128, 192, 255 },  // scrollbarFill
    { 0, 64, 128 }      // scrollbarSlider
};

// Allegro doesn't offer an easy way for two DIALOGs in the same place
// both to handle only a subset of mouse input...
void forwardMouseWheel(DIALOG* d, int c)
{
    for(DIALOG* d2=d; d2->proc; d2++)
    {
        if((d2->flags&D_WANTWHEEL)!=0 && dialogUnderMouse(d2))
        {
            object_message(d2, MSG_WHEEL, c);
            break;
        }
    }
}

void translateMouseInput(int msg, int c, int bmpX, int bmpY, int scale,
  MouseInput& out)
{
    switch(msg)
    {
    case MSG_IDLE:
        out.type=mouse_MOTION;
        break;
        
    case MSG_LPRESS:
        out.type=mouse_LEFTBUTTONDOWN;
        break;
        
    case MSG_LRELEASE:
        out.type=mouse_LEFTBUTTONUP;
        break;
    
    case MSG_RPRESS:
        out.type=mouse_RIGHTBUTTONDOWN;
        break;
        
    case MSG_RRELEASE:
        out.type=mouse_RIGHTBUTTONUP;
        break;
        
    case MSG_WHEEL:
        if(c<0)
            out.type=mouse_WHEELUP;
        else
            out.type=mouse_WHEELDOWN;
        break;
        
    default:
        return;
    }
    
    out.x=(gui_mouse_x()-bmpX)/scale;
    out.y=(gui_mouse_y()-bmpY)/scale;
    out.shiftPressed=key[KEY_LSHIFT] || key[KEY_RSHIFT];
    out.ctrlPressed=key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL];
    out.altPressed=key[KEY_ALT]||key[KEY_ALTGR];
}

bool translateKeyInput(int keycode, KeyInput& out)
{
    keycode>>=8;
    bool found=false;
    int i=0;
    for(; keyMap[i].alleg!=0; i++)
    {
        if(keyMap[i].alleg==keycode)
        {
            found=true;
            break;
        }
    }
    
    if(!found)
        return false;
    
    out.key=keyMap[i].zc;
    out.shiftPressed=key[KEY_LSHIFT] || key[KEY_RSHIFT];
    out.ctrlPressed=key[KEY_ZC_LCONTROL] || key[KEY_ZC_RCONTROL];
    out.altPressed=key[KEY_ALT]||key[KEY_ALTGR];
    
    return true;
}

int dummyProc(int msg, DIALOG* d, int c)
{
    // TODO: Does this need to refuse mouse input? Won't it block
    // the scroll wheel otherwise?
    if(msg==MSG_WHEEL)
        forwardMouseWheel(d, c);
    return D_O_K;
}

}

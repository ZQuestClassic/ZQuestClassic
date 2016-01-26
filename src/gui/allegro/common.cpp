#include "common.h"
#include "scrollbar.h"
#include "../mouse.h"
#include "../../zsys.h"

namespace GUI
{

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

int dummyProc(int msg, DIALOG* d, int c)
{
    // TODO: Does this need to refuse mouse input? Won't it block
    // the scroll wheel otherwise?
    if(msg==MSG_WHEEL)
        forwardMouseWheel(d, c);
    return D_O_K;
}

}

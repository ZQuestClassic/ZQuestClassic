#include "util.h"
#include "../key.h"
#include "../mouse.h"

namespace
{

struct KeyMapEntry
{
    int gtk;
    zcKey zc;
};

const KeyMapEntry keyMap[]={
    { GDK_KEY_Escape, key_ESCAPE },
    { GDK_KEY_Return, key_ENTER },
    { GDK_KEY_space, key_SPACE },
    { GDK_KEY_plus, key_PLUS },
    { GDK_KEY_equal, key_PLUS },
    { GDK_KEY_KP_Add, key_PLUS },
    { GDK_KEY_minus, key_MINUS },
    { GDK_KEY_underscore, key_MINUS },
    { GDK_KEY_KP_Subtract, key_MINUS },
    { GDK_KEY_Up, key_UP },
    { GDK_KEY_Down, key_DOWN },
    { GDK_KEY_Left, key_LEFT },
    { GDK_KEY_Right, key_RIGHT },
    { GDK_KEY_Page_Up, key_PAGEUP },
    { GDK_KEY_Page_Down, key_PAGEDOWN },
    { GDK_KEY_Home, key_HOME },
    { GDK_KEY_End, key_END },
    { GDK_KEY_A, key_A }, { GDK_KEY_a, key_A },
    { GDK_KEY_B, key_B }, { GDK_KEY_b, key_B },
    { GDK_KEY_C, key_C }, { GDK_KEY_c, key_C },
    { GDK_KEY_D, key_D }, { GDK_KEY_d, key_D },
    { GDK_KEY_E, key_E }, { GDK_KEY_e, key_E },
    { GDK_KEY_F, key_F }, { GDK_KEY_f, key_F },
    { GDK_KEY_G, key_G }, { GDK_KEY_g, key_G },
    { GDK_KEY_H, key_H }, { GDK_KEY_h, key_H },
    { GDK_KEY_I, key_I }, { GDK_KEY_i, key_I },
    { GDK_KEY_J, key_J }, { GDK_KEY_j, key_J },
    { GDK_KEY_K, key_K }, { GDK_KEY_k, key_K },
    { GDK_KEY_L, key_L }, { GDK_KEY_l, key_L },
    { GDK_KEY_M, key_M }, { GDK_KEY_m, key_M },
    { GDK_KEY_N, key_N }, { GDK_KEY_n, key_N },
    { GDK_KEY_O, key_O }, { GDK_KEY_o, key_O },
    { GDK_KEY_P, key_P }, { GDK_KEY_p, key_P },
    { GDK_KEY_Q, key_Q }, { GDK_KEY_q, key_Q },
    { GDK_KEY_R, key_R }, { GDK_KEY_r, key_R },
    { GDK_KEY_S, key_S }, { GDK_KEY_s, key_S },
    { GDK_KEY_T, key_T }, { GDK_KEY_t, key_T },
    { GDK_KEY_U, key_U }, { GDK_KEY_u, key_U },
    { GDK_KEY_V, key_V }, { GDK_KEY_v, key_V },
    { GDK_KEY_W, key_W }, { GDK_KEY_w, key_W },
    { GDK_KEY_X, key_X }, { GDK_KEY_x, key_X },
    { GDK_KEY_Y, key_Y }, { GDK_KEY_y, key_Y },
    { GDK_KEY_Z, key_Z }, { GDK_KEY_z, key_Z },
    { GDK_KEY_1, key_1 },
    { GDK_KEY_2, key_2 },
    { GDK_KEY_3, key_3 },
    { GDK_KEY_4, key_4 },
    { GDK_KEY_5, key_5 },
    { GDK_KEY_6, key_6 },
    { GDK_KEY_7, key_7 },
    { GDK_KEY_8, key_8 },
    { GDK_KEY_9, key_9 },
    { GDK_KEY_0, key_0 },
    { GDK_KEY_F1, key_F1 },
    { GDK_KEY_F2, key_F2 },
    { GDK_KEY_F3, key_F3 },
    { GDK_KEY_F4, key_F4 },
    { GDK_KEY_F5, key_F5 },
    { GDK_KEY_F6, key_F6 },
    { GDK_KEY_F7, key_F7 },
    { GDK_KEY_F8, key_F8 },
    { GDK_KEY_F9, key_F9 },
    { GDK_KEY_F10, key_F10 },
    { GDK_KEY_F11, key_F11 },
    { GDK_KEY_F12, key_F12 },
    { 0, key_ESCAPE }
};

}

bool translateKeyEvent(GdkEvent* event, KeyInput& out)
{
    bool found=false;
    int i=0;
    for(; keyMap[i].gtk!=0; i++)
    {
        if(keyMap[i].gtk==event->key.keyval)
        {
            found=true;
            break;
        }
    }
    
    if(!found)
        return false;
    
    out.key=keyMap[i].zc;
    out.shiftPressed=(event->key.state&GDK_SHIFT_MASK)!=0;
    out.ctrlPressed=(event->key.state&GDK_CONTROL_MASK)!=0;
    out.altPressed=(event->key.state&GDK_META_MASK)!=0;
    return true;
}

bool translateMouseEvent(GdkEvent* event, int scale, MouseInput& out)
{
    switch(event->type)
    {
    case GDK_MOTION_NOTIFY:
        out.type=mouse_MOTION;
        break;
        
    case GDK_BUTTON_PRESS:
        if(event->button.button==1)
            out.type=mouse_LEFTBUTTONDOWN;
        else if(event->button.button==3)
            out.type=mouse_RIGHTBUTTONDOWN;
        else
            return false;
        break;
    
    case GDK_BUTTON_RELEASE:
        if(event->button.button==1)
            out.type=mouse_LEFTBUTTONUP;
        else if(event->button.button==3)
            out.type=mouse_RIGHTBUTTONUP;
        else
            return false;
        break;
        
    case GDK_2BUTTON_PRESS:
        if(event->button.button==1)
            out.type=mouse_DOUBLECLICK;
        else
            return false;
        break;
        
    case GDK_SCROLL:
        if(event->scroll.direction==GDK_SCROLL_UP)
            out.type=mouse_WHEELUP;
        else if(event->scroll.direction==GDK_SCROLL_DOWN)
            out.type=mouse_WHEELDOWN;
        // There's also a delta_y field - does that need to be considered?
        break;
        
    default:
        return false;
    }
    
    // The relevant data for all three event types are in the same place
    out.x=event->button.x/scale;
    out.y=event->button.y/scale;
    out.shiftPressed=(event->button.state&GDK_SHIFT_MASK)!=0;
    out.ctrlPressed=(event->button.state&GDK_CONTROL_MASK)!=0;
    out.altPressed=(event->button.state&GDK_META_MASK)!=0;
    return true;
}

std::string convertMnemonic(const std::string& str)
{
    size_t pos=str.find('&');
    if(pos==std::string::npos)
        return str;
    std::string mod=str;
    mod.replace(pos, 1, "_");
    return mod;
}

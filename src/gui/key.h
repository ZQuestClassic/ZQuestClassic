#ifndef _ZC_GUI_KEY_H_
#define _ZC_GUI_KEY_H_

// Keyboard inputs. We'll likely want to replace this at some point,
// but it works for now.

enum zcKey
{
    key_ESCAPE, key_ENTER, key_SPACE,
    key_PLUS, key_MINUS,
    key_UP, key_DOWN, key_LEFT, key_RIGHT,
    key_PAGEUP, key_PAGEDOWN, key_HOME, key_END,
    key_A, key_B, key_C, key_D,
    key_E, key_F, key_G, key_H,
    key_I, key_J, key_K, key_L,
    key_M, key_N, key_O, key_P,
    key_Q, key_R, key_S, key_T,
    key_U, key_V, key_W, key_X,
    key_Y, key_Z, key_1, key_2,
    key_3, key_4, key_5, key_6,
    key_7, key_8, key_9, key_0,
    key_F1, key_F2, key_F3, key_F4,
    key_F5, key_F6, key_F7, key_F8,
    key_F9, key_F10, key_F11, key_F12
};

struct KeyInput
{
    zcKey key;
    bool shiftPressed;
    bool ctrlPressed;
    bool altPressed;
};

#endif

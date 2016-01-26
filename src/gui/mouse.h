#ifndef _ZC_GUI_MOUSE_H_
#define _ZC_GUI_MOUSE_H_

enum mouseInputType
{
    mouse_LEFTBUTTONDOWN, mouse_LEFTBUTTONUP,
    mouse_RIGHTBUTTONDOWN, mouse_RIGHTBUTTONUP,
    mouse_WHEELUP, mouse_WHEELDOWN,
    mouse_DOUBLECLICK, mouse_MOTION
};

struct MouseInput
{
    mouseInputType type;
    int x, y;
    bool shiftPressed;
    bool ctrlPressed;
    bool altPressed;
};

#endif
